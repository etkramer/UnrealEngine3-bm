//------------------------------------------------------------------------------
// Controls access to Max bone animation curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMaxAnimController.h"
#include "FxMaxData.h"
#include "FxToolLog.h"

using namespace OC3Ent;
using namespace Face;

// Constructor.
FxMaxAnimCurve::FxMaxAnimCurve( const OC3Ent::Face::FxString& inBoneName )
	: boneName(inBoneName)
	, pNode(NULL)
	, pPositionController(NULL)
	, pScaleController(NULL)
	, pRotationController(NULL)
	, wasTCB(FxFalse)
	, pPositionKeys(NULL)
	, pScaleKeys(NULL)
	, pRotationKeys(NULL)
{
}

// Destructor.
FxMaxAnimCurve::~FxMaxAnimCurve()
{
}

// Constructor.
FxMaxAnimController::FxMaxAnimController()
	: _pMaxInterface(NULL)
{
}

// Destructor.
FxMaxAnimController::~FxMaxAnimController()
{
	_clear();
}

// Sets the Max interface pointer.
void FxMaxAnimController::SetMaxInterfacePointer( Interface* pMaxInterface )
{
	_pMaxInterface = pMaxInterface;
}

// Initializes the controller with animation curves for the specified
// bones.
FxBool 
FxMaxAnimController::Initialize( const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& bones )
{
	// Clear any previous entries.
	_clear();

	// Set up the new entries.
	for( FxSize i = 0; i < bones.Length(); ++i )
	{
		// Get the node in the Max scene.
		INode* pNode = GetCOREInterface()->GetINodeByName(_T(bones[i].GetData()));
		
		if( pNode )
		{
			// Create a new entry.
			FxMaxAnimCurve* pAnimCurve = new FxMaxAnimCurve(bones[i]);
			pAnimCurve->pNode = pNode;

			// Get the controllers.
			pAnimCurve->pPositionController = pAnimCurve->pNode->GetTMController()->GetPositionController();
			pAnimCurve->pScaleController    = pAnimCurve->pNode->GetTMController()->GetScaleController();
			pAnimCurve->pRotationController = pAnimCurve->pNode->GetTMController()->GetRotationController();

			// Get the keys.
			if( pAnimCurve->pPositionController )
			{
				pAnimCurve->pPositionKeys = GetKeyControlInterface(pAnimCurve->pPositionController);
			}
			if( pAnimCurve->pScaleController )
			{
				pAnimCurve->pScaleKeys = GetKeyControlInterface(pAnimCurve->pScaleController);
			}
			if( pAnimCurve->pRotationController )
			{
				pAnimCurve->pRotationKeys = GetKeyControlInterface(pAnimCurve->pRotationController);
			}
			_animCurves.PushBack(pAnimCurve);
		}
	}

	// Verify that all of the bones exist in the Max scene and that the 
	// controllers are types that we can deal with.
	FxBool foundAllBonesInScene    = FxTrue;
	FxBool allControllersSupported = FxTrue;
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		if( _animCurves[i] )
		{
			if( _animCurves[i]->pNode               && 
				_animCurves[i]->pPositionController &&
				_animCurves[i]->pScaleController    &&
				_animCurves[i]->pRotationController )
			{
				if( Class_ID(HYBRIDINTERP_POSITION_CLASS_ID, 0) == _animCurves[i]->pPositionController->ClassID() )
				{
					// Do nothing.
				}
				else if( Class_ID(TCBINTERP_POSITION_CLASS_ID, 0) == _animCurves[i]->pPositionController->ClassID() )
				{
					// Do nothing.
				}
				else if( Class_ID(LININTERP_POSITION_CLASS_ID, 0) == _animCurves[i]->pPositionController->ClassID() )
				{
					// Do nothing.
				}
				else if( BIPBODY_CONTROL_CLASS_ID  == _animCurves[i]->pPositionController->ClassID() ||
						 BIPSLAVE_CONTROL_CLASS_ID == _animCurves[i]->pPositionController->ClassID() )
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Character Studio controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}
				else
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Unsupported position controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}

				if( Class_ID(HYBRIDINTERP_SCALE_CLASS_ID, 0) == _animCurves[i]->pScaleController->ClassID() )
				{
					// Do nothing.
				}
				else if( Class_ID(TCBINTERP_SCALE_CLASS_ID, 0) == _animCurves[i]->pScaleController->ClassID() )
				{
					// Do nothing.
				}
				else if( Class_ID(LININTERP_SCALE_CLASS_ID, 0) == _animCurves[i]->pScaleController->ClassID() )
				{
					// Do nothing.
				}
				else if( BIPBODY_CONTROL_CLASS_ID  == _animCurves[i]->pScaleController->ClassID() ||
						 BIPSLAVE_CONTROL_CLASS_ID == _animCurves[i]->pScaleController->ClassID() )
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Character Studio controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}
				else
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Unsupported scale controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}

				if( Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0) == _animCurves[i]->pRotationController->ClassID() )
				{
					// Do nothing.
				}
				else if( Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0) == _animCurves[i]->pRotationController->ClassID() )
				{
					// Do nothing.
				}
				else if( Class_ID(LININTERP_ROTATION_CLASS_ID, 0) == _animCurves[i]->pRotationController->ClassID() )
				{
					// Do nothing.
				}
				else if( BIPBODY_CONTROL_CLASS_ID  == _animCurves[i]->pRotationController->ClassID() ||
						 BIPSLAVE_CONTROL_CLASS_ID == _animCurves[i]->pRotationController->ClassID() )
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Character Studio controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}
				else if( Class_ID(EULER_CONTROL_CLASS_ID, 0) == _animCurves[i]->pRotationController->ClassID() )
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Euler controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}
				else
				{
					allControllersSupported = FxFalse;
					FxToolLog("WARNING Unsupported rotation controller detected on %s.  No keys will be set.", _animCurves[i]->boneName.GetData());
				}
			}
			else
			{
				foundAllBonesInScene = FxFalse;
				FxToolLog("WARNING could not find bone %s in Max scene", _animCurves[i]->boneName.GetData());
			}
		}
	}

	if( !foundAllBonesInScene )
	{
		if(  OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
		{
			::MessageBox(NULL, _T("Not all of the bones in the actor could be found in the Max scene.\nCheck the log file for details."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
		}
	}

	if( !allControllersSupported )
	{
		FxToolLog("WARNING Some of the bones could not be controlled by FaceFX because they use unsupported position, rotation, or scale controllers.");
		FxToolLog("To change the controllers, select the bones and type the following three commands:");
		FxToolLog("for obj in selection do obj.position.controller = Linear_Position()");
		FxToolLog("for obj in selection do obj.rotation.controller = Linear_Rotation()");
		FxToolLog("for obj in selection do obj.scale.controller = Linear_Scale()");
		if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
		{
			::MessageBox(NULL, _T("Some bones in the actor contain unsupported controllers in the Max scene.\nCheck the log file for details."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
		}
	}

	return FxTrue;

}

// Finds the animation curve associated with the specified bone name and
// returns a pointer to it or NULL if it does not exist.
FxMaxAnimCurve* FxMaxAnimController::FindAnimCurve( const OC3Ent::Face::FxString& bone )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		if( _animCurves[i] )
		{
			if( _animCurves[i]->boneName == bone )
			{
				return _animCurves[i];
			}
		}
	}
	return NULL;
}

// Removes all keys from all bone animation curves.
void FxMaxAnimController::Clean( void )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		if( _animCurves[i] )
		{
			if( _animCurves[i]->pPositionController && _animCurves[i]->pPositionKeys )
			{
				// SetNumKeys(0) on Character Studio Bip01 causes Max to crash.
				if( BIPBODY_CONTROL_CLASS_ID != _animCurves[i]->pPositionController->ClassID() )
				{
					_animCurves[i]->pPositionKeys->SetNumKeys(0);
				}
			}
			if( _animCurves[i]->pScaleController && _animCurves[i]->pScaleKeys )
			{
				// SetNumKeys(0) on Character Studio Bip01 causes Max to crash.
				if( BIPBODY_CONTROL_CLASS_ID != _animCurves[i]->pScaleController->ClassID() )
				{
					_animCurves[i]->pScaleKeys->SetNumKeys(0);
				}
			}
			if( _animCurves[i]->pRotationController && _animCurves[i]->pRotationKeys )
			{
				// SetNumKeys(0) on Character Studio Bip01 causes Max to crash.
				if( BIPBODY_CONTROL_CLASS_ID != _animCurves[i]->pRotationController->ClassID() )
				{
					_animCurves[i]->pRotationKeys->SetNumKeys(0);
				}
			}
		}
	}
}

