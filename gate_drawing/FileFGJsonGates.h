/**
 * @file
 * Loads flow cytometry gate information in the custom FlowGate JSON text
 * file format.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "GateTrees.h"
 * @li FlowGate "EventTable.h"
 * @li Gason (JASON parser)
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
#include <cerrno>       // Errno
#include <cstdio>       // std::FILE, std::fopen, std::fclose, ...
#include <cstring>      // std::strerror, ...

// Standard C++ libraries.
#include <iostream>     // std::cerr, ...
#include <limits>       // std::numeric_limits
#include <map>          // std::map
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector
#include <map>          // std::map

// Gason (JSON parser).
#include "gason.h"

// FlowGate libraries.
#include "EventTable.h" // Event storage
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
#define FLOWGATE_FILEFGJSONGATES_USE_FLOCKFILE
#endif





namespace FlowGate {
namespace File {





/**
 * Loads flow cytometry gate information in the custom FlowGate JSON text
 * file format.
 *
 * The FlowGate project has defined a "FlowGate JSON Gates" file format
 * for the storage of gate information. The format supports most of the
 * features in the International Society for Advancement of Cytometry (ISAC)
 * Gating-ML XML file format, but using the easier-to-parse JSON syntax.
 *
 * This file format has the following well-known revisions:
 * @li 1.0 from 2019.
 *
 * This file format uses the JSON file syntax originally intended for the
 * exchange of objects and arrays between Javascript apps. JSON has since
 * become a common exchange format used on the web between browser-basd
 * Javascript and the web server. JSON is easier to handle than XML and much
 * less verbose.
 *
 * Gating-ML should still be used for reference gate information intended
 * for archiving and use by multiple applications. The FlowGate JSON Gate
 * format, however, may be used as an alternate for intermediate gate
 * information passed among web tools.
 *
 *
 * <B>File format</B><BR>
 * Each JSON gate file contains:
 *
 * @li A list of gate trees containing nested gates in a tree hierarchy.
 *
 * @li Each gate has a gate type, arguments, and an optional transform.
 *
 *
 * <B>Loading JSON gate files</B><BR>
 * A JSON gate file may be loaded by providing a file path to the constructor.
 * @code
 * auto data = new FileFGJsonGates( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFGJsonGates( );
 * file->load( filepath );
 * #endcode
 *
 * The same JSON gate file object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the object is deleted
 * automatically:
 * @code
 * auto file = new FileFGJsonGates( );
 * file->load( filepath1 );
 * ...
 * file->load( filepath2 );
 * ...
 * file->load( filepath3 );
 * ...
 * @endcode
 *
 * If a problem is encountered while loading a JSON gate file, an exception
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
 * <B>Saving JSON gate files</B><BR>
 * The data in a GateTrees object may be written to a new JSON gate file by
 * calling the save() method:
 * @code
 * try {
 *   file->save( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 * If a problem is encountered while saving a JSON gate file, an exception
 * is thrown with a human-readable message indicating the problem, and
 * the file is deleted if it has already been started. Problems with
 * a save include the inability to create or write to the indicated file.
 *
 *
 * <B>Getting and setting gate trees</B><BR>
 * JSON gate data is composed of one or more gate trees. Each
 * gate tree has a root gate that has zero or more children gates, which
 * themselves may have children, and so on.
 *
 * Gate trees are stored in a GateTrees object that manages a simple list of
 * gate tree roots. The order of gates in the list has no significance and
 * typically matches the order in which gate tree roots are found in a
 * loaded JSON gate file.
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
 * for (Gates::GateIndex i = 0; i < nTrees; ++i)
 * {
 *   auto root = trees->getGateTree(i);
 *   ...
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
 * @endinternal
 */
class FileFGJsonGates final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate JSON Gates files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "FlowGate JSON Gates";

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



    // File format version ---------------------------------------------
    /*
     * The file format version.
     */
    inline static const std::string FORMAT_VERSION = "FlowGateJsonGates_1.0";



