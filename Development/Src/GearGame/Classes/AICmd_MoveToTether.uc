/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_MoveToTether extends AICommand
	within GearAI;

/** GoW global macros */

/** Actor we are currently attached to */
var Actor TetherActor;
/** Time the tether should persist */
var float TetherPersistDuration;
/** Desired tether distance, which TetherDistance will set back to when valid */
var float DesiredTetherDistance;
/** Dynamic tether distance */
var float TetherDistance;
/** Time at which the AI acquired the tether */
var transient float TetherAcquireTime;
/** Is this a dynamic (ie moving) tether? */
var bool bDynamicTether;
var bool bSetMovingToSquadLeader;
/** Tether move can be interupted */
var bool bInteruptable;
var bool bIsValidCache;

/** Last command push was a move to cover on the way to tether */
var bool	bMovedToCover;
/** Last command pushed was a peek over cover */
var bool	bPeeked;
/** No move to cover... making the final move to tether location because it's close enough */
var bool	bFinalMove;
/** Force final move (usually because a move to cover failed) */
var bool	bForceFinalMove;
/** Internal flag */
var bool	bFoundPath;

var CoverInfo CovToTether;
var float	  RouteCacheDist;
var Actor	  TetherTarget, PathToTarget;


/** Constructor that pushes a new instance of the command for the AI */
static function bool MoveToTether
(
	GearAI AI,
	Actor NewTetherActor,
	optional float NewTetherDistance,
	optional bool NewbDynamicTether,
	optional float NewTetherPersistDuration,
	optional bool NewbInteruptable,
	optional bool NewbIsValidCache
)
{
	local Controller	ControllerTether;
	local GearPRI		PRI;
	local bool			bNewSetMovingToSquadLeader;
    local AICmd_MoveToTether Cmd;

	// Don't allow tethering to controllers, instead to their pawns
	ControllerTether = Controller(NewTetherActor);
	if( ControllerTether != None )
	{
		// Only if it's a valid pawn
		if( ControllerTether.Pawn != None && ControllerTether.Pawn.Health > 0 )
		{
			// By default move to the pawn
			NewTetherActor = ControllerTether.Pawn;
			// check to see if it's a player with a squad we can join
			PRI = GearPRI(ControllerTether.PlayerReplicationInfo);
			if( PRI != None && AI.IsFriendly( ControllerTether ) )
			{
				// if we're already in the squad don't join it again
				if(AI.Squad.SquadName != PRI.SquadName)
				{
					AI.SetSquadName( PRI.SquadName );
				}

				// if we have a squad leader and that squadleader is not ourselves, go to squad position
				if( AI.GetSquadLeader() != None  && AI.GetSquadLeader() != AI)
				{
					// use the squad formation tether instead of the pawn
					bNewSetMovingToSquadLeader = TRUE;
					NewTetherActor = AI.GetSquadPosition();

					// mark it as a dynamic tether
					NewbDynamicTether = TRUE;
				}

			}
		}
		else
		{
			NewTetherActor = None;
		}
	}


	if( AI != None && NewTetherActor != None )
	{
		if( Pawn(NewTetherActor) != None )
		{
			// Enforce a min tether distance
			NewTetherDistance = FMax( NewTetherDistance, 256.f );
		}

		Cmd = new(AI) class'AICmd_MoveToTether';
		if( Cmd != None )
		{
			Cmd.TetherActor				= NewTetherActor;
			Cmd.TetherPersistDuration	= NewTetherPersistDuration;
			Cmd.TetherAcquireTime		= AI.WorldInfo.TimeSeconds;
			Cmd.TetherDistance			= NewTetherDistance;
			Cmd.DesiredTetherDistance	= Cmd.TetherDistance;
			Cmd.bDynamicTether			= NewbDynamicTether;				// Mark whether or not it is dynamic
			Cmd.bSetMovingToSquadLeader = bNewSetMovingToSquadLeader;
			Cmd.bInteruptable			= NewbInteruptable;
			Cmd.bIsValidCache			= NewbIsValidCache;

			AI.PushCommand( Cmd );
			return TRUE;
		}
    }
    return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Tether:"@TetherActor@TetherDistance;
}

function SetTetherActor( Actor NewTetherActor )
{
	TetherActor = NewTetherActor;

	if( AIOwner != None )
	{
		AIOwner.TetherActor				= TetherActor;
		AIOwner.TetherPersistDuration	= TetherPersistDuration;
		AIOwner.TetherAcquireTime		= TetherAcquireTime;
		AIOwner.TetherDistance			= TetherDistance;
		AIOwner.DesiredTetherDistance	= DesiredTetherDistance;
		AIOwner.bDynamicTether			= bDynamicTether;
	}
}

