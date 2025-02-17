/**
 * Specialized game replication info class for the pre-game lobby.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearPreGameGRI extends GearGRI;

`include(GearOnlineConstants.uci)

/************************************************************************/
/* Constants, structures, enums, etc.									*/
/************************************************************************/

/** Enum of the different states in the pre-game lobby */
enum EGearPGLState
{
	eGPGLSTATE_None,
	eGPGLSTATE_Initialize,
	eGPGLSTATE_GameVote,
	eGPGLSTATE_PreMapSelection,
	eGPGLSTATE_MapSelection,
	eGPGLSTATE_PreCharacterSelect,
	eGPGLSTATE_CharacterSelect,
	eGPGLSTATE_PostCharacterSelect,
	eGPGLSTATE_StartingMatch,
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** ----- GAMETYPE SECTION VARIABLES ----- */

/** the number of seconds remaining for clients to vote for a game */
var		databinding byte RemainingGameVoteSeconds;

/** indicates whether game voting is enabled */
var bool bGameVoteEnabled;

/** the indexes for the games the server chose randomly for game voting */
var	databinding	byte FirstGameIndex, SecondGameIndex, RandomGameIndex;

/**
 * if this is a gamemode vote this index is a provider index into the "PlaylistGameTypes" provider list
 * else this is a provider index into the "GameTypes" provider list
 */
var 	databinding byte WinningGameIndex;
/** Whether the WinningGameIndex was due to a vote that didn't happen since there was only one choice */
var bool bOneChoiceGame;

/** the number of votes for each game */
var		databinding byte FirstGameVoteCount, SecondGameVoteCount;

/** the voters for each game */
var	array<PlayerReplicationInfo> FirstGameVoters, SecondGameVoters;


/** ----- MAP SECTION VARIABLES ----- */

/** the number of seconds remaining for clients to vote for a map */
var	databinding byte RemainingMapVoteSeconds;

/** indicates whether map voting is enabled */
var bool bMapVoteEnabled;

/** the mapnames for the maps the server chose randomly for map voting */
var	string FirstMapName, SecondMapName, RandomMapName;
var databinding string WinningMapName;

/** Mapname of the currently selected map in a custom match */
var repnotify string SelectedMapName;

/** the number of votes for each map */
var	databinding byte FirstMapVoteCount, SecondMapVoteCount;
var byte NumPotentialVotes;

/** the number of votes for each map */
var	array<PlayerReplicationInfo> FirstMapVoters, SecondMapVoters;

/** The localized map name (for localized map ordering) */
var transient array<string> LocMapNames;

/** The matching map file name */
var transient array<string> MapFileNames;


/** ----- CHARACTER/WEAPON SECTION VARIABLES ----- */

/** the number of seconds remaining for clients to choose their character and weapon */
var	databinding byte RemainingSecondsForLoadoutSelection;

/** List of PRIs of characters that are ready to play. */
var	array<PlayerReplicationInfo> ReadyList;

/** Indicates whether the host has pressed start to begin the game brief game countdown */
var repnotify transient bool bHostTriggeredGameStart;

/** ----- OTHER VARIABLES ----- */

/** State of the Pre-game lobby scene */
var	repnotify EGearPGLState PreGameLobbyState;

/** Countdown value */
var int CountdownTime;
/** whether to suspend the countdown during map selection due to a player not having a map during custom game map selection */
var bool bSuspendMapSelection;

/**
 * Whether it is safe to start displaying player information in the playerlist slots
 * NOTE: until this is set to true the UI will display "LOADING" in the player slots
 */
var bool bAllPlayerArePresentForDisplay;

/** Possible COG classes for wingman */
var array<int> WingmanCOGTypes;
/** Possible Locust classes for wingman */
var array<int> WingmanLocustTypes;

/** Time the GRI initialization was finished initing */
var float TimeOfGRIInitializationForSelections;

replication
{
	if ( Role == ROLE_Authority && bNetDirty )
		RemainingGameVoteSeconds, FirstGameVoteCount, SecondGameVoteCount,
		RemainingMapVoteSeconds, FirstMapVoteCount, SecondMapVoteCount,
		RemainingSecondsForLoadoutSelection, PreGameLobbyState,
		FirstGameIndex, SecondGameIndex, WinningGameIndex, bGameVoteEnabled,
		FirstMapName, SecondMapName, WinningMapName, bMapVoteEnabled, NumPotentialVotes,
		bAllPlayerArePresentForDisplay, SelectedMapName, bOneChoiceGame, bSuspendMapSelection, bHostTriggeredGameStart;
}

/** Called when any change in the vote count for a game is changed */
delegate OnGameVoteSubmitted();

/** Called when an updated value is received for SelectedMapName */
delegate OnHostSelectedMapChanged();

/** Called when any change in the vote count for a map is changed */
delegate OnMapVoteSubmitted();

/** Called when the pre-game lobby state changes */
delegate OnLobbyStateChanged( EGearPGLState NewLobbyState );

/** Called when any of the 3 selection countdowns are changed */
delegate OnSelectionCountdownTick();

/** Called when a game-type has been determined */
delegate OnGameTypeWinnerDetermined();

/** Called when a map has been determined */
delegate OnMapWinnerDetermined();

/** Called when a host presses start to begin the pregame countdown in an unofficial game */
delegate OnHostStartedPregameCountdown();

/** Called when the GRI wants the lobby to update it's map selection UI */
delegate OnMapSelectionUpdate(bool bMapSelectSuspended);

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( Role == ROLE_Authority )
	{
		PreGameLobbyState = eGPGLSTATE_Initialize;
		SetTimer( 2.0f, false, nameof(PlayersConnectingCheck) );
		ResetCountdown();
	}
}

/**
 * Checks to see if the lobby is done initializing the players or not
 */
function PlayersConnectingCheck()
{
	local GearPreGameLobbyGame_Base PreGame;
	if ( Role == ROLE_Authority )
	{
		PreGame = GearPreGameLobbyGame_Base(WorldInfo.Game);
		if ( PreGame.ArePlayersReady() && !PreGame.ArePlayersNeeded() )
		{
			bAllPlayerArePresentForDisplay = true;
		}
		else
		{
			bAllPlayerArePresentForDisplay = false;
			SetTimer( 2.0f, false, nameof(PlayersConnectingCheck) );
		}
	}
}

/** Whether there is a splitscreen player present */
simulated function bool HasSplitscreenPlayer()
{
	return class'UIInteraction'.static.GetPlayerCount() > 1;
}

