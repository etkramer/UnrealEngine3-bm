/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MorphNodePose extends MorphNodeBase
	native(Anim)
	hidecategories(Object);

cpptext
{
	// UObject interface
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	// MorphNodeBase interface
	virtual void GetActiveMorphs(TArray<FActiveMorph>& OutMorphs);
	virtual void InitMorphNode(USkeletalMeshComponent* InSkelComp);

	/** 
	 * Draws this morph node in the AnimTreeEditor.
	 *
	 * @param	Canvas			The canvas to use.
	 * @param	bSelected		TRUE if this node is selected.
	 * @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
	 */	
	virtual void DrawMorphNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bCurves);
}

/** Cached pointer to actual MorphTarget object. */
var		transient MorphTarget	Target;

/** 
 *	Name of morph target to use for this pose node. 
 *	Actual MorphTarget is looked up by name in the MorphSets array in the SkeletalMeshComponent.
 */
var()	name					MorphName;
 
/** default weight is 1.f. But it can be scaled for tweaking. */
var()	float					Weight;
 
/** 
 *	Set the MorphTarget to use for this MorphNodePose by name. 
 *	Will find it in the owning SkeletalMeshComponent MorphSets array using FindMorphTarget.
 */
native final function SetMorphTarget(Name MorphTargetName);

defaultproperties
{
	Weight=1.f
}