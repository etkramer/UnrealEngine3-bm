
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_InitCombatReaction extends GearSpecialMove;

/** Animation */
var const	array<GearPawn.BodyStance>	BS_List;
var			int							BSIdx;


protected function bool InternalCanDoSpecialMove()
{
	// Can't stumble if doing another special move
	if( PawnOwner.SpecialMove != SM_None && PawnOwner.SpecialMove != SM_ChainsawHold && !PawnOwner.IsDoingMove2IdleTransition() )
	{
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	PawnOwner.StopFiring();

	BSIdx = Rand(BS_List.Length);
	`AILog_Ext(PawnOwner@"Chose Idx"@BSIdx@":"@BS_List[BSIdx].AnimName[BS_Std_Up],,PawnOwner.MyGearAI);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_List[BSIdx], SpeedModifier, 0.2f/SpeedModifier, 0.3f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_List[BSIdx], TRUE);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.BS_SetAnimEndNotify(BS_List[BSIdx], FALSE);
}


defaultproperties
{
	BS_List(0)=(AnimName[BS_Std_Up]="Locust_Sniff")
	BS_List(1)=(AnimName[BS_Std_Up]="Locust_Alert")
	BS_List(2)=(AnimName[BS_Std_Up]="Locust_Point")
	BS_List(3)=(AnimName[BS_Std_Up]="Locust_Yell")

	bShouldAbortWeaponReload=FALSE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE
	BSIdx=-1
}
