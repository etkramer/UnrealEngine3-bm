//------------------------------------------------------------------------------
// A MEL command to import the reference pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaImportRefPoseCommand_H__
#define FxMayaImportRefPoseCommand_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kFrameFlag     "-f"
#define kFrameFlagLong "-frame"

// Import reference pose for FaceFx actor command.
class FxMayaImportRefPoseCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaImportRefPoseCommand();
	// Destructor.
	virtual ~FxMayaImportRefPoseCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The frame number that the reference pose is to be imported on.
	OC3Ent::Face::FxInt32 _frame;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
