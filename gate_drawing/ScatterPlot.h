/**
 * @file
 * Creates scatter plots.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li libgd
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





//
// Includes.
// ---------
// This software intentionally uses a mix of C and C++. C is used for
// standard data types and stdio for fast buffered I/O.
//
// Data types.
#include <cstdint>      // size_t, uint32_t, uint16_t, uint8_t, ...

// Standard C libraries.
#include <cstdio>       // std::FILE, std::fopen, std::fgets, std::fclose, ...
#include <cstring>      // std::strerror, ...
#include <cerrno>       // Errno

// Standard C++ libraries.
#include <stdexcept>    // std::invalid_argument
#include <string>       // std::string
#include <vector>       // std::vector

// Drawing library.
#include <gd.h>         // gd*() for image drawing





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
#define FLOWGATE_SCATTERPLOT_USE_FLOCKFILE
#endif





namespace FlowGate {
namespace Plot {





/**
 * Creates scatter plots quickly.
 *
 * The constructors create an image of a fixed width and height, and
 * further functions draw dots, lines, and rectangles into the image.
 * save() writes the image to a PNG file.
 *
 * This classes style includes a large number of get/set methods to access
 * and set drawing attributes, such as colors, fonts, font sizes, line
 * thicknesses, and so forth. Once set, a number of draw and plot methods
 * use these attributes to draw shapes or text.
 *
 * <B>Warning:</B> This is not a general-purpose implementation of a scatter
 * plot class. It is a minimal implementation intended for plotting
 * flow cytometry scatter plots.
 *
 * @internal
 * This implementation uses libgd, which manages an image, colors, and
 * fonts, and provides functions to draw dots, lines, rectangles, etc.
 * libgd is not part of standard C or C++, but it is a standard library
 * available for most OSes.
 *
 * There are several libgd quirks we need to work around:
 *
 * - Plots are conventionally drawn with X increasing to the right, and
 *   Y increasing upwards. libgd, however, flips Y so that pixel (0,0) is
 *   at the upper left corner. We compensate here so that scatter plots
 *   are presented as if Y increases upwards.
 *
 * - String rendering in libgd requires a "char*", but all we can get
 *   back from a std::string is a "const char*", which is illegal to
 *   cast to "char*" just for libgd. So, we instead maintain strings
 *   internally as "char*" in allocated space, though the public method
 *   interface supports std::string.
 *
 * - Drawing a string requires an (x,y) position for it. To get that (x,y),
 *   we need to know how wide and tall the string is so we can calculate
 *   (x,y) for centering, etc. This requires issuing a libgd call to get
 *   that string size, which libgd then passes on to libfreetype for font
 *   handling. And libfreetype appears to be both slow and single-threaded.
 *   To reduce the cost of this, we cache these sizes on the first encounter
 *   of a string, and re-use the same sizes on all further draws of the same
 *   string.
 *
 * - When getting the string width and height, libgd returns a bounding
 *   rectangle for the specific string. If the string has no characters
 *   with descenders (e.g. "y", "g", "j"), then the bottom of that rectangle
 *   will be at the text base line. But if there are descenders, the bottom
 *   is at the bottom of the lowest descender. In either case, the text
 *   draw function needs an (x,y) of the base line, not the bottom of the
 *   bounding box. There is no way to get this. There is no way to query
 *   for the base line location, or the height of descenders.
 *
 * libgd is not fast, but it is minimal and stable. If better performance is
 * needed, libgd could be replaced with custom code to maintain an array of
 * pixels and an array of colors. libpng could be called directly to write the
 * image array to a PNG file. libfreetype could be called directly for fonts
 * and text.
 * @endinternal
 */
class ScatterPlot final
{
//----------------------------------------------------------------------
//
// Constants - Version.
//
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate scatter plotter";

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



//----------------------------------------------------------------------
//
// Constants - special values.
//
//----------------------------------------------------------------------
private:
    /**
     * Draw dots as circles.
     *
     * see dotShape
     */
    static const unsigned char DOT_CIRCLE = 0;

    /**
     * Draw dots as squares.
     *
     * see dotShape
     */
    static const unsigned char DOT_SQUARE = 1;

    /**
     * Transform coordinate without bounds checks.
     *
     * @see convertAxisXToImageX()
     * @see convertAxisYToImageY()
     */
    static const unsigned char NO_BOUNDS_CHECK = 0;

    /**
     * Reject coordinate if out of bounds.
     *
     * @see convertAxisXToImageX()
     * @see convertAxisYToImageY()
     */
    static const unsigned char BOUNDS_REJECT = 1;

    /**
     * Crop coordinate if out of bounds.
     *
     * @see convertAxisXToImageX()
     * @see convertAxisYToImageY()
     */
    static const unsigned char BOUNDS_CROP = 2;



//----------------------------------------------------------------------
//
// Constants - defaults.
//
//----------------------------------------------------------------------
private:
    /**
     * The default bottom margin.
     */
    static const unsigned short DEFAULT_PLOT_BOTTOM_MARGIN = 100;

    /**
     * The default left margin.
     */
    static const unsigned short DEFAULT_PLOT_LEFT_MARGIN   = 100;

    /**
     * The default right margin.
     */
    static const unsigned short DEFAULT_PLOT_RIGHT_MARGIN  = 100;

    /**
     * The default top margin.
     */
    static const unsigned short DEFAULT_PLOT_TOP_MARGIN    = 100;

    /**
     * The default length for tic marks.
     */
    static const unsigned short DEFAULT_TIC_LENGTH = 16;

    /**
     * The default line width for tic marks.
     */
    static const unsigned char DEFAULT_TIC_LINE_WIDTH = 2;

    /**
     * the default line width for the plot area border.
     */
    static const unsigned char DEFAULT_BORDER_LINE_WIDTH = 2;

    /**
     * The default size of dots.
     */
    static const unsigned char DEFAULT_DOT_SIZE = 1;

    /**
     * The default dot shape.
     */
    static const int DEFAULT_DOT_SHAPE = DOT_SQUARE;

    /**
     * The default title font size.
     */
    static const int DEFAULT_TITLE_FONT_SIZE = 54;

    /**
     * The default axis title font size.
     */
    static const int DEFAULT_AXIS_TITLE_FONT_SIZE = 46;

    /**
     * The default axis number font size.
     */
    static const int DEFAULT_AXIS_NUMBER_FONT_SIZE = 32;

    /**
     * The default legend font size.
     */
    static const int DEFAULT_LEGEND_FONT_SIZE = 32;

    /**
     * The default X axis title margin.
     */
    static const int DEFAULT_AXIS_X_TITLE_MARGIN = 0;

    /**
     * The default Y axis title margin.
     */
    static const int DEFAULT_AXIS_Y_TITLE_MARGIN = 0;

    /**
     * The default X axis maximum number of digits after the decimal.
     */
    static const int DEFAULT_AXIS_X_MAX_DIGITS_AFTER_DECIMAL = 5;

    /**
     * The default Y axis maximum number of digits after the decimal.
     */
    static const int DEFAULT_AXIS_Y_MAX_DIGITS_AFTER_DECIMAL = 5;



//----------------------------------------------------------------------
//
// Constants - hard-coded values.
//
//----------------------------------------------------------------------
private:
    /**
     * The dots per inch for the output image file.
     *
     * While this is stored in the image file, it has no impact on the
     * image resolution or content. It is strictly annotation.
     *
     * 300 dpi is the conventional value for print output.
     *
     * There are no method calls to override this.
     */
    static const int DEFAULT_DPI = 300;

    /**
     * The PNG image file compression level.
     *
     * There are no method calls to override this.
     */
    static const int PNG_COMPRESSION = 1;

    /**
     * The spacing between axis numbers and the axis.
     *
     * There are no method calls to override this.
     */
    static const int AXIS_NUMBER_SPACING_FROM_AXIS = 4;



//----------------------------------------------------------------------
//
// Fields - set at construction time and never changed.
//
//----------------------------------------------------------------------
private:
    /**
     * The image height, set at construction time.
     *
     * @see getImageHeight()
     */
    const unsigned int imageHeight;

    /**
     * The image width, set at construction time.
     *
     * @see getImageWidth()
     */
    const unsigned int imageWidth;

    /**
     * The current image.
     *
     * This value is strictly internal and is not returned by any method.
     */
    gdImagePtr image;

    /**
     * The standard black color.
     *
     * @see getBlack()
     */
    int black;

    /**
     * The standard white color.
     *
     * @see getWhite()
     */
    int white;



//----------------------------------------------------------------------
//
// Fields - set by methods and very frequently used.
//
//----------------------------------------------------------------------
private:
    //
    // The next few attributes are intentionally placed together here
    // because they are accessed together and we'd like them to end up
    // nearby in memory in order to improve cache use.
    //

    /**
     * The range of X image coordinates used for the plot area.
     *
     * [0] also may be interpreted as the left X margin, while
     * (imageWidth - [1]) is the right X margin.
     *
     * @see getPlotLeftMargin()
     * @see getPlotRightMargin()
     * @see getPlotXMargins()
     * @see setPlotXMargins()
     */
    unsigned short plotXRange[2];

    /**
     * The range of Y image coordinates used for the plot area.
     *
     * [0] also may be interpreted as the bottom Y margin, while
     * (imageHeight - [1]) is the top Y margin.
     *
     * @see getPlotBottomMargin()
     * @see getPlotTopMargin()
     * @see getPlotYMargins()
     * @see setPlotYMargins()
     */
    unsigned short plotYRange[2];

    /**
     * The plot area width.
     *
     * This is a cached result of (plotXRange[1] - plotXRange[0]).
     *
     * @see setPlotXMargins()
     */
    unsigned short plotWidth;

    /**
     * The plot area height.
     *
     * This is a cached result of (plotYRange[1] - plotYRange[0]).
     *
     * @see setPlotYMargins()
     */
    unsigned short plotHeight;

    /**
     * The range of plot coordinates along X.
     *
     * @see getAxisXRange()
     * @see setAxisXRange()
     */
    float axisXRange[2];

    /**
     * The range of plot coordinates along Y.
     *
     * @see getAxisYRange()
     * @see setAxisYRange()
     */
    float axisYRange[2];

    /**
     * The span of values along the X axis.
     *
     * This is a cached result of (axisXRange[1] - axisXRange[0]).
     *
     * @see setAxisXRange()
     */
    float axisXDelta;

    /**
     * The span of values along the Y axis.
     *
     * This is a cached result of (axisYRange[1] - axisYRange[0]).
     *
     * @see setAxisYRange()
     */
    float axisYDelta;

    /**
     * The diameter of dots.
     *
     * @see getDotSize()
     * @see setDotSize()
     */
    unsigned char dotSize;

    /**
     * The shape for dots.
     *
     * @see isDotCircle()
     * @see isDotSquare()
     * @see setDotCircle()
     * @see setDotSquare()
     *
     * @internal
     * This could have been implemented as an enum. C++ has two types of
     * enums: unscoped and scoped. An unscoped enum exposes all of the
     * enum value names, which we don't want to do. A scoped enum keeps the
     * internal values private, but it adds overhead. To keep the
     * implementation simple, fast, and compact, we just use a simple
     * 1-byte integer and internal constant values.
     * @endinternal
     */
    unsigned char dotShape;



//----------------------------------------------------------------------
//
// Fields - set by methods and less frequently used.
//
//----------------------------------------------------------------------
private:
    /**
     * The full image background color.
     *
     * @see getBackgroundColor()
     * @see setBackgroundColor()
     */
    int backgroundColor;

    /**
     * The plot area background color.
     *
     * @see getPlotBackgroundColor()
     * @see setPlotBackgroundColor()
     */
    int plotBackgroundColor;

    /**
     * The plot area border color.
     *
     * @see getPlotBorderColor()
     * @see setPlotBorderColor()
     */
    int plotBorderColor;

    /**
     * The plot area border line width.
     *
     * @see getPlotBorderLineWidth()
     * @see setPlotBorderLineWidth()
     */
    unsigned char plotBorderLineWidth;

    /**
     * The title string.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getTitle()
     * @see setTitle()
     */
    char* title;

    /**
     * The title color.
     *
     * @see getTitleFontcolor()
     * @see setTitleFontcolor()
     */
    int titleColor;

    /**
     * The title font names.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getTitleFontNames()
     * @see setTitleFontNames()
     */
    char* titleFontNames;

    /**
     * The title font size.
     *
     * @see getTitleFontSize()
     * @see setTitleFontSize()
     */
    int titleFontSize;

    /**
     * The X axis title string.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getAxisXTitle()
     * @see setAxisXTitle()
     */
    char* axisXTitle;

    /**
     * The Y axis title string.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getAxisYTitle()
     * @see setAxisYTitle()
     */
    char* axisYTitle;

    /**
     * The X axis title color.
     *
     * @see getAxisXTitleColor()
     * @see setAxisXTitleColor()
     */
    int axisXTitleColor;

    /**
     * The Y axis title color.
     *
     * @see getAxisYTitleColor()
     * @see setAxisYTitleColor()
     */
    int axisYTitleColor;

    /**
     * The X axis title font names.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getAxisXTitleFontNames()
     * @see setAxisXTitleFontNames()
     */
    char* axisXTitleFontNames;

    /**
     * The X axis margin.
     *
     * @see getAxisXTitleMargin()
     * @see setAxisXTitleMargin()
     */
    unsigned short axisXTitleMargin;

    /**
     * The Y axis title font names.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getAxisYTitleFontNames()
     * @see setAxisYTitleFontNames()
     */
    char* axisYTitleFontNames;

    /**
     * The X axis title font size.
     *
     * @see getAxisXTitleFontSize()
     * @see setAxisXTitleFontSize()
     */
    int axisXTitleFontSize;

    /**
     * The Y axis title font size.
     *
     * @see getAxisYTitleFontSize()
     * @see setAxisYTitleFontSize()
     */
    int axisYTitleFontSize;

    /**
     * The X axis number color.
     *
     * @see getAxisYNumberColor()
     * @see setAxisYNumberColor()
     */
    int axisXNumberColor;

    /**
     * The Y axis number color.
     *
     * @see getAxisYNumberColor()
     * @see setAxisYNumberColor()
     */
    int axisYNumberColor;

    /**
     * The X axis number font names.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getAxisXNumberFontNames()
     * @see setAxisXNumberFontNames()
     */
    char* axisXNumberFontNames;

    /**
     * The Y axis number font names.
     *
     * This is a local dynamically allocated copy of the string.
     *
     * @see getAxisYNumberFontNames()
     * @see setAxisYNumberFontNames()
     */
    char* axisYNumberFontNames;

    /**
     * The Y axis margin.
     *
     * @see getAxisYTitleMargin()
     * @see setAxisYTitleMargin()
     */
    unsigned short axisYTitleMargin;

    /**
     * The X axis number font size.
     *
     * @see getAxisXNumberFontSize()
     * @see setAxisXNumberFontSize()
     */
    int axisXNumberFontSize;

