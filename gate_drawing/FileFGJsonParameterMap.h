/**
 * @file
 * Loads parameter map information in the custom JSON text file format.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "GateTrees.h"
 * @li FlowGate "EventTable.h"
 * @li FlowGate "ParameterMap.h"
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
#include "ParameterMap.h" // Parameter map storage





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
#define FLOWGATE_FILEFGJSONPARAMETERMAP_USE_FLOCKFILE
#endif





namespace FlowGate {
namespace File {





/**
 * Loads parameter map information in the custom JSON text file format.
 *
 * The FlowGate project has defined a "FlowGate JSON Parameter Map" file format
 * for the storage of parameter map information. Such a map provides a
 * lookup table that maps the short parameter names found in flow cytometry
 * event files (e.g. FCS) to longer parameter names and default values used
 * by user interfaces.
 *
 * This file format has the following well-known revisions:
 * @li 1.0 from 2020.
 *
 * This file format uses the JSON file syntax originally intended for the
 * exchange of objects and arrays between Javascript apps. JSON has since
 * become a common exchange format used on the web between browser-basd
 * Javascript and the web server. JSON is easier to handle than XML and much
 * less verbose.
 *
 *
 * <B>File format</B><BR>
 * Each JSON parameter map file contains:
 *
 * @li A list of parameter entries that each contain a unique non-empty
 * short name, an optional longer name, an optional description, a default
 * transform, and a visibility flag.
 *
 *
 * <B>Loading JSON parameter map files</B><BR>
 * A JSON parameter map file may be loaded by providing a file path to the
 * constructor.
 * @code
 * auto data = new FileFGJsonParameterMap( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFGJsonParameterMap( );
 * file->load( filepath );
 * #endcode
 *
 * The same JSON parameter map file object may be used repeatedly to load
 * multiple files.  Each time a file is loaded, the prior content of the
 * object is deleted automatically:
 * @code
 * auto file = new FileFGJsonParameterMap( );
 * file->load( filepath1 );
 * ...
 * file->load( filepath2 );
 * ...
 * file->load( filepath3 );
 * ...
 * @endcode
 *
 * If a problem is encountered while loading a JSON parameter map file,
 * an exception is thrown with a human-readable message indicating the
 * problem. All such problems are fatal and will abort loading the file.
 * @code
 * try {
 *   file->load( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 *
 * <B>Saving JSON parameter map files</B><BR>
 * The data in a ParameterMap object may be written to a new JSON parameter
 * map file by calling the save() method:
 * @code
 * try {
 *   file->save( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 * If a problem is encountered while saving a JSON parameter map file,
 * an exception is thrown with a human-readable message indicating the
 * problem, and the file is deleted if it has already been started.
 * Problems with a save include the inability to create or write to the
 * indicated file.
 *
 *
 * <B>Getting and setting parameter maps</B><BR>
 * JSON parameter map data is composed of lookup table of parameter attributes
 * keyed by a unique short parameter name. The order in the file is not
 * defined. Instead the ParameterMap object maintains parameters within a
 * lookup table after the parameters have been read in.
 *
 * The ParameterMap object may be queried:
 * @code
 * // Get the parameter map.
 * auto map = file->getParameterMap( );
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
class FileFGJsonParameterMap final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate JSON Parameter Map files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "FlowGate JSON Parameter Map";

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
    inline static const std::string FORMAT_VERSION = "FlowGateJsonParameterMap_1.0";



private:
    // Configuration ---------------------------------------------------
    /**
     * The default verbosity prefix.
     */
    inline static const std::string DEFAULT_VERBOSE_PREFIX =
        "FileFGJsonParameterMap";



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
        "Malformed parameter map file.\n";

    /**
     * The error message 1st line for an unsupported file.
     */
    inline static const std::string ERROR_UNSUPPORTED =
        "Unsupported parameter map file.\n";

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



    // Parameter map ---------------------------------------------------
    /**
     * The current parameter map.
     *
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    std::shared_ptr<FlowGate::Events::ParameterMap> parameterMap;



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
        extensions.push_back( "map" );
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
     * Constructs a new object with no parameter map.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap( )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
    }



    // Copy constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with a copy of the parameter map
     * in the given FileFGJsonParameterMap object.
     *
     * @param[in] file
     *   The FileFGJsonParameterMap object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap( const FileFGJsonParameterMap*const file )
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
        if ( file->parameterMap != nullptr )
            this->copy( file->parameterMap );
    }

    /**
     * Constructs a new object initialized with a copy of the parameter map
     * in the given FileFGJsonParameterMap object.
     *
     * @param[in] file
     *   The FileFGJsonParameterMap object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap( const FileFGJsonParameterMap& file )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        if ( file.parameterMap != nullptr )
            this->copy( file.parameterMap );
    }



    // Parameter map constructors --------------------------------------
    /**
     * Constructs a new object using the given parameter map.
     *
     * The new object shares the given parameter map. External changes to the
     * parameter map will affect this object as well.
     *
     * @param[in] parameterMap
     *   The parameter map to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter map is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap(
        std::shared_ptr<FlowGate::Events::ParameterMap> parameterMap )
    {
        // Validate.
        if ( parameterMap == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Use.
        this->setParameterMap( parameterMap );
    }

    /**
     * Constructs a new object initialized with a copy of the given
     * parameter map.
     *
     * @param[in] parameterMap
     *   The parameter map to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the parameter map is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap(
        const FlowGate::Events::ParameterMap*const parameterMap )
    {
        // Validate.
        if ( parameterMap == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( parameterMap );
    }

    /**
     * Constructs a new object initialized with a copy of the given
     * parameter map.
     *
     * @param[in] parameterMap
     *   The parameter map to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap( const FlowGate::Events::ParameterMap& parameterMap )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( parameterMap );
    }



    // Load constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with data loaded from a
     * JSON parameter map file.
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
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     */
    FileFGJsonParameterMap( const std::string& path )
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
    virtual ~FileFGJsonParameterMap( )
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
        this->parameterMap.reset( new FlowGate::Events::ParameterMap( ) );
    }



    // Copy parameter map -------------------------------------------------
    /**
     * Copies the given parameter map.
     *
     * The file object is reset.
     *
     * The given parameter map is copied into a new parameter map stored
     * within the object. The new map is used by further methods,
     * such as to save the parameter map to a file.
     *
     * @param[in] parameterMap
     *   A list of parameter map to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the parameter map is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Events::ParameterMap*const parameterMap )
    {
        // Validate.
        if ( parameterMap == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map." );

        this->reset( );

        // Copy.
        this->copy( *parameterMap );
    }

    /**
     * Copies the given parameter map.
     *
     * The file object is reset.
     *
     * The given parameter map is copied into a new parameter map stored
     * within the object. The new map is used by further methods,
     * such as to save the parameter map to a file.
     *
     * @param[in] parameterMap
     *   A list of parameter map to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the parameter map is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     * @see ::reset()
     */
    inline void copy(
        const std::shared_ptr<FlowGate::Events::ParameterMap>& parameterMap )
    {
        // Validate.
        if ( parameterMap == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map." );

        this->reset( );

        // Copy.
        this->copy( *(parameterMap.get( )) );
    }

    /**
     * Copies the given parameter map.
     *
     * The file object is reset.
     *
     * The given parameter map is copied into a new parameter map stored
     * within the object. The new map is used by further methods,
     * such as to save the parameter map to a file.
     *
     * @param[in] parameterMap
     *   A list of parameter map to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the parameter map is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getParameterMap()
     * @see ::setParameterMap()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Events::ParameterMap& parameterMap )
        noexcept
    {
        this->reset( );

        this->parameterMap.reset(
            new FlowGate::Events::ParameterMap( parameterMap ) );
    }
    // @}



//----------------------------------------------------------------------
// Parameter map.
//----------------------------------------------------------------------
public:
    /**
     * @name Parameter map
     */
    // @{
    /**
     * Gets the parameter map in use.
     *
     * If a file has not been loaded and a parameter map has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the parameter map.
     *
     * @see ::setParameterMap()
     */
    inline std::shared_ptr<const FlowGate::Events::ParameterMap> getParameterMap( )
        const
    {
        return this->parameterMap;
    }

    /**
     * Gets the parameter map in use.
     *
     * If a file has not been loaded and a parameter map has not been
     * initialized, then a NULL pointer is returned.
     *
     * @return
     *   Returns a shared pointer for the parameter map.
     *
     * @see ::setParameterMap()
     */
    inline std::shared_ptr<FlowGate::Events::ParameterMap> getParameterMap( )
    {
        return this->parameterMap;
    }

    /**
     * Resets the object and sets the parameter map to use.
     *
     * The prior parameter map, if any, are released. The new parameter map
     * will be used for all further operations, such as saving to a file.
     *
     * @param[in] parameterMap
     *   The parameter map to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter map are a nullptr.
     *
     * @see ::reset()
     * @see ::copy()
     */
    inline void setParameterMap(
        std::shared_ptr<FlowGate::Events::ParameterMap> parameterMap )
    {
        // Validate.
        if ( parameterMap == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL parameter map." );

        // Initialize.
        this->reset( );
        this->parameterMap = parameterMap;
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
                "Loading JSON parameter map file \"" << filePath << "\".\n";

        this->reset( );

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
                "The JSON file's root node is not an object. The file is therefore not a valid JSON parameter map file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON parameter map." ) +
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
                        "The JSON file's \"version\" key is present, but the value is not a string. The file is therefore not a valid JSON parameter map file." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file does not contain JSON parameter map." ) +
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
                        "The JSON file's \"version\" key is present, indicating this may be a JSON parameter map file. But the version number in the key's value is not recognized. The version may be for a newer version of file format and this software is out of date." );
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
                "The JSON file does not include a \"version\" key giving the file format version number. The file is therefore not a valid JSON parameter map file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON parameter map." ) +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << versionNumber << "\"\n";

        //
        // Load parameters.
        // ----------------
        // At this point, the file text is valid JSON and there is a valid
        // version number. Start loading parameter map.
        try
        {
            loadParameters( rootValue );
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
     * The character text array is parsed as text in the JSON parameter map
     * format.
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

        this->parameterMap->setFileName( "" );


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
                "The JSON file's root node is not an object. The file is therefore not a valid JSON parameter map file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON parameter map." ) +
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
                        "The JSON file's \"version\" key is present, but the value is not a string. The file is therefore not a valid JSON parameter map file." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file does not contain JSON parameter map." ) +
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
                        "The JSON file's \"version\" key is present, indicating this may be a JSON parameter map file. But the version number in the key's value is not recognized. The version may be for a newer version of file format and this software is out of date." );
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
                "The JSON file does not include a \"version\" key giving the file format version number. The file is therefore not a valid JSON parameter map file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain JSON parameter map." ) +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << versionNumber << "\"\n";

        //
        // Load parameters.
        // ----------------
        // At this point, the file text is valid JSON and there is a valid
        // version number. Start loading parameter map.
        try
        {
            loadParameters( rootValue );
        }
        catch ( const std::exception& e )
        {
            throw;
        }
    }
    // @}



