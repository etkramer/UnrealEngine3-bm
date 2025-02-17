/*=============================================================================
	StaticLightingVertexMapping.cpp: Static lighting vertex mapping implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "StaticLightingPrivate.h"

// Don't compile the static lighting system on consoles.
#if !CONSOLE

/** The maximum number of shadow samples per triangle. */
#define MAX_SHADOW_SAMPLES_PER_TRIANGLE	32

class FStaticLightingVertexMappingProcessor
{
public:

	/** The shadow-maps for the vertex mapping. */
	TMap<ULightComponent*,FShadowMapData1D*> ShadowMapData;

	/** The light-map for the vertex mapping. */
	FLightMapData1D* LightMapData;

	/** Initialization constructor. */
	FStaticLightingVertexMappingProcessor(FStaticLightingVertexMapping* InVertexMapping,FStaticLightingSystem* InSystem)
	:	VertexMapping(InVertexMapping)
	,	Mesh(InVertexMapping->Mesh)
	,	System(InSystem)
	,	CoherentRayCache(InVertexMapping->Mesh)
	{}

	/** Processses the vertex mapping. */
	void Process();

private:

	struct FAdjacentVertexInfo
	{
		INT VertexIndex;
		FLOAT Weight;
	};

	/** A sample for static vertex lighting. */
	struct FVertexLightingSample
	{
		FStaticLightingVertex SampleVertex;
		TBitArray<> RelevantLightMask;
	};

	FStaticLightingVertexMapping* const VertexMapping;
	const FStaticLightingMesh* const Mesh;
	FStaticLightingSystem* const System;
	FCoherentRayCache CoherentRayCache;

	TArray<FVertexLightingSample> Samples;
	TMultiMap<INT,FAdjacentVertexInfo> SampleToAdjacentVertexMap;

	TArray<FStaticLightingVertex> Vertices;
	TMultiMap<FVector,INT> VertexPositionMap;
	TArray<FLOAT> VertexSampleWeightTotals;

	TArray<FShadowMapData1D*> ShadowMapDataByLightIndex;

	/**
	 * Caches the vertices of a mesh.
	 * @param Mesh - The mesh to cache vertices from.
	 * @param OutVertices - Upon return, contains the meshes vertices.
	 */
	void CacheVertices();

	/** Creates a list of samples for the mapping. */
	void CreateSamples();

	/** Calculates area lighting for the vertex lighting samples. */
	void CalculateAreaLighting();

	/** Calculates direct lighting for the vertex lighting samples. */
	void CalculateDirectLighting();
};

void FStaticLightingVertexMappingProcessor::Process()
{
	const FStaticLightingMesh* const Mesh = VertexMapping->Mesh;

	// Cache the mesh's vertex data, and build a map from position to indices of vertices at that position.
	CacheVertices();

	// Allocate shadow-map data.
	ShadowMapDataByLightIndex.Empty(Mesh->RelevantLights.Num());
	ShadowMapDataByLightIndex.AddZeroed(Mesh->RelevantLights.Num());

	// Allocate light-map data.
	LightMapData = new FLightMapData1D(Mesh->NumVertices);

	// Create the samples for the mesh.
	CreateSamples();

	// Calculate area lighting.
	CalculateAreaLighting();

	// Calculate direct lighting.
	CalculateDirectLighting();
}

void FStaticLightingVertexMappingProcessor::CacheVertices()
{
	Vertices.Empty(Mesh->NumVertices);
	Vertices.AddZeroed(Mesh->NumVertices);

	for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumTriangles;TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		FStaticLightingVertex V0;
		FStaticLightingVertex V1;
		FStaticLightingVertex V2;
		Mesh->GetTriangle(TriangleIndex,V0,V1,V2);
		INT I0 = 0;
		INT I1 = 0;
		INT I2 = 0;
		Mesh->GetTriangleIndices(TriangleIndex,I0,I1,I2);

		// Cache the vertices by vertex index.
		Vertices(I0) = V0;
		Vertices(I1) = V1;
		Vertices(I2) = V2;

		// Also map the vertices by position.
		VertexPositionMap.AddUnique(V0.WorldPosition,I0);
		VertexPositionMap.AddUnique(V1.WorldPosition,I1);
		VertexPositionMap.AddUnique(V2.WorldPosition,I2);
	}
}

