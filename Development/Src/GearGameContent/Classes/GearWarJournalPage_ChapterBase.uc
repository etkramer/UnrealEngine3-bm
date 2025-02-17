/**
 * Base class for page classes used by War Journal sections that spread data across multiple pages (similar to a chapter in a book).
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_ChapterBase extends GearWarJournalPage_Base
	abstract;

/** a label that displays the page number for this journal page (e.g. 3 / 5) */
var	transient	UILabel				lblPageNumber;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Retrieve the maximum number of items that can fit on the page.
 *
 * @param	PageIdx					the index of the page to calculate the max items for.
 * @param	bRequireValidPageIndex	if TRUE, function should return 0 if PageIdx is not within the range of pages
 *
 * @return	the number of items that can fit on the page specified.
 */
function int GetMaxItemCount( int PageIdx, optional bool bRequireValidPageIndex )
{
	return 0;
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */

/* == GearWarJournalPage_Base interface == */
/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences()
{
	lblPageNumber = UILabel(FindChild('lblPageNumber',true));
	Super.InitializePageReferences();
}

/**
 * Assigns the index for this page.
 *
 * @param	NewPageIndex	the index that is now associated with this page.
 */
function SetPageIndex( int NewPageIndex )
{
	local string PageNumberString;

	Super.SetPageIndex(NewPageIndex);

	if ( IsValidPage() )
	{
		// update the page number label with the correct value
		GetDataStoreStringValue("<Strings:GearGameUI.FrontEndStrings.WarJournalPageNumber>", PageNumberString);
		lblPageNumber.SetDataStoreBinding(Repl(Repl(PageNumberString, "\`current\`", NewPageIndex + 1), "\`total\`", GetTotalPages()));
	}
	else
	{
		lblPageNumber.SetDataStoreBinding("");
	}
}

DefaultProperties
{
	DefaultStates.Remove(class'UIState_Focused')
}
