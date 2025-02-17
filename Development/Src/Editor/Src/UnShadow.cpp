/*=============================================================================
	UnShadow.cpp: Bsp light mesh illumination builder code
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "LightingBuildOptions.h"
#include "StaticLightingPrivate.h"
#include "Database.h"

// Don't compile the static lighting system on consoles.
#if !CONSOLE

/** The distance to pull back the shadow visibility traces to avoid the surface shadowing itself. */
#define SHADOW_VISIBILITY_DISTANCE_BIAS	4.0f

/** The number of hardware threads to not use for building static lighting. */
#define NUM_STATIC_LIGHTING_UNUSED_THREADS 0

/** Whether only one thread should be used for building static lighting. */
#define USE_SINGLE_THREADED_STATIC_LIGHTING 0

FStaticLightingSystem::FStaticLightingSystem(const FLightingBuildOptions& InOptions):
	Options(InOptions),
	bBuildCanceled(FALSE)
{
	DOUBLE StartTime = appSeconds();

	// Prepare lights for rebuild.
	for(TObjectIterator<ULightComponent> LightIt;LightIt;++LightIt)
	{
		ULightComponent* const Light = *LightIt;
		const UBOOL bLightIsInWorld = Light->GetOwner() && GWorld->ContainsActor(Light->GetOwner());
		if(bLightIsInWorld && (Light->HasStaticShadowing() || Light->HasStaticLighting()))
		{
			// Make sure the light GUIDs and volumes are up-to-date.
			Light->ValidateLightGUIDs();
			Light->UpdateVolumes();

			// Add the light to the system's list of lights in the world.
			Lights.AddItem(Light);
		}
	}

	// Gather static lighting info from actor components.
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* const Level = GWorld->Levels(LevelIndex);
		const UBOOL bBuildLightingForLevel = Options.ShouldBuildLightingForLevel( Level );

		// Gather static lighting info from BSP.
		const UBOOL bBuildBSPLighting = 
			bBuildLightingForLevel &&
			Options.bBuildBSP &&
			!Options.bOnlyBuildSelectedActors;
		for( INT ComponentIndex=0; ComponentIndex<Level->ModelComponents.Num(); ComponentIndex++ )
		{
			UModelComponent* ModelComponent = Level->ModelComponents(ComponentIndex);
			if( ModelComponent )
			{
				AddPrimitiveStaticLightingInfo(ModelComponent,bBuildBSPLighting);
			}
		}

		// Gather static lighting info from actors.
		for(INT ActorIndex = 0;ActorIndex < Level->Actors.Num();ActorIndex++)
		{
			AActor* Actor = Level->Actors(ActorIndex);
			if(Actor)
			{
				const UBOOL bBuildActorLighting =
					bBuildLightingForLevel &&
					Options.bBuildActors &&
					(!Options.bOnlyBuildSelectedActors || Actor->IsSelected());

				if (bBuildActorLighting)
				{
					Actor->InvalidateLightingCache();
				}

				// Gather static lighting info from each of the actor's components.
				for(INT ComponentIndex = 0;ComponentIndex < Actor->AllComponents.Num();ComponentIndex++)
				{
					UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Actor->AllComponents(ComponentIndex));
					if(Primitive)
					{
						AddPrimitiveStaticLightingInfo(Primitive,bBuildActorLighting);
					}
				}
			}
		}
	}

	// Prepare the aggregate mesh for raytracing.
	AggregateMesh.PrepareForRaytracing();

	// Spawn the static lighting threads.
	const UINT NumStaticLightingThreads = USE_SINGLE_THREADED_STATIC_LIGHTING ? 1 : Max<UINT>(0,GNumHardwareThreads - NUM_STATIC_LIGHTING_UNUSED_THREADS);
	for(UINT ThreadIndex = 1;ThreadIndex < NumStaticLightingThreads;ThreadIndex++)
	{
		FStaticLightingThreadRunnable* ThreadRunnable = new(Threads) FStaticLightingThreadRunnable(this);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TEXT("StaticLightingThread"), 0, 0, 0, TPri_Normal);
	}

	// Begin the static lighting progress bar.
	GWarn->BeginSlowTask(TEXT("Building static lighting"),FALSE);

	// Start the static lighting thread loop on the main thread, too.
	// Once it returns, all static lighting mappings have begun processing.
	ThreadLoop(TRUE);

	// Stop the static lighting threads.
	for(INT ThreadIndex = 0;ThreadIndex < Threads.Num();ThreadIndex++)
	{
		// Wait for the thread to exit.
		Threads(ThreadIndex).Thread->WaitForCompletion();

		// Check that it didn't terminate with an error.
		Threads(ThreadIndex).CheckHealth();

		// Destroy the thread.
		GThreadFactory->Destroy(Threads(ThreadIndex).Thread);
	}
	Threads.Empty();

	// Apply the last of the completed mappings.
	CompleteVertexMappingList.ApplyAndClear();
	CompleteTextureMappingList.ApplyAndClear();

	// End the static lighting progress bar.
	GWarn->EndSlowTask();

	// Flush pending shadow-map and light-map encoding.
	DOUBLE EncodingStartTime = appSeconds();
	UShadowMap2D::FinishEncoding();
	FLightMap2D::FinishEncoding();

	// Ensure all primitives which were marked dirty by the lighting build are updated.
	DOUBLE UpdateComponentsStartTime = appSeconds();
	// First clear all components so that any references to static lighting assets held 
	// by scene proxies will be fully released before any components are reattached.
	GWorld->ClearComponents();
	GWorld->UpdateComponents(FALSE);

	// Clear the world's lighting needs rebuild flag.
	if(!bBuildCanceled)
	{
		GWorld->GetWorldInfo()->SetMapNeedsLightingFullyRebuilt( FALSE );
	}

	// Clean up old shadow-map and light-map data.
	UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

	// Log execution time.
	DOUBLE EndTime = appSeconds();
	warnf( NAME_Log, TEXT("Illumination: %7.2f seconds (%7.2f seconds encoding lightmaps), %i threads"), EndTime - StartTime, UpdateComponentsStartTime - EncodingStartTime, NumStaticLightingThreads );
	GTaskPerfTracker->AddTask( TEXT("Rebuilding lighting"), *GWorld->GetOutermost()->GetName(), EndTime - StartTime );
	GTaskPerfTracker->AddTask( TEXT("Encoding lightmaps"), *GWorld->GetOutermost()->GetName(), UpdateComponentsStartTime - EncodingStartTime );
}

