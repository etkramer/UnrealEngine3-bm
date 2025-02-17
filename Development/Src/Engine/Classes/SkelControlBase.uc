/**
 *	Abstract base class for a skeletal controller.
 *	A SkelControl is a module that can modify the position or orientation of a set of bones in a skeletal mesh in some programmtic way.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SkelControlBase extends Object
	hidecategories(Object)
	native(Anim)
	abstract;


 /** Enum for controlling which reference frame a controller is applied in. */
enum EBoneControlSpace
{
	/** Set absolute position of bone in world space. */
	BCS_WorldSpace,

	/** Set position of bone in Actor's reference frame. */
	BCS_ActorSpace,

	/** Set position of bone in SkeletalMeshComponent's reference frame. */
	BCS_ComponentSpace,

	/** Set position of bone relative to parent bone. */
	BCS_ParentBoneSpace,

	/** Set position of bone in its own reference frame. */
	BCS_BoneSpace,

	/** Set position of bone in the reference frame of another bone. */
	BCS_OtherBoneSpace
};


/** SkeletalMeshComponent owner */
var const transient	SkeletalMeshComponent		SkelComponent;

/** Name used to identify this SkelControl. */
var(Controller)	name			ControlName;

/**
 *	Used to control how much affect this SkelControl has.
 *	1.0 means fully active, 0.0 means have no affect.
 *	Exactly how the control ramps up depends on the specific control type.
 */
var(Controller)	float			ControlStrength;

/** When calling SetActive passing in 'true', indicates how many seconds to take to reach a ControlStrength of 1.0. */
var(Controller)	float			BlendInTime;

/** When calling SetActive passing in 'false', indicates how many seconds to take to reach a ControlStrength of 0.0. */
var(Controller)	float			BlendOutTime;

/** BlendType - Easy in/out */
var(Controller) AnimBlendType	BlendType;

/** Strength towards which we are currently ramping. */
var				float			StrengthTarget;

/** Amount of time left in the currently active blend. */
var				float			BlendTimeToGo;

/** If true, Strength will be the same as given AnimNode(s). This is to make transitions easier between nodes and Controllers. */
var(Controller)	bool			bSetStrengthFromAnimNode;
/** List of AnimNode names, to get Strength from */
var(Controller)	Array<Name>		StrengthAnimNodeNameList;
/** Cached list of nodes to get strength from */
var	transient	Array<AnimNode>	CachedNodeList;
var transient	bool			bInitializedCachedNodeList;

/** If true, calling SetSkelControlActive on this node will call SetSkelControlActive on the next one in the chain as well. */
var(Controller)	bool			bPropagateSetActive;

/** This scaling is applied to the bone that this control is acting upon. */
var(Controller) float			BoneScale;

/**
 *	Used to ensure we don't tick this SkelControl twice, if used in multiple different control chains.
 *	Compared against the SkeletalMeshComponent TickTag.
 */
var				int				ControlTickTag;

/** 
 *	whether this control should be ignored if the SkeletalMeshComponent being composed hasn't been rendered recently
 *	
 *	@note this can be forced by the SkeletalMeshComponent's bIgnoreControllersWhenNotRendered flag
 */
var()			bool			bIgnoreWhenNotRendered;

/** If true, when skel control becomes active it uses a curve rather than a linear blend to blend in more smoothly. */
/** This will change to BlendType in general **/
var() editconst deprecated bool			bEnableEaseInOut;

/** If the LOD of this skeletal mesh is at or above this LOD, then this SkelControl will not be applied. */
var(Controller)	int				IgnoreAtOrAboveLOD;

/** Next SkelControl in the linked list. */
var				SkelControlBase	NextControl;

/** Used by editor. */
var				int				ControlPosX;

/** Used by editor. */
var				int				ControlPosY;

/** Used by editor. */
var				int				DrawWidth;


