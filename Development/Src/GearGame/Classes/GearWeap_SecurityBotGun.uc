/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearWeap_SecurityBotGun extends GearWeapon;

/** TRUE when playing looping muzzle flash effect */
var()	bool		bPlayingMuzzleFlashEffect;


/** Looping SoundCue for ammo feeder. */
var protected const SoundCue BarrelSpinningLoopCue;
/** The AudioComponent that is used for the looping ammo loading sound **/
var protected AudioComponent BarrelSpinningLoopAC;


/** Looping SoundCue for firing. */
var protected const SoundCue FireLoopCue;
/** Looping SoundCue for firing. */
var protected const SoundCue FireLoopCue_Player;
/** The AudioComponent that is used for the looping shot sound **/
var protected AudioComponent FireLoopAC;

/** final shot echo, play when firing stops.  nonlooping. */
var const protected SoundCue FireStopCue;
var const protected SoundCue FireStopCue_Player;

var protected const SoundCue CasingImpactCue;

/** Sound to play when ammo feed stops */
var protected const SoundCue BarrelSpinningStopCue;

/** Smoke emitter at the muzzle */
var ParticleSystemComponent		PSC_BarrelSmoke;

simulated function Vector GetMuzzleLoc()
{
	return GearPawn(Instigator).GetPhysicalFireStartLoc(FireOffset);
}


simulated event LocalActiveReloadSuccess( bool bDidSuperSweetReload );
simulated function ActiveReloadSuccess(bool bDidSuperSweetReload);
simulated function FailedActiveReload(bool bFailedActiveReload);
simulated function TimeWeaponEquipping()
{
	SetTimer( 0.0001f, FALSE, 'WeaponEquipped' );
	return;
}

simulated function PlayWeaponReloading()
{
	if( ReloadDuration > 0.f )
	{
		SetTimer( ReloadDuration, FALSE, nameof(EndOfReloadTimer) );
	}
	else
	{
		EndOfReloadTimer();
	}
}


simulated event AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	local GearPawn	MeshOwner;

	
	MeshOwner = GearPawn(SkelMesh.Owner);
	//`log(GetFuncName()@self@SKelMesh@MeshOwner@MeshOwner.GetRightHandSocketName());
	if( MeshOwner != None )
	{
		Skelmesh.AttachComponentToSocket(PSC_BarrelSmoke, MeshOwner.GetRightHandSocketName());

		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			SkelMesh.AttachComponentToSocket(MuzFlashEmitter, MeshOwner.GetRightHandSocketName());
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashLight, MeshOwner.GetRightHandSocketName());
		}

		// shell ejection
		if( PSC_ShellEject != None && !SkelMesh.IsComponentAttached(PSC_ShellEject) )
		{
			SkelMesh.AttachComponentToSocket(PSC_ShellEject, MeshOwner.GetRightHandSocketName());
			SkelMesh.bForceUpdateAttachmentsInTick=true;
			PSC_ShellEject.bJustAttached = true;
			HidePSC_ShellEject();
		}
	}
}


simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name BoneName)
{
	AttachMuzzleEffectsComponents(MeshCpnt);
}

simulated function DetachWeapon()
{
	local SkeletalMeshComponent	PawnMesh;

	Super.DetachWeapon();


	if( Instigator != None )
	{
		PawnMesh = Instigator.Mesh;
	}

	if( PawnMesh != None )
	{
		// shell eject particle system
		if( MuzFlashEmitter != None && PawnMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			PawnMesh.DetachComponent(MuzFlashEmitter);
		}

		if( MuzzleFlashLight != None && PawnMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			PawnMesh.DetachComponent(MuzzleFlashLight);
		}

		if( PSC_ShellEject != None && PawnMesh.IsComponentAttached(PSC_ShellEject) )
		{
			PawnMesh.DetachComponent(PSC_ShellEject);
		}
	}

	if( BarrelSpinningLoopAC != none )
	{
		BarrelSpinningLoopAC.FadeOut( 0.1f, 0.0f );
		BarrelSpinningLoopAC = None;
	}

	if( FireLoopAC != none )
	{
		FireLoopAC.FadeOut( 0.1f, 0.0f );
		FireLoopAC = None;
	}
}

