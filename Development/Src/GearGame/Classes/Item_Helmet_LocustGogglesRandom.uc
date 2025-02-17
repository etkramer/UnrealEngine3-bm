/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_Helmet_LocustGogglesRandom extends Item_HelmetBase;



/**
 * This will return the static mesh for this spawnable type.  We use this function
 * in order to push the resources needed for the helmets to the content package where possible.
 **/
simulated static function class<ItemSpawnable> GetSpawnableItemClass()
{
	return GetSpawnableItemClassRandom();
}


defaultproperties
{
	SpawnableClassesData.Add( ( PercentageToSpawn=0.5f, ItemClass=class'Item_Helmet_LocustGogglesUp' ) )
	SpawnableClassesData.Add( ( PercentageToSpawn=0.5f, ItemClass=class'Item_Helmet_LocustGogglesDown' ) )
}



