/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Base class for frontend scenes, looks for buttonbar and tab control references.
 */
class UTUIFrontEnd extends UTUIScene
	config(Game)
	native(UIFrontEnd)
	dependson(UTUIScene_MessageBox);

cpptext
{
	virtual void Tick( FLOAT DeltaTime );
	virtual void Render_Scene( FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup );

	/** Reads the encrypted password and decrypts the data */
	virtual FString GetPassword();
}


/** Pointer to the button bar for this scene. */
var transient UTUIButtonBar	ButtonBar;

/** Pointer to the tab control for this scene. */
var transient UTUITabControl TabControl;
var transient int PreviousPageIndex;
var transient int CurrentPageIndex;

/** Current mod import state. */
enum EModImport
{
	MODIMPORT_Inactive,
	MODIMPORT_Started,
	MODIMPORT_Unpacking,
	MODIMPORT_Finished,
	MODIMPORT_Failed,
	MODIMPORT_PackageName,
};

var transient EModImport			ImportState;
var transient UTUIScene_MessageBox	ImportingMessageBoxReference;	/** Messagebox for the importing state. */

/** Whether or not we are currently installing on the PS3. */
var transient bool bInstallingPS3;

/** Whether or not we are currently cancelling the ps3 install on the PS3. */
var transient bool bCancellingInstallPS3;


/** Markup for the title for this scene. */
var() string	TitleMarkupString;

var localized string	VersionText;
var vector2D			VersionPos;
var font				VersionFont;

/** Post initialize callback. */
event PostInitialize()
{
	Super.PostInitialize();

	// Store a reference to the button bar and tab control.
	ButtonBar = UTUIButtonBar(FindChild('pnlButtonBar', true));
	TabControl = UTUITabControl(FindChild('pnlTabControl', true));

	if(TabControl != none)
	{
		TabControl.OnPageActivated = OnPageActivated;
	}

	// Setup initial button bar
	SetupButtonBar();

	// Sets up the title for the scene
	SetTitle();
}

/** Scene activated event, sets up the title for the scene. */
event SceneActivated(bool bInitialActivation)
{
	Super.SceneActivated(bInitialActivation);

	SetTitle();
}

/** @return	Returns the title label that is located on the background scene. */
function UILabel GetTitleLabel()
{
	local UIScene BackgroundScene;
	local UILabel TitleLabel;
	local GameUISceneClient GameSceneClient;

	TitleLabel = None;
	GameSceneClient = GetSceneClient();
	if ( GameSceneClient != None )
	{
		BackgroundScene = GameSceneClient.FindSceneByTag('Background');
		if ( BackgroundScene != None )
		{
			TitleLabel = UILabel(BackgroundScene.FindChild('lblTitle', true));
		}
	}

	return TitleLabel;
}

/** Sets the title for this scene. */
function SetTitle()
{
	local string FinalStr;
	local UILabel TitleLabel;

	TitleLabel = GetTitleLabel();
	if ( TitleLabel != None )
	{
		if(TabControl == None)
		{
			FinalStr = Caps(Localize("Titles", string(SceneTag), "UTGameUI"));
			TitleLabel.SetDataStoreBinding(FinalStr);
		}
		else
		{
			TitleLabel.SetDataStoreBinding("");
		}
	}
}

