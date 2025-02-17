/**
 * GearGame
 * Gear Game Info
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearGame extends GameInfo
	config(Game)
	native
	abstract
	dependson(GearDroppedPickup);


// include the stats definition w/ a GAME_INFO define so that we can avoid the extra GearGame(WorldInfo.Game) context
`define GAMEINFO(dummy)
`include(GearGame\GearStats.uci);
`undefine(GAMEINFO)


/** Prefix characters for names of maps for this game type */
var array<string> MapPrefixes;

/** Only to be used by combat zones in Campaign & Invasion since we don't know how many teams there
 * actually are in Versus.
 */
const COG_TEAM_INDEX	= 0;
const LOCUST_TEAM_INDEX = 1;

/** Min players needed to start a match */
var() config int MinPlayers;

`if(`notdefined(FINAL_RELEASE))
/** Min players needed to start a ranked match */
var() int MinRankedPlayers;
`else
/** Min players needed to start a ranked match */
var() config int MinRankedPlayers;
`endif

/** Total number of teams to create */
var config byte			NumTeams;

/** List of currently active teams */
var array<GearTeamInfo>	Teams;

/** manager object for GUDS */
var GUDManager			UnscriptedDialogueManager;

/** True to permit friendly fire, false to ignore it */
var	config bool			bAllowFriendlyFire;

/** Cheat to test DBNO. Forces the player to stay DBNO and not auto revive/die. */
var bool				bDBNOForeverCheat;

/** True if players are currently in co-op split section */
var bool				bInCoopSplit;
/** set when doing a solo split where going DBNO means game over */
var transient bool bInSoloSplit;

/** If true, this is a fake dedicated server */
var bool				bIsDedicatedListenServer;

/** Gamewide Anya remote speaker object. */
var RemoteSpeaker_COGAnya		Anya;

/** A generic remote speaker we can use for random stuff */
var RemoteSpeaker_Generic		GenericRemoteSpeaker;

/** gamewide monitor object for battle status */
var BattleStatusMonitor			BattleMonitor;

/** True if the host purposefully disconnected their cable, false otherwise */
var bool						bHostTamperedWithMatch;

/** Whether a round should have a time limit or not */
var() config bool				bInfiniteRoundDuration;

/** Whether or not this game type should create and allow tutorials or not */
var() config bool				bAllowTutorials;

/** Type of tutorials this game supports */
var() config EGearAutoInitTutorialTypes AutoTutorialType;

/** This will auto skip all matinees.  This is going to be used for external testing (so they do not see the entire story) **/
var config bool bSkipAllMatinees;

/** Whether the game should do the LocScreenShotTest */
var bool bDoLocScreenShotTest;

/** Whether or not to do automated Horde testing which will just kill mobs over and over and then do a memleackcheck at the start of each match **/
var bool bDoAutomatedHordeTesting;

/** Spectator Camera Points */
var transient protected array<GearSpectatorPoint> SpectatorPoints;

enum EEncouragementType
{
	ET_WonMatch,
	ET_LostMatch,
	ET_WonMatchByShutout,
	ET_LostMatchByShutout,
	ET_MatchOrRoundDraw,
	ET_WonRound,
	ET_LostRound,
};

struct native TeamEncouragementData
{
	var array<SoundCue> COGSounds;
	var array<SoundCue> LocustSounds;
};

/** Data for end-of-match encouragements */
var const TeamEncouragementData EncouragementData[EEncouragementType.EnumCount];

/** Sound cues to play when a player dies during a coop split game */
var SoundCue MarcusIsDownCue;
var SoundCue DomIsDownCue;

/** Number of respawns a team has remaining */
var array<int> NumTeamRespawns;
/** Total Number of respawns a team can have in the game. */
var() config int TotalNumTeamRespawns;
/** The time interval that respawns will occur */
var() config int RespawnTimeInterval;
/** Time the respawned player has of invincibility */
var() config float RespawnInvincibilityTimer;

/** Generic weapons disappear after x secs - 0 means they don't disappear */
var() config float WeaponFadeTimeGeneric;
/** Special weapons disappear after x secs - 0 means they don't disappear */
var() config float WeaponFadeTimeSpecial;

/** Tie Margin for Chainsaw Dueling */
var() config int ChainsawDuelingTieMargin;

/** Manager which handles asynchronous sight checks for AI **/
var instanced AIVisibilityManager AIVisMan;

/** Stats recording object */
var GearStatsObject StatsObject;

/** list of deployed shields for AI checks */
var array<GearDroppedPickup_Shield> DeployedShields;

/** difficulty level classes */
var array< class<DifficultySettings> > DifficultyLevels;

/** debug flag to force stats in non-arbitrated games so it's easier to test the stats server */
var bool bForceStats;

/** set if the difficulty came from the URL and the profile value should be ignored */
var bool bURLDifficultyOverride;

/**
 * SUPER HACK to ship the game! This bool is set when adding a bot who is replacing a human so
 * we can avoid the GetForcedTeam() functon when calling ChangeTeam
 */
var bool bSkipForcedTeamCallHack;

/** Set of efects to use when a gib comes off */
struct native GearGoreEffectInfo
{
	/** Effect to attach to falling gib  */
	var()	ParticleSystem	GibEffect;
	/** Effect to attach to creature where gib left */
	var()	ParticleSystem	HoleEfect;
	/** How much to scale up gib effect */
	var()	float			EffectScale;
	/** Sound to play when gib comes off */
	var()	SoundCue		LoseGibSound;

	structdefaultproperties
	{
		EffectScale=1.0
	}
};

/** Stats Mask Defines */

const STATS_LEVEL1	= 0x01;


cpptext
{
	virtual UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );
}

/**
 * @return	a reference to the gear-specific online game settings data store.
 */
static final function GearUIDataStore_GameSettings GetGameSettingsDataStore()
{
	return class'GearUIScene_Base'.static.GetGameSettingsDataStore();
}

/**
 * Wrapper for getting the OnlineGameSettings object associated with the currently selected gametype.
 *
 * @param	bCreateIfNecessary	TRUE to indicate that the OnlineGameSettings object associated with the current game
 *								should be created if it doesn't yet exist.
 *
 * @return	the OnlineGameSettings object that contains the settings for the currently selected gametype.
 */
function OnlineGameSettings GetCurrentGameSettings( optional bool bCreateIfNecessary )
{
	local GearUIDataStore_GameSettings SettingsDS;
	local OnlineGameSettings Result;

	// find the settings for the current match
	SettingsDS = GetGameSettingsDataStore();
	if ( SettingsDS != None )
	{
		Result = SettingsDS.GetCurrentGameSettings();
	}

	if ( Result == None && bCreateIfNecessary )
	{
		Result = new class'GearWarzoneSettings';
	}

	return Result;
}

event PreBeginPlay()
{
	local GearGRI OldGRI;
	local int Idx;

	OldGRI = GearGRI(WorldInfo.GRI);
	Super.PreBeginPlay();
	if (OldGRI != None)
	{
		for ( Idx = 0; Idx < 5; Idx++ )
		{
			GearGRI(WorldInfo.GRI).WingmanClassIndexes[Idx] = OldGRI.WingmanClassIndexes[Idx];
		}
		OldGRI.Destroy();
	}
}

