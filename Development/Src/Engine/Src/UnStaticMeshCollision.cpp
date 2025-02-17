/*=============================================================================
	UnStaticMeshCollision.cpp: Static mesh collision code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"
#include "UnCollision.h"

//
//	UStaticMesh::LineCheck
//

#define TRACK_START_PENETRATING (0)

#if TRACK_START_PENETRATING
static FVector LastPassStart(0);
static FVector LastPassEnd(0);
#endif // TRACK_START_PENETRATING


UBOOL UStaticMeshComponent::LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags)
{
	return LineCheck(Result, End, Start, Extent, TraceFlags, 0);
}


UBOOL UStaticMeshComponent::LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags, UINT LODIndex)
{
	if(!StaticMesh)
	{
		return Super::LineCheck(Result,End,Start,Extent,TraceFlags);
	}

	UBOOL	Hit = FALSE,
			ZeroExtent = Extent == FVector(0,0,0);
	// Reset the time otherwise it can default to zero
	Result.Time = 1.f;

#if STATS
	DWORD Counter = ZeroExtent ? (DWORD)STAT_StaticMeshZeroExtentTime : (DWORD)STAT_StaticMeshExtentTime;
	SCOPE_CYCLE_COUNTER(Counter);
#endif

	UBOOL bWantSimpleCheck = (StaticMesh->UseSimpleBoxCollision && !ZeroExtent) || (StaticMesh->UseSimpleLineCollision && ZeroExtent);
	
	if( Owner && 
		bWantSimpleCheck && 
		!(TraceFlags & TRACE_ShadowCast) && // Don't use simple collision for shadows or if specifically forced to use complex
		!(TraceFlags & TRACE_ComplexCollision) )
	{
		// If physics model are present. Note that if we asked for it, and it's not present, we just fail.
		if(StaticMesh->BodySetup)
		{
			FMatrix Transform;
			FVector Scale3D;
			GetTransformAndScale(Transform, Scale3D);
			UBOOL bStopAtAnyHit = (TraceFlags & TRACE_StopAtAnyHit);

			// Check against aggregates to see if one of those were hit sooner than the UModel
			Hit = !StaticMesh->BodySetup->AggGeom.LineCheck(Result, Transform, Scale3D, End, Start, Extent, bStopAtAnyHit, FALSE);
			if(Hit)
			{
#if TRACK_START_PENETRATING
				if(Result.bStartPenetrating && Abs(Start.Z - End.Z) < SMALL_NUMBER && Extent.Equals(FVector(34,34,72)))
				{
					FCheckResult Dummy1;
					StaticMesh->BodySetup->AggGeom.LineCheck(Dummy1, Transform, Scale3D, End, Start, Extent, bStopAtAnyHit);
					FCheckResult Dummy2;
					StaticMesh->BodySetup->AggGeom.LineCheck(Dummy2, Transform, Scale3D, LastPassEnd, LastPassStart, Extent, bStopAtAnyHit);
				}
#endif // TRACK_START_PENETRATING

				// Pull-back hit result
				FVector Vec = End - Start;
				FLOAT Dist = Vec.Size();
				Result.Time = Clamp(Result.Time - Clamp(0.1f,0.1f/Dist, 1.f/Dist),0.f,1.f);
				Result.Location = Start + (Vec * Result.Time);
				Result.Actor = Owner;
				Result.Component = this;

				// Get the physical material we hit.
				Result.PhysMaterial = StaticMesh->BodySetup->PhysMaterial;
			}
#if TRACK_START_PENETRATING
			else
			{
				if(Abs(Start.Z - End.Z) < SMALL_NUMBER && Extent.Equals(FVector(34,34,72)))
				{
					LastPassStart = Start;
					LastPassEnd = End;
				}
			}
#endif // TRACK_START_PENETRATING
		}
	}
	else if (StaticMesh->kDOPTree.Nodes.Num())
	{
		// Create the object that knows how to extract information from the component/mesh
		FStaticMeshCollisionDataProvider Provider(this, LODIndex);

		if (ZeroExtent == TRUE)
		{
			// Create the check structure with all the local space fun
			TkDOPLineCollisionCheck<FStaticMeshCollisionDataProvider,WORD> kDOPCheck(Start,End,TraceFlags,Provider,&Result);
			// Do the line check
			Hit = StaticMesh->kDOPTree.LineCheck(kDOPCheck);
			if (Hit == 1)
			{
				// Transform the hit normal to world space if there was a hit
				// This is deferred until now because multiple triangles might get
				// hit in the search and I want to delay the expensive transformation
				// as late as possible
				Result.Normal = kDOPCheck.GetHitNormal();
			}
		}
		else
		{
			// Create the check structure with all the local space fun
			TkDOPBoxCollisionCheck<FStaticMeshCollisionDataProvider,WORD> kDOPCheck(Start,End,Extent,TraceFlags,Provider,&Result);
			// And collide against it
			Hit = StaticMesh->kDOPTree.BoxCheck(kDOPCheck);
			if( Hit == 1 )
			{
				// Transform the hit normal to world space if there was a hit
				// This is deferred until now because multiple triangles might get
				// hit in the search and I want to delay the expensive transformation
				// as late as possible
				Result.Normal = kDOPCheck.GetHitNormal();
			}
		}
		// We hit this mesh, so update the common out values
		if (Hit == TRUE)
		{
			Result.Actor = Owner;
			Result.Component = this;
			Result.Time = Clamp(Result.Time - Clamp(0.1f,0.1f / (End - Start).Size(),4.0f / (End - Start).Size()),0.0f,1.0f);
			Result.Location = Start + (End - Start) * Result.Time;
		}
	}
	return !Hit;
}

//
//	UStaticMeshComponent::PointCheck
//

UBOOL UStaticMeshComponent::PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags)
{
	if(!StaticMesh)
	{
		return Super::PointCheck(Result,Location,Extent,TraceFlags);
	}

	SCOPE_CYCLE_COUNTER(STAT_StaticMeshPointTime);

	UBOOL	Hit = FALSE;

	UBOOL ZeroExtent = Extent.IsZero();
	if (!(TraceFlags & TRACE_ComplexCollision) && ((StaticMesh->UseSimpleBoxCollision && !ZeroExtent) || (StaticMesh->UseSimpleLineCollision && ZeroExtent)))
	{
		// Test against aggregate simplified collision in BodySetup
		if(StaticMesh->BodySetup)
		{
			// Get transform (without scaling) and scale separately
			FMatrix Transform;
			FVector Scale3D;
			GetTransformAndScale(Transform, Scale3D);

			// Do the point check
			FCheckResult TempCheck(1.f);
			Hit = !StaticMesh->BodySetup->AggGeom.PointCheck(TempCheck, Transform, Scale3D, Location, Extent);

			// If we hit, fill in the Result struct.
			if(Hit)
			{
				Result.Normal = TempCheck.Normal;
				Result.Location = TempCheck.Location;
				Result.Actor = Owner;
				Result.Component = this;
				Result.PhysMaterial = StaticMesh->BodySetup->PhysMaterial;
			}
		}
	}
	else if(StaticMesh->kDOPTree.Nodes.Num())
	{ 
		// Create the object that knows how to extract information from the component/mesh
		FStaticMeshCollisionDataProvider Provider(this);
		// Create the check structure with all the local space fun
		TkDOPPointCollisionCheck<FStaticMeshCollisionDataProvider,WORD> kDOPCheck(Location,Extent,Provider,&Result);
		Hit = StaticMesh->kDOPTree.PointCheck(kDOPCheck);
		// Transform the hit normal to world space if there was a hit
		// This is deferred until now because multiple triangles might get
		// hit in the search and I want to delay the expensive transformation
		// as late as possible. Same thing holds true for the hit location
		if (Hit == 1)
		{
			Result.Normal = kDOPCheck.GetHitNormal();
			Result.Location = kDOPCheck.GetHitLocation();
		}

		if(Hit)
		{
			Result.Normal.Normalize();
			// Now calculate the location of the hit in world space
			Result.Actor = Owner;
			Result.Component = this;
		}
	}

	return !Hit;
}

