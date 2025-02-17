/*=============================================================================
	SceneCore.cpp: Core scene implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "AllocatorFixedSizeFreeList.h"
#include "LightRendering.h"

#if WANTS_DRAW_MESH_EVENTS && PLATFORM_SUPPORTS_DRAW_MESH_EVENTS
/** Global counter of draw events, incremented when issued and reset by code updating STATS. */
DWORD FDrawEvent::Counter;
#endif

/**
* Fixed Size pool allocator for FLightPrimitiveInteractions
*/
TAllocatorFixedSizeFreeList<sizeof(FLightPrimitiveInteraction), 16> GLightPrimitiveInteractionAllocator;

/*-----------------------------------------------------------------------------
	FLightPrimitiveInteraction
-----------------------------------------------------------------------------*/

/**
 * Custom new
 */
void* FLightPrimitiveInteraction::operator new(size_t Size)
{
	// doesn't support derived classes with a different size
	checkSlow(Size == sizeof(FLightPrimitiveInteraction));
	return GLightPrimitiveInteractionAllocator.Allocate();
	//return appMalloc(Size);
}

/**
 * Custom delete
 */
void FLightPrimitiveInteraction::operator delete(void *RawMemory)
{
	GLightPrimitiveInteractionAllocator.Free(RawMemory);
	//appFree(RawMemory);
}	

/**
 * Initialize the memory pool with a default size from the ini file.
 * Called at render thread startup. Since the render thread is potentially
 * created/destroyed multiple times, must make sure we only do it once.
 */
void FLightPrimitiveInteraction::InitializeMemoryPool()
{
	static UBOOL bAlreadyInitialized = FALSE;
	if (!bAlreadyInitialized)
	{
		bAlreadyInitialized = TRUE;
		INT InitialBlockSize = 0;
		GConfig->GetInt(TEXT("MemoryPools"), TEXT("FLightPrimitiveInteractionInitialBlockSize"), InitialBlockSize, GEngineIni);
		GLightPrimitiveInteractionAllocator.Grow(InitialBlockSize);
	}
}

void FLightPrimitiveInteraction::Create(FLightSceneInfo* LightSceneInfo,FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	// Attach the light to the primitive's static meshes.
	UBOOL bDynamic = TRUE;
	UBOOL bRelevant = FALSE;
	UBOOL bLightMapped = TRUE;

	// Determine the light's relevance to the primitive.
	check(PrimitiveSceneInfo->Proxy);
	PrimitiveSceneInfo->Proxy->GetLightRelevance(LightSceneInfo, bDynamic, bRelevant, bLightMapped);

	// Special case handling for movable actors with precomputed lighting interacting with static lights that are active
	// during gameplay, like e.g. toggleable ones. The intention for precomputing lighting on moving objects is that
	// other 'static' lights won't affect them so we remove those interactions.
	//
	// Only look at interactions considered dynamic and relative.
	if( bDynamic 
	&&	bRelevant
	// Primitive needs to support static shadowing, be movable and the light needs to have static shadowing
	&&	PrimitiveSceneInfo->bStaticShadowing 
	&&	PrimitiveSceneInfo->Proxy->IsMovable()
	&&	LightSceneInfo->bStaticShadowing )
	{
		// If all that is the case, don't create the interaction.
		bRelevant = FALSE;
	}

	if( bRelevant )
	{
		// Attach the light to the primitive's static meshes.
		for(INT ElementIndex = 0;ElementIndex < PrimitiveSceneInfo->StaticMeshes.Num();ElementIndex++)
		{
			FMeshLightingDrawingPolicyFactory::AddStaticMesh(
				PrimitiveSceneInfo->Scene,
				&PrimitiveSceneInfo->StaticMeshes(ElementIndex),
				LightSceneInfo
				);
		}

		// Also handle static meshes for decal interactions
		for( INT DecalIdx = 0; DecalIdx < PrimitiveSceneInfo->Proxy->Decals.Num(); DecalIdx++ )
		{
			FDecalInteraction* Decal = PrimitiveSceneInfo->Proxy->Decals(DecalIdx);
			if( Decal && Decal->DecalStaticMesh )
			{
				FMeshLightingDrawingPolicyFactory::AddStaticMesh(
					PrimitiveSceneInfo->Scene,
					Decal->DecalStaticMesh,
					LightSceneInfo
					);				
			}
		}


		// Create the light interaction.
		FLightPrimitiveInteraction* Interaction = new FLightPrimitiveInteraction(LightSceneInfo,PrimitiveSceneInfo,bDynamic,bLightMapped);

		// Attach the light to the primitive.
		LightSceneInfo->AttachPrimitive(*Interaction);
	}
}