/** Function that sets up a buttonbar for this scene, automatically routes the call to the currently selected tab of the scene as well. */
function SetupButtonBar()
{
	if(ButtonBar != None)
	{
		ButtonBar.Clear();
		if ( TabControl != None && UTTabPage(TabControl.ActivePage) != None )
		{
			// Let the current tab page try to setup the button bar
			UTTabPage(TabControl.ActivePage).SetupButtonBar(ButtonBar);
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
	local int PageDiff;
	local UITabPage PreviousActivePage;

	// Anytime the tab page is changed, update the buttonbar.
	PreviousPageIndex = CurrentPageIndex;
	CurrentPageIndex = Sender.FindPageIndexByPageRef(NewlyActivePage);
	PreviousActivePage = Sender.GetPageAtIndex(PreviousPageIndex);

	// Start hide animations for previous page
	if( PreviousPageIndex != INDEX_NONE )
	{
		if ( TabControl.ContainsChild(PreviousActivePage, true) )
		{
			// Block input
			GetUTInteraction().BlockUIInput(true);

			NewlyActivePage.SetVisibility(false);

			PreviousActivePage.SetVisibility(true);
			PreviousActivePage.Add_UIAnimTrackCompletedHandler(OnTabPage_Hide_UIAnimEnd);

			PageDiff = CurrentPageIndex-PreviousPageIndex;

			if(Abs(PageDiff)>1)
			{
				PageDiff = -PageDiff;
			}

			if(PageDiff > 0)
			{
				PreviousActivePage.PlayUIAnimation('TabPageExitLeft');
			}
			else
			{
				PreviousActivePage.PlayUIAnimation('TabPageExitRight');
			}

			ButtonBar.PlayUIAnimation('ButtonBarHide');
		}
		else
		{
			OnTabPage_Hide_UIAnimEnd(PreviousActivePage, '', 0);
		}

		PlayUISound('RotateTabPage');
	}
	else
	{
		SetupButtonBar();
	}
}

/** Called when a tab page has finished hiding. */
function OnTabPage_Hide_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	local int PageDiff;
	local UITabPage NewlyActivePage;

	if ( TrackTypeMask == 0 )
	{
		AnimTarget.Remove_UIAnimTrackCompletedHandler(OnTabPage_Hide_UIAnimEnd);
		AnimTarget.SetVisibility(false);

		SetupButtonBar();

		// Start show animations for the new tab page
		ButtonBar.PlayUIAnimation('ButtonBarShow');

		NewlyActivePage = TabControl.GetPageAtIndex(CurrentPageIndex);
		if ( NewlyActivePage == None )
		{
			NewlyActivePage = TabControl.ActivePage;
		}

		if(NewlyActivePage != None)
		{
			NewlyActivePage.SetVisibility(true);
			NewlyActivePage.SetFocus(none);
			NewlyActivePage.Add_UIAnimTrackCompletedHandler(OnTabPage_Show_UIAnimEnd);
			PageDiff = CurrentPageIndex-PreviousPageIndex;

			if(Abs(PageDiff)>1)
			{
				PageDiff = -PageDiff;
			}

			CurrentPageIndex = TabControl.FindPageIndexByPageRef(NewlyActivePage);
			if(PageDiff > 0)
			{
				NewlyActivePage.PlayUIAnimation('TabPageEnterRight');
			}
			else
			{
				NewlyActivePage.PlayUIAnimation('TabPageEnterLeft');
			}
		}
	}
}

/** Called when a tab page has finished showing. */
function OnTabPage_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	if ( TrackTypeMask == 0 )
	{
		AnimTarget.Remove_UIAnimTrackCompletedHandler(OnTabPage_Show_UIAnimEnd);
		GetUTInteraction().BlockUIInput(false);
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
	return false;
}

/**
 * Starts the show animation for the scene.
 *
 * @param	bInitialActivation	TRUE if the scene is being opened; FALSE if the another scene was closed causing this one to become the
 *								topmost scene.
 * @param	bBypassAnimation	TRUE to force all animations to their last frame, effectively bypassing animations.  This can
 *								be necessary for e.g. scenes which start out off-screen or something.
 *
 *
 * @return TRUE if there's animation for this scene, FALSE otherwise.
 */
function bool BeginShowAnimation(bool bInitialActivation=true, bool bBypassAnimation=false)
{
	local bool bResult;
	local UIObject MainRegion;
	local UILabel TitleLabel;
	local float InitialAnimProgress;

	// if animations should be bypassed, start them all at 1.0
	InitialAnimProgress = bBypassAnimation ? 1.0 : 0.0;
	if(TabControl != none)
	{
		MainRegion = TabControl;
	}
	else
	{
		MainRegion = FindChild('pnlSafeRegion', true);
	}

	if(MainRegion != none)
	{
		MainRegion.Add_UIAnimTrackCompletedHandler(OnMainRegion_Show_UIAnimEnd);
		GetUTInteraction().BlockUIInput(true);

		if ( bInitialActivation )
		{
			MainRegion.PlayUIAnimation('SceneShowInitial',,,,InitialAnimProgress);
		}
		else
		{
			MainRegion.PlayUIAnimation('SceneShowRepeat',,,,InitialAnimProgress);
		}

		bResult = true;
	}

	TitleLabel = GetTitleLabel();
	if ( TitleLabel != None )
	{
		TitleLabel.StopUIAnimation('TitleLabelHide');
		TitleLabel.PlayUIAnimation('TitleLabelShow',,,,InitialAnimProgress);
	}

	if(ButtonBar != None)
	{
		ButtonBar.PlayUIAnimation('ButtonBarShow',,,,InitialAnimProgress);
		bResult = true;
	}

	return bResult;
}

/**
 * Starts the exit animation for the scene.
 *
 * @return TRUE if there's animation for this scene, FALSE otherwise.
 */
function bool BeginHideAnimation(bool bClosingScene=false)
{
	local bool bResult;
	local UIObject MainRegion;
	local UILabel TitleLabel;

	bResult = false;


	// Main Scene Region
	if(TabControl != none)
	{
		MainRegion = TabControl;
	}
	else
	{
		MainRegion = FindChild('pnlSafeRegion', true);
	}

	if(MainRegion != none)
	{
		if(bClosingScene)
		{
			MainRegion.PlayUIAnimation('SceneHideClosing');
		}
		else
		{
			MainRegion.PlayUIAnimation('SceneHide');
			PlayUISound('MenuSlide');
		}
		bResult = true;
	}

	// Title Label
	TitleLabel = GetTitleLabel();
	if ( TitleLabel != None )
	{
		TitleLabel.StopUIAnimation('TitleLabelShow');
		TitleLabel.PlayUIAnimation('TitleLabelHide');
	}

	// Button Bar
	if(ButtonBar != None)
	{
		ButtonBar.PlayUIAnimation('ButtonBarHide');
		bResult = true;
	}

	return bResult;
}

/** Called when a tab page has finished showing. */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	if ( TrackTypeMask == 0 )
	{
		AnimTarget.Remove_UIAnimTrackCompletedHandler(OnMainRegion_Show_UIAnimEnd);
		GetUTInteraction().BlockUIInput(false);
	}
}

