/*=============================================================================
	Scene.h: Scene manager definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes the draw mesh macros
#include "UnSceneUtils.h"

// enums are already declared in Engine.h
#define NO_ENUMS 1
#include "EngineSceneClasses.h"
#undef NO_ENUMS

// Forward declarations.
class FSceneViewFamily;
class FSceneInterface;
class FLightSceneInfo;
class ULightComponent;
class USceneCaptureComponent;
class FCaptureSceneInfo;
class UPostProcessChain;

//
//	EShowFlags
//

typedef QWORD EShowFlags;

#define	SHOW_MissingCollisionModel	DECLARE_UINT64(0x0000000000000001)	// Show any static meshes with collision on but without a collision model in bright colour.
#define	SHOW_Decals					DECLARE_UINT64(0x0000000000000002)	// Draw decals.
#define	SHOW_DecalInfo				DECLARE_UINT64(0x0000000000000004)	// Draw decal dev info (frustums, tangent axes, etc).
#define	SHOW_LightRadius			DECLARE_UINT64(0x0000000000000008)	// Draw point light radii.
#define	SHOW_AudioRadius			DECLARE_UINT64(0x0000000000000010)	// Draw sound actor radii.
#define	SHOW_DynamicShadows			DECLARE_UINT64(0x0000000000000020)	// Draw dynamic shadows.
#define SHOW_PostProcess			DECLARE_UINT64(0x0000000000000040)  // Draw post process effects
#define SHOW_BSPSplit				DECLARE_UINT64(0x0000000000000080)	// Colors BSP based on model component association.
#define SHOW_SceneCaptureUpdates	DECLARE_UINT64(0x0000000000000100)	// Update scene capture probes
#define SHOW_Sprites				DECLARE_UINT64(0x0000000000000200)	// Draw sprite components
#define SHOW_LevelColoration		DECLARE_UINT64(0x0000000000000400)  // Render objects with colors based on what the level they belong to.

#define SHOW_Wireframe				DECLARE_UINT64(0x0000000000000800)
#define SHOW_Lighting				DECLARE_UINT64(0x0000000000001000)
#define SHOW_Materials				DECLARE_UINT64(0x0000000000002000)
#define SHOW_LightComplexity		DECLARE_UINT64(0x0000000000004000)
#define SHOW_Brushes				DECLARE_UINT64(0x0000000000008000)

#define	SHOW_CollisionNonZeroExtent	DECLARE_UINT64(0x0000000000010000)
#define	SHOW_CollisionZeroExtent	DECLARE_UINT64(0x0000000000020000)
#define	SHOW_CollisionRigidBody		DECLARE_UINT64(0x0000000000040000)
#define SHOW_Collision_Any			(SHOW_CollisionNonZeroExtent | SHOW_CollisionZeroExtent | SHOW_CollisionRigidBody)

#define SHOW_PropertyColoration		DECLARE_UINT64(0x0000000000080000)  // Render objects with colors based on the property values.
#define SHOW_StreamingBounds		DECLARE_UINT64(0x0000000000100000)	// Render streaming bounding volumes for the currently selected texture

#define SHOW_TextureDensity			DECLARE_UINT64(0x0000000000200000)	// Colored according to world-space texture density.

#define SHOW_TerrainCollision		DECLARE_UINT64(0x0000000000400000)	// Renders the terrain collision mesh overlay
#define SHOW_ShaderComplexity		DECLARE_UINT64(0x0000000000800000)  // Renders world colored by shader complexity
#define SHOW_SpeedTrees				DECLARE_UINT64(0x0000000001000000)	// Renders SpeedTrees
#define SHOW_LensFlares				DECLARE_UINT64(0x0000000002000000)	// Renders LensFlares
#define	SHOW_LowDetail				DECLARE_UINT64(0x0000000004000000)
#define	SHOW_MedDetail				DECLARE_UINT64(0x0000000008000000)
#define	SHOW_AllDetail				(SHOW_LowDetail | SHOW_MedDetail);
#define SHOW_MotionBlur				DECLARE_UINT64(0x0000000010000000)	// Enables motion blur post process effects (disabled for non-realtime editor viewports) (SHOW_PostProcess must also be enabled!)
#define SHOW_SentinelStats			DECLARE_UINT64(0x0000000020000000)	// Render stats from Sentinel stats viewer in editor
#define	SHOW_Editor					DECLARE_UINT64(0x0000000100000000)
#define	SHOW_Game					DECLARE_UINT64(0x0000000200000000)
#define	SHOW_Collision				DECLARE_UINT64(0x0000000400000000)
#define	SHOW_Grid					DECLARE_UINT64(0x0000000800000000)
#define	SHOW_Selection				DECLARE_UINT64(0x0000001000000000)
#define	SHOW_Bounds					DECLARE_UINT64(0x0000002000000000)
#define	SHOW_StaticMeshes			DECLARE_UINT64(0x0000004000000000)
#define	SHOW_Terrain				DECLARE_UINT64(0x0000008000000000)
#define	SHOW_BSP					DECLARE_UINT64(0x0000010000000000)	// Draws BSP brushes.
#define	SHOW_SkeletalMeshes 		DECLARE_UINT64(0x0000020000000000)
#define	SHOW_Constraints			DECLARE_UINT64(0x0000040000000000)
#define	SHOW_Fog					DECLARE_UINT64(0x0000080000000000)
#define	SHOW_Foliage				DECLARE_UINT64(0x0000100000000000)
#define	SHOW_BSPTriangles			DECLARE_UINT64(0x0000200000000000)  // Draws BSP triangles.
#define	SHOW_Paths					DECLARE_UINT64(0x0000400000000000)
#define	SHOW_MeshEdges				DECLARE_UINT64(0x0000800000000000)	// In the filled view modes, render mesh edges as well as the filled surfaces.
#define	SHOW_LargeVertices			DECLARE_UINT64(0x0001000000000000)	// Displays large clickable icons on static mesh vertices
#define SHOW_UnlitTranslucency		DECLARE_UINT64(0x0002000000000000)	// Render unlit translucency
#define	SHOW_Portals				DECLARE_UINT64(0x0004000000000000)
#define	SHOW_HitProxies				DECLARE_UINT64(0x0008000000000000)	// Draws each hit proxy in the scene with a different color.
#define	SHOW_ShadowFrustums			DECLARE_UINT64(0x0010000000000000)	// Draws un-occluded shadow frustums as 
#define	SHOW_ModeWidgets			DECLARE_UINT64(0x0020000000000000)	// Draws mode specific widgets and controls in the viewports (should only be set on viewport clients that are editing the level itself)
#define	SHOW_KismetRefs				DECLARE_UINT64(0x0040000000000000)	// Draws green boxes around actors in level which are referenced by Kismet. Only works in editor.
#define	SHOW_Volumes				DECLARE_UINT64(0x0080000000000000)	// Draws Volumes
#define	SHOW_CamFrustums			DECLARE_UINT64(0x0100000000000000)	// Draws camera frustums
#define	SHOW_NavigationNodes		DECLARE_UINT64(0x0200000000000000)	// Draws actors associated with path noding
#define	SHOW_Particles				DECLARE_UINT64(0x0400000000000000)	// Draws particles
#define SHOW_LightInfluences		DECLARE_UINT64(0x0800000000000000)	// Visualize light influences
#define	SHOW_BuilderBrush			DECLARE_UINT64(0x1000000000000000)	// Draws the builder brush wireframe
#define	SHOW_TerrainPatches			DECLARE_UINT64(0x2000000000000000)	// Draws an outline around each terrain patch
#define	SHOW_Cover					DECLARE_UINT64(0x4000000000000000)	// Complex cover rendering
#define	SHOW_ActorTags				DECLARE_UINT64(0x8000000000000000)	// Draw an Actors Tag next to it in the viewport. Only works in the editor.


#define	SHOW_DefaultGame	(SHOW_Game | SHOW_StaticMeshes | SHOW_SkeletalMeshes | SHOW_Terrain | SHOW_SpeedTrees | SHOW_Foliage | SHOW_BSPTriangles | SHOW_BSP | SHOW_Fog | SHOW_Particles | SHOW_Decals | SHOW_DynamicShadows | SHOW_SceneCaptureUpdates | SHOW_Sprites | SHOW_PostProcess | SHOW_MotionBlur | SHOW_UnlitTranslucency | SHOW_ViewMode_Lit | SHOW_LensFlares)
#define	SHOW_DefaultEditor	(SHOW_Editor | SHOW_StaticMeshes | SHOW_SkeletalMeshes | SHOW_Terrain | SHOW_SpeedTrees | SHOW_Foliage | SHOW_BSP | SHOW_Fog | SHOW_Portals | SHOW_Grid | SHOW_Selection | SHOW_Volumes | SHOW_NavigationNodes | SHOW_Particles | SHOW_BuilderBrush | SHOW_Decals | SHOW_DecalInfo | SHOW_LightRadius | SHOW_AudioRadius | SHOW_DynamicShadows | SHOW_SceneCaptureUpdates | SHOW_Sprites | SHOW_PostProcess | SHOW_MotionBlur | SHOW_ModeWidgets | SHOW_UnlitTranslucency | SHOW_Constraints | SHOW_LensFlares)

// The show flags which only editor views can use.
#define SHOW_EditorOnly_Mask	(SHOW_Editor | SHOW_LightRadius | SHOW_AudioRadius | SHOW_LargeVertices | SHOW_HitProxies | SHOW_NavigationNodes | SHOW_LightInfluences | SHOW_TerrainPatches | SHOW_ActorTags | SHOW_PropertyColoration)

/**
 * View modes, predefined configurations of a subset of the show flags.
 */

