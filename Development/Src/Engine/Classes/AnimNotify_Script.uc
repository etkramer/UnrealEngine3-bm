/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNotify_Script extends AnimNotify
	native(Anim);

var() name NotifyName;

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class USkeletalMeshComponent* SkelComponent );
}
