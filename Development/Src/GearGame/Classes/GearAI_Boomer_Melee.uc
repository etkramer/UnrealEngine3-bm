class GearAI_Boomer_Melee extends GearAI_Boomer;

function FireFromOpen(optional int InBursts = 1, optional bool bAllowFireWhileMoving=true);

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Boomer_Melee'
	bAimAtFeet=FALSE
}
