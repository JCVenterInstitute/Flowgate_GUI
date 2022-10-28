/**
 * @file
 * Processes the gates of a gate tree.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 */
#pragma once





//
// Verify suitable compiler.
// -------------------------
// This software is written in C++17 and it uses POSIX-compliant functions.
// Verify that the current compiler supports this.
#if !defined(__cplusplus)
#error "Abort: This software must be compiled with a C++ compiler."
#elif __cplusplus < 201703L
#error "Abort: This software requires compilation using a C++17-capable compiler."
#endif

//
// Includes.
// ---------
// This software intentionally uses a mix of C and C++. C is used for
// standard data types, fast memory copies, etc.
//
// Data types.
#include <cstdint>      // size_t

// Standard C libraries.
#include <cstring>      // std::memcpy, std::memset

// Standard C++ libraries.
#include <map>          // std::map
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector

// FlowGate libraries.
#include "GatingCache.h" // Cache of gating state





namespace FlowGate {
namespace Gates {





//----------------------------------------------------------------------
//
// Gating!
//
//----------------------------------------------------------------------
/**
 * Performs gating of flow cytometry cell populations (i.e. events) in an
 * event table.
 *
 * This class performs two types of gating:
 *
 * @li "Manual" gating uses a gate's shape (e.g. a rectangle) to select
 *     events for inclusion or exclusion based upon event values.
 *
 * @li "DAFi" cluster gating uses a gate's shape to select entire clusters
 *     of events for inclusion or exclusion based upon the cluster's centroid.
 *
 *
 * <B>DAFi clustering</B><BR>
 * "DAFi" is an acronym for "Directed Automated Filtering and
 * Identification". The algorithm finds clusters on all parameters of
 * an event table, then uses a gate tree to select cluster centers as
 * inside or outside of a gate.
 *
 * The gate tree is user-authored. Rectangle gates, for instance, select
 * a 2D region of interest using two event parameters and a min/max
 * range on each one. In traditional hard-edged gating, all events
 * that fall within this gate's rectangle are assigned to the gate.
 * But in DAFi, the rectangle instead selects clusters, not events.
 * The clusters are computed on the events before gating and their bounds
 * may extend past the gate rectangle. This creates a kind of soft-edged
 * gating where the user's authored gates are a strong hint, but not an
 * absolute rule about what events are inside or outside of the gate.
 *
 * <B>History</B><BR>
 * The algorithm was developed and published as:
 *   "DAFi: A directed recursive data filtering and clustering approach
 *   for improving and interpreting data clustering identification of
 *   cell populations from polychromatic flow cytometry data", by
 *   Lee AJ, Chang I, Burel JG, Lindestam Arlehamn CS, Mandava A,
 *   Weiskopf D, Peters B, Sette A, Scheuermann RH, and Qian Y,
 *   Cytometry A., 2018 June, volume 93, number 6, pp. 597-610.
 *
 * The original C implementation is by Yu "Max" Qian at JCVI.
 */
class Gating final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate gating";

    /**
     * The software version number.
     */
    inline static const std::string VERSION = "1.4.0";

    /**
     * The software build date.
     */
    inline static const std::string BUILD_DATE =
        std::string( __DATE__ ) + std::string( " " ) + std::string(__TIME__);

    /**
     * The software credit.
     */
    inline static const std::string CREDIT =
        "David R. Nadeau (University of California at San Diego (UCSD)), and Yu \"Max\" Qian (J. Craig Venter Institute (JCVI)).";

    /**
     * The software copyright.
     */
    inline static const std::string COPYRIGHT =
        "Copyright (c) Regents of the University of California";

    /**
     * The software license.
     */
    inline static const std::string LICENSE =
        "GNU Lesser General Public License, version 2.1";

private:
    // Configuration ---------------------------------------------------
    /**
     * The default verbosity prefix.
     */
    inline static const std::string DEFAULT_VERBOSE_PREFIX =
        "Gating";

    /**
     * The default number of clusters.
     */
    static const int32_t DEFAULT_NUMBER_OF_SEED_CLUSTERS = 200;

    /**
     * The default number of iterations for cluster convergence.
     */
    static const int32_t DEFAULT_NUMBER_OF_ITERATIONS = 100;

    /**
     * The default random number seed.
     */
    static const unsigned int DEFAULT_RANDOM_NUMBER_SEED = 2;



    // Error messages --------------------------------------------------
    /**
     * The error message 1st line for a programmer error.
     */
    inline static const std::string ERROR_PROGRAMMER =
        "Programmer error.\n";



//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    // Verbosity -------------------------------------------------------
    /**
     * Whether to be verbose and output progress messages.
     *
     * When true, progress messages and commentary are output during
     * operations.
     *
     * @see ::isVerbose()
     * @see ::setVerbose()
     */
    bool verbose;

    /**
     * The verbose message prefix.
     *
     * When verbosity is enabled, all output messages are prefixed with
     * this string. Applications may set the string to be the application
     * name or some other useful prefix.
     *
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     */
    std::string verbosePrefix;



    // Data ------------------------------------------------------------
    /**
     * The gating cache.
     *
     * The cache includes:
     * @li The gate trees to use for gating.
     * @li A source event table.
     * @li State attached to the gate trees to track tree changes.
     * @li State attached to each gate to track gate changes and gating.
     *
     * @see ::getGatingCache()
     */
    std::shared_ptr<FlowGate::Gates::GatingCache> gatingCache;

    /**
     * The number of events.
     *
     * This value is copied out of the gating cache.
     */
    const size_t numberOfEvents;



    // Parameters ------------------------------------------------------
    /**
     * The number of seed clusters.
     *
     * Seed clusters are initial cluster positions, chosen randomly,
     * that are then refined and coallesced through iteration.
     */
    int32_t numberOfSeedClusters = DEFAULT_NUMBER_OF_SEED_CLUSTERS;

    /**
     * The maximum number of clusters.
     */
    int32_t numberOfIterations = DEFAULT_NUMBER_OF_ITERATIONS;

    /**
     * The random number seed.
     */
    unsigned int randomNumberSeed = DEFAULT_RANDOM_NUMBER_SEED;

