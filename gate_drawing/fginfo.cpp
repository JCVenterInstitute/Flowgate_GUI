/**
 * @file
 * Displays information about a flow cytometry event or gate file.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "EventTable.h"
 * @li FlowGate "GateTrees.h"
 * @li FlowGate "ParameterMap.h"
 * @li FlowGate "FileFCS.h"
 * @li FlowGate "FileFGBinaryEvents.h"
 * @li FlowGate "FileFGTextEvents.h"
 * @li FlowGate "FileFGGatingCache.h"
 * @li FlowGate "FileFGTextGates.h"
 * @li FlowGate "FileFGJsonGates.h"
 * @li FlowGate "FileFGJsonParameterMap.h"
 * @li FlowGate "FileGatingML.h"
 * @li RapidXML (via "FileGatingML.h")
 * @li Gason (via "FileFGJsonGates.h")
 *
 * @todo Consolidate various file-specific output methods to use a common
 * struct populated from the various file types, then passed to a generic
 * method to output the information.
 *
 * @todo Make text and JSON output contain the same information. They do
 * do not now (e.g. JSON parameter output includes min/max, Text parameter
 * output for FCS includes scaling).
 *
 * @todo Add format name to the start of all output.
 *
 * @todo For all JSON output, escape any double quotes within the output
 * strings (e.g. parameter names, file names, etc.).
 *
 * @todo Remove --saveparameters. The normal output already has parameters.
 *
 * @todo Add output of the long parameter names, such as with an additional
 * field in the JSON output for parameters.
 *
 * @todo Make the nested output of gates tighter and easier to read.
 *
 * @todo For the JSON output of gates, output in JSON gate syntax using
 * the JSON gate writer.
 *
 * @todo Write the JSON output for gating file formats, including Gating-ML,
 * JSON gates, and legacy text gates.
 */
#include <algorithm>    // std::transform
#include <cctype>       // Character typing
#include <fstream>      // std::ofstream, ...
#include <iomanip>      // I/O formatting
#include <iostream>     // std::cerr, ...
#include <map>          // Maps
#include <string>       // std::string, ...
#include <vector>       // Vectors

#include "EventTable.h"         // FlowGate Event Table
#include "GatingCache.h"        // FlowGate Gating Cache
#include "GateTrees.h"          // FlowGate Gate Trees
#include "ParameterMap.h"       // FlowGate Parameter Map

#include "FileFCS.h"            // FCS file format
#include "FileFGBinaryEvents.h" // FlowGate Binary Event file format
#include "FileFGTextEvents.h"   // FlowGate Text Event file format (legacy)
#include "FileFGGatingCache.h"  // FlowGate Gating Cache file format
#include "FileFGTextGates.h"    // FlowGate Text Gate file format (legacy)
#include "FileFGJsonGates.h"    // FlowGate JSON Gate file format
#include "FileFGJsonParameterMap.h" // FlowGate JSON Parameter Map file format
#include "FileGatingML.h"       // Gating-ML file format





//
// Verify suitable compiler.
// -------------------------
// This software is written in C++11 and it uses POSIX-compliant functions.
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
     * The file path for the input file.
     */
    std::string sourcePath;

    /**
     * The input file format.
     */
    DataFileFormat sourceFormat = UNKNOWN_DATA_FORMAT;

    /**
     * The file path for the output dictionary file for FCS files. For other
     * file types, this has no meaning.
     */
    std::string dictionaryPath;

    /**
     * The output dictionary file format.
     */
    TextFileFormat dictionaryFormat = UNKNOWN_TEXT_FORMAT;

    /**
     * The file path for the output parameters list file for event files.
     * For other file types, this has no meaning.
     */
    std::string parametersPath;

    /**
     * The output parameter list file format.
     */
    TextFileFormat parametersFormat = UNKNOWN_TEXT_FORMAT;

    /**
     * The stdout output format.
     */
    TextFileFormat displayFormat = UNKNOWN_TEXT_FORMAT;

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
     * When true, clean parameter keywords out of the dictionary for FCS
     * files. For other file types, this has no meaning.
     */
    bool cleanParameters = false;

    /**
     * Whether to be verbose.
     */
    bool verbose = false;

    /**
     * Whether to show error details.
     */
    bool showWarnings = false;
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

    // Query each of the input file formats we support.
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
 * Creates a list of known data file name extensions.
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
        case 3: list = FileFGTextGates::getFileNameExtensions( ); break;
        case 4: list = FileFGGatingCache::getFileNameExtensions( ); break;
        case 5: list = FileGatingML::getFileNameExtensions( ); break;
        case 6: list = FileFGJsonGates::getFileNameExtensions( ); break;
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
// Message utilities.
//
//----------------------------------------------------------------------
/**
 * Prints an error message and exits the application.
 *
 * @param[in] message
 *   The message to print before exiting.
 */
