class AICmd_Attack_SireMelee extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

	InvalidateCover();
	DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	bWantsLedgeCheck = TRUE;
	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	// Allow normal rotation again
	bForceDesiredRotation = FALSE;
}


state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return GSM_Sire_MeleeHeadGrab;
	}

	function bool ShouldFinishRotation() { return TRUE; }
}

defaultproperties
{
}