void FStaticLightingVertexMappingProcessor::CreateSamples()
{
	// Initialize the directly sampled vertex map.
	TBitArray<> DirectlySampledVertexMap;
	DirectlySampledVertexMap.Init(TRUE,Mesh->NumVertices);
	
	// Allocate the vertex sample weight total map.
	VertexSampleWeightTotals.Empty(Mesh->NumVertices);
	VertexSampleWeightTotals.AddZeroed(Mesh->NumVertices);

	// Generate random samples on the faces of larger triangles.
	if(!VertexMapping->bSampleVertices)
	{
		// Setup a thread-safe random stream with a fixed seed, so the sample points are deterministic.
		FRandomStream RandomStream(0);

		// Calculate light visibility for each triangle.
		for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumTriangles;TriangleIndex++)
		{
			// Query the mesh for the triangle's vertex indices.
			INT TriangleVertexIndices[3];
			Mesh->GetTriangleIndices(
				TriangleIndex,
				TriangleVertexIndices[0],
				TriangleVertexIndices[1],
				TriangleVertexIndices[2]
				);

			// Lookup the triangle's vertices.
			FStaticLightingVertex TriangleVertices[3];
			for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
			{
				TriangleVertices[VertexIndex] = Vertices(TriangleVertexIndices[VertexIndex]);
			}

			// Compute the triangle's normal.
			const FVector TriangleNormal = (TriangleVertices[2].WorldPosition - TriangleVertices[0].WorldPosition) ^ (TriangleVertices[1].WorldPosition - TriangleVertices[0].WorldPosition);

			// Find the lights which are in front of this triangle.
			const TBitArray<> TriangleRelevantLightMask = CullBackfacingLights(
				Mesh->bTwoSidedMaterial,
				TriangleVertices[0].WorldPosition,
				TriangleNormal,
				Mesh->RelevantLights
				);

			// Compute the triangle's area.
			const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();

			// Compute the number of samples to use for the triangle, proportional to the triangle area.
			const INT NumSamples = Clamp(appTrunc(TriangleArea * VertexMapping->SampleToAreaRatio),0,MAX_SHADOW_SAMPLES_PER_TRIANGLE);
			if(NumSamples)
			{
				// Look up the vertices adjacent to this triangle at each vertex.
				TArray<INT,TInlineAllocator<8> > AdjacentVertexIndices[3];
				for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
				{
					for(TMultiMap<FVector,INT>::TConstKeyIterator VertexPositionMapIt(VertexPositionMap,TriangleVertices[VertexIndex].WorldPosition);
						VertexPositionMapIt;
						++VertexPositionMapIt)
					{
						AdjacentVertexIndices[VertexIndex].AddItem(VertexPositionMapIt.Value());
					}
				}

				// Weight the triangle's samples proportional to the triangle size, but independently of the number of samples.
				const FLOAT TriangleWeight = TriangleArea / NumSamples;

				// Sample the triangle's lighting.
				for(INT TriangleSampleIndex = 0;TriangleSampleIndex < NumSamples;TriangleSampleIndex++)
				{
					// Choose a uniformly distributed random point on the triangle.
					const FLOAT S = 1.0f - appSqrt(RandomStream.GetFraction());
					const FLOAT T = RandomStream.GetFraction() * (1.0f - S);
					const FLOAT U = 1 - S - T;

					// Index the sample's vertex indices and weights.
					const FLOAT TriangleVertexWeights[3] =
					{
						S * TriangleWeight,
						T * TriangleWeight,
						U * TriangleWeight
					};

					// Interpolate the triangle's vertex attributes at the sample point.
					const FStaticLightingVertex SampleVertex =
						TriangleVertices[0] * S +
						TriangleVertices[1] * T +
						TriangleVertices[2] * U;

					// Create the sample.
					const INT SampleIndex = Samples.Num();
					FVertexLightingSample* Sample = new(Samples) FVertexLightingSample;
					Sample->SampleVertex = SampleVertex;
					Sample->RelevantLightMask = TriangleRelevantLightMask;

					// Build a list of vertices whose light-map values will affect this sample point.
					for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
					{
						for(INT AdjacentVertexIndex = 0;AdjacentVertexIndex < AdjacentVertexIndices[VertexIndex].Num();AdjacentVertexIndex++)
						{
							// Copy the adjacent vertex with its weight into the sample.
							FAdjacentVertexInfo AdjacentVertexInfo;
							AdjacentVertexInfo.VertexIndex = AdjacentVertexIndices[VertexIndex](AdjacentVertexIndex);
							AdjacentVertexInfo.Weight = TriangleVertexWeights[VertexIndex];
							SampleToAdjacentVertexMap.Add(SampleIndex,AdjacentVertexInfo);

							// Accumulate the vertex's sum of light-map sample weights.
							VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex) += TriangleVertexWeights[VertexIndex];

							// Indicate that the vertex doesn't need a direct sample.
							DirectlySampledVertexMap(AdjacentVertexInfo.VertexIndex) = FALSE;
						}
					}
				}
			}
		}
	}

	// Generate samples for vertices of small triangles.
	for(TConstSetBitIterator<> VertexIt(DirectlySampledVertexMap);VertexIt;++VertexIt)
	{
		const INT VertexIndex = VertexIt.GetIndex();
		const FStaticLightingVertex& SampleVertex = Vertices(VertexIndex);

		// Create the sample.
		const INT SampleIndex = Samples.Num();
		FVertexLightingSample* Sample = new(Samples) FVertexLightingSample;
		Sample->SampleVertex = SampleVertex;
		Sample->RelevantLightMask = CullBackfacingLights(
			Mesh->bTwoSidedMaterial,
			SampleVertex.WorldPosition,
			SampleVertex.WorldTangentZ,
			Mesh->RelevantLights
			);
		FAdjacentVertexInfo AdjacentVertexInfo;
		AdjacentVertexInfo.VertexIndex = VertexIndex;
		AdjacentVertexInfo.Weight = 1.0f;
		SampleToAdjacentVertexMap.Add(SampleIndex,AdjacentVertexInfo);

		// Set the vertex sample weight.
		VertexSampleWeightTotals(VertexIndex) = 1.0f;
	}
}

