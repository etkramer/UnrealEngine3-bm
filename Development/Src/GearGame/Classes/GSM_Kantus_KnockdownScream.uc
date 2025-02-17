/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_Kantus_KnockdownScream extends GearSpecialMove
	config(Pawn);


/** animation to play **/
var()	GearPawn.BodyStance	BS_Scream;


/** Camera shake parameters */
var() ScreenShakeStruct	ExploShake;
/** Radius to within which to play full-powered screenshake (will be scaled within radius) */
var() float				ExploShakeInnerRadius;
/** Between inner and outer radii, scale shake from full to zero */
var() float				ExploShakeOuterRadius;
/** Exponent for intensity falloff between inner and outer radii. */
var() float				ExploShakeFalloff;



protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.IsDBNO())
	{
		return FALSE;
	}

	return true;
}


function DoKnockDownCamShake()
{
	class'GearPlayerCamera'.static.PlayWorldCameraShake(ExploShake, PawnOwner, PawnOwner.Location, ExploShakeInnerRadius, ExploShakeOuterRadius, ExploShakeFalloff, TRUE);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearAI_Kantus KAI;
	Super.SpecialMoveStarted(bForced,PrevMove);

	//`log(self@GetFuncName()@"PawnsToKnockDown:"@PawnsToKnockDown.length);

	if(PawnOwner.ROLE == Role_Authority)
	{
		KAI = GearAI_Kantus(PawnOwner.MyGearAI);
		PawnOwner.SetTimer( KAI.KnockdownDelay*0.8f,false,nameof(self.DoKnockDownCamShake), self );
		PawnOwner.SetTimer( KAI.KnockdownDelay,false,nameof(GearPawn_LocustKantusBase(PawnOwner).DoKnockDown) );
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

	ExploShake=(TimeDuration=1.f,FOVAmplitude=0,LocAmplitude=(X=0,Y=0,Z=0),RotAmplitude=(X=1000,Y=400,Z=600),RotFrequency=(X=80,Y=40,Z=50))
	ExploShakeInnerRadius=800
	ExploShakeOuterRadius=2048
}

