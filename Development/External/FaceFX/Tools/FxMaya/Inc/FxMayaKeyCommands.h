//------------------------------------------------------------------------------
// MEL commands to modify keys in a FaceFX curve.
//
// Owner: Doug Perkowski
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaKeyCommands_H__
#define FxMayaKeyCommands_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"

#define kFrameFlag        "-f"
#define kFrameFlagLong    "-frame"
#define kValueFlag			"-v"
#define kValueFlagLong		"-value"
#define kSlopeInFlag		"-i"
#define kSlopeInFlagLong	"-inslope"
#define kSlopeOutFlag		"-o"
#define kSlopeOutFlagLong	"-outslope"

// Export reference pose for FaceFx actor command.
class FxMayaInsertKeyCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaInsertKeyCommand();
	// Destructor.
	virtual ~FxMayaInsertKeyCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );


private:
	// The name of the animation group to query.
	MString _group;
	// The name of the animation to query.
	MString _anim;
	// The name of the curve to query.
	MString _curve;

	// The key time.
	OC3Ent::Face::FxReal _time;
	// The key value.
	OC3Ent::Face::FxReal _value;
	// The key slope in.
	OC3Ent::Face::FxReal _slopeIn;
	// The key slope out.
	OC3Ent::Face::FxReal _slopeOut;


	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};



// Export reference pose for FaceFx actor command.
class FxMayaDeleteAllKeysCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaDeleteAllKeysCommand();
	// Destructor.
	virtual ~FxMayaDeleteAllKeysCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );


private:
	// The name of the animation group to query.
	MString _group;
	// The name of the animation to query.
	MString _anim;
	// The name of the curve to query.
	MString _curve;


	// The key time.
	OC3Ent::Face::FxReal _time;
	// The key value.
	OC3Ent::Face::FxReal _value;
	// The key slope in.
	OC3Ent::Face::FxReal _slopeIn;
	// The key slope out.
	OC3Ent::Face::FxReal _slopeOut;
	


	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};
#endif
