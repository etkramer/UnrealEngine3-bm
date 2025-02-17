/*=============================================================================
	UnUILists.cpp: Implementation of UIList and related classes.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "UnUIMarkupResolver.h"

#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnUIKeys.h"

IMPLEMENT_CLASS(UUIList);
IMPLEMENT_CLASS(UUIListElementProvider);
IMPLEMENT_CLASS(UUIListElementCellProvider);
IMPLEMENT_CLASS(UUIContextMenu);

IMPLEMENT_CLASS(UUINavigationList);

#define TEMP_SPLITSCREEN_INDEX UCONST_TEMP_SPLITSCREEN_INDEX

/* ==========================================================================================================
	UUIList
========================================================================================================== */
/**
 * Returns the number of elements in the list.
 */
INT UUIList::GetItemCount() const
{
	return Items.Num();
}

/**
 * Returns the maximum number of elements that can be displayed by the list given its current size and configuration.
 */
INT UUIList::GetMaxVisibleElementCount() const
{
	return MaxVisibleItems;
}

/**
 * Returns the maximum number of rows that can be displayed in the list, given its current size and configuration.
 */
INT UUIList::GetMaxNumVisibleRows() const
{
	INT Result = 0;

	switch ( CellLinkType )
	{
	case LINKED_None:
		Result = RowCount;
		break;

	case LINKED_Rows:
		Result = CellDataComponent ? CellDataComponent->GetSchemaCellCount() : RowCount;
		break;

	case LINKED_Columns:
		Result = MaxVisibleItems;

		break;
	}

	return Result;
}

/**
 * Returns the maximum number of columns that can be displayed in the list, given its current size and configuration.
 */
INT UUIList::GetMaxNumVisibleColumns() const
{
	INT Result = 0;

	switch ( CellLinkType )
	{
	case LINKED_None:
		Result = ColumnCount;
		break;

	case LINKED_Rows:
		Result = MaxVisibleItems;

		break;

	case LINKED_Columns:
		Result = CellDataComponent ? CellDataComponent->GetSchemaCellCount() : ColumnCount;
		break;
	}

	return Result;
}

/**
 * Returns the total number of rows in this list.
 */
INT UUIList::GetTotalRowCount() const
{
	INT Result = 0;

	switch ( CellLinkType )
	{
	case LINKED_None:
		Result = ColumnCount > 0 ? appCeil((FLOAT)GetItemCount() / (FLOAT)ColumnCount) : 0;
//		Result = RowCount;
		break;

	case LINKED_Rows:
		Result = CellDataComponent ? CellDataComponent->GetSchemaCellCount() : RowCount;
		break;

	case LINKED_Columns:
		Result = GetItemCount();
		break;
	}

	return Result;
}

/**
 * Returns the total number of columns in this list.
 */
INT UUIList::GetTotalColumnCount() const
{
	INT Result = 0;

	switch ( CellLinkType )
	{
	case LINKED_None:
// 		Result = RowCount > 0 ? appCeil((FLOAT)GetItemCount() / RowCount) : 0;
		Result = ColumnCount;
		break;

	case LINKED_Rows:
		Result = GetItemCount();
		break;

	case LINKED_Columns:
		Result = CellDataComponent ? CellDataComponent->GetSchemaCellCount() : ColumnCount;
		break;
	}

	return Result;
}

/**
 * Changes the list's ColumnCount to the value specified.
 */
void UUIList::SetColumnCount( INT NewColumnCount )
{
	ColumnCount = NewColumnCount;
	RefreshFormatting();
}

/**
 * Changes the list's RowCount to the value specified.
 */
void UUIList::SetRowCount( INT NewRowCount )
{
	RowCount = NewRowCount;
	RefreshFormatting();
}

/**
 * Returns the width of the specified column.
 *
 * @param	ColumnIndex		the index for the column to get the width for.  If the index is invalid, the list's configured CellWidth is returned instead.
 * @param	bColHeader		specify TRUE to apply HeaderCellPadding instead of CellPadding.
 * @param	bReturnUnformattedValue
 *							specify TRUE to return a value determined by the size of a typical character from the font applied to the cell; otherwise,
 *							uses the cell string's calculated StringExtent, which will include any scaling that has been applied.
 */
FLOAT UUIList::GetColumnWidth( INT ColumnIndex/*=INDEX_NONE*/, UBOOL bColHeader/*=FALSE*/, UBOOL bReturnUnformattedValue/*=TRUE*/ ) const
{
	FLOAT Result=0.f;

	UBOOL bIncludePadding = FALSE;
	FLOAT StylePadding = 0.f;
	if ( ColumnAutoSizeMode == CELLAUTOSIZE_Uniform )
	{
		const INT NumColumns = GetTotalColumnCount();
		if ( NumColumns > 0 )
		{
			Result = GetClientRegion().X / NumColumns;
		}
	}
	else if ( (IsElementAutoSizingEnabled() || bColHeader) && CellLinkType == LINKED_Rows )
	{
		if ( CellDataComponent != NULL )
		{
			CellDataComponent->CalculateAutoSizeColumnWidth(ColumnIndex, Result, StylePadding, bReturnUnformattedValue);
			bIncludePadding = TRUE;
		}
	}
	else if ( CellLinkType == LINKED_Columns )
	{
		if ( CellDataComponent != NULL && CellDataComponent->IsValidSchemaIndex(ColumnIndex) )
		{
			Result = CellDataComponent->GetSchemaCellSize(ColumnIndex);
		}
	}

	if ( Result == 0.f )
	{
		Result = ColumnWidth.GetValue(this);
	}

	if ( ColumnAutoSizeMode != CELLAUTOSIZE_Uniform )
	{
		if ( bIncludePadding )
		{
			const FLOAT Padding = bColHeader
				? HeaderCellPadding.GetValue(this)
				: CellPadding.GetValue(this);

			Result += Padding + StylePadding;
		}
		Result = Max(Result, MinColumnSize.GetValue(this));
	}

	return Result;
}

/**
 * Returns the width of the specified row.
 *
 * @param	RowIndex		the index for the row to get the width for.  If the index is invalid, the list's configured RowHeight is returned instead.
 * @param	bColHeader		specify TRUE to apply HeaderCellPadding instead of CellPadding.
 * @param	bReturnUnformattedValue
 *							specify TRUE to return a value determined by the size of a typical character from the font applied to the cell; otherwise,
 *							uses the cell string's calculated StringExtent, which will include any scaling that has been applied.
 */
FLOAT UUIList::GetRowHeight( INT RowIndex/*=INDEX_NONE*/, UBOOL bColHeader/*=FALSE*/, UBOOL bReturnUnformattedValue/*=FALSE*/ ) const
{
	FLOAT Result=0.f;
	
	UBOOL bIncludePadding = FALSE;
	FLOAT StylePadding = 0.f;
	if ( RowAutoSizeMode == CELLAUTOSIZE_Uniform )
	{
		const INT NumRows = GetTotalRowCount();
		if ( NumRows > 0 )
		{
			Result = GetClientRegion().Y / NumRows;
		}
	}
	else if ( (IsElementAutoSizingEnabled() || bColHeader) && CellLinkType != LINKED_Rows )
	{
		if ( CellDataComponent != NULL )
		{
			CellDataComponent->CalculateAutoSizeRowHeight(RowIndex, Result, StylePadding, bReturnUnformattedValue);
			bIncludePadding = TRUE;
		}
	}
	else if ( CellLinkType == LINKED_Rows )
	{
		if ( CellDataComponent != NULL && CellDataComponent->IsValidSchemaIndex(RowIndex) )
		{
			Result = CellDataComponent->GetSchemaCellSize(RowIndex);
		}
	}
	
	if ( Result == 0.f )
	{
		Result = RowHeight.GetValue(this);
		bIncludePadding = TRUE;

		//@fixme - where should I get value this from?
//		StylePadding = Max(StylePadding, 
	}

	if ( bIncludePadding )
	{
		const FLOAT Padding = bColHeader
			? HeaderCellPadding.GetValue(this)
			: CellPadding.GetValue(this);

		Result += Padding + StylePadding;
	}

	return Result;
}

/**
 * Returns the width and height of the bounding region for rendering the cells, taking into account whether the scrollbar
 * and column header are displayed.
 */
FVector2D UUIList::GetClientRegion() const
{
	FVector2D Result(GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport), GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport));

	if ( VerticalScrollbar != NULL && VerticalScrollbar->IsVisible() )
	{
		Result.X -= VerticalScrollbar->GetScrollZoneWidth();
	}

	//@todo ronp - might need to add a parameter to GetClientRegion that we can pass to GetRowHeight as the value for
	// its bReturnUnformattedValue parameter (for example, GetClientRegion() is called when we initialize the scrollbar and
	// at this point, the list rows may not have been formatted yet)
	Result.Y -= GetHeaderSize();
	return Result;
}
	
/**
 * Wrapper for calculating the amount of additional room the list needs at the top to render headers or other things.
 */
FLOAT UUIList::GetHeaderSize() const
{
	FLOAT Result = HeaderElementSpacing.GetValue(this);

	if ( ShouldRenderColumnHeaders() && CellDataComponent != NULL && CellDataComponent->GetSchemaCellCount() > 0 )
	{
		//@todo ronp - might need to add a parameter to GetClientRegion that we can pass to GetRowHeight as the value for
		// its bReturnUnformattedValue parameter (for example, GetClientRegion() is called when we initialize the scrollbar and
		// at this point, the list rows may not have been formatted yet)
		Result += (GetRowHeight(INDEX_NONE, TRUE));
	}
	
	return Result;
}

/**
 * Returns the items that are currently selected.
 *
 * @return	an array of values that represent indexes into the data source's data array for the list elements that are selected.
 *			these indexes are NOT indexes into the UIList.Items array; rather, they are the values of the UIList.Items elements which
 *			correspond to the selected items
 */
TArray<INT> UUIList::GetSelectedItems() const
{
	return SelectedItems;
}

/**
 * Returns the value of the element associated with the current list index
 *
 * @return	the value of the element at Index; this is not necessarily an index into the UIList.Items array; rather, it is the value
 *			of the UIList.Items element located at Index
 */
INT UUIList::GetCurrentItem() const
{
	INT Result = INDEX_NONE;
	
	if ( Items.IsValidIndex(Index) )
	{
		Result = Items(Index);
	}
	
	return Result;
}

/**
 * Returns the text value for the specified element.
 *
 * @param	ElementIndex	index [into the Items array] for the value to return.
 * @param	CellIndex		for lists which have linked columns or rows, indicates which column/row to retrieve.
 *
 * @return	the value of the specified element, or an empty string if that element doesn't have a text value.
 */
FString UUIList::GetElementValue( INT ElementIndex, INT CellIndex/*=INDEX_NONE*/ ) const
{
	FString Result;

	if ( CellDataComponent != NULL )
	{
		return CellDataComponent->GetElementValue(ElementIndex, CellIndex);
	}

	return Result;
}

/**
 * Inserts a new element into the list at the specified index
 *
 * @param	ElementToInsert		the element to insert into the list
 * @param	InsertIndex			an index in the range of 0 - Items.Num() to use for inserting the element.  If the value is
 *								not a valid index, the element will be added to the end of the list.
 * @param	bSkipSorting		specify TRUE to prevent the list from being resorted after this element is added (useful when
 *								adding items in bulk).
 *
 * @return	the index where the new element was inserted, or INDEX_NONE if the element wasn't added to the list.
 */
INT UUIList::InsertElement( INT ElementToInsert, INT InsertIndex/*=INDEX_NONE*/, UBOOL bSkipSorting/*=FALSE*/ )
{
	INT Result = INDEX_NONE;

	const INT PreviousItemCount = GetItemCount();

	InsertIndex = CellDataComponent->InsertElement(InsertIndex, ElementToInsert);
	if ( Items.IsValidIndex(InsertIndex) )
	{
		NotifyItemCountChanged(PreviousItemCount, GetBestPlayerIndex());

		//@todo - notification via event activation

		SetIndex( Index, TRUE );

		if ( !bSkipSorting && SortComponent != NULL )
		{
			// reapply sorting
			SortComponent->ResortItems();
			Result = Items.FindItemIndex(ElementToInsert);
		}
		else
		{
			Result = InsertIndex;
		}
	}

	return Result;
}

/**
 * Inserts multiple elements into the list at the specified index
 *
 * @param	ElementsToInsert	the elements to insert into the list
 * @param	InsertIndex			an index in the range of 0 - Items.Num() to use for inserting the elements.  If the value is
 *								not a valid index, the elements will be added to the end of the list.  Elements will be added
 *								in the order they appear in the array, so ElementsToInsert(0) will be inserted at InsertIndex,
 *								ElementsToInsert(1) will be inserted at InsertIndex+1, etc.
 * @param	bSkipSorting		specify TRUE to prevent the list from being resorted after this element is added (useful when
 *								adding items in bulk).
 *
 * @return	the number of elements that were added to the list
 */
INT UUIList::InsertElements( const TArray<INT>& ElementsToInsert, INT InsertIndex/*=INDEX_NONE*/, UBOOL bSkipSorting/*=FALSE*/ )
{
	const INT PreviousItemCount = GetItemCount();
	INT AddedCount = 0;

	eventDisableSetIndex();
	for ( INT ElementIndex = 0; ElementIndex < ElementsToInsert.Num(); ElementIndex++ )
	{
		INT ElementToInsert = ElementsToInsert(ElementIndex);

		// ActualInsertIndex might be different from InsertIndex if the cell data component has
		// different rules for adding items (i.e. tree control), 
		INT ActualInsertIndex = CellDataComponent->InsertElement(InsertIndex, ElementToInsert);
		if ( Items.IsValidIndex(ActualInsertIndex) )
		{
			AddedCount++;
		}
	}
	eventEnableSetIndex();

	if ( AddedCount > 0 )
	{
		NotifyItemCountChanged(PreviousItemCount, GetBestPlayerIndex());

		//@todo - notification via event activation
		SetIndex( Index, TRUE );

		if ( !bSkipSorting && SortComponent != NULL )
		{
			// reapply sorting
			SortComponent->ResortItems();
		}
	}

	return AddedCount;
}

/**
 * Removes the specified element from the list.
 *
 * @param	ElementToRemove		the element to remove from the list
 *
 * @return	the index [into the Items array] for the element that was removed, or INDEX_NONE if the element wasn't
 *			part of the list.
 */
INT UUIList::RemoveElement( INT ElementToRemove )
{
	INT RemovalIndex = FindElementIndex(ElementToRemove);

	return RemoveElementAtIndex(RemovalIndex);
}

/**
 * Removes the element located at the specified index from the list.
 *
 * @param	RemovalIndex	the index for the element that should be removed from the list
 *
 * @return	the index [into the Items array] for the element that was removed, or INDEX_NONE if the element wasn't
 *			part of the list.
 */
INT UUIList::RemoveElementAtIndex( INT RemovalIndex )
{
	INT Result = INDEX_NONE;

	INT ListIndex = CellDataComponent->RemoveElement(RemovalIndex);
	if ( Items.IsValidIndex(ListIndex) )
	{
		const INT PreviousItemCount = GetItemCount();

		const INT DataIndex = Items(ListIndex);

		Items.Remove(ListIndex);
		NotifyItemCountChanged(PreviousItemCount, GetBestPlayerIndex());

		// remove this element's index from the selection set
		SelectedItems.RemoveItem(DataIndex);

		//@todo - notification via event activation

		SetIndex(Index,TRUE);
		Result = RemovalIndex;
	}

	return Result;
}

/**
 * Removes the specified elements from the list.
 *
 * @param	ElementsToRemove	the elements to remove from the list (make sure this parameter creates a copy of the array, so
 *								that the removal algorithm works correctly; i.e. don't change this to a const& for performance or something)
 *
 * @return	the number of elements that were removed from the list
 */
