/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_StepAside extends AICommand
	within GearAI;

var BasedPosition PreStepAsideLocation;
var bool bDelayStep;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool StepAside( GearAI AI, Pawn NewStepAsideGoal, bool inbDelayStep )
{
	local AICmd_StepAside Cmd;

	if( AI != None &&
		AI.StepAsideGoal == None &&
		NewStepAsideGoal != None )
	{
		Cmd = new(AI) class'AICmd_StepAside';
		if( Cmd != None )
		{
			AI.StepAsideGoal = NewStepAsideGoal;
			Cmd.bDelayStep	 = inbDelayStep;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Goal:"@StepAsideGoal;
}

function Pushed()
{
	Super.Pushed();

	SetBasedPosition( PreStepAsideLocation, Pawn.Location );

	// If we just need to stand still for a second
	if( bDelayStep )
	{
		GotoState( 'Command_DelayStep' );
	}
	else
	{
		// Otherwise, do the step aside
		GotoState( 'Command_StepAside' );
	}
}

function Popped()
{
	Super.Popped();

	bWantsLedgeCheck = FALSE;
	StepAsideGoal = None;

	if( Pawn != None )
	{
		Pawn.ZeroMovementVariables();
	}

	if(bReevaluatePath)
	{
		outer.NotifyNeedRepath();
	}
}

state Command_StepAside
{
	/**
	 * Returns true if the specified vector is valid for us to move to.
	 */
	function bool IsValidMoveLocation( Vector ChkLoc )
	{
		return PointReachable( ChkLoc );
	}

	/**
	 * Returns a location that Best moves out of the way of StepAsideGoal.
	 */
	function Vector GetStepAsideLocation()
	{
		local Vector X, Y, Z;
		local array<Vector> ChkLocs;
		local int Idx;
		local float StepDist;

		// If stepping aside from self, pick random direction
		if( StepAsideGoal == Pawn )
		{
			X = VRand();
		}
		else
		// first try a direction perpendicular to the target's velocity
		if( VSizeSq(StepAsideGoal.Acceleration) > 0 )
		{
			X	= StepAsideGoal.Acceleration;
		}
		else
		if( bMovingToGoal || bMovingToCover )
		{
			//debug
			`AILog("- IS MOVING"@MoveTarget@MoveGoal@GetBasedPosition(MovePosition), 'Sensory' );

			if( MoveTarget != None )
			{
				X = MoveTarget.Location - Pawn.Location;
			}
 			else
			if( MoveGoal != None )
			{
				X = MoveGoal.Location - Pawn.Location;
			}
			else
			if( GetBasedPosition(MovePosition) != vect(0,0,0) )
			{
				X = GetBasedPosition( MovePosition ) - Pawn.Location;
			}
			else
			{
				//debug
				`AILog( "Moving but to where??" );
			}
		}
		else
		{
			X = Vector(Pawn.Rotation);
		}

		// Ignore vertical component
		X.Z = 0;
		X	= Normal(X);
		Z	= vect(0,0,1);
		Y	= Normal(X CROSS Z);

		if( Normal(Pawn.Location-StepAsideGoal.Location) DOT Y < 0.f )
		{
			// flip the y to avoid running into them more
			Y = -Y;
		}

		StepDist = MaxStepAsideDist;
		if( Pawn.Base != None && ClassIsChildOf( Pawn.Base.Class, class'InterpActor_GearBasePlatform' ) )
		{
			StepDist *= 0.5f;
		}

		ChkLocs.Length = 3;
		// test first dir at Max Distance
		ChkLocs[0] = Pawn.Location + Y * StepDist + X * StepDist * 0.1f;
		// half dir Max Distance
		ChkLocs[1] = Pawn.Location + X * StepDist * 0.5f + Y * StepDist * 0.5f;
		// other dir Max Distance
		ChkLocs[2] = Pawn.Location - Y * StepDist + X * StepDist * 0.1f;
		// half other dir Max Distance
		ChkLocs[3] = Pawn.Location + X * StepDist * 0.5f - Y * StepDist * 0.5f;
		// Forward until there is room to get out
		ChkLocs[4] = Pawn.Location + X * StepDist;

		// return the first valid one
		for( Idx = 0; Idx < ChkLocs.Length; Idx++ )
		{
			if( IsValidMoveLocation( ChkLocs[Idx] ) )
			{
				//debug
				Debug_StepRot = Rotator(X);
				SetBasedPosition( Debug_StepLoc, ChkLocs[Idx] );

				return ChkLocs[Idx];
			}
		}

		return Pawn.Location;
	}

	function Actor StepAside_GetMoveFocus()
	{
		if( FireTarget != None )
		{
			return FireTarget;
		}

		if( bMovingToGoal || bMovingToCover )
		{
			return MoveTarget;
		}

		if( !HasAnyEnemies() )
		{
			return StepAsideGoal;
		}

		return None;
	}

Begin:
	bWantsLedgeCheck = TRUE;
	if( StepAsideGoal != None )
	{
		//debug
		`AILog( "Starting to step aside..." );

		bReevaluatePath = TRUE;
		//`AILog("bReevaluatePath is now TRUE!");

		// get out of cover, as MoveTo() doesn't work while in cover
		//@todo: could check if an adjusttoslot would get us out of the way, but it seems unlikely enough that
		//	it's probably not worth it
		if (CoverOwner != None)
		{
			CoverOwner.ResetCoverType();
		}

		MoveTo( GetStepAsideLocation(), StepAside_GetMoveFocus(), Pawn.bIsWalking );

		// check to see if they are still running into us
		if( StepAsideGoal != None &&
			StepAsideGoal != Pawn &&
			(Pawn.Touching.Find(StepAsideGoal) != -1 || StepAsideGoal.ReachedPoint(GetBasedPosition( PreStepAsideLocation ), None )) )
		{
			//debug
			`AILog( "Still touching, moving again"@Pawn.Touching.Find(StepAsideGoal)@StepAsideGoal.ReachedPoint(GetBasedPosition( PreStepAsideLocation ), None ) );

			// Rest for a sec and let the world change
			// (he may get out of my way)
			Sleep( 0.5f + FRand()*1.f );

			// set the prestepasidelocation to the current location so that we avoid getting stuck
			SetBasedPosition( PreStepAsideLocation, Pawn.Location );

			Goto( 'Begin' );
		}

		Sleep( 0.25f + FRand() * 0.5f );
	}
	else
	{
		//debug
		`AILog( "No step aside goal?" );

		Sleep( 0.5f );
	}

	//debug
	`AILog( "Finished stepping aside" );

	Status = 'Success';
	PopCommand( self );
}

state Command_DelayStep
{
Begin:
	//debug
	`AILog( "Begin delay step aside" );

	if( StepAsideGoal != None )
	{
		if( Pawn != None )
		{
			Pawn.ZeroMovementVariables();
		}
	}

	// Just stop moving for a sec or two
	Sleep( 0.3f + (FRand() * 0.5f) );

	//debug
	`AILog( "Finished delay step aside" );

	Status = 'Success';
	PopCommand( self );
}

event DrawDebug( GearHUD_Base HUD, name Category )
{
	Super.DrawDebug( HUD, Category );

	if(Category == 'Pathing')
	{
		// Debug step asside
		DrawDebugCoordinateSystem( Pawn.Location, Debug_StepRot, 64.f );
		DrawDebugLine( GetBasedPosition( Debug_StepLoc ), Pawn.Location, 255, 255, 255 );
		DrawDebugLine( GetDestinationPosition(), Pawn.Location, 64, 128, 255 );
		DrawDebugBox( GetBasedPosition( PreStepAsideLocation ), vect(5,5,5), 255, 0, 0 );
	}
}

defaultproperties
{
	bReplaceActiveSameClassInstance=true
}

