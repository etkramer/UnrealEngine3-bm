/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_Kantus_SummoningScream extends GearSpecialMove
	config(Pawn);


/** animation to play **/
var()	GearPawn.BodyStance	BS_Scream;



protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.IsDBNO())
	{
		return FALSE;
	}

	return true;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearAI_Kantus KAI;
	Super.SpecialMoveStarted(bForced,PrevMove);

	//`log(self@GetFuncName()@"PawnsToKnockDown:"@PawnsToKnockDown.length);

	if(PawnOwner.ROLE == Role_Authority)
	{
		KAI = GearAI_Kantus(PawnOwner.MyGearAI);
		KAI.SummonMinions();
	}

	// quit playing grenade anims
	PawnOwner.BS_StopAll(0.1f);
	// Play body stance animation.
	PawnOwner.BS_Play(BS_Scream, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE );

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Scream, TRUE);
	
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.BS_SetAnimEndNotify(BS_Scream, FALSE);
}


defaultproperties
{
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	//BS_Scream=(AnimName[BS_FullBody]="AR_Idle_Ready_Pickup")
	BS_Scream=(AnimName[BS_FullBody]="kantus_yell")

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}