INT UUIList::RemoveElements( const TArray<INT>& ElementsToRemove )
{
	INT RemovedCount = 0;
	if ( Items.Num() > 0 )
	{
		eventDisableSetIndex();

		for ( INT ElementIndex = 0; ElementIndex < ElementsToRemove.Num(); ElementIndex++ )
		{
			INT ElementToRemove = ElementsToRemove(ElementIndex);
			INT ListIndex = Items.FindItemIndex(ElementToRemove);
			while ( ListIndex != INDEX_NONE )
			{
				if ( RemoveElementAtIndex(ListIndex) != INDEX_NONE )
				{
					RemovedCount++;
				}
				else
				{
					// if we couldn't remove that element, break out of the while loop 
					// so that we're not infinitely attempting to remove the element
					break;
				}

				ListIndex = Items.FindItemIndex(ElementToRemove);
			}
		}

		eventEnableSetIndex();
	}

	if ( RemovedCount > 0 )
	{
		//@todo - notification via event activation

		SetIndex(Index,TRUE);
	}

	return RemovedCount;
}

/**
 * Clears all elements from the list.
 */
void UUIList::ClearElements()
{
	// make a copy of this array, since RemoveElements takes a reference parameter
	TArray<INT> AllItems = Items;
	RemoveElements(AllItems);
}

/**
 * Moves the specified element by the specified number of items.
 *
 * @param	ElementToMove	the element to move. This is not an index into the Items array; rather, it is the value of an element
 *							in the Items array, which corresponds to an index into data store collection this list is bound to.
 * @param	MoveCount		the number of items to move the element.
 *
 * @param	TRUE if the element was moved successfully; FALSE otherwise
 */
UBOOL UUIList::MoveElement( INT ElementToMove, INT MoveCount )
{
	INT ElementIndex = FindElementIndex(ElementToMove);
	return MoveElementAtIndex(ElementIndex,MoveCount);
}

/**
 * Moves the element at the specified index by the specified number of items.
 *
 * @param	ElementIndex	the index for the element to move.
 * @param	MoveCount		the number of items to move the element.
 *
 * @param	TRUE if the element was moved successfully; FALSE otherwise
 */
