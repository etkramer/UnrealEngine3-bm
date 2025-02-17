//------------------------------------------------------------------------------
// Constructs that are shared between the easy-to-edit and "compiled" 
// representations of a Face Graph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphShared_H__
#define FxFaceGraphShared_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxNamedObject.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

/// The available %Face Graph node user property types.
/// \ingroup faceGraph
enum FxFaceGraphNodeUserPropertyType
{
	UPT_Integer = 0, ///< An integer property.
	UPT_Bool    = 1, ///< A boolean property.
	UPT_Real    = 2, ///< A floating-point property.
	UPT_String  = 3, ///< A string property.
	UPT_Choice  = 4  ///< A choice property.
};

/// A named %Face Graph node property.
/// \ingroup faceGraph
class FxFaceGraphNodeUserProperty : public FxNamedObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxFaceGraphNodeUserProperty, FxNamedObject)
public:
	/// Constructor.
	FxFaceGraphNodeUserProperty( FxFaceGraphNodeUserPropertyType propertyType = UPT_Integer );
	/// Copy constructor.
	FxFaceGraphNodeUserProperty( const FxFaceGraphNodeUserProperty& other );
	/// Assignment.
	FxFaceGraphNodeUserProperty& operator=( const FxFaceGraphNodeUserProperty& other );
	/// Destructor.
	virtual ~FxFaceGraphNodeUserProperty();

	/// Returns the user property type.
	FX_INLINE FxFaceGraphNodeUserPropertyType GetPropertyType( void ) const { return _propertyType; }

	/// Returns the integer property.  The return value is only valid if the 
	/// type of user property is Face::UPT_Integer.
	FX_INLINE FxInt32 GetIntegerProperty( void ) const { return _integerProperty; }
	/// Sets the integer property if the type of user property is Face::UPT_Integer.
	void SetIntegerProperty( FxInt32 integerProperty );

	/// Returns the bool property.  The return value is only valid if the
	/// type of user property is Face::UPT_Bool.
	FX_INLINE FxBool GetBoolProperty( void ) const { return (0 == _integerProperty) ? FxFalse : FxTrue; }
	/// Sets the bool property if the type of user property is Face::UPT_Bool.
	void SetBoolProperty( FxBool boolProperty );

	/// Returns the real property.  The return value is only valid if the type 
	/// of user property is Face::UPT_Real.
	FX_INLINE FxReal GetRealProperty( void ) const { return _realProperty; }
	/// Sets the real property if the type of user property is Face::UPT_Real.
	void SetRealProperty( FxReal realProperty );

	/// Returns the string property.  The return value is only valid if the 
	/// type of user property is Face::UPT_String.
	FX_INLINE const FxString& GetStringProperty( void ) const { return _choices[0]; }
	/// Sets the string property if the type of user property is Face::UPT_String.
	void SetStringProperty( const FxString& stringProperty );

	/// Returns the choice property (current choice).  The return value is only
	/// valid if the type of user property is Face::UPT_Choice.
	FX_INLINE const FxString& GetChoiceProperty( void ) const { return _choices[_integerProperty]; }
	/// Sets the choice property (current choice) if the type of user property
	/// is Face::UPT_Choice.
	void SetChoiceProperty( const FxString& choiceProperty );

	/// Returns the number of choices if the type of user property is 
	/// Face::UPT_Choice.
	FX_INLINE FxSize GetNumChoices( void ) const { return _choices.Length(); }
	/// Returns the choice at index if the type of user property is Face::UPT_Choice.
	FX_INLINE const FxString& GetChoice( FxSize index ) const { return _choices[index]; }
	/// Adds a choice if the type of user property is Face::UPT_Choice.
	void AddChoice( const FxString& choice );

	/// Serializes an FxFaceGraphNodeUserProperty to an archive.
	virtual void Serialize( FxArchive& arc );

protected:
	/// The type of user property.
	FxFaceGraphNodeUserPropertyType _propertyType;
	/// The integer property.  If the type is Face::UPT_Choice, this is used as an
	/// index into the \a _choices array.  If the type is Face::UPT_Bool, this is used
	/// as a boolean value where zero is false and non-zero is true.
	FxInt32 _integerProperty;
	/// The real property.
	FxReal _realProperty;
	/// The choices.  If the type is Face::UPT_String, the first element of \a _choices
	/// is used as the string property.
	FxArray<FxString> _choices;
};

/// The available operations for use with the node inputs.
/// \ingroup faceGraph
enum FxInputOp
{
	IO_Invalid = -1, ///< Invalid node input operation.
	IO_Sum,          ///< Sum the inputs.
	IO_Multiply,     ///< Multiply the inputs together.
	IO_Max,          ///< Take the maximum input value.
	IO_Min,          ///< Take the minimum input value.
};

/// The available operations for use with the SetUserValue() call.
/// \ingroup faceGraph
enum FxValueOp
{
	VO_Invalid = -1, ///< Invalid user operation.
	VO_Add,			 ///< Add the user value into the node value.
	VO_Multiply,	 ///< Multiply the user value with the node value.
	VO_Replace,		 ///< Replace the node value with the user value.
	VO_None		     ///< Do nothing.
};

