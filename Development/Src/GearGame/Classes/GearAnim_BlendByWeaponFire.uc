
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BlendByWeaponFire extends AnimNodeBlend
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		GearPawnOwner;

/** Maximum weight of fire node. Set lower to one to retain some of the idle movement */
var()	float	MaxFireBlendWeight;

/** duration of blends */
var()	float	BlendInTime;
var()	float	BlendOutTime;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};



defaultproperties
{
	MaxFireBlendWeight=1.f
	BlendInTime=0.1f
	BlendOutTime=0.33f
	bFixNumChildren=TRUE
	Children(0)=(Name="Idle")
	Children(1)=(Name="Fire")
}
