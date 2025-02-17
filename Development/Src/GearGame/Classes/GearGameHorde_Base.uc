/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameHorde_Base extends GearGameMP_Base
	dependson(SeqAct_AIFactory)
	dependson(GearPawn)
	abstract;

/** information on one enemy character that may be chosen to participate in a wave */
struct EnemySpawnInfo
{
	/** enemy type to spawn - we use the AIFactory enum to take advantage of the convenient list there */
	var EAITypes EnemyType;
	/** class names of inventory to give the enemy - optional, overrides the default if specified (class name only - no package) */
	var array<name> InventoryOverrides;
	/** optional combat mood that should be applied to the AI */
	var ECombatMood CombatMood;
	/** players must have reached this wave before this enemy appears */
	var int MinWave;
	/** max wave this enemy type is allowed to spawn in, 0 for any wave */
	var int MaxWave;
	/** minimum amount of this enemy we should try to keep around (while wave points are available) */
	var int MinCount;
	/** maximum amount of this enemy we can have around at once */
	var int MaxCount;
	/** how many wave points this enemy costs */
	var int WaveCost;
};
var const config array<EnemySpawnInfo> EnemyList;
/** a group of enemies defined in EnemyList that may spawn as a group */
struct EnemySquadInfo
{
	/** indices into the EnemyList */
	var array<int> EnemyIndices;
	/** minimum wave this squad may appear in - must be larger than the highest of the enemies that are in the squad */
	var int MinWave;
	/** total cost of this squad. NOT FOR CONFIGURING */
	var int WaveCost;
};
var config array<EnemySquadInfo> EnemySquadList;
/** whenever we might spawn a solo enemy, this is the chance a squad will be set up instead */
var config float SquadChance;
/** maximum number of waves (players win if they get past this) */
var config int MaxWaves;
/** number of "wave points" used for spawning enemies. Total points for a wave is (PointsPerWave * CurrentWave) */
var config int PointsPerWave;

/** current wave we are on */
var int CurrentWave;
/** remaining wave points left to use on spawning enemies */
var int WavePointsRemaining;
/** wave points that are still alive
 * this is different from points remaining in that it includes the value of enemies that have been spawned but not yet killed
 */
var int WavePointsAlive;
/** count of currently alive Pawns in each EnemyList slot */
var array<int> EnemyListAliveCount;
/** index into EnemySquadList if we are currently spawning enemies from a squad (if not, will be INDEX_NONE) */
var int CurrentSquadIndex;
/** enemy within the squad indicated by CurrentSquadIndex that will be spawned next (invalid if CurrentSquadIndex == INDEX_NONE) */
var int NextSquadEnemyIndex;
/** whether we have triggered the end of round suicidal rush */
var bool bTriggeredBanzai;

/** last NavigationPoint we spawned an enemy at (to spawn them in clumps) */
var NavigationPoint LastEnemyStartSpot;
/** size of enemy we're attempting to spawn (extra data for RatePlayerStart()) */
var Cylinder SpawningEnemySize;

/** class to spawn as flamethrower to combat the Berserker */
var class<GearWeapon> FlameThrowerClass;

var float WaveSpawningStartTime;

/** a modifier applied when wrapping around to an "extended" wave */
struct ExtendedMutator
{
	/** mutator class - tried first */
	var string MutClassPath;
	/** game rules class - only spawned if MutClassPath is empty */
	var string RulesClassPath;
	/** multiplier to pass to the class spawned */
	var float Multiplier;
};
/** wrapper struct so we can have an array of arrays */
struct ExtendedWaveInfo
{
	var array<ExtendedMutator> ExtendedMutators;
};
/** if the players win, restart from wave 1 and apply the mutators from the next element in this list */
var config array<ExtendedWaveInfo> ExtendedWaveList;

/** The stats read object used to get the player's high score */
var GearLeaderboardHorde HordeStats;

/** the difficulty level for the enemies */
var class<DifficultySettings> EnemyDifficulty;

/** number of wave points for an enemy considered the baseline for damage/death scoring
 * i.e. an enemy worth this many wave points is scored as an MP player, with other values being scaled proportionally
 */
var config int ScoringBaselineWavePoints;

/** percentage of total wave score given as a bonus to players based on how many of them survived */
var config float SurvivalBonusPct;

/** highest "checkpoint" wave reached, for "restart from checkpoint" functionality */
var int LastCheckpointWave;

var int MaxAlive;


event PreBeginPlay()
{
	Super.PreBeginPlay();

	DoGameSpecificPerformanceSettings( WorldInfo );
}


/** This allows us to have a generic interface which GameReplicationInfo can call when receiving the GameClass to set and performance settings per game type **/
static simulated function DoGameSpecificPerformanceSettings( WorldInfo TheWorldInfo )
{
	TheWorldInfo.bAllowModulateBetterShadows = FALSE;
}


/**
* Set the game rules via the online game settings
* @return - whether the game was able to set the rules via the gamesettings object
*/
function bool SetGameRules()
{
	local int IntValue;
	local GearVersusGameSettings MyGameSettings;

	MyGameSettings = GetGameSettingsObject();

	if (MyGameSettings != None)
	{
		Super.SetGameRules();

		// Enemy AI Difficulty
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_ENEMYDIFFICULTY, IntValue) )
		{
			EnemyDifficulty = DifficultyLevels[IntValue];
			InitialRevivalTime = EnemyDifficulty.default.HordeBleedOutTime;
		}

		// Starting wave
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_HORDE_WAVE, IntValue) )
		{
			CurrentWave = IntValue;
		}

		return true;
	}
	else
	{
		return FALSE;
	}
}

/** bots not supported in Horde */
function SetNumBots();

