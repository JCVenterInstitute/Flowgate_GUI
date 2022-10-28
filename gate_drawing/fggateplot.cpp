/**
 * @file
 * Creates a scatter plot of selected gates from a FlowGate gated event file.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "EventTable.h"
 * @li FlowGate "GatingCache.h"
 * @li FlowGate "GateTrees.h"
 * @li FlowGate "FileFCS.h"
 * @li FlowGate "FileFGBinaryEvents.h"
 * @li FlowGate "FileFGTextEvents.h"
 * @li FlowGate "FileFGGatingCache.h"
 * @li FlowGate "FileFGTextGates.h"
 * @li FlowGate "FileFGJsonGates.h"
 * @li FlowGate "FileGatingML.h"
 * @li FlowGate "FileFGJsonParameterMap.h"
 * @li FlowGate "ScatterPlot.h"
 * @li libgd (via "ScatterPlot.h")
 */
#include <algorithm>        // std::transform
#include <iostream>         // std::cerr, ...
#include <iomanip>          // I/O formatting
#include <string>           // String type and methods
#include <cstring>          // C string functions
#include <cmath>            // C math functions
#include <limits>           // Numeric limits

#if defined(_OPENMP)
#include <omp.h>            // Parallel thread handling
#endif

// Time for benchmarking.
#if defined(_WIN32)
#define BENCHMARK_TIME_AVAILABLE
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>     // POSIX flags
#define BENCHMARK_TIME_AVAILABLE
#include <time.h>       // clock_gettime(), time()
#include <sys/time.h>   // gethrtime(), gettimeofday()
#endif

// Gason (JSON parser).
#include "gason.h"

// FlowGate libraries.
#include "GateTrees.h"          // FlowGate gates
#include "EventTable.h"         // FlowGate event table
#include "GatingCache.h"        // FlowGate gating cache table
#include "Gating.h"             // FlowGate gated event table

#include "FileFCS.h"            // FCS file format
#include "FileFGBinaryEvents.h" // FlowGate Event Binary file format
#include "FileFGTextEvents.h"   // FlowGate Text Event file format (legacy)
#include "FileFGGatingCache.h"  // FlowGate Gating Cache file format
#include "FileFGTextGates.h"    // FlowGate Text Gate file format (legacy)
#include "FileFGJsonGates.h"    // FlowGate JSON Gate file format
#include "FileFGJsonParameterMap.h" // FlowGate JSON Parameter Map file format
#include "FileGatingML.h"       // Gating-ML file format

#include "ScatterPlot.h"        // Scatter plotter





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





using namespace FlowGate::Plot;
using namespace FlowGate::File;
using namespace FlowGate::Events;
using namespace FlowGate::Gates;





//----------------------------------------------------------------------
//
// Forward declarations.
//
// To keep code organized intuitively, primiarly logic is earlier in
// the file (e.g. main()) and support functions later on. Forward
// declarations are therefore needed to pre-declare those support functions.
//
//----------------------------------------------------------------------
double getRealTime( )
    noexcept;

void addBenchmark( const std::string& title, const double time )
    noexcept;
void printBenchmarks( )
    noexcept;

void printFileLog( const std::vector<std::pair<std::string,std::string>>& log )
    noexcept;

std::shared_ptr<GatingCache> loadOrExit( );

std::vector<ScatterPlot*> createPlotObjects( )
    noexcept;
void deletePlotObjects( std::vector<ScatterPlot*>& )
    noexcept;

void plotSetup( ScatterPlot*const )
    noexcept;

void parseArguments( int, char** )
    noexcept;
void printConfiguration( )
    noexcept;
void printUsageAndExit( const std::string )
    noexcept;
void printErrorAndExit( const std::string )
    noexcept;
void printWarning( const std::string )
    noexcept;



//----------------------------------------------------------------------
//
// Enums.
//
//----------------------------------------------------------------------
/**
 * Indicates a supported event file format.
 */
enum EventFileFormat
{
    UNKNOWN_EVENT_FORMAT,

    // Event formats.
    FCS_FORMAT,
    FGBINARYEVENTS_FORMAT,
    FGTEXTEVENTS_FORMAT,

    // Gating cache formats.
    FGGATINGCACHE_FORMAT
};

/**
 * Indicates a supported gate file format.
 */
enum GateFileFormat
{
    UNKNOWN_GATE_FORMAT,

    // Gate formats.
    FGTEXTGATES_FORMAT,
    FGJSONGATES_FORMAT,
    GATINGML_FORMAT,
};

/**
 * Indicates a supported parameter map file format.
 */
enum ParameterMapFileFormat
{
    UNKNOWN_PARAMETERMAP_FORMAT,

    // Parameter map formats.
    FGJSONPARAMETERMAP_FORMAT,
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
 * Indicates a supported output file format.
 */
enum ImageFileFormat
{
    UNKNOWN_IMAGE_FORMAT,

    // Image formats.
    PNG_INDEXED_COLOR,
    PNG_TRUE_COLOR,
    GIF_INDEXED_COLOR,
    JPG_INDEXED_COLOR,
    JPG_TRUE_COLOR
};





//----------------------------------------------------------------------
//
// Constants.
//
//----------------------------------------------------------------------
// Dot shapes.
#define DOT_CIRCLE  0
#define DOT_SQUARE  1

// Visual settings.
#define DEFAULT_IMAGE_WIDTH         1000
#define DEFAULT_IMAGE_HEIGHT        1000

#define DEFAULT_IMAGE_BOTTOM_MARGIN_PERCENT  0.09
#define DEFAULT_IMAGE_LEFT_MARGIN_PERCENT    0.12
#define DEFAULT_IMAGE_RIGHT_MARGIN_PERCENT   0.05
#define DEFAULT_IMAGE_TOP_MARGIN_PERCENT     0.09

#define DEFAULT_AXIS_NUMBER_COLOR    0x5E5E5E
//#define DEFAULT_AXIS_NUMBER_FONT_NAMES "Times New Roman"
//#define DEFAULT_AXIS_NUMBER_FONT_NAMES "Palatino"
#define DEFAULT_AXIS_NUMBER_FONT_NAMES "sans-serif"
#define DEFAULT_AXIS_NUMBER_FONT_SIZE_PERCENT 0.015
#define DEFAULT_AXIS_NUMBER_OF_TICS 6
#define DEFAULT_AXIS_TIC_COLOR       0x5E5E5E
#define DEFAULT_AXIS_TIC_LENGTH_PERCENT      0.01
#define DEFAULT_AXIS_TIC_LINE_WIDTH  1
#define DEFAULT_AXIS_TITLE_COLOR     0x000000
//#define DEFAULT_AXIS_TITLE_FONT_NAMES "Times New Roman"
//#define DEFAULT_AXIS_TITLE_FONT_NAMES "Palatino"
#define DEFAULT_AXIS_TITLE_FONT_NAMES "sans-serif"
#define DEFAULT_AXIS_TITLE_FONT_SIZE_PERCENT 0.02
#define DEFAULT_AXIS_X_TITLE_MARGIN_PERCENT 0.0
#define DEFAULT_AXIS_Y_TITLE_MARGIN_PERCENT 0.0
#define DEFAULT_AXIS_NUMBER_MAX_DIGITS_AFTER_DECIMAL 3

#define DEFAULT_BORDER_COLOR         0x5E5E5E
#define DEFAULT_BORDER_LINE_WIDTH    1

#define DEFAULT_DOT_SHAPE            DOT_SQUARE
#define DEFAULT_DOT_SIZE             1

#define DEFAULT_PLOT_BACKGROUND_COLOR 0xF8F8F8

#define DEFAULT_TITLE_COLOR          0x000000
//#define DEFAULT_TITLE_FONT_NAMES     "Times New Roman:bold"
//#define DEFAULT_TITLE_FONT_NAMES     "Palatino:bold"
#define DEFAULT_TITLE_FONT_NAMES     "sans-serif:bold"
//#define DEFAULT_TITLE_FONT_SIZE_PERCENT 0.035
#define DEFAULT_TITLE_FONT_SIZE_PERCENT 0.025

#define DEFAULT_GATE_LINE_WIDTH      1
#define DEFAULT_GATE_SHADOW_COLOR    0x8E8E8E

//#define DEFAULT_LEGEND_FONT_NAMES    "Times New Roman"
//#define DEFAULT_LEGEND_FONT_NAMES    "Palatino"
#define DEFAULT_LEGEND_FONT_NAMES    "sans-serif"
#define DEFAULT_LEGEND_FONT_SIZE_PERCENT 0.02

#define DEFAULT_DRAW_PARENTS_FIRST   true
#define DEFAULT_SHOW_FILLED_GATES    false
#define DEFAULT_SHOW_GATE_SHADOWS    false
#define DEFAULT_SHOW_LEGEND          false
#define DEFAULT_SHOW_MULTIPLE_GATES  false
#define DEFAULT_SHOW_PARENT_POPULATION false





//----------------------------------------------------------------------
//
// Globals - colors NOT set by command-line arguments.
//
//----------------------------------------------------------------------
#define PARENT_POPULATION_COLOR_LIST_INDEX  0
#define UNGATED_POPULATION_COLOR_LIST_INDEX 1
#define GATED_POPULATION_COLOR_LIST_INDEX   2

/**
 * A set of dot colors chosen to be attractive and maximally different
 * when multiple gates are on the same plot.
 */
std::vector<int> GoodDotColors(
{
    0x000000,   // Black        (used for parent population)
    0xBEBEBE,   // Gray         (used for ungated populations)
    //0x5F9DFF,   // Aqua         (first gated population)
    0x004b8e,   // Dark blue    (first gated population)
    0x70B911,   // Green        (second gated population)
    0xE7BE03,   // Tangerine    (and so on...)
    0xF2371B,   // Bright red
    0x2b4a80,   // Ocean
    0x669000,   // Fern
    0xAF2F7C,   // Strawberry
    0x801D93,   // Plum
    0x60A79C,   // Teal
    0xFCFE00,   // Lemon
    0x2A12FF,   // Blueberry
} );

/**
 * The dot colors with saturation dramatically reduced.
 */
std::vector<int> GoodFillColors(
{
    0x000000,   // Black        (never used for a gate fill)
    0xAEAEAE,   // Gray         (never used for a gate fill)
    //0xD8E7FF,   // Very light Aqua
    0xC9E1FF,   // Very light blue
    0xE3F3CE,   // Very light Flora
    0xF2EBCE,   // Very light Tangerine
    0xF2CCC6,   // Very light Bright red
    0xC7DBFF,   // Very light Ocean
    0xD6E0BE,   // Very light Fern
    0xFFd9F0,   // Very light Strawberry
    0xCFB5D5,   // Very light Plum
    0xCFFFF7,   // Very light Teal
    0xFEFEB2,   // Very light Lemon
    0xD1CCFF,   // Very light Blueberry
} );





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
    std::string eventPath;

    /**
     * The input file format.
     */
    EventFileFormat eventFormat = UNKNOWN_EVENT_FORMAT;

    /**
     * The file path for the input gate file.
     */
    std::string gatePath;

    /**
     * The input gate file format.
     */
    GateFileFormat gateFormat = UNKNOWN_GATE_FORMAT;

    /**
     * The file path for the input parameter map file.
     */
    std::string parameterMapPath;

    /**
     * The input parameter map file format.
     */
    ParameterMapFileFormat parameterMapFormat = UNKNOWN_PARAMETERMAP_FORMAT;

    /**
     * The file path for the output file.
     */
    std::string cachePath;

    /**
     * The base file path for the output plot file(s).
     */
    std::string imagePath;

    /**
     * The output image format.
     */
    ImageFileFormat imageFormat = UNKNOWN_IMAGE_FORMAT;

    /**
     * The stdout output format.
     */
    TextFileFormat displayFormat = UNKNOWN_TEXT_FORMAT;

    /**
     * The gating cache to gate and plot.
     */
    std::shared_ptr<GatingCache> gatingCache;

    /**
     * The maximum number of events to load and plot.
     */
    ssize_t maxEvents = (-1);

    /**
     * The parameter number for the X axis.
     */
    size_t xParameterNumber = 0;

    /**
     * The parameter number for the Y axis.
     */
    size_t yParameterNumber = 1;

    /**
     * The list of gate indexes to plot.
     */
    std::vector<GateIndex> gatesToPlot;

    /**
     * Output the parameter names.
     */
    bool showParameterNames = false;

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

    /**
     * Accumulated benchmark information.
     */
    std::vector<std::pair<std::string,double>> benchmarkData;

    /**
     * The number of threads to use.
     *
     * When 0, use all available threads.
     */
    uint32_t numberOfThreads = 0;

    /**
     * Whether to write the output file in the background.
     *
     * When FALSE (default), the output cache file (if any) is written
     * on the current process before the application exits.
     *
     * When TRUE, the current process forks and exits, leaving the second
     * background thread to finish writing the cache file. This lets scripts
     * and other code that runs this command continue, if it doesn't need
     * the cache file immediately.
     */
    bool backgroundCacheWrite = false;

    /**
     * The number of initial clusters to find for any gate using the
     * clustering gating method.
     *
     * When 0, use the default.
     */
    uint32_t numberOfClusters = 0;

    /**
     * The number of iterations to perform to converge clusters for any
     * gate using the clustering gating method.
     *
     * When 0, use the default.
     */
    uint32_t clusterIterations = 0;

    /**
     * The random number seed for initializing cluster centroids.
     */
    uint32_t randomNumberSeed = 2;

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
};

/**
 * The application's state.
 */
Application application;



/**
 * Holds application state, including values from the command line.
 */
class Visual final
{
public:
    // Image size ------------------------------------------------------
    /**
     * The image width.
     */
    int imageWidth = DEFAULT_IMAGE_WIDTH;

    /**
     * The image height.
     */
    int imageHeight = DEFAULT_IMAGE_HEIGHT;



    // Image margins ---------------------------------------------------
    /**
     * The image left margin as a percentage of the image width.
     */
    float imageMarginLeftPercent = DEFAULT_IMAGE_LEFT_MARGIN_PERCENT;

    /**
     * The image right margin as a percentage of the image width.
     */
    float imageMarginRightPercent = DEFAULT_IMAGE_RIGHT_MARGIN_PERCENT;

    /**
     * The image bottom margin as a percentage of the image height.
     */
    float imageMarginBottomPercent = DEFAULT_IMAGE_BOTTOM_MARGIN_PERCENT;

    /**
     * The image top margin as a percentage of the image height.
     */
    float imageMarginTopPercent = DEFAULT_IMAGE_TOP_MARGIN_PERCENT;



    // Title -----------------------------------------------------------
    /**
     * The title from the command line.
     *
     * If empty, a title is automatically generated from the file path.
     */
    std::string title;

