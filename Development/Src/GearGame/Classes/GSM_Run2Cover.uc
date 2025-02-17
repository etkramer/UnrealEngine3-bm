
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Run2Cover extends GearSpecialMove
	native(SpecialMoves);

var()	GearPawn.BodyStance	BS_TravelAnimation;
var()	GearPawn.BodyStance	BS_TravelAnimationMirrored;
var()	GearPawn.BodyStance	BS_ImpactAnimation;

var()	float	MinTravelAnimDuration;

/** TRUE if joystick monitoring is allowed */
var bool	bEnableJoystickMonitoring;

/** Monitor how long player is holding down joystick */
var	float	HoldingRight, HoldingLeft, HoldingBack;

/** Minimum time required to hold joystick in one direction, to have the player face that way. */
var float	MinHoldTimeToSwitchMirroring;

var config	float	AISpeedModifier;
var config	float	ImpactAnimSpeedModifier;

// C++ functions
cpptext
{
	virtual void	TickSpecialMove(FLOAT DeltaTime);
}

simulated function bool CanChainMove(ESpecialMove NextMove)
{
	// allow all the cover transition moves after run2cover
	return ( NextMove == SM_CoverSlip || NextMove == SM_CoverRun || NextMove == SM_MidLvlJumpOver ||
		NextMove == SM_MantleUpLowCover || NextMove == SM_StdLvlSwatTurn );
}


/**
 * Event called when Special Move is started.
 */
function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	bEnableJoystickMonitoring = FALSE;
	if( PawnOwner.IsLocallyControlled() )
	{
		if( PCOwner != None )
		{
			bEnableJoystickMonitoring	= TRUE;
			HoldingRight				= 0.f;
			HoldingLeft					= 0.f;
			HoldingBack					= 0.f;
			GearPlayerInput(PCOwner.PlayerInput).ForcePitchCentering(TRUE,, 10.f);
		}
	}

	// Disable root motion for run 2 cover
	PawnOwner.Mesh.RootMotionMode = RMM_Ignore;

	if (PawnOwner.Controller != None)
	{
		// Send player to cover location
		PawnOwner.Controller.SetDestinationPosition( PawnOwner.AcquiredCoverInfo.Location, TRUE );
		PawnOwner.Controller.bPreciseDestination	= TRUE;
	}
	PlayTransitionAnimation();
}

/**
 * Overridden to not replicate this move - it is instead simulated by receipt of GearPawn.AcquiredCoverInfo, since we can't properly execute
 * this move w/o that information.
 */
function bool ShouldReplicate()
{
	return FALSE;
}

protected function bool InternalCanDoSpecialMove()
{
	// make sure we're in cover
	return (PawnOwner.CoverType != CT_None && PawnOwner.AcquiredCoverInfo.Link != None);
}


function PlayTransitionAnimation()
{
	local bool	bShouldBeMirrored;
	local float	PlayDuration, BlendInTime, Rate;

	// Need to use GetMaxSpeedModifier, as it's doing more voodoo than just GroundSpeed * SpeedModifier.
	Rate = PawnOwner.GetMaxSpeedModifier();

	// Based on distance from cover, figure out the time it would take to travel there.
	PlayDuration	= VSize(PawnOwner.AcquiredCoverInfo.Location - PawnOwner.Location) / (PawnOwner.GroundSpeed * Rate);
	// Need a minium of time...
	PlayDuration	= FMax(PlayDuration, default.MinTravelAnimDuration);

	// Adjust BlendInTime, so animation can be blended fully
	BlendInTime		= FMin(0.2f/Rate, PlayDuration * 0.5f);

	// if we are sliding into the wall at a decent distance play the wall impact sound
	if( PlayDuration > 0.25f )
	{
		PawnOwner.PlaySlidingSound();
	}

	// TRUE = play animation. FALSE, bypass anim, just use standard running.
	if( TRUE )
	{
		// See if we should acquire cover being mirrored
		bShouldBeMirrored = (PawnOwner.FindBestCoverSideFor(PawnOwner.AcquiredCoverInfo) == CD_Left);

		if( bShouldBeMirrored )
		{
			PawnOwner.BS_PlayByDuration(default.BS_TravelAnimationMirrored, PlayDuration, BlendInTime, -1.f, FALSE, TRUE);

			// Play animation mirrored
			PawnOwner.BS_SetMirrorOptions(default.BS_TravelAnimationMirrored, TRUE, TRUE, FALSE);

			// If weapon is a heavy weapon, then don't do an early blend out. We need the animation to be over before we can re-enable IK Controllers.
			if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
			{
				PawnOwner.BS_SetEarlyAnimEndNotify(default.BS_TravelAnimationMirrored, FALSE);
			}
		}
		else
		{
			// Play Animation
			PawnOwner.BS_PlayByDuration(default.BS_TravelAnimation, PlayDuration, BlendInTime, -1.f, FALSE, TRUE);
		}

		// Enable end of animation notification. This is going to call SpecialMoveEnded()
		PawnOwner.BS_SetAnimEndNotify(default.BS_TravelAnimation, TRUE);
	}
	else
	{
		PawnOwner.SetTimer( PlayDuration, FALSE, nameof(self.EndRun2Cover), self );
	}

	PawnOwner.InterpolatePawnRotation(PlayDuration);
}

