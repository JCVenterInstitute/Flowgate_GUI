/**
 * @file
 * Manages an event table composed of named parameter columns.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
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
#include <limits>       // std::numeric_limits
#include <map>          // std::map
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector

// OpenMP.
#ifdef _OPENMP
#include <omp.h>        // OpenMP
#endif





namespace FlowGate {
namespace Events {





//----------------------------------------------------------------------
//
// Event table.
//
//----------------------------------------------------------------------
/**
 * Defines an interface for access to event tables.
 */
class EventTableInterface
{
//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
    /**
     * @name Constructors
     */
    // @{
    // General constructors --------------------------------------------
protected:
    /**
     * Constructs a new event table with the indicated parameter columns,
     * number of events, and event data type.
     */
    EventTableInterface( )
        noexcept
    {
        // Nothing to do.
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~EventTableInterface( )
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
    virtual std::string getVerbosePrefix( )
        const noexcept = 0;

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
    virtual bool isVerbose( )
        const noexcept = 0;

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
    virtual void setVerbose( const bool enable )
        noexcept = 0;

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
    virtual void setVerbosePrefix( const std::string prefix )
        noexcept = 0;
    // @}



//----------------------------------------------------------------------
// Parameters.
//----------------------------------------------------------------------
    /**
     * @name Parameters
     */
    // @{
public:
    /**
     * Appends a parameter column.
     *
     * @param[in] name
     *   The name of the new parameter. The name should differ from that
     *   of any other parameter already in the table.
     *
     * @return
     *   Returns the new parameter's index.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the name is empty or not unique, and
     *   unique names are required for the event table.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterIndex()
     */
    virtual uint32_t appendParameter( const std::string& name ) = 0;

    /**
     * Returns the number of parameter columns.
     *
     * @return
     *   Returns the number of parameter columns.
     *
     * @see ::getParameterName()
     */
    virtual uint32_t getNumberOfParameters( )
        const noexcept = 0;

    /**
     * Returns the long name of the indicated parameter column.
     *
     * The long name of a parameter is optional and it may be empty.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   The long name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterName()
     * @see ::getParameterNames()
     */
    virtual const std::string& getParameterLongName( const uint32_t index )
        const = 0;

    /**
     * Returns the primary (short) name of the indicated parameter column.
     *
     * Parameter names must not be empty and they must be unique within
     * the table.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   The name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterLongName()
     * @see ::getParameterNames()
     */
    virtual const std::string& getParameterName( const uint32_t index )
        const = 0;

    /**
     * Returns the index of the named parameter.
     *
     * @param[in] name
     *   The parameter name.
     *
     * @return
     *   The parameter index.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter name is not found.
     *
     * @see ::getParameterName()
     * @see ::isParameter()
     */
    virtual uint32_t getParameterIndex( const std::string& name )
        const = 0;

    /**
     * Returns a list of parameter long names.
     *
     * Long names are optional. An empty string is returned in the name
     * list if the parameter has no long name.
     *
     * <B>Performance note:</B><BR>
     * Use of this method is discouraged.  This method returns a <B>copy</B>
     * of all parameter long names in a vector that is, itself, <B>copied</B>
     * when it is returned. In contrast, this simpler getParameterLongName()
     * method returns a single const <B>reference</B> to the stored
     * parameter name. This will be much faster since it requires no memory
     * allocation or copying.
     *
     * @return
     *   Returns a new vector containing copies of all parameter long names,
     *   in the same order as parameters in the event table.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     */
    virtual std::vector<std::string> getParameterLongNames( )
        const noexcept = 0;

    /**
     * Returns a list of parameter names.
     *
     * <B>Performance note:</B><BR>
     * Use of this method is discouraged.  This method returns a <B>copy</B>
     * of all parameter names in a vector that is, itself, <B>copied</B>
     * when it is returned. In contrast, this simpler getParameterName()
     * method returns a single const <B>reference</B> to the stored
     * parameter name. This will be much faster since it requires no memory
     * allocation or copying.
     *
     * @return
     *   Returns a new vector containing copies of all parameter names,
     *   in the same order as parameters in the event table.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     */
    virtual std::vector<std::string> getParameterNames( )
        const noexcept = 0;

    /**
     * Returns TRUE if named parameter exists in this table.
     *
     * @param[in] name
     *   The parameter name.
     *
     * @return
     *   TRUE if the parameter name exists.
     *
     * @see ::getParameterIndex()
     */
    virtual bool isParameter( const std::string& name )
        const = 0;



    /**
     * Removes a parameter column.
     *
     * @param[in] name
     *   The parameter name.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter name is not found.
     */
    virtual void removeParameter( const std::string& name ) = 0;

    /**
     * Removes a parameter column.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    virtual void removeParameter( const uint32_t index ) = 0;

    /**
     * Sets the long name of the indicated parameter column.
     *
     * The long name of a parameter is optional and it may be empty.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] name
     *   The long name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::setParameterLongNames()
     * @see ::setParameterNames()
     */
    virtual void setParameterLongName(
        const uint32_t index,
        const std::string& name ) = 0;

    /**
     * Sets all parameter long names.
     *
     * The long name of a parameter is optional and it may be empty.
     *
     * @param[in] names
     *   The vector of long names. There must be one entry in the vector
     *   for each parameter.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the parameter name vector is too small.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterLongNames()
     * @see ::setParameterLongName()
     */
    virtual void setParameterLongNames( const std::vector<std::string> names ) = 0;

    /**
     * Sets the primary (short) name of the indicated parameter column.
     *
     * Parameter names must not be empty and they must be unique within
     * the table.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] name
     *   The name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range, or
     *   the name is already in use by another parameter.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterNames()
     * @see ::setParameterNames()
     */
    virtual void setParameterName(
        const uint32_t index,
        const std::string& name ) = 0;

    /**
     * Sets all parameter primary (short) names.
     *
     * Parameter names must not be empty and they must be unique within
     * the table.
     *
     * @param[in] names
     *   The vector of long names. There must be one entry in the vector
     *   for each parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if a name is already in use by another parameter.
     * @throw std::invalid_argument
     *   Throws an exception if the parameter name vector is too small.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterNames()
     * @see ::setParameterName()
     */
    virtual void setParameterNames( const std::vector<std::string> names ) = 0;
    // @}



//----------------------------------------------------------------------
// Event attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Event attributes
     */
    // @{
    /**
     * Returns true if events use single-precision floating point.
     *
     * The data type for events is set on the constructor and cannot be
     * changed afterwards.
     *
     * @return
     *   Returns true if events use single-precision floating point,
     *   and false if they use double-precision floating point.
     *
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::getNumberOfEvents()
     */
    virtual const bool areValuesFloats( )
        const noexcept = 0;

    /**
     * Returns the number of events.
     *
     * @return
     *   Returns the number of events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfOriginalEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     */
    virtual size_t getNumberOfEvents( )
        const noexcept = 0;

    /**
     * Returns the number of events in an original table, such as a file.
     *
     * When an event table is loaded from a file, and only a subset of
     * events are loaded, then the value returned here is the total number
     * of events in that file.
     *
     * @return
     *   Returns the number of original events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     * @see ::setNumberOfOriginalEvents()
     */
    virtual size_t getNumberOfOriginalEvents( )
        const noexcept = 0;



    /**
     * Sets the number of events.
     *
     * If the new number of events is larger than the current number,
     * additional events are added to all parameter columns and initialized
     * to zeroes.
     *
     * If the new number of events is smaller than the current number,
     * all parameter columns are cropped to the new number.
     *
     * If the new number of events is zero, all parameter columns are
     * truncated to zero events.
     *
     * @param[in] numberOfEvents
     *   Sets the number of events.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     */
    virtual void resize( const size_t numberOfEvents )
        noexcept = 0;

    /**
     * Sets the number of events in an original table, such as a file.
     *
     * When an event table is loaded from a file, and only a subset of
     * events are loaded, then the value set here is the total number
     * of events in that file.
     *
     * @param[in] numberOfEvents
     *   Sets the number of original events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfOriginalEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     */
    virtual void setNumberOfOriginalEvents( const size_t numberOfEvents )
        noexcept = 0;
    // @}



//----------------------------------------------------------------------
// Clear and copy.
//----------------------------------------------------------------------
public:
    /**
     * @name Clear and copy
     */
    // @{
    /**
     * Clears all parameter values to zeroes.
     *
     * The number of parameters and events remains unchanged.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     */
    virtual void clear( )
        noexcept = 0;

    /**
     * Overwrites this table with parameters and events copied from another
     * event table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     */
    virtual void copy( const std::shared_ptr<const EventTableInterface> eventTable ) = 0;

    /**
     * Overwrites this table with parameters and events copied from another
     * event table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     */
    virtual void copy( const EventTableInterface*const eventTable ) = 0;

    /**
     * Overwrites this table with parameters and events copied from another
     * event table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     */
    virtual void copy( const EventTableInterface& eventTable )
        noexcept = 0;

    /**
     * Overwrites the values of this table's selected parameter with event
     * values copied from another event table.
     *
     * @param[in] sourceEventTable
     *   The event table to copy.
     * @param[in] sourceIndex
     *   The parameter index to copy.
     * @param[in] index
     *   The parameter index in this table to overwrite.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     * @throw std::out_of_range
     *   Throws an exception if either index is out of range.
     */
    virtual void copyValues(
        const std::shared_ptr<const EventTableInterface> sourceEventTable,
        const uint32_t sourceIndex,
        const uint32_t index ) = 0;

    /**
     * Overwrites the values of this table's selected parameter with event
     * values copied from another event table.
     *
     * @param[in] sourceEventTable
     *   The event table to copy.
     * @param[in] sourceIndex
     *   The parameter index to copy.
     * @param[in] index
     *   The parameter index in this table to overwrite.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     * @throw std::out_of_range
     *   Throws an exception if either index is out of range.
     */
    virtual void copyValues(
        const EventTableInterface*const sourceEventTable,
        const uint32_t sourceIndex,
        const uint32_t index ) = 0;

    /**
     * Overwrites the values of this table's selected parameter with event
     * values copied from another event table.
     *
     * @param[in] sourceEventTable
     *   The event table to copy.
     * @param[in] sourceIndex
     *   The parameter index to copy.
     * @param[in] index
     *   The parameter index in this table to overwrite.
     *
     * @throw std::out_of_range
     *   Throws an exception if either index is out of range.
     */
    virtual void copyValues(
        const EventTableInterface& sourceEventTable,
        const uint32_t sourceIndex,
        const uint32_t index ) = 0;
    // @}



//----------------------------------------------------------------------
// Parameter values.
//----------------------------------------------------------------------
public:
    /**
     * @name Parameter values
     */
    // @{
    /**
     * Computes and saves the parameter data minimum and maximum.
     *
     * The data minimum and maximum are the largest and smallest values
     * within a parameter column. These may differ from the specified
     * maximum and minimum, which are the largest and smallest values
     * that can be generated by the original acquisition hardware or
     * software.
     *
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual void computeParameterDataMinimumMaximum( ) = 0;

    /**
     * Computes and saves the parameter minimum and maximum.
     *
     * The data minimum and maximum are the largest and smallest values
     * within a parameter column. These may differ from the specified
     * maximum and minimum, which are the largest and smallest values
     * that can be generated by the original acquisition hardware or
     * software.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual void computeParameterDataMinimumMaximum( const uint32_t index ) = 0;

    /**
     * Returns the data maximum for the parameter.
     *
     * The data maximum is the largest value within the parameter column.
     * This may differ from the specified maximum, which is the largest value
     * that can be generated by the original acquisition hardware or software.
     *
     * The data maximum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the data maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual double getParameterDataMaximum( const uint32_t index )
        const = 0;

    /**
     * Returns the data minimum for the parameter.
     *
     * The data minimum is the largest value within the parameter column.
     * This may differ from the specified minimum, which is the smallest value
     * that can be generated by the original acquisition hardware or software.
     *
     * The minimum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the data minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::getParameterDataMaximum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual double getParameterDataMinimum( const uint32_t index )
        const = 0;

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not doubles.
     */
    virtual const std::vector<double>& getParameterDoubles( const uint32_t index )
        const = 0;

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not doubles.
     */
    virtual std::vector<double>& getParameterDoubles( const uint32_t index ) = 0;

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not floats.
     */
    virtual const std::vector<float>& getParameterFloats( const uint32_t index )
        const = 0;

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not floats.
     */
    virtual std::vector<float>& getParameterFloats( const uint32_t index ) = 0;

    /**
     * Returns the best specified maximum for the parameter.
     *
     * An event table has two types of parameter minimum and maximum:
     *
     * - The min/max specified in a source file as the min/max that can be
     *   generated by the original acquisition hardware or software.
     *
     * - The min/max of the actual data values in the parameter column of
     *   the event table.
     *
     * This method returns the "best" value by getting the specified and
     * data values. If the specified maximum is less than the data maximum,
     * then the specified maximum is probably wrong and the data maximum
     * is returned. Otherwise the specified maximum is returned.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the "best" maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMinimum()
     * @see ::getParameterMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterBestMaximum( const uint32_t index )
        const = 0;

    /**
     * Returns the best minimum value for the parameter.
     *
     * An event table has two types of parameter minimum and maximum:
     *
     * - The min/max specified in a source file as the min/max that can be
     *   generated by the original acquisition hardware or software.
     *
     * - The min/max of the actual data values in the parameter column of
     *   the event table.
     *
     * This method returns the "best" value by getting the specified and
     * data values. If the specified minimum is greater than the data minimum,
     * then the specified minimum is probably wrong and the data minimum
     * is returned. Otherwise the specified minimum is returned.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the "best" minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMinimum()
     * @see ::getParameterMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterBestMinimum( const uint32_t index )
        const = 0;

    /**
     * Returns the specified maximum for the parameter.
     *
     * The specified maximum is the maximum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the data maximum, which is the maximum value in the parameter column.
     *
     * The specified maximum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the specified maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMinimum()
     * @see ::getParameterBestMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterMaximum( const uint32_t index )
        const = 0;

    /**
     * Returns the minimum value for the parameter.
     *
     * The specified minimum is the minimum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the data minimum, which is the minimum value in the parameter column.
     *
     * The specified minimum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the specified minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterMinimum( const uint32_t index )
        const = 0;

    /**
     * Sets the data maximum for the parameter.
     *
     * The data maximum is the maximum value stored in the parameter. This
     * may differ from the specified maximum, which is the maximum value
     * that can be generated by the original acquisition hardware or software.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] data maximum
     *   The data maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMinimum()
     */
    virtual void setParameterDataMaximum( const uint32_t index, const double maximum ) = 0;

    /**
     * Sets the data minimum for the parameter.
     *
     * The data minimum is the minimum value stored in the parameter. This
     * may differ from the specified minimum, which is the minimum value
     * that can be generated by the original acquisition hardware or software.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] minimum
     *   The data minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     */
    virtual void setParameterDataMinimum( const uint32_t index, const double minimum ) = 0;

    /**
     * Sets the specified maximum for the parameter.
     *
     * The specified maximum is the maximum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the maximum value currently stored for the parameter.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] maximum
     *   The specified maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMaximum()
     * @see ::getParameterMinimum()
     * @see ::setParameterMinimum()
     */
    virtual void setParameterMaximum( const uint32_t index, const double maximum ) = 0;

    /**
     * Sets the specified minimum for the parameter.
     *
     * The specified minimum is the minimum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the minimum value currently stored for the parameter.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] minimum
     *   The specified minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMaximum()
     * @see ::getParameterMinimum()
     * @see ::setParameterMaximum()
     */
    virtual void setParameterMinimum( const uint32_t index, const double minimum ) = 0;
    // @}



//----------------------------------------------------------------------
// Compensation
//----------------------------------------------------------------------
public:
    /**
     * @name Compensation
     */
    // @{
    /**
     * Performs fluorescence compensation on selected event parameters.
     *
     * Emission spectral overlap of the fluorescence labels during data
     * acquisition requires that the acquired data be corrected, or
     * "compensated" to reduce the effects of this overlap, or "spillover".
     *
     * Compensation sweeps through all events in the table and updates
     * the values of indicated parameters. Values for parameters not listed
     * in the matrix are left unchanged.
     *
     * The compensation matrix and parameters are typically found within
     * either an FCS file's spillover matrix or within a Gating-ML file's
     * compensation description.
     *
     * @param[in] matrixParameterNames
     *   The list of parameter names for rows and columns in the comensation
     *   matrix.
     * @param[in] matrix
     *   The values for a square compensation matrix.
     *
     * @throw std::invalid_argument
     *   Throws an exeption if the number of parameters is less than two,
     *   or if the matrix size is not equal to (n*n), where n is the number
     *   of parameters. An exception is also thrown if the parameter list
     *   names an unknown parameter, or if the compensation matrix includes
     *   an invalid value, such as a zero on the diagonal.
     */
    virtual void compensate(
        const std::vector<std::string>& matrixParameterNames,
        const std::vector<double>& matrix ) = 0;
    // @}
};





/**
 * Manages an event table composed of named parameter columns.
 *
 * An event table stores floating-point values collected by flow cytometery
 * hardware and software, and/or through further post-processing. Values
 * are typically loaded from an event file, such as one in the FCS format
 * standardized by the International Society for Advancement of Cytometry
 * (ISAC).
 *
 * Event values are typically used column-by-column, such as to sweep through
 * all values in a parameter column to apply a transform to apply gating.
 * To aid the performance of column-oriented operations, event data is
 * organized in columns so that consecutive values for the same parameter
 * are in consecutive locations in memory. This enables fast sweeps through
 * all values in a column and maximizes memory cache efficiency.
 *
 * Columns are managed as a table of side-by-side columns. Every column
 * has a unique numeric column index. The first column in the table has
 * an index of 0, the second is 1, and so forth.
 *
 * All parameter columns have the same number of event values. All columns
 * have the same single- or double-precision floating-point data type.
 *
 *
 * <B>Creating an event table</B><BR>
 * Constructors create an event table, given the number and names of all
 * parameter columns and the number of events for each column.
 *
 * @code
 * // Create a new event table.
 * std::vector<std::string> names;
 * names.push_back( "Parameter 1" );
 * names.push_back( "Parameter 2" );
 *
 * const size_t nEvents = 1000;
 * const bool useFloats = true;
 *
 * auto table = new EventTable( names, nEvents, useFloats );
 * @endcode
 *
 *
 * <B>Getting event table attributes</B><BR>
 * The number of parameters and events, and the table's data type may be
 * queried:
 *
 * @code
 * auto numberOfParameters = table->getNumberOfParameters( );
 * auto numberOfEvents = table->getNumberOfEvents( );
 * auto useFloats = table->areValuesFloats( );
 * @endcode
 *
 *
 * <B>Getting and setting parameter names</B><BR>
 * The names of parameters may be queried and set:
 *
 * @code
 * // Get and print all parameter names.
 * for ( uint32_t i = 0; i < numberOfParameters; ++i )
 *   std::cout << table->getParameterName(i) << std::endl;
 *
 * // Set all parameter names.
 * for ( uint32_t i = 0; i < numberOfParameters; ++i )
 *   table->setParameterName( i, std::string( "Parameter " ) + std::to_string( i ) );
 * @endcode
 *
 *
 * <B>Adding and removing parameters</B><BR>
 * Parameters may be added and removed. Adding a parameter initializes all
 * of the parameter's values to zero. Parameter names must be unique within
 * the table.
 *
 * @code
 * // Add parameter with new name.
 * auto index = table->appendParameter( "my parameter" );
 *
 * // Remove parameter by index.
 * table->removeParameter( index );
 *
 * // Remove parameter by name.
 * table->removeParameter( "my parameter" );
 * @endcode
 *
 *
 * <B>Getting and setting parameter values</B><BR>
 * The table contains a list of parameters and each parameter has a list
 * of values. All values of all parameters in the table have the same
 * single- or double-precision floating point data type.
 *
 * @code
 * auto nEvents = table->getNumberOfEvents( );
 * auto nParameters = table->getNumberOfParameters( );
 *
 * // Loop over all parameters and values.
 * if ( table->areValuesFloats( ) == true )
 * {
 *   for (uint32_t j = 0; j < nParameters; ++j)
 *   {
 *     auto values = table->getParameterFloats(j);
 *     for (size_t i = 0; i < nEvents; ++i)
 *     {
 *       auto value = values[i];
 *       ...
 *     }
 *   }
 * }
 * else
 * {
 *   for (uint32_t j = 0; j < nParameters; ++j)
 *   {
 *     auto values = table->getParameterDoubles(j);
 *     for (size_t i = 0; i < nEvents; ++i)
 *     {
 *       auto value = values[i];
 *       ...
 *     }
 *   }
 * }
 * @endcode
 *
 * Individual values may be set in a parameter's vector of values.
 *
 * @code
 * auto values = table->getParameterFloats(j);
 * values[i] = 123.45;
 * @endcode
 *
 *
 * <B>Multi-threading</B><BR>
 * If OpenMP is in use, the compensate() method uses multiple threads.
 *
 * All other class methods are presumed to be executed on a single thread,
 * or in a thread-safe manner. No thread locking is used in this class.
 *
 *
 * @internal
 * This class is implemented using techniques to improve performance:
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
class EventTable
    : public EventTableInterface
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate Event Tables";

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
        "EventTable";



    // Error messages --------------------------------------------------
    /**
     * The error message 1st line for a programmer error.
     */
    inline static const std::string ERROR_PROGRAMMER =
        "Programmer error.\n";



//----------------------------------------------------------------------
// Inner Classes.
//----------------------------------------------------------------------
protected:
    /**
     * Manages a column of parameter values.
     *
     * Each parameter column has a name and an array of float or double
     * values.
     *
     * For performance reasons, this class's methods do no error checking.
     * The caller is presumed to have checked the validity of values
     * before calling this class's methods.
     */
    class ParameterColumn
    {
    // Fields ----------------------------------------------------------
    public:
        /**
         * The parameter's primary (short) name.
         *
         * In normal use, the parameter name is never empty. However,
         * an uninitialized parameter column will have an empty name.
         *
         * Names must be unique among all parameters in a table.
         *
         * @see ::getParameterName()
         */
        std::string name;