// The flags to clear before setting any of the viewmode show flags.
#define SHOW_ViewMode_Mask (SHOW_Wireframe | SHOW_BSPTriangles | SHOW_Lighting | SHOW_Materials | SHOW_LightComplexity | SHOW_Brushes | SHOW_TextureDensity | SHOW_PostProcess | SHOW_ShaderComplexity | SHOW_Fog)

// Wireframe w/ BSP
#define SHOW_ViewMode_Wireframe (SHOW_Wireframe | SHOW_BSPTriangles)

// Wireframe w/ brushes
#define SHOW_ViewMode_BrushWireframe (SHOW_Wireframe | SHOW_Brushes)

// Unlit
#define SHOW_ViewMode_Unlit (SHOW_BSPTriangles | SHOW_Materials | SHOW_PostProcess | SHOW_Fog)

// Lit
#define SHOW_ViewMode_Lit (SHOW_BSPTriangles | SHOW_Materials | SHOW_Lighting | SHOW_PostProcess | SHOW_Fog)

// Lit wo/ materials
#define SHOW_ViewMode_LightingOnly (SHOW_BSPTriangles | SHOW_Lighting | SHOW_PostProcess)

// Colored according to light count.
#define SHOW_ViewMode_LightComplexity (SHOW_BSPTriangles | SHOW_LightComplexity)

// Colored according to shader complexity.
#define SHOW_ViewMode_ShaderComplexity (SHOW_BSPTriangles | SHOW_Materials | SHOW_Lighting | SHOW_ShaderComplexity | SHOW_Decals | SHOW_Fog)

// Colored according to world-space texture density.
#define SHOW_ViewMode_TextureDensity (SHOW_BSPTriangles | SHOW_Materials | SHOW_TextureDensity )

/**
 * The scene manager's persistent view state.
 */
class FSceneViewStateInterface
{
public:
	FSceneViewStateInterface()
		:	ViewParent( NULL )
		,	NumChildren( 0 )
	{}
	
	/** Called in the game thread to destroy the view state. */
	virtual void Destroy() = 0;

public:
	/** Sets the view state's scene parent. */
	void SetViewParent(FSceneViewStateInterface* InViewParent)
	{
		if ( ViewParent )
		{
			// Assert that the existing parent does not have a parent.
			check( !ViewParent->HasViewParent() );
			// Decrement ref ctr of existing parent.
			--ViewParent->NumChildren;
		}

		if ( InViewParent && InViewParent != this )
		{
			// Assert that the incoming parent does not have a parent.
			check( !InViewParent->HasViewParent() );
			ViewParent = InViewParent;
			// Increment ref ctr of new parent.
			InViewParent->NumChildren++;
		}
		else
		{
			ViewParent = NULL;
		}
	}
	/** @return			The view state's scene parent, or NULL if none present. */
	FSceneViewStateInterface* GetViewParent()
	{
		return ViewParent;
	}
	/** @return			The view state's scene parent, or NULL if none present. */
	const FSceneViewStateInterface* GetViewParent() const
	{
		return ViewParent;
	}
	/** @return			TRUE if the scene state has a parent, FALSE otherwise. */
	UBOOL HasViewParent() const
	{
		return GetViewParent() != NULL;
	}
	/** @return			TRUE if this scene state is a parent, FALSE otherwise. */
	UBOOL IsViewParent() const
	{
		return NumChildren > 0;
	}

protected:
	// Don't allow direct deletion of the view state, Destroy should be called instead.
	virtual ~FSceneViewStateInterface() {}

private:
	/** This scene state's view parent; NULL if no parent present. */
	FSceneViewStateInterface*	ViewParent;
	/** Reference counts the number of children parented to this state. */
	INT							NumChildren;
};

/**
 * Allocates a new instance of the private scene manager implementation of FSceneViewStateInterface
 */
extern FSceneViewStateInterface* AllocateViewState();

/**
 * The types of interactions between a light and a primitive.
 */
enum ELightInteractionType
{
	LIT_CachedIrrelevant,
	LIT_CachedLightMap,
	LIT_CachedShadowMap1D,
	LIT_CachedShadowMap2D,
	LIT_Uncached
};

/**
 * Information about an interaction between a light and a mesh.
 */
class FLightInteraction
{
public:

	// Factory functions.
	static FLightInteraction Uncached() { return FLightInteraction(LIT_Uncached,NULL,FVector2D(),FVector2D()); }
	static FLightInteraction LightMap() { return FLightInteraction(LIT_CachedLightMap,NULL,FVector2D(),FVector2D()); }
	static FLightInteraction Irrelevant() { return FLightInteraction(LIT_CachedIrrelevant,NULL,FVector2D(),FVector2D()); }
	static FLightInteraction ShadowMap1D(const FVertexBuffer* ShadowVertexBuffer)
	{
		return FLightInteraction(LIT_CachedShadowMap1D,ShadowVertexBuffer,FVector2D(),FVector2D());
	}
	static FLightInteraction ShadowMap2D(
		const UTexture2D* ShadowTexture,
		const FVector2D& InShadowCoordinateScale,
		const FVector2D& InShadowCoordinateBias
		)
	{
		return FLightInteraction(LIT_CachedShadowMap2D,ShadowTexture,InShadowCoordinateScale,InShadowCoordinateBias);
	}

	// Accessors.
	ELightInteractionType GetType() const { return Type; }
	const FVertexBuffer* GetShadowVertexBuffer() const
	{
		check(Type == LIT_CachedShadowMap1D);
		return (FVertexBuffer*)ShadowResource;
	}
	const UTexture2D* GetShadowTexture() const
	{
		check(Type == LIT_CachedShadowMap2D);
		return (UTexture2D*)ShadowResource;
	}
	const FVector2D& GetShadowCoordinateScale() const
	{
		check(Type == LIT_CachedShadowMap2D);
		return ShadowCoordinateScale;
	}
	const FVector2D& GetShadowCoordinateBias() const
	{
		check(Type == LIT_CachedShadowMap2D);
		return ShadowCoordinateBias;
	}

private:

	/**
	 * Minimal initialization constructor.
	 */
	FLightInteraction(
		ELightInteractionType InType,
		const void* InShadowResource,
		const FVector2D& InShadowCoordinateScale,
		const FVector2D& InShadowCoordinateBias
		):
		Type(InType),
		ShadowResource(InShadowResource),
		ShadowCoordinateScale(InShadowCoordinateScale),
		ShadowCoordinateBias(InShadowCoordinateBias)
	{}

	ELightInteractionType Type;
	const void* ShadowResource;
	FVector2D ShadowCoordinateScale;
	FVector2D ShadowCoordinateBias;
};

/**
 * The types of interactions between a light and a primitive.
 */
enum ELightMapInteractionType
{
	LMIT_None,
	LMIT_Vertex,
	LMIT_Texture,
};

/** The number of coefficients that are gathered for each lightmap. */ 
#define NUM_GATHERED_LIGHTMAP_COEF 4

/** The number of directional coefficients which the lightmap stores for each color component. */ 
#define NUM_DIRECTIONAL_LIGHTMAP_COEF 3

/** The number of simple coefficients which the lightmap stores for each color component. */ 
#define NUM_SIMPLE_LIGHTMAP_COEF 1

/** The index at which simple coefficients are stored in any array containing all NUM_GATHERED_LIGHTMAP_COEF coefficients. */ 
#define SIMPLE_LIGHTMAP_COEF_INDEX 3

/**
 * Information about an interaction between a light and a mesh.
 */
class FLightMapInteraction
{
public:

	// Factory functions.
	static FLightMapInteraction None()
	{
		FLightMapInteraction Result;
		Result.Type = LMIT_None;
		return Result;
	}
	static FLightMapInteraction Texture(
		const class ULightMapTexture2D* const* InTextures,
		const FVector4* InCoefficientScales,
		const FVector2D& InCoordinateScale,
		const FVector2D& InCoordinateBias,
		UBOOL bAllowDirectionalLightMaps)
	{
		FLightMapInteraction Result;
		Result.Type = LMIT_Texture;
		Result.bAllowDirectionalLightMaps = bAllowDirectionalLightMaps;
		//copy over the appropriate textures and scales
		if (bAllowDirectionalLightMaps)
		{
			Result.NumLightmapCoefficients = NUM_DIRECTIONAL_LIGHTMAP_COEF;
			for(UINT CoefficientIndex = 0;CoefficientIndex < Result.NumLightmapCoefficients;CoefficientIndex++)
			{
				Result.DirectionalTextures[CoefficientIndex] = InTextures[CoefficientIndex];
				Result.DirectionalCoefficientScales[CoefficientIndex] = InCoefficientScales[CoefficientIndex];
			}
		}
		else
		{
			Result.NumLightmapCoefficients = NUM_SIMPLE_LIGHTMAP_COEF;
			Result.SimpleTextures[0] = InTextures[SIMPLE_LIGHTMAP_COEF_INDEX];
			Result.SimpleCoefficientScales[0] = InCoefficientScales[SIMPLE_LIGHTMAP_COEF_INDEX];
		}
		
		Result.CoordinateScale = InCoordinateScale;
		Result.CoordinateBias = InCoordinateBias;
		return Result;
	}
	static FLightMapInteraction Vertex(const FVertexBuffer* InVertexBuffer,const FVector4* InCoefficientScales,UBOOL bAllowDirectionalLightMaps)
	{
		FLightMapInteraction Result;
		Result.Type = LMIT_Vertex;
		Result.VertexBuffer = InVertexBuffer;
		Result.bAllowDirectionalLightMaps = bAllowDirectionalLightMaps;
		//copy over the appropriate scales
		if (bAllowDirectionalLightMaps)
		{
			Result.NumLightmapCoefficients = NUM_DIRECTIONAL_LIGHTMAP_COEF;
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
			{
				Result.DirectionalCoefficientScales[CoefficientIndex] = InCoefficientScales[CoefficientIndex];
			}
		}
		else
		{
			Result.NumLightmapCoefficients = NUM_SIMPLE_LIGHTMAP_COEF;
			Result.SimpleCoefficientScales[0] = InCoefficientScales[SIMPLE_LIGHTMAP_COEF_INDEX];
		}

		return Result;
	}

	/** Default constructor. */
	FLightMapInteraction():
		Type(LMIT_None)
	{}

	// Accessors.
	ELightMapInteractionType GetType() const { return Type; }
	const FVertexBuffer* GetVertexBuffer() const
	{
		check(Type == LMIT_Vertex);
		return VertexBuffer;
	}
	
	const ULightMapTexture2D* GetTexture(INT TextureIndex) const
	{
		check(Type == LMIT_Texture);
		return bAllowDirectionalLightMaps ? DirectionalTextures[TextureIndex] : SimpleTextures[TextureIndex];
	}
	const FVector4* GetScaleArray() const
	{
		return bAllowDirectionalLightMaps ? DirectionalCoefficientScales : SimpleCoefficientScales;
	}
	
	const FVector2D& GetCoordinateScale() const
	{
		check(Type == LMIT_Texture);
		return CoordinateScale;
	}
	const FVector2D& GetCoordinateBias() const
	{
		check(Type == LMIT_Texture);
		return CoordinateBias;
	}

	const UINT GetNumLightmapCoefficients() const
	{
		return NumLightmapCoefficients;
	}

	/**
	* @return TRUE if non-simple lightmaps are allowed
	*/
	FORCEINLINE UBOOL AllowsDirectionalLightmaps() const
	{
		return bAllowDirectionalLightMaps;
	}

private:

	ELightMapInteractionType Type;
	FVector4 DirectionalCoefficientScales[NUM_DIRECTIONAL_LIGHTMAP_COEF];
	FVector4 SimpleCoefficientScales[NUM_SIMPLE_LIGHTMAP_COEF];

