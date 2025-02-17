//------------------------------------------------------------------------------
// The base class for a FaceFX Studio command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCommand_H__
#define FxCommand_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxArray.h"
#include "FxObject.h"

namespace OC3Ent
{

namespace Face
{

// An error code.
enum FxCommandError
{
	CE_Failure,
	CE_Success,
	CE_ArgumentFailure,
	CE_InvalidCommand,
	CE_InternalError
};

// A command argument type.
enum FxCommandArgumentType
{
	CAT_Flag,
	CAT_Real,
	CAT_DReal,
	CAT_Size,
	CAT_Int32,
	CAT_String,
	CAT_StringArray,
	CAT_Bool
};

// A command argument description.
class FxCommandArgumentDesc
{
public:
	// Constructors.
	FxCommandArgumentDesc();
	FxCommandArgumentDesc( const FxString& argNameShort,
		                   const FxString& argNameLong,
						   FxCommandArgumentType argType );

	// The short argument name.
	FxString ArgNameShort;
	// The long argument name.
	FxString ArgNameLong;
	// The argument type.
	FxCommandArgumentType ArgType;
};

// A command argument.
class FxCommandArgument
{
public:
	// The argument description.
	FxCommandArgumentDesc ArgDesc;
	// The argument.
	FxString Arg;
};

// An argument list.
class FxCommandArgumentList
{
public:
	// Constructor.
	FxCommandArgumentList();
	// Destructor.
	~FxCommandArgumentList();

	// Adds an argument to the list.
	void AddArgument( const FxCommandArgument& arg );
	// Returns the number of arguments in the list.
	FxSize GetNumArguments( void ) const;
	// Returns the argument at index.
	const FxCommandArgument& GetArgument( FxSize index ) const;

	// Returns the argument named argName into the arg variable.  If the syntax
	// of the command does not match the type of variable passed to this 
	// function, an error is raised and the function returns FxFalse.  The
	// first variant simply checks that an argument exists, which is useful
	// for arguments of type CAT_Flag.
	FxBool GetArgument( const FxString& argName ) const;
	FxBool GetArgument( const FxString& argName, FxReal&   arg ) const;
	FxBool GetArgument( const FxString& argName, FxDReal&  arg ) const;
	FxBool GetArgument( const FxString& argName, FxSize&   arg ) const;
	FxBool GetArgument( const FxString& argName, FxInt32&  arg ) const;
	FxBool GetArgument( const FxString& argName, FxString& arg ) const;
	FxBool GetArgument( const FxString& argName, FxArray<FxString>& arg ) const;
	FxBool GetArgument( const FxString& argName, FxBool&   arg ) const;

protected:
	// The argument list.
	FxArray<FxCommandArgument> _args;

	// Returns the index of the specified argument.
	FxSize _getArgIndex( const FxString& argName ) const;
};

// A command syntax.
class FxCommandSyntax
{
public:
	// Constructor.
	FxCommandSyntax();
	// Destructor.
	~FxCommandSyntax();

	// Add an argument description to the syntax.
	void AddArgDesc( const FxCommandArgumentDesc& argDesc );
	// Returns the number of argument descriptions in the syntax.
	FxSize GetNumArgDescs( void ) const;
	// Returns the argument description at index.
	const FxCommandArgumentDesc& GetArgDesc( FxSize index ) const;

protected:
	// The argument description list.
	FxArray<FxCommandArgumentDesc> _syntax;
};

//@todo Support command return values and querying?

// A command.  FxCommand derives from FxObject and internally FaceFx will call
// the ConstructObject() function provided by FxObject on each command to be
// executed.  Whether the instance returned by ConstructObject() stays around
// or not is determined by the IsUndoable() function of FxCommand.
class FxCommand : public FxObject
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxCommand, FxObject);
public:
	// Constructor.
	FxCommand();
	// Destructor.
	virtual ~FxCommand();

	// Sets up the argument syntax for the command.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.  The proper way to execute a command is to store any
	// data needed to execute or undo the command in the instance inside of 
	// Execute() and then invoke Redo() to actually execute the command.  If the
	// command is not undoable (by setting _isUndoable to FxTrue in the 
	// command's constructor, you can simply execute the command inside of the
	// Execute() function.
	// This must be overridden in any derived class.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList ) = 0;

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes (and if the command is coded properly initially executes - see 
	// Execute()) the command.
	virtual FxCommandError Redo( void );

	// Returns FxTrue if the command can be undone.  If a command can be undone,
	// the instance is stored inside the Undo / Redo manager, otherwise the
	// instance is destroyed after being executed.
	virtual FxBool IsUndoable( void ) const;
	// Returns FxTrue if the command should cause the Undo / Redo manager to be 
	// flushed.  Certain operations like loading a file, saving a file, closing
	// a file, and creating a new file should logically cause the Undo / Redo
	// system to be flushed.
	virtual FxBool ShouldFlushUndo( void ) const;

	// Returns FxTrue if the command should be echoed.
	virtual FxBool ShouldEcho( void ) const;

	// Displays an informational message to the console.
	void DisplayInfo( const FxString& msg );
	// Displays a warning message to the console and command panel.
	void DisplayWarning( const FxString& msg );
	// Displays an error message to the console and the command panel.
	void DisplayError( const FxString& msg );

	// Returns the full command string.
	const FxString& GetCommandString( void ) const;

	// The virtual machine can access any internal command parameter.
	friend class FxVM;
	
protected:
	// The undoable flag.
	FxBool _isUndoable;
	// The undo flush flag.
	FxBool _shouldFlushUndo;
	// The echo flag.
	FxBool _shouldEcho;
	// The command string.
	FxString _commandString;
};

} // namespace Face

} // namespace OC3Ent

#endif
