/**
 * This class is the game info for the pre-game lobby. It treats its session as
 * if it is the game and not the party lobby.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPreGameLobbyGame_Base extends GearPartyGame_Base;

`include(Engine/Classes/UIDev.uci)

/** The recent players list used to keep track of who has been seen recently */
var OnlineRecentPlayersList RecentPlayersList;

/** Used to auto start rather than waiting for someone to never arrive */
var bool bWaitForPlayersTimedOut;

/** Game settings object use in the pregame game */
var OnlineGameSettings PreGameGameSettings;

/** Whether to ignore logouts or not */
var bool bIsClosing;

event PreBeginPlay()
{
	local OnlineGameSettings GameSettings;
	local int NumberTeams, NumPlayerPerTeam, PlaylistId;

	Super.PreBeginPlay();

	if (GameInterface != None)
	{
		GameSettings = GetPreGameGameSettings();
		RecentPlayersList = OnlineRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
	}

	MaxPlayers = GameSettings != None ? GameSettings.NumPublicConnections : 10;

	if ( GameSettings.bUsesArbitration )
	{
		if ( OnlineSub != None )
		{
			if ( PlaylistMan != None )
			{
				GameSettings.GetIntProperty(PROPERTY_PLAYLISTID, PlaylistId);
				PlaylistMan.GetTeamInfoFromPlaylist( PlaylistId, NumPlayerPerTeam, NumberTeams );
				NumTeams = NumberTeams;
			}
		}
	}
	else
	{
		if ( IsWingmanGametype() )
		{
			NumTeams = 5;
		}
	}
	// Every time a player joins restart the timer
	SetTimer(10.0,false,nameof(OnWaitForPlayerTimerExpired));
}

/**
 * Accept or reject a player on the server.  Fails login if you set the Error to a non-empty string.
 *
 * @param	Options		the URL used to join the match
 * @param	Address		the IP address of the joining player
 * @param	ErrorMessage	if login fails, a message explaining why should be assigned to this variable.
 */
event PreLogin(string Options, string Address, out string ErrorMessage)
{
	Super.PreLogin(Options, Address, ErrorMessage);

	// Every time a player joins restart the timer
	SetTimer(10.0,false,nameof(OnWaitForPlayerTimerExpired));

	if ( ErrorMessage == "" )
	{
		GearPreGameGRI(GameReplicationInfo).ResetCountdown();
	}

	// Can't be invited into the a pre-game lobby
	if (HasOption(Options,"bIsFromInvite"))
	{
		ErrorMessage = GameMessageClass.Default.ArbitrationMessage;
		return;
	}
}

/** Whether this is a local game or not */
function bool IsLocalMatch()
{
	local OnlineGameSettings CurrentSettings;
	local int MatchMode;

	// If this is a local game put the player on team 0
	CurrentSettings = GetPreGameGameSettings();
	if ( CurrentSettings != None &&
		 CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchMode) &&
		 MatchMode == eGVMT_Local )
	{
		return true;
	}
	return false;
}

/**
 * Called after a successful login. This is the first place it is safe to call replicated functions on the PlayerController.
 *
 * @param	NewPlayer	the PlayerController that just successfully connected and logged in.
 */
event PostLogin( PlayerController NewPlayer )
{
	local GearMenuPC MenuPC;
	local GearMenuPC NewMenuPC;

	Super.PostLogin(NewPlayer);

	// Grab the PC and have him update his DLC values
	NewMenuPC = GearMenuPC(NewPlayer);
	NewMenuPC.UpdateLocalPlayersDLCValues();

	// Make sure that only the first logged in player sets the GRI game data
	foreach LocalPlayerControllers(class'GearMenuPC', MenuPC)
	{
		if (MenuPC == NewMenuPC)
		{
			GearPreGameGRI(WorldInfo.GRI).InitializeGameSelection(MenuPC);
		}
		break;
	}

	// Have the new player open their lobby
	if (NewMenuPC != None && LobbySceneResource != None)
	{
		// Tell the player to open the lobby scene
		NewMenuPC.ClientOpenScene(LobbySceneResource);
	}
}

/**
 * Handles a player leaving the session. If there are only local players left,
 * then return to the party lobby
 *
 * @param Exiting the player leaving the session
 */
function Logout(Controller Exiting)
{
	local int RemotePlayerCount;
	local GearPC PC;

	PC = GearPC(Exiting);
	if (PC != None && !bIsClosing)
	{
		// If a local player leaves, we need to return to main menu
		if (!PC.IsLocalPlayerController())
		{
			// Count how many remote players there are
			foreach WorldInfo.AllControllers(class'GearPC',PC)
			{
				if (!PC.IsLocalPlayerController() && PC != Exiting)
				{
					RemotePlayerCount++;
				}
			}

			// If there are no players left, return to the party lobby
			if (RemotePlayerCount == 0)
			{
				foreach LocalPlayerControllers(class'GearPC', PC)
				{
					PC.ClientReturnToParty();
					break;
				}
			}
		}
		else
		{
			bIsClosing = true;
		}
	}
}

