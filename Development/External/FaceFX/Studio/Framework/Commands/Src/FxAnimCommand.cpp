//------------------------------------------------------------------------------
// The animation command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAnimCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxAnimCommand, 0, FxCommand);

FxAnimCommand::FxAnimCommand()
{
	_isUndoable			   = FxTrue;
	_add				   = FxFalse;
	_remove				   = FxFalse;
	_isSelected            = FxFalse;
	_move		           = FxFalse;
	_copy				   = FxFalse;
	_pOldUserData		   = NULL;
	_okToDeleteOldUserData = FxFalse;
}

FxAnimCommand::~FxAnimCommand()
{
	if( _pOldUserData && _okToDeleteOldUserData )
	{
		delete _pOldUserData;
		_pOldUserData = NULL;
	}
}

FxCommandSyntax FxAnimCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-a", "-add", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-r", "-remove", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-gr", "-group", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-mv", "-move", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-cp", "-copy", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-fr", "-from", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-to", "-to", CAT_String));
	return newSyntax;
}

FxCommandError FxAnimCommand::Execute( const FxCommandArgumentList& argList )
{
	_add    = argList.GetArgument("-add");
	_remove = argList.GetArgument("-remove");
	_move   = argList.GetArgument("-move");
	_copy	= argList.GetArgument("-copy");     

	if( _add && (_remove || _move || _copy) )
	{
		FxVM::DisplayError("multiple operations were specified!");
		return CE_Failure;
	}
	else if( _remove && (_add || _move || _copy) )
	{
		FxVM::DisplayError("multiple operations were specified!");
		return CE_Failure;
    }
	else if( _move && (_add || _remove || _copy) )
	{
		FxVM::DisplayError("multiple operations were specified!");
		return CE_Failure;
	}
	else if( _copy && (_add || _remove || _move) )
	{
		FxVM::DisplayError("multiple operations were specified!");
		return CE_Failure;
	}
	else if( !_add && !_remove && !_move && !_copy)
	{
		FxVM::DisplayError("no operations were specified!");
		return CE_Failure;
	}

	if( _move || _copy )
	{
		if( !argList.GetArgument("-from", _from) )
		{
			FxVM::DisplayError("-from was not specified!");
			return CE_Failure;
		}
		if( !argList.GetArgument("-to", _to) )
		{
			FxVM::DisplayError("-to was not specified!");
			return CE_Failure;
		}
	}
	else
	{
		if( !argList.GetArgument("-group", _group) )
		{
			FxVM::DisplayError("-group was not specified!");
			return CE_Failure;
		}
	}

	if( !argList.GetArgument("-name", _name) )
	{
		FxVM::DisplayError("-name was not specified!");
		return CE_Failure;
	}

	return Redo();
}

FxCommandError FxAnimCommand::Undo( void )
{
	if( _add )
	{
		if( FxSessionProxy::RemoveAnim(_group, _name) )
		{
			FxSessionProxy::SetSelectedAnimGroup(_cachedAnimGroup);
			FxSessionProxy::SetSelectedAnim(_cachedAnim);
			FxSessionProxy::SetSelectedAnimCurves(_cachedCurves);
			FxSessionProxy::SetSelectedKeys(_cachedCurveIndices, _cachedKeyIndices);

			return CE_Success;
		}
	}
	if( _remove )
	{
		if( FxSessionProxy::AddAnim(_group, _oldAnim) )
		{
			_okToDeleteOldUserData = FxFalse;
			if( _isSelected )
			{
				FxSessionProxy::SetSelectedAnimGroup(_cachedAnimGroup);
				FxSessionProxy::SetSelectedAnim(_cachedAnim);
				FxSessionProxy::SetSelectedAnimCurves(_cachedCurves);
				FxSessionProxy::SetSelectedKeys(_cachedCurveIndices, _cachedKeyIndices);
			}
			return CE_Success;
		}
	}
	if( _move )
	{
		if( FxSessionProxy::MoveAnim(_to, _from, _name) )
		{
			FxSessionProxy::SetSelectedAnimGroup(_cachedAnimGroup);
			FxSessionProxy::SetSelectedAnim(_cachedAnim);
			FxSessionProxy::SetSelectedAnimCurves(_cachedCurves);
			FxSessionProxy::SetSelectedKeys(_cachedCurveIndices, _cachedKeyIndices);

			return CE_Success;
		}
	}
	if( _copy )
	{
		if( FxSessionProxy::RemoveAnim(_to, _name) )
		{
			return CE_Success;
		}
	}
	return CE_Failure;
}

FxCommandError FxAnimCommand::Redo( void )
{
	if( _add )
	{
		FxSessionProxy::GetSelectedAnimGroup(_cachedAnimGroup);
		FxSessionProxy::GetSelectedAnim(_cachedAnim);
		FxSessionProxy::GetSelectedAnimCurves(_cachedCurves);
		FxSessionProxy::GetSelectedKeys(_cachedCurveIndices, _cachedKeyIndices);

		if( FxSessionProxy::AddAnim(_group, _name) )
		{
			return CE_Success;
		}
		else
		{
			FxVM::DisplayError("could not add animation!");
		}
	}
	if( _remove )
	{
		FxSessionProxy::GetSelectedAnimGroup(_cachedAnimGroup);
		FxSessionProxy::GetSelectedAnim(_cachedAnim);
		FxSessionProxy::GetSelectedAnimCurves(_cachedCurves);
		FxSessionProxy::GetSelectedKeys(_cachedCurveIndices, _cachedKeyIndices);

		FxAnim* pTempAnim = NULL;
		if( FxSessionProxy::GetAnim(_group, _name, &pTempAnim) )
		{
			FxString selectedAnimGroup;
			FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
			FxString selectedAnim;
			FxSessionProxy::GetSelectedAnim(selectedAnim);
            if( _group == selectedAnimGroup && _name == selectedAnim )
			{
				_isSelected = FxTrue;
			}
			// PreRemoveAnim does some housekeeping if the animation about to
			// be removed is the currently selected animation.
			FxSessionProxy::PreRemoveAnim(_group, _name);
			_oldAnim = *pTempAnim;
			FxAnimUserData* pNewUserData = NULL;
			if( pTempAnim->GetUserData() )
			{
				pNewUserData = new FxAnimUserData();
				*pNewUserData = *reinterpret_cast<FxAnimUserData*>(pTempAnim->GetUserData());
				_oldAnim.SetUserData(pNewUserData);
			}
			if( FxSessionProxy::RemoveAnim(_group, _name) )
			{
				if( pNewUserData )
				{
					_pOldUserData = pNewUserData;
					_okToDeleteOldUserData = FxTrue;
				}
				return CE_Success;
			}
			else
			{
				if( pNewUserData )
				{
					delete pNewUserData;
					pNewUserData = NULL;
				}
				FxVM::DisplayError("could not remove animation!");
			}
		}
	}
	if( _move )
	{
		FxSessionProxy::GetSelectedAnimGroup(_cachedAnimGroup);
		FxSessionProxy::GetSelectedAnim(_cachedAnim);
		FxSessionProxy::GetSelectedAnimCurves(_cachedCurves);
		FxSessionProxy::GetSelectedKeys(_cachedCurveIndices, _cachedKeyIndices);

		if( FxSessionProxy::MoveAnim(_from, _to, _name) )
		{
			return CE_Success;
		}
		else
		{
			FxVM::DisplayError("could not move animation!");
		}
	}
	if( _copy )
	{
		if( FxSessionProxy::CopyAnim(_from, _to, _name) )
		{
			return CE_Success;
		}
		else
		{
			FxVM::DisplayError("could not copy animation!");
		}
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
