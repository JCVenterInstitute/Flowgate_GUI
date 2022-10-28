/**
 * @file
 * Converts between flow cytometry data file formats.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "EventTable.h"
 * @li FlowGate "GateTrees.h"
 * @li FlowGate "FileFCS.h"
 * @li FlowGate "FileFGBinaryEvents.h"
 * @li FlowGate "FileFGTextEvents.h"
 * @li FlowGate "FileFGGatingCache.h"
 * @li FlowGate "FileFGTextGates.h"
 * @li FlowGate "FileFGJsonGates.h"
 * @li FlowGate "FileGatingML.h"
 * @li FlowGate "FileFGJsonParameterMap.h"
 * @li Rapid XML (via "FileGatingML.h")
 */
#include <algorithm>    // std::transform
#include <iomanip>      // I/O formatting
#include <iostream>     // std::cerr, ...
#include <string>       // std::string, ...
#include <vector>       // Vectors

// OpenMP.
#ifdef _OPENMP
#include <omp.h>        // OpenMP
#endif

// Time for benchmarking.
#if defined(_WIN32)
#define BENCHMARK_TIME_AVAILABLE
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#define BENCHMARK_TIME_AVAILABLE
#include <unistd.h>     // POSIX flags
#include <time.h>       // clock_gettime(), time()
#include <sys/time.h>   // gethrtime(), gettimeofday()
#endif

#include "GateTrees.h"          // FlowGate gates
#include "EventTable.h"         // FlowGate event table
#include "GatingCache.h"        // FlowGate gating cache

#include "FileFCS.h"            // ISAC FCS file format
#include "FileFGBinaryEvents.h" // FlowGate Binary Events file format
#include "FileFGTextEvents.h"   // FlowGate Text Events file format (legacy)
#include "FileFGGatingCache.h"  // FlowGate Gating Cache file format
#include "FileFGTextGates.h"    // FlowGate Text Gates file format (legacy)
#include "FileFGJsonGates.h"    // FlowGate JSON Gates file format
#include "FileFGJsonParameterMap.h" // FlowGate JSON Parameter Map file format
#include "FileGatingML.h"       // ISAC Gating-ML file format





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





using namespace FlowGate::File;
using namespace FlowGate::Events;
using namespace FlowGate::Gates;





//----------------------------------------------------------------------
//
// Enums.
//
//----------------------------------------------------------------------
/**
 * Indicates a supported file format.
 */
/**
 * Indicates a supported data file format.
 */
enum DataFileFormat
{
    UNKNOWN_DATA_FORMAT,

    // Event formats.
    FCS_FORMAT,
    FGBINARYEVENTS_FORMAT,
    FGTEXTEVENTS_FORMAT,

    // Gate formats.
    FGTEXTGATES_FORMAT,
    FGJSONGATES_FORMAT,
    GATINGML_FORMAT,

    // Gating cache formats.
    FGGATINGCACHE_FORMAT,

    // Misc.
    FGJSONPARAMETERMAP_FORMAT
};

/**
 * Indicates a supported text file format.
 */
enum TextFileFormat
{
    UNKNOWN_TEXT_FORMAT,
    TEXT_FORMAT,
    JSON_FORMAT,
};

/**
 * Returns TRUE if the format is for an event file.
 *
 * @param format
 *   The format to check.
 *
 * @return
 *   Returns TRUE if an event format.
 */
bool isEventFile( const DataFileFormat format )
    noexcept
{
    switch ( format )
    {
        case FCS_FORMAT:
        case FGBINARYEVENTS_FORMAT:
        case FGTEXTEVENTS_FORMAT:
            return true;

        default:
            return false;
    }
}

/**
 * Returns TRUE if the format is for a gate file.
 *
 * @param format
 *   The format to check.
 *
 * @return
 *   Returns TRUE if a gate format.
 */
bool isGateFile( const DataFileFormat format )
    noexcept
{
    switch ( format )
    {
        case FGTEXTGATES_FORMAT:
        case FGJSONGATES_FORMAT:
        case GATINGML_FORMAT:
            return true;

        default:
            return false;
    }
}

/**
 * Returns TRUE if the format is for a cache file.
 *
 * @param format
 *   The format to check.
 *
 * @return
 *   Returns TRUE if a cache format.
 */
bool isCacheFile( const DataFileFormat format )
    noexcept
{
    return (format == FGGATINGCACHE_FORMAT);
}

/**
 * Returns TRUE if the format is for a parameter map file.
 *
 * @param format
 *   The format to check.
 *
 * @return
 *   Returns TRUE if a cache format.
 */
bool isParameterMapFile( const DataFileFormat format )
    noexcept
{
    return (format == FGJSONPARAMETERMAP_FORMAT);
}



//----------------------------------------------------------------------
//
// Application state.
//
//----------------------------------------------------------------------
/**
 * Holds application state, including values from the command line.
 */
class Application final
{
public:
    // Name and version ------------------------------------------------
    /**
     * The application's version number.
     */
    inline static const char* applicationVersion = "1.4.0";

    /**
     * The application's build date.
     */
    inline static const std::string applicationBuildDate =
        std::string( __DATE__ ) + std::string( " " ) + std::string(__TIME__);

    /**
     * The software credit.
     */
    inline static const std::string applicationCredit =
        "David R. Nadeau (University of California at San Diego (UCSD))";

    /**
     * The software copyright.
     */
    inline static const std::string applicationCopyright =
        "Copyright (c) Regents of the University of California";

    /**
     * The software license.
     */
    inline static const std::string applicationLicense =
        "GNU Lesser General Public License, version 2.1";



    // Command line arguments and state --------------------------------
    /**
     * The application's path, as it was typed to execute it.
     */
    std::string applicationPath;

    /**
     * The application's name at the end of the path.
     */
    std::string applicationName;

    /**
     * The file path for the source file.
     */
    std::string sourcePath;

    /**
     * The source file format.
     */
    DataFileFormat sourceFormat = UNKNOWN_DATA_FORMAT;

    /**
     * The file path for the support file.
     *
     * This may be empty if conversion does not require a source file.
     */
    std::string supportPath;

    /**
     * The support input file format.
     */
    DataFileFormat supportFormat = UNKNOWN_DATA_FORMAT;

    /**
     * The file path for the output file.
     */
    std::string destinationPath;

    /**
     * The output file format.
     */
    DataFileFormat destinationFormat = UNKNOWN_DATA_FORMAT;

    /**
     * The stdout output format.
     */
    TextFileFormat displayFormat = UNKNOWN_TEXT_FORMAT;