void FStaticLightingVertexMappingProcessor::CalculateAreaLighting()
{
	// Add the sky lights to the light-map's light list.
	UBOOL bHasAreaLights = FALSE;
	UBOOL bHasShadowedAreaLights = FALSE;
	for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
	{
		ULightComponent* Light = Mesh->RelevantLights(LightIndex);
		if(Light->IsA(USkyLightComponent::StaticClass()))
		{
			LightMapData->Lights.AddUniqueItem(Light);
			bHasAreaLights = TRUE;
			if(Light->CastShadows && Light->CastStaticShadows)
			{
				bHasShadowedAreaLights = TRUE;
			}
		}
	}

	// Populate the area lighting cache for the mesh.
	if(bHasShadowedAreaLights)
	{
		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			System->CalculatePointAreaLighting(VertexMapping,Sample.SampleVertex,CoherentRayCache);
		}
	}

	// Map the fully populated area lighting cache onto the mesh.
	if(bHasAreaLights)
	{
		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			const FLightSample AreaLightingSample = System->CalculatePointAreaLighting(VertexMapping,Sample.SampleVertex,CoherentRayCache);
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);
				(*LightMapData)(AdjacentVertexInfo.VertexIndex).AddWeighted(AreaLightingSample,NormalizedWeight);
			}
		}
	}
}

