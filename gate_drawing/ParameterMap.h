/**
 * @file
 * Manages a map of parameters from short names to more information.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "GateTrees.h"
 * @li FlowGate "EventTable.h"
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
#include <cstdint>      // size_t, uint32_t, uint16_t, uint8_t, ...

// Standard C++ libraries.
#include <map>          // std::map
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector

#include "GateTrees.h"  // Transforms
#include "EventTable.h" // Event tables





namespace FlowGate {
namespace Events {





//----------------------------------------------------------------------
//
// Parameter map.
//
//----------------------------------------------------------------------
/**
 * Manages a parameter map.
 *
 * A parameter map provides a mapping from the unique short parameter names
 * found in event tables and event table file formats (such as FCS) and
 * longer user-friendly names desirable in user interfaces. The map also
 * provides descriptive information about each parameter, including an
 * optional short description, an optional default transform, and a flag
 * indicating if the parameter is normally useful as a gate axis during
 * gating.
 *
 *
 * <B>Creating a parameter map</B><BR>
 * Constructors create an empty parameter map, or one initialized from
 * another parameter map or an event table. Parameters may be added and
 * changed after construction.
 *
 * @code
 * // Create an empty parameter map:
 * auto map = new ParameterMap( );
 *
 * // Create a parameter map copy of another parameter map:
 * auto mapCopy = new ParameterMap( map );
 *
 * // Create a parameter map initialized with parameters from a event table:
 * auto newMap = new ParameterMap( eventTable );
 * @endcode
 *
 *
 * <B>Getting parameter map attributes</B><BR>
 * The number of parameters and and their values may be queried:
 *
 * @code
 * auto parameterNames = map->getParameterNames( );
 * auto numberOfParameters = map->getNumberOfParameters( );
 * for ( size_t i = 0; i < numberOfParameters; ++i )
 * {
 *   const auto shortName = parameterNames[i];
 *   const auto longName = map->getParameterLongName( shortName );
 *   ...
 * }
 * @endcode
 *
 *
 * <B>Looking up parameter mapping</B><BR>
 * Given a short parameter name, the parameter's long name and other
 * attributes may be queried:
 *
 * @code
 * const auto longName = map->findParameterLongName( shortName );
 * @endcode
 *
 *
 * <B>Updating parameter attributes</B><BR>
 * The attributes of a parameter, such as its long name, may be set:
 *
 * @code
 * map->setParameterName( shortName, newLongName );
 * @endcode
 *
 *
 * <B>Adding and removing parameters</B><BR>
 * Parameters may be added and removed.
 *
 * @code
 * // Add new parameter.
 * map->addParameter( shortName, longName );
 *
 * // Remove parameter by name.
 * map->removeParameter( shortName );
 * @endcode
 *
 *
 * <B>Multi-threading</B><BR>
 * All class methods are presumed to be executed on a single thread,
 * or in a thread-safe manner. No thread locking is used in this class.
 */
class ParameterMap final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate Parameter Map";

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
        "David R. Nadeau (University of California at San Diego (UCSD))";

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
     *
     * @see getVerbosePrefix()
     */
    inline static const std::string DEFAULT_VERBOSE_PREFIX =
        "ParameterMap";



    // Error messages --------------------------------------------------
    /**
     * The error message 1st line for a programmer error.
     */
    inline static const std::string ERROR_PROGRAMMER =
        "Programmer error.\n";



