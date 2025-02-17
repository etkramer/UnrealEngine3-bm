//------------------------------------------------------------------------------
// A MEL command to save a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaSaveActorCommand.h"

// Constructor.
FxMayaSaveActorCommand::FxMayaSaveActorCommand()
{
}

// Destructor.
FxMayaSaveActorCommand::~FxMayaSaveActorCommand()
{
}

// Create a new instance of this command.
void* FxMayaSaveActorCommand::creator( void )
{
	return new FxMayaSaveActorCommand();
}

// The command syntax.
MSyntax FxMayaSaveActorCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kActorFileFlag, kActorFileFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaSaveActorCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.SaveActor(OC3Ent::Face::FxString(_actorFilename.asChar()));

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaSaveActorCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kActorFileFlag, 0, _actorFilename);
	return MS::kSuccess;
}