event PostBeginPlay()
{
	UnscriptedDialogueManager = Spawn( class'GUDManager' );
	GenericRemoteSpeaker = Spawn( class'RemoteSpeaker_Generic' );
	Anya = Spawn( class'RemoteSpeaker_COGAnya' );
	BattleMonitor = Spawn( class'BattleStatusMonitor' );

	// only record stats for ranked matches

	if (bUsingArbitration)
	{
		StatsObject = new class'GearStatsObject';
	}
`if(`notdefined(FINAL_RELEASE))
	else if (bForceStats)
	{
		StatsObject = new class'GearStatsObject';
	}
`endif

	super.PostBeginPlay();
}


/** */
native function RegisterGameWithLive();
native function String GetMapFilename();

final function GearGRI GetGRI()
{
	return GearGRI(GameReplicationInfo);
}

/**
 * Transition back to pre-match state.
 */
function Reset()
{
	super.Reset();
	GotoState('PendingMatch');
}

event PlayerController Login(string Portal, string Options, out string ErrorMessage)
{
	local PlayerController PC;
	local GearPC WPC;
	local bool bDedicated;
	local GearPRI PRI;
	// if this is the first player, and he has the dedicated server option, mark him as such and force to be spectator
	if (HasOption(Options, "dedicated"))
	{
		bDedicated = true;
		Options $= "?SpectatorOnly=1";
	}

	PC = Super.Login(Portal, Options, ErrorMessage);
	WPC = GearPC(PC);

	if (bDedicated)
	{
		bIsDedicatedListenServer = true;
		if (WPC != None)
		{
			WPC.bDedicatedServerSpectator = true;
			WPC.PlayerReplicationInfo.bIsSpectator = true;
		}
	}
	// log join-in-progess players
	if (StatsObject != None &&
		StatsObject.bGameplaySessionInProgress)
	{
		`RecordStat(STATS_LEVEL1,'PlayerLogin',WPC,WPC.PlayerReplicationInfo.PlayerName);
	}

	// check for a difficulty setting specified on the commandline
	if (PC != None)
	{
		PRI = GearPRI(PC.PlayerReplicationInfo);
		if (PRI != None)
		{
			if (HasOption(Options,"Casual"))
			{
				PRI.Difficulty = class'DifficultySettings_Casual';
				bURLDifficultyOverride = true;
			}
			else if (HasOption(Options,"Normal"))
			{
				PRI.Difficulty = class'DifficultySettings_Normal';
				bURLDifficultyOverride = true;
			}
			else if (HasOption(Options,"Hardcore"))
			{
				PRI.Difficulty = class'DifficultySettings_Hardcore';
				bURLDifficultyOverride = true;
			}
			else if (HasOption(Options,"Insane"))
			{
				PRI.Difficulty = class'DifficultySettings_Insane';
				bURLDifficultyOverride = true;
			}
			`log("Difficulty setting for"@PC@"is:"@PRI.Difficulty);
			`RecordStat(STATS_LEVEL1,'Difficulty',PC,PRI.Difficulty);
		}
	}
	return PC;
}

function PostLogin(PlayerController NewPlayer)
{
	local PlayerController HostPlayer;
	local GearPC OtherPC;

	// for mp games force all players to share the host difficulty
	if (GameReplicationInfo.IsMultiplayerGame() && LocalPlayer(NewPlayer.Player) == None)
	{
		foreach WorldInfo.LocalPlayerControllers(class'PlayerController',HostPlayer)
		{
			`log("Forcing player"@NewPlayer@"difficulty to"@GearPRI(HostPlayer.PlayerReplicationInfo).Difficulty);
			GearPRI(NewPlayer.PlayerReplicationInfo).Difficulty = GearPRI(HostPlayer.PlayerReplicationInfo).Difficulty;
			`RecordStat(STATS_LEVEL1,'Difficulty',NewPlayer,GearPRI(NewPlayer.PlayerReplicationInfo).Difficulty);
			break;
		}
	}

	InitializeTutorialSystem(GearPC(NewPlayer));
	Super.PostLogin(NewPlayer);

	if (UnscriptedDialogueManager != None && GearPC(NewPlayer) != None)
	{
		UnscriptedDialogueManager.ReplicateLoadedGUDBanks(GearPC(NewPlayer));
	}

	// The player has just joined, so ask for their saved skill
	GearPC(NewPlayer).ClientGetPlayerSkill();

	// tell client to display the "host has paused" menu if host has paused in a co-op game
	if ( WorldInfo.Pauser != None && LocalPlayer(NewPlayer.Player) == None &&
		(!WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.GRI.IsCoopMultiplayerGame()) )
	{
		foreach LocalPlayerControllers(class'GearPC', OtherPC)
		{
			if ( (OtherPC.MyGearHud.PauseUISceneInstance != None &&	class'UIRoot'.static.GetSceneClient().FindSceneIndex(OtherPC.MyGearHud.PauseUISceneInstance) != INDEX_NONE) || OtherPC.MyGearHud.DiscoverUISceneInstance != None )
			{
				GearPC(NewPlayer).ClientUpdateHostPause(true);
				break;
			}
		}
	}
}

/**
 * Calls a replicated function on the player to initialize their tutorial manager
 *
 * @param	PC	the player that should initialize the tutorial system.
 */
function InitializeTutorialSystem( GearPC PC )
{
	if ( PC != None && bAllowTutorials )
	{
		// Create the Tutorial Manager
		PC.ClientInitializeTutorialSystem( AutoTutorialType );
	}
}

function SetPlayerDefaults(Pawn PlayerPawn)
{
	local GearPawn WP;
	local GearPRI PRI;

	Super.SetPlayerDefaults(PlayerPawn);

	WP = GearPawn(PlayerPawn);
	if (WP != None)
	{
		WP.Health = WP.DefaultHealth;
		WP.HealthMax = WP.DefaultHealth;
		WP.GroundSpeed = WP.DefaultGroundSpeed;
		if (!AllowHealthRecharge(WP))
		{
			WP.HealthRechargeDelay = 0.f;
			WP.HealthRechargePercentPerSecond = 0.f;
		}
	}
	// apply the difficulty settings for the player
	if (PlayerPawn.Controller != None)
	{
		PRI = GearPRI(PlayerPawn.PlayerReplicationInfo);
		if (PRI != None)
		{
			PRI.Difficulty.static.ApplyDifficultySettings(PlayerPawn.Controller);
		}
	}
}

/**
 * Handles creating the default teams.
 */
function InitializeTeams()
{
	local int Idx;

	// initialize all the teams
	for (Idx = 0; Idx < NumTeams; Idx++)
	{
		Teams[Idx] = Spawn(class'GearTeamInfo',self);
		Teams[Idx].TeamIndex = Idx;
		GameReplicationInfo.SetTeam(Idx, Teams[Idx]);
	}
}

