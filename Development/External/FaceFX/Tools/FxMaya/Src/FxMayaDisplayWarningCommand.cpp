//------------------------------------------------------------------------------
// A MEL command that enables the user to turn off all warning dialogs.
//
// Owner: Doug Perkowski
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaDisplayWarningCommand.h"

//------------------------------------------------------------------------------
// Set the display warning flag
//------------------------------------------------------------------------------

// Constructor.
FxMayaDisplayWarningCommand::FxMayaDisplayWarningCommand()
{
}

// Destructor.
FxMayaDisplayWarningCommand::~FxMayaDisplayWarningCommand()
{
}

// Create a new instance of this command.
void* FxMayaDisplayWarningCommand::creator( void )
{
	return new FxMayaDisplayWarningCommand();
}

// The command syntax.
MSyntax FxMayaDisplayWarningCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kToggleFlag, kToggleFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaDisplayWarningCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxBool displayWarningCommand = FxFalse;
	if( _toggle.toLowerCase() == "on" )
	{
		displayWarningCommand = FxTrue;
	}
	OC3Ent::Face::FxMayaData::mayaInterface.SetShouldDisplayWarningDialogs(displayWarningCommand);

	return MS::kSuccess;

}
// Parse the command arguments.
MStatus FxMayaDisplayWarningCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kToggleFlag, 0, _toggle);
	return MS::kSuccess;
};
