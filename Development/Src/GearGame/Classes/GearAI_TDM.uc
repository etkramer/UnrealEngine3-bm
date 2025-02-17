/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAI_TDM extends GearAI_Cover
	native(AI);

cpptext
{
	virtual UBOOL IsPlayerOwner()
	{
		return TRUE;
	}
}

/** chance to select the hammerburst as our starting weapon instead of the lancer */
var config float HammerburstChance;

var Actor RoamGoal;
/** indicates RouteCache is valid path to RoamGoal */
var bool bRoamGoalCacheValid;
/** force weapon rush when idle (instead of considering game objective)
 * used for secondary squads to spread out the AI, take more map control, and get them attacking from different angles
 */
var bool bForceWeaponRush;
/** when we can't find any enemies in a non-objective game (e.g. TDM), after powering up on weapon pickups we go
 * looking around for the enemy, starting with their spawn location. This flag indicates whether we've reached that step.
 */
var bool bCheckedEnemySpawnPoint;

/** last time we tried to plant a grenade */
var float LastGrenadePlantTime;

/** reaction used for interrupting certain actions when we take damage */
var instanced AIReactCondition_Base DamageInterruptReaction;
/** similiar damage interrupt check only used for the team leader in leader-based gametypes (Guardian) */
var instanced AIReactCondition_Base LeaderDamageInterruptReaction;
/** cover evaluator used to keep the AI very close to important game objectives */
var instanced Goal_AtCover AtCover_VeryNear;
/** version of AtCover_Toward used when mood is aggressive and targeting enemy */
var instanced Goal_AtCover AtCover_TowardEnemyAggressive;

auto state WaitingForPawn
{
Begin:
	Stop;
}

event PreBeginPlay()
{
	// we can't do this in defaults because it will get instanced separately (so DamageInterruptReaction wouldn't be pointing to the array entry)
	DefaultReactConditions.AddItem(DamageInterruptReaction);
	DefaultReactConditions.AddItem(LeaderDamageInterruptReaction);

	Super.PreBeginPlay();
}

function InitPlayerReplicationInfo()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue WeaponValue;
	local int ProviderIndex;
	local GearGameWeaponSummary WeaponInfo;

	Super.InitPlayerReplicationInfo();

	// if we want to use the hammerburst, look up its index and set that in the PRI
	if (FRand() < HammerburstChance)
	{
		GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
		if (GameResourceDS != None)
		{
			// Find the provider id for the weapon
			WeaponValue.PropertyTag = 'ClassPathName';
			WeaponValue.PropertyType = DATATYPE_Property;
			WeaponValue.StringValue = "GearGame.GearWeap_LocustAssaultRifle";
			ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Weapons', 'ClassPathName', WeaponValue);
			if (ProviderIndex != INDEX_NONE)
			{
				WeaponInfo = GameResourceDS.GetWeaponProvider(ProviderIndex);
				if (WeaponInfo != None)
				{
					GearPRI(PlayerReplicationInfo).InitialWeaponType = WeaponInfo.WeaponId;
				}
			}
		}
	}
}

function NotifyReachedCover()
{
	Super.NotifyReachedCover();

	SetTimer(4.0, true, nameof(DecayCover));
}

/** adds a decay cost to the cover the AI is currently using */
final function DecayCover()
{
	if (Squad != None && MyGearPawn != None && MyGearPawn.IsInCover() && !GearPRI(PlayerReplicationInfo).bIsLeader)
	{
		// decay cover faster when aggressive to keep the AI closing in
		Squad.AddCoverDecay(MyGearPawn.CurrentLink.Slots[MyGearPawn.CurrentSlotIdx].SlotMarker, (CombatMood == AICM_Aggressive) ? 600 : 400);
	}
	else
	{
		ClearTimer('DecayCover');
	}
}

/** if the AI is on an AI team or the human leader is dead, considers changing combat mood based on game state */
final function CheckCombatMood()
{
	if (Pawn != None && Squad != None && (Squad.Leader == None || AIController(Squad.Leader) != None || Squad.Leader.IsDead()))
	{
		GearGameMP_Base(WorldInfo.Game).SetAICombatMoodFor(self);
	}
}

function SetIsInCombat()
{
	// in MP we always say not in combat (@see GearPC::IsInCombat())
	if (MyGearPawn != None)
	{
		MyGearPawn.bIsInCombat = false;
	}
	ClearTimer('SetIsInCombat');
}

function bool StepAsideFor(Pawn ChkPawn)
{
	// if we are the leader, we step aside for nobody!
	// this mainly avoids AI confusion at the beginning of the match when everyone is leaving the spawn point
	return (GetSquadLeader() != self && Super.StepAsideFor(ChkPawn));
}

function bool WantsInfiniteAmmoFor(GearWeapon Wpn)
{
	// only give MP bots infinite ammo for assault rifles
	return (Wpn.IsA('GearWeap_AssaultRifle') || Wpn.IsA('GearWeap_LocustAssaultRifle'));
}

/** @return whether the given point is the move goal for another AI on this team */
final function bool IsMoveGoalForTeamMember(NavigationPoint N)
{
	local GearTeamInfo Team;
	local int i;
	local GearAI AI;

	Team = GearTeamInfo(PlayerReplicationInfo.Team);
	if (Team != None)
	{
		for (i = 0; i < Team.TeamMembers.length; i++)
		{
			AI = GearAI(Team.TeamMembers[i]);
			if (AI != None && AI != self && (AI.MoveGoal == N || AI.RouteGoal == N))
			{
				return true;
			}
		}
	}

	return false;
}

/**
 * Picks someplace to go when not in combat and we have no squad leader or are the leader
 */
