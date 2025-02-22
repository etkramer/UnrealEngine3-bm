/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNotify extends Object
	native(Anim)
	abstract
	editinlinenew
	hidecategories(Object)
	collapsecategories;

struct native NotifierInfo
{
    var export editinline SkeletalMeshComponent SkelComponent;
    var bool AnimMirrored;
};

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class USkeletalMeshComponent* SkelComponent ) {};
}