    /**
     * The Y axis number font size.
     *
     * @see getAxisYNumberFontNames()
     * @see setAxisYNumberFontNames()
     */
    int axisYNumberFontSize;

    /**
     * The X axis maximum number of digits after the decimal.
     *
     * @see getAxisXMaximumDigitsAfterDecimal()
     * @see setAxisXMaximumDigitsAfterDecimal()
     */
    int axisXMaxDigitsAfterDecimal;

    /**
     * The Y axis maximum number of digits after the decimal.
     *
     * @see getAxisYMaximumDigitsAfterDecimal()
     * @see setAxisYMaximumDigitsAfterDecimal()
     */
    int axisYMaxDigitsAfterDecimal;

    /**
     * The X axis tic color.
     *
     * @see getAxisXTicColor()
     * @see setAxisXTicColor()
     */
    int axisXTicColor;

    /**
     * The Y axis tic color.
     *
     * @see getAxisYTicColor()
     * @see setAxisYTicColor()
     */
    int axisYTicColor;

    /**
     * The X axis tic length.
     *
     * @see getAxisXTicLength()
     * @see setAxisXTicLength()
     */
    unsigned short axisXTicLength;

    /**
     * The Y axis tic length.
     *
     * @see getAxisYTicLength()
     * @see setAxisYTicLength()
     */
    unsigned short axisYTicLength;

    /**
     * The X axis tic line width.
     *
     * @see getAxisXTicLineWidth()
     * @see setAxisXTicLineWidth()
     */
    unsigned char axisXTicLineWidth;

    /**
     * The Y axis tic line width.
     *
     * @see getAxisYTicLineWidth()
     * @see setAxisYTicLineWidth()
     */
    unsigned char axisYTicLineWidth;

    /**
     * The X axis list of tic values.
     *
     * @see getAxisXTics()
     * @see setAxisXTics()
     */
    std::vector<float> axisXTics;

    /**
     * The Y axis list of tic values.
     *
     * @see getAxisYTics()
     * @see setAxisYTics()
     */
    std::vector<float> axisYTics;

    /**
     * The legend font names.
     *
     * @see getLegendFontNames()
     * @see setLegendFontNames()
     */
    char* legendFontNames;

    /**
     * The legend font size.
     *
     * @see getLegendFontSize()
     * @see setLegendFontSize()
     */
    int legendFontSize;

    /**
     * The legend list of colors.
     *
     * @see getLegendColors()
     * @see setLegend()
     */
    std::vector<int> legendColors;

    /**
     * The legend list of values.
     *
     * @see getLegendValues()
     * @see setLegend()
     */
    std::vector<char*> legendValues;



//----------------------------------------------------------------------
//
// Fields - cached text widths and heights.
//
//----------------------------------------------------------------------
private:
    /**
     * The cached title width, or zero if unknown.
     *
     * This field is set to zero when the title, title font, or title size
     * is changed. The field is set to non-zero on the first draw of the
     * title.
     *
     * @see setTitle()
     * @see setTitleFontNames()
     * @see setTitleFontSize()
     */
    int titleWidth;

    /**
     * The cached title height, or zero if unknown.
     *
     * This field is set to zero when the title, title font, or title size
     * is changed. The field is set to non-zero on the first draw of the
     * title.
     *
     * @see setTitle()
     * @see setTitleFontNames()
     * @see setTitleFontSize()
     */
    int titleHeight;

    /**
     * The cached X axis title width, or zero if unknown.
     *
     * This field is set to zero when the title, title font, or title size
     * is changed. The field is set to non-zero on the first draw of the
     * title.
     *
     * @see setAxisXTitle()
     * @see setAxisXTitleFontNames()
     * @see setAxisXTitleFontSize()
     */
    int axisXTitleWidth;

    /**
     * The cached X axis title height, or zero if unknown.
     *
     * This field is set to zero when the title, title font, or title size
     * is changed. The field is set to non-zero on the first draw of the
     * title.
     *
     * @see setAxisXTitle()
     * @see setAxisXTitleFontNames()
     * @see setAxisXTitleFontSize()
     */
    int axisXTitleHeight;

    /**
     * The cached Y axis title width, or zero if unknown.
     *
     * This field is set to zero when the title, title font, or title size
     * is changed. The field is set to non-zero on the first draw of the
     * title.
     *
     * @see setAxisYTitle()
     * @see setAxisYTitleFontNames()
     * @see setAxisYTitleFontSize()
     */
    int axisYTitleWidth;

    /**
     * The cached Y axis title height, or zero if unknown.
     *
     * This field is set to zero when the title, title font, or title size
     * is changed. The field is set to non-zero on the first draw of the
     * title.
     *
     * @see setAxisYTitle()
     * @see setAxisYTitleFontNames()
     * @see setAxisYTitleFontSize()
     */
    int axisYTitleHeight;

    /**
     * The cached X axis number widths, or zero if unknown.
     *
     * This field is set to zeros when the tic numbers, number font, or
     * number size are changed. The field is set to non-zero on the first
     * draw of the numbers.
     *
     * @see setAxisXTics()
     * @see setAxisXNumberFontNames()
     * @see setAxisXNumberFontSize()
     */
    std::vector<int> axisXNumberWidths;

    /**
     * The cached X axis number heights, or zero if unknown.
     *
     * This field is set to zeros when the tic numbers, number font, or
     * number size are changed. The field is set to non-zero on the first
     * draw of the numbers.
     *
     * @see setAxisXTics()
     * @see setAxisXNumberFontNames()
     * @see setAxisXNumberFontSize()
     */
    std::vector<int> axisXNumberHeights;

    /**
     * The cached Y axis number widths, or zero if unknown.
     *
     * This field is set to zeros when the tic numbers, number font, or
     * number size are changed. The field is set to non-zero on the first
     * draw of the numbers.
     *
     * @see setAxisYTics()
     * @see setAxisYNumberFontNames()
     * @see setAxisYNumberFontSize()
     */
    std::vector<int> axisYNumberWidths;

    /**
     * The cached Y axis number heights, or zero if unknown.
     *
     * This field is set to zeros when the tic numbers, number font, or
     * number size are changed. The field is set to non-zero on the first
     * draw of the numbers.
     *
     * @see setAxisYTics()
     * @see setAxisYNumberFontNames()
     * @see setAxisYNumberFontSize()
     */
    std::vector<int> axisYNumberHeights;

    /**
     * The cached legend widths, or zero if unknown.
     *
     * This field is set to zeros when the legend values, legend font, or
     * legend size are changed. The field is set to non-zero on the first
     * draw of the legend.
     *
     * @see setLegend()
     * @see setLegendFontNames()
     * @see setLegendFontSize()
     */
    std::vector<int> legendWidths;

    /**
     * The cached legend heights, or zero if unknown.
     *
     * This field is set to zeros when the legend values, legend font, or
     * legend size are changed. The field is set to non-zero on the first
     * draw of the legend.
     *
     * @see setLegend()
     * @see setLegendFontNames()
     * @see setLegendFontSize()
     */
    std::vector<int> legendHeights;



//----------------------------------------------------------------------
//
// Constructors / Destructors
//
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Creates a new scatter plot image with default settings.
     *
     * The image is not cleared and will have undetermined content.
     * Applications should call clear() or clearAndClearPlotArea()
     * before plotting data.
     *
     * @param[in] imageWidth
     *   The image width, in pixels.
     * @param[in] imageHeight
     *   The image height, in pixels.
     *
     * @see clear()
     * @see clearAndClearPlotArea()
     */
    ScatterPlot( const unsigned int imageWidth, const unsigned int imageHeight )
        : imageHeight( imageHeight ),
          imageWidth( imageWidth )
    {
        // Initialize pointers. These are allocated as needed.
        this->title                = nullptr;
        this->titleFontNames       = nullptr;
        this->axisXTitle           = nullptr;
        this->axisYTitle           = nullptr;
        this->axisXTitleFontNames  = nullptr;
        this->axisYTitleFontNames  = nullptr;
        this->axisXNumberFontNames = nullptr;
        this->axisYNumberFontNames = nullptr;
        this->legendFontNames      = nullptr;

        // Insure the font cache has been initialized.
        gdFontCacheSetup( );
        gdFTUseFontConfig( true );

        // Create the image.
        this->image = gdImageCreate( imageWidth, imageHeight );
        gdImageSetResolution( this->image, DEFAULT_DPI, DEFAULT_DPI );

        // Initialize default colors.
        this->resetColors( );

        // Reset all attributes to their defaults.
        this->reset( );
    }

