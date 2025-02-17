/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Allows the user to pick a bunch of filters and then jump straight into a game.
 */
class UTUIFrontEnd_QuickMatch extends UTUIFrontEnd;

/** Tab page references for this scene. */
var transient UTUITabPage_ServerFilter ServerFilterTab;
var transient UTUITabPage_FindQuickMatch FindQuickMatchTab;

/** Reference to the searching messagebox. */
var transient UTUIScene_MessageBox SearchingMessageBox;


/** Whether or not we are currently searching. */
var bool bSearching;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize()
{
	Super.PostInitialize();

	// Grab a reference to the server filter tab.
	ServerFilterTab = UTUITabPage_ServerFilter(FindChild('pnlServerFilter', true));
	ServerFilterTab.OnAcceptOptions = OnServerFilter_AcceptOptions;

	FindQuickMatchTab = UTUITabPage_FindQuickMatch(FindChild('pnlFindQuickMatch', true));
	FindQuickMatchTab.OnBack = OnBack;
	FindQuickMatchTab.OnSearchComplete=OnGameSearchCompleted;
	FindQuickMatchTab.SetVisibility(false);

	// Let the currently active page setup the button bar.
	SetupButtonBar();
}

/** Callback for when the scene is activated. */
event SceneActivated(bool bInitialActivation)
{
	if ( !bInitialActivation )
	{
		ServerFilterTab.OptionList.SetFocus(none);
	}
}

/** Called when a tab page has finished showing. */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	local name MatchTypeOptionName;
	local UIObject MatchTypeOption;

	Super.OnMainRegion_Show_UIAnimEnd(AnimTarget, AnimName, TrackTypeMask);

	if ( AnimName == 'SceneShowInitial' && IsConsole() )
	{
		// if we're on console, then regenerating the option list won't trigger an OnValueChanged event like it
		// would on PC, so we need to do this manually in order to update the enabled state of the gametype option
		MatchTypeOptionName = IsConsole(CONSOLE_XBox360) ? 'MatchType360' : 'MatchType';
		MatchTypeOption = ServerFilterTab.FindChild(MatchTypeOptionName, true);
		if ( MatchTypeOption != None )
		{
			ServerFilterTab.OnOptionList_OptionChanged(MatchTypeOption, MatchTypeOptionName, GetPlayerIndex());
		}
	}
}

/** Setup the button bar for this scene. */
function SetupButtonBar()
{
	if(bSearching)
	{
		FindQuickMatchTab.SetupButtonBar(ButtonBar);
	}
	else
	{
		ButtonBar.Clear();
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.FindMatch>", OnButtonBar_FindMatch);
	}
}

/** Starts searching for a game. */
function StartSearch()
{
	if(bSearching==false)
	{
		bSearching = true;

		SearchingMessageBox = GetMessageBoxScene();
		SearchingMessageBox.DisplayCancelBox("<Strings:UTGameUI.Generic.Searching>", "", OnCancelSearch);
		FindQuickMatchTab.RefreshServerList(GetPlayerIndex());
	}
}

/** Callback for the user cancelling the search. */
function OnCancelSearch(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	if(bSearching)
	{
		bSearching = false;
		SearchingMessageBox=None;
	}
}

/** Callback for when the g */
function OnGameSearchCompleted(bool bSuccessful)
{
	if(bSearching)
	{
		bSearching=false;

		if(bSuccessful)
		{
			SearchingMessageBox.OnClosed=OnSearchDialogClosed_Success;
			SearchingMessageBox.Close();
		}
		else
		{
			SearchingMessageBox.OnClosed=OnSearchDialogClosed_Failed;
			SearchingMessageBox.Close();
		}
	}
}

/** Callback for when the search dialog has been closed after a successful search and we need to join a game. */
function OnSearchDialogClosed_Success()
{
	SearchingMessageBox.OnClosed=None;
	SearchingMessageBox=None;

	// Always join the first result.
	FindQuickMatchTab.JoinServer();
}

/** Callback for when the search dialog has been closed after a failed search and we need to display an error. */
function OnSearchDialogClosed_Failed()
{
	SearchingMessageBox.OnClosed=None;
	SearchingMessageBox=None;

	DisplayMessageBox("<Strings:UTGameUI.Errors.QuickMatchFailed_Message>","<Strings:UTGameUI.Errors.QuickMatchFailed_Title>");
}

/** Callback for when the user wants to go back or cancel the current search. */
function OnBack()
{
	if(bSearching==true)
	{
		bSearching=false;
		SearchingMessageBox.OnClosed=OnSearchingMessage_Closed;
		SearchingMessageBox.Close();
	}
	else if(SearchingMessageBox==None)
	{
		CloseScene(self);
	}
}

/** Callback for when the search message has closed. */
function OnSearchingMessage_Closed()
{
	SearchingMessageBox.OnClosed=None;
	SearchingMessageBox=None;
}

/** Called when the user accepts their filter settings and wants to go to the server browser. */
function OnServerFilter_AcceptOptions(UIScreenObject InObject, int PlayerIndex)
{
	StartSearch();
}

/** Buttonbar callbacks. */
function bool OnButtonBar_FindMatch(UIScreenObject InButton, int PlayerIndex)
{
	StartSearch();

	return true;
}


function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	OnBack();

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
	if(TabControl != None)
	{
		CurrentTabPage = UTTabPage(TabControl.ActivePage);
		bResult=CurrentTabPage.HandleInputKey(EventParms);
	}

	// If the tab page didn't handle it, let's handle it ourselves.
	if(bResult==false)
	{
		if(EventParms.EventType==IE_Released)
		{
			if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
			{
				OnBack();
				bResult=true;
			}
		}
	}

	return bResult;
}

defaultproperties
{
	bMenuLevelRestoresScene=true
	bRequiresNetwork=true
}
