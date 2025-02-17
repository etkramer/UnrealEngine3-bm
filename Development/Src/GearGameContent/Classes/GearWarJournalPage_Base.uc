/**
 * Base class for War Journal "page" panels.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_Base extends UIPanel
	abstract
	placeable;

/** the index [out of the total number of pages required for this war journal section] of this page */
var	transient	int		PageIndex;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Wrapper for getting a reference to the owning war journal scene.
 */
final function GearUISceneWJ_PageBase GetOwnerWarJournalScene()
{
	return GearUISceneWJ_PageBase(GetScene());
}

/**
 * Wrapper for getting the total number of pages required to display all content for this war journal section.
 */
function int GetTotalPages()
{
	local GearUISceneWJ_PageBase WJOwner;

	WJOwner = GetOwnerWarJournalScene();
	if ( WJOwner != None )
	{
		return WJOwner.TotalPages;
	}

	return 0;
}

/**
 * Wrapper for determining whether the index is within the range of available pages.
 */
function bool IsValidPage( optional int TestIndex=PageIndex )
{
	return TestIndex >= 0 && TestIndex < GetTotalPages();
}

/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences();

/* == SequenceAction handlers == */

/* == Delegate handlers == */

/**
 * Changes the index for this panel when the user "flips" to a new journal page.
 *
 * @param	NewPageIndex	the index to assign to this page.
 */
function SetPageIndex( int NewPageIndex )
{
	`log( `location @ `showvar(NewPageIndex) @ `showvar(PageIndex,Current),,'RON_DEBUG');
	if (PageIndex != NewPageIndex)
	{
		PlayUISound('G2UI_JournalNavigateCue');
	}

	PageIndex = NewPageIndex;
}

DefaultProperties
{
	DefaultStates.Add(class'UIState_Focused')
	PageIndex=INDEX_NONE
}
