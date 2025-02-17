/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for finding a quick match.
 */

class UTUITabPage_FindQuickMatch extends UTUITabPage_ServerBrowser
	placeable;

`include(UTOnlineConstants.uci)

const QUICKMATCH_MAX_RESULTS = 30;

/** Delegate for when a search has completed. */
delegate OnSearchComplete(bool bWasSuccessful);

/** PostInitialize event - Sets delegates for the page. */
event PostInitialize( )
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.JoinGame.Servers>");
}

/** Sets buttons for the scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.CancelSearch>", OnButtonBar_Back);
}

/** Refreshes the server list. */
function RefreshServerList(int InPlayerIndex, optional int MaxResults=1000)
{
	local OnlineGameSearch GameSearch;
	GameSearch = SearchDataStore.GetCurrentGameSearch();
	
	// Force no full, empty, or locked servers to be returned.
	GameSearch.SetStringSettingValue(CONTEXT_EMPTYSERVER, CONTEXT_EMPTYSERVER_NO, false);
	GameSearch.SetStringSettingValue(CONTEXT_FULLSERVER, CONTEXT_FULLSERVER_NO, false);
	GameSearch.SetStringSettingValue(CONTEXT_LOCKEDSERVER, CONTEXT_LOCKEDSERVER_NO, false);

	Super.RefreshServerList(InPlayerIndex, QUICKMATCH_MAX_RESULTS);
}


/**
 * Delegate fired when the search for an online game has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnFindOnlineGamesComplete(bool bWasSuccessful)
{
//	local int SearchIdx;
	local OnlineGameSearchResult GameToJoin;

	// Join the game if we found one, otherwise pop up a error message.
	if(bWasSuccessful && SearchDataStore.GetSearchResultFromIndex(0,GameToJoin)==true && ServerList.Items.length>0)
	{

		// Only finish the search when we do not have any outstanding queries.
		if(SearchDataStore.HasOutstandingQueries()==false)
		{
/*
			`Log("Query Count:"@ServerList.Items.length);
			for(SearchIdx=0; SearchIdx<ServerList.Items.length; SearchIdx++)
			{
				if(SearchDataStore.GetSearchResultFromIndex(SearchIdx,GameToJoin))
				{
					`Log("Ping ("$SearchIdx$"): "$GameToJoin.GameSettings.PingInMs);
				}
			}
*/
			OnSearchComplete(true);
		}
	}
	else
	{
		OnSearchComplete(false);
	}
}

/** Callback for when the server list value changed. */
function OnServerList_ValueChanged(UIObject Sender, int PlayerIndex)
{
	local int CurrentIndex;
//	local int SearchIdx;
	local OnlineGameSearchResult GameToJoin;

	CurrentIndex=ServerList.GetCurrentItem();

	if(CurrentIndex != INDEX_NONE && SearchDataStore.GetSearchResultFromIndex(0,GameToJoin)==true && ServerList.Items.length>0)
	{
		// Only finish the search when we do not have any outstanding queries.
		if(SearchDataStore.HasOutstandingQueries()==false)
		{
/*
			`Log("Query Count:"@ServerList.Items.length);
			for(SearchIdx=0; SearchIdx<ServerList.Items.length; SearchIdx++)
			{
				if(SearchDataStore.GetSearchResultFromIndex(SearchIdx,GameToJoin))
				{
					`Log("Ping ("$SearchIdx$"): "$GameToJoin.GameSettings.PingInMs);
				}
			}
*/
			OnSearchComplete(true);
		}
	}
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

	if(IsVisible() && EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
	}

	return bResult;
}

