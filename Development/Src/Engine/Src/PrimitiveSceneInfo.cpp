/*=============================================================================
	PrimitiveSceneInfo.cpp: Primitive scene info implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

#include "EngineParticleClasses.h"

/** An implementation of FStaticPrimitiveDrawInterface that stores the drawn elements for the rendering thread to use. */
class FBatchingSPDI : public FStaticPrimitiveDrawInterface
{
public:

	// Constructor.
	FBatchingSPDI(FPrimitiveSceneInfo* InPrimitiveSceneInfo):
		PrimitiveSceneInfo(InPrimitiveSceneInfo)
	{}

	// FStaticPrimitiveDrawInterface.
	virtual void SetHitProxy(HHitProxy* HitProxy)
	{
		CurrentHitProxy = HitProxy;

		if(HitProxy)
		{
			// Only use static scene primitive hit proxies in the editor.
			if(GIsEditor)
			{
				// Keep a reference to the hit proxy from the FPrimitiveSceneInfo, to ensure it isn't deleted while the static mesh still
				// uses its id.
				PrimitiveSceneInfo->HitProxies.AddItem(HitProxy);
			}
		}
	}
	virtual void DrawMesh(
		const FMeshElement& Mesh,
		FLOAT MinDrawDistance,
		FLOAT MaxDrawDistance
		)
	{
		check(Mesh.NumPrimitives > 0);
		FStaticMesh* StaticMesh = new(PrimitiveSceneInfo->StaticMeshes) FStaticMesh(
			PrimitiveSceneInfo,
			Mesh,
			Square(Max(0.0f,MinDrawDistance)),
			Square(Max(0.0f,MaxDrawDistance)),
			CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
			);
	}

private:
	FPrimitiveSceneInfo* PrimitiveSceneInfo;
	TRefCountPtr<HHitProxy> CurrentHitProxy;
};

void FPrimitiveSceneInfoCompact::Init(FPrimitiveSceneInfo* InPrimitiveSceneInfo)
{
	PrimitiveSceneInfo = InPrimitiveSceneInfo;
	Proxy = PrimitiveSceneInfo->Proxy;
	Component = PrimitiveSceneInfo->Component;
	LightEnvironment = PrimitiveSceneInfo->LightEnvironment;
	Bounds = PrimitiveSceneInfo->Bounds;
	MinDrawDistance = PrimitiveSceneInfo->MinDrawDistance;
	MaxDrawDistance = PrimitiveSceneInfo->MaxDrawDistance;
	LightingChannels = PrimitiveSceneInfo->LightingChannels;

	bAllowApproximateOcclusion = PrimitiveSceneInfo->bAllowApproximateOcclusion;
	bFirstFrameOcclusion = PrimitiveSceneInfo->bFirstFrameOcclusion;
	bAcceptsLights = PrimitiveSceneInfo->bAcceptsLights;
	bHasViewDependentDPG = Proxy->HasViewDependentDPG();
	bShouldCullModulatedShadows = Component->ShouldCullModulatedShadows();
	bCastDynamicShadow = PrimitiveSceneInfo->bCastDynamicShadow;
	bLightEnvironmentForceNonCompositeDynamicLights = PrimitiveSceneInfo->bLightEnvironmentForceNonCompositeDynamicLights;
	bIgnoreNearPlaneIntersection = PrimitiveSceneInfo->bIgnoreNearPlaneIntersection;
	bAlwaysVisible = PrimitiveSceneInfo->bAlwaysVisible;
	
	StaticDepthPriorityGroup = bHasViewDependentDPG ? 0 : Proxy->GetStaticDepthPriorityGroup();
}

