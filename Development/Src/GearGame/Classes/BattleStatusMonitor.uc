/**
 * The BattleStatusMonitor is intended to be an object that keeps
 * tabs on what is happening within a battle and takes actions where appropriate,
 * such as playing certain dialogue lines or whatever.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class BattleStatusMonitor extends Actor
	native(Sound)
	notplaceable
	dependson(GUDManager);


struct native BattleStatusStruct
{
	var array<GearPawn> KnownEnemies;
	var vector ToEnemyCentroid;

	/** True if there's an e-hole or AIFactory or something spawning enemies */
	var bool bSpawnerIsActive;

	/** List of active AIfactories */
	var array<SeqAct_AIFactory> AIFactories;
};


/** Battle status from the COG pov.  Could easily extend to store status from other team povs */
var private BattleStatusStruct COGBattleStatus;

/** Quiet time required to trigger a CombatLull notices */
var() private float CombatLullNotifyTime;

/** Last time a combat lull event was fired */
var() private transient float LastCombalLullEvent;

struct native RecentEnemyNotice
{
	var GearPawn			Enemy;
	var array<GearPawn>	WhoNoticed;
};

/** enemy notice notifications that have happened recently (since last poll), one per team */
var private array<RecentEnemyNotice> RecentNoticeQueue;

/** Last time there were zero known enemies. */
var() private transient float LastZeroKnownEnemiesTime;

/** If true, skip GUDS events on next update */
var transient bool bNoGUDSNextUpdate;


simulated event PostBeginPlay()
{
	// start timer to check battle status every so often
	SetTimer( 2.f, TRUE, nameof(PollBattleStatus) );

	LastCombalLullEvent = WorldInfo.TimeSeconds;
	LastZeroKnownEnemiesTime = WorldInfo.TimeSeconds;
}

/**
 * Call this when an enemy is "noticed", so the we can do special things
 * eg dialogue lines.
 */
simulated function NotifyEnemyNoticed(Pawn Noticer, Pawn Noticee)
{
	local int Idx;
	local GearPawn WPNoticer, WPNoticee;

	WPNoticer = GearPawn(Noticer);
	WPNoticee = GearPawn(Noticee);

	// if we noticed a pawn that is not a player, or is on the magical neutral team don't call out
	if((Noticee.Controller != none && !Noticee.Controller.bIsPlayer) || Noticee.GetTeamNum() == 254)
	{
		//`log(GetFuncName()@"Bailing Early!"@Noticee@Noticee.Controller@Noticee.Controller.bIsPlayer@Noticee.GetTeamNum());
		return;
	}

	if (COGBattleStatus.KnownEnemies.Find(WPNoticee) != INDEX_NONE)
	{
		// already a known enemy, bail
		return;
	}

	// player friendlies only for now
	if ( Noticer.IsA('GearPawn_COGGear') && (WPNoticer != None) )
	{
		// noticed a previously unknown enemy, add to list
		Idx = RecentNoticeQueue.Find('Enemy', WPNoticee);

		if (Idx == -1)
		{
			// no one else has noticed this guy yet
			RecentNoticeQueue.Insert(0,1);
			RecentNoticeQueue[0].Enemy = WPNoticee;
			Idx = 0;
		}

		// someone else already noticed, add this guy to the list
		// need uniqueness check here?  think not, but...
		RecentNoticeQueue[Idx].WhoNoticed.Insert(0,1);
		RecentNoticeQueue[Idx].WhoNoticed[0] = WPNoticer;
	}
}

/**
 * Search to see if there is an active enemy spawner in the world.
 */
private simulated function CheckForActiveSpawners()
{
	local int Idx, NumFactories;

	NumFactories = COGBattleStatus.AIFactories.Length;

	COGBattleStatus.bSpawnerIsActive = FALSE;

	//`log(" *** "@GetFuncName()@"NumFactories="@NumFactories);

	for (Idx=0; Idx<NumFactories; ++Idx)
	{
		//`log("      - "@COGBattleStatus.AIFactories[Idx]@COGBattleStatus.AIFactories[Idx].bActive@COGBattleStatus.AIFactories[Idx].bAllSpawned);
		if( COGBattleStatus.AIFactories[Idx] != none
			&& COGBattleStatus.AIFactories[Idx].bActive
			&& !COGBattleStatus.AIFactories[Idx].bAllSpawned
			)
		{
			COGBattleStatus.bSpawnerIsActive = TRUE;
			break;
		}
	}
}

/** GUDS telling us that it went dark */
function NotifyGUDSDisabled()
{
	bNoGUDSNextUpdate = TRUE;
}

