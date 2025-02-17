/**
 * This is our base class for MP games.  It has a bunch of functions that are used
 * through out all of our MP game types.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameMP_Base extends GearGame
	native
	abstract
	config(Game);

`define GAMEINFO(dummy)
`include(GearGame\GearStats.uci);
`undefine(GAMEINFO)

/** Game Replication Information reference so we don't have to call a function every time we want to use it */
var GearGRI	GearGRI;

/** How long before the game to wait for players entering.  If this is a listen server, this value is ignored and
the host starts the match */
var config int AutoStartDelay;

/** How long to delay after the host begins the match */
var config int HostStartDelay;

/** How long is a single round */
var config int RoundDuration;

/** How long to wait after a round before the next one begins */
var config int EndOfRoundDelay;

/** Delay before transitioning to next game */
var config int EndOfGameDelay;

/** How long is the bleed out */
var config int InitialRevivalTime;

/** This is the first round so we want to wait some extra time to allow everyone to join **/
var() bool bFirstRound;

/** Time when the first player joined */
var float FirstPlayerJoinTime;

/** Stores off the controller of the last killer **/
var Controller LastKiller;
/** Stores off the controller of the last victim */
var Controller LastVictim;

/** Whether or not a player will auto revive on bleed out or if they will die **/
var config bool bAutoReviveOnBleedOut;

/** Whether or not a player can been shot and killed while down but not out **/
var config bool bCanBeShotFromAfarAndKilledWhileDownButNotOut;

/** Whether to allow dead people to talk to living */
var config bool bDeadCanTalkToLiving;

/** the GearTeamPlayerStart TeamIndex that each team is using for this round */
var array<int> TeamPlayerStartIndex;
/** max GearTeamPlayerStart index */
var int MaxPlayerStartTeamIndex;
/** The class of the GearTeamPlayerStart that this game type uses */
var class<GearTeamPlayerStart> GearPlayerStartClass;

/** value to assign 'MaxDownCount' for GearPawns, -1 means don't override */
var config int MaxDownCountOverride;

/** [AI] for AI names */
var localized string AIPostfix;
/** Drone_ for AI names */
var localized string AIPrefix;

/** Voice channels used by these game types */
enum EVoiceChannel
{
	VC_SpectatorsDead,
	VC_Team1,
	VC_Team2,
	VC_Team3,
	VC_Team4,
	VC_Team5
};

/** The amount of time between checks for standby cheat detection */
var config float StandbyCheatTimer;

/** Whether we are paused due to cheat detection */
var bool bIsPausedDueToCheatDetection;

/** whether we are in system link, so don't clamp client update rate, etc */
var bool bIsSystemLink;

/** Structure to store the different types of character classes, and any additional data needed for them, that the player can be */
struct native PawnClassData
{
	var class<GearPawn> PawnClass;
	var int			count;
};

/** COG team class data */
var array<PawnClassData> COGClasses;
/** Locust team class data */
var array<PawnClassData> LocustClasses;

/** Weapon lookup data used for performing weapon replacement */
struct native WeaponLookupTableData
{
	var EGearWeaponType WeapSwapType;
	var int ProfileId;
	var class<Inventory> WeapClass;
};

/** Weapon lookup table for performing weapon replacement */
var array<WeaponLookupTableData> WeaponLookupTable;

/** Whether to use weapon swapping in this game or not */
var bool bUsingWeaponSwap;
/** Whether to cycle weapons or not */
var bool bUsingWeaponCycle;

/** Final kill has happened - don't score any kills between now and end of round */
var bool bLastKillerSet;

/** Number of bots requested to play the game */
var int RequestedBots;

/** Is the player in tourist mode? */
var bool bTourist;

/** Is the game in developper mode? Options for easy testing/debugging */
var bool	bInDevMode;

/** Size of radius to do radius check against same drop type with generic weapons - 0 means they don't check */
var config float WeaponDropRadiusCheckGeneric;
/** Size of radius to do radius check against same drop type with special weapons - 0 means they don't check */
var config float WeaponDropRadiusCheckSpecial;

/** whether we should start the game immediately without waiting for the host to press Start */
var bool bQuickStart;

/** Whether this game type is playing by execution rules or not */
var bool bExecutionRules;

/** this is used to give bots unique names */
var int BotIndex;
/** if set, bots always go on team 1 and players on team 0 - @note: ONLY WORKS IN GAMETYPES WITH TWO TEAMS! */
var bool bPlayersVsBots;

/** last time a player was killed or knocked DBNO */
var float LastDownOrKillTime;

/** Whether this gametype supports sudden death or not */
var config bool bSupportsSuddenDeath;
/** Whether this gametype is in sudden death or not */
var bool bIsInSuddenDeath;
/** Amount of time for sudden death */
var config int SuddenDeathTime;
/** Whether this round can go sudden death or not */
var bool bCanGoSuddenDeathThisRound;

/** Training Grounds scenario ID (if this is -1 we are not in training) */
var int TrainingGroundsID;
/** Training ground sound to play when the timer goes off */
var string TrainingGroundsSoundPath;

/** maximum squad size for bots (bots on a team are split up into squads of this size or less) */
var int MaxSquadSize;

/** Parsed from the URL so we know which game type won */
var int GameSettingsId;

/**
 * The UniqueNetId of the Meatflag, so we can show stats/playercard for them.
 * Note: there is a native to read this from the config cache
 */
var private UniqueNetId MeatflagId;

/** Last weapon index used for weapon swapping to prevent repeat selections */
var int PreviousPreferredWeaponIdx;

/** Should planted grenades be eliminated between rounds? */
var bool bEliminatePlantedGrenades;

/** The time we started waiting for players */
var float WaitForTravelingPlayersStart;

/** The maximum amount of time to wait for players */
var config float MaxWaitForTravelingPlayers;

/** whether we're allowed to swap in a bot when a human leaves and vice-versa */
var bool bCanSwapBotsAndPlayers;
/** number of players in the game at startup time */
var int InitialPlayers;

/** @return the xuid of the meatflag */
native static function UniqueNetId GetMeatflagUniqueId();

event PreBeginPlay()
{
	Super.PreBeginPlay();

	LoadGameTypeConfigs( "MP" );
}
/**
 * @STATS
 * Allows each game type to add it's own set of game rules to the stats object
 */
function string GetGameOptionsForStats(string OptionStr)
{
	local OnlineGameSettings GameSettings;
	local int PlaylistId;

    OptionStr $= "?RoundDuration="$RoundDuration;
    if (bExecutionRules)
    {
	    OptionStr $= "?ExecutionRules=1";
	}

	// Append the playlist id for tracking
	if (GameInterface != None)
	{
		GameSettings = GameInterface.GetGameSettings('Game');
		if (GameSettings != None)
		{
			if (GameSettings.GetIntProperty(class'GearVersusGameSettings'.const.PROPERTY_PLAYLISTID,PlaylistId))
			{
				OptionStr $= "?PlaylistId="$PlaylistId;
			}
		}
	}
    return Super.GetGameOptionsForStats(OptionStr);
}


/**
 * Tells the server player to do a tutorial for training grounds
 *
 * @param TutorialType - the tutorial to attempt to process
 * @param CheckTeamType - 0 = make sure it's same team; 1 = make sure it's enemy team; -1 no check
 * @param TeamIndexToCheck - the team index to use with CheckTeamType for team testing
 */
function DoTrainingGroundTutorial(EGearTutorialType TutorialType, optional int CheckTeamType = -1, optional int TeamIndexToCheck = 0)
{
	local GearPC PC;
	local bool bAttemptTutorial;

	if (TrainingGroundsID >= 0)
	{
		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			if (PC.TutorialMgr != None)
			{
				bAttemptTutorial = (CheckTeamType == -1);
				if (!bAttemptTutorial)
				{
					if (CheckTeamType == 0)
					{
						bAttemptTutorial = TeamIndexToCheck == PC.GetTeamNum();
					}
					else
					{
						bAttemptTutorial = TeamIndexToCheck != PC.GetTeamNum();
					}
				}

				if (bAttemptTutorial)
				{
					PC.TutorialMgr.StartTutorial(TutorialType);
				}
			}
			break;
		}
	}
}

/** Tells the server player to play an anya sound for training grounds */
function DoTrainingGroundSound(string SoundPath, optional float TimeDelay = 0.0f)
{
	if (TrainingGroundsID >= 0)
	{
		TrainingGroundsSoundPath = SoundPath;
		if (TimeDelay > 0)
		{
			SetTimer(TimeDelay, false, 'StartTrainingEndOfRoundSound');
		}
		else
		{
			StartTrainingEndOfRoundSound();
		}
	}
}

function StartTrainingEndOfRoundSound()
{
	local SoundCue CueToPlay;
	local AudioComponent TutorialSoundAC;
	local GearPC LocalGPC;

	foreach LocalPlayerControllers(class'GearPC', LocalGPC)
	{
		// Stop any audio coming from the tutorial system first
		if (LocalGPC.TutorialMgr != none)
		{
			LocalGPC.TutorialMgr.StopTutorialSound();
		}

		// Find the sound cue
		CueToPlay = SoundCue(DynamicLoadObject(TrainingGroundsSoundPath, class'SoundCue'));
		if (CueToPlay != None)
		{
			// Create and set the new sound
			TutorialSoundAC = CreateAudioComponent(CueToPlay, false, true);
			if (TutorialSoundAC != none)
			{
				TutorialSoundAC.bAllowSpatialization = false;
				TutorialSoundAC.bAutoDestroy = true;
				TutorialSoundAC.bIsUISound = true;
				TutorialSoundAC.Play();
			}
		}

		break;
	}
}

/**
 * Determines the game settings object to use for the game rules based off the playlist and
 * the game type that was voted in
 */
function GearVersusGameSettings GetGameSettingsObject()
{
	local int PlaylistId, MatchMode;
	local OnlinePlaylistManager PlaylistManager;
	local GearVersusGameSettings VsSettings;

	if (GameInterface != None)
	{
		// Read the playlist from the game settings
		VsSettings = GearVersusGameSettings(GameInterface.GetGameSettings(PlayerReplicationInfoClass.default.SessionName));
		if (VsSettings != None)
		{
			VsSettings.GetIntProperty(class'GearVersusGameSettings'.const.PROPERTY_PLAYLISTID,PlaylistId);
			// If this is a public match, get the playlist specific game settings
			if (VsSettings.bUsesArbitration)
			{
				PlaylistManager = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
				if (PlaylistManager != None)
				{
					// This will find the properly configured game settings object from the playlist
					return GearVersusGameSettings(PlaylistManager.GetGameSettings(PlaylistId,GameSettingsId));
				}
			}
		}
	}

	// If the settings are null and this is a local match use the current settings from the datastore
	if ( VsSettings == None )
	{
		VsSettings = GearVersusGameSettings(GetCurrentGameSettings(true));
		if ( VsSettings != None &&
			 VsSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchMode) &&
			 (MatchMode == eGVMT_Local || MatchMode == eGVMT_SystemLink) )
		{
			return VsSettings;
		}
		else
		{
			return None;
		}
	}

	// Return whatever happens to be there
	return VsSettings;
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

	if ( MyGameSettings != None )
	{
		// System Link
		bIsSystemLink = MyGameSettings.bIsLanMatch;

		// Friendly Fire
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_FRIENDLYFIRE, IntValue) )
		{
			bAllowFriendlyFire = (IntValue == MyGameSettings.const.CONTEXT_FRIENDLYFIRE_YES) ? TRUE : FALSE;
		}

		// Round Duration
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_ROUNDTIME, IntValue) )
		{
			RoundDuration = (IntValue + 1) * 60;
		}

		// Bleedout Duration
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_BLEEDOUTTIME, IntValue) )
		{
			switch ( IntValue )
			{
				case MyGameSettings.const.CONTEXT_BLEEDOUTTIME_FIVE:
					InitialRevivalTime = 5;
					break;
				case MyGameSettings.const.CONTEXT_BLEEDOUTTIME_TEN:
					InitialRevivalTime = 10;
					break;
				case MyGameSettings.const.CONTEXT_BLEEDOUTTIME_FIFTEEN:
					InitialRevivalTime = 15;
					break;
				case MyGameSettings.const.CONTEXT_BLEEDOUTTIME_TWENTY:
					InitialRevivalTime = 20;
					break;
				case MyGameSettings.const.CONTEXT_BLEEDOUTTIME_THIRTY:
					InitialRevivalTime = 30;
					break;
				case MyGameSettings.const.CONTEXT_BLEEDOUTTIME_SIXTY:
					InitialRevivalTime = 60;
					break;
			}
		}

		// Weapon Swap
		bUsingWeaponSwap = false;
		bUsingWeaponCycle = false;
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_WEAPONSWAP, IntValue) )
		{
			// Customized weapon swapping
			if ( IntValue == MyGameSettings.const.CONTEXT_WEAPONSWAP_CUSTOM )
			{
				bUsingWeaponSwap = true;
			}
			// Weapon cycling
			else if ( IntValue == MyGameSettings.const.CONTEXT_WEAPONSWAP_CYCLE )
			{
				bUsingWeaponCycle = true;
			}
		}

		// Goal Score to win match
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_ROUNDSTOWIN, IntValue) )
		{
			GoalScore = IntValue + 1;
		}

		// Number of bots
		SetNumBots();

		return TRUE;
	}

	return FALSE;
}

