/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_MidGameMenu extends UTUIScene_Hud
	dependson(UTUIScene_MessageBox)
	native(UI);

var transient UTUIButtonBar	ButtonBar;
var transient UTUITabControl TabControl;
var transient UILabel MapVoteClock;
var transient UIPanel LoadingPanel;
var transient UIImage LoadingRotator;
var transient bool bInitial;

var transient bool bNeedsProfileSave;
var transient bool bOkToAutoClose;

var transient UTUITabPage_InGame InGamePage;

var transient bool bLoading;
var transient bool bWaitingForReady;
var transient bool bReturningToMainMenu;

var transient UTUIScene_MessageBox MBScene;

cpptext
{
	virtual void Tick( FLOAT DeltaTime );
	virtual UBOOL PreChildrenInputKey( INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL bGamepad);
}

event SceneActivated( bool bInitialActivation )
{
	local WorldInfo WI;
	local int i,cnt;
	local UTPlayerController UTPC;

	Super.SceneActivated(bInitialActivation);

	if ( bInitialActivation )
	{
		UTPC = GetUTPlayerOwner();
		if ( UTPC != None )
		{
			// SceneActivated is called immediately after LoadSceneDataValues is called.  LoadSceneDataValues would caused
			// most of the widgets in the scene which are bound to the player profile's data store to re-initialize their value,
			// triggering a call to their ValueChanged delegate.  In the mid-game menu, the handler for this delegate sets the
			// "profile is dirty" flag in scene, but in the case of LoadSceneDataValues the profile isn't dirty so clear that
			// flag now.
			bNeedsProfileSave = false;
		}

		WI = GetWorldInfo();
		if ( WI != none && WI.GRI != none )
		{
			for (i=0;i<WI.GRI.PRIArray.Length;i++)
			{
				if ( WI.GRI.PRIArray[i] != none && !WI.GRI.PRIArray[i].bBot)
				{
					cnt++;
				}
			}

			if (!WI.GRI.bMatchIsOver && Cnt < 2)
			{
				bPauseGameWhileActive = true;
			}
		}
	}
}

event SceneDeactivated()
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;
	local UTPlayerController UTPC;

	Super.SceneDeactivated();

	UTPC = GetUTPlayerOwner();

	WI = GetWorldInfo();
    WI.ForceGarbageCollection();

	if (bNeedsProfileSave)
	{
		SaveProfile();
	}

	GRI = UTGameReplicationInfo(WI.GRI);
	if ( GRI != none )
	{
		GRI.LastUsedMidgameTab = TabControl.ActivePage.WidgetTag;
		GRI.MidGameMenuClosed();

		if (GRI.bMatchIsOver && !bReturningToMainMenu)
		{
			UTPC.ShowScoreboard();
		}
    }
}

/**
 * Opens the 'save profile' scene, which takes care of saving the player's profile.
 */
function SaveProfile()
{
	local OnlineSubsystem OnlineSub;
	local UTPlayerController UTPC;
	local UTUIScene_SaveProfile SaveProfileScene;

	UTPC = GetUTPlayerOwner();

	SaveProfileScene = UTPC.SaveProfile(GetPlayerIndex());
	if ( SaveProfileScene != None )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None && OnlineSub.PlayerInterface != None)
		{
			// Register the call back so we can shut down the scene upon completion
			OnlineSub.PlayerInterface.AddWriteProfileSettingsCompleteDelegate(GetPlayerIndex(),OnSaveProfileComplete);
		}
		SaveProfileScene.PerformSave();
	}
}

/**
 * Called when the save has completed the async operation
 *
 * @param bWasSuccessful whether the save worked ok or not
 */
function OnSaveProfileComplete(byte LocalUserNum,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local UTPlayerController UTPC;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.PlayerInterface != None)
	{
		// Register the call back so we can shut down the scene upon completion
		OnlineSub.PlayerInterface.ClearWriteProfileSettingsCompleteDelegate(GetPlayerIndex(),OnSaveProfileComplete);
	}

	UTPC = GetUTPlayerOwner();
	if ( UTPC != None )
	{
		UTPC.LoadSettingsFromProfile(false);
	}
}

/**
 * Setup the delegates for the scene and cache all of the various UI Widgets
 */
event PostInitialize( )
{
	local class<UTGame> GameClass;
	local WorldInfo WI;

	WI = GetWorldInfo();

	Super.PostInitialize();

	// Store a reference to the button bar.
	ButtonBar = UTUIButtonBar(FindChild('ButtonBar', true));
	ButtonBar.ClearButton(0);
	ButtonBar.ClearButton(1);

    LoadingPanel = UIPanel(FindChilD('LoadingPanel',true));
    LoadingRotator = UIImage(FindChild('ConnectingImage',true));

	MapVoteClock = UILabel(FindChild('MapVoteClock',true));

	// Find the tab control

	TabControl = UTUITabControl( FindChild('TabControl',true) );

	if ( TabControl != none && WI != none )
	{

		GameClass = Class<UTGame>( GetWorldInfo().GRI.GameClass);
		if ( GameClass != none && !GameClass.default.bMidGameHasMap )
		{
			TabControl.RemoveTabByTag('MapTab');
		}

		TabControl.OnPageActivated = OnPageActivated;

		if ( WI.NetMode == NM_StandALone || IsConsole() )
		{
			TabControl.RemoveTabByTag('ChatTab');
		}

		if ( WI.NetMode == NM_StandALone)
		{
			TabControl.RemoveTabByTag('FriendsTab');
			TabControl.RemoveTabByTag('MessageTab');
		}

		if ( WI.GRI.bMatchIsOver )
		{
			TabControl.RemoveTabByTag('SettingsTab');
		}
	}

	// Setup initial button bar
	SetupButtonBar();

	// Setup handler for input keys
	OnRawInputKey=HandleInputKey;
	OnInitialSceneUpdate = PreRenderCallBack;

	InGamePage = UTUITabPage_InGame(FindChild('GameTab',true));

}

