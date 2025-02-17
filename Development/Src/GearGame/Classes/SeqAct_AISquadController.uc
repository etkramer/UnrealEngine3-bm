class SeqAct_AISquadController extends SequenceAction
	dependsOn(GearSquadFormation);

var() EGearSquadFormationType	FormationType;
var	  array<GameplayRoute>		SquadRoute;
var() EPerceptionMood			PerceptionMood;
var() ECombatMood				CombatMood;
var() EAIMoveMood				MoveMood;
/** when this is TRUE (default) guys in this squad will have their tethers cleared when assigned with this move */
var() bool						bClearTethers;

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
	ObjName="Squad Controller"
	ObjCategory="Gear"

	bClearTethers=true
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Member",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Gameplay Route",PropertyName=SquadRoute)
}