    /**
     * Creates a new scatter plot image by copying another plot.
     *
     * The new plot will have the same size, content, and attributes as
     * the plot being copied.
     *
     * @param[in] plotToCopy
     *   The plot to copy.
     *
     * @internal
     * When creating multiple images, it can be faster to create a single
     * template image using the creation constructor, and then copy it
     * multiple times using this copy constructor.
     * @endinternal
     */
    ScatterPlot( const ScatterPlot*const plotToCopy )
        : imageHeight( plotToCopy->imageHeight ),
          imageWidth( plotToCopy->imageWidth )
    {
        // Initialize pointers. These are allocated as needed.
        this->title                = nullptr;
        this->titleFontNames       = nullptr;
        this->axisXTitle           = nullptr;
        this->axisYTitle           = nullptr;
        this->axisXTitleFontNames  = nullptr;
        this->axisYTitleFontNames  = nullptr;
        this->axisXNumberFontNames = nullptr;
        this->axisYNumberFontNames = nullptr;
        this->legendFontNames      = nullptr;

        // Copy the image.
        this->image = gdImageClone( plotToCopy->image );

        // Copy default colors.
        this->white = plotToCopy->white;
        this->black = plotToCopy->black;
        this->backgroundColor = plotToCopy->backgroundColor;

        // Title.
        this->setTitleFontNames( plotToCopy->getTitleFontNames( ) );
        this->setTitleFontSize(  plotToCopy->titleFontSize );
        this->setTitleColor(     plotToCopy->titleColor );
        this->setTitle(          plotToCopy->getTitle( ) );

        // Plot area.
        this->plotBackgroundColor = plotToCopy->plotBackgroundColor;
        this->plotBorderColor     = plotToCopy->plotBorderColor;
        this->plotBorderLineWidth = plotToCopy->plotBorderLineWidth;

        this->plotXRange[0] = plotToCopy->plotXRange[0];
        this->plotXRange[1] = plotToCopy->plotXRange[1];
        this->plotWidth     = plotToCopy->plotWidth;

        this->plotYRange[0] = plotToCopy->plotYRange[0];
        this->plotYRange[1] = plotToCopy->plotYRange[1];
        this->plotHeight    = plotToCopy->plotHeight;

        // Drawing.
        this->dotSize       = plotToCopy->dotSize;
        this->dotShape      = plotToCopy->dotShape;

        // X axis.
        this->setAxisXTitleFontNames(  plotToCopy->getAxisXTitleFontNames( ) );
        this->setAxisXNumberFontNames( plotToCopy->getAxisXNumberFontNames( ) );
        this->setAxisXTitleFontSize(   plotToCopy->axisXTitleFontSize );
        this->setAxisXTitleColor(      plotToCopy->axisXTitleColor );
        this->setAxisXTitle(           plotToCopy->axisXTitle );
        this->setAxisXTitleMargin(     plotToCopy->axisXTitleMargin );
        this->setAxisXNumberFontSize(  plotToCopy->axisXNumberFontSize );
        this->setAxisXNumberColor(     plotToCopy->axisXNumberColor );
        this->setAxisXTicColor(        plotToCopy->axisXTicColor );
        this->setAxisXTicLength(       plotToCopy->axisXTicLength );
        this->setAxisXTicLineWidth(    plotToCopy->axisXTicLineWidth );
        this->setAxisXRange(
            plotToCopy->axisXRange[0], plotToCopy->axisXRange[1] );
        this->setAxisXTics(            plotToCopy->axisXTics );
        this->setAxisXMaximumDigitsAfterDecimal( plotToCopy->axisXMaxDigitsAfterDecimal );

        // Y axis.
        this->setAxisYTitleFontNames(  plotToCopy->getAxisYTitleFontNames( ) );
        this->setAxisYNumberFontNames( plotToCopy->getAxisYNumberFontNames( ) );
        this->setAxisYTitleFontSize(   plotToCopy->axisYTitleFontSize );
        this->setAxisYTitleColor(      plotToCopy->axisYTitleColor );
        this->setAxisYTitle(           plotToCopy->axisYTitle );
        this->setAxisYTitleMargin(     plotToCopy->axisYTitleMargin );
        this->setAxisYNumberFontSize(  plotToCopy->axisYNumberFontSize );
        this->setAxisYNumberColor(     plotToCopy->axisYNumberColor );
        this->setAxisYTicColor(        plotToCopy->axisYTicColor );
        this->setAxisYTicLength(       plotToCopy->axisYTicLength );
        this->setAxisYTicLineWidth(    plotToCopy->axisYTicLineWidth );
        this->setAxisYRange(
            plotToCopy->axisYRange[0],
            plotToCopy->axisYRange[1] );
        this->setAxisYTics(            plotToCopy->axisXTics );
        this->setAxisYMaximumDigitsAfterDecimal( plotToCopy->axisYMaxDigitsAfterDecimal );

        // Legend.
        this->setLegend(
            plotToCopy->legendValues,
            plotToCopy->legendColors );
        this->setLegendFontNames(      plotToCopy->legendFontNames );
        this->setLegendFontSize(       plotToCopy->legendFontSize );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the plot.
     */
    virtual ~ScatterPlot( )
        noexcept
    {
        // Destroy the image.
        gdImageDestroy( this->image );

        // Free char array space, if used.
        if ( this->title != nullptr )
            delete this->title;
        if ( this->titleFontNames != nullptr )
            delete this->titleFontNames;
        if ( this->axisXTitle != nullptr )
            delete this->axisXTitle;
        if ( this->axisYTitle != nullptr )
            delete this->axisYTitle;
        if ( this->axisXTitleFontNames != nullptr )
            delete this->axisXTitleFontNames;
        if ( this->axisYTitleFontNames != nullptr )
            delete this->axisYTitleFontNames;
        if ( this->axisXNumberFontNames != nullptr )
            delete this->axisXNumberFontNames;
        if ( this->axisYNumberFontNames != nullptr )
            delete this->axisYNumberFontNames;

        this->clearLegend( );
    }
    // @}



//----------------------------------------------------------------------
//
// Initialize.
//
//----------------------------------------------------------------------
    /**
     * @name Initialization
     */
    // @{
private:
    /**
     * Returns the default font names.
     *
     * @return
     *   The default font names.
     */
    inline static const char* getDefaultFonts( )
        noexcept
    {
        return "Arial";
    }

public:
    /**
     * Resets plot attributes to their defaults.
     */
    void reset( )
        noexcept
    {
        // Image.
        this->backgroundColor = this->white;

        // Title.
        this->setTitleFontNames( this->getDefaultFonts( ) );
        this->setTitleFontSize( DEFAULT_TITLE_FONT_SIZE );
        this->setTitleColor( this->titleColor );
        this->setTitle( "" );

        // Plot area.
        this->plotBackgroundColor = this->white;
        this->plotBorderColor     = this->black;
        this->plotBorderLineWidth = DEFAULT_BORDER_LINE_WIDTH;

        this->setPlotXMargins(
            DEFAULT_PLOT_LEFT_MARGIN,
            DEFAULT_PLOT_RIGHT_MARGIN );
        this->setPlotYMargins(
            DEFAULT_PLOT_BOTTOM_MARGIN,
            DEFAULT_PLOT_TOP_MARGIN );

        // Axis.
        this->setAxisXTitleFontNames( this->getDefaultFonts( ) );
        this->setAxisXNumberFontNames( this->getDefaultFonts( ) );
        this->setAxisXTitleFontSize(  DEFAULT_AXIS_TITLE_FONT_SIZE );
        this->setAxisXNumberFontSize( DEFAULT_AXIS_NUMBER_FONT_SIZE );
        this->setAxisXTitleMargin( DEFAULT_AXIS_X_TITLE_MARGIN );
        this->setAxisXMaximumDigitsAfterDecimal( DEFAULT_AXIS_X_MAX_DIGITS_AFTER_DECIMAL );

        this->setAxisYTitleFontNames( this->getDefaultFonts( ) );
        this->setAxisYNumberFontNames( this->getDefaultFonts( ) );
        this->setAxisYTitleFontSize( DEFAULT_AXIS_TITLE_FONT_SIZE );
        this->setAxisYNumberFontSize(  DEFAULT_AXIS_NUMBER_FONT_SIZE );
        this->setAxisYTitleMargin( DEFAULT_AXIS_Y_TITLE_MARGIN );
        this->setAxisYMaximumDigitsAfterDecimal( DEFAULT_AXIS_Y_MAX_DIGITS_AFTER_DECIMAL );

        this->setAxisXTitle( "X" );
        this->setAxisYTitle( "Y" );

        this->axisXTitleColor  = this->black;
        this->axisXNumberColor = this->black;
        this->axisXTicColor    = this->black;

        this->axisYTitleColor  = this->black;
        this->axisYNumberColor = this->black;
        this->axisYTicColor    = this->black;

        this->axisXTicLength   = DEFAULT_TIC_LENGTH;
        this->axisXTicLineWidth = DEFAULT_TIC_LINE_WIDTH;
        this->axisYTicLength   = DEFAULT_TIC_LENGTH;
        this->axisYTicLineWidth = DEFAULT_TIC_LINE_WIDTH;

        this->setAxisXRange( 0.0, 100.0 );
        this->setAxisYRange( 0.0, 100.0 );

        this->axisXTics.clear( );
        this->axisXTics.shrink_to_fit( );

        this->axisYTics.clear( );
        this->axisYTics.shrink_to_fit( );

        this->axisXNumberWidths.clear( );
        this->axisXNumberHeights.clear( );
        this->axisYNumberWidths.clear( );
        this->axisYNumberHeights.clear( );

        // Legend.
        this->clearLegend( );
        this->setLegendFontNames( this->getDefaultFonts( ) );
        this->setLegendFontSize( DEFAULT_LEGEND_FONT_SIZE );

        // Drawing.
        this->dotSize = DEFAULT_DOT_SIZE;
        this->dotShape = DEFAULT_DOT_SHAPE;
    }
    // @}



//----------------------------------------------------------------------
//
// Colors.
//
//----------------------------------------------------------------------
    /**
     * @name Colors
     */
    // @{
private:
    /**
     * Allocates the default black and white colors.
     *
     * @see getBlack()
     * @see getWhite()
     * @see resetColors()
     */
    inline void allocateDefaultColors( )
        noexcept
    {
        this->white = gdImageColorAllocate( this->image, 0xFF, 0xFF, 0xFF );
        this->black = gdImageColorAllocate( this->image, 0, 0, 0 );
    }

public:
    /**
     * Clears all allocated colors and re-allocates default colors.
     *
     * @see getBlack()
     * @see getWhite()
     */
    inline void resetColors( )
        noexcept
    {
        const int nColors = gdImageColorsTotal( this->image );
        for ( int i = 0; i < nColors; ++i )
            gdImageColorDeallocate( this->image, i );

        this->allocateDefaultColors( );
    }

    /**
     * Allocates a (red,green,blue) color.
     *
     * @param[in] red
     *   The red component.
     * @param[in] green
     *   The green component.
     * @param[in] blue
     *   The blue component.
     *
     * @return
     *   Returns the allocated color.
     */
    inline int allocateColor( const int red, const int green, const int blue )
        noexcept
    {
        return gdImageColorAllocate( this->image, red, green, blue );
    }

    /**
     * Allocates a (red,green,blue) color.
     *
     * @param[in] rgb
     *   The red-green-blue color.
     *
     * @return
     *   Returns the allocated color.
     */
    inline int allocateColor( const int rgb )
        noexcept
    {
        const int red   = (rgb >> 16) & 0xFF;
        const int green = (rgb >>  8) & 0xFF;
        const int blue  = (rgb >>  0) & 0xFF;
        return gdImageColorAllocate( this->image, red, green, blue );
    }

    /**
     * Deallocates a color.
     *
     * Attempts to deallocate the default black or white are ignored.
     *
     * @param[in] color
     *   The color to deallocate.
     */
    inline void deallocateColor( const int color )
        noexcept
    {
        if ( color == this->black || color == this->white )
            return;
        gdImageColorDeallocate( this->image, color );
    }

    /**
     * Returns the default black color.
     *
     * @return
     *   Returns the color.
     */
    inline int getBlack( )
        const noexcept
    {
        return this->black;
    }

    /**
     * Returns the default white color.
     *
     * @return
     *   Returns the color.
     */
    inline int getWhite( )
        const noexcept
    {
        return this->white;
    }
    // @}



//----------------------------------------------------------------------
//
// Image attributes.
//
//----------------------------------------------------------------------
    /**
     * @name Image attributes
     */
    // @{
public:
    /**
     * Clears the image to the background color.
     *
     * @see clearAndClearPlotArea()
     * @see clearPlotArea()
     * @see getBackgroundColor()
     * @see setBackgroundColor()
     */
    inline void clear( )
        noexcept
    {
        gdImageFilledRectangle(
            this->image,
            0,
            0,
            this->imageWidth,
            this->imageHeight,
            this->backgroundColor );
    }

    /**
     * Gets the background color.
     *
     * @return
     *   Returns the color.
     *
     * @see clear()
     * @see setBackgroundColor()
     */
    inline int getBackgroundColor( )
        const noexcept
    {
        return this->backgroundColor;
    }

    /**
     * Gets the image size.
     *
     * @param[out] width
     *   The returned image width.
     * @param[out] height
     *   The returned image height.
     *
     * @see getImageHeight()
     * @see getImageWidth()
     */
    inline void getImageSize( unsigned int& width, unsigned int& height )
        const noexcept
    {
        width  = this->imageWidth;
        height = this->imageHeight;
    }

    /**
     * Gets the image height.
     *
     * @return
     *   The image height.
     *
     * @see getImageSize()
     * @see getImageWidth()
     */
    inline int getImageHeight( )
        const noexcept
    {
        return this->imageHeight;
    }

    /**
     * Gets the image width.
     *
     * @return
     *   The image width.
     *
     * @see getImageSize()
     * @see getImageHeight()
     */
    inline int getImageWidth( )
        const noexcept
    {
        return this->imageWidth;
    }

    /**
     * Gets the image title.
     *
     * @return
     *   The title.
     *
     * @see setTitle()
     */
    inline std::string getTitle( )
        const noexcept
    {
        return std::string( this->title );
    }

    /**
     * Gets the title color.
     *
     * @return
     *   Returns the color.
     *
     * @see setTitleColor()
     */
    inline int getTitleColor( )
        const noexcept
    {
        return this->titleColor;
    }

    /**
     * Gets the title font names.
     *
     * @return
     *   Returns the font names.
     *
     * @see setTitleFontNames()
     */
    inline std::string getTitleFontNames( )
        const noexcept
    {
        return std::string( this->titleFontNames );
    }

    /**
     * Gets the title font size.
     *
     * @return
     *   Returns the font size.
     *
     * @see setTitleFontSize()
     */
    inline int getTitleFontSize( )
        const noexcept
    {
        return this->titleFontSize;
    }



    /**
     * Sets the background color.
     *
     * @param[in] color
     *   The color.
     *
     * @see getBackgroundColor()
     */
    inline void setBackgroundColor( const int color )
        noexcept
    {
        this->backgroundColor = color;
    }

    /**
     * Sets the title.
     *
     * Setting the title to a nullptr or an empty string removes
     * the title.
     *
     * @param[in] title
     *   The title.
     *
     * @see getTitle()
     */
    inline void setTitle( const char*const title )
        noexcept
    {
        // If the title hasn't changed, do nothing.
        if ( title == nullptr && this->title == nullptr )
            return;
        if ( title != nullptr && this->title != nullptr &&
             std::strcmp( title, this->title ) == 0 )
            return;

        // The title has changed. Delete the current title.
        if ( this->title != nullptr )
        {
            delete this->title;
            this->title = nullptr;
            this->titleWidth = this->titleHeight = 0;
        }

        // Set the title.
        if ( title != nullptr )
        {
            this->title = new char[std::strlen( title ) + 1];
            std::strcpy( this->title, title );
        }
    }

    /**
     * Sets the title.
     *
     * Setting the title an empty string removes the title.
     *
     * @param[in] title
     *   The title.
     *
     * @see getTitle()
     */
    inline void setTitle( const std::string title )
        noexcept
    {
        this->setTitle( title.c_str( ) );
    }

    /**
     * Sets the title color.
     *
     * @param[in] color
     *   The color.
     *
     * @see getTitleColor()
     */
    inline void setTitleColor( const int color )
        noexcept
    {
        this->titleColor = color;
    }

    /**
     * Sets the title font names.
     *
     * Setting the font names to a nullptr or an empty string reverts
     * to the defaults.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getTitleFontNames()
     */
    inline void setTitleFontNames( const char*const names )
        noexcept
    {
        // If the font names haven't changed, do nothing.
        if ( names == nullptr && this->titleFontNames == nullptr )
            return;
        if ( names != nullptr && this->titleFontNames != nullptr &&
             std::strcmp( names, this->titleFontNames ) == 0 )
            return;

        // The font names have changed. Delete the current names.
        if ( this->titleFontNames != nullptr )
        {
            delete this->titleFontNames;
            this->titleFontNames = nullptr;
            this->titleWidth = this->titleHeight = 0;
        }

        // Save the names.
        if ( names != nullptr )
        {
            this->titleFontNames = new char[std::strlen( names ) + 1];
            std::strcpy( this->titleFontNames, names );
        }
        else
        {
            const char*const def = this->getDefaultFonts( );
            this->titleFontNames = new char[std::strlen( def ) + 1];
            std::strcpy( this->titleFontNames, def );
        }
    }

    /**
     * Sets the title font names.
     *
     * Setting the font names to an empty string reverts
     * to the defaults.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getTitleFontNames()
     */
    inline void setTitleFontNames( const std::string names )
        noexcept
    {
        this->setTitleFontNames( names.c_str( ) );
    }

    /**
     * Sets the title font size.
     *
     * @param[in] size
     *   The font size.
     *
     * @see getTitleFontSize()
     */
    inline void setTitleFontSize( const int size )
        noexcept
    {
        // If the size hasn't changed, do nothing.
        if ( size == this->titleFontSize )
            return;

        this->titleFontSize = size;
        this->titleWidth = this->titleHeight = 0;
    }
    // @}



//----------------------------------------------------------------------
//
// Axis number attributes.
//
//----------------------------------------------------------------------
    /**
     * @name Axis number attributes
     */
    // @{
public:
    /**
     * Gets the color for X axis numbers.
     *
     * @return
     *   The color.
     *
     * @see setAxisXNumberColor()
     */
    inline int getAxisXNumberColor( )
        const noexcept
    {
        return this->axisXNumberColor;
    }

    /**
     * Gets the font name list for the X axis numbers.
     *
     * @return
     *   The font names.
     *
     * @see setAxisXNumberFontNames()
     */
    inline std::string getAxisXNumberFontNames( )
        const noexcept
    {
        return std::string( this->axisXNumberFontNames );
    }

    /**
     * Gets the font size for the X axis numbers.
     *
     * @return
     *   The font size.
     *
     * @see setAxisXNumberFontSize()
     */
    inline int getAxisXNumberFontSize( )
        const noexcept
    {
        return this->axisXNumberFontSize;
    }

    /**
     * Gets the X axis maximum number of digits after the decimal.
     *
     * @return
     *   The number of digits.
     *
     * @see setAxisXMaximumDigitsAfterDecimal()
     */
    inline int getAxisXMaximumDigitsAfterDecimal( )
        const noexcept
    {
        return this->axisXMaxDigitsAfterDecimal;
    }

    /**
     * Gets the numeric range of the X axis.
     *
     * @param[out] low
     *   Returns the low end of the range.
     * @param[out] high
     *   Returns the high end of the range.
     *
     * @see setAxisXRange()
     */
    inline void getAxisXRange( float& low, float& high )
        noexcept
    {
        low  = this->axisXRange[0];
        high = this->axisXRange[1];
    }

    /**
     * Gets the color for Y axis numbers.
     *
     * @return
     *   The color.
     *
     * @see setAxisYNumberColor()
     */
    inline int getAxisYNumberColor( )
        const noexcept
    {
        return this->axisYNumberColor;
    }

    /**
     * Gets the font name list for the Y axis numbers.
     *
     * @return
     *   The font names.
     *
     * @see setAxisYNumberFontNames()
     */
    inline std::string getAxisYNumberFontNames( )
        const noexcept
    {
        return std::string( this->axisYNumberFontNames );
    }

    /**
     * Gets the font size for the Y axis numbers.
     *
     * @return
     *   The font size.
     *
     * @see setAxisYNumberFontSize()
     */
    inline int getAxisYNumberFontSize( )
        const noexcept
    {
        return this->axisYNumberFontSize;
    }

    /**
     * Gets the Y axis maximum number of digits after the decimal.
     *
     * @return
     *   The number of digits.
     *
     * @see setAxisYMaximumDigitsAfterDecimal()
     */
    inline int getAxisYMaximumDigitsAfterDecimal( )
        const noexcept
    {
        return this->axisYMaxDigitsAfterDecimal;
    }

    /**
     * Gets the numeric range of the Y axis.
     *
     * @param[out] low
     *   Returns the low end of the range.
     * @param[out] high
     *   Returns the high end of the range.
     *
     * @see setAxisYRange()
     */
    inline void getAxisYRange( int& low, int& high )
        noexcept
    {
        low  = this->axisYRange[0];
        high = this->axisYRange[1];
    }



    /**
     * Sets the color for X and Y axis numbers.
     *
     * @param[in] color
     *   The color.
     *
     * @see getAxisXNumberColor()
     * @see getAxisYNumberColor()
     * @see setAxisXNumberColor()
     * @see setAxisYNumberColor()
     */
    inline void setAxisNumberColor( const int color )
        noexcept
    {
        setAxisXNumberColor( color );
        setAxisYNumberColor( color );
    }

    /**
     * Sets the font name list for the X axis numbers.
     *
     * Setting the font names to a nullptr or an empty string reverts
     * them to defaults.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getAxisXNumberFontNames()
     */
    inline void setAxisXNumberFontNames( const char*const names )
        noexcept
    {
        // If the font names haven't changed, do nothing.
        if ( names == nullptr && this->axisXNumberFontNames == nullptr )
            return;
        if ( names != nullptr && this->axisXNumberFontNames != nullptr &&
             std::strcmp( names, this->axisXNumberFontNames ) == 0 )
            return;

        // The font names have changed. Delete the current names.
        if ( this->axisXNumberFontNames != nullptr )
        {
            delete this->axisXNumberFontNames;
            this->axisXNumberFontNames = nullptr;

            // Reset the text widths and heights lists to zeroes.
            const size_t n = this->axisXNumberWidths.size( );
            this->axisXNumberWidths.assign( n, 0 );
            this->axisXNumberHeights.assign( n, 0 );
        }

        // Save the names.
        if ( names != nullptr )
        {
            this->axisXNumberFontNames = new char[std::strlen( names ) + 1];
            std::strcpy( this->axisXNumberFontNames, names );
        }
        else
        {
            const char*const def = this->getDefaultFonts( );
            this->axisXNumberFontNames = new char[std::strlen( def ) + 1];
            std::strcpy( this->axisXNumberFontNames, def );
        }
    }

    /**
     * Sets the font name list for the X axis numbers.
     *
     * Setting the font names to an empty string reverts
     * them to defaults.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getAxisXNumberFontNames()
     */
    inline void setAxisXNumberFontNames( const std::string names )
        noexcept
    {
        this->setAxisXNumberFontNames( names.c_str( ) );
    }

    /**
     * Sets the font size for the X axis numbers.
     *
     * @param[in] size
     *   The font size.
     *
     * @see getAxisXNumberFontSize()
     */
    inline void setAxisXNumberFontSize( const int size )
        noexcept
    {
        if ( size == this->axisXNumberFontSize )
            return;

        this->axisXNumberFontSize = size;

        // Reset the text widths and heights lists to zeroes.
        const size_t n = this->axisXNumberWidths.size( );
        this->axisXNumberWidths.assign( n, 0 );
        this->axisXNumberHeights.assign( n, 0 );
    }

    /**
     * Sets the X axis maximum number of digits after the decimal.
     *
     * @param[in] digits
     *   The number of digits.
     *
     * @see getAxisXMaximumDigitsAfterDecimal()
     */
    inline void setAxisXMaximumDigitsAfterDecimal( const int digits )
        noexcept
    {
        this->axisXMaxDigitsAfterDecimal = digits;
    }

    /**
     * Sets the color for X axis numbers.
     *
     * @param[in] color
     *   The color.
     *
     * @see getAxisXNumberColor()
     */
    inline void setAxisXNumberColor( const int color )
        noexcept
    {
        this->axisXNumberColor = color;
    }

    /**
     * Sets the numeric range of the X axis.
     *
     * @param[in] low
     *   The low end of the range.
     * @param[in] high
     *   The high end of the range.
     *
     * @see getAxisXRange()
     */
    inline void setAxisXRange( const float low, const float high )
        noexcept
    {
        this->axisXRange[0] = low;
        this->axisXRange[1] = high;
        this->axisXDelta = high - low;
    }

    /**
     * Sets the color for Y axis numbers.
     *
     * @param[in] color
     *   The color.
     *
     * @see getAxisYNumberColor()
     */
    inline void setAxisYNumberColor( const int color )
        noexcept
    {
        this->axisYNumberColor = color;
    }

    /**
     * Sets the font name list for the Y axis numbers.
     *
     * Setting the font names to a nullptr or an empty string reverts
     * them to defaults.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getAxisYNumberFontNames()
     */
    inline void setAxisYNumberFontNames( const char*const names )
        noexcept
    {
        // If the font names haven't changed, do nothing.
        if ( names == nullptr && this->axisYNumberFontNames == nullptr )
            return;
        if ( names != nullptr && this->axisYNumberFontNames != nullptr &&
             std::strcmp( names, this->axisYNumberFontNames ) == 0 )
            return;

        // The font names have changed. Delete the current names.
        if ( this->axisYNumberFontNames != nullptr )
        {
            delete this->axisYNumberFontNames;
            this->axisYNumberFontNames = nullptr;

            // Reset the text widths and heights lists to zeroes.
            const size_t n = this->axisYNumberWidths.size( );
            this->axisYNumberWidths.assign( n, 0 );
            this->axisYNumberHeights.assign( n, 0 );
        }

        // Save the names.
        if ( names != nullptr )
        {
            this->axisYNumberFontNames = new char[std::strlen( names ) + 1];
            std::strcpy( this->axisYNumberFontNames, names );
        }
        else
        {
            const char*const def = this->getDefaultFonts( );
            this->axisYNumberFontNames = new char[std::strlen( def ) + 1];
            std::strcpy( this->axisYNumberFontNames, def );
        }
    }

    /**
     * Sets the font name list for the Y axis numbers.
     *
     * Setting the font names to an empty string reverts
     * them to defaults.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getAxisYNumberFontNames()
     */
    inline void setAxisYNumberFontNames( const std::string names )
        noexcept
    {
        this->setAxisYNumberFontNames( names.c_str( ) );
    }

    /**
     * Sets the font size for the Y axis numbers.
     *
     * @param[in] size
     *   The font size.
     *
     * @see getAxisYNumberFontSize()
     */
    inline void setAxisYNumberFontSize( const int size )
        noexcept
    {
        if ( size == this->axisYNumberFontSize )
            return;

        this->axisYNumberFontSize = size;

        // Reset the text widths and heights lists to zeroes.
        const size_t n = this->axisYNumberWidths.size( );
        this->axisYNumberWidths.assign( n, 0 );
        this->axisYNumberHeights.assign( n, 0 );
    }

    /**
     * Sets the Y axis maximum number of digits after the decimal.
     *
     * @param[in] digits
     *   The number of digits.
     *
     * @see getAxisYMaximumDigitsAfterDecimal()
     */
    inline void setAxisYMaximumDigitsAfterDecimal( const int digits )
        noexcept
    {
        this->axisYMaxDigitsAfterDecimal = digits;
    }

    /**
     * Sets the numeric range of the Y axis.
     *
     * @param[in] low
     *   The low end of the range.
     * @param[in] high
     *   The high end of the range.
     *
     * @see getAxisYRange()
     */
    inline void setAxisYRange( const float low, const float high )
        noexcept
    {
        this->axisYRange[0] = low;
        this->axisYRange[1] = high;
        this->axisYDelta = high - low;
    }
    // @}



//----------------------------------------------------------------------
//
// Axis title attributes.
//
//----------------------------------------------------------------------
    /**
     * @name Axis title attributes
     */
    // @{
public:
    /**
     * Gets the X axis title.
     *
     * @return
     *   The title.
     */
    inline const std::string getAxisXTitle( )
        const noexcept
    {
        return this->axisXTitle;
    }

    /**
     * Gets the color the X axis title.
     *
     * @return
     *   The color.
     */
    inline int getAxisXTitleColor( )
        const noexcept
    {
        return this->axisXTitleColor;
    }

    /**
     * Gets the font name list for the X axis title.
     *
     * @return
     *   The font names.
     */
    inline std::string getAxisXTitleFontNames( )
        const noexcept
    {
        return std::string( this->axisXTitleFontNames );
    }

    /**
     * Gets the font size for the X axis title.
     *
     * @return
     *   The font size.
     */
    inline int getAxisXTitleFontSize( )
        const noexcept
    {
        return this->axisXTitleFontSize;
    }

    /**
     * Gets the bottom margin for the X axis title.
     *
     * @return
     *   The bottom margin.
     */
    inline int getAxisXTitleMargin( )
        const noexcept
    {
        return this->axisXTitleMargin;
    }

    /**
     * Gets the Y axis title.
     *
     * @return
     *   The title.
     */
    inline const std::string getAxisYTitle( )
        const noexcept
    {
        return this->axisYTitle;
    }

    /**
     * Gets the color the Y axis title.
     *
     * @return
     *   The color.
     */
    inline int getAxisYTitleColor( )
        const noexcept
    {
        return this->axisYTitleColor;
    }

    /**
     * Gets the font name list for the Y axis title.
     *
     * @return
     *   The font names.
     */
    inline std::string getAxisYTitleFontNames( )
        const noexcept
    {
        return std::string( this->axisYTitleFontNames );
    }

    /**
     * Gets the font size for the Y axis title.
     *
     * @return
     *   The font size.
     */
    inline int getAxisYTitleFontSize( )
        const noexcept
    {
        return this->axisYTitleFontSize;
    }

    /**
     * Gets the left margin for the Y axis title.
     *
     * @return
     *   The bottom margin.
     */
    inline int getAxisYTitleMargin( )
        const noexcept
    {
        return this->axisYTitleMargin;
    }



    /**
     * Sets the color of the X and Y axis titles.
     *
     * @param[in] color
     *   The color.
     */
    inline void setAxisTitleColor( const int color )
        noexcept
    {
        this->axisXTitleColor = color;
        this->axisYTitleColor = color;
    }

    /**
     * Sets the X axis title.
     *
     * @param[in] title
     *   The title.
     */
    inline void setAxisXTitle( const char*const title )
        noexcept
    {
        // If the title hasn't changed, do nothing.
        if ( title == nullptr && this->axisXTitle == nullptr )
            return;
        if ( title != nullptr && this->axisXTitle != nullptr &&
             std::strcmp( title, this->axisXTitle ) == 0 )
            return;

        // The title has changed. Delete the current title.
        if ( this->axisXTitle != nullptr )
        {
            delete this->axisXTitle;
            this->axisXTitle = nullptr;
            this->axisXTitleWidth = this->axisXTitleHeight = 0;
        }

        // Set the title.
        if ( title != nullptr )
        {
            this->axisXTitle = new char[std::strlen( title ) + 1];
            std::strcpy( this->axisXTitle, title );
        }
    }

    /**
     * Sets the X axis title.
     *
     * @param[in] title
     *   The title.
     */
    inline void setAxisXTitle( const std::string title )
        noexcept
    {
        this->setAxisXTitle( title.c_str( ) );
    }

    /**
     * Sets the color of the X axis title.
     *
     * @param[in] color
     *   The color.
     */
    inline void setAxisXTitleColor( const int color )
        noexcept
    {
        this->axisXTitleColor = color;
    }

    /**
     * Sets the font name list for the X axis titles.
     *
     * @param[in] names
     *   The font names.
     */
    inline void setAxisXTitleFontNames( const char*const names )
        noexcept
    {
        // If the font names haven't changed, do nothing.
        if ( names == nullptr && this->axisXTitleFontNames == nullptr )
            return;
        if ( names != nullptr && this->axisXTitleFontNames != nullptr &&
             std::strcmp( names, this->axisXTitleFontNames ) == 0 )
            return;

        // The font names have changed. Delete the current names.
        if ( this->axisXTitleFontNames != nullptr )
        {
            delete this->axisXTitleFontNames;
            this->axisXTitleFontNames = nullptr;
            this->axisXTitleWidth = this->axisXTitleHeight = 0;
        }

        // Save the names.
        if ( names != nullptr )
        {
            this->axisXTitleFontNames = new char[std::strlen( names ) + 1];
            std::strcpy( this->axisXTitleFontNames, names );
        }
        else
        {
            const char*const def = this->getDefaultFonts( );
            this->axisXTitleFontNames = new char[std::strlen( def ) + 1];
            std::strcpy( this->axisXTitleFontNames, def );
        }
    }

    /**
     * Sets the font name list for the X axis title.
     *
     * @param[in] names
     *   The font names.
     */
    inline void setAxisXTitleFontNames( const std::string names )
        noexcept
    {
        this->setAxisXTitleFontNames( names.c_str( ) );
    }

    /**
     * Sets the font size for the X axis title.
     *
     * @param[in] size
     *   The font size.
     */
    inline void setAxisXTitleFontSize( const int size )
        noexcept
    {
        if ( size == this->axisXTitleFontSize )
            return;

        this->axisXTitleFontSize = size;
        this->axisXTitleWidth = this->axisXTitleHeight = 0;
    }

    /**
     * Sets the bottom margin for the X axis title.
     *
     * @param[in] margin
     *   The bottom margin.
     */
    inline void setAxisXTitleMargin( const int margin )
        noexcept
    {
        this->axisXTitleMargin = (unsigned short) margin;
    }

    /**
     * Sets the Y axis title.
     *
     * @param[in] title
     *   The title.
     */
    inline void setAxisYTitle( const char*const title )
        noexcept
    {
        // If the title hasn't changed, do nothing.
        if ( title == nullptr && this->axisXTitle == nullptr )
            return;
        if ( title != nullptr && this->axisXTitle != nullptr &&
             std::strcmp( title, this->axisXTitle ) == 0 )
            return;

        // The title has changed. Delete the current title.
        if ( this->axisYTitle != nullptr )
        {
            delete this->axisYTitle;
            this->axisYTitle = nullptr;
            this->axisYTitleWidth = this->axisYTitleHeight = 0;
        }

        // Set the title.
        if ( title != nullptr )
        {
            this->axisYTitle = new char[std::strlen( title ) + 1];
            std::strcpy( this->axisYTitle, title );
        }
    }

    /**
     * Sets the Y axis title.
     *
     * @param[in] title
     *   The title.
     */
    inline void setAxisYTitle( const std::string title )
        noexcept
    {
        this->setAxisYTitle( title.c_str( ) );
    }

    /**
     * Sets the color of the Y axis title.
     *
     * @param[in] color
     *   The color.
     */
    inline void setAxisYTitleColor( const int color )
        noexcept
    {
        this->axisYTitleColor = color;
    }

    /**
     * Sets the font name list for the Y axis titles.
     *
     * @param[in] names
     *   The font names.
     */
    inline void setAxisYTitleFontNames( const char*const names )
        noexcept
    {
        // If the font names haven't changed, do nothing.
        if ( names == nullptr && this->axisYTitleFontNames == nullptr )
            return;
        if ( names != nullptr && this->axisYTitleFontNames != nullptr &&
             std::strcmp( names, this->axisYTitleFontNames ) == 0 )
            return;

        // The font names have changed. Delete the current names.
        if ( this->axisYTitleFontNames != nullptr )
        {
            delete this->axisYTitleFontNames;
            this->axisYTitleFontNames = nullptr;
            this->axisYTitleWidth = this->axisYTitleHeight = 0;
        }

        // Save the names.
        if ( names != nullptr )
        {
            this->axisYTitleFontNames = new char[std::strlen( names ) + 1];
            std::strcpy( this->axisYTitleFontNames, names );
        }
        else
        {
            const char*const def = this->getDefaultFonts( );
            this->axisYTitleFontNames = new char[std::strlen( def ) + 1];
            std::strcpy( this->axisYTitleFontNames, def );
        }
    }

    /**
     * Sets the font name list for the Y axis title.
     *
     * @param[in] names
     *   The font names.
     */
    inline void setAxisYTitleFontNames( const std::string names )
        noexcept
    {
        this->setAxisYTitleFontNames( names.c_str( ) );
    }

    /**
     * Sets the font size for the Y axis title.
     *
     * @param[in] size
     *   The font size.
     */
    inline void setAxisYTitleFontSize( const int size )
        noexcept
    {
        if ( size == this->axisYTitleFontSize )
            return;

        this->axisYTitleFontSize = size;
        this->axisYTitleWidth = this->axisYTitleHeight = 0;
    }

    /**
     * Sets the left margin for the Y axis title.
     *
     * @param[in] margin
     *   The left margin.
     */
    inline void setAxisYTitleMargin( const int margin )
        noexcept
    {
        this->axisYTitleMargin = (unsigned short) margin;
    }
    // @}



//----------------------------------------------------------------------
//
// Axis tic attributes.
//
//----------------------------------------------------------------------
    /**
     * @name Axis tic mark attributes
     */
    // @{
public:
    /**
     * Gets the color of the X axis tics.
     *
     * @return
     *   The color.
     */
    inline int getAxisXTicColor( )
        const noexcept
    {
        return this->axisXTicColor;
    }

    /**
     * Gets the length of the X axis tics.
     *
     * @return
     *   The length.
     */
    inline unsigned int getAxisXTicLength( )
        const noexcept
    {
        return this->axisXTicLength;
    }

    /**
     * Gets the line width of the X axis tics.
     *
     * @return
     *   The width.
     */
    inline unsigned int getAxisXTicLineWidth( )
        const noexcept
    {
        return this->axisXTicLineWidth;
    }

    /**
     * Gets the positions of the X axis tics.
     *
     * @return
     *   A vector with axis coordinates for tics.
     */
    inline const std::vector<float> getAxisXTics( )
        const noexcept
    {
        return this->axisXTics;
    }

    /**
     * Gets the color of the Y axis tics.
     *
     * @return
     *   The color.
     */
    inline int getAxisYTicColor( )
        const noexcept
    {
        return this->axisYTicColor;
    }

    /**
     * Gets the length of the Y axis tics.
     *
     * @return
     *   The length.
     */
    inline unsigned int getAxisYTicLength( )
        const noexcept
    {
        return this->axisYTicLength;
    }

    /**
     * Gets the line width of the Y axis tics.
     *
     * @return
     *   The width.
     */
    inline unsigned int getAxisYTicLineWidth( )
        const noexcept
    {
        return this->axisYTicLineWidth;
    }

    /**
     * Gets the positions of the Y axis tics.
     *
     * @return
     *   A vector with axis coordinates for tics.
     */
    inline const std::vector<float> getAxisYTics( )
        const noexcept
    {
        return this->axisYTics;
    }



    /**
     * Sets the color of the X and Y axis tics.
     *
     * @param[in] color
     *   The color.
     */
    inline void setAxisTicColor( const int color )
        noexcept
    {
        this->axisXTicColor = color;
        this->axisYTicColor = color;
    }

    /**
     * Sets the length of the X and Y axis tics.
     *
     * The length is restricted to the range [0,65535].
     *
     * @param[in] length
     *   The length.
     */
    inline void setAxisTicLength( const unsigned int length )
        noexcept
    {
        this->axisXTicLength = (length > 65535) ? 65535 : length;
        this->axisYTicLength = (length > 65535) ? 65535 : length;
    }

    /**
     * Sets the line width of the X and Y axis tics.
     *
     * The line width is restricted to the range [0,255];
     *
     * @param[in] width
     *   The width.
     */
    inline void setAxisTicLineWidth( const unsigned int width )
        noexcept
    {
        this->axisXTicLineWidth = (width > 255) ? 255 : width;
        this->axisYTicLineWidth = (width > 255) ? 255 : width;
    }

    /**
     * Sets the color of the X axis tics.
     *
     * @param[in] color
     *   The color.
     */
    inline void setAxisXTicColor( const int color )
        noexcept
    {
        this->axisXTicColor = color;
    }

    /**
     * Sets the length of the X axis tics.
     *
     * The length is restricted to the range [0,65535].
     *
     * @param[in] length
     *   The length.
     */
    inline void setAxisXTicLength( const unsigned int length )
        noexcept
    {
        this->axisXTicLength = (length > 65535) ? 65535 : length;
    }

    /**
     * Sets the line width of the X axis tics.
     *
     * The line width is restricted to the range [0,255];
     *
     * @param[in] width
     *   The width.
     */
    inline void setAxisXTicLineWidth( const unsigned int width )
        noexcept
    {
        this->axisXTicLineWidth = (width > 255) ? 255 : width;
    }

    /**
     * Sets the positions of the X axis tics.
     *
     * @param[in] tics
     *   A vector with axis coordinates for tics.
     */
    inline void setAxisXTics( const std::vector<float> tics )
        noexcept
    {
        this->axisXTics.assign( tics.begin( ), tics.end( ) );
        this->axisXTics.shrink_to_fit( );

        // Reset the text widths and heights lists to zeroes.
        const size_t n = tics.size( );
        this->axisXNumberWidths.assign( n, 0 );
        this->axisXNumberHeights.assign( n, 0 );
        this->axisXNumberWidths.shrink_to_fit( );
        this->axisXNumberHeights.shrink_to_fit( );
    }

    /**
     * Sets the color of the Y axis tics.
     *
     * @param[in] color
     *   The color.
     */
    inline void setAxisYTicColor( const int color )
        noexcept
    {
        this->axisYTicColor = color;
    }

    /**
     * Sets the length of the Y axis tics.
     *
     * The length is restricted to the range [0,65535].
     *
     * @param[in] length
     *   The length.
     */
    inline void setAxisYTicLength( const unsigned int length )
        noexcept
    {
        this->axisYTicLength = (length > 65535) ? 65535 : length;
    }

    /**
     * Sets the line width of the Y axis tics.
     *
     * The line width is restricted to the range [0,255];
     *
     * @param[in] width
     *   The width.
     */
    inline void setAxisYTicLineWidth( const unsigned int width )
        noexcept
    {
        this->axisYTicLineWidth = (width > 255) ? 255 : width;
    }

    /**
     * Sets the positions of the Y axis tics.
     *
     * @param[in] tics
     *   A vector with axis coordinates for tics.
     */
    inline void setAxisYTics( const std::vector<float> tics )
        noexcept
    {
        this->axisYTics.assign( tics.begin( ), tics.end( ) );
        this->axisYTics.shrink_to_fit( );

        // Reset the text widths and heights lists to zeroes.
        const size_t n = tics.size( );
        this->axisYNumberWidths.assign( n, 0 );
        this->axisYNumberHeights.assign( n, 0 );
        this->axisYNumberWidths.shrink_to_fit( );
        this->axisYNumberHeights.shrink_to_fit( );
    }
    // @}



//----------------------------------------------------------------------
//
// Legend attributes.
//
//----------------------------------------------------------------------
    /**
     * @name Legend attributes
     */
    // @{
public:
    /**
     * Gets the legend colors.
     *
     * @return
     *   Returns a vector with each of the legend colors.
     *
     * @see setLegend()
     */
    inline std::vector<int> getLegendColors( )
        const noexcept
    {
        const size_t n = this->legendColors.size( );
        std::vector<int> colors( n );

        for ( size_t i = 0; i < n; ++i )
            colors.push_back( this->legendColors[i] );
        colors.shrink_to_fit( );

        return colors;
    }

    /**
     * Gets the legend values.
     *
     * @return
     *   Returns a vector with a copy of the legend string values.
     *
     * @see setLegend()
     */
    inline std::vector<std::string> getLegendValues( )
        const noexcept
    {
        const size_t n = this->legendValues.size( );
        std::vector<std::string> values( n );

        for ( size_t i = 0; i < n; ++i )
            values.push_back( this->legendValues[i] );
        values.shrink_to_fit( );

        return values;
    }

    /**
     * Gets the font name list for the legend.
     *
     * @return
     *   The font names.
     *
     * @see setLegendFontNames()
     */
    inline std::string getLegendFontNames( )
        const noexcept
    {
        return std::string( this->legendFontNames );
    }

    /**
     * Gets the font size for the legend.
     *
     * @return
     *   The font size.
     *
     * @see setLegendFontSizes()
     */
    inline int getLegendFontSize( )
        const noexcept
    {
        return this->legendFontSize;
    }



    /**
     * Clears the legend of all values and colors.
     *
     * @see setLegend()
     */
    inline void clearLegend( )
        noexcept
    {
        size_t n = this->legendValues.size();
        if ( n != 0 )
        {
            for ( size_t i = 0; i < n; ++i )
            {
                char* s = this->legendValues[i];
                if ( s != nullptr )
                    delete s;
            }

            this->legendValues.clear( );
            this->legendColors.clear( );
            this->legendWidths.clear( );
            this->legendHeights.clear( );
        }
    }

    /**
     * Sets the legend values and colors.
     *
     * @param[in] values
     *   The list of legend strings.
     * @param[in] colors
     *   The list of corresponding legend colors.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the values and colors lists are not the
     *   same size. There must be one color for each value.
     *
     * @see clearLegend()
     * @see getLegendValues()
     * @see getLegendColors()
     */
    inline void setLegend(
        const std::vector<std::string> values,
        const std::vector<int> colors )
    {
        if ( values.size() != colors.size() )
            throw std::invalid_argument(
                "The size of the legend values and colors lists must be the same." );

        this->clearLegend( );

        const size_t n = values.size();
        this->legendValues.reserve( n );
        this->legendColors.reserve( n );
        for ( size_t i = 0; i < n; ++i )
        {
            const char*const s = values[i].c_str();
            char*const ss = new char[std::strlen( s ) + 1];
            std::strcpy( ss, s );
            this->legendValues.push_back( ss );
            this->legendColors.push_back( colors[i] );
        }

        this->legendWidths.assign( n, 0 );
        this->legendHeights.assign( n, 0 );

        this->legendValues.shrink_to_fit();
        this->legendColors.shrink_to_fit();
        this->legendWidths.shrink_to_fit( );
        this->legendHeights.shrink_to_fit( );
    }

    /**
     * Sets the legend values and colors.
     *
     * @param[in] values
     *   The list of legend strings.
     * @param[in] colors
     *   The list of corresponding legend colors.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the values and colors lists are not the
     *   same size. There must be one color for each value.
     *
     * @see getLegendValues()
     * @see getLegendColors()
     */
    inline void setLegend(
        const std::vector<char*> values,
        const std::vector<int> colors )
    {
        if ( values.size() != colors.size() )
            throw std::invalid_argument(
                "The size of the legend values and colors lists must be the same." );

        size_t n = this->legendValues.size();
        if ( n != 0 )
        {
            for ( size_t i = 0; i < n; ++i )
            {
                char* s = this->legendValues[i];
                if ( s != nullptr )
                    delete s;
            }
            this->legendValues.clear();
            this->legendColors.clear();
        }

        n = values.size();
        this->legendValues.reserve( n );
        this->legendColors.reserve( n );
        for ( size_t i = 0; i < n; ++i )
        {
            const char*const s = values[i];
            char*const ss = new char[std::strlen( s ) + 1];
            std::strcpy( ss, s );
            this->legendValues.push_back( ss );
            this->legendColors.push_back( colors[i] );
        }

        this->legendWidths.assign( n, 0 );
        this->legendHeights.assign( n, 0 );

        this->legendValues.shrink_to_fit();
        this->legendColors.shrink_to_fit();
        this->legendWidths.shrink_to_fit( );
        this->legendHeights.shrink_to_fit( );
    }

    /**
     * Sets the font name list for the legend.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getLegendFontNames()
     */
    inline void setLegendFontNames( const char*const names )
        noexcept
    {
        // If the font names haven't changed, do nothing.
        if ( names == nullptr && this->legendFontNames == nullptr )
            return;
        if ( names != nullptr && this->legendFontNames != nullptr &&
             std::strcmp( names, this->legendFontNames ) == 0 )
            return;

        // The font names have changed. Delete the current names.
        if ( this->legendFontNames != nullptr )
        {
            delete this->legendFontNames;
            this->legendFontNames = nullptr;

            // Reset the text widths and heights lists to zeroes.
            const size_t n = legendValues.size( );
            this->legendWidths.assign( n, 0 );
            this->legendHeights.assign( n, 0 );
            this->legendWidths.shrink_to_fit( );
            this->legendHeights.shrink_to_fit( );
        }

        // Save the names.
        if ( names != nullptr )
        {
            this->legendFontNames = new char[std::strlen( names ) + 1];
            std::strcpy( this->legendFontNames, names );
        }
        else
        {
            const char*const def = this->getDefaultFonts( );
            this->legendFontNames = new char[std::strlen( def ) + 1];
            std::strcpy( this->legendFontNames, def );
        }
    }

    /**
     * Sets the font name list for the legend.
     *
     * @param[in] names
     *   The font names.
     *
     * @see getLegendFontNames()
     */
    inline void setLegendFontNames( const std::string names )
        noexcept
    {
        this->setLegendFontNames( names.c_str( ) );
    }

    /**
     * Sets the font size for the legend.
     *
     * @param[in] size
     *   The font size.
     *
     * @see getLegendFontSizes()
     */
    inline void setLegendFontSize( const int size )
        noexcept
    {
        if ( size == this->legendFontSize )
            return;

        this->legendFontSize = size;

        // Reset the text widths and heights lists to zeroes.
        const size_t n = legendValues.size( );
        this->legendWidths.assign( n, 0 );
        this->legendHeights.assign( n, 0 );
        this->legendWidths.shrink_to_fit( );
        this->legendHeights.shrink_to_fit( );
    }
    // @}



//----------------------------------------------------------------------
//
// Plot area attributes.
//
//----------------------------------------------------------------------
    /**
     * @name Plot area attributes
     */
    // @{
public:
    /**
     * Gets the width of the plot's bottom margin.
     *
     * @return
     *   The margin.
     */
    inline unsigned int getPlotBottomMargin( )
        const noexcept
    {
        return this->plotYRange[0];
    }

    /**
     * Gets the width of the plot's left margin.
     *
     * @return
     *   The margin.
     */
    inline unsigned int getPlotLeftMargin( )
        const noexcept
    {
        return this->plotXRange[0];
    }

    /**
     * Gets the width of the plot's right margin.
     *
     * @return
     *   The margin.
     */
    inline unsigned int getPlotRightMargin( )
        const noexcept
    {
        return this->imageWidth - this->plotXRange[1];
    }

    /**
     * Gets the width of the plot's top margin.
     *
     * @return
     *   The margin.
     */
    inline unsigned int getPlotTopMargin( )
        const noexcept
    {
        return this->imageHeight - this->plotYRange[1];
    }

    /**
     * Gets the plot area background color.
     *
     * @return
     *   The color.
     */
    inline int getPlotBackgroundColor( )
        const noexcept
    {
        return this->plotBackgroundColor;
    }

    /**
     * Gets the plot area border color.
     *
     * @return
     *   The color.
     */
    inline int getPlotBorderColor( )
        const noexcept
    {
        return this->plotBorderColor;
    }

    /*
     * Gets the plot area's border width.
     *
     * @return
     *   The border width.
     */
    inline unsigned int getPlotBorderLineWidth( )
        const noexcept
    {
        return this->plotBorderLineWidth;
    }

    /**
     * Gets the plot area's left and right margins.
     *
     * Margins are limited to the range [0,65535] (an unsigned short int).
     *
     * @param[out] leftMargin
     *   The returned left margin.
     * @param[out] rightMargin
     *   The returned right margin.
     */
    inline void getPlotXMargins(
        unsigned int &leftMargin,
        unsigned int &rightMargin )
        const noexcept
    {
        leftMargin = this->plotXRange[0];
        rightMargin = this->imageWidth - this->plotXRange[1];
    }

    /**
     * Gets the plot area's bottom and top margins.
     *
     * Margins are limited to the range [0,65535] (an unsigned short int).
     *
     * @param[out] bottomMargin
     *   The returned bottom margin.
     * @param[out] topMargin
     *   The returned top margin.
     */
    inline void getPlotYMargins(
        unsigned int &bottomMargin,
        unsigned int &topMargin )
        const noexcept
    {
        bottomMargin = this->plotYRange[0];
        topMargin = this->imageHeight - this->plotYRange[1];
    }



    /**
     * Sets the plot area's left and right margins.
     *
     * Margins are limited to the range [0,65535] (an unsigned short int).
     *
     * @param[in] leftMargin
     *   The left margin.
     * @param[in] rightMargin
     *   The right margin.
     *
     * @throws std::invalid_argument
     *   Throws an exception if (leftMargin + rightMargin) >= image width.
     */
    inline void setPlotXMargins(
        const unsigned int leftMargin,
        const unsigned int rightMargin )
    {
        if ( (leftMargin + rightMargin) >= this->imageWidth )
            throw std::invalid_argument(
                "Left and right margins are together too large for the image." );
        this->plotXRange[0] = (unsigned short)leftMargin;
        this->plotXRange[1] = (unsigned short)(this->imageWidth - rightMargin);
        this->plotWidth    = (this->plotXRange[1] - this->plotXRange[0]);
    }

    /**
     * Sets the plot area's bottom and top margins.
     *
     * Margins are limited to the range [0,65535] (an unsigned short int).
     *
     * @param[in] bottomMargin
     *   The bottom margin.
     * @param[in] topMargin
     *   The top margin.
     *
     * @throws std::invalid_argument
     *   Throws an exception if (bottomMargin + topMargin) >= image height.
     */
    inline void setPlotYMargins(
        const unsigned int bottomMargin,
        const unsigned int topMargin )
    {
        if ( (bottomMargin + topMargin) >= this->imageHeight )
            throw std::invalid_argument(
                "Bottom and top margins are together too large for the image." );
        this->plotYRange[0] = (unsigned short)bottomMargin;
        this->plotYRange[1] = (unsigned short)(this->imageHeight - topMargin);
        this->plotHeight    = (this->plotYRange[1] - this->plotYRange[0]);
    }

    /**
     * Sets the plot background color.
     *
     * @param[in] color
     *   The color.
     */
    inline void setPlotBackgroundColor( const int color )
        noexcept
    {
        this->plotBackgroundColor = color;
    }

    /**
     * Sets the plot border color.
     *
     * @param[in] color
     *   The color.
     */
    inline void setPlotBorderColor( const int color )
        noexcept
    {
        this->plotBorderColor = color;
    }

    /**
     * Sets the plot border line width.
     *
     * The line width is restricted to the range [0,255].
     *
     * @param[in] width
     *   The border width.
     */
    inline void setPlotBorderLineWidth( const unsigned int width )
        noexcept
    {
        this->plotBorderLineWidth = (width > 255) ? 255 : width;
    }
    // @}



//----------------------------------------------------------------------
//
// Dot attributes
//
//----------------------------------------------------------------------
    /**
     * @name Dot attributes
     */
    // @{
public:
    /**
     * Gets the dot size.
     *
     * @return
     *   The dot size.
     *
     * @see setDotSize()
     */
    inline unsigned int getDotSize( )
        const noexcept
    {
        return this->dotSize;
    }

    /**
     * Returns true if the dot shape is set to be a circle.
     *
     * @return
     *   Returns true if dots are circles.
     *
     * @see isDotSquare()
     * @see setDotCircle()
     */
    inline bool isDotCircle( )
        const noexcept
    {
        return this->dotShape == DOT_CIRCLE;
    }

    /**
     * Returns true if the dot shape is set to be a square.
     *
     * @return
     *   Returns true if dots are squares.
     *
     * @see isDotCircle()
     * @see setDotSquare()
     */
    inline bool isDotSquare( )
        const noexcept
    {
        return this->dotShape == DOT_SQUARE;
    }



    /**
     * Sets the dot shape to be a circle.
     *
     * @see isDotCircle()
     */
    inline void setDotCircle( )
        noexcept
    {
        this->dotShape = DOT_CIRCLE;
    }

    /**
     * Sets the dot size.
     *
     * Dot sizes are limited to the range [0,255].
     *
     * @param[in] size
     *   The dot size.
     *
     * @see getDotSize()
     */
    inline void setDotSize( const unsigned int size )
        noexcept
    {
        this->dotSize = (size > 255) ? 255 : size;
    }

    /**
     * Sets the dot shape to be a square.
     *
     * @see isDotSquare()
     */
    inline void setDotSquare( )
        noexcept
    {
        this->dotShape = DOT_SQUARE;
    }
    // @}



//----------------------------------------------------------------------
//
// Plot utilities.
//
//----------------------------------------------------------------------
    /**
     * @name Utilities
     */
    // @{
private:
    /**
     * Converts an axis X coordinate to an image-space X coordinate.
     *
     * @param[in] axisX
     *   The X coordinate to convert.
     * @param[in] boundsHandling
     *   (optional, default = BOUNDS_REJECT) Select how to handle
     *   coordinates that are out of bounds:
     *
     *     - NO_BOUNDS_CHECK: Convert to image space and do not check bounds.
     *       The returned value could be out of bounds.
     *
     *     - BOUNDS_REJECT: Convert to image space, but return -1 if out of
     *       bounds.
     *
     *     - BOUNDS_CROP: Convert to image space, cropping the returned
     *       value to the edge of the image.
     *
     * @return
     *   Returns the converted coordinate, or -1 if the coordinate is
     *   outside the axis range.
     */
    inline int convertAxisXToImageX(
        const float axisX,
        const unsigned char boundsHandling = BOUNDS_REJECT )
        const noexcept
    {
        float plotAreaPercent =
            (axisX - this->axisXRange[0]) / this->axisXDelta;

        switch ( boundsHandling )
        {
            case NO_BOUNDS_CHECK:
                break;

            default:
            case BOUNDS_REJECT:
                if ( plotAreaPercent < 0.0 || plotAreaPercent > 1.0 )
                    return -1;
                break;

            case BOUNDS_CROP:
                if ( plotAreaPercent < 0.0 )
                    plotAreaPercent = 0.0;
                else if ( plotAreaPercent > 1.0 )
                    plotAreaPercent = 1.0;
                break;
        }

        return (int)(this->plotXRange[0] +
            (plotAreaPercent * (float)this->plotWidth));
    }

    /**
     * Converts an axis Y coordinate to an image-space Y coordinate.
     *
     * @param[in] axisY
     *   The Y coordinate to convert.
     * @param[in] boundsHandling
     *   (optional, default = BOUNDS_REJECT) Select how to handle
     *   coordinates that are out of bounds:
     *
     *     - NO_BOUNDS_CHECK: Convert to image space and do not check bounds.
     *       The returned value could be out of bounds.
     *
     *     - BOUNDS_REJECT: Convert to image space, but return -1 if out of
     *       bounds.
     *
     *     - BOUNDS_CROP: Convert to image space, cropping the returned
     *       value to the edge of the image.
     *
     * @return
     *   Returns the converted coordinate, or -1 if the coordinate is
     *   outside the axis range.
     */
    inline int convertAxisYToImageY(
        const float axisY,
        const unsigned char boundsHandling = BOUNDS_REJECT )
        const noexcept
    {
        float plotAreaPercent =
            (axisY - this->axisYRange[0]) / this->axisYDelta;

        switch ( boundsHandling )
        {
            case NO_BOUNDS_CHECK:
                break;

            default:
            case BOUNDS_REJECT:
                if ( plotAreaPercent < 0.0 || plotAreaPercent > 1.0 )
                    return -1;
                break;

            case BOUNDS_CROP:
                if ( plotAreaPercent < 0.0 )
                    plotAreaPercent = 0.0;
                else if ( plotAreaPercent > 1.0 )
                    plotAreaPercent = 1.0;
                break;
        }

        return (int)(this->plotYRange[0] +
            (plotAreaPercent * (float)this->plotHeight));
    }

    /**
     * Computes the text width and height for a given text, font, etc.
     *
     * @param[in] text
     *   The text to try.
     * @param[in] fontNames
     *   The font(s) to use.
     * @param[in] fontSize
     *   The font size.
     * @param[in] rotate
     *   The rotation angle, in radians.
     * @param[out] textWidth
     *   The returned text width.
     * @param[out] textHeight
     *   The returned text height.
     */
    inline void computeTextBounds(
        char*const text,
        char*const fontNames,
        const int fontSize,
        const double rotate,
        int& textWidth,
        int& textHeight )
    {
        int bounds[8];
        gdImageStringFT( nullptr,
            bounds,
            this->black,
            fontNames,
            (double)fontSize,
            0.0,
            0, 0,
            text );

        textWidth  = bounds[2] - bounds[0];
        textHeight = bounds[1] - bounds[5];
    }
    // @}



//----------------------------------------------------------------------
//
// Plot area drawing.
//
//----------------------------------------------------------------------
    /**
     * @name Plot areadrawing
     */
    // @{
public:
    /**
     * Fills the plot area with its background.
     *
     * The plot area is cleared to the background color. Other areas
     * of the image, including the plot decoration, are not changed.
     *
     * @see clear()
     * @see clearAndClearPlotArea()
     */
    inline void clearPlotArea( )
        noexcept
    {
        const unsigned int y0Flip = this->imageHeight - this->plotYRange[0];
        const unsigned int y1Flip = this->imageHeight - this->plotYRange[1];
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[0],
            y0Flip,
            this->plotXRange[1],
            y1Flip,
            this->plotBackgroundColor );
    }

