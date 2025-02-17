/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Move_SwatTurn extends AICommand_SpecialMove
	within GearAI_Cover;

/** GoW global macros */

/** Start and End markers for the swat turn */
var CoverSlotMarker		TurnStart, TurnEnd;
var byte Direction;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool SwatTurn( GearAI_Cover AI, CoverSlotMarker Start, coverSlotMarker End )
{
	local AICmd_Move_SwatTurn Cmd;
	local SwatTurnReachSpec Spec;

	Spec = SwatTurnReachSpec(Start.GetReachSpecTo(End));
	if( AI		!= None &&
		Spec	!= None &&
		Start	!= None &&
		End		!= None )
	{
		Cmd = new(AI) class'AICmd_Move_SwatTurn';
		if( Cmd != None )
		{
			Cmd.TurnStart	= Start;
			Cmd.TurnEnd		= End;
			Cmd.Direction	= Spec.SpecDirection;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Start:"@TurnStart@"End:"@TurnEnd;
}

/** Should the AI run to cover before performing the turn? */
final private function bool ShouldRun2Cover()
{
	if( CoverOwner == None )
	{
		return FALSE;
	}

	//debug
	`AILog( GetFuncName()@AIOwner@MyGearPawn.CoverType@MyGearPawn.CurrentLink@TurnStart.OwningSlot.Link@TurnStart@MyGearPawn.Anchor );

	return (MyGearPawn.CoverType == CT_None || MyGearPawn.CurrentLink != TurnStart.OwningSlot.Link);
}

final private function bool ShouldChangeMirrorDirection()
{
	//debug
	`AILog( GetFuncName()@Direction@MyGearPawn.bIsMirrored@MyGearPawn.bWantsToBeMirrored );

	if( (Direction == CD_Right &&  AIOwner.MyGearPawn.bIsMirrored) ||
		(Direction == CD_Left  && !AIOwner.MyGearPawn.bIsMirrored) )
	{
		return TRUE;
	}

	return FALSE;
}


function Pushed()
{
	Super.Pushed();

	// If AI should slide into cover
	if( ShouldRun2Cover() )
	{
		//debug
		`AILog("Running to cover before mantling"@TurnStart.OwningSlot.Link@TurnStart.OwningSlot.SlotIdx);

		// Do run2cover first
		class'AICmd_Move_Run2Cover'.static.Run2Cover( CoverOwner, TurnStart );
	}
	else
	if( ShouldChangeMirrorDirection() )
	{
		//debug
		`AILog( "Update mirror direction first" );

		GotoState( 'ChangeMirrorDirection' );
	}
	else
	{
		// Otherwise, go right into the turn
		GotoState( 'Command_SpecialMove' );
	}
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	// Resumed swatturn command after sliding into cover
	// If successfully got into cover
	if( ChildStatus == 'Success' )
	{
		if( ShouldChangeMirrorDirection() )
		{
			//debug
			`AILog( "Update mirror direction first" );

			GotoState( 'ChangeMirrorDirection' );
		}
		else
		{
			// Do the turn
			GotoState( 'Command_SpecialMove' );
		}
	}
	else
	{
		// Otherwise, failure :(
		Status = 'Failure';
		PopCommand( self );
	}
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return SM_StdLvlSwatTurn;
	}
}

function NavigationPoint GetStartAnchor()
{
	return TurnStart;
}

function NavigationPoint GetUpdatedAnchor()
{
	return TurnEnd;
}

defaultproperties
{
	bUpdateStartAnchor=TRUE
	bUpdateAnchorOnSuccess=TRUE
	bShouldCheckSpecialMove=true
}
