
/**
 * Kneeling, targeting a "deployed" minigun heavy weapon.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_TargetMinigun extends GearSpecialMove;

/** Deployed animations */
var() const protected	GearPawn.BodyStance	BS_DeployEnter, BS_DeployIdle, BS_DeployEnterCover, BS_DeployIdleCover, BS_DeployEnterStdCover, BS_DeployEnterMidCoverLean;
var() const protected	SoundCue			MinigunDeploySound_LowCover;
var() const protected	SoundCue			MinigunDeploySound_Ground;
/** If TRUE, weapon is mounted. Set after the transition animation to deploy the gun is played */
var		protected		bool					bIsMounted;

var ForceFeedbackWaveForm MountFF;

function bool CanOverrideMoveWith(ESpecialMove NewMove)
{
	if( bIsMounted && NewMove == SM_UnMountMinigun )
	{
		return TRUE;
	}

	return FALSE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearWeap_HeavyBase	HeavyWeapon;
	local float					AnimPlayTime;

	// Heavy Weapon being carried.
	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);

	bIsMounted = FALSE;
	if( HeavyWeapon != None )
	{
		HeavyWeapon.SetMounted(FALSE);
	}

	Super.SpecialMoveStarted(bForced, PrevMove);

	// Remember what we used to mount heavy weapon.
	PawnOwner.HeavyMountedCoverAction	= ECoverAction(PawnOwner.SpecialMoveFlags >> 2);
	PawnOwner.HeavyMountedCoverType		= ECoverType(PawnOwner.SpecialMoveFlags & 0x3);

	if( PawnOwner.HeavyMountedCoverType == CT_MidLevel && PawnOwner.HeavyMountedCoverAction == CA_PopUp )
	{
		AnimPlayTime = PawnOwner.BS_Play(BS_DeployEnterCover, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE);
		PawnOwner.BS_SetAnimEndNotify(BS_DeployEnterCover, TRUE);

		// Play weapon mounting animation on top of cover.
		if( HeavyWeapon != None )
		{
			HeavyWeapon.PlayWeaponAnim('HW_Gatling_Cov_Mid_Deployed_Enter', SpeedModifier, 0.f, -1.f, FALSE, TRUE);
		}

		PawnOwner.PlaySound(MinigunDeploySound_LowCover);
	}
	else
	{
		// Mid Level Cover Leaning
		if( PawnOwner.HeavyMountedCoverType == CT_MidLevel )
		{
			AnimPlayTime = PawnOwner.BS_Play(BS_DeployEnterMidCoverLean, SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE);
		}
		// Standing cover leaning
		else if( PawnOwner.HeavyMountedCoverType == CT_Standing )
		{
			AnimPlayTime = PawnOwner.BS_Play(BS_DeployEnterStdCover, SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE);
		}
		// Out in the open mounting
		else
		{
			AnimPlayTime = PawnOwner.BS_Play(BS_DeployEnter, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE);
			PawnOwner.BS_SetAnimEndNotify(BS_DeployEnter, TRUE);
		}

		// Play weapon mounting animation outside of cover.
		if( HeavyWeapon != None )
		{
			HeavyWeapon.PlayWeaponAnim('HW_Gatling_Deployed_Enter', SpeedModifier, 0.f, -1.f, FALSE, TRUE);
		}

		PawnOwner.PlaySound(MinigunDeploySound_Ground);
	}

	// Notification when weapon will be mounted.
	PawnOwner.SetTimer( AnimPlayTime - 0.2f/SpeedModifier, FALSE, nameof(self.WeaponMountedNotify), self );

	// heavy lifting effort
	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_LiftHeavyWeaponEffort, true);

	if( PCOwner != None )
	{
		PCOwner.TargetHeavyWeaponBaseRotationYaw = PawnOwner.Rotation.Yaw;

		if (InterpActor_GearBasePlatform(PawnOwner.Base) != None)
		{
			// relative to base if on a baseplatform
			PCOwner.TargetHeavyWeaponBaseRotationYaw -= PawnOwner.Base.Rotation.Yaw;
		}
	}
}

/** Notification called when the deploy animation is finished playing. */
function WeaponMountedNotify()
{
	local GearWeap_HeavyBase	HeavyWeapon;
	local GearPC PC;

	if (PCOwner != None && PCOwner.IsLocalPlayerController())
	{
		PCOwner.ClientPlayForceFeedbackWaveform(MountFF);
	}

	// Weapon is now mounted.
	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);
	if( HeavyWeapon != None )
	{
		HeavyWeapon.SetMounted(TRUE);
	}

	bIsMounted = TRUE;

	// Gun is mounted, check is Pawn wants to unmount it.
	if( PawnOwner != None && PawnOwner.IsLocallyControlled() )
	{
		// Give the GearPC a chance to act on the cover mounting of the minigun
		PC = GearPC(PawnOwner.Controller);
		if ( PC != None && PawnOwner.HeavyMountedCoverType != CT_None )
		{
			PC.MinigunCoverMountedStateChanged(TRUE);
		}

		PawnOwner.CheckHeavyWeaponMounting(TRUE);
	}
}

