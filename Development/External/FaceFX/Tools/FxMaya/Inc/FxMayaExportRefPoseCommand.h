//------------------------------------------------------------------------------
// A MEL command to export the reference pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaExportRefPoseCommand_H__
#define FxMayaExportRefPoseCommand_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kFrameFlag        "-f"
#define kFrameFlagLong    "-frame"
#define kBoneListFlag	  "-bl"
#define kBoneListFlagLong "-bones"

// Export reference pose for FaceFx actor command.
class FxMayaExportRefPoseCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaExportRefPoseCommand();
	// Destructor.
	virtual ~FxMayaExportRefPoseCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The frame number that the reference pose is contained on.
	OC3Ent::Face::FxInt32 _frame;
	// The single string separated by spaces containing the reference pose bone
	// names (as passed in from MEL).
	MString _bones;
	// The list of bone names contained in the reference pose.
	MStringArray _boneList;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