/** Set the number of bots for the game */
function SetNumBots()
{
	local int IntValue;
	local GearVersusGameSettings MyGameSettings;

	MyGameSettings = GetGameSettingsObject();

	if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_NUMBOTS, IntValue) )
	{
		RequestedBots = IntValue;
	}
}

event InitGame(string Options, out string ErrorMessage)
{
	Super.InitGame(Options, ErrorMessage);

	GameSettingsId = GetIntOption(Options, "GameSettingsId", 0);

	// Training Grounds ID
	TrainingGroundsID = GetIntOption(Options, "Training", TrainingGroundsID);

	// Let the game settings set the game rules, unless we're doing training grounds
	if ( TrainingGroundsID < 0 )
	{
		if ( !SetGameRules() )
		{
			// Handle system link
			bIsSystemLink = HasOption(Options, "bIsLanMatch");

			// Handle friendly fire
			bAllowFriendlyFire = (ParseOption( Options, "FriendlyFire") != "");

			// Number of Bots
			RequestedBots = Max( 0, GetIntOption(Options,"Bots",RequestedBots) );

			CheckForExecutionRules( Options );

			// Goal Score
			if ( HasOption(Options, "GoalScore") )
			{
				GoalScore = Clamp( GetIntOption(Options, "GoalScore", GoalScore), 1, 19 );
			}
		}
	}
	else
	{
		// Number of Bots
		RequestedBots = Max( 0, GetIntOption(Options,"Bots",RequestedBots) );

		CheckForExecutionRules( Options );

		// Goal Score
		if ( HasOption(Options, "GoalScore") )
		{
			GoalScore = Clamp( GetIntOption(Options, "GoalScore", GoalScore), 1, 19 );
		}
	}

	// Handle Bleedout Time
	if ( HasOption(Options, "BleedOutTime") )
	{
		InitialRevivalTime = Clamp( GetIntOption(Options, "BleedOutTime", InitialRevivalTime), 0, 500 );
	}

	// Tourist
	if ( WorldInfo.IsPlayInEditor() || HasOption(Options, "bTourist") )
	{
		bTourist = TRUE;
	}

	// Check for DevMode option.
	bInDevMode = HasOption(Options, "DevMode");
	// Quick start
	bQuickStart = HasOption(Options, "QuickStart");
	// Players versus AI option
	bPlayersVsBots = HasOption(Options, "VsBots");
	// start delay
	AutoStartDelay = GetIntOption(Options, "StartDelay", AutoStartDelay);
	HostStartDelay = GetIntOption(Options, "StartDelay", HostStartDelay);

	// Don't use arbitration for system link or local
	if (GameInterface.GetGameSettings('Game') == None ||
		!GameInterface.GetGameSettings('Game').bUsesArbitration ||
		GameInterface.GetGameSettings('Game').bIsLanMatch)
	{
		bUsingArbitration = false;
	}

	bCanSwapBotsAndPlayers = bCanSwapBotsAndPlayers && RequestedBots > 0 &&
				(GameInterface.GetGameSettings('Game') == None || !GameInterface.GetGameSettings('Game').bUsesArbitration);
}

/**
 * Overridden to alter the tutorial type for training grounds
 *
 * @param	PC	the player that should initialize the tutorial system.
 */
function InitializeTutorialSystem( GearPC PC )
{
	local EGearAutoInitTutorialTypes TutType;

	if ( PC != None && bAllowTutorials )
	{
		TutType = AutoTutorialType;

		switch (TrainingGroundsID)
		{
			case eGEARTRAIN_Basic:
				TutType = eGAIT_Train1;
				break;
			case eGEARTRAIN_Execution:
				TutType = eGAIT_Train2;
				break;
			case eGEARTRAIN_Respawn:
				TutType = eGAIT_Train3;
				break;
			case eGEARTRAIN_Objective:
				TutType = eGAIT_Train4;
				break;
			case eGEARTRAIN_Meatflag:
				TutType = eGAIT_Train5;
				break;
		}
		// Create the Tutorial Manager
		PC.ClientInitializeTutorialSystem( TutType, true );
	}
}

/** Checks to see if execution rules should be applied to the game or not and sets it if it should */
function CheckForExecutionRules( string Options )
{
	// Must set special variables if in execution mode
	if ( HasOption(Options, "bIsExecution") )
	{
		SetExecutionRules();
		bExecutionRules = true;
	}
	else
	{
		bExecutionRules = false;
	}
}

/** Set the game for execution rules */
final function SetExecutionRules()
{
	bAutoReviveOnBleedOut = true;
	InitialRevivalTime = Max( InitialRevivalTime, 1 );
	bCanBeShotFromAfarAndKilledWhileDownButNotOut = false;
}

/** Whether a weapon is a generic drop or a special one */
function EGearDroppedPickupType GetWeaponDropType( GearWeapon Weap )
{
	if ( Weap.IsA( 'GearWeap_Shotgun' ) ||
		 Weap.IsA( 'GearWeap_AssaultRifle' ) ||
		 Weap.IsA( 'GearWeap_GrenadeBase' ) ||
		 Weap.IsA( 'GearWeap_LocustAssaultRifle' ) ||
		 Weap.IsA( 'GearWeap_PistolBase' ) )
	{
		return eGEARDROP_Generic;
	}

	return eGEARDROP_Special;
}

/** Allow Gametype to prevent weapon drops */
function bool CanDropWeapon( Weapon W, vector DropLocation )
{
	return CanDropWeaponAtLocation( W, DropLocation );
}

