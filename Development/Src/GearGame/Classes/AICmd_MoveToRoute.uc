/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_MoveToRoute extends AICommand
	within GearAI;
	
/** GoW global macros */

/** Currently assigned route, which is used to update tethers */
var Route			RouteMoveGoal;
/** Current direction for the route */
var	ERouteDirection	RouteDirection;
/** Current index within route stream */
var int				RouteIndex;
/** Running route is interruptable by enemies? */
var bool			bInterruptable;


/** internal vars used for route stitching */
var Actor CurRouteActor;
var Actor FinalStitchedGoal;
var NavigationPoint Anchor;
var NavigationPoint OldAnchor;
var int OldRouteIdx;
var int i;
var int WayPointsProcessed;
var byte bWrapped;
var byte bLastRouteWasEnd;
var ERouteDirection FallBackDirection; // direction to use if we need to re-stitch for any reason
var bool bNeedsReStitch;
var int	 FailCount;
/******************************************/

var array<NavigationPoint> StitchedRoute;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool MoveToRoute( GearAI AI, Route NewRouteMoveGoal, optional ERouteDirection NewRouteDirection, optional bool inbInterruptable = TRUE )
{
    local AICmd_MoveToRoute Cmd;

    if( AI != None && NewRouteMoveGoal != None )
    {
        Cmd = new(AI) class'AICmd_MoveToRoute';
        if( Cmd != None )
        {
			
            Cmd.RouteMoveGoal		= NewRouteMoveGoal;
			Cmd.RouteDirection		= NewRouteDirection;
			Cmd.RouteIndex			= -1;
			Cmd.bInterruptable		= inbInterruptable;
            AI.PushCommand( Cmd );
            return TRUE;
        }
    }
    return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Route:"@RouteMoveGoal@"Direction:"@RouteDirection;
}



function Pushed()
{
	Super.Pushed();

	bMovingToRoute   = TRUE;
	ActiveRoute = RouteMoveGoal;
	FallBackDirection = RouteDirection;
	bShouldRoadieRun = (MoveAction != None && MoveAction.MovementStyle == EMS_Fast);
	OldAnchor = Pawn.Anchor;
	GotoState('StitchRoute');
}

function Popped()
{
	Super.Popped();

	// make sure that this gets cleared so that other commands don't path from the wrong location
	Pawn.bForceKeepAnchor = Pawn.default.bForceKeepAnchor;
	Pawn.SetAnchor(OldAnchor);

	// clear out existing tether in case we got interrupted
	TetherActor = None;
	ActiveRoute = None;
	NotifyRouteComplete();
	bMovingToRoute = FALSE;
}

function Resumed( Name OldCommandName )
{

	Super.Resumed( OldCommandName );

	// Don't continue routes when in combat
	if( bInterruptable && HasAnyEnemies() )
	{
		Status = (ChildStatus != 'Success') ? 'Failure' : 'Success';
		PopCommand( self );
	}
	else
	{

		// if we're not done yet, stitch again!
		if( FailCount < 5 && (bNeedsReStitch || (RouteIndex != -1 && bLastRouteWasEnd == 0)) )
		{
			// if child command failed (we failed to make our target) try again from the best starting node
			if(ChildStatus == 'Failure')
			{
				RouteIndex = -1;
			}

			GotoState('StitchRoute',,TRUE);
		}
		else
		{
			Status = (ChildStatus != 'Success') ? 'Failure' : 'Success';
			PopCommand( self );
		}
	}
}