function PreRenderCallBack()
{
	local UTPlayerController PC;

	PC = GetUTPlayerOwner(0);

	if (PC != none && PC.VoteRI != none )
	{
		BeginVoting(PC.VoteRI);
	}

	bCloseOnLevelChange = false;

	OnInitialSceneUpdate = none;
}


function ActivateTab(name TabTag)
{

	if (TabTag == 'ChatTab' && IsConsole())
	{
		TabTag = 'ScoreTab';
	}

	if ( TabControl != none )
	{
		if ( TabControl.ActivePage.WidgetTag != 'ChatTab' && TabControl.ActivateTabByTag(TabTag) )
		{
			return;
		}
	}
}

/** Function that sets up a buttonbar for this scene, automatically routes the call to the currently selected tab of the scene as well. */
function SetupButtonBar()
{
	local UTGameReplicationInfo GRI;
	local WorldInfo WI;
	if(ButtonBar != none)
	{

    	WI = GetWorldInfo();
    	GRI = UTGameReplicationInfo(WI.GRI);

		ButtonBar.Clear();

		// Depending on when we are, set the proper button.

		if ( WI != none )
		{
			if ( !WI.IsInSeamlessTravel() )
			{

				if ( bWaitingForReady )
				{
				    ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Ready>", ButtonBarBack);
				}
				else
				{
				    ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", ButtonBarBack);
				}

			    if ( WI.NetMode == NM_StandAlone || GRI.bStoryMode )
			    {
					ButtonBar.AppendButton("<Strings:UTGameUI.MidGameMenu.Forfeit>",ButtonBarDisconnect) ;
				}
				else
			    {
					ButtonBar.AppendButton("<Strings:UTGameUI.MidGameMenu.LeaveGame>",ButtonBarDisconnect) ;
				}

                ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ExitGame>",ButtonBarExitGame);

				if(!bWaitingForReady && TabControl != none)
				{
					// Let the current tab page try to setup the button bar
					UTTabPage(TabControl.ActivePage).SetupButtonBar(ButtonBar);
				}
			}
		}
	}
}


/**
 * Called when a new page is activated.
 *
 * @param	Sender			the tab control that activated the page
 * @param	NewlyActivePage	the page that was just activated
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this event.
 */
function OnPageActivated( UITabControl Sender, UITabPage NewlyActivePage, int PlayerIndex )
{
	// Anytime the tab page is changed, update the buttonbar.
	SetupButtonBar();
	bOkToAutoClose = false;
}

/**
 * Back was selected, exit the menu
 * @Param	InButton			The button that selected
 * @Param	InPlayerIndex		Index of the local player that made the selection
 */

function bool ButtonBarBack(UIScreenObject InButton, int InPlayerIndex)
{
	Back();
	return true;
}

function Back()
{
	SceneClient.CloseScene(self);
}

function bool ButtonBarDisconnect(UIScreenObject InButton, int InPlayerIndex)
{
	Disconnect();
	return true;

}

function bool ButtonBarExitGame(UIScreenObject InButton, int InPlayerIndex)
{
	MBScene = GetMessageBoxScene();
	if(MBScene != none)
	{
		TabControl.PlayUIAnimation('FadeOut',,,5.0);
		ButtonBar.PlayUIAnimation('FadeOut',,,5.0);
		MBScene.DisplayAcceptCancelBox("<Strings:UTGameUI.MidGameMenu.QuitGameWarning>","<Strings:UTGameUI.Campaign.Confirmation", MB_ExitSelection);
	}

	return true;

}
function MB_ExitSelection(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	TabControl.PlayUIAnimation('FadeIn',,,5.0);
	ButtonBar.PlayUIAnimation('FadeIn',,,5.0);

	if (SelectedOption == 0)
	{
		ConsoleCommand("Quit");
	}
}


function MB_Selection(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local UTPlayerController PC;
	local UISceneClient SC;

	TabControl.PlayUIAnimation('FadeIn',,,5.0);
	ButtonBar.PlayUIAnimation('FadeIn',,,5.0);

	if (SelectedOption == 0)
	{
		PC = GetUTPlayerOwner();
		SC = GetSceneClient();
		if ( SC != None )
		{
			bReturningToMainMenu = true;
			SC.CloseScene(self);
		}

		PC.QuitToMainMenu();
	}
	MBScene = none;
}


