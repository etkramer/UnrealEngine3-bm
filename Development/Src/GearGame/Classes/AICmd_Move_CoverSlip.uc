/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Move_CoverSlip extends AICommand_SpecialMove
	within GearAI_Cover;

/** GoW global macros */

/** Start and End markers for the slip */
var CoverSlotMarker		SlipStart;
var NavigationPoint		SlipEnd;

/** used in FakeCoverSlip code */
var vector SidewaysDest;

var byte Direction;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool CoverSlip( GearAI_Cover AI, CoverSlotMarker Start, NavigationPoint End )
{
	local AICmd_Move_CoverSlip Cmd;
	local CoverSlipReachSpec Spec;

	Spec = CoverSlipReachSpec(Start.GetReachSpecTo(End));
	if( AI		!= None &&
		Spec	!= None &&
		Start	!= None &&
		End		!= None )
	{
		Cmd = new(AI) class'AICmd_Move_CoverSlip';
		if( Cmd != None )
		{
			Cmd.SlipStart	= Start;
			Cmd.SlipEnd		= End;
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
	return Super.GetDumpString()@"Start:"@SlipStart@"End:"@SlipEnd;
}

/** Should the AI run to cover before performing the slip? */
final private function bool ShouldRun2Cover()
{
	if( CoverOwner == None )
	{
		return FALSE;
	}

	//debug
	`AILog( GetFuncName()@AIOwner@MyGearPawn.CoverType@MyGearPawn.CurrentLink@SlipStart.OwningSlot.Link@SlipStart@MyGearPawn.Anchor );

	return (MyGearPawn.CoverType == CT_None || MyGearPawn.CurrentLink != SlipStart.OwningSlot.Link);
}

final private function bool ShouldChangeMirrorDirection()
{
	//debug
	`AILog( GetFuncName()@Direction@MyGearPawn.bIsMirrored@MyGearPawn.bWantsToBeMirrored );

	if( (Direction == CD_Right &&  MyGearPawn.bIsMirrored) ||
		(Direction == CD_Left  && !MyGearPawn.bIsMirrored) )
	{
		return TRUE;
	}

	return FALSE;
}

/** checks if AI isn't in the correct slot to do the cover slip */
final private function bool ShouldAdjustToSlot()
{
	return MyGearPawn != None && MyGearPawn.CurrentLink != None && (MyGearPawn.bIsMirrored ? !MyGearPawn.CurrentLink.IsLeftEdgeSlot(MyGearPawn.CurrentSlotIdx, true) : !MyGearPawn.CurrentLink.IsRightEdgeSlot(MyGearPawn.CurrentSlotIdx, true));
}

function Pushed()
{
	Super.Pushed();

	// if we don't support the special move or can't execute it, simulate through normal movement
	if (!bCanUseCoverSlipMove || !MyGearPawn.IsInCover() || !MyGearPawn.bCanCoverSlip || MyGearPawn.IsCarryingAHeavyWeapon())
	{
		GotoState('SimulateCoverSlipMovement');
	}
	// If AI should slide into cover
	else if( ShouldRun2Cover() )
	{
		//debug
		`AILog("Running to cover before cover slipping"@SlipStart.OwningSlot.Link@SlipStart.OwningSlot.SlotIdx);

		// Do run2cover first
		class'AICmd_Move_Run2Cover'.static.Run2Cover( CoverOwner, SlipStart );
	}
	else if( ShouldChangeMirrorDirection() )
	{
		//debug
		`AILog( "Update mirror direction first" );

		GotoState( 'ChangeMirrorDirection' );
	}
	else if (ShouldAdjustToSlot())
	{
		`AILog("Not in correct slot, adjusting");
		GotoState('AdjustToCorrectSlot');
	}
	else
	{
		// Otherwise, go right into the slip
		GotoState( 'Command_SpecialMove' );
	}
}

function Popped()
{
	Super.Popped();

	if (MyGearPawn != None)
	{
		MyGearPawn.CurrentSlotDirection = CD_Default;
		MyGearPawn.Velocity = vect(0,0,0);
	}
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	// Resumed slip command after sliding into cover
	// If successfully got into cover
	if( ChildStatus == 'Success' )
	{
		if( ShouldChangeMirrorDirection() )
		{
			//debug
			`AILog( "Update mirror direction first" );

			GotoState( 'ChangeMirrorDirection' );
		}
		else if (ShouldAdjustToSlot())
		{
			`AILog("Not in correct slot, adjusting");
			GotoState('AdjustToCorrectSlot');
		}
		else
		{
			// Do the slip
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
		return SM_CoverSlip;
	}
}

function NavigationPoint GetStartAnchor()
{
	return SlipStart;
}

function NavigationPoint GetUpdatedAnchor()
{
	return SlipEnd;
}

state ChangeMirrorDirection
{
	function FinishedMirroringChange()
	{
		if (ShouldAdjustToSlot())
		{
			`AILog("Not in correct slot, adjusting");
			GotoState('AdjustToCorrectSlot');
		}
		else
		{
			// Do the slip
			GotoState( 'Command_SpecialMove' );
		}
	}
}

state AdjustToCorrectSlot
{
	event EndState(name NextStateName)
	{
		if (MyGearPawn != None)
		{
			MyGearPawn.CurrentSlotDirection = CD_Default;
			MyGearPawn.Velocity = vect(0,0,0);
		}
	}

	final private function SetDesiredSlot()
	{
		local int i;

		if (MyGearPawn.bIsMirrored)
		{
			for (i = 0; i < MyGearPawn.CurrentLink.Slots.length; i++)
			{
				if (MyGearPawn.CurrentLink.IsLeftEdgeSlot(i, true))
				{
					MyGearPawn.TargetSlotMarker = MyGearPawn.CurrentLink.Slots[i].SlotMarker;
				}
			}

		}
		else
		{
			for (i = 0; i < MyGearPawn.CurrentLink.Slots.length; i++)
			{
				if (MyGearPawn.CurrentLink.IsRightEdgeSlot(i, true))
				{
					MyGearPawn.TargetSlotMarker = MyGearPawn.CurrentLink.Slots[i].SlotMarker;
				}
			}
		}
	}

Begin:
	SetDesiredSlot();
	AdjustToSlotTime = 0.0;
	MyGearPawn.CurrentSlotDirection = (MyGearPawn.bIsMirrored) ? CD_Left : CD_Right;
	do
	{
		if (AdjustToSlotTime > TIMER_ADJUSTTOSLOT_LIMIT)
		{
			`AILog("Failed to adjust to the required slot (timed out)");
			Status = 'Failure';
			PopCommand(self);
		}
		Sleep(0.1);
		AdjustToSlotTime += 0.1;
	} until (!ShouldAdjustToSlot());

	`AILog("Finished adjusting");
	GotoState('Command_SpecialMove');
}

/** fakes a cover slip by just moving the correct amounts so we can use CoverSlipReachSpecs even when
 * the special move wouldn't work
 */
state SimulateCoverSlipMovement
{
	function Resumed(name OldCommandName)
	{
		Super(AICommand).Resumed(OldCommandName);
	}

	final function vector GetSidewaysDest()
	{
		local vector X, Y, Z, Dest;

		GetAxes(SlipStart.GetSlotRotation(), X, Y, Z);
		if (Direction == CD_Left)
		{
			Y *= -1.0;
		}
		PointDistToLine(SlipEnd.Location, Y, Pawn.Location, Dest);
		Dest.Z = Pawn.Location.Z; //@warning: assumes minimal slope
		return Dest;
	}
Begin:
	SidewaysDest = GetSidewaysDest();
	ResetCoverType();
	MoveTo(SidewaysDest);
	Pawn.SetAnchor(None); // so we don't try to recurse into SpecialMoveTo()
	MoveToward(SlipEnd);
	Status = Pawn.ReachedDestination(SlipEnd) ? 'Success' : 'Failure';
	PopCommand(self);
}

defaultproperties
{
	bUpdateStartAnchor=TRUE
	bUpdateAnchorOnSuccess=TRUE
	bShouldCheckSpecialMove=true
}
