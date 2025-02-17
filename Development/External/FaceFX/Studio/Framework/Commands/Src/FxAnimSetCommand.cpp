//------------------------------------------------------------------------------
// The animSet command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAnimSetCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxAnimSetCommand, 0, FxCommand);

FxAnimSetCommand::FxAnimSetCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxAnimSetCommand::~FxAnimSetCommand()
{
}

FxCommandSyntax FxAnimSetCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-m", "-mount", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-u", "-unmount", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-i", "-import", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-e", "-export", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-to", "-to", CAT_String));
	return newSyntax;
}

FxCommandError FxAnimSetCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString animSetToMount;
	if( argList.GetArgument("-mount", animSetToMount) )
	{
		if( FxSessionProxy::MountAnimSet(animSetToMount) )
		{
			return CE_Success;
		}
	}
	FxString animSetToUnmount;
	if( argList.GetArgument("-unmount", animSetToUnmount) )
	{
		if( FxSessionProxy::UnmountAnimSet(animSetToUnmount.GetData()) )
		{
			return CE_Success;
		}
	}
	FxString animSetToImport;
	if( argList.GetArgument("-import", animSetToImport) )
	{
		if( FxSessionProxy::ImportAnimSet(animSetToImport) )
		{
			return CE_Success;
		}
	}
	FxString animGroupToExport;
	if( argList.GetArgument("-export", animGroupToExport) )
	{
		FxString fileToExportTo;
		if( argList.GetArgument("-to", fileToExportTo) )
		{
			if( FxSessionProxy::ExportAnimSet(animGroupToExport.GetData(), fileToExportTo) )
			{
				return CE_Success;
			}
		}
		else
		{
			FxVM::DisplayError("-to must be specified!");
		}
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
