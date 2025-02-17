/**
 * This class is a specialized server browser which displays the most recently visited servers.  It also allows the player
 * to move a server to the favorites list so that it doesn't get removed from the list if the player visits more servers than
 * the maximum number of servers allowed in the history.  This server browser does not respect filter options.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUITabPage_ServerHistory extends UTUITabPage_ServerBrowser;

var	transient	int		AddFavoriteIdx;

/** called when the user moves a server from the history page to the favorites page */
delegate transient OnAddToFavorite();

/**
 * Sets the correct tab button caption.
 */
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.JoinGame.History>");
}

/**
 * Adjusts the layout of the scene based on the current platform
 */
function AdjustLayout()
{
	Super.AdjustLayout();

	// if we're on the console, the gametype combo will be hidden anyway
	if ( !IsConsole() && GameTypeCombo != None )
	{
		GameTypeCombo.SetVisibility(false);
	}
}

/**
 * Determines which type of matches the user wishes to search for (i.e. LAN, unranked, ranked, etc.)
 */
function int GetDesiredMatchType()
{
	// for history - always return unranked matches
	return SERVERBROWSER_SERVERTYPE_UNRANKED;
}

/**
 * Wrapper for getting a reference to the favorites data store.
 */
function UTDataStore_GameSearchFavorites GetFavoritesDataStore()
{
	local UTDataStore_GameSearchHistory HistorySearchDataStore;
	local UTDataStore_GameSearchFavorites Result;

	HistorySearchDataStore = UTDataStore_GameSearchHistory(SearchDataStore);
	if ( HistorySearchDataStore != None )
	{
		Result = HistorySearchDataStore.FavoritesGameSearchDataStore;
	}

	return Result;
}

/**
 * Provides an easy way for child classes to add additional buttons before the ButtonBar's button states are updated
 */
function SetupExtraButtons( UTUIButtonBar ButtonBar )
{
	Super.SetupExtraButtons(ButtonBar);

	if ( ButtonBar != None )
	{
		AddFavoriteIdx = ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.AddToFavorite>", OnButtonBar_AddFavorite);
	}
}

/**
 * Updates the enabled state of certain button bar buttons depending on whether a server is selected or not.
 */
function UpdateButtonStates()
{
	local UTUIButtonBar ButtonBar;
	local UITabControl TabControlOwner;
	local bool bValidServerSelected;

	Super.UpdateButtonStates();

	TabControlOwner = GetOwnerTabControl();
	if ( TabControlOwner != None && TabControlOwner.ActivePage == Self )
	{
		ButtonBar = GetButtonBar();
		if ( ButtonBar != None )
		{
			bValidServerSelected = ServerList != None && ServerList.GetCurrentItem() != INDEX_NONE;
			if ( AddFavoriteIdx != INDEX_NONE )
			{
				ButtonBar.Buttons[AddFavoriteIdx].SetEnabled(bValidServerSelected && !HasSelectedServerInFavorites(GetBestControllerId()));
			}
		}
	}
}

/** ButtonBar - Add to favorite */
function bool OnButtonBar_AddFavorite(UIScreenObject InButton, int InPlayerIndex)
{
	if ( InButton != None && InButton.IsEnabled(InPlayerIndex) )
	{
		AddToFavorites(InPlayerIndex);
	}
	return true;
}

/**
 * Adds the selected server to the list of favorites
 */
function AddToFavorites( int inPlayerIndex )
{
	local int CurrentSelection, ControllerId;
	local OnlineGameSearchResult SelectedGame;
	local UTDataStore_GameSearchFavorites FavsDataStore;
	local UITabControl TabControlOwner;

	CurrentSelection = ServerList.GetCurrentItem();

	if ( SearchDataStore.GetSearchResultFromIndex(CurrentSelection, SelectedGame) )
	{
		ControllerId = GetBestControllerId();
		FavsDataStore = GetFavoritesDataStore();

		// if this server isn't already in the list of favorites
		if ( FavsDataStore != None && !HasServerInFavorites(ControllerId, SelectedGame.GameSettings.OwningPlayerId) )
		{
			// add it
			if ( FavsDataStore.AddServer(ControllerId, SelectedGame.GameSettings.OwningPlayerId) )
			{
				TabControlOwner = GetOwnerTabControl();
				if ( TabControlOwner != None && TabControlOwner.ActivePage == Self )
				{
					OnAddToFavorite();
				}

				UpdateButtonStates();
			}
		}
	}
}

DefaultProperties
{
	SearchDSName=UTGameHistory
	AddFavoriteIdx=INDEX_NONE
}
