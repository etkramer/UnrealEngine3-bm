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

XSIPLUGINCALLBACK CStatus FaceFXImportRefBonePose_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.AddWithHandler( L"frame",siArgHandlerFrame);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXImportRefBonePose_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	int frame = (int)args[0];
	OC3Ent::Face::FxXSIData::xsiInterface.ImportRefPose(frame);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXExportRefBonePose_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.AddWithHandler( L"frame",siArgHandlerFrame);
	args.AddWithHandler( L"bones",siArgHandlerCollection);


	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXExportRefBonePose_Execute( CRef& in_context )
{	
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	int frame = (int)args[0];
	CRefArray refArray = (CRefArray)args[1];
	OC3Ent::Face::FxArray<OC3Ent::Face::FxString> bones;
	for(int i = 0; i < refArray.GetCount(); ++i)
	{	
		ProjectItem projItem(refArray[i]);
		CString fullName(projItem.GetName());
		bones.PushBack(OC3Ent::Face::FxString(fullName.GetAsciiString()));
	}
	OC3Ent::Face::FxXSIData::xsiInterface.ExportRefPose(frame,bones);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXListRefBonePoses_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;
	cmd.EnableReturnValue(true);
	cmd.SetFlag(siNoLogging,true);


	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXListRefBonePoses_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	
	CValueArray objectNames;
	OC3Ent::Face::FxSize numRefBones = OC3Ent::Face::FxXSIData::xsiInterface.GetNumRefBones();
	for( OC3Ent::Face::FxSize i = 0; i < numRefBones; ++i )
	{

		XSI::CString tmpStr;
		tmpStr.PutAsciiString(OC3Ent::Face::FxXSIData::xsiInterface.GetRefBoneName(i).GetData());
		objectNames.Add(CValue(tmpStr));
	}
	CValue retValue(objectNames);
	ctxt.PutAttribute(L"ReturnValue",retValue);
	return CStatus::OK;
}



