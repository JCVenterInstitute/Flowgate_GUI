/**
 * @file
 * Loads and saves flow cytometry gate information in the ISAC standard
 * Gating-ML XML file format.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "GateTrees.h"
 * @li RapidXML for XML parsing
 *
 * @todo Implement loading ellipsoid gates.
 * @todo Implement loading quadrant gates.
 * @todo Implement loading boolean gates.
 * @todo Implement loading compensation transforms.
 *
 * @todo Implement saving compensation transforms.
 * @todo Implement saving ellipsoid gates.
 * @todo Implement saving quadrant gates.
 * @todo Implement saving boolean gates.
 *
 * @todo Re-implement saveText() to avoid saving to a temp file and reading
 * that file back in.
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

// RapidXML library.
#include "rapidxml.hpp"

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
#define FLOWGATE_FILEGATINGML_USE_FLOCKFILE
#endif





namespace FlowGate {
namespace File {





/**
 * Loads and saves flow cytometry gate information in the ISAC standard
 * Gating-ML XML file format.
 *
 * The International Society for Advancement of Cytometry (ISAC) has
 * defined the "Gating-ML" file format for the storage of gate information
 * to classify event data acquired from flow cytometry equipment and software.
 * This file format has the following well-known revisions:
 * @li 1.5 from 2008.
 * @li 2.0 from 2013.
 *
 * There is no 1.0 release of the Gating-ML specification.
 *
 * This software supports version 2.0 of the specification. Where features
 * are in common between versions 1.5 and 2.0, those features are supported.
 * However, version 2.0 deleted support for a number of gate and transform
 * types in the 1.5 specification, and these are not supported.
 *
 * Gating-ML uses the XML file syntax and has a well-defined schema.
 * All files should conform to this schema, though this software is
 * intentionally lax in its enforcement of the schema. Lax enforcement
 * enables the software to adapt to minor format errors introduced by other
 * software.
 *
 *
 * <B>File format</B><BR>
 * Each Gating-ML file contains:
 *
 * @li An optional list of compensation methods.
 *
 * @li An optional list of transform methods.
 *
 * @li A list of gates, optionally connected into one or more gate trees.
 *
 * Gating-ML files may contain additional vendor-specific or software-specific
 * data interspersed among the above items. For instance, additional data
 * may include the name and description of a gate or transform.
 *
 *
 * <B>Loading Gating-ML files</B><BR>
 * A Gating-ML file may be loaded by providing a file path to the constructor.
 * @code
 * auto data = new FileGatingML( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileGatingML( );
 * file->load( filepath );
 * #endcode
 *
 * The same Gating-ML data object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the Gating-ML data object
 * is deleted automatically:
 * @code
 * auto file = new FileGatingML( );
 * file->load( filepath1 );
 * ...
 * file->load( filepath2 );
 * ...
 * file->load( filepath3 );
 * ...
 * @endcode
 *
 * If a problem is encountered while loading a Gating-ML file, an exception
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
 * <B>Saving Gating-ML files</B><BR>
 * The data in a GateTrees object may be written to a new Gating-ML file by
 * calling the save() method:
 * @code
 * try {
 *   file->save( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 * If a problem is encountered while saving a Gating-ML file, an exception
 * is thrown with a human-readable message indicating the problem, and
 * the file is deleted if it has already been started. Problems with
 * a save include the inability to create or write to the indicated file.
 *
 *
 * <B>Getting and setting gate trees</B><BR>
 * Gating-ML data is primary composed of one or more gate trees. Each
 * gate tree has a root gate that has zero or more children gates, which
 * themselves may have children, and so on.
 *
 * Gate trees are stored in a GateTrees object that manages a simple list of
 * gate tree roots. The order of gates in the list has no significance and
 * typically matches the order in which gate tree roots are found in a
 * loaded Gating-ML file.
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
 * <B>Limitations of this software</B><BR>
 * This software supports the primary features of Gating-ML 1.5 and 2.0
 * files. It omits most support for deleted features of the Gating-ML 1.5
 * specification, and has only limited support for some of the more obscure
 * features of the Gating-ML 2.0 specification.
 *
 * Gating-ML 1.5 gate features deleted in Gating-ML 2.0 that are not
 * supported:
 *
 * @li Boolean gates that use gate expressions.
 * @li Decision tree gates ("gating:DecisionTreeGate").
 * @li Polytope gates ("gating:PolytopeGate").
 *
 * Gating-ML 1.5 transform features deleted in Gating-ML 2.0 that are
 * not supported:
 *
 * @li EH transforms ("transforms:EH").
 * @li Exponential transforms ("transforms:exponential").
 * @li Hyperbolic sine transforms ("transforms:sinh").
 * @li Hyperlog transforms ("transforms:hyperlog").
 * @li Inverse hyperbolic sine transforms ("transforms:asinh").
 * @li Inverse split scale transforms ("transforms:inverse-split-scale").
 * @li Natural log transforms ("transforms:ln").
 * @li Polynomial of degree one transforms ("transforms:dg1polynomial").
 * @li Quadratic transforms ("transforms:quadratic").
 * @li Ratio transforms ("transforms:ratio").
 * @li Split scale transforms ("transforms:split-scale").
 * @li Square root transforms ("transforms:sqrt").
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
 * @li XML parsing uses Rapid XML. This parser does in-situ parsing that
 *     allocates XML nodes that internally use pointers into the source
 *     character buffer for their values. This avoids additional memory
 *     allocations and copying for those values.
 * @endinternal
 */
