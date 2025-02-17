/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_Nemacyst_MoveToGoal extends AICommand
	within GearAI_Nemacyst;

var actor Dest;

static function bool InitCommandUserActor(GearAI AI, Actor UserActor )
{
	local AICmd_Nemacyst_MoveToGoal Cmd;

	if( AI != None )
	{
		Cmd = new(GearAI_Nemacyst(AI)) class'AICmd_Nemacyst_MoveToGoal';
		if( Cmd != None )
		{
			Cmd.Dest=UserActor;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}
function Pushed()
{
	Super.Pushed();
	GotoState('Moving');
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

	pawn.Velocity = vect(0,0,0);

	// If there is an assigned move action
	if( MoveAction != None )
	{
		//debug
		`AILog("Clear move action"@MoveAction@MoveAction.bClearTetherOnArrival@bMovingToSquadLeader);

		Outer.NotifyReachedTether(TetherActor);
		// check to see if the scripted tether should be cleared
		if( MoveAction.bClearTetherOnArrival || bMovingToSquadLeader )
		{
			`AILog("Clearing tether"@MoveAction@MoveAction.bClearTetherOnArrival);
			TetherActor = None;
		}
		else
		{
			// If tether actor hasn't been cleared
			// Keep it! (instead of moving back to squad formation position)
			// MUST do this for moves that have bClearTetherOnArrival == FALSE
			// so AI doesn't clear tether in Sleep section of Action_Idle and
			// move back to squad leader
			bKeepTether = TRUE;
			`AILog("Keeping tether");
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
			TetherActor = None;
			ClearTimer( 'TetherPersistDurationExpired', self );
		}
	}
}

function Popped()
{
	Super.Popped();
	NotifyReachedTether();
}


state Moving
{
Begin:
	MoveToward(Dest);
	PopCommand(self);
};