#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include "FxXSIData.h"
#include "FxXSIHelper.h"
#include "FxToolLog.h"
//#include 
using namespace XSI; 


XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"mbarrett");
	in_reg.PutName(L"FxXSI");
	in_reg.PutEmail(L"");
	in_reg.PutURL(L"");
	in_reg.PutVersion(1,0);
	
	XSI::CString helpPath = in_reg.GetOriginPath();
	helpPath += L"FxXSI.htm";
	in_reg.PutHelp(helpPath);

	//File manipulation
	in_reg.RegisterCommand(L"FaceFXOpenFXA",L"FaceFXOpenFXA");
	in_reg.RegisterCommand(L"FaceFXSaveFXA",L"FaceFXSaveFXA");
	in_reg.RegisterCommand(L"FaceFXNewFXA",L"FaceFXNewFXA");

	//Reference pose
	in_reg.RegisterCommand(L"FaceFXImportRefBonePose",L"FaceFXImportRefBonePose");
	in_reg.RegisterCommand(L"FaceFXExportRefBonePose",L"FaceFXExportRefBonePose");
	in_reg.RegisterCommand(L"FaceFXListRefBonePoses",L"FaceFXListRefBonePoses");

	//Bone pose
	in_reg.RegisterCommand(L"FaceFXImportBonePose",L"FaceFXImportBonePose");
	in_reg.RegisterCommand(L"FaceFXExportBonePose",L"FaceFXExportBonePose");
	in_reg.RegisterCommand(L"FaceFXBatchImportBonePoses",L"FaceFXBatchImportBonePoses");
	in_reg.RegisterCommand(L"FaceFXBatchExportBonePoses",L"FaceFXBatchExportBonePoses");
	in_reg.RegisterCommand(L"FaceFXListBonePoses",L"FaceFXListBonePoses");

	//Animation
	in_reg.RegisterCommand(L"FaceFXImportAnimation",L"FaceFXImportAnimation");
	in_reg.RegisterCommand(L"FaceFXListAnimationGroups",L"FaceFXListAnimationGroups");
	in_reg.RegisterCommand(L"FaceFXListAnimationsFromGroup",L"FaceFXListAnimationsFromGroup");

	
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus XSIUnloadPlugin( const PluginRegistrar& FxUnused(in_reg) )
{
	OC3Ent::Face::FxXSIData::xsiInterface.Shutdown();
	return CStatus::OK;
}

