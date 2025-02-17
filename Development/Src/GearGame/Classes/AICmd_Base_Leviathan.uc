class AICmd_Base_Leviathan extends AICommand_Base_Combat
	within GearAI_Leviathan;

function Pushed()
{
	Super.Pushed();
}
	
auto state InCombat
{
Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );
	
	Stop;
}

defaultproperties
{
	InitialTransitionCheckTime=(X=0.f,Y=0.f)
}