void FLightPrimitiveInteraction::Destroy(FLightPrimitiveInteraction* LightPrimitiveInteraction)
{
	delete LightPrimitiveInteraction;
}

FLightPrimitiveInteraction::FLightPrimitiveInteraction(
	FLightSceneInfo* InLightSceneInfo,
	FPrimitiveSceneInfo* InPrimitiveSceneInfo,
	UBOOL bInIsDynamic,
	UBOOL bInLightMapped
	):
	ModShadowStartFadeOutPercent(1.0f),
	// Disable the fade in, as there is currently a one frame pop and moving the shadow between DPG's is not handled.
	ModShadowStartFadeInPercent(1.0f),
	LightId(InLightSceneInfo->Id),
	LightSceneInfo(InLightSceneInfo),
	PrimitiveSceneInfo(InPrimitiveSceneInfo),
	bLightMapped(bInLightMapped),
	bIsDynamic(bInIsDynamic),
	bUncachedStaticLighting(FALSE)
{
	// Determine whether this light-primitive interaction produces a shadow.
	if(PrimitiveSceneInfo->bStaticShadowing)
	{
		const UBOOL bHasStaticShadow =
			LightSceneInfo->bStaticShadowing &&
			LightSceneInfo->bCastStaticShadow &&
			PrimitiveSceneInfo->bCastStaticShadow;
		const UBOOL bHasDynamicShadow =
			!LightSceneInfo->bStaticShadowing &&
			LightSceneInfo->bCastDynamicShadow &&
			PrimitiveSceneInfo->bCastDynamicShadow;
		bCastShadow = bHasStaticShadow || bHasDynamicShadow;
	}
	else
	{
		if(PrimitiveSceneInfo->LightEnvironment != NULL && LightSceneInfo->LightEnvironment == NULL && LightSceneInfo->bCastCompositeShadow)
		{
			// If the light will affect the primitive's light environment's composite shadow, don't use non-composite shadow casting.
			bCastShadow = FALSE;
		}
		else
		{
			bCastShadow = LightSceneInfo->bCastDynamicShadow && PrimitiveSceneInfo->bCastDynamicShadow;
		}
	}

	if(bCastShadow && bIsDynamic)
	{
		// Determine the type of dynamic shadow produced by this light.
		if(PrimitiveSceneInfo->bStaticShadowing)
		{
			if(LightSceneInfo->bStaticShadowing && PrimitiveSceneInfo->bCastStaticShadow)
			{
				// Update the game thread's counter of number of uncached static lighting interactions.
				bUncachedStaticLighting = TRUE;
				appInterlockedIncrement(&PrimitiveSceneInfo->Scene->NumUncachedStaticLightingInteractions);
			}
		}
	}

	// Add the interaction to the light's interaction list.
	if(bIsDynamic)
	{
		PrevPrimitiveLink = &LightSceneInfo->DynamicPrimitiveList;
	}
	else
	{
		PrevPrimitiveLink = &LightSceneInfo->StaticPrimitiveList;
	}
	NextPrimitive = *PrevPrimitiveLink;
	if(*PrevPrimitiveLink)
	{
		(*PrevPrimitiveLink)->PrevPrimitiveLink = &NextPrimitive;
	}
	*PrevPrimitiveLink = this;

	// Add the interaction to the primitive's interaction list.
	PrevLightLink = &PrimitiveSceneInfo->LightList;
	NextLight = *PrevLightLink;
	if(*PrevLightLink)
	{
		(*PrevLightLink)->PrevLightLink = &NextLight;
	}
	*PrevLightLink = this;
}

