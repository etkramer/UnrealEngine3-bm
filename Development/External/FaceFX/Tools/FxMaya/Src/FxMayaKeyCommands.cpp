//------------------------------------------------------------------------------
// MEL commands to modify keys in a FaceFX curve.
//
// Owner: Doug Perkowski
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaKeyCommands.h"


//----------------------------------------------------
// Inserts a key into the specified curve.  
//----------------------------------------------------

// Constructor.
FxMayaInsertKeyCommand::FxMayaInsertKeyCommand()
{
}

// Destructor.
FxMayaInsertKeyCommand::~FxMayaInsertKeyCommand()
{
}

// Create a new instance of this command.
void* FxMayaInsertKeyCommand::creator( void )
{
	return new FxMayaInsertKeyCommand();
}

// The command syntax.
MSyntax FxMayaInsertKeyCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kFrameFlag, kFrameFlagLong, MSyntax::MArgType::kLong);
	syntax.addFlag(kValueFlag, kValueFlagLong, MSyntax::MArgType::kDouble);
	syntax.addFlag(kSlopeInFlag, kSlopeInFlagLong, MSyntax::MArgType::kDouble);
	syntax.addFlag(kSlopeOutFlag, kSlopeOutFlagLong, MSyntax::MArgType::kDouble);
	return syntax;
}

// Execute the command.
MStatus	FxMayaInsertKeyCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
	{
		// Fail if a time isn't provided or the command can't be parsed.
		setResult(-1);
		return MS::kSuccess;
	}

	OC3Ent::Face::FxSize index = 
		OC3Ent::Face::FxMayaData::mayaInterface.InsertKey(_group.asChar(), _anim.asChar(), _curve.asChar(), _time, _value, _slopeIn, _slopeOut);
	setResult(static_cast<int>(index));
	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaInsertKeyCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);

	double frame = 0;
	double value = 0;
	double slopeIn = 0;
	double slopeOut = 0;

	if(!argData.getFlagArgument(kFrameFlag, 0, frame))
	{
		return MS::kFailure;
	}
	argData.getFlagArgument(kValueFlag, 0, value);
	argData.getFlagArgument(kSlopeInFlag, 0, slopeIn);
	argData.getFlagArgument(kSlopeInFlag, 0, slopeOut);

	// Convert from frames to seconds.
	MTime mayaTime;
	mayaTime.setValue(frame);
	mayaTime.setUnit(MTime::kSeconds);
	_time = static_cast<OC3Ent::Face::FxReal>(mayaTime.value());

	_value = static_cast<OC3Ent::Face::FxReal>(value);
	_slopeIn = static_cast<OC3Ent::Face::FxReal>(slopeIn);
	_slopeOut = static_cast<OC3Ent::Face::FxReal>(slopeOut);
	return MS::kSuccess;
}


//----------------------------------------------------
// Deletes all keys in the specified curve.  
//----------------------------------------------------

// Constructor.
FxMayaDeleteAllKeysCommand::FxMayaDeleteAllKeysCommand()
{
}

// Destructor.
FxMayaDeleteAllKeysCommand::~FxMayaDeleteAllKeysCommand()
{
}

// Create a new instance of this command.
void* FxMayaDeleteAllKeysCommand::creator( void )
{
	return new FxMayaDeleteAllKeysCommand();
}

// The command syntax.
MSyntax FxMayaDeleteAllKeysCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kGroupFlag, kGroupFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kAnimFlag, kAnimFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaDeleteAllKeysCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxBool returnValue = 
		OC3Ent::Face::FxMayaData::mayaInterface.DeleteAllKeys(_group.asChar(), _anim.asChar(), _curve.asChar());
	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaDeleteAllKeysCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kGroupFlag, 0, _group);
	argData.getFlagArgument(kAnimFlag, 0, _anim);
	argData.getFlagArgument(kCurveFlag, 0, _curve);
	return MS::kSuccess;
}
