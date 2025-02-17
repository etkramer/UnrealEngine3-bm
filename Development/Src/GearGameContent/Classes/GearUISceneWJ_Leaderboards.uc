/**
 * Displays the Xbox Live leaderboards for various gamemodes and friends lists.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_Leaderboards extends GearUISceneWJ_PopupBase;

var	transient	UILabel		lblPlayerCount;
var	transient	UILabel		lblGametype;
var	transient	UILabel		lblCurrentFilter;
var	transient	UILabel		lblTimeFrame;
var	transient	UILabel		lblPageTitle;
var	transient	UIList		lstVsLeaderboard;
var	transient	UIList		lstHordeLeaderboard;
var	transient	UILabel		lblRightBumper;
var	transient	UILabel		lblLeftBumper;
var transient UICalloutButtonPanel ButtonBar;
var transient string TotalPlayersBinding;
var	transient	UIList		lstLeaderboard;

/** Caches the leaderboard datastore so we can refresh or manipulate the filter */
var transient	GearLeaderboardsDataStoreBase			LeaderboardsDataStore;

/** ref to the stats details scene */
var	transient	GearUISceneWJ_LeaderboardDetails	DetailsSceneResource;

/** The type of leaderboard being displayed */
enum ELeaderboardType
{
	LBT_Public,
	LBT_Private,
	LBT_Horde
};

/** The type of leaderboard being displayed so we can bind to different datastores */
var	transient ELeaderboardType LeaderboardType;

/** String to display on top when viewing public leaderboards */
var transient localized string PublicLeaderboardString;
/** String to display on top when viewing private leaderboards */
var transient localized string PrivateLeaderboardString;
/** String to display on top when viewing horde leaderboards */
var transient localized string HordeLeaderboardString;

/** Whether the friends list was read or not */
var transient bool bHasReadFriends;


/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	lblPlayerCount		=	UILabel(FindChild('lblPlayerCount',true));
	lblGametype			=	UILabel(FindChild('lblGametype',true));
	lblCurrentFilter	=	UILabel(FindChild('lblCurrentFilter',true));
	lstVsLeaderboard	=	UIList(FindChild('lstLeaderboard',true));
	lblTimeFrame		=	UILabel(FindChild('lblTimeFrame',true));
	lblPageTitle		=	UILabel(FindChild('lblPageTitle',true));
	lstHordeLeaderboard	=	UIList(FindChild('lstHordeLeaderboard',true));
	lblRightBumper		=	UILabel(FindChild('lblRightBumper',true));
	lblLeftBumper		=	UILabel(FindChild('lblLeftBumper',true));
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	lstVsLeaderboard.OnSubmitSelection = LeaderboardClicked;

	ButtonBar = UICalloutButtonPanel(FindChild('btnbarMain',true));
	ButtonBar.SetButtonCallback('LB_TimeFrame', ChangeLeaderboardType);
	ButtonBar.SetButtonCallback('LB_Playerfilter', ChangeLeaderboardFilter);
	ButtonBar.SetButtonCallback('ShowGamercard', ShowGamercard);
}

/**
 * Handler for the leaderboard's OnSubmitSelection delegate.  Opens the leaderboard details scene for the selected player.
 *
 * @param	Sender	the list that is submitting the selection
 */
function LeaderboardClicked( UIList Sender, optional int PlayerIndex=GetBestPlayerIndex() )
{
	OpenDetailsScene(PlayerIndex);
}

/**
 * Wrapper for opening the scene that displays more detailed information about the selected player.
 */
function OpenDetailsScene( int PlayerIndex )
{
	local GearUISceneWJ_LeaderboardDetails DetailsScene;

	if (LeaderboardType != LBT_Horde)
	{
		DetailsScene = GearUISceneWJ_LeaderboardDetails(OpenScene(DetailsSceneResource, GetPlayerOwner()));
		if ( DetailsScene != None )
		{
			DetailsScene.SetLeaderboardIndex(LeaderboardsDataStore,lstLeaderboard.GetCurrentItem());
		}
	}
}

/**
 * Shows the gamer card for the selected index
 */
function bool ShowGamercard( UIScreenObject EventObject, int PlayerIndex )
{
	if ( LeaderboardsDataStore != None )
	{
		// Show the gamercard for the player that is selected in the list
		LeaderboardsDataStore.ShowGamercard(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex),lstLeaderboard.GetCurrentItem());
	}
	return true;
}