    /**
     * The font size of the title as a percentage of the image width.
     */
    float titleFontSizePercent = DEFAULT_TITLE_FONT_SIZE_PERCENT;

    /**
     * The title color.
     */
    int titleColor = DEFAULT_TITLE_COLOR;

    /**
     * The title font names.
     */
    std::string titleFontNames = DEFAULT_TITLE_FONT_NAMES;



    // Axes ------------------------------------------------------------
    /**
     * The font size of axis titles as a percentage of the image width.
     */
    float axisTitleFontSizePercent = DEFAULT_AXIS_TITLE_FONT_SIZE_PERCENT;

    /**
     * The line width for the plot tics.
     */
    int axisTicLineWidth = DEFAULT_AXIS_TIC_LINE_WIDTH;

    /**
     * The line length for the plot tics.
     */
    float axisTicLength = DEFAULT_AXIS_TIC_LENGTH_PERCENT;

    /**
     * The axis title color.
     */
    int axisTitleColor = DEFAULT_AXIS_TITLE_COLOR;

    /**
     * The axis numbers color.
     */
    int axisNumberColor = DEFAULT_AXIS_NUMBER_COLOR;

    /**
     * The axis tic color.
     */
    int axisTicColor = DEFAULT_AXIS_TIC_COLOR;

    /**
     * The axis title font names.
     */
    std::string axisTitleFontNames = DEFAULT_AXIS_TITLE_FONT_NAMES;

    /**
     * The axis number font names.
     */
    std::string axisNumberFontNames = DEFAULT_AXIS_NUMBER_FONT_NAMES;

    /**
     * The font size of the axis numbers as a percentage of the image width.
     */
    float axisNumberFontSizePercent = DEFAULT_AXIS_NUMBER_FONT_SIZE_PERCENT;

    /**
     * The maximum number of digits after the decimal in axis numbers.
     */
    int axisNumberMaxDigitsAfterDecimal = DEFAULT_AXIS_NUMBER_MAX_DIGITS_AFTER_DECIMAL;

    /**
     * The X axis title margin as a percentage of the image height.
     */
    float axisXTitleMarginPercent = DEFAULT_AXIS_X_TITLE_MARGIN_PERCENT;

    /**
     * The Y axis title margin as a percentage of the image width.
     */
    float axisYTitleMarginPercent = DEFAULT_AXIS_Y_TITLE_MARGIN_PERCENT;



    // Plot area -------------------------------------------------------
    /**
     * The line width for the plot area border.
     */
    int borderLineWidth = DEFAULT_BORDER_LINE_WIDTH;

    /**
     * The plot area border color.
     */
    int borderLineColor = DEFAULT_BORDER_COLOR;

    /**
     * The plot area background color.
     */
    int backgroundColor = DEFAULT_PLOT_BACKGROUND_COLOR;



    // Legend ----------------------------------------------------------
    /**
     * Whether to show the legend.
     *
     * The legend lists the names of the populations plotted, and their
     * corresponding colors.
     */
    bool showLegend = DEFAULT_SHOW_LEGEND;

    /**
     * The legend font names.
     */
    std::string legendFontNames = DEFAULT_LEGEND_FONT_NAMES;

    /**
     * The font size of axis titles as a percentage of the image width.
     */
    float legendFontSizePercent = DEFAULT_LEGEND_FONT_SIZE_PERCENT;



    // Gates -----------------------------------------------------------
    /**
     * Whether to show filled gate rectangles.
     */
    bool showFilledGates = DEFAULT_SHOW_FILLED_GATES;

    /**
     * Whether to show gate "shadows" on gate rectangles.
     */
    bool showGateShadows = DEFAULT_SHOW_GATE_SHADOWS;

    /**
     * Whether to recurse backwards and show parent gates.
     */
    bool showMultipleGates = DEFAULT_SHOW_MULTIPLE_GATES;

    /**
     * Whether to show the parent population's dots.
     */
    bool showParentPopulation = DEFAULT_SHOW_PARENT_POPULATION;

    /**
     * The line width for gate outlines.
     */
    int gateLineWidth = DEFAULT_GATE_LINE_WIDTH;

    /**
     * The gate shadow color.
     */
    int gateShadowColor = DEFAULT_GATE_SHADOW_COLOR;

    /**
     * The gate fill color set in RGB.
     */
    std::vector<int> gateFillColors = GoodFillColors;

    /**
     * The gate fill color indexes corresponding to the gate fill color set.
     */
    std::vector<int> gateFillColorIndexes;

    /**
     * The gate shadow color index.
     */
    bool gateShadowColorIndex;



    // Dots ------------------------------------------------------------
    /**
     * The size of dots.
     */
    int dotSize = DEFAULT_DOT_SIZE;

    /**
     * The shape of a dot.
     */
    int dotShape = DEFAULT_DOT_SHAPE;

    /**
     * The dot color indexes corresponding to the dot color set.
     */
    std::vector<int> dotColorIndexes;

    /**
     * The dot color set in RGB.
     */
    std::vector<int> dotColors = GoodDotColors;
};

Visual visual;





//----------------------------------------------------------------------
//
// File name extension utilities.
//
//----------------------------------------------------------------------
/**
 * Returns the event format enum for the given file name extension.
 *
 * @param[in] extension
 *   The file name extension.
 *
 * @return
 *   The file format enum.
 */
EventFileFormat findEventFileFormat( std::string extension )
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
    return UNKNOWN_EVENT_FORMAT;
}

/**
 * Returns the gate format enum for the given file name extension.
 *
 * @param[in] extension
 *   The file name extension.
 *
 * @return
 *   The file format enum.
 */
GateFileFormat findGateFileFormat( std::string extension )
    noexcept
{
    // Convert the extension to lower case.
    std::transform(
        extension.begin( ),
        extension.end( ),
        extension.begin( ),
        [](unsigned char c){ return std::tolower(c); } );

    if ( FileFGTextGates::isFileNameExtension( extension ) == true )
        return FGTEXTGATES_FORMAT;
    if ( FileFGJsonGates::isFileNameExtension( extension ) == true )
        return FGJSONGATES_FORMAT;
    if ( FileGatingML::isFileNameExtension( extension ) == true )
        return GATINGML_FORMAT;
    return UNKNOWN_GATE_FORMAT;
}

/**
 * Returns the parameter map format enum for the given file name extension.
 *
 * @param[in] extension
 *   The file name extension.
 *
 * @return
 *   The file format enum.
 */
ParameterMapFileFormat findParameterMapFileFormat( std::string extension )
    noexcept
{
    // Convert the extension to lower case.
    std::transform(
        extension.begin( ),
        extension.end( ),
        extension.begin( ),
        [](unsigned char c){ return std::tolower(c); } );

    if ( FileFGJsonParameterMap::isFileNameExtension( extension ) == true )
        return FGJSONPARAMETERMAP_FORMAT;
    return UNKNOWN_PARAMETERMAP_FORMAT;
}

/**
 * Returns the image format enum for the given file name extension.
 *
 * @param[in] extension
 *   The file name extension.
 *
 * @return
 *   Returns the file format enum.
 */
