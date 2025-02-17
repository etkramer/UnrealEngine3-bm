/*=============================================================================
	UnTerrainRender.h: Definitions and inline code for rendering TerrainComponet
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef TERRAIN_RENDER_HEADER
#define TERRAIN_RENDER_HEADER

#include "FoliageRendering.h"
#include "ScenePrivate.h"
#include "GenericOctree.h"

#if __INTEL_BYTE_ORDER__ || PS3
	#define	UBYTE4_BYTEORDER_XYZW		1
#else
	#define	UBYTE4_BYTEORDER_XYZW		0
#endif

// Forward declarations.
class FDecalInteraction;
class FDecalState;
class FDynamicTerrainData;
class FTerrainComponentSceneProxy;
struct FTerrainObject;
class UTerrainComponent;

/**
 *	Flags identifying morphing setup.
 */
enum ETerrainMorphing
{
	/** No morphing is applied to this terrain.			*/
	ETMORPH_Disabled	= 0x00,
	/** Morph the terrain height.						*/
	ETMORPH_Height		= 0x01,
	/** Morph the terrain gradients.					*/
	ETMORPH_Gradient	= 0x02,
	/** Morph both the terrain height and gradients.	*/
	ETMORPH_Full		= ETMORPH_Height | ETMORPH_Gradient
};

/**
 *	FTerrainVertex
 *	The vertex structure used for terrain. 
 */
struct FTerrainVertex
{
	union
	{
		DWORD PackedCoordinates;
		struct
		{
		#if UBYTE4_BYTEORDER_XYZW
			BYTE	X,
					Y,
					Z_LOBYTE,
					Z_HIBYTE;
		#else
			BYTE	Z_HIBYTE,
					Z_LOBYTE,
					Y,
					X;
		#endif
		};
	};
	FLOAT	Displacement;
	SWORD	GradientX,
			GradientY;
};

/**
 *	FTerrainMorphingVertex
 *	Vertex structure used when morphing terrain.
 *	Contains the transitional Height values
 */
struct FTerrainMorphingVertex : FTerrainVertex
{
	union
	{
		DWORD PackedData;
		struct
		{
			#if UBYTE4_BYTEORDER_XYZW
				BYTE	TESS_DATA_INDEX_LO,
						TESS_DATA_INDEX_HI,
						Z_TRANS_LOBYTE,
						Z_TRANS_HIBYTE;
			#else
				BYTE	Z_TRANS_HIBYTE,
						Z_TRANS_LOBYTE,
						TESS_DATA_INDEX_HI,
						TESS_DATA_INDEX_LO;
			#endif
		};
	};
};

/**
 *	FTerrainFullMorphingVertex
 *	Vertex structure used when morphing terrain.
 *	Contains the transitional Height and Gradient values
 */
struct FTerrainFullMorphingVertex : FTerrainMorphingVertex
{
	SWORD	TransGradientX,
			TransGradientY;
};

//
//	FTerrainVertexBuffer
//
struct FTerrainVertexBuffer: FVertexBuffer
{
	/**
	 * Constructor.
	 * @param InMeshRenderData pointer to parent structure
	 */
	FTerrainVertexBuffer(const FTerrainObject* InTerrainObject, const UTerrainComponent* InComponent, INT InMaxTessellation, UBOOL bInIsDynamic = FALSE) :
		  bIsDynamic(bInIsDynamic)
		, TerrainObject(InTerrainObject)
		, Component(InComponent)
		, MaxTessellation(InMaxTessellation)
		, MaxVertexCount(0)
		, CurrentTessellation(-1)
		, VertexCount(0)
		, bRepackRequired(bInIsDynamic)
		, MorphingFlags(ETMORPH_Disabled)
	{
		if (InComponent)
		{
			ATerrain* Terrain = InComponent->GetTerrain();
			if (Terrain)
			{
				if (Terrain->bMorphingEnabled)
				{
					MorphingFlags = ETMORPH_Height;
					if (Terrain->bMorphingGradientsEnabled)
					{
						MorphingFlags = ETMORPH_Full;
					}
				}
			}
		}
	}

	// FRenderResource interface.
	virtual void InitRHI();

	/** 
	 * Initialize the dynamic RHI for this rendering resource 
	 */
	virtual void InitDynamicRHI();

	/** 
	 * Release the dynamic RHI for this rendering resource 
	 */
	virtual void ReleaseDynamicRHI();

