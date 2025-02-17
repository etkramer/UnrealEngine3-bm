//------------------------------------------------------------------------------
// A MEL command to import an animation for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaImportAnimCommand.h"

// Constructor.
FxMayaImportAnimCommand::FxMayaImportAnimCommand()
{
	// Default the frame rate to 60.0.
	_fps = 60.0;
}

// Destructor.
FxMayaImportAnimCommand::~FxMayaImportAnimCommand()
{
}

// Create a new instance of this command.
void* FxMayaImportAnimCommand::creator( void )
{
	return new FxMayaImportAnimCommand();
}

// The command syntax.
MSyntax FxMayaImportAnimCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kFrameRateFlag, kFrameRateFlagLong, MSyntax::MArgType::kDouble);
	return syntax;
}

// Execute the command.
MStatus	FxMayaImportAnimCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;
	
	OC3Ent::Face::FxMayaData::mayaInterface.ImportAnim(OC3Ent::Face::FxString(_group.asChar()), 
		OC3Ent::Face::FxString(_anim.asChar()), static_cast<OC3Ent::Face::FxReal>(_fps));

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaImportAnimCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kFrameRateFlag, 0, _fps);
	return MS::kSuccess;
}