/** Whether we can drop a weapon at a specific location, based on whether there is a weapon of the same type within a range */
function bool CanDropWeaponAtLocation( Weapon W, vector DropLocation )
{
	local float RadiusCheck;
	local GearDroppedPickup GearDrop;

	RadiusCheck = (GetWeaponDropType(GearWeapon(W)) == eGEARDROP_Generic) ? WeaponDropRadiusCheckGeneric : WeaponDropRadiusCheckSpecial;

	if ( RadiusCheck > 0.0f )
	{
		foreach VisibleActors( class'GearDroppedPickup', GearDrop, RadiusCheck, DropLocation )
		{
			if (GearDrop.InventoryClass == W.Class && GearDrop.ClaimedBy == None)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

/** @return whether the server can pause this game when played online */
function bool AllowNetworkPause()
{
	return false;
}

function PostLogin(PlayerController NewPlayer)
{
	local GearAI_TDM Bot;

	if (LocalPlayer(NewPlayer.Player) == None)
	{
		GearPC(NewPlayer).ClientLoadGameTypeConfigs("MP");
	}

	Super.PostLogin(NewPlayer);

`if(`notdefined(dev_build))
	// disable pausing once a nonlocal player joins
	if (LocalPlayer(NewPlayer.Player) == None && !AllowNetworkPause())
	{
		bPauseable = false;
		ClearPause();
	}
`endif

	KickExtraBots();

	// force them to a team for testing
	if (!NewPlayer.PlayerReplicationInfo.bOnlySpectator)
	{
		// see if we should use this player to replace a bot
		if (bCanSwapBotsAndPlayers && GetNumPlayers() + NumBots > InitialPlayers)
		{
			foreach WorldInfo.AllControllers(class'GearAI_TDM', Bot)
			{
				if (Bot.Pawn != None)
				{
					Bot.Pawn.Suicide();
				}
				Bot.Destroy();
				break;
			}
		}

		PlacePlayerOnSmallestTeam(NewPlayer);
	}

	if (GearGRI.GameStatus == GS_RoundInProgress)
	{
		if (ShouldRespawnPC(NewPlayer.PlayerReplicationInfo.Team.TeamIndex,NewPlayer))
		{
			GearPC(NewPlayer).WaitForRespawn();
		}
		GearPC(NewPlayer).TransitionToSpectate();
		GearPRI(NewPlayer.PlayerReplicationInfo).bIsDead = TRUE;
		GearPRI(NewPlayer.PlayerReplicationInfo).PlayerStatus = WPS_Dead;
	}

	// Have the client initialize his preferred character class and weapon
	InitializePlayerMPOptions(NewPlayer);
}

/** Tells the client to set it's preferred character and weapon */
function InitializePlayerMPOptions(PlayerController NewPlayer)
{
	// Set the preferred pawn class and weapon class for this JIP player
	if (GearGRI.GameStatus == GS_RoundInProgress ||
		GearGRI.GameStatus == GS_PreMatch ||
		GearGRI.GameStatus == GS_RoundOver)
	{
		GearPC(NewPlayer).ClientSetPreferredMPOptions(NewPlayer.PlayerReplicationInfo.Team.TeamIndex % 2);
	}
}

/** Places the new player onto the smallest team */
function PlacePlayerOnSmallestTeam( PlayerController NewPlayer )
{
	local int Idx, SmallestTeam;

	SmallestTeam = 0;
	for ( Idx = 1; Idx < NumTeams; Idx++ )
	{
		if ( Teams[Idx].Size < Teams[SmallestTeam].Size )
		{
			SmallestTeam = Idx;
		}
	}

	ChangeTeam( NewPlayer, SmallestTeam, false );
}

final function RandomizeWeaponPickupFactories()
{
	local GearWeaponPickupFactory WF;
	local int Idx, PreferredIdx, HighestIdx;
	local class<Inventory> RemappedClass;
	local DroppedPickup Pickup;

	//`log(WorldInfo.TimeSeconds@GetFuncName());

	// figure out the biggest list
	HighestIdx = 0;
	foreach DynamicActors(class'GearWeaponPickupFactory', WF)
	{
		HighestIdx = Max(HighestIdx,WF.MPWeaponPickupList.Length);
	}

	// don't bother switching if there are no weapon switches in this map
	if (HighestIdx == 0)
	{
		return;
	}

	if (HighestIdx == 1)
	{
		PreferredIdx = 0;
	}
	else
	{
		// attempt to use the same idx across as many pickups as possible
		PreferredIdx = Rand(HighestIdx);
		while (PreferredIdx == PreviousPreferredWeaponIdx)
		{
			PreferredIdx = Rand(HighestIdx);
		}
	}
	PreviousPreferredWeaponIdx = PreferredIdx;
	//`log(`showvar(PreferredIdx)@`showvar(HighestIdx));
	foreach DynamicActors(class'GearWeaponPickupFactory', WF)
	{
		// If we are not using weapon cycling just set the index to 0
		Idx = bUsingWeaponCycle ? PreferredIdx : 0;
		if (WF.MPWeaponPickupList.Length > 0)
		{
			//`log("- inspecting"@WF);
			// try to share the same selection between all factories
			if (Idx == -1 || Idx >= WF.MPWeaponPickupList.Length)
			{
				Idx = Rand(WF.MPWeaponPickupList.Length);
				//`log("-> index:"@Idx);
			}

			// Allow the host's profile to remap the weapon class
			RemappedClass = RemapWeaponPickupClass(WF.MPWeaponPickupList[Idx]);
		}
		else
		{
			// check for swap on the base pickup class
			RemappedClass = RemapWeaponPickupClass(WF.WeaponPickupClass);
		}
		if (RemappedClass != WF.WeaponPickupClass)
		{
			WF.WeaponPickupClass = class<GearWeapon>(RemappedClass);
			WF.PlaceholderClass = None;
			// if it's not a GearWeapon, spawn a dropped pickup containing the item instead
			if (WF.WeaponPickupClass == None && RemappedClass != None)
			{
				Pickup = Spawn(RemappedClass.default.DroppedPickupClass,,, WF.Location, WF.Rotation);
				if (Pickup != None)
				{
					Pickup.Inventory = Spawn(RemappedClass);
					Pickup.InventoryClass = RemappedClass;
					Pickup.SetPickupMesh(Pickup.Inventory.DroppedPickupMesh);
					Pickup.SetPhysics(PHYS_None);
				}
				// mark the placeholder class so that other code can still figure out what pickup this factory presents
				WF.PlaceholderClass = RemappedClass;
			}
			WF.PickupClassChanged();
			WF.bForceNetUpdate = TRUE;
		}
	}
}

/** Returns the weapon that was swapped for WeapClassToSet */
function class<Inventory> RemapWeaponPickupClass( class<Inventory> WeapClassToSet )
{
	local GearPC PC;
	local GearProfileSettings HostProfile;
	local int Idx, ProfileId, ProfileValue, SwapIdx;

	// Is the weapon swap feature turned on?
	if ( bUsingWeaponSwap )
	{
		// Get the host's profile
		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			HostProfile = PC.ProfileSettings;
			break;
		}

		// If there's a valid profile
		if ( HostProfile != None )
		{
			// Look for a weapon swap
			for( Idx = 0; Idx < WeaponLookupTable.length; Idx++ )
			{
				// First find the profileId that will allow us to check the profile
				if ( WeapClassToSet == WeaponLookupTable[Idx].WeapClass )
				{
					ProfileId = WeaponLookupTable[Idx].ProfileId;
					if ( ProfileId != -1 )
					{
						// Now see what the profile has set
						if ( HostProfile.GetProfileSettingValueId(ProfileId, ProfileValue) )
						{
							// If the weapon is to be disabled just return none
							if ( ProfileValue == eGEARWEAP_Disabled )
							{
								return None;
							}
							// Else Loop through the list to find what the class is being swapped to
							else
							{
								for ( SwapIdx = 0; SwapIdx < WeaponLookupTable.length; SwapIdx++ )
								{
									if ( ProfileValue == WeaponLookupTable[SwapIdx].WeapSwapType )
									{
										return WeaponLookupTable[SwapIdx].WeapClass;
									}
								}
							}
						}
					}

					break;
				}
			}
		}
	}

	return WeapClassToSet;
}

/** Seed all the MP vars to the GRI */
function InitGameReplicationInfo()
{
	local GearGRI WGRI;
	local int Idx;
	local OnlineGameSettings GameSettings;

	Super.InitGameReplicationInfo();

	WGRI = GetGRI();
	if ( WGRI != None )
	{
		if (GameInterface != None)
		{
			GameSettings = GameInterface.GetGameSettings(PlayerReplicationInfoClass.default.SessionName);
			// If this is a local match, private match, or a system link match, reduce the smack talk time
			if (GameSettings == None ||
				!GameSettings.bUsesArbitration ||
				GameSettings.bIsLanMatch)
			{
				if (TrainingGroundsID < 0)
				{
					EndOfGameDelay = 10;
				}
				else
				{
					// Add more time for training grounds
					EndOfGameDelay = 20;
				}
			}
		}

		GearGRI = WGRI;		// Precache it

		WGRI.bTrackStats = true;

		WGRI.HostStartDelay		= bQuickStart ? 0 : HostStartDelay;
		WGRI.AutoStartDelay 	= AutoStartDelay;
		WGRI.EndOfRoundDelay 	= EndOfRoundDelay;
		WGRI.RoundDuration 		= RoundDuration;
		WGRI.EndOfGameDelay 	= EndOfGameDelay;
		WGRI.InitialRevivalTime = InitialRevivalTime;
		WGRI.bAllowFriendlyFire = bAllowFriendlyFire;

		WGRI.GoalScore = GoalScore;
		WGRI.TimeLimit = TimeLimit;

		for ( Idx = 0; Idx < NumTeamRespawns.length; Idx++ )
		{
			WGRI.NumTeamRespawns[Idx] = NumTeamRespawns[Idx];
		}

		WGRI.bGameIsExecutionRules = bExecutionRules;
		WGRI.NumSecondsUntilNextRound = 0;
		WGRI.TrainingGroundsID = TrainingGroundsID;
	}
}

event PostBeginPlay()
{
	Super.PostBeginPlay();

	CalculateMaxPlayerStartTeamIndex();
}

/**
 * Called on the first tick of the PendingMatch state so that we can initialized data only once,
 * but after the Sequence is initialized. This is handy for initialization code that requires
 * the kismet system to be ready (such as finding all events of type x)
 */
function InitializeGameOnFirstTick()
{
	// reset the visibility manager
	AIVisman.Reset();
}

/** Calculates the MaxPlayerStartTeamIndex which is game specific because of the number of teams on game modes like Wingman/CivilWar */
function CalculateMaxPlayerStartTeamIndex()
{
	MaxPlayerStartTeamIndex = NumTeams - 1;
}

/**
* Will randomize which team will spawn at a playerstart
*/
function ShuffleTeamPlayerStarts()
{
	local int RandomNumber, TeamIdx;

	// Clear the array
	TeamPlayerStartIndex.length = 0;

	// Calculate new indexes for all the teams.
	for ( TeamIdx = 0; TeamIdx < NumTeams; TeamIdx++ )
	{
		RandomNumber = Rand( NumTeams );
		while ( TeamPlayerStartIndex.Find(RandomNumber) != INDEX_NONE )
		{
			RandomNumber = (RandomNumber + 1) % NumTeams;
		}
		TeamPlayerStartIndex[TeamIdx] = RandomNumber;
	}
}

/** Initialized TeamInfo and other related team data */
function InitializeTeams()
{
	local int Idx;

	Super.InitializeTeams();

	// Initialize player start indexes for determining spawn points
	for (Idx = 0; Idx < NumTeams; Idx++)
	{
		TeamPlayerStartIndex[Idx] = Idx % (MaxPlayerStartTeamIndex + 1);
	}
}

/** Call as soon as a new round is queued up. */
function NewRound()
{
	local GearPC PC;
	local int Index;
	local bool bEncouragedLocalPC;

	// Send an encouragement sound cue.
	Index = Rand(255);
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		if ( LocalPlayer(PC.Player) != none  )
		{
			if (!bEncouragedLocalPC )
			{
				GearPRI(PC.PlayerReplicationInfo).EndOfRoundEncouragement(Index);
				bEncouragedLocalPC = true;
			}
		}
		else
		{
			GearPRI(PC.PlayerReplicationInfo).EndOfRoundEncouragement(Index);
		}
	}

	// reset any dangling gameplay
	if ( GearGRI.HODBeamManager != None )
	{
		GearGRI.HODBeamManager.ResetAll();
	}
}

/** Call as soon as the match has ended.  Forward the event to the clients. */
function EndMatch()
{
	local GearPC PC;
	local int EncIndex;
	local bool bEncouragedLocalPC;

	// Send an encouragement sound cue.
	EncIndex = Rand(255);
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		if ( LocalPlayer(PC.Player) != none  )
		{
			if (!bEncouragedLocalPC )
			{
				GearPRI(PC.PlayerReplicationInfo).EndOfGameEncouragement(EncIndex);
				bEncouragedLocalPC = true;
			}
		}
		else
		{
			GearPRI(PC.PlayerReplicationInfo).EndOfGameEncouragement(EncIndex);
		}
	}

	// Allow everyone to talk during map travel
	ResetGameplayMuting();
}

/** Lets the scoreboard know who was dead at the end of the round */
function PreparePostRoundScoreboardData()
{
	local Controller CurrController;
	foreach WorldInfo.AllControllers( class'Controller', CurrController )
	{
		// Allows us to know who died during the match for UI
		if ( CurrController.Pawn != None )
		{
			GearPRI(CurrController.PlayerReplicationInfo).EndOfRoundPlayerStatus = WPS_Alive;
		}
		else
		{
			GearPRI(CurrController.PlayerReplicationInfo).EndOfRoundPlayerStatus = WPS_Dead;
		}
	}
}

/** Kill anyone left alive in the match */
function KillRemainingPlayers()
{
	local Controller Player;
	local PlayerController PC;
	local Pawn OldPawn;
	local GearPRI MyGearPRI;

	// kill any players currently alive
	foreach WorldInfo.AllControllers(class'Controller', Player)
	{
		if(!Player.bIsPlayer)
		{
			continue;
		}

		if (Player.Pawn != None)
		{
			OldPawn = Player.Pawn;

			// stop any weapons fire
			Player.Pawn.StopFiring();
			// detach the pawn from the controller so that they don't stay around until the next match
			Player.UnPossess();

			if ( GearPawn(OldPawn) != None )
			{
				OldPawn.Destroy();
			}
		}

		Player.PlayerReplicationInfo.bReadyToPlay = false;
		MyGearPRI = GearPRI(Player.PlayerReplicationInfo);
		MyGearPRI.ClearDeathVariables();

		// send players to waiting state
		PC = PlayerController(Player);
		if (PC != None)
		{
			// reset the player
			PC.ClientReset();
			PC.Reset();
		}
	}
}


/** Rate whether player should choose this NavigationPoint as its start */
function float RatePlayerStart(PlayerStart P, byte Team, Controller Player)
{
	local float Score, NextDist;
	local Controller OtherPlayer;
	local GearTeamPlayerStart GearP;

	if (P == None || !P.bEnabled || P.PhysicsVolume.bWaterVolume || (P.Class != GearPlayerStartClass))
	{
		return -10000000;
	}

	if (Team < TeamPlayerStartIndex.length)
	{
		GearP = GearTeamPlayerStart(P);
		if (GearP != None && GearP.TeamIndex != TeamPlayerStartIndex[Team])
		{
			return -1000000;
		}
	}

	//assess candidate
	if ( P.bPrimaryStart )
		Score = 10000000;
	else
		Score = 5000000;

	Score += 100.0 * FRand();
	foreach WorldInfo.AllControllers(class'Controller', OtherPlayer)
	{
		if ( OtherPlayer.bIsPlayer && (OtherPlayer.Pawn != None) )
		{
			NextDist = VSize(OtherPlayer.Pawn.Location - P.Location);
			if ( VSize2D(OtherPlayer.Pawn.Location - P.Location) < 2.0 * OtherPlayer.Pawn.CylinderComponent.CollisionRadius &&
				Abs(OtherPlayer.Pawn.Location.Z - P.Location.Z) < 2.0 * OtherPlayer.Pawn.CylinderComponent.CollisionHeight )
			{
				Score -= (1000000.0 - NextDist);
			}
			else if (NextDist < 3000.0 && OtherPlayer.GetTeamNum() != Team && FastTrace(P.Location, OtherPlayer.Pawn.Location))
			{
				Score -= (10000.0 - NextDist);
			}
		}
	}
	return FMax(Score, 5);
}

/**
 * This is called once the client has sent the server their unique net id.
 * It is now possible to set up their mute list.
 *
 * @param PC the playercontroller that is ready for updates
 */
