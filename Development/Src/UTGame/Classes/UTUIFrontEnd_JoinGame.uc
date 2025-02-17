/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Join Game scene for UT3.
 */
class UTUIFrontEnd_JoinGame extends UTUIFrontEnd;

/** Tab page references for this scene. */
var UTUITabPage_ServerBrowser	ServerBrowserTab;
var UTUITabPage_ServerFilter 	ServerFilterTab;
var	UTUITabPage_ServerHistory	ServerHistoryTab;
var	UTUITabPage_ServerFavorites	ServerFavoritesTab;

/** true when we're opened via the campaign menu's 'join online game' option */
var transient	bool		bCampaignMode;

/**
 * Tracks whether a query has been initiated.  Set to TRUE once the first query is started - this is how we catch cases
 * where the user clicked on the sb tab directly instead of clicking the Search button.
 */
var	transient	bool		bIssuedInitialQuery, bIssuedInitialHistoryQuery, bIssuedInitialFavoritesQuery;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize()
{
	Super.PostInitialize();

	// Grab a reference to the server filter tab.
	ServerFilterTab = UTUITabPage_ServerFilter(FindChild('pnlServerFilter', true));
	if(ServerFilterTab != none)
	{
		TabControl.InsertPage(ServerFilterTab, 0, INDEX_NONE, true);
		ServerFilterTab.OnAcceptOptions = OnServerFilter_AcceptOptions;
		ServerFilterTab.OnSwitchedGameType = ServerFilterChangedGameType;
	}

	// Grab a reference to the server browser tab.
	ServerBrowserTab = UTUITabPage_ServerBrowser(FindChild('pnlServerBrowser', true));
	if(ServerBrowserTab != none)
	{
		TabControl.InsertPage(ServerBrowserTab, 0, INDEX_NONE, false);
		ServerBrowserTab.OnBack = OnServerBrowser_Back;
		ServerBrowserTab.OnSwitchedGameType = ServerBrowserChangedGameType;

		// this is no longer needed, as we call SaveSubscriberValue on each option as its changed
		//ServerBrowserTab.OnPrepareToSubmitQuery = PreSubmitQuery;
	}

	ServerHistoryTab = UTUITabPage_ServerHistory(FindChild('pnlServerHistory', true));
	if ( ServerHistoryTab != None )
	{
		TabControl.InsertPage(ServerHistoryTab, GetBestPlayerIndex(),, false);
		ServerHistoryTab.OnBack = OnServerBrowser_Back;
		ServerHistoryTab.OnAddToFavorite = OnServerHistory_AddToFavorite;

		// this is no longer needed, as we call SaveSubscriberValue on each option as its changed
		//ServerHistoryTab.OnPrepareToSubmitQuery = PreSubmitQuery;
	}

	ServerFavoritesTab = UTUITabPage_ServerFavorites(FindChild('pnlServerFavorites',true));
	if ( ServerFavoritesTab != None )
	{
		TabControl.InsertPage(ServerFavoritesTab, GetBestPlayerIndex(), , false);
		ServerFavoritesTab.OnBack = OnServerBrowser_Back;
	}

	// Let the currently active page setup the button bar.
	SetupButtonBar();
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	Super.SceneDeactivated();

	// if we're leaving the server browser area - clear all stored server query searches
	if ( ServerBrowserTab != None )
	{
		ServerBrowserTab.Cleanup();
	}

	if ( ServerHistoryTab != None )
	{
		ServerHistoryTab.Cleanup();
	}

	if ( ServerFavoritesTab != None )
	{
		ServerFavoritesTab.Cleanup();
	}
}

/**
 * Handler for the 'show' animation completed.
 */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	Super.OnMainRegion_Show_UIAnimEnd(AnimTarget, AnimName, TrackTypeMask);

	if ( AnimName == 'SceneShowInitial' )
	{
		// make sure we can't choose "internet" if we aren't signed in online
		ServerFilterTab.ValidateServerType(bCampaignMode);
	}
}

/** Callback for when the login changes after showing the login UI. */
function OnLoginUI_LoginChange()
{
	Super.OnLoginUI_LoginChange();

	if ( bCampaignMode )
	{
		ServerFilterTab.CampaignLoginCompleted();
	}
}


