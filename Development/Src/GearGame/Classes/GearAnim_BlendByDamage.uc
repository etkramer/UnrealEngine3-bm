
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByDamage extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		MyGearPawn;
var()					float			Duration;
var						float			TimeToGo;
var()					float			BlendInTime, BlendOutTime;
var()					bool			bRequiresStoppingPower;
cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	
	bRequiresStoppingPower=TRUE
	Duration=1.f
	BlendInTime=0.15f
	BlendOutTime=0.3f

	Children(0)=(Name="Default")
	Children(1)=(Name="Hit")
}
