/**
 *
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneFE_LAN extends GearUISceneFE_PostGameBase
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** button bar */
var transient UICalloutButtonPanel ButtonBar;

/** The list containing the servers available to join */
var transient UIList ServerList;
/** The list containing the servers available to join for coop campaign */
var transient UIList CoopServerList;

/** The datastore bound to the ServerList */
var transient GearUIDataStore_GameSearchLAN LANDataStore;
/** The datastore bound to the CoopServerList */
var transient GearUIDataStore_CoopSearchLAN CoopLANDataStore;

/** The panel of the feedback strip */
var transient UIPanel FeedbackPanel;
/** The label to draw feedback on */
var transient UILabel FeedbackLabel;

/** Header labels for the list columns */
var transient UILabel PartyLabel;
var transient UILabel PartySlotsLabel;
var transient UILabel CampLabel;
var transient UILabel ActLabel;
var transient UILabel ChapterLabel;

/** Localized strings */
var localized string Searching;
var localized string Cancel;
var localized string NoServers;
var localized string NoCampServers;

/** Whether a search is in progress */
var transient bool bIsSearching;

/** Whether a join is in progress */
var transient bool bIsJoining;

/** Whether this is a coop LAN (else it's party) */
var transient bool bIsCoopLAN;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called if the player chooses to start their own party */
delegate OnStartLANParty();

/** Called if the player canceled */
delegate OnCancelFromLANScene();

/**
 * Called after this screen object's children have been initialized
 * Overloaded to set the deactivated callback
 */
event PostInitialize()
{
	// Initialize references to widgets
	InitializeWidgetReferences();

	Super.PostInitialize();
}

/** Initialize references to widgets */
function InitializeWidgetReferences()
{
	local DataStoreClient DataStoreManager;
	local LocalPlayer LP;

	ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));
	ServerList = UIList(FindChild('listGames', true));
	CoopServerList = UIList(FindChild('listCamps', true));
	FeedbackPanel = UIPanel(FindChild('pnlFeedback', true));
	FeedbackLabel = UILabel(FindChild('lblFeedback', true));
	PartyLabel = UILabel(FindChild('lblPlayers', true));
	PartySlotsLabel = UILabel(FindChild('lblSlots', true));
	CampLabel = UILabel(FindChild('lblCamps', true));
	ActLabel = UILabel(FindChild('lblAct', true));
	ChapterLabel = UILabel(FindChild('lblChapter', true));

	DataStoreManager = class'UIInteraction'.static.GetDataStoreClient();
	if ( DataStoreManager != None )
	{
		LP = GetPlayerOwner(GetBestPlayerIndex());
		LANDataStore = GearUIDataStore_GameSearchLAN(DataStoreManager.FindDataStore('LANGameSearch', LP));
		if ( LANDataStore == None )
		{
			`log("ERROR! No GearUIDataStore_GameSearchLAN found in the LAN server browser!");
		}

		CoopLANDataStore = GearUIDataStore_CoopSearchLAN(DataStoreManager.FindDataStore('LANCoopSearch', LP));
		if ( CoopLANDataStore == None )
		{
			`log("ERROR! No GearUIDataStore_CoopSearchLAN found in the LAN server browser!");
		}
	}
}

/** Assigns delegates in important child widgets to functions in this scene class */
function SetupCallbacks()
{
	local int PlayerIndex;

	PlayerIndex = GetBestPlayerIndex();

	if ( ButtonBar != None )
	{
		ButtonBar.SetButtonCallback( 'Refresh', OnRefreshClicked );
		ButtonBar.SetButtonCallback('GenericCancel', OnCancelClicked);
		if (bIsCoopLAN)
		{
			ButtonBar.SetButtonInputAlias('JoinParty', 'JoinGame');
			ButtonBar.SetButtonCallback('JoinGame', OnCoopSelection);
			ButtonBar.EnableButton('CreateParty', PlayerIndex, false, true);
		}
		else
		{
			ButtonBar.SetButtonInputAlias('JoinGame', 'JoinParty');
			ButtonBar.SetButtonCallback('JoinParty', OnPartySelection);
			ButtonBar.SetButtonCallback('CreateParty', OnCreatePartyClicked);
		}

		RefreshButtonBar(PlayerIndex);
	}

	// Set up visibility for Party widgets
	ServerList.SetVisibility(!bIsCoopLAN);
	PartyLabel.SetVisibility(!bIsCoopLAN);
	PartySlotsLabel.SetVisibility(!bIsCoopLAN);

	// Set up visibitiy for Campaign widgets
	CoopServerList.SetVisibility(bIsCoopLAN);
	CampLabel.SetVisibility(bIsCoopLAN);
	ActLabel.SetVisibility(bIsCoopLAN);
	ChapterLabel.SetVisibility(bIsCoopLAN);

	// Need to set which list will take focus
	if (bIsCoopLAN)
	{
		CoopServerList.SetFocus(none);
	}
	else
	{
		ServerList.SetFocus(none);
	}
}