        /**
         * The parameter's long name, if any.
         *
         * Long names do not need to be unique and they may be empty.
         *
         * @see ::getParameterLongName( )
         */
        std::string longName;

        /**
         * A boolean indicating if the parameter's values are single-precision
         * floating point (true) or double-precision floating point (false).
         *
         * When true, the floats field has values. When false, the doubles
         * field has values.
         *
         * @see ::areFloats()
         */
        bool useFloats;

        /**
         * The parameter's column of event values as floats.
         *
         * When useFloats is true, this vector holds the parameter's values.
         * When false, this vector is empty.
         *
         * @see ::getFloat()
         * @see ::setFloat()
         * @see ::getFloats()
         *
         * @internal
         * A std::vector is used to manage an array of values. Methods on
         * the EventTable class can return a reference to this vector,
         * and external event processing functions can sweep through the
         * vector simply.
         *
         * Use of a std::vector instead of a simple array gives us several
         * useful features:
         *
         * @li The vector can be easily resized if more events are added to
         *     the table. The vector class handles re-allocation and copying
         *     of values from old to new internal arrays.
         *
         * @li The vector automatically deletes its internal array when the
         *     parameter column is deleted. No special delete code is required.
         *
         * For better performance, external event processing functions may
         * call the vector's data() method to get a pointer to the vector's
         * internal array. Accesses to that array then bypass the method call
         * overhead of std::vector.
         * @endinternal
         */
        std::vector<float> floats;

