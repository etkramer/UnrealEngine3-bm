//------------------------------------------------------------------------------
// The FaceFx virtual machine.  This module is responsible for executing
// commands and at some future date interpreting a scripting language.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxVM.h"
#include "FxUndoRedoManager.h"
#include "FxStudioSession.h"
#include "FxSessionProxy.h"
#include "FxInternalCommands.h"
#include "FxInternalConsoleVariables.h"
#include "FxConsole.h"
#include "FxSecurityWrapper.h"

// Error code colors.
wxColour kNormalColor(192,192,192);
wxColour kWarningColor(255,255,128);
wxColour kErrorColor(255,128,128);

namespace OC3Ent
{

namespace Face
{

FxArray<FxVM::FxCommandDesc> FxVM::_registeredCommands;
FxList<FxConsoleVariable> FxVM::_registeredConsoleVariables;
FxBool FxVM::_lastCommandWasUndoable = FxFalse;
FxBatchCommand* FxVM::_pBatchCommand = NULL;
FxBool FxVM::_executedBatch = FxFalse;
FxBool FxVM::_echoCommands = FxTrue;
static wxTextCtrl* _pOutputCtrl  = NULL;

FxBool FxVM::RegisterCommand( const FxString& commandName, 
							  const FxCommandSyntax& commandSyntax,
							  FxObject* (FX_CALL *createCommandFn)() )
{
	if( !_findCommand(commandName) )
	{
		FxCommandDesc commandDesc;
		commandDesc.CommandName   = commandName;
		commandDesc.CommandSyntax = commandSyntax;
		commandDesc.CreateCommand = createCommandFn;
		_registeredCommands.PushBack(commandDesc);
	}
	return FxFalse;
}

FxBool FxVM::UnregisterCommand( const FxString& commandName )
{
	for( FxSize i = 0; i < _registeredCommands.Length(); ++i )
	{
		if( _registeredCommands[i].CommandName == commandName )
		{
			_registeredCommands.Remove(i);
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxVM::RegisterConsoleVariable( const FxString& cvarName,
									  const FxString& cvarDefaultValue,
									  FxInt32 cvarFlags,
									  FxConsoleVariable::FX_CONSOLE_VARIABLE_CALLBACK cvarCallback )
{
	if( !_findConsoleVariable(cvarName) )
	{
		FxConsoleVariable cvar(cvarName.GetData(), cvarDefaultValue, cvarFlags, cvarCallback);
		_registeredConsoleVariables.PushBack(cvar);
	}
	return FxFalse;
}

FxBool FxVM::UnregisterConsoleVariable( const FxString& cvarName )
{
	FxList<FxConsoleVariable>::Iterator removePos = _registeredConsoleVariables.Begin();
	for( ; removePos != _registeredConsoleVariables.End(); ++removePos )
	{
		if( (*removePos).GetNameAsString() == cvarName )
		{
			_registeredConsoleVariables.Remove(removePos);
			return FxTrue;
		}
	}
	return FxFalse;
}

FxConsoleVariable* FxVM::FindConsoleVariable( const FxString& cvarName )
{
	return _findConsoleVariable(cvarName);
}

FxCommandError FxVM::ExecuteCommand( const FxString& command )
{
	// Extract the command.
#ifdef FX_ENABLE_SECURITY
	FxString cmd(command);
	FxCheckLargeNumber(cmd);
	FxArray<FxString> commandParts = _extractCommand(cmd);
#else
	FxArray<FxString> commandParts = _extractCommand(command);
#endif
	if( 0 == commandParts.Length() )
	{
		return CE_Success;
	}
	// Find the command.
	FxCommandDesc* pCommandDesc = _findCommand(commandParts[0]);
	if( !pCommandDesc )
	{
		_outputResult(CE_InvalidCommand, commandParts[0], commandParts[0], NULL);
		return CE_InvalidCommand;
	}
	else
	{
		// Construct a command of the appropriate type.
		FxObject*  pCommandObject = pCommandDesc->CreateCommand();
		FxCommand* pCommand = FxCast<FxCommand>(pCommandObject);
		if( !pCommand )
		{
			delete pCommandObject;
			pCommandObject= NULL;
			_outputResult(CE_InternalError, commandParts[0], "Could not create command!", NULL);
			return CE_InternalError;
		}
		else
		{
			FxString localCommandString = command;
			if( localCommandString[localCommandString.Length()-1] != ';' )
			{
				localCommandString = FxString::Concat(localCommandString, ';');
			}
			pCommand->_commandString = localCommandString;
			if( _pBatchCommand && !_executedBatch )
			{
				if( commandParts[0] == "batch" )
				{
					// Force an execBatch to happen to flush the system.  The
					// sequence of arguments to execBatch here ensure that any
					// sequence of commands in the batch will be flushed
					// correctly.
					ExecuteCommand("execBatch -editednodes -editedcurves -changedmapping");
					DisplayError("illegal call to batch!");
					delete pCommand;
					pCommand = NULL;
					return CE_Failure;
				}
				else if( commandParts[0] != "execBatch" )
				{
					if( !BatchCommand(localCommandString) )
					{
						DisplayError("could not add to batch!");
						delete pCommand;
						pCommand = NULL;
						return CE_Failure;
					}
					delete pCommand;
					pCommand = NULL;
                    return CE_Success;
				}
			}
			// Trap nasty illegal calls to batch through nested exec commands.
			else if( _pBatchCommand && _executedBatch )
			{
				if( commandParts[0] == "batch" )
				{
					DisplayError("illegal call to batch!");
					delete pCommand;
					pCommand = NULL;
					return CE_Failure;
				}
			}
			// Create the command argument list from the command syntax.
			FxCommandArgumentList argList;
			FxBool argListValid = _createArgumentList(commandParts, pCommandDesc, argList);
			if( !argListValid )
			{
				_outputResult(CE_ArgumentFailure, commandParts[0], localCommandString, pCommand);
				delete pCommand;
				pCommand = NULL;
				return CE_ArgumentFailure;
			}
			// Execute the command.
			FxCommandError commandReturnCode = pCommand->Execute(argList);

			// If the command was successful and it is undoable, push it to the 
			// undo system.  Otherwise delete it.
			if( CE_Success == commandReturnCode && pCommand->IsUndoable() )
			{
				FxUndoRedoManager::PushUndoableCommand(pCommand);
				_lastCommandWasUndoable = FxTrue;
				_outputResult(commandReturnCode, commandParts[0], localCommandString, pCommand);
			}
			else if( CE_Success == commandReturnCode && pCommand->ShouldFlushUndo() )
			{
				FxUndoRedoManager::Flush();
				_outputResult(commandReturnCode, commandParts[0], localCommandString, pCommand);
				delete pCommand;
				pCommand = NULL;
				_lastCommandWasUndoable = FxFalse;
			}
			else
			{
				// Call _outputResult() before the command is deleted.
				_outputResult(commandReturnCode, commandParts[0], localCommandString, pCommand);
				delete pCommand;
				pCommand = NULL;
				_lastCommandWasUndoable = FxFalse;
			}
			// Refresh undo / redo menu.
			FxSessionProxy::RefreshUndoRedoMenu();
			// Return the command return code.
			//@todo Actually do something with the return code here.
			return commandReturnCode;
		}
	}
}

FxBool FxVM::LastCommandWasUndoable( void )
{
	return _lastCommandWasUndoable;
}

void FxVM::Startup( void )
{
	// Register the internal commands.
	FxInternalCommands::RegisterAll();
	// Register the internal console variables.
	FxInternalConsoleVariables::RegisterAll();
}

void FxVM::Shutdown( void )
{
	FxUndoRedoManager::Flush();
	// Unregister the internal commands.
	FxInternalCommands::UnregisterAll();
	// Unregister the internal console variables.
	FxInternalConsoleVariables::UnregisterAll();
	// Clear the remaining user defined console variables.
	_registeredConsoleVariables.Clear();
}

void FxVM::SetOutputCtrl( void* pOutputCtrl )
{
	_pOutputCtrl = reinterpret_cast<wxTextCtrl*>(pOutputCtrl);
}

void FxVM::DisplayEcho( const FxString& msg )
{
	if( _echoCommands )
	{
		FxEcho("%s", msg.GetData());
		if( _pOutputCtrl )
		{
			_pOutputCtrl->SetBackgroundColour(kNormalColor);
			_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
		}
	}
}

void FxVM::DisplayEchoW( const FxWString& msg )
{
	if( _echoCommands )
	{
		FxEchoW(L"%s", msg.GetData());
		if( _pOutputCtrl )
		{
			_pOutputCtrl->SetBackgroundColour(kNormalColor);
			_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
		}
	}
}

void FxVM::DisplayInfo( const FxString& msg, FxBool echoToPanel )
{
	FxMsg("%s", msg.GetData());
	if( echoToPanel )
	{
		if( _pOutputCtrl )
		{
			_pOutputCtrl->SetBackgroundColour(kNormalColor);
			_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
		}
	}
}

void FxVM::DisplayInfoW( const FxWString& msg, FxBool echoToPanel )
{
	FxMsgW(L"%s", msg.GetData());
	if( echoToPanel )
	{
		if( _pOutputCtrl )
		{
			_pOutputCtrl->SetBackgroundColour(kNormalColor);
			_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
		}
	}
}

void FxVM::DisplayWarning( const FxString& msg )
{
	FxWarn("%s", msg.GetData());
	if( _pOutputCtrl )
	{
		_pOutputCtrl->SetBackgroundColour(kWarningColor);
		_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
	}
}

void FxVM::DisplayWarningW( const FxWString& msg )
{
	FxWarnW(L"%s", msg.GetData());
	if( _pOutputCtrl )
	{
		_pOutputCtrl->SetBackgroundColour(kWarningColor);
		_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
	}
}

void FxVM::DisplayError( const FxString& msg )
{
	FxError("%s", msg.GetData());
	if( _pOutputCtrl )
	{
		_pOutputCtrl->SetBackgroundColour(kErrorColor);
		_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
	}
}

void FxVM::DisplayErrorW( const FxWString& msg )
{
	FxErrorW(L"%s", msg.GetData());
	if( _pOutputCtrl )
	{
		_pOutputCtrl->SetBackgroundColour(kErrorColor);
		_pOutputCtrl->SetValue(wxString(msg.GetData(), wxConvLibc));
	}
}

FxBool FxVM::BeginBatch( void )
{
	if( _pBatchCommand && !_executedBatch )
	{
		return FxFalse;
	}
	_pBatchCommand = new FxBatchCommand();
	_executedBatch = FxFalse;
	DisplayEcho("batch;");
	return FxTrue;
}

FxBool FxVM::BatchCommand( const FxString& command )
{
	if( _pBatchCommand && !_executedBatch )
	{
		_pBatchCommand->AddToBatch(command);
		DisplayEcho(command);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxVM::ExecBatch( const FxArray<FxString>& args )
{
	if( _pBatchCommand && !_executedBatch )
	{
		_executedBatch = FxTrue;
		FxCommandArgumentList argList;
		for( FxSize i = 0; i < args.Length(); ++i )
		{
			FxCommandArgument flag;
			FxCommandArgumentDesc desc(args[i], args[i], CAT_Flag);
			flag.ArgDesc = desc;
			argList.AddArgument(flag);
		}
		if( CE_Success == _pBatchCommand->Execute(argList) )
		{
			FxString echoString("execBatch");
			for( FxSize i = 0; i < args.Length(); ++i )
			{
				echoString = FxString::Concat(echoString, " ");
				echoString = FxString::Concat(echoString, args[i]);
			}
			echoString = FxString::Concat(echoString, ";");
			DisplayEcho(echoString);
			// Need to check _pBatchCommand again here as the above call to
			// _pBatchCommand->Execute() could have come through this exact
			// code causing itself (_pBatchCommand) to become NULL.
			if( _pBatchCommand && _pBatchCommand->IsUndoable() )
			{
				// The batch command is not deleted here because the memory is
				// now claimed by the undo / redo manager.
				FxUndoRedoManager::PushUndoableCommand(_pBatchCommand);
				_pBatchCommand = NULL;
			}
			else
			{
				delete _pBatchCommand;
				_pBatchCommand = NULL;
			}
			return FxTrue;
		}
		delete _pBatchCommand;
		_pBatchCommand = NULL;
	}
	return FxFalse;
}

void FxVM::SetEchoCommands( FxBool echoCommands )
{
	_echoCommands = echoCommands;
}

FxSize FxVM::GetNumCommands( void )
{
	return _registeredCommands.Length();	
}

FxSize FxVM::GetNumConsoleVariables( void )
{
	return _registeredConsoleVariables.Length();
}

FxVM::FxCommandDesc* FxVM::GetCommandDesc( FxSize index )
{
	return &_registeredCommands[index];
}

FxConsoleVariable* FxVM::GetConsoleVariable( FxSize index )
{
	FxList<FxConsoleVariable>::Iterator cvarIter = _registeredConsoleVariables.Begin();
	cvarIter.Advance(index);
	return &(*cvarIter);
}

FxVM::FxCommandDesc* FxVM::_findCommand( const FxString& commandName )
{
	for( FxSize i = 0; i < _registeredCommands.Length(); ++i )
	{
		if( _registeredCommands[i].CommandName == commandName )
		{
			return &_registeredCommands[i];
		}
	}
	return NULL;
}

FxConsoleVariable* FxVM::_findConsoleVariable( const FxString& cvarName )
{
	FxList<FxConsoleVariable>::Iterator cvarPos = _registeredConsoleVariables.Begin();
	for( ; cvarPos != _registeredConsoleVariables.End(); ++cvarPos )
	{
		if( (*cvarPos).GetNameAsString() == cvarName )
		{
			return &(*cvarPos);
		}
	}
	return NULL;
}

FxArray<FxString> FxVM::_extractCommand( const FxString& command )
{
	FxArray<FxString> splitCommand;
	FxString helperString;
	FxBool inQuote = FxFalse;
	FxSize commandLength = command.Length();
	for( FxSize i = 0; i < commandLength; ++i )
	{
		// There is a quote, so toggle whether or not we're in a quoted argument.
		if( command[i] == '"' )
		{
			inQuote = !inQuote;
		}
		// There is a space or semi-colon indicating a new element or the end
		// of the command string.
		else if( (command[i] == ' ' || command[i] == ';') && !inQuote )
		{
			// Eat extra whitespace.
			if( i+1 < commandLength )
			{
				const FxChar* c = command.GetData()+i+1;
				while( *c == ' ' && i < commandLength )
				{
					++c; ++i;
				}
			}
			if( helperString.Length() )
			{
				splitCommand.PushBack(helperString);
				helperString.Clear();
			}
		}
		// Fill in the helperString.
		else
		{
			helperString = FxString::Concat(helperString, command[i]);
		}
	}
	// Make sure the last argument is picked up.
	if( helperString.Length() > 0 )
	{
		splitCommand.PushBack(helperString);
	}
	helperString.Clear();
	return splitCommand;
}

FxBool FxVM::_createArgumentList( const FxArray<FxString>& splitCommand, 
								  const FxCommandDesc* pCommandDesc,
								  FxCommandArgumentList& argList )
{
	if( pCommandDesc )
	{
		for( FxSize i = 0; i < pCommandDesc->CommandSyntax.GetNumArgDescs(); ++i )
		{
			const FxCommandArgumentDesc& argDesc = pCommandDesc->CommandSyntax.GetArgDesc(i);
			// Search the split command for any occurrence of this argument's
			// short or long name.  Start at one to skip the command name.
			for( FxSize j = 1; j < splitCommand.Length(); ++j )
			{
				if( splitCommand[j] == argDesc.ArgNameLong ||
					splitCommand[j] == argDesc.ArgNameShort )
				{
					// Make sure something follows this argument if it is 
					// supposed to.
					if( argDesc.ArgType != CAT_Flag )
					{
						if( j+1 < splitCommand.Length() )
						{
							FxCommandArgument arg;
							arg.ArgDesc = argDesc;
							arg.Arg = splitCommand[j+1];
							argList.AddArgument(arg);
							break;
						}
						else
						{
                            return FxFalse;
						}
					}
					else
					{
						FxCommandArgument arg;
						arg.ArgDesc = argDesc;
						argList.AddArgument(arg);
						break;
					}
				}
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

void FxVM::_outputResult( FxCommandError commandError, 
						  const FxString& FxUnused(commandName), 
						  const FxString& extraInfo,
						  FxCommand* pCommand )
{
	switch( commandError )
	{
	case CE_Failure:
		{
			FxString errorMsg = "Command Failed:  ";
			errorMsg = FxString::Concat(errorMsg, extraInfo);
			FxError("%s", errorMsg.GetData());
			if( _pOutputCtrl )
			{
				_pOutputCtrl->SetBackgroundColour(kErrorColor);
				_pOutputCtrl->SetValue(wxString(errorMsg.GetData(), wxConvLibc));
			}
		}
		break;
	case CE_ArgumentFailure:
		{
			FxString errorMsg = "Invalid Arguments: ";
			errorMsg = FxString::Concat(errorMsg, extraInfo);
			FxError("%s", errorMsg.GetData());
			if( _pOutputCtrl )
			{
				_pOutputCtrl->SetBackgroundColour(kErrorColor);
				_pOutputCtrl->SetValue(wxString(errorMsg.GetData(), wxConvLibc));
			}
		}
		break;
	case CE_InvalidCommand:
		{
			FxString errorMsg = "Unrecognized Command: ";
			errorMsg = FxString::Concat(errorMsg, extraInfo);
			FxError("%s", errorMsg.GetData());
			if( _pOutputCtrl )
			{
				_pOutputCtrl->SetBackgroundColour(kErrorColor);
				_pOutputCtrl->SetValue(wxString(errorMsg.GetData(), wxConvLibc));
			}
		}
		break;
	case CE_InternalError:
		{
			FxString errorMsg = "Internal Error:  ";
			errorMsg = FxString::Concat(errorMsg, extraInfo);
			FxError("%s", errorMsg.GetData());
			if( _pOutputCtrl )
			{
				_pOutputCtrl->SetBackgroundColour(kErrorColor);
				_pOutputCtrl->SetValue(wxString(errorMsg.GetData(), wxConvLibc));
			}
		}
		break;
	default:
	case CE_Success:
		{
			if( pCommand && pCommand->ShouldEcho() && _echoCommands )
			{
				FxEcho("%s", extraInfo.GetData());
				if( _pOutputCtrl )
				{
					_pOutputCtrl->SetBackgroundColour(kNormalColor);
					_pOutputCtrl->SetValue(wxString(extraInfo.GetData(), wxConvLibc));
				}
			}
		}
		break;
	}
}

} // namespace Face

} // namespace OC3Ent
