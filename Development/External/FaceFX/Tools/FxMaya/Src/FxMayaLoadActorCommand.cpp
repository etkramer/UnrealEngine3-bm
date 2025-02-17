//------------------------------------------------------------------------------
// A MEL command to load a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaLoadActorCommand.h"

// Constructor.
FxMayaLoadActorCommand::FxMayaLoadActorCommand()
{
}

// Destructor.
FxMayaLoadActorCommand::~FxMayaLoadActorCommand()
{
}

// Create a new instance of this command.
void* FxMayaLoadActorCommand::creator( void )
{
	return new FxMayaLoadActorCommand();
}

// The command syntax.
MSyntax FxMayaLoadActorCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kActorFileFlag, kActorFileFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaLoadActorCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.LoadActor(OC3Ent::Face::FxString(_actorFilename.asChar()));

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaLoadActorCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kActorFileFlag, 0, _actorFilename);
	return MS::kSuccess;
}
