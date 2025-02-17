/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_Base_Boomer_Gatling extends AICmd_Base_Boomer
	within GearAI_Boomer;


state InCombat
{

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();
	// select the best enemy
	SelectEnemy();
	FireTarget = Enemy;

	if (!HasValidEnemy() || !bAllowCombatTransitions )
	{
		//debug
		`AILog("No enemy");

		Sleep(1.f);
		Goto('Begin');
	}

	// If have not fired from here AND
	// Enemy is in medium and can fire at enemy
	if( ShouldFireFromHere() )

	{
FireWeapon:
		bFiredFromHere = TRUE;

		while( IsReloading() )
		{
			//debug
			`AILog("Waiting on reload",'Loop');

			Sleep(0.25f);
		}

		// Fire from open
		FireFromOpen();

		Sleep( 0.25f );
	}
	// Otherwise, move closer to them
	// (pick a location to move to)
	else
	{
		bFiredFromHere = FALSE;

		StartFiring();
		// Move closer to them
		SetEnemyMoveGoal();
		StopFiring();

		// If failed to move to enemy, just fire weapon
		if( bFailedToMoveToEnemy )
		{
			//debug 
			`AILog("Failed to move to enemy");
			Goto( 'FireWeapon' );
		}
	}

	// check combat transitions
	//CheckCombatTransition();
	CheckInterruptCombatTransitions();

	Goto('Begin');
}

defaultproperties
{
}