void printErrorAndExit( const std::string message )
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
void printWarning( const std::string message )
    noexcept
{
    std::cerr << application.applicationName << ": " << message << std::endl;
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
void printUsageAndExit( const std::string message )
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
        " [options] file" << std::endl;
    std::cerr << "Show information about a Flow Cytometry file." << std::endl;


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
    LISTOPTION( "--cleanparameters",
        "Remove internal parameter keywords from dictionary (FCS files only)." );
    LISTOPTION( "--deidentify",
        "Remove personal information." );

    LISTOPTIONGROUP( "Format choice:" );
    LISTOPTION( "--format FORMAT",
        "Specify next file's format." );

    LISTOPTIONGROUP( "Output:" );
    LISTOPTION( "--savedictionary PATH",
        "Save dictionary to a file (FCS files only)" );
    LISTOPTION( "--saveparameters PATH",
        "Save parameters to a file (event files only)" );


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
        "General information is written to the screen as text. Use '--format JSON'" <<
        std::endl <<
        "to output JSON instead." <<
        std::endl << std::endl;

    std::cerr <<
        "Parameter information may be written to a file with '--saveparameters PATH'." <<
        std::endl <<
        "Precede the option with '--format JSON' to save JSON instead." <<
        std::endl << std::endl;

    std::cerr <<
        "Use '--deidentify' to remove personal information. For FCS files, this removes" <<
        std::endl <<
        "dictionary entries that contain names, dates, and other possible personal" <<
        std::endl <<
        "information. For Gating-ML and JSON gating files, this removes all diagnostic" <<
        std::endl <<
        "notes and any reference to a specific FCS file." <<
        std::endl << std::endl;

    std::cerr <<
        "For FCS event files only, the dictionary of keyword-value pairs may be written" <<
        std::endl <<
        "to a file with '--savedictionary PATH'. Use '--cleanparameters' to remove" <<
        std::endl <<
        "internal parameter attributes. Precede --savedictionary with '--format JSON'" <<
        std::endl <<
        "to save JSON to the file." <<
        std::endl << std::endl;

    std::cerr <<
        "Examples:" <<
        std::endl;
    std::cerr <<
        "  Show information about an FCS event file:" <<
        std::endl <<
        "    " << application.applicationName << " events.fcs" <<
        std::endl << std::endl;
    std::cerr <<
        "  Show information about a Gating-ML file:" <<
        std::endl <<
        "    " << application.applicationName << " gates.xml" <<
        std::endl << std::endl;
    std::cerr <<
        "  Show information about a parameter map file:" <<
        std::endl <<
        "    " << application.applicationName << " parameters.map" <<
        std::endl << std::endl;
    std::cerr <<
        "  Show information about an event file with a specific format:" <<
        std::endl <<
        "    " << application.applicationName << " --format fcs events.data" <<
        std::endl << std::endl;
    std::cerr <<
        "  Show information about an FCS event file, outputing in JSON:" <<
        std::endl <<
        "    " << application.applicationName << " events.fcs --format json" <<
        std::endl << std::endl;
    std::cerr <<
        "  Show information about an FCS event file, de-identifying and cleaning" <<
        std::endl <<
        "  parameters first. Save the dictionary and parameters to separate JSON" <<
        std::endl <<
        "  files. Output to the screen in JSON:" <<
        std::endl <<
        "    " << application.applicationName << " events.fcs --deidentify --cleanparameters --savedictionary dictionary.json --saveparameters parameters.json --format json" <<
        std::endl;

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
    std::cerr << packageGroupIndent << "Gating cache files:" << std::endl;

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
// Output utilities.
//
//----------------------------------------------------------------------
/**
 * Prints a list of an event table's parameters as text.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] eventTable
 *   The event table.
 */
void printEventTableParametersAsText(
    std::ostream*const outputStream,
    std::shared_ptr<const EventTableInterface> eventTable )
    noexcept
{
    const auto parameterNames = eventTable->getParameterNames( );
    const auto n = parameterNames.size( );

    for ( uint32_t i = 0; i < n; ++i )
        (*outputStream) << parameterNames[i] << std::endl;
}

/**
 * Prints a list of an event table's parameters as JSON.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] eventTable
 *   The event table.
 */
void printEventTableParametersAsJson(
    std::ostream*const outputStream,
    std::shared_ptr<const EventTableInterface> eventTable )
    noexcept
{
    const auto parameterNames = eventTable->getParameterNames( );
    const auto n = parameterNames.size( );

    (*outputStream) << "[" << std::endl;
    for ( uint32_t i = 0; i < n; ++i )
    {
        if ( i != (n-1) )
            (*outputStream) << "  \"" << parameterNames[i] << "\"," << std::endl;
        else
            (*outputStream) << "  \"" << parameterNames[i] << "\"" << std::endl;
    }
    (*outputStream) << "]" << std::endl;
}

/**
 * Prints a list of an event table's parameters.
 *
 * @param[in] eventTable
 *   The event table.
 */
void printEventTableParameters( std::shared_ptr<const EventTableInterface> eventTable )
    noexcept
{
    if ( application.parametersPath.empty() == true )
        return;

    std::ofstream*const stream = new std::ofstream(
        application.parametersPath,
        std::fstream::out | std::fstream::trunc );

    if ( stream->fail( ) == true )
        printErrorAndExit( "Cannot open parameters output file.\n" );

    switch ( application.parametersFormat )
    {
    case JSON_FORMAT:
        printEventTableParametersAsJson( stream, eventTable );
        break;

    default:
    case TEXT_FORMAT:
        printEventTableParametersAsText( stream, eventTable );
        break;
    }

    stream->close( );
}



/**
 * Recursively prints information about a gate and its children.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] gate
 *   The gate to print.
 * @param[in] indent
 *   The line indent.
 */
void printGateAsText(
    std::ostream*const outputStream,
    std::shared_ptr<const Gate> gate,
    const std::string& indent )
    noexcept
{
    (*outputStream) << gate->toString( indent );
    const auto nChildren = gate->getNumberOfChildren( );
    for ( size_t i = 0; i < nChildren; ++i )
        printGateAsText( outputStream, gate->getChild( i ), indent + "  " );
}




/**
 * Prints the name-value pairs of a files attributes map.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] attributes
 *   The file attributes.
 * @param[in] eventTable
 *   (optional, default = nullptr) The event table.
 * @param[in] gateTrees
 *   (optional, default = nullptr) The gate trees.
 * @param[in] parameterMap
 *   (optional, default = nullptr) The parameter map.
 */
void printAttributesAsText(
    std::ostream*const outputStream,
    const std::map<const std::string, const std::string> attributes,
    std::shared_ptr<const EventTableInterface> eventTable = nullptr,
    std::shared_ptr<GateTrees> gateTrees = nullptr,
    std::shared_ptr<ParameterMap> parameterMap = nullptr )
    noexcept
{
    // Print out a list of name-value pairs from the attributes map.
    for ( auto it = attributes.begin( ); it != attributes.end( ); ++it )
    {
        // Look for and remove any control characters. This is rare,
        // but it can occur in a dictionary key for the FCS dictionary
        // delimiter where some FCS files use ^L or ^J.
        auto value = it->second;
        value.erase(
            std::remove_if( value.begin( ), value.end( ), [](char c) { return std::iscntrl(c); }),
            value.end( ) );

        (*outputStream) << std::setw(30) << std::left <<
            it->first << " = \"" <<
            value << "\"" << std::endl;
    }

    // Add parameters from a parameter map, if any.
    if ( parameterMap != nullptr )
    {
        const auto names = parameterMap->getParameterNames( );
        const auto n = names.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            const auto longName =
                parameterMap->findParameterLongName( names[i] );
            const auto description =
                parameterMap->findParameterDescription( names[i] );
            const auto visibleForGating =
                parameterMap->findParameterVisibleForGating( names[i] );
            const auto defaultTransform =
                parameterMap->findParameterDefaultTransform( names[i] );

            (*outputStream) << "  " << names[i] << std::endl;
            if ( longName.empty( ) == false )
                (*outputStream) << "    longName: \"" <<
                    longName << "\"" << std::endl;
            if ( description.empty( ) == false )
                (*outputStream) << "    description: \"" <<
                    description << "\"" << std::endl;
            if ( visibleForGating == true )
                (*outputStream) << "    visibleForGating: true" << std::endl;
            else
                (*outputStream) << "    visibleForGating: false" << std::endl;
            if ( defaultTransform == nullptr )
                (*outputStream) << "    defaultTransform: null" << std::endl;
            else
                (*outputStream) << "    defaultTransform: " <<
                    defaultTransform->toString( ) << std::endl;
        }
    }

    // Add attributes describing the event table's parameters, if any.
    if ( eventTable != nullptr )
    {
        (*outputStream) << std::setw(30) << std::left <<
            "numberOfEventsInFile" << " = " <<
            eventTable->getNumberOfOriginalEvents( ) << std::endl;

        (*outputStream) << std::setw(30) << std::left <<
            "numberOfParameters" << " = " <<
            eventTable->getNumberOfParameters( ) << std::endl;

        (*outputStream) << "parameters:" << std::endl;
        auto names = eventTable->getParameterNames( );
        auto longNames = eventTable->getParameterLongNames( );
        const size_t n = names.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            (*outputStream) << "  " << names[i] << std::endl;
            if ( longNames[i].empty( ) == false )
                (*outputStream) << "    longName: " <<
                    longNames[i] << std::endl;
            (*outputStream) << "    minimum: " <<
                eventTable->getParameterMinimum(i) << std::endl;
            (*outputStream) << "    maximum: " <<
                eventTable->getParameterMaximum(i) << std::endl;
        }
    }

    // Add a hierarchical dump of the gate trees, if any.
    if ( gateTrees != nullptr )
    {
        (*outputStream) << std::setw(30) << std::left <<
            "numberOfTransforms" << " = " <<
            gateTrees->findNumberOfDescendentTransforms( ) << std::endl;

        (*outputStream) << std::setw(30) << std::left <<
            "numberOfGates" << " = " <<
            gateTrees->findNumberOfDescendentGates( ) << std::endl;

        (*outputStream) << std::setw(30) << std::left <<
            "numberOfGateTrees" << " = " <<
            gateTrees->getNumberOfGateTrees( ) << std::endl;


        (*outputStream) << std::setw(30) << std::left <<
            "gates" << std::endl;
        const auto n = gateTrees->getNumberOfGateTrees( );
        for ( size_t i = 0; i < n; ++i )
            printGateAsText( outputStream, gateTrees->getGateTree( i ), "   " );
    }
}

