/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Move_Leap extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

/** Start and End for the leap */
var NavigationPoint	LeapStart, LeapEnd;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Leap( GearAI AI, NavigationPoint Start, NavigationPoint End )
{
	local AICmd_Move_Leap Cmd;

	if( AI		!= None && 
		Start	!= None &&
		End		!= None )
	{
		Cmd = new(AI) class'AICmd_Move_Leap';
		if( Cmd != None )
		{
			Cmd.LeapStart	= Start;
			Cmd.LeapEnd		= End;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Start:"@LeapStart@"End:"@LeapEnd;
}

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		DesiredRotation = rotator(LeapEnd.Location - Pawn.Location);
	}

	function bool DoLeap()
	{
		local Vector JumpVel;
		local LeapReachSpec Spec;

		Spec = LeapReachSpec(LeapStart.GetReachSpecTo( LeapEnd ));
		if( Spec != None )
		{
			JumpVel = Spec.CachedVelocity;
			if( LeapEnd.Location.Z > LeapStart.Location.Z )
			{
				JumpVel.Z *= 1.07f;
			}

			//debug
			`AILog( GetFuncName()@Spec@Spec.CachedVelocity@"--"@JumpVel );

			Pawn.Velocity = JumpVel;
			Pawn.SetPhysics( PHYS_Falling );
			return TRUE;
		}
		else
		// should be able to use cached value in reach spec
		if( Pawn.SuggestJumpVelocity( JumpVel, LeapEnd.Location, LeapStart.Location ) )
		{
			if( LeapEnd.Location.Z > LeapStart.Location.Z )
			{
				JumpVel.Z *= 1.07f;
			}

			//debug
			`AILog( GetFuncName()@JumpVel );

			Pawn.Velocity = JumpVel;
			Pawn.SetPhysics( PHYS_Falling );
			return TRUE;
		}

		//debug
		`AILog( "Failed to do leap" );

		return FALSE;
	}

Begin:
	
	if( DoLeap() )
	{
		do
		{
			Sleep( 0.25f );
		} until( Pawn.Physics != PHYS_Falling );

		Status = 'Success';
		PopCommand( self );
		Stop;
	}
	
	GotoState( 'DelayFailure' );
}


defaultproperties
{
}