/**
 * Changes the player filter type for the leaderboard
 */
function bool ChangeLeaderboardFilter( UIScreenObject EventObject, int PlayerIndex )
{
	ChangeFilterOption(LF_PlayerFilterType);
	UpdateFilterOptionText(lblCurrentFilter,LF_PlayerFilterType);
	return true;
}

/**
 * Changes the player filter type for the leaderboard
 */
function bool ChangeLeaderboardType( UIScreenObject EventObject, int PlayerIndex )
{
	ChangeFilterOption(LF_TimePeriod);
	UpdateFilterOptionText(lblTimeFrame,LF_TimePeriod);
	return true;
}

/**
 * Increments the specified option and wraps if it goes out of bounds
 *
 * @param FilterType the filter option to change
 * @param Direction the direction to move the selection (1 forward, -1 backward)
 */
function ChangeFilterOption(int FilterType,optional int Direction = 1)
{
	if ( LeaderboardsDataStore != None )
	{
		if ( LeaderboardsDataStore.LeaderboardSettings != None)
		{
			// Increment/decrement the selected item
			LeaderboardsDataStore.LeaderboardSettings.IncrementStringSettingValue(FilterType,Direction,true);
			RefreshStats();
		}
		else
		{
			`warn(`location @ "Leaderboards data store has NULL value for LeaderboardSettings!");
		}
	}
	else
	{
		`warn(`location @ "no Leaderboards data store found!");
	}
}

/**
 * Sets the text to the currently selected option in the specified filter type
 *
 * @param Label the UI element being updated
 * @param FilterType the filter type to query for the current value
 */
function UpdateFilterOptionText(UILabel Label,int FilterType)
{
	local int Value;
	local string Text;

	if ( LeaderboardsDataStore != None )
	{
		if ( LeaderboardsDataStore.LeaderboardSettings != None)
		{
			LeaderboardsDataStore.LeaderboardSettings.GetStringSettingValue(FilterType,Value);
			Text = string(LeaderboardsDataStore.LeaderboardSettings.GetStringSettingValueName(FilterType,Value));
			Label.SetDataStoreBinding(Text);
		}
		else
		{
			`warn(`location @ "Leaderboards data store has NULL value for LeaderboardSettings!");
		}
	}
	else
	{
		`warn(`location @ "no Leaderboards data store found!");
	}
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handler for the leaderboard list's OnRawInputKey delegate.  In Gears2, the triggers are not linked to the page up / page down input alias,
 * so for lists that need to support this, we must handle it manually.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool OnReceivedInputKey( const out InputEventParameters EventParms )
{
	local int PreviousIndex;
	local bool bResult;

	if ( EventParms.PlayerIndex == GetPlayerOwnerIndex() )
	{
		if ( EventParms.InputKeyName == 'XboxTypeS_LeftTrigger' )
		{
			if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
			{
				PreviousIndex = lstLeaderboard.Index;
				if ( lstLeaderboard.NavigateIndex(false, true, false) && PreviousIndex != lstLeaderboard.Index )
				{
					lstLeaderboard.PlayUISound(lstLeaderboard.DecrementIndexCue, EventParms.PlayerIndex);
				}
			}

			bResult = true;
		}
		else if ( EventParms.InputKeyName == 'XboxTypeS_RightTrigger' )
		{
			if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
			{
				PreviousIndex = lstLeaderboard.Index;
				if ( lstLeaderboard.NavigateIndex(true, true, false) && PreviousIndex != lstLeaderboard.Index )
				{
					lstLeaderboard.PlayUISound(lstLeaderboard.IncrementIndexCue, EventParms.PlayerIndex);
				}
			}

			bResult = true;
		}
	}

	return bResult;
}

