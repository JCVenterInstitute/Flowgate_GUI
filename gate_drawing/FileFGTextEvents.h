/**
 * @file
 * Loads and saves flow cytometry events in the custom (legacy) FlowGate text
 * file format, using tab-separated values.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
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

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
// For UNIX-based OSes, include <unistd.h> to get the _POSIX_* defined
// constants, which are checked for whether to use certain POSIX features.
// If POSIX features are not available, this software falls back to C/C++
// standard.
#include <unistd.h>
#endif

//
// Includes.
// ---------
// This software intentionally uses a mix of C and C++. C is used for
// standard data types, stdio for fast buffered I/O, and a variety of
// char* based string functions for fast string handling. This avoids the
// object overhead of C++ when parsing input files.
//
// Data types.
#include <cstdint>      // size_t, uint32_t, uint16_t, uint8_t, ...

// Standard C libraries.
#include <cerrno>       // Errno
#include <cstdio>       // std::FILE, std::fopen, std::fclose, ...
#include <cstring>      // std::strerror, ...

// Standard C++ libraries.
#include <codecvt>      // Unicode translation
#include <iostream>     // std::cerr, ...
#include <limits>       // std::numeric_limits
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector
#include <map>          // std::map

// FlowGate libraries.
#include "EventTable.h" // Event storage





//----------------------------------------------------------------------
//
// Defines and macros.
//
//----------------------------------------------------------------------
//
// File locking.
// -------------
// This software uses the stdio library for fast simple buffered file I/O.
// POSIX, BSD, Linux, macOS, and SystemV handle thread safety in stdio by
// locking and unlocking the file around every I/O call. This works, but it
// is expensive. To avoid this expense, this software can intentionally lock
// and unlock around the entire set of I/O calls. In this case, stdio will
// see that a lock is already in place and skip the per-call lock/unlock.
//
// Lock and unlock are done with flockfile() and funlockfile(). These calls
// are available in POSIX, BSD, Linux, macOS, and SystemV, but they are not
// part of the C/C++ standard. This software therefore uses these calls
// conditionally based upon the current OS.
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
#define FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
#endif





namespace FlowGate {
namespace File {





/**
 * Loads and saves flow cytometry events in the custom (legacy) FlowGate text
 * file format, using tab-separated values.
 *
 * The FlowGate project has defined a custom "FlowGate Text Event" file format
 * for the storage of event data from flow cytometry equipment and software.
 * <B>This is a legacy file format provided to interact with older software and
 * it is not recommended for further use.</B>
 *
 * The FlowGate Text Event file format is a simpler format than the ISAC FCS
 * format for flow cytometery data. FCS should still be used for original
 * reference data direct from acquisition hardware and software. The FlowGate
 * Text Event format, however, may be used as an alternate for derived data
 * that is passed among a chain of processing tools. The format's simpler
 * structure makes it easier to read and write than FCS and easier for
 * processing tools to manage.
 *
 *
 * <B>File format</B><BR>
 * An FlowGate Text Event file is a tab-separated value (TSV) text file format
 * that contains:
 *
 * @li A single line header containing parameter names for table columns.
 *   Parameter names are in UTF-8 Unicode encoding, which includes ASCII
 *   as a subset.
 *
 * @li A text table of acquired data treated as a list of events. Each
 *   event is on its own line, terminated by a carriage return ("\n").
 *   Tab-separated values on the line correspond to the named parameters
 *   in the header.  All values are floating point.
 *
 *
 * <B>Loading files</B><BR>
 * A file may be loaded by providing a file path to the constructor.
 * @code
 * auto file = new FileFGTextEvents( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFGTextEvents( );
 * file->load( filepath );
 * #endcode
 *
 * The same data object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the data object
 * is deleted automatically:
 * @code
 * auto file = new FileFGTextEvents( );
 * file->load( filepath1 );
 * ...
 * file->load( filepath2 );
 * ...
 * file->load( filepath3 );
 * ...
 * @endcode
 *
 * If a problem is encountered while loading a file, an exception
 * is thrown with a human-readable message indicating the problem. All
 * such problems are fatal and will abort loading the file.
 * @code
 * try {
 *   file->load( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 *
 * <B>Saving files</B><BR>
 * The data in a object may be written to a new file by calling
 * the save() method:
 * @code
 * try {
 *   file->save( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 * If a problem is encountered while saving a file, an exception
 * is thrown with a human-readable message indicating the problem, and
 * the file is deleted if it has already been started. Problems with
 * a save include the inability to create or write to the indicated file,
 * and having an incomplete data object. An incomplete object may be
 * missing data.
 *
 *
 * <B>Getting file attributes</B><BR>
 * After a file load or save, a map of name-value pairs is available
 * containing file attributes.
 * @code
 * // Get the file attributes.
 * auto attributes = file->getFileAttributes( );
 * @endcode
 *
 * File attributes are a std::map that maps keys to string values.
 * Well-known keys include:
 * @li "path" (string) The path to the most recently loaded or saved file.
 * @li "versionNumber" (string) The file version number.
 * @li "byteOrder" ("lsbf" or "msbf") The file's byte order.
 * @li "dataType" ("float" or "double") The file's event data type.
 * @li "numberFormat" ("binary" or "text") The file's number format.
 * @li "numberOfEvents" (long) The number of events in the file.
 *
 *
 * <B>Getting and setting events</B><BR>
 * Event data is accessed as an array of values for an event, selected by
 * an event index that begins at zero for the first event. Values are ordered
 * in the same order as parameters and all values have the same data type.
 *
 * The data type (float or double) for events may be queried and then
 * the appropriate method used to get event data:
 * @code
 * // Get the event table.
 * auto events = file->getEventTable( );
 *
 * // Get the number of events.
 * auto nEvents = events->getNumberOfEvents( );
 *
 * // Get the number of parameters per event.
 * auto nParam = events->getNumberOfParameters( );
 *
 * // Get the data type used for all events and parameters.
 * bool isFloat = events->areValuesFloats( );
 *
 * // Get floating point data.
 * if (isFloat == true )
 * {
 *   for (size_t i = 0; i < nEvents; ++i)
 *   {
 *     auto event = events->getEventFloats(i);
 *     for (size_t j = 0; j < nParam; ++j)
 *     {
 *       auto value = event[j];
 *       ...
 *     }
 *   }
 * }
 * else
 * {
 *   for (size_t i = 0; i < nEvents; ++i)
 *   {
 *     auto event = events->getEventDoubles(i);
 *     for (size_t j = 0; j < nParam; ++j)
 *     {
 *       auto value = event[j];
 *       ...
 *     }
 *   }
 * }
 * @endcode
 *
 *
 * @internal
 * This class is implemented using techniques to improve performance:
 *
 * @li The class is final and it has no virtual methods.
 *
 * @li All methods are implemented in this .h file so that a compiler can
 *     inline them.
 *
 * @li All I/O uses buffered stdio. Buffering reduces I/O system calls,
 *     and stdio has less method call overhead than C++ streams.
 *
 * @li Data is stored in linear arrays with all events and parameters in a
 *     single long sequence of values of the same data type.
 * @endinternal
 */
