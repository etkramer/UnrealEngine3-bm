//------------------------------------------------------------------------------
// A MEL command to import a bone pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaImportBonePoseCommand.h"

//------------------------------------------------------------------------------
// FxMayaImportBonePoseCommand.
//------------------------------------------------------------------------------

// Constructor.
FxMayaImportBonePoseCommand::FxMayaImportBonePoseCommand()
{
}

// Destructor.
FxMayaImportBonePoseCommand::~FxMayaImportBonePoseCommand()
{
}

// Create a new instance of this command.
void* FxMayaImportBonePoseCommand::creator( void )
{
	return new FxMayaImportBonePoseCommand();
}

// The command syntax.
MSyntax FxMayaImportBonePoseCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kFrameFlag, kFrameFlagLong, MSyntax::MArgType::kLong);
	syntax.addFlag(kNameFlag, kNameFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaImportBonePoseCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.ImportBonePose(OC3Ent::Face::FxString(_name.asChar()), _frame);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaImportBonePoseCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kFrameFlag, 0, _frame);
	argData.getFlagArgument(kNameFlag, 0, _name);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// FxMayaBatchImportBonePosesCommand.
//------------------------------------------------------------------------------

// Constructor.
FxMayaBatchImportBonePosesCommand::FxMayaBatchImportBonePosesCommand()
{
}

// Destructor.
FxMayaBatchImportBonePosesCommand::~FxMayaBatchImportBonePosesCommand()
{
}

// Create a new instance of this command.
void* FxMayaBatchImportBonePosesCommand::creator( void )
{
	return new FxMayaBatchImportBonePosesCommand();
}

// The command syntax.
MSyntax FxMayaBatchImportBonePosesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kFileFlag, kFileFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaBatchImportBonePosesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxInt32 startFrame = 0;
	OC3Ent::Face::FxInt32 endFrame   = 0;
	OC3Ent::Face::FxMayaData::mayaInterface.BatchImportBonePoses(
		OC3Ent::Face::FxString(_filename.asChar()), startFrame, endFrame);

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaBatchImportBonePosesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kFileFlag, 0, _filename);
	return MS::kSuccess;
}
