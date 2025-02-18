/*=============================================================================
	UnSkeletalMesh.h: Unreal skeletal mesh objects.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	USkeletalMeshComponent.
-----------------------------------------------------------------------------*/

#ifndef __UNSKELETALMESH_H__
#define __UNSKELETALMESH_H__

#if WITH_FACEFX
	#include "UnFaceFXSupport.h"
#endif // WITH_FACEFX

#include "GPUSkinPublicDefs.h"

// Define that controls showing chart of distance factors for skel meshes during entire run of the game on exit.
#define CHART_DISTANCE_FACTORS 0

//
//	FAttachment
//
struct FAttachment
{
	UActorComponent*	Component;
	FName				BoneName;
	FVector				RelativeLocation;
	FRotator			RelativeRotation;
	FVector				RelativeScale;

	// Constructor.

	FAttachment(UActorComponent* InComponent,FName InBoneName,FVector InRelativeLocation,FRotator InRelativeRotation,FVector InRelativeScale):
		Component(InComponent),
		BoneName(InBoneName),
		RelativeLocation(InRelativeLocation),
		RelativeRotation(InRelativeRotation),
		RelativeScale(InRelativeScale)
	{}
	FAttachment() {}
};
template <> struct TIsPODType<FAttachment> { enum { Value = true }; };

//
//	FBoneRotationControl
//
struct FBoneRotationControl
{
	INT			BoneIndex;
	FName		BoneName;
	FRotator	BoneRotation;
	BYTE		BoneSpace;

	// Constructor.
	FBoneRotationControl(INT InBoneIndex, FName InBoneName, FRotator InBoneRotation, BYTE InBoneSpace):
		BoneIndex(InBoneIndex),
		BoneName(InBoneName),
		BoneRotation(InBoneRotation),
		BoneSpace(InBoneSpace)
	{}
	FBoneRotationControl() {}
};
template <> struct TIsPODType<FBoneRotationControl> { enum { Value = true }; };

//
//	FBoneTranslationControl
//
struct FBoneTranslationControl
{
	INT			BoneIndex;
	FName		BoneName;
	FVector		BoneTranslation;

	FBoneTranslationControl(INT InBoneIndex,  FName InBoneName, const FVector& InBoneTranslation):
		BoneIndex(InBoneIndex),
		BoneName(InBoneName),
		BoneTranslation(InBoneTranslation)
	{}
	FBoneTranslationControl() {}
};
template <> struct TIsPODType<FBoneTranslationControl> { enum { Value = true }; };

/** Struct used to indicate one active morph target that should be applied to this SkeletalMesh when rendered. */
struct FActiveMorph
{
	/** The morph target that we want to apply. */
	class UMorphTarget*	Target;

	/** Strength of the morph target, between 0.0 and 1.0 */
	FLOAT				Weight;

	FActiveMorph(class UMorphTarget* InTarget, FLOAT InWeight):
		Target(InTarget),
		Weight(InWeight)
	{}
	FActiveMorph() {}
};
template <> struct TIsPODType<FActiveMorph> { enum { Value = true }; };

/** A pair of bone names */
struct FBonePair
{
	FName Bones[2];

	UBOOL operator==(const FBonePair& Other) const
	{
		return Bones[0] == Other.Bones[0] && Bones[1] == Other.Bones[1];
	}

	UBOOL IsMatch(const FBonePair& Other) const
	{
		return	(Bones[0] == Other.Bones[0] || Bones[1] == Other.Bones[0]) &&
				(Bones[0] == Other.Bones[1] || Bones[1] == Other.Bones[1]);
	}
};

enum ERootMotionMode
{
	RMM_Translate	= 0,
	RMM_Velocity	= 1,
	RMM_Ignore		= 2,
	RMM_Accel		= 3,
	RMM_Relative	= 4,
	RMM_MAX			= 5,
};

enum ERootMotionRotationMode
{
	RMRM_Ignore			= 0,
	RMRM_RotateActor	= 1,
	RMRM_MAX			= 2,
};

enum EFaceFXBlendMode
{
	FXBM_Overwrite	= 0,
	FXBM_Additive	= 1,
	FXBM_MAX		= 2,
};

enum EFaceFXRegOp
{
	FXRO_Add      = 0,	   
	FXRO_Multiply = 1, 
	FXRO_Replace  = 2,  
};

