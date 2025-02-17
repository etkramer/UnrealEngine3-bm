//------------------------------------------------------------------------------
// A MEL command to import the reference pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaImportRefPoseCommand.h"

// Constructor.
FxMayaImportRefPoseCommand::FxMayaImportRefPoseCommand()
{
}

// Destructor.
FxMayaImportRefPoseCommand::~FxMayaImportRefPoseCommand()
{
}

// Create a new instance of this command.
void* FxMayaImportRefPoseCommand::creator( void )
{
	return new FxMayaImportRefPoseCommand();
}

// The command syntax.
MSyntax FxMayaImportRefPoseCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kFrameFlag, kFrameFlagLong, MSyntax::MArgType::kLong);
	return syntax;
}

// Execute the command.
MStatus	FxMayaImportRefPoseCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.ImportRefPose(_frame);
	
	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaImportRefPoseCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kFrameFlag, 0, _frame);
	return MS::kSuccess;
}
