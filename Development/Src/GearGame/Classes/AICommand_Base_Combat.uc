/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICommand_Base_Combat extends AICommand
	within GearAI;

/** GoW global macros */

/** Amount of time before initial time check */
var vector2d	InitialTransitionCheckTime;
/** Amount of time between timed transition checks */
var vector2d	TransitionCheckTime;

function Pushed()
{
	Super.Pushed();

//	SetTimer( 0.25f, TRUE, nameof(CheckInterruptCombatTransitions) );
	SetTimer( GetInitialTransitionCheckTime(), FALSE, nameof(CheckTimedCombatTransition) );
}

function Popped()
{
	Super.Popped();

	ClearTimer( 'CheckInterruptCombatTransitions' );
	ClearTimer( 'CheckTimedCombatTransition'	  );
	ClearTimer( 'SelectTarget' );
}

final function float GetInitialTransitionCheckTime()
{
	return RandRange( default.InitialTransitionCheckTime.X, default.InitialTransitionCheckTime.Y );
}
final function float GetTransitionCheckTime()
{
	return RandRange( default.TransitionCheckTime.X, default.TransitionCheckTime.Y );
}

function bool TimedTransitionCheck( out class<AICommand_Base_Combat> out_NewCommand, out String out_Reason );
function bool CheckTransition( out class<AICommand_Base_Combat> out_NewCommand, out String out_Reason );

state InCombat `DEBUGSTATE
{
	function BeginState( Name PreviousStateName )
	{
		super.BeginState( PreviousStateName );

		// Set target selection timer
		SetTimer( 0.5f + FRand()*0.5f, TRUE, nameof(SelectTarget) );
	}

	function EndState( Name NextStateName )
	{
		super.EndState( NextStateName );

		// Clear target selection timer
		ClearTimer( 'SelectTarget' );
	}
}


defaultproperties
{
	InitialTransitionCheckTime=(X=1.f,Y=1.f)
	TransitionCheckTime=(X=1.f,Y=1.f)
}
