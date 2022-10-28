/**
 * @file
 * Creates a scatter plot of selected axes from a FlowGate event file.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "EventTable.h"
 * @li FlowGate "FileFCS.h"
 * @li FlowGate "FileFGBinaryEvents.h"
 * @li FlowGate "FileFGTextEvents.h"
 * @li FlowGate "ScatterPlot.h"
 * @li libgd (via "ScatterPlot.h")
 */
#include <algorithm>    // std::transform
#include <iostream>     // std::cerr, ...
#include <iomanip>      // I/O formatting

#include "EventTable.h"         // FlowGate event table

#include "FileFCS.h"            // FCS file format
#include "FileFGBinaryEvents.h" // FlowGate Event Binary file format
#include "FileFGTextEvents.h"   // FlowGate Event Text file format

#include "ScatterPlot.h"        // Scatter plotter

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





//----------------------------------------------------------------------
//
// Enums.
//
//----------------------------------------------------------------------
/**
 * Indicates a supported input file format.
 */
enum DataFileFormat
{
    UNKNOWN_DATA_FORMAT,

    // Event formats.
    FCS_FORMAT,
    FGBINARYEVENTS_FORMAT,
    FGTEXTEVENTS_FORMAT,
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
 * Indicates a supported image file format.
 */
enum ImageFileFormat
{
    UNKNOWN_IMAGE_FORMAT,
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
#define DEFAULT_AXIS_NUMBER_FONT_NAMES "Palatino"
#define DEFAULT_AXIS_NUMBER_FONT_SIZE_PERCENT 0.020
#define DEFAULT_AXIS_NUMBER_OF_TICS 6
#define DEFAULT_AXIS_TIC_COLOR       0x5E5E5E
#define DEFAULT_AXIS_TIC_LENGTH_PERCENT      0.01
#define DEFAULT_AXIS_TIC_LINE_WIDTH  1
#define DEFAULT_AXIS_TITLE_COLOR     0x000000
//#define DEFAULT_AXIS_TITLE_FONT_NAMES "Times New Roman"
#define DEFAULT_AXIS_TITLE_FONT_NAMES "Palatino"
#define DEFAULT_AXIS_TITLE_FONT_SIZE_PERCENT 0.025
#define DEFAULT_AXIS_X_TITLE_MARGIN_PERCENT 0.0
#define DEFAULT_AXIS_Y_TITLE_MARGIN_PERCENT 0.0
#define DEFAULT_AXIS_NUMBER_MAX_DIGITS_AFTER_DECIMAL 3

#define DEFAULT_BORDER_COLOR         0x5E5E5E
#define DEFAULT_BORDER_LINE_WIDTH    1

#define DEFAULT_DOT_SHAPE            DOT_SQUARE
#define DEFAULT_DOT_SIZE             1

#define DEFAULT_PLOT_BACKGROUND_COLOR 0xF0F0F0

#define DEFAULT_TITLE_COLOR          0x000000
//#define DEFAULT_TITLE_FONT_NAMES     "Times New Roman:bold"
#define DEFAULT_TITLE_FONT_NAMES     "Palatino:bold"
#define DEFAULT_TITLE_FONT_SIZE_PERCENT 0.035




/**
 * A set of dot colors chosen to be attractive and maximally different
 * when multiple gates are on the same plot.
 */
std::vector<int> GoodColors(
{
    0x000000,   // Black
    0xBEBEBE,   // Gray
    0x5F9DFF,   // Aqua
    0x70B911,   // Green
    0xE7BE03,   // Tangerine
    0xF2371B,   // Bright red
    0x2b4a80,   // Ocean
    0x669000,   // Fern
    0xAF2F7C,   // Strawberry
    0x801D93,   // Plum
    0x60A79C,   // Teal
    0xFCFE00,   // Lemon
    0x2A12FF,   // Blueberry
} );

#define DEFAULT_DOT_COLOR_INDEX 6





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
     * The file path for the output file.
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
    std::vector<int> dotColors = GoodColors;
};

Visual visual;





//----------------------------------------------------------------------
//
// File name extension utilities.
//
//----------------------------------------------------------------------
/**
 * Returns the format enum for the given file name extension.
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

    return UNKNOWN_DATA_FORMAT;
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

    for ( size_t f = 0; f < 3; ++f )
    {
        switch ( f )
        {
        case 0: list = FileFCS::getFileNameExtensions( ); break;
        case 1: list = FileFGBinaryEvents::getFileNameExtensions( ); break;
        case 2: list = FileFGTextEvents::getFileNameExtensions( ); break;
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
// Plot.
//
//----------------------------------------------------------------------
/**
 * Creates a scatter plot using the given data and current visuals.
 *
 * The current visual parameters, set up via the command-line, are used
 * to plot the given data.
 *
 * @param[in] eventTable
 *   The eventTable to plot.
 *
 * @return
 *   Returns a new scatter plot object containing the plotted data.
 */
ScatterPlot* plot( const std::shared_ptr<EventTableInterface> eventTable )
    noexcept
{
    if ( application.verbose == true )
    {
        std::cerr << application.applicationName <<
            ": Drawing plot:\n";
        std::cerr << application.applicationName <<
            ":   Scanning events for min/max\n";
    }

    //
    // Get numbers.
    // ------------
    // Get the number of events and the names of the parameters to plot.
    // Get the min/max for the X and Y parameters.
    const size_t nEvents = eventTable->getNumberOfEvents( );

    auto parameterNames = eventTable->getParameterNames( );

    const std::string xName( parameterNames[application.xParameterNumber] );
    const std::string yName( parameterNames[application.yParameterNumber] );

    const double xMin =
        eventTable->getParameterBestMinimum(application.xParameterNumber);
    const double xMax =
        eventTable->getParameterBestMaximum(application.xParameterNumber);

    const double yMin =
        eventTable->getParameterBestMinimum(application.yParameterNumber);
    const double yMax =
        eventTable->getParameterBestMaximum(application.yParameterNumber);


    //
    // Create a plot object.
    // ---------------------
    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ":   Initializing plot\n";

    ScatterPlot*const plot = new ScatterPlot(
        visual.imageWidth, visual.imageHeight );


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


    //
    // Set X and Y axis attributes.
    // ----------------------------
    // Set the axis title, number, and tic mark colors, the tic length and
    // thickness, the X and Y axis numeric ranges and the tic mark locations
    // on those axes.
    plot->setAxisTitleColor(  cAxisTitle );
    plot->setAxisNumberColor( cAxisNumber );
    plot->setAxisTicColor(    cAxisTic );

    plot->setAxisXRange( xMin, xMax );
    plot->setAxisYRange( yMin, yMax );
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

    std::vector<float> xTics;
    double value = xMin;
    double delta = (xMax - xMin) / (double) (DEFAULT_AXIS_NUMBER_OF_TICS - 1);
    for ( size_t i = 0; i < DEFAULT_AXIS_NUMBER_OF_TICS; ++i )
    {
        xTics.push_back( (float) value );
        value += delta;
    }
    plot->setAxisXTics( xTics );

    std::vector<float> yTics;
    value = yMin;
    delta = (yMax - yMin) / (double) (DEFAULT_AXIS_NUMBER_OF_TICS - 1);
    for ( size_t i = 0; i < DEFAULT_AXIS_NUMBER_OF_TICS; ++i )
    {
        yTics.push_back( (float) value );
        value += delta;
    }
    plot->setAxisYTics( yTics );


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


    //
    // Set decorations.
    // ---------------
    // Set plot-specific attributes, including the plot title, axis titles, ...
    plot->setTitle( visual.title );
    plot->setAxisXTitle( xName );
    plot->setAxisYTitle( yName );
    plot->clearLegend( );


    //
    // Draw background.
    // ----------------
    // Clear and draw the plot background.
    plot->clearAndClearPlotArea( );


    //
    // Plot events.
    // ------------
    // Loop through all of the events and plot dots.
    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ":   Plotting events\n";

    const int dotColorIndex = visual.dotColorIndexes[DEFAULT_DOT_COLOR_INDEX];

    if ( eventTable->areValuesFloats( ) == true )
    {
        const float*const xEvent =
            eventTable->getParameterFloats( application.xParameterNumber ).data( );
        const float*const yEvent =
            eventTable->getParameterFloats( application.yParameterNumber ).data( );
        for ( size_t i = 0; i < nEvents; ++i )
            plot->plotDot( xEvent[i], yEvent[i], dotColorIndex );
    }
    else
    {
        const double*const xEvent =
            eventTable->getParameterDoubles( application.xParameterNumber ).data( );
        const double*const yEvent =
            eventTable->getParameterDoubles( application.yParameterNumber ).data( );
        for ( size_t i = 0; i < nEvents; ++i )
            plot->plotDot( xEvent[i], yEvent[i], dotColorIndex );
    }


    //
    // Draw foreground.
    // ----------------
    // Draw the plot border, tic marks, axis titles, and axis numbers.
    // Drawing the border after the dots cleans up the edge of the plot.
    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ":   Drawing borders and labels\n";

    plot->drawDecoration( );

    return plot;
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
        " [options] eventfile imagefile" << std::endl;
    std::cerr << "Scatter plot events for selected X and Y parameters." <<
        std::endl;


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

    LISTOPTIONGROUP( "Format choice:" );
    LISTOPTION( "--format FORMAT",
        "Specify next file's format." );

    LISTOPTIONGROUP( "Input:" );
    LISTOPTION( "--maxevents N",
        "Maximum number of events to load." );

    LISTOPTIONGROUP( "Output:" );
    LISTOPTION( "--imagebase PATH",
        "Specify plot image file basename." );

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
    LISTOPTION( "--plotmargins L R T B",
        "Set image margins as % of image size." );
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
        "  Flow Gate project:" <<
        std::endl <<
        "    FG binary events, post-compensation" <<
        std::endl <<
        "  Flow Gate project (legacy):" <<
        std::endl <<
        "    FG text events, post-compensation, tab-separated values" <<
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
        "  Plot formats: " <<
        getImageFileFormatExtensionList( ) <<
        std::endl << std::endl;


    std::cerr <<
        "Examples:" <<
        std::endl;

    std::cerr <<
        "  Plot events from an FCS file:" <<
        std::endl <<
        "    " << application.applicationName << " --plotxparameter 0 --plotyparameter 1 events.fcs image.png" <<
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


    if ( longForm == true )
        std::cerr << std::endl;
    std::cerr << packageGroupIndent << "Tools:" << std::endl;

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
            if ( std::strcmp( "versionlong", option ) == 0 )
            {
                printVersionAndExit( true );
            }
            if ( std::strcmp( "version", option ) == 0 )
            {
                printVersionAndExit( false );
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
            // Input.
            //
            if ( std::strcmp( "maxevents", option ) == 0 )
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
            if ( std::strcmp( "imagebase", option ) == 0 )
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
                // Unsupported.
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
            // Plot options.
            //
            if ( std::strcmp( "plotxparameter", option ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing X parameter choice after " ) +
                        argv[0] + ".\n" );

                application.xParameterNumber = std::stol(argv[1]);
                if ( application.xParameterNumber < 0 )
                    printErrorAndExit(
                        "X parameter choice must non-negative.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }

            if ( std::strcmp( "plotyparameter", option ) == 0 )
            {
                if ( argc < 2 )
                    printErrorAndExit(
                        std::string( "Missing Y parameter choice after " ) +
                        argv[0] + ".\n" );

                application.yParameterNumber = std::stol(argv[1]);
                if ( application.yParameterNumber < 0 )
                    printErrorAndExit(
                        "Y parameter choice must non-negative.\n" );

                argc -= 2;
                argv += 2;
                continue;
            }
            if ( std::strcmp( "plottitle", option ) == 0 )
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
            if ( std::strcmp( "plotsize", option ) == 0 )
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
            if ( std::strcmp( "plotmargins", option ) == 0 )
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

            if ( std::strcmp( "plotdotsize", option ) == 0 )
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

            if ( std::strcmp( "plotdotshape", option ) == 0 )
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

            if ( std::strcmp( "plotaxistitlesize", option ) == 0 )
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
            if ( std::strcmp( "plotaxisnumbersize", option ) == 0 )
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
            if ( std::strcmp( "plotaxisnumbermaxdecimal", option ) == 0 )
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
            if ( std::strcmp( "plotaxisxtitlemargin", option ) == 0 )
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
            if ( std::strcmp( "plotaxisytitlemargin", option ) == 0 )
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

            if ( std::strcmp( "plottitlesize", option ) == 0 )
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

        if ( application.imagePath.empty( ) == true )
        {
            application.imagePath = argv[0];
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
            --argc;
            ++argv;
            continue;
        }

        // Too many files.
        printErrorAndExit(
            std::string( "Too many files. Just one is needed.\n" ) );
    }

    // Check that we got essential arguments.
    if ( application.sourcePath.empty( ) == true )
        printErrorAndExit(
            std::string( "An event file to plot is required.\n" ) );

    if ( application.imagePath.empty( ) == true )
    {
        // Create an image file name based on the source file name.
        const size_t dotPosition = application.sourcePath.rfind( '.' );
        if ( dotPosition == std::string::npos )
            application.imagePath = application.sourcePath + ".png";
        else
            application.imagePath =
                application.sourcePath.substr( 0, dotPosition ) + ".png";
        application.imageFormat = PNG_TRUE_COLOR;
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
    // Parse command-line arguments to select an input file and visual
    // attributes.
    parseArguments( argc, argv );


    //
    // Determine input and output file types.
    // --------------------------------------
    // Use the file name extension, or command-line arguments, to determine
    // the input format.
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

    if ( application.imageFormat == UNKNOWN_IMAGE_FORMAT )
    {
        const std::string ext = getFileExtension( application.imagePath );
        if ( ext.empty( ) == true )
            printErrorAndExit(
                std::string( "\"" ) + application.imagePath +
                "\" has no file name extension.\n" +
                "Use --format to specify the file format.\n" );

        application.imageFormat = findImageFileFormat( ext );
        if ( application.imageFormat == UNKNOWN_IMAGE_FORMAT )
            printErrorAndExit(
                std::string( "\"" ) + application.imagePath +
                "\" file name extension is not recognized.\n" +
                "Use --format to specify the file format.\n" );
    }

    if ( application.benchmark == true &&
         application.displayFormat == JSON_FORMAT )
        std::cout << "{" << std::endl;

    //
    // Load the data.
    // --------------
    // Load the input file.
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
                if ( application.verbose == true ||
                     application.showWarnings == true )
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
            }
            catch (const std::exception& e)
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
            }
            catch (const std::exception& e)
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
            std::exit( 1 );
    }

    const double timeAtEndOfLoad = getRealTime( );
    if ( application.benchmark == true )
        printBenchmark(
            "load",
            (timeAtEndOfLoad - timeAtStartOfLoad) );


    //
    // Set default title.
    // ------------------
    // Use the source file path to create a default plot title, if none
    // was specified.
    if ( visual.title.empty( ) == true )
    {
        const size_t lastSlash = application.sourcePath.find_last_of( "/\\" );
        std::string basename;
        if ( lastSlash == std::string::npos )
            basename = application.sourcePath;
        else
            basename = application.sourcePath.substr( lastSlash + 1 );

        const size_t lastDot = basename.rfind( '.' );
        if ( lastDot == std::string::npos )
            visual.title = basename;
        else
            visual.title = basename.substr( 0, lastDot );
    }

    //
    // Plot.
    // -----
    // Use the selected X-Y parameters and plot all events in the data.
    ScatterPlot* scatterPlot = nullptr;
    try
    {
        if ( application.verbose == true )
        {
            std::cerr << application.applicationName << ":\n";
            std::cerr << application.applicationName << ": Plotting:\n";
        }

        const double timeAtStartOfPlot = getRealTime( );

        scatterPlot = plot( eventTable );

        const double timeAtEndOfPlot = getRealTime( );
        if ( application.benchmark == true )
            printBenchmark(
                "plot",
                (timeAtEndOfPlot - timeAtStartOfPlot) );
    }
    catch ( const std::exception& e )
    {
        std::cerr << e.what( ) << std::endl;
        std::exit( 1 );
    }


    //
    // Save the plot image.
    // --------------------
    // Strip off the filename extension and append the correct one based
    // on the selected output file format. Then save the plot.
    if ( application.verbose == true )
        std::cerr << application.applicationName <<
            ": Saving image file \"" << application.imagePath << "\".\n";

    if ( application.verbose == true )
    {
        std::cerr << application.applicationName << ":\n";
        std::cerr << application.applicationName << ": Saving image:\n";
    }

    const double timeAtStartOfSave = getRealTime( );

    scatterPlot->save( application.imagePath );

    const double timeAtEndOfSave = getRealTime( );
    if ( application.benchmark == true )
    {
        printBenchmark(
            "save",
            (timeAtEndOfSave - timeAtStartOfSave),
            true);

        if ( application.displayFormat == JSON_FORMAT )
            std::cerr << "}" << std::endl;
    }

    delete scatterPlot;
}