/**
 * Initializes the party system object for manipulation by the lobby
 */
event InitGame(string Options, out string ErrorMessage)
{
	Super.InitGame(Options, ErrorMessage);

	InitializePreGameGameSettings();
}

/**
 * Used when creating a local match
 */
function InitializePreGameGameSettings()
{
	local OnlineGameSettings CurrentSettings;
	local int MatchModeIdx;

	// If we are in a local match we have to create the game settings object from the CurrentGameSettings and return.
	CurrentSettings = GetCurrentGameSettings(true);
	if ( CurrentSettings != None )
	{
		if ( CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchModeIdx) )
		{
			if ( MatchModeIdx == eGVMT_Local || MatchModeIdx == eGVMT_SystemLink )
			{
				PreGameGameSettings = new class'GearVersusGameSettings'(CurrentSettings);
				PreGameGameSettings.bUsesArbitration = false;
				return;
			}
		}
	}

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && OnlineSub.GameInterface != None )
	{
		PreGameGameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfoClass.default.SessionName);
	}
}

/** Returns the game settings object used while in the pregame */
function OnlineGameSettings GetPreGameGameSettings()
{
	return PreGameGameSettings;
}

function UpdateGameSettings()
{
	Super(GearMenuGame).UpdateGameSettings();
}

/*========================================================================================================================================*/
/**
 * Determines whether the PRI should be considered when determining whether to start the loadout countdown.
 *
 * @return	TRUE if this PRI should be checked when waiting for everyone to load the pre-game lobby.
 */
function bool IsValidPRI( PlayerReplicationInfo PRI )
{
	return	PRI != None			&& !PRI.bIsInactive
		&&	PRI.Owner != None	&& !PRI.bOnlySpectator
		&&	!GearPC(PRI.Owner).bDedicatedServerSpectator;
}

/**
 * Determines whether the PRI has received all information required for team balancing/changing from the client.
 *
 * @return	TRUE if the PRI has opened its lobby scene (meaning that the client has received GRI, PRI, and its PC & PRI are linked), and the
 *			server has received both the client's UniqueNetId and that of the party leader.
 */
function bool IsPRIInitialized( PlayerReplicationInfo PRI )
{
	local UniqueNetId ZeroId;
	local GearPreGameLobbyPRI LobbyPRI;

	LobbyPRI = GearPreGameLobbyPRI(PRI);
	return	LobbyPRI != None
		&&	LobbyPRI.bLobbySceneOpened
		&&	!class'OnlineSubsystem'.static.AreUniqueNetIdsEqual(LobbyPRI.UniqueId,ZeroId);
}

/**
 * Wrapper for checking whether the currently selected gametype is wingman.  This version checks the OnlineGameSettings object for the
 * currently selected gametype so that the result is also valid on clients (only the match host's profile would have the current gametype set)
 */
function bool IsWingmanGametype()
{
	local int GametypeContextId;
	local OnlineGameSettings GameSettings;

	if (GameInterface != None)
	{
		GameSettings = GetPreGameGameSettings();
	}

	return GameSettings != None &&
		GameSettings.GetStringSettingValue(PartySettings.const.CONTEXT_VERSUSMODES, GametypeContextId)
		&& GametypeContextId == eGEARMP_Wingman;
}

/** @return true if the match is private, false otherwise */
function bool IsPrivateMatch()
{
	local OnlineGameSettings GameSettings;

	if (GameInterface != None)
	{
		GameSettings = GetPreGameGameSettings();
	}
	return GameSettings == None || GameSettings.bUsesArbitration == false;
}

/**
 * Wrapper for determining whether the match can be started.  This version returns true once all players have opened their pre-game
 * lobby scenes and determines when the GRI can begin the loadout selection countdown.
 *
 * @return	TRUE if all requirements for beginning the match have been met.
 */
function bool CanStartMatch()
{
	local GearPreGameGRI PGGRI;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);

	if ( PGGRI != None &&
		 PGGRI.bAllPlayerArePresentForDisplay &&
		 ArePlayersReady() &&
		 !ArePlayersNeeded() )
	{
		return true;
	}
	return false;
}

/**
 * Starts the match if all conditions for starting the match have been met.  Initiates
 * the travel for all clients.
 */
