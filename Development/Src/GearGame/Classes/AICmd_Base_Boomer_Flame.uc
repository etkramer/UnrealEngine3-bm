/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_Boomer_Flame extends AICommand_Base_Combat
	within GearAI_Boomer_Flame;

/** GoW global macros */


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

state InCombat
{
	function BeginState(Name PreviousStateName)
	{
		InvalidateCover();
	}

	/** 
	*	Should search for enemy if enemy hasn't been seen in too long a time OR
	*	We can see the place we thought they were, and they aren't there :)
	*/
	function bool ShouldSearchForEnemy()
	{
		local Vector EnemyLoc;
		local CoverInfo EnemyCover;
		local Actor HitActor;
		local Vector HitLocation, HitNormal;

		if( Enemy == None )
			return FALSE;

		//debug
		`AILog( GetFuncName()@Enemy, 'Combat' );

		if( FailedPathToEnemyRecently( Enemy ) )
		{
			//debug
			`AILog( "Already failed path to enemy recently" );

			return FALSE;
		}
		else
		if( GeneratePathTo( Enemy,, TRUE ) == None )
		{
			//debug
			`AILog( "No valid path to enemy" );

			SetFailedPathToEnemy( Enemy );
			return FALSE;
		}


		if( IsTooLongSinceEnemySeen( Enemy ) )
		{
			//debug
			`AILog( "Enemy out of view too long", 'Combat' );

			return TRUE;
		}

		EnemyCover = GetEnemyCover( Enemy );
		if( EnemyCover.Link != None )
		{
			EnemyLoc = EnemyCover.Link.GetSlotLocation( EnemyCover.SlotIdx );
		}
		else
		{
			EnemyLoc = GetEnemyLocation();
		}

		HitActor = Pawn.Trace( HitLocation, HitNormal, EnemyLoc, Pawn.GetWeaponStartTraceLocation(), TRUE );
		if( HitActor == None )
		{
			//debug
			`AILog( "Enemy clearly not there!", 'Combat' );

			return TRUE;
		}

		return FALSE;
	}

	function bool ShouldFireFromHere()
	{
		local Vector EnemyLoc;

		EnemyLoc = GetEnemyLocation();
		if( IsShortRange(EnemyLoc) || !AllowedToMove() )
		{
			if( FailedPathToEnemyRecently(Enemy) || 
				CanFireAt(Enemy, Pawn.GetWeaponStartTraceLocation()) )
			{
				return TRUE;
			}
		}

		return FALSE;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();
	// select the best enemy
	SelectEnemy();
	FireTarget = Enemy;

	if (!HasValidEnemy())
	{
		//debug
		`AILog("No enemy");

		Sleep(1.f);
		Goto('Begin');
	}

	// If have not fired from here AND
	// Enemy is in medium and can fire at enemy
	if( ShouldFireFromHere() )

	{
FireWeapon:
		while( IsReloading() )
		{
			//debug
			`AILog("Waiting on reload",'Loop');

			Sleep(0.25f);
		}

		// Fire from open
		FireFromOpen();

		// Randomly add some step asides
		if( FRand() > 0.25f || !StepAsideFor( Pawn ) )
		{
			Sleep( 0.25f );
		}
	}
	// Otherwise, move closer to them
	// (pick a location to move to)
	else
	{
		// Move closer to them
		SetEnemyMoveGoal(true);

		// If failed to move to enemy, just fire weapon
		if( bFailedToMoveToEnemy )
		{
			//debug 
			`AILog("Failed to move to enemy... just fire from open");

			// Fire from open
			if( CanFireAt(Enemy, Pawn.GetWeaponStartTraceLocation()) )
			{
				FireFromOpen();
			}			
			
			Sleep( 0.25f );
		}
	}

	// check combat transitions
	CheckCombatTransition();
//	CheckInterruptCombatTransitions();
	Goto('Begin');
}

defaultproperties
{
}