//
//	USkeletalMeshComponent
//
class USkeletalMesh;
class USkeletalMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(USkeletalMeshComponent,UMeshComponent,CLASS_NoExport,Engine)

	USkeletalMesh*						SkeletalMesh;

	/** If this component is attached to another SkeletalMeshComponent, this is the one it's attached to. */
	USkeletalMeshComponent*				AttachedToSkelComponent;

	class UAnimTree*					AnimTreeTemplate;
	class UAnimNode*					Animations;

	/** Array of all AnimNodes in entire tree, in the order they should be ticked - that is, all parents appear before a child. */
	TArray<UAnimNode*>					AnimTickArray;

	class UPhysicsAsset*				PhysicsAsset;
	class UPhysicsAssetInstance*		PhysicsAssetInstance;

	FLOAT								PhysicsWeight;

	/** Used to scale speed of all animations on this skeletal mesh. */
	FLOAT								GlobalAnimRateScale;

	class FSkeletalMeshObject*			MeshObject;
	FColor								WireframeColor;


	// State of bone bases - set up in local mesh space.
	TArray <FMatrix>					SpaceBases; 

	/** Temporary array of local-space (ie relative to parent bone) rotation/translation for each bone. */
	TArray <FBoneAtom>					LocalAtoms;

	/** Temporary array of bone indices required this frame. Filled in by UpdateSkelPose. */
	TArray <BYTE>						RequiredBones;

	USkeletalMeshComponent*				ParentAnimComponent;
	TArrayNoInit<INT>					ParentBoneMap;


	// Array of AnimSets used to find sequences by name.
	TArrayNoInit<class UAnimSet*>			AnimSets;

	/**
	 *  Temporary array of AnimSets that are used as a backup target when the engine needs to temporarily modify the
	 *	actor's animation set list. (e.g. Matinee playback)
	 */
	TArrayNoInit<class UAnimSet*> TemporarySavedAnimSets;


	/** 
	 *	Array of MorphTargetSets that will be looked in to find a particular MorphTarget, specified by name.
	 *	It is searched in the same way as the AnimSets array above.
	 */
	TArrayNoInit<class UMorphTargetSet*>	MorphSets;

	/** Array indicating all active MorphTargets. This array is updated inside UpdateSkelPose based on the AnimTree's st of MorphNodes. */
	TArrayNoInit<FActiveMorph>				ActiveMorphs;


	// Attachments.
	TArrayNoInit<FAttachment>				Attachments;

	/** 
	 *	Array of indices into the Animations->SkelControls array. The size of this array must be equal to the number
	 *	of bones in the skeleton. After the transform for bone index 'i' is calculated, if SkelControlIndex(i) is not 255,
	 *	it will be looked up in Animations->SkelControls and executed.
	 */
	TArrayNoInit<BYTE>						SkelControlIndex;

	// Editor/debugging rendering mode flags.

	/** Force drawing of a specific lodmodel -1 if > 0. */
	INT									ForcedLodModel;
	/** 
	 * This is the min LOD that this component will use.  (e.g. if set to 2 then only 2+ LOD Models will be used.) This is useful to set on
	 * meshes which are known to be a certain distance away and still want to have better LODs when zoomed in on them.
	 **/
	INT									MinLodModel;
	/** 
	 *	Best LOD that was 'predicted' by UpdateSkelPose. 
	 *	This is what bones were updated based on, so we do not allow rendering at a better LOD than this. 
	 */
	INT									PredictedLODLevel;

	/** LOD level from previous frame, so we can detect changes in LOD to recalc required bones. */
	INT									OldPredictedLODLevel;

	/**	High (best) DistanceFactor that was desired for rendering this SkeletalMesh last frame. Represents how big this mesh was in screen space   */
	FLOAT								MaxDistanceFactor;

	/** Forces the mesh to draw in wireframe mode. */
	UBOOL								bForceWireframe;

	/** If true, force the mesh into the reference pose - is an optimisation. */
	UBOOL								bForceRefpose;

	/** If bForceRefPose was set last tick. */
	UBOOL								bOldForceRefPose;

	/** Skip UpdateSkelPose. */
	UBOOL								bNoSkeletonUpdate;

	/** Draw the skeleton hierarchy for this skel mesh. */
	UBOOL								bDisplayBones;

	/** Bool that enables debug drawing of the skeleton before it is passed to the physics. Useful for debugging animation-driven physics. */
	UBOOL								bShowPrePhysBones;

	/** Don't bother rendering the skin. */
	UBOOL								bHideSkin;

	/** Forces ignoring of the mesh's offset. */
	UBOOL								bForceRawOffset;

	/** Ignore and bone rotation controllers */
	UBOOL								bIgnoreControllers;


	/** 
	 *	Set the LocalToWorld of this component to be the same as the ParentAnimComponent (if there is one). 
	 *	Note that this results in using RotOrigin/Origin from the ParentAnimComponent SkeletalMesh as well, as that is included in the LocalToWorld.
	 */
	UBOOL								bTransformFromAnimParent;

	/** Used to avoid ticking nodes in the tree multiple times. Node will only be ticked if TickTag != NodeTickTag. */
	INT									TickTag;

	/** 
	 *	Used to avoid duplicating work when calling GetBoneAtom. 
	 *	If this is equal to a nodes NodeCachedAtomsTag, cache is up-to-date and can be used. 
	 */
	INT									CachedAtomsTag;

	/** 
	 *	If true, create single rigid body physics for this component (like a static mesh) using root bone of PhysicsAsset. 
	 */
	UBOOL								bUseSingleBodyPhysics;

	/** If false, indicates that on the next call to UpdateSkelPose the RequiredBones array should be recalculated. */
	UBOOL								bRequiredBonesUpToDate;

	/** 
	 *	If non-zero, skeletal mesh component will not update kinematic bones and bone springs when distance factor is greater than this (or has not been rendered for a while).
	 *	This also turns off BlockRigidBody, so you do not get collisions with 'left behind' ragdoll setups.
	 */
	FLOAT								MinDistFactorForKinematicUpdate;

	/** When blending in physics, translate data from physics so that this bone aligns with its current graphics position. Can be useful for hacking 'frame behind' issues. */
	FName								PhysicsBlendZeroDriftBoneName;

	/** Used to keep track of how many frames physics has been asleep for (when using PHYS_RigidBody). */
	INT									FramesPhysicsAsleep;

	/** When true, if owned by a PHYS_RigidBody Actor, skip all update (bones and bounds) when physics are asleep. */
	BITFIELD bSkipAllUpdateWhenPhysicsAsleep:1 GCC_BITFIELD_MAGIC;

	/** When true, if owned by a PHYS_RigidBody Actor, skip all update (bones and bounds) when physics are asleep. */
	BITFIELD bConsiderAllBodiesForBounds:1;

	/** if true, update skeleton/attachments even when our Owner has not been rendered recently */
	BITFIELD bUpdateSkelWhenNotRendered:1;

	/** If true, do not apply any SkelControls when owner has not been rendered recently. */
	BITFIELD bIgnoreControllersWhenNotRendered:1;

	/** If this is true, we are not updating kinematic bones and motors based on animation beacause the skeletal mesh is too far from any viewer. */
	BITFIELD bNotUpdatingKinematicDueToDistance:1;

	/** force root motion to be discarded, no matter what the AnimNodeSequence(s) are set to do */
	BITFIELD bForceDiscardRootMotion:1;

	/** 
	 * if TRUE, notify owning actor of root motion mode changes.
	 * This calls the Actor.RootMotionModeChanged() event.
	 * This is useful for synchronizing movements. 
	 * For intance, when using RMM_Translate, and the event is called, we know that root motion will kick in on next frame.
	 * It is possible to kill in-game physics, and then use root motion seemlessly.
	 */
	BITFIELD bRootMotionModeChangeNotify:1;
	
	/**
	 * if TRUE, the event RootMotionExtracted() will be called on this owning actor,
	 * after root motion has been extracted, and before it's been used.
	 * This notification can be used to alter extracted root motion before it is forwarded to physics.
	 */
	BITFIELD bRootMotionExtractedNotify:1;

	/** If true, FaceFX will not automatically create material instances. */
	BITFIELD bDisableFaceFXMaterialInstanceCreation:1;

	/** If true, AnimTree has been initialised. */
	BITFIELD bAnimTreeInitialised:1;

	/** If TRUE, UpdateTransform will always result in a call to MeshObject->Update. */
	BITFIELD bForceMeshObjectUpdate:1;

	/** 
	 *	Indicates whether this SkeletalMeshComponent should have a physics engine representation of its state. 
	 *	@see SetHasPhysicsAssetInstance
	 */
	BITFIELD bHasPhysicsAssetInstance:1;

	/** If we are running physics, should we update bFixed bones based on the animation bone positions. */
	BITFIELD bUpdateKinematicBonesFromAnimation:1;

	/** 
	 *	If we should pass joint position to joints each frame, so that they can be used by motorised joints to drive the
	 *	ragdoll based on the animation.
	 */
	BITFIELD bUpdateJointsFromAnimation:1;

	/** Indicates whether this SkeletalMeshComponent is currently considered 'fixed' (ie kinematic) */
	BITFIELD bSkelCompFixed:1;

	/** Used for consistancy checking. Indicates that the results of physics have been blended into SpaceBases this frame. */
	BITFIELD bHasHadPhysicsBlendedIn:1;

	/** 
	 *	If true, attachments will be updated twice a frame - once in Tick and again when UpdateTransform is called. 
	 *	This can resolve some 'frame behind' issues if an attachment need to be in the correct location for it's Tick, but at a cost.
	 */
	BITFIELD bForceUpdateAttachmentsInTick:1;

	/** Enables blending in of physics bodies with the bAlwaysFullAnimWeight flag set. */
	BITFIELD bEnableFullAnimWeightBodies:1;

	/** 
	 *	If true, when this skeletal mesh overlaps a physics volume, each body of it will be tested against the volume, so only limbs 
	 *	actually in the volume will be affected. Useful when gibbing bodies.
	 */
	BITFIELD bPerBoneVolumeEffects:1;

	/** If true, will move the Actors Location to match the root rigid body location when in PHYS_RigidBody. */
	BITFIELD bSyncActorLocationToRootRigidBody:1;

	/** If TRUE, force usage of raw animation data when animating this skeltal mesh; if FALSE, use compressed data. */
	BITFIELD bUseRawData:1;

	/** Disable warning when an AnimSequence is not found. FALSE by default. */
	BITFIELD bDisableWarningWhenAnimNotFound:1;

	/** if set, components that are attached to us have their bOwnerNoSee and bOnlyOwnerSee properties overridden by ours */
	BITFIELD bOverrideAttachmentOwnerVisibility:1;

	/** pauses animations (doesn't tick them) */
	BITFIELD bPauseAnims:1;

	/** If true, DistanceFactor for this SkeletalMeshComponent will be added to global chart. */
	BITFIELD bChartDistanceFactor:1;

	/** If TRUE, line checks will test against the bounding box of this skeletal mesh component and return a hit if there is a collision. */
	BITFIELD bEnableLineCheckWithBounds:1;

	/** If bEnableLineCheckWithBounds is TRUE, scale the bounds by this value before doing line check. */
	FVector LineCheckBoundsScale;

	// CLOTH

	/** 
	 *	Whether cloth simulation should currently be used on this SkeletalMeshComponent.
	 *	@see SetEnableClothSimulation
	 */
	BITFIELD bEnableClothSimulation:1;

	/** Turns off all cloth collision so not checks are done (improves performance). */
	BITFIELD bDisableClothCollision:1;

	/** If true, cloth is 'frozen' and no simulation is taking place for it, though it will keep its shape. */
	BITFIELD bClothFrozen:1;

	/** If true, cloth will automatically have bClothFrozen set when it is not rendered, and have it turned off when it is seen. */
	BITFIELD bAutoFreezeClothWhenNotRendered:1;

	/** If true, cloth will be awake when a level is started, otherwise it will be instantly put to sleep. */
	BITFIELD bClothAwakeOnStartup:1;

	/** It true, clamp velocity of cloth particles to be within ClothOwnerVelClampRange of Base velocity. */
	BITFIELD bClothBaseVelClamp:1;

	/** If true, fixed verts of the cloth are attached in the physics to the physics body that this components actor is attached to. */
	BITFIELD bAttachClothVertsToBaseBody:1;

	/** Whether this cloth is on a non-animating static object. */
	BITFIELD bIsClothOnStaticObject:1;
	/** Whether we've updated fixed cloth verts since last attachment. */
	BITFIELD bUpdatedFixedClothVerts:1;

	/** TRUE if mesh has been recently rendered, FALSE otherwise */
	BITFIELD bRecentlyRendered:1;

	BITFIELD bCacheAnimSequenceNodes:1;
	BITFIELD bForceMeshObjectUpdates:1;

	/** If TRUE, update the instanced vertex influences for this mesh during the next update */
	BITFIELD bNeedsInstanceWeightUpdate:1;
	/** If TRUE, always use instanced vertex influences for this mesh */
	BITFIELD bAlwaysUseInstanceWeights:1;
	/** 
	* Set of bones which will be used to find vertices to switch to using instanced influence weights
	* instead of the default skeletal mesh weighting.
	*/
	TArrayNoInit<FBonePair> InstanceVertexWeightBones;	

	/** Constant force applied to all vertices in the cloth. */
	FVector ClothExternalForce;

	/** 'Wind' force applied to cloth. Force on each vertex is based on the dot product between the wind vector and the surface normal. */
	FVector	ClothWind;

	/** Amount of variance from base's velocity the cloth is allowed. */
	FVector	ClothBaseVelClampRange;

	/** How much to blend in results from cloth simulation with results from regular skinning. */
	FLOAT	ClothBlendWeight;

	/** Pointer to internal simulation object for cloth on this skeletal mesh. */
	FPointer ClothSim;

	/** Index of physics scene that this components cloth simulation is taking place in. */
	INT SceneIndex;

	/** Output vertex position data. Filled in by simulation engine when fetching results */
	TArray<FVector> ClothMeshPosData;

	/** Output vertex normal data. Filled in by simulation engine when fetching results */
	TArray<FVector> ClothMeshNormalData;

	/** Output index buffer. Filled in by simulation engine when fetching results */
	TArray<INT> 	ClothMeshIndexData;

	/** Output number of verts in cloth mesh. Filled in by simulation engine when fetching results */
	INT	NumClothMeshVerts;

	/** Output number of indices in buffer. Filled in by simulation engine when fetching results */
	INT	NumClothMeshIndices;


	/** Cloth parent indices contain the index of the original vertex when a vertex is created during tearing.
	If it is an original vertex then the parent index is the same as the vertex index. 
	*/
	TArray<INT>		ClothMeshParentData;

	/** Number of cloth parent indices provided by the physics SDK */
	INT				NumClothMeshParentIndices;


	/** Replacement Output vertex position data if welding needs to be used. Data is filled into ClothMeshPosData during rendering */
	TArray<FVector> ClothMeshWeldedPosData;

	/** Replacement Output vertex normal data if welding needs to be used. Data is filled into ClothMeshPosData during rendering */
	TArray<FVector> ClothMeshWeldedNormalData;

	/** Replacement  Output index buffer. Since tearing is not supported these do not change anyways*/
	TArray<INT> 	ClothMeshWeldedIndexData;

	INT ClothDirtyBufferFlag;

	/** Enum indicating what type of object this cloth should be considered for rigid body collision. */
	BYTE ClothRBChannel;

	/** Types of objects that this cloth will collide with. */
	FRBCollisionChannelContainer ClothRBCollideWithChannels;

	/** How much force to apply to cloth, in relation to the force applied to rigid bodies(zero applies no force to cloth, 1 applies the same) */
	FLOAT			ClothForceScale;

	/** Amount to scale impulses applied to cloth simulation. */ 
	FLOAT			ClothImpulseScale;

	/** 
     * The cloth tear factor for this SkeletalMeshComponent, negative values take the tear factor from the SkeletalMesh.
     * Note: UpdateClothParams() should be called after modification so that the changes are reflected in the simulation.
	 */
	FLOAT			ClothAttachmentTearFactor;

	/** If TRUE, cloth uses compartment in physics scene (usually with fixed timstep for better behaviour) */
	BITFIELD		bClothUseCompartment:1;

	/** If the distance traveled between frames exceeds this value the vertices will be reset to avoid stretching. */
	FLOAT MinDistanceForClothReset;
	FVector LastClothLocation;

	/** Pointer to the simulated NxSoftBody object. */
	FPointer						SoftBodySim;

    /** Index of the Novodex scene the soft-body resides in. */
	INT								SoftBodySceneIndex;

    /** Whether soft-body simulation should currently be used on this SkeletalMeshComponent. */
	BITFIELD						bEnableSoftBodySimulation:1;

    /** Buffer of the updated tetrahedron-vertex positions. */
	TArray<FVector>					SoftBodyTetraPosData;

    /** Buffer of the updated tetrahedron-indices. */
	TArray<INT>						SoftBodyTetraIndexData;

    /** Number of tetrahedron vertices of the soft-body mesh. */
	INT								NumSoftBodyTetraVerts;

    /** Number of tetrahedron indices of the soft-body mesh (equal to four times the number of tetrahedra). */
	INT								NumSoftBodyTetraIndices;

	/** Number of tetrahedron indices of the soft-body mesh (equal to four times the number of tetrahedra). */
	FLOAT							SoftBodyImpulseScale;

	/** If true, the soft-body is 'frozen' and no simulation is taking place for it, though it will keep its shape. */
	BITFIELD						bSoftBodyFrozen:1;

	/** If true, the soft-body will automatically have bSoftBodyFrozen set when it is not rendered, and have it turned off when it is seen. */
	BITFIELD						bAutoFreezeSoftBodyWhenNotRendered:1;

	/** If true, the soft-body will be awake when a level is started, otherwise it will be instantly put to sleep. */
	BITFIELD						bSoftBodyAwakeOnStartup:1;

	/** If TRUE, soft body uses compartment in physics scene (usually with fixed timstep for better behaviour) */
	BITFIELD						bSoftBodyUseCompartment:1;

    /** Enum indicating what type of object this soft-body should be considered for rigid body collision. */
	ERBCollisionChannel				SoftBodyRBChannel;

    /** Types of objects that this soft-body will collide with. */
	FRBCollisionChannelContainer	SoftBodyRBCollideWithChannels;

    /** Pointer to the Novodex plane-actor used when previewing the soft-body in the AnimSet Editor. */
	FPointer						SoftBodyASVPlane;


	/** For rendering physics limits. TODO remove! */
	UMaterialInterface*					LimitMaterial;
		
	/** Root Motion extracted from animation. */
	FBoneAtom	RootMotionDelta;
	/** Root Motion velocity */
	FVector		RootMotionVelocity;

	/** Root Bone offset */
	FVector		RootBoneTranslation;

	/** Scale applied in physics when RootMotionMode == RMM_Accel */
	FVector		RootMotionAccelScale;

	/** Determines whether motion should be applied immediately or... (uses ERootMotionMode) */
	BYTE	RootMotionMode;
	/** Previous Root Motion Mode, to catch changes */
	BYTE	PreviousRMM;
	BYTE	PendingRMM;
	BYTE	OldPendingRMM;
	INT		bRMMOneFrameDelay;
	/** Root Motion Rotation mode (uses ERootMotionRotationMode) */
	BYTE	RootMotionRotationMode;

	/** How should be blend FaceFX animations? */
	BYTE	FaceFXBlendMode;

#if WITH_FACEFX
	// The FaceFX actor instance associated with the skeletal mesh component.
	OC3Ent::Face::FxActorInstance* FaceFXActorInstance;
#else
	void* FaceFXActorInstance;