/**
 * Allows games to force the team for specific players.
 */
function int GetForcedTeam(Controller Other, int Team)
{
	// default to whatever was already specified
	return Team;
}


/** Handles switching to Team 255 (Undecided) */
function bool ChangeTeam(Controller Other, int N, bool bNewTeam)
{
	// check to see if we need to initialize teams
	if (Teams.Length < NumTeams)
	{
		InitializeTeams();
	}

	if (Other != None)
	{
		if (!bSkipForcedTeamCallHack)
		{
			N = GetForcedTeam(Other,N);
		}
		// if not already on that team
		if (N == 255 || Other.PlayerReplicationInfo.Team == None || Other.PlayerReplicationInfo.Team != Teams[N] )
		{
			// remove from their current team
			if (Other.PlayerReplicationInfo.Team != None)
			{
				Other.PlayerReplicationInfo.Team.RemoveFromTeam(Other);
			}

			if (N < 255)
			{
				// and attempt to add them to their new team
				if ( Teams[N].AddToTeam(Other) )
				{
					// add bots to a default squad
					if (Other.IsA('GearAI_TDM'))
					{
						Teams[N].JoinSquad('Alpha', Other);
					}
					SetPlayerClass(Other,N);
					return true;
				}
				return  false;
			}
			else
			{
				if ( Other.PlayerReplicationInfo.Team != None )
				{
					Other.PlayerReplicationInfo.Team.RemoveFromTeam(Other);
				}

				Other.PlayerReplicationInfo.Team = none;
				GearPRI(Other.PlayerReplicationInfo).PawnClass=none;
				GearPRI(Other.PlayerReplicationInfo).bIsLeader=false;
				`RecordStat(STATS_LEVEL1,'TeamChange',Other, "255");
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

/**
 * Overridden to prevent friendly fire damage.
 */
function ReduceDamage(out int Damage, Pawn injured, Controller instigatedBy, vector HitLocation, out vector Momentum, class<DamageType> DamageType)
{
	local int			Idx;
	local Controller	injuredController;
	local GearPawn		InjuredGearPawn;
	local class<GearDamageType> GDT;

	InjuredGearPawn = GearPawn(Injured);

	// grab the injured controller
	if( injured != None )
	{
		// special case if we're driving a vehicle
		if( injured.DrivenVehicle != None )
		{
			injuredController = injured.DrivenVehicle.Controller;
		}
		if( injuredController == None )
		{
			injuredController = injured.Controller;
		}
	}

	GDT = class<GearDamageType>(DamageType);
	// Prevent friendly fire (but players can damage themselves)
	if ( (GDT == none || !GDT.default.bAlwaysDamageFriendlies) && !bAllowFriendlyFire && injured != None && instigatedBy != None &&
		( injuredController == None || injuredController != instigatedBy ||
			(!injured.IsHumanControlled() && (InjuredGearPawn == None || !InjuredGearPawn.IsDBNO()) && GearAI_TDM(injuredController) == None) ) )
	{
		// If preventing friendly fire, then revert rules for hostages.
		// They are now treated as a shield for the kidnapper.
		// So only friends of the kidnapper (enemies of the hostage), cannot damage him.
		if( InjuredGearPawn != None && InjuredGearPawn.IsAHostage() )
		{
			// Disable damage only for people in the same team as the kidnapper.
			if( WorldInfo.GRI.OnSameTeam(InjuredGearPawn.InteractionPawn, instigatedBy) )
			{
				Damage = 0;
			}
		}
		// If not hostage, then ignore damage if on same team (unless it's environmental/scripted)
		else if (!GDT.default.bEnvironmentalDamage)
		{
			if( WorldInfo.GRI.OnSameTeam(injured, instigatedBy) )
			{
				Damage = 0;
			}
		}
	}

	if( Damage > 0 )
	{
		// notify teams of the damage
		for (Idx = 0; Idx < Teams.Length; Idx++)
		{
			Teams[Idx].NotifyTakeDamage(injured.Controller,instigatedBy);
		}
		Super.ReduceDamage(Damage,injured,instigatedBy,HitLocation,Momentum,damageType);
	}
}

function Killed(Controller Killer, Controller KilledPlayer, Pawn KilledPawn, class<DamageType> damageType)
{
	// ignore the hostage kill since it has been counted prior
	if (damageType == class'GDT_Hostage')
	{
		return;
	}

	Super.Killed(Killer,KilledPlayer,KilledPawn,damageType);

	// log the events
	`RecordStat(STATS_LEVEL1,'PlayerKill',Killer,damageType,KilledPawn.Controller);
	`RecordStat(STATS_LEVEL1,'PlayerDeath',KilledPlayer,damageType,Killer);
}

/**
 * Overridden to notify teams of the death for the purposes of morale tracking.
 */
function NotifyKilled(Controller Killer, Controller KilledPlayer, Pawn KilledPawn )
{
	local int Idx;

	for (Idx = 0; Idx < Teams.Length; Idx++)
	{
		if (Teams[Idx] != None)
		{
			Teams[Idx].NotifyKilled(Killer,KilledPlayer,KilledPawn);
		}
	}

	// store killer for pawn
	if (GearPawn(KilledPawn) != None)
	{
		GearPawn(KilledPawn).NotifyKilled(Killer);
	}

	Super.NotifyKilled(Killer, KilledPlayer, KilledPawn);
}


/**
 * Overridden to not do anything
 * @see GearBroadcastDeathMessage
 **/
function BroadcastDeathMessage(Controller Killer, Controller Other, class<DamageType> damageType);

/**
 * Send out a weapon has been taken message to all players.
 **/
function GearBroadcastWeaponTakenMessage( Controller WeaponTaker, class<DamageType> damageType );

/** Allow Gametype to prevent weapon drops */
function bool CanDropWeapon( Weapon W, vector DropLocation )
{
	return TRUE;
}

/** Whether a dropped pickup is a generic drop or a special one */
function EGearDroppedPickupType GetDropType( GearDroppedPickup DroppedPickupObj )
{
	// Is not a weapon so it's generic
	if ( (DroppedPickupObj == None) || !DroppedPickupObj.Inventory.IsA('GearWeapon') )
	{
		return eGEARDROP_Generic;
	}
	// Is a weapon so we need to see if this game type thinks it's a generic drop
	else
	{
		return GetWeaponDropType( GearWeapon(DroppedPickupObj.Inventory) );
	}
}

/** Whether a weapon is a generic drop or a special one */
function EGearDroppedPickupType GetWeaponDropType( GearWeapon Weap )
{
	return eGEARDROP_Generic;
}

/**
 * Wrapper for determining whether the match can be started.
 *
 * @return	TRUE if all requirements for beginning the match have been met.
 */
function bool CanStartMatch()
{
	return ArePlayersReady() && !ArePlayersNeeded();
}

/**
 * Wrapper for determining whether all required players are ready to play.
 *
 * @return	TRUE if all players are ready to begin the match.
 *
 * @todo - one of the gameplay guys, please implement this method for GearGame with the correct logic
 */
function bool ArePlayersReady();

/**
 * Wrapper for determining whether the minimum number of players are connected.
 *
 * @return	TRUE if more players are needed to start the match.
 *
 * @todo - one of the gameplay guys, please implement this method for GearGame with the correct logic
 */
function bool ArePlayersNeeded();


/**
 * @STATS
 * Allows each game type to add it's own set of game rules to the stats object
 */
function string GetGameOptionsForStats(string OptionStr)
{
    OptionStr $= "?GoalScore="$GoalScore;
    return OptionStr;
}

/** This is our overridden StartMatch() function **/
function StartMatch()
{
	local GearPC PC;
	local string Options;
	local GearGRI WGRI;

	if (StatsObject != None && !StatsObject.bGameplaySessionInProgress && !class'WorldInfo'.static.IsMenuLevel())
	{
		// begin tracking events for this session
		StatsObject.BeginGameplaySession();
		`RecordStat(STATS_LEVEL1,'SessionInfo',None,Class,None);
		`RecordStat(STATS_LEVEL1,'MapName',None,GetURLMap(),None);
		Options =  GetGameOptionsForStats(Options);
		`RecordStat(STATS_LEVEL1,'GameOptions',None,Options);
		// log all the players who have already joined
		foreach WorldInfo.AllControllers(class'GearPC',PC)
		{
			`RecordStat(STATS_LEVEL1,'PlayerLogin',PC,PC.PlayerReplicationInfo.PlayerName);
		}
		WGRI = GetGRI();
		if (WGRI!=none)
		{
			WGRI.StatsGameplaySessionID = StatsObject.GameplaySessionID;
		}
	}

	if( bDoingASentinelRun == TRUE )
	{
		`log( "DoingASentinelRun!" );

		// this will take over the normal match rules and do its own thing
		if( SentinelTaskDescription == "TravelTheWorld" )
		{
			DoTravelTheWorld();
			return;
		}
		// any of these types are going to run in addition to what ever the player is doing
		// they just go gather stats based on a timer
		else
		{
			BeginSentinelRun( SentinelTaskDescription, SentinelTaskParameter, SentinelTagDesc );
			// for now we are just hard coding what we want to gather
			foreach WorldInfo.AllControllers( class'GearPC', PC )
			{
				//PC.ConsoleCommand( "stat " );
			}

			SetTimer( 3.0f, TRUE, nameof(DoTimeBasedSentinelStatGathering) );
		}
	}

	`log("Starting match...");
	Super.StartMatch();

	// transition to the new in progress state
	GotoState('MatchInProgress');


    // BugIt functionality
	// need to delay here as in MP maps there is a delay in starting the match even with QuickStart on
	SetTimer( 5.0f, FALSE, nameof(CheckForBugItCommand) );


	if (WorldInfo.Game.bAutomatedTestingWithOpen == true)
	{
		WorldInfo.Game.IncrementNumberOfMatchesPlayed();
	}
	else
	{
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			PC.IncrementNumberOfMatchesPlayed();
			break;
		}
	}

	WorldInfo.Game.IncrementAutomatedTestingMapIndex();

	if( bCheckingForFragmentation == TRUE )
	{
		//ConsoleCommand( "killparticles" );
		ConsoleCommand( "MemFragCheck" );
	}

	if( AutomatedTestingExecCommandToRunAtStartMatch != "" )
	{
		`log( "AutomatedTestingExecCommandToRunAtStartMatch: " $ AutomatedTestingExecCommandToRunAtStartMatch );
		ConsoleCommand( AutomatedTestingExecCommandToRunAtStartMatch );
	}
}

function ScoreKill(Controller Killer, Controller Other)
{
}

/** This will check to see if there is a BugIt command active and then call the BugItGo function **/
function CheckForBugItCommand()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		break;
	}

	if( BugLocString != "" || BugRotString != "" )
	{
		if( PC.CheatManager != none )
		{
			//`log( "BugLocString:" @ BugLocString );
			//`log( "BugRotString:" @ BugRotString );
			PC.BugItGoString( BugLocString, BugRotString );
		}
	}
}


