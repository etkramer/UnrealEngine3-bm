//------------------------------------------------------------------------------
// Represents the transformations which may take place along links.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

// Adding new link function types is easy:
// ---------------------------------------
// [1] Create a class for the new type.  Use the built-in functions as guides.
// [2] Increment FX_NUM_LINK_FUNCTIONS in FxLinkFn.cpp.
// [3] Initialize the new link function in FxLinkFn::Startup() in FxLinkFn.cpp
//     using the built-in functions as guides.
// [4] Add a new enum to FxLinkFnType and update FxLinkFnEval().

#ifndef FxLinkFn_H__
#define FxLinkFn_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"
#include "FxArchive.h"
#include "FxArray.h"
#include "FxString.h"
#include "FxMath.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

/// A name and default value for a FxLinkFnParameter.
/// \ingroup faceGraph
struct FxLinkFnParameterDesc
{
	FxLinkFnParameterDesc();
	FxLinkFnParameterDesc( const FxChar* name, FxReal defaultValue );

	FxName parameterName;
	FxReal parameterDefaultValue;

private:
	FxLinkFnParameterDesc& operator=( const FxLinkFnParameterDesc& other );
};

/// Macro to help define a link function.
/// \ingroup faceGraph
#define FX_DECLARE_LINKFN() \
	private: \
		FxString _fnDesc; \
		FxArray<FxLinkFnParameterDesc> _parameters; \
	public: \
		virtual const FxString& GetFnDesc( void ) const \
		{ \
			return _fnDesc; \
		} \
		virtual FxSize GetNumParams( void ) const \
		{ \
			return _parameters.Length(); \
		} \
		virtual const FxString& GetParamName( FxSize index ) const \
		{ \
			return _parameters[index].parameterName.GetAsString(); \
		} \
		virtual FxReal GetParamDefaultValue( FxSize index ) const \
		{ \
			return _parameters[index].parameterDefaultValue; \
		} 

/// The parameters to a FxLinkFn.
/// \ingroup faceGraph
struct FxLinkFnParameters
{
	/// The parameter array.
	FxArray<FxReal> parameters;
	
	/// Serializes the parameters to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxLinkFnParameters& key );
};

/// The link function types.
enum FxLinkFnType
{
	LFT_Invalid = -1,		///< No link function.
	LFT_Null,				///< Null or deprecated.
	LFT_Linear,				///< Linear.
	LFT_Quadratic,			///< Quadratic.
	LFT_Cubic,				///< Cubic.
	LFT_Sqrt,				///< Square root.
	LFT_Negate,				///< Negate.
	LFT_Inverse,			///< Inverse.
	LFT_OneClamp,			///< One clamp.
	LFT_Constant,			///< Constant.
	LFT_Corrective,			///< Corrective.
	LFT_ClampedLinear,		///< Clamped linear.

	LFT_First = LFT_Null,
	LFT_Last  = LFT_ClampedLinear,
	LFT_NumLinkFnTypes = LFT_Last - LFT_First + 1
};

/// The function to modify a value as it flows through a FxFaceGraphNodeLink.
/// \ingroup faceGraph
class FxLinkFn : public FxNamedObject
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxLinkFn, FxNamedObject)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxLinkFn();
	/// Destructor.
	virtual ~FxLinkFn();

	/// Starts up the link function system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Startup( void );
	/// Shuts down the link function system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Shutdown( void );

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const = 0;

	/// Returns the total number of link functions.
	static FxSize FX_CALL GetNumLinkFunctions( void );
	/// Returns the link function at index.
	static const FxLinkFn* FX_CALL GetLinkFunction( FxSize index );
	/// Find a specified link function.
	/// \param name The name of the link function to find.
	/// \return A pointer to the link function, or \p NULL if it was not found.
	static const FxLinkFn* FX_CALL FindLinkFunction( const FxName& name );
	/// Find a specified link function by type.
	/// \param lfType The type of the link function to find.
	/// \return A pointer to the link function, or \p NULL if it was not found.
	static const FxLinkFn* FX_CALL FindLinkFunctionByType( FxLinkFnType lfType );
	/// Finds the specified link function's type enum.
	/// \param name The name of the link function to find the enum for.
	/// \return An FxLinkFnType value, or LFT_Invalid if it was not found.
	static FxLinkFnType FX_CALL FindLinkFunctionType( const FxName& name );
	/// Adds the specified link function.
	/// \param pLinkFn A pointer to the link function to add.
	static void FX_CALL AddLinkFunction( FxLinkFn* pLinkFn );

	/// Checks if the link function is the corrective link function.
	FX_INLINE const FxBool IsCorrective() const { return _isCorrective; }

