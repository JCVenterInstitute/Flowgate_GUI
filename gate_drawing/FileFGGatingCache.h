/**
 * @file
 * Loads and saves a flow cytometry gating cache in the custom FlowGate gating
 * cache file format.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "GatingCache.h"
 * @li FlowGate "EventTable.h" (via "GatingCache.h")
 * @li FlowGate "GateTrees.h" (via "GatingCache.h")
 * @li FlowGate "FileGatingML.h"
 * @li FlowGate "FileFGJsonGates.h"
 * @li RapidXML (via "FileGatingML.h")
 * @li Gason (via "FileFGJsonGates.h")
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
#include "GatingCache.h" // Gating state, including source events & tree
#include "FileGatingML.h" // Gating-ML syntax support
#include "FileFGJsonGates.h" // JSON syntax support





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
#define FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
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
#define FLOWGATE_FILEFGGATINGCACHE_USE_BSWAP
#elif defined(__clang__) && __has_builtin(__builtin_bswap16__) && \
    __has_builtin(__builtin_bswap32__) && \
    __has_builtin(__builtin_bswap64__)
#define FLOWGATE_FILEFGGATINGCACHE_USE_BSWAP
#endif





namespace FlowGate {
namespace File {





/**
 * Loads and saves a flow cytometry gating cache in the custom FlowGate
 * gating cache file format.
 *
 * The FlowGate project has defined a "FlowGate Gating Cache" binary file
 * format for the storage of (1) source event data, (2) a gate tree, and
 * (3) post-gate-transform and post-gating event data. This data is
 * specific to FlowGate's gating algorithms and the state it maintains.
 * This file format has the following well-known revisions:
 * @li 1.0 from 2020.
 *
 * The source event data stored within a FlowGate Gating Cache file is related
 * to event data stored in the standard ISAC FCS file format. However, FCS
 * only stores pre-compensation, pre-transform, and pre-gating events. This
 * file stores the same data PLUS post-transform and post-gating data created
 * through the gating process. This file format also stores the gate tree
 * involved.
 *
 *
 * <B>File format</B><BR>
 * Each FlowGate Gating Cache file contains:
 *
 * @li A brief text header that includes the file format version number,
 *   the number of gated parameters and events, and the names of all gated
 *   parameters.
 *
 * @li A gate tree in Gating-ML text syntax.
 *
 * @li A binary table of gated events. Each event has a list of parameter
 *   columns containing numeric data.
 *
 * @li a binary table of gate containment boolean flags, with one table
 *   row for each event. Table columns correspond to the number of gates
 *   in the gate tree.
 *
 * To make it possible to skip sections of the file, or only read a portion
 * of the event and flag tables, byte offsets to each of the secions are
 * stored in the file's header.
 *
 *
 * <B>Loading files</B><BR>
 * A file may be loaded by providing a file path to the constructor.
 * @code
 * auto file = new FileFGGatingCache( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFGGatingCache( );
 * file->load( filepath );
 * #endcode
 *
 * The same data object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the data object
 * is deleted automatically:
 * @code
 * auto file = new FileFGGatingCache( );
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
 * <B>Getting the gate trees</B><BR>
 *
 *
 * <B>Getting parameter names</B><BR>
 *
 *
 * <B>Getting gated events</B><BR>
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
class FileFGGatingCache final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate Gating Cache files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "FlowGate Gating Cache";

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
        "FileFGGatingCache";

    /**
     * The field size, in bytes, for header values.
     */
    inline static const int HEADER_VALUE_WIDTH = 20;

    /**
     * The field size, in bytes, for the header's version name/number.
     */
    inline static const int HEADER_VERSION_WIDTH = 40;

    /**
     * The gate tree format for Gating-ML data.
     */
    inline static const std::string GATE_TREES_GATINGML_FORMAT = "xml";

    /**
     * The gate tree format for JSON data.
     */
    inline static const std::string GATE_TREES_JSON_FORMAT = "json";

    /**
     * The default gate tree format.
     */
    inline static const std::string DEFAULT_GATE_TREES_FORMAT = "json";



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
// Classes.
//----------------------------------------------------------------------
private:
    class Parameter final
    {
    public:
        /**
         * The primary (short) parameter name.
         */
        std::string name;

        /**
         * The long parameter name.
         */
        std::string longName;

        /**
         * The minimum parameter value.
         *
         * For a source event table, this is the minimum source event
         * value. For a gate's event table, this is the minimum post-transform
         * event value.
         */
        double minimum;

        /**
         * The maximum parameter value.
         *
         * For a source event table, this is the maximum source event
         * value. For a gate's event table, this is the maximum post-transform
         * event value.
         */
        double maximum;
    };

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
     * The current file's size, in bytes.
     *
     * This value is only used during file loading. Before and after loading,
     * the field's value is undefined.
     *
     * @see ::load()
     */
    long fileSize = 0L;

    /**
     * The current file's byte order.
     *
     * @see ::load()
     */
    bool fileIsLSBF = true;

    /**
     * The current file's number of events.
     *
     * This is a redundant value. It is initialized when a file header is
     * loaded, and becomes redundant when a file's data is loaded into an
     * event table. It is only used while there is no event table.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGatingCache()
     * @see ::setGatingCache()
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
     * @li "numberOfEvents" (long) The number of events in the file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getFileAttributes()
     */
    std::map<const std::string, const std::string> fileAttributes;

    /**
     * The current file's primary (short) parameter names.
     *
     * This is a temporary value used during file loads. After a file
     * load, it is no longer used.
     *
     * @see ::loadEventTableHeader()
     */
    std::vector<std::string> fileParameterNames;

    /**
     * The current file's long parameter names.
     *
     * This is a temporary value used during file loads. After a file
     * load, it is no longer used.
     *
     * @see ::loadEventTableHeader()
     */
    std::vector<std::string> fileParameterLongNames;

    /**
     * The current file's minimum parameter values.
     *
     * This is a temporary value used during file loads. After a file
     * load, it is no longer used.
     *
     * @see ::loadEventTableHeader()
     */
    std::vector<double> fileParameterMinimums;

    /**
     * The current file's maximum parameter values.
     *
     * This is a temporary value used during file loads. After a file
     * load, it is no longer used.
     *
     * @see ::loadEventTableHeader()
     */
    std::vector<double> fileParameterMaximums;

    /**
     * The current file's source event table parameter attributes.
     *
     * When loading a file, this list is initialized from the loaded data
     * and get*() methods return values from this list.
     *
     * When saving a file, this list is not used and get*() methods return
     * values from the gating cache table being saved.
     *
     * @see ::load()
     * @see ::save()
     */
    std::vector<Parameter> fileParameters;

    /**
     * The current file's gate trees format.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getFileGateTreesFormat()
     */
    std::string fileGateTreesFormat;

    /**
     * The current file's gate trees.
     *
     * This is only used during loading. It stores the loaded gate trees
     * before a gating cache object can be created that uses them.
     *
     * @see ::load()
     */
    std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees;

    /**
     * The current file's per-gate event table parameter attributes.
     *
     * When loading a file, this list is initialized from the loaded data
     * and get*() methods return values from this list.
     *
     * When saving a file, this list is not used and get*() methods return
     * values from the gating cache table being saved.
     *
     * @see ::load()
     * @see ::save()
     */
    std::vector<std::vector<Parameter>> fileGateParameters;

    // File offsets ----------------------------------------------------
    /**
     * The file byte offset to the first byte offset in the header.
     *
     * This is a temporary value only used during file saving. It records
     * the start of the file offsets section of the file header. These
     * offsets must be filled in after the rest of the file has been saved.
     *
     * @see ::save()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileOffsetToHeaderOffsets;

    /**
     * The file byte offset to the start of the source event table.
     *
     * This value is only used during file loading and saving.
     *
     * @see ::load()
     * @see ::save()
     * @see ::loadHeader()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileOffsetToSourceEventTable;

    /**
     * The length, in bytes, of the source event table in the file.
     *
     * This value is only used during file loading and saving.
     *
     * @see ::load()
     * @see ::save()
     * @see ::loadHeader()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileLengthOfSourceEventTable;

    /**
     * The file byte offset to the start of the gate trees description.
     *
     * This value is only used during file loading and saving.
     *
     * @see ::load()
     * @see ::save()
     * @see ::loadHeader()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileOffsetToGateTrees;

    /**
     * The length, in bytes, of the gate trees in the file.
     *
     * This value is only used during file loading and saving.
     *
     * @see ::load()
     * @see ::save()
     * @see ::loadHeader()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileLengthOfGateTrees;

    /**
     * The file byte offset to the start of gate state.
     *
     * This value is only used during file loading and saving.
     *
     * @see ::load()
     * @see ::save()
     * @see ::loadHeader()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileOffsetToGateState;

    /**
     * The length, in bytes, of the gate state in the file.
     *
     * This value is only used during file loading and saving.
     *
     * @see ::load()
     * @see ::save()
     * @see ::loadHeader()
     * @see ::saveHeader()
     * @see ::saveHeaderOffsets()
     */
    long fileLengthOfGateState;



    // Cache data ------------------------------------------------------
    /**
     * The gating cache.
     *
     * @see ::getGatingCache()
     * @see ::setGatingCache()
     */
    std::shared_ptr<FlowGate::Gates::GatingCache> gatingCache;



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
        extensions.push_back( "fgcache" );
        extensions.push_back( "cache" );
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
     * Constructs a new object with no gating cache.
     *
     * The new object has has no gating cache. These may be set later by
     * set*() methods or by loading data from a file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGatingCache()
     * @see ::setGatingCache()
     */
    FileFGGatingCache( )
        noexcept
    {
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
    }



    // Copy constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with data from the given
     * FileFGGatingCache object.
     *
     * The new object shares the gating cache. External changes to
     * the gating cache will affect this object as well.
     *
     * @param[in] file
     *   The FileFGGatingCache object.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGatingCache()
     * @see ::setGatingCache()
     */
    FileFGGatingCache( const FileFGGatingCache*const file ) = delete;

    /**
     * Constructs a new object initialized with data from the given
     * FileFGGatingCache object.
     *
     * The new object shares the gating cache. External changes to
     * the gating cache will affect this object as well.
     *
     * @param[in] file
     *   The FileFGGatingCache object.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGatingCache()
     * @see ::setGatingCache()
     */
    FileFGGatingCache( const FileFGGatingCache& file )
        noexcept = delete;



    // Event table constructors ----------------------------------------
    /**
     * Constructs a new object using the given gating cache.
     *
     * The new object shares the given gating cache. External changes to
     * the gating cache will affect this object as well.
     *
     * @param[in] gatingCache
     *   The gating cache to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gating cache is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGatingCache()
     * @see ::setGatingCache()
     */
    FileFGGatingCache(
        std::shared_ptr<FlowGate::Gates::GatingCache> gatingCache )
    {
        if ( gatingCache == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gating cache." );

        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
        this->setGatingCache( gatingCache );
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
     * @see ::getGatingCache()
     * @see ::setGatingCache()
     */
    FileFGGatingCache( const std::string& path )
    {
        if ( path.empty( ) == 0 )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty path." );

        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

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
    virtual ~FileFGGatingCache( )
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
     * Returns the file gate tree format.
     *
     * When a file is loaded, this value is set to the format for the stored
     * gate tree. This always has one of three values:
     * @li "xml" for Gating-ML XML.
     * @li "json" for JSON.
     *
     * When a file is saved, this value is set to the gate tree format
     * saved to the file.
     *
     * @return
     *   Returns the gate tree format of file data.
     *
     * @see ::load()
     * @see ::save()
     */
    inline const std::string getFileGateTreesFormat( )
        const noexcept
    {
        return this->fileGateTreesFormat;
    }



    // Paths -----------------------------------------------------------
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
            return this->fileAttributes.at( "path" );
        }
        catch ( ... )
        {
            // No path yet.
            return std::string( );
        }
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
     * Returns the number of events.
     *
     * If a file has not been loaded and a gated event table has not been
     * initialized, then a zero is returned.
     *
     * @return
     *   Returns the number of events.
     *
     * @see ::areValuesFloats()
     * @see ::getGatingCache()
     * @see ::load()
     */
    inline size_t getNumberOfEvents( )
        const noexcept
    {
        if ( this->gatingCache != nullptr )
            return this->gatingCache->getSourceEventTable()->getNumberOfEvents( );
        try
        {
            auto n = this->fileAttributes.at( "numberOfEvents" );
            if ( n.empty( ) == true )
                return 0;
            return std::stol( n );
        }
        catch ( ... )
        {
            // No events yet.
            return 0;
        }
    }
    // @}



//----------------------------------------------------------------------
// Reset
//----------------------------------------------------------------------
public:
    /**
     * @name Reset
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
        this->fileGateTreesFormat = "";
        this->fileAttributes.clear( );
        this->fileGateParameters.clear();
        this->gatingCache.reset( );
        this->gateTrees.reset( );
    }
    // @}



//----------------------------------------------------------------------
// Gating cache
//----------------------------------------------------------------------
public:
    /**
     * @name Gating cache
     */
    // @{
    /**
     * Gets the gating cache in use.
     *
     * If a file has not been loaded and a gating cache has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the gating cache.
     *
     * @see ::setGatingCache()
     * @see ::load()
     */
    inline std::shared_ptr<const FlowGate::Gates::GatingCache> getGatingCache( )
        const
    {
        return this->gatingCache;
    }

    /**
     * Gets the gating cache in use.
     *
     * If a file has not been loaded and a gating cache has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the gating cache.
     *
     * @see ::setGatingCache()
     * @see ::load()
     */
    inline std::shared_ptr<FlowGate::Gates::GatingCache> getGatingCache( )
    {
        return this->gatingCache;
    }

    /**
     * Gets the gate trees in use.
     *
     * If a file has not been loaded, but a gating cache has been set,
     * then this returns the gate trees from that cache.
     *
     * If a file has been loaded, including partial file loads, then
     * this returns the loaded gate trees.
     *
     * Otherwise, when no file has been loaded and there is no gating cache
     * set, this returns a NULL pointer.
     *
     * @return
     *   Returns a shared pointer for the gate trees.
     *
     * @see ::getGatingCache()
     * @see ::load()
     */
    inline std::shared_ptr<const FlowGate::Gates::GateTrees> getGateTrees( )
        const
    {
        if ( this->gatingCache != nullptr )
            return this->gatingCache->getGateTrees( );
        return this->gateTrees;
    }

    /**
     * Gets the gate trees in use.
     *
     * If a file has not been loaded, but a gating cache has been set,
     * then this returns the gate trees from that cache.
     *
     * If a file has been loaded, including partial file loads, then
     * this returns the loaded gate trees.
     *
     * Otherwise, when no file has been loaded and there is no gating cache
     * set, this returns a NULL pointer.
     *
     * @return
     *   Returns a shared pointer for the gate trees.
     *
     * @see ::getGatingCache()
     * @see ::load()
     */
    inline std::shared_ptr<FlowGate::Gates::GateTrees> getGateTrees( )
    {
        if ( this->gatingCache != nullptr )
            return this->gatingCache->getGateTrees( );
        return this->gateTrees;
    }

    /**
     * Resets the object and sets the gated event table to use.
     *
     * The prior event table, if any, is released. The new gated event table
     * will be used for all further operations, such as saving to a file.
     *
     * @param[in] eventTable
     *   The event table to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gated event table is a nullptr.
     *
     * @see ::reset()
     * @see ::save()
     */
    inline void setGatingCache(
        std::shared_ptr<FlowGate::Gates::GatingCache>& gatingCache )
    {
        if ( gatingCache == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gating cache." );

        this->reset( );
        this->gatingCache = gatingCache;

        const auto eventTable = gatingCache->getSourceEventTable( );
        this->fileNumberOfEvents = eventTable->getNumberOfEvents( );
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
     * @param[in] filePath
     *   The path to the file to load.
     * @param[in] loadData
     *   (optional, default = true) When true, all data in the file is
     *   loaded. When false, only the header and gate trees are loaded and
     *   validated. No event or gate state data is loaded.
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
        const bool loadData = true )
    {
        if ( filePath == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file path." );

        this->load( std::string( filePath ), loadData );
    }

    /**
     * Loads the indicated file's data.
     *
     * The file is loaded into the current object, clearing out any prior
     * data. Upon completion, each of the "get" methods in this class will
     * return information loaded from the file.
     *
     * @param[in] filePath
     *   The path to the file to load.
     * @param[in] loadData
     *   (optional, default = true) When true, all data in the file is
     *   loaded. When false, only the header and gate trees are loaded and
     *   validated. No event or gate state data is loaded.
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
        const bool loadData = true )
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
                "Loading FlowGate Gating Cache file \"" << filePath << "\".\n";
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
#ifdef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
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

            // Load the header. This provides the file format version in use,
            // byte order, etc. This does not load the source event table,
            // gate trees, or gate state. Since the source event table and
            // state are the data that have parameters, and their names,
            // loading the header does not load any parameter information.
            this->loadHeader( fp );

            // Always read the gate trees. This allows a caller to load
            // and extract the gate tree from a gating cache without also
            // loading all the rest of the event data.
            this->loadGateTrees( fp );

            // Load the source event table. When loadData === TRUE, a new
            // event table is returned. Otherwise a NULL is returned. In
            // either case, the table's parameters and associated attributes
            // are loaded into fields that can be queried after loading.
            auto table = this->loadSourceEventTable( fp, loadData );

            // Create the gating cache. This attaches the current event
            // table as the cache's source event table. This also attaches
            // the current gate trees and then constructs gate state
            // objects for all gates. Those gate objects are each
            // event tables for the gate's parameters.
            this->gatingCache.reset( new FlowGate::Gates::GatingCache(
                table,
                this->gateTrees ) );

            // Load the gate state. This is another event table per gate.
            // The table includes the transformed parameter values used
            // by the gate, plus a column of inclusion flags.
            this->loadGateStates( fp, loadData );

#ifdef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }

private:
    /**
     * Loads the file header containing the version number and byte offsets.
     *
     * The file header is in ASCII and contains:
     * @li The file format's version string.
     * @li The byte order.
     * @li The event data type.
     * @li The number of events.
     * @li The gate trees format.
     * @li The file byte offset to the gate trees.
     * @li The file byte offset to the gates applied flags table.
     * @li The file byte offset to the transformed gates flags table.
     * @li The file byte offset to the event table.
     * @li The file byte offset to the within gates flags table.
     *
     * Each value is in a field of 20 bytes, left-adjusted and space padded.
     * 20 bytes is sufficient to record a 64-bit integer in base-10 ASCII
     * digits. The exception is the file format version string, which is in
     * a field of 40 bytes.
     *
     * This method reads the header, leaving the file pointer on the first
     * byte after the header. All header values are read, parsed, and saved
     * into this object.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::load()
     * @see ::saveHeader()
     */
    void loadHeader( std::FILE*const fp )
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

        char buffer[HEADER_VERSION_WIDTH + 1];
        buffer[HEADER_VERSION_WIDTH] = '\0';

        //
        // Validate file size.
        // -------------------
        // The file header contains at least the following, in ASCII:
        // - 40 bytes for the file version number.
        // - 20 bytes for the byte order.
        // - 20 bytes for the data type.
        // - 20 bytes for the number of events.
        // - 20 bytes for the number of parameters.
        // ----
        // - 120 bytes
        if ( this->fileSize != 0 && this->fileSize < 120 )
        {
            this->appendFileLog(
                "error",
                "The file is too short to be a valid FlowGate Gating Cache file. A header of at least 120 bytes is required." );
            throw std::runtime_error(
                std::string( "Unrecognized data file format.\n" ) +
                std::string( "The file does not use a recognized format for FlowGate binary flow cytometry data, such as the FlowGate Gating Cache format." ) +
                ERROR_CANNOTLOAD );
        }

        //
        // Read the version number.
        // ------------------------
        // - 40 bytes, space padded.
        // - In ASCII.
        // - Has the format "FlowGateGatingCache_" followed by
        //   major.minor.subminor (e.g. "FlowGateGatingCache_1.0.0").
        if ( std::fread( buffer, 1, HEADER_VERSION_WIDTH, fp ) != HEADER_VERSION_WIDTH )
        {
            this->appendFileLog(
                "error",
                "A read error occurred while trying to read the first 40 bytes of the file, which should contain the FlowGate Gating Cache format's version number." );
            throw std::runtime_error(
                ERROR_READ +
                ERROR_CANNOTLOAD );
        }

        if ( std::strncmp( "FlowGateGatingCache_1.0.0", buffer, 25 ) != 0 )
        {
            this->appendFileLog(
                "error",
                "The file is not an FlowGate Gating Cache file. It does not start with \"FlowGateGatingCache_1.0.0\"." );
            throw std::runtime_error(
                std::string( "Unrecognized data file format.\n" ) +
                std::string( "The file does not use a recognized format for FlowGate binary flow cytometry data, such as the FlowGate Gating Cache format." ) +
                ERROR_CANNOTLOAD );
        }

        // Trim off trailing blanks.
        int i = HEADER_VERSION_WIDTH - 1;
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
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the format's byte order should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';

        if ( std::strncmp( buffer, "lsbf", 4 ) == 0 )
        {
            this->fileIsLSBF = true;
            this->fileAttributes.emplace( "byteOrder", "lsbf" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Byte order:" << "Least-significant byte first.\n";
        }
        else if ( std::strncmp( buffer, "msbf", 4 ) == 0 )
        {
            this->fileIsLSBF = false;
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
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the format's data type should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';

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
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the format's number of events should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';

        this->fileNumberOfEvents = 0;;
        try
        {
            this->fileNumberOfEvents = std::stol( buffer );
            this->fileAttributes.emplace( "numberOfEvents",
                std::to_string( this->fileNumberOfEvents ) );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable number of events." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

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
        // Read the gate trees format.
        // ---------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file is truncated where the gate tree format should be." );
            throw std::runtime_error(
                ERROR_READ +
                ERROR_CANNOTLOAD );
        }

        // Trim off trailing blanks.
        i = HEADER_VALUE_WIDTH - 1;
        while ( buffer[i] == ' ' )
            --i;
        buffer[i+1] = '\0';

        // Save the gate tree format.
        this->fileGateTreesFormat = std::string( buffer );
        this->fileAttributes.emplace( "gateTreesFormat", buffer );

        if ( this->fileGateTreesFormat != GATE_TREES_JSON_FORMAT &&
             this->fileGateTreesFormat != GATE_TREES_GATINGML_FORMAT )
        {
            this->appendFileLog(
                "error",
                std::string( "The file uses an unrecognized gate tree format: \"" ) +
                this->fileGateTreesFormat + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_READ +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Gate trees format:" <<
                "\"" << buffer << "\"\n";


        //
        // Read the byte offsets to file data.
        // -----------------------------------
        // - 20 bytes, space padded, offset to the source event table.
        // - 20 bytes, space padded, length of the source event table.
        // - 20 bytes, space padded, offset to the gate trees.
        // - 20 bytes, space padded, length of the gate trees.
        // - 20 bytes, space padded, offset to the gate state.
        // - 20 bytes, space padded, length of the gate state.
        // - In ASCII.
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's byte offset to the source event table should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';
        try
        {
            this->fileOffsetToSourceEventTable = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable offset to the start of the event table.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's byte length of the source event table should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';
        try
        {
            this->fileLengthOfSourceEventTable = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable length of the event table.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's byte offset to the gate trees should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';
        try
        {
            this->fileOffsetToGateTrees = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable offset to the start of the gate tree.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's byte length of the gate trees should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';
        try
        {
            this->fileLengthOfGateTrees = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable length of the gate tree.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's byte offset to the gate state should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';
        try
        {
            this->fileOffsetToGateState = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable offset to the start of the gate state.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                "The file header is truncated where the format's byte length of the gate state should be." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';
        try
        {
            this->fileLengthOfGateState = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable length of the gate state.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->fileOffsetToSourceEventTable <= 0 ||
             this->fileOffsetToGateTrees <= 0 ||
             this->fileOffsetToGateState <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header contains an invalid non-positive number for a file section offset." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->fileLengthOfSourceEventTable <= 0 ||
             this->fileLengthOfGateTrees <= 0 ||
             this->fileLengthOfGateState <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header contains an invalid non-positive number for a file section length." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File offset to event table:" <<
                this->fileOffsetToSourceEventTable << "\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File length of event table:" <<
                this->fileLengthOfSourceEventTable << "\n";

            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File offset to gate trees:" <<
                this->fileOffsetToGateTrees << "\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File length of gate trees:" <<
                this->fileLengthOfGateTrees << "\n";

            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File offset to gate state:" <<
                this->fileOffsetToGateState << "\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File length of gate state:" <<
                this->fileLengthOfGateState << "\n";
        }
    }

    /**
     * Loads and creates the source event table.
     *
     * The file location is advanced to the start of the source event table.
     *
     * An event table is created using the previously saved header values,
     * including parameter names, long names, and min/max. The table is
     * then initialized using data loaded from the file. The new event
     * table is returned.
     *
     * On completion, the file location is left just after the end of the
     * source event table.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] loadData
     *   (optional, default = true) When true, all data in the file is
     *   loaded. When false, only the header and gate trees are loaded and
     *   validated. No event data is loaded.
     *
     * @returns
     *   The new event table is returned.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     */
    std::shared_ptr<FlowGate::Events::EventTableInterface> loadSourceEventTable(
        std::FILE*const fp,
        const bool loadData = true )
    {
        // Seek to the start of the source event table.
        if ( std::fseek( fp, this->fileOffsetToSourceEventTable, SEEK_SET ) < 0 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated before the start of the source event table." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        const auto name = std::string( "source event table" );

        // Load the event table header to get the table's parameters.
        // This sets a variety of file* field values to hold values pending
        // their use in creating an event table below.
        this->loadEventTableHeader( fp, name );

        // Create a new float or double event table. The table's values
        // are not initialized since we're about to set them from the
        // file's data.
        bool areFloats = true;
        try
        {
            areFloats = (this->fileAttributes.at( "dataType" ) == "float");
        }
        catch ( ... )
        {
            // Unknown. Default to floats.
        }

        std::shared_ptr<FlowGate::Events::EventTableInterface> eventTable;
        eventTable.reset( new FlowGate::Events::EventTable(
            this->fileParameterNames,
            this->fileNumberOfEvents,
            areFloats ) );

        // Parameter names were set during construction. Add long names.
        eventTable->setParameterLongNames( this->fileParameterLongNames );

        eventTable->setNumberOfOriginalEvents( this->fileNumberOfEvents );

        // Set the specified min/max from the file.
        const FlowGate::Gates::DimensionIndex numberOfParameters =
            eventTable->getNumberOfParameters( );

        for ( FlowGate::Gates::DimensionIndex i = 0; i < numberOfParameters; ++i )
        {
            eventTable->setParameterMinimum( i,
                this->fileParameterMinimums[i] );
            eventTable->setParameterMaximum( i,
                this->fileParameterMaximums[i] );
        }

        // The temporary parameter names, etc., are no longer needed.
        this->fileParameterNames.clear( );
        this->fileParameterLongNames.clear( );
        this->fileParameterMinimums.clear( );
        this->fileParameterMaximums.clear( );

        if ( loadData == true )
        {
            // Load events into the event table.
            this->loadEventTableValues( fp, eventTable, name );
        }

        return eventTable;
    }

    /**
     * Loads gate state for all gates.
     *
     * The file location is advanced to the start of the gate state.
     *
     * The current gating cache is updated by looping over all gates
     * and loading their state from the file. That state includes event
     * table values for the gate and additional flags.
     *
     * On completion, the file location is left just after the end of the
     * gate state.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] loadData
     *   (optional, default = true) When true, all data for the gates are
     *   loaded. When false, only the header and gate trees are loaded and
     *   validated. No event data is loaded.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     */
    void loadGateStates(
        std::FILE*const fp,
        const bool loadData = true )
    {
        // Seek to the start of the gate state.
        if ( std::fseek( fp, this->fileOffsetToGateState, SEEK_SET ) < 0 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated before the start of the gate state." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        // Get a list of all gates.
        const auto gates = this->gateTrees->findDescendentGates( );

        // For each gate, load its event table.
        for ( auto& gate : gates )
        {
            const auto name = std::string( "gate " ) + gate->getName( );

            // The gate's state has already been created when the gating
            // cache was created, starting with the source event table and
            // gate trees. Get the gate state.
            auto state = gate->getState( );
            auto table = std::dynamic_pointer_cast<FlowGate::Events::EventTableInterface>( state );

            const size_t numberOfEvents = table->getNumberOfEvents( );

            // Load the event table header to get the table's parameters.
            // This sets a variety of file* field values to hold values pending
            // their use in creating an event table.
            this->loadEventTableHeader( fp, name );

            // Load additional header data for the gate.
            this->loadGateHeader( fp, state, name );

            // Set the long parameter names from the header.
            table->setParameterLongNames( this->fileParameterLongNames );

            // Set the specified min/max from the file.
            const FlowGate::Gates::DimensionIndex numberOfParameters =
                table->getNumberOfParameters( );

            for ( FlowGate::Gates::DimensionIndex i = 0; i < numberOfParameters; ++i )
            {
                table->setParameterMinimum( i,
                    this->fileParameterMinimums[i] );
                table->setParameterMaximum( i,
                    this->fileParameterMaximums[i] );
            }

            if ( loadData == true )
            {
                // The gate state is a subclass of EventTable. Load event
                // values directly into it.
                this->loadEventTableValues( fp, table, name );

                // Load additional data for the gate state.
                this->loadGateValues( fp, state, name );
            }
            else
            {
                // Since we aren't reading the event and flag values,
                // we have to skip past them to the next gate.
                const size_t skipBytes =
                    ((table->areValuesFloats( ) == true) ?
                     sizeof(float) : sizeof(double)) *
                    numberOfParameters *
                    numberOfEvents +
                    sizeof(uint8_t) * numberOfEvents;

                if ( std::fseek( fp, skipBytes, SEEK_CUR ) < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file is truncated before the start of gate state for " ) +
                        name );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }
            }
        }
    }

    /**
     * Loads the event table header containing the parameter names.
     *
     * The file header is in ASCII and contains:
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
     * Each value, except parameter names, is in a field of 20 bytes,
     * left-adjusted and space padded. 20 bytes is sufficient to record a
     * 64-bit integer in base-10 ASCII digits.
     *
     * This method reads the header, leaving the file location on the first
     * byte after the header. All header values are read, parsed, and saved
     * into this object.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] eventTableName
     *   The name of the event table, used in verbose output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::load()
     * @see ::saveEventTableHeader()
     */
    void loadEventTableHeader(
        std::FILE*const fp,
        const std::string eventTableName )
    {
        char buffer[HEADER_VALUE_WIDTH + 1];

        //
        // Read the number of parameters.
        // ------------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header is truncated where the " ) +
                eventTableName +
                std::string( " number of parameters should be." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';

        size_t numberOfParameters = 0;
        try
        {
            numberOfParameters = std::stol( buffer );
        }
        catch ( const std::exception& e )
        {
            this->appendFileLog(
                "error",
                "The file header contains an unparsable number of parameters.");
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( numberOfParameters <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header for the " ) +
                eventTableName +
                std::string( " contains an invalid non-positive number of parameters: " ) +
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
        // Read the parameter names.
        // -------------------------
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
        this->fileParameterNames.clear( );
        this->fileParameterNames.reserve( numberOfParameters );
        this->fileParameterLongNames.clear( );
        this->fileParameterLongNames.reserve( numberOfParameters );

        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter name length, in bytes.
            if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
            {
                this->appendFileLog(
                    "error",
                     std::string( "The file is truncated where the " ) +
                     eventTableName +
                     std::string( " parameter names should be." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[HEADER_VALUE_WIDTH] = '\0';

            size_t nBytes = 0;
            try
            {
                nBytes = std::stol( buffer );
            }
            catch ( const std::exception& e )
            {
                this->appendFileLog(
                    "error",
                    "The file header contains an unparsable parameter name size." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            if ( nBytes <= 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file header for the " ) +
                    eventTableName +
                    std::string( " contains an invalid non-positive parameter name size: " ) +
                    std::to_string( nBytes ) + std::string( "." ) );
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
                    std::string( "The file is truncated where the " ) +
                    eventTableName +
                    std::string( " parameter names should be." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            nameBuffer[nBytes] = '\0';

            // Save the parameter name.
            this->fileParameterNames.push_back( std::string( nameBuffer ) );
        }

        //
        // Read the parameter long names.
        // ------------------------------
        // Each parameter is read with:
        // - The number of bytes for the parameter name.
        //   - 20 bytes, space padded.
        //   - In ASCII.
        // - The parameter name.
        //   - However many bytes are required.
        //   - In UTF-8.
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter name length, in bytes.
            if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
            {
                this->appendFileLog(
                    "error",
                     std::string( "The file is truncated where the " ) +
                     eventTableName +
                     std::string( " long parameter names should be." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[HEADER_VALUE_WIDTH] = '\0';

            size_t nBytes = 0;
            try
            {
                nBytes = std::stol( buffer );
            }
            catch ( const std::exception& e )
            {
                this->appendFileLog(
                    "error",
                    "The file header contains an unparsable long parameter name size." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            if ( nBytes < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file header for the " ) +
                    eventTableName +
                    std::string( " contains an invalid non-positive parameter name size: " ) +
                    std::to_string( nBytes ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            else if ( nBytes == 0 )
            {
                // No long parameter name.
                this->fileParameterLongNames.push_back( std::string( ) );
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
                         std::string( "The file is truncated where the " ) +
                         eventTableName +
                         std::string( " long parameter names should be." ) );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        "The file is missing critical information." +
                        ERROR_CANNOTLOAD );
                }
                nameBuffer[nBytes] = '\0';

                // Save the parameter long name.
                this->fileParameterLongNames.push_back( std::string( nameBuffer ) );
            }
        }

        delete[] nameBuffer;

        //
        // Read the parameter minimums.
        // ----------------------------
        // Each parameter minimum is read with:
        // - 20 bytes, space padded.
        // - In ASCII.
        this->fileParameterMinimums.clear( );
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter minimum.
            if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
            {
                this->appendFileLog(
                    "error",
                     std::string( "The file is truncated where the " ) +
                     eventTableName +
                     std::string( " parameter minimums should be." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[HEADER_VALUE_WIDTH] = '\0';

            try
            {
                const double value = std::stod( buffer );
                this->fileParameterMinimums.push_back( value );
            }
            catch ( ... )
            {
                this->fileParameterMinimums.push_back( 0.0 );
            }
        }

        //
        // Read the parameter maximums.
        // ----------------------------
        // Each parameter maximum is read with:
        // - 20 bytes, space padded.
        // - In ASCII.
        this->fileParameterMaximums.clear( );
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Read the parameter maximum.
            if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
            {
                this->appendFileLog(
                    "error",
                     std::string( "The file is truncated where the " ) +
                     eventTableName +
                     std::string( " parameter maximums should be." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
            buffer[HEADER_VALUE_WIDTH] = '\0';

            try
            {
                const double value = std::stod( buffer );
                this->fileParameterMaximums.push_back( value );
            }
            catch ( ... )
            {
                this->fileParameterMaximums.push_back( 0.0 );
            }
        }
    }

    /**
     * Loads the gate state header.
     *
     * The file header is in ASCII and contains:
     * - The gate state's applied flag at 0 (false) or 1 (true)
     *
     * Each value is in a field of 20 bytes, left-adjusted and space padded.
     * This matches the style of other header values.
     *
     * The gate state is updated with the header values.
     *
     * This method reads the header, leaving the file location on the first
     * byte after the header. All header values are read, parsed, and saved
     * into this object.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] eventTable
     *   The event table to save.
     * @param[in] gateName
     *   The name of the gate, used in verbose output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::save()
     * @see ::saveGateHeader()
     */
    void loadGateHeader(
        std::FILE*const fp,
        std::shared_ptr<FlowGate::Gates::GateStateInterface> state,
        const std::string gateName )
    {
        char buffer[HEADER_VALUE_WIDTH + 1];

        //
        // Read the gate applied flag.
        // ---------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        if ( std::fread( buffer, 1, HEADER_VALUE_WIDTH, fp ) != HEADER_VALUE_WIDTH )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header is truncated where the " ) +
                gateName +
                std::string( " applied flag should be." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[HEADER_VALUE_WIDTH] = '\0';

        const int appliedInt = std::stod( buffer );
        if ( appliedInt != 0 && appliedInt != 1 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header for the " ) +
                gateName +
                std::string( " contains an invalid gate applied flag: " ) +
                std::to_string( appliedInt ) + std::string( "." ));
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        state->setApplied( (appliedInt != 0) );
    }

    /**
     * Loads event table values from the file.
     *
     * The data is a block of binary data composed entirely of
     * floats or doubles.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] eventTable
     *   The event table to save.
     * @param[in] eventTableName
     *   The name of the event table, used in verbose output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void loadEventTableValues(
        std::FILE*const fp,
        std::shared_ptr<FlowGate::Events::EventTableInterface> eventTable,
        const std::string eventTableName )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
              "  Loading " << eventTableName << " values:\n";

        //
        // Setup.
        // ------
        // Get the number of parameters and events and whether to swap bytes.
        const FlowGate::Gates::DimensionIndex numberOfParameters =
            eventTable->getNumberOfParameters( );

        const size_t numberOfEvents = eventTable->getNumberOfEvents( );

        // If the current host's byte order does not match that of the
        // data, swap the data.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        // Host is LSBF. Swap if the file is not LSBF.
        const bool swap = (this->fileIsLSBF == false);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        // Host is MSBF. Swap if the file is LSBF.
        const bool swap = (this->fileIsLSBF == true);
#else
        const bool swap = false;
#endif

        if ( this->verbose == true )
        {
            if ( eventTable->areValuesFloats( ) == true )
                std::cerr << this->verbosePrefix << ": " <<
                    "    Loading " << numberOfEvents <<
                    " 32-bit floats for " <<
                    numberOfParameters << " parameters" <<
                    ((swap == true) ? " swapping bytes\n" :
                    "in host byte order\n");
            else
                std::cerr << this->verbosePrefix << ": " <<
                    "    Loading " << numberOfEvents <<
                    " 64-bit doubles for " <<
                    numberOfParameters << " parameters" <<
                    ((swap == true) ? " swapping bytes\n" :
                    "in host byte order\n");
        }


        //
        // Read the events.
        // ----------------
        // Events are organized in columns with one column per parameter.
        // Consecutive values in the file are for consecutive events in
        // the same parameter.
        if ( eventTable->areValuesFloats( ) == true )
        {
            for ( FlowGate::Gates::DimensionIndex i = 0; i < numberOfParameters; ++i )
            {
                float*const values =
                    eventTable->getParameterFloats( i ).data( );

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
            for ( FlowGate::Gates::DimensionIndex i = 0; i < numberOfParameters; ++i )
            {
                double*const values =
                    eventTable->getParameterDoubles( i ).data( );

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
        eventTable->computeParameterDataMinimumMaximum( );
    }

    /**
     * Loads the gate state event-included flags.
     *
     * The data is a block of binary data composed entirely of 8-bit integers
     * used as booleans.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] state
     *   The gate state into which to load the flags.
     * @param[in] gateName
     *   The name of the gate, used in error output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::load()
     * @see ::saveGateValues()
     */
    void loadGateValues(
        std::FILE*const fp,
        std::shared_ptr<FlowGate::Gates::GateStateInterface> state,
        const std::string gateName )
    {
        // All data values are 1-byte integers.
        uint8_t*const values = state->getEventIncludedList( ).data( );

        if ( std::fread( values, sizeof(uint8_t), this->fileNumberOfEvents, fp ) != this->fileNumberOfEvents )
        {
            this->appendFileLog(
                "error",
                std::string( "The file is truncated before the end of the " ) +
                gateName +
                std::string( " event-included flags." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
    }

    /**
     * Loads the gate trees from the file.
     *
     * The file location is advanced to the start of the gate trees.
     *
     * The data is a block of text loaded and parsed based upon the header's
     * gate tree format.
     *
     * On completion, the file location is left just after the end of the
     * gate state.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::load()
     * @see ::saveGateTrees()
     */
    void loadGateTrees( std::FILE*const fp )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
              "  Loading file gate trees:\n";

        this->gateTrees.reset( );

        // Seek to the start of the gate trees.
        if ( std::fseek( fp, this->fileOffsetToGateTrees, SEEK_SET ) < 0 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated before the start of the gate trees." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        //
        // Read the text.
        // --------------
        // Read a block of text that is in either JSON or Gating-ML format.
        const size_t nBytes = this->fileLengthOfGateTrees;
        char* buffer = new char[nBytes + 1];

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "    Reading " << nBytes << " bytes of gate trees XML in Gating-ML format\n";

        if ( std::fread( buffer, 1, nBytes, fp ) < nBytes )
        {
            delete[] buffer;
            this->appendFileLog(
                "error",
                "The file is truncated before the end of the gate trees." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        buffer[nBytes] = '\0';

        //
        // Parse the text.
        // ----------------
        if ( this->fileGateTreesFormat == GATE_TREES_GATINGML_FORMAT )
        {
            FlowGate::File::FileGatingML* gatingFile = nullptr;
            try
            {
                gatingFile = new FlowGate::File::FileGatingML( );
                gatingFile->setVerbose( this->verbose );
                gatingFile->setVerbosePrefix( this->verbosePrefix );

                // Parse the Gating-ML. On an error, an exception is thrown and
                // a message is added to the Gating-ML file's log.
                //
                // The character buffer becomes owned by the FileGatingML
                // object, which will automatically delete it.
                gatingFile->loadText( buffer );

                // Get the parsed gate trees.
                this->gateTrees = gatingFile->getGateTrees( );

                // Copy any logged messages to our log.
                for ( const auto& [category, message] : gatingFile->getFileLog( ) )
                    this->appendFileLog( category, message );

                delete gatingFile;
                gatingFile = nullptr;
                buffer = nullptr;
            }
            catch ( ... )
            {
                // Copy any logged messages to our log.
                for ( const auto& [category, message] : gatingFile->getFileLog( ) )
                    this->appendFileLog( category, message );

                delete gatingFile;
                gatingFile = nullptr;
                throw;
            }
        }
        else
        {
            FlowGate::File::FileFGJsonGates* gatingFile = nullptr;
            try
            {
                gatingFile = new FlowGate::File::FileFGJsonGates( );
                gatingFile->setVerbose( this->verbose );
                gatingFile->setVerbosePrefix( this->verbosePrefix );

                // Parse the JSON. On an error, an exception is thrown and
                // a message is added to the JSON file's log.
                //
                // The character buffer becomes owned by the FileFGJsonGates
                // object, which will automatically delete it.
                gatingFile->loadText( buffer );

                // Get the parsed gate trees.
                this->gateTrees = gatingFile->getGateTrees( );

                // Copy any logged messages to our log.
                for ( const auto& [category, message] : gatingFile->getFileLog( ) )
                    this->appendFileLog( category, message );

                delete gatingFile;
                gatingFile = nullptr;
                buffer = nullptr;
            }
            catch ( ... )
            {
                // Copy any logged messages to our log.
                for ( const auto& [category, message] : gatingFile->getFileLog( ) )
                    this->appendFileLog( category, message );

                delete gatingFile;
                gatingFile = nullptr;
                throw;
            }
        }
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
     * Saves the gating cache to a new file.
     *
     * The file is written with data from the current gating cache.
     *
     * @param[in] filePath
     *   The path to the file to save.
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
    void save( const std::string& filePath )
    {
        //
        // Initialize.
        // -----------
        // Save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Saving FlowGate Gating Cache file \"" << filePath << "\".\n";

        this->clearFileLog();
        this->fileAttributes.clear( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );


        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( filePath.empty() == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty file path." );
        if ( this->gatingCache == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "A gating cache to save to the file has not been provided." ) );


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
        // Then save the header with file information, such as the format
        // version number and file offsets to important data. Then save
        // the gate tree and the event tables.
        try
        {
#ifdef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            this->fileNumberOfEvents =
                this->gatingCache->getSourceEventTable( )->getNumberOfEvents( );

            // Save the header. This stores the file format version in use
            // and dummy file offsets. When we're done, we'll come back
            // and set the file offsets.
            this->saveHeader( fp );

            // Write the source event table.
            this->saveSourceEventTable( fp );

            // Save the gate trees.
            this->saveGateTrees( fp );

            // Save each gate's state.
            this->saveGateStates( fp );

            // Go back to the header and fill in the file offsets.
            this->saveHeaderOffsets( fp );

#ifdef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }

private:
    /**
     * Saves the file header containing the version number and file offsets.
     *
     * The file header is in ASCII and contains:
     * - The file format's version number.
     * - The byte order.
     * - The data type.
     * - The number of events.
     * - The gate trees format.
     * - The file byte offset to the source event table.
     * - The file byte length of the source event table.
     * - The file byte offset to the gate trees.
     * - The file byte length of the gate trees.
     * - The file byte offset to the gate state.
     * - The file byte length of the gate state.
     *
     * Each value is in a field of 20 bytes, left-adjusted and space padded.
     * 20 bytes is sufficient to record a 64-bit integer in base-10 ASCII
     * digits. The exception is the versin number, which is written as a
     * 40 byte string.
     *
     * This method writes the header, leaving the file pointer on the first
     * byte after the header.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::saveHeaderOffsets()
     */
    void saveHeader( std::FILE*const fp )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving file header:\n";

        int status = 0;

        //
        // Write the version number.
        // ------------------------
        // - 40 bytes, space padded.
        // - In ASCII.
        // - Has the format "FlowGateGatingCache_" followed by
        //   major.minor.subminor.
        std::string version = "FlowGateGatingCache_1.0.0";
        this->fileAttributes.emplace( "versionNumber", version );
        this->fileAttributes.emplace( "numberFormat", "binary" );

        if ( std::fprintf( fp, "%-*s", HEADER_VERSION_WIDTH, version.c_str( ) ) < 0 )
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
        this->fileIsLSBF = true;
        status = std::fprintf( fp, "%-*s", HEADER_VALUE_WIDTH, "lsbf" );
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        this->fileIsLSBF = false;
        status = std::fprintf( fp, "%-*s", HEADER_VALUE_WIDTH, "msbf" );
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
                (this->fileIsLSBF == true ?
                    "Least-significant byte first.\n" :
                    "Most-significant byte first.\n");
        }


        //
        // Write the data type.
        // --------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        // - One of "float" or "double".
        if ( this->gatingCache->getSourceEventTable( )->areValuesFloats( ) == true )
        {
            this->fileAttributes.emplace( "dataType", "float" );
            status = std::fprintf( fp, "%-*s", HEADER_VALUE_WIDTH, "float" );
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "floats\n";
        }
        else
        {
            this->fileAttributes.emplace( "dataType", "double" );
            status = std::fprintf( fp, "%-*s", HEADER_VALUE_WIDTH, "double" );
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
        const size_t numberOfEvents =
            this->gatingCache->getSourceEventTable( )->getNumberOfEvents( );
        this->fileAttributes.emplace( "numberOfEvents",
            std::to_string( numberOfEvents ) );

        if ( std::fprintf( fp, "%-*ld", HEADER_VALUE_WIDTH, numberOfEvents ) < 0 )
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
        // Write the gate trees format.
        // ---------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        if ( this->fileGateTreesFormat.empty( ) == true )
            this->fileGateTreesFormat = DEFAULT_GATE_TREES_FORMAT;
        this->fileAttributes.emplace( "gateTreesFormat",
            this->fileGateTreesFormat );

        if ( std::fprintf( fp, "%-*s", HEADER_VALUE_WIDTH, this->fileGateTreesFormat.c_str( ) ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header gate tree format: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Gate tree format:" <<
                this->fileGateTreesFormat << "\n";


        //
        // Write the byte offsets and lengths to file data.
        // ------------------------------------------------
        // - 20 bytes, space padded, offset to the source event table.
        // - 20 bytes, space padded, length of the source event table.
        // - 20 bytes, space padded, offset to the gate trees.
        // - 20 bytes, space padded, length of the gate trees.
        // - 20 bytes, space padded, offset to the gate state.
        // - 20 bytes, space padded, length of the gate state.
        // - In ASCII.
        //
        // None of these are known yet, so just write zeroes. They will
        // be filled in at the end.
        //
        // Save the location so that it can be saved later back into
        // the header.
        this->fileOffsetToHeaderOffsets = std::ftell( fp );

        // Initialize the offsets to zeroes.
        this->fileOffsetToSourceEventTable = 0L;
        this->fileLengthOfSourceEventTable = 0L;
        this->fileOffsetToGateTrees = 0L;
        this->fileLengthOfGateTrees = 0L;
        this->fileOffsetToGateState = 0L;
        this->fileLengthOfGateState = 0L;

        if ( std::fprintf( fp, "%-*ld%-*ld%-*ld%-*ld%-*ld%-*ld",
            HEADER_VALUE_WIDTH, 0L,
            HEADER_VALUE_WIDTH, 0L,
            HEADER_VALUE_WIDTH, 0L,
            HEADER_VALUE_WIDTH, 0L,
            HEADER_VALUE_WIDTH, 0L,
            HEADER_VALUE_WIDTH, 0L ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header byte offsets: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves the file header byte offsets.
     *
     * The byte offsets to different sections of the file are collected
     * during file saving. This method writes those offsets back into
     * the header, overwritting the zeroes written there initially.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::saveHeader()
     */
    void saveHeaderOffsets( std::FILE*const fp )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving file header offsets:\n";

        //
        // Move to data location.
        // ----------------------
        // Move the file pointer to the start of the header offsets.
        if ( std::fseek( fp, this->fileOffsetToHeaderOffsets, SEEK_SET ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header byte offsets: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        //
        // Write the byte offsets to file data.
        // ------------------------------------
        // - 20 bytes, space padded, offset to the source event table.
        // - 20 bytes, space padded, length of the source event table.
        // - 20 bytes, space padded, offset to the gate trees.
        // - 20 bytes, space padded, length of the gate trees.
        // - 20 bytes, space padded, offset to the gate state.
        // - 20 bytes, space padded, length of the gate state.
        // - In ASCII.
        if ( std::fprintf( fp, "%-*ld%-*ld%-*ld%-*ld%-*ld%-*ld",
            HEADER_VALUE_WIDTH, this->fileOffsetToSourceEventTable,
            HEADER_VALUE_WIDTH, this->fileLengthOfSourceEventTable,
            HEADER_VALUE_WIDTH, this->fileOffsetToGateTrees,
            HEADER_VALUE_WIDTH, this->fileLengthOfGateTrees,
            HEADER_VALUE_WIDTH, this->fileOffsetToGateState,
            HEADER_VALUE_WIDTH, this->fileLengthOfGateState ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header byte offsets: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves the source event table.
     *
     * The current file location is recorded for later use.
     *
     * The gating cache (which is also a source event table) is saved
     * to the file, including its header (such as parameter names)
     * and its data (one column of data per parameter).
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::loadSourceEventTable()
     */
    void saveSourceEventTable( std::FILE*const fp )
    {
        // Record the current file location.
        this->fileOffsetToSourceEventTable = std::ftell( fp );

        const auto name = std::string( "source event table" );
        const auto sourceEventTable =
            this->gatingCache->getSourceEventTable( );

        this->saveEventTableHeader( fp, sourceEventTable, name );
        this->saveEventTableValues( fp, sourceEventTable, name );

        // Record the length of the data.
        this->fileLengthOfSourceEventTable =
            std::ftell( fp ) - this->fileOffsetToSourceEventTable;
    }

    /**
     * Saves the state of all gates.
     *
     * The current file location is recorded for later use.
     *
     * The cache's gate trees are used to get a list of gates and their
     * state. Each gate state (which is also a transformed event table)
     * is saved to the file, including its header (such as parameter names)
     * and its data (one column of data per parameter plus inclusion flags).
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::loadGateStates()
     */
    void saveGateStates( std::FILE*const fp )
    {
        // Record the current file location.
        this->fileOffsetToGateState = std::ftell( fp );

        const auto gates = this->gatingCache->getGateTrees()->findDescendentGates( );
        for ( const auto& gate : gates )
        {
            const auto name = std::string( "gate " ) + gate->getName( );

            // A gate state object is an event table.
            const auto state = gate->getState( );
            const auto table = std::dynamic_pointer_cast<FlowGate::Events::EventTableInterface>( state );

            this->saveEventTableHeader( fp, table, name );
            this->saveGateHeader( fp, state, name );
            this->saveEventTableValues( fp, table, name );
            this->saveGateValues( fp, state, name );
        }

        // Record the length of the data.
        this->fileLengthOfGateState =
            std::ftell( fp ) - this->fileOffsetToGateState;
    }

    /**
     * Saves the event table header containing the table's parameter names.
     *
     * The file header is in ASCII and contains:
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
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] eventTable
     *   The event table to save.
     * @param[in] eventTableName
     *   The name of the event table, used in verbose output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::save()
     * @see ::loadEventTableHeader()
     */
    void saveEventTableHeader(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Events::EventTableInterface> eventTable,
        const std::string eventTableName )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving " << eventTableName << " header:\n";

        //
        // Write the number of parameters.
        // -------------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        const size_t numberOfParameters =
            eventTable->getNumberOfParameters( );

        if ( std::fprintf( fp, "%-*ld", HEADER_VALUE_WIDTH, numberOfParameters ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the " ) +
                eventTableName +
                std::string( " header number of parameters: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of source parameters:" <<
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
        auto parameterNames = eventTable->getParameterNames( );
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            const auto name = parameterNames[i];
            const size_t nBytes = name.size( );
            if ( std::fprintf( fp, "%-*ld", HEADER_VALUE_WIDTH, nBytes ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the " ) +
                    eventTableName +
                    std::string( " header parameter names: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
            if ( std::fwrite( name.c_str( ), 1, nBytes, fp ) != nBytes )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the " ) +
                    eventTableName +
                    std::string( " header parameter names: " ) +
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
        auto parameterLongNames = eventTable->getParameterLongNames( );
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            const auto name = parameterLongNames[i];
            const size_t nBytes = name.size( );
            if ( std::fprintf( fp, "%-*ld", HEADER_VALUE_WIDTH, nBytes ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the " ) +
                    eventTableName +
                    std::string( " header parameter long names: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
            if ( std::fwrite( name.c_str( ), 1, nBytes, fp ) != nBytes )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the " ) +
                    eventTableName +
                    std::string( " header parameter long names: " ) +
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
            const auto value = eventTable->getParameterMinimum( i );
            if ( std::fprintf( fp, "%-*.*le", HEADER_VALUE_WIDTH, HEADER_VALUE_WIDTH-8, value ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the " ) +
                    eventTableName +
                    std::string( " header parameter minimum: " ) +
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
            const auto value = eventTable->getParameterMaximum( i );
            if ( std::fprintf( fp, "%-*.*le", HEADER_VALUE_WIDTH, HEADER_VALUE_WIDTH-8, value ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the " ) +
                    eventTableName +
                    std::string( " header parameter maximum: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
        }
    }

    /**
     * Saves event table containing the columns of events.
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
     *   The file pointer for the file to write.
     * @param[in] eventTable
     *   The event table to save.
     * @param[in] eventTableName
     *   The name of the event table, used in verbose output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::save()
     * @see ::loadEventTableValues()
     */
    void saveEventTableValues(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Events::EventTableInterface> eventTable,
        const std::string eventTableName )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving " << eventTableName << ":\n";

        //
        // Setup.
        // ------
        // Get the number of parameters and events.
        const uint32_t numberOfParameters =
            eventTable->getNumberOfParameters( );
        const size_t numberOfEvents = eventTable->getNumberOfEvents( );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "    Writing " << numberOfEvents << " values for " <<
                numberOfParameters << " parameters\n";


        //
        // Write the values.
        // -----------------
        // All data values have the same 32-bit or 64-bit floating-point
        // data type.
        if ( eventTable->areValuesFloats( ) == true )
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                const float*const values =
                    eventTable->getParameterFloats( i ).data( );

                if ( std::fwrite( values, sizeof(float), numberOfEvents, fp ) != numberOfEvents )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The system reported an error while writing " ) +
                        eventTableName +
                        std::string( " data: " ) +
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
                    eventTable->getParameterDoubles( i ).data( );

                if ( std::fwrite( values, sizeof(double), numberOfEvents, fp ) != numberOfEvents )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The system reported an error while writing " ) +
                        eventTableName +
                        std::string( " data: " ) +
                        std::string( std::strerror( errno ) ) );
                    throw std::runtime_error(
                        ERROR_WRITE +
                        ERROR_CANNOTSAVE );
                }
            }
        }
    }

    /**
     * Saves additional data in the gate state header.
     *
     * The values are in ASCII and contain:
     * - The gate state's applied flag at 0 (false) or 1 (true)
     *
     * The value is in a field of 20 bytes, left-adjusted and space padded.
     * This matches the style of other header values.
     *
     * This method writes the header, leaving the file location on the first
     * byte after the header.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] state
     *   The state to save.
     * @param[in] gateName
     *   The name of the gate, used in error output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::save()
     * @see ::loadGateHeader()
     */
    void saveGateHeader(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::GateStateInterface> state,
        const std::string gateName )
    {
        //
        // Write the gate applied flag.
        // ----------------------------
        // - 20 bytes, space padded.
        // - In ASCII.
        const size_t applied = (state->isApplied( )) ? 1 : 0;
        if ( std::fprintf( fp, "%-*ld", HEADER_VALUE_WIDTH, applied ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the " ) +
                gateName +
                std::string( " applied flag: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves the gate state event-included flags.
     *
     * The data is a block of binary data composed entirely of 8-bit integers
     * used as booleans.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] state
     *   The state to save.
     * @param[in] gateName
     *   The name of the gate, used in error output only.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::save()
     * @see ::loadGateValues()
     */
    void saveGateValues(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::GateStateInterface> state,
        const std::string gateName )
    {
        // All data values are 1-byte integers.
        const uint8_t*const values = state->getEventIncludedList( ).data( );

        if ( std::fwrite( values, sizeof(uint8_t), this->fileNumberOfEvents, fp ) != this->fileNumberOfEvents )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the " ) +
                gateName + 
                std::string( " event-included flags: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves the gate trees to the file.
     *
     * The current file location is recorded for later use.
     *
     * The gating cache's gate trees are converted to JSON or Gating-ML
     * format and saved to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     * @see ::save()
     * @see ::loadGateTrees()
     */
    void saveGateTrees( std::FILE*const fp )
    {
        // Record the current file location.
        this->fileOffsetToGateTrees = std::ftell( fp );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
              "  Saving gate trees:\n";

        const auto gateTrees = this->gatingCache->getGateTrees( );

        if ( this->fileGateTreesFormat.empty( ) == true )
            this->fileGateTreesFormat = DEFAULT_GATE_TREES_FORMAT;

        std::string text;
        if ( this->fileGateTreesFormat == GATE_TREES_GATINGML_FORMAT )
        {
            // Save the gate trees in Gating-ML format.
            //
            // Use the Gating-ML file writer to create text.
            FlowGate::File::FileGatingML* gatingFile = nullptr;
            try
            {
                gatingFile = new FlowGate::File::FileGatingML( );

                gatingFile->setVerbose( this->verbose );
                gatingFile->setVerbosePrefix( this->verbosePrefix );

                gatingFile->setGateTrees( gateTrees );
                gatingFile->saveText( text );

                delete gatingFile;
                gatingFile = nullptr;
            }
            catch ( ... )
            {
                // Copy any logged messages to our log.
                for ( const auto& [category, message] : gatingFile->getFileLog( ) )
                    this->appendFileLog( category, message );

                delete gatingFile;
                gatingFile = nullptr;
                throw;
            }
        }
        else
        {
            // Save the gate trees in JSON format.
            //
            // Use the JSON file writer to create text.
            FlowGate::File::FileFGJsonGates* gatingFile = nullptr;
            try
            {
                gatingFile = new FlowGate::File::FileFGJsonGates( gateTrees );
                text = gatingFile->saveText( );

                delete gatingFile;
                gatingFile = nullptr;
            }
            catch ( ... )
            {
                // Copy any logged messages to our log.
                for ( const auto& [category, message] : gatingFile->getFileLog( ) )
                    this->appendFileLog( category, message );

                delete gatingFile;
                gatingFile = nullptr;
                throw;
            }
        }

        const size_t nBytes = text.size( );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "    Writing " << nBytes << " bytes of gate trees format\n";

        if ( std::fwrite( text.c_str( ), 1, nBytes, fp ) < nBytes )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file gate trees: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        // Record the length of the data.
        this->fileLengthOfGateTrees = nBytes;
    }
    // @}
};

} // End File namespace
} // End FlowGate namespace

#undef FLOWGATE_FILEFGGATINGCACHE_USE_FLOCKFILE
#undef FLOWGATE_FILEFGGATINGCACHE_USE_BSWAP