    /**
     * Clears the image and fills the plot area with its background.
     *
     * The entire image is cleared to the background color, and the
     * plot area cleared to its background.
     *
     * This is equivalent to calling clear() then clearPlotArea(),
     * but faster because it skips redundantly filling the plot area
     * first with the image background, and then with the plot area
     * background.
     *
     * @see clear()
     * @see clearPlotArea()
     */
    inline void clearAndClearPlotArea( )
        noexcept
    {
        // The image can be divided into five rectangular regions:
        // - The plot area.
        // - The left margin.
        // - The right margin.
        // - The bottom margin.
        // - The top margin.
        //
        // Below we clear each of these separately.
        //
        // For all of these, recall that the public interface to this class
        // defines that pixel (0,0) is in the lower left corner. libgd instead
        // defines it as the upper left corner, so we have to flip all Y
        // coordinates.
        //
        // Clear the plot area.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[0], (this->imageHeight - this->plotYRange[0]),
            this->plotXRange[1], (this->imageHeight - this->plotYRange[1]),
            this->plotBackgroundColor );

        // Clear the left margin, from image top to image bottom.
        gdImageFilledRectangle(
            this->image,
            0, 0,
            this->plotXRange[0], this->imageHeight,
            this->backgroundColor );

        // Clear the right margin, from image top to image bottom.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[1], 0,
            this->imageWidth, this->imageHeight,
            this->backgroundColor );

        // Clear the top margin, directly above the plot area.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[0], 0,
            this->plotXRange[1], (this->imageHeight - this->plotYRange[1]),
            this->backgroundColor );

        // Clear the bottom margin, directly above the plot area.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[0], (this->imageHeight - this->plotYRange[0]),
            this->plotXRange[1], this->imageHeight,
            this->backgroundColor );
    }

    /**
     * Clears the image margins, outside of the plot area.
     *
     * The area outside of the plot area is cleared to its background.
     *
     * @see clear()
     * @see clearPlotArea()
     * @see clearAndClearPlotArea()
     */
    inline void clearMargins( )
        noexcept
    {
        // The image can be divided into five rectangular regions:
        // - The plot area.
        // - The left margin.
        // - The right margin.
        // - The bottom margin.
        // - The top margin.
        //
        // Below we clear the margins, but NOT the plot area.
        //
        // For all of these, recall that the public interface to this class
        // defines that pixel (0,0) is in the lower left corner. libgd instead
        // defines it as the upper left corner, so we have to flip all Y
        // coordinates.
        //
        // Clear the left margin, from image top to image bottom.
        gdImageFilledRectangle(
            this->image,
            0, 0,
            this->plotXRange[0], this->imageHeight,
            this->backgroundColor );

        // Clear the right margin, from image top to image bottom.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[1], 0,
            this->imageWidth, this->imageHeight,
            this->backgroundColor );

        // Clear the top margin, directly above the plot area.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[0], 0,
            this->plotXRange[1], (this->imageHeight - this->plotYRange[1]),
            this->backgroundColor );

        // Clear the bottom margin, directly above the plot area.
        gdImageFilledRectangle(
            this->image,
            this->plotXRange[0], (this->imageHeight - this->plotYRange[0]),
            this->plotXRange[1], this->imageHeight,
            this->backgroundColor );
    }

    /**
     * Returns an estimate of the number of X axis number digits after
     * the decimal point.
     */
    inline int estimateAxisXDigitsAfterDecimal( )
        const noexcept
    {
        // Get the number of integer digits needed for the biggest
        // X axis number.
        int xNumberOfIntegerDigits = 0;
        int nTics = (int) this->axisXTics.size( );
        for ( int i = 0; i < nTics; ++i )
        {
            // Get the next tick mark.
            const float number = this->axisXTics[i];

            // Count the number of integer digits required.
            int nDigits = 0;
            long tmp = (long) number;
            if ( tmp < 0 )
                tmp *= -1;
            while ( tmp > 0 )
            {
                tmp /= 10;
                ++nDigits;
            }

            if ( nDigits > xNumberOfIntegerDigits )
                xNumberOfIntegerDigits = nDigits;
        }

        // Compute an approximate maximum number of digits that will fit
        // within the space between adjacent X tics.
        const int xMaxDigits = this->plotWidth /
            (int) (this->axisXTics.size( ) - 1) /
            this->axisXNumberFontSize;

        // Determine the number of digits to show after the decimal.
        int xDigitsAfterDecimal = 0;
        if ( xMaxDigits <= xNumberOfIntegerDigits )
        {
            // The number of digits needed to show the tic numbers is
            // equal or more than the amount of space available. The integer
            // parts won't fit without overlap. Definitely don't show digits
            // after the decimal.
            xDigitsAfterDecimal = 0;
        }
        else if ( xNumberOfIntegerDigits == 0 )
        {
            // The tic numbers don't need any integer digits. Allow the
            // full available space, minus two for a leading "0.".
            xDigitsAfterDecimal = xMaxDigits - 2;
        }
        else
        {
            // The tic numbers need a few integer digits. Allocate the
            // rest of available space to digits after the decimal point.
            // Reserve 1 digit for ".".
            xDigitsAfterDecimal = (xMaxDigits - xNumberOfIntegerDigits) - 1;
        }

        if ( xDigitsAfterDecimal > this->axisXMaxDigitsAfterDecimal )
            return this->axisXMaxDigitsAfterDecimal;
        return xDigitsAfterDecimal;
    }

    /**
     * Returns an estimate of the number of Y axis number digits after
     * the decimal point.
     */
    inline int estimateAxisYDigitsAfterDecimal( )
        const noexcept
    {
        // Get the number of integer digits needed for the biggest
        // Y axis number.
        int yNumberOfIntegerDigits = 0;
        int nTics = (int) this->axisYTics.size( );
        for ( int i = 0; i < nTics; ++i )
        {
            // Get the next tick mark.
            const float number = this->axisYTics[i];

            // Count the number of integer digits required.
            int nDigits = 0;
            long tmp = (long) number;
            if ( tmp < 0 )
                tmp *= -1;
            while ( tmp > 0 )
            {
                tmp /= 10;
                ++nDigits;
            }

            if ( nDigits > yNumberOfIntegerDigits )
                yNumberOfIntegerDigits = nDigits;
        }

        // Since the Y axis numbers are horizontal, the space they have
        // available is based on the left margin size. Compute an approximate
        // number of digits that will fit there.
        const int yMaxDigits = this->plotXRange[0] / this->axisYNumberFontSize;

        // Compute an approximate maximum number of digits that will fit
        // within the margin.
        int yDigitsAfterDecimal = 0;
        if ( yMaxDigits <= yNumberOfIntegerDigits )
        {
            // The number of digits needed to show the numbers is bigger than
            // the left plot margin. The numbers will be cropped. Definitely
            // don't show digits after the decimal.
            yDigitsAfterDecimal = 0;
        }
        else if ( yNumberOfIntegerDigits == 0 )
        {
            // The tic numbers don't need any integer digits. Allow the
            // full available space, minus two for a leading "0.".
            yDigitsAfterDecimal = yMaxDigits - 2;
        }
        else
        {
            // The tic numbers need a few integer digits. Allocate the
            // rest of available space to digits after the decimal point.
            // Reserve 1 digit for ".".
            yDigitsAfterDecimal = (yMaxDigits - yNumberOfIntegerDigits) - 1;
        }

        if ( yDigitsAfterDecimal > this->axisYMaxDigitsAfterDecimal )
            return this->axisYMaxDigitsAfterDecimal;
        return yDigitsAfterDecimal;
    }

    /**
     * Draws the plot area's decoration, including border, tics, & labels.
     *
     * The decoration outside of the plot area is drawn. This includes
     * a border around the plot area, tick marks, tick mark labels,
     * the axis titles, and the legend.
     *
     * @see drawAxisXTics()
     * @see drawAxisXTitles()
     * @see drawAxisYTics()
     * @see drawAxisYTitles()
     * @see drawLegend()
     * @see drawPlotAreaBorder()
     * @see drawTitle()
     */
    inline void drawDecoration( )
        noexcept
    {
        this->drawPlotAreaBorder( );
        this->drawAxisXTicsAndNumbers( this->estimateAxisXDigitsAfterDecimal( ) );
        this->drawAxisYTicsAndNumbers( this->estimateAxisYDigitsAfterDecimal( ) );
        this->drawAxisXTitles( );
        this->drawAxisYTitles( );
        this->drawTitle( );
        this->drawLegend( );
    }

    /**
     * Draws the plot area border.
     *
     * The border is drawn using the current attributes:
     * - Plot area dimensions (left, right, bottom, and top margins).
     * - Plot area border color.
     * - Plot area border line width.
     *
     * @see getPlotXMargins()
     * @see getPlotYMargins()
     * @see getPlotBorderColor()
     * @see getPlotBorderLineWidth()
     */
    inline void drawPlotAreaBorder( )
        noexcept
    {
        // Get the (x,y) pair for opposite corners, in image space, and
        // accounting for margins. Flip Y for libgd.
        const int xStartImage = this->plotXRange[0];
        const int yStartImage = this->imageHeight - this->plotYRange[0];
        const int xEndImage   = this->plotXRange[1];
        const int yEndImage   = this->imageHeight - this->plotYRange[1];

        gdImageSetThickness( this->image, this->plotBorderLineWidth );

        // Swap start/end in Y because libgd requires the rectangle
        // corners be upper left and lower right.
        gdImageRectangle(
            this->image,
            xStartImage, yEndImage,
            xEndImage, yStartImage,
            this->plotBorderColor );
    }

    /**
     * Draws the X axis tics and numbers.
     *
     * The X axis tics are drawn using current attributes:
     * - Plot area dimensions.
     * - X axis tic mark locations.
     * - X axis tic lengths.
     * - X axis tic line widths.
     * - X axis tic colors.
     *
     * @param[in] digitsAfterDecimal
     *   (optional, default = 1) The number of digits after the decimal.
     *
     * @see getAxisXRange()
     * @see getAxisXTics()
     * @see getAxisXTicColor()
     * @see getAxisXTicLength()
     * @see getAxisXTicLineWidth()
     * @see getAxisXNumberColor()
     * @see getAxisXNumberFontNames()
     * @see getAxisXNumberFontSize()
     */
    inline void drawAxisXTicsAndNumbers( const int digitsAfterDecimal = 1 )
        noexcept
    {
        // If no tics have been defined, do nothing.
        if (this->axisXTics.empty( ) == true )
            return;

        // Tics along the X axis are all vertical and have the same length.
        // They all therefore share the same Y start and end points.
        //
        // Remember to flip Y for libgd.
        const int yStart = this->imageHeight - this->plotYRange[0];
        const int yEnd   = yStart - this->axisXTicLength;

        // Set the line thickness.
        gdImageSetThickness( this->image, this->axisXTicLineWidth );

        // Get a few values.
        // - Cast vector size to int because it is highly unlikely that
        //   we need a 64-bit integer's worth of tics.
        const int c = this->axisXTicColor;
        const int n = (int) this->axisXTics.size( );

        char numberBuffer[21];
        int textWidth, textHeight;
        int bounds[8];

        const char*const oldLocale = setlocale( LC_NUMERIC, NULL );
        if ( oldLocale[0] != '\0' )
            setlocale( LC_NUMERIC, "" );

        for ( int i = 0; i < n; ++i )
        {
            // Get the X plot space location and convert to image space.
            const float number = this->axisXTics[i];
            const int x = this->convertAxisXToImageX( number, BOUNDS_REJECT );

            // Skip it if out of bounds.
            if ( x < 0 )
                continue;

            // Draw the tic mark.
            gdImageLine(
                this->image,
                x, yStart,
                x, yEnd,
                c );

            // Create number label text.
            sprintf( numberBuffer, "%'.*f", digitsAfterDecimal, number );

            // If there are no cached text bounds, compute them.
            textWidth  = this->axisXNumberWidths[i];
            textHeight = this->axisXNumberHeights[i];
            if ( textWidth == 0 )
            {
                this->computeTextBounds(
                    numberBuffer,
                    this->axisXNumberFontNames,
                    this->axisXNumberFontSize,
                    0.0,
                    textWidth, textHeight );
                this->axisXNumberWidths[i] = textWidth;
                this->axisXNumberHeights[i] = textHeight;
            }

            const unsigned int xLeft = x - textWidth / 2;
            const unsigned int yBottom = yStart + textHeight +
                AXIS_NUMBER_SPACING_FROM_AXIS;

            gdImageStringFT( this->image,
                bounds,
                this->axisXNumberColor,
                this->axisXNumberFontNames,
                (double)this->axisXNumberFontSize,
                0.0,
                xLeft, yBottom,
                numberBuffer );
        }

        if ( oldLocale[0] != '\0' )
            setlocale( LC_NUMERIC, oldLocale );
    }

    /**
     * Draws the Y axis tics and numbers.
     *
     * The Y axis tics are drawn using current attributes:
     * - Plot area dimensions.
     * - Y axis tic mark locations.
     * - Y axis tic lengths.
     * - Y axis tic line widths.
     * - Y axis tic colors.
     *
     * @param[in] digitsAfterDecimal
     *   (optional, default = 1) The number of digits after the decimal.
     *
     * @see getAxisYRange()
     * @see getAxisYTics()
     * @see getAxisYTicColor()
     * @see getAxisYTicLength()
     * @see getAxisYTicLineWidth()
     * @see getAxisYNumberColor()
     * @see getAxisYNumberFontNames()
     * @see getAxisYNumberFontSize()
     */
    inline void drawAxisYTicsAndNumbers( const int digitsAfterDecimal = 1 )
        noexcept
    {
        // If no tics have been defined, do nothing.
        if (this->axisYTics.empty( ) == true )
            return;

        // Tics along the Y axis are all horizontal and have the same length.
        // They all therefore share the same X start and end points.
        const int xStart = this->plotXRange[0];
        const int xEnd   = xStart + this->axisYTicLength;

        // Set the line thickness.
        gdImageSetThickness( this->image, this->axisYTicLineWidth );

        // Get a few values.
        // - Cast vector size to int because it is highly unlikely that
        //   we need a 64-bit integer's worth of tics.
        const int c = this->axisYTicColor;
        const int n = (int) this->axisYTics.size( );

        char numberBuffer[21];
        int textWidth, textHeight;
        int bounds[8];

        const char*const oldLocale = setlocale( LC_NUMERIC, NULL );
        if ( oldLocale[0] != '\0' )
            setlocale( LC_NUMERIC, "" );

        for ( int i = 0; i < n; ++i )
        {
            // Get the Y plot space location and convert to image
            // space (not yet flipped). Ignore the tic if out of bounds.
            // Flip for libgd and draw the tic.
            const float number = this->axisYTics[i];
            const int y = this->convertAxisYToImageY( number, BOUNDS_REJECT );

            // Skip it if out of bounds.
            if ( y < 0 )
                continue;

            // Draw the tic mark.
            gdImageLine(
                this->image,
                xStart, this->imageHeight - y,
                xEnd, this->imageHeight - y,
                c );

            // Create number label text.
            sprintf( numberBuffer, "%'.*f", digitsAfterDecimal, number );

            // If there are no cached text bounds, compute them.
            textWidth  = this->axisYNumberWidths[i];
            textHeight = this->axisYNumberHeights[i];
            if ( textWidth == 0 )
            {
                this->computeTextBounds(
                    numberBuffer,
                    this->axisYNumberFontNames,
                    this->axisYNumberFontSize,
                    0.0,
                    textWidth, textHeight );
            }

            const unsigned int xLeft = xStart - textWidth -
                AXIS_NUMBER_SPACING_FROM_AXIS;
            const unsigned int yBottom = this->imageHeight -
                (y - textHeight / 2);

            gdImageStringFT( this->image,
                bounds,
                this->axisYNumberColor,
                this->axisYNumberFontNames,
                (double)this->axisYNumberFontSize,
                0.0,
                xLeft, yBottom,
                numberBuffer );
        }

        if ( oldLocale[0] != '\0' )
            setlocale( LC_NUMERIC, oldLocale );
    }

    /**
     * Draws the X axis title.
     *
     * The X axis title is drawn using current attributes:
     * - X axis title.
     * - X axis title color.
     * - X axis title font names.
     * - X axis title font size.
     *
     * @see getAxisXTitle()
     * @see getAxisXTitleColor()
     * @see getAxisXTitleFontNames()
     * @see getAxisXTitleFontSize()
     */
    inline void drawAxisXTitles( )
        noexcept
    {
        // If there is no title, do nothing.
        if ( this->axisXTitle == nullptr || this->axisXTitle[0] == '\0' )
            return;

        // If there are no cached text bounds, compute them.
        if ( this->axisXTitleWidth == 0 )
        {
            this->computeTextBounds(
                this->axisXTitle,
                this->axisXTitleFontNames,
                this->axisXTitleFontSize,
                0.0,
                this->axisXTitleWidth, this->axisXTitleHeight );
        }

        // Draw the title, centering it below the plot area and within
        // the lower margin.
        const unsigned int xLeft =
            (this->plotXRange[0] + this->plotWidth/2) -
            this->axisXTitleWidth / 2;
        const unsigned int yBottom =
            this->imageHeight - this->axisXTitleMargin;

        int bounds[8];
        gdImageStringFT( this->image,
            bounds,
            this->axisXTitleColor,
            this->axisXTitleFontNames,
            (double)this->axisXTitleFontSize,
            0.0,
            xLeft, yBottom,
            this->axisXTitle );
    }

    /**
     * Draws the Y axis title.
     *
     * The Y axis title is drawn using current attributes:
     * - Y axis title.
     * - Y axis title color.
     * - Y axis title font names.
     * - Y axis title font size.
     *
     * @see getAxisXTitle()
     * @see getAxisXTitleColor()
     * @see getAxisXTitleFontNames()
     * @see getAxisXTitleFontSize()
     */
    inline void drawAxisYTitles( )
        noexcept
    {
        // If there is no title, do nothing.
        if ( this->axisYTitle == nullptr || this->axisYTitle[0] == '\0' )
            return;

        // If there are no cached text bounds, compute them.
        if ( this->axisYTitleWidth == 0 )
        {
            this->computeTextBounds(
                this->axisYTitle,
                this->axisYTitleFontNames,
                this->axisYTitleFontSize,
                (3.1415927 / 2.0),
                this->axisYTitleWidth, this->axisYTitleHeight );
        }

        // Draw the title, centering it to the left of the plot area and
        // within the left margin.
        const unsigned int xLeft =
            this->axisYTitleMargin + this->axisYTitleHeight;
        const unsigned int yBottom =
            this->imageHeight - this->plotYRange[0] -
            this->plotHeight / 2 +
            this->axisYTitleWidth / 2;

        int bounds[8];
        gdImageStringFT( this->image,
            bounds,
            this->axisYTitleColor,
            this->axisYTitleFontNames,
            (double)this->axisYTitleFontSize,
            (3.1415927 / 2.0),
            xLeft, yBottom,
            this->axisYTitle );
    }

    /**
     * Draws the plot title, centered above the plot area.
     *
     * The title is drawn using current attributes:
     * - Title.
     * - Title color.
     * - Title font names.
     * - Title font size.
     *
     * @see getTitle()
     * @see getTitleColor()
     * @see getTitleFontNames()
     * @see getTitleFontSize()
     */
    inline void drawTitle( )
        noexcept
    {
        // If there is no title, do nothing.
        if ( this->title == nullptr || this->title[0] == '\0' )
            return;

        // If there are no cached text bounds, compute them.
        if ( this->titleWidth == 0 )
        {
            this->computeTextBounds(
                this->title,
                this->titleFontNames,
                this->titleFontSize,
                0.0,
                this->titleWidth, this->titleHeight );
        }

        // Draw the title, centering it above the plot area and within
        // the upper margin.
        const unsigned int xLeft =
            (this->plotXRange[0] + this->plotWidth/2) -
            this->titleWidth / 2;
        const unsigned int yBottom =
            (this->imageHeight - this->plotYRange[1]) / 2 +
            this->titleHeight / 2;

        int bounds[8];
        gdImageStringFT( this->image,
            bounds,
            this->titleColor,
            this->titleFontNames,
            (double)this->titleFontSize,
            0.0,
            xLeft, yBottom,
            this->title );
    }

    /**
     * Draws the plot legend.
     *
     * The title is drawn using current attributes:
     * - Legend values.
     * - Legend colors.
     * - Legend font names.
     * - Legend font size.
     *
     * @see getLegendValues()
     * @see getLegendColors()
     * @see getLegendFontNames()
     * @see getLegendFontSize()
     */
    inline void drawLegend( )
        noexcept
    {
        // If there are no legend values, do nothing.
        const size_t n = this->legendValues.size( );
        if ( n == 0 )
            return;

        // Loop over the legend values and find the largest legend height.
        // We'll use this to space all of the legend text evenly vertically.
        int maxTextHeight = 0;
        for ( size_t i = 0; i < n; ++i )
        {
            int textWidth = this->legendWidths[i];
            int textHeight = this->legendHeights[i];

            // If there are no cached text bounds, compute them.
            if ( textWidth == 0 )
            {
                this->computeTextBounds(
                    this->legendValues[i],
                    this->legendFontNames,
                    this->legendFontSize,
                    0.0,
                    textWidth, textHeight );
                this->legendWidths[i]  = textWidth;
                this->legendHeights[i] = textHeight;
            }

            if ( textHeight > maxTextHeight )
                maxTextHeight = textHeight;
        }

        // Place the left edge of each legend value near the right edge
        // of the plot.
        const int xLeft = this->plotXRange[1] + AXIS_NUMBER_SPACING_FROM_AXIS;

        // Center the lines of legend text vertically beside the plot.
        const int fullTextHeight = n * maxTextHeight;
        int yTop;
        if ( fullTextHeight > this->plotHeight )
        {
            // The legend text is taller than the plot. There's not much we
            // can do to draw this nicely. Just start at the top of the plot
            // and add the legend text row after row and let it go below the
            // plot height, and maybe off the image.
            yTop = this->imageHeight - this->plotYRange[1];
        }
        else {
            yTop = (this->plotHeight - fullTextHeight) / 2 +
                (this->imageHeight - this->plotYRange[1]);
        }

        // Loop over the legend values and draw each of them.
        for ( size_t i = 0; i < n; ++i )
        {
            int bounds[8];
            gdImageStringFT( this->image,
                bounds,
                this->legendColors[i],
                this->legendFontNames,
                (double)this->legendFontSize,
                0.0,
                xLeft, yTop,
                this->legendValues[i] );

            yTop += maxTextHeight;
        }
    }
    // @}