FLightPrimitiveInteraction::~FLightPrimitiveInteraction()
{
	check(IsInRenderingThread());

	/*
	 * Notify the scene proxy that the scene light is no longer
	 * associated with the proxy.
	 */
	if ( PrimitiveSceneInfo->Proxy )
	{
		PrimitiveSceneInfo->Proxy->OnDetachLight( LightSceneInfo );
	}

	// Update the game thread's counter of number of uncached static lighting interactions.
	if(bUncachedStaticLighting)
	{
		appInterlockedDecrement(&PrimitiveSceneInfo->Scene->NumUncachedStaticLightingInteractions);
	}

	// Detach the light from the primitive.
	LightSceneInfo->DetachPrimitive(*this);

	// Remove the interaction from the light's interaction list.
	if(NextPrimitive)
	{
		NextPrimitive->PrevPrimitiveLink = PrevPrimitiveLink;
	}
	*PrevPrimitiveLink = NextPrimitive;

	// Remove the interaction from the primitive's interaction list.
	if(NextLight)
	{
		NextLight->PrevLightLink = PrevLightLink;
	}
	*PrevLightLink = NextLight;
}

/*-----------------------------------------------------------------------------
	FCaptureSceneInfo
-----------------------------------------------------------------------------*/

/** 
* Constructor 
* @param InComponent - mirrored scene capture component requesting the capture
* @param InSceneCaptureProbe - new probe for capturing the scene
*/
FCaptureSceneInfo::FCaptureSceneInfo(USceneCaptureComponent* InComponent,FSceneCaptureProbe* InSceneCaptureProbe)
:	SceneCaptureProbe(InSceneCaptureProbe)
,	Component(InComponent)
,	RenderThreadId(INDEX_NONE)
,	GameThreadId(INDEX_NONE)
,	Scene(NULL)
{
	check(Component);
	check(SceneCaptureProbe);
    check(InComponent->CaptureInfo == NULL);
	InComponent->CaptureInfo = this;	
}

/** 
* Destructor
*/
FCaptureSceneInfo::~FCaptureSceneInfo()
{
	RemoveFromScene(Scene);
	delete SceneCaptureProbe;
}

/**
* Capture the scene
* @param SceneRenderer - original scene renderer so that we can match certain view settings
*/
void FCaptureSceneInfo::CaptureScene(class FSceneRenderer* SceneRenderer)
{
	if( SceneCaptureProbe )
	{
        SceneCaptureProbe->CaptureScene(SceneRenderer);
	}
}

/**
* Add this capture scene info to a scene 
* @param InScene - scene to add to
*/
void FCaptureSceneInfo::AddToScene(class FScene* InScene)
{
	check(InScene);

	// can only be active in a single scene
	RemoveFromScene(Scene);
	
	// add it to the scene and keep track of Id
	Scene = InScene;
	RenderThreadId = Scene->SceneCapturesRenderThread.AddItem(this);

}

/**
* Remove this capture scene info from a scene 
* @param InScene - scene to remove from
*/
void FCaptureSceneInfo::RemoveFromScene(class FScene* /*InScene*/)
{
	if( Scene &&
		RenderThreadId != INDEX_NONE )
	{
		Scene->SceneCapturesRenderThread.Remove(RenderThreadId);
		Scene = NULL;
	}
}

/*-----------------------------------------------------------------------------
	FStaticMesh
-----------------------------------------------------------------------------*/

void FStaticMesh::LinkDrawList(FStaticMesh::FDrawListElementLink* Link)
{
	check(IsInRenderingThread());
	check(!DrawListLinks.ContainsItem(Link));
	DrawListLinks.AddItem(Link);
}

