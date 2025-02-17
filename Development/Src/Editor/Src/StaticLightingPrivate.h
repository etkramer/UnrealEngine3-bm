/*=============================================================================
	StaticLightingPrivate.h: Private static lighting system definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __STATICLIGHTINGPRIVATE_H__
#define __STATICLIGHTINGPRIVATE_H__

// Don't compile the static lighting system on consoles.
#if !CONSOLE

// Includes.
#include "GenericOctree.h"
#include "LightingCache.h"

/** A line segment representing a direct light path through the scene. */
class FLightRay
{
public:

	FVector Start;
	FVector End;
	FVector Direction;
	FVector OneOverDirection;
	FLOAT Length;

	const FStaticLightingMapping* const Mapping;
	ULightComponent* const Light;

	/** Initialization constructor. */
	FLightRay(const FVector& InStart,const FVector& InEnd,const FStaticLightingMapping* InMapping,ULightComponent* InLight):
		Start(InStart),
		End(InEnd),
		Direction(InEnd - InStart),
		Mapping(InMapping),
		Light(InLight)
	{
		OneOverDirection.X = Square(Direction.X) > DELTA ? 1.0f / Direction.X : 0.0f;
		OneOverDirection.Y = Square(Direction.Y) > DELTA ? 1.0f / Direction.Y : 0.0f;
		OneOverDirection.Z = Square(Direction.Z) > DELTA ? 1.0f / Direction.Z : 0.0f;
		Length = 1.0f;
	}

	/** Clips the light ray to an intersection point. */
	void ClipAgainstIntersection(const FVector& IntersectionPoint)
	{
		End = IntersectionPoint;
		Length = (IntersectionPoint - Start) | Direction;
	}
};

/** A mesh and its bounding box. */
struct FMeshAndBounds
{
	const FStaticLightingMesh* Mesh;
	FBox BoundingBox;

	/** Initialization constructor. */
	FMeshAndBounds(const FStaticLightingMesh* InMesh):
		Mesh(InMesh)
	{
		if(Mesh)
		{
			BoundingBox = Mesh->BoundingBox;
		}
	}

	/**
	 * Checks a line segment for intersection with this mesh.
	 * @param LightRay - The line segment to check.
	 * @return The intersection of between the light ray and the mesh.
	 */
	FLightRayIntersection IntersectLightRay(const FLightRay& LightRay,UBOOL bFindClosestIntersection) const
	{
		if(	Mesh &&
			FLineBoxIntersection(BoundingBox,LightRay.Start,LightRay.End,LightRay.Direction,LightRay.OneOverDirection) &&
			Mesh->ShouldCastShadow(LightRay.Light,LightRay.Mapping))
		{
			return Mesh->IntersectLightRay(LightRay.Start,LightRay.End,bFindClosestIntersection);
		}
		else
		{
			return FLightRayIntersection::None();
		}
	}

};

typedef TOctree<FMeshAndBounds,struct FStaticLightingMeshOctreeSemantics> FStaticLightingMeshOctree;

/** Octree semantic definitions. */
struct FStaticLightingMeshOctreeSemantics
{
	enum { MaxElementsPerLeaf = 4 };
	enum { MaxNodeDepth = 9 };

	static FBoxCenterAndExtent GetBoundingBox(const FMeshAndBounds& MeshAndBounds)
	{
		return FBoxCenterAndExtent(MeshAndBounds.BoundingBox);
	}

	static void SetElementId(const FMeshAndBounds& Element,FOctreeElementId Id)
	{
	}
};

class FStaticLightingAggregateMeshDataProvider;  /* needs predeclaration... */

/** The static lighting mesh. */
class FStaticLightingAggregateMesh
{
public:

	/**
	 * Merges a mesh into the shadow mesh.
	 * @param Mesh - The mesh the triangle comes from.
	 */
	void AddMesh(const FStaticLightingMesh* Mesh);

	/** Prepares the mesh for raytracing. */
	void PrepareForRaytracing();

	/**
	 * Checks a light ray for intersection with the shadow mesh.
	 * @param LightRay - The line segment to check for intersection.
	 * @param CoherentRayCache - The calling thread's collision cache.
	 * @param bFindClosestIntersection - TRUE if the intersection must return the closest intersection.  FALSE if it may return any intersection.
	 * @return The intersection of between the light ray and the mesh.
	 */
	FLightRayIntersection IntersectLightRay(const FLightRay& LightRay,UBOOL bFindClosestIntersection,class FCoherentRayCache& CoherentRayCache) const;