	const FVertexBuffer* VertexBuffer;

	const class ULightMapTexture2D* DirectionalTextures[NUM_DIRECTIONAL_LIGHTMAP_COEF];
	const class ULightMapTexture2D* SimpleTextures[NUM_SIMPLE_LIGHTMAP_COEF];
	FVector2D CoordinateScale;
	FVector2D CoordinateBias;

	UBOOL bAllowDirectionalLightMaps;
	UINT NumLightmapCoefficients;
};

/**
 * An interface to cached lighting for a specific mesh.
 */
class FLightCacheInterface
{
public:
	virtual FLightInteraction GetInteraction(const class FLightSceneInfo* LightSceneInfo) const = 0;
	virtual FLightMapInteraction GetLightMapInteraction() const = 0;
};

/**
 * A mesh element definition.
 */
struct FMeshElement
{
	const FIndexBuffer* IndexBuffer;
	const FVertexFactory* VertexFactory;
	/** 
	 *	DynamicVertexData - pointer to user memory containing the vertex data.
	 *	Used for rendering dynamic data directly.
	 */
	const void* DynamicVertexData;
	INT DynamicVertexStride;
	/** 
	 *	DynamicIndexData - pointer to user memory containing the index data.
	 *	Used for rendering dynamic data directly.
	 */
	const void* DynamicIndexData;
	INT DynamicIndexStride;
	const FMaterialRenderProxy* MaterialRenderProxy;
	const FLightCacheInterface* LCI;
	/**	DecalState - pointer to a dynamically generated decal that wants to use a scissor rect when rendering. */
	const class FDecalState* DecalState;
	FMatrix LocalToWorld;
	FMatrix WorldToLocal;
	INT ScreenBoundsMinX;
	INT ScreenBoundsMinY;
	INT ScreenBoundsMaxX;
	INT ScreenBoundsMaxY;
	UINT FirstIndex;
	UINT NumPrimitives;
	UINT MinVertexIndex;
	UINT MaxVertexIndex;
	FLOAT DepthBias;
	FLOAT SlopeScaleDepthBias;
	void* UserData;
	BITFIELD UseDynamicData : 1;
	BITFIELD ReverseCulling : 1;
	BITFIELD CastShadow : 1;
	BITFIELD bWireframe : 1;
	BITFIELD Type : PT_NumBits;
	BITFIELD ParticleType : PET_NumBits;
	BITFIELD DepthPriorityGroup : UCONST_SDPG_NumBits;
	BITFIELD bUsePreVertexShaderCulling : 1;
	BITFIELD bUseAsOccluder : 1;
	BITFIELD bIsDecal : 1;

	FORCEINLINE UBOOL IsTranslucent() const
	{
        return MaterialRenderProxy && IsTranslucentBlendMode(MaterialRenderProxy->GetMaterial()->GetBlendMode());
	}

	FORCEINLINE UBOOL IsMasked() const
	{
		return MaterialRenderProxy && MaterialRenderProxy->GetMaterial()->IsMasked();
	}

	FORCEINLINE UBOOL IsDistortion() const
	{
		return MaterialRenderProxy && MaterialRenderProxy->GetMaterial()->IsDistorted();
	}

	//@return TRUE if the mesh element is used for decal rendering
	FORCEINLINE UBOOL IsDecal() const
	{
		return bIsDecal;
	}

	/** 
	* @return vertex stride specified for the mesh. 0 if not dynamic
	*/
	FORCEINLINE DWORD GetDynamicVertexStride() const
	{
		if (UseDynamicData && DynamicVertexData)
		{
			return DynamicVertexStride;
		}
		else
		{
			return 0;
		}
	}

	/** Default constructor. */
	FMeshElement():
		IndexBuffer(NULL),
		VertexFactory(NULL),
		DynamicVertexData(NULL),
		DynamicIndexData(NULL),
		MaterialRenderProxy(NULL),
		LCI(NULL),
		DecalState(NULL),
		ScreenBoundsMinX(-1),
		ScreenBoundsMinY(-1),
		ScreenBoundsMaxX(-1),
		ScreenBoundsMaxY(-1),
		DepthBias(0.f),
		SlopeScaleDepthBias(0.f),
		UserData(NULL),
		UseDynamicData(FALSE),
		ReverseCulling(FALSE),
		CastShadow(TRUE),
		bWireframe(FALSE),
		Type(PT_TriangleList),
		ParticleType(PET_None),
		bUsePreVertexShaderCulling(FALSE),
		bUseAsOccluder(TRUE),
		bIsDecal(FALSE)
	{}
};

/**
 * An interface implemented by dynamic resources which need to be initialized and cleaned up by the rendering thread.
 */
class FDynamicPrimitiveResource
{
public:

	virtual void InitPrimitiveResource() = 0;
	virtual void ReleasePrimitiveResource() = 0;
};

/**
 * The base interface used to query a primitive for its dynamic elements.
 */
class FPrimitiveDrawInterface
{
public:

	const FSceneView* const View;

	/** Initialization constructor. */
	FPrimitiveDrawInterface(const FSceneView* InView):
		View(InView)
	{}

	virtual UBOOL IsHitTesting() = 0;
	virtual void SetHitProxy(HHitProxy* HitProxy) = 0;

	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource) = 0;

	virtual void DrawSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Sprite,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		) = 0;

	virtual void DrawLine(
		const FVector& Start,
		const FVector& End,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		) = 0;

	virtual void DrawPoint(
		const FVector& Position,
		const FLinearColor& Color,
		FLOAT PointSize,
		BYTE DepthPriorityGroup
		) = 0;

	/**
	 * Determines whether a particular material will be ignored in this context.
	 * @param MaterialRenderProxy - The render proxy of the material to check.
	 * @return TRUE if meshes using the material will be ignored in this context.
	 */
	virtual UBOOL IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy) const
	{
		return FALSE;
	}

	/**
	 * Draw a mesh element.
	 * This should only be called through the DrawMesh function.
	 *
	 * @return Number of passes rendered for the mesh
	 */
	virtual INT DrawMesh(const FMeshElement& Mesh) = 0;
};

/**
 * An interface to a scene interaction.
 */
class FViewElementDrawer
{
public:

	/**
	 * Draws the interaction using the given draw interface.
	 */
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI) {}
};

/**
 * An interface used to query a primitive for its static elements.
 */
class FStaticPrimitiveDrawInterface
{
public:
	virtual void SetHitProxy(HHitProxy* HitProxy) = 0;
	virtual void DrawMesh(
		const FMeshElement& Mesh,
		FLOAT MinDrawDistance,
		FLOAT MaxDrawDistance
		) = 0;
};

/**
 * An interface used by a primitive to render its shadow volumes.
 */
class FShadowVolumeDrawInterface
{
public:
	/**
	  * Called by FPrimitiveSceneProxy to render shadow volumes to the stencil buffer.
	  *
	  * @param IndexBuffer Shadow volume index buffer, indexing into the original vertex buffer
	  * @param VertexFactory Vertex factory that represents the shadow volume vertex format
	  * @param LocalToWorld World matrix for the primitive
	  * @param FirstIndex First index to use in the index buffer
	  * @param NumPrimitives Number of triangles in the triangle list
	  * @param MinVertexIndex Lowest index used in the index buffer
	  * @param MaxVertexIndex Highest index used in the index buffer
	  */
	virtual void DrawShadowVolume(
		FIndexBufferRHIParamRef IndexBuffer,
		const class FLocalShadowVertexFactory& VertexFactory,
		const FMatrix& LocalToWorld,
		UINT FirstIndex,
		UINT NumPrimitives,
		UINT MinVertexIndex,
		UINT MaxVertexIndex
		) = 0;
};