        /**
         * The parameter's column of event values as doubles.
         *
         * When useFloats is false, this vector holds the parameter's values.
         * When true, this vector is empty.
         *
         * @see ::getDouble()
         * @see ::setDouble()
         * @see ::getDoubles()
         *
         * @internal
         * See notes for the floats field.
         * @endinternal
         */
        std::vector<double> doubles;

        /**
         * The specified maximum for event values as floats or doubles.
         *
         * The specified maximum is usually that specified during data
         * acquisition and it depends upon the hardware and software used.
         * The actual data maximum of values within the parameter column should
         * be less than or equal to this, but this is not enforced.
         */
        double maximum = 0.0;

        /**
         * The specified minimum for event values as floats or doubles.
         *
         * The specified minimum is usually that specified during data
         * acquisition and it depends upon the hardware and software used.
         * The actual data minimum of values within the parameter column should
         * be greater than or equal to this, but this is not enforced.
         */
        double minimum = 0.0;

        /**
         * The data maximum for event values as floats or doubles.
         *
         * The data maximum is the actual largest value found in the parameter
         * column when the maximum was computed. This should be less than or
         * equal to the specified maximum, but this is not enforced.
         */
        double dataMaximum = 0.0;

        /**
         * The data minimum for event values as floats or doubles.
         *
         * The data minimum is the actual largest value found in the parameter
         * column when the minimum was computed. This should be greater than or
         * equal to the specified minimum, but this is not enforced.
         */
        double dataMinimum = 0.0;

    // Constructors ----------------------------------------------------
    public:
        /**
         * Constructs an uninitialized parameter column.
         *
         * This constructor is used when creating empty entries in a
         * vector of parameters.
         */
        ParameterColumn( )
            noexcept
        {
            this->useFloats = true;
        }

    // Methods ---------------------------------------------------------
    public:
        /**
         * Returns the parameter's data type.
         *
         * @return
         *   When true, the parameter is using single-precision floating point.
         */
        inline bool areFloats( )
            const noexcept
        {
            return this->useFloats;
        }

        /**
         * Clears the parameter column's values to zeroes.
         */
        inline void clear( )
            noexcept
        {
            if ( this->useFloats == true )
                this->floats.assign( this->floats.size( ), 0 );
            else
                this->doubles.assign( this->doubles.size( ), 0 );
            this->minimum = this->maximum = 0.0;
            this->dataMinimum = this->dataMaximum = 0.0;
        }

        /**
         * Computes the data minimum and maximum for the parameter column's
         * values.
         *
         * The computed data minimum and maximum are saved into the parameter.
         * This does not affect the specified minimum and maximum, which are
         * typically set based upon the data acquisition hardware and software,
         * not on the values stored.
         *
         * @see ::getDataMinimum()
         * @see ::getDataMaximum()
         * @see ::setDataMinimum()
         * @see ::setDataMaximum()
         */
        inline void computeDataMinimumMaximum( )
            noexcept
        {
            if ( this->useFloats == true )
            {
                const size_t n = this->floats.size( );
                if ( n == 0 )
                {
                    // There is no data. Set the data min/max to zero.
                    this->dataMinimum = 0.0;
                    this->dataMaximum = 0.0;
                    return;
                }

                double maximum = (double) this->floats[0];
                double minimum = maximum;
                for ( size_t i = 1; i < n; ++i )
                {
                    const double value = (double) this->floats[i];
                    if ( value < minimum )
                        minimum = value;
                    if ( value > maximum )
                        maximum = value;
                }
                this->dataMinimum = minimum;
                this->dataMaximum = maximum;
            }
            else
            {
                const size_t n = this->doubles.size( );
                if ( n == 0 )
                {
                    // There is no data. Set the data min/max to zero.
                    this->dataMinimum = 0.0;
                    this->dataMaximum = 0.0;
                    return;
                }

                double maximum = this->doubles[0];
                double minimum = maximum;
                for ( size_t i = 1; i < n; ++i )
                {
                    const double value = this->doubles[i];
                    if ( value < minimum )
                        minimum = value;
                    if ( value > maximum )
                        maximum = value;
                }
                this->dataMinimum = minimum;
                this->dataMaximum = maximum;
            }
        }

        /**
         * Copies the parameter column to this column, changing its name,
         * data type, and values.
         *
         * @param[in] parameter
         *   The parameter to copy.
         */
        inline void copy( const ParameterColumn& parameter )
            noexcept
        {
            this->name = parameter.name;
            this->longName = parameter.longName;
            this->useFloats = parameter.useFloats;
            if ( this->useFloats == true )
            {
                this->floats = parameter.floats;
                this->doubles.clear( );
            }
            else
            {
                this->doubles = parameter.doubles;
                this->floats.clear( );
            }
            this->floats.shrink_to_fit( );
            this->doubles.shrink_to_fit( );
            this->minimum = parameter.minimum;
            this->maximum = parameter.maximum;
            this->dataMinimum = parameter.dataMinimum;
            this->dataMaximum = parameter.dataMaximum;
        }

        /**
         * Returns the best specified maximum for the parameter.
         *
         * An event table has two types of parameter minimum and maximum:
         *
         * - The min/max specified in a source file as the min/max that can be
         *   generated by the original acquisition hardware or software.
         *
         * - The min/max of the actual data values in the parameter column of
         *   the event table.
         *
         * This method returns the "best" value by getting the specified and
         * data values:
         *
         * - If there are no specified min/max for the parameter, then the
         *   data maximum is returned:
         *
         * - If the data maximum is greater than the specified maximum, then
         *   the specified maximum is assumed to be out of date and the
         *   data maximum is returned.
         *
         * - Otherwise, the specified maximum is returned.
         *
         * @return
         *   Returns the best maximum.
         *
         * @see ::getBestMinimum()
         * @see ::getDataMaximum()
         * @see ::getMaximum()
         * @see ::setMinimum()
         * @see ::setMaximum()
         */
        inline double getBestMaximum( )
            const noexcept
        {
            if ( this->minimum == this->maximum ||
                 this->dataMaximum > this->maximum )
                return this->dataMaximum;

            return this->maximum;
        }

        /**
         * Returns the best minimum value for the parameter.
         *
         * An event table has two types of parameter minimum and maximum:
         *
         * - The min/max specified in a source file as the min/max that can be
         *   generated by the original acquisition hardware or software.
         *
         * - The min/max of the actual data values in the parameter column of
         *   the event table.
         *
         * This method returns the "best" value by getting the specified and
         * data values:
         *
         * - If there are no specified min/max for the parameter, then the
         *   data minimum is returned:
         *
         * - If the data minimum is greater than the specified minimum, then
         *   the specified minimum is assumed to be out of date and the
         *   data minimum is returned.
         *
         * - Otherwise, the specified minimum is returned.
         *
         * @return
         *   Returns the best minimum.
         *
         * @see ::getBestMaximum()
         * @see ::getDataMinimum()
         * @see ::getMinimum()
         * @see ::setMinimum()
         * @see ::setMinimum()
         */
        inline double getBestMinimum( )
            const noexcept
        {
            if ( this->minimum == this->maximum ||
                 this->dataMinimum < this->minimum )
                return this->dataMinimum;

            return this->minimum;
        }

