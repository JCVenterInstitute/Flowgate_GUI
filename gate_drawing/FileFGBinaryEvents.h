/**
 * @file
 * Loads and saves flow cytometry events in the custom FlowGate binary
 * events file format.
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

// For UNIX-based OSes, include <sys/stat.h> to get fstat(), which may be
// used to get the file size during file loading.
#include <sys/stat.h>
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
#include <cctype>       // std::isdigit
#include <cerrno>       // Errno
#include <cstdio>       // std::FILE, std::fopen, std::fclose, ...
#include <cstring>      // std::strerror, ...

// Standard C++ libraries.
#include <iostream>     // std::cerr, ...
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
#define FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
#endif

//
// Byte order.
// -----------
// This software uses the current host's byte order to guide byte swapping
// of binary data. There are three well-known byte orders:
//
// - Little endian = least significant byte first (LSBF). This is the most
//   common byte order, primarily because it is used by Intel processors.
//
// - Big endian = most significant byte first (MSBF).
//
// - PDP endian. This is an ancient artifact of the DEC PDP processor, which
//   used a mix of little- and big-endian byte ordering.
//
// The ancient PDP byte order is not supported. Just to be thorough, check
// for it and reject.
#if __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#error "Abort: This software does not support the ancient PDP byte order."
#endif

//
// Byte swapping.
// --------------
// When doing byte swaps, modern processors have special instructions that
// can do the swap very efficiently. While a good compiler might recognize
// properly written C/C++ code and automatically use these instructions, a
// better way to insure their use is to use compiler-provided byte-swapping
// built-in macros:
// - __builtin_bswap16
// - __builtin_bswap32
// - __builtin_bswap64
//
// These macros are available in GCC and GCC-compatible compilers, such as
// CLANG.
#if defined(__GNUC__) && (__GNUC__ > 4)
#define FLOWGATE_FILEFGBINARYEVENTS_USE_BSWAP
#elif defined(__clang__) && __has_builtin(__builtin_bswap16__) && \
    __has_builtin(__builtin_bswap32__) && \
    __has_builtin(__builtin_bswap64__)
#define FLOWGATE_FILEFGBINARYEVENTS_USE_BSWAP
#endif





namespace FlowGate {
namespace File {





/**
 * Loads and saves flow cytometry events in the custom FlowGate binary
 * events file format.
 *
 * The FlowGate project has defined a "FlowGate Binary Events" file format
 * for the storage of event data from flow cytometry equipment and software.
 * The data is always floating point and is typically a result of converting
 * an International Society for Advancement of Cytometry (ISAC) FSC file
 * containing "channel" (raw) event values collected during data acquisition
 * from a flow cytometer. Values stored in this file are post-scaling and
 * post-compensation, but prior to gating and gate transforms.
 *
 * This file format has the following well-known revisions:
 * @li 1.0 from 2019.
 *
 * The FlowGate Binary Event file format is a simpler format than the ISAC FCS
 * format for flow cytometery data. FCS should still be used for original
 * reference data direct from acquisition hardware and software. The
 * FlowGate Binary Events format, however, may be used as an alternate for
 * derived data that is passed among a chain of processing tools. The
 * format's simpler structure makes it faster to read and write than FCS
 * and easier for processing tools to manage.
 *
 *
 * <B>File format</B><BR>
 * Each FlowGate Binary Event file contains:
 *
 * @li A brief text header that includes the file format version number,
 *   the number of parameters and events, and the short and long names of
 *   all parameters.
 *
 * @li A binary table of events arranged as a series of columns with one
 *   column for each parameter. Consecutive values in the file are for
 *   consecutive event values for the same parameter. Each column has the
 *   same number of event values and all values are either single- or
 *   double-precision floating point.
 *
 *
 * <B>Loading files</B><BR>
 * A file may be loaded by providing a file path to the constructor.
 * @code
 * auto file = new FileFGBinaryEvents( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFGBinaryEvents( );
 * file->load( filepath );
 * #endcode
 *
 * The same object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the object
 * is deleted automatically:
 * @code
 * auto file = new FileFGBinaryEvents( );
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
 * The data in an object may be written to a new file by calling
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
 * <B>Getting and setting the event table</B><BR>
 * After a file load or save, the event table loaded or saved is available:
 * @code
 * // Get the event table.
 * auto events = file->getEventTable( );
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
 * @endinternal
 */
