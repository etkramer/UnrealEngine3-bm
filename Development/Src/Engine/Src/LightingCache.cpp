/*=============================================================================
	LightingCache.cpp: Lighting cache implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "LightingCache.h"

/** The cosine of the angle threshold beyond which to ignore lighting records. */
#define LIGHTING_CACHE_NORMAL_THRESHOLD	0.9f

void FLightingCache::AddRecord(const FRecord& Record)
{
	Octree.AddElement(Record);
}

UBOOL FLightingCache::InterpolateLighting(const FStaticLightingVertex& Vertex,FLightSample& OutLighting) const
{
	FLightSample AccumulatedLighting;
	FLOAT TotalWeight = 0.0f;

	// Iterate over the octree nodes containing the query point.
	for(LightingOctreeType::TConstElementBoxIterator<> OctreeIt(
			Octree,
			FBoxCenterAndExtent(Vertex.WorldPosition,FVector(0,0,0))
			);
		OctreeIt.HasPendingElements();
		OctreeIt.Advance())
	{
		const FRecord& LightingRecord = OctreeIt.GetCurrentElement();

		// Check whether the query point is farther than the record's intersection distance for the direction to the query point.
		const FLOAT DistanceSquared = (LightingRecord.Vertex.WorldPosition - Vertex.WorldPosition).SizeSquared();
		if(DistanceSquared > Square(LightingRecord.Radius))
		{
			continue;
		}

		// Don't use an lighting record which was computed for a significantly different normal.
		const FLOAT NormalDot = (LightingRecord.Vertex.WorldTangentZ | Vertex.WorldTangentZ);
		if(NormalDot < LIGHTING_CACHE_NORMAL_THRESHOLD)
		{
			continue;
		}

		// TODO: Rotate the record's lighting into this vertex's tangent basis.
		if(	(LightingRecord.Vertex.WorldTangentX | Vertex.WorldTangentX) < LIGHTING_CACHE_NORMAL_THRESHOLD ||
			(LightingRecord.Vertex.WorldTangentY | Vertex.WorldTangentY) < LIGHTING_CACHE_NORMAL_THRESHOLD)
		{
			continue;
		}

		// Don't use an lighting record if it's in front of the query point.
		const FLOAT PlaneDistance = (LightingRecord.Vertex.WorldTangentZ + Vertex.WorldTangentZ) | (Vertex.WorldPosition - LightingRecord.Vertex.WorldPosition);
		if(PlaneDistance < -0.05f)
		{
			continue;
		}

		// Compute the lighting record's weight.
		const FLOAT RecordError = appSqrt(DistanceSquared) / (LightingRecord.Radius * NormalDot);

		// Don't use a record if it would have an inconsequential weight.
		if(RecordError >= 1.0f)
		{
			continue;
		}

		// Compute the record's weight, and add it to the accumulated result.
		const FLOAT RecordWeight = Square(1.0f - RecordError);
		AccumulatedLighting.AddWeighted(LightingRecord.Lighting,RecordWeight);
		TotalWeight += RecordWeight;
	}

	if(TotalWeight > DELTA)
	{
		// Normalize the accumulated lighting and return success.
		OutLighting.AddWeighted(AccumulatedLighting,1.0f / TotalWeight);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

FLightingCache::FLightingCache(const FBox& InBoundingBox):
	Octree(InBoundingBox.GetCenter(),InBoundingBox.GetExtent().GetMax())
{}
