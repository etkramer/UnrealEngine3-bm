
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_BloodMount_MantleOverCover extends GSM_NonCoverUserMantleOverCover;

/** Animations for Rider */
var GearPawn	Rider;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Get pointer to the Rider.
	Rider = GearPawn_LocustBloodmount(PawnOwner).Driver;
}

simulated function PlayJump()
{
	Super.PlayJump();

	if( Rider != None )
	{
		Rider.BS_Play(BS_PlayedStance, SpeedModifier, 0.25f/SpeedModifier, -1.f, FALSE, TRUE);
	}
}

simulated function PlayFallAnimation(float StartTime)
{
	Super.PlayFallAnimation(StartTime);

	if( Rider != None )
	{
		Rider.BS_Play(BS_PlayedStance, SpeedModifier * 0.3f, 0.15f/SpeedModifier, -1.f, FALSE, TRUE);

		// Our slot node has already been ticked, so it's going to remain stuck on the end of the jump animation on this frame.
		// and create a pop. So force a blend, so we can see the fall animation.
		// => smooth transition.
		Rider.BS_AccelerateBlend(BS_PlayedStance, StartTime);

		// Adjust animation transition, so it is smooth
		if( StartTime > 0.f )
		{
			Rider.BS_SetPosition(BS_PlayedStance, StartTime * SpeedModifier);
		}
	}
}

/** Adjust play rate to match fall duration */
simulated function AdjustFallingPlayRate(float FallDuration)
{
	local float TimeLeft, PlayRate;

	if( Rider != None )
	{
		TimeLeft = Rider.BS_GetTimeLeft(BS_PlayedStance);
		PlayRate = TimeLeft / FallDuration;

		Rider.BS_SetPlayRate(BS_PlayedStance, Rider.BS_GetPlayRate(BS_PlayedStance) * PlayRate);
	}
}

simulated function PlayLand()
{
	Super.PlayLand();
	if( Rider != None )
	{
		Rider.BS_Play(BS_PlayedStance, SpeedModifier, 0.05f/SpeedModifier, -1.f, FALSE, TRUE);
	}
}

simulated function StopLand()
{
	Super.StopLand();
	if( Rider != None )
	{
		PawnOwner.BS_Stop(BS_PlayedStance, 0.33f/SpeedModifier);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);
	if( Rider != None )
	{
		Rider.BS_Stop(BS_PlayedStance, 0.33f/SpeedModifier);
	}
}

defaultproperties
{
	bCheckForMirrorTransition=FALSE
	BSList_Jump(0)=(AnimName[BS_FullBody]="Mantle_Start")
	BSList_Fall(0)=(AnimName[BS_FullBody]="Mantle_Mid")
	BSList_Land(0)=(AnimName[BS_FullBody]="Mantle_End")

	DefaultMantleDistance=283.0f
	FallForwardVelocity=600.f
	GravityScale=6.f
}