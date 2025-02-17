/*=============================================================================
	Engine.h: Unreal engine public header file.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_ENGINE
#define _INC_ENGINE

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "Core.h"

/** Helper function to flush resource streaming. */
extern void FlushResourceStreaming();

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

extern class FStatChart*		GStatChart;
extern class UEngine*			GEngine;
extern UBOOL GAllowShadowVolumes;

/** when set, disallows all network travel (pending level rejects all client travel attempts) */
extern UBOOL GDisallowNetworkTravel;

/*-----------------------------------------------------------------------------
	Size of the world.
-----------------------------------------------------------------------------*/

#define WORLD_MAX			524288.0	/* Maximum size of the world */
#define HALF_WORLD_MAX		262144.0	/* Half the maximum size of the world */
#define HALF_WORLD_MAX1		262143.0	/* Half the maximum size of the world - 1*/
#define MIN_ORTHOZOOM		250.0		/* Limit of 2D viewport zoom in */
#define MAX_ORTHOZOOM		16000000.0	/* Limit of 2D viewport zoom out */
#define DEFAULT_ORTHOZOOM	10000		/* Default 2D viewport zoom */

/*-----------------------------------------------------------------------------
	Localization defines.
-----------------------------------------------------------------------------*/

#define LOCALIZED_SEEKFREE_SUFFIX	TEXT("_LOC")

/*-----------------------------------------------------------------------------
	In Editor Game viewport name (hardcoded)
-----------------------------------------------------------------------------*/

#define PLAYWORLD_VIEWPORT_NAME TEXT("In Editor Game")
#define PLAYWORLD_PACKAGE_PREFIX TEXT("UEDPIE")
#define PLAYWORLD_CONSOLE_BASE_PACKAGE_PREFIX TEXT("UED")

/*-----------------------------------------------------------------------------
	Level updating. Moved out here from UnLevel.h so that AActor.h can use it (needed for gcc)
-----------------------------------------------------------------------------*/

enum ELevelTick
{
	LEVELTICK_TimeOnly		= 0,	// Update the level time only.
	LEVELTICK_ViewportsOnly	= 1,	// Update time and viewports.
	LEVELTICK_All			= 2,	// Update all.
};

enum EAIFunctions
{
	AI_PollMoveTo = 501,
	AI_PollMoveToward = 503,
	AI_PollStrafeTo = 505,
	AI_PollStrafeFacing = 507,
	AI_PollFinishRotation = 509,
	AI_PollWaitForLanding = 528,
};

/*-----------------------------------------------------------------------------
	FaceFX support.
-----------------------------------------------------------------------------*/
#if WITH_FACEFX
#	include "UnFaceFXSupport.h"
#endif // WITH_FACEFX

/** Whether to support VSM (Variance Shadow Map) for shadow projection */
#ifndef SUPPORTS_VSM
#define SUPPORTS_VSM 0
#endif

/**
 * Engine stats
 */
enum EEngineStats
{
	/** Terrain stats */
	STAT_TerrainFoliageTime = STAT_EngineFirstStat,
	STAT_FoliageRenderTime,
	STAT_FoliageTickTime,
	STAT_TerrainRenderTime,
	STAT_TerrainSmoothTime,
	STAT_TerrainTriangles,
	STAT_TerrainFoliageInstances,
	/** Input stat */
	STAT_InputTime,
	STAT_InputLatencyTime,
	/** HUD stat */
	STAT_HudTime,
	/** Shadow volume stats */
	STAT_ShadowVolumeTriangles,
	STAT_ShadowExtrusionTime,
	STAT_ShadowVolumeRenderTime,
	/** Static mesh tris rendered */
	STAT_StaticMeshTriangles,
	STAT_SkinningTime,
	STAT_UpdateClothVertsTime,
	STAT_UpdateSoftBodyVertsTime,
	STAT_SkelMeshTriangles,
	STAT_SkelMeshDrawCalls,
	STAT_CPUSkinVertices,
	STAT_GPUSkinVertices,
	STAT_FracturePartPoolUsed,
	STAT_GameEngineTick,
	STAT_GameViewportTick,
	STAT_RedrawViewports,
	STAT_UpdateLevelStreaming,
	STAT_RHITickTime
};