    /**
     * The maximum number of events to load and save.
     */
    ssize_t maxEvents = (-1);

    /**
     * The number of threads to use.
     */
    int numberOfThreads = 0;

    /**
     * When true, deidentify the file. The meaning varies depending upon
     * the type of file:
     *
     * - FCS files: removes all dictionary entries that may contain personal
     *   information.
     *
     * - Gating-ML and JSON gating files: removes the reference FCS filename,
     *   and all diagnostic notes on gate trees and individual gates.
     */
    bool deidentify = false;

    /**
     * Whether to auto-scale values.
     */
    bool autoScale = true;

    /**
     * Whether to compensate values.
     */
    bool compensate = false;

    /**
     * A map of edits to perform.
     *
     * Each edit has a key that names the edit to be performed (e.g.
     * "setgatenotes"). The associated tuple's meaning varies among keys
     * but typically:
     * - Item 0 is a gate index.
     * - Item 1 is a dimension index.
     * - Item 3 is a value, such as a parameter name.
     *
     * All three are stored as strings and parsed by the edit code into
     * specific integer, float, or string values.
     */
    std::map<std::string, std::tuple<std::string,std::string,std::string>> edits;

    /**
     * Whether to be verbose.
     */
    bool verbose = false;

    /**
     * Whether to show error details.
     */
    bool showWarnings = false;

    /**
     * Whether to show benchmark times.
     */
    bool benchmark = false;
};

/**
 * The application's state.
 */
Application application;





//----------------------------------------------------------------------
//
// File name extension utilities.
//
//----------------------------------------------------------------------
/**
 * Returns the data format enum for the given file name extension.
 *
 * @param[in] extension
 *   The file name extension.
 *
 * @return
 *   The file format enum.
 */
DataFileFormat findDataFileFormat( std::string extension )
    noexcept
{
    // Convert the extension to lower case.
    std::transform(
        extension.begin( ),
        extension.end( ),
        extension.begin( ),
        [](unsigned char c){ return std::tolower(c); } );

    if ( FileFCS::isFileNameExtension( extension ) == true )
        return FCS_FORMAT;
    if ( FileFGBinaryEvents::isFileNameExtension( extension ) == true )
        return FGBINARYEVENTS_FORMAT;
    if ( FileFGTextEvents::isFileNameExtension( extension ) == true )
        return FGTEXTEVENTS_FORMAT;
    if ( FileFGGatingCache::isFileNameExtension( extension ) == true )
        return FGGATINGCACHE_FORMAT;
    if ( FileFGTextGates::isFileNameExtension( extension ) == true )
        return FGTEXTGATES_FORMAT;
    if ( FileFGJsonGates::isFileNameExtension( extension ) == true )
        return FGJSONGATES_FORMAT;
    if ( FileGatingML::isFileNameExtension( extension ) == true )
        return GATINGML_FORMAT;
    if ( FileFGJsonParameterMap::isFileNameExtension( extension ) == true )
        return FGJSONPARAMETERMAP_FORMAT;
    return UNKNOWN_DATA_FORMAT;
}

/**
 * Returns the text format enum for the given file name extension.
 *
 * @param[in] extension
 *   The file name extension.
 *
 * @return
 *   Returns the file format enum.
 */
TextFileFormat findTextFileFormat( std::string extension )
    noexcept
{
    // Convert the extension to lower case.
    std::transform(
        extension.begin( ),
        extension.end( ),
        extension.begin( ),
        [](unsigned char c){ return std::tolower(c); } );

    if ( extension == "text" || extension == "txt" )
        return TEXT_FORMAT;
    if ( extension == "json" )
        return JSON_FORMAT;

    return UNKNOWN_TEXT_FORMAT;
}

/**
 * Returns the filename extension of the given file.
 *
 * @param[in] path
 *   The file path.
 *
 * @return
 *   Returns the file extension, or an empty string if there is none.
 *
 * @todo File extension extraction may not be Unicode-safe.
 */
std::string getFileExtension( const std::string path )
{
    const size_t dotPosition = path.rfind( '.' );
    if ( dotPosition == std::string::npos )
        return std::string( );

    return path.substr( dotPosition + 1 );
}

/**
 * Creates a list of known file name extensions.
 *
 * @return
 *   Returns a string containing a comma-separated list of extensions.
 */
std::string getDataFileFormatExtensionList( )
    noexcept
{
    std::vector<std::string> list;
    std::string result;

    for ( size_t f = 0; f < 8; ++f )
    {
        switch ( f )
        {
        case 0: list = FileFCS::getFileNameExtensions( ); break;
        case 1: list = FileFGBinaryEvents::getFileNameExtensions( ); break;
        case 2: list = FileFGTextEvents::getFileNameExtensions( ); break;
        case 3: list = FileFGGatingCache::getFileNameExtensions( ); break;
        case 4: list = FileFGTextGates::getFileNameExtensions( ); break;
        case 5: list = FileFGJsonGates::getFileNameExtensions( ); break;
        case 6: list = FileGatingML::getFileNameExtensions( ); break;
        case 7: list = FileFGJsonParameterMap::getFileNameExtensions( ); break;
        }

        for ( size_t i = 0; i < list.size( ); ++i )
        {
            if ( result.empty( ) == true )
                result = list[i];
            else
                result += ", " + list[i];
        }
    }

    return result;
}

/**
 * Creates a list of known text file name extensions.
 *
 * @return
 *   Returns a string containing a comma-separated list of extensions.
 */
std::string getTextFileFormatExtensionList( )
    noexcept
{
    return "text, txt, json";
}





//----------------------------------------------------------------------
//
// Benchmark utilities.
//
//----------------------------------------------------------------------
/**
 * Returns the real time, in seconds, or -1.0 if an error occurred.
 *
 * Time is measured since an arbitrary and OS-dependent start time.
 * The returned real time is only useful for computing an elapsed time
 * between two calls to this function.
 *
 * @return
 *   Returns the real time, in seconds.
 */