function Disconnect()
{
	MBScene = GetMessageBoxScene();
	if(MBScene != none)
	{
		TabControl.PlayUIAnimation('FadeOut',,,5.0);
		ButtonBar.PlayUIAnimation('FadeOut',,,5.0);
		MBScene.DisplayAcceptCancelBox("<Strings:UTGameUI.MidGameMenu.ExitMatchWarning>","<Strings:UTGameUI.Campaign.Confirmation", MB_Selection);
	}
}


function bool HandleInputKey( const out InputEventParameters EventParms )
{

	local UTGameUISceneClient UTSceneClient;

	if ( ButtonBar.IsHidden() )
	{
		return true;
	}

	// Don't allow console commands when in seamless travel.

	if (EventParms.EventType == IE_Released)
	{
		if (!bWaitingForReady )
		{
			if (EventParms.InputKeyName == 'XBoxtypeS_B' || EventParms.InputKeyName == 'Escape' || EventParms.InputKeyName == 'XBoxTypeS_Start')
			{
			    UTSceneClient = UTGameUISceneClient(SceneClient);
				if ( UTSceneClient != none && UTSceneClient.IsInSeamlessTravel() )
				{
					return true;
				}

				Back();
				return true;
			}
		}

		if (EventParms.InputKeyName == 'XBoxTypeS_Y')
		{
			Disconnect();
			return true;
		}
	}

	if ( UTTabPage(TabControl.ActivePage) != none )
	{
		return UTTabPage(TabControl.ActivePage).HandleInputKey( EventParms );
	}
	return false;
}

event UpdateVote(UTGameReplicationInfo GRI)
{
	local string s;
	local UTVoteReplicationInfo VoteRI;
	local UTPlayerController UTPC;

	UTPC = GetUTPlayerOwner();

	if ( UTPC != none && UTPC.VoteRI != none )
	{

		VoteRI = UTPC.VoteRI;

	    S = "<Strings:UTGameUI.MidGameMenu.VoteTimePrefix>"@GRI.MapVoteTimeRemaining;
	    S @= (GRI.MapVoteTimeRemaining > 1) ? "<Strings:UTGameUI.MidGameMenu.VoteTimeSuffixA>" : "<Strings:UTGameUI.MidGameMenu.VoteTimeSuffixA>";
	    if( (VoteRI != none) && (VoteRI.LeadingMap != "") )
	    {
	    	S @= "("$InGamePage.GetMapFriendlyName(VoteRI.LeadingMap)$")";
	    }

	 	MapVoteClock.SetDataStoreBinding(S);

		if ( MapVoteClock.IsHidden() )
		{
	        MapVoteClock.SetVisibility(true);
	    }
	}
	else if (MapVoteClock.IsVisible() )
	{
		MapVoteClock.SetVisibility(False);
	}
}

function BeginVoting(UTVoteReplicationInfo NewVoteRI)
{
	local UTUITabPAge_InGame GameTab;


	GameTab = UTUITabPAge_InGame( FindChild('GameTab',true));
//	`log("### BeginVoting"@GameTab);
	if (GameTab != none )
	{
		GameTab.BeginVoting(NewVoteRI);
	}
}

/**
 * Parse the scrollback and create a string out of it
 */
function string ParseScrollback(const out array<string> Scrollback)
{
	local int i,Start;
	local string s, Result;

	Result = "";

	Start = (Scrollback.Length < 150) ? 0 : Scrollback.Length-150;
	for (i=Start;i<Scrollback.Length;i++)
	{
		if ( (Left(Scrollback[i],7) ~= ">>> Say") || (Left(Scrollback[i],11) ~= ">>> TeamSay") )
		{
			continue;
		}
		else
		{
			s = Repl(Scrollback[i],">","");
			s = Repl(S,"<","");

			Result $= s $ " \n ";
		}
	}

	return Result;
}

/**
 * Reset will set the bWaitingForReady flag and reset the button bar
 */
function Reset()
{
	local UTUITabPage_InGame InGameTab;

	bOkToAutoClose = true;

	bWaitingForReady = true;
	InGameTab = UTUITabPage_InGame( FindChild('GameTab',true));
	InGameTab.Reset(GetWorldInfo());
	SetupButtonBar();
}

event BeginLoading()
{
	local rotator r;

	if ( MBScene != none )
	{
		MBScene.CloseScene(self);
	}

	bLoading = true;
	LoadingPanel.SetVisibility(true);
	LoadingRotator.RotateWidget(r,false);
	ButtonBar.SetVisibility(false);
}

event EndLoading()
{

	bLoading = false;
	LoadingPanel.SetVisibility(false);
	ButtonBar.SetVisibility(true);
	SetupButtonBar();
}



defaultproperties
{
	SceneInputMode=INPUTMODE_None
	SceneRenderMode=SPLITRENDER_Fullscreen
	bDisplayCursor=true
	bRenderParentScenes=false
	bAlwaysRenderScene=true
	bCloseOnLevelChange=true
	bPauseGameWhileActive=false
	bSaveSceneValuesOnClose=false
	bDisableWorldRendering=true
}