private:
    // Configuration ---------------------------------------------------
    /**
     * The default verbosity prefix.
     */
    inline static const std::string DEFAULT_VERBOSE_PREFIX =
        "FileFGJsonGates";



    // Error messages --------------------------------------------------
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
        "Malformed gating file.\n";

    /**
     * The error message 1st line for an unsupported file.
     */
    inline static const std::string ERROR_UNSUPPORTED =
        "Unsupported gating file.\n";

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
     * @li "error" for errors that cause file load or save to abort.
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
     * @see ::getVerbosePrefix()
     * @see ::isVerbose()
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

    /**
     * Whether to include gate state parameters on a save.
     *
     * When TRUE, the JSON output is augmented with the transformed parameter
     * min/max for each gate dimension parameter, and each additional
     * clustering parameter. Since these min/max are based on transformed
     * event values in an associated source event table, this information
     * is specific to one use of the gate tree, and not generic.
     *
     * When FALSE, the JSON output does not include this use-specific
     * information.
     *
     * If the gate tree has no state, no additional information is saved.
     *
     * Loading a gate tree always ignores this additional information,
     * if present.
     *
     * @see ::save()
     * @see ::getFileIncludeGateStateParameters()
     * @see ::setFileIncludeGateStateParameters()
     */
    bool fileIncludeGateStateParameters = false;



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
        extensions.push_back( "json" );
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
        const int32_t n = vec.size( );
        for ( int32_t i = 0; i < n; ++i )
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
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( )
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
     * in the given FileFGJsonGates object.
     *
     * @param[in] file
     *   The FileFGJsonGates object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( const FileFGJsonGates*const file )
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
     * in the given FileFGJsonGates object.
     *
     * @param[in] file
     *   The FileFGJsonGates object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( const FileFGJsonGates& file )
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



    // Gate trees constructors -----------------------------------------
    /**
     * Constructs a new object using the given gate trees.
     *
     * The new object shares the given gate trees. External changes to the
     * gate trees will affect this object as well.
     *
     * @param[in] gateTrees
     *   The gate trees to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate trees is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees )
    {
        // Validate.
        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate trees." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Use.
        this->setGateTrees( gateTrees );
    }

    /**
     * Constructs a new object initialized with a copy of the given
     * gate trees.
     *
     * @param[in] gateTrees
     *   The gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( const FlowGate::Gates::GateTrees*const gateTrees )
    {
        // Validate.
        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate trees." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( gateTrees );
    }

    /**
     * Constructs a new object initialized with a copy of the given gate trees.
     *
     * @param[in] gateTrees
     *   The gate trees to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( const FlowGate::Gates::GateTrees& gateTrees )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( gateTrees );
    }



    // Load constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with data loaded from a
     * JSON gate file.
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
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileFGJsonGates( const std::string& path )
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
    virtual ~FileFGJsonGates( )
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
     * @li "path" (string) The path to the most recently loaded or saved file.
     * @li "versionNumber" (string) The format version name/number.
     *
     * @return
     *   Returns a map of string attribute keys to string attribute values.
     */
    inline const std::map<const std::string, const::std::string> getFileAttributes( )
        const noexcept
    {
        return this->fileAttributes;
    }



    // Misc. -----------------------------------------------------------
    /**
     * Returns whether gate state parameter information should be included
     * when a file is saved.
     *
     * @return
     *   Returns TRUE if the parameters used by a gate should have state
     *   information saved on the next file save.
     */
    inline bool getFileIncludeGateStateParameters( )
        const noexcept
    {
        return fileIncludeGateStateParameters;
    }

    /**
     * Sets whether gate state parameter information should be included
     * when a file is saved.
     *
     * @param bool
     *   When TRUE, the parameters used by a gate will have state
     *   information saved on the next file save.
     */
    inline void setFileIncludeGateStateParameters( const bool onOff )
        noexcept
    {
        fileIncludeGateStateParameters = onOff;
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
     * within the object. The new trees are used by further methods,
     * such as to save the gate trees to a file.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see ::load()
     * @see ::save()
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
                "Invalid NULL gate trees." );

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
     * within the object. The new trees are used by further methods,
     * such as to save the gate trees to a file.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see ::load()
     * @see ::save()
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
     * within the object. The new trees are used by further methods,
     * such as to save the gate trees to a file.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see ::load()
     * @see ::save()
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
     * will be used for all further operations, such as saving to a file.
     *
     * @param[in] gateTrees
     *   The gate trees to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate trees are a nullptr.
     *
     * @see ::reset()
     * @see ::copy()
     */
    inline void setGateTrees(
        std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees )
    {
        // Validate.
        if ( gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL gate trees." );

        // Initialize. The ID-to-gate map is left empty since it is only
        // needed during a save operation, which builds a map from the
        // gate tree as it exists at that time, which may differ from
        // the gate tree state when this method is called.
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
     *
     * @see ::load()
     * @see ::save()
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
                "Invalid empty file path" );


        //
        // Initialize.
        // -----------
        // Clear any prior data and the save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Loading JSON gate file \"" << filePath << "\".\n";

        this->reset( );
        this->fileIncludeGateStateParameters = false;

        std::string path = filePath;
        this->fileAttributes.emplace( "path", filePath );


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
        // Load the entire file into a text buffer for JSON parsing.
        std::shared_ptr<char> buffer;
        try
        {
            // Get the file size.
            size_t fileSize = 0L;
            if ( std::fseek( fp, 0, SEEK_END ) != 0 )
            {
                // Seek to end failed. SEEK_END is not a required feature
                // for fseek() implementations, so switch to an OS-specific
                // method.
#ifdef _POSIX_VERSION
                struct stat statBuffer;
                if ( stat( path.c_str(), &statBuffer ) == 0 )
                    fileSize = statBuffer.st_size;
#endif
            }
            else
            {
                const long size = std::ftell( fp );
                if ( size >= 0 )
                {
                    fileSize = size;
                    std::fseek( fp, 0, SEEK_SET );
                }
                else
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file size could not be retrieved from the system for file \"" ) +
                        path + std::string( "\". This is unusual and probably indicates the software is incompatible with this OS. Please report this to the developers." ) );
                    throw std::runtime_error(
                        std::string( "System error while reading the file.\n" ) +
                        std::string( "The file's size could not be retrieved from the system." ) +
                        ERROR_CANNOTLOAD );
                }
            }

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " <<
                    "  Loading file:\n";
                std::cerr << this->verbosePrefix << ": " << "    " <<
                    std::setw( 30 ) << std::left <<
                    "File size:" << fileSize << " bytes\n";
            }

            // Load the entire file into a buffer for JSON parsing.
            buffer.reset( new char[fileSize + 1] );
            if ( std::fread( buffer.get( ), 1, fileSize, fp ) != fileSize )
            {
                this->appendFileLog(
                    "error",
                    "A read of the entire file's contents failed." );
                throw std::runtime_error(
                    std::string( "Read error." ) +
                    std::string( "The file could not be fully read." ) +
                    ERROR_CANNOTLOAD );
            }
            buffer.get( )[fileSize] = '\0';

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
            std::fclose( fp );
            throw;
        }


        //
        // Parse JSON.
        // -----------
        // Aim the JSON parser at the buffer just read in. The parser
        // breaks the buffer down into nodes and values, or rejects it
        // all as malformed.
        int status;
        char* endptr;
        JsonValue rootValue;
        JsonAllocator allocator;
        if ( (status = jsonParse( buffer.get( ), &endptr, &rootValue, allocator )) != JSON_OK )
        {
            this->appendFileLog(
                "error",
                std::string( "The JSON parser failed with a parse error: \"" ) +
                std::string( jsonStrError(status) ) +
                std::string( "\"") );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain valid JSON." ) +
                ERROR_CANNOTLOAD );
        }

        // Verify that the root is an object. If it is anything else,
        // the file is not valid.
        if ( rootValue.getTag( ) != JSON_OBJECT )
        {
            this->appendFileLog(
                "error",
                "The JSON file's root node is not an object. The file is therefore not a valid JSON gates file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON gates." ) +
                ERROR_CANNOTLOAD );
        }

        // Loop through the root's keys looking for the format version.
        // If this is not present, the file is not valid.
        //
        // The "version" key is usually the first key in the object.
        std::string versionNumber;

        for ( const auto child: rootValue )
        {
            const auto& key = child->key;
            if ( std::strcmp( key, "version" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_STRING )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file's \"version\" key is present, but the value is not a string. The file is therefore not a valid JSON gates file." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file does not contain JSON gates." ) +
                        ERROR_CANNOTLOAD );
                }

                // Save the string.
                versionNumber = child->value.toString();
                this->fileAttributes.emplace( "versionNumber", versionNumber );

                // And check if it is recognized.
                if ( versionNumber != FORMAT_VERSION )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file's \"version\" key is present, indicating this may be a JSON gates file. But the version number in the key's value is not recognized. The version may be for a newer version of file format and this software is out of date." );
                    throw std::runtime_error(
                        ERROR_UNSUPPORTED +
                        std::string( "The file is using an unrecognized and unsupported file format version." ) +
                        ERROR_CANNOTLOAD );
                }
                break;
            }
        }

        if ( versionNumber.empty( ) == true )
        {
            this->appendFileLog(
                "error",
                "The JSON file does not include a \"version\" key giving the file format version number. The file is therefore not a valid JSON gates file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON gates." ) +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << versionNumber << "\"\n";

        //
        // Load gates.
        // -----------
        // At this point, the file text is valid JSON and there is a valid
        // version number. Start loading gate trees.
        try
        {
            loadGates( rootValue );
        }
        catch ( const std::exception& e )
        {
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Load from text.
//----------------------------------------------------------------------
public:
    /**
     * @name Load from text
     */
    // @{
    /**
     * Loads the indicated string's data.
     *
     * The character text array is parsed as text in the JSON gates format.
     *
     * The character text array becomes the property of this object and will
     * be modified during parsing. The buffer will be automatically deleted
     * when it is no longer needed.
     *
     * @param[in] text
     *   The text buffer to load.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the text buffer is a NULL.
     *
     * @see ::load()
     */
    void loadText( char*const text )
    {
        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( text == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL text buffer" );


        //
        // Initialize.
        // -----------
        // Clear any prior data and the save the path.
        this->reset( );

        this->fileAttributes.emplace( "path", "" );

        std::shared_ptr<char> buffer;
        buffer.reset( text );

        this->gateTrees->setFileName( "" );


        //
        // Parse JSON.
        // -----------
        // Aim the JSON parser at the buffer just read in. The parser
        // breaks the buffer down into nodes and values, or rejects it
        // all as malformed.
        int status;
        char* endptr;
        JsonValue rootValue;
        JsonAllocator allocator;
        if ( (status = jsonParse( buffer.get( ), &endptr, &rootValue, allocator )) != JSON_OK )
        {
            this->appendFileLog(
                "error",
                std::string( "The JSON parser failed with a parse error: \"" ) +
                std::string( jsonStrError(status) ) +
                std::string( "\"") );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain valid JSON." ) +
                ERROR_CANNOTLOAD );
        }

        // Verify that the root is an object. If it is anything else,
        // the file is not valid.
        if ( rootValue.getTag( ) != JSON_OBJECT )
        {
            this->appendFileLog(
                "error",
                "The JSON file's root node is not an object. The file is therefore not a valid JSON gates file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON gates." ) +
                ERROR_CANNOTLOAD );
        }

        // Loop through the root's keys looking for the format version.
        // If this is not present, the file is not valid.
        //
        // The "version" key is usually the first key in the object.
        std::string versionNumber;

        for ( const auto child: rootValue )
        {
            const auto& key = child->key;
            if ( std::strcmp( key, "version" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_STRING )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file's \"version\" key is present, but the value is not a string. The file is therefore not a valid JSON gates file." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file does not contain JSON gates." ) +
                        ERROR_CANNOTLOAD );
                }

                // Save the string.
                versionNumber = child->value.toString();
                this->fileAttributes.emplace( "versionNumber", versionNumber );

                // And check if it is recognized.
                if ( versionNumber != FORMAT_VERSION )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file's \"version\" key is present, indicating this may be a JSON gates file. But the version number in the key's value is not recognized. The version may be for a newer version of file format and this software is out of date." );
                    throw std::runtime_error(
                        ERROR_UNSUPPORTED +
                        std::string( "The file is using an unrecognized and unsupported file format version." ) +
                        ERROR_CANNOTLOAD );
                }
                break;
            }
        }

        if ( versionNumber.empty( ) == true )
        {
            this->appendFileLog(
                "error",
                "The JSON file does not include a \"version\" key giving the file format version number. The file is therefore not a valid JSON gates file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON gates." ) +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << versionNumber << "\"\n";

        //
        // Load gates.
        // -----------
        // At this point, the file text is valid JSON and there is a valid
        // version number. Start loading gate trees.
        try
        {
            loadGates( rootValue );
        }
        catch ( const std::exception& e )
        {
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Load gates from file or text.
//----------------------------------------------------------------------
private:
    /**
     * @name Load gates from file or text
     */
    // @{
    /**
     * Gets a string from a JSON node value.
     *
     * The node value is expected to be a JSON string value. An exception
     * is thrown if it is not.
     *
     * @param[in] key
     *   The key for the value, used in exception messages only.
     * @param[in] nodeValue
     *   The JSON node value from which to get the string.
     *
     * @return
     *   Returns the string value.
     *
     * @throws std::runtime_error
     *   Throws an exception if the value is not a string.
     */
    inline std::string getAndValidateString(
        const char*const key,
        const JsonValue& nodeValue )
    {
        if ( nodeValue.getTag( ) == JSON_STRING )
            return nodeValue.toString( );

        if ( nodeValue.getTag( ) == JSON_NULL )
            return "";

        this->appendFileLog(
            "error",
            std::string( "The JSON file includes a \"" ) +
            std::string( key ) +
            std::string( "\" key, but the value is not a string. The file is malformed." ) );

        throw std::runtime_error(
            ERROR_MALFORMED +
            ERROR_CANNOTLOAD );
    }

    /**
     * Gets a number from a JSON node value.
     *
     * The node value is expected to be a JSON string value. An exception
     * is thrown if it is not.
     *
     * @param[in] key
     *   The key for the value, used in exception messages only.
     * @param[in] nodeValue
     *   The JSON node value from which to get the number.
     *
     * @return
     *   Returns the numeric value.
     *
     * @throws std::runtime_error
     *   Throws an exception if the value is not a number.
     */
    inline double getAndValidateNumber(
        const char*const key,
        const JsonValue& nodeValue )
    {
        if ( nodeValue.getTag( ) == JSON_NUMBER )
            return nodeValue.toNumber( );

        this->appendFileLog(
            "error",
            std::string( "The JSON file includes a \"" ) +
            std::string( key ) +
            std::string( "\" key, but the value is not a number. The file is malformed." ) );

        throw std::runtime_error(
            ERROR_MALFORMED +
            ERROR_CANNOTLOAD );
    }

    /**
     * Finds the number of dimensions from a JSON value's children.
     *
     * The node value is expected to be a JSON object with the following
     * required key:
     * @li "numberOfDimensions" has a positive number.
     *
     * @param[in] nodeValue
     *   The JSON node value from which to get the number.
     *
     * @return
     *   Returns the numeric value.
     *
     * @throws std::runtime_error
     *   Throws an exception if the value is not a number, the number is not
     *   found, or the number is <= 0.
     */
    inline Gates::DimensionIndex findNumberOfDimensions(
        const JsonValue& nodeValue )
    {
        Gates::DimensionIndex numberOfDimensions = 0;
        bool foundNumberOfDimensions = false;

        // Loop through the value's children looking for the number of
        // dimensions. This needs to be known before we construct a gate.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "numberOfDimensions" ) == 0 )
            {
                if ( child->value.getTag( ) == JSON_STRING )
                {
                    numberOfDimensions = std::stol( child->value.toString( ) );
                    foundNumberOfDimensions = true;
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"numberOfDimensions\" key that should have a numeric value, but a string value was found. The file is malformed but the string was parsed as a number anyway." );
                    break;
                }
                if ( child->value.getTag( ) == JSON_NUMBER )
                {
                    numberOfDimensions = child->value.toNumber( );
                    foundNumberOfDimensions = true;
                    break;
                }

                this->appendFileLog(
                    "error",
                    "The JSON file includes a \"numberOfDimensions\" key on a gate, but the value is not a string or number. The file is malformed." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_CANNOTLOAD );
            }
        }

        if ( numberOfDimensions > 0 )
            return numberOfDimensions;

        if ( foundNumberOfDimensions == false )
        {
            this->appendFileLog(
                "error",
                "The JSON file describes a gate, but it does not provide a \"numberOfDimensions\" key. The file is malformed." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }

        this->appendFileLog(
            "error",
            "The JSON file describes a gate, but the value for the \"numberOfDimensions\" key an invalid non-positive value. The file is malformed." );
        throw std::runtime_error(
            ERROR_MALFORMED +
            ERROR_CANNOTLOAD );
    }

    /**
     * Traverses the JSON tree to load gates.
     *
     * Top-level root gates are at the top of the tree. Child gates
     * are nested within "gates" keys, which may in turn have nested
     * "gates" keys, and so on to build the tree.
     *
     * @param[in] rootValue
     *   The JSON root node value.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    void loadGates( const JsonValue& rootValue )
    {
        if ( this->isVerbose( ) == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading gates:\n";


        //
        // Loop through top level items.
        // -----------------------------
        // Look for well-known keys and, if properly formed, copy them
        // into the gate trees. The most important is the "gates" key
        // whose value is an array of gate roots.
        this->gateTrees.reset( new FlowGate::Gates::GateTrees( ) );

        for ( const auto child: rootValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "creatorSoftwareName" ) == 0 )
            {
                this->gateTrees->setCreatorSoftwareName(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "creatorSoftwareName",
                    this->gateTrees->getCreatorSoftwareName( ) );
                continue;
            }
            if ( std::strcmp( key, "name" ) == 0 )
            {
                this->gateTrees->setName(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "name",
                    this->gateTrees->getName( ) );
                continue;
            }
            if ( std::strcmp( key, "description" ) == 0 )
            {
                this->gateTrees->setDescription(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "description",
                    this->gateTrees->getDescription( ) );
                continue;
            }
            if ( std::strcmp( key, "notes" ) == 0 )
            {
                this->gateTrees->setNotes(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "notes",
                    this->gateTrees->getNotes( ) );
                continue;
            }
            if ( std::strcmp( key, "fcsFileName" ) == 0 )
            {
                this->gateTrees->setFCSFileName(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "fcsFileName",
                    this->gateTrees->getFCSFileName( ) );
                continue;
            }
            if ( std::strcmp( key, "gates" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"gates\" key, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                // Each child in the array is a gate object for a
                // gate tree root.
                for ( const auto g: child->value )
                {
                    if ( g->value.getTag( ) != JSON_OBJECT )
                    {
                        this->appendFileLog(
                            "error",
                            "The JSON file includes a \"gates\" key for a root gate, but the value is not a gate object. The file is malformed." );
                        throw std::runtime_error(
                            ERROR_MALFORMED +
                            ERROR_CANNOTLOAD );
                    }

                    this->loadGate( g->value, nullptr );
                }
                continue;
            }

            // Ignore any other keys.
        }


        const Gates::GateIndex nRoots = this->gateTrees->getNumberOfGateTrees( );

        if ( nRoots == 0 )
        {
            this->appendFileLog(
                "warning",
                "The JSON gates file does not have any recognized gates." );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of gate trees:" <<
                nRoots << "\n";
    }

    /**
     * Parses a JSON value as a transform object.
     *
     * A transform object has the following required keys:
     * @li "transformType" with the well-known transform type name.
     *
     * And the following optional keys.
     * @li "name" with a user-chosen name.
     * @li "description with a user-chosen name.
     *
     * Depending upon the transform type, additional keys provide numeric
     * arguments to the transform:
     * @li "a" for the A argument.
     * @li "m" for the M argument.
     * @li "t" for the T argument.
     * @li "w" for the W argument.
     *
     * @param[in] nodeValue
     *   The JSON node value from which to get the transform.
     *
     * @return
     *   Returns the transform, or a NULL if there is no transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if the value is not a transform.
     */
    inline std::shared_ptr<FlowGate::Gates::Transform> loadTransform(
        const JsonValue& nodeValue )
    {
        FlowGate::Gates::TransformType transformType =
            FlowGate::Gates::CUSTOM_TRANSFORM;
        std::string transformTypeName;
        std::string name;
        std::string description;
        double t = 0.0;
        double m = 0.0;
        double a = 0.0;
        double w = 0.0;
        bool foundT = false;
        bool foundM = false;
        bool foundA = false;
        bool foundW = false;
        bool emptyObject = true;

        if ( nodeValue.getTag( ) == JSON_NULL )
        {
            return nullptr;
        }

        if ( nodeValue.getTag( ) == JSON_STRING )
        {
            const std::string s = nodeValue.toString( );
            if ( s.empty( ) == true ||
                 s.compare( "null" ) == 0 ||
                 s.compare( "NULL" ) == 0 ||
                 s.compare( "\"null\"" ) == 0 )
            {
                return nullptr;
            }

            this->appendFileLog(
                "error",
                std::string( "The JSON file includes a \"transform\" key for gate transforms, but the value is not a transform object. The file is malformed." ) );

            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }

        //
        // Determine transform type.
        // -------------------------
        // Loop through the keys to find the transform type and other
        // transform attributes.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            emptyObject = false;
            if ( std::strcmp( key, "transformType" ) == 0 )
            {
                transformTypeName =
                    this->getAndValidateString( key, child->value );

                // Convert to a numeric gate type. If the type is
                // not recognized, CUSTOM_TRANSFORM is returned.
                transformType = FlowGate::Gates::findTransformTypeByName( transformTypeName );
                continue;
            }
            if ( std::strcmp( key, "name" ) == 0 )
            {
                name = this->getAndValidateString( key, child->value );
                continue;
            }
            if ( std::strcmp( key, "description" ) == 0 )
            {
                description = this->getAndValidateString( key, child->value );
                continue;
            }
            if ( std::strcmp( key, "a" ) == 0 )
            {
                a = this->getAndValidateNumber( key, child->value );
                foundA = true;
                continue;
            }
            if ( std::strcmp( key, "m" ) == 0 )
            {
                m = this->getAndValidateNumber( key, child->value );
                foundM = true;
                continue;
            }
            if ( std::strcmp( key, "t" ) == 0 )
            {
                t = this->getAndValidateNumber( key, child->value );
                foundT = true;
                continue;
            }
            if ( std::strcmp( key, "w" ) == 0 )
            {
                w = this->getAndValidateNumber( key, child->value );
                foundW = true;
                continue;
            }

            // Ignore other keys.
        }

        // If the JSON object was empty (e.g. "{ }"), then there is no
        // transform. Return a NULL.
        if ( emptyObject == true )
            return nullptr;

        // If the JSON object did not indicate the transform type, then
        // something is broken.
        if ( transformTypeName.empty( ) == true )
        {
            this->appendFileLog(
                "error",
                "The JSON file describes a transform, but omits the transform type." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file contains invalid transform types." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Create transform.
        // -----------------
        // Use the transform type to select the type of transform to
        // construct.
        std::shared_ptr<FlowGate::Gates::Transform> transform;

        switch ( transformType )
        {
        case FlowGate::Gates::PARAMETERIZED_LINEAR_TRANSFORM:
            if ( foundT == false || foundA == false )
            {
                this->appendFileLog(
                    "error",
                    "The JSON file describes a parameterized linear transform, but does not provide both T and A argument values." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete transform." ) +
                    ERROR_CANNOTLOAD );
            }
            transform.reset(
                new FlowGate::Gates::ParameterizedLinearTransform( t, a ) );
            break;

        case FlowGate::Gates::PARAMETERIZED_LOGARITHMIC_TRANSFORM:
            if ( foundT == false || foundM == false )
            {
                this->appendFileLog(
                    "error",
                    "The JSON file describes a parameterized logarithmic transform, but does not provide both T and M argument values." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete transform." ) +
                    ERROR_CANNOTLOAD );
            }
            transform.reset(
                new FlowGate::Gates::ParameterizedLogarithmicTransform( t, m ) );
            break;

        case FlowGate::Gates::PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM:
            if ( foundT == false || foundA == false || foundM == false )
            {
                this->appendFileLog(
                    "error",
                    "The JSON file describes a parameterized inverse hyperbolic sine transform, but does not provide all three T, A, and M argument values." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete transform." ) +
                    ERROR_CANNOTLOAD );
            }
            transform.reset(
                new FlowGate::Gates::ParameterizedInverseHyperbolicSineTransform( t, a, m ) );
            break;

        case FlowGate::Gates::LOGICLE_TRANSFORM:
            if ( foundT == false || foundA == false ||
                foundM == false || foundW == false )
            {
                this->appendFileLog(
                    "error",
                    "The JSON file describes a logicle transform, but does not provide all four T, A, M, and W argument values." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete transform." ) +
                    ERROR_CANNOTLOAD );
            }
            transform.reset(
                new FlowGate::Gates::LogicleTransform( t, a, m, w ) );
            break;

        case FlowGate::Gates::HYPERLOG_TRANSFORM:
            if ( foundT == false || foundA == false ||
                foundM == false || foundW == false )
            {
                this->appendFileLog(
                    "error",
                    "The JSON file describes a hyperlog transform, but does not provide all four T, A, M, and W argument values." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete transform." ) +
                    ERROR_CANNOTLOAD );
            }
            transform.reset(
                new FlowGate::Gates::HyperlogTransform( t, a, m, w ) );
            break;

        default:
        case FlowGate::Gates::CUSTOM_TRANSFORM:
            this->appendFileLog(
                "error",
                std::string( "The JSON file describes a transform with a transform type of \"" ) +
                transformTypeName +
                std::string( "\", but this transform type is not recognized." ) );

            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file contains invalid transform types." ) +
                ERROR_CANNOTLOAD );
        }

        transform->setName( name );
        transform->setDescription( description );

        return transform;
    }

    /**
     * Parses a JSON node value as a gate tree, recursively.
     *
     * The given JSON node is traversed to collect gate attributes
     * and recurse to parse gate children.
     *
     * @param[in] nodeValue
     *   The JSON node value from which to get the gate.
     * @param[in] parentGate
     *   The parent gate into which to add the parsed gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    void loadGate(
        const JsonValue& nodeValue,
        std::shared_ptr<FlowGate::Gates::Gate> parent )
    {
        //
        // Determine gate type.
        // --------------------
        // Loop through the node's keys to find the gate type.
        FlowGate::Gates::GateType gateType = FlowGate::Gates::CUSTOM_GATE;
        std::string gateTypeName;

        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "gateType" ) == 0 )
            {
                gateTypeName = this->getAndValidateString( key, child->value );

                // Convert to a numeric gate type. If the type is
                // not recognized, CUSTOM_GATE is returned.
                gateType = FlowGate::Gates::findGateTypeByName( gateTypeName );
                break;
            }
        }

        // If the gate did not include a gate type, then something is wrong.
        if ( gateTypeName.empty( ) == true )
        {
            this->appendFileLog(
                "error",
                "The JSON file describes a gate, but omits the gate type." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file contains invalid gate types." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Dispatch based on gate type.
        // ----------------------------
        // Use the gate type to dispatch to a parser specific to each
        // gate type to collect that type's attributes.
        //
        // Each parser throws an exception on errors, or returns a gate
        // on success. The parsers DO NOT parse children. That is done below.
        std::shared_ptr<FlowGate::Gates::Gate> gate;
        switch ( gateType )
        {
        case FlowGate::Gates::RECTANGLE_GATE:
            gate = this->loadRectangleGate( nodeValue );
            break;

        case FlowGate::Gates::POLYGON_GATE:
            gate = this->loadPolygonGate( nodeValue );
            break;

        case FlowGate::Gates::ELLIPSOID_GATE:
        case FlowGate::Gates::QUADRANT_GATE:
        case FlowGate::Gates::BOOLEAN_GATE:
            this->appendFileLog(
                "error",
                std::string( "The JSON file describes a gate with a gate type of \"" ) +
                gateTypeName +
                std::string( "\", but this gate type is not supported yet." ) );

            throw std::runtime_error(
                ERROR_UNSUPPORTED +
                std::string( "The file contains unsupported gate types." ) +
                ERROR_CANNOTLOAD );

        default:
        case FlowGate::Gates::CUSTOM_GATE:
            this->appendFileLog(
                "error",
                std::string( "The JSON file describes a gate with a gate type of \"" ) +
                gateTypeName +
                std::string( "\", but this gate type is not recognized." ) );

            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file contains invalid gate types." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Add gate to parent.
        // -------------------
        // The parsed gate is complete, but has no children yet (if any).
        // Add the gate to the parent, if any, or as a gate trees root.
        if ( parent == nullptr )
        {
            // Add the gate as a root.
            this->gateTrees->appendGateTree( gate );
        }
        else
        {
            // Add the gate as a child of the parent.
            parent->appendChild( gate );
        }


        //
        // Parse children, if any.
        // -----------------------
        // Look for a 'gates' key with an array of children. Recurse.
        // Leaf gates either have no 'gates' key, or its gates array is empty.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "gates" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"gates\" key, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                // Each child in the array is a gate object for a
                // gate tree root.
                for ( const auto g: child->value )
                {
                    if ( g->value.getTag( ) != JSON_OBJECT )
                    {
                        this->appendFileLog(
                            "error",
                            "The JSON file includes a \"gates\" key on a gate, but the value is not a gate object. The file is malformed." );
                        throw std::runtime_error(
                            ERROR_MALFORMED +
                            ERROR_CANNOTLOAD );
                    }

                    this->loadGate( g->value, gate );
                }
                break;
            }
        }
    }

    /**
     * Traverses the JSON tree to load a single rectangle gate.
     *
     * The given JSON node is traversed to collect gate attributes.
     *
     * @param[in] nodeValue
     *   The JSON node value.
     * @param[in] parentGate
     *   The parent gate into which to add the parsed gate.
     *
     * @return
     *   Returns the parsed gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    std::shared_ptr<FlowGate::Gates::Gate> loadRectangleGate(
        const JsonValue& nodeValue )
    {
        //
        // Get number of dimensions.
        // -------------------------
        // This is needed to construct the gate.
        const Gates::DimensionIndex numberOfDimensions =
            this->findNumberOfDimensions( nodeValue );


        //
        // Create the gate with defaults.
        // ------------------------------
        // With the number of dimensions in hand, create the gate.
        FlowGate::Gates::RectangleGate* rectangle =
            new FlowGate::Gates::RectangleGate( numberOfDimensions );
        std::shared_ptr<FlowGate::Gates::Gate> gate;
        gate.reset( rectangle );


        //
        // Parse general attributes.
        // -------------------------
        // General attributes, like the gate's name and transforms, are
        // parsed and added to the gate.
        this->loadGeneralGateAttributes( nodeValue, gate );


        //
        // Parse rectangle-specific attributes.
        // ------------------------------------
        // Gate type-specific attributes, like the min/max per dimension,
        // are parsed and added to the gate.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "minimumByDimension" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"minimumByDimension\" key for a rectangle gate, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                // Loop through the array and load each numeric value.
                Gates::DimensionIndex dimension = 0;
                for ( const auto g: child->value )
                {
                    if ( dimension >= numberOfDimensions )
                    {
                        ++dimension;
                        break;
                    }

                    rectangle->setDimensionMinimum( dimension,
                        this->getAndValidateNumber( key, g->value ) );
                    ++dimension;
                }

                if ( dimension < numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"minimumByDimension\" key on a rectangle gate with fewer values than the rectangle has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                if ( dimension > numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"minimumByDimension\" key on a rectangle gate with more values than the rectangle has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                continue;
            }
            if ( std::strcmp( key, "maximumByDimension" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"maximumByDimension\" key for a rectangle gate, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                // Loop through the array and load each numeric value.
                Gates::DimensionIndex dimension = 0;
                for ( const auto g: child->value )
                {
                    if ( dimension >= numberOfDimensions )
                    {
                        ++dimension;
                        break;
                    }

                    rectangle->setDimensionMaximum( dimension,
                        this->getAndValidateNumber( key, g->value ) );
                    ++dimension;
                }

                if ( dimension < numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"maximumByDimension\" key on a rectangle gate with fewer values than the rectangle has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                if ( dimension > numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"maximumByDimension\" key on a rectangle gate with more values than the rectangle has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                continue;
            }

            // Ignore any other keys.
        }

        return gate;
    }

    /**
     * Traverses the JSON tree to load a single polygon gate.
     *
     * The given JSON node is traversed to collect gate attributes.
     *
     * @param[in] nodeValue
     *   The JSON node value.
     * @param[in] parentGate
     *   The parent gate into which to add the parsed gate.
     *
     * @return
     *   Returns the parsed gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    std::shared_ptr<FlowGate::Gates::Gate> loadPolygonGate(
        const JsonValue& nodeValue )
    {
        //
        // Get number of dimensions.
        // -------------------------
        // Polygon gates are always 2D.
        const Gates::DimensionIndex numberOfDimensions =
            this->findNumberOfDimensions( nodeValue );
        if ( numberOfDimensions != 2 )
        {
            this->appendFileLog(
                "error",
                "The JSON file includes a \"numberOfDimensions\" key for a 2D polygon gate, but the value is not two. The file is malformed." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }


        //
        // Create the gate with defaults.
        // ------------------------------
        // With the number of dimensions in hand, create the gate.
        FlowGate::Gates::PolygonGate* polygon =
            new FlowGate::Gates::PolygonGate( );
        std::shared_ptr<FlowGate::Gates::Gate> gate;
        gate.reset( polygon );


        //
        // Parse general attributes.
        // -------------------------
        // General attributes, like the gate's name and transforms, are
        // parsed and added to the gate.
        this->loadGeneralGateAttributes( nodeValue, gate );


        //
        // Parse polygon-specific attributes.
        // ----------------------------------
        // Gate type-specific attributes, like the polygon's vertex list,
        // are parsed and added to the gate.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "vertices" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"vertices\" key for a polygon gate, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                // Loop through the array. Each entry should be a 2D array
                // with X and Y values.
                for ( const auto v: child->value )
                {
                    if ( v->value.getTag( ) != JSON_ARRAY )
                    {
                        this->appendFileLog(
                            "error",
                            "The JSON file includes a \"vertices\" key for a polygon gate, but one of the values is not an array. The file is malformed." );
                        throw std::runtime_error(
                            ERROR_MALFORMED +
                            ERROR_CANNOTLOAD );
                    }

                    double x = 0.0;
                    double y = 0.0;
                    Gates::VertexIndex n = 0;
                    for ( const auto num: v->value )
                    {
                        if ( n == 0 )
                            x = this->getAndValidateNumber( "vertices", num->value );
                        else
                            y = this->getAndValidateNumber( "vertices", num->value );
                        ++n;
                    }

                    if ( n != 2 )
                    {
                        this->appendFileLog(
                            "error",
                            "The JSON file includes a \"vertices\" key for a polygon gate, but one of the array values does not have two vertex components. The file is malformed." );
                        throw std::runtime_error(
                            ERROR_MALFORMED +
                            ERROR_CANNOTLOAD );
                    }

                    polygon->appendVertex( x, y );
                }

                if ( polygon->getNumberOfVertices( ) < 3 )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"vertices\" key for a polygon gate, but the array has fewer than 3 entries. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                continue;
            }

            // Ignore any other keys.
        }

        return gate;
    }


    /**
     * Traverses the JSON tree to add general gate attributes to a gate.
     *
     * The given JSON node is traversed to collect gate attributes.
     *
     * @param[in] nodeValue
     *   The JSON node value.
     * @param[in] gate
     *   The gate to add attributes to.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    void loadGeneralGateAttributes(
        const JsonValue& nodeValue,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        const Gates::DimensionIndex numberOfDimensions = gate->getNumberOfDimensions( );

        //
        // Collect attributes.
        // -------------------
        // Loop over the gate's keys looking for general gate attributes.
        // Add them to the given gate.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "gatingMethodName" ) == 0 )
            {
                const std::string gatingMethodName =
                    this->getAndValidateString( key, child->value );

                // Convert to a numeric gate type. If the type is
                // not recognized, CUSTOM_GATING is returned.
                gate->setGatingMethod(
                    FlowGate::Gates::findGatingMethodByName( gatingMethodName ) );
                continue;
            }
            if ( std::strcmp( key, "id" ) == 0 )
            {
                gate->setOriginalId(
                    this->getAndValidateString( key, child->value ) );
                continue;
            }
            if ( std::strcmp( key, "name" ) == 0 )
            {
                gate->setName(
                    this->getAndValidateString( key, child->value ) );
                continue;
            }
            if ( std::strcmp( key, "description" ) == 0 )
            {
                gate->setDescription(
                    this->getAndValidateString( key, child->value ) );
                continue;
            }
            if ( std::strcmp( key, "notes" ) == 0 )
            {
                gate->setNotes(
                    this->getAndValidateString( key, child->value ) );
                continue;
            }
            if ( std::strcmp( key, "reportPriority" ) == 0 )
            {
                const auto n = this->getAndValidateNumber( key, child->value );
                if ( n < 0.0 )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"reportPriority\" key for a gate, but the value is not zero or positive. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                gate->setReportPriority( n );
                continue;
            }
            if ( std::strcmp( key, "transformByDimension" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"transformByDimension\" key for a gate, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                Gates::DimensionIndex dimension = 0;
                for ( const auto g: child->value )
                {
                    if ( dimension >= numberOfDimensions )
                    {
                        ++dimension;
                        break;
                    }

                    gate->setDimensionParameterTransform( dimension,
                        this->loadTransform( g->value ) );
                    ++dimension;
                }

                if ( dimension < numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"parameterNameByDimension\" key on a gate with fewer values than the gate has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                if ( dimension > numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"parameterNameByDimension\" key on a gate with more values than the gate has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                continue;
            }
            if ( std::strcmp( key, "parameterNameByDimension" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"parameterNameByDimension\" key for a rectangle gate, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                Gates::DimensionIndex dimension = 0;
                for ( const auto g: child->value )
                {
                    if ( dimension >= numberOfDimensions )
                    {
                        ++dimension;
                        break;
                    }

                    gate->setDimensionParameterName( dimension,
                        this->getAndValidateString( key, g->value ) );
                    ++dimension;
                }

                if ( dimension < numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"parameterNameByDimension\" key on a gate with fewer values than the gate has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                if ( dimension > numberOfDimensions )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"parameterNameByDimension\" key on a gate with more values than the gate has dimensions. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                continue;
            }
            if ( std::strcmp( key, "additionalClusteringParameters" ) == 0 )
            {
                this->loadAdditionalClusteringParameters( child->value, gate );
                continue;
            }

            // Ignore any other keys.
        }
    }

    /**
     * Parses a JSON value as a list of initial clustering parameters.
     *
     * Parsed clustering parameters are added to the gate trees.
     *
     * @param[in] nodeValue
     *   The JSON node value from which to get the values
     * @param[in] gate
     *   The gate to add attributes to.
     *
     * @throws std::runtime_error
     *   Throws an exception if the value is not a transform.
     */
    inline void loadAdditionalClusteringParameters(
        const JsonValue& nodeValue,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        // Input looks like:
        //   "additionalClusteringParameters": [
        //     {
        //       "name": "NAME",
        //       "transform":
        //         {
        //           ...
        //         }
        //     }
        //     ...
        //   ]
        //
        // The incoming node is for the list of parameter objects.
        //
        if ( nodeValue.getTag( ) != JSON_ARRAY )
        {
            this->appendFileLog(
                "error",
                "The JSON file includes an \"additionalClusteringParameters\" key, but the value is not an array. The file is malformed." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }

        // Loop through the parameter list. Each entry is an object with
        // the parameter name and transform.
        for ( const auto child: nodeValue )
        {
            if ( child->value.getTag( ) != JSON_OBJECT )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The JSON file includes an \"additionalClusteringParameters\" array, but one of the values is not an object. The file is malformed." ) );

                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_CANNOTLOAD );
            }

            std::string name;
            std::shared_ptr<FlowGate::Gates::Transform> transform;

            // Loop through the object's keys to get the name and transform.
            for ( const auto grandChild: child->value )
            {
                auto& key = grandChild->key;

                if ( std::strcmp( key, "name" ) == 0 )
                    name = this->getAndValidateString( key, grandChild->value );
                else if (std::strcmp( key, "transform" ) == 0 )
                    transform = this->loadTransform( grandChild->value );
            }

            gate->appendAdditionalClusteringParameter( name, transform );
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
     * Saves the object to a new file.
     *
     * The file is written with data from the current object.
     *
     * @param[in] filePath
     *   The path to the file to save.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Load problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void save( const std::string& filePath )
    {
        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( filePath.empty() == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file path." );
        if ( this->gateTrees == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "Invalid NULL gate trees." ) );


        //
        // Initialize.
        // -----------
        // Save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Saving JSON gate file \"" << filePath << "\".\n";

        this->clearFileLog( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );


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
        // Generate JSON.
        std::string json;
        this->saveGates( json, "  " );

        try
        {
#ifdef FLOWGATE_FILEFGJSONGATES_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            if ( std::fwrite( json.c_str( ), 1, json.length( ), fp ) < json.length( ) )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file gates: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

#ifdef FLOWGATE_FILEFGJSONGATES_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGJSONGATES_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Save to string.
//----------------------------------------------------------------------
public:
    /**
     * @name Save to string
     */
    // @{
    /**
     * Saves the object to a new string.
     *
     * A string is returned with data from the current gate trees.
     *
     * @param[in] indent
     *   (optional, default = "  ") The per-line indent for the returned text.
     *
     * @see ::getFileLog()
     * @see ::save()
     */
    std::string saveText( const std::string indent = "  " )
        const noexcept
    {
        std::string json;
        this->saveGates( json, indent );
        return json;
    }
    // @}



//----------------------------------------------------------------------
// Save gates.
//----------------------------------------------------------------------
private:
    /**
     * @name Save gates.
     */
    // @{
    /**
     * Saves gates.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gates.
     * @param[in] indent
     *   The line indent.
     *
     * @see ::getFileLog()
     */
    void saveGates( std::string& json, const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the gate trees.
        const std::string name =
            this->gateTrees->getName( );
        const std::string description =
            this->gateTrees->getDescription( );
        const std::string notes =
            this->gateTrees->getNotes( );
        const std::string fcsFileName =
            this->gateTrees->getFCSFileName( );
        const std::string software =
            this->gateTrees->getCreatorSoftwareName( );
        const Gates::GateIndex numberOfGateTrees =
            this->gateTrees->getNumberOfGateTrees( );

        const std::string indentFields = indent + "  ";
        const std::string indentGate = indent + "    ";

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving gates:\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of gate trees:" <<
                numberOfGateTrees << "\n";
        }

        // Start the file's object.
        json += indent + "{\n";

        //
        // Header.
        // -------
        // The header contains optional information about the gate trees.
        //
        // Output looks like:
        //   "version": "FORMAT_VERSION",
        //   "name": "NAME",
        //   "description": "DESCRIPTION",
        //   "notes": "NOTES",
        //   "fcsFileName": "FCSFILENAME",
        //   "creatorSoftwareName": "NAME",
        //   ...
        //
        // Only the format version is always present. The others are present
        // only if they are non-empty strings.
        //
        json += indentFields + "\"version\": \"" + FORMAT_VERSION + "\",\n";

        if ( name.empty( ) == false )
            json += indentFields + "\"name\": \"" + name + "\",\n";
        if ( description.empty( ) == false )
            json += indentFields + "\"description\": \"" + description + "\",\n";
        if ( notes.empty( ) == false )
            json += indentFields + "\"notes\": \"" + notes + "\",\n";
        if ( fcsFileName.empty( ) == false )
            json += indentFields + "\"fcsFilename\": \"" + fcsFileName + "\",\n";
        if ( software.empty( ) == false )
            json += indentFields + "\"creatorSoftware\": \"" + software + "\",\n";

        //
        // Gates.
        // ------
        // Iterate over the gates and write them out.
        //
        // Output looks like:
        //   "gates": [
        //     {
        //       "gateType": "TYPE",
        //       ...
        //     }
        //     ...
        //   ]
        //
        json += indentFields + "\"gates\": [\n";
        for ( Gates::GateIndex i = 0; i < numberOfGateTrees; ++i )
        {
            json += indentGate + "{\n";

            this->saveGate(
                json,
                this->gateTrees->getGateTree( i ),
                indentGate);

            if ( i == (numberOfGateTrees-1) )
                json += indentGate + "}\n";
            else
                json += indentGate + "},\n";
        }
        json += indentFields + "]\n";
        json += indent + "}\n";
    }

    /**
     * Saves the gate to the file.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] gate
     *   The gate.
     * @param[in] indent
     *   The line indent.
     *
     * @see getFileLog()
     */
    void saveGate(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Gate> gate,
        const std::string& indent )
        const noexcept
    {
        const std::string indentArguments = indent + "  ";

        switch ( gate->getGateType( ) )
        {
        case FlowGate::Gates::RECTANGLE_GATE:
            this->saveGateStart( json, gate, indent );
            this->saveRectangleGate( json, gate, indentArguments );
            this->saveGateEnd( json, gate, indent );
            break;

        case FlowGate::Gates::POLYGON_GATE:
            this->saveGateStart( json, gate, indent );
            this->savePolygonGate( json, gate, indentArguments );
            this->saveGateEnd( json, gate, indent );
            break;

        case FlowGate::Gates::ELLIPSOID_GATE:
            //this->saveGateStart( json, gate, indent );
            //this->saveEllipsoidGate( json, gate, indentArguments );
            //this->saveGateEnd( json, gate, indent );
            break;

        case FlowGate::Gates::QUADRANT_GATE:
            //this->saveGateStart( json, gate, indent );
            //this->saveQuadrantGate( json, gate, indentArguments );
            //this->saveGateEnd( json, gate, indent );
            break;

        case FlowGate::Gates::BOOLEAN_GATE:
            //this->saveGateStart( json, gate, indent );
            //this->saveBooleanGate( json, gate, indentArguments );
            //this->saveGateEnd( json, gate, indent );
            break;

        default:
            // Unknown gate type?
            break;
        }
    }

    /**
     * Saves the start of a gate.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] gate
     *   The gate.
     * @param[in] indent
     *   The line indent.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void saveGateStart(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Gate> gate,
        const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the gate.
        const std::string name             = gate->getName( );
        const std::string description      = gate->getDescription( );
        const std::string notes            = gate->getNotes( );
        const std::string gateTypeName     = gate->getGateTypeName( );
        const std::string gatingMethodName = gate->getGatingMethodName( );
        const Gates::DimensionIndex nDimensions = gate->getNumberOfDimensions( );
        const uint32_t reportPriority      = gate->getReportPriority( );

        const std::string id = std::string( "Gate_" ) +
            std::to_string( gate->getId( ) );

        const std::string indent2 = indent + "  ";
        const std::string indent3 = indent2 + "  ";


        //
        // Header.
        // -------
        // The gate header includes the gate's name, description, type, etc.
        //
        // Output looks like:
        //   "id": "ID",
        //   "name": "NAME",
        //   "description": "DESCRIPTION",
        //   "notes": "NOTES",
        //   "gateType": "TYPE",
        //   "gatingMethodName": "METHOD",
        //   "reportPriority": PRIORITY,
        //   "numberOfDimensions": NUMBER,
        //   ...
        //
        // The name, description, and notes are omitted if they are empty. The
        // other fields are always present.
        //
        json += indent2 + "\"id\": \"" + id + "\",\n";

        if ( name.empty( ) == false )
            json += indent2 + "\"name\": \"" + name + "\",\n";

        if ( description.empty( ) == false )
            json += indent2 + "\"description\": \"" + description + "\",\n";
        if ( notes.empty( ) == false )
            json += indent2 + "\"notes\": \"" + notes + "\",\n";

        json += indent2 + "\"gateType\": \"" + gateTypeName + "\",\n";
        json += indent2 + "\"gatingMethodName\": \"" + gatingMethodName + "\",\n";
        json += indent2 + "\"reportPriority\": " + std::to_string( reportPriority ) + ",\n";
        json += indent2 + "\"numberOfDimensions\": " + std::to_string( nDimensions ) + ",\n";
        if ( gate->hasAdditionalClusteringParameters( ) == true )
            this->saveAdditionalClusteringParameters( json, gate, indent2 );


        //
        // Per-dimension.
        // --------------
        // Every gate may have zero or more dimensions, each of which has
        // additional information.
        //
        // Output looks like:
        //   "parameterNameByDimension": [
        //     "NAME",
        //     "NAME",
        //     ...
        //   ],
        //   "transformByDimension": [
        //     {
        //       "transformType": "TYPE",
        //       ...
        //     },
        //     ...
        //   ]
        //
        // If a parameter's transform is a nullptr, the transform is written
        // out as the word "null" instead of an object in "{...}".
        //
        // Parameter names.
        json += indent2 + "\"parameterNameByDimension\": [\n";
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            json += indent3 + "\"" + gate->getDimensionParameterName( i );

            if ( i == (nDimensions-1) )
                json += "\"\n";
            else
                json += "\",\n";
        }

        json += indent2 + "],\n";

        // Transforms.
        json += indent2 + "\"transformByDimension\": [\n";
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            this->saveTransform(
                json,
                gate->getDimensionParameterTransform( i ),
                indent3,
                (i == (nDimensions-1)) );
        }
        json += indent2 + "],\n";

        // Transformed parameter min/max.
        if ( this->fileIncludeGateStateParameters == true )
        {
            const auto state = gate->getState( );
            const auto table = std::dynamic_pointer_cast<const FlowGate::Events::EventTableInterface>( state );
            if ( table != nullptr && table->getNumberOfParameters( ) > 0 )
            {
                // The number of parameters can be larger than the gate's
                // dimension if there are additional clustering parameters.
                // However, we don't want those. So just use the first few
                // up to the gate's dimension.
                json += indent2 + "\"transformedParameterMinimumByDimension\": [\n";

                for ( size_t i = 0; i < nDimensions; ++i )
                {
                    const auto min = table->getParameterMinimum( i );
                    json += indent3 + std::to_string( min );
                    if ( i == (nDimensions-1) )
                        json += "\n";
                    else
                        json += ",\n";
                }
                json += indent2 + "],\n";

                json += indent2 + "\"transformedParameterMaximumByDimension\": [\n";
                for ( size_t i = 0; i < nDimensions; ++i )
                {
                    const auto max = table->getParameterMaximum( i );
                    json += indent3 + std::to_string( max );
                    if ( i == (nDimensions-1) )
                        json += "\n";
                    else
                        json += ",\n";
                }
                json += indent2 + "],\n";
            }
        }
    }

    /**
     * Saves the additional clustering parameters.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] gate
     *   The gate.
     * @param[in] indent
     *   The line indent.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void saveAdditionalClusteringParameters(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Gate> gate,
        const std::string& indent )
        const noexcept
    {
        // If there is no parameter list, there is nothing to output.
        const Gates::DimensionIndex n =
            gate->getNumberOfAdditionalClusteringParameters( );
        if ( n == 0 )
            return;

        const std::string indentT = indent + "      ";

        // Output looks like:
        //   "additionalClusteringParameters": [
        //     {
        //       "name": "NAME",
        //       "transform":
        //         {
        //           ...
        //         }
        //     }
        //     ...
        //   ]
        //
        // If the parameter has no transform, then the "transform" field
        // value is "null" instead of an object in "{...}".
        json += indent + "\"additionalClusteringParameters\": [\n";

        for ( Gates::DimensionIndex i = 0; i < n; ++i )
        {
            // Get the next parameter name and its optional transform.
            const auto name =
                gate->getAdditionalClusteringParameterName( i );
            const auto transform =
                gate->getAdditionalClusteringParameterTransform( i );

            // Output. A NULL transform is output as "null". A comma is
            // included on all output except the last one.
            json += indent + "  {\n";
            json += indent + "    \"name\": \"" + name + "\",\n";
            if ( transform == nullptr )
                json += indent + "    \"transform\": null\n";
            else
            {
                json += indent + "    \"transform\": \n";
                this->saveTransform( json, transform, indentT, true );
            }

            if ( i == (n-1) )
                json += indent + "  }\n";
            else
                json += indent + "  },\n";
        }

        json += indent + "],\n";
    }

    /**
     * Saves the end of a gate.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] gate
     *   The gate.
     * @param[in] indent
     *   The line indent.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void saveGateEnd(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Gate> gate,
        const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the gate.
        const Gates::GateIndex numberOfChildren =
            gate->getNumberOfChildren( );

        const std::string indentBraces   = indent + "  ";
        const std::string indentChildren = indentBraces + "  ";


        //
        // Children.
        // ---------
        // Recurse to output each of the gate's children, nested.
        // If there are no children, add an empty list.
        if ( numberOfChildren == 0 )
            json += indentBraces + "\"gates\": [ ]\n";
        else
        {
            json += indentBraces + "\"gates\": [\n";
            for ( Gates::GateIndex i = 0; i < numberOfChildren; ++i )
            {
                json += indentChildren + "{\n";
                this->saveGate(
                    json,
                    gate->getChild( i ),
                    indentChildren );
                if (i == (numberOfChildren-1) )
                    json += indentChildren + "}\n";
                else
                    json += indentChildren + "},\n";
            }
            json += indentBraces + "]\n";
        }
    }

    /**
     * Saves a rectangle gate.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] gate
     *   The gate.
     * @param[in] indent
     *   The line indent.
     *
     * @see ::getFileLog()
     */
    void saveRectangleGate(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Gate> gate,
        const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the gate.
        const auto g = (FlowGate::Gates::RectangleGate*)(gate.get());
        const Gates::DimensionIndex numberOfDimensions =
            gate->getNumberOfDimensions( );


        //
        // Min/max.
        // --------
        // Output lists of gate minimums and maximums, with one value each
        // per dimension.
        //
        // Minimums.
        json += indent + "\"minimumByDimension\": [ ";
        for ( Gates::DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
            json += std::to_string( g->getDimensionMinimum( i ) );
            if ( i == (numberOfDimensions-1) )
                json += " ],\n";
            else
                json += ", ";
        }

        // Maximums.
        json += indent + "\"maximumByDimension\": [ ";
        for ( Gates::DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
            json += std::to_string( g->getDimensionMaximum( i ) );
            if ( i == (numberOfDimensions-1) )
                json += " ],\n";
            else
                json += ", ";
        }
    }

    /**
     * Saves a polygon gate.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] gate
     *   The gate.
     * @param[in] indent
     *   The line indent.
     *
     * @see ::getFileLog()
     */
    void savePolygonGate(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Gate> gate,
        const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the gate.
        const auto g = (FlowGate::Gates::PolygonGate*)(gate.get());
        const Gates::VertexIndex nVertices = g->getNumberOfVertices( );


        //
        // Vertices.
        // ---------
        // Output a list of (x,y) pairs.
        json += indent + "\"vertices\": [\n";
        for ( Gates::VertexIndex i = 0; i < nVertices; ++i )
        {
            const auto x = g->getVertexX( i );
            const auto y = g->getVertexY( i );

            json += indent + "  [ " +
                std::to_string( x ) + ", " +
                std::to_string( y ) + " ]";
            if ( i == (nVertices - 1) )
                json += "\n";
            else
                json += ",\n";
        }
        json += indent + "],\n";
    }

    /**
     * Saves a transform.
     *
     * @param[in,out] json
     *   The JSON string to which to append the gate.
     * @param[in] transform
     *   The transform.
     * @param[in] indent
     *   The line indent.
     * @param[in] lastInList
     *   Indicate if the gate is the last in a list.
     *
     * @see ::getFileLog()
     */
    void saveTransform(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Transform> transform,
        const std::string& indent,
        const bool lastInList )
        const noexcept
    {
        //
        // Handle NULL transform.
        // ----------------------
        // Some dimensions have no transform. Output an empty object.
        if ( transform == nullptr )
        {
            if ( lastInList == true )
                json += indent + "null\n";
            else
                json += indent + "null,\n";
            return;
        }


        //
        // Setup.
        // ------
        // Get information about the transform.
        const std::string name              = transform->getName( );
        const std::string description       = transform->getDescription( );
        const std::string transformTypeName = transform->getTransformTypeName( );

        const std::string indent2 = indent + "  ";


        // Start the transform object.
        json += indent + "{\n";


        //
        // Header.
        // -------
        // The transform header includes the transform's name, description,
        // and type.  The name and description are omitted if they are empty.
        json += indent2 + "\"transformType\": \"" + transformTypeName + "\",\n";
        if ( name.empty( ) == false )
            json += indent2 + "\"name\": \"" + name + "\",\n";
        if ( description.empty( ) == false )
            json += indent2 + "\"description\": \"" + description + "\",\n";


        //
        // Transform-specific equation variable values.
        // --------------------------------------------
        // Every transform has variables specific to the transform.
        switch ( transform->getTransformType( ) )
        {
        case FlowGate::Gates::PARAMETERIZED_LINEAR_TRANSFORM:
            {
                const auto t = std::dynamic_pointer_cast<const FlowGate::Gates::ParameterizedLinearTransform>( transform );
                json += indent2 + "\"t\": " + std::to_string( t->getT( ) ) + ",\n";
                json += indent2 + "\"a\": " + std::to_string( t->getA( ) ) + "\n";
            }
            break;

        case FlowGate::Gates::PARAMETERIZED_LOGARITHMIC_TRANSFORM:
            {
                const auto t = std::dynamic_pointer_cast<const FlowGate::Gates::ParameterizedLogarithmicTransform>( transform );
                json += indent2 + "\"t\": " + std::to_string( t->getT( ) ) + ",\n";
                json += indent2 + "\"m\": " + std::to_string( t->getM( ) ) + "\n";
            }
            break;

        case FlowGate::Gates::PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM:
            {
                const auto t = std::dynamic_pointer_cast<const FlowGate::Gates::ParameterizedInverseHyperbolicSineTransform>( transform );
                json += indent2 + "\"t\": " + std::to_string( t->getT( ) ) + ",\n";
                json += indent2 + "\"a\": " + std::to_string( t->getA( ) ) + ",\n";
                json += indent2 + "\"m\": " + std::to_string( t->getM( ) ) + "\n";
            }
            break;

        case FlowGate::Gates::LOGICLE_TRANSFORM:
            {
                const auto t = std::dynamic_pointer_cast<const FlowGate::Gates::LogicleTransform>( transform );
                json += indent2 + "\"t\": " + std::to_string( t->getT( ) ) + ",\n";
                json += indent2 + "\"a\": " + std::to_string( t->getA( ) ) + ",\n";
                json += indent2 + "\"m\": " + std::to_string( t->getM( ) ) + ",\n";
                json += indent2 + "\"w\": " + std::to_string( t->getW( ) ) + "\n";
            }
            break;

        case FlowGate::Gates::HYPERLOG_TRANSFORM:
            {
                const auto t = std::dynamic_pointer_cast<const FlowGate::Gates::HyperlogTransform>( transform );
                json += indent2 + "\"t\": " + std::to_string( t->getT( ) ) + ",\n";
                json += indent2 + "\"a\": " + std::to_string( t->getA( ) ) + ",\n";
                json += indent2 + "\"m\": " + std::to_string( t->getM( ) ) + ",\n";
                json += indent2 + "\"w\": " + std::to_string( t->getW( ) ) + "\n";
            }
            break;

        case FlowGate::Gates::CUSTOM_TRANSFORM:
            // Unknown variables.
            break;
        }


        // End the transform object.
        if ( lastInList == true )
            json += indent + "}\n";
        else
            json += indent + "},\n";
    }
    // @}
};

} // End File namespace
} // End FlowGate namespace

