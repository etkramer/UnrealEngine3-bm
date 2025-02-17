//------------------------------------------------------------------------------
// The exec command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxExecCommand.h"
#include "FxVM.h"
#include "FxFile.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxExecCommand, 0, FxCommand);

FxExecCommand::FxExecCommand()
{
}

FxExecCommand::~FxExecCommand()
{
}

FxCommandSyntax FxExecCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	return newSyntax;
}

FxCommandError FxExecCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString scriptFilename;
	if( argList.GetArgument("-file", scriptFilename) )
	{
		// Load the entire file to a string.  The file must be opened in binary
		// mode for this reading as one huge chunk of bytes to work correctly.
		FxFile scriptFile(scriptFilename.GetData(), FxFile::FM_Read|FxFile::FM_Binary);
		if( !scriptFile.IsValid() )
		{
			FxString errorString = "Unable to open script file: ";
			errorString += scriptFilename;
			FxVM::DisplayError(errorString);
			return CE_Failure;
		}

		FxInt32 fileLen = scriptFile.Length();
		FxChar* scriptFileStr = new FxChar[fileLen + 1];
		FxMemset(scriptFileStr, 0, fileLen + 1);
		scriptFile.Read(reinterpret_cast<FxByte*>(scriptFileStr), fileLen);
		scriptFile.Close();

		FxString script(scriptFileStr);
		delete [] scriptFileStr;

		// Convert '\r' to ' ' leaving '\n' alone.
		for( FxSize i = 0; i < script.Length(); ++i )
		{
			if( script[i] == '\r' )
			{
				script[i] = ' ';
			}
		}
        
		// Parse out each individual command into an array of commands.
        FxArray<FxString> commands;
		if( !_parseCommands(script, commands) )
		{
			FxVM::DisplayError("Unable to parse script!");
			return CE_Failure;
		}
		// Execute each command.
		for( FxSize i = 0; i < commands.Length(); ++i )
		{
			if( CE_Failure == FxVM::ExecuteCommand(commands[i]) )
			{
				return CE_Failure;
			}
		}
		return CE_Success;
	}
	FxVM::DisplayError("-file was not specified!");
	return CE_Failure;
}

FxBool FxExecCommand::
_parseCommands( const FxString& script, FxArray<FxString>& commands )
{
	FxString helperString;
	FxBool inComment = FxFalse;
	FxBool inCommand = FxFalse;
	for( FxSize i = 0; i < script.Length(); ++i )
	{
		if( inComment )
		{
			if( script[i] == '\n' )
			{
				inComment = FxFalse;
			}
		}
		else if( script[i] == '/' )
		{
			if( (i+1) < script.Length() )
			{
				if( script[i+1] == '/' )
				{
					inComment = FxTrue;
				}
				else
				{
					helperString = FxString::Concat(helperString, script[i]);
				}
			}
		}
		// There is a newline or semi-colon indicating the end of the command.
		else if( script[i] == '\n' || script[i] == ';' )
		{
			helperString = FxString::Concat(helperString, ';');
			if( helperString.Length() > 1 )
			{
				commands.PushBack(helperString);
			}
			helperString.Clear();
			inCommand = FxFalse;
		}
		// Fill in helperString.
		else if( inCommand )
		{
			helperString = FxString::Concat(helperString, script[i]);
		}
		else if( script[i] != ' ' && script[i] != '\t' )
		{
			helperString = FxString::Concat(helperString, script[i]);
			inCommand = FxTrue;
		}
	}
	// Make sure the last command is picked up.
	if( helperString.Length() > 1 && !inComment )
	{
		commands.PushBack(helperString);
	}
	helperString.Clear();
	return FxTrue;
}

} // namespace Face

} // namespace OC3Ent
