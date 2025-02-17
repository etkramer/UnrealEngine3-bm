/**
 * This class is the game info for the campaign lobby.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearCampaignLobbyGame_Base extends GearMenuGame;

/** The controller index that is used for searching/creating a campaign */
var int OwningControllerIndex;

/** the URL to use when starting the game */
var	transient	string			GameURL;

/** Resource reference to the lobby scene */
var	transient UIScene LobbySceneResource;

/** Resource reference to the save slot scene */
var transient UIScene SaveSlotScene;
/** Resource reference to the difficulty scene */
var transient UIScene DifficultyScene;

// === GameInfo interface ===

/**
 * Initializes the campaign object for manipulation by the lobby
 */
event InitGame(string Options, out string ErrorMessage)
{
	local string EnterLobbyString;

	Super.InitGame(Options, ErrorMessage);

	// Cache the campaign lobby mode we are in
	class'UIRoot'.static.GetDataStoreStringValue("<Registry:EnterLobby>", EnterLobbyString);
	CampaignLobbyMode = class'GearUISceneFELobby_Campaign'.static.GetCampaignLobbyMode(EnterLobbyString);

	if (GameInterface != None)
	{
		CoopGameSettings = GearCoopGameSettings(GameInterface.GetGameSettings('Party'));
	}
	MaxPlayers = 2;
}

/**
 * Called after a successful login. This is the first place it is safe to call replicated functions on the PlayerController.
 *
 * @param	NewPlayer	the PlayerController that just successfully connected and logged in.
 */
event PostLogin( PlayerController NewPlayer )
{
	local GearMenuPC MenuPC;

	Super.PostLogin(NewPlayer);

	MenuPC = GearMenuPC(NewPlayer);
	if ( MenuPC != None )
	{
		if ( !NewPlayer.IsLocalPlayerController() && NewPlayer.IsPrimaryPlayer() )
		{
			// tell the player to open the lobby scene, if they are ready to.
			MenuPC.ClientOpenScene(LobbySceneResource);
		}
		// Have the client send us their difficulty
		MenuPC.ClientGetCampaignLobbyDifficulty();
	}

	// Don't update counts for local players
	if (!NewPlayer.IsLocalPlayerController())
	{
		// Decrement the number of slots that should show "Connecting..."
		GearCampaignGRI(GameReplicationInfo).ConnectingPlayerCount--;
	}
}

/**
 * Increments the connecting count to block starting while someone is loading
 *
 * @param Options URL options for the match
 * @param Address the IP address of the client connecting
 * @param ErrorMessage an out string that receives the error message
 */
event PreLogin(string Options,string Address,out string ErrorMessage)
{
	Super.PreLogin(Options,Address,ErrorMessage);

	if (GameReplicationInfo != None && len(ErrorMessage) == 0)
	{
		// Increment the number of slots that should show "Connecting..."
		GearCampaignGRI(GameReplicationInfo).ConnectingPlayerCount++;
	}
}

/** Called when a connection closes before getting to PostLogin() */
event NotifyPendingConnectionLost()
{
	local GearCampaignGRI GRI;

	GRI = GearCampaignGRI(GameReplicationInfo);
	if (GRI != None && GRI.ConnectingPlayerCount > 0)
	{
		GRI.ConnectingPlayerCount--;
	}
}

// End GameInfo interface

/**
 * Find the campaign lobby scene and send a message that the attempt to join a game has finished
 */
function FindCoopMatchAttemptFinished()
{
	local GearUISceneFELobby_Campaign CampaignLobby;
	local GameUISceneClient GameSceneClient;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if (GameSceneClient != None)
	{
		foreach GameSceneClient.AllActiveScenes(class'GearUISceneFELobby_Campaign', CampaignLobby, true)
		{
			CampaignLobby.JoinCampaignAttemptFinished();
			break;
		}
	}
}

/**
 * Party has moved from gathering to started so search for a match
 *
 * ControllerId - id of the controller that is searching
 */
function FindCoopMatch(int ControllerId)
{
	local GearCoopGameSearch CoopSearch;
	local int ChapterId, ActId;

	CoopSearch = new class'GearCoopGameSearch';
	// Read the versus mode type and use that for the search
	CoopGameSettings.GetIntProperty(PROPERTY_CHAPTERNUM,ChapterId);
	CoopSearch.SetIntProperty(PROPERTY_CHAPTERNUM,ChapterId);
	CoopGameSettings.GetIntProperty(PROPERTY_ACTNUM,ActId);
	CoopSearch.SetIntProperty(PROPERTY_ACTNUM,ActId);
	GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindCoopMatch);
	// Find a coop match on Live
	OwningControllerIndex = ControllerId;
	GameInterface.FindOnlineGames(OwningControllerIndex,CoopSearch);
}

