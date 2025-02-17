class AICmd_Berserker_Stunned extends AICommand_SpecialMove;

function Popped()
{
	Super.Popped();

	CheckCombatTransition();
}

auto state Command_SpecialMove
{
	function float GetPostSpecialMoveSleepTime()
	{
		// Give enough time to animations to blend out.
		return 0.2f;
	}

	function ESpecialMove GetSpecialMove()
	{
		return SM_Berserker_Stunned;
	}
}


DefaultProperties
{
	bIgnoreNotifies=true
}