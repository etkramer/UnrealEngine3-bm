/**
 * Base class for "heavy" weapons.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_HeavyBase extends GearWeapon
	abstract
	native(Weapon)
	config(Weapon);

/** Weapon aim error for when weapon is mounted. */
var() protected const config float	MountedWeaponAimError;
/** AI aim error for when weapon is mounted */
var() config vector2d				AI_AccCone_Mounted_Min;
var() config vector2d				AI_AccCone_Mounted_Max;

/** Recoil value for when weapon is mounted. */
var() protected const config float	MountedWeaponRecoil;

var() const config float			MountedCameraFOV;

/** Camera offsets. */
var() protected const config vector		MountedCameraViewOffsetsLow;
var() protected const config vector		MountedCameraViewOffsetsMid;
var() protected const config vector		MountedCameraViewOffsetsHigh;
var() protected const config vector		MountedCameraViewOffsetsLow_OnLowCover;
var() protected const config vector		MountedCameraViewOffsetsMid_OnLowCover;
var() protected const config vector		MountedCameraViewOffsetsHigh_OnLowCover;
var() protected const config vector		MountedCameraViewOffsetsLow_LeanCover;
var() protected const config vector		MountedCameraViewOffsetsMid_LeanCover;
var() protected const config vector		MountedCameraViewOffsetsHigh_LeanCover;

/** How far we can adjust our aim either direction while targeting them mortar */
var() const config float			MaxTargetedAimAdjustYaw;
var() const config int				PitchLimitMountedMin;
var() const config int				PitchLimitMountedMax;

/** How to adjust the turn/lookup controls when the weapon is mounted. */
var() protected const config float	MountedLookRightScale;
var() protected const config float	MountedLookUpScale;


/** Socket name for the "stand" part of the weapon, where it rests when mounted. */
var() protected const Name				StandSocketName;
/** Particle system to play when weapon becomes mounted. */
var() protected const ParticleSystem	PS_MountedImpact;
/** Sound to play when weapon becomes mounted. */
var() protected const SoundCue			MountedImpactSound;

/** TRUE when weapon is mounted. FALSE when not or transitioning (being unmounted/mounted) */
var protected transient bool			bIsMounted;

/** Rotation audio vars. */
var protected const SoundCue			RotateLoopSound;
var protected transient AudioComponent	RotateLoopAC;
//var protected const SoundCue			RotateStartSound;
//var protected const SoundCue			RotateStopSound;

var protected transient bool			bPlayingRotAudio;
var() protected const float				RotAudioStartVelThreshold;
var() protected const float				RotAudioStopVelThreshold;
var protected transient int				LastControllerYaw;

var() protected const vector2d			RotAudioVolumeVelRange;
var() protected const vector2d			RotAudioVolumeRange;


/** Returns proper set of camera offsets for the current situation. */
simulated event GetMountedCameraOffsets(out vector OffsetLow, out vector OffsetMid, out vector OffsetHigh)
{
	local GearPawn InstigatorGP;

	InstigatorGP = GearPawn(Instigator);
	if( InstigatorGP != None && InstigatorGP.IsPoppingUp() )
	{
		OffsetLow = MountedCameraViewOffsetsLow_OnLowCover;
		OffsetMid = MountedCameraViewOffsetsMid_OnLowCover;
		OffsetHigh = MountedCameraViewOffsetsHigh_OnLowCover;
	}
	else if( InstigatorGP != None && InstigatorGP.IsLeaning() )
	{
		OffsetLow = MountedCameraViewOffsetsLow_LeanCover;
		OffsetMid = MountedCameraViewOffsetsMid_LeanCover;
		OffsetHigh = MountedCameraViewOffsetsHigh_LeanCover;
	}
	else
	{
		OffsetLow = MountedCameraViewOffsetsLow;
		OffsetMid = MountedCameraViewOffsetsMid;
		OffsetHigh = MountedCameraViewOffsetsHigh;
	}
}