class FileFGTextEvents final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate (legacy) Text Events files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "FlowGate Text Events";

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
        "FileFGTextEvents";

    /**
     * The anticipated number of rows in a data file.
     *
     * This value is used to preallocate the event vector used to stage values
     * during file reading. Preallocating the vector reduces the overhead
     * of repeatedly growing those vectors.
     *
     * If there are more gate rows than this, the vector of results will be
     * automatically increased and parsing will continue. This is not a limit,
     * just a hint.
     *
     * A low number reallocates frequently, which requires more data copying
     * and more execution time. It also wastes less memory from over-
     * allocation. A high number wastes more memory, but does fewer expensive
     * reallocations.
     */
    static const size_t RESERVE_NUMBER_OF_EVENTS = 500000;

    /**
     * The initial size of read-line buffers.
     *
     * This value is used for the initial allocation of a buffer suitable
     * for reading a single line from a file.
     *
     * If a row in the file is longer than this, the read buffer will be
     * automatically increased and parsing will continue. This is not a limit,
     * just a hint.
     */
    static const size_t INITIAL_BUFFER_SIZE = 500;



    // Error messages --------------------------------------------------
    /**
     * The error message 1st line for a read error.
     */
    inline static const std::string ERROR_READ =
        "File read error.\n";

    /**
     * The error message 1st line for a write problem.
     */
    inline static const std::string ERROR_WRITE =
        "File write error.\n";

    /**
     * The error message 1st line for a programmer error.
     */
    inline static const std::string ERROR_PROGRAMMER =
        "Programmer error.\n";

    /**
     * The error message 1st line for a malformed file.
     */
    inline static const std::string ERROR_MALFORMED =
        "Malformed data file.\n";

    /**
     * The error message 1st line for a truncated file.
     */
    inline static const std::string ERROR_TRUNCATED =
        "Truncated data file.\n";

    /**
     * The error message introduction for a content problem.
     */
    inline static const std::string ERROR_BADCONTENTS =
        "The file's content is malformed or corrupted. ";

    /**
     * The error message ending for problems that cause data to not be loaded.
     */
    inline static const std::string ERROR_CANNOTLOAD =
        " The file's data cannot be loaded.";

    /**
     * The error message ending for problems that cause data to not be saved.
     */
    inline static const std::string ERROR_CANNOTSAVE =
        " The data cannot be saved to a file.";



//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    // File log --------------------------------------------------------
    /**
     * A log of file load and save error messages.
     *
     * Log entries are pairs where the first value is an error category,
     * and the second is an error message. Well-known categories are:
     *
     * @li "critical" for errors that cause file load or save to abort.
     *
     * @li "warning" for recoverable errors that do not cause file load
     *     or save to abort.
     *
     * @see ::clearFileLog()
     * @see ::appendFileLog()
     * @see ::getFileLog()
     */
    std::vector<std::pair<std::string,std::string>> fileLog;



    // Verbosity -------------------------------------------------------
    /**
     * Whether to be verbose and output progress messages.
     *
     * When true, progress messages and commentary are output during
     * file load, save, and other operations.
     *
     * @see ::verbosePrefix
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
     * @see ::verbose
     * @see ::isVerbose()
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     */
    std::string verbosePrefix;



    // File information ------------------------------------------------
    /**
     * The current file's parameter names.
     *
     * This is a redundant list. It is initialized when a file header is
     * loaded, and becomes redundant when a file's data is loaded into an
     * event table. It is only used whhile there is no event table.
     *
     * @see ::eventTable
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::getParameterNames()
     */
    std::vector<std::string> fileParameterNames;

    /**
     * The current file's attributes.
     *
     * During and after loading or saving a file, this contains name-value
     * pairs that describe the loaded or saved file. Prior to loading or
     * saving a file, this list is empty.
     *
     * Well-known keys are:
     * @li "path" (string) The path to the most recently loaded or saved file.
     * @li "versionNumber" (string) The file version number.
     * @li "byteOrder" ("lsbf" or "msbf") The file's byte order.
     * @li "dataType" ("float" or "double") The file's event data type.
     * @li "numberFormat" ("binary" or "text") The file's number format.
     * @li "numberOfEvents" (long) The number of events in the file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getFileAttributes()
     */
    std::map<const std::string, const std::string> fileAttributes;



    // Event data ------------------------------------------------------
    /**
     * The event table.
     *
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    std::shared_ptr<FlowGate::Events::EventTableInterface> eventTable;



//----------------------------------------------------------------------
// Format utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Format utilities
     */
    // @{
    /**
     * Returns a list of file name extensions associated with this format.
     *
     * @return
     *   Returns a list of extensions.
     *
     * @see ::isFileNameExtension()
     */
    inline static std::vector<std::string> getFileNameExtensions( )
        noexcept
    {
        std::vector<std::string> extensions;
        extensions.push_back( "fgt" );
        extensions.push_back( "txt" );
        return extensions;
    }

    /**
     * Returns true if the given file name extension is associated with
     * this format.
     *
     * @param[in] extension
     *   The file name extension. The extension should be in lower case
     *   and not include a leading dot (".").
     *
     * @return
     *   Returns true if the extension is associated with this format.
     *
     * @see ::getFileNameExtensions()
     */
    inline static bool isFileNameExtension( const std::string& extension )
        noexcept
    {
        const auto vec = getFileNameExtensions( );
        const size_t n = vec.size( );
        for ( size_t i = 0; i < n; ++i )
            if ( extension == vec[i] )
                return true;
        return false;
    }
    // @}