/**
 * The different types of relevance a primitive scene proxy can declare towards a particular scene view.
 */
struct FPrimitiveViewRelevance
{
	/** The primitive's static elements are rendered for the view. */
	BITFIELD bStaticRelevance : 1; 
	/** The primitive's dynamic elements are rendered for the view. */
	BITFIELD bDynamicRelevance : 1;
	/** The primitive is casting a shadow. */
	BITFIELD bShadowRelevance : 1;
	/** The primitive's static decal elements are rendered for the view. */
	BITFIELD bDecalStaticRelevance : 1;
	/** The primitive's dynamic decal elements are rendered for the view. */
	BITFIELD bDecalDynamicRelevance : 1;	

	/** The primitive has one or more UnrealEd background DPG elements. */
	BITFIELD bUnrealEdBackgroundDPG : 1;
	/** The primitive has one or more world DPG elements. */
	BITFIELD bWorldDPG : 1;
	/** The primitive has one or more foreground DPG elements. */
	BITFIELD bForegroundDPG : 1;
	/** The primitive has one or more UnrealEd DPG elements. */
	BITFIELD bUnrealEdForegroundDPG : 1;

	/** The primitive has one or more opaque or masked elements. */
	BITFIELD bOpaqueRelevance : 1;
	/** The primitive has one or more masked elements. */
	BITFIELD bMaskedRelevance : 1;
	/** The primitive has one or more translucent elements. */
	BITFIELD bTranslucentRelevance : 1;
	/** The primitive has one or more distortion elements. */
	BITFIELD bDistortionRelevance : 1;
	/** The primitive has one or more one-layer distortion elements. */
	BITFIELD bOneLayerDistortionRelevance : 1;
	/** The primitive has one or more lit elements. */
	BITFIELD bLightingRelevance : 1;
	/** The primitive has one or more elements which use a material that samples scene color */
	BITFIELD bUsesSceneColor : 1;
	/** Forces all directional lights to be dynamic. */
	BITFIELD bForceDirectionalLightsDynamic : 1;
	/** The primitive renders mesh elements with dynamic data (ie. FMeshElement.UsesDynamicData=TRUE) */
	BITFIELD bUsesDynamicMeshElementData : 1;

	/** TRUE if the primitive needs PreRenderView to be called before rendering. */
	BITFIELD bNeedsPreRenderView : 1;

	/** Initialization constructor. */
	FPrimitiveViewRelevance():
		bStaticRelevance(FALSE),
		bDynamicRelevance(FALSE),
		bShadowRelevance(FALSE),
		bDecalStaticRelevance(FALSE),
		bDecalDynamicRelevance(FALSE),
		bUnrealEdBackgroundDPG(FALSE),
		bWorldDPG(FALSE),
		bForegroundDPG(FALSE),
		bUnrealEdForegroundDPG(FALSE),
		bOpaqueRelevance(TRUE),
		bMaskedRelevance(FALSE),
		bTranslucentRelevance(FALSE),
		bDistortionRelevance(FALSE),
		bOneLayerDistortionRelevance(FALSE),
		bLightingRelevance(FALSE),
		bUsesSceneColor(FALSE),
		bForceDirectionalLightsDynamic(FALSE),
		bUsesDynamicMeshElementData(FALSE),
		bNeedsPreRenderView(FALSE)
	{}

	/** Bitwise OR operator.  Sets any relevance bits which are present in either FPrimitiveViewRelevance. */
	FPrimitiveViewRelevance& operator|=(const FPrimitiveViewRelevance& B)
	{
		bStaticRelevance |= B.bStaticRelevance != 0;
		bDynamicRelevance |= B.bDynamicRelevance != 0;
		bShadowRelevance |= B.bShadowRelevance != 0;
		bDecalStaticRelevance |= B.bDecalStaticRelevance != 0;
		bDecalDynamicRelevance |= B.bDecalDynamicRelevance != 0;
		bUnrealEdBackgroundDPG |= B.bUnrealEdBackgroundDPG != 0;
		bWorldDPG |= B.bWorldDPG != 0;
		bForegroundDPG |= B.bForegroundDPG != 0;
		bUnrealEdForegroundDPG |= B.bUnrealEdForegroundDPG != 0;
		bOpaqueRelevance |= B.bOpaqueRelevance != 0;
		bMaskedRelevance |= B.bMaskedRelevance != 0;
		bTranslucentRelevance |= B.bTranslucentRelevance != 0;
		bDistortionRelevance |= B.bDistortionRelevance != 0;
		bOneLayerDistortionRelevance |= B.bOneLayerDistortionRelevance != 0;
		bLightingRelevance |= B.bLightingRelevance != 0;
		bUsesSceneColor |= B.bUsesSceneColor != 0;
		bForceDirectionalLightsDynamic |= B.bForceDirectionalLightsDynamic != 0;
		bUsesDynamicMeshElementData |= B.bUsesDynamicMeshElementData != 0;
		bNeedsPreRenderView |= B.bNeedsPreRenderView != 0;
		return *this;
	}

	/** Binary bitwise OR operator. */
	friend FPrimitiveViewRelevance operator|(const FPrimitiveViewRelevance& A,const FPrimitiveViewRelevance& B)
	{
		FPrimitiveViewRelevance Result(A);
		Result |= B;
		return Result;
	}

	/** Sets the flag corresponding to the given DPG. */
	void SetDPG(UINT DPGIndex,UBOOL bValue)
	{
		switch(DPGIndex)
		{
		case SDPG_UnrealEdBackground:	bUnrealEdBackgroundDPG = bValue; break;
		case SDPG_World:				bWorldDPG = bValue; break;
		case SDPG_Foreground:			bForegroundDPG = bValue; break;
		case SDPG_UnrealEdForeground:	bUnrealEdForegroundDPG = bValue; break;
		}
	}

	/** Gets the flag corresponding to the given DPG. */
	UBOOL GetDPG(UINT DPGIndex) const
	{
		switch(DPGIndex)
		{
		case SDPG_UnrealEdBackground:	return bUnrealEdBackgroundDPG;
		case SDPG_World:				return bWorldDPG;
		case SDPG_Foreground:			return bForegroundDPG;
		case SDPG_UnrealEdForeground:	return bUnrealEdForegroundDPG;
		default:						return FALSE;
		}
	}

	/** @return True if the primitive is relevant to the view. */
	UBOOL IsRelevant() const
	{
		return (bStaticRelevance | bDynamicRelevance) != 0;
	}

	/** @return True if the primitive has decals which are relevant to the view. */
	UBOOL IsDecalRelevant() const
	{
		return (bDecalStaticRelevance | bDecalDynamicRelevance) != 0;
	}
};

/**
 * View relevance information about a material or set of materials.
 * This is mirrored in PrimitiveComponent.uc
 */
