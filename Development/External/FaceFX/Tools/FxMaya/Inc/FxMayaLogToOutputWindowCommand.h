//------------------------------------------------------------------------------
// A MEL command to toggle FaceFX logging to Maya's output panel.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaLogToOutputWindowCommand_H__
#define FxMayaLogToOutputWindowCommand_H__

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kToggleFlag     "-t"
#define kToggleFlagLong "-toggle"

// Toggle logging to Maya's output window command.
class FxMayaLogToOutputWindowCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaLogToOutputWindowCommand();
	// Destructor.
	virtual ~FxMayaLogToOutputWindowCommand();
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