	virtual FString GetFriendlyName() const
	{
		return TEXT("Terrain component vertices");
	}

	inline INT GetMaxTessellation()		{	return MaxTessellation;		}
	inline INT GetMaxVertexCount()		{	return MaxVertexCount;		}
	inline INT GetCurrentTessellation()	{	return CurrentTessellation;	}
	inline INT GetVertexCount()			{	return VertexCount;			}
	inline UBOOL GetRepackRequired()	{	return bRepackRequired;		}
	inline void ForceRepackRequired()	{	bRepackRequired = TRUE;		}
	inline void ClearRepackRequired()	{	bRepackRequired = FALSE;	}

	virtual void SetCurrentTessellation(INT InCurrentTessellation)
	{
		CurrentTessellation = Clamp<INT>(InCurrentTessellation, 0, MaxTessellation);
	}

	virtual UBOOL FillData(INT TessellationLevel);

private:
	/** Flag indicating it is dynamic						*/
	UBOOL				bIsDynamic;
	/** The owner terrain object							*/
	const FTerrainObject*		TerrainObject;
	/** The 'owner' component								*/
	const UTerrainComponent*	Component;
	/** The maximum tessellation to create vertices for		*/
	INT					MaxTessellation;
	/** The maximum number of vertices in the buffer		*/
	INT					MaxVertexCount;
	/** The maximum tessellation to create vertices for		*/
	INT					CurrentTessellation;
	/** The number of vertices in the buffer				*/
	INT					VertexCount;
	/** A repack is required								*/
	UBOOL				bRepackRequired;
	/** Flag indicating it is for morphing terrain			*/
	BYTE				MorphingFlags;
};


//
//	FTerrainFullVertexBuffer
//
struct FTerrainFullVertexBuffer : FTerrainVertexBuffer
{
	/**
	 * Constructor.
	 * @param InMeshRenderData pointer to parent structure
	 */
	FTerrainFullVertexBuffer(const FTerrainObject* InTerrainObject, const UTerrainComponent* InComponent, INT InMaxTessellation) :
		  FTerrainVertexBuffer(InTerrainObject, InComponent, InMaxTessellation, FALSE)
	{
	}

	// FRenderResource interface.
	virtual void InitRHI()
	{
		FTerrainVertexBuffer::InitRHI();
	}

	/** 
	 * Initialize the dynamic RHI for this rendering resource 
	 */
	virtual void InitDynamicRHI()
	{
		// Do NOTHING for the FullVertexBuffer
	}

	/** 
	 * Release the dynamic RHI for this rendering resource 
	 */
	virtual void ReleaseDynamicRHI()
	{
		// Do NOTHING for the FullVertexBuffer
	}

	virtual FString GetFriendlyName() const
	{
		return TEXT("Terrain FULL component vertices");
	}

	virtual void SetCurrentTessellation(INT InCurrentTessellation)
	{
		// Do NOTHING for the FullVertexBuffer
	}

	virtual UBOOL FillData(INT TessellationLevel)
	{
		return FTerrainVertexBuffer::FillData(TessellationLevel);
	}
};

// Forward declarations.
struct FTerrainIndexBuffer;
struct TerrainTessellationIndexBufferType;
struct TerrainDecalTessellationIndexBufferType;

struct FTerrainWireVertex
{
	FVector Position;
	FPackedNormal TangentX;
	FPackedNormal TangentZ;
	FVector2D UV;
};

class FTerrainWireVertexBuffer : public FVertexBuffer
{
public:

	/** Initialization constructor. */
	FTerrainWireVertexBuffer(UTerrainComponent* InComponent):
		Component(InComponent),
		NumVertices(0)
	{
	}

    // FRenderResource interface.
	virtual void InitRHI()
	{
		if (Component)
		{
			TArray<FVector> Vertices;
			TArray<INT> Indices;
			
			Component->GetCollisionData(Vertices, Indices);

			NumVertices = Vertices.Num();

			if(NumVertices)
			{
				VertexBufferRHI = RHICreateVertexBuffer(NumVertices * sizeof(FTerrainWireVertex),NULL,RUF_Static);

				FTerrainWireVertex* DestVertex = (FTerrainWireVertex*)RHILockVertexBuffer(VertexBufferRHI,0,NumVertices * sizeof(FTerrainWireVertex),FALSE);

				for (UINT VertIndex = 0; VertIndex < NumVertices; VertIndex++)
				{
					FVector& Vertex = Vertices(VertIndex);

					DestVertex->Position = Vertex;
					DestVertex->Position.X -= Component->SectionBaseX;
					DestVertex->Position.Y -= Component->SectionBaseY;
					DestVertex->TangentX = FVector(1,0,0);
					DestVertex->TangentZ = FVector(0,0,1);
					// TangentZ.w contains the sign of the tangent basis determinant. Assume +1
					DestVertex->TangentZ.Vector.W = 255;
					DestVertex->UV.X	 = 0.0f;
					DestVertex->UV.Y	 = 0.0f;
					DestVertex++;
				}
				RHIUnlockVertexBuffer(VertexBufferRHI);
			}
		}
	}

