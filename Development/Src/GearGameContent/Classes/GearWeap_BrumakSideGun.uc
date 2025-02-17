/**
 * Brumak Side Gun Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_BrumakSideGun extends GearWeap_BrumakWeaponBase
	hidedropdown;

/** Muzzle Flash particle system */
var()	editinline	ParticleSystemComponent		MuzFlashEmitterLeft;
/** dynamic light */
var()				PointLightComponent			MuzzleFlashLightLeft;

/** Internal flag used to handle left gun firing/effects/etc */
var transient bool bHandleLeftGun;

simulated function bool IsLeftGun()
{
	local GearPawn_LocustBrumak_SideGun	SideGun;

	SideGun = GetSideGun();
	if( SideGun != None )
	{
		return SideGun.bIsLeftGun;
	}
	else // Held by player based on Brumak
	{
		return bHandleLeftGun;
	}
}

simulated event WeaponFired( byte FiringMode, optional vector HitLocation )
{
	local GearPawn_LocustBrumakBase		Brumak;
	local INT							RecoilIndex;

	Brumak	= GetBrumak();
	if( Brumak != None )
	{
		RecoilIndex = IsLeftGun() ? 0 : 1;

		// Fix for low frame rate issue, do not overwrite shakes...
		if( Brumak.IKRecoilCtrl[RecoilIndex] != None && Brumak.IKRecoilCtrl[RecoilIndex].Recoil.TimeToGo < Brumak.IKRecoilCtrl[RecoilIndex].Recoil.TimeDuration*0.5f )
		{
			Brumak.IKRecoilCtrl[RecoilIndex].bPlayRecoil = !Brumak.IKRecoilCtrl[RecoilIndex].bPlayRecoil;
		}
	}

	Super.WeaponFired( FiringMode, HitLocation );
}

simulated function Vector GetMuzzleLoc()
{
	local Vector	MuzzleLoc;
	local Rotator	MuzzleRot;
	local SkeletalMeshComponent PawnMesh;

	PawnMesh = GetSkeletalMeshComponent();
	if( PawnMesh != None )
	{
		PawnMesh.GetSocketWorldLocationAndRotation( GetMuzzleSocketName(IsLeftGun()), MuzzleLoc, MuzzleRot );
		return MuzzleLoc;
	}

	return MuzzleLoc;
}

simulated function DetachWeapon()
{
	local SkeletalMeshComponent	PawnMesh;

	Super.DetachWeapon();

	PawnMesh = GetSkeletalMeshComponent();
	if( PawnMesh != None )
	{
		// shell eject particle system
		if( MuzFlashEmitterLeft != None && PawnMesh.IsComponentAttached(MuzFlashEmitterLeft) )
		{
			PawnMesh.DetachComponent(MuzFlashEmitterLeft);
		}

		if( MuzzleFlashLightLeft != None && PawnMesh.IsComponentAttached(MuzzleFlashLightLeft) )
		{
			PawnMesh.DetachComponent(MuzzleFlashLightLeft);
		}
	}
}

simulated event AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	local GearPawn_LocustBrumak_SideGun SideGun;

	SideGun = GetSideGun();
	if( SideGun != None )
	{
		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			MuzFlashEmitter.SetScale( SideGun.Base.DrawScale );
			SkelMesh.AttachComponentToSocket(MuzFlashEmitter, GetMuzzleSocketName(IsLeftGun()));
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashLight, GetMuzzleSocketName(IsLeftGun()));
		}
	}
	else // Held by player
	{
		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			MuzFlashEmitter.SetScale( Instigator.Base.DrawScale );
			SkelMesh.AttachComponentToSocket(MuzFlashEmitter, GetMuzzleSocketName(FALSE));
			HideMuzzleFlashEmitter();
		}
		if( MuzFlashEmitterLeft != None && !SkelMesh.IsComponentAttached(MuzFlashEmitterLeft) )
		{
			MuzFlashEmitterLeft.SetScale( Instigator.Base.DrawScale );
			SkelMesh.AttachComponentToSocket(MuzFlashEmitterLeft, GetMuzzleSocketName(TRUE));
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashLight, GetMuzzleSocketName(FALSE));
		}
		if( MuzzleFlashLightLeft != None && !SkelMesh.IsComponentAttached(MuzzleFlashLightLeft) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashLightLeft, GetMuzzleSocketName(TRUE));
		}
	}
}