class FileGatingML final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "ISAC Gating-ML Gates files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "ISAC Gating-ML";

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
        "FileGatingML";



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



    // Gate trees ------------------------------------------------------
    /**
     * The current list of gate trees.
     *
     * @see getGateTrees()
     * @see setGateTrees()
     */
    std::shared_ptr<FlowGate::Gates::GateTrees> gateTrees;

    /**
     * A mapping from gate IDs to the corresponding gates.
     *
     * During file loading, this map is used to map IDs to gates and help
     * build the pointer-based linkage of gates into gate trees. Once gate
     * trees are built, this map is no longer necessary and it is cleared.
     *
     * During file saving, this map is built from the gate trees to be saved.
     * During the process, unique IDs are assigned. The gate list is then
     * saved to a file, after which this map is no longer necessary and it
     * is cleared.
     */
    std::map<std::string,std::shared_ptr<FlowGate::Gates::Gate>> idsToGates;



    // Transforms ------------------------------------------------------
    /**
     * A mapping from the file's transform IDs to the corresponding transforms.
     *
     * During file loading, this map is used to map IDs to transforms and
     * help build the pointer-based linkage of gate dimensions to their
     * transforms. Once this connection is made, this map is no longer
     * necessary and it is cleared.
     *
     * During file saving, this map is built from the gate trees to be saved.
     * Each transform referenced by a gate in the gate tree is saved to this
     * list and assigned a unique ID. The transform list is then saved to a
     * file, after which this map is no longer necessary and it is cleared.
     */
    std::map<std::string,std::shared_ptr<FlowGate::Gates::Transform>> idsToTransforms;

    /**
     * A mapping from transforms to their IDs.
     *
     * During file saving, this map is built from the gate trees to be saved.
     * During the process, each transform is first assigned a unique ID, then
     * added to the transformsToIds map. The map is used later when writing out
     * gates to map the transform in the gate to the previously-assigned ID.
     *
     * IDs are not saved into the gate objects themselves because (1) they
     * do not support ID attributes, and (2) the data type, structure, and
     * use of IDs is dependent upon the file format used. For Gating-ML,
     * IDs are strings that usually contain a unique numeric ID embedded.
     * Another file format might require numeric indexes counting from zero,
     * or something else.
     */
    std::map<std::shared_ptr<const FlowGate::Gates::Transform>,std::string> transformsToIds;



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
        extensions.push_back( "gml" );
        extensions.push_back( "xml" );
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
    FileGatingML( )
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
     * in the given FileGatingML object.
     *
     * @param[in] file
     *   The FileGatingML object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileGatingML( const FileGatingML*const file )
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
     * in the given FileGatingML object.
     *
     * @param[in] file
     *   The FileGatingML object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getGateTrees()
     * @see ::setGateTrees()
     */
    FileGatingML( const FileGatingML& file )
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
    FileGatingML( std::shared_ptr<FlowGate::Gates::GateTrees>& gateTrees )
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
    FileGatingML( const FlowGate::Gates::GateTrees*const gateTrees )
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
    FileGatingML( const FlowGate::Gates::GateTrees& gateTrees )
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
     * GatingML file.
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
    FileGatingML( const std::string& path )
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
    virtual ~FileGatingML( )
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
        this->idsToGates.clear( );
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
     * @see load()
     * @see save()
     * @see getGateTrees()
     * @see setGateTrees()
     * @see reset()
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
     * within the object. The new trees are used by further methods,
     * such as to save the gate trees to a file.
     *
     * @param[in] gateTrees
     *   A list of gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the gate trees are a nullptr.
     *
     * @see load()
     * @see save()
     * @see getGateTrees()
     * @see setGateTrees()
     * @see reset()
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
     * @see load()
     * @see save()
     * @see getGateTrees()
     * @see setGateTrees()
     * @see reset()
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
     * @see setGateTrees()
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
     * @see setGateTrees()
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
     * @see reset()
     * @see copy()
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
     * @see load()
     * @see save()
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
                "Loading Gating-ML file \"" << filePath << "\".\n";

        this->reset( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", filePath );
        this->gateTrees->setFileName( path );


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
        // The XML parser works from a loaded text buffer, so start by
        // loading the entire file into memory. Use a shared pointer for
        // the buffer so that the buffer is automatically deleted when
        // it falls out of scope, such as via exceptions.
        //
        // Note that this could be replaced with memory mapping the file
        // into memory, but the RapidXML parser is an in-situ parser that
        // needs to modify the buffer in order to remove white space, add
        // null terminators on strings, etc. Memory mapping prevents this
        // and forces RapidXML into a slower parsing mode. So, memory
        // mapping would avoid read time and memory use, but cost a lot in
        // parse time.
        std::shared_ptr<char> buffer;
        try
        {
            // Get the file size.
            size_t fileSize = 0;
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
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "File size:" << fileSize << " bytes\n";
            }

            // Load the entire file into a buffer for XML parsing.
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
        // Parse XML.
        // ----------
        // Allocate a "document" object to contain the parsed document
        // object model (DOM). Use a shared pointer so that the document is
        // automatically deleted when it falls out of scope, such as from
        // an exception.
        //
        // Then pass the buffer to the parser to trigger creation of the DOM.
        // The parser records pointers to pieces of the buffer containing
        // content. This "in-situ" style reduces memory allocation, but it
        // also requires that the buffer be retained for the life of the
        // document.
        std::shared_ptr<rapidxml::xml_document<char>> document;
        document.reset( new rapidxml::xml_document<char>( ) );

        try
        {
            document->parse<0>( buffer.get( ) );
        }
        catch ( const rapidxml::parse_error& e )
        {
            document->clear( );
            this->appendFileLog(
                "error",
                std::string( "RapidXML failed with a parse error: \"" ) +
                std::string( e.what( ) ) +
                std::string( "\" at " ) +
                std::string( e.where<char>( ) ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain valid XML." ) +
                ERROR_CANNOTLOAD );
        }
        catch ( const std::exception& e )
        {
            document->clear( );
            this->appendFileLog(
                "error",
                std::string( "RapidXML failed with an unknown exception: \"" ) +
                e.what( ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain valid XML." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Validate.
        // ---------
        // Confirm that the XML file is a Gating-ML file.
        auto root = document->first_node( "gating:Gating-ML" );
        if ( root == nullptr )
        {
            document->clear( );
            this->appendFileLog(
                "error",
                "A root 'gating:Gating-ML' node was not found. While the file appears to be XML, it is not a valid Gating-ML file." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file does not contain Gating-ML information." ) +
                ERROR_CANNOTLOAD );
        }

        // Look for the version number.
        auto attr = root->first_attribute( "xmlns:gating" );
        std::string fileVersionNumber;
        if ( attr == nullptr )
        {
            // The XML syntax name is missing. This is optional, but it
            // means we can't be sure what version of the file format is
            // in use. Assume 2.0.
            fileVersionNumber = "Gating-ML2.0";
        }
        else
        {
            // The XML syntax file name is fixed and defined in the
            // Gating-ML specification.
            auto value = attr->value( );
            if ( std::strcmp( value,
                "http://www.isac-net.org/std/Gating-ML/v1.5/gating" ) == 0 )
                fileVersionNumber = "Gating-ML1.5";
            else if ( std::strcmp( value,
                "http://www.isac-net.org/std/Gating-ML/v2.0/gating" ) == 0 )
                fileVersionNumber = "Gating-ML2.0";
            else
            {
                fileVersionNumber = value;
                document->clear( );
                this->appendFileLog(
                    "error",
                    std::string( "The file header indicates the file is a Gating-ML file, but the version number is not recognized: \"" ) +
                    fileVersionNumber + "\"." );
                throw std::runtime_error(
                    std::string( "Unsupported version of the Gating-ML file format.\n" ) +
                    std::string( "The file uses a version of the Gating-ML file format that is not supported by this software: \"") +
                    fileVersionNumber +
                    std::string( "\"." ) +
                    ERROR_CANNOTLOAD );
            }
        }

        this->fileAttributes.emplace( "versionNumber", fileVersionNumber );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << fileVersionNumber << "\"\n";


        //
        // Parse.
        // ------
        // Search through the document for supported features and use them
        // to build gate trees and supporting objects.
        try
        {
            loadHeader( root );
            loadTransforms( root );
            // TODO. Implement loading compensations.
            // loadCompensations( root );
            loadGates( root );
        }
        catch ( const std::exception& e )
        {
            document->clear( );
            throw;
        }

        //
        // Clean up.
        // ---------
        // Clear the document to free up its memory use. The character
        // buffer that backs the document is in a shared pointer and will
        // be dropped at the end of this method.
        //
        // Also clear assorted temporary maps used to help build gate
        // trees. At this point the gate trees are built and the maps are
        // no longer needed.
        document->clear( );

        this->idsToGates.clear( );
        this->idsToTransforms.clear( );
    }
    // @}



//----------------------------------------------------------------------
// Load string.
//----------------------------------------------------------------------
public:
    /**
     * @name Load string
     */
    // @{
    /**
     * Loads the indicated string's data.
     *
     * The character text array is parsed as text in the Gating-ML format.
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
     * @see load()
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

        this->gateTrees->setFileName( "" );

        std::shared_ptr<char> buffer;
        buffer.reset( text );


        //
        // Parse XML.
        // ----------
        // Allocate a "document" object to contain the parsed document
        // object model (DOM). Use a shared pointer so that the document is
        // automatically deleted when it falls out of scope, such as from
        // an exception.
        //
        // Then pass the buffer to the parser to trigger creation of the DOM.
        // The parser records pointers to pieces of the buffer containing
        // content. This "in-situ" style reduces memory allocation, but it
        // also requires that the buffer be retained for the life of the
        // document.
        std::shared_ptr<rapidxml::xml_document<char>> document;
        document.reset( new rapidxml::xml_document<char>( ) );

        try
        {
            document->parse<0>( buffer.get( ) );
        }
        catch ( const rapidxml::parse_error& e )
        {
            document->clear( );
            this->appendFileLog(
                "error",
                std::string( "RapidXML failed with a parse error: \"" ) +
                std::string( e.what( ) ) +
                std::string( "\" at " ) +
                std::string( e.where<char>( ) ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The text does not contain valid XML." ) +
                ERROR_CANNOTLOAD );
        }
        catch ( const std::exception& e )
        {
            document->clear( );
            this->appendFileLog(
                "error",
                std::string( "RapidXML failed with an unknown exception: \"" ) +
                e.what( ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The text does not contain valid XML." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Validate.
        // ---------
        // Confirm that the XML text is in the Gating-ML format.
        auto root = document->first_node( "gating:Gating-ML" );
        if ( root == nullptr )
        {
            document->clear( );
            this->appendFileLog(
                "error",
                "A root 'gating:Gating-ML' node was not found. While the text appears to be XML, it is not a valid Gating-ML format." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The text does not contain Gating-ML information." ) +
                ERROR_CANNOTLOAD );
        }

        // Look for the version number.
        auto attr = root->first_attribute( "xmlns:gating" );
        std::string fileVersionNumber;
        if ( attr == nullptr )
        {
            // The XML syntax name is missing. This is optional, but it
            // means we can't be sure what version of the file format is
            // in use. Assume 2.0.
            fileVersionNumber = "Gating-ML2.0";
        }
        else
        {
            // The XML syntax file name is fixed and defined in the
            // Gating-ML specification.
            auto value = attr->value( );
            if ( std::strcmp( value,
                "http://www.isac-net.org/std/Gating-ML/v1.5/gating" ) == 0 )
                fileVersionNumber = "Gating-ML1.5";
            else if ( std::strcmp( value,
                "http://www.isac-net.org/std/Gating-ML/v2.0/gating" ) == 0 )
                fileVersionNumber = "Gating-ML2.0";
            else
            {
                fileVersionNumber = value;
                document->clear( );
                this->appendFileLog(
                    "error",
                    std::string( "The text header indicates the text is in the Gating-ML format, but the version number is not recognized: \"" ) +
                    fileVersionNumber + "\"." );
                throw std::runtime_error(
                    std::string( "Unsupported version of the Gating-ML format.\n" ) +
                    std::string( "The uses a version of the Gating-ML file format that is not supported by this software: \"") +
                    fileVersionNumber +
                    std::string( "\"." ) +
                    ERROR_CANNOTLOAD );
            }
        }

        this->fileAttributes.emplace( "versionNumber", fileVersionNumber );

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << fileVersionNumber << "\"\n";


        //
        // Parse.
        // ------
        // Search through the document for supported features and use them
        // to build gate trees and supporting objects.
        try
        {
            loadHeader( root );
            loadTransforms( root );
            // loadCompensations( root );
            loadGates( root );
        }
        catch ( const std::exception& e )
        {
            document->clear( );
            throw;
        }

        //
        // Clean up.
        // ---------
        // Clear the document to free up its memory use. The character
        // buffer that backs the document is in a shared pointer and will
        // be dropped at the end of this method.
        //
        // Also clear assorted temporary maps used to help build gate
        // trees. At this point the gate trees are built and the maps are
        // no longer needed.
        document->clear( );

        this->idsToGates.clear( );
        this->idsToTransforms.clear( );
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
     * Traverses the XML root to load header information.
     *
     * Header information includes vendor-specific custom information that 
     * may provide:
     * @li The gate trees name.
     * @li The gate trees description.
     * @li The gate trees diagnostic notes.
     * @li The FCS file name used when authoring/testing the gate trees.
     * @li The name of the software used to create the gate trees.
     *
     * @param[in] root
     *   The XML root.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the XML tree.
     */
    void loadHeader( const rapidxml::xml_node<>*const root )
    {
        //
        // Load custom FlowGate information:
        // - <name> with the gate tree's name, if any.
        // - <description> with the gate tree's description, if any.
        // - <notes> with the gate tree's diagnostic notes, if any.
        // - <fcsFileName> with the FCS file name, if any.
        // - <creatorSoftwareName> with the software name, if any.
        //
        // Load custom Cytobank-compatible information:
        // - <experiment_title> with the gate trees name, if any.
        // - <about> with the software name, if any.
        //
        // Load custom FlowUtils-compatible information:
        // - <info> with the software name, if any.
        //
        // Load in the order: FlowUtils, Cytobank, FlowGate. This insures
        // that anything set by FlowGate takes precedence.
        for ( auto child = root->first_node( "data-type:custom_info" );
            child != nullptr;
            child = child->next_sibling( "data-type:custom_info" ) )
        {
            // FlowUtils.
            auto infoChild = child->first_node( "info" );
            if ( infoChild != nullptr )
                this->gateTrees->setCreatorSoftwareName( infoChild->value( ) );

            // Cytobank.
            auto cytobankChild = child->first_node( "cytobank" );
            if ( cytobankChild != nullptr )
            {
                auto aboutChild = cytobankChild->first_node( "about" );
                if ( aboutChild != nullptr )
                    this->gateTrees->setCreatorSoftwareName( aboutChild->value( ) );

                auto experimentTitleChild = cytobankChild->first_node( "experiment_title" );
                if ( experimentTitleChild != nullptr )
                    this->gateTrees->setName( experimentTitleChild->value( ) );
            }

            // FlowGate.
            auto flowGateChild = child->first_node( "flowgate" );
            if ( flowGateChild != nullptr )
            {
                auto nameChild = flowGateChild->first_node( "name" );
                if ( nameChild != nullptr )
                    this->gateTrees->setName( nameChild->value( ) );

                auto descriptionChild = flowGateChild->first_node( "description" );
                if ( descriptionChild != nullptr )
                    this->gateTrees->setDescription( descriptionChild->value( ) );

                auto notesChild = flowGateChild->first_node( "notes" );
                if ( notesChild != nullptr )
                    this->gateTrees->setNotes( notesChild->value( ) );

                auto fcsFileNameChild = flowGateChild->first_node( "fcsFileName" );
                if ( fcsFileNameChild != nullptr )
                    this->gateTrees->setFCSFileName( fcsFileNameChild->value( ) );

                auto creatorSoftwareNameChild = flowGateChild->first_node( "creatorSoftwareName" );
                if ( creatorSoftwareNameChild != nullptr )
                    this->gateTrees->setCreatorSoftwareName( creatorSoftwareNameChild->value( ) );
            }
        }

        const std::string name =
            this->gateTrees->getName( );
        const std::string description =
            this->gateTrees->getDescription( );
        const std::string notes =
            this->gateTrees->getNotes( );
        const std::string fcsFileName =
            this->gateTrees->getFCSFileName( );
        const std::string creatorSoftwareName =
            this->gateTrees->getCreatorSoftwareName( );

        if ( name.empty( ) == false )
            this->fileAttributes.emplace( "name", name );
        if ( description.empty( ) == false )
            this->fileAttributes.emplace( "description", description );
        if ( notes.empty( ) == false )
            this->fileAttributes.emplace( "notes", notes );
        if ( fcsFileName.empty( ) == false )
            this->fileAttributes.emplace( "fcsFileName", fcsFileName );
        if ( creatorSoftwareName.empty( ) == false )
            this->fileAttributes.emplace( "creatorSoftwareName", creatorSoftwareName );

        if ( this->isVerbose( ) == true )
        {
            if ( creatorSoftwareName.empty( ) == false )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Authored using software:" <<
                    "\"" << creatorSoftwareName << "\"\n";

            if ( name.empty( ) == false )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Gate trees name:" <<
                    "\"" << name << "\"\n";

            if ( description.empty( ) == false )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Gate trees description:" <<
                    "\"" << description << "\"\n";

            if ( notes.empty( ) == false )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Gate trees diagnostic notes:" <<
                    "\"" << notes << "\"\n";

            if ( fcsFileName.empty( ) == false )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Authored using FCS file name:" <<
                    "\"" << fcsFileName << "\"\n";
        }
    }

    /**
     * Traverses the XML root to load gates.
     *
     * All gates should be direct children of the root. Each type of
     * gate has its own child nodes to describe features of the gate.
     *
     * @param[in] root
     *   The XML root.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the XML tree.
     */
    void loadGates( const rapidxml::xml_node<>*const root )
    {
        //
        // Allocate.
        // ---------
        // Create a list of gates in need of parents. Each entry has:
        // - The ID of the *parent* needed by the gate.
        // - The gate.
        std::vector<std::pair<std::string,std::shared_ptr<FlowGate::Gates::Gate>>> inNeedOfParent;

        // Create a list of root gates. Each entry is a gate that has
        // no parent.
        std::vector<std::shared_ptr<FlowGate::Gates::Gate>> roots;


        //
        // Parse gates.
        // ------------
        // Loop over the direct children, finding recognized gating nodes.
        for ( auto child = root->first_node( );
            child != nullptr;
            child = child->next_sibling( ) )
        {
            std::shared_ptr<FlowGate::Gates::Gate> gate;
            std::string id;
            std::string parentId;
            const char*const nodeName = child->name( );

            //
            // Skip non-gate nodes.
            // --------------------
            // The direct children of the root include gates, transforms,
            // compensation nodes, and custom or vendor-specific descriptions.
            // Skip anything that is not a gate.
            if ( std::strncmp( "gating:", nodeName, 7 ) != 0 )
                continue;

            //
            // Get IDs.
            // --------
            // Every gate *should* have a unique ID that is used to reference
            // the gate from other gates. But if a gate never needs to be
            // referenced, it could skip the ID (though it shouldn't).
            //
            // Gates that are children of other gates must have a unique
            // parent ID.
            id       = this->loadOptionalAttribute( child, "gating:id" );
            parentId = this->loadOptionalAttribute( child, "gating:parent_id" );

            //
            // Parse gates.
            // ------------
            // Look for known gate types and load them.
            //
            // Gating-ML 2.0 gate types.
            if ( std::strcmp( "gating:RectangleGate", nodeName ) == 0 )
                gate = this->loadRectangleGate( child );
            else if ( std::strcmp( "gating:PolygonGate", nodeName ) == 0 )
                gate = this->loadPolygonGate( child );
            else if ( std::strcmp( "gating:EllipsoidGate", nodeName ) == 0 )
                gate = this->loadEllipsoidGate( child );
            else if ( std::strcmp( "gating:BooleanGate", nodeName ) == 0 )
            {
                this->appendFileLog(
                    "warning",
                    "Boolean gates are not supported by this software." );
                continue;
            }
            else if ( std::strcmp( "gating:QuadrantGate", nodeName ) == 0 )
            {
                this->appendFileLog(
                    "warning",
                    "Quadrant gates are not supported by this software." );
                continue;
            }

            // Deprecated Gating-ML 1.5 gate types.
            else if ( std::strcmp( "gating:PolytopeGate", nodeName ) == 0 ||
                std::strcmp( "gating:DecisionTreeGate", nodeName ) == 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "A deprecated and unsupported gate of type \"" ) +
                    nodeName +
                    std::string( "\" was found. Gates of this type are no longer part of the Gating-ML file format specification and are not supported by this software." ) );
                throw std::runtime_error(
                    ERROR_UNSUPPORTED +
                    std::string( "The file contains an old-style deprecated and unsupported gate type." ) +
                    ERROR_CANNOTLOAD );
            }

            else
            {
                this->appendFileLog(
                    "error",
                    std::string( "A non-standard unrecognized gate of type \"" ) +
                    nodeName +
                    std::string( "\" was found. Gates of this type are not supported by this software." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains a non-standard and unrecognized gate type." ) +
                    ERROR_CANNOTLOAD );
            }

            // Save the ID as the original ID.
            gate->setOriginalId( id );

            // Load any custom gate information that may be present. This
            // includes the gate's name, description, notes, gating method, and
            // report priority.
            this->loadCustomGateInformation( child, gate );

            // All gates should have a unique original ID. If one was given,
            // add the gate to the map from IDs to gates.
            if ( id.empty( ) == false )
                this->idsToGates[id] = gate;

            // Many gates will have a parent ID. Those that do not
            // are root gates and added to the root list. Those that
            // do have a parent ID need to be connected to that parent,
            // so add them to the in-need list.
            if ( parentId.empty( ) == true )
                roots.push_back( gate );
            else
                inNeedOfParent.push_back( std::make_pair( parentId, gate ) );
        }


        //
        // Connect parents and children.
        // -----------------------------
        // Loop over all gates that referred to parents and find the
        // corresponding parent. Add the gate to the parent and continue.
        for ( auto const& [parentId, gate] : inNeedOfParent )
        {
            try
            {
                auto parentGate = this->idsToGates.at( parentId );
                parentGate->appendChild( gate );
            }
            catch ( ... )
            {
                this->appendFileLog(
                    "error",
                    std::string( "A gate refers to a parent gate ID \"" ) +
                    parentId +
                    std::string( "\" that does not exist. Without a valid ID, the gate is incomplete and invalid." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an incomplete and invalid gate." ) +
                    ERROR_CANNOTLOAD );
            }
        }

        inNeedOfParent.clear( );


        //
        // Create gate trees.
        // ------------------
        // Add collected root gates to a gate tree list.
        const Gates::GateIndex nRoots = roots.size( );
        if ( nRoots == 0 )
        {
            this->appendFileLog(
                "warning",
                "The Gating-ML file does not have any recognized gates." );
        }
        else
        {
            for ( Gates::GateIndex i = 0; i < nRoots; ++i )
                this->gateTrees->appendGateTree( roots[i] );

            roots.clear( );
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of gates:" <<
                this->idsToGates.size( ) << "\n";
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of gate trees:" <<
                this->gateTrees->getNumberOfGateTrees( ) << "\n";
        }
    }

    /**
     * Loads a rectangle gate.
     *
     * Each dimension has a minimum and/or a maximum, optional compensation
     * name, an optional transformation name, and an FCS parameter name.
     *
     * Example:
     * @code
     *   <gating:RectangleGate gating:id="myRect"
     *     gating:parent_id="Range_FCS_o100">
     *     <gating:dimension gating:min="0.2" gating:max="0.5"
     *             gating:compensation-ref="FCS">
     *       <data-type:fcs-dimension data-type:name="FL1-H" />
     *     </gating:dimension>
     *     <gating:dimension gating:min="0.25" gating:max="0.4"
     *             gating:compensation-ref="FCS">
     *       <data-type:fcs-dimension data-type:name="FL2-H" />
     *     </gating:dimension>
     *   </gating:RectangleGate>
     * @endcode
     *
     * @param[in] node
     *   The XML parent node for the rectangle gate.
     *
     * @return
     *   Returns a new rectangle gate.
     */
    std::shared_ptr<FlowGate::Gates::Gate> loadRectangleGate(
        const rapidxml::xml_node<>*const node )
    {
        //
        // Get dimensions.
        // ---------------
        // Find dimension children and their attributes.
        std::vector<std::string> parameterNames;
        std::vector<std::string> compensationIds;
        std::vector<std::string> transformIds;
        std::vector<double> minimums;
        std::vector<double> maximums;

        const Gates::DimensionIndex numberOfDimensions = this->loadDimensionList(
            node,
            parameterNames,
            compensationIds,
            transformIds,
            true,
            minimums,
            maximums );


        // At least one dimension is required.
        if ( numberOfDimensions == 0 )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. A rectangle gate was found that has no dimensions." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Create gate.
        // ------------
        // Allocate a gate, set its values, and return it as a shared pointer.
        auto gate = new FlowGate::Gates::RectangleGate(
            minimums, maximums );
        std::shared_ptr<FlowGate::Gates::Gate> sgate;
        sgate.reset( gate );

        for ( Gates::DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
            gate->setDimensionParameterName( i, parameterNames[i] );

            const auto transformId = transformIds[i];
            if ( transformId.empty( ) == false )
            {
                try
                {
                    auto transform = this->idsToTransforms.at( transformId );
                    gate->setDimensionParameterTransform( i, transform );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform ID \"" ) +
                        transformId +
                        std::string( "\" that does not exist." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file contains an invalid gate." ) +
                        ERROR_CANNOTLOAD );
                }
            }

#ifdef COMPENSATION_SUPPORTED
// TODO do something with compensation names.
#endif
        }

        return sgate;
    }

    /**
     * Loads a polygon gate.
     *
     * A polygon has two dimensions and a list of 2D vertices.
     *
     * Each dimension has an optional compensation name, an optional
     * transformation name, and an FCS parameter name.
     *
     * Example:
     * @code
     *  <gating:PolygonGate gating:id="Triangle">
     *    <gating:dimension gating:compensation-ref="uncompensated">
     *      <data-type:fcs-dimension data-type:name="FSC-H" />
     *    </gating:dimension>
     *    <gating:dimension gating:compensation-ref="uncompensated">
     *      <data-type:fcs-dimension data-type:name="SSC-H" />
     *    </gating:dimension>
     *    <gating:vertex>
     *      <gating:coordinate data-type:value="0" />
     *      <gating:coordinate data-type:value="0" />
     *    </gating:vertex>
     *    <gating:vertex>
     *      <gating:coordinate data-type:value="400" />
     *      <gating:coordinate data-type:value="0" />
     *    </gating:vertex>
     *    <gating:vertex>
     *      <gating:coordinate data-type:value="400" />
     *      <gating:coordinate data-type:value="300" />
     *    </gating:vertex>
     *  </gating:PolygonGate>
     * @endcode
     *
     * @param[in] node
     *   The XML parent node for the polygon gate.
     *
     * @return
     *   Returns a new polygon gate.
     */
    std::shared_ptr<FlowGate::Gates::Gate> loadPolygonGate(
        const rapidxml::xml_node<>*const node )
    {
        //
        // Get dimensions.
        // ---------------
        // Find dimension children and their attributes.
        std::vector<std::string> parameterNames;
        std::vector<std::string> compensationNames;
        std::vector<std::string> transformIds;
        std::vector<double> dummyMinimums;
        std::vector<double> dummyMaximums;

        const Gates::DimensionIndex numberOfDimensions = this->loadDimensionList(
            node,
            parameterNames,
            compensationNames,
            transformIds,
            false,
            dummyMinimums,
            dummyMaximums );


        // Exactly two dimensions must be given.
        if ( numberOfDimensions != 2 )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. A polygon gate was found that does not have exaclty two dimensions." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Get vertices.
        // -------------
        // There must be at least three "gating:vertex" children. Each vertex
        // must have two gating coordinate children.
        //
        // Look for them and collect their values.
        std::vector<double> xCoordinates;
        std::vector<double> yCoordinates;

        for ( auto child = node->first_node( "gating:vertex" );
            child != nullptr;
            child = child->next_sibling( "gating:vertex" ) )
        {
            // Initialize a new dimension.
            int nCoordinatesGiven = 0;
            double x, y;

            // Get X and Y children.
            for ( auto vert = child->first_node( "gating:coordinate" );
                vert != nullptr && nCoordinatesGiven <= 2;
                vert = vert->next_sibling( "gating:coordinate" ) )
            {
                auto vattr = vert->first_attribute( "data-type:value" );
                if ( vattr != nullptr )
                {
                    switch ( nCoordinatesGiven )
                    {
                    case 0:
                        x = std::stod( vattr->value( ) );
                        break;

                    case 1:
                        y = std::stod( vattr->value( ) );
                        break;

                    default:
                        break;
                    }

                    ++nCoordinatesGiven;
                }
            }

            // Validate.
            if ( nCoordinatesGiven > 2 )
            {
                const auto id = this->loadOptionalAttribute( node, "gating:id" );
                this->appendFileLog(
                    "error",
                    std::string( "A polygon gate with ID \"" ) + id +
                    std::string( "\" was found that lists a vertex with more than two coordinate values. This makes the vertex invalid, which makes the polygon gate invalid." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid polygon gate." ) +
                    ERROR_CANNOTLOAD );
            }

            if ( nCoordinatesGiven < 2 )
            {
                const auto id = this->loadOptionalAttribute( node, "gating:id" );
                this->appendFileLog(
                    "error",
                    std::string( "A polygon gate with ID \"" ) + id +
                    std::string( "\" was found that lists a vertex with less than two coordinate values. This makes the vertex invalid, which makes the polygon gate invalid." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid polygon gate." ) +
                    ERROR_CANNOTLOAD );
            }

            xCoordinates.push_back( x );
            yCoordinates.push_back( y );
        }


        // At least three vertices are required.
        if ( xCoordinates.size( ) < 3 )
        {
            const auto id = this->loadOptionalAttribute( node, "gating:id" );
            this->appendFileLog(
                "error",
                std::string( "An invalid polygon gate with ID \"" ) + id +
                std::string( "\" was found that has fewer than 3 vertices. This creates a degenerate polygon that has no meaning." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file contains an invalid or incomplete polygon gate." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Create gate.
        // ------------
        // Allocate a gate, set its values, and return it as a shared pointer.
        auto gate = new FlowGate::Gates::PolygonGate(
            xCoordinates, yCoordinates );
        std::shared_ptr<FlowGate::Gates::Gate> sgate;
        sgate.reset( gate );

        for ( Gates::DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
            gate->setDimensionParameterName( i, parameterNames[i] );

            const auto transformId = transformIds[i];
            if ( transformId.empty( ) == false )
            {
                try
                {
                    auto transform = this->idsToTransforms.at( transformId );
                    gate->setDimensionParameterTransform( i, transform );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform ID \"" ) +
                        transformId +
                        std::string( "\" that does not exist." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file contains an invalid gate." ) +
                        ERROR_CANNOTLOAD );
                }
            }

#ifdef COMPENSATION_SUPPORTED
// TODO do something with compensation names.
#endif
        }

        return sgate;
    }

    /**
     * Loads an ellipsoid gate.
     *
     * An ellipsoid has two or more dimensions, a center point (mean),
     * a size (distance squared), and a square covariance matrix controlling
     * the orientation and shape of the ellipsoid.
     *
     * Each dimension has an optional compensation name, an optional
     * transformation name, and an FCS parameter name.
     *
     * Example:
     * @code
     *  <gating:EllipsoidGate gating:id="Ellipse1">
     *    <gating:dimension gating:compensation-ref="uncompensated">
     *      <data-type:fcs-dimension data-type:name="FSC-H" />
     *    </gating:dimension>
     *    <gating:dimension gating:compensation-ref="uncompensated">
     *      <data-type:fcs-dimension data-type:name="SSC-H" />
     *    </gating:dimension>
     *    <gating:distanceSquare data-type:value="1" />
     *    <gating:mean>
     *      <gating:coordinate data-type:value="40" />
     *      <gating:coordinate data-type:value="40" />
     *    </gating:mean>
     *    <gating:covarianceMatrix>
     *      <gating:row>
     *        <gating:entry data-type:value="1134.5" />
     *        <gating:entry data-type:value="-234.5" />
     *      </gating:row>
     *      <gating:row>
     *        <gating:entry data-type:value="-234.5.5" />
     *        <gating:entry data-type:value="1134.5" />
     *      </gating:row>
     *    </gating:covarianceMatrix>
     *  </gating:EllipsoidGate>
     * @endcode
     *
     * @param[in] node
     *   The XML parent node for the polygon gate.
     *
     * @return
     *   Returns a new polygon gate.
     */
    std::shared_ptr<FlowGate::Gates::Gate> loadEllipsoidGate(
        const rapidxml::xml_node<>*const node )
    {
        //
        // Get dimensions.
        // ---------------
        // Find dimension children and their attributes.
        std::vector<std::string> parameterNames;
        std::vector<std::string> compensationNames;
        std::vector<std::string> transformIds;
        std::vector<double> dummyMinimums;
        std::vector<double> dummyMaximums;
        std::vector<double> center;
        std::vector<double> matrix;
        double distanceSquare = 0.0;

        const Gates::DimensionIndex numberOfDimensions = this->loadDimensionList(
            node,
            parameterNames,
            compensationNames,
            transformIds,
            false,
            dummyMinimums,
            dummyMaximums );


        // Exactly two dimensions must be given.
        if ( numberOfDimensions < 2 )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has fewer than two dimensions." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Get distance squared.
        // ---------------------
        // A single distance child gives the size of the ellipsoid.
        auto sizeChild = node->first_node( "gating:distanceSquare" );
        if ( sizeChild == nullptr )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has no distance square value." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }

        auto attr = sizeChild->first_attribute( "data-type:value" );
        if ( attr == nullptr )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has a distance square node with no value." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }

        distanceSquare = std::stod( attr->value( ) );

        //
        // Get center (mean).
        // ------------------
        // A mean child must include one value for each dimension.
        auto centerChild = node->first_node( "gating:mean" );
        if ( centerChild == nullptr )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has no center point." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }

        for ( auto child = centerChild->first_node( "gating:coordinate" );
            child != nullptr;
            child = child->next_sibling( "gating:coordinate" ) )
        {
            auto attr = child->first_attribute( "data-type:value" );
            if ( attr == nullptr )
            {
                this->appendFileLog(
                    "error",
                    "The file is malformed. An ellipsoid gate was found that has a mean node with no value." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file is missing essential information." ) +
                    ERROR_CANNOTLOAD );
            }

            center.push_back( std::stod( attr->value( ) ) );
        }

        if ( center.size( ) != numberOfDimensions )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has the wrong number of values for the center point." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }

        //
        // Get covariance matrix.
        // ----------------------
        // The matrix has N rows with N columns each, where N = the number
        // of dimensions for the gate.
        auto matrixChild = node->first_node( "gating:covarianceMatrix" );
        if ( matrixChild == nullptr )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has no covariance matrix." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }

        for ( auto rowChild = matrixChild->first_node( "gating:row" );
            rowChild != nullptr;
            rowChild = rowChild->next_sibling( "gating:row" ) )
        {
            for ( auto colChild = rowChild->first_node( "gating:entry" );
                colChild != nullptr;
                colChild = colChild->next_sibling( "gating:entry" ) )
            {
                auto attr = colChild->first_attribute( "data-type:value" );
                if ( attr == nullptr )
                {
                    this->appendFileLog(
                        "error",
                        "The file is malformed. An ellipsoid gate was found that has a covariance matrix entry with no value." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file is missing essential information." ) +
                        ERROR_CANNOTLOAD );
                }

                matrix.push_back( std::stod( attr->value( ) ) );
            }
        }

        if ( matrix.size( ) != (numberOfDimensions * numberOfDimensions) )
        {
            this->appendFileLog(
                "error",
                "The file is malformed. An ellipsoid gate was found that has the wrong number of values for the covariance matrix." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file is missing essential information." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Create gate.
        // ------------
        // Allocate a gate, set its values, and return it as a shared pointer.
        auto gate = new FlowGate::Gates::EllipsoidGate(
            center, matrix, distanceSquare );
        std::shared_ptr<FlowGate::Gates::Gate> sgate;
        sgate.reset( gate );

        for ( Gates::DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
            gate->setDimensionParameterName( i, parameterNames[i] );

            const auto transformId = transformIds[i];
            if ( transformId.empty( ) == false )
            {
                try
                {
                    auto transform = this->idsToTransforms.at( transformId );
                    gate->setDimensionParameterTransform( i, transform );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform ID \"" ) +
                        transformId +
                        std::string( "\" that does not exist." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file contains an invalid gate." ) +
                        ERROR_CANNOTLOAD );
                }
            }

#ifdef COMPENSATION_SUPPORTED
// TODO do something with compensation names.
#endif
        }

        return sgate;
    }

    /**
     * Loads any custom information that may be provided for the gate.
     *
     * Custom information is outside of the Gating-ML standard, but contained
     * within a standard-specified child "<data-type:custom_info>".
     *
     * Cytobank defines the following custom information within "<cytobank>":
     * - "<name>" - the gate name.
     *
     * Flowgate defines the following custom information within "<flowgate>"::
     * - "<name>" - the gate name.
     * - "<description>" - the gate description.
     * - "<notes>" - the gate diagnostic notes.
     * - "<gatingMethodName>" - the gating method.
     * - "<reportPriority>" - the report priority.
     *
     * Where there is more than one source of custom information (e.g.
     * Cytoban and Flowgate), Flowgate's information is used.
     *
     * @param[in] node
     *   The XML parent node for the gate.
     * @param[in] gate
     *   The previously parsed gate from the parent node. Custom gate
     *   information, if any, is added to the gate.
     */
    void loadCustomGateInformation(
        const rapidxml::xml_node<>*const node,
        std::shared_ptr<FlowGate::Gates::Gate> gate)
    {
        // Load custom FlowGate information:
        // - <name> with the gate name, if any.
        // - <description> with the gate description, if any.
        // - <notes> with the gate diagnostic notes, if any.
        // - <gatingMethodName> with the gating method, if any.
        // - <reportPriority> with the report priority.
        //
        // Load custom Cytobank-compatible information:
        // - <name> with the gate name, if any.
        //
        // Load custom FlowUtils-compatible information:
        // - None. FlowUtils does not include custom gate information.
        //
        // Load in the order: FlowUtils, Cytobank, FlowGate. This insures
        // that anything set by FlowGate takes precedence.
        auto child = node->first_node( "data-type:custom_info" );
        if ( child == nullptr )
            return;

        // FlowUtils. None.

        // Cytobank. Name only.
        auto cytobankChild = child->first_node( "cytobank" );
        if ( cytobankChild != nullptr )
        {
            auto nameChild = cytobankChild->first_node( "name" );
            if ( nameChild != nullptr )
                gate->setName( nameChild->value( ) );
        }

        // FlowGate. Name, description, notes, gating method, and
        // report priority.
        auto flowGateChild = child->first_node( "flowgate" );
        if ( flowGateChild != nullptr )
        {
            // Look for the gate name.
            auto nameChild = flowGateChild->first_node( "name" );
            if ( nameChild != nullptr )
                gate->setName( nameChild->value( ) );

            // Look for the gate description.
            auto descriptionChild = flowGateChild->first_node( "description" );
            if ( descriptionChild != nullptr )
                gate->setDescription( descriptionChild->value( ) );

            // Look for the gate diagnostic notes.
            auto notesChild = flowGateChild->first_node( "notes" );
            if ( notesChild != nullptr )
                gate->setNotes( notesChild->value( ) );

            // Look for the gating method.
            auto gatingMethodChild = flowGateChild->first_node( "gatingMethodName" );
            if ( gatingMethodChild != nullptr )
            {
                gate->setGatingMethod(
                    FlowGate::Gates::findGatingMethodByName(
                        gatingMethodChild->value( ) ) );
            }

            // Look for the report priority.
            auto priorityChild = flowGateChild->first_node( "reportPriority" );
            if ( priorityChild != nullptr )
            {
                gate->setReportPriority(
                    std::stoul( priorityChild->value( ) ) );
            }

            auto additionalClusteringParametersChild =
                flowGateChild->first_node( "additionalClusteringParameters" );
            if ( additionalClusteringParametersChild != nullptr )
                this->loadAdditionalClusteringParameters(
                    additionalClusteringParametersChild, gate );
        }
    }

    /**
     * Traverses the XML subtree to load additional clustering parameters.
     *
     * @param[in] node
     *   The XML child node containing clustering parameters.
     * @param[in] gate
     *   The previously parsed gate from the parent node. Custom gate
     *   information, if any, is added to the gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the XML tree.
     */
    void loadAdditionalClusteringParameters(
        const rapidxml::xml_node<>*const node,
        std::shared_ptr<FlowGate::Gates::Gate> gate)
    {
        // Validate.
        if ( gate->supportsAdditionalClusteringParameters( ) == false )
        {
            this->appendFileLog(
                "error",
                std::string( "Additional clustering parameters are not supported for " ) +
                FlowGate::Gates::getGateTypeName( gate->getGateType( ) ) +
                std::string( " gates" ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The file contains an invalid gate." ) +
                ERROR_CANNOTLOAD );
        }

        // Input looks like:
        //   <additionalClusteringParameters>
        //     <gating:dimension gating:transformation-ref="ID1">
        //       <data-type:fcs-dimension data-type:name="NAME1" />
        //     </gating:dimension>
        //     <gating:dimension gating:transformation-ref="ID2">
        //       <data-type:fcs-dimension data-type:name="NAME2" />
        //     </gating:dimension>
        //     ...
        //   </additionalClusteringParameters>
        for ( auto child = node->first_node( );
            child != nullptr;
            child = child->next_sibling( ) )
        {
            auto nodeName = child->name( );

            if ( std::strcmp( nodeName, "gating:dimension" ) != 0 )
                continue;

            // Get the transform ID, if any.
            const std::string transformId =
                this->loadOptionalAttribute( child, "gating:transformation-ref" );

            // There should be a child giving the FCS parameter name.
            auto parameterChild = child->first_node( );
            auto parameterChildName = parameterChild->name( );

            if ( std::strcmp( parameterChildName, "data-type:fcs-dimension" ) != 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "Additional clustering parameter is missing its FCS dimension " ) +
                    FlowGate::Gates::getGateTypeName( gate->getGateType( ) ) +
                    std::string( " gates" ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid gate." ) +
                    ERROR_CANNOTLOAD );
            }

            // Get the parameter name.
            const std::string parameterName =
                this->loadRequiredAttribute( parameterChild, "data-type:name" );

            if ( transformId.empty( ) == true )
            {
                // There is no transform.
                gate->appendAdditionalClusteringParameter(
                    parameterName,
                    nullptr );
            }
            else
            {
                // There is a transform. Look up its ID.
                try
                {
                    auto transform = this->idsToTransforms.at( transformId );
                    gate->appendAdditionalClusteringParameter(
                        parameterName,
                        transform );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform ID \"" ) +
                        transformId +
                        std::string( "\" that does not exist." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file contains an invalid gate." ) +
                        ERROR_CANNOTLOAD );
                }
            }
        }
    }
    // @}



//----------------------------------------------------------------------
// Load transforms from file.
//----------------------------------------------------------------------
private:
    /**
     * @name Load transforms from file
     */
    // @{
    /**
     * Traverses the XML root to load transforms.
     *
     * All transforms should be direct children of the root. Each transform
     * has an ID and a child node that describes the type and parameters for
     * the transform.
     *
     * @param[in] root
     *   The XML root.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the XML tree.
     */
    void loadTransforms( const rapidxml::xml_node<>*const root )
    {
        //
        // Parse transforms.
        // -----------------
        // Loop over the direct children, finding recognized transform nodes.
        for ( auto child = root->first_node( "transforms:transformation" );
            child != nullptr;
            child = child->next_sibling( "transforms:transformation" ) )
        {
            //
            // Get the ID.
            // -----------
            // The ID is required for transforms. It is how the transform
            // is referred to by gates.
            const std::string id =
                this->loadRequiredAttribute( child, "transforms:id" );

            //
            // Create transform.
            // -----------------
            // All Gating-ML 2.0 transforms have the same structure:
            // - a "transforms:*" node, where "*" is the transform type.
            // - a list of attributes on the node.
            //
            // The "transforms:*" node must be an immediate child of the
            // "transforms:transformation" node.
            //
            // Custom and vendor-specific information may be included as
            // additional children. These are silently skipped.
            std::shared_ptr<FlowGate::Gates::Transform> transform;
            for ( auto tchild = child->first_node( );
                tchild != nullptr;
                tchild = tchild->next_sibling( ) )
            {
                // There should be exactly one transform child.
                transform = this->loadTransform( tchild );
                if ( transform != nullptr )
                    break;
            }

            if ( transform == nullptr )
            {
                // No transform was built, which means no recognized
                // transform was found as a child of the
                // transforms node.
                this->appendFileLog(
                    "error",
                    std::string( "A transform with ID \"" ) + id +
                    std::string( "\" has no recognized child node describing the transform." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file contains an invalid transform." ) +
                    ERROR_CANNOTLOAD );
            }


            //
            // Custom information.
            // -------------------
            // Load custom FlowGate information:
            // - <name> with the transform name, if any.
            // - <description> with the transform description, if any.
            //
            // Load custom Cytobank-compatible information:
            // - None. Cytobank does not include custom transform information.
            //
            // Load custom FlowUtils-compatible information:
            // - None. FlowUtils does not include custom transform information.
            auto ichild = child->first_node( "data-type:custom_info" );
            if ( ichild != nullptr )
            {
                // FlowUtils. None.

                // Cytobank. None.

                // FlowGate.
                auto flowGateChild = ichild->first_node( "flowgate" );
                if ( flowGateChild != nullptr )
                {
                    auto nameChild = flowGateChild->first_node( "name" );
                    if ( nameChild != nullptr )
                        transform->setName( nameChild->value( ) );

                    auto descriptionChild = flowGateChild->first_node( "description" );
                    if ( descriptionChild != nullptr )
                        transform->setDescription( descriptionChild->value( ) );
                }
            }

            // Save the ID-to-transform mapping.
            this->idsToTransforms[id] = transform;
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of transforms:" <<
                this->idsToTransforms.size( ) << "\n";
        }
    }

    /**
     * Traverses the XML root to load one transform.
     *
     * @param[in] tchild
     *   The XML node describing a single transform.
     *
     * @return
     *   Returns the transform, or a nullptr if no transform was found.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the XML tree.
     */
    std::shared_ptr<FlowGate::Gates::Transform> loadTransform(
        const rapidxml::xml_node<>*const tchild )
    {
        auto tname = tchild->name( );
        std::shared_ptr<FlowGate::Gates::Transform> transform;

        //
        // Skip non-transform nodes.
        // -------------------------
        // The direct children of the transformation node may
        // custom or vendor-specific information. Skip anything
        // that is not a transform.
        if ( std::strncmp( tname, "transforms:", 11 ) != 0 )
            return transform;

        //
        // Parse transform.
        // ----------------
        // Gating-ML 2.0 transforms.
        if ( std::strcmp( tname, "transforms:flin" ) == 0 )
        {
            const double t = this->loadRequiredNumericAttribute(
                tchild, "transforms:T" );
            const double a = this->loadRequiredNumericAttribute(
                tchild, "transforms:A" );
            transform.reset(
                new FlowGate::Gates::ParameterizedLinearTransform( t, a ) );
            return transform;
        }
        if ( std::strcmp( tname, "transforms:flog" ) == 0 )
        {
            const double t = this->loadRequiredNumericAttribute(
                tchild, "transforms:T" );
            const double m = this->loadRequiredNumericAttribute(
                tchild, "transforms:M" );
            transform.reset(
                new FlowGate::Gates::ParameterizedLogarithmicTransform( t, m ) );
            return transform;
        }
        if ( std::strcmp( tname, "transforms:fasinh" ) == 0 )
        {
            const double t = this->loadRequiredNumericAttribute(
                tchild, "transforms:T" );
            const double m = this->loadRequiredNumericAttribute(
                tchild, "transforms:M" );
            const double a = this->loadRequiredNumericAttribute(
                tchild, "transforms:A" );
            transform.reset(
                new FlowGate::Gates::ParameterizedInverseHyperbolicSineTransform( t, a, m ) );
            return transform;
        }
        if ( std::strcmp( tname, "transforms:logicle" ) == 0 )
        {
            const double t = this->loadRequiredNumericAttribute(
                tchild, "transforms:T" );
            const double m = this->loadRequiredNumericAttribute(
                tchild, "transforms:M" );
            const double a = this->loadRequiredNumericAttribute(
                tchild, "transforms:A" );
            const double w = this->loadRequiredNumericAttribute(
                tchild, "transforms:W" );
            transform.reset(
                new FlowGate::Gates::LogicleTransform( t, a, m, w ) );
            return transform;
        }
        if ( std::strcmp( tname, "transforms:hyperlog" ) == 0 )
        {
            const auto b = this->loadOptionalAttribute(
                tchild, "transforms:b" );
            if ( b.empty( ) == true )
            {
                // The "b" attribute was not found and that is good.
                // The Gating-ML 2.0 form of the hyperlog transform
                // has no "b" attribute.
                const double t = this->loadRequiredNumericAttribute(
                    tchild, "transforms:T" );
                const double m = this->loadRequiredNumericAttribute(
                    tchild, "transforms:M" );
                const double a = this->loadRequiredNumericAttribute(
                    tchild, "transforms:A" );
                const double w = this->loadRequiredNumericAttribute(
                    tchild, "transforms:W" );
                transform.reset(
                    new FlowGate::Gates::HyperlogTransform( t, a, m, w ) );
                return transform;
            }

            // Otherwise "b" was found and this is not a Gating-ML 2.0
            // transform. Fall through to earlier transform types.
        }

        // Gating-ML 1.5 transforms.
        if ( std::strcmp( tname, "transforms:dg1polynomial" ) == 0 ||
            std::strcmp( tname, "transforms:ratio" ) == 0 ||
            std::strcmp( tname, "transforms:sqrt" ) == 0 ||
            std::strcmp( tname, "transforms:ln" ) == 0 ||
            std::strcmp( tname, "transforms:exponential" ) == 0 ||
            std::strcmp( tname, "transforms:hyperlog" ) == 0 ||
            std::strcmp( tname, "transforms:asinh" ) == 0 ||
            std::strcmp( tname, "transforms:sinh" ) == 0 ||
            std::strcmp( tname, "transforms:eh" ) == 0 ||
            std::strcmp( tname, "transforms:split-scale" ) == 0 ||
            std::strcmp( tname, "transforms:inverse-split-scale" ) == 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "A deprecated Gating-ML 1.5 transform of type \"" ) +
                tname +
                std::string( "\" was found. Transforms of this type are not supported by this software." ) );
            throw std::runtime_error(
                ERROR_UNSUPPORTED +
                std::string( "The file contains an unsupported transform." ) +
                ERROR_CANNOTLOAD );
        }

        // Unrecognized transform.
        this->appendFileLog(
            "error",
            std::string( "An unrecognized non-standard transform of type \"" ) +
            tname +
            std::string( "\" was found. This is not supported by this software." ) );
        throw std::runtime_error(
            ERROR_UNSUPPORTED +
            std::string( "The file contains an unrecognized transform." ) +
            ERROR_CANNOTLOAD );
    }
    // @}



//----------------------------------------------------------------------
// Load file utilities.
//----------------------------------------------------------------------
private:
    /**
     * @name Load file utilities
     */
    // @{
    /**
     * Loads, validates, and returns an optional attribute.
     *
     * This method looks for an attribute with the specified name and
     * returns its value. If the value is not found or empty, an empty
     * string is returned.
     *
     * @param[in] node
     *   The XML node that should contain the attribute.
     *
     * @return
     *   Returns the attribute's value, or an empty string if no attribute
     *   was found.
     */
    inline std::string loadOptionalAttribute(
        const rapidxml::xml_node<>*const node,
        const std::string& attributeType )
    {
        auto attr = node->first_attribute( attributeType.c_str( ) );
        if ( attr == nullptr )
            return "";

        return attr->value( );
    }

    /**
     * Loads, validates, and returns a required attribute.
     *
     * This method looks for an attribute with the specified name and
     * returns its value. If the value is not found or empty, an error is
     * logged and an exception thrown.
     *
     * @param[in] node
     *   The XML node that should contain the attribute.
     *
     * @return
     *   Returns the attribute's value.
     *
     * @throws std::runtime_error
     *   Throws an exception if the attribute is missing or empty.
     */
    inline std::string loadRequiredAttribute(
        const rapidxml::xml_node<>*const node,
        const std::string& attributeType )
    {
        auto attr = node->first_attribute( attributeType.c_str( ) );
        if ( attr == nullptr )
        {
            this->appendFileLog(
                "error",
                std::string( "A required attribute \"" ) + attributeType +
                std::string( "\" is missing from a \"" ) + node->name( ) +
                std::string( "\" node." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }

        auto s = std::string( attr->value() );
        if ( s.empty( ) == true )
        {
            this->appendFileLog(
                "error",
                std::string( "A required attribute \"" ) + attributeType +
                std::string( "\" is empty in a \"" ) + node->name( ) +
                std::string( "\" node." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }

        return s;
    }

    /**
     * Loads, validates, and returns a required numeric attribute.
     *
     * This method looks for an attribute with the specified name and
     * returns its value. If the value is not found, empty, or cannot
     * be parsed as a number, an error is logged and an exception thrown.
     *
     * @param[in] node
     *   The XML node that should contain the attribute.
     *
     * @return
     *   Returns the attribute's value.
     *
     * @throws std::runtime_error
     *   Throws an exception if the attribute is missing, empty, or cannot
     *   be parsed as a number.
     */
    inline double loadRequiredNumericAttribute(
        const rapidxml::xml_node<>*const node,
        const std::string& attributeType )
    {
        auto s = this->loadRequiredAttribute( node, attributeType );
        try
        {
            return std::stod( s );
        }
        catch ( ... )
        {
            this->appendFileLog(
                "error",
                std::string( "A required numeric attribute \"" ) +
                attributeType +
                std::string( "\" is not numeric in a \"" ) + node->name( ) +
                std::string( "\" node." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_CANNOTLOAD );
        }
    }

    /**
     * Loads and returns a list of dimension attributes.
     *
     * @param[in] node
     *   The XML parent node for the gate.
     * @param[out] parameterNames
     *   The names of FCS parameters with one entry per dimension.
     * @param[out] compensationIds
     *   The IDs of compensations with one entry per dimension.
     * @param[out] transformationIds
     *   The IDs of transformations with one entry per dimension.
     * @param[in] getMinMax
     *   Whether to get min/max attributes.
     * @param[out] minimums
     *   The minimums with one entry per dimension.
     * @param[out] maximums
     *   The maximums with one entry per dimension.
     *
     * @return
     *   Returns the number of dimensions.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a problem with the XML tree.
     */
    Gates::DimensionIndex loadDimensionList(
        const rapidxml::xml_node<>*const node,
        std::vector<std::string>& parameterNames,
        std::vector<std::string>& compensationIds,
        std::vector<std::string>& transformationIds,
        bool getMinMax,
        std::vector<double>& minimums,
        std::vector<double>& maximums )
    {
        //
        // Loop over all dimension children.
        // ---------------------------------
        // Look for immediate children of type "gating:dimension". Each
        // dimension node has optional attributes:
        // - gating:compensation-ref.
        // - gating:transformation-ref.
        // - gating:min.
        // - gating:max.
        //
        // Each dimension node has one of two required child types that
        // describe the dimension:
        // - data-type:new-dimension.
        // - data-type:fcs-dimension.
        //
        // Each of these has required attributes:
        // - data-type:name.
        //
        // SPECIAL HANDLING:
        // The Gating-ML 2.0 standard defines that FCS file parameter names
        // are given using a "data-type:fcs-dimension" node. But the older
        // Gating-ML 1.5 standard uses "data-type:parameter" instead. To
        // support older files, this code accepts either node type.
        Gates::DimensionIndex numberOfDimensions = 0;

        for ( auto child = node->first_node( "gating:dimension" );
            child != nullptr;
            child = child->next_sibling( "gating:dimension" ) )
        {
            //
            // Initialize.
            // -----------
            // Give the new dimension default values for:
            // - Compensation.
            // - Transformation.
            // - FCS parameter name.
            // - min.
            // - max.
            compensationIds.push_back( "" );
            transformationIds.push_back( "" );
            parameterNames.push_back( "" );
            minimums.push_back( std::numeric_limits<double>::lowest( ) );
            maximums.push_back( std::numeric_limits<double>::max( ) );
            int nMinMaxGiven = 0;

            //
            // Get min/max.
            // ------------
            // Get the optional min and max attributes. If given, override
            // the default.
            if ( getMinMax == true )
            {
                auto attr = child->first_attribute( "gating:min" );
                if ( attr != nullptr )
                {
                    minimums[numberOfDimensions] = std::stod( attr->value( ) );
                    ++nMinMaxGiven;
                }

                attr = child->first_attribute( "gating:max" );
                if ( attr != nullptr )
                {
                    maximums[numberOfDimensions] = std::stod( attr->value( ) );
                    ++nMinMaxGiven;
                }
            }

            //
            // Get compensation.
            // -----------------
            // The compensation reference is the name of a separately-defined
            // compensation matrix.
            auto attr = child->first_attribute( "gating:compensation-ref" );
            if ( attr != nullptr )
            {
                auto c = attr->value( );
                if ( std::strcmp( "uncompensated", c ) != 0 )
                    compensationIds[numberOfDimensions] = c;
            }

            //
            // Get transformation.
            // -------------------
            // The transformation reference is the name of a separately-defined
            // transformation method.
            attr = child->first_attribute( "gating:transformation-ref" );
            if ( attr != nullptr )
                transformationIds[numberOfDimensions] = attr->value( );

            //
            // Get dimension type and name.
            // ----------------------------
            // A single child is required that specifies whether the
            // dimension is custom or based upon data in an FCS file.
            rapidxml::xml_node<>* dimensionChild = nullptr;
            if ( (dimensionChild = child->first_node( "data-type:fcs-dimension" )) != nullptr ||
                (dimensionChild = child->first_node( "data-type:parameter" )) != nullptr)
            {
                // Dimension refers to an FCS parameter.
                attr = dimensionChild->first_attribute( "data-type:name" );
                if ( attr == nullptr )
                {
                    this->appendFileLog(
                        "error",
                        "The file is malformed. It defines a \"data-type:fcs-dimension\" node for an FCS file dimension, but does not include the \"data-type:name\" attribute to give the corresponding name of an FCS parameter." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "The file is missing essential information." ) +
                        ERROR_CANNOTLOAD );
                }

                parameterNames[numberOfDimensions] = attr->value( );
            }
            else if ( (dimensionChild = child->first_node( "data-type:new-dimension" )) != nullptr )
            {
                // Dimension refers to a custom dimension.
                this->appendFileLog(
                    "error",
                    "The file defines a \"data-type:new-dimension\" node for a gate, but custom dimensions are not supported by this software." );
                throw std::runtime_error(
                    ERROR_UNSUPPORTED +
                    std::string( "The file requires a custom dimension feature that is not supported by this software." ) +
                    ERROR_CANNOTLOAD );

            }
            else
            {
                this->appendFileLog(
                    "error",
                    "The file is malformed. It defines a \"data-type:new-dimension\" node for a custom dimension, but does not include the \"data-type:name\" attribute of that dimension." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file is missing essential information." ) +
                    ERROR_CANNOTLOAD );
            }

            //
            // Validate.
            // ---------
            if ( getMinMax == true && nMinMaxGiven == 0 )
            {
                this->appendFileLog(
                    "error",
                    "The file is malformed. It defines a \"gating:dimension\" node to describe a dimension used by a gate, but provides neither a \"gating:min\" or a \"gating:max\" range attribute for the dimension." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The file is missing essential information." ) +
                    ERROR_CANNOTLOAD );
            }

            ++numberOfDimensions;
        }

        return numberOfDimensions;
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
     * @see getFileLog()
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
                "Saving Gating-ML file \"" << filePath << "\".\n";

        this->clearFileLog( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", filePath );


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
        // Then save the header, transforms, gates, etc.
        try
        {
#ifdef FLOWGATE_FILEGATINGML_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Save the header.
            this->saveHeader( fp );

            // Save the transforms. Assigns IDs to transforms.
            this->saveTransforms( fp );

            // Save the gates. Assigns IDs to gates.
            this->saveGates( fp );

            // Save the footer.
            this->saveFooter( fp );

#ifdef FLOWGATE_FILEGATINGML_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEGATINGML_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }

        this->transformsToIds.clear( );
    }
    // @}



//----------------------------------------------------------------------
// Save string.
//----------------------------------------------------------------------
public:
    /**
     * @name Save string
     */
    // @{
    /**
     * Saves the object to a new string.
     *
     * The string is written with data from the current object.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Load problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    std::string saveText( )
    {
        std::string s;
        this->saveText( s );
        return s;
    }

    /**
     * Saves the object to the given string.
     *
     * The string is written with data from the current object.
     *
     * @param[out] std::string
     *   The string to save into. It is cleared first.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Load problems are also reported to the file log.
     *
     * @see getFileLog()
     *
     * @todo Rewrite to write directly to a string rather than writing to
     * a temp file and reading that back into a string.
     */
    void saveText( std::string& string )
    {
        string.clear( );

        // FOR NOW, save to a temp file, read the file back in, and
        // return it in the string. This is not efficient, but it is
        // a quicker implementation.
        auto pid = getpid( );
        std::string path = std::string( ".tmp_" ) + std::to_string( pid );
        try
        {
            this->save( path );
        }
        catch ( ... )
        {
            unlink( path.c_str( ) );
            throw;
        }

        this->fileAttributes.emplace( "path", "" );


        //
        // Open the file.
        // --------------
        // Open the file for buffered reading using stdio.
        // Throw an exception if the file cannot be opened or read.
        std::FILE*const fp = std::fopen( path.c_str( ), "r" );
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
        // Load the text in the file.
        std::shared_ptr<char> buffer;
        try
        {
            // Get the file size.
            size_t fileSize = 0;
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

            // Load the entire file into a buffer.
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

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
            std::fclose( fp );
            throw;
        }

        unlink( path.c_str( ) );

        // Return the buffer.
        string = buffer.get( );
    }
    // @}



//----------------------------------------------------------------------
// Save header and footer to file.
//----------------------------------------------------------------------
private:
    /**
     * @name Save header and footer to file.
     */
    // @{
    /**
     * Saves the header to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveHeader( std::FILE*const fp )
    {
        // For I/O error checking, only check the return status of the
        // last operation. If a file becomes unwritable earlier, all further
        // calls will fail with the same error, so checking on only the last
        // one is sufficient. This saves a lot of additional "if" checks and
        // code bloat.
        //
        // XML header.
        // -----------
        // The XML header indicates the XML version number and character
        // encoding. We always use UTF-8.
        std::fprintf( fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );

        //
        // Gating-ML header.
        // -----------------
        // The required header names the Gating-ML and related specifications:
        // - XML instance schema.
        // - Gating schema.
        // - Transforms schema.
        // - Data types schema.
        // - Schema locations.
        std::fprintf( fp, "<gating:Gating-ML" );
        std::fprintf( fp, "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" );
        std::fprintf( fp, "  xmlns:gating=\"http://www.isac-net.org/std/Gating-ML/v2.0/gating\"\n" );
        std::fprintf( fp, "  xmlns:transforms=\"http://www.isac-net.org/std/Gating-ML/v2.0/transformations\"\n" );
        std::fprintf( fp, "  xmlns:data-type=\"http://www.isac-net.org/std/Gating-ML/v2.0/datatypes\"\n" );
        std::fprintf( fp, "  xsi:schemaLocation=\"\n" );
        std::fprintf( fp, "    http://www.isac-net.org/std/Gating-ML/v2.0/gating\n" );
        std::fprintf( fp, "    http://flowcyt.sourceforge.net/gating/2.0/xsd/Gating-ML.v2.0.xsd\n" );
        std::fprintf( fp, "    http://www.isac-net.org/std/Gating-ML/v2.0/transformations\n" );
        std::fprintf( fp, "    http://flowcyt.sourceforge.net/gating/2.0/xsd/Transformations.v2.0.xsd\n" );
        std::fprintf( fp, "    http://www.isac-net.org/std/Gating-ML/v2.0/datatypes\n" );
        if ( std::fprintf( fp, "    http://flowcyt.sourceforge.net/gating/2.0/xsd/DataTypes.v2.0.xsd\">\n" ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file header: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }

        //
        // Custom information.
        // -------------------
        // Save custom FlowGate information:
        // - <name> with the gate tree's name, if any.
        // - <description> with the gate tree's description, if any.
        // - <notes> with the gate tree's diagnostic notes, if any.
        // - <fcsFileName> with the FCS file name, if any.
        // - <creatorSoftwareName> with the software name, if any.
        //
        // Save custom Cytobank-compatible information:
        // - <experiment_title> with the gate trees name, if any.
        // - <about> with the software name, if any.
        //
        // Save custom FlowUtils-compatible information:
        // - <info> with the software name, if any.
        const std::string name =
            this->gateTrees->getName( );
        const std::string description =
            this->gateTrees->getDescription( );
        const std::string notes =
            this->gateTrees->getNotes( );
        const std::string fcsFileName =
            this->gateTrees->getFCSFileName( );
        const std::string creatorSoftwareName =
            this->gateTrees->getCreatorSoftwareName( );

        std::fprintf( fp, "  <data-type:custom_info>\n" );

        // FlowGate.
        std::fprintf( fp, "    <flowgate>\n" );
        if ( name.empty( ) == false )
            std::fprintf( fp, "      <name>%s</name>\n",
                name.c_str( ) );
        if ( description.empty( ) == false )
            std::fprintf( fp, "      <description>%s</description>\n",
                description.c_str( ) );
        if ( notes.empty( ) == false )
            std::fprintf( fp, "      <notes>%s</notes>\n",
                notes.c_str( ) );
        if ( fcsFileName.empty( ) == false )
            std::fprintf( fp, "      <fcsFileName>%s</fcsFileName>\n",
                fcsFileName.c_str( ) );
        if ( creatorSoftwareName.empty( ) == false )
            std::fprintf( fp, "      <creatorSoftwareName>%s</creatorSoftwareName>\n",
                creatorSoftwareName.c_str( ) );
        std::fprintf( fp, "    </flowgate>\n" );

        // Cytobank.
        std::fprintf( fp, "    <cytobank>\n" );
        if ( name.empty( ) == false )
            std::fprintf( fp, "      <experiment_title>%s</experiment_title>\n",
                name.c_str( ) );
        if ( creatorSoftwareName.empty( ) == false )
            std::fprintf( fp, "      <about>%s</about>\n",
                creatorSoftwareName.c_str( ) );
        std::fprintf( fp, "    </cytobank>\n" );

        // FlowUtils.
        if ( creatorSoftwareName.empty( ) == false )
            std::fprintf( fp, "    <info>%s</info>\n",
                creatorSoftwareName.c_str( ) );

        std::fprintf( fp, "  </data-type:custom_info>\n" );
    }

    /**
     * Saves the footer to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveFooter( std::FILE*const fp )
    {
        // Write the generic XML header.
        if ( std::fprintf( fp, "</gating:Gating-ML>\n" ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file footer: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }
    // @}



//----------------------------------------------------------------------
// Save transforms to file.
//----------------------------------------------------------------------
private:
    /**
     * @name Save transforms to file.
     */
    // @{
    /**
     * Saves transforms to the file.
     *
     * Transforms are assigned unique IDs and the transform and ID added
     * to the returned map.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveTransforms( std::FILE*const fp )
    {
        //
        // Build transform list.
        // ---------------------
        // Create a list of all transforms in all gate trees.
        // Include the transforms used for gate shape dimensions AND
        // any additional transforms attached to each gate.
        const auto gates = this->gateTrees->findDescendentGates( );
        transformsToIds.clear( );

        for ( auto const& gate : gates )
        {
            // Transforms attached to gate shape dimensions.
            const Gates::DimensionIndex nDimensions =
                gate->getNumberOfDimensions( );
            for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
            {
                auto transform = gate->getDimensionParameterTransform( i );
                if ( transform != nullptr )
                {
                    const std::string id =
                        std::string( "Transform_" ) +
                        std::to_string( transform->getId( ) );

                    transformsToIds[transform] = id;
                }
            }

            // Additional transforms used for clustering.
            const Gates::GateIndex nAdditional =
                gate->getNumberOfAdditionalClusteringParameters( );
            for ( Gates::GateIndex i = 0; i < nAdditional; ++i )
            {
                auto transform =
                    gate->getAdditionalClusteringParameterTransform( i );
                if ( transform != nullptr )
                {
                    const std::string id =
                        std::string( "Transform_" ) +
                        std::to_string( transform->getId( ) );

                    transformsToIds[transform] = id;
                }
            }
        }


        //
        // Save transforms.
        // ----------------
        // Iterate over the transforms and write them out.
        std::fprintf( fp, "\n" );
        for ( auto const& [transform, id] : transformsToIds )
        {
            this->saveTransformStart( fp, id, transform );
            this->saveTransform( fp, transform );
            this->saveTransformEnd( fp, transform );
        }
    }

    /**
     * Saves a transform to a file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveTransform(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        if ( transform == nullptr )
            return;

        switch ( transform->getTransformType( ) )
        {
        case FlowGate::Gates::PARAMETERIZED_LINEAR_TRANSFORM:
            this->saveParameterizedLinearTransform( fp, transform );
            break;

        case FlowGate::Gates::PARAMETERIZED_LOGARITHMIC_TRANSFORM:
            this->saveParameterizedLogarithmicTransform( fp, transform );
            break;

        case FlowGate::Gates::PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM:
            this->saveParameterizedInverseHyperbolicSineTransform( fp, transform );
            break;

        case FlowGate::Gates::LOGICLE_TRANSFORM:
            this->saveLogicleTransform( fp, transform );
            break;

        case FlowGate::Gates::HYPERLOG_TRANSFORM:
            this->saveHyperlogTransform( fp, transform );
            break;

        default:
            // Unknown transform type?
            break;
        }
    }

    /**
     * Saves the start of a transform to a file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] id
     *   The ID of the transform.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveTransformStart(
        std::FILE*const fp,
        const std::string& id,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        //
        // Open transform.
        // ---------------
        // Start the transform, giving it a unique ID.
        std::fprintf( fp, "  <transforms:transformation transforms:id=\"%s\">\n", id.c_str( ) );


        //
        // Custom information.
        // -------------------
        // Save custom FlowGate information:
        // - <name> with the transform name, if any.
        // - <description> with the transform description, if any.
        //
        // Save custom Cytobank-compatible information:
        // - None. Cytobank does not include custom transform information.
        //
        // Save custom FlowUtils-compatible information:
        // - None. FlowUtils does not include custom transform information.
        const std::string name        = transform->getName( );
        const std::string description = transform->getDescription( );

        if ( name.empty( ) == false || description.empty( ) == false )
        {
            std::fprintf( fp, "    <data-type:custom_info>\n" );
            std::fprintf( fp, "      <flowgate>\n" );

            if ( name.empty( ) == false )
                std::fprintf( fp, "        <name>%s</name>\n",
                    name.c_str( ) );

            if ( description.empty( ) == false )
                std::fprintf( fp, "        <description>%s</description>\n",
                    description.c_str( ) );

            std::fprintf( fp, "      </flowgate>\n" );
            std::fprintf( fp, "    </data-type:custom_info>\n" );
        }
    }

    /**
     * Saves the end of a transform to a file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveTransformEnd(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        // For I/O error checking, only check the return status of the
        // last operation. If a file becomes unwritable earlier, all further
        // calls will fail with the same error, so checking on only the last
        // one is sufficient. This saves a lot of additional "if" checks and
        // code bloat.
        if ( std::fprintf( fp, "  </transforms:transformation>\n" ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file transforms: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves a parameterized linear transform to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveParameterizedLinearTransform(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        const auto t = (const FlowGate::Gates::ParameterizedLinearTransform*)(transform.get());

        std::fprintf( fp, "    <transforms:flin\n" );
        std::fprintf( fp, "      transforms:T=\"%lf\"\n", t->getT( ) );
        std::fprintf( fp, "      transforms:A=\"%lf\" />\n", t->getA( ) );
    }

    /**
     * Saves a parameterized logarithmic transform to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveParameterizedLogarithmicTransform(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        const auto t = (const FlowGate::Gates::ParameterizedLogarithmicTransform*)(transform.get());

        std::fprintf( fp, "    <transforms:flog\n" );
        std::fprintf( fp, "      transforms:T=\"%lf\"\n", t->getT( ) );
        std::fprintf( fp, "      transforms:M=\"%lf\" />\n", t->getM( ) );
    }

    /**
     * Saves a parameterized inverse hyperbolic sine transform to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveParameterizedInverseHyperbolicSineTransform(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        const auto t = (const FlowGate::Gates::ParameterizedInverseHyperbolicSineTransform*)(transform.get());

        std::fprintf( fp, "    <transforms:fasinh\n" );
        std::fprintf( fp, "      transforms:T=\"%lf\"\n", t->getT( ) );
        std::fprintf( fp, "      transforms:A=\"%lf\"\n", t->getA( ) );
        std::fprintf( fp, "      transforms:M=\"%lf\" />\n", t->getM( ) );
    }

    /**
     * Saves a logicle transform to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveLogicleTransform(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        const auto t = (const FlowGate::Gates::LogicleTransform*)(transform.get());
        std::fprintf( fp, "    <transforms:logicle\n" );
        std::fprintf( fp, "      transforms:T=\"%lf\"\n", t->getT( ) );
        std::fprintf( fp, "      transforms:A=\"%lf\"\n", t->getA( ) );
        std::fprintf( fp, "      transforms:W=\"%lf\"\n", t->getW( ) );
        std::fprintf( fp, "      transforms:M=\"%lf\" />\n", t->getM( ) );
    }

    /**
     * Saves a hyperlog transform to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] transform
     *   The transform.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveHyperlogTransform(
        std::FILE*const fp,
        std::shared_ptr<const FlowGate::Gates::Transform> transform )
    {
        const auto t = (const FlowGate::Gates::HyperlogTransform*)(transform.get());

        std::fprintf( fp, "    <transforms:hyperlog\n" );
        std::fprintf( fp, "      transforms:T=\"%lf\"\n", t->getT( ) );
        std::fprintf( fp, "      transforms:A=\"%lf\"\n", t->getA( ) );
        std::fprintf( fp, "      transforms:W=\"%lf\"\n", t->getW( ) );
        std::fprintf( fp, "      transforms:M=\"%lf\" />\n", t->getM( ) );
    }
    // @}



//----------------------------------------------------------------------
// Save gates to file.
//----------------------------------------------------------------------
private:
    /**
     * @name Save gates to file.
     */
    // @{
    /**
     * Saves gates to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveGates( std::FILE*const fp )
    {
        //
        // Build gate list.
        // ----------------
        // Create a list of all gates in all gate trees, ordered such that
        // parents are earlier in the list than their children. Each entry
        // is a tuple with the parent's list index and the child gate.
        const auto gates =
            this->gateTrees->findDescendentGatesWithParentIndexes( );
        const Gates::GateIndex nGates = gates.size( );


        //
        // Save gates.
        // -----------
        // Iterate over the gates and write them out.
        std::fprintf( fp, "\n" );
        for ( Gates::GateIndex i = 0; i < nGates; ++i )
        {
            // Get the next gate and build it's ID.
            const auto& [parentIndex, gate] = gates[i];
            const std::string id = std::string( "Gate_" ) +
                std::to_string( gate->getId( ) );

            // Get the parent gate, if any, and build it's ID.
            std::string parentId = "";
            if ( parentIndex != i )
            {
                const auto parentGate = gates[parentIndex].second;
                parentId = (std::string( "Gate_" ) +
                    std::to_string( parentGate->getId( ) ));
            }

            switch ( gate->getGateType( ) )
            {
            case FlowGate::Gates::RECTANGLE_GATE:
                saveGateStart( fp, id, parentId, "RectangleGate", gate );
                saveRectangleGate( fp, gate );
                saveGateEnd( fp, "RectangleGate", gate );
                break;

            case FlowGate::Gates::POLYGON_GATE:
                saveGateStart( fp, id, parentId, "PolygonGate", gate );
                savePolygonGate( fp, gate );
                saveGateEnd( fp, "PolygonGate", gate );
                break;

            case FlowGate::Gates::ELLIPSOID_GATE:
                saveGateStart( fp, id, parentId, "EllipsoidGate", gate );
                saveEllipsoidGate( fp, gate );
                saveGateEnd( fp, "EllipsoidGate", gate );
                break;

            case FlowGate::Gates::QUADRANT_GATE:
                //saveGateStart( fp, id, parentId, "QuadrantGate", gate );
                //saveQuadrantGate( fp, gate );
                //saveGateEnd( fp, "QuadrantGate", gate );
                break;

            case FlowGate::Gates::BOOLEAN_GATE:
                //saveGateStart( fp, id, parentId, "BooleanGate", gate );
                //saveBooleanGate( fp, gate );
                //saveGateEnd( fp, "BooleanGate", gate );
                break;

            default:
                // Unknown gate type?
                break;
            }
        }
    }

    /**
     * Saves the start of a gate to a file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] id
     *   The ID of the gate.
     * @param[in] parentId
     *   The ID of the gate's parent, if any.
     * @param[in] gateTypeName
     *   The gate type for the XML node.
     * @param[in] gate
     *   The gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveGateStart(
        std::FILE*const fp,
        const std::string& id,
        const std::string& parentId,
        const std::string& gateTypeName,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        //
        // Open gate.
        // ----------
        // Start the gate, giving it a unique ID.
        if ( parentId.empty( ) == true )
            std::fprintf( fp, "  <gating:%s gating:id=\"%s\">\n",
                gateTypeName.c_str( ),
                id.c_str( ) );
        else
            std::fprintf( fp, "  <gating:%s gating:id=\"%s\" gating:parent_id=\"%s\">\n",
                gateTypeName.c_str( ),
                id.c_str( ),
                parentId.c_str( ) );


        //
        // Custom information.
        // -------------------
        // Save custom FlowGate information:
        // - <name> with the gate name, if any.
        // - <description> with the gate description, if any.
        // - <notes> with the gate diagnostic notes, if any.
        // - <gatingMethodName> with the gating method, if any.
        // - <reportPriority> with the report priority.
        //
        // Save custom Cytobank-compatible information:
        // - <name> with the gate name, if any.
        //
        // Save custom FlowUtils-compatible information:
        // - None. FlowUtils does not include custom gate information.
        const std::string name         = gate->getName( );
        const std::string description  = gate->getDescription( );
        const std::string notes        = gate->getNotes( );
        const std::string gatingMethod = gate->getGatingMethodName( );
        const uint32_t priority        = gate->getReportPriority();

        std::fprintf( fp, "    <data-type:custom_info>\n" );

        // FlowGate.
        std::fprintf( fp, "      <flowgate>\n" );
        if ( name.empty( ) == false )
            std::fprintf( fp, "        <name>%s</name>\n",
                name.c_str( ) );
        if ( description.empty( ) == false )
            std::fprintf( fp, "        <description>%s</description>\n",
                description.c_str( ) );
        if ( notes.empty( ) == false )
            std::fprintf( fp, "        <notes>%s</notes>\n",
                notes.c_str( ) );
        if ( gatingMethod.empty( ) == false )
            std::fprintf( fp, "        <gatingMethodName>%s</gatingMethodName>\n",
                gatingMethod.c_str( ) );
        std::fprintf( fp, "        <reportPriority>%u</reportPriority>\n",
            priority);

        if ( gate->hasAdditionalClusteringParameters( ) == true )
        {
            // Output looks like:
            //   <additionalClusteringParameters>
            //     <gating:dimension gating:transformation-ref="ID1">
            //       <data-type:fcs-dimension data-type:name="NAME1" />
            //     </gating:dimension>
            //     <gating:dimension gating:transformation-ref="ID2">
            //       <data-type:fcs-dimension data-type:name="NAME2" />
            //     </gating:dimension>
            //     ...
            //   </additionalClusteringParameters>
            std::fprintf( fp, "        <additionalClusteringParameters>\n" );
            const Gates::GateIndex n =
                gate->getNumberOfAdditionalClusteringParameters( );

            for ( Gates::GateIndex i = 0; i < n; ++i )
            {
                // Get the parameter name and it's optional transform.
                const auto pname =
                    gate->getAdditionalClusteringParameterName( i );
                const auto transform =
                    gate->getAdditionalClusteringParameterTransform( i );

                if ( transform == nullptr )
                {
                    // No transform. Just give the parameter's name.
                    std::fprintf( fp, "          <gating:dimension>\n" );
                    std::fprintf( fp, "            <data-type:fcs-dimension data-type:name=\"%s\" />\n",
                        pname.c_str( ) );
                    std::fprintf( fp, "          </gating:dimension>\n" );
                }
                else
                {
                    // Has transform. Give the transform's ID and then
                    // the parameter's name.
                    const std::string transformId =
                        this->transformsToIds.at( transform );

                    std::fprintf( fp, "          <gating:dimension gating:transformation-ref=\"%s\">\n",
                        transformId.c_str( ) );

                    std::fprintf( fp, "            <data-type:fcs-dimension data-type:name=\"%s\" />\n",
                        pname.c_str( ) );
                    std::fprintf( fp, "          </gating:dimension>\n" );
                }
            }
            std::fprintf( fp, "        </additionalClusteringParameters>\n" );
        }

        std::fprintf( fp, "      </flowgate>\n" );

        // Cytobank.
        std::fprintf( fp, "      <cytobank>\n" );
        if ( name.empty( ) == false )
            std::fprintf( fp, "        <name>%s</name>\n",
                name.c_str( ) );
        std::fprintf( fp, "        <type>%s</type>\n",
            gateTypeName.c_str( ) );
        std::fprintf( fp, "      </cytobank>\n" );

        std::fprintf( fp, "    </data-type:custom_info>\n" );
    }

    /**
     * Saves the end of a gate to a file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] gateTypeName
     *   The gate type for the XML node.
     * @param[in] gate
     *   The gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveGateEnd(
        std::FILE*const fp,
        const std::string& gateTypeName,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        // For I/O error checking, only check the return status of the
        // last operation. If a file becomes unwritable earlier, all further
        // calls will fail with the same error, so checking on only the last
        // one is sufficient. This saves a lot of additional "if" checks and
        // code bloat.
        if ( std::fprintf( fp, "  </gating:%s>\n", gateTypeName.c_str( ) ) < 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The system reported an error while writing the file gates: " ) +
                std::string( std::strerror( errno ) ) );
            throw std::runtime_error(
                ERROR_WRITE +
                ERROR_CANNOTSAVE );
        }
    }

    /**
     * Saves a rectangle gate to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] gate
     *   The gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveRectangleGate(
        std::FILE*const fp,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        // Get the gate and its dimensions.
        const auto g = (FlowGate::Gates::RectangleGate*)(gate.get());
        const Gates::DimensionIndex nDimensions = g->getNumberOfDimensions( );

        // Loop over the gate's dimensions.
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            // Get the dimension's attributes.
            double min, max;
            g->getDimensionMinimumMaximum( i, min, max );
            const auto parameterName = g->getDimensionParameterName( i );
            const auto transform     = g->getDimensionParameterTransform( i );
#ifdef COMPENSATION_SUPPORTED
            const auto compensation  = g->getDimensionCompensation( i );
// TODO do something with compensation names.
#endif

            // Write out the gate's dimension and its parameter name.
            std::fprintf( fp, "    <gating:dimension\n" );
            std::fprintf( fp, "      gating:min=\"%lf\"\n", min );
            std::fprintf( fp, "      gating:max=\"%lf\"\n", max );
            if ( transform != nullptr )
            {
                try
                {
                    const std::string transformId =
                        this->transformsToIds.at( transform );
                    std::fprintf( fp, "      gating:transformation-ref=\"%s\"\n",
                        transformId.c_str( ) );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform that cannot be found. The gate and its transform may have been modified by another thread while the gate was being saved to a file." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "A gate's transform is malformed." ) +
                        ERROR_CANNOTSAVE );
                }
            }
            std::fprintf( fp, "      gating:compensation-ref=\"uncompensated\">\n" );
            std::fprintf( fp, "     <data-type:fcs-dimension data-type:name=\"%s\" />\n",
                parameterName.c_str( ) );
            std::fprintf( fp, "    </gating:dimension>\n" );
        }
    }

    /**
     * Saves a polygon gate to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] gate
     *   The gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void savePolygonGate(
        std::FILE*const fp,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        // Get the gate and its dimensions.
        const auto g = (FlowGate::Gates::PolygonGate*)(gate.get());
        const Gates::DimensionIndex nDimensions = g->getNumberOfDimensions( );
        const Gates::VertexIndex nVertices   = g->getNumberOfVertices( );

        // Loop over the gate's dimensions.
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            // Get the dimension's attributes.
            const auto parameterName = g->getDimensionParameterName( i );
            const auto transform     = g->getDimensionParameterTransform( i );
#ifdef COMPENSATION_SUPPORTED
            const auto compensation  = g->getDimensionCompensation( i );
// TODO do something with compensation names.
#endif

            // Write out the gate's dimension and its parameter name.
            std::fprintf( fp, "    <gating:dimension\n" );
            if ( transform != nullptr )
            {
                try
                {
                    const std::string transformId =
                        this->transformsToIds.at( transform );
                    std::fprintf( fp, "      gating:transformation-ref=\"%s\"\n",
                        transformId.c_str( ) );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform that cannot be found. The gate and its transform may have been modified by another thread while the gate was being saved to a file." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "A gate's transform is malformed." ) +
                        ERROR_CANNOTSAVE );
                }
            }
            std::fprintf( fp, "      gating:compensation-ref=\"uncompensated\"\n" );
            std::fprintf( fp, "      >\n" );
            std::fprintf( fp, "      <data-type:fcs-dimension data-type:name=\"%s\" />\n",
                parameterName.c_str( ) );
            std::fprintf( fp, "    </gating:dimension>\n" );
        }

        // Loop over the gate's vertices.
        for ( Gates::VertexIndex i = 0; i < nVertices; ++i )
        {
            // Get the vertex.
            double x, y;
            g->getVertex( i, x, y );

            // Write the vertex.
            std::fprintf( fp, "    <gating:vertex>\n" );
            std::fprintf( fp, "      <gating:coordinate data-type:value=\"%lf\" />\n", x );
            std::fprintf( fp, "      <gating:coordinate data-type:value=\"%lf\" />\n", y );
            std::fprintf( fp, "    </gating:vertex>\n" );
        }
    }

    /**
     * Saves an ellipsoid gate to the file.
     *
     * @param[in] fp
     *   The file pointer for the file to write.
     * @param[in] gate
     *   The gate.
     *
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void saveEllipsoidGate(
        std::FILE*const fp,
        std::shared_ptr<FlowGate::Gates::Gate> gate )
    {
        // Get the gate and its dimensions.
        const auto g = (FlowGate::Gates::EllipsoidGate*)(gate.get());
        const Gates::DimensionIndex nDimensions = g->getNumberOfDimensions( );
        const auto center = g->getCenter( );
        const auto distance = g->getSquaredDistance( );
        const auto matrix = g->getCovarianceMatrix( );

        // Loop over the gate's dimensions.
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            // Get the dimension's attributes.
            const auto parameterName = g->getDimensionParameterName( i );
            const auto transform     = g->getDimensionParameterTransform( i );
#ifdef COMPENSATION_SUPPORTED
            const auto compensation  = g->getDimensionCompensation( i );
// TODO do something with compensation names.
#endif

            // Write out the gate's dimension and its parameter name.
            std::fprintf( fp, "    <gating:dimension\n" );
            if ( transform != nullptr )
            {
                try
                {
                    const std::string transformId =
                        this->transformsToIds.at( transform );
                    std::fprintf( fp, "      gating:transformation-ref=\"%s\"\n",
                        transformId.c_str( ) );
                }
                catch ( ... )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "A gate refers to a transform that cannot be found. The gate and its transform may have been modified by another thread while the gate was being saved to a file." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        std::string( "A gate's transform is malformed." ) +
                        ERROR_CANNOTSAVE );
                }
            }
            std::fprintf( fp, "      gating:compensation-ref=\"uncompensated\"\n" );
            std::fprintf( fp, "      >\n" );
            std::fprintf( fp, "      <data-type:fcs-dimension data-type:name=\"%s\" />\n",
                parameterName.c_str( ) );
            std::fprintf( fp, "    </gating:dimension>\n" );
        }

        // Output the center (mean) by looping again over the dimensions.
        std::fprintf( fp, "    <gating:mean>\n" );
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            std::fprintf( fp, "      <gating:coordinate data-type:value=\"%lf\" />\n", center[i] );
        }
        std::fprintf( fp, "    </gating:mean>\n" );

        // Output the distance squared.
        std::fprintf( fp, "    <gating:distanceSquare data-type:value=\"%lf\" />\n", distance );

        // Output the covariance matrix. The matrix is square, with the
        // number of rows and columns equal to the number of dimensions.
        std::fprintf( fp, "    <gating:covarianceMatrix>\n" );
        for ( Gates::DimensionIndex i = 0; i < nDimensions; ++i )
        {
            std::fprintf( fp, "      <gating:row>\n" );
            for ( Gates::DimensionIndex j = 0; j < nDimensions; ++j )
            {
                const auto c = matrix[i*nDimensions + j];
                std::fprintf( fp, "        <gating:entry data-type:value=\"%lf\" />\n", c );
            }
            std::fprintf( fp, "      </gating:row>\n" );
        }
        std::fprintf( fp, "    </gating:covarianceMatrix>\n" );
    }
    // @}
};

} // End File namespace
} // End FlowGate namespace