//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    // Empty constructor -----------------------------------------------
    /**
     * Constructs a new object with no parameters or events.
     *
     * The new object has has no parameters or events. These may be set
     * by setting the event table or loading data from a file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
    }



    // Copy constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given FileFGTextEvents object.
     *
     * @param[in] file
     *   The FileFGTextEvents object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( const FileFGTextEvents*const file )
    {
        // Validate.
        if ( file == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        if ( file->eventTable != nullptr )
            this->copy( file->eventTable );
    }

    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given FileFGTextEvents object.
     *
     * @param[in] file
     *   The FileFGTextEvents object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( const FileFGTextEvents& file )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        if ( file.eventTable != nullptr )
            this->copy( file.eventTable );
    }



    // Event table constructors ----------------------------------------
    /**
     * Constructs a new object using the given event table.
     *
     * The new object shares the given event table and its parameters
     * and events. External changes to the event table will affect this
     * object as well.
     *
     * @param[in] eventTable
     *   The event table to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( std::shared_ptr<FlowGate::Events::EventTableInterface> eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Use.
        this->setEventTable( eventTable );
    }

    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given event table.
     *
     * <B>Performance note</B><BR>
     * Run time can be reduced by sharing an existing event table instead
     * of copying it. This may be appropriate when an event table is read
     * from one file, then written to another. To share an event table, use
     * the shared pointer form of this constructor.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( const FlowGate::Events::EventTableInterface*const eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( eventTable );
    }

    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given event table.
     *
     * <B>Performance note</B><BR>
     * Run time can be reduced by sharing an existing event table instead
     * of copying it. This may be appropriate when an event table is read
     * from one file, then written to another. To share an event table, use
     * the shared pointer form of this constructor.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( const FlowGate::Events::EventTableInterface& eventTable )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( eventTable );
    }



    // Load constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with data loaded from a file.
     *
     * @param[in] path
     *   The file path to a file to load.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty.
     * @throws std::runtime_error
     *   Throws an exception if there is a problem loading the file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGTextEvents( const std::string& path )
    {
        // Validate.
        if ( path.empty( ) == 0 )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty path." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Load.
        this->load( path );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~FileFGTextEvents( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// File log.
//----------------------------------------------------------------------
    /**
     * @name File log
     */
    // @{
private:
    /**
     * Appends a message to the file log.
     *
     * The given category and message are used to create a log pair
     * appended to the end of the file log.
     *
     * Well-known categories are:
     *
     * @li "error" for errors that cause file load or save to abort.
     *
     * @li "warning" for recoverable errors that do not cause file load
     *     or save to abort.
     *
     * @param[in] category
     *   The message category.
     * @param[in] message
     *   The message.
     *
     * @see ::load()
     * @see ::save()
     * @see ::clearFileLog()
     * @see ::getFileLog()
     */
    inline void appendFileLog(
        const std::string& category,
        const std::string& message )
        noexcept
    {
        this->fileLog.push_back( std::make_pair( category, message ) );
    }

