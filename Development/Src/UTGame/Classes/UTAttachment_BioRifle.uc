/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAttachment_BioRifle extends UTWeaponAttachment;

var name CurrentIdleAnim;
var name WeaponFireAnims[3];
var name WeaponIdleAnim;
var name WeaponAltFireAnim;
var name WeaponAltChargeAnim;
var name WeaponAltChargeIdleAnim;
var float FireAnimDuration;
/** The particle system while the bio is charging*/
var ParticleSystemComponent ChargeComponent;

simulated function FireModeUpdated(byte FireModeNum, bool bViaReplication)
{
	if(FireModeNum == 0 && CurrentIdleAnim != WeaponIdleAnim)
	{
		CurrentIdleAnim = WeaponIdleAnim;
		RestartIdle();
	}
	else if(FireModeNum == 1 && CurrentIdleAnim != WeaponAltChargeIdleAnim)
	{
		StartCharging();
	}
}

simulated function AttachTo(UTPawn OwnerPawn)
{
	if (OwnerPawn.Mesh != None)
	{
		if (Mesh != None && MuzzleFlashSocket != '')
		{
			Mesh.AttachComponentToSocket(ChargeComponent, MuzzleFlashSocket);
		}
	}
	Super.AttachTo(OwnerPawn);

}
simulated function StartCharging()
{
	ClearTimer('RestartIdle');
	CurrentIdleAnim = WeaponAltChargeIdleAnim;
	if(WorldInfo.NetMode != NM_DedicatedServer)
	{
		Play3pAnimation(WeaponAltChargeAnim,2.7f);
		ChargeComponent.ActivateSystem();
	}
	SetTimer(2.7,false,'RestartIdle');
}

simulated event ThirdPersonFireEffects(vector HitLocation)
{
	local name Anim;
	local UTPawn UTP;
	Super.ThirdPersonFireEffects(HitLocation);
	UTP = UTPawn(Owner);
	if(UTP==none || UTP.FiringMode == 0)
	{
		Anim = WeaponFireAnims[rand(3)];
	}
	else
	{
		Anim = WeaponAltFireAnim;
	}
	ChargeComponent.DeactivateSystem();
	ChargeComponent.KillParticlesForced();
	Play3pAnimation(Anim,FireAnimDuration);
	FireModeUpdated(0,false); // force bio back to 'base' state
	SetTimer(FireAnimDuration,false,'RestartIdle');
}

simulated function RestartIdle()
{
//	`log("Idling with"@currentidleanim);
	Play3pAnimation(CurrentIdleAnim,0.03f,true);
}

simulated event StopThirdPersonFireEffects()
{
	ChargeComponent.DeactivateSystem();
	ChargeComponent.KillParticlesForced();
	Super.StopThirdPersonFireEffects();
}

simulated function Play3pAnimation(name Sequence, float fDesiredDuration, optional bool bLoop)
{
	// Check we have access to mesh and animations
	if (Mesh != None && Mesh.Animations != None)
	{
		// @todo - this should call GetWeaponAnimNodeSeq, move 'duration' code into AnimNodeSequence and use that.
		Mesh.PlayAnim(Sequence, fDesiredDuration, bLoop);
	}
}

defaultproperties
{
	// Weapon SkeletalMesh
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'WP_BioRifle.Mesh.SK_WP_BioRifle_3P_Mid'
		AnimSets(0)=AnimSet'WP_BioRifle.Anims.K_WP_BioRifle_3P'
		bForceRefPose=0
	End Object

	Begin Object Class=ParticleSystemComponent Name=BioChargeEffect
    	Template=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_3P_Alt_MF'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
	End Object
	ChargeComponent = BioChargeEffect

	MuzzleFlashLightClass=class'UTGame.UTGreenMuzzleFlashLight'

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_3P_MF'
	MuzzleFlashAltPSCTemplate=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_3P_MF'
	MuzzleFlashDuration=0.33
	WeaponFireAnims[0]=WeaponFire1;
	WeaponFireAnims[1]=WeaponFire2;
	WeaponFireAnims[2]=WeaponFire3;
	WeaponIdleAnim=WeaponIdle;
	WeaponAltFireAnim=WeaponAltFire;
	WeaponAltChargeAnim=WeaponAltCharge;
	WeaponAltChargeIdleAnim=WeaponAltIdle;
	FireAnimDuration = 0.4; // all the fires are in this range
}
