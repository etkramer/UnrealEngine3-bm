/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNotify_Scripted extends AnimNotify
	native(Anim)
	abstract;

event Notify( Actor Owner, SkeletalMeshComponent AnimSeqInstigator );

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class USkeletalMeshComponent* SkelComponent );
}