final function protected Actor PickRoamGoal()
{
	local GearWeaponPickupFactory Pickup;
	local Controller C;
	local Actor ObjectiveActor;

	ObjectiveActor = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self);
	if (!bForceWeaponRush)
	{
		// first ask gametype if there's somewhere important to go
		RoamGoal = ObjectiveActor;
	}
	if (RoamGoal == None)
	{
		// next look at weapon pickups, except if we're in sudden death so need to hurry up and resolve things
		if (!GearGameMP_Base(WorldInfo.Game).bIsInSuddenDeath)
		{
			foreach WorldInfo.AllNavigationPoints(class'GearWeaponPickupFactory', Pickup)
			{
				if ( Pickup != ObjectiveActor &&
					Pickup.WeaponPickupClass != None && Pickup.ReadyToPickup(0) &&
					VSize(Pickup.Location - Pawn.Location) > 512.f &&
					(Pickup.FindInteractionWith(Pawn, false) != '' || Pickup.FindInteractionWith(Pawn, true) == 'SwapWithCurrent') &&
					!IsMoveGoalForTeamMember(Pickup) )
				{
					Pickup.bTransientEndPoint = true;
					// put a random extra cost on the pickups so they don't always go for the same one
					Pickup.TransientCost = Rand(6000);
					RoamGoal = Pickup;
				}
			}
		}
		// this will actually check all the pickups we marked
		Pawn.PathSearchType = PST_Breadth; //@todo: really just want to avoid the biasing for paths toward RoamGoal,
						//	since there are usually multiple choices
		if (RoamGoal != None && FindPathToward(RoamGoal) != None)
		{
			RoamGoal = RouteGoal;
			bRoamGoalCacheValid = true;
		}
		// try enemy start location if we haven't already
		else if (!bForceWeaponRush && !bCheckedEnemySpawnPoint)
		{
			foreach WorldInfo.AllControllers(class'Controller', C)
			{
				if ( C.GetTeamNum() != GetTeamNum() && !C.IsDead() && C.Pawn != None && C.Pawn.Health > 0 &&
					C.IsPlayerOwned() )
				{
					RoamGoal = WorldInfo.Game.FindPlayerStart(None, C.GetTeamNum());
					break;
				}
			}
			if (RoamGoal == None)
			{
				`AILog("Failed to find an enemy PlayerStart to check for stragglers!");
				RoamGoal = Pawn.Anchor;
			}
			bCheckedEnemySpawnPoint = true;
		}
		// try some random location so the AI looks around for enemies
		else if (!bForceWeaponRush && FindRandomDest() != None)
		{
			RoamGoal = RouteGoal;
			bRoamGoalCacheValid = true;
		}
		else
		{
			// fall back to just staying where we are
			RoamGoal = Pawn.Anchor;
			bForceWeaponRush = false;
		}
		Pawn.PathSearchType = Pawn.default.PathSearchType;
	}
	return RoamGoal;
}

/** checks if the AI is standing on a weapon pickup and whether it wants that weapon
 * if so, pushes AICmd_PickupWeapon
 * @return if the AI is picking up a weapon now
 */
final function bool CheckPickupWeapon()
{
	local GearWeaponPickupFactory Factory;
	local GearDroppedPickup Pickup;
	local class<GearWeapon> WeaponClass;
	local int i;

	// can't pick stuff up while holding a meat shield or heavy weapon
	if (MyGearPawn != None && (MyGearPawn.IsAKidnapper() || MyGearPawn.IsCarryingAHeavyWeapon()))
	{
		return false;
	}

	foreach Pawn.TouchingActors(class'GearWeaponPickupFactory', Factory)
	{
		//@note: we currently assume that the LD-placed pickups are always better than what we have
		if ( Factory.WeaponPickupClass != None && Factory.ReadyToPickup(0.0) &&
			(Factory.FindInteractionWith(Pawn, false) != '' || Factory.FindInteractionWith(Pawn, true) == 'SwapWithCurrent') )
		{
			class'AICmd_PickupWeapon'.static.PickupWeaponFromFactory(self, Factory);
			return true;
		}
	}

	foreach Pawn.TouchingActors(class'GearDroppedPickup', Pickup)
	{
		WeaponClass = class<GearWeapon>(Pickup.InventoryClass);
		if ( WeaponClass != None && WeaponClass.default.AIRating >= 1.0 &&
			(Pickup.FindInteractionWith(Pawn, false) != '' || Pickup.FindInteractionWith(Pawn, true) == 'SwapWithCurrent') )
		{
			class'AICmd_PickupWeapon'.static.PickupWeaponFromPickup(self, Pickup);
			return true;
		}
	}

	// if we are adjacent to one then consider moving to it if doing so won't get us killed
	if (!IsUnderHeavyFire() && Pawn.ValidAnchor())
	{
		for (i = 0; i < Pawn.Anchor.PathList.length; i++)
		{
			if (Pawn.Anchor.PathList[i] != None)
			{
				Factory = GearWeaponPickupFactory(Pawn.Anchor.PathList[i].End.Actor);
				if ( Factory != None && Factory.WeaponPickupClass != None && Factory.ReadyToPickup(0.0) && Factory.CanBePickedUpBy(Pawn) &&
					(Factory.FindInteractionWith(Pawn, false) != '' || Factory.FindInteractionWith(Pawn, true) == 'SwapWithCurrent') )
				{
					SetMoveGoal(Factory,, true);
					return true;
				}
			}
		}
	}


	return false;
}

final function bool AllowDetourToPickUp(class<Inventory> InvClass)
{
	// if we're not in any known danger and we're not about to kill/DBNO an enemy
	// then query the inventory and ask it if it is good enough
	return ( MyGearPawn != None && !MyGearPawn.IsAKidnapper() &&
		(Enemy == None || float(Enemy.Health) / float(Enemy.HealthMax) > 0.35) &&
		InvClass.static.DetourWeight(Pawn, 1) > 0.0 &&
		FindCommandOfClass(class'AICmd_Revive') == None && FindCommandOfClass(class'AICmd_Execute') == None );
}

function bool ShouldMoveToSquad( optional bool bSkipLastPositionCheck )
{
	// don't run around looking for the leader when there are enemies around unless playing wingman
	return ( ( GearGameWingman_Base(WorldInfo.Game) != None || (HasSquadLeader() && GearPRI(GetSquadLeader().PlayerReplicationInfo).bIsLeader) ||
			(!IsUnderHeavyFire() && GetNearEnemyDistance(Pawn.Location) > EnemyDistance_Long) ) &&
		Super.ShouldMoveToSquad(bSkipLastPositionCheck) );
}

function CheckCombatTransition()
{
	local bool bAbort;
	local Pawn P;
	local AICmd_MoveToSquadLeader MoveToLeaderCmd;

	CheckCombatMood();

	// don't abort roaming for long range enemy unless we're actually getting hit
	if ((CommandList != None || !HasSquadLeader()) && AICommand_Base_Combat(CommandList) == None)
	{
		if (GetHealthPercentage() >= 0.9)
		{
			bAbort = true;
			foreach Squad.AllEnemies(class'Pawn', P)
			{
				if (IsMediumRange(GetEnemyLocation(P)))
				{
					bAbort = false;
					break;
				}
			}
		}
	}

	if (bAbort)
	{
		// check again after some time
		// this handles the case where we don't receive any more enemy notifications, but
		// our roam goal happens to put the enemy in range anyway
		SetTimer( 1.5, false, nameof(CheckCombatTransition) );
	}
	else
	{
		// check if we should abort any move to leader commands because we saw enemies in the way
		MoveToLeaderCmd = FindCommandOfClass(class'AICmd_MoveToSquadLeader');
		if (MoveToLeaderCmd != None && !ShouldMoveToSquad())
		{
			AbortCommand(MoveToLeaderCmd);
		}

		Super.CheckCombatTransition();
	}
}

function bool ShouldReturnToIdle()
{
	local int i;

	if (Super.ShouldReturnToIdle())
	{
		return true;
	}
	else
	{
		// return to idle if we haven't seen any enemy in over 30 seconds
		for (i = 0; i < Squad.EnemyList.length; i++)
		{
			if ( Squad.EnemyList[i].Pawn != None && Squad.EnemyList[i].Pawn.IsValidEnemyTargetFor(PlayerReplicationInfo, true) &&
				WorldInfo.TimeSeconds - Squad.EnemyList[i].LastUpdateTime < 30.0 )
			{
				return false;
			}
		}

		return true;
	}
}

function bool CheckInterruptCombatTransitions()
{
	// if casual (dumb) AI gets caught moving around, sometimes get lost and stand there shooting at them
	if ( GetDifficultyLevel() == DL_Casual && IsFiringWeapon() && IsUnderHeavyFire() && Enemy != None &&
		Pawn.LastHitBy != None && Pawn.LastHitBy.Pawn == Enemy && GetHealthPercentage() < 0.9 &&
		FRand() < 0.5 && AICmd_MoveToGoal(GetActiveCommand()) != None && CanFireAt(FireTarget, Pawn.GetWeaponStartTraceLocation()) )
	{
		FireFromOpen();
		return true;
	}
	else
	{
		return Super.CheckInterruptCombatTransitions();
	}
}

function bool IsValidMeleeTarget(GearPawn WP)
{
	local GearGameMP_Base Game;
	local Actor ObjectiveActor;

	// if we need to be standing on an objective point, don't leave it to melee somebody
	Game = GearGameMP_Base(WorldInfo.Game);
	if (Game.MustStandOnObjective(self))
	{
		ObjectiveActor = Game.GetObjectivePointForAI(self);
		if ( ObjectiveActor != None && VSize(ObjectiveActor.Location - Pawn.Location) <= 256.0 &&
			VSize(WP.Location - ObjectiveActor.Location) > 256.0 )
		{
			return false;
		}
	}

	return Super.IsValidMeleeTarget(WP);
}

function bool CheckReviveOrExecute(float CheckDist)
{
	local Pawn P;
	local bool bEnemiesRemain;

	// increase range if we need to execute enemies to kill them
	if (GearGameMP_Base(WorldInfo.Game).bExecutionRules)
	{
		CheckDist *= 1.5;
	}

	// if we have no important objective and all enemies are dead or DBNO,
	// then allow execution or revival at any range
	if (GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self) == None)
	{
		foreach Squad.AllEnemies(class'Pawn', P, self)
		{
			if (!P.IsA('GearPawn') || !GearPawn(P).IsDBNO())
			{
				bEnemiesRemain = true;
				break;
			}
		}

		if (!bEnemiesRemain)
		{
			if (HasAnyEnemies())
			{
				bShouldRoadieRun = true;
			}
			CheckDist = 1000000.0;
		}
	}

	return Super.CheckReviveOrExecute(CheckDist);
}

function bool CanExecutePawn(GearPawn GP)
{
	local GearGameMP_Base Game;
	local Actor ObjectiveActor;
	local float EnemyDist;

	if (Pawn != None)
	{
		// if we need to stand on a game objective, don't execute anyone unless they are on the way
		Game = GearGameMP_Base(WorldInfo.Game);
		if (Game.MustStandOnObjective(self))
		{
			ObjectiveActor = Game.GetObjectivePointForAI(self);
			if (ObjectiveActor != None && ObjectiveActor != GP)
			{
				EnemyDist = VSize(GP.Location - ObjectiveActor.Location);
				if (EnemyDist > 256.0 && VSize(ObjectiveActor.Location - Pawn.Location) < EnemyDist)
				{
					return false;
				}
			}
		}
	}

	return Super.CanExecutePawn(GP);
}

function GoExecuteEnemy(GearPawn GP)
{
	// see if we should kidnap this Pawn instead
	if ( MyGearPawn != None && !MyGearPawn.IsDBNO() && !MyGearPawn.IsAKidnapper() &&
		GP == GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self) )
	{
		class'AICmd_Kidnap'.static.Kidnap(self, GP);
	}
	else
	{
		Super.GoExecuteEnemy(GP);
	}
}

function bool EvaluateCover( ECoverSearchType Type, Actor GoalActor, out CoverInfo out_Cover,
				optional CoverSlotMarker InBestMarker, optional Goal_AtCover GoalEvaluator, optional float MaxDistFromGoal )
{
	// if we want to charge, use the special goal evaluator with more weight towards enemy
	if ( CombatMood == AICM_Aggressive && GoalEvaluator == None && GoalActor == Enemy && Type == SearchType_Towards &&
		(MyGearPawn == None || MyGearPawn.MyGearWeapon == None || !MyGearPawn.MyGearWeapon.bSniping) )
	{
		return Super.EvaluateCover(Type, GoalActor, out_Cover, InBestMarker, AtCover_TowardEnemyAggressive, MaxDistFromGoal );
	}
	else
	{
		return Super.EvaluateCover(Type, GoalActor, out_Cover, InBestMarker, GoalEvaluator, MaxDistFromGoal );
	}
}

function bool CanFireWeapon(Weapon Wpn, byte FireModeNum)
{
	local bool bResult;
	local Actor ObjectiveActor;
	local AICmd_Execute ExecuteCmd;

	// if our target is highly exposed and we're close, then stop and take aim for better accuracy
	bResult = Super.CanFireWeapon(Wpn, FireModeNum);
	if ( bResult && FireModeNum == 0 && MyGearPawn != None && !MyGearPawn.IsInCover() && !MyGearPawn.IsAKidnapper() &&
		CoverSlotMarker(MoveTarget) == None && GetHealthPercentage() > 0.75 && HasValidTarget() &&
		(GearPawn(FireTarget) == None || !GearPawn(FireTarget).IsAKidnapper() || (MyGearPawn.MyGearWeapon != None && MyGearPawn.MyGearWeapon.bCanNegateMeatShield)) &&
		(IsMediumRange(FireTarget.Location) || (MyGearPawn.MyGearWeapon != None && MyGearPawn.MyGearWeapon.bSniping)) &&
		AICmd_MoveToGoal(GetActiveCommand()) != None && FindCommandOfClass(class'AICmd_Attack_FireFromOpen') == None &&
		FastTrace(FireTarget.Location, Pawn.GetWeaponStartTraceLocation(Wpn)) )
	{
		// if we're on the way to execute an enemy that requires it to die, don't get distracted
		ExecuteCmd = FindCommandOfClass(class'AICmd_Execute');
  		if (ExecuteCmd != None && !GearGame(WorldInfo.Game).CanBeShotFromAfarAndKilledWhileDownButNotOut(ExecuteCmd.ExecuteTarget, Pawn, None))
  		{
  			return true;
  		}
  		else
  		{
			// don't if we're about to reach important objective
			ObjectiveActor = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self);
			if ( ObjectiveActor != None && VSize(ObjectiveActor.Location - Pawn.Location) < 512.0 &&
				GearGameMP_Base(WorldInfo.Game).MustStandOnObjective(self) )
			{
				return true;
			}
			else
			{
				// see if we want a better weapon first - if so, start the switch and keep moving
				SelectWeapon();
				if (!IsSwitchingWeapons())
				{
					FireFromOpen();
				}
				return false;
			}
		}
	}
	else
	{
		// see if we have a lull in combat and should try reloading
		if ( !bResult && FireModeNum != class'GearWeapon'.const.RELOAD_FIREMODE &&
			MyGearPawn != None && MyGearPawn.MyGearWeapon != None &&
			MyGearPawn.MyGearWeapon.CanReload() && !MyGearPawn.MyGearWeapon.IsReloading() &&
			!MyGearPawn.MyGearWeapon.IsFiring() && WorldInfo.TimeSeconds - LastShotAtTime > 3.0 &&
			GearPRI(PlayerReplicationInfo).Difficulty.default.DifficultyLevel > DL_Casual )
		{
			if (Enemy == None)
			{
				MyGearPawn.MyGearWeapon.ForceReload();
			}
			else if ( WorldInfo.TimeSeconds - Squad.GetEnemyLastSeenTime(Enemy) > 3.0 &&
				VSize(Enemy.Location - Pawn.Location) > EnemyDistance_Melee * 1.5 &&
				float(Enemy.Health) / Enemy.HealthMax > 0.4 )
			{
				MyGearPawn.MyGearWeapon.ForceReload();
			}
		}
		return bResult;
	}
}

/** @return whether the AI should plant a grenade near its current location */
final function bool ShouldPlantGrenade()
{
	local Pawn CheckEnemy;
	local GearWeap_GrenadeBase Grenade;
	local Actor Objective;

	// if we have a grenade, we're near our objective, don't need to stay on it, and are not currently engaged with any enemies
	if ( WorldInfo.TimeSeconds - LastGrenadePlantTime > 20.0 &&
		WorldInfo.TimeSeconds - LastShotAtTime > 2.0 && FRand() < 0.25 * GetDifficultyLevel() )
	{
		Objective = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self);
		if ( Objective != None && Pawn(Objective) == None && VSize(Objective.Location - Pawn.Location) < 512.0 &&
			!GearGameMP_Base(WorldInfo.Game).MustStandOnObjective(self) )
		{
			Grenade = GearWeap_GrenadeBase(Pawn.FindInventoryType(class'GearWeap_GrenadeBase', true));
			if (Grenade != None && Grenade.HasAnyAmmo())
			{
				foreach Squad.AllEnemies(class'Pawn', CheckEnemy, self)
				{
					if (VSize(CheckEnemy.Location - Pawn.Location) < 1500.0 && TimeSinceLastSeenEnemy(CheckEnemy, true) < 4.0)
					{
						return false;
					}
				}

				return true;
			}
		}
	}

	return false;
}

/** interrupts the first child of the base command (if any) */
function InterruptAction(Actor EventInstigator, AIReactChannel OrigChannel)
{
	if (CommandList != None && CommandList.ChildCommand != None)
	{
		`AILog("Aborting current action due to impulse from" @ OrigChannel.ChannelName);
		AbortCommand(CommandList.ChildCommand);

		if (FRand() < 0.25 * GearPRI(PlayerReplicationInfo).Difficulty.default.DifficultyLevel && CanEvade())
		{
			DoEvade(GetBestEvadeDir(MyGearPawn.Location, Pawn(EventInstigator)), true);
		}
	}
}

/** called when the leader takes damage to consider whether he should be interrupted so he can run away instead */
function CheckLeaderInterrupt(Actor EventInstigator, AIReactChannel OrigChannel)
{
	local float ExposedScale;

	// trigger if we take a bunch of damage in the open
	if ( GearPRI(PlayerReplicationInfo).bIsLeader && GetHealthPercentage() < 0.7 && MyGearPawn != None &&
		(!MyGearPawn.IsInCover() || (IsCoverExposedToAnEnemy(Cover,, ExposedScale) && ExposedScale > 0.7)) )
	{
		if (AICmd_Base_CoverAI(CommandList) == None)
		{
			BeginCombatCommand(GetDefaultCommand(), "Restart combat logic due to leader damage and exposure");
		}
		else
		{
			if (CommandList.ChildCommand != None)
			{
				AbortCommand(CommandList.ChildCommand);
			}
			CommandList.GotoState(, 'Begin');
		}
	}
}

/** @return whether bot should move to objective when idle instead of staying in formation around leader */
final function bool IdleFollowerShouldMoveToObjective()
{
	local Actor ObjectiveActor;

	// do so if we have an objective and it is reasonably close to our leader
	ObjectiveActor = GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self);
	return (ObjectiveActor != None && VSize(ObjectiveActor.Location - GetSquadLeaderLocation()) < 1500.0);
}

