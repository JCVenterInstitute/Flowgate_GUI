/**
 * @file
 * Loads flow cytometry gate descriptions in the custom (legacy)
 * FlowGate text gates file format, using tab-separated values.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "GateTrees.h"
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
#include <map>          // std::map
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector
#include <map>          // std::map

// FlowGate libraries.
#include "GateTrees.h"  // Gate storage





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
#define FLOWGATE_FILEFGTEXTGATES_USE_FLOCKFILE
#endif

// Use a fast text-to-integer parser.
#define FAST_ATOI





namespace FlowGate {
namespace File {





/**
 * Loads flow cytometry gate descriptions in the custom (legacy)
 * FlowGate text file format, using tab-separated values.
 *
 * The FlowGate project has defined a custom "FlowGate Text Gates" file format
 * for the storage of two-dimensional rectangular gates ordered into a single
 * gate tree. <B>This is a legacy file format provided to interact with older
 * software and it is not recommended for further use.</B>
 *
 * The FlowGate Text Gates file format is a much simpler format than the ISAC
 * Gating-ML XML format for flow cytometery gates. Gating-ML be used as a
 * full-featured description of gates and gating trees. The FlowGate Text
 * Gates format only supports rectangle gates with an assumed logicle "FCS"
 * transform. It's gates use event table parameter column indexes rather than
 * parameter names, which makes text gate files tightly bound to a specific
 * event table file, such as an FCS file or a FlowGate binary or text event
 * file.
 *
 *
 * <B>File format</B><BR>
 * A FlowGate Text Gates file is a tab-separated value (TSV) text file format
 * that contains:
 *
 * @li No header. The columns of the file are not named.
 *
 * @li A text table of gates. Each gate is on its own line, terminated by a
 *   carriage return ("\n"). Tab-separated values on the line describe the
 *   gate.  All values are integers or text.
 *
 * Table columns have the following fixed meanings:
 * @li Column 0 = Gate number (in increasing order, starting at 1).
 * @li Column 1 = Event parameter index for the X axis.
 * @li Column 2 = Event parameter index for the Y axis.
 * @li Column 3 = X axis parameter (see gate type below).
 * @li Column 4 = X axis parameter (see gate type below).
 * @li Column 5 = Y axis parameter (see gate type below).
 * @li Column 6 = Y axis parameter (see gate type below).
 * @li Column 7 = Parent gate number, or 0 if there is no parent.
 * @li Column 8 = Gate type.
 * @li Column 9 = Debug output enable.
 * @li Column 10 = Multi-pass sub-population enable.
 * @li Column 11 = (optional) Gate name.
 *
 * Each line's gate number in column 0 is a unique ID for the gate. The
 * same numbers are used in column 7 to refer to a parent gate. A zero
 * indicates there is no parent and the row's gate is a gate tree root.
 * The file format allows only one gate tree root. Legacy software assumes
 * that gates are ordered in the file in increasing gate number so that
 * the 1st gate with ID 1 is in row 0, the 2nd gate with ID 2 is in row 1,
 * and so forth.
 *
 * Columns 1 and 2 are parameter indexes that select parameters to gate on
 * in an associated FCS event file. Because the gate file does not include
 * the names of those parameters, the gate file is tightly bound to the
 * FCS file for which it was authored.
 *
 * Columns 3, 4, 5, and 6 are gate shape parameters with meanings that vary
 * based on the value of column 8's gate type:
 * @li Gate type = 0: Rectangle based on cluster centroids.
 *   @li Column 3 = X axis start for a gate rectangle (0 to 200).
 *   @li Column 4 = X axis end for a gate rectangle (0 to 200).
 *   @li Column 5 = Y axis start for a gate rectangle (0 to 200).
 *   @li Column 6 = Y axis end for a gate rectangle (0 to 200).
 *
 * @li Gate type = 1: Rectangle based on event values.
 *   @li Column 3 = X axis start for a gate rectangle (0 to 200).
 *   @li Column 4 = X axis end for a gate rectangle (0 to 200).
 *   @li Column 5 = Y axis start for a gate rectangle (0 to 200).
 *   @li Column 6 = Y axis end for a gate rectangle (0 to 200).
 *
 * @li Gate type = 2: Ratio based on event values.
 *   @li Column 3 = X axis low (0 to 200).
 *   @li Column 4 = X axis high (0 to 200).
 *   @li Column 5 = Y axis low (0 to 200).
 *   @li Column 6 = Y axis high (0 to 200).
 *
 * Column 9 is a boolean that enables (value of 1) or disables (value of 0)
 * additional debug output in legacy software.
 *
 * Column 10 is a boolean that enables (value of 1) or disables (value of 0)
 * multi-pass sub-clustering in the DAFi clustering algorithm. When disabled,
 * clustering is computed over the entire event list and those clusters used
 * for gating. When enabled, clustering is computed hierarchically so that
 * only those events that pass a parent's gates are used in clustering for
 * the child gates.
 *
 *
 * <B>Conversion to gate trees</B><BR>
 * When loaded, a legacy FlowGate Text Gate file creates a GateTrees object
 * with a single gate tree root. Gates are arranged as descendents of the
 * root. The root and all descendents are two-dimensional RectangleGate
 * objects, and all gate dimensions have a logicle "FCS" transform attached
 * to match legacy software's use of the transform on all event parameters.
 *
 * If the file's gate type is 0 for rectangle gates on cluster centroids,
 * the gate tree's corresponding rectangle gate is set to use the DAFi
 * gating method. If the gate type is 2 instead, for rectangle gates on
 * event values, the corresponding rectangle gate is set to use event gating.
 *
 * Ratio gates (gate type 2) are not supported.
 *
 * The debug output enable flag in column 9, and the multi-pass sub-clustering
 * flag in column 10 are ignored.
 *
 * If column 11 is not empty, the gate tree's corresponding rectangle gate
 * name is set.
 *
 *
 * <B>Loading files</B><BR>
 * A file may be loaded by providing a file path to the constructor.
 * @code
 * auto file = new FileFGTextGates( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFGTextGates( );
 * file->load( filepath );
 * #endcode
 *
 * The same data object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the data object
 * is deleted automatically:
 * @code
 * auto file = new FileFGTextGates( );
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
 * FlowGate Text Gates files can be loaded by this class, *but not saved*.
 * It is not practical to support saving gate trees because the file format
 * is so limited in its ability to describe gates. Gating-ML should be used
 * instead.
 *
 *
 * <B>Getting gate trees</B><BR>
 * File data is composed of one or more gate trees. Each gate tree has a
 * root gate that has zero or more children gates, which themselves may
 * have children, and so on.
 *
 * Gate trees are stored in a GateTrees object that manages a simple list of
 * gate tree roots. The order of gates in the list has no significance and
 * typically matches the order in which gate tree roots are found in a
 * loaded file.
 *
 * The GateTrees object may be queried and then used to get gate trees:
 * @code
 * // Get the gate trees.
 * auto trees = file->getGateTrees( );
 *
 * // Get the number of gate trees.
 * auto nTrees = trees->getNumberOfGateTrees( );
 *
 * // Loop over all gate trees.
 * for (size_t i = 0; i < nTrees; ++i)
 * {
 *   auto root = trees->getGateTree(i);
 *   ...
 * }
 * @endcode
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
 * @li Integer parsing uses a fast custom parser that doesn't have the
 *     overhead of a standard parser because it doesn't handle negatives,
 *     decimals, or exponents.
 * @endinternal
 */