	/** Initialization constructor. */
	FStaticLightingAggregateMesh();

private:

	/** Theoctree used to cull ray-mesh intersections. */
	FStaticLightingMeshOctree MeshOctree;

	friend class FStaticLightingAggregateMeshDataProvider;

	/** The world-space kd-tree which is used by the simple meshes in the world. */
	TkDOPTree<const FStaticLightingAggregateMeshDataProvider,DWORD> kDopTree;

	/** The triangles used to build the kd-tree, valid until PrepareForRaytracing is called. */
	TArray<FkDOPBuildCollisionTriangle<DWORD> > kDOPTriangles;

	/** The meshes used to build the kd-tree. */
	TArray<const FStaticLightingMesh*> kDopMeshes;

	/** The vertices used by the kd-tree. */
	TArray<FVector> Vertices;
};

/** Information which is cached while processing a group of coherent rays. */
class FCoherentRayCache
{
public:

	/** The mesh that was last hit by a ray in this thread. */
	FMeshAndBounds LastHitMesh;

	/** The thread's local area lighting cache. */
	FLightingCache AreaLightingCache;

	/** Initialization constructor. */
	FCoherentRayCache(const FStaticLightingMesh* InSubjectMesh):
		LastHitMesh(NULL),
		AreaLightingCache(InSubjectMesh->BoundingBox)
	{}
};

/** The state of the static lighting system. */
class FStaticLightingSystem
{
public:

	/**
	 * Initializes this static lighting system, and builds static lighting based on the provided options.
	 * @param InOptions - The static lighting build options.
	 */
	FStaticLightingSystem(const FLightingBuildOptions& InOptions);

	/**
	 * Calculates shadowing for a given mapping surface point and light.
	 * @param Mapping - The mapping the point comes from.
	 * @param WorldSurfacePoint - The point to check shadowing at.
	 * @param Light - The light to check shadowing from.
	 * @param CoherentRayCache - The calling thread's collision cache.
	 * @return TRUE if the surface point is shadowed from the light.
	 */
	UBOOL CalculatePointShadowing(const FStaticLightingMapping* Mapping,const FVector& WorldSurfacePoint,ULightComponent* Light,FCoherentRayCache& CoherentRayCache) const;

	/**
	 * Calculates the lighting contribution of a light to a mapping vertex.
	 * @param Mapping - The mapping the vertex comes from.
	 * @param Vertex - The vertex to calculate the lighting contribution at.
	 * @param Light - The light to calculate the lighting contribution from.
	 * @return The incident lighting on the vertex.
	 */
	FLightSample CalculatePointLighting(const FStaticLightingMapping* Mapping,const FStaticLightingVertex& Vertex,ULightComponent* Light) const;

	/**
	 * Calculates the lighting contribution of all area lights to a mapping vertex.
	 * @param Mapping - The mapping the vertex comes from.
	 * @param Vertex - The vertex to calculate the lighting contribution at.
	 * @param CoherentRayCache - The calling thread's collision cache.
	 * @return The incident area lighting on the vertex.
	 */
	FLightSample CalculatePointAreaLighting(const FStaticLightingMapping* Mapping,const FStaticLightingVertex& Vertex,FCoherentRayCache& CoherentRayCache);

private:

	/** A thread which processes static lighting mappings. */
	class FStaticLightingThreadRunnable : public FRunnable
	{
	public:

		FRunnableThread* Thread;

		/** Initialization constructor. */
		FStaticLightingThreadRunnable(FStaticLightingSystem* InSystem):
			System(InSystem),
			bTerminatedByError(FALSE)
		{}

		/** Checks the thread's health, and passes on any errors that have occured.  Called by the main thread. */
		void CheckHealth() const;

		// FRunnable interface.
		virtual UBOOL Init(void) { return TRUE; }
		virtual void Exit(void) {}
		virtual void Stop(void) {}
		virtual DWORD Run(void);

	private:
		FStaticLightingSystem* System;

		/** If the thread has been terminated by an unhandled exception, this contains the error message. */
		FString ErrorMessage;

		/** TRUE if the thread has been terminated by an unhandled exception. */
		UBOOL bTerminatedByError;
	};

	/** The static lighting data for a vertex mapping. */
	struct FVertexMappingStaticLightingData
	{
		FStaticLightingVertexMapping* Mapping;
		FLightMapData1D* LightMapData;
		TMap<ULightComponent*,FShadowMapData1D*> ShadowMaps;
	};

