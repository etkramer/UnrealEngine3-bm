class AICmd_Base_Melee_TDM extends AICmd_Base_Melee
	within GearAI_TDM;

function Pushed()
{
	Super.Pushed();

	// damage interrupts longer range melee charges
	if (GearPawn(Enemy) != None && VSize(Enemy.Location - Pawn.Location) > 512.0 && !GearPawn(Enemy).IsAKidnapper())
	{
		DamageInterruptReaction.UnSuppress();
	}
}

function Popped()
{
	// turn the interrupt reaction back off
	DamageInterruptReaction.Suppress();

	Super.Popped();
}

function Resumed(name OldCommandName)
{
	if (ChildStatus == 'Aborted')
	{
		AbortCommand(self);
	}
	else
	{
		Super.Resumed(OldCommandName);
	}
}

state InCombat
{
	function AboutToExecuteMeleeAttack()
	{
		// no longer interruptible
		DamageInterruptReaction.Suppress();
	}
}