        /**
         * Returns the data maximum value for the parameter.
         *
         * The data maximum must have been set earlier to the maximum
         * value stored in the parameter column. This may differ from
         * the specified maximum, which is the maximum value that could
         * be generated by the original acquisition hardware and software.
         *
         * Typically, the data minimum and maximum are set by
         * computeDataMinimumMaximum(), which sweeps through the parameter's
         * data to calcuate the right values. If this has not been done,
         * or the data minimum and maximum have not been set explicitly,
         * then the data minimum and maximum are invalid are are both
         * zeroes.
         *
         * @return
         *   Returns the data maximum.
         *
         * @see ::getDataMinimum()
         * @see ::getMinimum()
         * @see ::setDataMinimum()
         * @see ::setDataMaximum()
         * @see ::setMinimum()
         * @see ::setMaximum()
         * @see ::computeDataMinimumMaximum()
         */
        inline double getDataMaximum( )
            const noexcept
        {
            return this->dataMaximum;
        }

        /**
         * Returns the data minimum value for the parameter.
         *
         * The data minimum must have been set earlier to the minimum
         * value stored in the parameter column. This may differ from
         * the specified minimum, which is the minimum value that could
         * be generated by the original acquisition hardware and software.
         *
         * Typically, the data minimum and maximum are set by
         * computeDataMinimumMaximum(), which sweeps through the parameter's
         * data to calcuate the right values. If this has not been done,
         * or the data minimum and maximum have not been set explicitly,
         * then the data minimum and maximum are invalid are are both
         * zeroes.
         *
         * @return
         *   Returns the data minimum.
         *
         * @see ::getDataMaximum()
         * @see ::getMaximum()
         * @see ::setDataMaximum()
         * @see ::setDataMinimum()
         * @see ::setMaximum()
         * @see ::setMinimum()
         * @see ::computeDataMinimumMaximum()
         */
        inline double getDataMinimum( )
            const noexcept
        {
            return this->dataMinimum;
        }

        /**
         * Returns a single parameter value.
         *
         * @param[in] index
         *   The event index. No bounds checking is done.
         *
         * @return
         *   Returns the value.
         *
         * @see ::getDoubles()
         */
        inline float getDouble( const size_t index )
            const noexcept
        {
            return this->doubles[index];
        }

        /**
         * Returns a reference the parameter's vector of doubles.
         *
         * @return
         *   Returns a reference.
         *
         * @see ::getDouble()
         */
        inline const std::vector<double>& getDoubles( )
            const noexcept
        {
            return this->doubles;
        }

        /**
         * Returns a reference the parameter's vector of doubles.
         *
         * @return
         *   Returns a reference.
         *
         * @see ::getDouble()
         */
        inline std::vector<double>& getDoubles( )
            noexcept
        {
            return this->doubles;
        }

        /**
         * Returns a single parameter value.
         *
         * @param[in] index
         *   The event index. No bounds checking is done.
         *
         * @return
         *   Returns the value.
         *
         * @see ::getFloats()
         */
        inline float getFloat( const size_t index )
            const noexcept
        {
            return this->floats[index];
        }

        /**
         * Returns a reference the parameter's vector of floats.
         *
         * @return
         *   Returns a reference.
         *
         * @see ::getFloat()
         */
        inline const std::vector<float>& getFloats( )
            const noexcept
        {
            return this->floats;
        }

        /**
         * Returns a reference the parameter's vector of flaots.
         *
         * @return
         *   Returns a reference.
         *
         * @see ::getFloat()
         */
        inline std::vector<float>& getFloats( )
            noexcept
        {
            return this->floats;
        }

        /**
         * Returns a reference to the parameter's long name.
         *
         * @return
         *   Returns a reference to the name. The name may be empty if the
         *   parameter has no long name.
         *
         * @see ::getParameterName()
         */
        inline const std::string& getLongName( )
            const noexcept
        {
            return this->longName;
        }

        /**
         * Returns the specified maximum value for the parameter.
         *
         * The specified maximum is the maximum value that could be generated
         * by the original data acquisition hardware or software. This could
         * be different from the maximum value currently in the parameter
         * column.
         *
         * @return
         *   Returns the specified maximum.
         *
         * @see ::getDataMinimum()
         * @see ::getMinimum()
         * @see ::setMinimum()
         * @see ::setMaximum()
         */
        inline double getMaximum( )
            const noexcept
        {
            return this->maximum;
        }

        /**
         * Returns the minimum value for the parameter.
         *
         * The specified minimum is the minimum value that could be generated
         * by the original data acquisition hardware or software. This could
         * be different from the minimum value currently in the parameter
         * column.
         *
         * @return
         *   Returns the minimum.
         *
         * @see ::getDataMinimum()
         * @see ::getMinimum()
         * @see ::setMinimum()
         * @see ::setMinimum()
         */
        inline double getMinimum( )
            const noexcept
        {
            return this->minimum;
        }

        /**
         * Returns a reference to the parameter's primary (short) name.
         *
         * @return
         *   Returns a reference to the name. The name is only empty when
         *   the parameter has not yet been initialized.
         */
        inline const std::string& getName( )
            const noexcept
        {
            return this->name;
        }

        /**
         * Returns the number of parameter values.
         *
         * @return
         *   Returns the number of parameter values.
         */
        inline size_t getNumberOfValues( )
            const noexcept
        {
            if ( this->useFloats == true )
                return this->floats.size( );
            return this->doubles.size( );
        }

        /**
         * Empties the parameter column of all values.
         *
         * @see ::clear()
         * @see ::resize()
         */
        inline void reset( )
            noexcept
        {
            this->floats.clear( );
            this->doubles.clear( );
        }

        /**
         * Resizes the parameter column.
         *
         * If the size is increased, new zero values are added to the
         * end of the column. If the size is decreased, the list is
         * truncated.
         *
         * @param[in] newSize
         *   The new size of the column.
         *
         * @see ::clear()
         * @see ::reset()
         */
        inline void resize( const size_t newSize )
        {
            if ( this->useFloats == true )
                this->floats.resize( newSize, 0.0f );
            else
                this->doubles.resize( newSize, 0.0 );
        }

        /**
         * Sets the data maximum value for the parameter.
         *
         * The data maximum is the maximum value in the parameter column.
         * This may differ from the specified maximum, which is the maximum
         * value that could be generated by the original acquisition hardware
         * and software.
         *
         * No error checking is done on the value. It is presumed to be valid.
         *
         * Typically, the data minimum and maximum are set by
         * computeDataMinimumMaximum(), which sweeps through the parameter's
         * data to calcuate the right values. If this has not been done,
         * or the data minimum and maximum have not been set explicitly,
         * then the data minimum and maximum are invalid are are both
         * zeroes.
         *
         * @param[in] maximum
         *   Sets the data maximum.
         *
         * @see ::getDataMaximum()
         * @see ::getDataMinimum()
         * @see ::setDataMinimum()
         * @see ::computeDataMinimumMaximum()
         */
        inline void setDataMaximum( const double maximum )
            noexcept
        {
            this->dataMaximum = maximum;
        }

        /**
         * Sets the data minimum value for the parameter.
         *
         * The data minimum is the minimum value in the parameter column.
         * This may differ from the specified minimum, which is the minimum
         * value that could be generated by the original acquisition hardware
         * and software.
         *
         * No error checking is done on the value. It is presumed to be valid.
         *
         * Typically, the data minimum and maximum are set by
         * computeDataMinimumMaximum(), which sweeps through the parameter's
         * data to calcuate the right values. If this has not been done,
         * or the data minimum and maximum have not been set explicitly,
         * then the data minimum and maximum are invalid are are both
         * zeroes.
         *
         * @param[in] minimum
         *   Sets the data minimum.
         *
         * @see ::getDataMaximum()
         * @see ::getDataMinimum()
         * @see ::setDataMaximum()
         * @see ::computeDataMinimumMaximum()
         */
        inline void setDataMinimum( const double minimum )
            noexcept
        {
            this->dataMinimum = minimum;
        }

        /**
         * Sets the parameter's data type.
         *
         * Setting the data type clears the parameter's values.
         *
         * @param[in] useFloats
         *   When true, use single-precision floating point.
         */
        inline void setDataType( const bool useFloats )
            noexcept
        {
            if ( this->useFloats == useFloats )
                return;
            this->useFloats = useFloats;
            this->reset( );
        }

        /**
         * Sets a single parameter value.
         *
         * @param[in] index
         *   The event index. No bounds checking is done.
         * @param[in] value
         *   The event value.
         *
         * @see ::getDouble()
         */
        inline void setDouble( const size_t index, const double value )
            noexcept
        {
            this->doubles[index] = value;
        }

        /**
         * Sets a single parameter value.
         *
         * @param[in] index
         *   The event index. No bounds checking is done.
         * @param[in] value
         *   The event value.
         *
         * @see ::getFloat()
         */
        inline void setFloat( const size_t index, const float value )
            noexcept
        {
            this->floats[index] = value;
        }

        /**
         * Sets the parameter's long name.
         *
         * The name may be empty and there is no requirement that it be unique.
         *
         * @param[in] name
         *   The new name.
         *
         * @see ::getName()
         */
        inline void setLongName( const std::string& name )
            noexcept
        {
            this->longName = name;
        }

        /**
         * Sets the specified maximum value for the parameter.
         *
         * The specified maximum is the maximum value that could be generated
         * by the original data acquisition hardware or software. This could
         * be different from the maximum value currently in the parameter
         * column.
         *
         * No error checking is done on the value. It is presumed to be valid.
         *
         * @param[in] maximum
         *   Sets the specified maximum.
         *
         * @see ::getMaximum()
         * @see ::getMinimum()
         * @see ::setMinimum()
         */
        inline void setMaximum( const double maximum )
            noexcept
        {
            this->maximum = maximum;
        }

        /**
         * Sets the specified minimum value for the parameter.
         *
         * The specified maximum is the maximum value that could be generated
         * by the original data acquisition hardware or software. This could
         * be different from the maximum value currently in the parameter
         * column.
         *
         * No error checking is done on the value. It is presumed to be valid.
         *
         * @param[in] minimum
         *   Sets the specified minimum.
         *
         * @see ::getMaximum()
         * @see ::getMinimum()
         * @see ::setMaximum()
         */
        inline void setMinimum( const double minimum )
            noexcept
        {
            this->minimum = minimum;
        }

