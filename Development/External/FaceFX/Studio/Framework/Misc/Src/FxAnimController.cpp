//------------------------------------------------------------------------------
// Provides for tying an animation to an FxFaceGraph, and exposes mechanisms 
// to operate on them both as a single unit, ensuring correct operation.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAnimController.h"
#include "FxCurveUserData.h"
#include "FxAnimUserData.h"
#include "FxAnim.h"
#include "FxFaceGraph.h"
#include "FxColourMapping.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FxAnimController::FxAnimController()
	: _pAnim(NULL)
	, _pFaceGraph(NULL)
	, _editCurve(FxInvalidIndex)
	, _editKey(FxInvalidIndex)
	, _currentTime(0.0f)
	, _lastEvalTime(0.0f)
{
}

FxAnimController::~FxAnimController()
{
	DestroyUserData();
	_pAnim       = NULL;
	_pFaceGraph  = NULL;
	_nodeUserValues.Clear();
}

void FxAnimController::SetAnimPointer( FxAnim* pAnim )
{
	_pAnim = pAnim;
	ProofUserData();
}

void FxAnimController::SetFaceGraphPointer( FxFaceGraph* pFaceGraph )
{
	_pFaceGraph = pFaceGraph;
	_nodeUserValues.Clear();
	if( _pFaceGraph )
	{
		_nodeUserValues.Reserve(_pFaceGraph->GetNumNodes());
	}
	Link();
}

void FxAnimController::GetAnimationTime( FxReal& startTime, FxReal& endTime )
{
	if( _pAnim )
	{
		startTime = _pAnim->GetStartTime();
		endTime   = _pAnim->GetEndTime();
	}
}

void FxAnimController::DestroyUserData( void )
{
	if( _pAnim )
	{
		FxSize numAnimCurves = _pAnim->GetNumAnimCurves();
		for( FxSize i = 0; i < numAnimCurves; ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			FxSize numKeys = curve.GetNumKeys();
			for( FxSize j = 0; j < numKeys; ++j )
			{
				FxAnimKey& key = curve.GetKeyM(j);
				delete reinterpret_cast<FxKeyUserData*>(key.GetUserData());
				key.SetUserData(NULL);
			}
			delete reinterpret_cast<FxCurveUserData*>(curve.GetUserData());
			curve.SetUserData(NULL);
		}
	}
}

void FxAnimController::DestroyCurveUserData( FxSize index )
{
	if( _pAnim )
	{
		FxAnimCurve& curve = _pAnim->GetAnimCurveM(index);
		FxSize numKeys = curve.GetNumKeys();
		for( FxSize j = 0; j < numKeys; ++j )
		{
			FxAnimKey& key = curve.GetKeyM(j);
			delete reinterpret_cast<FxKeyUserData*>(key.GetUserData());
			key.SetUserData( NULL );
		}
	}
}

void FxAnimController::ProofUserData( void )
{
	if( _pAnim )
	{
		// Attach all the user data to the curves and keys if it doesn't exist.
		FxSize numAnimCurves = _pAnim->GetNumAnimCurves();
		for( FxSize i = 0; i < numAnimCurves; ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( !curve.GetUserData() )
			{
				curve.SetUserData(new FxCurveUserData());
			}

			FxSize numKeys = curve.GetNumKeys();
			for( FxSize j = 0; j < numKeys; ++j )
			{
				FxAnimKey& key = curve.GetKeyM(j);
				if( !key.GetUserData() )
				{
					FxBool lockedTangents = FxRealEquality(key.GetSlopeIn(), key.GetSlopeOut());
					key.SetUserData(new FxKeyUserData(lockedTangents ? TangentsLocked : 0));
				}
			}
		}

		Link();
	}
}

const FxName& FxAnimController::GetCurveName( FxSize index ) const
{
	if( _pAnim && index != FxInvalidIndex )
	{
		return _pAnim->GetAnimCurve(index).GetName();
	}
	return FxName::NullName;
}

FxSize FxAnimController::GetCurveIndex( const FxName& name ) const
{
	if( OK() )
	{
		FxSize numAnimCurves = _pAnim->GetNumAnimCurves();
		for( FxSize i = 0; i < numAnimCurves; ++i )
		{
			if( _pAnim->GetAnimCurve(i).GetName() == name )
			{
				return i;
			}
		}
	}
	return FxInvalidIndex;
}

