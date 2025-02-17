/**
* Provides data for a UT3 weapon.
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class UTUIDataProvider_Weapon extends UTUIResourceDataProvider
	PerObjectConfig
	native(UI);

/** The weapon class path */
var config string ClassName;
/** class path to this weapon's preferred ammo class
 * this is primarily used by weapon replacement mutators to get the ammo types to replace from/to
 */
var config string AmmoClassPath;

/** optional flags separated by pipes | that can be parsed by the UI as arbitrary options (for example, to exclude weapons from some menus, etc) */
var config string Flags;

/** Friendly name for the weapon */
var config localized string FriendlyName;

/** Description for the weapon */
var config localized string Description;

/** String reference to the 3rd person mesh for this weapon.  This mesh is used in the UI. */
var config string MeshReference;

defaultproperties
{
	bSearchAllInis=true
}