        /**
         * Sets the parameter's primary (short) name.
         *
         * The name should not be empty and it should be unique within the
         * table containing the parameter. It is up to the caller to insure
         * this.
         *
         * @param[in] name
         *   The new name.
         *
         * @see ::getLongName()
         */
        inline void setName( const std::string& name )
            noexcept
        {
            this->name = name;
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



    // Event data ------------------------------------------------------
    /**
     * The number of events in the table.
     *
     * Every parameter column has this many values.
     *
     * @see ::getNumberOfEvents()
     */
    size_t numberOfEvents = 0;

    /**
     * The number of original events in the event source
     *
     * When an event table is loaded from a file, only a subset of the file's
     * events may be loaded. This method returns the total number of events
     * in that file.
     *
     * @see ::getNumberOfOriginalEvents()
     * @see ::setNumberOfOriginalEvents()
     */
    size_t numberOfOriginalEvents = 0;

    /**
     * The number of parameters per event.
     *
     * This is a cached copy of parameters.size().
     *
     * @see ::getNumberOfParameters()
     */
    uint32_t numberOfParameters = 0;

    /**
     * A boolean indicating if the parameter's values are single-precision
     * floating point (true) or double-precision floating point (false).
     *
     * When true, the floats field has values. When false, the doubles
     * field has values.
     *
     * @see ::areValuesFloats()
     */
    bool useFloats = true;

    /**
     * An array of parameters.
     *
     * Each parameter has a name and a list of event values.
     *
     * @see ::getParameterName()
     * @see ::getParameterIndex()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     */
    std::vector<ParameterColumn> parameters;

    /**
     * A boolean indicating if parameter names must be unique.
     *
     * Normally, each parameter in an event table must have a unique
     * name. These names are used in look-ups used by compensation
     * spillover matrices and by gate transforms.
     *
     * However, when an event table is subclassed for other uses (such
     * as a gated event table), parameter names may not be unique.
     */
    bool parameterNamesMustBeUnique = true;

    /**
     * A map of parameter names to parameter indexes.
     *
     * This map is used to accelerate the lookup of parameter names to
     * get a corresponding parameter index. This is only initialized
     * for event tables where parameter names must be unique.
     *
     * @see ::getParameterIndex()
     */
    std::map<std::string,uint32_t> parameterIndexByName;



//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
    /**
     * @name Constructors
     */
    // @{
    // General constructors --------------------------------------------
protected:
    /**
     * Constructs a new event table with the indicated parameter columns,
     * number of events, and event data type.
     *
     * @param[in] parameterNames
     *   A vector with the short parameter names for the event table's columns.
     * @param[in] numberOfEvents
     *   The number of events in the event table. If the number is zero, an
     *   empty table is created.
     * @param[in] useFloats
     *   (optional, default = true) When true, event parameter values are
     *   stored as single-precision floating-point values. When false, they
     *   are stored as double-precision floating-point values.
     * @param[in] parameterNamesMustBeUnique
     *   (optional, default = true) When true, all parameter names are
     *   presumed to be unique and are added into a search tree that maps
     *   names to their table column indexes. When false, parameter names
     *   are not necessarily unique and the index table is not initialized.
     *   This is used by subclasses that have alternate lookup mechanisms
     *   for parameter names.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter names list is empty, if any
     *   name is empty, or if the names are not all unique.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterNames()
     */
    EventTable(
        const std::vector<std::string>& parameterNames,
        const size_t numberOfEvents,
        const bool useFloats,
        const bool parameterNamesMustBeUnique )
    {
        //
        // Initialize.
        // -----------
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        this->numberOfParameters = parameterNames.size( );
        this->numberOfEvents     = numberOfEvents;
        this->useFloats          = useFloats;
        this->parameterNamesMustBeUnique = parameterNamesMustBeUnique;

        if ( this->numberOfParameters != 0 )
        {
            // Add all parameter columns, initialized to empty.
            this->parameters.resize( this->numberOfParameters );
            this->parameters.shrink_to_fit( );

            // Set each parameter's name.
            for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            {
                const std::string& name = parameterNames[i];

                if ( name.empty( ) == true )
                    throw std::out_of_range(
                        ERROR_PROGRAMMER +
                        "Invalid empty parameter name." );

                if ( parameterNamesMustBeUnique == true &&
                    this->parameterIndexByName.count( name ) != 0 )
                    throw std::out_of_range(
                        ERROR_PROGRAMMER +
                        "Parameter name is not unique within the event table." );

                this->parameters[i].setName( name );
                this->parameters[i].setDataType( useFloats );
                this->parameters[i].resize( numberOfEvents );

                if ( parameterNamesMustBeUnique == true )
                    this->parameterIndexByName[name] = i;
            }
        }
    }

public:
    /**
     * Constructs a new event table with the indicated parameter columns,
     * number of events, and event data type.
     *
     * @param[in] parameterNames
     *   A vector with the short parameter names for the event table's columns.
     * @param[in] numberOfEvents
     *   The number of events in the event table.
     * @param[in] useFloats
     *   (optional, default = true) When true, event parameter values are
     *   stored as single-precision floating-point values. When false, they
     *   are stored as double-precision floating-point values.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter names list is empty or the
     *   number of events is zero.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterNames()
     */
    EventTable(
        const std::vector<std::string>& parameterNames,
        const size_t numberOfEvents,
        const bool useFloats = true )
        : EventTable( parameterNames, numberOfEvents, useFloats, true )
    {
    }

    /**
     * Constructs a new event table initialized by copying another table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterNames()
     */
    EventTable( const EventTableInterface*const eventTable )
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( eventTable );
    }

    /**
     * Constructs a new event table initialized by copying another table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterNames()
     */
    EventTable( const EventTableInterface& eventTable )
        noexcept
    {
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->copy( eventTable );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~EventTable( )
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
    virtual std::string getVerbosePrefix( )
        const noexcept override final
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
    virtual bool isVerbose( )
        const noexcept override final
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
    virtual void setVerbose( const bool enable )
        noexcept override final
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
    virtual void setVerbosePrefix( const std::string prefix )
        noexcept override final
    {
        this->verbosePrefix = prefix;
    }
    // @}



//----------------------------------------------------------------------
// Parameters.
//----------------------------------------------------------------------
    /**
     * @name Parameters
     */
    // @{
public:
    /**
     * Appends a parameter column.
     *
     * @param[in] name
     *   The name of the new parameter. The name should differ from that
     *   of any other parameter already in the table.
     *
     * @return
     *   Returns the new parameter's index.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the name is empty or not unique, and
     *   unique names are required for the event table.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterIndex()
     */
    virtual uint32_t appendParameter( const std::string& name )
        override final
    {
        if ( name.empty( ) == true )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid empty parameter name." );

        if ( this->parameterNamesMustBeUnique == true &&
            this->parameterIndexByName.count( name ) != 0 )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter name is already in use in the event table." );

        const uint32_t index = this->numberOfParameters++;
        this->parameters.resize( this->numberOfParameters );
        this->parameters[index].setName( name );
        this->parameters[index].setDataType( this->useFloats );
        this->parameters[index].resize( this->numberOfEvents );
        if ( this->parameterNamesMustBeUnique == true )
            this->parameterIndexByName[name] = index;
        return index;
    }

    /**
     * Returns the number of parameter columns.
     *
     * @return
     *   Returns the number of parameter columns.
     *
     * @see ::getParameterName()
     */
    virtual uint32_t getNumberOfParameters( )
        const noexcept override final
    {
        return this->numberOfParameters;
    }

    /**
     * Returns the long name of the indicated parameter column.
     *
     * The long name of a parameter is optional and it may be empty.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   The long name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterName()
     * @see ::getParameterNames()
     */
    virtual const std::string& getParameterLongName( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getLongName( );
    }

    /**
     * Returns the primary (short) name of the indicated parameter column.
     *
     * Parameter names must not be empty and they must be unique within
     * the table.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   The name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterLongName()
     * @see ::getParameterNames()
     */
    virtual const std::string& getParameterName( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getName( );
    }

    /**
     * Returns the index of the named parameter.
     *
     * @param[in] name
     *   The parameter name.
     *
     * @return
     *   The parameter index.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter name is not found.
     *
     * @see ::getParameterName()
     * @see ::isParameter()
     */
    virtual uint32_t getParameterIndex( const std::string& name )
        const override final
    {
        try
        {
            // If parameter names are unique, then use a hash table to
            // get the parameter's index.
            if ( parameterNamesMustBeUnique == true )
                return this->parameterIndexByName.at( name );

            // When parameter names are not unique, look for the first
            // parameter with a matching name and return its index.
            for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            {
                if ( name.compare( this->parameters[i].getName( ) ) == 0 )
                    return i;
            }

            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter name not found." );
        }
        catch ( ... )
        {
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter name not found." );
        }
    }

    /**
     * Returns a list of parameter long names.
     *
     * Long names are optional. An empty string is returned in the name
     * list if the parameter has no long name.
     *
     * <B>Performance note:</B><BR>
     * Use of this method is discouraged.  This method returns a <B>copy</B>
     * of all parameter long names in a vector that is, itself, <B>copied</B>
     * when it is returned. In contrast, this simpler getParameterLongName()
     * method returns a single const <B>reference</B> to the stored
     * parameter name. This will be much faster since it requires no memory
     * allocation or copying.
     *
     * @return
     *   Returns a new vector containing copies of all parameter long names,
     *   in the same order as parameters in the event table.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     */
    virtual std::vector<std::string> getParameterLongNames( )
        const noexcept override final
    {
        std::vector<std::string> names;
        for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            names.push_back( this->parameters[i].getLongName( ) );
        return names;
    }

    /**
     * Returns a list of parameter names.
     *
     * <B>Performance note:</B><BR>
     * Use of this method is discouraged.  This method returns a <B>copy</B>
     * of all parameter names in a vector that is, itself, <B>copied</B>
     * when it is returned. In contrast, this simpler getParameterName()
     * method returns a single const <B>reference</B> to the stored
     * parameter name. This will be much faster since it requires no memory
     * allocation or copying.
     *
     * @return
     *   Returns a new vector containing copies of all parameter names,
     *   in the same order as parameters in the event table.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     */
    virtual std::vector<std::string> getParameterNames( )
        const noexcept override final
    {
        std::vector<std::string> names;
        for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            names.push_back( this->parameters[i].getName( ) );
        return names;
    }

    /**
     * Returns TRUE if named parameter exists in this table.
     *
     * @param[in] name
     *   The parameter name.
     *
     * @return
     *   TRUE if the parameter name exists.
     *
     * @see ::getParameterIndex()
     */
    virtual bool isParameter( const std::string& name )
        const override final
    {
        try
        {
            if ( parameterNamesMustBeUnique == true )
            {
                // When names must be unique, check the hash table.
                this->parameterIndexByName.at( name );
                return true;
            }

            // When names may not be unique, loop through the whole list.
            for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            {
                if ( name.compare( this->parameters[i].getName( ) ) == 0 )
                    return false;
            }
            return true;
        }
        catch ( ... )
        {
            return false;
        }
    }



    /**
     * Removes a parameter column.
     *
     * @param[in] name
     *   The parameter name.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter name is not found.
     */
    virtual void removeParameter( const std::string& name )
        override final
    {
        const auto index = this->getParameterIndex( name );
        this->removeParameter( index );
    }

    /**
     * Removes a parameter column.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    virtual void removeParameter( const uint32_t index )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        if ( this->numberOfParameters == 0 )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Cannot remove parameter from empty table." );

        if ( this->parameterNamesMustBeUnique == true )
            this->parameterIndexByName.erase(
                this->parameters[index].getName( ) );

        this->parameters.erase( this->parameters.begin() + index );
        this->parameters.shrink_to_fit( );
        --this->numberOfParameters;
    }

    /**
     * Sets the long name of the indicated parameter column.
     *
     * The long name of a parameter is optional and it may be empty.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] name
     *   The long name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::setParameterLongNames()
     * @see ::setParameterNames()
     */
    virtual void setParameterLongName(
        const uint32_t index,
        const std::string& name )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        this->parameters[index].setLongName( name );
    }

