/*=============================================================================
	SpeedTreeStaticLighting.h: SpeedTreeComponent static lighting definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SPEEDTREESTATICLIGHTING_H__
#define __SPEEDTREESTATICLIGHTING_H__
#if WITH_SPEEDTREE

#include "SpeedTree.h"
#include "EngineMaterialClasses.h"

// Forward declarations.
class FSpeedTreeComponentStaticLighting;
class FSpeedTreeStaticLightingMesh;
class FSpeedTreeStaticLightingMapping;

/** Encapsulates ray tracing of a SpeedTreeComponent. */
class FStaticLightingSpeedTreeRayTracer
{
public:

	/** Initialization constructor. */
	FStaticLightingSpeedTreeRayTracer(
		const TArray<FStaticLightingVertex>& InVertices,
		const TArray<WORD>& InIndices,
		const FMeshElement& InMeshElement,
		FLOAT InOpacity
		):
		Vertices(InVertices),
		Indices(InIndices),
		MeshElement(InMeshElement),
		Opacity(InOpacity)
	{
	}

	/** Initializes the ray tracer. */
	void Init();

	/** @return TRUE if a line segment intersects the SpeedTree geometry. */
	FLightRayIntersection IntersectLightRay(const FVector& Start,const FVector& End) const;

	// kDOP data provider interface.
	FORCEINLINE const FVector& GetVertex(WORD Index) const
	{
		return Vertices(Index).WorldPosition;
	}
	FORCEINLINE UMaterialInstance* GetMaterial(WORD MaterialIndex) const
	{
		return NULL;
	}
	FORCEINLINE INT GetItemIndex(WORD MaterialIndex) const
	{
		return 0;
	}
	FORCEINLINE UBOOL ShouldCheckMaterial(INT MaterialIndex) const
	{
		return TRUE;
	}
	FORCEINLINE const TkDOPTree<const FStaticLightingSpeedTreeRayTracer,WORD>& GetkDOPTree(void) const
	{
		return kDopTree;
	}
	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return FMatrix::Identity;
	}
	FORCEINLINE const FMatrix& GetWorldToLocal(void) const
	{
		return FMatrix::Identity;
	}
	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return FMatrix::Identity;
	}
	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return 1.0f;
	}

private:

	TkDOPTree<const FStaticLightingSpeedTreeRayTracer,WORD> kDopTree;
	const TArray<FStaticLightingVertex>& Vertices;
	const TArray<WORD>& Indices;
	const FMeshElement MeshElement;
	FLOAT Opacity;
};

/** Represents a single LOD of a SpeedTreeComponent's mesh element to the static lighting system. */
class FSpeedTreeStaticLightingMesh : public FStaticLightingMesh
{
public:

	/** Initialization constructor. */
	FSpeedTreeStaticLightingMesh(
		FSpeedTreeComponentStaticLighting* InComponentStaticLighting,
		INT InLODIndex,
		const FMeshElement& InMeshElement,
		ESpeedTreeMeshType InMeshType,
		FLOAT InShadowOpacity,
		UBOOL bInTwoSidedMaterial,
		const TArray<ULightComponent*>& InRelevantLights,
		const FLightingBuildOptions& Options
		);

	// FStaticLightingMesh interface.
	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const;
	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;
	virtual UBOOL ShouldCastShadow(ULightComponent* Light,const FStaticLightingMapping* Receiver) const;
	virtual UBOOL IsUniformShadowCaster() const;
	virtual FLightRayIntersection IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindClosestIntersection) const;

private:

	/** The static lighting for the component this mesh is part of. */
	TRefCountPtr<FSpeedTreeComponentStaticLighting> ComponentStaticLighting;

	/** The LOD this mesh is part of. */
	INT LODIndex;

	/** The mesh element which this object represents to the static lighting system. */
	FMeshElement MeshElement;

	/** The type of this mesh. */
	ESpeedTreeMeshType MeshType;

	/** A helper object used to raytrace the mesh's triangles. */
	FStaticLightingSpeedTreeRayTracer RayTracer;
};

/** Represents the per-vertex static lighting of a single LOD of a SpeedTreeComponent's mesh element to the static lighting system. */
class FSpeedTreeStaticLightingMapping : public FStaticLightingVertexMapping
{
public:

	/** Initialization constructor. */
	FSpeedTreeStaticLightingMapping(FStaticLightingMesh* InMesh,FSpeedTreeComponentStaticLighting* InComponentStaticLighting,INT InLODIndex,const FMeshElement& InMeshElement,ESpeedTreeMeshType InMeshType);

	// FStaticLightingVertexMapping interface.
	virtual void Apply(FLightMapData1D* LightMapData,const TMap<ULightComponent*,FShadowMapData1D*>& ShadowMapData);

	// Accessors.
	INT GetLODIndex() const { return LODIndex; }
	const FMeshElement& GetMeshElement() const { return MeshElement; }
	ESpeedTreeMeshType GetMeshType() const { return MeshType; }

private:

	/** The static lighting for the component this mapping is part of. */
	TRefCountPtr<FSpeedTreeComponentStaticLighting> ComponentStaticLighting;