function Pushed()
{
	Super.Pushed();

	//debug
	`AILog( "Set tether,"@TetherActor@TetherDistance@"dynamic?"@bDynamicTether@"duration?"@TetherPersistDuration@"moving to squad?"@bMovingToSquadLeader, 'Move' );

	bReachedTether			 = FALSE;
	bMovingToTether			 = TRUE;
	bTeleportOnTetherFailure = FALSE;
	if( bSetMovingToSquadLeader )
	{
		bMovingToSquadLeader = TRUE;
	}

	if( TetherPersistDuration > 0.f )
	{
		SetTimer( TetherPersistDuration, FALSE, nameof(self.TetherPersistDurationExpired), self );
	}
	else
	{
		ClearTimer( 'TetherPersistDurationExpired', self );
	}

	// Setup AI info with the actor we are moving to
	SetTetherActor( TetherActor );

	if( !IsWithinTether( Pawn.Location ) )
	{
		DoMove();
	}
	else
	{
		//debug
		`AILog("Already within tether:"@TetherActor@TetherDistance@VSize(Pawn.Location-TetherActor.Location));

		// Already touching it, notify completion
		NotifyReachedTether();

		GotoState('DelaySuccess');
	}
}

function Popped()
{
	Super.Popped();

	bTeleportOnTetherFailure = FALSE;

	if( bSetMovingToSquadLeader )
	{
		bMovingToSquadLeader = FALSE;
	}
	// No longer moving to tether
	bMovingToTether = FALSE;

	ClearTimer('TryPeek', self);

	bReachedMoveGoal = (Status == 'Success');
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if( ChildStatus == 'Success' )
	{
		bIsValidCache=false;
		SetTetherActor( TetherActor );

		//debug
		`AILog( "Resuming..."@bMovedToCover@bMovingToSquadLeader@bMovingToCover@bFinalMove@IsWithinTether(Pawn.Location)@AIOwner.TetherActor );

		// If moved to cover, try peeking over the cover
		if( bMovedToCover )
		{
			bMovedToCover = FALSE;
			SetTimer( 0.5f + FRand() * 1.5f, FALSE, nameof(self.TryPeek), self );
		}
		else
		// Otherwise, if within tether, notify
		if( (!bMovingToCover && bMovingToSquadLeader) || bFinalMove || IsWithinTether(Pawn.Location) )
		{			
			// we never want to return success if we are not near our cover because we'll end up setting cover type when we're far away from our cover
			if(bMovingToCover && !Pawn.ReachedDestination(TetherActor))
			{
				`AILog("Moving to Cover finished, but not within tether, so returning failure..."@bMovedToCover@bMovingToSquadLeader@bMovingToCover@bFinalMove@IsWithinTether(Pawn.Location)@AIOwner.TetherActor);
				NotifyReachedTether();
				Status = 'Failure';
				PopCommand( self );
			}
			else
			{
				NotifyReachedTether();
				Status = 'Success';
				PopCommand( self );
			}
			
		}
		else
		{
			// Otherwise, do another move (delay after peeking)
			if( bPeeked )
			{
				bPeeked = FALSE;
				SetTimer( 0.5f + FRand() * 0.5f, FALSE, nameof(self.DoMove), self );
			}
			else
			{
				DoMove();
			}
		}
	}
	else
	{
		//debug
		`AILog( "Failed to reached tether"@TetherActor@"move action:"@MoveAction@bFinalMove@bForceFinalMove@"teleport?"@bTeleportOnTetherFailure@ChildStatus );

		bMovedToCover = false;

		if( !bFinalMove && !bForceFinalMove )
		{
			bForceFinalMove = TRUE;
			DoMove();
		}
		else
		{
			if( bTeleportOnTetherFailure )
			{
				//debug
				`AILog("Teleporting to tether location");

				Pawn.SetLocation(GetTetherTarget().Location);
				NotifyReachedTether();
			}

			Status = 'Failure';
			PopCommand( self );
		}
	}
}

