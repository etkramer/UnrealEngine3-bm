//------------------------------------------------------------------------------
// A MEL command that enables the user to turn off all warning dialogs.
//
// Owner: Doug Perkowski
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaDisplayWarningCommand_H__
#define FxMayaDisplayWarningCommand_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kToggleFlag     "-t"
#define kToggleFlagLong "-toggle"

//------------------------------------------------------------------------------
// Set the display warning flag
//------------------------------------------------------------------------------
class FxMayaDisplayWarningCommand : public MPxCommand

{
public:
	// Constructor.
	FxMayaDisplayWarningCommand();
	// Destructor.
	virtual ~FxMayaDisplayWarningCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );
private:
	// The toggle.
	MString _toggle;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
