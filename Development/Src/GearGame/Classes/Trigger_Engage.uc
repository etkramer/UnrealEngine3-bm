/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class Trigger_Engage extends Trigger
	native;

// The actor this trigger is associated with.
var() Actor ENGAGE_Actor;

// Counter for how many times it takes to finish this trigger.
var() int ENGAGE_TurnsToComplete;

/** minimum amount of time between turns */
var() float ENGAGE_LoopDelay;

/** Offset to move the player away from the ENGAGE_Actor */
var() vector ENGAGE_OffsetFromActor;

/** Speed at which the pawn will face the valve. */
var() float ENGAGE_TurnTowardSpeed;

/** Name of the animation to START the engaging */
var() Name ENGAGE_AnimName_Start;
/** Name of the animation to END the engaging */
var() Name ENGAGE_AnimName_End;
/** Name of the animation to LOOP when engaging - If empty will assume no looping */
var() Name ENGAGE_AnimName_Loop;
/** Name of the idle animation when engaging - If empty will assume no looping */
var() Name ENGAGE_AnimName_Idle;
/** Name of the forced off animation when engaging - If empty ENGAGE_AnimName_End will be used */
var() Name ENGAGE_AnimName_ForceOff;
/** Should Pawn holster his weapon first? */
var() bool	ENGAGE_bShouldHolsterWeaponFirst;
/** Should Pawn holster his weapon first? */
var() bool	ENGAGE_bPlayAnimationForHeavyWeapons;
/** Should player auto drop his weapon when triggering this interaction? */
var() bool	ENGAGE_bShouldAutoDropHeavyWeapon;

/** Whether the trigger is enabled or not - will also set the events used by this trigger */
var bool bEngageEnabled;

// Number of times the interaction occured
var transient int LoopCounter;

/** last time the engaging action happened */
var transient float LastTurnTime;

var() Trigger_Engage LinkedCoopTrigger;
var transient float LastAttemptTurnTime;

var transient GearPawn EngagedPawn;

simulated event PostBeginPlay()
{
	LoopCounter = ENGAGE_TurnsToComplete;

	// Little Hack for Gears 2 ship. Too risky to update maps, so we do this in code.
	if( ENGAGE_AnimName_Start == 'Floor_Lever_Pull_Start' ||
		ENGAGE_AnimName_Start == 'Wall_Lever_Pull_A_Start' ||
		ENGAGE_AnimName_Start == 'Wall_Lever_Pull_B_Start' )
	{
		ENGAGE_bShouldHolsterWeaponFirst = FALSE;
		ENGAGE_bShouldAutoDropHeavyWeapon = FALSE;
	}
}

// Override the untrigger so it's not triggered anymore.
function UnTrigger()
{
}

// Reset the trigger
simulated function OnManageEngage( SeqAct_ManageEngage inAction )
{
	local array<SequenceEvent> SeqEvents;
	local SeqEvt_Engage EngageEvent;
	local int EventIdx;

	// Reset and exit
	if( inAction.InputLinks[eMANAGEENGAGEIN_Reset].bHasImpulse )
	{
		LoopCounter = ENGAGE_TurnsToComplete;
		FindEventsOfClass( class'SeqEvt_Engage', SeqEvents, TRUE );
		for ( EventIdx = 0; EventIdx < SeqEvents.length; EventIdx++ )
		{
			EngageEvent = SeqEvt_Engage(SeqEvents[EventIdx]);
			EngageEvent.bEnabled = true;
		}

		return;
	}

	// Enable
	if( inAction.InputLinks[eMANAGEENGAGEIN_Enable].bHasImpulse )
	{
		bEngageEnabled = TRUE;
	}
	// Disable
	else if( inAction.InputLinks[eMANAGEENGAGEIN_Disable].bHasImpulse )
	{
		bEngageEnabled = FALSE;
	}
	// Toggle
	else if( inAction.InputLinks[eMANAGEENGAGEIN_Toggle].bHasImpulse )
	{
		bEngageEnabled = !bEngageEnabled;
	}

	FindEventsOfClass( class'SeqEvt_Engage', SeqEvents, TRUE );

	// Loop through all of the events and see if we need to enable or disable them
	for ( EventIdx = 0; EventIdx < SeqEvents.length; EventIdx++ )
	{
		EngageEvent = SeqEvt_Engage(SeqEvents[EventIdx]);
		EngageEvent.bEnabled = bEngageEnabled;
	}

	// See if we need to kick any players off the engage
	if ( !bEngageEnabled )
	{
		BootEngagedPlayers();
	}
}

/** Boot any players engaged off of the engage object */
final function BootEngagedPlayers()
{
	local GearPawn GearPawnTouching;
	local GearPC MyGearPC;
	local BodyStance IdleStance;

	IdleStance.AnimName[BS_FullBody] = ENGAGE_AnimName_Idle;

	foreach TouchingActors( class'GearPawn', GearPawnTouching )
	{
		if ( !GearPawnTouching.IsDoingSpecialMove( SM_Engage_ForceOff ) &&
			 ClassIsChildOf( GearPawnTouching.SpecialMoves[GearPawnTouching.SpecialMove].Class, class'GSM_Engage' ) )
		{
			MyGearPC = GearPC(GearPawnTouching.Controller);
			if ( MyGearPC != None )
			{
				GearPawnTouching.BS_Stop( IdleStance, 0.2f );
				GearPawnTouching.DoSpecialMove( SM_Engage_ForceOff, true );
				MyGearPC.ClientGotoState( 'PlayerWalking' );
				MyGearPC.GotoState( 'PlayerWalking' );
			}
		}
	}
}


defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=128.f
		CollisionHeight=128.f
	End Object

	bStatic=false

	SupportedEvents.Add(class'SeqEvt_Engage')
	SupportedEvents.Remove(class'SeqEvent_Used')

	ENGAGE_TurnsToComplete=5
	ENGAGE_LoopDelay=1.2
	ENGAGE_OffsetFromActor=(X=5.f,Y=100.f,Z=0.f)
	ENGAGE_TurnTowardSpeed=8.f
	bEngageEnabled=true
	ENGAGE_bShouldHolsterWeaponFirst=TRUE
	ENGAGE_bShouldAutoDropHeavyWeapon=TRUE

	ENGAGE_AnimName_Start="Valve_Turn_Start"
	ENGAGE_AnimName_End="Valve_Turn_End"
	ENGAGE_AnimName_Loop="Valve_Turn_Loop"
	ENGAGE_AnimName_Idle="Valve_Turn_Idle"
}