function Paused(AICommand NewCommand)
{
	//debug
	`AILog( "COMMAND PAUSED:"@self@"by"@NewCommand );

	// make sure that this gets cleared so that other commands don't path from the wrong location
	Pawn.bForceKeepAnchor = Pawn.default.bForceKeepAnchor;
	Pawn.SetAnchor(OldAnchor);
}


/**
 *	Retrieve next point along the route
 *	Handle reversing directions, wrapping route, etc
 */
final protected function Actor GetNextRouteNode(out byte bReverse, out byte bComplete)
{
	bComplete=0;
	bReverse=0;
	bReachedMoveGoal = FALSE;

	if( RouteIndex < 0 )
	{
		RouteIndex = RouteMoveGoal.MoveOntoRoutePath( Pawn, RouteDirection );
	}
	else
	{

		if( RouteDirection == ERD_Forward )
		{
			`AILog(GetFuncName()@"RouteDir=FORWARD, resolving index "$RouteIndex+1);
			RouteIndex = RouteMoveGoal.ResolveRouteIndex( RouteIndex+1, RouteDirection, bComplete, bReverse );
			if( bReverse != 0 )
			{
				FallBackDirection = ERD_Forward;
				RouteDirection	= ERD_Reverse;
			}
		}
		else
		{
			`AILog(GetFuncName()@"RouteDir != FORWARD, resolving index "$RouteIndex-1);
			RouteIndex = RouteMoveGoal.ResolveRouteIndex( RouteIndex-1, RouteDirection, bComplete, bReverse );
			if( bReverse != 0 )
			{
				FallBackDirection = ERD_Reverse;
				RouteDirection	= ERD_Forward;
			}
		}

	}

	`AILog(GetFuncName()@"Using route index"@RouteIndex@( RouteIndex >= 0 ? RouteMoveGoal.RouteList[RouteIndex].Actor : None )@bReverse);
	return ( RouteIndex >= 0 ? RouteMoveGoal.RouteList[RouteIndex].Actor : None );
}



/** Route has been completed, trigger events */
final function NotifyRouteComplete()
{
	//debug
	`AILog( GetFuncName()@"L?"@IsSquadLeader()@"SquadRoute:"@Squad.SquadRoute, 'Scripted' );

	// this is so that if we're given a route from a spawner tether we clear it out once we arrive
	bSpawnerTetherInterruptable=TRUE;

	// if we're the squad leader and we got to the end of the route, clear the squad route
	if(IsSquadLeader() &&  Squad.SquadRoute != none )
	{
		Squad.SetSquadRoute(none);
	}
	// Notify the move action that the goal has been reached
	if( MoveAction != None )
	{
		if(Status=='Success')
		{
			MoveAction.ReachedGoal( AIOwner );
		}
		ClearMoveAction();
	}
}

event DrawDebug( GearHUD_Base HUD, name Category )
{
	local int Idx;

	Super.DrawDebug( HUD, Category );

	if(Category != 'Pathing')
	{
		return;
	}

	if( RouteMoveGoal != None )
	{
		for( Idx = 1; Idx < RouteMoveGoal.RouteList.Length; Idx++ )
		{
			DrawDebugLine( RouteMoveGoal.RouteList[Idx-1].Actor.Location + vect(0,0,15), RouteMoveGoal.RouteList[Idx].Actor.Location + vect(0,0,15), 255, 128, 255 );
		}
	}
}

// if a repath is required, we need to abort active movetogoal, so that we can have a chance to re-stich
function NotifyNeedRepath()
{
	`AILog(self@GetFuncName()@"Aborting tether/movetogoal and initiating a re-stitch"@RouteIndex@bLastRouteWasEnd@RouteMoveGoal.RouteType@FallBackDirection);
	ChildCommand.NotifyNeedRepath();
	RouteIndex=-1;
	RouteDirection = FallBackDirection;	
	bNeedsReStitch=true;
	bReevaluatePath=false;
	AbortCommand(none,class'AICmd_MoveToTether');
	AbortCommand(none,class'AICmd_MoveToGoal');	
}

