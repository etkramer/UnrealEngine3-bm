class AICmd_React_Brumak_ExposeDriver extends AICommand_SpecialMove
	within GearAI_Brumak;

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}
	
state Command_SpecialMove
{
	function bool SetupSpecialMove()
	{
		if( LeftGunAI  != None ) { LeftGunAI.GotoState( 'Silent' );	 }
		if( RightGunAI != None ) { RightGunAI.GotoState( 'Silent' ); }
		if( DriverAI   != None ) { DriverAI.GotoState( 'Silent' );	 }

		return TRUE;
	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_Brumak_ExposeDriver;
	}

	function FinishedSpecialMove()
	{
		if( LeftGunAI  != None ) { LeftGunAI.GotoState( 'Auto' );	}
		if( RightGunAI != None ) { RightGunAI.GotoState( 'Auto' );	}
		if( DriverAI   != None ) { DriverAI.GotoState( 'Auto' );	}

		// Check combat transitions
		CheckCombatTransition();
	}
}

defaultproperties
{
}