cpptext
{
	/**
	*	Called from the SkeletalMeshComponent Tick function, to allow SkelControls to do any time-based update,
	*	such as adjusting their current blend amount.
	*/
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);

	/**
	*	Get the array of bones that this controller affects. Must be in hierarchy order, that is, parents before children.
	*/
	virtual void GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices) {}

	/**
	*	Calculate the new component-space transforms for the affected bones.
	*	The output array OutBoneTransforms must correspond to the OutBoneIndices array returned above (be in the same order etc).
	*/
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms) {}

	/**
	*	Allows you to modify the scaling of all affected bones.
	*	The output array OutBoneScales must correspond to the OutBoneIndices array returned above (be in the same order etc).
	*/
	virtual void CalculateNewBoneScales(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FLOAT>& OutBoneScales) {}

	/** Allows you to modify the scaling of the controller bone. */
	virtual FLOAT GetBoneScale(INT BoneIndex, USkeletalMeshComponent* SkelComp) { return BoneScale; }

	// UObject functions
	virtual void PostLoad();

	// UTILS

	/** Utility function for turning axis indicator enum into direction vector, possibly inverted. */
	static FVector GetAxisDirVector(BYTE InAxis, UBOOL bInvert);

	/**
	*	Create a matrix given two arbitrary rows of it.
	*	We generate the missing row using another cross product, but we have to get the order right to avoid changing handedness.
	*/
	static FMatrix BuildMatrixFromVectors(BYTE Vec1Axis, const FVector& Vec1, BYTE Vec2Axis, const FVector& Vec2);

	/** Given two unit direction vectors, find the axis and angle between them. */
	static void FindAxisAndAngle(const FVector& A, const FVector& B, FVector& OutAxis, FLOAT& OutAngle);

	// ANIMTREE EDITOR SUPPORT

	/**
	* Draw this SkelControl in the AnimTreeEditor.
	*
	* @param	Canvas			The canvas to use.
	* @param	bSelected		TRUE if this node is selected.
	* @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
	*/
	virtual void DrawSkelControl(FCanvas* Canvas, UBOOL bSelected, UBOOL bCurves);

	/** For editor use. */
	FIntPoint GetConnectionLocation(INT ConnType);

	/** If we should draw manipulation widget in 3D viewport. */
	virtual INT GetWidgetCount() { return 0; }

	/** The transform to use when drawing the widget, in world space. */
	virtual FMatrix GetWidgetTM(INT WidgetIndex, USkeletalMeshComponent* SkelComp, INT BoneIndex) { return FMatrix::Identity; }

	/** Update properties of this controller based on the widget being manipulated in the 3D viewport. */
	virtual void HandleWidgetDrag(INT WidgetIndex, const FVector& DragVec) {}

	/** Extra function for drawing special debug info in the 3D viewport if desired. */
	virtual void DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex) {}

	/** Called when slider is moved in the AnimTreeEditor. NewSliderValue is in range 0.0 to 1.0. */
	void HandleControlSliderMove(FLOAT NewSliderValue);

	/**
	* Get Alpha for this control. By default it is ControlStrength.
	* 0.f means no effect, 1.f means full effect.
	* ControlStrength controls whether or not CalculateNewBoneTransforms() is called.
	* By modifying GetControlAlpha() you can still get CalculateNewBoneTransforms() called
	* but not have the controller's effect applied on the mesh.
	* This is useful for cases where you need to have the skeleton built in mesh space
	* for testing, which is not available in TickSkelControl().
	*/
	virtual FLOAT	GetControlAlpha();

	/* Returns Final Blend Control Alpha based on Blend Type*/
	FORCEINLINE FLOAT GetBlendControlAlpha(FLOAT ControlAlpha);

	/** Used by high level systems to assign a 'target' for a generic control. One example is the ControlTargets array in SkeletalMeshActor. */
	virtual void SetControlTargetLocation(const FVector& TargetLocation) {}
}


/**
 *	Toggle the active state of the SkeControl.
 *	If passing in true, will take BlendInTime to reach a ControlStrength of 1.0.
 *	If passing in false, will take BlendOutTime to reach a ControlStrength of 0.0.
 */
native final function SetSkelControlActive(bool bInActive);

/**
 * Set custom strength with optional blend time.
 * @param	NewStrength		Target Strength for this controller.
 * @param	InBlendTime		Time it will take to reach that new strength. (0.f == Instant)
 */
native final function SetSkelControlStrength(float NewStrength, float InBlendTime);


defaultproperties
{
	BoneScale=1.0
	ControlStrength=1.0
	StrengthTarget=1.0
	BlendInTime=0.2
	BlendOutTime=0.2
	IgnoreAtOrAboveLOD=1000
}
