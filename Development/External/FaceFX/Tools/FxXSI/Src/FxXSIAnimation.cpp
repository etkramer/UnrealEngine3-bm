#include <xsi_ref.h>
#include <xsi_value.h>
#include <xsi_status.h>
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_decl.h>
#include <xsi_plugin.h>
#include <xsi_pluginitem.h>
#include <xsi_command.h>
#include <xsi_argument.h>
#include "FxXSIHelper.h"
#include "FxXSIData.h"
#include <xsi_value.h>
#include <xsi_string.h>
#include <xsi_projectitem.h>
using namespace XSI;
//

XSIPLUGINCALLBACK CStatus FaceFXImportAnimation_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;
	ArgumentArray args = cmd.GetArguments();
	args.Add( L"animationgroupname");
	args.Add( L"animationname");
	args.Add( L"framerate",30.0f);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXImportAnimation_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);	
	CValueArray argValue = (CValueArray)ctxt.GetAttribute(L"Arguments");
	CString animationGroupName = (CString)argValue[0];
	OC3Ent::Face::FxString animationGroupNameStr(animationGroupName.GetAsciiString());

	CString animationName = (CString)argValue[1];
	OC3Ent::Face::FxString animationNameStr(animationName.GetAsciiString());

	float frameRate = (float)argValue[2];

	OC3Ent::Face::FxXSIData::xsiInterface.ImportAnim(animationGroupNameStr, animationNameStr,frameRate);

	return CStatus::OK;
}


XSIPLUGINCALLBACK CStatus FaceFXListAnimationGroups_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;	
	cmd.EnableReturnValue(true);
	cmd.SetFlag(siNoLogging,true);


	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXListAnimationGroups_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);	
	CValueArray animGroupNames;
	OC3Ent::Face::FxSize numAnimGroups = OC3Ent::Face::FxXSIData::xsiInterface.GetNumAnimGroups();
	for( OC3Ent::Face::FxSize i = 0; i < numAnimGroups; ++i )
	{

		CString strTemp;
		strTemp.PutAsciiString(OC3Ent::Face::FxXSIData::xsiInterface.GetAnimGroupName(i).GetData());
		animGroupNames.Add(CValue(strTemp));
	}
	CValue retValue(animGroupNames);
	ctxt.PutAttribute(L"ReturnValue",retValue);
	return CStatus::OK;
}


XSIPLUGINCALLBACK CStatus FaceFXListAnimationsFromGroup_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;
	ArgumentArray args = cmd.GetArguments();
	args.Add( L"animationgroupname");
	cmd.EnableReturnValue(true);
	cmd.SetFlag(siNoLogging,true);


	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXListAnimationsFromGroup_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray argValue = (CValueArray)ctxt.GetAttribute(L"Arguments");
	CString animationGroupName = (CString)argValue[0];
	OC3Ent::Face::FxString animationGroupNameStr(animationGroupName.GetAsciiString());
	CValueArray animationNames;
	OC3Ent::Face::FxSize numAnimations = OC3Ent::Face::FxXSIData::xsiInterface.GetNumAnims(animationGroupNameStr);
	for( OC3Ent::Face::FxSize i = 0; i < numAnimations; ++i )
	{
		CString strTemp;
		strTemp.PutAsciiString(OC3Ent::Face::FxXSIData::xsiInterface.GetAnimName(animationGroupNameStr,i).GetData());
		animationNames.Add(CValue(strTemp));
	}
	CValue retValue(animationNames);
	ctxt.PutAttribute(L"ReturnValue",retValue);
	return CStatus::OK;
}