/** We wait a few seconds to let the level load **/
function DoMemStartupStats()
{
	SetTimer( 3.0f, FALSE, nameof(DelayedExit) );
}


function DelayedExit()
{
	`log( "doing Sentinel_Memory!" );
	ConsoleCommand( "Sentinel_Memory" );
	ConsoleCommand( "exit" );
}

function Logout(Controller Exiting)
{
	local GearPC PC;
	if (GearAI_TDM(Exiting) != None || GearPC(Exiting) != None)
	{
		foreach WorldInfo.AllControllers(class'GearPC',PC)
		{
			if (PC != Exiting)
			{
				PC.ReceiveLocalizedMessage( class'GearGameMessage', 4, Exiting.PlayerReplicationInfo);
			}
		}
	}
	Super.LogOut(Exiting);
}

event PreExit()
{
	local Controller Player;
	foreach WorldInfo.AllControllers(class'Controller',Player)
	{
		`RecordStat(STATS_LEVEL1,'PlayerLogout',Player,Player.PlayerReplicationInfo.PlayerName);
	}
	// Write out our stats to file, Vince, etc.
	UploadGameplayStats();
	Super.PreExit();
}

auto State PendingMatch
{
	function CheckStartMatch()
	{
		// start the match immediately
		StartMatch();
	}

Begin:
	CheckStartMatch();
	Sleep(1.f);
	Goto('Begin');
}