/** Initializes the GRI using data from the PC's profile */
function InitializeGRIUsingPC( GearMenuPC MenuPC )
{
	if ( Role == ROLE_Authority )
	{
		InitializeMapSelection( MenuPC );
		InitializeWingmanCharacterIndexes();
	}
}

/** Sets up the WingmanClassIndexes array of indexes for which wingman characters to use */
function InitializeWingmanCharacterIndexes()
{
	local int NumWingmanTeams, TeamIdx, CurrID, SearchIdx;
	local array<int> UsedCOGClassIndexes;
	local array<int> UsedLocustClassIndexes;

	NumWingmanTeams = 5;
	for ( TeamIdx = 0; TeamIdx < NumWingmanTeams; TeamIdx++ )
	{
		// Find a COG class that hasn't been used yet
		if ( TeamIdx % 2 == 0 )
		{
			// Pick random character
			CurrID = rand( WingmanCOGTypes.length );
			// See if it was already used
			SearchIdx = UsedCOGClassIndexes.Find( WingmanCOGTypes[CurrID] );
			// Increment the index until we find one that wasn't used
			while ( SearchIdx != INDEX_NONE )
			{
				CurrID = (CurrID + 1) % WingmanCOGTypes.length;
				SearchIdx = UsedCOGClassIndexes.Find( WingmanCOGTypes[CurrID] );
			}
			// Found one so add it to both the used array and the replicated wingman list
			UsedCOGClassIndexes.AddItem( WingmanCOGTypes[CurrID] );
			WingmanClassIndexes[TeamIdx] = WingmanCOGTypes[CurrID];
		}
		// Find a Locust class that hasn't been used yet
		else
		{
			// Pick random character
			CurrID = rand( WingmanLocustTypes.length );
			// See if it was already used
			SearchIdx = UsedLocustClassIndexes.Find( WingmanLocustTypes[CurrID] );
			// Increment the index until we find one that wasn't used
			while ( SearchIdx != INDEX_NONE )
			{
				CurrID = (CurrID + 1) % WingmanLocustTypes.length;
				SearchIdx = UsedLocustClassIndexes.Find( WingmanLocustTypes[CurrID] );
			}
			// Found one so add it to both the used array and the replicated wingman list
			UsedLocustClassIndexes.AddItem( WingmanLocustTypes[CurrID] );
			WingmanClassIndexes[TeamIdx] = WingmanLocustTypes[CurrID];
		}
	}
}

/**
 * Figures out the map(s) to initially select for the match
 */
function InitializeMapSelection( GearMenuPC HostPC )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local int MatchModeIdx, MapSelectModeIdx, CurrPlaylistId;
	local OnlineGameSettings GameSettings;
	local OnlineSubsystem OnlineSub;
	local int FirstMapIndex, SecondMapIndex, RandomMapIndex;
	local int NumTotalMaps, NumSupportedMaps;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		GameSettings = GearPreGameLobbyGame_Base(WorldInfo.Game).GetPreGameGameSettings();
	}

	if ( Role == ROLE_Authority &&
		 HostPC.ProfileSettings != None &&
		 GameSettings != None )
	{
		// See if we are voting on a map or allowing the host to select it manually
		if ( GameSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchModeIdx) &&
			 MatchModeIdx != eGVMT_Official &&
			 HostPC.ProfileSettings.GetProfileSettingValueId(class'GearProfileSettings'.const.MAP_SELECTION_MODE, MapSelectModeIdx) &&
			 MapSelectModeIdx == eGEARMAPSELECT_HOSTSELECT )
		{
			bMapVoteEnabled = false;
		}
		else
		{
			bMapVoteEnabled = true;
		}

		GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
		if (GameResourceDS != None)
		{
			RefreshAlphabeticalMapNames();

			// Map vote is enabled so we must find the map indexes to vote on,
			// including the map to select if the vote is tied
			if (bMapVoteEnabled)
			{
				// Get the CurrPlaylistId for the game, -1 will mean that we are not using a playlist
				CurrPlaylistId = (MatchModeIdx != eGVMT_Official) ? -1 : PlaylistId;
				// Get the number of supported maps for this playlist
				NumSupportedMaps = GetNumberOfSupportedMapsForPlaylist(CurrPlaylistId);
				// Get the total number of maps the host has
				NumTotalMaps = GameResourceDS.GetProviderCount('Maps');

				// Set first map to the first random number that is valid
				do 
				{
					FirstMapIndex = CheckMapAgainstPlaylist(Rand(NumTotalMaps), CurrPlaylistId);
				}
				until(FirstMapIndex != INDEX_NONE ||
					  NumSupportedMaps <= 0);
				// If we were unsuccessful, just grab the first one
				if (FirstMapIndex == INDEX_NONE)
				{
					FirstMapIndex = GetFirstSupportedMapsForPlaylist(CurrPlaylistId);
				}

				// Set the second to the next valid random number
				do 
				{
					SecondMapIndex = CheckMapAgainstPlaylist(Rand(NumTotalMaps), CurrPlaylistId);
				}
				until((SecondMapIndex != INDEX_NONE && SecondMapIndex != FirstMapIndex) ||
					  NumSupportedMaps <= 1);
				// If we were unsuccessful, just grab the first one
				if (SecondMapIndex == INDEX_NONE)
				{
					SecondMapIndex = GetFirstSupportedMapsForPlaylist(CurrPlaylistId);
				}

				// Now lets set the random map choice
				do 
				{
					RandomMapIndex = CheckMapAgainstPlaylist(Rand(NumTotalMaps), CurrPlaylistId);
				}
				until((RandomMapIndex != INDEX_NONE && RandomMapIndex != FirstMapIndex && RandomMapIndex != SecondMapIndex) ||
					  NumSupportedMaps <= 2);
				// If we were unsuccessful, just grab the first one
				if (RandomMapIndex == INDEX_NONE)
				{
					RandomMapIndex = GetFirstSupportedMapsForPlaylist(CurrPlaylistId);
				}

				// Grab the map names
				FirstMapName = GameResourceDS.GetMapNameUsingProviderIndex( FirstMapIndex );
				SecondMapName = GameResourceDS.GetMapNameUsingProviderIndex( SecondMapIndex );
				RandomMapName = GameResourceDS.GetMapNameUsingProviderIndex( RandomMapIndex );
			}
			// No map vote so just set the currently selected map
			else
			{
				SelectedMapName = MapNameFromAlphabeticalIndex(0);
			}
		}
	}
	bForceNetUpdate = TRUE;
}