// bots are never stale!
function SetEnableDeleteWhenStale(bool bAmIExpendable);
function ConditionalDeleteWhenStale();

state Action_Idle
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@TetherActor, 'State' );

	bRoamGoalCacheValid = false;
	RoamGoal = None;

	CheckCombatMood();

	// check if should pickup weapon
	if (CheckPickupWeapon())
	{
		// CheckPickupWeapon() should have pushed the pickup weapon command
		Sleep(0.5f);
	}
	// check for a tether
	else if( (Pawn != None) && !IsWithinTether( Pawn.Location ) )
	{
		`AILog("Not within tether, moving");

		// Move to tether
		SetTether( TetherActor, DesiredTetherDistance, bDynamicTether, TetherPersistDuration );
		// clear it if we failed
		if (!IsWithinTether(Pawn.Location))
		{
			ClearMovementInfo();
		}
	}
	else
	{
		// If we have enemies
		if (HasAnyEnemies() && !ShouldReturnToIdle())
		{
			// Try to notice one which will put us into combat
			CheckNoticedEnemy();
		}
		if (ShouldPlantGrenade())
		{
			class'AICmd_Attack_PlantGrenade'.static.PlantGrenade(self, 750, 1250);
			LastGrenadePlantTime = WorldInfo.TimeSeconds;
		}
		if (!bForceWeaponRush && HasSquadLeader())
		{
			if (IdleFollowerShouldMoveToObjective())
			{
				Goto('Roam');
			}
			// otherwise if we have no enemies but do have a squad
			else if ( ShouldMoveToSquad() )
			{
MoveToSquad:
				//debug
				`AILog("Moving to squad...");

				// then move to the squad
				MoveToSquadPosition();

				// If no tether actor to clear
				if( bFailedToMoveToEnemy )
				{
					// Sleep to prevent recursive craziness
					Sleep( 1.0f );
				}

				// If we shouldn't keep the tether
				if( !bKeepTether )
				{
					// Clear the tether so that we always update to a new one
					TetherActor = None;
				}
				bKeepTether = FALSE;

				// check to see if we should immediately move again
				if (!HasAnyEnemies() &&
					ShouldMoveToSquad())
				{
					//debug
					`AILog("Continuing move to squad");
					Sleep(0.1f);
					Goto('MoveToSquad');
				}
				else
				{
					`AILog("Not continuing move to squad?"@HasAnyEnemies()@ShouldMoveToSquad());
				}
				Sleep(0.25f);
			}
			else if (VSize(GetSquadLeaderLocation() - Pawn.Location) < 1024.0)
			{
MoveToCover:
				// check to see if we should get into cover because of our squad leader
				if (IsAlert() &&
					(!HasValidCover() || !IsAtCover()))
				{
					`AILog("Moving to cover because of alertness or squad leader being in cover");
					ClearMovementInfo();
					MoveToNearestCoverIdle();
					if (!bReachedMoveGoal)
					{
						`AILog("No nearby cover or failed to reach cover, sleeping");
						Sleep(0.5f);
					}
				}
				else if (IsAlert() && IsAtCover())
				{
					PeekFromCover();
					if( bFailedToFireFromCover )
					{
						Sleep( 0.5f );
					}
				}
				else
				{
					`AILog("Shouldn't move to squad or cover, sleeping"@TetherActor@MoveAction);
					Sleep(0.5f);
				}
			}
			else
			{
				Goto('Roam');
			}
		}
		else
		{
Roam:
			PickRoamGoal();
			// if we're near our goal then chill in cover
			if ( RoamGoal != None && VSize(RoamGoal.Location - Pawn.Location) < 256.f &&
				(!RoamGoal.IsA('PickupFactory') || !PickupFactory(RoamGoal).ReadyToPickup(0)) )
			{
				if (HasAnyEnemies())
				{
					// intentional override of custom "do other stuff until enemy is close" logic
					// because if we got here, there's nothing better to do but shoot at them
					Super.CheckCombatTransition();
					Sleep(0.5f);
				}
				else
				{
					Goto('MoveToCover');
				}
			}
			else
			{
				// no squad leader, so move to a random goal
				SetMoveGoal(RoamGoal,, TRUE,, bRoamGoalCacheValid,,,, TRUE );
				Sleep(0.5f);
			}
		}
	}
	Goto('Begin');
}

function CleanupPRI()
{
	`AILog(GetFuncName()@"booooo");
	PlayerReplicationInfo.Destroy();
	PlayerReplicationInfo = None;
}

