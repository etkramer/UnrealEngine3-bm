/**
 * Dataprovider that providers detailed stats information for a given stats row.  Should be subclassed.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_StatsElementProvider extends UTUIDataProvider_SimpleElementProvider
	native(UI);

cpptext
{
	/** 
	 * Returns a reference to the stats row that is used to expose data.
	 *
	 * @param StatsRow	Object to hold the stats row.
	 *
	 * @return TRUE if a stats row was found, FALSE otherwise. 
	 */
	virtual UBOOL GetStatsRow(FOnlineStatsRow& OutStatsRow);

	/** 
	* Returns a reference to the stats id in this particular view that is used to expose data.
	*
	* @param StatsName	Name of the stat we want a mapping in this view for.
	*
	* @return ColumnId if a stats row was found, -1 otherwise. 
	*/
	virtual INT GetColumnIdFromStatName(const FName StatName);
};

/** Cache of the data we are using to display to the UI. */
var transient OnlineStatsRead ReadObject;

/**
 * @return Returns the stats read object used by this provider.
 */
event OnlineStatsRead GetStatsReadObject()
{
	return ReadObject;
}

/**
 * Returns the localized name of a column given its ID and row
 *
 * @param StatsRow	Row to get the value from.
 * @param ColumnId	ColumnId to search for
 *
 * @return	The string value of the column.
 */
native function string GetColumnValue(const out OnlineStatsRow StatsRow, name ColumnId);

/**
 * Returns the localized name of a column given its stat name
 *
 * @param ColumnId	ColumnId to search for
 *
 * @return	The localized name of the column.
 */
native function string GetColumnName(name StatName);

DefaultProperties
{
}