// Prepares the bone animation curve rotation controllers so that any TCB
// controllers are switched to Linear controllers before importing an
// animation.  This is an optimization for importing animations in Max.
void FxMaxAnimController::PrepareRotationControllers( void )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		if( _animCurves[i] && _animCurves[i]->pRotationController )
		{
			if( Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0) == _animCurves[i]->pRotationController->ClassID() )
			{
				// TCB rotations in Max are stored with each successive quaternion relative to the
				// previous quaternion.  This makes inserting keys at "random" locations in the key
				// list problematic because the complete quaternion "chain" needs to be recomputed.
				// Several methods were attempted to do this correctly, but the documentation was
				// incomplete and the SDK appeared to be missing some functionality to handle this
				// operation.  The operation worked fine in MaxScript and the Max GUI, but the SDK
				// code just did not work the same way.  That is where this hack-ish procedure came
				// from - it was either this or drop support for TCB rotations completely.

				// Set wasTCB to FxTrue so that RestoreRotationControllers() can restore the
				// controller back to TCB.
				_animCurves[i]->wasTCB = FxTrue;

				// Brute force switching rotation controllers by hacking into MaxScript.
				FxString maxScript = "tempobj = getnodebyname \"";
				maxScript = FxString::Concat(maxScript, _animCurves[i]->boneName);
				maxScript = FxString::Concat(maxScript, "\"");
				GUP* pGP = (GUP*)CreateInstance(GUP_CLASS_ID, Class_ID(470000002, 0));
				pGP->ExecuteStringScript(_T(const_cast<FxChar*>(maxScript.GetData())));
				pGP->ExecuteStringScript(_T("tempobj.rotation.controller = Linear_Rotation()"));

				// Hook into the new controllers.
				_animCurves[i]->pRotationController = _animCurves[i]->pNode->GetTMController()->GetRotationController();
				_animCurves[i]->pRotationKeys       = GetKeyControlInterface(_animCurves[i]->pRotationController);
			}
		}
	}
}