double getRealTime( )
    noexcept
{
#ifdef BENCHMARK_TIME_AVAILABLE
#if defined(_WIN32)
	FILETIME tm;
	ULONGLONG t;
#if defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8
	/* Windows 8, Windows Server 2012 and later. ---------------- */
	GetSystemTimePreciseAsFileTime( &tm );
#else
	/* Windows 2000 and later. ---------------------------------- */
	GetSystemTimeAsFileTime( &tm );
#endif
	t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
	return (double)t / 10000000.0;

#elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))
	/* HP-UX, Solaris. ------------------------------------------ */
	return (double)gethrtime( ) / 1000000000.0;

#elif defined(_POSIX_VERSION)
	/* POSIX. --------------------------------------------------- */
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
	{
		struct timespec ts;
#if defined(CLOCK_MONOTONIC_PRECISE)
		/* BSD. --------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_PRECISE;
#elif defined(CLOCK_MONOTONIC_RAW)
		/* Linux. ------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_HIGHRES)
		/* Solaris. ----------------------------------------- */
		const clockid_t id = CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
		/* AIX, BSD, Linux, POSIX, Solaris. ----------------- */
		const clockid_t id = CLOCK_MONOTONIC;
#elif defined(CLOCK_REALTIME)
		/* AIX, BSD, HP-UX, Linux, POSIX. ------------------- */
		const clockid_t id = CLOCK_REALTIME;
#else
		const clockid_t id = (clockid_t)-1;	/* Unknown. */
#endif /* CLOCK_* */
		if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
			return (double)ts.tv_sec +
				(double)ts.tv_nsec / 1000000000.0;
		/* Fall thru. */
	}
#endif /* _POSIX_TIMERS */

	/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, POSIX, Solaris. ----- */
	struct timeval tm;
	gettimeofday( &tm, NULL );
	return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#else
	return -1.0;		// Failed. Unrecognized OS.
#endif

#else
	return -1.0;		// Failed. Benchmark time not available.
#endif
}





//----------------------------------------------------------------------
//
// Message utilities.
//
//----------------------------------------------------------------------
/**
 * Prints an error message and exits the application.
 *
 * @param[in] message
 *   The message to print before exiting.
 */
void printErrorAndExit( const std::string& message )
    noexcept
{
    std::cerr << application.applicationName << ": " << message << std::endl;
    std::cerr << "Use --help for a list of options and how to use them." << std::endl;
    std::exit( 1 );
}

/**
 * Prints a warning message.
 *
 * @param[in] message
 *   The message to print.
 */
void printWarning( const std::string& message )
    noexcept
{
    std::cerr << application.applicationName << ": " << message << std::endl;
}

/**
 * Prints a benchmark time.
 *
 * @param[in] title
 *   The benchmark value title.
 * @param[in] time
 *   The real time delta, in seconds.
 * @param[in] islast
 *   (optional, default = false) Whether the item is the last benchmark.
 *   This affects JSON output only.
 */
void printBenchmark(
    const std::string& title,
    const double time,
    const bool islast = false )
    noexcept
{
    switch ( application.displayFormat )
    {
        case JSON_FORMAT:
            std::cout <<
                "  \"" << title << "\": " <<
                std::fixed << std::setprecision(8) <<
                time <<
                ((islast == false) ? "," : "") <<
                std::endl;
            break;

        default:
        case TEXT_FORMAT:
            std::cout << std::left << std::setw(10) <<
                title << " " <<
                std::right << std::fixed << std::setprecision(8) <<
                time << " sec" <<
                std::endl;
            break;
    }
}

/**
 * Prints the file log.
 *
 * @param[in] log
 *   The file log.
 */
void printFileLog( const std::vector<std::pair<std::string,std::string>>& log )
    noexcept
{
    const size_t n = log.size( );
    if ( n == 0 )
        return;

    for ( size_t i = 0; i < n; ++i )
    {
        auto category = log[i].first;
        auto message  = log[i].second;
        std::cerr << category << ":" << std::endl;
        std::cerr << "  " << message << std::endl << std::endl;
    }
}

/**
 * Prints the application's usage and exits.
 *
 * @param[in] message
 *   The message to print prior to usage information.
 */
void printUsageAndExit( const std::string& message )
    noexcept
{
    // (Optional) Error message
    // ------------------------
    if ( !message.empty( ) )
    {
        std::cerr << application.applicationName << ": " << message << std::endl;
        std::cerr << std::endl;
    }


    // Command line
    // ------------
    std::cerr << "Usage is: " << application.applicationName <<
        " [options] sourcefile [supportfile] destinationfile" << std::endl;
    std::cerr << "Convert between Flow Cytometry file formats." << std::endl;


    // Options
    // -------
    // Defien a few formatting macros, then print help.
#define LISTOPTIONGROUP(title) \
    std::cerr << std::endl << std::left << std::setw(20) << \
    title << std::endl;

#define LISTOPTION(title,explain) \
    std::cerr << "  " << std::left << std::setw(25) << title << \
    explain << std::endl;


    LISTOPTIONGROUP( "Help:" );
    LISTOPTION( "--help",
        "Show this help message." );
    LISTOPTION( "--showwarnings",
        "Show warning messages." );
    LISTOPTION( "--verbose",
        "Announce each processing step." );
    LISTOPTION( "--version",
        "Show short-form version information." );
    LISTOPTION( "--versionlong",
        "Show long-form version information." );

    LISTOPTIONGROUP( "Processing control:" );
    LISTOPTION( "--benchmark",
        "Report the time for each step." );
    LISTOPTION( "--threads N",
        "Use up to N threads." );

    LISTOPTIONGROUP( "Processing:" );
    LISTOPTION( "--compensate",
        "Compensate for spillover between parameters (FCS files only)." );
    LISTOPTION( "--deidentify",
        "Remove personal information." );

    LISTOPTIONGROUP( "Format choice:" );
    LISTOPTION( "--format FORMAT",
        "Specify next file's format." );

    LISTOPTIONGROUP( "Input:" );
    LISTOPTION( "--maxevents N",
        "Maximum number of events to load." );

    LISTOPTIONGROUP( "Gate tree and parameter map edits:" );
    LISTOPTION( "--setdescription TEXT",
        "Set description." );
    LISTOPTION( "--setname TEXT",
        "Set name." );


    // Description
    // -----------
    std::cerr << std::endl;

    std::cerr <<
        "Supported file formats:" <<
        std::endl <<
        "  Standards:" <<
        std::endl <<
        "    FCS, versions 1.0, 2.0, 3.0, & 3.1" <<
        std::endl <<
        "    Gating-ML, versions 1.5 & 2.0" <<
        std::endl <<
        "  Flow Gate project:" <<
        std::endl <<
        "    FG binary events, post-compensation" <<
        std::endl <<
        "    FG JSON gates" <<
        std::endl <<
        "    FG JSON parameter map" <<
        std::endl <<
        "    FG gate cache, post-transform, pre- & post-gating" <<
        std::endl <<
        "  Flow Gate project (legacy):" <<
        std::endl <<
        "    FG text events, post-compensation, tab-separated values" <<
        std::endl <<
        "    FG text gates, tab-separated values" <<
        std::endl << std::endl;

    std::cerr <<
        "A file's format is inferred from file name extension (e.g. 'file.fcs'" <<
        std::endl <<
        "is an FCS file). This inference can be overridden with '--format FORMAT'" <<
        std::endl <<
        "before the file name." <<
        std::endl <<
        "  Input formats: " <<
        getDataFileFormatExtensionList( ) <<
        std::endl <<
        "  Output formats: " <<
        getTextFileFormatExtensionList( ) <<
        std::endl << std::endl;

    std::cerr <<
        "Use '--deidentify' to remove personal information. For FCS files, this removes" <<
        std::endl <<
        "dictionary entries that contain names, dates, and other possible personal" <<
        std::endl <<
        "information. For Gating-ML and JSON gating files, this removes all diagnostic" <<
        std::endl <<
        "notes and any reference to a specific FCS file." <<
        std::endl <<
        std::endl <<
        "For FCS files only, use '--compensate' to compensate values using the file's" <<
        std::endl <<
        "spillover matrix." <<
        std::endl << std::endl;

    std::cerr <<
        "Examples:" <<
        std::endl;
    std::cerr <<
        "  De-identify an FCS file:" <<
        std::endl <<
        "    " << application.applicationName << " in.fcs --deidentify out.fcs" <<
        std::endl << std::endl;

    std::cerr <<
        "  De-identify a Gating-ML file:" <<
        std::endl <<
        "    " << application.applicationName << " in.gml --deidentify out.gml" <<
        std::endl << std::endl;

    std::cerr <<
        "  Compensate and truncate an FCS file into a binary event file:" <<
        std::endl <<
        "    " << application.applicationName << " in.fcs --deidentify --compensate --maxevents 10000 out.fgb" <<

        std::endl << std::endl;

    std::cerr <<
        "  Convert a Gating-ML file to a JSON gate file:" <<
        std::endl <<
        "    " << application.applicationName << " gates.xml gates.json" <<
        std::endl << std::endl;

    std::cerr <<
        "  Combine event and gate data into a gating cache file:" <<
        std::endl <<
        "    " << application.applicationName << " events.fgb gates.xml cache.cache" <<
        std::endl << std::endl;

    std::exit( 1 );
}

