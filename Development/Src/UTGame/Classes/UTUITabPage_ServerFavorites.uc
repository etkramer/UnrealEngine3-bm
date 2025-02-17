/**
 * This class is a specialized server browser which displays only those servers which the player has marked as a favorite.
 * This server browser does not respect filter options.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUITabPage_ServerFavorites extends UTUITabPage_ServerBrowser;

var	transient	int		RemoveFavoriteIdx;

/**
 * Sets the correct tab button caption.
 */
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.JoinGame.Favorites>");
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
	return UTDataStore_GameSearchFavorites(SearchDataStore);
}

/**
 * Provides an easy way for child classes to add additional buttons before the ButtonBar's button states are updated
 */
function SetupExtraButtons( UTUIButtonBar ButtonBar )
{
	Super.SetupExtraButtons(ButtonBar);

	if ( ButtonBar != None )
	{
		RemoveFavoriteIdx = ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.RemoveFromFavorite>", OnButtonBar_RemoveFavorite);
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
	if ( RemoveFavoriteIdx != INDEX_NONE && TabControlOwner != None && TabControlOwner.ActivePage == Self )
	{
		ButtonBar = GetButtonBar();
		if ( ButtonBar != None )
		{
			bValidServerSelected = ServerList != None && ServerList.GetCurrentItem() != INDEX_NONE;
			if ( RemoveFavoriteIdx != InDEX_NONE )
			{
				ButtonBar.Buttons[RemoveFavoriteIdx].SetEnabled(bValidServerSelected && HasSelectedServerInFavorites(GetBestControllerId()));
			}
		}
	}
}

/** ButtonBar - Remove from favorite */
function bool OnButtonBar_RemoveFavorite(UIScreenObject InButton, int InPlayerIndex)
{
	if ( InButton != None && InButton.IsEnabled(InPlayerIndex) )
	{
		RemoveFavorite(InPlayerIndex);
	}
	return true;
}

/**
 * Removes the currently selected server from the list of favorites
 */
function RemoveFavorite( int inPlayerIndex )
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

		// if this server is in the list of favorites
		if ( FavsDataStore != None && HasServerInFavorites(ControllerId, SelectedGame.GameSettings.OwningPlayerId) )
		{
			// remove it
			if ( FavsDataStore.RemoveServer(ControllerId, SelectedGame.GameSettings.OwningPlayerId) )
			{
				TabControlOwner = GetOwnerTabControl();
				if ( TabControlOwner != None && TabControlOwner.ActivePage == Self )
				{
					RefreshServerList(inPlayerIndex);
				}
			}
		}
	}
}

DefaultProperties
{
	SearchDSName=UTGameFavorites
	RemoveFavoriteIdx=INDEX_NONE
}