template<typename StaticLightingDataType>
void FStaticLightingSystem::TCompleteStaticLightingList<StaticLightingDataType>::ApplyAndClear()
{
	while(FirstElement)
	{
		// Atomically read the complete list and clear the shared head pointer.
		TList<StaticLightingDataType>* LocalFirstElement;
		do { LocalFirstElement = FirstElement; }
		while(appInterlockedCompareExchangePointer((void**)&FirstElement,NULL,LocalFirstElement) != LocalFirstElement);

		// Traverse the local list.
		TList<StaticLightingDataType>* Element = LocalFirstElement;
		while(Element)
		{
			// Flag the lights in the mapping's light-map as having been used in a light-map.
			if(Element->Element.LightMapData)
			{
				for(INT LightIndex = 0;LightIndex < Element->Element.LightMapData->Lights.Num();LightIndex++)
				{
					Element->Element.LightMapData->Lights(LightIndex)->bHasLightEverBeenBuiltIntoLightMap = TRUE;
				}
			}

			// Apply the mapping's new static lighting.
			Element->Element.Mapping->Apply(Element->Element.LightMapData,Element->Element.ShadowMaps);

			// Delete this link and advance to the next.
			TList<StaticLightingDataType>* NextElement = Element->Next;
			delete Element;
			Element = NextElement;
		};
	};
}

void FStaticLightingSystem::AddPrimitiveStaticLightingInfo(UPrimitiveComponent* Primitive,UBOOL bBuildLightingForComponent)
{
	// Find the lights relevant to the primitive.
	TArray<ULightComponent*> PrimitiveRelevantLights;
	for(INT LightIndex = 0;LightIndex < Lights.Num();LightIndex++)
	{
		ULightComponent* Light = Lights(LightIndex);
		if(Light->AffectsPrimitive(Primitive))
		{
			PrimitiveRelevantLights.AddItem(Light);
		}
	}

	// Query the component for its static lighting info.
	FStaticLightingPrimitiveInfo PrimitiveInfo;
	Primitive->GetStaticLightingInfo(PrimitiveInfo,PrimitiveRelevantLights,Options);

	// Add the component's shadow casting meshes to the system.
	for(INT MeshIndex = 0;MeshIndex < PrimitiveInfo.Meshes.Num();MeshIndex++)
	{
		FStaticLightingMesh* Mesh = PrimitiveInfo.Meshes(MeshIndex);
		Meshes.AddItem(Mesh);
		AggregateMesh.AddMesh(Mesh);
	}

	// If lighting is being built for this component, add its mappings to the system.
	if(bBuildLightingForComponent)
	{
		for(INT MappingIndex = 0;MappingIndex < PrimitiveInfo.Mappings.Num();MappingIndex++)
		{
			Mappings.AddItem(PrimitiveInfo.Mappings(MappingIndex));
		}
	}
}