protected:
	/// Whether or not the link function is the corrective link function.
	FxBool _isCorrective;

private:
	/// The list of all link functions in the system.
	static FxArray<FxLinkFn*>* _linkFnTable;
};

//------------------------------------------------------------------------------
// Null.
//------------------------------------------------------------------------------
/// The Null link function.  It is used to denote deprecated link functions and
/// always returns 0.0f in its Evaluate() function.
/// \ingroup faceGraph
class FxNullLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxNullLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxNullLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.  The Null link function must be supplied a name upon
	/// construction.  The name should be the name of the link function being
	/// deprecated.
	FxNullLinkFn( const FxChar* name );
	/// Destructor.
	virtual ~FxNullLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Null; }
};

//------------------------------------------------------------------------------
// Linear.
//------------------------------------------------------------------------------
/// The linear link function.
/// \ingroup faceGraph
class FxLinearLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxLinearLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxLinearLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxLinearLinkFn();
	/// Destructor.
	virtual ~FxLinearLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Linear; }
};

//------------------------------------------------------------------------------
// Quadratic.
//------------------------------------------------------------------------------
/// The quadratic link function.
/// \ingroup faceGraph
class FxQuadraticLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxQuadraticLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxQuadraticLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxQuadraticLinkFn();
	/// Destructor.
	virtual ~FxQuadraticLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Quadratic; }
};

//------------------------------------------------------------------------------
// Cubic.
//------------------------------------------------------------------------------
/// The cubic link function.
/// \ingroup faceGraph
class FxCubicLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxCubicLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxCubicLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxCubicLinkFn();
	/// Destructor.
	virtual ~FxCubicLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Cubic; }
};

//------------------------------------------------------------------------------
// Square Root.
//------------------------------------------------------------------------------
/// The square root link function.
/// \ingroup faceGraph
class FxSqrtLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxSqrtLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxSqrtLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxSqrtLinkFn();
	/// Destructor.
	virtual ~FxSqrtLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Sqrt; }
};

//------------------------------------------------------------------------------
// Negate.
//------------------------------------------------------------------------------
/// The negate link function.
/// \ingroup faceGraph
class FxNegateLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxNegateLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxNegateLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxNegateLinkFn();
	/// Destructor.
	virtual ~FxNegateLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Negate; }
};

//------------------------------------------------------------------------------
// Inverse.
//------------------------------------------------------------------------------
/// The inverse link function.
/// \ingroup faceGraph
class FxInverseLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxInverseLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxInverseLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxInverseLinkFn();
	/// Destructor.
	virtual ~FxInverseLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Inverse; }
};

//------------------------------------------------------------------------------
// One Clamp.
//------------------------------------------------------------------------------
/// The one clamp link function.
/// The one clamp link function is a special link function used in the implementation
/// of our old expressions algorithm.  It returns 1 if the value of the node is
/// less than or equal to one, or (1/node value) if the node value is greater than one.
/// \ingroup faceGraph
class FxOneClampLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxOneClampLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxOneClampLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxOneClampLinkFn();
	/// Destructor.
	virtual ~FxOneClampLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_OneClamp; }
};

//------------------------------------------------------------------------------
// Constant.
//------------------------------------------------------------------------------
/// The constant link function.
/// \ingroup faceGraph
class FxConstantLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxConstantLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxConstantLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxConstantLinkFn();
	/// Destructor.
	virtual ~FxConstantLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Constant; }
};

//------------------------------------------------------------------------------
// Corrective.
//------------------------------------------------------------------------------
/// The corrective link function.
/// \ingroup faceGraph
class FxCorrectiveLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxCorrectiveLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxCorrectiveLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxCorrectiveLinkFn();
	/// Destructor.
	virtual ~FxCorrectiveLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_Corrective; }
};

