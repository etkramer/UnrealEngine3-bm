/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 */
class GearPawn_LocustTroikaGunner extends GearPawn_LocustDroneBase
	config(Pawn);


/** Gunner's helmets do not come off **/
//simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );


/**  if hit in the head then we are going to override the impact effects **/
simulated function bool HasImpactOverride()
{
	return TookHeadShot(Mesh.GetBoneName(LastTakeHitInfo.HitBoneIndex), LastTakeHitInfo.HitLocation, LastTakeHitInfo.Momentum);
}

simulated function ParticleSystem GetImpactOverrideParticleSystem()
{
	return ParticleSystem'COG_AssaultRifle.Effects.P_Impact_No_Damage_Sparks';
}


defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=392,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_LocustTroikaMask'
}


