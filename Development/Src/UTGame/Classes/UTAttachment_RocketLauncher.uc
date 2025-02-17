/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class UTAttachment_RocketLauncher extends UTWeaponAttachment;

/** component for the laser effect */
var ParticleSystemComponent LaserEffect;

simulated function AttachTo(UTPawn OwnerPawn)
{
	if (LaserEffect != None)
	{
		Mesh.AttachComponentToSocket(LaserEffect, 'MuzzleFlashSocket');
	}

	Super.AttachTo(OwnerPawn);
}

simulated function ThirdPersonFireEffects(vector HitLocation)
{
	if (IsZero(HitLocation))
	{
		// fired rocket
		Mesh.PlayAnim('WeaponReload', (WeaponClass != None) ? WeaponClass.default.FireInterval[0] : 0.0);
		Super.ThirdPersonFireEffects(HitLocation);
	}
	else
	{
		// firing targeting laser
		LaserEffect.SetActive(true);
		LaserEffect.SetVectorParameter('BeamEnd', HitLocation);
	}
}

simulated function StopThirdPersonFireEffects()
{
	Super.StopThirdPersonFireEffects();

	if (Instigator == None || IsZero(Instigator.FlashLocation))
	{
		// turned off targeting laser
		LaserEffect.DeactivateSystem();
	}
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'WP_AVRiL.Mesh.SK_WP_Avril_3P_Mid'
		MaxDrawDistance=5000
		AnimSets(0)=AnimSet'WP_AVRiL.Anims.K_WP_Avril_3P_Base'
		Translation=(Y=1,Z=1)
		Rotation=(Roll=-599)
		Scale=1.1
		End Object

		// 3p targeting beam
		Begin Object Class=ParticleSystemComponent Name=LaserComp
		Template=ParticleSystem'WP_AVRiL.Particles.P_WP_AVRiL_TargetBeam'
		bAutoActivate=false
		bUpdateComponentInTick=true
		TickGroup=TG_PostAsyncWork
		End Object
		LaserEffect=LaserComp

		MuzzleFlashLightClass=class'UTGame.UTRocketMuzzleFlashLight'
		WeaponClass=class'UTWeap_RocketLauncher'

		WeapAnimType=EWAT_Stinger
}