state MatchInProgress
{
	function BeginState(Name PreviousStateName)
	{
		GetGRI().GameStatus = GS_RoundInProgress;
		Super.BeginState(PreviousStateName);
	}


Begin:

	if( CauseEventCommand != "" )
	{
		// wait until all levels are streamed back in
		do
		{
			bSentinelStreamingLevelStillLoading = FALSE;

			for( SentinelIdx = 0; SentinelIdx < WorldInfo.StreamingLevels.length; ++SentinelIdx )
			{
				if( WorldInfo.StreamingLevels[SentinelIdx].bHasLoadRequestPending == TRUE )
				{
					`log( "levels not streamed in yet sleeping 1s" );
					bSentinelStreamingLevelStillLoading = TRUE;
					Sleep( 1.0f );
					break;
				}
			}

		} until( bSentinelStreamingLevelStillLoading == FALSE );

		// check to see if we should fire off the FlyThrough event again as preround starting usually stops the first event
		if( CauseEventCommand != "" )
		{
			foreach WorldInfo.AllControllers(class'PlayerController', SentinelPC)
			{
				SentinelPC.ConsoleCommand( "ce " $ CauseEventCommand );
				break;
			}
		}

		// wait 500 ms to let the switching camera Hitch work itself out
		if( ( SentinelTaskDescription == "FlyThrough" ) || ( SentinelTaskDescription == "FlyThroughSplitScreen" ) )
		{
			SetTimer( 0.500f, TRUE, nameof(DoTimeBasedSentinelStatGathering) );
		}
	}

}

event TriggerGUDEvent(EGUDEventID ID, Pawn InstigatedBy, optional Pawn Recipient, optional float Delay)
{
	UnscriptedDialogueManager.TriggerGUDEvent(ID, InstigatedBy, Recipient, Delay);
}


/**
 * change player to another character type, useful for testing
 */
exec function SetPlayerChar(string type)
{
	local PlayerController	PC;
	local Pawn				P, OldP;
	local vector			SpawnLoc;
	local rotator			SpawnRot;
	local class<GearPawn>	NewPawnClass;

	if (type ~= "baird")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGBaird",class'class'));
	}
	else if (type ~= "bairdmp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGBairdMP",class'class'));
	}
	else if (type ~= "gus" || type ~= "cole")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGGus",class'class'));
	}
	else if (type ~= "gusmp" || type ~= "colemp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGGusMP",class'class'));
	}
	else if ( (type ~= "minh") || (type ~= "kim") )
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGMinh",class'class'));
	}
	else if ( (type ~= "minhmp") || (type ~= "kimmp") )
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGMinhMP",class'class'));
	}
	else if (type ~= "dizzy")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGDizzy",class'class'));
	}
	else if (type ~= "dizzymp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGDizzyMP",class'class'));
	}
	else if (type ~= "tai")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGTai",class'class'));
	}
	else if (type ~= "taimp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGTaiMP",class'class'));
	}
	else if (type ~= "dom")
	{
		NewPawnClass = class'GearPawn_COGDom';
	}
	else if (type ~= "dommp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGDomMP",class'class'));
	}
	else if (type ~= "marcus")
	{
		NewPawnClass = class'GearPawn_COGMarcus';
	}
	else if (type ~= "marcusmp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGMarcusMP",class'class'));
	}
	else if (type ~= "drone")
	{
		NewPawnClass = class'GearPawn_LocustDrone';
	}
	else if (type ~= "dronemp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustMP",class'class'));
	}
	else if (type ~= "dronesnipermp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustSniperMP",class'class'));
	}
	else if (type ~= "theron")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustTheron",class'class'));
	}
	else if (type ~= "theronmp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustTheronmp",class'class'));
	}
	else if (type ~= "carmine" || type ~= "redshirt")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGCarmine",class'class'));
	}
	else if (type ~= "carminemp" || type ~= "redshirtmp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGCarmineMP",class'class'));
	}
	else if (type ~= "hoffman")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGHoffman",class'class'));
	}
	else if (type ~= "hoffmanmp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_COGHoffmanMP",class'class'));
	}
	else if ( (type ~= "hunter") || (type ~= "grenadier") )
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustHunterArmorNoGrenades",class'class'));
	}
	else if ( (type ~= "hunter2") || (type ~= "grenadier2") )
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustHunterNoArmorNoGrenades",class'class'));
	}
	else if ( (type ~= "huntermp") || (type ~= "grenadiermp") )
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustHunterArmorMP",class'class'));
	}
	else if (type ~= "raammp")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustGeneralRaamMP",class'class'));
	}
	else if (type ~= "boomer")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustBoomer",class'class'));
	}
	else if (type ~= "butcherboomer")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustBoomerButcher",class'class'));
	}
	else if (type ~= "wretch")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustWretch",class'class'));
	}
	else if (type ~= "tank")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustTankMP",class'class'));
	}
	else if (type ~= "chopper")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustChopperMP",class'class'));
	}
	else if (type ~= "paladin")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustPaladinMP",class'class'));
	}
	else if (type ~= "beastlord")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustBeastLordMP",class'class'));
	}
	else if (type ~= "franklin")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_MeatflagFranklin",class'class'));
	}
	else if (type ~= "drunk")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_MeatflagDrunk",class'class'));
	}
	else if (type ~= "oldman")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_MeatflagOldman",class'class'));
	}
	else if ( (type ~= "kantus") || (type ~= "priest") )
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustKantus",class'class'));
	}
	else if (type ~= "skorge")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustSkorge",class'class'));
	}
	else if (type ~= "palaceguard")
	{
		NewPawnClass = class<GearPawn>(DynamicLoadObject("GearGameContent.GearPawn_LocustPalaceGuard",class'class'));
	}
	if (NewPawnClass != None)
	{
		foreach WorldInfo.AllControllers(class'PlayerController', PC)
		{
			SpawnLoc = PC.Pawn.Location;
			SpawnRot = PC.Pawn.Rotation;

			OldP = PC.Pawn;
			PC.UnPossess();
			OldP.Destroy();

			P = Spawn(NewPawnClass,,, SpawnLoc, SpawnRot);

			PC.Possess(P, FALSE);

			PC.ConsoleCommand("loaded");
		}
	}
}


exec function FF()
{
	local PlayerController PC;
	local GearGRI GRI;

	bAllowFriendlyFire = !bAllowFriendlyFire;
	GRI = GearGRI(GameReplicationInfo);
	if( GRI != None )
	{
		GRI.bAllowFriendlyFire = bAllowFriendlyFire;
	}

	// Get the first local player controller
	foreach LocalPlayerControllers(class'PlayerController', PC) { break; }
	PC.ClientMessage("bAllowFriendlyFire is"@bAllowFriendlyFire);
}

/**
 * Does this game type allow pawns to recharge health?
 */
function bool AllowHealthRecharge(GearPawn Pawn)
{
	// allow players and friendly ai to recharge
	return (Pawn.IsPlayerOwned() || GearPawn_COGGear(Pawn) != None);
}

/** Reload configs for the specified game type. */
native static final function LoadGameTypeConfigs( string GameTypeExtension );

/**
* called for MP games.
*
* @see WarPawn:  state Reviving
**/
function bool AutoReviveOnBleedOut( Pawn TestPawn );

/**
* called for MP games.
*
* @see WarPawn:  state Reviving
**/
function bool CanBeShotFromAfarAndKilledWhileDownButNotOut( Pawn TestPawn, Pawn InstPawn, class<GearDamageType> TheDamageType );

event GetSeamlessTravelActorList(bool bToEntry, out array<Actor> ActorList)
{
	local int i;

	Super.GetSeamlessTravelActorList(bToEntry, ActorList);

	for (i = 0; i < Teams.length; i++)
	{
		if (Teams[i] != None && (bToEntry || Teams[i].Size > 0))
		{
			ActorList[ActorList.length] = Teams[i];
		}
	}

	// add GRI even when going to destination so we can copy some stuff
	if (!bToEntry)
	{
		ActorList[ActorList.length] = WorldInfo.GRI;
	}
}

event PostSeamlessTravel()
{
	InitializeTeams();

	Super.PostSeamlessTravel();
}

event HandleSeamlessTravelPlayer(out Controller C)
{
	local TeamInfo NewTeam;

	if (C.PlayerReplicationInfo != None && C.PlayerReplicationInfo.Team != None && C.PlayerReplicationInfo.Team.TeamIndex < Teams.length)
	{
		// move this player to the new team object with the same team index
		NewTeam = Teams[C.PlayerReplicationInfo.Team.TeamIndex];
		// if the old team would now be empty, we don't need it anymore, so destroy it
		if (C.PlayerReplicationInfo.Team.Size <= 1)
		{
			C.PlayerReplicationInfo.Team.Destroy();
		}
		else
		{
			C.PlayerReplicationInfo.Team.RemoveFromTeam(C);
		}
		NewTeam.AddToTeam(C);
	}

	Super.HandleSeamlessTravelPlayer(C);

	InitializeTutorialSystem(GearPC(C));

	// The player has just joined, so ask for their saved skill
	GearPC(C).ClientGetPlayerSkill();
}

/**
 * Overridden so we can have our pawn just be reset nicely as there is all kinds of logic
 * that shorts circuits the call to ClientRestart
 **/
function RestartPlayer(Controller NewPlayer)
{
	local GearPawn TestPawn;

	Super.RestartPlayer(NewPlayer);

	if (NewPlayer.Pawn != None)
	{
		`RecordStat(STATS_LEVEL1,'PlayerSpawn',NewPlayer,NewPlayer.Pawn.Class @ string(NewPlayer.GetTeamNum()));

		//@STATS
		if (NewPlayer.Pawn != None)
		{
			GearPRI(NewPlayer.PlayerReplicationInfo).AliveStartTime = WorldInfo.TimeSeconds;
		}

		NewPlayer.Pawn.ClientRestart();
		// Reclaim sole ownership of the PRI
		if (GearPawn(NewPlayer.Pawn) != None)
		{
			foreach WorldInfo.AllPawns(class'GearPawn', TestPawn)
			{
				if ( (TestPawn != NewPlayer.Pawn) && (TestPawn.PlayerReplicationInfo == NewPlayer.Pawn.PlayerReplicationInfo) )
				{
					TestPawn.PlayerReplicationInfo = None;
				}
			}
		}
	}
}

/**
 * @Returns true if we should show a lobby on login
 */
function bool ShowLobby(GearPC LobbyTarget)
{
	return false;
}

/** Initialize the respawn count array. */
function InitTeamRespawnCounts()
{
	local int Idx;

	// Initialized the respawn count array.
	for ( Idx = 0; Idx < NumTeams; Idx++ )
	{
		if ( Idx >= NumTeamRespawns.length )
		{
			NumTeamRespawns.length = Idx+1;
		}
		NumTeamRespawns[Idx] = 0;
	}
}

function InitGameReplicationInfo()
{
	local GearGRI WGRI;

	Super.InitGameReplicationInfo();

	WGRI = GetGRI();
	if (WGRI!=none)
	{
		WGRI.bIsDedicatedListenServer = bIsDedicatedListenServer;
	}

	InitTeamRespawnCounts();
}


/**
 * Returns the total number of human players currently in the game.
 */
final function int GetHumanPlayerCount()
{
	local PlayerController Player;
	local int Count;

	foreach WorldInfo.AllControllers(class'PlayerController', Player)
	{
		if (!Player.bPendingDelete)
		{
			Count++;
		}
	}

	return Count;
}

function ChangeName(Controller Other, coerce string S, bool bNameChange)
{
	// do nothing for initial name change - wait for client to send his gamertag
	// (we can't pass it on the URL because it might contain spaces and that screws up the URL parsing at the moment)
	if (bNameChange || AIController(Other) != None)
	{
		Super.ChangeName(Other, S, bNameChange);
	}
}

function SetPlayerClass(Controller Other,int N);

/**
 *
 *
 * @param
 */
static function int FindCharacterProviderIndex( int TeamIndex, byte ProfileId )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue CharacterValue;
	local int Result;
	local name TeamName;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		CharacterValue.PropertyTag = 'ProfileId';
		CharacterValue.PropertyType = DATATYPE_Property;
		CharacterValue.StringValue = string(GetEnum(GameResourceDS.GetTeamCharacterProviderIdType(TeamIndex), ProfileId));
		CharacterValue.ArrayValue.AddItem(ProfileId);

		TeamName = GameResourceDS.GetTeamCharacterProviderTag(TeamIndex);

		Result = GameResourceDS.FindProviderIndexByFieldValue(TeamName, 'ProfileId', CharacterValue);
	}

	return Result;
}

static function string GetCharacterProviderClassName( int TeamIndex, int ProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue CharacterValue;
	local string Result;
	local name TeamName;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		TeamName = GameResourceDS.GetTeamCharacterProviderTag(TeamIndex);
		if ( GameResourceDS.GetProviderFieldValue(TeamName, 'ClassPathName', ProviderIndex, CharacterValue) )
		{
			Result = CharacterValue.StringValue;
		}
	}

	return Result;
}