simulated function PlayShellCaseEject()
{
	// shell eject particle system
	if( PSC_ShellEject != None )
	{
		if (IsTimerActive( 'HidePSC_ShellEject' ))
		{
			ClearTimer( 'HidePSC_ShellEject' );
			if (PSC_ShellEject != none)
			{
				PSC_ShellEject.ActivateSystem();
				PSC_ShellEject.RewindEmitterInstances();
			}
		}

		if (PSC_ShellEject.HiddenGame == TRUE)
		{
			PSC_ShellEject.SetHidden( FALSE );
			PSC_ShellEject.ActivateSystem();
		}
	}
}


simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{

	//`log(GetFUncName()@self@Instigator);
	// Start muzzle flash effect
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		if( (Instigator != None) )
		{
			PlayShellCaseEject();
		}

		// Play muzzle flash effect
		PlayMuzzleFlashEffect();
	}

	// Play fire effects for owned player
	if (WorldInfo.Netmode != NM_Client || (Instigator != None && Instigator.IsLocallyControlled()) || bDummyFireWeapon)
	{
		PlayOwnedFireEffects( FireModeNum, HitLocation );
	}

}

simulated function WeaponFired( byte FiringMode, optional vector HitLocation )
{
	//local Turret_TroikaBase Troika;
	Super.WeaponFired( FiringMode, HitLocation );

	if( !bSuppressAudio && (FireLoopAC == None || !FireLoopAC.IsPlaying()) )
	{
		FireLoopAC = GearWeaponPlaySoundLocalEx(FireLoopCue, FireLoopCue_Player, FireLoopAC);
	}

	if (!PSC_BarrelSmoke.bSuppressSpawning)
	{
		PSC_BarrelSmoke.DeactivateSystem();
	}

	ClearTimer(nameof(ReallyStoppedFiring));
}

simulated function EndFiringSounds()
{
	// stop any firing or spinning up sounds
	if ( (FireLoopAC != None) && FireLoopAC.IsPlaying() )
	{
		FireLoopAC.FadeOut(0.2f, 0.0f);

		/** last shot echo/tail off */
		GearWeaponPlaySoundLocal(FireStopCue, FireStopCue_Player,, 1.f);
		GearWeaponPlaySoundLocal(CasingImpactCue);
	}
	GearWeaponPlaySoundLocal(BarrelSpinningStopCue);

}



simulated function ReallyStoppedFiring()
{
	EndFiringSounds();
	PSC_BarrelSmoke.ActivateSystem(TRUE);
}
simulated function WeaponStoppedFiring(byte FiringMode)
{
	Super.WeaponStoppedFiring(FiringMode);
	SetTimer(0.1f,FALSE,nameof(ReallyStoppedFiring));
}

defaultproperties
{
	bWeaponCanBeReloaded=TRUE

	AmmoTypeClass=class'GearAmmoType_AssaultRifle'

	BS_MeleeAttack.Empty()

	FireOffset=(X=32,Y=0,Z=0)

	//FireSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleFireCue'

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_AssaultRifle.EffectS.P_COG_AssaultRifle_MuzzleFlash_Constant'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	MuzFlashParticleSystem=ParticleSystem'COG_AssaultRifle.EffectS.P_COG_AssaultRifle_MuzzleFlash_Constant'

	// Muzzle Flash point light
	Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=245,G=174,B=122,A=255)
	End Object
	MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
		Template=ParticleSystem'COG_AssaultRifle.Effects.P_GOG_AssaultRifle_Shells'
		Translation=(X=16,Z=6)
	End Object
	PSC_ShellEject=PSC_WeaponShellCaseComp


	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=222,V=100,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=305,UL=128,VL=34)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Assault'

	bInstantHit=true

	TracerType=WTT_MinigunFastFiring

	bAllowTracers=TRUE

	InstantHitDamageTypes(0)=class'GDT_SecurityBotGun'


}