event InitGame( string Options, out string ErrorMessage )
{
	local int i, j;

	EnemyListAliveCount.length = EnemyList.length;

	// validate enemy squads
	for (i = 0; i < EnemySquadList.length; i++)
	{
		EnemySquadList[i].WaveCost = 0;
		for (j = 0; j < EnemySquadList[i].EnemyIndices.length; j++)
		{
			EnemySquadList[i].MinWave = Max(EnemySquadList[i].MinWave, EnemyList[EnemySquadList[i].EnemyIndices[j]].MinWave);
			EnemySquadList[i].WaveCost += EnemyList[EnemySquadList[i].EnemyIndices[j]].WaveCost;
		}
	}

	Super.InitGame(Options, ErrorMessage);

	if (HasOption(Options, "WAVE"))
	{
		CurrentWave = int(ParseOption(Options, "WAVE")) - 1;
	}

	RequestedBots = 0; // not supported
	TimeLimit = 0;
	bTourist = false;
}

/** Overloaded to set the total waves so the scoreboard has access to this value before the game starts */
function InitGameReplicationInfo()
{
	Super.InitGameReplicationInfo();

	GearGRI.InvasionNumTotalWaves = MaxWaves;
	GearGRI.EnemyDifficulty = EnemyDifficulty;
	// Set this back to whatever is in the INI file
	EndOfGameDelay = default.EndOfGameDelay;
}

/** Only half the max is allowed for invasion. */
function bool AtCapacity(bool bSpectator)
{
	return NumPlayers >= (MaxPlayers/2);
}

final function StartNextWave()
{
	local Controller C;
	local int i;
	local class<GearHordeExtendedMutator> MutClass;
	local class<GearHordeExtendedGameRules> RulesClass;
	local Mutator M;
	local GameRules G;
	local GearHordePC PC;
	local bool bDidExtendedRestart;

	WaveSpawningStartTime = WorldInfo.TimeSeconds + 15.f;

	CurrentWave++;
	while (CurrentWave > MaxWaves && ExtendedWaveList.length > 0)
	{
		// add extended mutators/rules classes with starting zero modifier
		for (i = 0; i < ExtendedWaveList[0].ExtendedMutators.length; i++)
		{
			if (ExtendedWaveList[0].ExtendedMutators[i].MutClassPath != "")
			{
				MutClass = class<GearHordeExtendedMutator>(FindObject(ExtendedWaveList[0].ExtendedMutators[i].MutClassPath, class'Class'));
				if (MutClass != None)
				{
					// see if it already exists
					for (M = BaseMutator; M != None; M = M.NextMutator)
					{
						if (M.Class == MutClass)
						{
							break;
						}
					}
					if (M == None)
					{
						// add it
						M = Spawn(MutClass);
						if (BaseMutator == None)
						{
							BaseMutator = M;
						}
						else
						{
							BaseMutator.AddMutator(M);
						}
					}
					GearHordeExtendedMutator(M).SetMultiplier(ExtendedWaveList[0].ExtendedMutators[i].Multiplier);
				}
				else
				{
					`Warn("Couldn't find extended mutator" @ ExtendedWaveList[0].ExtendedMutators[i].MutClassPath @ "in memory!");
				}
			}
			else
			{
				RulesClass = class<GearHordeExtendedGameRules>(FindObject(ExtendedWaveList[0].ExtendedMutators[i].RulesClassPath, class'Class'));
				if (RulesClass != None)
				{
					// see if it already exists
					for (G = GameRulesModifiers; G != None; G = G.NextGameRules)
					{
						if (G.Class == RulesClass)
						{
							break;
						}
					}
					if (G == None)
					{
						// add it
						G = Spawn(RulesClass);
						if (GameRulesModifiers == None)
						{
							GameRulesModifiers = G;
						}
						else
						{
							GameRulesModifiers.AddGameRules(G);
						}
					}
					GearHordeExtendedGameRules(G).SetMultiplier(ExtendedWaveList[0].ExtendedMutators[i].Multiplier);
				}
				else
				{
					`Warn("Couldn't find extended game rules" @ ExtendedWaveList[0].ExtendedMutators[i].RulesClassPath @ "in memory!");
				}
			}
		}

		ExtendedWaveList.Remove(0, 1);
		GearGRI.ExtendedRestartCount++;
		bDidExtendedRestart = true;
		CurrentWave -= MaxWaves;
	}
	if (bDidExtendedRestart)
	{
		GearGRI.CheckHordeMessage();
	}
	if (CurrentWave > MaxWaves)
	{
		// ran out of mutators
		CurrentWave = CurrentWave % MaxWaves;
	}

	LastCheckpointWave = CurrentWave + (GearGRI.ExtendedRestartCount * MaxWaves);

	GearGRI.RoundCount = CurrentWave;
	GearGRI.InvasionCurrentWaveIndex = CurrentWave;

	WavePointsRemaining = CurrentWave * PointsPerWave;
	WavePointsAlive = WavePointsRemaining;
	GearGRI.WavePointsAlivePct = 255;
	CurrentSquadIndex = INDEX_NONE;
	NextSquadEnemyIndex = INDEX_NONE;
	bTriggeredBanzai = false;

	// respawn all players
	foreach Teams[0].TeamMembers(C)
	{
		//@FIXME: why does state 'Reviving' return true for IsDead()? Scared to change that this late...
		if (!C.PlayerReplicationInfo.bOnlySpectator && (C.IsDead() || C.IsSpectating()) && !C.IsInState('Reviving'))
		{
			// make sure functions aren't blocked due to spectating
			if (PlayerController(C) != None)
			{
				PlayerController(C).GotoState('Dead');
				PlayerController(C).ClientGotoState('Dead');
			}
			Global.RestartPlayer(C);
		}
	}

	foreach WorldInfo.AllControllers(class'GearHordePC',PC)
	{
		// Update the rich presence with the wave number
		PC.ClientSetHordePresence(LastCheckpointWave);
	}

	RandomizeWeaponPickupFactories();
	SetTimer(15.0, false, nameof(ResetLevelForNewRoundTimed));

	GotoState('MatchInProgress');
}

final function ResetLevelForNewRoundTimed()
{
	//@hack: don't destroy shields that were spawned as part of the wave initialization/reset
	// time needs to be greater than the timer rate for this function, above
	ResetLevelForNewRound(16.0);
}