UBOOL UUIList::MoveElementAtIndex( INT ElementIndex, INT MoveCount )
{
	UBOOL bResult = FALSE;

	if ( Items.IsValidIndex(ElementIndex) && MoveCount != 0 && Items.IsValidIndex(ElementIndex + MoveCount) )
	{
		if ( Abs<INT>(MoveCount) == 1 )
		{
			// if the move count is only 1, we can use the [faster] SwapElements method instead
			bResult = SwapElementsByIndex(ElementIndex, ElementIndex + MoveCount);
		}
		else
		{
			// store the current value
			INT ElementToMove = Items(ElementIndex);

			// now remove the element from the old location
			RemoveElementAtIndex(ElementIndex);

			// and insert it into the new location
			InsertElement(ElementToMove, ElementIndex + MoveCount);

			if ( ElementIndex == Index )
			{
				SetIndex(ElementIndex + MoveCount);
			}
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Swaps the elements specified, reversing their positions in the Items array.
 *
 * @param	ElementA	the first element to swap. This is not an index into the Items array; rather, it is the value of an element
 *						in the Items array, which corresponds to an index into data store collection this list is bound to.
 * @param	ElementB	the second element to swap. This is not an index into the Items array; rather, it is the value of an element
 *						in the Items array, which corresponds to an index into data store collection this list is bound to.
 *
 * @param	TRUE if the swap was successful
 */
UBOOL UUIList::SwapElementsByValue( INT ElementA, INT ElementB )
{
	INT IndexA = FindElementIndex(ElementA);
	INT IndexB = FindElementIndex(ElementB);

	return SwapElementsByIndex(IndexA, IndexB);
}

/**
 * Swaps the values at the specified indexes, reversing their positions in the Items array.
 *
 * @param	IndexA	the index into the Items array for the first element to swap
 * @param	IndexB	the index into the Items array for the second element to swap
 *
 * @param	TRUE if the swap was successful
 */
UBOOL UUIList::SwapElementsByIndex( INT IndexA, INT IndexB )
{
	UBOOL bResult = FALSE;

	if ( Items.IsValidIndex(IndexA) && Items.IsValidIndex(IndexB) )
	{
		if ( CellDataComponent == NULL || CellDataComponent->SwapElements(IndexA, IndexB) )
		{
			Items.SwapItems(IndexA, IndexB);
			if ( Index == IndexA )
			{
				SetIndex(IndexB);
			}
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Finds the index for the element specified
 *
 * @param	ElementToFind	the element to search for
 *
 * @return	the index [into the Items array] for the element specified, or INDEX_NONE if the element wasn't
 *			part of the list.
 */
INT UUIList::FindElementIndex( INT ElementToFind ) const
{
	return Items.FindItemIndex(ElementToFind);
}

/**
 * Finds the index for the element with the specified text.
 *
 * @param	StringValue		the value to find
 * @param	CellIndex		for lists which have linked columns or rows, indicates which column/row to check
 *
 * @return	the index [into the Items array] for the element with the specified value, or INDEX_NONE if not found.
 */
INT UUIList::FindItemIndex( const FString& ItemValue, INT CellIndex/*=INDEX_NONE*/ ) const
{
	INT Result = INDEX_NONE;

	for ( INT Idx = 0; Idx < GetItemCount(); Idx++ )
	{
		if ( ItemValue == GetElementValue(Idx, CellIndex) )
		{
			Result = Idx;
			break;
		}
	}

	return Result;
}

/**
 * Sets the list's index to the value specified and activates the appropriate notification events.
 *
 * @param	NewIndex			An index into the Items array that should become the new Index for the list.
 * @param	bClampValue			if TRUE, NewIndex will be clamped to a valid value in the range of 0 -> ItemCount
 * @param	bSkipNotification	if TRUE, no events are generated as a result of updating the list's index.
 *
 * @return	TRUE if the list's Index was successfully changed.
 */
UBOOL UUIList::SetIndex( INT NewIndex, UBOOL bClampValue/*=TRUE*/, UBOOL bSkipNotification/*=FALSE*/ )
{
	// store the current index
	const INT PreviousIndex = Index;
	const INT CurrentItemCount = GetItemCount();
	if ( bClampValue )
	{
		NewIndex = CurrentItemCount > 0
			? Clamp(NewIndex, 0, CurrentItemCount - 1)
			: INDEX_NONE;
	}
	else if ( !Items.IsValidIndex(NewIndex) )
	{
		// the only "valid" invalid index is -1, so if the new index is out of range, change it to INDEX_NONE
		NewIndex = INDEX_NONE;
	}

	UBOOL bIndexChanged = FALSE;
	if ( (NewIndex == INDEX_NONE || CanSelectElement(NewIndex))
	&&	eventIsSetIndexEnabled() )
	{
		bIndexChanged = Index != NewIndex;
		Index = NewIndex;

		const INT VisibleElements = GetMaxVisibleElementCount();
		if ( Index >= 0 && VisibleElements > 0 )
		{
			// if the new index is less than the first visible item
			if ( Index < TopIndex )
			{
// 				debugf(TEXT("%s changing TopIndex (a) - Index:%i => %i  TopIndex:%i => %i  ItemCount:%i  VisibleElements:%i"),
// 					*GetName(), PreviousIndex, Index, TopIndex, Index, CurrentItemCount, VisibleElements);
				// change the first visible item to be the element at index
				SetTopIndex(Index, bClampValue);
			}

			// if the new index is greater than the index of the last visible item, 
			else if ( Index >= TopIndex + VisibleElements )
			{
// 				debugf(TEXT("%s changing TopIndex (b) - Index:%i => %i  TopIndex:%i => %i  ItemCount:%i  VisibleElements:%i"),
// 					*GetName(), PreviousIndex, Index, TopIndex, Index - VisibleElements + 1, CurrentItemCount, VisibleElements);
				// change the first visible item such that the last visible item is the item located at Index
				SetTopIndex(Index - VisibleElements + 1, bClampValue);
			}

			// if the first visible item + the number of items being displayed is greater than the number of items in the list
			else if ( bForceFullPageDisplay && TopIndex > 0 && TopIndex + VisibleElements > CurrentItemCount )
			{
// 				debugf(TEXT("%s changing TopIndex (c) - Index:%i => %i  TopIndex:%i => %i  ItemCount:%i  VisibleElements:%i"),
// 					*GetName(), PreviousIndex, Index, TopIndex, CurrentItemCount - VisibleElements, CurrentItemCount, VisibleElements);
				// change the first visible item so that the visible items represent the last items in the list
				SetTopIndex(CurrentItemCount - VisibleElements, bClampValue);
			}

			// If top index hasn't been set yet...
			else if(TopIndex == INDEX_NONE)
			{
// 				debugf(TEXT("%s changing TopIndex (d) - Index:%i => %i  TopIndex:%i => %i  ItemCount:%i  VisibleElements:%i"),
// 					*GetName(), PreviousIndex, Index, TopIndex, 0, CurrentItemCount, VisibleElements);
				SetTopIndex(0, bClampValue);
			}
		}
		else
		{
// 			debugf(TEXT("%s changing TopIndex (e) - Index:%i => %i  TopIndex:%i => -1  ItemCount:%i  VisibleElements:%i"),
// 				*GetName(), PreviousIndex, Index, TopIndex, CurrentItemCount, VisibleElements);
			SetTopIndex(INDEX_NONE,TRUE);
		}

		if ( bIndexChanged )
		{
			//@todo - support multi-select
			SelectElement(PreviousIndex,FALSE);
			SelectElement(Index,TRUE);

			if ( !bSkipNotification && eventIsValueChangeNotificationEnabled() )
			{
				NotifyIndexChanged( PreviousIndex, GetBestPlayerIndex() );
			}
		}
		else if ( Items.IsValidIndex(Index) )
		{
			if ( !SelectedItems.ContainsItem(Items(Index)) )
			{
				SelectElement(Index, TRUE);
			}
		}
	}

	return bIndexChanged;
}

/**
 * Changes the list's first visible item to the element at the index specified.
 *
 * @param	NewTopIndex		an index into the Items array that should become the new first visible item.
 * @param	bClampValue		if TRUE, NewTopIndex will be clamped to a valid value in the range of 0 - ItemCount - 1
 *
 * @return	TRUE if the list's TopIndex was successfully changed.
 */
UBOOL UUIList::SetTopIndex( INT NewTopIndex, UBOOL bClampValue/*=TRUE*/ )
{
	const INT CurrentItemCount = GetItemCount();
	const INT VisibleElements = GetMaxVisibleElementCount();
	if ( VisibleElements > 0 )
	{
		if ( bClampValue && !Items.IsValidIndex(NewTopIndex) )
		{
			NewTopIndex = CurrentItemCount > 0
				? Clamp(NewTopIndex, 0, CurrentItemCount - 1)
				: INDEX_NONE;
		}
	}
	else
	{
		NewTopIndex = INDEX_NONE;
	}

	INT PreviousTopIndex = TopIndex;
	TopIndex = NewTopIndex;

	// if the first visible item + the number of items being displayed is greater than the number of items in the list
	if ( bForceFullPageDisplay && TopIndex + VisibleElements > CurrentItemCount )
	{
		// change the first visible item so that the visible items represent the last items in the list
		TopIndex = Max(0, CurrentItemCount - VisibleElements);
	}

	UBOOL bResult = FALSE;
	if ( PreviousTopIndex != TopIndex )
	{
		//@todo ronp - notifications
		//@todo ronp - GetBestPlayerIndex() might not be the correct thing to do here
		NotifyTopIndexChanged(PreviousTopIndex, GetBestPlayerIndex());
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Utility function for modifying the index in response to keyboard or gamepad input.
 *
 * @param	bIncrementIndex		TRUE if the index should be increased, FALSE if the index should be decreased
 * @param	bFullPage			TRUE to change the index by a full page, FALSE to change the index by 1
 * @param	bHorizontalNavigation	TRUE if the user pressed right or left, FALSE if the user pressed up or down.
 *
 * @return	TRUE if the index was successfully changed
 */
UBOOL UUIList::NavigateIndex( UBOOL bIncrementIndex, UBOOL bFullPage, UBOOL bHorizontalNavigation )
{
	UBOOL bResult = FALSE;

	const INT CurrentItemCount = GetItemCount();
	const INT CurrentColumnCount = GetMaxNumVisibleColumns();
	const INT CurrentRowCount = GetMaxNumVisibleRows();
	const INT VisibleElements = GetMaxVisibleElementCount();

	// number of items that could potentially be navigated to in this direction (i.e. the current number of columns if we're moving left/right)
	const INT PotentialNavCount = bHorizontalNavigation ? CurrentColumnCount : CurrentRowCount;

	// the extrema value that the index could be changed to
	const INT IndexBoundary = bIncrementIndex ? CurrentItemCount - 1 : 0;

	// the cell link type for the orientation that the user is attempting to navigate 
	const ECellLinkType CorrespondingLinkType = bHorizontalNavigation ? LINKED_Columns : LINKED_Rows;

	// the cell link type for the orientation perpendicular to the direction that the user is attempting to navigate 
	const ECellLinkType OppositeLinkType = bHorizontalNavigation ? LINKED_Rows : LINKED_Columns;

	// used for changing the index in the correct direction
	const INT DeltaModifier = bIncrementIndex ? 1 : -1;

	// the amount to change the index by
	INT IndexDelta = (bFullPage ? (VisibleElements - 1) : 1) * DeltaModifier;

	if ( PotentialNavCount > 0 && IndexDelta != 0 )
	{
		// if we don't have enough items to change the index, can't navigate anywhere
		if ( CurrentItemCount < 2 )
		{
			bResult = FALSE;
		}
		else
		{
			if ( CellLinkType == CorrespondingLinkType )
			{
				// we are linked along the axis the user is attempting to navigate - scroll the list without changing the index
				if ( bHorizontalNavigation )
				{
					//@todo - scroll left/right?
				}
				else
				{
					// scroll up or down
					//@todo - support page scrolling by checking whether Ctrl is being held
					SetTopIndex( TopIndex + (CurrentColumnCount * DeltaModifier) );
				}

				bResult = TRUE;
			}
			else
			{
				INT StartIndex = Index;
				do 
				{
					INT NewIndex = Clamp(StartIndex + IndexDelta, 0, CurrentItemCount - 1);

					UBOOL bBoundaryElementSelected = StartIndex == IndexBoundary;
					if ( CellLinkType != OppositeLinkType && CellLinkType != LINKED_None )
					{
						bBoundaryElementSelected = (StartIndex % PotentialNavCount) == (bIncrementIndex ? PotentialNavCount - 1 : 0);
					}

					// if the first (or last) element is already selected, we can't go anywhere
					if ( bBoundaryElementSelected )
					{
						// if this list is configured to wrap index selection, change the index to the next element starting
						// from the other end of this list... note we only do this when the boundary index is selected, so that
						// the user can't accidentally roll past the end of the list using pageup/pagedown (the index must be the first/last element
						// for these keys to cause an index wrap)
						if ( WrapType != LISTWRAP_None && WrapType != LISTWRAP_MAX )
						{
							if ( WrapType == LISTWRAP_Jump )
							{
								NewIndex = (bIncrementIndex ? 0 : (CurrentItemCount - 1));
							}
							else
							{
								NewIndex = ((bIncrementIndex ? INDEX_NONE : CurrentItemCount) + IndexDelta) % CurrentItemCount;
							}
						}
						else
						{
							// can't navigate anywhere - bail
							break;
						}
					}
					// when paging from anywhere but the top or bottom of the page, move to the top or bottom first
					else if ( bFullPage && WrapType == LISTWRAP_Jump )
					{
						INT BottomIndex = Clamp(TopIndex + VisibleElements - 1, 0, CurrentItemCount - 1);
						if (StartIndex > TopIndex && StartIndex < BottomIndex )
						{
							NewIndex = Clamp(NewIndex, TopIndex, BottomIndex);
						}
					}

					if ( bResult == FALSE )
					{
						if ( NewIndex == Index || !Items.IsValidIndex(NewIndex) )
						{
							// if we're here, it means that we couldn't select any other element - gotta bail
							break;
						}
						else if ( CanSelectElement(NewIndex) )
						{
							bResult = TRUE;

							SetIndex(NewIndex);
						}
						else
						{
							// the next element in that direction was not eligible for selection - reduce the IndexDelta to 1/-1 (in case we were paging up/down)
							// and try to "jump" over this element to the next one
							IndexDelta /= Abs(IndexDelta);
							StartIndex = NewIndex;
						}
					}
				} while ( bResult == FALSE );
			}
		}
	}

	return bResult;
}

/**
 * Activates the focus hint widget for this object; child classes which override this method should set the position of the focus hint
 * as well as any other properties necessary for correctly displaying the focus hint for this widget.
 *
 * @param	FocusHintObject		reference to the widget that supplies the focus hint.
 *
 * @return	TRUE if the focus hint object was initialized / repositioned by this widget; FALSE if this widget doesn't support focus hints.
 */
UBOOL UUIList::AttachFocusHint( UUIObject* FocusHintObject )
{
	UBOOL bResult = FALSE;

	if ( bSupportsFocusHint && FocusHintObject != NULL && CellDataComponent != NULL )
	{
		UUIScreenObject* CurrentParent = FocusHintObject->GetParent();
		if ( CurrentParent != NULL )
		{
			if ( CurrentParent != this )
			{
				CurrentParent->ReparentChild(FocusHintObject, this);
			}
		}
		else
		{
			InsertChild(FocusHintObject);
		}

		FocusHintObject->eventSetVisibility(TRUE);
		UpdateSelectionHint(FocusHintObject);
		bResult = TRUE;
	}

	return bResult;
}
	
/**
 * Updates the visibility and position of the selection hint to appear next to the currently selected item
 */
void UUIList::UpdateSelectionHint( UUIObject* FocusHintObject/*=NULL*/ )
{
	if ( IsFocused(GetBestPlayerIndex()) )
	{
		if ( FocusHintObject == NULL )
		{
			UUIScene* SceneOwner = GetScene();
			if ( SceneOwner != NULL )
			{
				FocusHintObject = SceneOwner->eventGetFocusHint();
			}
		}

//		debugf(TEXT("%s::UpdateSelectionHint - Index:%i  ItemCount:%i  TopIndex:%i  MaxVisibleItems:%i"), *GetName(), Index, Items.Num(), TopIndex, MaxVisibleItems);
		if ( FocusHintObject != NULL )
		{
			if ( CellDataComponent != NULL && Items.IsValidIndex(Index) && Index >= TopIndex && Index < TopIndex + MaxVisibleItems )
			{
				FocusHintObject->eventSetVisibility(TRUE);

				// SetSelectionHintPosition will set the visibility accordingly
				CellDataComponent->SetSelectionHintPosition(FocusHintObject, Index);
			}
			else
			{
				FocusHintObject->eventSetVisibility(FALSE);
			}
		}
	}
}

/**
 * Sets the selection state of the specified element.
 *
 * @param	ElementIndex	the index [into the Items array] of the element to change selection state for
 * @param	bSelected		TRUE to select the element, FALSE to unselect the element.
 */
void UUIList::SelectElement( INT ElementIndex, UBOOL bSelected/*=TRUE*/ )
{
	// veto selection if the element isn't valid for selection
	bSelected = bSelected && CanSelectElement(ElementIndex);
	
	if ( CellDataComponent != NULL && Items.IsValidIndex(ElementIndex) )
	{
		if ( bSelected )
		{
			SelectedItems.AddItem(Items(ElementIndex));
		}
		else
		{
			SelectedItems.RemoveItem(Items(ElementIndex));
		}

		// by default, the element will assume the normal state
		EUIListElementState NewElementState=ELEMENT_Normal;
		
		// if this element is currently under the mouse cursor and bUpdateItemUnderCursor is enabled, always use that state
		if ( bUpdateItemUnderCursor && (!bSelected || bHoverStateOverridesSelected) )
		{
			INT MouseOverIndex = CalculateIndexFromCursorLocation(TRUE);
			if ( MouseOverIndex == ElementIndex )
			{
				NewElementState = ELEMENT_UnderCursor;
			}
		}

		// if this item isn't the one under the cursor (or we have hot-tracking disabled), NewElementState will
		// still have the value ELEMENT_Normal - now figure out what the element's new state should be
		if ( NewElementState == ELEMENT_Normal )
		{
			if ( bSelected )
			{
				// the only thing that supersedes ELEMENT_Selected is ELEMENT_UnderCursor (if bHoverStateOverridesSelected is set), 
				// so we can use the selected state
				NewElementState = ELEMENT_Selected;
			}
			// if this element is the same as the list's Index, it should be in the active state
			else if ( Index == ElementIndex )
			{
				NewElementState = ELEMENT_Active;
			}
		}

		if ( SetElementCellState(ElementIndex, NewElementState) )
		{
			// the CellDataComponent should probably do this, but we'll do it as well just in case
			RequestSceneUpdate(FALSE, TRUE);
		}
	}
}

/**
 * Called when the list's index has changed.
 *
 * @param	PreviousIndex	the list's Index before it was changed
 * @param	PlayerIndex		the index of the player associated with this index change.
 */
void UUIList::NotifyIndexChanged( INT PreviousIndex, INT PlayerIndex )
{
	// notify script (base version calls a delegate)
	NotifyValueChanged(PlayerIndex);

	if ( GIsGame )
	{
		UpdateSelectionHint();

		TArray<UUIEvent_ListIndexChanged*> IndexChangedEvents;
		ActivateEventByClass(PlayerIndex, UUIEvent_ListIndexChanged::StaticClass(), this, FALSE, NULL, (TArray<UUIEvent*>*)&IndexChangedEvents);

		for ( INT EventIndex = 0; EventIndex < IndexChangedEvents.Num(); EventIndex++ )
		{
			UUIEvent_ListIndexChanged* Event = IndexChangedEvents(EventIndex);
			Event->PreviousIndex = PreviousIndex;
			Event->CurrentIndex = Index;
			Event->PopulateLinkedVariableValues();
		}
	}
}

/**
 * Called when the list's top item has changed
 *
 * @param	PreviousIndex	the list's TopIndex before it was changed
 * @param	PlayerIndex		the index of the player that generated this change
 */
void UUIList::NotifyTopIndexChanged( INT PreviousTopIndex, INT PlayerIndex )
{
	// this will cause the scrollbar's position to be sync'd during the next scene update.
	RefreshFormatting();
	UpdateSelectionHint();
}

/**
 * Called when the number of elements in this list is changed.
 *
 * @param	PreviousNumberOfItems	the number of items previously in the list
 * @param	PlayerIndex				the index of the player that generated this change.
 */
void UUIList::NotifyItemCountChanged( INT PreviousNumberOfItems, INT PlayerIndex )
{
	// we need to recalculate everything about the scrollbar (its size, whether it should be visible, etc.)
	bInitializeScrollbars = TRUE;

	// we'll also potentially need to update the formatting of the list's cells to account for any changes in the scrollbar's visibility.
	RefreshFormatting();

	UpdateSelectionHint();
}

/**
 * Called whenever the user chooses an item while this list is focused.  Activates the SubmitSelection kismet event and calls
 * the OnSubmitSelection delegate.
 */
void UUIList::NotifySubmitSelection( INT PlayerIndex/*=0*/ )
{
	if ( GIsGame && GetMaxVisibleElementCount() > 0 && Items.IsValidIndex(Index) )
	{
		if ( DELEGATE_IS_SET(OnSubmitSelection) )
		{
			// notify unrealscript that the user has submitted the selection
			delegateOnSubmitSelection(this,PlayerIndex);
		}

		if ( IsElementEnabled(Index) )
		{
			PlayUISound(SubmitDataSuccessCue, PlayerIndex);

			// notify kismet that the user has submitted the selection
			TArray<UUIEvent_SubmitListData*> Events;
			ActivateEventByClass(PlayerIndex, UUIEvent_SubmitListData::StaticClass(), this, FALSE, NULL, (TArray<UUIEvent*>*)&Events);

			const INT CurrentItem = GetCurrentItem();
			for ( INT EventIndex = 0; EventIndex < Events.Num(); EventIndex++ )
			{
				UUIEvent_SubmitListData* Event = Events(EventIndex);
				Event->SelectedItem = CurrentItem;
				Event->PopulateLinkedVariableValues();
			}
		}
		else
		{
			PlayUISound(SubmitDataFailedCue, PlayerIndex);
		}
	}
}


/**
 * Called after this list's elements have been sorted.  Synchronizes the list's Items array to the data component's elements array.
 */
void UUIList::NotifyListElementsSorted()
{
	// keep track of the currently selected items - we'll restore those once we've resorted
	INT PreviousSelection = GetCurrentItem();

	//@fixme list refactor
	UUIComp_ListPresenter* ReportPresenter = Cast<UUIComp_ListPresenter>(CellDataComponent);
	if ( ReportPresenter != NULL )
	{
		Items.Empty(ReportPresenter->ListItems.Num());
		Items.AddZeroed(ReportPresenter->ListItems.Num());

		for ( INT ItemIndex = 0; ItemIndex < ReportPresenter->ListItems.Num(); ItemIndex++ )
		{
			FUIListItem& ListItem = ReportPresenter->ListItems(ItemIndex);
			Items(ItemIndex) = ListItem.DataSource.DataSourceIndex;

			// UIListElementCell holds a pointer to its owning UIListItem, but once the array has been sorted, this
			// pointer is no longer valid.  So we must re-synchronize those pointers to point to the correct UIListItem
			for ( INT CellIndex = 0; CellIndex < ListItem.Cells.Num(); CellIndex++ )
			{
				FUIListElementCell& ElementCell = ListItem.Cells(CellIndex);
				ElementCell.ContainerElementIndex = ItemIndex;
			}
		}

		// now the Items array is synchronized with the cell data component's items array.  Remove any previously selected items that no longer exist.
		for ( INT SelectionIndex = SelectedItems.Num() - 1; SelectionIndex >= 0; SelectionIndex-- )
		{
			const INT SelectedItem = SelectedItems(SelectionIndex);
			if ( !Items.ContainsItem(SelectedItem) )
			{
				SelectedItems.Remove(SelectionIndex);
			}
		}

		const INT SelectedItemIndex = FindElementIndex(PreviousSelection);
		UUIScene* SceneOwner = GetScene();
		if ( SceneOwner != NULL && SceneOwner->bPerformedInitialUpdate )
		{
			// and update the list's Index to point to the new location of the element previously at Index
			if ( SelectedItemIndex != INDEX_NONE )
			{
				// no need for notification, since this element should still be in the selected state.
				if ( !SetIndex(SelectedItemIndex, TRUE, TRUE) )
				{
					SelectElement(SelectedItemIndex, FALSE);
					SelectElement(Index, TRUE);
				}
			}
		}
		else if ( SelectedItemIndex != Index )
		{
			// Sorting the list probably moved the selected item (the item previously at Index) to another location, but
			// if this is the first time the list is being populated and sorted, we want the index to remain at 0, and we
			// don't want the item previously at 0 to be in the selected cell state.  So if this is the first sort, rather
			// than changing Index to the element that is in the selected state, we deselect that element and select element 0.
			SelectElement(SelectedItemIndex, FALSE);
			SelectElement(Index);
		}

		// finally, reapply
		RefreshFormatting();
		UpdateSelectionHint();
	}

	if ( DELEGATE_IS_SET(OnListElementsSorted) )
	{
		delegateOnListElementsSorted(this);
	}
}

/**
 * Changes the data binding for the specified cell index.
 *
 * @param	CellDataBinding		a name corresponding to a tag from the UIListElementProvider currently bound to this list.
 * @param	ColumnHeader		the string that should be displayed in the column header for this cell.
 * @param	BindingIndex		the column or row to bind this data field to.  If BindingIndex is greater than the number
 *								schema cells, empty schema cells will be added to meet the number required to place the data 
 *								at BindingIndex.
 *								If a value of INDEX_NONE is specified, the cell binding will only occur if there are no other
 *								schema cells bound to that data field.  In this case, a new schema cell will be appended and
 *								it will be bound to the data field specified.
 */
UBOOL UUIList::SetCellBinding( FName CellDataBinding, const FString& ColumnHeader, INT BindingIndex )
{
	UBOOL bResult = FALSE;

	if ( CellDataComponent != NULL )
	{
		bResult = CellDataComponent->SetCellBinding(CellDataBinding,ColumnHeader,BindingIndex);
		if ( bResult == TRUE )
		{
			RefreshListData(FALSE);
		}
	}
	
	return bResult;
}

/**
 * Inserts a new schema cell at the specified index and assigns the data binding.
 *
 * @param	InsertIndex			the column/row to insert the schema cell; must be a valid index.
 * @param	CellDataBinding		a name corresponding to a tag from the UIListElementProvider currently bound to this list.
 * @param	ColumnHeader	the string that should be displayed in the column header for this cell.
 *
 * @return	TRUE if the schema cell was successfully inserted into the list
 */
UBOOL UUIList::InsertSchemaCell( INT InsertIndex, FName CellDataBinding, const FString& ColumnHeader )
{
	UBOOL bResult = FALSE;

	if ( CellDataComponent != NULL )
	{
		bResult = CellDataComponent->InsertSchemaCell(InsertIndex, CellDataBinding, ColumnHeader);
		if ( bResult )
		{
			// now pull the data for the new schema cell from the schema provider
			RefreshListData(FALSE);
		}
	}
	return bResult;
}
	
/**
 * Retrieves the name of the binding for the specified location in the schema.
 *
 * @param	BindingIndex	the index for the cell/column to get the binding for
 *
 * @return	the value assigned to the schema cell at the specified location, or NAME_None if the binding index is invalid.
 */
FName UUIList::GetCellBinding( INT BindingIndex ) const
{
	FName Result = NAME_None;

	if ( CellDataComponent != NULL )
	{
		Result = CellDataComponent->GetCellBinding(BindingIndex);
	}

	return Result;
}

/**
 * Removes all schema cells which are bound to the specified data field.
 *
 * @return	TRUE if one or more schema cells were successfully removed.
 */
UBOOL UUIList::ClearCellBinding( FName CellDataBinding )
{
	UBOOL bResult = FALSE;

	if ( CellDataComponent != NULL )
	{
		bResult = CellDataComponent->ClearCellBinding(CellDataBinding);
		if ( bResult == TRUE )
		{
			RefreshListData(FALSE);
		}
	}

	return bResult;
}

/**
 * Removes schema cells at the location specified.  If the list's columns are linked, this index should correspond to
 * the column that should be removed; if the list's rows are linked, this index should correspond to the row that should
 * be removed.
 *
 * @return	TRUE if the schema cell at BindingIndex was successfully removed.
 */
UBOOL UUIList::ClearCellBinding( INT BindingIndex )
{
	UBOOL bResult = FALSE;

	if ( CellDataComponent != NULL )
	{
		bResult = CellDataComponent->ClearCellBinding(BindingIndex);
		if ( bResult == TRUE )
		{
			RefreshListData(FALSE);
		}
	}

	return bResult;
}

/**
 * Returns the menu state that should be used for rendering the specified element.  By default, returns the list's current
 * menu state, but might return a different menu state in some special cases (for example, when specific elements should be
 * rendered as though they were disabled)
 *
 * @param	ElementIndex	the index into the Items array for the element to retrieve the menu state for.
 *
 * @return	a pointer to the menu state that should be used for rendering the specified element; should correspond to one of the elements
 *			of the UIList's InactiveStates array.
 */
UUIState* UUIList::GetElementMenuState( INT ElementIndex )
{
	UUIState* Result = GetCurrentState();

	UClass* MenuStateClass=NULL;

	// allow the data component to override the menu state that is returned for the specified element
	if ( CellDataComponent != NULL && CellDataComponent->GetOverrideMenuState(ElementIndex,MenuStateClass) )
	{
		if ( MenuStateClass != NULL )
		{
			check(MenuStateClass->IsChildOf(UUIState::StaticClass()));

			for ( INT StateIndex = 0; StateIndex < InactiveStates.Num(); StateIndex++ )
			{
				if ( InactiveStates(StateIndex)->IsA(MenuStateClass) )
				{
					Result = InactiveStates(StateIndex);
					break;
				}
			}
		}
	}

	return Result;
}

/**
 * Changes the cell state for the specified element.
 *
 * @param	ElementIndex	the index [into the Items array] of the element to change states for
 * @param	NewElementState	the new state to place the element in
 *
 * @return	TRUE if the new state was successfully applied to the new element, FALSE otherwise.
 */
UBOOL UUIList::SetElementCellState( INT ElementIndex, /*EUIListElementState*/BYTE NewElementState )
{
	UBOOL bResult = FALSE;

	if ( Items.IsValidIndex(ElementIndex) && CellDataComponent != NULL )
	{
		if ( DELEGATE_IS_SET(OnOverrideListElementState) )
		{
			BYTE CurrentCellState = CellDataComponent->GetElementState(ElementIndex);
			NewElementState = delegateOnOverrideListElementState(this, ElementIndex, CurrentCellState, NewElementState);
		}

		bResult = CellDataComponent->SetElementState(ElementIndex, static_cast<EUIListElementState>(NewElementState));
	}

	return bResult;
}

/**
 * @return	the cell state for the specified element, or ELEMENT_MAX if the index is invalid.
 */
BYTE/*EUIListElementState*/ UUIList::GetElementCellState( INT ElementIndex ) const
{
	EUIListElementState Result = ELEMENT_MAX;

	if ( Items.IsValidIndex(ElementIndex) && CellDataComponent != NULL )
	{
		Result = CellDataComponent->GetElementState(ElementIndex);
	}

	return Result;
}

/**
 * Determines whether the specified list element is disabled by the data source bound to this list.
 *
 * @param	ElementIndex	the index into the Items array for the element to retrieve the menu state for.
 */
UBOOL UUIList::IsElementEnabled( INT ElementIndex )
{
	UBOOL bResult = TRUE;

	if ( DataProvider && Items.IsValidIndex(ElementIndex) )
	{
		const FName DataSourceField = DataSource.DataStoreField;
		const INT DataSourceIndex = Items(ElementIndex);

		// query the bound data store to see if this element is disabled
		bResult = (!DELEGATE_IS_SET(ShouldDisableElement) || !delegateShouldDisableElement(this, ElementIndex))
				&&	DataProvider->IsElementEnabled(DataSourceField, DataSourceIndex);
	}

	return bResult;
}

/**
 * Wrapper for checking whether the specified element is in the selected state.
 *
 * @param	ElementIndex	the index into the Items array for the element to retrieve the menu state for.
 */
UBOOL UUIList::IsElementSelected( INT ElementIndex ) const
{
	UBOOL bResult = FALSE;

	if ( Items.IsValidIndex(ElementIndex) )
	{
		const INT& CollectionIndex = Items(ElementIndex);
		bResult = SelectedItems.ContainsItem(CollectionIndex);
	}

	return bResult;
}

/**
 * Determines whether the specified list element can be selected.
 *
 * @param	ElementIndex	the index into the Items array for the element to query
 *
 * @return	true if the specified element can enter the ELEMENT_Selected state.
 */
UBOOL UUIList::CanSelectElement( INT ElementIndex )
{
	UBOOL bResult = TRUE;

	if ( Items.IsValidIndex(ElementIndex) )
	{
		if ( !bAllowDisabledItemSelection && !IsElementEnabled(ElementIndex) )
		{
			bResult = FALSE;
		}

		//@todo - some more logic here
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

/**
 * Change the value of bUpdateItemUnderCursor to the specified value.
 */
void UUIList::SetHotTracking( UBOOL bShouldUpdateItemUnderCursor )
{
	// this will end up setting both values.
	SetActiveCursorUpdate(bShouldUpdateItemUnderCursor);
}

/**
 * Returns the value of bUpdateItemUnderCursor.
 */
UBOOL UUIList::IsHotTrackingEnabled() const
{
	return bUpdateItemUnderCursor;
}

/**
 * Resolves DataSource into the list element provider that it references.
 *
 * @return	a pointer to the list element provider indicated by DataSource, or NULL if it couldn't be resolved.
 */
TScriptInterface<IUIListElementProvider> UUIList::ResolveListElementProvider()
{
	TScriptInterface<IUIListElementProvider> Result;

	if ( DataSource.ResolveMarkup( this ) )
	{
		Result = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());
	}

	return Result;
}

/**
 * Loads the data for this list from the data store bound via DataSource.
 *
 * @param	bResolveDataSource	if TRUE, re-resolves DataSource into DataProvider prior to refilling the list's data
 *
 * @return	TRUE if the list data was successfully loaded; FALSE if the data source couldn't be resolved or it didn't
 *			contain the data indicated by SourceData
 */
UBOOL UUIList::RefreshListData( UBOOL bResolveDataSource/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

//	debugf(NAME_DevUI, TEXT(">->->->->->   %s::RefreshListData   Items:%i   Markup:%s"), *GetPathName(), Items.Num(), *DataSource.MarkupString);
	if ( !DataProvider || bResolveDataSource )
	{
		// first, resolve the list element provider from the value of DataSource
		DataProvider = ResolveListElementProvider();
		if ( CellDataComponent != NULL )
		{
			// now refresh the schema for the elements that will be contained by this list.
			CellDataComponent->RefreshElementSchema();
		}
	}

	// first, clear any existing data
	ClearElements();
	if ( DataProvider )
	{
		// now load the data from this element provider
		bResult = PopulateListElements();
	}

//	debugf(NAME_DevUI, TEXT("<-<-<-<-<-<   %s::RefreshListData   Items:%i   Markup:%s   bResult:%i"), *GetPathName(), Items.Num(), *DataSource.MarkupString, bResult);
	return bResult;
}

/**
 * Retrieves the list of elements from the data provider and adds them to the list.
 *
 * @return	TRUE if the list was successfully populated.
 */
UBOOL UUIList::PopulateListElements()
{
	UBOOL bResult = FALSE;

	if ( DataProvider )
	{
		TArray<INT> NewElements;
		if ( DataProvider->GetListElements(DataSource.DataStoreField, NewElements) && NewElements.Num() == 0 && ShouldRenderDataBindings() )
		{
			// if the data provider can't provide any list elements for this list, and we're configured to display the data bindings, use the
			// schema itself as a list element so that we display at least one element of data
			NewElements.AddItem(0);
		}

		InsertElements(NewElements);
		bResult = TRUE;
	}

	return bResult;
}

/** === UIDataSourceSubscriber interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UUIList::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=-1*/ )
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		SetDefaultDataBinding(MarkupText,BindingIndex);
	}
	else if ( BindingIndex == INDEX_NONE || BindingIndex == DataSource.BindingIndex )
	{
		if ( DataSource.MarkupString != MarkupText )
		{
			Modify();
			DataSource.MarkupString = MarkupText;

			if ( IsInitialized() )
			{
				RefreshSubscriberValue(BindingIndex);
			}
		}
	}
}

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUIList::GetDataStoreBinding( INT BindingIndex/*=-1*/ ) const
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return GetDefaultDataBinding(BindingIndex);
	}
	else if ( BindingIndex == INDEX_NONE || BindingIndex == DataSource.BindingIndex )
	{
		return DataSource.MarkupString;
	}

	return TEXT("");
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUIList::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		bResult = TRUE;
	}
	else if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		bResult = ResolveDefaultDataBinding(BindingIndex);
	}
	else if ( BindingIndex == INDEX_NONE || BindingIndex == DataSource.BindingIndex )
	{
		const UBOOL bWasValid = DataSource || Items.Num() > 0;

		eventIncrementAllMutexes();
		if ( RefreshListData(TRUE) && DataSource )
		{
			FUIProviderFieldValue ResolvedValue(EC_EventParm);
			if ( DataSource.GetBindingValue(ResolvedValue) && ResolvedValue.ArrayValue.Num() > 0 )
			{
				eventDecrementAllMutexes();

				INT ListValueIndex = Items.FindItemIndex(ResolvedValue.ArrayValue(0));
				SetIndex(ListValueIndex, TRUE);
				for ( INT ListIndex = 0; ListIndex < Items.Num(); ListIndex++ )
				{
					INT DataSourceIndex = Items(ListIndex);
					UBOOL bShouldBeSelected = ResolvedValue.ArrayValue.FindItemIndex(DataSourceIndex) != INDEX_NONE;

					SelectElement( ListIndex, bShouldBeSelected );
				}

				bResult = TRUE;
			}
			else
			{
				eventDecrementAllMutexes();
				if ( !SetIndex(Index, TRUE) )
				{
// 					SelectElement(Index, TRUE);
				}
			}

			InvalidateAllPositions();
		}
		else
		{
			eventDecrementAllMutexes(bWasValid);
		}
	}

	return bResult;
}

/**
 * Handler for the UIDataStore.OnDataStoreValueUpdated delegate.  Used by data stores to indicate that some data provided by the data
 * has changed.  Subscribers should use this function to refresh any data store values being displayed with the updated value.
 * notify subscribers when they should refresh their values from this data store.
 *
 * @param	SourceDataStore		the data store that generated the refresh notification; useful for subscribers with multiple data store
 *								bindings, to tell which data store sent the notification.
 * @param	PropertyTag			the tag associated with the data field that was updated; Subscribers can use this tag to determine whether
 *								there is any need to refresh their data values.
 * @param	SourceProvider		for data stores which contain nested providers, the provider that contains the data which changed.
 * @param	CollectionIndex		for collection fields, indicates which element was changed.  value of INDEX_NONE indicates not an array
 *								or that the entire array was updated.
 */
void UUIList::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT CollectionIndex )
{
	UBOOL bBoundToDataStore = SourceDataStore == DataSource.ResolvedDataStore;
	if ( bBoundToDataStore && PropertyTag != NAME_None )
	{
		// now we're going to parse off the portion that is bound to the list, then look at arrayindex, etc. to fix false negatives where we're getting
		// updates but think we're not bound to that field....like:
		// Teams;0.Players or
		// Teams;0.Players;0.PlayerName or
		// Players;3.PlayerName
		// etc.
		if ( PropertyTag != DataSource.DataStoreField && CellDataComponent != NULL )
		{
			bBoundToDataStore = FALSE;

			// see if this list is actually bound to the property tag specified
			FString PropertyTagString = PropertyTag.ToString();
			if ( PropertyTagString.InStr(DataSource.DataStoreField.ToString()) != INDEX_NONE )
			{
				INT DelimPos = PropertyTagString.InStr(TEXT("."), TRUE);
				const FString ProviderFieldName = PropertyTagString.Mid(DelimPos+1);

				const INT SchemaCount = CellDataComponent->GetSchemaCellCount();
				for ( INT CellIndex = 0; CellIndex < SchemaCount; CellIndex++ )
				{
					FName CellBinding = CellDataComponent->GetCellBinding(CellIndex);
					if ( CellBinding == *ProviderFieldName )
					{
						bBoundToDataStore = TRUE;
						break;
					}
				}
			}
		}
	}

	LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,CollectionIndex);


// 	TArray<UUIDataStore*> BoundDataStores;
// 	GetBoundDataStores(BoundDataStores);
// 
// 	if (BoundDataStores.ContainsItem(SourceDataStore)
	//@todo ronp - rather than checking SourceDataStore against DataSource, we should call GetBoundDataStores and check whether SourceDataStore is 
	// contained in that array so that cell strings which contain data store markup can be updated from this function....but if the SourceDataStore
	// IS linked through a cell string, the data store will need to pass the correct index
	if ( bBoundToDataStore )
	{
		if ( bValuesInvalidated )
		{
			RefreshSubscriberValue(DataSource.BindingIndex);
		}
		else
		{
			if ( !DataProvider )
			{
				// first, resolve the list element provider from the value of DataSource
				DataProvider = ResolveListElementProvider();
				if ( CellDataComponent != NULL )
				{
					// now refresh the schema for the elements that will be contained by this list.
					CellDataComponent->RefreshElementSchema();
				}
			}

			if ( DataProvider )
			{
				eventDisableSetIndex();

				TArray<INT> NewElements;
				if ( DataProvider->GetListElements(DataSource.DataStoreField, NewElements) )
				{
					if ( NewElements.Num() > 0 )
					{
						const INT OldItemCount = Items.Num();
						for ( INT NewElemIdx = OldItemCount; NewElemIdx < NewElements.Num(); NewElemIdx++ )
						{
							const INT SourceIndex = NewElements(NewElemIdx);
							checkSlow(!Items.ContainsItem(SourceIndex));

							InsertElement(SourceIndex, NewElemIdx, NewElemIdx < NewElements.Num()-1);
						}

						if ( CollectionIndex >= 0 && CollectionIndex < OldItemCount )
						{
							CellDataComponent->RefreshElement(CollectionIndex);
						}
					}
					else
					{
						ClearElements();
					}
				}

				eventEnableSetIndex();
				if ( Items.Num() == 0 && Index != INDEX_NONE )
				{
					SetIndex(INDEX_NONE, FALSE);
				}
				else if ( Items.Num() > 0 && Index == INDEX_NONE )
				{
					INT NewIndex = 0;
					FUIProviderFieldValue ResolvedValue(EC_EventParm);
					if ( DataSource.GetBindingValue(ResolvedValue) && ResolvedValue.ArrayValue.Num() > 0 )
					{
						NewIndex = ResolvedValue.ArrayValue(0);
					}

					if ( !SetIndex(NewIndex) )
					{
						SelectElement(NewIndex, TRUE);
					}
				}
			}
		}
	}
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UUIList::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	GetDefaultDataStores(out_BoundDataStores);

	// add overall data store to the list
	if ( DataSource )
	{
		out_BoundDataStores.AddUniqueItem(*DataSource);
	}

	// add any data stores from our cell strings
	if ( CellDataComponent != NULL )
	{
		CellDataComponent->GetBoundDataStores(out_BoundDataStores);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUIList::ClearBoundDataStores()
{
	TMultiMap<FName,FUIDataStoreBinding*> DataBindingMap;
	GetDataBindings(DataBindingMap);

	TArray<FUIDataStoreBinding*> DataBindings;
	DataBindingMap.GenerateValueArray(DataBindings);
	for ( INT BindingIndex = 0; BindingIndex < DataBindings.Num(); BindingIndex++ )
	{
		FUIDataStoreBinding* Binding = DataBindings(BindingIndex);
		Binding->ClearDataBinding();
	}

	TArray<UUIDataStore*> DataStores;
	GetBoundDataStores(DataStores);

	for ( INT DataStoreIndex = 0; DataStoreIndex < DataStores.Num(); DataStoreIndex++ )
	{
		UUIDataStore* DataStore = DataStores(DataStoreIndex);
		DataStore->eventSubscriberDetached(this);
	}
}

/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL UUIList::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex/*=INDEX_NONE*/ )
{
	FUIProviderScriptFieldValue SelectedItems(EC_EventParm);
	SelectedItems.PropertyTag = DataSource.DataStoreField;
	SelectedItems.PropertyType = DATATYPE_Collection;
	SelectedItems.ArrayValue = GetSelectedItems();

	GetBoundDataStores(out_BoundDataStores);
	return DataSource.SetBindingValue(SelectedItems);
}

/**
 * Evaluates the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUIList::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( CellDataComponent != NULL )
	{
		CellDataComponent->ResolveFacePosition(Face);
	}
}

/**
 * Render this list.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUIList::Render_Widget( FCanvas* Canvas )
{
	if ( CellDataComponent != NULL )
	{
		CellDataComponent->Render_List(Canvas);
	}
}
	
/**
 * Renders the list's background image, if assigned.
 */
void UUIList::RenderBackgroundImage( FCanvas* Canvas, const FRenderParameters& Parameters )
{
	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->RenderComponent(Canvas, Parameters);
	}
}