    /**
     * Sets all parameter long names.
     *
     * The long name of a parameter is optional and it may be empty.
     *
     * @param[in] names
     *   The vector of long names. There must be one entry in the vector
     *   for each parameter.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the parameter name vector is too small.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterLongNames()
     * @see ::setParameterLongName()
     */
    virtual void setParameterLongNames( const std::vector<std::string> names )
        override final
    {
        const uint32_t n = names.size( );
        if ( n < this->numberOfParameters )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Parameter long name list is too small." );
        for ( uint32_t i = 0; i < n; ++i )
            this->parameters[i].setLongName( names[i] );
    }

    /**
     * Sets the primary (short) name of the indicated parameter column.
     *
     * Parameter names must not be empty and they must be unique within
     * the table.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] name
     *   The name of the parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range, or
     *   the name is already in use by another parameter.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterNames()
     * @see ::setParameterNames()
     */
    virtual void setParameterName(
        const uint32_t index,
        const std::string& name )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );

        const std::string& oldName = this->parameters[index].getName( );
        if ( oldName == name )
            return;

        if ( this->parameterNamesMustBeUnique == true &&
            this->parameterIndexByName.count(name) != 0 )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter name is already in use in the event table." );

        if ( this->parameterNamesMustBeUnique == true )
            this->parameterIndexByName.erase( oldName );

        this->parameters[index].setName( name );

        if ( this->parameterNamesMustBeUnique == true )
            this->parameterIndexByName[name] = index;
    }

    /**
     * Sets all parameter primary (short) names.
     *
     * Parameter names must not be empty and they must be unique within
     * the table.
     *
     * @param[in] names
     *   The vector of long names. There must be one entry in the vector
     *   for each parameter.
     *
     * @throw std::out_of_range
     *   Throws an exception if a name is already in use by another parameter.
     * @throw std::invalid_argument
     *   Throws an exception if the parameter name vector is too small.
     *
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     * @see ::getParameterNames()
     * @see ::setParameterName()
     */
    virtual void setParameterNames( const std::vector<std::string> names )
        override final
    {
        const uint32_t n = names.size( );
        if ( n < this->numberOfParameters )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Parameter name list is too small." );
        for ( uint32_t i = 0; i < n; ++i )
            this->parameters[i].setName( names[i] );
    }
    // @}



//----------------------------------------------------------------------
// Event attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Event attributes
     */
    // @{
    /**
     * Returns true if events use single-precision floating point.
     *
     * The data type for events is set on the constructor and cannot be
     * changed afterwards.
     *
     * @return
     *   Returns true if events use single-precision floating point,
     *   and false if they use double-precision floating point.
     *
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::getNumberOfEvents()
     */
    virtual const bool areValuesFloats( )
        const noexcept override final
    {
        return this->useFloats;
    }

    /**
     * Returns the number of events.
     *
     * @return
     *   Returns the number of events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfOriginalEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     */
    virtual size_t getNumberOfEvents( )
        const noexcept override final
    {
        return this->numberOfEvents;
    }

    /**
     * Returns the number of events in an original table, such as a file.
     *
     * When an event table is loaded from a file, and only a subset of
     * events are loaded, then the value returned here is the total number
     * of events in that file.
     *
     * @return
     *   Returns the number of original events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     * @see ::setNumberOfOriginalEvents()
     */
    virtual size_t getNumberOfOriginalEvents( )
        const noexcept override final
    {
        if ( this->numberOfOriginalEvents < this->numberOfEvents )
            return this->numberOfEvents;
        return this->numberOfOriginalEvents;
    }



    /**
     * Sets the number of events.
     *
     * If the new number of events is larger than the current number,
     * additional events are added to all parameter columns and initialized
     * to zeroes.
     *
     * If the new number of events is smaller than the current number,
     * all parameter columns are cropped to the new number.
     *
     * If the new number of events is zero, all parameter columns are
     * truncated to zero events.
     *
     * @param[in] numberOfEvents
     *   Sets the number of events.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     */
    virtual void resize( const size_t numberOfEvents )
        noexcept override final
    {
        if ( numberOfEvents == this->numberOfEvents )
            return;

        for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            this->parameters[i].resize( numberOfEvents );
        this->numberOfEvents = numberOfEvents;
    }

    /**
     * Sets the number of events in an original table, such as a file.
     *
     * When an event table is loaded from a file, and only a subset of
     * events are loaded, then the value set here is the total number
     * of events in that file.
     *
     * @param[in] numberOfEvents
     *   Sets the number of original events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfOriginalEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     */
    virtual void setNumberOfOriginalEvents( const size_t numberOfEvents )
        noexcept override final
    {
        this->numberOfOriginalEvents = numberOfEvents;
    }
    // @}



