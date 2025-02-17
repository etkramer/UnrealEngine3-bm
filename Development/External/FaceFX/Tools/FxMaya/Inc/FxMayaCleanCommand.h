//------------------------------------------------------------------------------
// A MEL command to clean up FaceFx animation curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaCleanCommand_H__
#define FxMayaCleanCommand_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

// Clean up FaceFx animation curves.
class FxMayaCleanCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaCleanCommand();
	// Destructor.
	virtual ~FxMayaCleanCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );
};

#endif
