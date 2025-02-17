/*=============================================================================
	LightSceneInfo.cpp: Light scene info implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

void FLightSceneInfoCompact::Init(FLightSceneInfo* InLightSceneInfo)
{
	LightSceneInfo = InLightSceneInfo;
	LightEnvironment = InLightSceneInfo->LightEnvironment;
	LightingChannels = InLightSceneInfo->LightingChannels;
	FSphere BoundingSphere(
		InLightSceneInfo->GetOrigin(),
		InLightSceneInfo->GetRadius() > 0.0f ?
			InLightSceneInfo->GetRadius() :
			FLT_MAX
		);
	appMemcpy(&BoundingSphereVector,&BoundingSphere,sizeof(BoundingSphereVector));
	Color = InLightSceneInfo->Color;

	bStaticShadowing = InLightSceneInfo->bStaticShadowing;
	bCastDynamicShadow = InLightSceneInfo->bCastDynamicShadow;
	bCastStaticShadow = InLightSceneInfo->bCastStaticShadow;
	bProjectedShadows = InLightSceneInfo->bProjectedShadows;
	bStaticLighting = InLightSceneInfo->bStaticLighting;
	bCastCompositeShadow = InLightSceneInfo->bCastCompositeShadow;
	bModulateBetterShadows = InLightSceneInfo->LightShadowMode == LightShadow_ModulateBetter;
}

FLightSceneInfo::FLightSceneInfo(const ULightComponent* Component)
	: LightComponent(Component)
	, LightGuid(Component->LightGuid)
	, LightmapGuid(Component->LightmapGuid)
	, WorldToLight(Component->WorldToLight)
	, LightToWorld(Component->LightToWorld)
	, Position(Component->GetPosition())
	, Color(FLinearColor(Component->LightColor) * Component->Brightness)
	, LightingChannels(Component->LightingChannels)
	, StaticPrimitiveList(NULL)
	, DynamicPrimitiveList(NULL)
	, Id(INDEX_NONE)
	, bProjectedShadows(Component->HasProjectedShadowing())
	, bStaticLighting(Component->HasStaticLighting())
	, bStaticShadowing(Component->HasStaticShadowing())
	, bCastDynamicShadow(Component->CastShadows && Component->CastDynamicShadows)
	, bCastCompositeShadow(Component->bCastCompositeShadow)
	, bCastStaticShadow(Component->CastShadows && Component->CastStaticShadows)
	, bOnlyAffectSameAndSpecifiedLevels(Component->bOnlyAffectSameAndSpecifiedLevels)
	, bUseVolumes(Component->bUseVolumes)
	, LightEnvironment(Component->LightEnvironment)
	, LightType(Component->GetLightType())
	, LightShadowMode(Component->LightShadowMode)
	, ShadowProjectionTechnique(Component->ShadowProjectionTechnique)
	, ShadowFilterQuality(Component->ShadowFilterQuality)
	, MinShadowResolution(Component->MinShadowResolution)
	, MaxShadowResolution(Component->MaxShadowResolution)
	, ShadowFadeResolution(Component->ShadowFadeResolution)
	, LevelName(Component->GetOutermost()->GetFName())
	, OtherLevelsToAffect(Component->OtherLevelsToAffect)
	, ExclusionConvexVolumes(Component->ExclusionConvexVolumes)
	, InclusionConvexVolumes(Component->InclusionConvexVolumes)
	, ModShadowColor(Component->ModShadowColor)
	, ModShadowFadeoutTime(Component->ModShadowFadeoutTime)
	, ModShadowFadeoutExponent(Component->ModShadowFadeoutExponent)
	, LightComponentName(Component->GetOwner() ? Component->GetOwner()->GetFName() : Component->GetFName())
	, Scene((FScene*)Component->GetScene())
{
	if ( Component->Function && Component->Function->SourceMaterial && Component->Function->SourceMaterial->GetMaterial()->bUsedAsLightFunction )
	{
		LightFunctionScale = Component->Function->Scale;
		LightFunction = Component->Function->SourceMaterial->GetRenderProxy(FALSE);
	}
	else
	{
		LightFunction = NULL;
	}
}

void FLightSceneInfo::AddToScene()
{
	const FLightSceneInfoCompact& LightSceneInfoCompact = Scene->Lights(Id);

    if(LightEnvironment)
    {
		FLightEnvironmentSceneInfo& LightEnvironmentSceneInfo = Scene->GetLightEnvironmentSceneInfo(LightEnvironment);

		// For a light in a light environment, attach it to primitives which are in the same light environment.
		for(INT PrimitiveIndex = 0;PrimitiveIndex < LightEnvironmentSceneInfo.Primitives.Num();PrimitiveIndex++)
		{
			FPrimitiveSceneInfo* PrimitiveSceneInfo = LightEnvironmentSceneInfo.Primitives(PrimitiveIndex);
			if(LightSceneInfoCompact.AffectsPrimitive(FPrimitiveSceneInfoCompact(PrimitiveSceneInfo)))
			{
				// create light interaction and add to light/primitive lists
				FLightPrimitiveInteraction::Create(this,PrimitiveSceneInfo);
			}
		}

	    // Add the light to the light environment's light list.
	    LightEnvironmentSceneInfo.Lights.AddItem(this);
    }
	else
	{
		// Add the light to the scene's light octree.
		Scene->LightOctree.AddElement(LightSceneInfoCompact);

		// Find primitives that the light affects in the primitive octree.
		FMemMark MemStackMark(GRenderingThreadMemStack);
		for(FScenePrimitiveOctree::TConstElementBoxIterator<SceneRenderingAllocator> PrimitiveIt(
				Scene->PrimitiveOctree,
				GetBoundingBox()
				);
			PrimitiveIt.HasPendingElements();
			PrimitiveIt.Advance())
		{
			const FPrimitiveSceneInfoCompact& PrimitiveSceneInfoCompact = PrimitiveIt.GetCurrentElement();
			if(	LightSceneInfoCompact.AffectsPrimitive(PrimitiveSceneInfoCompact))
			{
				// create light interaction and add to light/primitive lists
				FLightPrimitiveInteraction::Create(this,PrimitiveSceneInfoCompact.PrimitiveSceneInfo);
			}
		}
	}
}

void FLightSceneInfo::RemoveFromScene()
{
	if(LightEnvironment)
	{
		FLightEnvironmentSceneInfo& LightEnvironmentSceneInfo = Scene->GetLightEnvironmentSceneInfo(LightEnvironment);

		// Remove the light from its light environment's light list.
		LightEnvironmentSceneInfo.Lights.RemoveItem(this);

		// If the light environment scene info is now empty, free it.
		if(!LightEnvironmentSceneInfo.Lights.Num() && !LightEnvironmentSceneInfo.Primitives.Num())
		{
			Scene->LightEnvironments.Remove(LightEnvironment);
		}
	}
	else
	{
		// Remove the light from the octree.
		Scene->LightOctree.RemoveElement(OctreeId);
	}

	// Detach the light from the primitives it affects.
	Detach();
}

void FLightSceneInfo::Detach()
{
	check(IsInRenderingThread());

	// implicit linked list. The destruction will update this "head" pointer to the next item in the list.
	while(StaticPrimitiveList)
	{
		FLightPrimitiveInteraction::Destroy(StaticPrimitiveList);
	}
	// implicit linked list. The destruction will update this "head" pointer to the next item in the list.
	while(DynamicPrimitiveList)
	{
		FLightPrimitiveInteraction::Destroy(DynamicPrimitiveList);
	}

	// Clear the light's static mesh draw lists.
	for(INT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
	{
		GetDPGInfo(DPGIndex)->DetachStaticMeshes();
	}
}

/** Determines whether two bounding spheres intersect. */
FORCEINLINE UBOOL AreSpheresNotIntersecting(
	const VectorRegister& A_XYZ,
	const VectorRegister& A_Radius,
	const VectorRegister& B_XYZ,
	const VectorRegister& B_Radius
	)
{
	const VectorRegister DeltaVector = VectorSubtract(A_XYZ,B_XYZ);
	const VectorRegister DistanceSquared = VectorDot3(DeltaVector,DeltaVector);
	const VectorRegister MaxDistance = VectorAdd(A_Radius,B_Radius);
	const VectorRegister MaxDistanceSquared = VectorMultiply(MaxDistance,MaxDistance);
	return VectoryAnyGreaterThan(DistanceSquared,MaxDistanceSquared);
}

