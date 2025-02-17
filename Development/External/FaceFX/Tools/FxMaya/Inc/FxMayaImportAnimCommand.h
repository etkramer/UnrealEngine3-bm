//------------------------------------------------------------------------------
// A MEL command to import an animation for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaImportAnimCommand_H__
#define FxMayaImportAnimCommand_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kGroupFlag         "-g"
#define kGroupFlagLong     "-group"
#define kAnimFlag          "-a"
#define kAnimFlagLong      "-anim"
#define kFrameRateFlag     "-f"
#define kFrameRateFlagLong "-framerate"

// Import an animation for FaceFx actor command.
class FxMayaImportAnimCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaImportAnimCommand();
	// Destructor.
	virtual ~FxMayaImportAnimCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The name of the animation group to import from.
	MString _group;
	// The name of the animation to import.
	MString _anim;
	// The frame rate at which to import the animation.
	OC3Ent::Face::FxDReal _fps;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
