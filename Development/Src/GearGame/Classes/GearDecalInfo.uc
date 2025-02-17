/** 
 * 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDecalInfo extends Object
	native
	hidecategories(Object)
	editinlinenew;

/** This is the version for the blood info.  This is primary used to have nice ordered BloodDecals as they are indexed by enum value **/
var int Version;


struct native GearDecalDatum
{
	/** This is left for the sake of editing, knowing which entry is which weapon */
	var() editconst EGearDecalType Type;

	/** This holds the decal data for this blood decal type **/
	var() editinline array<DecalData> DecalData;
};

/** This contains all of the types of blood decals that we can place in the world **/
var() editinline array<GearDecalDatum> DecalInfo;   



cpptext
{
	virtual void PostLoad();
}



defaultproperties
{
	Version=2
		
	DecalInfo(GDTT_Blood_GenericSplat)=(Type=GDTT_Blood_GenericSplat)
	DecalInfo(GDTT_Chainsaw_Ground)=(Type=GDTT_Chainsaw_Ground)
	DecalInfo(GDTT_Chainsaw_Wall)=(Type=GDTT_Chainsaw_Wall)
	DecalInfo(GDTT_DBNO_BodyHittingFloor)=(Type=GDTT_DBNO_BodyHittingFloor)
	DecalInfo(GDTT_DBNO_Smear)=(Type=GDTT_DBNO_Smear)
	DecalInfo(GDTT_ExitWoundSplatter)=(Type=GDTT_ExitWoundSplatter)
	DecalInfo(GDTT_GibExplode_Ceiling)=(Type=GDTT_GibExplode_Ceiling)
	DecalInfo(GDTT_GibExplode_Ground)=(Type=GDTT_GibExplode_Ground)
	DecalInfo(GDTT_GibExplode_Wall)=(Type=GDTT_GibExplode_Wall)
	DecalInfo(GDTT_GibImpact)=(Type=GDTT_GibImpact)
	DecalInfo(GDTT_GibTrail)=(Type=GDTT_GibTrail)
	DecalInfo(GDTT_LimbBreak)=(Type=GDTT_LimbBreak)
	DecalInfo(GDTT_MeatBag_BloodSplatter)=(Type=GDTT_MeatBag_BloodSplatter)
	DecalInfo(GDTT_MeatBag_FirstGrabbing)=(Type=GDTT_MeatBag_FirstGrabbing)
	DecalInfo(GDTT_MeatBag_HeelScuff)=(Type=GDTT_MeatBag_HeelScuff)
	DecalInfo(GDTT_NeckSpurt)=(Type=GDTT_NeckSpurt)
	DecalInfo(GDTT_Wall_SlammingIntoCover)=(Type=GDTT_Wall_SlammingIntoCover)
	DecalInfo(GDTT_Wall_Smear)=(Type=GDTT_Wall_Smear)
	DecalInfo(GDTT_Wall_Smear_Mirrored)=(Type=GDTT_Wall_Smear_Mirrored)
	DecalInfo(GDTT_GibExplode_Ground_SmallSplat)=(Type=GDTT_GibExplode_Ground_SmallSplat)
	DecalInfo(GDTT_BloodPool)=(Type=GDTT_BloodPool)
	DecalInfo(GDTT_PawnIsReallyHurtSmallSplat)=(Type=GDTT_PawnIsReallyHurtSmallSplat)
	DecalInfo(GDTT_HitByBulletSmallSplat)=(Type=GDTT_HitByBulletSmallSplat)
	DecalInfo(GDTT_ExecutionLong_PunchFace)=(Type=GDTT_ExecutionLong_PunchFace)
	DecalInfo(GDTT_ExecutionLong_Sniper)=(Type=GDTT_ExecutionLong_Sniper)
	DecalInfo(GDTT_ExecutionLong_TorqueBow)=(Type=GDTT_ExecutionLong_TorqueBow)
	DecalInfo(GDTT_ExecutionLong_PistolWhip)=(Type=GDTT_ExecutionLong_PistolWhip)
	DecalInfo(GDTT_FlameThrower_StartFlameBlast)=(Type=GDTT_FlameThrower_StartFlameBlast)
	DecalInfo(GDTT_FlameThrower_FlameIsBurning)=(Type=GDTT_FlameThrower_FlameIsBurning)
	DecalInfo(GDDT_ReaverGibExplode)=(Type=GDDT_ReaverGibExplode)
}



