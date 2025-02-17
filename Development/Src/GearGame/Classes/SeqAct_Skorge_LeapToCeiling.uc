class SeqAct_Skorge_LeapToCeiling extends SeqAct_Latent;

/** Location to teleport to when done jumping to ceiling */
var() Actor	Destination;

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Skorge: Leap"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Leap Up")
	InputLinks(1)=(LinkDesc="Drop Down")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Skorge",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Start/Dest",PropertyName=Destination)
}