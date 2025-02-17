/**
 * Base class for behavior of Brumak
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */ 
class AICmd_Base_Brumak extends AICommand_Base_Combat
	within GearAI_Brumak;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

	GotoState('InCombat');
}

state InCombat
{
	function Name PickCombatAction()
	{
		local Pawn	NearEnemy;
		local float NearestEnemyDist;

		//debug
		`AILog( GetFuncName()@HasAnyEnemies()@MoveAction );

		if( !HasAnyEnemies() || MoveAction != None )
		{
			return 'Delay';
		}

		// Find the nearest enemy
		NearestEnemyDist = GetNearEnemyDistance(Pawn.Location, NearEnemy);

		// Otherwise, if no enemies near by...
		if( !HasEnemyWithinDistance(EnemyDistance_Medium) )
		{
			// Adjust to a good starting spot
			return 'AdjustForAll';
		}
		else
		// Otherwise, enemies are in range
		// If not doing a special move...
		if( !Brumak.IsDoingASpecialMove() )
		{
			// If nearest enemy is melee target...
			if( (NearestEnemyDist <= EnemyDistance_Melee) ||
				(NearestEnemyDist <= EnemyDistance_Short  && FRand() < 0.25f) )
			{
				// Do melee attack
				SetEnemy( NearEnemy );
				return 'MeleeAttack';
			}
			else
			// Otherwise, if can't fire at enemy...
			if( !CanFireAtAnyTarget() )
			{
				// If someone is nearby, scream!
				if( FRand() < 0.33f )
				{
					SetEnemy( NearEnemy );
					if( NearestEnemyDist <= EnemyDistance_Short  ||
						(NearestEnemyDist <= EnemyDistance_Medium && FRand() < 0.50f) )
					{
						return 'Scream';
					}
				}

				// Move to a firing position...
				return 'DirectAttack';
			}
			else
			// Otherwise, if haven't moved in too long...
			if( TimeSince( LastMoveTime ) > 5.f )
			{
				return 'DirectAttack';
			}
		}

		// Sometimes Brumak can be stuck somewhere, so force him to move from time to time.
		if( FRand() < 0.33f )
		{
			return 'AdjustForAll';
		}

		return 'Delay';
	}

	function TransitionToMelee()
	{
		/*		local class<GearCombatAction> Action;

		if(  CanEngageMelee() &&
		!ClassIsChildOf( CombatAction, class'GCA_Melee' ) &&
		GetAction( class'GCA_Melee', Action, TRUE ) &&
		IsValidMeleeTarget(GearPawn(Enemy)) )
		{
		TransitionTo( Action, "Enemy in short range" );
		}
		*/
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if( !SelectEnemy() )
	{
		//debug
		`AILog("Unable to select an enemy");

		Sleep(1.f);
		Goto('End');
	}
	if( bHijackedByLD )
	{
		Sleep(0.5f);
		Goto('End');
	}

	Focus = Enemy;

	// Decide if what to do
	Goto( PickCombatAction() );
	Sleep( 1.f ); // should never get here
	Goto( 'End' );

AdjustForAll:
	//debug
	`AILog( "ADJUSTFORALL TAG", 'State' );
	//	MessagePlayer( "ADJUST FOR ALL TAG" );

	GotoBestFirePosition( BrumakSearchType_All );
	if( !bReachedMoveGoal )
	{
		`AILog( "Failed to good position vs all enemies" );
		Sleep( 1.f );
	}
	else
	{
		LastMoveTime = WorldInfo.TimeSeconds;
	}

	Goto( 'End' );

DirectAttack:
	//debug
	`AILog( "DIRECTATTACK TAG", 'State' );
	//	MessagePlayer( "DIRECT ATTACK TAG" );

	GotoBestFirePosition( BrumakSearchType_Direct );
	if( !bReachedMoveGoal )
	{
		`AILog( "Failed to direct attack enemy" );
		Sleep( 1.f );
	}
	else
	{
		LastMoveTime = WorldInfo.TimeSeconds;
	}

	Goto( 'End' );

MeleeAttack:
	//debug
	`AILog( "MELEEATTACK TAG", 'State' );
	//	MessagePlayer( "MELEE ATTACK TAG" );

	TransitionToMelee();
	`AILog( "Failed to transition to melee" );
	Sleep( 1.f );
	Goto( 'End' );

Delay:
	//debug
	`AILog( "DELAY TAG", 'State' );
	//	MessagePlayer( "DELAY TAG" );

	Sleep( 1.f );
	Goto( 'End' );

Scream:
	//debug
	`AILog( "SCREAM TAG", 'State' );
	//	MessagePlayer( "SCREAM TAG" );

	// Play Scream animation
	if( !Brumak.IsDoingASpecialMove() )
	{
		DoRoar();
	}

	// Move
	Goto( 'AdjustForAll' );

End:
	//debug
	`AILog( "END TAG", 'State' );

	CheckCombatTransition();
	Goto('Begin');
}

defaultproperties
{
}
