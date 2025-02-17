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
#include "FxToolLog.h"
#include <xsi_value.h>
#include <xsi_string.h>
#include <xsi_projectitem.h>
using namespace XSI;
//

XSIPLUGINCALLBACK CStatus FaceFXImportBonePose_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"posename");
	args.AddWithHandler( L"frame",siArgHandlerFrame);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXImportBonePose_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	CString poseName = (CString)args[0];
	int frame = (int)args[1];
	OC3Ent::Face::FxXSIData::xsiInterface.ImportBonePose(OC3Ent::Face::FxString(poseName.GetAsciiString()),frame);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXExportBonePose_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.AddWithHandler( L"frame",siArgHandlerFrame);
	args.Add( L"poseName");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXExportBonePose_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	int frame = (int)args[0];
	CString poseName = (CString)args[1];
	OC3Ent::Face::FxXSIData::xsiInterface.ExportBonePose(frame,OC3Ent::Face::FxString(poseName.GetAsciiString()));
	return CStatus::OK;
}

/* D.P. 02/27/2008 - Removed Delete Bone Pose functions.
This operation must be done in FaceFX Studio to properly delete session data for links and the node.
XSIPLUGINCALLBACK CStatus FaceFXDeleteBonePose_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.Add( L"posename");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXDeleteBonePose_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	CString poseName = (CString)args[0];
	OC3Ent::Face::FxXSIData::xsiInterface.RemoveBonePose(OC3Ent::Face::FxString(poseName.GetAsciiString()));

	return CStatus::OK;
}
*/

XSIPLUGINCALLBACK CStatus FaceFXBatchImportBonePoses_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"filename");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXBatchImportBonePoses_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	CString fileName = (CString)args[0];

	OC3Ent::Face::FxInt32 dummyFrame(0);
	OC3Ent::Face::FxXSIData::xsiInterface.BatchImportBonePoses(OC3Ent::Face::FxString(fileName.GetAsciiString()),dummyFrame,dummyFrame);

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXBatchExportBonePoses_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.Add( L"filename");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXBatchExportBonePoses_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	CString fileName = (CString)args[0];
	OC3Ent::Face::FxXSIData::xsiInterface.BatchExportBonePoses(OC3Ent::Face::FxString(fileName.GetAsciiString()));

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXListBonePoses_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;
	ArgumentArray args = cmd.GetArguments();
	cmd.EnableReturnValue(true);
	cmd.SetFlag(siNoLogging,true);


	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXListBonePoses_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute(L"Arguments");

	CValueArray poseNames;
	OC3Ent::Face::FxActor* actor = OC3Ent::Face::FxXSIData::xsiInterface.GetActor();
	if(actor)
	{
		OC3Ent::Face::FxFaceGraph::Iterator iter = actor->GetDecompiledFaceGraph().Begin(OC3Ent::Face::FxBonePoseNode::StaticClass());
		for(int currBonePose = 0; iter != actor->GetDecompiledFaceGraph().End(OC3Ent::Face::FxBonePoseNode::StaticClass()); ++iter,++currBonePose)
		{
			OC3Ent::Face::FxFaceGraphNode* graphNode = iter.GetNode();
			XSI::CString poseName;
			poseName.PutAsciiString(graphNode->GetNameAsCstr());
			poseNames.Add(CValue(poseName));
		}
	}
	

	CValue retValue(poseNames);
	ctxt.PutAttribute(L"ReturnValue",retValue);
	return CStatus::OK;
}