FxAnimCurve* FxAnimController::GetCurve( FxSize index )
{
	if( OK() )
	{
		return &_pAnim->GetAnimCurveM(index);
	}
	return NULL;
}

FxSize FxAnimController::GetNumCurves( void ) const
{
	if( _pAnim )
	{
		return _pAnim->GetNumAnimCurves();
	}
	return 0;
}

FxBool FxAnimController::IsCurveVisible( FxSize index ) const
{
	if( _pAnim && index != FxInvalidIndex && _pAnim->GetAnimCurveM(index).GetUserData() )
	{
		return reinterpret_cast<FxCurveUserData*>(_pAnim->GetAnimCurveM(index).GetUserData())->isVisible;
	}
	return FxFalse;
}

FxBool FxAnimController::IsCurveVisible( const FxName& name ) const
{
	if( _pAnim )
	{
		return IsCurveVisible(_pAnim->FindAnimCurve(name));
	}
	return FxFalse;
}

void FxAnimController::SetCurveVisible( FxSize index, FxBool visible )
{
	if( _pAnim && index != FxInvalidIndex )
	{
		if( !_pAnim->GetAnimCurveM(index).GetUserData() )
		{
			_pAnim->GetAnimCurveM(index).SetUserData(new FxCurveUserData);
		}

		reinterpret_cast<FxCurveUserData*>(_pAnim->GetAnimCurveM(index).GetUserData())->isVisible = visible;
	}
}

void FxAnimController::SetCurveVisible( const FxName& name, FxBool visible )
{
	if( _pAnim )
	{
		SetCurveVisible(_pAnim->FindAnimCurve(name), visible);
	}
}

wxColour FxAnimController::GetCurveColour( FxSize index ) const
{
	if( _pAnim && index != FxInvalidIndex )
	{
		return FxColourMap::Get(_pAnim->GetAnimCurve(index).GetName());
	}
	return *wxBLACK;
}

wxColour FxAnimController::GetCurveColour( const FxName& name ) const
{
	return FxColourMap::Get(name);
}

void FxAnimController::SetCurveColour( FxSize index, const wxColour& colour )
{
	if( _pAnim && index != FxInvalidIndex )
	{
		FxColourMap::Set(_pAnim->GetAnimCurve(index).GetName(), colour);
	}
}

void FxAnimController::SetCurveColour( const FxName& name, const wxColour& colour )
{
	if( _pAnim )
	{
		FxColourMap::Set(name, colour);
	}
}

FxBool FxAnimController::IsCurveVirtual( FxSize index ) const
{
	FxBool retval = FxFalse;
	if( OK() && index != FxInvalidIndex )
	{
		FxFaceGraphNode* node = _pFaceGraph->FindNode(_pAnim->GetAnimCurve(index).GetName());
		if( node )
		{
			retval = node->GetNumInputLinks() > 0;
		}
	}
	return retval;
}

// Returns FxTrue if the curve is owned by the analysis.
FxBool FxAnimController::IsCurveOwnedByAnalysis( FxSize index ) const
{
	FxBool retval = FxFalse;
	if( _pAnim && index != FxInvalidIndex )
	{
		FxAnimUserData* pUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
		if( pUserData )
		{
			retval = pUserData->IsCurveOwnedByAnalysis(_pAnim->GetAnimCurve(index).GetName());
		}
	}
	return retval;
}

FxSize FxAnimController::GetNumKeys( FxSize curveIndex ) const
{
	if( OK() && curveIndex != FxInvalidIndex )
	{
		return _pAnim->GetAnimCurve(curveIndex).GetNumKeys();
	}
	return 0;
}

void FxAnimController::SetKeySelected( FxSize curveIndex, FxSize keyIndex, FxBool selected )
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		if( selected )
		{
			reinterpret_cast<FxKeyUserData*>(_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).GetUserData())->flags |= KeyIsSelected;
		}
		else
		{
			reinterpret_cast<FxKeyUserData*>(_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).GetUserData())->flags &= ~KeyIsSelected;
		}
	}
}

FxBool FxAnimController::IsKeySelected( FxSize curveIndex, FxSize keyIndex ) const
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		return (reinterpret_cast<FxKeyUserData*>(
			_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).
			GetUserData())->flags & KeyIsSelected) ? FxTrue : FxFalse;
	}
	return FxFalse;
}

