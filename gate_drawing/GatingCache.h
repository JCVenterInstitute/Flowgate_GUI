/**
 * @file
 * Manages state created during gating and used to accelerate further gating.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "EventTable.h"
 * @li FlowGate "GateTrees.h"
 *
 * @todo parameter name callback should get and save parameter long name,
 * and min/max from source event table into gate state.
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
#include "EventTable.h" // EventTable
#include "GateTrees.h"  // GateTrees





namespace FlowGate {
namespace Gates {

class GatingCache;





//----------------------------------------------------------------------
//
// Gate State.
//
//----------------------------------------------------------------------
/**
 * Manages the gating state for a single gate.
 *
 * Gating state is dominated by an event table containing columns for
 * the gate's parameters. This includes parameters used directly by the
 * gate's shape (e.g. the gate's dimensions) and parameters in the
 * gate's optional additional clustering parameters list. The latter
 * provides a list of additional parameters to transform and use when
 * finding clusters, for gates that use clustering.
 *
 * The underlying event table is a two-dimensional array of single- or
 * double-precision floating point values. Events in the table are arranged
 * in columns with one column per parameter used by the gate. Values in a
 * column are in rows. All columns have the same number of rows. The values
 * in a row, passing through multiple columns, are for the same event.
 *
 * Gate state also contains an additional column of event inclusion flags
 * with one value er event table row. A flag in the column set if the row's
 * event is included within the gate's shape or logic (e.g. inside the
 * bounds of a rectangle gate, or passing the conditions of a boolean gate).
 *
 * Additional gate state indicates which of the columns have been transformed
 * by the appropriate parameter transform from the gate in the gate tree.
 *
 *
 * <B>Creating gate state</B>
 * Gate state objects are created automatically by the GatingCache class.
 * They cannot be created directly.
 *
 *
 * <B>Working with parameter columns</B>
 *
 *
 * <B>Getting parameter values</B>
 *
 *
 * <B>Multi-threading</B><BR>
 * If OpenMP is in use, methods that copy events from one table to another
 * will use multiple threads for the event copying.
 *
 * All other class methods are presumed to be executed on a single thread,
 * or in a thread-safe manner. No thread locking is used in this class.
 *
 *
 * @internal
 * This class is implemented using techniques to improve performance:
 *
 * @li This class is final and it adds no new virtual methods.
 *
 * @li All methods are implemented in this .h file so that a compiler can
 *     inline them.
 *
 * @li Data is stored in parameter columns because most operations that use
 *     this class do column operations. This keeps consecutive values of the
 *     same column in consecutive locations in memory and improves processor
 *     cache efficiency.
 *
 * @li This class does not use a template to select the event data type.
 *     Doing so would prevent code from passing around an event table
 *     pointer without knowing its event data type.
 * @endinternal
 */