function EndRun2Cover()
{
	PawnOwner.EndSpecialMove();
}

/**
 * Joystick has been held long enough during transition,
 * So mirror ahead of time, before impact, so i can be properly replicated.
 */
simulated event JoystickHeldLongEnough(GearPC PC)
{
	// stop monitoring the joystick
	bEnableJoystickMonitoring = FALSE;

	// if holding back then abort the special move and cover
	if (HoldingBack > 0.f)
	{
		PC.AbortRun2Cover();
	}
	else
	{
		// otherwise set the pawn side to match the joystick direction
		PawnOwner.SetMirroredSide( HoldingLeft >= MinHoldTimeToSwitchMirroring );
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearPC		PC;
	local vector	CamLoc;
	local rotator	CamRot;
	local bool		bForceNoBlend;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.ClearTimer('EndRun2Cover', Self);

	// Make sure rotation interpolation is finished so it doesn't mess any moves such mantles over cover.
	if( IsPawnRotationLocked() || PawnOwner.PawnRotationBlendTimeToGo > 0.f )
	{
		PawnOwner.PawnRotationBlendTimeToGo = 0.f;
		PawnOwner.FaceRotation(PawnOwner.Rotation, 0.f);
	}

	// Can't do a blend out with heavy weapons when mirrored. argh.
	bForceNoBlend = (PawnOwner.MyGearWeapon.WeaponType == WT_Heavy);

	//`log(self@GetFuncName()@PawnOwner.AcquiredCoverInfo.Link);
	if (PawnOwner.AcquiredCoverInfo.Link != None)
	{
		// reset the cover time to avoid issues w/ immediately pushing out of cover
		PawnOwner.LastCoverTime = PawnOwner.WorldInfo.TimeSeconds;

		// Play Wall impact effects
		PawnOwner.DoRun2CoverWallHitEffects();

		// Turn off mirroring
		PawnOwner.BS_SetMirrorOptions(BS_TravelAnimation, FALSE, FALSE, FALSE);

		// Turn off AnimEnd notification
		PawnOwner.BS_SetAnimEndNotify(BS_TravelAnimation, FALSE);

		// See if player was holding stick at the moment of impact.
		PC = GearPC(PawnOwner.Controller);
		if( bEnableJoystickMonitoring && PC != None )
		{
			if( HoldingLeft > 0.f )
			{
				PawnOwner.SetMirroredSide(TRUE);
			}
			else if( HoldingRight > 0.f )
			{
				PawnOwner.SetMirroredSide(FALSE);
			}
		}

		// Play wall impact effect only if there is no mirror transition pending,
		// For better responsiveness.
		if( PawnOwner.bWantsToBeMirrored == PawnOwner.bIsMirrored )
		{
			// Play Animation
			PawnOwner.BS_Play(BS_ImpactAnimation, ImpactAnimSpeedModifier, bForceNoBlend ? 0.f : 0.05f, 0.33f/ImpactAnimSpeedModifier, FALSE, TRUE);
		}
		else
		{
			// If not playing impact animation, do a gentle blend out
			PawnOwner.BS_Stop(BS_TravelAnimation, bForceNoBlend ? 0.f : 0.1f);
			PawnOwner.BS_Stop(BS_TravelAnimationMirrored, bForceNoBlend ? 0.f : 0.1f);
		}

		if( PawnOwner.Controller != None )
		{
			// turn off forced movement
			PawnOwner.Controller.bPreciseDestination = FALSE;
			if( PC != None && PC.IsInCoverState() )
			{
				// remap controls once we've actually gotten into cover, to account for any rotation between start/end of anim
				PC.GetPlayerViewPoint(CamLoc, CamRot);
				PC.ControlsRemapRotation = CamRot;

				// if they're holding 'a' still try another special move at this point
				if (PC.IsHoldingActionButton())
				{
					PC.TryASpecialMove(FALSE);
				}
			}
		}

		// Restore default root motion mode
		PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;

		// Clear movement, otherwise it triggers a move2idle transition when getting in cover.
		PawnOwner.ZeroMovementVariables();

		// Send event telling Pawn cover slot has been reached.
		if( PawnOwner.CurrentSlotPct == 0.f || PawnOwner.CurrentSlotPct == 1.f )
		{
			PawnOwner.ReachedCoverSlot(PawnOwner.CurrentSlotIdx);
		}
	}
	else
	{
		// If not playing impact animation, do a gentle blend out
		PawnOwner.BS_Stop(BS_TravelAnimation, bForceNoBlend ? 0.f : 0.1f);
		PawnOwner.BS_Stop(BS_TravelAnimationMirrored, bForceNoBlend ? 0.f : 0.1f);

		// Restore default root motion mode
		PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
		if( PawnOwner.Controller != None )
		{
			// turn off forced movement
			PawnOwner.Controller.bPreciseDestination = FALSE;
		}
	}

	if (PCOwner != None && GearPlayerInput(PCOwner.PlayerInput) != None)
	{
		GearPlayerInput(PCOwner.PlayerInput).ForcePitchCentering(FALSE);
	}
}

defaultproperties
{
	MinTravelAnimDuration=0.2f
	MinHoldTimeToSwitchMirroring=0.25f

	bCanFireWeapon=FALSE
	bLockPawnRotation=FALSE
	bDisableMovement=TRUE

	bDisableAI=TRUE
}