	// Accessors.
	UINT GetNumVertices() const
	{
		return NumVertices;
	}

private:
	UTerrainComponent* Component;
	UINT NumVertices;
};

class FTerrainWireIndexBuffer : public FIndexBuffer
{
public:

	/** Initialization constructor. */
	FTerrainWireIndexBuffer(UTerrainComponent* InComponent):
		Component(InComponent),
		NumTriangles(0)
	{
	}

	// FRenderResource interface.
	virtual void InitRHI()
	{
		if (Component)
		{
			TArray<FVector> Vertices;
			TArray<INT> Indices;
			
			Component->GetCollisionData(Vertices, Indices);

			NumTriangles = Indices.Num() / 3;

			if (NumTriangles)
			{
				IndexBufferRHI = RHICreateIndexBuffer(sizeof(WORD), NumTriangles * 3 * sizeof(WORD),NULL,RUF_Static);

				WORD* DestIndex = (WORD*)RHILockIndexBuffer(IndexBufferRHI,0, NumTriangles * 3 * sizeof(WORD));
				
				for (INT Index = 0; Index < Indices.Num(); Index++)
				{
					*DestIndex++ = (WORD)(Indices(Index));
				}
				RHIUnlockIndexBuffer(IndexBufferRHI);
			}
		}
	}

	// Accessors.
	UINT GetNumTriangles() const
	{
		return NumTriangles;
	}

private:
	UTerrainComponent* Component;
	UINT NumTriangles;
};

//
//	FTerrainObject
//
struct FTerrainObject : public FDeferredCleanupInterface
{
public:
	FTerrainObject(UTerrainComponent* InTerrainComponent, INT InMaxTessellation) :
		  bIsInitialized(FALSE)
		, bIsDeadInGameThread(FALSE)
		, bRepackRequired(TRUE)
		, MorphingFlags(ETMORPH_Disabled)
	    , TerrainComponent(InTerrainComponent)
		, TessellationLevels(NULL)
		, ScaleFactorX(1.0f)
		, ScaleFactorY(1.0f)
		, VertexFactory(NULL)
		, DecalVertexFactory(NULL)
		, VertexBuffer(NULL)
		, FullVertexBuffer(NULL)
		, FullIndexBuffer(NULL)
		, CollisionVertexFactory(NULL)
		, CollisionVertexBuffer(NULL)
		, CollisionIndexBuffer(NULL)
		, CollisionWireframeMaterialInstance(NULL)
	{
		check(TerrainComponent);
		ATerrain* Terrain = TerrainComponent->GetTerrain();
		if (Terrain)
		{
			ScaleFactorX = Terrain->DrawScale3D.Z / Terrain->DrawScale3D.X;
			ScaleFactorY = Terrain->DrawScale3D.Z / Terrain->DrawScale3D.Y;
			if (Terrain->bMorphingEnabled)
			{
				MorphingFlags = ETMORPH_Height;
				if (Terrain->bMorphingGradientsEnabled)
				{
					MorphingFlags = ETMORPH_Full;
				}
			}
		}
		Init();
	}
	
	virtual ~FTerrainObject();

	void Init();

	virtual void InitResources();
	virtual void ReleaseResources();
	virtual void Update();
	virtual const FVertexFactory* GetVertexFactory() const;

	//@todo.SAS. Remove this! START
	UBOOL GetIsDeadInGameThread()
	{
		return bIsDeadInGameThread;
	}
	void SetIsDeadInGameThread(UBOOL bInIsDeadInGameThread)
	{
		bIsDeadInGameThread = bInIsDeadInGameThread;
	}
	//@todo.SAS. Remove this! STOP

	const UBOOL GetRepackRequired() const
	{
		return bRepackRequired;
	}