/** Checks to see if a frontend error message was set by the game before returning to the main menu, if so, we skip to the main menu and display the message. */
function CheckForFrontEndError()
{
	local string ErrorTitle;
	local string ErrorMessage;
	local string ErrorDisplay;

	local UTPlayerController UTPC;
	local UTUIScene_ConnectionStatus ConnectionFailedScene;

	if(GetDataStoreStringValue("<Registry:FrontEndError_Title>", ErrorTitle) &&
		GetDataStoreStringValue("<Registry:FrontEndError_Message>", ErrorMessage) &&
		GetDataStoreStringValue("<Registry:FrontEndError_Display>", ErrorDisplay))
	{
		if(ErrorDisplay=="1")
		{
			UTPC = GetUTPlayerOwner();
			if ( UTPC != None )
			{
				ConnectionFailedScene = UTPC.OpenProgressMessageScene();
				if ( ConnectionFailedScene != None )
				{
					ConnectionFailedScene.DisplayAcceptBox(ErrorMessage, ErrorTitle);
				}
			}

			if ( ConnectionFailedScene == None )
			{
				DisplayMessageBox(ErrorMessage, ErrorTitle);
			}
		}
	}

	// Clear the display flag.
	SetDataStoreStringValue("<Registry:FrontEndError_Display>", "0");
}

/** Callback for when the login required box has finished displaying. */
function OnLoginError_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	if(SelectedItem==0 && IsConsole(CONSOLE_PS3) )
	{
		ShowLoginUI(false);
	}
}

/** Starts the mod importing process. */
native function BeginImportMod();

/** Function to start importing a mod. */
function ImportMod()
{
	ImportingMessageBoxReference = GetMessageBoxScene();
	ImportingMessageBoxReference.DisplayModalBox("<Strings:UTGameUI.MessageBox.ImportingMod_Message>");

	BeginImportMod();
}

/** Callback for when the import has finished for a mod. */
function OnImportModFinished();

/**
 * Updates the current state of the mod import.
 *
 * @param NewState	New import state
 */