/**
 * Prints version information and exits the application.
 *
 * @param[in] longForm
 *   When true, show long-form version information that also includes the
 *   author(s), copyright, and license for each item.
 */
void printVersionAndExit( const bool longForm = false )
    noexcept
{
    const std::string packageGroupIndent  = "  ";
    const std::string packageIndent       = "    ";
    const std::string packageDetailIndent = "      ";

    // Application.
    std::cerr << application.applicationName << " " <<
        application.applicationVersion <<
        " (built " << application.applicationBuildDate << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            application.applicationCredit << std::endl;
        std::cerr << packageDetailIndent <<
            application.applicationCopyright << std::endl;
        std::cerr << packageDetailIndent <<
            application.applicationLicense << std::endl;
    }

    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Data models:" << std::endl;

    // EventTable.
    std::cerr << packageIndent << EventTable::NAME << " " <<
        EventTable::VERSION <<
        " (built " << EventTable::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            EventTable::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            EventTable::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            EventTable::LICENSE << std::endl;
    }

    // GateTrees.
    std::cerr << packageIndent << GateTrees::NAME << " " <<
        GateTrees::VERSION <<
        " (built " << GateTrees::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            GateTrees::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            GateTrees::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            GateTrees::LICENSE << std::endl;
    }

    // ParameterMap.
    std::cerr << packageIndent << ParameterMap::NAME << " " <<
        ParameterMap::VERSION <<
        " (built " << ParameterMap::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            ParameterMap::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            ParameterMap::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            ParameterMap::LICENSE << std::endl;
    }

    // GatingCache.
    std::cerr << packageIndent << GatingCache::NAME << " " <<
        GatingCache::VERSION <<
        " (built " << GatingCache::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            GatingCache::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            GatingCache::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            GatingCache::LICENSE << std::endl;
    }


    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Event files:" << std::endl;

    // FileFCS.
    std::cerr << packageIndent << FileFCS::NAME << " " <<
        FileFCS::VERSION <<
        " (built " << FileFCS::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFCS::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFCS::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFCS::LICENSE << std::endl;
    }

    // FileFGBinaryEvents.
    std::cerr << packageIndent << FileFGBinaryEvents::NAME << " " <<
        FileFGBinaryEvents::VERSION <<
        " (built " << FileFGBinaryEvents::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFGBinaryEvents::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGBinaryEvents::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGBinaryEvents::LICENSE << std::endl;
    }

    // FileFGTextEvents.
    std::cerr << packageIndent << FileFGTextEvents::NAME << " " <<
        FileFGTextEvents::VERSION <<
        " (built " << FileFGTextEvents::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFGTextEvents::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGTextEvents::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGTextEvents::LICENSE << std::endl;
    }


    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Gating Cache files:" << std::endl;

    // FileFGGatingCache.
    std::cerr << packageIndent << FileFGGatingCache::NAME << " " <<
        FileFGGatingCache::VERSION <<
        " (built " << FileFGGatingCache::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFGGatingCache::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGGatingCache::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGGatingCache::LICENSE << std::endl;
    }


    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Gate files:" << std::endl;

    // FileGatingML.
    std::cerr << packageIndent << FileGatingML::NAME << " " <<
        FileGatingML::VERSION <<
        " (built " << FileGatingML::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileGatingML::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileGatingML::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileGatingML::LICENSE << std::endl;
    }

    // FileFGJsonGates.
    std::cerr << packageIndent << FileFGJsonGates::NAME << " " <<
        FileFGJsonGates::VERSION <<
        " (built " << FileFGJsonGates::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFGJsonGates::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGJsonGates::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGJsonGates::LICENSE << std::endl;
    }

    // FileFGTextGates.
    std::cerr << packageIndent << FileFGTextGates::NAME << " " <<
        FileFGTextGates::VERSION <<
        " (built " << FileFGTextGates::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFGTextGates::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGTextGates::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGTextGates::LICENSE << std::endl;
    }

    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Parameter map files:" << std::endl;

    // FileFGJsonParameterMap.
    std::cerr << packageIndent << FileFGJsonParameterMap::NAME << " " <<
        FileFGJsonParameterMap::VERSION <<
        " (built " << FileFGJsonParameterMap::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            FileFGJsonParameterMap::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGJsonParameterMap::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            FileFGJsonParameterMap::LICENSE << std::endl;
    }

    std::exit( 1 );
}