#endif

	/** 
	 *	The audio component that we are using to play audio for a facial animation. 
	 *	Assigned in PlayFaceFXAnim and cleared in StopFaceFXAnim.
	 */
	UAudioComponent* CachedFaceFXAudioComp;

	TArrayNoInit <BYTE>	BoneVisibility;

	// USkeletalMeshComponent interface
	void DeleteAnimTree();

	// FaceFX functions.
	void	UpdateFaceFX( TArray<FBoneAtom>& LocalTransforms, UBOOL bTickFaceFX );
	UBOOL	PlayFaceFXAnim(UFaceFXAnimSet* FaceFXAnimSetRef, const FString& AnimName, const FString& GroupName,class USoundCue* SoundCueToPlay);
	void	StopFaceFXAnim( void );
	UBOOL	IsPlayingFaceFXAnim();
	void	DeclareFaceFXRegister( const FString& RegName );
	FLOAT	GetFaceFXRegister( const FString& RegName );
	void	SetFaceFXRegister( const FString& RegName, FLOAT RegVal, BYTE RegOp, FLOAT InterpDuration );
	void	SetFaceFXRegisterEx( const FString& RegName, BYTE RegOp, FLOAT FirstValue, FLOAT FirstInterpDuration, FLOAT NextValue, FLOAT NextInterpDuration );

	/** Update the PredictedLODLevel and MaxDistanceFactor in the component from its MeshObject. */
	UBOOL UpdateLODStatus();

	void UpdateSkelPose(FLOAT DeltaTime = 0.f, UBOOL bTickFaceFX = TRUE);
	void ComposeSkeleton(TArray<FBoneAtom>& LocalTransforms, const TArray<BYTE>& RequiredBones);
	void UpdateActiveMorphs();
	void BuildComposePriorityList(TArray<BYTE>& PriorityList);
	void BlendPhysicsBones( TArray<BYTE>& RequiredBones, FLOAT PhysicsWeight );
	void BlendInPhysics();

	void RecalcRequiredBones(INT LODIndex);

	void SetSkeletalMesh(USkeletalMesh* InSkelMesh, UBOOL bKeepSpaceBases = FALSE, UBOOL InbAlwaysUseInstanceWeights=FALSE );
	void SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset, UBOOL bForceReInit = FALSE);
	void SetForceRefPose(UBOOL bNewForceRefPose);

	void SetParentAnimComponent(USkeletalMeshComponent* NewParentAnimComp);
	
	/**
	 *	Sets the value of the bForceWireframe flag and reattaches the component as necessary.
	 *
	 *	@param	InForceWireframe		New value of bForceWireframe.
	 */
	void SetForceWireframe(UBOOL InForceWireframe);

	/**
	 *	Set value of bHasPhysicsAssetInstance flag.
	 *	Will create/destroy PhysicsAssetInstance as desired.
	 */
	void SetHasPhysicsAssetInstance(UBOOL bHasInstance);

	/** Find a BodyInstance by BoneName */
	URB_BodyInstance* FindBodyInstanceNamed(FName BoneName);

	// Search through AnimSets to find an animation with the given name
	class UAnimSequence* FindAnimSequence(FName AnimSeqName);

	// Search through MorphSets to find a morph target with the given name
	class UMorphTarget* FindMorphTarget(FName MorphTargetName);
	class UMorphNodeBase* FindMorphNode(FName InNodeName);

	FMatrix	GetBoneMatrix(DWORD BoneIdx) const;
	
	// Controller Interface

	INT	MatchRefBone(FName StartBoneName) const;
	FName GetParentBone( FName BoneName ) const;

	/** 
	 * Returns bone name linked to a given named socket on the skeletal mesh component.
	 * If you're unsure to deal with sockets or bones names, you can use this function to filter through, and always return the bone name.
	 * @input	bone name or socket name
	 * @output	bone name
	 */
	FName GetSocketBoneName(FName InSocketName);

	FQuat	GetBoneQuaternion(FName BoneName, INT Space=0);
	FVector GetBoneLocation(FName BoneName, INT Space=0);
	FVector GetBoneAxis( FName BoneName, BYTE Axis );
	
	virtual void InitArticulated(UBOOL bFixed);
	virtual void TermArticulated(FRBPhysScene* Scene);


	void SetAnimTreeTemplate(UAnimTree* NewTemplate);

	void UpdateParentBoneMap();

	/** forces an update to the mesh's skeleton/attachments, even if bUpdateSkelWhenNotRendered is false and it has not been recently rendered
	* @note if bUpdateSkelWhenNotRendered is true, there is no reason to call this function (but doing so anyway will have no effect)
	*/
	void ForceSkelUpdate();

	/** 
	 * Force AnimTree to recache all animations.
	 * Call this when the AnimSets array has been changed.
	 */
	void UpdateAnimations();
	UBOOL GetBonesWithinRadius( const FVector& Origin, FLOAT Radius, DWORD TraceFlags, TArray<FName>& out_Bones );

	// SkelControls.

	FMatrix CalcComponentToFrameMatrix( INT BoneIndex, BYTE Space, FName OtherBoneName );

	void InitAnimTree(UBOOL bForceReInit=TRUE);
	void InitSkelControls();
	class UAnimNode*		FindAnimNode(FName InNodeName);
	class USkelControlBase* FindSkelControl(FName InControlName);
	void TickSkelControls(FLOAT DeltaSeconds);
	virtual UBOOL LegLineCheck(const FVector& Start, const FVector& End, FVector& HitLocation, FVector& HitNormal);

	// Attachment interface.
	void AttachComponent(UActorComponent* Component,FName BoneName,FVector RelativeLocation = FVector(0,0,0),FRotator RelativeRotation = FRotator(0,0,0),FVector RelativeScale = FVector(1,1,1));
	void DetachComponent(UActorComponent* Component);

	UBOOL GetSocketWorldLocationAndRotation(FName InSocketName, FVector& OutLocation, FRotator* OutRotation);
	void AttachComponentToSocket(UActorComponent* Component,FName SocketName);


	/**
	 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
	 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
	 * you have a component of interest but what you really want is some characteristic that you can use to track
	 * down where it came from.  
	 *
	 */
	virtual FString GetDetailedInfoInternal() const;

	/** if bOverrideAttachmentOwnerVisibility is true, overrides the owner visibility values in the specified attachment with our own
	 * @param Component the attached primitive whose settings to override
	 */
	void SetAttachmentOwnerVisibility(UActorComponent* Component);

	/** finds the closest bone to the given location
	 * @param TestLocation the location to test against
	 * @param BoneLocation (optional, out) if specified, set to the world space location of the bone that was found, or (0,0,0) if no bone was found
	 * @param IgnoreScale (optional) if specified, only bones with scaling larger than the specified factor are considered
	 * @return the name of the bone that was found, or 'None' if no bone was found
	 */
	FName FindClosestBone(FVector TestLocation, FVector* BoneLocation = NULL, FLOAT IgnoreScale = -1.0f);

	/** Calculate the up-to-date transform of the supplied SkeletalMeshComponent, which should be attached to this component. */
	FMatrix CalcAttachedSkelCompMatrix(const USkeletalMeshComponent* AttachedComp);

	/** 
	* Update the instanced vertex influences (weights/bones) 
	* Uses the cached list of bones to find vertices that need to use instanced influences
	* instead of the defaults from the skeletal mesh 
	*/
	void UpdateInstanceVertexWeights();

	/** 
	* Add a new bone to the list of instance vertex weight bones
	*
	* @param BoneNames - set of bones (implicitly parented) to use for finding vertices
	*/
	void AddInstanceVertexWeightBoneParented(FName BoneName, UBOOL bPairWithParent = TRUE);

	/** 
	* Remove a new bone to the list of instance vertex weight bones
	*
	* @param BoneNames - set of bones (implicitly parented) to use for finding vertices
	*/
	void RemoveInstanceVertexWeightBoneParented(FName BoneName);

	/** 
	* Find an existing bone pair entry in the list of InstanceVertexWeightBones
	*
	* @param BonePair - pair of bones to search for
	* @return index of entry found or -1 if not found
	*/
	INT FindInstanceVertexweightBonePair(const FBonePair& BonePair) const;
	
	/** 
	* Update the bones that specify which vertices will use instanced influences
	* This will also trigger an update of the vertex weights.
	*
	* @param BonePairs - set of bone pairs to use for finding vertices
	*/
	void UpdateInstanceVertexWeightBones( const TArray<FBonePair>& BonePairs );
	
	/**
	* Enabled or disable the instanced vertex weights buffer for the skeletal mesh object
	*
	* @param bEnable - TRUE to enable, FALSE to disable
	*/
	void ToggleInstanceVertexWeights( UBOOL bEnabled );

	/**
	* Verify FaceFX Bone indices match SkeletalMesh bone indices
	*/
	void DebugVerifyFaceFXBoneList();

	/**
	* Debug function that traces FaceFX bone list
	* to see if more than one mesh is referencing/relinking master bone list
	*/
	void TraceFaceFX(UBOOL bOutput = FALSE);

	// UActorComponent interface.
protected:
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void UpdateChildComponents();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void BeginPlay();
	virtual void Tick(FLOAT DeltaTime);

	/**
	 * Called to finish destroying the object.  After UObject::FinishDestroy is called, the object's memory should no longer be accessed.
	 */
	virtual void FinishDestroy();

	/** Internal function that updates physics objects to match the RBChannel/RBCollidesWithChannel info. */
	virtual void UpdatePhysicsToRBChannels();
