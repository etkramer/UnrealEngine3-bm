//------------------------------------------------------------------------------
// A MEL command to export a bone pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaExportBonePoseCommand.h"

//------------------------------------------------------------------------------
// FxMayaExportBonePoseCommand.
//------------------------------------------------------------------------------

// Constructor.
FxMayaExportBonePoseCommand::FxMayaExportBonePoseCommand()
{
}

// Destructor.
FxMayaExportBonePoseCommand::~FxMayaExportBonePoseCommand()
{
}

// Create a new instance of this command.
void* FxMayaExportBonePoseCommand::creator( void )
{
	return new FxMayaExportBonePoseCommand();
}

// The command syntax.
MSyntax FxMayaExportBonePoseCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kFrameFlag, kFrameFlagLong, MSyntax::MArgType::kLong);
	syntax.addFlag(kNameFlag, kNameFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaExportBonePoseCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.ExportBonePose(_frame, OC3Ent::Face::FxString(_name.asChar()));	

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaExportBonePoseCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kFrameFlag, 0, _frame);
	argData.getFlagArgument(kNameFlag, 0, _name);
	return MS::kSuccess;
}

//------------------------------------------------------------------------------
// FxMayaBatchExportBonePosesCommand.
//------------------------------------------------------------------------------

// Constructor.
FxMayaBatchExportBonePosesCommand::FxMayaBatchExportBonePosesCommand()
{
}

// Destructor.
FxMayaBatchExportBonePosesCommand::~FxMayaBatchExportBonePosesCommand()
{
}

// Create a new instance of this command.
void* FxMayaBatchExportBonePosesCommand::creator( void )
{
	return new FxMayaBatchExportBonePosesCommand();
}

// The command syntax.
MSyntax FxMayaBatchExportBonePosesCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kFileFlag, kFileFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaBatchExportBonePosesCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	OC3Ent::Face::FxMayaData::mayaInterface.BatchExportBonePoses(OC3Ent::Face::FxString(_filename.asChar()));

	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaBatchExportBonePosesCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kFileFlag, 0, _filename);
	return MS::kSuccess;
}
