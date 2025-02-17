//------------------------------------------------------------------------------
// A MEL command to save a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaSaveActorCommand_H__
#define FxMayaSaveActorCommand_H__

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kActorFileFlag	   "-f"
#define kActorFileFlagLong "-file"

// Import FaceFx actor command.
class FxMayaSaveActorCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaSaveActorCommand();
	// Destructor.
	virtual ~FxMayaSaveActorCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The filename of the actor to save.
	MString _actorFilename;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
