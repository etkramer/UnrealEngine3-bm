class AICmd_Berserker_Kamikaze extends AICommand_Base_Combat
	within GearAI_Berserker;


auto state InCombat
{
	function BeginState( Name PreviousStateName )
	{
		super.BeginState( PreviousStateName );
		if( MyGearPawn != None )
		{
			MyGearPawn.DoSpecialMove(SM_Berserker_Alert,TRUE);
		}
	}

	function ContinuedState()
	{
		super.ContinuedState();
		if( MyGearPawn != None )
		{
			MyGearPawn.DoSpecialMove(SM_Berserker_Alert,TRUE);
		}
	}

	function EndState( Name NextStateName )
	{
		super.EndState( NextStateName );
		if( MyGearPawn != None )
		{
			MyGearPawn.EndSpecialMove();
		}
	}

	function MoveToChargeLocation()
	{
		local Actor Find;
		local int	RouteIdx;

		//debug
		`AILog( GetFuncName()@ChargeLocation@ChargeActor@ChargePivot );

		bReachedMoveGoal = FALSE;

		Pawn.PathSearchType = PST_NewBestPathTo;
		Find = FindPathTo( ChargePivot,, TRUE );
		if( Find != None )
		{
			//debug
			DebugLogRoute();

			// Find actor that is farthest along path and still directly reachable
			for( RouteIdx = 0; RouteIdx < RouteCache.Length; RouteIdx++ )
			{
				if( !ActorReachable(RouteCache[RouteIdx]) )
				{
					break;
				}
				Find = RouteCache[RouteIdx];
			}

			SetMoveGoal( Find, None, TRUE );
		}
		Pawn.PathSearchType = Pawn.default.PathSearchType;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@bDirectCharge, 'State' );

	// If charge target directly reachable
	if( bDirectCharge || ChargeLocationReachable( ChargePivot ) )
	{
		// CHARGE!!!
		class'AICmd_Attack_BerserkerCharge'.static.InitCommand(Outer);
	}
	else
	{
		// Otherwise, move along path to a point that target is reachable
		MoveToChargeLocation();

		// If reached point on path
		if( bReachedMoveGoal )
		{
			// debug
			`AILog( "Reached move goal, check for charge again" );

			// Try to charge again
			Goto( 'Begin' );
		}
		else
		{
			//debug
			`AILog( "Failed to path toward charge target"@ChargeActor@ChargeLocation@Pawn.Anchor );
		}
	}

	// Check combat transitions
	CheckCombatTransition();

	// if nothing happened, transition to the default combat command
	class'AICmd_Base_Berserker'.static.InitCommand(Outer);
}