/**
 * Game stats
 */
enum EGameStats
{
	STAT_DecalTime = STAT_UnrealScriptTime + 1,
	STAT_PhysicsTime,
	STAT_SpawnActorTime,
	STAT_MoveActorTime,
	STAT_FarMoveActorTime,
	STAT_GCMarkTime,
	STAT_GCSweepTime,
	STAT_UpdateComponentsTime,
	STAT_KismetTime,
	STAT_TempTime,
	STAT_TempTime1,
	STAT_TempTime2,
	STAT_TempTime3,
	/** Ticking stats */
	STAT_PostUpdateTickTime,
	STAT_PostAsyncTickTime,
	STAT_DuringAsyncTickTime,
	STAT_PreAsyncTickTime,
	STAT_PostUpdateComponentTickTime,
	STAT_PostAsyncComponentTickTime,
	STAT_DuringAsyncComponentTickTime,
	STAT_PreAsyncComponentTickTime,
	STAT_TickTime,
	STAT_WorldTickTime,
	STAT_PostTickComponentUpdate,
	STAT_ParticleManagerUpdateData,
	STAT_PostUpdateWorkActorsTicked,
	STAT_PostAsyncActorsTicked,
	STAT_DuringAsyncActorsTicked,
	STAT_PreAsyncActorsTicked,
	STAT_PostAsyncComponentsTicked,
	STAT_DuringAsyncComponentsTicked,
	STAT_PreAsyncComponentsTicked,
	STAT_AsyncWorkWaitTime
};

/**
 * FPS chart stats
 */
enum FPSChartStats
{
	STAT_FPSChart_0_5	= STAT_FPSChartFirstStat,
	STAT_FPSChart_5_10,
	STAT_FPSChart_10_15,
	STAT_FPSChart_15_20,
	STAT_FPSChart_20_25,
	STAT_FPSChart_25_30,
	STAT_FPSChart_30_35,
	STAT_FPSChart_35_40,
	STAT_FPSChart_40_45,
	STAT_FPSChart_45_50,
	STAT_FPSChart_50_55,
	STAT_FPSChart_55_60,
	STAT_FPSChart_60_INF,
	STAT_FPSChartLastBucketStat,
	STAT_FPSChart_30Plus,
	STAT_FPSChart_UnaccountedTime,
	STAT_FPSChart_FrameCount,

	/** Hitch stats */
	STAT_FPSChart_FirstHitchStat,
	STAT_FPSChart_Hitch_5000_Plus = STAT_FPSChart_FirstHitchStat,
	STAT_FPSChart_Hitch_2500_5000,
	STAT_FPSChart_Hitch_2000_2500,
	STAT_FPSChart_Hitch_1500_2000,
	STAT_FPSChart_Hitch_1000_1500,
	STAT_FPSChart_Hitch_750_1000,
	STAT_FPSChart_Hitch_500_750,
	STAT_FPSChart_Hitch_300_500,
	STAT_FPSChart_Hitch_200_300,
	STAT_FPSChart_Hitch_150_200,
	STAT_FPSChart_Hitch_100_150,
	STAT_FPSChart_LastHitchBucketStat,
	STAT_FPSChart_TotalHitchCount,
};

/**
 * Path finding stats
 */
enum EPathFindingStats
{
	STAT_PathFinding_Reachable = STAT_PathFindingFirstStat,
	STAT_PathFinding_FindPathToward,
	STAT_PathFinding_BestPathTo,
};

/**
 * UI Stats
 */
enum EUIStats
{
	STAT_UIKismetTime = STAT_UIFirstStat,
	STAT_UITickTime,
	STAT_UIDrawingTime,
};

/**
* Canvas Stats
*/
enum ECanvasStats
{
	STAT_Canvas_FlushTime = STAT_CanvasFirstStat,	
	STAT_Canvas_DrawTextureTileTime,
	STAT_Canvas_DrawMaterialTileTime,
	STAT_Canvas_DrawStringTime,
	STAT_Canvas_GetBatchElementsTime,
	STAT_Canvas_AddTileRenderTime,
	STAT_Canvas_NumBatchesCreated	
};

