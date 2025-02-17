/**
 * Dataprovider that returns a row for each general stat for a user.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_StatsGeneral extends UTUIDataProvider_StatsElementProvider
	native(UI);

`include(UTStats.uci)

cpptext
{

public:
	/* === IUIListElementCellProvider interface === */
	/**
	 * Retrieves the list of tags that can be bound to individual cells in a single list element.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
	 */
	virtual void GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags );

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
	virtual UBOOL GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );
}

/** Struct that defines a reward stat row. */
struct native GeneralStatsRow
{
    //Name of the stat we're serving
	var const name StatName;
};
var transient array<GeneralStatsRow> Stats;

/** @return Returns the number of elements(rows) provided. */
native function int GetElementCount();

DefaultProperties
{
	Stats.Empty();

	//All possible stats for all game modes
	Stats.Add((StatName="EVENT_KILLS"))
	Stats.Add((StatName="EVENT_DEATHS"))
	Stats.Add((StatName="EVENT_RANOVERKILLS"))
    Stats.Add((StatName="EVENT_RANOVERDEATHS"))
	Stats.Add((StatName="EVENT_HIJACKED"))
	Stats.Add((StatName="PICKUPS_HEALTH"))
	Stats.Add((StatName="PICKUPS_ARMOR"))
	Stats.Add((StatName="PICKUPS_JUMPBOOTS"))
	Stats.Add((StatName="PICKUPS_SHIELDBELT"))
	Stats.Add((StatName="PICKUPS_BERSERK"))
	Stats.Add((StatName="POWERUPTIME_BERSERK"))
	Stats.Add((StatName="PICKUPS_INVISIBILITY"))
	Stats.Add((StatName="POWERUPTIME_INVISIBILITY"))
	Stats.Add((StatName="PICKUPS_INVULNERABILITY"))
	Stats.Add((StatName="POWERUPTIME_INVULNERABILITY"))
	Stats.Add((StatName="PICKUPS_UDAMAGE"))
	Stats.Add((StatName="POWERUPTIME_UDAMAGE"))

	//CTF/VCTF
	Stats.Add((StatName="EVENT_HATTRICK"))
	Stats.Add((StatName="EVENT_KILLEDFLAGCARRIER")
	Stats.Add((StatName="EVENT_RETURNEDFLAG"))
	Stats.Add((StatName="EVENT_SCOREDFLAG"))
	Stats.Add((StatName="EVENT_LASTSECONDSAVE"))

	//WAR
	Stats.Add((StatName="EVENT_RETURNEDORB"))
	Stats.Add((StatName="EVENT_SCOREDORB"))
	Stats.Add((StatName="NODE_DAMAGEDCORE"))
	Stats.Add((StatName="NODE_DESTROYEDCORE"))
	Stats.Add((StatName="NODE_DESTROYEDNODE"))
	Stats.Add((StatName="NODE_HEALEDNODE"))
	Stats.Add((StatName="NODE_NODEBUILT"))
	Stats.Add((StatName="NODE_NODEBUSTER"))
}
