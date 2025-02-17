//------------------------------------------------------------------------------
// A MEL command to load a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaLoadActorCommand_H__
#define FxMayaLoadActorCommand_H__

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
class FxMayaLoadActorCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaLoadActorCommand();
	// Destructor.
	virtual ~FxMayaLoadActorCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The filename of the actor to load.
	MString _actorFilename;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
