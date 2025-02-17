/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_Helmet_LocustDroneRandom extends Item_HelmetBase;


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
	SpawnableClassesData.Add( ( PercentageToSpawn=0.2f, ItemClass=class'Item_Helmet_None' ) )
	SpawnableClassesData.Add( ( PercentageToSpawn=0.2f, ItemClass=class'Item_Helmet_LocustMask' ) )
	SpawnableClassesData.Add( ( PercentageToSpawn=0.2f, ItemClass=class'Item_Helmet_LocustOpenFace' ) )
	SpawnableClassesData.Add( ( PercentageToSpawn=0.2f, ItemClass=class'Item_Helmet_LocustMouth' ) )
	SpawnableClassesData.Add( ( PercentageToSpawn=0.2f, ItemClass=class'Item_Helmet_LocustFull' ) )
	SpawnableClassesData.Add( ( PercentageToSpawn=0.2f, ItemClass=class'Item_Helmet_LocustGogglesRandom' ) )
}