final function TetherPersistDurationExpired()
{
	//debug
	`AILog("Tether duration expired, clearing:"@TetherActor);

	SetTetherActor( None );
}

final private function DoMove()
{
	local Actor		SquadLeaderPos;
	local bool		bCanUseCover;

	bReachedTether	= FALSE;

	// Get the actor we are tethered to
	TetherTarget = GetTetherTarget();
	if( TetherTarget != None )
	{
		bCanUseCover = CoverOwner != None &&
						(MovementMood == AIMM_WithCover ||
							(IsAlert() && (GetNearEnemyDistance(Pawn.Location) < EnemyDistance_Medium)));

		if( bMovingToSquadLeader )
		{
			SquadLeaderPos = GetSquadLeaderPosition();
			bCanUseCover = bCanUseCover && (IsCloseToSquadLeader(Pawn.Location) || MovementMood == AIMM_WithCover);
			// Roadie run if in combat (regroup!) or leader is roadie running
			bShouldRoadieRun = WantsToRoadieRunToLeader();
		}
		else
		{
			bCanUseCover = bCanUseCover &&
							(MovementMood == AIMM_WithCover ||
								(bInteruptable && VSize(TetherTarget.Location - Pawn.Location) > 256.f));
			// Roadie run if move action tells us too
			bShouldRoadieRun = (MoveAction != None && MoveAction.MovementStyle == EMS_Fast);
		}

		//debug
		`AILog( "Trying to move towards tether target:"@TetherTarget@bShouldRoadieRun@bMovingToSquadLeader@TetherDistance@bCanUseCover@bIsValidCache );

		bFoundPath	= bIsValidCache;
		if( !bFoundPath )
		{
			// If can't use cover or close enough to target... check if we are directly reachable
			if( (!bCanUseCover || VSize(TetherTarget.Location-Pawn.Location) < 256.f) && ActorReachable( TetherTarget ) )
			{
				bFoundPath = TRUE;
				bIsValidCache = FALSE;
				RouteCache_Empty();
			}
			else
			{
				// Generate path to target
				bFoundPath = (GeneratePathTo( TetherTarget, TetherDistance, TRUE ) != None);
				// If no path found and have valid squad leader position
				if( !bFoundPath && SquadLeaderPos != None )
				{
					// Try to find a path to the squad leader
					bFoundPath = (GeneratePathTo( SquadLeaderPos ) != None);
					TetherTarget = SquadLeaderPos;
				}
			}
		}

		if( bFoundPath )
		{
			// Save initial route cache distance
			RouteCacheDist = GetRouteCacheDistance();

			//debug
			`AILog( "Initial path..."@RouteCacheDist@bForceFinalMove@TetherTarget@SquadLeaderPos@bCanUseCover );

			// If forcing final move or path isn't long enough
			if( bForceFinalMove || RouteCacheDist < 256.f )
			{
				//debug
				if (!bForceFinalMove)
				{
					`AILog( "Too close to move goal already"@TetherTarget@RouteCacheDist );
				}

				// Move right to the end w/o using cover
				bFinalMove	 = TRUE;
				bCanUseCover = FALSE;
			}

			// If using cover...
			if( bCanUseCover )
			{
				GotoState( 'PathToCover', 'Begin' );
				return;
			}


			// Move to the goal
			SetMoveGoal( TetherTarget,, bInteruptable,GetTetherMoveOffset(TetherTarget), (RouteCacheDist > 0.f),,TRUE );
			return;
		}
	}

	if( TetherActor != None && PointReachable(TetherActor.Location) )
	{
		//debug
		`AILog( "Move directly to tether point..." );

		bFinalMove = true;
		SetMovePoint(TetherActor.Location,, bInteruptable, FMin(TetherDistance, Pawn.GetCollisionRadius()));
		return;
	}

	`AILog(GetFuncName()@"not reachable, and no path found.. bailing");
	FailedToFindPath( TetherTarget );
}


/**
 *	Used to try to path through cover along the way to the tether.
 *	Delays the cover search to another frame, so we aren't doing (potentially) 3
 *	path searches on a single frame.
 */
