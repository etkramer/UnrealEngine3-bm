class AICmd_Base_Boomer_Melee extends AICommand_Base_Combat;

auto state InCombat
{
Begin:
	// Select Enemy
	if( !SelectEnemy() )
	{
		//debug
		`AILog( "Failed to select an enemy -- delay failure" );

		GotoState( 'DelayFailure' );
	}

	// make sure stale value doesn't cause AI to think it reached enemy when it didn't
	Pawn.DestinationOffset = 0.0;

	//debug
	`AILog( "BEGIN TAG"@IsMeleeRange(Enemy.Location)@Pawn.ReachedDestination(Enemy)@CanEngageMelee()@VSize(Enemy.Location-Pawn.Location) );


	// If close enough to melee enemy
	if ((IsMeleeRange(Enemy.Location) && FastTrace(Enemy.Location, Pawn.Location)) || Pawn.ReachedDestination(Enemy))
	{
		// SWING!
		if( CanEngageMelee() )
		{
			DoMeleeAttack();
		}
		else
		{
			Sleep(0.1f);
		}
	}
	else
	{
		SetEnemyMoveGoal(false, EnemyDistance_Melee);
	}

	// Goto TOP!
	Goto( 'Begin' );
}
