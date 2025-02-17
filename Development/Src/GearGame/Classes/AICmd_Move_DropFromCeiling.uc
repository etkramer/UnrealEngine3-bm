class AICmd_Move_DropFromCeiling extends AICommand_SpecialMove
	within GearAI;

var NavigationPoint JumpStart;
var NavigationPoint JumpEnd;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Jump( GearAI AI, NavigationPoint Start, NavigationPoint End )
{
	local AICmd_Move_DropFromCeiling Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_Move_DropFromCeiling';
		if( Cmd != None )
		{
			Cmd.JumpStart	= Start;
			Cmd.JumpEnd		= End;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Start:"@JumpStart@"End:"@JumpEnd;
}

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}
	
state Command_SpecialMove
{
	// Don't allow transition until AnimNode has had a chance to reset
	function bool IsReady()
	{
		if( MyGearPawn != None &&
			TimeSince( MyGearPawn.LastEvadeTime ) < MyGearPawn.MinTimeBetweenEvades )
		{
			return FALSE;
		}

		return TRUE;
	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_DropFromCeiling;
	}

	function bool IsSpecialMoveComplete()
	{
		return (Physics == PHYS_Walking || Super.IsSpecialMoveComplete());
	}

	function FinishedSpecialMove()
	{
		CheckCombatTransition();
	}
}

function NavigationPoint GetUpdatedAnchor()
{
	if( JumpEnd != None )
	{
		return JumpEnd;
	}
	return None;
}

function NavigationPoint GetStartAnchor()
{
	// Make sure our anchor is set properly
	if( JumpStart != None )
	{
		return JumpStart;
	}
	return Pawn.Anchor;
}

defaultproperties
{
	bUpdateStartAnchor=TRUE
		bUpdateAnchorOnSuccess=TRUE
		bShouldCheckSpecialMove=TRUE
}