public:
	void TickAnimNodes(FLOAT DeltaTime);

	virtual void InitComponentRBPhys(UBOOL bFixed);
	virtual void SetComponentRBFixed(UBOOL bFixed);
	virtual void TermComponentRBPhys(FRBPhysScene* InScene);
	virtual class URB_BodySetup* GetRBBodySetup();

	/** Initialize physx cloth support. */
	void InitClothSim(FRBPhysScene* Scene);

	/** Destructs and frees cloth support. */
	void TermClothSim(FRBPhysScene* Scene);

	/** Initialize breakable cloth vertices. */
	void InitClothBreakableAttachments();

	/** Initialize "metal" cloth support... stiff cloth that is associated with rigid-bodies to simulate denting. */
	void InitClothMetal();

	/** Initialize physx soft-body support. */
	void InitSoftBodySim(FRBPhysScene* Scene, UBOOL bRunsInAnimSetViewer = false);

	/** Destructs and frees soft-body support. */
	void TermSoftBodySim(FRBPhysScene* Scene);

	/** Updates soft-body parameters from the UActor to the simulation. */
	void UpdateSoftBodyParams();

	/** Initialize soft-body to rigid-body attach points. */
	void InitSoftBodyAttachments();

	/** Freeze or unfreeze soft-body simulation. */
	void SetSoftBodyFrozen(UBOOL bNewFrozen);

	/** Force awake any soft body simulation on this component */
	void WakeSoftBody();

	/** Inialize internal soft-body memory buffers. */
	void InitSoftBodySimBuffers();

	virtual UBOOL IsValidComponent() const;

	// UPrimitiveComponent interface.
	virtual void SetTransformedToWorld();

	virtual void GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const;

	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);

	virtual void UpdateBounds();
	void UpdateClothBounds();

	virtual void AddImpulse(FVector Impulse, FVector Position = FVector(0,0,0), FName BoneName = NAME_None, UBOOL bVelChange=false);
	virtual void AddRadialImpulse(const FVector& Origin, FLOAT Radius, FLOAT Strength, BYTE Falloff, UBOOL bVelChange=false);
	virtual void AddRadialForce(const FVector& Origin, FLOAT Radius, FLOAT Strength, BYTE Falloff);
	virtual void AddForceField(FForceApplicator* Applicator, const FBox& FieldBoundingBox, UBOOL bApplyToCloth, UBOOL bApplyToRigidBody);
	virtual void WakeRigidBody( FName BoneName = NAME_None );
	virtual void PutRigidBodyToSleep( FName BoneName = NAME_None );
	virtual UBOOL RigidBodyIsAwake( FName BoneName = NAME_None );
	virtual void SetRBLinearVelocity(const FVector& NewVel, UBOOL bAddToCurrent=false);
	virtual void SetRBAngularVelocity(const FVector& NewVel, UBOOL bAddToCurrent=false);
	virtual void RetardRBLinearVelocity(const FVector& RetardDir, FLOAT VelScale);
	virtual void SetRBPosition(const FVector& NewPos, FName BoneName = NAME_None);
	virtual void SetRBRotation(const FRotator& NewRot, FName BoneName = NAME_None);
	virtual void SetBlockRigidBody(UBOOL bNewBlockRigidBody);
	virtual void SetNotifyRigidBodyCollision(UBOOL bNewNotifyRigidBodyCollision);
	virtual void SetPhysMaterialOverride(UPhysicalMaterial* NewPhysMaterial);
	virtual URB_BodyInstance* GetRootBodyInstance();

	/** 
	 *	Used for creating one-way physics interactions.
	 *	@see RBDominanceGroup
	 */
	virtual void SetRBDominanceGroup(BYTE InDomGroup);

	/** Utility for calculating the current LocalToWorld matrix of this SkelMeshComp, given its parent transform. */
	FMatrix CalcCurrentLocalToWorld(const FMatrix& ParentMatrix);

	void UpdateRBBonesFromSpaceBases(const FMatrix& CompLocalToWorld, UBOOL bMoveUnfixedBodies, UBOOL bTeleport);
	void UpdateRBJointMotors();
	void UpdateFixedClothVerts();

	/** Update forces applied to each cloth particle based on the ClothWind parameter. */
	void UpdateClothWindForces(FLOAT DeltaSeconds);

	/** Move all vertices in the cloth to the reference pose and zero their velocity. */
	void ResetClothVertsToRefPose();

	/**
	* Looks up all bodies for broken constraints.
	* Makes sure child bodies of a broken constraints are not fixed and using bone springs, and child joints not motorized.
	*/
	void UpdateMeshForBrokenConstraints();

	//Some get*() APIs
	FLOAT GetClothAttachmentResponseCoefficient();
	FLOAT GetClothAttachmentTearFactor();
	FLOAT GetClothBendingStiffness();
	FLOAT GetClothCollisionResponseCoefficient();
	FLOAT GetClothDampingCoefficient();
	INT GetClothFlags();
	FLOAT GetClothFriction();
	FLOAT GetClothPressure();
	FLOAT GetClothSleepLinearVelocity();
	INT GetClothSolverIterations();
	FLOAT GetClothStretchingStiffness();
	FLOAT GetClothTearFactor();
	FLOAT GetClothThickness();
	//some set*() APIs
	void SetClothAttachmentResponseCoefficient(FLOAT ClothAttachmentResponseCoefficient);
	void SetClothAttachmentTearFactor(FLOAT ClothAttachmentTearFactor);
	void SetClothBendingStiffness(FLOAT ClothBendingStiffness);
	void SetClothCollisionResponseCoefficient(FLOAT ClothCollisionResponseCoefficient);
	void SetClothDampingCoefficient(FLOAT ClothDampingCoefficient);
	void SetClothFlags(INT ClothFlags);
	void SetClothFriction(FLOAT ClothFriction);
	void SetClothPressure(FLOAT ClothPressure);
	void SetClothSleepLinearVelocity(FLOAT ClothSleepLinearVelocity);
	void SetClothSolverIterations(INT ClothSolverIterations);
	void SetClothStretchingStiffness(FLOAT ClothStretchingStiffness);
	void SetClothTearFactor(FLOAT ClothTearFactor);
	void SetClothThickness(FLOAT ClothThickness);
	//Other APIs
	void SetClothSleep(UBOOL IfClothSleep);
	void SetClothPosition(const FVector& ClothOffSet);
	void SetClothVelocity(const FVector& VelocityOffSet);
	//Attachment API
	void AttachClothToCollidingShapes(UBOOL AttatchTwoWay, UBOOL AttachTearable);
	//ValidBounds APIs
	void EnableClothValidBounds(UBOOL IfEnableClothValidBounds);
	void SetClothValidBounds(const FVector& ClothValidBoundsMin, const FVector& ClothValidBoundsMax);

	virtual void UpdateRBKinematicData();
	void SetEnableClothSimulation(UBOOL bInEnable);

	/** Toggle active simulation of cloth. Cheaper than doing SetEnableClothSimulation, and keeps its shape while frozen. */
	void SetClothFrozen(UBOOL bNewFrozen);

	void UpdateClothParams();
	void SetClothExternalForce(const FVector& InForce);

	/** Attach/detach verts from physics body that this components actor is attached to. */
	void SetAttachClothVertsToBaseBody(UBOOL bAttachVerts);

	/**
	 * Saves the skeletal component's current AnimSets to a temporary buffer.  You can restore them later by calling
	 * RestoreSavedAnimSets().  This is the C++ version of the method.  The script version just calls this one.
	 */
	void SaveAnimSets();

	/**
	 * Restores saved AnimSets to the master list of AnimSets and clears the temporary saved list of AnimSets.  This
	 * is the C++ version of the method.  The script version just calls this one.
	 */
	void RestoreSavedAnimSets();

	virtual class FDecalRenderData* GenerateDecalRenderData(class FDecalState* Decal) const;

	/**
	 * Transforms the specified decal info into reference pose space.
	 *
	 * @param	Decal			Info of decal to transform.
	 * @param	BoneIndex		The index of the bone hit by the decal.
	 * @return					A reference to the transformed decal info, or NULL if the operation failed.
	 */
	FDecalState* TransformDecalToRefPoseSpace(FDecalState* Decal, INT BoneIndex) const;

	/** 
	* @return TRUE if the primitive component can render decals
	*/
	virtual UBOOL SupportsDecalRendering() const;

	virtual FPrimitiveSceneProxy* CreateSceneProxy();	

#if WITH_NOVODEX
	virtual class NxActor* GetNxActor(FName BoneName = NAME_None);
	virtual class NxActor* GetIndexedNxActor(INT BodyIndex = INDEX_NONE);

	/** Utility for getting all physics bodies contained within this component. */
	virtual void GetAllNxActors(TArray<class NxActor*>& OutActors);

	virtual FVector NxGetPointVelocity(FVector LocationInWorldSpace);
#endif // WITH_NOVODEX

	void HideBone( INT BoneIndex, UBOOL bTermPhysicsBelow );
	void UnHideBone( INT BoneIndex );
	UBOOL IsBoneHidden( INT BoneIndex );

	virtual FKCachedConvexData* GetBoneCachedPhysConvexData(const FVector& InScale3D, const FName& BoneName);

	// UMeshComponent interface.

	virtual UMaterialInterface* GetMaterial(INT MaterialIndex) const;
	virtual INT GetNumElements() const;

	/**
	 * Called by AnimNotify_PlayParticleEffect
	 * Looks for a socket name first then bone name
 	 *
	 * @param AnimNotifyData The AnimNotify_PlayParticleEffect which will have all of the various params on it
	 */
    void eventPlayParticleEffect(const class UAnimNotify_PlayParticleEffect* AnimNotifyData)
    {
        Actor_eventPlayParticleEffect_Parms Parms(EC_EventParm);
        Parms.AnimNotifyData=AnimNotifyData;
        ProcessEvent(FindFunctionChecked(ENGINE_PlayParticleEffect),&Parms);
    }

	// Script functions.

	DECLARE_FUNCTION(execAttachComponent);
	DECLARE_FUNCTION(execDetachComponent);
	DECLARE_FUNCTION(execAttachComponentToSocket);
	DECLARE_FUNCTION(execGetSocketWorldLocationAndRotation);
	DECLARE_FUNCTION(execGetSocketByName);
	DECLARE_FUNCTION(execGetSocketBoneName);
	DECLARE_FUNCTION(execFindComponentAttachedToBone);
	DECLARE_FUNCTION(execIsComponentAttached);
	DECLARE_FUNCTION(execAttachedComponents);
	DECLARE_FUNCTION(execSetSkeletalMesh);
	DECLARE_FUNCTION(execSetPhysicsAsset);
	DECLARE_FUNCTION(execSetForceRefPose);
	DECLARE_FUNCTION(execSetParentAnimComponent);
	DECLARE_FUNCTION(execFindAnimSequence);
	DECLARE_FUNCTION(execFindMorphTarget);
	DECLARE_FUNCTION(execGetBoneQuaternion);
	DECLARE_FUNCTION(execGetBoneLocation);
	DECLARE_FUNCTION(execGetBoneAxis);
	DECLARE_FUNCTION(execTransformToBoneSpace);
	DECLARE_FUNCTION(execTransformFromBoneSpace);
	DECLARE_FUNCTION(execFindClosestBone);
	DECLARE_FUNCTION(execGetClosestCollidingBoneLocation);
	DECLARE_FUNCTION(execSetAnimTreeTemplate);
	DECLARE_FUNCTION(execUpdateParentBoneMap);
	DECLARE_FUNCTION(execInitSkelControls);
	DECLARE_FUNCTION(execFindAnimNode);
	DECLARE_FUNCTION(execAllAnimNodes);
	DECLARE_FUNCTION(execFindSkelControl);
	DECLARE_FUNCTION(execFindMorphNode);
	DECLARE_FUNCTION(execFindConstraintIndex);
	DECLARE_FUNCTION(execFindConstraintBoneName);
	DECLARE_FUNCTION(execFindBodyInstanceNamed);
	DECLARE_FUNCTION(execForceSkelUpdate);
	DECLARE_FUNCTION(execUpdateAnimations);
	DECLARE_FUNCTION(execGetBonesWithinRadius);
	DECLARE_FUNCTION(execAddInstanceVertexWeightBoneParented);
	DECLARE_FUNCTION(execRemoveInstanceVertexWeightBoneParented);
	DECLARE_FUNCTION(execFindInstanceVertexweightBonePair);
	DECLARE_FUNCTION(execUpdateInstanceVertexWeightBones);
	DECLARE_FUNCTION(execToggleInstanceVertexWeights);
	DECLARE_FUNCTION(execSetHasPhysicsAssetInstance);
	DECLARE_FUNCTION(execUpdateRBBonesFromSpaceBases);
	DECLARE_FUNCTION(execPlayFaceFXAnim);
	DECLARE_FUNCTION(execStopFaceFXAnim);
	DECLARE_FUNCTION(execIsPlayingFaceFXAnim);
	DECLARE_FUNCTION(execDeclareFaceFXRegister);
	DECLARE_FUNCTION(execGetFaceFXRegister);
	DECLARE_FUNCTION(execSetFaceFXRegister);
	DECLARE_FUNCTION(execSetFaceFXRegisterEx);
	DECLARE_FUNCTION(execSetEnableClothSimulation);
	DECLARE_FUNCTION(execSetClothFrozen);
	DECLARE_FUNCTION(execUpdateClothParams);
	DECLARE_FUNCTION(execSetClothExternalForce);
	DECLARE_FUNCTION(execSetAttachClothVertsToBaseBody);
	DECLARE_FUNCTION(execResetClothVertsToRefPose);
	DECLARE_FUNCTION(execUpdateMeshForBrokenConstraints);

	

	//Some get*() APIs
	DECLARE_FUNCTION(execGetClothAttachmentResponseCoefficient);
	DECLARE_FUNCTION(execGetClothAttachmentTearFactor);
	DECLARE_FUNCTION(execGetClothBendingStiffness);
	DECLARE_FUNCTION(execGetClothCollisionResponseCoefficient);
	DECLARE_FUNCTION(execGetClothDampingCoefficient);
	DECLARE_FUNCTION(execGetClothFlags);
	DECLARE_FUNCTION(execGetClothFriction);
	DECLARE_FUNCTION(execGetClothPressure);
	DECLARE_FUNCTION(execGetClothSleepLinearVelocity);
	DECLARE_FUNCTION(execGetClothSolverIterations);
	DECLARE_FUNCTION(execGetClothStretchingStiffness);
	DECLARE_FUNCTION(execGetClothTearFactor);
	DECLARE_FUNCTION(execGetClothThickness);
	//some set*() APIs
	DECLARE_FUNCTION(execSetClothAttachmentResponseCoefficient);
	DECLARE_FUNCTION(execSetClothAttachmentTearFactor);
	DECLARE_FUNCTION(execSetClothBendingStiffness);
	DECLARE_FUNCTION(execSetClothCollisionResponseCoefficient);
	DECLARE_FUNCTION(execSetClothDampingCoefficient);
	DECLARE_FUNCTION(execSetClothFlags);
	DECLARE_FUNCTION(execSetClothFriction);
	DECLARE_FUNCTION(execSetClothPressure);
	DECLARE_FUNCTION(execSetClothSleepLinearVelocity);
	DECLARE_FUNCTION(execSetClothSolverIterations);
	DECLARE_FUNCTION(execSetClothStretchingStiffness);
	DECLARE_FUNCTION(execSetClothTearFactor);
	DECLARE_FUNCTION(execSetClothThickness);
	//Other APIs
	DECLARE_FUNCTION(execSetClothSleep);
	DECLARE_FUNCTION(execSetClothPosition);
	DECLARE_FUNCTION(execSetClothVelocity);
	//Attachment API
	DECLARE_FUNCTION(execAttachClothToCollidingShapes);
	//ValidBounds APIs
	DECLARE_FUNCTION(execEnableClothValidBounds);
	DECLARE_FUNCTION(execSetClothValidBounds);

	DECLARE_FUNCTION(execSaveAnimSets);
	DECLARE_FUNCTION(execRestoreSavedAnimSets);
	DECLARE_FUNCTION(execGetBoneMatrix);
	DECLARE_FUNCTION(execMatchRefBone);
	DECLARE_FUNCTION(execGetBoneName);
	DECLARE_FUNCTION(execGetParentBone);
	DECLARE_FUNCTION(execGetBoneNames);
	DECLARE_FUNCTION(execBoneIsChildOf);
	DECLARE_FUNCTION(execGetRefPosePosition);

	DECLARE_FUNCTION(execUpdateSoftBodyParams);
	DECLARE_FUNCTION(execSetSoftBodyFrozen);
	DECLARE_FUNCTION(execWakeSoftBody);

	DECLARE_FUNCTION(execHideBone);
	DECLARE_FUNCTION(execUnHideBone);
	DECLARE_FUNCTION(execIsBoneHidden);
};