/**
 * Delegate used in notifying the UI/game that the manual login failed after showing the login UI.
 *
 * @param ControllerId	the controller number of the associated user
 * @param ErrorCode		the async error code that occurred
 */
function OnLoginUI_LoginFailed( byte ControllerId,EOnlineServerConnectionStatus ErrorCode)
{
	Super.OnLoginUI_LoginFailed(ControllerId, ErrorCode);

	if ( bCampaignMode )
	{
		ServerFilterTab.CampaignLoginCompleted();
	}
}

/**
 * Called when the server browser page is activated.  Begins a server list query if the page was activated by the user
 * clicking directly on the server browser's tab (as opposed clicking the Search button or pressing enter or something).
 *
 * @param	Sender			the tab control that activated the page
 * @param	NewlyActivePage	the page that was just activated
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this event.
 */
function OnPageActivated( UITabControl Sender, UITabPage NewlyActivePage, int PlayerIndex )
{
	Super.OnPageActivated(Sender, NewlyActivePage, PlayerIndex);

	if ( NewlyActivePage == ServerBrowserTab && !bIssuedInitialQuery )
	{
		bIssuedInitialQuery = true;

		ServerBrowserTab.RefreshServerList(PlayerIndex);
	}
	else if ( !bIssuedInitialHistoryQuery && NewlyActivePage == ServerHistoryTab )
	{
		bIssuedInitialHistoryQuery = true;
		ServerHistoryTab.RefreshServerList(PlayerIndex);
	}
	else if ( !bIssuedInitialFavoritesQuery && NewlyActivePage == ServerFavoritesTab )
	{
		bIssuedInitialFavoritesQuery = true;
		ServerFavoritesTab.RefreshServerList(PlayerIndex);
	}
}

/** Sets up the button bar for the scene. */
function SetupButtonBar()
{
	if(ButtonBar != None)
	{
		ButtonBar.Clear();

		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Search>", OnButtonBar_Search);

		if ( TabControl != None && UTTabPage(TabControl.ActivePage) != None )
		{
			// Let the current tab page try to setup the button bar
			UTTabPage(TabControl.ActivePage).SetupButtonBar(ButtonBar);
		}
	}
}

/**
 * Handler for the server filter panel's OnSwitchedGameType delegate - updates the combo box on the server browser menu
 */
function ServerFilterChangedGameType()
{
	if ( ServerBrowserTab != None )
	{
		ServerBrowserTab.NotifyGameTypeChanged();
	}
}

/**
 * Handler for the server browser panel's OnSwitchedGameType delegate - updates the options in the Filter panel
 * for the newly selected game type.
 */
function ServerBrowserChangedGameType()
{
	if ( ServerFilterTab != None )
	{
		ServerFilterTab.MarkOptionsDirty();
	}
}

/**
 * Handler for the sb tab's OnPrepareToSubmitQuery delegate.  Publishes all configured settings to the game search object.
 */
function PreSubmitQuery( UTUITabPage_ServerBrowser ServerBrowser )
{
	SaveSceneDataValues(false);
}

/** Shows the previous tab page, if we are at the first tab, then we close the scene. */
function ShowPrevTab()
{
	if ( !TabControl.ActivatePreviousPage(0,false,false) )
	{
		if ((ServerBrowserTab == None	|| ServerBrowserTab.AllowCloseScene())
		&&	(ServerHistoryTab == None	|| ServerHistoryTab.AllowCloseScene())
		&&	(ServerFavoritesTab == None	|| ServerFavoritesTab.AllowCloseScene()))
		{
			CloseScene(self);
		}
	}
}

/** Shows the next tab page, if we are at the last tab, then we start the game. */
function ShowNextTab()
{
	TabControl.ActivateNextPage(0,false,false);
}

/** Called when the user accepts their filter settings and wants to go to the server browser. */
function OnAcceptFilterOptions(int PlayerIndex)
{
	bIssuedInitialQuery = true;

	ShowNextTab();

	// Start a game search
	if ( TabControl.ActivePage == ServerBrowserTab )
	{
		ServerBrowserTab.RefreshServerList(PlayerIndex);
	}
}