/**
 * Handler for the scene's OnProcessInputKey delegate.
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called AFTER kismet is given a chance to process the input, but BEFORE any native code processes the input.
 *
 * @param	EventParms	information about the input event, including the name of the input alias associated with the
 *						current key name (Tab, Space, etc.), event type (Pressed, Released, etc.) and modifier keys (Ctrl, Alt)
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	local bool bResult;

	if ( EventParms.EventType == IE_Released )
	{
		if ( HandleCalloutButtonClick(EventParms.InputAliasName, EventParms.PlayerIndex) )
		{
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'PreviousLeaderboardGametype' )
		{
			if (LeaderboardType != LBT_Horde)
			{
				// view stats for a different gametype
				ChangeFilterOption(LF_GameMode,-1);
				UpdateFilterOptionText(lblGametype,LF_GameMode);
				// Also update the timeperiod since Horde/private only has all time
				UpdateFilterOptionText(lblTimeFrame,LF_TimePeriod);
			}
			else
			{
				ChangeFilterOption(LF_MapType,-1);
				UpdateFilterOptionText(lblGametype,LF_MapType);
			}

			PlayUISound('G2UI_JournalNavigateCue');
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'NextLeaderboardGametype' )
		{
			if (LeaderboardType != LBT_Horde)
			{
				// view stats for a different gametype
				ChangeFilterOption(LF_GameMode);
				UpdateFilterOptionText(lblGametype,LF_GameMode);
				// Also update the timeperiod since Horde/private only has all time
				UpdateFilterOptionText(lblTimeFrame,LF_TimePeriod);
			}
			else
			{
				ChangeFilterOption(LF_MapType);
				UpdateFilterOptionText(lblGametype,LF_MapType);
			}

			PlayUISound('G2UI_JournalNavigateCue');
			bResult = true;
		}
	}

	return bResult;
}

/** Called when the scene is activated so we can set the difficulty strings */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local GearUISceneWJ_TOCMain TOCParent;
	local string ItemTag;
	local name LeaderboardDSName;

	if ( bInitialActivation )
	{
		TOCParent = GearUISceneWJ_TOCMain(GetPreviousScene(true));
		if ( TOCParent != None && TOCParent.TOCPanel != None && TOCParent.TOCPanel.lstNavigation != None )
		{
			ItemTag = TOCParent.TOCPanel.lstNavigation.GetSelectedItemTag();
			// Determine which leaderboard we are displaying
			switch (ItemTag)
			{
				case "PublicLeaderboards":
					LeaderboardType = LBT_Public;
					break;
				case "PrivateLeaderboards":
					LeaderboardType = LBT_Private;
					break;
				case "HordeLeaderboards":
					LeaderboardType = LBT_Horde;
					break;
			}
		}

		// Set up the bindings based upon leaderboard type
		switch (LeaderboardType)
		{
			case LBT_Public:
				LeaderboardDSName = 'GearLeaderboardsPublic';
				lstLeaderboard = lstVsLeaderboard;
				lstLeaderboard.SetDatastoreBinding("<GearLeaderboardsPublic:StatsReadResults>");
				lblPageTitle.SetDatastoreBinding(PublicLeaderboardString);
				TotalPlayersBinding = "<GearLeaderboardsPublic:TotalRows>";
				break;
			case LBT_Private:
				LeaderboardDSName = 'GearLeaderboardsPrivate';
				lstLeaderboard = lstVsLeaderboard;
				lstLeaderboard.SetDatastoreBinding("<GearLeaderboardsPrivate:StatsReadResults>");
				lblPageTitle.SetDatastoreBinding(PrivateLeaderboardString);
				TotalPlayersBinding = "<GearLeaderboardsPrivate:TotalRows>";
				ButtonBar.EnableButton('LB_Playerfilter', GetPlayerOwnerIndex(), false, false);
				ButtonBar.EnableButton('LB_TimeFrame', GetPlayerOwnerIndex(), false, false);
				lblLeftBumper.SetEnabled(false);
				lblRightBumper.SetEnabled(false);
				break;
			case LBT_Horde:
				LeaderboardDSName = 'GearLeaderboardsHorde';
				lstVsLeaderboard.SetVisibility(false);
				lstLeaderboard = lstHordeLeaderboard;
				lstHordeLeaderboard.SetVisibility(true);
				lstHordeLeaderboard.SetDatastoreBinding("<GearLeaderboardsHorde:StatsReadResults>");
				TotalPlayersBinding = "<GearLeaderboardsHorde:TotalRows>";
				lblPageTitle.SetDatastoreBinding(HordeLeaderboardString);
				ButtonBar.EnableButton('LB_TimeFrame', GetPlayerOwnerIndex(), false, false);
				lblRightBumper.SetEnabled(false);
				break;
		}

		// Find the gears leaderboard so we can tell it to refresh
		LeaderboardsDataStore = GearLeaderboardsDataStoreBase(StaticResolveDataStore(LeaderboardDSName, Self, GetPlayerOwner()));

		// Update all of the lablels
		UpdateFilterOptionText(lblCurrentFilter,LF_PlayerFilterType);
		UpdateFilterOptionText(lblTimeFrame,LF_TimePeriod);
		// Horde use the gametype label as the map name
		if (LeaderboardType != LBT_Horde)
		{
			UpdateFilterOptionText(lblGametype,LF_GameMode);
		}
		else
		{
			UpdateFilterOptionText(lblGametype,LF_MapType);
		}

		if ( LeaderboardsDataStore != None && IsConsole() )
		{
			if (!IsLoggedIn(GetBestControllerId(),true))
			{
				ButtonAliases.AddItem('GenericAccept');
				GameSceneClient = class'UIInteraction'.static.GetSceneClient();
				GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedGoldTier_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedGoldTierLeaderboard_Message>",
					"",
					ButtonAliases, CloseOnAccept, GetPlayerOwner());
			}
			else
			{
				RefreshStats();
			}
		}
	}
}