/*-----------------------------------------------------------------------------
	USkeletalMesh.
-----------------------------------------------------------------------------*/

struct FMeshWedge
{
	WORD			iVertex;		// Vertex index.
	FLOAT			U,V;			// UVs.
	friend FArchive &operator<<( FArchive& Ar, FMeshWedge& T )
	{
		Ar << T.iVertex << T.U << T.V;
		return Ar;
	}
};
template <> struct TIsPODType<FMeshWedge> { enum { Value = true }; };

struct FMeshFace
{
	WORD		iWedge[3];			// Textured Vertex indices.
	WORD		MeshMaterialIndex;	// Source Material (= texture plus unique flags) index.

	friend FArchive &operator<<( FArchive& Ar, FMeshFace& F )
	{
		Ar << F.iWedge[0] << F.iWedge[1] << F.iWedge[2];
		Ar << F.MeshMaterialIndex;
		return Ar;
	}
};
template <> struct TIsPODType<FMeshFace> { enum { Value = true }; };

// A bone: an orientation, and a position, all relative to their parent.
struct VJointPos
{
	FQuat   	Orientation;  //
	FVector		Position;     //  needed or not ?

	FLOAT       Length;       //  For collision testing / debugging drawing...
	FLOAT       XSize;
	FLOAT       YSize;
	FLOAT       ZSize;

	friend FArchive &operator<<( FArchive& Ar, VJointPos& V )
	{
		return Ar << V.Orientation << V.Position;
	}
};
template <> struct TIsPODType<VJointPos> { enum { Value = true }; };

// Reference-skeleton bone, the package-serializable version.
struct FMeshBone
{
	FName 		Name;		  // Bone's name.
	DWORD		Flags;        // reserved
	VJointPos	BonePos;      // reference position
	INT         ParentIndex;  // 0/NULL if this is the root bone.  
	INT 		NumChildren;  // children  // only needed in animation ?
	INT         Depth;        // Number of steps to root in the skeletal hierarcy; root=0.

	// DEBUG rendering
	FColor		BoneColor;		// Color to use when drawing bone on screen.

	UBOOL operator==( const FMeshBone& B ) const
	{
		return( Name == B.Name );
	}
	
	friend FArchive &operator<<( FArchive& Ar, FMeshBone& F)
	{
		Ar << F.Name << F.Flags << F.BonePos << F.NumChildren << F.ParentIndex;

		if( Ar.IsLoading() && Ar.Ver() < VER_SKELMESH_DRAWSKELTREEMANAGER )
		{
			F.BoneColor = FColor(255, 255, 255, 255);
		}
		else
		{
			Ar << F.BoneColor;
		}

		return Ar;
	}
};
template <> struct TIsPODType<FMeshBone> { enum { Value = true }; };

// Textured triangle.
struct VTriangle
{
	WORD   WedgeIndex[3];	 // Point to three vertices in the vertex list.
	BYTE    MatIndex;	     // Materials can be anything.
	BYTE    AuxMatIndex;     // Second material from exporter (unused)
	DWORD   SmoothingGroups; // 32-bit flag for smoothing groups.

	friend FArchive &operator<<( FArchive& Ar, VTriangle& V )
	{
		Ar << V.WedgeIndex[0] << V.WedgeIndex[1] << V.WedgeIndex[2];
		Ar << V.MatIndex << V.AuxMatIndex;
		Ar << V.SmoothingGroups;
		return Ar;
	}

	VTriangle& operator=( const VTriangle& Other)
	{
		this->AuxMatIndex = Other.AuxMatIndex;
		this->MatIndex        =  Other.MatIndex;
		this->SmoothingGroups =  Other.SmoothingGroups;
		this->WedgeIndex[0]   =  Other.WedgeIndex[0];
		this->WedgeIndex[1]   =  Other.WedgeIndex[1];
		this->WedgeIndex[2]   =  Other.WedgeIndex[2];
		return *this;
	}
};
template <> struct TIsPODType<VTriangle> { enum { Value = true }; };

struct FVertInfluence 
{
	FLOAT Weight;
	WORD VertIndex;
	WORD BoneIndex;
	friend FArchive &operator<<( FArchive& Ar, FVertInfluence& F )
	{
		return Ar << F.Weight << F.VertIndex << F.BoneIndex;
	}
};
template <> struct TIsPODType<FVertInfluence> { enum { Value = true }; };

/**
* Data needed for importing an extra set of vertex influences
*/
struct FSkelMeshExtraInfluenceImportData
{
	TArray<FMeshBone> RefSkeleton;
	TArray<FVertInfluence> Influences;
	TArray<FMeshWedge> Wedges;
	TArray<FMeshFace> Faces;
	TArray<FVector> Points;
};

//
//	FSoftSkinVertex
//

struct FSoftSkinVertex
{
	FVector			Position;
	FPackedNormal	TangentX,	// Tangent, U-direction
					TangentY,	// Binormal, V-direction
					TangentZ;	// Normal
	FLOAT			U,
					V;
	BYTE			InfluenceBones[MAX_INFLUENCES];
	BYTE			InfluenceWeights[MAX_INFLUENCES];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,FSoftSkinVertex& V);
};

//
//	FRigidSkinVertex
//

struct FRigidSkinVertex
{
	FVector			Position;
	FPackedNormal	TangentX,	// Tangent, U-direction
					TangentY,	// Binormal, V-direction
					TangentZ;	// Normal
	FLOAT			U,
					V;
	BYTE			Bone;

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,FRigidSkinVertex& V);
};

/**
 * A set of the skeletal mesh vertices which use the same set of <MAX_GPUSKIN_BONES bones.
 * In practice, there is a 1:1 mapping between chunks and sections, but for meshes which
 * were imported before chunking was implemented, there will be a single chunk for all
 * sections.
 */
struct FSkelMeshChunk
{
	/** The offset into the LOD's vertex buffer of this chunk's vertices. */
	UINT BaseVertexIndex;

	/** The rigid vertices of this chunk. */
	TArray<FRigidSkinVertex> RigidVertices;

	/** The soft vertices of this chunk. */
	TArray<FSoftSkinVertex> SoftVertices;

	/** The bones which are used by the vertices of this chunk. */
	TArray<WORD> BoneMap;

	/** The number of rigid vertices in this chunk */
	INT NumRigidVertices;
	/** The number of soft vertices in this chunk */
	INT NumSoftVertices;

	/** max # of bones used to skin the vertices in this chunk */
	INT MaxBoneInfluences;

	/**
	* @return total num rigid verts for this chunk
	*/
	FORCEINLINE const INT GetNumRigidVertices() const
	{
		return NumRigidVertices;
	}

	/**
	* @return total num soft verts for this chunk
	*/
	FORCEINLINE const INT GetNumSoftVertices() const
	{
		return NumSoftVertices;
	}

	/**
	* @return total number of soft and rigid verts for this chunk
	*/
	FORCEINLINE const INT GetNumVertices() const
	{
		return GetNumRigidVertices() + GetNumSoftVertices();
	}

	/**
	* @return starting index for rigid verts for this chunk in the LOD vertex buffer
	*/
	FORCEINLINE const INT GetRigidVertexBufferIndex() const
	{
        return BaseVertexIndex;
	}

	/**
	* @return starting index for soft verts for this chunk in the LOD vertex buffer
	*/
	FORCEINLINE const INT GetSoftVertexBufferIndex() const
	{
        return BaseVertexIndex + NumRigidVertices;
	}

	/**
	* Calculate max # of bone influences used by this skel mesh chunk
	*/
	void CalcMaxBoneInfluences();

	/**
	* Serialize this class
	* @param Ar - archive to serialize to
	* @param C - skel mesh chunk to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,FSkelMeshChunk& C)
	{
		Ar << C.BaseVertexIndex;
		Ar << C.RigidVertices;
		Ar << C.SoftVertices;
		Ar << C.BoneMap;
		Ar << C.NumRigidVertices;
		Ar << C.NumSoftVertices;
		Ar << C.MaxBoneInfluences;
		return Ar;
	}
};

/**
 * A set of skeletal mesh triangles which use the same material and chunk.
 */
struct FSkelMeshSection
{
	/** Material (texture) used for this section. */
	WORD MaterialIndex;

	/** The chunk that vertices for this section are from. */
	WORD ChunkIndex;

	/** The offset of this section's indices in the LOD's index buffer. */
	DWORD BaseIndex;

	/** The number of triangles in this section. */
	WORD NumTriangles;

	// Serialization.
	friend FArchive& operator<<(FArchive& Ar,FSkelMeshSection& S)
	{		
		Ar << S.MaterialIndex;
		Ar << S.ChunkIndex;
		Ar << S.BaseIndex;
		Ar << S.NumTriangles;
		return Ar;
	}	
};
template <> struct TIsPODType<FSkelMeshSection> { enum { Value = true }; };

/**
* Base vertex data for GPU skinned skeletal meshes
*/
struct FGPUSkinVertexBase
{
	FVector			Position;
	FPackedNormal	TangentX,	// Tangent, U-direction
					TangentZ;	// Normal	
	BYTE			InfluenceBones[MAX_INFLUENCES];
	BYTE			InfluenceWeights[MAX_INFLUENCES];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	*/
	void Serialize(FArchive& Ar);
};

/** 
* 16 bit UV version of skeletal mesh vertex
*/
template<UINT NumTexCoords=1>
struct TGPUSkinVertexFloat16Uvs : public FGPUSkinVertexBase
{
	/** half float UVs */
	FVector2DHalf UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TGPUSkinVertexFloat16Uvs& V)
	{
		V.Serialize(Ar);
		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << V.UVs[UVIndex];
		}
		return Ar;
	}
};

/** 
* 32 bit UV version of skeletal mesh vertex
*/
template<UINT NumTexCoords=1>
struct TGPUSkinVertexFloat32Uvs : public FGPUSkinVertexBase
{
	/** full float UVs */
	FVector2D UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TGPUSkinVertexFloat32Uvs& V)
	{
		V.Serialize(Ar);
		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << V.UVs[UVIndex];
		}
		return Ar;
	}
};

/** An interface to the skel-mesh vertex data storage type. */
class FSkeletalMeshVertexDataInterface
{
public:

	/** Virtual destructor. */
	virtual ~FSkeletalMeshVertexDataInterface() {}

	/**
	* Resizes the vertex data buffer, discarding any data which no longer fits.
	* @param NumVertices - The number of vertices to allocate the buffer for.
	*/
	virtual void ResizeBuffer(UINT NumVertices) = 0;

	/** @return The stride of the vertex data in the buffer. */
	virtual UINT GetStride() const = 0;

	/** @return A pointer to the data in the buffer. */
	virtual BYTE* GetDataPointer() = 0;

	/** @return number of vertices in the buffer */
	virtual UINT GetNumVertices() = 0;

	/** @return A pointer to the FResourceArrayInterface for the vertex data. */
	virtual FResourceArrayInterface* GetResourceArray() = 0;

	/** Serializer. */
	virtual void Serialize(FArchive& Ar) = 0;
};

/** 
* Vertex buffer with static lod chunk vertices for use with GPU skinning 
*/
class FSkeletalMeshVertexBuffer : public FVertexBuffer
{
public:
	/**
	* Constructor
	*/
	FSkeletalMeshVertexBuffer();

	/**
	* Destructor
	*/
	virtual ~FSkeletalMeshVertexBuffer();

	/**
	* Assignment. Assumes that vertex buffer will be rebuilt 
	*/
	FSkeletalMeshVertexBuffer& operator=(const FSkeletalMeshVertexBuffer& Other);
	/**
	* Constructor (copy)
	*/
	FSkeletalMeshVertexBuffer(const FSkeletalMeshVertexBuffer& Other);

	/** 
	* Delete existing resources 
	*/
	void CleanUp();

	/**
	* Initializes the buffer with the given vertices.
	* @param InVertices - The vertices to initialize the buffer with.
	*/
	void Init(const TArray<FSoftSkinVertex>& InVertices);

	/**
	* Serializer for this class
	* @param Ar - archive to serialize to
	* @param B - data to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,FSkeletalMeshVertexBuffer& VertexBuffer);

	// FRenderResource interface.

	/**
	* Initialize the RHI resource for this vertex buffer
	*/
	virtual void InitRHI();

	/**
	* @return text description for the resource type
	*/
	virtual FString GetFriendlyName() const;

