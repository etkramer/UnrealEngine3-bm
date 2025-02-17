//------------------------------------------------------------------------------
// A MEL command to create a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaCreateActorCommand.h"

// Constructor.
FxMayaCreateActorCommand::FxMayaCreateActorCommand()
{
}

// Destructor.
FxMayaCreateActorCommand::~FxMayaCreateActorCommand()
{
}

// Create a new instance of this command.
void* FxMayaCreateActorCommand::creator( void )
{
	return new FxMayaCreateActorCommand();
}

// The command syntax.
MSyntax FxMayaCreateActorCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kActorNameFlag, kActorNameFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaCreateActorCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.CreateActor(OC3Ent::Face::FxString(_actorName.asChar()));

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaCreateActorCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kActorNameFlag, 0, _actorName);
	return MS::kSuccess;
}
