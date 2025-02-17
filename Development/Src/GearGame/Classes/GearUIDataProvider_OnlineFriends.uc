/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * UT specific version of the friends dataprovider.
 */
class GearUIDataProvider_OnlineFriends extends UIDataProvider_OnlineFriends
	native(UI);

/** The column name to display in the UI */
var localized string bHasInvited;

cpptext
{
	/* === IUIListElement interface === */

	/**
	 * Returns the names of the exposed members in OnlineFriend
	 *
	 * @see OnlineFriend structure in OnlineSubsystem
	 */
	virtual void GetElementCellTags(FName FieldName, TMap<FName,FString>& CellTags)
	{
		CellTags.Set(FName(TEXT("bHasInvited")),*bHasInvited);
		Super::GetElementCellTags(FieldName, CellTags);
	}

	/**
	 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	CellTag			the tag for the element cell to resolve the value for
	 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
	 *							do not provide unique UIListElement objects for each element.
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
	 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
	 *							to a data collection.
	 */
	virtual UBOOL GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );

};
