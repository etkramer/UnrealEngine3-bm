
/**
 * Kneeling over, targeting a mortar heavy weapon.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_TargetMortar extends GearSpecialMove;

/** GoW global macros */

/** Deployed animations */
var() protected			GearPawn.BodyStance		BS_PlayedStance, BS_Idl_DeployEnter, BS_Mid_DeployEnter, BS_MidLean_DeployEnter, BS_StdLean_DeployEnter;
var() const protected	SoundCue				MortarDeploySound;
/** If TRUE, weapon is mounted. Set after the transition animation to deploy the gun is played */
var		protected		bool					bIsMounted;

var ForceFeedbackWaveForm MountFF;

function bool CanOverrideMoveWith(ESpecialMove NewMove)
{
	if( bIsMounted && NewMove == SM_UnMountMortar )
	{
		return TRUE;
	}

	return FALSE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearWeap_HeavyBase	HeavyWeapon;
	local float					AnimPlayTime;

	Super.SpecialMoveStarted(bForced, PrevMove);

	// Heavy Weapon being carried.
	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);
	bIsMounted = FALSE;
	if( HeavyWeapon != None )
	{
		HeavyWeapon.SetMounted(FALSE);
	}

	// Remember what we used to mount heavy weapon.
	PawnOwner.HeavyMountedCoverAction	= ECoverAction(PawnOwner.SpecialMoveFlags >> 2);
	PawnOwner.HeavyMountedCoverType		= ECoverType(PawnOwner.SpecialMoveFlags & 0x3);

	switch( PawnOwner.HeavyMountedCoverType )
	{
		case CT_MidLevel :	
			if( PawnOwner.HeavyMountedCoverAction == CA_LeanLeft || PawnOwner.HeavyMountedCoverAction == CA_LeanRight )
			{
				BS_PlayedStance = BS_MidLean_DeployEnter;	
			}
			else
			{
				BS_PlayedStance = BS_Mid_DeployEnter;	
			}
			break;
		case CT_Standing :	
			BS_PlayedStance = BS_StdLean_DeployEnter;	
			break;		
		default:			
			BS_PlayedStance = BS_Idl_DeployEnter;	
			break;
	}

	AnimPlayTime = PawnOwner.BS_Play(BS_PlayedStance, SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE);
	// Notification when weapon will be mounted.
	PawnOwner.SetTimer( AnimPlayTime - 0.2f/SpeedModifier, FALSE, nameof(self.WeaponMountedNotify), self );

	// heavy lifting effort
	if(pawnOwner.GetTeamNum() == TEAM_COG)
	{
		// don't play effort on locust
		PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_LiftHeavyWeaponEffort, true);
	}

	PawnOwner.PlaySound(MortarDeploySound);

	// Play weapon mounting animation
	if( HeavyWeapon != None )
	{
		HeavyWeapon.PlayWeaponAnim('AR_Mortar_Deployed_Enter', SpeedModifier, 0.f, -1.f);
	}

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
	bShouldAbortWeaponReload = FALSE;

	// check to see if we interrupted an auto-reload
	if( PawnOwner.MyGearWeapon.ShouldAutoReload() )
	{
		PawnOwner.MyGearWeapon.ForceReload();
	}

	// Gun is mounted, check is Pawn wants to unmount it.
	if( PawnOwner != None && PawnOwner.IsLocallyControlled() )
	{
		PawnOwner.CheckHeavyWeaponMounting(TRUE);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearWeap_HeavyBase	HeavyWeapon;

	// Heavy Weapon being carried.
	HeavyWeapon = GearWeap_HeavyBase(PawnOwner.Weapon);
	if( HeavyWeapon != None )
	{
		HeavyWeapon.StopWeaponAnim(0.2f);
		HeavyWeapon.SetMounted(FALSE);
	}

	PawnOwner.ClearTimer('WeaponMountedNotify', self);

	bIsMounted = FALSE;
	PawnOwner.BS_Stop(BS_PlayedStance, 0.2f);

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
	BS_Idl_DeployEnter=(AnimName[BS_FullBody]="HW_Mortar_Deployed_Enter")
	BS_Mid_DeployEnter=(AnimName[BS_FullBody]="HW_Mortar_Cov_Mid2Deployed_Enter")
	BS_MidLean_DeployEnter=(AnimName[BS_FullBody]="HW_Mortar_Cov_Mid2Deployed_Lean_Enter")
	BS_StdLean_DeployEnter=(AnimName[BS_FullBody]="HW_Mortar_Cov_Std2Deployed_Lean_Enter")

	MortarDeploySound=SoundCue'Weapon_Mortar.Sounds.MortarDeploySetCue'

	bBreakFromCover=FALSE
	bCanFireWeapon=TRUE
	bLockPawnRotation=TRUE
	//bDisablePhysics=TRUE
	bCameraFocusOnPawn=FALSE
	bDisableLook=FALSE
	bShouldAbortWeaponReload=TRUE
	bDisableMovement=TRUE

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformMortarMount1
		Samples(0)=(LeftAmplitude=75,RightAmplitude=75,LeftFunction=WF_Noise,RightFunction=WF_Noise,Duration=0.3)
		bIsLooping=FALSE
	End Object
	MountFF=ForceFeedbackWaveformMortarMount1
}