function PollBattleStatus()
{
	//local float Total;
	//local float AssembleKEList, recentnotices;

	// poll scene, fill out battlestatus struct
	local GearAI AIC, LocalEnemyAIC;
	local GearPC WPC;
	local int /*OldNumEnemies,*/ NewNumEnemies, EnemyIdx;
	//local EGUDEventID Event;
	local Controller C;
	local GearPawn WP;
	local float LastEnemyFireTime;
	local bool bInCombat, bAddToKnownEnemies;

	//Clock(Total);

	//CLOCK_CYCLES(recentnotices);
	// do any "I noticed something!" callouts
	// this needs to happen before searching for active spawners
	HandleRecentNotices();
	CheckForActiveSpawners();
	//UNCLOCK_CYCLES(recentnotices);

	//OldNumEnemies = COGBattleStatus.KnownEnemies.Length;

	// clear out old known enemies list
	COGBattleStatus.KnownEnemies.Length = 0;

	//`log("*** Battle Status Report");

	//CLOCK_CYCLES(AssembleKEList);
	// create global enemy list, which is union of all enemies known by all
	// COG teammates
	foreach WorldInfo.AllControllers( class'Controller', C )
	{
		AIC = GearAI_COGGear(C);

		if( AIC != None && AIC.Squad != None )
		{
			if (COGBattleStatus.KnownEnemies.Length == 0)
			{
				// this is an optimization to skip the uniqueness check on first ai encountered
				foreach AIC.Squad.AllEnemies( class'GearPawn', WP )
				{
					COGBattleStatus.KnownEnemies[COGBattleStatus.KnownEnemies.Length] = WP;
				}
			}
			else
			{
				// insert uniquely into enemy list
				foreach AIC.Squad.AllEnemies( class'GearPawn', WP )
				{
					if (COGBattleStatus.KnownEnemies.Find(WP) == -1)
					{
						COGBattleStatus.KnownEnemies.Insert(0,1);
						COGBattleStatus.KnownEnemies[0] = WP;
					}
				}
			}
		}
		else
		{
			WPC = GearPC(C);

			// need to check team here for MP purposes, sicne you can have Locust PCs in versus.
			if ( (WPC != None) && (WPC.GetTeamNum() == 0) )
			{
				// insert uniquely into enemy list
				for (EnemyIdx=0; EnemyIdx<WPC.LocalEnemies.Length; ++EnemyIdx)
				{
					WP = WPC.LocalEnemies[EnemyIdx].Enemy;

					if (WP != None)
					{
						if ( (WPC.LocalEnemies[EnemyIdx].bSeen) )
						{
							// add to known enemies if I've seen him
							bAddToKnownEnemies = TRUE;
						}
						else
						{
							LocalEnemyAIC = GearAI(WP.Controller);
							if ( (LocalEnemyAIC != None) && (LocalEnemyAIC.IsInCombat()) )
							{
								// add to known enemies if he's in combat
								bAddToKnownEnemies = TRUE;
							}
						}

						if (bAddToKnownEnemies)
						{
							if (COGBattleStatus.KnownEnemies.Find(WP) == -1)
							{
								// add him!
								COGBattleStatus.KnownEnemies.Insert(0,1);
								COGBattleStatus.KnownEnemies[0] = WP;
							}
						}
					}
				}

				// check for combat status
				bInCombat = bInCombat || WPC.IsInCombat();
			}
		}

		// while we're going thru the list, save off the most recent fire time for the locust
		if ( (GearPawn(C.Pawn) != None) && (C.GetTeamNum() != 0) )
		{
			LastEnemyFireTime = Max(LastEnemyFireTime, GearPawn(C.Pawn).LastWeaponStartFireTime);
		}
	}
	//UNCLOCK_CYCLES(AssembleKEList);

	NewNumEnemies = COGBattleStatus.KnownEnemies.Length;

	//`log("Known enemies"@OldNumEnemies@NewNumEnemies);
	//for(EnemyIdx=0; EnemyIdx<COGBattleStatus.KnownEnemies.Length; EnemyIdx++)
	//{
	//	`log("-- "@COGBattleStatus.KnownEnemies[EnemyIdx]@"spaweractive="@COGBattleStatus.bSpawnerIsActive);
	//}

	if (bInCombat)
	{
		CheckForCombatLull(LastEnemyFireTime);
	}
	else
	{
		// timer doesn't tick while not in combat
		LastCombalLullEvent = WorldInfo.TimeSeconds;
	}

	if (NewNumEnemies == 0)
	{
		LastZeroKnownEnemiesTime = WorldInfo.TimeSeconds;
	}

	bNoGUDSNextUpdate = FALSE;

	//Unclock(Total);
	//`log(GetFuncName()@"took"@Total);
	//`log("-- recentnotices"@recentnotices);
	//`log("-- AssembleKEList"@AssembleKEList);
}


