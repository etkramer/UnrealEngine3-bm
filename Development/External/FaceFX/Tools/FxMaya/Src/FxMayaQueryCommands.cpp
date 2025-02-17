//------------------------------------------------------------------------------
// Various MEL commands to query the state of a FaceFx actor.  These commands
// are useful for implementing a UI in MEL.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaQueryCommands.h"

//------------------------------------------------------------------------------
// Get the name of an actor as a string.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetActorNameCommand::FxMayaGetActorNameCommand()
{
}

// Destructor.
FxMayaGetActorNameCommand::~FxMayaGetActorNameCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetActorNameCommand::creator( void )
{
	return new FxMayaGetActorNameCommand();
}

// The command syntax.
MSyntax FxMayaGetActorNameCommand::newSyntax( void )
{
	MSyntax syntax;
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetActorNameCommand::doIt( const MArgList& args )
{
	clearResult();
	setResult(MString(OC3Ent::Face::FxMayaData::mayaInterface.GetActorName().GetData()));
	return MS::kSuccess;
};

//------------------------------------------------------------------------------
// Get a string array of animation group names in an actor.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetAnimGroupsCommand::FxMayaGetAnimGroupsCommand()
{
}

// Destructor.
FxMayaGetAnimGroupsCommand::~FxMayaGetAnimGroupsCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetAnimGroupsCommand::creator( void )
{
	return new FxMayaGetAnimGroupsCommand();
}

// The command syntax.
MSyntax FxMayaGetAnimGroupsCommand::newSyntax( void )
{
	MSyntax syntax;
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetAnimGroupsCommand::doIt( const MArgList& args )
{
	clearResult();
	MStringArray animGroups;
	OC3Ent::Face::FxSize numAnimGroups = OC3Ent::Face::FxMayaData::mayaInterface.GetNumAnimGroups();
	for( OC3Ent::Face::FxSize i = 0; i < numAnimGroups; ++i )
	{
		animGroups.append(MString(OC3Ent::Face::FxMayaData::mayaInterface.GetAnimGroupName(i).GetData()));
	}
	setResult(animGroups);
	return MS::kSuccess;
};

//------------------------------------------------------------------------------
// Get a string array of animation names in the specified group in an actor.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetAnimsCommand::FxMayaGetAnimsCommand()
{
}

// Destructor.
FxMayaGetAnimsCommand::~FxMayaGetAnimsCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetAnimsCommand::creator( void )
{
	return new FxMayaGetAnimsCommand();
}

// The command syntax.
MSyntax FxMayaGetAnimsCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetAnimsCommand::doIt( const MArgList& args )
{
	clearResult();
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MStringArray anims;
	OC3Ent::Face::FxSize numAnims = OC3Ent::Face::FxMayaData::mayaInterface.GetNumAnims(_group.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < numAnims; ++i )
	{
		anims.append(MString(OC3Ent::Face::FxMayaData::mayaInterface.GetAnimName(_group.asChar(), i).GetData()));
	}
	setResult(anims);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaGetAnimsCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Gets a string array of curve names included in the specified animation in
// the specified group of an actor.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetCurvesCommand::FxMayaGetCurvesCommand()
{
}

// Destructor.
FxMayaGetCurvesCommand::~FxMayaGetCurvesCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetCurvesCommand::creator( void )
{
	return new FxMayaGetCurvesCommand();
}

// The command syntax.
MSyntax FxMayaGetCurvesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetCurvesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MStringArray curves;
	OC3Ent::Face::FxSize numCurves = OC3Ent::Face::FxMayaData::mayaInterface.GetNumAnimCurves(_group.asChar(), _anim.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < numCurves; ++i )
	{
		curves.append(MString(OC3Ent::Face::FxMayaData::mayaInterface.GetAnimCurveName(_group.asChar(), _anim.asChar(), i).GetData()));
	}
	setResult(curves);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaGetCurvesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Get a string array of face graph node names of the specified type in an actor.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetNodesCommand::FxMayaGetNodesCommand()
{
}

// Destructor.
FxMayaGetNodesCommand::~FxMayaGetNodesCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetNodesCommand::creator( void )
{
	return new FxMayaGetNodesCommand();
}

// The command syntax.
MSyntax FxMayaGetNodesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kTypeFlag, kTypeFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetNodesCommand::doIt( const MArgList& args )
{
	clearResult();
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxArray<OC3Ent::Face::FxString> nodeNames = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetNodesOfType(_type.asChar());
	MStringArray nodes;
	for( OC3Ent::Face::FxSize i = 0; i < nodeNames.Length(); ++i )
	{
		nodes.append(MString(nodeNames[i].GetData()));
	}
	setResult(nodes);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaGetNodesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kTypeFlag, 0, _type);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Get a string array of bone names included in the reference pose of an actor.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetRefBonesCommand::FxMayaGetRefBonesCommand()
{
}

// Destructor.
FxMayaGetRefBonesCommand::~FxMayaGetRefBonesCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetRefBonesCommand::creator( void )
{
	return new FxMayaGetRefBonesCommand();
}

// The command syntax.
MSyntax FxMayaGetRefBonesCommand::newSyntax( void )
{
	MSyntax syntax;
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetRefBonesCommand::doIt( const MArgList& args )
{
	clearResult();
	MStringArray refBones;
	OC3Ent::Face::FxSize numRefBones = OC3Ent::Face::FxMayaData::mayaInterface.GetNumRefBones();
	for( OC3Ent::Face::FxSize i = 0; i < numRefBones; ++i )
	{
		refBones.append(MString(OC3Ent::Face::FxMayaData::mayaInterface.GetRefBoneName(i).GetData()));
	}
	setResult(refBones);
	return MS::kSuccess;
};

//------------------------------------------------------------------------------
// Get a string array of bone names included in the specified pose of an actor.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetBonesCommand::FxMayaGetBonesCommand()
{
}

// Destructor.
FxMayaGetBonesCommand::~FxMayaGetBonesCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetBonesCommand::creator( void )
{
	return new FxMayaGetBonesCommand();
}

// The command syntax.
MSyntax FxMayaGetBonesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kNameFlag, kNameFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetBonesCommand::doIt( const MArgList& args )
{
	clearResult();
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MStringArray bones;
	OC3Ent::Face::FxSize numBones = OC3Ent::Face::FxMayaData::mayaInterface.GetNumBones(_name.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < numBones; ++i )
	{
		bones.append(MString(OC3Ent::Face::FxMayaData::mayaInterface.GetBoneName(_name.asChar(), i).GetData()));
	}
	setResult(bones);
	
	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaGetBonesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kNameFlag, 0, _name);
	return MS::kSuccess;
}



//------------------------------------------------------------------------------
// Returns an array of key values for the specified curve in the specified
// group and animation.  Most users will want to get "baked" curves instead 
// to constuct a curve that accurately reflects a node's value over time.  
// The function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetRawCurveKeyValuesCommand::FxMayaGetRawCurveKeyValuesCommand()
{
}

// Destructor.
FxMayaGetRawCurveKeyValuesCommand::~FxMayaGetRawCurveKeyValuesCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetRawCurveKeyValuesCommand::creator( void )
{
	return new FxMayaGetRawCurveKeyValuesCommand();
}
// The command syntax.
MSyntax FxMayaGetRawCurveKeyValuesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetRawCurveKeyValuesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetRawCurveKeyValues(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetRawCurveKeyValuesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns an array of key times for the specified curve in the specified
// group and animation.  Most users will want to get "baked" curves instead 
// to constuct a curve that accurately reflects a node's value over time.  
// The function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetRawCurveKeyTimesCommand::FxMayaGetRawCurveKeyTimesCommand()
{
}
// Destructor.
FxMayaGetRawCurveKeyTimesCommand::~FxMayaGetRawCurveKeyTimesCommand()
{
}

// Create a new instance of this command.
void* FxMayaGetRawCurveKeyTimesCommand::creator( void )
{
	return new FxMayaGetRawCurveKeyTimesCommand();
}
// The command syntax.
MSyntax FxMayaGetRawCurveKeyTimesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetRawCurveKeyTimesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetRawCurveKeyTimes(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetRawCurveKeyTimesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns an array of key input slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to constuct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetRawCurveKeySlopeInCommand::FxMayaGetRawCurveKeySlopeInCommand()
{
}

// Destructor.
FxMayaGetRawCurveKeySlopeInCommand::~FxMayaGetRawCurveKeySlopeInCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetRawCurveKeySlopeInCommand::creator( void )
{
	return new FxMayaGetRawCurveKeySlopeInCommand();
}

// The command syntax.
MSyntax FxMayaGetRawCurveKeySlopeInCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetRawCurveKeySlopeInCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetRawCurveKeySlopeIn(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};


// Parse the command arguments.
MStatus	FxMayaGetRawCurveKeySlopeInCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns an array of key output slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to constuct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetRawCurveKeySlopeOutCommand::FxMayaGetRawCurveKeySlopeOutCommand()
{
}
// Destructor.
FxMayaGetRawCurveKeySlopeOutCommand::~FxMayaGetRawCurveKeySlopeOutCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetRawCurveKeySlopeOutCommand::creator( void )
{
	return new FxMayaGetRawCurveKeySlopeOutCommand();
}

// The command syntax.
MSyntax FxMayaGetRawCurveKeySlopeOutCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetRawCurveKeySlopeOutCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetRawCurveKeySlopeOut(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetRawCurveKeySlopeOutCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns an array of key values for the specified node in the specified 
// group and animation.  The funtion evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function should be used in
// conjunction with GetBakedCurveKeyTimes. The function returns an empty 
// array if the node or animation can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetBakedCurveKeyValuesCommand::FxMayaGetBakedCurveKeyValuesCommand()
{
}

// Destructor.
FxMayaGetBakedCurveKeyValuesCommand::~FxMayaGetBakedCurveKeyValuesCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetBakedCurveKeyValuesCommand::creator( void )
{
	return new FxMayaGetBakedCurveKeyValuesCommand();
}

// The command syntax.
MSyntax FxMayaGetBakedCurveKeyValuesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetBakedCurveKeyValuesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetBakedCurveKeyValues(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetBakedCurveKeyValuesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}


//------------------------------------------------------------------------------
// Returns an array of key times for the specified node in the specified 
// group and animation.  The funtion evaluates the face graph at 60fps and
// returns time values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function should be used in
// conjunction with GetBakedCurveKeyValues. The function returns an empty 
// array if the node or animation can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetBakedCurveKeyTimesCommand::FxMayaGetBakedCurveKeyTimesCommand()
{
}

// Destructor.
FxMayaGetBakedCurveKeyTimesCommand::~FxMayaGetBakedCurveKeyTimesCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetBakedCurveKeyTimesCommand::creator( void )
{
	return new FxMayaGetBakedCurveKeyTimesCommand();
}

// The command syntax.
MSyntax FxMayaGetBakedCurveKeyTimesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetBakedCurveKeyTimesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetBakedCurveKeyTimes(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetBakedCurveKeyTimesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns an array of key input slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to constuct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetBakedCurveKeySlopeInCommand::FxMayaGetBakedCurveKeySlopeInCommand()
{
}

// Destructor.
FxMayaGetBakedCurveKeySlopeInCommand::~FxMayaGetBakedCurveKeySlopeInCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetBakedCurveKeySlopeInCommand::creator( void )
{
	return new FxMayaGetBakedCurveKeySlopeInCommand();
}

// The command syntax.
MSyntax FxMayaGetBakedCurveKeySlopeInCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetBakedCurveKeySlopeInCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetBakedCurveKeySlopeIn(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};


// Parse the command arguments.
MStatus	FxMayaGetBakedCurveKeySlopeInCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns an array of key output slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to constuct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetBakedCurveKeySlopeOutCommand::FxMayaGetBakedCurveKeySlopeOutCommand()
{
}
// Destructor.
FxMayaGetBakedCurveKeySlopeOutCommand::~FxMayaGetBakedCurveKeySlopeOutCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetBakedCurveKeySlopeOutCommand::creator( void )
{
	return new FxMayaGetBakedCurveKeySlopeOutCommand();
}

// The command syntax.
MSyntax FxMayaGetBakedCurveKeySlopeOutCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetBakedCurveKeySlopeOutCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	MDoubleArray curveData;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> FaceFxArray = 
		OC3Ent::Face::FxMayaData::mayaInterface.GetBakedCurveKeySlopeOut(_group.asChar(), _anim.asChar(), _curve.asChar());
	for( OC3Ent::Face::FxSize i = 0; i < FaceFxArray.Length(); ++i )
	{
		curveData.append(FaceFxArray[i]);
	}
	setResult(curveData);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetBakedCurveKeySlopeOutCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns the specified animation length in seconds
//------------------------------------------------------------------------------

// Constructor.
FxMayaGetAnimDurationCommand::FxMayaGetAnimDurationCommand()
{
}

// Destructor.
FxMayaGetAnimDurationCommand::~FxMayaGetAnimDurationCommand()
{
}
// Create a new instance of this command.
void* FxMayaGetAnimDurationCommand::creator( void )
{
	return new FxMayaGetAnimDurationCommand();
}

// The command syntax.
MSyntax FxMayaGetAnimDurationCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaGetAnimDurationCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxReal duration = OC3Ent::Face::FxMayaData::mayaInterface.GetAnimDuration(_group.asChar(), _anim.asChar());
	setResult(duration);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaGetAnimDurationCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns if the plugin is a mod developer plugin
//------------------------------------------------------------------------------

// Constructor.
FxMayaIsModDevloperCommand::FxMayaIsModDevloperCommand()
{
}

// Destructor.
FxMayaIsModDevloperCommand::~FxMayaIsModDevloperCommand()
{
}
// Create a new instance of this command.
void* FxMayaIsModDevloperCommand::creator( void )
{
	return new FxMayaIsModDevloperCommand();
}

// The command syntax.
MSyntax FxMayaIsModDevloperCommand::newSyntax( void )
{
	MSyntax syntax;
	return syntax;
}

// Execute the command.
MStatus	FxMayaIsModDevloperCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	bool answer = OC3Ent::Face::FxMayaData::mayaInterface.IsModDeveloper();
	setResult(answer);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaIsModDevloperCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// Returns if the plugin is a no-save plugin
//------------------------------------------------------------------------------

// Constructor.
FxMayaIsNoSaveCommand::FxMayaIsNoSaveCommand()
{
}

// Destructor.
FxMayaIsNoSaveCommand::~FxMayaIsNoSaveCommand()
{
}
// Create a new instance of this command.
void* FxMayaIsNoSaveCommand::creator( void )
{
	return new FxMayaIsNoSaveCommand();
}

// The command syntax.
MSyntax FxMayaIsNoSaveCommand::newSyntax( void )
{
	MSyntax syntax;
	return syntax;
}

// Execute the command.
MStatus	FxMayaIsNoSaveCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	bool answer = OC3Ent::Face::FxMayaData::mayaInterface.IsNoSave();
	setResult(answer);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus	FxMayaIsNoSaveCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	return MS::kSuccess;
}

