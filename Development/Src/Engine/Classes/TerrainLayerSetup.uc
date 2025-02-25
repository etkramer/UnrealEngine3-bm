/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class TerrainLayerSetup extends Object
	native(Terrain)
	hidecategories(Object)
	collapsecategories;

struct FilterLimit
{
	var() bool	Enabled;
	var() float	Base,
				NoiseScale,
				NoiseAmount;
};

struct TerrainFilteredMaterial
{
	var() bool			UseNoise;
	var() float			NoiseScale,
						NoisePercent;

	var() FilterLimit	MinHeight;
	var() FilterLimit	MaxHeight;

	var() FilterLimit	MinSlope;
	var() FilterLimit	MaxSlope;

	var() float				Alpha;
	var() TerrainMaterial	Material;

	structdefaultproperties
	{
		Alpha=1.0
	}
};

var() const array<TerrainFilteredMaterial> Materials;

cpptext
{
	/** returns the alpha that should be used in the weighting for the given material in this layer at the given world vertex
	 * @param Material the filtered material that is being weighted
	 * @param WorldVertex the world location of the vertex
	 * @return the alpha to use for weighting
	 */
	virtual FLOAT GetMaterialAlpha(const FTerrainFilteredMaterial* Material, const FVector& WorldVertex) { return Material->Alpha; }

	// UObject interface.

	virtual void PostEditChange(UProperty* PropertyThatChanged);
	
	/**
	 * Called after serialization. Ensures that there are only 64 materials.
	 */
	virtual void PostLoad();
}

/** Set the materials used for this layer
 * @note this function recaches the weight/displacement maps of affected terrain sections and is therefore slow, so use with caution
 * @param NewMaterials the new array of TerrainFilteredMaterials to replace the Materials array with
 */
native final function SetMaterials(array<TerrainFilteredMaterial> NewMaterials);

/** called from Terrain::PostBeginPlay() to allow the layer to initialize itself for gameplay
 * @note this function will be called once for each terrain the layer is part of
 */
simulated function PostBeginPlay();
