/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Move_Mantle extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

/** Start and End markers for the mantle over */
var CoverSlotMarker		MantleStart, MantleEnd;
/** Start and End markers for climb up/down */
var MantleMarker		ClimbStart,  ClimbEnd;
var bool bUsingRun2Cover;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Mantle( GearAI AI, NavigationPoint Start, NavigationPoint End )
{
	local AICmd_Move_Mantle Cmd;

	if (AI != None && Start != None && End != None && AI.MyGearPawn != None && !AI.MyGearPawn.IsAKidnapper())
	{
		Cmd = new(AI) class'AICmd_Move_Mantle';
		Cmd.MantleStart = CoverSlotMarker(Start);
		Cmd.MantleEnd	= CoverSlotMarker(End);
		Cmd.ClimbStart  = MantleMarker(Start);
		Cmd.ClimbEnd	= MantleMarker(End);

		AI.PushCommand( Cmd );
		return TRUE;
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Start:"@MantleStart@ClimbStart@"End:"@MantleEnd@ClimbEnd;
}

/** Should the AI run to cover before performing the mantle? */
final private function bool ShouldRun2Cover()
{
	if( CoverOwner == None || MantleStart == None )
	{
		return FALSE;
	}

	//debug
	`AILog( GetFuncName()@AIOwner@MyGearPawn.CoverType@MyGearPawn.CurrentLink@MantleStart.OwningSlot.Link@MantleStart@MyGearPawn.Anchor );

	return (MyGearPawn.CoverType == CT_None || MyGearPawn.CurrentLink != MantleStart.OwningSlot.Link);
}

function Pushed()
{
	Super.Pushed();

	// If AI should slide into cover
	if( ShouldRun2Cover() )
	{
		//debug
		`AILog("Running to cover before mantling"@MantleStart.OwningSlot.Link@MantleStart.OwningSlot.SlotIdx);

		bUsingRun2Cover=true;
		// Do run2cover first
		class'AICmd_Move_Run2Cover'.static.Run2Cover( CoverOwner, MantleStart );
	}
	else
	{
		// Otherwise, go right into the mantle
		GotoState( 'Command_SpecialMove' );
	}
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	// Resumed mantle command after sliding into cover
	// If successfully got into cover
	if( ChildStatus == 'Success' )
	{
		// Do the mantle
		GotoState( 'Command_SpecialMove' );
	}
	else if(bUsingRun2Cover && MantleStart != none && CoverOwner != none)
	{
		// if we tried run2cover and it didn't work, walk there
		bUsingRun2Cover=false;
		CoverOwner.SetCoverGoal(MantleStart.OwningSlot,false);
	}
	else
	{
		//otherwise, failure :(
		Status = 'Failure';
		PopCommand( self );
	}
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		// Face mantle target slot
		if( MantleEnd != None )
		{
			DesiredRotation = rotator(MantleEnd.Location - Pawn.Location);
		}
		else
		{
			DesiredRotation = rotator(ClimbEnd.Location - Pawn.Location);
		}

		bIgnoreNotifies = true; // don't interrupt mantles, we might get stuck!
	}

	function ESpecialMove GetSpecialMove()
	{
		// Climb Up
		if( MantleStart != None && ClimbEnd != None )
		{
			return SM_MantleUpLowCover;
		}
		else
		// Climb Down
		if( MantleEnd != None && ClimbStart != None )
		{
			return SM_MantleDown;
		}

		// Default mantle over
		return SM_MidLvlJumpOver;
	}

	function INT GetSpecialMoveFlags(ESpecialMove InSpecialMove)
	{
		local GearSpecialMove USpecialMove;

		if( InSpecialMove == SM_MidLvlJumpOver )
		{
			MyGearPawn.VerifySMHasBeenInstanced(InSpecialMove);
			USpecialMove = MyGearPawn.SpecialMoves[InSpecialMove];
			if( USpecialMove.IsA('GSM_MidLvlJumpOver') )
			{
				return GSM_MidLvlJumpOver(USpecialMove).PackSpecialMoveFlags();
			}
		}
		return 0;
	}

	// Make sure AI is facing the correct way
	function bool ShouldFinishRotation()	{ return TRUE; }
}

function NavigationPoint GetUpdatedAnchor()
{
	if( MantleEnd != None )
	{
		return MantleEnd;
	}
	return ClimbEnd;
}

function NavigationPoint GetStartAnchor()
{
	// Make sure our anchor is set properly
	if( MantleStart != None )
	{
		return MantleStart;
	}
	return ClimbStart;
}

defaultproperties
{
	bUpdateStartAnchor=TRUE
	bUpdateAnchorOnSuccess=TRUE
	bShouldCheckSpecialMove=TRUE
}
