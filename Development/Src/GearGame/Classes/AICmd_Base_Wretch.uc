/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_Wretch extends AICmd_Base_CoverAI
	within GearAI_Wretch;

/** GoW global macros */

var instanced Goal_AtCover AtCover_Closer;
var CovGoal_GoalProximity CovGoalProx;
var bool bLastCoverSearchFailed;
var AIReactCond_GenericCallDelegate CoverExposedEvent;
/** Last time wrech reached good cover */
var float ReachedGoodCoverTime;

var   int NumHolds;
var() int MaxHolds;

function Pushed()
{
	local int i;

	Super.Pushed();

	// find the instanced goal proximity constraint
	for (i = 0; i < AtCover_Closer.CoverGoalConstraints.length; i++)
	{
		CovGoalProx = CovGoal_GoalProximity(AtCover_Closer.CoverGoalConstraints[i]);
		if (CovGoalProx != None)
		{
			break;
		}
	}

	CoverExposedEvent = new(outer) class'AIReactCond_GenericCallDelegate';
	CoverExposedEvent.OutputFunction = CoverExposed;
	CoverExposedEvent.AutoSubscribeChannels[0] = 'CoverExposed';
	CoverExposedEvent.Initialize();
}

function Popped()
{
	Super.Popped();

	CoverExposedEvent.UnsubscribeAll();
	CoverExposedEvent = none;
}