event UpdateModState(EModImport NewState)
{
	if(ImportState != MODIMPORT_Finished)
	{
		if(NewState != ImportState)
		{
			`Log("UTUIFrontEnd::UpdateModState() - Import State Changed!  NewState:"@NewState@"OldState:"@ImportState);
		}

		if(NewState==MODIMPORT_Finished)
		{
			ImportingMessageBoxReference.Close();
			ImportingMessageBoxReference = None;

			OnImportModFinished();
		}
		else if(ImportState==MODIMPORT_Started && NewState==MODIMPORT_Unpacking)
		{
			ImportingMessageBoxReference.DisplayModalBox("<Strings:UTGameUI.MessageBox.UnpackingMod_Message>");
		}
		else if(NewState==MODIMPORT_Failed)
		{
			// Display a message indicating that mod import failed.
			ImportingMessageBoxReference = GetMessageBoxScene();
			ImportingMessageBoxReference.DisplayAcceptBox("<Strings:UTGameUI.MessageBox.ModImportFailed_Message>");
			OnImportModFinished();
		}
		else if (NewState == MODIMPORT_PackageName)
		{
			ImportingMessageBoxReference = GetMessageBoxScene();
			ImportingMessageBoxReference.DisplayAcceptBox("<Strings:UTGameUI.MessageBox.ModImportPackageName_Message>");
			OnImportModFinished();
		}

		ImportState = NewState;
	}
}

/** Installs the game on PS3. */
function InstallPS3()
{
	bCancellingInstallPS3=false;
	ImportingMessageBoxReference = GetMessageBoxScene();
	ImportingMessageBoxReference.DisplayCancelBox("<Strings:UTGameUI.MessageBox.InstallPS3Start_Message>", "", OnCancelInstallPS3);

	BeginInstallPS3();
}

/** Starts the PS3 install process. */
native function BeginInstallPS3();

/** Cancels the PS3 install. */
native function CancelInstallPS3();

/** Confirmation for the cancel dialog. */
function OnCancelInstallPS3(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	if(bInstallingPS3)
	{
		bCancellingInstallPS3=true;
		ImportingMessageBoxReference = GetMessageBoxScene();
		ImportingMessageBoxReference.PotentialOptions.length=0;
		ImportingMessageBoxReference.PotentialOptionKeyMappings.length=0;
		ImportingMessageBoxReference.ButtonBar.Clear();
		ImportingMessageBoxReference.DisplayModalBox(Localize("MessageBox", "InstallPS3Cancelling_Message", "UTGameUI"));
		CancelInstallPS3();
	}
}

/**
 * Updates the current install state for the PS3 Install.
 *
 * @param CurrentFileNum		Current file number installing.
 * @param TotalFileNum			Total number of files to install.
 * @param bFinishedCanceling	TRUE if the install was canceled, and now it's done
 * @param bHasError				TRUE if an error occurred during the install process.
 */
event UpdatePS3InstallState(int CurrentFileNum, int TotalFileNum, bool bFinishedCancelling, bool bHasError)
{
	local string FinalString;

	if(bInstallingPS3)
	{
		if(bHasError)
		{
			bInstallingPS3=false;
			ImportingMessageBoxReference = GetMessageBoxScene();
			ImportingMessageBoxReference.DisplayAcceptBox("<Strings:UTGameUI.MessageBox.InstallingFailed_Message>");
		}
		else if(CurrentFileNum==TotalFileNum && TotalFileNum > 0 && !bCancellingInstallPS3)
		{
			bInstallingPS3=false;
			ImportingMessageBoxReference.OnSelection=None;
			ImportingMessageBoxReference.Close();
			ImportingMessageBoxReference.OnClosed = OnInstallMessageClosed;
		}
		else if(bFinishedCancelling)
		{
			ImportingMessageBoxReference.Close();
			ImportingMessageBoxReference.OnClosed = OnCancelMessageClosed;
			bInstallingPS3=false;
		}
		else if(TotalFileNum > 0 && !bCancellingInstallPS3)
		{
			if(ImportingMessageBoxReference.bFullyVisible)
			{
				FinalString = Localize("MessageBox", "InstallPS3Progress_Message", "UTGameUI");
				FinalString = Repl(FinalString, "\`CurrentFile\`", string(CurrentFileNum));
				FinalString = Repl(FinalString, "\`TotalFiles\`", string(TotalFileNum));

				ImportingMessageBoxReference.DisplayModalBox(FinalString);
			}
		}
	}
}

/** Callback for the install PS3 messagebox has closed. */
function OnInstallMessageClosed()
{
	ImportingMessageBoxReference.OnClosed = None;
	ImportingMessageBoxReference = None;
	DisplayMessageBox("<Strings:UTGameUI.MessageBox.InstallPS3Finished_Message>","<Strings:UTGameUI.MessageBox.InstallPS3Finished_Title>");
}

/** Callback for the install PS3 messagebox has closed. */
function OnCancelMessageClosed()
{
	ImportingMessageBoxReference.OnClosed = None;
	ImportingMessageBoxReference = None;
}

/** Encrypts the password and saves the data */
native function SavePassword(string Password);


/**
 * Called when a new scene is opened over this one.  Propagates the values for bRequiresNetwork and bRequiresOnlineService to the new page.
 */
function ChildSceneOpened( UIScene NewTopScene )
{
	local UTUIFrontEnd FrontEndScene;

	FrontEndScene = UTUIFrontEnd(NewTopScene);
	if ( FrontEndScene != None )
	{
		if ( bRequiresNetwork )
		{
			FrontEndScene.bRequiresNetwork = true;
		}

		if ( bRequiresOnlineService )
		{
			FrontEndScene.bRequiresOnlineService = true;
		}
	}
}

defaultproperties
{
	bPauseGameWhileActive=false
	PreviousPageIndex=INDEX_NONE;
	CurrentPageIndex=INDEX_NONE;
	SceneRenderMode=SPLITRENDER_Fullscreen

	// Setup handler for input keys - do it in defaultproperties so that it doesn't get serialized in the editor
	OnRawInputKey=HandleInputKey
	OnTopSceneChanged=ChildSceneOpened
	VersionPos=(X=0.0090,Y=0.0570)
}
