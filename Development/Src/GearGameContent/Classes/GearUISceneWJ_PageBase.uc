/**
 * Base class for all scenes which are displayed with an open book style.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_PageBase extends GearUISceneWJ_Base
	abstract;

var	transient	GearUISceneWJ_Base			PreviousChapterScene, NextChapterScene;

/** the left and right pages of the war journal */
var	transient	GearWarJournalPage_Base		LeftPage, RightPage;

var	transient	UICalloutButtonPanel		btnbarLeftPage, btnbarRightPage;

/** the index for the page currently on the left side */
var	transient	int							CurrentPage;

/** the total number of pages in this war journal section */
var	transient	int							TotalPages;

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

	btnbarLeftPage = UICalloutButtonPanel(FindChild('btnbarLeftPage',true));
	btnbarRightPage = UICalloutButtonPanel(FindChild('btnbarRightPage',true));

	LeftPage = GearWarJournalPage_Base(FindChild('pnlLeftPage',true));
	RightPage = GearWarJournalPage_Base(FindChild('pnlRightPage',true));

	if ( LeftPage != None )
	{
		LeftPage.InitializePageReferences();
	}

	if ( RightPage != None )
	{
		RightPage.InitializePageReferences();
	}
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
`if(`notdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
	local int PlayerIndex;
`endif

	Super.SetupCallbacks();

	if ( btnbarLeftPage != None )
	{
		btnbarLeftPage.SetButtonCallback('WarJournalPrevChapter', CalloutButtonClicked);
		btnbarLeftPage.SetButtonCallback('WarJournalPrevPage', CalloutButtonClicked);

//		btnbarLeftPage.SetButtonCaption('WarJournalPrevPage', "<Strings:GearGameUI.FrontEndStrings.WJPreviousPage>");
	}

	if ( btnbarRightPage != None )
	{
		btnbarRightPage.SetButtonCallback('WarJournalNextChapter', CalloutButtonClicked);
		btnbarRightPage.SetButtonCallback('WarJournalNextPage', CalloutButtonClicked);
//		btnbarRightPage.SetButtonCaption('WarJournalNextPage', "<Strings:GearGameUI.FrontEndStrings.WJNextPage>");
	}

`if(`notdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
	PlayerIndex = GetPlayerOwnerIndex();
	if ( btnbarLeftPage != None )
	{
		btnbarLeftPage.EnableButton('WarJournalPrevChapter',PlayerIndex, false);
	}
	if ( btnbarRightPage != None )
	{
		btnbarRightPage.EnableButton('WarJournalNextChapter',PlayerIndex,false);
	}
`endif
}

/**
 * Sets up forced navigation links between panels in the left and right pages.
 */
function SetupForcedNavigationTargets();

/**
 * Changes the currently active "page".
 *
 * @param	NewPageIndex	the index for the page that should be active (should always be the index of a page on the left)
 *
 * @return	true if the page was changed successfully.
 */
