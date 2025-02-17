//------------------------------------------------------------------------------
// A MEL command to create a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaCreateActorCommand_H__
#define FxMayaCreateActorCommand_H__

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kActorNameFlag	   "-n"
#define kActorNameFlagLong "-name"

// Create FaceFx actor command.
class FxMayaCreateActorCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaCreateActorCommand();
	// Destructor.
	virtual ~FxMayaCreateActorCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The name of the actor.
	MString _actorName;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
