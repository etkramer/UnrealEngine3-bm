//------------------------------------------------------------------------------
// A MEL command to import a bone pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaImportBonePoseCommand_H__
#define FxMayaImportBonePoseCommand_H__

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
#define kNameFlag      "-n"
#define kNameFlagLong  "-name"

// Import a bone pose for FaceFx actor command.
class FxMayaImportBonePoseCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaImportBonePoseCommand();
	// Destructor.
	virtual ~FxMayaImportBonePoseCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The frame number that the bone pose is to be imported on.
	OC3Ent::Face::FxInt32 _frame;
	// The name of the bone pose to import.
	MString _name;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#define kFileFlag     "-f"
#define kFileFlagLong "-file"

// Batch import bone poses command.
class FxMayaBatchImportBonePosesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaBatchImportBonePosesCommand();
	// Destructor.
	virtual ~FxMayaBatchImportBonePosesCommand();
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