FxBool FxAnimController::IsKeyPivot( FxSize curveIndex, FxSize keyIndex ) const
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		return (reinterpret_cast<FxKeyUserData*>(
			_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).
			GetUserData())->flags & KeyIsPivot) ? FxTrue : FxFalse;
	}
	return FxFalse;
}

FxBool FxAnimController::IsKeyTemporary( FxSize curveIndex, FxSize keyIndex ) const
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		FxKeyUserData* userData = reinterpret_cast<FxKeyUserData*>(
			_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).GetUserData());
		if( userData )
		{
			return (userData->flags & KeyIsTemporary) ? FxTrue : FxFalse;
		}
	}
	return FxFalse;
}

FxBool FxAnimController::IsTangentLocked( FxSize curveIndex, FxSize keyIndex ) const
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		return (reinterpret_cast<FxKeyUserData*>(
			_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).
			GetUserData())->flags & TangentsLocked) ? FxTrue : FxFalse;
	}
	return FxFalse;
}

void FxAnimController::SetTangentLocked( FxSize curveIndex, FxSize keyIndex, FxBool isLocked )
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		FxAnimKey& key = _pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex);
		FxInt32& flags = reinterpret_cast<FxKeyUserData*>(key.GetUserData())->flags;
		if( isLocked )
		{
			flags |= TangentsLocked;
			key.SetSlopeOut(key.GetSlopeIn());
		}
		else
		{
			flags &= ~TangentsLocked;
		}
	}
}

//@todo Clean up all these places that OK() isn't used.
void FxAnimController::GetTangentHandles( FxSize curveIndex, FxSize keyIndex,
								          wxPoint& incTanHandle, wxPoint& outTanHandle )
{
	if( _pAnim && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		FxAnimKey& key = _pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex);
		FxKeyUserData* pData = reinterpret_cast<FxKeyUserData*>(key.GetUserData());
		if( pData )
		{
			incTanHandle = pData->incomingTanHandle;
			outTanHandle = pData->outgoingTanHandle;
		}
	}
}

void FxAnimController::SetTangentHandles( FxSize curveIndex, FxSize keyIndex,
					   const wxPoint& incTanHandle, const wxPoint outTanHandle )
{
	if( _pAnim && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		FxAnimKey& key = _pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex);
		FxKeyUserData* pData = reinterpret_cast<FxKeyUserData*>(key.GetUserData());
		if( pData )
		{
			pData->incomingTanHandle = incTanHandle;
			pData->outgoingTanHandle = outTanHandle;
		}
	}
}

void FxAnimController::GetKeyInfo( FxSize curveIndex, FxSize keyIndex, 
								   FxReal& time, FxReal& value, 
								   FxReal& slopeIn, FxReal& slopeOut )
{
	if( _pAnim && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		FxAnimKey& key = _pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex);
		value = key.GetValue();
		time  = key.GetTime();
		slopeIn = key.GetSlopeIn();
		slopeOut = key.GetSlopeOut();
	}
	else
	{
		value = time = slopeIn = slopeOut = 0.0f;
	}
}

FxReal FxAnimController::GetKeyTime( FxSize curveIndex, FxSize keyIndex )
{
	if( _pAnim && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		return _pAnim->GetAnimCurve(curveIndex).GetKey(keyIndex).GetTime();
	}
	return FxInvalidValue;
}

FxSize FxAnimController::FindKeyAtTime( FxSize curveIndex, FxReal time )
{
	if( OK() && curveIndex != FxInvalidIndex && curveIndex < _pAnim->GetNumAnimCurves() )
	{
		return _pAnim->GetAnimCurve(curveIndex).FindKey(time);
	}
	return FxInvalidIndex;
}