/**
 * Called from UGameUISceneClient::UpdateMousePosition; provides a hook for widgets to respond to the precise cursor
 * position.  Only called on the scene's ActiveControl if the ActiveControl's bEnableActiveCursorUpdates is TRUE and
 * the mouse is currently over the widget.
 *
 * This version ensures that the element under the mouse is in the proper cell state, if bUpdateItemUnderCursor is true.
 */
void UUIList::NotifyMouseOver( const FVector2D& MousePos )
{
	// bUpdateItemUnderCursor should always be TRUE if we're receiving this function call, but it can't hurt to be sure
	if ( bUpdateItemUnderCursor && CellDataComponent != NULL )
	{
		INT MouseOverIndex = CalculateIndexFromCursorLocation(TRUE);
		if ( MouseOverIndex != INDEX_NONE )
		{
			const INT LastVisibleElement = Min(GetItemCount(), TopIndex + GetMaxVisibleElementCount());
			for ( INT ItemIndex = TopIndex; ItemIndex < LastVisibleElement; ItemIndex++ )
			{
				if ( ItemIndex != MouseOverIndex )
				{
					if ( GetElementCellState(ItemIndex) == ELEMENT_UnderCursor )
					{
						// by default, the element will assume the normal state
						EUIListElementState NewElementState=ELEMENT_Normal;

						// but if this element is the same as the list's Index, it should be in the active state
						if ( Index == ItemIndex )
						{
							NewElementState = ELEMENT_Active;
						}

						// selected supercedes Active, so if this item is both the active and selected item,
						// use the selected state's style.
						if ( SelectedItems.ContainsItem(Items(ItemIndex)) )
						{
							NewElementState = ELEMENT_Selected;
						}
						
						SetElementCellState(ItemIndex, NewElementState);
					}
				}
				else if ( !SelectedItems.ContainsItem(Items(ItemIndex)) || bHoverStateOverridesSelected )
				{
					SetElementCellState(ItemIndex, ELEMENT_UnderCursor);
				}
			}
		}
	}
}

