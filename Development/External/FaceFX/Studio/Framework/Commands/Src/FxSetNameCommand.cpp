//------------------------------------------------------------------------------
// The set name command, for renaming specific objects in an actor.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxSetNameCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxSetNameCommand, 0, FxCommand);

FxSetNameCommand::FxSetNameCommand()
{
	_isUndoable = FxTrue;
}

FxSetNameCommand::~FxSetNameCommand()
{
}

FxCommandSyntax FxSetNameCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-fg", "-facegraphnode", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ag", "-animgroup", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-a",  "-anim", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ac", "-animcurve", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-p",  "-path", CAT_StringArray));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-o",  "-old", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n",  "-new", CAT_String));
	return newSyntax;
}

FxCommandError FxSetNameCommand::Execute(const FxCommandArgumentList& argList)
{
	// Retrieve the values of the flags.
	_faceGraphNode = argList.GetArgument("-facegraphnode");
	_animGroup     = argList.GetArgument("-animgroup");
	_anim          = argList.GetArgument("-anim");
	_animCurve     = argList.GetArgument("-animcurve");
	FxSize numOps  = _faceGraphNode ? 1 : 0 + 
					 _animGroup     ? 1 : 0 + 
					 _anim          ? 1 : 0 + 
					 _animCurve     ? 1 : 0;
	if( numOps > 1 )
	{
		FxVM::DisplayError("Multiple operations were specified!");
		return CE_Failure;
	}
	if( numOps < 1 )
	{
		FxVM::DisplayError("No operations were specified. Specify one of [-facegraphnode, -animgroup, -anim, -animcurve]");
		return CE_Failure;
	}
	if( !argList.GetArgument("-old", _oldName) )
	{
		FxVM::DisplayError("The current name (-old) was not specified.");
		return CE_Failure;
	}
	if( !argList.GetArgument("-new", _newName) )
	{
		FxVM::DisplayError("The new name (-new) was not specified.");
		return CE_Failure;
	}
	
	if( !argList.GetArgument("-path", _path) && (_anim || _animCurve) )
	{
		FxVM::DisplayError("The path (-path) was not specified.");
		return CE_Failure;
	}
	else
	{
		if( _anim && _path.Length() != 1 )
		{
			FxVM::DisplayError("Incorrect path length. Should be -path \"animGroup\"");
			return CE_Failure;
		}
		if( _animCurve && _path.Length() != 2 )
		{
			FxVM::DisplayError("Incorrect path length. Should be -path \"animGroup|anim\"");
			return CE_Failure;
		}
	}

	return Redo();
}

FxCommandError FxSetNameCommand::Undo()
{
	if( _faceGraphNode )
	{
		if( FxSessionProxy::SetFaceGraphNodeName(_newName.GetData(), _oldName.GetData()) )
		{
			return CE_Success;
		}
	}
	else if( _animGroup )
	{
	}
	else if( _anim )
	{
	}
	else if( _animCurve )
	{
	}
	return CE_Failure;
}

FxCommandError FxSetNameCommand::Redo()
{
	if( _faceGraphNode )
	{
		FxFaceGraphNode* pNode = NULL;
		if( FxSessionProxy::GetFaceGraphNode(_newName.GetData(), &pNode) )
		{
			FxVM::DisplayError("A node of that name already exists in the face graph.");
			return CE_Failure;
		}

		if( FxSessionProxy::SetFaceGraphNodeName(_oldName.GetData(), _newName.GetData()) )
		{
			return CE_Success;
		}
	}
	else if( _animGroup )
	{
	}
	else if( _anim )
	{
	}
	else if( _animCurve )
	{
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
