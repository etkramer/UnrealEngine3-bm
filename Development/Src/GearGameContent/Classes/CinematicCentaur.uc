/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class CinematicCentaur extends CinematicCentaur_Base
	placeable;

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'COG_Centaur.Meshes.COG_Centaur'
		PhysicsAsset=PhysicsAsset'COG_Centaur.COG_Centaur_Physics'
		Animations=None
		AnimTreeTemplate=AnimTree'COG_Centaur.Anims.COG_Centaur_Animtree'
		AnimSets(0)=AnimSet'COG_Centaur.Anims.COG_Centaur_Animset'
	End Object
	Physics=PHYS_Interpolating

	CentaurCannonMuzzzleEffect=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_MuzzleFlash'
	CentaurCannonSound=SoundCue'Weapon_Boomer.Firing.BoomerFireCue'

	CentaurDeathSound=SoundCue'Weapon_Grenade.Impacts.GrenadeBoloExplosionCue'
	CentaurDeathEffectKeepTurret=ParticleSystem'Effects_Gameplay.Explosions.P_Centaur_Death'
	CentaurDeathEffectLoseTurret=ParticleSystem'Effects_Gameplay.Explosions.P_Centaur_Death'

	WheelBoneName[0]=b_Rear_Rt_Tire
	WheelBoneName[1]=b_Rear_Lt_Tire
	WheelBoneName[2]=b_Front_Rt_Tire
	WheelBoneName[3]=b_Front_Lt_Tire

	DrawScale=0.85

	WheelRadius=90.0
	SuspensionTravel=60.0

	AngVelSteerFactor=20.0
	MaxSteerAngle=30.0

	DeathAnimNames[0]=Explode

	MinDistFactorForUpdate=0.1
}
