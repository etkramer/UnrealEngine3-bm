//------------------------------------------------------------------------------
// Constructs that are shared between the easy-to-edit and "compiled" 
// representations of a Face Graph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxFaceGraphShared.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxFaceGraphNodeUserPropertyVersion 0

FX_IMPLEMENT_CLASS(FxFaceGraphNodeUserProperty, kCurrentFxFaceGraphNodeUserPropertyVersion, FxNamedObject)

FxFaceGraphNodeUserProperty::
FxFaceGraphNodeUserProperty( FxFaceGraphNodeUserPropertyType propertyType )
	: _propertyType(propertyType)
	, _integerProperty(0)
	, _realProperty(0.0f)
{
	if( UPT_String == _propertyType )
	{
		_choices.Reserve(1);
		_choices.PushBack(FxString());
	}
}

FxFaceGraphNodeUserProperty::
FxFaceGraphNodeUserProperty( const FxFaceGraphNodeUserProperty& other )
	: Super(other)
	, _propertyType(other._propertyType)
	, _integerProperty(other._integerProperty)
	, _realProperty(other._realProperty)
	, _choices(other._choices)
{
}

FxFaceGraphNodeUserProperty& 
FxFaceGraphNodeUserProperty::operator=( const FxFaceGraphNodeUserProperty& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	_propertyType	 = other._propertyType;
	_integerProperty = other._integerProperty;
	_realProperty	 = other._realProperty;
	_choices		 = other._choices;
	return *this;
}

FxFaceGraphNodeUserProperty::~FxFaceGraphNodeUserProperty()
{
}

void FxFaceGraphNodeUserProperty::SetIntegerProperty( FxInt32 integerProperty )
{
	if( UPT_Integer == _propertyType )
	{
		_integerProperty = integerProperty;
	}
}

void FxFaceGraphNodeUserProperty::SetBoolProperty( FxBool boolProperty )
{
	if( FxFalse == boolProperty )
	{
		_integerProperty = 0;
	}
	else
	{
		_integerProperty = 1;
	}
}

void FxFaceGraphNodeUserProperty::SetRealProperty( FxReal realProperty )
{
	if( UPT_Real == _propertyType )
	{
		_realProperty = realProperty;
	}
}

void FxFaceGraphNodeUserProperty::SetStringProperty( const FxString& stringProperty )
{
	if( UPT_String == _propertyType )
	{
		_choices[0] = stringProperty;
	}
}

void FxFaceGraphNodeUserProperty::SetChoiceProperty( const FxString& choiceProperty )
{
	if( UPT_Choice == _propertyType )
	{
		_integerProperty = 0;
		FxSize numChoices = _choices.Length();
		for( FxSize i = 0; i < numChoices; ++i )
		{
			if( _choices[i] == choiceProperty )
			{
				_integerProperty = static_cast<FxInt32>(i);
			}
		}
	}
}

void FxFaceGraphNodeUserProperty::AddChoice( const FxString& choice )
{
	_choices.PushBack(choice);
}

void FxFaceGraphNodeUserProperty::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxFaceGraphNodeUserProperty");

	FxInt32 tempPropertyType = static_cast<FxInt32>(_propertyType);
	arc << tempPropertyType << _integerProperty << _realProperty << _choices;
	_propertyType = static_cast<FxFaceGraphNodeUserPropertyType>(tempPropertyType);
}

} // namespace Face

} // namespace OC3Ent