/**
* Decal stats
*/
enum EDecalStats
{
	STAT_DecalAttachTime = STAT_DecalFirstStat,
	STAT_DecalBSPAttachTime,
	STAT_DecalStaticMeshAttachTime,
	STAT_DecalSkeletalMeshAttachTime,
	STAT_DecalTerrainAttachTime,
	STAT_DecalHitComponentAttachTime,
	STAT_DecalHitNodeAttachTime,
	STAT_DecalMultiComponentAttachTime,
	STAT_DecalReceiverImagesAttachTime,

	/** Decal stats */
	STAT_DecalTriangles,
	STAT_DecalDrawCalls,
	STAT_DecalRenderUnlitTime,
	STAT_DecalRenderLitTime,
	STAT_DecalRenderDynamicBSPTime,
	STAT_DecalRenderDynamicSMTime,
	STAT_DecalRenderDynamicTerrainTime,
	STAT_DecalRenderDynamicSkelTime
};


/*-----------------------------------------------------------------------------
	Forward declarations.
-----------------------------------------------------------------------------*/
class UTexture;
class UTexture2D;
class FLightMap2D;
class UShadowMap2D;
class FSceneInterface;
class FPrimitiveSceneInfo;
class FPrimitiveSceneProxy;
class UMaterialExpression;
class FMaterialRenderProxy;
class UMaterial;
class FSceneView;
class FSceneViewFamily;
class FViewportClient;
class FCanvas;

#define ENUMS_ONLY 1
#include "EngineClasses.h"
#include "EngineSceneClasses.h"
#undef ENUMS_ONLY

/*-----------------------------------------------------------------------------
	Engine public includes.
-----------------------------------------------------------------------------*/
struct FURL;

#include "UnTickable.h"						// FTickableObject interface.
#include "UnSelection.h"					// Tools used by UnrealEd for managing resource selections.
#include "UnContentStreaming.h"				// Content streaming class definitions.
#include "UnRenderUtils.h"					// Render utility classes.
#include "HitProxies.h"						// Hit proxy definitions.
#include "ConvexVolume.h"					// Convex volume definition.
#include "ShaderCompiler.h"					// Platform independent shader compilation definitions.
#include "RHI.h"							// Common RHI definitions.
#include "RenderingThread.h"				// Rendering thread definitions.
#include "RenderResource.h"					// Render resource definitions.
#include "RHIStaticStates.h"				// RHI static state template definition.
#include "RawIndexBuffer.h"					// Raw index buffer definitions.
#include "VertexFactory.h"					// Vertex factory definitions.
#include "MaterialShared.h"					// Shared material definitions.
#include "ShaderManager.h"					// Shader manager definitions.
#include "ShaderCache.h"					// Shader cache definitions.
#include "GlobalShader.h"					// Global shader definitions.
#include "MaterialShader.h"					// Material shader definitions.
#include "MeshMaterialShader.h"				// Mesh material shader defintions.
#include "LensFlareVertexFactory.h"			// Lens flare vertex factory definition.
#include "LocalVertexFactory.h"				// Local vertex factory definition.
#include "ParticleVertexFactory.h"			// Particle sprite vertex factory definition.
#include "ParticleSubUVVertexFactory.h"		// Particle sprite subUV vertex factory definition.
#include "ParticleBeamTrailVertexFactory.h"	// Particle beam/trail vertex factory definition.
#include "TerrainVertexFactory.h"			// Terrain vertex factory definition
#include "UnClient.h"						// Platform specific client interface definition.
#include "UnTex.h"							// Textures.
#include "SystemSettings.h"					// Scalability options.
#include "UnObj.h"							// Standard object definitions.
#include "UnPrim.h"							// Primitive class.
#include "UnShadowVolume.h"					// Shadow volume definitions.
#include "UnAnimTree.h"						// Animation.
#include "UnComponents.h"					// Forward declarations of object components of actors
#include "Scene.h"							// Scene management.
#include "DynamicMeshBuilder.h"				// Dynamic mesh builder.
#include "PreviewScene.h"					// Preview scene definitions.
#include "UnPhysPublic.h"					// Public physics integration types.
#include "LightingBuildOptions.h"			// Definition of lighting build option struct.
#include "StaticLighting.h"					// Static lighting definitions.
#include "UnActorComponent.h"				// Actor component definitions.
#include "PrimitiveComponent.h"				// Primitive component definitions.
#include "UnParticleHelper.h"				// Particle helper definitions.
#include "ParticleEmitterInstances.h"
#include "UnSceneCapture.h"					// Scene render to texture probes
#include "UnPhysAsset.h"					// Physics Asset.
#include "UnInterpolation.h"				// Matinee.
#include "UnForceFeedbackWaveform.h"		// Gamepad vibration support
#include "GenericOctreePublic.h"			// Generic octree public defines
#define NO_ENUMS 1
#include "EngineClasses.h"					// All actor classes.
#undef NO_ENUMS
#include "UnLightMap.h"						// Light-maps.
#include "UnShadowMap.h"					// Shadow-maps.
#include "UnModel.h"						// Model class.
#include "UnPrefab.h"						// Prefabs.
#include "UnPhysic.h"						// Physics constants
#include "EngineGameEngineClasses.h"		// Main Unreal engine declarations
#include "UnLevel.h"						// Level object.
#include "UnWorld.h"						// World object.
#include "UnKeys.h"							// Key name definitions.
#include "UnUIKeys.h"						// UI key name definitions.
#include "UnEngine.h"						// Unreal engine helpers.
#include "UnSkeletalMesh.h"					// Skeletal animated mesh.
#include "UnMorphMesh.h"					// Morph target mesh
#include "UnActor.h"						// Actor inlines.
#include "UnAudio.h"						// Audio code.
#include "UnStaticMesh.h"					// Static T&L meshes.
#include "UnCDKey.h"						// CD key validation.
#include "UnCanvas.h"						// Canvas.
#include "UnPNG.h"							// PNG helper code for storing compressed source art.
#include "UnStandardObjectPropagator.h"		// A class containing common propagator code.
#include "UnMiscDeclarations.h"				// Header containing misc class declarations.
#include "UnPostProcess.h"					// Post process defs/decls
#include "FullScreenMovie.h"				// Full screen movie playback support
#include "FullScreenMovieFallback.h"		// Full screen movie fallback 

