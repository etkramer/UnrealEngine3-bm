/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICommand_SpecialMove extends AICommand
	within GearAI;

/** GoW global macros */

/** Special move this command is monitoring */
var ESpecialMove SpecialMove;
/** Time after special move is complete that command should terminate */
var float		 TerminationTime;

/** whether we should check if the special move is valid (CanDoSpecialMove()) */
var bool bShouldCheckSpecialMove;
/** Should update Pawn's anchor when starting a special move */
var bool		 bUpdateStartAnchor;
/** Should update Pawn's anchor when popping after a successful special move */
var bool		 bUpdateAnchorOnSuccess;
/** Time in seconds we should wait before considering this special move as timed out **/
var float		 TimeOutDelaySeconds;

function Pushed()
{
	Super.Pushed();

	LockdownAI();
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	LockdownAI();
}

function Paused( AICommand NewCommand )
{
	Super.Paused( NewCommand );

	UnlockAI();
}

function Popped()
{
	local int Idx;
	local NavigationPoint NewAnchor;

	Super.Popped();

	UnlockAI();
	ClearTimer( 'SpecialMoveTimeout', self );

	if( bUpdateAnchorOnSuccess )
	{
		// If successful
		if( Status == 'Success' )
		{
			NewAnchor = GetUpdatedAnchor();
			if( NewAnchor != None )
			{
				// Update pawn's anchor
				Pawn.SetAnchor( NewAnchor );

				// Remove anything in the route cache up to our new movetarget
				Idx = RouteCache.Find( NewAnchor );
				if( Idx != INDEX_NONE )
				{
					RouteCache_RemoveIndex( 0, Idx + 1 );
				}
			}
		}
	}
}

function NavigationPoint GetStartAnchor()
{
	return None;
}

function NavigationPoint GetUpdatedAnchor()
{
	return None;
}

function LockdownAI()
{
	//debug
	`AILog( GetFuncName() );

	bReachedCover			= FALSE;
	bPreparingMove			= TRUE;	// Don't move until move done
	bForceDesiredRotation	= TRUE;	// Fix pawn rotation and set the rotation

	if( MyGearPawn != None )
	{
		// Force pawn rotation dest to ours and kill movement
		MyGearPawn.DesiredRotation	= DesiredRotation;
		Pawn.ZeroMovementVariables();

		MyGearPawn.StopMeleeAttack();
	}
}

function UnlockAI()
{
	//debug
	`AILog( GetFuncName() );

	// Turn off flags
	bPreparingMove			= FALSE;
	bPreciseDestination		= FALSE;
	bForceDesiredRotation	= FALSE;
	bReachedCover			= FALSE;
}

// Stub function
function SpecialMoveTimeout();



state Command_SpecialMove `DEBUGSTATE
{
	function BeginState( Name PreviousStateName )
	{
		if( bUpdateStartAnchor )
		{
			Pawn.SetAnchor( GetStartAnchor() );
		}
	}

	function bool AllowPushOfDefaultCommandForSpecialMove(ESpecialMove SM)
	{
		// if we're handling this special move, then NO!
		if(SM == GetSpecialMove())
		{
			return FALSE;
		}

		return Super.AllowPushOfDefaultCommandForSpecialMove(SM);
	}

	function bool ShouldFinishRotation();
	function bool ShouldFinishPostRotation();

	/** @return the Pawn the special move should interact with (if any) */
	function GearPawn	GetInteractionPawn();
	function INT		GetSpecialMoveFlags(ESpecialMove InSpecialMove);

	function bool ExecuteSpecialMove()
	{
		SpecialMove = GetSpecialMove();

		//debug
		`AILog( GetFuncName()@SpecialMove );

		if( SpecialMove != SM_None &&
			(!bShouldCheckSpecialMove || MyGearPawn.CanDoSpecialMove( SpecialMove )) )
		{
			MyGearPawn.DoSpecialMove(SpecialMove, TRUE, GetInteractionPawn(), GetSpecialMoveFlags(SpecialMove));
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	function bool IsSpecialMoveComplete()
	{
		if( !bPreparingMove || MyGearPawn == None || MyGearPawn.SpecialMove == SM_None )
		{
			return TRUE;
		}
		return FALSE;
	}

	function SpecialMoveTimeout()
	{
		//debug
		`AILog( "Special move timed out" );

		if( MyGearPawn.SpecialMove == SpecialMove )
		{
			MyGearPawn.EndSpecialMove();
		}

		Status = 'Failure';
		AbortCommand( self );
	}

	// Overridden in child states
	function bool SetupSpecialMove() { return TRUE; }
	function FinishedSpecialMove();
	function ESpecialMove GetSpecialMove();
	function float GetPostSpecialMoveSleepTime();
	function bool IsReady() { return TRUE; }

Begin:
	//debug
	`AILog( "--"@GetStateName()$":"$class@"-- BEGIN TAG", 'State' );

	if( !SetupSpecialMove() )
	{
		//debug
		`AILog( "Setup Special Move failed" );

		Goto( 'Abort' );
	}

	if (ShouldFinishRotation() && !Pawn.ReachedDesiredRotation())
	{
		FinishRotation();
	}

	while( !IsReady() )
	{
		//debug
		`AILog( "Waiting for ready", 'Loop' );

		Sleep(0.1f);
	}

	if( ExecuteSpecialMove() )
	{
		SetTimer( TimeOutDelaySeconds, FALSE, nameof(self.SpecialMoveTimeOut), self );
		do
		{
			//debug
			`AILog( "Waiting for SM to end", 'Loop' );

			Sleep( 0.1f );
		} until( IsSpecialMoveComplete() );

		//debug
		`AILog( "Special move ended" );

		if( ShouldFinishPostRotation() )
		{
			FinishRotation();
		}

		FinishedSpecialMove();

		TerminationTime = WorldInfo.TimeSeconds + GetPostSpecialMoveSleepTime();
		while( TerminationTime > WorldInfo.TimeSeconds )
		{
			//debug
			`AILog( "Waiting for SM delay to end", 'Loop' );

			Sleep( 0.1f );
		}

		Status = 'Success';
	}
	else
	{
Abort:
		//debug
		`AILog( "Failed to do special move" );

		Status = 'Failure';
		Sleep(0.5f);
	}

	PopCommand( self );
}

state ChangeMirrorDirection `DEBUGSTATE
{
	function FinishedMirroringChange()
	{
		GotoState('Command_SpecialMove');
	}
Begin:
	//debug
	`AILog( "Updating Mirror Direction"@MyGearPawn.bIsMirrored@"to"@!MyGearPawn.bIsMirrored@"trans?"@IsTransitioning() );

	MyGearPawn.SetMirroredSide( !MyGearPawn.bIsMirrored );
	LastAnimTransitionTime = WorldInfo.TimeSeconds;
	do
	{
		Sleep( 0.1f );
	} until( !IsTransitioning() );

	FinishedMirroringChange();
}

defaultproperties
{
	bAllowedToFireWeapon=FALSE
	TimeOutDelaySeconds=10.0f
}