/**
 * Change the value of bEnableActiveCursorUpdates to the specified value.
 */
void UUIList::SetActiveCursorUpdate( UBOOL bShouldReceiveCursorUpdates )
{
	Super::SetActiveCursorUpdate(bShouldReceiveCursorUpdates);
	bUpdateItemUnderCursor = bShouldReceiveCursorUpdates;

	if ( CellDataComponent != NULL && !bUpdateItemUnderCursor )
	{
		// if the item currently under the cursor was in the hover state and we disabled
		// cursor tracking, put it back into the normal (or whichever is appropriate) state
		INT MouseOverIndex = CalculateIndexFromCursorLocation(TRUE);
		if ( MouseOverIndex != INDEX_NONE )
		{
			// by default, the element will assume the normal state
			EUIListElementState NewElementState=ELEMENT_Normal;

			// but if this element is the same as the list's Index, it should be in the active state
			if ( Index == MouseOverIndex )
			{
				NewElementState = ELEMENT_Active;
			}

			// selected supersedes Active, so if this item is both the active and selected item,
			// use the selected state's style.
			if ( SelectedItems.ContainsItem(Items(MouseOverIndex)) )
			{
				NewElementState = ELEMENT_Selected;
			}

			SetElementCellState(MouseOverIndex, NewElementState);
		}
	}
}


/**
 * Called when a style reference is resolved successfully.
 *
 * @param	ResolvedStyle			the style resolved by the style reference
 * @param	StyleProperty			the name of the style reference property that was resolved.
 * @param	ArrayIndex				the array index of the style reference that was resolved.  should only be >0 for style reference arrays.
 * @param	bInvalidateStyleData	if TRUE, the resolved style is different than the style that was previously resolved by this style reference.
 */
void UUIList::OnStyleResolved( UUIStyle* ResolvedStyle, const FStyleReferenceId& StylePropertyId, INT ArrayIndex, UBOOL bInvalidateStyleData )
{
	Super::OnStyleResolved(ResolvedStyle,StylePropertyId,ArrayIndex,bInvalidateStyleData);

	if ( CellDataComponent != NULL )
	{
		FString StylePropertyName = StylePropertyId.GetStyleReferenceName();
		if ( StylePropertyName == TEXT("GlobalCellStyle") )
		{
			if ( ArrayIndex >= 0 && ArrayIndex < ELEMENT_MAX )
			{
				EUIListElementState ElementState = (EUIListElementState)ArrayIndex;

				// if our previous style is the same as the current style, then we don't need to invalidate
				// the cached styles in all of our element cells - just have them update according to the current menu state
				if ( bInvalidateStyleData == FALSE )
				{
					CellDataComponent->OnListMenuStateChanged(ElementState);
				}
				else
				{
					CellDataComponent->OnListStyleChanged(ElementState);
				}
			}
		}
		else if ( StylePropertyName == TEXT("ColumnHeaderStyle") )
		{
			CellDataComponent->ApplyColumnHeaderStyle(ResolvedStyle);
		}
		else if ( StylePropertyName == TEXT("ColumnHeaderBackgroundStyle") )
		{
			UUIComp_ListPresenter* ReportPresenter = Cast<UUIComp_ListPresenter>(CellDataComponent);
			if ( ReportPresenter != NULL && ArrayIndex >= 0 && ArrayIndex < COLUMNHEADER_MAX )
			{
				UUIStyle_Image* HeaderBackgroundStyle = Cast<UUIStyle_Image>(ColumnHeaderBackgroundStyle[ArrayIndex].GetStyleData(GetCurrentState()));
				if ( HeaderBackgroundStyle != NULL )
				{
					if (ReportPresenter->ColumnHeaderBackground[ArrayIndex] == NULL
					&&	!(ColumnHeaderBackgroundStyle[ArrayIndex].IsDefaultStyle(ResolvedStyle)))
					{
						// passing NULL for the image will create the UITexture that is required in order to render the style's default image
						ReportPresenter->SetImage(&ReportPresenter->ColumnHeaderBackground[ArrayIndex],NULL);
					}
				}
			}
		}
		else if ( StylePropertyName == TEXT("ItemOverlayStyle") )
		{
			UUIComp_ListPresenter* ReportPresenter = Cast<UUIComp_ListPresenter>(CellDataComponent);
			if ( ReportPresenter != NULL && ArrayIndex >= 0 && ArrayIndex < ELEMENT_MAX )
			{
				UUIStyle_Image* OverlayStyle = Cast<UUIStyle_Image>(ItemOverlayStyle[ArrayIndex].GetStyleData(GetCurrentState()));
				if ( OverlayStyle != NULL )
				{
					//@todo list refactor - might need this for all lists...
					if ( ReportPresenter->ListItemOverlay[ArrayIndex] == NULL )
					{
						// passing NULL for the image will create the UITexture that is required in order to render the style's default image
						CellDataComponent->SetImage(&ReportPresenter->ListItemOverlay[ArrayIndex], NULL);
					}
					ReportPresenter->ListItemOverlay[ArrayIndex]->SetImageStyle(OverlayStyle);
				}
			}
		}
	}
}

/**
 * Applies the value of bShouldBeDirty to the current style data for all style references in this widget.  Used to force
 * updating of style data.
 *
 * @param	bShouldBeDirty	the value to use for marking the style data for the specified menu state of all style references
 *							in this widget as dirty.
 * @param	MenuState		if specified, the style data for that menu state will be modified; otherwise, uses the widget's current
 *							menu state
 */
void UUIList::ToggleStyleDirtiness( UBOOL bShouldBeDirty, UUIState* MenuState/*=NULL*/ )
{
	Super::ToggleStyleDirtiness(bShouldBeDirty, MenuState);

	if ( CellDataComponent != NULL )
	{
		UUIState* CurrentMenuState = MenuState;
		if ( CurrentMenuState == NULL )
		{
			CurrentMenuState = GetCurrentState();
		}
		check(CurrentMenuState);

		CellDataComponent->ToggleStyleDirtiness(bShouldBeDirty, CurrentMenuState);
	}
}

/**
 * Determines whether this widget references the specified style.
 *
 * @param	CheckStyle		the style to check for referencers
 */
UBOOL UUIList::UsesStyle( UUIStyle* CheckStyle )
{
	UBOOL bResult = Super::UsesStyle(CheckStyle);
	if ( !bResult && CellDataComponent != NULL )
	{
		bResult = CellDataComponent->UsesStyle(CheckStyle);
	}

	return bResult;
}

/**
 * Determines whether elements should render the names of the fields they're bound to.
 *
 * @return	TRUE if list elements should render the names for the data fields they're bound to, FALSE
 *			if list elements should render the actual data for the list element they're associated with.
 */
UBOOL UUIList::ShouldRenderDataBindings() const
{
	UBOOL bResult = bDisplayDataBindings;
	if ( GIsGame )
	{
		bResult = bResult && GetDefaultUIController()->GetDefaultSceneClient()->bRenderDebugInfo;
	}
	
	return bResult;
}

/**
 * Returns whether element size is determined by the elements themselves.  For lists with linked columns, returns whether
 * the item height is autosized; for lists with linked rows, returns whether item width is autosized.
 */
UBOOL UUIList::IsElementAutoSizingEnabled() const
{
	UBOOL bResult = FALSE;

	switch ( CellLinkType )
	{
	case LINKED_None:
		bResult =	ColumnAutoSizeMode	== CELLAUTOSIZE_Constrain || ColumnAutoSizeMode	== CELLAUTOSIZE_AdjustList
				||	RowAutoSizeMode		== CELLAUTOSIZE_Constrain || RowAutoSizeMode	== CELLAUTOSIZE_AdjustList;
		break;

	case LINKED_Rows:
		bResult = ColumnAutoSizeMode == CELLAUTOSIZE_Constrain || ColumnAutoSizeMode == CELLAUTOSIZE_AdjustList;
		break;

	case LINKED_Columns:
		bResult = RowAutoSizeMode == CELLAUTOSIZE_Constrain || RowAutoSizeMode == CELLAUTOSIZE_AdjustList;
		break;
	}

	return bResult;
}

/* === UUIScreenObject interface === */
/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * Initializes the value of bDisplayDataBindings based on whether we're in the game or not.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUIList::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	// construct the vertical scrollbar if it doesn't exist yet
	UUIScrollbar* VertScrollbarTemplate = VerticalScrollbar->GetArchetype<UUIScrollbar>();
	if ( VerticalScrollbar == NULL )
	{
		VerticalScrollbar = Cast<UUIScrollbar>(CreateWidget(this, UUIScrollbar::StaticClass(), VertScrollbarTemplate, TEXT("VertScrollbar")));
		VerticalScrollbar->ScrollbarOrientation = UIORIENT_Vertical;
		
		InsertChild( VerticalScrollbar );
	}
	else if ( VerticalScrollbar != NULL )
	{
		VerticalScrollbar->SetArchetype(VertScrollbarTemplate);
	}

	Super::Initialize(inOwnerScene, inOwner);

	if ( SortComponent != NULL )
	{
		SortComponent->ResetSortColumns(FALSE);
	}
}

/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIList::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(BackgroundImageComponent);
	AddStyleSubscriber(BackgroundImageComponent);
}

/**
 * Initializes the vertical and horizontal scrollbars for the current state of the List
 */
void UUIList::InitializeScrollbars()
{
	bInitializeScrollbars = FALSE;

	// Initialize the Vertical Scrollbar values
	const INT TotalItems = GetItemCount();
	const INT VisibleItems = GetMaxVisibleElementCount();
	const INT NumHiddenItems = TotalItems - VisibleItems;

	const FLOAT HeaderSize = GetItemCount() > 0 ? GetHeaderSize() : 0.f;
	VerticalScrollbar->SetDockParameters(UIFACE_Top, this, UIFACE_Top, HeaderSize);
	VerticalScrollbar->SetDockTarget(UIFACE_Bottom, this, UIFACE_Bottom);

	if ( NumHiddenItems > 0 && bEnableVerticalScrollbar )
	{
		// Initialize marker size to be proportional to the visible area of the list
		FLOAT MarkerSize = (FLOAT)VisibleItems / TotalItems;
		VerticalScrollbar->SetMarkerSize( MarkerSize );

		// Show the vertical scrollbar since the list has elements which aren't visible.
		VerticalScrollbar->eventSetVisibility(TRUE);
		
		// Initialize the nudge value to be the size of one item
		VerticalScrollbar->SetNudgeSizePercent( 1.f / TotalItems );

		// Since we do not have a horizontal scrollbar yet, we can disable the corner padding
		//@fixme horz scrollbar.
		VerticalScrollbar->EnableCornerPadding(FALSE);

		// Initialize the marker position to reflect the position of the topmost item in the list
		UpdateScrollbars();
	}
	else
	{
		// Default Initialization
		VerticalScrollbar->SetMarkerSize( 1.0f );
		VerticalScrollbar->SetNudgeSizePercent( 0.0f );
		VerticalScrollbar->eventSetVisibility(FALSE);
		UpdateScrollbars();
	}
}


/**
 * Sets up the positions of scrollbar markers according to which items are currently visible
 */
void UUIList::UpdateScrollbars()
{
	// Setup the position of the vertical scrollbar marker according to the current top index of the list
	const INT ItemCount = GetItemCount();
	const INT VisibleElements = GetMaxVisibleElementCount();
	const INT NumHiddenItems = ItemCount - VisibleElements;
	if ( NumHiddenItems > 0 && VisibleElements > 0 )
	{
		// Initialize the marker position
		VerticalScrollbar->SetMarkerPosition( (FLOAT)TopIndex / NumHiddenItems );
	}
	else
	{
		// in this case, the scrollbar shouldn't even be visible since the number of items is less than the number of visible items
		VerticalScrollbar->SetMarkerPosition( 0.f );
	}
}

/**
 * Handler for vertical scrolling activity
 * PositionChange should be a number of nudge values by which the slider was moved
 * The nudge value in the UIList slider is equal to one list Item.
 *
 * @param	Sender			the scrollbar that generated the event.
 * @param	PositionChange	indicates how many items to scroll the list by
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          unused in this function
 */
UBOOL UUIList::ScrollVertical( UUIScrollbar* Sender, FLOAT PositionChange, UBOOL bPositionMaxed/*=FALSE*/)
{
	UBOOL bResult = FALSE;

	if ( Sender != NULL )
	{
		// Scrolling the list just changes the value of the list's TopIndex
		INT NewTopIndex=INDEX_NONE;
		if ( bPositionMaxed )
		{
			// When position flag is set we need to manually specify the top index to be either the first list item
			// or first visible item when visible region is at the very bottom
			if ( PositionChange < 0 )
			{	
				NewTopIndex = 0;
			}
			else
			{
				NewTopIndex = GetItemCount() - GetMaxNumVisibleRows();
			}
		}
		else
		{
			NewTopIndex = TopIndex + appRound(PositionChange);
		}

		if ( NewTopIndex != INDEX_NONE )
		{
			bResult = SetTopIndex(NewTopIndex, TRUE);
			if ( bResult )
			{
				// Force cell reformatting
				RefreshFormatting();
			}
		}
	}

	return bResult;
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUIList::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames )
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem(UIKEY_SubmitListSelection);
	out_KeyNames.AddItem(UIKEY_MoveSelectionUp);
	out_KeyNames.AddItem(UIKEY_MoveSelectionDown);
	out_KeyNames.AddItem(UIKEY_MoveSelectionLeft);
	out_KeyNames.AddItem(UIKEY_MoveSelectionRight);
	out_KeyNames.AddItem(UIKEY_SelectFirstElement);
	out_KeyNames.AddItem(UIKEY_SelectLastElement);
	out_KeyNames.AddItem(UIKEY_SelectAllItems);
	out_KeyNames.AddItem(UIKEY_PageUp);
	out_KeyNames.AddItem(UIKEY_PageDown);
	out_KeyNames.AddItem(UIKEY_Clicked);
	out_KeyNames.AddItem(UIKEY_ResizeColumn);
}

/**
 * Called when a property is modified that could potentially affect the widget's position onscreen.
 */
void UUIList::RefreshPosition()
{
	Super::RefreshPosition();

	RefreshFormatting();
}

/**
 * Called to globally update the formatting of all UIStrings.
 */
void UUIList::RefreshFormatting( UBOOL bRequestSceneUpdate/*=TRUE*/ )
{
	Super::RefreshFormatting(bRequestSceneUpdate);

	bInitializeScrollbars = TRUE;
	if ( CellDataComponent )
	{
		CellDataComponent->ReapplyFormatting(bRequestSceneUpdate);
	}
}

/**
 * Removes the specified state from the screen object's state stack.
 *
 * @param	StateToRemove	the state to be removed
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 *
 * @return	TRUE if the state was successfully removed, or if the state didn't exist in the widget's list of states;
 *			false if the state overrode the request to be removed
 */
