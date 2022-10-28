/**
 * @file
 * Manages a list of gate tree roots and their associated data.
 *
 * This software was developed for the J. Craig Venter Institute (JCVI)
 * in partnership with the San Diego Supercomputer Center (SDSC) at the
 * University of California at San Diego (UCSD).
 *
 * Dependencies:
 * @li C++17
 *
 * @todo Implement "fast" logicle.
 * @todo Implement hypersine transform
 * @todo Implement hyperlog transform
 *
 * @todo Finish quadrant gates
 * @todo Finish boolean gates
 * @todo Implement "within gate" query for all gate types
 *
 * @todo Implement compensation matrix container
 * @todo Implement compensation matrix execution
 * @todo Add user-given name to each compensation
 * @todo Add user-given description to each compensation
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
// standard data types and math functions, stdio for fast buffered I/O,
// and a variety of char* based string functions for fast string handling.
// This avoids the object overhead of C++.
//
// Data types.
#include <cstdint>      // size_t, uint32_t, uint16_t, uint8_t, ...

// Standard C libraries.
#include <cstring>      // std::strerror, ...
#include <cmath>        // std::sinh, std::asinh, ...

// Standard C++ libraries.
#include <limits>       // std::numeric_limits
#include <memory>       // std::shared_ptr
#include <stdexcept>    // std::invalid_argument, std::out_of_range, ...
#include <string>       // std::string, ...
#include <utility>      // std::pair
#include <vector>       // std::vector





namespace FlowGate {
namespace Gates {

class Gate;
class GateTrees;





//----------------------------------------------------------------------
//
// Typedefs.
//
//----------------------------------------------------------------------
//
/**
 * The data type for a unique numeric gate ID.
 *
 * @see Gate::getId()
 */
typedef uint32_t GateId;

/**
 * The data type for a unique numeric transform ID.
 *
 * @see Transform::getId()
 */
typedef uint32_t TransformId;

/**
 * The data type for a gate index within a parent gate list or a gate
 * tree roots list.
 *
 * @internal
 * This is intentionally a 32-bit value, even though gates are stored
 * in a std::vector with a 64-bit size. The number of gates in a gate
 * tree will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t GateIndex;

/**
 * The data type for a dimension index within a gate list of dimensions.
 *
 * @internal
 * This is intentionally a 32-bit value, even though values per dimension
 * are stored in a std::vector with a 64-bit size. The number of dimensions
 * in a gate will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t DimensionIndex;

/**
 * The data type for a transform index within a gate's list of transforms.
 *
 * @internal
 * This is intentionally a 32-bit value, even though transforms per gate
 * are stored in a std::vector with a 64-bit size. The number of transforms
 * in a gate will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t TransformIndex;

/**
 * The data type for a vertex index within a polygon gate's list of vertices.
 *
 * @internal
 * This is intentionally a 32-bit value, even though vertices are stored
 * in a std::vector with a 64-bit size. The number of vertices will never
 * will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t VertexIndex;

/**
 * The data type for a divider index within a quadrant gate's list of dividers.
 *
 * @internal
 * This is intentionally a 32-bit value, even though dividers are stored
 * in a std::vector with a 64-bit size. The number of dividers will never
 * will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t DividerIndex;

/**
 * The data type for a divider index within a quadrant gate's list of quadrants.
 *
 * @internal
 * This is intentionally a 32-bit value, even though quadrants are stored
 * in a std::vector with a 64-bit size. The number of quadrants will never
 * will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t QuadrantIndex;

/**
 * The data type for a divider index within a quadrant gate's list of divider
 * positions.
 *
 * @internal
 * This is intentionally a 32-bit value, even though positions are stored
 * in a std::vector with a 64-bit size. The number of positions will never
 * will never approach a 64-bit value, or even a 32-bit value.
 * But 32-bit integers are fast for processor loops.
 * @endinternal
 */
typedef uint32_t PositionIndex;





//----------------------------------------------------------------------
//
// Utilities for transforms.
//
//----------------------------------------------------------------------
//
/**
 * Indicates the type of transform.
 *
 * The transform type has a one-to-one correspondence with the various
 * standard transform subclasses defined below. For instance, the type
 * PARAMETERIZED_LINEAR_TRANSFORM indicates the ParameterizedLinearTransform
 * class.
 *
 * @see Transform::getTransformType()
 * @see getTransformTypeName()
 * @see findTransformTypeByName()
 *
 * @internal
 * The type of a transform can be determined from the transform instance's
 * class by using a dynamic cast, but this is slow to execute and cumbersome
 * to code, and particularly when using shared pointers. The code would read:
 * @code
 * auto linearTransform = dynamic_cast<ParameterizedLinearTransform>( transform.get( ) );
 * if ( linearTransform != nullptr )
 * {
 *   ...
 * }
 * else
 * {
 *   auto logTransform = dynamic_cast<ParameterizedLogarithmicTransform>( transform.get( ) );
 *   if ( logTransform != nullptr )
 *   {
 *     ...
 *   }
 *   else
 *   {
 *     ...
 *   }
 * }
 * @endcode
 *
 * Since this type of transform type check is common, it is useful to make it
 * faster to execute and easier to code. Using a transform type, this reads:
 * @code
 * switch ( transform->getTransformType( ) )
 * {
 * case PARAMETERIZED_LINEAR_TRANSFORM:
 *   auto linearTransform = (ParameterizedLinearTransform*)(transform.get( ));
 *   ...
 *   break;
 *
 * case PARAMETERIZED_LOGARITHMIC_TRANSFORM:
 *   auto logTransform = (ParameterizedLogarithmicTransform*)(transform.get( ));
 *   ...
 *   break;
 * }
 * @endcode
 * @endinternal
 */
enum TransformType
{
    // Gating-ML 2.0 standard ------------------------------------------
    PARAMETERIZED_LINEAR_TRANSFORM,
    PARAMETERIZED_LOGARITHMIC_TRANSFORM,
    PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM,
    LOGICLE_TRANSFORM,
    HYPERLOG_TRANSFORM,

    // Custom ----------------------------------------------------------
    CUSTOM_TRANSFORM
};

/**
 * Returns a human-readable shorthand name for the transform type.
 *
 * This function maps a transform type enum value to a corresponding
 * human-readable shorthand name:
 * - "custom" for CUSTOM_TRANSFORM.
 * - "hyperlog" for HYPERLOG_TRANSFORM.
 * - "inverse_hyperbolic_sine" for PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANFORM.
 * - "linear" for PARAMETERIZED_LINEAR_TRANSFORM.
 * - "log" for PARAMETERIZED_LOGARITHMIC_TRANSFORM.
 * - "logicle" for LOGICLE_TRANSFORM.
 *
 * These names are stable and may be used in output formats, such as
 * GatingML or JSON gates.
 *
 * @param[in] transformType
 *   The transform type.
 *
 * @return
 *   Returns a human-readable name for the transform type.
 *
 * @see TransformType
 * @see findTransformTypeByName()
 * @see Transform::getTransformType()
 * @see FileGatingML
 * @see FileFGJsonGates
 */
static std::string getTransformTypeName(
    const TransformType& transformType )
    noexcept
{
    switch ( transformType )
    {
    // Gating-ML 2.0 standard ------------------------------------------
    case PARAMETERIZED_LINEAR_TRANSFORM:
        return "linear";
    case PARAMETERIZED_LOGARITHMIC_TRANSFORM:
        return "log";
    case PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM:
        return "inverse_hyperbolic_sine";
    case LOGICLE_TRANSFORM:
        return "logicle";
    case HYPERLOG_TRANSFORM:
        return "hyperlog";

    // Custom ----------------------------------------------------------
    default:
    case CUSTOM_TRANSFORM:
        return "custom";
    }
}

/**
 * Returns a transform type from a human-readable shorthnad name.
 *
 * This function maps a human-readable shorthand transform type name to
 * the corresponding transform type enum value.
 *
 * @param[in] name
 *   The transform type name.
 *
 * @return
 *   Returns the transform type.
 *
 * @see TransformType
 * @see getTransformTypeName()
 * @see Transform::getTransformType()
 * @see FileGatingML
 * @see FileFGJsonGates
 */
TransformType findTransformTypeByName( const std::string& name )
    noexcept
{
    if ( name == "linear" )
        return PARAMETERIZED_LINEAR_TRANSFORM;
    if ( name == "log" )
        return PARAMETERIZED_LOGARITHMIC_TRANSFORM;
    if ( name == "inverse_hyperbolic_sine" )
        return PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM;
    if ( name == "logicle" )
        return LOGICLE_TRANSFORM;
    if ( name == "hyperlog" )
        return HYPERLOG_TRANSFORM;

    return CUSTOM_TRANSFORM;
}





//----------------------------------------------------------------------
//
// Utilities for gate trees
//
//----------------------------------------------------------------------
//
/**
 * Indicates the type of gate.
 *
 * The gate type has a one-to-one correspondence with the various standard
 * gate subclasses defined below. For instance, the type RECTANGLE_GATE
 * indicates the RectangleGate class.
 *
 * @see Gate::getGateType()
 * @see getGateTypeName()
 * @see findGateTypeByName()
 *
 * @internal
 * The type of a gate can be determined from the gate instance's class by using
 * a dynamic cast, but this is slow to execute and cumbersome to code, and
 * particularly when using shared pointers. The code would read:
 * @code
 * auto rectangleGate = dynamic_cast<RectangleGate>( gate.get( ) );
 * if ( rectangleGate != nullptr )
 * {
 *   ...
 * }
 * else
 * {
 *   auto polygonGate = dynamic_cast<PolygonGate>( gate.get( ) );
 *   if ( polygonGate != nullptr )
 *   {
 *     ...
 *   }
 *   else
 *   {
 *     ...
 *   }
 * }
 * @endcode
 *
 * Since this type of gate type check is common, it is useful to make it faster
 * to execute and easier to code. Using a gate type, this reads:
 * @code
 * switch ( gate->getGateType( ) )
 * {
 * case RECTANGLE_GATE:
 *   auto rectangleGate = (RectangleGate*)(gate.get( ));
 *   ...
 *   break;
 *
 * case POLYGON_GATE:
 *   auto polygonGate = (PolygonGate*)(gate.get( ));
 *   ...
 *   break;
 * }
 * @endcode
 * @endinternal
 */
enum GateType
{
    // Gating-ML 2.0 standard ------------------------------------------
    RECTANGLE_GATE,
    POLYGON_GATE,
    ELLIPSOID_GATE,
    QUADRANT_GATE,
    BOOLEAN_GATE,

    // Custom ----------------------------------------------------------
    CUSTOM_GATE
};

/**
 * Returns a human-readable shorthand name for the gate type.
 *
 * This function maps a gate type enum value to a corresponding
 * human-readable shorthand name:
 * - "boolean' for BOOLEAN_GATE.
 * - "custom" for CUSTOM_GATE.
 * - "ellipsoid" for ELLIPSOID_GATE.
 * - "polygon" for POLYGON_GATE.
 * - "quadrant" for QUADRANT_GATE.
 * - "rectangle" for RECTANGLE_GATE.
 *
 * These names are stable and may be used in output formats, such as
 * JSON gates.
 *
 * @param[in] gateType
 *   The gate type.
 *
 * @return
 *   Returns a human-readable name for the gate type.
 *
 * @see GateType
 * @see findGateTypeByName()
 * @see Gate::getGateType()
 * @see FileGatingML
 * @see FileFGJsonGates
 */
std::string getGateTypeName( const GateType& gateType )
    noexcept
{
    switch ( gateType )
    {
    // Gating-ML 2.0 standard ------------------------------------------
    case RECTANGLE_GATE:
        return "rectangle";
    case POLYGON_GATE:
        return "polygon";
    case ELLIPSOID_GATE:
        return "ellipsoid";
    case QUADRANT_GATE:
        return "quadrant";
    case BOOLEAN_GATE:
        return "boolean";

    // Custom ----------------------------------------------------------
    default:
    case CUSTOM_GATE:
        return "custom";
    }
}

/**
 * Returns a gate type from a human-readable shorthand name.
 *
 * This function maps a human-readable shorthand gate type name to
 * the corresponding gate type enum value.
 *
 * @param[in] name
 *   The gate type name.
 *
 * @return
 *   Returns the gate type.
 *
 * @see Gatetype
 * @see getGateTypeName()
 * @see Gate::getGateType()
 * @see FileGatingML
 * @see FileFGJsonGates
 */
GateType findGateTypeByName( const std::string& name )
    noexcept
{
    if ( name == "rectangle" )
        return RECTANGLE_GATE;
    if ( name == "polygon" )
        return POLYGON_GATE;
    if ( name == "ellipsoid" )
        return ELLIPSOID_GATE;
    if ( name == "quadrant" )
        return QUADRANT_GATE;
    if ( name == "boolean" )
        return BOOLEAN_GATE;

    return CUSTOM_GATE;
}

/**
 * Indicates the gating method.
 *
 * Events may be gated using the shape and size of the gate (e.g. a
 * rectangular gate) in one of several methods:
 *
 * @li EVENT_VALUE_GATING uses the event's value(s) to determine if the
 *   event is inside or outside of the gate.
 *
 * @li DAFI_CLUSTER_CENTROID_GATING uses the DAFi clustering algorithm
 *   to find the centroids of event value clusters, then uses those
 *   centroids to determine if the cluster is inside or outside of the
 *   gate.
 *
 * Additional methods may be defined in the future.
 *
 * @see Gate::getGatingMethod()
 * @see getGatingMethodName()
 * @see findGatingMethodByName()
 */
enum GatingMethod
{
    // Conventional ----------------------------------------------------
    EVENT_VALUE_GATING,

    // FlowGate custom -------------------------------------------------
    DAFI_CLUSTER_CENTROID_GATING,

    // Other -----------------------------------------------------------
    CUSTOM_GATING
};

/**
 * Returns a human-readable shorthand name for the gating method.
 *
 * This function maps a gating method enum value to a corresponding
 * human-readable shorthand name:
 * - "custom" for CUSTOM_GATING.
 * - "event" for EVENT_VALUE_GATING.
 * - "dafi" for DAFI_CLUSTER_CENTROID_GATING.
 *
 * These names are stable and may be used in output formats, such as
 * GatingML or JSON gates.
 *
 * @param[in] gatingMethod
 *   The gating method.
 *
 * @return
 *   Returns a human-readable name for the gating method.
 *
 * @see GatingMethod
 * @see findGatingMethodByName()
 * @see Gate::getGatingMethod()
 * @see FileGatingML
 * @see FileFGJsonGates
 */
std::string getGatingMethodName( const GatingMethod& gatingMethod )
    noexcept
{
    switch ( gatingMethod )
    {
    // Conventional.
    case EVENT_VALUE_GATING:
        return "event";

    // FlowGate custom.
    case DAFI_CLUSTER_CENTROID_GATING:
        return "dafi";

    // Other.
    default:
    case CUSTOM_GATING:
        return "custom";
    }
}

/**
 * Returns a gating method from a human-readable shorthand name.
 *
 * This function maps a human-readable shorthand gating method name to
 * the corresponding gating method enum value.
 *
 * @param[in] name
 *   The gating method name.
 *
 * @return
 *   Returns the gating method.
 *
 * @see GatingMethod
 * @see gatGatingMethodName()
 * @see Gate::getGatingMethod()
 * @see FileGatingML
 * @see FileFGJsonGates
 */
GatingMethod findGatingMethodByName( const std::string& name )
    noexcept
{
    if ( name == "event" || name == "manual" )
        return EVENT_VALUE_GATING;
    if ( name == "dafi" || name == "cluster" )
        return DAFI_CLUSTER_CENTROID_GATING;

    return CUSTOM_GATING;
}





//----------------------------------------------------------------------
//
// Transforms.
//
//----------------------------------------------------------------------
/**
 * Describes a flow cytometry transform.
 *
 * This is the parent class of each of several types of transform.
 * All subclasses of this class contain the following attributes:
 *
 * @li A transform type that indicates the subclass of transform.
 */
class Transform
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    // Transform management --------------------------------------------
    /**
     * The unique ID of the transform.
     *
     * The ID is automatically generated as Transform objects are created.
     * IDs are assigned sequentially, starting at 0, and they do not repeat
     * until a 64-bit unsigned integer overflows.
     *
     * The ID cannot be set. However, an "original ID" may be set as an
     * alternate ID for the Transform. Such IDs are not guaranteed to be
     * unique and may come from reading in a file, such as Gating-ML.
     *
     * @see ::getId()
     * @see ::getOriginalId()
     * @see ::setOriginalId()
     */
    const TransformId id;

    /**
     * The transform type.
     *
     * @see ::getTransformType()
     */
    const TransformType transformType;

    // FlowGate-specific features --------------------------------------
    /**
     * The original possibly unique ID of the transform.
     *
     * The ID default to the empty string, but it may be set to an arbitrary
     * value. The caller should insure that the ID is unique, but this is not
     * guaranteed. A typical use is to set the ID to a value read from a
     * gating file, such as Gating-ML.
     *
     * @see ::getId()
     * @see ::getOriginalId()
     * @see ::setOriginalId()
     */
    std::string originalId;

    /**
     * The optional name of the transform.
     *
     * The name defaults to the empty string.
     *
     * In Gating-ML, this is found in in custom information for the transform:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <name>...</name>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * This also may be found by importing Cytobank features in custom
     * information for the transform:
     * @code
     * <data-type:custom_info>
     *   <cytobank>
     *     <name>...</name>
     *   </cytobank>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getName()
     * @see ::setName()
     */
    std::string name;

    /**
     * The optional description of the transform.
     *
     * In Gating-ML, this is found in custom information for the transform:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <description>...</description>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * Or generically in custom information for the transform:
     * @code
     * <data-type:custom_info>
     *   ...
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getDescription()
     * @see ::setDescription()
     */
    std::string description;




//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
protected:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a transform.
     *
     * @param[in] transformType
     *   The type of transform. There is a one-to-one correspondence between
     *   transform types and transform subclasses. The transform type must
     *   match the subclass.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the transform type is not one of the well-known
     *   transform types defined in this class.
     *
     * @see getTransformType()
     */
    Transform( const TransformType transformType )
        : id( getNextId() ),
        transformType( transformType )
    {
        switch ( transformType )
        {
        case PARAMETERIZED_LINEAR_TRANSFORM:
        case PARAMETERIZED_LOGARITHMIC_TRANSFORM:
        case PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM:
        case LOGICLE_TRANSFORM:
        case HYPERLOG_TRANSFORM:
            break;

        default:
            throw std::invalid_argument( "Invalid transform type." );
        }
    }

