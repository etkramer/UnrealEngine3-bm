//------------------------------------------------------------------------------
// The rename command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxRenameCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxName.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxRenameCommand, 0, FxCommand);

FxRenameCommand::FxRenameCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxRenameCommand::~FxRenameCommand()
{
}

FxCommandSyntax FxRenameCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-t", "-to", CAT_String));
	return newSyntax;
}

FxCommandError FxRenameCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString previousName;
	if( !argList.GetArgument("-name", previousName) )
	{
		FxVM::DisplayError("-name was not specified!");
		return CE_Failure;
	}
	FxString requestedName;
	if( !argList.GetArgument("-to", requestedName) )
	{
		FxVM::DisplayError("-to was not specified!");
		return CE_Failure;
	}
	if( previousName == requestedName )
	{
		FxVM::DisplayError("-name is the same as -to!");
		return CE_Failure;
	}
	else
	{
		if( FxInvalidIndex != FxName::Find(previousName.GetData()) )
		{
			// Make sure the operation will not result in duplicate names in the
			// face graph.
			FxFaceGraphNode* pNode1 = NULL;
			FxSessionProxy::GetFaceGraphNode(previousName, &pNode1);
			if( pNode1 )
			{
				FxFaceGraphNode* pNode2 = NULL;
				FxSessionProxy::GetFaceGraphNode(requestedName, &pNode2);
				if( pNode2 )
				{
					FxVM::DisplayError("operation would result in duplicate nodes in the face graph!");
					return CE_Failure;
				}
			}

			FxName prevName(previousName.GetData());
			if( prevName == FxName::NullName )
			{
				prevName.SetName(requestedName.GetData());
			}
			else
			{
				prevName.Rename(requestedName.GetData());
			}

			// Notify the face graph that something might have just rocked its
			// world.
			FxFaceGraph* pFaceGraph = NULL;
			if( FxSessionProxy::GetFaceGraph(pFaceGraph) && pFaceGraph )
			{
				pFaceGraph->NotifyRenamed(previousName.GetData(), requestedName.GetData());
			}

			// If there is more than one reference to the previous name 
			// (more than one because obviously there is one right above
			// this code, i.e. prevName), then the rename operation in the 
			// SDK failed.  Most likely the user was trying to do something
			// stupid like renaming the null name, a class name, or a link
			// function name.
			FxSize prevNameIndex = FxName::Find(previousName.GetData());
			if( FxInvalidIndex != prevNameIndex )
			{
				if( FxName::GetNumReferences(prevNameIndex) > 1 )
				{
					FxString errorMessage = "could not rename ";
					errorMessage = FxString::Concat(errorMessage, prevName.GetAsString());
					errorMessage = FxString::Concat(errorMessage, "!");
					FxVM::DisplayError(errorMessage);
					return CE_Failure;
				}
			}
			if( !FxSessionProxy::ActorDataChanged() )
			{
				FxVM::DisplayError("internal session error!");
				return CE_Failure;
			}
		}
		else
		{
			FxVM::DisplayError("name does not exist!");
			return CE_Failure;
		}
	}
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