static function class<GearPawn> GetCharacterProviderClass( int TeamIndex, int ProviderIndex )
{
	local string ClassPathName;
	local class<GearPawn> Result;

	ClassPathName = GetCharacterProviderClassName(TeamIndex, ProviderIndex);
	if ( ClassPathName != "" )
	{
		Result = class<GearPawn>(DynamicLoadObject(ClassPathName, class'Class'));
	}

	return Result;
}

/**
 * Changes the character class for a player when the player chooses to switch characters.
 *
 * @param	WPRI				the PlayerReplicationInfo for the player changing character
 * @param	DesiredCharacterId	the id of the character to switch to; will be one of the values from either the ECogMPCharacter
 *								or ELocustMPCharacter enums, depending on the player's team.
 */
function SetPlayerClassFromPlayerSelection( GearPRI WPRI, byte DesiredCharacterId )
{
	local int ProviderIndex;

	if ( WPRI.Team != none )
	{
		// find the index for the data provider which has a ProfileId matching DesiredCharacterId
		ProviderIndex = FindCharacterProviderIndex(WPRI.Team.TeamIndex, DesiredCharacterId);
		if ( ProviderIndex != INDEX_NONE )
		{
			ApplyPlayerRequestedCharacterSwitch(WPRI, ProviderIndex);
		}
	}
}

/**
 * Accessor method for applying a user-generated character class change.  This version takes the index into the
 * actual array of character data providers, and should only be used internally.  For communicating with this class
 * SetPlayerClassFromPlayerSelection should be used, and the value passed should be an ID, not an index.
 */
protected function ApplyPlayerRequestedCharacterSwitch( GearPRI WPRI, int CharacterProviderIndex )
{
	local class<GearPawn> DesiredClass;

	if ( WPRI != None && CharacterProviderIndex != INDEX_NONE )
	{
		DesiredClass = GetCharacterProviderClass(WPRI.Team.TeamIndex, CharacterProviderIndex);
		if ( DesiredClass != None )
		{
			WPRI.SetPawnClass(DesiredClass);
			WPRI.bForceNetUpdate = true;
		}
	}
}

/**
 * Called when the link status indicates the player has removed their network
 * connection. Used to mark them as losing.
 */
function SetHostTamperedWithMatch();
/** Called from AI factories when a spawn is killed */
event SpawnFromAIFactoryHasDied(Pawn KilledSpawn, SeqAct_AIFactory AIFactory);

/** Called from AI factories when a spawner had been disabled */
event AIFactoryDisabled( int NumUnspawnedPawns, SeqAct_AIFactory AIFactory );