class FMaterialViewRelevance
{
public:

	BITFIELD bOpaque : 1;
	BITFIELD bMasked : 1;
	BITFIELD bTranslucency : 1;
	BITFIELD bDistortion : 1;
	BITFIELD bOneLayerDistortionRelevance : 1;
	BITFIELD bLit : 1;
	BITFIELD bUsesSceneColor : 1;

	/** Default constructor. */
	FMaterialViewRelevance():
		bOpaque(FALSE),
		bMasked(FALSE),
		bTranslucency(FALSE),
		bDistortion(FALSE),
		bOneLayerDistortionRelevance(FALSE),
		bLit(FALSE),
		bUsesSceneColor(FALSE)
	{}

	/** Bitwise OR operator.  Sets any relevance bits which are present in either FMaterialViewRelevance. */
	FMaterialViewRelevance& operator|=(const FMaterialViewRelevance& B)
	{
		bOpaque |= B.bOpaque;
		bMasked |= B.bMasked;
		bTranslucency |= B.bTranslucency;
		bDistortion |= B.bDistortion;
		bOneLayerDistortionRelevance |= B.bOneLayerDistortionRelevance;
		bLit |= B.bLit;
		bUsesSceneColor |= B.bUsesSceneColor;
		return *this;
	}
	
	/** Binary bitwise OR operator. */
	friend FMaterialViewRelevance operator|(const FMaterialViewRelevance& A,const FMaterialViewRelevance& B)
	{
		FMaterialViewRelevance Result(A);
		Result |= B;
		return Result;
	}

	/** Copies the material's relevance flags to a primitive's view relevance flags. */
	void SetPrimitiveViewRelevance(FPrimitiveViewRelevance& OutViewRelevance) const
	{
		OutViewRelevance.bOpaqueRelevance = bOpaque;
		OutViewRelevance.bMaskedRelevance = bMasked;
		OutViewRelevance.bTranslucentRelevance = bTranslucency;
		OutViewRelevance.bDistortionRelevance = bDistortion;
		OutViewRelevance.bOneLayerDistortionRelevance = bOneLayerDistortionRelevance;
		OutViewRelevance.bLightingRelevance = bLit;
		OutViewRelevance.bUsesSceneColor = bUsesSceneColor;
	}
};

/**
 *	Helper structure for storing motion blur information for a primitive
 */
struct FMotionBlurInfo
{
	/** The component this info represents.			*/
	UPrimitiveComponent* Component;
	/** The primitive scene info for the component.	*/
	FPrimitiveSceneInfo* PrimitiveSceneInfo;
	/** The previous LocalToWorld of the component.	*/
	FMatrix	PreviousLocalToWorld;
};

/**
 * Interface for rendering a post process mask
 **/
class FPostProcessMaskBase
{
public:
	/**
	 * @return TRUE if the masking should be rendered
	 **/
	virtual UBOOL ShouldRender() const =0;
	/**
	* Begin stencil mask for post process 
	**/
	virtual void BeginStencilMask() const =0;
	/**
	* End stencil mask for post process 
	**/
	virtual void EndStencilMask() const =0;
};

/**
 * An interface to the private scene manager implementation of a scene.  Use AllocateScene to create.
 * The scene
 */
class FSceneInterface
{
public:

	// FSceneInterface interface

	/** 
	 * Adds a new primitive component to the scene
	 * 
	 * @param Primitive - primitive component to add
	 */
	virtual void AddPrimitive(UPrimitiveComponent* Primitive) = 0;
	/** 
	 * Removes a primitive component from the scene
	 * 
	 * @param Primitive - primitive component to remove
	 */
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive) = 0;
	/** 
	 * Updates the transform of a primitive which has already been added to the scene. 
	 * 
	 * @param Primitive - primitive component to update
	 */
	virtual void UpdatePrimitiveTransform(UPrimitiveComponent* Primitive) = 0;
	/** 
	 * Adds a new light component to the scene
	 * 
	 * @param Light - light component to add
	 */
	virtual void AddLight(ULightComponent* Light) = 0;
	/** 
	 * Removes a light component from the scene
	 * 
	 * @param Light - light component to remove
	 */
	virtual void RemoveLight(ULightComponent* Light) = 0;
	/** 
	 * Updates the transform of a light which has already been added to the scene. 
	 *
	 * @param Light - light component to update
	 */
	virtual void UpdateLightTransform(ULightComponent* Light) = 0;
	/** 
	 * Updates the color and brightness of a light which has already been added to the scene. 
	 *
	 * @param Light - light component to update
	 */
	virtual void UpdateLightColorAndBrightness(ULightComponent* Light) = 0;
	/** 
	 * Adds a new scene capture component to the scene
	 * 
	 * @param CaptureComponent - scene capture component to add
	 */	
	virtual void AddSceneCapture(USceneCaptureComponent* CaptureComponent) = 0;
	/** 
	 * Removes a scene capture component from the scene
	 * 
	 * @param CaptureComponent - scene capture component to remove
	 */	
	virtual void RemoveSceneCapture(USceneCaptureComponent* CaptureComponent) = 0;
	/**
	 * Adds a fluidsurface to the scene (gamethread)
	 *
	 * @param FluidComponent - component to add to the scene 
	 */
	virtual void AddFluidSurface(class UFluidSurfaceComponent* FluidComponent) = 0;
	/**
	 * Removes a fluidsurface from the scene (gamethread)
	 *
	 * @param CaptureComponent - component to remove from the scene 
	 */
	virtual void RemoveFluidSurface(class UFluidSurfaceComponent* FluidComponent) = 0;
	/**
	 * Retrieves a pointer to the fluidsurface container.
	 * @return TArray pointer, or NULL if the scene doesn't support fluidsurfaces.
	 **/
	virtual const TArray<class UFluidSurfaceComponent*>* GetFluidSurfaces() = 0;
	/** 
	 * Adds a new height fog component to the scene
	 * 
	 * @param FogComponent - fog component to add
	 */	
	virtual void AddHeightFog(class UHeightFogComponent* FogComponent) = 0;
	/** 
	 * Removes a height fog component from the scene
	 * 
	 * @param FogComponent - fog component to remove
	 */	
	virtual void RemoveHeightFog(class UHeightFogComponent* FogComponent) = 0;
	/** 
	 * Adds a new default fog volume scene info to the scene
	 * 
	 * @param VolumeComponent - fog primitive to add
	 */	
	virtual void AddFogVolume(const UPrimitiveComponent* MeshComponent) = 0;
	/** 
	 * Adds a new fog volume scene info to the scene
	 * 
	 * @param FogVolumeSceneInfo - fog volume scene info to add
	 * @param VolumeComponent - fog primitive to add
	 */	
	virtual void AddFogVolume(const class UFogVolumeDensityComponent* FogVolumeComponent, const UPrimitiveComponent* MeshComponent) = 0;
	/** 
	 * Removes a fog volume primitive component from the scene
	 * 
	 * @param VolumeComponent - fog primitive to remove
	 */
	virtual void RemoveFogVolume(const UPrimitiveComponent* MeshComponent) = 0;
	/**
	 * Adds a wind source component to the scene.
	 * @param WindComponent - The component to add.
	 */
	virtual void AddWindSource(class UWindDirectionalSourceComponent* WindComponent) = 0;
	/**
	 * Removes a wind source component from the scene.
	 * @param WindComponent - The component to remove.
	 */
	virtual void RemoveWindSource(class UWindDirectionalSourceComponent* WindComponent) = 0;
	/**
	 * Accesses the wind source list.  Must be called in the rendering thread.
	 * @return The list of wind sources in the scene.
	 */
	virtual const TArray<class FWindSourceSceneProxy*>& GetWindSources_RenderThread() const = 0;
	/**
	 * Release this scene and remove it from the rendering thread
	 */
	virtual void Release() = 0;
	/**
	 * Retrieves the lights interacting with the passed in primitive and adds them to the out array.
	 *
	 * @param	Primitive				Primitive to retrieve interacting lights for
	 * @param	RelevantLights	[out]	Array of lights interacting with primitive
	 */
	virtual void GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const = 0;
	/** 
	 * Indicates if sounds in this should be allowed to play. 
	 * 
	 * @return TRUE if audio playback is allowed
	 */
	virtual UBOOL AllowAudioPlayback() = 0;
	/**
	 * Indicates if hit proxies should be processed by this scene
	 *
	 * @return TRUE if hit proxies should be rendered in this scene.
	 */
	virtual UBOOL RequiresHitProxies() const = 0;
	/**
	 * Get the optional UWorld that is associated with this scene
	 * 
 	 * @return UWorld instance used by this scene
	 */
	virtual class UWorld* GetWorld() const = 0;
	/**
	 * Return the scene to be used for rendering
	 */
	virtual FSceneInterface* GetRenderScene()
	{
		return NULL;
	}

	/**
	 *	Add the scene captures view info to the streaming manager
	 *
	 *	@param	StreamingManager			The streaming manager to add the view info to.
	 *	@param	View						The scene view information
	 */
	virtual void AddSceneCaptureViewInformation(FStreamingManagerCollection* StreamingManager, FSceneView* View) {};

	/**
	 * Dumps dynamic lighting and shadow interactions for scene to log.
	 *
	 * @param	bOnlyIncludeShadowCastingInteractions	Whether to only include shadow casting interactions
	 */
	virtual void DumpDynamicLightShadowInteractions( UBOOL bOnlyIncludeShadowCastingInteractions ) const
	{}