/** updates whether or not we are mounted */
simulated function SetMounted(bool bNowMounted)
{
	bIsMounted = bNowMounted;
}

/** Returns TRUE if weapon is in a "mounted" state, false otherwise */
simulated final event bool IsMounted()
{
	return bIsMounted;
}

/** Returns TRUE if gun is being mounted. */
simulated function bool IsBeingMounted()
{
	local GearPawn	MyGearPawn;

	MyGearPawn = GearPawn(Instigator);
	return MyGearPawn != None && !bIsMounted && (MyGearPawn.IsDoingSpecialMove(SM_TargetMinigun) || MyGearPawn.IsDoingSpecialMove(SM_TargetMortar));
}

/** Returns TRUE if gun is being unmounted */
simulated function bool IsBeingUnMounted()
{
	local GearPawn	MyGearPawn;

	MyGearPawn = GearPawn(Instigator);
	return MyGearPawn != None && (MyGearPawn.IsDoingSpecialMove(SM_UnMountMinigun) || MyGearPawn.IsDoingSpecialMove(SM_UnMountMortar));
}


/**
 * Overridden version to force the player to unmount the weapon first.
 */
simulated function PutDownWeapon()
{
	// Make sure all pending fires are cleared.
	ForceEndFire();

	// Put weapon to sleep
	//@warning: must be before ChangedWeapon() because that can reactivate this weapon in some cases
	GotoState('Inactive');

	// Throw weapon
	Instigator.ThrowActiveWeapon();

	// Switch to pending weapon
	InvManager.ChangedWeapon();
}


/** No melee with heavy weapons. */
simulated function bool CanDoSpecialMeleeAttack()
{
	return FALSE;
}

/** Overridden to provide for more situational accuracy settings. */
simulated function float GetPlayerAimError()
{
	// if weapon is mounted, use special accuracy values
	if (IsMounted())
	{
		return MountedWeaponAimError;
	}
	else
	{
		// use normal accuracy values
		return super.GetPlayerAimError();
	}
}

function GetAIAccuracyCone(out vector2D AccCone_Min, out vector2D AccCone_Max)
{
	if (IsMounted())
	{
		AccCone_Min = AI_AccCone_Mounted_Min;
		AccCone_Max = AI_AccCone_Mounted_Max;
	}
	else
	{
		Super.GetAIAccuracyCone(AccCone_Min, AccCone_Max);
	}
}

/** Overridden to provide for more situational recoil settings. */
simulated function int GetWeaponRecoil()
{
	// if weapon is mounted, use special accuracy values
	if (IsMounted())
	{
		return MountedWeaponRecoil;
	}
	else
	{
		// use normal accuracy values
		return super.GetWeaponRecoil();
	}
}

simulated function GetRotationControlScale(out float TurnScale, out float LookUpScale)
{
	if (IsMounted())
	{
		TurnScale = MountedLookRightScale;
		LookUpScale = MountedLookUpScale;
	}
	else
	{
		TurnScale = 1.f;
		LookUpScale = 1.f;
	}
}

/** Triggered from AnimNotify to tell us the foot part of the weapon hit the ground (e.g. during mounting). */
simulated function Notify_MountedImpact()
{
	local Emitter Emitter;
	local vector SpawnLoc;

	// spawn particles
	if (PS_MountedImpact != None)
	{
		SkeletalMeshComponent(Mesh).GetSocketWorldLocationAndRotation('Stand', SpawnLoc);

		Emitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_MountedImpact, SpawnLoc, rot(0,0,0) );
		Emitter.ParticleSystemComponent.ActivateSystem();
	}

	// audio
	if (MountedImpactSound != None)
	{
		GearWeaponPlaySoundLocal(MountedImpactSound,,, 1.f);
	}

	// @fixme, do small camera shake here?
}