/**
 * Joins the best coop match if the search was successful
 *
 * @param bWasSuccessful if the search worked or not
 */
function OnFindCoopMatch(bool bWasSuccessful)
{
	local OnlineGameSearch CoopSearch;
	local OnlineGameSearchResult SessionToJoin;
	local int PlayerIndex;
	local int CoopIndex;
	local int Index;
	local UniqueNetId NetId;
	local bool bSuccess;

	GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindCoopMatch);
	if (bWasSuccessful)
	{
		// Start at a value that means "not found"
		CoopIndex = -1;
		// Get our net id so we can filter out our own session from a search
		OnlineSub.PlayerInterface.GetUniquePlayerId(OwningControllerIndex,NetId);
		CoopSearch = GameInterface.GetGameSearch();
		// Find the first match that is not us
		for (Index = 0; Index < CoopSearch.Results.Length && CoopIndex == -1; Index++)
		{
			if (CoopSearch.Results[Index].GameSettings.OwningPlayerId != NetId)
			{
				CoopIndex = Index;
			}
		}

		if (CoopIndex != -1)
		{
			// Determine the best session to join
			SessionToJoin = CoopSearch.Results[0];

			GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinCoopGameComplete);
			// Now join this Session
			GameInterface.JoinOnlineGame(OwningControllerIndex,'Party',SessionToJoin);
			bSuccess = true;
		}
	}

	// If the attempt to join a match failed prompt the player and update the lobby
	if (!bSuccess)
	{
		// Let the lobby know we're done searching so it can update
		FindCoopMatchAttemptFinished();
		PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(OwningControllerIndex);
		class'GearUIScene_Base'.static.DisplayErrorMessage("NoCampaignFound_Message", "NoCampaignFound_Title", "GenericContinue", GetPlayerOwner(PlayerIndex));
	}

	GameInterface.FreeSearchResults();
}

/**
 * Has the coop host traveled to the session that was just joined
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the join completed successfully or not
 */
function OnJoinCoopGameComplete(name SessionName,bool bWasSuccessful)
{
	local string URL;

	GameInterface.ClearJoinOnlineGameCompleteDelegate(OnJoinCoopGameComplete);

	if (SessionName == 'Party')
	{
		if (bWasSuccessful)
		{
			// We are joining so grab the connect string to use
			if (GameInterface.GetResolvedConnectString('Party',URL))
			{
				`Log("Resulting url for 'Party' is ("$URL$")");
				// Trigger a console command to connect to this url
				class'GearUIScene_Base'.static.ShowLoadingMovie(true);
				//@todo should this use ClientTravel instead? otherwise the UI doesn't get the notification...
				ConsoleCommand("open "$URL);
			}
			else
			{
//@todo RobM -- show some ui
			}
		}
	}
	// Let the lobby know we're done searching so it can update
	FindCoopMatchAttemptFinished();
}

/** Calculates updated values for the coop session's dynamic values and begins the update task */
function UpdateCoopSettings()
{
	// Only need to update when hosting a coop match
	if (CampaignLobbyMode == eGCLOBBYMODE_Host)
	{
		GameInterface.AddUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
		if (GameInterface.UpdateOnlineGame('Party',CoopGameSettings,true))
		{
			OpenUpdatingPartyScene();
		}
		else
		{
			GameInterface.ClearUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
		}
	}
}

/**
 * Called when the update of the session state has completed
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the update completed ok or not
 */
function OnUpdateComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
	if (SessionName == 'Party')
	{
		CloseUpdatingPartyScene();
	}
}

/** Closes the party session so we can join a SystemLink or Local game */
function bool DestroyCampaign()
{
	GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyComplete);
	if (GameInterface.DestroyOnlineGame('Party'))
	{
		CoopGameSettings = None;
		OpenUpdatingPartyScene();
	}
	else
	{
		GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyComplete);
	}
	return true;
}

/**
 * Delegate fired when a destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnDestroyComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyComplete);
	CloseUpdatingPartyScene();
}

defaultproperties
{
	OwningControllerIndex=INDEX_NONE

	// Use seamless travel when handling server travel requests
	bUseSeamlessTravel=false

	// Specify the campaign lobby specific PC
	PlayerControllerClass=class'GearGame.GearCampaignLobbyPC'

	// Use the campaign lobby game PRI
	PlayerReplicationInfoClass=class'GearGame.GearCampaignLobbyPRI'

	GameReplicationInfoClass=class'GearGame.GearCampaignGRI'
}