ImageFileFormat findImageFileFormat( std::string extension )
    noexcept
{
    // Convert the extension to lower case.
    std::transform(
        extension.begin( ),
        extension.end( ),
        extension.begin( ),
        [](unsigned char c){ return std::tolower(c); } );

    if ( extension == "png" )
        return PNG_TRUE_COLOR;
    if ( extension == "gif" )
        return GIF_INDEXED_COLOR;
    if ( extension == "jpg" || extension == "jpeg" )
        return JPG_TRUE_COLOR;

    return UNKNOWN_IMAGE_FORMAT;
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
 * Creates a list of known event file name extensions.
 *
 * @return
 *   Returns a string containing a comma-separated list of extensions.
 */
std::string getEventFileFormatExtensionList( )
    noexcept
{
    std::vector<std::string> list;
    std::string result;

    for ( size_t f = 0; f < 4; ++f )
    {
        switch ( f )
        {
        case 0: list = FileFCS::getFileNameExtensions( ); break;
        case 1: list = FileFGBinaryEvents::getFileNameExtensions( ); break;
        case 2: list = FileFGTextEvents::getFileNameExtensions( ); break;
        case 3: list = FileFGGatingCache::getFileNameExtensions( ); break;
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
 * Creates a list of known gate file name extensions.
 *
 * @return
 *   Returns a string containing a comma-separated list of extensions.
 */
std::string getGateFileFormatExtensionList( )
    noexcept
{
    std::vector<std::string> list;
    std::string result;

    for ( size_t f = 0; f < 3; ++f )
    {
        switch ( f )
        {
        case 0: list = FileGatingML::getFileNameExtensions( ); break;
        case 1: list = FileFGJsonGates::getFileNameExtensions( ); break;
        case 2: list = FileFGTextGates::getFileNameExtensions( ); break;
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
 * Creates a list of known parameter map file name extensions.
 *
 * @return
 *   Returns a string containing a comma-separated list of extensions.
 */
std::string getParameterMapFileFormatExtensionList( )
    noexcept
{
    std::vector<std::string> list;
    std::string result;

    for ( size_t f = 0; f < 1; ++f )
    {
        switch ( f )
        {
        case 0: list = FileFGJsonParameterMap::getFileNameExtensions( ); break;
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
 * Creates a list of known image file name extensions.
 *
 * @return
 *   Returns a string containing a comma-separated list of extensions.
 */
std::string getImageFileFormatExtensionList( )
    noexcept
{
    return "gif, jpg, jpeg, png";
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
// Edits
//
//----------------------------------------------------------------------
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
 */
inline std::string getAndValidateJsonString(
    const char*const key,
    const JsonValue& nodeValue )
    noexcept
{
    if ( nodeValue.getTag( ) == JSON_STRING )
        return nodeValue.toString( );

    if ( nodeValue.getTag( ) != JSON_NULL )
        printErrorAndExit(
            "The JSON string is malformed. It includes a \"" +
            std::string( key ) +
            std::string( "\" key, but the value is not a string." ) );

    return "";
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
 */
inline double getAndValidateJsonNumber(
    const char*const key,
    const JsonValue& nodeValue )
    noexcept
{
    if ( nodeValue.getTag( ) != JSON_NUMBER )
        printErrorAndExit(
            "The JSON string is malformed. It includes a \"" +
            std::string( key ) +
            std::string( "\" key, but the value is not a number." ) );

    return nodeValue.toNumber( );
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
 */
std::shared_ptr<Transform> parseJsonTransform( const std::string &value )
    noexcept
{
    // Watch for empty strings and null transforms.
    if ( value.empty( ) == true )
        return nullptr;

    auto valueStart = std::find_if_not(
        value.begin( ),
        value.end( ),
        [](int c) { return std::isspace( c ); } );
    auto trimmed = std::string(
        valueStart,
        std::find_if_not(
            value.rbegin( ),
            std::string::const_reverse_iterator( valueStart ),
            [](int c) { return std::isspace( c ); } ).base( ) );

    if ( trimmed.empty( ) == true )
        return nullptr;
    if ( trimmed.compare( "null" ) == 0 )
        return nullptr;
    if ( trimmed.compare( "NULL" ) == 0 )
        return nullptr;
    if ( trimmed.compare( "\"null\"" ) == 0 )
        return nullptr;
    if ( trimmed.compare( "\"NULL\"" ) == 0 )
        return nullptr;

    // Copy the given value into a temp string that can be modified
    // by the JSON parse below.
    char*const string = new char[ trimmed.length( ) + 1 ];
    std::strcpy( string, trimmed.c_str( ) );

    // Parse the JSON.
    char* endptr;
    JsonValue rootValue;
    JsonAllocator allocator;
    const int status = jsonParse( string, &endptr, &rootValue, allocator );
    if ( status != JSON_OK )
    {
        delete string;
        printErrorAndExit(
            "The JSON transform is malformed and cannot be parsed.\n" );
    }

    // Declare/initialize transform attributes.
    TransformType transformType = CUSTOM_TRANSFORM;
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

    // Loop through the keys to find the transform type and other
    // transform attributes.
    for ( const auto child: rootValue )
    {
        auto& key = child->key;
        emptyObject = false;
        if ( std::strcmp( key, "transformType" ) == 0 )
        {
            transformTypeName =
                getAndValidateJsonString( key, child->value );

            // Convert to a numeric gate type. If the type is
            // not recognized, CUSTOM_TRANSFORM is returned.
            transformType = findTransformTypeByName( transformTypeName );
            continue;
        }
        if ( std::strcmp( key, "name" ) == 0 )
        {
            name = getAndValidateJsonString( key, child->value );
            continue;
        }
        if ( std::strcmp( key, "description" ) == 0 )
        {
            description = getAndValidateJsonString( key, child->value );
            continue;
        }
        if ( std::strcmp( key, "a" ) == 0 )
        {
            a = getAndValidateJsonNumber( key, child->value );
            foundA = true;
            continue;
        }
        if ( std::strcmp( key, "m" ) == 0 )
        {
            m = getAndValidateJsonNumber( key, child->value );
            foundM = true;
            continue;
        }
        if ( std::strcmp( key, "t" ) == 0 )
        {
            t = getAndValidateJsonNumber( key, child->value );
            foundT = true;
            continue;
        }
        if ( std::strcmp( key, "w" ) == 0 )
        {
            w = getAndValidateJsonNumber( key, child->value );
            foundW = true;
            continue;
        }

        // Ignore other keys.
    }
    delete string;

    if ( emptyObject == true )
        return nullptr;

    //
    // Create transform.
    // -----------------
    // Use the transform type to select the type of transform to
    // construct.
    std::shared_ptr<Transform> transform;

    try
    {
        switch ( transformType )
        {
        case PARAMETERIZED_LINEAR_TRANSFORM:
            if ( foundT == false || foundA == false )
            {
                printErrorAndExit(
                    "The JSON transform describes a parameterized linear transform, but does not provide both T and A argument values." );
            }
            transform.reset(
                new ParameterizedLinearTransform( t, a ) );
            break;

        case PARAMETERIZED_LOGARITHMIC_TRANSFORM:
            if ( foundT == false || foundM == false )
            {
                printErrorAndExit(
                    "The JSON transform describes a parameterized logarithmic transform, but does not provide both T and M argument values." );
            }
            transform.reset(
                new ParameterizedLogarithmicTransform( t, m ) );
            break;

        case PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM:
            if ( foundT == false || foundA == false || foundM == false )
            {
                printErrorAndExit(
                    "The JSON transform describes a parameterized inverse hyperbolic sine transform, but does not provide all three T, A, and M argument values." );
            }
            transform.reset(
                new ParameterizedInverseHyperbolicSineTransform( t, a, m ) );
            break;

        case LOGICLE_TRANSFORM:
            if ( foundT == false || foundA == false ||
                foundM == false || foundW == false )
            {
                printErrorAndExit(
                    "The JSON transform describes a logicle transform, but does not provide all four T, A, M, and W argument values." );
            }
            transform.reset(
                new LogicleTransform( t, a, m, w ) );
            break;

        case HYPERLOG_TRANSFORM:
            if ( foundT == false || foundA == false ||
                foundM == false || foundW == false )
            {
                printErrorAndExit(
                    "The JSON transform describes a hyperlog transform, but does not provide all four T, A, M, and W argument values." );
            }
            transform.reset(
                new HyperlogTransform( t, a, m, w ) );
            break;

        default:
        case CUSTOM_TRANSFORM:
            printErrorAndExit(
                std::string( "The JSON transform describes a transform with a transform type of \"" ) +
                transformTypeName +
                std::string( "\", but this transform type is not recognized." ) );
        }
    }
    catch ( const std::exception& e )
    {
        // Fail to create transform.
        printErrorAndExit( e.what( ) );
    }

    transform->setName( name );
    transform->setDescription( description );

    return transform;
}

/**
 * Applies edits set on the command-line.
 *
 * @param gatingCache
 *   The gating cache used in gating and plotting.
 * @param gates
 *   The linear gate list pointing to gates in the gating cache's gate trees.
 */
void applyEdits(
    std::shared_ptr<GatingCache> gatingCache,
    std::vector<std::pair<GateIndex,std::shared_ptr<Gate>>> gates )
    noexcept
{
    if ( application.edits.size( ) == 0 )
        return;

    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ": Editing gate tree.\n";

    auto gateTrees        = gatingCache->getGateTrees( );
    auto sourceEventTable = gatingCache->getSourceEventTable( );
    const auto nGates     = gates.size( );


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
        if ( key.compare( "setgatetreedescription" ) == 0 )
        {
            // TUPLE ( *, *, DESCRIPTION )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate tree description to \"" << value3 << "\".\n";
            gateTrees->setDescription( value3 );
            continue;
        }
        if ( key.compare( "setgatetreename" ) == 0 )
        {
            // TUPLE ( *, *, NAME )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate tree name to \"" << value3 << "\".\n";
            gateTrees->setName( value3 );
            continue;
        }
        if ( key.compare( "setgatetreenotes" ) == 0 )
        {
            // TUPLE ( *, *, NOTES )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate tree notes to \"" << value3 << "\".\n";
            gateTrees->setNotes( value3 );
            continue;
        }

        //
        // Gate edits.
        // - All of these edits have two values:
        //   - A numeric gate index.
        //   - The value to use for that gate
        // - All keys have the gate index appended to make them unique.
        //
        const auto gateIndex = std::stoul( value1 );
        auto keyNoGateIndex = key.substr(
            0,
            key.length( ) - value1.length( ) );

        if ( gateIndex >= nGates )
            printErrorAndExit(
                "Gate index \"" +
                value1 +
                "\" for --" + keyNoGateIndex + " is out of range.\n");

        auto gate = gates[gateIndex].second;

        if ( keyNoGateIndex.compare( "setgatedescription" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, DESCRIPTION )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" description to \"" << value3 << "\".\n";
            gate->setDescription( value3 );
            continue;
        }
        if ( keyNoGateIndex.compare( "setgatemethod" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, METHOD )
            const GatingMethod method = findGatingMethodByName( value3 );
            if ( method == CUSTOM_GATING )
                printErrorAndExit(
                    "Unknown gating method \"" + value3 + "\".\n" );

            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" method to \"" << value3 << "\".\n";
            gate->setGatingMethod( method );
            continue;
        }
        if ( keyNoGateIndex.compare( "setgatename" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, NAME )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" name to \"" << value3 << "\".\n";
            gate->setName( value3 );
            continue;
        }
        if ( keyNoGateIndex.compare( "setgatenotes" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, NOTES )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" notes to \"" << value3 << "\".\n";
            gate->setNotes( value3 );
            continue;
        }
        if ( keyNoGateIndex.compare( "setgatereportpriority" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, PRIORITY )
            const auto priority = std::stoul( value3 );
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" report priority to \"" << priority << "\".\n";
            gate->setReportPriority( priority );
            continue;
        }
        if ( keyNoGateIndex.compare( "enablegateadditionalparameter" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, NAME )
            // Verify that the named parameter exists in the source.
            if ( sourceEventTable->isParameter( value3 ) == false )
                printErrorAndExit(
                    "Unknown parameter name \"" + value3 + "\".\n" );

            // Check if the parameter is already in use as a dimension
            // parameter.
            if ( gate->isDimensionParameter( value3 ) == true )
                printErrorAndExit(
                    "Parameter \"" + value3 + "\" is already used as a gate parameter.\nIt cannot be used for additional clustering too.\n" );

            // If the parameter is not already in the list as an additional
            // clustering parameter, add it.
            if ( gate->isAdditionalClusteringParameter( value3 ) == false )
            {
                if ( application.verbose == true )
                    std::cerr << application.applicationName <<
                        ":   gate \"" << gateIndex <<
                        "\" enable additional clustering parameter \"" <<
                        value3 << "\".\n";
                try
                {
                    gate->appendAdditionalClusteringParameter( value3, NULL );
                }
                catch ( const std::exception& e )
                {
                    // Fail. Parameter name is empty or additional clustering
                    // parameters are not supported for this gate.
                    printErrorAndExit(
                        "Cannot enable additional clustering parameter \"" +
                        value3 + "\" for gate \"" +
                        value1 + "\": " + e.what( ) );
                }
            }
            continue;
        }
        if ( keyNoGateIndex.compare( "disablegateadditionalparameter" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, NAME )
            if ( sourceEventTable->isParameter( value3 ) == false )
                printErrorAndExit(
                    "Unknown parameter name \"" + value3 + "\".\n" );
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" disable additional clustering parameter \"" <<
                    value3 << "\".\n";
            try
            {
                gate->removeAdditionalClusteringParameter( value3 );
            }
            catch ( const std::exception& e )
            {
                // Fail. Parameter name is empty, additional clustering
                // parameters are not supported for this gate, or the
                // parameter not in the list.
                printErrorAndExit(
                    "Cannot disable additional clustering parameter \"" +
                    value3 + "\" for gate \"" +
                    value1 + "\": " + e.what( ) );
            }
            continue;
        }
        if ( keyNoGateIndex.compare( "setgateadditionalparametertransform" ) == 0 )
        {
            // TUPLE ( GATEINDEX, NAME, TRANSFORM )
            if ( sourceEventTable->isParameter( value2 ) == false )
                printErrorAndExit(
                    "Unknown parameter name \"" + value2 + "\".\n" );
            if ( gate->isAdditionalClusteringParameter( value2 ) == false )
                printErrorAndExit(
                    "Parameter name \"" + value2 + "\" is not an additional clustering parameter.\n" );
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" set additional clustering parameter \"" <<
                    value2 << "\" transform \"" <<
                    value3 << "\".\n";
            try
            {
                gate->setAdditionalClusteringParameterTransform(
                    value2,
                    parseJsonTransform( value3 ) );
            }
            catch ( const std::exception& e )
            {
                // Fail. Parameter name is empty, or additional clustering
                // parameters are not supported for this gate. If the JSON
                // transform could not be parsed, an error message has
                // already been written out and the application exited.
                printErrorAndExit(
                    "Cannot set additional clustering parameter \"" +
                    value3 + "\" transform for gate \"" +
                    value1 + "\": " + e.what( ) );
            }
            continue;
        }
        if ( keyNoGateIndex.compare( "deletegate" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, * )
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   delete gate \"" << gateIndex << "\".\n";

            // Unlike the --set* options above, deletion is not related to
            // the current gate. Instead, the given gate index selects the
            // gate in the tree and deletes it, independent of the current
            // gate choice.
            auto parentGateIndex = gates[gateIndex].first;
            if ( parentGateIndex == gateIndex )
            {
                // The indicated gate is a root gate.
                auto rootGate = gates[gateIndex].second;
                gateTrees->removeGateTree( rootGate );
            }
            else
            {
                // The indicated gate is a child of the parent.
                auto parentGate = gates[parentGateIndex].second;
                auto childGate = gates[gateIndex].second;
                parentGate->removeChild( childGate );
            }

            // After deletion, the given gate list is out of date.
            gates = gateTrees->findDescendentGatesWithParentIndexes( );
            continue;
        }
        if ( keyNoGateIndex.compare( "setgateparametervertices" ) == 0 )
        {
            // TUPLE ( GATEINDEX, *, VERTICES )
            // The gate must be a PolygonGate.
            auto p = dynamic_cast<PolygonGate*>(gate.get());
            if ( p == nullptr )
                printErrorAndExit(
                    "Cannot set vertices for gate \"" +
                    std::to_string(gateIndex) +
                    "\" that is not a polygon gate.\n" );

            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" parameter vertices to \"" << value3 << "\".\n";

            // The value is a comma-separated list of values. Each N
            // of them is a vertex, where N is the number of dimensions
            // for the gate. For polygons, N is always 2.
            std::vector<double> numbers;
            size_t startIndex = 0;
            size_t commaIndex = value3.find(',', startIndex);
            while ( commaIndex != std::string::npos )
            {
                numbers.push_back( std::stod(
                    value3.substr( startIndex, startIndex - commaIndex ) ) );
                startIndex = commaIndex + 1;
                commaIndex = value3.find(',', startIndex);
            }

            numbers.push_back( std::stod(
                value3.substr( startIndex, commaIndex ) ) );

            // Loop over the numbers in groups of 2 (for X and Y).
            const size_t nNumbers = numbers.size( );
            if ( (nNumbers%2) != 0 )
                printErrorAndExit(
                    "Cannot set vertices for gate \"" +
                    std::to_string(gateIndex) +
                    "\" using an odd number of values.\n" );

            p->clearVertices( );
            for ( size_t i = 0; i < nNumbers; i += 2 )
                p->appendVertex( numbers[i], numbers[i+1] );
            continue;
        }

        //
        // Gate dimension edits.
        // - All of these edits have three values:
        //   - A numeric gate index.
        //   - A numeric dimension index.
        //   - The value to use for that gate
        //
        const auto dimensionIndex = std::stoul( value2 );
        auto keyNoGateOrDimensionIndex = key.substr(
            0,
            key.length( ) - value1.length( ) - value2.length( ) );

        if ( dimensionIndex > gate->getNumberOfDimensions( ) )
            printErrorAndExit(
                "Gate dimension index \"" +
                value2 +
                "\" for --" + keyNoGateOrDimensionIndex + " is out of range.\n");
        if ( keyNoGateOrDimensionIndex.compare( "setgateparametername" ) == 0 )
        {
            // Verify that the named parameter exists in the source.
            if ( sourceEventTable->isParameter( value3 ) == false )
                printErrorAndExit(
                    "Unknown parameter name \"" + value3 + "\".\n" );

            // Check if the parameter is already in use as an additional
            // clustering parameter.
            if ( gate->isAdditionalClusteringParameter( value3 ) == true )
                printErrorAndExit(
                    "Parameter \"" + value3 + "\" is already used for the gate's additional\nclustering. It cannot be used as a gate dimension parameter too.\n" );

            // If the parameter is not already assigned to the dimension,
            // assign it.
            if ( gate->getDimensionParameterName( dimensionIndex ).compare( value3 ) == 0 )
            {
                if ( application.verbose == true )
                    std::cerr << application.applicationName <<
                        ":   set gate \"" << gateIndex <<
                        "\" dimension \"" << dimensionIndex <<
                        "\" parameter name to \"" << value3 << "\".\n";
                gate->setDimensionParameterName(
                    dimensionIndex,
                    value3 );
            }
            continue;
        }
        if ( keyNoGateOrDimensionIndex.compare( "setgateparametertransform" ) == 0 )
        {
            // The value is JSON text giving the transform's values.
            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" dimension \"" << dimensionIndex <<
                    "\" parameter transform to \"" << value3 << "\".\n";
            gate->setDimensionParameterTransform(
                dimensionIndex,
                parseJsonTransform( value3 ) );
            continue;
        }
        if ( keyNoGateOrDimensionIndex.compare( "setgateparameterminimum" ) == 0 )
        {
            // The gate must be a RectangleGate.
            auto r = dynamic_cast<RectangleGate*>(gate.get());
            if ( r == nullptr )
                printErrorAndExit(
                    "Cannot set minimum for gate \"" +
                    std::to_string(gateIndex) +
                    "\" that is not a rectangle gate.\n" );

            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" dimension \"" << dimensionIndex <<
                    "\" parameter minimum to \"" << value3 << "\".\n";
            r->setDimensionMinimum(
                dimensionIndex,
                std::stod( value3 ) );
            continue;
        }
        if ( keyNoGateOrDimensionIndex.compare( "setgateparametermaximum" ) == 0 )
        {
            // The gate must be a RectangleGate.
            auto r = dynamic_cast<RectangleGate*>(gate.get());
            if ( r == nullptr )
                printErrorAndExit(
                    "Cannot set maximum for gate \"" +
                    std::to_string(gateIndex) +
                    "\" that is not a rectangle gate.\n" );

            if ( application.verbose == true )
                std::cerr << application.applicationName <<
                    ":   set gate \"" << gateIndex <<
                    "\" dimension \"" << dimensionIndex <<
                    "\" parameter maximum to \"" << value3 << "\".\n";
            r->setDimensionMaximum(
                dimensionIndex,
                std::stod( value3 ) );
            continue;
        }
    }
}





//----------------------------------------------------------------------
//
// Plot utilities.
//
//----------------------------------------------------------------------
/**
 * Creates and initializes a scatter plot for each thread.
 *
 * To minimize memory allocation costs, each thread reuses the same
 * scatter plot object for all of its plots, clearing it for each new plot.
 *
 * @return
 *   Returns a vector of scatter plot objects, one per thread.
 */
std::vector<ScatterPlot*> createPlotObjects( )
    noexcept
{
    // Create a vector of plot object pointers.
    std::vector<ScatterPlot*> scatterPlots;
    scatterPlots.reserve( application.numberOfThreads );

    // Create and initialize the first plot object.
    ScatterPlot*const plot0 = new ScatterPlot(
        visual.imageWidth,
        visual.imageHeight );
    plotSetup( plot0 );
    scatterPlots.push_back( plot0 );

    // Create all further plot objects by copying the first one.
    // Copying a previous object's setup is faster than doing the
    // setup from scratch on a new object.
    for ( uint32_t i = 1; i < application.numberOfThreads; ++i )
        scatterPlots.push_back( new ScatterPlot( plot0 ) );

    return scatterPlots;
}

/**
 * Deletes previously created plots per thread.
 *
 * @param[in] scatterPlots
 *   The vector of plot objects to destroy.
 */
void deletePlotObjects( std::vector<ScatterPlot*>& scatterPlots )
    noexcept
{
    for ( uint32_t i = 0; i < application.numberOfThreads; ++i )
        delete scatterPlots[i];
    scatterPlots.clear( );
}





//----------------------------------------------------------------------
//
// Plot.
//
//----------------------------------------------------------------------

/**
 * Sets up a scatter plot with common attributes.
 *
 * Common attributes include colors, fonts, and sizes that are not dependent
 * upon the data being plotted. These are all based upon visual parameters,
 * most of which may be set from the command line.
 *
 * @param[in] plot
 *   The scatter plot to set up.
 */
void plotSetup( ScatterPlot*const plot )
    noexcept
{
    if ( application.verbose == true )
    {
        std::cerr << application.applicationName << ":\n";
        std::cerr << application.applicationName <<
            ": Setting up plot:\n";
    }


    //
    // Get colors.
    // -----------
    // Get the default colors.
    const int cWhite          = plot->getWhite( );
    const int cBlack          = plot->getBlack( );

    // Allocate and assign additional colors.
    const int cTitle          = plot->allocateColor( visual.titleColor );
    const int cAxisTitle      = (visual.axisTitleColor == 0) ?
        cBlack :
        plot->allocateColor( visual.axisTitleColor );
    const int cAxisNumber     = (visual.axisNumberColor == 0) ?
        cBlack :
        plot->allocateColor( visual.axisNumberColor );
    const int cAxisTic        = plot->allocateColor( visual.axisTicColor );
    const int cPlotBackground = plot->allocateColor( visual.backgroundColor );
    const int cPlotBorder     = plot->allocateColor( visual.borderLineColor );

    // Allocate all dot colors.
    visual.dotColorIndexes.clear( );
    size_t nColors = visual.dotColors.size( );
    for ( size_t i = 0; i < nColors; ++i )
        visual.dotColorIndexes.push_back(
            plot->allocateColor( visual.dotColors[i] ) );

    // Allocate all gate background colors.
    visual.gateFillColorIndexes.clear( );
    nColors = visual.gateFillColors.size( );
    for ( size_t i = 0; i < nColors; ++i )
        visual.gateFillColorIndexes.push_back(
            plot->allocateColor( visual.gateFillColors[i] ) );


    //
    // Set image attributes.
    // ---------------------
    // Set the image background and title colors, the plot's X-Y margins,
    // the plot area's background and border color, and the border thickness.
    plot->setBackgroundColor(     cWhite );
    plot->setTitleColor(          cTitle );
    plot->setPlotBackgroundColor( cPlotBackground );
    plot->setPlotBorderColor(     cPlotBorder );

    plot->setPlotXMargins(
        (int)(visual.imageMarginLeftPercent * visual.imageWidth),
        (int)(visual.imageMarginRightPercent * visual.imageWidth) );
    plot->setPlotYMargins(
        (int)(visual.imageMarginBottomPercent * visual.imageHeight),
        (int)(visual.imageMarginTopPercent * visual.imageHeight) );
    plot->setPlotBorderLineWidth( visual.borderLineWidth );

    plot->setTitleFontSize(
        (int)(visual.titleFontSizePercent * visual.imageWidth) );
    plot->setTitleFontNames( visual.titleFontNames );

    if ( visual.showLegend == true )
    {
        plot->setLegendFontSize(
            (int)(visual.legendFontSizePercent * visual.imageWidth) );
        plot->setLegendFontNames( visual.legendFontNames );
    }


    //
    // Set X and Y axis attributes.
    // ----------------------------
    // Set the axis title, number, and tic mark colors, the tic length and
    // thickness. The axis ranges and tic marks, however, are dependent upon
    // the parameters being plotted, which varies from plot to plot and
    // therefore cannot be set generically here.
    plot->setAxisTitleColor(  cAxisTitle );
    plot->setAxisNumberColor( cAxisNumber );
    plot->setAxisTicColor(    cAxisTic );

    plot->setAxisTicLength(
        (int)(visual.axisTicLength * visual.imageWidth) );
    plot->setAxisTicLineWidth( visual.axisTicLineWidth );
    plot->setAxisXMaximumDigitsAfterDecimal( visual.axisNumberMaxDigitsAfterDecimal );
    plot->setAxisYMaximumDigitsAfterDecimal( visual.axisNumberMaxDigitsAfterDecimal );

    plot->setAxisXTitleFontNames( visual.axisTitleFontNames );
    plot->setAxisYTitleFontNames( visual.axisTitleFontNames );
    plot->setAxisXTitleFontSize(
        (int)(visual.axisTitleFontSizePercent * visual.imageWidth) );
    plot->setAxisYTitleFontSize(
        (int)(visual.axisTitleFontSizePercent * visual.imageWidth) );
    plot->setAxisXTitleMargin(
        (int)(visual.axisXTitleMarginPercent * visual.imageHeight) );

    plot->setAxisXNumberFontNames( visual.axisNumberFontNames );
    plot->setAxisYNumberFontNames( visual.axisNumberFontNames );
    plot->setAxisXNumberFontSize(
        (int)(visual.axisNumberFontSizePercent * visual.imageWidth) );
    plot->setAxisYNumberFontSize(
        (int)(visual.axisNumberFontSizePercent * visual.imageWidth) );
    plot->setAxisYTitleMargin(
        (int)(visual.axisYTitleMarginPercent * visual.imageWidth) );


    //
    // Dot attributes.
    // ---------------
    // Set the dot size and shape.
    plot->setDotSize( visual.dotSize );
    switch ( visual.dotShape )
    {
        default:
        case DOT_CIRCLE:
            plot->setDotCircle( );
            break;

        case DOT_SQUARE:
            plot->setDotSquare( );
            break;
    }
}

/**
 * Creates a scatter plot using the given data and current visuals.
 *
 * The current visual parameters, set up via the command-line, are used
 * to plot the selected gate.
 *
 * @param[in] plot
 *   The scatter plot to draw into.
 * @param[in] eventTable
 *   The source event table. Used for long parameter names only.
 * @param[in] parameterMap
 *   The optional parameter map. Used for long parameter names only.
 * @param[in] gate
 *   The parent gate.
 * @param[in] gate
 *   The gate to plot.
 * @param[in] gateIndex
 *   The gate's index.
 * @param[in] numberOfEventsToPlot
 *   The number of events to plot, of course.
 */
void plot(
    ScatterPlot*const plot,
    std::shared_ptr<const EventTableInterface> eventTable,
    std::shared_ptr<const ParameterMap> parameterMap,
    std::shared_ptr<const Gate> parent,
    std::shared_ptr<const Gate> gate,
    const GateIndex gateIndex,
    const size_t numberOfEventsToPlot )
    noexcept
{
    const double timeAtStartOfPlot = getRealTime( );

    //
    // Set up gate data.
    // -----------------
    // Get gate state.
    const auto state =
        std::dynamic_pointer_cast<const GateState>( gate->getState( ) );
    const auto parentState = (parent == nullptr) ?
        nullptr :
        std::dynamic_pointer_cast<const GateState>( parent->getState( ) );

    const auto numberOfDimensions = gate->getNumberOfDimensions( );


    //
    // Set up axis names.
    // ------------------
    // Start with the X (and optionally Y) short axis names from the gate.
    //
    // If there is a parameter map, get the optional longer name from the map.
    //
    // Otherwise get the optional longer name from the event table.
    std::string xAxisName = gate->getDimensionParameterName( 0 );
    std::string yAxisName = "";
    if ( numberOfDimensions > 1 )
        yAxisName = gate->getDimensionParameterName( 1 );

    if ( parameterMap != nullptr )
    {
        auto s = parameterMap->findParameterLongName( xAxisName );
        if ( s.empty( ) == false )
            xAxisName = s;

        if ( numberOfDimensions > 1 )
        {
            s = parameterMap->findParameterLongName( yAxisName );
            if ( s.empty( ) == false )
                yAxisName = s;
        }
    }
    else if ( eventTable != nullptr )
    {
        try
        {
            auto index = eventTable->getParameterIndex( xAxisName );
            auto s = eventTable->getParameterLongName( index );
            if ( s.empty( ) == false )
                xAxisName = s;
        }
        catch ( ... )
        {
        }

        if ( numberOfDimensions > 1 )
        {
            try
            {
                auto index = eventTable->getParameterIndex( yAxisName );
                auto s = eventTable->getParameterLongName( index );
                if ( s.empty( ) == false )
                    yAxisName = s;
            }
            catch ( ... )
            {
            }
        }
    }


    //
    // Set decorations.
    // ---------------
    // Set plot-specific attributes, including the plot title, axis titles, ...
    const auto name = gate->getName( );
    if ( name.empty( ) == false )
    {
        plot->setTitle( gate->getName( ) );
    }
    else
    {
        std::string generatedName = xAxisName;
        if ( numberOfDimensions > 1 )
            generatedName += " x " + yAxisName;
        plot->setTitle( generatedName );
    }

    // Set the axis name, range, and tic marks.
    //
    // For a 1D gate, there are only X min and max. For a 2D gate, there
    // are also Y min and max. For ND, we only support the first two
    // dimensions.
    //
    // Note that we use the "best" min/max. Normally, this will return the
    // min/max specified for the parameter as based upon the hardware and
    // software used to acquire the data. However, occassionally that min/max
    // is either not given or is incorrect and the actual data has a different
    // min/max range. The "best" min/max determines if the specified min/max
    // are usable, or if the actual data min/max is needed, and returns the
    // best choice.
    plot->setAxisXTitle( xAxisName );

    const double xAxisMin = state->getParameterBestMinimum( 0 );
    const double xAxisMax = state->getParameterBestMaximum( 0 );
    plot->setAxisXRange( xAxisMin, xAxisMax );

    std::vector<float> xTics;
    double value = xAxisMin;
    double delta = (xAxisMax - xAxisMin) /
        (double) (DEFAULT_AXIS_NUMBER_OF_TICS - 1);
    for ( size_t i = 0; i < DEFAULT_AXIS_NUMBER_OF_TICS; ++i )
    {
        xTics.push_back( (float) value );
        value += delta;
    }
    plot->setAxisXTics( xTics );

    // Create Y-axis tic marks.
    if ( numberOfDimensions > 1 )
    {
        plot->setAxisYTitle( yAxisName );

        const double yAxisMin = state->getParameterBestMinimum( 1 );
        const double yAxisMax = state->getParameterBestMaximum( 1 );
        plot->setAxisYRange( yAxisMin, yAxisMax );

        std::vector<float> yTics;
        value = yAxisMin;
        delta = (yAxisMax - yAxisMin) /
            (double) (DEFAULT_AXIS_NUMBER_OF_TICS - 1);
        for ( size_t i = 0; i < DEFAULT_AXIS_NUMBER_OF_TICS; ++i )
        {
            yTics.push_back( (float) value );
            value += delta;
        }
        plot->setAxisYTics( yTics );
    }
    else
        plot->setAxisYRange( 0.0, 0.0 );


    //
    // Colors and legend.
    // ------------------
    // Colors are needed for the dots, gate fill, etc.
    plot->clearLegend( );
    if ( visual.showLegend == true )
    {
        // Set up the legend text and build the aggregate population name
        // used later for the fame.
        //
        // Since the legend needs to show the top item as the top drawn item,
        // the second item as the drawn item below the top one, and so on.
        // The legend order is the REVERSE of the drawing order. The last item
        // in the legend is the optional parent population, which is actually
        // the first population drawn (and then overdrawn by other populations).
        std::vector<std::string> legendText;
        std::vector<int> legendColors;

        legendText.push_back( gate->getName( ) );
        legendColors.push_back(
            visual.dotColorIndexes[GATED_POPULATION_COLOR_LIST_INDEX] );

        if ( visual.showParentPopulation == true && parent != nullptr )
        {
            legendText.push_back( parent->getName( ) );
            legendColors.push_back(
                visual.dotColorIndexes[PARENT_POPULATION_COLOR_LIST_INDEX] );
        }

        plot->setLegend( legendText, legendColors );
    }


    //
    // Draw background.
    // ----------------
    // Clear and draw the plot background.
    plot->clearPlotArea( );

    // Draw the filled gate in the background. Data dots plotted
    // below are atop the filled gate.
    if ( visual.showFilledGates == true )
    {
        if ( gate->getGateType( ) == RECTANGLE_GATE )
        {
            const auto r = dynamic_cast<const RectangleGate*>(gate.get());
            double xRectangleMin = 0.0, xRectangleMax = 0.0;
            double yRectangleMin = 0.0, yRectangleMax = 0.0;
            r->getDimensionMinimumMaximum( 0, xRectangleMin, xRectangleMax );
            if ( numberOfDimensions > 1 )
                r->getDimensionMinimumMaximum( 1, yRectangleMin, yRectangleMax );

            plot->plotFilledRectangle(
                xRectangleMin, yRectangleMin,
                xRectangleMax, yRectangleMax,
                visual.gateFillColorIndexes[GATED_POPULATION_COLOR_LIST_INDEX] );
        }
        else if ( gate->getGateType( ) == POLYGON_GATE )
        {
            const auto p = dynamic_cast<const PolygonGate*>(gate.get());
            const auto xCoordinates = p->getVerticesX( );
            const auto yCoordinates = p->getVerticesY( );

            plot->plotFilledPolygon(
                xCoordinates,
                yCoordinates,
                visual.gateFillColorIndexes[GATED_POPULATION_COLOR_LIST_INDEX] );
        }
        else if ( gate->getGateType( ) == ELLIPSOID_GATE )
        {
            // Not yet supported.
            if ( application.verbose == true )
            {
                std::cerr << application.applicationName << ": Ellipsoid gates are not supported.\n";
            }
        }
        else if ( gate->getGateType( ) == QUADRANT_GATE )
        {
            // Not yet supported.
            if ( application.verbose == true )
            {
                std::cerr << application.applicationName << ": Quadrant gates are not supported.\n";
            }
        }
        else if ( gate->getGateType( ) == BOOLEAN_GATE )
        {
            // Boolean gates have no specific shape, so there is nothing
            // to draw here.
        }
        else
        {
            // Unrecognized gate type.
            if ( application.verbose == true )
            {
                std::cerr << application.applicationName << ": Unrecognized gate type is not supported.\n";
            }
        }
    }


    //
    // Plot data.
    // ----------
    // There are three "populations" of event dots to plot:
    //
    // 1. Events within the current gate.
    //
    // 2. Events within the parent gate, but not within the current gate.
    //
    // 3. Events within the overall event list, but not within the parent gate
    //    or the current gate.
    //
    // Population 2, the parent gate's events, is merged with population 3,
    // everything else, if:
    //
    // - There is no parent gate because the current gate is a root gate.
    //
    // - There is a parent gate, but 'showParentPopulation' is FALSE.
    int dotColorIndex;

    // Decide if the parent population is needed.
    const bool showParentPopulation =
        (visual.showParentPopulation == true) && (parent != nullptr);

    // Get pointers to the current and parent gate's event inclusion flags.
    // If there is no parent, or no need to check the parent's gate flags,
    // then skip that pointer.
    const uint8_t*const inclusion =
        state->getEventIncludedList( ).data( );

    const uint8_t*const parentInclusion =
        (showParentPopulation == true) ?
        parentState->getEventIncludedList( ).data( ) :
        nullptr;


    // Get float or double pointers to the current gate's X and Y events.
    const bool areFloats = state->areValuesFloats( );

    const float*const xEventFloat =
        (areFloats == true) ?
        state->getParameterFloats( 0 ).data( ) :
        nullptr;
    const float*const yEventFloat = (numberOfDimensions <= 1) ? nullptr :
        ((areFloats == true) ?
        state->getParameterFloats( 1 ).data( ) :
        nullptr);

    const double*const xEventDouble =
        (areFloats == false) ?
        state->getParameterDoubles( 0 ).data( ) :
        nullptr;
    const double*const yEventDouble = (numberOfDimensions <= 1) ? nullptr :
        ((areFloats == false) ?
        state->getParameterDoubles( 1 ).data( ) :
        nullptr);

    // Population 3. Everything that is neither within the current gate or the
    // parent gate.
    dotColorIndex = visual.dotColorIndexes[UNGATED_POPULATION_COLOR_LIST_INDEX];

    if ( showParentPopulation == false )
    {
        // Draw all event dots that are NOT included in the current gate.
        if ( areFloats == true )
        {
            for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
            {
                if ( inclusion[i] == 0 )
                    plot->plotDot(
                        xEventFloat[i],
                        (numberOfDimensions > 1) ? yEventFloat[i] : 0.0,
                        dotColorIndex );
            }
        }
        else
        {
            for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
            {
                if ( inclusion[i] == 0 )
                    plot->plotDot(
                        xEventDouble[i],
                        (numberOfDimensions > 1) ? yEventDouble[i] : 0.0,
                        dotColorIndex );
            }
        }
    }
    else
    {
        // Draw all event dots that are NOT included in the current gate
        // or the parent gate.
        if ( areFloats == true )
        {
            for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
            {
                if ( inclusion[i] == 0 || parentInclusion[i] == 0 )
                    plot->plotDot(
                        xEventFloat[i],
                        (numberOfDimensions > 1) ? yEventFloat[i] : 0.0,
                        dotColorIndex );
            }
        }
        else
        {
            for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
            {
                if ( inclusion[i] == 0 || parentInclusion[i] == 0 )
                    plot->plotDot(
                        xEventDouble[i],
                        (numberOfDimensions > 1) ? yEventDouble[i] : 0.0,
                        dotColorIndex );
            }
        }
    }

    // Population 2. Everything that is within the parent gate but not within
    // the current gate.
    if ( showParentPopulation == true )
    {
        dotColorIndex =
            visual.dotColorIndexes[PARENT_POPULATION_COLOR_LIST_INDEX];

        if ( areFloats == true )
        {
            for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
            {
                if ( inclusion[i] == 0 && parentInclusion[i] != 0 )
                    plot->plotDot(
                        xEventFloat[i],
                        (numberOfDimensions > 1) ? yEventFloat[i] : 0.0,
                        dotColorIndex );
            }
        }
        else
        {
            for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
            {
                if ( inclusion[i] == 0 && parentInclusion[i] != 0 )
                    plot->plotDot(
                        xEventDouble[i],
                        (numberOfDimensions > 1) ? yEventDouble[i] : 0.0,
                        dotColorIndex );
            }
        }
    }

    // Population 1. Everything that is within the current gate.
    dotColorIndex = visual.dotColorIndexes[GATED_POPULATION_COLOR_LIST_INDEX];

    if ( areFloats == true )
    {
        for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
        {
            if ( inclusion[i] != 0 )
                plot->plotDot(
                    xEventFloat[i],
                    (numberOfDimensions > 1) ? yEventFloat[i] : 0.0,
                    dotColorIndex );
        }
    }
    else
    {
        for ( size_t i = 0; i < numberOfEventsToPlot; ++i )
        {
            if ( inclusion[i] != 0 )
                plot->plotDot(
                    xEventDouble[i],
                    (numberOfDimensions > 1) ? yEventDouble[i] : 0.0,
                    dotColorIndex );
        }
    }


    //
    // Draw foreground.
    // ----------------
    // Draw the gate outline, starting with the "shadow".
    if ( gate->getGateType( ) == RECTANGLE_GATE )
    {
        const auto r = dynamic_cast<const RectangleGate*>(gate.get());
        double xRectangleMin = 0.0, xRectangleMax = 0.0;
        double yRectangleMin = 0.0, yRectangleMax = 0.0;
        r->getDimensionMinimumMaximum( 0, xRectangleMin, xRectangleMax );
        if ( numberOfDimensions > 1 )
            r->getDimensionMinimumMaximum( 1, yRectangleMin, yRectangleMax );

        if ( visual.showGateShadows == true )
            plot->plotRectangle(
                xRectangleMin, yRectangleMin,
                xRectangleMax, yRectangleMax,
                visual.gateShadowColorIndex, 2 * visual.gateLineWidth );

        plot->plotRectangle(
            xRectangleMin, yRectangleMin,
            xRectangleMax, yRectangleMax,
            visual.dotColorIndexes[GATED_POPULATION_COLOR_LIST_INDEX],
            visual.gateLineWidth );
    }
    else if ( gate->getGateType( ) == POLYGON_GATE )
    {
        const auto p = dynamic_cast<const PolygonGate*>(gate.get());
        const auto xCoordinates = p->getVerticesX( );
        const auto yCoordinates = p->getVerticesY( );

        if ( visual.showGateShadows == true )
            plot->plotPolygon(
                xCoordinates,
                yCoordinates,
                visual.gateShadowColorIndex, 2 * visual.gateLineWidth );

        plot->plotPolygon(
            xCoordinates,
            yCoordinates,
            visual.dotColorIndexes[GATED_POPULATION_COLOR_LIST_INDEX],
            visual.gateLineWidth );
    }
    else if ( gate->getGateType( ) == ELLIPSOID_GATE )
    {
        // Not yet supported.
    }
    else if ( gate->getGateType( ) == QUADRANT_GATE )
    {
        // Not yet supported.
    }
    else if ( gate->getGateType( ) == BOOLEAN_GATE )
    {
        // Boolean gates have no specific shape, so there is nothing
        // to draw here.
    }
    else
    {
        // Unrecognized gate type. Already reported earlier.
    }

    // Unfortunately, it is possible for the gate shape to extend outside
    // of the plot area. To block that, we need to clear the margins.
    plot->clearMargins( );

    // Draw the plot border, tic marks, axis titles, and axis numbers.
    // Drawing the border after the dots cleans up the edge of the plot.
    plot->drawPlotAreaBorder( );
    plot->drawAxisXTicsAndNumbers( plot->estimateAxisXDigitsAfterDecimal( ) );
    plot->drawAxisYTicsAndNumbers( plot->estimateAxisYDigitsAfterDecimal( ) );
    plot->drawAxisXTitles( );
    plot->drawAxisYTitles( );
    plot->drawTitle( );
    if ( visual.showLegend == true )
        plot->drawLegend( );


    const double timeAtEndOfPlot = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark( "Plot gates",
            (timeAtEndOfPlot - timeAtStartOfPlot) );


    //
    // Save the plot image.
    // --------------------
    // Strip off the filename extension, add axis information, then
    // append the correct extension based on the selected output file
    // format. Then save the plot.
    const size_t lastDotPosition = application.imagePath.rfind( '.' );
    std::string basename;
    if ( lastDotPosition == application.imagePath.npos )
        basename = application.imagePath;
    else
        basename = application.imagePath.substr( 0, lastDotPosition );

    std::string fileName = basename + "_" + std::to_string( gateIndex );

    switch ( application.imageFormat )
    {
        default:
        case PNG_INDEXED_COLOR:
        case PNG_TRUE_COLOR:
            fileName += ".png";
            break;

        case GIF_INDEXED_COLOR:
            fileName += ".gif";
            break;

        case JPG_INDEXED_COLOR:
        case JPG_TRUE_COLOR:
            fileName += ".jpg";
            break;
    }

    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ":   Saving image file \"" << fileName << "\".\n";

    const double timeAtStartOfSave = getRealTime( );

    plot->save( fileName );

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark( "Save plot image file",
            (timeAtEndOfSave - timeAtStartOfSave) );
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
 * Adds a benchmark time.
 *
 * @param[in] title
 *   The benchmark entry name.
 * @param[in] time
 *   The real time delta, in seconds.
 */
void addBenchmark( const std::string& title, const double time )
    noexcept
{
    application.benchmarkData.push_back( std::make_pair( title, time ) );
}

/**
 * Prints benchmarks in the current output format.
 */
void printBenchmarks( )
    noexcept
{
    const size_t n = application.benchmarkData.size( );
    if ( n == 0 )
        return;

    switch ( application.displayFormat )
    {
        default:
        case UNKNOWN_TEXT_FORMAT:
        case TEXT_FORMAT:
            for ( size_t i = 0; i < n; ++i )
            {
                auto title = application.benchmarkData[i].first;
                auto time  = application.benchmarkData[i].second;
                std::cout <<
                    std::left << std::setw(20) << title << " " <<
                    std::right << std::fixed << std::setprecision(8) << time <<
                    " sec" << std::endl;
            }
            break;

        case JSON_FORMAT:
            std::cout << "  \"benchmark\": {" << std::endl;
            for ( size_t i = 0; i < n; ++i )
            {
                auto title = application.benchmarkData[i].first;
                auto time  = application.benchmarkData[i].second;
                if ( i == (n-1) )
                    std::cout << "    \"" << title << "\": " <<
                        std::fixed << std::setprecision(8) << time <<
                        std::endl;
                else
                    std::cout << "    \"" << title << "\": " <<
                        std::fixed << std::setprecision(8) << time <<
                        "," << std::endl;
            }
            std::cout << "  }," << std::endl;
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
        " [options] cachefile [cachefileout]" << std::endl;
    std::cerr << "Usage is: " << application.applicationName <<
        " [options] eventfile gatefile [cachefileout]" << std::endl;
    std::cerr << "Cluster, gate, and scatter plot gate results." << std::endl;


    // Options
    // -------
    // Define a few formatting macros, then print help.
#define LISTOPTIONGROUP(title) \
    std::cerr << std::endl << std::left << std::setw(20) << \
    title << std::endl;

#define LISTOPTION(title,explain) \
    std::cerr << "  " << std::left << std::setw(25) << title << \
    " " << explain << std::endl;


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
    LISTOPTION( "--clusters N",
        "Number of initial clusters." );
    LISTOPTION( "--clusteriterations N",
        "Number of clusters iterations." );
    LISTOPTION( "--threads N",
        "Use up to N threads." );

    LISTOPTIONGROUP( "Format choice:" );
    LISTOPTION( "--format FORMAT",
        "Specify next file's format." );

    LISTOPTIONGROUP( "Input:" );
    LISTOPTION( "--maxevents N",
        "Maximum number of events to load." );
    LISTOPTION( "--gates G1,G2,G3,...",
        "A list of gate indexes for gates to plot." );
    LISTOPTION( "--parametermap file",
        "Optional parameter map for long names." );

    LISTOPTIONGROUP( "Output:" );
    LISTOPTION( "--imagebase PATH",
        "Specify plot image file basename." );
    LISTOPTION( "--bgwrite",
        "Enable program exit before cache file is written." );

    LISTOPTIONGROUP( "Plot attributes:" );
    LISTOPTION( "--plotaxistitlesize N",
        "Set axis title font size as % of image width." );
    LISTOPTION( "--plotaxisnumbersize N",
        "Set axis number font size as % of image width." );
    LISTOPTION( "--plotaxisnumbermaxdecimal N",
        "Set axis number max decimal digits." );
    LISTOPTION( "--plotaxisxtitlemargin N",
        "Set X axis title margin (bottom) as % of image height." );
    LISTOPTION( "--plotaxisytitlemargin N",
        "Set Y axis title margin (left) as % of image width." );
    LISTOPTION( "--plotdotshape S",
        "Set dot shape: circle, square." );
    LISTOPTION( "--plotdotsize N",
        "Set dot size." );
    LISTOPTION( "--plotfilledgates",
        "Enable filling the gate shapes on the plot." );
    LISTOPTION( "--plotshadows",
        "Enable shadows for the gate shapes on the plot." );
    LISTOPTION( "--plotlegend",
        "Enable a legend beside the plot." );
    LISTOPTION( "--plotlegendtitlesize N",
        "Set legend title font size as % of image width." );
    LISTOPTION( "--plotmargins L R T B",
        "Set image margins as % of image size." );
    LISTOPTION( "--plotparent",
        "Enable the parent population on the child's plot." );
    LISTOPTION( "--plotsize W H",
        "Set image size to wxh pixels." );
    LISTOPTION( "--plottitle T",
        "Set plot title." );
    LISTOPTION( "--plottitlesize N",
        "Set plot title font size as % of image width." );
    LISTOPTION( "--plotxparameter I",
        "Parameter number to plot on X axis." );
    LISTOPTION( "--plotyparameter J",
        "Parameter number to plot on Y axis." );

    LISTOPTIONGROUP( "Gate tree edits:" );
    LISTOPTION( "--setgatetreedescription TEXT",
        "Set gate tree description." );
    LISTOPTION( "--setgatetreename TEXT",
        "Set gate tree name." );
    LISTOPTION( "--setgatetreenotes TEXT",
        "Set gate tree notes." );

    LISTOPTIONGROUP( "Gate edits:" );
    LISTOPTION( "--deletegate I",
        "Delete gate." );
    LISTOPTION( "--setgatedescription I TEXT",
        "Set gate description." );
    LISTOPTION( "--setgatemethod I TEXT",
        "Set gate gating method." );
    LISTOPTION( "--setgatename I TEXT",
        "Set gate name." );
    LISTOPTION( "--setgatenotes I TEXT",
        "Set gate notes." );
    LISTOPTION( "--setgatereportpriority I P",
        "Set gate report priority." );
    LISTOPTION( "--setgateparametervertices I VERTICES",
        "Set polygon gate vertices (comma-separated list)." );

    LISTOPTIONGROUP( "Gate dimension edits:" );
    LISTOPTION( "--setgateparametername I DIM TEXT",
        "Set gate dimension parameter name." );
    LISTOPTION( "--setgateparametertransform I DIM JSON",
        "Set gate dimension parameter transform." );
    LISTOPTION( "--setgateparameterminimum I DIM MIN",
        "Set rectangle gate dimension minimum." );
    LISTOPTION( "--setgateparametermaximum I DIM MAX",
        "Set rectangle gate dimension maximum." );


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
        "  Event formats: " <<
        getEventFileFormatExtensionList( ) << std::endl <<
        "  Gate formats: " <<
        getGateFileFormatExtensionList( ) << std::endl <<
        "  Parameter map formats: " <<
        getParameterMapFileFormatExtensionList( ) << std::endl <<
        "  Plot formats: " <<
        getImageFileFormatExtensionList( ) << std::endl << std::endl;


    std::cerr <<
        "Examples:" <<
        std::endl;
    std::cerr <<
        "  Gate and plot all gates from a cache file:" <<
        std::endl <<
        "    " << application.applicationName << " cache.cache" <<
        std::endl << std::endl;

    std::cerr <<
        "  Gate and plot all gates from a cache file, saving back to the cache file:" <<
        std::endl <<
        "    " << application.applicationName << " cache.cache cache.cache" <<
        std::endl << std::endl;

    std::cerr <<
        "  Gate and plot all gates from event and gate tree files:" <<
        std::endl <<
        "    " << application.applicationName << " events.fcs gates.xml" <<
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


    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Tools:" << std::endl;

    // Gating.
    std::cerr << packageIndent << Gating::NAME << " " <<
        Gating::VERSION <<
        " (built " << Gating::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent << "by " <<
            Gating::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            Gating::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            Gating::LICENSE << std::endl;
    }

    // ScatterPlot.
    std::cerr << packageIndent << ScatterPlot::NAME << " " <<
        ScatterPlot::VERSION <<
        " (built " << ScatterPlot::BUILD_DATE << ")" <<
        std::endl;
    if ( longForm == true )
    {
        std::cerr << packageDetailIndent <<
            ScatterPlot::CREDIT << std::endl;
        std::cerr << packageDetailIndent <<
            ScatterPlot::COPYRIGHT << std::endl;
        std::cerr << packageDetailIndent <<
            ScatterPlot::LICENSE << std::endl;
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
    std::cerr << packageGroupIndent << "Gated event files:" << std::endl;

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

    std::exit( 1 );
}

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

    application.eventPath.clear( );


    // Parse options and file names
    // ----------------------------
    std::string pendingFormat = "";
    std::vector<std::string> paths;
    std::vector<std::string> formats;
    std::string pendingParameterMapFormat = "";

    while ( argc != 0 )
    {
        if ( argv[0][0] == '-' )
        {
            // Skip past one or two leading '-'s.
            std::string option = (argv[0][1] == '-') ?
                std::string( &argv[0][2] ) :
                std::string( &argv[0][1] );

            // Convert to lower case.
            std::transform(
                option.begin( ),
                option.end( ),
                option.begin( ),
                ::tolower );


            //
            // Help & verbosity.
            //
            if ( option.compare( "help" ) == 0 )
            {
                printUsageAndExit( "" );
            }
            if ( option.compare( "version" ) == 0 )
            {
                printVersionAndExit( false );
            }
            if ( option.compare( "versionlong" ) == 0 )
            {
                printVersionAndExit( true );
            }
            if ( option.compare( "verbose" ) == 0 )
            {
                application.verbose = true;
                --argc;
                ++argv;
                continue;
            }
            if ( option.compare( "showwarnings" ) == 0 )
            {
                application.showWarnings = true;
                --argc;
                ++argv;
                continue;
            }


            //
            // Input.
            //
            if ( option.compare( "maxevents" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing maximum events choice after " ) +
                        argv[0] + ".\n" );

                application.maxEvents = std::stol(argv[1]);
                if ( application.maxEvents < (-1) )
                    printErrorAndExit(
                        "Maximum events choice must -1, 0, or a positive integer.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }


            //
            // Output.
            //
            if ( option.compare( "imagebase" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing image file basename after " ) +
                        argv[0] + ".\n" );

                application.imagePath = argv[1];
                application.imageFormat = UNKNOWN_IMAGE_FORMAT;

                // Use the pending format choice, if any.
                if ( pendingFormat.empty( ) == false )
                {
                    application.imageFormat = findImageFileFormat( pendingFormat );
                    if ( application.imageFormat == UNKNOWN_IMAGE_FORMAT )
                        printErrorAndExit(
                            std::string( "Unknown file format name: " ) +
                            pendingFormat + "\n" );

                    pendingFormat.clear( );
                }

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "bgwrite" ) == 0 )
            {
                application.backgroundCacheWrite = true;
                --argc;
                ++argv;
                continue;
            }


            //
            // Generic.
            //
            if ( option.compare( "benchmark" ) == 0 )
            {
                application.benchmark = true;
                --argc;
                ++argv;
                continue;
            }
            if ( option.compare( "threads" ) == 0 )
            {
                // Unsupported.
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing number of threads choice after " ) +
                        argv[0] + ".\n" );

                application.numberOfThreads = std::stol(argv[1]);
                if ( application.numberOfThreads <= 0 )
                    printErrorAndExit(
                        "Number of threads must non-negative.\n" );
                argc -= 2;
                argv += 2;
                continue;
            }

            //
            // Format.
            //
            if ( option.compare( "format" ) == 0 )
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
            // Parameter map.
            //
            if ( option.compare( "parametermap" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing parameter map file after " ) +
                        argv[0] + ".\n" );

                application.parameterMapPath = argv[1];
                pendingParameterMapFormat = pendingFormat;
                argc -= 2;
                argv += 2;
                continue;
            }

            //
            // Gates and clusters.
            // /
            if ( option.compare( "gates" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing gate list choice after " ) +
                        argv[0] + ".\n" );

                std::string list = argv[1];
                size_t numberPosition = 0;
                size_t commaPosition;
                while ( (commaPosition = list.find( ',', numberPosition )) != list.npos )
                {
                    application.gatesToPlot.push_back(
                        std::stol( list.substr( numberPosition, (commaPosition - numberPosition) ) ) );
                        numberPosition = commaPosition + 1;
                }

                application.gatesToPlot.push_back(
                    std::stol( list.substr( numberPosition ) ) );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "clusters" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing clusters choice after " ) +
                        argv[0] + ".\n" );

                application.numberOfClusters = std::stol(argv[1]);
                if ( application.numberOfClusters < 0 )
                    printErrorAndExit(
                        "Cluster number must be non-negative.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "clusteriterations" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing cluster iterations choice after " ) +
                        argv[0] + ".\n" );

                application.clusterIterations = std::stol(argv[1]);
                if ( application.clusterIterations < 0 )
                    printErrorAndExit(
                        "Cluster iterations number must be non-negative.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "randomseed" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing random number seed after " ) +
                        argv[0] + ".\n" );

                application.randomNumberSeed = std::stol(argv[1]);
                if ( application.randomNumberSeed < 0 )
                    printErrorAndExit(
                        "Random number seeds must be non-negative.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }

            //
            // Gate tree edits.
            //
            if ( option.compare( "setgatetreedescription" ) == 0 ||
                option.compare( "setgatetreenotes" ) == 0 ||
                option.compare( "setgatetreename" ) == 0 )
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

            //
            // Gate edits.
            //
            if ( option.compare( "deletegate" ) == 0 )
            {
                // --KEY GATEINDEX
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing gate index after " ) +
                        argv[0] + ".\n" );

                // To enable multiple --KEY edits, but for different
                // gate indexes, the key for the edit map entry must be made
                // unique by including the gate index.
                const auto key = option + argv[1];

                application.edits[key] = std::make_tuple(
                    argv[1],
                    "",
                    "" );
                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "setgatedescription" ) == 0 ||
                option.compare( "setgatemethod" ) == 0 ||
                option.compare( "setgatenotes" ) == 0 ||
                option.compare( "setgatename" ) == 0 ||
                option.compare( "setgatereportpriority" ) == 0 ||
                option.compare( "enablegateadditionalparameter" ) == 0 ||
                option.compare( "disablegateadditionalparameter" ) == 0 ||
                option.compare( "setgateparametervertices" ) == 0 )
            {
                // --KEY GATEINDEX VALUE
                if ( argc < 3 )
                    printErrorAndExit(
                        std::string( "Missing gate index and value after " ) +
                        argv[0] + ".\n" );

                // To enable multiple --KEY edits, but for different
                // gate indexes, the key for the edit map entry must be made
                // unique by including the gate index.
                const auto key = option + argv[1];

                application.edits[key] = std::make_tuple(
                    argv[1],
                    "",
                    argv[2] );
                argc -= 3;
                argv += 3;
                continue;
            }
            if ( option.compare( "setgateadditionalparametertransform" ) == 0 )
            {
                // --KEY GATEINDEX PARAMETERNAME PARAMETERTRANSFORM
                if ( argc < 4 )
                    printErrorAndExit(
                        std::string( "Missing gate index, dimension index, and value after " ) +
                        argv[0] + ".\n" );

                // To enable multiple --KEY edits, but for different gate
                // indexes, the key for the edit map entry must be made
                // unique by including the gate index.
                const auto key = option + argv[1];

                application.edits[key] = std::make_tuple(
                    argv[1],
                    argv[2],
                    argv[3] );
                argc -= 4;
                argv += 4;
                continue;
            }

            if ( option.compare( "setgateparametername" ) == 0 ||
                option.compare( "setgateparametertransform" ) == 0 ||
                option.compare( "setgateparameterminimum" ) == 0 ||
                option.compare( "setgateparametermaximum" ) == 0 )
            {
                // --KEY GATEINDEX DIMENSIONINDEX VALUE
                if ( argc < 4 )
                    printErrorAndExit(
                        std::string( "Missing gate index, dimension index, and value after " ) +
                        argv[0] + ".\n" );

                // To enable multiple --KEY edits, but for
                // different gate and dimension indexes, the key for the edit
                // map entry must be made unique by including the gate index
                // and dimension index.
                const auto key = option + argv[1] + argv[2];

                application.edits[key] = std::make_tuple(
                    argv[1],
                    argv[2],
                    argv[3] );
                argc -= 4;
                argv += 4;
                continue;
            }

            //
            // Plot options.
            //
            if ( option.compare( "plottitle" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing plot title after " ) +
                        argv[0] + ".\n" );

                visual.title = argv[1];

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotsize" ) == 0 )
            {
                if ( argc < 3 )
                    printErrorAndExit(
                        std::string( "Missing image size after " ) +
                        argv[0] + ".\n" );

                visual.imageWidth  = std::stoi( argv[1] );
                visual.imageHeight = std::stoi( argv[2] );

                if ( visual.imageWidth <= 0 || visual.imageHeight <= 0 )
                    printErrorAndExit(
                        "Image size must be positive.\n" );

                argc -= 3;
                argv += 3;
                continue;
            }
            if ( option.compare( "plotmargins" ) == 0 )
            {
                if ( argc < 5 )
                    printErrorAndExit(
                        std::string( "Missing margin sizes after " ) +
                        argv[0] + ".\n" );

                visual.imageMarginLeftPercent   = std::stof( argv[1] );
                visual.imageMarginRightPercent  = std::stof( argv[2] );
                visual.imageMarginTopPercent    = std::stof( argv[3] );
                visual.imageMarginBottomPercent = std::stof( argv[4] );

                if ( visual.imageMarginLeftPercent < 0 ||
                     visual.imageMarginRightPercent < 0 ||
                     visual.imageMarginTopPercent < 0 ||
                     visual.imageMarginBottomPercent < 0 )
                    printErrorAndExit(
                        "Margins must be non-negative.\n" );

                argc -= 5;
                argv += 5;
                continue;
            }
            if ( option.compare( "plotdotsize" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing dot size after " ) +
                        argv[0] + ".\n" );

                visual.dotSize = std::stoi( argv[1] );

                if ( visual.dotSize <= 0 || visual.dotSize > 255 )
                    printErrorAndExit(
                        std::string( "Dot size is too large or small: " ) +
                        std::to_string( visual.dotSize ) + ".\n" +
                        "Dot sizes must be >= 1 and <= 255.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotdotshape" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing dot shape name after " ) +
                        argv[0] + ".\n" +
                        "Please use one of 'circle' or 'square'.\n" );

                if ( std::strcmp( argv[1], "circle" ) == 0 )
                    visual.dotShape = DOT_CIRCLE;
                else if ( std::strcmp( argv[1], "square" ) == 0 )
                    visual.dotShape = DOT_SQUARE;
                else
                    printErrorAndExit(
                        std::string( "Unknown dot shape: " ) +
                        argv[0] + ".\n" +
                        "Please use one of 'circle' or 'square'.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotaxistitlesize" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing axis title font size after " ) +
                        argv[0] + ".\n" );

                visual.axisTitleFontSizePercent = std::stof( argv[1] );

                if ( visual.axisTitleFontSizePercent <= 0.0f )
                    printErrorAndExit(
                        "Font size must be positive.\n" );
                if ( visual.axisTitleFontSizePercent >= 1.0f )
                    printErrorAndExit(
                        "Font size must be less than 100% (1.0).\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotaxisnumbersize" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing axis number font size after " ) +
                        argv[0] + ".\n" );

                visual.axisNumberFontSizePercent = std::stof( argv[1] );

                if ( visual.axisNumberFontSizePercent <= 0.0f )
                    printErrorAndExit(
                        "Font size must be positive.\n" );
                if ( visual.axisNumberFontSizePercent >= 1.0f )
                    printErrorAndExit(
                        "Font size must be less than 100% (1.0).\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotaxisnumbermaxdecimal" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing axis number max digits after decimal after " ) +
                        argv[0] + ".\n" );

                visual.axisNumberMaxDigitsAfterDecimal = std::stoi( argv[1] );

                if ( visual.axisNumberMaxDigitsAfterDecimal < 0.0f )
                    printErrorAndExit(
                        "Axis number max decimal digits must be positive.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotaxisxtitlemargin" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing X axis margin after " ) +
                        argv[0] + ".\n" );

                visual.axisXTitleMarginPercent = std::stof( argv[1] );

                if ( visual.axisXTitleMarginPercent < 0.0f )
                    printErrorAndExit(
                        "Title margin must be zero or positive.\n" );
                if ( visual.axisXTitleMarginPercent >= 1.0f )
                    printErrorAndExit(
                        "Title margin must be less than 100% (1.0).\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotaxisytitlemargin" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing Y axis margin after " ) +
                        argv[0] + ".\n" );

                visual.axisYTitleMarginPercent = std::stof( argv[1] );

                if ( visual.axisYTitleMarginPercent < 0.0f )
                    printErrorAndExit(
                        "Title margin must be zero or positive.\n" );
                if ( visual.axisYTitleMarginPercent >= 1.0f )
                    printErrorAndExit(
                        "Title margin must be less than 100% (1.0).\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plottitlesize" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing title font size after " ) +
                        argv[0] + ".\n" );

                visual.titleFontSizePercent = std::stof( argv[1] );

                if ( visual.titleFontSizePercent <= 0.0f )
                    printErrorAndExit(
                        "Font size must be positive.\n" );
                if ( visual.titleFontSizePercent >= 1.0f )
                    printErrorAndExit(
                        "Font size must be less than 100% (1.0).\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotlegendtitlesize" ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing legend title font size after " ) +
                        argv[0] + ".\n" );

                visual.legendFontSizePercent = std::stof( argv[1] );

                if ( visual.legendFontSizePercent <= 0.0f )
                    printErrorAndExit(
                        "Font size must be positive.\n" );
                if ( visual.legendFontSizePercent >= 1.0f )
                    printErrorAndExit(
                        "Font size must be less than 100% (1.0).\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( option.compare( "plotlegend" ) == 0 )
            {
                visual.showLegend = true;
                --argc;
                ++argv;
                continue;
            }
            if ( option.compare( "plotfilledgates" ) == 0 )
            {
                visual.showFilledGates = true;
                --argc;
                ++argv;
                continue;
            }
            if ( option.compare( "plotshadows" ) == 0 )
            {
                visual.showGateShadows = true;
                --argc;
                ++argv;
                continue;
            }
            if ( option.compare( "plotparent" ) == 0 )
            {
                visual.showParentPopulation = true;
                --argc;
                ++argv;
                continue;
            }

            //
            // Unknown.
            //
            printErrorAndExit(
                std::string( "Unknown option: '" ) +
                argv[0] + "'.\n" );
        }

        if ( paths.size( ) < 3 )
        {
            paths.push_back( argv[0] );
            formats.push_back( pendingFormat );
            pendingFormat.clear( );
            --argc;
            ++argv;
            continue;
        }

        // Too many files.
        printErrorAndExit(
            std::string( "Too many files.\n" ) );
    }

    //
    // Interpret paths.
    // ----------------
    // The number of paths determine how they are interpreted:
    //
    // - FILE1.
    //   FILE1 must be a cache file, which is read but not saved.
    //
    // - FILE1 FILE2.
    //   If FILE1 and FILE2 are cache files, read FILE1 and save to FILE2.
    //   If FILE1 and FILE2 are event and gate files, in either order, read
    //   and gate, but do not save.
    //
    // - FILE1 FILE2 FILE3.
    //   FILE1 and FILE2 must be event and gate files, in either order.
    //   FILE3 must be a cache file for saved results.
    //
    // Plot files are named after the cache or event file unless an image
    // basename is given with --imagebase.
    const size_t nPaths = paths.size( );
    for ( size_t i = 0; i < nPaths; ++i )
    {
        if ( formats[i].empty( ) == true )
        {
            const std::string ext = getFileExtension( paths[i] );
            if ( ext.empty( ) == true )
                printErrorAndExit(
                    std::string( "\"" ) + paths[i] +
                    "\" has no file name extension.\n" +
                    "Use --format to specify the file format.\n" );
            formats[i] = ext;
        }
    }

    switch ( paths.size( ) )
    {
        case 0:
            printErrorAndExit(
                std::string( "An event or cache file to plot is required.\n" ) );

        case 1:
            application.eventPath = paths[0];
            application.eventFormat = findEventFileFormat( formats[0] );

            if ( application.eventFormat == UNKNOWN_EVENT_FORMAT )
                printErrorAndExit(
                    std::string( "Unknown file format name: " ) +
                    pendingFormat + "\n" );

            if ( application.eventFormat != FGGATINGCACHE_FORMAT )
                printErrorAndExit(
                    "When only one file path is provided, it must be for a gating cache file.\n" );
            break;

        case 2:
        {
            auto d1 = findEventFileFormat( formats[0] );
            if ( d1 == FGGATINGCACHE_FORMAT )
            {
                // 1st file is a cache file. 2nd file must be a cache file
                // in which to save the gating results.
                auto d2 = findEventFileFormat( formats[1] );
                if ( d2 != FGGATINGCACHE_FORMAT )
                    printErrorAndExit(
                        "When the first file path is for a gating cache file, the second must be too.\n" );

                application.eventPath = paths[0];
                application.eventFormat = d1;
                application.cachePath = paths[1];
            }
            else if ( d1 != UNKNOWN_EVENT_FORMAT )
            {
                // 1st file is an event file. 2nd file must be a gate tree
                // file.
                auto d2 = findGateFileFormat( formats[1] );
                if ( d2 == UNKNOWN_GATE_FORMAT )
                    printErrorAndExit(
                        "When the first file path is for an event file, the second must be a gate file.\n" );

                application.eventPath = paths[0];
                application.eventFormat = d1;
                application.gatePath = paths[1];
                application.gateFormat = d2;
            }
            else
            {
                // 1st file is not an event or cache file. Is it a gate
                // tree file?
                auto d1alt = findGateFileFormat( formats[0] );
                if ( d1alt == UNKNOWN_GATE_FORMAT )
                    printErrorAndExit(
                        std::string( "Unknown file format: " ) +
                        formats[0] + "\n" );

                // 1st file is a gate tree file. 2nd file must be an
                // event file.
                auto d2 = findEventFileFormat( formats[1] );
                if ( d2 == UNKNOWN_EVENT_FORMAT ||
                     d2 == FGGATINGCACHE_FORMAT )
                    printErrorAndExit(
                        "When the first file path is for a gate file, the second must be an event file.\n" );

                application.eventPath = paths[1];
                application.eventFormat = d2;
                application.gatePath = paths[0];
                application.gateFormat = d1alt;
            }
            break;
        }

        case 3:
        {
            auto d3 = findEventFileFormat( formats[2] );
            if ( d3 != FGGATINGCACHE_FORMAT )
                printErrorAndExit(
                    "When three file paths are provided, the last one must be for a gating cache file.\n" );

            application.cachePath = paths[2];

            auto d1 = findEventFileFormat( formats[0] );
            if ( d1 == FGGATINGCACHE_FORMAT )
                printErrorAndExit(
                    "When three file paths are provided, the first and second must be event and gate files.\n" );
            else if ( d1 != UNKNOWN_EVENT_FORMAT )
            {
                // 1st file is an event file. 2nd file msut be a gate file.
                auto d2 = findGateFileFormat( formats[1] );
                if ( d2 == UNKNOWN_GATE_FORMAT )
                    printErrorAndExit(
                        "When the first file path is for an event file, the second must be a gate file.\n" );

                application.eventPath = paths[0];
                application.eventFormat = d1;
                application.gatePath = paths[1];
                application.gateFormat = d2;
            }
            else
            {
                // 1st file is not an event file. Is it a gate
                // tree file?
                auto d1alt = findGateFileFormat( formats[0] );
                if ( d1alt == UNKNOWN_GATE_FORMAT )
                    printErrorAndExit(
                        std::string( "Unknown file format: " ) +
                        formats[0] + "\n" );

                // 1st file is a gate tree file. 2nd file must be an
                // event file.
                auto d2 = findEventFileFormat( formats[1] );
                if ( d2 == UNKNOWN_EVENT_FORMAT ||
                     d2 == FGGATINGCACHE_FORMAT )
                    printErrorAndExit(
                        "When the first file path is for a gate file, the second must be an event file.\n" );

                application.eventPath = paths[1];
                application.eventFormat = d2;
                application.gatePath = paths[0];
                application.gateFormat = d1alt;
            }
            break;
        }
    }

    if ( application.parameterMapPath.empty( ) == false )
    {
        if ( pendingParameterMapFormat.empty( ) == true )
        {
            const std::string ext = getFileExtension( application.parameterMapPath );
            if ( ext.empty( ) == true )
                printErrorAndExit(
                    std::string( "\"" ) + application.parameterMapPath +
                    "\" has no file name extension.\n" +
                    "Use --format to specify the file format.\n" );
            pendingParameterMapFormat = ext;
        }

        application.parameterMapFormat =
            findParameterMapFileFormat( pendingParameterMapFormat );

        if ( application.parameterMapFormat == UNKNOWN_PARAMETERMAP_FORMAT )
            printErrorAndExit(
                std::string( "Unknown file format name: " ) +
                pendingParameterMapFormat + "\n" );
    }

    if ( application.imagePath.empty( ) == true )
    {
        // Create an image file name based on the event file name.
        const size_t dotPosition = application.eventPath.rfind( '.' );
        if ( dotPosition == std::string::npos )
            application.imagePath = application.eventPath + ".png";
        else
            application.imagePath =
                application.eventPath.substr( 0, dotPosition ) + ".png";
        application.imageFormat = PNG_TRUE_COLOR;
    }

    application.displayFormat = TEXT_FORMAT;
    if ( pendingFormat.empty( ) == false )
    {
        application.displayFormat = findTextFileFormat( pendingFormat );
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
 * Uses command-line values to load a file and return a gating cache.
 *
 * This function supports loading data from a variety of file formats.
 * @li Event files.
 * @li Gate tree files.
 * @li Gating cache files.
 *
 * When loading an event file, a companion gate tree file must be loaded
 * as well. The events and gate trees are then combined into a new gating
 * cache, which is returned.
 *
 * When loading a gating cache file, there cannot be a companion gate tree
 * file. The gating cache is returned.
 *
 * When loading an FCS file, the file's events are automatically compensated
 * before further use.
 *
 * On any error, messages are output and the application exits.
 *
 * @returns
 *   The new gating cache is returned.
 */
std::shared_ptr<GatingCache> loadOrExit( )
{
    const double timeAtStartOfLoad = getRealTime( );

    const bool showLog = application.verbose || application.showWarnings;

    // If the event input file is a gating cache, load it.
    if ( application.eventFormat == FGGATINGCACHE_FORMAT )
    {
        auto file = new FileFGGatingCache( );
        file->setVerbose( application.verbose );
        file->setVerbosePrefix( application.applicationName );

        try
        {
            file->load( application.eventPath, true );
        }
        catch ( const std::exception& e )
        {
            if ( showLog == true )
                printFileLog( file->getFileLog( ) );
            std::cerr << e.what( ) << std::endl;
            std::exit( 1 );
        }

        if ( showLog == true )
            printFileLog( file->getFileLog( ) );

        auto gatingCache = file->getGatingCache( );

        gatingCache->setVerbose( application.verbose );
        gatingCache->setVerbosePrefix( application.applicationName );

        delete file;

        const double timeAtEndOfLoad = getRealTime( );
        if ( application.benchmark == true )
            addBenchmark(
                "Load gating cache file",
                (timeAtEndOfLoad - timeAtStartOfLoad) );

        return gatingCache;
    }

    // Load the event file.
    std::shared_ptr<EventTableInterface> eventTable;

    switch ( application.eventFormat )
    {
        case FCS_FORMAT:
        {
            auto file = new FileFCS( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );
            file->setAutoScaling( true );

            try
            {
                file->load( application.eventPath, application.maxEvents );
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );

                // Compensate.
                std::vector<std::string> matrixParameterNames;
                std::vector<double> matrix;
                file->getSpilloverMatrix( matrixParameterNames, matrix );

                if ( matrixParameterNames.empty( ) == false )
                    eventTable->compensate( matrixParameterNames, matrix );
            }
            catch (const std::exception& e)
            {
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        case FGBINARYEVENTS_FORMAT:
        {
            auto file = new FileFGBinaryEvents( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.eventPath, application.maxEvents );
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );
            }
            catch (const std::exception& e)
            {
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }
        case FGTEXTEVENTS_FORMAT:
        {
            auto file = new FileFGTextEvents( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.eventPath, application.maxEvents );
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                eventTable = file->getEventTable( );
            }
            catch (const std::exception& e)
            {
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            delete file;
            break;
        }

        default:
            std::exit( 1 );
    }


    // Load the gate trees file.
    std::shared_ptr<GateTrees> gateTrees;

    switch ( application.gateFormat )
    {
        case GATINGML_FORMAT:
        {
            auto file = new FileGatingML( );
            try
            {
                file->setVerbose( application.verbose );
                file->setVerbosePrefix( application.applicationName );
                file->load( application.gatePath );
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( showLog == true )
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
                file->load( application.gatePath );
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( showLog == true )
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
                file->load( application.gatePath );
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                gateTrees = file->getGateTrees( );
                delete file;
            }
            catch ( const std::exception& e )
            {
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }
            break;
        }

        default:
            return nullptr;
    }

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark(
            "Load event and gate tree files",
            (timeAtEndOfLoad - timeAtStartOfLoad) );


    // Create the gating cache.
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

    return cache;
}





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
    const double timeAtStartOfMain = getRealTime( );

    //
    // Parse arguments.
    // ----------------
    // Parse command-line arguments to select an input file and visual
    // attributes.
    parseArguments( argc, argv );

    // If the command line did not specify the number of threads to use,
    // default to the maximum possible on the current host. This is typically
    // equal to the number of cores or some multiple if hyperthreading or
    // multithreading are supported and enabled on the processor.
#if defined(_OPENMP)
    if ( application.numberOfThreads == 0 )
        application.numberOfThreads = omp_get_max_threads( );
    omp_set_num_threads( application.numberOfThreads );
#else
    application.numberOfThreads = 1;
#endif

    const double timeAtEndOfMainStart = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark(
            "Main startup",
            (timeAtEndOfMainStart - timeAtStartOfMain) );


    //
    // Load data.
    // ----------
    // Load input files and return a gating cache. The cache may have been
    // loaded from an input file, or created using event and gate trees
    // loaded from input files.
    auto gatingCache = loadOrExit( );

    auto sourceEventTable = gatingCache->getSourceEventTable( );
    const auto numberOfEvents = sourceEventTable->getNumberOfEvents( );

    std::shared_ptr<ParameterMap> parameterMap;
    if ( application.parameterMapPath.empty( ) == false )
    {
        const double timeAtStartOfLoad = getRealTime( );

        const bool showLog = application.verbose || application.showWarnings;

        // Load the parameter map.
        if ( application.parameterMapFormat == FGJSONPARAMETERMAP_FORMAT )
        {
            auto file = new FileFGJsonParameterMap( );
            file->setVerbose( application.verbose );
            file->setVerbosePrefix( application.applicationName );

            try
            {
                file->load( application.parameterMapPath );
            }
            catch ( const std::exception& e )
            {
                if ( showLog == true )
                    printFileLog( file->getFileLog( ) );
                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }

            if ( showLog == true )
                printFileLog( file->getFileLog( ) );

            parameterMap = file->getParameterMap( );

            gatingCache->setVerbose( application.verbose );
            gatingCache->setVerbosePrefix( application.applicationName );

            delete file;

            const double timeAtEndOfLoad = getRealTime( );
            if ( application.benchmark == true )
                addBenchmark(
                    "Load parameter map file",
                    (timeAtEndOfLoad - timeAtStartOfLoad) );
        }
    }


    //
    // Apply edits.
    // ------------
    // If the command-line provided any edits, apply them first.
    //
    // If the command-line does not include a list of gates to process,
    // process them all.
    //
    // If specific gates are requested, process those and their ancestors.
    // Maintain two lists:
    // - application.gatesToPlot is a list of gates to plot.
    // - gatesNeeded here is a temporary list with one bool per gate to
    //   indicate if the corresponding gate needs to be processed.
    const double timeAtStartOfGateEditing = getRealTime( );

    auto gateTrees = gatingCache->getGateTrees( );
    auto gates = gateTrees->findDescendentGatesWithParentIndexes( );

    // Edit. Some operations (e.g. delete gate) can change the gate list
    // and gate trees.
    applyEdits( gatingCache, gates );

    // Get the number of gates left after editing.
    const GateIndex numberOfGates = gates.size( );

    // Build a list of gates to plot based upon what was requested on
    // the command line, if anything.
    std::vector<bool> gatesNeeded;
    GateIndex nGatesNeeded = 0;
    bool gateAll = false;
    GateIndex nGatesToPlot = application.gatesToPlot.size( );

    if ( nGatesToPlot == 0 )
    {
        // No gates were specified on the command line.
        //
        // Add all gates to the gates-to-plot list.
        for ( uint32_t i = 0; i < numberOfGates; ++i )
            application.gatesToPlot.push_back( i );

        // And mark all gates as needed.
        gatesNeeded.resize( numberOfGates, true );
        nGatesNeeded = numberOfGates;
        nGatesToPlot = numberOfGates;
        gateAll = true;
    }
    else
    {
        // A list of one or more gates were specified on the command line.
        //
        // Loop through the gate indexes. Validate each one, then add it
        // to the list. Note that the user may have entered the same gate
        // index more than once.
        const uint32_t n = application.gatesToPlot.size( );
        gatesNeeded.resize( numberOfGates, false );

        for ( uint32_t i = 0; i < n; ++i )
        {
            const GateIndex gateIndex = application.gatesToPlot[i];

            // Validate the index.
            if ( gateIndex < 0 )
                printErrorAndExit(
                    std::string( "The requested gate number " ) +
                    std::to_string( gateIndex ) +
                    std::string( " is negative and invalid.\n" ) );

            if ( gateIndex >= numberOfGates )
                printErrorAndExit(
                    std::string( "The requested gate number " ) +
                    std::to_string( gateIndex ) +
                    std::string( " is higher than the number of gates available.\n" ) );

            // Mark the gate as needed.
            if ( gatesNeeded[gateIndex] == false )
            {
                gatesNeeded[gateIndex] = true;
                ++nGatesNeeded;
            }

            // All of its ancestors also need to be processed. Find them.
            uint32_t index = gateIndex;
            uint32_t parentGateIndex = gates[index].first;
            while (parentGateIndex != index &&
                   gatesNeeded[parentGateIndex] == false )
            {
                gatesNeeded[parentGateIndex] = true;
                ++nGatesNeeded;

                index = parentGateIndex;
                parentGateIndex = gates[index].first;
            }
        }

        // If the entire gate list was marked as in need of processing,
        // then simplify.
        gateAll = (nGatesNeeded == numberOfGates);
    }

    const double timeAtEndOfGateEditing = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark(
            "Edit gates",
            (timeAtEndOfGateEditing - timeAtStartOfGateEditing) );


    //
    // Apply gating.
    // -------------
    // Create a gater and apply it to all needed gates.
    const double timeAtStartOfGating = getRealTime( );

    try
    {
        auto gating = new Gating( gatingCache );

        gating->setVerbose( application.verbose );
        gating->setVerbosePrefix( application.applicationName );

        if ( application.numberOfClusters > 0 )
            gating->setNumberOfSeedClusters( application.numberOfClusters );

        if ( application.clusterIterations > 0 )
            gating->setNumberOfIterations( application.clusterIterations );

        gating->setRandomNumberSeed( application.randomNumberSeed );

        if ( gateAll == true )
        {
            // All gates need to be processed.
            if ( application.verbose == true )
            {
                std::cerr << application.applicationName << ":\n";
                std::cerr << application.applicationName <<
                    ": Gating events for all " << numberOfGates << " gates:\n";
            }

            gating->gateAll( );
        }
        else
        {
            // Only some gates are needed.
            if ( application.verbose == true )
            {
                std::cerr << application.applicationName << ":\n";
                std::cerr << application.applicationName <<
                    ": Gating events for " <<
                    nGatesNeeded << " needed gates:\n";
            }

            for ( uint32_t i = 0; i < numberOfGates; ++i )
            {
                if ( gatesNeeded[i] == true )
                    gating->gate( gates[i].second );
            }
        }

        delete gating;
    }
    catch ( const std::exception& e )
    {
        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }

    const double timeAtEndOfGating = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark( "Cluster and gating",
            (timeAtEndOfGating - timeAtStartOfGating) );


    //
    // Plot.
    // -----
    // Plot each of the requested gates in parallel using multiple threads.
    // Each thread needs its own plot object.
    const double timeAtStartOfPlotSetup = getRealTime( );

    const auto numberOfEventsToPlot =
        (application.maxEvents < 0 || (size_t) application.maxEvents > numberOfEvents) ?
        numberOfEvents : application.maxEvents;

    std::vector<ScatterPlot*> scatterPlots = createPlotObjects( );

    const double timeAtEndOfPlotSetup = getRealTime( );
    if ( application.benchmark == true )
        addBenchmark( "Set up plot",
            (timeAtEndOfPlotSetup - timeAtStartOfPlotSetup) );

    if ( application.verbose == true )
    {
        std::cerr << application.applicationName << ":\n";
        std::cerr << application.applicationName << ": Plotting:\n";
    }

    try
    {
#pragma omp parallel for schedule(static) num_threads(application.numberOfThreads)
        for ( uint32_t i = 0; i < nGatesToPlot; ++i )
        {
            // Based on the thread number, get the thread's plot object.
#if defined(_OPENMP)
            const int threadNumber = omp_get_thread_num( );
#else
            const int threadNumber = 0;
#endif
            ScatterPlot*const scatterPlot = scatterPlots[threadNumber];

            const auto gateIndex       = application.gatesToPlot[i];
            const auto parentGateIndex = gates[gateIndex].first;

            std::shared_ptr<const Gate> gate = gates[gateIndex].second;
            std::shared_ptr<const Gate> parent  =
                (gateIndex == parentGateIndex) ?
                nullptr : gates[parentGateIndex].second;

            plot(
                scatterPlot,
                sourceEventTable,
                parameterMap,
                parent,
                gate,
                gateIndex,
                numberOfEventsToPlot );
        }
    }
    catch ( const std::exception& e )
    {
        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }


    //
    // Output statistics.
    // ------------------
    // Statistics are based upon the total number of events in the source
    // event table and gated above. This can be less than the number of
    // events plotted.
    if ( application.displayFormat == JSON_FORMAT )
    {
        std::cout << "{" << std::endl;
        printBenchmarks( );
        std::cout << "  \"numberOfEvents\": " <<
            numberOfEvents << "," << std::endl;
        std::cout << "  \"gateStatistics\": {" << std::endl;

        for ( uint32_t i = 0; i < nGatesToPlot; ++i )
        {
            const auto gateIndex       = application.gatesToPlot[i];
            const auto parentGateIndex = gates[gateIndex].first;

            std::shared_ptr<const Gate> gate = gates[gateIndex].second;
            std::shared_ptr<const Gate> parent  =
                (gateIndex == parentGateIndex) ?
                nullptr : gates[parentGateIndex].second;

            const auto state =
                std::dynamic_pointer_cast<const GateState>( gate->getState( ) );
            const auto areFloats = state->areValuesFloats( );
            int maxDigits = 0;
            if ( areFloats == true )
                maxDigits = std::numeric_limits<float>::max_digits10;
            else
                maxDigits = std::numeric_limits<double>::max_digits10;

            // Output information about the gate:
            // - The number of events in and out.
            // - The min/max for transformed values for the gate's dimensions.
            //
            // For min/max, watch for special values +/- infinity, which
            // can occur for log transforms. Such values cannot be represented
            // in JSON. Replace them with highest and lowest possible values
            // for a double.
            std::cout << "    \"" << gateIndex << "\": {" << std::endl;

            if ( parent != nullptr )
            {
                const auto n = parent->getState( )->findNumberOfIncludedEvents( );
                std::cout << "      \"numberOfEventsIn\": " <<
                    n << "," << std::endl;
            }
            else
            {
                std::cout << "      \"numberOfEventsIn\": " <<
                    numberOfEvents << "," << std::endl;
            }

            const auto n = state->findNumberOfIncludedEvents( );
            std::cout << "      \"numberOfEventsOut\": " <<
                n << "," << std::endl;

            const auto nDimensions = gate->getNumberOfDimensions( );
            std::cout << "      \"transformedParameterMinimums\": [ ";
            for ( uint32_t j = 0; j < nDimensions; ++j )
            {
                // Watch for +/- infinity.
                double min = state->getParameterBestMinimum( j );
                if ( areFloats == true )
                {
                    // Test against <double>, but use min/max <float>.
                    if ( min == std::numeric_limits<double>::infinity( ) )
                        min = std::numeric_limits<float>::max( );
                    else if ( min == -std::numeric_limits<double>::infinity( ) )
                        min = std::numeric_limits<float>::lowest( );
                }
                else
                {
                    if ( min == std::numeric_limits<double>::infinity( ) )
                        min = std::numeric_limits<double>::max( );
                    else if ( min == -std::numeric_limits<double>::infinity( ) )
                        min = std::numeric_limits<double>::lowest( );
                }

                std::cout << std::setprecision( maxDigits );
                if ( j == (nDimensions-1) )
                    std::cout << min << " ]," << std::endl;
                else
                    std::cout << min << ", ";
            }

            std::cout << "      \"transformedParameterMaximums\": [ ";
            for ( uint32_t j = 0; j < nDimensions; ++j )
            {
                // Watch for +/- infinity.
                double max = state->getParameterBestMaximum( j );
                if ( areFloats == true )
                {
                    // Test against <double>, but use min/max <float>.
                    if ( max == std::numeric_limits<double>::infinity( ) )
                        max = std::numeric_limits<float>::max( );
                    else if ( max == -std::numeric_limits<double>::infinity( ) )
                        max = std::numeric_limits<float>::lowest( );
                }
                else
                {
                    if ( max == std::numeric_limits<double>::infinity( ) )
                        max = std::numeric_limits<double>::max( );
                    else if ( max == -std::numeric_limits<double>::infinity( ) )
                        max = std::numeric_limits<double>::lowest( );
                }

                std::cout << std::setprecision( maxDigits );
                if ( j == (nDimensions-1) )
                    std::cout << max << " ]" << std::endl;
                else
                    std::cout << max << ", ";
            }

            if ( i == (nGatesToPlot-1) )
                std::cout << "    }" << std::endl;
            else
                std::cout << "    }," << std::endl;
        }

        std::cout << "  }" << std::endl;
        std::cout << "}" << std::endl;
    }
    else
    {
        std::cout << "Total events:\t" << numberOfEvents << std::endl;
        printBenchmarks( );

        for ( uint32_t i = 0; i < nGatesToPlot; ++i )
        {
            const auto gateIndex       = application.gatesToPlot[i];
            const auto parentGateIndex = gates[gateIndex].first;

            std::shared_ptr<const Gate> gate = gates[gateIndex].second;
            std::shared_ptr<const Gate> parent  =
                (gateIndex == parentGateIndex) ?
                nullptr : gates[parentGateIndex].second;

            const auto state =
                std::dynamic_pointer_cast<const GateState>( gate->getState( ) );

            std::cout << "  Gate " << gateIndex << ":" << std::endl;

            if ( parent != nullptr )
            {
                // The gate has a parent. Report the number of events within
                // that gate as the input to this gate.
                const auto n = parent->getState( )->findNumberOfIncludedEvents( );
                std::cout << "    Events in to gate:\t" <<
                    n << std::endl;
            }
            else
            {
                // The number of events input to this gate is all of them.
                std::cout << "    Events in to gate:\t" <<
                    numberOfEvents << std::endl;
            }

            const auto n = state->findNumberOfIncludedEvents( );
            std::cout << "    Events out of gate:\t" <<
                n << std::endl;

            const auto nDimensions = gate->getNumberOfDimensions( );
            for ( uint32_t j = 0; j < nDimensions; ++j )
            {
                const auto min = state->getParameterBestMinimum( j );
                const auto max = state->getParameterBestMaximum( j );
                std::cout << "    Transformed minimum, dimension " <<
                    j << ":\t" << min << std::endl;
                std::cout << "    Transformed maximum, dimension " <<
                    j << ":\t" << max << std::endl;
            }
        }
    }


    //
    // Save results.
    // -------------
    // Save the gating cache file.
    if ( application.cachePath.empty( ) == false )
    {
        if ( application.verbose == true )
        {
            std::cerr << application.applicationName << ":\n";
            if ( application.backgroundCacheWrite == true )
                std::cerr << application.applicationName <<
                    ": Save gating cache by background process\n";
            else
                std::cerr << application.applicationName <<
                    ": Save gating cache\n";
        }

#if defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
        pid_t pid = -1;
        if ( application.backgroundCacheWrite == true )
        {
            // Fork a new child process and let the child save the file.
            pid = fork( );
        }

        if ( pid <= 0 )
#endif
        {
            // Child process OR
            // Parent process where fork() was not done or failed.
            //
            // If a child, immediately move the process into its own
            // session so that it will not exist when the parent process
            // finishes. And then continue and write the file.
            //
            // In any case, continue on and write the cache file.
            if ( pid == 0 )
            {
                // Child process.
                setsid( );
            }

            auto file = new FileFGGatingCache( gatingCache );

            try
            {
                file->setVerbose( application.verbose );
                file->setVerbosePrefix( application.applicationName );

                file->save( application.cachePath );
            }
            catch ( const std::exception& e )
            {
                if ( application.verbose == true ||
                     application.showWarnings == true )
                    printFileLog( file->getFileLog( ) );

                std::cerr << e.what( ) << std::endl;
                std::exit( 1 );
            }

            if ( application.verbose == true ||
                 application.showWarnings == true )
                printFileLog( file->getFileLog( ) );

            delete file;
        }
    }
}