	// Vertex data accessors.

	/** 
	* Const access to entry in vertex data array
	*
	* @param VertexIndex - index into the vertex buffer
	* @return pointer to vertex data cast to base vertex type
	*/
	FORCEINLINE const FGPUSkinVertexBase* GetVertexPtr(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return (FGPUSkinVertexBase*)(Data + VertexIndex * Stride);
	}
	/** 
	* Non=Const access to entry in vertex data array
	*
	* @param VertexIndex - index into the vertex buffer
	* @return pointer to vertex data cast to base vertex type
	*/
	FORCEINLINE FGPUSkinVertexBase* GetVertexPtr(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return (FGPUSkinVertexBase*)(Data + VertexIndex * Stride);
	}
	/**
	* Fet the vertex UV values at the given index in the vertex buffer
	*
	* @param VertexIndex - index into the vertex buffer
	* @param UVIndex - [0,MAX_TEXCOORDS] value to index into UVs array
	* @return 2D UV values
	*/
	FORCEINLINE FVector2D GetVertexUV(UINT VertexIndex,UINT UVIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		if( !bUseFullPrecisionUVs )
		{
			return ((TGPUSkinVertexFloat16Uvs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
		}
		else
		{
			return ((TGPUSkinVertexFloat32Uvs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
		}		
	}

	// Other accessors.

	/** 
	* @return TRUE if using 32 bit floats for UVs 
	*/
	FORCEINLINE UBOOL GetUseFullPrecisionUVs() const
	{
		return bUseFullPrecisionUVs;
	}
	/** 
	* @param UseFull - set to TRUE if using 32 bit floats for UVs 
	*/
	FORCEINLINE void SetUseFullPrecisionUVs(UBOOL UseFull)
	{
		bUseFullPrecisionUVs = UseFull;
	}
	/** 
	* @return number of vertices in this vertex buffer
	*/
	FORCEINLINE UINT GetNumVertices() const
	{
		return NumVertices;
	}
	/** 
	* @return cached stride for vertex data type for this vertex buffer
	*/
	FORCEINLINE UINT GetStride() const
	{
		return Stride;
	}
	/** 
	* @return total size of data in resource array
	*/
	FORCEINLINE DWORD GetVertexDataSize() const
	{
		return NumVertices * Stride;
	}
	/** 
	* @param UseCPUSkinning - set to TRUE if using cpu skinning with this vertex buffer
	*/
	void SetUseCPUSkinning(UBOOL UseCPUSkinning);

	/**
	* Assignment operator. 
	*/
	FSkeletalMeshVertexBuffer& operator=(const TArray< TGPUSkinVertexFloat16Uvs<> >& InVertices);
	/**
	* Assignment operator. 
	*/
	FSkeletalMeshVertexBuffer& operator=(const TArray< TGPUSkinVertexFloat32Uvs<> >& InVertices);

	/**
	* Convert the existing data in this mesh from 16 bit to 32 bit UVs.
	* Without rebuilding the mesh (loss of precision)
	*/
	void ConvertToFullPrecisionUVs();

private:
	/** InfluenceBones/InfluenceWeights byte order has been swapped */
	UBOOL bInflucencesByteSwapped;
	/** Corresponds to USkeletalMesh::bUseFullPrecisionUVs. if TRUE then 32 bit UVs are used */
	UBOOL bUseFullPrecisionUVs;
	/** TRUE if this vertex buffer will be used with CPU skinning. Resource arrays are set to cpu accessible if this is TRUE */
	UBOOL bUseCPUSkinning;

	/** The vertex data storage type */
	FSkeletalMeshVertexDataInterface* VertexData;
	/** The cached vertex data pointer. */
	BYTE* Data;
	/** The cached vertex stride. */
	UINT Stride;
	/** The cached number of vertices. */
	UINT NumVertices;
	
	/** 
	* Allocates the vertex data storage type. Based on UV precision needed
	*/
	void AllocateData();	
	
	/** 
	* Copy the contents of the source vertex to the destination vertex in the buffer 
	*
	* @param VertexIndex - index into the vertex buffer
	* @param SrcVertex - source vertex to copy from
	*/
	void SetVertex(UINT VertexIndex,const FSoftSkinVertex& SrcVertex);
};

/**
* Vertex influence weights
*/
struct FInfluenceWeights
{
	union 
	{ 
		struct
		{ 
			BYTE InfluenceWeights[MAX_INFLUENCES];
		};
		// for byte-swapped serialization
		DWORD InfluenceWeightsDWORD; 
	};

	/**
	* Serialize to Archive
	*/
	friend FArchive& operator<<( FArchive& Ar, FInfluenceWeights& W )
	{
		return Ar << W.InfluenceWeightsDWORD;
	}
};

/**
* Vertex influence bones
*/
struct FInfluenceBones
{
	union 
	{ 
		struct
		{ 
			BYTE InfluenceBones[MAX_INFLUENCES];
		};
		// for byte-swapped serialization
		DWORD InfluenceBonesDWORD; 
	};

	/**
	* Serialize to Archive
	*/
	friend FArchive& operator<<( FArchive& Ar, FInfluenceBones& W )
	{
		return Ar << W.InfluenceBonesDWORD;
	}

};

/**
* Vertex influence weights and bones
*/
struct FVertexInfluence
{
	FInfluenceWeights Weights;
	FInfluenceBones Bones;

	friend FArchive& operator<<( FArchive& Ar, FVertexInfluence& W )
	{
		return Ar << W.Weights << W.Bones;
	}
};

/**
* Array of vertex influences
*/
class FSkeletalMeshVertexInfluences
{
public:
	TArray<FVertexInfluence> Influences;

	friend FArchive& operator<<( FArchive& Ar, FSkeletalMeshVertexInfluences& W )
	{
		return Ar << W.Influences;
	}
};

/**
* All data to define a certain LOD model for a skeletal mesh.
* All necessary data to render smooth-parts is in SkinningStream, SmoothVerts, SmoothSections and SmoothIndexbuffer.
* For rigid parts: RigidVertexStream, RigidIndexBuffer, and RigidSections.
*/
class FStaticLODModel
{
public:
	/** Sections. */
	TArray<FSkelMeshSection> Sections;

	/** The vertex chunks which make up this LOD. */
	TArray<FSkelMeshChunk> Chunks;

	/** 
	* Bone hierarchy subset active for this chunk.
	* This is a map between the bones index of this LOD (as used by the vertex structs) and the bone index in the reference skeleton of this SkeletalMesh.
	*/
	TArray<WORD> ActiveBoneIndices;  
	
	/** 
	* Bones that should be updated when rendering this LOD. This may include bones that are not required for rendering.
	* All parents for bones in this array should be present as well - that is, a complete path from the root to each bone.
	* For bone LOD code to work, this array must be in strictly increasing order, to allow easy merging of other required bones.
	*/
	TArray<BYTE> RequiredBones;

	/** 
	* Rendering data.
	*/
	FRawStaticIndexBuffer		IndexBuffer; 
	TArray<FMeshEdge>			Edges;
	TArray<WORD>				ShadowIndices;
	TArray<BYTE>				ShadowTriangleDoubleSided;
	UINT						Size;
	UINT						NumVertices;
	/** static vertices from chunks for skinning on GPU */
	FSkeletalMeshVertexBuffer	VertexBufferGPUSkin;

	/** Optional array of weight/bone influences that can be used by this mesh. Defaults are in VertexBufferGPUSkin */
	TArray<FSkeletalMeshVertexInfluences> VertexInfluences;
	
	/** Editor only data: array of the original point (wedge) indices for each of the vertices in a FStaticLODModel */
	FWordBulkData				RawPointIndices;

	/**
	* Initialize the LOD's render resources.
	*
	* @param Parent Parent mesh
	*/
	void InitResources(class USkeletalMesh* Parent);

	/**
	* Releases the LOD's render resources.
	*/
	void ReleaseResources();

	/** Constructor (default) */
	FStaticLODModel()
	:	IndexBuffer(TRUE)	// needs to be CPU accessible for CPU-skinned decals.
	,	Size(0)
	,	NumVertices(0)
	{}

	/**
	 * Special serialize function passing the owning UObject along as required by FUnytpedBulkData
	 * serialization.
	 *
	 * @param	Ar		Archive to serialize with
	 * @param	Owner	UObject this structure is serialized within
	 * @param	Idx		Index of current array entry being serialized
	 */
	void Serialize( FArchive& Ar, UObject* Owner, INT Idx );

	/**
	* Initialize postion and tangent vertex buffers from skel mesh chunks
	*
	* @param Mesh Parent mesh
	*/
	void BuildVertexBuffers(class USkeletalMesh* Mesh);

	/** Utility function for returning total number of faces in this LOD. */
	INT GetTotalFaces();
};

/**
 *	Contains the vertices that are most dominated by that bone. Vertices are in Bone space.
 *	Not used at runtime, but useful for fitting physics assets etc.
 */
struct FBoneVertInfo
{
	// Invariant: Arrays should be same length!
	TArray<FVector>	Positions;
	TArray<FVector>	Normals;
};

struct FBoneBounds
{
	INT					BoneIndex;
	FSimpleBox			Box;

	friend FArchive& operator<<(FArchive& Ar, FBoneBounds& B)
	{
		return Ar << B.BoneIndex << B.Box.Min << B.Box.Max;
	}
};

/** Struct containing information for a particular LOD level, such as materials and info for when to use it. */
struct FSkeletalMeshLODInfo
{
	/**	Indicates when to use this LOD. A smaller number means use this LOD when further away. */
	FLOAT								DisplayFactor;

	/**	Used to avoid 'flickering' when on LOD boundary. Only taken into account when moving from complex->simple. */
	FLOAT								LODHysteresis;

	/** Mapping table from this LOD's materials to the SkeletalMesh materials array. */
	TArray<INT>							LODMaterialMap;

	/** Per-section control over whether to enable shadow casting. */
	TArray<UBOOL>						bEnableShadowCasting;
};

struct FBoneMirrorInfo
{
	/** The bone to mirror. */
	INT		SourceIndex;
	/** Axis the bone is mirrored across. */
	BYTE	BoneFlipAxis;
};
template <> struct TIsPODType<FBoneMirrorInfo> { enum { Value = true }; };

struct FBoneMirrorExport
{
	FName	BoneName;
	FName	SourceBoneName;
	BYTE	BoneFlipAxis;
};

/** Used to specify special properties for cloth vertices */
enum ClothBoneType
{
	/** The cloth Vertex is attached to the physics asset if available */
	CLOTHBONE_Fixed					= 0,

	/** The cloth Vertex is attached to the physics asset if available and made breakable */
	CLOTHBONE_BreakableAttachment	= 1,
};

struct FClothSpecialBoneInfo
{
	/** The bone name to attach to a cloth vertex */
	FName BoneName;

	/** The type of attachment */
	BYTE BoneType;

	/** Array used to cache cloth indices which will be attached to this bone, created in BuildClothMapping(),
	 * Note: These are welded indices.
	 */
	TArray<INT> AttachedVertexIndices;
};


struct FSoftBodyTetraLink
{
	INT TetIndex;
	FVector Bary;
};

struct FSoftBodyTetraLinkArray
{
	FSoftBodyTetraLinkArray(TArray<FSoftBodyTetraLink>& TL) : TetraLinks(TL)
	{
	}

	TArray<FSoftBodyTetraLink> TetraLinks;
};

/** Used to specify special properties for cloth vertices */
enum SoftBodyBoneType
{
	/** The softbody Vertex is attached to the physics asset if available */
	SOFTBODYBONE_Fixed					= 0,
	SOFTBODYBONE_BreakableAttachment	= 1,
	SOFTBODYBONE_TwoWayAttachment		= 2,
};

struct FSoftBodySpecialBoneInfo
{
	/** The bone name to attach to a softbody vertex */
	FName BoneName;

	/** The type of attachment */
	BYTE BoneType;

	/** Array used to cache softbody indices which will be attached to this bone, created in BuildSoftBodyMapping(),
	 * Note: These are welded indices.
	 */
	TArray<INT> AttachedVertexIndices;
};

struct FStretchDescription
{
	FName			Bone;
	FVector			Translation;
	FLOAT			Scale;
};

/**
* Skeletal mesh.
*/
class USkeletalMesh : public UObject
{
	DECLARE_CLASS(USkeletalMesh, UObject, CLASS_SafeReplace | CLASS_NoExport, Engine)

	FBoxSphereBounds				Bounds;
	FLOAT 							ConservativeBounds;
	TArray<FBoneBounds>				PerBoneBounds;
	/** List of materials applied to this mesh. */
	TArray<UMaterialInterface*>		Materials;	
	/** Origin in original coordinate system */
	FVector 						Origin;				
	/** Amount to rotate when importing (mostly for yawing) */
	FRotator						RotOrigin;			
	/** Reference skeleton */
	TArray<FMeshBone>				RefSkeleton;
	/** The max hierarchy depth. */
	INT								SkeletalDepth;

	/** Map from bone name to bone index. Used to accelerate MatchRefBone. */
	TMap<FName,INT>					NameIndexMap;

	/** Static LOD models */
	TIndirectArray<FStaticLODModel>	LODModels;
	/** Reference skeleton precomputed bases. */
	TArray<FMatrix>					RefBasesInvMatrix;	// @todo: wasteful ?!
	/** List of bones that should be mirrored. */
	TArray<FBoneMirrorInfo>			SkelMirrorTable;
	BYTE							SkelMirrorAxis;
	BYTE							SkelMirrorFlipAxis;
	
	/** 
	 *	Array of named socket locations, set up in editor and used as a shortcut instead of specifying 
	 *	everything explicitly to AttachComponent in the SkeletalMeshComponent. 
	 */
	TArray<USkeletalMeshSocket*>	Sockets;

	/** Array of information for each LOD level. */	
	TArray<FSkeletalMeshLODInfo>	LODInfo;

	/** For each bone specified here, all triangles rigidly weighted to that bone are entered into a kDOP, allowing per-poly collision checks. */
	TArray<FName>					PerPolyCollisionBones;
	
	/** For each of these bones, find the parent that is in PerPolyCollisionBones and add its polys to that bone. */
	TArray<FName>					AddToParentPerPolyCollisionBone;

	/** KDOP tree's used for storing rigid triangle information for a subset of bones. */
	TArray<struct FPerPolyBoneCollisionData> PerPolyBoneKDOPs;

	/** If true, include triangles that are soft weighted to bones. */
	BITFIELD						bPerPolyUseSoftWeighting:1;

	/** If true, use PhysicsAsset for line collision checks. If false, use per-poly bone collision (if present). */
	BITFIELD						bUseSimpleLineCollision:1;

	/** If true, use PhysicsAsset for extent (swept box) collision checks. If false, use per-poly bone collision (if present). */
	BITFIELD						bUseSimpleBoxCollision:1;

	/** All meshes default to GPU skinning. Set to True to enable CPU skinning */
	BITFIELD						bForceCPUSkinning:1;

	/** If true, use 32 bit UVs. If false, use 16 bit UVs to save memory */
	BITFIELD						bUseFullPrecisionUVs:1;

	BITFIELD						ForceShadowVolumes:1;

	/** FaceFX animation asset */
	UFaceFXAsset*					FaceFXAsset;
	
	/** Asset used for previewing bounds in AnimSetViewer. Makes setting up LOD distance factors more reliable. */
	UPhysicsAsset*					BoundsPreviewAsset;

	/** LOD bias to use for PC.						*/
	INT								LODBiasPC;
	/** LOD bias to use for PS3.					*/
	INT								LODBiasPS3;
	/** LOD bias to use for Xbox 360.				*/
	INT								LODBiasXbox360;

	// CLOTH
	// Under Development! Not a fully supported feature at the moment.

	/** Cache of ClothMesh objects at different scales. */
	TArray<FPointer>				ClothMesh;

	/** Scale of each of the ClothMesh objects in cache. This array is same size as ClothMesh. */
	TArray<FLOAT>					ClothMeshScale;

	/** 
	 *	Mapping between each vertex in the simulation mesh and the graphics mesh. 
	 *	This is ordered so that 'free' vertices are first, and then after NumFreeClothVerts they are 'fixed' to the skinned mesh.
	 */
	TArray<INT>						ClothToGraphicsVertMap;

	/**
	 * Mapping from index of rendered mesh to index of simulated mesh.
	 * This mapping applies before ClothToGraphicsVertMap which can then operate normally
	 * The reason for this mapping is to weld several vertices with the same position but different texture coordinates into one
	 * simulated vertex which makes it possible to run closed meshes for cloth.
	 */
	TArray<INT>						ClothWeldingMap;

	/**
	 * This is the highest value stored in ClothWeldingMap
	 */
	INT								ClothWeldingDomain;

	/**
	 * This will hold the indices to the reduced number of cloth vertices used for cooking the NxClothMesh.
	 */
	TArray<INT>						ClothWeldedIndices;

	/**
	 * This will hold the indices to the reduced number of cloth vertices used for cooking the NxClothMesh.
	 */
	BITFIELD						bForceNoWelding:1;

	/** Point in the simulation cloth vertex array where the free verts finish and we start having 'fixed' verts. */
	INT								NumFreeClothVerts;

	/** Index buffer for simulation cloth. */
	TArray<INT>						ClothIndexBuffer;

	/** Vertices with any weight to these bones are considered 'cloth'. */
	TArray<FName>					ClothBones;

	/** Enable constraints that attempt to minimize curvature or folding of the cloth. */
	BITFIELD 						bEnableClothBendConstraints:1;

	/** Enable damping forces on the cloth. */
	BITFIELD 						bEnableClothDamping:1;

	/** Enable center of mass damping of cloth internal velocities.  */
	BITFIELD						bUseClothCOMDamping:1;

	/** Controls strength of springs that attempts to keep particles in the cloth together. */
	FLOAT 							ClothStretchStiffness;

	/** 
	 *	Controls strength of springs that stop the cloth from bending. 
	 *	bEnableClothBendConstraints must be true to take affect. 
	 */
	FLOAT 							ClothBendStiffness;

	/** 
	 *	This is multiplied by the size of triangles sharing a point to calculate the points mass.
	 *	This cannot be modified after the cloth has been created.
	 */
	FLOAT 							ClothDensity;

	/** How thick the cloth is considered when doing collision detection. */
	FLOAT 							ClothThickness;

	/** 
	 *	Controls how much damping force is applied to cloth particles.
	 *	bEnableClothDamping must be true to take affect.
	 */
	FLOAT 							ClothDamping;

	/** Increasing the number of solver iterations improves how accurately the cloth is simulated, but will also slow down simulation. */
	INT 							ClothIterations;

	/** Controls movement of cloth when in contact with other bodies. */
	FLOAT 							ClothFriction;

	/** 
	 * Controls the size of the grid cells a cloth is divided into when performing broadphase collision. 
	 * The cell size is relative to the AABB of the cloth.
	 */
	FLOAT							ClothRelativeGridSpacing;

	/** Adjusts the internal "air" pressure of the cloth. Only has affect when bEnableClothPressure. */
	FLOAT							ClothPressure;

	/** Response coefficient for cloth/rb collision */
	FLOAT							ClothCollisionResponseCoefficient;

	/** How much an attachment to a rigid body influences the cloth */
	FLOAT							ClothAttachmentResponseCoefficient;

	/** How much extension an attachment can undergo before it tears/breaks */
	FLOAT							ClothAttachmentTearFactor;

	/**
	 * Maximum linear velocity at which cloth can go to sleep.
	 * If negative, the global default will be used.
	 */
	FLOAT							ClothSleepLinearVelocity;



	/** Enable orthogonal bending resistance to minimize curvature or folding of the cloth. 
	 *  This technique uses angular springs instead of distance springs as used in 
	 *  'bEnableClothBendConstraints'. This mode is slower but independent of stretching resistance.
	 */
	BITFIELD						bEnableClothOrthoBendConstraints : 1;

	/** Enables cloth self collision. */
	BITFIELD						bEnableClothSelfCollision : 1;

	/** Enables pressure support. Simulates inflated objects like balloons. */
	BITFIELD						bEnableClothPressure : 1;

	/** Enables two way collision with rigid-bodies. */
	BITFIELD						bEnableClothTwoWayCollision : 1;

	/** 
	 * Vertices with any weight to these bones are considered cloth with special behavoir, currently
	 * they are attached to the physics asset with fixed or breakable attachments.
	 */
	TArray<FClothSpecialBoneInfo>	ClothSpecialBones; //ClothBones could probably be eliminated, but that requires and interface change

/** 
 * Enable cloth line/extent/point checks. 
 * Note: line checks are performed with a raycast against the cloth, but point and swept extent checks are performed against the cloth AABB 
 */
	BITFIELD						bEnableClothLineChecks : 1;

	/**
	 *  Whether cloth simulation should be wrapped inside a Rigid Body and only be used upon impact
	 */
	BITFIELD						bClothMetal : 1;

	/** Threshold for when deformation is allowed */
	FLOAT							ClothMetalImpulseThreshold;
	/** Amount by which colliding objects are brought closer to the cloth */
	FLOAT							ClothMetalPenetrationDepth;
	/** Maximum deviation of cloth particles from initial position */
	FLOAT							ClothMetalMaxDeformationDistance;

/** Used to enable cloth tearing. Note, extra vertices/indices must be reserved using ClothTearReserve */
	BITFIELD						bEnableClothTearing : 1;

/** Stretch factor beyond which a cloth edge/vertex will tear. Should be greater than 1. */
	FLOAT							ClothTearFactor;

/** Number of vertices/indices to set aside to accomodate new triangles created as a result of tearing */
	INT								ClothTearReserve;

/** Map which maps from a set of 3 triangle indices packet in a 64bit to the location in the index buffer,
 *  Used to update indices for torn triangles. Generated in InitClothSim().
 */
	TMap<QWORD,INT>					ClothTornTriMap;

	TArray<BOOL>					GraphicsIndexIsCloth;
	FString							SourceFilePath;
	FString							SourceFileTimestamp;
	FString							MaxFilePath;
	FName							SkeletonName;

	/** Mapping between each vertex of the simulated soft-body's surface-mesh and the graphics mesh. */ 	
	TArray<INT>								SoftBodySurfaceToGraphicsVertMap;

	/** Index buffer of the triangles of the soft-body's surface mesh. */
	TArray<INT>								SoftBodySurfaceIndices;

	/** Base array of tetrahedron vertex positions, used to generate the scaled versions from. */
	TArray<FVector>							SoftBodyTetraVertsUnscaled;

	/** Index buffer of the tetrahedra of the soft-body's tetra-mesh. */	
	TArray<INT>								SoftBodyTetraIndices;

	/** Mapping between each vertex of the surface-mesh and its tetrahedron, with local positions given in barycentric coordinates. */
	TArray<FSoftBodyTetraLink>				SoftBodyTetraLinks;

	/** Cache of pointers to NxSoftBodyMesh objects at different scales. */
	TArray<FPointer>						CachedSoftBodyMeshes;

	/** Scale of each of the NxSoftBodyMesh objects in cache. This array is same size as CachedSoftBodyMeshes. */
	TArray<FLOAT>							CachedSoftBodyMeshScales;

	/** Vertices with any weight to these bones are considered 'soft-body'. */
	TArray<FName>							SoftBodyBones;

	/** 
	 * Vertices with any weight to these bones are considered softbody with special behavoir, currently
	 * they are attached to the physics asset with fixed attachments.
	 */
	TArray<FSoftBodySpecialBoneInfo>		SoftBodySpecialBones; //SoftBodyBones could probably be eliminated, but that requires and interface change

	FLOAT							SoftBodyVolumeStiffness;
	FLOAT							SoftBodyStretchingStiffness;
	FLOAT							SoftBodyDensity;
	FLOAT							SoftBodyParticleRadius;
	FLOAT							SoftBodyDamping;
	INT								SoftBodySolverIterations;
	FLOAT							SoftBodyFriction;
	FLOAT							SoftBodyRelativeGridSpacing;
	FLOAT							SoftBodySleepLinearVelocity;
	BITFIELD						bEnableSoftBodySelfCollision : 1;
	FLOAT							SoftBodyAttachmentResponse;
	FLOAT							SoftBodyCollisionResponse;
	FLOAT							SoftBodyDetailLevel;
	INT								SoftBodySubdivisionLevel;
	BITFIELD						bSoftBodyIsoSurface : 1;
	BITFIELD						bEnableSoftBodyDamping : 1;
	BITFIELD						bUseSoftBodyCOMDamping : 1;
	FLOAT							SoftBodyAttachmentThreshold;
	BITFIELD						bEnableSoftBodyTwoWayCollision : 1;
	FLOAT							SoftBodyAttachmentTearFactor;

	/** Enable soft body line checks. */
	BITFIELD						bEnableSoftBodyLineChecks : 1;

	/** A fence which is used to keep track of the rendering thread releasing the static mesh resources. */
	FRenderCommandFence				ReleaseResourcesFence;

	/** GUID for this SkeletalMeshm, used when linking meshes to AnimSets. */
	FGuid							SkelMeshGUID;

	TArray<FStretchDescription>		Stretches;

	/**
	* Initialize the mesh's render resources.
	*/
	void InitResources();

	/**
	* Releases the mesh's render resources.
	*/
	void ReleaseResources();

	// Object interface.
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	void Serialize( FArchive& Ar );
	virtual void PostLoad();
	virtual void PreSave();
	virtual void FinishDestroy();
	virtual void PostDuplicate();
	
	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 * 
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	void StripData(UE3::EPlatformType TargetPlatform);
	
	/** Re-generate the per-poly collision data in the PerPolyBoneKDOPs array, based on names in the PerPolyCollisionBones array. */
	void UpdatePerPolyKDOPs();

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );


	/**
	 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
	 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
	 * you have a component of interest but what you really want is some characteristic that you can use to track
	 * down where it came from.  
	 *
	*/
	virtual FString GetDetailedInfoInternal() const;

	/** Setup-only routines - not concerned with the instance. */
	
	/**
	 * Create all render specific (but serializable) data like e.g. the 'compiled' rendering stream,
	 * mesh sections and index buffer.
	 *
	 * @todo: currently only handles LOD level 0.
	 */
	void CreateSkinningStreams( 
		const TArray<FVertInfluence>& Influences, 
		const TArray<FMeshWedge>& Wedges, 
		const TArray<FMeshFace>& Faces, 
		const TArray<FVector>& Points,
		const FSkelMeshExtraInfluenceImportData* ExtraInfluenceData=NULL
		);
	void CalculateInvRefMatrices();
	void CalcBoneVertInfos( TArray<FBoneVertInfo>& Infos, UBOOL bOnlyDominant );

	/** Clear and create the NameIndexMap of bone name to bone index. */
	void InitNameIndexMap();

	UBOOL	IsCPUSkinned() const;
	INT		MatchRefBone( FName StartBoneName) const;
	UBOOL	BoneIsChildOf( INT ChildBoneIndex, INT ParentBoneIndex ) const;
	class USkeletalMeshSocket* FindSocket(FName InSocketName);

	FMatrix	GetRefPoseMatrix( INT BoneIndex ) const;

	/** Allocate and initialise bone mirroring table for this skeletal mesh. Default is source = destination for each bone. */
	void InitBoneMirrorInfo();

	/** Utility for copying and converting a mirroring table from another SkeletalMesh. */
	void CopyMirrorTableFrom(USkeletalMesh* SrcMesh);
	void ExportMirrorTable(TArray<FBoneMirrorExport> &MirrorExportInfo);
	void ImportMirrorTable(TArray<FBoneMirrorExport> &MirrorExportInfo);

	/** 
	 *	Utility for checking that the bone mirroring table of this mesh is good.
	 *	Return TRUE if mirror table is OK, false if there are problems.
	 *	@param	ProblemBones	Output string containing information on bones that are currently bad.
	 */
	UBOOL MirrorTableIsGood(FString& ProblemBones);

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	INT GetResourceSize();

	/** Uses the ClothBones array to analyze the graphics mesh and generate informaton needed to construct simulation mesh (ClothToGraphicsVertMap etc). */
	void BuildClothMapping();

	void BuildClothTornTriMap();

	/** Determines if this mesh is only cloth. */
	UBOOL IsOnlyClothMesh() const;

	/** Reset the store of cooked cloth meshes. Need to make sure you are not actually using any when you call this. */
	void ClearClothMeshCache();

#if WITH_NOVODEX && !NX_DISABLE_CLOTH
	/** Get the cooked NxClothMesh for this mesh at the given scale. */
	class NxClothMesh* GetClothMeshForScale(FLOAT InScale);

	/** Pull the cloth mesh positions from the SkeletalMesh skinning data. */
	UBOOL ComputeClothSectionVertices(TArray<FVector>& ClothSectionVerts, FLOAT InScale, UBOOL ForceNoWelding=FALSE);
#endif

	/** Clears internal soft-body buffers. */
	void ClearSoftBodyMeshCache();

#if WITH_NOVODEX && !NX_DISABLE_SOFTBODY

	class NxSoftBodyMesh* GetSoftBodyMeshForScale(FLOAT InScale);

#endif //WITH_NOVODEX && !NX_DISABLE_SOFTBODY

	/** 
	* Removes all vertex data needed for shadow volume rendering 
	*/
	void RemoveShadowVolumeData();

	/**
	* Verify SkeletalMeshLOD is set up correctly	
	*/
	void DebugVerifySkeletalMeshLOD();
};


#include "UnkDOP.h"

/** Data used for doing line checks against triangles rigidly weighted to a specific bone. */
struct FPerPolyBoneCollisionData
{
	/** KDOP tree spacial data structure used for collision checks. */
	TkDOPTree<class FSkelMeshCollisionDataProvider,WORD>	KDOPTree;

	/** Collision vertices, in local bone space */
	TArray<FVector>									CollisionVerts;

	FPerPolyBoneCollisionData() {}

	friend FArchive& operator<<(FArchive& Ar,FPerPolyBoneCollisionData& Data)
	{
		Ar << Data.KDOPTree;
		Ar << Data.CollisionVerts;
		return Ar;
	}
};

/** This struct provides the interface into the skeletal mesh collision data */
class FSkelMeshCollisionDataProvider
{
	/** The component this mesh is attached to */
	const USkeletalMeshComponent* Component;
	/** The mesh that is being collided against */
	class USkeletalMesh* Mesh;
	/** Index into PerPolyBoneKDOPs array within SkeletalMesh */
	INT BoneCollisionIndex;
	/** Index of bone that this collision is for within the skel mesh. */
	INT BoneIndex;
	/** Cached calculated bone transform. Includes scaling. */
	FMatrix BoneToWorld;
	/** Cached calculated inverse bone transform. Includes scaling. */
	FMatrix WorldToBone;

	/** Hide default ctor */
	FSkelMeshCollisionDataProvider(void)
	{
	}

public:
	/** Sets the component and mesh members */
	FORCEINLINE FSkelMeshCollisionDataProvider(const USkeletalMeshComponent* InComponent, USkeletalMesh* InMesh, INT InBoneIndex, INT InBoneCollisionIndex) :
		Component(InComponent),
		Mesh(InComponent->SkeletalMesh),
		BoneCollisionIndex(InBoneCollisionIndex),
		BoneIndex(InBoneIndex)
		{
			BoneToWorld = Component->GetBoneMatrix(BoneIndex);
			WorldToBone = BoneToWorld.InverseSafe();
		}

	FORCEINLINE const FVector& GetVertex(WORD Index) const
	{
		return Mesh->PerPolyBoneKDOPs(BoneCollisionIndex).CollisionVerts(Index);
	}

	FORCEINLINE UMaterialInterface* GetMaterial(WORD MaterialIndex) const
	{
		return Component->GetMaterial(MaterialIndex);
	}

	FORCEINLINE INT GetItemIndex(WORD MaterialIndex) const
	{
		return 0;
	}

	FORCEINLINE UBOOL ShouldCheckMaterial(INT MaterialIndex) const
	{
		return TRUE;
	}

	FORCEINLINE const TkDOPTree<FSkelMeshCollisionDataProvider,WORD>& GetkDOPTree(void) const
	{
		return Mesh->PerPolyBoneKDOPs(BoneCollisionIndex).KDOPTree;
	}

	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return BoneToWorld;
	}

	FORCEINLINE const FMatrix& GetWorldToLocal(void) const
	{
		return WorldToBone;
	}

	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return GetLocalToWorld().TransposeAdjoint();
	}

	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return GetLocalToWorld().Determinant();
	}
};




