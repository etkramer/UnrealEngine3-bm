//------------------------------------------------------------------------------
// Engine-independent camera management for workspace viewports.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCameraManager.h"
#ifdef IS_FACEFX_STUDIO
	#include "FxSessionProxy.h"
#endif

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxViewportCameraVersion 0
    
FX_IMPLEMENT_CLASS(FxViewportCamera, kCurrentFxViewportCameraVersion, FxNamedObject)

FxViewportCamera::FxViewportCamera()
	: _isBoundToBone(FxFalse)
	, _usesFixedAspectRatio(FxFalse)
	, _fixedAspectRatioWidth(1.0f)
	, _fixedAspectRatioHeight(1.0f)
{
}

FxViewportCamera::~FxViewportCamera()
{
}

FxBool FxViewportCamera::IsBoundToBone( void ) const
{
	return _isBoundToBone;
}

void FxViewportCamera::SetIsBoundToBone( FxBool isBoundToBone )
{
	_isBoundToBone = isBoundToBone;
}

const FxName& FxViewportCamera::GetBoundBoneName( void ) const
{
	return _boneName;
}

void FxViewportCamera::SetBoundBoneName( const FxName& boneName )
{
	_boneName = boneName;
}

FxBool FxViewportCamera::UsesFixedAspectRatio( void ) const
{
	return _usesFixedAspectRatio;
}

void FxViewportCamera::SetUsesFixedAspectRatio( FxBool usesFixedAspectRatio )
{
	_usesFixedAspectRatio = usesFixedAspectRatio;
}

FxReal FxViewportCamera::GetFixedAspectRatioWidth( void ) const
{
	return _fixedAspectRatioWidth;
}

void FxViewportCamera::SetFixedAspectRatioWidth( FxReal fixedAspectRatioWidth )
{
	_fixedAspectRatioWidth = fixedAspectRatioWidth;
}

FxReal FxViewportCamera::GetFixedAspectRatioHeight( void ) const
{
	return _fixedAspectRatioHeight;
}

void FxViewportCamera::SetFixedAspectRatioHeight( FxReal fixedAspectRatioHeight )
{
	_fixedAspectRatioHeight = fixedAspectRatioHeight;
}

void FxViewportCamera::SetBoneBindRot( const FxQuat& boneBindRot )
{
	_boneBindRot = boneBindRot;
}

const FxQuat& FxViewportCamera::GetBoneBindRot( void ) const
{
	return _boneBindRot;
}

void FxViewportCamera::SetBoneBindPos( const FxVec3& boneBindPos )
{
	_boneBindPos = boneBindPos;
}

const FxVec3& FxViewportCamera::GetBoneBindPos( void ) const
{
	return _boneBindPos;
}

void FxViewportCamera::SetBindPos( const FxVec3& bindPos )
{
	_bindPos = bindPos;
}

const FxVec3& FxViewportCamera::GetBindPos( void ) const
{
	return _bindPos;
}

void FxViewportCamera::SetBindRot( const FxQuat& bindRot )
{
	_bindRot = bindRot;
}

const FxQuat& FxViewportCamera::GetBindRot( void ) const
{
	return _bindRot;
}

void FxViewportCamera::SetPos( const FxVec3& pos )
{
	_pos = pos;
}

const FxVec3& FxViewportCamera::GetPos( void ) const
{
	return _pos;
}

void FxViewportCamera::SetRot( const FxQuat& rot )
{
	_rot = rot;
}

const FxQuat& FxViewportCamera::GetRot( void ) const
{
	return _rot;
}

void FxViewportCamera::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxViewportCamera");
		
	arc << _isBoundToBone << _boneName << _usesFixedAspectRatio 
		<< _fixedAspectRatioWidth << _fixedAspectRatioHeight 
		<< _bindPos << _bindRot;

	if( arc.IsLoading() )
	{
		_pos = _bindPos;
		_rot = _bindRot;
	}
}

#define kCurrentFxCameraManagerVersion 0

FxArray<FxViewportCamera> FxCameraManager::_cameras;

FxSize FxCameraManager::GetNumCameras( void )
{
	return _cameras.Length();
}

void FxCameraManager::AddCamera( const FxViewportCamera& camera )
{
	if( NULL == GetCamera(camera.GetName()) )
	{
		_cameras.PushBack(camera);
	}
}

void FxCameraManager::RemoveCamera( const FxName& cameraName )
{
	for( FxSize i = 0; i < _cameras.Length(); ++i )
	{
		if( _cameras[i].GetName() == cameraName )
		{
			_cameras.Remove(i);
			break;
		}
	}
}

FxViewportCamera* FxCameraManager::GetCamera( const FxName& cameraName )
{
	for( FxSize i = 0; i < _cameras.Length(); ++i )
	{
		if( _cameras[i].GetName() == cameraName )
		{
			return &_cameras[i];
		}
	}
	return NULL;
}

FxViewportCamera* FxCameraManager::GetCamera( FxSize index )
{
	return &_cameras[index];
}

void FxCameraManager::FlushAllCameras( void )
{
	_cameras.Clear();
}

void FxCameraManager::Serialize( FxArchive& arc )
{
	arc.SerializeClassVersion("FxCameraManager", FxTrue, kCurrentFxCameraManagerVersion);

	arc << _cameras;

	if( arc.IsLoading() )
	{
#ifdef IS_FACEFX_STUDIO
		FxSessionProxy::ActorDataChanged(ADCF_CamerasChanged);
#endif
	}
}

} // namespace Face

} // namespace OC3Ent
