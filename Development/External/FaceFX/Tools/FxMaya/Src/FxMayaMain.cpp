//------------------------------------------------------------------------------
// The main module for the FaceFx Maya plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxMayaData.h"
#include <maya/MFnPlugin.h>
#include <maya/MSceneMessage.h>
#include <maya/MGlobal.h>

#include "FxMayaLogToOutputWindowCommand.h"
#include "FxMayaCreateActorCommand.h"
#include "FxMayaLoadActorCommand.h"
#include "FxMayaSaveActorCommand.h"
#include "FxMayaExportRefPoseCommand.h"
#include "FxMayaImportRefPoseCommand.h"
#include "FxMayaExportBonePoseCommand.h"
#include "FxMayaImportBonePoseCommand.h"
#include "FxMayaImportAnimCommand.h"
#include "FxMayaCleanCommand.h"
#include "FxMayaQueryCommands.h"
#include "FxMayaDisplayWarningCommand.h"
#include "FxMayaKeyCommands.h"


// The exit callback function id.
static MCallbackId mayaExitCallbackId = 0;

// The exit callback function.  This is called when Maya exits because Maya does
// not call the uninitializePlugin function when exiting.
FX_PLUGIN_API void OnMayaExit( void* clientData )
{
	// Shutdown the Maya interface.
	OC3Ent::Face::FxMayaData::mayaInterface.Shutdown();
}