function UpdateGameplayMuteList(PlayerController PC)
{
	local GearPC WPC;

	WPC = GearPC(PC);
	if (WPC != None)
	{
		if (!bDeadCanTalkToLiving || !MatchIsInProgress())
		{
			// Put them in the spectator channel if the match hasn't started or
			// dead can't talk to living players
			InitSpectateMuteList(WPC);
		}
		else
		{
			// Respawn type that is in progress so route straight to team channel
			InitTeamMuteList(WPC);
		}
	}

	Super.UpdateGameplayMuteList(PC);
}

/** Make this player only talk to other spectators. */
function InitSpectateMuteList(GearPC PC)
{
	local GearPC OtherPC;

	PC.VoiceChannel = VC_SpectatorsDead;

	foreach WorldInfo.AllControllers(class'GearPC',OtherPC)
	{
		if (OtherPC != PC && OtherPC.PlayerReplicationInfo != None)
		{
			// spectators can't talk to non-spectating/living players
			if (OtherPC.VoiceChannel != PC.VoiceChannel)
			{
				PC.GameplayMutePlayer(OtherPC.PlayerReplicationInfo.UniqueId);
				OtherPC.GameplayMutePlayer(PC.PlayerReplicationInfo.UniqueId);
			}
			else
			{
				PC.GameplayUnmutePlayer(OtherPC.PlayerReplicationInfo.UniqueId);
				OtherPC.GameplayUnmutePlayer(PC.PlayerReplicationInfo.UniqueId);
			}
		}
	}
}

/**
 * Reset everyone's channel to the same thing and then rebuild the gameplay
 * mute lists for each player
 */
function ResetGameplayMuting()
{
	local GearPC PC;

	// Do a pass through resetting everyones voice channels
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		PC.VoiceChannel = VC_SpectatorsDead;
	}
	// Do a second pass through rebuilding the mute list
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		// Clear the gameplay mute list so everyone can talk to each other while traveling
		InitSpectateMuteList(PC);
	}
}

/**
 * Sets up the voice mute list based upon this player's team. Opponents are
 * added the specified player's mute list and vice versa.
 *
 * @param PC the player controller to initialize muting for
 */
function InitTeamMuteList(GearPC PC)
{
	local TeamInfo Team;
	local GearPC OtherPC;

	Team = PC.PlayerReplicationInfo.Team;
	// Spectators are channel 0, so use TeamIndex + 1 to select a unique voice channel
	PC.VoiceChannel = Team.TeamIndex != 255 ? Team.TeamIndex + 1 : 0;
	// Check all players
	foreach WorldInfo.AllControllers(class'GearPC',OtherPC)
	{
		if (OtherPC != PC)
		{
			// Do the same channel calculation for the other PC
			Team = OtherPC.PlayerReplicationInfo.Team;
			OtherPC.VoiceChannel = Team.TeamIndex != 255 ? Team.TeamIndex + 1 : 0;
			// If not on the same team and not in the list already, mute
			if (OtherPC.VoiceChannel != PC.VoiceChannel)
			{
				PC.GameplayMutePlayer(OtherPC.PlayerReplicationInfo.UniqueId);
				OtherPC.GameplayMutePlayer(PC.PlayerReplicationInfo.UniqueId);
			}
			else
			{
				PC.GameplayUnmutePlayer(OtherPC.PlayerReplicationInfo.UniqueId);
				OtherPC.GameplayUnmutePlayer(PC.PlayerReplicationInfo.UniqueId);
			}
		}
	}
}

/** Called when someone is killed */
function NotifyKilled(Controller Killer, Controller Killed, Pawn KilledPawn )
{
	local GearPC KilledPC;

	LastDownOrKillTime = WorldInfo.TimeSeconds;

	Super.NotifyKilled( Killer, Killed, KilledPawn );

	// If a player, move them to the "dead man talking" channel
	KilledPC = GearPC(Killed);
	if (KilledPC != None && !bDeadCanTalkToLiving)
	{
		InitSpectateMuteList(KilledPC);
	}
}

function NotifyDBNO(Controller InstigatedBy, Controller Victim, Pawn DownedPawn)
{
	LastDownOrKillTime = WorldInfo.TimeSeconds;
}

/** Returns the index of the team that should receive the next available player */
function int GetForcedTeam(Controller Other, int Team)
{
	local int Idx, TeamIdx;

	// If neutral, just leave them there.
	if ( Team == 255 )
	{
		return 255;
	}

	// Force team balancing for players
	if (Other.IsA('PlayerController') || Other.IsA('GearAI_TDM'))
	{
		if (bPlayersVsBots)
		{
			return (Other.IsA('PlayerController') ? 0 : 1);
		}
		TeamIdx = 0;
		for ( Idx = 1; Idx < Teams.Length; Idx++ )
		{
			if ( Teams[TeamIdx].TeamMembers.Length > Teams[Idx].TeamMembers.Length )
			{
				TeamIdx = Idx;
			}
		}
		return TeamIdx;
	}
	return Team;
}

/** cycles the PlayerStarts each team uses for the next round */
function CycleTeamPlayerStarts()
{
	local int i;

	for (i = 0; i < TeamPlayerStartIndex.length; i++)
	{
		TeamPlayerStartIndex[i] = (TeamPlayerStartIndex[i] + 1) % (MaxPlayerStartTeamIndex + 1);
	}
}

