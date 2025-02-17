/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AICmd_Attack_Brumak_Rockets extends AICommand_SpecialMove
	within GearAI_Brumak;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

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
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		if( bHijackedByLD && Focus != None )
		{
			DesiredRotation = Rotator(Focus.Location - Pawn.Location);
		}
		else
		{
			DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
		}
	}

	function bool ShouldFinishRotation()
	{
		return TRUE;
	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_Brumak_CannonFire;
	}

	function bool SetupSpecialMove()
	{
		// Perfect knowledge of enemy positions
		ForceUpdateOfEnemies();

		if( DriverAI != None )
		{
			// Set FireTarget, so Brumak AimOffset points to that target.
			FireTarget = DriverAI.GetFireTarget();
			
			// Reset attack timer
			if( LeftGunAI  != None ) { LeftGunAI.GotoState( 'Silent' );		}
			if( RightGunAI != None ) { RightGunAI.GotoState( 'Silent' );	}
			if( DriverAI   != None ) 
			{ 
				DriverAI.StopFiring();
				DriverAI.bAbleToFire = FALSE;
			}

			return TRUE;
		}

		return FALSE;
	}

	function bool ExecuteSpecialMove()
	{
		if( DriverAI != None )
		{
			DriverAI.GotoState( 'RocketAttack' );
		}

		return Super.ExecuteSpecialMove();
	}

	function bool IsSpecialMoveComplete()
	{
		if( DriverAI == None || DriverAI.Pawn == None )
		{
			return TRUE;
		}

		if( !DriverAI.IsInState('RocketAttack') && !Brumak.IsDoingSpecialMove(GSM_Brumak_CannonFire) )
		{
			return TRUE;
		}

	   return FALSE;
	}

	function FinishedSpecialMove()
	{
		if( LeftGunAI  != None ) { LeftGunAI.GotoState( 'Auto' );	}
		if( RightGunAI != None ) { RightGunAI.GotoState( 'Auto' );	}
		if( DriverAI   != None ) { DriverAI.GotoState( 'Auto' );	}
	}
}

defaultproperties
{
	bShouldCheckSpecialMove=true
}