/**
 * Prints the name-value pairs of a files attributes map in JSON format.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] attributes
 *   The file attributes.
 * @param[in] eventTable
 *   (optional, default = nullptr) The event table.
 * @param[in] gateTrees
 *   (optional, default = nullptr) The gate trees.
 * @param[in] parameterMap
 *   (optional, default = nullptr) The parameter map.
 */
void printAttributesAsJson(
    std::ostream*const outputStream,
    const std::map<const std::string, const std::string> attributes,
    std::shared_ptr<const EventTableInterface> eventTable = nullptr,
    std::shared_ptr<GateTrees> gateTrees = nullptr,
    std::shared_ptr<ParameterMap> parameterMap = nullptr )
    noexcept
{
    (*outputStream) << "{" << std::endl;

    // Print out a list of name-value pairs from the attributes map.
    for ( auto it = attributes.begin( ); it != attributes.end( ); ++it )
    {
        // Look for and remove any control characters. This is rare,
        // but it can occur in a dictionary key for the FCS dictionary
        // delimiter where some FCS files use ^L or ^J.
        auto value = it->second;
        value.erase(
            std::remove_if( value.begin( ), value.end( ), [](char c) { return std::iscntrl(c); }),
            value.end( ) );

        (*outputStream) << "  \"" <<
            it->first << "\": \"" <<
            value << "\"," << std::endl;
    }

    // Add attributes describing the event table's parameters, if any.
    if ( eventTable != nullptr )
    {
        (*outputStream) << "  \"numberOfEventsInFile\": " <<
            eventTable->getNumberOfOriginalEvents( ) << "," << std::endl;

        (*outputStream) << "  \"numberOfParameters\": " <<
            eventTable->getNumberOfParameters( ) << "," << std::endl;

        (*outputStream) << "  \"parameters\": {" << std::endl;
        auto names = eventTable->getParameterNames( );
        auto longNames = eventTable->getParameterLongNames( );
        const size_t n = names.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            (*outputStream) << "    \"" << names[i] << "\": {" << std::endl;
            (*outputStream) << "      \"longName\": \"" <<
                longNames[i] << "\"," << std::endl;
            (*outputStream) << "      \"minimum\": \"" <<
                eventTable->getParameterMinimum(i) << "\"," << std::endl;
            (*outputStream) << "      \"maximum\": \"" <<
                eventTable->getParameterMaximum(i) << "\"" << std::endl;
            if ( i == (n-1) )
                (*outputStream) << "    }" << std::endl;
            else
                (*outputStream) << "    }," << std::endl;
        }
        if ( parameterMap == nullptr && gateTrees == nullptr )
            (*outputStream) << "  }" << std::endl;
        else
            (*outputStream) << "  }," << std::endl;
    }

    // Add parameters from a parameter map, if any.
    if ( parameterMap != nullptr )
    {
        (*outputStream) << "  \"parameters\": {" << std::endl;

        const auto names = parameterMap->getParameterNames( );
        const auto n = names.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            const auto longName =
                parameterMap->findParameterLongName( names[i] );
            const auto description =
                parameterMap->findParameterDescription( names[i] );
            const auto visibleForGating =
                parameterMap->findParameterVisibleForGating( names[i] );
            const auto defaultTransform =
                parameterMap->findParameterDefaultTransform( names[i] );

            (*outputStream) << "    \"" << names[i] << "\": {" << std::endl;
            (*outputStream) << "      \"longName\": \"" <<
                longName << "\"," << std::endl;
            (*outputStream) << "      \"description\": \"" <<
                description << "\"," << std::endl;
            if ( visibleForGating == true )
                (*outputStream) << "      \"visibleForGating\": 1," << std::endl;
            else
                (*outputStream) << "      \"visibleForGating\": 0," << std::endl;
            if ( defaultTransform == nullptr )
                (*outputStream) << "      \"defaultTransform\": null" << std::endl;
            else
            {
// TODO Output the transform in JSON, not as a string.
                (*outputStream) << "      \"defaultTransform\": \"" <<
                    defaultTransform->toString( ) << "\"" << std::endl;
            }

            if ( i == (n-1) )
                (*outputStream) << "    }" << std::endl;
            else
                (*outputStream) << "    }," << std::endl;
        }

        if ( gateTrees == nullptr )
            (*outputStream) << "  }" << std::endl;
        else
            (*outputStream) << "  }," << std::endl;
    }


    // Add a hierarchical dump of the gate trees, if any.
    if ( gateTrees != nullptr )
    {
        auto pseudoFile = new FileFGJsonGates( gateTrees );
        pseudoFile->setFileIncludeGateStateParameters( true );
        (*outputStream) << "  \"gateTrees\":" << pseudoFile->saveText( "    " );
        delete pseudoFile;
    }

    (*outputStream) << "}" << std::endl;
}

