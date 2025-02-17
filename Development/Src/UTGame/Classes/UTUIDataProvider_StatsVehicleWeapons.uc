/**
 * Dataprovider that returns a row for each vehicle weapon with kills/death/suicides given a user's stats results.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_StatsVehicleWeapons extends UTUIDataProvider_StatsElementProvider
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
struct native VehicleWeaponStatsRow
{
	var string WeaponName;

	var const name KillsName;
	var const name DeathsName;
	var const name SuicidesName;

};
var transient array<VehicleWeaponStatsRow> Stats;

/** @return Returns the number of elements(rows) provided. */
native function int GetElementCount();

DefaultProperties
{
	Stats.Empty();
	Stats.Add((WeaponName="CicadaRocket",KillsName=KILLS_CICADAROCKET,DeathsName=DEATHS_CICADAROCKET,SuicidesName=SUICIDES_CICADAROCKET))
	Stats.Add((WeaponName="CicadaTurret",KillsName=KILLS_CICADATURRET,DeathsName=DEATHS_CICADATURRET,SuicidesName=SUICIDES_CICADATURRET))
	Stats.Add((WeaponName="DarkWalkerPassGun",KillsName=KILLS_DARKWALKERPASSGUN,DeathsName=DEATHS_DARKWALKERPASSGUN,SuicidesName=SUICIDES_DARKWALKERPASSGUN))
	Stats.Add((WeaponName="DarkWalkerTurret",KillsName=KILLS_DARKWALKERTURRET,DeathsName=DEATHS_DARKWALKERTURRET,SuicidesName=SUICIDES_DARKWALKERTURRET))
	Stats.Add((WeaponName="FuryGun",KillsName=KILLS_FURYGUN,DeathsName=DEATHS_FURYGUN,SuicidesName=SUICIDES_FURYGUN))
	Stats.Add((WeaponName="GoliathMachineGun",KillsName=KILLS_GOLIATHMACHINEGUN,DeathsName=DEATHS_GOLIATHMACHINEGUN,SuicidesName=SUICIDES_GOLIATHMACHINEGUN))
	Stats.Add((WeaponName="GoliathTurret",KillsName=KILLS_GOLIATHTURRET,DeathsName=DEATHS_GOLIATHTURRET,SuicidesName=SUICIDES_GOLIATHTURRET))
	Stats.Add((WeaponName="HellbenderPrimary",KillsName=KILLS_HELLBENDERPRIMARY,DeathsName=DEATHS_HELLBENDERPRIMARY,SuicidesName=SUICIDES_HELLBENDERPRIMARY))
	Stats.Add((WeaponName="LeviathanExplosion",KillsName=KILLS_LEVIATHANEXPLOSION,DeathsName=DEATHS_LEVIATHANEXPLOSION,SuicidesName=SUICIDES_LEVIATHANEXPLOSION))
	Stats.Add((WeaponName="LeviathanPrimary",KillsName=KILLS_LEVIATHANPRIMARY,DeathsName=DEATHS_LEVIATHANPRIMARY,SuicidesName=SUICIDES_LEVIATHANPRIMARY))
	Stats.Add((WeaponName="LeviathanTurretBeam",KillsName=KILLS_LEVIATHANTURRETBEAM,DeathsName=DEATHS_LEVIATHANTURRETBEAM,SuicidesName=SUICIDES_LEVIATHANTURRETBEAM))
	Stats.Add((WeaponName="LeviathanTurretRocket",KillsName=KILLS_LEVIATHANTURRETROCKET,DeathsName=DEATHS_LEVIATHANTURRETROCKET,SuicidesName=SUICIDES_LEVIATHANTURRETROCKET))
	Stats.Add((WeaponName="LeviathanTurretShock",KillsName=KILLS_LEVIATHANTURRETSHOCK,DeathsName=DEATHS_LEVIATHANTURRETSHOCK,SuicidesName=SUICIDES_LEVIATHANTURRETSHOCK))
	Stats.Add((WeaponName="LeviathanTurretStinger",KillsName=KILLS_LEVIATHANTURRETSTINGER,DeathsName=DEATHS_LEVIATHANTURRETSTINGER,SuicidesName=SUICIDES_LEVIATHANTURRETSTINGER))
	Stats.Add((WeaponName="MantaGun",KillsName=KILLS_MANTAGUN,DeathsName=DEATHS_MANTAGUN,SuicidesName=SUICIDES_MANTAGUN))
	Stats.Add((WeaponName="NemesisTurret",KillsName=KILLS_NEMESISTURRET,DeathsName=DEATHS_NEMESISTURRET,SuicidesName=SUICIDES_NEMESISTURRET))
	Stats.Add((WeaponName="NightshadeGun",KillsName=KILLS_NIGHTSHADEGUN,DeathsName=DEATHS_NIGHTSHADEGUN,SuicidesName=SUICIDES_NIGHTSHADEGUN))
	Stats.Add((WeaponName="PaladinExplosion",KillsName=KILLS_PALADINEXPLOSION,DeathsName=DEATHS_PALADINEXPLOSION,SuicidesName=SUICIDES_PALADINEXPLOSION))
	Stats.Add((WeaponName="PaladinGun",KillsName=KILLS_PALADINGUN,DeathsName=DEATHS_PALADINGUN,SuicidesName=SUICIDES_PALADINGUN))
	Stats.Add((WeaponName="RaptorGun",KillsName=KILLS_RAPTORGUN,DeathsName=DEATHS_RAPTORGUN,SuicidesName=SUICIDES_RAPTORGUN))
	Stats.Add((WeaponName="RaptorRocket",KillsName=KILLS_RAPTORROCKET,DeathsName=DEATHS_RAPTORROCKET,SuicidesName=SUICIDES_RAPTORROCKET))
	Stats.Add((WeaponName="SPMACameraCrush",KillsName=KILLS_SPMACAMERACRUSH,DeathsName=DEATHS_SPMACAMERACRUSH,SuicidesName=SUICIDES_SPMACAMERACRUSH))
	Stats.Add((WeaponName="SPMACannon",KillsName=KILLS_SPMACANNON,DeathsName=DEATHS_SPMACANNON,SuicidesName=SUICIDES_SPMACANNON))
	Stats.Add((WeaponName="SPMATurret",KillsName=KILLS_SPMATURRET,DeathsName=DEATHS_SPMATURRET,SuicidesName=SUICIDES_SPMATURRET))
	Stats.Add((WeaponName="ScavengerStabbed",KillsName=KILLS_SCAVENGERSTABBED,DeathsName=DEATHS_SCAVENGERSTABBED,SuicidesName=SUICIDES_SCAVENGERSTABBED))
	Stats.Add((WeaponName="ScavengerGun",KillsName=KILLS_SCAVENGERGUN,DeathsName=DEATHS_SCAVENGERGUN,SuicidesName=SUICIDES_SCAVENGERGUN))
	Stats.Add((WeaponName="ScorpionBlade",KillsName=KILLS_SCORPIONBLADE,DeathsName=DEATHS_SCORPIONBLADE,SuicidesName=SUICIDES_SCORPIONBLADE))
	Stats.Add((WeaponName="ScorpionGlob",KillsName=KILLS_SCORPIONGLOB,DeathsName=DEATHS_SCORPIONGLOB,SuicidesName=SUICIDES_SCORPIONGLOB))
	Stats.Add((WeaponName="ScorpionSelfDestruct",KillsName=KILLS_SCORPIONSELFDESTRUCT,DeathsName=DEATHS_SCORPIONSELFDESTRUCT,SuicidesName=SUICIDES_SCORPIONSELFDESTRUCT))
	Stats.Add((WeaponName="StealthBenderGun",KillsName=KILLS_STEALTHBENDERGUN,DeathsName=DEATHS_STEALTHBENDERGUN,SuicidesName=SUICIDES_STEALTHBENDERGUN))
	Stats.Add((WeaponName="TurretPrimary",KillsName=KILLS_TURRETPRIMARY,DeathsName=DEATHS_TURRETPRIMARY,SuicidesName=SUICIDES_TURRETPRIMARY))
	Stats.Add((WeaponName="TurretRocket",KillsName=KILLS_TURRETROCKET,DeathsName=DEATHS_TURRETROCKET,SuicidesName=SUICIDES_TURRETROCKET))
	Stats.Add((WeaponName="TurretShock",KillsName=KILLS_TURRETSHOCK,DeathsName=DEATHS_TURRETSHOCK,SuicidesName=SUICIDES_TURRETSHOCK))
	Stats.Add((WeaponName="TurretStinger",KillsName=KILLS_TURRETSTINGER,DeathsName=DEATHS_TURRETSTINGER,SuicidesName=SUICIDES_TURRETSTINGER))
	Stats.Add((WeaponName="ViperGun",KillsName=KILLS_VIPERGUN,DeathsName=DEATHS_VIPERGUN,SuicidesName=SUICIDES_VIPERGUN))
	Stats.Add((WeaponName="ViperSelfDestruct",KillsName=KILLS_VIPERSELFDESTRUCT,DeathsName=DEATHS_VIPERSELFDESTRUCT,SuicidesName=SUICIDES_VIPERSELFDESTRUCT))
}