FxSize FxAnimController::
InsertKey( FxSize curveIndex, FxReal time, FxReal value, 
		   FxReal slopeIn, FxReal slopeOut, 
		   FxInt32 flags, FxBool autoSlope )
{
	if( OK() && curveIndex != FxInvalidIndex && !IsCurveOwnedByAnalysis(curveIndex) )
	{
		FxAnimKey newKey(time, value, slopeIn, slopeOut);
		FxSize newKeyIndex = _pAnim->GetAnimCurveM(curveIndex).InsertKey(newKey, autoSlope);
		FxKeyUserData* newKeyUserData = new FxKeyUserData(flags);
		_pAnim->GetAnimCurveM(curveIndex).GetKeyM(newKeyIndex).SetUserData(newKeyUserData);
		// If the new key exceeds the bounds, recalculate them.
		if( time < _pAnim->GetStartTime() || time > _pAnim->GetEndTime() )
		{
			_pAnim->FindBounds();
		}
		return newKeyIndex;
	}
	return FxInvalidIndex;
}

void FxAnimController::RemoveKey( FxSize curveIndex, FxSize keyIndex )
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex
		&& !IsCurveOwnedByAnalysis(curveIndex) )
	{
		if( _pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).GetUserData() )
		{
			delete _pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).GetUserData();
		}
		FxReal keyTime = _pAnim->GetAnimCurve(curveIndex).GetKey(keyIndex).GetTime();
		_pAnim->GetAnimCurveM(curveIndex).RemoveKey(keyIndex);
		// Recalculate the bounds if this was a bounding key.
		if( FxRealEquality(keyTime, _pAnim->GetStartTime()) ||
			FxRealEquality(keyTime, _pAnim->GetEndTime()) )
		{
			_pAnim->FindBounds();
		}
	}
}

void FxAnimController::
ModifyKeyValue( FxSize curveIndex, FxSize keyIndex, FxReal value )
{
	if( OK() && curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex 
		&& !IsCurveOwnedByAnalysis(curveIndex) )
	{
		_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).SetValue(value);
	}
}

void FxAnimController::SetPivotKey( FxSize curveIndex, FxSize keyIndex )
{
	if( _pAnim )
	{
		ClearPivotKey();
		reinterpret_cast<FxKeyUserData*>(_pAnim->GetAnimCurveM(curveIndex).GetKeyM(keyIndex).GetUserData())->flags |= KeyIsPivot;
	}
}

// Gets the index of the pivot key.
void FxAnimController::GetPivotKey( FxSize& curveIndex, FxSize& keyIndex )
{
	if( _pAnim )
	{
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < _pAnim->GetAnimCurveM(i).GetNumKeys(); ++j )
				{
					if( IsKeyPivot(i, j) )
					{
						curveIndex = i;
						keyIndex = j;
						return;
					}
				}
			}
		}
	}
	curveIndex = FxInvalidIndex;
	keyIndex = FxInvalidIndex;
}

void FxAnimController::ModifyPivotKey( FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut, FxBool FxUnused(canRecalc) )
{
	if( _pAnim )
	{
		FxSize pivotCurve = FxInvalidIndex;
		FxSize pivotKey = FxInvalidIndex;
		GetPivotKey(pivotCurve, pivotKey);
		if( pivotCurve != FxInvalidIndex && pivotKey != FxInvalidIndex )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(pivotCurve);
			FxAnimKey newKey(curve.GetKey(pivotKey));
			newKey.SetTime(time);
			newKey.SetValue(value);
			newKey.SetSlopeIn(slopeIn);
			newKey.SetSlopeOut(slopeOut);
			curve.ModifyKey(pivotKey, newKey);
//			if( canRecalc )
//			{
//				RecalcPivotKey();
//			}
			_pAnim->FindBounds();
		}
	}
}