//----------------------------------------------------------------------
// Clear and copy.
//----------------------------------------------------------------------
public:
    /**
     * @name Clear and copy
     */
    // @{
    /**
     * Clears all parameter values to zeroes.
     *
     * The number of parameters and events remains unchanged.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     */
    virtual void clear( )
        noexcept override final
    {
        for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            this->parameters[i].clear( );
    }

    /**
     * Overwrites this table with parameters and events copied from another
     * event table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     */
    virtual void copy(
        const std::shared_ptr<const EventTableInterface> eventTable )
        override final
    {
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        const auto et = std::dynamic_pointer_cast<const EventTable>( eventTable );
        if ( et == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Event table is not a known implementation." );

        this->numberOfParameters = et->numberOfParameters;
        this->numberOfEvents     = et->numberOfEvents;
        this->numberOfOriginalEvents = et->numberOfOriginalEvents;
        this->useFloats          = et->useFloats;
        this->parameterNamesMustBeUnique = et->parameterNamesMustBeUnique;

        this->parameters.clear( );
        this->parameters.resize( this->numberOfParameters );
        this->parameters.shrink_to_fit( );

        if ( this->parameterNamesMustBeUnique == true )
            this->parameterIndexByName = et->parameterIndexByName;

        for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            this->parameters[i].copy( et->parameters[i] );
    }

    /**
     * Overwrites this table with parameters and events copied from another
     * event table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     */
    virtual void copy( const EventTableInterface*const eventTable )
        override final
    {
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        const auto et = dynamic_cast<const EventTable*>( eventTable );
        if ( et == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Event table is not a known implementation." );

        this->numberOfParameters = et->numberOfParameters;
        this->numberOfEvents     = et->numberOfEvents;
        this->numberOfOriginalEvents = et->numberOfOriginalEvents;
        this->useFloats          = et->useFloats;
        this->parameterNamesMustBeUnique = et->parameterNamesMustBeUnique;

        this->parameters.clear( );
        this->parameters.resize( this->numberOfParameters );
        this->parameters.shrink_to_fit( );

        if ( this->parameterNamesMustBeUnique == true )
            this->parameterIndexByName = et->parameterIndexByName;

        for ( uint32_t i = 0; i < this->numberOfParameters; ++i )
            this->parameters[i].copy( et->parameters[i] );
    }

    /**
     * Overwrites this table with parameters and events copied from another
     * event table.
     *
     * @param[in] eventTable
     *   The event table to copy.
     *
     * @see ::areValuesFloats()
     * @see ::getParameterDoubles()
     * @see ::getParameterFloats()
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfParameters()
     * @see ::getParameterName()
     * @see ::getParameterLongName()
     */
    virtual void copy( const EventTableInterface& eventTable )
        noexcept override final
    {
        this->copy( &eventTable );
    }

    /**
     * Overwrites the values of this table's selected parameter with event
     * values copied from another event table.
     *
     * @param[in] sourceEventTable
     *   The event table to copy.
     * @param[in] sourceIndex
     *   The parameter index to copy.
     * @param[in] index
     *   The parameter index in this table to overwrite.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr, or if
     *   the source event table is not a known implementation of the
     *   EventTableInterface. Only known implementations provide the
     *   necessary direct access to parameter column values.
     * @throw std::out_of_range
     *   Throws an exception if either index is out of range.
     */
    virtual void copyValues(
        const std::shared_ptr<const EventTableInterface> sourceEventTable,
        const uint32_t sourceIndex,
        const uint32_t index )
        override final
    {
        if ( sourceEventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL source event table." );

        if ( sourceIndex >= sourceEventTable->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Source event table index is out of range." );

        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Event table index is out of range." );

        const auto et = std::dynamic_pointer_cast<const EventTable>( sourceEventTable );
        if ( et == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Event table is not a known implementation." );

        this->parameters[index].copy( et->parameters[sourceIndex] );
    }

    /**
     * Overwrites the values of this table's selected parameter with event
     * values copied from another event table.
     *
     * @param[in] sourceEventTable
     *   The event table to copy.
     * @param[in] sourceIndex
     *   The parameter index to copy.
     * @param[in] index
     *   The parameter index in this table to overwrite.
     *
     * @throw std::invalid_argument
     *   Throws an exception if the event table is a nullptr, or if
     *   the source event table is not a known implementation of the
     *   EventTableInterface. Only known implementations provide the
     *   necessary direct access to parameter column values.
     * @throw std::out_of_range
     *   Throws an exception if either index is out of range.
     */
    virtual void copyValues(
        const EventTableInterface*const sourceEventTable,
        const uint32_t sourceIndex,
        const uint32_t index )
        override final
    {
        if ( sourceEventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL source event table." );

        if ( sourceIndex >= sourceEventTable->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Source event table index is out of range." );

        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Event table index is out of range." );

        const auto et = dynamic_cast<const EventTable*>( sourceEventTable );
        if ( et == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Event table is not a known implementation." );

        this->parameters[index].copy( et->parameters[sourceIndex] );
    }

    /**
     * Overwrites the values of this table's selected parameter with event
     * values copied from another event table.
     *
     * @param[in] sourceEventTable
     *   The event table to copy.
     * @param[in] sourceIndex
     *   The parameter index to copy.
     * @param[in] index
     *   The parameter index in this table to overwrite.
     *
     * @throw std::out_of_range
     *   Throws an exception if either index is out of range.
     * @throw std::invalid_argument
     *   Throws an exception if the source event table is not a known
     *   implementation of the EventTableInterface. Only known implementations
     *   provide the necessary direct access to parameter column values.
     */
    virtual void copyValues(
        const EventTableInterface& sourceEventTable,
        const uint32_t sourceIndex,
        const uint32_t index )
        override final
    {
        this->copyValues( &sourceEventTable, sourceIndex, index );
    }
    // @}



//----------------------------------------------------------------------
// Parameter values.
//----------------------------------------------------------------------
public:
    /**
     * @name Parameter values
     */
    // @{
    /**
     * Computes and saves the parameter data minimum and maximum.
     *
     * The data minimum and maximum are the largest and smallest values
     * within a parameter column. These may differ from the specified
     * maximum and minimum, which are the largest and smallest values
     * that can be generated by the original acquisition hardware or
     * software.
     *
     * This method, or its related form, should be called explicitly
     * after data has been changed, such as after data has been loaded
     * from a file. Until it is called, the reported data minimum and
     * maximum will not be valid.
     *
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual void computeParameterDataMinimumMaximum( )
        override final
    {
        const uint32_t n = this->numberOfParameters;

#pragma omp parallel for schedule(static)
        for ( uint32_t i = 0; i < n; ++i )
            this->parameters[i].computeDataMinimumMaximum( );
    }

    /**
     * Computes and saves the parameter minimum and maximum.
     *
     * The data minimum and maximum are the largest and smallest values
     * within a parameter column. These may differ from the specified
     * maximum and minimum, which are the largest and smallest values
     * that can be generated by the original acquisition hardware or
     * software.
     *
     * This method, or its related form, should be called explicitly
     * after data has been changed, such as after data has been loaded
     * from a file. Until it is called, the reported data minimum and
     * maximum will not be valid.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual void computeParameterDataMinimumMaximum( const uint32_t index )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        this->parameters[index].computeDataMinimumMaximum( );
    }

    /**
     * Returns the best specified maximum for the parameter.
     *
     * An event table has two types of parameter minimum and maximum:
     *
     * - The min/max specified in a source file as the min/max that can be
     *   generated by the original acquisition hardware or software.
     *
     * - The min/max of the actual data values in the parameter column of
     *   the event table.
     *
     * This method returns the "best" value by getting the specified and
     * data values:
     *
     * - If there are no specified min/max for the parameter, then the
     *   data maximum is returned:
     *
     * - If the data maximum is greater than the specified maximum, then
     *   the specified maximum is assumed to be out of date and the
     *   data maximum is returned.
     *
     * - Otherwise, the specified maximum is returned.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the "best" maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMinimum()
     * @see ::getParameterMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterBestMaximum( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getBestMaximum( );
    }

    /**
     * Returns the best minimum value for the parameter.
     *
     * An event table has two types of parameter minimum and maximum:
     *
     * - The min/max specified in a source file as the min/max that can be
     *   generated by the original acquisition hardware or software.
     *
     * - The min/max of the actual data values in the parameter column of
     *   the event table.
     *
     * This method returns the "best" value by getting the specified and
     * data values:
     *
     * - If there are no specified min/max for the parameter, then the
     *   data minimum is returned:
     *
     * - If the data minimum is greater than the specified minimum, then
     *   the specified minimum is assumed to be out of date and the
     *   data minimum is returned.
     *
     * - Otherwise, the specified minimum is returned.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the "best" minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMinimum()
     * @see ::getParameterMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterBestMinimum( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getBestMinimum( );
    }

    /**
     * Returns the data maximum for the parameter.
     *
     * The data maximum is the largest value within the parameter column.
     * This may differ from the specified maximum, which is the largest value
     * that can be generated by the original acquisition hardware or software.
     *
     * The data maximum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the data maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual double getParameterDataMaximum( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getDataMaximum( );
    }

    /**
     * Returns the data minimum for the parameter.
     *
     * The data minimum is the largest value within the parameter column.
     * This may differ from the specified minimum, which is the smallest value
     * that can be generated by the original acquisition hardware or software.
     *
     * The minimum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the data minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterBestMaximum()
     * @see ::getParameterBestMinimum()
     * @see ::getParameterDataMaximum()
     * @see ::setParameterDataMaximum()
     * @see ::setParameterDataMinimum()
     */
    virtual double getParameterDataMinimum( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getDataMinimum( );
    }

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not doubles.
     */
    virtual const std::vector<double>& getParameterDoubles(
        const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        if ( this->parameters[index].areFloats( ) == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid request for double values on a non-double parameter." );
        return this->parameters[index].getDoubles( );
    }

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not doubles.
     */
    virtual std::vector<double>& getParameterDoubles( const uint32_t index )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        if ( this->parameters[index].areFloats( ) == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid request for double values on a non-double parameter." );
        return this->parameters[index].getDoubles( );
    }

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not floats.
     */
    virtual const std::vector<float>& getParameterFloats( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        if ( this->parameters[index].areFloats( ) == false )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid request for float values on a non-float parameter." );
        return this->parameters[index].getFloats( );
    }

    /**
     * Returns a reference to the indicated parameter's vector of values.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector reference.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the parameter's values are not floats.
     */
    virtual std::vector<float>& getParameterFloats( const uint32_t index )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        if ( this->parameters[index].areFloats( ) == false )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid request for float values on a non-float parameter." );
        return this->parameters[index].getFloats( );
    }

    /**
     * Returns the specified maximum for the parameter.
     *
     * The specified maximum is the maximum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the data maximum, which is the maximum value in the parameter column.
     *
     * The specified maximum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the specified maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMinimum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterMaximum( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getMaximum( );
    }

    /**
     * Returns the minimum value for the parameter.
     *
     * The specified minimum is the minimum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the data minimum, which is the minimum value in the parameter column.
     *
     * The specified minimum must have been set earlier.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the specified minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMaximum()
     * @see ::setParameterMaximum()
     * @see ::setParameterMinimum()
     */
    virtual double getParameterMinimum( const uint32_t index )
        const override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        return this->parameters[index].getMinimum( );
    }

    /**
     * Sets the data maximum for the parameter.
     *
     * The data maximum is the maximum value stored in the parameter. This
     * may differ from the specified maximum, which is the maximum value
     * that can be generated by the original acquisition hardware or software.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] data maximum
     *   The data maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMinimum()
     */
    virtual void setParameterDataMaximum( const uint32_t index, const double maximum )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        this->parameters[index].setDataMaximum( maximum );
    }

    /**
     * Sets the data minimum for the parameter.
     *
     * The data minimum is the minimum value stored in the parameter. This
     * may differ from the specified minimum, which is the minimum value
     * that can be generated by the original acquisition hardware or software.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] minimum
     *   The data minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::computeParameterDataMinimumMaximum()
     * @see ::getParameterDataMaximum()
     * @see ::getParameterDataMinimum()
     * @see ::setParameterDataMaximum()
     */
    virtual void setParameterDataMinimum( const uint32_t index, const double minimum )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        this->parameters[index].setDataMinimum( minimum );
    }

    /**
     * Sets the specified maximum for the parameter.
     *
     * The specified maximum is the maximum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the maximum value currently stored for the parameter.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] maximum
     *   The specified maximum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMaximum()
     * @see ::getParameterMinimum()
     * @see ::setParameterMinimum()
     */
    virtual void setParameterMaximum( const uint32_t index, const double maximum )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        this->parameters[index].setMaximum( maximum );
    }

    /**
     * Sets the specified minimum for the parameter.
     *
     * The specified minimum is the minimum value that can be generated by
     * the original acquisition hardware or software. This may differ from
     * the minimum value currently stored for the parameter.
     *
     * No error checking is done on the value. It is presumed to be valid.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] minimum
     *   The specified minimum.
     *
     * @throw std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterMaximum()
     * @see ::getParameterMinimum()
     * @see ::setParameterMaximum()
     */
    virtual void setParameterMinimum( const uint32_t index, const double minimum )
        override final
    {
        if ( index >= this->numberOfParameters )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );
        this->parameters[index].setMinimum( minimum );
    }
    // @}



//----------------------------------------------------------------------
// Compensation
//----------------------------------------------------------------------
public:
    /**
     * @name Compensation
     */
    // @{
    /**
     * Performs fluorescence compensation on selected event parameters.
     *
     * Emission spectral overlap of the fluorescence labels during data
     * acquisition requires that the acquired data be corrected, or
     * "compensated" to reduce the effects of this overlap, or "spillover".
     *
     * Compensation sweeps through all events in the table and updates
     * the values of indicated parameters. Values for parameters not listed
     * in the matrix are left unchanged.
     *
     * The compensation matrix and parameters are typically found within
     * either an FCS file's spillover matrix or within a Gating-ML file's
     * compensation description.
     *
     * @param[in] matrixParameterNames
     *   The list of parameter names for rows and columns in the comensation
     *   matrix.
     * @param[in] matrix
     *   The values for a square compensation matrix.
     *
     * @throw std::invalid_argument
     *   Throws an exeption if the number of parameters is less than two,
     *   or if the matrix size is not equal to (n*n), where n is the number
     *   of parameters. An exception is also thrown if the parameter list
     *   names an unknown parameter, or if the compensation matrix includes
     *   an invalid value, such as a zero on the diagonal.
     */
    virtual void compensate(
        const std::vector<std::string>& matrixParameterNames,
        const std::vector<double>& matrix )
        override final
    {
        //
        // Validate.
        // ---------
        // Let n be the number of parameter names given, which must be
        // at least 2. The matrix must contain n*n values. All parameter
        // names must correspond to parameters in the event table.
        const uint32_t n = matrixParameterNames.size( );
        if ( n < 2 )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid compensation matrix parameter name vector must have at least two entries." );
        if ( matrix.size( ) != n*n )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid compensation matrix must have n*n values where n is the number of parameters given." );

        //
        // Detect identity and problems.
        // -----------------------------
        // If the spillover matrix is identity, then there is no work to do.
        // If it has a zero on a diagonal, then the matrix is not valid and
        // cannot be inverted (a zero on a diagonal would mean that a parameter
        // value for an event has zero weight, which would be odd anyway).
        bool identity = true;
        for ( uint32_t i = 0; i < n && identity == true; ++i )
        {
            for ( uint32_t j = 0; j < n && identity == true; ++j )
            {
                const double value = matrix[i*n + j];
                if ( i == j )
                {
                    if ( value == 0.0 )
                        throw std::invalid_argument(
                            std::string( "Malformed compensation matrix.\n" ) +
                            "An invalid compensation matrix has a zero on the diagonal. The matrix cannot be inverted, and therefore cannot be used to compensate event data." );
                    if ( value != 1.0 )
                        identity = false;
                }
                else if ( value != 0.0 )
                    identity = false;
            }
        }

        if ( identity == true )
            return;


        // Get the parameter index associated with each parameter. If any
        // parameter name is not found, throw an exception.
        std::vector<uint32_t> matrixParameterIndexes;
        try
        {
            for ( uint32_t i = 0; i < n; ++i )
                matrixParameterIndexes.push_back(
                    this->getParameterIndex( matrixParameterNames[i] ) );
        }
        catch ( ... )
        {
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid compensation matrix parameter name vector contains a name that does not match any known parameter names." );
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": Compensating:\n";
            std::cerr << this->verbosePrefix << ":   Spillover matrix:\n";
            std::cerr << "  ";
            for ( uint32_t j = 0; j < n; ++j )
                std::cerr << std::setw(8) << matrixParameterNames[j];
            std::cerr << "\n";

            for ( uint32_t i = 0; i < n; ++i )
            {
                std::cerr << "  ";
                for ( uint32_t j = 0; j < n; ++j )
                {
                    std::cerr << std::setw(8) << std::setprecision(4) << std::fixed;
                    std::cerr << matrix[i*n + j] << " ";
                }
                std::cerr << "\n";
            }
        }


        //
        // Calculate inverse.
        // ------------------
        // While std::vector works as a dynamic array, accessing elements
        // requires method calls. This code avoids this overhead by
        // working directly with array pointers.
        //
        // The matrix inverse method only returns a nullptr if a diagonal
        // is zero, which we've already checked for.
        const double*const inverse = computeMatrixInverse( n, matrix.data( ) );

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ":   Inverse spillover matrix:\n";
            std::cerr << "  ";
            for ( uint32_t j = 0; j < n; ++j )
                std::cerr << std::setw(8) << matrixParameterNames[j];
            std::cerr << "\n";

            for ( uint32_t i = 0; i < n; ++i )
            {
                std::cerr << "  ";
                for ( uint32_t j = 0; j < n; ++j )
                {
                    std::cerr << std::setw(8) << std::setprecision(4) << std::fixed;
                    std::cerr << inverse[i*n + j] << " ";
                }
                std::cerr << "\n";
            }
        }


        //
        // Compensate.
        // -----------
        // For each event row, multiply the row's values times the matrix
        // inverse. Row and matrix is indexing is not straight-forward
        // because:
        //
        // - The row may contain more parameters than the matrix has. The
        //   parameters not covered by the matrix are left as-is. A common
        //   uncompensated parameter is "TIME".
        //
        // - The order of parameters in the matrix may not match the order
        //   of parameters in the event.
        //
        // This means we cannot do a simple numeric row * matrix.
        const size_t numberOfEvents = this->numberOfEvents;

        const uint32_t*const pi = matrixParameterIndexes.data( );

#ifdef _OPENMP
        // Create one temp array for each thread. The array is large
        // enough for one set of compensated parameters.
        const int numberOfThreads = omp_get_max_threads( );
        std::vector<double*> temps( numberOfThreads );
        for ( int i = 0; i < numberOfThreads; ++i )
            temps[i] = new double[n];
#else
        // Create one temp array. The array is large enough for one set
        // of compensated parameters.
        double*const temp = new double[n];
#endif

        if ( this->useFloats == true )
        {
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ":   compensate floats:\n";

#pragma omp parallel for schedule(static)
            for ( size_t i = 0; i < numberOfEvents; ++i )
            {
#ifdef _OPENMP
                double*const temp = temps[omp_get_thread_num( )];
#endif

                // Loop over the n parameters affected by the matrix.
                for ( uint32_t j = 0; j < n; ++j )
                {
                    // Loop over the rows of the matrix.
                    double value = 0.0;
                    for ( uint32_t k = 0; k < n; ++k )
                    {
                        // j is the column of the matrix, and k the row.
                        // so (k*n + j) selects the matrix value to use.
                        // k also indicates the parameter value needed
                        // and pi[k] is the column index for that parameter.
                        const auto src = this->parameters[pi[k]].getFloat(i);
                        value += src * inverse[k*n + j];
                    }

                    // The j-th parameter used by the matrix being affected
                    // and pi[j] is the index for that parameter.
                    temp[j] = value;
                }

                // And copy the computed value back into the parameters.
                for ( uint32_t j = 0; j < n; ++j )
                    this->parameters[pi[j]].setFloat( i, temp[j] );
            }
        }
        else
        {
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ":   compensate doubles:\n";

#pragma omp parallel for schedule(static)
            for ( size_t i = 0; i < numberOfEvents; ++i )
            {
#ifdef _OPENMP
                double*const temp = temps[omp_get_thread_num( )];
#endif

                // Loop over the n parameters affected by the matrix.
                for ( uint32_t j = 0; j < n; ++j )
                {
                    // Loop over the rows of the matrix.
                    double value = 0.0;
                    for ( uint32_t k = 0; k < n; ++k )
                    {
                        // j is the column of the matrix, and k the row.
                        // so (k*n + j) selects the matrix value to use.
                        // k also indicates the parameter value needed
                        // and pi[k] is the column index for that parameter.
                        const auto src = this->parameters[pi[k]].getDouble(i);
                        value += src * inverse[k*n + j];
                    }

                    // The j-th parameter used by the matrix being affected
                    // and pi[j] is the index for that parameter.
                    temp[j] = value;
                }

                // And copy the computed value back into the parameters.
                for ( uint32_t j = 0; j < n; ++j )
                    this->parameters[pi[j]].setDouble( i, temp[j] );
            }
        }


#ifdef _OPENMP
        for ( int i = 0; i < numberOfThreads; ++i )
            delete[] temps[i];
#else
        delete[] temp;
#endif
        delete[] inverse;

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ":   compensate done:\n";
    }

private:
    /**
     * Computes the inverse of the given square matrix.
     *
     * @param[in] n
     *   The width and height of the square matrix.
     * @param[in] matrix
     *   The matrix to invert.
     *
     * @return
     *   Returns the inverse matrix, or a nullptr if the matrix has a zero
     *   on the diagonal and therefore cannot be inverted.
     */
    double* computeMatrixInverse(
        const uint32_t n,
        const double*const matrix )
    {
        //
        // Initialize.
        // -----------
        // Allocate a double-width augmented matrix and initialize it
        // with the original matrix on the left side, and identity on the right.
        // Watch for zero on a diagonal and reject.
        double* augmented = new double[2*n*n];
        std::memset( augmented, 0, sizeof(double)*2*n*n );
        const uint32_t n2 = n * 2;

        for ( uint32_t i = 0; i < n; ++i )
        {
            const uint32_t in2 = i * n2;

            // If there is a zero diagonal, the matrix cannot be inverted.
            const double diagonal = matrix[i*n + i];
            if ( diagonal == 0.0 )
            {
                delete[] augmented;
                return nullptr;
            }

            augmented[in2 + i + n] = 1.0;
            for ( uint32_t j = 0; j < n; ++j )
                augmented[in2 + j] = matrix[i*n + j];
        }


        //
        // Perform Gauss-Jordan Elimintation.
        // ----------------------------------
        for ( uint32_t i = 0; i < n; ++i )
        {
            const uint32_t in2 = i * n2;
            const double diagonal = augmented[in2 + i];
            for ( uint32_t j = 0; j < n; ++j )
            {
                if ( i == j )
                    continue;

                const uint32_t jn2 = j * n2;
                const double ratio = augmented[j*n2 + i] / diagonal;
                for ( uint32_t k = 0; k < n2; ++k )
                    augmented[jn2 + k] -= ratio * augmented[in2 + k];
            }
        }

        // Make principal diagonal equal to 1.
        for ( uint32_t i = 0; i < n; ++i )
        {
            const uint32_t in2 = i * n2;
            const double diagonal = augmented[in2 + i];
            for ( uint32_t j = 0; j < n2; ++j )
                augmented[in2 + j] /= diagonal;
        }

        //
        // Copy.
        // -----
        // Copy the inverse matrix from the right side of the agumented matrix.
        double* inverse = new double[n*n];
        for ( uint32_t i = 0; i < n; ++i )
        {
            const uint32_t in2 = i * n2;
            for ( uint32_t j = 0; j < n; ++j )
                inverse[i*n + j] = augmented[in2 + j + n];
        }

        delete[] augmented;
        return inverse;
    }
    // @}
};

} // End Events namespace
} // End FlowGate namespace