UBOOL UUIList::DeactivateState( UUIState* StateToRemove, INT PlayerIndex )
{
	UBOOL bResult = Super::DeactivateState(StateToRemove,PlayerIndex);
	if ( bResult && StateToRemove->IsA(UUIState_Active::StaticClass()) )
	{
		// make sure to restore the mouse cursor in case the user moved the mouse outside of our bounds
		// while the mouse was hovering over a column boundary
		UGameUISceneClient* GameSceneClient = GetSceneClient();
		if ( GameSceneClient != NULL )
		{
			GameSceneClient->ChangeMouseCursor(TEXT("Arrow"));
		}
	}

	return bResult;
}


/**
 * Handles input events for this list.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUIList::ProcessInputKey( const FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	// Pressed Event
	if ( EventParms.InputAliasName == UIKEY_Clicked )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_DoubleClick )
		{
			bSortingList = FALSE;

			// notify unrealscript
			if ( DELEGATE_IS_SET(OnPressed) )
			{
				delegateOnPressed(this, EventParms.PlayerIndex);
			}

			if ( EventParms.EventType == IE_DoubleClick && DELEGATE_IS_SET(OnDoubleClick) )
			{
				delegateOnDoubleClick(this, EventParms.PlayerIndex);
			}

			// activate the pressed state
			ActivateStateByClass(UUIState_Pressed::StaticClass(),EventParms.PlayerIndex);

			if ( ShouldRenderColumnHeaders() )
			{
				if ( SortComponent != NULL )
				{
					//@todo ronp - might also be benificial to add a separate input alias to trigger a list sort, for use on consoles
					FCellHitDetectionInfo CellHitData(EC_EventParm);
					ResizeColumn = GetResizeColumn(&CellHitData);
					if ( ResizeColumn == INDEX_NONE && CellHitData.HitRow == INDEX_NONE && CellHitData.HitColumn != INDEX_NONE )
					{
						bSortingList = TRUE;

						// user clicked on a column header - sort this column
						//@fixme ronp - for now, we'll hardcode secondary sorting to the Shift key
						//@fixme ronp - for now, we'll hardcode case-sensitivity to the Alt key
						if ( SortComponent->SortItems(CellHitData.HitColumn, EventParms.bShiftPressed, EventParms.bAltPressed) )
						{
							//@todo - play ascending/descending sort sound cue
							RefreshFormatting();
						}
					}
				}
				else
				{
					ResizeColumn = GetResizeColumn();
				}
			}

			if ( EventParms.EventType == IE_DoubleClick )
			{
				ActivateEventByClass(EventParms.PlayerIndex, UUIEvent_OnDoubleClick::StaticClass(), this);
				if ( ResizeColumn == INDEX_NONE && !bSortingList )
				{
					if ( !bSingleClickSubmission )
					{
						NotifySubmitSelection(EventParms.PlayerIndex);
					}
				}
			}

			bResult = TRUE;
		}
		else if ( EventParms.EventType == IE_Repeat )
		{
			if ( !bSortingList && ResizeColumn == INDEX_NONE )
			{
				// Play the ClickedCue
				//PlayUISound(ClickedCue,EventParms.PlayerIndex);
				if ( DELEGATE_IS_SET(OnPressRepeat) )
				{
					delegateOnPressRepeat(this, EventParms.PlayerIndex);
				}
			}

			bResult = TRUE;
		}
		else if ( EventParms.EventType == IE_Released )
		{
			if ( DELEGATE_IS_SET(OnPressRelease) )
			{
				delegateOnPressRelease(this, EventParms.PlayerIndex);
			}

			if ( IsPressed(EventParms.PlayerIndex) )
			{
				// if we're not resizing or clicking a column, trigger a click event
				if ( ResizeColumn == INDEX_NONE && !bSortingList )
				{
					FVector2D MousePos(0,0);
					const UBOOL bCursorSelection = IsCursorInputKey(EventParms.InputKeyName);
					if ( !bCursorSelection || !GetCursorPosition(MousePos, GetScene()) || ContainsPoint(MousePos) )
					{
						if ( !DELEGATE_IS_SET(OnClicked) || !delegateOnClicked(this, EventParms.PlayerIndex) )
						{
							if ( bCursorSelection )
							{
								// if the user used the cursor the select this element, update the list's index
								INT ClickedIndex = CalculateIndexFromCursorLocation();
								if ( ClickedIndex != INDEX_NONE )
								{
									SetIndex(ClickedIndex);
								}
							}

							// activate the on click event
							ActivateEventByClass(EventParms.PlayerIndex,UUIEvent_OnClick::StaticClass(), this);
						}

						if ( bSingleClickSubmission || !bCursorSelection )
						{
							NotifySubmitSelection(EventParms.PlayerIndex);
						}
					}
				}

				// done resizing.
				ResizeColumn = INDEX_NONE;
				// done sorting
				bSortingList = FALSE;

				// deactivate the pressed state
				DeactivateStateByClass(UUIState_Pressed::StaticClass(),EventParms.PlayerIndex);
			}

			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_SubmitListSelection )
	{
		if ( EventParms.EventType == IE_Released )
		{
			FVector2D MousePos(0,0);				
			if (!IsCursorInputKey(EventParms.InputKeyName)
			|| !GetCursorPosition(MousePos, GetScene())
			||	ContainsPoint(MousePos))
			{
				// only fire a submitselection notification when the button is released
				NotifySubmitSelection(EventParms.PlayerIndex);
			}
		}
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_MoveSelectionLeft )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			const INT OldIndex = Index;
			if ( NavigateIndex(FALSE, FALSE, TRUE) && OldIndex != Index )
			{
				bResult = TRUE;
				PlayUISound(DecrementIndexCue,EventParms.PlayerIndex);
			}

			// unless our CellLinkType is LINKED_Columns, swallow this input event so that the user doesn't accidentally
			// switch focus on the PC
			bResult = CellLinkType != LINKED_Columns;
		}
		else
		{
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_MoveSelectionRight )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			const INT OldIndex = Index;
			if ( NavigateIndex(TRUE, FALSE, TRUE) && OldIndex != Index )
			{
				bResult = TRUE;
				PlayUISound(IncrementIndexCue,EventParms.PlayerIndex);
			}

			// unless our CellLinkType is LINKED_Columns, swallow this input event so that the user doesn't accidentally
			// switch focus on the PC
			bResult = CellLinkType != LINKED_Columns;
		}
		else
		{
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_MoveSelectionUp )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			const INT OldIndex = Index;
			if ( NavigateIndex(FALSE, FALSE, FALSE) && OldIndex != Index )
			{
				PlayUISound(DecrementIndexCue,EventParms.PlayerIndex);
			}

			// unless our CellLinkType is LINKED_Rows, swallow this input event so that the user doesn't accidentally
			// switch focus on the PC
			bResult = CellLinkType != LINKED_Rows;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_MoveSelectionDown )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			const INT OldIndex = Index;
			if ( NavigateIndex(TRUE, FALSE, FALSE) && OldIndex != Index )
			{
				bResult = TRUE;
				PlayUISound(IncrementIndexCue,EventParms.PlayerIndex);
			}

			// unless our CellLinkType is LINKED_Rows, swallow this input event so that the user doesn't accidentally
			// switch focus on the PC
			bResult = CellLinkType != LINKED_Rows;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_PageUp )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			const INT OldIndex = Index;
			if ( NavigateIndex(FALSE, TRUE, FALSE) && OldIndex != Index )
			{
				PlayUISound(DecrementIndexCue,EventParms.PlayerIndex);
			}

			bResult = TRUE;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_PageDown )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			const INT OldIndex = Index;	
			if ( NavigateIndex(TRUE, TRUE, FALSE) && OldIndex != Index )
			{
				PlayUISound(IncrementIndexCue,EventParms.PlayerIndex);
			}

			bResult = TRUE;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_SelectFirstElement )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			if ( GetItemCount() > 1 && Index > 0 )
			{
				SetIndex(0);
				PlayUISound(DecrementIndexCue,EventParms.PlayerIndex);
			}

			bResult = TRUE;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_SelectLastElement )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			if ( GetItemCount() > 1 && Index < GetItemCount() - 1 )
			{
				SetIndex(GetItemCount() - 1);
				PlayUISound(IncrementIndexCue,EventParms.PlayerIndex);
			}

			bResult = TRUE;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_SelectAllItems )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			// select all items when the user presses Ctrl+A
			if ( bEnableMultiSelect == TRUE )
			{
				for ( INT ItemIndex = 0; ItemIndex < GetItemCount(); ItemIndex++ )
				{
					SelectElement(ItemIndex, TRUE);
				}
			}

			bResult = TRUE;
		}
		else
		{
			// swallow IE_Released
			bResult = TRUE;
		}
	}

	// Make sure to call the superclass's implementation after trying to consume input ourselves so that
	// we can respond to events defined in the super's class.
	bResult = bResult || Super::ProcessInputKey(EventParms);
	return bResult;
}

/**
 * Processes input axis movement. Only called while the list is in the pressed state; resizes a column if ResizeColumn
 * is a valid value.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUIList::ProcessInputAxis( const FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	if ( EventParms.InputAliasName == UIKEY_ResizeColumn && CellLinkType != LINKED_None )
	{
		if (CellDataComponent!= NULL
		&&	CellDataComponent->IsValidSchemaIndex(ResizeColumn) )
		{
			FVector2D MousePos2D;
			if ( GetCursorPosition(MousePos2D, GetScene()) )
			{
				const FVector MousePos = PixelToCanvas(MousePos2D);

// 				FUIListElementCellTemplate& SchemaCell = ReportPresenter->ElementSchema.Cells(ResizeColumn);

				const FLOAT PreviousCellSize = CellDataComponent->GetSchemaCellSize(ResizeColumn);
				const FLOAT ResizeDelta = CellDataComponent->GetSchemaCellPosition(ResizeColumn) + PreviousCellSize - (CellLinkType == LINKED_Columns ? MousePos.X : MousePos.Y);
				const FLOAT NewCellSize = PreviousCellSize - ResizeDelta;
				const FLOAT MinColumnPixels = MinColumnSize.GetValue(this);

				UBOOL bValidResize = TRUE, bReapplyFormatting=FALSE;
				if ( CellDataComponent->IsValidSchemaIndex(ResizeColumn + 1) )
				{
// 					FUIListElementCellTemplate& NextSchemaCell = ReportPresenter->ElementSchema.Cells(ResizeColumn + 1);
					const FLOAT NextCellSize = CellDataComponent->GetSchemaCellSize(ResizeColumn + 1) + ResizeDelta;
					if ( NextCellSize <= MinColumnPixels )
					{
						// this would cause the column to overlap the one next to it - bail
						bValidResize = FALSE;
					}
					else if ( NewCellSize > MinColumnPixels )
					{
						bReapplyFormatting = CellDataComponent->SetSchemaCellSize(ResizeColumn + 1, Max(MinColumnPixels, NextCellSize));
					}
				}

				if ( bValidResize )
				{
					bReapplyFormatting = CellDataComponent->SetSchemaCellSize(ResizeColumn, Max(MinColumnPixels, NewCellSize));
				}

				if ( bReapplyFormatting )
				{
					CellDataComponent->ReapplyFormatting();
				}
			}

			bResult = TRUE;
		}
		else
		{
			UGameUISceneClient* GameSceneClient = GetSceneClient();
			if ( GameSceneClient != NULL )
			{
				INT MouseOverColumn = GetResizeColumn();
				if ( MouseOverColumn != INDEX_NONE )
				{
					GameSceneClient->ChangeMouseCursor(TEXT("SplitterHorz"));
				}
				else
				{
					GameSceneClient->ChangeMouseCursor(TEXT("Arrow"));
				}
			}
		}
	}

	return bResult;
}

/**
 * Calculates the row/column location of the cursor.  
 *
 * @param	HitLocation		the point to use for calculating the hit information.
 * @param	out_HitInfo		receives the results of the calculation.  The row/column that was hit does not necessarily
 *							correspond to an actual item in the list (i.e. the row/column may be higher than the actual
 *							number of rows or columns).
 *
 * @return	TRUE if HitLocation was located inside this list.
 */
UBOOL UUIList::CalculateCellFromPosition( const FIntPoint& HitLocation, FCellHitDetectionInfo& out_HitInfo ) const
{
	UBOOL bResult = FALSE;
	
	// only relevant if the hit location is within the bounds of the list
	if ( ContainsPoint(FVector2D(HitLocation)) )
	{
		bResult = TRUE;

		const FVector TransformedHitLocation = PixelToCanvas(HitLocation);
		const FLOAT Spacing = CellSpacing.GetValue(this);

		// initialize all values to -1
		out_HitInfo.HitColumn = out_HitInfo.HitRow = out_HitInfo.ResizeColumn = out_HitInfo.ResizeRow = INDEX_NONE;
		UBOOL bFoundColumn = FALSE, bFoundRow = FALSE, bFoundResizeColumn = FALSE, bFoundResizeRow = FALSE;

		FRenderParameters Parms;
		CellDataComponent->InitializeRenderingParms(Parms);

		// start at the left face
		FLOAT CurrentX = Parms.DrawX;
		const INT NumColumns = GetMaxNumVisibleColumns();
		for ( INT ColIndex = 0; ColIndex < NumColumns && (!bFoundColumn || !bFoundResizeColumn); ColIndex++ )
		{
			// increment the current position by the width of this colum
			const FLOAT ColumnWidth = GetColumnWidth(ColIndex);
			CurrentX += ColumnWidth;

			// if the hit location is less that the position of the right edge of this column, this is the column that was clicked on
			if ( !bFoundColumn && TransformedHitLocation.X <= CurrentX )
			{
				out_HitInfo.HitColumn = ColIndex;
				bFoundColumn = TRUE;
			}

			// if the hit location is within UCONST_ResizeBufferPixels of the column boundary, consider the hit to be
			// within the area used for resizing the column
			if ( !bFoundResizeColumn )
			{
				if ( Abs<FLOAT>(TransformedHitLocation.X - CurrentX) <= UCONST_ResizeBufferPixels )
				{
					out_HitInfo.ResizeColumn = ColIndex;
					bFoundResizeColumn = TRUE;
				}
				else if ( TransformedHitLocation.X < CurrentX )
				{
					// we've passed the last column that could have possibly been resized considering the mouse position,
					// so we can stop checking this now
					bFoundResizeColumn = TRUE;
				}
			}

			if ( CellLinkType != LINKED_Columns )
			{
				CurrentX += Spacing;
			}
		}

		// start at the top face
		FLOAT CurrentY = Parms.DrawY;
		if ( ShouldRenderColumnHeaders() )
		{
			CurrentY += GetRowHeight(INDEX_NONE, TRUE);
			if ( TransformedHitLocation.Y <= CurrentY )
			{
				// if the user clicked on a column header, HitRow/ResizeRow should remain INDEX_NONE
				bFoundRow = bFoundResizeRow = TRUE;
			}
		}

		CurrentY += HeaderElementSpacing.GetValue(this);

		const INT NumRows = GetMaxNumVisibleRows();
		for ( INT RowIndex = 0; RowIndex < NumRows && (!bFoundRow || !bFoundResizeRow); RowIndex++ )
		{
			// increment the current position by the height of this row
			CurrentY += GetRowHeight(RowIndex);

			// if the hit location is less that the position of the bottom edge of this row, this is the row that was clicked on
			if ( !bFoundRow && TransformedHitLocation.Y <= CurrentY )
			{
				out_HitInfo.HitRow = RowIndex;
				bFoundRow = TRUE;
			}

			// if the hit location is within UCONST_ResizeBufferPixels of the row boundary, consider the hit to be
			// within the area used for resizing the row
			if ( !bFoundResizeRow )
			{
				if ( Abs<FLOAT>(TransformedHitLocation.Y - CurrentY) <= UCONST_ResizeBufferPixels )
				{
					out_HitInfo.ResizeRow = RowIndex;
					bFoundResizeRow = TRUE;
				}
				else if ( TransformedHitLocation.Y < CurrentY )
				{
					// we've passed the last row that could have possibly been resized considering the mouse position,
					// so we can stop checking this now
					bFoundResizeRow = TRUE;
				}
			}

			if ( CellLinkType != LINKED_Rows )
			{
				CurrentY += Spacing;
			}
		}
	}
	return bResult;
}