class FileFGBinaryEvents final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate Binary Events files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "FlowGate Binary Events";

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
        "FileFGBinaryEvents";



    // Error messages --------------------------------------------------
    /**
     * The error message 1st line for a programmer error.
     */
    inline static const std::string ERROR_PROGRAMMER =
        "Programmer error.\n";

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
     * The error message 1st line for an unsupported file.
     */
    inline static const std::string ERROR_UNSUPPORTED =
        "Unsupported data file format.\n";

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
    bool verbose = false;

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
     * The current file's size, in bytes.
     *
     * This value is only used during file loading. Before and after loading,
     * the field's value is undefined.
     *
     * @see ::load()
     */
    long fileSize = 0L;

    /**
     * The current file's number of events.
     *
     * This value is only used during file loading. Before and after loading,
     * the field's value is undefined.
     *
     * @see ::eventTable
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    size_t fileNumberOfEvents = 0;

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
     * After loading or saving a file, this is the event table that was
     * loaded or saved. Prior to loading or saving a file, this is NULL.
     *
     * When loading a file, without loading its data, this may be an
     * event table with parameters, but no events.
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
        extensions.push_back( "fgb" );
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
    FileFGBinaryEvents( )
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
     * and events in the given FileFGBinaryEvents object.
     *
     * @param[in] file
     *   The FileFGBinaryEvents object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGBinaryEvents( const FileFGBinaryEvents*const file )
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
     * and events in the given FileFGBinaryEvents object.
     *
     * @param[in] file
     *   The FileFGBinaryEvents object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFGBinaryEvents( const FileFGBinaryEvents& file )
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
    FileFGBinaryEvents( std::shared_ptr<FlowGate::Events::EventTableInterface> eventTable )
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
    FileFGBinaryEvents( const FlowGate::Events::EventTableInterface*const eventTable )
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
    FileFGBinaryEvents( const FlowGate::Events::EventTableInterface& eventTable )
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
    FileFGBinaryEvents( const std::string& path )
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
    virtual ~FileFGBinaryEvents( )
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
     * @li "versionNumber" (string) The format version name/number.
     * @li "byteOrder" ("lsbf" or "msbf") The byte order for binary numbers.
     * @li "dataType" ("float" or "double") The data type for numbers.
     * @li "numberFormat" ("binary" or "text") The file's number format.
     * @li "numberOfEvents" (long) The number of events.
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
     *   Returns the path of the most recently loaded or saved file.
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
        this->clearFileLog();
        this->fileSize = 0L;
        this->fileNumberOfEvents = 0;
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
        this->reset( );
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
                "Loading FlowGate Binary Event file \"" << filePath << "\".\n";
        this->reset( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );


        //
        // Open the file.
        // --------------
        // Open the file for binary buffered reading using stdio.
        // Throw an exception if the file cannot be opened or read.
        std::FILE*const fp = std::fopen( path.c_str(), "rb" );
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
        // the names of parameters. Then load the binary event data.
        try
        {
#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Get the file size.
            this->fileSize = 0;
            if ( std::fseek( fp, 0, SEEK_END ) != 0 )
            {
                // Seek to end failed. SEEK_END is not a required feature
                // for fseek() implementations, so switch to an OS-specific
                // method.
#ifdef _POSIX_VERSION
                struct stat statBuffer;
                if ( stat( path.c_str(), &statBuffer ) == 0 )
                    this->fileSize = statBuffer.st_size;
#endif
            }
            else
            {
                const long size = std::ftell( fp );
                if ( size >= 0 )
                {
                    this->fileSize = size;
                    std::fseek( fp, 0, SEEK_SET );
                }
            }

            // Load the header. This provides the file format version in use
            // and the names of parameters.  If maximumEvents is (-1),
            // an event table is created for all events in the file. If zero,
            // the table has no events and no event file I/O is done.
            // Otherwise space for the requested number of events is allocated.
            this->loadHeader( fp, maximumEvents );

            // Read the event table. The number of events indicated in the
            // event table are loaded.
            this->loadEvents( fp );

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": Parameter min/max:\n";
                const auto numberOfParameters =
                    this->eventTable->getNumberOfParameters( );
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

#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }

private:
    /**
     * Loads the file header containing the version number and parameter names.
     *
     * The file header is in ASCII and contains:
     * - The file format's version number.
     * - The byte order.
     * - The event data type.
     * - The number of events.
     * - The number of parameters.
     * - A list of parameter names, containing:
     *   - The number of BYTES of the name.
     *   - The UTF-8 bytes for the name.
     * - A list of parameter long names, containing:
     *   - The number of BYTES of the name.
     *   - The UTF-8 bytes for the name.
     * - A list of parameter minimum values.
     * - A list of parameter maximum values.
     *
     * Each value, except parameter names and version number, is in a field
     * of 20 bytes, left-adjusted and space padded. 20 bytes is sufficient to
     * record a 64-bit integer in base-10 ASCII digits.
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
     */
    void loadHeader( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading file header:\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File size:";
            if ( this->fileSize == 0 )
                std::cerr << "unknown\n";
            else
                std::cerr << this->fileSize << " bytes\n";
        }

        static const int VALUESIZE = 20;
        static const int VERSIONSIZE = 40;
        char buffer[VERSIONSIZE + 1];
        buffer[VERSIONSIZE] = '\0';

        //
        // Validate file size.
        // -------------------
        // The file header contains at least the following, in ASCII:
        // - 40 bytes for the file version number.
        // - 20 bytes for the byte order.
        // - 20 bytes for the data type.
        // - 20 bytes for the number of events.
        // - 20 bytes for the number of parameters.
        //
        // The total is then (5 + 20) = 100 bytes. Make sure the
        // file has at least that much in it.
        if ( this->fileSize != 0 && this->fileSize < 100 )
        {
            this->appendFileLog(
                "error",
                "The file is too short to be a valid FlowGate Binary Event file. A header of at least 100 bytes is required." );
            throw std::runtime_error(
                std::string( "Unrecognized data file format.\n" ) +
                std::string( "The file does not use a recognized format for FlowGate binary flow cytometry data, such as the FlowGate Binary Event format." ) +
                ERROR_CANNOTLOAD );
        }

        //
        // Read the version number.
        // ------------------------
        // - 40 bytes, space padded.
        // - In ASCII.
        // - Has the format "FlowGateBinaryEvents_" followed by
        //   major.minor.subminor (e.g. "FlowGateBinaryEvents_1.0.0").
        if ( std::fread( buffer, 1, VERSIONSIZE, fp ) != VERSIONSIZE )
        {
            this->appendFileLog(
                "error",
                "A read error occurred while trying to read the first 20 bytes of the file, which should contain the FlowGate Binary Event format's version number." );
            throw std::runtime_error(
                ERROR_READ +
                ERROR_CANNOTLOAD );
        }

        if ( std::strncmp( "FlowGateBinaryEvents_1.0.0", buffer, 26 ) != 0 )
        {
            this->appendFileLog(
                "error",
                "The file is not an FlowGate Binary Events file. It does not start with \"FlowGateBinaryEvents_1.0.0\"." );
            throw std::runtime_error(
                std::string( "Unrecognized data file format.\n" ) +
                std::string( "The file does not use a recognized format for FlowGate binary flow cytometry data, such as the FlowGate Binary Events format." ) +
                ERROR_CANNOTLOAD );
        }

        // Trim off trailing blanks.
        size_t i = VERSIONSIZE - 1;
        while ( buffer[i] == ' ' )
            --i;
        buffer[i+1] = '\0';

        // Save the version number.
        this->fileAttributes.emplace( "versionNumber", buffer );
        this->fileAttributes.emplace( "numberFormat", "binary" );
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << buffer << "\"\n";

        //
        // Read the byte order.
        // --------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        // - One of "lsbf" or "msbf".
        if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the format's byte order should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[VALUESIZE] = '\0';

        if ( std::strncmp( buffer, "lsbf", 4 ) == 0 )
        {
            this->fileAttributes.emplace( "byteOrder", "lsbf" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Byte order:" << "Least-significant byte first.\n";
        }
        else if ( std::strncmp( buffer, "msbf", 4 ) == 0 )
        {
            this->fileAttributes.emplace( "byteOrder", "msbf" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Byte order:" << "Most-significant byte first.\n";
        }
        else
        {
            this->appendFileLog(
                "error",
                std::string( "The file header contains an unrecognized byte order: \"" ) +
                std::string( buffer ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        //
        // Read the data type.
        // -------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        // - One of "float" or "double".
        if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the format's data type should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[VALUESIZE] = '\0';

        bool areFloats = true;

        if ( std::strncmp( buffer, "float", 4 ) == 0 )
        {
            this->fileAttributes.emplace( "dataType", "float" );
            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Number format:" << "binary\n";
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "floats\n";
            }
        }
        else if ( std::strncmp( buffer, "double", 4 ) == 0 )
        {
            areFloats = false;
            this->fileAttributes.emplace( "dataType", "double" );
            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Number format:" << "binary\n";
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "doubles\n";
            }
        }
        else
        {
            this->appendFileLog(
                "error",
                std::string( "The file header contains an unrecognized data type: \"" ) +
                std::string( buffer ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        //
        // Read the number of events.
        // --------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the format's number of events should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[VALUESIZE] = '\0';

        this->fileNumberOfEvents = std::stol( buffer );
        this->fileAttributes.emplace( "numberOfEvents",
            std::to_string( this->fileNumberOfEvents ) );

        if ( this->fileNumberOfEvents <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header contains an invalid non-positive number of events: " ) +
                std::to_string( this->fileNumberOfEvents ) +
                std::string( "." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of events:" <<
                this->fileNumberOfEvents << "\n";

        //
        // Read the number of parameters.
        // ------------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's number of parameters should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[VALUESIZE] = '\0';

        const size_t numberOfParameters = std::stol( buffer );
        if ( numberOfParameters <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header contains an invalid non-positive number of parameters: " ) +
                std::to_string( numberOfParameters ) + std::string( "." ));
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of parameters:" <<
                numberOfParameters << "\n";

        //
        // Read the parameter short names.
        // -------------------------------
        // Each parameter is read with:
        // - The number of bytes for the parameter name.
        //   - 20 bytes, space padded.
        //   - In ASCII.
        // - The parameter name.
        //   - However many bytes are required.
        //   - In UTF-8.
        //
        // Create a temporary buffer for storing the name. While this is
        // allocated at an initial size, usage will automatically increase
        // this size as needed to accomodate really long names.
        size_t nameBufferSize = 1000;
        char* nameBuffer = new char[nameBufferSize];

        // Reserve space for a list of primary (short) names and long names.
        std::vector<std::string> fileParameterNames;
        std::vector<std::string> fileParameterLongNames;

        fileParameterNames.reserve( numberOfParameters );
        fileParameterLongNames.reserve( numberOfParameters );

        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter name length, in bytes.
            if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
            {
                this->appendFileLog(
                    "error",
                     "The file is truncated where the format's parameter names should be." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[VALUESIZE] = '\0';

            const size_t nBytes = std::stol( buffer );
            if ( nBytes <= 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file header contains an invalid non-positive parameter name size: " ) +
                    std::to_string( nBytes ) + std::string( "." ));
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            // Read the parameter name's bytes.
            if ( nBytes >= nameBufferSize )
            {
                // Increase the buffer size to accomodate a larger
                // parameter name.
                delete[] nameBuffer;
                nameBufferSize *= 2;
                nameBuffer = new char[nameBufferSize];
            }

            if ( std::fread( nameBuffer, 1, nBytes, fp ) != nBytes )
            {
                this->appendFileLog(
                    "error",
                    "The file is truncated where the format's parameter name should be." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            nameBuffer[nBytes] = '\0';

            // Save the parameter name.
            fileParameterNames.push_back( nameBuffer );
        }

        //
        // Read the parameter long names.
        // ------------------------------
        // Each parameter is read with:
        // - The number of bytes for the parameter long name.
        //   - 20 bytes, space padded.
        //   - In ASCII.
        // - The parameter long name.
        //   - However many bytes are required.
        //   - In UTF-8.
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter long name length, in bytes.
            if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
            {
                this->appendFileLog(
                    "error",
                     "The file is truncated where the format's parameter long names should be." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[VALUESIZE] = '\0';

            const size_t nBytes = std::stol( buffer );
            if ( nBytes < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file header contains an invalid negative parameter long name size: " ) +
                    std::to_string( nBytes ) + std::string( "." ));
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            else if ( nBytes == 0 )
            {
                // No long parameter name.
                fileParameterLongNames.push_back( std::string( ) );
            }
            else
            {
                // Read the parameter long name's bytes.
                if ( nBytes >= nameBufferSize )
                {
                    // Increase the buffer size to accomodate a larger
                    // parameter long name.
                    delete[] nameBuffer;
                    nameBufferSize *= 2;
                    nameBuffer = new char[nameBufferSize];
                }

                if ( std::fread( nameBuffer, 1, nBytes, fp ) != nBytes )
                {
                    this->appendFileLog(
                        "error",
                        "The file is truncated where the format's parameter long name should be." );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        "The file is missing critical information." +
                        ERROR_CANNOTLOAD );
                }
                nameBuffer[nBytes] = '\0';

                // Save the parameter long name.
                fileParameterLongNames.push_back( nameBuffer );
            }
        }

        delete[] nameBuffer;

        //
        // Read the parameter minimums.
        // ----------------------------
        // Each parameter minimum is read with:
        // - 20 bytes, space padded.
        // - In ASCII.
        std::vector<double> fileParameterMinimums;
        std::vector<double> fileParameterMaximums;

        fileParameterMinimums.reserve( numberOfParameters );
        fileParameterMaximums.reserve( numberOfParameters );

        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter minimum.
            if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
            {
                this->appendFileLog(
                    "error",
                     "The file is truncated where the format's parameter minimum should be." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[VALUESIZE] = '\0';

            const double value = std::stod( buffer );
            fileParameterMinimums.push_back( value );
        }

        //
        // Read the parameter maximums.
        // ----------------------------
        // Each parameter maximum is read with:
        // - 20 bytes, space padded.
        // - In ASCII.
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter maximum.
            if ( std::fread( buffer, 1, VALUESIZE, fp ) != VALUESIZE )
            {
                this->appendFileLog(
                    "error",
                     "The file is truncated where the format's parameter maximum should be." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[VALUESIZE] = '\0';

            const double value = std::stod( buffer );
            fileParameterMaximums.push_back( value );
        }


        //
        // Create empty event table.
        // -------------------------
        // Create a new float or double event table.
        size_t numberOfEvents = (size_t) maximumEvents;
        if ( maximumEvents < 0 ||
             numberOfEvents > this->fileNumberOfEvents )
            numberOfEvents = this->fileNumberOfEvents;

        this->eventTable.reset( new FlowGate::Events::EventTable(
            fileParameterNames,
            numberOfEvents,
            areFloats ) );

        this->eventTable->setParameterLongNames(
            fileParameterLongNames );

        this->eventTable->setNumberOfOriginalEvents( this->fileNumberOfEvents );

        // Set the specified min/max from the file.
        for ( uint32_t i = 0; i < numberOfParameters; ++i )
        {
            this->eventTable->setParameterMinimum( i,
                fileParameterMinimums[i] );
            this->eventTable->setParameterMaximum( i,
                fileParameterMaximums[i] );
        }
    }

    /**
     * Loads events from the file.
     *
     * Events are organized in parameter columns so that all values for a
     * parameter in consecutive locations in the file and in event table
     * memory. Columns of values in the file are in the same order as
     * parameter names from the file's header.
     *
     * All values are either single- or double-precision floating point,
     * depending upon the choice set in the header. Binary values are in
     * least- or most-significant byte first order, also depending upon
     * the choice set in the header.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void loadEvents( std::FILE*const fp )
    {
        const size_t numberOfEvents = this->eventTable->getNumberOfEvents( );
        if ( numberOfEvents == 0 )
            return;

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
              "  Loading file event table:\n";

        //
        // Setup.
        // ------
        // Get the number of parameters and whether to swap bytes.
        const bool truncated = (numberOfEvents != this->fileNumberOfEvents);
        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        // If the current host's byte order does not match that of the
        // data, swap the data.
        std::string byteOrder;
        try
        {
            byteOrder = this->fileAttributes.at( "byteOrder" );
        }
        catch ( ... )
        {
            // Unknown byte order. Default to LSBF.
            byteOrder = "lsbf";
        }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        // Host is LSBF. Swap if the file is not LSBF.
        const bool swap = (byteOrder != "lsbf");
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        // Host is MSBF. Swap if the file is LSBF.
        const bool swap = (byteOrder == "lsbf");
#else
        const bool swap = false;
#endif

        if ( this->verbose == true )
        {
            if ( this->eventTable->areValuesFloats( ) == true )
                std::cerr << this->verbosePrefix << ": " <<
                    "    Loading " << numberOfEvents <<
                    " 32-bit floats for " <<
                    numberOfParameters << " parameters" <<
                    ((swap == true) ? " swapping bytes\n" :
                    " in host byte order\n");
            else
                std::cerr << this->verbosePrefix << ": " <<
                    "    Loading " << numberOfEvents <<
                    " 64-bit doubles for " <<
                    numberOfParameters << " parameters" <<
                    ((swap == true) ? " swapping bytes\n" :
                    " in host byte order\n");
        }


        // Get the current file location, which should be at the start of
        // the event values.
        const long startOfEvents = std::ftell( fp );


        //
        // Read the events.
        // ----------------
        // Events are organized in columns with one column per parameter.
        // Consecutive values in the file are for consecutive events in
        // the same parameter.
        if ( this->eventTable->areValuesFloats( ) == true )
        {
            const size_t nBytesPerParameter =
                this->fileNumberOfEvents * sizeof(float);

            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                // If needed, advance to the start of the parameter's values.
                if ( truncated == true )
                    std::fseek( fp, startOfEvents + i * nBytesPerParameter, SEEK_SET );

                float*const values =
                    this->eventTable->getParameterFloats( i ).data( );

                if ( std::fread( values, sizeof(float), numberOfEvents, fp ) != numberOfEvents )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file is truncated before the end of the event list for parameter " ) +
                        std::to_string( i ) + "." );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        "The file is missing critical information." +
                        ERROR_CANNOTLOAD );
                }

                if ( swap == true )
                {
                    uint32_t* bytes = (uint32_t*)values;
                    for ( size_t j = 0; j < numberOfEvents; ++j )
                    {
#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_BSWAP
                        *bytes = __builtin_bswap32(*bytes);
#else
                        const uint32_t ui = *bytes;
                        *bytes =
                            ((ui & 0xFF000000u) >> 24u) |
                            ((ui & 0x00FF0000u) >>  8u) |
                            ((ui & 0x0000FF00u) <<  8u) |
                            ((ui & 0x000000FFu) << 24u);
#endif
                        ++bytes;
                    }
                }
            }
        }
        else
        {
            const size_t nBytesPerParameter =
                this->fileNumberOfEvents * sizeof(double);

            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                // If needed, advance to the start of the parameter's values.
                if ( truncated == true )
                    std::fseek( fp, startOfEvents + i * nBytesPerParameter, SEEK_SET );

                double*const values =
                    this->eventTable->getParameterDoubles( i ).data( );

                if ( std::fread( values, sizeof(double), numberOfEvents, fp ) != numberOfEvents )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file is truncated before the end of the event list for parameter " ) +
                        std::to_string( i ) + "." );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        "The file is missing critical information." +
                        ERROR_CANNOTLOAD );
                }

                if ( swap == true )
                {
                    uint64_t* bytes = (uint64_t*)values;
                    for ( size_t i = 0; i < numberOfEvents; ++i )
                    {
#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_BSWAP
                        *bytes = __builtin_bswap64(*bytes);
#else
                        const uint64_t ui = *bytes;
                        *bytes =
                            ((ui & 0xFF00000000000000u) >> 56u) |
                            ((ui & 0x00FF000000000000u) >> 40u) |
                            ((ui & 0x0000FF0000000000u) >> 24u) |
                            ((ui & 0x000000FF00000000u) >>  8u) |
                            ((ui & 0x00000000FF000000u) <<  8u) |      
                            ((ui & 0x0000000000FF0000u) << 24u) |
                            ((ui & 0x000000000000FF00u) << 40u) |
                            ((ui & 0x00000000000000FFu) << 56u);
#endif
                        ++bytes;
                    }
                }
            }
        }

        // Compute the data min/max from parameter values.
        this->eventTable->computeParameterDataMinimumMaximum( );
    }
    // @}



//----------------------------------------------------------------------
// Save file.
//----------------------------------------------------------------------
    /**
     * @name Save file
     */
    // @{
public:
    /**
     * Saves the object to a new file.
     *
     * The file is written with data from the current object.
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
     * @see ::getFileLog()
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
                "Saving FlowGate Binary Events file \"" << filePath << "\".\n";

        this->clearFileLog();
        this->fileAttributes.clear( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );

        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( path.empty() == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty file path." );
        if ( this->eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "Invalid NULL event table." ) );

        //
        // Open the file.
        // --------------
        // Open the file for binary buffered writing using stdio.
        // Throw an exception if the file cannot be opened.
        std::FILE*const fp = std::fopen( path.c_str(), "wb" );
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
        // Save the file.
        // --------------
        // Thread-lock the file prior to stdio operations so that stdio
        // doesn't lock repeatedly on every function call.
        //
        // Then save the header with file information, such as
        // the names of parameters. Then save the binary event data.
        try
        {
#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
            // Thread lock during the save. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Save the header. This stores the file format version in use
            // and the names of parameters.
            this->saveHeader( fp, maximumEvents );

            // Write the data.
            this->saveEvents( fp, maximumEvents );

#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }

private:
    /**
     * Saves the file header containing the version number and parameter names.
     *
     * The file header is in ASCII and contains:
     * - The file format's version number.
     * - The byte order.
     * - The data type.
     * - The number of events.
     * - The number of parameters.
     * - A list of parameter names, containing:
     *   - The number of BYTES of the name.
     *   - The UTF-8 bytes for the name.
     * - A list of parameter long names, containing:
     *   - The number of BYTES of the name.
     *   - The UTF-8 bytes for the name.
     * - A list of parameter minimum values.
     * - A list of parameter maximum values.
     *
     * Each value, except parameter names and the versino number, is in a
     * field of 20 bytes, left-adjusted and space padded.
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
     * @see ::getFileLog()
     */
    void saveHeader( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving file header:\n";

        size_t numberOfEvents = this->eventTable->getNumberOfEvents( );
        if ( maximumEvents >= 0 && (size_t) maximumEvents < numberOfEvents )
            numberOfEvents = maximumEvents;


        static const int VALUESIZE = 20;
        static const int VERSIONSIZE = 40;
        int status = 0;


        //
        // Write the version number.
        // ------------------------
        // - 40 bytes, space padded.
        // - In ASCII.
        // - Has the format "FlowGateBinaryEvents_" followed by
        //   major.minor.subminor (e.g. "FlowGateBinaryEvents_1.0.0").
        std::string version = "FlowGateBinaryEvents_1.0.0";
        this->fileAttributes.emplace( "versionNumber", version );
        this->fileAttributes.emplace( "numberFormat", "binary" );

        if ( std::fprintf( fp, "%-*s", VERSIONSIZE, version.c_str( ) ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header file format version number: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << version << "\"\n";

        //
        // Write the byte order.
        // ---------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        // - One of "lsbf" or "msbf".
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        this->fileAttributes.emplace( "byteOrder", "lsbf" );
        const bool fileIsLSBF = true;
        status = std::fprintf( fp, "%-*s", VALUESIZE, "lsbf" );
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        this->fileAttributes.emplace( "byteOrder", "msbf" );
        const bool fileIsLSBF = false;
        status = std::fprintf( fp, "%-*s", VALUESIZE, "msbf" );
#endif

        if ( status < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header file format byte order: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number format" << "binary\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Byte order:" <<
                (fileIsLSBF == true ?
                    "Least-significant byte first.\n" :
                    "Most-significant byte first.\n");
        }


        //
        // Write the data type.
        // --------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        // - One of "float" or "double".
        if ( this->eventTable->areValuesFloats( ) == true )
        {
            this->fileAttributes.emplace( "dataType", "float" );
            status = std::fprintf( fp, "%-*s", VALUESIZE, "float" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "floats\n";
        }
        else
        {
            this->fileAttributes.emplace( "dataType", "double" );
            status = std::fprintf( fp, "%-*s", VALUESIZE, "double" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "doubles\n";
        }

        if ( status < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header data type: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }


        //
        // Write the number of events.
        // ---------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        this->fileAttributes.emplace( "numberOfEvents",
            std::to_string( numberOfEvents ) );

        if ( std::fprintf( fp, "%-*ld", VALUESIZE, numberOfEvents ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header number of events: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of events:" <<
                numberOfEvents << "\n";


        //
        // Write the number of parameters.
        // -------------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        const size_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        if ( std::fprintf( fp, "%-*ld", VALUESIZE, numberOfParameters ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header number of parameters: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of parameters:" <<
                numberOfParameters << "\n";

        //
        // Write the parameter primary (short) names.
        // ------------------------------------------
        // Each parameter is saved with:
        // - The number of bytes for the parameter name.
        //   - 20 bytes, space padded.
        //   - In ASCII.
        // - The parameter name.
        //   - However many bytes are required.
        //   - In UTF-8.
        auto parameterNames = this->eventTable->getParameterNames( );

        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            const auto name = parameterNames[i];
            const size_t nBytes = name.size( );
            if ( std::fprintf( fp, "%-*ld", VALUESIZE, nBytes ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file header parameter names: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
            if ( std::fwrite( name.c_str( ), 1, nBytes, fp ) != nBytes )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file header file parameter names: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
        }

        //
        // Write the parameter long names.
        // -------------------------------
        // Each parameter is saved with:
        // - The number of bytes for the parameter name.
        //   - 20 bytes, space padded.
        //   - In ASCII.
        // - The parameter name.
        //   - However many bytes are required.
        //   - In UTF-8.
        auto parameterLongNames = this->eventTable->getParameterLongNames( );

        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            const auto name = parameterLongNames[i];
            const size_t nBytes = name.size( );
            if ( std::fprintf( fp, "%-*ld", VALUESIZE, nBytes ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file header parameter long names: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
            if ( std::fwrite( name.c_str( ), 1, nBytes, fp ) != nBytes )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file header file parameter long names: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
        }

        //
        // Write the parameter minimums.
        // -----------------------------
        // Each parameter is saved with a minimum value.
        // - 20 bytes, space padded.
        // - In ASCII.
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            const auto value = this->eventTable->getParameterMinimum( i );
            if ( std::fprintf( fp, "%-*.*le", VALUESIZE, VALUESIZE-8, value ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file header parameter minimum: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
        }

        //
        // Write the parameter maximums.
        // -----------------------------
        // Each parameter is saved with a maximum value.
        // - 20 bytes, space padded.
        // - In ASCII.
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            const auto value = this->eventTable->getParameterMaximum( i );
            if ( std::fprintf( fp, "%-*.*le", VALUESIZE, VALUESIZE-8, value ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file header parameter maximum: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
        }
    }

    /**
     * Saves event data.
     *
     * Events are organized in parameter columns so that all values for a
     * parameter in consecutive locations in the file and in event table
     * memory. Columns of values in the file are in the same order as
     * parameter names from the file's header.
     *
     * All values are either single- or double-precision floating point,
     * depending upon the choice set in the header. Binary values are in
     * least- or most-significant byte first order, also depending upon
     * the choice set in the header.
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
     * @see ::getFileLog()
     */
    void saveEvents( std::FILE*const fp, const ssize_t maximumEvents = (-1) )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving file event table:\n";

        //
        // Setup.
        // ------
        // Get the number of parameters and events.
        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        size_t numberOfEvents = this->eventTable->getNumberOfEvents( );
        if ( maximumEvents >= 0 && (size_t) maximumEvents < numberOfEvents )
            numberOfEvents = maximumEvents;

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "    Writing " << numberOfEvents << " values for " <<
                numberOfParameters << " parameters\n";


        //
        // Write the values.
        // -----------------
        // All data values have the same 32-bit or 64-bit floating-point
        // data type.
        if ( this->eventTable->areValuesFloats( ) == true )
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                const float*const values =
                    this->eventTable->getParameterFloats( i ).data( );

                if ( std::fwrite( values, sizeof(float), numberOfEvents, fp ) != numberOfEvents )
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
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                const double*const values =
                    this->eventTable->getParameterDoubles( i ).data( );

                if ( std::fwrite( values, sizeof(double), numberOfEvents, fp ) != numberOfEvents )
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

#undef FLOWGATE_FILEFGBINARYEVENTS_USE_FLOCKFILE
#undef FLOWGATE_FILEFGBINARYEVENTS_USE_BSWAP