//----------------------------------------------------------------------
// Load parameters from file or text.
//----------------------------------------------------------------------
private:
    /**
     * @name Load parameters from file or text
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
     * Traverses the JSON tree to load parameters.
     *
     * The root node of the JSON tree contains the file format version
     * and related metadata. A "parameters" array then has one parameter
     * object for each parameter in the map.
     *
     * @param[in] rootValue
     *   The JSON root node value.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    void loadParameters( const JsonValue& rootValue )
    {
        if ( this->isVerbose( ) == true )
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading parameters:\n";


        //
        // Loop through top level items.
        // -----------------------------
        // Look for well-known keys and, if properly formed, copy them
        // into the parameter map. The most important is the "parameters" key
        // whose value is an array of parameters.
        this->parameterMap.reset( new FlowGate::Events::ParameterMap( ) );

        for ( const auto child: rootValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "creatorSoftwareName" ) == 0 )
            {
                this->parameterMap->setCreatorSoftwareName(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "creatorSoftwareName",
                    this->parameterMap->getCreatorSoftwareName( ) );
                continue;
            }
            if ( std::strcmp( key, "name" ) == 0 )
            {
                this->parameterMap->setName(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "name",
                    this->parameterMap->getName( ) );
                continue;
            }
            if ( std::strcmp( key, "description" ) == 0 )
            {
                this->parameterMap->setDescription(
                    this->getAndValidateString( key, child->value ) );
                this->fileAttributes.emplace(
                    "description",
                    this->parameterMap->getDescription( ) );
                continue;
            }
            if ( std::strcmp( key, "parameters" ) == 0 )
            {
                if ( child->value.getTag( ) != JSON_ARRAY )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"parameters\" key, but the value is not an array. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }

                // Each child in the array is a parameter object for a
                // parameter.
                for ( const auto p: child->value )
                {
                    if ( p->value.getTag( ) != JSON_OBJECT )
                    {
                        this->appendFileLog(
                            "error",
                            "The JSON file includes a \"parameters\" key for a parameter list, but the value is not a parameter object. The file is malformed." );
                        throw std::runtime_error(
                            ERROR_MALFORMED +
                            ERROR_CANNOTLOAD );
                    }

                    this->loadParameter( p->value );
                }
                continue;
            }

            // Ignore any other keys.
        }


        const auto n = this->parameterMap->getNumberOfParameters( );

        if ( n == 0 )
        {
            this->appendFileLog(
                "warning",
                "The JSON parameter map file does not have any recognized parameters." );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of parameters:" <<
                n << "\n";
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
     *   Returns the transform.
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

                // Convert to a numeric transform type. If the type is
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
     * Parses a JSON node value as a parameter.
     *
     * The given JSON node is traversed to collect parameter attributes.
     *
     * @param[in] nodeValue
     *   The JSON node value from which to get the parameter.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the JSON tree.
     */
    void loadParameter( const JsonValue& nodeValue )
    {
        //
        // Find the parameter name.
        // ------------------------
        // We have to have the parameter name before we can add an
        // entry to the parameter map. While the name could be the
        // first child of the node, it might not be. And if it isn't,
        // we have no place to put the rest of the values. So, go
        // through the node's children first to find the name so we
        // can create the map entry.
        std::string name;
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "name" ) == 0 )
            {
                name = this->getAndValidateString( key, child->value );
                break;
            }
        }

        // Add the parameter to the map.
        if ( this->parameterMap->isParameter( name ) == false )
            this->parameterMap->addParameter( name );


        //
        // Collect remaining attributes.
        // -----------------------------
        // Loop over the parameter's keys looking for other parameter
        // attributes.
        for ( const auto child: nodeValue )
        {
            auto& key = child->key;
            if ( std::strcmp( key, "name" ) == 0 )
            {
                // The parameter name has already been handled above.
                continue;
            }
            if ( std::strcmp( key, "description" ) == 0 )
            {
                this->parameterMap->setParameterDescription( name,
                    this->getAndValidateString( key, child->value ) );
                continue;
            }
            if ( std::strcmp( key, "longName" ) == 0 )
            {
                this->parameterMap->setParameterLongName( name,
                    this->getAndValidateString( key, child->value ) );
                continue;
            }
            if ( std::strcmp( key, "visibleForGating" ) == 0 )
            {
                const auto n = this->getAndValidateNumber( key, child->value );
                if ( n < 0.0 )
                {
                    this->appendFileLog(
                        "error",
                        "The JSON file includes a \"visibleForGating\" key for a parameter, but the value is not zero or positive. The file is malformed." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                this->parameterMap->setParameterVisibleForGating( name, (n != 0) );
                continue;
            }
            if ( std::strcmp( key, "defaultTransform" ) == 0 )
            {
                // The default transform should be a valid transform object
                // OR one of several ways to indicate a NULL transform:
                // - NULL.
                // - "null".
                // - 0.
                // - empty object.
                // - empty array.
                bool error = false;
                char* s = NULL;
                switch ( child->value.getTag() )
                {
                    default:
                    case JSON_NULL:
                        // Null transform.
                        this->parameterMap->setParameterDefaultTransform(
                            name,
                            nullptr );
                        break;

                    case JSON_STRING:
                        // Possible null transform.
                        s = child->value.toString( );
                        if ( *s == '\0' ||
                            std::strcmp( s, "null" ) == 0 ||
                            std::strcmp( s, "NULL" ) == 0 ||
                            std::strcmp( s, "\"null\"" ) == 0 )
                        {
                            // Null transform.
                            this->parameterMap->setParameterDefaultTransform(
                                name,
                                nullptr );
                        }
                        else
                            error = true;
                        break;

                    case JSON_NUMBER:
                        // Possible zero transform.
                        if ( child->value.toNumber( ) == 0 )
                        {
                            // Null transform.
                            this->parameterMap->setParameterDefaultTransform(
                                name,
                                nullptr );
                        }
                        else
                            error = true;
                        break;

                    case JSON_ARRAY:
                    case JSON_OBJECT:
                        auto transform = this->loadTransform( child->value );
                        this->parameterMap->setParameterDefaultTransform(
                            name,
                            transform );
                        break;
                }

                if ( error == true )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The JSON file includes a \"" ) +
                        std::string( key ) +
                        std::string( "\" key for parameter default transforms, but the value is not a transform object. The file is malformed." ) );

                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_CANNOTLOAD );
                }
                continue;
            }

            // Ignore any other keys.
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
        if ( this->parameterMap == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "Invalid NULL parameter map." ) );


        //
        // Initialize.
        // -----------
        // Save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Saving JSON parameter map file \"" << filePath << "\".\n";

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
        this->saveParameters( json, "" );

        try
        {
#ifdef FLOWGATE_FILEFGJSONPARAMETERMAP_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            if ( std::fwrite( json.c_str( ), 1, json.length( ), fp ) < json.length( ) )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file parameters: " ) +
                    std::string( std::strerror( errno ) ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

#ifdef FLOWGATE_FILEFGJSONPARAMETERMAP_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFGJSONPARAMETERMAP_USE_FLOCKFILE
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
     * A string is returned with data from the current parameter map.
     *
     * @param[in] indent
     *   (optional, default = "  ") The per-line indent for the returned text.
     *
     * @see ::getFileLog()
     * @see ::save()
     */
    std::string saveText( const std::string indent = "" )
        const noexcept
    {
        std::string json;
        this->saveParameters( json, indent );
        return json;
    }
    // @}



//----------------------------------------------------------------------
// Save parameters.
//----------------------------------------------------------------------
private:
    /**
     * @name Save parameters.
     */
    // @{
    /**
     * Saves parameters.
     *
     * @param[in,out] json
     *   The JSON string to which to append the parameters.
     * @param[in] indent
     *   The line indent.
     *
     * @see ::getFileLog()
     */
    void saveParameters( std::string& json, const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the parameter map.
        const std::string name =
            this->parameterMap->getName( );
        const std::string description =
            this->parameterMap->getDescription( );
        const std::string software =
            this->parameterMap->getCreatorSoftwareName( );
        const size_t numberOfParameters =
            this->parameterMap->getNumberOfParameters( );

        const std::string indentFields = indent + "  ";
        const std::string indentParameter = indent + "    ";

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " <<
                "  Saving parameters:\n";
            std::cerr << this->verbosePrefix << ": " << "    " <<
                std::setw( 30 ) << std::left <<
                "Number of parameter:" <<
                numberOfParameters << "\n";
        }

        // Start the file's object.
        json += indent + "{\n";

        //
        // Header.
        // -------
        // The header contains optional information about the parameter map.
        //
        // Output looks like:
        //   "version": "FORMAT_VERSION",
        //   "name": "NAME",
        //   "description": "DESCRIPTION",
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
        if ( software.empty( ) == false )
            json += indentFields + "\"creatorSoftware\": \"" + software + "\",\n";

        //
        // Parameters.
        // -----------
        // Iterate over the parameters and write them out.
        //
        // Output looks like:
        //   "parameters": [
        //     {
        //       "name": "FCS-A",
        //       ...
        //     }
        //     ...
        //   ]
        //
        json += indentFields + "\"parameters\": [\n";
        const auto names = this->parameterMap->getParameterNames( );
        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            json += indentParameter + "{\n";

            this->saveParameter(
                json,
                names[i],
                indentParameter);

            if ( i == (numberOfParameters-1) )
                json += indentParameter + "}\n";
            else
                json += indentParameter + "},\n";
        }
        json += indentFields + "]\n";
        json += indent + "}\n";
    }


    /**
     * Saves a parameter.
     *
     * @param[in,out] json
     *   The JSON string to which to append the parameter.
     * @param[in] shortName
     *   The short name of a parameter.
     * @param[in] indent
     *   The line indent.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see ::getFileLog()
     */
    void saveParameter(
        std::string& json,
        const std::string& shortName,
        const std::string& indent )
        const noexcept
    {
        //
        // Setup.
        // ------
        // Get information about the parameter.
        const std::string longName =
            this->parameterMap->findParameterLongName( shortName );
        const std::string description =
            this->parameterMap->findParameterDescription( shortName );
        const int visibleForGating =
            (this->parameterMap->findParameterVisibleForGating( shortName ) == true) ? 1 : 0;
        const auto defaultTransform =
            this->parameterMap->findParameterDefaultTransform( shortName );

        const std::string indent2 = indent + "  ";


        //
        // Attributes.
        // -----------
        // Output looks like:
        //   "name": "NAME",
        //   "longName": "LONGNAME",
        //   "description": "DESCRIPTION",
        //   "visibleForGating": FLAG,
        //   "defaultTransform": TRANSFORM,
        //   ...
        //
        // Only the short name is required. The others are omitted if
        // they are empty.
        //
        json += indent2 + "\"name\": \"" + shortName + "\",\n";

        if ( longName.empty( ) == false )
            json += indent2 + "\"longName\": \"" + longName + "\",\n";

        if ( description.empty( ) == false )
            json += indent2 + "\"description\": \"" + description + "\",\n";

        json += indent2 + "\"visibleForGating\": " + std::to_string( visibleForGating ) + ",\n";

        json += indent2 + "\"defaultTransform\": ";
        this->saveTransform(
            json,
            defaultTransform,
            indent2 );
    }

    /**
     * Saves a transform.
     *
     * @param[in,out] json
     *   The JSON string to which to append the parameters.
     * @param[in] transform
     *   The transform.
     * @param[in] indent
     *   The line indent.
     *
     * @see ::getFileLog()
     */
    void saveTransform(
        std::string& json,
        std::shared_ptr<const FlowGate::Gates::Transform> transform,
        const std::string& indent )
        const noexcept
    {
        //
        // Handle NULL transform.
        // ----------------------
        // Some dimensions have no transform. Output an empty object.
        if ( transform == nullptr )
        {
            json += indent + "null\n";
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
        json += indent + "}\n";
    }
    // @}
};

} // End File namespace
} // End FlowGate namespace