UBOOL FStaticLightingSystem::CalculatePointShadowing(
	const FStaticLightingMapping* Mapping,
	const FVector& WorldSurfacePoint,
	ULightComponent* Light,
	FCoherentRayCache& CoherentRayCache
	) const
{
	const UBOOL bIsSkyLight = Light->IsA(USkyLightComponent::StaticClass());
	if(bIsSkyLight)
	{
		// Sky lighting is computed in CalculatePointAreaLighting, so we simply treat sky lights as shadowed here.
		return TRUE;
	}
	else
	{
		// Treat points which the light doesn't affect as shadowed to avoid the costly ray check.
		if(!Light->AffectsBounds(FBoxSphereBounds(WorldSurfacePoint,FVector(0,0,0),0)))
		{
			return TRUE;
		}

		// Check for visibility between the point and the light.
		UBOOL bIsShadowed = FALSE;
		if(Light->CastShadows && Light->CastStaticShadows)
		{
			const FVector4 LightPosition = Light->GetPosition();

			// Construct a line segment between the light and the surface point.
			const FVector LightVector = (FVector)LightPosition - WorldSurfacePoint * LightPosition.W;
			const FLightRay LightRay(
				WorldSurfacePoint + LightVector.SafeNormal() * SHADOW_VISIBILITY_DISTANCE_BIAS,
				WorldSurfacePoint + LightVector,
				Mapping,
				Light
				);

			// Check the line segment for intersection with the static lighting meshes.
			bIsShadowed = AggregateMesh.IntersectLightRay(LightRay,FALSE,CoherentRayCache).bIntersects;
		}

		return bIsShadowed;
	}
}

FLightSample FStaticLightingSystem::CalculatePointLighting(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	ULightComponent* Light
	) const
{
	const UBOOL bIsSkyLight = Light->IsA(USkyLightComponent::StaticClass());
	if(!bIsSkyLight)
	{
	    // Calculate the direction from the vertex to the light.
	    const FVector4 LightPosition = Light->GetPosition();
	    const FVector WorldLightVector = (FVector)LightPosition - Vertex.WorldPosition * LightPosition.W;
    
	    // Transform the light vector to tangent space.
	    const FVector TangentLightVector = 
		    FVector(
			    WorldLightVector | Vertex.WorldTangentX,
			    WorldLightVector | Vertex.WorldTangentY,
			    WorldLightVector | Vertex.WorldTangentZ
			    ).SafeNormal();

		// Compute the incident lighting of the light on the vertex.
		const FLinearColor LightIntensity = Light->GetDirectIntensity(Vertex.WorldPosition);

		// Compute the light-map sample for the front-face of the vertex.
		const FLightSample FrontFaceSample = FLightSample::PointLight(LightIntensity,TangentLightVector);

		if(Mapping->Mesh->bTwoSidedMaterial)
		{
			// Compute the light-map sample for the back-face of the vertex.
			const FLightSample BackFaceSample = FLightSample::PointLight(LightIntensity,-TangentLightVector);

			// If the vertex has a two-sided material, add both front-face and back-face samples.
			FLightSample TwoSidedSample;
			TwoSidedSample.AddWeighted(FrontFaceSample,1.0f);
			TwoSidedSample.AddWeighted(BackFaceSample,1.0f);

			return TwoSidedSample;
		}
		else
		{
			return FrontFaceSample;
		}
	}

	return FLightSample();
}

