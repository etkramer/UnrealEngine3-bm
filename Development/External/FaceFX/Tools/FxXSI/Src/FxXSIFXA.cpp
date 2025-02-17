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
#include "FxToolXSI.h"
#include <xsi_value.h>
#include <xsi_string.h>
using namespace XSI;
using namespace OC3Ent;
using namespace Face;


XSIPLUGINCALLBACK CStatus FaceFXOpenFXA_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.Add( L"FXAFilePath");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXOpenFXA_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	
	Application app;
	Context ctxt(in_context);
	CValueArray argValue = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	CString strFileValue((CString)argValue[0]);
	OC3Ent::Face::FxXSIData::xsiInterface.LoadActor(OC3Ent::Face::FxString(strFileValue.GetAsciiString()));

	return CStatus::OK;
}


XSIPLUGINCALLBACK CStatus FaceFXSaveFXA_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.Add( L"FXAFilePath");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXSaveFXA_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);	
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );
	CString strFileValue((CString)args[0]);
	OC3Ent::Face::FxXSIData::xsiInterface.SaveActor(OC3Ent::Face::FxString(strFileValue.GetAsciiString()));

	return CStatus::OK;
}


XSIPLUGINCALLBACK CStatus FaceFXNewFXA_Init( const CRef& in_context )
{
	Context ctx(in_context);
	Command cmd(ctx.GetSource());

	Application app;

	ArgumentArray args = cmd.GetArguments();

	args.Add( L"ActorName");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus FaceFXNewFXA_Execute( CRef& in_context )
{
	//Temp
	OC3Ent::Face::FxXSIData::xsiInterface.Startup();
	Application app;
	Context ctxt(in_context);
	CValueArray args = (CValueArray)ctxt.GetAttribute( L"Arguments" );

	CString strActorValue((CString)args[0]);

	OC3Ent::Face::FxXSIData::xsiInterface.CreateActor(OC3Ent::Face::FxString(strActorValue.GetAsciiString()));

	return CStatus::OK;
}