FX_PLUGIN_API MStatus initializePlugin( MObject obj )
{ 
	// Initialize the plug-in.
	MStatus status;
	MFnPlugin plugin(obj, "OC3 Entertainment, Inc.", kCurrentFxMayaVersion, "Any");

	// Startup the Maya interface.
	OC3Ent::Face::FxMayaData::mayaInterface.Startup();

	// Only initialize the logging system if it hasn't been initialized yet.
	if( 0 == OC3Ent::Face::FxToolLogFile::GetPath().Length() )
	{
		OC3Ent::Face::FxToolLogFile::SetEchoToCout(FxTrue);
#ifdef WIN32
		OC3Ent::Face::FxChar* mayaRootDirectory = getenv("MAYA_APP_DIR");
		if( mayaRootDirectory )
		{
			OC3Ent::Face::FxString logPath = mayaRootDirectory;
			logPath = OC3Ent::Face::FxString::Concat(logPath, "/FaceFX_Log.txt");
			OC3Ent::Face::FxToolLogFile::SetPath(logPath);
		}
		else
		{
			OC3Ent::Face::FxString logPath = "C:/FaceFX_Log.txt";
			OC3Ent::Face::FxToolLogFile::SetPath(logPath);
		}
		
#endif
	}

	// Register FaceFX MEL commands.
	status = plugin.registerCommand("fxoutputwindowlog",
		FxMayaLogToOutputWindowCommand::creator,
		FxMayaLogToOutputWindowCommand::newSyntax);
	status = plugin.registerCommand("fxgetactorname",
		FxMayaGetActorNameCommand::creator,
		FxMayaGetActorNameCommand::newSyntax);
	status = plugin.registerCommand("fxcreateactor",
		FxMayaCreateActorCommand::creator,
		FxMayaCreateActorCommand::newSyntax);
	status = plugin.registerCommand("fxloadactor", 
		FxMayaLoadActorCommand::creator,
		FxMayaLoadActorCommand::newSyntax);
	status = plugin.registerCommand("fxsaveactor", 
		FxMayaSaveActorCommand::creator,
		FxMayaSaveActorCommand::newSyntax);
	status = plugin.registerCommand("fxexportrefpose",
		FxMayaExportRefPoseCommand::creator,
		FxMayaExportRefPoseCommand::newSyntax);
	status = plugin.registerCommand("fximportrefpose",
		FxMayaImportRefPoseCommand::creator,
		FxMayaImportRefPoseCommand::newSyntax);
	status = plugin.registerCommand("fxexportbonepose",
		FxMayaExportBonePoseCommand::creator,
		FxMayaExportBonePoseCommand::newSyntax);
	status = plugin.registerCommand("fxbatchexportboneposes",
		FxMayaBatchExportBonePosesCommand::creator,
		FxMayaBatchExportBonePosesCommand::newSyntax);
	status = plugin.registerCommand("fximportbonepose",
		FxMayaImportBonePoseCommand::creator,
		FxMayaImportBonePoseCommand::newSyntax);
	status = plugin.registerCommand("fxbatchimportboneposes",
		FxMayaBatchImportBonePosesCommand::creator,
		FxMayaBatchImportBonePosesCommand::newSyntax);
	status = plugin.registerCommand("fximportanim",
		FxMayaImportAnimCommand::creator,
		FxMayaImportAnimCommand::newSyntax);
	status = plugin.registerCommand("fxclean",
		FxMayaCleanCommand::creator,
		FxMayaCleanCommand::newSyntax);

	status = plugin.registerCommand("fxgetanimgroups",
		FxMayaGetAnimGroupsCommand::creator,
		FxMayaGetAnimGroupsCommand::newSyntax);
	status = plugin.registerCommand("fxgetanims",
		FxMayaGetAnimsCommand::creator,
		FxMayaGetAnimsCommand::newSyntax);
	status = plugin.registerCommand("fxgetcurves",
		FxMayaGetCurvesCommand::creator,
		FxMayaGetCurvesCommand::newSyntax);
	status = plugin.registerCommand("fxgetnodes",
		FxMayaGetNodesCommand::creator,
		FxMayaGetNodesCommand::newSyntax);
	status = plugin.registerCommand("fxgetrefbones",
		FxMayaGetRefBonesCommand::creator,
		FxMayaGetRefBonesCommand::newSyntax);
	status = plugin.registerCommand("fxgetbones",
		FxMayaGetBonesCommand::creator,
		FxMayaGetBonesCommand::newSyntax);
	status = plugin.registerCommand("fxgetrawcurvekeyvalues",
		FxMayaGetRawCurveKeyValuesCommand::creator,
		FxMayaGetRawCurveKeyValuesCommand::newSyntax);
	status = plugin.registerCommand("fxgetrawcurvekeytimes",
		FxMayaGetRawCurveKeyTimesCommand::creator,
		FxMayaGetRawCurveKeyTimesCommand::newSyntax);
	status = plugin.registerCommand("fxgetrawcurvekeyslopein",
		FxMayaGetRawCurveKeySlopeInCommand::creator,
		FxMayaGetRawCurveKeySlopeInCommand::newSyntax);
	status = plugin.registerCommand("fxgetrawcurvekeyslopeout",
		FxMayaGetRawCurveKeySlopeOutCommand::creator,
		FxMayaGetRawCurveKeySlopeOutCommand::newSyntax);
	status = plugin.registerCommand("fxgetbakedcurvekeyvalues",
		FxMayaGetBakedCurveKeyValuesCommand::creator,
		FxMayaGetBakedCurveKeyValuesCommand::newSyntax);
	status = plugin.registerCommand("fxgetbakedcurvekeytimes",
		FxMayaGetBakedCurveKeyTimesCommand::creator,
		FxMayaGetBakedCurveKeyTimesCommand::newSyntax);
	status = plugin.registerCommand("fxgetbakedcurvekeyslopein",
		FxMayaGetBakedCurveKeySlopeInCommand::creator,
		FxMayaGetBakedCurveKeySlopeInCommand::newSyntax);
	status = plugin.registerCommand("fxgetbakedcurvekeyslopeout",
		FxMayaGetBakedCurveKeySlopeOutCommand::creator,
		FxMayaGetBakedCurveKeySlopeOutCommand::newSyntax);
	status = plugin.registerCommand("fxgetanimduration",
		FxMayaGetAnimDurationCommand::creator,
		FxMayaGetAnimDurationCommand::newSyntax);
	status = plugin.registerCommand("fxinsertkey",
		FxMayaInsertKeyCommand::creator,
		FxMayaInsertKeyCommand::newSyntax);
	status = plugin.registerCommand("fxdeleteallkeys",
		FxMayaDeleteAllKeysCommand::creator,
		FxMayaDeleteAllKeysCommand::newSyntax);
	status = plugin.registerCommand("fxismoddeveloper",
		FxMayaIsModDevloperCommand::creator,
		FxMayaIsModDevloperCommand::newSyntax);
	status = plugin.registerCommand("fxisnosave",
		FxMayaIsNoSaveCommand::creator,
		FxMayaIsNoSaveCommand::newSyntax);

	status = plugin.registerCommand("fxsetdisplaywarningdialogs",
		FxMayaDisplayWarningCommand::creator,
		FxMayaDisplayWarningCommand::newSyntax);

	// Register a callback to be executed when Maya exits because Maya does not
	// call uninitializePlugin in that case.
	mayaExitCallbackId = MSceneMessage::addCallback(MSceneMessage::kMayaExiting, OnMayaExit);

	FxToolLog("Loaded FaceFX for Maya %s", kCurrentFxMayaVersion);
	FxToolLog("Running under Maya %s", MGlobal::mayaVersion().asChar());

	MGlobal::MMayaState mode = MGlobal::mayaState();
	switch( mode )
	{
	case MGlobal::kInteractive:
		FxToolLog("normal mode");
		break;
	case MGlobal::kBatch:
		FxToolLog("batch mode");
		break;
	case MGlobal::kLibraryApp:
		FxToolLog("standalone library mode");
		break;
	case MGlobal::kBaseUIMode:
		FxToolLog("base ui mode");
		break;
	default:
		FxToolLog("unknown mode");
		break;
	}
	
	return status;
}

