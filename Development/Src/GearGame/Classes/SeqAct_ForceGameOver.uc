class SeqAct_ForceGameOver extends SequenceAction;

event Activated()
{
	local GearPC PC;

	foreach GetWorldInfo().AllControllers(class'GearPC', PC)
	{
		PC.ClientGameOver();
	}
}

defaultproperties
{
	ObjName="Force Game Over"
	ObjCategory="Gear"
	VariableLinks.Empty()
}