function CoverExposed(Actor EventInstigator, AIReactChannel OrigChannel)
{
	if( GetActiveCommand() != self)
	{
		return;
	}

	//MessagePlayer("My cover is exposed to "$EventInstigator$"!.."@outer);
	`AILog("My cover is exposed to "$EventInstigator$"!..");
	if( IsShortRange(GetEnemyLocation(Pawn(EventInstigator))) )
	{
		`AILog(EventInstigator$" is within range, going to melee!");
		//MessagePlayer(EventInstigator$" is within range, going to melee!");
		DoMeleeAttack(Pawn(EventInstigator));
	}
	else
	{
		`AILog(EventInstigator$" is NOT within range, looking for new cover");
		//MessagePlayer(EventInstigator$" is NOT within range, looking for new cover");
		GotoState('InCombat',,TRUE);
	}

}

function bool HasBeenInCoverTooLong()
{
	if( TimeSince( ReachedGoodCoverTime ) < 1.f )
	{
		return TRUE;
	}
	return FALSE;
}


state InCombat
{
	function bool PeekOutOfCover()
	{
		local array<ECoverAction>	Actions;
		local ECoverAction			Action;
		local GearPawn				GP;

		// Only peek out of cover half the time
		if( FRand() > 0.5 )
		{
			return FALSE;
		}

		GP = GearPawn(Pawn);
		if( Cover.Link != None && GP != None )
		{
			// Get a list of actions for this slot
			Cover.Link.GetSlotActions( Cover.SlotIdx, Actions );
			// If actions are available
			if( Actions.Length > 0 )
			{
				// Randomly choose an action
				Action = Actions[Rand(Actions.Length)];
				PendingFireLinkItem.SrcAction	= Action;
				PendingFireLinkItem.SrcType		= Cover.Link.Slots[Cover.SlotIdx].CoverType;

				return TRUE;
			}
		}

		return FALSE;
	}

	protected function MoveToBetterCover( CoverSlotMarker PrevBestMarker )
	{
		local CoverInfo NewCover;
		local float EnemyDist;

		// if cover search failed try increasing the radius a bit
		if (bLastCoverSearchFailed)
		{
			CovGoalProx.MaxGoalDist *= 1.5;
		}
		else
		{
			CovGoalProx.MaxGoalDist = EnemyDistance_Short;
		}
		CovGoalProx.bHardLimits = false;
		if(Enemy != none)
		{
			EnemyDist = Max(EnemyDistance_Melee,VSize((GetEnemyLocation(Enemy) - MyGearPawn.Location))*0.5f);// try and get closer every time
			if (EnemyDist < CovGoalProx.MaxGoalDist)
			{
				CovGoalProx.MaxGoalDist = EnemyDist;
				CovGoalProx.bHardLimits = true;
			}
		}

		if( EvaluateCover( SearchType_Towards, Enemy, NewCover, ,AtCover_Closer ) )
		{
			bLastCoverSearchFailed = false;
			//debug
			`AILog( "Moving to better cover"@NewCover.Link.GetDebugString(NewCover.SlotIdx)@"range: "$VSize(NewCover.Link.GetSlotLocation(NewCover.SlotIdx) -  Enemy.Location));

			// moving on up
			SetCoverGoal( NewCover, TRUE );

			if( Pawn.GetTeamNum() != 0 )
			{
				GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_TransToAdvanceAI, Pawn);
			}
		}
		else
		{
			bLastCoverSearchFailed = true;
			//debug
			`AILog("Unable to find mo'bettah covar!! :("@Cover.Link@Cover.SlotIdx );

			// set cover goal to current so that we don't treat it as a failure
			if( HasValidCover() )
			{
				SetCoverGoal( Cover );
			}			
		}
	}

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


	// Always clear cover b/c we want to advance
	ClearCoverGoal();

	// If we don't have cover claimed
	if( !IsShortRange(GetEnemyLocation()) )
	{
		//debug
		`AILog( "Enemy not in short range - move to better cover" );

		// Small chance that wretch updates all enemy positions
		// (prevents trying to continuously move closer to enemy)
		if( FRand() < 0.25f )
		{
			ForceUpdateOfEnemies();
		}

		MoveToBetterCover((WorldInfo.TimeSeconds - LastChangeCoverTime > 6.0) ? GetCoverSlotMarker() : None);
		if(ChildStatus == 'Failure')
		{
			// make sure that if the move failed we don't go into Hold
			`AILog("MoveToBetterCover cover failed... !");
			
			bFailedToMoveToEnemy = TRUE;
			if( HasValidEnemy() )
			{
				`AILog( "Try to move to an enemy instead..." );

				SetEnemyMoveGoal();
			}
			
			if( bFailedToMoveToEnemy )
			{
				`AILog( "Failed to mvoe to an enemy... sleep then end" );

				Sleep(0.25f);
				Goto('End');
			}			
		}

		ReachedGoodCoverTime = WorldInfo.TimeSeconds;
	}
	else if( FRand() < 0.50 || HasBeenInCoverTooLong() || IsCoverExposedToAnEnemy(Cover) )
	{
		ForceUpdateOfEnemies();

		DoMeleeAttack();
		Goto('End');
	}

	if (!HasValidCover())
	{
		//debug
		`AILog( "No cover found...");

		DoMeleeAttack();
	}
	else if (ActorReachable(Enemy))
	{
		`AILog("Attacking directly reachable enemy" @ Enemy);
		DoMeleeAttack();
	}
	else if(NumHolds++ > MaxHolds)
	{
		`AILog("Hit max holds.. going for it!");
		NumHolds=0;
		DoMeleeAttack();
	}
	else
	{
		//debug
		`AILog( "Holding... "@Cover.Link@Cover.SlotIdx, 'Combat' );
		Sleep( 1.f + (FRand() * 2.f) );
		if( PeekOutOfCover() )
		{
			// before transitioning again
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			while( IsTransitioning() )
			{
				Sleep( 0.25f );
			}

			// If should wait for transition (this function will mirror pawn if needed)
			if( ShouldWaitForTransition( PendingFireLinkItem.SrcAction ) )
			{
				//debug
				`AILog( "Wait for mirror transition", 'Combat' );

				// While still transitioning, wait
				LastAnimTransitionTime = WorldInfo.TimeSeconds;
				do
				{
					Sleep( 0.25f );
				} until( !IsTransitioning() );
			}

			// Set the action
			SetCoverAction( PendingFireLinkItem.SrcAction );

			// Sleep until done doing action
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );

			//debug
			`AILog( "Peeking... ", 'Combat' );

			// Sleep until we need to move again
			// Or we have a chance to peek again
			Sleep(0.5f + FRand() * 1.5f);

			ResetCoverAction();

			// Sleep until done resetting action
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );

			Sleep( 0.25f );
		}
	}

	CheckInterruptCombatTransitions();
	Sleep( 0.1f );

End:
	// Check combat transitions
	CheckCombatTransition();
	Goto( 'Begin' );
}

defaultproperties
{
	// ---->Goal proximity constraint
	Begin object Class=CovGoal_GoalProximity Name=CovGoal_Proximity0
		BestGoalDist=0.f
		MinGoalDist=0.f
		MaxGoalDist=768.0f
	End Object
	CovGoalProx=CovGoal_Proximity0
	// ---->Movement distance
	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist0
		BestCoverDist=768.0f
		MinCoverDist=256.0f
		MaxCoverDist=4096.0f
	End object

	Begin Object Class=Goal_AtCover name=AtCov_Closer0
		CoverGoalConstraints.Add(CovGoal_Proximity0)
		CoverGoalConstraints.Add(CovGoal_MovDist0)
		MaxToRate=100
	End object
	AtCover_Closer=AtCov_Closer0

	MaxHolds=1
}
