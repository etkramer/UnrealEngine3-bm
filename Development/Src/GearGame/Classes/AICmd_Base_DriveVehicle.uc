class AICmd_Base_DriveVehicle extends AICommand_Base_Combat
	within GearAI;

var bool bAbleToFire;

function Pushed()
{
	Super.Pushed();

	ReactionManager.SuppressAll();

	if( Pawn.DrivenVehicle != None && 
		Pawn.DrivenVehicle.IsA( 'Vehicle_Centaur' ) )
	{
		bAbleToFire = FALSE;
	}
	else
	{
		bAbleToFire = TRUE;
	}

	GotoState( 'InCombat' );
}

function Popped()
{
	Super.Popped();

	ReactionManager.UnSuppressAll();
}

state InCombat
{

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if( !bAbleToFire || !SelectTarget() || GearVehicleWeapon(Pawn.Weapon) == None )
	{
		//debug
		`AILog( "Not able to fire at enemy"@bAbleToFire@Pawn.Weapon, 'Combat' );

		Goto( 'End' );
	}

	StartFiring();
	do
	{
		Sleep(0.25f);

		//debug
		`AILog( "Firing weapon...", 'Loop' );
	} until( !IsFiringWeapon() );

End:
	Sleep( 0.5f );
	Goto( 'Begin' );
}

defaultproperties
{
	InitialTransitionCheckTime=(X=0.f,Y=0.f)
}