simulated function AttachWeaponTo(SkeletalMeshComponent MeshCpnt, optional Name SocketName)
{
	// can't mantle or climb while holding heavy weapon
	Instigator.bCanMantle = false;
	Instigator.bCanClimbUp = false;
	Instigator.bCanClimbLadders = false;

	Super.AttachWeaponTo(MeshCpnt, SocketName);
}

simulated function DetachWeapon()
{
	if (Instigator != None)
	{
		Instigator.bCanMantle = Instigator.default.bCanMantle;
		Instigator.bCanClimbUp = Instigator.default.bCanClimbUp;
		Instigator.bCanClimbLadders = Instigator.default.bCanClimbLadders;
	}
	Super.DetachWeapon();
}

simulated function bool ShouldDrawCrosshair()
{
	return (IsMounted() || Super.ShouldDrawCrosshair());
}

simulated function Tick(float DeltaTime)
{
	local bool bPlayRotAudio;
	local float AimRotVel, RotAudioPct;

	// update rotation audio
	if ( (Instigator != None) && (RotateLoopSound != None) )
	{
		if (IsMounted())
		{
			if (Instigator.Controller != None)
			{
				AimRotVel = Abs(float(Instigator.Controller.Rotation.Yaw - LastControllerYaw) / DeltaTime);
			}
			else
			{
				//@fixme - is the correct value to use in this case?
				AimRotVel = Abs(float(Instigator.Rotation.Yaw - LastControllerYaw) / DeltaTime);
			}
			if ( (!bPlayingRotAudio && (AimRotVel > RotAudioStartVelThreshold)) ||
				(bPlayingRotAudio && (AimRotVel > RotAudioStopVelThreshold)) )
			{
				bPlayRotAudio = TRUE;
			}
		}

		if (bPlayRotAudio)
		{
			if (!bPlayingRotAudio)
			{
				RotateLoopAC = GearWeaponPlaySoundLocalEx(RotateLoopSound,, RotateLoopAC, 0.15f);
				bPlayingRotAudio = (RotateLoopAC != None);
			}
		}
		else
		{
			if (bPlayingRotAudio)
			{
				RotateLoopAC.Stop();
				bPlayingRotAudio = FALSE;
			}
		}

		if (bPlayingRotAudio)
		{
			RotAudioPct = FClamp(GetRangePctByValue(RotAudioVolumeVelRange, AimRotVel), 0.f, 1.f);
			RotateLoopAC.VolumeMultiplier = GetRangeValueByPct(RotAudioVolumeRange, RotAudioPct);
		}

		if (Instigator.Controller != None)
		{
			LastControllerYaw = Instigator.Controller.Rotation.Yaw;
		}
	}

	super.Tick(DeltaTime);

}

simulated function PlayEquipAnimation(float BlendInTime);
simulated function PlayHolsterAnimation();


simulated function AutoSwitchToNewWeapon()
{
	// don't autoswitch during the unmount, it messes things up (anim end notifications)
	if (!ShouldPreventWeaponSwitch())
	{
		GearInventoryManager(InvManager).AutoSwitchWeapon();
	}
}

simulated function bool ShouldPreventWeaponSwitch()
{
	if (IsMounted() || IsBeingUnMounted())
	{
		return TRUE;
	}

	return FALSE;
}

simulated function MaybeAutoSwitchWeapon()
{
	if (!HasAnyAmmo())
	{
		AutoSwitchToNewWeapon();
	}
}


defaultproperties
{
	WeaponType=WT_Heavy

	bAllowsRoadieRunning=FALSE
	bWeaponCanBeReloaded=TRUE
	bBlindFirable=FALSE
	bSupports360AimingInCover=FALSE
	bNoInteractionWhileEquipped=TRUE	// heavy weapons prevent the player from interacting with anything
	bPlayIKRecoil=FALSE
	bAllowIdleStance=FALSE
	bAllowAimingStance=TRUE	// Force into aiming all the time.
	bAllowDownsightsStance=FALSE
	AIRating=2.0

	StandSocketName="Stand"

	bCanSelectWithoutAmmo=FALSE
	bCanNegateMeatShield=true
}
