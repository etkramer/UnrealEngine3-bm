/**
 * Base class for behavior of Gunners mounted to another pawn
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_MountedGunner extends AICommand_Base_Combat;

/** GoW global macros */

var Pawn Mount;
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool InitCommand( GearAI AI )
{
    local AICmd_Base_MountedGunner Cmd;

    if( AI != None )
    {
	Cmd = new(AI) class'AICmd_Base_MountedGunner';
	if( Cmd != None )
	{
			Cmd.Mount = Pawn(AI.MyGearPawn.Base);
			AI.PushCommand( Cmd );
	    return TRUE;
	}
    }
    return FALSE;
}

function Pushed()
{
	Super.Pushed();
	ReactionManager.SuppressAllChannels();
	MyGearPawn.bRespondToExplosions=FALSE;
	GotoState('InCombat');
}

function Popped()
{
	Super.Popped();
	ReactionManager.UnSuppressAllChannels();
	MyGearPawn.bRespondToExplosions=MyGearPawn.default.bRespondToExplosions;
}
function Resumed( Name OldCommandName )
{
	if(Mount != Pawn.Base)
	{
		CheckCombatTransition();
	}
	Super.Resumed(OldCommandName);
}

function bool AllowTransitionTo(class<AICommand> AttemptCommand )
{
	if(Mount == Pawn.Base)
	{
		return false;
	}

	return Super.AllowTransitionTo(AttemptCommand);
}


state InCombat
{
	function BeginState(Name previousStateName)
	{
		Super.BeginState(previousStateName);
		bIgnoreStepAside=true;
	}

	function EndState(Name nextStateName)
	{
		Super.EndState(nextStateName);
		bIgnoreStepAside=false;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if(!bAllowCombatTransitions)
	{
		Sleep(1.0f);
		Goto('Begin');
	}
	if( SelectTarget() &&
		CanFireAt( FireTarget, Pawn.GetWeaponStartTraceLocation(), TRUE ) )
	{
		// If burst fire successful (fire line not blocked)
		StartFiring();

		// wait for the weapon to finish firing
		do
		{
			if(!bAllowCombatTransitions)
			{
				Sleep(1.0f);
				Goto('Begin');
			}

			if(Mount != Pawn.Base)
			{
				`AILog("Mount:"$Mount$" != Pawn.Base:"$Pawn.Base$", Checking for combat transitions...");
				AbortCommand(self);
				CheckCombatTransition();
			}

			Sleep( 0.25f );

			//debug
			`AILog( "Firing weapon...", 'Loop' );
		} until(!HasAnyEnemies() || !HasValidTarget());
		StopFiring();
	}
	else
	{
		//debug
		`AILog("HasAnyEnemies?"@HasAnyEnemies());
	}

	Sleep(0.8f);

	Goto('Begin');
}


defaultproperties
{
}