/** Let's the game decide whether the PC should respawn or not */
function bool ShouldRespawnPC( int TeamIdx, Controller PCToRespawn )
{
	return TeamHasRespawn(TeamIdx);
}

/** Whether the team has a respawn left */
function bool TeamHasRespawn( int TeamIdx )
{
	return (NumTeamRespawns.Length > TeamIdx && NumTeamRespawns[TeamIdx] > 0);
}

/** Use a team respawn */
function UseTeamRespawn( int TeamIdx )
{
	if ( NumTeamRespawns[TeamIdx] > 0 )
	{
		NumTeamRespawns[TeamIdx]--;
		GetGRI().NumTeamRespawns[TeamIdx] = NumTeamRespawns[TeamIdx];
	}
}

/** Clear all respawns */
function ClearRespawns( int TeamIdx )
{
	NumTeamRespawns[TeamIdx] = 0;
	GetGRI().NumTeamRespawns[TeamIdx] = 0;
}

/** Stub so that we can call this from the game's PlayerController */
function AutoSeedTeams();

// ---
// Spectator point functionality
// --

/** Finds all of the spectator cameras in the map and puts them into a list to be used when in spectator mode */
protected function AcquireSpectatorPoints()
{
	local GearSpectatorPoint SpecPoint;
	local int Idx;

	// make sure the list is empty
	SpectatorPoints.length = 0;

	// Find and sort the spectator camera points
	foreach DynamicActors( class'GearSpectatorPoint', SpecPoint )
	{
		for ( Idx = 0; Idx < SpectatorPoints.length; Idx++ )
		{
			if ( SpectatorPoints[Idx].OrderIndex > SpecPoint.OrderIndex )
			{
				SpectatorPoints.Insert( Idx, 1 );
				SpectatorPoints[Idx] = SpecPoint;
				break;
			}
		}

		if ( Idx == SpectatorPoints.length )
		{
			SpectatorPoints[Idx] = SpecPoint;
		}
	}
}


/** Returns the first spectator camera if there is one available */
function GearSpectatorPoint GetFirstSpectatorPoint()
{
	if (SpectatorPoints.length == 0)
	{
		AcquireSpectatorPoints();
	}

	if ( SpectatorPoints.length > 0 )
	{
		return SpectatorPoints[0];
	}

	return None;
}

/** Returns the next spectator camera based on the one currently being viewed */
function GearSpectatorPoint GetNextSpectatorPoint( GearSpectatorPoint CurrPoint )
{
	local int Idx;
	local GearSpectatorPoint NextPoint;

	NextPoint = CurrPoint;

	if ( SpectatorPoints.length > 1 )
	{
		Idx = SpectatorPoints.Find( CurrPoint );
		if ( Idx != -1 )
		{
			NextPoint = SpectatorPoints[ ++Idx % SpectatorPoints.length ];
		}
	}

	return NextPoint;
}

/** Returns the previous spectator camera based on the one currently being viewed */
function GearSpectatorPoint GetPrevSpectatorPoint( GearSpectatorPoint CurrPoint )
{
	local int Idx;
	local GearSpectatorPoint PrevPoint;

	PrevPoint = CurrPoint;

	if ( SpectatorPoints.length > 1 )
	{
		Idx = SpectatorPoints.Find( CurrPoint );
		if ( Idx != -1 )
		{
			PrevPoint = (Idx == 0) ? SpectatorPoints[ SpectatorPoints.length-1 ] : SpectatorPoints[ Idx-1 ];
		}
	}

	return PrevPoint;
}

/** called when a pawn goes DBNO */
function NotifyDBNO(Controller InstigatedBy, Controller Victim, Pawn DownedPawn);

/** Function called when someone becomes kidnapped */
function KidnappingStarted( GearPawn Hostage, GearPawn Kidnapper );

/** Function called when someone is released from kidnapping */
function KidnappingStopped( GearPawn Hostage, GearPawn Kidnapper );


/**
 * Sends the gameplay stats to the configured services
 */
function UploadGameplayStats()
{
	local GearGameplayStatsUploader Uploader;
	local OnlineEventsInterface McpUploader;

	if (StatsObject != None)
	{
		StatsObject.EndGameplaySession();
		// Ask for the interface by name and cast to our well known type
		Uploader = GearGameplayStatsUploader(OnlineSub.GetNamedInterface('StatsUpload'));
		if (Uploader != None)
		{
			// Submit to Vince or our LSP
			Uploader.UploadStats(StatsObject);
		}
		// Ask for the interface by name and cast to our well known type
		McpUploader = OnlineEventsInterface(OnlineSub.GetNamedInterface('McpUpload'));
		if (McpUploader != None)
		{
			// Submit to our LSP
			McpUploader.UploadGameplayEventsData(StatsObject);
		}
	}
}


/**
 * Sends the gameplay stats to log files in non-release builds for debugging
 */
function DumpGameplayStats()
{
	StatsObject.DumpGameplayStats();
}


/**
 * Initializes the gameplay stats upload object
 */
event InitGame(string Options, out string ErrorMessage)
{
	local GearGameplayStatsUploader Uploader;

	Super.InitGame(Options, ErrorMessage);

	bDoLocScreenShotTest = ( ParseOption( Options, "DoLocScreenShotTest" ) ~= "1" );

	bDoAutomatedHordeTesting = ( ParseOption( Options, "AutomatedHorde" ) ~= "1" );

	if ( OnlineSub != None )
	{
		// Ask for the interface by name and cast to our well known type
		Uploader = GearGameplayStatsUploader(OnlineSub.GetNamedInterface('StatsUpload'));
		if (Uploader != None)
		{
			Uploader.Init(256);
		}
	}

	// so if either of two skip ways are true AND the "secret" enabling way is NOT true then we want to skip matinees
	bSkipAllMatinees = ( bSkipAllMatinees || ( ( (ParseOption( Options, "SkipAllMatinees") ~= "1") ) || (ParseOption( Options, "gSAM") ~= "1") )  )
		&& (ParseOption( Options, "WatchAllMatinees") != "31337");

`if(`notdefined(FINAL_RELEASE))
	bForceStats = HasOption(Options, "forcestats");