void FStaticLightingVertexMappingProcessor::CalculateDirectLighting()
{
	// Calculate direct lighting at the generated sample points.
	for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
	{
		const FVertexLightingSample& Sample = Samples(SampleIndex);
		const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

		// Add the sample's contribution to the vertex'x shadow-map values.
		for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
		{
			if(Sample.RelevantLightMask(LightIndex))
			{
				ULightComponent* Light = Mesh->RelevantLights(LightIndex);

				// Skip sky lights, since their static lighting is computed separately.
				if(Light->IsA(USkyLightComponent::StaticClass()))
				{
					continue;
				}

				// Compute the shadowing of this sample point from the light.
				const UBOOL bIsShadowed = System->CalculatePointShadowing(
					VertexMapping,
					SampleVertex.WorldPosition,
					Light,
					CoherentRayCache
					);
				if(!bIsShadowed)
				{
					// Accumulate the sample lighting and shadowing at the adjacent vertices.
					for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
						AdjacentVertexIt;
						++AdjacentVertexIt
						)
					{
						const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
						const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
						const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);

						// Determine whether to use a shadow-map or the light-map for this light.
						const UBOOL bUseStaticLighting = Light->UseStaticLighting(VertexMapping->bForceDirectLightMap);
						if(bUseStaticLighting)
						{
							// Use the adjacent vertex's tangent basis to calculate this sample's contribution to its light-map value.
							FStaticLightingVertex AdjacentSampleVertex = SampleVertex;
							AdjacentSampleVertex.WorldTangentX = AdjacentVertex.WorldTangentX;
							AdjacentSampleVertex.WorldTangentY = AdjacentVertex.WorldTangentY;
							AdjacentSampleVertex.WorldTangentZ = AdjacentVertex.WorldTangentZ;

							// Calculate the sample's direct lighting from this light.
							const FLightSample DirectLighting = System->CalculatePointLighting(
								VertexMapping,
								AdjacentSampleVertex,
								Light);

							// Add the sampled direct lighting to the vertex's light-map value.
							(*LightMapData)(AdjacentVertexInfo.VertexIndex).AddWeighted(DirectLighting,NormalizedWeight);

							// Add the light to the light-map's light list.
							LightMapData->Lights.AddUniqueItem(Light);
						}
						else
						{
							// Lookup the shadow-map used by this light.
							FShadowMapData1D* CurrentLightShadowMapData = ShadowMapDataByLightIndex(LightIndex);
							if(!CurrentLightShadowMapData)
							{
								// If this the first sample unshadowed from this light, create a shadow-map for it.
								CurrentLightShadowMapData = new FShadowMapData1D(Mesh->NumVertices);
								ShadowMapDataByLightIndex(LightIndex) = CurrentLightShadowMapData;
								ShadowMapData.Set(Light,CurrentLightShadowMapData);
							}
								
							// Accumulate the sample shadowing.
							(*CurrentLightShadowMapData)(AdjacentVertexInfo.VertexIndex) += NormalizedWeight;
						}
					}
				}
			}
		}
	}
}

void FStaticLightingSystem::ProcessVertexMapping(FStaticLightingVertexMapping* VertexMapping)
{
	// Process the vertex mapping.
	FStaticLightingVertexMappingProcessor Processor(VertexMapping,this);
	Processor.Process();

	// Enqueue the static lighting for application in the main thread.
	TList<FVertexMappingStaticLightingData>* StaticLightingLink = new TList<FVertexMappingStaticLightingData>(FVertexMappingStaticLightingData(),NULL);
	StaticLightingLink->Element.Mapping = VertexMapping;
	StaticLightingLink->Element.LightMapData = Processor.LightMapData;
	StaticLightingLink->Element.ShadowMaps = Processor.ShadowMapData;
	CompleteVertexMappingList.AddElement(StaticLightingLink);
}

#endif
