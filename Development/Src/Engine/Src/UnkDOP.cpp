/*=============================================================================
	UnkDOP.cpp: k-DOP collision
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/** Collision stats */
DECLARE_STATS_GROUP(TEXT("Collision"),STATGROUP_Collision);
DECLARE_CYCLE_STAT(TEXT("Terrain Line Check"),STAT_TerrainZeroExtentTime,STATGROUP_Collision);
DECLARE_CYCLE_STAT(TEXT("Terrain Extent Check"),STAT_TerrainExtentTime,STATGROUP_Collision);
DECLARE_CYCLE_STAT(TEXT("Terrain Point Check"),STAT_TerrainPointTime,STATGROUP_Collision);
DECLARE_CYCLE_STAT(TEXT("SM Line Check"),STAT_StaticMeshZeroExtentTime,STATGROUP_Collision);
DECLARE_CYCLE_STAT(TEXT("SM Extent Check"),STAT_StaticMeshExtentTime,STATGROUP_Collision);
DECLARE_CYCLE_STAT(TEXT("SM Point Check"),STAT_StaticMeshPointTime,STATGROUP_Collision);

// These are the plane normals for the kDOP that we use (bounding box)
FVector FkDOPPlanes::PlaneNormals[NUM_PLANES] =
{
	FVector(1.f,0.f,0.f),
	FVector(0.f,1.f,0.f),
	FVector(0.f,0.f,1.f)
};