//------------------------------------------------------------------------------
// ClampedLinear
//------------------------------------------------------------------------------
/// The clamped linear link function.
/// \ingroup faceGraph
class FxClampedLinearLinkFn : public FxLinkFn
{
public:
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxClampedLinearLinkFn, FxLinkFn)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxClampedLinearLinkFn)
	/// Declare it as a link function.
	FX_DECLARE_LINKFN()
	/// Constructor.
	FxClampedLinearLinkFn();
	/// Destructor.
	virtual ~FxClampedLinearLinkFn();

	/// Returns the FxLinkFnType associated with the link function.
	virtual FxLinkFnType GetType( void ) const { return LFT_ClampedLinear; }
};

//------------------------------------------------------------------------------
// Add new link function types here.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// End licensee defined link function types.
//------------------------------------------------------------------------------

#define FX_SLOPE     params.parameters[0]
#define FX_CLAMP_X   params.parameters[1]
#define FX_CLAMP_Y   params.parameters[2]
#define FX_CLAMP_DIR params.parameters[3]
FX_INLINE FxReal FxLinkFnEval( FxLinkFnType lfType, FxReal x, const FxLinkFnParameters& params )
{
	switch( lfType )
	{
	case LFT_Linear:
		{
			if( 2 == params.parameters.Length() )
			{
				return (x * params.parameters[0] + params.parameters[1]);
			}
			else if( 1 == params.parameters.Length() )
			{
				return (x * params.parameters[0]);
			}
			else if( params.parameters.IsEmpty() )
			{
				return x;
			}
			return x;
		}
		break;
	case LFT_Corrective:
		{
			// This does nothing on purpose.  We need more information than we have at
			// this level to compute the corrective effect.
			if( params.parameters.Length() > 0 )
			{
				return params.parameters[0];
			}
			return 0.0f;
		}
		break;
	case LFT_Negate:
		{
			return -x;
		}
		break;
	case LFT_Inverse:
		{
			if( !FxRealEquality(x, 0.0f) )
			{
				return (1.0f / x);
			}
			return 0.0f;
		}
		break;
	case LFT_ClampedLinear:
		{
			if( 4 == params.parameters.Length() )
			{
				if( FX_CLAMP_DIR > 0.0f && x > FX_CLAMP_X ||
					FX_CLAMP_DIR <= 0.0f && x < FX_CLAMP_X )
				{
					return x * FX_SLOPE + (-1.0f * FX_SLOPE * FX_CLAMP_X + FX_CLAMP_Y);
				}
				return FX_CLAMP_Y;
			}
			else if( 1 == params.parameters.Length() )
			{
				if( x > 0.0f )
				{
					return x * FX_SLOPE;
				}
				return 0.0f;
			}
			return x;
		}
		break;
	case LFT_OneClamp:
		{
			return ((x <= 1.0f) ? 1.0f : (1.0f / x));
		}
		break;
	case LFT_Constant:
		{
			if( params.parameters.Length() > 0 )
			{
				return params.parameters[0];
			}
			return 1.0f;
		}
		break;
	case LFT_Cubic:
		{
			if( 1 == params.parameters.Length() )
			{
				return ((x * x * x) * params.parameters[0]);
			}
			return (x * x * x);
		}
		break;
	case LFT_Quadratic:
		{
			if( 1 == params.parameters.Length() )
			{
				return (((x < 0.0f) ? -1.0f : 1.0f) * (x * x) * params.parameters[0]);
			}
			return (((x < 0.0f) ? -1.0f : 1.0f) * (x * x));
		}
		break;
	case LFT_Sqrt:
		{
			if( 0.0f == x ) return 0.0f;
			if( 1 == params.parameters.Length() )
			{
				return (((x < 0.0f) ? -1.0f : 1.0f) * FxSqrt(FxAbs(x)) * params.parameters[0]);
			}
			return (((x < 0.0f) ? -1.0f : 1.0f) * FxSqrt(FxAbs(x)));
		}
		break;
	case LFT_Null:
		{
			return 0.0f;
		}
		break;
	case LFT_Invalid:
	default:
		{
			FxAssert(!"Unknown FxLinkFn type in FxLinkFnEval()!");
			return 0.0f;
		}
		break;
	}
}
#undef FX_SLOPE
#undef FX_CLAMP_X
#undef FX_CLAMP_Y
#undef FX_CLAMP_DIR

} // namespace Face

} // namespace OC3Ent

#endif