/** Detect and handle combat lulls. */
function CheckForCombatLull(float LastEnemyFireTime)
{
	// spin thru Locust guys and check LastWeaponStartFireTime
	local float TimeSinceLastEnemyFire, TimeSinceLastCombatLullEvent, TestTime;

	if (!bNoGUDSNextUpdate)
	{
		TimeSinceLastEnemyFire = WorldInfo.TimeSeconds - LastEnemyFireTime;
		TimeSinceLastCombatLullEvent = WorldInfo.TimeSeconds - LastCombalLullEvent;
		TestTime = Min(TimeSinceLastEnemyFire, TimeSinceLastCombatLullEvent);

		if (TestTime > CombatLullNotifyTime)
		{
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_CombatLull, None);
			LastCombalLullEvent = WorldInfo.TimeSeconds;
		}
	}
}

function HandleRecentNotices()
{
	local int					Idx, NumRecentNotices, BestThreatScore;
	local EGUDEventID			BestThreatEvent;
	local GearPawn				Enemy, BestThreatEnemy;
	local bool					bAdds;

	// each monster type will get a unique threat score.  scores 100 or over
	// will be treated as "high threat" and are more likely to get called out by name
	// even if they appear with other enemies.
	local int Threat;

	NumRecentNotices = RecentNoticeQueue.Length;
	if (NumRecentNotices == 0)
	{
		// nothing to do, bail
		return;
	}

	// don't do notice lines in multiplayer or in a coop split (can sound funny in splits)
	if (!GearGRI(WorldInfo.GRI).IsMultiPlayerGame() && !bNoGUDSNextUpdate && !GearGame(WorldInfo.Game).bInCoopSplit)
	{
		// are these new enemies additional or reinforcements?
		bAdds = (COGBattleStatus.KnownEnemies.Length != 0);

		// no callouts if spawner is active.
		if (!COGBattleStatus.bSpawnerIsActive)
		{
			// threat scores, noted here for reference
			// kantus is		150			
			// boomer is		130
			// bloodmount is	120
			// ticker			105
			// theron is		103
			// palace guard		100
			// flamedrone		90
			// hunter is		40
			// wretch is		30
			// drone is			20

			// first, search for high-threat enemy types, call them out first
			for (Idx=0; Idx<NumRecentNotices; ++Idx)
			{
				Enemy = RecentNoticeQueue[Idx].Enemy;
				Threat = Enemy.NoticedGUDSPriority;

				if ( (Threat > BestThreatScore) && (Enemy.NoticedGUDSEvent != GUDEvent_None) )
				{
					BestThreatEnemy = Enemy;
					BestThreatScore = Threat;
					BestThreatEvent = Enemy.NoticedGUDSEvent;
				}
			}

			if (!BestThreatEnemy.bSuppressNoticedGUDSEvents)
			{	
				if ( (BestThreatEvent != GUDEvent_None) && 
					 ( (BestThreatScore >= 100) || ( !bAdds && ((Len == 1) || (FRand() > 0.5f)) ) )
				   )
				{
					// call out by name
					GearGame(WorldInfo.Game).TriggerGUDEvent(BestThreatEvent, None, BestThreatEnemy);
				}
				else if ( bAdds )
				{
					if (WorldInfo.TimeSince(LastZeroKnownEnemiesTime) > 12.f)
					{
						// reinforcements!
						GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_NoticedReinforcements, None, BestThreatEnemy);
					}
				}
				else if ( (NumRecentNotices > 2) && (FRand() > 0.5f) )
				{
					GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_NoticedManyEnemies, None, BestThreatEnemy);
				}
				else
				{
					// generic notice
					GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_NoticedEnemyGeneric, None, BestThreatEnemy);
				}
			}
		}
	}

	RecentNoticeQueue.Length = 0;
}

/** Let BSM know that an AI factory has been activated */
event RegisterAIFactory(SeqAct_AIFactory Factory)
{
	local int Idx;
	Idx = COGBattleStatus.AIFactories.Find(Factory);
	if (Idx == INDEX_NONE)
	{
//		`log(" *** Registered Factory"@Factory);
		COGBattleStatus.AIFactories[COGBattleStatus.AIFactories.Length] = Factory;
	}
}

/** Let BSM know that an AI factory has been deactivated */
event UnRegisterAIFactory(SeqAct_AIFactory Factory)
{
	local int Idx;
	Idx = COGBattleStatus.AIFactories.Find(Factory);
	if (Idx != INDEX_NONE)
	{
//		`log(" *** UNRegistered Factory"@Factory);
		COGBattleStatus.AIFactories.Remove(Idx,1);
	}
}

/** flush out known state.  used on checkpoint loads & level transitions */
function Flush()
{
	COGBattleStatus.bSpawnerIsActive = FALSE;
	COGBattleStatus.KnownEnemies.Length = 0;
	COGBattleStatus.AIFactories.Length = 0;
	bNoGUDSNextUpdate = TRUE;	// just in case
}


defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	CombatLullNotifyTime=8.f
}