public:
    /**
     * Constructs a copy of the given transform.
     *
     * @param[in] transform
     *   The transform to copy.
     */
    Transform( const Transform& transform )
        noexcept
        : id( getNextId() ),
        transformType( transform.transformType )
    {
        this->description = transform.description;
        this->name        = transform.name;
        this->originalId  = transform.originalId;
    }

    /**
     * Constructs a copy of the given transform.
     *
     * The new transform has the same type and attributes as the given
     * transform, but a unique ID.
     *
     * @param[in] transform
     *   The transform to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the transform is nullptr.
     */
    Transform( std::shared_ptr<const Transform> transform )
        : id( getNextId() ),
        transformType( (transform == nullptr) ?
            PARAMETERIZED_LINEAR_TRANSFORM :
            transform->transformType )
    {
        if ( transform == nullptr )
            throw std::invalid_argument( "Invalid nullptr transform." );

        this->description = transform->description;
        this->name        = transform->name;
        this->originalId  = transform->originalId;
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~Transform( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
    /**
     * @name Attributes
     */
    // @{
private:
    /**
     * Returns the next unique ID to use for transforms.
     *
     * @return
     *   Returns the next ID.
     */
    inline TransformId getNextId()
        noexcept
    {
        static TransformId nextId = 0;
        return nextId++;
    }

public:
    /**
     * Returns the optional description for the transform.
     *
     * The description defaults to the empty string.
     *
     * @return
     *   Returns the description, or an empty string if no description
     *   has been set.
     *
     * @see ::setDescription()
     */
    inline const std::string& getDescription( )
        const noexcept
    {
        return this->description;
    }

    /**
     * Returns the unique ID for the transform.
     *
     * The ID is set when the transform is created, it is guaranteed to
     * be unique (up to 2^32 transforms), and it cannot be changed.
     *
     * @return
     *   Returns the unique ID.
     *
     * @see ::setId()
     */
    inline TransformId getId( )
        const noexcept
    {
        return this->id;
    }

    /**
     * Returns the optional name for the transform.
     *
     * @return
     *   Returns the name, or an empty string if no name has been set.
     *
     * @see ::setName()
     */
    inline const std::string& getName( )
        const noexcept
    {
        return this->name;
    }

    /**
     * Returns the optional original ID for the transform.
     *
     * The default original ID is an empty string. The ID may be set to
     * any value and it should be unique, though this is not enforced.
     * A typical use is to set the original ID to an ID found in an input
     * file, such as one using Gating-ML.
     *
     * See the getId() method for a guaranteed unique numeric ID for each
     * transform.
     *
     * @return
     *   Returns the original ID, or an empty string if no ID has been set.
     *
     * @see ::getId()
     * @see ::setOriginalId()
     */
    inline const std::string& getOriginalId( )
        const noexcept
    {
        return this->originalId;
    }

    /**
     * Returns the transform's type.
     *
     * The transform type has a one-to-one correspondence with the transform
     * subclass for known transform types. The type is set when a transform
     * is constructed and cannot be changed afterwards.
     *
     * The transform type may be used to test for a specific type of transform
     * before casting the transform's pointer to a transform of that type:
     * @code
     * if ( transform->getTransformType( ) == PARAMETERIZED_LINEAR_TRANSFORM )
     * {
     *   auto linearTransform = (ParameterizedLinearTransform*)(transform.get());
     *   ...
     * }
     * @endcode
     *
     * @return
     *   The transform type.
     *
     * @see ::getTransformTypeName()
     */
    inline TransformType getTransformType( )
        const noexcept
    {
        return this->transformType;
    }

    /**
     * Returns the transform's type name.
     *
     * The transform type has a one-to-one correspondence with the transform
     * subclass for known transform types. The type is set when a transform
     * is constructed and cannot be changed afterwards.
     *
     * The transform type name may be used to display information to a user:
     * @code
     * std::cout << "Transform type is: " << transform->getTransformTypeName( ) << std::endl;
     * @endcode
     *
     * @return
     *   The transform type name.
     *
     * @see ::getTransformType()
     */
    inline std::string getTransformTypeName( )
        const noexcept
    {
        return FlowGate::Gates::getTransformTypeName( this->transformType );
    }



    /**
     * Sets the optional description for the transform.
     *
     * @param[in] description
     *   The description.
     *
     * @see ::getDescription()
     */
    inline void setDescription( const std::string& description )
        noexcept
    {
        this->description = description;
    }

    /**
     * Sets the optional name for the transform.
     *
     * @param[in] name
     *   The name.
     *
     * @see ::getName()
     */
    inline void setName( const std::string& name )
        noexcept
    {
        this->name = name;
    }

    /**
     * Sets the optional original ID for the transform.
     *
     * The ID may be set to any value and it should be unique, though this
     * method does not enforce this. A typical use is to set the original ID
     * to an ID found in an input file, such as one using Gating-ML.
     *
     * See the getId() method for a guaranteed unique numeric ID for each
     * transform.
     *
     * @param[in] id
     *   The id.
     *
     * @see ::getId()
     * @see ::getOriginalId()
     */
    inline void setOriginalId( const std::string& id )
        noexcept
    {
        this->originalId = id;
    }
    // @}





//----------------------------------------------------------------------
// Copy & clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the transform.
     *
     * @return
     *   Returns a clone of this transform.
     */
    virtual Transform* clone( ) const = 0;
    // @}





//----------------------------------------------------------------------
// Transform.
//----------------------------------------------------------------------
public:
    /**
     * @name Transform
     */
    // @{
    /**
     * Transforms an event parameter value into a new value.
     *
     * @param[in] x
     *   The value to transform.
     *
     * @return
     *   Returns the transformed value.
     */
    virtual double transform( const double x ) const noexcept = 0;

    /**
     * Transforms an array of event parameter values into new values,
     * storing them back into the same array.
     *
     * @param[in] size
     *   The number of elements in the array.
     * @param[in,out] array
     *   The array of values to transform.
     *
     * @throws std::invalid_argument
     *   Throws an exeption if the array is nullptr or the size is zero.
     *
     * @internal
     * This is a default implementation. Individual subclasses should
     * override this in order to provide loop code that a compiler can
     * vectorize for better performance.
     * @endinternal
     */
    virtual void transform( const size_t size, double*const array )
        const
    {
        if ( size <= 0 )
            throw std::invalid_argument( "Invalid non-positive array size." );
        if ( array == nullptr )
            throw std::invalid_argument( "Invalid nullptr array." );

#pragma omp parallel for schedule(static)
        for ( size_t i = 0; i < size; ++i )
            array[i] = this->transform( array[i] );
    }

    /**
     * Transforms an array of event parameter values into new values,
     * storing them back into the same array.
     *
     * @param[in] size
     *   The number of elements in the array.
     * @param[in,out] array
     *   The array of values to transform.
     *
     * @throws std::invalid_argument
     *   Throws an exeption if the array is nullptr or the size is zero.
     *
     * @internal
     * This is a default implementation. Individual subclasses should
     * override this in order to provide loop code that a compiler can
     * vectorize for better performance.
     * @endinternal
     */
    virtual void transform( const size_t size, float*const array )
        const
    {
        if ( size <= 0 )
            throw std::invalid_argument( "Invalid non-positive array size." );
        if ( array == nullptr )
            throw std::invalid_argument( "Invalid nullptr array." );

#pragma omp parallel for schedule(static)
        for ( size_t i = 0; i < size; ++i )
            array[i] = (float)this->transform( (double)array[i] );
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the transform.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string indent = "" )
        const noexcept
    {
        return indent + "Transform( )\n";
    }
    // @}
};



/**
 * Describes a parameterized linear transform (a.k.a. "flin").
 *
 * Like all transforms, the parameterized linear transform remaps an
 * event parameter value "x" into a new value. This transform uses the
 * following equation:
 *
 * @code
 *   y = (x + A) / (T + A)
 * @endcode
 *
 * T is the "top of scale", or highest expected value for the parameter,
 * and A is an offset that determines the bottom end of the transformation.
 *
 * The result is a mapping from the incoming value range of [-A,T] to a
 * unit interval [0,1].
 *
 * Requirements:
 * @li T > 0
 * @li 0 <= A <= T
 */
class ParameterizedLinearTransform final
    : public Transform
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The "T" top of scale.
     *
     * The scale factor must be a positive number.
     */
    const double t;

    /**
     * The "A" bottom of scale.
     *
     * The scale factor must be a non-negative number, even though it is used
     * to define the bottom end of the [-A,T] value range.
     */
    const double a;

    /**
     * The cached inverse of (T + A).
     */
    const double inverseSum;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new transform.
     *
     * @param[in] t
     *   The top of scale.
     * @param[in] a
     *   The bottom of scale.
     *
     * @throws std::invalid_argument
     *   Throws an exception if t is <= 0, a is < 0, or a > t.
     */
    ParameterizedLinearTransform( const double t, const double a )
        : Transform( PARAMETERIZED_LINEAR_TRANSFORM ),
        t( t ),
        a( a ),
        inverseSum( (t == 0.0 && a == 0.0) ? 0.0 : (1.0 / (t + a)) )
    {
        if ( t <= 0.0 )
            throw std::invalid_argument( "Invalid T must be positive." );
        if ( a < 0.0 )
            throw std::invalid_argument( "Invalid A must be non-negative." );
        if ( a > t )
            throw std::invalid_argument( "Invalid A must be less than or equal to T." );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~ParameterizedLinearTransform( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the top-of-scale or "T" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getT( )
        const noexcept
    {
        return this->t;
    }

    /**
     * Returns the bottom-of-scale or "A" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getA( )
        const noexcept
    {
        return this->a;
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the transform.
     *
     * @return
     *   Returns a clone of this transform.
     */
    virtual Transform* clone( )
        const override
    {
        return new ParameterizedLinearTransform( this->t, this->a );
    }
    // @}



//----------------------------------------------------------------------
// Transform.
//----------------------------------------------------------------------
public:
    /**
     * @name Transform
     */
    // @{
    /**
     * Transforms an event parameter value into a new value.
     *
     * @param[in] x
     *   The value to transform.
     *
     * @return
     *   Returns the transformed value.
     */
    virtual double transform( const double x )
        const noexcept override
    {
        return (x + this->a) * this->inverseSum;
    }

    /**
     * Transforms an array of event parameter values into new values,
     * storing them back into the same array.
     *
     * @param[in] size
     *   The number of elements in the array.
     * @param[in,out] array
     *   The array of values to transform.
     *
     * @throws std::invalid_argument
     *   Throws an exeption if the array is nullptr or the size is zero.
     */
    virtual void transform( const size_t size, double*const array )
        const override
    {
        if ( size <= 0 )
            throw std::invalid_argument( "Invalid non-positive array size." );
        if ( array == nullptr )
            throw std::invalid_argument( "Invalid nullptr array." );

        const double is = this->inverseSum;
#pragma omp parallel for simd schedule(static)
        for ( size_t i = 0; i < size; ++i )
            array[i] = (a + array[i]) * is;
    }

    /**
     * Transforms an array of event parameter values into new values,
     * storing them back into the same array.
     *
     * @param[in] size
     *   The number of elements in the array.
     * @param[in,out] array
     *   The array of values to transform.
     *
     * @throws std::invalid_argument
     *   Throws an exeption if the array is nullptr or the size is zero.
     */
    virtual void transform( const size_t size, float*const array )
        const override
    {
        if ( size <= 0 )
        {
            throw std::invalid_argument( "Invalid non-positive array size." );
        }
        if ( array == nullptr )
        {
            throw std::invalid_argument( "Invalid nullptr array." );
        }

        const double is = this->inverseSum;
#pragma omp parallel for simd schedule(static)
        for ( size_t i = 0; i < size; ++i )
            array[i] = (float)((a + array[i]) * is);
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the transform.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string indent = "" )
        const noexcept override
    {
        return indent + "Linear ( t=" +
            std::to_string( this->t ) + ", a=" +
            std::to_string( this->a ) + " )\n";
    }
    // @}
};



/**
 * Describes a parameterized logarithmic transform (a.k.a. "flog").
 *
 * Like all transforms, the parameterized lienar transform remaps an
 * event parameter value "x" into a new value. This transform uses the
 * following equation:
 *
 * @code
 *   newx = (1/M) * log10(x/T) + 1
 * @endcode
 *
 * T is the "top of scale", or highest expected value for the parameter,
 * and M is the desired number of logarithmic decades.
 *
 * The result is a mapping from the incoming value range of (0,T] to a
 * (-infinity,1] interval in a log space. The data value at T is mapped to
 * 1, and M decades of values are mapped onto the unit interval.
 *
 * Requirements:
 * @li T > 0
 * @li M > 0
 */
class ParameterizedLogarithmicTransform final
    : public Transform
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The "T" top of scale.
     *
     * The scale factor must be a positive number.
     */
    const double t;

    /**
     * The "M" number of decades for the logarithmic space.
     *
     * The number of decades must be a positive number.
     */
    const double m;

    /**
     * The cached inverse of "T".
     */
    const double inverseT;

    /**
     * The cached inverse of "M".
     */
    const double inverseM;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new transform.
     *
     * @param[in] t
     *   The top of scale.
     * @param[in] m
     *   The number of decades.
     *
     * @throws std::invalid_argument
     *   Throws an exception if t or m are non-positive.
     */
    ParameterizedLogarithmicTransform( const double t, const double m )
        : Transform( PARAMETERIZED_LOGARITHMIC_TRANSFORM ),
        t( t ),
        m( m ),
        inverseT( (t == 0.0) ? 0.0 : (1.0 / t) ),
        inverseM( (m == 0.0) ? 0.0 : (1.0 / m) )
    {
        if ( t <= 0.0 )
            throw std::invalid_argument( "Invalid T must be positive." );
        if ( m <= 0.0 )
            throw std::invalid_argument( "Invalid M must be positive." );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~ParameterizedLogarithmicTransform( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the top-of-scale or "T" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getT( )
        const noexcept
    {
        return this->t;
    }

    /**
     * Returns the number of decades or "M" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getM( )
        const noexcept
    {
        return this->m;
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the transform.
     *
     * @return
     *   Returns a clone of this transform.
     */
    virtual Transform* clone( )
        const override
    {
        return new ParameterizedLogarithmicTransform( this->t, this->m );
    }
    // @}



//----------------------------------------------------------------------
// Transform.
//----------------------------------------------------------------------
public:
    /**
     * @name Transform
     */
    // @{
    /**
     * Transforms an event parameter value into a new value.
     *
     * @param[in] x
     *   The value to transform.
     *
     * @return
     *   Returns the transformed value.
     */
    virtual double transform( const double x )
        const noexcept override
    {
        return this->inverseM * std::log10( x * this->inverseT ) + 1.0;
    }

    /**
     * Transforms an array of event parameter values into new values,
     * storing them back into the same array.
     *
     * @param[in] size
     *   The number of elements in the array.
     * @param[in,out] array
     *   The array of values to transform.
     *
     * @throws std::invalid_argument
     *   Throws an exeption if the array is nullptr or the size is zero.
     */
    virtual void transform( const size_t size, double*const array )
        const override
    {
        if ( size <= 0 )
            throw std::invalid_argument( "Invalid non-positive array size." );
        if ( array == nullptr )
            throw std::invalid_argument( "Invalid nullptr array." );

	const double it = this->inverseT;
	const double im = this->inverseM;
#pragma omp parallel for simd schedule(static)
        for ( size_t i = 0; i < size; ++i )
            array[i] = im * std::log10( array[i] * it ) + 1.0;
    }

    /**
     * Transforms an array of event parameter values into new values,
     * storing them back into the same array.
     *
     * @param[in] size
     *   The number of elements in the array.
     * @param[in,out] array
     *   The array of values to transform.
     *
     * @throws std::invalid_argument
     *   Throws an exeption if the array is nullptr or the size is zero.
     */
    virtual void transform( const size_t size, float*const array )
        const override
    {
        if ( size <= 0 )
            throw std::invalid_argument( "Invalid non-positive array size." );
        if ( array == nullptr )
            throw std::invalid_argument( "Invalid nullptr array." );

	const double it = this->inverseT;
	const double im = this->inverseM;
#pragma omp parallel for simd schedule(static)
        for ( size_t i = 0; i < size; ++i )
            array[i] = (float)(im * std::log10( array[i] * it ) + 1.0);
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the transform.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string indent = "" )
        const noexcept override
    {
        return indent + "Log ( t=" +
            std::to_string( this->t ) + ", m=" +
            std::to_string( this->m ) + " )\n";
    }
    // @}
};



/**
 * Describes a parameterized inverse hyperbolic sine transform
 * (a.k.a. "fasinh").
 *
 * Like all transforms, the parameterized inverse hyperbolic sine transform
 * remaps an event parameter value "x" into a new value. This transform uses
 * the following equation:
 *
 * @code
 *   y = asinh( x * sinh( M * ln(10)) / T ) + A * ln(10) / ((M + A) * ln(10))
 * @endcode
 *
 * sinh() is the hyperbolic sine function. asinh() is the inverse hyperbolic
 * sine function. ln() is the natural logarithm with base e.
 *
 * T is the "top of scale", or highest expected value for the parameter,
 * and M is the desired number of decades. A is the number of additional
 * negative decades that will be "brought to scale".
 *
 * The result is a mapping from the incoming value range (0,T] to a unit
 * interval [0,1]. The data value at T is mapped to 1.
 *
 * Requirements:
 * @li T > 0
 * @li M > 0
 * @li 0 <= A <= M
 */
class ParameterizedInverseHyperbolicSineTransform final
    : public Transform
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The "T" top of scale.
     *
     * The scale factor must be a positive number.
     */
    const double t;

    /**
     * The "A" bottom of scale.
     *
     * The scale factor must be a non-negative number.
     */
    const double a;

    /**
     * The "M" number of decades for the logarithmic space.
     *
     * The number of decades must be a positive number.
     */
    const double m;

    /**
     * The cached value of "1 / ((M + A) * ln(10))".
     */
    const double inverseDenominator;

    /**
     * The cached value of "A * ln(10)".
     */
    const double aln10;

    /**
     * The cached value of "sinh( M * ln(10) ) / T".
     */
    const double sinhmt;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new transform.
     *
     * @param[in] t
     *   The top of scale.
     * @param[in] a
     *   The bottom of scale.
     * @param[in] m
     *   The number of decades.
     *
     * @throws std::invalid_argument
     *   Throws an exception if t or m are zero.
     */
    ParameterizedInverseHyperbolicSineTransform(
        const double t,
        const double a,
        const double m )
        : Transform( PARAMETERIZED_INVERSE_HYPERBOLIC_SINE_TRANSFORM ),
        t( t ),
        a( a ),
        m( m ),
        inverseDenominator( ((m + a) == 0.0) ?
            0.0 :
            (1.0 / ((m+a) * std::log(10.0))) ),
        aln10( a * std::log(10.0) ),
        sinhmt( (t == 0.0) ?
            0.0 :
            std::sinh( m * std::log(10.0) ) / t )
    {
        if ( t <= 0.0 )
            throw std::invalid_argument( "Invalid T must be positive." );
        if ( m <= 0.0 )
            throw std::invalid_argument( "Invalid M must be positive." );
        if ( a < 0.0 )
            throw std::invalid_argument( "Invalid A must be non-negative." );
        if ( a > m )
            throw std::invalid_argument( "Invalid A must be less than or equal to M." );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~ParameterizedInverseHyperbolicSineTransform( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the top-of-scale or "T" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getT( )
        const noexcept
    {
        return this->t;
    }

    /**
     * Returns the bottom-of-scale or "A" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getA( )
        const noexcept
    {
        return this->a;
    }

    /**
     * Returns the number of decades or "M" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getM( )
        const noexcept
    {
        return this->m;
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the transform.
     *
     * @return
     *   Returns a clone of this transform.
     */
    virtual Transform* clone( )
        const override
    {
        return new ParameterizedInverseHyperbolicSineTransform(
            this->t, this->a, this->m );
    }
    // @}



//----------------------------------------------------------------------
// Transform.
//----------------------------------------------------------------------
public:
    /**
     * @name Transform
     */
    // @{
    using Transform::transform;

    /**
     * Transforms an event parameter value into a new value.
     *
     * @param[in] x
     *   The value to transform.
     *
     * @return
     *   Returns the transformed value.
     */
    virtual double transform( const double x )
        const noexcept override
    {
        return (std::asinh( x * this->sinhmt ) + this->aln10) *
            this->inverseDenominator;
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the transform.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string indent = "" )
        const noexcept override
    {
        return indent + "Inverse hyperbolic sine( t=" +
            std::to_string( this->t ) + ", a=" +
            std::to_string( this->a ) + ", m=" +
            std::to_string( this->m ) + " )\n";
    }
    // @}
};



/**
 * Describes a logicle transform.
 *
 * Like all transforms, the logicle transform remaps an
 * event parameter value "x" into a new value. This transform uses the
 * following equation:
 *
 * @code
 *   B = a * e^(b*y) - c * e^(-d*y) - f
 *   y = root(B - x)
 * @endcode
 *
 * root() is a root finding algorithm (e.g. Newton's method) that finds
 * y such that B(y,T,W,M,A) = x.
 *
 * T is the "top of scale", or highest expected value for the parameter,
 * and M is the desired number of decades. e is "e" (i.e. 2.718281828).
 * W is the number of linear decades in the near-linear region of the
 * output. A is the number of additional decades of negative data values
 * to be included.
 *
 * The remaining variables are defined as:
 *
 * @code
 *   a = T / (e^b - fa - ca / e^d)
 *   b = (M + A) * ln(10)
 *   c = ca * a
 *   ca = e^(x0 * (b+ d))
 *   d is defined such that 2 * (ln(d) - ln(b)) + w(d + b) = 0
 *   f = fa * a
 *   fa = e^(b*x1) - ca / e^(d*x1)
 *   w = W / (M+A)
 *   x0 = x2 + 2*w
 *   x1 = x2 + w
 *   x2 = A / (M+A)
 * @endcode
 *
 * Requirements:
 * @li T > 0
 * @li M > 0
 * @li 0 <= W <= (M/2)
 * @li -W <= A <= (M-2W)
 *
 *
 * <B>History</B><BR>
 * The "logicle" transform was introduced in the publication:
 * @li "A new 'Logicle' display method avoids deceptive effects of
 *   logarithmic scaling for low signals and compensated data", by
 *   David R. Parks, Mario Roederer, and Wayne A. Moore, Cytometry
 *   Part A, Volume 69A, Issue 6, pp. 541-551.
 *
 * The method is patented under U.S. Patent 6,954,722, held by The
 * Board of Trustees of the Leland Stanford Junior University (i.e.
 * "Stanford U."). However, the university does not enforce the patent
 * for non-profit academic purposes or for commercial use in the field
 * of flow cytometry.
 *
 * The logicle transform is a general form of an inverse hyperbolic
 * sine function (see the transform of that name). It is logarithmic
 * for large values, linear near zero, and symmetric around zero.
 * It changes smoothly between log and linear regions.
 */
class LogicleTransform final
    : public Transform
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
private:
    /**
     * The length of the computed Taylor series.
     *
     * A value of 16 is considered sufficient.
     */
    static const int TAYLOR_SERIES_LENGTH = 16;

    /**
     * The limit of precision for calculations.
     */
    static constexpr double EPSILON = std::numeric_limits<double>::epsilon( );



//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    // Provided values -------------------------------------------------
    /**
     * The "T" top of scale.
     *
     * The scale factor must be a positive number.
     */
    const double T;

    /**
     * The "A" bottom of scale.
     *
     * The scale factor must be a non-negative number.
     */
    const double A;

    /**
     * The "M" number of decades for the logarithmic space.
     *
     * The number of decades must be a positive number.
     */
    const double M;

    /**
     * The "W" number of decades for the linear space.
     *
     * The number of linear decades must be a non-negative number.
     */
    const double W;


    // Internal values -------------------------------------------------
    double a;
    double b;
    double c;
    double d;
    double f;
    double w;
    double x0;
    double x1;
    double x2;
    double xTaylor;
    double taylorSeries[TAYLOR_SERIES_LENGTH];



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new logicle transform with default equation variable
     * values that match the "FCS Transform".
     */
    LogicleTransform( )
        : LogicleTransform( 262144.0, 0.0, 4.5, 0.5 )
    {
    }

    /**
     * Constructs a new logicle transform with the given equation variable
     * values.
     *
     * @param[in] t
     *   The top of scale.
     * @param[in] a
     *   The bottom of scale.
     * @param[in] m
     *   The number of decades.
     * @param[in] w
     *   The number of linear decades.
     */
    LogicleTransform(
        const double t,
        const double a,
        const double m,
        const double w )
        : Transform( LOGICLE_TRANSFORM ),
        T( t ),
        A( a ),
        M( m ),
        W( w )
    {
        if ( t <= 0.0 )
            throw std::invalid_argument( "Invalid T must be positive." );
        if ( m <= 0.0 )
            throw std::invalid_argument( "Invalid M must be positive." );
        if ( w < 0.0 )
            throw std::invalid_argument( "Invalid W must be non-negative." );
        if ( w > (m/2.0) )
            throw std::invalid_argument( "Invalid W must be less than or equal to M/2." );
        if ( a < -w )
            throw std::invalid_argument( "Invalid A must be greater than or equal to -W." );
        if ( a > (m-2*w) )
            throw std::invalid_argument( "Invalid A must be less than or equal to (M-2W)." );

        this->initialize( );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~LogicleTransform( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the top-of-scale or "T" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getT( )
        const noexcept
    {
        return this->T;
    }

    /**
     * Returns the bottom-of-scale or "A" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getA( )
        const noexcept
    {
        return this->A;
    }

    /**
     * Returns the number of decades or "M" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getM( )
        const noexcept
    {
        return this->M;
    }

    /**
     * Returns the number of linear decades or "W" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getW( )
        const noexcept
    {
        return this->W;
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the transform.
     *
     * @return
     *   Returns a clone of this transform.
     */
    virtual Transform* clone( )
        const override
    {
        return new LogicleTransform( this->T, this->A, this->M, this->W );
    }
    // @}



//----------------------------------------------------------------------
// Transform.
//----------------------------------------------------------------------
public:
    /**
     * @name Transform
     */
    // @{
    using Transform::transform;

    /**
     * Transforms an event parameter value into a new value.
     *
     * @param[in] x
     *   The value to transform.
     *
     * @return
     *   Returns the transformed value.
     */
    virtual double transform( const double x )
        const noexcept override
    {
        return this->transformInternal( x );
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the transform.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string indent = "" )
        const noexcept override
    {
        return indent + "Logicle ( t=" +
            std::to_string( this->T ) + ", a=" +
            std::to_string( this->A ) + ", m=" +
            std::to_string( this->M ) + ", w=" +
            std::to_string( this->W ) + " )\n";
    }
    // @}



//----------------------------------------------------------------------
// Implementation.
//
// This implementation is loosely based upon C++ source provided by
// Wayne A. Moore of Stanford University.
//----------------------------------------------------------------------
private:
    /**
     * @name Implementation
     */
    // @{
    /**
     * Initializes the transform.
     *
     * Internal variables are computed based upon the public parameters.
     * The variables are then later used during transforms.
     *
     * @see ::seriesBiexponential()
     * @see ::solve()
     * @see ::transform()
     * @see ::transformInternal()
     */
    void initialize( )
        noexcept
    {
        // Use the transform's (T,A,M,W) values and compute intermediate
        // values.
        this->w  = W / (M + A);
        this->x2 = A / (M + A);
        this->x1 = this->x2 * this->w;
        this->x0 = this->x1 * 2.0;
        this->b  = (M + A) * std::log( 10.0 );

        this->d  = this->solve( this->b, this->w );

        const double exp_b_x1 = std::exp( this->b * this->x1 );
        const double exp_d_x1 = std::exp( this->d * this->x1 );

        const double c_a  = std::exp( this->x0 * (this->b + this->d) );
        const double mf_a = exp_b_x1 - c_a / exp_d_x1;

        this->a = this->T /
            ((std::exp( this->b ) - mf_a) - c_a / std::exp( this->d ));
        this->c = c_a * this->a;
        this->f = -mf_a * this->a;

        // To prevent round off problems from the formal definition,
        // use a Taylor series near x1.
        this->xTaylor = this->x1 + this->w / 4.0;

        double posCoef = this->a * exp_b_x1;
        double negCoef = -this->c * exp_d_x1;
        for ( int i = 0; i < this->TAYLOR_SERIES_LENGTH; ++i )
        {
            posCoef *= (this->b / (double)(i + 1));
            negCoef *= -(this->d / (double)(i + 1));
            this->taylorSeries[i] = posCoef * negCoef;
        }

        // Force the 1-st entry to zero as the exact result of the
        // logicle condition.
        this->taylorSeries[1] = 0.0;
    }

    /**
     * Solves the equation.
     */
    double solve( const double b, const double w )
        noexcept
    {
        // When w == 0, the logicle function is really an inverse
        // hyperbolic sine.
        if ( w == 0.0 )
            return b;

        // Compute the tolerance.
        const double tolerance = 2.0 * b * this->EPSILON;

        // Use "RTSAFE" from the book "Numerical Recipes".
        // Bracket the root.
        double d_lo = 0.0;
        double d_hi = b;

        // Bisection first step.
        double d = (d_lo + d_hi) / 2.0;
        double last_delta = d_hi - d_lo;
        double delta = 0.0;

        // Evaluate the function f(w,b) = 2 * (ln(d) - ln(b)) + w * (b + d)
        // and its derivative.
        double f_b = -2.0 * std::log(b) + w * b;
        double f   = 2.0 * std::log(d) + w * d + f_b;
        double last_f = std::numeric_limits<double>::quiet_NaN( );

        for ( int i = 1; i < 20; ++i )
        {
            // Compute the derivative.
            double df = (2.0 / d) + w;

            // If Newton's method would step outside the bracket
            // or if it isn't converging quickly enough.
            if ( (((d - d_hi) * df - f) * ((d - d_lo) * df - f) >= 0.0) ||
                (std::abs(1.9 * f) > std::abs(last_delta * df)) )
            {
                // Take a bisection step.
                delta = (d_hi - d_lo) / 2.0;
                d = d_lo + delta;
                if ( d == d_lo )
                    return d;           // Nothing changed, we're done.
            }
            else
            {
                // Otherwise take a Newton's method step.
                delta = f / df;
                const double t = d;
                d -= delta;
                if ( d == t )
                    return d;           // Nothing changed, we're done.
            }

            // If we've reached the desired precision, we're done.
            if ( std::abs(delta) < tolerance )
                return d;
            last_delta = delta;

            // Recompute the function.
            f = 2.0 * std::log(d) + w * d + f_b;
            if ( f == 0.0 || f == last_f )
                return d;               // Found root or not going to get closer
            last_f = f;

            // Update the bracketing interval.
            if ( f < 0.0 )
                d_lo = d;
            else
                d_hi = d;
        }

        // The loop above exceeded its maximum number of iterations
        // without solving the equation. Close enough?
        return d;
    }

    /**
     * Hmm.
     */
    double seriesBiexponential( const double scale )
        const noexcept
    {
        // Taylor series is around x1.
        const double x = scale - this->x1;

        // Note that taylor[1] should be identically zero according
        // to the Logicle condition so skip it here.
        double sum = this->taylorSeries[TAYLOR_SERIES_LENGTH - 1] * x;

        for ( int i = TAYLOR_SERIES_LENGTH - 2; i >= 2; --i )
            sum = (sum + this->taylorSeries[i]) * x;

        return (sum * x + this->taylorSeries[0]) * x;
    }

    /**
     * Computes the transformed value.
     *
     * @param[in] value
     *   The value to transform.
     *
     * @return
     *   The transformed value.
     */
    double transformInternal( double value )
        const noexcept
    {
        // Handle true zero specially.
        if ( value == 0.0 )
            return this->x1;

        // Reflect negative values.
        const bool wasNegative = value < 0.0;
        if ( wasNegative == true )
            value = -value;

        // Make an initial guess at the solution.
        double x;
        if ( value < this->f )
        {
            // Use linear approximation in the quasi-linear region.
            x = this->x1 + value / this->taylorSeries[0];
        }
        else
        {
            // Otherwise use an ordinary natural log.
            x = std::log( value / this->a ) / this->b;
        }

        // Try for double precision unless in the extended range.
        double tolerance = 3.0 * this->EPSILON;
        if ( x > 1.0 )
            tolerance = 3.0 * x * this->EPSILON;

        for ( int i = 0; i < 10; ++i )
        {
            // Compute the function and its first two derivatives.
            const double ae2bx  = this->a * std::exp(this->b * x);
            const double ce2mdx = this->c / std::exp(this->d * x);
            double y;
            if ( x < this->xTaylor )
                // near zero use the Taylor series
                y = this->seriesBiexponential( x ) - value;
            else
                // This formulation has better roundoff behavior
                y = (ae2bx + this->f) - (ce2mdx + value);

            const double abe2bx  = this->b * ae2bx;
            const double cde2mdx = this->d * ce2mdx;
            const double dy      = abe2bx + cde2mdx;
            const double ddy     = this->b * abe2bx - this->d * cde2mdx;

            // This is Halley's method with cubic convergence.
            const double delta = y / (dy * (1.0 - y * ddy / (2.0 * dy * dy)));
            x -= delta;

            // If we've reached the desired precision, we're done.
            if ( std::abs( delta ) < tolerance )
            {
                // Restore the original negative, if needed.
                if ( wasNegative == true )
                    return 2.0 * this->x1 - x;
                return x;
            }
        }

        // Did not converge? Close enough?
        return x;
    }
    // @}
};



/**
 * Describes a hyperlog transform.
 *
 * Like all transforms, the hyperlog transform remaps an
 * event parameter value "x" into a new value. This transform uses the
 * following equation:
 *
 * @code
 *   EN = a * e^(b*y) + c*y - f
 *   y = root(EH - x)
 * @endcode
 *
 * root() is a root finding algorithm (e.g. Newton's method).
 *
 * T is the "top of scale", or highest expected value for the parameter,
 * and M is the desired number of decades. e is "e" (i.e. 2.718281828).
 * W is the number of linear decades in the near-linear region of the
 * output. A is the number of additional decades of negative data values
 * to be included.
 *
 * The remaining variables are defined as:
 *
 * @code
 *   a = T / (e^b + ca - fa)
 *   b = (M + A) * ln(10)
 *   c = ca * a
 *   ca = e0 / w
 *   e0 = e^(b * x0)
 *   f = fa * a
 *   fa = e^(b*x1) + ca * x1
 *   w = W / (M+A)
 *   x0 = x2 * 2*w
 *   x1 = x2 * w
 *   x2 = A / (M+A)
 * @endcode
 *
 * Requirements:
 * @li T > 0
 * @li M > 0
 * @li 0 < W <= (M/2)
 * @li -W <= A <= (M-2W)
 */
class HyperlogTransform final
    : public Transform
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The "T" top of scale.
     *
     * The scale factor must be a positive number.
     */
    double t;

    /**
     * The "A" bottom of scale.
     *
     * The scale factor must be a non-negative number.
     */
    double a;

    /**
     * The "M" number of decades for the logarithmic space.
     *
     * The number of decades must be a positive number.
     */
    double m;

    /**
     * The "W" number of decades for the linear space.
     *
     * The number of linear decades must be a non-negative number.
     */
    double w;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new transform.
     *
     * @param[in] t
     *   The top of scale.
     * @param[in] a
     *   The bottom of scale.
     * @param[in] m
     *   The number of decades.
     */
    HyperlogTransform(
        const double t,
        const double a,
        const double m,
        const double w )
        : Transform( HYPERLOG_TRANSFORM ),
        t( t ),
        a( a ),
        m( m ),
        w( w )
    {
        if ( t <= 0.0 )
            throw std::invalid_argument( "Invalid T must be positive." );
        if ( m <= 0.0 )
            throw std::invalid_argument( "Invalid M must be positive." );
        if ( w <= 0.0 )
            throw std::invalid_argument( "Invalid W must be positive." );
        if ( w > (m/2.0) )
            throw std::invalid_argument( "Invalid W must be less than or equal to M/2." );
        if ( a < -w )
            throw std::invalid_argument( "Invalid A must be greater than or equal to -W." );
        if ( a > (m-2*w) )
            throw std::invalid_argument( "Invalid A must be less than or equal to (M-2W)." );
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~HyperlogTransform( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the top-of-scale or "T" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getT( )
        const noexcept
    {
        return this->t;
    }

    /**
     * Returns the bottom-of-scale or "A" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getA( )
        const noexcept
    {
        return this->a;
    }

    /**
     * Returns the number of decades or "M" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getM( )
        const noexcept
    {
        return this->m;
    }

    /**
     * Returns the number of linear decades or "W" parameter.
     *
     * @return
     *   Returns the parameter.
     */
    inline double getW( )
        const noexcept
    {
        return this->w;
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the transform.
     *
     * @return
     *   Returns a clone of this transform.
     */
    virtual Transform* clone( )
        const override
    {
        return new HyperlogTransform( this->t, this->a, this->m, this->w );
    }
    // @}



//----------------------------------------------------------------------
// Transform.
//----------------------------------------------------------------------
public:
    /**
     * @name Transform
     */
    // @{
    using Transform::transform;

    /**
     * Transforms an event parameter value into a new value.
     *
     * @param[in] x
     *   The value to transform.
     *
     * @return
     *   Returns the transformed value.
     */
    virtual double transform( const double x )
        const noexcept override
    {
        // PENDING.
        return 0.0;
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the transform.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string indent = "" )
        const noexcept override
    {
        return indent + "Hyperlog ( t=" +
            std::to_string( this->t ) + ", a=" +
            std::to_string( this->a ) + ", m=" +
            std::to_string( this->m ) + ", w=" +
            std::to_string( this->w ) + " )\n";
    }
    // @}
};



//----------------------------------------------------------------------
//
// Compensation.
//
//----------------------------------------------------------------------
/**
 * Indicates a compensation method.
 *
 * Emission spectral overlap of the fluorescence labels during data
 * acquisition requires that the acquired data be corrected, or
 * "compensated" to reduce the effects of this overlap, or "spillover".
 *
 * Compensation sweeps through all events in the table and updates
 * the values of indicated parameters. Compensation is typically indicated
 * with a spillover matrix that selects parameters to be corrected and
 * correction scaling factors.
 *
 * The compensation matrix and parameters are typically found within
 * either an FCS file's spillover matrix or within a Gating-ML file's
 * compensation description.
 */
#ifdef COMPENSATION_SUPPORTED
enum class CompensationMethod
{
    /**
     * Indicates that no compensation method has been defined.
     */
    UNCOMPENSATED,

    /**
     * Indicates the compensation spillover matrix should be found in
     * the associated FCS event file.
     */
    COMPENSATION_FROM_FCS_FILE,

    /**
     * Indicates the compensation details should be found within
     * a Gating-ML file.
     */
    COMPENSATION_FROM_GATINGML_FILE
};
#endif



//----------------------------------------------------------------------
//
// Gates and gate state.
//
//----------------------------------------------------------------------
/**
 * Defines an interface for optional gate state.
 *
 * Gate state may be attached to any gate. When methods change a gate,
 * the method calls the gate state's callback afterwards to let the
 * state object update any further state it may be maintaining.
 */
class GateStateInterface
{
//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
protected:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a gate state object.
     */
    GateStateInterface( )
    {
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GateStateInterface( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Callbacks.
//----------------------------------------------------------------------
public:
    /**
     * @name Callbacks
     */
    // @{
    /**
     * Updates state when a parameter is added to the additional clustering
     * list.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     */
    virtual void callbackAppendAdditionalClusteringParameter(
        const std::string name,
        std::shared_ptr<const Transform> transform )
        noexcept = 0;

    /**
     * Updates state when a child gate is added to a parent gate.
     *
     * @param[in] child
     *   The new child gate.
     */
    virtual void callbackAppendChild( std::shared_ptr<Gate> child )
        noexcept = 0;

    /**
     * Updates state when the additional clustering parameter list is cleared.
     */
    virtual void callbackClearAdditionalClusteringParameters( )
        noexcept = 0;

    /**
     * Updates state when a gate's children list is cleared.
     */
    virtual void callbackClearChildren( )
        noexcept = 0;

    /**
     * Updates state when a parameter is removed from the additional
     * clustering list.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     */
    virtual void callbackRemoveAdditionalClusteringParameter(
        const std::string name )
        noexcept = 0;

    /**
     * Updates state when a child is removed from a parent.
     *
     * @param[in] gate
     *   The gate to remove as a child.
     */
    virtual void callbackRemoveChild( const std::shared_ptr<Gate>& gate )
        noexcept = 0;

    /**
     * Updates state when an additional clustering parameter's transform
     * changes.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     */
    virtual void callbackSetAdditionalClusteringParameterTransform(
        const std::string name,
        std::shared_ptr<const Transform> transform )
        noexcept = 0;

    /**
     * Updates state when the description is set for the gate.
     *
     * @param[in] description
     *   The description.
     */
    virtual void callbackSetDescription( const std::string& description )
        noexcept = 0;

    /**
     * Updates state when the diagnostic notes rae set for the gate.
     *
     * @param[in] notes
     *   The notes.
     */
    virtual void callbackSetNotes( const std::string& notes )
        noexcept = 0;

#ifdef COMPENSATION_SUPPORTED
    /**
     * Updates state when the compensation method is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] method
     *   The compensation method.
     */
    virtual void callbackSetDimensionCompensationMethod(
        const DimensionIndex index,
        const CompensationMethod method )
        noexcept = 0;
#endif

    /**
     * Updates state when the parameter name is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] name
     *   The parameter name.
     */
    virtual void callbackSetDimensionParameterName(
        const DimensionIndex index,
        const std::string& name )
        noexcept = 0;

    /**
     * Updates state when the parameter transform is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] transform
     *   The transform transform.
     */
    virtual void callbackSetDimensionParameterTransform(
        const DimensionIndex index,
        std::shared_ptr<Transform> transform )
        noexcept = 0;

    /**
     * Updates state when the gating method is set for the gate.
     *
     * @param[in] method
     *   The gating method.
     */
    virtual void callbackSetGatingMethod( const GatingMethod method )
        noexcept = 0;

    /**
     * Updates state when the name is set for the gate.
     *
     * @param[in] name
     *   The name.
     */
    virtual void callbackSetName( const std::string& name )
        noexcept = 0;

    /**
     * Updates state when the original ID is set for the gate.
     *
     * @param[in] id
     *   The id.
     */
    virtual void callbackSetOriginalId( const std::string& id )
        noexcept = 0;

    /**
     * Updates state when the report priority is set for the gate.
     *
     * @param[in] priority
     *   The report priority.
     */
    virtual void callbackSetReportPriority( const uint32_t priority )
        noexcept = 0;

    /**
     * Updates state when a rectangle gate's min/max are changed.
     *
     * @param[in] index
     *   The dimension.
     * @param[in] minimum
     *   The new minimum.
     * @param[in] maximum
     *   The new maximum.
     */
    virtual void callbackSetRectangleMinimumMaximum(
        const DimensionIndex index,
        const double minimum,
        const double maximum )
        noexcept = 0;

    /**
     * Updates state when a polygon gate's vertex is appended.
     *
     * @param[in] index
     *   The vertex index.
     * @param[in] x
     *   The new X coordinate.
     * @param[in] y
     *   The new Y coordinate.
     */
    virtual void callbackAppendPolygonVertex(
        const VertexIndex index,
        const double x,
        const double y )
        noexcept = 0;

    /**
     * Updates state when a polygon gate's vertex list is cleared.
     */
    virtual void callbackClearPolygonVertices( )
        noexcept = 0;

    /**
     * Updates state when a polygon gate's vertex is removed.
     *
     * @param[in] index
     *   The vertex index.
     */
    virtual void callbackRemovePolygonVertex(
        const VertexIndex index )
        noexcept = 0;

    /**
     * Updates state when a polygon gate's vertex is changed.
     *
     * @param[in] index
     *   The vertex index.
     * @param[in] x
     *   The new X coordinate.
     * @param[in] y
     *   The new Y coordinate.
     */
    virtual void callbackSetPolygonVertex(
        const VertexIndex index,
        const double x,
        const double y )
        noexcept = 0;
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the number of events that have been marked as included
     * within the gate.
     *
     * If the gate has not been applied, this always the total number
     * of events.
     *
     * @return
     *   Returns the number of events included within the gate.
     *
     * @see ::getEventIncludedList()
     */
    virtual int findNumberOfIncludedEvents( )
        const noexcept = 0;

    /**
     * Returns the vector of event-included flags.
     *
     * The returned vector has one value per event. A value of 0 indicates
     * the event is NOT included within the gate, and a non-O (usually 1)
     * value indicates it IS included.
     *
     * @return
     *   The vector of event-included flags.
     */
    virtual const std::vector<uint8_t>& getEventIncludedList( )
        const noexcept = 0;

    /**
     * Returns the vector of event-included flags.
     *
     * The returned vector has one value per event. A value of 0 indicates
     * the event is NOT included within the gate, and a non-O (usually 1)
     * value indicates it IS included.
     *
     * @return
     *   The vector of event-included flags.
     */
    virtual std::vector<uint8_t>& getEventIncludedList( )
        noexcept = 0;

    /**
     * Returns the gate backing this gate state.
     *
     * @return
     *   Returns the gate.
     */
    virtual std::shared_ptr<const Gate> getGate( )
        const noexcept = 0;

    /**
     * Returns TRUE if the gate has already been applied to the state's events.
     *
     * @return
     *   Returns TRUE if applied.
     */
    virtual bool isApplied( )
        const noexcept = 0;

    /**
     * Returns TRUE if the indicated event is included within the gate's shape
     * or conditions.
     *
     * @param[in] index
     *   The event index.
     *
     * @return
     *  Returns TRUE if the event is included.
     */
    virtual bool isEventIncluded( const size_t index )
        const = 0;



    /**
     * Sets whether the gate has been applied to its events.
     *
     * It is the caller's responsability to know if the gate's event values
     * have been used for gating and the per-event inclusion flags are valid.
     *
     * @param[in] trueFalse
     *   TRUE if applied, FALSE if not.
     */
    virtual void setApplied( const bool trueFalse )
        noexcept = 0;

    /**
     * Sets whether the indicated event is included within the gate's shape
     * or conditions.
     *
     * @param[in] index
     *   The event index.
     * @param[in] trueFalse
     *   TRUE if included, FALSE if not.
     */
    virtual void setEventIncluded( const size_t index, const bool trueFalse ) = 0;
    // @}
};





/**
 * Defines the base class for optional gate state.
 *
 * This base class provides dummy implementations of all GateStateInterface
 * virtual methods.
 */
class GateStateBase
    : public GateStateInterface
{
//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
protected:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a gate state object.
     */
    GateStateBase( )
    {
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GateStateBase( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Callbacks.
//----------------------------------------------------------------------
public:
    /**
     * @name Callbacks
     */
    // @{
    /**
     * Updates state when a parameter is added to the additional clustering
     * list.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     */
    virtual void callbackAppendAdditionalClusteringParameter(
        const std::string name,
        std::shared_ptr<const Transform> transform )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a child gate is added to a parent gate.
     *
     * @param[in] child
     *   The new child gate.
     */
    virtual void callbackAppendChild( std::shared_ptr<Gate> child )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the additional clustering parameter list is cleared.
     */
    virtual void callbackClearAdditionalClusteringParameters( )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a gate's children list is cleared.
     */
    virtual void callbackClearChildren( )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a parameter is removed from the additional
     * clustering list.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     */
    virtual void callbackRemoveAdditionalClusteringParameter(
        const std::string name )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a child is removed from a parent.
     *
     * @param[in] gate
     *   The gate to remove as a child.
     */
    virtual void callbackRemoveChild( const std::shared_ptr<Gate>& gate )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when an additional clustering parameter's transform
     * changes.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     */
    virtual void callbackSetAdditionalClusteringParameterTransform(
        const std::string name,
        std::shared_ptr<const Transform> transform )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the description is set for the gate.
     *
     * @param[in] description
     *   The description.
     */
    virtual void callbackSetDescription( const std::string& description )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the diagnostic notes rae set for the gate.
     *
     * @param[in] notes
     *   The notes.
     */
    virtual void callbackSetNotes( const std::string& notes )
        noexcept override
    {
        // Do nothing.
    }

#ifdef COMPENSATION_SUPPORTED
    /**
     * Updates state when the compensation method is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] method
     *   The compensation method.
     */
    virtual void callbackSetDimensionCompensationMethod(
        const DimensionIndex index,
        const CompensationMethod method )
        noexcept override
    {
        // Do nothing.
    }
#endif

    /**
     * Updates state when the parameter name is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] name
     *   The parameter name.
     */
    virtual void callbackSetDimensionParameterName(
        const DimensionIndex index,
        const std::string& name )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the parameter transform is set for the gate.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] transform
     *   The transform transform.
     */
    virtual void callbackSetDimensionParameterTransform(
        const DimensionIndex index,
        std::shared_ptr<Transform> transform )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the gating method is set for the gate.
     *
     * @param[in] method
     *   The gating method.
     */
    virtual void callbackSetGatingMethod( const GatingMethod method )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the name is set for the gate.
     *
     * @param[in] name
     *   The name.
     */
    virtual void callbackSetName( const std::string& name )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the original ID is set for the gate.
     *
     * @param[in] id
     *   The id.
     */
    virtual void callbackSetOriginalId( const std::string& id )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when the report priority is set for the gate.
     *
     * @param[in] priority
     *   The report priority.
     */
    virtual void callbackSetReportPriority( const uint32_t priority )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a rectangle gate's min/max are changed.
     *
     * @param[in] index
     *   The dimension.
     * @param[in] minimum
     *   The new minimum.
     * @param[in] maximum
     *   The new maximum.
     */
    virtual void callbackSetRectangleMinimumMaximum(
        const DimensionIndex index,
        const double minimum,
        const double maximum )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a polygon gate's vertex is appended.
     *
     * @param[in] index
     *   The vertex index.
     * @param[in] x
     *   The new X coordinate.
     * @param[in] y
     *   The new Y coordinate.
     */
    virtual void callbackAppendPolygonVertex(
        const VertexIndex index,
        const double x,
        const double y )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a polygon gate's vertex list is cleared.
     */
    virtual void callbackClearPolygonVertices( )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a polygon gate's vertex is removed.
     *
     * @param[in] index
     *   The vertex index.
     */
    virtual void callbackRemovePolygonVertex(
        const VertexIndex index )
        noexcept override
    {
        // Do nothing.
    }

    /**
     * Updates state when a polygon gate's vertex is changed.
     *
     * @param[in] index
     *   The vertex index.
     * @param[in] x
     *   The new X coordinate.
     * @param[in] y
     *   The new Y coordinate.
     */
    virtual void callbackSetPolygonVertex(
        const VertexIndex index,
        const double x,
        const double y )
        noexcept override
    {
        // Do nothing.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the number of events that have been marked as included
     * within the gate.
     *
     * If the gate has not been applied, this always the total number
     * of events.
     *
     * @return
     *   Returns the number of events included within the gate.
     *
     * @see ::getEventIncludedList()
     */
    virtual int findNumberOfIncludedEvents( )
        const noexcept override
    {
        size_t n = 0;
        const auto inclusion = this->getEventIncludedList( );
        const auto nEvents = inclusion.size( );
        for ( size_t i = 0; i < nEvents; ++i )
            if ( inclusion[i] != 0 )
                ++n;

        return n;
    }

    /**
     * Returns the vector of event-included flags.
     *
     * The returned vector has one value per event. A value of 0 indicates
     * the event is NOT included within the gate, and a non-O (usually 1)
     * value indicates it IS included.
     *
     * @return
     *   The vector of event-included flags.
     */
    virtual const std::vector<uint8_t>& getEventIncludedList( )
        const noexcept = 0;

    /**
     * Returns the vector of event-included flags.
     *
     * The returned vector has one value per event. A value of 0 indicates
     * the event is NOT included within the gate, and a non-O (usually 1)
     * value indicates it IS included.
     *
     * @return
     *   The vector of event-included flags.
     */
    virtual std::vector<uint8_t>& getEventIncludedList( )
        noexcept = 0;

    /**
     * Returns the gate backing this gate state.
     *
     * @return
     *   Returns the gate.
     */
    virtual std::shared_ptr<const Gate> getGate( )
        const noexcept = 0;

    /**
     * Returns TRUE if the gate has already been applied to the state's events.
     *
     * @return
     *   Returns TRUE if applied.
     */
    virtual bool isApplied( )
        const noexcept = 0;

    /**
     * Returns TRUE if the indicated event is included within the gate's shape
     * or conditions.
     *
     * @param[in] index
     *   The event index.
     *
     * @return
     *  Returns TRUE if the event is included.
     */
    virtual bool isEventIncluded( const size_t index )
        const = 0;



    /**
     * Sets whether the gate has been applied to its events.
     *
     * It is the caller's responsability to know if the gate's event values
     * have been used for gating and the per-event inclusion flags are valid.
     *
     * @param[in] trueFalse
     *   TRUE if applied, FALSE if not.
     */
    virtual void setApplied( const bool trueFalse )
        noexcept = 0;

    /**
     * Sets whether the indicated event is included within the gate's shape
     * or conditions.
     *
     * @param[in] index
     *   The event index.
     * @param[in] trueFalse
     *   TRUE if included, FALSE if not.
     */
    virtual void setEventIncluded( const size_t index, const bool trueFalse ) = 0;
    // @}
};





/**
 * Defines the base class for flow cytometry gates.
 *
 * A gate classifies fluorescence cytometry events into those inside
 * the gate and those outside. Subclasses each have their own classification
 * methods, such as to classify events as inside or outside of a rectangle,
 * polygon, or ellipsoid.
 *
 * Gates are organized into gate trees. Each gate tree starts at a root
 * gate which may have zero or more children, each of which may have their
 * own children, and so on within an arbitrarily deep hierarchy.
 *
 * A parent gate classifies events as inside and outside, and a child gate
 * further classifies the events inside its parent's gate, dividing them
 * into those inside and outside the child's gate.
 *
 *
 * <B>Attributes</B><BR>
 * Every gate has the following attributes:
 *
 * @li A gate type.
 *
 * @li A list of zero or more children.
 *
 * @li A number of dimensions. Some types of gates support as little as one
 *     dimension, while the majority require two or more dimensions.
 *
 * @li An optional compensation method for each dimension. Compensation
 *     applies spectrum matrix or similar mechanism to account for cross-talk
 *     among the acquired parameters of an event.
 *
 * @li An optional transform for each dimension. A transform applies a
 *     mathematical expression to map the original value range to a more
 *     convenient range, such as to do linear or logarithmic scaling.
 *
 * @li An event parameter name for each dimension. Parameters are the columns
 *     of an event table read from an external event file, such as one in the
 *     ISAC FCS format.
 *
 *
 * <B>Gate children</B><BR>
 * Each gate has a list of zero or more children. Methods on this class
 * query the number of children and the specific child from the gate's
 * list of children. Gate children are ordered into a list, but the order
 * of gates in the list has no significance.
 * @code
 * // Get the number of children.
 * GateIndex nChildren = gate->getNumberOfChildren( );
 *
 * // Iterate over children.
 * for ( GateIndex i = 0; i < nChildren; ++i )
 * {
 *   auto child = gate->getChild( i );
 *   ...
 * }
 * @endcode
 *
 * Gates may be appended to and removed from the list.
 * @code
 * // Append child gate.
 * gate->appendChild( child );
 *
 * // Delete child gate.
 * gate->removeChild( gate );
 *
 * // Clear all children.
 * gate->clearChildren( );
 * @endcode
 */
class Gate
{
private:
    friend class GateTrees;

//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    // Gate management -------------------------------------------------
    /**
     * The unique ID of the gate.
     *
     * The ID is automatically generated as Gate objects are created.
     * IDs are assigned sequentially, starting at 0, and they do not repeat
     * until a 64-bit unsigned integer overflows.
     *
     * The ID cannot be set. However, an "original ID" may be set as an
     * alternate ID for the Gate. Such IDs are not guaranteed to be
     * unique and may come from reading in a file, such as Gating-ML.
     *
     * @see ::getId()
     * @see ::getOriginalId()
     * @see ::setOriginalId()
     */
    const GateId id;

    /**
     * The gate type.
     *
     * @see ::getGateType()
     */
    const GateType gateType;

    /**
     * The gate's children.
     *
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     * @see ::appendChild()
     * @see ::removeChild()
     */
    std::vector<std::shared_ptr<Gate>> children;

    /**
     * A boolean flag indicating if the gate is a child of another gate.
     *
     * @see ::appendChild()
     * @see ::removeChild()
     */
    bool hasParent;

protected:
    /**
     * Optional gate state.
     *
     * @see ::getState()
     * @see ::setState()
     */
    std::shared_ptr<GateStateInterface> state;



protected:
    // Gating-ML features ----------------------------------------------
    /**
     * The number of dimensions for the gate.
     *
     * For gates that have no dimensions, this is zero.
     *
     * @see ::getNumberOfDimensions()
     */
    const DimensionIndex numberOfDimensions;

#ifdef COMPENSATION_SUPPORTED
    /**
     * The method of compensating each dimension.
     *
     * The vector will have one entry for each dimension.
     */
    std::vector<CompensationMethod> compensationByDimension;
#endif

    /**
     * The optional scale transform for each dimension.
     *
     * The vector will have one entry for each dimension.
     *
     * @see ::getDimensionParameterTransform()
     */
    std::vector<std::shared_ptr<Transform>> transformByDimension;

    /**
     * The parameter name for each dimension.
     *
     * The vector will have one entry for each dimension.
     *
     * @see ::getDimensionParameterName()
     */
    std::vector<std::string> parameterNameByDimension;



private:
    // FlowGate-specific features --------------------------------------
    /**
     * The original possibly unique ID of the gate.
     *
     * The ID default to the empty string, but it may be set to an arbitrary
     * value. The caller should insure that the ID is unique, but this is not
     * guaranteed. A typical use is to set the ID to a value read from a
     * gating file, such as Gating-ML.
     *
     * @see ::getId()
     * @see ::getOriginalId()
     * @see ::setOriginalId()
     */
    std::string originalId;

    /**
     * The optional name of the gate.
     *
     * The name defaults to the empty string.
     *
     * In Gating-ML, this is found in in custom information for the gate:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <name>...</name>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * This also may be found by importing Cytobank features in custom
     * information for the gate:
     * @code
     * <data-type:custom_info>
     *   <cytobank>
     *     <name>...</name>
     *   </cytobank>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getName()
     */
    std::string name;

    /**
     * The optional description of the gate.
     *
     * In Gating-ML, this is found in custom information for the gate:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <description>...</description>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getDescription()
     */
    std::string description;

    /**
     * The optional diagnostic notes of the gate.
     *
     * Diagnostic notes are considered personal information and are removed
     * from the gate tree during de-identiication.
     *
     * In Gating-ML, this is found in custom information for the gate:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <notes>...</notes>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getNotes()
     */
    std::string notes;

    /**
     * The optional gating method for the gate.
     *
     * Events may be gated using the shape and size of the gate (e.g. a
     * rectangular gate) in one of several methods:
     *
     * @li EVENT_VALUE_GATING uses the event's value(s) to determine if the
     *   event is inside or outside of the gate. This is the default.
     *
     * @li DAFI_CLUSTER_CENTROID_GATING uses the DAFi clustering algorithm
     *   to find the centroids of event value clusters, then uses those
     *   centroids to determine if the cluster is inside or outside of the
     *   gate.
     *
     * Additional gating methods may be defined in the future.
     *
     * In Gating-ML, this is found in custom information for the gate:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <gatingMethodName>...</gatingMethodName>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getGatingMethod()
     * @see ::getGatingMethodName()
     */
    GatingMethod gatingMethod = EVENT_VALUE_GATING;

    /**
     * The optional reporting priority for this gate.
     *
     * While all gates are important in filtering events through a gate tree,
     * the output of some gates may be more important for human-review during
     * diagnosis. These priority gates are more likely to be included in a
     * final report.
     *
     * Specific priority levels defined here are:
     *
     * @li 0 = lowest priority and least likely to be included in a
     *   diagnosis report.
     *
     * @li 1 = higher priority and more likely to be included in a
     *   diagnosis report.
     *
     * This value is specifically an integer to allow for additional priority
     * levels in the future.
     *
     * By default, all gates have a high reporting priority.
     *
     * In Gating-ML, this is found in custom information for the gate:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <reportPriority>...</reportPriority>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getReportPriority()
     */
    uint32_t reportPriority = 1;

    /**
     * The optional list of additional parameters to use for clustering.
     *
     * When clustering is used within a gate, the results can be better
     * if a clustering is done using more parameters than those strictly
     * needed by the gate. These additional parameters are an optional
     * part of a gate.
     *
     * When clustering should be done, the gate's shape parameters
     * are always used (e.g. the X and Y parameters for a rectangle).
     * If additional parameters are needed, they are included in this
     * list, along with their optional transforms. All parameters
     * included in this list must exist in the event table being gated.
     *
     * If an event parameter does not need to be transformed, the transform
     * pointer for the parameter is nullptr. When it is not nullptr, the
     * transform used is independent of any other transform specified for
     * the same parameter elsewhere in the gate tree.
     *
     * In Gating-ML, this list is found in custom information for the gate:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <additionalClusteringParameters>...</additionalClusteringParameters>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::isAdditionalClustering()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameter()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameter()
     * @see ::removeAdditionalClusteringParameter()
     */
    std::vector<std::pair<std::string, std::shared_ptr<Transform>>> additionalClusteringParameters;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
    /**
     * @name Constructors
     */
    // @{
protected:
    /**
     * Constructs a new gate with no children.
     *
     * @param[in] gateType
     *   The type of gate. There is a one-to-one correspondence between
     *   gate types and gate subclasses. The gate type must match the
     *   subclass.
     * @param[in] numberOfDimensions
     *   The number of dimensions for the gate. For gate types that have
     *   no dimensions (e.g. BooleanGate), this should be zero.
     *
     * @see ::getGateType()
     */
    Gate(
        const GateType gateType,
        const DimensionIndex numberOfDimensions = 0 )
        noexcept
        : id( getNextId() ),
        gateType( gateType ),
        hasParent( false ),
        numberOfDimensions( numberOfDimensions )
    {
        // Intentionally allow unknown gate types in order to support
        // custom types outside of this API.
        //
        // Initialize attributes based on the number of dimensions.
        for ( DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
#ifdef COMPENSATION_SUPPORTED
            this->compensationByDimension.push_back(
                CompensationMethod.UNCOMPENSATED );
#endif
            this->transformByDimension.push_back( nullptr );
            this->parameterNameByDimension.push_back( "" );
        }
        this->additionalClusteringParameters.clear( );

        this->state = nullptr;
    }

public:
    /**
     * Constructs a copy of the given gate.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * The new gate has the same gate type and attributes as the given
     * gate, but a unique ID.
     *
     * @param[in] gate
     *   The gate to copy.
     */
    Gate( const Gate& gate )
        noexcept
        : id( getNextId() ),
        gateType( gate.gateType ),
        hasParent( false ),
        numberOfDimensions( gate.numberOfDimensions )
    {
        // Copy dimensions.
#ifdef COMPENSATION_SUPPORTED
        this->compensationByDimension = gate.compensationByDimension;
#endif
        this->transformByDimension = gate.transformByDimension;
        this->parameterNameByDimension = gate.parameterNameByDimension;

        // Copy annotation.
        this->name           = gate.name;
        this->description    = gate.description;
        this->notes          = gate.notes;
        this->originalId     = gate.originalId;
        this->gatingMethod   = gate.gatingMethod;
        this->reportPriority = gate.reportPriority;

        // Do not copy gate state.
        this->state = nullptr;

        this->additionalClusteringParameters.clear( );
        const size_t size = gate.additionalClusteringParameters.size( );
        for ( size_t i = 0; i < size; ++i )
        {
            const auto n = gate.additionalClusteringParameters[i].first;
            const auto t = gate.additionalClusteringParameters[i].second;
            if ( t == nullptr )
                this->additionalClusteringParameters.emplace_back(
                    n,
                    nullptr );
            else
                this->additionalClusteringParameters.emplace_back(
                    n,
                    t->clone());
        }

        // The children of the given gate are NOT copied.
    }

    /**
     * Constructs a copy of the given gate.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * The new gate has the same gate type and attributes as the given
     * gate, but a unique ID.
     *
     * @param[in] gate
     *   The gate to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is nullptr.
     */
    Gate( std::shared_ptr<const Gate> gate )
        : id( getNextId() ),
        gateType( (gate == nullptr) ?
            GateType::RECTANGLE_GATE : gate->gateType ),
        hasParent( false ),
        numberOfDimensions( gate->numberOfDimensions )
    {
        if ( gate == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate." );

        // Copy dimensions.
#ifdef COMPENSATION_SUPPORTED
        this->compensationByDimension = gate->compensationByDimension;
#endif
        this->transformByDimension = gate->transformByDimension;
        this->parameterNameByDimension = gate->parameterNameByDimension;

        // Copy annotation.
        this->name           = gate->name;
        this->originalId     = gate->originalId;
        this->description    = gate->description;
        this->notes          = gate->notes;
        this->gatingMethod   = gate->gatingMethod;
        this->reportPriority = gate->reportPriority;

        // Do not copy gate state.
        this->state = nullptr;

        this->additionalClusteringParameters.clear( );
        const size_t size = gate->additionalClusteringParameters.size( );
        for ( size_t i = 0; i < size; ++i )
        {
            const auto n = gate->additionalClusteringParameters[i].first;
            const auto t = gate->additionalClusteringParameters[i].second;
            if ( t == nullptr )
                this->additionalClusteringParameters.emplace_back(
                    n,
                    nullptr );
            else
                this->additionalClusteringParameters.emplace_back(
                    n,
                    t->clone());
        }

        // The children of the given gate are NOT copied.
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~Gate( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
    /**
     * @name Attributes
     */
    // @{
private:
    /**
     * Returns the next unique ID to use for gates.
     *
     * @return
     *   Returns the next ID.
     */
    inline GateId getNextId()
        noexcept
    {
        static GateId nextId = 0;
        return nextId++;
    }

public:
    /**
     * Returns the optional description for the gate.
     *
     * The description defaults to the empty string.
     *
     * @return
     *   Returns the description, or an empty string if no description
     *   has been set.
     *
     * @see ::setDescription()
     */
    inline const std::string& getDescription( )
        const noexcept
    {
        return this->description;
    }

#ifdef COMPENSATION_SUPPORTED
    /**
     * Returns the compensation method for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     *
     * @return
     *   Returns the compensation method.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getNumberOfDimensions()
     * @see ::setDimensionCompensationMethod()
     */
    inline CompensationMethod getDimensionCompensationMethod(
        const DimensionIndex index )
        const
    {
        // Validate.
        if ( index >= this->numberOfDimensions )
            throw std::out_of_range( "Invalid dimension index out of range." );

        return this->compensationByDimension[index];
    }
#endif

    /**
     * Returns the event data parameter name for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     *
     * @return
     *   Returns the parameter name.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getNumberOfDimensions()
     * @see ::setDimensionParameterName()
     */
    inline const std::string& getDimensionParameterName(
        const DimensionIndex index )
        const
    {
        // Validate.
        if ( index >= this->numberOfDimensions )
            throw std::out_of_range( "Invalid dimension index out of range." );

        return this->parameterNameByDimension[index];
    }

    /**
     * Returns the transform for the indicated dimension, if any.
     *
     * @param[in] index
     *   The dimension index.
     *
     * @return
     *   Returns the transform, or a nullptr pointer if there is none.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getNumberOfDimensions()
     * @see ::setDimensionParameterTransform()
     */
    inline std::shared_ptr<Transform> getDimensionParameterTransform(
        const DimensionIndex index )
        const
    {
        // Validate.
        if ( index >= this->numberOfDimensions )
            throw std::out_of_range( "Invalid dimension index out of range." );

        return this->transformByDimension[index];
    }

    /**
     * Returns the unique ID for the gate.
     *
     * The ID is set when the gate is created, it is guaranteed to
     * be unique (up to 2^32 gates), and it cannot be changed.
     *
     * @return
     *   Returns the unique ID.
     *
     * @see ::setId()
     */
    inline GateId getId( )
        const noexcept
    {
        return this->id;
    }

    /**
     * Returns the gate's type.
     *
     * The gate type has a one-to-one correspondence with the gate subclass
     * for known gate types. The type is set when a gate is constructed and
     * cannot be changed afterwards.
     *
     * The gate type may be used to test for a specific type of gate before
     * casting the gate's pointer to a gate of that type:
     * @code
     * if ( gate->getGateType( ) == RECTANGLE_GATE )
     * {
     *   auto rectangle = (RectangleGate*)(gate.get());
     *   ...
     * }
     * @endcode
     *
     * @return
     *   The gate type.
     *
     * @see ::getGateTypeName()
     */
    inline GateType getGateType( )
        const noexcept
    {
        return this->gateType;
    }

    /**
     * Returns the gate's type's name.
     *
     * The gate type has a one-to-one correspondence with the gate subclass
     * for known gate types. The type is set when a gate is constructed and
     * cannot be changed afterwards.
     *
     * The gate type name may be used to display information to a user:
     * @code
     * std::cout << "Gate type is: " << gate->getGateTypeName( ) << std::endl;
     * @endcode
     *
     * @return
     *   The gate type.
     *
     * @see ::getGateType()
     */
    inline std::string getGateTypeName( )
        const noexcept
    {
        return FlowGate::Gates::getGateTypeName( this->gateType );
    }

    /**
     * Returns the gating method.
     *
     * Events may be gated using the shape and size of the gate (e.g. a
     * rectangular gate) in one of several methods:
     *
     * @li EVENT_VALUE_GATING uses the event's value(s) to determine if the
     *   event is inside or outside of the gate.
     *
     * @li DAFI_CLUSTER_CENTROID_GATING uses the DAFi clustering algorithm
     *   to find the centroids of event value clusters, then uses those
     *   centroids to determine if the cluster is inside or outside of the
     *   gate.
     *
     * Additional methods may be defined in the future.
     *
     * @return
     *   Returns the gating method.
     *
     * @see ::getGatingMethodName()
     * @see ::setGatingMethod()
     */
    inline GatingMethod getGatingMethod( )
        const noexcept
    {
        return this->gatingMethod;
    }

    /**
     * Returns the gating method name.
     *
     * Events may be gated using the shape and size of the gate (e.g. a
     * rectangular gate) in one of several methods:
     *
     * @li EVENT_VALUE_GATING uses the event's value(s) to determine if the
     *   event is inside or outside of the gate.
     *
     * @li DAFI_CLUSTER_CENTROID_GATING uses the DAFi clustering algorithm
     *   to find the centroids of event value clusters, then uses those
     *   centroids to determine if the cluster is inside or outside of the
     *   gate.
     *
     * Additional methods may be defined in the future.
     *
     * @return
     *   Returns the gating method.
     *
     * @see ::getGatingMethod()
     * @see ::setGatingMethod()
     */
    inline std::string getGatingMethodName( )
        const noexcept
    {
        return FlowGate::Gates::getGatingMethodName( this->gatingMethod );
    }

    /**
     * Returns the optional name for the gate.
     *
     * @return
     *   Returns the name, or an empty string if no name has been set.
     *
     * @see ::setName()
     */
    inline const std::string& getName( )
        const noexcept
    {
        return this->name;
    }

    /**
     * Returns the optional diagnostic notes for the gate.
     *
     * The diagnostic notes defaults to the empty string.
     *
     * Diagnostic notes are considered personal information and are removed
     * from the gate tree during de-identiication.
     *
     * @return
     *   Returns the diagnostic notes, or an empty string if no diagnostic notes
     *   has been set.
     *
     * @see ::setNotes()
     */
    inline const std::string& getNotes( )
        const noexcept
    {
        return this->notes;
    }

    /**
     * Returns the number of dimensions.
     *
     * @return
     *   Returns the number of dimensions.
     */
    inline DimensionIndex getNumberOfDimensions( )
        const noexcept
    {
        return this->numberOfDimensions;
    }

    /**
     * Returns the optional original ID for the gate.
     *
     * The default original ID is an empty string. The ID may be set to
     * any value and it should be unique, though this is not enforced.
     * A typical use is to set the original ID to an ID found in an input
     * file, such as one using Gating-ML.
     *
     * See the getId() method for a guaranteed unique numeric ID for each
     * transform.
     *
     * @return
     *   Returns the original ID, or an empty string if no ID has been set.
     *
     * @see ::getId()
     * @see ::setOriginalId()
     */
    inline const std::string& getOriginalId( )
        const noexcept
    {
        return this->originalId;
    }

    /**
     * Returns the report priority.
     *
     * While all gates are important in filtering events through a gate tree,
     * the output of some gates may be more important for human-review during
     * diagnosis. These priority gates are more likely to be included in a
     * final report.
     *
     * Specific priority levels defined here are:
     *
     * @li 0 = lowest priority and least likely to be included in a
     *   diagnosis report.
     *
     * @li 1 = higher priority and more likely to be included in a
     *   diagnosis report.
     *
     * @return
     *   Returns the gate priority.
     *
     * @see ::setReportPriority()
     */
    inline uint32_t getReportPriority( )
        const noexcept
    {
        return this->reportPriority;
    }

    /**
     * Returns the optional gate state attached to the gate.
     *
     * @return
     *   The gate state.
     */
    inline std::shared_ptr<const GateStateInterface> getState( )
        const noexcept
    {
        return this->state;
    }

    /**
     * Returns the optional gate state attached to the gate.
     *
     * @return
     *   The gate state.
     */
    inline std::shared_ptr<GateStateInterface> getState( )
        noexcept
    {
        return this->state;
    }

    /**
     * Returns TRUE if the parameter is in use by one of the gate's dimensions.
     *
     * @param name
     *   The name of the parameter.
     *
     * @return
     *   Returns TRUE if the parameter is in use, and FALSE otherwise.
     */
    inline bool isDimensionParameter( const std::string name )
        const noexcept
    {
        for ( DimensionIndex i = 0; i < this->numberOfDimensions; ++i )
        {
            if ( name.compare( this->parameterNameByDimension[i] ) == 0 )
                return true;
        }
        return false;
    }



    /**
     * Sets the optional description for the gate.
     *
     * @param[in] description
     *   The description. An empty string clears the description.
     *
     * @see ::getDescription()
     */
    inline void setDescription( const std::string& description )
        noexcept
    {
        if ( this->description == description )
            return;

        this->description = description;
        if ( this->state != nullptr )
            this->state->callbackSetDescription( description );
    }

#ifdef COMPENSATION_SUPPORTED
    /**
     * Sets the compensation method for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] method
     *   The compensation method.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getNumberOfDimensions()
     * @see ::getDimensionCompensationMethod()
     */
    inline void setDimensionCompensationMethod(
        const DimensionIndex index,
        const CompensationMethod method )
    {
        // Validate.
        if ( index >= this->numberOfDimensions )
            throw std::out_of_range( "Invalid dimension index out of range." );

        this->compensationByDimension[index] = method;
        if ( this->state != nullptr )
            this->state->callbackSetDimensionCompensationMethod(
                index,
                method );
    }
#endif

    /**
     * Sets the event data parameter name for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] name
     *   The parameter name. An empty string clears the name, but leaves
     *   the gate illdefined.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getNumberOfDimensions()
     * @see ::getDimensionParameterName()
     */
    inline void setDimensionParameterName(
        const DimensionIndex index,
        const std::string& name )
    {
        // Validate.
        if ( index >= this->numberOfDimensions )
            throw std::out_of_range( "Invalid dimension index out of range." );

        if ( this->parameterNameByDimension[index] == name )
            return;

        this->parameterNameByDimension[index] = name;
        if ( this->state != nullptr )
            this->state->callbackSetDimensionParameterName( index, name );
    }

    /**
     * Sets the transform for the indicated dimension.
     *
     * Setting the transform to a nullptr pointer clears the transform.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] transform
     *   The transform transform. A NULL pointer clears the transform.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getNumberOfDimensions()
     * @see ::getDimensionParameterTransform()
     */
    inline void setDimensionParameterTransform(
        const DimensionIndex index,
        std::shared_ptr<Transform> transform )
    {
        // Validate.
        if ( index >= this->numberOfDimensions )
            throw std::out_of_range( "Invalid dimension index out of range." );

        this->transformByDimension[index] = transform;
        if ( this->state != nullptr )
            this->state->callbackSetDimensionParameterTransform( index, transform );
    }

    /**
     * Sets the gating method.
     *
     * Events may be gated using the shape and size of the gate (e.g. a
     * rectangular gate) in one of several methods:
     *
     * @li EVENT_VALUE_GATING uses the event's value(s) to determine if the
     *   event is inside or outside of the gate.
     *
     * @li DAFI_CLUSTER_CENTROID_GATING uses the DAFi clustering algorithm
     *   to find the centroids of event value clusters, then uses those
     *   centroids to determine if the cluster is inside or outside of the
     *   gate.
     *
     * Additional methods may be defined in the future.
     *
     * @param[in] method
     *   The gating method.
     *
     * @see ::getGatingMethod()
     */
    inline void setGatingMethod( const GatingMethod method )
        noexcept
    {
        if ( this->gatingMethod == method )
            return;

        this->gatingMethod = method;
        if ( this->state != nullptr )
            this->state->callbackSetGatingMethod( method );
    }

    /**
     * Sets the optional name for the gate.
     *
     * @param[in] name
     *   The name. An empty string clears the name.
     *
     * @see ::getName()
     */
    inline void setName( const std::string& name )
        noexcept
    {
        if ( this->name == name )
            return;

        this->name = name;
        if ( this->state != nullptr )
            this->state->callbackSetName( name );
    }

    /**
     * Sets the optional diagnostic notes for the gate.
     *
     * Diagnostic notes are considered personal information and are removed
     * from the gate tree during de-identiication.
     *
     * @param[in] notes
     *   The notes. An empty string clears the notes.
     *
     * @see ::getNotes()
     */
    inline void setNotes( const std::string& notes )
        noexcept
    {
        if ( this->notes == notes )
            return;

        this->notes = notes;
        if ( this->state != nullptr )
            this->state->callbackSetNotes( notes );
    }

    /**
     * Sets the optional original ID for the gate.
     *
     * The ID may be set to any value and it should be unique, though this
     * method does not enforce this. A typical use is to set the original ID
     * to an ID found in an input file, such as one using Gating-ML.
     *
     * See the getId() method for a guaranteed unique numeric ID for each
     * transform.
     *
     * @param[in] id
     *   The id.
     *
     * @see ::getId()
     * @see ::getOriginalId()
     */
    inline void setOriginalId( const std::string& id )
        noexcept
    {
        if ( this->originalId == id )
            return;

        this->originalId = id;
        if ( this->state != nullptr )
            this->state->callbackSetOriginalId( id );
    }

    /**
     * Sets the report priority.
     *
     * Specific priority levels defined here are:
     *
     * @li 0 = lowest priority and least likely to be included in a
     *   diagnosis report.
     *
     * @li 1 = higher priority and more likely to be included in a
     *   diagnosis report.
     *
     * The value range of the priority is not checked, though it should
     * be one of the above.
     *
     * @param[in] priority
     *   The report priority.
     *
     * @see ::getReportPriority()
     */
    inline void setReportPriority( const uint32_t priority )
        noexcept
    {
        if ( this->reportPriority == priority )
            return;

        this->reportPriority = priority;
        if ( this->state != nullptr )
            this->state->callbackSetReportPriority( priority );
    }

    /**
     * Sets the optional gate state attached to the gate.
     *
     * Setting the state to a nullptr clears the state.
     *
     * The state object receives callbacks for all operations that change
     * the gate.
     *
     * @param[in] state
     *   The gate state.
     */
    inline void setState( std::shared_ptr<GateStateInterface> state )
        noexcept
    {
        this->state = state;
    }
    // @}



//----------------------------------------------------------------------
// Attributes for additional clustering.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes for additional clustering
     */
    // @{
    /**
     * Adds to the additional clustering parameters list.
     *
     * If the parameter name is already in use in the list, that entry is
     * removed and a new entry added using the same name and the given
     * transform.  Otherwise a new entry is added using the given name
     * and transform.
     *
     * The order of entries in the list is not defined or relevant.
     *
     * The list should not include any parameters used by the gate itself,
     * however this is not checked.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     *
     * @returns
     *   Returns the new parameter's index.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter name is empty, or if the
     *   gate does not support additional clustering parameters.
     *
     * @see ::clearAdditionalClusteringParameters()
     * @see ::removeAdditionalClusteringParameter()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline size_t appendAdditionalClusteringParameter(
        const std::string name,
        std::shared_ptr<const Transform> transform )
    {
        // Validate.
        if ( this->supportsAdditionalClusteringParameters( ) == false )
            throw std::invalid_argument(
                "Additional clustering parameters not supported for the gate type." );
        if ( name.empty( ) == true )
            throw std::invalid_argument( "Invalid empty parameter name." );

        if ( this->isDimensionParameter( name ) == true )
            throw std::invalid_argument(
                "Parameter is already in use by a gate dimension." );

        if ( this->isAdditionalClusteringParameter( name ) == true )
            throw std::invalid_argument(
                "Parameter is already in use as an additional clustering parameter." );

        // Append a new entry for the parameter.
        if ( transform == nullptr )
            this->additionalClusteringParameters.emplace_back(
                name,
                nullptr );
        else
            this->additionalClusteringParameters.emplace_back(
                name,
                transform->clone( ) );

        if ( this->state != nullptr )
            this->state->callbackAppendAdditionalClusteringParameter(
                name,
                transform );

        return this->additionalClusteringParameters.size( ) - 1;
    }

    /**
     * Sets the additional clustering parameter transform.
     *
     * If the parameter name is not already in use in the list, an exception
     * is thrown. Otherwise, the entry's transform is updated.
     *
     * @param[in] name
     *   The non-empty name of a parameter.
     * @param[in] transform
     *   The transform to use for the parameter.
     *
     * @returns
     *   Returns the new parameter's index.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter name is empty, or if the
     *   gate does not support additional clustering parameters.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::removeAdditionalClusteringParameter()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterIndex()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline size_t setAdditionalClusteringParameterTransform(
        const std::string name,
        std::shared_ptr<const Transform> transform )
    {
        // Validate.
        size_t index = 0;
        try
        {
            index = this->getAdditionalClusteringParameterIndex( name );
        }
        catch ( ... )
        {
            throw std::invalid_argument(
                "Parameter is not an additional clustering parameter." );
        }

        // Update the entry.
        if ( transform == nullptr )
        {
            this->additionalClusteringParameters.at( index ).second =
                nullptr;
        }
        else
        {
            this->additionalClusteringParameters.at( index ).second.reset(
                transform->clone( ) );
        }

        if ( this->state != nullptr )
            this->state->callbackSetAdditionalClusteringParameterTransform(
                name,
                transform );
        return index;
    }

    /**
     * Clears the additional clustering parameters list.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::removeAdditionalClusteringParameter()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline void clearAdditionalClusteringParameters( )
        noexcept
    {
        if ( this->additionalClusteringParameters.size( ) == 0 )
            return;

        this->additionalClusteringParameters.clear( );
        if ( this->state != nullptr )
            this->state->callbackClearAdditionalClusteringParameters( );
    }

    /**
     * Returns the index of the addiitional clustering parameter.
     *
     * @param[in] name
     *   The name of a parameter that may be in the list.
     *
     * @return
     *   Returns index of the additional clustering parameter is in the list.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate does include the additional
     *   clustering parameter.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline size_t getAdditionalClusteringParameterIndex(
        const std::string name )
    {
        const size_t n = this->additionalClusteringParameters.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            const auto pname = this->additionalClusteringParameters[i].first;
            if ( pname == name )
                return i;
        }

        throw std::invalid_argument(
            "Additional clustering parameter not found." );
    }

    /**
     * Returns the indicated additional clustering parameters list entry name.
     *
     * @param[in] index
     *   The list index.
     *
     * @return
     *   Returns the parameter name.
     *
     * @throws std::out_of_range
     *   Throws an exception if the list index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the gate does not support additional
     *   clustering parameters.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::removeAdditionalClusteringParameter()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline const std::string &getAdditionalClusteringParameterName(
        const size_t index )
        const
    {
        // Validate.
        if ( this->supportsAdditionalClusteringParameters( ) == false )
            throw std::invalid_argument(
                "Additional clustering parameters not supported for the gate type." );
        if ( index >= this->additionalClusteringParameters.size( ) )
            throw std::out_of_range(
                "Invalid additional clustering parameters list index out of range." );

        return this->additionalClusteringParameters[index].first;
    }

    /**
     * Returns the indicated additional clustering parameters list entry
     * transform.
     *
     * @param[in] index
     *   The list index.
     *
     * @return
     *   Returns the transform. The value may be nullptr if no transform is
     *   required for the parameter.
     *
     * @throws std::out_of_range
     *   Throws an exception if the list index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the gate does not support additional
     *   clustering parameters.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::removeAdditionalClusteringParameter()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline std::shared_ptr<const Transform> getAdditionalClusteringParameterTransform(
        const size_t index )
        const
    {
        // Validate.
        if ( this->supportsAdditionalClusteringParameters( ) == false )
            throw std::invalid_argument(
                "Additional clustering parameters not supported for the gate type." );
        if ( index >= this->additionalClusteringParameters.size( ) )
            throw std::out_of_range(
                "Invalid additional clustering parameters list index out of range." );

        return this->additionalClusteringParameters[index].second;
    }

    /**
     * Returns the additional clustering parameters list.
     *
     * When clustering is used within a gate, the results can be better
     * if a additional clustering parameters are used, along with the
     * gate shape's parameters (e.g. the X and Y parameters of a rectangle
     * gate).
     *
     * When clustering should be done, the returned list provides the
     * names of additional event parameters and transforms for those
     * paramters. All parameters included in the list must exist in the
     * event table being gated.
     *
     * If an event parameter does not need to be transformed, the transform
     * for the parameter is nullptr. When it is not nullptr, the transform
     * used is independent of any other transform specified for the same
     * parameter elsewhere in the gate tree.
     *
     * The returned list contains pairs that include the name of a parameter
     * in an event file and the transform to apply to that parameter.
     *
     * The order of entries in the list is not defined or relevant.
     *
     * @return
     *   Returns a vector of pairs. The first element of each pair is the
     *   parameter name, and the second is the transform for that parameter.
     *   The parameter name cannot be empty, but the transform can be a nullptr
     *   if no transform is required for the parameter.
     *
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline const std::vector<std::pair<std::string, std::shared_ptr<Transform>>> &getAdditionalClusteringParameters( )
        const noexcept
    {
        return this->additionalClusteringParameters;
    }

    /**
     * Returns the number of additional clustering parameters.
     *
     * @return
     *   Returns the list size.
     *
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline size_t getNumberOfAdditionalClusteringParameters( )
        const noexcept
    {
        return this->additionalClusteringParameters.size( );
    }

    /**
     * Returns TRUE if additional clustering parameters have been defined.
     *
     * @return
     *   Returns TRUE if the additional clustering parameters list is
     *   not empty.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline bool hasAdditionalClusteringParameters( )
        const noexcept
    {
        return (this->additionalClusteringParameters.empty( ) == false);
    }

    /**
     * Returns TRUE if the named parameter is used in additional clustering.
     *
     * @param[in] name
     *   The name of a parameter that may be in the list.
     *
     * @return
     *   Returns TRUE if the additional clustering parameter is in the list.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline bool isAdditionalClusteringParameter( const std::string name )
    {
        const size_t n = this->additionalClusteringParameters.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            const auto pname = this->additionalClusteringParameters[i].first;
            if ( pname == name )
                return true;
        }
        return false;
    }

    /**
     * Removes the parameter's entry from the additional clustering
     * parameters list.
     *
     * @param[in] name
     *   The name of a parameter in the list.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the parameter name is empty or not found
     *   in the list, or if the gate does not support additional clustering
     *   parameters.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline void removeAdditionalClusteringParameter( const std::string name )
    {
        // Validate.
        if ( this->supportsAdditionalClusteringParameters( ) == false )
            throw std::invalid_argument(
                "Additional clustering parameters not supported for the gate type." );
        if ( name.empty( ) == true )
            throw std::invalid_argument( "Invalid empty parameter name." );

        const size_t n = this->additionalClusteringParameters.size( );
        for ( size_t i = 0; i < n; ++i )
        {
            const auto pname = this->additionalClusteringParameters[i].first;
            if ( pname == name )
            {
                this->additionalClusteringParameters.erase(
                    this->additionalClusteringParameters.begin( ) + i );
                if ( this->state != nullptr )
                    this->state->callbackRemoveAdditionalClusteringParameter(
                        name );
                return;
            }
        }

        throw std::invalid_argument(
            "Parameter name not found in additional clustering parameters list." );
    }

    /**
     * Removes the indicated entry from the additional clustering
     * parameters list.
     *
     * @param[in] index
     *   The list index.
     *
     * @throws std::out_of_range
     *   Throws an exception if the list index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the gate does not support additional
     *   clustering parameters.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     * @see ::isAdditionalClusteringParameters()
     * @see ::supportsAdditionalClusteringParameters()
     */
    inline void removeAdditionalClusteringParameter( const size_t index )
    {
        // Validate.
        if ( this->supportsAdditionalClusteringParameters( ) == false )
            throw std::invalid_argument(
                "Additional clustering parameters not supported for the gate type." );
        if ( index >= this->additionalClusteringParameters.size( ) )
            throw std::out_of_range(
                "Invalid additional clustering parameters list index out of range." );

        const std::string name = this->additionalClusteringParameters[index].first;

        this->additionalClusteringParameters.erase(
            this->additionalClusteringParameters.begin( ) + index );

        if ( this->state != nullptr )
            this->state->callbackRemoveAdditionalClusteringParameter( name );
    }

    /**
     * Returns TRUE if additional clustering parameters are supported
     * by the gate.
     *
     * The default implementation returns TRUE. Subclasses may return
     * FALSE if they do not support the feature.
     *
     * @return
     *   Returns TRUE if additional clustering parameters are supported.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     */
    virtual bool supportsAdditionalClusteringParameters( )
        const noexcept
    {
        return true;
    }
    // @}



//----------------------------------------------------------------------
// Copy & clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the gate and all of its children.
     *
     * @return
     *   Returns a clone of this gate.
     */
    virtual Gate* clone( ) const = 0;

protected:
    /**
     * Clones this gate's children and adds them to the given gate.
     *
     * @param[in,out] newParent
     *   The gate to copy into.
     *
     * @return
     *   Returns the given gate.
     */
    inline Gate* cloneChildren( Gate*const newParent )
        const
    {
        // Copy children.
        const auto nChildren = this->children.size( );
        for ( GateIndex i = 0; i < nChildren; ++i )
        {
            // Clone the child.
            std::shared_ptr<Gate> newChild;
            newChild.reset( this->getChild( i )->clone( ) );

            newParent->appendChild( newChild );
        }

        return newParent;
    }
    // @}



//----------------------------------------------------------------------
// Tree get, append, and remove.
//----------------------------------------------------------------------
public:
    /**
     * @name Tree get, append, and remove
     */
    // @{
    /**
     * Appends a gate to the list of children.
     *
     * @param[in] gate
     *   The gate to add as a child.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the given gate is nullptr or if the gate is
     *   already the child of another parent.
     *
     * @see ::clearChildren()
     * @see ::removeChild()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    virtual void appendChild( const std::shared_ptr<Gate>& gate )
    {
        // Validate.
        if ( gate == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate." );
        if ( gate->hasParent == true )
            throw std::invalid_argument(
                "Invalid append of a gate that is already a child of another gate." );

        // Add the child to the children list.
        this->children.push_back( gate );
        gate->hasParent = true;

        if ( this->state != nullptr )
            this->state->callbackAppendChild( gate );
    }

    /**
     * Clears the list of children.
     *
     * @see ::removeChild()
     * @see ::getNumberOfChildren()
     */
    virtual void clearChildren( )
        noexcept
    {
        const auto n = this->children.size( );
        for ( GateIndex i = 0; i < n; ++i )
            this->children[i]->hasParent = false;
        this->children.clear( );

        if ( this->state != nullptr )
            this->state->callbackClearChildren( );
    }

    /**
     * Returns TRUE if the gate has no parent gate and is therefore a
     * root gate.
     *
     * @return bool
     *   Returns TRUE if the gate has a parent.
     */
    inline bool isRoot( )
        const noexcept
    {
        return !this->hasParent;
    }

    /**
     * Returns the number of children.
     *
     * A zero is returned if the gate has no children.
     *
     * @return
     *   Returns the number of children.
     *
     * @see ::appendChild()
     * @see ::clearChildren()
     * @see ::removeChild()
     * @see ::getChild()
     */
    inline GateIndex getNumberOfChildren( )
        const noexcept
    {
        return this->children.size( );
    }

    /**
     * Returns the indicated child.
     *
     * @param[in] index
     *   The child list index.
     *
     * @return
     *   Returns the child.
     *
     * @throws std::out_of_range
     *   Throws an exception if the child index is out of range.
     *
     * @see ::getNumberOfChildren()
     */
    inline std::shared_ptr<const Gate> getChild( const GateIndex index )
        const
    {
        // Validate.
        if ( index >= this->children.size( ) )
            throw std::out_of_range( "Invalid child index out of range." );

        return this->children[index];
    }

    /**
     * Returns the indicated child.
     *
     * @param[in] index
     *   The child list index.
     *
     * @return
     *   Returns the child.
     *
     * @throws std::out_of_range
     *   Throws an exception if the child index is out of range.
     *
     * @see ::getNumberOfChildren()
     */
    inline std::shared_ptr<Gate> getChild( const GateIndex index )
    {
        // Validate.
        if ( index >= this->children.size( ) )
            throw std::out_of_range( "Invalid child index out of range." );

        return this->children[index];
    }

    /**
     * Removes a gate from the list of children.
     *
     * @param[in] index
     *   The index into the gate's list of children for the gate to
     *   remove as a child.
     *
     * @return
     *   Returns the children list index of the removed child.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr pointer or is not a
     *   child of this gate.
     *
     * @see ::appendChild()
     * @see ::clearChildren()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    virtual GateIndex removeChild( const GateIndex index )
    {
        // Validate.
        if ( index >= this->children.size( ) )
            throw std::out_of_range( "Invalid child index out of range." );

        // Remove the child from the children list.
        auto gate = this->children[index];
        this->children.erase( this->children.begin( ) + index );
        gate->hasParent = false;

        if ( this->state != nullptr )
            this->state->callbackRemoveChild( gate );

        return index;
    }

    /**
     * Removes a gate from the list of children.
     *
     * @param[in] gate
     *   The gate to remove as a child.
     *
     * @return
     *   Returns the children list index of the removed child.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr pointer or is not a
     *   child of this gate.
     *
     * @see ::appendChild()
     * @see ::clearChildren()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    virtual GateIndex removeChild( const std::shared_ptr<Gate>& gate )
    {
        // Validate.
        if ( gate == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate." );
        if ( gate->hasParent == false )
            throw std::invalid_argument(
                "Invalid remove of a gate that is not a child." );

        // Remove the child from the children list.
        const auto n = this->children.size( );
        for ( GateIndex i = 0; i < n; ++i )
        {
            if ( this->children[i] == gate )
            {
                this->children.erase( this->children.begin( ) + i );
                gate->hasParent = false;

                if ( this->state != nullptr )
                    this->state->callbackRemoveChild( gate );
                return i;
            }
        }

        throw std::invalid_argument(
            "Invalid remove of a gate that is not a child of this gate." );
    }
    // @}



//----------------------------------------------------------------------
// Tree search.
//----------------------------------------------------------------------
public:
    /**
     * @name Tree search
     */
    // @{
    /**
     * Returns a list including all of this gates descendents.
     *
     * This gate is not included in the list.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children.
     */
    inline std::vector<std::shared_ptr<const Gate>> findDescendentGates( )
        const noexcept
    {
        std::vector<std::shared_ptr<const Gate>> gates;
        const GateIndex nChildren = this->children.size( );
        for ( GateIndex i = 0; i < nChildren; ++i )
        {
            gates.push_back( this->children[i] );
            const auto descendents = this->children[i]->findDescendentGates( );
            const auto n = descendents.size( );
            for ( GateIndex j = 0; j < n; ++j )
                gates.push_back( descendents[j] );
        }

        return gates;
    }

    /**
     * Returns a list including all of this gates descendents.
     *
     * This gate is not included in the list.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children.
     */
    inline std::vector<std::shared_ptr<Gate>> findDescendentGates( )
        noexcept
    {
        std::vector<std::shared_ptr<Gate>> gates;
        const GateIndex nChildren = this->children.size( );
        for ( GateIndex i = 0; i < nChildren; ++i )
        {
            gates.push_back( this->children[i] );
            const auto descendents = this->children[i]->findDescendentGates( );
            const auto n = descendents.size( );
            for ( GateIndex j = 0; j < n; ++j )
                gates.push_back( descendents[j] );
        }

        return gates;
    }

private:
    /**
     * Returns a list of all root gates and their descendents, including
     * parent indexing.
     *
     * The returned vector contains pairs where each pair as:
     * @li A vector index for the parent gate.
     * @li The gate.
     *
     * The parent vector index may be used to get the parent gate for an
     * entry:
     * @code
     * const auto& [parentIndex, gate] = vector[i];
     * const auto& [grandParentIndex, parentGate] = vector[parentIndex];
     * @endcode
     *
     * Root gates have parent indexes that are the same as their own index.
     *
     * @param[in] parentIndex
     *   The index of the root gate.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children. The list is not valid until
     *   the root gate is included at the start of the list.
     *
     * @TODO Do we need this method?
     */
    inline void findDescendentGatesWithParentIndexes(
        std::vector<std::pair<GateIndex,std::shared_ptr<const Gate>>>& gates,
        const GateIndex parentIndex )
        const noexcept
    {
        for ( const auto& child : this->children )
        {
            const GateIndex childIndex = gates.size( );
            gates.push_back( std::make_pair( parentIndex, child ) );

            child->findDescendentGatesWithParentIndexes( gates, childIndex );
        }
    }

    /**
     * Returns a list of all root gates and their descendents, including
     * parent indexing.
     *
     * The returned vector contains pairs where each pair as:
     * @li A vector index for the parent gate.
     * @li The gate.
     *
     * The parent vector index may be used to get the parent gate for an
     * entry:
     * @code
     * const auto& [parentIndex, gate] = vector[i];
     * const auto& [grandParentIndex, parentGate] = vector[parentIndex];
     * @endcode
     *
     * Root gates have parent indexes that are the same as their own index.
     *
     * @param[in] parentIndex
     *   The index of the root gate.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children. The list is not valid until
     *   the root gate is included at the start of the list.
     */
    inline void findDescendentGatesWithParentIndexes(
        std::vector<std::pair<GateIndex,std::shared_ptr<Gate>>>& gates,
        GateIndex parentIndex )
        const noexcept
    {
        for ( auto& child : this->children )
        {
            const GateIndex childIndex = gates.size( );
            gates.push_back( std::make_pair( parentIndex, child ) );

            child->findDescendentGatesWithParentIndexes( gates, childIndex );
        }
    }

public:
    /**
     * Returns the number of descendent gates.
     *
     * This method recurses downward through the gate's children and
     * returns total number of gates, including this gate.
     *
     * @return
     *   Returns the number of gates.
     */
    inline GateIndex findNumberOfDescendentGates( )
        const noexcept
    {
        GateIndex sum = 1;
        for ( GateIndex i = 0; i < this->children.size( ); ++i )
            sum += this->children[i]->findNumberOfDescendentGates( );
        return sum;
    }

    /**
     * Returns the number of descendent transforms.
     *
     * This method recurses downward through the gate's children and
     * returns total number of transforms, including any used by this gate.
     *
     * @return
     *   Returns the number of transforms.
     */
    inline TransformIndex findNumberOfDescendentTransforms( )
        const noexcept
    {
        TransformIndex sum = 0;
        for ( DimensionIndex i = 0; i < this->numberOfDimensions; ++i )
            if ( this->transformByDimension[i] != nullptr )
                ++sum;

        const GateIndex nChildren = this->children.size( );
        for ( GateIndex i = 0; i < nChildren; ++i )
            sum += this->children[i]->findNumberOfDescendentTransforms( );
        return sum;
    }

    /**
     * Returns the transform, if any, with the given ID.
     *
     * This method looks through the gate's dimensions additional clustering
     * parameters for a transform with the given ID. The transform is
     * returned, or a nullptr if no transform is found.
     *
     * @param[in] id
     *   The transform ID to look up.
     *
     * @return
     *   Returns the transform, or a nullptr.
     *
     * @see Transform::getId()
     */
    inline std::shared_ptr<const Transform> findTransformById(
        TransformId id )
        const noexcept
    {
        for ( DimensionIndex i = 0; i < this->numberOfDimensions; ++i )
        {
            const auto t = this->transformByDimension[i];
            if ( t != nullptr && t->getId() == id )
                return t;
        }

        const TransformIndex nAdd = this->additionalClusteringParameters.size( );
        for ( TransformIndex i = 0; i < nAdd; ++i )
        {
            const auto t = this->additionalClusteringParameters[i].second;
            if ( t != nullptr && t->getId() == id )
                return t;
        }

        return nullptr;
    }

    /**
     * Returns the transform, if any, with the given ID.
     *
     * This method looks through the gate's dimensions additional clustering
     * parameters for a transform with the given ID. The transform is
     * returned, or a nullptr if no transform is found.
     *
     * @param[in] id
     *   The transform ID to look up.
     *
     * @return
     *   Returns the transform, or a nullptr.
     *
     * @see Transform::getId()
     */
    inline std::shared_ptr<Transform> findTransformById(
        TransformId id )
        noexcept
    {
        for ( DimensionIndex i = 0; i < this->numberOfDimensions; ++i )
        {
            auto t = this->transformByDimension[i];
            if ( t != nullptr && t->getId() == id )
                return t;
        }

        const TransformIndex nAdd = this->additionalClusteringParameters.size( );
        for ( TransformIndex i = 0; i < nAdd; ++i )
        {
            auto t = this->additionalClusteringParameters[i].second;
            if ( t != nullptr && t->getId() == id )
                return t;
        }

        return nullptr;
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the gate.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string& indent = "" )
        const noexcept
    {
        std::string s = indent + this->getGateTypeName( ) + ":";

        if ( this->name.empty( ) == false )
            s += " \"" + this->name + "\"\n";
        else
            s += "\n";

        if ( this->description.empty( ) == false )
            s += indent + "  " + this->description + "\n";

        if ( this->notes.empty( ) == false )
            s += indent + "  " + this->notes + "\n";

        s += indent + "  Use " + getGatingMethodName( ) + " gating\n";

        s += indent + "  Report priority " +
            std::to_string(getReportPriority( )) + "\n";

        const DimensionIndex n = this->getNumberOfDimensions( );
        for ( DimensionIndex i = 0; i < n; ++i )
        {
            s += indent + "  Parameter \"" + this->getDimensionParameterName(i) + "\"\n";
            const auto transform = this->getDimensionParameterTransform(i);
            if ( transform != nullptr )
                s += indent + "    " + transform->toString( );
        }

        return s;
    }
    // @}
};



/**
 * Describes a boolean gate expression.
 *
 * Boolean gates describe a boolean expression using AND, OR, and NOT
 * operators for its list of gate children. Each gate child implicitly
 * defines a set of event values included within the gate. The boolean
 * gate's operator combines those events with a boolean expression.
 *
 * In addition to the gate's operator, each child in the gate's list of
 * children can have its own NOT to invert its gate criteria.
 *
 * When the gate's operator is NOT, there must be exactly one child.
 * When the operator is AND or OR, there can be two or more children.
 *
 *
 * <B>Attributes</B><BR>
 * In addition to the attributes of the parent class, a boolean gate has
 * the following attributes:
 *
 * @li An operator. Operators are one of AND_OPERATOR, OR_OPERATOR, or
 *     NOT_OPERATOR.
 *
 * @li An optional NOT unary operator per child.
 */
class BooleanGate final
    : public Gate
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
public:
    /**
     * Indicates the boolean operator in a gate expression.
     */
    enum Operator
    {
        AND_OPERATOR,
        OR_OPERATOR,
        NOT_OPERATOR
    };

private:
    /**
     * The operator for the boolean expression.
     *
     * @see ::getOperator()
     * @see ::setOperator()
     */
    const Operator op;

    /**
     * Whether each child has a unary NOT.
     */
    std::vector<bool> childrenNot;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new boolean gate.
     *
     * The boolean gate operates upon its children. When the operator is
     * NOT_OPERATOR, there can be at most one child. When the operator is
     * AND_OPERATOR or OR_OPERATOR, the gate may have any number of children.
     *
     * @param[in] op
     *   The boolean operator. One of AND_OPERATOR, OR_OPERATOR or
     *   NOT_OPERATOR.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the operator is not recognized.
     *
     * @see ::getOperator()
     */
    BooleanGate( const Operator op )
        : Gate( BOOLEAN_GATE ),
        op( op )
    {
        switch ( op )
        {
        case Operator::AND_OPERATOR:
        case Operator::OR_OPERATOR:
        case Operator::NOT_OPERATOR:
            break;

        default:
            throw std::invalid_argument( "Invalid boolean gate operator." );
        }
    }

    /**
     * Constructs a copy of the given boolean gate.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * @param[in] gate
     *   The gate to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the operator is not recognized.
     *
     * @see ::getOperator()
     */
    BooleanGate( const BooleanGate& gate )
        : Gate( gate ),
        op( gate.op )
    {
        // Nothing to do. The children of the given gate are NOT copied.
    }

    /**
     * Constructs a copy of the given boolean gate.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * @param[in] gate
     *   The gate to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the operator is not recognized.
     *
     * @see ::getOperator()
     */
    BooleanGate( std::shared_ptr<const BooleanGate> gate )
        : BooleanGate( *(gate.get()) )
    {
        // Nothing to do. The children of the given gate are NOT copied.
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~BooleanGate( )
        noexcept
    {
        this->childrenNot.clear( );
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the boolean operator.
     *
     * @return
     *   Returns the operator.
     *
     * @see ::isChildNot()
     * @see ::setChildNot()
     */
    inline Operator getOperator( )
        const noexcept
    {
        return this->op;
    }

    /**
     * Returns whether the indicated child has a NOT in the boolean expression.
     *
     * @param[in] index
     *   The child list index.
     *
     * @return
     *   Returns the child.
     *
     * @throws std::out_of_range
     *   Throws an exception if the child index is out of range.
     *
     * @see ::setChildNot()
     */
    inline bool isChildNot( const GateIndex index )
        const
    {
        // Validate.
        if ( index >= this->getNumberOfChildren( ) )
            throw std::out_of_range( "Invalid child index out of range." );

        return this->childrenNot[index];
    }



    /**
     * Sets whether the indicated child has a NOT in the boolean expression.
     *
     * @param[in] index
     *   The child list index.
     * @param[in] compliment
     *   When true, the child has a NOT in the boolean expression.
     *
     * @throws std::out_of_range
     *   Throws an exception if the child index is out of range.
     *
     * @see ::setChildNot()
     */
    inline void setChildNot( const GateIndex index, const bool compliment )
    {
        // Validate.
        if ( index >= this->getNumberOfChildren( ) )
            throw std::out_of_range( "Invalid child index out of range." );

        this->childrenNot[index] = compliment;
    }

    /**
     * Returns TRUE if additional clustering parameters are supported
     * by the gate.
     *
     * Boolean gates do not support additional clustering parameters,
     * so this method always returns FALSE.
     *
     * @return
     *   Returns FALSE.
     *
     * @see ::appendAdditionalClusteringParameter()
     * @see ::clearAdditionalClusteringParameters()
     * @see ::getAdditionalClusteringParameterName()
     * @see ::getAdditionalClusteringParameterTransform()
     * @see ::getAdditionalClusteringParameters()
     * @see ::getNumberOfAdditionalClusteringParameters()
     * @see ::hasAdditionalClusteringParameters()
     */
    virtual bool supportsAdditionalClusteringParameters( )
        const noexcept override
    {
        return false;
    }
    // @}



//----------------------------------------------------------------------
// Tree get, append, and remove.
//----------------------------------------------------------------------
public:
    /**
     * @name Tree get, append, and remove
     */
    // @{
    /**
     * Appends a gate to the list of children.
     *
     * @param[in] gate
     *   The gate to add as a child.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the given gate is nullptr or if the gate is
     *   already the child of another parent.
     *
     * @see ::clearChildren()
     * @see ::removeChild()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    virtual void appendChild( const std::shared_ptr<Gate>& gate )
        override
    {
        this->appendChild( gate, false );
    }

    /**
     * Appends a gate to the list of children.
     *
     * @param[in] gate
     *   The gate to add as a child.
     * @param[in] not
     *   Whether to apply a NOT to the child within the boolean expression.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the given gate is nullptr or if the gate is
     *   already the child of another parent.
     *
     * @see ::clearChildren()
     * @see ::removeChild()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    inline void appendChild(
        const std::shared_ptr<Gate>& gate,
        const bool negate )
    {
        // Validate.
        if ( this->op == Operator::NOT_OPERATOR &&
            this->getNumberOfChildren( ) == 1 )
            throw std::invalid_argument(
                "Invalid append of more than one gate to a boolean gate using a NOT operator." );

        Gate::appendChild( gate );
        this->childrenNot.push_back( negate );
    }

    /**
     * Clears the list of children.
     *
     * @see ::removeChild()
     * @see ::getNumberOfChildren()
     */
    virtual void clearChildren( )
        noexcept override
    {
        Gate::clearChildren( );
        this->childrenNot.clear( );
    }

    /**
     * Removes a gate from the list of children.
     *
     * @param[in] index
     *   The index of the gate to remove as a child.
     *
     * @return
     *   Returns the children list index of the removed child.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr pointer or is not a
     *   child of this gate.
     *
     * @see ::appendChild()
     * @see ::clearChildren()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    virtual GateIndex removeChild( const GateIndex index )
        override
    {
        this->childrenNot.erase( this->childrenNot.begin( ) + index );
        return index;
    }

    /**
     * Removes a gate from the list of children.
     *
     * @param[in] gate
     *   The gate to remove as a child.
     *
     * @return
     *   Returns the children list index of the removed child.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr pointer or is not a
     *   child of this gate.
     *
     * @see ::appendChild()
     * @see ::clearChildren()
     * @see ::getNumberOfChildren()
     * @see ::getChild()
     */
    virtual GateIndex removeChild( const std::shared_ptr<Gate>& gate )
        override
    {
        const GateIndex index = Gate::removeChild( gate );
        this->childrenNot.erase( this->childrenNot.begin( ) + index );
        return index;
    }
    // @}
};



/**
 * Describes a range or multi-dimensional rectangular gate.
 *
 * Rectangular gates specify a numeric value range for each dimension in
 * a multi-dimensional rectangle. When dimensionality is one, a single
 * dimension's value range is given. When dimension is two, two dimension
 * value ranges are given, and so on.
 *
 * <B>Attributes</B><BR>
 * In addition to the attributes of the parent class, a rectangular gate has
 * the following attributes:
 *
 * @li A (min,max) pair for each dimension. The pair defines the numeric
 *     range for parameter values that are considered within the gate region.
 */
class RectangleGate final
    : public Gate
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The maximum values for a multi-dimensional rectangle.
     *
     * The vector will have one entry for each dimension.
     *
     * @see ::getDimensionMaximum()
     * @see ::getDimensionMinimumMaximum()
     */
    std::vector<double> maximumByDimension;

    /**
     * The minimum values for a multi-dimensional rectangle.
     *
     * The vector will have one entry for each dimension.
     *
     * @see ::getDimensionMinimum()
     * @see ::getDimensionMinimumMaximum()
     */
    std::vector<double> minimumByDimension;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new rectangle gate with zero min/max per dimension.
     *
     * A rectangle is defined by a minimum and maximum for each dimension.
     * The number of dimensions must be specified on construction. This
     * constructor initializes each minimum and maximum to the lowest and
     * highest possible value for a double-precision floating point. This
     * creates an all-inclusive multi-dimensional rectangle.
     *
     * The number of dimensions may be one or more. For a one-dimensional
     * rectangle, a single min/max pair specifies a range along one
     * parameter axis.
     *
     * @param[in] numberOfDimensions
     *   The number of dimensions for the gate.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the number of dimensions is zero.
     */
    RectangleGate(
        const DimensionIndex numberOfDimensions )
        : Gate(
            RECTANGLE_GATE,
            numberOfDimensions )
    {
        if ( numberOfDimensions == 0 )
            throw std::invalid_argument(
                "Invalid zero number of dimensions." );

        for ( DimensionIndex i = 0; i < numberOfDimensions; ++i )
        {
            this->minimumByDimension.push_back(
                std::numeric_limits<double>::lowest( ) );
            this->maximumByDimension.push_back(
                std::numeric_limits<double>::max( ) );
        }
    }

    /**
     * Constructs a new rectangle gate.
     *
     * A rectangle is defined by a minimum and maximum for each dimension.
     * This constructor takes a list each for minimums and maximums. The
     * two lists must have the same number of values, and the list size
     * defines the rectangle's dimensionality.
     *
     * The number of dimensions may be one or more. For a one-dimensional
     * rectangle, a single min/max pair specifies a range along one
     * parameter axis.
     *
     * @param[in] minimums
     *   A list of range minimums.
     * @param[in] maximums
     *   A list of range maximums.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the list of minimums or maximums is empty,
     *   or if the two lists do not have the same size.
     *
     * @see ::getDimensionMinimum()
     * @see ::getDimensionMinimumMaximum()
     * @see ::getDimensionMaximum()
     */
    RectangleGate(
        const std::vector<double>& minimums,
        const std::vector<double>& maximums )
        : Gate( RECTANGLE_GATE, minimums.size() )
    {
        // Validate.
        if ( minimums.size( ) != maximums.size( ) )
            throw std::invalid_argument(
                "Invalid min/max lists do not have the same size." );

        // Copy. Swap min/max if needed.
        const DimensionIndex n = minimums.size( );
        for ( DimensionIndex i = 0; i < n; ++i )
        {
            const double min = minimums[i];
            const double max = maximums[i];
            if ( min < max )
            {
                this->minimumByDimension.push_back( min );
                this->maximumByDimension.push_back( max );
            }
            else
            {
                this->minimumByDimension.push_back( max );
                this->maximumByDimension.push_back( min );
            }
        }
    }

    /**
     * Constructs a copy of the given gate shape.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * @param[in] gate
     *   The gate to copy.
     */
    RectangleGate( const RectangleGate& gate )
        noexcept
        : Gate( gate )
    {
        this->maximumByDimension = gate.maximumByDimension;
        this->minimumByDimension = gate.minimumByDimension;
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~RectangleGate( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the maximum for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     *
     * @return
     *   Returns the maximum.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getDimensionMinimum()
     * @see ::getDimensionMinimumMaximum()
     * @see ::setDimensionMinimumMaximum()
     */
    inline double getDimensionMaximum( const DimensionIndex index )
        const
    {
        // Validate.
        if ( index >= this->getNumberOfDimensions( ) )
            throw std::out_of_range( "Invalid dimension index out of range." );

        return this->maximumByDimension[index];
    }

    /**
     * Returns the minimum for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     *
     * @return
     *   Returns the min.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getDimensionMaximum()
     * @see ::getDimensionMinimumMaximum()
     * @see ::setDimensionMinimumMaximum()
     */
    inline double getDimensionMinimum( const DimensionIndex index )
        const
    {
        // Validate.
        if ( index >= this->getNumberOfDimensions( ) )
            throw std::out_of_range( "Invalid dimension index out of range." );

        return this->minimumByDimension[index];
    }

    /**
     * Gets the minimum and maximum for the indicated dimension.
     *
     * @param[in] index
     *   The dimension index.
     * @param[out] minimum
     *   The minimum of the range.
     * @param[out] maximum
     *   The maximum of the range.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getDimensionMaximum()
     * @see ::getDimensionMinimum()
     */
    const void getDimensionMinimumMaximum(
        const DimensionIndex index,
        double& minimum,
        double& maximum )
        const
    {
        // Validate.
        if ( index >= this->getNumberOfDimensions( ) )
            throw std::out_of_range( "Invalid dimension index out of range." );

        minimum = this->minimumByDimension[index];
        maximum = this->maximumByDimension[index];
    }



    /**
     * Sets the maximum for the indicated dimension.
     *
     * The new maximum is presumed to be larger than the dimension's minimum.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] maximum
     *   The maximum of the range.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getDimensionMaximum()
     * @see ::getDimensionMinimumMaximum()
     */
    const void setDimensionMaximum(
        const DimensionIndex index,
        const double maximum )
    {
        // Validate.
        if ( index >= this->getNumberOfDimensions( ) )
            throw std::out_of_range( "Invalid dimension index out of range." );

        this->maximumByDimension[index] = maximum;

        if ( this->state != nullptr )
            this->state->callbackSetRectangleMinimumMaximum(
                index,
                this->minimumByDimension[index],
                maximum );
    }

    /**
     * Sets the minimum for the indicated dimension.
     *
     * The new minimum is presumed to be smaller than the dimension's maximum.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] minimum
     *   The minimum of the range.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getDimensionMinimum()
     * @see ::getDimensionMinimumMaximum()
     */
    const void setDimensionMinimum(
        const DimensionIndex index,
        const double minimum )
    {
        // Validate.
        if ( index >= this->getNumberOfDimensions( ) )
            throw std::out_of_range( "Invalid dimension index out of range." );

        this->minimumByDimension[index] = minimum;

        if ( this->state != nullptr )
            this->state->callbackSetRectangleMinimumMaximum(
                index,
                minimum,
                this->maximumByDimension[index] );
    }

    /**
     * Sets the minimum and maximum for the indicated dimension.
     *
     * The new minimum is presumed to be smaller than the new maximum.
     *
     * @param[in] index
     *   The dimension index.
     * @param[in] minimum
     *   The minimum of the range.
     * @param[in] maximum
     *   The maximum of the range.
     *
     * @throws std::out_of_range
     *   Throws an exception if the dimension index is out of range.
     *
     * @see ::getDimensionMaximum()
     * @see ::getDimensionMinimum()
     * @see ::getDimensionMinimumMaximum()
     */
    const void setDimensionMinimumMaximum(
        const DimensionIndex index,
        const double minimum,
        const double maximum )
    {
        // Validate.
        if ( index >= this->getNumberOfDimensions( ) )
            throw std::out_of_range( "Invalid dimension index out of range." );

        this->minimumByDimension[index] = minimum;
        this->maximumByDimension[index] = maximum;

        if ( this->state != nullptr )
            this->state->callbackSetRectangleMinimumMaximum(
                index,
                minimum,
                maximum );
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the gate and all of its children.
     *
     * @return
     *   Returns a clone of this gate.
     */
    virtual Gate* clone( )
        const override
    {
        return this->cloneChildren( new RectangleGate( *this ) );
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the gate.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string& indent = "" )
        const noexcept
    {
        std::string s = indent + this->getGateTypeName( ) + ":";

        const std::string name = this->getName( );
        if ( name.empty( ) == false )
            s += " \"" + name + "\"\n";
        else
            s += "\n";

        const std::string description = this->getDescription( );
        if ( description.empty( ) == false )
            s += indent + "  Description: " + description + "\n";

        const std::string notes = this->getNotes( );
        if ( notes.empty( ) == false )
            s += indent + "  Notes: " + notes + "\n";

        s += indent + "  Use " + this->getGatingMethodName( ) + " gating\n";

        const DimensionIndex n = this->getNumberOfDimensions( );
        for ( DimensionIndex i = 0; i < n; ++i )
        {
            s += indent + "  Parameter \"" + this->getDimensionParameterName(i) +
                "\" min=" +
                std::to_string( this->minimumByDimension[i] ) +
                " max=" +
                std::to_string( this->maximumByDimension[i] ) + "\n";
            const auto transform = this->getDimensionParameterTransform(i);
            if ( transform != nullptr )
                s += indent + "    " + transform->toString( );
        }

        return s;
    }
    // @}
};



/**
 * Describes a two-dimensional polygonal gate.
 *
 * Polygonal gates specify a list of three or more vertices that describe
 * the outline of an irregular shape in two dimensions. Event parameter values
 * that fall within the polygon are considered within the gate region.
 *
 *
 * <B>Attributes</B><BR>
 * In addition to the attributes of the parent class, a polygon gate has
 * the following attributes:
 *
 * @li A list of (x,y) vertices, where the x axis corresponds to the first
 *     dimension, and the y axis corresponds to the second dimension.
 *
 *
 * <B>Warning</B><BR>
 * It is up to the caller to validate the polygon's attributes.
 *
 * @li A polygon must have at least three vertices, but validating this is
 *   left to the caller after all vertices have been added to the polygon.
 *
 * @li The polygon's last vertex implicitly connects back to the first vertex
 *   to close the polygon. There is no need for the last vertex to duplicate
 *   the first vertex, but it is up to the caller to check for this.
 *
 * @li Typically, polygons should not have self-crossing boundaries (e.g.
 *   a bow tie shape). It is the caller's responsability to check for this
 *   if it cannot use polygons of this type.
 */
class PolygonGate
    : public Gate
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The X coordinates for vertices for the 2D polygon.
     */
    std::vector<double> xCoordinates;

    /**
     * The Y coordinates for vertices for the 2D polygon.
     */
    std::vector<double> yCoordinates;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new polygon gate.
     *
     * A polygon gate has a list of vertices for the perimiter of a
     * 2D polygon. The vertex list must contain at least three vertices.
     */
    PolygonGate( )
        noexcept
        : Gate( POLYGON_GATE, 2 )
    {
        // Nothing to do.
    }

    /**
     * Constructs a new polygon gate.
     *
     * A polygon gate has a list of vertices for the perimiter of a
     * 2D polygon. The vertex list must contain at least three vertices.
     *
     * @param[in] x
     *   The polygon's X coordinates.
     * @param[in] y
     *   The polygon's Y coordinates.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the two coordinate lists are not the same size.
     */
    PolygonGate(
        const std::vector<double> x,
        const std::vector<double> y )
        : Gate( POLYGON_GATE, 2 )
    {
        // Validate.
        if ( x.size( ) != y.size( ) )
            throw std::invalid_argument(
                "Invalid coordinate lists are not the same size." );

        this->xCoordinates = x;
        this->yCoordinates = y;
    }

    /**
     * Constructs a copy of the given gate shape.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * @param[in] gate
     *   The gate to copy.
     */
    PolygonGate( const PolygonGate& gate )
        noexcept
        : Gate( gate )
    {
        this->xCoordinates = gate.xCoordinates;
        this->yCoordinates = gate.yCoordinates;
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~PolygonGate( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the gate and all of its children.
     *
     * @return
     *   Returns a clone of this gate.
     */
    virtual Gate* clone( )
        const override
    {
        return this->cloneChildren( new PolygonGate( *this ) );
    }
    // @}



//----------------------------------------------------------------------
// Vertices.
//----------------------------------------------------------------------
public:
    /**
     * @name Vertices
     */
    // @{
    /**
     * Appends the (x,y) coordinate pair.
     *
     * @param[in] x
     *   The X coordinate.
     * @param[in] y
     *   The Y coordinate.
     *
     * @see ::getNumberOfVertices()
     * @see ::getVertex()
     * @see ::removeVertext()
     * @see ::clearVertices()
     */
    inline void appendVertex( const double x, const double y )
        noexcept
    {
        const size_t n = this->xCoordinates.size( );
        this->xCoordinates.push_back( x );
        this->yCoordinates.push_back( y );

        if ( this->state != nullptr )
            this->state->callbackAppendPolygonVertex( n, x, y );
    }

    /**
     * Clears the vertex list.
     */
    inline void clearVertices( )
        noexcept
    {
        this->xCoordinates.clear( );
        this->yCoordinates.clear( );

        if ( this->state != nullptr )
            this->state->callbackClearPolygonVertices( );
    }

    /**
     * Returns the bounding box for the polygon's vertices.
     *
     * @param[out] xMin
     *   The lowest X value of the bounding box.
     * @param[out] yMin
     *   The lowest Y value of the bounding box.
     * @param[out] xMax
     *   The highest X value of the bounding box.
     * @param[out] yMax
     *   The highest Y value of the bounding box.
     */
    inline void getBoundingBox(
        double& xMin, double& yMin,
        double& xMax, double& yMax )
        const noexcept
    {
        const size_t n = this->xCoordinates.size( );
        xMax = xMin = this->xCoordinates[0];
        yMax = yMin = this->yCoordinates[0];
        for ( size_t i = 1; i < n; ++i )
        {
            const double x = this->xCoordinates[i];
            const double y = this->yCoordinates[i];
            if ( x < xMin )
                xMin = x;
            if ( x > xMax )
                xMax = x;
            if ( y < yMin )
                yMin = y;
            if ( y > yMax )
                yMax = y;
        }
    }

    /**
     * Returns the number of vertices.
     *
     * @return
     *   Returns the number of vertices.
     */
    inline const VertexIndex getNumberOfVertices( )
        const noexcept
    {
        return this->xCoordinates.size( );
    }

    /**
     * Returns the selected X coordinate.
     *
     * @param[in] index
     *   The vertex list index.
     *
     * @return
     *   Returns the X coordinate.
     *
     * @throws std::out_of_range
     *   Throws an exception if the vertex list index is out of range.
     *
     * @see ::appendVertex()
     * @see ::getNumberOfVertices()
     * @see ::removeVertext()
     * @see ::getVertex()
     */
    inline double getVertexX( const VertexIndex index )
        const
    {
        // Validate.
        if ( index >= this->xCoordinates.size( ) )
            throw std::out_of_range( "Invalid vertex index out of range." );

        return this->xCoordinates[index];
    }

    /**
     * Returns the selected Y coordinate.
     *
     * @param[in] index
     *   The vertex list index.
     *
     * @return
     *   Returns the Y coordinate.
     *
     * @throws std::out_of_range
     *   Throws an exception if the vertex list index is out of range.
     *
     * @see ::appendVertex()
     * @see ::getNumberOfVertices()
     * @see ::removeVertext()
     * @see ::getVertex()
     */
    inline double getVertexY( const VertexIndex index )
        const
    {
        // Validate.
        if ( index >= this->xCoordinates.size( ) )
            throw std::out_of_range( "Invalid vertex index out of range." );

        return this->yCoordinates[index];
    }

    /**
     * Returns the selected (x,y) coordinate pair.
     *
     * @param[in] index
     *   The vertex list index.
     * @param[out] x
     *   Returns the X coordinate.
     * @param[out] y
     *   Returns the Y coordinate.
     *
     * @throws std::out_of_range
     *   Throws an exception if the vertex list index is out of range.
     *
     * @see ::appendVertex()
     * @see ::getNumberOfVertices()
     * @see ::removeVertext()
     * @see ::getVertexX()
     * @see ::getVertexY()
     */
    inline void getVertex( const VertexIndex index, double& x, double& y )
        const
    {
        // Validate.
        if ( index >= this->xCoordinates.size( ) )
            throw std::out_of_range( "Invalid vertex index out of range." );

        x = this->xCoordinates[index];
        y = this->yCoordinates[index];
    }

    /**
     * Deletes the indicated vertex.
     *
     * @param[in] index
     *   The vertex list index.
     *
     * @see ::appendVertex()
     * @see ::clearVertices()
     * @see ::getNumberOfVertices()
     * @see ::getVertex()
     */
    inline void removeVertex( const VertexIndex index )
    {
        // Validate.
        if ( index >= this->xCoordinates.size( ) )
            throw std::out_of_range( "Invalid vertex index out of range." );

        this->xCoordinates.erase( this->xCoordinates.begin( ) + index );
        this->yCoordinates.erase( this->yCoordinates.begin( ) + index );

        if ( this->state != nullptr )
            this->state->callbackRemovePolygonVertex( index );
    }

    /**
     * Returns a vector of X coordinates.
     *
     * @return
     *   Returns an X coordinate vector.
     *
     * @see ::appendVertex()
     * @see ::getNumberOfVertices()
     * @see ::removeVertext()
     * @see ::getVertex()
     * @see ::getVertexX()
     */
    inline const std::vector<double> getVerticesX( )
        const
    {
        return this->xCoordinates;
    }

    /**
     * Returns a vector of Y coordinates.
     *
     * @return
     *   Returns an Y coordinate vector.
     *
     * @see ::appendVertex()
     * @see ::getNumberOfVertices()
     * @see ::removeVertext()
     * @see ::getVertex()
     * @see ::getVertexY()
     */
    inline const std::vector<double> getVerticesY( )
        const
    {
        return this->yCoordinates;
    }

    /**
     * Sets the selected (x,y) coordinate pair.
     *
     * @param[in] index
     *   The vertex list index.
     * @param[in] x
     *   The X coordinate.
     * @param[in] y
     *   The Y coordinate.
     *
     * @throws std::out_of_range
     *   Throws an exception if the vertex list index is out of range.
     *
     * @see ::getNumberOfVertices()
     * @see ::getVertex()
     */
    inline void setVertex(
        const VertexIndex index,
        const double x,
        const double y )
    {
        // Validate.
        if ( index >= this->xCoordinates.size( ) )
            throw std::out_of_range( "Invalid vertex index out of range." );

        this->xCoordinates[index] = x;
        this->yCoordinates[index] = y;

        if ( this->state != nullptr )
            this->state->callbackSetPolygonVertex( index, x, y );
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the gate.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string& indent = "" )
        const noexcept
    {
        std::string s = Gate::toString( indent );

        const VertexIndex nVertices = this->xCoordinates.size( );
        s += indent + "  vertices=";
        for ( VertexIndex i = 0; i < nVertices; ++i )
        {
            s += "(" +
                std::to_string( this->xCoordinates[i] ) +
                "," +
                std::to_string( this->yCoordinates[i] ) +
                ") ";
        }
        s += "\n";

        return s;
    }
    // @}
};



/**
 * Describes a multi-dimensional ellipsoid gate.
 *
 * Ellipsoid gates specify a multi-dimensional ellipsoid with two or more
 * dimensions, characterized by a center point (mean in each dimension),
 * a covariance matrix to orient the ellipsoid, and a distance squared value
 * to size the ellipsoid.
 *
 *
 * <B>Attributes</B><BR>
 * In addition to the attributes of the parent class, an ellipsoid gate has
 * the following attributes:
 *
 * @li A list of center values, with one value for each dimension.
 *
 * @li A square covariance matrix with one row and column for each dimension.
 *
 * @li A Mahalanobis distance, squared.
 *
 *
 * <B>Warning</B><BR>
 * It is up to the caller to validate the ellipsoid's attributes.
 *
 * @li The ellipsoid's center coordinate should be the mean of the ellipsoid's
 *   dimensions, but it is up to the caller to insure that it is.
 *
 * @li The covariance matrix must include reasonable values for defining
 *   the ellipsoid's extent, but it is up to the caller to check this.
 *
 * @li The distance squared value should be the square of the Mahalanobis
 *   distance, but again it is up to the caller to insure this.
 */
class EllipsoidGate final
    : public Gate
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    /**
     * The center (mean) of the ellipsoid.
     *
     * The vector will have one entry for each dimension.
     */
    std::vector<double> center;

    /**
     * The covariance matrix of the ellipsoid.
     *
     * The vector will include a 2D NxN matrix, where N is the number of
     * dimensions. Values are arranged so that [0] is the first value of
     * the first row, [1] is the second value of the first row, and so on.
     */
    std::vector<double> covarianceMatrix;

    /**
     * The distance squared of the ellipsoid.
     *
     * This is the "square of the Mahalanobis distance".
     */
    double distanceSquared;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new ellipsoid gate.
     *
     * @param[in] center
     *   A list of center values. The list length is the gate's dimensionality.
     * @param[in] covarianceMatrix
     *   The ellpisoid's covariance trix.
     * @param[in] distanceSquared
     *   The Mahalanobis distance squared.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the center list has fewer than 2 values,
     *   if the covariance matrix does not have enough values for the
     *   number of dimensions squared, or if the squared distance is
     *   non-positive.
     */
    EllipsoidGate(
        const std::vector<double> center,
        const std::vector<double> covarianceMatrix,
        const double distanceSquared )
        : Gate(
            ELLIPSOID_GATE,
            center.size( ) ),
        distanceSquared( distanceSquared )
    {
        // Validate.
        if ( center.size( ) < 2 )
            throw std::invalid_argument(
                "Invalid center size (gate dimension) must be at least 2." );
        if ( covarianceMatrix.size( ) != (center.size() * center.size()) )
            throw std::invalid_argument(
                "Invalid covariance matrix does not have exactly one value per gate dimension squared." );
        if ( distanceSquared <= 0.0 )
            throw std::invalid_argument(
                "Invalid squared distance is non-positive." );

        this->center = center ;
        this->covarianceMatrix = covarianceMatrix;
    }

    /**
     * Constructs a copy of the given gate.
     *
     * The given gate's children are <B>not copied</B><BR>.
     *
     * @param[in] gate
     *   The gate to copy.
     */
    EllipsoidGate( const EllipsoidGate& gate )
        noexcept
        : Gate( gate ),
        distanceSquared( gate.distanceSquared )
    {
        this->center = gate.center;
        this->covarianceMatrix = gate.covarianceMatrix;
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~EllipsoidGate( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the gate and all of its children.
     *
     * @return
     *   Returns a clone of this gate.
     */
    virtual Gate* clone( )
        const override
    {
        return this->cloneChildren( new EllipsoidGate( *this ) );
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the ellipsoid center.
     *
     * The returned list contains one value for each dimension of
     * the gate's coverage.
     *
     * @return
     *   Returns the ellipsoid center.
     */
    inline const std::vector<double>& getCenter( )
        const noexcept
    {
        return this->center;
    }

    /**
     * Returns the ellipsoid center.
     *
     * The returned list contains one value for each dimension of
     * the gate's coverage.
     *
     * @return
     *   Returns the ellipsoid center.
     */
    inline std::vector<double>& getCenter( )
        noexcept
    {
        return this->center;
    }

    /**
     * Returns the ellipsoid's covariance matrix.
     *
     * @return
     *   Returns the matrix.
     */
    inline const std::vector<double>& getCovarianceMatrix( )
        const noexcept
    {
        return this->covarianceMatrix;
    }

    /**
     * Returns the ellipsoid's covariance matrix.
     *
     * @return
     *   Returns the matrix.
     */
    inline std::vector<double>& getCovarianceMatrix( )
        noexcept
    {
        return this->covarianceMatrix;
    }

    /**
     * Returns the ellipsoid's squared Mahalanobis distance.
     *
     * @return
     *   Returns the distance.
     */
    inline double getSquaredDistance( )
        const noexcept
    {
        return this->distanceSquared;
    }



    /**
     * Sets the ellipsoid center.
     *
     * The given list contains one value for each dimension of
     * the gate's coverage.
     *
     * @param[in] center
     *   The ellipsoid center.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the vector does not have exactly one value
     *   for each gate dimension.
     */
    inline void setCenter( const std::vector<double>& center )
    {
        if ( center.size( ) != this->getNumberOfDimensions( ) )
            throw std::invalid_argument(
                "Invalid center vector does not have exactly one value per gate dimension." );

        this->center = center;
    }

    /**
     * Sets the ellipsoid's covariance matrix.
     *
     * @param[in] matrix
     *   The matrix.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the vector does not have exactly one value
     *   for each gate dimension, squared.
     */
    inline void setCovarianceMatrix( const std::vector<double>& matrix )
    {
        const auto n = this->getNumberOfDimensions( );
        if ( matrix.size( ) != (n * n) )
            throw std::invalid_argument(
                "Invalid covariance matrix does not have exactly one value per gate dimension squared." );

        this->covarianceMatrix = matrix;
    }


    /**
     * Sets the ellipsoid's squared Mahalanobis distance.
     *
     * @param[in] distance
     *   The distance.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the distance is non-positive.
     */
    inline void setSquaredDistance( const double distance )
    {
        if ( distance <= 0.0 )
            throw std::invalid_argument(
                "Invalid squared distance is non-positive." );

        this->distanceSquared = distance;
    }
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the gate.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string& indent = "" )
        const noexcept
    {
        std::string s = Gate::toString( indent );

        s += indent + "  distanceSquared=" +
            std::to_string( this->distanceSquared ) + "\n";

        s += indent + "  center=(";
        const DimensionIndex n = this->getNumberOfDimensions( );
        for ( DimensionIndex i = 0; i < n; ++i )
        {
            if ( i == (n-1) )
                s += std::to_string( this->center[i] ) + ")\n";
            else
                s += std::to_string( this->center[i] ) + ", ";
        }

        s += indent + "  covarianceMatrix=(";
        const auto nn = n * n;

        for ( DimensionIndex i = 0; i < nn; ++i )
        {
            if ( i == (n-1) )
                s += std::to_string( this->covarianceMatrix[i] ) + ")\n";
            else
                s += std::to_string( this->covarianceMatrix[i] ) + ", ";
        }

        return s;
    }
    // @}
};



/**
 * Describes a multi-quadrant gate.
 *
 * Quadrant gates, despite the name, divide the event space into more than
 * just four (quad) regions. The gate describes a series of dividers that
 * divide the event space. Each divider divides at one or more places along
 * a selected parameter dimension. Once divided, a series of quadrant
 * definitions select and name specific divisions.
 *
 * In addition to the attributes of the parent class, each quadrant
 * gate has the following attributes:
 *
 * @li The number of dividers. There is always at least one.
 *
 * @li The unique ID for each divider. The ID is unique within the gate,
 *     but not globally.
 *
 * @li A compensation method for each divider. Compensation applies a
 *     spillover matrix or similar mechanism to account for cross-talk
 *     among the acquired parameters of an event.
 *
 * @li An optional transformation method for each divider. Transformation
 *     applies a mathematical expression to map the original value range to
 *     a more convenient range, such as to do linear or logarithmic scaling.
 *
 * @li An event parameter name for each divider. Parameters are the columns
 *     of an event table read from an external event file, such as one in the
 *     ISAC FCS format.
 *
 * @li A list of division points along each divider. A division point is where
 *     the divider divides the event space along the indicated parameter's
 *     value range. With one value, the divider divides that range in two.
 *     With two values, the divider divides that range into three regions, etc.
 *
 * @li A list of labeled quadrants. Not all regions created by divisions are
 *     relevant. Only those that are labeled as quadrants are relevant. Each
 *     quadrant has:
 *
 *     @li A unique quadrant ID. The quadrant's ID acts as a gate ID and may
 *         reference the quadrant within gating expressions.
 *
 *     @li A list of dividers that bound the quadrant, and a sample position
 *         along each divider.
 *
 * @internal
 * Each "divider" for a quadrant gate is similar to a new dimension for
 * a rectangle gate, so the parent class's getNumberOfDimensions() and
 * this class's getNumberOfDividers() return the same values. Similarly,
 * the getDimension*() methods on the parent class are the same as the
 * getDivider*() methods here.
 * @endinternal
 *
 * @todo This class is missing set*() methods to define named quadrants.
 * Methods are needed add/remove quadrants in the list, and set the divider
 * ID and position list for each quadrant. Validation is also required to
 * check that the quadrants are well-defined by referring to known divider
 * IDs in the same gate.
 */
class QuadrantGate final
    : public Gate
{
//----------------------------------------------------------------------
// Fields.
//----------------------------------------------------------------------
private:
    class Quadrant
    {
    public:
        /**
         * The unique identifier for the quadrant.
         *
         * Identifiers must be unique among all quadrants AND among all
         * gates in the gate definition.
         */
        std::string id;

        /**
         * A list of positions.
         *
         * Each position is a pair that includes the string identifier of
         * a divider in the gate, and a representative value (location)
         * for events that lie within the quadrant.
         *
         * The list of positions includes at least one entry, and no
         * more than N entries where N is the number of dividers defined.
         */
        std::vector<std::pair<std::string,double>> positions;
    };

private:
    /**
     * The unique identifier of each divider.
     *
     * Identifiers must be unique among all dividers in the quadrant.
     * These identifiers are used solely to indicate boundaries when
     * defining individual quandrants.
     */
    std::vector<std::string> idByDivider;

    /**
     * The divider value list for each divider.
     *
     * The vector will have one entry for each divider. Each entry is
     * itself a vector with at least one division value along the divider's
     * dimension. Division values are ordered in increasing order.
     */
    std::vector<std::vector<double>> divisionsByDivider;

    /**
     * A list of quadrants defined using the dividers.
     *
     * The vector will have at least one quadrant entry, but may include
     * enough entries to label every division of the multi-dimensional
     * space by dividers.
     */
    std::vector<Quadrant> quadrants;



//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a new quadrant gate.
     *
    // TODO This constructor is invalid. It has a zero dimension. Instead,
    // this constructor needs arguments for dividers and quadrants.
     */
    QuadrantGate( )
        : Gate( QUADRANT_GATE, 0 )
    {
        // Nothing to do.
    }

    /**
     * Constructs a copy of the given gate.
     *
     * @param[in] gate
     *   The gate to copy.
     */
    QuadrantGate( const QuadrantGate& gate )
        : Gate( gate )
    {
        this->idByDivider = gate.idByDivider;
        this->divisionsByDivider = gate.divisionsByDivider;
        this->quadrants = gate.quadrants;
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~QuadrantGate( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Copy and clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the gate and all of its children.
     *
     * @return
     *   Returns a clone of this gate.
     */
    virtual Gate* clone( )
        const override
    {
        return this->cloneChildren( new QuadrantGate( *this ) );
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the number of dividers.
     *
     * @return
     *   The number of dividers.
     */
    inline DividerIndex getNumberOfDividers( )
        const noexcept
    {
        return this->getNumberOfDimensions( );
    }

#ifdef COMPENSATION_SUPPORTED
    /**
     * Returns the compensation method for the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     *
     * @return
     *   Returns the compensation method.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     */
    inline CompensationMethod getDividerCompensationMethod(
        const DividerIndex index )
        const
    {
        return this->getDimensionCompensationMethod( index );
    }
#endif

    /**
     * Returns the list of divisions for the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     *
     * @return
     *   Returns the list of divisions for the divider.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     *
     * @see ::getNumberOfDividers()
     */
    const std::vector<double>& getDividerDivisions( const DividerIndex index )
        const
    {
        if ( index > this->idByDivider.size( ) )
            throw std::out_of_range(
                "Invalid divider list index out of range." );

        return this->divisionsByDivider[index];
    }

    /**
     * Returns the unique ID within this gate of the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     *
     * @return
     *   Returns the unique divider ID.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     *
     * @see ::getNumberOfDividers()
     * @see ::setDividerId()
     */
    const std::string& getDividerId( const DividerIndex index )
        const
    {
        if ( index > this->idByDivider.size( ) )
            throw std::out_of_range(
                "Invalid divider list index out of range." );

        return this->idByDivider[index];
    }

    /**
     * Returns the event data parameter name for the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     *
     * @return
     *   Returns the parameter name.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     *
     * @see ::getNumberOfDividers()
     * @see ::getDividerId()
     * @see ::setDividerParameterName()
     */
    const std::string& getDividerParameterName( const DividerIndex index )
        const
    {
        return this->getDimensionParameterName( index );
    }

    /**
     * Returns the unique ID within this gate of the indicated quadrant.
     *
     * @param[in] index
     *   The quadrant index.
     *
     * @return
     *   Returns the unique quadrant ID.
     *
     * @throws std::out_of_range
     *   Throws an exception if the quadrant index is out of range.
     *
     * @see ::getNumberOfQuadrants()
     * @see ::setQuadrantId()
     */
    const std::string& getQuadrantId( const QuadrantIndex index )
        const
    {
        if ( index > this->quadrants.size( ) )
            throw std::out_of_range(
                "Invalid quadrant list index out of range." );

        return this->quadrants[index].id;
    }

    /**
     * Returns the transform for the indicated divider, if any.
     *
     * @param[in] index
     *   The divider index.
     *
     * @return
     *   Returns the transform, or a nullptr if there is no transform.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     *
     * @see ::getNumberOfDividers()
     * @see ::setDividerTransform()
     */
    inline std::shared_ptr<Transform> getDividerTransform(
        const DividerIndex index )
        const
    {
        return this->getDimensionParameterTransform( index );
    }

    /**
     * Returns the divider ID for the indicated quadrant's selected
     * position.
     *
     * @param[in] index
     *   The quadrant index.
     * @param[in] pindex
     *   The position index within the quadrant's position list.
     *
     * @return
     *   Returns the divider Id.
     *
     * @throws std::out_of_range
     *   Throws an exception if the quadrant or position list indexes are
     *   out of range.
     *
     * @see ::getNumberOfQuadrants()
     * @see ::getNumberOfQuadrantPositions()
     * @see ::getQuadrantPositionValue()
     */
    const std::string &getQuadrantPositionId(
        const QuadrantIndex index,
        const PositionIndex pindex )
        const
    {
        if ( index > this->quadrants.size( ) )
            throw std::out_of_range(
                "Invalid quadrant list index out of range." );
        if ( pindex > this->quadrants[index].positions.size( ) )
            throw std::out_of_range(
                "Invalid quadrant position list index out of range." );

        return this->quadrants[index].positions[pindex].first;
    }

    /**
     * Returns the value for the indicated quadrant's selected
     * position.
     *
     * @param[in] index
     *   The quadrant index.
     * @param[in] pindex
     *   The position index within the quadrant's position list.
     *
     * @return
     *   Returns the position value for the position's divider.
     *
     * @throws std::out_of_range
     *   Throws an exception if the quadrant or position list indexes are
     *   out of range.
     *
     * @see ::getNumberOfQuadrants()
     * @see ::getNumberOfQuadrantPositions()
     * @see ::getQuadrantPositionId()
     */
    double getQuadrantPositionValue(
        const QuadrantIndex index,
        const PositionIndex pindex )
        const
    {
        if ( index > this->quadrants.size( ) )
            throw std::out_of_range(
                "Invalid quadrant list index out of range." );
        if ( pindex > this->quadrants[index].positions.size( ) )
            throw std::out_of_range(
                "Invalid quadrant position list index out of range." );

        return this->quadrants[index].positions[pindex].second;
    }

    /**
     * Returns the number of quadrants.
     *
     * @return
     *   Returns the number of quadrants.
     *
     * @see ::getQuadrantId()
     * @see ::getNumberOfQuadrantPositions()
     * @see ::getQuadrantPositionId()
     * @see ::getQuadrantPositionValue()
     */
    inline QuadrantIndex getNumberOfQuadrants( )
        const noexcept
    {
        return this->quadrants.size( );
    }

    /**
     * Returns the number of quadrant positions.
     *
     * @param[in] index
     *   The quadrant index.
     *
     * @return
     *   Returns the number of positions for the indicated quadrant.
     *
     * @see ::getQuadrantId()
     * @see ::getNumberOfQuadrants()
     * @see ::getQuadrantPositionId()
     * @see ::getQuadrantPositionValue()
     */
    inline PositionIndex getNumberOfQuadrantPositions(
        const QuadrantIndex index )
        const
    {
        if ( index > this->quadrants.size( ) )
            throw std::out_of_range(
                "Invalid quadrant list index out of range." );

        return this->quadrants[index].positions.size( );
    }


#ifdef COMPENSATION_SUPPORTED
    /**
     * Sets the compensation method for the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     * @param[in] method
     *   The compensation method.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     */
    const void setDividerCompensationMethod(
        const DividerIndex index,
        const CompensationMethod method )
    {
        this->setDimensionCompensationMethod( index, method );
    }
#endif

    /**
     * Sets the list of divisions for the indicated divider.
     *
     * The list of divisions must include at least one division.
     *
     * @param[in] index
     *   The divider index.
     * @param[in] list
     *   The list of divisions along the divider.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the division list does not have at least
     *   one entry.
     *
     * @see ::getNumberOfDividers()
     * @see ::getDividerDivisions()
     */
    void setDividerDivisions(
        const DividerIndex index,
        const std::vector<double>& divisions )
    {
        if ( index > this->idByDivider.size( ) )
            throw std::out_of_range(
                "Invalid divider list index out of range." );
        if ( divisions.size( ) == 0 )
            throw std::invalid_argument(
                "Invalid divider division list is empty." );

        this->divisionsByDivider[index] = divisions;
    }

    /**
     * Sets the unique ID within this gate of the indicated divider.
     *
     * It is up to the caller to insure that divider IDs are unique
     * within the gate.
     *
     * @param[in] index
     *   The divider index.
     * @param[in] id
     *   The divider unique ID.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     * @throws std::invalid_argument
     *   Throws an exception if the divider ID is empty.
     *
     * @see ::getNumberOfDividers()
     * @see ::getDividerId()
     */
    void setDividerId( const DividerIndex index, const std::string id )
    {
        if ( index > this->idByDivider.size( ) )
            throw std::out_of_range(
                "Invalid divider list index out of range." );
        if ( id.empty( ) == true )
            throw std::invalid_argument(
                "Invalid divider ID is empty." );

        this->idByDivider[index] = id;
    }

    /**
     * Sets the event data parameter name for the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     * @param[in] name
     *   The parameter name.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     *
     * @see ::getNumberOfDividers()
     * @see ::getDividerParameterName()
     */
    const void setDividerParameterName(
        const DividerIndex index,
        const std::string& name )
    {
        this->setDimensionParameterName( index, name );
    }

    /**
     * Sets the transform name for the indicated divider.
     *
     * @param[in] index
     *   The divider index.
     * @param[in] name
     *   The transform name.
     *
     * @throws std::out_of_range
     *   Throws an exception if the divider index is out of range.
     *
     * @see ::getNumberOfDividers()
     * @see ::getDividerTransform()
     */
    const void setDividerTransform(
        const DividerIndex index,
        std::shared_ptr<Transform> transform )
    {
        this->setDimensionParameterTransform( index, transform );
    }

    // TODO set methods are needed for the quadrant list.
    // @}



//----------------------------------------------------------------------
// Utilities.
//----------------------------------------------------------------------
public:
    /**
     * @name Utilities
     */
    // @{
    /**
     * Returns a string representation of the gate.
     *
     * @param[in] indent
     *   (optional, default = "") The indent for each line in the string.
     *
     * @return
     *   Returns a string.
     */
    virtual std::string toString( const std::string& indent = "" )
        const noexcept
    {
        std::string s = Gate::toString( indent );

        s += indent + "  dividers:\n";
        const DividerIndex n = this->getNumberOfDividers( );
        for ( DividerIndex i = 0; i < n; ++i )
        {
            s += "    \"" + this->idByDivider[i] + "\"=(";

            const auto divisions = this->divisionsByDivider[i];
            const auto nd = divisions.size( );
            for ( PositionIndex j = 0; j < nd; ++j )
            {
                s += std::to_string( divisions[j] );
                if ( j != (nd-1) )
                    s += ", ";
            }
            s += "\n";
        }

        s += indent + "  quadrants:\n";
        const QuadrantIndex nq = this->quadrants.size( );
        for ( QuadrantIndex i = 0; i < nq; ++i )
        {
            const Quadrant& q = this->quadrants[i];

            s += "    \"" + q.id + "\"=(";

            const auto np = q.positions.size( );
            for ( PositionIndex j = 0; j < np; ++j )
            {
                s += q.positions[j].first + "@" +
                    std::to_string( q.positions[j].second );
                if ( j != (np-1) )
                    s += ", ";
            }
            s += "\n";
        }

        return s;
    }
    // @}
};



//----------------------------------------------------------------------
//
// GateTrees and GateTree State.
//
//----------------------------------------------------------------------
/**
 * Defines an interface for gate trees state.
 */
class GateTreesStateInterface
{
//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
protected:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a gate tree state object.
     */
    GateTreesStateInterface( )
    {
        // Do nothing.
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GateTreesStateInterface( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Callbacks.
//----------------------------------------------------------------------
public:
    /**
     * @name Callbacks
     */
    // @{
    /**
     * Updates state when a gate root is appended to the root list.
     *
     * @param[in] root
     *   The gate tree root to append.
     */
    virtual void callbackAppendGateTree( const std::shared_ptr<Gate>& root )
        noexcept = 0;

    /**
     * Updates state when the gate tree root list is cleared.
     */
    virtual void callbackClearGateTrees( )
        noexcept = 0;

    /**
     * Updates state when a gate tree root is removed from the list.
     *
     * @param[in] root
     *   The gate tree root to remove.
     */
    virtual void callbackRemoveGateTree( const std::shared_ptr<Gate>& root )
        noexcept = 0;

    /**
     * Updates state when the creator software name is changed.
     *
     * @param[in] name
     *   The software name.
     */
    virtual void callbackSetCreatorSoftwareName( const std::string& name )
        noexcept = 0;

    /**
     * Updates state when the FCS software file name is changed.
     *
     * @param[in] fileName
     *   The FCS file name.
     */
    virtual void callbackSetFCSFileName( const std::string& fileName )
        noexcept = 0;

    /**
     * Updates state when the file name is changed.
     *
     * @param[in] fileName
     *   The FCS file name.
     */
    virtual void callbackSetFileName( const std::string& fileName )
        noexcept = 0;

    /**
     * Updates state when the description is changed.
     *
     * @param[in] fileName
     *   The file name.
     */
    virtual void callbackSetDescription( const std::string& description )
        noexcept = 0;

    /**
     * Updates state when the name is changed.
     *
     * @param[in] name
     *   The name.
     */
    virtual void callbackSetName( const std::string& name )
        noexcept = 0;

    /**
     * Updates state when the diagnostic notes are changed.
     *
     * @param[in] fileName
     *   The file name.
     */
    virtual void callbackSetNotes( const std::string& notes )
        noexcept = 0;
    // @}
};





/**
 * Defines the base class for optional gate tree state.
 *
 * Gate trees state may be attached to any gate. When methods change
 * a GateTrees object, the method calls the gate tree state's callback
 * afterwards to let the state object update any further state it may be
 * maintaining.
 */
class GateTreesStateBase
    : public GateTreesStateInterface
{
//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
protected:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Constructs a gate tree state object.
     */
    GateTreesStateBase( )
    {
        // Do nothing.
    }
    // @}

public:
    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GateTreesStateBase( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Callbacks.
//----------------------------------------------------------------------
public:
    /**
     * @name Callbacks
     */
    // @{
    /**
     * Updates state when a gate root is appended to the root list.
     *
     * @param[in] root
     *   The gate tree root to append.
     */
    virtual void callbackAppendGateTree( const std::shared_ptr<Gate>& root )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the gate tree root list is cleared.
     */
    virtual void callbackClearGateTrees( )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when a gate tree root is removed from the list.
     *
     * @param[in] root
     *   The gate tree root to remove.
     */
    virtual void callbackRemoveGateTree( const std::shared_ptr<Gate>& root )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the creator software name is changed.
     *
     * @param[in] name
     *   The software name.
     */
    virtual void callbackSetCreatorSoftwareName( const std::string& name )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the FCS software file name is changed.
     *
     * @param[in] fileName
     *   The FCS file name.
     */
    virtual void callbackSetFCSFileName( const std::string& fileName )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the file name is changed.
     *
     * @param[in] fileName
     *   The FCS file name.
     */
    virtual void callbackSetFileName( const std::string& fileName )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the description is changed.
     *
     * @param[in] fileName
     *   The file name.
     */
    virtual void callbackSetDescription( const std::string& description )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the name is changed.
     *
     * @param[in] name
     *   The name.
     */
    virtual void callbackSetName( const std::string& name )
        noexcept
    {
        // Do nothing.
    }

    /**
     * Updates state when the notes is changed.
     *
     * @param[in] fileName
     *   The file name.
     */
    virtual void callbackSetNotes( const std::string& notes )
        noexcept
    {
        // Do nothing.
    }
    // @}
};





/**
 * Describes a collection of gate trees.
 *
 * The class maintains an unordered list of gate tree roots. Each tree
 * root is a gate that may have children, who may have children, and so
 * forth.
 *
 * Each gate classifies fluorescence cytometry events into those inside
 * the gate and those outside. A child gate further classifies the events
 * inside its parent's gate, dividing them into those inside and outside
 * the child's gate.
 *
 *
 * <B>Gate trees</B><BR>
 * Methods query the number of gate trees and their roots:
 * @code
 * // Get the number of trees.
 * GateIndex nTrees = gatetrees->getNumberOfGateTrees();
 *
 * // Loop over all gate trees.
 * for ( GateIndex i = 0; i < nTrees; ++i )
 * {
 *   auto gate = gatetrees->getGateTree( i );
 *   ...
 * }
 * @endcode
 *
 * Gate trees may be appended to and removed from the gate tree list:
 * @code
 * // Append gate tree.
 * gatetrees->appendGateTree( gate );
 *
 * // Remove gate tree.
 * gatetrees->removeGateTree( gate );
 *
 * // Clear all gate trees.
 * gatetrees->clearGateTrees( );
 * @endcode
 */
class GateTrees final
{
//----------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------
public:
    // Name and version ------------------------------------------------
    /**
     * The software name.
     */
    inline static const std::string NAME = "FlowGate Gate Trees";

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
// Fields.
//----------------------------------------------------------------------
private:
    // Gating-ML features ----------------------------------------------
    /**
     * The unordered list of gate tree roots
     *
     * Gates in the list are in the order in which they were added to
     * the list.
     *
     * @see ::getGateTree()
     */
    std::vector<std::shared_ptr<Gate>> roots;

    /**
     * The optional gate tree state.
     *
     * @see ::getState()
     */
    std::shared_ptr<GateTreesStateInterface> state;

    // FlowGate-specific features --------------------------------------
    /**
     * The optional name of the gate trees.
     *
     * The name defaults to the file name, if any, that contained the
     * gate tree (e.g. a Gating-ML file).
     *
     * In Gating-ML, this is found in in custom information for the file:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <name>...</name>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * This also may be found by importing Cytobank features in custom
     * information for the file:
     * @code
     * <data-type:custom_info>
     *   <cytobank>
     *     <experiment_title>...</experiment_title>
     *   </cytobank>
     * </data-type:custom_info>
     * @endcode
     *
     * Elsewhere Cytobank uses a <name> node for gate names, so a name
     * for the gate tree set also may be found in:
     * @code
     * <data-type:custom_info>
     *   <cytobank>
     *     <name>...</name>
     *   </cytobank>
     * </data-type:custom_info>
     * @endcode
     *
     * Or generically in custom information for the file:
     * @code
     * <data-type:custom_info>
     *   ...
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getName()
     */
    std::string name;

    /**
     * The optional description of the gate trees.
     *
     * In Gating-ML, this is found in custom information for the file:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <description>...</description>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * This also may be found by importing Cytobank features in custom
     * information for the file:
     * @code
     * <data-type:custom_info>
     *   <cytobank>
     *     <about>...</about>
     *   </cytobank>
     * </data-type:custom_info>
     * @endcode
     *
     * Or generically in custom information for the file:
     * @code
     * <data-type:custom_info>
     *   ...
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getDescription()
     */
    std::string description;

    /**
     * The optional diagnostic notes of the gate trees.
     *
     * Diagnostic notes are considered personal information and are removed
     * from the gate tree during de-identiication.
     *
     * In Gating-ML, this is found in custom information for the file:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <notes>...</notes>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getNotes()
     */
    std::string notes;

    /**
     * The optional file name containing the gate trees.
     *
     * The file name may be set when the gate trees are loaded from a file,
     * if any.
     *
     * @see ::getFileName( )
     */
    std::string fileName;

    /**
     * The optional file name of an FCS file used as a template when authoring
     * the gate trees.
     *
     * While the same gate trees may be applied to many FCS files, they are
     * often authored using a representative template FCS file.
     *
     * The FCS file name is considered personal information and is removed
     * from the gate tree during de-identiication.
     *
     * In Gating-ML, this is found in custom information for the file:
     * @code
     * <data-type:custom_info>
     *   <flowgate>
     *     <fcs-filename>...</fcs-filename>
     *   </flowgate>
     * </data-type:custom_info>
     * @endcode
     *
     * This also may be found by importing Cytobank features in custom
     * information. While this is not usually at the top of the file, it
     * may be embeded later on:
     * @code
     * <data-type:custom_info>
     *   <cytobank>
     *     <fcs_file_filename>...</fcs_file_filename>
     *   </cytobank>
     * </data-type:custom_info>
     * @endcode
     *
     * @see ::getFCSFileName()
     */
    std::string fcsFileName;

    /**
     * The optional name of the software used to create the gate trees.
     *
     * In Gating-ML, this may be inferred from the use of custom information
     * for the file. If a <flowgate> node is found, the software name will
     * be for FlowGate. If a <cytobank> node is found, the software name
     * will be for Cytobank, etc.
     *
     * @see ::getCreatorSoftwareName( )
     */
    std::string creatorSoftwareName;




//----------------------------------------------------------------------
// Constructors / destructors.
//----------------------------------------------------------------------
public:
    /**
     * @name Constructors
     */
    // @{
    /**
     * Creates an empty list of gate trees.
     */
    GateTrees( )
        noexcept
    {
        this->creatorSoftwareName = NAME + " version " + VERSION;
    }

    /**
     * Creates a copy of the given list of gate trees.
     *
     * @param[in] gateTrees
     *   The gate trees to copy.
     */
    GateTrees( const GateTrees& gateTrees )
        noexcept
    {
        this->copy( gateTrees );
        if ( this->creatorSoftwareName.empty( ) == true )
            this->creatorSoftwareName = NAME + " version " + VERSION;
    }

    /**
     * Creates a copy of the given list of gate trees.
     *
     * @param[in] gateTrees
     *   The gate trees to copy.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate tree list is a nullptr pointer.
     */
    GateTrees( std::shared_ptr<GateTrees> gateTrees )
    {
        if ( gateTrees == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate trees." );
        this->copy( *(gateTrees.get()) );
        if ( this->creatorSoftwareName.empty( ) == true )
            this->creatorSoftwareName = NAME + " version " + VERSION;
    }
    // @}

    /**
     * @name Destructors
     */
    // @{
    /**
     * Destroys the object.
     */
    virtual ~GateTrees( )
        noexcept
    {
        // Nothing to do.
    }
    // @}



//----------------------------------------------------------------------
// Attributes.
//----------------------------------------------------------------------
public:
    /**
     * @name Attributes
     */
    // @{
    /**
     * Returns the optional software name used to create the gate trees.
     *
     * When gate trees are loaded from a file, this is initialized to the
     * explicit or inferred software package name used to create that file.
     * Otherwise, this defaults to the name of this FlowGate software.
     *
     * @return
     *   Returns the software name, or an empty string if no name
     *   has been set.
     *
     * @see ::setCreatorSoftwareName()
     */
    inline const std::string& getCreatorSoftwareName( )
        const noexcept
    {
        return this->creatorSoftwareName;
    }

    /**
     * Returns the optional description for the gate trees.
     *
     * The description defaults to the empty string.
     *
     * @return
     *   Returns the description, or an empty string if no description
     *   has been set.
     *
     * @see ::setDescription()
     */
    inline const std::string& getDescription( )
        const noexcept
    {
        return this->description;
    }

    /**
     * Returns the optional FCS file name used when authoring or using the
     * gate trees.
     *
     * While the same gate trees may be applied to many FCS files, they are
     * often authored using a representative template FCS file.
     *
     * The FCS file name defaults to the empty string.
     *
     * The FCS file name is considered personal information and is removed
     * from the gate tree during de-identiication.
     *
     * @return
     *   Returns the FCS file name, or an empty string if no FCS file name has
     *   been set.
     *
     * @see ::getName()
     * @see ::getFileName()
     * @see ::setFCSFileName()
     */
    inline const std::string& getFCSFileName( )
        const noexcept
    {
        return this->fcsFileName;
    }

    /**
     * Returns the optional file name for the gate trees.
     *
     * The name defaults to the file name, if any, that contained the
     * gate trees (e.g. a Gating-ML file).
     *
     * @return
     *   Returns the file name, or an empty string if no file name has
     *   been set.
     *
     * @see ::getName()
     * @see ::getFCSFileName()
     * @see ::setFileName()
     */
    inline const std::string& getFileName( )
        const noexcept
    {
        return this->fileName;
    }

    /**
     * Returns the optional name for the gate trees.
     *
     * The name defaults to the file name, if any, that contained the
     * gate trees (e.g. a Gating-ML file).
     *
     * @return
     *   Returns the name, or an empty string if no name has been set.
     *
     * @see ::getFCSFileName()
     * @see ::getFileName()
     * @see ::setName()
     */
    inline const std::string& getName( )
        const noexcept
    {
        return this->name;
    }

    /**
     * Returns the optional diagnostic notes for the gate trees.
     *
     * The diagnostic notes defaults to the empty string.
     *
     * Diagnostic notes are considered personal information and are removed
     * from the gate tree during de-identiication.
     *
     * @return
     *   Returns the diagnostic notes, or an empty string if no diagnostic notes
     *   has been set.
     *
     * @see ::setNotes()
     */
    inline const std::string& getNotes( )
        const noexcept
    {
        return this->notes;
    }



    /**
     * Sets the optional software name used to create the gate trees.
     *
     * When gate trees are loaded from a file, this may be set to the
     * explicit or inferred software package name used to create that file.
     * Otherwise, this defaults to the name of this FlowGate software.
     *
     * @param[in] name
     *   The software name. An empty string clears the name.
     *
     * @see ::getCreatorSoftwareName()
     */
    inline void setCreatorSoftwareName( const std::string& name )
        noexcept
    {
        if ( this->creatorSoftwareName == name )
            return;

        this->creatorSoftwareName = name;
        if ( this->state != nullptr )
            this->state->callbackSetCreatorSoftwareName( name );
    }

    /**
     * Sets the optional FCS file name used when authoring or using the
     * gate trees.
     *
     * While the same gate trees may be applied to many FCS files, they are
     * often authored using a representative template FCS file.
     *
     * The FCS filename is considered personal information and is removed
     * from the gate tree during de-identiication.
     *
     * @param[in] fileName
     *   The FCS file name. An empty string clears the name.
     *
     * @see ::getFCSFileName()
     * @see ::setFileName()
     * @see ::setName()
     */
    inline void setFCSFileName( const std::string& fileName )
        noexcept
    {
        if ( this->fcsFileName == fileName )
            return;

        this->fcsFileName = fileName;
        if ( this->state != nullptr )
            this->state->callbackSetFCSFileName( fileName );
    }

    /**
     * Sets the optional file name for the gate trees.
     *
     * @param[in] fileName
     *   The file name.
     *
     * @see ::getFileName()
     * @see ::setFCSFileName()
     * @see ::setName()
     */
    inline void setFileName( const std::string& fileName )
        noexcept
    {
        if ( this->fileName == fileName )
            return;

        this->fileName = fileName;
        if ( this->state != nullptr )
            this->state->callbackSetFileName( fileName );
    }

    /**
     * Sets the optional description for the gate trees.
     *
     * @param[in] description
     *   The description. An empty string clears the description.
     *
     * @see ::getDescription()
     */
    inline void setDescription( const std::string& description )
        noexcept
    {
        if ( this->description == description )
            return;

        this->description = description;
        if ( this->state != nullptr )
            this->state->callbackSetDescription( description );
    }

    /**
     * Sets the optional name for the gate trees.
     *
     * @param[in] name
     *   The name.
     *
     * @see ::getName()
     * @see ::setFCSFileName()
     * @see ::setFileName()
     */
    inline void setName( const std::string& name )
        noexcept
    {
        if ( this->name == name )
            return;

        this->name = name;
        if ( this->state != nullptr )
            this->state->callbackSetName( name );
    }

    /**
     * Sets the optional diagnostic notes for the gate trees.
     *
     * Diagnostic notes are considered personal information and are removed
     * from the gate tree during de-identiication.
     *
     * @param[in] notes
     *   The notes. An empty string clears the notes.
     *
     * @see ::getNotes()
     */
    inline void setNotes( const std::string& notes )
        noexcept
    {
        if ( this->notes == notes )
            return;

        this->notes = notes;
        if ( this->state != nullptr )
            this->state->callbackSetNotes( notes );
    }
    // @}



//----------------------------------------------------------------------
// Copy & clone.
//----------------------------------------------------------------------
public:
    /**
     * @name De-identify
     */
    // @{
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
     * @li Remove the FCS file name for the gate tree.
     *
     * @li Remove the diagnostic notes for all gate trees.
     *
     * @li Remove the diagnostic notes for all gates in all agate trees.
     */
    inline void deidentify( )
        noexcept
    {
        this->setFCSFileName( "" );
        this->setNotes( "" );

        auto gates = this->findDescendentGates( );
        for ( auto& g : gates )
        {
            g->setNotes( "" );
        }
    }
    // @}



//----------------------------------------------------------------------
// Copy & clone.
//----------------------------------------------------------------------
public:
    /**
     * @name Copy and clone
     */
    // @{
    /**
     * Clones the gate tree list.
     *
     * @return
     *   Returns a clone of this gate tree.
     *
     * @see ::GateTrees()
     * @see ::copy()
     */
    inline GateTrees* clone( )
        const noexcept
    {
        return new GateTrees( *this );
    }

    /**
     * Copies the gate trees into this gate tree list.
     *
     * @param[in] gateTrees
     *   The gate trees to copy.
     *
     * @see ::clone()
     */
    inline void copy( const GateTrees& gateTrees )
    {
        // Copy trees.
        const GateIndex nRoots = gateTrees.roots.size( );
        for ( GateIndex i = 0; i < nRoots; ++i )
        {
            std::shared_ptr<Gate> root;
            root.reset( gateTrees.roots[i]->clone( ) );
            this->roots.push_back( root );
        }

        // Copy annotation.
        this->name                = gateTrees.name;
        this->description         = gateTrees.description;
        this->notes               = gateTrees.notes;
        this->fileName            = gateTrees.fileName;
        this->fcsFileName         = gateTrees.fcsFileName;
        this->creatorSoftwareName = gateTrees.creatorSoftwareName;

        // State is not cloned.
        this->state = nullptr;
    }

    /**
     * Copies the gate trees into this gate tree list.
     *
     * @param[in] gateTrees
     *   The gate trees to copy.
     *
     * @see ::clone()
     */
    inline void copy( std::shared_ptr<GateTrees> gateTrees )
    {
        // Copy trees.
        const GateIndex nRoots = gateTrees->roots.size( );
        for ( GateIndex i = 0; i < nRoots; ++i )
        {
            std::shared_ptr<Gate> root;
            root.reset( gateTrees->roots[i]->clone( ) );
            this->roots.push_back( root );
        }

        // Copy annotation.
        this->name                = gateTrees->name;
        this->description         = gateTrees->description;
        this->notes               = gateTrees->notes;
        this->fileName            = gateTrees->fileName;
        this->fcsFileName         = gateTrees->fcsFileName;
        this->creatorSoftwareName = gateTrees->creatorSoftwareName;

        // State is not cloned.
        this->state = nullptr;
    }
    // @}



//----------------------------------------------------------------------
// Tree get, append, and remove
//----------------------------------------------------------------------
public:
    /**
     * @name Tree get, append, and remove
     */
    // @{
    /**
     * Appends a gate tree to the list.
     *
     * @param[in] root
     *   The gate tree root to append.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr pointer or if it
     *   has a parent (and thus is not a gate tree root).
     *
     * @see ::getGateTree()
     * @see ::removeGateTree()
     * @see ::clearGateTrees()
     */
    inline void appendGateTree( const std::shared_ptr<Gate>& root )
    {
        // Validate.
        if ( root == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate." );

        this->roots.push_back( root );

        if ( this->state != nullptr )
            this->state->callbackAppendGateTree( root );
    }

    /**
     * Clears the gate tree list.
     *
     * @see ::removeGateTree()
     */
    inline void clearGateTrees( )
        noexcept
    {
        this->roots.clear( );

        if ( this->state != nullptr )
            this->state->callbackClearGateTrees( );
    }

    /**
     * Returns the indicated gate from the list.
     *
     * @param[in] index
     *   The gate list index.
     *
     * @return
     *   Returns the gate.
     *
     * @throws std::out_of_range
     *   Throws an exception if the gate list index is out of range.
     *
     * @see ::appendGateTree()
     * @see ::getNumberOfGateTrees()
     * @see ::removeGateTree()
     */
    inline std::shared_ptr<const Gate> getGateTree(
        const GateIndex index )
        const
    {
        // Validate.
        if ( index >= this->roots.size( ) )
            throw std::out_of_range( "Invalid gate list index out of range." );

        return this->roots[index];
    }

    /**
     * Returns the indicated gate from the list.
     *
     * @param[in] index
     *   The gate list index.
     *
     * @return
     *   Returns the gate.
     *
     * @throws std::out_of_range
     *   Throws an exception if the gate list index is out of range.
     *
     * @see ::appendGateTree()
     * @see ::getNumberOfGateTrees()
     * @see ::removeGateTree()
     */
    inline std::shared_ptr<Gate> getGateTree( const GateIndex index )
    {
        // Validate.
        if ( index >= this->roots.size( ) )
            throw std::out_of_range( "Invalid gate list index out of range." );

        return this->roots[index];
    }

    /**
     * Returns the number of gate trees defined.
     *
     * @return
     *   Returns the number of gate trees.
     *
     * @see ::getGateTree()
     */
    inline GateIndex getNumberOfGateTrees( )
        const noexcept
    {
        return this->roots.size( );
    }

    /**
     * Deletes a gate tree from the list.
     *
     * @param[in] index
     *   The gate list index.
     *
     * @throws std::out_of_range
     *   Throws an exception if the gate list index is out of range.
     *
     * @see ::appendGateTree()
     * @see ::getNumberOfGateTrees()
     * @see ::clearGateTrees()
     */
    inline void removeGateTree( const GateIndex index )
    {
        // Validate.
        if ( index >= this->roots.size( ) )
            throw std::out_of_range( "Invalid gate list index out of range." );

        auto gate = this->roots[index];
        this->roots.erase( this->roots.begin() + index );

        if ( this->state != nullptr )
            this->state->callbackRemoveGateTree( gate );
    }

    /**
     * Deletes a gate tree root from the list.
     *
     * @param[in] root
     *   The gate tree root to remove.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr pointer or is not in
     *   the gate tree list.
     *
     * @see ::appendGateTree()
     * @see ::getNumberOfGateTrees()
     * @see ::clearGateTrees()
     */
    inline void removeGateTree( const std::shared_ptr<Gate>& root )
    {
        // Validate.
        if ( root == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate." );

        const GateIndex nGates = this->roots.size( );
        for ( GateIndex i = 0; i < nGates; ++i )
        {
            if ( this->roots[i] == root )
            {
                this->roots.erase( this->roots.begin() + i );

                if ( this->state != nullptr )
                    this->state->callbackRemoveGateTree( root );
                return;
            }
        }

        throw std::invalid_argument( "Invalid gate is not in the gate list." );
    }
    // @}



//----------------------------------------------------------------------
// Tree search for gates.
//----------------------------------------------------------------------
public:
    /**
     * @name Tree search for gates
     */
    // @{
    /**
     * Returns a list of all root gates and their descendents.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children.
     *
     * @see ::findDescendentGatesWithParentIndexes()
     * @see ::findNumberOfDescendentGates()
     * @see ::findNumberOfDescendentTransforms()
     */
    inline std::vector<std::shared_ptr<const Gate>> findDescendentGates( )
        const noexcept
    {
        std::vector<std::shared_ptr<const Gate>> gates;
        const GateIndex nRoots = this->roots.size( );
        for ( GateIndex i = 0; i < nRoots; ++i )
        {
            gates.push_back( this->roots[i] );
            const auto descendents = this->roots[i]->findDescendentGates( );
            const auto n = descendents.size( );
            for ( GateIndex j = 0; j < n; ++j )
                gates.push_back( descendents[j] );
        }

        return gates;
    }

    /**
     * Returns a list of all root gates and their descendents.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children.
     *
     * @see ::findDescendentGatesWithParentIndexes()
     * @see ::findNumberOfDescendentGates()
     * @see ::findNumberOfDescendentTransforms()
     */
    inline std::vector<std::shared_ptr<Gate>> findDescendentGates( )
        noexcept
    {
        std::vector<std::shared_ptr<Gate>> gates;
        const GateIndex nRoots = this->roots.size( );
        for ( GateIndex i = 0; i < nRoots; ++i )
        {
            gates.push_back( this->roots[i] );
            const auto descendents = this->roots[i]->findDescendentGates( );
            const auto n = descendents.size( );
            for ( GateIndex j = 0; j < n; ++j )
                gates.push_back( descendents[j] );
        }

        return gates;
    }

    /**
     * Returns a list of all root gates and their descendents, including
     * parent indexing.
     *
     * The returned vector contains pairs where each pair as:
     * @li A vector index for the parent gate.
     * @li The gate.
     *
     * The parent vector index may be used to get the parent gate for an
     * entry:
     * @code
     * const auto& [parentIndex, gate] = vector[i];
     * const auto& [grandParentIndex, parentGate] = vector[parentIndex];
     * @endcode
     *
     * Root gates have parent indexes that are the same as their own index.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children.
     *
     * @see ::findDescendentGates()
     * @see ::findNumberOfDescendentGates()
     * @see ::findNumberOfDescendentTransforms()
     */
    inline std::vector<std::pair<GateIndex,std::shared_ptr<const Gate>>> findDescendentGatesWithParentIndexes( )
        const noexcept
    {
        std::vector<std::pair<GateIndex,std::shared_ptr<const Gate>>> gates;
        for ( const auto& root : this->roots )
        {
            const GateIndex rootIndex = gates.size( );
            gates.push_back( std::make_pair( rootIndex, root ) );

            root->findDescendentGatesWithParentIndexes( gates, rootIndex );
        }

        return gates;
    }

    /**
     * Returns a list of all root gates and their descendents, including
     * parent indexing.
     *
     * The returned vector contains pairs where each pair as:
     * @li A vector index for the parent gate.
     * @li The gate.
     *
     * The parent vector index may be used to get the parent gate for an
     * entry:
     * @code
     * auto& [parentIndex, gate] = vector[i];
     * auto& [grandParentIndex, parentGate] = vector[parentIndex];
     * @endcode
     *
     * Root gates have parent indexes that are the same as their own index.
     *
     * @return
     *   Returns a vector of gates. Gates in the vector are ordered so that
     *   parents are always before their children.
     *
     * @see ::findDescendentGates()
     * @see ::findNumberOfDescendentGates()
     * @see ::findNumberOfDescendentTransforms()
     */
    inline std::vector<std::pair<GateIndex,std::shared_ptr<Gate>>> findDescendentGatesWithParentIndexes( )
        noexcept
    {
        std::vector<std::pair<GateIndex,std::shared_ptr<Gate>>> gates;
        for ( auto& root : this->roots )
        {
            const GateIndex rootIndex = gates.size( );
            gates.push_back( std::make_pair( rootIndex, root ) );

            root->findDescendentGatesWithParentIndexes( gates, rootIndex );
        }

        return gates;
    }

    /**
     * Returns the gate, if any, with the given ID.
     *
     * This method looks through the gate tree for a gate with the
     * given ID. The gate is returned, or a nullptr if no gate is
     * found. Since IDs are unique, there should never be more than one
     * match.
     *
     * @param[in] id
     *   The gate ID to look up.
     *
     * @return
     *   Returns the gate, or a nullptr.
     */
    inline std::shared_ptr<const Gate> findGateById( const GateId id )
        const noexcept
    {
        const auto gates = this->findDescendentGates( );

        for ( auto& g : gates )
        {
            if ( g->getId() == id )
                return g;
        }

        return nullptr;
    }

    /**
     * Returns the gate, if any, with the given ID.
     *
     * This method looks through the gate tree for a gate with the
     * given ID. The gate is returned, or a nullptr if no gate is
     * found. Since IDs are unique, there should never be more than one
     * match.
     *
     * @param[in] id
     *   The gate ID to look up.
     *
     * @return
     *   Returns the gate, or a nullptr.
     */
    inline std::shared_ptr<Gate> findGateById( const GateId id )
        noexcept
    {
        auto gates = this->findDescendentGates( );

        for ( auto& g : gates )
        {
            if ( g->getId() == id )
                return g;
        }

        return nullptr;
    }

    /**
     * Returns the number of gates defined.
     *
     * This method recurses downward through all gate trees to count the
     * total number of gates involved.
     *
     * @return
     *   Returns the number of gates.
     *
     * @see ::getNumberOfGateTrees()
     * @see ::findNumberOfDescendentTransforms()
     * @see ::findDescendentGates()
     * @see ::findDescendentGatesWithParentIndexes()
     */
    inline GateIndex findNumberOfDescendentGates( )
        const noexcept
    {
        GateIndex sum = 0;
        const GateIndex nRoots = this->roots.size( );
        for ( GateIndex i = 0; i < nRoots; ++i )
            sum += this->roots[i]->findNumberOfDescendentGates( );
        return sum;
    }

    /**
     * Returns the parent of the given gate.
     *
     * @param[in] gate
     *   The gate for whome to find the parent.
     *
     * @return
     *   Returns the parent gate, or a nullptr if the gate is a root.
     *
     * @throws std::invalid_argument
     *   Throws an exception if the gate is a nullptr or if it cannot be
     *   found in the gate trees.
     */
    inline std::shared_ptr<Gate> findParentGate(
        std::shared_ptr<Gate> gate )
        const
    {
        if ( gate == nullptr )
            throw std::invalid_argument( "Invalid nullptr gate." );

        std::vector<std::shared_ptr<Gate>> pending;
        const GateIndex nRoots = this->roots.size( );
        for ( GateIndex i = 0; i < nRoots; ++i )
        {
            auto root = this->roots[i];

            // If the gate is a root, it has no parent. Return nullptr.
            if ( gate == root )
                return nullptr;

            pending.push_back( root );
        }

        while ( pending.empty( ) == false )
        {
            auto parent = pending.back( );
            pending.pop_back( );

            // If any of the parent's children match, return the parent.
            const GateIndex nChildren = parent->getNumberOfChildren( );
            for ( GateIndex i = 0; i < nChildren; ++i )
            {
                const auto child = parent->getChild( i );
                if ( child == gate )
                    return parent;
                if ( child->getNumberOfChildren( ) > 0 )
                    pending.push_back( child );
            }
        }

        throw std::invalid_argument( "Gate not found." );
    }
    // @}



//----------------------------------------------------------------------
// Tree search for transforms.
//----------------------------------------------------------------------
public:
    /**
     * @name Tree search for transforms
     */
    // @{
    /**
     * Returns the number of transforms defined.
     *
     * This method recurses downward through all gate trees to count the
     * total number of transforms involved.
     *
     * @return
     *   Returns the number of transforms.
     *
     * @see ::getNumberOfGateTrees()
     * @see ::findNumberOfDescendentGates()
     * @see ::findDescendentGates()
     * @see ::findDescendentGatesWithParentIndexes()
     */
    inline TransformIndex findNumberOfDescendentTransforms( )
        const noexcept
    {
        TransformIndex sum = 0;
        for ( TransformIndex i = 0; i < this->roots.size( ); ++i )
            sum += this->roots[i]->findNumberOfDescendentTransforms( );
        return sum;
    }

    /**
     * Returns the transform, if any, with the given ID.
     *
     * This method looks through the gate tree for a transform with the
     * given ID. The transform is returned, or a nullptr if no transform is
     * found.
     *
     * Each gate is checked, and each of the gate's dimension transforms
     * and additional clustering parameters. Since IDs are unique, there
     * should never be more than one match.
     *
     * @param[in] id
     *   The transform ID to look up.
     *
     * @return
     *   Returns the transform, or a nullptr.
     *
     * @see Gate::findTransformById()
     */
    inline std::shared_ptr<const Transform> findTransformById(
        const TransformId id )
        const noexcept
    {
        const auto gates = this->findDescendentGates( );

        for ( auto& g : gates )
        {
            const auto t = g->findTransformById( id );
            if ( t != nullptr )
                return t;
        }

        return nullptr;
    }

    /**
     * Returns the transform, if any, with the given ID.
     *
     * This method looks through the gate tree for a transform with the
     * given ID. The transform is returned, or a nullptr if no transform is
     * found.
     *
     * Each gate is checked, and each of the gate's dimension transforms
     * and additional clustering parameters. Since IDs are unique, there
     * should never be more than one match.
     *
     * @param[in] id
     *   The transform ID to look up.
     *
     * @return
     *   Returns the transform, or a nullptr.
     *
     * @see Gate::findTransformById()
     */
    inline std::shared_ptr<Transform> findTransformById(
        const TransformId id )
        noexcept
    {
        auto gates = this->findDescendentGates( );

        for ( auto& g : gates )
        {
            auto t = g->findTransformById( id );
            if ( t != nullptr )
                return t;
        }

        return nullptr;
    }
    // @}
};

} // End Gates namespace
} // End FlowGate namespace