//----------------------------------------------------------------------
//
// Plot.
//
//----------------------------------------------------------------------
    /**
     * @name Shape drawing
     */
    // @{
public:
    /**
     * Draws a dot at (x,y) in the plot area, with the given color.
     *
     * @param[in] x
     *   The X coordiante for the dot's center.
     * @param[in] y
     *   The Y coordiante for the dot's center.
     * @param[in] color
     *   The dot color.
     */
    inline void plotDot( const float x, const float y, const int color )
        noexcept
    {
        // Convert from plot space to image space.
        // Ignore dots that are out of bounds.
        const int xImage = this->convertAxisXToImageX( x, BOUNDS_REJECT );
        if ( xImage < 0 || (unsigned int)xImage >= this->imageWidth )
            return;

        const int yImage = this->convertAxisYToImageY( y, BOUNDS_REJECT );
        if ( yImage < 0 || (unsigned int)yImage >= this->imageHeight )
            return;

        // Plot a dot. Flip Y for the libgd Y axis.
        if ( this->dotSize == 1 )
        {
            gdImageSetPixel(
                this->image,
                xImage, this->imageHeight - yImage,
                color );
        }
        else if ( this->dotShape == DOT_CIRCLE )
        {
            gdImageFilledEllipse(
                this->image,
                xImage, this->imageHeight - yImage,
                this->dotSize, this->dotSize,
                color );
        }
        else
        {
            const int ds2 = this->dotSize / 2;
            gdImageFilledRectangle(
                this->image,
                xImage - ds2, this->imageHeight - yImage - ds2,
                xImage + ds2, this->imageHeight - yImage + ds2,
                color );
        }
    }

    /**
     * Draws a filled polygon in the plot area, with the given color.
     *
     * The X and Y coordinate lists must be the same size.
     *
     * @param[in] xCoordinates
     *   The X coordinates around the polygon.
     * @param[in] yCoordinates
     *   The Y coordinates around the polygon.
     * @param[in] color
     *   The color.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the X and Y coordinate lists are not both
     *   the same size.
     */
    inline void plotFilledPolygon(
        const std::vector<double> xCoordinates,
        const std::vector<double> yCoordinates,
        const int color )
    {
        // Create a point list. Convert the incoming (x,y) coordinates
        // from plot space to image space.  Flip Y for the libgd Y axis.
        const size_t nCoordinates = xCoordinates.size( );
        if ( nCoordinates != yCoordinates.size( ) )
            throw std::invalid_argument(
                "X and Y coordinate lists are not the same size." );

        gdPointPtr points = new gdPoint[nCoordinates];
        for ( size_t i = 0; i < nCoordinates; ++i )
        {
            points[i].x = this->convertAxisXToImageX(
                xCoordinates[i],
                NO_BOUNDS_CHECK );
            points[i].y = this->imageHeight - this->convertAxisYToImageY(
                yCoordinates[i],
                NO_BOUNDS_CHECK );
        }

        gdImageFilledPolygon(
            this->image,
            points,
            nCoordinates,
            color );
        delete[] points;
    }

    /**
     * Draws a filled rectangle in the plot area, with the given color.
     *
     * @param[in] xStart
     *   The X coordinate at one side of the rectangle.
     * @param[in] yStart
     *   The Y coordinate at one side of the rectangle.
     * @param[in] xEnd
     *   The X coordinate at the other side of the rectangle.
     * @param[in] yEnd
     *   The Y coordinate at the other side of the rectangle.
     * @param[in] color
     *   The color.
     */
    inline void plotFilledRectangle(
        const float xStart, const float yStart,
        const float xEnd, const float yEnd,
        const int color )
        noexcept
    {
        // Convert from plot space to image space.
        // Ignore rectangles where any part is out of bounds.
        const int xStartImage = this->convertAxisXToImageX( xStart, BOUNDS_CROP );
        const int xEndImage = this->convertAxisXToImageX( xEnd, BOUNDS_CROP );
        const int yStartImage = this->convertAxisYToImageY( yStart, BOUNDS_CROP );
        const int yEndImage = this->convertAxisYToImageY( yEnd, BOUNDS_CROP );

        // Swap start/end in Y because libgd requires the rectangle
        // corners be upper left and lower right.
        // Flip Y for the libgd Y axis.
        gdImageFilledRectangle(
            this->image,
            xStartImage, this->imageHeight - yEndImage,
            xEndImage, this->imageHeight - yStartImage,
            color );
    }

    /**
     * Draws a polygon outline in the plot area, with the given color.
     *
     * The X and Y coordinate lists must be the same size.
     *
     * @param[in] xCoordinates
     *   The X coordinates around the polygon.
     * @param[in] yCoordinates
     *   The Y coordinates around the polygon.
     * @param[in] color
     *   The color.
     * @param[in] lineWidth
     *   The line width.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the X and Y coordinate lists are not both
     *   the same size.
     */
    inline void plotPolygon(
        const std::vector<double> xCoordinates,
        const std::vector<double> yCoordinates,
        const int color,
        const unsigned int lineWidth )
    {
        // Create a point list. Convert the incoming (x,y) coordinates
        // from plot space to image space.  Flip Y for the libgd Y axis.
        const size_t nCoordinates = xCoordinates.size( );
        if ( nCoordinates != yCoordinates.size( ) )
            throw std::invalid_argument(
                "X and Y coordinate lists are not the same size." );

        gdPointPtr points = new gdPoint[nCoordinates];
        for ( size_t i = 0; i < nCoordinates; ++i )
        {
            points[i].x = this->convertAxisXToImageX(
                xCoordinates[i],
                NO_BOUNDS_CHECK );
            points[i].y = this->imageHeight - this->convertAxisYToImageY(
                yCoordinates[i],
                NO_BOUNDS_CHECK );
        }

        gdImageSetThickness( this->image, lineWidth );

        gdImagePolygon(
            this->image,
            points,
            nCoordinates,
            color );
        delete[] points;
    }

    /**
     * Draws a rectangle outline in the plot area, with the given color.
     *
     * @param[in] xStart
     *   The X coordinate at one side of the rectangle.
     * @param[in] yStart
     *   The Y coordinate at one side of the rectangle.
     * @param[in] xEnd
     *   The X coordinate at the other side of the rectangle.
     * @param[in] yEnd
     *   The Y coordinate at the other side of the rectangle.
     * @param[in] color
     *   The color.
     * @param[in] lineWidth
     *   The line width.
     */
    inline void plotRectangle(
        const float xStart, const float yStart,
        const float xEnd, const float yEnd,
        const int color,
        const unsigned int lineWidth )
        noexcept
    {
        // Convert from plot space to image space.
        // Ignore rectangles where any part is out of bounds.
        const int xStartImage = this->convertAxisXToImageX( xStart, BOUNDS_CROP );
        const int xEndImage   = this->convertAxisXToImageX( xEnd, BOUNDS_CROP );
        const int yStartImage = this->convertAxisYToImageY( yStart, BOUNDS_CROP );
        const int yEndImage   = this->convertAxisYToImageY( yEnd, BOUNDS_CROP );

        gdImageSetThickness( this->image, lineWidth );

        // Swap start/end in Y because libgd requires the rectangle
        // corners be upper left and lower right.
        // Flip Y for the libgd Y axis.
        gdImageRectangle(
            this->image,
            xStartImage, this->imageHeight - yEndImage,
            xEndImage, this->imageHeight - yStartImage,
            color );
    }
    // @}



