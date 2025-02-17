
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_AdditiveBlending extends AnimNodeAdditiveBlending
	native(Anim);

/** Cached ref to GearPawn Owner */
var protected transient GearPawn	GearPawnOwner;

/** If TRUE, pass through for Heavy Weapons */
var()	bool	bNotForHeavyWeapons;
/** Controlled by NearHitMisses */
var()	bool	bControlledByNearMisses;
/** Controlled by taking damage */
var()	bool	bControlledByDamage;
/** Max blend Alpha when controlled */
var()	float	ControlMaxBlendAlpha;	
/** Duration of time to hold ControlMaxBlendAlpha when controlled */
var()	float	ControlHoldTime;
var()	float	BlendInTime;
var()	float	BlendOutTime;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
}

defaultproperties
{
	BlendInTime=0.33f
	BlendOutTime=1.f
	BlendType=ABT_EaseInOutExponent3
	ControlHoldTime=1.5f
	ControlMaxBlendAlpha=1.f
}