function PawnDied(Pawn InPawn)
{
	local GearPawn WP;
	local GearGame Game;

	//debug
	`AILog(GetFuncName()@InPawn);

	if (InPawn == Pawn)
	{
		if (MyGearPawn != None)
		{
			// catch a special case when killed mid spawn
			MyGearPawn.bSpawning = false;
		}

		RouteCache_Empty();

		if (Squad != None)
		{
			foreach Squad.AllEnemies(class'GearPawn', WP)
			{
				WP.ReleaseFireTicket(self);
			}
		}

		while (CommandList != None)
		{
			AbortCommand(None, class'AICommand');
		}

		UnClaimCover();

		Game = GearGame(WorldInfo.Game);
		if (Game != None && Game.ShouldRespawnPC(PlayerReplicationInfo.Team.TeamIndex,self))
		{
			GearPRI(PlayerReplicationInfo).PlayerStatus = WPS_Respawning;
			Game.UseTeamRespawn(PlayerReplicationInfo.Team.TeamIndex);
		}
		else
		{
			GearPRI(PlayerReplicationInfo).PlayerStatus = WPS_Dead;
		}

		TetherActor = None;
	}

	// skip GearAI::PawnDied() so we don't lose Squad, etc
	Super(AIController).PawnDied(InPawn);

	MyGearPawn = None;
}

/** called to start AI's begin game rush delayed when it's a stupid AI (casual) */
final function StartRush()
{
	bShouldRoadieRun = true;
}

function Possess(Pawn NewPawn, bool bVehicleTransition)
{
	Super.Possess(NewPawn, bVehicleTransition);

	if (Pawn != None && !bVehicleTransition)
	{
		GearPRI(PlayerReplicationInfo).PlayerStatus = WPS_Alive;
		if (GearPRI(PlayerReplicationInfo).bIsLeader)
		{
			LeaderDamageInterruptReaction.UnSuppress();
		}
		else
		{
			// roadie run from start location like players do unless we're the leader (so the other guys can get ahead and protect)
			// delayed a bit if we're on a low difficulty level so humans can get ahead and take all the weapons
			if (GetDifficultyLevel() == DL_Casual)
			{
				SetTimer(10.0 + 5.0 * FRand(), false, nameof(StartRush));
			}
			else
			{
				bShouldRoadieRun = true;
			}
		}
		// clear out old transient AI info
		LastSquadLeaderPosition = vect(0,0,0);
		bCheckedEnemySpawnPoint = false;
	}
}

function bool WantsToRoadieRunToLeader()
{
	return !IsTimerActive(nameof(StartRush));
}

function bool ShouldRoadieRun()
{
	local GearPawn GP;

	// if moving to objective, roadie run only if can't see enemy
	// also roadie run if objective is a kidnapper or hostage (e.g. Meatflag), since firing from afar against that is not useful
	if ( MoveGoal != None && GearGameMP_Base(WorldInfo.Game).GetObjectivePointForAI(self) == MoveGoal &&
		(!bShouldRoadieRun || AICommand_Base_Combat(CommandList) != None) )
	{
		bShouldRoadieRun = false;
		if (Enemy == None || VSize(Enemy.Location - Pawn.Location) > EnemyDistance_Long * 1.5)
		{
			bShouldRoadieRun = true;
		}
		else if ( !IsUnderHeavyFire() &&
			(MyGearPawn == None || MyGearPawn.MyGearWeapon == None || !MyGearPawn.MyGearWeapon.bCanNegateMeatShield) )
		{
			GP = GearPawn(MoveGoal);
			if ( GP != None && (GP.IsAHostage() || GP.IsAKidnapper()) &&
				(vector(GP.Rotation) dot Normal(GP.Location - Pawn.Location) > 0.0 ? !IsShortRange(GP.Location) : !IsMediumRange(GP.Location)) )
			{
				bShouldRoadieRun = true;
			}
		}
		if (!bShouldRoadieRun)
		{
			bShouldRoadieRun = GetNearEnemyDistance(Pawn.Location) > 512.0 && !LineOfSightTo(Enemy);
		}
	}
	// if the enemy is tagging us from cover and we can't easily return fire (i.e. no exposure), roadie run to get away faster
	else if ( !bShouldRoadieRun && GetDifficultyLevel() > DL_Normal && GetHealthPercentage() < 0.9 && IsUnderHeavyFire() &&
		(MyGearPawn == None || !MyGearPawn.IsInCover()) )
	{
		GP = GearPawn(Enemy);
		if ( GP != None && GP.IsInCover() &&
			vector(GP.CurrentLink.GetSlotRotation(GP.CurrentSlotIdx)) dot Normal(Pawn.Location - Enemy.Location) > 0.7 )
		{
			bShouldRoadieRun = true;
		}
	}
	return Super.ShouldRoadieRun();
}

function InvalidateCoverFromDamage(Actor Damager)
{
	local CoverSlotMarker CurrentMarker;
	local CoverInfo NewCover;
	local AICmd_Attack_FireFromCover FireCmd;

	if (GetDifficultyLevel() > DL_Casual)
	{
		// pop back down
		StopFiring();
		// try moving to a different cover slot
		CurrentMarker = GetCoverSlotMarker();
		CurrentMarker.FearCost += 500;
		WorldInfo.Game.bDoFearCostFallOff = true;
		// don't disrupt an existing move for this
		if (FindCommandOfClass(class'AICmd_MoveToGoal') == None)
		{
			class'Path_CoverSlotsOnly'.static.CoverSlotsOnly(Pawn);
			if (EvaluateCover(SearchType_Towards, Enemy, NewCover, CurrentMarker))
			{
				//debug
				`AILog("Switching cover slots to" @ NewCover.Link.GetDebugString(NewCover.SlotIdx) @ "due to damage");

				// abort any fire from cover command first
				FireCmd = AICmd_Attack_FireFromCover(GetActiveCommand());
				if (FireCmd != None)
				{
					AbortCommand(FireCmd);
				}

				// moving on up
				SetCoverGoal(NewCover, true);
			}
		}
	}
}