FxBool FxAnimController::HitTest( FxInt32 flags, FxReal time, FxReal value, 
								  FxReal timeEpsilon, FxReal valueEpsilon,
								  FxSize* outCurveIndex, FxSize* outKeyIndex )
{
	if( _pAnim )
	{
		FxSize numCurves = _pAnim->GetNumAnimCurves();
		for( FxSize curveIndex = 0; curveIndex < numCurves; ++curveIndex )
		{
			if( IsCurveVisible(curveIndex) && !IsCurveOwnedByAnalysis(curveIndex) )
			{
				FxAnimCurve& curve = _pAnim->GetAnimCurveM(curveIndex);
				FxSize numKeys = curve.GetNumKeys();
				for( FxSize keyIndex = 0; keyIndex < numKeys; ++keyIndex )
				{
					FxAnimKey& key = curve.GetKeyM(keyIndex);
					FxKeyUserData* pData = reinterpret_cast<FxKeyUserData*>(key.GetUserData());
					if( (flags & HitTestSelected && pData->flags & KeyIsSelected ) ||
						(flags & HitTestPivot && pData->flags & KeyIsPivot ) || 
						flags & HitTestAll  )
					{
						FxReal keyTime = key.GetTime();
						FxReal keyValue = key.GetValue();
						if( time - timeEpsilon <= keyTime && keyTime <= time + timeEpsilon &&
							value - valueEpsilon <= keyValue && keyValue <= value + valueEpsilon )
						{
							// Hit test is true.
							if( outCurveIndex )
							{
								*outCurveIndex = curveIndex;
							}
							if( outKeyIndex )
							{
								*outKeyIndex = keyIndex;
							}
							return FxTrue;
						}
					}
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxAnimController::HitTestTanHandles( FxInt32 x, FxInt32 y, 
											FxInt32 dx, FxInt32 dy,
											FxInt32& whichHandle )
{
	if( _pAnim )
	{
		FxSize numCurves = _pAnim->GetNumAnimCurves();
		for( FxSize curveIndex = 0; curveIndex < numCurves; ++curveIndex )
		{
			if( IsCurveVisible(curveIndex) && !IsCurveOwnedByAnalysis(curveIndex) )
			{
				FxAnimCurve& curve = _pAnim->GetAnimCurveM(curveIndex);
				FxSize numKeys = curve.GetNumKeys();
				for( FxSize keyIndex = 0; keyIndex < numKeys; ++keyIndex )
				{
					FxAnimKey& key = curve.GetKeyM(keyIndex);
					FxKeyUserData* pData = reinterpret_cast<FxKeyUserData*>(key.GetUserData());
					if( pData->flags & KeyIsSelected || pData->flags & KeyIsPivot )
					{
						wxPoint incHandle = pData->incomingTanHandle;
						wxPoint outHandle = pData->outgoingTanHandle;
						// Check the incoming handle.
						if( x - dx <= incHandle.x && incHandle.x <= x + dx &&
							y - dy <= incHandle.y && incHandle.y <= y + dy )
						{
							_editKey = keyIndex;
							_editCurve = curveIndex;
							SetPivotKey(_editCurve, _editKey);
							whichHandle = 1;
							return FxTrue;
						}
						// Check the outgoing handle.
						if( x - dx <= outHandle.x && outHandle.x <= x + dx &&
							y - dy <= outHandle.y && outHandle.y <= y + dy )
						{
							_editKey = keyIndex;
							_editCurve = curveIndex;
							SetPivotKey(_editCurve, _editKey);
							whichHandle = 2;
							return FxTrue;
						}
					}
				}
			}
		}
	}
	return FxFalse;
}

void FxAnimController::AddToSelection( FxReal minTime, FxReal maxTime, 
									   FxReal minValue, FxReal maxValue )
{
	if( _pAnim )
	{
		if( maxTime < minTime ) FxSwap(minTime, maxTime);
		if( maxValue < minValue ) FxSwap(minValue, maxValue);
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					FxAnimKey& key = curve.GetKeyM(j);
					if( minTime <= key.GetTime() && key.GetTime() <= maxTime &&
						minValue <= key.GetValue() && key.GetValue() <= maxValue
						&& !IsKeyTemporary(i, j) )
					{
						reinterpret_cast<FxKeyUserData*>(key.GetUserData())->flags |= KeyIsSelected;
					}
				}
			}
		}
	}
	RecalcPivotKey();
}

// Toggles a selection.
void FxAnimController::ToggleSelection( FxReal minTime, FxReal maxTime,
										FxReal minValue, FxReal maxValue )
{
	if( _pAnim )
	{
		if( maxTime < minTime ) FxSwap(minTime, maxTime);
		if( maxValue < minValue ) FxSwap(minValue, maxValue);
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					FxAnimKey& key = curve.GetKeyM(j);
					if( minTime <= key.GetTime() && key.GetTime() <= maxTime &&
						minValue <= key.GetValue() && key.GetValue() <= maxValue
						&& !IsKeyTemporary(i, j) )
					{
						if( reinterpret_cast<FxKeyUserData*>(key.GetUserData())->flags & KeyIsSelected )
						{
							reinterpret_cast<FxKeyUserData*>(key.GetUserData())->flags &= ~KeyIsSelected;
						}
						else
						{
							reinterpret_cast<FxKeyUserData*>(key.GetUserData())->flags |= KeyIsSelected;
						}
					}
				}
			}
		}
	}
	RecalcPivotKey();
}