state StitchRoute
{
	function int GetRouteLength()
	{
		if(RouteMoveGoal.RouteType == ERT_Circle)
		{
			return RouteMoveGoal.RouteList.Length-1;
		}

		return RouteMoveGoal.RouteList.Length;
	}
Begin:
	// initialize stuff 
	FinalStitchedGoal=none;
	StitchedRoute.length=0;
	WayPointsProcessed=0;
	RouteCache_Empty();

	Anchor=Pawn.Anchor;
	OldAnchor = Anchor;

	OldRouteIdx = RouteIndex;
	//`AILog("GetNextRouteNode 214");
	CurRouteActor = GetNextRouteNode(bWrapped,bLastRouteWasEnd);
	while(bWrapped==0 && bLastRouteWasEnd==0 && WayPointsProcessed++ < GetRouteLength())
	{
		// if we're right on top of the current node or the current node is NULL, skip it and go to the next one
		if( (CurRouteActor == none || OldAnchor == CurRouteActor || Pawn.ReachedDestination(CurRouteActor)) && (WayPointsProcessed <  GetRouteLength()) )
		{
			`AILog("Skipping current waypoint:"@CurRouteActor);
			//`AILog("GetNextRouteNode 222");
			CurRouteActor = GetNextRouteNode(bWrapped,bLastRouteWasEnd);
			continue;
		}

		// if we hit a non-navpt route goal, pull counter back and bail
		if(!CurRouteActor.IsA('NavigationPoint'))
		{
			RouteIndex = OldRouteIdx;
			break;
		}

		// path to our current route point

		// if we're mucking with the anchor, don't let it get reset
		if(Anchor != Pawn.Anchor)
		{
			Pawn.bForceKeepAnchor = TRUE;
		}		

		// set the anchor so we start the A* in the right place
		if(Anchor == none && Pawn.bForceKeepAnchor)
		{
			`AILog("WARNING!"@GetFuncName()@self@"badness during route stitching!  Anchor was none?!? -- CurRouteActor:"@CurRouteActor@"WayPointsProcessed:"@WayPointsProcessed@"OldAnchor:"@OldAnchor@"Pawn.Anchor"@Pawn.Anchor@"Pawn.bForceKeepAnchor"@Pawn.bForceKeepAnchor);
			GotoState('DelayFailure');
		}
		Pawn.SetAnchor(Anchor);
		if( GeneratePathTo(CurRouteActor) == none)
		{
			FailCount++;
			`AILog(GetStateName()@Anchor@pawn.bForceKeepAnchor@"StitchPath failed to path to "@CurRouteActor);
			RouteIndex = OldRouteIdx;
			break;
		}

		// advance our final goal ref
		FinalStitchedGoal = CurRouteActor;

		// append newly found path to our stitched path
		`AILog(GetStateName()@"Incremental path from"@Anchor@"to"@CurRouteActor@"...",'RouteDebug');
		for(i=0;i<RouteCache.length-1;i++)
		{
			`AILog(GetFuncName()@i@RouteCache[i],'RouteDebug');
			StitchedRoute.AddItem(RouteCache[i]);
		}

		if(WayPointsProcessed <  GetRouteLength())
		{
			OldRouteIdx = RouteIndex;
			Anchor=NavigationPoint(CurRouteActor);
			//`AILog("GetNextRouteNode 271");
			CurRouteActor = GetNextRouteNode(bWrapped,bLastRouteWasEnd);
			`AILog(GetStateName()@self@outer@CurRouteActor@"++"@WayPointsProcessed@"/"@GetRouteLength(),'RouteDebug');
			Sleep(0.0);//don't pathfind more than once a frame
		}

		//`AILog("End while loop Cond:"@"bWrapped==0:"@bWrapped==0@"bLastRouteWasEnd==0"@bLastRouteWasEnd==0@"WayPointsProcessed++ < GetRouteLength()"@WayPointsProcessed < GetRouteLength());
	}

	if(bWrapped!=0 && bLastRouteWasEnd==0)
	{
		`AILog("Wrapped during route stitch! setting routeIndex back to "$RouteIndex@"..",'RouteDebug');
		RouteIndex = OldRouteIdx;
	}

	// restore stuff we messed with :)
	Pawn.bForceKeepAnchor = Pawn.default.bForceKeepAnchor;
	Pawn.SetAnchor(OldAnchor);


	// copy stitched route into routecache, assuming we actually stitched anything
	if(FinalStitchedGoal != none)
	{
		StitchedRoute.AddItem(RouteCache[RouteCache.length-1]);
		RouteCache_Empty();


		`AILog(GetStateName()@outer@"Final Stitched path:  ...",'RouteDebug');
		for(i=0;i<StitchedRoute.length;i++)
		{
			//Pawn.DrawDebugLine(StitchedRoute[i].Location,StitchedRoute[i+i].Location,255,255,0,TRUE);
			`AILog(GetStateName()@i@StitchedRoute[i],'RouteDebug');
			RouteCache_AddItem(StitchedRoute[i]);
		}
	}
	else if(bLastRouteWasEnd == 0)
	{
		// if we didn't do any stitching, then just return nextroute like normal
		//`AILog("GetNextRouteNode 307");
		FinalStitchedGoal=GetNextRouteNode(bWrapped,bLastRouteWasEnd);
	}
	
	`AILog(GetStateName()@"using"@FinalStitchedGoal,'RouteDebug');
	// Start the move
	`AILog(GetFuncName()@self@(RouteCache.length > 0)@MoveIsInterruptable());
	if(FinalStitchedGoal != none)
	{
		SetTether( FinalStitchedGoal, Pawn.GetCollisionRadius(),,, MoveIsInterruptable(), (RouteCache.length > 0));
	}
	`AILog("Made it here after SetTether, which means resume was not called..");
	Status = 'Success';
	PopCommand( self );
};

