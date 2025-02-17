//------------------------------------------------------------------------------
// Various MEL commands to query the state of a FaceFx actor.  These commands
// are useful for implementing a UI in MEL.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaQueryCommands_H__
#define FxMayaQueryCommands_H__

#include "FxPlatform.h"

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MArgList.h>

//------------------------------------------------------------------------------
// Get the name of an actor as a string.
//------------------------------------------------------------------------------
class FxMayaGetActorNameCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetActorNameCommand();
	// Destructor.
	virtual ~FxMayaGetActorNameCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );
};

//------------------------------------------------------------------------------
// Get a string array of animation group names in an actor.
//------------------------------------------------------------------------------
class FxMayaGetAnimGroupsCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetAnimGroupsCommand();
	// Destructor.
	virtual ~FxMayaGetAnimGroupsCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );
};

//------------------------------------------------------------------------------
// Get a string array of animation names in the specified group in an actor.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
class FxMayaGetAnimsCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetAnimsCommand();
	// Destructor.
	virtual ~FxMayaGetAnimsCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The name of the group to query.
	MString _group;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Gets a string array of curve names included in the specified animation in
// the specified group of an actor.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
class FxMayaGetCurvesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetCurvesCommand();
	// Destructor.
	virtual ~FxMayaGetCurvesCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Get a string array of face graph node names of the specified type in an actor.
//------------------------------------------------------------------------------
#define kTypeFlag     "-t"
#define kTypeFlagLong "-type"
class FxMayaGetNodesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetNodesCommand();
	// Destructor.
	virtual ~FxMayaGetNodesCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// The node type to query.
	MString _type;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Get a string array of bone names included in the reference pose of an actor.
//------------------------------------------------------------------------------
class FxMayaGetRefBonesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetRefBonesCommand();
	// Destructor.
	virtual ~FxMayaGetRefBonesCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );
};

//------------------------------------------------------------------------------
// Get a string array of bone names included in the specified pose of an actor.
//------------------------------------------------------------------------------
#define kNameFlag     "-n"
#define kNameFlagLong "-name"
class FxMayaGetBonesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetBonesCommand();
	// Destructor.
	virtual ~FxMayaGetBonesCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );
private:
	// The bone pose to query.
	MString _name;

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key values for the specified curve in the specified
// group and animation.  Most users will want to get "baked" curves instead 
// to constuct a curve that accurately reflects a node's value over time.  
// The function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetRawCurveKeyValuesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetRawCurveKeyValuesCommand();
	// Destructor.
	virtual ~FxMayaGetRawCurveKeyValuesCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key times for the specified curve in the specified
// group and animation.  Most users will want to get "baked" curves instead 
// to constuct a curve that accurately reflects a node's value over time.  
// The function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetRawCurveKeyTimesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetRawCurveKeyTimesCommand();
	// Destructor.
	virtual ~FxMayaGetRawCurveKeyTimesCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key input slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to constuct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetRawCurveKeySlopeInCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetRawCurveKeySlopeInCommand();
	// Destructor.
	virtual ~FxMayaGetRawCurveKeySlopeInCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key output slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to constuct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetRawCurveKeySlopeOutCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetRawCurveKeySlopeOutCommand();
	// Destructor.
	virtual ~FxMayaGetRawCurveKeySlopeOutCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key values for the specified node in the specified 
// group and animation.  The funtion evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function should be used in
// conjunction with GetBakedCurveKeyTimes. The function returns an empty 
// array if the node or animation can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetBakedCurveKeyValuesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetBakedCurveKeyValuesCommand();
	// Destructor.
	virtual ~FxMayaGetBakedCurveKeyValuesCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key values for the specified node in the specified 
// group and animation.  The funtion evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function should be used in
// conjunction with GetBakedCurveKeyTimes. The function returns an empty 
// array if the node or animation can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetBakedCurveKeyTimesCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetBakedCurveKeyTimesCommand();
	// Destructor.
	virtual ~FxMayaGetBakedCurveKeyTimesCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key output slopes for the specified node in the specified 
// group and animation.  The funtion evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function should be used in
// conjunction with GetBakedCurveKeyTimes. The function returns an empty 
// array if the node or animation can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetBakedCurveKeySlopeOutCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetBakedCurveKeySlopeOutCommand();
	// Destructor.
	virtual ~FxMayaGetBakedCurveKeySlopeOutCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns an array of key input slopes for the specified node in the specified 
// group and animation.  The funtion evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function should be used in
// conjunction with GetBakedCurveKeyTimes. The function returns an empty 
// array if the node or animation can't be found.
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
#define kCurveFlag      "-c"
#define kCurveFlagLong  "-curve"
class FxMayaGetBakedCurveKeySlopeInCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetBakedCurveKeySlopeInCommand();
	// Destructor.
	virtual ~FxMayaGetBakedCurveKeySlopeInCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns the specified animation length in seconds
//------------------------------------------------------------------------------
#define kGroupFlag     "-g"
#define kGroupFlagLong "-group"
#define kAnimFlag      "-a"
#define kAnimFlagLong  "-anim"
class FxMayaGetAnimDurationCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaGetAnimDurationCommand();
	// Destructor.
	virtual ~FxMayaGetAnimDurationCommand();
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

	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns if the plugin is a mod developer plugin
//------------------------------------------------------------------------------
class FxMayaIsModDevloperCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaIsModDevloperCommand();
	// Destructor.
	virtual ~FxMayaIsModDevloperCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

//------------------------------------------------------------------------------
// Returns if the plugin is a no-save plugin
//------------------------------------------------------------------------------
class FxMayaIsNoSaveCommand : public MPxCommand
{
public:
	// Constructor.
	FxMayaIsNoSaveCommand();
	// Destructor.
	virtual ~FxMayaIsNoSaveCommand();
	// Create a new instance of this command.
	static void* creator( void );
	// The command syntax.
	static MSyntax newSyntax( void );
	// Execute the command.
	virtual MStatus	doIt( const MArgList& args );

private:
	// Parse the command arguments.
	MStatus	_parseArgs( const MArgList& args );
};

#endif
