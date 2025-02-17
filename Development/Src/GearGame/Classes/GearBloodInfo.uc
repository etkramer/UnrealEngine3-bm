/** 
 * 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearBloodInfo extends Object
	native
	deprecated
	hidecategories(Object)
	editinlinenew;

/** This is the version for the blood info.  This is primary used to have nice ordered BloodDecals as they are indexed by enum value **/
var int Version;


struct native BloodDecalDatum
{
	/** This is left for the sake of editing, knowing which entry is which weapon */
	var() editconst EGearDecalType Type;

	/** This holds the decal data for this blood decal type **/
	var() editinline array<DecalData> DecalData;
};

/** This contains all of the types of blood decals that we can place in the world **/
var() editinline array<BloodDecalDatum> BloodInfo;   



cpptext
{
	virtual void PostLoad();
}



defaultproperties
{
	Version=2
		
}