state PathToCover
{
Begin:
	// Small delay between path searches
	Sleep(0.1f);

	// Limit our route length
	LimitRouteCacheDistance( RouteCacheDist * GetCoverDistModifier() );
	PathToTarget = RouteCache[RouteCache.Length-1];

	//debug
	`AILog( "About to search for cover..."@PathToTarget );

	// If found cover along the path to the last node in the route and
	// path is shorter than our original path to the tether target
	bFoundPath = CoverOwner.EvaluateCover( SearchType_AlongPath, PathToTarget, CovToTether );

	//debug
	`AILog( "path?"@bFoundPath@GetRouteCacheDistance()@RouteCacheDist@CovToTether.Link@CovToTether.SlotIdx );

	if( bFoundPath &&
		GetRouteCacheDistance() < RouteCacheDist )
	{
		// Move using cover
		bMovedToCover = TRUE;
		CoverOwner.SetCoverGoal( CovToTether, TRUE );
		Goto('End');
	}
	else
	{
		// Otherwise, failed to use cover and route cache is no longer good
		bFoundPath = FALSE;
	}

	if( bFoundPath )
	{
		//debug
		`AILog( "Found path to new tether target"@PathToTarget );

		TetherTarget = PathToTarget;

		// If moving to squad leader using original route... limit the cache length
		if( bMovingToSquadLeader && !bFinalMove )
		{
			TetherTarget = LimitRouteCacheDistance(FMin(VSize(TetherTarget.Location - Pawn.Location), 512.0));

			//debug
			`AILog( "Limit cache moving to squad leader" );
		}
	}

	// Move to the goal
	SetMoveGoal( TetherTarget,, bInteruptable,, bFoundPath,,TRUE );

End:
	Stop;
};


/**
 *	Adds a small delay once getting into cover, peek over
 *	before continuing move toward tether
 */
final function TryPeek()
{
	CoverOwner.NextAllowPeekTime = 0.f;
	bPeeked = TRUE;
	if( !CoverOwner.PeekFromCover() )
	{
		//debug
		`AILog( "Failed to peek... SHOULD NOT HAPPEN" );

		bPeeked = FALSE;
		DoMove();
	}
}

final function FailedToFindPath( Actor A )
{
	//debug
	`AILog( GetFuncName()@"to"@TetherActor@"("$A$")"@"from"@Pawn.Anchor, 'Move' );

	InvalidateAnchor( Pawn.Anchor );
	
	GotoState( 'DelayFailure' );
}

function float GetCoverDistModifier()
{
	local float Result;

	Result = 1.f;
	switch( CombatMood )
	{
	case AICM_Passive: //fall through
	case AICM_Ambush:
		Result = 0.25f;
		break;
	case AICM_Aggressive:
		Result = 0.65f;
		break;
	case AICM_Normal:
		Result = 0.40f;
		break;
	}
	return Result;
}


/**
* Called once the tether has been reached.
*/
final function NotifyReachedTether()
{
	//debug
	`AILog( "Reached tether"@MoveAction@TetherPersistDuration@TimeSince(TetherAcquireTime) );

	bSpawnerTetherInterruptable = TRUE;
	bReachedTether = TRUE;

	// clear the focus
	Focus = None;

	// If there is an assigned move action
	if( MoveAction != None )
	{
		//debug
		`AILog("Clear move action"@MoveAction@MoveAction.bClearTetherOnArrival@bMovingToSquadLeader);

	
		Outer.NotifyReachedTether(TetherActor);
		// check to see if the scripted tether should be cleared
		if( MoveAction.bClearTetherOnArrival || bMovingToSquadLeader )
		{
			SetTetherActor( None );
		}
		else
		{
			// If tether actor hasn't been cleared
			// Keep it! (instead of moving back to squad formation position)
			// MUST do this for moves that have bClearTetherOnArrival == FALSE
			// so AI doesn't clear tether in Sleep section of Action_Idle and
			// move back to squad leader
			bKeepTether = TRUE;
		}

		// then notify the action that the move is complete
		if(!bMovingToRoute)
		{
			MoveAction.ReachedGoal( AIOwner );
			// tether dist 0 is magical, means don't ever move from where I just put you
			if(AllowedToMove())
			{
				ClearMoveAction();
			}

		}
	}
	else
	{
		if( TetherPersistDuration == 0.f || TimeSince(TetherAcquireTime) >= TetherPersistDuration )
		{
			// clear AI generated tethers by default
			`AILog("Clearing tether because we arrived at our tether and TetherPersistDuration was 0");
			SetTetherActor( None );
			ClearTimer( 'TetherPersistDurationExpired', self );
		}
	}
}

event DrawDebug( GearHUD_Base HUD, Name Category )
{
	Super.DrawDebug( HUD, Category );

	if( TetherActor != None && Category == 'Pathing')
	{
		// PURPLE
		DrawDebugLine( Pawn.Location, TetherActor.Location, 184, 152, 250 );
		DrawDebugCylinder( TetherActor.Location, TetherActor.Location, TetherDistance, 10.f, 184, 152, 250 );
	}
}

defaultproperties
{
}