//----------------------------------------------------------------------
// Inner classes.
//----------------------------------------------------------------------
protected:
    /**
     * Holds attributes of a single parameter.
     *
     * Attributes include:
     * - A non-empty short name.
     * - A long name, which may be empty.
     * - A boolean flag indicating if the parameter should be visible
     *   for gating.
     * - A default transform, which may be NULL.
     *
     * Parameters are maintained by the outer class in a lookup map
     * based on the short name, which must be non-empty and unique.
     */
    class Parameter final
    {
    // Fields ----------------------------------------------------------
    protected:
        /**
         * The parameter's short name.
         *
         * The name may not be empty. It must be unique among all
         * parameters in a map.
         *
         * @see ::getName()
         * @see ::setName()
         */
        std::string shortName;

        /**
         * The parameter's long name.
         *
         * The name may be empty.
         *
         * @see ::getLongName()
         * @see ::setLongName()
         */
        std::string longName;

        /**
         * The parameter's description.
         *
         * The description may be empty.
         *
         * @see ::getDescription()
         * @see ::setDescription()
         */
        std::string description;

        /**
         * The parameter's visibility flag for use in gating.
         *
         * When TRUE (the default), the parameter may be shown in
         * user interface menus of available parameters to use for
         * gating axes. When FALSE, the parameter may be hidden by
         * a user interface.
         *
         * @see ::getVisibleForGating()
         * @see ::setVisibleForGating()
         */
        bool visibleForGating = true;

        /**
         * The parameter's default transform.
         *
         * When NULL (the default), there is no default transform.
         *
         * @see ::getDefaultTransform()
         * @see ::setDefaultTransform()
         */
        std::shared_ptr<FlowGate::Gates::Transform> defaultTransform;


    // Constructors / Destructors --------------------------------------
    public:
        /**
         * Creates a new parameter with the given values.
         *
         * @param[in] shortName
         *   The short name for the new parameter map entry. The name cannot
         *   be empty.
         * @param[in] longName
         *   (optional, default = empty string) The long name for the parameter.
         *   The string may be empty.
         * @param[in] visibleForGating
         *   (optional, default = true) The gating visibility flag for the
         *   parameter.
         * @param[in] defaultTransform
         *   (optional, default = nullptr) The default transform for the
         *   parameter. A nullptr indicates that there is no default transform.
         *
         * @throw std::out_of_range
         *   Throws an exception if the short name is empty.
         */
        Parameter(
            const std::string shortName,
            const std::string longName = "",
            const std::string description = "",
            const bool visibleForGating = true,
            const std::shared_ptr<FlowGate::Gates::Transform> transform = nullptr )
        {
            if ( shortName.empty( ) == true )
                throw std::out_of_range(
                    ERROR_PROGRAMMER +
                    "Invalid empty short parameter name." );

            this->shortName        = shortName;
            this->longName         = longName;
            this->description      = description;
            this->visibleForGating = visibleForGating;
            this->defaultTransform = transform;
        }

        /**
         * Constructs a new parameter as a copy of the given parameter.
         *
         * @param[in] parameter
         *   The parameter to copy.
         */
        Parameter( const Parameter& parameter )
            noexcept
        {
            this->shortName        = parameter.shortName;
            this->longName         = parameter.longName;
            this->description      = parameter.description;
            this->visibleForGating = parameter.visibleForGating;
            this->defaultTransform = parameter.defaultTransform;
        }

        /**
         * Constructs a new parameter as a copy of the given parameter.
         *
         * @param[in] parameter
         *   The parameter to copy.
         *
         * @throw std::invalid_argument
         *   Throws an exception if the parameter is NULL.
         */
        Parameter( const Parameter*const parameter )
        {
            if ( parameter == nullptr )
                throw new std::invalid_argument(
                    ERROR_PROGRAMMER +
                    "Invalid NULL parameter.");

            this->shortName        = parameter->shortName;
            this->longName         = parameter->longName;
            this->description      = parameter->description;
            this->visibleForGating = parameter->visibleForGating;
            this->defaultTransform = parameter->defaultTransform;
        }

        /**
         * Constructs a new parameter as a copy of the given parameter.
         *
         * @param[in] parameter
         *   The parameter to copy.
         *
         * @throw std::invalid_argument
         *   Throws an exception if the parameter is NULL.
         */
        Parameter( const std::shared_ptr<Parameter> parameter )
        {
            if ( parameter == nullptr )
                throw new std::invalid_argument(
                    ERROR_PROGRAMMER +
                    "Invalid NULL parameter.");

            this->shortName        = parameter->shortName;
            this->longName         = parameter->longName;
            this->description      = parameter->description;
            this->visibleForGating = parameter->visibleForGating;
            this->defaultTransform = parameter->defaultTransform;
        }

        /**
         * Destroys the object.
         */
        ~Parameter( )
        {
            // Do nothing.
        }


    // Attributes ------------------------------------------------------
        /**
         * Returns the parameter's default transform.
         *
         * @return
         *   Returns the parameter's transform or NULL if there isn't one.
         *
         * @see ::setDefaultTransform()
         */
        std::shared_ptr<FlowGate::Gates::Transform> getDefaultTransform( )
            const noexcept
        {
            return this->defaultTransform;
        }

        /**
         * Returns the parameter's description.
         *
         * @return
         *   Returns the parameter's description, or an empty string.
         *
         * @see ::setDescription()
         */
        const std::string& getDescription( )
            const noexcept
        {
            return this->description;
        }

        /**
         * Returns the parameter's long name.
         *
         * @return
         *   Returns the parameter's long name, or an empty string.
         *
         * @see ::setLongName()
         */
        const std::string& getLongName( )
            const noexcept
        {
            return this->longName;
        }

        /**
         * Returns the parameter's short name.
         *
         * @return
         *   Returns the parameter's short name. The name cannot be empty.
         *
         * @see ::setName()
         */
        const std::string& getName( )
            const noexcept
        {
            return this->shortName;
        }

        /**
         * Returns the parameter's gating visibility flag.
         *
         * @return
         *   Returns TRUE or FALSE for the parameter's visibility flag.
         *
         * @see ::setVisibleForGating()
         */
        bool getVisibleForGating( )
            const noexcept
        {
            return this->visibleForGating;
        }


        /**
         * Sets the parameter's default transform.
         *
         * @param[in] transform
         *   The parameter's default transform object, or a NULL if there
         *   isn't one.
         *
         * @see ::getDefaultTransform()
         */
        void setDefaultTransform(
            const std::shared_ptr<FlowGate::Gates::Transform> transform )
            noexcept
        {
            this->defaultTransform = transform;
        }

        /**
         * Sets the parameter's description.
         *
         * @param[in] description
         *   The parameter's description, or an empty string if there isn't one.
         *
         * @see ::getDescription()
         */
        void setDescription( const std::string description )
            noexcept
        {
            this->description = description;
        }

        /**
         * Sets the parameter's long name.
         *
         * @param[in] longName
         *   The parameter's long name, or an empty string if there isn't one.
         *
         * @see ::getLongName()
         */
        void setLongName( const std::string longName )
            noexcept
        {
            this->longName = longName;
        }

        /**
         * Sets the parameter's short name.
         *
         * @param[in] longName
         *   The parameter's short name.
         *
         * @throw std::out_of_range
         *   Throws an exception if the short name is empty.
         *
         * @see ::getName()
         */
        void setName( const std::string shortName )
        {
            if ( shortName.empty( ) == true )
                throw std::out_of_range(
                    ERROR_PROGRAMMER +
                    "Invalid empty short parameter name." );

            this->shortName = shortName;
        }

        /**
         * Sets the parameter's gating visibility flag.
         *
         * @param[in] visibleForGating
         *   TRUE if the parameter should be used in gating, and FALSE
         *   otherwise.
         *
         * @see ::getVisibleForGating()
         */
        void setVisibleForGating( const bool visibleForGating )
            noexcept
        {
            this->visibleForGating = visibleForGating;
        }
    };