void FxAnimController::ClearSelection( void )
{
	if( _pAnim )
	{
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( reinterpret_cast<FxCurveUserData*>(curve.GetUserData())->isVisible )
			{
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					reinterpret_cast<FxKeyUserData*>(curve.GetKeyM(j).GetUserData())->flags &= ~KeyIsSelected;
				}
			}
		}
		ClearPivotKey();
	}
}

void FxAnimController::TransformSelection( FxReal deltaTime, FxReal deltaValue, FxReal slope )
{
	if( _pAnim )
	{
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				FxAnimCurve temp;

				// Create a new curve with the modified keys.
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					if( IsKeySelected(i, j) )
					{
						FxAnimKey newKey(curve.GetKey(j));
						newKey.SetTime(newKey.GetTime() + deltaTime);
						newKey.SetValue(newKey.GetValue() + deltaValue);
						if( slope != FxInvalidValue )
						{
							newKey.SetSlopeIn(slope);
							newKey.SetSlopeOut(slope);
						}
						curve.RemoveKey(j--);
						temp.InsertKey(newKey);
					}
				}

				// Merge the modified keys back into the original curve.
				for( FxSize j = 0; j < temp.GetNumKeys(); ++j )
				{
					curve.InsertKey(temp.GetKey(j));
				}
			}
		}
		//RecalcPivotKey();
		_pAnim->FindBounds();
	}
}

void FxAnimController::DeleteSelection( void )
{
	if( _pAnim )
	{
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					if( IsKeySelected(i, j) )
					{
						RemoveKey(i, j);
						j--;
					}
				}
			}
		}
		_pAnim->FindBounds();
	}
}

FxSize FxAnimController::GetNumSelectedKeys( void )
{
	FxSize keyCount = 0;
	if( _pAnim )
	{
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					if( IsKeySelected(i, j) )
					{
						keyCount++;
					}
				}
			}
		}
	}
	return keyCount;
}

FxInt32 FxAnimController::GetEditCurve( void )
{
	return _editCurve;
}

FxInt32 FxAnimController::GetEditKey( void )
{
	return _editKey;
}

FxBool FxAnimController::InsertNewKey( FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut )
{
	if( _pAnim )
	{
		// First, flag each track which needs a key.
		FxArray<FxSize> selectedTracks;
		FxArray<FxSize> visibleTracks;
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				visibleTracks.PushBack(i);
				FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					if( IsKeySelected(i, j) )
					{
						selectedTracks.PushBack(i);
						break;
					}
				}
			}
		}

		if( selectedTracks.Length() == 0 )
		{
			if( visibleTracks.Length() != 0 )
			{
				selectedTracks = visibleTracks;
			}
			else
			{
				// There is nothing to do.  Return failure.
				return FxFalse;
			}
		}

		// Clear the selection.
		ClearSelection();
		// Insert the keys, making them the new selection.
		for( FxSize i = 0; i < selectedTracks.Length(); ++i )
		{
			FxAnimKey newKey(time, value, slopeIn, slopeOut);
			newKey.SetUserData(new FxKeyUserData(TangentsLocked|KeyIsSelected));
			_pAnim->GetAnimCurveM(selectedTracks[i]).InsertKey(newKey, FxTrue);
		}
		RecalcPivotKey();
		_pAnim->FindBounds();
	}
	return FxTrue;
}

void FxAnimController::SetSelectionTangentLocked( FxBool locked )
{
	if( _pAnim )
	{
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			FxAnimCurve& curve = _pAnim->GetAnimCurveM(i);
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					if( IsKeySelected(i, j) )
					{
						SetTangentLocked(i, j, locked);
					}
				}
			}
		}
	}
}