	void SetRepackRequired(UBOOL bInRepackRequired)
	{
		bRepackRequired = bInRepackRequired;
	}

#if 1	//@todo. Remove these as we depend on the component anyway!
	inline INT		GetComponentSectionSizeX() const		{	return ComponentSectionSizeX;		}
	inline INT		GetComponentSectionSizeY() const		{	return ComponentSectionSizeY;		}
	inline INT		GetComponentSectionBaseX() const		{	return ComponentSectionBaseX;		}
	inline INT		GetComponentSectionBaseY() const		{	return ComponentSectionBaseY;		}
	inline INT		GetComponentTrueSectionSizeX() const	{	return ComponentTrueSectionSizeX;	}
	inline INT		GetComponentTrueSectionSizeY() const	{	return ComponentTrueSectionSizeY;	}
#endif	//#if 1	//@todo. Remove these as we depend on the component anyway!
	inline INT		GetNumVerticesX() const					{	return NumVerticesX;				}
	inline INT		GetNumVerticesY() const					{	return NumVerticesY;				}
	inline INT		GetMaxTessellationLevel() const			{	return MaxTessellationLevel;		}
	inline FLOAT	GetTerrainHeightScale() const			{	return TerrainHeightScale;			}
	inline FLOAT	GetTessellationDistanceScale() const	{	return TessellationDistanceScale;	}
	inline BYTE		GetTessellationLevel(INT Index) const	{	return TessellationLevels[Index];	}
	inline FLOAT	GetScaleFactorX() const					{	return ScaleFactorX;				}
	inline FLOAT	GetScaleFactorY() const					{	return ScaleFactorY;				}
	inline INT		GetLightMapResolution() const			{	return LightMapResolution;			}
	inline FLOAT	GetShadowCoordinateScaleX() const		{	return ShadowCoordinateScale.X;		}
	inline FLOAT	GetShadowCoordinateScaleY() const		{	return ShadowCoordinateScale.Y;		}
	inline FLOAT	GetShadowCoordinateBiasX() const		{	return ShadowCoordinateBias.X;		}
	inline FLOAT	GetShadowCoordinateBiasY() const		{	return ShadowCoordinateBias.Y;		}
	inline FMatrix&	GetLocalToWorld() const				
	{
		check(TerrainComponent);
		return TerrainComponent->LocalToWorld;
	}

	inline void		SetShadowCoordinateScale(const FVector2D& InShadowCoordinateScale)
	{
		ShadowCoordinateScale = InShadowCoordinateScale;
	}
	inline void		SetShadowCoordinateBias(const FVector2D& InShadowCoordinateBias)
	{
		ShadowCoordinateBias = InShadowCoordinateBias;
	}

	/** Called by FTerrainComponentSceneProxy; repacks vertex and index buffers as needed. */
	UBOOL UpdateResources_RenderingThread(INT TessellationLevel, TArray<FDecalInteraction*>& ProxyDecals);

	// FDeferredCleanupInterface
	virtual void FinishCleanup()
	{
		delete this;
	}

	ATerrain* GetTerrain()
	{
		return TerrainComponent->GetTerrain();
	}

	const ATerrain* GetTerrain() const
	{
		return TerrainComponent->GetTerrain();
	}

	/** Adds a decal interaction to the game object. */
	void AddDecalInteraction_RenderingThread(FDecalInteraction& DecalInteraction, UINT ProxyMaxTesellation);
	FDecalRenderData* GenerateDecalRenderData(class FDecalState* Decal) const;

	// allow access to mesh component
	friend class FDynamicTerrainData;
	friend class FTerrainComponentSceneProxy;
	friend struct FTerrainIndexBuffer;
	template<typename TerrainQuadRelevance> friend struct FTerrainTessellationIndexBuffer;
	friend struct FTerrainDetessellationIndexBuffer;

protected:
	/** Set to TRUE in InitResources() and FALSE in ReleaseResources()	*/
	UBOOL					bIsInitialized;
	/** Debugging flag...												*/
	UBOOL					bIsDeadInGameThread;
	UBOOL					bRepackRequired;
	/** Morphing is enabled flag...										*/
	BYTE					MorphingFlags;
	/** The owner component												*/
	UTerrainComponent*		TerrainComponent;

