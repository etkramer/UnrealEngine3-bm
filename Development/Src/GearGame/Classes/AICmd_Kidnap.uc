class AICmd_Kidnap extends AICommand_SpecialMove
	within GearAI;

/** Pawn that this AI should go over and attempt to kidnap */
var GearPawn KidnapTarget;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Kidnap(GearAI AI, GearPawn NewKidnapTarget)
{
	local AICmd_Kidnap OtherCmd, Cmd;

	if (AI != None && AI.Pawn != None && NewKidnapTarget != None)
	{
		OtherCmd = AI.FindCommandOfClass(class'AICmd_Kidnap');
		if (OtherCmd == None || OtherCmd.KidnapTarget != NewKidnapTarget)
		{
			Cmd = new(AI) default.Class;
			Cmd.KidnapTarget = NewKidnapTarget;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Target:"@KidnapTarget;
}

function Pushed()
{
	Super.Pushed();

	//debug
	`AILog( "... KidnapTarget"@KidnapTarget );

	// Move to the target first
	SetMoveGoal( KidnapTarget,,, KidnapTarget.GetCollisionRadius() );
}

function Resumed(name OldCommandName)
{
	Super.Resumed(OldCommandName);

	// If we reached the target successfully
	// and the target can still be executed
	if (ChildStatus == 'Success' && KidnapTarget != None && KidnapTarget.IsDBNO() && !KidnapTarget.IsAHostage())
	{
		// Do the kidnap
		GotoState('Command_SpecialMove');
	}
	else
	{
		// Otherwise, didn't get there so fail the command
		Status = 'Failure';
		PopCommand(self);
	}
}

function Popped()
{
	Super.Popped();

	SelectWeapon();
}

function bool IsAllowedToFireWeapon()
{
	// allow firing at other enemies until we get close to target
	return (KidnapTarget != FireTarget && ChildCommand != None && MoveTarget != KidnapTarget && Super.IsAllowedToFireWeapon());
}

state Command_SpecialMove
{
	function BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		bIgnoreNotifies = true;

		StopFiring();

		// Need to face the target
		DesiredRotation = rotator(KidnapTarget.Location - Pawn.Location);
	}

	function EndState(name NextStateName)
	{
		local GearPawn GP;

		Super.EndState(NextStateName);

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
		return KidnapTarget;
	}

	function ESpecialMove GetSpecialMove()
	{
		return SM_Kidnapper;
	}

	function bool ExecuteSpecialMove()
	{
		return (KidnapTarget.IsDBNO() && !KidnapTarget.IsAHostage() && Super.ExecuteSpecialMove());
	}

	function bool IsSpecialMoveComplete()
	{
		// the kidnapper special move persists so exit if we got to it
		return (MyGearPawn == None || MyGearPawn.SpecialMove == SM_Kidnapper || Super.IsSpecialMoveComplete());
	}
}

defaultproperties
{
	bShouldCheckSpecialMove=true
	bAllowedToFireWeapon=true
}
