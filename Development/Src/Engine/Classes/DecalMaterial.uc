/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DecalMaterial extends Material
	native(Decal);

cpptext
{
	// UMaterial interface.
	virtual FMaterialResource* AllocateResource();

	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PreSave();
	virtual void PostLoad();
	virtual void Serialize(FArchive& Ar);
}

defaultproperties
{	
	bUsedWithStaticLighting=TRUE
	bUsedWithSkeletalMesh=TRUE
	bUsedWithDecals=TRUE
	bUsedWithFluidSurfaces=TRUE
	bUsedWithFracturedMeshes=TRUE
}
