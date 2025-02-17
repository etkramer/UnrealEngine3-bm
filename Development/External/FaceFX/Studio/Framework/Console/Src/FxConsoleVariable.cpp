//------------------------------------------------------------------------------
// A FaceFX Studio console variable.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxConsoleVariable.h"

#include <cstdlib>

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxConsoleVariableVersion 0

FX_IMPLEMENT_CLASS(FxConsoleVariable, kCurrentFxConsoleVariableVersion, FxNamedObject)

FxConsoleVariable::FxConsoleVariable()
{
	_flags = 0;
	_onChangeCallback = NULL;
}

FxConsoleVariable::FxConsoleVariable( const FxName& cvarName, 
								      const FxString& cvarDefaultValue, 
									  FxInt32 cvarFlags, 
									  FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK cvarCallback )
{
	SetName(cvarName);
	SetString(cvarDefaultValue);
	_flags = cvarFlags;
	_defaultValue = cvarDefaultValue;
	_onChangeCallback = cvarCallback;
}

FxConsoleVariable::~FxConsoleVariable()
{
}

FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK 
FxConsoleVariable::GetOnChangeCallback( void )
{
	return _onChangeCallback;
}

void FxConsoleVariable::
SetOnChangeCallback( FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK changeCallback )
{
	_onChangeCallback = changeCallback;
}

FxInt32 FxConsoleVariable::GetFlags( void ) const
{
	return _flags;
}

void FxConsoleVariable::SetFlags( FxInt32 flags )
{
	_flags = flags;
}

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4714)
#endif // _MSC_VER

FxConsoleVariable& FxConsoleVariable::operator=( const FxInt32 i )
{
	_integerValue = i;
	_floatValue = static_cast<FxReal>(i);
	FxChar str[32] = {0};
	FxItoa(i, str, 32);
	_stringValue = str;
	return *this;
}

#ifdef _MSC_VER
	#pragma warning(pop)
#endif // _MSC_VER

FxConsoleVariable& FxConsoleVariable::operator=( const FxReal f )
{
	_integerValue = static_cast<FxInt32>(f);
	_floatValue = f;
	FxChar str[32] = {0};
	sprintf(str, "%f", f);
	_stringValue = str;
	return *this;
}

FxConsoleVariable& FxConsoleVariable::operator=( const FxString& s )
{
	_integerValue = FxAtoi(s.GetData());
	_floatValue = static_cast<FxReal>(FxAtof(s.GetData()));
	_stringValue = s;
	return *this;
}

} // namespace Face

} // namespace OC3Ent
