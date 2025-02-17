/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICommand extends Object within GearAI
    native
    config(AI)
	DependsOn(GearPawn);

/** GoW global macros */

cpptext
{
    virtual void TickCommand(FLOAT DeltaTime);
    void ProcessState(FLOAT DeltaSeconds);
    virtual EGotoState GotoState( FName State, UBOOL bForceEvents = 0, UBOOL bKeepStack = 0 );
    void PopChildCommand();
}

/** Current child node executing on top of this command */
var const AICommand ChildCommand;

/** Exiting status of the last child command to execute */
var const Name ChildStatus;

/** Extra reference to the AI this command is being used by */
var const GearAI		AIOwner;
var const GearAI_Cover	CoverOwner;

/** Exiting status of this command */
var Name Status;

/** if this is FALSE and we're trying to push a new instance of a given class, but the top of the stack is already an instance of that class ignore the attempted push */
var bool bAllowNewSameClassInstance;

/** if this is TRUE, when we try to push a new instance of a command who has the same class as the command on the top of the stack, pop the one on the stack, and push the new one
	 NOTE: This trumps bAllowNewClassInstance (e.g. if this is true and bAllowNewClassInstance is false the active instance will still be replaced) */
var bool bReplaceActiveSameClassInstance;

/** whether this command is one in which the AI fires its weapon without moving */
var const bool bIsStationaryFiringCommand;

/** Is the AI allowed to fire their weapon currently? */
var protected bool bAllowedToFireWeapon;

/** Command has been aborted and should be popped next frame */
var private	bool bAborted;

var bool bIgnoreNotifies;

/** this command is about to be popped, and shouldn't have resumed called on it when children are popped */
var private bool bPendingPop;

