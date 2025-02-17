//------------------------------------------------------------------------------
// A MEL command to toggle FaceFX logging to Maya's output panel.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaLogToOutputWindowCommand.h"

// Constructor.
FxMayaLogToOutputWindowCommand::FxMayaLogToOutputWindowCommand()
{
}

// Destructor.
FxMayaLogToOutputWindowCommand::~FxMayaLogToOutputWindowCommand()
{
}

// Create a new instance of this command.
void* FxMayaLogToOutputWindowCommand::creator( void )
{
	return new FxMayaLogToOutputWindowCommand();
}

// The command syntax.
MSyntax FxMayaLogToOutputWindowCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kToggleFlag, kToggleFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaLogToOutputWindowCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxBool logToCout = FxFalse;
	if( _toggle.toLowerCase() == "on" )
	{
		logToCout = FxTrue;
	}
	OC3Ent::Face::FxToolLogFile::SetEchoToCout(logToCout);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaLogToOutputWindowCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kToggleFlag, 0, _toggle);
	return MS::kSuccess;
}