/**
 * Calculates the index of the element under the mouse or joystick cursor
 *
 * @return	the index [into the Items array] for the element under the mouse/joystick cursor, or INDEX_NONE if the mouse is not
 *			over a valid element.
 */
INT UUIList::CalculateIndexFromCursorLocation( UBOOL bRequireValidIndex/*=TRUE*/ ) const
{
	INT Result = INDEX_NONE;

	if ( GetItemCount() > 0 )
	{
		// grab the current position of the cursor
		UGameUISceneClient* SceneClient = GetSceneClient();
		if ( SceneClient != NULL )
		{
			// determine which cell was clicked on
			FCellHitDetectionInfo ClickedCell;
			if ( CalculateCellFromPosition(SceneClient->MousePosition, ClickedCell) && ClickedCell.HitRow != INDEX_NONE )
			{
				// extrapolate the index corresonding to this cell based on the CellLinkType of the list
				switch ( CellLinkType )
				{
				case LINKED_None:
					Result = TopIndex + ClickedCell.HitColumn + (ClickedCell.HitRow * GetMaxNumVisibleColumns());
					break;

				case LINKED_Rows:
					Result = TopIndex + ClickedCell.HitColumn;
					break;

				case LINKED_Columns:
					Result = TopIndex + ClickedCell.HitRow;
					break;
				}

				// if no valid item was clicked on, set the return value to INDEX_NONE
				if ( bRequireValidIndex && !Items.IsValidIndex(Result) )
				{
					Result = INDEX_NONE;
				}
			}
		}
	}

	return Result;
}

/**
 * If the mouse is over a column boundary, returns the index of the column that would be resized, or INDEX_NONE if the mouse is not
 * hovering over a column boundary.
 *
 * @param	CellHitDetectionInfo	will be filled with information about which cells the cursor is currently over
 *
 * @return	if the cursor is within ResizeBufferPixels of a column boundary, the index of the column the left of the cursor; INDEX_NONE
 *			otherwise.
 */
