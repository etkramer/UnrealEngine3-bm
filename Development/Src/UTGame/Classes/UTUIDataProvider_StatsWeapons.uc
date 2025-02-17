/**
 * Dataprovider that returns a row for each weapon with kills/death/suicides given a user's stats results.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_StatsWeapons extends UTUIDataProvider_StatsElementProvider
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

/** Struct that defines a weapon row. */
struct native WeaponStatsRow
{
	var string WeaponName;

	var const name KillsName;
	var const name DeathsName;
	var const name SuicidesName;
};
var transient array<WeaponStatsRow> Stats;

/** @return Returns the number of elements(rows) provided. */
native function int GetElementCount();

DefaultProperties
{
	Stats.Empty();

	Stats.Add((WeaponName="UTWeap_BioRifle_Content",KillsName=KILLS_BIORIFLE,DeathsName=DEATHS_BIORIFLE,SuicidesName=SUICIDES_BIORIFLE))
	Stats.Add((WeaponName="UTWeap_Enforcer",KillsName=KILLS_ENFORCER,DeathsName=DEATHS_ENFORCER,SuicidesName=SUICIDES_ENFORCER))
	Stats.Add((WeaponName="UTWeap_FlakCannon",KillsName=KILLS_FLAKCANNON,DeathsName=DEATHS_FLAKCANNON,SuicidesName=SUICIDES_FLAKCANNON))
	Stats.Add((WeaponName="UTWeap_ImpactHammer",KillsName=KILLS_IMPACTHAMMER,DeathsName=DEATHS_IMPACTHAMMER,SuicidesName=SUICIDES_IMPACTHAMMER))
	Stats.Add((WeaponName="UTWeap_LinkGun",KillsName=KILLS_LINKGUN,DeathsName=DEATHS_LINKGUN,SuicidesName=SUICIDES_LINKGUN))
	Stats.Add((WeaponName="UTWeap_RocketLauncher",KillsName=KILLS_ROCKETLAUNCHER,DeathsName=DEATHS_ROCKETLAUNCHER,SuicidesName=SUICIDES_ROCKETLAUNCHER))
	Stats.Add((WeaponName="UTWeap_ShockRifle",KillsName=KILLS_SHOCKRIFLE,DeathsName=DEATHS_SHOCKRIFLE,SuicidesName=SUICIDES_SHOCKRIFLE))
	Stats.Add((WeaponName="UTWeap_ShockCombo",KillsName=KILLS_SHOCKCOMBO,DeathsName=DEATHS_SHOCKCOMBO,SuicidesName=SUICIDES_SHOCKCOMBO))
	Stats.Add((WeaponName="UTWeap_SniperRifle",KillsName=KILLS_SNIPERRIFLE,DeathsName=DEATHS_SNIPERRIFLE,SuicidesName=SUICIDES_SNIPERRIFLE))
	Stats.Add((WeaponName="UTWeap_Headshot",KillsName=KILLS_HEADSHOT,DeathsName=DEATHS_HEADSHOT,SuicidesName=SUICIDES_HEADSHOT))
	Stats.Add((WeaponName="UTWeap_Stinger",KillsName=KILLS_STINGER,DeathsName=DEATHS_STINGER,SuicidesName=SUICIDES_STINGER))
	Stats.Add((WeaponName="UTWeap_Translocator",KillsName=KILLS_TRANSLOCATOR,DeathsName=DEATHS_TRANSLOCATOR,SuicidesName=SUICIDES_TRANSLOCATOR)

	Stats.Add((WeaponName="UTWeap_Instagib",KillsName=KILLS_INSTAGIB,DeathsName=DEATHS_INSTAGIB,SuicidesName=SUICIDES_INSTAGIB))

	Stats.Add((WeaponName="UTWeap_Environment",KillsName=KILLS_ENVIRONMENT,DeathsName=DEATHS_ENVIRONMENT,SuicidesName=SUICIDES_ENVIRONMENT))
	Stats.Add((WeaponName="UTWeap_Spidermine",KillsName=KILLS_SPIDERMINE,DeathsName=DEATHS_SPIDERMINE,SuicidesName=SUICIDES_SPIDERMINE))
	Stats.Add((WeaponName="UTWeap_ShapedCharge",KillsName=KILLS_SHAPEDCHARGE,DeathsName=DEATHS_SHAPEDCHARGE,SuicidesName=SUICIDES_SHAPEDCHARGE))
}