/** Finished playing the mount animation, transition to a looping idle animation. */
function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( PawnOwner.HeavyMountedCoverType == CT_MidLevel )
	{
		PawnOwner.BS_SetAnimEndNotify(BS_DeployEnterCover, FALSE);
		PawnOwner.BS_Play(BS_DeployIdleCover, 1.f, 0.2f/SpeedModifier, 0.f, TRUE);
	}
	else
	{
		PawnOwner.BS_SetAnimEndNotify(BS_DeployEnter, FALSE);
		PawnOwner.BS_Play(BS_DeployIdle, 1.f, 0.2f/SpeedModifier, 0.f, TRUE);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearWeap_HeavyBase	HeavyWeapon;
	local GearPC PC;

	// Heavy Weapon being carried.
	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);

	if( HeavyWeapon != None )
	{
		HeavyWeapon.StopWeaponAnim(0.2f);
		HeavyWeapon.SetMounted(FALSE);
	}

	bIsMounted = FALSE;

	PawnOwner.ClearTimer('WeaponMountedNotify', self);

	// Stop played animations in case we don't get aborted properly.
	PawnOwner.BS_Stop(BS_DeployEnterCover, 0.2f);
	PawnOwner.BS_Stop(BS_DeployEnter, 0.2f);
	PawnOwner.BS_Stop(BS_DeployEnterStdCover, 0.2f);
	PawnOwner.BS_Stop(BS_DeployIdle, 0.2f);
	PawnOwner.BS_Stop(BS_DeployIdleCover, 0.2f);
	PawnOwner.BS_Stop(BS_DeployEnterMidCoverLean, 0.2f);

	// Gun is mounted, check is Pawn wants to unmount it.
	if( PawnOwner.IsLocallyControlled() )
	{
		// Give the GearPC a chance to act on the cover dismounting of the minigun
		PC = GearPC(PawnOwner.Controller);
		if ( PC != None && PawnOwner.HeavyMountedCoverType != CT_None )
		{
			PC.MinigunCoverMountedStateChanged(FALSE);
		}
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

/** Called from GearPawn to give special moves a chance to limit view rotation. */
function LimitViewRotation(out Rotator out_ViewRotation)
{
	local GearWeap_HeavyBase	HeavyWeapon;
	local int BaseYaw;

	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);
	if( HeavyWeapon != None )
	{
		BaseYaw = PCOwner.TargetHeavyWeaponBaseRotationYaw;
		if (InterpActor_GearBasePlatform(PawnOwner.Base) != None)
		{
			BaseYaw += PawnOwner.Base.Rotation.Yaw;
		}

		out_ViewRotation.Yaw	= ClampRotAxisFromBase(out_ViewRotation.Yaw, BaseYaw, HeavyWeapon.MaxTargetedAimAdjustYaw);
		out_ViewRotation.Pitch	= ClampRotAxisFromRange(out_ViewRotation.Pitch, HeavyWeapon.PitchLimitMountedMin, HeavyWeapon.PitchLimitMountedMax);
	}
}

/** Don't override AimOffset */
function bool GetAimOffsetOverride(out rotator DeltaRot)
{
	return FALSE;
}

function Tick(float DeltaTime)
{
	local GearWeap_HeavyBase HeavyWeap;

	Super.Tick(DeltaTime);

	if( PawnOwner.IsLocallyControlled() )
	{
		// If we ever lose our weapon, then exit this special move.
		HeavyWeap = GearWeap_HeavyBase(PawnOwner.Weapon);
		if( HeavyWeap == None )
		{
			PawnOwner.LocalEndSpecialMove();
		}
	}
}

function PreProcessInput(GearPlayerInput Input)
{
	// manually disable movement
	Input.RawJoyRight = 0.f;
	Input.RawJoyUp = 0.f;
}

defaultproperties
{
	BS_DeployEnter=(AnimName[BS_FullBody]="HW_Gatling_Deployed_Enter")
	BS_DeployIdle=(AnimName[BS_FullBody]="HW_Gatling_Deployed_Idle")
	BS_DeployEnterCover=(AnimName[BS_FullBody]="HW_Gatling_Cov_Mid_Deployed_Enter")
	BS_DeployIdleCover=(AnimName[BS_FullBody]="HW_Gatling_Cov_Mid_Deployed_Idle")
	BS_DeployEnterStdCover=(AnimName[BS_FullBody]="HW_Gatling_Cov_Std2Deployed_Lean_Enter")
	BS_DeployEnterMidCoverLean=(AnimName[BS_FullBody]="HW_Gatling_Cov_Mid2Deployed_Lean_Enter")

	MinigunDeploySound_LowCover=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingLowerCoverCue'
	MinigunDeploySound_Ground=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingLowerGroundCue'

	//bShouldAbortWeaponReload=TRUE
	bBreakFromCover=FALSE
	bCanFireWeapon=TRUE
	bLockPawnRotation=TRUE
	//bDisablePhysics=TRUE
	bCameraFocusOnPawn=FALSE
	bDisableLook=FALSE
	bDisableMovement=TRUE

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformMinigunMount1
		Samples(0)=(LeftAmplitude=75,RightAmplitude=75,LeftFunction=WF_Noise,RightFunction=WF_Noise,Duration=0.3)
		bIsLooping=FALSE
	End Object
	MountFF=ForceFeedbackWaveformMinigunMount1
}