	/** The static lighting data for a texture mapping. */
	struct FTextureMappingStaticLightingData
	{
		FStaticLightingTextureMapping* Mapping;
		FLightMapData2D* LightMapData;
		TMap<ULightComponent*,FShadowMapData2D*> ShadowMaps;
	};

	/** Encapsulates a list of mappings which static lighting has been computed for, but not yet applied. */
	template<typename StaticLightingDataType>
	class TCompleteStaticLightingList
	{
	public:

		/** Initialization constructor. */
		TCompleteStaticLightingList():
			FirstElement(NULL)
		{}

		/** Adds an element to the list. */
		void AddElement(TList<StaticLightingDataType>* Element)
		{
			// Link the element at the beginning of the list.
			TList<StaticLightingDataType>* LocalFirstElement;
			do 
			{
				LocalFirstElement = FirstElement;
				Element->Next = LocalFirstElement;
			}
			while(appInterlockedCompareExchangePointer((void**)&FirstElement,Element,LocalFirstElement) != LocalFirstElement);
		}

		/** Applies the static lighting to the mappings in the list, and clears the list. */
		void ApplyAndClear();

	private:

		TList<StaticLightingDataType>* FirstElement;
	};

	/** The lights in the world which the system is building. */
	TArray<ULightComponent*> Lights;

	/** The options the system is building lighting with. */
	const FLightingBuildOptions Options;

	/** TRUE if the static lighting build has been canceled.  Written by the main thread, read by all static lighting threads. */
	UBOOL bBuildCanceled;

	/** The aggregate mesh used for raytracing. */
	FStaticLightingAggregateMesh AggregateMesh;

	/** All meshes in the system. */
	TArray< TRefCountPtr<FStaticLightingMesh> > Meshes;

	/** All mappings in the system. */
	TArray< TRefCountPtr<FStaticLightingMapping> > Mappings;

	/** The next index into Mappings which processing hasn't started for yet. */
	FThreadSafeCounter NextMappingToProcess;

	/** A list of the vertex mappings which static lighting has been computed for, but not yet applied.  This is accessed by multiple threads and should be written to using interlocked functions. */
	TCompleteStaticLightingList<FVertexMappingStaticLightingData> CompleteVertexMappingList;

	/** A list of the texture mappings which static lighting has been computed for, but not yet applied.  This is accessed by multiple threads and should be written to using interlocked functions. */
	TCompleteStaticLightingList<FTextureMappingStaticLightingData> CompleteTextureMappingList;

	/** The threads spawned by the static lighting system. */
	TIndirectArray<FStaticLightingThreadRunnable> Threads;

	/**
	 * Queries a primitive for its static lighting info, and adds it to the system.
	 * @param Primitive - The primitive to query.
	 * @param bBuildLightingForPrimitive - TRUE if lighting needs to be built for this primitive.
	 */
	void AddPrimitiveStaticLightingInfo(UPrimitiveComponent* Primitive,UBOOL bBuildLightingForPrimitive);

	/**
	 * Builds lighting for a vertex mapping.
	 * @param VertexMapping - The mapping to build lighting for.
	 */
	void ProcessVertexMapping(FStaticLightingVertexMapping* VertexMapping);

	/**
	 * Builds lighting for a texture mapping.
	 * @param TextureMapping - The mapping to build lighting for.
	 */
	void ProcessTextureMapping(FStaticLightingTextureMapping* TextureMapping);

	/**
	 * The processing loop for a static lighting thread.
	 * @param bIsMainThread - TRUE if this is running in the main thread.
	 */
	void ThreadLoop(UBOOL bIsMainThread);
};

/**
 * Checks if a light is behind a triangle.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Light - The light to classify.
 * @return TRUE if the light is behind the triangle.
 */
extern UBOOL IsLightBehindSurface(const FVector& TrianglePoint,const FVector& TriangleNormal,const ULightComponent* Light);

/**
 * Culls lights that are behind a triangle.
 * @param bTwoSidedMaterial - TRUE if the triangle has a two-sided material.  If so, lights behind the surface are not culled.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Lights - The lights to cull.
 * @return A map from Lights index to a boolean which is TRUE if the light is in front of the triangle.
 */
extern TBitArray<> CullBackfacingLights(UBOOL bTwoSidedMaterial,const FVector& TrianglePoint,const FVector& TriangleNormal,const TArray<ULightComponent*>& Lights);

#endif

#endif