FPrimitiveSceneInfo::FPrimitiveSceneInfo(UPrimitiveComponent* InComponent,FPrimitiveSceneProxy* InProxy,FScene* InScene):
	Proxy(InProxy),
	Component(InComponent),
	Owner(InComponent->GetOwner()),
	Id(INDEX_NONE),
	TranslucencySortPriority(InComponent->TranslucencySortPriority),
	bStaticShadowing(InComponent->HasStaticShadowing()),
	bCastDynamicShadow(InComponent->bCastDynamicShadow && InComponent->CastShadow),
	bSelfShadowOnly(InComponent->bSelfShadowOnly),
	bCastStaticShadow(InComponent->CastShadow),
	bCastHiddenShadow(InComponent->bCastHiddenShadow),
	bAcceptsLights(InComponent->bAcceptsLights),
	bAcceptsDynamicLights(InComponent->bAcceptsDynamicLights),
	bSelfContainedLighting((InComponent->GetOutermost()->PackageFlags & PKG_SelfContainedLighting) != 0),
	bUseAsOccluder(InComponent->bUseAsOccluder),
	bAllowApproximateOcclusion(InComponent->bAllowApproximateOcclusion),
	bFirstFrameOcclusion(InComponent->bFirstFrameOcclusion),
	bIgnoreNearPlaneIntersection(InComponent->bIgnoreNearPlaneIntersection),
	bAlwaysVisible(FALSE),
	bSelectable(InComponent->bSelectable),
	bNeedsStaticMeshUpdate(FALSE),
	bCullModulatedShadowOnBackfaces(InComponent->bCullModulatedShadowOnBackfaces),
	bCullModulatedShadowOnEmissive(InComponent->bCullModulatedShadowOnEmissive),
	bAllowAmbientOcclusion(InComponent->bAllowAmbientOcclusion),
	bRenderSHLightInBasePass(FALSE),
	bEnableMotionBlur(InComponent->MotionBlurScale > 0.5f),
	bLightEnvironmentForceNonCompositeDynamicLights(InComponent->LightEnvironment ? InComponent->LightEnvironment->bForceNonCompositeDynamicLights : TRUE),
	Bounds(InComponent->Bounds),
	MaxDrawDistance(InComponent->CachedMaxDrawDistance),
	MinDrawDistance(InComponent->MinDrawDistance),
	LightingChannels(InComponent->LightingChannels),
	LightEnvironment(
		(InComponent->LightEnvironment && InComponent->LightEnvironment->IsEnabled()) ?
			InComponent->LightEnvironment :
			NULL
		),
	LevelName(InComponent->GetOutermost()->GetFName()),
	LightList(NULL),
	UpperSkyLightColor(FLinearColor::Black),
	LowerSkyLightColor(FLinearColor::Black),
	DirectionalLightSceneInfo(NULL),
	SHLightSceneInfo(NULL),
	ShadowParent(InComponent->ShadowParent),
	FogVolumeSceneInfo(NULL),
	LastRenderTime(0.0f),
	LastVisibilityChangeTime(0.0f),
	Scene(InScene)
{
	check(Component);
	check(Proxy);

	InComponent->SceneInfo = this;
	Proxy->PrimitiveSceneInfo = this;

	// if the primitive is shadow parented then use cull mode of its parent
	if( ShadowParent )
	{
		bCullModulatedShadowOnBackfaces = ShadowParent->bCullModulatedShadowOnBackfaces;
		bCullModulatedShadowOnEmissive = ShadowParent->bCullModulatedShadowOnEmissive;
	}

	// Replace 0 MaxDrawDistance with FLT_MAX so we don't have to add any special case handling later.
	if( MaxDrawDistance == 0 )
	{
		MaxDrawDistance = FLT_MAX;
	}

	// Only create hit proxies in the Editor as that's where they are used.
	HHitProxy* DefaultHitProxy = NULL;
	if(GIsEditor)
	{
		// Create a dynamic hit proxy for the primitive. 
		DefaultHitProxy = Proxy->CreateHitProxies(Component,HitProxies);
		if( DefaultHitProxy )
		{
			DefaultDynamicHitProxyId = DefaultHitProxy->Id;
		}
	}

	// Cache the primitive's static mesh elements.
	FBatchingSPDI BatchingSPDI(this);
	BatchingSPDI.SetHitProxy(DefaultHitProxy);
	Proxy->DrawStaticElements(&BatchingSPDI);
	StaticMeshes.Shrink();
}

FPrimitiveSceneInfo::~FPrimitiveSceneInfo()
{
	check(!OctreeId.IsValidId());
}