/** Called when a match is started */
function StartMatch()
{
	local int i;
	local GearPC PC;
	local Actor A;
	local GearGRI GGRI;
	local GearAI_TDM Bot;

	// if we didn't get the initial player count yet, do that now (happens when not using seamless travel)
	if (InitialPlayers == 0)
	{
		InitialPlayers = Clamp(GetNumPlayers() + RequestedBots, 0, MaxPlayers);
	}

	KillRemainingPlayers();

	bFirstRound = FALSE;

	foreach WorldInfo.AllControllers(class'GearAI_TDM', Bot)
	{
		if (Bot.Pawn != None)
		{
			Bot.PawnDied(Bot.Pawn);
		}
	}

	/** @todo see if we can just call ResetLevel to make this all happen */
	foreach DynamicActors(class'Actor', A)
	{
		if (A.IsA('Pawn') || A.IsA('GearDestructibleObject') || A.IsA('DroppedPickup') || A.IsA('PickupFactory'))
		{
			A.Reset();
		}
	}

	RandomizeWeaponPickupFactories();

	// Right before starting match go ahead and collect garbage
	WorldInfo.ForceGarbageCollection();

	CycleTeamPlayerStarts();

	// sanity check the team assignments for all current players
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		if (PC.PlayerReplicationInfo.Team == None || PC.PlayerReplicationInfo.Team.TeamIndex >= Teams.Length)
		{
			`log("Player with invalid team!"@PC.PlayerReplicationInfo.Team.TeamIndex);
			PlacePlayerOnSmallestTeam(PC);
		}
	}

	// try to divide up AI bots among the human players in the same team
	for (i = 0; i < Teams.length; i++)
	{
		Teams[i].CreateMPSquads();
	}

	Super.StartMatch();

	// Iterate through and set up the team specific mute lists now that
	// people are no longer spectators
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		InitTeamMuteList(PC);
	}

	// Check for the last man standing array in case a team has only 1 guy
	GGRI = GearGRI(WorldInfo.GRI);
	for (i = 0; i < Teams.length; i++)
	{
		GGRI.LastStandingOnTeamName[i] = "";
		GGRI.TeamIndexLossOrder[i] = 0;
	}
	CheckForLastMenStanding();
}

/**
 * called for MP games.
 *
 * @see GearPawn:  state Reviving
 **/
function bool AutoReviveOnBleedOut( Pawn TestPawn )
{
	return bAutoReviveOnBleedOut;
}

/**
* called for MP games.
*
* @see WarPawn:  state Reviving
**/
function bool CanBeShotFromAfarAndKilledWhileDownButNotOut( Pawn TestPawn, Pawn InstPawn, class<GearDamageType> TheDamageType )
{
	return ((TheDamageType != None && TheDamageType.static.ShouldIgnoreExecutionRules(TestPawn,InstPawn)) || bCanBeShotFromAfarAndKilledWhileDownButNotOut);
}

/** Allows a DBNO player to not take damage unless finished off (Execution Rules) */
function ReduceDamage(out int Damage, Pawn injured, Controller instigatedBy, vector HitLocation, out vector Momentum, class<DamageType> DamageType)
{
	local GearPawn GP;
	// Check for execution rules
	if (instigatedBy != None && instigatedBy.Pawn != None && !CanBeShotFromAfarAndKilledWhileDownButNotOut(injured, instigatedBy.Pawn, class<GearDamageType>(DamageType)))
	{
		GP = GearPawn(injured);
		if (GP != None && GP.IsDBNO() && !GP.IsAHostage() && VSize(instigatedBy.Pawn.Location - injured.Location) > 256.f)
		{
			Damage = 0;
			if( PlayerController(InstigatedBy) != None )
			{
				PlayerController(InstigatedBy).ClientPlaySound(SoundCue'Interface_Audio.Interface.BurstDamageWarning01Cue');
				// See if we need to play a TG tutorial
				if (TrainingGroundsID >= 0)
				{
					GearPRI(InstigatedBy.PlayerReplicationInfo).AttemptTrainingGroundTutorial(GEARTUT_TRAIN_ExeRule, eGEARTRAIN_Execution, 1, GearPRI(injured.PlayerReplicationInfo));
				}
			}
			// no need to process further since we've eliminated the damage
			return;
		}
	}

	Super.ReduceDamage(Damage,injured,instigatedBy,HitLocation,Momentum,DamageType);
}

/** Returns true if Viewer is allowed to spectate ViewTarget */
function bool CanSpectate( PlayerController Viewer, PlayerReplicationInfo ViewTarget )
{
	if ( (ViewTarget == None) || ViewTarget.bOnlySpectator )
	{
		return false;
	}

	return ( Viewer.PlayerReplicationInfo.bOnlySpectator || (ViewTarget.Team == Viewer.PlayerReplicationInfo.Team) );
}


/** Whether this player is alive or not */
function bool PlayerIsAlive(Controller Player, Controller Killed)
{
	if( Player == Killed )
	{
		return FALSE;
	}

	if( (Player != None) &&
		!Player.bDeleteMe &&
		(!Player.IsDead() || Player.IsInState('Reviving')) &&
		!Player.IsSpectating() &&
		(GearPawn(Player.Pawn) == None || !GearPawn(Player.Pawn).IsAHostage()) &&	// don't consider hostages alive players for end game conditions.
		(Player.PlayerReplicationInfo.Team.TeamIndex >= 0 && Player.PlayerReplicationInfo.Team.TeamIndex < Teams.Length) &&
		(Player.Pawn == None || Player.Pawn.HitDamageType != class'DmgType_Suicided')  && // if last hitdamage is suicide they are dead
		((GearPRI(Player.PlayerReplicationInfo).LastToKillMePRI == None) && (GearPRI(Player.PlayerReplicationInfo).DamageTypeToKillMe == None))
		)
	{
		return TRUE;
	}

	return FALSE;
}

/** @return the number of players alive on Team 0 (the good guys) */
final function int GetNumPlayersAlive(int TeamIdx)
{
	local int i, Num;

	if ( (TeamIdx >= 0) && (TeamIdx < Teams.length) )
	{
		for (i = 0; i < Teams[TeamIdx].TeamMembers.length; i++)
		{
			if (PlayerIsAlive(Teams[TeamIdx].TeamMembers[i], None))
			{
				Num++;
			}
		}
	}

	return Num;
}


/** Record the names of the last man standing on each team */
function CheckForLastMenStanding()
{
	local int TeamIdx, PlayerIdx, HighestLossCount;
	local array<int> NumPlayersAlivePerTeam;
	local array<String> AlivePlayerNames;
	local GearGRI GGRI;

	GGRI = GearGRI(WorldInfo.GRI);
	NumPlayersAlivePerTeam.length = Teams.length;
	AlivePlayerNames.length = Teams.length;

	// go through each team
	for ( TeamIdx = 0; TeamIdx < Teams.length; TeamIdx++ )
	{
		if ( Teams[TeamIdx] != None )
		{
			// go through each player
			for ( PlayerIdx = 0; PlayerIdx < Teams[TeamIdx].TeamMembers.Length; PlayerIdx++ )
			{
				// see if they are alive and have a pawn
				if ( PlayerIsAlive(Teams[TeamIdx].TeamMembers[PlayerIdx], none ) && (Teams[TeamIdx].TeamMembers[PlayerIdx].Pawn != None) )
				{
					// count them and record the name
					NumPlayersAlivePerTeam[TeamIdx]++;
					AlivePlayerNames[TeamIdx] = Teams[TeamIdx].TeamMembers[PlayerIdx].Pawn.PlayerReplicationInfo.PlayerName;
				}
			}

			// Record the highest loss count so we can set the next team to lose all of it's players to the next value
			if ( HighestLossCount < GGRI.TeamIndexLossOrder[TeamIdx] )
			{
				HighestLossCount = GGRI.TeamIndexLossOrder[TeamIdx];
			}
		}
	}

	// go through teams again
	for ( TeamIdx = 0; TeamIdx < Teams.length; TeamIdx++ )
	{
		if ( Teams[TeamIdx].TeamMembers.length <= 0 )
		{
			continue;
		}

		// if there's only one guy left, record his name
		if ( NumPlayersAlivePerTeam[TeamIdx] == 1 )
		{
			GGRI.LastStandingOnTeamName[TeamIdx] = AlivePlayerNames[TeamIdx];
		}

		// if there's no guys left and we don't have a record for the team having been killed off, record it
		if ( (NumPlayersAlivePerTeam[TeamIdx] <= 0) && (GGRI.TeamIndexLossOrder[TeamIdx] == 0) )
		{
			GGRI.TeamIndexLossOrder[TeamIdx] = HighestLossCount + 1;
		}
	}
}

/** Initiate bots */
function StartBots()
{
	local GearAI_TDM AI;
	foreach WorldInfo.AllControllers(class'GearAI_TDM', AI)
	{
		`log("starting AI:"@AI);
		RestartPlayer(AI);
		`log("- ai pawn:"@AI.Pawn@AI.GetStateName());
	}
}

/** Adds a bot to whatever team is TeamIdx - will default to non-COG team */
exec function GearAI_TDM AddBot(optional int TeamIdx = -1)
{
	local GearAI_TDM Bot;
	local GearVersusGameSettings MyGameSettings;
	local int IntValue;

	Bot = Spawn(class'GearAI_TDM');
	// auto balance if no team specified
	if (TeamIdx == -1)
	{
		TeamIdx = GetForcedTeam(Bot, 0);
	}
	// Bot Difficulty
	if (TrainingGroundsID >= 0 && TeamIdx == 0)
	{
		// in training grounds, we make the bots on the human team smarter (Normal instead of Casual)
		GearPRI(Bot.PlayerReplicationInfo).Difficulty = DifficultyLevels[1];
	}
	else
	{
		MyGameSettings = GetGameSettingsObject();
		if (MyGameSettings != None &&  MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_BOTDIFFICULTY, IntValue))
		{
			GearPRI(Bot.PlayerReplicationInfo).Difficulty = DifficultyLevels[IntValue];
		}
		else
		{
			GearPRI(Bot.PlayerReplicationInfo).Difficulty = class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(WorldInfo);
		}
	}

	Bot.PlayerReplicationInfo.PlayerName = AIPrefix $ BotIndex;
	BotIndex++;
	bSkipForcedTeamCallHack = true;
	Bot.SetTeam(TeamIdx);
	bSkipForcedTeamCallHack = false;
	SetPlayerClass(Bot, Bot.GetTeamNum());
	Bot.GotoState('Dead');
	NumBots++;

	return Bot;
}

function Logout(Controller Exiting)
{
	local byte ExitingTeam;

	ExitingTeam = Exiting.GetTeamNum();

	Super.Logout(Exiting);
	if (GearAI_TDM(Exiting) != None)
	{
		NumBots--;
	}
	// replace the player with a bot in system link or private matches
	else if (PlayerController(Exiting) != None && bCanSwapBotsAndPlayers && GetNumPlayers() + NumBots < InitialPlayers)
	{
		AddBot(ExitingTeam);
	}
}

/**
 * Call the Multiplayer version of adding inventory.
 * @see RestartPlayer
 * @see AddDefaultInventory
 * @see SetPlayerDefaults
 */
function AddDefaultInventory( Pawn P )
{
	// for DM we are going to randomize the set of weapons a pawn gets
	GearPawn(P).AddDefaultInventoryTDM();

	// don't allow them to respawn
	P.Controller.PlayerReplicationInfo.bOutOfLives = TRUE;
}

/** By default, everyone who comes in get's assgined to team 255 (spectator). */
function byte PickTeam(byte Current, Controller C)
{
	if ( bIsDedicatedListenServer || GearGRI.bMatchHasBegun )
	{
		if (bPlayersVsBots)
		{
			if (PlayerController(C) != None)
			{
				return 0;
			}
			else if (GearAI_TDM(C) != None)
			{
				return 1;
			}
		}
		return Super.PickTeam(Current, C);
	}
	else				// Always start as a spectator
	{
		return 255;
	}
}

/** if we have too many players in the game, attempt to kick out bots to make room */
function KickExtraBots()
{
	local GearAI_TDM Bot;

	if (NumPlayers + NumBots > MaxPlayers && NumBots > 0)
	{
		foreach WorldInfo.AllControllers(class'GearAI_TDM', Bot)
		{
			if (Bot.Pawn != None)
			{
				Bot.Pawn.Suicide();
			}
			Bot.Destroy();
			if (NumPlayers + NumBots <= MaxPlayers || NumBots == 0)
			{
				break;
			}
		}
	}
}

/** Called from StartMatch, this will auto-seed any spectators on to a team based on player count */
function AutoSeedTeams()
{
	local int DesiredTeam;
	local Controller C;

	KickExtraBots();

	// if the teams are unbalanced by more than one player
	if (Abs(Teams[0].Size - Teams[1].Size) > 1)
	{
		foreach WorldInfo.AllControllers(class'Controller',c)
		{
			if ( C.PlayerReplicationInfo != none && C.bIsPlayer)
			{
				// Force everyone to be ready to play
				C.PlayerReplicationInfo.bReadyToPlay = true;

//				if (C.PlayerReplicationInfo.Team == None && !C.PlayerReplicationInfo.bOnlySpectator)
				if ( (C.PlayerReplicationInfo.Team == None && !C.PlayerReplicationInfo.bOnlySpectator) ||
					 ((Abs(Teams[0].Size - Teams[1].Size) > 1) && (C.PlayerReplicationInfo.GetTeamNum() == (Teams[0].Size < Teams[1].Size?1:0))) )
				{
					if ( GearPC(C) != none && !GearPC(C).bDedicatedServerSpectator )
					{
						DesiredTeam = Teams[0].Size <= Teams[1].Size ? 0 : 1;
						ChangeTeam(C, DesiredTeam, false);
					}
				}
			}
		}
	}
}

/** Only show the lobby if we are waiting for the host */
function bool ShowLobby(GearPC LobbyTarget)
{
	return ( !LobbyTarget.bDedicatedServerSpectator && GearGRI.GameStatus == GS_WaitingForHost );
}

/** Returns the default player class for this player */
function class<Pawn> GetDefaultPlayerClass( Controller C )
{
	local GearPRI PRI;

	PRI = GearPRI(C.PlayerReplicationInfo);
	if (PRI != none)
	{
		if (PRI.PawnClass == None)
		{
			// fallback to make sure we got a valid class
			SetPlayerClass(C, C.GetTeamNum());
		}
		return PRI.PawnClass;
	}
	else
	{
		return Super.GetDefaultPlayerClass(c);
	}
}

function bool IsValidPawnClassChoice(class<GearPawn> PawnClass, int TeamNum)
{
	return true;
}

/**
 * Sets the player class for this player.  It first tries to assign a preferred class from the profile,
 * but will then attempt to find a class not used much.
 */
function SetPlayerClass(Controller Other, int N)
{
	local int i,j,l, PreferredIdx;
	local GearPRI PRI;
	local GearPC OtherPC;
	local array<PawnClassData> ClassesArray;

	OtherPC = GearPC(Other);
	PRI = GearPRI(Other.PlayerReplicationInfo);

	// Set the classes array.
	ClassesArray = (N % 2 == 0) ? CogClasses : LocustClasses;

	// See if the player has a preferred class to use.
	if (OtherPC != None)
	{
		//@todo:  need to figure how to determine when we need to allow the preferred character instead of the
		// selected one (basically anytime the game forces the player to switch teams, but not when just travelling
		// between maps)
		if ( PRI.Team == None || (PRI.Team.TeamIndex != 255 && PRI.Team.TeamIndex != N) )
		{
			PreferredIdx = (N == 0) ? PRI.PreferredCOGIndex : PRI.PreferredLocustIndex;
			if (PreferredIdx >= 0 && PreferredIdx < ClassesArray.length)
			{
				PRI.SetPawnClass(ClassesArray[PreferredIdx].PawnClass);
			}
		}
		else
		{
			SetPlayerClassFromPlayerSelection(PRI, PRI.SelectedCharacterProfileId);
		}
		// PawnClass might still not be set if they had no preference
		if (PRI.PawnClass != None)
		{
			return;
		}
	}
	else if (GearAI_TDM(Other) != None)
	{
		// use the AI's previous class, if any
		PreferredIdx = (N == 0) ? PRI.PreferredCOGIndex : PRI.PreferredLocustIndex;
		if (PreferredIdx >= 0 && PreferredIdx < ClassesArray.length)
		{
			PRI.SetPawnClass(ClassesArray[PreferredIdx].PawnClass);
		}
		if (PRI.PawnClass != None)
		{
			return;
		}
	}

	// Reset Count
	for ( i = 0; i < ClassesArray.Length; i++ )
	{
		ClassesArray[i].Count = 0;
	}

	// Generate Counts
	for ( i = 0; i < GameReplicationInfo.PRIArray.Length; i++ )
	{
		PRI = GearPRI(GameReplicationInfo.PRIArray[i]);
		if ( PRI != none )
		{
			for ( j = 0; j < ClassesArray.length; j++ )
			{
				if ( PRI.PawnClass == ClassesArray[j].PawnClass )
				{
					ClassesArray[j].Count++;
				}
			}
		}
	}

	// Find the best Class
	j = Rand( ClassesArray.Length );
	l = j;
	while (ClassesArray[j].Count > 0 || !IsValidPawnClassChoice(ClassesArray[j].PawnClass, N))
	{
		j = (j + 1) % ClassesArray.Length;
		if ( j == l )
		{
			break;
		}
	}

	PRI = GearPRI(Other.PlayerReplicationInfo);
	PRI.SetPawnClass(ClassesArray[j].PawnClass);

	if (Other.IsA('GearAI_TDM'))
	{
		// save this class as the AI's preference so that if the game clobbers them (e.g. leader change) we can recover them
		if (N == 0)
		{
			PRI.PreferredCOGIndex = j;
		}
		else
		{
			PRI.PreferredLocustIndex = j;
		}
		// make the AI's name match its character
		ChangeName(Other, ClassesArray[j].PawnClass.default.CharacterName @ AIPostfix, false);
	}
}

/** Called when a player first enters the server */
event PlayerController Login(string Portal, string Options, out string ErrorMessage)
{
	local PlayerController PC;
	local GearPC WPC;

	// First allow the super classes to do their thing, they will spawn a playercontroller
	// and take care of other fun stuff.
	PC = Super.Login( Portal, Options, ErrorMessage );

	// Set the newly created player to the neutral team position.
	WPC = GearPC(PC);
	if ( WPC != none && WPC.bDedicatedServerSpectator )
	{
		ChangeTeam( WPC,255,false );
	}

	return PC;
}

/** Optional handling of ServerTravel for network games. Overwritten here so that we can set the game status. */
function ProcessServerTravel(string URL, optional bool bAbsolute)
{
	local Pawn P;
	// destroy all pawns before travelling
	foreach WorldInfo.AllPawns(class'Pawn',P)
	{
		P.Destroy();
	}
	GearGRI.SetGameStatus( GS_Loading );
	Super.ProcessServerTravel( URL, bAbsolute );
}

event PostSeamlessTravel()
{
	Super.PostSeamlessTravel();

	InitialPlayers = Clamp(GetNumPlayers() + RequestedBots, 0, MaxPlayers);
}

event HandleSeamlessTravelPlayer(out Controller C)
{
	local GearPRI GPRI;
`if(`notdefined(dev_build))
	local PlayerController PC;
`endif

	GearPC(C).ClientLoadGametypeConfigs("MP");

	if (C.IsA('GearAI'))
	{
		if (C.IsA('GearAI_TDM'))
		{
			NumBots++; // compensates for decrement in Logout()
		}
		C.Destroy();
	}
	else
	{
`if(`notdefined(dev_build))
		// disable pausing once a nonlocal player joins
		PC = PlayerController(C);
		if (PC != None && LocalPlayer(PC.Player) == None && !AllowNetworkPause())
		{
			bPauseable = false;
			ClearPause();
		}
`endif
		Super.HandleSeamlessTravelPlayer(C);

		GPRI = GearPRI(C.PlayerReplicationInfo);
		if ( GPRI != None && GPRI.PawnClass == None )
		{
			SetPlayerClass(C, C.GetTeamNum());
		}

		KickExtraBots();
	}
}

function SetSeamlessTravelViewTarget(PlayerController PC)
{
	// do nothing in this case since it's handled already
}

/** Returns the string name of the next map in the map cycle located in the player's profile. */
function string GetNextMap()
{
	`Log("Someone please fix GetNextMap()");
	return WorldInfo.GetMapName();
}

/**
 * Called when the link status indicates the player has removed their network
 * connection. Used to mark them as losing.
 */
function SetHostTamperedWithMatch()
{
	bHostTamperedWithMatch = true;
}

/**
 * Tells each client to stream their teammates textures in for a number of seconds.
 */
function PrestreamTeamTextures()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientPrestreamTeamSkins(GearGRI.RoundTime + 5.0);
	}
}

