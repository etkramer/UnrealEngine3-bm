//------------------------------------------------------------------------------
// The select command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxSelectCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

#define kNodeType "node"
#define kLinkType "link"
#define kNodeGroupType "nodegroup"
#define kAnimGroupType "animgroup"
#define kAnimType "anim"
#define kCurveType "curve"

FX_IMPLEMENT_CLASS(FxSelectCommand, 0, FxCommand);

FxSelectCommand::FxSelectCommand()
{
	_isUndoable        = FxTrue;
	_clear             = FxFalse;
	_zoomToSelection   = FxTrue;
	_addToCurrent      = FxFalse;
	_removeFromCurrent = FxFalse;
}

FxSelectCommand::~FxSelectCommand()
{
}

FxCommandSyntax FxSelectCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-t", "-type", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-c", "-clear", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nz", "-nozoom", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-a", "-add", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-r", "-remove", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-names", CAT_StringArray));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-g", "-group", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n1", "-node1", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n2", "-node2", CAT_String));
	return newSyntax;
}

FxCommandError FxSelectCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-type", _type) )
	{
		if( argList.GetArgument("-clear") )
		{
			_clear = FxTrue;
		}
		if( argList.GetArgument("-nozoom") )
		{
			_zoomToSelection = FxFalse;
		}
		if( kNodeType == _type )
		{
			if( !FxSessionProxy::GetSelectedFaceGraphNodes(_prevSelection) )
			{
				return CE_Failure;
			}
		}
		else if( kLinkType == _type )
		{
			if( !FxSessionProxy::GetSelectedFaceGraphNodeLink(_prevNode1, _prevNode2) )
			{
				return CE_Failure;
			}
			if( !_clear && !argList.GetArgument("-node1", _node1) )
			{
				FxVM::DisplayError("-node1 was not specified!");
				return CE_Failure;
			}
			if( !_clear && !argList.GetArgument("-node2", _node2) )
			{
				FxVM::DisplayError("-node2 was not specified!");
				return CE_Failure;
			}
		}
		else if( kNodeGroupType == _type )
		{
			FxString nodeGroup;
			if( !FxSessionProxy::GetSelectedFaceGraphNodeGroup(nodeGroup) )
			{
				return CE_Failure;
			}
			_prevSelection.PushBack(nodeGroup);
		}
		else if( kAnimGroupType == _type )
		{
			FxString animGroup;
			if( !FxSessionProxy::GetSelectedAnimGroup(animGroup) )
			{
				return CE_Failure;
			}
			_prevSelection.PushBack(animGroup);

			if( !FxSessionProxy::GetSelectedAnim(_prevAnim) )
			{
				_prevAnim = "";
			}
			FxSessionProxy::GetSelectedAnimCurves(_prevCurves);
			FxSessionProxy::GetSelectedKeys(_curveIndices, _keyIndices);
        }
		else if( kAnimType == _type )
		{
			FxString anim;
			if( !FxSessionProxy::GetSelectedAnim(anim) )
			{
				return CE_Failure;
			}
			_prevSelection.PushBack(anim);

			FxSessionProxy::GetSelectedAnimCurves(_prevCurves);
			FxSessionProxy::GetSelectedKeys(_curveIndices, _keyIndices);
		}
		else if( kCurveType == _type )
		{
			if( !FxSessionProxy::GetSelectedAnimCurves(_prevSelection) )
			{
				return CE_Failure;
			}
		}
		else
		{
			FxVM::DisplayError("unknown object type!");
			return CE_Failure;
		}

		if( !_clear )
		{
			if( argList.GetArgument("-names", _objects) )
			{
				if( argList.GetArgument("-add") )
				{
					_addToCurrent = FxTrue;
				}
				else if( argList.GetArgument("-remove") )
				{
					_removeFromCurrent = FxTrue;
				}
				if( argList.GetArgument("-group", _group) )
				{
					if( kAnimType != _type )
					{
						FxVM::DisplayError("-group valid only for type \"anim\"");
						return CE_Failure;
					}
					if( !FxSessionProxy::GetSelectedAnimGroup(_prevGroup) )
					{
						return CE_Failure;
					}
				}
			}
			else
			{
				if( kLinkType != _type )
				{
					FxVM::DisplayError("-names was not specified!");
					return CE_Failure;
				}
			}
		}
		return Redo();
	}
	else
	{
		FxVM::DisplayError("-type was not specified!");
	}
	return CE_Failure;
}