/** Refreshes the button bar to reflect what the user can actually do */
function RefreshButtonBar(int PlayerIndex)
{
	local bool bEnableButtons;

	if (ButtonBar != none)
	{
		bEnableButtons = !bIsJoining && !bIsSearching;

		ButtonBar.EnableButton('Refresh', PlayerIndex, bEnableButtons, false );
		ButtonBar.EnableButton('GenericCancel', PlayerIndex, bEnableButtons, false );

		if (bIsCoopLAN)
		{
			ButtonBar.EnableButton('JoinGame', PlayerIndex, bEnableButtons && !FeedbackPanel.IsVisible(), false);
			ButtonBar.EnableButton('CreateParty', PlayerIndex, false, true );
		}
		else
		{
			ButtonBar.EnableButton('JoinParty', PlayerIndex, bEnableButtons && !FeedbackPanel.IsVisible(), false);
			ButtonBar.EnableButton('CreateParty', PlayerIndex, bEnableButtons, false );
		}
	}
}

/**
 * Called when the player presses the A button
 */
function bool OnPartySelection( UIScreenObject EventObject, int PlayerIndex )
{
	local int CurrListIndex;
	local OnlineGameSearchResult Result;
	local OnlineSubsystem OnlineSub;

	if ( !bIsSearching && !bIsJoining )
	{
		CurrListIndex = ServerList.GetCurrentItem();
		if ( LANDataStore.GetSearchResultFromIndex(CurrListIndex, Result) )
		{
			bIsJoining = true;
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinPartyComplete);
			// Now join the party that is advertised
			OnlineSub.GameInterface.JoinOnlineGame( GetBestControllerId(), 'Party', Result );
		}
	}

	RefreshButtonBar(GetBestPlayerIndex());
	return true;
}

/**
 * Has the pary host travel to the session that was just joined
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the join completed successfully or not
 */
function OnJoinPartyComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local string URL;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnJoinPartyComplete);

	if ( SessionName == 'Party' && bWasSuccessful )
	{
		// We are joining so grab the connect string to use
		if (OnlineSub.GameInterface.GetResolvedConnectString('Party',URL))
		{
			`Log("Resulting url for 'Party' is ("$URL$")");
			// Trigger a console command to connect to this url
			ShowLoadingMovie(true);
			GetWorldInfo().ConsoleCommand("start " $ URL);
			CloseScene();
		}
	}
	else
	{
		bIsJoining = false;
		DisplayErrorMessage("UnableToConnect_Message", "UnableToConnect_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}

	RefreshButtonBar(GetBestPlayerIndex());
}

/**
 * Called when the player presses the A button
 */
function bool OnCoopSelection( UIScreenObject EventObject, int PlayerIndex )
{
	local int CurrListIndex;
	local OnlineGameSearchResult Result;
	local OnlineSubsystem OnlineSub;

	if ( !bIsSearching && !bIsJoining )
	{
		CurrListIndex = CoopServerList.GetCurrentItem();
		if ( CoopLANDataStore.GetSearchResultFromIndex(CurrListIndex, Result) )
		{
			bIsJoining = true;
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnCoopPartyComplete);
			// Now join the campaign that is advertised
			OnlineSub.GameInterface.JoinOnlineGame( GetBestControllerId(), 'Party', Result );
		}
	}
	return true;
}

/**
 * Has the host travel to the session that was just joined
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the join completed successfully or not
 */
function OnCoopPartyComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local string URL;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnCoopPartyComplete);

	if ( SessionName == 'Party' && bWasSuccessful )
	{
		// We are joining so grab the connect string to use
		if (OnlineSub.GameInterface.GetResolvedConnectString('Party',URL))
		{
			`Log("Resulting url for 'Lobby' is ("$URL$")");
			ShowLoadingMovie(true);
			// Trigger a console command to connect to this url
			GetWorldInfo().ConsoleCommand("start " $ URL);
			CloseScene();
		}
	}
	else
	{
		bIsJoining = false;
		DisplayErrorMessage("UnableToConnect_Message", "UnableToConnect_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}

	RefreshButtonBar(GetBestPlayerIndex());
}

/** Called when the refresh button is pressed */
function bool OnRefreshClicked( UIScreenObject EventObject, int PlayerIndex )
{
	if ( !bIsSearching )
	{
		if (bIsCoopLAN)
		{
			TriggerCoopSearch();
		}
		else
		{
			TriggerPartySearch();
		}
	}
	else
	{
		PlayUISound('Error');
	}
	return true;
}

/** Called when the CreateParty button is pressed */
function bool OnCreatePartyClicked( UIScreenObject EventObject, int PlayerIndex )
{
	if ( !bIsSearching )
	{
		OnStartLANParty();
		//@todo ronp animation
		CloseScene(self);
	}
	else
	{
		PlayUISound('Error');
	}
	return true;
}

/** Called when the Cancel button is pressed */
function bool OnCancelClicked( UIScreenObject EventObject, int PlayerIndex )
{
	if ( !bIsSearching )
	{
		OnCancelFromLANScene();
		//@todo ronp animation
		CloseScene(self);
	}
	else
	{
		PlayUISound('Error');
	}
	return true;
}

/**
 * Delegate fired when the search for an online game has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnFindOnlineGamesComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;

	bIsSearching = false;
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOnlineGamesComplete);
	if ( OnlineSub.GameInterface.GetGameSearch().Results.length == 0 )
	{
		FeedbackPanel.SetVisibility( true );
		FeedbackLabel.SetDataStoreBinding( bIsCoopLAN ? NoCampServers : NoServers );
	}
	else
	{
		FeedbackPanel.SetVisibility( false );
	}

	RefreshButtonBar(GetBestPlayerIndex());
}

/**
 * Allows others to be notified when this scene becomes the active scene.  Called after other activation methods have been called
 * and after focus has been set on the scene
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	if ( ActivatedScene == Self && bInitialActivation && !IsEditor() )
	{
		// Setup any callbacks need, such as the button bar
		SetupCallbacks();

		if (bIsCoopLAN)
		{
			TriggerCoopSearch();
		}
		else
		{
			TriggerPartySearch();
		}
	}
}

/** Starts a party search and turns on the appropriate labels */
function TriggerPartySearch()
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && LANDataStore != None )
	{
		OnlineSub.GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindOnlineGamesComplete);
		LANDataStore.SubmitGameSearch( GetBestControllerId(), true );
		FeedbackPanel.SetVisibility( true );
		FeedbackLabel.SetDataStoreBinding( Searching );
		bIsSearching = true;
	}

	RefreshButtonBar(GetBestPlayerIndex());
}

/** Starts a coop search and turns on the appropriate labels */
function TriggerCoopSearch()
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && CoopLANDataStore != None )
	{
		OnlineSub.GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindOnlineGamesComplete);
		CoopLANDataStore.SubmitGameSearch( GetBestControllerId(), true );
		FeedbackPanel.SetVisibility( true );
		FeedbackLabel.SetDataStoreBinding( Searching );
		bIsSearching = true;
	}

	RefreshButtonBar(GetBestPlayerIndex());
}

event SceneDeactivated()
{
	local OnlineSubsystem OnlineSub;

	Super.SceneDeactivated();

	// make sure we unsubscribed from the online subsystem delegate...
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOnlineGamesComplete);
}

defaultproperties
{
	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	OnSceneActivated=SceneActivationComplete
}