/*-----------------------------------------------------------------------------
FSkeletalMeshSceneProxy
-----------------------------------------------------------------------------*/

/**
 * A skeletal mesh component scene proxy.
 */
class FSkeletalMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	/** 
	 * Constructor. 
	 * @param	Component - skeletal mesh primitive being added
	 */
	FSkeletalMeshSceneProxy(const USkeletalMeshComponent* Component, const FColor& Color = FColor(230, 230, 255));

	// FPrimitiveSceneProxy interface.

	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags);

	/**
	* Draws the primitive's dynamic decal elements.  This is called from the rendering thread for each frame of each view.
	* The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	* Called in the rendering thread.
	*
	* @param	PDI						The interface which receives the primitive elements.
	* @param	View					The view which is being rendered.
	* @param	InDepthPriorityGroup	The DPG which is being rendered.
	* @param	bDynamicLightingPass	TRUE if drawing dynamic lights, FALSE if drawing static lights.
	* @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	*/
	virtual void DrawDynamicDecalElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		UBOOL bDynamicLightingPass,
		UBOOL bTranslucentReceiverPass
		);

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

	/**
	 * Removes a decal interaction from the primitive.  This is called in the rendering thread by RemoveDecalInteraction_GameThread.
	 */
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent);

	/**
	 * Draws the primitive's shadow volumes.  This is called from the rendering thread,
	 * during the FSceneRenderer::RenderLights phase.
	 * @param SVDI - The interface which performs the actual rendering of a shadow volume.
	 * @param View - The view which is being rendered.
	 * @param Light - The light for which shadows should be drawn.
	 * @param DPGIndex - The depth priority group the light is being drawn for.
	 */
	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const FLightSceneInfo* Light,UINT DPGIndex);

	/**
	 * Returns the world transform to use for drawing.
	 * @param View - Current view
	 * @param OutLocalToWorld - Will contain the local-to-world transform when the function returns.
	 * @param OutWorldToLocal - Will contain the world-to-local transform when the function returns.
	 */
	virtual void GetWorldMatrices( const FSceneView* View, FMatrix& OutLocalToWorld, FMatrix& OutWorldToLocal );

	/**
	 * Relevance is always dynamic for skel meshes unless they are disabled
	 */
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber);

	/** Util for getting LOD index currently used by this SceneProxy. */
	INT GetCurrentLODIndex();

	/** 
	 * Render bones for debug display
	 */
	void DebugDrawBones(FPrimitiveDrawInterface* PDI,const FSceneView* View, const TArray<FMatrix>& InSpaceBases, const class FStaticLODModel& LODModel, const FColor& LineColor);

	/** 
	 * Render physics asset for debug display
	 */
	void DebugDrawPhysicsAsset(FPrimitiveDrawInterface* PDI,const FSceneView* View);

	/** Render any per-poly collision data for tri's rigidly weighted to bones. */
	void DebugDrawPerPolyCollision(FPrimitiveDrawInterface* PDI, const TArray<FMatrix>& InSpaceBases);

	/** 
	 * Render bounds for debug display
	 */
	void DebugDrawBounds(FPrimitiveDrawInterface* PDI,const FSceneView* View);

	/** 
	 * Render soft body tetrahedra for debug display
	 */
	void DebugDrawSoftBodyTetras(FPrimitiveDrawInterface* PDI, const FSceneView* View);

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocSkMSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + LODSections.GetAllocatedSize() ); }