#include "LensFlare.h"

/*-----------------------------------------------------------------------------
	Hit proxies.
-----------------------------------------------------------------------------*/

// Hit an actor.
struct HActor : public HHitProxy
{
	DECLARE_HIT_PROXY(HActor,HHitProxy)
	AActor* Actor;
	HActor( AActor* InActor ) : Actor( InActor ) {}
	HActor( AActor* InActor, EHitProxyPriority InPriority) :
		HHitProxy(InPriority),
		Actor(InActor) {}

	virtual void Serialize(FArchive& Ar)
	{
		Ar << Actor;
	}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

/**
 * Hit an actor with additional information.
 */
struct HActorComplex : public HHitProxy
{
	DECLARE_HIT_PROXY(HActorComplex,HHitProxy)
	/** Owning actor */
	AActor *Actor;
	/** Any additional information to be stored */
	FString Desc;
	INT Index;

	HActorComplex(AActor *InActor, const TCHAR *InDesc, INT InIndex)
	: Actor(InActor)
	, Desc(InDesc)
	, Index(InIndex)
	{
	}

	virtual void Serialize(FArchive& Ar)
	{
		Ar << Actor;
		Ar << Desc;
		Ar << Index;
	}
	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

//
//	HBSPBrushVert
//

struct HBSPBrushVert : public HHitProxy
{
	DECLARE_HIT_PROXY(HBSPBrushVert,HHitProxy);
	ABrush*	Brush;
	FVector* Vertex;
	HBSPBrushVert(ABrush* InBrush,FVector* InVertex):
		HHitProxy(HPP_UI),
		Brush(InBrush),
		Vertex(InVertex)
	{}
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Brush;
	}
};

//
//	HStaticMeshVert
//

struct HStaticMeshVert : public HHitProxy
{
	DECLARE_HIT_PROXY(HStaticMeshVert,HHitProxy);
	AActor*	Actor;
	FVector Vertex;
	HStaticMeshVert(AActor* InActor,FVector InVertex):
		HHitProxy(HPP_UI),
		Actor(InActor),
		Vertex(InVertex)
	{}
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Actor << Vertex;
	}
};

