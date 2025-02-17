/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Leaderboards scene for UT3.
 */
class UTUIFrontEnd_Leaderboards extends UTUIFrontEnd;

/** Reference to the stats datastore. */
var transient UTDataStore_OnlineStats StatsDataStore;

/** Reference to the stats interface. */
var transient OnlineStatsInterface StatsInterface;

/** Refreshing label. */
var transient UILabel RefreshingLabel;

/** MatchType label. */
var transient UILabel MatchTypeLabel;

/** ViewType label. */
var transient UILabel ViewTypeLabel;

/** List of stats results. */
var transient UIList StatsList;

/** Reference to the scene to show when displaying details for a player. */
var string DetailsScene;

/** Index of the player we went to get detailed stats about */
var int CurrentPlayerIndex;

/** Activated event for the scene, should set focus to the main list. */
event SceneActivated(bool bInitialActivation)
{
	Super.SceneActivated(bInitialActivation);

	StatsList.SetFocus(none);

	//Be sure to fit the stats information to the space
    StatsList.ColumnAutoSizeMode = CELLAUTOSIZE_Constrain;
	StatsList.bAllowColumnResizing = false;
	if(bInitialActivation==false)
	{
		StatsDataStore.ResetToDefaultRead();
		StatsList.RefreshSubscriberValue();
		if (CurrentPlayerIndex >= 0)
		{
			//Restore who we were focused on when we left
			StatsList.SetIndex(CurrentPlayerIndex);
		}
	}
}

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize()
{
	local UTLeaderboardSettings LBSettings;
	local OnlineSubsystem OnlineSub;
	local int CurrentValueIndex;

	Super.PostInitialize();

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the game interface to verify the subsystem supports it
		StatsInterface = OnlineSub.StatsInterface;

		if(StatsInterface==None)
		{
			`Log("UTUIFrontEnd_Leaderboards::PostInitialize() - Stats Interface is None!");
		}
	}
	else
	{
		`Log("UTUIFrontEnd_Leaderboards::PostInitialize() - OnlineSub is None!");
	}

	// Store a reference to the stats datastore.
	StatsDataStore = UTDataStore_OnlineStats(FindDataStore('UTLeaderboards'));
	LBSettings = UTLeaderboardSettings(StatsDataStore.LeaderboardSettings);

	RefreshingLabel = UILabel(FindChild('lblRefreshing',true));

	MatchTypeLabel = UILabel(FindChild('lblMatchType',true));
	if(MatchTypeLabel != None && LBSettings.GetStringSettingValue(LF_MatchType, CurrentValueIndex))
	{
		MatchTypeLabel.SetDataStoreBinding(string(LBSettings.GetStringSettingValueName(LF_MatchType, CurrentValueIndex)));
	}

	ViewTypeLabel = UILabel(FindChild('lblViewType',true));
	if(ViewTypeLabel != None && LBSettings.GetStringSettingValue(LF_PlayerFilterType, CurrentValueIndex))
	{
		ViewTypeLabel.SetDataStoreBinding(string(LBSettings.GetStringSettingValueName(LF_PlayerFilterType, CurrentValueIndex)));
	}

	// Stats list submitting a selection
	StatsList = UIList(FindChild('lstStats', true));
	StatsList.OnSubmitSelection = OnStatsList_SubmitSelection;

	RefreshStats();
}

/* Called when the stats list has been updated, so we can set focus on the player's place in the list */
function OnRankListChanged()
{
	local UTPlayerController UTPC;
	local string ActiveElement;
	local int ListIndex;

	UTPC = GetUTPlayerOwner();
	if (UTPC != none)
	{
	   ActiveElement = UTPC.PlayerReplicationInfo.PlayerName;
	   ListIndex = StatsList.FindItemIndex(ActiveElement, 1);
	   if (ListIndex >= 0)
	   {
		   StatsList.SetIndex(ListIndex);
	   }
	}
}

/** Setup the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.PlayerDetails>", OnButtonBar_Details);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ToggleView>", OnButtonBar_ToggleView);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ToggleMatchType>", OnButtonBar_ToggleMatchType);
}

/** Callback for when the user wants to exit the scene. */
function OnBack()
{
	CloseScene(self);
}

/** Displays the details page for the currently selected leaderboard player. */
function OnDetails()
{
	CurrentPlayerIndex = StatsList.GetCurrentItem();
	if (CurrentPlayerIndex >= 0)
	{
		StatsDataStore.SetDetailedStatsRowIndex(CurrentPlayerIndex);
		OpenSceneByName(DetailsScene);
	}
}