/** Trigger the SeqEvt_MPRoundStart event */
function TriggerRoundStartEvent( bool bIsCountdown )
{
	TriggerEventClass( class'SeqEvt_MPRoundStart', None, bIsCountdown?0:1 );
}

/** Trigger the SeqEvt_MPRoundEnd event */
function TriggerRoundEndEvent()
{
	TriggerEventClass( class'SeqEvt_MPRoundEnd', None );
}

/** Makes the host ready */
function ForceHostReady()
{
	local GearPC PC;

	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		if ( PC.PlayerReplicationInfo != None )
		{
			GearPRI(PC.PlayerReplicationInfo).HostIsReady();
		}
	}
}

/**
 * Wrapper for determining whether all required players are ready to play.
 */
function bool ArePlayersReady()
{
	local PlayerController PC;
	local GearPRI PRI;

	// look for the host ready flag
	foreach WorldInfo.AllControllers(class'PlayerController',PC)
	{
		PRI = GearPRI(PC.PlayerReplicationInfo);

		if (PRI != None && PRI.bHostIsReady)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/** Check for minimum number of players */
function bool ArePlayersNeeded()
{
	// check for min players needed for a nonranked match
	return (!bTourist && NumPlayers + RequestedBots < MinPlayers);
}

/**
* Checks to see if the match is ready to be started, and starts it if so.  Here is what signals that the
* match is ready to start.
*/
function CheckStartMatch()
{
	if ( !ArePlayersNeeded() )
	{
		GearGRI.bHasEnoughPlayers = true;
		if (ArePlayersReady())
		{
			GotoState('PreRound');
		}
		else
		{
			//`log("players aren't ready");
		}
	}
	else
	{
		//`log("need more players");
		// Since we don't have enough people to play, we keep resetting the Autostart clock
		GearGRI.bHasEnoughPlayers = false;
		GearGRI.ResetRoundClock(GearGRI.AutoStartDelay);
	}
}

/** Called for each new round to clean up any actors that shouldn't persist for the entire match. */
function ResetLevelForNewRound(float MinShieldLifetime)
{
	local DynamicSMActor_Spawnable SMActor_Spawnable;
	local CoverLink_Spawnable Link_Spawnable;
	local GearProj_Grenade Grenade;
	local Actor Actor;
	local GearDroppedPickup DroppedPickup;
	local GearPC PC;

	foreach DynamicActors(class'Actor',Actor)
	{
		// destroy any spawned static meshes
		SMActor_Spawnable = DynamicSMActor_Spawnable(Actor);
		if (SMActor_Spawnable != None)
		{
			SMActor_Spawnable.Destroy();
			continue;
		}
		// clean up any spawned cover
		Link_Spawnable = CoverLink_Spawnable(Actor);
		if (Link_Spawnable != None)
		{
			// links will destroy slot markers as well so no need to worry about cleaning those up
			Link_Spawnable.Destroy();
			continue;
		}
		// clean up prox grenades
		if (bEliminatePlantedGrenades)
		{
			Grenade = GearProj_Grenade(Actor);
			if (Grenade != None)
			{
				Grenade.Destroy();
				continue;
			}
		}
		DroppedPickup = GearDroppedPickup(Actor);
		if (DroppedPickup != None)
		{
			//@hack: for Horde's delayed pickup destruction timer
			// shields (which always use dropped pickups) have already been spawned at the beginning
			// but this is called later on a timer
			// so don't destroy recently spawned shields
			if (!DroppedPickup.IsA('GearDroppedPickup_Shield') || WorldInfo.TimeSeconds - DroppedPickup.CreationTime > MinShieldLifetime)
			{
				DroppedPickup.Destroy();
				continue;
			}
		}
	}

	// ifdef out for ship
	// so if we are auto hording then start the killing in N seconds
	if (bDoAutomatedHordeTesting)
	{
		foreach WorldInfo.AllControllers( class'GearPC', PC )
		{
			PC.ClearTimer( nameof(PC.Automation_KillAMob) );
			PC.SetTimer( 5.0f, FALSE, nameof(PC.Automation_KillMobs) );
		}
	}

	if( bCheckingForMemLeaks )
	{
		foreach WorldInfo.AllControllers( class'GearPC', PC )
		{
			PC.Automation_MemoryChecking();
		}
	}

}

function bool ShouldRespawn(PickupFactory Other)
{
	return true;
}

function bool AllowHealthRecharge(GearPawn Pawn)
{
	// always allow recharge in MP, even for AI bots
	return true;
}

/** Put the game into sudden death if we have a 1 on 1 situation */
function CheckForSuddenDeath( array<Controller> AlivePlayers )
{
	if ( bCanGoSuddenDeathThisRound && !bIsInSuddenDeath )
	{
		if ( (AlivePlayers.length == 2) && (AlivePlayers[0].GetTeamNum() != AlivePlayers[1].GetTeamNum()) && (!bInfiniteRoundDuration && GetGRI().RoundTime > SuddenDeathTime) )
		{
			EnterSuddenDeath();
		}
	}
}

// See if this round can go sudden death
function CheckForSuddenDeathAvailability()
{
	bCanGoSuddenDeathThisRound = false;

	// Disable sudden death when in dev mode.
	if( bSupportsSuddenDeath && !bInDevMode )
	{
		if ( GetGRI().PRIArray.length > 2 )
		{
			bCanGoSuddenDeathThisRound = true;
		}
	}
}

/** Put the game into sudden death */
function EnterSuddenDeath()
{
	local GearPC PC;

	bIsInSuddenDeath = true;
	GetGRI().RoundTime = SuddenDeathTime;

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		PC.ClientStartSuddenDeathText();
	}
}

/** Clear the sudden death data */
function ClearSuddenDeath()
{
	bIsInSuddenDeath = false;
}

/** sends all players to the spectating state */
final function SendAllPlayersToSpectating()
{
	local Controller C;

	foreach WorldInfo.AllControllers(class'Controller', C)
	{
		if (C.IsA('GearPC') || C.IsA('GearAI_TDM'))
		{
			if (C.Pawn != None)
			{
				C.Pawn.bNoWeaponFiring = true;
			}
			C.SetTimer( class'GearHUDMP_Base'.default.TotalEndRoundTime, false, 'TransitionToSpectateFromEndOfRound' );
		}
	}
}

/** @return Actor the AI should move towards to complete game objectives (if any) */
function Actor GetObjectivePointForAI(GearAI AI);
/** @return whether the AI needs to stand on top of the objective point for it to count */
function bool MustStandOnObjective(GearAI AI);
/** sets the appropriate combat mood for the AI based on the current game state */
function SetAICombatMoodFor(GearAI AI);
/** adjust the AI's rating of the given enemy */
function AdjustEnemyRating(GearAI AI, out float out_Rating, Pawn EnemyPawn);


function SetPlayerDefaults(Pawn PlayerPawn)
{
	local GearPawn GP;

	if(MaxDownCountOverride != -1)
	{
		GP = GearPawn(PlayerPawn);
		if ( GP != None )
		{
			GP.MaxDownCount = MaxDownCountOverride;
		}
	}
	Super.SetPlayerDefaults(PlayerPawn);

	// in the tutorial give the human more health
	if (TrainingGroundsID != INDEX_NONE && PlayerPawn.IsHumanControlled())
	{
		GP = GearPawn(PlayerPawn);
		if (GP != None)
		{
			GP.DefaultHealth *= 1.5;
			GP.HealthMax *= 1.5;
			GP.Health = GP.DefaultHealth;
		}
	}
}

/** Changes the invite state to allow JIP for private matches */
function AllowJIPForPrivateMatch()
{
	local OnlineGameSettings GameSettings;

	if (GameInterface != None)
	{
		GameSettings = GameInterface.GetGameSettings('Game');
		// If this match is not arbitrated, let people invite & join in progress
		if (GameSettings != None && !GameSettings.bUsesArbitration)
		{
			GameSettings = GameInterface.GetGameSettings('Party');
			if (GameSettings != None && !GameSettings.bAllowInvites)
			{
				TellClientsToSetPartyInviteFlags(true,false,true);
			}
		}
	}
}

/************************************************************************
 *						PreRound State Begin
 * This is the State that the game will enter when between rounds.  It does nothing
 * but watch the RoundTime.  When it hits 0, the round begins via StartMatch()
================================================================================ */
state PreRound
{
	// Notify the GRI of the Game's status.  We make a distinction regarding round 0
	// just in case we need to utilize it elsewhere
	function BeginState(Name PreviousStateName)
	{
		bHasArbitratedHandshakeBegun = true;

		PrepareNextRound();

		// Place any stragglers on a team.
		//AutoSeedTeams();

		// Tell all clients to prestream their team's textures
		PrestreamTeamTextures();

		// If this is private, allow JIP to happen
		AllowJIPForPrivateMatch();

		// Have players register with the object that tracks stats for the post game results, so we can track drops
		SetTimer(1.0,false,nameof(InitLastMatchResults));

		// Send an event to kismet that the countdown for the start of a round is beginning
		TriggerRoundStartEvent( true );

		if (GearGRI.RoundCount == 0)
		{
			// See if we should play a TG sound for intro
			DoTrainingGroundSound("Human_Anya_Chatter_Cue.TutorialIntro.AnyaChatter_TutorialIntro01Cue");
		}
	}

	/** Prepare for the next round */
	function PrepareNextRound()
	{
		if ( GearGRI.RoundCount == 0 )
		{
			GearGRI.SetGameStatus(GS_PreMatch);
		}
		else
		{
			// set the game status
			GearGRI.SetGameStatus(GS_RoundOver);
			SendAllPlayersToSpectating();
			NewRound();
			`log("New round in:"@EndOfRoundDelay);
		}
	}

	/** See if the match should start yet */
	function CheckStartRound()
	{
		if ( GearGRI.RoundTime <= 0 )
		{
			// clean up any actors created in the last round
			ResetLevelForNewRound(0.0);
			// start the match
			StartMatch();
		}
	}

Begin:
	// Moved from PendingMatch to here so all networked players have a chance
	// to make it before bots are balanced on teams
	if (GearGRI.RoundCount == 0)
	{
		while (RequestedBots > NumBots && NumPlayers + NumBots < MaxPlayers)
		{
			AddBot();
		}
	}
	Sleep(0.2f);
	CheckStartRound();
	Goto('Begin');
}
/************************************************************************
*						PreRound State End
************************************************************************/

/************************************************************************
 *						PendingMatch State Begin
 *	This is the State that the game will be in before a match is ready to play.
 *	We will use this state to watch for either the time-out, Host/AllPlayers being
 *	ready and if it occurs, we will transition in to PreMatch.  The game will only
 *	enter this state once (the first time).  Each additional time the game will
 *	enter PreMatch.
 ************************************************************************/
auto state PendingMatch
{
	function BeginState(Name PreviousStateName)
	{
		// don't allow the match to start yet
		if (GameReplicationInfo != None)
		{
			GameReplicationInfo.bMatchHasBegun = FALSE;
		}

		bWaitingToStartMatch = TRUE;

		KillRemainingPlayers();
		AcquireSpectatorPoints();

		// Signal the GRI that we are in PendingMatch
		GearGRI.SetGameStatus(GS_WaitingForHost);

		if (!bUsingArbitration)
		{
			SetTimer((WorldInfo.NetMode == NM_Standalone) ? 0.1f : 10.0f, false, nameof(ForceHostReady));
		}
	}

	/**
	 * After transitioning from the pregame lobby, it checks that all players have
	 * arrived, that their PRIs are set up properly, and that they have acked the
	 * server for the PC type transition
	 */
	function CheckStartMatch()
	{
		local int ReadyPlayers;
		local int StalePCCount;
		local int Index;
		local UniqueNetId ZeroId;
		local PlayerController PC;
		local OnlineRecentPlayersList RecentPlayersList;

		// When this is zero, everyone is in the game
		if (NumTravellingPlayers == 0)
		{
			RecentPlayersList = OnlineRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
			// Iterate the controller list and tell them to register with arbitration
			foreach WorldInfo.AllControllers(class'PlayerController', PC)
			{
				// We need to wait until all stale PCs have been removed
				if (PC.class == class'GearPreGamePC')
				{
					StalePCCount++;
				}
			}
			// Don't bother checking as long as there are PCs from the previous level around
			if (StalePCCount == 0)
			{
				// Count the number of players that are ready (replicated IDs, etc.)
				for (Index = 0; Index < GameReplicationInfo.PRIArray.Length; Index++)
				{
					if (!GameReplicationInfo.PRIArray[Index].bBot &&
						!GameReplicationInfo.PRIArray[Index].bFromPreviousLevel &&
						GameReplicationInfo.PRIArray[Index].UniqueId != ZeroId)
					{
						ReadyPlayers++;
					}
				}
			}
			// The number of public connections shrinks when players logout, so this is safe/correct to check
			if (RecentPlayersList != None && ReadyPlayers >= RecentPlayersList.GetCurrentPlayersListCount())
			{
				// Start if everyone has made it
				GearGRI.bHasEnoughPlayers = true;
				GotoState('PreRound');
			}
		}
		else
		{
			// Not everyone has travelled yet
			GearGRI.bHasEnoughPlayers = false;
			GearGRI.ResetRoundClock(GearGRI.AutoStartDelay);
		}
		// Always start if they haven't traveled in time (no deadlocks)
		if (WorldInfo.TimeSeconds - WaitForTravelingPlayersStart > MaxWaitForTravelingPlayers)
		{
			`Log("Starting match without waiting for players due to timeout");
			GearGRI.bHasEnoughPlayers = true;
			GotoState('PreRound');
		}
	}

Begin:
	InitializeGameOnFirstTick();
	// Start tracking how long we've waited for players
	WaitForTravelingPlayersStart = WorldInfo.TimeSeconds;

BeginAgain:
	// check if we should start without waiting for input
	if (bAutomatedPerfTesting || bQuickStart)
	{
		GearGRI.bHasEnoughPlayers = true;
		GotoState('PreRound');
	}
	// Don't wait for remote players to fully transition if we are standalone or local only
	if (WorldInfo.NetMode == NM_Standalone || GameInterface == None || GameInterface.GetGameSettings('Game') == None)
	{
		Global.CheckStartMatch();
	}
	else
	{
		CheckStartMatch();
	}
	Sleep(0.5f);	// Up the frequency so it's more responsive since we aren't doing actual timing here
	Goto('BeginAgain');
}
/************************************************************************
 *						PendingMatch State End
 ************************************************************************/

/************************************************************************
 *						MatchInProgress State Begin
 *	This is the state the game will be in while the match is in
 *  progress. The game will remain in this state until the end of match
 *  condition is met.
 ************************************************************************/
state MatchInProgress
{
	/** Overridden to update the game status. */
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		`RecordStat(STATS_LEVEL1,'RoundBegun',None,GearGRI.RoundCount + 1);

		GearGRI.SetGameStatus(GS_RoundInProgress);

		if ( RespawnTimeInterval > 0 )
		{
			GetGRI().RespawnTime = RespawnTimeInterval;
			SetTimer( 1.0f, TRUE, nameof(RespawnTimerUpdate) );
		}

		// beginning of round comments
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_VersusRoundBegunCOG, None,, 1.f);
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_VersusRoundBegunLocust, None,, 1.f);

		// Send an event to kismet that the round is beginning
		TriggerRoundStartEvent( false );

		// See if this round can go sudden death
		CheckForSuddenDeathAvailability();

		bLastKillerSet = false;

		if (TrainingGroundsID >= 0 && GearGRI.RoundCount == 0)
		{
			SetTimer(4.0f, false, 'DoIntroTutorial');
		}
	}

	function DoIntroTutorial()
	{
		local EGearTutorialType TutType;

		switch (TrainingGroundsID)
		{
			case eGEARTRAIN_Basic:		TutType = GEARTUT_TRAIN_WarWelcome; break;
			case eGEARTRAIN_Execution:	TutType = GEARTUT_TRAIN_ExeWelcome; break;
			case eGEARTRAIN_Respawn:	TutType = GEARTUT_TRAIN_GuarWelcome; break;
			case eGEARTRAIN_Objective:	TutType = GEARTUT_TRAIN_AnxWelcome; break;
			case eGEARTRAIN_Meatflag:	TutType = GEARTUT_TRAIN_MeatWelcome; break;

		}
		DoTrainingGroundTutorial(TutType);
	}

	/** Overridden to update the game status. */
	simulated function EndState(Name NextStateName)
	{
		local int i;

		Super.EndState(NextStateName);

		// Write out Road Run Times/etc for this round
		for (i=0;i<GameReplicationInfo.PRIArray.Length;i++)
		{
			GearPRI(GameReplicationInfo.PRIArray[i]).RecordEndRound();
		}


		`RecordStat(STATS_LEVEL1,'RoundEnded');
		ClearTimer('CheckEndMatch');
		ClearTimer( 'RespawnTimerUpdate' );

		// Clear the sudden death data
		ClearSuddenDeath();

		// Send an event to kismet that the round ended
		TriggerRoundEndEvent();

		// Lets the scoreboard know who was dead at the end of the round
		PreparePostRoundScoreboardData();
	}

	/** Called every RespawnTimeInterval to respawn dead players */
	function RespawnTimerUpdate()
	{
		local GearGRI GGRI;
		GGRI = GetGRI();
		GGRI.RespawnTime--;
		GGRI.RespawnTimeUpdated();
		if ( GGRI.RespawnTime <= 0 )
		{
			GGRI.RespawnTime = RespawnTimeInterval;
			BroadcastRespawnMessage();
		}
	}

	/** Tells players that they can respawn */
	function BroadcastRespawnMessage()
	{
		local Controller C;
		local GearPC PC;

		foreach WorldInfo.AllControllers(class'Controller', C)
		{
			PC = GearPC(C);
			if ( (PC != None && PC.bWaitingToRespawn) ||
				(C.IsA('GearAI_TDM') && C.IsDead() && ShouldRespawnPC(C.GetTeamNum(), C)) )
			{
				RestartPlayer(C);
			}
		}
	}

	/** See if the condition has been met to end the round */
	function bool MatchEndCondition( Controller Killed )
	{
		local int TeamIdx, Idx, NumTeamsAlive;
		local Controller Player;

		// for each team,
		for (TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++)
		{
			if( Teams[TeamIdx] == None )
			{
				continue;
			}
			// check for alive players
			for (Idx = 0; Idx < Teams[TeamIdx].TeamMembers.Length; Idx++)
			{
				Player = Teams[TeamIdx].TeamMembers[Idx];
				if ( PlayerIsAlive(Player, Killed) || TeamHasRespawn(Player.PlayerReplicationInfo.Team.TeamIndex) || (GearPC(Player) != None && GearPC(Player).bWaitingToRespawn) )
				{
					NumTeamsAlive++;
					break;
				}
			}
		}

		return (NumTeamsAlive <= 1);
	}

	/** Checks to see if only one team is surviving.  And if so, do some end of match cleanup etc. */
	function bool CheckEndMatch( optional Controller Killed )
	{
		local int TeamIdx, Idx, NumTeamsAlive, WinningTeamIdx;
		local Controller Player;
		local array<Controller> AlivePlayers;
		local bool bTeamIsAlive;

		// default to draw
		WinningTeamIdx = -1;

		// for each team,
		for (TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++)
		{
			if( Teams[TeamIdx] == None )
			{
				continue;
			}

			bTeamIsAlive = false;

			// check for alive players
			for (Idx = 0; Idx < Teams[TeamIdx].TeamMembers.Length; Idx++)
			{
				Player = Teams[TeamIdx].TeamMembers[Idx];
				if ( PlayerIsAlive(Player, Killed) || TeamHasRespawn(Player.PlayerReplicationInfo.Team.TeamIndex) || (GearPC(Player) != None && GearPC(Player).bWaitingToRespawn) )
				{
					bTeamIsAlive = true;
					AlivePlayers.AddItem(Player);
				}
			}

			// Mark the team as being alive
			if ( bTeamIsAlive )
			{
				NumTeamsAlive++;
				WinningTeamIdx = TeamIdx;
			}
		}

		// if both teams are dead (e.g. A kills B and B managed to kill A with delayed explosion, then we need to make certain that the one who fired first gets credit)
		if ( NumTeamsAlive == 0 )
		{
			if ( LastKiller != None && LastKiller.PlayerReplicationInfo != None && LastKiller.PlayerReplicationInfo.Team != None)
			{
				WinningTeamIdx = LastKiller.PlayerReplicationInfo.Team.TeamIndex;
			}
		}

		if( NumTeamsAlive <= 1 )
		{
			return HandleTeamWin( WinningTeamIdx );
		}
		else
		{
			CheckForSuddenDeath( AlivePlayers );
		}

		// Check to see if the round is over
		if ( !bInfiniteRoundDuration )
		{
			// See if round is over
			if ( GearGRI.RoundTime <= 0 )
			{
				return HandleRoundTimerExpired();
			}
		}

		return false;
	}

	function bool HandleTeamWin( int WinningTeamIdx )
	{
		local int TeamIdx, WinningTeamIdxOverride;
		local Controller Player;

		if (WinningTeamIdx == -1)
		{
			GameReplicationInfo.Winner = None;
			GearGRI(GameReplicationInfo).FinalVictim = LastVictim.PlayerReplicationInfo;
		}
		else
		{
			GameReplicationInfo.Winner = Teams[WinningTeamIdx];
			GearGRI(GameReplicationInfo).FinalVictim = LastVictim.PlayerReplicationInfo;
			if ( LastKiller != None )
			{
				GearGRI(GameReplicationInfo).FinalKiller = LastKiller.PlayerReplicationInfo;
			}

			`log("Team"@WinningTeamIdx+1@"has won the round!");

			// award the score to the winning team
			Teams[WinningTeamIdx].Score += 1;

			// See if there is still a team to player.
			for ( TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++ )
			{
				if ( TeamIdx != WinningTeamIdx )
				{
					if ( Teams[TeamIdx].TeamMembers.Length > 0 )
					{
						// Found a non-winning team with players on it so we're good for next round.
						break;
					}
				}
			}

			// end the game if no one left on losing teams
			if ( TeamIdx >= Teams.Length )
			{
				// Award the game to another team if the host cheated.
				if ( bHostTamperedWithMatch )
				{
					Teams[WinningTeamIdx].Score = 0;
					// Swap the winning team with a losing team... grab the first loser.
					// @TODO: We need to base the winner off of the last loser alive.
					for ( TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++ )
					{
						if ( WinningTeamIdx != TeamIdx )
						{
							WinningTeamIdx = TeamIdx;
							break;
						}
					}
					GameReplicationInfo.Winner = Teams[WinningTeamIdx];
				}
				// Set the winning team to win all round since there is no more opponents.
				Teams[WinningTeamIdx].Score = GoalScore;
			}

			// Give the game type a chance to override who actually wins the game
			WinningTeamIdxOverride = HandleWinningTeamOverride( WinningTeamIdx );

			if ( (WinningTeamIdxOverride != -1) && (WinningTeamIdxOverride != WinningTeamIdx) )
			{
				WinningTeamIdx = WinningTeamIdxOverride;
				GameReplicationInfo.Winner = Teams[WinningTeamIdx];
			}
			// send a notification for certain gametypes to handle
			TeamHasWon(WinningTeamIdx);
		}

		if( WinningTeamIdx != -1  && (WinningTeamIdxOverride != -1) && (Teams[WinningTeamIdx].Score >= GoalScore) && CheckEndGame(LastKiller.PlayerReplicationInfo , "LastMan") )
		{
			// record stats for win/loss
			foreach WorldInfo.AllControllers(class'Controller',Player)
			{
				if (Player.PlayerReplicationInfo != None && Player.PlayerReplicationInfo.Team == Teams[WinningTeamIdx])
				{
					`RecordStat(STATS_LEVEL1,'WonMatch',Player);
				}
				else
				{
					`RecordStat(STATS_LEVEL1,'LostMatch',Player);
				}
			}
			// See if we should play a TG sound for end of match
			if (TrainingGroundsID >= 0)
			{
				PlayEndOfRoundTrainingSound(true, WinningTeamIdx);
			}
			GotoState('MatchOver');
			return true;
		}
		else
		{
			GetGRI().RoundCount = (GetGRI().RoundCount + 1) % 100;
			// See if we should play a TG sound for end of round
			if (TrainingGroundsID >= 0)
			{
				PlayEndOfRoundTrainingSound(false, WinningTeamIdx);
			}
			GotoState('PreRound');
			return true;
		}
	}

	/** Plays a training grounds sound based on the scenario */
	function PlayEndOfRoundTrainingSound(bool bIsEndOfMatch, int WinTeamIndex)
	{
		local GearPC LocalGPC;
		local int NumCompletedTraining;

		foreach LocalPlayerControllers(class'GearPC', LocalGPC)
		{
			// end of round
			if (!bIsEndOfMatch)
			{
				// we've won
				if (WinTeamIndex == LocalGPC.GetTeamNum())
				{
					if (TrainingGroundsID == eGEARTRAIN_Basic || TrainingGroundsID == eGEARTRAIN_Execution)
					{
						// first win
						if (Teams[WinTeamIndex].Score == 1)
						{
							DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone07Cue", 10.0f);
						}
						// second win
						else
						{
							DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone08Cue", 10.0f);
						}
					}
				}
				// we've lost
				else
				{
					DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone11Cue", 10.0f);
				}
			}
			// end of match
			else
			{
				if (LocalGPC.ProfileSettings != none)
				{
					NumCompletedTraining = LocalGPC.ProfileSettings.GetNumCompletedTraining();
				}

				// we've won
				if (WinTeamIndex == LocalGPC.GetTeamNum())
				{
					// we won and did all training missions
					if (NumCompletedTraining >= eGEARTRAIN_MAX)
					{
						DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone09Cue", 10.0f);
					}
					// we won but haven't done all training missions yet
					else
					{
						DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone10Cue", 10.0f);
					}
				}
				// we've lost
				else
				{
					// we lost and have only played 2 different missions
					if (NumCompletedTraining <= 2)
					{
						DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone15Cue", 10.0f);
					}
					// we lost and haven't done all training missions yet
					else
					{
						DoTrainingGroundSound("Human_Anya_Chatter_Cue.Warzone.AnyaChatter_Warzone14Cue", 10.0f);
					}
				}
			}
			break;
		}
	}

	/** Gives a derived game type a chance to override which team actually won */
	function int HandleWinningTeamOverride( int WinningTeamIdx )
	{
		return WinningTeamIdx;
	}

	function bool HandleRoundTimerExpired()
	{
		local int TeamIdx, Idx, WinningTeamIdxOverride, NumTeamsAlive;
		local Controller Player;
		local GearPawn GP;

		if ( !AutoReviveOnBleedOut( None ) )
		{
			// don't end round yet if everyone is DBNO on one team
			for (TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++)
			{
				if( Teams[TeamIdx] == None || Teams[TeamIdx].TeamMembers.length <= 0 )
				{
					continue;
				}
				// check for alive players
				for (Idx = 0; Idx < Teams[TeamIdx].TeamMembers.Length; Idx++)
				{
					Player = Teams[TeamIdx].TeamMembers[Idx];
					GP = GearPawn(Player.Pawn);
					if( PlayerIsAlive(Player, None) && (GP == None || (!GP.IsDBNO() && !GP.IsDoingSpecialMove(SM_ChainsawVictim))) )
					{
						NumTeamsAlive++;
						break;
					}
				}
			}

			if ( NumTeamsAlive <= 1 )
			{
				// Round is about to end because of bleedouts so don't allow the timer to run out
				return FALSE;
			}
		}

		// See if the game mode would like to override a stalemate for match over
		WinningTeamIdxOverride = HandleWinningTeamOverride( -1 );

		if ( WinningTeamIdxOverride != -1 )
		{
			GameReplicationInfo.Winner = Teams[WinningTeamIdxOverride];
			GearGRI(GameReplicationInfo).FinalKiller = None;
			GotoState('MatchOver');
		}
		else
		{
			GameReplicationInfo.Winner = None;
			GearGRI(GameReplicationInfo).FinalKiller = None;
			GetGRI().RoundCount = (GetGRI().RoundCount + 1) % 100;
			GotoState('PreRound');
		}

		return TRUE;
	}

	/** Don't allow any spawns until a new round starts. */
	function RestartPlayer(Controller NewPlayer)
	{
	}

	/** Overridden to check end of match conditions. */
	function ScoreKill(Controller Killer, Controller Other)
	{
		if ( bLastKillerSet )
		{
			return;
		}

		LastKiller = Killer;
		LastVictim = Other;
		//`log(((Killer != None) ? Killer.PlayerReplicationInfo.PlayerName : "World") @ "killed" @ Other.PlayerReplicationInfo.PlayerName);

		// Check for last men standing
		CheckForLastMenStanding();

		// If last player on team killed, force an endmatch check and set a flag
		if ( MatchEndCondition(Other) )
		{
			bLastKillerSet = true;
			// add a delay before ending the match so players can see the final kill
			SetTimer( 0.55, FALSE, nameof(CheckEndMatch) );
		}
	}

	/**
	* Sends a message to all players when someone has picked up a weapon from a weapon spawner so that we can
	* communicate the event to everyone with things like a message at the bottom of the screen etc.
	*/
	function GearBroadcastWeaponTakenMessage( Controller WeaponTaker, class<DamageType> damageType )
	{
		local GearPC PC;

		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			PC.AddWeaponTakenMessage( WeaponTaker.PlayerReplicationInfo, damageType );
		}
	}

Begin:
	if (!bTourist)
	{
		// Just set a timer that will check for the end of the match every second.
		SetTimer( 1.f, TRUE, nameof(CheckEndMatch) );
	}

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
				`log( " CE: " $ CauseEventCommand );
				ScriptTrace();
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

function TeamHasWon(int TeamIdx);

/************************************************************************
 *				        MatchInProgress State End
 ************************************************************************/

/************************************************************************
 *						MatchOver State Begin
 *	When the match is over, the player lingers here for a while.
 ************************************************************************/
state MatchOver
{


	function BeginState(Name PreviousStateName)
	{
		local int i;

		`RecordStat(STATS_LEVEL1,'MatchEnded',None,GameReplicationInfo.ElapsedTime);
		ReportFinalScoreStat();

		// Write out final Road Run Times
		for (i=0;i<GameReplicationInfo.PRIArray.Length;i++)
		{
			GearPRI(GameReplicationInfo.PRIArray[i]).RecordEndGameStats();
		}


		// Write out our stats to file, Vince, etc.
		UploadGameplayStats();

		`log("MATCH OVER");
		bFirstRound = TRUE;
		GearGRI.SetGameStatus(GS_EndMatch);
		SendAllPlayersToSpectating();

		// Removed (Move back if this causes problems)
		//KillRemainingPlayers();

		EndMatch();
		EndGame( (LastKiller != None) ? LastKiller.PlayerReplicationInfo : None, "LastMan");

	}

	// Don't allow anymore spawns.
	function RestartPlayer(Controller NewPlayer)
	{
	}

	/** Return to the training screen */
	function ReturnToTrainingGroundsScreen()
	{
		local GearPC PC;
		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			PC.ClientCompletedTraining(EGearTrainingType(TrainingGroundsID), GearGRI.Winner != None && TeamInfo(GearGRI.Winner).TeamIndex == PC.PlayerReplicationInfo.GetTeamNum());
			break;
		}
	}

	// Tell all players to save their profiles
	function SaveAllProfiles()
	{
		local GearPC PC;
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			PC.ClientSaveProfile();
		}
	}

Begin:
	Sleep( EndOfGameDelay );
	// Tell all players to save their profiles
	SaveAllProfiles();

	if ( TrainingGroundsID > -1 )
	{
		ReturnToTrainingGroundsScreen();
	}
	else
	{
		if (GameInterface.GetGameSettings('Party') == None)
		{
			WorldInfo.ServerTravel("gearstart?game=GearGameContent.GearPartyGame");
		}
		else
		{
			TellClientsToReturnToPartyHost();
		}
	}
}

/************************************************************************
 *				        MatchOver State End
 ************************************************************************/


/**
 * Adds a stat event for the final score.  Broken out so that we can override it per gametype
 */
function ReportFinalScoreStat()
{
	`RecordStat(STATS_LEVEL1,'FinalScore',none, (GameReplicationInfo.Teams.Length>1 ? (int(GameReplicationInfo.Teams[0].Score) $","$ int(GameReplicationInfo.Teams[1].Score)) : ""));
}


/**
 * Have players register with the object that tracks stats for the post game results, so we can track drops
 */
function InitLastMatchResults()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientInitLastMatchResults(OnlineStatsWriteClass,bUsingArbitration);
	}
}


defaultproperties
{
	MapPrefixes[0]="MP"

	bWaitingToStartMatch=TRUE
	bFirstRound=TRUE
	bTeamGame=TRUE
	bUseSeamlessTravel=false

	GearPlayerStartClass=class'GearTeamPlayerStart'

	TrainingGroundsID=-1

	LeaderboardId=0xFFFE0002
	ArbitratedLeaderboardId=0xFFFF0002

	// Needed because GearPRI is base for all PRI classes and specialization needs to happen here
	PlayerReplicationInfoClass=class'GearMPPRI'

	MaxSquadSize=3

	bEliminatePlantedGrenades=TRUE
	bUsingWeaponCycle=true
	bCanSwapBotsAndPlayers=true
}