void FStaticMesh::UnlinkDrawList(FStaticMesh::FDrawListElementLink* Link)
{
	check(IsInRenderingThread());
	verify(DrawListLinks.RemoveItem(Link) == 1);
}

void FStaticMesh::AddToDrawLists(FScene* Scene)
{
	if( IsDecal() )
	{
		// Add the decal static mesh to the DPG's base pass draw list.
		FBasePassOpaqueDrawingPolicyFactory::AddStaticMesh(Scene,this);

		// Add the decal static mesh to the light draw lists for the primitive's light interactions.
		for(FLightPrimitiveInteraction* LightInteraction = PrimitiveSceneInfo->LightList;LightInteraction;LightInteraction = LightInteraction->GetNextLight())
		{
			// separate draw lists are maintained for decals
			FMeshLightingDrawingPolicyFactory::AddStaticMesh(
				Scene,
				this,
				LightInteraction->GetLight()
				);
		}
	}
	else
	{
		// not all platforms need this
		const UBOOL bRequiresHitProxies = Scene->RequiresHitProxies();
		if ( bRequiresHitProxies && PrimitiveSceneInfo->bSelectable )
		{
			// Add the static mesh to the DPG's hit proxy draw list.
			FHitProxyDrawingPolicyFactory::AddStaticMesh(Scene,this);
		}

		if(!IsTranslucent())
		{
			if(DepthPriorityGroup == SDPG_World)
			{
				if(PrimitiveSceneInfo->bUseAsOccluder && !IsMasked())
				{
					// Add the static mesh to the depth-only draw list.
					FDepthDrawingPolicyFactory::AddStaticMesh(Scene,this,DDM_NonMaskedOnly);
				}

				if ( !PrimitiveSceneInfo->bStaticShadowing )
				{
					FVelocityDrawingPolicyFactory::AddStaticMesh(Scene,this);
				}
			}

			// Add the static mesh to the DPG's base pass draw list.
			FBasePassOpaqueDrawingPolicyFactory::AddStaticMesh(Scene,this);
		}

		// Add the static mesh to the light draw lists for the primitive's light interactions.
		for(FLightPrimitiveInteraction* LightInteraction = PrimitiveSceneInfo->LightList;LightInteraction;LightInteraction = LightInteraction->GetNextLight())
		{
			FMeshLightingDrawingPolicyFactory::AddStaticMesh(
				Scene,
				this,
				LightInteraction->GetLight()
				);
		}
	}
}

void FStaticMesh::RemoveFromDrawLists()
{
	// Remove the mesh from all draw lists.
	while(DrawListLinks.Num())
	{
		FStaticMesh::FDrawListElementLink* Link = DrawListLinks(0);
		const INT OriginalNumLinks = DrawListLinks.Num();
		// This will call UnlinkDrawList.
		Link->Remove();
		check(DrawListLinks.Num() == OriginalNumLinks - 1);
		if(DrawListLinks.Num())
		{
			check(DrawListLinks(0) != Link);
		}
	}
}

FStaticMesh::~FStaticMesh()
{
	if( IsDecal() )
	{
		// Remove this decal static mesh from the scene's list.
		PrimitiveSceneInfo->Scene->DecalStaticMeshes.Remove(Id);
	}
	else
	{
		// Remove this static mesh from the scene's list.
		PrimitiveSceneInfo->Scene->StaticMeshes.Remove(Id);
	}
	RemoveFromDrawLists();
}

/** Initialization constructor. */
FHeightFogSceneInfo::FHeightFogSceneInfo(const UHeightFogComponent* InComponent):
	Component(InComponent),
	Height(InComponent->Height),
	Density(InComponent->Density),
	LightColor(FLinearColor(InComponent->LightColor) * InComponent->LightBrightness),
	ExtinctionDistance(InComponent->ExtinctionDistance),
	StartDistance(InComponent->StartDistance)
{
	bIncreaseNearPrecision = GWorld->GetWorldInfo()->GetIncreaseFogNearPrecision();
}