/**
 * Prints the name-value pairs of a files attributes map in JSON format.
 *
 * @param[in] attributes
 *   The file attributes.
 * @param[in] eventTable
 *   (optional, default = nullptr) The event table.
 * @param[in] gateTrees
 *   (optional, default = nullptr) The gate trees.
 * @param[in] parameterMap
 *   (optional, default = nullptr) The parameter map.
 */
void printAttributes(
    const std::map<const std::string, const std::string> attributes,
    std::shared_ptr<const EventTableInterface> eventTable = nullptr,
    std::shared_ptr<GateTrees> gateTrees = nullptr,
    std::shared_ptr<ParameterMap> parameterMap = nullptr )
    noexcept
{
    switch ( application.displayFormat )
    {
    case JSON_FORMAT:
        printAttributesAsJson(
            &std::cout,
            attributes,
            eventTable,
            gateTrees,
            parameterMap );
        break;

    default:
    case TEXT_FORMAT:
        printAttributesAsText(
            &std::cout,
            attributes,
            eventTable,
            gateTrees,
            parameterMap );
        break;
    }
}



/**
 * Prints the dictionary as text.
 *
 * Keywords and values are output in alphabetical order.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] fcsfile
 *   The file object.
 */
void printFCSDictionaryAsText(
    std::ostream*const outputStream,
    const FileFCS*const fcsfile )
    noexcept
{
    // Get all dictionary keywords.
    auto keys = fcsfile->getDictionaryKeywords( );

    // Output each keyword and its value.
    const auto n = keys.size( );
    for ( size_t i = 0; i < n; ++i )
    {
        const auto key = keys[i];
        auto value = fcsfile->getDictionaryString( key );

        // Look for and remove any control characters. This is rare,
        // but it can occur in a dictionary key for the FCS dictionary
        // delimiter where some FCS files use ^L or ^J.
        value.erase(
            std::remove_if( value.begin( ), value.end( ), [](char c) { return std::iscntrl(c); }),
            value.end( ) );

        (*outputStream) << std::setw(30) << std::left << key << " = \"" <<
            value << "\"" << std::endl;
    }
}