function bool SetCurrentPage( int NewPageIndex )
{
	local bool bResult;

	if ( NewPageIndex == `LAST_ITEM_INDEX )
	{
		NewPageIndex = TotalPages - 1;
	}

	// make sure it's a page number that corresponds to the left side
	if ( NewPageIndex >= 0 && NewPageIndex < TotalPages )
	{
		if ( NewPageIndex % 2 != 0 )
		{
			NewPageIndex = NewPageIndex - 1;
		}

		CurrentPage = NewPageIndex;

		LeftPage.SetPageIndex(CurrentPage);
		if ( CurrentPage < TotalPages - 1 )
		{
			RightPage.SetPageIndex(CurrentPage + 1);
		}
		else
		{
			RightPage.SetPageIndex(INDEX_NONE);
		}

		if ( btnbarLeftPage != None )
		{
			// disable the 'prev page' button if this is the first page. if we only have two pages, don't show it at all
			btnbarLeftPage.EnableButton('WarJournalPrevPage', GetPlayerOwnerIndex(), CurrentPage > 0, TotalPages <= 2);
		}

		if ( btnbarRightPage != None )
		{
			// disable the 'next page' button if this is the last page. if we only have two pages, don't show it at all
			btnbarRightPage.EnableButton('WarJournalNextPage', GetPlayerOwnerIndex(), CurrentPage + 2 < TotalPages, TotalPages <= 2);
		}

		SetupForcedNavigationTargets();
		bResult = true;
	}
	else if ( TotalPages <= 2 )
	{
		if ( btnbarLeftPage != None )
		{
			// disable the 'prev page' button if this is the first page. if we only have two pages, don't show it at all
			btnbarLeftPage.EnableButton('WarJournalPrevPage', GetPlayerOwnerIndex(), false);
		}

		if ( btnbarRightPage != None )
		{
			// disable the 'next page' button if this is the last page. if we only have two pages, don't show it at all
			btnbarRightPage.EnableButton('WarJournalNextPage', GetPlayerOwnerIndex(), false);
		}
	}

	return bResult;
}

/**
 * Calculates the total number of pages required by this scene.
 *
 * @return	the total number of pages needed to display all content for this journal type.
 */
function int CalculateRequiredPageCount( int PlayerIndex=GetBestPlayerIndex() )
{
	return 0;
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
		if ( ReplaceScene(Self, PreviousChapterScene, GetPlayerOwner(PlayerIndex), SceneInstance) )
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
		if ( CloseScene() )
		{
		//@todo ronp animation
			bResult = true;
		}
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
		if ( ReplaceScene(Self, NextChapterScene, GetPlayerOwner(PlayerIndex), SceneInstance) )
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
		if ( CloseScene() )
		{
		//@todo ronp animation
			bResult = true;
		}
	}

	return bResult;
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Worker for CalloutButtonClicked - only called once all conditions for handling the input have been met.  Child classes should
 * override this function rather than CalloutButtonClicked, unless additional constraints are necessary.
 *
 * @param	InputAliasTag	the callout input alias associated with the input that was received
 * @param	PlayerIndex		index for the player that generated the event
 *
 * @return	TRUE if this click was processed.
 */
function bool HandleCalloutButtonClick( name InputAliasTag, int PlayerIndex )
{
	local bool bResult;

	if ( InputAliasTag == 'WarJournalPrevChapter' )
	{
		OpenPreviousChapter(PlayerIndex);
		bResult = true;
	}
	else if ( InputAliasTag == 'WarJournalNextChapter' )
	{
		OpenNextChapter(PlayerIndex);
		bResult = true;
	}
	else if ( InputAliasTag == 'WarJournalPrevPage' )
	{
		if ( !SetCurrentPage(CurrentPage - 2) )
		{
		`if(`isdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
			OpenPreviousChapter(PlayerIndex);
		`else
			PlayUISound('GenericError');
		`endif
		}
		bResult = true;
	}
	else if ( InputAliasTag == 'WarJournalNextPage' )
	{
		if ( !SetCurrentPage(CurrentPage + 2) )
		{
		`if(`isdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
			OpenNextChapter(PlayerIndex);
		`else
			PlayUISound('GenericError');
		`endif
		}
		bResult = true;
	}

	return bResult || Super.HandleCalloutButtonClick(InputAliasTag, PlayerIndex);
}

/**
 * Handler for this widget's OnProcessInputKey delegate.  Responsible for displaying this achievement's details when the correct
 * button is pressed.
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

	if ( EventParms.PlayerIndex == GetPlayerOwnerIndex() )
	{
//	`log(`location @ `showvar(EventParms.InputAliasName) @ `showvar(EventParms.EventType) @ `showvar(CurrentPage)
//		@ `showvar(LeftPage.IsFocused(EventParms.PlayerIndex),LeftIsFocused)
//		@ `showvar(LeftPage.GetFocusedControl(false, EventParms.PlayerIndex),LeftFocusedControl)
//		@ `showvar(RightPage.IsFocused(EventParms.PlayerIndex),RightIsFocused)
//		@ `showvar(RightPage.GetFocusedControl(false, EventParms.PlayerIndex),RightFocusedControl));

		if ( EventParms.EventType == IE_Released && HandleCalloutButtonClick(EventParms.InputAliasName, EventParms.PlayerIndex) )
		{
			bResult = true;
		}
		else if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			if ( EventParms.InputAliasName == 'NavFocusLeft' )
			{
				if (!RightPage.IsFocused(EventParms.PlayerIndex)
				||	LeftPage.GetFocusedControl(false, EventParms.PlayerIndex) == None )
				{
					SetCurrentPage(CurrentPage - 1);
					bResult = true;
				}
			}
			else if ( EventParms.InputAliasName == 'NavFocusRight' )
			{
				if (!LeftPage.IsFocused(EventParms.PlayerIndex)
				||	RightPage.GetFocusedControl(false, EventParms.PlayerIndex) == None)
				{
					SetCurrentPage(CurrentPage + 2);
					bResult = true;
				}
			}
		}
	}

	return bResult;
}

/**
 * Handler for the scene's OnSceneActivated delegate.  Called after scene focus has been initialized, or when the scene becomes active as
 * the result of another scene closing.
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	if (ActivatedScene == self && bInitialActivation)
	{
		TotalPages = CalculateRequiredPageCount();

		// this might be clobbered by whoever opened this page
		SetCurrentPage(0);

		`log(`location @ `showvar(TotalPages),,'RON_DEBUG');
	}

//	UpdateSceneState();
}

DefaultProperties
{
	OnSceneActivated=SceneActivationComplete
	OnProcessInputKey=ProcessInputKey

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object
}

