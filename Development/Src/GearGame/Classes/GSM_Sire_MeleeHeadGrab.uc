
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GSM_Sire_MeleeHeadGrab extends GearSpecialMove;

/** animation */
var()	GearPawn.BodyStance	BS_AnimationRight;
var()	GearPawn.BodyStance	BS_AnimationLeft;
var()	GearPawn.BodyStance	BS_AnimationRunning;
var()	GearPawn.BodyStance	BS_Animation;
var()	Gearpawn.BodyStance BS_Armless;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearPawn_LocustSireBase Sire;

	Super.SpecialMoveStarted(bForced,PrevMove);
	Sire = GearPawn_LocustSireBase(Pawnowner);

	//PawnOwner.MessagePlayer(GearPawn_LocustSireBase(PawnOwner).StartAttackSpeed);
	if(GearPawn_LocustSireBase(PawnOwner).StartAttackSpeed > 0.f)
	{
		BS_Animation = BS_AnimationRunning;
	}
	else
	{
		if(Sire == none || (!Sire.bLostRightArm && !Sire.bLostLeftArm))
		{
			if(frand() < 0.5f)
			{
				BS_Animation = BS_AnimationRight;
			}
			else
			{
				BS_Animation = BS_AnimationLeft;
			}
		}
		else if(Sire.bLostRightArm && !Sire.bLostLeftArm)
		{
			BS_Animation = BS_AnimationLeft;
		}
		else if(Sire.bLostLeftArm && !Sire.bLostRightArm)
		{
			BS_Animation = BS_AnimationRight;
		}
		else
		{
			BS_Animation = BS_Armless;
		}
	}

	// consume last attack speed
	GearPawn_LocustSireBase(PawnOwner).StartAttackSpeed = 0;

	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);
}

defaultproperties
{
	BS_AnimationRight=(AnimName[BS_FullBody]="Melee_Slash_A")
	BS_AnimationLeft=(AnimName[BS_FullBody]="slash_melee_left")
	BS_AnimationRunning=(AnimName[BS_FullBody]="run2idle")
	BS_Armless=(AnimName[BS_FullBody]="armless_attack")

	bLockPawnRotation=TRUE
	bDisableCollision=FALSE
	bDisableMovement=TRUE
}
