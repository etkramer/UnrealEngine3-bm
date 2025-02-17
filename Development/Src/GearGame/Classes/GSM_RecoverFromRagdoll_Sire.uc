
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_RecoverFromRagdoll_Sire extends GSM_RecoverFromRagdoll;

var()	GearPawn.BodyStance	BS_GetUpLeft, BS_GetUpRight;
var()	GearPawn.BodyStance	BS_GetUpLeftMir, BS_GetUpRightMir, BS_GetUpFrontMir, BS_GetUpBackMir;


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	if(PawnOwner.bIsGore || PawnOwner.Health <= 0)
	{
		`log(self@"- trying to recover from ragdoll but dead/gored!");
		PawnOwner.EndSpecialMove();
		return;
	}

	Super.SpecialMoveStarted(bForced,PrevMove);
}

simulated function protected GearPawn.BodyStance GetGetUpAnim()
{
	local vector DirToEnemy;
	local float Angle;

	//PawnOwner.MessagePlayer(GetFuncName()@PawnOwner@PawnOwner.Controller != none @ PawnOwner.Controller.Enemy != none);
	if(PawnOwner.Controller != none && PawnOwner.Controller.Enemy != none)
	{
		// decide which anim we want to play, based on our relative rotation to target

		// direction to enemy
		DirToEnemy = Normal(PawnOwner.Controller.Enemy.Location - PawnOwner.Location);
		// Convert direction vector from world to local space
		DirToEnemy = DirToEnemy << PawnOwner.Rotation;
		DirToEnemy.Z = 0.0;
		DirToEnemy = Normal(DirToEnemy);

		// Then work out yaw bearing in local space
		Angle = GetHeadingAngle(DirToEnemy);

		//back
		if(Angle < -0.75f * PI || Angle > 0.75f * PI)
		{			
			return bGetUpFromBack ? BS_GetUpBackMir : BS_GetUpBack;
		}
		// left
		else if(Angle < -0.25f * PI)
		{
			return bGetUpFromBack ? BS_GetUpLeftMir : BS_GetUpLeft;
		}
		// right
		else if(Angle > 0.25f * PI)
		{
			return bGetUpFromBack ? BS_GetUpRightMir : BS_GetUpRight;
		}
		// front
		else
		{
			return bGetUpFromBack ? BS_GetUpFrontMir : BS_GetUpFront;
		}

	}

	return Super.GetGetUpAnim();
}

/** 
*	Done with blend, start the get-up animation, start updating physics bones again, and show result.
*	Bones should all be fixed at this point.
*/
simulated event FinishedBlendToGetUp()
{
	Super.FinishedBlendToGetUp();
	if(pawnOwner.MyGearAI.Enemy == none)
	{
		PawnOwner.Velocity = vect(0,0,0);
		PawnOwner.Acceleration= vect(0,0,0);
	}

}

defaultproperties
{
	BS_GetUpFront=(AnimName[BS_FullBody]="getup_run_fwd")
	BS_GetUpBack=(AnimName[BS_FullBody]="getup_run_bwd")
	BS_GetUpLeft=(AnimName[BS_FullBody]="getup_run_left")
	BS_GetUpRight=(AnimName[BS_FullBody]="getup_run_right")
	BS_GetUpFrontMir=(AnimName[BS_FullBody]="Sire_getup_run_fwd_mir")
	BS_GetUpBackMir=(AnimName[BS_FullBody]="Sire_getup_run_bwd_mir")
	BS_GetUpLeftMir=(AnimName[BS_FullBody]="Sire_getup_run_left_mir")
	BS_GetUpRightMir=(AnimName[BS_FullBody]="Sire_getup_run_right_mir")

	UpDownBoneName="body"
	UpDownAxis=AXIS_Z
	bInvertUpDownBoneAxis=FALSE

	OrientationBoneName="body"
	OrientationAxis=AXIS_Y

	GetUpFromBackYawOffset=0
	bIgnorePawnsOnRecover=TRUE

	GetUpBlendTime=0.4
	GetUpAnimRate=1.0
	GetUpAnimStartPos=1.0

	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
	bCanFireWeapon=FALSE
	bShouldAbortWeaponReload=TRUE

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}