/** Loops through all maps returns the first valid and supported map */
function int GetFirstSupportedMapsForPlaylist(int CurrPlaylistId)
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local int MapIdx;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if (GameResourceDS != None)
	{
		if (GameResourceDS.GetResourceProviders('Maps', MapProviders))
		{
			// Loop through all maps
			for (MapIdx = 0; MapIdx < MapProviders.length; MapIdx++)
			{
				// See if the playlist supports this map and if the players have the content
				if (CheckMapAgainstPlaylist(MapIdx, CurrPlaylistId) != INDEX_NONE)
				{
					return MapIdx;
				}
			}
		}
	}
	return INDEX_NONE;
}

/** Loops through all maps and counts how many maps are supported by this playlist */
function int GetNumberOfSupportedMapsForPlaylist(int CurrPlaylistId)
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local int MapIdx;
	local int SupportedCount;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if (GameResourceDS != None)
	{
		if (GameResourceDS.GetResourceProviders('Maps', MapProviders))
		{
			// Loop through all maps
			for (MapIdx = 0; MapIdx < MapProviders.length; MapIdx++)
			{
				// See if the playlist supports this map and if the players have the content
				if (CheckMapAgainstPlaylist(MapIdx, CurrPlaylistId) != INDEX_NONE)
				{
					SupportedCount++;
				}
			}
		}
	}
	return SupportedCount;
}

/** Checks to see if the playlist supports the map at the index and returns the next map in the list that is supported */
function int CheckMapAgainstPlaylist(int MapProviderIndex, int PlaylistIdToCheck)
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local GearGameMapSummary MapData;
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;
	local int ReturnProviderIndex;
	local array<int> ContentIds;

	ReturnProviderIndex = INDEX_NONE;
	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();

	if (GameResourceDS != None)
	{
		// Grab the map providers
		if (GameResourceDS.GetResourceProviders('Maps', MapProviders))
		{
			// Get the map provider for this provider index
			MapData = GearGameMapSummary(MapProviders[MapProviderIndex]);
			if (MapData != none)
			{
				// Check for a private game which will not have a valid playlist id
				if (PlaylistIdToCheck == -1)
				{
					// If players have the map return the index
					if (PlayersHaveRequiredContentForMap(MapData, true))
					{
						ReturnProviderIndex = MapProviderIndex;
					}
				}
				// Public match
				else
				{
					OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
					if (OnlineSub != None)
					{
						PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
						if (PlaylistMan != None)
						{
							// Get the ContentId array that tells us what DLC this playlist requires
							PlaylistMan.GetContentIdsFromPlaylist(PlaylistIdToCheck, ContentIds);
							// Loop until we find a valid map for this playlist
							if (PlaylistSupportsMap(MapData, ContentIds))
							{
								ReturnProviderIndex = MapProviderIndex;
							}
						}
					}
				}
			}
		}
	}

	return ReturnProviderIndex;
}

/** Whether the playlist supports the map */
function bool PlaylistSupportsMap(GearGameMapSummary MapData, out array<int> ContentIds)
{
	local int ContentIdx;
	local bool bResult;

	// If there are content ids, this playlist is filtering out content and we don't need to
	// check for player content because the party lobby already took care of that
	if (ContentIds.length > 0)
	{
		// There is a content list so let's see if this playlist supports the map
		for (ContentIdx = 0; ContentIdx < ContentIds.length; ContentIdx++)
		{
			if (ContentIds[ContentIdx] == MapData.DLCId)
			{
				bResult = true;
				break;
			}
		}
	}
	// There are no content ids, so we just need to check player content
	else
	{
		bResult = PlayersHaveRequiredContentForMap(MapData, true);
	}

	return bResult;
}

/**
 * Whether the players in the lobby have the required content to play the map
 *
 * @param MapData the map provider we are testing against
 * @param bUseStrictCheck used when proper replication has not taken place yet
 *		(this is useful for when you MUST know if the player has DLC, for normal user feedback we won't care about
 *		 this but for a button click that will require a proper check, we must use it)
 *
 * @return whether the player has the map or not
 */
function bool PlayersHaveRequiredContentForMap(GearGameMapSummary MapData, optional bool bUseStrictCheck)
{
	local int PRIIdx;
	local bool bResult;

	bResult = true;

	// Loop through all the PRIs
	for (PRIIdx = 0; PRIIdx < PRIArray.length; PRIIdx++)
	{
		// If someone doesn't have the content break out and return false
		if (!PlayerHasRequiredContentForMap(MapData, GearPRI(PRIArray[PRIIdx]), bUseStrictCheck))
		{
			bResult = false;
			break;
		}
	}
	return bResult;
}

/**
 * Whether the player has the required content to play the map
 *
 * @param MapData the map provider we are testing against
 * @param bUseStrictCheck used when proper replication has not taken place yet
 *		(this is useful for when you MUST know if the player has DLC, for normal user feedback we won't care about
 *		 this but for a button click that will require a proper check, we must use it)
 *
 * @return whether the player has the map or not
 */
simulated function bool PlayerHasRequiredContentForMap(GearGameMapSummary MapData, GearPRI PlayerPRI, optional bool bUseStrictCheck)
{
	local bool bResult;

	bResult = true;
	// Make sure valid data was passed in
	if (MapData != None && PlayerPRI != none && !PlayerPRI.bIsInactive)
	{
		// If the map in question was shipped with the game (DLCId of 0) we don't need to do anything,
		// but if it's greater than 0 we do
		if (MapData.DLCId > 0)
		{
			// If the DLCFlag hasn't replicated yet
			if (PlayerPRI.DLCFlag == -1)
			{
				bResult = !bUseStrictCheck;
			}
			// DLCFlag has replicated so we need to see if this map is supported by the player
			else if ((PlayerPRI.DLCFlag & (1<<MapData.DLCId)) == 0)
			{
				bResult = false;
			}
		}
	}
	return bResult;
}

/**
 * Figures out the game(s) to initially select for the match
 */