/* ==========================================================================================================
	Data stores
========================================================================================================== */

/**
 * This class creates and holds a reference to the global data store client.
 */
class FGlobalDataStoreClientManager
{
public:
	/** Destructor */
	virtual ~FGlobalDataStoreClientManager() {}

protected:
	/**
	 * Creates and initializes an instance of a UDataStoreClient.
	 *
	 * @param	InOuter		the object to use for Outer when creating the global data store client
	 *
	 * @return	a pointer to a fully initialized instance of the global data store client class.
	 */
	class UDataStoreClient* CreateGlobalDataStoreClient( UObject* InOuter ) const;

	/**
	 * Initializes the singleton data store client that will manage the global data stores.
	 */
	virtual void InitializeGlobalDataStore()=0;
};

/*-----------------------------------------------------------------------------
	Terrain editing brush types.
-----------------------------------------------------------------------------*/

enum ETerrainBrush
{
	TB_None				= -1,
	TB_VertexEdit		= 0,	// Select/Drag Vertices on heightmap
	TB_Paint			= 1,	// Paint on selected layer
	TB_Smooth			= 2,	// Does a filter on the selected vertices
	TB_Noise			= 3,	// Adds random noise into the selected vertices
	TB_Flatten			= 4,	// Flattens the selected vertices to the height of the vertex which was initially clicked
	TB_TexturePan		= 5,	// Pans the texture on selected layer
	TB_TextureRotate	= 6,	// Rotates the texture on selected layer
	TB_TextureScale		= 7,	// Scales the texture on selected layer
	TB_Select			= 8,	// Selects areas of the terrain for copying, generating new terrain, etc
	TB_Visibility		= 9,	// Toggles terrain sectors on/off
	TB_Color			= 10,	// Paints color into the RGB channels of layers
	TB_EdgeTurn			= 11,	// Turns edges of terrain triangulation
};

/*-----------------------------------------------------------------------------
	Iterator for the editor that loops through all selected actors.
-----------------------------------------------------------------------------*/

/**
 * Abstract base class for actor iteration. Implements all operators and relies on IsSuitable
 * to be overridden by derived classed. Also the code currently relies on the ++ operator being
 * called from within the constructor which is bad as it uses IsSuitable which is a virtual function.
 * This means that all derived classes are treated as "final" and they need to manually call
 * ++(*this) in their constructor so it ends up calling the right one. The long term plan is to
 * replace the use of virtual functions with template tricks.
 * Note that when Playing In Editor, this will find actors only in GWorld.
 */
class FActorIteratorBase
{
protected:
	/** Current index into actors array							*/
	INT		ActorIndex;
	/** Current index into levels array							*/
	INT		LevelIndex;
	/** Whether we already reached the end						*/
	UBOOL	ReachedEnd;
	/** Number of actors that have been considered thus far		*/
	INT		ConsideredCount;
	/** Current actor pointed to by actor iterator				*/
	AActor*	CurrentActor;

	/**
	 * Default ctor, inits everything
	 */
	FActorIteratorBase(void) :
		ActorIndex( -1 ),
		LevelIndex( 0 ),
		ReachedEnd( FALSE ),
		ConsideredCount( 0 ),
		CurrentActor( NULL )
	{
		check(IsInGameThread());
	}

public:
	/**
	 * Returns the actor count.
	 *
	 * @param total actor count
	 */
	static INT GetProgressDenominator(void);
	/**
	 * Returns the actor count.
	 *
	 * @param total actor count
	 */
	static INT GetActorCount(void);
	/**
	 * Returns the dynamic actor count.
	 *
	 * @param total dynamic actor count
	 */
	static INT GetDynamicActorCount(void);
	/**
	 * Returns the net relevant actor count.
	 *
	 * @param total net relevant actor count
	 */
	static INT GetNetRelevantActorCount(void);