/** Called when the user accepts their filter settings and wants to go to the server browser. */
function OnServerFilter_AcceptOptions(UIScreenObject InObject, int PlayerIndex)
{
	OnAcceptFilterOptions(PlayerIndex);
}

/** Called when the user wants to back out of the server browser. */
function OnServerBrowser_Back()
{
	ShowPrevTab();
}

/**
 * Handler for when user moves a server from the server history tab to the server favorites tab; refreshes
 * the server favorites query if the favorites tab is active; otherwise flags the server favorites to be
 * requeried the next time that tab is shown
 */
function OnServerHistory_AddToFavorite()
{
	if (ServerFavoritesTab != None && TabControl != None )
	{
		if ( TabControl.ActivePage == ServerFavoritesTab )
		{
			ServerFavoritesTab.RefreshServerList(GetBestPlayerIndex());
		}
		else
		{
			bIssuedInitialFavoritesQuery = false;
		}
	}
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Search(UIScreenObject InButton, int PlayerIndex)
{
	OnAcceptFilterOptions(PlayerIndex);

	return true;
}


function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	ShowPrevTab();

	return true;
}

/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;
	local UTTabPage CurrentTabPage;

	// Let the tab page's get first chance at the input
	CurrentTabPage = UTTabPage(TabControl.ActivePage);
	bResult=CurrentTabPage.HandleInputKey(EventParms);

	// If the tab page didn't handle it, let's handle it ourselves.
	if(bResult==false)
	{
		if(EventParms.EventType==IE_Released)
		{
			if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
			{
				ShowPrevTab();
				bResult=true;
			}
		}
	}

	return bResult;
}

/**
 * Switch to the Campaign filter and show the tab
 */
function UseCampaignMode()
{
	local int ValueIndex;
	local OnlineGameSearch CurrentSearchSettings;

	ValueIndex = ServerFilterTab.MenuDataStore.FindValueInProviderSet('GameModeFilter', 'GameSearchClass', "UTGameSearchCampaign");

	if(ValueIndex != -1)
	{
		bCampaignMode = true;

		// make sure that the "Pure Server" option is set to ANY
		ServerFilterTab.MenuDataStore.GameModeFilter = ValueIndex;
		ServerFilterTab.MarkOptionsDirty();
		ServerFilterTab.SearchDataStore.SetCurrentByName('UTGameSearchCampaign', false);

		CurrentSearchSettings = ServerFilterTab.SearchDataStore.GetCurrentGameSearch();
		if ( CurrentSearchSettings != None )
		{
			CurrentSearchSettings.SetStringSettingValue(CONTEXT_PURESERVER, CONTEXT_PURESERVER_ANY, false);
		}

		ServerFilterChangedGameType();
	}
	TabControl.RemovePage(ServerHistoryTab, GetBestPlayerIndex());
	TabControl.RemovePage(ServerFavoritesTab, GetBestPlayerIndex());
	ServerHistoryTab = None;
	ServerFavoritesTab = None;
}

/**
 * Notification that the player's connection to the platform's online service is changed.
 */
function NotifyOnlineServiceStatusChanged( EOnlineServerConnectionStatus NewConnectionStatus )
{
	Super.NotifyOnlineServiceStatusChanged(NewConnectionStatus);

	if ( NewConnectionStatus != OSCS_Connected )
	{
		// make sure we are using the LAN option
		ServerFilterTab.ForceLANOption(GetBestPlayerIndex());
		if ( bIssuedInitialQuery )
		{
			ServerBrowserTab.CancelQuery(QUERYACTION_RefreshAll);
		}
		if ( bIssuedInitialHistoryQuery )
		{
			ServerHistoryTab.CancelQuery(QUERYACTION_RefreshAll);
		}
		if ( bIssuedInitialHistoryQuery )
		{
			ServerFavoritesTab.CancelQuery(QUERYACTION_RefreshAll);
		}

		ServerBrowserTab.NotifyGameTypeChanged();
	}
}

defaultproperties
{
	bMenuLevelRestoresScene=true
	bRequiresNetwork=true
}
