class SeqAct_RemoveDroppedPickups extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}

defaultproperties
{
	ObjName="Redrop Pickups"
 	ObjCategory="Gear"

	VariableLinks.Empty
}
