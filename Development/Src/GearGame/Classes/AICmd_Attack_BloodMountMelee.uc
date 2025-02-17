/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Attack_BloodMountMelee extends AICommand_SpecialMove
	within GearAI;

// CIRCLE VARIABLES
/** Circling left (-1) or right (+1) */
var int CircleDir;
/** Min/Max distance of the arc for circling the enemy */
var Vector2D CircleDistance;
/** Point we are moving to for this circle step */
var transient Vector CirclePoint;

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_BloodMount_Melee;
	}

	function bool ShouldFinishRotation() { return TRUE; }

	function FinishedSpecialMove()
	{
		UnlockAI();
		CircleDir = (FRand() < 0.5f) ? -1 : 1;
		PushState( 'Circle' );
	}
}

function CircleTimer(); 

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

	SetTimer( 0.25f + (FRand() * 0.25f), FALSE, nameof(self.CircleTimer), self );
	while( IsTimerActive( 'CircleTimer', self ) )
	{
		if( GetCirclePoint() )
		{
			SetFocalPoint( Enemy.Location, TRUE );
			MoveTo( CirclePoint, Enemy, FALSE );
		}
		else
		{
			break;
		}
	}

	if( FRand() < 0.25f ) 
	{ 
		Goto( 'Begin' ); 
	}
	
	PopState();
}


defaultproperties
{
	CircleDistance=(X=128.f,Y=256.f)
}