class SkelControlLookAt extends SkelControlBase
	native(Anim);
	
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *	Controller that rotates a single bone to 'look at' a given target.
 */
 
cpptext
{
	// USkelControlBase interface
	/** LookAtAlpha allows to cancel head look when going beyond boundaries */
	virtual	FLOAT	GetControlAlpha();
	virtual void	TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
	virtual void	GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices);
	virtual void	CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);	
	
	virtual INT		GetWidgetCount() { return 1; }
	virtual FMatrix GetWidgetTM(INT WidgetIndex, USkeletalMeshComponent* SkelComp, INT BoneIndex);
	virtual void	HandleWidgetDrag(INT WidgetIndex, const FVector& DragVec);
	virtual void	DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex);

	virtual void SetControlTargetLocation(const FVector& InTargetLocation);

protected:
	virtual UBOOL	ApplyLookDirectionLimits(FVector& DesiredLookDir, const FVector &CurrentLookDir, INT BoneIndex, USkeletalMeshComponent* SkelComp);

public:
}

/** Position in world space that this bone is looking at. */
var(LookAt)		vector				TargetLocation;

/** Reference frame that TargetLocation is defined in. */
var(LookAt)		EBoneControlSpace	TargetLocationSpace;

/** Axis of the controlled bone that you wish to point at the TargetLocation. */
var(LookAt)		EAxis				LookAtAxis;

/** Axis of bone to point upwards. Cannot be the same as LookAtAxis. */
var(LookAt)		EAxis				UpAxis;

var(Limit)		EBoneControlSpace	AllowRotationSpace;

/** Name of bone used if TargetLocationSpace is BCS_OtherBoneSpace. */
var(LookAt)		name				TargetSpaceBoneName;

/** Whether to invert the LookAtAxis, so it points away from the TargetLocation. */
var(LookAt)		bool				bInvertLookAtAxis;

/** If you want to also define which axis should try to point 'up' (world +Z). */
var(LookAt)		bool				bDefineUpAxis;

/** Whether to invert the UpAxis, so it points down instead. */
var(LookAt)		bool				bInvertUpAxis;

/** If true, only allow a certain adjustment from the reference pose of the bone. */
var(Limit)		bool				bEnableLimit;
/** By default limit is based on ref pose of the character */
var(Limit)		bool				bLimitBasedOnRefPose;
/** Interp back to zero strength if limit surpassed */
var(Limit)		bool				bDisableBeyondLimit;
/** Call event to notify owner of limit break */
var(Limit)		bool				bNotifyBeyondLimit;

/** If true, draw a cone in the editor to indicate the maximum allowed movement of the bone. */
var(Limit)		bool				bShowLimit;

var(Limit)		bool				bAllowRotationX;
var(Limit)		bool				bAllowRotationY;
var(Limit)		bool				bAllowRotationZ;

/** Interpolation speed for TargetLocation to DesiredTargetLocation */
var(LookAt)		float				TargetLocationInterpSpeed;

/** Interpolation target for TargetLocation */
var				vector				DesiredTargetLocation;

/** The maximum rotation applied from the reference pose of the bone, in degrees. */
var(Limit)		float				MaxAngle;

/** The outer maximum rotation applied from the reference pose of the bone, in degrees, that allows bone to stay in the MaxAngle */
var(Limit)		float				OuterMaxAngle;

/** Allowed error between the current look direction and the desired look direction. */
var(Limit)		float				DeadZoneAngle;

/** Per rotation axis filtering */
var(Limit)		Name				AllowRotationOtherBoneName;

/** LookAtAlpha allows to cancel head look when going beyond boundaries */
var const transient	float	LookAtAlpha;
var const transient float	LookAtAlphaTarget;
var const transient float	LookAtAlphaBlendTimeToGo;

/** internal, used to draw base orientation for limits */
var const transient Vector	LimitLookDir;
/** Internal, base look dir, without skel controller's influence. */
var const transient Vector	BaseLookDir;
/** Internal, base bone position in component space. */
var const transient Vector	BaseBonePos;
/** 
 * Keep track of when the controller was last calculated.
 * We need this to make sure BaseLookDir is accurate, when using CanLookAt().
 */
var const transient float	LastCalcTime;

/** Sets DesiredTargetLocation to a new location */
final native function SetTargetLocation(Vector NewTargetLocation);

/** Interpolate TargetLocation towards DesiredTargetLocation based on TargetLocationInterpSpeed */
final native function InterpolateTargetLocation(float DeltaTime);

/** 
 * Set LookAtAlpha. 
 * Allows the controller to blend in/out while still being processed.
 * CalculateNewBoneTransforms() is still called when ControllerStength > 0 and LookAtAlpha <= 0
 */
final native function SetLookAtAlpha(float DesiredAlpha, float DesiredBlendTime);

/** 
 * returns TRUE if PointLoc is within cone of vision of look at controller. 
 * This requires the mesh to be rendered, SpaceBases has to be up to date.
 * @param	PointLoc		Point in world space.
 * @param	bDrawDebugInfo	if true, debug information will be drawn on hud.
 */
final native function bool CanLookAtPoint(vector PointLoc, optional bool bDrawDebugInfo, optional bool bDebugUsePersistentLines, optional bool bDebugFlushLinesFirst);

defaultproperties
{
	LookAtAxis=AXIS_X
	UpAxis=AXIS_Z
	bShowLimit=TRUE
	bLimitBasedOnRefPose=TRUE
	TargetLocationInterpSpeed=10.f

	LookAtAlphaTarget=1.f
	LookAtAlpha=1.f

	bAllowRotationX=TRUE
	bAllowRotationY=TRUE
	bAllowRotationZ=TRUE
	AllowRotationSpace=BCS_BoneSpace

	// default outer max angle
	OuterMaxAngle=90
}