public:
    /**
     * Clears the file log.
     *
     * The log of error messages from the most recent file load or save
     * is cleared.
     *
     * The log is cleared automatically each time a file load or save is begun.
     *
     * @see ::load()
     * @see ::save()
     * @see ::appendFileLog()
     * @see ::getFileLog()
     */
    inline void clearFileLog( )
        noexcept
    {
        this->fileLog.clear( );
    }

    /**
     * Gets the file log.
     *
     * The file log records error and warning messages about issues
     * encountered during the most recent file load and save. Each
     * entry is a pair containing a log message category and the
     * associated message. Well-known categories are:
     *
     * @li "error" for errors that cause file load or save to abort.
     *
     * @li "warning" for recoverable errors that do not cause file load
     *     or save to abort.
     *
     * Critical errors typically cause an exception to be thrown. The
     * exception message is usually user-friendly and generic, while
     * the log will include more technical detail.
     *
     * The log is cleared automatically each time a file load or save is begun.
     *
     * @return
     *   Returns the verbose message prefix.
     *
     * @see ::load()
     * @see ::save()
     * @see ::appendFileLog()
     * @see ::clearFileLog()
     */
    inline const std::vector<std::pair<std::string,std::string>>& getFileLog( )
        const noexcept
    {
        return this->fileLog;
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
     * @see ::setVerbosePrefix()
     * @see ::isVerbose()
     * @see ::setVerbose()
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
     * @see ::setVerbosePrefix()
     * @see ::setVerbose()
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
     * @see ::getVerbosePrefix()
     * @see ::setVerbosePrefix()
     * @see ::isVerbose()
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
     * @see ::getVerbosePrefix()
     * @see ::isVerbose()
     * @see ::setVerbose()
     */
    inline void setVerbosePrefix( const std::string prefix )
        noexcept
    {
        this->verbosePrefix = prefix;
    }
    // @}



//----------------------------------------------------------------------
// File attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name File attributes
     */
    // @{
    /**
     * Returns a map of name-value pair file attributes.
     *
     * File attributes are set when a file is loaded or saved and primarily
     * provide information about the file itself, rather than the events in
     * the file. Each entry in the returned map uses an attribute key string
     * to name an associated string value. Some string values may be interpreted
     * as integers.
     *
     * Well-known keys include:
     * @li "versionNumber" (string)
     * @li "byteOrder" ("lsbf" or "msbf")
     * @li "dataType" ("float" or "double")
     * @li "numberFormat" ("binary" or "text") The file's number format.
     * @li "numberOfEvents" (long)
     *
     * @return
     *   Returns a map of string attribute keys to string attribute values.
     */
    inline const std::map<const std::string, const::std::string> getFileAttributes( )
        const noexcept
    {
        return this->fileAttributes;
    }

    /**
     * Returns the file path of the most recently loaded or saved file.
     *
     * When a file is loaded, this value is set to the path of the file
     * being read.
     *
     * When a file is saved, this value is set to the path of the file
     * being written.
     *
     * If a file has not been loaded or saved, an empty string is returned.
     *
     * @return
     *   Returns the path of the most recently loaded file.
     *
     * @see ::load()
     * @see ::save()
     */
    inline std::string getFilePath( )
        const noexcept
    {
        try
        {
            return this->fileAttributes.at("path");
        }
        catch ( ... )
        {
            // No file path yet.
            return std::string( );
        }
    }
    // @}



//----------------------------------------------------------------------
// Event clear and copy.
//----------------------------------------------------------------------
public:
    /**
     * @name Event clear and copy
     */
    // @{
    /**
     * Resets the object, clearing it of all content.
     */
    inline void reset( )
    {
        this->clearFileLog( );
        this->fileAttributes.clear( );
        this->eventTable.reset( );
    }



    // Copy event table ------------------------------------------------
    /**
     * Copies the given event table.
     *
     * The file object is reset.
     *
     * The parameter names and event array of the given event table are
     * copied into a new table stored within the object. The new table is
     * used by further methods, such as to save the event table to a file.
     *
     * <B>Performance note</B><BR>
     * Copying the event table requires allocating memory and copying values
     * from the given table to a new table. To avoid this memory use and copy
     * time, pass the event table within a shared pointer. The constructed
     * file object will then share the same instance of the event table.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Events::EventTableInterface*const eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Copy.
        this->copy( *eventTable );
    }

    /**
     * Copies the given event table.
     *
     * The file object is reset.
     *
     * The parameter names and event array of the given event table are
     * copied into a new table stored within the object. The new table is
     * used by further methods, such as to save the event table to a file.
     *
     * <B>Performance note</B><BR>
     * Copying the event table requires allocating memory and copying values
     * from the given table to a new table. To avoid this memory use and copy
     * time, pass the event table within a shared pointer. The constructed
     * file object will then share the same instance of the event table.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::reset()
     */
    inline void copy(
        const std::shared_ptr<FlowGate::Events::EventTableInterface>& eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Copy.
        this->copy( *(eventTable.get( )) );
    }

    /**
     * Copies the given event table.
     *
     * The file object is reset.
     *
     * The parameter names and event array of the given event table are
     * copied into a new table stored within the object. The new table is
     * used by further methods, such as to save the event table to a file.
     *
     * <B>Performance note</B><BR>
     * Copying the event table requires allocating memory and copying values
     * from the given table to a new table. To avoid this memory use and copy
     * time, pass the event table within a shared pointer. The constructed
     * file object will then share the same instance of the event table.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Events::EventTableInterface& eventTable )
        noexcept
    {
        // Initialize.
        this->reset( );

        // Copy.
        this->eventTable.reset( new FlowGate::Events::EventTable( eventTable ) );
    }
    // @}



//----------------------------------------------------------------------
// Event data.
//----------------------------------------------------------------------
public:
    /**
     * @name Event data
     */
    // @{
    /**
     * Gets the event table in use.
     *
     * If a file has not been loaded and an event table has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the event table.
     *
     * @see ::setEventTable()
     */
    inline std::shared_ptr<const FlowGate::Events::EventTableInterface> getEventTable( )
        const
    {
        return this->eventTable;
    }

    /**
     * Gets the event table in use.
     *
     * If a file has not been loaded and an event table has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the event table.
     *
     * @see ::setEventTable()
     */
    inline std::shared_ptr<FlowGate::Events::EventTableInterface> getEventTable( )
    {
        return this->eventTable;
    }

    /**
     * Returns the number of events.
     *
     * If a file has not been loaded and an event table has not been
     * initialized, then a zero is returned.
     *
     * @return
     *   Returns the number of events.
     *
     * @see ::getEventTable()
     */
    inline size_t getNumberOfEvents( )
        const noexcept
    {
        if ( this->eventTable != nullptr )
            return this->eventTable->getNumberOfEvents( );
        try
        {
            return std::stol( this->fileAttributes.at( "numberOfEvents" ) );
        }
        catch ( ... )
        {
            // No events yet.
            return 0;
        }
    }

    /**
     * Resets the object and sets the event table to use.
     *
     * The prior event table, if any, is released. The new event table
     * will be used for all further operations, such as saving to a file.
     *
     * @param[in] eventTable
     *   The event table to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::reset()
     * @see ::copy()
     */
    inline void setEventTable(
        std::shared_ptr<FlowGate::Events::EventTableInterface>& eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize.
        this->reset( );
        this->eventTable = eventTable;
    }
    // @}



//----------------------------------------------------------------------
// Load file.
//----------------------------------------------------------------------
public:
    /**
     * @name Load file
     */
    // @{
    /**
     * Loads the indicated file's data.
     *
     * The file is loaded into the current object, clearing out any prior
     * data. Upon completion, each of the "get" methods in this class will
     * return information loaded from the file.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] filePath
     *   The path to the file to load.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    inline void load(
        const char*const filePath,
        const ssize_t maximumEvents = (-1) )
    {
        if ( filePath == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file path." );

        this->load( std::string( filePath ), maximumEvents );
    }

    /**
     * Loads the indicated file's data.
     *
     * The file is loaded into the current object, clearing out any prior
     * data. Upon completion, each of the "get" methods in this class will
     * return information loaded from the file.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] filePath
     *   The path to the file to load.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void load(
        const std::string& filePath,
        ssize_t maximumEvents = (-1) )
    {
        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( filePath.empty() == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty file path." );


        //
        // Initialize.
        // -----------
        // Clear any prior data and the save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Loading FlowGate Text Event file \"" << filePath << "\".\n";

        this->reset( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );
        this->fileAttributes.emplace( "numberFormat", "text" );
        this->fileAttributes.emplace( "byteOrder", "N/A" );
        this->fileAttributes.emplace( "versionNumber", "none" );
        this->fileAttributes.emplace( "dataType", "float" );

        this->appendFileLog(
            "warning",
            "The FlowGate text events file is a legacy format. It does not support modern features, such as long parameter names, Unicode parameter names, double-precision floating point values, or other descriptive information. It is also a very inefficient way to store event data that requires several times the storage space of an original ISAC FCS event file and it requires about 10x the time to load the data. Continued use of this legacy file format is not recommended. Use the ISAC FCS or FlowGate binary events file formats instead." );


        //
        // Open the file.
        // --------------
        // Open the file for buffered reading using stdio.
        // Throw an exception if the file cannot be opened or read.
        std::FILE*const fp = std::fopen( path.c_str(), "r" );
        if ( fp == nullptr )
        {
            switch ( errno )
            {
            case EACCES:    // Access denied.
                throw std::invalid_argument(
                    std::string( "Access denied.\n" ) +
                    std::string( "Access permission was denied when trying to open the file \"" ) +
                    path + std::string( "\"." ) );

            case ENOENT:    // File does not exist.
                throw std::invalid_argument(
                    std::string( "File not found.\n" ) +
                    std::string( "The file was not found when trying to open the file \"" ) +
                    path + std::string( "\"." ) );

            default:
                throw std::invalid_argument(
                    std::string( "System error while opening the file.\n" ) +
                    std::string( "A system problem was encountered when trying to open the file \"" ) + path +
                    std::string( "\". The system reported: " ) +
                    std::string( std::strerror( errno ) ) );
            }
        }


        //
        // Load the file.
        // --------------
        // Thread-lock the file prior to stdio operations so that stdio
        // doesn't lock repeatedly on every function call.
        //
        // Then load the header to initialize file information, such as
        // the names of parameters. Then load the event data.
        try
        {
#ifdef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Load the header. This provides the names of parameters.
            // If maximumEvents is (-1), an event table is created for all
            // events in the file. If zero, the table has no events and no
            // event file I/O is done.  Otherwise space for the requested
            // number of events is allocated.
            this->loadHeader( fp, maximumEvents );

            // Read the data.
            this->loadEvents( fp, maximumEvents );
            this->fileAttributes.emplace(
                "numberOfEvents",
                std::to_string( this->eventTable->getNumberOfEvents( ) ) );

            // Compute the data min/max from parameter values.
            this->eventTable->computeParameterDataMinimumMaximum( );

            // Unfortunately, the text event format does not store
            // the specified min/max from the original data acquisition.
            // So just set the specified min/max to the current data
            // min/max.
            const size_t numberOfParameters =
                this->eventTable->getNumberOfParameters( );
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                this->eventTable->setParameterMinimum( i,
                    this->eventTable->getParameterDataMinimum( i ) );
                this->eventTable->setParameterMaximum( i,
                    this->eventTable->getParameterDataMaximum( i ) );
            }

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": Parameter min/max:\n";
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                {
                    std::cerr << this->verbosePrefix << ":   " <<
                        this->eventTable->getParameterName( i ) << "\n";
                    std::cerr << this->verbosePrefix << ":     Specified min, max: " <<
                        this->eventTable->getParameterMinimum( i ) << ", " <<
                        this->eventTable->getParameterMaximum( i ) << "\n";
                    std::cerr << this->verbosePrefix << ":     Data min, max: " <<
                        this->eventTable->getParameterDataMinimum( i ) << ", " <<
                        this->eventTable->getParameterDataMaximum( i ) << "\n";
                }
            }

#ifdef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Load header and data from file.
//----------------------------------------------------------------------
private:
    /**
     * @name Load header and data from file
     */
    // @{
    /**
     * Loads the file header containing the parameter names.
     *
     * The file header contains a tab-separated list of parameter names.
     *
     * This method reads the header, leaving the file pointer on the first
     * byte after the header. All header values are read, parsed, and saved
     * into this object.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     *
     * @internal
     * Parameter names in the file may be encoded using UTF-8's variable
     * character width encoding, of which ASCII is a subset. To parse the
     * input, the first line of the file is converted to wide characters
     * and then searched for tabs to split the line into parameter names.
     * The resulting wide strings are converted back to UTF-8 when saved
     * into the parameter name list.
     * @endinternal
     */
    void loadHeader( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        //
        // Read one line.
        // --------------
        // The first line of the file contains tab-separated parameter names.
        size_t bufferLength = 0;
        char*const buffer = this->readLine( fp, nullptr, bufferLength );
        if ( buffer == nullptr )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the first line, which should include a list of parameter names." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }

        //
        // Convert to wide characters.
        // ---------------------------
        // Parameter names may be UTF-8 Unicode. Because UTF-8 requires a
        // variable number of bytes per character, we cannot parse the text
        // by simple array indexing unless we first widden all characters to
        // wide characters, each of which is supposed to be large enough to
        // hold any Unicode character (i.e. 32-bits).
        //
        // Create a UTF-8<->wchar converter.
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> uconvert;

        // Convert UTF-8 buffer to a wide string.
        const std::wstring wBuffer = uconvert.from_bytes( buffer );
        const size_t wBufferLength = wBuffer.length( );

        delete[] buffer;

        //
        // Collect parameter names.
        // ------------------------
        // Loop over the buffer looking for tabs separating names.
        size_t startOfToken = 0;
        size_t endOfToken   = 0;

        std::vector<std::string> fileParameterNames;

        while ( startOfToken < wBufferLength )
        {
            while (wBuffer[endOfToken] != '\t' && wBuffer[endOfToken] != '\n' )
                ++endOfToken;

            const std::string name = uconvert.to_bytes(
                wBuffer.substr( startOfToken, (endOfToken - startOfToken) ) );
            fileParameterNames.push_back( name );

            startOfToken = (++endOfToken);
        }

        if ( fileParameterNames.size( ) == 0 )
        {
            this->appendFileLog(
                "error",
                "The first line of the file is empty, though it should contain a list of parameter names." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        //
        // Create empty event table.
        // -------------------------
        // Create a new float or double event table.
        size_t numberOfEvents = (size_t) maximumEvents;
        if ( maximumEvents < 0 )
            numberOfEvents = RESERVE_NUMBER_OF_EVENTS;

        this->eventTable.reset( new FlowGate::Events::EventTable(
            fileParameterNames,
            numberOfEvents,
            true ) );

        // We cannot set the total number of events in the file without
        // reading all of them, which is expensive. So don't set it.

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "File format version" << "generic tab-separated values (TSV)\n";
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number format" << "text\n";
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Data type" << "floats\n";
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of parameters:" <<
                fileParameterNames.size( ) << "\n";
        }
    }

    /**
     * Loads data from the file.
     *
     * The data is a sequence of text lines, each containing tab-separated
     * values.  Each line is a single event and values along a line are in
     * parameter order. All values are treated as single-precision floats.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void loadEvents( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        size_t numberOfEvents = this->eventTable->getNumberOfEvents( );
        if ( numberOfEvents == 0 )
            return;

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading file event table:\n";

        // Collect direct pointers to the data arrays for each of the
        // parameters. Using these speeds up adding values to the parameters.
        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        float** parameterData = new float*[numberOfParameters];
        for ( uint32_t i = 0; i < numberOfParameters; ++i )
            parameterData[i] = this->eventTable->getParameterFloats(i).data( );

        //
        // Read.
        // -----
        // Read the file's lines, parsing values into the table.
        //
        // Each line in the file is for a single event. Parameter values
        // are tab-separated. All values are floating-pint numbers, so
        // this is entirely ASCII and thereis no need to handle Unicode.
        char* buffer = nullptr;
        size_t bufferLength = 0;
        size_t eventIndex = 0;

        for ( ; ; )
        {
            // Read a line. On the first need of a buffer, the function will
            // allocate the buffer and return both the buffer and the updated
            // buffer length.
            buffer = this->readLine( fp, buffer, bufferLength );
            if ( buffer == nullptr )
            {
                // EOF. The buffer has already been deallocated by the
                // function.
                break;
            }

            // Sweep through the buffer and look for column boundaries.
            // For each column, read it as a float and add it to the
            // event table.
            const char* b = buffer;
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                char c;

                // Skip tabs to the next column value.
                while ( (c = *b) == '\t' && c != '\n' )
                    ++b;
                if (c == '\n')
                    break;

                // Advance to the end of the value.
                const char* bend = b + 1;
                while ( (c = *bend) != '\t' && c != '\n' )
                    ++bend;

                // Parse the float and add it to the event table.
                parameterData[i][eventIndex] = std::stof( b );

                if ( c == '\n' )
                    break;

                b = bend + 1;
            }

            ++eventIndex;
            if ( maximumEvents > 0 && eventIndex >= (size_t) maximumEvents )
            {
                // Stop early.
                if ( buffer != nullptr )
                    delete[] buffer;
                break;
            }

            if ( eventIndex >= numberOfEvents )
            {
                // We've filled the event table. Increase its size.
                numberOfEvents += RESERVE_NUMBER_OF_EVENTS;
                if ( maximumEvents > 0 &&
                     numberOfEvents > (size_t) maximumEvents )
                    numberOfEvents = maximumEvents;
                this->eventTable->resize( numberOfEvents );

                // After a resize, the parameter vectors have reallocated
                // their backing arrays and we need to get their pointers
                // again.
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                    parameterData[i] = this->eventTable->getParameterFloats(i).data( );
            }
        }

        delete[] parameterData;

        if ( eventIndex == 0 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated without including any event values." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }

        // Resize event table to the actual number of events read.
        this->eventTable->resize( eventIndex );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of events:" <<
                eventIndex << "\n";
    }

    /**
     * Reads a single line from the input file.
     *
     * @param[in] fp
     *   The file pointer to read from.
     * @param[in] line
     *   The line buffer to use, or nullptr if a new buffer should be
     *   allocated. On an error, this is deleted and a nullptr returned.
     * @param[out] lineSize
     *   The current size of the line buffer, if one is provided. On an
     *   error, a nullptr is returned and this value is undefined.
     *
     * @return
     *   Returns a pointer to allocated memory holding the characters of a
     *   line of text from the file, terminated by a "\n". If a line buffer
     *   was passed as an argument and did not need to be resized, the same
     *   buffer is returned. Otherwise a new larger buffer is allocated and
     *   returned. A nullptr is returned if an I/O error occurs. The line
     *   buffer is deleted on an error.
     *
     * @internal
     * This implementation primarily calls fgets(), which handles buffered
     * I/O and returns a single line of text, up to a maximum number of
     * characters.
     *
     * The complexity here is that we don't know how many characters might
     * be on the first line of text. We can guess that it is less than
     * several hundred, and allocate a buffer big enough for that initially.
     * But if fgets() doesn't reach the end of line in one call, we have
     * to allocate a bigger buffer and continue, and so on.
     *
     * To facilitate inline-ing, this functionality is split into two
     * functions:
     * - readLine() is called by other code to read a line of text. It
     *   contains enough logic for the most common case of reading into a
     *   buffer that is big enough. This function is small enough that it is
     *   easily inlined, avoiding the cost of a function call during the
     *   tight read loop.
     *
     * - readLineAgain() is called by readLine() only for the case where
     *   fgets() indicated there was more to the line than could fit in the
     *   buffer. The buffer has to be increased in size and additional bytes
     *   read. This is rarely needed since once the buffer is big enough for
     *   one file line, it is likely big enough for most or all of the rest.
     * @endinternal
     */
    inline char* readLine( std::FILE*const fp, char* line, size_t& lineSize )
    {
        // Create a line buffer, if needed.
        if ( line == nullptr )
        {
            lineSize = INITIAL_BUFFER_SIZE;
            line     = new char[lineSize];
        }

        // Read a line into the buffer.
        //
        // @todo fgets() is probably not safe for UTF-8.
        if ( std::fgets( line, lineSize, fp ) == nullptr )
        {
            // Fail. EOF or I/O error.
            delete[] line;
            return nullptr;
        }

        // If the end of the string has an EOL, then we've read the
        // entire line and we're done.
        //
        // @todo strlen() is not safe for UTF-8.
        const size_t n = std::strlen( line );
        if ( line[n-1] == '\n' )
            return line;

        // For the uncommon case where the buffer was not big enough,
        // increase the buffer size and read some more.
        return readLineAgain( fp, line, lineSize );
    }

    /**
     * Reads more of the line from the input file.
     *
     * @param[in] fp
     *   The file pointer to read from.
     * @param[in] line
     *   The line buffer to use, or nullptr if a new buffer should be
     *   allocated. On an error, this is deleted and a nullptr returned.
     * @param[out] lineSize
     *   The current size of the line buffer, if one is provided. On an
     *   error, a nullptr is returned and this value is undefined.
     *
     * @return
     *   Returns a pointer to allocated memory holding the characters of a
     *   line of text from the file, terminated by a "\n". If a line buffer
     *   was passed as an argument and did not need to be resized, the same
     *   buffer is returned. Otherwise a new larger buffer is allocated and
     *   returned. A nullptr is returned if an I/O error occurs. The line
     *   buffer is deleted on an error.
     */
    inline char* readLineAgain( std::FILE*const fp, char* line, size_t& lineSize )
    {
        int lineOffset;
        for ( ; ; )
        {
            // There's more to the line and we need space for it. Allocate
            // a new bigger buffer, copy the old buffer into it, and
            // prepare to read more.
            const size_t newLineSize = lineSize * 2;
            char*const newLine = new char[newLineSize];

            std::memcpy( newLine, line, lineSize );

            delete[] line;
            line = newLine;

            lineOffset = lineSize - 1;
            lineSize   = newLineSize;

            if ( std::fgets( line + lineOffset, lineSize / 2, fp ) == nullptr )
            {
                // Fail. EOF or I/O error.
                delete[] line;
                return nullptr;
            }

            // If the end of the string has an EOL, then we've read the
            // entire line and we're done.
            const size_t n = std::strlen(line);
            if ( line[n-1] == '\n' )
                return line;
        }
    }
    // @}



//----------------------------------------------------------------------
// Save file.
//----------------------------------------------------------------------
public:
    /**
     * @name Save file
     */
    // @{
    /**
     * Saves the event table to a new file.
     *
     * The file is written with events from the current event table.
     *
     * A maximum number of events may be set to limit the number of events
     * saved to the file.  A value of zero does not save any event data.
     * A value of (-1) saves all event data.    
     *
     * @param[in] filePath
     *   The path to the file to save.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to save.
     *   A value of zero does not save any data.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void save(
        const std::string& filePath,
        const ssize_t maximumEvents = (-1) )
    {
        //
        // Initialize.
        // -----------
        // Save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Saving FlowGate Text Events file \"" << filePath << "\".\n";

        this->clearFileLog( );
        this->fileAttributes.clear( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );
        this->fileAttributes.emplace( "numberFormat", "text" );
        this->fileAttributes.emplace( "byteOrder", "N/A" );
        this->fileAttributes.emplace( "versionNumber", "none" );

        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( path.empty() == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file path." );
        if ( this->eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "Invalid NULL event table." ) );

        this->appendFileLog(
            "warning",
            "The FlowGate text events file is a legacy format. It does not support modern features, such as long user-chosen parameter names, Unicode parameter names, double-precision floating point values, or other descriptive information. It is also a very inefficient way to store event data that requires several times the storage space of an original ISAC FCS event file and it requires about 10x the time to load the data. Continued use of this legacy file format is not recommended. Use the ISAC FCS or FlowGate binary events file formats instead." );

        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );
        bool hasLongName = false;
        for ( uint32_t i = 0; i < numberOfParameters && hasLongName == false; ++i )
            if ( this->eventTable->getParameterLongName( i ).empty( ) == false )
                hasLongName = true;
        if ( hasLongName == true )
        {
            this->appendFileLog(
                "warning",
                "The event data being saved includes user-chosen long parameter names. Unfortunately, the legacy FlowGate text events file format does not support storing these names. To avoid losing these names, use the ISAC FCS or FlowGate binary events file formats instead." );
        }

        if ( this->eventTable->areValuesFloats( ) == false )
        {
            this->appendFileLog(
                "warning",
                "The event data being saved uses double-precision floating point. However, the legacy FlowGate text events file format does not support double precision. Saved values will be reduced to single-precision. To avoid this loss of precision, use the ISAC FCS or FlowGate binary events file formats instead." );
        }

        //
        // Open the file.
        // --------------
        // Open the file for buffered writing using stdio.
        // Throw an exception if the file cannot be opened.
        std::FILE*const fp = std::fopen( path.c_str(), "w" );
        if ( fp == nullptr )
        {
            switch ( errno )
            {
            case EACCES:    // Access denied.
                throw std::invalid_argument(
                    std::string( "Access denied.\n" ) +
                    std::string( "Access was denied when trying to open the file \"" ) +
                    path + std::string( "\"." ) );

            case ENOENT:    // File does not exist.
                throw std::invalid_argument(
                    std::string( "File not found.\n" ) +
                    std::string( "The file was not found when trying to open the file \"" ) +
                    path + std::string( "\"." ) );

            default:
                throw std::invalid_argument(
                    std::string( "System error while opening the file.\n" ) +

                    std::string( "A system problem was encountered when trying to open the file \"" ) +
                    path +
                    std::string( "\". The system reported: " ) +
                    std::string( std::strerror( errno ) ) );
            }
        }

        //
        // Save the data.
        // --------------
        // Thread-lock the file prior to stdio operations so that stdio
        // doesn't lock repeatedly on every function call.
        //
        // Then save the header with file information, such as
        // the names of parameters. Then save the binary event data.
        try
        {
#ifdef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Save the header. This stores a list of parameter names.
            this->saveHeader( fp, maximumEvents );

            // Write the data.
            this->saveEvents( fp, maximumEvents );

#ifdef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Save header and data to file.
//----------------------------------------------------------------------
private:
    /**
     * @name Save header and data to file
     */
    // @{
    /**
     * Saves the file header containing the parameter names.
     *
     * The text file header contains a single line listing the event
     * table's parameter names, separated by tabs.
     *
     * This method writes the header, leaving the file pointer on the first
     * byte after the header.
     *
     * A maximum number of events may be set to limit the number of events
     * saved to the file.  A value of zero does not save any event data.
     * A value of (-1) saves all event data.    
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to save.
     *   A value of zero does not save any data.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     *
     * @internal
     * Parameter names are stored in the event table using std::string,
     * which supports the variable character width encoding of UTF-8.
     * Parameter names are output here using std::fprintf(), which supports
     * this as well. The result is a file header that is Unicode safe.
     * @endinternal
     */
    void saveHeader( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        //
        // Setup.
        // ------
        auto parameterNames = this->eventTable->getParameterNames( );
        const size_t numberOfParameters = parameterNames.size( );

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving file header:\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version" << "generic tab-separated values (TSV)\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number format" << "text\n";
            if ( this->eventTable->areValuesFloats( ) == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "floats\n";
            else
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "doubles\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of parameters:" <<
                this->eventTable->getNumberOfParameters( ) << "\n";

            size_t n = this->getNumberOfEvents( );
            if ( maximumEvents >= 0 && (size_t) maximumEvents < n )
                n = maximumEvents;
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of events:" <<
                n << "\n";
        }

        //
        // Write the parameter names.
        // --------------------------
        // Parameter names are separated by tabs and terminated with
        // a carriage return.
        int status = std::fprintf( fp, "%s", parameterNames[0].c_str( ) );
        for ( size_t i = 1; i < numberOfParameters && status >= 0; ++i )
            status = std::fprintf( fp, "\t%s", parameterNames[i].c_str( ) );

        if ( status < 0 || (status = std::fprintf( fp, "\n" )) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header parameter names: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves event data.
     *
     * The event data is text and contains a list of events. Each
     * event is on its own line, terminated by a carriage return. Tab-separated
     * values on the line correspond to the named parameters in the header.
     *
     * A maximum number of events may be set to limit the number of events
     * saved to the file.  A value of zero does not save any event data.
     * A value of (-1) saves all event data.    
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to save.
     *   A value of zero does not save any data.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveEvents( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        size_t numberOfEvents = this->eventTable->getNumberOfEvents( );
        if ( numberOfEvents == 0 || maximumEvents == 0 )
        {
            this->fileAttributes.emplace( "dataType", "float" );
            this->fileAttributes.emplace( "numberOfEvents", "0" );
            return;
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving file events:\n";

        //
        // Set up.
        // -------
        // Limit the number of events and get the amount to write out.
        if ( maximumEvents > 0 && (size_t) maximumEvents < numberOfEvents )
            numberOfEvents = maximumEvents;

        this->fileAttributes.emplace(
            "numberOfEvents",
            std::to_string( numberOfEvents ) );

        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        //
        // Write the values.
        // -----------------
        // All values are written as floating point with an output
        // precision set to be the maximum number of digits required for
        // base-10 output of the data type.
        //
        // Each line in the file has one event, with tabs separating
        // paramater values.
        //
        // For I/O error checking, only check the return status of the
        // last operation per line. If a file becomes unwritable earlier,
        // all further calls will fail with the same error, so checking on
        // only the last one is sufficient. This saves a lot of additional
        // "if" checks and code bloat.
        if ( this->eventTable->areValuesFloats( ) == true )
        {
            this->fileAttributes.emplace( "dataType", "float" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " <<
                    "    Saving 32-bit floats\n";

            const int digits = std::numeric_limits<float>::digits10 + 1;

            // Collect direct pointers to the data arrays for each of the
            // parameters. Using these speeds up getting values from
            // the parameters.
            float** parameterData = new float*[numberOfParameters];
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterFloats( i ).data( );

            for ( size_t i = 0; i < numberOfEvents; ++i )
            {
                std::fprintf( fp, "%0.*f", digits, parameterData[0][i] );
                for ( uint32_t j = 1; j < numberOfParameters; ++j )
                    std::fprintf( fp, "\t%0.*f", digits, parameterData[j][i] );

                if ( std::fprintf( fp, "\n" ) < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The system reported an error while writing the file event data: " ) +
                        std::string( std::strerror( errno ) ) );
                    throw std::runtime_error(
                        ERROR_WRITE +
                        ERROR_CANNOTSAVE );
                }
            }
        }
        else
        {
            this->fileAttributes.emplace( "dataType", "double" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " <<
                    "    Saving 64-bit floats\n";

            const int digits = std::numeric_limits<double>::digits10 + 1;

            // Collect direct pointers to the data arrays for each of the
            // parameters. Using these speeds up getting values from
            // the parameters.
            double** parameterData = new double*[numberOfParameters];
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterDoubles( i ).data( );

            for ( size_t i = 0; i < numberOfEvents; ++i )
            {
                std::fprintf( fp, "%0.*lf", digits, parameterData[0][i] );
                for ( uint32_t j = 1; j < numberOfParameters; ++j )
                    std::fprintf( fp, "\t%0.*lf", digits, parameterData[j][i] );

                if ( std::fprintf( fp, "\n" ) < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The system reported an error while writing the file event data: " ) +
                        std::string( std::strerror( errno ) ) );
                    throw std::runtime_error(
                        ERROR_WRITE +
                        ERROR_CANNOTSAVE );
                }
            }
        }
    }
    // @}
};

} // End File namespace
} // End FlowGate namespace

#undef FLOWGATE_FILEFGTEXTEVENTS_USE_FLOCKFILE
