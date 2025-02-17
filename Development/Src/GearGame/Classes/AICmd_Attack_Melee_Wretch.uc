/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Melee_Wretch extends AICommand
	within GearAI;

/** GoW global macros */

// CIRCLE VARIABLES
/** Circling left (-1) or right (+1) */
var int CircleDir;
/** Min/Max distance of the arc for circling the enemy */
var Vector2D CircleDistance;
/** Point we are moving to for this circle step */
var transient Vector CirclePoint;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Melee( GearAI AI )
{
	local AICmd_Attack_Melee_Wretch Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_Attack_Melee_Wretch';
        if( Cmd != None )
        {
            AI.PushCommand( Cmd );
            return TRUE;
        }
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();

	InvalidateCover();
	
	bDoingMeleeAttack = TRUE;

	// Do melee attack immediately
	GotoState( 'ChooseAction' );
}

function Popped()
{
	Super.Popped();

	bDoingMeleeAttack = FALSE;
}

state ChooseAction
{
	function Name PickAttackType()
	{
		local float		Dist, Radius;
		//local GearPawn	GearEnemyPawn;

		//GearEnemyPawn = GearPawn(Enemy);

		if( IsPawnVisibleViaTrace(Enemy) )
		{
			// If this is a dark wretch, always explode when in melee range
			if( GearPawn_LocustWretchBase(Pawn).bSuicidal )
			{
				return 'Explode';
			}

			Dist   = VSize(Pawn.Location - Enemy.Location);
			Radius = (Pawn.GetCollisionRadius() + Enemy.GetCollisionRadius()) * 1.3f;

			//debug
			`AILog( GetFuncName()@Dist@Radius );

			/* oops!
			// try a grab attack
			if( Dist <= EnemyDistance_Melee && FRand() < 0.25f )
			{
				// Check if can do grab attack
				if( GearEnemyPawn != None && MyGearPawn.CanDoSpecialMovePawnInteraction(SM_Wretch_GrabAttack, GearEnemyPawn) )
				{
					MyGearPawn.ServerDoSpecialMove(SM_Wretch_GrabAttack, TRUE, GearEnemyPawn);
					return 'Finished';
				}
			}
			*/

			if( Dist <= Radius )
			{
				// Do ground swipe
				return 'Swipe';
			}

			if( Dist <= EnemyDistance_Melee )
			{
				// Try a leaping swipe
				return 'Pounce';
			}
		}

		// Close distance on enemy
		return 'RunToEnemy';
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@GetStateName(), 'State' );

	// Chose which action to take
	Goto( PickAttackType() );

// Wretch not close enough to enemy for any attack
// Close the gap
RunToEnemy:
	//debug
	`AILog( "RUNTOENEMY TAG"@GetStateName(), 'State' );

	SetEnemyMoveGoal(true);
	Goto( 'Finished' );

// Dark wretch should explode
Explode:
	//debug
	`AILog( "EXPLODE TAG"@GetStateName(), 'State' );

	Pawn.Died( AIOwner, class'DamageType', Pawn.Location );
	Goto( 'Finished' );

// Ground swipe attack
Swipe:
	//debug
	`AILog( "SWIPE TAG"@GetStateName(), 'State' );

	class'AICmd_Attack_Wretch_Swipe'.static.InitCommand( AIOwner );
	if( FRand() < 0.5f ) { Goto( 'Circle' ); }
	else				 { Goto( 'Finished' ); }

// Leaping pounce attack
Pounce:
	//debug
	`AILog( "POUNCE TAG"@GetStateName(), 'State' );

	class'AICmd_Attack_Wretch_Pounce'.static.InitCommand( AIOwner );
	if( FRand() < 0.5f ) { Goto( 'Circle' ); }
	else				 { Goto( 'Finished' ); }

// Circle around the enemy
Circle:
	//debug
	`AILog( "CIRCLE TAG"@GetStateName(), 'State' );

	CircleDir = (FRand() < 0.5f) ? -1 : 1;
	PushState( 'Circle' );
	if( FRand() < 0.25f ) { Goto( 'Circle' ); }
	else				  { Goto( 'Finished' ); }

Finished:
	//debug
	`AILog( "FINISHED TAG"@GetStateName(), 'State' );

	// Prevent infinite recursion and pop stack
	Status = 'Success';
	PopCommand( self );
}

function CircleTimer(); 

/** Abstract state */
state Circle `DEBUGSTATE
{
	function PushedState()
	{
		super.PushedState();

		bWantsLedgeCheck = TRUE;
	}

	function PoppedState()
	{
		super.PoppedState();

		bWantsLedgeCheck = FALSE;
		if( Pawn != None )
		{
			Pawn.ZeroMovementVariables();
		}
	}
	
	function bool GetCirclePoint()
	{
		local bool bResult;
		local Vector EnemyToPawn;
		local float Dist;

		EnemyToPawn = Pawn.Location - Enemy.Location;
		Dist = VSize(EnemyToPawn);
		if( Dist < CircleDistance.X )
		{
			EnemyToPawn *= (CircleDistance.X / Dist);
		}
		if( Dist > CircleDistance.Y )
		{
			EnemyToPawn *= (CircleDistance.Y / Dist);
		}

		CirclePoint = Enemy.Location + (EnemyToPawn >> (rot(0,8192,0) * CircleDir));

		bResult = PointReachable( CirclePoint );

		//debug
		`AILog( GetFuncName()@CirclePoint@Dist@bResult, 'Combat' );

		return bResult;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@GetStateName(), 'State' );

	SetTimer( 0.5f + (FRand() * 0.5f), FALSE, nameof(self.CircleTimer), self );
	while( IsTimerActive( 'CircleTimer', self ) )
	{
		if( GetCirclePoint() )
		{
			MoveTo( CirclePoint, Enemy, FALSE );
		}
		else
		{
			break;
		}
	}

	PopState();
}

defaultproperties
{
	CircleDistance=(X=128.f,Y=256.f)
}