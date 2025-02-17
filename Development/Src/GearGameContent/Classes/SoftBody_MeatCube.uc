/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SoftBody_MeatCube extends Gear_SoftBodySpawnable
	placeable;

defaultproperties
{
	bNoDelete=TRUE
	bAutoRemove=FALSE

    DrawScale=1.500000

	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'SoftBodies.Skeletal.SK_SoftCube'
		Materials(0)=Material'SoftBodies.Materials.GDC_Muscle'
		Materials(1)=MaterialInstanceConstant'Proto_WaterMaterials.Materials.WaterySurface_Opaque_INST2'
		PhysMaterialOverride=PhysicalMaterial'GearPhysMats.Flesh'
	End Object
}
