class Trigger_LeviathanMouth extends Trigger
	notplaceable;

var GearPawn_LocustLeviathanBase Leviathan;

event BaseChange()
{
	Super.BaseChange();

	Leviathan = GearPawn_LocustLeviathanBase(Base);
}
	
defaultproperties
{
	bStatic=FALSE
	bNoDelete=FALSE

	Begin Object Name=CollisionCylinder
		CollisionRadius=80.0
		CollisionHeight=80.0
	End Object

	bHidden=TRUE
}