FLightSample FStaticLightingSystem::CalculatePointAreaLighting(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	FCoherentRayCache& CoherentRayCache
	)
{
	FLightSample AreaLighting;

	// Transform the up vector to tangent space.
	const FVector TangentUpVector(
		Vertex.WorldTangentX.Z,
		Vertex.WorldTangentY.Z,
		Vertex.WorldTangentZ.Z
		);

	// Compute the total unshadowed upper and lower hemisphere sky light.
	FLinearColor UpperHemisphereIntensity = FLinearColor::Black;
	FLinearColor LowerHemisphereIntensity = FLinearColor::Black;
	FLinearColor ShadowedUpperHemisphereIntensity = FLinearColor::Black;
	FLinearColor ShadowedLowerHemisphereIntensity = FLinearColor::Black;
	for(INT LightIndex = 0;LightIndex < Mapping->Mesh->RelevantLights.Num();LightIndex++)
	{
		const USkyLightComponent* SkyLight = Cast<USkyLightComponent>(Mapping->Mesh->RelevantLights(LightIndex));
		if(SkyLight)
		{
			if(SkyLight->CastShadows && SkyLight->CastStaticShadows)
			{
				ShadowedUpperHemisphereIntensity += FLinearColor(SkyLight->LightColor) * SkyLight->Brightness;
				ShadowedLowerHemisphereIntensity += FLinearColor(SkyLight->LowerColor) * SkyLight->LowerBrightness;
			}
			else
			{
				UpperHemisphereIntensity += FLinearColor(SkyLight->LightColor) * SkyLight->Brightness;
				LowerHemisphereIntensity += FLinearColor(SkyLight->LowerColor) * SkyLight->LowerBrightness;
			}
		}
	}

	// Determine whether there is shadowed sky light in each hemisphere.
	const UBOOL bHasShadowedUpperSkyLight = (ShadowedUpperHemisphereIntensity != FLinearColor::Black);
	const UBOOL bHasShadowedLowerSkyLight = (ShadowedLowerHemisphereIntensity != FLinearColor::Black);
	if(bHasShadowedUpperSkyLight || bHasShadowedLowerSkyLight)
	{
		// If there are enough relevant samples in the area lighting cache, interpolate them to estimate this vertex's area lighting.
		if(!CoherentRayCache.AreaLightingCache.InterpolateLighting(Vertex,AreaLighting))
		{
			// Create a table of random directions to sample sky shadowing for.
			static const INT NumRandomDirections = 128;
			static UBOOL bGeneratedDirections = FALSE;
			static FVector RandomDirections[NumRandomDirections];
			if(!bGeneratedDirections)
			{
				for(INT DirectionIndex = 0;DirectionIndex < NumRandomDirections;DirectionIndex++)
				{
					RandomDirections[DirectionIndex] = VRand();
				}

				bGeneratedDirections = TRUE;
			}

			// Use monte carlo integration to compute the shadowed sky lighting.
			FLOAT InverseDistanceSum = 0.0f;
			UINT NumRays = 0;
			for(INT SampleIndex = 0;SampleIndex < NumRandomDirections;SampleIndex++)
			{
				const FVector WorldDirection = RandomDirections[SampleIndex];
				const FVector TangentDirection(
					Vertex.WorldTangentX | WorldDirection,
					Vertex.WorldTangentY | WorldDirection,
					Vertex.WorldTangentZ | WorldDirection
					);

				if(TangentDirection.Z > 0.0f)
				{
					const UBOOL bRayIsInUpperHemisphere = WorldDirection.Z > 0.0f;
					if( (bRayIsInUpperHemisphere && bHasShadowedUpperSkyLight) ||
						(!bRayIsInUpperHemisphere && bHasShadowedLowerSkyLight))
					{
						// Construct a line segment between the light and the surface point.
						const FLightRay LightRay(
							Vertex.WorldPosition + WorldDirection * SHADOW_VISIBILITY_DISTANCE_BIAS,
							Vertex.WorldPosition + WorldDirection * HALF_WORLD_MAX,
							Mapping,
							NULL
							);

						// Check the line segment for intersection with the static lighting meshes.
						const FLightRayIntersection RayIntersection = AggregateMesh.IntersectLightRay(LightRay,TRUE,CoherentRayCache);
						if(!RayIntersection.bIntersects)
						{
							// Add this sample's contribution to the vertex's sky lighting.
							const FLinearColor LightIntensity = bRayIsInUpperHemisphere ? ShadowedUpperHemisphereIntensity : ShadowedLowerHemisphereIntensity;
							AreaLighting.AddWeighted(FLightSample::PointLight(LightIntensity,TangentDirection),2.0f / NumRandomDirections);
						}
						else
						{
							InverseDistanceSum += 1.0f / (Vertex.WorldPosition - RayIntersection.IntersectionVertex.WorldPosition).Size();
						}
						NumRays++;
					}
				}
			}

			// Compute this lighting record's radius based on the harmonic mean of the intersection distance.
			const FLOAT IntersectionDistanceHarmonicMean = (FLOAT)NumRays / InverseDistanceSum;
			const FLOAT RecordRadius = Clamp(IntersectionDistanceHarmonicMean * 0.02f,64.0f,512.0f);

			// Add the vertex's area lighting to the area lighting cache.
			CoherentRayCache.AreaLightingCache.AddRecord(FLightingCache::FRecord(
				Vertex,
				RecordRadius,
				AreaLighting
				));
		}
	}

	// Add the unshadowed sky lighting.
	AreaLighting.AddWeighted(FLightSample::SkyLight(UpperHemisphereIntensity,LowerHemisphereIntensity,TangentUpVector),1.0f);

	return AreaLighting;
}