/** checks if the given enemy we just received an update on has snuck up close to us (near melee range) when we weren't paying attention */
final function CheckVeryCloseEnemyWarning(Pawn CheckEnemy)
{
	local float CheckEnemyDist;

	// if we're a stupid bot, never pay attention
	// if we're already paying attention to that enemy or we're in cover and not exposed to that enemy from here, ignore it
	if ( GetDifficultyLevel() > DL_Casual && Enemy != CheckEnemy && CheckEnemy.Health > 0 && CheckEnemy.Physics != PHYS_RigidBody &&
		(MyGearPawn == None || !MyGearPawn.IsInCover() || (MyGearPawn.CoverAction == CA_Default && IsCoverExposedToAnEnemy(Cover, CheckEnemy))) )
	{
		// check if it's close and that we don't have an even more dangerous target
		CheckEnemyDist = VSize(CheckEnemy.Location - Pawn.Location);
		if (CheckEnemyDist < 384.0 && (Enemy == None || CheckEnemyDist < VSize(Enemy.Location - Pawn.Location)))
		{
			`AILog("Detected untargeted enemy" @ Enemy @ "within very short range!");
			// switch enemies
			SetEnemy(CheckEnemy);
			// abort fire/peek from cover actions
			AbortCommand(None, class'AICmd_Attack_FireFromCover');
			AbortCommand(None, class'AICmd_Attack_PeekFromCover');
			// maybe evade
			if (CanEvade() && FRand() < 0.25 * GearPRI(PlayerReplicationInfo).Difficulty.default.DifficultyLevel)
			{
				ResetCoverType();
				DoEvade(GetBestEvadeDir(CheckEnemy.Location, CheckEnemy));
			}
			// go melee them instead
			else if (IsValidMeleeTarget(GearPawn(CheckEnemy)) && !CommandList.IsA('AICmd_Base_Melee'))
			{
				ReactionManager.NudgeChannel(CheckEnemy, 'EnemyWithinMeleeDistance');
			}
		}
	}
}

event NotifyEnemyHeard(Pawn HeardEnemy, name NoiseType)
{
	Super.NotifyEnemyHeard(HeardEnemy, NoiseType);

	CheckVeryCloseEnemyWarning(HeardEnemy);
}
event NotifyEnemySeen(Pawn SeenEnemy)
{
	Super.NotifyEnemySeen(SeenEnemy);

	CheckVeryCloseEnemyWarning(SeenEnemy);
}

function CheckNearMiss(Pawn Shooter, GearWeapon GearWeap, const out ImpactInfo Impact, class<DamageType> InDamageType)
{
	local float OldNearMissDistance;

	OldNearMissDistance = AI_NearMissDistance;

	// if we're pretty sure they're shooting at us, increase check distance
	if (Shooter != None && Shooter.Controller != None && Shooter.Controller.ShotTarget == Pawn && GetDifficultyLevel() > DL_Casual)
	{
		AI_NearMissDistance *= 2.0;
	}

	Super.CheckNearMiss(Shooter, GearWeap, Impact, InDamageType);

	AI_NearMissDistance = OldNearMissDistance;
}

function ReceiveChargeTargetWarning(Pawn Shooter)
{
	// if we're looking in that direction and skilled enough, try to evade
	if ( (Enemy == Shooter || vector(Pawn.Rotation) dot Normal(Shooter.Location - Pawn.Location) > 0.7) &&
		FRand() < 0.25 * GearPRI(PlayerReplicationInfo).Difficulty.default.DifficultyLevel && CanEvade() )
	{
		DoEvade(GetBestEvadeDir(Shooter.Location, Shooter), true);
	}
}

function AdjustEnemyRating(out float out_Rating, Pawn EnemyPawn)
{
	Super.AdjustEnemyRating(out_Rating, EnemyPawn);

	if (out_Rating > 0.0)
	{
		GearGameMP_Base(WorldInfo.Game).AdjustEnemyRating(self, out_Rating, EnemyPawn);
	}
}

event float GetEvadeChanceScale()
{
	return 1.0;
}

function bool CanEvade(optional bool bCheckChanceToEvade, optional float InChanceToEvade = -1.f, optional float ChanceToEvadeScale = 1.f)
{
	// on casual, whenever we might evade, additional 50% chance that we don't
	if (GetDifficultyLevel() == DL_Casual)
	{
		bCheckChanceToEvade = true;
		if (InChanceToEvade > 0.0)
		{
			InChanceToEvade *= 0.5;
		}
		else
		{
			InChanceToEvade = 0.5;
		}
	}
	return Super.CanEvade(bCheckChanceToEvade, InChanceToEvade, ChanceToEvadeScale);
}

function rotator GetAccuracyRotationModifier(GearWeapon Wpn, float InaccuracyPct)
{
	// reduce accuracy of casual bots against humans slightly
	if (GetDifficultyLevel() == DL_Casual && Enemy != None && Enemy == FireTarget && PlayerController(Enemy.Controller) != None)
	{
		InaccuracyPct = FMax(InaccuracyPct, 0.15);
	}

	return Super.GetAccuracyRotationModifier(Wpn, InaccuracyPct);
}

function NotifyChangedWeapon(Weapon PrevWeapon, Weapon NewWeapon)
{
	Super.NotifyChangedWeapon(PrevWeapon, NewWeapon);

	if (GetDifficultyLevel() == DL_Casual)
	{
		// slight additional delay
		WeaponAimDelay += 0.5;
	}
}

function float AboutToFireFromOpen()
{
	local float Result;

	Result = Super.AboutToFireFromOpen();
	if (GetDifficultyLevel() == DL_Casual)
	{
		// slight additional delay
		Result += 0.5;
	}
	return Result;
}

function TransitionToSpectateFromEndOfRound()
{
	if (Pawn != None)
	{
		PawnDied(Pawn);
	}
}

state Dead
{
	function CheckCombatTransition();
}

function float GetReviveOrExecuteInterruptDist()
{
	return 512.f;
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_CoverAI_TDM'
	MeleeCommand=class'AICmd_Base_Melee_TDM'
	bDestroyOnPawnDeath=false
	bUseFireTickets=false
	bAvoidIdleMantling=false
	AI_NearMissDistance=100.0
	InUseNodeCostMultiplier=0.0 // doesn't seem useful given that we don't repath each node most of the time

	DefaultReactConditionClasses.Remove(class'AIReactCond_MeatShield')
	DefaultReactConditionClasses.Remove(class'AIReactCond_Targeted')
	// remove it from the class list so we can mess with it
	DefaultReactConditionClasses.Remove(class'AIReactCond_Stumble')
	DefaultReactConditionClasses.Remove(class'AIReactCond_DmgLeaveCover')

	Begin Object Class=AIReactCond_Stumble Name=DamageStumble0
		bIgnoreLegShotStumbles=true
	End Object
	DefaultReactConditions.Add(DamageStumble0)

	Begin Object Class=AIReactCond_DmgLeaveCover Name=DmgLeaveCover0
		bSkipIfBeatingEnemy=true
		DamageThreshPct=0.33
	End Object
	DefaultReactConditions.Add(DmgLeaveCover0)

	Begin Object Class=AIReactCond_GenericCallDelegate Name=DamageInterrupt0
		AutoSubscribeChannels[0]=Damage
		OutputFunction=InterruptAction
		bSuppressed=true
	End Object
	DamageInterruptReaction=DamageInterrupt0

	Begin Object Class=AIReactCond_GenericCallDelegate Name=DamageInterrupt1
		AutoSubscribeChannels[0]=Damage
		OutputFunction=CheckLeaderInterrupt
		bSuppressed=true
	End Object
	LeaderDamageInterruptReaction=DamageInterrupt1

	// override melee reaction
	Begin Object Class=AIReactCond_EnemyCanBeMeleed_MP Name=EnemyInMeleeRangeReaction1
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Remove(EnemyInMeleeRangeReaction0_0) // why the extra "_0"? I have no idea. But that's what it wants.
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction1)

	Begin Object Class=AIReactCond_StoppingPowerThreshold Name=StoppingPowerTemplate2
		ChanceToStumble=0.0
	End Object
	StoppingPowerReactionTemplate=StoppingPowerTemplate2

	Begin Object Class=CovGoal_GoalProximity Name=CovGoal_Proximity3
		BestGoalDist=0.0f
		MinGoalDist=0.0f
		MaxGoalDist=256.0f
		bHardLimits=true
	End Object
	Begin Object Class=Goal_AtCover Name=AtCov_Near1
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_EnemyProx0)
		CoverGoalConstraints.Add(CovGoal_Proximity3)
		CoverGoalConstraints.Add(CovGoal_MovDist0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
	End Object
	AtCover_VeryNear=AtCov_Near1

	Begin Object Class=CovGoal_GoalProximity Name=CovGoal_Proximity4
		BestGoalDist=512.0
		MinGoalDist=128.0
		MaxGoalDist=2048.0
	End Object
	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist2
		BestCoverDist=768.0f
		MinCoverDist=256.0f
		MaxCoverDist=1500.0f
		bMoveTowardGoal=true
		MinDistTowardGoal=256.f
	End object
	Begin Object Class=Goal_AtCover Name=AtCover_Toward2
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_WeaponRange0)
		CoverGoalConstraints.Add(CovGoal_MovDist2)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
		CoverGoalConstraints.Add(CovGoal_Proximity4)
	End Object
	AtCover_TowardEnemyAggressive=AtCover_Toward2

	Begin Object Name=CovGoal_MovDist1
		BestCoverDist=768.0f
		MinCoverDist=256.0f
		MaxCoverDist=1024.0f
		bMoveTowardGoal=true
		MinDistTowardGoal=256.f
	End object
	Begin Object Name=AtCover_AlongPath0
		CoverGoalConstraints.Empty()
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_MovDist1)
	End Object

	Begin Object Name=CovGoal_Proximity1
		BestGoalDist=2048.0
		MinGoalDist=1024.0
		MaxGoalDist=4096.0
	End Object
	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist3
		BestCoverDist=1024.0f
		MinCoverDist=768.0f
		MaxCoverDist=2048.0f
	End Object
	Begin Object name=AtCov_Away0
		CoverGoalConstraints.Remove(CovGoal_MovDist0_1)
		CoverGoalConstraints.Add(CovGoal_MovDist3)
	End Object
}