// Restores the bone animation curve rotation controllers so that any former
// TCB controllers and restored back to TCB controllers.  This is an
// optimization for importing animations in Max.
void FxMaxAnimController::RestoreRotationControllers( void )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		if( _animCurves[i] && _animCurves[i]->pNode )
		{
			if( _animCurves[i]->wasTCB )
			{
				// Brute force switching rotation controllers by hacking into MaxScript.
				FxString maxScript = "tempobj = getnodebyname \"";
				maxScript = FxString::Concat(maxScript, _animCurves[i]->boneName);
				maxScript = FxString::Concat(maxScript, "\"");
				GUP* pGP = (GUP*)CreateInstance(GUP_CLASS_ID, Class_ID(470000002, 0));
				pGP->ExecuteStringScript(_T(const_cast<FxChar*>(maxScript.GetData())));
				pGP->ExecuteStringScript(_T("tempobj.rotation.controller = TCB_Rotation()"));

				// Hook into the new controllers.
				_animCurves[i]->pRotationController = _animCurves[i]->pNode->GetTMController()->GetRotationController();
				if( _animCurves[i]->pRotationController )
				{
					_animCurves[i]->pRotationKeys = GetKeyControlInterface(_animCurves[i]->pRotationController);
				}
				// Set wasTCB back to FxFalse so that if someone calls RestoreRotationControllers()
				// consecutively we don't do any unneeded work.
				_animCurves[i]->wasTCB = FxFalse;
			}
		}
	}
}

// Clears the list of bone animation curves.
void FxMaxAnimController::_clear( void )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		delete _animCurves[i];
		_animCurves[i] = NULL;
	}
	_animCurves.Clear();
}
