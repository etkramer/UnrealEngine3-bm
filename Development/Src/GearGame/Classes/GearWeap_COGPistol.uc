/**
 * COG MX-8 "Snub" Pistol
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_COGPistol extends GearWeap_PistolBase;

/** range of multiplier for MP AI firing speed to simulate the slowdown from having to press the button for each shot */
var config vector2D MPAI_RateOfFire_Scale;

simulated function float GetRateOfFire()
{
	local float Result;

	Result = Super.GetRateOfFire();
	if (GearAI_TDM(GearAIController) != None)
	{
		Result *= RandRange(MPAI_RateOfFire_Scale.X, MPAI_RateOfFire_Scale.Y);
	}
	return Result;
}

/**
 * State WeaponFiring
 * Fires a bullet at a time
 */

simulated state WeaponFiring
{
	/** @see GearWeapon::ShouldRefire() */
	simulated function bool ShouldRefire()
	{
		// For Players: in single fire mode, it is not possible to refire. You have to release and re-press fire to shot every time
		if( Instigator != None && Instigator.IsHumanControlled() )
		{
			EndFire( CurrentFireMode );
			Instigator.Controller.bFire = 0;
			
			return FALSE;
		}

		return Global.ShouldRefire();
	}

	simulated function EndState( Name NextStateName )
	{
		// do not clear flash location. Because rate of fire is too small, and weapon is release to refire. replication sometimes fail to happen.
		//ClearFlashCount();
		DelayedWeaponStoppedFiring();
		ClearTimer('RefireCheckTimer');
	}
}


/**
 * ROF is very high, so on low frame rates, server can't send FlashLocation fast enough, it is reset to 0 when weapon stops firing.
 * Same problem can happen on clients who receive the packets in the same tick. Weapon will be fired and stopped at the same time.
 * So we just replicate the shot, and stop the weapon in GetFireInterval(FireModeNum) for remote clients.
 * ClearFlashCount() is not called at all to avoid resetting the location.
 */

simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	if( FireModeNum == 0 && bSlaveWeapon )
	{
		SetTimer( GetFireInterval(FireModeNum), false, nameof(DelayedWeaponStoppedFiring) );
	}

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// decrease animation play rate. ROF is fast, don't make anim ridiculously fast as well.
		PlayWeaponAnimByDuration(WeaponFireAnim, GetFireInterval(FireModeNum)/FireAnimRateScale);
	}

	super.PlayFireEffects(FireModeNum, HitLocation);
}

simulated function DelayedWeaponStoppedFiring()
{
	WeaponStoppedFiring( Instigator.FiringMode );
	//Instigator.WeaponStoppedFiring( bSlaveWeapon );
}


simulated function PlayWeaponReloading()
{
	ClearTimer('DelayedWeaponStoppedFiring');
	super.PlayWeaponReloading();
}

simulated function EjectClip()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipEjectCue');
}
simulated function CockOpen()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolCockOpenCue');
}
simulated function CockClose()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolCockCloseCue');
}

simulated function InsertClip()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipReloadInsertCue');
}

simulated function Jammed()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Actions.CogPistolDropCue');
}

simulated function HandSlam()
{
	WeaponPlaySound(SoundCue'Weapon_AssaultRifle.Reloads.CogRifleJammedHandCue');
}
simulated function ClipImpact()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipEjectImpactCue');
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.COGPistol;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.COGPistol;
}
*/


// Base Weapon version of this function just casts Animations to an AnimNodeSequence, but here it is
// an AnimTree, so we can have controls. So we dig into the tree and grab the only child.
simulated function AnimNodeSequence GetWeaponAnimNodeSeq()
{
	local SkeletalMeshComponent SkelMesh;
	local AnimNodeSequence AnimSeq;

	SkelMesh = SkeletalMeshComponent(Mesh);
	AnimSeq = AnimNodeSequence(AnimTree(SkelMesh.Animations).Children[0].Anim);
	return AnimSeq;
}


/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease()
{
	local SkeletalMeshComponent	SkelMesh;
	local Vector				MagLoc;
	local Rotator				MagRot;

	//`log( self @ 'Notify_AmmoRelease' );

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Hide Ammo part on weapon mesh
	SetWeaponAmmoBoneDisplay( FALSE );

	// Spawn physics magazine instead
	SkelMesh = SkeletalMeshComponent(Mesh);
	SkelMesh.GetSocketWorldLocationAndRotation('Magazine', MagLoc, MagRot);

	// Spawn physics Magazine
	SpawnPhysicsMagazine(MagLoc, MagRot);
}


