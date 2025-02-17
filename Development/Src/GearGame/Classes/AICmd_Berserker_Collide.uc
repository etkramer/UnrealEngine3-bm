class AICmd_Berserker_Collide extends AICommand_SpecialMove;

function Popped()
{
	Super.Popped();

	bIgnoreNotifies = false; // so we can do the melee command if enemy is still in range
	CheckCombatTransition();
}

auto state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return SM_Berserker_Collide;
	}
}


DefaultProperties
{
	bIgnoreNotifies=true
}
