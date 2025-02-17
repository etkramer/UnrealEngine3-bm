/**
 * Overall table of content for the War Journal.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_TOCMain extends GearUISceneWJ_TOCBase;

var	transient	GearWarJournalPage_TOCMain	TOCPanel;

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	// on the main TOC, the button bar has a slightly different tag
	btnbarMain = UICalloutButtonPanel(FindChild('btnbarMainTOC',true));
	TOCPanel = GearWarJournalPage_TOCMain(RightPage);
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	btnbarMain.SetButtonCallback('CloseWindow', CalloutButtonClicked);
}

/**
 * Wrapper for replacing this scene with the scene for the previous section of the War Journal
 */
function bool OpenPreviousChapter( int PlayerIndex )
{
	local GearUISceneWJ_PageBase WarJournalScene;
	local UIScene SceneInstance;
	local bool bResult;

	if ( PreviousChapterScene != None )
	{
		SceneInstance = OpenScene(PreviousChapterScene);
		if ( SceneInstance != None )
		{
			WarJournalScene = GearUISceneWJ_PageBase(SceneInstance);
			if ( WarJournalScene != None )
			{
				WarJournalScene.SetCurrentPage(`LAST_ITEM_INDEX);
			}

			bResult = true;
		}
	}
	else
	{
		bResult = CloseScene();
	}

	return bResult;
}

/**
 * Wrapper for replacing this scene with the scene for the next section of the War Journal
 */
function bool OpenNextChapter( int PlayerIndex )
{
	local GearUISceneWJ_PageBase WarJournalScene;
	local UIScene SceneInstance;
	local bool bResult;

	if ( NextChapterScene != None )
	{
		SceneInstance = OpenScene(NextChapterScene);
		if ( SceneInstance != None )
		{
			WarJournalScene = GearUISceneWJ_PageBase(SceneInstance);
			if ( WarJournalScene != None )
			{
				WarJournalScene.SetCurrentPage(0);
			}

			bResult = true;
		}
	}
	else
	{
		bResult = CloseScene();
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
		bResult = HandleCalloutButtonClick(EventParms.InputAliasName, EventParms.PlayerIndex);
	}

	return bResult;
}

/* === UIScene interface === */

/**
 * Called just after the scene is added to the ActiveScenes array, or when this scene has become the active scene as a result
 * of closing another scene.
 *
 * @note: this version updates the nav list's data bindings when the user closes a submenu.
 *
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
event SceneActivated( bool bInitialActivation )
{
	local GearPC OwningPC;

	Super.SceneActivated(bInitialActivation);

	if ( !bInitialActivation && TOCPanel != None )
	{
		TOCPanel.lstNavigation.NotifyDataStoreValueUpdated(TOCPanel.lstNavigation.DataSource.ResolvedDataStore, false, '', None, TOCPanel.lstNavigation.GetCurrentItem());
	}
	PlayUISound('G2UI_JournalNavigateCue');

	OwningPC = GetGearPlayerOwner(GetPlayerOwnerIndex());
	if (OwningPC != None && OwningPC.ProfileSettings != None)
	{
		// Just in case they missed their achievement, update their progress
		OwningPC.ProfileSettings.UpdateDiscoverableProgression(OwningPC,true);
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	local GearPC OwningPC;

	OwningPC = GetGearPlayerOwner(GetPlayerOwnerIndex());
	if (OwningPC != none)
	{
		OwningPC.SaveProfile();
	}
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

`if(`isdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
	out_KeyNames.AddItem('WarJournalPrevChapter');
	out_KeyNames.AddItem('WarJournalNextChapter');
`endif
}

DefaultProperties
{
	PreviousChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.AchievementsJournal'
	NextChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.TOC.WarJournalCollectableTOC'

`if(`isdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
	OnProcessInputKey=ProcessInputKey

//@todo ronp animations - only these animations are allowed, until we add animation support to 'replacescene'
	SceneAnimation_Open=ActivateSceneSeq
	SceneAnimation_Close=DeactivateSceneSeq
`endif
}