	/**
	 * Returns the current suitable actor pointed at by the Iterator
	 *
	 * @return	Current suitable actor
	 */
	FORCEINLINE AActor* operator*()
	{
		check(CurrentActor);
		checkf(!CurrentActor->HasAnyFlags(RF_Unreachable), TEXT("%s"), *CurrentActor->GetFullName());
		return CurrentActor;
	}
	/**
	 * Returns the current suitable actor pointed at by the Iterator
	 *
	 * @return	Current suitable actor
	 */
	FORCEINLINE AActor* operator->()
	{
		check(CurrentActor);
		checkf(!CurrentActor->HasAnyFlags(RF_Unreachable), TEXT("%s"), *CurrentActor->GetFullName());
		return CurrentActor;
	}
	/**
	 * Returns whether the iterator has reached the end and no longer points
	 * to a suitable actor.
	 *
	 * @return TRUE if iterator points to a suitable actor, FALSE if it has reached the end
	 */
	FORCEINLINE operator UBOOL()
	{
		return !ReachedEnd;
	}

	/**
	 * Returns whether the currently pointed to actor is a level's world info (actor index 0).
	 *
	 * return TRUE if the currently pointed to actor is a level's world info, FALSE otherwise
	 */
	UBOOL IsWorldInfo() const
	{
		check(!ReachedEnd);
		check(ActorIndex>=0);
		if( ActorIndex == 0 )
		{
			check(CurrentActor);
			check(CurrentActor->IsA(AWorldInfo::StaticClass()));
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	/**
	 * Returns whether the currently pointed to actor is a level's default brush (actor index 1).
	 *
	 * return TRUE if the currently pointed to actor is a level default brush, FALSE otherwise
	 */
	UBOOL IsDefaultBrush() const
	{
		check(!ReachedEnd);
		check(ActorIndex>=0);
		if( ActorIndex == 1 )
		{
			check(CurrentActor);
			check(CurrentActor->IsA(ABrush::StaticClass()));
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	/**
	 * Removes the current Actor from the array.
	 */
	void RemoveCurrent()
	{
		check(!ReachedEnd);
		ULevel* Level = GWorld->Levels(LevelIndex);
		Level->Actors.Remove(ActorIndex--);
	}

	/**
	 * Clears the current Actor in the array (setting it to NULL).
	 */
	void ClearCurrent()
	{
		check(!ReachedEnd);
		ULevel* Level = GWorld->Levels(LevelIndex);
		Level->Actors.ModifyItem(ActorIndex);
		Level->Actors(ActorIndex) = NULL;
	}

	/**
	 * Returns the number of actors considered thus far. Can be used in combination
	 * with GetProgressDenominator to gauge progress iterating over all actors.
	 *
	 * @return number of actors considered thus far.
	 */
	INT GetProgressNumerator()
	{
		return ConsideredCount;
	}
};

/**
 * Default level iteration filter. Doesn't cull levels out.
 */
struct FDefaultLevelFilter
{
	/**
	 * Used to examine whether this level is valid for iteration or not
	 *
	 * @param Level the level to check for iteration
	 *
	 * @return TRUE if the level can be iterated, FALSE otherwise
	 */
	UBOOL CanIterateLevel(ULevel* Level) const
	{
		return TRUE;
	}
};

/**
 * Filter class that prevents ticking of levels that are still loading
 */
struct FTickableLevelFilter
{
	/**
	 * Used to examine whether this level is valid for iteration or not
	 *
	 * @param Level the level to check for iteration
	 *
	 * @return TRUE if the level can be iterated, FALSE otherwise
	 */
	UBOOL CanIterateLevel(ULevel* Level) const
	{
		return !Level->bHasVisibilityRequestPending || GIsAssociatingLevel;
	}
};

/**
 * Template class used to avoid the virtual function call cost for filtering
 * actors by certain characteristics
 */
template<typename FILTER_CLASS,typename LEVEL_FILTER_CLASS = FDefaultLevelFilter>
class TActorIteratorBase :
	public FActorIteratorBase
{
protected:
	/**
	 * Ref to the class that is going to filter actors
	 */
	const FILTER_CLASS& FilterClass;
	/**
	 * Ref to the object that is going to filter levels
	 */
	const LEVEL_FILTER_CLASS& LevelFilterClass;

	/**
	 * Hide the constructor as construction on this class should only be done by subclasses
	 */
	TActorIteratorBase(const FILTER_CLASS& InFilterClass = FILTER_CLASS(),
		const LEVEL_FILTER_CLASS& InLevelClass = LEVEL_FILTER_CLASS())
	:	FilterClass(InFilterClass),
		LevelFilterClass(InLevelClass)
	{
	}

public:
	/**
	 * Iterates to next suitable actor.
	 */
	void operator++()
	{
		// Use local version to avoid LHSs as compiler is not required to write out member variables to memory.
		AActor* LocalCurrentActor	= NULL;
		INT		LocalActorIndex		= ActorIndex;
		ULevel*	Level				= GWorld->Levels(LevelIndex);
		INT		LevelActorNum		= Level->Actors.Num();

		// Iterate over actors till a suitable is found or we reach the end.
		while( !ReachedEnd && !LocalCurrentActor )
		{
			// If this level can't be iterating or is at the end of its list
			if( LevelFilterClass.CanIterateLevel(Level) == FALSE ||
				++LocalActorIndex >= LevelActorNum )
			{
				if( ++LevelIndex >= GWorld->Levels.Num() )
				{
					LocalActorIndex = 0;
					LevelIndex = 0;
					ReachedEnd = TRUE;
					break;
				}
				else
				{
					// Get the next level for iterating
					Level = GWorld->Levels(LevelIndex);
					LevelActorNum = Level->Actors.Num();
					// Skip processing levels that can't be ticked
					if (LevelFilterClass.CanIterateLevel(Level) == FALSE)
					{
						continue;
					}

					// Now get the actor index to start iterating at
					LocalActorIndex	= FilterClass.GetFirstSuitableActorIndex( Level );
					
					// Gracefully handle levels with insufficient number of actors.
					if( LocalActorIndex >= LevelActorNum )
					{
						continue;
					}
				}
			}
			ConsideredCount++;

			if (LocalActorIndex+2 < LevelActorNum)
			{
				CONSOLE_PREFETCH(((char*)Level->Actors(LocalActorIndex+2)));
				CONSOLE_PREFETCH(((char*)Level->Actors(LocalActorIndex+2)) + CACHE_LINE_SIZE);
			}
			// See whether current actor is suitable for actor iterator and reset it if not.
			LocalCurrentActor = Level->Actors(LocalActorIndex);
			if( !FilterClass.IsSuitable( LocalActorIndex, LocalCurrentActor ) )
			{
				LocalCurrentActor = NULL;
			}
		}

		// Propagate persistent data from local version.
		CurrentActor	= LocalCurrentActor;
		ActorIndex		= LocalActorIndex;
	}
};

/**
 * Base filter class for actor filtering in iterators.
 */
class FActorFilter
{
public:
	/**
	 * Determines whether this is a valid actor or not
	 *
	 * @param	ActorIndex ignored
	 * @param	Actor	Actor to check
	 * @return	TRUE if actor is != NULL, FALSE otherwise
	 */
	FORCEINLINE UBOOL IsSuitable( INT /*ActorIndex*/, AActor* Actor ) const
	{
		return Actor != NULL;
	}

	/**
	 * Returns the index of the first suitable actor in the passed in level. Used as an 
	 * optimization for e.g. the dynamic and net relevant actor iterators.
	 *
	 * @param	Level	level to use
	 * @return	first suitable actor index in level
	 */
	FORCEINLINE INT GetFirstSuitableActorIndex( ULevel* Level ) const
	{
		// Skip the AWorldInfo actor (index == 0) for levels that aren't the persistent one.
		if( Level == GWorld->PersistentLevel )
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
};

/**
 * Actor iterator
 * Note that when Playing In Editor, this will find actors only in GWorld
 */
class FActorIterator :
	public TActorIteratorBase<FActorFilter, FTickableLevelFilter>
{
public:
	/**
	 * Constructor, inits the starting position for iteration
	 */
	FActorIterator()
	{
		++(*this);
	}
};

/**
 * Filters actors by whether or not they are selected
 */
class FSelectedActorFilter :
	public FActorFilter
{
public:
	/**
	 * Determines if the actor should be returned during iteration or not
	 *
	 * @param	ActorIndex ignored
	 * @param	Actor	Actor to check
	 * @return	TRUE if actor is selected, FALSE otherwise
	 */
	FORCEINLINE UBOOL IsSuitable( INT /*ActorIndex*/, AActor* Actor ) const
	{
		return Actor ? Actor->IsSelected() : FALSE;
	}
};

/**
 * Selected actor iterator
 */
class FSelectedActorIterator :
	public TActorIteratorBase<FSelectedActorFilter>
{
public:
	/**
	 * Constructor, inits the starting position for iteration
	 */
	FSelectedActorIterator()
	{
		++(*this);
	}
};

/**
 * Filters actors by whether or not they are dynamic
 */
class FDynamicActorFilter
{
public:
	/**
	 * Determines if the actor should be returned during iteration or not
	 *
	 * @param	ActorIndex ignored
	 * @param	Actor	Actor to check
	 * @return	TRUE if actor is selected, FALSE otherwise
	 */
	FORCEINLINE UBOOL IsSuitable( INT /*ActorIndex*/, AActor* Actor ) const
	{
		return (Actor != NULL && !Actor->bStatic);
	}

	/**
	 * Returns the index of the first suitable actor in the passed in level. Used as an 
	 * optimization for e.g. the dynamic and net relevant actor iterators.
	 *
	 * @param	Level	level to use
	 * @return	first suitable actor index in level
	 */
	FORCEINLINE INT GetFirstSuitableActorIndex( ULevel* Level ) const
	{
		return Level->iFirstDynamicActor;
	}
};

/**
 * Dynamic actor iterator
 */
class FDynamicActorIterator :
	public TActorIteratorBase<FDynamicActorFilter, FTickableLevelFilter>
{
public:
	/**
	 * Positions the iterator at the first dynamic actor
	 */
	FDynamicActorIterator()
	{
		// Override initialization of ActorIndex.
		ULevel* Level	= GWorld->Levels(LevelIndex);
		ActorIndex		= FilterClass.GetFirstSuitableActorIndex( Level ) - 1; // -1 as ++ will increase Actor index first
		++(*this);
	}
};

/**
 * Filters actors by whether or not they are network relevant
 */
class FNetRelevantActorFilter :
	public FActorFilter
{
public:
	/**
	 * Returns the index of the first suitable actor in the passed in level. Used as an 
	 * optimization for e.g. the dynamic and net relevant actor iterators.
	 *
	 * @param	Level	level to use
	 * @return	first suitable actor index in level
	 */
	FORCEINLINE INT GetFirstSuitableActorIndex( ULevel* Level ) const
	{
		return Level->iFirstNetRelevantActor;
	}
};

/**
 * Net relevant actor iterator
 */
class FNetRelevantActorIterator :
	public TActorIteratorBase<FNetRelevantActorFilter, FTickableLevelFilter>
{
public:
	/**
	 * Positions the iterator at the first network relevant actor
	 */
	FNetRelevantActorIterator()
	{
		// Override initialization of ActorIndex.
		ULevel* Level	= GWorld->Levels(LevelIndex);
		ActorIndex		= FilterClass.GetFirstSuitableActorIndex( Level ) - 1; // -1 as ++ will increase Actor index first
		++(*this);
	}
};

/**
 * An output device that forwards output to both the log and the console.
 */
class FConsoleOutputDevice : public FStringOutputDevice
{
public:

	/**
	 * Minimal initialization constructor.
	 */
	FConsoleOutputDevice(class UConsole* InConsole):
		FStringOutputDevice(TEXT("")),
		Console(InConsole)
	{}

	/** @name FOutputDevice interface. */
	//@{
	virtual void Serialize(const TCHAR* Text,EName Event);
	//@}

private:

	/** The console which output is written to. */
	UConsole* Console;
};

#include "UnSurveys.h"

#endif // _INC_ENGINE

