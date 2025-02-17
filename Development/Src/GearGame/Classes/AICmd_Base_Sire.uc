class AICmd_Base_Sire extends AICmd_Base_CoverAI
	within GearAI_Sire;

/** GoW global macros */

state InCombat
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@GetStateName(), 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();

	if( !SelectEnemy() )
	{
		//debug
		`AILog( "Unable to acquire an enemy" );

		Sleep( 1.f );
		Goto( 'End' );
	}
	FireTarget = Enemy;


	GearPawn_LocustSireBase(MyGearPawn).PlayChargeSound();
	SetEnemyMoveGoal(true);
	CheckInterruptCombatTransitions();
End:
	Goto('Begin');
};


defaultproperties
{

}
