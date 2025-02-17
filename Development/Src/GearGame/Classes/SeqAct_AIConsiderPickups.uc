class SeqAct_AIConsiderPickups extends SequenceAction;

var() array< class<GearWeapon> > ConsiderList;

defaultproperties
{
	ObjCategory="AI"
	ObjName="AI Consider Pickups"
	InputLinks[0]=(LinkDesc="Start")
	InputLinks[1]=(LinkDesc="Stop")
}
