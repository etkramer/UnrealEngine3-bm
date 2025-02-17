//------------------------------------------------------------------------------
// A MEL command to export the reference pose for a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include "FxMayaExportRefPoseCommand.h"

// Constructor.
FxMayaExportRefPoseCommand::FxMayaExportRefPoseCommand()
{
}

// Destructor.
FxMayaExportRefPoseCommand::~FxMayaExportRefPoseCommand()
{
}

// Create a new instance of this command.
void* FxMayaExportRefPoseCommand::creator( void )
{
	return new FxMayaExportRefPoseCommand();
}

// The command syntax.
MSyntax FxMayaExportRefPoseCommand::newSyntax( void )
{
	MSyntax syntax;
	syntax.addFlag(kFrameFlag, kFrameFlagLong, MSyntax::MArgType::kLong);
	syntax.addFlag(kBoneListFlag, kBoneListFlagLong, MSyntax::MArgType::kString);
	return syntax;
}

// Execute the command.
MStatus	FxMayaExportRefPoseCommand::doIt( const MArgList& args )
{
	MStatus status = _parseArgs(args);
	if( !status )
		return status;

	// Put all of the bone names from _bones (which were separated by spaces)
	// into the string array _boneList.
	_bones.split(' ', _boneList);
	OC3Ent::Face::FxArray<OC3Ent::Face::FxString> bones;
	for( OC3Ent::Face::FxSize i = 0; i < _boneList.length(); ++i )
	{
		OC3Ent::Face::FxString boneName(_boneList[i].asChar());
		bones.PushBack(boneName);
	}

	OC3Ent::Face::FxMayaData::mayaInterface.ExportRefPose(_frame, bones);	
	
	return MS::kSuccess;
};

// Parse the command arguments.
MStatus FxMayaExportRefPoseCommand::_parseArgs( const MArgList& args )
{
	MStatus status;
	MArgDatabase argData(syntax(), args);
	argData.getFlagArgument(kFrameFlag, 0, _frame);
	argData.getFlagArgument(kBoneListFlag, 0, _bones);
	return MS::kSuccess;
}
