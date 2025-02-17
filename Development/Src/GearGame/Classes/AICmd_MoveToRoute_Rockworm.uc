/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AICmd_MoveToRoute_Rockworm extends AICmd_MoveToRoute
	within GearAI;


/** Simple constructor that pushes a new instance of the command for the AI */
static function bool MoveToRoute( GearAI AI, Route NewRouteMoveGoal, optional ERouteDirection NewRouteDirection, optional bool inbInterruptable = TRUE )
{
	local AICmd_MoveToRoute_Rockworm Cmd;

	if( AI != None && NewRouteMoveGoal != None )
	{
		Cmd = new(AI) class'AICmd_MoveToRoute_Rockworm';
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

function Pushed()
{
	Super(AICommand).Pushed();

	bMovingToRoute   = TRUE;
	bShouldRoadieRun = (MoveAction != None && MoveAction.MovementStyle == EMS_Fast);

	// Start the move
	SetTether( GetNextRouteNode(bWrapped,bLastRouteWasEnd), Pawn.GetCollisionRadius(),,, bInterruptable );
}

function Popped()
{
	Super(AICommand).Popped();
	NotifyRouteComplete();
	bMovingToRoute = FALSE;
}

function Resumed( Name OldCommandName )
{
	local Actor NewRouteNode;

	Super(AICommand).Resumed( OldCommandName );

	// Don't continue routes when in combat
	if( bInterruptable && HasAnyEnemies() )
	{
		Status = (ChildStatus != 'Success') ? 'Failure' : 'Success';
		PopCommand( self );
	}
	else
	{
		// then grab the new route move point
		NewRouteNode = GetNextRouteNode(bWrapped,bLastRouteWasEnd);
		if( NewRouteNode != None )
		{
			// If there was a node then tether to that
			SetTether( NewRouteNode, Pawn.GetCollisionRadius(),,, bInterruptable );
		}
		else
		{
			Status = (ChildStatus != 'Success') ? 'Failure' : 'Success';
			PopCommand( self );
		}
	}
}