FX_PLUGIN_API MStatus uninitializePlugin( MObject obj )
{
	// De-register the exit callback.
	MSceneMessage::removeCallback(mayaExitCallbackId);
	
	// Un-initialize the plug-in.
	MStatus   status;
	MFnPlugin plugin(obj);

	// Un-register FaceFX MEL commands.
	status = plugin.deregisterCommand("fxoutputwindowlog");
	status = plugin.deregisterCommand("fxgetactorname");
	status = plugin.deregisterCommand("fxcreateactor");
	status = plugin.deregisterCommand("fxloadactor");
	status = plugin.deregisterCommand("fxsaveactor");
	status = plugin.deregisterCommand("fxexportrefpose");
	status = plugin.deregisterCommand("fximportrefpose");
	status = plugin.deregisterCommand("fxexportbonepose");
	status = plugin.deregisterCommand("fxbatchexportboneposes");
	status = plugin.deregisterCommand("fximportbonepose");
	status = plugin.deregisterCommand("fxbatchimportboneposes");
	status = plugin.deregisterCommand("fximportanim");
	status = plugin.deregisterCommand("fxclean");

	status = plugin.deregisterCommand("fxgetanimgroups");
	status = plugin.deregisterCommand("fxgetanims");
	status = plugin.deregisterCommand("fxgetcurves");
	status = plugin.deregisterCommand("fxgetnodes");
	status = plugin.deregisterCommand("fxgetrefbones");
	status = plugin.deregisterCommand("fxgetbones");

	status = plugin.deregisterCommand("fxgetrawcurvekeyvalues");
	status = plugin.deregisterCommand("fxgetrawcurvekeytimes");
	status = plugin.deregisterCommand("fxgetrawcurvekeyslopein");
	status = plugin.deregisterCommand("fxgetrawcurvekeyslopeout");
	status = plugin.deregisterCommand("fxgetbakedcurvekeyvalues");
	status = plugin.deregisterCommand("fxgetbakedcurvekeytimes");
	status = plugin.deregisterCommand("fxgetanimduration");
	status = plugin.deregisterCommand("fxinsertkey");
	status = plugin.deregisterCommand("fxdeleteallkeys");
	status = plugin.deregisterCommand("fxismoddeveloper");
	status = plugin.deregisterCommand("fxisnosave");	

	status = plugin.deregisterCommand("fxsetdisplaywarningdialogs");

	// Shutdown the Maya interface.
	OC3Ent::Face::FxMayaData::mayaInterface.Shutdown();

	FxToolLog("Unloaded FaceFX for Maya %s", kCurrentFxMayaVersion);

	return status;
}