	/** The component section size and base (may not need these...)		*/
#if 1	//@todo. Remove these as we depend on the component anyway!
	INT						ComponentSectionSizeX;
	INT						ComponentSectionSizeY;
	INT						ComponentSectionBaseX;
	INT						ComponentSectionBaseY;
	INT						ComponentTrueSectionSizeX;
	INT						ComponentTrueSectionSizeY;
#endif	//#if 1	//@todo. Remove these as we depend on the component anyway!
	INT						NumVerticesX;
	INT						NumVerticesY;
	/** The maximum tessellation level of the terrain					*/
	INT						MaxTessellationLevel;
	/** The minimum tessellation level of the terrain					*/
	INT						MinTessellationLevel;
	/** The editor-desired tessellation level to display at				*/
	INT						EditorTessellationLevel;
	FLOAT					TerrainHeightScale;
	FLOAT					TessellationDistanceScale;
	INT						LightMapResolution;
	FVector2D				ShadowCoordinateScale;
	FVector2D				ShadowCoordinateBias;
	/** Copy of the ATerrain::NumPatchesX. */
	INT						NumPatchesX;
	/** Copy of the ATerrain::NumPatchesY. */
	INT						NumPatchesY;

	/** The TessellationLevels arrays (per-batch)						*/
	BYTE*								TessellationLevels;

	/** The parent scale factors... */
	FLOAT					ScaleFactorX;
	FLOAT					ScaleFactorY;

	/** The vertex factory												*/
	FTerrainVertexFactory*				VertexFactory;
	/** The decal vertex factory										*/
	FTerrainDecalVertexFactoryBase*		DecalVertexFactory;
	/** The vertex buffer containing the vertices for the component		*/
	FTerrainVertexBuffer*				VertexBuffer;
	/** The index buffers for each batch material						*/
	TerrainTessellationIndexBufferType*	SmoothIndexBuffer;
	/** The material resources for each batch							*/
	TArray<FMaterialRenderProxy*>		BatchMaterialResources;

	/** For rendering at full-patch (lowest tessellation)				*/
	FTerrainVertexFactory				FullVertexFactory;
	FTerrainDecalVertexFactory			FullDecalVertexFactory;
	FTerrainFullVertexBuffer*			FullVertexBuffer;
	FTerrainIndexBuffer*				FullIndexBuffer;
	FMaterialRenderProxy*				FullMaterialResource;

	/** For rendering the terrain collision */
	FLocalVertexFactory*				CollisionVertexFactory;
	FTerrainWireVertexBuffer*			CollisionVertexBuffer;
	FTerrainWireIndexBuffer*			CollisionIndexBuffer;
	FColoredMaterialRenderProxy*		CollisionWireframeMaterialInstance;

	void RepackDecalIndexBuffers_RenderingThread(INT TessellationLevel, INT MaxTessellation, TArray<FDecalInteraction*>& Decals);
	void ReinitDecalResources_RenderThread();
};

//
//	FTerrainIndexBuffer
//
struct FTerrainIndexBuffer: FIndexBuffer
{
	const FTerrainObject* TerrainObject;
	INT	SectionSizeX;
	INT	SectionSizeY;
	INT NumVisibleTriangles;

	// Constructor.
	FTerrainIndexBuffer(const FTerrainObject* InTerrainObject) :
		  TerrainObject(InTerrainObject)
		, SectionSizeX(InTerrainObject->GetComponentSectionSizeX())
		, SectionSizeY(InTerrainObject->GetComponentSectionSizeY())
		, NumVisibleTriangles(INDEX_NONE)
	{
	}

	// FRenderResource interface.
	virtual void InitRHI();

	virtual FString GetFriendlyName() const
	{
		return TEXT("Terrain component indices (full batch)");
	}
};

/** An instance of a terrain foliage mesh. */
struct FTerrainFoliageInstance
{
	FVector Location;
    FVector XAxis;
    FVector YAxis;
    FVector ZAxis;
	FVector2D StaticLightingTexCoord;
	FLOAT BoundingRadius;
	FLOAT DistanceFactorSquared;
};

/** The type of the octree which holds the component's foliage instances. */
typedef TOctree<FTerrainFoliageInstance,struct FTerrainFoliageInstanceOctreeSemantics> FTerrainFoliageInstanceOctree;

/** The octree semantics for foliage instances. */
struct FTerrainFoliageInstanceOctreeSemantics
{
	enum { MaxElementsPerLeaf = 33 };
	enum { MaxNodeDepth = 5 };

