/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_ShieldBoomer extends AICommand_Base_Combat
	within GearAI_Boomer_Shield;

/** GoW global macros */

/**
 *	GENERAL BEHAVIORS
 */
// STATE - INCOMBAT
// Select Enemy
// If close enough to melee enemy
	// Swing!
// ElseIf close enough to charge an enemy
	// Lumber towards him!
// ElseIf hasn't been shoot in X secs (ie 3 secs)
	// Move toward enemy
// Else
	// Turtle
// Goto TOP

// EVENT - TAKE DAMAGE
// If in melee or charge range
	// Ignore
// Else
	// Stop moving
	// If haven't turned to face new attacker within X secs (ie 0.5f) -- prevent thrashing
		// Turn to face attacker
		// Reset turtle timer
/**
 *	END BEHAVIORS
 */

/** Used to detect transition to roadie run. */
var bool bLastShouldRoadieRun;

function Pushed()
{
	Super.Pushed();

	GotoState( 'InCombat' );
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );
	bShouldRoadieRun = FALSE;
}

function Popped()
{
	Super.Popped();

	bShouldRoadieRun = FALSE;
}

final function float GetChargeAbortDistance()
{
	if( bShouldRoadieRun )
	{
		return EnemyDistance_Short * 1.1f;
	}

	return 0.f;
}

state InCombat
{
Begin:
	// Select Enemy
	if( !SelectEnemy() )
	{
		//debug
		`AILog( "Failed to select an enemy -- delay failure" );

		GotoState( 'DelayFailure' );
	}

	// make sure a stale large DestinationOffset doesn't make us think we reached our enemy early
	Pawn.DestinationOffset = Min(Pawn.DestinationOffset, MyGearPawn.GetAIMeleeAttackRange());

	//debug
	`AILog( "BEGIN TAG"@IsMeleeRange(Enemy.Location)@Pawn.ReachedDestination(Enemy)@CanEngageMelee()@VSize(Enemy.Location-Pawn.Location) );

	// If close enough to melee enemy
	if( (IsMeleeRange(Enemy.Location) && FastTrace(Enemy.Location, Pawn.Location)) || Pawn.ReachedDestination(Enemy) )
	{
		CloseShield();

		// SWING!
		if( CanEngageMelee() )
		{
			DoMeleeAttack();
		}
		else
		{
			Sleep(0.1f);
		}
	}
	else
	{
		bShouldRoadieRun = ShouldCharge();
		if( bShouldRoadieRun || WorldInfo.TimeSeconds >= AllowedMoveTime )
		{
			// Move toward enemy normally
			CloseShield();
		}
		if (bShouldRoadieRun && !bLastShouldRoadieRun)
		{
			MyGearPawn.TelegraphCharge();
		}
		bLastShouldRoadieRun = bShouldRoadieRun;

		SetEnemyMoveGoal(FALSE,EnemyDistance_Melee,GetChargeAbortDistance());
	}

	// Goto TOP!
	Goto( 'Begin' );
}


defaultproperties
{
	
}