/**
* Start muzzle flash effect
* Network: Local Player and Clients
*/
simulated function PlayMuzzleFlashEffect()
{
	// Play muzzle flash effects only if they are relevant
	if( !bSuppressMuzzleFlash && IsMuzzleFlashRelevant() )
	{
		// activate muzzle flash particle system
		ClearTimer( 'HideMuzzleFlashEmitter' );
		if( MuzFlashEmitter != None )
		{
			MuzFlashEmitter.SetHidden( FALSE );
			if( bMuzFlashEmitterIsLooping )
			{
				MuzFlashEmitter.SetActive(TRUE);
			}
			else
			{
				MuzFlashEmitter.ActivateSystem();
			}
		}
		if( MuzFlashEmitterLeft != None )
		{
			MuzFlashEmitterLeft.SetHidden( FALSE );
			if( bMuzFlashEmitterIsLooping )
			{
				MuzFlashEmitter.SetActive(TRUE);
			}
			else
			{
				MuzFlashEmitter.ActivateSystem();
			}
		}

		// Reset Pulse time to zero, so we start at the beginning.
		MuzzleLightPulseTime = 0.f;
		if( !GearGRI(WorldInfo.GRI).ShouldDisableEffectsDueToFramerate(Instigator) )
		{
			if( MuzzleFlashLight != None && (!bLoopMuzzleFlashLight || !MuzzleFlashLight.bEnabled) )
			{
				MuzzleFlashLight.CastDynamicShadows = bDynamicMuzzleFlashes;
				MuzzleFlashLight.SetEnabled(TRUE);
			}
			if( MuzzleFlashLightLeft != None && (!bLoopMuzzleFlashLight || !MuzzleFlashLightLeft.bEnabled) )
			{
				MuzzleFlashLightLeft.CastDynamicShadows = bDynamicMuzzleFlashes;
				MuzzleFlashLightLeft.SetEnabled(TRUE);
			}
		}

		SetTimer( MuzzleLightDuration, FALSE, nameof(StopMuzzleFlashEffect) );
	}
}

/**
* Stop muzzle flash effect
* Network: Local Player and Clients
*/
simulated function StopMuzzleFlashEffect()
{
	SetTimer( TimeToHideMuzzleFlashPS, FALSE, nameof(HideMuzzleFlashEmitter) );
	if( MuzFlashEmitter != None )
	{
		MuzFlashEmitter.SetActive(FALSE);
	}
	if( MuzFlashEmitterLeft != None )
	{
		MuzFlashEmitterLeft.SetActive(FALSE);
	}

	// turn off muzzle flash light only if it's non looping
	// Looping ones are turned off when the weapon stops firing
	// Used by rapid fire weapons
	if( !bLoopMuzzleFlashLight )
	{
		if( MuzzleFlashLight != None )
		{
			MuzzleFlashLight.SetEnabled( FALSE );
		}
		if( MuzzleFlashLightLeft != None )
		{
			MuzzleFlashLightLeft.SetEnabled( FALSE );
		}		
	}
}

simulated function HideMuzzleFlashEmitter()
{
	if( MuzFlashEmitter != None )
	{
		MuzFlashEmitter.SetHidden( TRUE );
	}
	if( MuzFlashEmitterLeft != None )
	{
		MuzFlashEmitterLeft.SetHidden( TRUE );
	}
}

defaultproperties
{
	AmmoTypeClass=class'GearAmmoType_AssaultRifle'
	bInfiniteSpareAmmo=TRUE
	bWeaponCanBeReloaded=TRUE
	bCanThrow=FALSE

	// MAIN FIREMODE
	FiringStatesArray(0)="WarmingUp"
	WeaponFireTypes(0)=EWFT_InstantHit
	InstantHitDamageTypes(0)=class'GDT_BrumakBullet'
	InstantHitMomentum(0)=1
	FireInterval(0)=0.1

	FireOffset=(X=0,Y=0,Z=0)

//@reimport
	WeaponWhipSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleWhipCue'
//	PS_DefaultImpactEffect=ParticleSystem'Locust_Brumak.Effects.P_Geist_Brumak_SideGun_Impact'

	Mesh=None
	DroppedPickupMesh=None

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
	    LightColor=(R=255,G=192,B=128,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5
	bLoopMuzzleFlashLight=TRUE

	// MF flash
	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle0
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_MuzzleFlash'
	End Object
	MuzFlashEmitter=PSC_Muzzle0
	
	TracerType=WTT_Brumak

	bKillDuringLevelTransition=TRUE
	bInstantHit=TRUE

	PutDownTime=0.0

	FireLoopCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireLoopCue_Cue'
	FireLoopCue_Player=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireLoopCue_Cue'
	FireStopCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireStopCue_Cue'
	FireStopCue_Player=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireStopCue_Cue'
	BarrelSpinningStartCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_AmmoFeederStartCue_Cue'
	BarrelSpinningStopCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_AmmoFeederStopCue_Cue'

	bNewFiringAttack=TRUE
}