    /**
     * The source event parameter seeds.
     */
    std::vector<unsigned int> sourceParameterSeeds;



//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    // General constructors --------------------------------------------
    /**
     * Constructs a new gater using the given gated event table.
     *
     * The gated event table should already have been initialized to
     * contain the gate trees to use, and ungated events.
     *
     * @param[in] gatingCache
     *   The gating cache.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is NULL or empty, if
     *   the gate tree includes more than one gate tree, or if a gate
     *   tree includes an unsupported gate type.
     *
     * @see ::getGatingCache()
     */
    Gating( std::shared_ptr<GatingCache> gatingCache )
        : gatingCache( gatingCache ),
        numberOfEvents( (gatingCache == nullptr) ?
            0 : gatingCache->getSourceEventTable( )->getNumberOfEvents( ) )
    {
        this->verbosePrefix = DEFAULT_VERBOSE_PREFIX;

        // Validate.
        if ( gatingCache == nullptr )
            throw std::invalid_argument(
                "Invalid NULL gating cache." );

        auto gateTrees = this->gatingCache->getGateTrees( );

        const size_t numberOfGateTrees = gateTrees->getNumberOfGateTrees( );
        if ( numberOfGateTrees != 1 )
            throw std::invalid_argument(
                "Invalid multiple gate trees, only one tree supported." );

        const auto gates = gateTrees->findDescendentGates( );
        for ( const auto& gate : gates )
        {
            switch ( gate->getGateType( ) )
            {
                case FlowGate::Gates::RECTANGLE_GATE:
                case FlowGate::Gates::POLYGON_GATE:
                    // Supported.
                    break;

                default:
                case FlowGate::Gates::ELLIPSOID_GATE:
                case FlowGate::Gates::QUADRANT_GATE:
                case FlowGate::Gates::BOOLEAN_GATE:
                    // Not supported yet.
                    throw std::invalid_argument(
                        std::string( "Unsupported gate type in gate tree: " ) +
                        FlowGate::Gates::getGateTypeName(
                            gate->getGateType( ) ) );
            }
        }

        // Generate random number seeds.
        this->generateRandomNumberSeeds( );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~Gating( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Verbosity.
//----------------------------------------------------------------------
public:
    /**
     * @name Verbosity
     */
    // @{
    /**
     * Gets the verbose output prefix.
     *
     * When verbosity is enabled, the prefix string starts each line
     * of verbose output. This is often set to the application's name.
     *
     * @return
     *   Returns the verbose message prefix.
     *
     * @see ::isVerbose()
     * @see ::setVerbose()
     * @see ::setVerbosePrefix()
     */
    inline std::string getVerbosePrefix( )
        const noexcept
    {
        return this->verbosePrefix;
    }

    /**
     * Returns true if verbose output is enabled.
     *
     * @return
     *   Returns true if verbose output is enabled.
     *
     * @see ::getVerbosePrefix()
     * @see ::setVerbose()
     * @see ::setVerbosePrefix()
     */
    inline bool isVerbose( )
        const noexcept
    {
        return this->verbose;
    }

    /**
     * Enables or disables verbose output.
     *
     * When true, operations output progress messages and commentary
     * to stderr. This is primarily useful during debugging.
     *
     * @param[in] enable
     *   Set to true to enable verbosity, false To disable.
     *
     * @see ::isVerbose()
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     */
    inline void setVerbose( const bool enable )
        noexcept
    {
        this->verbose = enable;
    }

    /**
     * Sets the verbose output prefix.
     *
     * When verbosity is enabled, the given prefix string starts each line
     * of verbose output. This is often set to the application's name.
     *
     * @param[in] prefix
     *   The verbose message prefix.
     *
     * @see ::isVerbose()
     * @see ::setVerbose()
     * @see ::getVerbosePrefix()
     */
    inline void setVerbosePrefix( const std::string prefix )
        noexcept
    {
        this->verbosePrefix = prefix;
    }
    // @}



//----------------------------------------------------------------------
// Parameters.
//----------------------------------------------------------------------
public:
    /**
     * @name Parameters
     */
    // @{
    /**
     * Returns the gating cache in use.
     *
     * @return
     *   The gating cache.
     */
    inline std::shared_ptr<const GatingCache> getGatingCache( )
        const noexcept
    {
        return this->gatingCache;
    }

    /**
     * Returns the number of seed clusters.
     *
     * Clustering starts by picking random locations for a large number of
     * seed clusters. Iteration then gradually moves those cluster locations
     * to minimize the collective distance between the cluster locations
     * and the nearest events. This may consolidate clusters that become
     * co-located (though the current code does not).
     *
     * This method returns the number of seed cluster locations used.
     *
     * @return
     *   The number of clusters.
     *
     * @see ::setNumberOfSeedClusters()
     */
    inline int32_t getNumberOfSeedClusters( )
        const noexcept
    {
        return this->numberOfSeedClusters;
    }

    /**
     * Returns the number of cluster iterations for convergence.
     *
     * @return
     *   The number of cluster iterations.
     *
     * @see ::setNumberOfIterations()
     */
    inline uint32_t getNumberOfIterations( )
        const noexcept
    {
        return this->numberOfIterations;
    }

    /**
     * Returns the random number seed.
     *
     * @return
     *   The seed.
     *
     * @see ::setRandomNumberSeed()
     */
    inline unsigned int getRandomNumberSeed( )
        const noexcept
    {
        return this->randomNumberSeed;
    }



    /**
     * Sets the number of seed clusters.
     *
     * Setting the value to zero resets to a default.
     *
     * This value should be set before gating. Setting it after gating
     * creates a mismatch between the number of seed clusters set here
     * and the previously computed gated event table.
     *
     * @param[in] number
     *   The number of clusters.
     *
     * @see ::getNumberOfSeedClusters()
     */
    inline void setNumberOfSeedClusters( const int32_t number )
        noexcept
    {
        if ( number <= 0 )
            this->numberOfSeedClusters = DEFAULT_NUMBER_OF_SEED_CLUSTERS;
        else
            this->numberOfSeedClusters = number;
    }

    /**
     * Sets the number of cluster iterations for convergence.
     *
     * Setting the value to zero resets to a default.
     *
     * This value should be set before gating. Setting it after gating
     * creates a mismatch between the number of iterations set here
     * and the previously computed gated event table.
     *
     * @param[in] number
     *   The number of cluster iterations.
     *
     * @see ::getNumberOfIterations()
     */
    inline void setNumberOfIterations( const uint32_t number )
        noexcept
    {
        if ( number == 0 )
            this->numberOfIterations = DEFAULT_NUMBER_OF_ITERATIONS;
        else
            this->numberOfIterations = number;
    }

    /**
     * Sets the random number seed.
     *
     * This value should be set before gating. Setting it after gating
     * creates a mismatch between the random number seed set here
     * and the previously computed gated event table.
     *
     * @param[in] number
     *   The seed.
     *
     * @see ::getRandomNumberSeed()
     */
    inline void setRandomNumberSeed( const unsigned int number )
        noexcept
    {
        this->randomNumberSeed = number;
        this->generateRandomNumberSeeds( );
    }

private:
    /**
     * Generate random numbers for the source event parameters.
     */
    void generateRandomNumberSeeds( )
    {
        // Random numbers are needed to initialize cluster centroids.
        //
        // A random number is needed for each parameter of each cluster for
        // every gate that uses clustering.
        //
        // Several things can change during gate tree editing, and these
        // changes affect the number and order of random numbers used:
        //
        // - Gates can have clustering turned on and off.
        //
        // - Gate clustering can be processed at different times depending
        //   upon prior activity and what's in the cache.
        //
        // - Parameters can be added, removed, and changed for a gate.
        //
        // - The number of clusters can be changed.
        //
        // If we just rolled random numbers as we needed them, then any of
        // the above changes would cause a change to the order or number of
        // random numbers generated for a gate. This would cause changes in
        // the initial cluster positions, and that would cause the clusters
        // to shift a bit.
        //
        // To keep clusters stable, independent of the above, we need to be
        // a bit less random. This is done with several policies:
        //
        // 1. The srand() pseudo-random number generator is initialized here
        //    using the chosen random number seed.
        //
        // 2. A new random number *seed* is generated for each of the source
        //    event table parameters here. This is done for ALL parameters,
        //    independent of which ones are used in gating and clustering.
        //
        // 3. At the start of clustering for a gate, the above per-parameter
        //    seeds are used in an srand() call before generating random
        //    numbers for the cluster centroids for a parameter. Because
        //    these parameter seeds are used identically regardless of the
        //    gate or the order of parameters in a gate, the same centroids
        //    are computed every time.
        //
        // Step 1: Initialize the random number generator.
        srand( this->randomNumberSeed );

        // Step 2: Roll a random number for each source parameter.
        const auto source = this->gatingCache->getSourceEventTable( );
        const uint32_t nTotalParameters = source->getNumberOfParameters( );

        this->sourceParameterSeeds.resize( nTotalParameters, 0 );

        for ( uint32_t i = 0; i < nTotalParameters; ++i )
            this->sourceParameterSeeds[i] = rand( );
    }
    // @}



//----------------------------------------------------------------------
// Gating.
//----------------------------------------------------------------------
public:
    /**
     * @name Gating
     */
    // @{
    /**
     * Performs gating for all gates.
     *
     * Gating is performed, starting with the root gate and continuing
     * downwards through the gate tree. All gates are applied.
     *
     * Upon completion, the results are in the gated event table.
     *
     * @throws std::invalid_argument
     *   Throws an exception if any gate specifies an unknown gating method
     *   or uses an unsupported gate type.
     *
     * @see ::gate()
     * @see ::getGatingCache()
     */
    inline void gateAll( )
    {
        // Get a list of all gates. The list is constructed such that
        // parents are always earlier in the list than children.
        const std::vector<std::shared_ptr<FlowGate::Gates::Gate>> gates =
            this->gatingCache->getGateTrees( )->findDescendentGates( );

        // Loop over the list and gate everything.
        const GateIndex numberOfGates = gates.size( );
        for ( GateIndex i = 0; i < numberOfGates; ++i )
            this->gate( gates[i] );
    }

    /**
     * Performs gating for the specified gate.
     *
     * If gating for the gate has already been applied, this method returns
     * immediately without doing any further work. To re-gate, clear the
     * gate's application flag in the event table first:
     * @code
     * auto gateState = gate->getState( );
     * gate->setApplied( false );
     * @endcode
     *
     * If gating for the gate's parent gate, if any, has not been applied yet,
     * this method throws an exception. Ancestor gates must be computed
     * prior to applying a descendent gate.
     *
     * @param[in] gate
     *   The gate to process.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate's parent has not been
     *   computed yet, or if the gate requires an unknown gating method.
     *
     * @see ::gateAll()
     * @see ::getGatingCache()
     */
    void gate( std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        //
        // Validate.
        // ---------
        // Get the parent gate. This throws an exception if the gate is a
        // nullptr, or if the gate is not in the trees. It returns a nullptr
        // if the gate is a root, and thus it has no parent.
        const auto parent =
            this->gatingCache->getGateTrees( )->findParentGate( gate );

        // If the gate's parent has not been applied, complain. If the parent
        // gate index equals the gate index, there is no parent and the gate
        // is a root.
        if ( parent != nullptr )
        {
            const auto state = parent->getState( );
            if ( state->isApplied( ) == false )
                throw std::invalid_argument(
                    ERROR_PROGRAMMER +
                    "Gate cannot be processed. Parent gating has not been applied yet." );
        }

        // If gating has already been done, then do nothing more.
        const auto state = gate->getState( );
        if ( state->isApplied( ) == true )
        {
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix <<
                    ": Gating skipped for ID " << gate->getId( ) <<
                    ", since gating has already been applied" << std::endl;
            return;
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix <<
                ": Gating ID " << gate->getId( ) << std::endl;


        //
        // Dispatch.
        // ---------
        // Redirect to the proper gating method for the gate.
        switch ( gate->getGatingMethod( ) )
        {
        case FlowGate::Gates::EVENT_VALUE_GATING:
            this->gateByEvent( parent, gate );
            break;

        case FlowGate::Gates::DAFI_CLUSTER_CENTROID_GATING:
            this->gateByCluster( parent, gate );
            break;

        default:
            throw std::invalid_argument(
                "The gate requests an unknown gating method." );
        }

        // Mark the gate as having been applied.
        state->setApplied( true );
    }
    // @}



//----------------------------------------------------------------------
// Gating by event.
//----------------------------------------------------------------------
private:
    /**
     * @name Gating by event
     */
    // @{
    /**
     * Performs gating for the specified gate using event values.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     *
     * @see ::gate()
     * @see ::gateByCluster()
     */
    void gateByEvent(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
    {
        // Dispatch to the proper gate shape.
        switch ( gate->getGateType( ) )
        {
        case FlowGate::Gates::RECTANGLE_GATE:
            this->gateByEventRectangle( parent, gate );
            break;

        case FlowGate::Gates::POLYGON_GATE:
            this->gateByEventPolygon( parent, gate );
            break;

        default:
        case FlowGate::Gates::ELLIPSOID_GATE:
        case FlowGate::Gates::QUADRANT_GATE:
        case FlowGate::Gates::BOOLEAN_GATE:
            throw std::invalid_argument(
                std::string( "Unsupported gate type in gate tree: " ) +
                gate->getGateTypeName( ) );
        }
    }

    /**
     * Performs rectangle gating for the specified gate using event values.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     */
    void gateByEventRectangle(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
        noexcept
    {
        const auto nDimensions = gate->getNumberOfDimensions( );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix <<
                ":   Gate by " <<
                nDimensions << "-D event rectangle" << std::endl;

        switch ( nDimensions )
        {
            case 1:
                gateByEventRectangle1D( parent, gate );
                return;

            case 2:
                gateByEventRectangle2D( parent, gate );
                return;

            default:
                gateByEventRectangleND( parent, gate );
                return;

        }
    }

    /**
     * Performs 1D rectangle gating for the specified gate using event values.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     */
    void gateByEventRectangle1D(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
        noexcept
    {
        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        uint8_t*const inclusion = state->getEventIncludedList( ).data( );

        const uint8_t*const parentInclusion =
            (parent == nullptr) ?
            nullptr :       // No parent.
            parent->getState( )->getEventIncludedList( ).data( );

        const auto rectangle = (RectangleGate*)(gate.get());
        double dxmin = rectangle->getDimensionMinimum( 0 );
        double dxmax = rectangle->getDimensionMaximum( 0 );

        if ( dxmin > dxmax )
        {
            const double swap = dxmin;
            dxmin = dxmax;
            dxmax = swap;
        }

        const size_t nEvents = this->numberOfEvents;

        // Four cases:
        // - Float values, has parent.
        // - Float values, no parent.
        // - Double values, has parent.
        // - Double values, no parent.
        if ( state->areValuesFloats( ) == true )
        {
            // Set up everything as const before the parallel loop.
            const float xmin = (float)dxmin;
            const float xmax = (float)dxmax;
            const float*const xEvent = state->getParameterFloats( 0 ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( parentInclusion[i] == 0 ||
                         xEvent[i] < xmin ||
                         xEvent[i] > xmax )
                        inclusion[i] = 0;
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( xEvent[i] < xmin ||
                         xEvent[i] > xmax )
                        inclusion[i] = 0;
                }
            }
        }
        else
        {
            // Set up everything as const before the parallel loop.
            const double*const xEvent = state->getParameterDoubles( 0 ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( parentInclusion[i] == 0 ||
                         xEvent[i] < dxmin ||
                         xEvent[i] > dxmax )
                        inclusion[i] = 0;
                }
            }
            else
            {
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( xEvent[i] < dxmin ||
                         xEvent[i] > dxmax )
                        inclusion[i] = 0;
                }
            }
        }
    }

    /**
     * Performs 2D rectangle gating for the specified gate using event values.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     */
    void gateByEventRectangle2D(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
        noexcept
    {
        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        uint8_t*const inclusion = state->getEventIncludedList( ).data( );

        const uint8_t*const parentInclusion =
            (parent == nullptr) ?
            nullptr :       // No parent.
            parent->getState( )->getEventIncludedList( ).data( );

        const auto rectangle = (RectangleGate*)(gate.get());
        double dxmin = rectangle->getDimensionMinimum( 0 );
        double dxmax = rectangle->getDimensionMaximum( 0 );
        double dymin = rectangle->getDimensionMinimum( 1 );
        double dymax = rectangle->getDimensionMaximum( 1 );

        if ( dxmin > dxmax )
        {
            const double swap = dxmin;
            dxmin = dxmax;
            dxmax = swap;
        }
        if ( dymin > dymax )
        {
            const double swap = dymin;
            dymin = dymax;
            dymax = swap;
        }

        const size_t nEvents = this->numberOfEvents;

        // Four cases:
        // - Float values, has parent.
        // - Float values, no parent.
        // - Double values, has parent.
        // - Double values, no parent.
        if ( state->areValuesFloats( ) == true )
        {
            // Set up everything as const before the parallel loop.
            const float xmin = (float)dxmin;
            const float xmax = (float)dxmax;
            const float ymin = (float)dymin;
            const float ymax = (float)dymax;

            const float*const xEvent = state->getParameterFloats( 0 ).data( );
            const float*const yEvent = state->getParameterFloats( 1 ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( parentInclusion[i] == 0 ||
                         xEvent[i] < xmin ||
                         xEvent[i] > xmax ||
                         yEvent[i] < ymin ||
                         yEvent[i] > ymax )
                        inclusion[i] = 0;
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( xEvent[i] < xmin ||
                         xEvent[i] > xmax ||
                         yEvent[i] < ymin ||
                         yEvent[i] > ymax )
                        inclusion[i] = 0;
                }
            }
        }
        else
        {
            // Set up everything as const before the parallel loop.
            const double*const xEvent = state->getParameterDoubles( 0 ).data( );
            const double*const yEvent = state->getParameterDoubles( 1 ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( parentInclusion[i] == 0 ||
                         xEvent[i] < dxmin ||
                         xEvent[i] > dxmax ||
                         yEvent[i] < dymin ||
                         yEvent[i] > dymax )
                        inclusion[i] = 0;
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( xEvent[i] < dxmin ||
                         xEvent[i] > dxmax ||
                         yEvent[i] < dymin ||
                         yEvent[i] > dymax )
                        inclusion[i] = 0;
                }
            }
        }
    }

    /**
     * Performs ND rectangle gating for the specified gate using event values.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     */
    void gateByEventRectangleND(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
        noexcept
    {
        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        const GateIndex nDimensions = gate->getNumberOfDimensions( );
        uint8_t*const inclusion = state->getEventIncludedList( ).data( );

        const uint8_t*const parentInclusion =
            (parent == nullptr) ?
            nullptr :       // No parent.
            parent->getState( )->getEventIncludedList( ).data( );

        // We need to loop over all gate dimensions for each event. To do
        // this more efficiently, collect values before hand.
        double*const min = new double[nDimensions];
        double*const max = new double[nDimensions];
        const auto rectangle = (RectangleGate*)(gate.get());
        for ( uint32_t i = 0; i < nDimensions; ++i )
        {
            double tmpmin, tmpmax;
            rectangle->getDimensionMinimumMaximum( i, tmpmin, tmpmax );
            if ( tmpmin > tmpmax )
            {
                const double swap = tmpmin;
                tmpmin = tmpmax;
                tmpmax = swap;
            }

            min[i] = tmpmin;
            max[i] = tmpmax;
        }

        const size_t nEvents = this->numberOfEvents;

        // Four cases:
        // - Float values, has parent.
        // - Float values, no parent.
        // - Double values, has parent.
        // - Double values, no parent.
        if ( state->areValuesFloats( ) == true )
        {
            float** event = new float*[nDimensions];
            for ( uint32_t i = 0; i < nDimensions; ++i )
                event[i] = state->getParameterFloats( i ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( parentInclusion[i] == 0 )
                        inclusion[i] = 0;
                    else
                    {
                        for ( uint32_t j = 0; j < nDimensions; ++j )
                        {
                            if ( event[j][i] < (float)min[j] ||
                                 event[j][i] > (float)max[j] )
                            {
                                inclusion[i] = 0;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    for ( uint32_t j = 0; j < nDimensions; ++j )
                    {
                        if ( event[j][i] < (float)min[j] ||
                             event[j][i] > (float)max[j] )
                        {
                            inclusion[i] = 0;
                            break;
                        }
                    }
                }
            }

            delete[] event;
        }
        else
        {
            double** event = new double*[nDimensions];
            for ( uint32_t i = 0; i < nDimensions; ++i )
                event[i] = state->getParameterDoubles( i ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    if ( parentInclusion[i] == 0 )
                        inclusion[i] = 0;
                    else
                    {
                        for ( uint32_t j = 0; j < nDimensions; ++j )
                        {
                            if ( event[j][i] < min[j] ||
                                 event[j][i] > max[j] )
                            {
                                inclusion[i] = 0;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    for ( uint32_t j = 0; j < nDimensions; ++j )
                    {
                        if ( event[j][i] < min[j] ||
                             event[j][i] > max[j] )
                        {
                            inclusion[i] = 0;
                            break;
                        }
                    }
                }
            }

            delete[] event;
        }

        delete[] min;
        delete[] max;
    }

    /**
     * Performs 2D polygon gating for the specified gate using event values.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     */
    void gateByEventPolygon(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
        noexcept
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix <<
                ":   Gate by 2-D event polygon" << std::endl;

        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        uint8_t*const inclusion = state->getEventIncludedList( ).data( );

        const uint8_t*const parentInclusion =
            (parent == nullptr) ?
            nullptr :       // No parent.
            parent->getState( )->getEventIncludedList( ).data( );

        const auto polygon = (PolygonGate*)(gate.get());
        const auto xCoordinates = polygon->getVerticesX( );
        const auto yCoordinates = polygon->getVerticesY( );
        double bbxmin, bbymin, bbxmax, bbymax;
        polygon->getBoundingBox( bbxmin, bbymin, bbxmax, bbymax );

        const size_t nEvents = this->numberOfEvents;

        // Four cases:
        // - Float values, has parent.
        // - Float values, no parent.
        // - Double values, has parent.
        // - Double values, no parent.
        if ( state->areValuesFloats( ) == true )
        {
            // Set up everything as const before the parallel loop.
            const float xmin = (float)bbxmin;
            const float xmax = (float)bbxmax;
            const float ymin = (float)bbymin;
            const float ymax = (float)bbymax;

            const float*const xEvent = state->getParameterFloats( 0 ).data( );
            const float*const yEvent = state->getParameterFloats( 1 ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    const float x = xEvent[i];
                    const float y = yEvent[i];

                    if ( parentInclusion[i] == 0 ||
                         x < xmin ||
                         x > xmax ||
                         y < ymin ||
                         y > ymax ||
                         isInsidePolygon( x, y, xCoordinates, yCoordinates ) == false )
                        inclusion[i] = 0;
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    const float x = xEvent[i];
                    const float y = yEvent[i];

                    if ( x < xmin ||
                         x > xmax ||
                         y < ymin ||
                         y > ymax ||
                         isInsidePolygon( x, y, xCoordinates, yCoordinates ) == false )
                        inclusion[i] = 0;
                }
            }
        }
        else
        {
            // Set up everything as const before the parallel loop.
            const double xmin = bbxmin;
            const double xmax = bbxmax;
            const double ymin = bbymin;
            const double ymax = bbymax;

            const double*const xEvent = state->getParameterDoubles( 0 ).data( );
            const double*const yEvent = state->getParameterDoubles( 1 ).data( );

            // Loop over all events and mark each as outside (0) or inside (1).
            // Ungated event inclusion flags are always initialize to 1
            // prior to gating, so in the loop only set to 0.
            if ( parentInclusion != nullptr )
            {
                // Gate has a parent. Only gate events included by parent.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    const double x = xEvent[i];
                    const double y = yEvent[i];

                    if ( parentInclusion[i] == 0 ||
                         x < xmin ||
                         x > xmax ||
                         y < ymin ||
                         y > ymax ||
                         isInsidePolygon( x, y, xCoordinates, yCoordinates ) == false )
                        inclusion[i] = 0;
                }
            }
            else
            {
                // Gate has no parent. Gate all events.
#pragma omp parallel for schedule(static)
                for ( size_t i = 0; i < nEvents; ++i )
                {
                    const double x = xEvent[i];
                    const double y = yEvent[i];

                    if ( x < xmin ||
                         x > xmax ||
                         y < ymin ||
                         y > ymax ||
                         isInsidePolygon( x, y, xCoordinates, yCoordinates ) == false )
                        inclusion[i] = 0;
                }
            }
        }
    }

    /**
     * Returns whether a point is left, on, or to the right of a line.
     *
     * @param[in] x0
     *   The X coordinate of the first point of a line.
     * @param[in] y0
     *   The Y coordinate of the first point of a line.
     * @param[in] x1
     *   The X coordinate of the second point of a line.
     * @param[in] y1
     *   The Y coordinate of the second point of a line.
     * @param[in] x2
     *   The X coordinate of the point to test.
     * @param[in] y2
     *   The Y coordinate of the point to test.
     *
     * @return
     *   Returns a value that is > 0 if (x2,y2) is left of the line through
     *   (x0,y0) and (x1,y1). Returns > 0 if (x2,y2) is right of the line.
     *   Returns = 0 if (x2,y2) is on the line.
     *
     * @see http://geomalgorithms.com/a03-_inclusion.html
     */
    template <typename FLOAT>
    FLOAT isLeft(
        const FLOAT x0, const FLOAT y0,
        const FLOAT x1, const FLOAT y1,
        const FLOAT x2, const FLOAT y2 )
        const noexcept
    {
        return ( (x1 - x0) * (y2 - y0) -
                 (x2 - x0) * (y1 - y0) );
    }

    template <typename FLOAT>
    bool isInsidePolygon(
        const FLOAT x, const FLOAT y,
        const std::vector<double> xCoordinates,
        const std::vector<double> yCoordinates )
        const noexcept
    {
        // Use the Winding Number algorithm.
        // See http://geomalgorithms.com/a03-_inclusion.html
        const size_t n = xCoordinates.size( );
        const size_t nm1 = n - 1;

        size_t winding = 0;

        // Loop through all coordinates. An edge goes from coordinate i
        // to (i+1). The coordinate lists DO NOT include a duplicate of
        // the first coordinate.
        for ( size_t i = 0; i < nm1; ++i )
        {
            const FLOAT xi   = (FLOAT) xCoordinates[i];
            const FLOAT yi   = (FLOAT) yCoordinates[i];
            const FLOAT xip1 = (FLOAT) xCoordinates[i+1];
            const FLOAT yip1 = (FLOAT) yCoordinates[i+1];

            if ( yi <= y )
            {
                if ( yip1 > y && isLeft( xi, yi, xip1, yip1, x, y ) > 0 )
                    ++winding;
            }
            else
            {
                if ( yip1 <= y && isLeft( xi, yi, xip1, yip1, x, y ) < 0 )
                    --winding;
            }
        }

        // Check the last edge from the last coordinate to the first.
        const FLOAT xi   = (FLOAT) xCoordinates[nm1];
        const FLOAT yi   = (FLOAT) yCoordinates[nm1];
        const FLOAT xip1 = (FLOAT) xCoordinates[0];
        const FLOAT yip1 = (FLOAT) yCoordinates[0];

        if ( yi <= y )
        {
            if ( yip1 > y && isLeft( xi, yi, xip1, yip1, x, y ) > 0 )
                ++winding;
        }
        else
        {
            if ( yip1 <= y && isLeft( xi, yi, xip1, yip1, x, y ) < 0 )
                --winding;
        }

        return (winding != 0);
    }
    // @}



//----------------------------------------------------------------------
// Gating by cluster.
//----------------------------------------------------------------------
private:
    /**
     * @name Gating by cluster
     */
    // @{
    /**
     * Performs gating for the specified gate using event clusters.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     *
     * @see ::gate()
     * @see ::gateByEvent()
     * @see ::generateRandomNumberSeeds()
     */
    void gateByCluster(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate )
    {
        //
        // Get gate information.
        // ---------------------
        // Get the gate's state and the number of parameters involved in
        // clustering.
        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        const GateIndex nDimensionParameters = gate->getNumberOfDimensions( );
        const GateIndex nAdditionalParameters = gate->getNumberOfAdditionalClusteringParameters( );
        const GateIndex nParameters =
            nDimensionParameters + nAdditionalParameters;

        //
        // Get gate parent inclusion flags.
        // --------------------------------
        // Get the gate's list of parent include true/false flags with one flag
        // per event. If the gate has no parent, set this to NULL.
        const uint8_t*const parentInclusion =
            (parent == nullptr) ?
            nullptr :       // No parent.
            parent->getState( )->getEventIncludedList( ).data( );

        //
        // Prepare for clustering.
        // -----------------------
        // Clustering finds a number of clusters using the gate's parameters.
        // Each cluster has a centroid. Each event is marked with the
        // cluster number that includes the event. A (-1) cluster number
        // indicates the event is not in a cluster (yet).
        const int32_t numberOfClusters = this->numberOfSeedClusters;
        std::vector<int32_t> clusterNumberPerEvent( this->numberOfEvents, -1 );

        // Clustering computes the centroid for each gate parameter.
        // Initialize those to zero, for lack of a better choice.
        std::vector<std::vector<double>> clusterCentersPerParameter(
            nParameters );
        for ( DimensionIndex i = 0; i < nParameters; ++i )
            clusterCentersPerParameter[i].resize( numberOfClusters, 0.0 );

        //
        // Get parameter random number seeds, mins, and ranges.
        // ----------------------------------------------------
        // Copy the seeds, mins, and ranges we need for this gate from the
        // previously computed seeds for all source parameters.
        std::vector<unsigned int> gateSeeds;
        gateSeeds.resize( nParameters, 0 );

        const auto source = this->gatingCache->getSourceEventTable( );

        int index = 0;
        for ( uint32_t i = 0; i < nDimensionParameters; ++i )
        {
            // Get the parameter's name and find its parameter index.
            const auto name = gate->getDimensionParameterName( i );
            const auto sourceIndex = source->getParameterIndex( name );

            // Use the parameter's random number seed.
            gateSeeds[index] = this->sourceParameterSeeds[sourceIndex];

            ++index;
        }

        for ( uint32_t i = 0; i < nAdditionalParameters; ++i )
        {
            // Get the parameter's name and find its parameter index.
            const auto name = gate->getAdditionalClusteringParameterName( i );
            const auto sourceIndex = source->getParameterIndex( name );

            // Use the parameter's random number seed.
            gateSeeds[index] = this->sourceParameterSeeds[sourceIndex];

            ++index;
        }

        std::vector<double> parameterMins;
        parameterMins.resize( nParameters, 0 );

        std::vector<double> parameterRanges;
        parameterRanges.resize( nParameters, 0 );

        for ( DimensionIndex i = 0; i < nParameters; ++i )
        {
            parameterMins[i] = state->getParameterBestMinimum( i );
            parameterRanges[i] =
                state->getParameterBestMaximum( i ) - parameterMins[i];
        }

        //
        // Compute clusters.
        // -----------------
        // Start by allocating a vector with one parameter array pointer
        // per gate parameter. Then cluster using that.
        if ( state->areValuesFloats( ) == true )
        {
            float** parameters = new float*[nParameters];
            for ( DimensionIndex i = 0; i < nParameters; ++i )
                parameters[i] = state->getParameterFloats( i ).data( );

            cluster<float>(
                gateSeeds,
                parameters,
                parameterMins,
                parameterRanges,
                parentInclusion,
                clusterCentersPerParameter,
                clusterNumberPerEvent );

            delete[] parameters;
        }
        else
        {
            double** parameters = new double*[nParameters];
            for ( DimensionIndex i = 0; i < nParameters; ++i )
                parameters[i] = state->getParameterDoubles( i ).data( );

            cluster<double>(
                gateSeeds,
                parameters,
                parameterMins,
                parameterRanges,
                parentInclusion,
                clusterCentersPerParameter,
                clusterNumberPerEvent );

            delete[] parameters;
        }

        //
        // Dispatch.
        // ---------
        // Forward the cluster results a method for each gate type.
        switch ( gate->getGateType( ) )
        {
        case FlowGate::Gates::RECTANGLE_GATE:
            this->gateByClusterRectangle(
                parent,
                gate,
                clusterCentersPerParameter,
                clusterNumberPerEvent );
            break;

        case FlowGate::Gates::POLYGON_GATE:
            this->gateByClusterPolygon(
                parent,
                gate,
                clusterCentersPerParameter,
                clusterNumberPerEvent );
            break;

        default:
        case FlowGate::Gates::ELLIPSOID_GATE:
        case FlowGate::Gates::QUADRANT_GATE:
        case FlowGate::Gates::BOOLEAN_GATE:
            throw std::invalid_argument(
                std::string( "Unsupported gate type in gate tree: " ) +
                gate->getGateTypeName( ) );
        }
    }

    /**
     * Finds clusters using the given parameters.
     *
     * @param[in] gateSeeds
     *   A vector of parameter random number seeds. There is one value for
     *   each parameter.
     * @param[in] parameters
     *   A vector of parameter arrays to use in clustering.
     * @param[in] parameterMins
     *   A vector of parameter minimum values. There is one value for
     *   each parameter.
     * @param[in] parameterRanges
     *   A vector of parameter range values (max minus min). There is one
     *   value for each parameter.
     * @param[in] parentInclusion
     *   A vector of within-the-parent-gate flags. If the gate is a root
     *   gate and has no parent, then this vector will be empty.
     * @param[out] clusterCentersPerParameter
     *   A vector of vectors. The outer vector has one vector per parameter.
     *   The parameter's vector has one value per cluster. The value is set
     *   to the center parameter value for cluster.
     * @param[out] clusterNumberPerEvent
     *   A vector with the number of events in each cluster.
     */
    template <typename FLOAT>
    void cluster(
        const std::vector<unsigned int> gateSeeds,
        const FLOAT*const *const parameters,
        const std::vector<double> parameterMins,
        const std::vector<double> parameterRanges,
        const uint8_t*const parentInclusion,
        std::vector<std::vector<double>>& clusterCentersPerParameter,
        std::vector<int32_t>& clusterNumberPerEvent )
    {
        //
        // Setup.
        // ------
        // Get the numbers of things and pointers to vector data.
        const DimensionIndex numberOfParameters =
            clusterCentersPerParameter.size( );
        const int32_t numberOfClusters = clusterCentersPerParameter[0].size( );

        int32_t*const clusterNumberPerEventData = clusterNumberPerEvent.data( );

        const size_t nEvents = this->numberOfEvents;

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix <<
                ":   Computing clusters with " <<
                numberOfParameters << " parameters" << std::endl;

        // Allocate temp space for the sum of the number of events in each
        // cluster in each event parameter. This array is cleared each time
        // through the loop below.
        double*const sum = new double[numberOfClusters * numberOfParameters];

        // Allocate an array to store the number of events found in
        // each cluster. This is cleared before each use.
        size_t*const eventCountPerCluster = new size_t[numberOfClusters];

        // Initialize the center point for each cluster to a randomly
        // chosen value from the corresponding event table parameter.
        for ( DimensionIndex t = 0; t < numberOfParameters; ++t )
        {
            const FLOAT*const p = parameters[t];
            double*const c = clusterCentersPerParameter[t].data( );

            // Initialize the random generator for this parameter using
            // the given seed.
            srand( gateSeeds[t] );

            // Roll a random number for each cluster and select a
            // corresponding event value as the starting point for
            // the cluster.
            for ( int32_t i = 0; i < numberOfClusters; ++i )
            {
                const double value = p[rand() % nEvents];

                // Normalize the value to a 0..1.0 range.
                c[i] = (value - parameterMins[t]) / parameterRanges[t];
            }
        }

        //
        // Loop.
        // -----
        // Each time through the loop below, compute the distance
        // between each parameter value of each event and the current
        // cluster center point for that parameter. This repeats
        // for multiple iterations, gradually moving the center
        // points to reduce the distances.
        for ( int32_t iteration = 0;
              iteration < this->numberOfIterations;
              ++iteration )
        {
            // Compute Euclidean distance.
            //
            // For each event, compute the distance between the event and
            // the center.
            //
            // To enable this loop to work well in parallel across
            // multiple threads, results from the loop are stored in
            // the shortest population ID, with one entry per event.
            // Each thread only changes array entries it is handling,
            // so there are no thread collisions.
#pragma omp parallel for schedule(static)
            for ( size_t i = 0; i < nEvents; ++i )
            {
                // If there is a parent gate, and the event was not included
                // within it, then ignore the event for clustering.
                if ( parentInclusion != nullptr &&
                    parentInclusion[i] == 0 )
                {
                    // There is a parent and the event was excluded by it.
                    clusterNumberPerEventData[i] = (-1);
                    continue;
                }

                double distanceSumPerCluster[numberOfClusters];
                std::memset( distanceSumPerCluster, 0, numberOfClusters * sizeof(double) );

                // Loop over all parameters and sum the distances squared
                // between each parameter and the cluster center.
                for ( DimensionIndex t = 0 ; t < numberOfParameters; ++t )
                {
                    const FLOAT*const p = parameters[t];
                    double*const c = clusterCentersPerParameter[t].data( );

                    for ( int32_t j = 0; j < numberOfClusters; ++j )
                    {
                        // Get and normalize the value to a 0..1.0 range.
                        const double value =
                            (p[i] - parameterMins[t]) / parameterRanges[t];

                        // Compute the difference (distance) along this
                        // parameter's axis to the cluster centroid. The
                        // centroid is already in a normalized 0..1.0 range.
                        const double diff = c[j] - value;

                        // And add the square to the total distance.
                        distanceSumPerCluster[j] += diff * diff;
                    }
                }

                // Assign the event to the cluster for which it has
                // the shortest distance to the cluster's center.
                double shortestDistance = std::numeric_limits<double>::max( );
                for ( int32_t j = 0; j < numberOfClusters; ++j )
                {
                    if ( distanceSumPerCluster[j] < shortestDistance )
                    {
                        shortestDistance = distanceSumPerCluster[j];
                        clusterNumberPerEventData[i] = j;
                    }
                }
            }

            // Clear the array counting the number of events in each cluster.
            std::memset( eventCountPerCluster, 0, numberOfClusters * sizeof(size_t) );

            // Clear the array containing the sum of the event distances
            // for all parameters of all clusters.
            std::memset( sum, 0, numberOfClusters * numberOfParameters * sizeof(double) );

            // Consolidate the parallel loop results to count the number of
            // events assigned to each cluster.
            for ( size_t i = 0; i < nEvents; ++i )
            {
                // Get the population ID found to be closest to this event
                // during the parallel loop above.
                const int32_t id = clusterNumberPerEventData[i];
                if ( id != -1 )
                {
                    // Increment the number of events favoring that cluster.
                    ++eventCountPerCluster[id];

                    // For each parameter in the event, add the parameter's
                    // value to a sum associated with the population.
                    for ( DimensionIndex t = 0; t < numberOfParameters; ++t )
                    {
                        // Normalize the value to a 0..1.0 range.
                        const double value =
                            (parameters[t][i] - parameterMins[t]) /
                            parameterRanges[t];
                        sum[id * numberOfParameters + t] += value;
                    }
                }
            }

            // Recompute the centers.
#pragma omp parallel for schedule(static)
            for ( int32_t j = 0; j < numberOfClusters; ++j )
            {
                if ( eventCountPerCluster[j] > 0 )
                {
                    for ( DimensionIndex t = 0; t < numberOfParameters; ++t )
                    {
                        clusterCentersPerParameter[t][j] =
                            sum[j * numberOfParameters + t] /
                            (double)eventCountPerCluster[j];
                    }
                }
            }
        }

        //
        // Un-normalize cluster centroids.
        // -------------------------------
        // Above, event values and cluster centroids were normalized to a
        // 0..1.0 range so that calculated distances were in the same
        // coordinate space. Now un-normalize the centroids back to original
        // parameter ranges.
        for ( int32_t j = 0; j < numberOfClusters; ++j )
        {
            for ( DimensionIndex t = 0; t < numberOfParameters; ++t )
            {
                clusterCentersPerParameter[t][j] =
                    clusterCentersPerParameter[t][j] * parameterRanges[t] +
                    parameterMins[t];
            }
        }


        //
        // Clean up.
        // ---------
        delete[] eventCountPerCluster;
        delete[] sum;
    }

    /**
     * Performs rectangle gating for the specified gate using clustering.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     * @param[in] clusterCentersPerParameter
     *   A vector with one vector per parameter. The parameter's vector has
     *   one center value for each cluster.
     * @param[in] clusterNumberPerEvent
     *   A vector with one cluster number assigned to each event.
     */
    void gateByClusterRectangle(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate,
        std::vector<std::vector<double>>& clusterCentersPerParameter,
        std::vector<int32_t>& clusterNumberPerEvent )
        noexcept
    {
        const auto nDimensions = gate->getNumberOfDimensions( );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix <<
                ":   Gate by " <<
                nDimensions << "-D cluster rectangle" << std::endl;

        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        uint8_t*const inclusion = state->getEventIncludedList( ).data( );
        const int32_t numberOfClusters = clusterCentersPerParameter[0].size( );

        const auto rectangle = (RectangleGate*)(gate.get());
        double*const min = new double[nDimensions];
        double*const max = new double[nDimensions];
        for ( DimensionIndex t = 0; t < nDimensions; ++t )
        {
            double tmpmin, tmpmax;
            rectangle->getDimensionMinimumMaximum( t, tmpmin, tmpmax );
            if ( tmpmin > tmpmax )
            {
                const double swap = tmpmin;
                tmpmin = tmpmax;
                tmpmax = swap;
            }

            min[t] = tmpmin;
            max[t] = tmpmax;
        }

        const size_t nEvents = this->numberOfEvents;

        //
        // Gate the clusters.
        // ------------------
        // For each cluster, get its cluster center and check if the cluster
        // center is within the gate's min/max.
        //
        // Initialize per-cluster inclusion to 1 (true). In the loop, set
        // to false if the cluster center is not inside the gate.
        uint8_t*const clusterInclusion = new uint8_t[nEvents];
        std::memset( clusterInclusion, 1, nEvents );

#pragma omp parallel for schedule(static)
        for ( int32_t i = 0; i < numberOfClusters; ++i )
        {
            for ( DimensionIndex t = 0; t < nDimensions; ++t )
            {
                const double value = clusterCentersPerParameter[t][i];
                if ( value < min[t] || value > max[t] )
                {
                    // Cluster is NOT inside this gate's rectangle.
                    clusterInclusion[i] = 0;
                    break;
                }
            }
        }

        delete[] min;
        delete[] max;

        //
        // Apply gate.
        // -----------
        // For each event, if the event was not included in the parent
        // gate, or if the event was not included in a cluster that was
        // included in this gate, then mark the event as not included.
        //
        // The cluster number for an event is (-1) if it was excluded by
        // the parent.
        const int32_t*const clusterNumber = clusterNumberPerEvent.data( );

#pragma omp parallel for schedule(static)
        for ( size_t i = 0; i < nEvents; ++i )
        {
            const int32_t num = clusterNumber[i];
            if ( num >= 0 )
                inclusion[i] = clusterInclusion[num];
            else
                inclusion[i] = 0;
        }

        delete[] clusterInclusion;
    }

    /**
     * Performs polygon gating for the specified gate using clustering.
     *
     * @param[in] parent
     *   The parent gate. A nullptr indicates there is no parent because
     *   the given gate is a root.
     * @param[in] gate
     *   The gate to process.
     * @param[in] clusterCentersPerParameter
     *   A vector with one vector per parameter. The parameter's vector has
     *   one center value for each cluster.
     * @param[in] clusterNumberPerEvent
     *   A vector with one cluster number assigned to each event.
     */
    void gateByClusterPolygon(
        std::shared_ptr<const Gate> parent,
        std::shared_ptr<Gate> gate,
        std::vector<std::vector<double>>& clusterCentersPerParameter,
        std::vector<int32_t>& clusterNumberPerEvent )
        noexcept
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix <<
                ":   Gate by 2-D cluster polygon" << std::endl;

        auto state = std::dynamic_pointer_cast<FlowGate::Gates::GateState>( gate->getState( ) );
        uint8_t*const inclusion = state->getEventIncludedList( ).data( );
        const int32_t numberOfClusters = clusterCentersPerParameter[0].size( );

        const auto polygon = (PolygonGate*)(gate.get());
        const auto xCoordinates = polygon->getVerticesX( );
        const auto yCoordinates = polygon->getVerticesY( );
        double bbxmin, bbymin, bbxmax, bbymax;
        polygon->getBoundingBox( bbxmin, bbymin, bbxmax, bbymax );

        const size_t nEvents = this->numberOfEvents;

        //
        // Gate the clusters.
        // ------------------
        // For each cluster, get its cluster center and check if the cluster
        // center is within the gate's polygon.
        //
        // Initialize per-cluster inclusion to 1 (true). In the loop, set
        // to false if the cluster center is not inside the gate.
        uint8_t*const clusterInclusion = new uint8_t[nEvents];
        std::memset( clusterInclusion, 1, nEvents );

        // Set up everything as const before the parallel loop.
        const double xmin = bbxmin;
        const double xmax = bbxmax;
        const double ymin = bbymin;
        const double ymax = bbymax;

#pragma omp parallel for schedule(static)
        for ( int32_t i = 0; i < numberOfClusters; ++i )
        {
            const double x = clusterCentersPerParameter[0][i];
            const double y = clusterCentersPerParameter[1][i];

            if ( x < xmin ||
                 x > xmax ||
                 y < ymin ||
                 y > ymax ||
                 isInsidePolygon( x, y, xCoordinates, yCoordinates ) == false )
                clusterInclusion[i] = 0;
        }

        //
        // Apply gate.
        // -----------
        // For each event, if the event was not included in the parent
        // gate, or if the event was not included in a cluster that was
        // included in this gate, then mark the event as not included.
        //
        // The cluster number for an event is (-1) if it was excluded by
        // the parent.
        const int32_t*const clusterNumber = clusterNumberPerEvent.data( );

#pragma omp parallel for schedule(static)
        for ( size_t i = 0; i < nEvents; ++i )
        {
            const int32_t num = clusterNumber[i];
            if ( num >= 0 )
                inclusion[i] = clusterInclusion[num];
            else
                inclusion[i] = 0;
        }

        delete[] clusterInclusion;
    }
    // @}
};

} // End Gates namespace
} // End FlowGate namespace
