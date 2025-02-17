/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_CoverAI extends AICommand_Base_Combat
	within GearAI_Cover;

/** GoW global macros */

function Pushed()
{
	local bool bInitReact;
	Super.Pushed();

	// turn on flank reactions
	Reactionmanager.UnSuppressReactionsByType(class'AIReactCond_Flanked',false);

	// Check if we should play initial reaction for combat first
	bInitReact = (FRand() < PlayIntialCombatReactionChance);
	PlayIntialCombatReactionChance = 0.f;	// Clear immediately
	if( bInitReact && FALSE) // FIXME: anims for inital combat reaction don' exist any more?
	{
		class'AICmd_React_InitialCombat'.static.InitialCombatReact( CoverOwner );
	}
	else
	{
		GotoState( 'InCombat' );
	}
}

function Popped()
{
	Super.Popped();

	// turn off flank reactions
	Reactionmanager.SuppressReactionsByType(class'AIReactCond_Flanked',false);
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if( OldCommandName == 'AICmd_React_InitialCombat' )
	{
		GotoState( 'InCombat' );
	}
}

state InCombat
{
	/** used when the AI wants to move to cover and it is not already in cover */
	protected function MoveToInitialCover()
	{
		MoveToNearestCoverEnemy();
	}

	/** used when the AI is in cover but wants to evaluate moving to different cover */
	protected function MoveToBetterCover( CoverSlotMarker PrevBestMarker )
	{
		local CoverInfo NewCover;
		local bool		bTowardsGoal;
		local float		NearEnemyDist;

		NearEnemyDist = GetNearEnemyDistance(Pawn.Location);

		// search for cover either towards/away based on closest enemy distance
		bTowardsGoal = (CombatMood == AICM_Aggressive || (CombatMood != AICM_Passive && NearEnemyDist > EnemyDistance_Short));
		if( EvaluateCover( bTowardsGoal ? SearchType_Towards : SearchType_Away, Enemy, NewCover,PrevBestMarker ) )
		{
			//debug
			`AILog( "Moving to better cover"@NewCover.Link.GetDebugString(NewCover.SlotIdx) );

			// moving on up
			SetCoverGoal( NewCover, TRUE );

			if( Pawn.GetTeamNum() != 0 )
			{
				// @fixme test for flanking here too?
				if( bTowardsGoal )
				{
					GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_TransToAdvanceAI, Pawn);
				}
				else
				{
					GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_TransToRetreatAI, Pawn);
				}
			}
		}
		else
		{
			//debug
			`AILog("Unable to find mo'bettah covar!! :("@Cover.Link@Cover.SlotIdx );

			// set cover goal to current so that we don't treat it as a failure
			SetCoverGoal( Cover );
		}
	}

	/** @return whether we should look for better cover */
	function bool ShouldChangeCover()
	{
		if( (!HasValidCover() || !IsAtCover()) && AllowedToMove())
		{
			return TRUE;
		}

		if(  AcquireCover == ACT_Immediate ||
			(AcquireCover == ACT_DesireBetter &&
				TimeSince(EnterCoverTime) >= MinStayAtCoverTime) &&
				AllowedToMove())
		{
			return TRUE;
		}

		return FALSE;
	}

	/** if the LD specified any pickups the AI should consider, check if we should get one here */
	final function CheckScriptedPickups()
	{
		local Actor A;
		local GearWeaponPickupFactory Factory;
		local GearDroppedPickup Pickup;
		local class<GearWeapon> WeaponClass;

		if (SPItemsToConsider.length > 0 && FRand() < 0.35)
		{
			// first check if we are touching one
			foreach Pawn.TouchingActors(class'Actor', A)
			{
				Factory = GearWeaponPickupFactory(A);
				if (Factory != None)
				{
					if ( SPItemsToConsider.Find(Factory.WeaponPickupClass) != INDEX_NONE &&
						Factory.CanBePickedUpBy(Pawn) )
					{
						class'AICmd_PickupWeapon'.static.PickupWeaponFromFactory(Outer, Factory);
						return;
					}
				}
				else
				{
					Pickup = GearDroppedPickup(A);
					if ( Pickup != None)
					{
						WeaponClass = class<GearWeapon>(Pickup.InventoryClass);
						if ( WeaponClass != None && SPItemsToConsider.Find(WeaponClass) != INDEX_NONE &&
							Pickup.CanBePickedUpBy(Pawn) )
						{
							class'AICmd_PickupWeapon'.static.PickupWeaponFromPickup(Outer, Pickup);
							return;
						}
					}
				}
			}
			// now check if there's one nearby we should move to
			foreach WorldInfo.RadiusNavigationPoints(class'GearWeaponPickupFactory', Factory, Pawn.Location, 1024.0)
			{
				if ( SPItemsToConsider.Find(Factory.WeaponPickupClass) != INDEX_NONE &&
						Factory.CanBePickedUpBy(Pawn) )
				{
					class'AICmd_PickupWeapon'.static.PickupWeaponFromFactory(Outer, Factory);
					return;
				}
			}
			foreach WorldInfo.CollidingActors(class'GearDroppedPickup', Pickup, 1024.0, Pawn.Location, false)
			{
				WeaponClass = class<GearWeapon>(Pickup.InventoryClass);
				if ( WeaponClass != None && SPItemsToConsider.Find(WeaponClass) != INDEX_NONE &&
					Pickup.CanBePickedUpBy(Pawn) )
				{
					class'AICmd_PickupWeapon'.static.PickupWeaponFromPickup(Outer, Pickup);
					return;
				}
			}
		}
	}

	/** @return if we need to change cover immediately because we are too exposed */
	protected final function bool ShouldChangeCoverDueToExposure()
	{
		local float ExposedScale;
		local bool	bResult;

		// if we are significantly exposed or we're somewhat exposed but taking damage
		bResult = IsCoverExposedToAnEnemy(Cover,, ExposedScale);

		//debug
		`AILog( GetFuncName()@bResult@ExposedScale@GetHealthPercentage() );

		if( bResult )
		{
			if( ExposedScale > 0.5 || GetHealthPercentage() < 0.75 )
			{
				return TRUE;
			}
		}

		return FALSE;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@ShouldMoveToSquad()@AcquireCover@HasValidCover()@IsAtCover()@Cover.Link@Cover.SlotIdx@TetherActor, 'State' );

	CheckScriptedPickups();
CheckDBNO:
	CheckReviveOrExecute(1200.0);

	// check to see if we should move to our squad
	if( ShouldMoveToSquad() )
	{
MoveToSquad:
		//debug
		`AILog( "MOVETOSQUAD TAG", 'State' );

		MoveToSquadPosition();
	}
CoverCheck:
	//debug
	`AILog( "COVERCHECK TAG", 'State' );

	CheckInterruptCombatTransitions();
	// make sure we can acquire an enemy
	if( !SelectEnemy() )
	{
		//debug
		`AILog( "Unable to acquire an enemy", 'Combat' );

		if (HasAnyEnemies() && AllowedToMove())
		{
			ClearMovementInfo();
			// move to nearest cover if we don't have cover already
			if (!HasValidCover() || !IsAtCover())
			{
				// try to move to cover
				MoveToNearestCoverIdle();
			}
		}

		Sleep(0.5f);
		Goto('End');
	}
	FireTarget = Enemy;

	//debug
	`AILog( "Move to cover?"@AcquireCover@HasValidCover()@IsAtCover()@TimeSince(EnterCoverTime)@AllowedToMove() );

	// if we're not allowed to move, then just fire
	if(!AllowedToMove() && !HasValidCover())
	{
		// if we're not allowed to move, and we don't have valid cover check to see if we have a move action trying to get us into cover.. if so reapply it
		if(MoveAction != none && MoveAction.DestinationSlotIndex > -1 && MoveAction.Targets.length > 0)
		{
			`AILog(">>> I'm not allowed to move, and I have a tethered cover node, but I'm not at it!  Re-initing move action to get back to my tether");
			OnAIMove(MoveAction);
		}
		else
		{
			FireFromOpen();

			if( bFailedToFireFromOpen )
			{
				//debug
				`AILog( "Failed to fire from open" );

				Sleep( 0.25f );
				// go to the end and loop all the way back to the beginning so we'll try move to squad and such
				Goto('End');
			}

		}
	}
	// check to see if we want/need to change cover
	else
	if( ShouldChangeCover() && AllowedToMove())
	{
		ClearMovementInfo();

		//debug
		`AILog( "ShouldChangeCover() TRUE!"@HasAnyEnemiesRequiringCover() );

		if( HasAnyEnemiesRequiringCover() )
		{
			// move to nearest cover if we don't have cover already
			if( !HasValidCover() || !IsAtCover() )
			{
				// try to move to cover
				MoveToInitialCover();
			}
			else
			{
				// look for some better cover
				MoveToBetterCover( (AcquireCover == ACT_DesireBetter) ? GetCoverSlotMarker() : None );
			}
		}

		// If we failed to find cover
		if( !IsValidCover( CoverGoal ) )
		{
			// then move to the nearest enemy for a couple seconds, then try again
			if( !IsUsingSuppressiveWeapon() &&
				!FailedPathToEnemyRecently( Enemy ) &&
				 VSize(Enemy.Location - Pawn.Location) > EnemyDistance_Short &&
				 IsEnemyWithinCombatZone(Enemy)
			  )
			{
				SetTimer( 3.f,FALSE,nameof(TimedAbortMove) );
				SetTether(Enemy,EnemyDistance_Short,TRUE,2.f);
				TetherActor = None;
				if (!bReachedMoveGoal && ChildStatus != 'Aborted')
				{
					//debug
					`AILog("Unable to move to enemy, attempt to just shoot them?");

					// this only happens in my horrible test map, but figured might as
					// well have yet another fallback
					FireFromOpen();

					if( bFailedToFireFromOpen )
					{
						//debug
						`AILog( "Failed to fire from open" );

						Sleep( 0.25f );
						// go to the end and loop all the way back to the beginning so we'll try move to squad and such
						Goto('End');
					}
				}
			}
			else if (!HasValidCover() || !IsAtCover())
			{
				ResetCoverType();

				//debug
				`AILog("No cover, and already close, just shooting at them");

				FireFromOpen();

				if( bFailedToFireFromOpen )
				{
					//debug
					`AILog( "Failed to fire from open" );

					Sleep( 0.25f );
					// go to the end and loop all the way back to the beginning so we'll try move to squad and such
					Goto('End');
				}
			}
			else
			{
				SetAcquireCover( ACT_None, "Clearing acquisition check in hopes that things are better" );
			}
		}
		Sleep(0.25f);
		if (MyGearPawn != None && !MyGearPawn.IsAKidnapper() && !MyGearPawn.IsCarryingShield())
		{
			Goto('CoverCheck');
		}
	}
	else
	{
FireFromCover:
		if (MyGearPawn.HealthRechargePercentPerSecond > 0.0 && GetHealthPercentage() < 0.4 && GetDifficultyLevel() > DL_Normal)
		{
			`AILog("Hide in cover while health recharges");
			bFailedToFireFromCover = false;
			Sleep(0.75 + FRand());
			Goto('End');
		}
		else if( ShouldFireFromCover() )
		{
			//debug
			`AILog( "should fire from cover" );

			FireFromCover();

			//debug
			`AILog( "finished firing from cover"@bFailedToFireFromCover@AcquireCover@HasValidCover()@IsAtCover()@TimeSince(LastFireFromCoverTime)@TimeSince(EnterCoverTime) );
		}
		else
		{
			//debug
			`AILog("Shouldn't fire from cover, peeking instead"@TimeSince(LastFireFromCoverTime) );

			Goto('PeekFromCover');
		}


		if( HasValidCover() && IsAtCover() )
		{
			//debug
			`AILog( "Cover is currently valid... failed?"@bFailedToFireFromCover@TimeSince(LastFireFromCoverTime)@TimeSince(EnterCoverTime)@IsCoverWithinCombatZone( Cover )@ShouldChangeCoverDueToExposure()@AllowedToMove() );

			if(AllowedToMove())
			{
				// if this cover is exposed or cover isn't within our assigned zones
				if (!IsCoverWithinCombatZone( Cover ) || ShouldChangeCoverDueToExposure())
				{
					SetAcquireCover( ACT_Immediate, "Change cover immediately CZ:"@IsCoverWithinCombatZone(Cover)@"Exposed:"@IsCoverExposedToAnEnemy(Cover) );
				}
				// if not able to fire
				else if (bFailedToFireFromCover && (WorldInfo.TimeSeconds - LastFireFromCoverTime < 3.f || ShouldFireFromCover()))
				{
					if (WorldInfo.TimeSeconds - LastChangeCoverTime > 20.0)
					{
						SetAcquireCover(ACT_Immediate, "Failed to fire from cover and have been here a long time");
					}
					else
					{
						// acquire new cover on the next loop
						SetAcquireCover(ACT_DesireBetter, "Failed to fire from cover only a short time ago");
						Goto('PeekFromCover');
					}
				}
				// the enemy isn't within our desired ranges
				else if( GetNearEnemyDistance(Pawn.Location) < EnemyDistance_Short ||
					GetNearEnemyDistance(Pawn.Location) > EnemyDistance_Medium )
				{
					// then try to find new cover right away
					SetAcquireCover( ACT_DesireBetter, "Not in optimal range" );
				}
				// if the enemy is attempting to snipe us, try to move between cover slots each time we fire
				// to make ourselves a harder target
				else if (Enemy != None && GearWeapon(Enemy.Weapon) != None && GearWeapon(Enemy.Weapon).bSniping)
				{
					`AILog("Switch coverslots to evade sniping enemy");
					class'Path_CoverSlotsOnly'.static.CoverSlotsOnly(Pawn);
					MoveToBetterCover(GetCoverSlotMarker());
				}
				else
				{
					//debug
					`AILog( "Everything is happy"@AcquireCover );
				}
			}
		}
		else
		{
			//debug
			`AILog( "Something not happy"@bFailedToFireFromCover@TimeSince(LastFireFromCoverTime)@AcquireCover@HasValidCover()@IsAtCover() );
		}
	}
End:
	//debug
	`AILog( "END TAG", 'State' );

	CheckCombatTransition();
	Goto('Begin');

PeekFromCover:
	//debug
	`AILog( "PEEKFROMCOVER TAG", 'State' );

	PeekFromCover();
	if( bFailedToFireFromCover )
	{
		Sleep( 0.5f );
	}
	Goto( 'End' );
}

defaultproperties
{
}