function PostLogin(PlayerController NewPlayer)
{
	Super.PostLogin(NewPlayer);

	// force them to the COG team
	ChangeTeam( NewPlayer, 0, false );

	// if a difficulty was specified in the UI, use it
	// otherwise, use the difficulty the player specified on the URL
	if (EnemyDifficulty == None)
	{
		EnemyDifficulty = GearPRI(NewPlayer.PlayerReplicationInfo).Difficulty;
		GearGRI.EnemyDifficulty = EnemyDifficulty;
		InitialRevivalTime = EnemyDifficulty.default.HordeBleedOutTime;
		GearGRI.InitialRevivalTime = InitialRevivalTime;
	}
	else
	{
		GearPRI(NewPlayer.PlayerReplicationInfo).Difficulty = EnemyDifficulty;
	}
}

event HandleSeamlessTravelPlayer(out Controller NewPlayer)
{
	Super.HandleSeamlessTravelPlayer(NewPlayer);

	// if a difficulty was specified in the UI, use it
	// otherwise, use the difficulty the player specified on the URL
	if (EnemyDifficulty == None)
	{
		EnemyDifficulty = GearPRI(NewPlayer.PlayerReplicationInfo).Difficulty;
		GearGRI.EnemyDifficulty = EnemyDifficulty;
	}
	else
	{
		GearPRI(NewPlayer.PlayerReplicationInfo).Difficulty = EnemyDifficulty;
	}
}

function bool AllowHealthRecharge(GearPawn Pawn)
{
	// locust don't recharge in Invasion
	return Super(GearGame).AllowHealthRecharge(Pawn);
}

function StartMatch()
{
	// Again, make sure all the teams are seeded
	//AutoSeedTeams();

	// Read the Horde stats for this map
	ReadHordeStats();

	Super.StartMatch();
}

function AutoSeedTeams()
{
	local controller c;

	foreach WorldInfo.AllControllers(class'Controller',c)
	{
		if ( C.PlayerReplicationInfo != none )
		{
			// Force everyone to be ready to play
			C.PlayerReplicationInfo.bReadyToPlay = true;
			if ( C.PlayerReplicationInfo.Team == none )
			{
				if ( GearPC(C) != none && !GearPC(C).bDedicatedServerSpectator )
				{
					ChangeTeam(C, 0, false);
				}
			}
		}
	}
}

