//------------------------------------------------------------------------------
// The FaceFx virtual machine.  This module is responsible for executing
// commands and at some future date interpreting a scripting language.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxVM_H__
#define FxVM_H__

#include "FxCommand.h"
#include "FxConsoleVariable.h"
#include "FxArray.h"
#include "FxList.h"
#include "FxBatchCommand.h"

namespace OC3Ent
{

namespace Face
{

// The FaceFx virtual machine.
class FxVM
{
public:
	// Registers a new command with the virtual machine.  Returns FxTrue if the
	// command was registered or FxFalse if it was not.
	static FxBool RegisterCommand( const FxString& commandName, 
								   const FxCommandSyntax& commandSyntax,
								   FxObject* (FX_CALL *createCommandFn)() );
	// Unregisters a command with the virtual machine.  Returns FxTrue if the
	// command was unregistered or FxFalse if it was not.
	static FxBool UnregisterCommand( const FxString& commandName );

	// Registers a new console variable with the virtual machine.  Returns 
	// FxTrue if the console variable was registered or FxFalse if it was not.
	static FxBool RegisterConsoleVariable( const FxString& cvarName,
										   const FxString& cvarDefaultValue = "",
										   FxInt32 cvarFlags = 0,
										   FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK cvarCallback = NULL );
	// Unregisters a console variable with the virtual machine.  Returns FxTrue
	// if the console variable was unregistered or FxFalse if it was not.
	static FxBool UnregisterConsoleVariable( const FxString& cvarName );

	// Finds the specified console variable and returns a pointer to it or NULL
	// if the console variable could not be found.
	static FxConsoleVariable* FindConsoleVariable( const FxString& cvarName );

	// Executes the specified command string.
	static FxCommandError ExecuteCommand( const FxString& command );
	// Returns FxTrue if the last executed command was pushed to the undo stack.
	static FxBool LastCommandWasUndoable( void );

	// Internal usage only.
	// A command description.
	class FxCommandDesc
	{
	public:
		// The command name.
		FxString CommandName;
		// The command syntax.
		FxCommandSyntax CommandSyntax;
		// A pointer to a function that can construct a command object of the 
		// correct type.
		FxObject* (FX_CALL *CreateCommand)();
	};
	static void Startup( void );
	static void Shutdown( void );
	static void SetOutputCtrl( void* pOutputCtrl );
	static void DisplayEcho( const FxString& msg );
	static void DisplayEchoW( const FxWString& msg );
	static void DisplayInfo( const FxString& msg, FxBool echoToPanel = FxFalse );
	static void DisplayInfoW( const FxWString& msg, FxBool echoToPanel = FxFalse );
	static void DisplayWarning( const FxString& msg );
	static void DisplayWarningW( const FxWString& msg );
	static void DisplayError( const FxString& msg );
	static void DisplayErrorW( const FxWString& msg );
	static FxBool BeginBatch( void );
	static FxBool BatchCommand( const FxString& command );
	static FxBool ExecBatch( const FxArray<FxString>& args );
	static void SetEchoCommands( FxBool echoCommands );
	static FxSize GetNumCommands( void );
	static FxSize GetNumConsoleVariables( void );
	static FxCommandDesc* GetCommandDesc( FxSize index );
	static FxConsoleVariable* GetConsoleVariable( FxSize index );

protected:
	// The commands registered with the virtual machine.
	static FxArray<FxCommandDesc> _registeredCommands;

	// The console variables registered with the virtual machine.
	static FxList<FxConsoleVariable> _registeredConsoleVariables;

	// FxTrue if the last executed command was pushed to the undo stack.
	static FxBool _lastCommandWasUndoable;

	// An internal batch command object.
	static FxBatchCommand* _pBatchCommand;
	// FxTrue if the previous batch command was executed.
	static FxBool _executedBatch;
	// FxTrue if no commands should be echoed.
	static FxBool _echoCommands;

	// Finds the specified command and returns a pointer to it or NULL if the
	// command could not be found.
	static FxCommandDesc* _findCommand( const FxString& commandName );

	// Finds the specified console variable and returns a pointer to it or NULL
	// if the console variable could not be found.
	static FxConsoleVariable* _findConsoleVariable( const FxString& cvarName );

	// Extracts the command name and arguments from the command string.  No
	// syntax checking is done in this function - it only splits the command
	// string into it's parts.  The command name is stored in the first slot
	// of the returned string array.
	static FxArray<FxString> _extractCommand( const FxString& command );
	// Builds the argument list by checking the arguments against the command
	// syntax.  Returns FxFalse if the command syntax is incorrect.
	static FxBool _createArgumentList( const FxArray<FxString>& splitCommand, 
		const FxCommandDesc* pCommandDesc, FxCommandArgumentList& argList );

	// Output command results.
	static void _outputResult( FxCommandError commandError, const FxString& commandName, 
		const FxString& extraInfo, FxCommand* pCommand );
};

} // namespace Face

} // namespace OC3Ent

#endif

