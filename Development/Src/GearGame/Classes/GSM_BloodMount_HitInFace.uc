
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_BloodMount_HitInFace extends GSM_BasePlaySingleAnim;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearPawn	Rider;

	Super.SpecialMoveStarted(bForced, PrevMove);

	// Play sync'd animation on driver
	Rider = GearPawn_LocustBloodmount(PawnOwner).Driver;
	if( Rider != None )
	{
		Rider.BS_Play(BS_Animation, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier, FALSE);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearPawn	Rider;

	// Have rider end his special move in sync with us.
	Rider = GearPawn_LocustBloodmount(PawnOwner).Driver;
	if( Rider != None && Rider.IsDoingSpecialMove(SM_BloodMountDriver_CalmMount) )
	{
		Rider.ServerEndSpecialMove();
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	BlendOutTime=0.25f
	BS_Animation=(AnimName[BS_FullBody]="Shot_Reaction")
	bLockPawnRotation=TRUE
	bDisablePhysics=TRUE
	bCameraFocusOnPawn=FALSE
	bDisableLook=FALSE
	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	bDisableAI=TRUE
}