/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab()
{
	//`log( self @ 'Notify_AmmoGrab' );

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Attach magazine mesh to left hand
	SetPawnAmmoAttachment(TRUE);
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload()
{
	//`log( self @ 'Notify_AmmoReload' );

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Detach magazine mesh from instigator's left
	SetPawnAmmoAttachment(FALSE);

	// Show Ammo part on weapon mesh.
	SetWeaponAmmoBoneDisplay(TRUE);
}

defaultproperties
{
	FireAnimRateScale=0.5
	FireOffset=(X=19,Y=0.5,Z=4)

	bWeaponCanBeReloaded=TRUE

	InstantHitDamageTypes(0)=class'GDT_COGPistol'
	AmmoTypeClass=class'GearAmmoType_Pistol'

	FireSound=SoundCue'Weapon_Pistol.Firing.CogPistolFireCue'
	FireSound_Player=SoundCue'Weapon_Pistol.Firing.CogPistolFirePlayerCue'
	FireNoAmmoSound=SoundCue'Weapon_Pistol.Firing.CogPistolFireEmptyCue'

	WeaponWhipSound=SoundCue'Weapon_Pistol.Firing.CogPistolWhipCue'

	WeaponReloadSound=None

	WeaponEquipSound=SoundCue'Weapon_Pistol.Actions.CogPistolRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_Pistol.Actions.CogPistolLowerCue'
	PickupSound=SoundCue'Weapon_Pistol.Actions.CogPistolPickupCue'
	WeaponDropSound=SoundCue'Weapon_Pistol.Actions.CogPistolDropCue'

	// Weapon Mesh
	Begin Object Name=WeaponMesh
	    SkeletalMesh=SkeletalMesh'COG_Pistol.COG_Pistol'
		PhysicsAsset=PhysicsAsset'COG_Pistol.COG_Pistol_Physics'
		AnimTreeTemplate=AnimTree'COG_Pistol.AT_COG_Pistol'
		AnimSets(0)=AnimSet'COG_Pistol.COG_Pistol'
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_Pistol.Effects.P_COG_Pistol_Muzzleflash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	MuzFlashParticleSystem=ParticleSystem'COG_Pistol.Effects.P_COG_Pistol_Muzzleflash'
	MuzFlashParticleSystemActiveReload=ParticleSystem'COG_Pistol.Effects.P_COG_Pistol_Muzzleflash_AR'

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
		Template=ParticleSystem'COG_Pistol.Effects.P_GOG_Pistol_shell'
	End Object
	PSC_ShellEject=PSC_WeaponShellCaseComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'COG_Pistol.Effects.P_COG_Pistol_Smoke'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=247,G=123,B=57,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.2f
	MuzzleLightPulseFreq=20
	MuzzleLightPulseExp=1.5

	DamageTypeClassForUI=class'GDT_COGPistol'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=222,V=147,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=40,UL=128,VL=39)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Snub'

	bIsSuppressive=FALSE
	bInstantHit=TRUE
	AIRating=0.4

	NeedReloadNotifyThreshold=3

	HUDDrawData			= (DisplayCount=12,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=288,UL=106,VL=7),ULPerAmmo=9)
	HUDDrawDataSuper	= (DisplayCount=12,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=288,UL=106,VL=7),ULPerAmmo=9)

	MeleeImpactSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_HitCue'

	LC_EmisDefaultCOG=(R=5.0,G=5.0,B=10.0,A=1.0)
	LC_EmisDefaultLocust=(R=60.0,G=2.0,B=0.2,A=1.0)

	Recoil_Hand={(
				TimeDuration=0.25f,
				RotAmplitude=(X=3000,Y=200,Z=0),
				RotFrequency=(X=10,Y=10,Z=0),
				RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				LocAmplitude=(X=-6,Y=0,Z=2),
				LocFrequency=(X=10,Y=0,Z=10),
				LocParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero)
				)}

	Recoil_Spine={(
				TimeDuration=0.67f,
				RotAmplitude=(X=500,Y=100,Z=0),
				RotFrequency=(X=10,Y=10,Z=0),
				RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero)
				)}


	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=50,RightAmplitude=50,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.200)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

    WeaponID=WC_Snub

	BS_PawnWeaponReload={(
		AnimName[BS_Hostage_Upper]		="Hostage_Pistol_reload",
		AnimName[BS_Kidnapper_Upper]	="Kidnapper_Pistol_reload"
	)}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_Hostage_Upper]		="Hostage_Pistol_reload_fail",
		AnimName[BS_Kidnapper_Upper]	="Kidnapper_Pistol_reload_fail"
	)}

	bMuzFlashEmitterIsLooping=FALSE

	Begin Object Class=StaticMeshComponent Name=MagazineMesh0
		StaticMesh=StaticMesh'COG_Pistol.COG_Pistol_Magazine'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object
	MagazineMesh=MagazineMesh0
}
