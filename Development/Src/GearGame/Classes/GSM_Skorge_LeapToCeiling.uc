
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Skorge_LeapToCeiling extends GSM_BaseVariableFall;

var GearPawn_LocustSkorgeBase Skorge;

var	const GearPawn.BodyStance	BS_Jump_Staff, BS_Fall_Staff, BS_Land_Staff;
var	const GearPawn.BodyStance	BS_Jump_TwoStick, BS_Fall_TwoStick, BS_Land_TwoStick;
var	const GearPawn.BodyStance	BS_Jump_OneStick, BS_Fall_OneStick, BS_Land_OneStick;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Skorge = GearPawn_LocustSkorgeBase(PawnOwner);

	Super.SpecialMoveStarted(bForced,PrevMove);

	PawnOwner.Mesh.RootMotionMode = RMM_Ignore;
}


simulated function GearPawn.BodyStance GetJumpBS()
{
	if( Skorge.bHideGun )
	{
		if( Skorge.Stage == SKORGE_Staff )
		{
			return BS_Jump_Staff;
		}
		else
		if( Skorge.Stage == SKORGE_TwoStick )
		{
			return BS_Jump_TwoStick;
		}
		else
		{
			return BS_Jump_OneStick;
		}
	}
	return BS_Jump;
}
simulated function GearPawn.BodyStance GetFallBS()
{
	if( Skorge.bHideGun )
	{
		if( Skorge.Stage == SKORGE_Staff )
		{
			return BS_Fall_Staff;
		}
		else
		if( Skorge.Stage == SKORGE_TwoStick )
		{
			return BS_Fall_TwoStick;
		}
		else
		{
			return BS_Fall_OneStick;
		}
	}
	return BS_Fall;
}

simulated function bool IsFallLooping()
{
	return TRUE;
}

simulated function GearPawn.BodyStance GetLandBS()
{
	if( Skorge.bHideGun )
	{
	
		if( Skorge.Stage == SKORGE_Staff )
		{
			return BS_Land_Staff;
		}
		else
		if( Skorge.Stage == SKORGE_TwoStick )
		{
			return BS_Land_TwoStick;
		}
		else
		{
			return BS_Land_OneStick;
		}
	}

	return BS_Land;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// reset animation mirroring
	PawnOwner.SetMirroredSide( FALSE );
	PawnOwner.LastEvadeTime = PawnOwner.WorldInfo.TimeSeconds;
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode; 
}

simulated function PlayFall()
{
	local GearAI_Skorge AI;

	super.PlayFall();

	AI = GearAI_Skorge(PawnOwner.Controller);
	if( AI != None )
	{
		AI.NotifyCeilingTransitionFall();
	}

	Skorge.PlayTakeOffEffects();
}


defaultproperties
{
	BS_Jump=(AnimName[BS_FullBody]="Pistol_Jump_High_Start")
	BS_Fall=(AnimName[BS_FullBody]="Pistol_Jump_High_Up")
	BS_Land=(AnimName[BS_FullBody]="Pistol_Jump_High_End_Loop")

	BS_Jump_Staff=(AnimName[BS_FullBody]="Staff_Jump_High_Start")
	BS_Fall_Staff=(AnimName[BS_FullBody]="Staff_Jump_High_Up")
	BS_Land_Staff=(AnimName[BS_FullBody]="Staff_Jump_High_End_Loop")

	BS_Jump_TwoStick=(AnimName[BS_FullBody]="2stick_Jump_High_Start")
	BS_Fall_TwoStick=(AnimName[BS_FullBody]="2stick_Jump_High_Up")
	BS_Land_TwoStick=(AnimName[BS_FullBody]="2stick_Jump_High_End_Loop")

	BS_Jump_OneStick=(AnimName[BS_FullBody]="1stick_Jump_High_Start")
	BS_Fall_OneStick=(AnimName[BS_FullBody]="1stick_Jump_High_Up")
	BS_Land_OneStick=(AnimName[BS_FullBody]="1stick_Jump_High_End_Loop")

	PreImpactTime=0.35f

	JumpingPhysics=PHYS_None
}
