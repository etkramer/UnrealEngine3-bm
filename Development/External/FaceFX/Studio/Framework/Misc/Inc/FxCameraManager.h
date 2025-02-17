//------------------------------------------------------------------------------
// Engine-independent camera management for workspace viewports.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCameraManager_H__
#define FxCameraManager_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxNamedObject.h"
#include "FxVec3.h"
#include "FxQuat.h"

namespace OC3Ent
{

namespace Face
{

// An engine-independent workspace viewport camera.
class FxViewportCamera : public FxNamedObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxViewportCamera, FxNamedObject)
public:
	// Constructor.
	FxViewportCamera();
	// Destructor.
	virtual ~FxViewportCamera();

	// Returns FxTrue if the camera is currently "bound" to a bone.
	FxBool IsBoundToBone( void ) const;
	// Sets whether or not the camera is "bound" to a bone.
	void SetIsBoundToBone( FxBool isBoundToBone );

	// Returns the name of the bone the camera is currently "bound" to, if any.
	const FxName& GetBoundBoneName( void ) const;
	// Sets the name of the bone the camera is currently "bound" to.
	void SetBoundBoneName( const FxName& boneName );

	// Returns FxTrue if the camera uses a fixed aspect ratio.
	FxBool UsesFixedAspectRatio( void ) const;
	// Sets whether or not the camera uses a fixed aspect ratio.
	void SetUsesFixedAspectRatio( FxBool usesFixedAspectRatio );

	// Returns the width of the fixed aspect ratio.
	FxReal GetFixedAspectRatioWidth( void ) const;
	// Sets the width of the fixed aspect ratio.
	void SetFixedAspectRatioWidth( FxReal fixedAspectRatioWidth );

	// Returns the height of the fixed aspect ratio.
	FxReal GetFixedAspectRatioHeight( void ) const;
	// Sets the height of the fixed aspect ratio.
	void SetFixedAspectRatioHeight( FxReal fixedAspectRatioHeight );

	//@todo Note the the following BoneBind* functions set internal bone "bind"
	//      variables that are not yet serialized in 1.5.
	// Sets the bone "bind" rotation.
	void SetBoneBindRot( const FxQuat& boneBindRot );
	// Returns the bone "bind" rotation.
	const FxQuat& GetBoneBindRot( void ) const;
	// Sets the bone "bind" position.
	void SetBoneBindPos( const FxVec3& boneBindPos );
	// Returns the bone "bind" position.
	const FxVec3& GetBoneBindPos( void ) const;

	// Sets the "bind" position.
	void SetBindPos( const FxVec3& bindPos );
	// Returns the "bind" position.
	const FxVec3& GetBindPos( void ) const;

	// Sets the "bind" rotation.
	void SetBindRot( const FxQuat& bindRot );
	// Returns the "bind" rotation.
	const FxQuat& GetBindRot( void ) const;

	// Sets the current position.
	void SetPos( const FxVec3& pos );
	// Returns the current position.
	const FxVec3& GetPos( void ) const;

	// Sets the current rotation.
	void SetRot( const FxQuat& rot );
	// Returns the current rotation.
	const FxQuat& GetRot( void ) const;

	// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	// FxTrue if the camera is "bound" to a bone.
	FxBool _isBoundToBone;
	// The name of the bone the camera is "bound" to, if any.
	FxName _boneName;
	// FxTrue if the camera should use a fixed aspect ratio.
	FxBool _usesFixedAspectRatio;
	// The width used in the fixed aspect ratio.
	FxReal _fixedAspectRatioWidth;
	// The height used in the fixed aspect ratio.
	FxReal _fixedAspectRatioHeight;
	// The rotation of the bone the camera is "bound" to, in world space, at the
	// time of the "bind."
	//@todo Note these are currently not yet serialized in 1.5.
	FxQuat _boneBindRot;
	FxVec3 _boneBindPos;
	// The "bind" position.  If the camera is "bound" to a bone, this value
	// is in "bone space."
	FxVec3 _bindPos;
	// The "bind" rotation.  This value is always the world space rotation
	// of the camera at creation time.
	FxQuat _bindRot;
	// The current world space camera position.
	FxVec3 _pos;
	// The current world space camera rotation.
	FxQuat _rot;
};

// Manages the list of engine-independent camera objects.
class FxCameraManager
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxCameraManager);
public:

	// Returns the number of cameras in the manager.
	static FxSize GetNumCameras( void );

	// Add a camera to the manager.
	static void AddCamera( const FxViewportCamera& camera );
	// Remove a camera from the manager.
	static void RemoveCamera( const FxName& cameraName );

	// Returns a pointer to the specified camera or NULL if the camera does not
	// exist.
	static FxViewportCamera* GetCamera( const FxName& cameraName );
	// Returns a pointer to the camera at index.
	static FxViewportCamera* GetCamera( FxSize index );

	// Removes all cameras from the manager.
	static void FlushAllCameras( void );

	// Serialization.
	static void Serialize( FxArchive& arc );

protected:
	// The viewport cameras.
	static FxArray<FxViewportCamera> _cameras;
};

} // namespace Face

} // namespace OC3Ent

#endif
