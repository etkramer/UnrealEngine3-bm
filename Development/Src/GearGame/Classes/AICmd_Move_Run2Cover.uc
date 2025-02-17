/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Move_Run2Cover extends AICommand_SpecialMove
	within GearAI_Cover;

/** GoW global macros */

/** Slot marker AI is running to */
var CoverSlotMarker Run2CoverMarker;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Run2Cover( GearAI_Cover AI, CoverSlotMarker Marker )
{
	local AICmd_Move_Run2Cover Cmd;

	if( AI		!= None &&
		Marker  != None )
	{
		Cmd = new(AI) class'AICmd_Move_Run2Cover';
		if( Cmd != None )
		{
			Cmd.Run2CoverMarker = Marker;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Marker:"@Run2CoverMarker@"CoverInfo:"@Run2CoverMarker.OwningSlot.Link$"/"$Run2CoverMarker.OwningSlot.SlotIdx;
}

function Pushed()
{
	Super.Pushed();

	// don't allow interruptions!
	ReactionManager.SuppressAll();

	// Go right to doing the slide
	GotoState( 'Command_SpecialMove' );
}
function Popped()
{
	Super.Popped();

	// restore
	ReactionManager.UnSuppressAll();
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		// Set rotation toward the slot marker rotation
		DesiredRotation = Run2CoverMarker.GetSlotRotation();

		// stop roadie running if necessary
		if (IsRoadieRunning() || IsCoverRunning())
		{
			DictateSpecialMove(SM_None);
		}
	}

	function bool ShouldFinishPostRotation() { return TRUE; }

	function bool ExecuteSpecialMove()
	{
		local CovPosInfo	CoverPosInfo;

		//debug
		`AILog( GetFuncName()@Run2CoverMarker );

		// clear the run2cover flag
		bShouldRun2Cover = FALSE;
		// note the last time we tried, so if there's an error we don't get stuck trying to run2cover
		LastRun2CoverTime = WorldInfo.TimeSeconds;

		//debug
		`AILog( "Attempting to run2cover:"@Run2CoverMarker.OwningSlot.Link.GetDebugString(Run2CoverMarker.OwningSlot.SlotIdx) );

		if( Run2CoverMarker != None &&
			Run2CoverMarker.OwningSlot.Link != None &&
			Run2CoverMarker.OwningSlot.Link.IsValidClaim( MyGearPawn, Run2CoverMarker.OwningSlot.SlotIdx ) )
		{
			// Acquire cover, this will take care of playing the run2cover acquisition animation
			// And triggering the special move.
			CoverPosInfo = MyGearPawn.CoverInfoToCovPosInfo(Run2CoverMarker.OwningSlot);
			MyGearPawn.CoverAcquired( CoverPosInfo );

			return TRUE;
		}
		else
		{
			//debug
			`AILog("- failed to run2cover, invalid marker/claim Link disabled?"@Run2CoverMarker.OwningSlot.Link.bDisabled@"Slot enabled?"@Run2CoverMarker.OwningSlot.Link.Slots[Run2CoverMarker.OwningSlot.SlotIdx].bEnabled);

			return FALSE;
		}
	}

	function FinishedSpecialMove()
	{
		//debug
		`AILog( "Finished run2cover"@MyGearPawn.CurrentLink@Cover.Link@MyGearPawn.CurrentSlotIdx@Cover.SlotIdx );

		if( MyGearPawn != None &&
			(MyGearPawn.CurrentLink		!= Cover.Link ||
			 MyGearPawn.CurrentSlotIdx	!= Cover.SlotIdx) )
		{
			//debug
			`AILog( "Release cover claim after run2cover" );

			// Let go of the cover after sliding in if this isn't our final cover destination
			MyGearPawn.CurrentLink.Unclaim( MyGearPawn, MyGearPawn.CurrentSlotIdx, TRUE );
		}
	}
}

function NavigationPoint GetUpdatedAnchor()
{
	return Run2CoverMarker;
}

defaultproperties
{
	bUpdateAnchorOnSuccess=TRUE
}
