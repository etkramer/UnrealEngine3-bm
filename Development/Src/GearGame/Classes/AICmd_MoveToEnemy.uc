/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_MoveToEnemy extends AICommand
	within GearAI;


var Actor Path, Find;
var float	Radius;
var bool    bWasFiring;
/** number of times remaining we can fail the MoveToGoal before we give up */
var int MoveTriesRemaining;

/** if this is TRUE, we will move all the way to the enemy instead of just to the first goal */
var() bool bCompleteMove;
/** how close to get to the enemy (only valid of bCompleteMove is TRUE) */
var float GoalDistance;
/** if enemy gets this far from us we should abandon the move */
var float AbandonDistance;
/** Last location of enemy... if they've moved too much, redo pathing */
var BasedPosition LastEnemyLocation;

/** are we allowed to fire during this move? */
var bool bAllowedToFire;

/** GoW global macros */

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool MoveToEnemy( GearAI AI, bool bInCompleteMove, float InGoalDistance, float InAbandonDistance, optional bool bInAllowedToFire=true )
{
	local AICmd_MoveToEnemy Cmd;

	if( AI != None && AI.Pawn != None )
	{
		Cmd = new(AI) class'AICmd_MoveToEnemy';
		if( Cmd != None )
		{
			Cmd.bCompleteMove = bInCompleteMove;
			Cmd.GoalDistance = InGoalDistance;
			Cmd.AbandonDistance = InAbandonDistance;
			Cmd.bAllowedToFire = bInAllowedToFire;
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@Enemy@GoalDistance@bCompleteMove;
}

function Pushed()
{
	Super.Pushed();
	bWasFiring = (bWeaponCanFire && bFire == 1);
	GotoState('Moving');
}

function Popped()
{
	Super.Popped();

	ClearTimer( 'CheckReachedEnemy', self );

	// Mark failure to move
	bFailedToMoveToEnemy = (Status != 'Success');
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if(!bCompleteMove)
	{
		Status = ChildStatus;
		PopCommand( self );
	}
}


function CheckReachedEnemy()
{
	local float DistToEnemySq, DistEnemyMovedSq;

	// only abort if we don't have a child, or our child is a move command.. otherwise let it slide 
	if(ChildCommand == none || (ChildCommand.IsA('AICmd_MoveToGoal') && ChildCommand.ChildCommand == none))
	{
		if (Enemy != None)
		{
			DistToEnemySq = VSizeSq(Enemy.Location-Pawn.Location);

		//	`log( GetFuncName()@VSize(Enemy.Location-Pawn.Location)@"--"@GoalDistance@"--"@AbandonDistance);

			if( GoalDistance > 0.f && DistToEnemySq <= GoalDistance*GoalDistance && FastTrace(Enemy.Location,Pawn.Location) )
			{
				//debug
				`AILog( "Move got us within goal dist..." );

				AbortCommand( self );
			}
			else if( AbandonDistance > 0.f && DistToEnemySq >= AbandonDistance*AbandonDistance )
			{
				//debug
				`AILog( "Move took us outside abandon dist..." );

				AbortCommand( self );
			}
			else if (GearAI_TDM(Outer) == None)
			{
				DistEnemyMovedSq = VSizeSq(GetBasedPosition(LastEnemyLocation)-Enemy.Location);
				if( DistEnemyMovedSq > 1024.f*1024.f || !FastTrace(Enemy.Location,GetBasedPosition(LastEnemyLocation)) )
				{
					//debug
					`AILog( "Enemy moved too far from pathed location... pop child command and redo pathing" );
					ChildCommand.Status = 'Success';
					AbortCommand( ChildCommand );
				}
			}
		}
		else
		{
			`AILog("Enemy became invalid, aborting");
			AbortCommand(self);
		}
	}
}


state Moving `DEBUGSTATE
{
Begin:
	`AILog("BEGIN TAG"@GetSTatename());
	// UnMark failure to move
	bFailedToMoveToEnemy = FALSE;

	SelectEnemy();
	FireTarget = Enemy;
	if( HasValidEnemy() && AllowedToMove() && MoveIsInterruptable() )
	{
		`AILog("Found valid enemy");
		bReachedMoveGoal = FALSE;

		Radius = Pawn.GetCollisionRadius() + Enemy.GetCollisionRadius() + Pawn.MeleeRange + 1.0;
		// If enemy is on a cart, path find to the cart
		Find = Enemy;
		if( GearInterActorAttachableBase(Enemy.Base) != None )
		{
			Find = Enemy.Base;
			Radius = 0.f;
		}
		Radius = FMax(0.f, GoalDistance - Radius); // subtract because collision radius already considered by ReachedDestination()


Loop:
		// If enemy directly reachable
		`AILog("Going to next path to "$find);

		SetBasedPosition(LastEnemyLocation,Find.Location);

		if( IsValidDirectMoveGoal( Find ) )
		{
			`AILog("Calling SetMoveGoal to "$MoveToEnemy_GetMoveFocus()@Radius@bAllowedToFire);
			SetTimer( 0.5f, TRUE, nameof(self.CheckReachedEnemy), self );
			SetMoveGoal( Find, MoveToEnemy_GetMoveFocus(), TRUE, Radius,,,,bAllowedToFire );
			ClearTimer( 'CheckReachedEnemy', self );
			`AILog("Finished moving to enemy");
			Status = 'Success';
			PopCommand(self);
		}
		else
		{
			// Try to find path to enemy
			Path = GeneratePathTo( Find,, TRUE );

			// If no path available
			if( Path == None )
			{
				// Update failed path time
				SetFailedPathToEnemy( Enemy );
				GotoState( 'DelayFailure' );
			}
			else
			{
				//debug
				`AILog( "Found path toward enemy..."@Find@Path@bCompleteMove@Pawn.Anchor@CombatZoneList.Length, 'Move' );

				//`log( Pawn.Anchor@GearPawn(Enemy).GetCombatZoneForNav(Pawn.Anchor)@Path@GearPawn(Enemy).GetCombatZoneForNav(NavigationPoint(Path)) );
				
				if(!bCompleteMove)
				{
					// Move to first path...
					// will research for new path when reached first one
					// make sure we don't try to move to our anchor
					if(Path == Pawn.Anchor)
					{
						Path = RouteCache[1];

						// clip off the end of the routecache
						if(RouteCache.length > 2)
						{
							RouteCache_RemoveIndex(2,RouteCache.length-2);
						}
					}
					else
					{
						if(RouteCache.length > 1)
						{
							RouteCache_RemoveIndex(1,RouteCache.length-1);
						}
					}
				}
				else
				{
					Path = Find;
				}

				SetTimer( 0.5f, TRUE, nameof(CheckReachedEnemy), self );
				SetMoveGoal(Path, MoveToEnemy_GetMoveFocus(), true,, true,,,bAllowedToFire);
				ClearTimer( 'CheckReachedEnemy', self );

				if (ChildStatus != 'Success' && --MoveTriesRemaining <= 0)
				{
					GotoState('DelayFailure');
				}
				if(bWasFiring)
				{
					StartFiring();
				}
				Sleep(0.0);
				if (!HasValidEnemy())
				{
					`AILog("Lost enemy or enemy became invalid, aborting");
					AbortCommand(self);
				}
				Goto('Loop');
			}
		}
	}
	else
	{
		`AILog("No valid enemy, or not allowed to move.. bailing ValidEnemy?:"@HasValidEnemy()@"Allowed to move?"@AllowedToMove()@"Move Interruptable?"@MoveIsInterruptable());
		GotoState( 'DelayFailure' );
	}
}

defaultproperties
{
	MoveTriesRemaining=3
}
