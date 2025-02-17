
/**
 * @fixme laurent - OBSOLETE. REMOVE ME.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_ExecutionCurbStomp extends GearSpecialMove;

/** Weapon pickup body stance animation */
var()	GearPawn.BodyStance	BS_ExecutionMove;

/** This is the pawn that we are going to be doing the the CurbStomp on **/
var	transient GearPawn		TargetPawn;

/** Screen Shake play on impact */
var()	ScreenShakeStruct	ImpactScreenShake;

protected function bool InternalCanDoSpecialMove()
{
	TargetPawn = GetTargetPawn();

	return (TargetPawn != None && TargetPawn.CanBeSpecialMeleeAttacked(PawnOwner));
}

simulated function GearPawn GetTargetPawn()
{
	local GearPawn	FoundPawn;

	ForEach PawnOwner.VisibleCollidingActors(class'GearPawn', FoundPawn, 64.f, PawnOwner.Location, TRUE)
	{
		if( FoundPawn != PawnOwner && !PawnOwner.IsSameTeam(FoundPawn) && FoundPawn.CanBeCurbStomped() )
		{
			if( (Normal(FoundPawn.Location - PawnOwner.Location) dot vector(PawnOwner.Rotation)) >= 0.50 &&
				Abs(FoundPawn.Location.Z - PawnOwner.Location.Z) < PawnOwner.CylinderComponent.CollisionHeight * 0.8f ) // facing the dude
			{
				return FoundPawn;
			}
		}
	}

	return None;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearAI AI;

	Super.SpecialMoveStarted(bForced,PrevMove);

	// make sure we have the target pawn
	if( TargetPawn == None )
	{
		TargetPawn = GetTargetPawn();
	}

	if( TargetPawn != None )
	{
		// Face TargetPawn
		SetFacePreciseRotation(Rotator(TargetPawn.Location - PawnOwner.Location), 0.33f);

		// camera
		if( TargetPawn.IsLocallyControlled() )
		{
			TargetPawn.bOverrideDeathCamLookat = TRUE;
			TargetPawn.DeathCamLookatBoneName = TargetPawn.PelvisBoneName;
			TargetPawn.DeathCamLookatPawn = PawnOwner;
		}

		// Notify AI they are a victim so they don't get up
		AI = GearAI(TargetPawn.Controller);
		if( AI != None )
		{
			AI.DictateSpecialMove( SM_Execution_CurbStomp, PawnOwner );
		}
	}

	// Play body stance animation.
	PawnOwner.BS_Play(BS_ExecutionMove, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_ExecutionMove, TRUE);

	// Play Face FX Emotion
	if( !PawnOwner.IsActorPlayingFaceFXAnim() )
	{
		PawnOwner.PlayActorFaceFXAnim(None, "Emotions", "Strained", None);
	}

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_MeleeAttackLargeEffort, true);

	// Setup the timer to execute
	PawnOwner.SetTimer( 1.1f/SpeedModifier, FALSE, nameof(self.Execution), self );
}

/**
 * Called on a timer when doing a curb stomp.
 */
simulated function Execution()
{
	local GearPC PC;

	if( PawnOwner.WorldInfo.Role == Role_Authority &&
		TargetPawn != None && !TargetPawn.bPlayedDeath && !TargetPawn.bDeleteMe )
	{
		// Have the momentum be in the direction the killer is looking
		TargetPawn.TearOffMomentum = Vector(PawnOwner.Rotation);
		TargetPawn.Died(PawnOwner.Controller, class'GDT_CurbStomp', TargetPawn.Location);
	}

	PC = GearPC(PawnOwner.Controller);
	if( PC != None )
	{
		// Play Force Feed Back effect
		PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);

		// Play Screen Shake effect
		PC.ClientPlayCameraShake(ImpactScreenShake);
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable end of animation notification
	PawnOwner.BS_SetAnimEndNotify(BS_ExecutionMove, FALSE);

	// Clear execution timer.
	PawnOwner.ClearTimer('ServerExecute');

	// we always have to reset the flag because the pawn has usually lost its controller on the client by this point
	// so it will always return false for IsLocallyControlled()
	TargetPawn.bOverrideDeathCamLookat = FALSE;

	// Clear TargetPawn for next time.
	TargetPawn = None;
}

/*

*/
defaultproperties
{
	bShouldAbortWeaponReload=FALSE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE

	BS_ExecutionMove=(AnimName[BS_FullBody]="AR_Melee_Kick")

// 	SpecialMoveCameraBoneAnims(0)=(AnimName="Camera_CurbStomp_A",CollisionTestVector=(X=53.f,Y=-226.f,Z=-25.f))
// 	SpecialMoveCameraBoneAnims(1)=(AnimName="Camera_CurbStomp_B",CollisionTestVector=(X=-121.f,Y=-163.f,Z=116.f))
// 	SpecialMoveCameraBoneAnims(2)=(AnimName="Camera_CurbStomp_C",CollisionTestVector=(X=-38.f,Y=235.f,Z=4.f))
// 	SpecialMoveCameraBoneAnims(3)=(AnimName="Camera_CurbStomp_D",CollisionTestVector=(X=-21.f,Y=174.f,Z=146.f))
// 	SpecialMoveCameraBoneAnims(4)=(AnimName="Camera_CurbStomp_E",CollisionTestVector=(X=50.f,Y=-137.f,Z=195.f))

	Action={(
			ActionName=CurbStomp,
			ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=284,V=346,UL=104,VL=91)))	),
	)}

	ImpactScreenShake={(TimeDuration=0.25f,
						RotAmplitude=(X=1500,Y=500,Z=250),
						RotFrequency=(X=10,Y=10,Z=10),
						RotParam=(X=ESP_OffsetRandom,Y=ESP_OffsetRandom,Z=ESP_OffsetRandom),
						LocAmplitude=(X=-10,Y=2,Z=2),
						LocFrequency=(X=10,Y=10,Z=10),
						LocParam=(X=ESP_OffsetZero,Y=ESP_OffsetRandom,Z=ESP_OffsetRandom)
						)}
}