`endif

	ParseAutomatedTestingOptions( Options );

	// Allow each game type to override where that is set
	SetNetworkNotificationLocation();
}

/**
 * Allow each game type to override where that is set. The default is lower right
 */
function SetNetworkNotificationLocation()
{
	if (OnlineSub != None &&
		OnlineSub.SystemInterface != None)
	{
		OnlineSub.SystemInterface.SetNetworkNotificationPosition(NNP_BottomRight);
	}
}

/**
 * Skip this step as we don't want this
 */
function RegisterServer()
{
	// joeg - purposefully left blank
}

/** Functionality  */
event GoreSystemSpawnGibEffects(SkeletalMeshComponent GibComponent, Name GibBoneName, vector GibBoneLocation, vector GibVel, Name GibParentBoneName, GearGoreEffectInfo EffectInfo, bool bSpawnGibEffect)
{
	local Emitter GibEffect;
	local Actor GibOwner;
	local rotator GibDir;

	GibOwner = GibComponent.Owner;
	GibDir = rotator(GibVel);

	// Spawn effect to attach to bone that falls away
	if(EffectInfo.GibEffect != None && bSpawnGibEffect)
	{
		GibEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(EffectInfo.GibEffect, GibBoneLocation, GibDir);
		GibEffect.SetDrawScale(EffectInfo.EffectScale);
		GibEffect.ParticleSystemComponent.ActivateSystem();
		GibEffect.SetBase(GibOwner,, GibComponent, GibBoneName);
	}

	// Spawn effect to attach to parent bone
	if(EffectInfo.HoleEfect != None)
	{
		GibEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(EffectInfo.HoleEfect, GibBoneLocation, GibDir);
		GibEffect.SetDrawScale(EffectInfo.EffectScale);
		GibEffect.ParticleSystemComponent.ActivateSystem();
		GibEffect.SetBase(GibOwner,, GibComponent, GibParentBoneName);
	}

	// Play sound
	if(EffectInfo.LoseGibSound != None)
	{
		PlaySound(EffectInfo.LoseGibSound, TRUE, TRUE, TRUE, GibBoneLocation, TRUE);
	}
}


/**
 * Skips skill updates since the party system manages all skills for sessions
 */
function RecalculateSkillRating()
{
	// joeg - purposefully left blank
}

/**
 * Iterates all clients updating their rich presence string
 * NOTE: This does not set any parameters so only set non-parameterized strings
 *
 * @param StringId the string to set as their presence text
 */
function SetClientsRichPresenceStrings(int StringId)
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientSetRichPresenceString(StringId);
	}
}

function NotifyNavigationChanged( NavigationPoint N )
{
	local GearAI AI;
	local int	 Idx;

	if( N.bBlocked )
	{
		foreach WorldInfo.AllControllers( class'GearAI', AI )
		{
			if( !AI.bMovingToGoal )
				continue;

			Idx = AI.RouteCache.Find( N );
			if( Idx >= 0 )
			{
				AI.bReevaluatePath = TRUE;
				AI.MoveTimer = -1.f;
			}
		}
	}
}

/** @return whether all living players are DBNO, used for co-op/Horde failure conditions */
final function bool AllPlayersAreDBNO()
{
	local Controller C;
	local GearPawn P;

	foreach WorldInfo.AllControllers(class'Controller', C)
	{
		if (C.IsPlayerOwned())
		{
			`log("Is"@`showvar(C)@"DBNO?"@`showvar(C.Pawn));
			P = GearPawn(C.Pawn);
			if (P != None && !P.IsDBNO())
			{
				return FALSE;
			}
			// turrets are people too!
			if (Vehicle(C.Pawn) != None)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

final function bool TeamHasAnyChanceOfSurvival(int TeamIdx, optional GearPawn DBNOPawn)
{
	local Controller C;
	local GearPC PC;
	local GearAI AI;
	local GearPawn GP;
	local bool bFoundReviver;
	// if we are in co-op then you lose if all human players go down no matter what
	if (NumPlayers > 1 && AllPlayersAreDBNO())
	{
		`log("All players are DBNO in coop!");
		return FALSE;
	}
	// search for a DBNO player if one wasn't specified
	if (DBNOPawn == None)
	{
		foreach WorldInfo.AllPawns(class'GearPawn',GP)
		{
			if (GP.IsDBNO() && GP.IsPlayerOwned() && GP.GetTeamNum() == TeamIdx)
			{
				DBNOPawn = GP;
				break;
			}
		}
	}
	`log(WorldInfo.TimeSeconds@"Looking for team survival"@`showvar(TeamIdx)@`showvar(DBNOPawn)@`showvar(bInCoopSplit)@`showvar(bInSoloSplit));
	// if nobody is DBNO then survival is good :)
	if (DBNOPawn == None)
	{
		return TRUE;
	}
	// look for someone to revive the DBNO pawn
	bFoundReviver = FALSE;
	foreach WorldInfo.AllControllers(class'Controller', C)
	{
		if (C.GetTeamNum() == TeamIdx && C != DBNOPawn.Controller)
		{
			GP = GearPawn(C.Pawn);
			if (GP == None && GearTurret(C.Pawn) != None)
			{
				GP = GearPawn(GearTurret(C.Pawn).Driver);
			}
			if (GP == None || GP.Health <= 0 || GP.IsDBNO())
			{
				continue;
			}
			AI = GearAI(C);
			if (!bInSoloSplit && AI != None && AI.CanRevivePawn(DBNOPawn,TRUE))
			{
				if (!bInCoopSplit || AI.GetSquadName() == GearPRI(DBNOPawn.PlayerReplicationInfo).SquadName)
				{
					`log("AI:"@AI.PlayerReplicationInfo.PlayerName@"can revive:"@DBNOPawn.PlayerReplicationInfo.PlayerName);
					bFoundReviver = TRUE;
					break;
				}
			}
			else
			{
				PC = GearPC(C);
				if (PC != None && !bInCoopSplit && !bInSoloSplit)
				{
					`log("PC:"@PC.PlayerReplicationInfo.PlayerName@"can revive:"@DBNOPawn.PlayerReplicationInfo.PlayerName);
					bFoundReviver = TRUE;
					break;
				}
			}
	    }
	}
	return bFoundReviver;
}

/**
 * Tells clients to set their invite settings to the specified values
 *
 * @param bAllowInvites whether invites are allowed or not
 * @param bAllowJoinInProgress whether join in progress is allowed or not
 * @param bAllowJoinInProgressFriendsOnly whether friends only join in progress is allowed or not
 */
function TellClientsToSetPartyInviteFlags(bool bAllowInvites,bool bAllowJoinInProgress,bool bAllowJoinInProgressFriendsOnly)
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientSetPartyInviteFlags(bAllowInvites,bAllowJoinInProgress,bAllowJoinInProgressFriendsOnly);
	}
}

defaultproperties
{
	MaxPlayersAllowed=10

	ScoreBoardType=class'GearGame.GearScoreboard'
	PlayerReplicationInfoClass=class'GearGame.GearPRI'
	PlayerControllerClass=class'GearGame.GearPC'
	DefaultPawnClass=class'GearGame.GearPawn_COGMarcus'
	GameReplicationInfoClass=class'GearGame.GearGRI'
	bRestartLevel=FALSE
	bDelayedStart=TRUE

`if(`notdefined(FINAL_RELEASE))
	// For testing purposes make this only require 2
	MinRankedPlayers=2
`endif

	Begin Object Class=AIVisibilitymanager Name=VisMan0
	End Object
	AIVisMan=VisMan0

	DifficultyLevels[DL_Casual]=class'DifficultySettings_Casual'
	DifficultyLevels[DL_Normal]=class'DifficultySettings_Normal'
	DifficultyLevels[DL_Hardcore]=class'DifficultySettings_Hardcore'
	DifficultyLevels[DL_Insane]=class'DifficultySettings_Insane'
}