protected:
	virtual ~FSceneInterface() {}
};

/**
 * Allocates a new instance of the private FScene implementation for the given world.
 * @param World - An optional world to associate with the scene.
 * @param bAlwaysAllowAudioPlayback - Indicate that audio in this scene should always be played, even if its not the GWorld.
 * @param bInRequiresHitProxies- Indicates that hit proxies should be rendered in the scene.
 */
extern FSceneInterface* AllocateScene(UWorld* World, UBOOL bAlwaysAllowAudioPlayback, UBOOL bInRequiresHitProxies);

/**
 * An interface to an actor visibility state for a single frame of a view.
 */
class FActorVisibilityHistoryInterface
{
public:

	/** Virtual destructor. */
	virtual ~FActorVisibilityHistoryInterface() {}

	/**
	 * Tests whether an actor was visible for the scene view.
	 * @param Actor - The actor to test.
	 * @return True if the actor was visible.
	 */
	virtual UBOOL GetActorVisibility(const AActor* Actor) const = 0;
};

/**
 * Encapsulates the most recent actor visibility states for a particular view.
 * Provides thread-safe access to the actor visibility states while a scene is being rendered which updates the states.
 */
class FSynchronizedActorVisibilityHistory
{
public:

	/** Initialization constructor. */
	FSynchronizedActorVisibilityHistory();

	/** Destructor. */
	~FSynchronizedActorVisibilityHistory();

	/** Initializes the history.  Must be called before calling any of the other members. */
	void Init();

	/**
	 * Tests whether an actor was visible for the scene view.
	 * @param Actor - The actor to test.
	 * @return True if the actor was visible.
	 */
	UBOOL GetActorVisibility(const AActor* Actor) const;

	/**
	 * Updates the actor visibility states for a new frame.
	 * @param NewStates - The new actor visibility states.  Ownership is transferred to the callee.
	 */
	void SetStates(FActorVisibilityHistoryInterface* NewStates);

private:

	/** The most recent actor visibility states. */
	FActorVisibilityHistoryInterface* States;

	/** The critical section used to synchronize access to the actor visibility states. */
	FCriticalSection* CriticalSection;
};

/**
 * A projection from scene space into a 2D screen region.
 */
class FSceneView
{
public:
	const FSceneViewFamily* Family;
	FSceneViewStateInterface* State;

	/**
	 *	The index into the ParentViewFamily array indicating which view is 'active'.
	 *	If -1, it indicates that the operation should iterate over the ParentViews itself.
	 */
	INT ParentViewIndex;
	/** 
	 *	The parent view family of the main scene - used for scene capture objects like portals.
	 */
	const FSceneViewFamily* ParentViewFamily;

	/** A pointer to the history to write the view's actor visibility states to.  May be NULL. */
	FSynchronizedActorVisibilityHistory* ActorVisibilityHistory;

	/** The actor which is being viewed from. */
	const AActor* ViewActor;

	/** Chain of post process effects for this view */
	const UPostProcessChain* PostProcessChain;

	/** The view-specific post process settings. */
	const struct FPostProcessSettings* PostProcessSettings;

	/** view-specific optional post process mask */
	const FPostProcessMaskBase* PostProcessMask;	

	/** An interaction which draws the view's interaction elements. */
	FViewElementDrawer* Drawer;

	FLOAT X;
	FLOAT Y;
	FLOAT SizeX;
	FLOAT SizeY;

	/** The X coordinate of the view in the scene render targets. */
	INT RenderTargetX;

	/** The Y coordinate of the view in the scene render targets. */
	INT RenderTargetY;

	/** The width of the view in the scene render targets. */
	INT RenderTargetSizeX;

	/** The height of the view in the scene render targets. */
	INT RenderTargetSizeY;

	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;

	FLinearColor BackgroundColor;
	FLinearColor OverlayColor;

	/** Color scale multiplier used during post processing */
	FLinearColor ColorScale;

	/** The primitives which are hidden for this view. */
	TSet<UPrimitiveComponent*> HiddenPrimitives;

	// Derived members.

	/** The view transform, starting from world-space points translated by -ViewOrigin. */
	FMatrix TranslatedViewMatrix;

	/** The view-projection transform, starting from world-space points translated by -ViewOrigin. */
	FMatrix TranslatedViewProjectionMatrix;

	/** The inverse view-projection transform, ending with world-space points translated by -ViewOrigin. */
	FMatrix InvTranslatedViewProjectionMatrix;

	/** The translation to apply to the world before TranslatedViewProjectionMatrix. */
	FVector PreViewTranslation;

	FMatrix ViewProjectionMatrix;
	FMatrix InvProjectionMatrix;
	FMatrix InvViewProjectionMatrix;
	FVector4 ViewOrigin;
	FConvexVolume ViewFrustum;
	UBOOL bHasNearClippingPlane;
	FPlane NearClippingPlane;
	FLOAT NearClippingDistance;

