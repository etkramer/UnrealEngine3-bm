
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_MeleeAttack extends GSM_Brumak_BasePlaySingleAnim
	native(SpecialMoves);

// C++ functions
cpptext
{
	virtual void TickSpecialMove(float DeltaTime);
}

/** Old location of hands for traces during melee attack */
var()	Vector RHand_OldLocation, LHand_OldLocation;
/** Damage of attack is active */
var()	bool	bDamageActive;

var(Debug) bool bDebugLines;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	RHand_OldLocation = vect(0,0,0);
	LHand_OldLocation = vect(0,0,0);

	Super.SpecialMoveStarted( bForced, PrevMove );
}

event MeleeDamageTo( Actor Other, Vector HitLocation )
{
	if( Brumak != None )
	{
		Brumak.NotifyBrumakSmashCollision( Other, HitLocation );
	}
}

defaultproperties
{
	bDamageActive=TRUE
	bDebugLines=FALSE

	BS_Animation=(AnimName[BS_FullBody]="Arm_Swing_Lft")
	BlendInTime=0.45f
	BlendOutTime=0.67f

	NearbyPlayerSynchedCameraAnimName="Left_Arm_Swing"
	NearbyPlayerSynchedCameraAnimRadius=(X=1280,Y=2560)

	bLockPawnRotation=TRUE
	bDisablePhysics=TRUE
	bDisableMovement=TRUE
	bCanFireWeapon=FALSE
}