	/** The LOD this mapping is part of. */
	INT LODIndex;

	/** The mesh element which this object represents to the static lighting system. */
	FMeshElement MeshElement;

	/** The type of this mesh. */
	ESpeedTreeMeshType MeshType;
};

/** Manages the static lighting mappings for a single SpeedTreeComponent. */
class FSpeedTreeComponentStaticLighting : public FRefCountedObject
{
public:

	/** Initialization constructor. */
	FSpeedTreeComponentStaticLighting(USpeedTreeComponent* InComponent,const TArray<ULightComponent*>& InRelevantLights);

	/** Creates the mesh and mapping for a mesh of the SpeedTreeComponent. */
	void CreateMapping(
		FStaticLightingPrimitiveInfo& OutPrimitiveInfo,
		INT LODIndex,
		const FMeshElement& MeshElement,
		ESpeedTreeMeshType MeshType,
		FLOAT ShadowOpacity,
		UBOOL bInTwoSidedMaterial,
		const FLightingBuildOptions& Options
		);

	/**
	 * Applies a mapping's static lighting data.
	 * @param Mapping - The mapping which the static lighting data is for.
	 * @param MappingLightMapData - The mapping's light-map data.
	 * @param MappingShadowMapData - The mapping's shadow-map data.
	 */
	void ApplyCompletedMapping(
		FSpeedTreeStaticLightingMapping* Mapping,
		FLightMapData1D* MappingLightMapData,
		const TMap<ULightComponent*,FShadowMapData1D*>& MappingShadowMapData
		);

	// Accessors.
	USpeedTreeComponent* GetComponent() const
	{
		return Component;
	}
	const TArray<FStaticLightingVertex>& GetVertices(ESpeedTreeMeshType MeshType) const
	{
		return ChooseByMeshType<TArray<FStaticLightingVertex> >(MeshType,BranchAndFrondVertices,BranchAndFrondVertices,LeafMeshVertices,LeafCardVertices,BillboardVertices);
	}
	const TArray<WORD>& GetIndices() const
	{
		return Indices;
	}
	UBOOL IsMappingFromThisComponent(const FStaticLightingMapping* Mapping) const
	{
		return Mappings.ContainsItem((FSpeedTreeStaticLightingMapping*)Mapping);
	}

private:

	/** The component that mappings are being managed for. */
	USpeedTreeComponent* const Component;

	/** The component's relevant lights. */
	TArray<ULightComponent*> RelevantLights;

	/** The static lighting vertices of the component's branch and frond meshes. */
	TArray<FStaticLightingVertex> BranchAndFrondVertices;

	/** The static lighting vertices of the component's leaf meshes. */
	TArray<FStaticLightingVertex> LeafMeshVertices;

	/** The static lighting vertices of the component's leaf cards. */
	TArray<FStaticLightingVertex> LeafCardVertices;

	/** The static lighting vertices of the component's billboard meshes. */
	TArray<FStaticLightingVertex> BillboardVertices;

	/** The component's triangle vertex indices. */
	const TArray<WORD>& Indices;

	/** The light-map data for the branch and frond meshes. */
	FLightMapData1D BranchAndFrondLightMapData;
	
	/** The light-map data for the leaf meshes. */
	FLightMapData1D LeafMeshLightMapData;

	/** The light-map data for the leaf card. */
	FLightMapData1D LeafCardLightMapData;

	/** The light-map data for the billboard meshes. */
	FLightMapData1D BillboardLightMapData;

	/** TRUE if the component has branch and frond static lighting. */
	UBOOL bHasBranchAndFrondStaticLighting;

	/** TRUE if the component has leaf mesh static lighting. */
	UBOOL bHasLeafMeshStaticLighting;

	/** TRUE if the component has leaf card static lighting. */
	UBOOL bHasLeafCardStaticLighting;

	/** TRUE if the component has billboard static lighting. */
	UBOOL bHasBillboardStaticLighting;

	/** The shadow-map data for a single light. */
	class FLightShadowMaps : public FRefCountedObject
	{
	public:

		FShadowMapData1D BranchAndFrondShadowMapData;
		FShadowMapData1D LeafMeshShadowMapData;
		FShadowMapData1D LeafCardShadowMapData;
		FShadowMapData1D BillboardShadowMapData;

		/** Initialization constructor. */
		FLightShadowMaps(INT NumBranchAndFrondVertices,INT NumLeafMeshVertices,INT NumLeafCardVertices,INT NumBillboardVertices):
			BranchAndFrondShadowMapData(NumBranchAndFrondVertices),
			LeafMeshShadowMapData(NumLeafMeshVertices),
			LeafCardShadowMapData(NumLeafCardVertices),
			BillboardShadowMapData(NumBillboardVertices)
		{}
	};

	/** The shadow-map data for the component. */
	TMap<ULightComponent*,TRefCountPtr<FLightShadowMaps> > ComponentShadowMaps;

	/** All mappings for the component. */
	TArray<FSpeedTreeStaticLightingMapping*> Mappings;

	/** The number of mappings which have complete static lighting. */
	INT NumCompleteMappings;
};

#endif
#endif