/** Handler to close the scene on click */
function bool CloseOnAccept(UIMessageBoxBase Sender,name SelectedInputAlias,int PlayerIndex)
{
	CloseScene();
	return true;
}

/* === UIScreenObject interface === */
/**
 * Generates a array of UI input aliases that this widget supports.
 *
 * @param	out_KeyNames	receives the list of input alias names supported by this widget
 */
event GetSupportedUIActionKeyNames( out array<Name> out_KeyNames )
{
	Super.GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem('ChangeLeaderboardFilter');
	out_KeyNames.AddItem('ChangeLeaderboardType');
	out_KeyNames.AddItem('PreviousLeaderboardGametype');
	out_KeyNames.AddItem('NextLeaderboardGametype');
}

/** Refreshes the stats and opens a scene while it's updating */
function RefreshStats()
{
	local OnlineSubsystem OnlineSub;
	local GearPC PC;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		OpenCogSpin();

		ControllerId = GetBestControllerId();
		if (bHasReadFriends)
		{
			// Safe to read the stats
			OnlineSub.StatsInterface.AddReadOnlineStatsCompleteDelegate(OnStatsReadComplete);
			LeaderboardsDataStore.RefreshStats(ControllerId);
		}
		else
		{
			// Need to read the friends list first
			PC = GetGearPlayerOwner(GetBestPlayerIndex());
			if (PC != None)
			{
				OnlineSub.PlayerInterface.AddReadFriendsCompleteDelegate(ControllerId,OnRefreshFriendsComplete);
				PC.OnlinePlayerData.FriendsProvider.FriendsList.Length = 0;
				PC.OnlinePlayerData.FriendsProvider.NotifyPropertyChanged();
				PC.OnlinePlayerData.FriendsProvider.RefreshFriendsList();
			}
		}
	}
}

/**
 * Delegate used when the friends read request has completed. Triggers the stats read
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnRefreshFriendsComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		OnlineSub.PlayerInterface.ClearReadFriendsCompleteDelegate(GetBestControllerId(),OnRefreshFriendsComplete);
	}
	// So we don't read friends again while the UI is up
	bHasReadFriends = true;
	RefreshStats();
}

/**
 * Called once the stats read has completed
 *
 * @param bWasSuccessful whether the call completed ok or not
 */
function OnStatsReadComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		OnlineSub.StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnStatsReadComplete);
	}
	lblPlayerCount.SetDatastoreBinding(TotalPlayersBinding);
	lblPlayerCount.RefreshSubscriberValue();
	CloseCogSpin();
}

/** Opens or restarts the updating scene (used for blocking input during async tasks) */
function OpenCogSpin()
{
	local GearUISceneFE_Updating SceneInstance;

	SceneInstance = class'GearUIScene_Base'.static.OpenUpdatingScene();
	if (SceneInstance != None)
	{
		SceneInstance.InitializeUpdatingScene("DLStatsTitle", "DLStatsDesc", 0.5f);
	}
}

/** Begins the process of closing the updating scene (there is a min time the scene must be open) */
function CloseCogSpin()
{
	class'GearUIScene_Base'.static.CloseUpdatingScene();
}

DefaultProperties
{
	bRequiresProfile=true
	bRequiresNetwork=true
	bRequiresOnlineService=true

	OnSceneActivated=OnSceneActivatedCallback
	OnProcessInputKey=ProcessInputKey
	OnRawInputKey=OnReceivedInputKey

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	DetailsSceneResource=GearUISceneWJ_LeaderboardDetails'UI_Scenes_WarJournal.LeaderboardDetails'
}
