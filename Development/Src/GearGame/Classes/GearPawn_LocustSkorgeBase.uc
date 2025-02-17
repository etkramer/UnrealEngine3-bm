
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustSkorgeBase extends GearPawn_LocustKantusBase
	native(Pawn)
	config(Pawn);

/** Whether gun is hidden or not */
var() repnotify	bool			bHideGun;
/** Whether Skorge is swinging his staff to dodge bullets */
var()			bool			bBlockingBullets;
/** Time that skore should complete pistol strafe attack */
var				float			CompleteStrafeAttackTime;
/** Used for testing only, force outcome of duel */
var				Byte			ForcedOutcome;

/** the number of button presses the Skorge simulates for the duel at each difficulty */
var config int DuelingRequirements[4];

/** List of weapon stages for Skorge */
enum ESkorgeStage
{
	SKORGE_Staff,
	SKORGE_TwoStick,
	SKORGE_OneStick,
};
/** Current stage of weapon combat */
var() repnotify ESkorgeStage	Stage;
/** Node used to play charge animation */
var() AnimNodeSequence	ChargeAnimNode;

var AudioComponent AC_StaffTwirl;

replication
{
	// Replicated to ALL
	if( Role == Role_Authority )
		Stage, bHideGun, bBlockingBullets;
}

simulated event ReplicatedEvent( name VarName )
{
	switch( VarName )
	{
		case 'Stage':
			UpdateStage();
			break;
		case 'bHideGun':
			HideGun( bHideGun );
			break;
	}

	Super.ReplicatedEvent( VarName );
}

simulated function CacheAnimNodes()
{
	Super.CacheAnimNodes();

	ChargeAnimNode = AnimNodeSequence(Mesh.FindAnimNode('ChargeAnim'));
}

function RegisterStageChange()
{
	if( Stage == SKORGE_Staff )
	{
		SetStage( SKORGE_TwoStick );
	}
	else
	if( Stage == SKORGE_TwoStick )
	{
		SetStage( SKORGE_OneStick );
	}
}

function SetStage( ESkorgeStage inStage )
{
	Stage = inStage;
	UpdateStage();
}

simulated function UpdateStage();
simulated function HideGun( bool bHide );

function ThrowingInkGrenade( float Delay )
{
	local GearAI_Skorge AI;
	ThrowCount++;

	AI = GearAI_Skorge(Controller);
	if( AI != None )
	{
		// let the controller do whatever it needs to when we throw this thing
		AI.ThrowingInkGrenade( Delay );
	}
}

simulated function DuelingMiniGameStartNotification()
{
	bInDuelingMiniGame = TRUE;

	DuelingMiniGameButtonPresses = DuelingRequirements[GearAI(Controller).GetDifficultyLevel()];
}

simulated function PlayTakeOffEffects();
simulated function PlayLandingEffects();
simulated function PlayBlockedBulletEffect( Pawn Shooter );

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

/** Be safe, force chainsaw sounds to stop after duel is finished */
simulated function ForceStopChainsaw();

defaultproperties
{
	bInvalidMeleeTarget=TRUE
}