	static FBoxCenterAndExtent GetBoundingBox(const FTerrainFoliageInstance& FoliageInstance)
	{
		return FBoxCenterAndExtent(
			FoliageInstance.Location,
			FVector(FoliageInstance.BoundingRadius,FoliageInstance.BoundingRadius,FoliageInstance.BoundingRadius)
			);
	}
		
	static void SetElementId(const FTerrainFoliageInstance& Element,FOctreeElementId Id)
	{
	}
};

/** Implements the hooks used by the common foliage rendering code to build the render data for a TerrainComponent's foliage. */
class FTerrainFoliageRenderDataPolicy
{
public:

	typedef FTerrainFoliageInstance InstanceType;
	typedef const FTerrainFoliageInstance* InstanceReferenceType;

	/** The time since the foliage was last rendered. */
	FLOAT TimeSinceLastRendered;

	/** Initialization constructor. */
	FTerrainFoliageRenderDataPolicy(const FTerrainFoliageMesh* InMesh,const FBox& InComponentBoundingBox):
		TimeSinceLastRendered(0.0f),
		Mesh(InMesh),
		InstanceOctree(InComponentBoundingBox.GetCenter(),InComponentBoundingBox.GetExtent().GetMax()),
		NumInstances(0),
		StaticMeshRenderData(InMesh->StaticMesh->LODModels(0)),
		MinTransitionRadiusSquared(Square(InMesh->MinTransitionRadius)),
		InvTransitionSize(1.0f / Max(InMesh->MaxDrawRadius - InMesh->MinTransitionRadius,DELTA))
	{
	}

	/**
	* Creates the foliage instances for this mesh+component pair.  Fills in the Instances and Vertices arrays.
	* @param Mesh - The mesh being instances.
	* @param Component - The terrain component which the mesh is being instanced on.
	* @param WeightedMaterial - The terrain weightmap which controls this foliage mesh's density.
	*/
	void InitInstances(const UTerrainComponent* Component,const FTerrainWeightedMaterial& WeightedMaterial);

	/**
	 * Computes the set of instances which are visible in a specific view.
	 * @param View - The view to find visible instances for.
	 * @param OutVisibleFoliageInstances - Upon return, contains the foliage instance indices which are visible in the view.
	 */
	void GetVisibleFoliageInstances(const FSceneView* View,TArray<const FTerrainFoliageInstance*>& OutVisibleFoliageInstances);

	/**
	 * Accesses an instance for a specific view.  The instance is scaled according to the view.
	 * @param InstanceReference - A reference to the instance to access.
	 * @param View - The view to access the instance for.
	 * @param OutInstance - Upon return, contains the instance data.
	 * @param OutWindScale - Upon return, contains the amount to scale the wind skew by.
	 */
	void GetInstance(InstanceReferenceType InstanceReference,const FSceneView* View,InstanceType& OutInstance,FLOAT& OutWindScale) const;

	/**
	 * Sets up the foliage vertex factory for the instance's static lighting data.
	 * @param InstanceBuffer - The vertex buffer containing the instance data.
	 * @param OutData - Upon return, the static lighting components are set for the terrain foliage rendering.
	 */
	void InitVertexFactoryStaticLighting(FVertexBuffer* InstanceBuffer,FFoliageVertexFactory::DataType& OutData);

	/** @return The maximum number of instances which will be rendered. */
	INT GetMaxInstances() const
	{
		return NumInstances;
	}

	/** @return The static mesh render data for a single foliage mesh. */
	const FStaticMeshRenderData& GetMeshRenderData() const
	{
		return StaticMeshRenderData;
	}

private:

	/** The mesh that is being instanced. */
	const FTerrainFoliageMesh* Mesh;

	/** An octree holding the component's foliage instances. */
	FTerrainFoliageInstanceOctree InstanceOctree;

	/** The total number of instances. */
	INT NumInstances;

	/** The static mesh render data for a single foliage mesh. */
	const FStaticMeshRenderData& StaticMeshRenderData;

	/** The squared distance that foliage instances begin to scale down at. */
	const FLOAT MinTransitionRadiusSquared;

	/** The inverse difference between the distance where foliage instances begin to scale down, and the distance where they disappear. */
	const FLOAT InvTransitionSize;
};

inline DWORD GetTypeHash(const FTerrainFoliageMesh* Mesh)
{
	return PointerHash(Mesh);
}

//
//	FTerrainComponentSceneProxy
//
class FTerrainComponentSceneProxy : public FPrimitiveSceneProxy, public FTickableObject
{
private:
	class FTerrainComponentInfo : public FLightCacheInterface
	{
	public:

		/** Initialization constructor. */
		FTerrainComponentInfo(const UTerrainComponent& Component)
		{
			// Build the static light interaction map.
			for (INT LightIndex = 0; LightIndex < Component.IrrelevantLights.Num(); LightIndex++)
			{
				StaticLightInteractionMap.Set(Component.IrrelevantLights(LightIndex), FLightInteraction::Irrelevant());
			}
			
			LightMap = Component.LightMap;
			if (LightMap)
			{
				for (INT LightIndex = 0; LightIndex < LightMap->LightGuids.Num(); LightIndex++)
				{
					StaticLightInteractionMap.Set(LightMap->LightGuids(LightIndex), FLightInteraction::LightMap());
				}
			}

			for (INT LightIndex = 0; LightIndex < Component.ShadowMaps.Num(); LightIndex++)
			{
				UShadowMap2D* ShadowMap = Component.ShadowMaps(LightIndex);
				if (ShadowMap && ShadowMap->IsValid())
				{
					StaticLightInteractionMap.Set(
						ShadowMap->GetLightGuid(),
						FLightInteraction::ShadowMap2D(
							ShadowMap->GetTexture(),
							ShadowMap->GetCoordinateScale(),
							ShadowMap->GetCoordinateBias()
							)
						);

					Component.TerrainObject->SetShadowCoordinateBias(ShadowMap->GetCoordinateBias());
					Component.TerrainObject->SetShadowCoordinateScale(ShadowMap->GetCoordinateScale());
				}
			}
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
		{
			// Check for a static light interaction.
			const FLightInteraction* Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightmapGuid);
			if (!Interaction)
			{
				Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightGuid);
			}
			return Interaction ? *Interaction : FLightInteraction::Uncached();
		}

		virtual FLightMapInteraction GetLightMapInteraction() const
		{
			return LightMap ? LightMap->GetInteraction() : FLightMapInteraction();
		}

	private:
		/** A map from persistent light IDs to information about the light's interaction with the model element. */
		TMap<FGuid,FLightInteraction> StaticLightInteractionMap;

		/** The light-map used by the element. */
		const FLightMap* LightMap;
	};

	/** */
	struct FTerrainBatchInfo
	{
		FTerrainBatchInfo(UTerrainComponent* Component, INT BatchIndex);
		~FTerrainBatchInfo();

        FMaterialRenderProxy* MaterialRenderProxy;
		UBOOL bIsTerrainMaterialResourceInstance;
		TArray<UTexture2D*> WeightMaps;
	};

	struct FTerrainMaterialInfo
	{
		FTerrainMaterialInfo(UTerrainComponent* Component);
		~FTerrainMaterialInfo();
		
		TArray<FTerrainBatchInfo*> BatchInfoArray;
		FTerrainComponentInfo* ComponentLightInfo;
	};