/**
* Tests whether this light's modulated shadow affects the given primitive by doing a bounds check.
*
* @param CompactPrimitiveSceneInfo - The primitive to test.
* @return True if the modulated shadow affects the primitive.
*/
UBOOL FLightSceneInfoCompact::AffectsModShadowPrimitive(const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo) const
{
	// Check if the light's bounds intersect the primitive's bounds.
	if(AreSpheresNotIntersecting(
		BoundingSphereVector,
		VectorReplicate(BoundingSphereVector,3),
		VectorLoadFloat3(&CompactPrimitiveSceneInfo.Bounds.Origin),
		VectorLoadFloat1(&CompactPrimitiveSceneInfo.Bounds.SphereRadius)
		))
	{
		return FALSE;
	}

	if(!LightSceneInfo->AffectsBounds(CompactPrimitiveSceneInfo.Bounds))
	{
		return FALSE;
	}

	return TRUE;
}

/**
* Tests whether this light affects the given primitive.  This checks both the primitive and light settings for light relevance
* and also calls AffectsBounds.
*
* @param CompactPrimitiveSceneInfo - The primitive to test.
* @return True if the light affects the primitive.
*/
UBOOL FLightSceneInfoCompact::AffectsPrimitive(const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo) const
{
	// Check if the light's bounds intersect the primitive's bounds.
	if(AreSpheresNotIntersecting(
		BoundingSphereVector,
		VectorReplicate(BoundingSphereVector,3),
		VectorLoadFloat3(&CompactPrimitiveSceneInfo.Bounds.Origin),
		VectorLoadFloat1(&CompactPrimitiveSceneInfo.Bounds.SphereRadius)
		))
	{
		return FALSE;
	}

	if(!CompactPrimitiveSceneInfo.bAcceptsLights)
	{
		return FALSE;
	}

	const FPrimitiveSceneInfo* PrimitiveSceneInfo = CompactPrimitiveSceneInfo.PrimitiveSceneInfo;
	PREFETCH(PrimitiveSceneInfo);
	PREFETCH(LightSceneInfo);

	// Dynamic lights that affect the default light environment will also affect primitives in other light environments,
	// unless they're being composited into the light environments.
	const UBOOL bUseCompositeDynamicLights =
		!CompactPrimitiveSceneInfo.bLightEnvironmentForceNonCompositeDynamicLights &&
		GSystemSettings.bUseCompositeDynamicLights;
	const ULightEnvironmentComponent* PrimitiveLightEnvironment = CompactPrimitiveSceneInfo.LightEnvironment;
	if(!LightEnvironment && !bStaticLighting && !bUseCompositeDynamicLights)
	{
		PrimitiveLightEnvironment = NULL;
	}

	if( PrimitiveLightEnvironment != LightEnvironment )
	{
		return FALSE;
	}

	if( !LightingChannels.OverlapsWith( CompactPrimitiveSceneInfo.LightingChannels ) )
	{
		return FALSE;
	}

	// Cull based on information in the full scene infos.

	if(!LightSceneInfo->AffectsBounds(CompactPrimitiveSceneInfo.Bounds))
	{
		return FALSE;
	}

	if( !PrimitiveSceneInfo->bAcceptsDynamicLights && !bStaticShadowing )
	{
		return FALSE; 
	}

	// Check whether the package the primitive resides in is set to have self contained lighting.
	if(PrimitiveSceneInfo->bSelfContainedLighting && bStaticShadowing)
	{
		// Only allow interaction in the case of self contained lighting if both light and primitive are in the 
		// the same package/ level, aka share an outermost.
		if(LightSceneInfo->LevelName != PrimitiveSceneInfo->LevelName)
		{
			return FALSE;
		}
	}

	if( LightSceneInfo->bOnlyAffectSameAndSpecifiedLevels )
	{
		// Retrieve the FName of the light's and primtive's level.
		FName LightLevelName			= LightSceneInfo->LevelName;
		FName PrimitiveLevelName		= PrimitiveSceneInfo->LevelName;
		UBOOL bShouldAffectPrimitive	= FALSE;

		// Check whether the light's level matches the primitives.
		if( LightLevelName == PrimitiveLevelName )
		{
			bShouldAffectPrimitive = TRUE;
		}
		// Check whether the primitve's level is in the OtherLevelsToAffect array.
		else
		{
			// Iterate over all levels and look for match.
			for( INT LevelIndex=0; LevelIndex<LightSceneInfo->OtherLevelsToAffect.Num(); LevelIndex++ )
			{
				const FName& OtherLevelName = LightSceneInfo->OtherLevelsToAffect(LevelIndex);
				if( OtherLevelName == PrimitiveLevelName )
				{
					// Match found, early out.
					bShouldAffectPrimitive = TRUE;
					break;
				}
			}
		}

		// Light should not affect primitive, early out.
		if( !bShouldAffectPrimitive )
		{
			return FALSE;
		}
	}

	// Use inclusion/ exclusion volumes to determine whether primitive is affected by light.
	if( LightSceneInfo->bUseVolumes )
	{
		// Exclusion volumes have higher priority. Return FALSE if primitive intersects or is
		// encompassed by any exclusion volume.
		for( INT VolumeIndex=0; VolumeIndex<LightSceneInfo->ExclusionConvexVolumes.Num(); VolumeIndex++ )
		{
			const FConvexVolume& ExclusionConvexVolume	= LightSceneInfo->ExclusionConvexVolumes(VolumeIndex);
			if( ExclusionConvexVolume.IntersectBox( PrimitiveSceneInfo->Bounds.Origin, PrimitiveSceneInfo->Bounds.BoxExtent ) )
			{
				return FALSE;	
			}
		}

		// Check whether primitive intersects or is encompassed by any inclusion volume.
		UBOOL bIntersectsInclusionVolume = FALSE;
		for( INT VolumeIndex=0; VolumeIndex<LightSceneInfo->InclusionConvexVolumes.Num(); VolumeIndex++ )
		{
			const FConvexVolume& InclusionConvexVolume	= LightSceneInfo->InclusionConvexVolumes(VolumeIndex);
			if( InclusionConvexVolume.IntersectBox( PrimitiveSceneInfo->Bounds.Origin, PrimitiveSceneInfo->Bounds.BoxExtent ) )
			{
				bIntersectsInclusionVolume = TRUE;
				break;
			}
		}
		// No interaction unless there is intersection or there are no inclusion volumes, in which case
		// we assume that the whole world is included unless explicitly excluded.
		if( !bIntersectsInclusionVolume && LightSceneInfo->InclusionConvexVolumes.Num() )
		{
			return FALSE;
		}
	}

	return TRUE;
}