//----------------------------------------------------------------------
//
// Save to image file.
//
//----------------------------------------------------------------------
    /**
     * @name Save file
     */
    // @{
public:
    /**
     * Saves the image to a file.
     *
     * @param[in] filePath
     *   The path to the file.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the file path is NULL or empty, or if
     *   an I/O error occurs.
     */
    inline void save( const char*const filePath )
    {
        // Validate.
        // ---------
        // Make sure the path is not a NULL pointer.
        if ( filePath == nullptr )
            throw std::invalid_argument(
                std::string( "NULL file path argument.\n" ) +
                std::string( "The file path should indicate an absolute or relative path to an FGB file to save." ) );

        this->save( std::string( filePath ) );
    }

    /**
     * Saves the image to a file.
     *
     * @param[in] filePath
     *   The path to the file.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the file path is empty, or if
     *   an I/O error occurs.
     *
     * @internal
     * The default image file write call in libgd is gdImageFile(). This
     * call gets the file name extension from the path, selects a corresponding
     * file format, and writes the image into that file using default
     * compression settings.
     *
     * For PNG files, the default compression setting depends upon the
     * default in libz, which is determined when that library is compiled.
     * And the usual default there is level 9 = compress as much as possible,
     * taking as long as necessary.
     *
     * For high performance, however, the time to write the file is more
     * important than obtaining the absolute smallest image size. We therefore
     * need to scale back the compression effort. And this requires using
     * the gdImagePngEx() function instead.
     *
     * PNG compression levels vary from 0, for no compression, to 1,
     * for compress quickly, up to 9, for compress slowly. Experimentally,
     * a 0 is slower than a 1 for scatter plot data - probably because the
     * uncompressed data is so much larger that the additional I/O time
     * exceeds the time it would have taken to compress it a little first.
     * @endinternal
     */
    inline void save( const std::string filePath )
    {
        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( filePath.empty( ) == true )
            throw std::invalid_argument(
                std::string( "Empty file path argument.\n" ) +
                std::string( "The file path should indicate an absolute or relative path to an image file to save." ) );

        // Find the file name extension.
        const size_t lastDot = filePath.rfind( '.' );
        if ( lastDot == std::string::npos )
            throw std::invalid_argument(
                std::string( "Missing file name extension.\n" ) +
                std::string( "The file path for the new image file does not have a file name extension. The image file format cannot be determined." ) );

        const std::string extension = filePath.substr( lastDot + 1 );
        if ( extension != "png" &&
             extension != "gif" &&
             extension != "jpg" &&
             extension != "jpeg" )
            throw std::invalid_argument(
                std::string( "Unknown file name extension.\n" ) +
                std::string( "The file path for the new image file has an unrecognized file name extension. The image file format cannot be determined. Supported image formats are 'png', 'gif', and 'jpg'." ) );


        //
        // Open, save, and close.
        // ----------------------
        // Open the file for binary buffered reading using stdio.
        // Throw an exception if the file cannot be found or opened.
        std::FILE* fp = std::fopen( filePath.c_str( ), "wb" );
        if ( fp == nullptr )
            throw std::invalid_argument(
                std::string( "Cannot open the save image file.\n" ) +
                std::string( "When sasving the file \"" ) + filePath +
                std::string( "\" the system reported the error: " ) +
                std::string( std::strerror( errno ) ) );


        //
        // Save the file.
        // --------------
        // Thread-lock the file prior to stdio operations so that stdio
        // doesn't lock repeatedly on every function call.
        try
        {
#ifdef FLOWGATE_SCATTERPLOT_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            if ( extension == "png" )
            {
                // Write the PNG file using a fast compression level.
                gdImagePngEx( this->image, fp, PNG_COMPRESSION );
            }
            else if ( extension == "gif" )
            {
                // Write the GIF file. There is no compression control.
                gdImageGif( this->image, fp );
            }
            else if ( extension == "jpg" || extension == "jpeg" )
            {
                // Write the JPEG file using a fast quality level.
                gdImageJpeg( this->image, fp, 30 );
            }

            fclose( fp );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_SCATTERPLOT_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );
            throw;
        }
    }
    // @}
};

} // End Plot namespace
} // End FlowGate namespace

#undef FLOWGATE_SCATTERPLOT_USE_FLOCKFILE