function bool ChangeTeam(Controller Other, int N, bool bNewTeam)
{
	// check to see if we need to initialize teams
	if (Teams.Length < NumTeams)
	{
		InitializeTeams();
	}

	if ( Other != None )
	{
		N = GetForcedTeam(Other,N);
		// if not already on that team
		if ( N == 255 || Other.PlayerReplicationInfo.Team == None || Other.PlayerReplicationInfo.Team != Teams[N] )
		{
			// remove from their current team
			if ( Other.PlayerReplicationInfo.Team != None )
			{
				Other.PlayerReplicationInfo.Team.RemoveFromTeam(Other);
			}

			// We only deal with guys on teams 0 = COG and team 1 = AI Enemies
			if ( N == 0 || N == 1 )
			{
				// and attempt to add them to their new team
				if ( Teams[N].AddToTeam(Other) )
				{
					// Only set the player class for COG
					if ( N == 0 )
					{
						SetPlayerClass(Other,N);
					}
					return true;
				}
				return  false;
			}
			else
			{
				Other.PlayerReplicationInfo.Team = none;
				GearPRI(Other.PlayerReplicationInfo).PawnClass=none;
				GearPRI(Other.PlayerReplicationInfo).bIsLeader=false;
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	return false;
}

/** Returns the index of the team that should receive the next available player */
function int GetForcedTeam(Controller Other, int Team)
{
	if ( Team == 255 )
	{
		return 0;
	}
	return Team;
}

function bool AllowNetworkPause()
{
	// allow server to pause in private/system link Horde games
	return (GameInterface.GetGameSettings('Game') == None || !GameInterface.GetGameSettings('Game').bUsesArbitration);
}

`if(`notdefined(SHIPPING_PC_GAME))
/** adds some bots to fill out the team */
exec function Friends()
{
	local int Count;
	local GearAI_TDM AI;

	Count = 5 - NumPlayers - NumBots;
	while (Count > 0)
	{
		AddBot();
		Count--;
	}

	// allow the AI to spawn even if the match is in progress
	if (IsInState('MatchInProgress'))
	{
		foreach WorldInfo.AllControllers(class'GearAI_TDM', AI)
		{
			if (AI.Squad == None)
			{
				Teams[0].CreateMPSquads();
			}
			Global.RestartPlayer(AI);
		}
	}
}
`endif

final function float RateEnemyStart(NavigationPoint N, bool bDoTraceCheck)
{
	local float Score, NextDist;
	local Controller OtherPlayer;
	local PlayerStart P;

	// abort if enemy can't fit
	if (N.MaxPathSize.Radius < SpawningEnemySize.Radius || N.MaxPathSize.Height < SpawningEnemySize.Height || N.bBlocked)
	{
		return -1;
	}

	// ignore disabled PlayerStarts
	P = PlayerStart(N);
	if (P != None && !P.bEnabled)
	{
		return -1;
	}

	Score = (P != None && P.bPrimaryStart) ? 10000000 : 5000000;
	Score += 10000.0 * FRand();

	foreach WorldInfo.AllControllers(class'Controller', OtherPlayer)
	{
		if (OtherPlayer.bIsPlayer && OtherPlayer.Pawn != None)
		{
			NextDist = VSize(OtherPlayer.Pawn.Location - N.Location);
			if (NextDist < 2 * (OtherPlayer.Pawn.CylinderComponent.CollisionRadius + OtherPlayer.Pawn.CylinderComponent.CollisionHeight))
			{
				Score -= (1000000.0 - NextDist);
			}
			else if (NextDist < 3000.0 && OtherPlayer.GetTeamNum() == 0)
			{
				if (bDoTraceCheck && FastTrace(N.Location, OtherPlayer.Pawn.Location))
				{
					Score -= (20000.0 - NextDist);
				}
				if (NextDist < 1500.0)
				{
					Score -= (10000.0 - NextDist * 3.0);
				}
			}
		}
	}
	return FMax(Score, 5);
}

function float RatePlayerStart(PlayerStart P, byte Team, Controller Player)
{
	local float Rating, Dist;
	local Controller C;

	if (Player != None)
	{
		if (CurrentWave == 0 || IsInState('PreRound'))
		{
			return Super.RatePlayerStart(P, Team, Player);
		}
		else if (P == None || !P.bEnabled || P.PhysicsVolume.bWaterVolume)
		{
			return -10000000;
		}

		//assess candidate
		Rating = P.bPrimaryStart ? 10000000 : 5000000;
		Rating += 1000.0 * FRand();
		// rate based on distance from friendlies
		foreach WorldInfo.AllControllers(class'Controller', C)
		{
			if (C != Player && C.Pawn != None && C.GetTeamNum() == Team)
			{
				Dist = VSize(C.Pawn.Location - P.Location);
				if (Dist < 2.0 * C.Pawn.GetCollisionRadius())
				{
					// don't want to spawn on top of someone unless we have to
					Rating -= 50000.0;
				}
				else
				{
					Rating -= Dist;
				}
			}
		}

		return Rating;
	}
	else
	{
		return RateEnemyStart(P, true);
	}
}

/** consider selecting and starting the creation of a squad of enemies instead of completely random ones */
function MaybeStartSquad()
{
	local int i;
	local array<int> ValidChoices;

	if (Teams[0].TeamMembers.Length >= 3 && CurrentSquadIndex == INDEX_NONE && FRand() < SquadChance)
	{
		for (i = 0; i < EnemySquadList.length; i++)
		{
			if (CurrentWave >= EnemySquadList[i].MinWave && WavePointsRemaining >= EnemyList[i].WaveCost)
			{
				ValidChoices.AddItem(i);
			}
		}
		if (ValidChoices.length > 0)
		{
			CurrentSquadIndex = Rand(ValidChoices.length);
			NextSquadEnemyIndex = 0;
		}
	}
}

/** if the passed in pawn class is a unique type, check that there isn't already one in the level
 * @return whether it's ok to spawn a Pawn of the passed in class
 */
final function bool CheckUniqueness(class<Pawn> PawnClass)
{
	local Pawn P;

	// currently only reavers are unique
	if (ClassIsChildOf(PawnClass, class'Vehicle_Reaver_Base'))
	{
		foreach WorldInfo.AllPawns(PawnClass, P)
		{
			if (P.IsA(PawnClass.Name))
			{
				return false;
			}
		}
	}

	return true;
}

/** randomly selects and spawns enemy characters
 * @param MaxNum - maximum number of enemies to spawn (unless we run out of wave points early)
 */
final function SpawnEnemies(int MaxNum)
{
	local array<int> ValidChoices;
	local int i, Index;
	local class<Pawn> PawnClass;
	local class<GearAI> ControllerClass;
	local NavigationPoint StartSpot;
	local Pawn P;
	local GearAI AI;
	local name SquadName;
	local class<Inventory> InvClass;
	local Vehicle_Reaver_Base Reaver;
	local HordeReaverStart ReaverStart;
	local float Rating, BestRating;
	local GearPRI PRI;

	// construct a list of elements that are valid for selection
	for (i = 0; i < EnemyList.length; i++)
	{
		if ( CurrentWave >= EnemyList[i].MinWave && WavePointsRemaining >= EnemyList[i].WaveCost &&
			(EnemyList[i].MaxWave == 0 || CurrentWave <= EnemyList[i].MaxWave) &&
			(EnemyList[i].MaxCount == 0 || EnemyListAliveCount[i] < EnemyList[i].MaxCount) )
		{
			if (EnemyList[i].MinCount > 0 && EnemyListAliveCount[i] < EnemyList[i].MinCount)
			{
				// force this enemy to spawn because we don't have enough alive
				ValidChoices.length = 1;
				ValidChoices[0] = i;
				break;
			}
			else
			{
				ValidChoices.AddItem(i);
			}
		}
	}

	// the bad guys can spawn from any side, but each group always spawns together
	while (MaxNum > 0 && WavePointsRemaining > 0)
	{
		if (CurrentSquadIndex != INDEX_NONE)
		{
			// choose the next squad enemy
			Index = EnemySquadList[CurrentSquadIndex].EnemyIndices[NextSquadEnemyIndex];
			//@note: we don't retry on failure of a squad member enemy so that we can't get stuck on one
			//	that isn't spawning correctly for whatever reason
			NextSquadEnemyIndex++;
			if (NextSquadEnemyIndex >= EnemySquadList[CurrentSquadIndex].EnemyIndices.length)
			{
				// this squad is now complete
				CurrentSquadIndex = INDEX_NONE;
			}
		}
		else
		{
			// if there are no choices, but we still have wave points, just pick the first one in the list to use up the remaining amount
			if (ValidChoices.length == 0)
			{
				Index = 0;
			}
			else
			{
				// choose one at random
				Index = ValidChoices[Rand(ValidChoices.length)];
			}
		}
		// find the appropriate classes
		// try both GearGame and GearGameContent for Pawns
		PawnClass = class<Pawn>(FindObject("GearGameContent." $ class'SeqAct_AIFactory'.default.SpawnInfo[EnemyList[Index].EnemyType].PawnClassName, class'Class'));
		if (PawnClass == None)
		{
			PawnClass = class<Pawn>(FindObject("GearGame." $ class'SeqAct_AIFactory'.default.SpawnInfo[EnemyList[Index].EnemyType].PawnClassName, class'Class'));
		}
		ControllerClass = class<GearAI>(FindObject("GearGame." $ class'SeqAct_AIFactory'.default.SpawnInfo[EnemyList[Index].EnemyType].ControllerClassName, class'Class'));
		if (PawnClass == None || ControllerClass == None)
		{
			`Warn("Failed to find Pawn or Controller class for enemy type" @ EnemyList[Index].EnemyType);
		}
		else if (CheckUniqueness(PawnClass))
		{
			// spawn it
			StartSpot = None;
			// special case for Reaver
			if (ClassIsChildOf(PawnClass, class'Vehicle_Reaver_Base'))
			{
				BestRating = 0.0;
				foreach WorldInfo.AllNavigationPoints(class'HordeReaverStart', ReaverStart)
				{
					Rating = RateEnemyStart(ReaverStart, true);
					if (Rating > BestRating)
					{
						StartSpot = ReaverStart;
					}
				}
			}
			if (StartSpot == None)
			{
				SpawningEnemySize.Radius = PawnClass.default.CylinderComponent.CollisionRadius;
				SpawningEnemySize.Height = PawnClass.default.CylinderComponent.CollisionHeight;
				// try to spawn enemies close to each other if possible
				if (LastEnemyStartSpot != None)
				{
					for (i = 0; i < LastEnemyStartSpot.PathList.length; i++)
					{
						if ( LastEnemyStartSpot.PathList[i].End.Actor != None &&
							RateEnemyStart( NavigationPoint(LastEnemyStartSpot.PathList[i].End.Actor), false) > 1000.0 )
						{
							StartSpot = NavigationPoint(LastEnemyStartSpot.PathList[i].End.Actor);
							break;
						}
					}
				}
			}
			if (StartSpot == None)
			{
				StartSpot = FindPlayerStart(None);
			}
			if (StartSpot == None)
			{
				`Warn("Failed to find start spot for enemy");
			}
			else
			{
				LastEnemyStartSpot = StartSpot;
				P = Spawn(PawnClass,,, StartSpot.Location, StartSpot.Rotation);
				if (P == None)
				{
					`Warn("Failed to spawn" @ PawnClass @ "at" @ StartSpot);
				}
				else
				{
					// assign point value
					if (GearPawn(P) != None)
					{
						GearPawn(P).HordeEnemyIndex = Index;
					}
					else if (GearVehicle(P) != None)
					{
						GearVehicle(P).HordeEnemyIndex = Index;
					}
					EnemyListAliveCount[Index]++;
					// special case Reaver to make it land immediately
					Reaver = Vehicle_Reaver_Base(P);
					if (Reaver != None)
					{
						Reaver.ServerBeginLanding(StartSpot);
					}
					// give inventory
					if (EnemyList[Index].InventoryOverrides.length == 0)
					{
						P.AddDefaultInventory();
					}
					else
					{
						for (i = 0; i < EnemyList[Index].InventoryOverrides.length; i++)
						{
							InvClass = class<Inventory>(FindObject("GearGame." $ EnemyList[Index].InventoryOverrides[i], class'Class'));
							if (InvClass == None)
							{
								InvClass = class<Inventory>(FindObject("GearGameContent." $ EnemyList[Index].InventoryOverrides[i], class'Class'));
							}
							if (InvClass != None)
							{
								P.CreateInventory(InvClass);
							}
							else
							{
								`Warn("Failed to find inventory item in memory:" @ EnemyList[Index].InventoryOverrides[i]);
							}
						}
					}
					// spawn AI
					AI = Spawn(ControllerClass);
					ChangeTeam(AI, 1, false);
					AI.Possess(P, false);
					if (SquadName == 'None')
					{
						//@note: use of Rand() here instead of incremental is intentional...
						//	might be cool to have some existing baddies sometimes run back to reinforcements
						//	due to their squad leader getting assigned to a new spawn
						SquadName = name("Horde" $ Rand(10));
						AI.SetSquadName(SquadName, true);
					}
					else
					{
						AI.SetSquadName(SquadName, false);
					}
					// make AI move to players if there is nothing else to do
					AI.bIdleMoveToNearestEnemy = true;
					// set mood if specified
					if (EnemyList[Index].CombatMood != AICM_None && EnemyList[Index].CombatMood != AI.CombatMood)
					{
						AI.SetCombatMood(EnemyList[Index].CombatMood);
					}

					// set enemy scoring values
					PRI = GearPRI(AI.PlayerReplicationInfo);
					if (PRI != None)
					{
						PRI.ScaleScoringValues(float(EnemyList[Index].WaveCost) / float(ScoringBaselineWavePoints));
					}

					WavePointsRemaining = Max(0, WavePointsRemaining - EnemyList[Index].WaveCost);
				}
			}
		}
		// we decrease the amount even if we failed so we can't get into infinite recursion
		MaxNum--;
	}
}

function ProcessServerTravel(string URL, optional bool bAbsolute)
{
	while (Teams[1].TeamMembers.length > 0)
	{
		if (Teams[1].TeamMembers[0] == None || Teams[1].TeamMembers[0].bDeleteMe)
		{
			Teams[1].TeamMembers.Remove(0, 1);
		}
		else
		{
			Teams[1].TeamMembers[0].Destroy();
		}
	}

	Super.ProcessServerTravel(URL, bAbsolute);
}

function NotifyDBNO(Controller InstigatedBy, Controller Victim, Pawn DownedPawn)
{
	local int i;
	local GearPawn P;

	Super.NotifyDBNO(InstigatedBy, Victim, DownedPawn);

	// if all the players are DBNO, kill them so the game ends immediately
	if (DownedPawn != None && DownedPawn.GetTeamNum() == 0 && AllPlayersAreDBNO())
	{
		for (i = 0; i < Teams[0].TeamMembers.length; i++)
		{
			if (Teams[0].TeamMembers[i] != None)
			{
				P = GearPawn(Teams[0].TeamMembers[i].Pawn);
				if (P != None && P.IsDBNO())
				{
					P.Suicide();
				}
			}
		}
	}
}

function GearBroadcastWeaponTakenMessage( Controller WeaponTaker, class<DamageType> damageType )
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		PC.AddWeaponTakenMessage( WeaponTaker.PlayerReplicationInfo, damageType );
	}
}

/** updates enemy count in the GRI for display on clients' HUD */
final function UpdateEnemyCount()
{
	local GearPC PC;
	local GearHUDInvasion_Base InvHUD;
	local int DisplayNum;

	// 255 is a special value indicating "don't display this now"
	DisplayNum = (WavePointsRemaining <= 0 && Teams[1].TeamMembers.length <= 5) ? Teams[1].TeamMembers.length : 255;
	if (GetGRI().EnemiesLeftThisRound != DisplayNum)
	{
		GetGRI().EnemiesLeftThisRound = DisplayNum;

		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			InvHUD = GearHUDInvasion_Base(PC.myHUD);
			if (InvHUD != None)
			{
				InvHUD.EnemyCountChanged();
			}
		}
	}
}

state MatchInProgress
{
	event BeginState(name PrevStateName)
	{
		local Controller Player;

		// start first wave from here, subsequent waves started from RoundOver
		if (CurrentWave == 0 || PrevStateName == 'PreRound')
		{
			StartNextWave();
		}

		Super.BeginState(PrevStateName);

		foreach WorldInfo.AllControllers(class'Controller', Player)
		{
			if ( Player.PlayerReplicationInfo != None )
			{
				GearPRI(Player.PlayerReplicationInfo).ClearDeathVariables();
			}
		}
	}

	/** @return whether there is only one player remaining alive */
	final function bool OnlyOnePlayerAlive()
	{
		local int Count;
		local Controller C;

		foreach Teams[0].TeamMembers(C)
		{
			if (PlayerIsAlive(C, None))
			{
				Count++;
				if (Count > 1)
				{
					return false;
				}
			}
		}

		return (Count == 1);
	}

	event Timer()
	{
		Super.Timer();

		// normal max
		MaxAlive = 12;

		// reduce the max alive if less than 5 players
		if (Teams[0].TeamMembers.length < 5)
		{
			MaxAlive -= (5 - Teams[0].TeamMembers.length) * 2;
		}

		// if the host is low performance prevent extra spawns temporarily
		if (WorldInfo.bDropDetail)
		{
			MaxAlive = Min(MaxAlive, 9);
		}

		if (CurrentWave > 0 && WorldInfo.TimeSeconds > WaveSpawningStartTime)
		{
			if (WavePointsRemaining > 0)
			{
				if (Teams[1].TeamMembers.length < 5 || (Teams[1].TeamMembers.length < MaxAlive && FRand() < 0.1 + (WorldInfo.TimeSeconds - WaveSpawningStartTime) * 0.001))
				{
					// spawn some baddies
					GotoState(, 'DoSpawns');
				}
			}
			else if (Teams[1].TeamMembers.length == 0)
			{
				// all of the enemies have been killed
				GameReplicationInfo.Winner = Teams[0];

				WavePointsAlive = 0;
				GearGRI.WavePointsAlivePct = 0;
				if (CurrentWave >= MaxWaves && ExtendedWaveList.length == 0)
				{
					// the players have won the match
					GotoState('MatchOver');
				}
				else
				{
					GotoState('RoundOver');
				}
			}
			else if (!bTriggeredBanzai && ShouldBanzai())
			{
				`log("banzai!");
				class'AICmd_Attack_Banzai'.static.EveryoneAttack(WorldInfo, 1);
				bTriggeredBanzai = true;
			}

			UpdateEnemyCount();
		}
	}

	function bool ShouldBanzai()
	{
		local Controller C;
		local int COGAlive, LocustAlive;
		// tally the alive members for both teams
		foreach Teams[0].TeamMembers(C)
		{
			if (PlayerIsAlive(C,None))
			{
				COGAlive++;
			}
		}
		foreach Teams[1].TeamMembers(C)
		{
			if (PlayerIsAlive(C,None))
			{
				LocustAlive++;
			}
		}
		// if number of alive locust is <= alive cog
		if (LocustAlive <= COGAlive)
		{
			return TRUE;
		}
		// if only one player is alive (when there are more than two total players)
		if (COGAlive == 1 && Teams[0].TeamMembers.Length >= 2)
		{
			return TRUE;
		}
		return FALSE;
	}

	function bool MatchEndCondition(Controller Killed)
	{
		local int Idx;
		local Controller Player;

		// continue as long as any COG player is left alive
		if (Teams[0] != None)
		{
			for (Idx = 0; Idx < Teams[0].TeamMembers.Length; Idx++)
			{
				Player = Teams[0].TeamMembers[Idx];
				if (PlayerIsAlive(Player, Killed))
				{
					return false;
				}
			}
		}

		return true;
	}

	function bool CheckEndMatch(optional Controller Killed)
	{
		bLastKillerSet = false;

		if (MatchEndCondition(Killed))
		{
			GameReplicationInfo.Winner = Teams[1];
			GotoState('MatchOver');
			return true;
		}
		else
		{
			return false;
		}
	}

	function Killed(Controller Killer, Controller KilledPlayer, Pawn KilledPawn, class<DamageType> DamageType)
	{
		local GearPawn P;
		local GearVehicle V;
		local GearPRI PRI;
		local int EnemyIndex;

		Super.Killed(Killer, KilledPlayer, KilledPawn, DamageType);

		if (KilledPlayer != None && KilledPlayer.GetTeamNum() == 1)
		{
			if (Killer != None)
			{
				PRI = GearPRI(Killer.PlayerReplicationInfo);
				if (PRI != None)
				{
					PRI.ScoreGameSpecific1('Wave_Kills', "Kills this wave", 0, TRUE);
				}
			}
			P = GearPawn(KilledPawn);
			V = GearVehicle(KilledPawn);
			EnemyIndex = INDEX_NONE;
			if (P != None)
			{
				EnemyIndex = P.HordeEnemyIndex;
			}
			else if (V != None)
			{
				EnemyIndex = V.HordeEnemyIndex;
			}
			if (EnemyIndex != INDEX_NONE)
			{
				WavePointsAlive = Max(0, WavePointsAlive - EnemyList[EnemyIndex].WaveCost);
				EnemyListAliveCount[EnemyIndex]--;
				GearGRI.WavePointsAlivePct = FloatToByte(float(WavePointsAlive) / float(CurrentWave * PointsPerWave), false);
				GearGRI.bForceNetUpdate = true;
			}
		}
	}
// used to spread out spawning of enemies to reduce hitches
DoSpawns:
	LastEnemyStartSpot = None;
	MaybeStartSquad();
	if (CurrentSquadIndex != INDEX_NONE)
	{
		do
		{
			SpawnEnemies(1);
			UpdateEnemyCount();
			Sleep(0.05);
		} until (CurrentSquadIndex == INDEX_NONE);
	}
	else
	{
		SpawnEnemies(1);
		UpdateEnemyCount();
		Sleep(0.05);
		if (Teams[1].TeamMembers.Length <= MaxAlive)
		{
			SpawnEnemies(1);
			UpdateEnemyCount();
			Sleep(0.05);
			if (Teams[1].TeamMembers.Length <= MaxAlive)
			{
				SpawnEnemies(1);
				UpdateEnemyCount();
				Sleep(0.05);
			}
		}
	}
	Stop;
}

/** restarts the game from the last "checkpointed" wave */
final function RestartFromCheckpoint()
{
	RestartFromWave(LastCheckpointWave);
}

/** restarts the game from wave 1 */
final function RestartFromBeginning()
{
	RestartFromWave(1);
}

/** restarts the game from the passed in wave */
exec final function RestartFromWave(int NewWave)
{
	local int i;
	local GearPRI PRI;
	local GameRules NextRules;
	local Controller C;

	// Start the game again, so we can record stats
	StartOnlineGame();

	// reset score
	for (i = 0; i < GearGRI.PRIArray.length; i++)
	{
		PRI = GearPRI(GearGRI.PRIArray[i]);
		if (PRI != None)
		{
			PRI.Score_Kills = 0;
			PRI.Score_Takedowns = 0;
			PRI.Score_Revives = 0;
			PRI.Score_GameSpecific1 = 0;
			PRI.Score_GameSpecific2 = 0;
			PRI.Score_GameSpecific3 = 0;
			PRI.Score = 0;
			PRI.Deaths = 0;
		}
	}
	for (i = 0; i < Teams.length; i++)
	{
		Teams[i].Score = 0;
		Teams[i].TotalScore = 0;
	}

	// delete extended mutators
	ExtendedWaveList = default.ExtendedWaveList;
	while (BaseMutator != None)
	{
		BaseMutator.Destroy();
	}
	NextRules = GameRulesModifiers;
	while (NextRules != None)
	{
		GameRulesModifiers = NextRules;
		NextRules = NextRules.NextGameRules;
		GameRulesModifiers.Destroy();
	}
	GearGRI.ExtendedRestartCount = 0;

	// kill any leftover enemies
	while (Teams[1].TeamMembers.length > 0)
	{
		C = Teams[1].TeamMembers[0];
		if (C.Pawn != None)
		{
			C.Pawn.Destroy();
		}
		C.Destroy();
	}
	EnemyListAliveCount.length = 0;
	EnemyListAliveCount.length = EnemyList.length;

	// clean up the level including any planted grenades
	bEliminatePlantedGrenades = TRUE;
	ResetLevelForNewRound(0.0);
	bEliminatePlantedGrenades = FALSE;

	// reset wave
	CurrentWave = NewWave - 1;

	// start the game
	if (CurrentWave > 0)
	{
		StartNextWave();
	}
	else
	{
		GotoState('MatchInProgress');
	}
}

/**
 * Callback when the start completes
 *
 * @param SessionName the name of the session this is for
 * @param bWasSuccessful true if it worked, false otherwise
 */
function OnStartOnlineGameComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearStartOnlineGameCompleteDelegate(OnStartOnlineGameComplete);
	GearGRI.StartMatch();
}

state RoundOver
{
	// nobody takes damage during round intermission
	function ReduceDamage(out int Damage, Pawn Injured, Controller InstigatedBy, vector HitLocation, out vector Momentum, class<DamageType> DamageType)
	{
		Damage = 0;
	}

	event BeginState(name PrevStateName)
	{
		local PlayerController PC;
		local GearPawn P;
		local GearWeapon W;
		local int DesiredAmmo;

		// Set the game status.. note: RoundOver and PreRound share the same GameStatus
		GearGRI.SetGameStatus(GS_RoundOver);
		// Do all of the stats processing so that we can record stats and mark the party leader as present
		if (GetNumPlayersAlive(0) == 0)
		{
			WriteOnlineStats();
		}

		// survival bonus
		Teams[0].Score += float(GetNumPlayersAlive(0)) / (NumPlayers + NumBots) * CurrentWave *
				(PointsPerWave / ScoringBaselineWavePoints) * class'GearPRI'.default.PointsFor_DamagePct * SurvivalBonusPct;
		// difficulty bonus
		Teams[0].Score *= EnemyDifficulty.default.HordeScoreMultiplier;
		// add to total
		Teams[0].TotalScore += Teams[0].Score;
		Teams[0].bForceNetUpdate = true;

		// Put the game on halt to let players see the scoreboard
		ForEach LocalPlayerControllers(class'PlayerController', PC)
		{
			if (PC != None)
			{
				GearPRI(PC.PlayerReplicationInfo).bHostIsReady = false;
				break;
			}
		}

		// revive DBNO pawns
		// refresh the players' ammo to initial levels
		foreach WorldInfo.AllPawns(class'GearPawn', P)
		{
			if (P.IsDBNO())
			{
				P.DoRevival(P);
			}
			P.DownCount = 0;
			if (P.InvManager != None)
			{
				foreach P.InvManager.InventoryActors(class'GearWeapon', W)
				{
					if (!W.bUniqueSpawn)
					{
						DesiredAmmo = W.InitialMagazines * W.GetMagazineSize();
						if (W.SpareAmmoCount < DesiredAmmo)
						{
							W.AddAmmo(DesiredAmmo - W.SpareAmmoCount);
						}
					}
				}
			}
		}

		`RecordStat(STATS_LEVEL1,'WaveCompleted',None,CurrentWave);

		// delay before starting the next wave
		SetTimer(EndOfRoundDelay, false, nameof(StartNextWave));
	}

	event EndState(name NextStateName)
	{
		local GearPC GPC;
		local Actor A;
		local int i;
		local GearPRI PRI;

		ClearTimer(nameof(StartNextWave));

		// Turn everyone's scoreboard off.
		foreach WorldInfo.AllControllers(class'GearPC',GPC)
		{
			if (GPC != None)
			{
				GPC.ClientToggleScoreboard(false);
			}
		}

		// reset stuff before starting the next round
		foreach DynamicActors(class'Actor', A)
		{
			if (A.IsA('GearDestructibleObject'))
			{
				A.Reset();
			}
		}

		// clear per-round scores
		Teams[0].Score = 0;
		for (i = 0; i < GearGRI.PRIArray.length; i++)
		{
			PRI = GearPRI(GearGRI.PRIArray[i]);
			if (PRI != None)
			{
				PRI.Score_GameSpecific1 = 0;
				PRI.Score_GameSpecific2 = 0;
			}
		}

		// reset values for HUD displays
		GearGRI.WavePointsAlivePct = 255;
		GearGRI.EnemiesLeftThisRound = 255;
	}
}

state MatchOver
{
	event BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		// you only get the difficulty bonus if you fail
		Teams[0].Score *= EnemyDifficulty.default.HordeScoreMultiplier;
		Teams[0].TotalScore += Teams[0].Score;
	}

	event EndState(name NextStateName)
	{
		local Actor A;

		// see if we're restarting the game
		if (NextStateName == 'MatchInProgress')
		{
			// reset stuff before starting the next round
			foreach DynamicActors(class'Actor', A)
			{
				if (A.IsA('GearDestructibleObject') || A.IsA('DroppedPickup') || A.IsA('PickupFactory'))
				{
					A.Reset();
				}
			}

			// reset values for HUD displays
			GearGRI.WavePointsAlivePct = 255;
			GearGRI.EnemiesLeftThisRound = 255;
		}
	}
}

/**
 * Tells each client to write out their stats entries
 */
function WriteOnlineStats()
{
	local GearPC PC;

	// Iterate through the controllers telling them to write stats
	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		if (PC.IsLocalPlayerController() == false)
		{
			PC.ClientWriteLeaderboardStats(OnlineStatsWriteClass);
		}
	}
	// Iterate through local controllers telling them to write stats
	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		if (PC.IsLocalPlayerController())
		{
			PC.ClientWriteLeaderboardStats(OnlineStatsWriteClass);
		}
	}
}

/**
 * Reads all of the Horde stats for the players in the session
 */
function ReadHordeStats()
{
	local array<UniqueNetId> Players;
	local int Index;
	local UniqueNetId ZeroId;

	if (HordeStats == None &&
		OnlineSub != None &&
		OnlineSub.StatsInterface != None)
	{
		// Iterate through the PRIs adding them to the list to read
		for (Index = 0; Index < GearGRI.PRIArray.Length; Index++)
		{
			if (GearGRI.PRIArray[Index].UniqueId != ZeroId)
			{
				Players.AddItem(GearGRI.PRIArray[Index].UniqueId);
			}
		}
		HordeStats = new class'GearLeaderboardHorde';
		// Tell the leaderboard what map we are on so it grabs the right high score
		HordeStats.UpdateViewIdFromName(WorldInfo.GetMapName(true));
		// Kick off the read and then process the results in the callback
		OnlineSub.StatsInterface.AddReadOnlineStatsCompleteDelegate(OnReadHordeStatsComplete);
		OnlineSub.StatsInterface.ReadOnlineStats(Players,HordeStats);
	}
}

/**
 * Called once the read has completed. Updates the PRI data so that it will flow
 * to all players in the game
 *
 * @param bWasSuccessful
 */
function OnReadHordeStatsComplete(bool bWasSuccessful)
{
	local int PriIndex;
	local GearHordePRI HordePRI;
	local UniqueNetId ZeroId;

	OnlineSub.StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnReadHordeStatsComplete);
	if (bWasSuccessful)
	{
		// Read the high score for each PRI
		for (PriIndex = 0; PriIndex < GearGRI.PRIArray.Length; PriIndex++)
		{
			HordePRI = GearHordePRI(GearGRI.PRIArray[PriIndex]);
			if (HordePRI != None && HordePRI.UniqueId != ZeroId)
			{
				HordePRI.HighScore = HordeStats.GetHighScoreForPlayer(HordePRI.UniqueId);
			}
		}
	}
	else
	{
		// Mark all PRIs as being in error
		for (PriIndex = 0; PriIndex < GearGRI.PRIArray.Length; PriIndex++)
		{
			HordePRI = GearHordePRI(GearGRI.PRIArray[PriIndex]);
			if (HordePRI != None && HordePRI.UniqueId != ZeroId)
			{
				HordePRI.HighScore = -1;
			}
		}
	}
}

/** Changes the invite state to allow JIP for private matches */
function AllowJIPForPrivateMatch()
{
	// Do not allow JIP for Horde, even in private
}

/**
 * We override PBP so that we can hack a flag in the game stats object if it exists.  This flag will allow us to have fun later
 */
event PostBeginPlay()
{
	super.PostBeginPlay();

	if ( StatsObject != none )
	{
		StatsObject.bUglyHordeHackFlag = true;
	}
}

defaultproperties
{
	// This PRI also has support for a player's high score
	PlayerReplicationInfoClass=class'GearGame.GearHordePRI'

	// This PC also has support for a player's high score
	PlayerControllerClass=class'GearGame.GearHordePC'

	// Leaderboard to write stats to
	OnlineStatsWriteClass=class'GearLeaderboardHordeWrite'
	// Leaderboards to write skill info to
	LeaderboardId=0xFFFE0000
	ArbitratedLeaderboardId=0xFFFF0000

	LastCheckpointWave=1

	bEliminatePlantedGrenades=FALSE
	bCanSwapBotsAndPlayers=false
}
