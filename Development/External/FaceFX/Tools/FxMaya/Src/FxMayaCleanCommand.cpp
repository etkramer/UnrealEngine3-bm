//------------------------------------------------------------------------------
// A MEL command to clean up FaceFx animation curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaCleanCommand.h"

// Constructor.
FxMayaCleanCommand::FxMayaCleanCommand()
{
}

// Destructor.
FxMayaCleanCommand::~FxMayaCleanCommand()
{
}

// Create a new instance of this command.
void* FxMayaCleanCommand::creator( void )
{
	return new FxMayaCleanCommand();
}

// The command syntax.
MSyntax FxMayaCleanCommand::newSyntax( void )
{
	MSyntax syntax;
	return syntax;
}

// Execute the command.
MStatus	FxMayaCleanCommand::doIt( const MArgList& args )
{
	OC3Ent::Face::FxMayaData::mayaInterface.Clean();
	return MS::kSuccess;
};