class GateState final
    : public FlowGate::Events::EventTable,
    public FlowGate::Gates::GateStateBase
{
    friend class FlowGate::Gates::GatingCache;

//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
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
    /**
     * The gate to which this state applies.
     *
     * The gate is set at construction time.
     *
     * @see ::getGate()
     */
// TODO WEAK PTR?
    std::shared_ptr<FlowGate::Gates::Gate> gate;

    /**
     * The source event table for the gate's parameter's events.
     *
     * The event table is set at construction time.
     */
// TODO WEAK PTR?
    std::shared_ptr<const FlowGate::Events::EventTableInterface> sourceEventTable;

    /**
     * A boolean flag indicating if the gate has been applied to its
     * parameter's events.
     *
     * @see ::isApplied()
     * @see ::setApplied()
     */
    bool applied;

    /**
     * A column of flags that indicate whether an event is included or
     * not within the bounds or conditions of the gate.
     *
     * @see ::isEventIncluded()
     * @see ::setEventIncluded()
     *
     * @internal
     * Each entry is used as a boolean flag, but the flags are stored as
     * 8-bit unsigned integers because a vector of integers can be safely
     * accessed in parallel, but a vector of booleans cannot be.
     * @endinternal
     */
    std::vector<uint8_t> eventIncluded;





//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
private:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs new gate state with an underlying event table initialized
     * with the gate's current parameter columns and the indicated number of
     * events, and event data type.
     *
     * The gate state is initialized to indicate the gate has not yet been
     * applied to its events.
     *
     * @param[in] gate
     *   The gate for which to maintain state.
     * @param[in] sourceEventTable
     *   The event table supplying the source events for the gate.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate or source event table are nullptrs,
     *   or if any parameter name used by the gate does not exist in the
     *   source event table.
     *
     * @see FlowGate::Events::EventTableInterface::areValuesFloats()
     * @see FlowGate::Events::EventTableInterface::getParameterDoubles()
     * @see FlowGate::Events::EventTableInterface::getParameterFloats()
     * @see FlowGate::Events::EventTableInterface::getNumberOfEvents()
     * @see FlowGate::Events::EventTableInterface::getParameterNames()
     */
    GateState(
        std::shared_ptr<FlowGate::Gates::Gate> gate,
        std::shared_ptr<const FlowGate::Events::EventTableInterface> sourceEventTable )
        : FlowGate::Events::EventTable(
            getAndValidateAllParameterNames( gate, sourceEventTable ),
            sourceEventTable->getNumberOfEvents( ),
            sourceEventTable->areValuesFloats( ),
// TODO Names must be unique?
            true ),
        gate( gate ),
        sourceEventTable( sourceEventTable )
    {
        // Initially, the gate has not been applied.
        this->applied = false;

        // All of the inclusion flags are TRUE.
        this->eventIncluded.resize( numberOfEvents, 1 );
        this->eventIncluded.shrink_to_fit( );

        // Copy all source event values from the event table into
        // parameter columns for gate shape dimensions and additional
        // clustering parameters.
        for ( DimensionIndex i = 0; i < this->numberOfParameters; ++i )
        {
            const uint32_t sourceIndex =
                this->sourceEventTable->getParameterIndex(
                    this->getParameterName( i ) );
            this->copyValues( this->sourceEventTable, sourceIndex, i );
        }

        // Transform everything.
        transformAll( );
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GateState( )
        noexcept
    {
        // Nothing to do.
    }
    // @}





//----------------------------------------------------------------------
// Construction utilities.
//----------------------------------------------------------------------
private:
    /**
     * @name Construction utilities
     */
    // @{
    /**
     * Returns a list of all parameter names in use by the gate.
     *
     * The returned list includes all names in use by the gate's dimensions,
     * if any, plus all names in use as additional clustering parameters, if
     * any. All names are checked for existence in the source event table.
     *
     * @return
     *   Returns a vector of parameter names. Names are in the same order
     *   as gate dimensions + gate additional clustering parameters.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate or source event table are nullptrs,
     *   or if any parameter name used by the gate does not exist in the
     *   source event table.
     */
    static std::vector<std::string> getAndValidateAllParameterNames(
        std::shared_ptr<const Gate> gate,
        std::shared_ptr<const FlowGate::Events::EventTableInterface> sourceEventTable )
    {
        // Validate.
        if ( gate == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate." );

        if ( sourceEventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL source event table." );

        // Start with gate shape dimension parameters.
        std::vector<std::string> names;
        DimensionIndex n = gate->getNumberOfDimensions( );
        for ( DimensionIndex i = 0; i < n; ++i )
            names.push_back( gate->getDimensionParameterName( i ) );

        // Add any additional clustering parameters.
        n = gate->getNumberOfAdditionalClusteringParameters( );
        for ( DimensionIndex i = 0; i < n; ++i )
            names.push_back( gate->getAdditionalClusteringParameterName( i ) );

        // Validate that all gate parameters exist in the event table.
        n = names.size( );
        for ( DimensionIndex i = 0; i < n; ++i )
        {
            if ( sourceEventTable->isParameter( names[i] ) == false )
                throw std::invalid_argument(
                    ERROR_PROGRAMMER +
                    "Parameter \"" + names[i] + "\" in gate not found in event table." );
        }

        return names;
    }
    // @}





//----------------------------------------------------------------------
// Reset.
//----------------------------------------------------------------------
private:
    /**
     * @name Reset
     */
    // @{
    /**
     * Invalidates gate state on whether it has been applied and the results.
     *
     * If the gate is already marked as not applied yet, then this method
     * returns immediately without doing anything. Otherwise the gate
     * applied flag is set to FALSE and all event inclusion flags are
     * reset to their default (TRUE).
     *
     * @param[in] invalidateChildren
     *   (optional, default = TRUE) When TRUE, the gate's children are
     *   updated to invalidate their current gating results. This is only
     *   done if the gate was previously applied.
     */
    inline void invalidateGateResults( const bool invalidateChildren = true )
        noexcept
    {
        // If this gate has not been applied, then the gate inclusion
        // flags are still at their default state and none of the gate's
        // children can be applied either. Just return.
        if ( this->applied == false )
            return;

        // Set this gate to be NOT applied and reset all of the event
        // inclusion flags to their default (TRUE).
        this->applied = false;
        std::memset( this->eventIncluded.data( ), 1, this->numberOfEvents );

        if ( invalidateChildren == true )
        {
            // Loop through all children and reset them too since if this gate
            // has not been applied, then neither can the gate's children be.
            const GateIndex n = this->gate->getNumberOfChildren( );
            for ( GateIndex i = 0; i < n; ++i )
            {
                auto baseState = this->gate->getChild( i )->getState( );
                auto gState = std::dynamic_pointer_cast<GateState>( baseState );
                if ( gState != nullptr )
                    gState->invalidateGateResults( invalidateChildren );
            }
        }
    }

    /**
     * Resets gate state.
     *
     * Source event table values for all parameters are copied into
     * the gate state. Gate inclusion flags are reset and the gate
     * is marked as not applied.
     *
     * @param[in] invalidateChildren
     *   (optional, default = TRUE) When TRUE, the gate's children are
     *   updated to invalidate their current gating results.
     */
    inline void reset( const bool invalidateChildren = true )
        noexcept
    {
        // Copy all source event values from the event table back into
        // parameter columns for gate shape dimensions and additional
        // clustering parameters.
        for ( DimensionIndex i = 0; i < this->numberOfParameters; ++i )
        {
            const uint32_t sourceIndex =
                this->sourceEventTable->getParameterIndex(
                    this->getParameterName( i ) );
            this->copyValues( this->sourceEventTable, sourceIndex, i );
        }

        // With event values reset, the gate has no longer been applied.
        // Reset the applied flag and set all event inclusions to TRUE
        // (the default).
        invalidateGateResults( invalidateChildren );

        // Transform everything.
        transformAll( );
    }

    /**
     * Resets gate state for the selected parameter.
     *
     * Source event table values for the indicated parameter are copied into
     * the gate state and transformed. Gate inclusion flags are reset and
     * the gate is marked as not applied.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] invalidateChildren
     *   (optional, default = TRUE) When TRUE, the gate's children are
     *   updated to invalidate their current gating results.
     */
    inline void reset(
        const DimensionIndex index,
        const bool invalidateChildren = true )
    {
        // Validate.
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid parameter index is out of range." );

        // Copy source event values from the event table back into the
        // parameter's column.
        const auto parameterName = this->getParameterName( index );
        const uint32_t sourceIndex =
            this->sourceEventTable->getParameterIndex( parameterName );
        this->copyValues( this->sourceEventTable, sourceIndex, index );

        // Update the parameter's column with the source column's parameter
        // name, long name, and min/max.
        this->setParameterLongName(
            index,
            this->sourceEventTable->getParameterLongName( sourceIndex ) );

        this->setParameterMinimum(
            index,
            this->sourceEventTable->getParameterMinimum( sourceIndex ) );

        this->setParameterMaximum(
            index,
            this->sourceEventTable->getParameterMaximum( sourceIndex ) );

        this->setParameterDataMinimum(
            index,
            this->sourceEventTable->getParameterDataMinimum( sourceIndex ) );

        this->setParameterDataMaximum(
            index,
            this->sourceEventTable->getParameterDataMaximum( sourceIndex ) );

        // With event values reset, the gate has no longer been applied.
        // Reset the applied flag and set all event inclusions to TRUE
        // (the default).
        invalidateGateResults( invalidateChildren );

        // Transform the new values.
        transform( index );
    }

    /**
     * Transforms the indicated parameter's events, if needed.
     *
     * The gate is marked as not applied and its inclusion flags for all
     * events reset.
     *
     * @param[in] index
     *   The parameter index. The index is presumed to be valid.
     */
    inline void transform( const DimensionIndex index )
        noexcept
    {
        // Get the transform.
        std::shared_ptr<const Transform> transform;
        const auto nDimensions = this->gate->getNumberOfDimensions( );
        if ( index < nDimensions )
            transform = this->gate->getDimensionParameterTransform( index );
        else
            transform = this->gate->getAdditionalClusteringParameterTransform( index - nDimensions );

        // If the transform is not NULL, then use it to modify the events
        // in place.
        if ( transform != nullptr )
        {
            if ( this->areValuesFloats( ) == true )
            {
                transform->transform(
                    this->numberOfEvents,
                    this->getParameterFloats( index ).data( ) );
            }
            else
            {
                transform->transform(
                    this->numberOfEvents,
                    this->getParameterDoubles( index ).data( ) );
            }

            // Transform the parameter's specified min/max.
            // The specified min/max is supposed to be the full range that
            // the original instrument can produce for event values. This
            // may be a larger range than for the actual data here.
            double min = this->getParameterMinimum( index );
            double tmin = transform->transform( min );
            this->setParameterMinimum( index, tmin );

            double max = this->getParameterMaximum( index );
            double tmax = transform->transform( max );
            this->setParameterMaximum( index, tmax );

            // Transform the parameter's data min/max too.
            // The data min/max is supposed to be the range of the actual
            // data here. On first look, it should be possible to just
            // transform the pre-transform data min/max. But this presumes
            // a strictly increasing transform result from low to high.
            // In practice, a transform can do all sorts of mappings and
            // the post-transform min/max of the entire data column may
            // not be the same as a post-transform of the previous data
            // min/max. So, we must actually scan the data to find the
            // new data min/max.
            this->computeParameterDataMinimumMaximum( );
        }

        // Since event values have changed, the gate's inclusion flags are
        // no longer valid. Mark the gate as not applied and reset the flags.
        invalidateGateResults( );
    }

    /**
     * Transforms all parameters for the gate.
     *
     * The gate is marked as not applied and its inclusion flags for all
     * events reset.
     */
    inline void transformAll( )
        noexcept
    {
        const DimensionIndex n = this->numberOfParameters;
        for ( DimensionIndex i = 0; i < n; ++i )
            this->transform( i );
    }
    // @}





//----------------------------------------------------------------------
// Callbacks.
//----------------------------------------------------------------------
public:
    /**
     * @name Callbacks
     */
    // @{
    /**
     * Updates state when a parameter is added to the list of additional
     * clustering parameters.
     *
     * When a parameter is added, the gate state's table of event values
     * is updated to include values for the additional parameter.
     *
     * If the gate's gating method only uses event values, then additional
     * clustering parameters are not involved in gating and the addition
     * of a parameter does not invalidate the current gate results.
     *
     * But if the gate's gating method is using clustering, then the
     * added parameter invalidates prior gating results. If the gate had
     * been previously applied, then the gate is now marked as not applied,
     * per-event inclusion flags are reset, and children are reset.
     *
     * The new event values are automatically transformed, if the parameter
     * has a transform.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     */
    virtual void callbackAppendAdditionalClusteringParameter(
        const std::string name,
        std::shared_ptr<const Transform> transform )
        noexcept override final
    {
        uint32_t index = 0;
        try
        {
            // Get the parameter index. This SHOULD FAIL since appending
            // should not occur for a parameter already in the list.
            // But, just in case, look it up anyway.
            index = this->getParameterIndex( name );
        }
        catch ( ... )
        {
            // The parameter name is not already in use, which is as it
            // should be. Append it.
            try
            {
                index = this->appendParameter( name );
            }
            catch ( ... )
            {
                // Should not be possible. This only occurs if the name is
                // empty (it isn't) or it is already in use (it isn't.).
                return;
            }
        }

        // Reset the parameter by copying event values from the source
        // event table, and transforming them with the latest parameter
        // transform (if any). Then, recurse through children to invalidate
        // their gating if this gate had already been applied and this
        // gate is using a non-event (i.e. clustering) gating method.
        const bool invalidateChildren =
            (this->applied == true) &&
            (this->gate->getGatingMethod( ) != EVENT_VALUE_GATING);
        reset( index, invalidateChildren );
    }

    /**
     * Updates state when a child gate is added to a parent gate.
     *
     * If the new child does not have state attached to it, new state is
     * attached and initialized.
     *
     * Otherwise the new child's gating results are invalidated.
     *
     * @param[in] child
     *   The new child gate.
     */
    virtual void callbackAppendChild( std::shared_ptr<Gate> child )
        noexcept override final
    {
        // The child may or may not have existing state, and that state may
        // or may not be valid:
        // - If no existing state, allocate it.
        // - If existing state is not a GateState object, replace it.
        // - If existing state has wrong event table replace it.
        //
        // If a new state object is allocated, it is automatically initialized
        // to have parameter columns for the child gate's parameters. Those
        // columns are initialized to values, and the values are transformed.
        // The initial state is also marked as not applied and all per-event
        // inclusion flags are reset.
        auto sstate = child->getState( );
        if ( sstate == nullptr )
        {
            // No state yet. Add it.
            sstate.reset( new GateState( child, this->sourceEventTable ) );
            child->setState( sstate );
            return;
        }

        auto state = dynamic_cast<GateState*>( sstate.get( ) );
        if ( state == nullptr )
        {
            // Wrong state object. Replace it.
            sstate.reset( new GateState( child, this->sourceEventTable ) );
            child->setState( sstate );
            return;
        }

        if ( state->sourceEventTable != this->sourceEventTable )
        {
            // Wrong event table. Replace it.
            sstate.reset( new GateState( child, this->sourceEventTable ) );
            child->setState( sstate );
            return;
        }

        // Invalidate the gate's results, and those of its children.
        invalidateGateResults( true );
    }

    /**
     * Updates state when the additional clustering parameter list is cleared.
     *
     * When the list is cleared, all of the additional parameters are removed
     * from the gate state's table of event values.
     *
     * If the gate's gating method only uses event values, then the parameters
     * were not involved in gating anyway and the gate's results are not
     * invalidated.
     *
     * Otherwise, if the gate had been previously applied, then the gate
     * is now marked as not applied, per-event inclusion flags are reset,
     * and children are reset.
     */
    virtual void callbackClearAdditionalClusteringParameters( )
        noexcept override final
    {
        const DimensionIndex nDimensions = this->gate->getNumberOfDimensions( );
        const DimensionIndex nParameters = this->numberOfParameters;

        // If there weren't any to begin with, then no change.
        if ( nDimensions == this->numberOfParameters )
            return;

        // Remove all additional clustering parameters.
        for ( DimensionIndex i = nParameters - 1; i >= nDimensions; --i )
            removeParameter( i );

        // If the gate has not been applied, or it isn't using the additional
        // clustering parameters anyway, then there's nothing more to do.
        if ( this->applied == false ||
             this->gate->getGatingMethod( ) == EVENT_VALUE_GATING )
            return;

        // Invalidate the gate and its children.
        invalidateGateResults( true );
    }

    /**
     * Updates state when a gate's children list is cleared.
     */
    virtual void callbackClearChildren( )
        noexcept override final
    {
        // The cleared children, and their state, are automatically deleted.
        // The parent's state has nothing to update.
    }

    /**
     * Updates state when a parameter is removed from the additional
     * clustering list.
     *
     * When a parameter is removed from the list, the corresponding values
     * in the gate state's table of event values are removed.
     *
     * If the gate's gating method only uses event values, then the parameters
     * were not involved in gating anyway and the gate's results are not
     * invalidated.
     *
     * Otherwise, if the gate had been previously applied, then the gate
     * is now marked as not applied, per-event inclusion flags are reset,
     * and children are reset.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     */
    virtual void callbackRemoveAdditionalClusteringParameter(
        const std::string name )
        noexcept override final
    {
        // Remove the parameter column from the state event table.
        removeParameter( name );

        // If the gate has not been applied, or it isn't using the additional
        // clustering parameters anyway, then there's nothing more to do.
        if ( this->applied == false ||
             this->gate->getGatingMethod( ) == EVENT_VALUE_GATING )
            return;

        // Invalidate the gate and its children.
        invalidateGateResults( true );
    }

    /**
     * Updates state when a child is removed from a parent.
     *
     * @param[in] gate
     *   The gate to remove as a child.
     */
    virtual void callbackRemoveChild( const std::shared_ptr<Gate>& gate )
        noexcept override final
    {
        // The removed child, and its state, are automatically deleted.
        // The parent's state has nothing to update.
    }

    /**
     * Updates state when an additional clustering parameter's transform
     * changes.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     */
    virtual void callbackSetAdditionalClusteringParameterTransform(
        const std::string name,
        std::shared_ptr<const Transform> transform )
        noexcept override final
    {
        try
        {
            const auto index = this->getParameterIndex( name );

            // Reset the parameter by copying event values from the source
            // event table, and transforming them with the latest parameter
            // transform (if any). Then, recurse through children to invalidate
            // their gating if this gate had already been applied and this
            // gate is using a non-event (i.e. clustering) gating method.
            const bool invalidateChildren =
                (this->applied == true) &&
                (this->gate->getGatingMethod( ) != EVENT_VALUE_GATING);
            reset( index, invalidateChildren );
        }
        catch ( ... )
        {
            // Parameter is not known? Should not be able to happen.
        }
    }

#ifdef COMPENSATION_SUPPORTED
    /**
     * Updates state when the compensation method is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] method
     *   The compensation method.
     */
    virtual void callbackSetDimensionCompensationMethod(
        const DimensionIndex index,
        const CompensationMethod method )
        noexcept override final
    {
        // ??
    }
#endif

    /**
     * Updates state when the parameter name is set for the gate.
     *
     * When a parameter's name is changed, the gate state's table of event
     * values is updated to use values from the newly chosen parameter.
     *
     * If the gate had been previously applied, then the gate is now marked
     * as not applied, per-event inclusion flags are reset, and children
     * are reset.
     *
     * The new event values are automatically transformed, if the parameter
     * has a transform.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] name
     *   The parameter name.
     */
    virtual void callbackSetDimensionParameterName(
        const DimensionIndex index,
        const std::string& name )
        noexcept override final
    {
        // Reset the parameter by copying event values from the source
        // event table, and transforming them with the latest parameter
        // transform (if any). Then, recurse through children to invalidate
        // their gating if this gate had already been applied.
        this->setParameterName( index, name );
        const bool invalidateChildren = (this->applied == true);
        reset( index, invalidateChildren );
    }

    /**
     * Updates state when the parameter transform is set for the gate.
     *
     * When a parameter's transform is changed, the gate state's table of
     * event values is reset to their original values.
     *
     * If the gate had been previously applied, then the gate is
     * now marked as not applied, per-event inclusion flags are reset, and
     * children are reset.
     *
     * The new event values are automatically transformed, if the parameter
     * has a transform.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] transform
     *   The transform transform.
     */
    virtual void callbackSetDimensionParameterTransform(
        const DimensionIndex index,
        std::shared_ptr<Transform> transform )
        noexcept override final
    {
        // Reset the parameter by copying event values from the source
        // event table, and transforming them with the latest parameter
        // transform (if any). Then, recurse through children to invalidate
        // their gating if this gate had already been applied.
        const bool invalidateChildren = (this->applied == true);
        reset( index, invalidateChildren );
    }

    /**
     * Updates state when the gating method is set for the gate.
     *
     * When the gating method changes, the current per-event inclusion
     * flags are no longer valid. The gate is marked as not applied
     * and the flags reset. If the gate had been applied, invalidate
     * all children too.
     *
     * @param[in] method
     *   The gating method.
     */
    virtual void callbackSetGatingMethod( const GatingMethod method )
        noexcept override final
    {
        if ( this->applied == true )
            invalidateGateResults( true );
    }

    /**
     * Updates state when a rectangle gate's min/max are changed.
     *
     * When the gate min/max changes, the current per-event inclusion
     * flags are no longer valid. The gate is marked as not applied
     * and the flags reset. If the gate had been applied, invalidate
     * all children too.
     *
     * @param[in] index
     *   The dimension.
     * @param[in] minimum
     *   The new minimum.
     * @param[in] maximum
     *   The new maximum.
     */
    virtual void callbackSetRectangleMinimumMaximum(
        const DimensionIndex index,
        const double minimum,
        const double maximum )
        noexcept override final
    {
        if ( this->applied == true )
            invalidateGateResults( true );
    }

    /**
     * Updates state when a polygon gate's vertex is appended.
     *
     * @param[in] index
     *   The vertex index.
     * @param[in] x
     *   The new X coordinate.
     * @param[in] y
     *   The new Y coordinate.
     */
    virtual void callbackAppendPolygonVertex(
        const VertexIndex index,
        const double x,
        const double y )
        noexcept override final
    {
        if ( this->applied == true )
            invalidateGateResults( true );
    }

    /**
     * Updates state when a polygon gate's vertex list is cleared.
     */
    virtual void callbackClearPolygonVertices( )
        noexcept override final
    {
        if ( this->applied == true )
            invalidateGateResults( true );
    }

    /**
     * Updates state when a polygon gate's vertex is removed.
     *
     * @param[in] index
     *   The vertex index.
     */
    virtual void callbackRemovePolygonVertex(
        const VertexIndex index )
        noexcept override final
    {
        if ( this->applied == true )
            invalidateGateResults( true );
    }

    /**
     * Updates state when a polygon gate's vertex is changed.
     *
     * @param[in] index
     *   The vertex index.
     * @param[in] x
     *   The new X coordinate.
     * @param[in] y
     *   The new Y coordinate.
     */
    virtual void callbackSetPolygonVertex(
        const VertexIndex index,
        const double x,
        const double y )
        noexcept override final
    {
        if ( this->applied == true )
            invalidateGateResults( true );
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
     * Returns the vector of event-included flags.
     *
     * The returned vector has one value per event. A value of 0 indicates
     * the event is NOT included within the gate, and a non-O (usually 1)
     * value indicates it IS included.
     *
     * @return
     *   The vector of event-included flags.
     */
    virtual const std::vector<uint8_t>& getEventIncludedList( )
        const noexcept override final
    {
        return this->eventIncluded;
    }

    /**
     * Returns the vector of event-included flags.
     *
     * The returned vector has one value per event. A value of 0 indicates
     * the event is NOT included within the gate, and a non-O (usually 1)
     * value indicates it IS included.
     *
     * @return
     *   The vector of event-included flags.
     */
    virtual std::vector<uint8_t>& getEventIncludedList( )
        noexcept override final
    {
        return this->eventIncluded;
    }

    /**
     * Returns the gate backing this gate state.
     *
     * @return
     *   Returns the gate.
     */
    virtual std::shared_ptr<const Gate> getGate( )
        const noexcept override final
    {
        return this->gate;
    }

    /**
     * Returns TRUE if the gate has already been applied to the state's events.
     *
     * @return
     *   Returns TRUE if applied.
     */
    virtual bool isApplied( )
        const noexcept override final
    {
        return this->applied;
    }

    /**
     * Returns TRUE if the indicated event is included within the gate's shape
     * or conditions.
     *
     * @param[in] index
     *   The event index.
     *
     * @return
     *  Returns TRUE if the event is included.
     */
    virtual bool isEventIncluded( const size_t index )
        const override final
    {
        if ( index >= this->numberOfEvents )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid event index is out of range." );
        return (this->eventIncluded[index] != 0);
    }



    /**
     * Sets whether the gate has been applied to its events.
     *
     * It is the caller's responsability to know if the gate's event values
     * have been used for gating and the per-event inclusion flags are valid.
     *
     * @param[in] trueFalse
     *   TRUE if applied, FALSE if not.
     */
    virtual void setApplied( const bool trueFalse )
        noexcept override final
    {
        if ( this->applied == trueFalse )
        {
            // No change. Do nothing.
            return;
        }

        if ( trueFalse == true )
        {
            // Mark the gate as applied. Do nothing else.
            this->applied = trueFalse;
            return;
        }

        // Otherwise we're marking the gate as NOT applied. Reset the
        // event inclusion flags.
        invalidateGateResults( );
    }

    /**
     * Sets whether the indicated event is included within the gate's shape
     * or conditions.
     *
     * @param[in] index
     *   The event index.
     * @param[in] trueFalse
     *   TRUE if included, FALSE if not.
     */
    virtual void setEventIncluded( const size_t index, const bool trueFalse )
        override final
    {
        if ( index >= this->numberOfEvents )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid event index is out of range." );
        this->eventIncluded[index] = (trueFalse) ? 1 : 0;
    }
    // @}
};





//----------------------------------------------------------------------
//
// Gating Cache.
//
//----------------------------------------------------------------------
/**
 * Manages event data and a gate tree as a cache of state during gating.
 *
 * The principal components of a gating cache include:
 * - A source event table.
 * - A gate tree.
 * - State for each gate, including transformed events and gating flags.
 *
 * The source event table is a two-dimensional array of single- or
 * double-precision floating point values that are typically read from a
 * file. Values are arranged in named parameter columns and each gate in
 * the gate tree selects a few of these for use in gating.
 *
 *
 * <B>Creating a gating cache</B><BR>
 *
 *
 * <B>Transforming events</B><BR>
 *
 *
 * <B>Getting and setting events</B><BR>
 *
 *
 * <B>Multi-threading</B><BR>
 * If OpenMP is in use, the constructor that copies a source event table
 * will use multiple threads for the event copying.
 *
 * All other class methods are presumed to be executed on a single thread,
 * or in a thread-safe manner. No thread locking is used in this class.
 *
 *
 * @internal
 * This class is implemented using techniques to improve performance:
 *
 * @li This class is final and it adds no new virtual methods.
 *
 * @li All methods are implemented in this .h file so that a compiler can
 *     inline them.
 *
 * @li Data is stored in parameter columns because most operations that use
 *     this class do column operations. This keeps consecutive values of the
 *     same column in consecutive locations in memory and improves processor
 *     cache efficiency.
 *
 * @li This class does not use a template to select the event data type.
 *     Doing so would prevent code from passing around an event table
 *     pointer without knowing its event data type.
 * @endinternal
 */
class GatingCache final
    : public GateTreesStateBase
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate Gating Cache";

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
     */
    inline static const std::string DEFAULT_VERBOSE_PREFIX =
        "GatingCache";



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



    // Cache -----------------------------------------------------------
    /**
     * The source event table, set at construction time.
     *
     * @see ::getSourceEventTable()
     */
    std::shared_ptr<const FlowGate::Events::EventTableInterface> sourceEventTable;

    /**
     * The gate trees used to build and transform the events.
     *
     * @see ::getGateTrees()
     */
    std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees;



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
     * Constructs a new gating cache with the given source event table
     * and gate trees.
     *
     * @param[in] eventTable
     *   The source event table.
     * @param[in] gateTrees
     *   The source gate trees.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table or gate trees are NULL
     *   pointers or empty.
     */
    GatingCache(
        std::shared_ptr<const FlowGate::Events::EventTableInterface> eventTable,
        std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees )
        : sourceEventTable( eventTable ),
        gateTrees( gateTrees )
    {
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate trees." );

        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL source event table." );

        // Allocate state for all gates.
        const auto gates = gateTrees->findDescendentGates( );
        for ( auto& gate : gates )
        {
            std::shared_ptr<GateStateInterface> state;
            state.reset( new GateState( gate, this->sourceEventTable ) );
            gate->setState( state );
        }
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GatingCache( )
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
     * @see ::setVerbose()
     * @see ::getVerbosePrefix()
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
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the gate trees in the cache.
     *
     * The gate trees are set at construction time and cannot be changed.
     *
     * @return
     *   Returns the gate tree.
     */
    inline std::shared_ptr<const FlowGate::Gates::GateTrees> getGateTrees( )
        const noexcept
    {
        return this->gateTrees;
    }

    /**
     * Returns the gate trees in the cache.
     *
     * The gate trees are set at construction time and cannot be changed.
     *
     * @return
     *   Returns the gate tree.
     */
    inline std::shared_ptr<FlowGate::Gates::GateTrees> getGateTrees( )
        noexcept
    {
        return this->gateTrees;
    }

    /**
     * Returns the source event table.
     *
     * The source event table is set at construction time and cannot
     * be changed.
     *
     * @return
     *   Returns the table.
     */
    inline std::shared_ptr<const FlowGate::Events::EventTableInterface> getSourceEventTable( )
        const noexcept
    {
        return this->sourceEventTable;
    }
    // @}



//----------------------------------------------------------------------
// Callbacks.
//----------------------------------------------------------------------
public:
    /**
     * @name Callbacks
     */
    // @{
    /**
     * Updates state when a gate root is appended to the root list.
     *
     * @param[in] root
     *   The gate tree root to append.
     */
    virtual void callbackAppendGateTree( const std::shared_ptr<Gate>& root )
        noexcept override final
    {
        // The new root may or may not have existing state, and that state may
        // or may not be valid:
        // - If no existing state, allocate it.
        // - If existing state is not a GateState object, replace it.
        // - If existing state has wrong event table replace it.
        //
        // If a new state object is allocated, it is automatically initialized
        // to have parameter columns for the root gate's parameters. Those
        // columns are initialized to values, and the values are transformed.
        // The initial state is also marked as not applied and all per-event
        // inclusion flags are reset.
        auto baseState = root->getState( );
        if ( baseState == nullptr )
        {
            // No state yet. Add it.
            baseState.reset( new GateState( root, this->sourceEventTable ) );
            root->setState( baseState );
            return;
        }

        auto gState = std::dynamic_pointer_cast<GateState>( baseState );
        if ( gState == nullptr )
        {
            // Wrong state object. Replace it.
            baseState.reset( new GateState( root, this->sourceEventTable ) );
            root->setState( baseState );
            return;
        }

        if ( gState->sourceEventTable != this->sourceEventTable )
        {
            // Wrong event table. Replace it.
            baseState.reset( new GateState( root, this->sourceEventTable ) );
            root->setState( baseState );
            return;
        }

        // Invalidate the gate's results, and those of its children.
        gState->invalidateGateResults( true );
    }

    /**
     * Updates state when the gate tree root list is cleared.
     */
    virtual void callbackClearGateTrees( )
        noexcept override final
    {
        // The cleared roots, and their state, are automatically deleted.
        // The GateTrees state has nothing to update.
    }

    /**
     * Updates state when a gate tree root is removed from the list.
     *
     * @param[in] root
     *   The gate tree root to remove.
     */
    virtual void callbackRemoveGateTree( const std::shared_ptr<Gate>& root )
        noexcept override final
    {
        // The removed root, and its state, are automatically deleted.
        // The GateTrees state has nothing to update.
    }
    // @}
};

} // End Gates namespace
} // End FlowGate namespace