//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
protected:
    // Verbosity -------------------------------------------------------
    /**
     * Whether to be verbose and output progress messages.
     *
     * When true, progress messages and commentary are output during
     * operations.
     *
     * @see ::isVerbose()
     * @see ::setVerbose()
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     */
    bool verbose = false;

    /**
     * The verbose message prefix.
     *
     * When verbosity is enabled, all output messages are prefixed with
     * this string. Applications may set the string to be the application
     * name or some other useful prefix.
     *
     * @see ::isVerbose()
     * @see ::setVerbose()
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     */
    std::string verbosePrefix;



    // Parameter data --------------------------------------------------
    /**
     * A map of parameter short names to parameter objects.
     *
     * This map is used to look up parameter metadata based upon a
     * unique short parameter name.
     *
     * @see ::addParameter()
     * @see ::copy()
     * @see ::setParameter(()
     * @see ::removeParameter()
     */
    std::map<std::string,std::shared_ptr<Parameter>> map;



    // Generic information ---------------------------------------------
    /**
     * The optional name of the parameter map.
     *
     * The name defaults to the empty string.
     *
     * @see ::getName()
     */
    std::string name;

    /**
     * The optional description of the parameter map.
     *
     * @see ::getDescription()
     */
    std::string description;

    /**
     * The optional file name containing the parameter map.
     *
     * The file name may be set when the parameter map is loaded from a file,
     * if any.
     *
     * @see ::getFileName( )
     */
    std::string fileName;

    /**
     * The optional name of the software used to create the parameter map.
     *
     * @see ::getCreatorSoftwareName( )
     */
    std::string creatorSoftwareName;



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
     * Constructs a new empty parameter map.
     */
    ParameterMap( )
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->creatorSoftwareName = NAME + " version " + VERSION;
    }



    /**
     * Constructs a new parameter map initialized by copying another map.
     *
     * @param[in] parameterMap
     *   The parameter map to copy.
     *
     * @see ::copy()
     */
    ParameterMap( const ParameterMap& parameterMap )
        noexcept
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( parameterMap );
        if ( this->creatorSoftwareName.empty( ) == true )
            this->creatorSoftwareName = NAME + " version " + VERSION;
    }

    /**
     * Constructs a new parameter map initialized by copying another map.
     *
     * @param[in] parameterMap
     *   The parameter map to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the parameter map is a nullptr.
     *
     * @see ::copy()
     */
    ParameterMap( const ParameterMap*const parameterMap )
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( parameterMap );
        if ( this->creatorSoftwareName.empty( ) == true )
            this->creatorSoftwareName = NAME + " version " + VERSION;
    }

    /**
     * Constructs a new parameter map initialized by copying another map.
     *
     * @param[in] parameterMap
     *   The parameter map to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the parameter map is a nullptr.
     *
     * @see ::copy()
     */
    ParameterMap( std::shared_ptr<const ParameterMap> parameterMap )
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( parameterMap );
        if ( this->creatorSoftwareName.empty( ) == true )
            this->creatorSoftwareName = NAME + " version " + VERSION;
    }



    /**
     * Constructs a new parameter map from parameters in an event table.
     *
     * @param[in] eventTable
     *   The event table providing short and long parameter names.
     *
     * @see ::copy()
     */
    ParameterMap( const FlowGate::Events::EventTableInterface& eventTable )
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( eventTable );
        this->creatorSoftwareName = NAME + " version " + VERSION;
    }

    /**
     * Constructs a new parameter map from parameters in an event table.
     *
     * @param[in] eventTable
     *   The event table providing short and long parameter names.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a NULL pointer.
     *
     * @see ::copy()
     */
    ParameterMap(
        const FlowGate::Events::EventTableInterface*const eventTable )
    {
        if ( eventTable == nullptr )
            throw new std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table.");

        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( eventTable );
        this->creatorSoftwareName = NAME + " version " + VERSION;
    }

    /**
     * Constructs a new parameter map from parameters in an event table.
     *
     * @param[in] eventTable
     *   The event table providing short and long parameter names.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a NULL pointer.
     *
     * @see ::copy()
     */
    ParameterMap(
        std::shared_ptr<const FlowGate::Events::EventTableInterface> eventTable )
    {
        if ( eventTable == nullptr )
            throw new std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table.");

        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( eventTable );
        this->creatorSoftwareName = NAME + " version " + VERSION;
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~ParameterMap( )
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
    std::string getVerbosePrefix( )
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
     * @see ::setVerbose()
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     */
    bool isVerbose( )
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
    void setVerbose( const bool enable )
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
    void setVerbosePrefix( const std::string prefix )
        noexcept
    {
        this->verbosePrefix = prefix;
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the optional software name used to create the parameter map.
     *
     * When a parameter map is loaded from a file, this is initialized to the
     * explicit or inferred software package name used to create that file.
     * Otherwise, this defaults to the name of this FlowGate software.
     *
     * @return
     *   Returns the software name, or an empty string if no name
     *   has been set.
     *
     * @see ::setCreatorSoftwareName()
     */
    inline const std::string& getCreatorSoftwareName( )
        const noexcept
    {
        return this->creatorSoftwareName;
    }

    /**
     * Returns the optional description for the parameter map.
     *
     * The description defaults to the empty string.
     *
     * @return
     *   Returns the description, or an empty string if no description
     *   has been set.
     *
     * @see ::setDescription()
     */
    inline const std::string& getDescription( )
        const noexcept
    {
        return this->description;
    }

    /**
     * Returns the optional file name for the parameter map.
     *
     * The name defaults to the file name, if any, that contained the
     * parameter map.
     *
     * @return
     *   Returns the file name, or an empty string if no file name has
     *   been set.
     *
     * @see ::getName()
     * @see ::setFileName()
     */
    inline const std::string& getFileName( )
        const noexcept
    {
        return this->fileName;
    }

    /**
     * Returns the optional name for the parameter map.
     *
     * The name defaults to the file name, if any, that contained the
     * parameter map.
     *
     * @return
     *   Returns the name, or an empty string if no name has been set.
     *
     * @see ::getFileName()
     * @see ::setName()
     */
    inline const std::string& getName( )
        const noexcept
    {
        return this->name;
    }



    /**
     * Sets the optional software name used to create the parameter map.
     *
     * When a parameter map is loaded from a file, this may be set to the
     * explicit or inferred software package name used to create that file.
     * Otherwise, this defaults to the name of this FlowGate software.
     *
     * @param[in] name
     *   The software name. An empty string clears the name.
     *
     * @see ::getCreatorSoftwareName()
     */
    inline void setCreatorSoftwareName( const std::string& name )
        noexcept
    {
        if ( this->creatorSoftwareName == name )
            return;

        this->creatorSoftwareName = name;
    }

    /**
     * Sets the optional file name for the parameter map.
     *
     * @param[in] fileName
     *   The file name.
     *
     * @see ::getFileName()
     * @see ::setName()
     */
    inline void setFileName( const std::string& fileName )
        noexcept
    {
        if ( this->fileName == fileName )
            return;

        this->fileName = fileName;
    }

    /**
     * Sets the optional description for the parameter map.
     *
     * @param[in] description
     *   The description. An empty string clears the description.
     *
     * @see ::getDescription()
     */
    inline void setDescription( const std::string& description )
        noexcept
    {
        if ( this->description == description )
            return;

        this->description = description;
    }

    /**
     * Sets the optional name for the parameter map.
     *
     * @param[in] name
     *   The name.
     *
     * @see ::getName()
     * @see ::setFileName()
     */
    inline void setName( const std::string& name )
        noexcept
    {
        if ( this->name == name )
            return;

        this->name = name;
    }
    // @}



//----------------------------------------------------------------------
// Copy.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy
     */
    // @{
    /**
     * Copies entries from the given parameter map into this map.
     *
     * If a parameter in the given map is already in this map, it is
     * overwritten with values from the given map.
     *
     * @param[in] map
     *   The parameter map to copy.
     *
     * @see ::addParameter()
     */
    void copy( const ParameterMap& map )
        noexcept
    {
        for ( auto it = map.map.cbegin( ); it != map.map.cend( ); ++it )
        {
            auto mapp = (*it).second;
            auto p = this->findParameter( mapp->getName( ) );
            if ( p == nullptr )
            {
                this->setParameter(
                    mapp->getName( ),
                    mapp->getLongName( ),
                    mapp->getDescription( ),
                    mapp->getVisibleForGating( ),
                    mapp->getDefaultTransform( ));
            }
        }
    }

    /**
     * Copies entries from the given parameter map into this map.
     *
     * If a parameter in the given map is already in this map, it is
     * overwritten with values from the given map.
     *
     * @param[in] map
     *   The parameter map to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the map is a NULL pointer.
     *
     * @see ::addParameter()
     */
    void copy( const ParameterMap*const map )
    {
        if ( map == nullptr )
            throw new std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map.");

        for ( auto it = map->map.cbegin( ); it != map->map.cend( ); ++it )
        {
            auto mapp = (*it).second;
            auto p = this->findParameter( mapp->getName( ) );
            if ( p == nullptr )
            {
                this->setParameter(
                    mapp->getName( ),
                    mapp->getLongName( ),
                    mapp->getDescription( ),
                    mapp->getVisibleForGating( ),
                    mapp->getDefaultTransform( ));
            }
        }
    }

    /**
     * Copies entries from the given parameter map into this map.
     *
     * If a parameter in the given map is already in this map, it is
     * overwritten with values from the given map.
     *
     * @param[in] map
     *   The parameter map to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the map is a NULL pointer.
     *
     * @see ::addParameter()
     */
    void copy( std::shared_ptr<const ParameterMap> map )
    {
        if ( map == nullptr )
            throw new std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map.");

        for ( auto it = map->map.cbegin( ); it != map->map.cend( ); ++it )
        {
            auto mapp = (*it).second;
            auto p = this->findParameter( mapp->getName( ) );
            if ( p == nullptr )
            {
                this->setParameter(
                    mapp->getName( ),
                    mapp->getLongName( ),
                    mapp->getDescription( ),
                    mapp->getVisibleForGating( ),
                    mapp->getDefaultTransform( ));
            }
        }
    }

    /**
     * Copies parameters from an event table into this map.
     *
     * If a parameter in the given event table is already in this map, and
     * the event table's parameter has a non-empty long name, the long name
     * for the entry already in this map is overwritten with the event
     * table's long name.
     *
     * If a parameter is not already in this map, an entry is added with
     * the short and long names from the event table. The entry's
     * visibility and transform values are initialized to defaults.
     *
     * @param[in] eventTable
     *   The event table providing short and long parameter names.
     *
     * @see ::addParameter()
     */
    void copy( const FlowGate::Events::EventTableInterface& eventTable )
    {
        const size_t n = eventTable.getNumberOfParameters( );
        for ( size_t i = 0; i < n; ++i )
        {
            const std::string shortName = eventTable.getParameterName( i );
            const std::string longName = eventTable.getParameterLongName( i );
            auto p = this->findParameter( shortName );
            if ( p == nullptr )
                this->addParameter( shortName, longName );
            else
                this->setParameterLongName( shortName, longName );
        }
    }

    /**
     * Copies parameters from an event table into this map.
     *
     * If a parameter in the given event table is already in this map, and
     * the event table's parameter has a non-empty long name, the long name
     * for the entry already in this map is overwritten with the event
     * table's long name.
     *
     * If a parameter is not already in this map, an entry is added with
     * the short and long names from the event table. The entry's
     * visibility and transform values are initialized to defaults.
     *
     * @param[in] eventTable
     *   The event table providing short and long parameter names.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a NULL pointer.
     *
     * @see ::addParameter()
     */
    void copy(
        const FlowGate::Events::EventTableInterface* eventTable )
    {
        if ( eventTable == nullptr )
            throw new std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table.");

        const size_t n = eventTable->getNumberOfParameters( );
        for ( size_t i = 0; i < n; ++i )
        {
            const std::string shortName = eventTable->getParameterName( i );
            const std::string longName = eventTable->getParameterLongName( i );
            auto p = this->findParameter( shortName );
            if ( p == nullptr )
                this->addParameter( shortName, longName );
            else
                this->setParameterLongName( shortName, longName );
        }
    }

    /**
     * Copies parameters from an event table into this map.
     *
     * If a parameter in the given event table is already in this map, and
     * the event table's parameter has a non-empty long name, the long name
     * for the entry already in this map is overwritten with the event
     * table's long name.
     *
     * If a parameter is not already in this map, an entry is added with
     * the short and long names from the event table. The entry's
     * visibility and transform values are initialized to defaults.
     *
     * @param[in] eventTable
     *   The event table providing short and long parameter names.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a NULL pointer.
     *
     * @see ::addParameter()
     */
    void copy(
        std::shared_ptr<const FlowGate::Events::EventTableInterface> eventTable )
    {
        if ( eventTable == nullptr )
            throw new std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table.");

        const size_t n = eventTable->getNumberOfParameters( );
        for ( size_t i = 0; i < n; ++i )
        {
            const std::string shortName = eventTable->getParameterName( i );
            const std::string longName = eventTable->getParameterLongName( i );
            auto p = this->findParameter( shortName );
            if ( p == nullptr )
                this->addParameter( shortName, longName );
            else
                this->setParameterLongName( shortName, longName );
        }
    }
    // @}



//----------------------------------------------------------------------
// Parameters.
//----------------------------------------------------------------------
    /**
     * @name Parameters
     */
protected:
    /**
     * Returns the parameter with the given short name.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns the parameter object from the map, or a nullptr if the
     *   parameter is not found.
     */
    std::shared_ptr<const Parameter> findParameter(
        const std::string shortName )
        const noexcept
    {
        try
        {
            return this->map.at( shortName );
        }
        catch ( ... )
        {
            return nullptr;
        }
    }

    /**
     * Returns the parameter with the given short name.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns the parameter object from the map, or a nullptr if the
     *   parameter is not found.
     */
    std::shared_ptr<Parameter> findParameter(
        const std::string shortName )
        noexcept
    {
        try
        {
            return this->map.at( shortName );
        }
        catch ( ... )
        {
            return nullptr;
        }
    }


public:
    // @{
    /**
     * Adds a new parameter to the parameter map.
     *
     * @param[in] shortName
     *   The short name for the new parameter map entry.
     * @param[in] longName
     *   (optional, default = empty string) The long name for the parameter.
     *   The string may be empty.
     * @param[in] description
     *   (optional, default = empty string) The description for the parameter.
     *   The string may be empty.
     * @param[in] visibleForGating
     *   (optional, default = true) The visible for gating flag for the
     *   parameter.
     * @param[in] defaultTransform
     *   (optional, default = nullptr The default transform for the parameter.
     *   A nullptr indicates that there is no default transform.
     *
     * @throw std::invalid_argument
     *   Throws an exception if a parameter with the same short name is
     *   already in the map.
     *
     * @throw std::out_of_range
     *   Throws an exception if the short name is empty.
     *
     * @see ::isParameter()
     * @see ::setParameter()
     */
    void addParameter(
        const std::string shortName,
        const std::string longName = "",
        const std::string description = "",
        const bool visibleForGating = true,
        const std::shared_ptr<FlowGate::Gates::Transform> transform = nullptr )
    {
        if ( shortName.empty( ) == true )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid empty short parameter name." );
        if ( this->isParameter( shortName ) == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Parameter map already includes a parameter with the same short name.");

        auto p = std::make_shared<Parameter>(
            shortName,
            longName,
            description,
            visibleForGating,
            transform );
        this->map.emplace( shortName, p );
    }



    /**
     * Clears the parameter map.
     *
     * @see ::addParameter()
     * @see ::isEmpty()
     * @see ::setParameter()
     */
    void clear( )
        noexcept
    {
        this->map.clear( );
    }



    /**
     * Finds the default transform for the indicated parameter.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns a Transform object for the parameter's default transform,
     *   or a nullptr if there is node default. If the parameter does not
     *   exist in the map, a nullptr is returned.
     *
     * @see ::setParameterDefaultTransform()
     */
    std::shared_ptr<const FlowGate::Gates::Transform> findParameterDefaultTransform(
        const std::string shortName )
        const noexcept
    {
        const auto p = this->findParameter( shortName );
        if ( p == nullptr )
            return nullptr;
        return p->getDefaultTransform( );
    }

    /**
     * Finds the description for the indicated parameter.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns the parameter's description, or an empty string if there
     *   isn't one. If the parameter does not exist in the map, an empty
     *   string is returned.
     *
     * @see ::setParameterDescription()
     */
    const std::string findParameterDescription( const std::string shortName )
        const noexcept
    {
        const auto p = this->findParameter( shortName );
        if ( p == nullptr )
            return std::string( );
        return p->getDescription( );
    }

    /**
     * Finds the long parameter name for the indicated parameter.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns the parameter's long name, or an empty string if there
     *   isn't one. If the parameter does not exist in the map, an empty
     *   string is returned.
     *
     * @see ::setParameterLongName()
     * @see FlowGate::Events::EventTable::getParameterLongName()
     */
    const std::string findParameterLongName( const std::string shortName )
        const noexcept
    {
        const auto p = this->findParameter( shortName );
        if ( p == nullptr )
            return std::string( );
        return p->getLongName( );
    }

    /**
     * Finds the visible for gating flag for the indicated parameter.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns TRUE or FALSE for the parameter's visible for gating flag.
     *   If the parameter does not exist in the map, TRUE is returned.
     *
     * @see ::setParameterVisibleForGating()
     */
    bool findParameterVisibleForGating( const std::string shortName )
        const noexcept
    {
        const auto p = this->findParameter( shortName );
        if ( p == nullptr )
            return true;
        return p->getVisibleForGating( );
    }



    /**
     * Returns the number of parameters in the map.
     *
     * @return
     *   Returns the size of the map.
     *
     * @see ::isEmpty()
     * @see FlowGate::Events::EventTable::getNumberOfParameters()
     */
    size_t getNumberOfParameters( )
        const noexcept
    {
        return this->map.size( );
    }

    /**
     * Returns a vector of parameter names in the map.
     *
     * @return
     *   Returns a new vector containing parameter short names from the map.
     *   Names in the vector are not sorted.
     *
     * @see ::getNumberOfParameters()
     * @see ::isEmpty()
     * @see ::isParameter()
     * @see FlowGate::Events::EventTable::getParameterName()
     * @see FlowGate::Events::EventTable::getParameterNames()
     */
    std::vector<std::string> getParameterNames( )
        const noexcept
    {
        std::vector<std::string> v;

        for ( auto it = this->map.cbegin( ); it != this->map.cend( ); ++it )
            v.push_back( (*it).first );

        return v;
    }



    /**
     * Returns TRUE if the parameter map is empty.
     *
     * @return
     *   Returns TRUE if the map is empty, and FALSE otherwise.
     *
     * @see ::clear()
     * @see ::getNumberOfParameters()
     */
    bool isEmpty( )
        const noexcept
    {
        return this->map.empty( );
    }

    /**
     * Returns TRUE if a parameter with the short name exists in the map.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @return
     *   Returns TRUE if the parameter exists in the map, and FALSE otherwise.
     */
    bool isParameter( const std::string shortName )
        const noexcept
    {
        const auto it = this->map.find( shortName );
        return (it != this->map.end( ));
    }




    /**
     * Removes a parameter from the parameter map.
     *
     * If the parameter is not in the map, this method returns immediately
     * without an error.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     *
     * @see ::addParameter()
     * @see ::setParameter()
     */
    void removeParameter( const std::string shortName )
        noexcept
    {
        const auto it = this->map.find( shortName );
        if (it != this->map.end( ))
            return;
        this->map.erase( it );
    }



    /**
     * Sets a parameter in the parameter map.
     *
     * If the parameter is not in the map already, it is added.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     * @param[in] longName
     *   (optional, default = empty string) The long name for the parameter.
     *   The string may be empty.
     * @param[in] description
     *   (optional, default = empty string) The description for the parameter.
     *   The string may be empty.
     * @param[in] visibleForGating
     *   (optional, default = true) The visible for gating flag for the
     *   parameter.
     * @param[in] defaultTransform
     *   (optional, default = nullptr The default transform for the parameter.
     *   A nullptr indicates that there is no default transform.
     *
     * @throw std::out_of_range
     *   Throws an exception if the short name is empty.
     *
     * @see ::findParameterDefaultTransform()
     * @see ::findParameterDescription()
     * @see ::findParameterLongName()
     * @see ::findParameterVisibleForGating()
     * @see ::setParameterDefaultTransform()
     * @see ::setParameterDescription()
     * @see ::setParameterLongName()
     * @see ::setParameterVisibleForGating()
     */
    void setParameter(
        const std::string shortName,
        const std::string longName = "",
        const std::string description = "",
        const bool visibleForGating = true,
        const std::shared_ptr<FlowGate::Gates::Transform> transform = nullptr )
    {
        if ( shortName.empty( ) == true )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid empty short parameter name." );

        auto p = this->findParameter( shortName );
        if ( p == nullptr )
        {
            this->addParameter(
                shortName,
                longName,
                description,
                visibleForGating,
                transform );
        }
        else
        {
            p->setLongName( longName );
            p->setDescription( description );
            p->setVisibleForGating( visibleForGating );
            p->setDefaultTransform( transform );
        }
    }

    /**
     * Sets the default transform for the indicated parameter.
     *
     * If the parameter is not in the map yet, an entry is added first with
     * initial values.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     * @param[in] defaultTransform
     *   The default for the parameter. A nullptr clears the default.
     *
     * @see ::findParameterDefaultTransform()
     */
    void setParameterDefaultTransform(
        const std::string shortName,
        const std::shared_ptr<FlowGate::Gates::Transform> transform )
        noexcept
    {
        auto p = this->findParameter( shortName );
        if ( p == nullptr )
        {
            p.reset( new Parameter( shortName ) );
            this->map.emplace( shortName, p );
        }
        p->setDefaultTransform( transform );
    }

    /**
     * Sets the description for the indicated parameter.
     *
     * If the parameter is not in the map yet, an entry is added first with
     * initial values.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     * @param[in] description
     *   The description for the parameter. An empty string clears the name.
     *
     * @see ::findParameterDescription()
     */
    void setParameterDescription(
        const std::string& shortName,
        const std::string& description )
        noexcept
    {
        auto p = this->findParameter( shortName );
        if ( p == nullptr )
        {
            p.reset( new Parameter( shortName ) );
            this->map.emplace( shortName, p );
        }
        p->setDescription( description );
    }

    /**
     * Sets the long name for the indicated parameter.
     *
     * If the parameter is not in the map yet, an entry is added first with
     * initial values.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     * @param[in] longName
     *   The long name for the parameter. An empty string clears the name.
     *
     * @see ::findParameterLongName()
     * @see FlowGate::Events::EventTable::getParameterLongName()
     */
    void setParameterLongName(
        const std::string& shortName,
        const std::string& longName )
        noexcept
    {
        auto p = this->findParameter( shortName );
        if ( p == nullptr )
        {
            p.reset( new Parameter( shortName ) );
            this->map.emplace( shortName, p );
        }
        p->setLongName( longName );
    }

    /**
     * Sets the visible for gating flag for the indicated parameter.
     *
     * If the parameter is not in the map yet, an entry is added first with
     * initial values.
     *
     * @param[in] shortName
     *   The short name for a parameter.
     * @param[in] visibleForGating
     *   The visible for gating flag for the parameter.
     *
     * @see ::findParameterVisibleForGating()
     */
    void setParameterVisibleForGating(
        const std::string shortName,
        const bool visibleForGating )
        noexcept
    {
        auto p = this->findParameter( shortName );
        if ( p == nullptr )
        {
            p.reset( new Parameter( shortName ) );
            this->map.emplace( shortName, p );
        }
        p->setVisibleForGating( visibleForGating );
    }
    // @}
};

} // End Events namespace
} // End FlowGate namespace
