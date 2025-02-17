/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_CoverAI_TDM extends AICmd_Base_CoverAI
	within GearAI_TDM;

/** GoW global macros */

/** time this state was pushed */
var float EnteredCombatTime;
/** last time we tried to flank */
var float LastFlankAttemptTime;


function Pushed()
{
	EnteredCombatTime = WorldInfo.TimeSeconds;

	Super.Pushed();
}

function Popped()
{
	// make sure we turn off the interrupt reaction
	DamageInterruptReaction.Suppress();

	Super.Popped();
}

state InCombat
{
	/** considers overriding the normal cover search with one that tries to get the AI closer to an important game objective */
	final function bool MoveToCoverTowardsObjective(optional CoverSlotMarker PrevBestMarker)
	{
		local Actor ObjectiveActor;
		local CoverInfo NewCover;
		local bool bOldIgnoreFireLinks, bMustGetCloser, bResult;

		// if there is an objective the AI needs to move toward, try to pick cover in that direction
		ObjectiveActor = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(Outer);
		// don't care if can hit enemy if moving to objective
		bOldIgnoreFireLinks = bIgnoreFireLinks;
		bIgnoreFireLinks = true;
		if (ObjectiveActor != None)
		{
			bMustGetCloser = ( GearGameMP_Base(WorldInfo.Game).MustStandOnObjective(Outer) &&
						VSize(Pawn.Location - ObjectiveActor.Location) <= 512.0 );
			if ( EvaluateCover( SearchType_Towards, ObjectiveActor, NewCover,
						(WorldInfo.TimeSeconds - LastShotAtTime < 3.0) ? PrevBestMarker : None,
						bMustGetCloser ? AtCover_VeryNear : None ) )
			{
				//debug
				`AILog("Moving to better cover" @ NewCover.Link.GetDebugString(NewCover.SlotIdx) @ "towards game objective" @ ObjectiveActor @ " - distance:" @ VSize(Pawn.Location - ObjectiveActor.Location));

				// moving on up
				SetCoverGoal(NewCover, true);
				bResult = true;
			}
			else if (bMustGetCloser)
			{
				`AILog("No cover closer to must stand on objective" @ ObjectiveActor);
				// override normal cover selection and just stand on the objective
				bResult = true;
				if (HasValidCover() && IsAtCover())
				{
					GotoState(, 'FireFromCover');
				}
				else if (HasValidTarget() && CanFireAt(FireTarget, Pawn.GetWeaponStartTraceLocation()))
				{
					FireFromOpen();
				}
				else
				{
					GotoState(, 'SleepOnObjective');
				}
			}
		}
		bIgnoreFireLinks = bOldIgnoreFireLinks;

		return bResult;
	}

	protected function MoveToInitialCover()
	{
		if (!MoveToCoverTowardsObjective())
		{
			Super.MoveToInitialCover();
		}
	}

	protected function MoveToBetterCover(CoverSlotMarker PrevBestMarker)
	{
		if (!MoveToCoverTowardsObjective(PrevBestMarker))
		{
			Super.MoveToBetterCover(PrevBestMarker);
		}
	}

	/** @return whether AI should move directly to important game objective */
	final function bool ShouldChargeObjective()
	{
		local Actor ObjectiveActor;
		local Pawn P;
		local float MyDist;

		if (Squad != None)
		{
			ObjectiveActor = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(Outer);
			if (ObjectiveActor != None)
			{
				MyDist = VSize(ObjectiveActor.Location - Pawn.Location);
				// if we have a meat shield, then always yes as we can use the shield for cover
				if (MyGearPawn != None && MyGearPawn.IsAKidnapper() && MyDist > 64.0)
				{
					`AILog("Charge game objective" @ ObjectiveActor @ "because have meat shield");
					return true;
				}
				if (MyDist <= 1024.0 || !IsUnderHeavyFire())
				{
					// don't bother if already close and don't need to be right on top of it
					if ( MyDist > 1024.0 || Abs(ObjectiveActor.Location.Z - Pawn.Location.Z) > 256.0 ||
						GearGameMP_Base(WorldInfo.Game).MustStandOnObjective(Outer) )
					{
						// if both we and our enemy are in close range, fight them instead
						if (MyDist < 256.0)
						{
							if (WorldInfo.TimeSeconds - LastShotAtTime < 2.0)
							{
								return false;
							}
							foreach Squad.AllEnemies(class'Pawn', P, Outer)
							{
								if (VSize(ObjectiveActor.Location - GetEnemyLocation(P)) < 1024.0)
								{
									return false;
								}
							}
						}
						// don't if under fire and any enemy is closer than we are
						else if (WorldInfo.TimeSeconds - LastShotAtTime < 4.0 || GetHealthPercentage() < 0.6)
						{
							foreach Squad.AllEnemies(class'Pawn', P, Outer)
							{
								// ignore ragdolled/DBNO enemies since they're helpless for the moment
								if ( P.Physics != PHYS_RigidBody && P.Health > 0 &&
									VSize(ObjectiveActor.Location - GetEnemyLocation(P)) < MyDist )
								{
									return false;
								}
							}
						}

						`AILog("Charge game objective" @ ObjectiveActor);
						return true;
					}
				}
			}
		}

		return false;
	}

	/** check if we should try to flank the enemy. Basically checks if our current situation clearly isn't getting anywhere. */
	final function bool CheckTryFlank()
	{
		local int LiveCount, FlankingCount;
		local GearAI OtherAI;
		local float DelayTime;
		local PlayerController PC;

		// don't flank on Casual unless all humans are dead
		if (GetDifficultyLevel() == DL_Casual && WorldInfo.Game.NumPlayers > 0)
		{
			foreach WorldInfo.AllControllers(class'PlayerController', PC)
			{
				if (PC.Pawn != None && !PC.IsDead())
				{
					break;
				}
			}
			if (PC != None)
			{
				return false;
			}
		}

		DelayTime = (CombatMood == AICM_Aggressive) ? 20.0 : 30.0;

		// if we're in an AI led squad
		// (and not the leader ourselves, because that will cause the other AI dudes to move to our destination,
		// effectively causing them to do a suicidal charge)
		// and we've been in combat for a while without anything interesting happening
		if ( Enemy != None && Squad != None && (Squad.Leader != Outer || GearTeamInfo(PlayerReplicationInfo.Team).GetLiveMembers() == 1) &&
			(PlayerController(Squad.Leader) == None || Squad.Leader.IsDead()) &&
			WorldInfo.TimeSeconds - LastShotAtTime > 2.0 &&
			WorldInfo.TimeSeconds - EnteredCombatTime > DelayTime &&
			WorldInfo.TimeSeconds - LastFlankAttemptTime > DelayTime &&
			WorldInfo.TimeSeconds - GearGameMP_Base(WorldInfo.Game).LastDownOrKillTime > DelayTime )
		{
			// see how many other squadmates are already flanking
			foreach Squad.AllMembers(class'GearAI', OtherAI)
			{
				if (!OtherAI.IsDead())
				{
					LiveCount++;
				}
				if (OtherAI != Outer && OtherAI.FindCommandOfClass(class'AICmd_Flank') != None)
				{
					FlankingCount++;
				}
			}

			if (FlankingCount < Max(1, LiveCount / 2))
			{
				class'AICmd_Flank'.static.InitCommand(Outer);
				return true;
			}
		}

		return false;
	}

	/** check if we should run away and if so pushes the appropriate movement command */
	final function bool CheckRetreat()
	{
		local Pawn CheckEnemy, ClosestEnemy;
		local float Dist, BestDist, ExposedScale;
		local bool bShouldRetreat;
		local CoverInfo NewCover;

		// only the leader considers tactical withdrawals
		if (GearPRI(PlayerReplicationInfo).bIsLeader)
		{
			// look at our enemies and evaluate danger
			BestDist = 1000000.0;
			foreach Squad.AllEnemies(class'Pawn', CheckEnemy, Outer)
			{
				// should retreat from this enemy if close or exposed
				Dist = VSize(GetEnemyLocation(CheckEnemy) - Pawn.Location);
				bShouldRetreat = (Dist < 1500.0);
				if (!bShouldRetreat)
				{
					if (MyGearPawn != None && MyGearPawn.IsInCover())
					{
						bShouldRetreat = (IsCoverExposedToAnEnemy(Cover, CheckEnemy, ExposedScale) && ExposedScale > 0.1);
					}
					else
					{
						bShouldRetreat = LineOfSightTo(CheckEnemy);
					}
				}

				// record closest enemy we should run from
				if (bShouldRetreat && Dist < BestDist)
				{
					ClosestEnemy = CheckEnemy;
					BestDist = Dist;
				}
			}

			if (ClosestEnemy != None)
			{
				`AILog("Need to retreat from" @ ClosestEnemy);
				bShouldRoadieRun = true;
				bIgnoreFireLinks = true;
				if (BestDist > 1024.0 && EvaluateCover(SearchType_Away, ClosestEnemy, NewCover, None))
				{
					bIgnoreFireLinks = false;
					SetCoverGoal(NewCover, true);
				}
				else
				{
					bIgnoreFireLinks =  false;
					// enemy is too close or couldn't find cover - just run!
					if (MyGearPawn == None || (!MyGearPawn.IsAKidnapper() && MyGearPawn.EquippedShield == None))
					{
						class'Path_AvoidFireFromCover'.static.AvoidFireFromCover(Pawn);
					}
					class'Path_AlongLine'.static.AlongLine(Pawn, -Normal(ClosestEnemy.Location - Pawn.Location));
					class'Path_AvoidanceVolumes'.static.AvoidThoseVolumes(Pawn);
					class'Goal_AwayFromPosition'.static.FleeFrom(Pawn, ClosestEnemy.Location, 4000);
					if (FindPathToward(ClosestEnemy) != None)
					{
						SetMoveGoal(RouteGoal,, true,, true, false);
					}
					else
					{
						`AILog("Failed to find path away!");
						return false;
					}
				}
				LeaderDamageInterruptReaction.Suppress();
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	function bool ShouldChangeCover()
	{
		local Actor ObjectiveActor;

		// override to change cover more frequently if the AI has a priority objective it needs to be close to
		if (Super.ShouldChangeCover())
		{
			return true;
		}
		else if ( WorldInfo.TimeSeconds - EnterCoverTime < 1.0 || GetHealthPercentage() < 0.9 ||
				!GearGameMP_Base(WorldInfo.Game).MustStandOnObjective(Outer) )
		{
			return false;
		}
		else
		{
			ObjectiveActor = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(Outer);
			return (ObjectiveActor != None && VSize(ObjectiveActor.Location - Pawn.Location) > 512.0);
		}
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@ShouldMoveToSquad()@AcquireCover@HasValidCover()@IsAtCover()@Cover.Link@Cover.SlotIdx@TetherActor, 'State' );

	if (CheckPickupWeapon())
	{
		Sleep(0.25);
		Goto('Begin');
	}

	CheckCombatMood();

	if (ShouldChargeObjective())
	{
		CheckReviveOrExecute(1200.0);
		if (MyGearPawn == None || !MyGearPawn.IsAKidnapper())
		{
			// interrupt objective charge if we take damage
			DamageInterruptReaction.UnSuppress();
		}
		SetMoveGoal(GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(Outer),, true,,,,,, true);
		DamageInterruptReaction.Suppress();
		Sleep(0.25);
		Goto('Begin');
	}

	if (ShouldPlantGrenade())
	{
		class'AICmd_Attack_PlantGrenade'.static.PlantGrenade(Outer, 750, 1250);
		LastGrenadePlantTime = WorldInfo.TimeSeconds;
	}

	if (CheckRetreat())
	{
		Sleep(0.25);
		// if our retreat dest was cover and we got hurt on the way, try to heal up before moving again
		if (GetDifficultyLevel() > DL_Normal)
		{
			while (GetHealthPercentage() < 0.95 && HasValidCover() && IsAtCover() && !IsCoverExposedToAnEnemy(Cover))
			{
				`AILog("Hiding out here while waiting for health to recover", 'Loop');
				Sleep(0.25);
			}
		}
		if (GearPRI(PlayerReplicationInfo).bIsLeader)
		{
			LeaderDamageInterruptReaction.UnSuppress();
		}
		Goto('Begin');
	}

	if (CheckTryFlank())
	{
		//@note: this should actually happen *after* the flank attempt since the function will push a new command
		LastFlankAttemptTime = WorldInfo.TimeSeconds;
	}

	// this will go to the label in the super class
	Goto('CheckDBNO');

// used when we need to stand on our objective for a while and there's no currently valid cover or enemies to shoot at
SleepOnObjective:
	Sleep(0.25 + 0.5 * FRand());
	Goto('CoverCheck');
}