function BeginMatch()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local string LaunchURL;
	local GearPreGameGRI PGGRI;
	local GearGameInfoSummary MatchGameTypeProvider;
	local GearGamePlaylistGameTypeProvider GameTypeProvider;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);
	GameResourceDS = GetGameResourceDataStore();

	if ( GameResourceDS != None )
	{
		MatchGameTypeProvider = GetMatchGameTypeDataProvider();

		if ( MatchGameTypeProvider != None )
		{
			GameTypeProvider = GetWinningPlaylistGameTypeProvider();

			SetClientsVersusRichPresenceStrings();

			// generate the URL for the travel
			LaunchURL = PGGRI.WinningMapName $ "?Game=" $ MatchGameTypeProvider.ClassName $
				"?GameSettingsId=" $ GameTypeProvider.GameTypeId;
			WorldInfo.ServerTravel(LaunchURL);
		}
	}
}

/** Determines whether this was a custom match or a voting playlist match and returns the proper GameType provider */
function GearGameInfoSummary GetMatchGameTypeDataProvider()
{
	local GearPreGameGRI PGGRI;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);

	if ( PGGRI.bGameVoteEnabled )
	{
		return GetGameTypeProviderOfWinningPlaylistGameType();
	}
	else
	{
		return GetGameTypeProviderForCustomMatch();
	}
}

/** Returns the provider for the playlist gamemode that won the vote */
function GearGamePlaylistGameTypeProvider GetWinningPlaylistGameTypeProvider()
{
	local GearPreGameGRI PGGRI;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);

	return class'GearUIDataStore_GameResource'.static.GetPlaylistGameTypeProvider(PGGRI.WinningGameIndex);
}

/** Returns the GameType provider for the playlist gamemode that won the vote */
function GearGameInfoSummary GetGameTypeProviderOfWinningPlaylistGameType()
{
	local GearGamePlaylistGameTypeProvider WinningPlaylistGameTypeProvider;
	local GearPreGameGRI PGGRI;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);
	WinningPlaylistGameTypeProvider = class'GearUIDataStore_GameResource'.static.GetPlaylistGameTypeProvider(PGGRI.WinningGameIndex);

	return class'GearUIDataStore_GameResource'.static.GetGameTypeProvider(WinningPlaylistGameTypeProvider.MPGameMode);
}

/** Returns the GameType provider for a custom match */
function GearGameInfoSummary GetGameTypeProviderForCustomMatch()
{
	local GearPreGameGRI PGGRI;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);

	return class'GearUIDataStore_GameResource'.static.GetGameTypeProviderUsingProviderIndex(PGGRI.WinningGameIndex);
}

/**
 * Wrapper for determining whether all required players are ready to begin the countdown.
 *
 * @return	TRUE if all players are ready to begin the match.
 */
function bool ArePlayersReady()
{
	local int Idx;
	local GearPreGameLobbyPRI LobbyPRI;
	local UniqueNetId ZeroID;

	for ( Idx = 0; Idx < GameReplicationInfo.PRIArray.Length; Idx++ )
	{
		LobbyPRI = GearPreGameLobbyPRI(GameReplicationInfo.PRIArray[Idx]);
		if ( IsValidPRI(LobbyPRI) )
		{
			if (!LobbyPRI.bLobbySceneOpened || ((LobbyPRI.UniqueId == ZeroId || LobbyPRI.DLCFlag == -1) && !IsLocalMatch()))
			{
				return false;
			}
		}
	}

	return true;
}

/**
 * If this timer fires, not all players joined fast enough, so start without them
 */
function OnWaitForPlayerTimerExpired()
{
	bWaitForPlayersTimedOut = true;
}

/**
 * Compares the number of connected clients with the number of expected clients
 *
 * @return true if more players are needed to start the match, false otherwise
 */
function bool ArePlayersNeeded()
{
	if (!bWaitForPlayersTimedOut)
	{
		return GameReplicationInfo.PRIArray.Length < RecentPlayersList.GetCurrentPlayersListCount();
	}
	return false;
}

/*========================================================================================================================================*/
/**
 * Wrapper for getting the maximum team size for the currently selected gametype.
 */
function int GetMaxTeamSize()
{
//	return IsWingmanGametype() ? 2 : 5;	// retarded that we have to hard-code this....but that's what the gametypes do, so...

	// each team can only be as large as the party
	return PartySettings.NumPublicConnections;
}

/*
 * Return whether a team change is allowed.
 */
function bool ChangeTeam(Controller Other, int N, bool bNewTeam)
{
	// skip over the partygame version, as it always returns true
	return Super(GearGame).ChangeTeam(Other, N, bNewTeam);
}

function string itos( const out UniqueNetId NetId )
{
	return class'OnlineSubsystem'.static.UniqueNetIdToString(NetId);
}