void FStaticLightingSystem::FStaticLightingThreadRunnable::CheckHealth() const
{
	if(bTerminatedByError)
	{
		GErrorHist[0] = 0;
		GIsCriticalError = FALSE;
		GError->Logf(TEXT("Static lighting thread exception:\r\n%s"),*ErrorMessage);
	}
}

DWORD FStaticLightingSystem::FStaticLightingThreadRunnable::Run()
{
#if _MSC_VER && !XBOX
	extern INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );
	if(!appIsDebuggerPresent())
	{
		__try
		{
			System->ThreadLoop(FALSE);
		}
		__except( CreateMiniDump( GetExceptionInformation() ) )
		{
			ErrorMessage = GErrorHist;

			// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
			// the write to bTerminatedByError.
			appMemoryBarrier();

			bTerminatedByError = FALSE;
		}
	}
	else
#endif
	{
		System->ThreadLoop(FALSE);
	}

	return 0;
}

void FStaticLightingSystem::ThreadLoop(UBOOL bIsMainThread)
{
	UBOOL bIsDone = FALSE;
	while(!bIsDone && !bBuildCanceled)
	{
		// Atomically read and increment the next mapping index to process.
		INT MappingIndex = NextMappingToProcess.Increment() - 1;

		if(MappingIndex < Mappings.Num())
		{
			// If this is the main thread, update progress and apply completed static lighting.
			if(bIsMainThread)
			{
				// Update the progress bar.
				GWarn->StatusUpdatef(MappingIndex,Mappings.Num(),TEXT("Building static lighting"));

				// Apply completed static lighting.
				CompleteVertexMappingList.ApplyAndClear();
				CompleteTextureMappingList.ApplyAndClear();

				// Check the health of all static lighting threads.
				for(INT ThreadIndex = 0;ThreadIndex < Threads.Num();ThreadIndex++)
				{
					Threads(ThreadIndex).CheckHealth();
				}

				// Check the for build cancellation.
				if(GEditor->GetMapBuildCancelled())
				{
					bBuildCanceled = TRUE;
				}
			}

			// Build the mapping's static lighting.
			if(Mappings(MappingIndex)->GetVertexMapping())
			{
				ProcessVertexMapping(Mappings(MappingIndex)->GetVertexMapping());
			}
			else if(Mappings(MappingIndex)->GetTextureMapping())
			{
				ProcessTextureMapping(Mappings(MappingIndex)->GetTextureMapping());
			}
		}
		else
		{
			// Processing has begun for all mappings.
			bIsDone = TRUE;
		}
	}
}

UBOOL IsLightBehindSurface(const FVector& TrianglePoint,const FVector& TriangleNormal,const ULightComponent* Light)
{
	const UBOOL bIsSkyLight = Light->IsA(USkyLightComponent::StaticClass());
	if(!bIsSkyLight)
	{
		// Calculate the direction from the triangle to the light.
		const FVector4 LightPosition = Light->GetPosition();
		const FVector WorldLightVector = (FVector)LightPosition - TrianglePoint * LightPosition.W;

		// Check if the light is in front of the triangle.
		const FLOAT Dot = WorldLightVector | TriangleNormal;
		return Dot < 0.0f;
	}
	else
	{
		// Sky lights are always in front of a surface.
		return FALSE;
	}
}

TBitArray<> CullBackfacingLights(UBOOL bTwoSidedMaterial,const FVector& TrianglePoint,const FVector& TriangleNormal,const TArray<ULightComponent*>& Lights)
{
	if(!bTwoSidedMaterial)
	{
		TBitArray<> Result(FALSE,Lights.Num());
		for(INT LightIndex = 0;LightIndex < Lights.Num();LightIndex++)
		{
			Result(LightIndex) = !IsLightBehindSurface(TrianglePoint,TriangleNormal,Lights(LightIndex));
		}
		return Result;
	}
	else
	{
		return TBitArray<>(TRUE,Lights.Num());
	}
}

#endif

/**
 * Builds lighting information depending on passed in options.
 *
 * @param	Options		Options determining on what and how lighting is built
 */
void UEditorEngine::BuildLighting(const FLightingBuildOptions& Options)
{
#if !CONSOLE
	// Invoke the static lighting system.
	{FStaticLightingSystem System(Options);}
#endif

	// Commit the changes to the world's BSP surfaces.
	GWorld->CommitModelSurfaces();
}
