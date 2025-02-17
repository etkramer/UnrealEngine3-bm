/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class TerrainMaterial extends Object
	native(Terrain)
	hidecategories(Object);

var matrix					LocalToMapping;

enum ETerrainMappingType
{
	TMT_Auto,
	TMT_XY,
	TMT_XZ,
	TMT_YZ
};

var(Material) ETerrainMappingType	MappingType;
var(Material) float					MappingScale;
var(Material) float					MappingRotation;
var(Material) float					MappingPanU,
									MappingPanV;

var(Material) MaterialInterface		Material;

var(Displacement) Texture2D			DisplacementMap;
var(Displacement) float				DisplacementScale;

struct native TerrainFoliageMesh
{
	var() StaticMesh		StaticMesh;
	var() MaterialInterface	Material;
	var() int				Density;
	var() float				MaxDrawRadius;
	var() float				MinTransitionRadius;
	var() float				MinScale;
	var() float				MaxScale;
	var() float				MinUniformScale;
	var() float				MaxUniformScale;
	var() float				MinThinningRadius;
	var() int				Seed;
	var() float				SwayScale;

	/** The weight of the terrain material above which the foliage is spawned. */
	var() float				AlphaMapThreshold;
	
	/**	
	 *	The amount to rotate the mesh to match the slope of the terrain 
	 *	where it is being placed. If 1.0, the mesh will match the slope
	 *	exactly.
	 */
	var() float				SlopeRotationBlend;

	structdefaultproperties
	{
		MaxDrawRadius=1024.0
		MinThinningRadius=1024.0
		MinScale=1.0
		MaxScale=1.0
		MinUniformScale=1.0
		MaxUniformScale=1.0
		SwayScale=1.0
	}
};

var(Foliage) array<TerrainFoliageMesh>	FoliageMeshes;

cpptext
{
	// UpdateMappingTransform

	void UpdateMappingTransform();

	// Displacement sampler.

	FLOAT GetDisplacement(BYTE* DisplacementData,FLOAT U,FLOAT V) const;

	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostLoad();
}

defaultproperties
{
	MappingScale=4.0
	DisplacementScale=0.25
}