/**
 * Prints the dictionary as JSON.
 *
 * Keywords are grouped by category, then output as nested alphabetical
 * lists.
 *
 * @param[in,out] outputStream
 *   The stream to write to.
 * @param[in] fcsfile
 *   The file object.
 *
 * @todo If a keyword value has a double quote, the output will become
 * invalid JSON syntax. Escape embedded double quotes.
 */
void printFCSDictionaryAsJson(
    std::ostream*const outputStream,
    const FileFCS*const fcsfile )
    noexcept
{
    //
    // Get all dictionary keywords.
    // ----------------------------
    // The returned list of keywords are alphabetical and include both
    // standard and non-standard keywords mixed together.
    auto keys = fcsfile->getDictionaryKeywords( );
    const auto n = keys.size( );


    // Sort keywords by category.
    // --------------------------
    // Loop over the keywords. For each one, get its attributes and from those
    // attributes determine its category and whether it is a parameter keyword.
    //
    // Sort keywords into a list for each category. For parameters in the
    // ACQUISITION category only, separate them into their own list ordered
    // by parameter number.
    typedef std::pair<std::string,std::string> KeyValue;
    typedef std::vector<KeyValue> KeyValueList;

    const size_t nParameters = fcsfile->getNumberOfParameters( );

    std::map<uint32_t,KeyValueList> groups;

    std::vector<KeyValueList> parameters;
    parameters.resize( nParameters + 1 );

    for ( size_t i = 0; i < n; ++i )
    {
        // Create a key-value pair to add to the groups lists.
        const auto key   = keys[i];
        auto value = fcsfile->getDictionaryString( key );

        // Look for and remove any control characters. This is rare,
        // but it can occur in a dictionary key for the FCS dictionary
        // delimiter where some FCS files use ^L or ^J.
        value.erase(
            std::remove_if( value.begin( ), value.end( ), [](char c) { return std::iscntrl(c); }),
            value.end( ) );

        const auto pair  = std::make_pair( key, value );

        // Look up the category attributes of the keyword.
        auto cat = FCSKeywordCategory::OTHER;
        bool added = false;
        try
        {
            const auto attr = FCSVocabulary::find( key );
            cat = attr.getCategory( );

            // If the keyword category is for ACQUISITION keywords, and
            // the keyword is for a parameter, then extract the parameter
            // index embedded in the keyword and use it to add the keyword
            // and value to a per-parameter list of keywords.
            if ( cat == FCSKeywordCategory::ACQUISITION &&
                attr.isParameter( ) == true )
            {
                const auto index = FCSVocabulary::getParameterIndexFromKeyword( key );
                if ( index != 0 )
                {
                    parameters[index].push_back( pair );
                    added = true;
                }
            }
        }
        catch ( ... )
        {
            // Not found. Stay with the default OTHER category.
        }

        // If not already added, add the keyword and value to the keyword's
        // category list.
        if ( added == false )
        {
            // Create the group's list, if needed.
            if ( groups.count(cat) == 0 )
                groups[cat] = KeyValueList( );

            // Add to the list.
            groups[cat].push_back( pair );
        }
    }


    // Output JSON.
    // ------------
    // Loop over the category groups and output the key-value pairs in each
    // one. For the ACQUISITION group, also output the parameters as an
    // array with one entry per parameter.
    (*outputStream) << "{" << std::endl;
    for ( auto it = groups.cbegin( ); it != groups.cend( ); ++it )
    {
        // Close the previous group.
        if ( it != groups.cbegin( ) )
            (*outputStream) << "  }," << std::endl;

        const auto cat = (*it).first;
        const auto vec = (*it).second;
        const auto catName = FCSKeywordCategory::toString( cat );
        const auto size = vec.size( );

        // Start the category.
        (*outputStream) << "  \"" << catName << "\": {" << std::endl;

        // If the category is for ACQUISITION, then write out parameters first.
        if ( cat == FCSKeywordCategory::ACQUISITION )
        {
            // Start the parameter list.
            (*outputStream) << "    " << std::setw(30) << std::left <<
                "\"parameters\": {" << std::endl;

            // Skip entry 0 which does not exist. Embedded parameter indexes
            // start at 1.
            const size_t np = parameters.size( );
            for ( size_t i = 1; i < np; ++i )
            {
                // Start the i-th parameter.
                (*outputStream) << "      \"" << i << "\": {" << std::endl;

                const size_t n = parameters[i].size( );
                for ( size_t j = 0; j < n; ++j )
                {
                    const auto pair = parameters[i][j];
                    (*outputStream) << "        " << std::setw(30) << std::left <<
                        ("\"" + pair.first + "\": ") << "\"" << pair.second;
                    if ( j == (n-1) )
                        (*outputStream) << "\"" << std::endl;
                    else
                        (*outputStream) << "\"," << std::endl;
                }

                if ( i == (np-1) )
                    (*outputStream) << "      }" << std::endl;
                else
                    (*outputStream) << "      }," << std::endl;
            }

            // End the parameter list.
            (*outputStream) << "    }," << std::endl;
        }

        // Write out key-value pairs in the category.
        for ( size_t i = 0; i < size; ++i )
        {
            const auto pair = vec[i];
            (*outputStream) << "    " << std::setw(30) << std::left <<
                ("\"" + pair.first + "\": ") << "\"" << pair.second;
            if ( i == (size-1) )
                (*outputStream) << "\"" << std::endl;
            else
                (*outputStream) << "\"," << std::endl;
        }
    }
    (*outputStream) << "  }" << std::endl;
    (*outputStream) << "}" << std::endl;
}