//----------------------------------------------------------------------
//
// Argument parsing.
//
//----------------------------------------------------------------------
/**
 * Parses arguments into globals.
 *
 * On an error, the function prints a message and exits the application.
 *
 * @param[in] argc
 *   The number of arguments.
 * @param[in] argv
 *   The arguments.
 */
void parseArguments( int argc, char** argv )
    noexcept
{
    // Save the application path
    // -------------------------
    application.applicationPath = std::string( argv[0] );
    unsigned int lastSlash = application.applicationPath.find_last_of( "/\\" );
    if ( lastSlash == std::string::npos )
        application.applicationName = application.applicationPath;
    else
        application.applicationName = application.applicationPath.substr( lastSlash + 1 );
    --argc;
    ++argv;

    application.sourcePath.clear( );


    // Parse options and file names
    // ----------------------------
    std::string pendingFormat = "";

    while ( argc != 0 )
    {
        if ( argv[0][0] == '-' )
        {
            // Skip past one or two leading '-'s.
            const char*const option = (argv[0][1] == '-') ?
                &argv[0][2] : &argv[0][1];

            //
            // Help & verbosity.
            //
            if ( std::strcmp( "help", option ) == 0 )
            {
                printUsageAndExit( "" );
            }
            if ( std::strcmp( "version", option ) == 0 )
            {
                printVersionAndExit( false );
            }
            if ( std::strcmp( "versionlong", option ) == 0 )
            {
                printVersionAndExit( true );
            }
            if ( std::strcmp( "verbose", option ) == 0 )
            {
                application.verbose = true;
                --argc;
                ++argv;
                continue;
            }
            if ( std::strcmp( "showwarnings", option ) == 0 )
            {
                application.showWarnings = true;
                --argc;
                ++argv;
                continue;
            }

            //
            // Generic.
            //
            if ( std::strcmp( "benchmark", option ) == 0 )
            {
                application.benchmark = true;
                --argc;
                ++argv;
                continue;
            }
            if ( std::strcmp( "threads", option ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing number of threads choice after " ) +
                        argv[0] + ".\n" );

                application.numberOfThreads = atol(argv[1]);
                if ( application.numberOfThreads < 0 )
                    printErrorAndExit(
                        "Number of threads choice must non-negative.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }

            //
            // Format.
            //
            if ( std::strcmp( "format", option ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing file format choice after " ) +
                        argv[0] + ".\n" );

                pendingFormat = argv[1];
                argc -= 2;
                argv += 2;
                continue;
            }


            //
            // Input.
            //
            if ( std::strcmp( "maxevents", option ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing maximum events choice after " ) +
                        argv[0] + ".\n" );

                application.maxEvents = atol(argv[1]);
                if ( application.maxEvents < (-1) )
                    printErrorAndExit(
                        "Maximum events choice must -1, 0, or a positive integer.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }

            //
            // Processing.
            //
            if ( std::strcmp( "compensate", option ) == 0 )
            {
                application.compensate = true;
                --argc;
                ++argv;
                continue;
            }
            if ( std::strcmp( "deidentify", option ) == 0 )
            {
                application.deidentify = true;
                --argc;
                ++argv;
                continue;
            }

            //
            // Edits.
            //
            if ( std::strcmp( "setdescription", option ) == 0 ||
                 std::strcmp( "setname", option ) == 0 ||
                 std::strcmp( "setgatetreedescription", option ) == 0 ||
                 std::strcmp( "setgatetreename", option ) == 0)
            {
                // --KEY VALUE
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing value after " ) +
                        argv[0] + ".\n" );

                application.edits[option] = std::make_tuple(
                    "",
                    "",
                    argv[1] );
                argc -= 2;
                argv += 2;
                continue;
            }

            // Unknown.
            printErrorAndExit(
                std::string( "Unknown option: '" ) +
                argv[0] + "'.\n" );
        }

        if ( application.sourcePath.empty( ) == true )
        {
            application.sourcePath = argv[0];
            application.sourceFormat = UNKNOWN_DATA_FORMAT;
            --argc;
            ++argv;

            // Use the pending format choice, if any.
            if ( pendingFormat.empty( ) == false )
            {
                application.sourceFormat = findDataFileFormat( pendingFormat );
                if ( application.sourceFormat == UNKNOWN_DATA_FORMAT )
                    printErrorAndExit(
                        std::string( "Unknown file format name: " ) +
                        pendingFormat + "\n" );

                pendingFormat.clear( );
            }
            continue;
        }

        if ( application.supportPath.empty( ) == true )
        {
            application.supportPath = argv[0];
            application.supportFormat = UNKNOWN_DATA_FORMAT;
            --argc;
            ++argv;

            // Use the pending format choice, if any.
            if ( pendingFormat.empty( ) == false )
            {
                application.supportFormat = findDataFileFormat( pendingFormat );
                if ( application.supportFormat == UNKNOWN_DATA_FORMAT )
                    printErrorAndExit(
                        std::string( "Unknown file format name: " ) +
                        pendingFormat + "\n" );

                pendingFormat.clear( );
            }
            continue;
        }

        if ( application.destinationPath.empty( ) == true )
        {
            application.destinationPath = argv[0];
            application.destinationFormat = UNKNOWN_DATA_FORMAT;
            --argc;
            ++argv;

            // Use the pending format choice, if any.
            if ( pendingFormat.empty( ) == false )
            {
                application.destinationFormat = findDataFileFormat( pendingFormat );
                if ( application.supportFormat == UNKNOWN_DATA_FORMAT )
                    printErrorAndExit(
                        std::string( "Unknown file format name: " ) +
                        pendingFormat + "\n" );

                pendingFormat.clear( );
            }
            continue;
        }

        // Too many files.
        printErrorAndExit(
            std::string( "Too many files." ) );
    }

    // Check that we got essential arguments.
    if ( application.sourcePath.empty( ) == true )
        printErrorAndExit(
            std::string( "An event or gate file to convert is required.\n" ) );

    if ( application.destinationPath.empty( ) == true )
    {
        if (application.supportPath.empty( ) == true )
            printErrorAndExit(
                std::string( "An output event or gate file is required.\n" ) );

        // The second file was assigned as a support file. Make it
        // the destination file instead.
        application.destinationPath = application.supportPath;
        application.destinationFormat = application.supportFormat;
        application.supportPath.clear( );
        application.supportFormat = UNKNOWN_DATA_FORMAT;
    }

    application.displayFormat = TEXT_FORMAT;
    if ( pendingFormat.empty( ) == false )
    {
        application.displayFormat =
            findTextFileFormat( pendingFormat );
        if ( application.displayFormat == UNKNOWN_TEXT_FORMAT )
            printErrorAndExit(
                std::string( "Unknown file format name: " ) +
                pendingFormat + "\n" );
    }
}





//----------------------------------------------------------------------
//
// Edits and conversions.
//
//----------------------------------------------------------------------
/**
 * Applies gate tree edits set on the command-line.
 *
 * @param gateTrees
 *   The gate trees being edited.
 */
void applyEdits(
    std::shared_ptr<GateTrees> gateTrees )
    noexcept
{
    if ( application.edits.size( ) == 0 )
        return;

    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ": Editing gate tree.\n";

    for ( auto& entry : application.edits )
    {
        const auto key    = entry.first;
        const auto value1 = std::get<0>( entry.second );
        const auto value2 = std::get<1>( entry.second );
        const auto value3 = std::get<2>( entry.second );

        //
        // Gate tree edits.
        // - All of these edits have a single value.
        //
        if ( key.compare( "setdescription" ) == 0 ||
             key.compare( "setgatetreedescription" ) == 0 )
        {
            // TUPLE ( *, *, DESCRIPTION )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate tree description to \"" << value3 << "\".\n";
            gateTrees->setDescription( value3 );
            continue;
        }
        if ( key.compare( "setname" ) == 0 ||
             key.compare( "setgatetreename" ) == 0 )
        {
            // TUPLE ( *, *, NAME )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate tree name to \"" << value3 << "\".\n";
            gateTrees->setName( value3 );
            continue;
        }
    }
}

/**
 * Applies parameter map edits set on the command-line.
 *
 * @param parameterMap
 *   The parameter map being edited.
 */
void applyEdits(
    std::shared_ptr<ParameterMap> parameterMap )
    noexcept
{
    if ( application.edits.size( ) == 0 )
        return;

    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ": Editing parameter map.\n";

    for ( auto& entry : application.edits )
    {
        const auto key    = entry.first;
        const auto value1 = std::get<0>( entry.second );
        const auto value2 = std::get<1>( entry.second );
        const auto value3 = std::get<2>( entry.second );

        //
        // Gate tree edits.
        // - All of these edits have a single value.
        //
        if ( key.compare( "setdescription" ) == 0 )
        {
            // TUPLE ( *, *, DESCRIPTION )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set parameter map description to \"" << value3 << "\".\n";
            parameterMap->setDescription( value3 );
            continue;
        }
        if ( key.compare( "setname" ) == 0 )
        {
            // TUPLE ( *, *, NAME )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set parameter map name to \"" << value3 << "\".\n";
            parameterMap->setName( value3 );
            continue;
        }
    }
}

/**
 * Converts the source event file to a destination event file.
 */
void convertEventToEvent( )
    noexcept
{
    // Load event file and get the event table.
    std::shared_ptr<EventTableInterface> eventTable;
    FileFCS* fcsFile = nullptr;

    const double timeAtStartOfLoad = getRealTime( );

    switch ( application.sourceFormat )
    {
        case FCS_FORMAT:
        {
            auto file = new FileFCS( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );
            file->setAutoScaling( true );

            try
            {
                file->load( application.sourcePath, application.maxEvents );
                if ( application.deidentify == true )
                    file->deidentify( );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );

                if ( application.compensate == true &&
                     file->isCompensationRequired( ) == true )
                {
                    std::vector<std::string> matrixParameterNames;
                    std::vector<double> matrix;
                    file->getSpilloverMatrix( matrixParameterNames, matrix );

                    if ( matrixParameterNames.empty( ) == true )
                        std::cerr << application.applicationName << ": " <<
                            "Compensation cannot be done." <<
                            std::endl <<
                            "The file does not include a spillover matrix." <<
                            std::endl;
                    else
                    {
                        eventTable->compensate( matrixParameterNames, matrix );
                    }
                }

                fcsFile = file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGBINARYEVENTS_FORMAT:
        {
            auto file = new FileFGBinaryEvents( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath, application.maxEvents );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGTEXTEVENTS_FORMAT:
        {
            auto file = new FileFGTextEvents( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath, application.maxEvents );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        default:
            return;
    }

    eventTable->setVerbose( application.verbose );
    eventTable->setVerbosePrefix( application.applicationName );

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );

    // Save event file.
    const double timeAtStartOfSave = getRealTime( );

    switch ( application.destinationFormat )
    {
        case FCS_FORMAT:
        {
            // If the input file was an FCS file, re-use its FileFCS object,
            // and its dictionary of keyword-value pairs to propagate into
            // the output file.
            if ( fcsFile == nullptr )
            {
                fcsFile = new FileFCS( eventTable );
                fcsFile->setVerbose( application.verbose );
                fcsFile->setVerbosePrefix( application.applicationName );
            }
            else
                fcsFile->clearFileLog( );

            try
            {
                fcsFile->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( fcsFile->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( fcsFile != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( fcsFile->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGBINARYEVENTS_FORMAT:
        {
            auto file = new FileFGBinaryEvents( eventTable );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        case FGTEXTEVENTS_FORMAT:
        {
            auto file = new FileFGTextEvents( eventTable );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        default:
            return;
    }

    if ( fcsFile != nullptr )
        delete fcsFile;

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true );
}

/**
 * Converts the source parameter map file to a destination parameter map file.
 */
void convertParameterMapToParameterMap( )
    noexcept
{
    // Load parameter file and get the parameter map.
    std::shared_ptr<ParameterMap> parameterMap;

    const double timeAtStartOfLoad = getRealTime( );

    switch ( application.sourceFormat )
    {
        case FGJSONPARAMETERMAP_FORMAT:
        {
            auto file = new FileFGJsonParameterMap( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                parameterMap = file->getParameterMap( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        default:
            return;
    }

    parameterMap->setVerbose( application.verbose );
    parameterMap->setVerbosePrefix( application.applicationName );

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );

    // Edit.
    applyEdits( parameterMap );

    // Save event file.
    const double timeAtStartOfSave = getRealTime( );

    switch ( application.destinationFormat )
    {
        case FGJSONPARAMETERMAP_FORMAT:
        {
            auto file = new FileFGJsonParameterMap( parameterMap );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        default:
            return;
    }

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true );
}

/**
 * Converts the source gate tree file to a destination gate tree file.
 */
void convertGateToGate( )
    noexcept
{
    // Load gate file and get the gate tree.
    std::shared_ptr<GateTrees> gateTrees;

    const double timeAtStartOfLoad = getRealTime( );

    switch ( application.sourceFormat )
    {
        case GATINGML_FORMAT:
        {
            auto file = new FileGatingML( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGTEXTGATES_FORMAT:
        {
            auto file = new FileFGTextGates( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGJSONGATES_FORMAT:
        {
            auto file = new FileFGJsonGates( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        default:
            return;
    }

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );

    // Edit.
    applyEdits( gateTrees );

    // De-identify.
    if ( application.deidentify == true )
        gateTrees->deidentify( );

    // Save gate file.
    const double timeAtStartOfSave = getRealTime( );

    switch ( application.destinationFormat )
    {
        case GATINGML_FORMAT:
        {
            auto file = new FileGatingML( gateTrees );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        case FGJSONGATES_FORMAT:
        {
            auto file = new FileFGJsonGates( gateTrees );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        default:
            return;
    }

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true );
}

/**
 * Converts the source cache file to a destination cache file.
 */
void convertCacheToCache( )
    noexcept
{
    // Load cache file.
    const double timeAtStartOfLoad = getRealTime( );

    auto file = new FileFGGatingCache( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        file->load( application.sourcePath, true );
        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true ) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    file->clearFileLog( );

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );

    // Save cache file. Re-use the same file object and the same
    // gating cache within that object.
    const double timeAtStartOfSave = getRealTime( );

    try
    {
        file->save( application.destinationPath );
        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true ) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }
    delete file;

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true );
}

/**
 * Converts the source cache file to a destination gate tree file.
 */
void convertCacheToGate( )
    noexcept
{
    // Load cache file and get the gate tree.
    const double timeAtStartOfLoad = getRealTime( );

    auto file = new FileFGGatingCache( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        file->load( application.sourcePath, true );
        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true ) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    file->clearFileLog( );

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );

    // Edit.
    std::shared_ptr<GateTrees> gateTrees = file->getGateTrees( );
    applyEdits( gateTrees );

    // De-identify.
    if ( application.deidentify == true )
        gateTrees->deidentify( );

    // Save gate file.
    const double timeAtStartOfSave = getRealTime( );

    switch ( application.destinationFormat )
    {
        case GATINGML_FORMAT:
        {
            auto file = new FileGatingML( gateTrees );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        case FGJSONGATES_FORMAT:
        {
            auto file = new FileFGJsonGates( gateTrees );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->save( application.destinationPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        default:
            return;
    }

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true );
}


/**
 * Converts the source event file and support gate trees file
 * to a destination cache file.
 */
void convertEventAndGateToCache( )
    noexcept
{
    // Load event file and get the event table.
    std::shared_ptr<EventTableInterface> eventTable;

    const double timeAtStartOfLoad = getRealTime( );

    switch ( application.sourceFormat )
    {
        case FCS_FORMAT:
        {
            auto file = new FileFCS( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );
            file->setAutoScaling( true );

            try
            {
                file->load( application.sourcePath, application.maxEvents );
                if ( application.deidentify == true )
                    file->deidentify( );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );

                if ( application.compensate == true &&
                     file->isCompensationRequired( ) == true )
                {
                    std::vector<std::string> matrixParameterNames;
                    std::vector<double> matrix;
                    file->getSpilloverMatrix( matrixParameterNames, matrix );

                    if ( matrixParameterNames.empty( ) == true )
                        std::cerr << application.applicationName << ": " <<
                            "Compensation cannot be done." <<
                            std::endl <<
                            "The file does not include a spillover matrix." <<
                            std::endl;
                    else
                    {
                        eventTable->compensate( matrixParameterNames, matrix );
                    }

                }
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGBINARYEVENTS_FORMAT:
        {
            auto file = new FileFGBinaryEvents( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath, application.maxEvents );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGTEXTEVENTS_FORMAT:
        {
            auto file = new FileFGTextEvents( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.sourcePath, application.maxEvents );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        default:
            return;
    }

    eventTable->setVerbose( application.verbose );
    eventTable->setVerbosePrefix( application.applicationName );

    // Load gate file and get the gate tree.
    std::shared_ptr<GateTrees> gateTrees;

    switch ( application.supportFormat )
    {
        case GATINGML_FORMAT:
        {
            auto file = new FileGatingML( );
            try
            {
                file->setVerbose( application.verbose );
                file->setVerbosePrefix( application.applicationName );
                file->load( application.supportPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGTEXTGATES_FORMAT:
        {
            auto file = new FileFGTextGates( );
            try
            {
                file->setVerbose( application.verbose );
                file->setVerbosePrefix( application.applicationName );
                file->load( application.supportPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        case FGJSONGATES_FORMAT:
        {
            auto file = new FileFGJsonGates( );
            try
            {
                file->setVerbose( application.verbose );
                file->setVerbosePrefix( application.applicationName );
                file->load( application.supportPath );
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( file != nullptr &&
                     (application.verbose == true ||
                     application.showWarnings == true ) )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        default:
            return;
    }

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );

    // Edit.
    applyEdits( gateTrees );

    // Save cache file.
    const double timeAtStartOfSave = getRealTime( );

    std::shared_ptr<GatingCache> cache;
    try
    {
        cache.reset( new GatingCache( eventTable, gateTrees ) );
    }
    catch ( const std::exception& e )
    {
        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    auto file = new FileFGGatingCache( cache );

    try
    {
        file->save( application.destinationPath );
        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true ) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }
    delete file;

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true );
}





//----------------------------------------------------------------------
//
// Main.
//
//----------------------------------------------------------------------
/**
 * Runs the application.
 *
 * @param[in] argc
 *   The number of arguments.
 * @param[in] argv
 *   The arguments.
 *
 * @return
 *   Returns the exit status. By POSIX convention, a returned zero indicates
 *   the application completed successfully. All other values indicate an
 *   error occurred.
 */
int main( int argc, char** argv )
{
    //
    // Parse arguments.
    // ----------------
    // Parse command-line arguments to get the input and output file
    // and options on how to use them.
    parseArguments( argc, argv );


    //
    // Set the number of threads.
    // --------------------------
    // If OpenMP is enabled, set the number of threads to use to whatever
    // has been set on the command line.
#ifdef _OPENMP
    if ( application.numberOfThreads != 0 )
        omp_set_num_threads( application.numberOfThreads );
#endif


    //
    // Determine file foramts.
    // -----------------------
    // Use the file name extension, or command-line arguments, to determine
    // the input and output formats.
    if ( application.sourceFormat == UNKNOWN_DATA_FORMAT )
    {
        const std::string ext = getFileExtension( application.sourcePath );
        if ( ext.empty( ) == true )
            printErrorAndExit(
                std::string( "\"" ) + application.sourcePath +
                "\" has no file name extension.\n" +
                "Use --format to specify the file format.\n" );

        application.sourceFormat = findDataFileFormat( ext );
        if ( application.sourceFormat == UNKNOWN_DATA_FORMAT )
            printErrorAndExit(
                std::string( "\"" ) + application.sourcePath +
                "\" file name extension is not recognized.\n" +
                "Use --format to specify the file format.\n" );
    }

    if ( application.supportPath.empty( ) == false &&
         application.supportFormat == UNKNOWN_DATA_FORMAT )
    {
        const std::string ext = getFileExtension( application.supportPath );
        if ( ext.empty( ) == true )
            printErrorAndExit(
                std::string( "\"" ) + application.supportPath +
                "\" has no file name extension.\n" +
                "Use --format to specify the file format.\n" );

        application.supportFormat = findDataFileFormat( ext );
        if ( application.supportFormat == UNKNOWN_DATA_FORMAT )
            printErrorAndExit(
                std::string( "\"" ) + application.supportPath +
                "\" file name extension is not recognized.\n" +
                "Use --format to specify the file format.\n" );
    }

    if ( application.destinationFormat == UNKNOWN_DATA_FORMAT )
    {
        const std::string ext = getFileExtension( application.destinationPath );
        if ( ext.empty( ) == true )
            printErrorAndExit(
                std::string( "\"" ) + application.destinationPath +
                "\" has no file name extension.\n" +
                "Use --format to specify the file format.\n" );

        application.destinationFormat = findDataFileFormat( ext );
        if ( application.destinationFormat == UNKNOWN_DATA_FORMAT )
            printErrorAndExit(
                std::string( "\"" ) + application.destinationPath +
                "\" file name extension is not recognized.\n" +
                "Use --format to specify the file format.\n" );
    }


    //
    // Validate matching conversions.
    // ------------------------------
    // The following forms are supported:
    //
    // - Event file to Event file.
    // - Gate file to Gate file.
    // - Cache file to Cache file.
    // - Event & Gate files to Cache file.
    // - Cache file to Gate file.
#define EVENT_TO_EVENT 0
#define GATE_TO_GATE 1
#define CACHE_TO_CACHE 2
#define EVENT_AND_GATE_TO_CACHE 3
#define CACHE_TO_GATE 4
#define MAP_TO_MAP 5
    int conversionType = EVENT_TO_EVENT;

    if ( application.supportPath.empty( ) == false )
    {
        // Source, support, and destination files provided. This must be
        // a conversion to a cache file.
        if ( isCacheFile( application.destinationFormat ) == false )
            printErrorAndExit(
                std::string( "Conversion of an event and gate file into a new file requires\n" ) +
                "that the new file be a gating cache file.\n" );

        // Check the format for the source
        // and support files and swap them if needed.
        if ( isGateFile( application.sourceFormat ) &&
             isEventFile( application.supportFormat ) )
        {
            // Swap so that the source is the event file.
            std::string tmpPath = application.sourcePath;
            application.sourcePath = application.supportPath;
            application.supportPath = tmpPath;

            DataFileFormat tmpFormat = application.sourceFormat;
            application.sourceFormat = application.supportFormat;
            application.supportFormat = tmpFormat;
        }

        if ( isEventFile( application.sourceFormat ) == false )
            printErrorAndExit(
                std::string( "Conversion into a gating cache file requires that the first\n" ) +
                "input file be an event file (e.g. FCS).\n" );

        if ( isGateFile( application.supportFormat ) == false )
            printErrorAndExit(
                std::string( "Conversion into a gating cache file requires that the second\n" ) +
                "input file be a gate tree file (e.g. Gating-ML).\n" );

        conversionType = EVENT_AND_GATE_TO_CACHE;
    }
    else if ( isCacheFile( application.sourceFormat ) )
    {
        if ( isGateFile( application.destinationFormat ) )
            conversionType = CACHE_TO_GATE;
        else if ( isCacheFile( application.destinationFormat ) )
            conversionType = CACHE_TO_CACHE;
        else
            printErrorAndExit(
                std::string( "Conversion of a source gating cache file requires that\n" ) + 
                "the destination file be a gating cache or gate tree file.\n" );
    }
    else if ( isEventFile( application.sourceFormat ) )
    {
        if ( !isEventFile( application.destinationFormat ) )
            printErrorAndExit(
                std::string( "Conversion of a source event file requires that the destination\n" ) + 
                "file also be an event file (e.g. FCS to FGB).\n" );
        conversionType = EVENT_TO_EVENT;
    }
    else if ( isGateFile( application.sourceFormat ) )
    {
        if ( !isGateFile( application.destinationFormat ) )
            printErrorAndExit(
                std::string( "Conversion of a source gate tree file requires that the\n" ) + 
                "destination file also be a gate tree file (e.g. Gating-ML to JSON).\n" );

        if ( application.destinationFormat == FGTEXTGATES_FORMAT )
            printErrorAndExit(
                "Conversion to the legacy text gate tree format is not supported.\n" );
        conversionType = GATE_TO_GATE;
    }
    else if ( isParameterMapFile( application.sourceFormat ) )
    {
        if ( !isParameterMapFile( application.destinationFormat ) )
            printErrorAndExit(
                std::string( "Conversion of a source parawmeter map file requires that the\n" ) + 
                "destination file also be a parameter map file.\n" );
        conversionType = MAP_TO_MAP;
    }

    //
    // Convert!
    // --------
    // Call appropriate functions for the conversion.
    if ( application.benchmark == true &&
         application.displayFormat == JSON_FORMAT )
        std::cerr << "{" << std::endl;

    switch ( conversionType )
    {
        case EVENT_TO_EVENT:
            convertEventToEvent( );
            break;

        case GATE_TO_GATE:
            convertGateToGate( );
            break;

        case CACHE_TO_CACHE:
            convertCacheToCache( );
            break;

        case CACHE_TO_GATE:
            convertCacheToGate( );
            break;

        case EVENT_AND_GATE_TO_CACHE:
            convertEventAndGateToCache( );
            break;

        case MAP_TO_MAP:
            convertParameterMapToParameterMap( );
            break;

        default:
            break;
    }

    if ( application.benchmark == true &&
         application.displayFormat == JSON_FORMAT )
        std::cerr << "}" << std::endl;
}