function InitializeGameSelection( GearMenuPC HostPC )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local int GameCount, MatchModeIdx, GameTypeIdx, CurrPlaylistId, PlaylistProviderIndex;
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings GameSettings;
	local OnlinePlaylistProvider PlaylistProvider;
	local array<UIResourceDataProvider> PlaylistProviders;
	local UIProviderScriptFieldValue Value;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		GameSettings = GearPreGameLobbyGame_Base(WorldInfo.Game).GetPreGameGameSettings();
	}

	if ( Role == ROLE_Authority &&
		 HostPC.ProfileSettings != None &&
		 GameSettings != None )
	{
		if ( HostPC.ProfileSettings.GetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchModeIdx) &&
			 MatchModeIdx == eGVMT_Official )
		{
			bGameVoteEnabled = true;
		}
		else
		{
			bGameVoteEnabled = false;
		}

		// Game vote is enabled so we must find the game indexes to vote on,
		// including the game to select if the vote is tied
		if ( bGameVoteEnabled )
		{
			GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
			if ( GameResourceDS != None )
			{
				// Get the playlist id from the host's profile
				CurrPlaylistId = HostPC.ProfileSettings.GetPlaylistID();

				// Set the PlaylistId on the GRI
				PlaylistId = CurrPlaylistId;

				// Grab the playlist providers
				if ( GameResourceDS.GetResourceProviders('Playlists', PlaylistProviders) )
				{
					Value.PropertyTag = 'PlaylistId';
					Value.PropertyType = DATATYPE_Property;
					Value.StringValue = String(CurrPlaylistId);

					// Find the index to the provider we're looking for
					PlaylistProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Playlists', 'PlaylistId', Value);

					// If this playlist wasn't found grab the first one available
					if ( PlaylistProviderIndex == INDEX_NONE )
					{
						`log(`location@"Playlist could not be found!"@`showvar(PlaylistProviderIndex));
						PlaylistProviderIndex = 0;
					}

					// Get the playlist provider
					PlaylistProvider = OnlinePlaylistProvider(PlaylistProviders[PlaylistProviderIndex]);
					if ( PlaylistProvider != None )
					{
						// Get the game count
						GameCount = PlaylistProvider.PlaylistGameTypeNames.length;

						// Set first game to the first random number
						FirstGameIndex = Rand(GameCount);

						// Set the second to the next random number
						do 
						{
							SecondGameIndex = Rand(GameCount);
						}
						until(SecondGameIndex != FirstGameIndex ||
							  GameCount == 1);

						// Now lets set the random map choice
						do 
						{
							RandomGameIndex = Rand(GameCount);
						}
						until((RandomGameIndex != FirstGameIndex &&
							  RandomGameIndex != SecondGameIndex) ||
							  GameCount <= 2);

						// Convert all the name indexes into provider indexes in the "PlaylistGameTypes" provider list
						FirstGameIndex = class'GearUIDataStore_GameResource'.static.ConvertPlaylistGameNameIndexToPlaylistGameTypeProviderIndex(PlaylistProvider, FirstGameIndex);
						SecondGameIndex = class'GearUIDataStore_GameResource'.static.ConvertPlaylistGameNameIndexToPlaylistGameTypeProviderIndex(PlaylistProvider, SecondGameIndex);
						RandomGameIndex = class'GearUIDataStore_GameResource'.static.ConvertPlaylistGameNameIndexToPlaylistGameTypeProviderIndex(PlaylistProvider, RandomGameIndex);

						// Same choice so not voting!
						if ( FirstGameIndex == SecondGameIndex )
						{
							WinningGameIndex = FirstGameIndex;
							bOneChoiceGame = true;
							OnGameTypeWinnerDetermined();
						}

						`log(`location@"Game being voted on: First:"$FirstGameIndex@"Second:"$SecondGameIndex@"Random:"$RandomGameIndex);
					}
				}
			}
		}
		// No map vote so just set the currently selected map
		else if ( GameSettings.GetStringSettingValue(CONTEXT_VERSUSMODES, GameTypeIdx) )
		{
			// Convert the Game enum into a provider index
			GameTypeIdx = class'GearUIDataStore_GameResource'.static.GetGameTypeProviderIndex( EGearMPTypes(GameTypeIdx) );
			if ( GameTypeIdx != INDEX_NONE )
			{
				WinningGameIndex = GameTypeIdx;
				OnGameTypeWinnerDetermined();
			}
		}
	}
}

/**
 * Submits the player's vote for a map.
 *
 * @param	PRI		the PRI for the player submitting the vote
 * @param	MapIndex	0 to vote for the first map, 1 to vote for the second map.
 *
 * @return	TRUE if the vote was successfully submitted.
 */
function bool SubmitMapVote( PlayerReplicationInfo PRI, int MapIndex )
{
	local GearPreGameLobbyPRI PGPRI;
	local bool bSuccess;

	PGPRI = GearPreGameLobbyPRI(PRI);
	if ( PGPRI != None && PreGameLobbyState == eGPGLSTATE_MapSelection )
	{
		if ( FirstMapVoters.Find(PGPRI) == INDEX_NONE && SecondMapVoters.Find(PGPRI) == INDEX_NONE )
		{
			if ( MapIndex == 0 )
			{
				FirstMapVoters.AddItem(PGPRI);
				FirstMapVoteCount = FirstMapVoters.Length;
				OnMapVoteSubmitted();
				bSuccess = true;
			}
			else if ( MapIndex == 1 )
			{
				SecondMapVoters.AddItem(PGPRI);
				SecondMapVoteCount = SecondMapVoters.Length;
				OnMapVoteSubmitted();
				bSuccess = true;
			}
		}

		if ( Role == ROLE_Authority && bMapVoteEnabled )
		{
			// See if the submitted map vote should end the voting
			CheckForMapVoteStop();
		}
	}

	return bSuccess;
}

/**
 * In host select mode the host has chosen a map.
 */
function SubmitHostFinishedMapSelection()
{
	if ( PreGameLobbyState == eGPGLSTATE_MapSelection )
	{
		if ( Role == ROLE_Authority && !bMapVoteEnabled )
		{
			CalculateMapWinner();
			SetLobbyState( EGearPGLState(PreGameLobbyState + 1) );
		}
	}

}

/**
 * Submits that the player is ready to play (done with character selection)
 *
 * @param	PRI		the PRI for the player submitting the vote
 *
 * @return	TRUE if the player was successfully set to ready.
 */
function bool SubmitReady( PlayerReplicationInfo PRI )
{
	local GearPreGameLobbyPRI PGPRI;
	local bool bSuccess;

	//@TODO: It would be nice to check that we're an official match here!
	PGPRI = GearPreGameLobbyPRI(PRI);
	if ( PGPRI != None && PreGameLobbyState == eGPGLSTATE_CharacterSelect )
	{
		if ( ReadyList.Find(PGPRI) == INDEX_NONE )
		{
			ReadyList.AddItem(PGPRI);
			bSuccess = true;
		}

		if ( Role == ROLE_Authority )
		{
			// See if the submitted map vote should end the voting
			CheckForCharacterSelectionStop();
		}
	}

	return bSuccess;
}


/**
 * Selects a new map forward or backward and calls the update functions
 *
 * @param DesiredMapDirection - Whether to move forward or backward in the map list (can move multiple at a time if needed)
 */
function bool SubmitMapSelection( int DesiredMapDirection )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local int MapCount, MapIndex;
	local string NewMapName;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();

	if ( GameResourceDS != None )
	{
		RefreshAlphabeticalMapNames();
		MapIndex = AlphabeticalIndexFromMapName( SelectedMapName );
		MapCount = GameResourceDS.GetProviderCount('Maps');
		if ( MapCount > 0 )
		{
			MapIndex += DesiredMapDirection;
			if ( MapIndex < 0 )
			{
				MapIndex = MapCount - 1 ;
			}
			else
			{
				MapIndex = MapIndex % MapCount;
			}
		}

		NewMapName = MapNameFromAlphabeticalIndex( MapIndex );

		if ( NewMapName != SelectedMapName )
		{
			SelectedMapName = NewMapName;
			bSuspendMapSelection = PlayersLackRequiredDLC( SelectedMapName, true );
			OnHostSelectedMapChanged();
			ResetCountdown();
			return true;
		}
	}

	return false;
}

/** Will attempt to change the player's team, and will mark him as wanting to change teams if not possible */
function RequestChangeTeam( GearPreGameLobbyPRI PlayerPRI, int NumPlayersPerTeam )
{
	local int TeamIdx, TeamIndexToCheck, PRIIdx, NewPlayerTeam, NewOtherTeam;
	local GearPreGameLobbyPRI CurrPRI;
	local GearPreGameLobbyGame_Base PreGame;

	if ( PlayerPRI.bRequestedTeamChange || bHostTriggeredGameStart )
	{
		PlayerPRI.bRequestedTeamChange = false;
	}
	else
	{
		PreGame = GearPreGameLobbyGame_Base(WorldInfo.Game);

		// First see if someone else is requesting a change from another team
		for ( PRIIdx = 0; PRIIdx < PRIArray.length; PRIIdx++ )
		{
			CurrPRI = GearPreGameLobbyPRI(PRIArray[PRIIdx]);
			if ( CurrPRI != None &&
				 CurrPRI != PlayerPRI &&
				 !CurrPRI.bIsInactive &&
				 CurrPRI.bRequestedTeamChange &&
				 CurrPRI.GetTeamNum() != PlayerPRI.GetTeamNum() )
			{
				NewPlayerTeam = CurrPRI.GetTeamNum();
				NewOtherTeam = PlayerPRI.GetTeamNum();
				if ( NewPlayerTeam != 255 && NewOtherTeam != 255 )
				{
					// Swap teams
					if ( PreGame.ChangeTeam( Controller(PlayerPRI.Owner), NewPlayerTeam, true ) &&
						 PreGame.ChangeTeam( Controller(CurrPRI.Owner), NewOtherTeam, true ) )
					{
						CurrPRI.bRequestedTeamChange = false;
						PlayerPRI.bRequestedTeamChange = false;
						return;
					}
				}
			}
		}

		// Next see if we can just move to a team with an empty slot
		TeamIndexToCheck = PlayerPRI.GetTeamNum();
		TeamIndexToCheck = (TeamIndexToCheck + 1) % Teams.length;
		for ( TeamIdx = 0; TeamIdx < Teams.length; TeamIdx++ )
		{
			if ( Teams[TeamIndexToCheck].Size < NumPlayersPerTeam )
			{
				PreGame.ChangeTeam( Controller(PlayerPRI.Owner), TeamIndexToCheck, true );
				return;
			}
			else
			{
				TeamIndexToCheck = (TeamIndexToCheck + 1) % Teams.length;
			}
		}

		// Didn't find a team to change to so mark him as wanting to change
		PlayerPRI.bRequestedTeamChange = true;
	}
}

// Set the number of potential votes
simulated function int GetNumPotentialVoters()
{
	local int NumVoters, Idx;

	NumVoters = 0;
	for ( Idx = 0; Idx < PRIArray.length; Idx++ )
	{
		if ( PRIArray[Idx] != None &&
			 !PRIArray[Idx].bIsInactive &&
			 !PRIArray[Idx].bBot )
		{
			NumVoters++;
		}
	}

	return NumVoters;
}

/**
 * Removes a player's vote for a map.
 *
 * @param	PRI		the PRI for the player that removed the vote.
 *
 * @return	TRUE if the vote was removed.
 */
simulated function bool RemoveMapVote( PlayerReplicationInfo PRI )
{
	FirstMapVoters.RemoveItem(PRI);
	SecondMapVoters.RemoveItem(PRI);

	FirstMapVoteCount = FirstMapVoters.Length;
	SecondMapVoteCount = SecondMapVoters.Length;
	if ( Role == ROLE_Authority && bMapVoteEnabled )
	{
		OnMapVoteSubmitted();

		// See if the removal of a map vote should end the voting
		CheckForMapVoteStop();
	}

	return true;
}

// See if the submitted map vote should end the voting
function CheckForMapVoteStop()
{
	local int PRIIdx, PRICount;
	local PlayerReplicationInfo CurrPRI;

	for ( PRIIdx = 0; PRIIdx < PRIArray.length; PRIIdx++ )
	{
		CurrPRI = PRIArray[PRIIdx];
		if ( CurrPRI != None && !CurrPRI.bIsInactive && !CurrPRI.bBot )
		{
			PRICount++;
		}
	}

	if ( (FirstMapVoteCount + SecondMapVoteCount) >= PRICount ||
	     FirstMapVoteCount > PRICount / 2 ||
		 SecondMapVoteCount > PRICount / 2 )
	{
		CalculateMapWinner();
		SetLobbyState( EGearPGLState(PreGameLobbyState+1) );
	}
}

/** Check if everyone has marked themselves as ready */
function CheckForCharacterSelectionStop()
{
	local int PRIIdx, PRICount;
	local PlayerReplicationInfo CurrPRI;

	// How many actual players do we have?
	for ( PRIIdx = 0; PRIIdx < PRIArray.length; PRIIdx++ )
	{
		CurrPRI = PRIArray[PRIIdx];
		if ( CurrPRI != None && !CurrPRI.bIsInactive && !CurrPRI.bBot )
		{
			PRICount++;
		}
	}

	// If everyone is ready then start the match
	if ( ReadyList.Length >= PRICount )
	{
		SetLobbyState( EGearPGLState(PreGameLobbyState+1) );
	}

}

/** Select map vote winner */
function CalculateMapWinner()
{
	if ( bMapVoteEnabled )
	{
		// Did first map win?
		if ( FirstMapVoteCount > SecondMapVoteCount )
		{
			WinningMapName = FirstMapName;
		}
		// Did second map win?
		else if ( SecondMapVoteCount > FirstMapVoteCount )
		{
			WinningMapName = SecondMapName;
		}
		// Choose random
		else
		{
			WinningMapName = RandomMapName;
		}
	}
	else
	{
		WinningMapName = SelectedMapName;
	}

	OnMapWinnerDetermined();
}

/**
 * Submits the player's vote for a game.
 *
 * @param	PRI		the PRI for the player submitting the vote
 * @param	GameIndex	0 to vote for the first game, 1 to vote for the second game.
 *
 * @return	TRUE if the vote was successfully submitted.
 */
function bool SubmitGameVote( PlayerReplicationInfo PRI, int GameIndex )
{
	local GearPreGameLobbyPRI PGPRI;
	local bool bSuccess;

	PGPRI = GearPreGameLobbyPRI(PRI);
	if ( PGPRI != None && PreGameLobbyState == eGPGLSTATE_GameVote )
	{
		if ( FirstGameVoters.Find(PGPRI) == INDEX_NONE && SecondGameVoters.Find(PGPRI) == INDEX_NONE )
		{
			if ( GameIndex == 0 )
			{
				FirstGameVoters.AddItem(PGPRI);
				FirstGameVoteCount = FirstGameVoters.Length;
				OnGameVoteSubmitted();
				bSuccess = true;
			}
			else if ( GameIndex == 1 )
			{
				SecondGameVoters.AddItem(PGPRI);
				SecondGameVoteCount = SecondGameVoters.Length;
				OnGameVoteSubmitted();
				bSuccess = true;
			}
		}

		if ( Role == ROLE_Authority && bGameVoteEnabled )
		{
			// See if the submitted game vote should end the voting
			CheckForGameVoteStop();
		}
	}

	return bSuccess;
}

/**
 * Removes a player's vote for a game.
 *
 * @param	PRI		the PRI for the player that removed the vote.
 *
 * @return	TRUE if the vote was removed.
 */
simulated function bool RemoveGameVote( PlayerReplicationInfo PRI )
{
	FirstGameVoters.RemoveItem(PRI);
	SecondGameVoters.RemoveItem(PRI);

	FirstGameVoteCount = FirstGameVoters.Length;
	SecondGameVoteCount = SecondGameVoters.Length;
	if ( Role == ROLE_Authority && bGameVoteEnabled )
	{
		OnGameVoteSubmitted();

		// See if the removal of a game vote should end the voting
		CheckForGameVoteStop();
	}

	return true;
}

// See if the submitted game vote should end the voting
function CheckForGameVoteStop()
{
	local int PRIIdx, PRICount;
	local PlayerReplicationInfo CurrPRI;

	for ( PRIIdx = 0; PRIIdx < PRIArray.length; PRIIdx++ )
	{
		CurrPRI = PRIArray[PRIIdx];
		if ( CurrPRI != None && !CurrPRI.bIsInactive && !CurrPRI.bBot )
		{
			PRICount++;
		}
	}

	if ( (FirstGameVoteCount + SecondGameVoteCount) >= PRICount ||
		 FirstGameVoteCount > PRICount / 2 ||
		 SecondGameVoteCount > PRICount / 2 )
	{
		CalculateGameWinner();
		SetLobbyState( EGearPGLState(PreGameLobbyState+1) );
	}
}


/** Select game vote winner */
function CalculateGameWinner()
{
	// Did first game win?
	if ( FirstGameVoteCount > SecondGameVoteCount )
	{
		WinningGameIndex = FirstGameIndex;
	}
	// Did second game win?
	else if ( SecondGameVoteCount > FirstGameVoteCount )
	{
		WinningGameIndex = SecondGameIndex;
	}
	// Choose random
	else
	{
		WinningGameIndex = RandomGameIndex;
	}

	OnGameTypeWinnerDetermined();
}

/**
 * Determines whether all players have successfully connected and opened their lobby scene.
 *
 * @return	TRUE if all players have their lobby scene open.
 */
function bool LobbyIsFinishedInitializing()
{
	local GearPreGameLobbyGame_Base LobbyGame;
	local bool bResult;
	local GearMenuPC MenuPC;

	LobbyGame = GearPreGameLobbyGame_Base(WorldInfo.Game);
	if ( LobbyGame != None )
	{
		bResult = LobbyGame.CanStartMatch();
		// Initialize the GRI data if we are ready to start
		if (bResult && TimeOfGRIInitializationForSelections < 0)
		{
			// Make sure that only the first logged in player sets the GRI data
			foreach LocalPlayerControllers(class'GearMenuPC', MenuPC)
			{
				if (MenuPC != none)
				{
					GearPreGameGRI(WorldInfo.GRI).InitializeGRIUsingPC(MenuPC);
					TimeOfGRIInitializationForSelections = WorldInfo.TimeSeconds;
				}
				break;
			}
		}
	}

	return bResult;
}

/** Returns whether this is a local player in a local match or not */
simulated function bool IsLocalPlayerInLocalMatch()
{
	local OnlineGameSettings CurrentSettings;
	local int MatchMode;

	if ( Role == ROLE_Authority )
	{
		CurrentSettings = GearGame(WorldInfo.Game).GetCurrentGameSettings(true);
		if ( CurrentSettings != None &&
			 CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchMode) &&
			 MatchMode == eGVMT_Local )
		{
			return true;
		}
	}
	return false;
}

function ResetCountdown()
{
	switch ( PreGameLobbyState )
	{
		case eGPGLSTATE_Initialize:
		case eGPGLSTATE_PreMapSelection:
		case eGPGLSTATE_PreCharacterSelect:
		case eGPGLSTATE_PostCharacterSelect:
			CountdownTime = 1;
			break;
		case eGPGLSTATE_GameVote:
			CountdownTime = default.RemainingGameVoteSeconds;
			RemainingGameVoteSeconds = default.RemainingGameVoteSeconds;
			break;
		case eGPGLSTATE_MapSelection:
			CountdownTime = default.RemainingMapVoteSeconds;
			RemainingMapVoteSeconds = default.RemainingMapVoteSeconds;
			break;
		case eGPGLSTATE_CharacterSelect:
			CountdownTime = default.RemainingSecondsForLoadoutSelection;
			RemainingSecondsForLoadoutSelection = default.RemainingSecondsForLoadoutSelection;
			break;
	}

	SetTimer( 1.0f, true, nameof(UpdateCountdowns) );
}

/**
 * Decrement the remaining seconds.
 */
function UpdateCountdowns()
{
	local EGearPGLState NextState;

	if ( Role == ROLE_Authority )
	{
		NextState = eGPGLSTATE_None;
		if ( !bSuspendMapSelection && bAllPlayerArePresentForDisplay )
		{
			CountdownTime--;
		}

		// Determine if it's time to move to the next lobby state
		switch ( PreGameLobbyState )
		{
			case eGPGLSTATE_Initialize:
				if ( CountdownTime <= 0 )
				{
					if ( LobbyIsFinishedInitializing() && (WorldInfo.TimeSeconds - TimeOfGRIInitializationForSelections) > 1.f )
					{
						if ( !bGameVoteEnabled || WinningGameIndex != 255 )
						{
							NextState = eGPGLSTATE_MapSelection;
						}
						else
						{
							NextState = eGPGLSTATE_GameVote;
						}
					}
					else
					{
						ResetCountdown();
					}
				}
				break;

			case eGPGLSTATE_GameVote:
				RemainingGameVoteSeconds = max(0, RemainingGameVoteSeconds-1);
				OnSelectionCountdownTick();
				// Give clients an extra couple seconds when voting/selecting since they'll be a second or two behind the server
				if ( CountdownTime <= -2 )
				{
					CalculateGameWinner();
					NextState = EGearPGLState(PreGameLobbyState + 1);
				}
				break;

			case eGPGLSTATE_MapSelection:
				if ( PlayersLackRequiredDLC(SelectedMapName, true) )
				{
					if (!bSuspendMapSelection)
					{
						bSuspendMapSelection = true;					
						OnMapSelectionUpdate(true);
					}
				}
				else
				{
					if (!bMapVoteEnabled)
					{
						// Host is selecting map, so there is no need for a countdown.
						ResetCountdown();
					}
					if (bSuspendMapSelection)
					{
						bSuspendMapSelection = false;
						OnMapSelectionUpdate(false);
					}
				}
				RemainingMapVoteSeconds = max(0, RemainingMapVoteSeconds-1);
				OnSelectionCountdownTick();
				// Give clients an extra couple seconds when voting/selecting since they'll be a second or two behind the server
				if ( CountdownTime <= -2 )
				{
					CalculateMapWinner();
					NextState = EGearPGLState(PreGameLobbyState + 1);
				}
				break;

			case eGPGLSTATE_CharacterSelect:
				RemainingSecondsForLoadoutSelection = max(0, RemainingSecondsForLoadoutSelection-1);
				OnSelectionCountdownTick();
				// Give clients an extra couple seconds when voting/selecting since they'll be a second or two behind the server
				if ( CountdownTime <= -2 )
				{
						NextState = EGearPGLState(PreGameLobbyState + 1);
				}
				
				// Hack test for non-public (if custom matches are ever able to vote on gametypes this check must be fixed)
				// Only advance if this is a public match, host must press START to advance non-public matches
				if ( !bGameVoteEnabled && !bHostTriggeredGameStart )
				{
					ResetCountdown();
				}

				break;

			case eGPGLSTATE_PreCharacterSelect:
				if ( CountdownTime <= 0 )
				{
					// Start arbitration handshaking on the game
					StartArbitration();
					NextState = EGearPGLState(PreGameLobbyState + 1);
				}
				break;
			case eGPGLSTATE_PreMapSelection:
			case eGPGLSTATE_PostCharacterSelect:
				if ( CountdownTime <= 0 )
				{
					NextState = EGearPGLState(PreGameLobbyState + 1);
				}
				break;
		}

		if ( NextState != eGPGLSTATE_None )
		{
			SetLobbyState( NextState );
		}
	}
}

/** Set the lobby state and reset the counter */
function SetLobbyState( EGearPGLState NewState )
{
	PreGameLobbyState = NewState;
	OnLobbyStateChanged( NewState );
	ResetCountdown();

	if ( Role == ROLE_Authority )
	{
		if ( PreGameLobbyState == eGPGLSTATE_StartingMatch )
		{
			GearPreGameLobbyGame_Base(WorldInfo.Game).BeginMatch();
		}
	}
}

/**
 * Whether any players do not have the required DLCs needed for the selected map or not
 *
 * @param MapName - the name of the map we are checking for content on
 * @param bUseStrictCheck - used when proper replication has not taken place yet
 *		(this is useful for when you MUST know if the player has DLC, for normal user feedback we won't care about
 *		 this but for a button click that will require a proper check, we must use it)
 */
function bool PlayersLackRequiredDLC( string MapName, optional bool bUseStrictCheck )
{
	local int DLCTest, PRIIdx;
	local GearGameMapSummary MapData;
	local GearPRI PlayerPRI;


	// Testing against both vote enabled flags lets us know that this is a custom match where the host selects the map
	if ( !bMapVoteEnabled && !bGameVoteEnabled )
	{
		MapData = class'GearUIDataStore_GameResource'.static.GetMapSummaryFromMapName( MapName );
		if ( MapData != None )
		{
			if ( MapData.DLCId > 0 )
			{
				for ( PRIIdx = 0; PRIIdx < PRIArray.length; PRIIdx++ )
				{
					PlayerPRI = GearPRI(PRIArray[PRIIdx]);
					if ( PlayerPRI != None && !PlayerPRI.bIsInactive )
					{
						if ( bUseStrictCheck && PlayerPRI.DLCFlag == -1 )
						{
							return true;
						}
						else
						{
							DLCTest = PlayerPRI.DLCFlag >> MapData.DLCId;
							if ( DLCTest % 2 != 1 )
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

/**
 * Called when a player joins the game.  Resets the countdown timers to give everyone a chance to vote.
 */
simulated function AddPRI(PlayerReplicationInfo PRI)
{
	Super.AddPRI(PRI);

	// Determine whether it should go in the active or inactive list
	if ( Role == ROLE_Authority && !PRI.bIsInactive )
	{
		ResetCountdown();
	}
}

/**
 * Called when a player leaves the game.  Removes that player's vote, if applicable.
 *
 * @param	PRI		the player that left.
 */
simulated function RemovePRI( PlayerReplicationInfo PRI )
{
	Super.RemovePRI(PRI);

	RemoveMapVote(PRI);
}

/**
 * Called when a variable is replicated that has the 'repnotify' keyword.
 *
 * @param	VarName		the name of the variable that was replicated.
 */
simulated event ReplicatedEvent( name VarName )
{
	if ( VarName == 'PreGameLobbyState' )
	{
		OnLobbyStateChanged( PreGameLobbyState );
	}
	else if ( VarName == 'SelectedMapName' )
	{
		OnHostSelectedMapChanged();
	}
	else if (VarName == 'bHostTriggeredGameStart')
	{
		OnHostStartedPregameCountdown();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/**
 * Called when a variable is replicated that has the 'databinding' keyword.
 *
 * @param	VarName		the name of the variable that was replicated.
 */
simulated event ReplicatedDataBinding( name VarName )
{
	switch ( VarName )
	{
		case 'FirstGameVoteCount':
		case 'SecondGameVoteCount':
			OnGameVoteSubmitted();
			break;

		case 'FirstMapVoteCount':
		case 'SecondMapVoteCount':
			OnMapVoteSubmitted();
			break;

		case 'RemainingGameVoteSeconds':
		case 'RemainingMapVoteSeconds':
		case 'RemainingSecondsForLoadoutSelection':
			OnSelectionCountdownTick();
			break;

		case 'WinningGameIndex':
			OnGameTypeWinnerDetermined();
			break;

		case 'WinningMapName':
			OnMapWinnerDetermined();
			break;

		default:
			Super.ReplicatedDataBinding(VarName);
			break;
	}
}

/**
 * Called once the game has decided that it is time to start voting (all players present)
 */
function StartArbitration()
{
	local GearPreGameLobbyGame_Base LobbyGame;

	LobbyGame = GearPreGameLobbyGame_Base(WorldInfo.Game);
	if (LobbyGame != None)
	{
		// Tells all of the players to register for arbitration
		LobbyGame.StartArbitrationRegistration();
	}
}

/**
 * Update the cached map names from the datastore.
 *
 */
function RefreshAlphabeticalMapNames()
{
	local GearUIDataStore_GameResource GameResourceDS;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		BuildSortedMapList(GameResourceDS, LocMapNames, MapFileNames);
	}
}

/**
 * Build an alphabetically sorted list of map names with an associated list of real map names (e.g. localized: BloodBank vs. MP_BloodBank)
 *
 * @param GameResourceDS	Resource DataStore which contains the map information
 * @param out_LocMapNames	Names of maps available to player in alphabetical order
 * @param out_MapFileNames	Corresponding list of map filenames
 */
private function BuildSortedMapList( GearUIDataStore_GameResource GameResourceDS, out array<string> out_LocMapNames, out array<string> out_MapFileNames )
{
	
	local UIProviderScriptFieldValue GameResourceData;
	local array<UIResourceDataProvider> MapProviders;
	local string TempMapName;
	local int MapIdx;
	local int MapIdx2;

	// Empty the output; by default we find no maps
	out_LocMapNames.Length = 0;
	out_MapFileNames.Length = 0;

	GameResourceDS.GetResourceProviders('Maps', MapProviders);
	
	if (MapProviders.Length > 0)
	{
		// Get the maps
		for ( MapIdx = 0; MapIdx < MapProviders.Length; ++MapIdx )
		{
			if ( GameResourceDS.GetProviderFieldValue('Maps', 'DisplayName', MapIdx, GameResourceData) )
			{
				out_LocMapNames.AddItem( GameResourceData.StringValue );
				out_MapFileNames.AddItem( GearGameMapSummary(MapProviders[MapIdx]).MapName );
			}
		}

		// Sort the maps... with bubblesort :(
		for ( MapIdx = 0; MapIdx < MapProviders.Length; ++MapIdx )
		{
			for ( MapIdx2 = 0; MapIdx2 < MapProviders.Length; ++MapIdx2 )
			{
				if (out_LocMapNames[MapIdx] < out_LocMapNames[MapIdx2])
				{
					TempMapName = out_LocMapNames[MapIdx2];
					out_LocMapNames[MapIdx2] = out_LocMapNames[MapIdx];
					out_LocMapNames[MapIdx] = TempMapName;

					TempMapName = out_MapFileNames[MapIdx2];
					out_MapFileNames[MapIdx2] = out_MapFileNames[MapIdx];
					out_MapFileNames[MapIdx] = TempMapName;

				}
			}
		}
	}

}

/**
 * Get map name based on map index
 *
 * @param MapName	The name of the map (real name; e.g. MP_BloodSomething)
 *
 * @return Index of map in the list
 */
function int AlphabeticalIndexFromMapName(string MapName)
{
	return MapFileNames.Find(MapName);
}

/**
 * Get map name based on map index
 *
 * @param MapIndex	The index of the map in the list
 *
 * @return Map Name
 */
function string MapNameFromAlphabeticalIndex(int MapIndex)
{
	if (MapIndex >= 0 && MapIndex < MapFileNames.Length && MapFileNames.Length > 0)
	{
		return MapFileNames[MapIndex];
	}
	else
	{
		return "";
	}
}

/** Called once arbitration completes and starts the match. Just ignores the call */
simulated function StartMatch()
{
	// Eats the call intentionally
}

DefaultProperties
{
	RemainingSecondsForLoadoutSelection=10
	RemainingMapVoteSeconds=15
	RemainingGameVoteSeconds=15
	TimeOfGRIInitializationForSelections=-1

	FirstGameIndex=255
	SecondGameIndex=255
	WinningGameIndex=255

	bMapVoteEnabled=true
	bGameVoteEnabled=true

	WingmanCOGTypes.Add(CMPC_Marcus)
	WingmanCOGTypes.Add(CMPC_BenCarmine)
	WingmanCOGTypes.Add(CMPC_Dom)
	WingmanCOGTypes.Add(CMPC_Cole)
	WingmanCOGTypes.Add(CMPC_Baird)
	WingmanCOGTypes.Add(CMPC_Tai)
	WingmanCOGTypes.Add(CMPC_Dizzy)

	WingmanLocustTypes.Add(LMPC_HunterNoArmor)
	WingmanLocustTypes.Add(LMPC_Kantus)
	WingmanLocustTypes.Add(LMPC_DroneSniper)
	WingmanLocustTypes.Add(LMPC_Theron)
	WingmanLocustTypes.Add(LMPC_BeastLord)
}

