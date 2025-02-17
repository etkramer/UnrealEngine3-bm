/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * News/DLC scene for UT3.
 */
class UTUIFrontEnd_News extends UTUIFrontEnd;

/** References to tab pages. */
var transient UTUITabPage_News NewsTab;
var transient UTUITabPage_EpicContent EpicContentTab;
var transient UTUITabPage_MyContent MyContentTab;


/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	local int PlayerIndex, ControllerId;

	Super.PostInitialize();

	PlayerIndex = GetPlayerIndex();
	ControllerId = GetPlayerControllerId(PlayerIndex);

	// Add tab pages to tab control
	NewsTab = UTUITabPage_News(FindChild('pnlNews', true));
	if ( NewsTab != none && IsLoggedIn(ControllerId, true) )
	{
		TabControl.InsertPage(NewsTab, 0, 0, false);
	}

	EpicContentTab = UTUITabPage_EpicContent(FindChild('pnlEpicContent', true));
	if ( EpicContentTab != none && IsLoggedIn(ControllerId, true) )
	{
		TabControl.InsertPage(EpicContentTab, 0, 1, false);
	}

	MyContentTab = UTUITabPage_MyContent(FindChild('pnlMyContent', true));
	if ( MyContentTab != none )
	{
		TabControl.InsertPage(MyContentTab, 0, 2, false);
	}
}

/** Called when a tab page has finished showing. */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	local int PlayerIndex, ControllerId;

	Super.OnMainRegion_Show_UIAnimEnd(AnimTarget, AnimName, TrackTypeMask);

	if ( TrackTypeMask == 0 )
	{
		PlayerIndex = GetPlayerIndex();
		ControllerId = GetPlayerControllerId(PlayerIndex);

		// Disable content tabs for now
		if ( IsGame() )
		{
			// @todo: Enable when we show the epic content tab on PC.
			EpicContentTab.StopUIAnimation('TabPageExitLeft', , true);
			EpicContentTab.StopUIAnimation('TabPageExitRight', , true);
			TabControl.RemovePage(EpicContentTab,PlayerIndex);

			// in case the tab pages were added in the editor, attempt to remove them again if we're not logged in
			if ( !IsLoggedIn(ControllerId, true) )
			{
				NewsTab.StopUIAnimation('TabPageExitLeft', , true);
				NewsTab.StopUIAnimation('TabPageExitRight', , true);
				TabControl.RemovePage(NewsTab, PlayerIndex);
			}

			if ( !IsConsole(CONSOLE_PS3) )	// Only show content tabs on ps3 for now.
			{
				MyContentTab.StopUIAnimation('TabPageExitLeft', , true);
				MyContentTab.StopUIAnimation('TabPageExitRight', , true);
				TabControl.RemovePage(MyContentTab,PlayerIndex);

				// @todo: Enable when we show the epic content tab on PC.
				//EpicContentTab.ReadContent();
			}
		}
	}
}

/**
 * Notification that the player's connection to the platform's online service is changed.
 */
function NotifyOnlineServiceStatusChanged( EOnlineServerConnectionStatus NewConnectionStatus )
{
	local int PlayerIndex;

	// if bRequiresOnlineService is true, this page will be closed by UIScene.NotifyOnlineServiceStatusChanged anyway
	if ( TabControl != None && NewConnectionStatus != OSCS_Connected && !bRequiresOnlineService )
	{
		PlayerIndex = GetPlayerIndex();

		if ( NewsTab != None && TabControl.ContainsChild(NewsTab,true) )
		{
			NewsTab.StopUIAnimation('TabPageExitLeft', , true);
			NewsTab.StopUIAnimation('TabPageExitRight', , true);
			TabControl.RemovePage(NewsTab, PlayerIndex);
		}

		if ( EpicContentTab != None && TabControl.ContainsChild(MyContentTab, true) )
		{
			MyContentTab.StopUIAnimation('TabPageExitLeft', , true);
			MyContentTab.StopUIAnimation('TabPageExitRight', , true);
			TabControl.RemovePage(EpicContentTab, PlayerIndex);
		}
	}

	Super.NotifyOnlineServiceStatusChanged(NewConnectionStatus);
}

/**
 * Called when the status of the platform's network connection changes.
 */
function NotifyLinkStatusChanged( bool bConnected )
{
	local int PlayerIndex;

	// if !bConnected and bRequiresNetwork == true, then this page will be closed by UIScene.NotifyLinkStatusChanged anyway
	if ( TabControl != None && !bConnected && !bRequiresNetwork )
	{
		PlayerIndex = GetPlayerIndex();

		if ( NewsTab != None && TabControl.ContainsChild(NewsTab,true) )
		{
			NewsTab.StopUIAnimation('TabPageExitLeft', , true);
			NewsTab.StopUIAnimation('TabPageExitRight', , true);
			TabControl.RemovePage(NewsTab, PlayerIndex);
		}

		if ( EpicContentTab != None && TabControl.ContainsChild(MyContentTab, true) )
		{
			MyContentTab.StopUIAnimation('TabPageExitLeft', , true);
			MyContentTab.StopUIAnimation('TabPageExitRight', , true);
			TabControl.RemovePage(EpicContentTab, PlayerIndex);
		}
	}

	Super.NotifyLinkStatusChanged(bConnected);
}

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	if(ButtonBar != None)
	{
		ButtonBar.Clear();
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);

		if( IsConsole(CONSOLE_PS3) && UTTabPage(TabControl.ActivePage).Class == class'UTUITabPage_MyContent')
		{
			ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ImportContent>", OnButtonBar_ImportContent);
		}
		UTTabPage(TabControl.ActivePage).SetupButtonBar(ButtonBar);
	}
}

/** Callback for when the user wants to back out of this screen. */
function OnBack()
{
	CloseScene(self);
}

/** Callback for when the user wants to import content from a memory stick. */
function OnImportContent()
{
	ImportMod();
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_ImportContent(UIScreenObject InButton, int InPlayerIndex)
{
	OnImportContent();

	return true;
}

/** Callback for when the import has finished for a mod. */
function OnImportModFinished()
{
	// Tell the my content tab to refresh the content list.
	MyContentTab.OnContentListChanged();
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

	bResult = UTTabPage(TabControl.ActivePage).HandleInputKey(EventParms);

	if(bResult==false && EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_X')
		{
			if ( IsConsole(CONSOLE_PS3) && UTTabPage(TabControl.ActivePage).Class == class'UTUITabPage_MyContent' )
			{
				OnImportContent();
				bResult=true;
			}
		}
	}

	return bResult;
}


defaultproperties
{

}