void FPrimitiveSceneInfo::AddToScene()
{
	check(IsInRenderingThread());

	for(INT MeshIndex = 0;MeshIndex < StaticMeshes.Num();MeshIndex++)
	{
		// Add the static mesh to the scene's static mesh list.
		FSparseArrayAllocationInfo SceneArrayAllocation = Scene->StaticMeshes.Add();
		Scene->StaticMeshes(SceneArrayAllocation.Index) = &StaticMeshes(MeshIndex);
		StaticMeshes(MeshIndex).Id = SceneArrayAllocation.Index;

		// Add the static mesh to the appropriate draw lists.
		StaticMeshes(MeshIndex).AddToDrawLists(Scene);
	}

	// Add the primitive to the octree.
	FPrimitiveSceneInfoCompact CompactPrimitiveSceneInfo(this);

	check(!OctreeId.IsValidId());
	Scene->PrimitiveOctree.AddElement(CompactPrimitiveSceneInfo);
	check(OctreeId.IsValidId());

	if(bAcceptsLights)
	{
		if(LightEnvironment)
		{
			// For primitives in a light environment, attach it to lights which are in the same light environment.
			FLightEnvironmentSceneInfo& LightEnvironmentSceneInfo = Scene->GetLightEnvironmentSceneInfo(LightEnvironment);
			for(INT LightIndex = 0;LightIndex < LightEnvironmentSceneInfo.Lights.Num();LightIndex++)
			{
				FLightSceneInfo* LightSceneInfo = LightEnvironmentSceneInfo.Lights(LightIndex);
				if(FLightSceneInfoCompact(LightSceneInfo).AffectsPrimitive(CompactPrimitiveSceneInfo))
				{
					FLightPrimitiveInteraction::Create(LightSceneInfo,this);
				}
			}
		}

		// Find lights that affect the primitive in the light octree.		
		{
			FMemMark MemStackMark(GRenderingThreadMemStack);

			for(FSceneLightOctree::TConstElementBoxIterator<SceneRenderingAllocator> LightIt(Scene->LightOctree,Bounds.GetBox());
				LightIt.HasPendingElements();
				LightIt.Advance())
			{
				const FLightSceneInfoCompact& LightSceneInfoCompact = LightIt.GetCurrentElement();

				if(LightSceneInfoCompact.AffectsPrimitive(CompactPrimitiveSceneInfo))
				{
					FLightPrimitiveInteraction::Create(LightSceneInfoCompact.LightSceneInfo,this);
				}
			}
		}
	}
}

void FPrimitiveSceneInfo::RemoveFromScene()
{
	check(IsInRenderingThread());

	// implicit linked list. The destruction will update this "head" pointer to the next item in the list.
	while(LightList)
	{
		FLightPrimitiveInteraction::Destroy(LightList);
	}
	
	// Remove the primitive from the octree.
	check(OctreeId.IsValidId());
	check(Scene->PrimitiveOctree.GetElementById(OctreeId).PrimitiveSceneInfo == this);
	Scene->PrimitiveOctree.RemoveElement(OctreeId);
	OctreeId = FOctreeElementId();

	// Remove static meshes from the scene.
	StaticMeshes.Empty();
}

void FPrimitiveSceneInfo::ConditionalUpdateStaticMeshes()
{
	if(bNeedsStaticMeshUpdate)
	{
		bNeedsStaticMeshUpdate = FALSE;

		// Remove the primitive's static meshes from the draw lists they're currently in, and readd them to the appropriate draw lists.
		for(INT MeshIndex = 0;MeshIndex < StaticMeshes.Num();MeshIndex++)
		{
			StaticMeshes(MeshIndex).RemoveFromDrawLists();
			StaticMeshes(MeshIndex).AddToDrawLists(Scene);
		}
	}
}

void FPrimitiveSceneInfo::BeginDeferredUpdateStaticMeshes()
{
	// Set a flag which causes InitViews to update the static meshes the next time the primitive is visible.
	bNeedsStaticMeshUpdate = TRUE;
}

void FPrimitiveSceneInfo::LinkShadowParent()
{
	// Add the primitive to its shadow group.
	if(ShadowParent)
	{
		FShadowGroupSceneInfo* ShadowGroup = Scene->ShadowGroups.Find(ShadowParent);
		if(!ShadowGroup)
		{
			// If this is the first primitive attached that uses this shadow parent, create a new shadow group.
			ShadowGroup = &Scene->ShadowGroups.Set(ShadowParent,FShadowGroupSceneInfo());
		}
		ShadowGroup->Primitives.AddItem(this);
	}
}

void FPrimitiveSceneInfo::UnlinkShadowParent()
{
	// Remove the primitive from its shadow group.
	if(ShadowParent)
	{
		FShadowGroupSceneInfo* ShadowGroup = Scene->ShadowGroups.Find(ShadowParent);
		check(ShadowGroup);
		ShadowGroup->Primitives.RemoveItemSwap(this);

		if(!ShadowGroup->Primitives.Num())
		{
			// If this was the last primitive attached that uses this shadow parent, free the shadow group.
			Scene->ShadowGroups.Remove(ShadowParent);
		}
	}
}

void FPrimitiveSceneInfo::FinishCleanup()
{
	delete this;
}

UINT FPrimitiveSceneInfo::GetMemoryFootprint()
{
	return( sizeof( *this ) + HitProxies.GetAllocatedSize() + StaticMeshes.GetAllocatedSize() );
}
