class AICmd_PushButton extends AICommand_SpecialMove;

var Trigger_ButtonInteraction Button;

static function bool PushButton(GearAI AI, Trigger_ButtonInteraction InTrigger)
{
	local AICmd_PushButton Cmd;

	if (InTrigger.TriggerEventClass(class'SeqEvt_Interaction', AI.MyGearPawn))
	{
		InTrigger.NotifyTriggered();
		Cmd = new(AI) default.Class;
		Cmd.Button = InTrigger;
		AI.PushCommand(Cmd);
		return true;
	}
	else
	{
		return false;
	}
}

function Pushed()
{
	MyGearPawn.SpecialMoveLocation = Button.Location;
	Focus = Button;
	MyGearPawn.DesiredRotation = rotator(Button.Location - MyGearPawn.Location);
	FireTarget = None;
	StopFiring();

	Super.Pushed();
}

auto state Command_SpecialMove
{
	function bool ShouldFinishRotation()
	{
		return true;
	}

	function ESpecialMove GetSpecialMove()
	{
		return SM_PushButton;
	}
}

defaultproperties
{
	TimeOutDelaySeconds=5.0
}