void FxAnimController::Evaluate( FxReal time, FxBool useDummyRegisters )
{
	_lastEvalTime = time;
	if( OK() )
	{
		// Load the curve evaluation results into the Face Graph.
		FxCompiledFaceGraph* pCG = NULL;
		FxSessionProxy::GetCompiledFaceGraph(&pCG);
		FxAssert(pCG);
		FxSize numCurves = _pAnim->GetNumAnimCurves();
		for( FxSize i = 0; i < numCurves; ++i )
		{
			const FxAnimCurve& curve = _pAnim->GetAnimCurve(i);
			FxSize nodeIndex = pCG->FindNodeIndex(curve.GetName());
			if( FxInvalidIndex != nodeIndex )
			{
				pCG->nodes[nodeIndex].trackValue = curve.EvaluateAt(time);
			}
		}
		FxSessionProxy::UpdateActorInstance(useDummyRegisters);
	}
}

FxReal FxAnimController::GetCurveValue( FxSize curveIndex, FxReal time )
{
	if( _pAnim )
	{
		return _pAnim->GetAnimCurve(curveIndex).EvaluateAt(time);
	}
	return 0.0f;
}

FxReal FxAnimController::GetValue( FxSize curveIndex, FxBool useDummyRegisters )
{
	if( OK() )
	{
		FxCompiledFaceGraph* pCG = NULL;
		FxSessionProxy::GetCompiledFaceGraph(&pCG);
		FxAssert(pCG);
		const FxName& curveName = _pAnim->GetAnimCurve(curveIndex).GetName();
		FxSize nodeIndex = pCG->FindNodeIndex(curveName);
		if( FxInvalidIndex != nodeIndex )
		{
			FxReal value;
			if( useDummyRegisters )
			{
				FxSessionProxy::GetDummyRegister(curveName, value);
			}
			else
			{
				FxSessionProxy::GetRegister(curveName, value);
			}
			return value;
		}
		else
		{
			return GetCurveValue(curveIndex, _lastEvalTime);
		}
	}
	return 0.0f;
}

FxReal FxAnimController::GetTime( void ) const
{
	return _currentTime;
}

void FxAnimController::SetTime( FxReal time, FxBool shouldDisableRegisters )
{
	_currentTime = time;
	if( OK() )
	{
		if( shouldDisableRegisters )
		{
			FxSessionProxy::DisableAllRegisters();
		}
		Evaluate(_currentTime);
	}
}

//@todo compiled Remove this?
FxBool FxAnimController::Link( void )
{
	if( OK() )
	{
		return FxTrue;
	}
	return FxFalse;
}

void FxAnimController::ClearPivotKey( void )
{
	for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
	{
		for( FxSize j = 0; j < _pAnim->GetAnimCurveM(i).GetNumKeys(); ++j )
		{
			reinterpret_cast<FxKeyUserData*>(_pAnim->GetAnimCurveM(i).GetKeyM(j).GetUserData())->flags &= ~KeyIsPivot;
		}
	}
}

void FxAnimController::RecalcPivotKey( void )
{
	if( _pAnim )
	{
		FxReal minTime = FX_REAL_MAX;
		_editKey = FxInvalidIndex;
		_editCurve = FxInvalidIndex;

		ClearPivotKey();

		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			if( IsCurveVisible(i) )
			{
				for( FxSize j = 0; j < _pAnim->GetAnimCurveM(i).GetNumKeys(); ++j )
				{
					if( IsKeySelected(i, j) )
					{
						FxReal keyTime = _pAnim->GetAnimCurveM(i).GetKeyM(j).GetTime();
						if( keyTime < minTime )
						{
							minTime = keyTime;
							_editKey = j;
							_editCurve = i;
						}
					}
				}
			}
		}

		if( _editCurve != FxInvalidIndex && _editKey != FxInvalidIndex )
		{
			reinterpret_cast<FxKeyUserData*>(_pAnim->GetAnimCurveM(_editCurve).GetKeyM(_editKey).GetUserData())->flags |= KeyIsPivot;
		}
	}
}

void FxAnimController::RecalcEditKey( void )
{
	if( _pAnim )
	{
		_editKey = FxInvalidIndex;
		_editCurve = FxInvalidIndex;
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			if( IsCurveVisible(i) && !IsCurveOwnedByAnalysis(i) )
			{
				for( FxSize j = 0; j < _pAnim->GetAnimCurveM(i).GetNumKeys(); ++j )
				{
					if( IsKeyPivot(i, j) )
					{
						_editKey = j;
						_editCurve = i;
					}
				}
			}
		}
	}
}

} // namespace Face

} // namespace OC3Ent