/**
 * Prints the dictionary.
 *
 * Keywords are grouped by category, then output as nested alphabetical
 * lists.
 *
 * @param[in] file
 *   The file object.
 */
void printFCSDictionary( const FileFCS*const file )
    noexcept
{
    if ( application.dictionaryPath.empty( ) == true )
        return;

    std::ofstream* stream = new std::ofstream(
        application.dictionaryPath,
        std::fstream::out | std::fstream::trunc );

    if ( stream->fail( ) == true )
        printErrorAndExit( "Cannot open dictionary output file.\n" );

    switch ( application.dictionaryFormat )
    {
    case JSON_FORMAT:
        printFCSDictionaryAsJson( stream, file );
        break;

    default:
    case TEXT_FORMAT:
        printFCSDictionaryAsText( stream, file );
        break;
    }

    stream->close( );
}





//----------------------------------------------------------------------
//
// Get info about event files.
//
//----------------------------------------------------------------------
/**
 * Outputs information about an FCS file.
 *
 * The input file is loaded and optionally de-identified and cleaned.
 * The file's dictionary of keyword-value pairs is optionally saved to
 * an output file. The file's event table parameters are optionally
 * saved to another output file. And the file's attributes are printed
 * to the output or optionally into another file.
 */
void infoFCS( )
    noexcept
{
    FileFCS* file = new FileFCS( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        // Load the file's attributes, but no event data.
        file->load( application.sourcePath, 0 );

        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    // Deidentify and clean, if so directed.
    if ( application.deidentify == true )
        file->deidentify( );

    if ( application.cleanParameters == true )
        file->cleanByFlags( FCSKeyword::PARAMETER );

    // Output a list of keywords and their values to a dictionary file.
    printFCSDictionary( file );

    // Output parameter information to a parameter file.
    printEventTableParameters( file->getEventTable( ) );

    // Output file attributes.
    printAttributes( file->getFileAttributes( ), file->getEventTable( ) );

    delete file;
}

/**
 * Outputs information about an FGB file.
 *
 * The input file is loaded. The file's event table parameters are optionally
 * saved to an output file. And the file's attributes are printed
 * to the output or optionally into another file.
 */
template <class FILETYPE>
void infoFGEvents( )
    noexcept
{
    FILETYPE* file = new FILETYPE( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        // Load the file's attributes, but no event data.
        file->load( application.sourcePath, 0 );

        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    // Output parameter information to a parameter file.
    printEventTableParameters( file->getEventTable( ) );

    // Output file attributes.
    printAttributes( file->getFileAttributes( ), file->getEventTable( ) );

    delete file;
}





//----------------------------------------------------------------------
//
// Get info about gating cache files.
//
//----------------------------------------------------------------------
/**
 * Outputs information about an FG Gating Cache file.
 *
 * The input file is loaded. The file's event table parameters are optionally
 * saved to an output file. And the file's attributes and gate trees are printed
 * to the output or optionally into another file.
 */
void infoFGGatingCache( )
    noexcept
{
    FileFGGatingCache* file = new FileFGGatingCache( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        // Load the file's attributes and gate trees, but no event data.
        file->load( application.sourcePath, false );

        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    // Output parameter information to a parameter file.
    printEventTableParameters(
        file->getGatingCache( )->getSourceEventTable( ) );

    // Deidentify.
    auto gateTrees = file->getGateTrees();
    if ( application.deidentify == true )
        gateTrees->deidentify( );

    // Output file attributes.
    printAttributes(
        file->getFileAttributes( ),
        file->getGatingCache( )->getSourceEventTable( ),
        gateTrees );

    delete file;
}





//----------------------------------------------------------------------
//
// Get info about gate tree files.
//
//----------------------------------------------------------------------
/**
 * Outputs information about a gating file.
 *
 * The input file is loaded. The file's gate trees are printed
 * to the output or optionally into a file.
 */
template <class FILETYPE>
void infoGateTrees( )
    noexcept
{
    FILETYPE* file = new FILETYPE( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        // Load the file's attributes and gate trees.
        file->load( application.sourcePath );

        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    // Deidentify.
    auto gateTrees = file->getGateTrees();
    if ( application.deidentify == true )
        gateTrees->deidentify( );

    // Output file attributes.
    printAttributes(
        file->getFileAttributes( ),
        nullptr,
        gateTrees );

    delete file;
}





//----------------------------------------------------------------------
//
// Get info about parameter map files.
//
//----------------------------------------------------------------------
/**
 * Outputs information about a parameter map file.
 *
 * The input file is loaded. The file's gate trees are printed
 * to the output or optionally into a file.
 */
template <class FILETYPE>
void infoParameterMap( )
    noexcept
{
    FILETYPE* file = new FILETYPE( );
    file->setVerbose( application.verbose );
    file->setVerbosePrefix( application.applicationName );

    try
    {
        // Load the file's attributes and parameter map.
        file->load( application.sourcePath );

        if ( application.verbose == true ||
             application.showWarnings == true )
            printFileLog( file->getFileLog( ) );
    }
    catch ( const std::exception& e )
    {
        if ( file != nullptr &&
             (application.verbose == true ||
             application.showWarnings == true) )
            printFileLog( file->getFileLog( ) );

        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    auto map = file->getParameterMap();

    // Output file attributes.
    printAttributes(
        file->getFileAttributes( ),
        nullptr,
        nullptr,
        map );

    delete file;
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
            // Generic, but unsupported.
            //
            if ( std::strcmp( "benchmark", option ) == 0 )
            {
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
            // Output.
            //
            if ( std::strcmp( "savedictionary", option ) == 0 )
            {
                if ( argc < 2 )
                    printUsageAndExit( "Too few arguments after --savedictionary\n" );
                application.dictionaryPath = std::string( argv[1] );
                application.dictionaryFormat = UNKNOWN_TEXT_FORMAT;
                argc -= 2;
                argv += 2;

                if ( pendingFormat.empty( ) == false )
                {
                    application.dictionaryFormat =
                        findTextFileFormat( pendingFormat );
                    if ( application.dictionaryFormat == UNKNOWN_TEXT_FORMAT )
                        printErrorAndExit(
                            std::string( "Unknown file format name: " ) +
                            pendingFormat + "\n" );

                    pendingFormat.clear( );
                }
                continue;
            }
            if ( std::strcmp( "saveparameters", option ) == 0 )
            {
                if ( argc < 2 )
                    printUsageAndExit( "Too few arguments after --saveparameters\n" );
                application.parametersPath = std::string( argv[1] );
                application.parametersFormat = UNKNOWN_TEXT_FORMAT;
                argc -= 2;
                argv += 2;

                if ( pendingFormat.empty( ) == false )
                {
                    application.parametersFormat =
                        findTextFileFormat( pendingFormat );
                    if ( application.parametersFormat == UNKNOWN_TEXT_FORMAT )
                        printErrorAndExit(
                            std::string( "Unknown file format name: " ) +
                            pendingFormat + "\n" );

                    pendingFormat.clear( );
                }
                continue;
            }

            //
            // Processing.
            //
            if ( std::strcmp( "deidentify", option ) == 0 )
            {
                application.deidentify = true;
                --argc;
                ++argv;
                continue;
            }
            if ( std::strcmp( "cleanparameters", option ) == 0 )
            {
                application.cleanParameters = true;
                --argc;
                ++argv;
                continue;
            }

            // Unknown.
            printErrorAndExit(
                std::string( "Unknown option: '" ) +
                argv[0] + "'.\n" );
        }

        // Input files.
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

        // Too many files.
        printErrorAndExit(
            std::string( "Too many files." ) );
    }

    // Check that we got essential arguments.
    if ( application.sourcePath.empty( ) == true )
        printErrorAndExit(
            std::string( "An input event or gate file is required.\n" ) );

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
    // Parse command-line arguments to get the input file
    // and options on how to use it.
    parseArguments( argc, argv );


    //
    // Determine file formats.
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

    if ( application.dictionaryPath.empty( ) == false &&
         application.dictionaryFormat == UNKNOWN_TEXT_FORMAT )
    {
        const std::string ext = getFileExtension(
            application.dictionaryPath );
        if ( ext.empty( ) == true )
            printErrorAndExit(
                std::string( "\"" ) + application.dictionaryPath +
                "\" has no file name extension.\n" +
                "Use --format to specify the file format.\n" );

        application.dictionaryFormat = findTextFileFormat( ext );
        if ( application.dictionaryFormat == UNKNOWN_TEXT_FORMAT )
            printErrorAndExit(
                std::string( "\"" ) + application.dictionaryPath +
                "\" file name extension is not recognized.\n" +
                "Use --format to specify the file format.\n" );
    }

    if ( application.parametersPath.empty( ) == false &&
         application.parametersFormat == UNKNOWN_TEXT_FORMAT )
    {
        const std::string ext = getFileExtension(
            application.parametersPath );
        if ( ext.empty( ) == true )
            printErrorAndExit(
                std::string( "\"" ) + application.parametersPath +
                "\" has no file name extension.\n" +
                "Use --format to specify the file format.\n" );

        application.parametersFormat = findTextFileFormat( ext );
        if ( application.parametersFormat == UNKNOWN_TEXT_FORMAT )
            printErrorAndExit(
                std::string( "\"" ) + application.parametersPath +
                "\" file name extension is not recognized.\n" +
                "Use --format to specify the file format.\n" );
    }

    //
    // Comment on invalid arguments.
    // -----------------------------
    // Some command-line arguments are for particular file types.
    if ( application.sourceFormat != FCS_FORMAT &&
        application.sourceFormat != GATINGML_FORMAT &&
        application.sourceFormat != FGJSONGATES_FORMAT)
    {
        if ( application.deidentify == true )
            std::cerr <<
                "De-identification is only available for FCS, Gating-ML, and JSON gate files." <<
                std::endl;
    }

    if ( application.sourceFormat != FCS_FORMAT )
    {
        if ( application.cleanParameters == true )
            std::cerr <<
                "Cleaning parameter keywords is only available for FCS files." <<
                std::endl;

        if ( application.dictionaryPath.empty( ) == false )
            std::cerr <<
                "Dictionary output is only available for FCS files." <<
                std::endl;
    }

    //
    // Get information.
    // ----------------
    // For each supported file type, get file information and print it.
    switch ( application.sourceFormat )
    {
        case FCS_FORMAT:
            infoFCS( );
            break;

        case FGBINARYEVENTS_FORMAT:
            infoFGEvents<FileFGBinaryEvents>( );
            break;

        case FGTEXTEVENTS_FORMAT:
            infoFGEvents<FileFGTextEvents>( );
            break;

        case FGGATINGCACHE_FORMAT:
            infoFGGatingCache( );
            break;

        case GATINGML_FORMAT:
            infoGateTrees<FileGatingML>( );
            break;

        case FGTEXTGATES_FORMAT:
            infoGateTrees<FileFGTextGates>( );
            break;

        case FGJSONGATES_FORMAT:
            infoGateTrees<FileFGJsonGates>( );
            break;

        case FGJSONPARAMETERMAP_FORMAT:
            infoParameterMap<FileFGJsonParameterMap>( );
            break;

        default:
        case UNKNOWN_DATA_FORMAT:
            std::exit( 1 );
    }
}
