/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AnimObject extends Object
	native(Anim)
	hidecategories(Object)
	abstract;

/** For editor use. */
var	editoronly int									DrawWidth;

/** for editor use  */
var editoronly int									DrawHeight;

/** for editor use. */
var	editoronly int									NodePosX;

/** For editor use. */
var editoronly int									NodePosY;

/** for editor use. */
var editoronly int									OutDrawY;

/**
 * Editor category for this object.  Determines which animtree submenu this object
 * should be placed in
 */
var editoronly string 					CategoryDesc;

/** SkeletalMeshComponent owner */
var const transient	SkeletalMeshComponent		SkelComponent;

/** Various ways to blend animations together. */
enum AnimBlendType
{
	ABT_Linear,
	ABT_Cubic,
	ABT_Sinusoidal,
	ABT_EaseInOutExponent2,
	ABT_EaseInOutExponent3,
	ABT_EaseInOutExponent4,
	ABT_EaseInOutExponent5,
};

cpptext
{
	virtual UAnimNode * GetAnimNode() { return NULL;}
	virtual UMorphNodeBase * GetMorphNodeBase() { return NULL;}
	virtual USkelControlBase * GetSkelControlBase() { return NULL; }

	virtual void DrawNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves) {}
	/** Called after (copy/)pasted - reset values or re-link if needed**/
	virtual void OnPaste() {};
};

DefaultProperties
{

}
