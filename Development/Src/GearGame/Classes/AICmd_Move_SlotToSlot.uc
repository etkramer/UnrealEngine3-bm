/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Move_SlotToSlot extends AICommand
	within GearAI_Cover;

/** GoW global macros */

/** Start and End markers for the adjustment in cover */
var CoverSlotMarker		SlotStart, SlotEnd;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool SlotToSlot( GearAI_Cover AI, CoverSlotMarker Start, CoverSlotMarker End )
{
	local AICmd_Move_SlotToSlot Cmd;

	if( AI		!= None && 
		Start	!= None &&
		End		!= None )
	{
		Cmd = new(AI) class'AICmd_Move_SlotToSlot';
		if( Cmd != None )
		{
			Cmd.SlotStart = Start;
			Cmd.SlotEnd	= End;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Start:"@SlotStart@"End:"@SlotEnd;
}

function Pushed()
{
	Super.Pushed();

	GotoState( 'AdjustingToSlot' );
}

state AdjustingToSlot
{
	function SetupCoverInfo()
	{
		local CovPosInfo	CoverPosInfo;

		// if the indices are the 
		if(MyGearpawn.LeftSlotIdx == MyGearPawn.RightSlotidx)
		{
			CoverPosInfo = MyGearPawn.CoverInfoToCovPosInfo(SlotStart.OwningSlot);

			if(SlotStart.OwningSlot.SlotIdx < SlotEnd.OwningSlot.SlotIdx)
			{
				CoverPosInfo.LtToRtPct = 0;
				CoverPosInfo.LtSlotIdx = SlotStart.OwningSlot.SlotIdx;
				CoverPosInfo.RtSlotIdx = SlotEnd.OwningSlot.SlotIdx;
			}
			else
			{
				CoverPosInfo.LtToRtPct = 1;
				CoverPosInfo.LtSlotIdx = SlotEnd.OwningSlot.SlotIdx;
				CoverPosInfo.RtSlotIdx = SlotStart.OwningSlot.SlotIdx;
			}
			`AILog(GetFuncName()@"WARNING! Adjusting cover indices because we're trying to slot-to-slot and both left and right indices are the same!"@CoverPosInfo.LtSlotIdx@CoverPosInfo.RtSlotIdx@CoverPosInfo.LtToRtPct);

			MyGearPawn.CoverAcquired( CoverPosInfo );
		}

		ResetCoverAction();
	}
Begin:
	//debug
	`AILog( "Adjust to slot..."@SlotStart.OwningSlot.Link.GetDebugString(SlotStart.OwningSlot.SlotIdx)@SlotEnd.OwningSlot.Link.GetDebugString(SlotEnd.OwningSlot.SlotIdx), 'Move' );

	SetupCoverInfo();
	// Latent adjust to cover - will return when pawn reaches given slot or move times out
	AdjustToSlot( SlotEnd );

	// Reached the target slot successfully
	if( bReachedCover )
	{
		// Set the current cover info
		MyGearPawn.CurrentLink	  = SlotEnd.OwningSlot.Link;
		MyGearPawn.CurrentSlotIdx = SlotEnd.OwningSlot.SlotIdx;

		// Return success
		Status = 'Success';
		PopCommand( self );
	}
	else
	{
		// Otherwise, failed to reach the target slot
		Status = 'Failure';
		PopCommand( self );
	}
}

defaultproperties
{
}