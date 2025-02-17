class AICmd_React_Brumak_Roar extends AICommand_SpecialMove
	within GearAI_Brumak;

var bool bFinishRotation;

function Pushed()
{
	Super.Pushed();

	if( Focus != None )
	{
		DesiredRotation = rotator(Focus.Location - Pawn.Location);
		bFinishRotation = TRUE;
	}	

	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();

	if( bHijackedByLD )
	{
		ClearLatentAction( class'SeqAct_BrumakControl', (Status!='Success') );
	}	
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
		return GSM_Brumak_ROAR;
	}

	function FinishedSpecialMove()
	{
		if( LeftGunAI  != None ) { LeftGunAI.GotoState( 'Auto' );	}
		if( RightGunAI != None ) { RightGunAI.GotoState( 'Auto' );	}
		if( DriverAI   != None ) { DriverAI.GotoState( 'Auto' );	}

		// Check combat transitions
		CheckCombatTransition();
	}

	function bool ShouldFinishRotation() 
	{ 
		return bFinishRotation; 
	}
}

defaultproperties
{
}