/** Toggles the current game mode. */
function OnToggleGameMode()
{
	local UTLeaderboardSettings LBSettings;

	LBSettings = UTLeaderboardSettings(StatsDataStore.LeaderboardSettings);
	LBSettings.MoveToNextSettingValue(LF_GameMode);

    `Log("UTUIFrontEnd_Leaderboards::OnToggleGameMode() - Moving to next game mode");
	//RefreshStats();  All stats are aggregate now
}

/** Toggles the current view type */
function OnToggleView()
{
	local UTLeaderboardSettings LBSettings;
	local string ViewType;
	local int CurrentValueIndex;

	LBSettings = UTLeaderboardSettings(StatsDataStore.LeaderboardSettings);
	LBSettings.MoveToNextSettingValue(LF_PlayerFilterType);

	//Tell the UI the new view type name
	if(LBSettings.GetStringSettingValue(LF_PlayerFilterType, CurrentValueIndex))
	{
		ViewType = string(LBSettings.GetStringSettingValueName(LF_PlayerFilterType, CurrentValueIndex));
		ViewTypeLabel.SetDataStoreBinding(ViewType);
	}

	`Log("UTUIFrontEnd_Leaderboards::OnToggleViewType() - Moving to next view type(player filter)");
	RefreshStats();
}


/** Toggles the current match type. */
function OnToggleMatchType()
{
	local UTLeaderboardSettings LBSettings;
	local string MatchType;
    local int CurrentValueIndex;


	LBSettings = UTLeaderboardSettings(StatsDataStore.LeaderboardSettings);
	LBSettings.MoveToNextSettingValue(LF_MatchType);

    //Tell the UI the new match type name
	if(LBSettings.GetStringSettingValue(LF_MatchType, CurrentValueIndex))
	{
        MatchType = string(LBSettings.GetStringSettingValueName(LF_MatchType, CurrentValueIndex));
		MatchTypeLabel.SetDataStoreBinding(MatchType);
	}

    `Log("UTUIFrontEnd_Leaderboards::OnToggleMatchType() - Moving to next match type"@MatchType);
	RefreshStats();
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

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_X')
		{
			OnToggleView();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnToggleMatchType();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
	}

	return bResult;
}


/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_Details(UIScreenObject InButton, int PlayerIndex)
{
	OnDetails();

	return true;
}


function bool OnButtonBar_ToggleGameMode(UIScreenObject InButton, int PlayerIndex)
{
	OnToggleGameMode();

	return true;
}

function bool OnButtonBar_ToggleView(UIScreenObject InButton, int PlayerIndex)
{
	OnToggleView();

	return true;
}

function bool OnButtonBar_ToggleMatchType(UIScreenObject InButton, int PlayerIndex)
{
	OnToggleMatchType();

	return true;
}



/** Refreshes the leaderboard stats list. */
function RefreshStats()
{
	// Add the delegate for when the read is complete.
	StatsInterface.AddReadOnlineStatsCompleteDelegate(OnStatsReadComplete);

	// Show the refreshing label
	StatsList.SetVisibility(false);
	RefreshingLabel.SetVisibility(true);
	ButtonBar.Buttons[1].SetEnabled(false);
	ButtonBar.Buttons[2].SetEnabled(false);
	ButtonBar.Buttons[3].SetEnabled(false);

	// Issue the stats.
	`Log("UTUIFrontEnd_Leaderboards::RefreshStats() - Refreshing leaderboard.");
	StatsDataStore.ResetToDefaultRead();
	if( StatsDataStore.RefreshStats(GetPlayerOwner().ControllerId) == false )
	{
		OnStatsReadComplete(false);
	}
}

/** Callback for when the stats read has completed. */
function OnStatsReadComplete(bool bWasSuccessful)
{
	`Log("UTUIFrontEnd_Leaderboards::OnStatsReadComplete() - Stats read completed, bWasSuccessful: " $ bWasSuccessful);

	// Hide refreshing label.
	StatsList.SetVisibility(true);
	RefreshingLabel.SetVisibility(false);
	StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnStatsReadComplete);

	ButtonBar.Buttons[1].SetEnabled(true);
	ButtonBar.Buttons[2].SetEnabled(true);
	ButtonBar.Buttons[3].SetEnabled(true);

	OnRankListChanged();
}

/** Callback for when the user submits the currently selected list item. */
function OnStatsList_SubmitSelection( UIList Sender, optional int PlayerIndex )
{
	OnDetails();
}

defaultproperties
{
	DetailsScene="UI_Scenes_FrontEnd.Scenes.PlayerStatsDetails"
	bRequiresNetwork=true
	bRequiresOnlineService=true
}