/// The available operations for use with the SetRegister() calls.
enum FxRegisterOp
{
	RO_Invalid = -1, ///< Invalid register operation.
	RO_Add,			 ///< Add the register value into the node value.
	RO_Multiply,	 ///< Multiply the register value with the node value.
	RO_Replace,		 ///< Replace the node value with the register value.
	RO_LoadAdd,		 ///< Load the node value into the register as the 
					 ///< interpolation starting value and then add the register
					 ///< value into the node value.
	RO_LoadMultiply, ///< Load the node value into the register as the 
					 ///< interpolation starting value and then multiply the 
					 ///< register value into the node value.
	RO_LoadReplace,  ///< Load the node value into the register as the 
					 ///< interpolation starting value and then replace the node
					 ///< value with the register value.
	RO_None			 ///< Do nothing.
};

/// A register is for interacting with an actor instance's %Face Graph.
/// A register is used for setting and retrieving results from 
/// FxFaceGraphNode objects contained in the FxActor this instance
/// refers to.
struct FxRegister
{
	/// Constructor.
	FxRegister()
		: value(0.0f)
		, interpStartValue(0.0f)
		, interpEndValue(0.0f)
		, interpNextEndValue(0.0f)
		, interpLastValue(0.0f)
		, interpInverseDuration(0.0f)
		, interpNextInverseDuration(0.0f)
		, interpStartTime(FxInvalidValue)
		, scratch(0.0f)
		, firstRegOp(VO_None) 
		, nextRegOp(RO_None)
		, isInterpolating(FxFalse) 
	{}
	/// Copy constructor.
	FxRegister( const FxRegister& other )
		: value(other.value)
		, interpStartValue(other.interpStartValue)
		, interpEndValue(other.interpEndValue)
		, interpNextEndValue(other.interpNextEndValue)
		, interpLastValue(other.interpLastValue)
		, interpInverseDuration(other.interpInverseDuration)
		, interpNextInverseDuration(other.interpNextInverseDuration)
		, interpStartTime(other.interpStartTime)
		, scratch(other.scratch)
		, firstRegOp(other.firstRegOp) 
		, nextRegOp(other.nextRegOp)
		, isInterpolating(other.isInterpolating) 
	{}
	/// Assignment operator.
	FxRegister& operator=( const FxRegister& other )
	{
		if( this == &other ) return *this;
		value = other.value;
		interpStartValue = other.interpStartValue;
		interpEndValue = other.interpEndValue;
		interpNextEndValue = other.interpNextEndValue;
		interpLastValue = other.interpLastValue;
		interpInverseDuration = other.interpInverseDuration;
		interpNextInverseDuration = other.interpNextInverseDuration;
		interpStartTime = other.interpStartTime;
		scratch = other.scratch;
		firstRegOp = other.firstRegOp; 
		nextRegOp = other.nextRegOp;
		isInterpolating = other.isInterpolating;
		return *this;
	}
	/// Returns the FxUserOp associated with the given FxRegisterOp.
	static FxValueOp FX_CALL GetUserOpForRegOp( FxRegisterOp regOp )
	{
		FxValueOp valueOp = VO_None;
		switch( regOp )
		{
		case RO_Add:
		case RO_LoadAdd:
			valueOp = VO_Add;
			break;
		case RO_Multiply:
		case RO_LoadMultiply:
			valueOp = VO_Multiply;
			break;
		case RO_Replace:
		case RO_LoadReplace:
			valueOp = VO_Replace;
			break;
		case RO_None:
			valueOp = VO_None;
			break;
		case RO_Invalid:
		default:
			FxAssert(!"Invalid register operation!");
			break;
		}
		return valueOp;
	}
	/// The value of the register.
	FxReal value;
	/// The start value of the interpolation.
	FxReal interpStartValue;
	/// The end value of the interpolation.
	FxReal interpEndValue;
	/// The end value of the next interpolation.
	FxReal interpNextEndValue;
	/// The last known interpolation value.
	FxReal interpLastValue;
	/// The inverse duration of the interpolation.
	FxReal interpInverseDuration;
	/// The inverse duration of the next interpolation.
	FxReal interpNextInverseDuration;
	/// The start time of the interpolation.
	FxReal interpStartTime;
	/// Scratch space in the register for internal calculations.  For example, 
	/// this is used by delta nodes to keep track of the node's first input
	/// link value from the previous frame.
	FxReal scratch;
	/// The first register operation to perform when setting the value of the
	/// node.  Note that this is not an FxRegisterOp but rather an FxValueOp
	/// because it is what is used to set the user value of the node.
	FxValueOp firstRegOp;
	/// The next register operation to perform when setting the value of the
	/// node.  Note that this is not a FxRegisterOp so that the proper 
	/// FxValueOp can be promoted to firstRegOp.
	FxRegisterOp nextRegOp;
	/// FxTrue if the register is interpolating.
	FxBool isInterpolating;
};

} // namespace Face

} // namespace OC3Ent

#endif
