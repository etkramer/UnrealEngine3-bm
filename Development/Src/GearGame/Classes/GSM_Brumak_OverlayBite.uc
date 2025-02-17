
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_OverlayBite extends GSM_Brumak_BasePlaySingleAnim
	native(SpecialMoves);

// C++ functions
cpptext
{
	virtual void TickSpecialMove(float DeltaTime);
}

/** Old location of hands for traces during melee attack */
var()		Vector					Mouth_OldLocation;
/** Name of jaw socket */
var()		Name					JawSocketName;
/** Damage of attack is active */
var()		bool					bDamageActive;
/** Display debug damage traces */
var(Debug)	bool					bDebugLines;


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Mouth_OldLocation = vect(0,0,0);

	Super.SpecialMoveStarted( bForced, PrevMove );
}

event MeleeDamageTo( Actor Other, Vector HitLocation )
{
	if( Brumak != None )
	{
		Brumak.NotifyBrumakBiteCollision( Other, HitLocation );
	}
}

defaultproperties
{
	JawSocketName="JawSocket"
	bDamageActive=TRUE
	bDebugLines=FALSE

	BS_Animation=(AnimName[BS_Additive]="Bite")
	BlendInTime=0.33f
	BlendOutTime=0.45f

	NearbyPlayerSynchedCameraAnimName="Bite_Brumak"
	NearbyPlayerSynchedCameraAnimRadius=(X=1280,Y=2560)

	bCheckForGlobalInterrupts=FALSE
}