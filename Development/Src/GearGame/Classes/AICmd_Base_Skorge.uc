class AICmd_Base_Skorge extends AICommand_Base_Combat
	within GearAI_Skorge;

function Pushed()
{
	Super.Pushed();

	MyGearPawn.EndSpecialMove();
	StopFiring();

	Pawn.ZeroMovementVariables();
	Pawn.SetPhysics( PHYS_None );
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