	/** TRUE if ViewMatrix.Determinant() is negative. */
	UBOOL bReverseCulling;

	/* Vector used by shaders to convert depth buffer samples into z coordinates in world space */
	FVector4 InvDeviceZToWorldZTransform;

	/* Vector used by shaders to convert projection-space coordinates to texture space. */
	FVector4 ScreenPositionScaleBias;

	/** FOV based multiplier for cull distance on objects */
	FLOAT LODDistanceFactor;

	/** Whether the scene color is contained in the LDR surface */
	UBOOL bUseLDRSceneColor;


	/** Initialization constructor. */
	FSceneView(
		const FSceneViewFamily* InFamily,
		FSceneViewStateInterface* InState,
		INT InParentViewIndex,
		const FSceneViewFamily* InParentViewFamily,
		FSynchronizedActorVisibilityHistory* InActorVisibilityHistory,
		const AActor* InViewActor,
		const UPostProcessChain* InPostProcessChain,
		const FPostProcessSettings* InPostProcessSettings,
		const FPostProcessMaskBase* InPostProcessMask,
		FViewElementDrawer* InDrawer,
		FLOAT InX,
		FLOAT InY,
		FLOAT InSizeX,
		FLOAT InSizeY,
		const FMatrix& InViewMatrix,
		const FMatrix& InProjectionMatrix,
		const FLinearColor& InBackgroundColor,
		const FLinearColor& InOverlayColor,
		const FLinearColor& InColorScale,
		const TSet<UPrimitiveComponent*>& InHiddenPrimitives,
		FLOAT InLODDistanceFactor = 1.0f
		);

	/** Transforms a point from world-space to the view's screen-space. */
	FVector4 WorldToScreen(const FVector& WorldPoint) const;

	/** Transforms a point from the view's screen-space to world-space. */
	FVector ScreenToWorld(const FVector4& ScreenPoint) const;

	/** Transforms a point from the view's screen-space into pixel coordinates relative to the view's X,Y. */
	UBOOL ScreenToPixel(const FVector4& ScreenPoint,FVector2D& OutPixelLocation) const;

	/** Transforms a point from pixel coordinates into the view's screen-space. */
	FVector4 PixelToScreen(FLOAT X,FLOAT Y,FLOAT Z) const;

	/** Transforms a point from the view's world-space into pixel coordinates relative to the view's X,Y. */
	UBOOL WorldToPixel(const FVector& WorldPoint,FVector2D& OutPixelLocation) const;

	/** 
	 * Transforms a point from the view's world-space into the view's screen-space. 
	 * Divides the resulting X, Y, Z by W before returning. 
	 */
	FPlane Project(const FVector& WorldPoint) const;

	/** 
	 * Transforms a point from the view's screen-space into world coordinates
	 * multiplies X, Y, Z by W before transforming. 
	 */
	FVector Deproject(const FPlane& ScreenPoint) const;
};

/**
 * Helper structure for dynamic shadow stats containing stringified information in
 * column form.
 */
struct FDynamicShadowStatsRow
{
	/** Columns of this row. */
	TArray<FString> Columns;
};
typedef TArray<FDynamicShadowStatsRow> FDynamicShadowStats;

/**
 * A set of views into a scene which only have different view transforms and owner actors.
 */
class FSceneViewFamily
{
public:

	/** The views which make up the family. */
	TArray<const FSceneView*> Views;

	/** The render target which the views are being rendered to. */
	const FRenderTarget* RenderTarget;

	/** The scene being viewed. */
	FSceneInterface* Scene;

	/** The show flags for the views. */
	EShowFlags ShowFlags;

	/** The current world time. */
	FLOAT CurrentWorldTime;

	/** The difference between the last world time and CurrentWorldTime. */
	FLOAT DeltaWorldTime;

	/** The current real time. */
	FLOAT CurrentRealTime;

	/** Pointer to dynamic shadow stats. If non- NULL stats will be written to it. */
	FDynamicShadowStats* DynamicShadowStats;

	/** Indicates whether the view family is updated in realtime. */
	UBOOL bRealtimeUpdate;

	/** Indicates whether ambient occlusion is allowed on any views in this view family. */
	UBOOL bAllowAmbientOcclusion;

	/** Used to defer the back buffer clearing to just before the back buffer is drawn to */
	UBOOL bDeferClear;

	/** if TRUE then the scene color is not cleared when rendering this set of views. */
	UBOOL bClearScene;

	/** if TRUE then results of scene rendering are copied/resolved to the RenderTarget. */
	UBOOL bResolveScene;

	/** Gamma correction used when rendering this family. Default is 1.0 */
	FLOAT GammaCorrection;

	/** Initialization constructor. */
	FSceneViewFamily(
		const FRenderTarget* InRenderTarget,
		FSceneInterface* InScene,
		EShowFlags InShowFlags,
		FLOAT InCurrentWorldTime,
		FLOAT InDeltaWorldTime,
		FLOAT InCurrentRealTime,
		FDynamicShadowStats* InDynamicShadowStats,
		UBOOL InbRealtimeUpdate,
		UBOOL InbAllowAmbientOcclusion,
		UBOOL InbDeferClear,
		UBOOL InbClearScene,
		UBOOL InbResolveScene,
		FLOAT InGammaCorrection
		);
};

/**
 * Call from the game thread to send a message to the rendering thread to being rendering this view family.
 */
extern void BeginRenderingViewFamily(FCanvas* Canvas,const FSceneViewFamily* ViewFamily);

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
/**
* Sends a freeze rendering message over the render queue
*/
extern void FreezeRendering();

/**
* Sends a freeze rendering message over the render queue
*/
extern void UnfreezeRendering();

#endif

/**
 * A view family which deletes its views when it goes out of scope.
 */
class FSceneViewFamilyContext : public FSceneViewFamily
{
public:
	/** Initialization constructor. */
	FSceneViewFamilyContext(
		const FRenderTarget* InRenderTarget,
		FSceneInterface* InScene,
		EShowFlags InShowFlags,
		FLOAT InCurrentWorldTime,
		FLOAT InDeltaWorldTime,
		FLOAT InCurrentRealTime,
		FDynamicShadowStats* InDynamicShadowStats,
		UBOOL InbRealtimeUpdate=FALSE,
		UBOOL InbAllowAmbientOcclusion=FALSE,
		UBOOL InbDeferClear=FALSE,
		UBOOL InbClearScene=TRUE,
		UBOOL InbResolveScene=TRUE,
		FLOAT InGammaCorrection=1.0f)
		:	FSceneViewFamily(
		InRenderTarget,
		InScene,
		InShowFlags,
		InCurrentWorldTime,
		InDeltaWorldTime,
		InCurrentRealTime,
		InDynamicShadowStats,
		InbRealtimeUpdate,
		InbAllowAmbientOcclusion,
		InbDeferClear,
		InbClearScene,
		InbResolveScene,
		InGammaCorrection
		)
	{}
	/** Destructor. */
	~FSceneViewFamilyContext();
};

// Static-function 
class FPrimitiveSceneProxy;
FPrimitiveSceneProxy* Scene_GetProxyFromInfo(FPrimitiveSceneInfo* Info);

