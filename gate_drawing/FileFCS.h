/**
 * @file
 * Loads and saves flow cytometry events in the ISAC standard FCS file format.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 * @li FlowGate "EventTable.h"
 *
 * @todo Implement set*() methods:
 * @li setParameterCalibrationScaling( )
 * @li setParameterDetectorLightPercentage( )
 * @li setParameterDetectorOpticalFilter( )
 * @li setParameterDetectorType( )
 * @li setParameterDetectorVoltage( )
 * @li setParameterDisplayScaling( )
 * @li setParameterDetectorExcitationPower( )
 * @li setParameterDetectorExcitationWavelengths( )
 * @li setParameterScaling( )
 * @li setParameterLongName( )
 * @li setParameterRange( )
 * @li setParameterShortName( )
 * @li setSpilloverMatrix( )
 * @li setCompensationRequired( )
 * @li setTimestep( )
 * @li setTriggerParameter( )
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
#include <cctype>       // std::isdigit, std::toupper, ...
#include <cerrno>       // Errno
#include <cmath>        // std::pow()
#include <cstdio>       // std::FILE, std::fopen, std::fclose, ...
#include <cstring>      // std::strerror, ...

// Standard C++ libraries.
#include <algorithm>    // std::transform
#include <iostream>     // std::cerr, ...
#include <limits>       // std::numeric_limits
#include <map>          // std::map
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <vector>       // std::vector
#include <map>          // std::map

// Deprecated C++ libraries.
// <codecvt> to convert between wide character Unicode strings and
// variable-character UTF-8 strings has been deprecated in C++17, but
// no replacement was defined. We still need it. Include it anyway,
// but suppress compiler warnings (Visual Studio).
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>      // Unicode translation

// FlowGate libraries.
#include "EventTable.h" // Event storage





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
#define FLOWGATE_FILEFCS_USE_FLOCKFILE
#endif

//
// Byte order.
// -----------
// This software uses the current host's byte order to guide byte swapping
// of FCS binary data. There are three well-known byte orders:
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
#define FLOWGATE_FILEFCS_USE_BSWAP
#elif defined(__clang__) && __has_builtin(__builtin_bswap16__) && \
    __has_builtin(__builtin_bswap32__) && \
    __has_builtin(__builtin_bswap64__)
#define FLOWGATE_FILEFCS_USE_BSWAP
#endif





namespace FlowGate {
namespace File {

class FCSVocabulary;
class FileFCS;





//----------------------------------------------------------------------
//
// Vocabulary.
//
// The vocabulary records the keywords and attributes defined in the FCS
// specification, and through observation of keywords added by vendors.
//
//----------------------------------------------------------------------
/**
 * Indicates an informal keyword category.
 *
 * The FCS specification does not categorize keywords. The categories
 * defined here are unofficial and informal. They are intended as an
 * aid in grouping and filtering keywords.
 */
class FCSKeywordCategory final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    /**
     * The keyword's value describes how data was acquired.
     *
     * Values may indicate the date and time data was collected,
     * and the instrumentation used.
     */
    static const uint8_t ACQUISITION = 0;

    /**
     * The keyword's value describes cell subsets.
     *
     * Values indicate the number of subsets, the bits used, and so on.
     */
    static const uint8_t CELLSUBSET = 1;

    /**
     * The keyword's value describes information to perform compensation.
     *
     * Values indicate the fluorescence spillover matrix and related
     * values.
     */
    static const uint8_t COMPENSATION = 2;

    /**
     * The keyword's value provides device documentation.
     *
     * Values indicate the name of a device and software used,
     * values for device parameters, and software version numbers.
     */
    static const uint8_t DOCDEVICE = 3;

    /**
     * The keyword's value provides general documentation.
     *
     * Values indicate the name of a project or experiment, or provide
     * generic comments.
     */
    static const uint8_t DOCGENERAL = 4;

    /**
     * The keyword's value describes the history of the data.
     *
     * Values indicate the date and time the data was acquired, where it
     * was acquired, and by whome it was acquired.
     */
    static const uint8_t DOCHISTORY = 5;

    /**
     * The keyword's value describes the source of the sample.
     *
     * Values indicate the ID of the well, plate, and sample, and
     * possibly a patient ID.
     */
    static const uint8_t DOCSOURCE = 6;

    /**
     * The keyword's value describes how data is stored in a file.
     *
     * Values indicate the byte offsets to different parts of the file
     * and the way data is stored.
     */
    static const uint8_t FILESTORAGE = 7;

    /**
     * The keyword's value describes gates.
     *
     * Values indicate the number of gates and their attributes.
     */
    static const uint8_t GATING = 8;

    /**
     * The keyword's value describes a histogram.
     *
     * Values indicate the number of events and maximum value for
     * individual histogram channels.
     *
     * This class does not directly support histogram values. This
     * feature has been deprecated in the FCS specification.
     */
    static const uint8_t HISTOGRAM = 9;

    /**
     * The keyword's value describes presentation attributes.
     *
     * Values may indicate names and other visual presentation values.
     */
    static const uint8_t PRESENTATION = 10;

    /**
     * The keyword is unknown.
     */
    static const uint8_t OTHER = 11;



private:
//----------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------
    /**
     * This class cannot be instantiated.
     */
    FCSKeywordCategory( ) = delete;
    ~FCSKeywordCategory( ) = delete;



public:
//----------------------------------------------------------------------
// Methods.
//----------------------------------------------------------------------
    /**
     * Returns the number of categories.
     *
     * @return
     *   Returns the number of categories.
     */
    static size_t getNumberOfCategories( )
        noexcept
    {
        return 12;
    }

    /**
     * Returns a string representation of a keyword category.
     *
     * @param[in] cat
     *   The keyword category.
     *
     * @return
     *   Returns a string representation of the category.
     *
     * @see findCategory()
     */
    static std::string toString( const uint8_t cat )
        noexcept
    {
        switch ( cat )
        {
        case ACQUISITION:
            return std::string( "acquisition" );
        case COMPENSATION:
            return std::string( "compensation" );
        case DOCDEVICE:
            return std::string( "docdevice" );
        case DOCGENERAL:
            return std::string( "docgeneral" );
        case DOCHISTORY:
            return std::string( "dochistory" );
        case DOCSOURCE:
            return std::string( "docsource" );
        case PRESENTATION:
            return std::string( "presentation" );
        case FILESTORAGE:
            return std::string( "filestorage" );
        case CELLSUBSET:
            return std::string( "cellsubset" );
        case GATING:
            return std::string( "gating" );
        case HISTOGRAM:
            return std::string( "histogram" );
        default:
        case OTHER:
            return std::string( "other" );
        }
    }

    /**
     * Returns the dictionary keyword category for a string name.
     *
     * If the category is not recognized, OTHER is returned.
     *
     * @param[in] name
     *   The name of a keyword category.
     *
     * @return
     *   Returns the keyword category for the name.
     *
     * @see toString()
     */
    static uint8_t findCategory( const std::string& name )
        noexcept
    {
        if ( name == "acquisition" )
            return ACQUISITION;
        if ( name == "compensation" )
            return COMPENSATION;
        if ( name == "docdevice" )
            return DOCDEVICE;
        if ( name == "docgeneral" )
            return DOCGENERAL;
        if ( name == "dochistory" )
            return DOCHISTORY;
        if ( name == "docsource" )
            return DOCSOURCE;
        if ( name == "presentation" )
            return PRESENTATION;
        if ( name == "filestorage" )
            return FILESTORAGE;
        if ( name == "cellsubset" )
            return CELLSUBSET;
        if ( name == "gating" )
            return GATING;
        if ( name == "histogram" )
            return HISTOGRAM;
        return OTHER;
    }
};



/**
 * Defines attributes of a keyword in the FCS vocabulary.
 *
 * The FCS specifications define a set of standard keywords for
 * keyword-value pairs that may occur within an FCS file. Vendors have
 * defined additional keywords for their own hardware and software.
 *
 * Each keyword has a name and value in an expected data type, such as
 * string, long integer, or double-precision floating point. Additional
 * keyword attributes defined here include a short description and an
 * informal category used to group keywords. This class also notes
 * which keywords are defined in the FCS standards, and which standard
 * version, along with whether the keyword is required, deprecated,
 * and so forth.
 *
 * Some keyword values may contain personally identifying information
 * that is regulated by HIPAA and other country medical data laws.
 * This too is noted for the keyword.
 */
class FCSKeyword final
{
    friend class FCSVocabulary;

//----------------------------------------------------------------------
// Enums and constants.
//----------------------------------------------------------------------
public:
    // Specification versions ------------------------------------------
    /**
     * The version bit set when the keyword is supported in the
     * FCS 1.0 specification.
     */
    static const uint32_t FCS_VERSION_1_0 = 0x00000001u;

    /**
     * The version bit set when the keyword is supported in the
     * FCS 2.0 specification.
     */
    static const uint32_t FCS_VERSION_2_0 = 0x00000010u;

    /**
     * The version bit set when the keyword is supported in the
     * FCS 3.0 specification.
     */
    static const uint32_t FCS_VERSION_3_0 = 0x00000100u;

    /**
     * The version bit set when the keyword is supported in the
     * FCS 3.1 specification.
     */
    static const uint32_t FCS_VERSION_3_1 = 0x00000200u;



    // Value data types ------------------------------------------------
    /**
     * A UTF-8 string.
     */
    static const uint8_t STRING_VALUE = 0;

    /**
     * An integer number, stored here as a long.
     */
    static const uint8_t LONG_VALUE = 1;

    /**
     * A floating point number, stored here as a double.
     */
    static const uint8_t DOUBLE_VALUE = 2;

    /**
     * Multi-values with a keyword-specific syntax, such as a comma-separated
     * list of strings and/or numbers.
     */
    static const uint8_t MULTI_VALUE = 3;



    // Standard flags --------------------------------------------------
    /**
     * The flag bit set when the keyword is standard and defined in
     * the FCS 1.0, 2.0, 3.0, or 3.1 specifications.
     */
    static const uint32_t STANDARD = 0x00000001u;

    /**
     * The flag bit set when the keyword is required, according to
     * the FCS 3.1 specification.
     */
    static const uint32_t REQUIRED = 0x00000002u;

    /**
     * The flag bit set when the keyword is deprecated, according to
     * the FCS 3.1 specification.
     */
    static const uint32_t DEPRECATED = 0x00000004u;



    // Characteristics flags -------------------------------------------
    /**
     * The flag bit set when the keyword is for a parameter attribute.
     */
    static const uint32_t PARAMETER = 0x00000008u;

    /**
     * The flag bit set when the keyword is for a gate attribute.
     */
    static const uint32_t GATE = 0x00000010u;



    // De-identify flags -----------------------------------------------
    /**
     * The flag bit set when the keyword may contain date and time information.
     *
     * Date and time information describes the date and time at which the
     * data was acquired or processed. Because data acquisition at an
     * insitution remains an involved process and a limited number of patients
     * can be processed per day, knowing the date on which data was acquired
     * could conceivably be used to trace back to the patient. This data
     * is therefore potentially personal information and possibly considered
     * "personal health information" that may be governed by privacy
     * regulations, such as the U.S. HIPAA privacy rule.
     *
     * @see isDateInformation()
     * @see FileFCS::deidentify()
     */
    static const uint32_t DATE_DATA = 0x00001000u;

    /**
     * The flag bit set when the keyword may contain patient information.
     *
     * Patient information describes the individual from whom a sample was
     * taken and used for data acquisition. This may include the patient's
     * name or an identification number that can be used to trace back to
     * the patient. Patient information is considered "personal health
     * information" that may be governed by privacy regulations, such as
     * the U.S. HIPAA privacy rule.
     *
     * @see isPersonalInformation()
     * @see FileFCS::deidentify()
     */
    static const uint32_t PERSONAL_DATA = 0x00002000u;

    /**
     * The flag bit set when the keyword's value may contain user
     * information.
     *
     * User information describes the individual, department, organization,
     * or institution that acquired the data or processed it.
     *
     * @see isUserInformation()
     * @see FileFCS::deidentify()
     */
    static const uint32_t USER_DATA = 0x00004000u;


//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The keyword.
     */
    std::string keyword;

    /**
     * A short description of the keyword and value.
     */
    std::string description;

    /**
     * The FCS format versions that support this keyword.
     *
     * The value is a bitmask of version values.
     */
    uint32_t versions;

    /**
     * The expected data type for the keyword's value.
     *
     * Data type values include:
     * @li STRING_VALUE
     * @li LONG_VALUE
     * @li DOUBLE_VALUE
     * @li MULTI_VALUE
     */
    uint8_t dataType;

    /**
     * The keyword category.
     */
    uint8_t category;

    /**
     * The character offset to an embedded index number within the
     * keyword name. When zero, there is no embedded index.
     */
    uint8_t indexOffset;

    /**
     * A set of single-bit flags indicating attributes of the keyword.
     */
    uint32_t flags;



//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
public:
    /**
     * Constructs an empty dictionary keyword description.
     *
     * This is strictly for use in initializing a std::map, or similar
     * container.
     */
    FCSKeyword( )
        noexcept
    {
        // Keyword and description are left empty.
    }

private:
    /**
     * Constructs a dictionary keyword description.
     *
     * @param[in] keyword
     *   The keyword.
     * @param[in] description
     *   The short description.
     * @param[in] dataType
     *   The data type for the keyword's value.
     * @param[in] category
     *   The category for the keyword.
     * @param[in] versions
     *   The bitmask of version number flags for the keyword.
     * @param[in] flags
     *   The bitmask of flags for the keyword.
     * @param[in] indexOffset
     *   The character offset from the start of the keyword to the point
     *   where an index number may be embedded within the keyword. For
     *   instance, the standard "$PnN" keyword provides the short name for
     *   parameter n, where n is an embedded parameter index as character
     *   2 (with 0-based indexing) in the keyword.
     */
    FCSKeyword(
        const std::string keyword,
        const std::string description,
        const uint32_t dataType,
        const uint32_t category,
        const uint32_t versions,
        const uint32_t flags,
        const uint32_t indexOffset )
        noexcept
        : keyword( keyword ),
          description( description ),
          versions( versions ),
          dataType( dataType ),
          category( category ),
          indexOffset( indexOffset ),
          flags( flags )
    {
        // Nothing to do.
    }

public:
    /**
     * Constructs a dictionary keyword description by copying another.
     *
     * @param[in] a
     *   The attributes to copy.
     */
    FCSKeyword( const FCSKeyword& a )
        noexcept
        : keyword( a.keyword ),
          description( a.description ),
          versions( a.versions ),
          dataType( a.dataType ),
          category( a.category ),
          indexOffset( a.indexOffset ),
          flags( a.flags )
    {
        // Nothing to do.
    }

    /**
     * Destroys the object.
     */
    ~FCSKeyword( )
        noexcept
    {
        // Nothing to do.
    }



//----------------------------------------------------------------------
// Attribute methods.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the informal category for the keyword.
     *
     * Keyword categories are informal non-official groupings of keywords
     * that are provided to help organize keywords and guide their
     * presentation.
     *
     * @return
     *   Returns the keyword category.
     *
     * @see FCSKeywordCategory
     */
    inline const uint32_t getCategory( )
        const noexcept
    {
        return this->category;
    }

    /**
     * Returns the short description for this attribute.
     *
     * Short descriptions are sentence fragments containing a few words.
     * They are not full descriptions of the keyword and the meaning of
     * its values. Please see the FCS specification for detailed
     * information.
     *
     * Short descriptions are also returned for known keywords that are
     * not defined in the FCS specification. Such keywords are defined by
     * hardware and software vendors, and are often not fully documented
     * for public use. Short descriptions for these keywords are only
     * approximate.
     *
     * @return
     *   Returns a const reference to the short description.
     */
    inline const std::string& getDescription( )
        const noexcept
    {
        return this->description;
    }

    /**
     * Returns the keyword for this attribute.
     *
     * @return
     *   Returns a const reference to the keyword.
     */
    inline const std::string& getKeyword( )
        const noexcept
    {
        return this->keyword;
    }

    /**
     * Returns the bitmask indicating flags for keyword attributes.
     *
     * Known flags are:
     * @li STANDARD
     * @li REQUIRED
     * @li DEPRECATEd
     * @li PARAMETER
     * @li GATE
     * @li PESONAL
     *
     * @return
     *   Returns the flags.
     */
    inline const uint32_t getFlags( )
        const noexcept
    {
        return this->flags;
    }

    /**
     * Returns a bitmask indicating the specification versions for the keyword.
     *
     * Known versions are:
     * @li FCS_VERSION_1_0
     * @li FCS_VERSION_2_0
     * @li FCS_VERSION_3_0
     * @li FCS_VERSION_3_1
     *
     * Most keywords are defined in all versions of the FCS specifications,
     * but some are only defined for specific versions. Version testing
     * may be done by masking against the returned value:
     * @code
     * auto versions = attributes.getSpecificationVersions( );
     * if ( (versions & attributes.FCS_VERSION_3_1) != 0 )
     * {
     *   ...
     * }
     * @endcode
     *
     * If a keyword is defined by one or more vendors and/or common use,
     * but is not in the specification, then this method returns zero.
     *
     * All keywords defined in the FCS specifications start with a
     * dollar-sign ("$"). However, this is not a reliable indicator that
     * a keyword is defined in a specification. Some vendors have defined
     * their own keywords starting with dollar-signs.
     *
     * @return
     *   Returns a bitmask of version codes.
     *
     * @see getKeyword()
     * @see isStandard( )
     */
    inline const uint32_t getSpecificationVersions( )
        const noexcept
    {
        return this->versions;
    }



    /**
     * Returns true if the keyword has double values.
     *
     * @return
     *   Returns true if the keyword has double values.
     */
    inline bool hasDoubleValue( )
        const noexcept
    {
        return (this->dataType == DOUBLE_VALUE);
    }

    /**
     * Returns true if the keyword has long values.
     *
     * @return
     *   Returns true if the keyword has long values.
     */
    inline bool hasLongValue( )
        const noexcept
    {
        return (this->dataType == LONG_VALUE);
    }

    /**
     * Returns true if the keyword has multiple values that must be parsed.
     *
     * @return
     *   Returns true if the keyword has multiple values.
     */
    inline bool hasMultipleValues( )
        const noexcept
    {
        return (this->dataType == MULTI_VALUE);
    }

    /**
     * Returns true if the keyword has string values.
     *
     * @return
     *   Returns true if the keyword has string values.
     */
    inline bool hasStringValue( )
        const noexcept
    {
        return (this->dataType == STRING_VALUE);
    }



    /**
     * Returns true if the keyword's value may contain date and time
     * information.
     *
     * Date and time information describes the date and time at which the
     * data was acquired or processed. Because data acquisition at an
     * insitution remains an involved process and a limited number of patients
     * can be processed per day, knowing the date on which data was acquired
     * could conceivably be used to trace back to the patient. This data
     * is therefore potentially personal information and possibly considered
     * "personal health information" that may be governed by privacy
     * regulations, such as the U.S. HIPAA privacy rule.
     *
     * @return
     *   Returns true if the keyword may be contain user information.
     *
     * @see FileFCS::deidentify()
     */
    inline bool isDateInformation( )
        const noexcept
    {
        return (this->flags & DATE_DATA) != 0;
    }

    /**
     * Returns true if the keyword is deprecated in the latest FCS
     * specification.
     *
     * @return
     *   Returns true if the keyword is deprecated.
     */
    inline bool isDeprecated( )
        const noexcept
    {
        return (this->flags & DEPRECATED) != 0;
    }

    /**
     * Returns true if the keyword is for a gate.
     *
     * @return
     *   Returns true if the keyword is for a gate.
     */
    inline bool isGate( )
        const noexcept
    {
        return (this->flags & GATE) != 0;
    }

    /**
     * Returns true if the keyword is for a parameter.
     *
     * @return
     *   Returns true if the keyword is for a parameter.
     */
    inline bool isParameter( )
        const noexcept
    {
        return (this->flags & PARAMETER) != 0;
    }

    /**
     * Returns true if the keyword's value may contain personal information.
     *
     * Patient information describes the individual from whom a sample was
     * taken and used for data acquisition. This may include the patient's
     * name or an identification number that can be used to trace back to
     * the user. Patient information is considered "personal health
     * information" that may be governed by privacy regulations, such as
     * the U.S. HIPAA privacy rule.
     *
     * @return
     *   Returns true if the keyword may be contain personal information.
     *
     * @see FileFCS::deidentify()
     */
    inline bool isPersonalInformation( )
        const noexcept
    {
        return (this->flags & PERSONAL_DATA) != 0;
    }

    /**
     * Returns true if the keyword is required in the latest FCS
     * specification.
     *
     * @return
     *   Returns true if the keyword is required.
     */
    inline bool isRequired( )
        const noexcept
    {
        return (this->flags & REQUIRED) != 0;
    }

    /**
     * Returns true if the keyword is standard in any of the FCS
     * specification versions.
     *
     * If a keyword is defined by one or more vendors and/or common use,
     * but is not in the specification, then this method returns false.
     *
     * All keywords defined in the FCS specifications start with a
     * dollar-sign ("$"). However, this is not a reliable indicator that
     * a keyword is defined in a specification. Some vendors have defined
     * their own keywords starting with dollar-signs.
     *
     * @return
     *   Returns true if the keyword is standard.
     *
     * @see getKeyword()
     * @see getSpecificationVersions()
     */
    inline bool isStandard( )
        const noexcept
    {
        return (this->flags & STANDARD) != 0;
    }

    /**
     * Returns true if the keyword's value may contain user information.
     *
     * User information describes the individual, department, organization,
     * or institution that acquired the data or processed it.
     *
     * @return
     *   Returns true if the keyword may be contain user information.
     *
     * @see FileFCS::deidentify()
     */
    inline bool isUserInformation( )
        const noexcept
    {
        return (this->flags & USER_DATA) != 0;
    }
    // @}
};



/**
 * Defines attributes of a known dictionary keyword.
 *
 * The FCS specifications define a set of standard keywords for
 * keyword-value pairs that may occur within an FCS file. Vendors have
 * defined additional keywords for their own hardware and software.
 *
 * Each keyword has a name and value in an expected data type, such as
 * string, long integer, or double-precision floating point. Additional
 * keyword attributes defined here include a short description and an
 * informal category used to group keywords. This class also notes
 * which keywords are defined in the FCS standards, and which standard
 * version, along with whether the keyword is required, deprecated,
 * and so forth.
 *
 * Some keyword values may contain personally identifying information
 * that is regulated by HIPAA and other country medical data laws.
 * This too is noted for the keyword.
 */
class FCSVocabulary final
{
//----------------------------------------------------------------------
// Constructors / Destructors.
//----------------------------------------------------------------------
private:
    /**
     * A vocabulary cannot be constructed.
     */
    FCSVocabulary( ) = delete;
    ~FCSVocabulary( ) = delete;



//----------------------------------------------------------------------
// Methods.
//----------------------------------------------------------------------
    /**
     * @name Search
     */
    // @{
private:
    /**
     * Returns the vocabulary's map of keyword names and attributes.
     *
     * The static keyword dictionary attributes map uses keywords keys to
     * deliver attributes objects that characterize known keywords.
     *
     * @return
     *   Returns the vocabulary's map of keyword names and attributes.
     */
    static std::map<std::string, FCSKeyword>* initialize( )
        noexcept;

public:
    /**
     * Looks up a keyword and returns its attributes, if known.
     *
     * The given keyword is looked up within a list of known keywords
     * defined either by the FCS specifications or by vendors and common use.
     * If they keyword is not found, an exception is thrown. Otherwise
     * a reference to the keyword attributes is returned.
     *
     * Keyword attributes describe the keyword, including providing a short
     * description, an informal category for the keyword, the FCS
     * specification versions that support it, the data type for the keyword's
     * value, and flags indicating if the keyword is required, deprecated, or
     * may contain personally identifying information in its value.
     *
     * FCS keywords include simple keywords that may occur once in a file,
     * and keyword templates that have an embedded numeric index so that
     * the keyword can occur multiple times in a file. This is particularly
     * important for parameter keywords that provide attributes for a
     * specific parameter (column) in each event.
     *
     * The given keyword may be either a simple keyword or one with an
     * embedded numeric index. Numeric indexes are automatically recognized
     * and the keyword converted to a generic representation that may be
     * found in the internal list of known keywords.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the keyword attributes for the matched keyword.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is not found.
     */
    static const FCSKeyword& find( const std::string keyword )
    {
        static std::map<std::string, FCSKeyword>* map = nullptr;

        if ( map == nullptr )
            map = initialize( );

        //
        // Validate.
        // ---------
        // Empty keywords are invalid.
        if ( keyword.empty( ) == true )
            throw std::invalid_argument( "Empty FCS keyword." );

        //
        // Look up keyword as-is.
        // ----------------------
        // Get the map and look up the keyword as-is, ignoring the possibility
        // of an embedded index.
        if ( map->count( keyword ) != 0 )
            return map->at( keyword );

        //
        // Look up keyword, considering embedded indexes.
        // ----------------------------------------------
        // A keyword like "$BLAHnBLAH" has an embedded integer index "n".
        // The attribute map lists these with a non-zero character offset
        // to the position of the index. In the case of "$BLAHnBLAH", the
        // offset would be 5, counting up from 0 for the $.
        //
        // Loop over all keywords and check them against the given keyword.
        const size_t length = keyword.length( );
        for ( auto it = map->cbegin( ); it != map->cend( ); ++it )
        {
            auto mapKeyword = (*it).first;
            auto mapAttr    = (*it).second;

            // If the keyword doesn't support an embedded index, skip it.
            if ( mapAttr.indexOffset == 0 )
                continue;

            // If the embedded index location is beyond the end of the
            // given keyword, skip it.
            if ( mapAttr.indexOffset >= length )
                continue;

            // If the prefix up to the embedded index doesn't match, skip it.
            if ( mapKeyword.compare( 0, mapAttr.indexOffset, keyword, 0, mapAttr.indexOffset ) != 0 )
                continue;

            // If the next character in the given keyword is not a digit,
            // skip it.
            size_t i = mapAttr.indexOffset;
            if ( std::isdigit( keyword[i] ) == false )
                continue;

            // Skip past the digits to get rest of the keyword.
            while ( i < length && std::isdigit( keyword[i] ) == true )
                ++i;

            if ( i >= length )
                continue;

            // If the suffix after the embedded index doesn't match, skip it.
            auto suffix = keyword.substr( i );
            if ( mapKeyword.compare( mapAttr.indexOffset + 1, mapKeyword.length( ), suffix ) != 0 )
                continue;

            // The prefix and suffix both match.
            return (*it).second;
        }

        throw std::invalid_argument( "Unrecognized FCS keyword." );
    }
    // @}

    /**
     * @name Parse
     */
    // @{
    /**
     * Extracts and returns the parameter number from a parameter keyword.
     *
     * @param[in] key
     *   The parameter keyword with an embedded parameter index.
     *
     * @return
     *   The embedded parameter index, or 0 if no index was found.
     */
    static size_t getParameterIndexFromKeyword( const std::string& key )
        noexcept
    {
        const size_t length = key.length( );

        // Find the first digit, if any.
        size_t startOfDigits = 0;
        for ( ; startOfDigits < length; ++startOfDigits )
            if ( std::isdigit( key[startOfDigits] ) == true )
                break;

        // If no digits were found, return zero.
        if ( startOfDigits >= length )
            return 0;

        // Find the first non-digit after the first digit.
        size_t startOfSuffix = startOfDigits + 1;
        for ( ; startOfSuffix < length; ++startOfSuffix )
            if ( std::isdigit( key[startOfSuffix] ) == false )
                break;

        std::string digits;
        if ( startOfSuffix >= length )
            digits = key.substr( startOfDigits );
        else
            digits = key.substr( startOfDigits,
                (startOfSuffix - startOfDigits + 1) );

        return std::stol( digits );
    }
    // @}
};



//----------------------------------------------------------------------
//
// FCS file load, save, and data access.
//
// A container holds FCS data loaded from a file, or prepared to save
// to a new file.
//
//----------------------------------------------------------------------
/**
 * Loads and saves flow cytometry events in the ISAC standard FCS file format.
 *
 * The International Society for Advancement of Cytometry (ISAC) has
 * defined the "FCS" file format for the storage of event data from
 * flow cytometry equipment and software. This file format has the
 * following well-known revisions:
 * @li 1.0 from 1984.
 * @li 2.0 from 1990.
 * @li 3.0 from 1997.
 * @li 3.1 from 2010.
 *
 * This software supports loading files using versions 2.0, 3.0, and 3.1,
 * and a variety of vendor customizations. FCS data using version 1.0 may 
 * load as well, if it does not use deprecated or unsupported features.
 *
 *
 * <B>File format</B><BR>
 * Each FCS file contains:
 *
 * @li A text dictionary of keyword-value pairs for named attributes.
 *
 * @li A binary table of acquired data treated as a list of events. Each
 *   event has a list of parameter columns containing numeric data.
 *
 * Some FCS files may contain additional vendor-specific or software-specific
 * data.
 *
 *
 * <B>Loading FCS files</B><BR>
 * An FCS file may be loaded by providing a file path to the constructor.
 * @code
 * auto data = new FileFCS( filepath );
 * @endcode
 *
 * Calling the constructor without a path creates an empty data object
 * that may be loaded from a file by calling load():
 * #code
 * auto file = new FileFCS( );
 * file->load( filepath );
 * #endcode
 *
 * The same FCS data object may be used repeatedly to load multiple files.
 * Each time a file is loaded, the prior content of the FCS data object
 * is deleted automatically:
 * @code
 * auto file = new FileFCS( );
 * file->load( filepath1 );
 * ...
 * file->load( filepath2 );
 * ...
 * file->load( filepath3 );
 * ...
 * @endcode
 *
 * If a problem is encountered while loading an FCS file, an exception
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
 * <B>Saving FCS files</B><BR>
 * The data in an FCS object may be written to a new FCS file by calling
 * the save() method:
 * @code
 * try {
 *   file->save( filepath );
 * } catch ( const std::exception& e ) {
 *   ...
 * }
 * @endcode
 *
 * If a problem is encountered while saving an FCS file, an exception
 * is thrown with a human-readable message indicating the problem, and
 * the file is deleted if it has already been started. Problems with
 * a save include the inability to create or write to the indicated file,
 * and having an incomplete FCS data object. An incomplete object may be
 * missing data, or it may not have required dictionary values set.
 *
 *
 * <B>Getting and setting dictionary values generically</B><BR>
 * FCS dictionary entries all have an ASCII text keyword and a UTF-8 text
 * value that is often parsed as an integer or floating-point value.
 * Standard keywords are defined in the FCS file format specification and
 * have well-known names starting with a dollar-sign ("$"), such as "$TOT"
 * for the total number of events or "$DATE" for the date on which the
 * data was acquired. All keywords that do not start with a dollar-sign
 * are non-standard and may be used by device or software vendors,
 * institutions, and projects to store additional information.
 *
 * All keywords found when loading an FCS file are included in the
 * general-purpose dictionary, including non-standard keywords. The
 * dictionary may be queried to get a list of those keywords.
 * @code
 * // Get a list of dictionary keywords.
 * auto keywords = file->getDictionaryKeywords( );
 * @endcode
 *
 * Keyword values are always stored as UTF-8 Unicode text. For most keywords,
 * the value may be used as a string (such as the name of a parameter), as
 * a long integer (such as an excitation wavelength), or as a double-precision
 * floating-point value (such as the signal gain for a parameter). Some
 * keywords have complex values that may include multiple values separated
 * by commas. For all standard keywords, the data type and possible multi-value
 * structure is defined by the FCS specification and it is up to the caller
 * to access the value properly. For non-standard keywords, the data type is
 * defined by the organization that created the keyword and it is again up to
 * the caller to access the value properly.
 *
 * Several general-purpose methods are available to get dictionary values:
 * @code
 * // Get the acquisition start date as a string.
 * auto adate = file->getDictionaryString( "$DATE" );
 *
 * // Get the total number of events as an integer.
 * auto nEvents = file->getDictionaryLong( "$BTOT" );
 *
 * // Get the linear gain for the 1st parameter as a double.
 * auto linearGain = file->getDictionaryDouble( "$P1G" );
 * @endcode
 *
 * All general-purpose methods that get a value will throw an exception if
 * the keyword is not defined. The dictionary may be checked first:
 * @code
 * if ( file->inDictionary( "$DATE" ) == true )
 * {
 *   auto adate = file->getDictionaryString( "$DATE" );
 *   ...
 * }
 * @endcode
 *
 * Applications may set these values with equivalent generic set methods:
 * @code
 * // Set the acquisition start date from a string.
 * file->setDictionaryString( "$DATE", adate );
 *
 * // Set the total number of events from an integer.
 * file->setDictionaryLong( "$BTOT", nEvents );
 *
 * // Set the linear gain for the 1st parameter from a double.
 * file->setDictionaryDouble( "$P1G", linearGain );
 * @endcode
 *
 * Numeric values used to set dictionary entries are automatically converted
 * to UTF-8 strings when saved into the dictionary.
 *
 * Setting the value of storage-related keywords can be done, but it has no
 * meaning. For instance, the "$BEGINDATA" keyword is used to store the byte
 * offset to the binary data portion of a file. This value is read from a file
 * and used when an FCS file is loaded. But once the file is loaded, the value
 * has no further use. It is retained in the dictionary, but setting it will
 * only change the dictionary entry, and not affect the loaded data. Further,
 * when saving an FCS file, this value is computed automatically when the
 * data is saved. Any value set in the dictionary is ignored.
 *
 *
 * <B>Getting and setting dictionary values specifically</B><BR>
 * The generic methods above get and set dictionary values without a priori
 * knowledge of what the keyword and value mean. For standard keywords, this
 * class provides additional methods that are aware of the data type and
 * structure of the data and will parse and set values appropriately, with
 * validation. These methods use longer names based on the keyword's meaning.
 * For instance, the following methods are equivalent to the earlier
 * dictionary get examples:
 * @code
 * // Get the acquisition start date.
 * auto adate = file->getStartDate( );
 *
 * // Get the total number of events.
 * auto nEvents = file->getNumberOfEvents( );
 *
 * // Get the short name for the 1st parameter.
 * auto shortName = file->getParameterShortName( 0 );
 * @endcode
 *
 * All specific methods that get a value will return an empty string or zero
 * value if the keyword is not defined.
 *
 * The corresponding set methods are also available for most cases:
 * @code
 * // Set the acquisition start date.
 * file->setStartDate( adate );
 *
 * // Set the short name for the 1st parameter.
 * file->setParameterShortName( 0, "ABC" );
 * @endcode
 *
 * These methods get and set the corresponding keywords. Generic and specific
 * methods may be interchanged. For instance, the acquisition date can be
 * retrieved generically then set specifically:
 * @code
 * // Get the acquisition start date as a string.
 * auto adate = file->getDictionaryString( "$DATE" );
 *
 * // Set the acquisition start date.
 * file->setStartDate( adate );
 * @endcode
 *
 *
 * <B>Getting and setting parameter values</B><BR>
 * Parameters are columns in the event data table. Each parameter has
 * multiple attributes, including its name, range, and gain. All of these
 * are accessible via standard keywords in the dictionary, but it is more
 * convenient to access them via specific methods for parameters.
 *
 * All parameter methods take a parameter index. FCS files number parameters
 * starting at "1" for the first one. However, this class conforms instead
 * to C/C++ conventions and numbers parameters starting at "0".
 * @code
 * // Get the short name for the 1st parameter.
 * auto shortName = file->getParameterShortName( 0 );
 *
 * auto voltage = file->getParameterDetectorVoltage( 0 );
 * @endcode
 *
 * Some parameter values are optional. If they are not set, string values
 * return an empty string and numeric values return a zero.
 *
 * Some parameter keyword values do not have specific methods if the value
 * is primarily used when parsing FCS files. For instance, the number of
 * bits reserved for the parameter is not available with its own method,
 * though it is available directly from the dictionary as "$P1B" for the
 * 1st parameter.
 *
 *
 * <B>Getting and setting events</B><BR>
 * Event data for all parameters is stored within an event table.
 *
 * Within original FCS files, events may be single- or double-precision
 * floating point, or a variety of integer widths. For simplicity, all
 * integer event data is automatically widened to single- or double-precision
 * floating point when a file is loaded. While this may require more memory,
 * it improves the performance of subsequent event processing and it
 * simplifies writing event data back out to a new file. Most modern FCS
 * files use single- or double-precision floating point values anyway.
 *
 * @code
 * // Get the event table.
 * auto events = file->getEventTable( );
 * @endcode
 *
 *
 * <B>Limitations of this software</B><BR>
 * This software supports the primary features of FCS 1.0, 2.0, 3.0, and 3.1
 * files. It omits most support for antiquated or infrequently-used
 * features, and for features that have been deprecated in FCS 3.1.
 *
 * Deprecated features that are minimally supported include:
 *
 * - Deprecated keywords. All keywords and values found in the FCS file remain
 *   available in the dictionary, even if deprecated. However, there are no
 *   specific get/set methods and no validition checking on their values.
 *   Deprecated keywords include histogram attributes ($PKn, $PKNn) and
 *   manual gating attributes ($GnE, $GnF, $GnN, $GnP, $GnR, $GnS, $GnT, $GnV).
 *   Deprecated keywords also include the $ASC, $DFCmn, and $GmGnW keywords
 *   from FCS 1.0, the $DFCnTOn keyword from FCS 2.0, and the replaced $COMP
 *   and $UNICODE keywords in FCS 3.0.
 *
 * Deprecated features that are not supported and will throw an exception:
 *
 * - Multiple data sets in the same file, indicated by the $NEXTDATA keyword
 *   with a non-zero value. This feature has been deprecated in FCS 3.1.
 *
 * - The ASCII text data type for storing event data, indicated by the
 *   $DATATYPE keyword with an "A" value, or use of the old $ASC keyword. This
 *   feature has been discouraged since FCS 2.0 and is deprecated in FCS 3.1.
 *
 * - The correlated multivariate histogram and uncorrelated univariate
 *   histogram data modes, indicated by the $MODE keyword with "C" or "U"
 *   values. This feature has been deprecated in FCS 3.1.
 *
 * Antiquated features that are not supported and will throw an exception:
 *
 * - Integer parameter widths that are not 8, 16, 32, or 64-bits. Partial
 *   integer data storage is rarely used any more because of the high cost
 *   of shifting and masking bitwise data read from or written to a file.
 *   These features have been discouraged in FCS 3.1.
 *
 * - The CRC value at the end of the file is not checked on load, and it is
 *   written as blanks on save.
 *
 * Infrequently used features are not supported and are silently ignored:
 *
 * - The analysis segment. This analysis segment is primarily used by
 *   specialized software that adds further information after the event
 *   data. Interpretation of this data is usually not practical outside of
 *   the specialized software that wrote it. If analysis data is in the file,
 *   this software will silently skip it. The caller may check if analysis
 *   data exists in the file by checking if $BEGINANALYSIS is non-zero.
 *
 * - Custom data elsewhere in the file. The FCS specification does not require
 *   that an FCS file contain only FCS data. Additional data may be included
 *   by specialized software. Interpretation of this data is usually not
 *   practical outside of the specialized software that wrote it. If any such
 *   data is present, this software will silently skip it.
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
class FileFCS final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "ISAC FCS files";

    /**
     * The file format name.
     */
    inline static const std::string FORMAT_NAME = "ISAC FCS";

    /**
     * The software version number.
     *
     * The version number is intentionally chosen so that the major and
     * minor numbers match the FCS specification version supported. The
     * subminor number is used to indicate updates to this software.
     */
    inline static const std::string VERSION = "3.1.0";

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
        "FileFCS";

    /**
     * The file format version string when writing new FCS files.
     */
    inline static const std::string DEFAULT_FCS_VERSION = "FCS3.1";

    /**
     * The number of events to load in a block.
     *
     * During file loading, FCS events are ordered row-by-row, but event
     * table data is column-by-column. Transposing from one to the other
     * could be done by reading each file value one at a time and saving
     * it to the proper column, but this has a huge number of read function
     * calls. This can be reduced to just one function call by loading all
     * the events into a buffer first, and then distributing to columns,
     * but this doubles the memory footprint. The middle-ground is to load
     * events in blocks with a size, in events, set here.
     */
    inline static const size_t LOAD_EVENT_BLOCK_SIZE = 10000;



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
     * The total number of events in the file.
     *
     * @see ::load()
     */
    size_t numberOfFileEvents = 0L;

    /**
     * The current file's delimiter for keyword values in the dictionary
     *
     * For example, if the delimiter is "/" then the dictionary entry
     * for the total number of events would be "$TOT/1234/".
     *
     * This value is used during parsing of a file being loaded, and it
     * is used when saving a file. It has no meaning after a file is
     * loaded or saved.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getFileDictionaryValueDelimiter()
     * @see ::setFileDictionaryValueDelimiter()
     */
    char fileDictionaryValueDelimiter = '/';

    /**
     * The current file's byte order for numeric values in the DATA segment.
     *
     * When true, data is in the least-significant-byte-first (LSBF)
     * order. When false, data is in the most-significant-byte-first (MSBF)
     * order.
     *
     * On file load, this value is set to the byte order of the loaded file.
     *
     * On file save, this value is set to the host's byte order, which is
     * the byte order of the saved file.
     *
     * @see ::load()
     * @see ::save()
     */
    bool fileIsLSBF = true;

    /**
     * The current file's raw data type.
     *
     * Values match those of the $DATATYPE keyword and are one of:
     * @li "I" for integers.
     * @li "F" for single-precision floats.
     * @li "D" for double-precision floats.
     *
     * On file load, this value is set to the data type of the loaded file.
     * Integer data is always upscaled to floating-point in the event table,
     * but this value may be queried to determine if the file's data was
     * integers.
     *
     * On file save, this value is set to the data type of the event table
     * being saved to the file. This is always single- or double-precision
     * floating point. Integer file saving is not supported.
     *
     * @see ::load()
     * @see ::save()
     */
    char fileDataType = 'F';

    /**
     * The widest parameter, in bytes.
     *
     * This value is only used during file loading. Before and after loading,
     * the field's value is undefined.
     *
     * @see ::load()
     */
    size_t fileMaxParameterBytes = 0L;

    /**
     * The narrowest parameter, in bytes.
     *
     * This value is only used during file loading. Before and after loading,
     * the field's value is undefined.
     *
     * @see ::load()
     */
    size_t fileMinParameterBytes = 0L;

    /**
     * The highest parameter range.
     *
     * This value is only used during file loading. Before and after loading,
     * the field's value is undefined.
     *
     * @see ::load()
     */
    long fileMaxParameterRange = 0L;

    /**
     * Enables auto-scaling on integer to floating-point upconvert on load.
     *
     * When true (the default), integer channel data is converted to
     * floating-point scale data by casting integers to floats or doubles,
     * and scaling them based upon parameter linear and logarithmic scaling
     * set using the $PnG and $PnE keywords.
     *
     * When false, integer channel data is still converted to floating-point,
     * but it is not scaled. The $PnG and $PnE keyword values are not used.
     * This is not recommended.
     *
     * If file data is already floating point, then $PnG and $PnE do not
     * apply and the data is already scaled. No auto-scaling is done.
     *
     * @see ::load()
     * @see ::isAutoScale()
     * @see ::setAutoScaling()
     * @see ::scaleParameter()
     * @see ::scaleAllParameters()
     */
    bool fileAutoScale = true;

    /**
     * The current file's attributes.
     *
     * During and after loading or saving a file, this contains name-value
     * pairs that describe the loaded or saved file. Prior to loading or
     * saving a file, this list is empty.
     *
     * Well-known keys are:
     * @li "byteOrder" ("lsbf" or "msbf") The file's byte order.
     * @li "dataType" ("float" or "double") The file's event data type.
     * @li "numberFormat" ("binary" or "text") The file's number format.
     * @li "numberOfEvents" (long) The number of events in the file.
     * @li "path" (string) The path to the most recently loaded or saved file.
     * @li "versionNumber" (string) The file version number.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getFileAttributes()
     */
    std::map<const std::string, const std::string> fileAttributes;



    // Dictionary of keyword-value pairs -------------------------------
    /**
     * A map of keyword-value pairs for the current file.
     *
     * Values are stored as UTF-8 strings. There is nothing in the FCS
     * file that indicates which keywords have string vs. numeric values,
     * so this class stores all values as strings. Conversion to numbers
     * is done on use.
     *
     * @see ::getDictionaryDouble()
     * @see ::getDictionaryLong()
     * @see ::getDictionaryString()
     * @see ::setDictionaryDouble()
     * @see ::setDictionaryLong()
     * @see ::setDictionaryString()
     */
    std::map<std::string, std::string> dictionary;



    // Event data ------------------------------------------------------
    /**
     * The event table.
     *
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    std::shared_ptr<FlowGate::Events::EventTableInterface> eventTable;



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
        extensions.push_back( "fcs" );
        extensions.push_back( "lmd" );
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
     * Constructs a new object with no parameters or events.
     *
     * The new object has has no parameters events, and empty dictionary.
     * These may be set by setting the event table or loading data from a file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );
    }



    // Copy constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given FileFCS object.
     *
     * @param[in] file
     *   The FileFCS object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( const FileFCS*const file )
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
        if ( file->eventTable != nullptr )
            this->copy( file->eventTable );
    }

    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given FileFCS object.
     *
     * @param[in] file
     *   The FileFCS object to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( const FileFCS& file )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        if ( file.eventTable != nullptr )
            this->copy( file.eventTable );
    }



    // Event table constructors ----------------------------------------
    /**
     * Constructs a new object using the given event table.
     *
     * The new object shares the given event table and its parameters
     * and events. External changes to the event table will affect this
     * object as well.
     *
     * @param[in] eventTable
     *   The event table to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( std::shared_ptr<FlowGate::Events::EventTableInterface>& eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Use.
        this->setEventTable( eventTable );
    }

    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given event table.
     *
     * <B>Performance note</B><BR>
     * Run time can be reduced by sharing an existing event table instead
     * of copying it. This may be appropriate when an event table is read
     * from one file, then written to another. To share an event table, use
     * the shared pointer form of this constructor.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( const FlowGate::Events::EventTableInterface*const eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( eventTable );
    }

    /**
     * Constructs a new object initialized with a copy of the parameters
     * and events in the given event table.
     *
     * <B>Performance note</B><BR>
     * Run time can be reduced by sharing an existing event table instead
     * of copying it. This may be appropriate when an event table is read
     * from one file, then written to another. To share an event table, use
     * the shared pointer form of this constructor.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( const FlowGate::Events::EventTableInterface& eventTable )
        noexcept
    {
        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Copy.
        this->copy( eventTable );
    }



    // Load constructors -----------------------------------------------
    /**
     * Constructs a new object initialized with data loaded from an FCS file.
     *
     * @param[in] path
     *   The file path to an FCS file to load.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or invalid.
     * @throws std::runtime_error
     *   Throws an exception if there is a problem parsing the file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( const char*const path )
    {
        // Validate.
        if ( path == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file path." );

        if ( std::strlen( path ) == 0 )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty file path." );

        // Initialize.
        this->reset( );
        this->setVerbose( false );
        this->setVerbosePrefix( DEFAULT_VERBOSE_PREFIX );

        // Load.
        this->load( std::string( path ) );
    }

    /**
     * Constructs a new object initialized with data loaded from an FCS file.
     *
     * After loading the file, further method calls will access the loaded
     * parameters, events, and dictionary.
     *
     * @param[in] path
     *   The file path to an FCS file to load.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or invalid.
     * @throws std::runtime_error
     *   Throws an exception if there is a problem parsing the file.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     */
    FileFCS( const std::string path )
    {
        // Validate.
        if ( path.empty( ) == 0 )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty file path." );

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
    virtual ~FileFCS( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
private:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Trims simple white space (blanks) from the start and end of a string.
     *
     * The given wide string is searched for simple spaces (not all other
     * types of Unicode spaces) at the start and end of the string. The
     * spaces are removed a new trimmed wide string is returned.
     *
     * @param[in] string
     *   The string to trim.
     *
     * @return
     *   Returns a new string with simple blanks trimmed.
     */
    std::wstring trim( const std::wstring& string )
        const noexcept
    {
        const size_t length = string.length( );

        // Search from the start for the first non-blank character.
        size_t startOfNonBlank = 0;
        while ( startOfNonBlank < length && string[startOfNonBlank] == ' ' )
            ++startOfNonBlank;

        if ( startOfNonBlank == length )
            return std::wstring( );

        // Search from the end for the first non-blank character.
        size_t endOfNonBlank = length - 1;
        while ( endOfNonBlank > startOfNonBlank && string[endOfNonBlank] == ' ' )
            --endOfNonBlank;

        return string.substr( startOfNonBlank, (endOfNonBlank - startOfNonBlank + 1) );
    }

    /**
     * Trims simple white space (blanks) from the start and end of a string.
     *
     * The given variable-byte UTF-8 string is converted to a wide character
     * Unicode string and searched for simple spaces (not all other types
     * of Unicode spaces) at the start and end of the string. The spaces are
     * removed a new trimmed UTF-8 string is returned.
     *
     * @param[in] string
     *   The string to trim.
     *
     * @return
     *   Returns a new string with simple blanks trimmed.
     */
    std::string trim( const std::string& string )
        const noexcept
    {
        std::wstring_convert<const std::codecvt_utf8<wchar_t>, wchar_t> unicode;
        const std::wstring wstring = unicode.from_bytes( string );
        return unicode.to_bytes( this->trim( wstring ) );
    }

    /**
     * Processes a string to escape a given delimiter, if present.
     *
     * The FCS specification defines that a single delimiter character is
     * used to mark the start and end of each value when stored in the
     * FCS dictionary in the text segments of a file. If the string uses
     * the delimiter, then it needs to be escaped by converting it to
     * a double-delimiter.
     *
     * The incoming UTF-8 string is converted to wide character Unicode
     * and searched for the delimiter. For each delimiter found, the
     * character is replaced with two delimiters. The updated string is
     * converted back to UTF-8 and returned.
     *
     * @param[in] wdelimiter
     *   The single-character delimiter.
     * @param[in] string
     *   The string to check and escape if the delimiter is used.
     *
     * @return
     *   Returns the escaped string.
     */
    std::string escapeDelimiter(
        const wchar_t wdelimiter,
        const std::string& string )
        const noexcept
    {
        // Convert the given string to wide character Unicode so that we
        // can safely search it for the delimiter.
        std::wstring_convert<const std::codecvt_utf8<wchar_t>, wchar_t> unicode;
        const std::wstring wstring = unicode.from_bytes( string );

        // Look for the delimiter. In most cases, there won't be one.
        size_t startOfSubstr = 0;
        size_t posOfDelimiter = wstring.find( wdelimiter, startOfSubstr );
        if ( posOfDelimiter == std::wstring::npos )
            return string;

        // The delimiter is present, so loop through the string and copy
        // it to a new string in pieces. At each delimiter, replace the
        // single delimiter with a double delimiter per FCS rules.
        std::wstring result;
        while ( posOfDelimiter != std::wstring::npos )
        {
            const size_t length = posOfDelimiter - startOfSubstr + 1;
            result += wstring.substr( startOfSubstr, length );
            result += wdelimiter;

            startOfSubstr = posOfDelimiter + 1;

            posOfDelimiter = wstring.find( wdelimiter, startOfSubstr );
        }

        result += wstring.substr( startOfSubstr );

        return unicode.to_bytes( result );
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
     * @li "byteOrder" ("lsbf" or "msbf") The byte order for binary numbers.
     * @li "dataType" ("int", "float", or "double") The data type for numbers.
     * @li "numberFormat" ("binary" or "text") The file's number format.
     * @li "numberOfEvents" (long) The number of events.
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



    // Delimiter -------------------------------------------------------
    /**
     * Returns the file keyword value delimiter character.
     *
     * When a file is loaded, this value is set to the delimiter used by
     * the file's dictionary. The delimiter marks the start and end of
     * a value after a keyword. The FCS file specification allows this
     * to be any single ASCI character and uses a slash ("/") in all examples.
     * Many FCS files use this same delimiter. Other common delimiters are
     * a vertical bar ("|") or a form feed (CTRL-L).
     *
     * After a file has been loaded, this method returns the delimiter used.
     * This is not particularly useful information, but it is available.
     *
     * When a file is saved, this value is used when writing keywords and
     * values to the new file.
     *
     * @return
     *   Returns the file keyword value delimiter.
     *
     * @see ::load()
     * @see ::setFileDictionaryValueDelimiter()
     */
    inline const char getFileDictionaryValueDelimiter( )
        const noexcept
    {
        return this->fileDictionaryValueDelimiter;
    }

    /**
     * Sets the file keyword value delimiter character.
     *
     * When a file is saved, this value is used in the file's dictionary to
     * mark the start and end of a value after a keyword. The FCS file
     * specification allows this to be any single ASCI character and uses a
     * slash ("/") in all examples. Many FCS files use this same delimiter.
     * Other common delimiters are a vertical bar ("|") or a form feed
     * (CTRL-L).
     *
     * The default delimiter is the slash ("/") character. Any ASCII character
     * may be used, except for the following:
     *
     * @li NULL ('\0') is reserved to mean end-of-string.
     *
     * @li Comma (',') is reserved for use as a value separator in keyword
     *     values that include lists (e.g. $SPILLOVER).
     *
     * @li All characters with the high bit set since these are not ASCII
     *     and several of these are reserved for Unicode encoding.
     *
     * @param[in] delimiter
     *   The file keyword value delimiter.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the delimiter is a NULL or comma.
     *
     * @see ::getFileDictionaryValueDelimiter()
     * @see ::save()
     */
    inline void setFileDictionaryValueDelimiter( const char delimiter )
    {
        if ( delimiter == '\0' )
            throw std::invalid_argument(
                "Invalid NULL keyword value delimiter." );
        if ( delimiter == ',' )
            throw std::invalid_argument(
                "Invalid comma keyword value delimiter." );
        if ( (int)delimiter < 0 )
            throw std::invalid_argument(
                "Invalid non-ASCII keyword value delimiter." );

        this->fileDictionaryValueDelimiter = delimiter;
    }



    // Paths -----------------------------------------------------------
    /**
     * Returns the original file name.
     *
     * The original file name contains the file name used when the data was
     * first acquired. This may be an institutional name that differs
     * from the current file name.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$FIL" keyword.
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Some institutions include a patient's name or identifier within the
     * file name for original files. This information will be present in
     * this field.
     *
     * De-identification may remove this value.
     *
     * @return
     *   Returns the name of the original file. If the name is not known,
     *   an empty string is returned.
     *
     * @see ::load()
     * @see ::setOriginalFileName()
     */
    inline std::string getOriginalFileName( )
        const noexcept
    {
        if ( this->inDictionary( "$FIL" ) == false )
            return std::string( );
        return this->getDictionaryString( "$FIL" );
    }



    /**
     * Sets the original file name.
     *
     * The original file name contains the file name used when the data was
     * first acquired. This may be an institutional name that differs
     * from the current file name.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$FIL" keyword.
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Some institutions include a patient's name or identifier within the
     * file name for original files. This information will be present in
     * this field.
     *
     * De-identification may remove this value.
     *
     * @param[in] fileName
     *   The name of the original file.
     *
     * @see ::getOriginalFileName()
     */
    inline void setOriginalFileName( const std::string& fileName )
        noexcept
    {
        if ( fileName.empty( ) == true )
            this->eraseDictionaryEntry( "$FIL" );
        else
            this->setDictionaryStringInternal(
                "$FIL",
                fileName,
                false,      // No need to convert keyword.
                true );     // Do trim value.
    }


    // Auto-scaling ----------------------------------------------------
    /**
     * Returns true if integer parameter auto-scaling is enabled.
     *
     * Auto-scaling automatically processes file data during a load to
     * convert it from raw "channel data" into "scale data" that is useful
     * for further processing.
     *
     * @return
     *   Returns true if auto-scaling of parameters is enabled for future loads.
     *
     * @see ::load()
     * @see ::scaleParameter()
     * @see ::scaleAllParameters()
     * @see ::setAutoScaling()
     * @see ::getParameterScaling()
     * @see ::getParameterRange()
     */
    inline bool isAutoScaling( )
        const noexcept
    {
        return this->fileAutoScale;
    }

    /**
     * Sets whether to apply auto-scaling to integer parameter data on load.
     *
     * The FCS file format supports the storage of parameter values as
     * integers of varying size, and single- and double-precision floating
     * point. Modern FCS files are typcially floating point, but legacy files
     * may contain integer data.
     *
     * Legacy integer data files were constructed to minimize the number of
     * bits used per integer by converting the original floating point values
     * to integer values by a linear or log gain. This creates "channel data"
     * stored to the FCS file. To restore the original floating point, the
     * channel data in the file needs to be rescaled by the inverse of this
     * gain to create "scale data" that is useful for further processing.
     *
     * Modern floating point data files may still use linear scaling, but not
     * log scaling. To restore values to their original range, the floating
     * point channel data in the file needs to be rescaled as above to create
     * "scale data".
     *
     * Data scaling is normally done automatically when a file is
     * loaded. However, auto-scaling can be disabled if the raw channel
     * data is needed or if the caller wishes to apply their own scaling.
     * The scaling factors used by auto-scaling are available from
     * getParameterScaling(). The integer value range also may be
     * needed from getParameterRange(). Scaling can be done for a parameter
     * using scaleParameter(), which gets and uses these values. And
     * scaling can be done for all parameters using scaleAllParameters(),
     * which is what auto-scaling does on file load.
     *
     * Auto-scaling is only applied on file load.
     *
     * @param[in] enable
     *   When true, enable auto-scaling on file load. When false, disable
     *   auto-scaling. Integer data is still upconverted to floating point
     *   event data, but no values are scaled.
     *
     * @see ::load()
     * @see ::isAutoScaling()
     * @see ::getParameterScaling()
     * @see ::getParameterRange()
     * @see ::scaleParameter()
     * @see ::scaleAllParameters()
     */
    inline void setAutoScaling( const bool enable )
        noexcept
    {
        this->fileAutoScale = enable;
    }
    // @}



//----------------------------------------------------------------------
// Dictionary cleaning.
//----------------------------------------------------------------------
public:
    /**
     * @name Dictionary cleaning
     */
    // @{
    /**
     * Cleans the dictionary of all keywords with the indicated category.
     *
     * @param[in] category
     *   The keyword category for keywords to remove.
     */
    void cleanByCategory( const uint32_t category )
        noexcept
    {
        std::vector<std::string> keywordsToRemove;

        // Loop over the dictionary and find all keywords to remove.
        // Add them to a list since deleting them during this loop
        // will confuse the iterators.
        uint32_t foundCat;
        for ( auto it = this->dictionary.cbegin( );
            it != this->dictionary.cend( ); ++it )
        {
            const auto keyword = (*it).first;
            try
            {
                foundCat = FCSVocabulary::find( keyword ).getCategory( );
            }
            catch ( ... )
            {
                // Not found. Default to OTHER.
                foundCat = FCSKeywordCategory::OTHER;
            }

            if ( foundCat == category )
                keywordsToRemove.push_back( keyword );
        }

        // Loop over the keywords to remove and remove them from the
        // dictionary.
        const auto n = keywordsToRemove.size( );
        for ( size_t i = 0; i < n; ++i )
            this->dictionary.erase( keywordsToRemove[i] );
    }

    /**
     * Cleans the dictionary of all keywords with the indicated flag(s) set.
     *
     * @param[in] flags
     *   The keyword attributes flags for keywords to remove.
     * @param[in] reverse
     *   (optional, default = false) When false, keywords are removed when
     *   they have any flag bit that matches the given flags. When true,
     *   keywords are removed when they DO NOT have any flag bit that
     *   matches the given flags.
     */
    void cleanByFlags( const uint32_t flags, const bool reverse = false )
        noexcept
    {
        std::vector<std::string> keywordsToRemove;

        // Loop over the dictionary and find all keywords to remove.
        // Add them to a list since deleting them during this loop
        // will confuse the iterators.
        for ( auto it = this->dictionary.cbegin( );
            it != this->dictionary.cend( ); ++it )
        {
            const auto keyword = (*it).first;

            try
            {
                auto attr = FCSVocabulary::find( keyword );
                if ( reverse == false )
                {
                    // Remove if flags do match.
                    if ( (attr.getFlags() & flags) != 0 )
                        keywordsToRemove.push_back( keyword );
                }
                else
                {
                    // Remove if flags do not match.
                    if ( (attr.getFlags() & flags) == 0 )
                        keywordsToRemove.push_back( keyword );
                }
            }
            catch ( ... )
            {
                // Not found.
                if ( reverse == true )
                    keywordsToRemove.push_back( keyword );
            }
        }

        // Loop over the keywords to remove and remove them from the
        // dictionary.
        const auto n = keywordsToRemove.size( );
        for ( size_t i = 0; i < n; ++i )
            this->dictionary.erase( keywordsToRemove[i] );
    }

    /**
     * De-identifies the content by removing potentially protected health
     * information.
     *
     * Privacy regulations, such as the U.S. HIPAA regulations, place
     * strict limits on the distribution of protected health information.
     * Flow Cytometry data collected in a clinical setting may include
     * dictionary values that could include a patient's name, identification,
     * or other information. De-identification removes or clears the value
     * for all dictionary entries that could contain personal information.
     *
     * This method performs the following operations:
     *
     * @li Remove all dictionary keywords known to be used or usable to
     *     store a patient's personal health information.
     *
     * @li Remove all dictionary keywords known to be used or usable to
     *     store the names, addresses, or email addresses of individuals,
     *     departments, or institutions operating the equipment or software
     *     or in charge of their use.
     *
     * @li Remove all dictionary keywords known to be used or usable to
     *     store dates and times, such as for the acquisition of the data
     *     or for later processing of the data.
     *
     * @li Remove all dictionary keywords that are not recognized or that
     *     are not known well enough to be safely out of scope from the
     *     above exclusions (e.g. known to not include patient, user, or
     *     date/time information).
     *
     * @see cleanByFlags()
     * @see cleanByCategory()
     */
    void deidentify( )
        noexcept
    {
        this->cleanByFlags(
            FCSKeyword::PERSONAL_DATA |
            FCSKeyword::USER_DATA |
            FCSKeyword::DATE_DATA );
        this->cleanByCategory( FCSKeywordCategory::OTHER );
    }
    // @}



//----------------------------------------------------------------------
// General-purpose dictionary access.
//----------------------------------------------------------------------
public:
    /**
     * @name General-purpose dictionary access
     */
    // @{
    /**
     * Returns a vector containing all current dictionary keywords.
     *
     * @return
     *   Returns a vector of keywords.
     *
     * @see ::getDictionaryDouble()
     * @see ::getDictionaryLong()
     * @see ::getDictionaryString()
     */
    inline std::vector<std::string> getDictionaryKeywords( )
        const noexcept
    {
        std::vector<std::string> keywords;
        keywords.reserve( this->dictionary.size( ) );

        for ( auto it = this->dictionary.cbegin( );
            it != this->dictionary.cend( ); ++it )
            keywords.push_back( (*it).first );

        return keywords;
    }

    /**
     * Returns a dictionary keyword's value parsed as a floating point number.
     *
     * Dictionary keyword values are stored as strings. Some keyword values
     * may be parsed as floating point numbers, integers, or as more complex
     * values. It is up to the caller to select the appropriate method to
     * parse a keyword's value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the double value of the keyword. If the keyword's value is
     *   empty, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the keyword is not found.
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer or if
     *   the keyword's value cannot be parsed as a double.
     *
     * @see ::inDictionary()
     * @see FCSKeyword::hasDoubleValue()
     */
    inline double getDictionaryDouble( const char*const keyword )
        const
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        return this->getDictionaryDouble( std::string( keyword ) );
    }

    /**
     * Returns a dictionary keyword's value parsed as a floating point number.
     *
     * Dictionary keyword values are stored as strings. Some keyword values
     * may be parsed as floating point numbers, integers, or as more complex
     * values. It is up to the caller to select the appropriate method to
     * parse a keyword's value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the double value of the keyword. If the keyword's value is
     *   empty, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the keyword is not found.
     * @throws std::invalid_argument
     *   Throws an exception if the keyword's value cannot be parsed as
     *   a double.
     *
     * @see ::inDictionary()
     * @see FCSKeyword::hasDoubleValue()
     */
    inline double getDictionaryDouble( const std::string& keyword )
        const
    {
        try
        {
            const std::string value = this->dictionary.at( keyword );
            if ( value.empty( ) == true )
                return 0.0;
            return std::stod( value );
        }
        catch ( const std::invalid_argument& e )
        {
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "The keyword \"" ) + keyword +
                std::string( "\" has a string value \"" ) +
                this->dictionary.at( keyword ) +
                std::string( "\" that cannot be parsed as a double." ) );
        }
    }

    /**
     * Returns a dictionary keyword's value parsed as an integer number.
     *
     * Dictionary keyword values are stored as strings. Some keyword values
     * may be parsed as floating point numbers, integers, or as more complex
     * values. It is up to the caller to select the appropriate method to
     * parse a keyword's value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the long integer value of the keyword. If the keyword's
     *   value is empty, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the keyword is not found.
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer or if
     *   the keyword's value cannot be parsed as a long integer.
     *
     * @see ::inDictionary()
     * @see FCSKeyword::hasLongValue()
     */
    inline long getDictionaryLong( const char*const keyword )
        const
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        return this->getDictionaryLong( std::string( keyword ) );
    }

    /**
     * Returns a dictionary keyword's value parsed as an integer number.
     *
     * Dictionary keyword values are stored as strings. Some keyword values
     * may be parsed as floating point numbers, integers, or as more complex
     * values. It is up to the caller to select the appropriate method to
     * parse a keyword's value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the long integer value of the keyword. If the keyword's
     *   value is empty, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the keyword is not found.
     * @throws std::invalid_argument
     *   Throws an exception if the keyword's value cannot be parsed as
     *   an integer.
     *
     * @see ::inDictionary()
     * @see FCSKeyword::hasLongValue()
     */
    inline long getDictionaryLong( const std::string& keyword )
        const
    {
        try
        {
            const std::string value = this->dictionary.at( keyword );
            if ( value.empty( ) == true )
                return 0L;
            return std::stol( value );
        }
        catch ( const std::invalid_argument& e )
        {
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "The keyword \"" ) + keyword +
                std::string( "\" has a string value \"" ) +
                this->dictionary.at( keyword ) +
                std::string( "\" that cannot be parsed as an integer." ) );
        }
    }

    /**
     * Returns a dictionary keyword's value as a string.
     *
     * Dictionary keyword values are stored as strings. Some keyword values
     * may be parsed as floating point numbers, integers, or as more complex
     * values. It is up to the caller to select the appropriate method to
     * parse a keyword's value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the string value of the keyword.
     *
     * @throws std::out_of_range
     *   Throws an exception if the keyword is not found.
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer.
     *
     * @see ::inDictionary()
     * @see FCSKeyword::hasStringValue()
     */
    inline std::string getDictionaryString( const char*const keyword )
        const
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        return this->getDictionaryString( std::string( keyword ) );
    }

    /**
     * Returns a dictionary keyword's value as a string.
     *
     * Dictionary keyword values are stored as strings. Some keyword values
     * may be parsed as floating point numbers, integers, or as more complex
     * values. It is up to the caller to select the appropriate method to
     * parse a keyword's value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns the string value of the keyword.
     *
     * @throws std::out_of_range
     *   Throws an exception if the keyword is not found.
     *
     * @see ::inDictionary()
     * @see FCSKeyword::hasStringValue()
     */
    inline std::string getDictionaryString( const std::string& keyword )
        const
    {
        return this->dictionary.at( keyword );
    }



    /**
     * Erases the dictionary entry for the keyword.
     *
     * If there is no entry for the keyword, this method returns immediately.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer.
     */
    inline void eraseDictionaryEntry( const char*const keyword )
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        if ( this->dictionary.count( keyword ) == 1 )
            this->dictionary.erase( keyword );
    }

    /**
     * Erases the dictionary entry for the keyword.
     *
     * If there is no entry for the keyword, this method returns immediately.
     *
     * @param[in] keyword
     *   The keyword to look up.
     */
    inline void eraseDictionaryEntry( const std::string& keyword )
        noexcept
    {
        if ( this->dictionary.count( keyword ) == 1 )
            this->dictionary.erase( keyword );
    }



    /**
     * Returns true if the indicated keyword has a value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns true if the keyword is found, and false otherwise.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer.
     */
    inline bool inDictionary( const char*const keyword )
        const
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        return (this->dictionary.count( keyword ) == 1);
    }

    /**
     * Returns true if the indicated keyword has a value.
     *
     * @param[in] keyword
     *   The keyword to look up.
     *
     * @return
     *   Returns true if the keyword is found, and false otherwise.
     */
    inline bool inDictionary( const std::string& keyword )
        const noexcept
    {
        return (this->dictionary.count( keyword ) == 1);
    }



    /**
     * Sets the double value of the indicated keyword.
     *
     * The given value is converted to a string and stored in the dictionary.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most numeric keywords, a zero
     *   value indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer, or if the
     *   keyword cannot be set.
     */
    inline void setDictionaryDouble(
        const char*const keyword,
        const double value )
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        // Route setting through a common method.
        this->setDictionaryStringInternal(
            std::string( keyword ),
            std::to_string( value ),
            true,       // Do convert the keyword to uppercase and trim.
            false );    // No need to convert the value.
    }

    /**
     * Sets the double value of the indicated keyword.
     *
     * The given value is converted to a string and stored in the dictionary.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most numeric keywords, a zero
     *   value indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword cannot be set.
     */
    inline void setDictionaryDouble(
        const std::string& keyword,
        const double value )
    {
        // Route setting through a common method.
        this->setDictionaryStringInternal(
            keyword,
            std::to_string( value ),
            true,       // Do convert the keyword to uppercase and trim.
            false );    // No need to convert the value.
    }

    /**
     * Sets the long value of the indicated keyword.
     *
     * The given value is converted to a string and stored in the dictionary.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most numeric keywords, a zero
     *   value indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer, or if the
     *   keyword cannot be set.
     */
    inline void setDictionaryLong(
        const char*const keyword,
        const long value )
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        // Route setting through a common method.
        this->setDictionaryStringInternal(
            std::string( keyword ),
            std::to_string( value ),
            true,       // Do convert the keyword to uppercase and trim.
            false );    // No need to convert the value.
    }

    /**
     * Sets the long value of the indicated keyword.
     *
     * The given value is converted to a string and stored in the dictionary.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most numeric keywords, a zero
     *   value indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword cannot be set.
     */
    inline void setDictionaryLong(
        const std::string& keyword,
        const long value )
    {
        // Route setting through a common method.
        this->setDictionaryStringInternal(
            keyword,
            std::to_string( value ),
            true,       // Do convert the keyword to uppercase and trim.
            false );    // No need to convert the value.
    }

    /**
     * Sets the string value of the indicated keyword.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     * @li "$PnN" = the short name of a parameter.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most keywords, an empty string
     *   indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword cannot be set.
     */
    inline void setDictionaryString(
        const std::string& keyword,
        const std::string& value )
    {
        // This method is used by all methods that set a keyword and value.
        // Watch for keywords that cannot be set.
        if ( keyword == "$TOT" )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "The \"$TOT\" dictionary keyword value cannot be changed." );
        if ( keyword == "$PAR" )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "The \"$PAR\" dictionary keyword value cannot be changed." );

        const auto genericKeyword = findGenericFormOfIndexedKeyword( keyword );
        if ( genericKeyword == "$PnN" )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "The \"$PnN\" dictionary keyword value cannot be changed." );

        this->setDictionaryStringInternal(
            keyword,
            value,
            true,       // Do convert the keyword to uppercase and trim.
            true );     // Do trim the value.
    }

private:
    /**
     * Sets the string value of the indicated keyword.
     *
     * This is an internal method that does not check for setting special
     * keywords.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most keywords, an empty string
     *   indicates that no specific value is in use.
     * @param[in] convertKeyword
     *   (optional, default = true) When true, the keyword is converted to
     *   upper case and trimmed of white space. When false, this is skipped.
     *   Setting this to false is typical for internal code that is already
     *   sure that the keyword is upper case and without leading or trailing
     *   white space.
     * @param[in] convertValue
     *   (optional, default = true) When true, the value is trimmed of white
     *   space. When false, this is skipped. Setting this to false is typical
     *   for internal code that is already sure that the value is without
     *   leading or trailing white space.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword cannot be set.
     */
    inline void setDictionaryStringInternal(
        const std::string& keyword,
        const std::string& value,
        const bool convertKeyword = true,
        const bool convertValue = true )
    {
        // A common case requires no keyword or value conversion.
        if ( convertKeyword == false && convertValue == false )
        {
            this->dictionary[keyword] = value;
            return;
        }

        // Convert the keyword and value.
        std::string convertedKeyword = keyword;
        std::string convertedValue = value;

        if ( convertKeyword == true )
        {
            std::transform(
                convertedKeyword.begin( ),
                convertedKeyword.end( ),
                convertedKeyword.begin( ),
                [](unsigned char c){ return std::toupper(c); } );
            convertedKeyword = this->trim( convertedKeyword );
        }

        if ( convertValue == true )
            convertedValue = this->trim( convertedValue );

        this->dictionary[convertedKeyword] = convertedValue;
    }

public:
    /**
     * Sets the string value of the indicated keyword.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most keywords, an empty string
     *   indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer, or if the
     *   keyword cannot be set.
     */
    inline void setDictionaryString(
        const char*const keyword,
        const std::string& value )
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        // Route setting through a common method.
        this->setDictionaryStringInternal(
            std::string( keyword ),
            value,
            true,       // Do convert the keyword to uppercase and trim.
            true );     // Do trim the value.
    }

    /**
     * Sets the string value of the indicated keyword.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most keywords, an empty string
     *   indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword cannot be set.
     */
    inline void setDictionaryString(
        const std::string& keyword,
        const char*const value )
    {
        // Route setting through a common method.
        if ( value == nullptr )
            this->setDictionaryStringInternal(
                keyword,
                std::string( ),
                true,       // Do convert the keyword to uppercase and trim.
                false );    // No need to convert the value.
        else
            this->setDictionaryStringInternal(
                keyword,
                std::string( value ),
                true,       // Do convert the keyword to uppercase and trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the string value of the indicated keyword.
     *
     * Some keywords are special and cannot be set directly:
     * @li "$TOT" = the number of events.
     * @li "$PAR" = the number of parameters.
     *
     * @param[in] keyword
     *   The keyword to set.
     * @param[in] value
     *   The value for the keyword. For most keywords, an empty string
     *   indicates that no specific value is in use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the keyword is a NULL pointer, or if the
     *   keyword cannot be set.
     */
    inline void setDictionaryString(
        const char*const keyword,
        const char*const value )
    {
        if ( keyword == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL keyword." );

        // Route setting through a common method.
        if ( value == nullptr )
            this->setDictionaryStringInternal(
                std::string( keyword ),
                std::string( ),
                true,       // Do convert the keyword to uppercase and trim.
                false );    // No need to convert the value.
        else
            this->setDictionaryStringInternal(
                std::string( keyword ),
                std::string( value ),
                true,       // Do convert the keyword to uppercase and trim.
                true );     // Do trim the value.
    }
    // @}



//----------------------------------------------------------------------
// General documentation.
//----------------------------------------------------------------------
public:
    /**
     * @name General documentation
     */
    // @{
    /**
     * Returns the comment text.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$COM" keyword.
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Institutions may use the comment text to include a patient's name or
     * identifier.
     *
     * De-identification may remove this value.
     *
     * @return
     *   Returns the optional comment text. If the comment is not set,
     *   an empty string is returned.
     *
     * @see ::getProjectName()
     * @see ::setComment()
     */
    inline std::string getComment( )
        const noexcept
    {
        if ( this->inDictionary( "$COM" ) == false )
            return std::string( );
        return this->getDictionaryString( "$COM" );
    }

    /**
     * Returns the project name.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PROJ" keyword.
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Institutions may use the project name to include a patient's name or
     * identifier.
     *
     * De-identification may remove this value.
     *
     * @return
     *   Returns the name of the project for which data was acquired.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getComment()
     * @see ::setProjectName()
     */
    inline std::string getProjectName( )
        const noexcept
    {
        if ( this->inDictionary( "$PROJ" ) == false )
            return std::string( );
        return this->getDictionaryString( "$PROJ" );
    }



    /**
     * Sets the comment text.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the text to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$COM" keyword.
     *
     * @param[in] text
     *   The comment text.
     *
     * @see ::getComment()
     * @see ::setProjectName()
     */
    inline void setComment( const std::string& text )
        noexcept
    {
        if ( text.empty( ) == true )
            this->eraseDictionaryEntry( "$COM" );
        else
            this->setDictionaryStringInternal(
                "$COM",
                text,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the project name.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the text to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$PROJ" keyword.
     *
     * @param[in] name
     *   The name of the project.
     *
     * @see ::getProjectName()
     * @see ::setComment()
     */
    inline void setProjectName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$PROJ" );
        else
            this->setDictionaryStringInternal(
                "$PROJ",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }
    // @}



//----------------------------------------------------------------------
// Device documentation.
//----------------------------------------------------------------------
public:
    /**
     * @name Device documentation
     */
    // @{
    /**
     * Returns the name of the computer system used to acquire the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$SYS" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$SYS" keyword was introduced in the 3.0 FCS specification.
     *
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "Intel64 Family 6 Model 23 Stepping 10, GenuineIntel, Microsoft Windows NT 6.1.7600.0"
     * @li "Microsoft Windows NT 5.1.2600 Service Pack 3"
     * @li "Microsoft Windows XP Professonal Service Pack 2 (Build 2600)"
     * @li "Summit V5.2.0.7477 / Windows NT Version 5.1 / Intel Processor"
     * @li "Windows XP 5.1"
     * @li "WindowsXPE"
     * @li "windows xp"
     *
     * Some vendors define additional non-standard keywords that provide
     * further information about the system used:
     * @li "ACQ. SOFTWARE"
     * @li "APPLICATION"
     * @li "CREATED_BY"
     * @li "CREATOR"
     * @li "FJ_FCS_VERSION"
     * @li "SOFTWARE"
     * @li "SOFTWAREREVISION"
     *
     * @return
     *   Returns the name of the computer system used to collect the data.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getCytometerName()
     * @see ::getCytometerSerialNumber()
     * @see ::setSystemName()
     */
    inline std::string getSystemName( )
        const noexcept
    {
        if ( this->inDictionary( "$SYS" ) == false )
            return std::string( );
        return this->getDictionaryString( "$SYS" );
    }

    /**
     * Returns the name of the cytometer used to acquire the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$CYT" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$CYT" keyword has been defined in all versions of the FCS
     * specification. Some vendors have used the keyword to store the name
     * of the software used, rather than the name of the device.
     *
     * Some vendors define additional non-standard keywords that provide
     * further information about the cytometer and/or software used:
     * @li "ACQ. SOFTWARE"
     * @li "APPLICATION"
     * @li "CREATED_BY"
     * @li "CREATOR"
     * @li "CYTOMETER CONFIG NAME"
     * @li "CYTOMETER CONFIGURATION NAME"
     * @li "FIRMWAREVERSION"
     * @li "FJ_FCS_VERSION"
     * @li "SOFTWARE"
     * @li "SOFTWAREREVISION"
     *
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "Accuri C6"
     * @li "Attune Cytometric Software v1.0"
     * @li "CyAn offline"
     * @li "Cytek DxP10: nickname"
     * @li "Eclipse Analyzer"
     * @li "FACSariaII"
     * @li "GemStone"
     * @li "MACSQuant"
     * @li "MoFlo Astrios"
     * @li "MoFlo XDP"
     * @li "inFlux v7 Sorter"
     *
     * @return
     *   Returns the name of the flow cytometer used to collect the data.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getCytometerSerialNumber()
     * @see ::getSystemName()
     * @see ::setCytometerName()
     */
    inline std::string getCytometerName( )
        const noexcept
    {
        if ( this->inDictionary( "$CYT" ) == false )
            return std::string( );
        return this->getDictionaryString( "$CYT" );
    }

    /**
     * Returns the version number of the cytometer used to acquire the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$CYTSN" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$CYTSN" keyword was introduced in the 3.0 FCS specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * further information about the cytometer used:
     * @li "CONNECTION"
     * @li "CYTNUM"
     * @li "CYTOMETER CONFIG NAME"
     * @li "CYTOMETER CONFIGURATION NAME"
     * @li "FIRMWAREVERSION"
     *
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "00-00033"
     * @li "100A0N04070001"
     * @li "2086"
     * @li "A01-0013"
     *
     * @return
     *   Returns the serial number of the flow cytometer used to collect
     *   the data. If the number is not known, an empty string is returned.
     *
     * @see ::getCytometerName()
     * @see ::getSystemName()
     * @see ::setCytometerSerialNumber()
     */
    inline std::string getCytometerSerialNumber( )
        const noexcept
    {
        if ( this->inDictionary( "$CYTSN" ) == false )
            return std::string( );
        return this->getDictionaryString( "$CYTSN" );
    }



    /**
     * Sets the name of the computer system used to acquire the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$SYS" keyword.
     *
     * @param[in] name
     *   The name of the computer used.
     *
     * @see ::getSystemName()
     * @see ::setCytometerName()
     * @see ::setCytometerSerialNumber()
     */
    inline void setSystemName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$SYS" );
        else
            this->setDictionaryStringInternal(
                "$SYS",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the name of the cytometer used to acquire the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$CYT" keyword.
     *
     * @param[in] name
     *   The name of the flow cytometer used.
     *
     * @see ::getCytometerName()
     * @see ::setCytometerSerialNumber()
     * @see ::setSystemName()
     */
    inline void setCytometerName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$CYT" );
        else
            this->setDictionaryStringInternal(
                "$CYT",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the acquisition device serial number.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$CYTSN" keyword.
     *
     * @param[in] name
     *   The serial number of the flow cytometer used.
     *
     * @see ::getCytometerSerialNumber()
     * @see ::setCytometerName()
     * @see ::setSystemName()
     */
    inline void setCytometerSerialNumber( const std::string& number )
        noexcept
    {
        if ( number.empty( ) == true )
            this->eraseDictionaryEntry( "$CYTSN" );
        else
            this->setDictionaryStringInternal(
                "$CYTSN",
                number,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }
    // @}



//----------------------------------------------------------------------
// Data source documentation.
//----------------------------------------------------------------------
public:
    /**
     * @name Data source documentation
     */
    // @{
    /**
     * Returns the type of cells, or other objects, measured.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$CELLS" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$CELLS" keyword has been defined in all versions of the FCS
     * specification.
     *
     * @return
     *   Returns the name of the type of cells, or other objects, measured.
     *   If the value is not known, an empty string is returned.
     *
     * @see ::getSourcePlateID()
     * @see ::getSourcePlateName()
     * @see ::getSourceID()
     * @see ::getSourceVolume()
     * @see ::getSourceName()
     * @see ::getSourceWellID()
     * @see ::setSourcePlateName()
     */
    inline std::string getSourceCellType( )
        const noexcept
    {
        if ( this->inDictionary( "$CELLS" ) == false )
            return std::string( );
        return this->getDictionaryString( "$CELLS" );
    }

    /**
     * Returns the source plate ID.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PLATEID" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$PLATEID" keyword was introduced in the 3.1 FCS specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information:
     * @li "ASSAY ID"
     * @li "PLATE ID"
     * @li "RUNID"
     *
     * @return
     *   Returns the ID of the plate from which a well was the
     *   source of the data. If the ID is not known, an empty string
     *   is returned.
     *
     * @see ::getSourceCellType()
     * @see ::getSourcePlateName()
     * @see ::getSourceID()
     * @see ::getSourceVolume()
     * @see ::getSourceName()
     * @see ::getSourceWellID()
     * @see ::setSourcePlateID()
     */
    inline std::string getSourcePlateID( )
        const noexcept
    {
        if ( this->inDictionary( "$PLATEID" ) == false )
            return std::string( );
        return this->getDictionaryString( "$PLATEID" );
    }

    /**
     * Returns the source plate name.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PLATENAME" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$PLATENAME" keyword was introduced in the 3.1 FCS specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information. Keywords seen in FCS files:
     * @li "PLATE NAME"
     * @li "PANEL NAME"
     *
     * @return
     *   Returns the name of the plate from which a well was the
     *   source of the data. If the name is not known, an empty string
     *   is returned.
     *
     * @see ::getSourceCellType()
     * @see ::getSourcePlateID()
     * @see ::getSourceID()
     * @see ::getSourceVolume()
     * @see ::getSourceName()
     * @see ::getSourceWellID()
     * @see ::setSourcePlateName()
     */
    inline std::string getSourcePlateName( )
        const noexcept
    {
        if ( this->inDictionary( "$PLATENAME" ) == false )
            return std::string( );
        return this->getDictionaryString( "$PLATENAME" );
    }

    /**
     * Returns the source well ID.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$WELLID" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$WELLID" keyword was introduced in the 3.1 FCS specification.
     *
     * See also "$SMNO", returned by getSourceID(), which may contain
     * a tube or well number.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information:
     * @li "WELL ID"
     *
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "A01"
     * @li "C07"
     *
     * @return
     *   Returns the location on the plate of the well that was the source
     *   of the data. If the ID is not known, an empty string is returned.
     *
     * @see ::getSourceCellType()
     * @see ::getSourcePlateID()
     * @see ::getSourcePlateName()
     * @see ::getSourceID()
     * @see ::getSourceVolume()
     * @see ::getSourceName()
     * @see ::setSourceWellID()
     */
    inline std::string getSourceWellID( )
        const noexcept
    {
        if ( this->inDictionary( "$WELLID" ) == false )
            return std::string( );
        return this->getDictionaryString( "$WELLID" );
    }

    /**
     * Returns the name of the source of the specimen, such as a patient name.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$SRC" keyword.
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Institutions may use the source name to include a patient's name or
     * identifier.
     *
     * De-identification may remove this value.
     *
     *
     * <B>History</B><BR>
     * The "$SRC" keyword has been defined in all versions of the FCS
     * specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information:
     * @li "CASE NUMBER"
     * @li "PATIENT ID"
     * @li "SAMPLE ID"
     * @li "SAMPLE NAME"
     * @li "SAMPLEID"
     * @li "SAMPLE_NAME"
     *
     * @return
     *   Returns the name of the source from which the data came.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getSourceCellType()
     * @see ::getSourcePlateID()
     * @see ::getSourcePlateName()
     * @see ::getSourceID()
     * @see ::getSourceVolume()
     * @see ::getSourceWellID()
     * @see ::setSourceName()
     */
    inline std::string getSourceName( )
        const noexcept
    {
        if ( this->inDictionary( "$SRC" ) == false )
            return std::string( );
        return this->getDictionaryString( "$SRC" );
    }

    /**
     * Returns the source sample volume, in nanoliters.
     *
     * The value indicates the quantity of the sample that was consumed
     * during data acquisition.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$VOL" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$VOL" keyword was introduced in the 3.1 FCS specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information:
     * @li "VOL"
     *
     * @return
     *   Returns the volume of the sample consumed. If the volume is not
     *   known, a zero is returned.
     *
     * @see ::getSourceCellType()
     * @see ::getSourcePlateID()
     * @see ::getSourcePlateName()
     * @see ::getSourceID()
     * @see ::getSourceName()
     * @see ::getSourceWellID()
     * @see ::setSourceVolume()
     */
    inline double getSourceVolume( )
        const noexcept
    {
        if ( this->inDictionary( "$VOL" ) == false )
            return 0.0;
        return this->getDictionaryDouble( "$VOL" );
    }

    /**
     * Returns the ID of the source of the specimen.
     *
     * The ID may indicate the sample, specimen, tube, well, or similar.
     * The ID in an FCS file has no well-known format or recommended practice
     * for its content. The ID's meaning may be instutition-dependent.
     *
     * This method returns the dictionary value of the "$SMNO" keyword.
     * The keyword is an abbrevation of "Specimen Number".
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Institutions may use the comment text to include a patient's name or
     * identifier.
     *
     * De-identification may remove this value.
     *
     *
     * <B>History</B><BR>
     * The "$SMNO" keyword has been defined in all versions of the FCS
     * specification.
     *
     * @return
     *   Returns the specimen name of the sample from which the data came.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getSourceCellType()
     * @see ::getSourcePlateID()
     * @see ::getSourcePlateName()
     * @see ::getSourceVolume()
     * @see ::getSourceName()
     * @see ::getSourceWellID()
     * @see ::setSourceID()
     */
    inline std::string getSourceID( )
        const noexcept
    {
        if ( this->inDictionary( "$SMNO" ) == false )
            return std::string( );
        return this->getDictionaryString( "$SMNO" );
    }



    /**
     * Sets the type of cells, or other objects, measured.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the type to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$CELLS" keyword.
     *
     * @param[in] type
     *   The name of the type of cells, or other objects, measured.
     *
     * @see ::getSourceCellType()
     * @see ::setSourcePlateName()
     * @see ::setSourcePlateID()
     * @see ::setSourceID()
     * @see ::setSourceVolume()
     * @see ::setSourceName()
     * @see ::setSourceWellID()
     */
    inline void setSourceCellType( const std::string& type )
        noexcept
    {
        if ( type.empty( ) == true )
            this->eraseDictionaryEntry( "$CELLS" );
        else
            this->setDictionaryStringInternal(
                "$CELLS",
                type,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the source plate ID.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the value to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$PLATEID" keyword.
     *
     * @param[in] id
     *   The ID of the plate from which a well was the source of the data.
     *
     * @see ::getSourcePlateID()
     * @see ::setSourceCellType()
     * @see ::setSourcePlateName()
     * @see ::setSourceID()
     * @see ::setSourceVolume()
     * @see ::setSourceName()
     * @see ::setSourceWellID()
     */
    inline void setSourcePlateID( const std::string& id )
        noexcept
    {
        if ( id.empty( ) == true )
            this->eraseDictionaryEntry( "$PLATEID" );
        else
            this->setDictionaryStringInternal(
                "$PLATEID",
                id,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the source plate name.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$PLATENAME" keyword.
     *
     * @param[in] name
     *   The name of the plate from which a well was the source of the data.
     *
     * @see ::getSourcePlateName()
     * @see ::setSourceCellType()
     * @see ::setSourcePlateID()
     * @see ::setSourceID()
     * @see ::setSourceVolume()
     * @see ::setSourceName()
     * @see ::setSourceWellID()
     */
    inline void setSourcePlateName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$PLATENAME" );
        else
            this->setDictionaryStringInternal(
                "$PLATENAME",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the source well ID.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the value to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$WELLID" keyword.
     *
     * @param[in] id
     *   The ID of the location on the plate for the source of the data.
     *
     * @see ::getSourceWellID()
     * @see ::setSourceCellType()
     * @see ::setSourcePlateID()
     * @see ::setSourcePlateName()
     * @see ::setSourceID()
     * @see ::setSourceVolume()
     * @see ::setSourceName()
     */
    inline void setSourceWellID( const std::string& id )
        noexcept
    {
        if ( id.empty( ) == true )
            this->eraseDictionaryEntry( "$WELLID" );
        else
            this->setDictionaryStringInternal(
                "$WELLID",
                id,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the source name.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$SRC" keyword.
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Institutions may use the source name to include a patient's name or
     * identifier.
     *
     * De-identification may remove this value.
     *
     * @param[in] name
     *   The name of the source.
     *
     * @see ::getSourceName()
     * @see ::setSourceCellType()
     * @see ::setSourcePlateID()
     * @see ::setSourcePlateName()
     * @see ::setSourceID()
     * @see ::setSourceVolume()
     * @see ::setSourceWellID()
     */
    inline void setSourceName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$SRC" );
        else
            this->setDictionaryStringInternal(
                "$SRC",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the source sample volume, in nanoliters.
     *
     * The value indicates the quantity of the sample that was consumed
     * during data acquisition.
     *
     * Setting the value to zero deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$VOL" keyword.
     *
     * @param[in] value
     *   The volume of the sample.
     *
     * @see ::getSourceVolume()
     * @see ::setSourceCellType()
     * @see ::setSourcePlateID()
     * @see ::setSourcePlateName()
     * @see ::setSourceID()
     * @see ::setSourceName()
     * @see ::setSourceWellID()
     */
    inline void setSourceVolume( const double value )
        noexcept
    {
        if ( value == 0.0 )
            this->eraseDictionaryEntry( "$VOL" );
        else
            this->setDictionaryStringInternal(
                "$VOL",
                std::to_string( value ),
                false,      // No need to convert keyword or trim.
                false );    // No need to trim the value.
    }

    /**
     * Sets the source specimen name.
     *
     * The name may indicate the sample, specimen, tube, or similar.
     * The specimen name in an FCS file has no well-known format or
     * recommended practice for its content. The specimen name's
     * meaning may be instutition-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$SMNO" keyword.
     *
     *
     * <B>Protected health information</B><BR>
     * This value may contain protected health information, such as that
     * regulated by U.S. HIPAA privacy regulations.
     *
     * Institutions may use the source ID to include a patient's name or
     * identifier.
     *
     * De-identification may remove this value.
     *
     * @param[in] name
     *   The name of the specimen from which the data came.
     *
     * @see ::getSourceID()
     * @see ::setSourceCellType()
     * @see ::setSourcePlateID()
     * @see ::setSourcePlateName()
     * @see ::setSourceVolume()
     * @see ::setSourceName()
     * @see ::setSourceWellID()
     */
    inline void setSourceID( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$SMNO" );
        else
            this->setDictionaryStringInternal(
                "$SMNO",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }
    // @}



//----------------------------------------------------------------------
// Data history documentation.
//----------------------------------------------------------------------
public:
    /**
     * @name Data history documentation
     */
    // @{
    /**
     * Returns the data acquisition end time.
     *
     * The FCS specification requires that the time have the format
     * "hh:mm:ss" and optionally ":tt" for fractions of a second. Some
     * vendors omit the fractions of a second, and some use a period (".")
     * separator instead of a colon before the fractions of a second.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$ETIM" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$ETIM" keyword has been defined in all versions of the FCS
     * specification.
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "07:32:13:00"
     * @li "11:18:35.67"
     * @li "11:29:32:378"
     * @li "12:07:14"
     *
     * @return
     *   Returns the clock time at which data acquisition ended. The
     *   returned string has the form hh:mm:ss and possibly :cc or .cc for
     *   fractional seconds. If the time is not known, an empty string
     *   is returned.
     *
     * @see ::getStartDate()
     * @see ::getStartTime()
     * @see ::setEndTime()
     */
    inline std::string getEndTime( )
        const noexcept
    {
        if ( this->inDictionary( "$ETIM" ) == false )
            return std::string( );
        return this->getDictionaryString( "$ETIM" );
    }

    /**
     * Returns the name of the institution acquiring the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$INST" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$INST" keyword has been defined in all versions of the FCS
     * specification.
     *
     * @return
     *   Returns the name of the institution acquiring the data.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getInvestigatorName()
     * @see ::getOperatorName()
     * @see ::setInstitutionName()
     */
    inline std::string getInstitutionName( )
        const noexcept
    {
        if ( this->inDictionary( "$INST" ) == false )
            return std::string( );
        return this->getDictionaryString( "$INST" );
    }

    /**
     * Returns the name of the investigator directing acquisition of
     * the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$EXP" keyword.
     * The keyword is an abbreviation of "Experiment".
     *
     *
     * <B>History</B><BR>
     * The "$EXP" keyword has been defined in all versions of the FCS
     * specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information:
     * @li "DEPARTMENT"
     * @li "DIRECTOR"
     * @li "EXPORT USER NAME"
     * @li "OPERATOR EMAIL"
     *
     * Common values for this keyword are generic defaults such as "Admin"
     * or "Administrator".
     *
     * @return
     *   Returns the name of the person directing that the data be acquired.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getInstitutionName()
     * @see ::getOperatorName()
     * @see ::setInvestigatorName()
     */
    inline std::string getInvestigatorName( )
        const noexcept
    {
        if ( this->inDictionary( "$EXP" ) == false )
            return std::string( );
        return this->getDictionaryString( "$EXP" );
    }

    /**
     * Returns the name of the individual modifying the data, if not original.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$LAST_MODIFIER" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$LAST_MODIFIER" keyword is defined in the 3.1 FCS specification.
     *
     * @return
     *   Returns the name of the individual that last modified
     *   the data. If the data has is original or the last modification
     *   individual is not known, an empty string is returned.
     *
     * @see ::getModificationDateTime()
     * @see ::getModificationState()
     * @see ::setModifierName()
     */
    inline std::string getModifierName( )
        const noexcept
    {
        if ( this->inDictionary( "$LAST_MODIFIER" ) == false )
            return std::string( );
        return this->getDictionaryString( "$LAST_MODIFIER" );
    }

    /**
     * Returns the data modification date and time, if not original.
     *
     * The date should have the form dd-mmm-yyyy hh:mm:ss and possibly .cc
     * for fractional seconds.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$LAST_MODIFIED" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$LAST_MODIFIED" keyword is defined in the 3.1 FCS specification.
     *
     * @return
     *   Returns the date and time at which the data was last modified.
     *   If the data has is original or the last modification date is
     *   not known, an empty string is returned. The returned date has
     *   the form dd-mmm-yyyy hh:mm:ss and possibly .cc for fractional
     *   seconds.
     *
     * @see ::getModifierName()
     * @see ::getModificationState()
     * @see ::setModificationDateTime()
     */
    inline std::string getModificationDateTime( )
        const noexcept
    {
        if ( this->inDictionary( "$LAST_MODIFIED" ) == false )
            return std::string( );
        return this->getDictionaryString( "$LAST_MODIFIED" );
    }

    /**
     * Returns the modification/originality state.
     *
     * The returned value indicates if the data is original or modified.
     * The following values are standard:
     *
     * - "Original": the data is as it was originally acquired or created
     *   by an instrument or software. No part of the data has been modified.
     *
     * - "NonDataModified": the data is original, but modifications have been
     *   made to dictionary values or an analysis has been added.
     *
     * - "Appended": the data is original, except that new data has been
     *   added either as additional events or additional parameters on the
     *   original events.
     *
     * - "DataModified": the data is not original. Changes have been made
     *   to the data.
     *
     * While the above values are standard, vendors may choose to include
     * other values.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$ORIGINALITY" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$ORIGINALITY" keyword is defined in the 3.1 FCS specification.
     *
     * @return
     *   Returns the modification/originality state. If the state is not
     *   known, an empty string is returned.
     *
     * @see ::getModifierName()
     * @see ::getModificationDateTime()
     * @see ::setModificationState()
     */
    inline std::string getModificationState( )
        const noexcept
    {
        if ( this->inDictionary( "$ORIGINALITY" ) == false )
            return std::string( );
        return this->getDictionaryString( "$ORIGINALITY" );
    }

    /**
     * Returns the name of the operator acquiring the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$OP" keyword.
     * The keyword is an abbreviation of "Operator".
     *
     *
     * <B>History</B><BR>
     * The "$OP" keyword has been defined in all versions of the FCS
     * specification.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information.
     * @li "DEPARTMENT"
     * @li "DIRECTOR"
     * @li "EXPORT USER NAME"
     * @li "OPERATOR EMAIL"
     *
     * Common values for this keyword are generic defaults such as "Admin"
     * or "Administrator".
     *
     * @return
     *   Returns the name of the person operating the acquisition equipment.
     *   If the name is not known, an empty string is returned.
     *
     * @see ::getInvestigatorName()
     * @see ::getInstitutionName()
     * @see ::setOperatorName()
     */
    inline std::string getOperatorName( )
        const noexcept
    {
        if ( this->inDictionary( "$OP" ) == false )
            return std::string( );
        return this->getDictionaryString( "$OP" );
    }

    /**
     * Returns the data acquisition start date.
     *
     * The FCS specification requires that the date have the format
     * "dd-mmm-yyyy". However, some vendors replaces the dashes ("-") with
     * spaces for "dd mmm yyyy". Other vendors flip the order to put
     * the year first for "yyyy-mmm-dd". Month names are not case sensitive.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$DATE" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$DATE" keyword has been defined in all versions of the FCS
     * specification.
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "01-JUN-2009"
     * @li "06-NOV-2012"
     * @li "2011-Jun-30"
     * @li "21 Dec 2010"
     *
     * @return
     *   Returns the date on which data acquisition began. The
     *   returned string has the form dd-mmm-yyyy, though some vendors
     *   use yyy-mmm-dd instead. If the date is not known, an empty
     *   string is returned.
     *
     * @see ::getStartTime()
     * @see ::getEndTime()
     * @see ::setStartDate()
     */
    inline std::string getStartDate( )
        const noexcept
    {
        if ( this->inDictionary( "$DATE" ) == false )
            return std::string( );
        return this->getDictionaryString( "$DATE" );
    }

    /**
     * Returns the data acquisition start time.
     *
     * The FCS specification requires that the time have the format
     * "hh:mm:ss" and optionally ":tt" for fractions of a second. Some
     * vendors omit the fractions of a second, and some use a period (".")
     * separator instead of a colon before the fractions of a second.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$BTIM" keyword.
     *
     *
     * <B>History</B><BR>
     * The "$BTIM" keyword has been defined in all versions of the FCS
     * specification.
     *
     * <B>Sample values</B><BR>
     * Values seen in FCS files:
     * @li "09:27:50"
     * @li "11:15:29.47"
     * @li "11:26:06:852"
     * @li "12:06:22"
     *
     * @return
     *   Returns the clock time at which data acquisition began. The
     *   returned string has the form hh:mm::ss and possible :cc for
     *   fractional seconds. If the time is not known, an empty string
     *   is returned.
     *
     * @see ::getStartDate()
     * @see ::getEndTime()
     * @see ::setStartTime()
     */
    inline std::string getStartTime( )
        const noexcept
    {
        if ( this->inDictionary( "$BTIM" ) == false )
            return std::string( );
        return this->getDictionaryString( "$BTIM" );
    }



    /**
     * Sets the data acquisition end time.
     *
     * Setting the time to an empty string deletes the dictionary entry.
     *
     * The FCS specification requires that the time have the format
     * "hh:mm:ss" and optionally ":tt" for fractions of a second. Some
     * vendors omit the fractions of a second, and some use a period (".")
     * separator instead of a colon before the fractions of a second.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$ETIM" keyword.
     *
     * @param[in] time
     *   The time at which data acquisition ended.
     *
     * @see ::getEndTime()
     * @see ::setStartDate()
     * @see ::setStartTime()
     */
    inline void setEndTime( const std::string& time )
        noexcept
    {
        if ( time.empty( ) == true )
            this->eraseDictionaryEntry( "$ETIM" );
        else
            this->setDictionaryStringInternal(
                "$ETIM",
                time,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the name of the institution acquiring the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$INST" keyword.
     *
     * @param[in] name
     *   The name of the institution acquiring the data.
     *
     * @see ::getInstitutionName()
     * @see ::setInvestigatorName()
     * @see ::setOperatorName()
     */
    inline void setInstitutionName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$INST" );
        else
            this->setDictionaryStringInternal(
                "$INST",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the name of the investigator directing acquisition of
     * the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$EXP" keyword.
     *
     * @param[in] name
     *   The name of the person directing that the data be acquired.
     *
     * @see ::getInvestigatorName()
     * @see ::setInstitutionName()
     * @see ::setOperatorName()
     */
    inline void setInvestigatorName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$EXP" );
        else
            this->setDictionaryStringInternal(
                "$EXP",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the name of the individual modifying the data, if not original.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$LAST_MODIFIER" keyword.
     *
     * @param[in] name
     *   The name of the authority that modified the data.
     *
     * @see ::getModifierName()
     * @see ::setModificationDateTime()
     * @see ::setModificationState()
     */
    inline void setModifierName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$LAST_MODIFIER" );
        else
            this->setDictionaryStringInternal(
                "$LAST_MODIFIER",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the data modification date and time, if not original.
     *
     * Setting the value to an empty string deletes the dictionary entry.
     *
     * The date should have the form dd-mmm-yyyy hh:mm:ss and possibly .cc
     * for fractional seconds.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$LAST_MODIFIED" keyword.
     *
     * @param[in] datetime
     *   The date and time when the data was modified.
     *
     * @see ::getModificationDateTime()
     * @see ::setModifierName()
     * @see ::setModificationState()
     */
    inline void setModificationDateTime( const std::string& datetime )
        noexcept
    {
        if ( datetime.empty( ) == true )
            this->eraseDictionaryEntry( "$LAST_MODIFIED" );
        else
            this->setDictionaryStringInternal(
                "$LAST_MODIFIED",
                datetime,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the modification/originality state.
     *
     * Setting the value to an empty string deletes the dictionary entry.
     *
     * The following values are standard:
     *
     * - "Original": the data is as it was originally acquired or created
     *   by an instrument or software. No part of the data has been modified.
     *
     * - "NonDataModified": the data is original, but modifications have been
     *   made to dictionary values or an analysis has been added.
     *
     * - "Appended": the data is original, except that new data has been
     *   added either as additional events or additional parameters on the
     *   original events.
     *
     * - "DataModified": the data is not original. Changes have been made
     *   to the data.
     *
     * While the above values are standard, vendors may choose to include
     * other values. This method does not enforce the above values.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$ORIGINALITY" keyword.
     *
     * @param[in] value
     *   The originallity state when the data was modified.
     *
     * @see ::getModificationState()
     * @see ::setModifierName()
     * @see ::setModificationDateTime()
     */
    inline void setModificationState( const std::string& value )
        noexcept
    {
        if ( value.empty( ) == true )
            this->eraseDictionaryEntry( "$ORIGINALITY" );
        else
            this->setDictionaryStringInternal(
                "$ORIGINALITY",
                value,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the name of the operator acquiring the data.
     *
     * This value has no well-known format or recommended practice for its
     * content. The value's meaning may be institution-dependent.
     *
     * Setting the name to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$OP" keyword.
     *
     * @param[in] name
     *   The name of the individual operating the acquisition equipment.
     *
     * @see ::getOperatorName()
     * @see ::setInvestigatorName()
     * @see ::setInstitutionName()
     */
    inline void setOperatorName( const std::string& name )
        noexcept
    {
        if ( name.empty( ) == true )
            this->eraseDictionaryEntry( "$OP" );
        else
            this->setDictionaryStringInternal(
                "$OP",
                name,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the data acquisition start date.
     *
     * The FCS specification requires that the date have the format
     * "dd-mmm-yyyy". However, some vendors replaces the dashes ("-") with
     * spaces for "dd mmm yyyy". Other vendors flip the order to put
     * the year first for "yyyy-mmm-dd". Month names are not case sensitive.
     *
     * Setting the date to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$DATE" keyword.
     *
     * @param[in] date
     *   The date on which data acquisition began.
     *
     * @see ::getStartDate()
     * @see ::setStartTime()
     * @see ::setEndTime()
     */
    inline void setStartDate( const std::string& date )
        noexcept
    {
        if ( date.empty( ) == true )
            this->eraseDictionaryEntry( "$DATE" );
        else
            this->setDictionaryStringInternal(
                "$DATE",
                date,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }

    /**
     * Sets the data acquisition start time.
     *
     * The FCS specification requires that the time have the format
     * "hh:mm:ss" and optionally ":tt" for fractions of a second. Some
     * vendors omit the fractions of a second, and some use a period (".")
     * separator instead of a colon before the fractions of a second.
     *
     * Setting the time to an empty string deletes the dictionary entry.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method sets the dictionary value of the "$BTIM" keyword.
     *
     * @param[in] time
     *   The time at which data acquisition began.
     *
     * @see ::getStartTime()
     * @see ::setStartDate()
     * @see ::setEndTime()
     */
    inline void setStartTime( const std::string& time )
        noexcept
    {
        if ( time.empty( ) == true )
            this->eraseDictionaryEntry( "$BTIM" );
        else
            this->setDictionaryStringInternal(
                "$BTIM",
                time,
                false,      // No need to convert keyword or trim.
                true );     // Do trim the value.
    }
    // @}



//----------------------------------------------------------------------
// Parameters.
//----------------------------------------------------------------------
    /**
     * @name Parameters
     */
    // @{
private:
    /**
     * Builds a parameter keyword with an embedded parameter index.
     *
     * The returned keyword is the concatenation of a keyword prefix,
     * a parameter index, and a keyword suffix.
     *
     * @param[in] prefix
     *   The prefix for the parameter keyword. This is usually "$P".
     * @param[in] index
     *   The parameter's index as a positive integer. The index should
     *   be 0-based per standard C/C++ practice. It will be incremented
     *   by one when forming the FCS file parameter keyword.
     * @param[in] suffix
     *   The suffix for the parameter keyword.
     *
     * @return
     *   Returns the keyword name.
     */
    static std::string buildParameterKeyword(
        const char*const prefix,
        const size_t index,
        const char*const suffix )
        noexcept
    {
        return std::string( prefix ) +
            std::to_string( index + 1 ) +
            std::string( suffix );
    }

    /**
     * Builds a generic parameter keyword without an embedded parameter index.
     *
     * The given keyword is parsed to find and replace an embedded parameter
     * index with a generic "n" to create a generic parameter keyword.
     *
     * @param[in] key
     *   The keyword that may have an embedded parameter index.
     *
     * @return
     *   The generic keyword.
     */
    static std::string findGenericFormOfIndexedKeyword( const std::string& key )
        noexcept
    {
        const size_t length = key.length( );

        // Find the first digit, if any.
        size_t startOfDigits = 0;
        for ( ; startOfDigits < length; ++startOfDigits )
            if ( std::isdigit( key[startOfDigits] ) == true )
                break;

        // If no digits were found, return the keyword as-is.
        if ( startOfDigits >= length )
            return key;

        // Find the first non-digit after the first digit.
        size_t startOfSuffix = startOfDigits + 1;
        for ( ; startOfSuffix < length; ++startOfSuffix )
            if ( std::isdigit( key[startOfSuffix] ) == false )
                break;

        // Return the keyword text before the digits, an "n" in place of
        // the digits, and the keyword text after the digits.
        if ( startOfSuffix >= length )
            return key.substr( 0, startOfDigits ) + std::string( "n" );

        return key.substr( 0, startOfDigits ) + std::string( "n" ) +
            key.substr( startOfSuffix );
    }

public:
    /**
     * Returns the number of parameters per event.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PAR" keyword.
     * The keyword is a required part of FCS files.
     *
     * @return
     *   Returns the number of parameters per event. If no file has been
     *   loaded or there is no data, this method returns zero.
     */
    inline size_t getNumberOfParameters( )
        const noexcept
    {
        if ( this->eventTable != nullptr )
            return this->eventTable->getNumberOfParameters( );
        if ( this->inDictionary( "$PAR" ) == false )
            return 0L;
        return this->getDictionaryLong( "$PAR" );
    }

    /**
     * Returns a list of short parameter names.
     *
     * This method gets each of the short parameter names and appends them,
     * in order, to the returned list.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary values of the "$PnN" keywords,
     * where "n" is the (index + 1). The keyword is a required part of
     * FCS files.
     *
     * @param[out] names
     *   The vector to fill with parameter names.
     *
     * @see getParameterShortName()
     */
    inline std::vector<std::string> getParameterShortNames( )
        const noexcept
    {
        // The event table's list of parameter names correspond to the
        // parameter short names in the FCS dictionary. Returning them
        // is faster if there is an event table.
        if ( this->eventTable != nullptr )
            return this->eventTable->getParameterNames( );

        const size_t n = this->getNumberOfParameters( );
        std::vector<std::string> names;
        names.reserve( n );

        for ( size_t i = 0; i < n; ++i )
            names.push_back( this->getParameterShortName( i ) );

        return names;
    }

    /**
     * Returns the parameter's calibration scaling factor.
     *
     * The calibration scaling factor maps parameter values to a calibration
     * units of measure, such as MESF (mean equivalent soluable fluorochrome).
     * Conversion from a parameter's stored value to the calibration units
     * is done by multiplying the two values:
     * @code
     * std::string units;
     * double factor;
     * data->getParameterCalibrationScaling( index, &factor, &units );
     *
     * auto event = data->getEventFloats( eventIndex );
     * auto parameter = data->event[parameterIndex];
     * auto scaled = parameter * factor;
     *
     * printf( "%lf %s\n", scaled, units );
     * @endcode
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnCALIBRATION" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The FCS 3.1 specification introduced the optional "$PnCALIBRATION"
     * keyword. The keyword's value is a string containing a comma-separated
     * pair where the first value is a floating-point value used to map
     * parameter values to a target unit of measure, and the second value is
     * the name of that unit of measure.
     *
     * @param[in] index
     *   The parameter index.
     * @param[out] factor
     *   The returned scaling factor. If there is no calibration scaling
     *   set, this is returned as 0.0 and the units are an empty string.
     * @param[out] units
     *   The returned scaling units of measure. If there is no
     *   calibration scaling set, this is returned as an empty string and
     *   the factor is 0.0.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    inline void getParameterCalibrationScaling(
        const size_t index,
        double& factor,
        std::string& units )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            // The value for the keyword is a comma-separated tuple: f,s.
            // - "f" is the calibration scaling factor.
            // - "s" is the units of measure post scaling.
            //
            // The value starts with the digits of a floating point value,
            // which are defined in ASCII. Searching for an ASCII comma
            // after these digits therefore does not require UTF-8 handling.
            // The units name after the comma includes all characters after
            // the comma, which could be UTF-8 and are simply copied to
            // the output.
            const auto s = this->getDictionaryString(
                this->buildParameterKeyword(
                    "$P",
                    index,
                    "CALIBRATION" ) );
            const size_t commaPosition = s.find( ',' );
            factor = std::stod( s.substr( 0, commaPosition ) );
            units = s.substr( commaPosition + 1 );
        }
        catch ( ... )
        {
            factor = 0;
            units = std::string( );
        }
    }

    /**
     * Returns the parameter's detector light collection percentage.
     *
     * The returned value is the amount of lighted collected by the detector
     * for the parameter, expressed as a percentage of the light emitted
     * by a fluorescent object. For example, if 50% of the emitted light
     * was captured by the detector, the returned value is 50.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnP" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$PnP" keyword is defined in all versions of the FCS specification
     * and is optional. When present it is the percentage of the light emitted
     * that was captured by the detector for this parameter.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the excitation power used for the parameter.
     *   If no excitation power is set, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    inline long getParameterDetectorLightPercentage( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryLong( this->buildParameterKeyword(
                "$P",
                index,
                "P" ) );
        }
        catch ( ... )
        {
            return 0;
        }
    }

    /**
     * Returns the parameter's optical filter.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnF" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History>
     * The "$PnF" keyword is defined in all versions of the FCS specification
     * and is optional. In all cases, the value is a string that names the
     * optical filter used. The format of the string is not defined.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the name of the optical filter used for the parameter.
     *   If no optical filter is set, an empty string is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    inline std::string getParameterDetectorOpticalFilter( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryString( this->buildParameterKeyword(
                "$P",
                index,
                "F" ) );
        }
        catch ( ... )
        {
            return std::string( );
        }
    }

    /**
     * Returns the parameter's detector type.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnT" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$PnT" keyword is defined in all versions of the FCS specification
     * and is optional. When present it is the name of the photomultiplier
     * tube or detector type. The name has no specific format.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the name of the detector type used for the parameter.
     *   If no detector type is set, an empty string is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterDetectorVoltage()
     */
    inline std::string getParameterDetectorType( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryString( this->buildParameterKeyword(
                "$P",
                index,
                "T" ) );
        }
        catch ( ... )
        {
            return std::string( );
        }
    }

    /**
     * Returns the parameter's detector voltage.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnV" keyword,
     * where "n" is the (index + 1). The keyword is a required part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$PnV" keyword is defined in all versions of the FCS specification
     * and is optional. When present it is the voltage of the photomultiplier
     * used to measure the parameter's value.
     *
     * In FCS 1.0, 2.0, and 3.0, the keyword's value is an integer. In 3.1 it
     * is floating point. This method returns floating point.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the name of the detector voltage used for the parameter.
     *   If no detector voltage is set, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterDetectorType()
     */
    inline double getParameterDetectorVoltage( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryDouble( this->buildParameterKeyword(
                "$P",
                index,
                "V" ) );
        }
        catch ( ... )
        {
            return 0.0;
        }
    }

    /**
     * Returns the parameter's recommended display scaling.
     *
     * The display scaling factor maps parameter values to a recommended
     * linear or logarithmic scale. For linear scaling, the returned
     * scaling name is "Linear" and the two returned values are the lower
     * and upper bounds. For log scaling, the returned scaling name is
     * "Logarithmic" and the two returned values are the number of decades
     * to display and the offset from 0 that selects the starting value.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnD" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The FCS 3.1 specification introduced the optional "$PnD" keyword.
     *
     * @param[in] index
     *   The parameter index.
     * @param[out] name
     *   The returned scaling name. Recognized values are "Linear" and
     *   "Logarithmic". The value is empty if neither one is defined.
     * @param[out] value1
     *   The lower bound for linear scaling, and the number of decades for
     *   log scaling.
     * @param[out] value2
     *   The upper bound for linear scaling, and the offset for log scaling.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    inline void getParameterDisplayScaling(
        const size_t index,
        std::string& name,
        double& value1,
        double& value2 )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            // The value for the keyword is a comma-separated tuple: name,v1,v2.
            // - "name" is the scaling name.
            // - "v1" is the first value.
            // - "v2" is the second value.
            //
            // The name is expected to be "Linear" or "Logarithmic", both of
            // which use only ASCII letters. The remainder of the value should
            // be number digits, and related punctuation, which are again
            // ASCII. Parsing of this value therefore does not require UTF-8
            // string handling.
            const auto s = this->getDictionaryString(
                this->buildParameterKeyword(
                    "$P",
                    index,
                    "D" ) );
            size_t commaPosition1 = s.find( ',' );
            size_t commaPosition2 = s.find( ',', commaPosition1 + 1 );
            name = s.substr( 0, commaPosition1 );
            value1 = std::stod( s.substr(
                commaPosition1 + 1,
                (commaPosition2 - commaPosition1 - 1) ) );
            value2 = std::stod( s.substr( commaPosition2 + 1 ) );
        }
        catch ( ... )
        {
            name = std::string( );
            value1 = 0.0;
            value2 = 0.0;
        }
    }

    /**
     * Returns the parameter detector's excitation power in milliwats.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnO" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$PnO" keyword is defined in all versions of the FCS
     * specification and is optional. When present it is the output
     * power (in milliwats) for the light source associated with
     * measurements of the parameter.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the excitation power used for the parameter.
     *   If no excitation power is set, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    inline long getParameterDetectorExcitationPower( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryLong( this->buildParameterKeyword(
                "$P",
                index,
                "O" ) );
        }
        catch ( ... )
        {
            return 0;
        }
    }

    /**
     * Returns the parameter's excitation wavelengths in nm.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnL" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$PnL" keyword is defined in all versions of the FCS specification
     * and is optional. The format of the keyword's value has changed between
     * specification versions. In FCS 1.0, 2.0, and 3.0, the keyword's value
     * is a single integer for the excitation wavelength (in nm) for the
     * parameter. In FCS 3.1, however, the value changed into a comma
     * separated list of excitation wavelengths (still in nm) for devices that
     * can use multiple lasers with different wavelengths to measure
     * a single parameter.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns a vector of excitation wavelengths used for the parameter.
     *   If no wavelengths are set, an empty vector is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     */
    std::vector<long> getParameterDetectorExcitationWavelengths(
        const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        std::vector<long> wavelengths;
        try
        {
            // The value for the keyword may be a comma-separated list of
            // integer wavelengths, in nm.
            //
            // The entire list is composed of integer number digits and
            // comma separators, all of which are defined in ASCII. No
            // special UTF-8 handling is needed.
            const auto s = this->getDictionaryString(
                this->buildParameterKeyword(
                    "$P",
                    index,
                    "L" ) );

            for ( size_t pos = 0; ; )
            {
                const size_t commaPosition = s.find( ',', pos );
                if ( commaPosition == std::string::npos )
                {
                    if ( pos < s.length( ) )
                        wavelengths.push_back( std::stol( s.substr( pos ) ) );
                    break;
                }

                wavelengths.push_back( std::stol( s.substr( pos, commaPosition ) ) );
                pos = commaPosition + 1;
            }

            return wavelengths;
        }
        catch ( ... )
        {
            return wavelengths;
        }
    }

    /**
     * Returns the parameter's linear or logarithmic scaling for integer data.
     *
     * Parameter scaling is only relevant if:
     *
     * @li Current event table data has been loaded from a file.
     *
     * @li Auto-scaling was disabled prior to loading the file. Auto-scaling
     *     is normally enabled and causes loading to automatically apply
     *     linear or logarithmic scaling as data is loaded.
     *
     * If no scaling factors are set in a file, then FCS file data is
     * considered "scale data" and ready for further use.
     *
     * If scaling factors are set in a file, then FCS file data is instead
     * considered "channel data" that was scaled before it was written to
     * the file during acquisition. Before the data can be used, it must
     * be re-scaled to restore its original value range and convert it to
     * "scale data".
     *
     * In general, data written as floating point values in a file does not
     * need scaling and is already "scale data". Data written as integers
     * is usually "channel data" and in need of scaling. However, integer
     * data does not necessarily have scaling, and floating point data can
     * have linear (but not not log) scaling.
     *
     * Scaling for channel data is always one of:
     *
     * @li "linear" scaling that returns a single scale factor as the
     *     first of two numeric method arguments. Linear scaling can be
     *     applied to file data read as integers or floating point.
     *
     * @li "log" scaling that returns a number of log decades and an
     *     offset from zero as the two numeric method arguments. Log scaling
     *     only can be applied to file data read as integers.
     *
     * For linear scaling, integer and floating point channel values may be
     * scaled using the
     * formula:
     * @code
     * scaled = channel / scale
     * @endcode
     * where "channel" is the unscaled integer data from the file, "scaled"
     * is the scaled result, and "scale" is the linear scale factor returned
     * by this method. All math is done in floating point.
     *
     * For logarithmic scaling, integer channel values may be scaled using
     * the formula:
     * @code
     * scaled = pow( 10, (decades * channel / range)) * offset
     * @endcode
     * where "channel" is the unscaled integer data from the file, "scaled"
     * is the scaled result, "decades" and "offset" are the number of log
     * decades and the log offset returned by this method, and "range" is
     * the integer range of the parameter. The latter is returned by
     * getParameterRange(). All math is done in floating point.
     *
     * Parameter scaling is normally done automatically when a file is loaded
     * and it contains integer data. Scaling is not needed when a file contains
     * floating point data. See setAutoScaling().
     *
     * When auto-scaling is not used, parameter scaling can be done by the
     * caller using the values returned by this method and getParameterRange().
     * Alternately, parameter scaling can be done by scaleParameter(), which
     * does the above math on a selected parameter. See also
     * scaleAllParameters(), which performs appropriate scaling on all
     * parameters.
     *
     * If scaling factors are not available in the file's dictionary, then
     * returned values indicate unit linear scaling.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns a combination of two dictionary values from the
     * "$PnG" and "$PnE" keywords, where "n" is the (index + 1). The
     * "$PnG" keyword is an optional part of FCS files, but the "$PnE"
     * keyword is required.
     *
     * "$PnE" specifies an optional logarithmic gain for a parameter and
     * includes the number of decades and offset from zero. If the decades
     * value is zero, then no logarithmic gain is required and linear gain
     * may be used instead. "$PnG" then specifies the linear gain. If this
     * keyword is not given, gain defaults to 1.0 (no gain).
     *
     *
     * <B>History</B><BR>
     * The "$PnE" keyword was introduced in the FCS 2.0 specification, but
     * not required. For the FCS 3.0 and 3.1 specifications, the keyword is
     * required.
     *
     * The keyword value is a string with two comma-separated floating-point
     * values. The first is the logarithmic scaling factor (the number
     * of decades) and the second is an offset.
     *
     * For linear scaling, the FCS 2.0 specification requires that the first
     * value be 0.0, but the value of the second value is unspecified.
     * For the FCS 3.0 and 3.1 specifications, both values should be 0.0 for
     * linear scaling.
     *
     * When using linear scaling, the "$PnG" keyword provides the gain.
     * This keyword was introduced in the FCS 3.0 specification to
     * indicate the linear gain that was used to amplify the signal for
     * the parameter.
     *
     * @param[in] index
     *   The parameter index.
     * @param[out] scaleType
     *   The name of the gain type used. This is either "linear" or "log".
     *   If no gain type is explicitly set in the dictionary, "linear" is
     *   returned with a 1.0 gain factor.
     * @param[out] value1
     *   For linear gain, this is the gain factor. For log gain, this is
     *   the number of decades used.
     * @param[out] value2
     *   For linear gain, this value is not used. For log gain, this is
     *   the offset, or minimum on the log scale.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterRange()
     * @see ::scaleParameter()
     * @see ::scaleAllParameters()
     */
    void getParameterScaling(
        const size_t index,
        std::string& scaleType,
        double& value1,
        double& value2 )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            std::string keyword = this->buildParameterKeyword(
                    "$P",
                    index,
                    "E" );

            if ( this->inDictionary( keyword ) == true )
            {
                // The value for the keyword is a comma-separated tuple: f1,f2.
                // "f1" is the log scale factor.
                // "f1" is the log offset.
                //
                // Both values are composed entirely of number digits and
                // related punctuation, which are all defined in ASCII.
                // Parsing the value therefore does not require UTF-8 handling.
                const auto s = this->getDictionaryString( keyword );
                const size_t commaPosition = s.find( ',' );
                value1 = std::stod( s.substr( 0, commaPosition ) );
                value2 = std::stod( s.substr( commaPosition + 1 ) );

                if ( value1 != 0.0 )
                {
                    scaleType = "log";

                    // SPECIAL HANDLING: Prior to FCS 3.1, it was not
                    // explicitly stated that the second value in log
                    // scaling cannot be zero. The 3.1 specification
                    // recommends that if a 0.0 is encountered for the
                    // second value, that it be forced to a 1.0.
                    if ( value2 == 0.0 )
                        value2 = 1.0;
                    return;
                }
            }

            scaleType = "linear";
            value1 = this->getDictionaryDouble( this->buildParameterKeyword(
                "$P",
                index,
                "G" ) );
            value2 = 0.0;
        }
        catch ( ... )
        {
            scaleType = "linear";
            value1 = 1.0;
            value2 = 0.0;
        }
    }

    /**
     * Returns the long name of the indicated parameter.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnS" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * The FCS 1.0 and 2.0 specifications use "$PnS" for the name of
     * the fluorescence stain or probe used with parameter "n". The "S"
     * in the keyword is therefore for "stain" and the keyword should
     * not be confused with the keyword for a parameter's short name,
     * which is "$PnN" and available from getParameterShortName().
     *
     * The FCS 3.0 and 3.1 specifications simplified this keyword to
     * mean any long name. The long name is intended for use as axis names
     * in plots.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the long name of the parameter. If no long name has been
     *   set, an empty string is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterShortName()
     * @see ::getParameterShortNames()
     */
    inline std::string getParameterLongName( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryString( this->buildParameterKeyword(
                "$P",
                index,
                "S" ) );
        }
        catch ( ... )
        {
            return std::string( );
        }
    }

    /**
     * Returns a list of long parameter names.
     *
     * This method gets each of the long parameter names and appends them,
     * in order, to the returned list.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary values of the "$PnS" keywords,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files. If a long name is not provided, an empty string is returned
     * for the parameter.
     *
     * @param[out] names
     *   The vector to fill with parameter long names.
     *
     * @see ::getParameterLongName()
     */
    inline std::vector<std::string> getParameterLongNames( )
        const noexcept
    {
        // The event table's list of parameter long names correspond to the
        // parameter long names in the FCS dictionary. Returning them
        // is faster if there is an event table.
        if ( this->eventTable != nullptr )
            return this->eventTable->getParameterLongNames( );

        const size_t n = this->getNumberOfParameters( );
        std::vector<std::string> names;
        names.reserve( n );

        for ( size_t i = 0; i < n; ++i )
            names.push_back( this->getParameterLongName( i ) );

        return names;
    }


    /**
     * Returns the parameter's numeric range.
     *
     * For unsigned integer values, the parameter's range specifies the
     * legal range of values from 0 up to, but not including, the range
     * value. For instance, if the value is 1024, then the range of valid
     * numbers is 0 through 1023.
     *
     * For floating point values, the parameter's range is not as meaningful.
     * While it still indicates the numeric range used during data collection,
     * the floating point stored in the file may be negative and it may be
     * more positive or more negative than the range.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnR" keyword,
     * where "n" is the (index + 1). The keyword is an optional part of
     * FCS files.
     *
     *
     * <B>History</B><BR>
     * For FCS 1.0 and 2.0, the "$PnR" keyword's value indicated the
     * number of events in the file. This meaning became redundant with
     * the introduction of the "$PAR" keyword in FCS 2.0.
     *
     * In FCS 3.0 and 3.1, the meaning of "$PnR" was shifted to specify
     * the numeric range of the parameter. The value is also used to mask
     * off the upper bits of integer values stored in the file.
     *
     * In modern data, FCS files typically store floating-point values for
     * which the "$PnR" keyword is not that useful. The keyword is nevertheless
     * still a required part of FCS files.
     *
     * The range value is needed for logarithmic scaling. While log scaling
     * is technically only supported for integer data, some vendors use it
     * for floating point data as well.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the amplifier log gain offset used for the parameter.
     *   If no value is set, a zero is returned.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterScaling()
     * @see ::scaleParameter()
     * @see ::scaleAllParameters()
     */
    inline double getParameterRange( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryDouble( this->buildParameterKeyword(
                "$P",
                index,
                "R" ) );
        }
        catch ( ... )
        {
            return 0.0;
        }
    }

    /**
     * Returns the short name of the indicated parameter.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$PnN" keyword,
     * where "n" is the (index + 1). The keyword is a required part of
     * FCS files.
     *
     *
     * <B>Short name use by other keywords</B><BR>
     * Short parameter names are used in other keywords as a unique selector
     * for a parameter. That usage requires that a comma (",") be used to
     * separate names and values. For that reason, commas should not be used
     * in short parameter names.
     *
     * Other keywords that use short names:
     *
     * - The spillover matrix (see getSpilloverMatrix()) used for compensation
     *   calculations uses short parameter names to name matrix columns.
     *
     * - The trigger parameter (see getTriggerParameter()) uses the short name
     *   of a parameter to indicate a parameter (if any) used as the trigger
     *   for generating an event.
     *
     * <B>History</B><BR>
     * The "$PnN" keyword was first defined in the FCS 1.0 and 2.0
     * specifications, but its value was limited to a few choices:
     *
     * @li "FS" for "Forward scatter".
     * @li "SS" for "Side scatter".
     * @li "FL" for "Fluorescence".
     * @li "AE" for "Axial extinction".
     * @li "CV" for "Coulter volume".
     * @li "TI" for "Time".
     *
     * The FCS 3.0 specification again limited the value to a few choices:
     * @li "CS" for "Cell subset".
     * @li "FS" for "Forward scatter".
     * @li "SS" for "Side scatter".
     * @li "FLn" for "Fluorescence" for channel "n".
     * @li "AE" for "Axial extinction".
     * @li "CV" for "Coulter volume".
     * @li "TIME" for "Time".
     *
     * Note that in FCS 3.0, the time parameter is now "TIME" and not "TI".
     * The use of "TIME" is required when the "$TIMESTEP" keyword is used
     * to indicate the delta between event times.
     *
     * The FCS 3.1 specification removed all of the above choices and allows
     * arbitrary short names, however the time parameter still must be named
     * "TIME".
     *
     *
     * <B>Validation</B><BR>
     * When an FCS file is loaded, parameter short names are checked and
     * must be non-empty. For FCS 1.0 and 2.0 files, the well-known "TI"
     * parameter name for time is changed to the modern "TIME" name defined
     * in FCS 3.0 and later.
     *
     * @param[in] index
     *   The parameter index.
     *
     * @return
     *   Returns the short name of the parameter.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     *
     * @see ::getParameterLongName()
     * @see ::getParameterShortNames()
     */
    inline std::string getParameterShortName( const size_t index )
        const
    {
        if ( index < 0 || index >= this->getNumberOfParameters( ) )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range" );

        try
        {
            return this->getDictionaryString( this->buildParameterKeyword(
                "$P",
                index,
                "N" ) );
        }
        catch ( ... )
        {
            return std::string( );
        }
    }

    /**
     * Returns the spillover matrix keyword, if any.
     *
     *
     * <B>History</B><BR>
     * The "$SPILLOVER" keyword was introduced in the 3.1 FCS specification.
     * The earlier "$COMP" keyword introduced in the 3.0 FCS specification
     * had a similar format but lacked the necessary parameter names. The
     * "$COMP" keyword was therefore removed in FCS 3.1.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information.
     * @li "$SPILL"
     * @li "SPILL"
     * @li "SPILLOVER"
     *
     * @param[in] allowAlternatives
     *   (optionial, default = true) When false, this method looks for the
     *   FCS standard "$SPILLOVER" keyword and parses its value. If the
     *   keyword is not found, an empty names list and matrix are returned.
     *   But when allowAlternatives is true, this method looks for alternative
     *   spillover matrix keywords if "$SPILLOVER" is not found. Keywords
     *   searched in order are:
     *   @li "$COMP" from the FCS 3.0 standard.
     *   @li "$SPILL", which was never a standard keyword but is considered
     *   one by the R FlowCore FCS library.
     *   @li "SPILL", which is widely used by vendors prior to and in parallel
     *   with the FCS 3.1 standard.
     *   @li "SPILLOVER", which is a vendor alternative to "SPILL".
     *
     * @return
     *   Returns the keyword containing the spillover matrix, or an empty
     *   string if no matrix is defined.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the spillover matrix keyword value is malformed
     *   and cannot be parsed into a valid list of parameter names and a
     *   numeric matrix.
     *
     * @see ::getParameterShortName()
     * @see ::getSpilloverMatrix()
     * @see ::isCompensationRequired()
     */
     std::string getSpilloverMatrixKeyword( bool allowAlternatives = true )
        const
    {
        // Get the spillover matrix keyword, if any. Start with the
        // FCS 3.1 standard "$SPILLOVER". If that is not found and the
        // caller doesn't want to consider alternatives, then return.
        // Otherwise cycle through other known spillover keywords.
        if ( this->inDictionary( "$SPILLOVER" ) == true )
            return "$SPILLOVER";
        if ( allowAlternatives == false )
            return "";

        if ( this->inDictionary( "$COMP" ) == true )
            return "$COMP";
        if ( this->inDictionary( "$SPILL" ) == true )
            return "$SPILL";
        if ( this->inDictionary( "SPILL" ) == true )
            return "SPILL";
        if ( this->inDictionary( "SPILLOVER" ) == true )
            return "SPILLOVER";

        return "";
    }

    /**
     * Returns the spillover matrix used for compensation.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$SPILLOVER" keyword.
     * The keyword is an optional part of FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$SPILLOVER" keyword was introduced in the 3.1 FCS specification.
     * The earlier "$COMP" keyword introduced in the 3.0 FCS specification
     * had a similar format but lacked the necessary parameter names. The
     * "$COMP" keyword was therefore removed in FCS 3.1.
     *
     * Some vendors define additional non-standard keywords that provide
     * the same or related information.
     * @li "$SPILL"
     * @li "SPILL"
     * @li "SPILLOVER"
     *
     * @param[out] names
     *   Fills the given string vector with the names of parameters
     *   involved in the spillover matrix. If no spillover matrix is found,
     *   an empty list is returned.
     * @param[out] matrix
     *   Fills the given double vector with the values of the 2D spillover
     *   matrix. The number of values is (n*n) where n is the number of
     *   parameters with names in the names vector. If no spillover matrix is
     *   found, an empty list is returned.
     * @param[in] allowAlternatives
     *   (optionial, default = true) When false, this method looks for the
     *   FCS standard "$SPILLOVER" keyword and parses its value. If the
     *   keyword is not found, an empty names list and matrix are returned.
     *   But when allowAlternatives is true, this method looks for alternative
     *   spillover matrix keywords if "$SPILLOVER" is not found. Keywords
     *   searched in order are:
     *   @li "$COMP" from the FCS 3.0 standard.
     *   @li "$SPILL", which was never a standard keyword but is considered
     *   one by the R FlowCore FCS library.
     *   @li "SPILL", which is widely used by vendors prior to and in parallel
     *   with the FCS 3.1 standard.
     *   @li "SPILLOVER", which is a vendor alternative to "SPILL".
     *
     * @throws std::invalid_argument
     *   Throws an exception if the spillover matrix keyword value is malformed
     *   and cannot be parsed into a valid list of parameter names and a
     *   numeric matrix.
     *
     * @see ::getParameterShortName()
     * @see ::getSpilloverMatrixKeyword()
     * @see ::isCompensationRequired()
     */
    void getSpilloverMatrix(
        std::vector<std::string>& names,
        std::vector<double>& matrix,
        bool allowAlternatives = true )
        const
    {
        // Initialize.
        // -----------
        // Clear the names and matrix in preparation for parsing.
        names.clear( );
        matrix.clear( );

        // Get the spillover matrix keyword value, if any. Start with the
        // FCS 3.1 standard "$SPILLOVER". If that is not found and the
        // caller doesn't want to consider alternatives, then return.
        // Otherwise cycle through other known spillover keywords.
        std::string keyword = this->getSpilloverMatrixKeyword( allowAlternatives );
        if ( keyword.empty( ) == true )
            return;
        std::string string = this->getDictionaryString( keyword );;

        // The FCS 3.1 standard "$SPILLOVER" keyword's value has the form:
        //   n,name1,name2,name3,...,f1,f2,f3,f4,...
        //
        // where:
        // - "n" is the number of parameters in the matrix.
        // - "name1,name2,name3,..." are parameter short names for columns
        //    and rows in the matrix.
        // - "f1,f2,f3,f4,..." are the values of a 2D matrix with "n"
        //   rows and "n" columns.
        //
        // Prior to FCS 3.1, some vendors included a "SPILL" keyword with
        // a value with the same format.
        //
        // The FCS 3.0 "$COMP" keyword used a simpler form that did not
        // have parameter names. It was assumed that the matrix had one
        // row and column for every parameter, so "n" had to equal the
        // number of parameters.
        //
        //
        // Parse into string list.
        // -----------------------
        // Split the string at commas to build a list of string values.
        std::vector<std::string> values;
        size_t i = 0;
        size_t comma;
        while ( (comma = string.find( ',', i )) != std::string::npos )
        {
            values.push_back( string.substr( i, (comma - i) ) );
            i = comma + 1;
        }
        values.push_back( string.substr( i ) );

        const size_t numberOfParameters = this->getNumberOfParameters( );
        const auto parameterNames = this->getParameterShortNames( );

        //
        // Validate.
        // ---------
        // The first value should be an integer for the matrix size.
        size_t n = 0;
        try
        {
            n = std::stol( values[0] );
        }
        catch ( ... )
        {
            throw std::invalid_argument(
                std::string( "Malformed spillover matrix.\n" ) +
                std::string( "The first value in the spillover matrix string must be the integer matrix size, but the value cannot be parsed as an integer: \"" ) +
                values[0] +
                std::string( "\"." ) );
        }

        if ( n == 0 )
            throw std::invalid_argument(
                std::string( "Malformed spillover matrix.\n" ) +
                std::string( "The first value in the spillover matrix string must be the matrix size, but an invalid zero value was found." ) );
        else if ( n == 1 )
            throw std::invalid_argument(
                std::string( "Malformed spillover matrix.\n" ) +
                std::string( "The spillover matrix must be at least 2x2 but an invalid size of 1x1 was found." ) );
        else if ( n > numberOfParameters )
            throw std::invalid_argument(
                std::string( "Malformed spillover matrix.\n" ) +
                std::string( "The first value in the spillover matrix string must be the matrix size that is limited to the number of parameters defined, but an invalid value larger than the number of parameters was found: \"" ) +
                std::to_string( n ) +
                std::string( "\"." ) );

        //
        // Select variant and get parameter names.
        // ---------------------------------------
        // The second value can be one of:
        //
        // - A floating point value, indicating an old style of matrix
        //   that has no parameter names or indexes and immediately starts
        //   with matrix values. This variant will also have "n" fewer
        //   values than the other variants.
        //
        // - A string, indicating a correctly formatted matrix with a
        //   list of names.
        //
        // - An integer, indicating an incorrectly formatted matrix with
        //   a list of integer parameter indexes.
        size_t floatsIndex = 1;
        if ( values.size( ) == (n * n) + 1 )
        {
            // Possibly the old style matrix defined in the FCS 3.0
            // specification for the "$COMP" keyword. This keyword was
            // poorly defined and does not seem to be used.
            //
            // SPECIAL HANDLING: While "$COMP" can get away with having
            // no parameter names, no other keyword should.
            if ( this->verbose == true && keyword != "$COMP" )
                std::cerr << this->verbosePrefix << ": " <<
                    "FORMAT ERROR: Spillover matrix does not include parameter names and is not the old style $COMP. Interpreted as $COMP anyway.\n";

            // The parameter names list defaults to all parameters in
            // the file.
            names = parameterNames;
        }
        else if ( values.size( ) == (n * n) + n + 1 )
        {
            // Current style matrix defined in the FCS 3.1 specification
            // for the "$SPILLOVER" keyword and by vendor convention for
            // the "SPILL" keyword.
            //
            // The next n values are parameter names. Or maybe integer
            // parameter indexes for some vendor variants.
            //
            // SPECIAL HANDLING: Some vendors incorrectly implement
            // "$SPILLOVER" et al by using integer parameter indexes
            // instead of parameter short names.
            bool verboseReportedIntegerParameter = false;
            for ( size_t i = 0; i < n; ++i )
            {
                // The parameter name should be a text name. But check
                // to see if it can be fully parsed as an integer.
                std::string s = values[i+1];
                bool wasIndex = false;
                size_t index = 0;
                try
                {
                    size_t nUsed = 0;
                    index = std::stol( s, &nUsed );

                    // If the parse did not throw an exception and it
                    // used the entire string, then treat it as an int.
                    if ( nUsed == s.length( ) )
                        wasIndex = true;
                }
                catch ( ... )
                {
                    // The name could not be parsed as an integer,
                    // which is the way it should be. Use the name
                    // as a parameter name.
                }

                if ( wasIndex == true )
                {
                    // The parameter was an integer parameter index.
                    // Validate the index and use it to select the
                    // parameter name.
                    if ( index >= this->getNumberOfParameters( ) )
                        throw std::invalid_argument(
                            std::string( "Malformed spillover matrix.\n" ) +
                            std::string( "The parameter name list incorrectly uses parameter indexes, and one of those indexes is larger than the number of parameters defined in the file." ) );

                    // Look up the parameter name.
                    s = this->getParameterShortName( index );

                    if ( verboseReportedIntegerParameter == false &&
                        this->verbose == true )
                    {
                        std::cerr << this->verbosePrefix << ": " <<
                            "    FORMAT ERROR: Spillover matrix uses integer parameter indexes instead of parameter names. Handled.\n";
                        verboseReportedIntegerParameter = true;
                    }
                }
                else
                {
                    // The parameter was given as a short name.
                    // Validate that the short name is in fact the name
                    // of a parameter
                    bool found = false;
                    for ( size_t j = 0; j < numberOfParameters; ++j )
                    {
                        if ( parameterNames[j] == s )
                        {
                            found = true;
                            break;
                        }
                    }

                    if ( found == false )
                        throw std::invalid_argument(
                            std::string( "Malformed spillover matrix.\n" ) +
                            std::string( "The keyword value refers to a named parameter for a row and column, but no parameter not th that name is defined by the file: \"" ) +
                            s + std::string( "\"." ) );
                }

                names.push_back( s );
            }

            floatsIndex += n;
        }
        else
            throw std::invalid_argument(
                std::string( "Malformed spillover matrix.\n" ) +
                std::string( "The keyword value does not have the proper number of values. The first number in the value specifies a matrix with " ) +
                std::to_string( n ) + " x " + std::to_string( n ) +
                std::string( " values, which requires this many numbers preceded by a list of parameter names for a total of " ) +
                std::to_string( n*n + n ) +
                std::string( " comma-separated values after the first number. But instead " ) +
                std::to_string( values.size( ) ) +
                std::string( " values were found." ) );

        //
        // Parse matrix.
        // -------------
        // The remainder of the values should all be floating point numbers.
        const size_t nn = n * n;
        try
        {
            for ( size_t i = 0; i < nn; ++i )
                matrix.push_back( std::stod( values[floatsIndex + i] ) );
        }
        catch ( const std::exception& e )
        {
            throw std::invalid_argument(
                std::string( "Malformed spillover matrix.\n" ) +
                std::string( "The spillover matrix should contain a list of numeric values for compensation, but the values cannot be parsed as floating point." ) );
        }
    }

    /**
     * Returns true if compensation is required.
     *
     * By default, all FCS files require compensation. A spillover matrix
     * should be defined in the file, or in an associated Gating-ML file.
     * Alternately, compensation can be computed using calibration data and
     * other techniques.
     *
     * Some vendors pre-compensate data stored into the file. Technically,
     * this is in violation of the FCS specification. But if it is done,
     * vendors should include a dictionary keyword indicating that compensation
     * has already been done. There is no FCS standard keyword indicating
     * that compensation has already been done, so any such keyword is
     * vendor-specific.
     *
     * This method looks for vendor keywords that indicate that compensation
     * has already been applied. If no keyword is found, this method assumes
     * that compensation is required and returns true.
     *
     * If a vendor-specific keyword is found that indicates that compensation
     * has already been applied, this method returns false.
     *
     * @return
     *   Returns true if compensation is required, and false otherwise.
     */
    inline bool isCompensationRequired( )
        const noexcept
    {
        if ( this->inDictionary( "APPLY COMPENSATION" ) == false )
            return true;

        auto value = this->getDictionaryString( "APPLY COMPENSATION" );
        std::transform(
            value.begin( ),
            value.end( ),
            value.begin( ),
            [](unsigned char c){ return std::toupper(c); } );
        if ( value == "FALSE" )
            return false;

        return true;
    }

    /**
     * Returns the time step, in seconds, for a time parameter, if any.
     *
     * When a parameter is used to store the event time, the parameter must
     * have the short name "TIME". Time parameter values start at 0.0 for
     * the first event and are in units of timesteps. Conversion from the
     * value in a parameter to seconds is done by multiplying the parameter's
     * value by the timestep fraction of a second returned by this method.
     *
     * For example, let event one occur at time 0, and event two occur 1 second
     * later. With a timestep of 0.01, the stored parameter value is
     * (1 / 0.01) = 100. To convert the parameter value back to a time, in
     * seconds, use (1 * 0.01) = 0.01 seconds.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$TIMESTEP" keyword.
     * The keyword is an optional part of FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$TIMESTEP" keyword was introduced in the 3.0 FCS specification.
     * Prior to its standardization, a similar value was available in
     * vendor-specific keyword values:
     * @li "TIMETICKS"
     * @li "FJ_$TIMESTEP"
     *
     * @return
     *   Returns the timestep length, as a fraction of a second. If the
     *   value is not known, a zero is returned.
     *
     * @see ::getParameterShortName()
     */
    inline double getTimestep( )
        const noexcept
    {
        if ( this->inDictionary( "$TIMESTEP" ) == false )
            return 0.0;
        return this->getDictionaryDouble( "$TIMESTEP" );
    }

    /**
     * Returns the name and threshold for a trigger parameter, if any.
     *
     * When a parameter is used to store a trigger signal for an event,
     * this method returns short name of the parameter and a threshold.
     * The threshold indicates a channel value that must be exceeded to
     * declare an event.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$TR" keyword.
     * The keyword is an optional part of FCS files.
     *
     *
     * <B>History</B><BR>
     * The "$TR" keyword was introduced in the 3.0 FCS specification.
     *
     * @param[out] name
     *   Returns the short name for the parameter used as the trigger.
     * @param[out] threshold
     *   Returns the threshold used on the trigger during acquisition.
     *
     * @see getParameterShortName()
     */
    inline void getTriggerParameter( std::string& name, long& threshold )
        const noexcept
    {
        if ( this->inDictionary( "$TR" ) == false )
        {
            name.clear( );
            threshold = 0L;
        }

        try
        {
            // The value for the keyword is a comma-separated tuple: name,d.
            // - "name" is the short parameter name.
            // - "d" is the threshold.
            //
            // The threshold text is composed of digits and related
            // punctuation, which are entirely ASCII. This code searches
            // backwards from the end of the string to find a comma, then
            // parses the number after the comma. All characters from the
            // start of the string to the comma are used for the returned
            // parameter name. Because parsing never looks at those characters,
            // they can be UTF-8 without any special handling required.
            const auto s = this->getDictionaryString( "$TR" );
            const size_t commaPosition = s.rfind( ',' );
            name = s.substr( 0, commaPosition );
            threshold = std::stod( s.substr( commaPosition + 1 ) );
        }
        catch ( ... )
        {
            name.clear( );
            threshold = 0L;
        }
    }

// TODO set methods
    // @}



//----------------------------------------------------------------------
// Event attributes and data.
//----------------------------------------------------------------------
public:
    /**
     * @name Event attributes
     */
    // @{
    /**
     * Returns the number of events.
     *
     *
     * <B>Dictionary entry</B><BR>
     * When a file is loaded, this value is initialized from the "$TOT"
     * keyword value in the dictionary. Changes to the event table also
     * affect this value.
     *
     * @return
     *   Returns the number of events.
     *
     * @see ::getEventTable()
     * @see ::getNumberOfAbortedEvents()
     * @see ::getNumberOfLostEvents()
     */
    inline size_t getNumberOfEvents( )
        const noexcept
    {
        if ( this->eventTable != nullptr )
            return this->eventTable->getNumberOfEvents( );
        if ( this->inDictionary( "$TOT" ) == false )
            return 0L;
        return this->getDictionaryLong( "$TOT" );
    }

    /**
     * Returns the number of events in the file.
     *
     * When an event table is loaded from a file, and only a subset of
     * events are loaded, then the value returned here is the total number
     * of events in that file.
     *
     * @return
     *   Returns the number of original events.
     *
     * @see ::areValuesFloats()
     * @see ::getNumberOfEvents()
     * @see ::getParameterFloats()
     * @see ::getParameterDoubles()
     * @see ::resize()
     * @see ::setNumberOfOriginalEvents()
     */
    size_t getNumberOfOriginalEvents( )
        const noexcept
    {
        return this->numberOfFileEvents;
    }

    /**
     * Returns the number of events lost due to electronics issues.
     *
     * Aborted events are those lost due to data acquisition electronic
     * coincidence effects.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$ABRT" keyword.
     * The keyword is an optional part of FCS files.
     *
     * @return
     *   Returns the number of events that were lost due to aborts.
     *   If the value is not known, a zero is returned.
     *
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfLostEvents()
     */
    inline size_t getNumberOfAbortedEvents( )
        const noexcept
    {
        if ( this->inDictionary( "$ABRT" ) == false )
            return 0;
        return this->getDictionaryLong( "$ABRT" );
    }

    /**
     * Returns the number of events lost due to the computer being busy.
     *
     *
     * <B>Dictionary entry</B><BR>
     * This method returns the dictionary value of the "$LOST" keyword.
     * The keyword is an optional part of FCS files.
     *
     * @return
     *   Returns the number of events that were lost during data acquisition
     *   due to the computer being busy. If the value is not known, a zero
     *   is returned.
     *
     * @see ::getNumberOfEvents()
     * @see ::getNumberOfAbortedEvents()
     */
    inline size_t getNumberOfLostEvents( )
        const noexcept
    {
        if ( this->inDictionary( "$LOST" ) == false )
            return 0;
        return this->getDictionaryLong( "$LOST" );
    }
    // @}



//----------------------------------------------------------------------
// Event clear and copy.
//----------------------------------------------------------------------
    /**
     * @name Event clear and copy
     */
    // @{
private:
    /**
     * Resets file fields, while leaving the dictionary and event table
     * unaffected.
     *
     * @see ::reset()
     */
    inline void resetFileFields( )
    {
        this->clearFileLog();
        this->fileAttributes.clear( );
        this->fileSize = 0L;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        this->fileIsLSBF = true;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        this->fileIsLSBF = false;
#endif
        this->fileDataType = 'F';
        this->fileMaxParameterBytes = 0L;
        this->fileMinParameterBytes = 0L;
        this->fileMaxParameterRange = 0L;
        this->fileDictionaryValueDelimiter = '/';
    }

public:
    /**
     * Resets the object, clearing it of all content.
     */
    inline void reset( )
    {
        this->resetFileFields( );
        this->dictionary.clear( );
        this->eventTable.reset( );
    }



    // Copy event table ------------------------------------------------
    /**
     * Copies the given event table and clears the dictionary.
     *
     * The FCS file object is reset, while leaving the dictionary as-is.
     *
     * The parameter names and event array of the given event table are
     * copied into a new table stored within the object. The new table is
     * used by further methods, such as to save the event table to an
     * file.
     *
     * <B>Performance note</B><BR>
     * Copying the event table requires allocating memory and copying values
     * from the given table to a new table. To avoid this memory use and copy
     * time, pass the event table within a shared pointer. The constructed
     * file object will then share the same instance of the event table.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Events::EventTableInterface*const eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize file fields, but do not touch dictionary.
        this->resetFileFields( );

        // Copy.
        this->copy( *eventTable );
    }

    /**
     * Copies the given event table and clears the dictionary.
     *
     * The FCS file object is reset, while leaving the dictionary as-is.
     *
     * The parameter names and event array of the given event table are
     * copied into a new table stored within the object. The new table is
     * used by further methods, such as to save the event table to an
     * file.
     *
     * <B>Performance note</B><BR>
     * Copying the event table requires allocating memory and copying values
     * from the given table to a new table. To avoid this memory use and copy
     * time, pass the event table within a shared pointer. The constructed
     * file object will then share the same instance of the event table.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @throws std::invalid_argument
     *   Throws an invalid argument exception if the event table is a nullptr.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::reset()
     */
    inline void copy(
        const std::shared_ptr<FlowGate::Events::EventTableInterface>& eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize file fields, but do not touch dictionary.
        this->resetFileFields( );

        // Copy.
        this->copy( *(eventTable.get( )) );
    }

    /**
     * Copies the given event table and clears the dictionary.
     *
     * The FCS file object is reset, while leaving the dictionary as-is.
     *
     * The parameter names and event array of the given event table are
     * copied into a new table stored within the object. The new table is
     * used by further methods, such as to save the event table to an
     * file.
     *
     * <B>Performance note</B><BR>
     * Copying the event table requires allocating memory and copying values
     * from the given table to a new table. To avoid this memory use and copy
     * time, pass the event table within a shared pointer. The constructed
     * file object will then share the same instance of the event table.
     *
     * @param[in] eventTable
     *   An event table to copy.
     *
     * @see ::load()
     * @see ::save()
     * @see ::getEventTable()
     * @see ::setEventTable()
     * @see ::reset()
     */
    inline void copy( const FlowGate::Events::EventTableInterface& eventTable )
        noexcept
    {
        // Initialize file fields, but do not touch dictionary.
        this->resetFileFields( );

        // Copy.
        this->eventTable.reset( new FlowGate::Events::EventTable( eventTable ) );

        // Copy the number of parameters and events into fields and into
        // the dictionary.
        const size_t numberOfEvents     = this->eventTable->getNumberOfEvents( );
        const size_t numberOfParameters = this->eventTable->getNumberOfParameters( );
        this->numberOfFileEvents = this->eventTable->getNumberOfOriginalEvents( );


        this->setDictionaryStringInternal(
            "$TOT",
            std::to_string( numberOfEvents ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
        this->setDictionaryStringInternal(
            "$PAR",
            std::to_string( numberOfParameters ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        // Copy parameter names into the dictionary.
        const auto parameterNames = this->eventTable->getParameterNames( );
        for ( size_t i = 0; i < parameterNames.size( ); ++i )
        {
            this->setDictionaryStringInternal(
                buildParameterKeyword( "$P", i, "N" ),
                parameterNames[i],
                false,      // No need to convert keyword or trim.
                true );     // Trim the value.
        }

        // Update file data type.
        this->fileDataType = (this->eventTable->areValuesFloats( ) == true) ?
            'F' : 'D';
    }
    // @}



//----------------------------------------------------------------------
// Event data.
//----------------------------------------------------------------------
public:
    /**
     * @name Event data
     */
    // @{
    /**
     * Gets the event table in use.
     *
     * If a file has not been loaded and an event table has not been
     * initialized, then a NULL pointer is returned.
     *
     * @param[in] eventTable
     *   Returns a shared pointer for the event table.
     *
     * @see ::load()
     * @see ::setEventTable()
     */
    inline std::shared_ptr<const FlowGate::Events::EventTableInterface> getEventTable( )
        const
    {
        return this->eventTable;
    }

    /**
     * Gets the event table in use.
     *
     * If a file has not been loaded and an event table has not been
     * initialized, then a NULL pointer is returned.
     *
     * @param[in] eventTable
     *   Returns a shared pointer for the event table.
     *
     * @see ::load()
     * @see ::setEventTable()
     */
    inline std::shared_ptr<FlowGate::Events::EventTableInterface> getEventTable( )
    {
        return this->eventTable;
    }

    /**
     * Resets the object, except for the dictionary, and sets the event
     * table to use.
     *
     * The prior event table, if any, is released. The new event table
     * will be used for all further operations, such as saving to a file.
     *
     * @param[in] eventTable
     *   The event table to use.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the event table is a nullptr.
     *
     * @see ::reset()
     * @see ::copy()
     */
    inline void setEventTable(
        std::shared_ptr<FlowGate::Events::EventTableInterface>& eventTable )
    {
        // Validate.
        if ( eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        // Initialize file fields, but do not touch dictionary.
        this->resetFileFields( );

        // Set event table.
        this->eventTable = eventTable;

        // Copy the number of parameters and events into fields and into
        // the dictionary.
        const size_t numberOfEvents     = this->eventTable->getNumberOfEvents( );
        const size_t numberOfParameters = this->eventTable->getNumberOfParameters( );

        this->setDictionaryStringInternal(
            "$TOT",
            std::to_string( numberOfEvents ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
        this->setDictionaryStringInternal(
            "$PAR",
            std::to_string( numberOfParameters ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        // Copy parameter names into the dictionary.
        const auto parameterNames = this->eventTable->getParameterNames( );
        for ( size_t i = 0; i < parameterNames.size( ); ++i )
        {
            this->setDictionaryStringInternal(
                buildParameterKeyword( "$P", i, "N" ),
                parameterNames[i],
                false,      // No need to convert keyword or trim.
                true );     // Trim the value.
        }

        // Update file data type.
        this->fileDataType = (this->eventTable->areValuesFloats( ) == true) ?
            'F' : 'D';
    }
    // @}



//----------------------------------------------------------------------
// Event scaling.
//----------------------------------------------------------------------
public:
    /**
     * @name Event scaling
     */
    // @{
    /**
     * Performs scaling of all values of all parameters.
     *
     * All parameters are scaled, one at a time, using scaleParameter().
     *
     * @see ::load()
     * @see ::isAutoScale()
     * @see ::setAutoScale()
     * @see ::getParameterScaling()
     * @see ::getParameterRange()
     * @see ::scaleParameter()
     */
    void scaleAllParameters( )
    {
        for ( size_t i = 0; i < this->getNumberOfParameters( ); ++i )
            scaleParameter( i );
    }

    /**
     * Performs scaling of all values of the selected parameter.
     *
     * The scaling applied to a parameter depends upon the values of several
     * standard keywords:
     *
     * @li "$PnE" indicates if channel data should be scaled using linear or
     *     log scaling. For log scaling, the parameter provides the number of
     *     decades used in the log scale, and the offset.
     *
     * @li "$PnG" indicates the scale factor used for linear scaling, if
     *     the values in "$PnE" indicate linear scaling is used.
     *
     * @li "$PnR" indicates the numeric range. This range is needed to apply
     *     log scaling.
     *
     * Linear scaling using "$PnG" can be applied to integer, and single- and
     * double-precision floating point data.
     *
     * Log scaling using "$PnE", however, is supposed to only be applied to
     * integer event data. However, vendors sometimes use it for floating-point
     * data as well.
     *
     * The scaling factors needed are returned by getParameterScaling()
     * and the range is returned by getParameterRange().
     *
     * For linear scaling, integer or floating-point channel values are
     * scaled using the formula:
     * @code
     * scaled = channel / scale
     * @endcode
     * where "channel" is the unscaled integer data from the file, "scaled"
     * is the scaled result, and "scale" is the linear scale factor. All math
     * is done in floating point.
     *
     * For logarithmic scaling, integer channel values may be scaled using
     * the formula:
     * @code
     * scaled = pow( 10, (decades * channel / range)) * offset
     * @endcode
     * where "channel" is the unscaled integer data from the file, "scaled"
     * is the scaled result, "decades" and "offset" are the number of log
     * decades and the log offset, and "range" is the integer range of the
     * parameter. All math is done in floating point.
     *
     * <B>Multi-threading</B><BR>
     * Parameter scaling is computed in parallel using multiple threads,
     * when available.
     *
     * @param[in] index
     *   The parameter index.
     * @param[in] cleanKeywords
     *   (optional, default = true) When true, the parameter's "$PnG" keyword
     *   and value are removed, and its "$PnR" and "$PnE" are reset to defaults
     *   that prevent further redundant scaling of the same parameter.
     *   When false, these keywords are retained and it is the caller's
     *   responsibility to insure that redundant scaling is not done. For
     *   instance, the caller should insure that these keywords are not
     *   saved to a new FCS file with the same data.
     *
     * @throws std::out_of_range
     *   Throws an exception if the parameter index is out of range.
     * @throws std::runtime_error
     *   Throws an exception if the scaling factors or range needed are not
     *   available in the dictionary, or if their values are malformed.
     *
     * @see ::load()
     * @see ::isAutoScale()
     * @see ::setAutoScale()
     * @see ::getParameterScaling()
     * @see ::getParameterRange()
     * @see ::scaleAllParameters()
     */
    void scaleParameter( const size_t index, const bool cleanKeywords = true )
    {
        //
        // Validate.
        // ---------
        // Complain if the parameter index is out of range.
        if ( this->eventTable == nullptr )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Invalid NULL event table." );

        if ( index < 0 || index >= this->getNumberOfParameters() )
            throw std::out_of_range(
                ERROR_PROGRAMMER +
                "Parameter index out of range." );

        //
        // Scale.
        // ------
        // Determine if the parameter needs to be scaled linearly,
        // logarithmically, or not at all. Then do it.
        std::string scaleType;
        double value1;
        double value2;
        this->getParameterScaling( index, scaleType, value1, value2 );

        // Get the parameter range. If this is zero, then the parameter
        // value is malformed and we won't scale.
        const double range = this->getParameterRange( index );
        if ( range <= 0.0 )
            throw std::runtime_error(
                ERROR_MALFORMED +
                std::string( "The range for parameter " ) +
                std::to_string( index ) +
                std::string( " is invalid. It must be greater zero." ) );

        std::pair<double,double> newRange;

        if ( scaleType == "linear" )
        {
            // Linear scaling. getParameterScaling() returns a gain
            // factor in value1. value2 is unused.
            //
            // If the gain is 1.0, then no scaling is needed.
            //
            // If the gain is 0.0 or negative, then the scaling value
            // is malformed.
            if ( value1 <= 0.0 )
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The linear gain factor for parameter " ) +
                    std::to_string( index ) +
                    std::string( " is invalid. It must be greater than zero." ) );

            if ( value1 == 1.0 )
            {
                // Linear scaling with a factor of 1 means no scaling.
                // Just save the existing range as the new range.
                newRange.first = 0.0;
                newRange.second = range;
            }
            else
                newRange = this->scaleLinear( index, range, value1 );
        }
        else if ( scaleType == "log" )
        {
            // Log scaling. getParameterScaling() returns the decades and
            // offset in value1 and value2.
            //
            // Zero values have already been caught and mapped to linear gain.
            //
            // Negative values are malformed.
            if ( value1 < 0.0 || value2 < 0.0 )
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "The log gain decades and/or offset for parameter " ) +
                    std::to_string( index ) +
                    std::string( " are invalid. Both must be greater than or equal to zero." ) );

            newRange = this->scaleLog( index, range, value1, value2 );
        }

        //
        // Update keywords.
        // ----------------
        // The keywords used for scaling are either removed or updated so
        // that the dictionary reflects the current scaled state of the events.
        //
        // Clear gain. The keyword is optional and defaults to
        // 1.0 gain when not present.
        this->eraseDictionaryEntry(
            this->buildParameterKeyword( "$P", index, "G" ) );

        // Reset log scaling to 0,0. The keyword is required.
        this->setDictionaryStringInternal(
            this->buildParameterKeyword( "$P", index, "E" ),
            "0,0",
            false,      // Do not convert or trim the keyword.
            false );    // Do not trim the value.

        // Reset the range. The keyword is required.
        this->setDictionaryDouble(
            this->buildParameterKeyword( "$P", index, "R" ),
            newRange.second );

        // Update the data min/max.
        this->eventTable->computeParameterDataMinimumMaximum( index );

        // Update the specified min/max.
        this->eventTable->setParameterMinimum( index, newRange.first );
        this->eventTable->setParameterMaximum( index, newRange.second );
    }



private:
    /**
     * Performs linear scaling on all values of the selected parameter.
     *
     * Once scaled, this method should not be called again on the same
     * parameter.
     *
     * Channel values are scaled using the formula:
     * @code
     * scaled = channel / scale
     * @endcode
     * where "channel" is the unscaled integer data from the file, "scaled"
     * is the scaled result, and "scale" is the linear scale factor. All math
     * is done in floating point.
     *
     * @param[in]
     *   The parameter index.
     * @param[in] range
     *   The original range.
     * @param[in] gain
     *   The gain factor.
     *
     * @return
     *   Returns the computed new (min,max) range by scaling the parameter's
     *   specified minimum and maximum in the same way as event values are
     *   scaled.
     */
     std::pair<double,double> scaleLinear(
        const size_t index,
        const double range,
        const double gain )
        noexcept
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "    Linear scale parameter " << index << ", \"" <<
                this->eventTable->getParameterName( index ) << "\"\n";

        const size_t numberOfEvents =
            this->eventTable->getNumberOfEvents( );

        double newMax = range;
        double newMin = 0.0;

        if ( this->eventTable->areValuesFloats( ) == true )
        {
            auto values = this->eventTable->getParameterFloats( index );
            float*const e = values.data( );
            const float fmult = (float)(1.0f / gain);

#pragma omp parallel for simd
            for ( size_t i = 0; i < numberOfEvents; ++i )
                e[i] *= fmult;

            newMax *= fmult;
        }
        else
        {
            auto values = this->eventTable->getParameterDoubles( index );
            double*const e = values.data( );
            const double dmult = (double)(1.0 / gain);

#pragma omp parallel for simd
            for ( size_t i = 0; i < numberOfEvents; ++i )
                e[i] *= dmult;

            newMax *= dmult;
        }

        // The specified minimum for the parameter is always presumed to
        // be zero (because FCS has no way to say otherwise). When linearly
        // scaled, the minimum is still zero.
        return std::make_pair(newMin, newMax);
    }

    /**
     * Performs logarithmic scaling on all values of the selected parameter.
     *
     * Once scaled, this method should not be called again on the same
     * parameter.
     *
     * Channel values may be scaled using the formula:
     * @code
     * scaled = pow( 10, (decades * channel / range)) * offset
     * @endcode
     * where "channel" is the unscaled integer data from the file, "scaled"
     * is the scaled result, "decades" and "offset" are the number of log
     * decades and the log offset, and "range" is the integer range of the
     * parameter. All math is done in floating point.
     *
     * @param[in]
     *   The parameter index.
     * @param[in] range
     *   The original range.
     * @param[in] decades
     *   The number of log decades.
     * @param[in] offset
     *   The log offset.
     *
     * @return
     *   Returns the computed new range by scaling the range in the same way
     *   as event values are scaled.
     */
     std::pair<double,double> scaleLog(
        const size_t index,
        const double range,
        const double decades,
        const double offset )
    {
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "    Log scale parameter " << index << ", \"" <<
                this->eventTable->getParameterName( index ) << "\"\n";

        const size_t numberOfEvents =
            this->eventTable->getNumberOfEvents( );

        double newMax = range;
        double newMin = 0.0;

        if ( this->eventTable->areValuesFloats( ) == true )
        {
            auto values = this->eventTable->getParameterFloats( index );
            float*const e = values.data( );
            const float expMult = decades / range;

#pragma omp parallel for schedule(static)
            for ( size_t i = 0; i < numberOfEvents; ++i )
                e[i] = (float)(std::pow( 10, expMult * e[i] ) * offset);

            newMax = (float)(std::pow( 10, expMult * (float)newMax ) * offset);
            newMin = (float)(std::pow( 10, expMult * (float)newMin ) * offset);
        }
        else
        {
            auto values = this->eventTable->getParameterDoubles( index );
            double*const e = values.data( );
            const double expMult = decades / range;

#pragma omp parallel for schedule(static)
            for ( size_t i = 0; i < numberOfEvents; ++i )
                e[i] = std::pow( 10, expMult * e[i] ) * offset;

            newMax = (std::pow( 10, expMult * newMax ) * offset);
            newMin = (std::pow( 10, expMult * newMin ) * offset);
        }

        return std::make_pair(newMin, newMax);
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
     * Loads the indicated file's FCS data.
     *
     * The file is loaded into the current object, clearing out any prior
     * data. Upon completion, each of the "get" methods in this class will
     * return information loaded from the file.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] filePath
     *   The path to the file to load.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
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
        const ssize_t maximumEvents = (-1) )
    {
        if ( filePath == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid NULL file path." );

        this->load( std::string( filePath ), maximumEvents );
    }

    /**
     * Loads the indicated file's FCS data.
     *
     * The file is loaded into the current object, clearing out any prior
     * data. Upon completion, each of the "get" methods in this class will
     * return information loaded from the file.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] filePath
     *   The path to the file to load.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
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
        const std::string filePath,
        ssize_t maximumEvents = (-1) )
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
                "Loading FCS file \"" << filePath << "\".\n";

        this->reset( );

        std::string path = filePath;
        this->fileAttributes.emplace( "path", filePath );


        //
        // Open the file.
        // --------------
        // Open the file for binary buffered reading using stdio.
        // Throw an exception if the file cannot be found or opened.
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
        // the byte offset to the TEXT segment. Then load the TEXT and
        // SUPPLEMENTAL TEXT segments to initialize the dictionary.
        // Finally, load the data.
        try
        {
#ifdef FLOWGATE_FILEFCS_USE_FLOCKFILE
            // Thread lock during the load. By locking now, all further
            // stdio calls will go faster.
            flockfile( fp );
#endif

            // Get the file size. This is used during validation to check
            // that byte offsets are valid. The file size field is initialized
            // to zero here. If we can't get the file size, then it remains
            // zero and byte offset validations are not done.
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

            // Load the header. This provides the file format version in use
            // and byte offsets to the TEXT, DATA, and ANALYSIS segments.
            // However, only the TEXT byte offset is guaranteed to be present.
            this->loadHeader( fp );

            // Validate the header. Make sure the file is in a format version
            // this software supports and check that the segment offsets
            // are meaningful.
            this->loadValidateHeader( );

            // Read the dictionary from the TEXT segment.
            this->loadTextSegment( fp, true );

            // Validate that essential information that should be in the
            // TEXT segment was there and is correct.
            this->loadValidateTextSegment( );

            // Read more of the dictionary from the SUPPLEMENTAL TEXT segment,
            // if there is one.
            this->loadTextSegment( fp, false );

            // Validate that all needed values are present and reasonable.
            this->loadValidateReadyToLoadData( );

            // Read the data from the DATA segment.
            this->loadDataSegment( fp, maximumEvents );

            if ( this->fileAutoScale == true )
            {
                // Auto-scale data. The last step of this is to compute
                // the data min/max from parameter values and to set the
                // specified min/max by scaling the original min/max.
                if ( this->verbose == true )
                    std::cerr << this->verbosePrefix << ": " <<
                        "  Auto-scaling\n";

                this->scaleAllParameters( );
            }
            else
            {
                // Compute the data min/max from parameter values.
                this->eventTable->computeParameterDataMinimumMaximum( );

                // Set the specified min/max from the dictionary.
                // Unfortunately, the dictionary only supports a maximum
                // value. The minimum is presumed to be zero.
                //
                // The maximum also could be zero if the dictionary was
                // not set properly.
                const size_t numberOfParameters =
                    this->eventTable->getNumberOfParameters( );
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                {
                    this->eventTable->setParameterMinimum( i, 0.0 );
                    this->eventTable->setParameterMaximum( i,
                        this->getParameterRange( i ) );
                }
            }

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": Parameter min/max:\n";
                const size_t numberOfParameters =
                    this->eventTable->getNumberOfParameters( );
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                {
                    std::cerr << this->verbosePrefix << ":   " <<
                        this->eventTable->getParameterName( i ) << "\n";
                    std::cerr << this->verbosePrefix << ":     Specified min, max: " <<
                        this->eventTable->getParameterMinimum( i ) << ", " <<
                        this->eventTable->getParameterMaximum( i ) << "\n";
                    std::cerr << this->verbosePrefix << ":     Data min, max: " <<
                        this->eventTable->getParameterDataMinimum( i ) << ", " <<
                        this->eventTable->getParameterDataMaximum( i ) << "\n";
                    std::cerr << this->verbosePrefix << ":     Best min, max: " <<
                        this->eventTable->getParameterBestMinimum( i ) << ", " <<
                        this->eventTable->getParameterBestMaximum( i ) << "\n";
                }
            }

            // This software skips the following deprecated, rarely-used,
            // or unsupported features:
            // - The ANALYSIS segment.
            // - Any additional data segments.
            // - Any additional data.

#ifdef FLOWGATE_FILEFCS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif

            std::fclose( fp );

            // Loading temporarily added $BEGINTEXT and $ENDTEXT to the
            // dictionary, though neither of these are defined in the FCS
            // specification. Remove them.
            this->eraseDictionaryEntry( "$BEGINTEXT" );
            this->eraseDictionaryEntry( "$ENDTEXT" );

            // Clear all file storage keywords from the dictionary. These
            // are only relevant during file loading.
            this->cleanByCategory( FCSKeywordCategory::FILESTORAGE );
        }
        catch ( const std::exception& e )
        {
#ifdef FLOWGATE_FILEFCS_USE_FLOCKFILE
            // Unlock.
            funlockfile( fp );
#endif
            std::fclose( fp );

            // Loading temporarily added $BEGINTEXT and $ENDTEXT to the
            // dictionary, though neither of these are defined in the FCS
            // specification. Remove them.
            this->eraseDictionaryEntry( "$BEGINTEXT" );
            this->eraseDictionaryEntry( "$ENDTEXT" );

            // Clear all file storage keywords from the dictionary. These
            // are only relevant during file loading.
            this->cleanByCategory( FCSKeywordCategory::FILESTORAGE );

            throw;
        }
    }

private:
    /**
     * Gets the size, in bytes, of each parameter.
     *
     * The parameter's size is used to indicate how many bytes are loaded
     * from the file. These values are only available after the file's
     * dictionary has been loaded, but before file loading completes. On
     * file loading completion, these keywords are cleaned from the
     * dictionary.
     *
     * @return
     *   Returns an array with one entry per parameter, indicating the
     *   size in bytes for that parameter.
     */
    size_t* buildParameterSizes( )
        const noexcept
    {
        const size_t nParam = this->getNumberOfParameters( );
        size_t*const bytesPerParameter = new size_t[nParam];

        for ( size_t j = 0; j < nParam; ++j )
        {
            auto keyword = this->buildParameterKeyword( "$P", j, "B" );
            if ( this->inDictionary( keyword ) == false )
                bytesPerParameter[j] = 0;
            else
                // Divide by 8 to convert bits to bytes.
                bytesPerParameter[j] = this->getDictionaryLong( keyword ) / 8;
        }

        return bytesPerParameter;
    }

    /**
     * Computes a bitmask to limit an integer parameter value to its range.
     *
     * The parameter's range is used to compute a bitmask that masks off the
     * upper bits of values loaded from a file. If the range is not given
     * or invalid, the mask for the parameter is set to full range.
     *
     * @return
     *   Returns an allocated array of masks with one entry per parameter.
     */
    template <typename UINTTYPE>
    UINTTYPE* buildParameterMasks( )
        const noexcept
    {
        const size_t nParam = this->getNumberOfParameters( );
        UINTTYPE*const masks = new UINTTYPE[nParam];

        for ( size_t j = 0; j < nParam; ++j )
        {
            // Get the maximum value for the parameter. Even though the
            // range is only really useful for integer parameters, the
            // specification requires that the range value be floating point.
            const double drange = this->getParameterRange( j );
            UINTTYPE mask = (UINTTYPE) ~0u;

            // If the range is zero or negative, then it is invalid and
            // we treat it as meaning full-range for the data type.
            if ( drange <= 0.0 )
            {
                masks[j] = mask;
                continue;
            }

            // If the range is larger than the integer data type can store,
            // Then it is invalid, or at least not very useful, and again
            // we treat it as meaning full-range for the data type.
            if ( drange > (double) mask )
            {
                masks[j] = mask;
                continue;
            }

            // Otherwise the range is relevant. Cast it down to our
            // mask data type.
            const UINTTYPE range = (UINTTYPE) drange;

            // Compute a mask just big enough for the maximum range.
            while ( (range & ~(mask >> 1)) == 0 )
                mask >>= 1;
            masks[j] = mask;
        }

        return masks;
    }

    /**
     * Masks raw data by parameter-specific masks and casts to a float type.
     *
     * A new floating point data array is allocated and its values set to
     * the source data, masked by the given parameter masks.
     *
     * @param[in] destination
     *   The destination for the masked data.
     * @param[in] source
     *   The source data to be masked and copied.
     * @param[in] masks
     *   The per-parameter masks.
     */
    template <typename FLOATTYPE, typename UINTTYPE>
    void maskAndCopy(
        FLOATTYPE*const destination,
        const UINTTYPE*const source,
        const UINTTYPE*const masks )
        const noexcept
    {
        const size_t nEvents = this->getNumberOfEvents( );
        const size_t nParam  = this->getNumberOfParameters( );
        const size_t nValues = nEvents * nParam;

        // Sweep through the parameter masks and get the minimum mask.
        UINTTYPE minMask = 0;
        for ( size_t j = 0; j < nParam; ++j )
            if ( masks[j] < minMask )
                minMask = masks[j];

        // If the minimum mask is the full range of the data type,
        // then no masking is needed and we can just copy.
        if ( minMask == (UINTTYPE)(~0u) )
        {
            for ( size_t i = 0; i < nValues; ++i )
                destination[i] = (FLOATTYPE)source[i];
        }
        else
        {
            for ( size_t i = 0, k = 0; i < nEvents; ++i )
                for ( size_t j = 0; j < nParam; ++j, ++k )
                    destination[k] = (FLOATTYPE)(source[k] & masks[j]);
        }
    }

    /**
     * Loads the file header containing the version number and segment offsets.
     *
     * The FCS file header is in ASCII and contains:
     * - The file format's version number (e.g. "FCS3.1").
     * - The start & end byte offsets of the TEXT segment.
     * - The start & end byte offsets of the DATA segment.
     * - The start & end byte offsets of the ANALYSIS segment.
     *
     * This method reads the header, leaving the file pointer on the first
     * byte after the header. The version number is read, parsed, and saved
     * into this object. The offsets are read, parsed, and returned as a
     * segment offsets object.
     *
     * Segment offsets for the TEXT segment are required to be valid.
     * DATA and ANALSYSIS segment offsets may be zeroes. When zero,
     * the true size is found in a variable in the TEXT segment.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's content cannot be parsed.
     *
     * @see ::loadValidateHeader()
     */
    void loadHeader( std::FILE*const fp )
    {
        if ( this->verbose == true )
        {
            if ( this->fileSize == 0 )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "File size:" <<
                    "unknown\n";
            else
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "File size:" <<
                    this->fileSize << " bytes\n";
        }


        //
        // Validate file size.
        // -------------------
        // The FCS file header contains the following:
        // - 6 bytes for the file format version.
        // - 4 bytes of blanks.
        // - Three pairs of 8-byte offsets in ASCII.
        //
        // The total is then (6 + 4 + 3*2*8) = 58 bytes. Make sure the
        // file has at least that much in it.
        if ( this->fileSize != 0 && this->fileSize < 58 )
        {
            this->appendFileLog(
                "error",
                "The file is too short to be a valid FCS file. A header of at least 58 bytes is required." );
            throw std::runtime_error(
                std::string( "Unrecognized data file format.\n" ) +
                std::string( "The file does not use a recognized format for flow cytometry data, such as the FCS format." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Read the version number.
        // ------------------------
        // Read and parse the version number.
        // - First 6 bytes.
        // - In ASCII.
        // - Has the format "FCS" followed by major.minor (e.g. "FCS3.1").
        char buffer[9];
        buffer[6] = '\0';
        if ( std::fread( buffer, 1, 6, fp ) != 6 )
        {
            this->appendFileLog(
                "error",
                "A read error occurred while trying to read the first 6 bytes of the file, which should contain the FCS format's version number." );
            throw std::runtime_error(
                ERROR_READ +
                ERROR_CANNOTLOAD );
        }

        if ( buffer[0] != 'F' || buffer[1] != 'C' || buffer[2] != 'S' )
        {
            this->appendFileLog(
                "error",
                "The file is not an FCS file. It does not start with \"FCS\"." );
            throw std::runtime_error(
                std::string( "Unrecognized data file format.\n" ) +
                std::string( "The file does not use a recognized format for flow cytometry data, such as the FCS format." ) +
                ERROR_CANNOTLOAD );
        }

        // Save the version number.
        this->fileAttributes.emplace( "versionNumber", buffer );
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "File format version:" <<
                "\"" << buffer << "\"\n";


        //
        // Skip blanks.
        // ------------
        // Skip blank space.
        // - Next 4 bytes.
        if ( std::fread( buffer, 1, 4, fp ) != 4 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header, immediately after the format's version number." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }


        //
        // Read segment offsets.
        // ---------------------
        // - Three pairs of 8-byte offsets.
        //   1. Begin and end of TEXT segment.
        //   2. Begin and end of DATA segment.
        //   3. Begin and end of ANALYSIS segment.
        // - In ASCII.
        //
        // Text start/end.
        // ---------------
        // Required positive integers.
        buffer[8] = '\0';
        if ( std::fread( buffer, 1, 8, fp ) != 8 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header at the text segment beginning byte offset." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        ssize_t textBegin = 0;
        sscanf( buffer, "%ld", &textBegin );

        if ( std::fread( buffer, 1, 8, fp ) != 8 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header at the text segment ending byte offset." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        ssize_t textEnd = 0;
        sscanf( buffer, "%ld", &textEnd );


        //
        // Data start/end.
        // ---------------
        // Optional positive integers. If zeroes, the start/end are
        // too large and are in the TEXT segment's dictionary.
        if ( std::fread( buffer, 1, 8, fp ) != 8 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header at the data segment beginning byte offset." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        ssize_t dataBegin = 0;
        sscanf( buffer, "%ld", &dataBegin );

        if ( std::fread( buffer, 1, 8, fp ) != 8 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header at the data segment ending byte offset." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        ssize_t dataEnd = 0;
        sscanf( buffer, "%ld", &dataEnd );


        //
        // Analysis start/end.
        // -------------------
        // Optional positive integers. If zeroes, the start/end are
        // too large and are in the TEXT segment's dictionary.
        if ( std::fread( buffer, 1, 8, fp ) != 8 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header at the analysis segment beginning byte offset." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        ssize_t analysisBegin = 0;
        sscanf( buffer, "%ld", &analysisBegin );

        if ( std::fread( buffer, 1, 8, fp ) != 8 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated within the file header at the analysis segment ending byte offset." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        ssize_t analysisEnd = 0;
        sscanf( buffer, "%ld", &analysisEnd );


        //
        // Save for later.
        // ---------------
        // Initialize the dictionary with the begin/end values. Use
        // keywords from FCS 3.0 and 3.1.
        this->setDictionaryStringInternal(
            "$BEGINDATA",
            std::to_string( dataBegin ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
        this->setDictionaryStringInternal(
            "$ENDDATA",
            std::to_string( dataEnd ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
        this->setDictionaryStringInternal(
            "$BEGINANALYSIS",
            std::to_string( analysisBegin ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
        this->setDictionaryStringInternal(
            "$ENDANALYSIS",
            std::to_string( analysisEnd ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        // There is no dictionary keyword for the beginning and ending of
        // the TEXT segment because dictionary keywords come from the TEXT
        // segment. There would be no way to get a begin/end TEXT from the
        // TEXT segment without first knowing the begin/end of the TEXT
        // segment!
        //
        // For now... create fake keyword entries for the begin/end of
        // the TEXT segment. We'll delete these later at the end of loading.
        this->setDictionaryStringInternal(
            "$BEGINTEXT",
            std::to_string( textBegin ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
        this->setDictionaryStringInternal(
            "$ENDTEXT",
            std::to_string( textEnd ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.
    }

    /**
     * Validates that the loaded header values are reasonable.
     *
     * Loading the header has set dictionary entries based upon values found
     * in the header. The following keywords and values are validated:
     * @li $BEGINANALYSIS
     * @li $BEGINDATA
     * @li $BEGINTEXT
     * @li $ENDANALYSIS
     * @li $ENDDATA
     * @li $ENDTEXT
     *
     * The $BEGINTEXT and $ENDTEXT are not FCS specification keywords and
     * are automatically deleted after the file is loaded.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's header values are not reasonable.
     */
    void loadValidateHeader( )
    {
        long segmentBegin = 0L;
        long segmentEnd   = 0L;

        //
        // Validate version number.
        // ------------------------
        // The version number must be one of the known specification
        // versions:
        // @li 1.0
        // @li 2.0
        // @li 3.0
        // @li 3.1
        const std::string& versionNumber =
            this->fileAttributes.at( "versionNumber" );
        if ( versionNumber != "FCS1.0" &&
             versionNumber != "FCS2.0" &&
             versionNumber != "FCS3.0" &&
             versionNumber != "FCS3.1" )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header indicates the file is an FCS file, but the version number is not recognized: \"" ) +
                versionNumber + "\"." );
            throw std::runtime_error(
                std::string( "Unsupported version of the FCS file format.\n" ) +
                std::string( "The file uses a version of the FCS file format that is not supported by this software: \"") +
                versionNumber +
                std::string( "\"." ) +
                ERROR_CANNOTLOAD );
        }


        //
        // Validate TEXT begin/end.
        // ------------------------
        // The TEXT segment is a required part of the file that contains
        // the dictionary of keyword-value pairs.
        //
        // The begin/end of the TEXT segment cannot be zero or malformed.
        segmentBegin = this->getDictionaryLong( "$BEGINTEXT" );
        segmentEnd   = this->getDictionaryLong( "$ENDTEXT" );

        if ( segmentBegin <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header value used to locate the start of the text segment and data dictionary has an invalid " ) +
                (segmentBegin == 0 ?
                    std::string( "zero" ) : std::string( "negative" )) +
                std::string( " value: \"" ) +
                std::to_string( segmentBegin ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( segmentEnd <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header value used to locate the end of the text segment and data dictionary has an invalid " ) +
                (segmentEnd == 0 ?
                    std::string( "zero" ) : std::string( "negative" )) +
                std::string( " value: \"" ) +
                std::to_string( segmentBegin ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( segmentEnd < segmentBegin )
        {
            this->appendFileLog(
                "error",
                std::string( "The file header value used to locate the end of the text segment and data dictionary has an invalid value earlier than the value for the start of the same text segment: \"" ) +
                std::to_string( segmentEnd ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( segmentEnd == segmentBegin )
        {
            this->appendFileLog(
                "error",
                "The file header values used to locate the start and end of the text segment and data dictionary have the same value, indicating an empty text segment. However, the text segment is a required part of the file and cannot be empty."  );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->fileSize != 0 )
        {
            if ( segmentBegin >= this->fileSize )
            {
                this->appendFileLog(
                    "error",
                    "The file is truncated before the start of the text segment data dictionary." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    ERROR_CANNOTLOAD );
            }

            if ( segmentEnd >= this->fileSize )
            {
                this->appendFileLog(
                    "error",
                    "The file is truncated before the end of the text segment data dictionary." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    ERROR_CANNOTLOAD );
            }
        }


        //
        // Validate DATA begin/end.
        // ------------------------
        // The DATA segment is a required part of the file that contains
        // the event data. This cannot be missing from the file.
        //
        // However, the FCS file header has only 8 ASCII bytes with which
        // to specify byte offsets to the begin/end of the segment. The
        // maximum value in 8 bytes is 99999999 = 10 Mbytes, which is far
        // too small for byte offsets for most data. To support bigger data,
        // FCS 3.0 and 3.1 allow the header offsets to be zeroes and then
        // new keywords in the TEXT segment set the actual offsets.
        //
        // So, either the offsets encountered so far must be valid, or they
        // must be zero.
        segmentBegin = this->getDictionaryLong( "$BEGINDATA" );
        segmentEnd   = this->getDictionaryLong( "$ENDDATA" );

        if ( segmentBegin != 0 )
        {
            if ( segmentBegin < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file header value used to locate the start of the data segment has an invalid negative value: \"" ) +
                    std::to_string( segmentBegin ) + std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            // Segment begin cannot be beyond the end of the file.
            if ( this->fileSize != 0 && segmentBegin >= this->fileSize )
            {
                this->appendFileLog(
                    "error",
                    "The file is truncated before the start of the data segment event data." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    ERROR_CANNOTLOAD );
            }

            // SPECIAL HANDLING: FCS files have been encountered that have
            // the DATA segment's beginning value set to a non-zero offset,
            // but the ending value is zero. Technically, this means the
            // end offset was too large for the header and therefore must
            // be in the TEXT segment. But this is a messy reading of the
            // specification and most FCS software notes that if either the
            // begin or end offsets are too large, then BOTH should be zero
            // in the header and set in the TEXT segment.
            //
            // Nevertheless, watch for a segment end that is zero, even
            // though the begin is not. Only validate the end value if it
            // is non-zero.
            if ( segmentEnd != 0 )
            {
                if ( segmentEnd < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file header value used to locate the end of the data segment has an invalid negative value: \"" ) +
                        std::to_string( segmentBegin ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd < segmentBegin )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file header value used to locate the end of the data segment has an invalid value earlier than the value for the start of the same data segment: \"" ) +
                        std::to_string( segmentEnd ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd == segmentBegin )
                {
                    this->appendFileLog(
                        "error",
                        "The file header values used to locate the start and end of the data segment have the same value, indicating an empty data segment. However, the data segment is a required part of the file and cannot be empty."  );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                // Segment end cannot be beyond the end of the file by more
                // than one byte.
                if ( this->fileSize != 0 && segmentEnd >= (this->fileSize + 1) )
                {
                    this->appendFileLog(
                        "error",
                        "The file is truncated before the end of the data segment event data." );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        ERROR_CANNOTLOAD );
                }
            }
            else
            {
                this->appendFileLog(
                    "warning",
                    "The FCS file's header has a non-zero byte offset for the start of the data segment, but a zero byte offset for the end. This does not make sense. If the start byte offset is non-zero, the end also must be. This software ignores the header's end byte offset in the hope that a correct byte offset will be found in the text segment as the value of $ENDDATA." );
            }
        }
        else if ( segmentEnd != 0 )
        {
            // SPECIAL HANDLING: If the segment begin is zero, then the segment
            // end cannot be anything but zero. Nevertheless, FCS files have
            // been seen that set the segment end to (-1) when the begin is
            // zero.
            //
            // Report the situation, then force the segment end to zero.
            this->appendFileLog(
                "warning",
                "The FCS file's header has a zero byte offset for the start of the data segment, but a non-zero byte offset for the end. This does not make sense. If the start byte offset is zero, the end also must be. This software ignores the header's end byte offset in the hope that the correct byte offset will be found in the text segment as the value of $ENDDATA." );

            this->setDictionaryStringInternal(
                "$ENDDATA",
                "0",
                false,      // Do not convert or trim keyword.
                false );    // Do not trim value.
        }


        //
        // Validate ANALYSIS begin/end.
        // ----------------------------
        // The ANALYSIS segment is an optional part of the file and it is
        // not used by this software. Nevertheless, the segment offsets
        // should be reasonable or we may have a malformed file.
        //
        // As with the DATA segment, the header's 8 ASCII bytes are often
        // insufficient to indicate byte offsets for a large file. FCS 3.0
        // and 3.1 allow bigger values to be set in the TEXT segment.
        //
        // So, either the offsets encountered so far must be valid, or they
        // must be zero.
        //
        // It is possible for the segment begin to be set to a non-zero
        // value, while the segment end is left at zero because it is too
        // large to include in the header. In this case, the values cannot
        // be validated until they are updated from the TEXT segment.
        segmentBegin = this->getDictionaryLong( "$BEGINANALYSIS" );
        segmentEnd   = this->getDictionaryLong( "$ENDANALYSIS" );

        if ( segmentBegin != 0 )
        {
            if ( segmentBegin < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file header value used to locate the start of the analysis segment has an invalid negative value: \"" ) +
                    std::to_string( segmentBegin ) + std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            // Segment begin cannot be beyond the end of the file.
            if ( this->fileSize != 0 && segmentBegin >= this->fileSize )
            {
                this->appendFileLog(
                    "error",
                    "The file is truncated before the start of the analysis segment event data." );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    ERROR_CANNOTLOAD );
            }

            // SPECIAL HANDLING: See earlier comments about the begin offset
            // being set in the header, but the end offset set to zero.
            if ( segmentEnd != 0 )
            {
                if ( segmentEnd < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file header value used to locate the end of the analysis segment has an invalid negative value: \"" ) +
                        std::to_string( segmentBegin ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd < segmentBegin )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file header value used to locate the end of the analysis segment has an invalid value earlier than the value for the start of the same analysis segment: \"" ) +
                        std::to_string( segmentEnd ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd == segmentBegin )
                {
                    this->appendFileLog(
                        "error",
                        "The file header values used to locate the start and end of the analysis segment have the same value, indicating an empty analysis segment." );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                // Segment end cannot be beyond the end of the file by more
                // than one byte.
                if ( this->fileSize != 0 && segmentEnd >= (this->fileSize + 1) )
                {
                    this->appendFileLog(
                        "error",
                        "The file is truncated before the end of the analysis segment." );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        ERROR_CANNOTLOAD );
                }
            }
            else
            {
                this->appendFileLog(
                    "warning",
                    "The FCS file's header has a non-zero byte offset for the start of the analysis segment, but a zero byte offset for the end. This does not make sense. If the start byte offset is non-zero, the end also must be. The header's end byte offset is ignored in the hope that a correct byte offset will be found in the text segment as the value of $ENDANALYSIS." );
            }
        }
        else if ( segmentEnd != 0 )
        {
            // SPECIAL HANDLING: If the segment begin is zero, then the segment
            // end cannot be anything but zero. Nevertheless, FCS files have
            // been seen that set the segment end to (-1) when the begin is
            // zero.
            //
            // Report the situation, then force the segment end to zero.
            this->appendFileLog(
                "warning",
                "The FCS file's header has a zero byte offset for the start of the analysis segment, but a non-zero byte offset for the end. This does not make sense. If the start byte offset is zero, the end also must be. The header's end byte offset is ignored in the hope that the correct byte offset will be found in the text segment as the value of $ENDANALYSIS." );


            this->setDictionaryStringInternal(
                "$ENDANALYSIS",
                "0",
                false,      // Do not convert or trim keyword.
                false );    // Do not trim value.
        }
    }

    /**
     * Loads the TEXT or supplemental TEXT segment of the file.
     *
     * The TEXT and supplemental TEXT segments are in a mix of ASCII and
     * UTF-8 text and composed of a series of keyword-value pairs.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] loadText
     *   When true, load the TEXT segment. When false, load the supplemental
     *   TEXT segment.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's content cannot be parsed or
     *   if any value is malformed.
     */
    void loadTextSegment(
        std::FILE*const fp,
        const bool loadText )
    {
        long segmentBegin = 0L;
        long segmentEnd   = 0L;
        std::string segmentName;

        char delimiter = this->fileDictionaryValueDelimiter;

        //
        // Initialize.
        // -----------
        // Get the byte offsets to the segment.
        if ( loadText == true )
        {
            // Loading the text segment. Get the previously-stored values
            // from the file's header.
            segmentBegin = this->getDictionaryLong( "$BEGINTEXT" );
            segmentEnd   = this->getDictionaryLong( "$ENDTEXT" );
            segmentName  = "Text";
        }
        else
        {
            // Loading the supplemental text segment. Get the previously-stored
            // values from the text segment, if any.
            //
            // While these keywords are required in FCS 3.0 and 3.1, they do
            // not exist in earlier versions of the specification. If not
            // present, skip reading the non-existant segment.
            if ( this->inDictionary( "$BEGINSTEXT" ) == false ||
                 this->inDictionary( "$ENDSTEXT" ) == false )
                return;

            segmentBegin = this->getDictionaryLong( "$BEGINSTEXT" );
            segmentEnd   = this->getDictionaryLong( "$ENDSTEXT" );
            segmentName  = "Supplemental text";

            // The supplemental text segment is optional. If these
            // values are zero, then skip them again.
            if ( segmentBegin == 0 )
                return;
        }


        //
        // Seek to start.
        // --------------
        // Use the byte offset of the segment from the header to seek
        // to it's start.
        if ( fseek( fp, segmentBegin, SEEK_SET ) != 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file is truncated before the start of the " ) +
                segmentName + std::string( " segment." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }


        //
        // Get delimiter.
        // --------------
        // The first byte of the TEXT segment is the delimiter that marks
        // the start and end of a value following a keyword. Documentation
        // uses a "/", but this could also be a double-quote, or anything.
        //
        // The same delimiter is used for ALL segments.
        //
        // SPECIAL HANDLING: Some FCS files encountered repeat the delimiter
        // as the first character of the SUPPLEMENTAL TEXT section, even though
        // the specification does not allow for that. To accomodate these,
        // we always read one byte from the start of the segment. For the
        // TEXT segment, this is the delimiter. For the SUPPLEMENTAL TEXT
        // segment, if the first byte matches the TEXT segment's delimiter,
        // then we assume an extra delimiter character has been added.
        // Otherwise, it hasn't and we back one byte.
        if ( std::fread( &delimiter, 1, 1, fp ) != 1 )
        {
            this->appendFileLog(
                "error",
                "The file is truncated before the delimiter is specified at the start of the text segment." );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }

        if ( loadText == true )
        {
            // For the TEXT segment, there is always a delimiter. Save it.
            this->fileDictionaryValueDelimiter = delimiter;

            /*
             * TODO. Delete this. Including the delimiter character in the
             * attributes is of minimal use (it is an internal value used during
             * parsing) AND it is often problematic. It is common for FCS files
             * to use unusual or control characters (such as a back-slash or
             * EOL), which become difficult to output well when tools print
             * out or convert attributes to JSON.
             *
            this->fileAttributes.emplace( "dictionaryValueDelimiter",
                std::string( 1, delimiter ) );
             */

            // Validate the delimiter.
            if ( delimiter == '\0' )
            {
                this->appendFileLog(
                    "error",
                    "The file uses an invalid NULL character keyword value delimiter." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            if ( delimiter == ',' )
            {
                this->appendFileLog(
                    "error",
                    "The file uses an invalid comma (\",\") character keyword value delimiter." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            if ( (delimiter & 0x80) != 0 )
            {
                this->appendFileLog(
                    "error",
                    "The file uses an invalid non-ASCII character keyword value delimiter." );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
        }
        else if ( delimiter != this->fileDictionaryValueDelimiter )
        {
            // For the SUPPLEMENTAL TEXT segment, a redundant delimiter has
            // NOT been included, which is the way the segment should be.
            // Since we read a byte, re-seek back to the start.
            fseek( fp, segmentBegin, SEEK_SET );
        }
        else
        {
            // For the SUPPLEMENTAL TEXT segment, a redundant delimiter was
            // included.
            this->appendFileLog(
                "warning",
                "The FCS file's supplemental text segment starts with a redundant keyword value delimiter. Such a delimiter is only needed at the start of the text segment, not the supplemental text segment. This software ignores the redundant delimiter and continues to use the delimiter set in the text segment." );
        }

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Keyword value delimiter:";
            if ( isgraph(delimiter) == true )
                std::cerr << "\"" << delimiter << "\"\n";
            else if ( delimiter == '\n' )
                std::cerr << "line feed (CTRL-J)\n";
            else if ( delimiter == '\r' )
                std::cerr << "carriage return (CTRL-M)\n";
            else if ( delimiter == '\t' )
                std::cerr << "tab (CTRL-I)\n";
            else if ( delimiter == 0x0c )
                std::cerr << "page feed (CTRL-L)\n";
            else
            {
                std::cerr << "0x" <<
                    std::setw(2) << std::setfill('0') << std::setbase(16) <<
                    (uint32_t) delimiter <<
                    " (non-printing character)\n" <<
                    std::setbase(10) << std::setfill(' ') <<
                    std::resetiosflags( std::ios::showbase );
            }
        }


        //
        // Read keywords-value pairs
        // -------------------------
        // The remainder of the TEXT segment contains keyword-value pairs
        // of the form "KEYWORD/VALUE/" where KEYWORD is the keyword name,
        // / is the delimiter above, and VALUE is a numeric or text value.
        //
        // KEYWORD and VALUE cannot be empty. Two delimiters next to each
        // other escape the delimiter to include it in a text value
        // (e.g. "STUFF/one//two/" creates a value "one/two").
        //
        // All KEYWORDs are ASCII, though we store them here as wide
        // strings for Unicode.
        //
        // All VALUEs are either UTF-8 strings or numeric values. Since
        // the file entry does not indicate if a value should be parsed
        // as a number, we always store values as wide strings for Unicode.
        // Values are parsed to integers or doubles later by code that knows
        // which keyword has what type of value.
        //
        // When reading keywords and values, it is awkward to repeatedly
        // switch back and forth from ASCII to UTF-8 parsing. Fortunately,
        // ASCII is a subset of UTF-8, so it is safe to treat all input
        // as UTF-8.
        //
        // To read the UTF-8 TEXT segments, we read the entire segment into
        // a byte buffer, then interpret those bytes as UTF-8. This yields
        // a wide character buffer that we then use for parsing to pull out
        // individual keywords and values.
        //
        // The TEXT segment's size is limited to the largest number that
        // can be represented in the 8 bytes of ASCII allowed in the header.
        // This maximum is 99999999 bytes = 100 Mbytes. This is small
        // enough that we can read the bytes into a buffer all at once.
        //
        // Conversion to wide characters quadruples storage to 400 Mbytes max.
        size_t textSize = segmentEnd - segmentBegin;

        // Allocate enough space for all of the text segment PLUS an extra
        // character for special handling below PLUS a NULL terminator.
        char* byteBuffer = new char[textSize + 2];

        // Read bytes.
        if ( std::fread( byteBuffer, 1, textSize, fp ) != textSize )
        {
            this->appendFileLog(
                "error",
                std::string( "The file is truncated before the end of the " ) +
                segmentName + std::string( " segment." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }
        byteBuffer[textSize] = '\0';

        // SPECIAL HANDLING: FCS files have been encountered that set the
        // segment END one byte early. This will cause the last keyword
        // value to be missing its final delimiter.
        //
        // To detect this case, read one more byte and see if it is a
        // delimiter.
        char oneMoreByte;
        if ( std::fread( &oneMoreByte, 1, 1, fp ) != 1 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file is truncated before the end of the " ) +
                segmentName + std::string( " segment." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }

        if ( oneMoreByte == delimiter )
        {
            // Yup. It's a delimiter. Add it to the buffer.
            this->appendFileLog(
                "warning",
                std::string( "The FCS file's byte offset for the end of the " ) +
                segmentName + std::string( " segment is too small by one byte. The byte offset should indicate the location of the last byte of the segment, which should be a keyword value closing delimiter. An additional byte is automatically read and appended to the segment." ) );

            byteBuffer[textSize] = oneMoreByte;
            ++textSize;
            byteBuffer[textSize] = '\0';
        }

        // SPECIAL HANDLING: FCS files have been encountered that include
        // trailing blanks at the end of the segment and after the last
        // keyword value delimiter. This does not conform to the FCS
        // specification.
        //
        // If present, silently trim blanks off the end.
        if ( delimiter != ' ' && byteBuffer[textSize - 1] == ' ' )
        {
            this->appendFileLog(
                "warning",
                std::string( "The FCS file's " ) + segmentName +
                std::string( " segment ends with white space. This wastes space in the file since it contains neither dictionary entries or event data. The extra white space is ignored." ) );

            while ( byteBuffer[textSize - 1] == ' ' )
                --textSize;
            byteBuffer[textSize] = '\0';
        }

        // SPECIAL HANDLING: FCS files have been encountered that include
        // an extra delimiter at the end of the segment after the last
        // keyword value delimiter. This does not conform to the FCS
        // specification.
        //
        // If present, silently trim extra delimiters off the end.
        if ( byteBuffer[textSize - 1] == delimiter )
        {
            const size_t textSizeBeforeBackup = textSize;
            while ( byteBuffer[textSize - 1] == delimiter )
                --textSize;

            // Keep one of the delimiters since it ends the last value.
            ++textSize;
            byteBuffer[textSize] = '\0';

            if ( textSize != textSizeBeforeBackup )
            {
                this->appendFileLog(
                    "warning",
                    std::string( "The FCS file's " ) + segmentName +
                    std::string( " segment ends with redundant keyword value delimiters. The extra delimiters are ignored." ) );
            }
        }

        //
        // Convert to wide characters.
        // ---------------------------
        // The FCS specifications have always required that keywords be
        // ASCII. Numeric values are always inherently ASCII. The handling
        // of string values, however, varies.
        //
        // For FCS 1.0 and 2.0, string values were always ASCII.
        //
        // FCS 3.0 introduced a "$UNICODE" keyword that could be used to
        // indicate which specific keywords had values that were in the
        // UTF-8 encoding of Unicode. UTF-8 has ASCII as a subset. However,
        // this keyword was so awkward that it wasn't used.
        //
        // FCS 3.1 dropped the "$UNICODE" keyword and declared that *ALL*
        // keyword values were in UTF-8.
        //
        // UTF-8 requires a variable number of bytes per character, so we
        // cannot parse the text by simple array indexing unless we first
        // widden all characters to fixed-width wide characters, each of
        // which is supposed to be large enough to hold any Unicode character
        // (i.e. 32-bits).
        //
        // SPECIAL HANDLING: While string values are supposed to be either
        // ASCII (pre 3.1) or UTF-8 (3.1), FCS files have been encountered
        // that include non-ASCII characters. Such characters have the high
        // bit set in an 8-bit byte, which is illegal ASCII. Apple and
        // Microsoft both used this "extended ASCII" to add another 128
        // characters to the character set so that they could support accents
        // and special symbols, like TM and (C). One FCS file encountered
        // had the software product name in a keyword field and included the
        // Mac extended ASCII character for TM.
        //
        // Unfortunately, there is no standard for extended ASCII. The meaning
        // of these bytes varies between Apple and Microsoft systems. Without
        // knowing which system the FCS file was written on, it is not possible
        // to map that system's extended ASCII to UTF-8. Furthermore, because
        // these are illegal ASCII characters, they cannot be converted to
        // wide characters. Doing so throws an exception.
        //
        // Illegal ASCII characters therefore require special handling.
        std::wstring_convert<const std::codecvt_utf8<wchar_t>, wchar_t> unicode;
        std::wstring wBuffer;
        try
        {
            // Convert byte buffer to a wide string.
            wBuffer = unicode.from_bytes( byteBuffer );
        }
        catch ( ... )
        {
            // Wide string conversion failed. This is likely because the
            // byte buffer includes illegal "extended ASCII" characters.
            // Clean those out. Replace each one with "?".
            bool foundIllegal = false;
            for ( size_t i = 0; i < textSize; ++i )
            {
                if ( (byteBuffer[i] & 0x80) != 0 )
                {
                    byteBuffer[i] = '?';
                    foundIllegal = true;
                }
            }

            if ( foundIllegal == true )
            {
                this->appendFileLog(
                    "warning",
                    std::string( "The FCS file's " ) + segmentName +
                    std::string( " segment contains text that is not valid ASCII. It uses a platform-specific \"extended ASCII\" encoding for special non-standard symbols. Without knowledge of the specific platform used to create the file, this text cannot be converted to a modern Unicode encoding. Invalid characters have therefore been replaced with '?' wherever they occur." ) );

                try
                {
                    // Try to convert to a wide string again.
                    wBuffer = unicode.from_bytes( byteBuffer );
                }
                catch ( ... )
                {
                    // Conversion still failed. There is something more deeply
                    // wrong with the text. Fall through to error handling
                    // below.
                    foundIllegal = false;
                }
            }

            if ( foundIllegal == false )
            {
                // There is something more deeply wrong with the text.
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + segmentName +
                    std::string( " segment contains text that is not valid ASCII or Unicode and cannot be parsed." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    std::string( "Text in the file is not stored properly." ) +
                    ERROR_CANNOTLOAD );
            }
        }

        const size_t wBufferLength = wBuffer.length( );

        delete[] byteBuffer;
        byteBuffer = nullptr;

        wchar_t wdelimiter = delimiter;

        //
        // Parse keyword-value pairs
        // -------------------------
        // Tokenize the wide string buffer, dividing tokens at delimiters.
        // Tokens will alternate between keywords and values.
        std::wstring keyword;
        std::wstring value;

        size_t startOfToken = 0;
        size_t endOfToken   = 0;

        bool verboseReportedSkipWhiteSpace = false;
        bool verboseReportedEmptyValueDoubleDelimiter = false;

        const int END_OF_STRING           = 0;
        const int READY_FOR_KEYWORD       = 1;
        const int START_OF_KEYWORD        = 2;
        const int DELIMITER_AFTER_KEYWORD = 3;
        const int START_OF_VALUE          = 4;
        const int MIDDLE_OF_VALUE         = 5;
        const int DELIMITER_AFTER_VALUE   = 6;
        const int SAVE_KEYWORD_VALUE      = 7;

        int state = READY_FOR_KEYWORD;
        while ( state != END_OF_STRING )
        {
            switch ( state )
            {
            case READY_FOR_KEYWORD:
                // Token indexes are at the start of where a keyword
                // could be.
                //
                // SPECIAL HANDLING: The FCS specification does not rule out
                // white space at the start or end of a keyword. However,
                // FCS files have been encountered that include white space
                // at the end of the text segment after the last value (which
                // this parser would see as at the start of the next keyword).
                //
                // For generality, and to keep keywords clean, we skip white
                // space before a keyword. This will also catch white space at
                // the end of the text segment after the last value.
                if ( wdelimiter != ' ' )
                {
                    // Since FCS dictionary keywords must be ASCII, just
                    // watch for ASCII spaces instead of all Unicode
                    // blank characters.
                    size_t initialStartOfToken = startOfToken;
                    while ( startOfToken < wBufferLength &&
                            wBuffer[startOfToken] == ' ' )
                        ++startOfToken;

                    if ( startOfToken == wBufferLength )
                    {
                        // End of string after white space.
                        state = END_OF_STRING;
                        continue;
                    }

                    if ( this->verbose == true &&
                         startOfToken != initialStartOfToken &&
                         verboseReportedSkipWhiteSpace == false )
                    {
                        this->appendFileLog(
                            "warning",
                            std::string( "The FCS file's " ) + segmentName +
                            std::string( " segment starts with white space before the first keyword. This is not valid and it wastes file space. This leading white space has been skipped." ) );
                        verboseReportedSkipWhiteSpace = true;
                    }

                    endOfToken = startOfToken;
                }
                state = START_OF_KEYWORD;
                continue;

            case START_OF_KEYWORD:
                // Token indexes are at the start of where a keyword is.
                //
                // Search forward to the value delimiter and watch for
                // end-of-string.
                while ( endOfToken < wBufferLength &&
                        wBuffer[endOfToken] != wdelimiter )
                    ++endOfToken;

                if ( endOfToken == wBufferLength )
                {
                    // Unexpected end of string at the start of a keyword.
                    if ( keyword.empty( ) == false )
                    {
                        // Add the keyword and give it an empty value.
                        keyword = wBuffer.substr(
                            startOfToken,
                            (endOfToken - startOfToken) );
                        state = SAVE_KEYWORD_VALUE;

                        this->appendFileLog(
                            "warning",
                            std::string( "The FCS file's end of the " ) +
                            segmentName + std::string( " segment was encountered unexpectedly while starting a new dictionary keyword. The segment's byte offsets are incorrect. The segment has therefore been ended prematurely and the last keyword assigned an empty value." ) );
                    }
                    else
                    {
                        state = END_OF_STRING;
                        this->appendFileLog(
                            "warning",
                            std::string( "The FCS file's end of the " ) +
                            segmentName + std::string( " segment was encountered unexpectedly while starting a new dictionary keyword. The segment's byte offsets are incorrect. The segment has therefore been ended prematurely." ) );
                    }
                }
                else
                {
                    // Save the keyword.
                    keyword = wBuffer.substr(
                        startOfToken,
                        (endOfToken - startOfToken) );

                    startOfToken = endOfToken;
                    state = DELIMITER_AFTER_KEYWORD;
                }
                continue;

            case DELIMITER_AFTER_KEYWORD:
                // Token indexes are at a delimiter after a keyword.
                //
                // SPECIAL HANDLING: The FCS specification requires that:
                // - Values are delimited before and after.
                // - Empty values have at least one blank.
                // - Side-by-side delimiters mid-value escape the delimiter
                //   to include one delimiter in the value.
                //
                // Examples:
                // - "$ABC/one/" assigns keyword $ABC the value "one".
                // - "$ABC/ /" assigns keyword $ABC the value " " (empty).
                // - "$ABC/one//two/" assigns keyword $ABC the value "one/two".
                //
                // Unfortunately, FCS files have been encountered that store
                // empty values as two delimiters side-by-side immediately
                // after the keyword:
                // - "$ABC//" assigns keyword $ABC the value "" (empty).
                //
                // This is technically incorrect. It confuses parsing by
                // giving side-by-side delimiters two meanings. Nevertheless,
                // this is done in real FCS files and we need to parse it.
                ++endOfToken;
                ++startOfToken;

                if ( startOfToken == wBufferLength )
                {
                    // Unexpected end of string immediately after a value
                    // delimiter after a keyword.
                    //
                    // SPECIAL HANDLING: While the keyword's value certainly
                    // should have a closing delimiter before the end of the
                    // segment, FCS files have been encountered that don't.
                    // This appears to only occur when the value is empty
                    // (as in "$ABC//" without the last "/"). Allow this
                    // special case.
                    state = SAVE_KEYWORD_VALUE;
                    this->appendFileLog(
                        "warning",
                        std::string( "The FCS file's " ) + segmentName +
                        std::string( " segment ends unexpectedly immediately after a keyword and its value delimiter, but before the keyword's value. The last keyword has therefore been assigned an empty value." ) );
                    continue;
                }

                if ( wBuffer[endOfToken] == wdelimiter )
                {
                    // Next character is a delimiter.
                    //
                    // SPECIAL HANDLING: As noted above, a keyword with
                    // an empty value should be "$ABC/ /", but some vendors
                    // write out "$ABC//". And that's what we've encountered.
                    // Allow it.
                    ++startOfToken;
                    ++endOfToken;
                    state = SAVE_KEYWORD_VALUE;

                    if ( verboseReportedEmptyValueDoubleDelimiter == false )
                    {
                        this->appendFileLog(
                            "warning",
                            "One or more FCS file dictionary keywords have been found where an empty value is indicated by a pair of side-by-side value delimiters. This is invalid since the FCS file specification requires that a pair of delimiters cause inclusion of the delimiter itself in the value, rather than mark an empty value. This special case is nevertheless recognized and interpreted as meaning the keyword has an empty value." );
                        verboseReportedEmptyValueDoubleDelimiter = true;
                    }
                    continue;
                }

                // The current character is not a delimiter. Start a value.
                startOfToken = endOfToken;
                state = START_OF_VALUE;
                continue;

            case START_OF_VALUE:
                // Token indexes are at the start of a value for the current
                // keyword.
                //
                // Search forward to the value delimiter and watch for
                // end-of-string.
                while ( endOfToken < wBufferLength &&
                        wBuffer[endOfToken] != wdelimiter )
                    ++endOfToken;

                // Extract the value.
                value = wBuffer.substr(
                    startOfToken,
                    (endOfToken - startOfToken) );
                startOfToken = endOfToken;
                if ( endOfToken == wBufferLength )
                {
                    // Unexpected end of string before the ending value
                    // delimiter.
                    state = SAVE_KEYWORD_VALUE;
                    this->appendFileLog(
                        "warning",
                        std::string( "The FCS file's " ) + segmentName +
                        std::string( " segment ends unexpectedly within the value for a dictionary keyword, before a closing value delimiter. The keyword value has been closed automatically and saved." ) );
                }
                else
                    state = DELIMITER_AFTER_VALUE;

                continue;

            case MIDDLE_OF_VALUE:
                // Token indexes are after a double-delimiter within a value.
                //
                // Search forward to the value delimiter and watch for
                // end-of-string.
                while ( endOfToken < wBufferLength &&
                        wBuffer[endOfToken] != wdelimiter )
                    ++endOfToken;

                // Extract the value and append to the current keyword value.
                value += wBuffer.substr(
                    startOfToken,
                    (endOfToken - startOfToken) );
                startOfToken = endOfToken;

                if ( endOfToken == wBufferLength )
                {
                    // Unexpected end of string before the ending value
                    // delimiter.
                    state = SAVE_KEYWORD_VALUE;
                    this->appendFileLog(
                        "warning",
                        std::string( "The FCS file's " ) + segmentName +
                        std::string( " segment ends unexpectedly within the value for a dictionary keyword, before a closing value delimiter. The keyword value has been closed automatically and saved." ) );
                }
                else
                    state = DELIMITER_AFTER_VALUE;
                continue;

            case DELIMITER_AFTER_VALUE:
                // Token indexes are at a delimiter after a value OR a
                // delimiter within a value.
                //
                // Skip past the delimiter and look at the next character.
                // If it is a delimiter too, then we have a double-delimiter
                // in the middle of a value and the delimiter itself should
                // be added to the value. Otherwise we're at the end of
                // the value and the start of a new keyword.
                ++endOfToken;
                ++startOfToken;

                if ( endOfToken >= wBufferLength )
                {
                    // Nothing after the delimiter.
                    state = SAVE_KEYWORD_VALUE;
                    continue;
                }

                if ( wBuffer[endOfToken] == wdelimiter )
                {
                    // Next character is a delimiter, so we have a double-
                    // delimiter. Include the delimiter in the value and
                    // keep parsing the same value.
                    value += wdelimiter;
                    ++endOfToken;
                    ++startOfToken;
                    state = MIDDLE_OF_VALUE;
                    continue;
                }

                // Next character after the end-of-value delimiter is not
                // another delimiter, so the keyword and value are done.
                state = SAVE_KEYWORD_VALUE;
                continue;

            case SAVE_KEYWORD_VALUE:
                // Token indexes are immediately after a closing value
                // delimiter or at end-of-string.
                //
                // Convert to UTF-8 strings.
                std::string k = unicode.to_bytes( trim( keyword ) );
                std::string v = unicode.to_bytes( trim( value ) );

                // Convert keyword to upper case.
                std::transform(
                    k.begin( ),
                    k.end( ),
                    k.begin( ),
                    [](unsigned char c){ return std::toupper(c); } );

                // Clear the current keyword and value.
                keyword.clear( );
                value.clear( );

                // Add the new keyword-value pair.
                //
                // SPECIAL HANDLING: From notes at FlowRepository.org, some
                // manufacturers incorrectly set the text segment
                // $BEGIN*/$END* keywords to zeroes. In this case, the
                // header's values should be used (which hopefully are not
                // zeroes).
                //
                // Header loading has already added $BEGIN*/$END* values
                // from the header into the dictionary. So if the new values
                // are zeroes, leave the existing dictionary entries alone.
                if ( k == "$BEGINDATA"     ||
                     k == "$ENDDATA"       ||
                     k == "$BEGINANALYSIS" ||
                     k == "$ENDANALYSIS" )
                {
                    const long vl = std::strtol( v.c_str( ), nullptr, 10 );
                    if ( vl != 0 )
                    {
                        // The value is non-zero. But was a non-zero value
                        // already set in the header and is it the same?
                        const long vheader = this->getDictionaryLong( k );
                        if ( vheader != 0 && vheader != vl )
                            this->appendFileLog(
                                "warning",
                                std::string( "The FCS file's dictionary sets the byte offset for the " ) + k +
                                std::string( " keyword to a value that differs from the equivalent value set in the header, and neither one is zero. The byte offers are therefore in conflict. This software uses the byte offset in the dictionary and ignores the value in the header." ) );
                        else
                            this->setDictionaryStringInternal(
                                k,
                                v,
                                false,      // No need to convert to upper case.
                                false );    // No need to trim value.
                    }
                    else if ( this->getDictionaryLong( k ) != 0 )
                    {
                        this->appendFileLog(
                            "warning",
                            std::string( "The FCS file's dictionary sets the byte offset for the " ) + k +
                            std::string( " keyword to zero, rather than repeating the non-zero value in the file header for the same segment. The new zero value is ignored and the header's value is used instead." ) );
                    }
                }
                else
                {
                    this->setDictionaryStringInternal(
                        k,
                        v,
                        false,      // No need to convert to upper case.
                        false );    // No need to trim value.
                }

                if ( endOfToken >= wBufferLength )
                {
                    // End of string.
                    state = END_OF_STRING;
                    continue;
                }

                state = READY_FOR_KEYWORD;
                continue;
            }
        }
    }

    /**
     * Validates information loaded from the TEXT segment.
     *
     * The byte offsets in the FCS header are limited to 8 ASCII bytes,
     * which hit a maximum of 99999999 bytes, or 10 Mbytes. This is far too
     * little for long event lists or files with big dictionaries. To
     * overcome this limitation, the 3.0 and 3.1 FCS specifications allow
     * the header to contain zeroes for the offsets for the DATA and
     * ANALYSIS segments. The actual size is then indicated with the
     * $BEGINDATA, $ENDDATA, $BEGINANALYSIS, and $ENDANALYSIS keyword values
     * in the TEXT segment's dictionary. These are validated here.
     *
     * The same header limitations also constrain the TEXT segment to be
     * at most 10 Mbytes, which is too little if a lot of additional
     * information is in the dictionary. To overcome this limitation, the
     * 3.0 and 3.1 FCS specifications allow a second SUPPLEMENTAL TEXT
     * section with more dictionary entries. The byte offsets for this
     * additional section must be in the original smaller TEXT segment.
     * These values are validated here.
     *
     * Earlier when the file's header was read, the beginning and ending
     * byte offsets for the DATA, TEXT, and ANALYSIS segments were added
     * to the dictionary using the FCS 3.0/3.1 keywords, even for FCS
     * files using an earlier version number. So these values are now
     * guaranteed to be in the dictionary.
     *
     * Later, when the TEXT segment was read, any better values will have
     * overwritten these initial values from the header. Here we validate
     * these updated values.
     *
     * The following keywords and values are validated:
     * @li $BEGINANALYSIS (required)
     * @li $BEGINDATA (required)
     * @li $BEGINSTEXT (required)
     * @li $ENDANALYSIS (required)
     * @li $ENDDATA (required)
     * @li $ENDSTEXT (required)
     *
     * @throws std::runtime_error
     *   Throws an exception if any required keyword is missing or if
     *   the values are malformed.
     */
    void loadValidateTextSegment( )
    {
        long segmentBegin = 0L;
        long segmentEnd   = 0L;

        //
        // Data begin/end.
        // ---------------
        // Header parsing initialized the dictionary with $BEGINDATA
        // and $ENDDATA values parsed from the header. These dictionary
        // entries are therefore guaranteed to exist.
        //
        // Validate that the begin/end byte offsets are reasonable.
        segmentBegin = this->getDictionaryLong( "$BEGINDATA" );
        segmentEnd   = this->getDictionaryLong( "$ENDDATA" );

        if ( segmentBegin <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The $BEGINDATA dictionary keyword used to locate the start of the data segment containing the file's events has an invalid non-positive value: \"" ) +
                this->getDictionaryString( "$BEGINDATA" ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( segmentEnd <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The $ENDDATA dictionary keyword used to locate the end of the data segment containing the file's events has an invalid non-positive value: \"" ) +
                this->getDictionaryString( "$BEGINEND" ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( segmentEnd < segmentBegin )
        {
            this->appendFileLog(
                "error",
                std::string( "The $ENDDATA dictionary keyword used to locate the end of the data segment has an invalid value earlier than the $BEGINDATA value for the start of the same data segment: \"" ) +
                this->getDictionaryString( "$BEGINEND" ) + std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( segmentEnd == segmentBegin )
        {
            this->appendFileLog(
                "error",
                std::string( "The $BEGINDATA and $ENDDATA dictionary keywords used to locate the start and end of the data segment have the same value, indicating an empty data segment. However, the data segment is a required part of the file that contains event data and it cannot be empty." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->fileSize != 0 )
        {
            // Segment begin cannot be beyond the end of the file.
            if ( segmentBegin >= this->fileSize )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file is truncated before the start of event data." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }

            // Segment end cannot be beyond the end of the file by more
            // than one byte.
            if ( segmentEnd >= (this->fileSize + 1) )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The file is truncated before the end of event data." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }
        }


        //
        // Analysis begin/end.
        // -------------------
        // Header parsing initialized the dictionary with $BEGINANALYSIS
        // and $ENDANALYSIS values parsed from the header. These dictionary
        // entries are therefore guaranteed to exist.
        //
        // Validate that the begin/end byte offsets are reasonable. The
        // segment can be empty.
        segmentBegin = this->getDictionaryLong( "$BEGINANALYSIS" );
        segmentEnd   = this->getDictionaryLong( "$ENDANALYSIS" );

        if ( segmentBegin != 0 )
        {
            if ( segmentBegin < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $BEGINANALYSIS dictionary keyword used to locate the start of the analysis segment has an invalid negative value: \"" ) +
                    this->getDictionaryString( "$BEGINANALYSIS" ) + std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            if ( segmentEnd < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $ENDANALYSIS dictionary keyword used to locate the end of the analysis segment has an invalid negative value: \"" ) +
                    this->getDictionaryString( "$ENDANALYSIS" ) + std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            if ( segmentEnd < segmentBegin )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $ENDANALYSIS dictionary keyword value used to locate the end of the analysis segment has an invalid value earlier than the value for the $BEGINANALYSIS keyword used to locate the start of the same analysis segment: \"" ) +
                    this->getDictionaryString( "$ENDANALYSIS" ) + std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            if ( segmentEnd == segmentBegin )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $BEGINANALYSIS and $ENDANALYSIS dictionary keyword values used to locate the start and end of the analysis segment have the same value, indicating an empty analysis segment. However, an empty segment should have been indicated with zero values instead." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            if ( this->fileSize != 0 )
            {
                // Segment begin cannot be beyond the end of the file.
                if ( segmentBegin >= this->fileSize )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file is truncated before the start of analysis data." ) );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        "The file is missing critical information." +
                        ERROR_CANNOTLOAD );
                }

                // Segment end cannot be beyond the end of the file by more
                // than one byte.
                if ( segmentEnd >= (this->fileSize + 1) )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The file appears to be truncated before the end of analysis data." ) );
                    throw std::runtime_error(
                        ERROR_TRUNCATED +
                        "The file is missing critical information." +
                        ERROR_CANNOTLOAD );
                }
            }
        }


        //
        // Supplemental text begin/end.
        // ----------------------------
        // The original FCS header does not support byte offsets to the
        // SUPPLEMENTAL TEXT segment. If the segment exists, it must be
        // indicated with new dictionary values set in the TEXT segment.
        //
        // The begin/end dictionary keywords for the SUPPLEMENTAL TEXT
        // segment are required in the FCS 3.0 and 3.1 specifications, but
        // the keywords aren't defined in the earlier FCS 1.0 and 2.0.
        //
        // Here we consider the keywords as optional. But if they exist,
        // they muyst be set properly. They can be set to zeroes.
        if ( this->inDictionary( "$BEGINSTEXT" ) == true )
        {
            // If there is a begin, there must be an end.
            if ( this->inDictionary( "$ENDSTEXT" ) == false )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $ENDSTEXT dictionary keyword is missing from the text segment and is required to locate the end of the supplemental text segment in the file. The data in the file cannot be loaded.") );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            segmentBegin = this->getDictionaryLong( "$BEGINSTEXT" );
            segmentEnd   = this->getDictionaryLong( "$ENDSTEXT" );

            // SPECIAL HANDLING: FCS files have been encountered that
            // incorrectly set the SUPPLEMENTAL TEXT segment begin/end
            // to be the same as the TEXT segment. This causes software
            // to think there are two segments, when really there is
            // just one.
            //
            // Here we check for this case and, if found, the SUPPLEMENTAL TEXT
            // segment's begin/end are properly set to zero.
            if ( segmentBegin == this->getDictionaryLong( "$BEGINTEXT" ) )
            {
                // Same begin as the TEXT segment. Zero out the
                // SUPPLEMENTAL TEXT segment.
                this->setDictionaryStringInternal(
                    "$BEGINSTEXT",
                    "0",
                    false,      // No need to convert keyword or trim.
                    false );    // No need to trim value.
                this->setDictionaryStringInternal(
                    "$ENDSTEXT",
                    "0",
                    false,      // No need to convert keyword or trim.
                    false );    // No need to trim value.
                segmentBegin = segmentEnd = 0L;

                this->appendFileLog(
                    "warning",
                    "The FCS file's supplemental text segment has the same byte offsets as the text segment. The redundant supplemental text segment offsets have therefore been ignored." );
            }

            if ( segmentBegin != 0 )
            {
                if ( segmentBegin < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The $BEGINSTEXT dictionary keyword used to locate the start of the supplemental text segment has an invalid negative value: \"" ) +
                        this->getDictionaryString( "$BEGINSTEXT" ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The $ENDSTEXT dictionary keyword used to locate the end of the supplemental text segment has an invalid negative value: \"" ) +
                        this->getDictionaryString( "$ENDSTEXT" ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd < segmentBegin )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The $ENDSTEXT dictionary keyword value used to locate the end of the supplemental text segment has an invalid value earlier than the value for the $BEGINSTEXT keyword used to locate the start of the same supplemental text segment: \"" ) +
                        this->getDictionaryString( "$ENDSTEXT" ) + std::string( "\"." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( segmentEnd == segmentBegin )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The $BEGINSTEXT and $ENDSTEXT dictionary keyword values used to locate the start and end of the supplemental text segment have the same value, indicating an empty supplemental text segment. However, an empty segment should have been indicated with zero values instead." ) );
                    throw std::runtime_error(
                        ERROR_MALFORMED +
                        ERROR_BADCONTENTS +
                        ERROR_CANNOTLOAD );
                }

                if ( this->fileSize != 0 )
                {
                    // Segment begin cannot be beyond the end of the file.
                    if ( segmentBegin >= this->fileSize )
                    {
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the start of supplemental dictionary data." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }

                    // Segment end cannot be beyond the end of the file by more
                    // than one byte.
                    if ( segmentEnd >= (this->fileSize + 1) )
                    {
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the end of supplemental dictionary data." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }
                }
            }
        }
    }

    /**
     * Validates necessary dictionary entries for data.
     *
     * <B>Validated keywords</B><BR>
     * This method validates the existance and values of the most important
     * dictionary entries needed to continue parsing and using the file's
     * data. Validation checks that required keywords are present and that
     * values are reasonable.
     *
     * The following keywords and values are validated:
     * @li $BYTEORD (required but treated as optional)
     * @li $DATATYPE (required)
     * @li $MODE (required but treated as optional)
     * @li $NEXTDATA (required but treated as optional)
     * @li $PAR (required)
     * @li $TOT (required but treated as optional)
     *
     * The following keywords and values are validated for each parameter:
     * @li $PnB (required)
     * @li $PnCALIBRATION
     * @li $PnE (required)
     * @li $PnN (required)
     * @li $PnR (required)
     *
     * <B>Unvalidated keywords</B><BR>
     * This method does not validate keywords that are optional, that have
     * unformatted string data, that have simple numeric values, or that have
     * multi-value values that are not directly used by this software.
     *
     * The following optional parameter value keywords are NOT validated:
     * @li $PnD
     * @li $PnF
     * @li $PnG
     * @li $PnL
     * @li $PnO
     * @li $PnP
     * @li $PnS
     * @li $PnT
     * @li $PnV
     *
     * The following optional date/time value keywords are NOT validated:
     * @li $BTIM
     * @li $DATE
     * @li $ETIM
     * @li $LAST_MODIFIED
     *
     * The following optional string value keywords are NOT validated:
     * @li $COM
     * @li $CYT
     * @li $CYTSN
     * @li $EXP
     * @li $FIL
     * @li $INST
     * @li $LAST_MODIFIER
     * @li $OP
     * @li $ORIGINALITY
     * @li $PLATEID
     * @li $PLATENAME
     * @li $PROJ
     * @li $SMNO
     * @li $SRC
     * @li $SYS
     * @li $WELLID
     *
     * The following optional multi-value keywords are NOT validated:
     * @li $TR
     *
     * The following optional numeric value keywords are NOT validated:
     * @li $ABRT
     * @li $CELLS
     * @li $CSMODE
     * @li $CSVBITS
     * @li $CSVnFLAG
     * @li $LOST
     * @li $VOL
     *
     * <B>Unsupported gating keywords</B><BR>
     * This software does not provide any special support for manual gating
     * values stored in FCS files. They may be stored in the dictionary,
     * but they are not validated.
     *
     * The following optional gating keywords are NOT validated:
     * @li $GATE
     * @li $GATEING
     * @li $GnE (deprecated)
     * @li $GnF (deprecated)
     * @li $GnN (deprecated)
     * @li $GnP (deprecated)
     * @li $GnR (deprecated)
     * @li $GnS (deprecated)
     * @li $GnT (deprecated)
     * @li $GnV (deprecated)
     * @li $RnI
     * @li $RnW
     *
     * <B>Unsupported deprecated keywords</B><BR>
     * The following deprecated keywords are NOT validated:
     * @li $ASC (deprecated)
     * @li $COMP (deprecated)
     * @li $PKn (deprecated)
     * @li $PKNn (deprecated)
     * @li $UNICODE (deprecated)
     *
     * @throws std::runtime_error
     *   Throws an exception if any required keyword is missing or if
     *   the values are malformed.
     */
    void loadValidateReadyToLoadData( )
    {
        std::string nv;

        //
        // Next data.
        // ----------
        // The $NEXTDATA keyword was introduced in FCS 2.0 to allow
        // multiple data sets to be stored in the same file. In FCS 3.1
        // the keyword was deprecated because the practice was awkward
        // and unnecessary.
        //
        // While the $NEXTDATA keyword is required in FCS 2.0, 3.0, and 3.1,
        // we treat it as optional because it did not exist in FCS 1.0.
        // If the keyword is present, it must have a zero value to indicate
        // there is no next data set.
        if ( this->inDictionary( "$NEXTDATA" ) == true )
        {
            if ( this->getDictionaryLong( "$NEXTDATA" ) != 0 )
            {
                this->appendFileLog(
                    "warning",
                    "The FCS file contains multiple data sets, but this feature has been deprecated and is not supported by this software. Only the first one is loaded. The additional data sets are ignored." );
            }
        }


        //
        // Mode.
        // -----
        // The $MODE keyword has been required since FCS 1.0. It indicates
        // whether the file's data are events ("list mode") or values for one
        // of two types of histogram ("correlated multivariate" or
        // "uncorrelated univariate"). The FCS 3.1 specification deprecated
        // the two histogram types, leaving only the list mode.
        //
        // While the keyword required, we will treat it as optional and
        // default to the only non-deprecated value allowed: list mode.
        // This is also the only format supported by this software.
        if ( this->inDictionary( "$MODE" ) == true )
        {
            nv = this->getDictionaryString( "$MODE" );
            if ( nv == "C" )
            {
                this->appendFileLog(
                    "error",
                    "The file contains a correlated multi-variate histogram, but this feature has been deprecated and is not supported by this software." );
                throw std::runtime_error(
                    ERROR_UNSUPPORTED +
                    std::string( "The file does not contain event data. Instead it contains a correlated multi-variate histogram. This feature has been deprecated and is not supported by this software." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( nv == "U" )
            {
                this->appendFileLog(
                    "error",
                    "The file contains an uncorrelated univariate histogram, but this feature has been deprecated and is not supported by this software." );
                throw std::runtime_error(
                    ERROR_UNSUPPORTED +
                    std::string( "The file does not contain event data. Instead it contains an uncorrelated univariate histogram, but this feature has been deprecated and is not supported by this software." ) +
                    ERROR_CANNOTLOAD );
            }
            if ( nv != "L" )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $MODE dictionary keyword used to indicate the type of data in the file has an unrecognized value of: \"" ) + nv +
                    std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
        }


        //
        // Data type.
        // ----------
        // The $DATATYPE keyword has been required since FCS 1.0. It provides
        // a broad indication of the data type for the DATA segment:
        // - "I" for integers.
        // - "F" for single-precision floats.
        // - "D" for double-precision floats.
        // - "A" for ASCII, which is deprecated.
        //
        // All other values are invalid.
        //
        // The "A" value for ASCII text is poorly specified and makes data
        // difficult to parse. The use of ASCII also makes the data huge.
        // The FCS 3.1 specification deprecates this way of storing data.
        //
        // This software does not support the "A" data type.
        if ( this->inDictionary( "$DATATYPE" ) == false )
        {
            this->appendFileLog(
                "error",
                std::string( "The $DATATYPE dictionary keyword is missing and is required to indicate the data type for stored data.") );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        nv = this->getDictionaryString( "$DATATYPE" );
        if ( nv == "I" )
        {
            // The exact integer width will be determined later based upon
            // parameter width values. For now, indicate 32-bit unsigned
            // integers.
            this->fileDataType = 'I';
            this->fileAttributes.emplace( "dataType", "int" );
            this->fileAttributes.emplace( "numberFormat", "binary" );
            this->fileMaxParameterBytes = 0L;
            this->fileMinParameterBytes = 8L;
            this->fileMaxParameterRange = 0L;

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Number format:" << "binary\n";
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "integers\n";
            }
        }
        else if ( nv == "F" )
        {
            this->fileDataType = 'F';
            this->fileAttributes.emplace( "dataType", "float" );
            this->fileAttributes.emplace( "numberFormat", "binary" );
            this->fileMaxParameterBytes = 4L;
            this->fileMinParameterBytes = 4L;
            this->fileMaxParameterRange = 0L;

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Number format:" << "binary\n";
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "floats\n";
            }
        }
        else if ( nv == "D" )
        {
            this->fileDataType = 'D';
            this->fileAttributes.emplace( "dataType", "double" );
            this->fileAttributes.emplace( "numberFormat", "binary" );
            this->fileMaxParameterBytes = 8L;
            this->fileMinParameterBytes = 8L;
            this->fileMaxParameterRange = 0L;

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Number format:" << "binary\n";
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Data type" << "doubles\n";
            }
        }
        else if ( nv == "A" )
        {
            this->appendFileLog(
                "error",
                "The $DATATYPE dictionary keyword value is 'A' to indicate that event data is stored as ASCII text. This feature has been deprecated in the modern FCS specification and it is not supported by this software." );
            throw std::runtime_error(
                ERROR_UNSUPPORTED +
                std::string( "The file uses an old style that stores event data as text. This feature has been deprecated in the modern FCS specification and it is not supported by this software." ) +
                ERROR_CANNOTLOAD );
        }
        else
        {
            this->appendFileLog(
                "error",
                std::string( "The $DATATYPE dictionary keyword used to indicate the data type for event data has an invalid value of \"" ) + nv +
                std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        //
        // Byte order.
        // -----------
        // The $BYTEORD keyword was introduced in FCS 2.0 and the value
        // is a comma-separated list of byte indexes that indicate the
        // meaning of consecutive bytes. For instance, "1,2,3,4" indicates
        // that least significant byte is first, while "4,3,2,1" indicates
        // that the most significant byte is first.
        //
        // The original specification allowed for arbitrary byte orders.
        // An example in the specification was "2,1,4,3" for a Motorola 68000
        // processor. The 3.0 specification added the "3,4,1,2" example
        // for the DEC PDP-11 processor.
        //
        // Fortunately, the world has simplified and today there are only
        // two byte orders used by modern processors:
        //
        //   "1,2,3,4" for least significant byte first.
        //
        //   "4,3,2,1" for most significant byte first.
        //
        // All other values are for ancient processors and are not supported
        // by this software.
        //
        // The $BYTEORD keyword is required in FCS 2.0, 3.0, and 3.1, but we
        // treat it as optional because it did not exist in FCS 1.0. If
        // present, it must have a reasonable modern value.
        if ( this->inDictionary( "$BYTEORD" ) == false )
        {
            // Use the FCS 1.0 default of "1,2,3,4".
            this->fileIsLSBF = true;
            this->fileAttributes.emplace( "byteOrder", "lsbf" );
        }
        else
        {
            // SPECIAL HANDLING: According to notes at FlowRepository.com,
            // some manufacturers are slopping in setting the byte order
            // shorthand in FCS files and use these non-standard values:
            //
            //   "1,2" for least significant byte first.
            //
            //   "2,1" for most significant byte first.
            //
            // So, below we accept these values as well.
            nv = this->getDictionaryString( "$BYTEORD" );
            if ( nv == "4,3,2,1" )
            {
                this->fileIsLSBF = false;
                this->fileAttributes.emplace( "byteOrder", "msbf" );
            }
            else if ( nv == "2,1" )
            {
                this->fileIsLSBF = false;
                this->fileAttributes.emplace( "byteOrder", "msbf" );
                this->appendFileLog(
                    "warning",
                    "The FCS file's $BYTEORD keyword has an invalid byte order of \"2,1\" used instead of a full \"4,3,2,1\". The byte order has been accepted anyway as meaning most-significant-byte-first." );
            }
            else if ( nv == "1,2,3,4" )
            {
                this->fileIsLSBF = true;
                this->fileAttributes.emplace( "byteOrder", "lsbf" );
            }
            else if ( nv == "1,2" )
            {
                this->fileIsLSBF = true;
                this->fileAttributes.emplace( "byteOrder", "lsbf" );
                this->appendFileLog(
                    "warning",
                    "The FCS file's $BYTEORD keyword has an invalid byte order of \"1,2\" used instead of a full \"1,2,3,4\". The byte order has been accepted anyway as meaning least-significant-byte-first." );
            }
            else
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $BYTEORD dictionary keyword used to indicate the byte order for stored data has an unsupported value of: \"" ) + nv +
                    std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Byte order:" <<
                    (this->fileIsLSBF == true ?
                        "Least-significant byte first\n" :
                        "Most-significant byte first\n");
        }

        //
        // Number of parameters.
        // ---------------------
        // The $PAR keyword has been required for all versions of the FCS
        // specification.
        //
        // Validate that it exists and that it has a reasoanble value.
        if ( this->inDictionary( "$PAR" ) == false )
        {
            this->appendFileLog(
                "error",
                "The $PAR dictionary keyword is missing and is required to indicate the number of parameters per event." );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        const size_t numberOfParameters = this->getDictionaryLong( "$PAR" );
        if ( numberOfParameters <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The $PAR dictionary keyword used to indicate the number of parameters per event has an invalid non-positive value: \"" ) +
                this->getDictionaryString( "$PAR" ) +
                std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of parameters:" <<
                numberOfParameters << "\n";

        //
        // Validate parameters.
        // --------------------
        // Each parameter is defined with a series of $P dictionary
        // keywords. Validate that they are defined and reasonable.
        //
        // A side-effect of this validation is that the narrowest and
        // widest parameter storage requirements are determined.
        int nBytesInEvent = 0;
        for ( size_t i = 0; i < numberOfParameters; ++i )
            nBytesInEvent += this->loadValidateParameter( i );

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Min parameter bytes:" <<
                this->fileMinParameterBytes << "\n";
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Max parameter bytes:" <<
                this->fileMaxParameterBytes << "\n";
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Max parameter range:" << "0 to " <<
                this->fileMaxParameterRange << "\n";
        }

        switch ( this->fileMaxParameterBytes )
        {
        case 1:
        case 2:
        case 3:
        case 4:
        case 8:
            break;

        default:
            this->appendFileLog(
                "error",
                std::string( "The file requires that some parameter values be stored with " ) +
                std::to_string( this->fileMaxParameterBytes ) +
                std::string( " bytes, but this size is not supported by this software." ) );
            throw std::runtime_error(
                ERROR_UNSUPPORTED +
                std::string( "The file requires that some parameter values be stored in an unusual way that is not supported by this software." ) +
                ERROR_CANNOTLOAD );
        }

        //
        // Calculate the number of bytes per event.
        // ----------------------------------------
        // The number of bytes per event equals (the number of parameters *
        // the size of those parameters). There are four options:
        // - Single-precision floating point parameters.
        // - Double-precision floating point parameters.
        // - Integer parameters.
        //
        // Both floating-point options have fixed sizes for each parameter
        // value: 4 and 8 bytes each for float and double values. This makes
        // it easy to calculate the number of bytes per event.
        //
        // The integer option can have a different number of BITS for each
        // integer parameter. This software does not support that option,
        // but rather requires that all parameters have 8, 16, 24, 32, or 64
        // bits (not 3 or 5 or 13 bits). The sum of those bits per parameter,
        // divided by 8, then gives the total bytes per parameter and event.
        const long segmentBegin = this->getDictionaryLong( "$BEGINDATA" );
        const long segmentEnd   = this->getDictionaryLong( "$ENDDATA" );
        const size_t nBytes = (size_t) segmentEnd - (size_t) segmentBegin + 1;

        //
        // Number of events.
        // -----------------
        // The $TOT keyword is required in FCS 3.1, but optional in 2.0
        // and 3.0, and absent in 1.0. Look for the keyword and validate
        // its value. But if the value is not found, compute the value
        size_t numberOfEvents = 0;
        if ( this->inDictionary( "$TOT" ) == true )
        {
            numberOfEvents = this->getDictionaryLong( "$TOT" );
            if ( numberOfEvents <= 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $TOT dictionary keyword for the number of events in the file has an invalid non-positive value: \"" ) +
                    this->getDictionaryString( "$TOT" ) +
                    std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            if ( (numberOfEvents * nBytesInEvent) > nBytes )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The $TOT dictionary keyword for the number of events in the the file specifies a number of events that exceeds the size of the file's data segment: \"" ) + 
                    this->getDictionaryString( "$TOT" ) +
                    std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
        }
        else
        {
            numberOfEvents = nBytes / nBytesInEvent;
            if ( (numberOfEvents * nBytesInEvent) != nBytes )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The number of bytes in the data segment does is not a multiple of the event size. Something is wrong in the file's byte offsets." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            this->setDictionaryStringInternal(
                "$TOT",
                std::to_string( numberOfEvents ),
                false,      // No need to convert keyword or trim.
                false );    // No need to trim value.
        }

        this->numberOfFileEvents = numberOfEvents;

        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " << "  " <<
                std::setw( 30 ) << std::left <<
                "Number of events:" <<
                numberOfEvents << "\n";
    }

    /**
     * Validates dictionary entries for a parameter.
     *
     * The following keywords and values are validated:
     * @li $PnB (required)
     * @li $PnCALIBRATION
     * @li $PnE (required)
     * @li $PnN (required)
     * @li $PnR (required)
     *
     * The following keywords and values are not validated. In each case,
     * the keyword is optional and the absence of a value or a negative value
     * is not significant enough to be worth throwing an exception and aborting
     * a file load:
     * @li $PnD
     * @li $PnF
     * @li $PnG
     * @li $PnL
     * @li $PnO
     * @li $PnP
     * @li $PnS
     * @li $PnT
     * @li $PnV
     *
     * @param[in] index
     *   The parameter index, starting at 0 for the first parameter.
     *
     * @return
     *   Returns the number of bytes the parameter requires in the file.
     *
     * @throws std::runtime_error
     *   Throws an exception if any required keyword is missing or if
     *   the values are malformed.
     */
    int loadValidateParameter( const size_t index )
    {
        int nBytesInParameter = 0;
        long ivalue = 0;
        double fvalue = 0.0;
        size_t sizeValue = 0;
        std::string svalue;
        std::string keyword;

        //
        // Number of bits: $PnB.
        // ---------------------
        // The "$PnB" keyword was defined in FCS 1.0 and continues to be
        // required in later specifications.
        //
        // The $DATATYPE keyword specifies the base data type for
        // all parameters. Values are one of:
        // - "I" for integers.
        // - "F" for single-precision floats.
        // - "D" for double-precision floats.
        // - "A" for ASCII, which is deprecated.
        //
        // For ASCII data, the "$PnB" value indicates the number of BYTES
        // required per parameter value, and the special "*" value indicates
        // that ASCII data has a variable number of characters per value
        // and that a delimiter is used between consecutive values. While
        // the 1.0 and 2.0 specifications say any of space, tab, comma, or
        // carriage return may be used as delimiters, there is nothing in the
        // file that says which one is used.
        //
        // Fortunately, ASCII data use has been deprecated in FCS 3.1. It is
        // difficult to parse, uses a lot of space in the file, and is rarely
        // used. This software does not support it and this form of data has
        // already been checked for earlier and files using it rejected.
        //
        // For binary floating point data, the value of "$PnB" is always
        // "32" if $DATATYPE specified "F" for single-precision floats. The
        // value of "$PnB" is always "64" if $DATATYPE specified "D" for
        // double-precision floats.
        //
        // For binary integer data where $DATATYPE specified "I", the value
        // of "$PnB" indicates the number of BITS (not bytes!) required in
        // the file to store the parameter's values.  It may have seemed like
        // a good idea to do bit packing to save file space back in 1984 when
        // FCS 1.0 was defined, but today this is cumbersome. Storage space
        // is plentiful and the extra effort to do bit packing and unpacking
        // is not necessary. The FCS 3.1 specification encourages normal
        // values here that are at byte boundaries. This software therefore
        // requires values like 8, 16, 24, 32, or 64.
        keyword = this->buildParameterKeyword( "$P", index, "B" );
        if ( this->inDictionary( keyword ) == false )
        {
            this->appendFileLog(
                "error",
                std::string( "The " ) + keyword +
                std::string( " keyword is missing and is required to indicate the parameter's number of stored bits." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        sizeValue = (size_t) this->getDictionaryLong( keyword );

        switch ( this->fileDataType )
        {
        case 'F':
            if ( sizeValue != 32 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + keyword +
                    std::string( " keyword for the parameter's number of stored bits for single-precision float data has an invalid value: \"" ) +
                    this->getDictionaryString( keyword ) +
                    std::string( "\". The value must be \"32\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            nBytesInParameter = 4;
            break;

        case 'D':
            if ( sizeValue != 64 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + keyword +
                    std::string( " keyword for the parameter's number of stored bits for double-precision float data has an invalid value: \"" ) +
                    this->getDictionaryString( keyword ) +
                    std::string( "\". The value must be \"64\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
            nBytesInParameter = 8;
            break;

        default:
        case 'I':
            switch ( sizeValue )
            {
            case 8:
            case 16:
            case 24:
            case 32:
            case 64:
                nBytesInParameter = sizeValue / 8;
                break;

            default:
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + keyword +
                    std::string( " keyword for the parameter's number of stored bits for integer data has an unsupported value: \"" ) +
                    this->getDictionaryString( keyword ) +
                    std::string( "\". Only 8, 16, 24, 32, and 64-bit integer values are supported." ) );
                throw std::runtime_error(
                    ERROR_UNSUPPORTED +
                    std::string( "The file's data is stored using an unusual integer size that is not supported by this software." ) +
                    ERROR_CANNOTLOAD );
            }
            break;
        }

        sizeValue /= 8;
        if ( sizeValue < this->fileMinParameterBytes )
            this->fileMinParameterBytes = sizeValue;
        if ( sizeValue > this->fileMaxParameterBytes )
            this->fileMaxParameterBytes = sizeValue;

        //
        // Calibration scaling: $PnCALIBRATION.
        // ------------------------------------
        // FCS 3.1 introduced the "$PnCALIBRATION" keyword. The value is
        // a comma separated pair where the first value is a floating-point
        // value used to map parameter values to a target unit of measure,
        // and the second value is the name of that unit of measure.
        //
        // The keyword is optional. When present, the value must be parsable
        // and contain a positive floating-point scaling factor and a
        // non-empty units value.
        keyword = this->buildParameterKeyword( "$P", index, "CALIBRATION" );
        if ( this->inDictionary( keyword ) == true )
        {
            // The value for the keyword is a comma-separated tuple: f,s.
            // - "f" is the calibration scaling factor.
            // - "s" is the units of measure post scaling.
            auto s = this->getDictionaryString( keyword );

            size_t commaPosition = s.find( ',' );
            if ( commaPosition == std::string::npos ||
                 commaPosition == 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + keyword +
                    std::string( " keyword for the parameter's calibration scaling has an invalid value: \"" ) +
                    this->getDictionaryString( keyword ) +
                    std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            fvalue = std::stof( s.substr( 0, commaPosition ) );
            if ( fvalue <= 0.0f )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + keyword +
                    std::string( " keyword for the parameter's calibration scaling has an invalid scaling value: \"" ) +
                    std::to_string( fvalue ) +
                    std::string( "\"." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }

            svalue = s.substr( commaPosition + 1 );
            if ( svalue.empty( ) == true )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The " ) + keyword +
                    std::string( " keyword for the parameter's calibration scaling has an invalid empty scaling units name." ) );
                throw std::runtime_error(
                    ERROR_MALFORMED +
                    ERROR_BADCONTENTS +
                    ERROR_CANNOTLOAD );
            }
        }

        //
        // Instrument visualization scale: $PnD.
        // -------------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a string value and non-negative value is not worth an exception
        // for a fatal error. So, do not validate.

        //
        // Instrument log gain: $PnE.
        // --------------------------
        // The "$PnE" keyword was introduced in FCS 2.0, but not required.
        // For FCS 3.0 and 3.1, the keyword is required.
        //
        // The keyword is a string with two comma-separated floating-point
        // values. The first is the logarithmic scaling factor (the number
        // of decades) and the second is an offset.
        //
        // For linear scaling, FCS 2.0 specifies that the first value should
        // be 0.0. FCS 3.0 and 3.1 say that BOTH values should be 0.0 for
        // linear scaling.
        //
        // Validation checks that the value can be parsed into two floats
        // and that neither float is negative.
        keyword = this->buildParameterKeyword( "$P", index, "E" );
        if ( this->inDictionary( keyword ) == true )
        {
            // The value for the keyword is a comma-separated tuple: f1,f2.
            // "f1" is the log scale factor.
            // "f1" is the log offset.
            auto s = this->getDictionaryString( keyword );

            size_t commaPosition = s.find( ',' );
            if ( commaPosition == std::string::npos ||
                 commaPosition == 0 )
            {
                this->appendFileLog(
                    "warning",
                    std::string( "The FCS file's " ) + keyword +
                    std::string( " keyword for the parameter's log scaling has an invalid value: \"" ) +
                    this->getDictionaryString( keyword ) +
                    std::string( "\"." ) );
            }
            else
            {
                const double decades = std::stof( s.substr( 0, commaPosition ) );
                if ( decades < 0.0 )
                {
                    this->appendFileLog(
                        "warning",
                        std::string( "The FCS file's " ) + keyword +
                        std::string( " keyword for the parameter's log scaling has an invalid negative scale value: \"" ) +
                        std::to_string( decades ) +
                        std::string( "\"." ) );
                }
                else
                {
                    const double offset = std::stof( s.substr( commaPosition + 1 ) );
                    if ( offset < 0.0 )
                    {
                        this->appendFileLog(
                            "warning",
                            std::string( "The FCS file's " ) + keyword +
                            std::string( " keyword for the parameter's log scaling has an invalid negative offset value: \"" ) +
                            std::to_string( offset ) +
                            std::string( "\"." ) );
                    }
                    else if ( this->fileDataType != 'I' &&
                        (decades != 0.0 || offset != 0.0) )
                    {
                        this->appendFileLog(
                            "warning",
                            std::string( "The FCS file's " ) + keyword +
                            std::string( " keyword for the parameter's log scaling is not zero, but should be for the file's floating point data. This is out-of-spec, but is supported by this software." ) );
                    }
                }
            }
        }

        //
        // Instrument optical filter: $PnF.
        // --------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a string value is not worth an exception for a fatal error.
        // So, do not validate.

        //
        // Instrument linear gain: $PnG.
        // -----------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a value, or a non-positive value, is not worth an exception for a
        // fatal error. So, do not validate.

        //
        // Instrument excitation wavelengths: $PnL.
        // ----------------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a value, or an odd value, is not worth an exception for a
        // fatal error. So, do not validate.

        //
        // Short name: $PnN.
        // -----------------
        // The "$PnN" keyword has been required for FCS 1.0, 2.0, 3.0,
        // and 3.1, though the meaning has changed a bit. In 1.0, 2.0, and
        // 3.0, the allowed values were fixed strings, while in 3.1 the
        // value can be any string.
        //
        // FCS 1.0 and 2.0 required that the time parameter have a short
        // name of "TI". FCS 3.0 and 3.1 require that it instead be
        // named "TIME".
        //
        // Here we verify that the value exists. All values are supported,
        // but the special case of "TI" is automatically mapped to "TIME".
        keyword = this->buildParameterKeyword( "$P", index, "N" );
        if ( this->inDictionary( keyword ) == false )
        {
            this->appendFileLog(
                "error",
                std::string( "The " ) + keyword +
                std::string( " keyword is missing and is required to indicate a short parameter name." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        svalue = this->getDictionaryString( keyword );
        const std::string& versionNumber =
            this->fileAttributes.at( "versionNumber" );
        if ( (versionNumber == "FCS1.0" ||
              versionNumber == "FCS2.0") &&
              (svalue == "TI" || svalue == "ti" ||
               svalue == "Ti" || svalue == "tI" ) )
        {
            svalue = std::string( "TIME" );
            this->setDictionaryStringInternal(
                keyword,
                svalue,
                false,      // No need to convert keyword or trim.
                false );    // No need to trim the value.
        }
        else if ( svalue.empty( ) == true )
        {
            this->appendFileLog(
                "error",
                std::string( "The " ) + keyword +
                std::string( " keyword for the parameter's short name is empty, but a value is required." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        //
        // Maximum range: $PnR.
        // --------------------
        // The maximum range has little meaning for floating point values,
        // which can be negative and more positive and negative than the
        // maximum range value.
        //
        // For integer values, the maximum range is used to create a bitmask
        // to remove non-significant upper bits.
        //
        // For all versions of the specification, the keyword is required.
        // Validation just checks that the value exists and is not zero or
        // negative.
        keyword = this->buildParameterKeyword( "$P", index, "R" );
        if ( this->inDictionary( keyword ) == false )
        {
            this->appendFileLog(
                "error",
                std::string( "The " ) + keyword +
                std::string( " keyword is missing and is required to indicate the parameter's maximum numeric range." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        ivalue = this->getDictionaryLong( keyword );
        if ( ivalue <= 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The " ) + keyword +
                std::string( " keyword for the parameter's maximum numeric range has an invalid non-positive value: \"" ) +
                this->getDictionaryString( keyword ) +
                std::string( "\"." ) );
            throw std::runtime_error(
                ERROR_MALFORMED +
                ERROR_BADCONTENTS +
                ERROR_CANNOTLOAD );
        }

        if ( ivalue > this->fileMaxParameterRange )
            this->fileMaxParameterRange = ivalue;

        //
        // Instrument excitation power: $PnO.
        // ----------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a value, or a non-positive value, is not worth an exception for a
        // fatal error. So, do not validate.

        //
        // Instrument detector percent of light emitted: $PnP.
        // ---------------------------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a value, or a negative value, is not worth an exception for a
        // fatal error. So, do not validate.

        //
        // Long name: $PnS.
        // ----------------
        // The keyword is optional. While it should not be empty, the absence
        // of a string value is not worth an exception for a fatal error.
        // So, do not validate.

        //
        // Instrument detector type: $PnT.
        // -------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a string value is not worth an exception for a fatal error.
        // So, do not validate.

        //
        // Instrument detector voltage: $PnV.
        // ----------------------------------
        // The keyword is optional. While it should not be empty, the absence
        // of a value, or a negative value, is not worth an exception for a
        // fatal error. So, do not validate.

        return nBytesInParameter;
    }

    /**
     * Loads the DATA segment of the file.
     *
     * The DATA segment is a block of binary data composed entirely of
     * integers, floats, or doubles.
     *
     * A maximum number of events may be set to truncate the input and
     * limit the number of events loaded and available for further use.
     * A value of zero does not load any event data. A value of (-1) loads
     * all event data. In all cases, an event table is created and initialized
     * with parameter information from the file.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to load.
     *   A value of zero does not load any data.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's content cannot be parsed.
     */
    void loadDataSegment( std::FILE*const fp, ssize_t maximumEvents = (-1) )
    {
        const long segmentBegin = this->getDictionaryLong( "$BEGINDATA" );

        //
        // Seek to start
        // -------------
        // Use the byte offset of the DATA segment from the header to seek
        // to it's start.
        if ( fseek( fp, segmentBegin, SEEK_SET ) != 0 )
        {
            this->appendFileLog(
                "error",
                std::string( "The file is truncated before the start of the data segment." ) );
            throw std::runtime_error(
                ERROR_TRUNCATED +
                "The file is missing critical information." +
                ERROR_CANNOTLOAD );
        }


        //
        // Truncate.
        // ---------
        // Adjust the number of events to read.
        if ( maximumEvents < 0 )
            maximumEvents = this->getNumberOfEvents( );

        if ( (size_t) maximumEvents < this->getNumberOfEvents( ) )
        {
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " <<
                  "  Truncating input from " <<
                  this->getNumberOfEvents( ) <<
                  " to " << maximumEvents << " events\n";

            // Set the dictionary to the modified event number.
            this->setDictionaryStringInternal(
                "$TOT",
                std::to_string( maximumEvents ),
                false,      // No need to convert keyword.
                false );    // No need to trim value.
        }


        //
        // Load data.
        // ----------
        // The data segment is a series of events. Each event has a series
        // of numeric parameter values in the same order as parameters are
        // defined.
        //
        // All numeric values have the same data type (integer, float,
        // or double). When the data type is float or double, all values
        // for all parameters have the same 32-bit or 64-bit size. When
        // the data type is integer, each parameter can have a different
        // number of bits (which this software forces to 8, 16, 24, 32, or 64).
        // Each integer parameter also needs to be masked based upon a
        // maximum value range.
        //
        // Because integer, float, and double parameters are handled
        // differently, we dispatch to separate data load methods for each one.
        //
        // Integer data will be converted to floating point before storage
        // into an event table. That table can be single- or
        // double-precision, and the choice depends upon the size of the
        // integer data type and the numeric range of stored values.
        //
        // By default, use single-precision. But use double-precision if:
        // - Integers are 64-bit.
        // - Integers are 32-bit and the range is unknown.
        // - Integers are 32-bit and the range extends past 24 bits.
        //
        // The last case is important because storing a 32-bit integer
        // as a 32-bit IEEE floating point restricts it to just 24 bits
        // for the mantissa. If more than 24 bits are needed in the integer,
        // then resolution is lost by converting to a float. A double is
        // needed instead.
        if ( this->fileDataType == 'F' )
        {
            // When using floating-point, all parameters always have
            // the same float type in the file.
            this->loadUniformFloatDataSegment<float>( fp );
        }
        else if ( this->fileDataType == 'D' )
        {
            // When using floating-point, all parameters always have
            // the same float type in the file.
            this->loadUniformFloatDataSegment<double>( fp );
        }
        else if ( this->fileMaxParameterBytes == 1 )
        {
            // When using 8-bit integers, all parameters always have the
            // same 1-byte size in the file.
            this->loadUniformIntegerDataSegment<uint8_t,float>( fp );
        }
        else if ( this->fileMaxParameterBytes == 2 )
        {
            // If all of the parameters have the same size, then use
            // faster uniform integer size loading.
            if ( this->fileMaxParameterBytes == this->fileMinParameterBytes )
                this->loadUniformIntegerDataSegment<uint16_t,float>( fp );
            else
                this->loadNonuniformIntegerDataSegment<uint16_t,float>( fp );
        }
        else if ( this->fileMaxParameterBytes == 3 )
        {
            // Since there is no such thing as a 3-byte integer on modern
            // processors, there is no way to do a fast load of uniform
            // integer sizes (if they are). Always load as non-uniform.
            this->loadNonuniformIntegerDataSegment<uint32_t,float>( fp );
        }
        else if ( this->fileMaxParameterBytes == 4 )
        {
            if ( this->fileMaxParameterRange == 0 ||
                (this->fileMaxParameterRange & 0xFF000000) != 0 )
            {
                // If all of the parameters have the same size, then use
                // faster uniform integer size loading.
                if ( this->fileMaxParameterBytes == this->fileMinParameterBytes )
                    this->loadUniformIntegerDataSegment<uint32_t,double>( fp );
                else
                    this->loadNonuniformIntegerDataSegment<uint32_t,double>( fp );
            }
            else
            {
                // If all of the parameters have the same size, then use
                // faster uniform integer size loading.
                if ( this->fileMaxParameterBytes == this->fileMinParameterBytes )
                    this->loadUniformIntegerDataSegment<uint32_t,float>( fp );
                else
                    this->loadNonuniformIntegerDataSegment<uint32_t,float>( fp );
            }
        }
        else
        {
            // If all of the parameters have the same size, then use
            // faster uniform integer size loading.
            if ( this->fileMaxParameterBytes == this->fileMinParameterBytes )
                this->loadUniformIntegerDataSegment<uint64_t,double>( fp );
            else
                this->loadNonuniformIntegerDataSegment<uint64_t,double>( fp );
        }
    }

    /**
     * Loads uniform-size floating point data from the DATA segment of the file.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's content cannot be parsed.
     */
    template<typename FLOAT>
    void loadUniformFloatDataSegment( std::FILE*const fp )
    {
        static_assert(
            std::is_same<float,FLOAT>::value || std::is_same<double,FLOAT>::value,
            "Method requires a floating-point template type." );

        //
        // Create event table.
        // -------------------
        // Create the table, initialized to zeroes. The number of events
        // for the table has already been truncated to the maximum requested.
        const size_t numberOfEvents = this->getNumberOfEvents( );

        this->eventTable.reset( new FlowGate::Events::EventTable(
            this->getParameterShortNames( ),
            numberOfEvents,
            std::is_same<float, FLOAT>::value ) );

        this->eventTable->setParameterLongNames(
            this->getParameterLongNames( ) );

        this->eventTable->setNumberOfOriginalEvents(
            this->numberOfFileEvents );

        if ( numberOfEvents == 0 )
            return;


        //
        // Set up.
        // -------
        // Get the number of parameters and pointers to the parameter data.
        // Determine the number of events to read at a time and whether
        // we need to swap bytes. Allocate a block to hold read values.
        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        // Get pointers to each of the parameter's arrays.
        FLOAT** parameterData = new FLOAT*[numberOfParameters];

        if constexpr ( std::is_same<float,FLOAT>::value == true )
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterFloats(i).data( );
        }
        else
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterDoubles(i).data( );
        }

        // Allocate a block of values to be filled through
        // consecutive reads.
        FLOAT* block = new FLOAT[numberOfParameters *
            ((LOAD_EVENT_BLOCK_SIZE > numberOfEvents) ?
            numberOfEvents : LOAD_EVENT_BLOCK_SIZE)];

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
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading " << numberOfEvents;

            if ( std::is_same<float, FLOAT>::value == true )
                std::cerr << " 32-bit floats";
            else
                std::cerr << " 64-bit doubles";

            std::cerr << " for " << numberOfParameters << " parameters" <<
                ((swap == true) ?
                " swapping bytes\n" :
                " in host byte order\n");
        }


        //
        // Read the values.
        // ----------------
        // Events in an FCS file are organized in rows, with consecutive
        // values for consecutive parameters of the same event.
        //
        // Events in an event table are organized in columns, because this
        // is more efficient for event computation for compensation,
        // transforms, and gating. But this means we need to transpose the
        // incoming data.
        //
        // So load events in large blocks, to minimize I/O function calls,
        // and then distribute the block of events to the event table's
        // parameters.
        for ( size_t eventIndex = 0;
            eventIndex < numberOfEvents;
            eventIndex += LOAD_EVENT_BLOCK_SIZE )
        {
            // Determine the number of events to read. Either all the rest
            // or the block size.
            size_t nEventsToRead = (numberOfEvents - eventIndex);
            if ( nEventsToRead > LOAD_EVENT_BLOCK_SIZE )
                nEventsToRead = LOAD_EVENT_BLOCK_SIZE;

            // Calculate the number of values to read.
            const size_t nValuesToRead = numberOfParameters * nEventsToRead;

            // Read a block.
            if ( std::fread( block, sizeof(FLOAT), nValuesToRead, fp ) != nValuesToRead )
            {
                delete[] parameterData;
                delete[] block;

                this->appendFileLog(
                    "error",
                    std::string( "The file is truncated before the end of the data segment." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }

            // Swap bytes in the block, if needed.
            if ( swap == true )
            {
                if ( std::is_same<float, FLOAT>::value == true )
                {
                    uint32_t* bytes = (uint32_t*)block;
                    for ( size_t i = 0; i < nValuesToRead; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
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
                else
                {
                    uint64_t* bytes = (uint64_t*)block;
                    for ( size_t i = 0; i < nValuesToRead; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
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

            // Distribute values to parameters.
            for ( size_t j = 0; j < nEventsToRead; ++j )
            {
                const size_t jn = j * numberOfParameters;
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                    parameterData[i][eventIndex+j] = block[jn + i];
            }
        }

        delete[] block;
        delete[] parameterData;
    }

    /**
     * Loads uniform-size integer data from the DATA segment of the file.
     *
     * Integer data is converted to single- or double-precision floating point.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's content cannot be parsed.
     */
    template <typename INTEGER, typename FLOAT>
    void loadUniformIntegerDataSegment( std::FILE*const fp )
    {
        static_assert(
            std::is_same<float,FLOAT>::value || std::is_same<double,FLOAT>::value,
            "Method requires a floating-point template type." );

        //
        // Create event table.
        // -------------------
        // Create the table, initialized to zeroes. The number of events
        // for the table has already been truncated to the maximum requested.
        const size_t numberOfEvents = this->getNumberOfEvents( );

        this->eventTable.reset( new FlowGate::Events::EventTable(
            this->getParameterShortNames( ),
            numberOfEvents,
            std::is_same<float, FLOAT>::value ) );

        this->eventTable->setParameterLongNames(
            this->getParameterLongNames( ) );

        this->eventTable->setNumberOfOriginalEvents(
            this->numberOfFileEvents );

        if ( numberOfEvents == 0 )
            return;


        //
        // Set up.
        // -------
        // Get the number of parameters and pointers to the parameter data.
        // Determine the number of events to read.  Allocate a block to hold
        // read values. Determine if integer masking is needed.
        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        // Get pointers to each of the parameter's arrays.
        FLOAT** parameterData = new FLOAT*[numberOfParameters];
        if constexpr ( std::is_same<float,FLOAT>::value == true )
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterFloats(i).data( );
        }
        else
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterDoubles(i).data( );
        }

        // Allocate a block of values to be filled through
        // consecutive reads.
        INTEGER* block = new INTEGER[numberOfParameters *
            ((LOAD_EVENT_BLOCK_SIZE > numberOfEvents) ?
            numberOfEvents : LOAD_EVENT_BLOCK_SIZE)];

        // Get the integer value mask for each parameter and check if any
        // masking is needed.
        const INTEGER*const masks = this->buildParameterMasks<INTEGER>( );
        INTEGER minMask = 0;
        for ( uint32_t i = 0; i < numberOfParameters; ++i )
            if ( masks[i] < minMask )
                minMask = masks[i];

        const bool maskNeeded = (minMask != (INTEGER)(~0u));

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        // Host is LSBF. Swap if the file is not LSBF.
        const bool swap = (sizeof(INTEGER) != 1) && (this->fileIsLSBF == false);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        // Host is MSBF. Swap if the file is LSBF.
        const bool swap = (sizeof(INTEGER) != 1) && (this->fileIsLSBF == true);
#else
        const bool swap = false;
#endif

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading " << numberOfEvents <<
                (this->fileMaxParameterBytes * 8) <<
                "-bit integers into ";

            if ( std::is_same<float, FLOAT>::value == true )
                std::cerr << "32-bit floats";
            else
                std::cerr << "64-bit doubles";

            std::cerr << " for " << numberOfParameters << " parameters";
            if ( sizeof(INTEGER) != 1 )
                std::cerr << ((swap == true) ?
                " swapping bytes\n" :
                " in host byte order\n");
        }


        //
        // Read the values.
        // ----------------
        // Events in an FCS file are organized in rows, with consecutive
        // values for consecutive parameters of the same event.
        //
        // Events in an event table are organized in columns, because this
        // is more efficient for event computation for compensation,
        // transforms, and gating. But this means we need to transpose the
        // incoming data.
        //
        // Load events in large blocks, to minimize I/O function calls,
        // and then distribute the block of events to the event table's
        // parameters.
        for ( size_t eventIndex = 0;
            eventIndex < numberOfEvents;
            eventIndex += LOAD_EVENT_BLOCK_SIZE )
        {
            // Determine the number of events to read. Either all the rest
            // or the block size.
            size_t nEventsToRead = (numberOfEvents - eventIndex);
            if ( nEventsToRead > LOAD_EVENT_BLOCK_SIZE )
                nEventsToRead = LOAD_EVENT_BLOCK_SIZE;

            // Calculate the number of values to read.
            const size_t nValuesToRead = numberOfParameters *
                nEventsToRead;

            // Read a block.
            if ( std::fread( block, sizeof(INTEGER), nValuesToRead, fp ) != nValuesToRead )
            {
                delete[] block;
                delete[] parameterData;
                delete[] masks;

                this->appendFileLog(
                    "error",
                    std::string( "The file is truncated before the end of the data segment." ) );
                throw std::runtime_error(
                    ERROR_TRUNCATED +
                    "The file is missing critical information." +
                    ERROR_CANNOTLOAD );
            }

            // Swap bytes in the block, if needed.
            if ( swap == true )
            {
                if ( sizeof(INTEGER) == 2 )
                {
                    uint16_t* bytes = (uint16_t*)block;
                    for ( size_t i = 0; i < nValuesToRead; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
                        *bytes = __builtin_bswap16(*bytes);
#else
                        const uint16_t ui = *bytes;
                        *bytes =
                            ((ui & 0xFF00u) >> 8u) |
                            ((ui & 0x00FFu) << 8u);
#endif
                        ++bytes;
                    }
                }
                else if ( sizeof(INTEGER) == 4 )
                {
                    uint32_t* bytes = (uint32_t*)block;
                    for ( size_t i = 0; i < nValuesToRead; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
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
                else
                {
                    uint64_t* bytes = (uint64_t*)block;
                    for ( size_t i = 0; i < nValuesToRead; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
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

            // Distribute values to parameters. Mask if needed.
            if ( maskNeeded == false )
            {
                for ( size_t j = 0; j < nEventsToRead; ++j )
                {
                    const size_t jn = j * numberOfParameters;
                    for ( uint32_t i = 0; i < numberOfParameters; ++i )
                        parameterData[i][eventIndex+j] = (FLOAT)block[jn + i];
                }
            }
            else
            {
                for ( size_t j = 0; j < nEventsToRead; ++j )
                {
                    const size_t jn = j * numberOfParameters;
                    for ( uint32_t i = 0; i < numberOfParameters; ++i )
                        parameterData[i][eventIndex+j] = (FLOAT)(block[jn + i] & masks[i]);
                }
            }
        }

        delete[] block;
        delete[] parameterData;
        delete[] masks;
    }

    /**
     * Loads non-uniform-size integer data from the DATA segment of the file.
     *
     * Integer data is converted to single- or double-precision floating point.
     *
     * @param[in] fp
     *   The file pointer for the file to read.
     *
     * @throws std::runtime_error
     *   Throws an exception if the file's content cannot be parsed.
     */
    template <typename INTEGER, typename FLOAT>
    void loadNonuniformIntegerDataSegment( std::FILE*const fp )
    {
        static_assert(
            std::is_same<float,FLOAT>::value || std::is_same<double,FLOAT>::value,
            "Method requires a floating-point template type." );

        //
        // Create event table.
        // -------------------
        // Create the table, initialized to zeroes. The number of events
        // for the table has already been truncated to the maximum requested.
        const size_t numberOfEvents = this->getNumberOfEvents( );

        this->eventTable.reset( new FlowGate::Events::EventTable(
            this->getParameterShortNames( ),
            numberOfEvents,
            std::is_same<float, FLOAT>::value ) );

        this->eventTable->setParameterLongNames(
            this->getParameterLongNames( ) );

        this->eventTable->setNumberOfOriginalEvents(
            this->numberOfFileEvents );

        if ( numberOfEvents == 0 )
            return;


        //
        // Set up.
        // -------
        // Get the number of parameters and pointers to the parameter data.
        // Determine the number of events to read.  Allocate a block to hold
        // read values. Determine if integer masking is needed.
        const uint32_t numberOfParameters =
            this->eventTable->getNumberOfParameters( );

        // Get pointers to each of the parameter's arrays.
        FLOAT** parameterData = new FLOAT*[numberOfParameters];
        if constexpr ( std::is_same<float,FLOAT>::value == true )
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterFloats(i).data( );
        }
        else
        {
            for ( uint32_t i = 0; i < numberOfParameters; ++i )
                parameterData[i] = this->eventTable->getParameterDoubles(i).data( );
        }

        // Get the integer value mask for each parameter and check if any
        // masking is needed.
        const INTEGER*const masks = this->buildParameterMasks<INTEGER>( );
        INTEGER minMask = 0;
        for ( uint32_t i = 0; i < numberOfParameters; ++i )
            if ( masks[i] < minMask )
                minMask = masks[i];

        const bool maskNeeded = (minMask != (INTEGER)(~0u));

        // Get the number of bytes used in the file for each parameter.
        const size_t*const bytesPerParameter = this->buildParameterSizes( );

        // Allocate a temp array for one row of event values, widened to
        // the widest integer used for all parameters.
        INTEGER*const block = new INTEGER[numberOfParameters];

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        // Host is LSBF. Swap if the file is not LSBF.
        const bool swap = (sizeof(INTEGER) != 1) && (this->fileIsLSBF == false);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        // Host is MSBF. Swap if the file is LSBF.
        const bool swap = (sizeof(INTEGER) != 1) && (this->fileIsLSBF == true);
#else
        const bool swap = false;
#endif

        if ( this->verbose == true )
        {
            std::cerr << this->verbosePrefix << ": " <<
                "  Loading " << numberOfEvents <<
                (this->fileMaxParameterBytes * 8) <<
                "-bit integers into ";

            if ( std::is_same<float, FLOAT>::value == true )
                std::cerr << "32-bit floats";
            else
                std::cerr << "64-bit doubles";

            std::cerr << " for " << numberOfParameters << " parameters";
            if ( sizeof(INTEGER) != 1 )
                std::cerr << ((swap == true) ?
                " swapping bytes\n" :
                " in host byte order\n");
            std::cerr << this->verbosePrefix << ": " <<
                "      Reading bytes one parameter at a time.\n";
        }


        //
        // Read the values.
        // ----------------
        // Events in an FCS file are organized in rows, with consecutive
        // values for consecutive parameters of the same event.
        //
        // Events in an event table are organized in columns, because this
        // is more efficient for event computation for compensation,
        // transforms, and gating. But this means we need to transpose the
        // incoming data.
        //
        // Parameters do not have uniform size (which is rare) or they have
        // a non-standard size (e.g. 3-byte integers). Loop through all
        // events and parameters and read each parameter value one at a time.
        for ( size_t eventIndex = 0; eventIndex < numberOfEvents; ++eventIndex )
        {
            uint8_t  tmp8;
            uint16_t tmp16;
            uint32_t tmp32;
            uint64_t tmp64;
            uint8_t tmp24[3];
            INTEGER* values = block;

            for ( uint32_t i = 0; i < numberOfParameters; ++i )
            {
                const size_t nBytes = bytesPerParameter[i];

                switch ( nBytes )
                {
                case 1:
                    // Read a 1-byte parameter value into a 1-byte temp.
                    if ( std::fread( &tmp8, nBytes, 1, fp ) != 1 )
                    {
                        delete[] bytesPerParameter;
                        delete[] block;
                        delete[] parameterData;
                        delete[] masks;
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the end of the data segment." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }
                    *values = tmp8;
                    break;

                case 2:
                    // Read a 2-byte parameter value into a 2-byte temp.
                    if ( std::fread( &tmp16, nBytes, 1, fp ) != 1 )
                    {
                        delete[] bytesPerParameter;
                        delete[] block;
                        delete[] parameterData;
                        delete[] masks;
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the end of the data segment." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }
                    *values = tmp16;
                    break;

                case 3:
                    // Read a 3-byte parameter value into a 4-byte temp array,
                    // then copy it to a value in the array. Retain the same
                    // byte order in the value as that from the file.
                    if ( std::fread( tmp24, 1, 3, fp ) != 3 )
                    {
                        delete[] bytesPerParameter;
                        delete[] block;
                        delete[] parameterData;
                        delete[] masks;
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the end of the data segment." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }
                    *values = (tmp24[0] << 24) | (tmp24[1] << 16) | (tmp24[2] << 8);

                case 4:
                    // Read a 4-byte parameter value into a 4-byte temp.
                    if ( std::fread( &tmp32, nBytes, 1, fp ) != 1 )
                    {
                        delete[] bytesPerParameter;
                        delete[] block;
                        delete[] parameterData;
                        delete[] masks;
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the end of the data segment." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }
                    *values = tmp32;
                    break;

                case 8:
                    // Read a 8-byte parameter value into an 8-byte temp.
                    if ( std::fread( &tmp64, nBytes, 1, fp ) != 1 )
                    {
                        delete[] bytesPerParameter;
                        delete[] block;
                        delete[] parameterData;
                        delete[] masks;
                        this->appendFileLog(
                            "error",
                            std::string( "The file is truncated before the end of the data segment." ) );
                        throw std::runtime_error(
                            ERROR_TRUNCATED +
                            "The file is missing critical information." +
                            ERROR_CANNOTLOAD );
                    }
                    *values = tmp64;
                    break;
                }
            }

            // Swap bytes in the single-event array, if needed.
            if ( swap == true )
            {
                if ( sizeof(INTEGER) == 2 )
                {
                    uint16_t* bytes = (uint16_t*)block;
                    for ( size_t i = 0; i < numberOfParameters; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
                        *bytes = __builtin_bswap16(*bytes);
#else
                        const uint16_t ui = *bytes;
                        *bytes =
                            ((ui & 0xFF00u) >> 8u) |
                            ((ui & 0x00FFu) << 8u);
#endif
                        ++bytes;
                    }
                }
                else if ( sizeof(INTEGER) == 4 )
                {
                    uint32_t* bytes = (uint32_t*)block;
                    for ( size_t i = 0; i < numberOfParameters; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
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
                else
                {
                    uint64_t* bytes = (uint64_t*)block;
                    for ( size_t i = 0; i < numberOfParameters; ++i )
                    {
#ifdef FLOWGATE_FILEFCS_USE_BSWAP
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

            // Distribute values to parameters. Mask if needed.
            if ( maskNeeded == false )
            {
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                    parameterData[i][eventIndex] = (FLOAT)block[i];
            }
            else
            {
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                    parameterData[i][eventIndex] = (FLOAT)(block[i] & masks[i]);
            }
        }

        delete[] bytesPerParameter;
        delete[] block;
        delete[] parameterData;
        delete[] masks;
    }
    // @}



//----------------------------------------------------------------------
//
// Save file.
//
//----------------------------------------------------------------------
public:
    /**
     * @name Save file
     */
    // @{
    /**
     * Saves the object to a new FCS file.
     *
     * The file is written with data from the current object.
     *
     * A maximum number of events may be set to limit the number of events
     * saved to the file.  A value of zero does not save any event data.
     * A value of (-1) saves all event data.
     *
     * @param[in] filePath
     *   The path to the file to save.
     * @param[in] maximumEvents
     *   (optional, default = -1 = all) The maximum number of events to save.
     *   A value of zero does not save any data.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the path is empty or the indicated file
     *   cannot be opened.
     * @throws std::runtime_error
     *   Throws an exception if there is a critical problem when saving
     *   the file. Save problems are also reported to the file log.
     *
     * @see getFileLog()
     */
    void save(
        const std::string& filePath,
        ssize_t maximumEvents = (-1) )
    {
        //
        // Validate.
        // ---------
        // Make sure the path is not empty.
        if ( filePath.empty() == true )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                "Invalid empty file path." );
        if ( this->eventTable == nullptr )
            throw std::invalid_argument(
                ERROR_PROGRAMMER +
                std::string( "Invalid NULL event table." ) );

        //
        // Initialize.
        // -----------
        // Save the path.
        if ( this->verbose == true )
            std::cerr << this->verbosePrefix << ": " <<
                "Saving FCS file \"" << filePath << "\".\n";

        this->clearFileLog();

        std::string path = filePath;
        this->fileAttributes.emplace( "path", path );


        //
        // Open the file.
        // --------------
        // Open the file for buffered binary writing using stdio.
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
        // Setup.
        // ------
        // The FCS file header contains the following:
        // - 6 bytes for the file format version (e.g. "FCS3.1").
        // - 4 bytes of blanks.
        // - Three pairs of 8-byte offsets in ASCII.
        //   - TEXT segment begin/end.
        //   - DATA segment begin/end.
        //   - ANALYSIS segment begin/end.
        //
        // The total is then (6 + 4 + 3*2*8) = 58 bytes.
        const size_t headerLength = 58;

        // The 8 bytes for each offset have a maximum value of 10^8 = 100K,
        // when treated as a base 10 number.
        const size_t maxHeaderOffset = (size_t) std::pow( 10, 8 ) - 1;

        // A file offset in size_t requires some number of bytes when expressed
        // as ASCII in base 10.
        const int nDigitsForOffset = std::numeric_limits<size_t>::digits10;

        // The output byte order is whatever this host is using.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        this->fileIsLSBF = true;
        this->fileAttributes.emplace( "byteOrder", "lsbf" );
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        this->fileIsLSBF = false;
        this->fileAttributes.emplace( "byteOrder", "msbf" );
#endif

        // Create a string containing the single-character delimiter.
        std::string delimiterString;
        delimiterString = this->fileDictionaryValueDelimiter;
        const wchar_t wdelimiter = this->fileDictionaryValueDelimiter;
        /*
         * TODO. Delete this. Including the delimiter character in the
         * attributes is of minimal use (it is an internal value used during
         * parsing) AND it is often problematic. It is common for FCS files
         * to use unusual or control characters (such as a back-slash or EOL),
         * which become difficult to output well when tools print out or
         * convert attributes to JSON.
         *
        this->fileAttributes.emplace( "dictionaryValueDelimiter", delimiterString );
        */

        // Compute the number of events, which may be truncated.
        // If truncated, update the $TOT keyword for the new total
        // number of events.
        size_t numberOfEvents = this->getNumberOfEvents( );
        if ( maximumEvents < 0 )
            maximumEvents = numberOfEvents;

        bool truncated = false;
        if ( (size_t) maximumEvents < numberOfEvents )
        {
            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " <<
                    "  Limiting output to " << maximumEvents << " events\n";
            numberOfEvents = maximumEvents;
            truncated = true;

            // Make sure the dictionary has the truncated value when it
            // is built into a string of values below. This will be restored
            // before the end of this method.
            this->setDictionaryStringInternal(
                "$TOT",
                std::to_string( numberOfEvents ),
                false,      // No need to convert keyword or trim.
                false );    // No need to trim the value.
        }

        const size_t numberOfParameters  = this->getNumberOfParameters( );
        const size_t nValues = numberOfEvents * numberOfParameters;

        // Insure the $PAR keyword is uptodate with the number of parameters.
        this->setDictionaryStringInternal(
            "$PAR",
            std::to_string( numberOfParameters ),
            false,      // No need to convert keyword or trim.
            false );    // No need to trim the value.


        //
        // Add file storage keywords.
        // --------------------------
        // The dictionary may or may not already have file storage keywords,
        // but anything there may be wrong. Add or update keywords to match
        // the data about to be written.
        //
        // $BEGINANALYSIS, $ENDANALYSIS, $BEGINDATA, $ENDDATA,
        // $BEGINSTEXT, and $ENDSTEXT are handled specially below because
        // their values need to be calculated after these new keywords
        // are added and the dictionary is split among the TEXT and
        // SUPPLEMENTAL TEXT segments.
        //
        // Always list mode.
        this->setDictionaryStringInternal(
            "$MODE",
            "L",
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        // Never any more data.
        this->setDictionaryStringInternal(
            "$NEXTDATA",
            "0",
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        // Use the host's byte order.
        this->setDictionaryStringInternal(
            "$BYTEORD",
            (this->fileIsLSBF == true) ? "1,2,3,4" : "4,3,2,1",
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        // Data is either floats or doubles.
        const bool isFloat = this->eventTable->areValuesFloats( );
        this->setDictionaryStringInternal(
            "$DATATYPE",
            (isFloat == true) ? "F" : "D",
            false,      // No need to convert keyword or trim.
            false );    // No need to trim value.

        for ( size_t i = 0; i < numberOfParameters; ++i )
        {
            // Every parameter is written in 32-bits or 64-bits.
            std::string keyword = this->buildParameterKeyword( "$P", i, "B" );
            this->setDictionaryStringInternal(
                keyword,
                (isFloat == true) ? "32" : "64",
                false,      // No need to convert keyword or trim.
                false );    // No need to trim value.

            // Parameter log/linear scaling. According to the FCS
            // specification, log scaling is never allowed on floating point
            // data and this software only uses floating point data.
            //
            // SPECIAL HANDLING: In practice, FCS files do use scaling
            // on floating point values.
            //
            // If the data being saved came from a prior FCS file, then
            // there may already be a scaling keyword. Keep it. Otherwise,
            // override it. But since we do not know what scaling to use,
            // just default to no scaling.
            keyword = this->buildParameterKeyword( "$P", i, "E" );
            if ( this->inDictionary( keyword ) == false )
            {
                this->setDictionaryStringInternal(
                    keyword,
                    "0,0",
                    false,      // No need to convert keyword or trim.
                    false );    // No need to trim value.
            }

            // Parameter numeric range. According to the FCS
            // specification, a numeric range is never allowed on floating
            // point data and this software only uses floating point data.
            //
            // SPECIAL HANDLING: In practice, FCS files do use scaling and
            // so they need this range, even on floating point values.
            //
            // If the data being saved came from a prior FCS file, then
            // there may already be a range keyword. Keep it. Otherwise,
            // override it. But since we do not know what range to use,
            // just default to "1.0".
            keyword = this->buildParameterKeyword( "$P", i, "R" );
            if ( this->inDictionary( keyword ) == false )
            {
                this->setDictionaryStringInternal(
                    keyword,
                    "1.0",
                    false,      // No need to convert keyword or trim.
                    false );    // No need to trim value.
            }
        }


        //
        // Build text segment strings.
        // ---------------------------
        // The FCS file header must contain byte offsets to the start and
        // end of the TEXT segment. Unfortunately, the header is limited
        // to 8 characters for an ASCII representation of each offset.
        // The maximum number is therefore 99999999 = about 100 Kbytes. If the
        // dictionary cannot be fit into this space, it overflows into a
        // SUPPLEMENTAL TEXT segment that can be any length. The byte offsets
        // to this are in the TEXT segment in the "$BEGINSTEXT" and "$ENDSTEXT"
        // keywords.
        //
        // Many keywords have no specific length limit on their values. The
        // "$COM" keyword, for instance, holds a generic comment that could
        // be a few characters or pages of text. Therefore, we cannot know
        // immediately if the dictionary will fit entirely within the limits
        // of the TEXT segment.
        //
        // Below we define three strings containing content from the
        // dictionary:
        //
        // 1. Keywords and values that MUST be in the TEXT segment. These are
        //    strictly the file structure keywords containing byte offsets to
        //    the other segments:
        //    - "$BEGINSTEXT" and "$ENDSTEXT" to the SUPPLEMENTAL TEXT segment.
        //    - "$BEGINDATA" and "$ENDDATA" to the DATA segment.
        //    - "$BEGINANALYSIS" and "$ENDANALYSIS" to the ANALYSIS segment.
        //
        // 2. Keywords and values that we'd PREFER to be in the TEXT segment.
        //    These include all of the parameter keywords, and all required
        //    standard keywords except those that could be large, such
        //    as "$COM".
        //
        // 3. Everything else. This includes all large standard keyword values
        //    and all vendor keywords and values.
        //
        // After creating these strings, they are combined and written to the
        // file in one of these configurations:
        //
        // - If (MUST + PREFER + the rest) fits within the TEXT segment, write
        //   them all as the TEXT segment. There is no SUPPLEMENTAL TEXT
        //   segment.
        //
        // - If only (MUST + PREFER) fits within the TEXT segment, write them
        //   both as the TEXT segment. Write the rest as the SUPPLEMENTAL TEXT
        //   segment.
        //
        // - Otherwise write the MUST string to the TEXT segment. Write the
        //   PREFER string and the rest to the SUPPLEMENTAL TEXT segment.
        //
        // All of these actually end up writing the text as consecutive bytes
        // at the top of the file, just after the header. The only difference
        // is whether begin/end values are included and what those values are.
        std::string mustText;
        std::string preferText;
        std::string restText;

        const auto keywords = this->getDictionaryKeywords( );
        const size_t n = keywords.size( );

        // Loop over the keywords and add them to one of the output strings.
        for ( size_t i = 0; i < n; ++i )
        {
            const std::string keyword = keywords[i];
            std::string value = this->escapeDelimiter(
                wdelimiter, this->getDictionaryString( keyword ) );

            // Empty values are not allowed. Add at least one space.
            if ( value.empty( ) == true )
                value = " ";

            try
            {
                auto attr = FCSVocabulary::find( keyword );

                if ( attr.isStandard( ) == true &&
                    (attr.isRequired( ) == true ||
                     attr.isParameter( ) == true ||
                     attr.hasStringValue( ) == false) )
                {
                    // The keyword is standard AND it is either required, part
                    // of a parameter definition, or it has a numeric value (and
                    // is thus bounded in size). These keywords are important
                    // and worth trying to fit within the TEXT segment.
                    preferText += keyword + delimiterString + value + delimiterString;
                }
                else
                {
                    // All other keywords are not standard, not required, not
                    // part of a parameter definition, or they have a string
                    // value that could be huge (e.g. "$COM"). These keywords
                    // are a low priority to fit in the TEXT segment.
                    restText += keyword + delimiterString + value + delimiterString;
                }
            }
            catch ( const std::exception& e )
            {
                // Keyword not found in vocabulary. It is a vendor-specific
                // keyword. Add it to the lowest-priority string list.
                restText += keyword + delimiterString + value + delimiterString;
            }
        }

        if ( truncated == true )
        {
            // Restore the original dictionary total events value.
            this->setDictionaryStringInternal(
                "$TOT",
                std::to_string( this->getNumberOfEvents( ) ),
                false,      // No need to convert keyword or trim.
                false );    // No need to trim value.
        }


        //
        // Allocate strings to segments.
        // -----------------------------
        // Compute the lengths of the three strings. The MUST string must
        // contain enough space for the begin/end keywords and offsets for
        // the DATA, ANALYSIS, and SUPPLEMENTAL TEXT segments (even though
        // we might not need them all).
        //
        // The TEXT segment always starts with the one-byte delimiter,
        // which accounts for the initial "1" below.
        size_t mustLength = 1 +
            // Keyword                     delim   offset        delim
            (std::strlen( "$BEGINDATA" )     + 1 + nDigitsForOffset + 1) +
            (std::strlen( "$ENDDATA" )       + 1 + nDigitsForOffset + 1) +
            (std::strlen( "$BEGINSTEXT" )    + 1 + nDigitsForOffset + 1) +
            (std::strlen( "$ENDSTEXT" )      + 1 + nDigitsForOffset + 1) +
            (std::strlen( "$BEGINANALYSIS" ) + 1 + nDigitsForOffset + 1) +
            (std::strlen( "$ENDANALYSIS" )   + 1 + nDigitsForOffset + 1);
        const size_t preferLength = preferText.length( );
        const size_t restLength = restText.length( );

        // Compute text and supplemental text segment lengths.
        size_t textSegmentLength = 0;
        size_t supplementalTextSegmentLength = 0;

        if ( (headerLength + mustLength + preferLength + restLength) < maxHeaderOffset )
        {
            // Everything will fit within the TEXT segment, while keeping
            // byte offsets small enough to fit in the header.
            textSegmentLength = mustLength + preferLength + restLength;
            supplementalTextSegmentLength = 0;

            if ( this->verbose == true )
                std::cerr << this->verbosePrefix << ": " <<
                    "  Saving all dictionary entries in text segment.\n";
        }
        else if ( (headerLength + mustLength + preferLength) < maxHeaderOffset )
        {
            // The MUST and PREFER strings will fit in the TEXT segment, but
            // the rest overflows and will have to go into the SUPPLEMENTAL
            // TEXT segment.
            textSegmentLength = mustLength + preferLength;
            supplementalTextSegmentLength = restLength;

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " <<
                    "  Saving primary dictionary entries in text segment.\n";
                std::cerr << this->verbosePrefix << ": " <<
                    "  Saving additional dictionary entries in supplemental text segment.\n";
            }
        }
        else
        {
            // The MUST string will fit in the TEXT segment, but adding the
            // PREFER string overflows. The PREFER string and the rest will
            // have to go into the SUPPLEMENTAL TEXT segment.
            textSegmentLength = mustLength;
            supplementalTextSegmentLength = preferLength + restLength;

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " <<
                    "  Saving only essential dictionary entries in text segment.\n";
                std::cerr << this->verbosePrefix << ": " <<
                    "  Saving all other dictionary entries in supplemental text segment.\n";
            }
        }


        //
        // Compute segment begin/end.
        // --------------------------
        // The file header size is fixed and known. The assignment of text
        // strings to the TEXT and SUPPLEMENTAL TEXT segments is now known
        // and their size is known. And the DATA segment's event data size
        // is known. The begin/end file offsets can now be computed, before
        // the file is written.
        //
        // Compute the data segment length.
        const size_t dataSegmentLength = nValues *
            (this->eventTable->areValuesFloats( ) == true ?
                sizeof(float) : sizeof(double));

        // Compute the begin/end byte offsets.
        //
        // - TEXT segment begins after the header.
        //
        // - SUPPLEMENTAL TEXT segment begins after the TEXT segment.
        //
        // - DATA segment begins after the SUPPLEMENTAL TEXT segment.
        //
        // In each case the $END* value is the offset to the last value in
        // the segment, and NOT the offset to the last value plus one. This
        // causes the (-1) on each of the $END* calculations.
        const size_t textBegin = headerLength;
        const size_t textEnd   = textBegin + textSegmentLength - 1;
        size_t supplementalTextBegin = 0;
        size_t supplementalTextEnd = 0;
        size_t dataBegin = 0;
        size_t dataEnd = 0;

        if ( supplementalTextSegmentLength == 0 )
        {
            // No SUPPLEMENTAL TEXT segment. The DATA segment begins
            // immediately after the TEXT segment.
            dataBegin = textEnd + 1;
            dataEnd   = dataBegin + dataSegmentLength - 1;
        }
        else
        {
            // There is a SUPPLEMENTAL TEXT segment. It begins immediately
            // after the TEXT segment, and is followed by the DATA segment.
            supplementalTextBegin = textEnd + 1;
            supplementalTextEnd   = supplementalTextBegin + supplementalTextSegmentLength - 1;
            dataBegin = supplementalTextEnd + 1;
            dataEnd   = dataBegin + dataSegmentLength - 1;
        }

        const size_t analysisBegin = 0;     // NO ANALYSIS SEGMENT
        const size_t analysisEnd   = 0;     // NO ANALYSIS SEGMENT

        // Create the MUST keyword-value entries of the TEXT segment. These
        // provide the begin/end offsets. Because we've alread included the
        // length of this text in calculations above, we need to make sure
        // the text actually has this length!
        char numberBuffer[nDigitsForOffset+1];

        mustText += delimiterString;

        std::sprintf( numberBuffer, "%*ld", nDigitsForOffset, dataBegin );
        mustText += std::string( "$BEGINDATA" ) +
            delimiterString + numberBuffer + delimiterString;

        std::sprintf( numberBuffer, "%*ld", nDigitsForOffset, dataEnd );
        mustText += std::string( "$ENDDATA" ) +
            delimiterString + numberBuffer + delimiterString;

        std::sprintf( numberBuffer, "%*ld", nDigitsForOffset, analysisBegin );
        mustText += std::string( "$BEGINANALYSIS" ) +
            delimiterString + numberBuffer + delimiterString;

        std::sprintf( numberBuffer, "%*ld", nDigitsForOffset, analysisEnd );
        mustText += std::string( "$ENDANALYSIS" ) +
            delimiterString + numberBuffer + delimiterString;

        std::sprintf( numberBuffer, "%*ld", nDigitsForOffset, supplementalTextBegin );
        mustText += std::string( "$BEGINSTEXT" ) +
            delimiterString + numberBuffer + delimiterString;

        std::sprintf( numberBuffer, "%*ld", nDigitsForOffset, supplementalTextEnd );
        mustText += std::string( "$ENDSTEXT" ) +
            delimiterString + numberBuffer + delimiterString;


        //
        // Write the file.
        // ---------------
        // Write the header, then each of the segments.
        try
        {
            const std::string versionNumber = DEFAULT_FCS_VERSION;
            this->fileAttributes.emplace( "versionNumber", versionNumber );

            if ( this->verbose == true )
            {
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "File format version:" <<
                    "\"" << versionNumber << "\"\n";

                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Keyword value delimiter:";
                if ( isgraph(this->fileDictionaryValueDelimiter) == true )
                    std::cerr << "\"" << this->fileDictionaryValueDelimiter << "\"\n";
                else if ( this->fileDictionaryValueDelimiter == '\n' )
                    std::cerr << "line feed (CTRL-J)\n";
                else if ( this->fileDictionaryValueDelimiter == '\r' )
                    std::cerr << "carriage return (CTRL-M)\n";
                else if ( this->fileDictionaryValueDelimiter == '\t' )
                    std::cerr << "tab (CTRL-I)\n";
                else if ( this->fileDictionaryValueDelimiter == 0x0c )
                    std::cerr << "page feed (CTRL-L)\n";
                else
                {
                    std::cerr << "0x" <<
                        std::setw(2) << std::setfill('0') << std::setbase(16) <<
                        (uint32_t) this->fileDictionaryValueDelimiter <<
                        " (non-printing character)\n" <<
                        std::setbase(10) << std::setfill(' ') <<
                        std::resetiosflags( std::ios::showbase );
                }

                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Number format:" << "binary\n";
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Byte order:" <<
                    (this->fileIsLSBF == true ?
                        "Least-significant byte first\n" :
                        "Most-significant byte first\n");
                if ( this->eventTable->areValuesFloats( ) == true )
                    std::cerr << this->verbosePrefix << ": " << "  " <<
                        std::setw( 30 ) << std::left <<
                        "Data type" << "floats\n";
                else
                    std::cerr << this->verbosePrefix << ": " << "  " <<
                        std::setw( 30 ) << std::left <<
                        "Data type" << "doubles\n";
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Number of parameters:" <<
                    numberOfParameters << "\n";
                std::cerr << this->verbosePrefix << ": " << "  " <<
                    std::setw( 30 ) << std::left <<
                    "Number of events:" <<
                    numberOfEvents << "\n";
            }

            //
            // Write header.
            // -------------
            // - File format version as the first 6 bytes of a 10 byte field.
            //   The extra bytes are padded as blanks.
            // - Three pairs of 8-byte offsets in ASCII.
            //   - TEXT segment begin/end.
            //   - DATA segment begin/end.
            //   - ANALYSIS segment begin/end.
            if ( std::fprintf( fp, "%-10s", versionNumber.c_str() ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file's header: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

            if ( std::fprintf( fp, "%8ld%8ld", textBegin, textEnd ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file's header: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

            int status;
            if ( dataEnd > maxHeaderOffset )
                status = std::fprintf( fp, "%8d%8d", 0, 0 );
            else
                status = std::fprintf( fp, "%8ld%8ld", dataBegin, dataEnd );
            if ( status < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file's header: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

            if ( analysisEnd > maxHeaderOffset )
                status = std::fprintf( fp, "%8d%8d", 0, 0 );
            else
                status = std::fprintf( fp, "%8ld%8ld", analysisBegin, analysisEnd );
            if ( status < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file's header: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }


            //
            // Write TEXT and SUPPLEMENTAL TEXT segments.
            // ------------------------------------------
            // The TEXT segment always immediately follows the header. The
            // SUPPLEMENTAL TEXT segment, if any, always follows the TEXT
            // segment. The TEXT segment begins with the delimiter. The
            // SUPPLEMENTAL TEXT segment doesn't start with a delimiter,
            // though some FCS files do so.
            //
            // Since there is nothing needed between the end of the TEXT
            // segment and the start of the SUPPLEMENTAL TEXT segment, we
            // can write them all back-to-back. The begin/end offsets
            // computed earlier are all that divide the segments.
            if ( std::fwrite( mustText.c_str( ), 1, mustText.length( ), fp ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file's dictionary: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

            if ( std::fwrite( preferText.c_str( ), 1, preferText.length( ), fp ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the file's dictionary: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }
            if ( restText.empty( ) == false )
            {
                if ( std::fwrite( restText.c_str( ), 1, restText.length( ), fp ) < 0 )
                {
                    this->appendFileLog(
                        "error",
                        std::string( "The system reported an error while writing the file's dictionary: " ) +
                        std::string( std::strerror( errno ) ) + std::string( "." ) );
                    throw std::runtime_error(
                        ERROR_WRITE +
                        ERROR_CANNOTSAVE );
                }
            }


            //
            // Write DATA segment.
            // -------------------
            // The DATA segment contains the event table's values either as
            // single- or double-precision floating point.
            //
            // However, the FCS event table is ordered row-by-row, with each
            // row describing one event. Consecutive values are for consecutive
            // parameters of the same event. BUT the event table in memory
            // is instead ordered column-by-column because that is more
            // efficient for downstream processing. To write out events, then,
            // we have to transpose the columns into rows.
            if ( this->eventTable->areValuesFloats( ) == true )
            {
                if ( this->verbose == true )
                    std::cerr << this->verbosePrefix << ": " <<
                        "  Saving " << nValues << " 32-bit floats\n";

                // Get pointers to each of the parameter's arrays.
                float** parameterData = new float*[numberOfParameters];
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                    parameterData[i] = this->eventTable->getParameterFloats(i).data( );

                for ( size_t eventIndex = 0; eventIndex < numberOfEvents; ++eventIndex )
                {
                    for ( uint32_t i = 0; i < numberOfParameters; ++i )
                        status = std::fwrite( &parameterData[i][eventIndex],
                            sizeof(float), 1, fp );

                    if ( status < 0 )
                    {
                        delete[] parameterData;

                        this->appendFileLog(
                            "error",
                            std::string( "The system reported an error while writing the file event data: " ) +
                            std::string( std::strerror( errno ) ) + std::string( "." ) );
                        throw std::runtime_error(
                            ERROR_WRITE +
                            ERROR_CANNOTSAVE );
                    }
                }

                delete[] parameterData;
            }
            else
            {
                if ( this->verbose == true )
                    std::cerr << this->verbosePrefix << ": " <<
                        "  Saving " << nValues << " 64-bit doubles\n";

                // Get pointers to each of the parameter's arrays.
                double** parameterData = new double*[numberOfParameters];
                for ( uint32_t i = 0; i < numberOfParameters; ++i )
                    parameterData[i] = this->eventTable->getParameterDoubles(i).data( );

                for ( size_t eventIndex = 0; eventIndex < numberOfEvents; ++eventIndex )
                {
                    for ( uint32_t i = 0; i < numberOfParameters; ++i )
                        status = std::fwrite( &parameterData[i][eventIndex],
                            sizeof(double), 1, fp );

                    if ( status < 0 )
                    {
                        delete[] parameterData;

                        this->appendFileLog(
                            "error",
                            std::string( "The system reported an error while writing the file event data: " ) +
                            std::string( std::strerror( errno ) ) + std::string( "." ) );
                        throw std::runtime_error(
                            ERROR_WRITE +
                            ERROR_CANNOTSAVE );
                    }
                }

                delete[] parameterData;
            }

            //
            // Write CRC.
            // ----------
            // This software does not compute a "CRC" checksum. The FCS
            // specification therefore requires that 8 bytes of ASCII zeroes
            // be written in place of the CRC value.
            if ( std::fwrite( "00000000", 1, 8, fp ) < 0 )
            {
                this->appendFileLog(
                    "error",
                    std::string( "The system reported an error while writing the CRC terminator: " ) +
                    std::string( std::strerror( errno ) ) + std::string( "." ) );
                throw std::runtime_error(
                    ERROR_WRITE +
                    ERROR_CANNOTSAVE );
            }

            std::fclose( fp );
        }
        catch ( const std::exception& e )
        {
            std::fclose( fp );
            throw;
        }
    }
    // @}
};





/**
 * Returns a new initialized vocabulary map of keyword names and attributes.
 *
 * The map's keys are FCS keywords and the values are FCSKeyword objects
 * that describe the keyword, the values it expects, and other attributes.
 *
 * @return
 *   Returns a new initialized map of keyword names and attributes. It is
 *   the responsibility of the caller to save and, if appropriate, delete
 *   the returned map when it is no longer needed.
 */
std::map<std::string, FCSKeyword>* FCSVocabulary::initialize( )
    noexcept
{
    auto map = new std::map<std::string, FCSKeyword>( );

    // Define some macros for this initialization code to make the
    // code less verbose. The macros are undefined after the code.
    //
    // Specification versions:
#define _V_1_______   FCSKeyword::FCS_VERSION_1_0
#define _V___2_____   FCSKeyword::FCS_VERSION_2_0
#define _V_____3___   FCSKeyword::FCS_VERSION_3_0
#define _V_______31   FCSKeyword::FCS_VERSION_3_1
#define _V_____3_31   (_V_____3___|_V_______31)
#define _V___2_3_31   (_V___2_____|_V_____3_31)
#define _V_1_2_3_31   (_V_1_______|_V___2_3_31)

    // Flags about standard keywords:
#define _DEP   FCSKeyword::DEPRECATED
#define _REQ   FCSKeyword::REQUIRED
#define _STD   FCSKeyword::STANDARD

    // Flags about the type of information:
#define _PRM   FCSKeyword::PARAMETER
#define _GAT   FCSKeyword::GATE

    // Flags about de-identification:
#define _USR    FCSKeyword::USER_DATA
#define _PER   FCSKeyword::PERSONAL_DATA
#define _DAT   FCSKeyword::DATE_DATA

#define _MAPINSERT(KEYWORD, DESCRIPTION, TYPE, CAT, VERSION, FLAGS, OFFSET) \
map->insert( std::make_pair( std::string( KEYWORD ), FCSKeyword( \
    std::string( KEYWORD ), \
    std::string( DESCRIPTION ), \
    FCSKeyword::TYPE, \
    FCSKeywordCategory::CAT, \
    VERSION, \
    FLAGS, \
    OFFSET ) ) )

    // The map is shared across all threads. Initialization is therefore
    // a critical region that is restricted to one thread.
#pragma omp critical
    {
        map->clear( );

        //
        // File storage.
        // -------------
        _MAPINSERT( "$BEGINANALYSIS",
            "Byte offset to beginning of the file's analysis segment",
            LONG_VALUE,   FILESTORAGE,     _V_____3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$BEGINDATA",
            "Byte offset to beginning of the file's data segment",
            LONG_VALUE,   FILESTORAGE,     _V_____3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$BEGINSTEXT",
            "Byte offset to beginning of the file's supplemental text segment",
            LONG_VALUE,   FILESTORAGE,     _V_____3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$BYTEORD",
            "Byte order for binary data stored in the file",
            MULTI_VALUE,  FILESTORAGE,     _V___2_3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$DATATYPE",
            "Data type (float, double, integer) for data stored in the file",
            STRING_VALUE, FILESTORAGE,     _V___2_3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$ENDANALYSIS",
            "Byte offset to ending of the file's analysis segment",
            LONG_VALUE,   FILESTORAGE,     _V_____3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$ENDDATA",
            "Byte offset to ending of the file's data segment",
            LONG_VALUE,   FILESTORAGE,     _V_____3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$ENDSTEXT",
            "Byte offset to ending of the file's supplemental text segment",
            LONG_VALUE,   FILESTORAGE,     _V_____3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$MODE",
            "Storage mode for the file's data",
            STRING_VALUE, FILESTORAGE,     _V_1_2_3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$NEXTDATA",
            "Byte offset to the next data set in the file",
            LONG_VALUE,   FILESTORAGE,     _V___2_3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$PnB",
            "Bits used to store parameter 'n' in the file",
            LONG_VALUE,   FILESTORAGE,     _V___2_3_31, _STD|_REQ|_PRM, 2 );
        _MAPINSERT( "$ASC",
            "ASCII storage specification for data stored in the file",
            STRING_VALUE, FILESTORAGE,     _V_1_______, _STD|_DEP, 0 );
        _MAPINSERT( "$UNICODE",
            "UNICODE usage indicator for selected keyword string values",
            MULTI_VALUE,  FILESTORAGE,     _V_____3___, _STD|_DEP, 0 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "CHARSET",
            "Character set for keyword string values",
            STRING_VALUE, FILESTORAGE,               0, 0, 0 );

        //
        // General documentation.
        // ----------------------
        _MAPINSERT( "$COM",
            "General-purpose comment",
            STRING_VALUE, DOCGENERAL,      _V_1_2_3_31, _STD|_PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.
        _MAPINSERT( "$PROJ",
            "Project name",
            STRING_VALUE, DOCGENERAL,      _V_1_2_3_31, _STD|_PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.

        // Vendor: Becton-Dickenson
        _MAPINSERT( "EXPERIMENT NAME",
            "Experiment name",
            STRING_VALUE, DOCGENERAL,                0, _PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.
        _MAPINSERT( "KEYWORD 1",
            "General-purpose keyword",
            STRING_VALUE, DOCGENERAL,                0, _PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.
        _MAPINSERT( "KEYWORD 2",
            "General-purpose keyword",
            STRING_VALUE, DOCGENERAL,                0, _PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.

        // Vendor: FlowJo.
        _MAPINSERT( "EXPERIMENT_NAME",
            "Experiment name",
            STRING_VALUE, DOCGENERAL,                0, _PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.
        _MAPINSERT( "SF_EXPERIMENT_UID",
            "Experiment user ID",
            STRING_VALUE, DOCGENERAL,                0, _PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.

        // Vendor: Mitenyl Biotec.
        _MAPINSERT( "@MB_EXPERIMENT",
            "MACSQuantifySoftware experiment information (base64 encoded)",
            STRING_VALUE, DOCGENERAL,                0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is large and vendor-defined.
            // Without a public definition from the vendor, we must assume
            // that it may contain personal health information, user
            // information, or date information.

        // Vendor: Unknown.
        _MAPINSERT( "$COMMENT",
            "General-purpose comment",
            STRING_VALUE, DOCGENERAL,      _V_1_2_3_31, _STD|_PER|_USR, 0 );
            // The value for this keyword is not well-defined. We must
            // assume that it may contain personal health information or
            // user information.

        //
        // Device documentation.
        // ---------------------
        _MAPINSERT( "$CYT",
            "Flow cytometer device name",
            STRING_VALUE, DOCDEVICE,       _V_1_2_3_31, _STD, 0 );
        _MAPINSERT( "$CYTSN",
            "Flow cytometer device serial number",
            STRING_VALUE, DOCDEVICE,       _V_____3_31, _STD, 0 );
        _MAPINSERT( "$SYS",
            "Computer type and operating system",
            STRING_VALUE, DOCDEVICE,       _V_____3_31, _STD, 0 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "APPLICATION",
            "Software name and version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "BD$ACQLIBVERSION",
            "Acquisition library software version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CALIBFILE",
            "Calibration file used",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTNUM",
            "Flow cytometer device serial number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIG NAME",
            "Flow cytometer configuration file name",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIG DATE CREATED",
            "Flow cytometer configuration file creation date",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIG CREATE DATE",
            "Flow cytometer configuration file creation date",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIG DATE MODIFIED",
            "Flow cytometer configuration file modification date",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIGURATION NAME",
            "Flow cytometer configuration file name",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIGURATION DATE CREATED",
            "Flow cytometer configuration file creation date",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CYTOMETER CONFIGURATION DATE MODIFIED",
            "Flow cytometer configuration file modification date",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "FIRMWAREVERSION",
            "Flow cytometer firmware version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "UTOPEXBUILD",
            "Build version",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "WINDOW EXTENSION",
            "Microsoft Windows operating system version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "WORKSTATION",
            "Computer type and operating system",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );

        // Vendor: Becton-Dickenson, Cytek
        _MAPINSERT( "CREATOR",
            "Software name and version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "ACQ. SOFTWARE",
            "Software name",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );

        // Vendor: Beckman Coulter
        _MAPINSERT( "SOFTWAREREVISION",
            "Software version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );

        // Vendor: FlowJo
        _MAPINSERT( "FJ_FCS_VERSION",
            "FCS version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "SOFTWARE",
            "Software name and version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );

        // Vendor: Mitenyl Biotec.
        _MAPINSERT( "@MB_INSSET",
            "MACSQuantify Software instrument settings (base64 encoded)",
            STRING_VALUE, DOCDEVICE,                 0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is large and vendor-defined.
            // Without a public definition from the vendor, we must assume
            // that it may contain personal health information, user
            // information, or date information.

        // Vendor: Verity software house
        _MAPINSERT( "CONNECTION",
            "WinList connection type to Sony cytometer",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "CREATED_BY",
            "WinList software name and version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "HAPS_MODULE",
            "Sony flow cytometer device name",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "RESAVED_BY",
            "WinList software name and version number on a resave",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "WINLIST_BUILD_DATE",
            "WinList software build date",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "WINLIST_CONNECTION_SETTINGS",
            "WinList software connection settings",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "VSH_APP",
            "Verity Software House software name",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "VSH_APP_VERSION",
            "Verity Software House software version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );

        // Vendor: unknown
        _MAPINSERT( "FCSVERSION",
            "FCS version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "UTOPEXBUILD",
            "UTOPEX software version number",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );
        _MAPINSERT( "SETTINGS",
            "Settings name",
            STRING_VALUE, DOCDEVICE,                 0, 0, 0 );

        //
        // Acquisition source.
        // -------------------
        _MAPINSERT( "$CELLS",
            "Type of cells measured",
            STRING_VALUE, DOCSOURCE,       _V_1_2_3_31, _STD, 0 );
        _MAPINSERT( "$FIL",
            "File name the data was saved into",
            STRING_VALUE, DOCSOURCE,       _V_1_2_3_31, _STD|_PER|_USR|_DAT, 0 );
            // The value for this keyword is a file name. Some institutions
            // encode the patient name or ID within the file name. We must
            // assume that this practice is in use and that the file name
            // is therefore personal health information. The file name also
            // could include the institution name, department name, or date,
            // which also makes the name possibly contain user and date
            // information.
        _MAPINSERT( "$PLATEID",
            "Plate identifier",
            STRING_VALUE, DOCSOURCE,       _V_______31, _STD, 0 );
        _MAPINSERT( "$PLATENAME",
            "Plate name",
            STRING_VALUE, DOCSOURCE,       _V_______31, _STD, 0 );
        _MAPINSERT( "$SMNO",
            "Specimen, sample, or tube label",
            STRING_VALUE, DOCSOURCE,       _V_1_2_3_31, _STD|_PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "$SRC",
            "Specimen source name, such as a patient name",
            STRING_VALUE, DOCSOURCE,       _V_1_2_3_31, _STD|_PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "$VOL",
            "Volume of sample run during data acquisition",
            DOUBLE_VALUE, DOCSOURCE,       _V_______31, _STD, 0 );
        _MAPINSERT( "$WELLID",
            "Well identifier",
            STRING_VALUE, DOCSOURCE,       _V_______31, _STD, 0 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "ACQUIRED_CLINICAL",
            "Whether the data was acquired for clinical use",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "ASSAY ID",
            "Assay identifier",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "CASE NUMBER",
            "Case or patient number",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.
        _MAPINSERT( "END BATCH",
            "Whether this file is the last in a series for a single patient",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "FILE GUID",
            "File unique identifier",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.
        _MAPINSERT( "GUID",
            "File unique identifier",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.
        _MAPINSERT( "PATIENT ID",
            "Patient identifier",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is a patient ID, which is personal
            // health information.
        _MAPINSERT( "PLATE ID",
            "Plate identifier",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "PLATE NAME",
            "Plate name",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "PANEL NAME",
            "Panel name under which the tube was acquired",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "PREP",
            "Sample preparation method",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "SAMPLE NAME",
            "Sample name",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "SOURCE TUBE NAME",
            "Source tube name",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "START BATCH",
            "Whether this file is the first in a series for a single patient",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "TUBE NAME",
            "Tube name",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "TUBE SETTINGS ID",
            "Tube settings identifier",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "TUBE SETTINGS NAME",
            "Tube settings name",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "VOL",
            "Volume of sample run during data acquisition",
            STRING_VALUE, DOCSOURCE,                 0,    0, 0 );
        _MAPINSERT( "WELL ID",
            "Well identifier",
            STRING_VALUE, DOCSOURCE,                 0, _STD, 0 );

        // Vendor: Becton-Dickenson, Verity software house
        _MAPINSERT( "SAMPLE ID",
            "Sample identifier",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.

        // Vendor: Verity software house
        _MAPINSERT( "RUNID",
            "Run identifier",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.
        _MAPINSERT( "ORIGINAL_$FIL",
            "Original file name the data was saved into, prior to processing",
            STRING_VALUE, DOCSOURCE,                 0, _PER|_USR, 0 );

        // Vendor: FlowJo
        _MAPINSERT( "SAMPLE_NAME",
            "Sample name",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "TUBE_NAME",
            "Tube name",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.

        // Vendor: Beckman Coulter
        _MAPINSERT( "SAMPLEID",
            "Sample identifier",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.

        // Vendor: Mitenyl Biotec.
        _MAPINSERT( "@MB_SESSIONID",
            "MACSQuantify Software unique ID of the acquired file",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID, but the structure of the
            // ID is not well-defined. We must assume that it may contain a
            // patient ID, which is personal health information.

        // Vendor: Unknown.
        _MAPINSERT( "FILENAME",
            "File name the data was saved into",
            STRING_VALUE, DOCSOURCE,                 0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is a file name. Some institutions
            // encode the patient name or ID within the file name. We must
            // assume that this practice is in use and that the file name
            // is therefore personal health information. The file name also
            // could include the institution name, department name, or date,
            // which also makes the name possibly contain user and date
            // information.
        _MAPINSERT( "MANUFACTURER",
            "Manufacturer name",
            STRING_VALUE, DOCSOURCE,                 0, 0, 0 );
        _MAPINSERT( "#SAMPLE",
            "Sample name",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 0 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.
        _MAPINSERT( "#SPACERS",
            "Number of spacers",
            STRING_VALUE, DOCSOURCE,                 0, 0, 0 );
        _MAPINSERT( "@SAMPLEIDn",
            "Sample ID",
            STRING_VALUE, DOCSOURCE,                 0, _PER, 9 );
            // The value for this keyword is an ID or name, but the structure
            // of the value is not well-defined. We must assume that it may
            // contain a patient ID, which is personal health information.

        //
        // Acquisition history.
        // --------------------
        _MAPINSERT( "$BTIM",
            "Time for beginning of data acquisition",
            STRING_VALUE, DOCHISTORY,      _V_1_2_3_31, _STD|_DAT, 0 );
        _MAPINSERT( "$DATE",
            "Date for beginning of data acquisition",
            STRING_VALUE, DOCHISTORY,      _V_1_2_3_31, _STD|_DAT, 0 );
        _MAPINSERT( "$ETIM",
            "Time for ending of data acquisition",
            STRING_VALUE, DOCHISTORY,      _V_1_2_3_31, _STD|_DAT, 0 );
        _MAPINSERT( "$EXP",
            "Investigator name for experiment",
            STRING_VALUE, DOCHISTORY,      _V_1_2_3_31, _STD|_USR, 0 );
        _MAPINSERT( "$INST",
            "Institution name at which data was acquired",
            STRING_VALUE, DOCHISTORY,      _V_1_2_3_31, _STD|_USR, 0 );
        _MAPINSERT( "$LAST_MODIFIED",
            "Date and time of last data modification",
            STRING_VALUE, DOCHISTORY,      _V_______31, _STD|_DAT, 0 );
        _MAPINSERT( "$LAST_MODIFIER",
            "Investigator name who last modified the data",
            STRING_VALUE, DOCHISTORY,      _V_______31, _STD|_USR, 0 );
        _MAPINSERT( "$OP",
            "Operator name for flow cytometer during data acquisition",
            STRING_VALUE, DOCHISTORY,      _V_1_2_3_31, _STD|_USR, 0 );
        _MAPINSERT( "$ORIGINALITY",
            "Whether the data has been modified since it was acquired",
            STRING_VALUE, DOCHISTORY,      _V_______31, _STD, 0 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "ADDRESS1",
            "First line of address for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "ADDRESS2",
            "Second line of address for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "ADDRESS3",
            "Third line of address for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "ADDRESS4",
            "Fourth line of address for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "DEPARTMENT",
            "Department name at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "DEPARTMENT FAX",
            "Department FAX telephone number for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "DEPARTMENT PHONE",
            "Department telephone number for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "DIRECTOR",
            "Director name for department at which data was acquired",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "EXPORT TIME",
            "Date and time at which acquired data was exported to a file",
            STRING_VALUE, DOCHISTORY,                0, _DAT, 0 );
        _MAPINSERT( "EXPORT USER NAME",
            "User name who exported acquired data to the file",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "OPERATOR EMAIL",
            "Email address of the operator of the flow cytometer",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );
        _MAPINSERT( "LOGIN NAME",
            "The user's login name",
            STRING_VALUE, DOCHISTORY,                0, _USR, 0 );

        // Vendor: Unknown
        _MAPINSERT( "#ACQUISITIONTIMEMILLI",
            "Time for beginning of data acquisition milliseconds",
            LONG_VALUE,   DOCHISTORY,                0, _DAT, 0 );
        _MAPINSERT( "#ATIM",
            "Time for beginning of data acquisition milliseconds",
            LONG_VALUE,   DOCHISTORY,                0, _DAT, 0 );
        _MAPINSERT( "#CFLOWCAPTUREDDATE",
            "Cytometry flow capture date in seconds since the epoch",
            LONG_VALUE,   DOCHISTORY,                0, _DAT, 0 );

        //
        // Acquisition.
        // ------------
        _MAPINSERT( "$ABRT",
            "Number of events lost due to data acquisition electronic coincidence",
            LONG_VALUE,   ACQUISITION,     _V_1_2_3_31, _STD, 0 );
        _MAPINSERT( "$LOST",
            "Number of events lost due to computer busy",
            LONG_VALUE,   ACQUISITION,     _V_1_2_3_31, _STD, 0 );
        _MAPINSERT( "$PAR",
            "Number of parameters per event",
            LONG_VALUE,   ACQUISITION,     _V_1_2_3_31, _STD|_REQ|_PRM, 0 );
        _MAPINSERT( "$PnCALIBRATION",
            "Parameter value scaling to convert to well-defined units",
            MULTI_VALUE,  ACQUISITION,     _V_______31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnE",
            "Parameter value log scaling and offset",
            MULTI_VALUE,  ACQUISITION,     _V___2_3_31, _STD|_REQ|_PRM, 2 );
        _MAPINSERT( "$PnF",
            "Parameter detector's optical filter name",
            STRING_VALUE, ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnG",
            "Parameter value detector's linear amplifier gain",
            DOUBLE_VALUE, ACQUISITION,     _V_____3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnL",
            "Parameter value detector's excitation wavelengths",
            MULTI_VALUE,  ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnN",
            "Parameter short name",
            STRING_VALUE, ACQUISITION,     _V_1_2_3_31, _STD|_REQ|_PRM, 2 );
        _MAPINSERT( "$PnO",
            "Parameter detector's excitation power",
            LONG_VALUE,   ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnP",
            "Parameter value detector's percentage of emitted light",
            LONG_VALUE,   ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnR",
            "Parameter value numeric range",
            LONG_VALUE,   ACQUISITION,     _V___2_3_31, _STD|_REQ|_PRM, 2 );
        _MAPINSERT( "$PnS",
            "Parameter long name",
            STRING_VALUE, ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnT",
            "Parameter detector type",
            STRING_VALUE, ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$PnV",
            "Parameter detector voltage",
            DOUBLE_VALUE, ACQUISITION,     _V_1_2_3_31, _STD|_PRM, 2 );
        _MAPINSERT( "$TIMESTEP",
            "Time step as a fraction of a second",
            DOUBLE_VALUE, ACQUISITION,     _V_____3_31, _STD, 0 );
        _MAPINSERT( "$TOT",
            "Number of events acquired",
            LONG_VALUE,   ACQUISITION,     _V_1_2_3_31, _STD|_REQ, 0 );
        _MAPINSERT( "$TR",
            "Trigger parameter name and threshold",
            MULTI_VALUE,  ACQUISITION,     _V_____3_31, _STD, 0 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "AUTOBS",
            "Whether automatic biexponential scaling was enabled on export",
            STRING_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "BD$NPAR",
            "Number of parameters per event",
            LONG_VALUE,   ACQUISITION,               0, _PRM, 0 );
        _MAPINSERT( "BD$LASERMODE",
            "Laser mode",
            LONG_VALUE,   ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "BD$PnN",
            "Parameter name",
            STRING_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "FSC ASF",
            "Forward scatterring area scaling factor",
            DOUBLE_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "FLUIDICS MODE",
            "Normal or high sensititivey for fluidics during acquisition",
            STRING_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "LASERn",
            "Laser number",
            LONG_VALUE, ACQUISITION,                 0, 0, 5 );
        _MAPINSERT( "LASERnASF",
            "Laser area scaling factor",
            DOUBLE_VALUE, ACQUISITION,               0, 0, 5 );
        _MAPINSERT( "LASERnPOWER",
            "Laser power",
            DOUBLE_VALUE, ACQUISITION,               0, 0, 5 );
        _MAPINSERT( "LASERnWAVELENGTH",
            "Laser wavelength",
            LONG_VALUE,   ACQUISITION,               0, 0, 5 );
        _MAPINSERT( "NOZZLEDIAMETER",
            "Nozzle diameter",
            DOUBLE_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "PnBS",
            "Parameter R-value for biexponential scaling",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnCHANNELTYPE",
            "Parameter channel type",
            LONG_VALUE,   ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnLASER",
            "Parameter laer number",
            LONG_VALUE,   ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnLOTID",
            "Parameter tube reagent lot ID",
            STRING_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnMS",
            "Parameter manual R-values for scaling",
            MULTI_VALUE,  ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "REAGENTNAMEn",
            "Reagent name",
            STRING_VALUE, ACQUISITION,               0, 0, 11 );
        _MAPINSERT( "REAGENTLOTIDn",
            "Reagent lot ID",
            STRING_VALUE, ACQUISITION,               0, 0, 12 );
        _MAPINSERT( "SHEATHPRESSURE",
            "Sheath pressure",
            DOUBLE_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "THRESHOLD",
            "Per-parameter thresholds during acquisition",
            MULTI_VALUE,  ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "TIMETICKS",
            "Time step as a number of milliseconds",
            LONG_VALUE,   ACQUISITION,               0, 0, 0 );

        // Vendor: Becton-Dickenson, Beckman Coulter
        _MAPINSERT( "LASERnDELAY",
            "Laser delay",
            DOUBLE_VALUE, ACQUISITION,               0, 0, 5 );

        // Vendor: Becton-Dickenson, Cytek, FlowJo
        _MAPINSERT( "LASERnNAME",
            "Laser name",
            STRING_VALUE, ACQUISITION,               0, 0, 5 );

        // Vendor: Cytek
        _MAPINSERT( "CYTEKPnG",
            "Parameter value detector's linear amplifier gain",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 6 );

        // Vendor: FlowJo
        _MAPINSERT( "FJ_$TIMESTEP",
            "FlowJo time step as a fraction of a second",
            STRING_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "FJ$ACQSTATE",
            "FlowJo acquisition state",
            STRING_VALUE, ACQUISITION,               0, 0, 0 );

        // Vendor: Verity software house
        _MAPINSERT( "DGEN_TOTAL",
            "Number of events",
            LONG_VALUE,   ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "PnALIAS",
            "Parameter value name alias",
            STRING_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnID",
            "Parameter value unique ID",
            LONG_VALUE,   ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnLOWRANGE",
            "Parameter value lowest value in dynamic range",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnHIGHRANGE",
            "Parameter value highest value in dynamic range",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );

        // Vendor: FlowJo.
        _MAPINSERT( "FJ_$PnR",
            "Parameter value range?",
            LONG_VALUE,   ACQUISITION,               0, _PRM, 5 );
        _MAPINSERT( "PnDETECTORNAME",
            "Parameter's detector name",
            STRING_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnLO",
            "Parameter value lowest value in dynamic range",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnHI",
            "Parameter value highest value in dynamic range",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "PnTYPE",
            "Parameter value scaling type",
            STRING_VALUE, ACQUISITION,               0, _PRM, 1 );

        // Vendor: Mitenyl Biotec.
        _MAPINSERT( "@MB_Pn_BASE",
            "MACSQuantify Software parameter basis information",
            MULTI_VALUE,  ACQUISITION,               0, _PRM, 5 );
        _MAPINSERT( "@MB_Pn_RANGE",
            "MACSQuantify Software parameter range",
            MULTI_VALUE,  ACQUISITION,               0, _PRM, 5 );
        _MAPINSERT( "@MB_Pn_SCALE",
            "MACSQuantify Software parameter scale",
            MULTI_VALUE,  ACQUISITION,               0, _PRM, 5 );
        _MAPINSERT( "@MB_Pn_USERNAME",
            "MACSQuantify Software parameter name chosen by user",
            STRING_VALUE, ACQUISITION,               0, _PRM, 5 );
        _MAPINSERT( "@MB_Pn_VIEW",
            "MACSQuantify Software parameter view?",
            MULTI_VALUE,  ACQUISITION,               0, _PRM, 5 );

        // Vendor: Unknown.
        _MAPINSERT( "#LASERCONFIGURATION",
            "Laser configuration",
            STRING_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "#CFLOWnCOLORCOMP",
            "Cytometery flow color",
            MULTI_VALUE,  ACQUISITION,               0, _PRM, 6 );
        _MAPINSERT( "#CFLOWDECADESn",
            "Cytometery flow scaling decades",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 13 );
        _MAPINSERT( "#PnMAXUSEFULDATACHANNEL",
            "Parameter maximum useful data channel value",
            LONG_VALUE,   ACQUISITION,               0, _PRM, 2 );
        _MAPINSERT( "#PnVIRTUALGAIN",
            "Parameter virtual gain",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 2 );
        _MAPINSERT( "VOLTAGECHANGED",
            "Voltage changed",
            LONG_VALUE,   ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "@MCAnDATAMAX",
            "Parameter data maximum",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "@MCAnDATAMIN",
            "Parameter data minimum",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "@MCAnSCALEMAX",
            "Parameter scale maximum",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "@MCAnSCALEMIN",
            "Parameter scale minimum",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "@MCAnVIEWMAX",
            "Parameter view maximum",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "@MCAnVIEWMIN",
            "Parameter view minimum",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 4 );
        _MAPINSERT( "@MCAACQMODE",
            "Acquisition mode?",
            STRING_VALUE, ACQUISITION,               0, 0, 0 );
        _MAPINSERT( "QnI",
            "Parameter I?",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "QnS",
            "Parameter S?",
            DOUBLE_VALUE, ACQUISITION,               0, _PRM, 1 );
        _MAPINSERT( "QnU",
            "Parameter S?",
            STRING_VALUE, ACQUISITION,               0, _PRM, 1 );

        //
        // Presentation.
        // -------------
        _MAPINSERT( "$PnD",
            "Parameter value scaling preference for data display",
            MULTI_VALUE,  PRESENTATION,    _V_______31, _STD|_PRM, 2 );

        // Vendor: Becton-Dickenson, Cytek, Verity software house, FlowJo
        _MAPINSERT( "PnDISPLAY",
            "Parameter value scaling preference for data display",
            STRING_VALUE, PRESENTATION,              0, _PRM, 1 );

        // Vendor: Verity software house
        _MAPINSERT( "PnXFORM",
            "Parameter value log scaling decades preference for data display",
            STRING_VALUE, PRESENTATION,              0, _PRM, 1 );

        //
        // Compensation.
        // -------------
        _MAPINSERT( "$COMP",
            "Fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,    _V_____3___, _STD|_DEP, 0 );
        _MAPINSERT( "$DFCnTOn",
            "Compensation percentage used between two parameters",
            LONG_VALUE,   COMPENSATION,    _V___2_____, _DEP, 4 );
        _MAPINSERT( "$DFCmn",
            "Compensation percentage used between two parameters",
            STRING_VALUE, COMPENSATION,    _V_1_______, _DEP, 4 );
        _MAPINSERT( "$SPILLOVER",
            "Fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,    _V_______31, _STD, 0 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "APPLY COMPENSATION",
            "Whether software should apply compensation",
            STRING_VALUE, COMPENSATION,              0, 0, 0 );

        // Vendor: Becton-Dickenson, Verity software house, FlowJo
        _MAPINSERT( "SPILL",
            "Fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );

        // Vendor: Verity software house
        _MAPINSERT( "SPILL_ORIGINAL",
            "Original file fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );
        _MAPINSERT( "$SPILLOVER_ORIGINAL",
            "Original file fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );
        _MAPINSERT( "ORIGINAL_$SPILLOVER",
            "Original file fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );

        // Vendor: Cytek
        _MAPINSERT( "ANALOG_COMP",
            "Fluorescence compensation matrix for analog cytometers with hardware compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );

        // Vendor: FlowJo
        _MAPINSERT( "DET_SPILL",
            "Fluorescence spillover matrix determinant? for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );

        // Vendor: Unknown (mentioned in FlowCore R documentation)
        _MAPINSERT( "$SPILL",
            "Fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );
        _MAPINSERT( "SPILLOVER",
            "Fluorescence spillover matrix for compensation",
            MULTI_VALUE,  COMPENSATION,              0, 0, 0 );


        //
        // Gating.
        // --------
        _MAPINSERT( "$GATE",
            "Number of gating parameters",
            LONG_VALUE,   GATING,          _V___2_3_31, 0, 0 );
        _MAPINSERT( "$GATING",
            "Region combinations used for gating",
            STRING_VALUE, GATING,          _V___2_3_31, 0, 0 );
        _MAPINSERT( "$RnI",
            "Gating parameter region",
            MULTI_VALUE,  GATING,          _V___2_3_31, _GAT, 2 );
        _MAPINSERT( "$RnW",
            "Gating parameter window",
            MULTI_VALUE,  GATING,          _V___2_3_31, _GAT, 2 );
        _MAPINSERT( "$GnE",
            "Gating parameter value amplification type",
            STRING_VALUE, GATING,          _V___2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnF",
            "Gating parameter value detector's optical filter",
            MULTI_VALUE,  GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnN",
            "Gating parameter long name",
            STRING_VALUE, GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnP",
            "Gating parameter value percent of emitted light",
            LONG_VALUE,   GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnR",
            "Gating parameter value range",
            LONG_VALUE,   GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnS",
            "Gating parameter short name",
            STRING_VALUE, GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnT",
            "Gating parameter detector type",
            STRING_VALUE, GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GnV",
            "Gating parameter detector voltage",
            LONG_VALUE,   GATING,          _V_1_2_3_31, _DEP|_GAT, 2 );
        _MAPINSERT( "$GmGnW",
            "Gating window",
            LONG_VALUE,   GATING,          _V_1_______, _DEP|_GAT, 2 );

        // Vendor: Becton-Dickenson
        _MAPINSERT( "EXPORT GATE",
            "Gating population name",
            STRING_VALUE, GATING,                    0, _GAT, 0 );

        //
        // Cell subsets.
        // -------------
        _MAPINSERT( "$CSDATE",
            "Cell subset anlaysis date",
            STRING_VALUE, CELLSUBSET,      _V_____3_31, _STD|_DAT, 0 );
        _MAPINSERT( "$CSDEFFILE",
            "Cell subset definition file name",
            STRING_VALUE, CELLSUBSET,      _V_____3_31, _STD, 0 );
        _MAPINSERT( "$CSEXP",
            "Name of person who performed the cell subset analysis",
            STRING_VALUE, CELLSUBSET,      _V_____3_31, _STD|_USR, 0 );
        _MAPINSERT( "$CSMODE",
            "Cell subset mode",
            LONG_VALUE,   CELLSUBSET,      _V_____3_31, _STD, 0 );
        _MAPINSERT( "$CSnNAME",
            "Cell subset name",
            STRING_VALUE, CELLSUBSET,      _V_____3_31, _STD, 3 );
        _MAPINSERT( "$CSnNUM",
            "Number of cells in cell subset",
            LONG_VALUE,   CELLSUBSET,      _V_____3_31, _STD, 3 );
        _MAPINSERT( "$CSVBITS",
            "Number of bits used to encode a cell subset identifier",
            LONG_VALUE,   CELLSUBSET,      _V_____3_31, _STD, 0 );
        _MAPINSERT( "$CSVnFLAG",
            "Cell subset flag bit set",
            LONG_VALUE,   CELLSUBSET,      _V_____3_31, _STD, 4 );

        //
        // Histograms.
        // -----------
        _MAPINSERT( "$PKn",
            "Peak channel number of univariate histogram",
            LONG_VALUE,   HISTOGRAM,       _V_1_2_3_31, _DEP, 3 );
        _MAPINSERT( "$PKNn",
            "Count in peak channel of univariate histogram",
            LONG_VALUE,   HISTOGRAM,       _V_1_2_3_31, _DEP, 4 );

        //
        // Other.
        // ------
        // These keywords have been seen in FCS files, but their use
        // is not known.
        //
        // Vendor: Mitenyl Biotec.
        _MAPINSERT( "@MB_ANALYSIS",
            "MACSQuantify Software analysis template (base64 encoded)",
            STRING_VALUE, OTHER,                     0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is large and vendor-defined.
            // Without a public definition from the vendor, we must assume
            // that it may contain personal health information, user
            // information, or date information.
        _MAPINSERT( "@MB_EXTENSIONS",
            "MACSQuantify Software extensions (base64 encoded)",
            STRING_VALUE, OTHER,                     0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is large and vendor-defined.
            // Without a public definition from the vendor, we must assume
            // that it may contain personal health information, user
            // information, or date information.
        _MAPINSERT( "@MB_GRPINFO",
            "MACSQuantify Software group information (base64 encoded)",
            STRING_VALUE, OTHER,                     0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is large and vendor-defined.
            // Without a public definition from the vendor, we must assume
            // that it may contain personal health information, user
            // information, or date information.
        _MAPINSERT( "@MB_VIEWPAGE",
            "MACSQuantify Software easy mode script information (base64 encoded)",
            STRING_VALUE, OTHER,                     0, _PER|_USR|_DAT, 0 );
            // The value for this keyword is large and vendor-defined.
            // Without a public definition from the vendor, we must assume
            // that it may contain personal health information, user
            // information, or date information.

        // Vendor: Unknown.
        _MAPINSERT( "@ABSSCALFACTOR",
            "Unknown",
            DOUBLE_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "CST BASELINE DATE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "CST BEADS LOT ID",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "CST PERFORMANCE EXPIRED",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "CST REGULATORY STATUS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "CST SETUP DATE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "CST SETUP STATUS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );

        _MAPINSERT( "GTI$ACCESSRIGHTS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$ANALYSISDATE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "GTI$ANALYSISSETUPBEGIN",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "GTI$ANALYSISSETUPEND",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "GTI$ANALYSISTIME",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "GTI$AREASCALE",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$ASSAYTYPE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$BEGINLOG",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "GTI$CHANnTOPARAM",
            "Unknown",
            MULTI_VALUE,  OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$CHANNELnBASELINE",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 11 );
        _MAPINSERT( "GTI$CHANNELnFINEGAIN",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 11 );
        _MAPINSERT( "GTI$CHANNELFORAREAWIDTH",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$COMPENSATIONPMnPMn",
            "Unknown",
            DOUBLE_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$DILUTIONFACTOR",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$ENDLOG",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "GTI$ERRORCOUNT",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$FLOWRATECAL",
            "Unknown",
            DOUBLE_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$FULLUSERNAME",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _USR, 0 );
        _MAPINSERT( "GTI$GnACT",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$GnCOL",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$GnDEF",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$GnS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$HIGHCONCTRIGGER",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$INSTRUMENTTYPE",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$NGATES",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$NREGIONS",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$NUMPLOTS",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$ORIGINALRUNGUID",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _PER, 0 );
        _MAPINSERT( "GTI$ORIGINALVOLUME",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$PARAMnTYPE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 9 );
        _MAPINSERT( "GTI$PLOTnCSVSETTING",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnDOTPLOTMARKER1CSVSETTING",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnDOTPLOTMARKER1GLOBAL",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnDOTPLOTMARKER1NAME",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnDOTPLOTMARKER1TYPE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnDOTPLOTXPARAM",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnDOTPLOTYPARAM",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnHISTOOVERLAP",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnHISTOPARAM",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnNUMDOTPLOTMARKERS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnNUMDOTPLOTOVERLAY",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnNUMHISTOMARKERS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnNUMHISTOOVERLAY",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnPERCENTDOTSTODISPLAY",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnSHOWOVERLAY",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnSHOWSTAT",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnSTATWINDOWPOS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnTYPE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnZOOMSTATE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PLOTnCSVSETTING",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 8 );
        _MAPINSERT( "GTI$PMTnUSEHIGHVOLTAGE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 7 );
        _MAPINSERT( "GTI$PUMPSAMPLESPEED",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$QUANTITY",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$RnACT",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$RnCOL",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$RnDEF",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$RnS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$RnTYP",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 5 );
        _MAPINSERT( "GTI$REFRESHRATE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$RUNGUID",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _PER, 0 );
        _MAPINSERT( "GTI$SAMPLEID",
            "Unknown",
            STRING_VALUE, OTHER,                     0, _PER, 0 );
        _MAPINSERT( "GTI$SPILLOVERUUID",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$SUBREGIONLIST",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$TERMINATIONCOUNT",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$TIMESCALE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$TOTALNUMREADINGS",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$WELL",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "GTI$WIDTHSCALE",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );

        _MAPINSERT( "NUMSORTWAYS",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "SETIM",
            "Time?",
            STRING_VALUE, OTHER,                     0, _DAT, 0 );
        _MAPINSERT( "TEMPELECTRONICS",
            "Unknown",
            DOUBLE_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "TEMPOPTICS",
            "Unknown",
            DOUBLE_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "WIDTHPARAMUPSHIFT",
            "Unknown",
            LONG_VALUE,   OTHER,                     0, 0, 0 );
        _MAPINSERT( "@MCAANALYSISSET",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
        _MAPINSERT( "@MCAINSTSET",
            "Unknown",
            STRING_VALUE, OTHER,                     0, 0, 0 );
    }

#undef _MAPINSERT

#undef _DEP
#undef _GAT
#undef _USR
#undef _PER
#undef _PRM
#undef _REQ
#undef _STD

#undef _V_1_______
#undef _V___2_____
#undef _V_____3___
#undef _V_______31
#undef _V_____3_31
#undef _V___2_3_31
#undef _V_1_2_3_31

    return map;
}

} // End File namespace
} // End FlowGate namespace

#undef FLOWGATE_FILEFCS_USE_FLOCKFILE
#undef FLOWGATE_FILEFCS_USE_BSWAP
