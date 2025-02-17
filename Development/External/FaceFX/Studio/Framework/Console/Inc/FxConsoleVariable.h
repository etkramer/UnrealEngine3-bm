//------------------------------------------------------------------------------
// A FaceFX Studio console variable.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxInternalConsoleVariable_H__
#define FxInternalConsoleVariable_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"

namespace OC3Ent
{

namespace Face
{

#define FX_CONSOLE_VARIABLE_READONLY (1<<0)
#define FX_CONSOLE_VARIABLE_SAVECFG  (1<<1)

// A console variable.
class FxConsoleVariable : public FxNamedObject
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxConsoleVariable, FxNamedObject)
public:
	// Console variable callback function signature.
	typedef void (*FX_CONSOLE_VARIABLE_CALLBACK)( FxConsoleVariable& cvar );

	// Constructors.
	FxConsoleVariable();
	FxConsoleVariable( const FxName& cvarName, 
		               const FxString& cvarDefaultValue, 
		               FxInt32 cvarFlags = 0, 
					   FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK cvarCallback = NULL );
	// Destructor.
	virtual ~FxConsoleVariable();

	// Returns the on change callback function.
	FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK GetOnChangeCallback( void );
	// Sets the on change callback function.
	void SetOnChangeCallback( FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK changeCallback );

	// Returns the console variable flags.
	FxInt32 GetFlags( void ) const;
	// Sets the console variable flags.
	void SetFlags( FxInt32 flags );

	// Returns the default value of the console variable as a string.
	FxString GetDefaultValue( void ) const { return _defaultValue; }
	// Returns the string value of the console variable.
	FxString GetString( void ) const { return _stringValue;  }
	// Returns the floating point value of the console variable.
	FxReal GetFloat( void )    const { return _floatValue;   }
	// Returns the integer value of the console variable.
	FxInt32 GetInt( void )     const { return _integerValue; }

	// Sets the string value of the console variable.
	void SetString( const FxString& value ) { *this = value; }
	// Sets the floating point value of the console variable.
	void SetFloat( FxReal f )               { *this = f;     }
	// Sets the integer value of the console variable.
	void SetInt( FxInt32 i )                { *this = i;     }

	// Integer assignment operator.
	FxConsoleVariable& operator=( const FxInt32 i );
	// Floating point assignment operator.
	FxConsoleVariable& operator=( const FxReal f );
	// String assignment operator.
	FxConsoleVariable& operator=( const FxString& s );

	// Integer cast operator.
	operator FxInt32()         const { return _integerValue; }
	// Floating point cast operator.
	operator FxReal()          const { return _floatValue;   }
	// String cast operator.
	operator const FxString&() const { return _stringValue;  }
#ifdef FX_USE_BUILT_IN_BOOL_TYPE
	// Boolean cast operator.
	operator FxBool()          const { return _integerValue ? FxTrue : FxFalse; }
#endif

protected:
	// The string value of the console variable.
	FxString _stringValue;
	// The default value of the console variable as a string.
	FxString _defaultValue;
	// The integer value of the console variable.
	FxInt32	 _integerValue;
	// The floating point value of the console variable.
	FxReal   _floatValue;
	// The console variable flags.
	FxInt32	 _flags;

	// The console variable on change callback function.
	FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK _onChangeCallback;
};

} // namespace Face

} // namespace OC3Ent

#endif
