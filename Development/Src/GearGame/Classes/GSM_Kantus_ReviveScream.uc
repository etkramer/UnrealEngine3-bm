
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Kantus_ReviveScream extends GearSpecialMove
	config(Pawn);


/** animation to play **/
var()	GearPawn.BodyStance	BS_Scream;

/** Revive Search radius */
var()	config	float	ReviveSearchRadius;

/** Revive Delay (after animation starts) **/
var()	config	float	ReviveDelay;

var array<Gearpawn> PawnsToRevive;

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
	if( PawnOwner.IsDBNO() || !GatherValidTargets())
	{
		//`log(self@GetFuncName()@"RETURNING FALSE!");
		return FALSE;
	}

	return TRUE;
}

simulated function bool GatherValidTargets()
{
	local GearPawn	FoundPawn;

	//`log(self@GetFuncName()@KnockdownRange@KnockdownFOV);
	PawnsToRevive.length = 0;
	foreach PawnOwner.CollidingActors( class'GearPawn', FoundPawn, ReviveSearchRadius, PawnOwner.Location, TRUE )
	{
		//`log(self@GetFuncName()@"Potential victim:"@FoundPawn);
		if( FoundPawn.bCanRecoverFromDBNO && FoundPawn.IsDoingSpecialMove(SM_DBNO) && (PawnOwner != FoundPawn) && PawnOwner.IsSameTeam(FoundPawn) && !FoundPawn.IsAHostage() )
		{
			//`log(self@GetFuncName()@"Found victim:"$FoundPawn);
			// add Pawn we found on the server.
			if( PawnOwner.Role == Role_Authority )
			{
				PawnsToRevive.AddItem(FoundPawn);
			}
			else
			{
				return true;
			}
		}
	}

	//`log(self@GetFuncName()@"found"@PawnsToRevive.length);
	return (PawnsToRevive.length > 0);

}

function ReviveCutShort()
{
	DoRevive();
	pawnOwner.ClearTimer('DoRevive',self);
	PawnOwner.BS_StopAll(0.3f);
	PawnOwner.EndSpecialMove();
}

function DoRevive()
{
	local int i;

	for(i=0;i<PawnsToRevive.length;i++)
	{
		if( AIOwner != None && PawnsToRevive[i] != none && !PawnsToRevive[i].IsPendingKill() && PawnsToRevive[i].IsDoingSpecialMove(SM_DBNO))
		{
			AIOwner.ReviveTeamMate(PawnsToRevive[i], TRUE);
		}
	}
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);


	if(PawnOwner.ROLE==Role_Authority)
	{
		pawnOwner.SetTimer( ReviveDelay,false,nameof(self.DoRevive), self );
	}

	// quit playing grenade anims
	PawnOwner.BS_StopAll(0.1f);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Scream, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE );

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Scream, TRUE);

	PawnOwner.DesiredKantusFadeVal = 1.0f;
	PawnOwner.KantusReviver = GearPawn_LocustKantusBase(Pawnowner);
	//class'GearPlayerCamera'.static.PlayWorldCameraShake(ExploShake, PawnOwner, PawnOwner.Location, ExploShakeInnerRadius, ExploShakeOuterRadius, ExploShakeFalloff, TRUE );

	
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.BS_SetAnimEndNotify(BS_Scream, FALSE);
	PawnOwner.DesiredKantusFadeVal = 0.f;
}


defaultproperties
{
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE

	BS_Scream=(AnimName[BS_FullBody]="kantus_chant")

	ExploShake=(TimeDuration=4.f,FOVAmplitude=0,LocAmplitude=(X=0,Y=0,Z=0),RotAmplitude=(X=1000,Y=400,Z=600),RotFrequency=(X=80,Y=40,Z=50))
	ExploShakeInnerRadius=800
	ExploShakeOuterRadius=2048
}

