/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_NemaSlug extends AICommand_Base_Combat
	within GearAI_NemaSlug;

var float NextMeleeDamageTime;
var vector movept;
var float NextLatMoveTime;

function Pushed()
{
	Super.Pushed();
	GotoState('InCombat');
}

function Popped()
{
	Super.Popped();
	ReactionManager.UnSuppressChannel('EnemyWithinMeleeDistance');
}

function Paused(AICommand NewCommand)
{
	Super.Paused(NewCommand);
	ReactionManager.SuppressChannel('EnemyWithinMeleeDistance');
}

function Resumed( Name OldCommandName )
{
	Super.Resumed(OldCommandName);
	ReactionManager.UnSuppressChannel('EnemyWithinMeleeDistance');
}

function DoMeleeDamage()
{
	NextMeleeDamageTime = WorldInfo.TimeSeconds + 0.5f + FRand() * 1.5f;
	if( Enemy != None )
	{
		Enemy.TakeDamage( 30, AIOwner, Enemy.Location, vect(0,0,0), class'GDT_Nemaslug_Melee' );
	}

	//debug
	`AILog( GetFuncName()@(NextMeleeDamageTime-WorldInfo.TimeSeconds) );
}

function ForceClaimFireTicket()
{
	local GearPawn GPEnemy;

	GPEnemy = GearPawn(Enemy);
	if(GPEnemy != none)
	{
		GPEnemy.ClaimFireTicket(outer);
	}
}
state InCombat
{
	function bool GetMovePoint(out vector TryPt)
	{
		local vector MeToEnemy;
		local vector right;
		MeToEnemy = Normal(Enemy.Location - Pawn.location);
			
		right = MeToEnemy cross vect(0,0,1);
		right *= EnemyDistance_Melee;
		TryPt = Pawn.Location + right;
		if(FRand() < 0.5)
		{
			TryPt = Pawn.Location - right;
		}

		if(PointReachable(TryPt))
		{
			return true;
		}
		else
		{
			TryPt = Pawn.Location + (Pawn.Location - TryPt);
			if(PointReachable(TryPt))
			{
				return true;
			}
		}
		
		return false;
	}

Begin:
	if( !SelectTarget() )
	{
		//debug
		`AILog( "Could not select enemy!" );

		Pawn.ZeroMovementVariables();
		GotoState('DelayFailure');
	}
	if( Enemy == none )
	{
		Sleep(0.1f);
		Goto( 'Begin' );
	}

	// claim a ticket on the dude
	ForceClaimFireTicket();

	if( IsMeleeRange( Enemy.Location ) )
	{
		if(WorldInfo.TimeSeconds > NextLatMoveTime)
		{
			`AILog("Moving lat");
			// move laterally to the dir from us to enemy
			if(GetMovePoint(movept))
			{
				ReactionManager.SuppressChannel('EnemyWithinMeleeDistance');
				MoveTo(movept);
				ReactionManager.UnSuppressChannel('EnemyWithinMeleeDistance');

				Pawn.ZeroMovementVariables();
				NextLatMoveTime=WorldInfo.TimeSeconds+RandRange(2.5f,5.0f);
			}
		}

		CheckInterruptCombatTransitions();
		Sleep(0.5f);
	}
	else
	{
		SetEnemyMoveGoal(true);
		if( ChildStatus == 'Failure' )
		{
			AbortCommand( self );
		}
	}

	Goto( 'Begin' );
}

defaultproperties
{
}