protected:
	AActor* Owner;
	const USkeletalMesh* SkeletalMesh;
	FSkeletalMeshObject* MeshObject;
	UPhysicsAsset* PhysicsAsset;

	/** data copied for rendering */
	FColor LevelColor;
	FColor PropertyColor;
	BITFIELD bCastShadow : 1;
	BITFIELD bSelected : 1;
	BITFIELD bShouldCollide : 1;
	BITFIELD bDisplayBones : 1;
	BITFIELD bForceWireframe : 1;
	FMaterialViewRelevance MaterialViewRelevance;

	/** info for section element in an LOD */
	struct FSectionElementInfo
	{
		/*
		FSectionElementInfo() 
		:	Material(NULL)
		,	bEnableShadowCasting(TRUE)
		{}
		*/
		FSectionElementInfo(UMaterialInterface* InMaterial, UBOOL bInEnableShadowCasting)
		:	Material( InMaterial )
		,	bEnableShadowCasting( bInEnableShadowCasting )
		{}
		UMaterialInterface* Material;
		/** Whether shadow casting is enabled for this section. */
		UBOOL bEnableShadowCasting;
	};

	/** Section elements for a particular LOD */
	struct FLODSectionElements
	{
		TArray<FSectionElementInfo> SectionElements;
	};

	/** Array of section elements for each LOD */
	TArray<FLODSectionElements> LODSections;

	/** This is the color used to render bones if bDisplayBones is set to TRUE */
	FColor BoneColor;
};


#endif // __UNSKELETALMESH_H__
