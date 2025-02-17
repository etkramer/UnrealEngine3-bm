//------------------------------------------------------------------------------
// A MEL command to export a bone pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaExportBonePoseCommand_H__
#define FxMayaExportBonePoseCommand_H__

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
#define kNameFlag	   "-n"
#define kNameFlagLong  "-name"

// Export a bone pose for FaceFx actor command.
class FxMayaExportBonePoseCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaExportBonePoseCommand();
	// Destructor.
	virtual ~FxMayaExportBonePoseCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The frame number that the reference pose is contained on.
	OC3Ent::Face::FxInt32 _frame;
	// The list of bone names contained in the reference pose.
	MString _name;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#define kFileFlag     "-f"
#define kFileFlagLong "-file"

// Batch export bone poses command.
class FxMayaBatchExportBonePosesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaBatchExportBonePosesCommand();
	// Destructor.
	virtual ~FxMayaBatchExportBonePosesCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus doIt( const MArgList& args );

private:
	// The batch file.
	MString _filename;

	// Parse the command arguments.
	MStatus _parseArgs( const MArgList& args );
};

#endif
