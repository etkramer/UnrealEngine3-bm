/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Execute extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

/** Pawn that this AI should go over and attempt to execute */
var GearPawn ExecuteTarget;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Execute( GearAI AI, GearPawn NewExecuteTarget )
{
	local AICmd_Execute OtherCmd, Cmd;

	if( AI != None &&
		NewExecuteTarget != None &&
		AI.CanExecutePawn( NewExecuteTarget ) )
	{
		OtherCmd = AI.FindCommandOfClass(class'AICmd_Execute');
		if (OtherCmd == None || OtherCmd.ExecuteTarget != NewExecuteTarget)
		{
			Cmd = new(AI) class'AICmd_Execute';
			if( Cmd != None )
			{
			    Cmd.ExecuteTarget = NewExecuteTarget;
			    AI.PushCommand(Cmd);
			    return TRUE;
			}
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Target:"@ExecuteTarget;
}

function Pushed()
{
	Super.Pushed();

	//debug
	`AILog( "... ExecuteTarget"@ExecuteTarget );

	// Move to the target first
	SetMoveGoal( ExecuteTarget,, true);
}

function Resumed( Name OldCommandName )
{
	local bool bFailedKidnap;

	Super.Resumed( OldCommandName );

	if (OldCommandName == 'AICmd_Kidnap')
	{
		if (ChildStatus == 'Success')
		{
			Status = 'Success';
			PopCommand(self);
		}
		else
		{
			bFailedKidnap = true;
		}
	}
	// If we reached the target successfully
	// and the target can still be executed
	if ((bFailedKidnap || ChildStatus == 'Success') && ExecuteTarget != None && CanExecutePawn(ExecuteTarget))
	{
		// consider taking this guy as a hostage instead
		if ( bFailedKidnap || !IsUnderHeavyFire() || GetHealthPercentage() > 0.8 || MyGearPawn.IsCarryingAHeavyWeapon() ||
			MyGearPawn.IsCarryingShield() || !MyGearPawn.CanDoSpecialMove(SM_Kidnapper) ||
			GearInventoryManager(MyGearPawn.InvManager).GetInventoryInSlot(EASlot_Holster) == None ||
			!class'AICmd_Kidnap'.static.Kidnap(Outer, ExecuteTarget) )
		{
			// Do the execution
			GotoState( 'Command_SpecialMove' );
		}
	}
	else
	{
		// Otherwise, didn't get there so fail the command
		Status = 'Failure';
		PopCommand( self );
	}
}

function bool IsAllowedToFireWeapon()
{
	local bool bResult;

	// allow firing at other enemies until we get close to target
	bResult = (ExecuteTarget != FireTarget && ChildCommand != None && MoveTarget != ExecuteTarget && Super.IsAllowedToFireWeapon());
	if (!bResult)
	{
		// turn off firing completely so we stop facing FireTarget and rotate towards ExecuteTarget
		StopFiring();
		Focus = ExecuteTarget;
		SetFocalPoint(vect(0,0,0));
	}
	return bResult;
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		StopFiring();

		// Need to face the target
		DesiredRotation = Rotator(ExecuteTarget.Location-Pawn.Location);
		Focus = ExecuteTarget;
	}

	function EndState( Name NextStateName )
	{
		local GearPawn GP;

		Super.EndState( NextStateName );

		// Clear the interaction pawn once execution is done
		GP = GearPawn(AIOwner.Pawn);
		if( GP != None )
		{
			GP.InteractionPawn = None;
		}
	}

	// Turn to face the target before doing the special move
	function bool ShouldFinishRotation() { return TRUE; }

	function GearPawn GetInteractionPawn()
	{
		return ExecuteTarget;
	}

	function ESpecialMove GetSpecialMove()
	{
		// Randomly choose an execution move
		if( FRand() >= 0.5f )
		{
			return SM_CQCMove_CurbStomp;
		}
		return SM_CQCMove_PunchFace;
	}
}

defaultproperties
{
	bShouldCheckSpecialMove=true
	bAllowedToFireWeapon=true
}
