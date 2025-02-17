/**
 * Dataprovider that returns a row for each vehicle/vehicle weapons with kills/death/suicides given a user's stats results.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_StatsVehicles extends UTUIDataProvider_StatsElementProvider
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

/** Struct that defines a vehicle row. */
struct native VehicleStatsRow
{
	var string VehicleName;

	var const name DrivingTimeName;
	var const name VehicleKillsName;
};
var transient array<VehicleStatsRow> Stats;

/** @return Returns the number of elements(rows) provided. */
native function int GetElementCount();

DefaultProperties
{
	Stats.Empty();

	Stats.Add((VehicleName="UTVehicle_Cicada", VehicleKillsName=VEHICLEKILL_UTVEHICLE_CICADA_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_CICADA_CONTENT))
	Stats.Add((VehicleName="UTVehicle_DarkWalker", VehicleKillsName=VEHICLEKILL_UTVEHICLE_DARKWALKER_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_DARKWALKER_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Fury", VehicleKillsName=VEHICLEKILL_UTVEHICLE_FURY_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_FURY_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Goliath", VehicleKillsName=VEHICLEKILL_UTVEHICLE_GOLIATH_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_GOLIATH_CONTENT))
	Stats.Add((VehicleName="UTVehicle_HellBender", VehicleKillsName=VEHICLEKILL_UTVEHICLE_HELLBENDER_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_HELLBENDER_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Hoverboard", VehicleKillsName=VEHICLEKILL_UTVEHICLE_HOVERBOARD, DrivingTimeName=DRIVING_UTVEHICLE_HOVERBOARD))
	Stats.Add((VehicleName="UTVehicle_Leviathan", VehicleKillsName=VEHICLEKILL_UTVEHICLE_LEVIATHAN_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_LEVIATHAN_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Manta", VehicleKillsName=VEHICLEKILL_UTVEHICLE_MANTA_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_MANTA_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Nemesis", VehicleKillsName=VEHICLEKILL_UTVEHICLE_NEMESIS, DrivingTimeName=DRIVING_UTVEHICLE_NEMESIS))
	Stats.Add((VehicleName="UTVehicle_NightShade", VehicleKillsName=VEHICLEKILL_UTVEHICLE_NIGHTSHADE_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_NIGHTSHADE_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Paladin", VehicleKillsName=VEHICLEKILL_UTVEHICLE_PALADIN, DrivingTimeName=DRIVING_UTVEHICLE_PALADIN))
	Stats.Add((VehicleName="UTVehicle_Raptor", VehicleKillsName=VEHICLEKILL_UTVEHICLE_RAPTOR_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_RAPTOR_CONTENT))
	Stats.Add((VehicleName="UTVehicle_SPMA", VehicleKillsName=VEHICLEKILL_UTVEHICLE_SPMA_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_SPMA_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Scavenger", VehicleKillsName=VEHICLEKILL_UTVEHICLE_SCAVENGER_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_SCAVENGER_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Scorpion", VehicleKillsName=VEHICLEKILL_UTVEHICLE_SCORPION_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_SCORPION_CONTENT))
	Stats.Add((VehicleName="UTVehicle_StealthBender", VehicleKillsName=VEHICLEKILL_UTVEHICLE_STEALTHBENDER_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_STEALTHBENDER_CONTENT))
	Stats.Add((VehicleName="UTVehicle_Turret", VehicleKillsName=VEHICLEKILL_UTVEHICLE_TURRET, DrivingTimeName=DRIVING_UTVEHICLE_TURRET))
	Stats.Add((VehicleName="UTVehicle_Viper", VehicleKillsName=VEHICLEKILL_UTVEHICLE_VIPER_CONTENT, DrivingTimeName=DRIVING_UTVEHICLE_VIPER_CONTENT))
}