/** Simple constructor that takes one extra userdata param **/
static function bool InitCommandUserActor( GearAI AI, Actor UserActor )
{
	//default is to call the constructor
	return InitCommand(AI);
}
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool InitCommand( GearAI AI )
{
	local AICommand Cmd;

	if( AI != None )
	{
		Cmd = new(AI) Default.Class;
		if( Cmd != None )
		{
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}


/** == INTERNAL INTERFACE == */

/** Called when command pushed to perform any necessary work, independent of the individual command implementations - @see Pushed() instead */
final event InternalPushed()
{
	GotoState('Auto');
	// call the overridable notification
	Pushed();
}

/** Called when command popped to perform any necessary cleanup, independent of the individual command implementations - @see Popped() instead */
final event InternalPopped()
{
	// call the overridable notification
	Popped();
}

/** Called when another command is pushed on top of this one */
final event InternalPaused( AICommand NewCommand )
{
	Paused( NewCommand );
}

/** Called when the command that was on top of this one in the stack is popped */
final event InternalResumed( Name OldCommandName )
{
	Resumed( OldCommandName );
}

final event InternalTick( float DeltaTime )
{
	Tick( DeltaTime );
}

/** Checks to see if we're allowed to fire our weapon in this command, passing through to children until blocked. */
function bool IsAllowedToFireWeapon()
{
	return (bAllowedToFireWeapon && (ChildCommand == None || ChildCommand.IsAllowedToFireWeapon()));
}

function bool ShouldSelectTarget()
{
	if( ChildCommand != None )
	{
		return ChildCommand.ShouldSelectTarget();
	}
	return TRUE;
}

final native function bool ShouldIgnoreNotifies() const;


/** == OVERRIDABLE INTERFACE == */
function Tick( float DeltaTime )
{
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	return (ChildCommand == None || ChildCommand.AllowTransitionTo( AttemptCommand ));
}

/** Notification called when this command has pushed */
function Pushed()
{
	//debug
	`AILog( "COMMAND PUSHED:"@self );
}

/** Notification when this command has popped */
function Popped()
{
	//debug
	`AILog( "COMMAND POPPED:"@self@"with"@Status );
}

function Paused(AICommand NewCommand)
{
	//debug
	`AILog( "COMMAND PAUSED:"@self@"by"@NewCommand );
}

function Resumed( Name OldCommandName )
{
	//debug
	`AILog( "COMMAND RESUMED:"@self@"from"@OldCommandName@"with"@ChildStatus );
}

event String GetDumpString()
{
	return String(self);
}

event DrawDebug( GearHUD_Base HUD, Name Category )
{
	if( ChildCommand != None )
	{
		ChildCommand.DrawDebug( HUD,Category );
	}
}

function AdjustEnemyRating(out float out_Rating, Pawn EnemyPawn)
{
	if (ChildCommand != None)
	{
		ChildCommand.AdjustEnemyRating(out_Rating, EnemyPawn);
	}
}

// AI Notification Functions
function bool NotifyCoverClaimViolation( Controller NewClaim, CoverLink Link, int SlotIdx )
{
	if( ChildCommand != None )
	{
		return ChildCommand.NotifyCoverClaimViolation( NewClaim, Link, SlotIdx );
	}
	return FALSE;
}

function bool MoveUnreachable( Vector AttemptedDest, Actor AttemptedTarget )
{
	if( ChildCommand != None )
	{
		return ChildCommand.MoveUnreachable( AttemptedDest, AttemptedTarget );
	}
	return FALSE;
}

function bool HandlePathObstruction( Actor BlockedBy )
{
	if( ChildCommand != None )
	{
		return ChildCommand.HandlePathObstruction( BlockedBy );
	}
	return FALSE;
}

function bool NotifyEnemyBecameVisible( Pawn VisibleEnemy )
{
	if( ChildCommand != None )
	{
		return ChildCommand.NotifyEnemyBecameVisible( VisibleEnemy );
	}
	return FALSE;
}

function Rotator GetAdjustedAimFor( Weapon W, vector StartFireLoc )
{
	// by default, call the controller's default version
	if( ChildCommand != None )
	{
		return ChildCommand.GetAdjustedAimFor(W,StartFireLoc);
	}
	return DefaultGetAdjustedAimFor(W,StartFireLoc);
}

function bool ShouldIgnoreTimeTransitions()
{
	if(ChildCommand != None)
	{
		return ChildCommand.ShouldIgnoreTimeTransitions();
	}

	return FALSE;
}

function NotifyNeedRepath()
{
	if(ChildCommand != none)
	{
		ChildCommand.NotifyNeedRepath();
	}
}

function bool AllowPushOfDefaultCommandForSpecialMove(ESpecialMove SM)
{
	return TRUE;
}

/**
 * ===========
 * DEBUG STATES
 * ===========
 */
state DEBUGSTATE
{
	function BeginState( Name PreviousStateName )
	{
		//debug
		`AILog( "BEGINSTATE"@GetStatename()@PreviousStateName, 'State' );
	}

	function EndState( Name NextStateName )
	{
		//debug
		`AILog( "ENDSTATE"@GetStatename()@NextStateName, 'State' );
	}

	function PushedState()
	{
		//debug
		`AILog( "PUSHED", 'State' );
	}

	function PoppedState()
	{
		//debug
		`AILog( "POPPED", 'State' );
	}

	function ContinuedState()
	{
		//debug
		`AILog( "CONTINUED", 'State' );
	}

	function PausedState()
	{
		//debug
		`AILog( "PAUSED", 'State' );
	}
}

/**
 *	Command has failed but delay pop to avoid infinite recursion
 */
state DelayFailure `DEBUGSTATE
{
Begin:
	Sleep( 0.5f );

	Status = 'Failure';
	PopCommand( self );
}

state DelaySuccess `DEBUGSTATE
{
Begin:
	Sleep( 0.1f );
	Status = 'Success';
	PopCommand( self );
}





defaultproperties
{
	bAllowedToFireWeapon=TRUE
	bAllowNewSameClassInstance=FALSE
	bReplaceActiveSameClassInstance=FALSE
}