public:
	/** Initialization constructor. */
	FTerrainComponentSceneProxy(UTerrainComponent* Component, FLOAT InCheckTessellationDistance, WORD InCheckTessellationOffset = 0);
	~FTerrainComponentSceneProxy();

	virtual void InitLitDecalFlags(UINT InDepthPriorityGroup);

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

	// FPrimitiveSceneProxy interface.
	
	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags);

	/**
	* Draws the primitive's dynamic decal elements.  This is called from the rendering thread for each frame of each view.
	* The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	* Called in the rendering thread.
	*
	* @param	PDI						The interface which receives the primitive elements.
	* @param	View					The view which is being rendered.
	* @param	InDepthPriorityGroup	The DPG which is being rendered.
	* @param	bDynamicLightingPass	TRUE if drawing dynamic lights, FALSE if drawing static lights.
	* @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	*/
	virtual void DrawDynamicDecalElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		UBOOL bDynamicLightingPass,
		UBOOL bTranslucentReceiverPass
		);

	/**
	* Draws the primitive's static decal elements.  This is called from the game thread whenever this primitive is attached
	* as a receiver for a decal.
	*
	* The static elements will only be rendered if GetViewRelevance declares both static and decal relevance.
	* Called in the game thread.
	*
	* @param PDI - The interface which receives the primitive elements.
	*/
	virtual void DrawStaticDecalElements(
		FStaticPrimitiveDrawInterface* PDI,
		const FDecalInteraction& DecalInteraction
		);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	 *	Determines the relevance of this primitive's elements to the given light.
	 *	@param	LightSceneInfo			The light to determine relevance for
	 *	@param	bDynamic (output)		The light is dynamic for this primitive
	 *	@param	bRelevant (output)		The light is relevant for this primitive
	 *	@param	bLightMapped (output)	The light is light mapped for this primitive
	 */
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped);

	/**
	 *	Helper function for determining if a given view requires a tessellation check based on distance.
	 *
	 *	@param	View		The view of interest.
	 *	@return	UBOOL		TRUE if it does, FALSE if it doesn't.
	 */
	UBOOL CheckViewDistance(const FSceneView* View, const FVector& Position, const FVector& MaxMinusMin, const FLOAT ComponentSize);
	/**
	 *	Helper function for calculating the tessellation for a given view.
	 *
	 *	@param	View		The view of interest.
	 *	@param	Terrain		The terrain of interest.
	 *	@param	bFirstTime	The first time this call was made in a frame.
	 */
	void ProcessPreRenderView(const FSceneView* View, ATerrain* Terrain, UBOOL bFirstTime);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber);

	/**
	 *	Called when the rendering thread adds the proxy to the scene.
	 *	This function allows for generating renderer-side resources.
	 */
	virtual UBOOL CreateRenderThreadResources();

	/**
	 *	Called when the rendering thread removes the dynamic data from the scene.
	 */
	virtual UBOOL ReleaseRenderThreadResources();

	// FTickableObject interface.
	virtual void Tick( FLOAT DeltaTime );
	virtual UBOOL IsTickable() const
	{
		return bInitialized;
	}

	void UpdateData(UTerrainComponent* Component);
	void UpdateData_RenderThread(FTerrainMaterialInfo* NewMaterialInfo);

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

	/** @return			Cached value of MaxTessellationLevel, as computed in DrawDynamicElements. */
	UINT GetMaxTessellation() const
	{
		return MaxTessellation;
	}

protected:
	AActor* GetOwner();

private:
	/** Counter to determine when to check the tessellation */
	INT CheckTessellationCounter;
	/** Random offset to check tessellation */
	INT CheckTessellationOffset;
	/** The last frame that the tessellation check was performed */
	INT LastTessellationCheck;
	/**
	 *	The radius from the view origin that terrain tessellation checks should be performed.
	 *	If 0.0, every component will be checked for tessellation changes each frame.
	 */
	FLOAT CheckTessellationDistance;

	/**
	 *	To catch when visibility changes...
	 */
	FLOAT TrackedLastVisibilityChangeTime;

	AActor* Owner;
	UTerrainComponent* ComponentOwner;

	FTerrainObject* TerrainObject;

	UBOOL bSelected;

	FLinearColor LinearLevelColor;
	FLinearColor LinearPropertyColor;

	FLOAT CullDistance;

	BITFIELD bCastShadow : 1;
	/** TRUE once this scene proxy has been added to the scene */
	BITFIELD bInitialized : 1;

	FColoredMaterialRenderProxy SelectedWireframeMaterialInstance;
	FColoredMaterialRenderProxy DeselectedWireframeMaterialInstance;

	FTerrainMaterialInfo*	CurrentMaterialInfo;

	/** Cache of MaxTessellationLevel, as computed in DrawDynamicElements. */
	UINT MaxTessellation;

	/** Array of meshes, one for each batch material.  Populated by DrawDynamicElements. */
	TArray<FMeshElement> Meshes;

	/** The component's foliage instances. */
	TMap<const FTerrainFoliageMesh*,TFoliageRenderData<FTerrainFoliageRenderDataPolicy>*> FoliageRenderDataSet;

	/** The component's foliage mesh materials. */
	TMap<const FTerrainFoliageMesh*,const UMaterialInterface*> FoliageMeshToMaterialMap;

	/** The foliage materials' view relevance. */
	FMaterialViewRelevance FoliageMaterialViewRelevance;

	/** Draws the component's foliage. */
	void DrawFoliage(FPrimitiveDrawInterface* PDI,const FSceneView* View, UINT DPGIndex);

	/**
	 * Check for foliage that hasn't been rendered recently, and free its data.
	 * @param DeltaTime - The time since the last update.
	 */
	void TickFoliage(FLOAT DeltaTime);

	/** Releases all foliage render data. */
	void ReleaseFoliageRenderData();
};

#endif	//#ifndef TERRAIN_RENDER_HEADER
