/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_PeekFromCover extends AICommand
	within GearAI_Cover;

/** GoW global macros */

/** AI is looking for the enemy */
var bool bLookingForEnemy;
var bool bSpottedEnemy;
var float PeekTime;

var float FinishedPeekingTime;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool PeekFromCover( GearAI_Cover AI, optional bool inbLookingForEnemy = TRUE, optional float InPeekTime )
{
	local AICmd_Attack_PeekFromCover Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_Attack_PeekFromCover';
		if( Cmd != None )
		{
			Cmd.bLookingForEnemy = inbLookingForEnemy;
			Cmd.PeekTime = InPeekTime;
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();

	bFailedToFireFromCover = FALSE;

	GotoState( 'Peeking' );
}

function Popped()
{
	Super.Popped();

	bFailedToFireFromCover = (Status != 'Success');
	NextAllowPeekTime = WorldInfo.TimeSeconds + 5.f + FRand() * 10.f;

	if( bLookingForEnemy && !bSpottedEnemy )
	{
		// Clear cover action
		ResetCoverAction();
	}
}

function bool IsFinishedPeeking()
{
	if( WorldInfo.TimeSeconds > FinishedPeekingTime )
	{
		return TRUE;
	}
	if( bSpottedEnemy )
	{
		return TRUE;
	}
	if( ShouldMoveToSquad() )
	{
		return TRUE;
	}

	return FALSE;
}

function bool NotifyEnemyBecameVisible( Pawn VisibleEnemy )
{
	if( bLookingForEnemy )
	{
		//debug
		`AILog( "Spotted enemy while peeking"@VisibleEnemy );

		SetEnemy( VisibleEnemy );
		bSpottedEnemy = TRUE;
		return TRUE;
	}
	return Super.NotifyEnemyBecameVisible( VisibleEnemy );
}

state Peeking
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@bTurtle@IsReloading()@IsSwitchingWeapons()@IsUsingSuppressiveWeapon(), 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();

	if( GetPeekCoverAction() )
	{
		// Wait for any transitions to finish before
		// transitioning again
		LastAnimTransitionTime = WorldInfo.TimeSeconds;
		while( IsTransitioning() )
		{
			Sleep( 0.25f );
		}

		// If should wait for transition (this function will mirror pawn if needed)
		if( ShouldWaitForTransition( PendingFireLinkItem.SrcAction ) )
		{
			//debug
			`AILog( "Wait for mirror transition", 'Combat' );

			// While still transitioning, wait
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );
		}
		// Wait for action anim
		SetCoverAction( PendingFireLinkItem.SrcAction );

		// Sleep until done doing action
		LastAnimTransitionTime = WorldInfo.TimeSeconds;
		do
		{
			Sleep( 0.25f );
		} until( !IsTransitioning() );

		FinishedPeekingTime = Max(0.5f + (FRand() * 5.f),PeekTime);

		// Sleep until we need to move again
		// Or we have a chance to put our head back down
		do
		{
			//debug
			`AILog( "In peek loop", 'Loop' );

			Sleep( 0.1f );
		} until( IsFinishedPeeking() );

		if( !bSpottedEnemy )
		{
			// return back to cover
			ResetCoverAction();

			// Sleep until done resetting action
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );
		}

		Status = 'Success';
		PopCommand( self );
	}

	GotoState( 'DelayFailure' );
}

defaultproperties
{
}

