class SeqAct_Skorge_PistolStrafe extends SeqAct_Latent;

/** Route Skorge should run back and forth on */
var() Route RouteObj;
/** Route duration */
var() float	RouteDuration;
/** Location to teleport to before dropping from ceiling */
var() Actor	Start;
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
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Skorge: Pistol Strafe"
	ObjCategory="Boss"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Skorge",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Start",PropertyName=Start)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Dest",PropertyName=Destination)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="Route",PropertyName=RouteObj)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Float',LinkDesc="Duration",PropertyName=RouteDuration)
}