/**
 * Always returns what is passed in since it assumes teams are already assigned
 *
 * @param Other the controller being assigned a team
 * @param Team the team requested
 */
function int GetForcedTeam(Controller Other, int Team)
{
	return Team;
}

/**
 * Balance the teams, attempting to keep players on the same team as their party leader
 */
function AutoSeedTeams()
{
	//@todo robm/joeg -- do we need this for private matches?
}

/**
 * Uses the recent players list data to update this player's skill and team
 *
 * @param PC the player needing to be updated
 */
function UpdateTeamAndSkillData(GearPreGamePC PC)
{
	local int TeamNum;
	local int PlayerSkill;
	local UniqueNetId ZeroId;

	if (PC.PlayerReplicationInfo.UniqueId != ZeroId)
	{
		// Get the team the beacon assigned and use that
		TeamNum = RecentPlayersList.GetTeamForCurrentPlayer(PC.PlayerReplicationInfo.UniqueId);
		ChangeTeam(PC,TeamNum,true);

		// Set the cached skill information so it flows to all clients
		PlayerSkill = RecentPlayersList.GetSkillForCurrentPlayer(PC.PlayerReplicationInfo.UniqueId);
		PC.PlayerReplicationInfo.PlayerSkill = PlayerSkill;
	}
	else
	{
		// No profile so dump him on team 0
		ChangeTeam(PC,0,true);
	}
}


auto state PendingMatch
{
	/**
	 * Skips arbitration registration if this match isn't arbitrated
	 */
	function StartArbitrationRegistration()
	{
		local OnlineGameSettings GameSettings;

		GameSettings = GameInterface.GetGameSettings('Game');
		if (GameSettings != None)
		{
			// Are we a public match?
			if (GameSettings.bUsesArbitration)
			{
				Super.StartArbitrationRegistration();
			}
			else
			{
				StartOnlineGame();
			}
		}
	}

	/**
	 * Called once the arbitration handshaking is complete. It starts the online
	 * session which shrinks the session to the current arbitrated registrants
	 */
	function StartArbitratedMatch()
	{
		bNeedsEndGameHandshake = true;
		// Start the online game and once started travel to preround
		Super(GameInfo).StartOnlineGame();
	}
}

/**
 * Using the voted for map and game type, updates all clients' rich presence
 */
function SetClientsVersusRichPresenceStrings()
{
	local GearPreGamePC PC;
	local GearUIDataStore_GameResource GameResourceDS;
	local GearPreGameGRI PGGRI;
	local GearGameInfoSummary MatchGameTypeProvider;
	local byte MapNameId;
	local byte MpType;
	local byte StringToUse;
	local OnlineGameSettings GameSettings;

	PGGRI = GearPreGameGRI(WorldInfo.GRI);
	MapNameId = byte(class'GearLeaderboardSettings'.static.GetMapIdFromName(name(PGGRI.WinningMapName)));

	GameResourceDS = GetGameResourceDataStore();
	if (GameResourceDS != None)
	{
		MatchGameTypeProvider = GetMatchGameTypeDataProvider();

		if (MatchGameTypeProvider != None)
		{
			MpType = MatchGameTypeProvider.MPGameMode;
		}
	}

	GameSettings = GameInterface.GetGameSettings('Game');
	if (GameSettings != None)
	{
		// Update rich presence to match the game mode
		if (GameSettings.bUsesArbitration)
		{
			StringToUse = CONTEXT_PRESENCE_VERSUSPRESENCE;
		}
		// Default to private match
		else
		{
			StringToUse = CONTEXT_PRESENCE_PRIVATEVERSUSPRESENCE;
		}
	}
	else
	{
		GameSettings = GameInterface.GetGameSettings('Party');
		// Is this a system link match?
		if (GameSettings == None || GameSettings.bIsLanMatch)
		{
			StringToUse = CONTEXT_PRESENCE_SYSTEMLINKVERSUSPRESENCE;
		}
	}

	// Tell each client the presence to use
	foreach WorldInfo.AllControllers(class'GearPreGamePC',PC)
	{
		PC.ClientSetVersusRichPresenceString(StringToUse,MpType,MapNameId);
	}
}

defaultproperties
{
	// This PRI registers as part of the game session not party one
	PlayerReplicationInfoClass=class'GearGame.GearPreGameLobbyPRI'

	// Specify the standard game PC so voice and recent player's list to work as part of the game session
	PlayerControllerClass=class'GearGame.GearPreGamePC'

	// Need to use a special GRI class to handle the mapvote and loadout counters
	GameReplicationInfoClass=class'GearGame.GearPreGameGRI'

`if(`isdefined(dev_build))
	// allow pausing during development
	bPauseable=true
`endif

	// Pre-game to game uses seamless travel
	bUseSeamlessTravel=true
}