INT UUIList::GetResizeColumn( FCellHitDetectionInfo* CellHitDetectionInfo/*=NULL*/ ) const
{
	INT Result = INDEX_NONE;

	FCellHitDetectionInfo ClickedCell;
	FIntPoint MousePosition;
	if (GetCursorPosition(MousePosition.X, MousePosition.Y,GetScene())
	&&	CalculateCellFromPosition(MousePosition, ClickedCell))
	{
		// never allow the user to resize the last column
		if ( ClickedCell.HitRow == INDEX_NONE && ClickedCell.ResizeColumn != GetTotalColumnCount() - 1
		&&	ColumnAutoSizeMode == CELLAUTOSIZE_None )
		{
			Result = ClickedCell.ResizeColumn;
		}

		if ( CellHitDetectionInfo != NULL )
		{
			*CellHitDetectionInfo = ClickedCell;
		}
	}

	return Result;
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIList::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
				{
					// the user either cleared the value of the BackgroundImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(BackgroundImageComponent);
				}
			}
		}
	}
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void UUIList::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("DataSource")	)
			{
				// this requires refreshing the schema cells and repopulating the list
				RefreshListData(TRUE);
			}
			else if ( PropertyName == TEXT("CellLinkType") )
			{
				if ( CellLinkType == LINKED_Columns )
				{
					const FLOAT MinValue = MinColumnSize.GetValue(this);
					if ( ColumnWidth.GetValue(this) < MinValue )
					{
						ColumnWidth.SetValue(this, MinValue);
					}
				}
				else if ( CellLinkType == LINKED_Rows )
				{
					const FLOAT MinValue = MinColumnSize.GetValue(this);
					if ( RowHeight.GetValue(this) < MinValue )
					{
						RowHeight.SetValue(this, MinValue);
					}
				}

				// changing this property requires repopulating the list, but no need to refresh the schema
				RefreshListData(FALSE);
			}
			else if (
				PropertyName == TEXT("RowHeight")		||
				PropertyName == TEXT("ColumnWidth")		||
				PropertyName == TEXT("ColumnAutoSizeMode")	||
				PropertyName == TEXT("RowAutoSizeMode")	||
				PropertyName == TEXT("CellPadding")		||
				PropertyName == TEXT("CellSpacing")		||
				PropertyName == TEXT("ColumnCount")		||
				PropertyName == TEXT("RowCount") )
			{
				if ( ColumnAutoSizeMode != CELLAUTOSIZE_Uniform )
				{
					if ( CellLinkType == LINKED_Columns )
					{
						const FLOAT MinValue = MinColumnSize.GetValue(this);
						if ( ColumnWidth.GetValue(this) < MinValue )
						{
							ColumnWidth.SetValue(this, MinValue);
						}
					}
					else if ( CellLinkType == LINKED_Rows )
					{
						const FLOAT MinValue = MinColumnSize.GetValue(this);
						if ( RowHeight.GetValue(this) < MinValue )
						{
							RowHeight.SetValue(this, MinValue);
						}
					}
				}

				// only formatting changes
				RefreshFormatting();
			}
			else if ( PropertyName == TEXT("MinColumnSize") )
			{
				// ensure that columns cannot have negative sizes
				const FLOAT Value = MinColumnSize.GetValue(this);
				if ( Value < 0 )
				{
					MinColumnSize.SetValue(this, 0);
				}

				if ( ColumnAutoSizeMode != CELLAUTOSIZE_Uniform )
				{
					if ( CellLinkType == LINKED_Columns )
					{
						const FLOAT MinValue = MinColumnSize.GetValue(this);
						if ( ColumnWidth.GetValue(this) < MinValue )
						{
							ColumnWidth.SetValue(this, MinValue);
						}
					}
					else if ( CellLinkType == LINKED_Rows )
					{
						const FLOAT MinValue = MinColumnSize.GetValue(this);
						if ( RowHeight.GetValue(this) < MinValue )
						{
							RowHeight.SetValue(this, MinValue);
						}
					}
				}
			}
			else if ( PropertyName == TEXT("bEnableVerticalScrollbar") )
			{
				if(VerticalScrollbar != NULL)
				{
					RefreshFormatting();
				}
			}
			else if ( PropertyName == TEXT("SortComponent") )
			{
				// user could have just removed a SortComponent from the list, so double-check
				if ( SortComponent != NULL )
				{
					SortComponent->ResetSortColumns(TRUE);
				}
			}
			else if ( PropertyName == TEXT("bUpdateItemUnderCursor") )
			{
				SetActiveCursorUpdate(IsHotTrackingEnabled());
			}
			else if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( BackgroundImageComponent != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<ThisClass>()->BackgroundImageComponent;
						if ( ComponentTemplate != NULL )
						{
							BackgroundImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						}
						else
						{
							BackgroundImageComponent->StyleResolverTag = TEXT("Background Image Style");
						}

						// user created a new background image component - add it to the list of style subscribers
						AddStyleSubscriber(BackgroundImageComponent);

						// now initialize the component's image
						BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
					}
				}
				else if ( BackgroundImageComponent != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && BackgroundImageComponent->GetImage() != NULL )
					{
#if 0
						USurface* CurrentValue = BackgroundImageComponent->GetImage();

						// changed the value of the image texture/material
						// clear the data store binding
						//@fixme ronp - do we always need to clear the data store binding?
						SetDataStoreBinding(TEXT(""));

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetImage(CurrentValue);
#endif
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called when a member property value has been changed in the editor.
 */
void UUIList::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Copies the values from the deprecated SelectionOverlayStyle property into the appropriate element of the ItemOverlayStyle array.
 */
void UUIList::PostLoad()
{
	Super::PostLoad();
}

/* ==========================================================================================================
	UUIObjectList
========================================================================================================== */
/**
 * Returns the widget for the specified element.
 *
 * @param	ElementIndex	index [into the Items array] for the value to return.
 * @param	CellIndex		for lists which have linked columns or rows, indicates which column/row to retrieve.
 *
 * @return	the value of the specified element, or an empty string if that element doesn't have a text value.
 */
UUIObject* UUIObjectList::GetElementObjectValue( INT ElementIndex, INT CellIndex/*=INDEX_NONE*/ ) const
{
	UUIObject* Result = NULL;

	return Result;
}

/* ==========================================================================================================
	FUIListItem
========================================================================================================== */
FUIListItem::FUIListItem( const FUIListItemDataBinding& InDataSource, UUIObject* inValueChild/*=NULL*/ )
: DataSource(InDataSource)
, ElementState(ELEMENT_Normal)
, ElementWidget(inValueChild)
{
	appMemzero( &Cells, sizeof(FScriptArray) );
}


/**
 * Changes the ElementState for this element and refreshes its cell's cached style references based on the new cell state
 *
 * @param	NewElementState	the new element state to use.
 *
 * @return	TRUE if the element state actually changed.
 */
UBOOL FUIListItem::SetElementState( EUIListElementState NewElementState )
{
	const UBOOL bElementStateChanged = NewElementState != ElementState;

	ElementState = NewElementState;

	if ( bElementStateChanged )
	{
		for ( INT CellIndex = 0; CellIndex < Cells.Num(); CellIndex++ )
		{
			Cells(CellIndex).ApplyCellStyleData(NewElementState);
		}
	}

	return bElementStateChanged;
}

/* ==========================================================================================================
	FUIListElementCell
========================================================================================================== */
/** Default initialization ctor */
FUIListElementCell::FUIListElementCell(EEventParm)
: ContainerElementIndex(INDEX_NONE)
, OwnerList(NULL)
, ValueString(NULL)
{
	appMemzero(CellStyle,	sizeof(CellStyle));

	// we must manually set the struct defaults since we were created natively
	for ( INT ElementStateIndex = 0; ElementStateIndex < ELEMENT_MAX; ElementStateIndex++ )
	{
		CellStyle[ElementStateIndex].RequiredStyleClass = UUIStyle_Combo::StaticClass();
	}
}

/**
 * Initializes this cell's values and creates the cell's UIListString.
 */
void FUIListElementCell::OnCellCreated( INT ElementIndex, UUIList* InOwnerList )
{
	check(InOwnerList);

	ContainerElementIndex = ElementIndex;
	OwnerList = InOwnerList;
	if ( ValueString == NULL && !IsObjectCell() )
	{
		ValueString = ConstructObject<UUIListString>(UUIListString::StaticClass(), OwnerList);
	}
}

/**
 * Called when this cell is created while populating the elements for the owning list. Assigns the specified
 * widget as the value for ValueChild.
 */
void FUIListElementCell::OnCellCreated( INT ElementIndex, UUIList* inOwnerList, UUIObject* CellWidget )
{
	check(inOwnerList);

	ContainerElementIndex = ElementIndex;
	OwnerList = inOwnerList;
	ValueChild = CellWidget;
}

/**
 * Resolves the value of the specified tag from the DataProvider and assigns the result to this cell's ValueString.
 * 
 * @param	CellBindingTag	the tag (from the list supported by DataProvider) that should be associated with this
 *							UIListElementCell.
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 */
void FUIListElementCell::AssignBinding( FUIListItemDataBinding& DataSource, FName CellBindingTag )
{
	UBOOL bContainsMarkup = FALSE;
	FUIProviderFieldValue CellValue(EC_EventParm);

	// bind the new cell to the data source
	if ( DataSource.DataSourceProvider && ValueString != NULL && CellBindingTag != NAME_None && !IsObjectCell() )
	{
		//@todo - parse CellBindingTag to see if it contains an array index or refers to internal data providers
		INT ArrayIndex = INDEX_NONE;

		UBOOL bAssignedSuccessfully = FALSE;
		if ( OwnerList->ShouldRenderDataBindings() )
		{
			bAssignedSuccessfully = TRUE;
			CellValue.PropertyTag = CellBindingTag;
			CellValue.PropertyType = DATATYPE_Property;
			CellValue.StringValue = CellBindingTag.ToString();
		}
		else
		{
			// attempt to retrieve the value of this cell for the element indicated by DataSource.DataSourceIndex
			bAssignedSuccessfully = DataSource.DataSourceProvider->GetCellFieldValue(DataSource.DataSourceTag, CellBindingTag, DataSource.DataSourceIndex, CellValue, ArrayIndex);
		}

		if ( bAssignedSuccessfully )
		{
			// Delay setting the string value until we have resolved styles if the string value contains markup.
			bContainsMarkup = CellValue.StringValue.Len() && FUIStringParser::StringContainsMarkup(CellValue.StringValue);
			if( bContainsMarkup==FALSE )
			{
				// request a string node from the data provider that will be responsible for rendering the value of this cell
				FUIStringNode* CellNode = UUIDataProvider::CreateStringNode(CellBindingTag.ToString(), CellValue);
				if ( CellNode != NULL )
				{
					// assign that node to the cell's string
					ValueString->SetValue(CellNode);
				}
			}
		}
	}

	// propagate the cell style to the newly created string node
	FUIListItem* ContainerElement = GetContainerElement();

	check(ContainerElement);
	ApplyCellStyleData( (EUIListElementState)ContainerElement->ElementState );

	// The string value contained markup, so set the value now after styles have been resolved.
	if ( bContainsMarkup )
	{
		ValueString->UUIString::SetValue(CellValue.StringValue, FALSE);
	}
}

/**
 * Resolves the CellStyle for the specified element state using the currently active skin.  This function is called
 * anytime the cached cell style no longer is out of date, such as when the currently active skin has been changed.
 *
 * @param	ElementState	the list element state to update the element style for
 */
void FUIListElementCell::ResolveCellStyles( EUIListElementState ElementState )
{
	checkSlow(OwnerList);

	UUISkin* CurrentSkin = OwnerList->GetActiveSkin();
	checkSlow(CurrentSkin);

	UUIStyle* PreviouslyResolvedStyle = CellStyle[ElementState].GetResolvedStyle();

	// invalidate the current style for this element state
	CellStyle[ElementState].InvalidateResolvedStyle();

	UBOOL bStyleChanged = FALSE;
	if (!CellStyle[ElementState].GetResolvedStyle(CurrentSkin, &bStyleChanged)
	||	CellStyle[ElementState].IsDefaultStyle(CellStyle[ElementState].GetResolvedStyle()) )
	{
		UUIStyle* InheritedStyle = OwnerList->GlobalCellStyle[ElementState].GetResolvedStyle();
		check(InheritedStyle);

		CellStyle[ElementState].SetStyle(InheritedStyle);

		// clear the AssignedStyleId for this style reference so that future attempts to resolve
		// the cell style will fail if the cell style's ResolvedStyle has been invalidated (indicating
		// that we need to use the global cell style)
		CellStyle[ElementState].SetStyleID(FGuid(0,0,0,0));

		bStyleChanged = PreviouslyResolvedStyle != InheritedStyle;
	}

	if ( bStyleChanged )
	{
		ApplyCellStyleData(ElementState);
	}
}

/**
 * Propagates the style data for the current menu state and element state to each cell .  This function is called anytime
 * the style data that is applied to each cell is no longer valid, such as when the cell's CellState changes or when the
 * owning list's menu state is changed.
 *
 * @param	ElementState	the list element state to update the element style for
 */
void FUIListElementCell::ApplyCellStyleData( EUIListElementState ElementState )
{
	// ContainerElement should only be NULL if this is a UIListElementCellTemplate
	FUIListItem* ContainerElement = GetContainerElement();
	if ( ContainerElement != NULL && ElementState == ContainerElement->ElementState )
	{
		checkSlow(OwnerList);

		UUIStyle* CurrentStyle = CellStyle[ElementState].GetResolvedStyle();
		check(CurrentStyle);

		INT ElementIndex = OwnerList->FindElementIndex(ContainerElement->DataSource.DataSourceIndex);
		UUIState* CellMenuState = OwnerList->GetElementMenuState(ElementIndex);
		UUIStyle_Data* CurrentStyleData = CurrentStyle->GetStyleForState(CellMenuState);
		check(CurrentStyleData);

		UUIStyle_Combo* ComboStyleData = Cast<UUIStyle_Combo>(CurrentStyleData);
		if ( ComboStyleData == NULL )
		{
			debugf(TEXT("Styles for list cells must be combo styles! (Currently assigned style: %s)"), *CurrentStyleData->GetFullName());
			return;
		}

		if ( ValueString != NULL && ValueString->SetStringStyle(ComboStyleData) )
		{
			// anything else?  ReapplyFormatting is handled by the callers of this method
		}
	}
}

		
/**
 * @return	the list element (UIListItem) that contains this cell
 */
FUIListItem* FUIListElementCell::GetContainerElement() const
{
	FUIListItem* ContainerElement = NULL;

	UUIComp_ListPresenter* ReportPresenter = OwnerList != NULL
		? Cast<UUIComp_ListPresenter>(OwnerList->CellDataComponent)
		: NULL;

	if (ReportPresenter != NULL
	&&	ReportPresenter->ListItems.IsValidIndex(ContainerElementIndex) )
	{
		ContainerElement = &ReportPresenter->ListItems(ContainerElementIndex);
	}

	return ContainerElement;
}
		
/**
 * @return	TRUE if this cell displays a UIObject rather than a string.
 */
UBOOL FUIListElementCell::IsObjectCell() const
{
	UBOOL bResult = FALSE;

	if ( ValueChild != NULL )
	{
		FUIListItem* ContainerElement = GetContainerElement();
		bResult = ContainerElement != NULL && ContainerElement->ElementWidget != NULL;
	}

	return bResult;
}

/* ==========================================================================================================
	FUIListElementCellTemplate
========================================================================================================== */
/** Default initialization ctor */
FUIListElementCellTemplate::FUIListElementCellTemplate(EEventParm)
: FUIListElementCell(EC_EventParm)
, CellDataField(NAME_None)
, CellSize(0.f, UIORIENT_Horizontal)
, CellPosition(0.f)
{
	appMemzero(&ColumnHeaderText, sizeof(FString));
}

/**
 * Called when this cell is created while populating the elements for the owning list. Creates the cell's UIListString.
 */
void FUIListElementCellTemplate::OnCellCreated( UUIList* inOwnerList )
{
	check(inOwnerList);

	OwnerList = inOwnerList;
	if ( ValueString == NULL )
	{
		ValueString = ConstructObject<UUIListString>(UUIListString::StaticClass(), OwnerList);
	}
}

/**
 * Initializes the specified cell based on this cell template.
 *
 * @param	DataSource		the information about the data source for this element
 * @param	TargetCell		the cell to initialize.
 */
void FUIListElementCellTemplate::InitializeCell( FUIListItemDataBinding& DataSource, FUIListElementCell& TargetCell )
{
	// copy the values for the cell state styles
	for ( INT CellStateIndex = 0; CellStateIndex < ELEMENT_MAX; CellStateIndex++ )
	{
		TargetCell.CellStyle[CellStateIndex] = CellStyle[CellStateIndex];
	}

	// bind the data field to the new cell
	TargetCell.AssignBinding(DataSource, CellDataField);
}

/**
 * Resolves the value of the specified tag from the DataProvider and assigns the result to this cell's ValueString.
 * 
 * @param	DataProvider	the object which contains the data for this element cell.
 * @param	CellBindingTag	the tag (from the list supported by DataProvider) that should be associated with this
 *							UIListElementCell.
 * @param	ColumnHeader	the string that should be displayed in the column header for this cell.
 */
void FUIListElementCellTemplate::AssignBinding( TScriptInterface<IUIListElementCellProvider> DataProvider, FName CellBindingTag, const FString& ColumnHeader )
{
	// bind the new cell to the data source
	if ( DataProvider && ValueString != NULL && CellBindingTag != NAME_None )
	{
		// assign the data tag to our member var
		CellDataField = CellBindingTag;
		ColumnHeaderText = ColumnHeader;
		if ( ColumnHeaderText.Len() == 0 )
		{
			ColumnHeaderText = CellDataField.ToString();
		}

		FUIProviderFieldValue CellValue(EC_EventParm);

		//@todo - do we need to validate the CellBindingTag is actually a valid cell tag contained within DataProvider
//		if ( DataProvider->GetCellFieldValue(CellBindingTag, CellValue, INDEX_NONE, INDEX_NONE) )
		{
			CellValue.PropertyTag = CellBindingTag;
			CellValue.PropertyType = DATATYPE_Property;
			CellValue.StringValue = ColumnHeaderText;

			// request a string node from the data provider that will be responsible for rendering the value of this cell
			FUIStringNode* CellNode = UUIDataProvider::CreateStringNode(ColumnHeaderText, CellValue);
			if ( CellNode != NULL )
			{
				// assign that node to the cell's string
				ValueString->SetValue(CellNode);
			}
		}
	}

	// refresh the cell's styles so that the cell styles are propagated to the newly created string node
	for ( INT ElementStateIndex = 0; ElementStateIndex < ELEMENT_MAX; ElementStateIndex++ )
	{
		ResolveCellStyles((EUIListElementState)ElementStateIndex);
	}

	check(OwnerList);
	UUIStyle* ColumnHeaderStyle = OwnerList->ColumnHeaderStyle.GetResolvedStyle();
	if ( ColumnHeaderStyle == NULL )
	{
		ColumnHeaderStyle = CellStyle[ELEMENT_Normal].GetResolvedStyle();
	}

	ApplyHeaderStyleData(ColumnHeaderStyle);
}


/**
 * Applies the resolved style data for the column header style to the schema cells' strings.  This function is called anytime
 * the header style data that is applied to the schema cells is no longer valid, such as when the owning list's menu state is changed.
 *
 * @param	ResolvedStyle			the style resolved by the style reference
 */
void FUIListElementCellTemplate::ApplyHeaderStyleData( UUIStyle* ResolvedStyle )
{
	checkSlow(OwnerList);
	check(ResolvedStyle);

	UUIStyle_Data* CurrentStyleData = ResolvedStyle->GetStyleForState(OwnerList->GetCurrentState());
	check(CurrentStyleData);

	UUIStyle_Combo* ComboStyleData = Cast<UUIStyle_Combo>(CurrentStyleData);
	if ( ComboStyleData == NULL )
	{
		debugf(TEXT("Styles for list cells must be combo styles! (Currently assigned style: %s)"), *CurrentStyleData->GetFullName());
		return;
	}

	if ( ValueString != NULL )
	{
		ValueString->SetStringStyle(ComboStyleData);
	}
}


/* ==========================================================================================================
	UUIContextMenu
========================================================================================================== */
/**
 * Returns TRUE if this context menu is the scene's currently active context menu.
 */
UBOOL UUIContextMenu::IsActiveContextMenu() const
{
	const UUIScene* SceneOwner = GetScene();
	return InvokingWidget != NULL
		&& SceneOwner != NULL
		&& SceneOwner->GetActiveContextMenu() == this;
}

/**
 * Opens this context menu.  Called by the owning scene when a new context menu is activated.
 *
 * @param	PlayerIndex		the index of the player that triggered the context menu to be opened.
 *
 * @return	TRUE if the context menu was successfully opened.
 */
UBOOL UUIContextMenu::Open( INT PlayerIndex/*=0*/ )
{
	UBOOL bResult = FALSE;

	// only allow the context menu to opened if it's the scene's active context menu
	UUIScene* SceneOwner = GetScene();
	if ( SceneOwner != NULL )
	{
		// now reposition the context menu so that it appears where the cursor is
		FVector2D MousePos(0.f, 0.f);
		if (GetCursorPosition(MousePos, SceneOwner)
		&&	SceneOwner->SetActiveContextMenu(this, PlayerIndex))
		{
			// we need to reformat and resize the list to match the elements that will be displayed
			if ( CellDataComponent != NULL )
			{
				CellDataComponent->bReapplyFormatting = TRUE;
			}

			// update the context menu's position without triggering a scene update
			Position.SetRawPositionValue(UIFACE_Left, MousePos.X, EVALPOS_PixelViewport);
			Position.SetRawPositionValue(UIFACE_Top, MousePos.Y, EVALPOS_PixelViewport);

			// now manually resolve the context menu's position in case the menu was activated between after UUIScene::UpdateScene()
			// but before UUIScene::Render_Scene()
			ResolveContextMenuPosition();
			bResult = TRUE;

			// other stuff.
		}
	}

	return bResult;
}

/**
 * Closes this context menu.
 *
 * @return	TRUE if the context menu closed successfully; FALSE if it didn't close (as a result of OnClose returning
 *			FALSE, for example)
 */
UBOOL UUIContextMenu::Close( INT PlayerIndex/*=0*/ )
{
	UBOOL bResult = FALSE;

	if ( InvokingWidget != NULL
	&&	(!OBJ_DELEGATE_IS_SET(InvokingWidget,OnCloseContextMenu)
	||	InvokingWidget->delegateOnCloseContextMenu(this, PlayerIndex)) )
	{
		const UBOOL bWasActiveMenu = IsActiveContextMenu();
		if ( bWasActiveMenu )
		{
			// clear manually so that calling SetActiveContextMenu can safely call Close().
			GetScene()->ActiveContextMenu = NULL;
		}

		const UBOOL bIsFocusedControl = IsFocused(PlayerIndex) && GetParent() && GetParent()->GetFocusedControl(FALSE, PlayerIndex) == this;
		if (!IsFocused(PlayerIndex)
		||	(bIsFocusedControl 
				? InvokingWidget->SetFocus(NULL, PlayerIndex) 

				//@fixme ronp - this call to LoseFocus causes the scene to have no focused controls, which prevents Next/PrevControl aliases from working
				: LoseFocus(NULL, PlayerIndex)) )
		{
			InvokingWidget = NULL;
			DeactivateStateByClass(UUIState_Active::StaticClass(), PlayerIndex);
			eventSetVisibility(FALSE);
			
			DataSource.UnregisterSubscriberCallback();
			bResult = TRUE;
		}
		else if ( bWasActiveMenu )
		{
			GetScene()->ActiveContextMenu = this;
		}
	}

	return bResult;
}

/**
 * Resolves this context menu's position into actual pixel values.
 */
void UUIContextMenu::ResolveContextMenuPosition()
{
	TLookupMap<FUIDockingNode> DockingStack;
	AddDockingLink(DockingStack);

	for ( INT StackIndex = 0; StackIndex < DockingStack.Num(); StackIndex++ )
	{
		const FUIDockingNode& DockingNode = DockingStack(StackIndex);
		DockingNode.Widget->DockTargets.MarkResolved(DockingNode.Face,0);
		DockingNode.Widget->Position.InvalidatePosition(DockingNode.Face);
	}

	for ( INT StackIndex = 0; StackIndex < DockingStack.Num(); StackIndex++ )
	{
		const FUIDockingNode& DockingNode = DockingStack(StackIndex);
		DockingNode.Widget->ResolveFacePosition( (EUIWidgetFace)DockingNode.Face );
	}

	bResolvePosition = FALSE;
}

/* === UUIList interface === */
/**
 * Called whenever the user chooses an item while this list is focused.  Activates the SubmitSelection kismet event and calls
 * the OnSubmitSelection delegate.
 */
void UUIContextMenu::NotifySubmitSelection( INT PlayerIndex/*=0*/ )
{
	if ( GIsGame && GetMaxVisibleElementCount() > 0 && Items.IsValidIndex(Index) )
	{
		if ( DELEGATE_IS_SET(OnSubmitSelection) )
		{
			// notify unrealscript that the user has submitted the selection
			delegateOnSubmitSelection(this,PlayerIndex);
		}

		if ( IsElementEnabled(Index) )
		{
			if ( OBJ_DELEGATE_IS_SET(InvokingWidget,OnContextMenuItemSelected) )
			{
				// commented out because I'm not sure if we'll need to use ItemId
// 				INT ItemId=INDEX_NONE;
// 				if ( MenuItems.IsValidIndex(Index) )
// 				{
// 					ItemId = MenuItems(Index).ItemId;
// 				}
// 
// 				delegateOnContextMenuItemSelected(this, PlayerIndex, ItemId, Index);
				InvokingWidget->delegateOnContextMenuItemSelected(this, PlayerIndex, Index);
			}

// 			PlayUISound(SubmitDataSuccessCue, PlayerIndex);
// 
// 			// notify kismet that the user has submitted the selection
// 			TArray<UUIEvent_SubmitListData*> Events;
// 			ActivateEventByClass(PlayerIndex, UUIEvent_SubmitListData::StaticClass(), this, FALSE, NULL, (TArray<UUIEvent*>*)&Events);
// 
// 			const INT CurrentItem = GetCurrentItem();
// 			for ( INT EventIndex = 0; EventIndex < Events.Num(); EventIndex++ )
// 			{
// 				UUIEvent_SubmitListData* Event = Events(EventIndex);
// 				Event->SelectedItem = CurrentItem;
// 				Event->PopulateLinkedVariableValues();
// 			}

			Close(PlayerIndex);
		}
		else
		{
			PlayUISound(SubmitDataFailedCue, PlayerIndex);
		}
	}
}

/**
 * Deactivates the UIState_Focused menu state and updates the pertinent members of FocusControls.
 *
 * @param	FocusedChild	the child of this widget that is currently "focused" control for this widget.
 *							A value of NULL indicates that there is no focused child.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 */
UBOOL UUIContextMenu::LoseFocus( UUIObject* FocusedChild, INT PlayerIndex )
{
//	tracef(TEXT("UUIContextMenu::LoseFocus  FocusedChild: %s"), FocusedChild != NULL ? *FocusedChild->GetTag().ToString() : TEXT("NULL"));
	UBOOL bResult = Super::LoseFocus(FocusedChild, PlayerIndex);

	// if bResult is TRUE and FocusedChild is NULL, it means that this context menu was the focused control
	// and just lost focus  (such as when the user clicks on another widget while the context menu is up)
	const UBOOL bWasEndOfFocusChain = bResult && FocusedChild == NULL;
	if ( bWasEndOfFocusChain && IsActiveContextMenu() )
	{
		if ( !Close(PlayerIndex) )
		{
			// if we couldn't be closed, re-take focus
			GainFocus(NULL, PlayerIndex);
			bResult = FALSE;
		}
	}

//	tracef(TEXT("UUIContextMenu::LoseFocus  FocusedChild: %s"), FocusedChild != NULL ? *FocusedChild->GetTag().ToString() : TEXT("NULL"));
	return bResult;
}

/**
 * Handles input events for this context menu.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUIContextMenu::ProcessInputKey( const FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	if ( EventParms.EventType == IE_Released )
	{
		if ( EventParms.InputAliasName == UIKEY_HideContextMenu )
		{
			FVector2D MousePos(0,0);				
			UBOOL bInputConsumed = FALSE;
			if ( !IsCursorInputKey(EventParms.InputKeyName) || !GetCursorPosition(MousePos, GetScene()) || ContainsPoint(MousePos) )
			{
				Close(EventParms.PlayerIndex);
				bResult = TRUE;
			}
		}
	}

	return bResult || Super::ProcessInputKey(EventParms);
}

/* === UUIScreenObject interface === */
/**
 * Overridden to prevent any of this widget's inherited methods or components from triggering a scene update, as context menu
 * positions are updated differently.
 *
 * @param	bDockingStackChanged	if TRUE, the scene will rebuild its DockingStack at the beginning
 *									the next frame
 * @param	bPositionsChanged		if TRUE, the scene will update the positions for all its widgets
 *									at the beginning of the next frame
 * @param	bNavLinksOutdated		if TRUE, the scene will update the navigation links for all widgets
 *									at the beginning of the next frame
 * @param	bWidgetStylesChanged	if TRUE, the scene will refresh the widgets reapplying their current styles
 */
void UUIContextMenu::RequestSceneUpdate( UBOOL bDockingStackChanged, UBOOL bPositionsChanged, UBOOL bNavLinksOutdated/*=FALSE*/, UBOOL bWidgetStylesChanged/*=FALSE*/ )
{
	// don't need to update positions in the entire scene - just flag the context menu to be updated next tick
	if ( bPositionsChanged )
	{
		bResolvePosition = TRUE;
	}
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUIContextMenu::GetSupportedUIActionKeyNames( TArray<FName>& out_KeyNames )
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem(UIKEY_HideContextMenu);
}



/* ==========================================================================================================
	UUIObjectList
========================================================================================================== */
IMPLEMENT_CLASS(UUIObjectList);

// EOF