FxCommandError FxSelectCommand::Undo( void )
{
	if( kNodeType == _type )
	{
		if( !FxSessionProxy::SetSelectedFaceGraphNodes(_prevSelection, _zoomToSelection) )
		{
			return CE_Failure;
		}
	}
	else if( kLinkType == _type )
	{
		FxArray<FxString> node1;
		node1.PushBack(_prevNode1);
		if( !FxSessionProxy::SetSelectedFaceGraphNodes(node1, _zoomToSelection) )
		{
			return CE_Failure;
		}
		if( !FxSessionProxy::SetSelectedFaceGraphNodeLink(_prevNode1, _prevNode2) )
		{
			return CE_Failure;
		}
	}
	else if( kNodeGroupType == _type )
	{
		if( _prevSelection.Length() > 0 )
		{
			if( !FxSessionProxy::SetSelectedFaceGraphNodeGroup(_prevSelection[0]) )
			{
				return CE_Failure;
			}
		}
		else
		{
			if( !FxSessionProxy::SetSelectedFaceGraphNodeGroup("") )
			{
				return CE_Failure;
			}
		}
	}
	else if( kAnimGroupType == _type )
	{
		if( _prevSelection.Length() > 0 )
		{
			if( !FxSessionProxy::SetSelectedAnimGroup(_prevSelection[0]) )
			{
				return CE_Failure;
			}
			if( _prevAnim.Length() > 0 )
			{
				FxSessionProxy::SetSelectedAnim(_prevAnim);
			}
			FxSessionProxy::SetSelectedAnimCurves(_prevCurves);
			FxSessionProxy::SetSelectedKeys(_curveIndices, _keyIndices);
		}
		else
		{
			if( !FxSessionProxy::SetSelectedAnimGroup("") )
			{
				return CE_Failure;
			}
		}
	}
	else if( kAnimType == _type )
	{
		if( _prevGroup.Length() > 0 )
		{
			if( !FxSessionProxy::SetSelectedAnimGroup(_prevGroup) )
			{
				return CE_Failure;
			}
		}
		if( _prevSelection.Length() > 0 )
		{
			if( !FxSessionProxy::SetSelectedAnim(_prevSelection[0]) )
			{
				return CE_Failure;
			}

			FxSessionProxy::SetSelectedAnimCurves(_prevCurves);
			FxSessionProxy::SetSelectedKeys(_curveIndices, _keyIndices);
		}
		else
		{
			if( !FxSessionProxy::SetSelectedAnim("") )
			{
				return CE_Failure;
			}
		}
	}
	else if( kCurveType == _type )
	{
		if( !FxSessionProxy::SetSelectedAnimCurves(_prevSelection) )
		{
			return CE_Failure;
		}
	}
	else
	{
		FxVM::DisplayError("unknown object type!");
		return CE_Failure;
	}
	return CE_Success;
}

FxCommandError FxSelectCommand::Redo( void )
{
	FxArray<FxString> newSelection;
	if( _clear )
	{
		_node1 = "";
		_node2 = "";
		_group = "";
		if( _type != kNodeType && _type != kCurveType )
		{
			newSelection.PushBack("");
		}
	}
	else
	{
		if( _addToCurrent )
		{
			newSelection = _prevSelection;
			for( FxSize i = 0; i < _objects.Length(); ++i )
			{
				newSelection.PushBack(_objects[i]);
			}
		}
		else if( _removeFromCurrent )
		{
			newSelection = _prevSelection;
			for( FxSize i = 0; i < newSelection.Length(); ++i )
			{
				for( FxSize j = 0; j < _objects.Length(); ++j )
				{
					if( newSelection[i] == _objects[j] )
					{
						newSelection.Remove(i);
					}
				}
			}
		}
		else
		{
			for( FxSize i = 0; i < _objects.Length(); ++i )
			{
				newSelection.PushBack(_objects[i]);
			}
		}
	}

	if( kLinkType == _type )
	{
		FxArray<FxString> node1;
		node1.PushBack(_node1);
		if( !FxSessionProxy::SetSelectedFaceGraphNodes(node1, _zoomToSelection) )
		{
			return CE_Failure;
		}
		if( !FxSessionProxy::SetSelectedFaceGraphNodeLink(_node1, _node2) )
		{
			return CE_Failure;
		}
		return CE_Success;
	}

	if( !_clear && (0 == newSelection.Length() && !(_type == kNodeType || _type == kCurveType)) )
	{
		return CE_Failure;
	}

	if( kNodeType == _type )
	{
		if( !FxSessionProxy::SetSelectedFaceGraphNodes(newSelection, _zoomToSelection) )
		{
			return CE_Failure;
		}
	}
	else if( kNodeGroupType == _type )
	{
		if( !FxSessionProxy::SetSelectedFaceGraphNodeGroup(newSelection[0]) )
		{
			return CE_Failure;
		}
	}
	else if( kAnimGroupType == _type )
	{
		if( !FxSessionProxy::SetSelectedAnimGroup(newSelection[0]) )
		{
			return CE_Failure;
		}
	}
	else if( kAnimType == _type )
	{
		if( _group.Length() > 0 )
		{
			if( !FxSessionProxy::SetSelectedAnimGroup(_group) )
			{
				return CE_Failure;
			}
		}
		if( !FxSessionProxy::SetSelectedAnim(newSelection[0]) )
		{
			return CE_Failure;
		}
	}
	else if( kCurveType == _type )
	{
		if( !FxSessionProxy::SetSelectedAnimCurves(newSelection) )
		{
			return CE_Failure;
		}
	}
	else
	{
		FxVM::DisplayError("unknown object type!");
		return CE_Failure;
	}
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