class FileFGTextGates final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate (legacy) Text Gates files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "FlowGate Text Gates";

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
        "FileFGTextGates";

    /**
     * The anticipated number of rows in a data file.
     *
     * This value is used to preallocate the gate vector used to stage values
     * during file reading. Preallocating the vector reduces the overhead
     * of repeatedly growing those vectors.
     *
     * If there are more gate rows than this, the vector of results will be
     * automatically increased and parsing will continue. This is not a limit,
     * just a hint.
     */
    static const size_t RESERVE_NUMBER_OF_GATES = 30;

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
     * The error message 1st line for an unsupported feature error.
     */
    inline static const std::string ERROR_UNSUPPORTED_FEATURE =
        "Unsupported file format feature.\n";

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



//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    // File log --------------------------------------------------------
    /**
     * A log of file load error messages.
     *
     * Log entries are pairs where the first value is an error category,
     * and the second is an error message. Well-known categories are:
     *
     * @li "critical" for errors that cause file load to abort.
     *
     * @li "warning" for recoverable errors that do not cause file load
     *     to abort.
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
     * file load, and other operations.
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
     * The current file's attributes.
     *
     * During and after loading or saving a file, this contains name-value
     * pairs that describe the loaded or saved file. Prior to loading or
     * saving a file, this list is empty.
     *
     * Well-known keys are:
     * @li "path" (string) The path to the most recently loaded or saved file.
     * @li "versionNumber" (string) The file version number.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getFileAttributes()
     */
    std::map<const std::string, const std::string> fileAttributes;



    // Support information ---------------------------------------------
    /**
     * An optional list of parameter names.
     *
     * The gate text file format does not include parameter names for its
     * gates. Instead, it refers to parameters by their integer list index.
     * This makes it impossible to properly build a gate tree unless a
     * name list is also available.
     *
     * @see getParameterNames()
     * @see setParameterNames()
     */
    std::vector<std::string> fileParameterNames;



    // Gate trees ------------------------------------------------------
    /**
     * The current list of gate trees.
     *
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees;



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
        extensions.push_back( "config" );
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
     * Constructs a new object with no gates.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGTextGates( )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
    }



    // Copy constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with a copy of the gate trees
     * in the given FileFGTextGates object.
     *
     * @param[in] file
     *   The FileFGTextGates object to copy.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGTextGates( const FileFGTextGates*const file )
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
        if ( file->gateTrees != nullptr )
            this->copy( file->gateTrees );
    }

    /**
     * Constructs a new object initialized with a copy of the gate trees
     * in the given FileFGTextGates object.
     *
     * @param[in] file
     *   The FileFGTextGates object to copy.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGTextGates( const FileFGTextGates& file )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        if ( file.gateTrees != nullptr )
            this->copy( file.gateTrees );
    }



    // Load constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with data loaded from a
     * gating file.
     *
     * @param[in] path
     *   The file path to a file to load.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is a NULL pointer or empty.
     * @throws std::runtime_error
     *   Throws an exception if there is a problem loading the file.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGTextGates( const char*const path )
    {
        // Validate.
        if ( path == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL path." );

        if ( std::strlen( path ) == 0 )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty path." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Load.
        this->load( std::string( path ) );
    }

    /**
     * Constructs a new object initialized with data loaded from a
     * gating file.
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
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGTextGates( const std::string& path )
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
    virtual ~FileFGTextGates( )
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
     * @li "error" for errors that cause file load to abort.
     *
     * @li "warning" for recoverable errors that do not cause file load
     *     to abort.
     *
     * @param[in] category
     *   The message category.
     * @param[in] message
     *   The message.
     *
     * @see ::load()
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
     * The log of error messages from the most recent file load
     * is cleared.
     *
     * The log is cleared automatically each time a file load is begun.
     *
     * @see ::load()
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
     * encountered during the most recent file load. Each
     * entry is a pair containing a log message category and the
     * associated message. Well-known categories are:
     *
     * @li "error" for errors that cause file load to abort.
     *
     * @li "warning" for recoverable errors that do not cause file load
     *     to abort.
     *
     * Critical errors typically cause an exception to be thrown. The
     * exception message is usually user-friendly and generic, while
     * the log will include more technical detail.
     *
     * The log is cleared automatically each time a file load is begun.
     *
     * @return
     *   Returns the verbose message prefix.
     *
     * @see ::load()
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
     * provide information about the file itself, rather than the gate trees in
     * the file. Each entry in the returned map uses an attribute key string
     * to name an associated string value. Some string values may be interpreted
     * as integers.
     *
     * Well-known keys include:
     * @li "path" (string)
     * @li "versionNumber" (string)
     *
     * @return
     *   Returns a map of string attribute keys to string attribute values.
     */
    inline const std::map<const std::string, const::std::string> getFileAttributes( )
        const noexcept
    {
        return this->fileAttributes;
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
     * Resets the object, clearing it of all content.
     */
    inline void reset( )
    {
        this->clearFileLog( );
        this->fileAttributes.clear( );
        this->gateTrees.reset( new FlowGate::Gates::GateTrees( ) );
    }



    // Copy gate trees -------------------------------------------------
    /**
     * Copies the given gate trees.
     *
     * The file object is reset.
     *
     * The given gate trees are copied into a new gate tree list stored
     * within the object. The new trees are used by further methods.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Gates::GateTrees*const gateTrees )
    {
        // Validate.
        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        this->reset( );

        // Copy.
        this->copy( *gateTrees );
    }

    /**
     * Copies the given gate trees.
     *
     * The file object is reset.
     *
     * The given gate trees are copied into a new gate tree list stored
     * within the object. The new trees are used by further methods.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     * @see ::reset()
     */
    inline void copy(
        const std::shared_ptr<FlowGate::Gates::GateTrees>& gateTrees )
    {
        // Validate.
        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate trees." );

        this->reset( );

        // Copy.
        this->copy( *(gateTrees.get( )) );
    }

    /**
     * Copies the given gate trees.
     *
     * The file object is reset.
     *
     * The given gate trees are copied into a new gate tree list stored
     * within the object. The new trees are used by further methods.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see ::load()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Gates::GateTrees& gateTrees )
        noexcept
    {
        this->reset( );

        this->gateTrees.reset(
            new FlowGate::Gates::GateTrees( gateTrees ) );
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
     * Returns the number of parameters per event.
     *
     * If a parameter name list has not been provided, then a zero is
     * returned.
     *
     * @return
     *   Returns the number of parameters per event.
     *
     * @see ::getParameterNames()
     * @see ::setParameterNames()
     */
    inline size_t getNumberOfParameters( )
        const noexcept
    {
        return this->fileParameterNames.size( );
    }

    /**
     * Returns a list of parameter names.
     *
     * Text gate files do not include parameter names. An optional parameter
     * name list may be set and used to help map a file's parameter indexes
     * to names. If such a list has been set,this method returns it. If no
     * name list has been set, then an empty vector is returned.
     *
     * @return
     *   Returns a reference to the parameter names vector.
     *
     * @see ::getNumberOfParameters()
     * @see ::setParameterNames()
     */
    inline const std::vector<std::string>& getParameterNames( )
        const noexcept
    {
        return this->fileParameterNames;
    }

    /**
     * Sets a list of parameter names.
     *
     * Text gate files do not include parameter names. An optional parameter
     * name list may be set and used to help map a file's parameter indexes
     * to names.
     *
     * @param[in] parameterNames
     *   A vector of parameter names.
     *
     * @see ::getNumberOfParameters()
     * @see ::getParameterNames()
     */
    inline void getParameterNames(
        const std::vector<std::string>& parameterNames )
        noexcept
    {
        this->fileParameterNames = parameterNames;
    }
    // @}



//----------------------------------------------------------------------
// Gates and gate trees.
//----------------------------------------------------------------------
public:
    /**
     * @name Gate trees
     */
    // @{
    /**
     * Gets the gate tree list in use.
     *
     * If a file has not been loaded and a gate tree has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the gate tree list.
     *
     * @see ::setGateTrees()
     */
    inline std::shared_ptr<const FlowGate::Gates::GateTrees> getGateTrees( )
        const
    {
        return this->gateTrees;
    }

    /**
     * Gets the gate tree list in use.
     *
     * If a file has not been loaded and a gate tree has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the gate tree list.
     *
     * @see ::setGateTrees()
     */
    inline std::shared_ptr<FlowGate::Gates::GateTrees> getGateTrees( )
    {
        return this->gateTrees;
    }

    /**
     * Resets the object and sets the gate trees to use.
     *
     * The prior gate trees, if any, are released. The new gate trees
     * will be used for all further operations.
     *
     * @param[in] gateTrees
     *   The gate trees to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate trees are a nullptr.
     *
     * @see ::reset()
     * @see ::copy()
     * @see ::getGateTrees()
     */
    inline void setGateTrees(
        std::shared_ptr<FlowGate::Gates::GateTrees>& gateTrees )
    {
        // Validate.
        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate trees." );

        this->reset( );
        this->gateTrees = gateTrees;
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
    void load( const std::string& filePath )
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
                "Loading FlowGate Text Gates file \"" << filePath << "\".\n";

        this->reset( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );

        this->appendFileLog(
            "warning",
            "The FlowGate text gates file is a legacy format. It does not support modern gating features, such as those available in the ISAC Gating-ML format. It does not support transforms, compensation matricies, non-rectangular gates, boolean gate tree expressions, Unicode gate names, and other descriptive gate information. Continued use of this legacy file format is not recommended. Use the ISAC Gating-ML file format instead." );

        this->appendFileLog(
            "warning",
            "The FlowGate text gates file format refers to event parameters by their column number, instead of their parameter name. This makes a gate file strongly dependent upon a specific associated event file, and yet the name of this file is not specified. The user is expected to keep track of this separately. Since this event file is not known, the mapping from parameter numbers to names cannot be done and the imported gates are incomplete and cannot be used for gating. To correct this problem, convert the legacy file to a ISAC Gating-ML file, then use a text editor to fill in the missing parameter names." );


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
#ifdef FLOWGATE_FILEFGTEXTGATES_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Read the gates.
            this->loadGates( fp );

#ifdef FLOWGATE_FILEFGTEXTGATES_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGTEXTGATES_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Load gates from file.
//----------------------------------------------------------------------
private:
    /**
     * @name Load gates from file
     */
    // @{
    /**
     * Loads gates from the file.
     *
     * The gates are a sequence of lines, each containing tab-separated values.
     * All values are treated as integers.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when loading
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::load()
     * @see ::getFileLog()
     *
     * @internal
     * This implementation loops reading a line of text, then parsing it
     * simply into integers. The approach is intentionally simple:
     *
     * @li Reading a line of text upfront reduces the I/O to a single call.
     *   This minimizes function calls, system calls, and stdio's internal
     *   process lock cost.
     *
     * @li Parsing the line in-place within a character array avoids the cost
     *   of tokenizers or allocation of std::string objects or other
     *   temporary copies. Values in the file are known to be delimited by
     *   tabs and lines always end with a carriage return, so there is no
     *   need for regular expressions or other complex parsing.
     *
     * @li Parsing an integer out of a string could be done with sscanf() or
     *   atoi()/strtol(), but the latter is 3x faster because it is simpler.
     *   Simpler still is a custom integer parser that only looks for digits,
     *   and not signs or exponential notation, which are not used by the
     *   file's values.
     * @endinternal
     */
    void loadGates( std::FILE*const fp )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
              "  Loading file gates:\n";

        //
        // Initialize.
        // -----------
        // Create a temporary array of integers to hold the file's values,
        // and another array to hold optional gate names.
        const uint32_t nColumns = 11;
        std::vector<uint32_t> data;
        std::vector<std::string> gateNames;

        // Preallocate space in the data vector in anticipation of filling
        // it in the loops below. This avoids the cost of repeated allocations
        // during filling.
        data.reserve( RESERVE_NUMBER_OF_GATES * nColumns );
        gateNames.reserve( RESERVE_NUMBER_OF_GATES );


        //
        // Read file.
        // ----------
        // Read the file's lines, parsing values into the data vector.
        char* buffer = nullptr;
        size_t bufferLength = 0;
        size_t onColumn = 0;

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
            // For each column, read it as an integer and add it to the
            // data vector.
            const char* b = buffer;
            char c;
            for ( onColumn = 0; onColumn < nColumns; ++onColumn )
            {
                // Skip tabs to the next column value.
                while ( (c = *b) == '\t' && c != '\n' )
                    ++b;
                if (c == '\n')
                    break;

                // Advance to the end of the value.
                const char* bend = b + 1;
                while ( (c = *bend) != '\t' && c != '\n' )
                    ++bend;

                // Parse the integer and add it to the list.
#ifdef FAST_ATOI
                const int value = this->fastAToI( b );
#else
                const int value = std::stoi( b );
#endif

                data.push_back( value );

                if ( c == '\n' )
                    break;

                b = bend + 1;
            }

            // The above loop has ended either because it hit EOL, or
            // because it found all the integer columns needed.
            if ( onColumn < (nColumns-1) )
            {
                this->appendFileLog(
                    "error",
                    "The file does not have the correct number of columns of integer values. It may not be a FlowGate Text Gate file." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }

            // If there is more on the line, use it as the gate name.
            if ( c != '\n' )
            {
                // Skip tabs to the next column value.
                while ( (c = *b) == '\t' && c != '\n' )
                    ++b;
                if (c == '\n')
                    break;

                // Advance to the end of the value.
                const char* bend = b + 1;
                while ( (c = *bend) != '\t' && c != '\n' )
                    ++bend;

                if ( (bend - b) > 0 )
                    gateNames.emplace_back( b, (bend-b) );
                else
                    gateNames.emplace_back( "" );
            }
            else
                gateNames.emplace_back( "" );
        }

        if ( buffer != nullptr )
            delete[] buffer;

        if ( data.size( ) == 0 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated without including any gates." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }


        //
        // Create gates.
        // -------------
        // Loop through the data and interpret it as a list of rows that
        // each describe a gate.
        const size_t nGates = data.size( ) / nColumns;

        // Create a list of gates. The vector's index is the gate number.
        std::vector<std::shared_ptr<FlowGate::Gates::Gate>> idsToGates;
        idsToGates.resize( nGates );

        // Create a list of gates in need of parents. Each entry has:
        // - The ID of the *parent* needed by the gate.
        // - The gate.
        std::vector<std::pair<uint32_t,std::shared_ptr<FlowGate::Gates::Gate>>> inNeedOfParent;

        // Create a list of root gates. Each entry is a gate that has
        // no parent.
        std::vector<std::shared_ptr<FlowGate::Gates::Gate>> roots;

        bool wasDebugEnabled = false;
        bool wasMultipassEnabled = false;


        for ( size_t i = 0; i < nGates; ++i )
        {
            // Get the values.
            const size_t offset = i * nColumns;
            const uint32_t gateNumber       = data[offset + 0];
            const uint32_t xParameterIndex  = data[offset + 1];
            const uint32_t yParameterIndex  = data[offset + 2];
            const uint32_t xMin             = data[offset + 3];
            const uint32_t xMax             = data[offset + 4];
            const uint32_t yMin             = data[offset + 5];
            const uint32_t yMax             = data[offset + 6];
            const uint32_t parentGateNumber = data[offset + 7];
            const uint32_t gateType         = data[offset + 8];
            const uint32_t debugEnable      = data[offset + 9];
            const uint32_t multipassEnable  = data[offset + 10];

            wasDebugEnabled     |= (debugEnable != 0);
            wasMultipassEnabled |= (multipassEnable != 0);

            // Validate.
            if ( gateNumber <= 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number " ) +
                    std::to_string( gateNumber ) +
                    std::string( " gate has an invalid gate number. Gate numbers must be > 0." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete and invalid gate tree description." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( parentGateNumber < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number " ) +
                    std::to_string( gateNumber ) +
                    std::string( " gate has an invalid parent gate number. Parent gate numbers must be >= 0." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete and invalid gate tree description." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( xParameterIndex <= 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number " ) +
                    std::to_string( gateNumber ) +
                    std::string( " gate has an invalid X parameter index. Parameter indexes must be > 0." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid gate parameter description." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( yParameterIndex <= 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number " ) +
                    std::to_string( gateNumber ) +
                    std::string( " gate has an invalid Y parameter index. Parameter indexes must be > 0." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid gate parameter description." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( gateType < 0 || gateType > 2 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number " ) +
                    std::to_string( gateNumber ) +
                    std::string( " gate has an invalid gate type. Gate types must be 0 (cluster-based rectangle), 1 (event-based rectangle), or 2 (event-based fan)." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid gate type description." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( gateType == 2 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number " ) +
                    std::to_string( gateNumber ) +
                    std::string( " gate uses a fan-shaped gate type that is not supported." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid gate type description." ) +
                    ERROR_CANNOTLOAD );
            }

            // Create a gate.
            // Rectangles here are always 2D.
            std::shared_ptr<FlowGate::Gates::Gate> gate;

            FlowGate::Gates::RectangleGate* rectangle =
                new FlowGate::Gates::RectangleGate( 2 );
            gate.reset( rectangle );

            // Set the gate name. The name may be empty.
            gate->setName( gateNames[i] );

            // Set the gating method.
            //
            // - Gate type 0 (cluster-based rectangle) uses the DAFI
            //   clustering method.
            //
            // - Gate types 1 (event-based fan) and 2 (event-based rectangle)
            //   use the event (manual) method.
            if ( gateType == 0 )
                gate->setGatingMethod(
                    FlowGate::Gates::DAFI_CLUSTER_CENTROID_GATING );
            else
                gate->setGatingMethod(
                    FlowGate::Gates::EVENT_VALUE_GATING );

            // Set the rectangle min/max.
            //
            // File values are always in the range 0..200. The reason
            // for this range is unclear since it has nothing to do with
            // real event values (which are often 0..262144 (which is
            // 2^18).
            //
            // Legacy software converts these to a 0..4095 range. This
            // range is used because the FCS Trans transform used in
            // legacy software always produces values in the 0..4095
            // range. Why this range was chosen is unclear since it again
            // has nothing to do with the real event values.
            //
            // We need to convert these to something more "reasonable".
            // Further on we add a Logicle transform that uses default
            // arguments that map values to a 0..1.0 range. This curve
            // matches the curve in the old FCS Trans transform, except
            // that the top value is 1.0 instead of 4095. So.... since we
            // are using Logicle and a 0..1.0 range, map the rectangle
            // min/max to that same range.
            //
            // This is not a perfect choice. We simply don't know the
            // actual event parameter range or that FCS Trans was meant
            // to have been applied to the data first. So we can't be
            // sure what to convert to. This is just a best guess.
            rectangle->setDimensionMinimumMaximum( 0,
                (double)xMin / 200.0,
                (double)xMax / 200.0 );
            rectangle->setDimensionMinimumMaximum( 1,
                (double)yMin / 200.0,
                (double)yMax / 200.0 );

            // Unfortunately, the text gate format refers to event table
            // parameters by index, not by name. To map indexes to names,
            // we need a parameter name list that may have been provided.
            if ( this->fileParameterNames.empty( ) == false )
            {
                // Gate file indexes start at 1. C/C++ indexes start at 0.
                if ( (xParameterIndex-1) >= this->fileParameterNames.size( ) )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The number " ) +
                        std::to_string( gateNumber ) +
                        std::string( " gate uses an X parameter index of " ) +
                        std::to_string( xParameterIndex ) +
                        std::string( " that is out of range for the list of parameter names used by the file loader. Either the parameter name list is inappropriate for the gate file, or the gate file is malformed." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file contains an incomplete and invalid gate description." ) +
                        ERROR_CANNOTLOAD );
                }
                if ( (yParameterIndex-1) >= this->fileParameterNames.size( ) )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The number " ) +
                        std::to_string( gateNumber ) +
                        std::string( " gate uses a Y parameter index of " ) +
                        std::to_string( yParameterIndex ) +
                        std::string( " that is out of range for the list of parameter names used by the file loader. Either the parameter name list is inappropriate for the gate file, or the gate file is malformed." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file contains an incomplete and invalid gate description." ) +
                        ERROR_CANNOTLOAD );
                }

                rectangle->setDimensionParameterName( 0,
                    this->fileParameterNames[xParameterIndex - 1] );
                rectangle->setDimensionParameterName( 1,
                    this->fileParameterNames[yParameterIndex - 1] );
            }
            else
            {
                // A parameter name list is not available. Set the parameter
                // name to the parameter name index (0-based indexing).
                rectangle->setDimensionParameterName( 0,
                    std::to_string( xParameterIndex - 1 ) );
                rectangle->setDimensionParameterName( 1,
                    std::to_string( yParameterIndex - 1 ) );
            }

            // The text gate format has no notion of compensations.

            // FlowGate legacy gates always assume that event data has been
            // transformed by the "FCSTrans" method. The latter is really just
            // a Logicle transform with default arguments.
            //
            // The problem with using a Logicle transform with defaults is
            // that the numeric range of the incoming parameter values is
            // presumed to be 2^18 (262144), which is the Logicle transform's
            // default. If the parameter range is different, then we need to
            // change the transform's "T" argument. But we don't have the
            // event data and parameter range at hand, so we can't.
            std::shared_ptr<FlowGate::Gates::Transform> transform;
            transform.reset( new FlowGate::Gates::LogicleTransform( ) );
            rectangle->setDimensionParameterTransform( 0, transform );
            rectangle->setDimensionParameterTransform( 1, transform );

            // Add the new gate to the ID-to-gate mapping list. If the
            // gate has no parent, add it to the roots list. Otherwise add
            // it to the in-need-of-parent connection list.
            idsToGates[gateNumber - 1] = gate;
            if ( parentGateNumber == 0 )
                roots.push_back( gate );
            else
                inNeedOfParent.push_back(
                    std::make_pair( parentGateNumber-1, gate ) );
        }


        //
        // Connect gates into gate trees.
        // ------------------------------
        // Loop over the gates in need of parents, find their parents,
        // and add the gate to the parent's list of children.
        for ( auto const& [parentId, gate] : inNeedOfParent )
        {
            try
            {
                auto parentGate = idsToGates.at( parentId );
                parentGate->appendChild( gate );
            }
            catch ( ... )
            {
                this->appendFileLog(
                    "error",
                    std::string( "A gate refers to a parent gate ID " ) +
                    std::to_string( parentId ) +
                    std::string( " that does not exist. Without a valid ID, the gate description is incomplete and invalid." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete and invalid gate description." ) +
                    ERROR_CANNOTLOAD );
            }
        }

        inNeedOfParent.clear( );


        //
        // Create gate trees.
        // ------------------
        // Add collected root gates to a gate tree list.
        const size_t nRoots = roots.size( );
        if ( nRoots == 0 )
        {
            this->appendFileLog(
                "warning",
                "The gate file appears to be empty. It does not contain any recognized gates." );
        }
        else
        {
            for ( size_t i = 0; i < nRoots; ++i )
                gateTrees->appendGateTree( roots[i] );

            roots.clear( );
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of gates:" <<
                idsToGates.size( ) << "\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of gate trees:" <<
                this->gateTrees->getNumberOfGateTrees( ) << "\n";
        }

        if ( wasDebugEnabled == true )
        {
            this->appendFileLog(
                "warning",
                "The legacy FlowGate Text Gates file's 9th column enables the output of debugging information in legacy gating tools. This feature is no longer supported and has been ignored. Debug output should be enabled using the appropriate gating tool's features." );
        }

        if ( wasMultipassEnabled == true )
        {
            this->appendFileLog(
                "warning",
                "The legacy FlowGate Text Gates file's 10th column enables sub-populating gating in legacy gating tools. This feature is no longer supported and has been ignored. However, sub-population gating is now usually the default anyway for all cluster-based gating." );
        }
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
     *
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

#ifdef FAST_ATOI
    /**
     * Converts a string representation of an integer to an integer.
     *
     * The integer must be a simple string of digits. Negatives, floating
     * point, and exponents are not supported.
     *
     * <B>Warning:</B> for performance reasons, no bounds checking
     * is done. The given string is presumed to be non-NULL and to contain
     * digits terminated with a non-digit character, such as a NULL.
     *
     * @param[in] s
     *   The string.
     * @return
     *   Returns the integer.
     *
     * @internal
     * This function is similar to atoi() or strtol(), except that it
     * does not support anything beyond simple numeric digits. It does not
     * support positives or negatives, floating point, exponents, or Unicode.
     * It stops parsing on the first non-digit character. It presumes the
     * standard consecutive order of digit characters in ASCII, Unicode, and
     * even EBCIDIC.
     * @endinternal
     */
    inline static int fastAToI( const char* s )
        noexcept
    {
        int i = 0;
        while ( *s >= '0' && *s <= '9' )
            i = (i*10) - '0' + *s++;
        return i;
    }
#endif
    // @}
};

} // End File namespace
} // End FlowGate namespace

#undef FLOWGATE_FILEFGTEXTGATES_USE_FLOCKFILE
