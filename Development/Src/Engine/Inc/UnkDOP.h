/*=============================================================================
	UnkDOP.h: k-DOP collision
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	This file contains the definition of the kDOP static mesh collision classes,
	and structures.
	Details on the algorithm can be found at:

	ftp://ams.sunysb.edu/pub/geometry/jklosow/tvcg98.pdf

	There is also information on the topic in Real Time Rendering

=============================================================================*/

#include "UnCollision.h"

#ifndef _KDOP_H
#define _KDOP_H

// Indicates how many "k / 2" there are in the k-DOP. 3 == AABB == 6 DOP
#define NUM_PLANES	3
// The number of triangles to store in each leaf
#define MAX_TRIS_PER_LEAF 5
// Copied from float.h
#define MAX_FLT 3.402823466e+38F
// Line triangle epsilon (see Real-Time Rendering page 581)
#define LINE_TRIANGLE_EPSILON 1e-5f
// Parallel line kDOP plane epsilon
#define PARALLEL_LINE_KDOP_EPSILON 1e-30f
// Amount to expand the kDOP by
#define FUDGE_SIZE 0.1f

/**
 * Represents a single triangle. A kDOP may have 0 or more triangles contained
 * within the node. If it has any triangles, it will be in list (allocated
 * memory) form.
 */
template<typename KDOP_IDX_TYPE> struct FkDOPCollisionTriangle
{
	// Triangle index (indexes into Vertices)
	KDOP_IDX_TYPE v1, v2, v3;
	// The material of this triangle
	KDOP_IDX_TYPE MaterialIndex;

	// Set up indices
	FkDOPCollisionTriangle(KDOP_IDX_TYPE Index1,KDOP_IDX_TYPE Index2,KDOP_IDX_TYPE Index3) :
		v1(Index1), v2(Index2), v3(Index3), MaterialIndex(0)
	{
		}
	/**
	 * Full constructor that sets indices and material
	 */
	FkDOPCollisionTriangle(KDOP_IDX_TYPE Index1,KDOP_IDX_TYPE Index2,KDOP_IDX_TYPE Index3,KDOP_IDX_TYPE Material) :
		v1(Index1), v2(Index2), v3(Index3), MaterialIndex(Material)
	{
	}
	// Default constructor for serialization
	FkDOPCollisionTriangle(void) : 
		v1(0), v2(0), v3(0), MaterialIndex(0)
	{
	}

	// Serialization
	friend FArchive& operator<<(FArchive& Ar, FkDOPCollisionTriangle<KDOP_IDX_TYPE>& Tri)
	{
		// @warning BulkSerialize: FkDOPCollisionTriangle is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		Ar << Tri.v1 << Tri.v2 << Tri.v3 << Tri.MaterialIndex;
		return Ar;
	}
};

// This structure is used during the build process. It contains the triangle's
// centroid for calculating which plane it should be split or not with
template<typename KDOP_IDX_TYPE> struct FkDOPBuildCollisionTriangle : public FkDOPCollisionTriangle<KDOP_IDX_TYPE>
{
	/**
	 * Centroid of the triangle used for determining which bounding volume to
	 * place the triangle in
	 */
	FVector Centroid;
	/**
	 * First vertex in the triangle
	 */
	FVector V0;
	/**
	 * Second vertex in the triangle
	 */
	FVector V1;
	/**
	 * Third vertex in the triangle
	 */
	FVector V2;

	/**
	 * Sets the indices, material index, calculates the centroid using the
	 * specified triangle vertex positions
	 */
	FkDOPBuildCollisionTriangle(KDOP_IDX_TYPE Index1,KDOP_IDX_TYPE Index2,KDOP_IDX_TYPE Index3,
		KDOP_IDX_TYPE InMaterialIndex,
		const FVector& vert0,const FVector& vert1,const FVector& vert2) :
		FkDOPCollisionTriangle<KDOP_IDX_TYPE>(Index1,Index2,Index3,InMaterialIndex),
		V0(vert0), V1(vert1), V2(vert2)
	{
		// Now calculate the centroid for the triangle
		Centroid = (V0 + V1 + V2) / 3.f;
	}
};

// Forward declarations
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOP;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPNode;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPTree;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPLineCollisionCheck;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPBoxCollisionCheck;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPPointCollisionCheck;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPSphereQuery;
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPFrustumQuery;

/**
 * Contains the set of planes we check against.
 *
 * @todo templatize for higher numbers of planes (assuming this is faster)
 */
struct FkDOPPlanes
{
	/**
	 * The set of plane normals that have been pushed out in the min/max list
	 */
	static FVector PlaneNormals[NUM_PLANES];
};

/**
 * Holds the min/max planes that make up a bounding volume
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOP :
	public FkDOPPlanes
{
	/** Exposes data provider type to clients. */
	typedef COLL_DATA_PROVIDER DataProviderType;

	/**
	 * Min planes for this bounding volume
	 */
	FLOAT Min[NUM_PLANES];
	/**
	 * Max planes for this bounding volume
	 */
	FLOAT Max[NUM_PLANES];

	/**
	 * Initializes the planes to invalid states so that adding any point
	 * makes the volume update
	 */
	FORCEINLINE TkDOP()
	{
		Init();
	}

	/**
	 * Copies the passed in FkDOP and expands it by the extent. Note assumes AABB.
	 *
	 * @param kDOP -- The kDOP to copy
	 * @param Extent -- The extent to expand it by
	 */
	FORCEINLINE TkDOP(const TkDOP<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& kDOP,const FVector& Extent)
	{
		Min[0] = kDOP.Min[0] - Extent.X;
		Min[1] = kDOP.Min[1] - Extent.Y;
		Min[2] = kDOP.Min[2] - Extent.Z;
		Max[0] = kDOP.Max[0] + Extent.X;
		Max[1] = kDOP.Max[1] + Extent.Y;
		Max[2] = kDOP.Max[2] + Extent.Z;
	}

	/**
	 * Adds a new point to the kDOP volume, expanding it if needed.
	 *
	 * @param Point The vector to add to the volume
	 */
	void AddPoint(const FVector& Point)
	{
		// Dot against each plane and expand the volume out to encompass it if the
		// point is further away than either (min/max) of the previous points
		for (INT Plane = 0; Plane < NUM_PLANES; Plane++)
		{
			// Project this point onto the plane normal
			FLOAT Dot = Point | FkDOPPlanes::PlaneNormals[Plane];
			// Move the plane out as needed
			if (Dot < Min[Plane])
			{
				Min[Plane] = Dot;
			}
			if (Dot > Max[Plane])
			{
				Max[Plane] = Dot;
			}
		}
	}

	/**
	 * Adds a list of triangles to this volume
	 *
	 * @param Start the starting point in the build triangles array
	 * @param NumTris the number of tris to iterate through adding from the array
	 * @param BuildTriangles the list of triangles used for building the collision data
	 */
	void AddTriangles(KDOP_IDX_TYPE StartIndex,KDOP_IDX_TYPE NumTris,
		TArray<FkDOPBuildCollisionTriangle<KDOP_IDX_TYPE> >& BuildTriangles)
	{
		// Reset the min/max planes
		Init();
		// Go through the list and add each of the triangle verts to our volume
		for (KDOP_IDX_TYPE Triangle = StartIndex; Triangle < StartIndex + NumTris; Triangle++)
		{
			AddPoint(BuildTriangles(Triangle).V0);
			AddPoint(BuildTriangles(Triangle).V1);
			AddPoint(BuildTriangles(Triangle).V2);
		}
	}

	/**
	 * Sets the data to an invalid state (inside out volume)
	 */
	FORCEINLINE void Init(void)
	{
		for (INT nPlane = 0; nPlane < NUM_PLANES; nPlane++)
		{
			Min[nPlane] = MAX_FLT;
			Max[nPlane] = -MAX_FLT;
		}
	}

	/**
	 * Checks a line against this kDOP. Note this assumes a AABB. If more planes
	 * are to be used, this needs to be rewritten. Also note, this code is Andrew's
	 * original code modified to work with FkDOP
	 *
	 * input:	Check The aggregated line check structure
	 *			HitTime The out value indicating hit time
	 */
	UBOOL LineCheck(TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check,FLOAT& HitTime) const
	{
		FVector	Time(0.f,0.f,0.f);
		UBOOL Inside = 1;

		HitTime = 0.0f;  // always initialize (prevent valgrind whining) --ryan.

		if(Check.LocalStart.X < Min[0])
		{
			if(Check.LocalDir.X <= 0.0f)
				return 0;
			else
			{
				Inside = 0;
				Time.X = (Min[0] - Check.LocalStart.X) * Check.LocalOneOverDir.X;
			}
		}
		else if(Check.LocalStart.X > Max[0])
		{
			if(Check.LocalDir.X >= 0.0f)
				return 0;
			else
			{
				Inside = 0;
				Time.X = (Max[0] - Check.LocalStart.X) * Check.LocalOneOverDir.X;
			}
		}

		if(Check.LocalStart.Y < Min[1])
		{
			if(Check.LocalDir.Y <= 0.0f)
				return 0;
			else
			{
				Inside = 0;
				Time.Y = (Min[1] - Check.LocalStart.Y) * Check.LocalOneOverDir.Y;
			}
		}
		else if(Check.LocalStart.Y > Max[1])
		{
			if(Check.LocalDir.Y >= 0.0f)
				return 0;
			else
			{
				Inside = 0;
				Time.Y = (Max[1] - Check.LocalStart.Y) * Check.LocalOneOverDir.Y;
			}
		}

		if(Check.LocalStart.Z < Min[2])
		{
			if(Check.LocalDir.Z <= 0.0f)
				return 0;
			else
			{
				Inside = 0;
				Time.Z = (Min[2] - Check.LocalStart.Z) * Check.LocalOneOverDir.Z;
			}
		}
		else if(Check.LocalStart.Z > Max[2])
		{
			if(Check.LocalDir.Z >= 0.0f)
				return 0;
			else
			{
				Inside = 0;
				Time.Z = (Max[2] - Check.LocalStart.Z) * Check.LocalOneOverDir.Z;
			}
		}

		if(Inside)
		{
			HitTime = 0.f;
			return 1;
		}

		HitTime = Time.GetMax();

		if(HitTime >= 0.0f && HitTime <= 1.0f)
		{
			const FVector& Hit = Check.LocalStart + Check.LocalDir * HitTime;

			return (Hit.X > Min[0] - FUDGE_SIZE && Hit.X < Max[0] + FUDGE_SIZE &&
					Hit.Y > Min[1] - FUDGE_SIZE && Hit.Y < Max[1] + FUDGE_SIZE &&
					Hit.Z > Min[2] - FUDGE_SIZE && Hit.Z < Max[2] + FUDGE_SIZE);
		}
		return 0;
	}
	
	/**
	 * Checks a point with extent against this kDOP. The extent is already added in
	 * to the kDOP being tested (Minkowski sum), so this code just checks to see if
	 * the point is inside the kDOP. Note this assumes a AABB. If more planes are 
	 * to be used, this needs to be rewritten.
	 *
	 * @param Check The aggregated point check structure
	 */
	UBOOL PointCheck(TkDOPPointCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		return Check.LocalStart.X >= Min[0] && Check.LocalStart.X <= Max[0] 
			&& Check.LocalStart.Y >= Min[1] && Check.LocalStart.Y <= Max[1] 
			&& Check.LocalStart.Z >= Min[2] && Check.LocalStart.Z <= Max[2];
	}

	/**
	 * Check (local space) AABB against this kDOP.
	 *
	 * @param LocalAABB box in local space
	 */
	UBOOL AABBOverlapCheck(const FBox& LocalAABB) const
	{
		return Min[0] <= LocalAABB.Max.X && LocalAABB.Min.X <= Max[0] &&
			Min[1] <= LocalAABB.Max.Y && LocalAABB.Min.Y <= Max[1] &&
			Min[2] <= LocalAABB.Max.Z && LocalAABB.Min.Z <= Max[2];
	}

	/**
	 * Check frustum planes against this kDOP.
	 *
	 * input:	frustum planes
	 */
	UBOOL FrustumCheck(const TArray<FPlane>& FrustumPlanes) const
	{
		const FVector Extent((Max[0] - Min[0]) * 0.5f, (Max[1] - Min[1]) * 0.5f, (Max[2] - Min[2]) * 0.5f);
		const FVector Center(Min[0] + Extent.X, Min[1] + Extent.Y, Min[2] + Extent.Z);

		for( INT PlaneIdx = 0; PlaneIdx < FrustumPlanes.Num() ; PlaneIdx++ )
		{
			const FLOAT PushOut = FBoxPushOut( FrustumPlanes(PlaneIdx), Extent );
			const FLOAT Dist = FrustumPlanes(PlaneIdx).PlaneDot(Center);
			if ( Dist > PushOut )
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	/** Create an FBox for these bounds. */
	FBox ToFBox() const
	{
		FVector NodeMin(Min[0], Min[1], Min[2]);
		FVector NodeMax(Max[0], Max[1], Max[2]);
		return FBox(NodeMin, NodeMax);
	}

	// Serialization
	friend FArchive& operator<<(FArchive& Ar,TkDOP<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& kDOP)
	{
		// Serialize the min planes
		for (INT nIndex = 0; nIndex < NUM_PLANES; nIndex++)
		{
			Ar << kDOP.Min[nIndex];
		}
		// Serialize the max planes
		for (INT nIndex = 0; nIndex < NUM_PLANES; nIndex++)
		{
			Ar << kDOP.Max[nIndex];
		}
		return Ar;
	}
};

/**
 * A node in the kDOP tree. The node contains the kDOP volume that encompasses
 * it's children and/or triangles
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPNode
{
	/** Exposes data provider type to clients. */
	typedef COLL_DATA_PROVIDER							DataProviderType;

	/** Exposes node type to clients. */
	typedef TkDOPNode<DataProviderType,KDOP_IDX_TYPE>	NodeType;

	/** Exposes kDOP type to clients. */
	typedef TkDOP<DataProviderType,KDOP_IDX_TYPE>		kDOPType;

	// The bounding kDOP for this node
	kDOPType BoundingVolume;

	// Note this isn't smaller since 4 byte alignment will take over anyway
	UBOOL bIsLeaf;

	// Union of either child kDOP nodes or a list of enclosed triangles
	union
	{
		// This structure contains the left and right child kDOP indices
		// These index values correspond to the array in the FkDOPTree
		struct
		{
			KDOP_IDX_TYPE LeftNode;
			KDOP_IDX_TYPE RightNode;
		} n;
		// This structure contains the list of enclosed triangles
		// These index values correspond to the triangle information in the
		// FkDOPTree using the start and count as the means of delineating
		// which triangles are involved
		struct
		{
			KDOP_IDX_TYPE NumTriangles;
			KDOP_IDX_TYPE StartIndex;
		} t;
	};

	/**
	 * Inits the data to no child nodes and an inverted volume
	 */
	FORCEINLINE TkDOPNode()
	{
		n.LeftNode = ((KDOP_IDX_TYPE) -1);
        n.RightNode = ((KDOP_IDX_TYPE) -1);
		BoundingVolume.Init();
	}

	/**
	 * Determines if the node is a leaf or not. If it is not a leaf, it subdivides
	 * the list of triangles again adding two child nodes and splitting them on
	 * the mean (splatter method). Otherwise it sets up the triangle information.
	 *
	 * @param Start -- The triangle index to start processing with
	 * @param NumTris -- The number of triangles to process
	 * @param BuildTriangles -- The list of triangles to use for the build process
	 * @param Nodes -- The list of nodes in this tree
	 */
	void SplitTriangleList(INT Start,INT NumTris,
		TArray<FkDOPBuildCollisionTriangle<KDOP_IDX_TYPE> >& BuildTriangles,
		TArray<NodeType>& Nodes)
	{
		// Add all of the triangles to the bounding volume
		BoundingVolume.AddTriangles(Start,NumTris,BuildTriangles);
		// Figure out if we are a leaf node or not
		if (NumTris > MAX_TRIS_PER_LEAF)
		{
			// Still too many triangles, so continue subdividing the triangle list
			bIsLeaf = 0;
			INT BestPlane = -1;
			FLOAT BestMean = 0.f;
			FLOAT BestVariance = 0.f;
			// Determine how to split using the splatter algorithm
			for (INT nPlane = 0; nPlane < NUM_PLANES; nPlane++)
			{
				FLOAT Mean = 0.f;
				FLOAT Variance = 0.f;
				// Compute the mean for the triangle list
				for (INT nTriangle = Start; nTriangle < Start + NumTris; nTriangle++)
				{
					// Project the centroid of the triangle against the plane
					// normals and accumulate to find the total projected
					// weighting
					Mean += BuildTriangles(nTriangle).Centroid | FkDOPPlanes::PlaneNormals[nPlane];
				}
				// Divide by the number of triangles to get the average
				Mean /= FLOAT(NumTris);
				// Compute variance of the triangle list
				for (INT nTriangle = Start; nTriangle < Start + NumTris;nTriangle++)
				{
					// Project the centroid again
					FLOAT Dot = BuildTriangles(nTriangle).Centroid | FkDOPPlanes::PlaneNormals[nPlane];
					// Now calculate the variance and accumulate it
					Variance += (Dot - Mean) * (Dot - Mean);
				}
				// Get the average variance
				Variance /= FLOAT(NumTris);
				// Determine if this plane is the best to split on or not
				if (Variance >= BestVariance)
				{
					BestPlane = nPlane;
					BestVariance = Variance;
					BestMean = Mean;
				}
			}
			// Now that we have the plane to split on, work through the triangle
			// list placing them on the left or right of the splitting plane
			INT Left = Start - 1;
			INT Right = Start + NumTris;
			// Keep working through until the left index passes the right
			while (Left < Right)
			{
				FLOAT Dot;
				// Find all the triangles to the "left" of the splitting plane
				do
				{
					Dot = BuildTriangles(++Left).Centroid | FkDOPPlanes::PlaneNormals[BestPlane];
				}
				while (Dot < BestMean && Left < Right);
				// Find all the triangles to the "right" of the splitting plane
				do
				{
					Dot = BuildTriangles(--Right).Centroid | FkDOPPlanes::PlaneNormals[BestPlane];
				}
				while (Dot >= BestMean && Right > 0 && Left < Right);
				// Don't swap the triangle data if we just hit the end
				if (Left < Right)
				{
					// Swap the triangles since they are on the wrong sides of the
					// splitting plane
					FkDOPBuildCollisionTriangle<KDOP_IDX_TYPE> Temp = BuildTriangles(Left);
					BuildTriangles(Left) = BuildTriangles(Right);
					BuildTriangles(Right) = Temp;
				}
			}
			// Check for wacky degenerate case where more than MAX_TRIS_PER_LEAF
			// fall all in the same kDOP
			if (Left == Start + NumTris || Right == Start)
			{
				Left = Start + (NumTris / 2);
			}
			// Add the two child nodes
			n.LeftNode = Nodes.Add(2);
			n.RightNode = n.LeftNode + 1;
			// Have the left node recursively subdivide it's list
			Nodes(n.LeftNode).SplitTriangleList(Start,Left - Start,BuildTriangles,Nodes);
			// And now have the right node recursively subdivide it's list
			Nodes(n.RightNode).SplitTriangleList(Left,Start + NumTris - Left,BuildTriangles,Nodes);
		}
		else
		{
			// No need to subdivide further so make this a leaf node
			bIsLeaf = 1;
			// Copy in the triangle information
			t.StartIndex = Start;
			t.NumTriangles = NumTris;
		}
	}

	/* 
	 * Determines the line in the FkDOPLineCollisionCheck intersects this node. It
	 * also will check the child nodes if it is not a leaf, otherwise it will check
	 * against the triangle data.
	 *
	 * @param Check -- The aggregated line check data
	 */
	inline UBOOL LineCheck(TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		UBOOL bHit = 0;
		// If this is a node, check the two child nodes and pick the closest one
		// to recursively check against and only check the second one if there is
		// not a hit or the hit returned is further out than the second node
		if (bIsLeaf == 0)
		{
			// Holds the indices for the closest and farthest nodes
			INT NearNode = -1;
			INT FarNode = -1;
			// Holds the hit times for the child nodes
			FLOAT NodeHitTime, NearTime = 0.f, FarTime = 0.f;
			// Assume the left node is closer (it will be adjusted later)
			if (Check.Nodes(n.LeftNode).BoundingVolume.LineCheck(Check,NodeHitTime))
			{
				NearNode = n.LeftNode;
				NearTime = NodeHitTime;
			}
			// Find out if the second node is closer
			if (Check.Nodes(n.RightNode).BoundingVolume.LineCheck(Check,NodeHitTime))
			{
				// See if the left node was a miss and make the right the near node
				if (NearNode == -1)
				{
					NearNode = n.RightNode;
					NearTime = NodeHitTime;
				}
				else
				{
					FarNode = n.RightNode;
					FarTime = NodeHitTime;
				}
			}
			// Swap the Near/FarNodes if the right node is closer than the left
			if (NearNode != -1 && FarNode != -1 && FarTime < NearTime)
			{
				Exchange(NearNode,FarNode);
				Exchange(NearTime,FarTime);
			}
			// See if we need to search the near node or not
			if (NearNode != -1 && Check.Result->Time > NearTime)
			{
				bHit = Check.Nodes(NearNode).LineCheck(Check);
			}
			// Check for an early out
			const UBOOL bStopAtAnyHit = Check.TraceFlags & TRACE_StopAtAnyHit;
			// Now do the same for the far node. This will only happen if a miss in
			// the near node or the nodes overlapped and this one is closer
			if (FarNode != -1 &&
				(Check.Result->Time > FarTime || bHit == FALSE) &&
				(bHit == FALSE || bStopAtAnyHit == FALSE))
			{
				bHit |= Check.Nodes(FarNode).LineCheck(Check);
			}
		}
		else
		{
			// This is a leaf, check the triangles for a hit
			bHit = LineCheckTriangles(Check);
		}
		return bHit;
	}

	/**
	 * Works through the list of triangles in this node checking each one for a
	 * collision.
	 *
	 * @param Check -- The aggregated line check data
	 */
	FORCEINLINE UBOOL LineCheckTriangles(TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		// Assume a miss
		UBOOL bHit = FALSE;
		// Check for an early out
		const UBOOL bStopAtAnyHit = Check.TraceFlags & TRACE_StopAtAnyHit;
		// Loop through all of our triangles. We need to check them all in case
		// there are two (or more) potential triangles that would collide and let
		// the code choose the closest
		for (KDOP_IDX_TYPE CollTriIndex = t.StartIndex;
			CollTriIndex < t.StartIndex + t.NumTriangles &&
			(bHit == FALSE || bStopAtAnyHit == FALSE);
			CollTriIndex++)
		{
			// Get the collision triangle that we are checking against
			const FkDOPCollisionTriangle<KDOP_IDX_TYPE>& CollTri =	Check.CollisionTriangles(CollTriIndex);
			if(Check.CollDataProvider.ShouldCheckMaterial(CollTri.MaterialIndex))
			{
				// Now get refs to the 3 verts to check against
				const FVector& v1 = Check.CollDataProvider.GetVertex(CollTri.v1);
				const FVector& v2 = Check.CollDataProvider.GetVertex(CollTri.v2);
				const FVector& v3 = Check.CollDataProvider.GetVertex(CollTri.v3);
				// Now check for an intersection
				bHit |= LineCheckTriangle(Check,v1,v2,v3,CollTri.MaterialIndex);
			}
		}
		return bHit;
	}

	/**
	 * Performs collision checking against the triangle using the old collision
	 * code to handle it. This is done to provide consistency in collision.
	 *
	 * @param Check -- The aggregated line check data
	 * @param v1 -- The first vertex of the triangle
	 * @param v2 -- The second vertex of the triangle
	 * @param v3 -- The third vertex of the triangle
	 * @param MaterialIndex -- The material for this triangle if it is hit
	 */
	UBOOL LineCheckTriangle(TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check,
		const FVector& v1,const FVector& v2,const FVector& v3,KDOP_IDX_TYPE MaterialIndex) const
	{
		// Calculate the hit normal the same way the old code
		// did so things are the same
		const FVector& LocalNormal = ((v2 - v3) ^ (v1 - v3)).SafeNormal();
		// Calculate the hit time the same way the old code
		// did so things are the same
		FPlane TrianglePlane(v1,LocalNormal);
		const FLOAT StartDist = TrianglePlane.PlaneDot(Check.LocalStart);
		const FLOAT EndDist = TrianglePlane.PlaneDot(Check.LocalEnd);
		if ((StartDist == EndDist) || (StartDist < -0.001f && EndDist < -0.001f) || (StartDist > 0.001f && EndDist > 0.001f))
		{
			return FALSE;
		}
		// Figure out when it will hit the triangle
		FLOAT Time = -StartDist / (EndDist - StartDist);
		// If this triangle is not closer than the previous hit, reject it
		if (Time < 0.f || Time >= Check.Result->Time)
		{
			return FALSE;
		}
		// Calculate the line's point of intersection with the node's plane
		const FVector& Intersection = Check.LocalStart + Check.LocalDir * Time;
		const FVector* Verts[3] = 
		{ 
			&v1, &v2, &v3
		};
		// Check if the point of intersection is inside the triangle's edges.
		for( INT SideIndex = 0; SideIndex < 3; SideIndex++ )
		{
			const FVector& SideDirection = LocalNormal ^
				(*Verts[(SideIndex + 1) % 3] - *Verts[SideIndex]);
			const FLOAT SideW = SideDirection | *Verts[SideIndex];
			if (((SideDirection | Intersection) - SideW) >= 0.001f)
			{
				return FALSE;
			}
		}
		// Return results
		Check.LocalHitNormal = LocalNormal;
		Check.Result->Time = Time;
		Check.Result->Material = Check.CollDataProvider.GetMaterial(MaterialIndex);
		Check.Result->Item = Check.CollDataProvider.GetItemIndex(MaterialIndex);
		return TRUE;
	}

	/**
	 * Determines the line + extent in the FkDOPBoxCollisionCheck intersects this
	 * node. It also will check the child nodes if it is not a leaf, otherwise it
	 * will check against the triangle data.
	 *
	 * @param Check -- The aggregated box check data
	 */
	UBOOL BoxCheck(TkDOPBoxCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		UBOOL bHit = FALSE;
		// If this is a node, check the two child nodes and pick the closest one
		// to recursively check against and only check the second one if there is
		// not a hit or the hit returned is further out than the second node
		if (bIsLeaf == 0)
		{
			// Holds the indices for the closest and farthest nodes
			INT NearNode = -1;
			INT FarNode = -1;
			// Holds the hit times for the child nodes
			FLOAT NodeHitTime = 0.f, NearTime = 0.f, FarTime = 0.f;
			// Update the kDOP with the extent and test against that
			kDOPType kDOPNear(Check.Nodes(n.LeftNode).BoundingVolume,
				Check.LocalExtent);
			// Assume the left node is closer (it will be adjusted later)
			if (kDOPNear.LineCheck(Check,NodeHitTime))
			{
				NearNode = n.LeftNode;
				NearTime = NodeHitTime;
			}
			// Update the kDOP with the extent and test against that
			kDOPType kDOPFar(Check.Nodes(n.RightNode).BoundingVolume,
				Check.LocalExtent);
			// Find out if the second node is closer
			if (kDOPFar.LineCheck(Check,NodeHitTime))
			{
				// See if the left node was a miss and make the right the near node
				if (NearNode == -1)
				{
					NearNode = n.RightNode;
					NearTime = NodeHitTime;
				}
				else
				{
					FarNode = n.RightNode;
					FarTime = NodeHitTime;
				}
			}
			// Swap the Near/FarNodes if the right node is closer than the left
			if (NearNode != -1 && FarNode != -1 && FarTime < NearTime)
			{
				Exchange(NearNode,FarNode);
				Exchange(NearTime,FarTime);
			}
			// See if we need to search the near node or not
			if (NearNode != -1 && Check.Result->Time > NearTime)
			{
				bHit = Check.Nodes(NearNode).BoxCheck(Check);
			}
			// Check for an early out
			const UBOOL bStopAtAnyHit = Check.TraceFlags & TRACE_StopAtAnyHit;
			// Now do the same for the far node. This will only happen if a miss in
			// the near node or the nodes overlapped and this one is closer
			if (FarNode != -1 &&
				(Check.Result->Time > FarTime || bHit == FALSE) &&
				(bHit == FALSE || bStopAtAnyHit == FALSE))
			{
				bHit |= Check.Nodes(FarNode).BoxCheck(Check);
			}
		}
		else
		{
			// This is a leaf, check the triangles for a hit
			bHit = BoxCheckTriangles(Check);
		}
		return bHit;
	}

	/**
	 * Works through the list of triangles in this node checking each one for a
	 * collision.
	 *
	 * @param Check -- The aggregated box check data
	 */
	FORCEINLINE UBOOL BoxCheckTriangles(TkDOPBoxCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		// Assume a miss
		UBOOL bHit = 0;
		// Use an early out if possible
		const UBOOL bStopAtAnyHit = Check.TraceFlags & TRACE_StopAtAnyHit;
		// Loop through all of our triangles. We need to check them all in case
		// there are two (or more) potential triangles that would collide and let
		// the code choose the closest
		for (KDOP_IDX_TYPE CollTriIndex = t.StartIndex;
			CollTriIndex < t.StartIndex + t.NumTriangles &&
			(bHit == FALSE || bStopAtAnyHit == FALSE);
			CollTriIndex++)
		{
			// Get the collision triangle that we are checking against
			const FkDOPCollisionTriangle<KDOP_IDX_TYPE>& CollTri = Check.CollisionTriangles(CollTriIndex);
			if(Check.CollDataProvider.ShouldCheckMaterial(CollTri.MaterialIndex))
			{
				// Now get refs to the 3 verts to check against
				const FVector& v1 = Check.CollDataProvider.GetVertex(CollTri.v1);
				const FVector& v2 = Check.CollDataProvider.GetVertex(CollTri.v2);
				const FVector& v3 = Check.CollDataProvider.GetVertex(CollTri.v3);
				// Now check for an intersection using the Separating Axis Theorem
				bHit |= BoxCheckTriangle(Check,v1,v2,v3,CollTri.MaterialIndex);
			}
		}
		return bHit;
	}

	/**
	 * Uses the separating axis theorem to check for triangle box collision.
	 *
	 * @param Check -- The aggregated box check data
	 * @param v1 -- The first vertex of the triangle
	 * @param v2 -- The second vertex of the triangle
	 * @param v3 -- The third vertex of the triangle
	 * @param MaterialIndex -- The material for this triangle if it is hit
	 */
	FORCEINLINE UBOOL BoxCheckTriangle(TkDOPBoxCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check,
		const FVector& v1,const FVector& v2,const FVector& v3,INT MaterialIndex) const
	{
		FLOAT HitTime = 1.f;
		FVector HitNormal(0.f,0.f,0.f);
		// Now check for an intersection using the Separating Axis Theorem
		UBOOL Result = FindSeparatingAxis(v1,v2,v3,Check.LocalStart,
			Check.LocalEnd,Check.Extent,Check.LocalBoxX,Check.LocalBoxY,
			Check.LocalBoxZ,HitTime,HitNormal);
		if (Result)
		{
			if (HitTime < Check.Result->Time)
			{
				// Store the better time
				Check.Result->Time = HitTime;
				// Get the material and item that was hit
				Check.Result->Material = Check.CollDataProvider.GetMaterial(MaterialIndex);
				Check.Result->Item = Check.CollDataProvider.GetItemIndex(MaterialIndex);
				// Normal will get transformed to world space at end of check
				Check.LocalHitNormal = HitNormal;
			}
			else
			{
				Result = FALSE;
			}
		}
		return Result;
	}

	/**
	 * Determines the point + extent in the FkDOPPointCollisionCheck intersects
	 * this node. It also will check the child nodes if it is not a leaf, otherwise
	 * it will check against the triangle data.
	 *
	 * @param Check -- The aggregated point check data
	 */
	UBOOL PointCheck(TkDOPPointCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		UBOOL bHit = FALSE;
		// If this is a node, check the two child nodes recursively
		if (bIsLeaf == 0)
		{
			// Holds the indices for the closest and farthest nodes
			INT NearNode = -1;
			INT FarNode = -1;
			// Update the kDOP with the extent and test against that
			kDOPType kDOPNear(Check.Nodes(n.LeftNode).BoundingVolume,
				Check.LocalExtent);
			// Assume the left node is closer (it will be adjusted later)
			if (kDOPNear.PointCheck(Check))
			{
				NearNode = n.LeftNode;
			}
			// Update the kDOP with the extent and test against that
			kDOPType kDOPFar(Check.Nodes(n.RightNode).BoundingVolume,
				Check.LocalExtent);
			// Find out if the second node is closer
			if (kDOPFar.PointCheck(Check))
			{
				// See if the left node was a miss and make the right the near node
				if (NearNode == -1)
				{
					NearNode = n.RightNode;
				}
				else
				{
					FarNode = n.RightNode;
				}
			}
			// See if we need to search the near node or not
			if (NearNode != -1)
			{
				bHit = Check.Nodes(NearNode).PointCheck(Check);
			}
			// Now do the same for the far node
			if (FarNode != -1)
			{
				bHit |= Check.Nodes(FarNode).PointCheck(Check);
			}
		}
		else
		{
			// This is a leaf, check the triangles for a hit
			bHit = PointCheckTriangles(Check);
		}
		return bHit;
	}

	/**
	 * Works through the list of triangles in this node checking each one for a
	 * collision.
	 *
	 * @param Check -- The aggregated point check data
	 */
	FORCEINLINE UBOOL PointCheckTriangles(TkDOPPointCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		// Assume a miss
		UBOOL bHit = FALSE;
		// Loop through all of our triangles. We need to check them all in case
		// there are two (or more) potential triangles that would collide and let
		// the code choose the closest
		for (KDOP_IDX_TYPE CollTriIndex = t.StartIndex;
			CollTriIndex < t.StartIndex + t.NumTriangles;
			CollTriIndex++)
		{
			// Get the collision triangle that we are checking against
			const FkDOPCollisionTriangle<KDOP_IDX_TYPE>& CollTri =	Check.CollisionTriangles(CollTriIndex);
			if(Check.CollDataProvider.ShouldCheckMaterial(CollTri.MaterialIndex))
			{
				// Now get refs to the 3 verts to check against
				const FVector& v1 = Check.CollDataProvider.GetVertex(CollTri.v1);
				const FVector& v2 = Check.CollDataProvider.GetVertex(CollTri.v2);
				const FVector& v3 = Check.CollDataProvider.GetVertex(CollTri.v3);
				// Now check for an intersection using the Separating Axis Theorem
				bHit |= PointCheckTriangle(Check,v1,v2,v3,CollTri.MaterialIndex);
			}
		}
		return bHit;
	}

	/**
	 * Uses the separating axis theorem to check for triangle box collision.
	 *
	 * @param Check -- The aggregated box check data
	 * @param v1 -- The first vertex of the triangle
	 * @param v2 -- The second vertex of the triangle
	 * @param v3 -- The third vertex of the triangle
	 * @param MaterialIndex -- The material for this triangle if it is hit
	 */
	FORCEINLINE UBOOL PointCheckTriangle(TkDOPPointCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check,
		const FVector& v1,const FVector& v2,const FVector& v3,INT InMaterialIndex) const
	{
		// Use the separating axis theorem to see if we hit
		FSeparatingAxisPointCheck ThePointCheck(v1,v2,v3,Check.LocalStart,Check.Extent,
			Check.LocalBoxX,Check.LocalBoxY,Check.LocalBoxZ,Check.BestDistance);

		// If we hit and it is closer update the out values
		if (ThePointCheck.Hit && ThePointCheck.BestDist < Check.BestDistance)
		{
			// Get the material and item that was hit
			Check.Result->Material = Check.CollDataProvider.GetMaterial(InMaterialIndex);
			Check.Result->Item = Check.CollDataProvider.GetItemIndex(InMaterialIndex);
			// Normal will get transformed to world space at end of check
			Check.LocalHitNormal = ThePointCheck.HitNormal;
			// Copy the distance for push out calculations
			Check.BestDistance = ThePointCheck.BestDist;
			return TRUE;
		}
		return FALSE;
	}

	/**
	 * Find triangles that overlap the given sphere. We assume that the supplied box overlaps this node.
	 *
	 * @param Query -- Query information
	 */
	FORCEINLINE void SphereQuery(TkDOPSphereQuery<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Query) const
	{
		// If not leaf, check against each child.
		if(bIsLeaf == 0)
		{
			if( Query.Nodes(n.LeftNode).BoundingVolume.AABBOverlapCheck(Query.LocalBox) )
			{
				Query.Nodes(n.LeftNode).SphereQuery(Query);
			}

			if( Query.Nodes(n.RightNode).BoundingVolume.AABBOverlapCheck(Query.LocalBox) )
			{
				Query.Nodes(n.RightNode).SphereQuery(Query);
			}
		}
		else // Otherwise, add all the triangles in this node to the list.
		{
			for (KDOP_IDX_TYPE TriIndex = t.StartIndex;
				TriIndex < t.StartIndex + t.NumTriangles; TriIndex++)
			{
				Query.ReturnTriangles.AddItem(TriIndex);
			}
		}

	}

	/**
	 * Find nodes that overlap the given frustum. We assume that the supplied
	 * frustum overlaps this node.
	 *
	 * input:	Query -- Query information
	 */
	UBOOL FrustumQuery( TkDOPFrustumQuery<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Query ) const
	{
		if( !BoundingVolume.FrustumCheck(Query.LocalFrustumPlanes) )
			return 0;

		// left node
		if( Query.Nodes.IsValidIndex( n.LeftNode ) )
		{
			const NodeType& Left = Query.Nodes(n.LeftNode);
			if( !Left.bIsLeaf )
			{
				// recurse
				Left.FrustumQuery( Query );
			}
			else
			{
				// add index to this leaf node
				Query.AddLeafIndex( n.LeftNode );
			}
		}

		// right node
		if( Query.Nodes.IsValidIndex( n.RightNode ) )
		{
			const NodeType& Right = Query.Nodes(n.RightNode);
			if( !Right.bIsLeaf )
			{
				// recurse
				Right.FrustumQuery( Query );
			}
			else
			{
				// add index to this leaf node
				Query.AddLeafIndex( n.RightNode );
			}
		}

		return 1;
	}

	// Serialization
	friend FArchive& operator<<(FArchive& Ar,NodeType& Node)
	{
		// @warning BulkSerialize: FkDOPNode is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		Ar << Node.BoundingVolume << Node.bIsLeaf;
		// If we are a leaf node, serialize out the child node indices, otherwise serialize out the triangle collision data.
		// n.LeftNode overlaps t.NumTriangles in memory
		// n.RightNode overlaps t.StartIndex in memory
		Ar << Node.n.LeftNode << Node.n.RightNode;
		return Ar;
	}
};

/**
 * This is the tree of kDOPs that spatially divides the static mesh. It is
 * a binary tree of kDOP nodes.
 */
template<typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPTree
{
	/** Exposes data provider type to clients. */
	typedef COLL_DATA_PROVIDER							DataProviderType;

	/** Exposes node type to clients. */
	typedef TkDOPNode<DataProviderType,KDOP_IDX_TYPE>	NodeType;

	/** Exposes kDOP type to clients. */
	typedef typename NodeType::kDOPType					kDOPType;

	/** The list of nodes contained within this tree. Node 0 is always the root node. */
	TArray<NodeType> Nodes;

	/** The list of collision triangles in this tree. */
	TArray<FkDOPCollisionTriangle<KDOP_IDX_TYPE> > Triangles;

	/**
	 * Creates the root node and recursively splits the triangles into smaller
	 * volumes
	 *
	 * @param BuildTriangles -- The list of triangles to use for the build process
	 */
	void Build(TArray<FkDOPBuildCollisionTriangle<KDOP_IDX_TYPE> >& BuildTriangles)
	{
		// Empty the current set of nodes and preallocate the memory so it doesn't
		// reallocate memory while we are recursively walking the tree
		Nodes.Empty(BuildTriangles.Num() * 2);
		// Add the root node
		Nodes.Add();
		// Now tell that node to recursively subdivide the entire set of triangles
		Nodes(0).SplitTriangleList(0,BuildTriangles.Num(),BuildTriangles,Nodes);
		// Don't waste memory.
		Nodes.Shrink();
		// Copy over the triangle information afterward, since they will have
		// been sorted into their bounding volumes at this point
		Triangles.Empty(BuildTriangles.Num());
		Triangles.Add(BuildTriangles.Num());
		// Copy the triangles from the build list into the full list
		for (INT nIndex = 0; nIndex < BuildTriangles.Num(); nIndex++)
		{
			Triangles(nIndex) = BuildTriangles(nIndex);
		}
	}

	/**
	 * Figures out whether the check even hits the root node's bounding volume. If
	 * it does, it recursively searches for a triangle to hit.
	 *
	 * @param Check -- The aggregated line check data
	 */
	UBOOL LineCheck(TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		UBOOL bHit = FALSE;
		FLOAT HitTime;
		// Check against the first bounding volume and decide whether to go further
		if (Nodes(0).BoundingVolume.LineCheck(Check,HitTime))
		{
			// Recursively check for a hit
			bHit = Nodes(0).LineCheck(Check);
		}
		return bHit;
	}

	/**
	 * Figures out whether the check even hits the root node's bounding volume. If
	 * it does, it recursively searches for a triangle to hit.
	 *
	 * @param Check -- The aggregated box check data
	 */
	UBOOL BoxCheck(TkDOPBoxCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		UBOOL bHit = FALSE;
		FLOAT HitTime;
		// Check the root node's bounding volume expanded by the extent
		kDOPType kDOP(Nodes(0).BoundingVolume,Check.LocalExtent);
		// Check against the first bounding volume and decide whether to go further
		if (kDOP.LineCheck(Check,HitTime))
		{
			// Recursively check for a hit
			bHit = Nodes(0).BoxCheck(Check);
		}
		return bHit;
	}

	/**
	 * Figures out whether the check even hits the root node's bounding volume. If
	 * it does, it recursively searches for a triangle to hit.
	 *
	 * @param Check -- The aggregated point check data
	 */
	UBOOL PointCheck(TkDOPPointCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Check) const
	{
		UBOOL bHit = FALSE;
		// Check the root node's bounding volume expanded by the extent
		kDOPType kDOP(Nodes(0).BoundingVolume,Check.LocalExtent);
		// Check against the first bounding volume and decide whether to go further
		if (kDOP.PointCheck(Check))
		{
			// Recursively check for a hit
			bHit = Nodes(0).PointCheck(Check);
		}
		return bHit;
	}

	/**
	 * Find all triangles in static mesh that overlap a supplied bounding sphere.
	 *
	 * @param Query -- The aggregated sphere query data
	 */
	void SphereQuery(TkDOPSphereQuery<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Query) const
	{
		// Check the query box overlaps the root node KDOP. If so, run query recursively.
		if( Query.Nodes(0).BoundingVolume.AABBOverlapCheck( Query.LocalBox ) )
		{
			Query.Nodes(0).SphereQuery( Query );
		}
	}

	/**
	 * Find all kdop nodes in static mesh that overlap given frustum.
	 * This is just the entry point to the tree
	 *
	 * @param Query -- The aggregated frustum query data
	 */
	UBOOL FrustumQuery( TkDOPFrustumQuery<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Query ) const
	{
		if ( Query.Nodes.Num() > 1 )
		{
			// Begin at the root.
			Query.Nodes(0).FrustumQuery( Query );
		}
		else if( Query.Nodes.Num() == 1 )
		{
			// Special case for when the kDOP tree contains only a single node (and thus that node is a leaf).
			if( Query.Nodes(0).BoundingVolume.FrustumCheck(Query.LocalFrustumPlanes) )
			{
				Query.AddLeafIndex( 0 );
			}
		}

		// The query was successful only if there were intersecting nodes.
		return Query.ReturnLeaves.Num() > 0;
	}

	// Serialization
	friend FArchive& operator<<(FArchive& Ar,TkDOPTree<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Tree)
	{
		Tree.Nodes.BulkSerialize( Ar );
		Tree.Triangles.BulkSerialize( Ar );
		return Ar;
	}
};

/**
 * Base struct for all collision checks. Holds a reference to the collision
 * data provider, which is a struct that abstracts out the access to a
 * particular mesh/primitives data
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPCollisionCheck
{
	/** Exposes data provider type to clients. */
	typedef COLL_DATA_PROVIDER DataProviderType;

	/** Exposes node type to clients. */
	typedef TkDOPNode<DataProviderType,KDOP_IDX_TYPE> NodeType;

	/** Exposes tree type to clients. */
	typedef TkDOPTree<DataProviderType,KDOP_IDX_TYPE> TreeType;

	/** Exposes kDOP type to clients. */
	typedef typename NodeType::kDOPType	kDOPType;

	/**
	 * Used to get access to local->world, vertices, etc. without using virtuals
	 */
	const DataProviderType& CollDataProvider;
	/**
	 * The kDOP tree
	 */
	const TreeType& kDOPTree;
	/**
	 * The array of the nodes for the kDOP tree
	 */
	const TArray<NodeType>& Nodes;
	/**
	 * The collision triangle data for the kDOP tree
	 */
	const TArray<FkDOPCollisionTriangle<KDOP_IDX_TYPE> >& CollisionTriangles;

	/**
	 * Hide the default ctor
	 */
	TkDOPCollisionCheck(const DataProviderType& InCollDataProvider) :
		CollDataProvider(InCollDataProvider),
		kDOPTree(CollDataProvider.GetkDOPTree()),
		Nodes(kDOPTree.Nodes),
		CollisionTriangles(kDOPTree.Triangles)
	{
	}

};

/**
 * This struct holds the information used to do a line check against the kDOP
 * tree. The collision provider gives access to various matrices, vertex data
 * etc. without having to use virtual functions.
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPLineCollisionCheck :
	public TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>
{
	/**
	 * Where the collision results get stored
	 */
	FCheckResult* Result;
	// Constant input vars
	const FVector& Start;
	const FVector& End;
	/**
	 * Flags for optimizing a trace
	 */
	const DWORD TraceFlags;
	// Locally calculated vectors
	FVector LocalStart;
	FVector LocalEnd;
	FVector LocalDir;
	FVector LocalOneOverDir;
	// Normal in local space which gets transformed to world at the very end
	FVector LocalHitNormal;

	/**
	 * Sets up the FkDOPLineCollisionCheck structure for performing line checks
	 * against a kDOPTree. Initializes all of the variables that are used
	 * throughout the line check.
	 *
	 * @param InStart -- The starting point of the trace
	 * @param InEnd -- The ending point of the trace
	 * @param InTraceFlags -- The trace flags that might modify searches
	 * @param InCollDataProvider -- The struct that provides access to mesh/primitive
	 *		specific data, such as L2W, W2L, Vertices, and so on
	 * @param InResult -- The out param for hit result information
	 */
	TkDOPLineCollisionCheck(const FVector& InStart,const FVector& InEnd,
		DWORD InTraceFlags,const COLL_DATA_PROVIDER& InCollDataProvider,
		FCheckResult* InResult) :
		TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>(InCollDataProvider),
		Result(InResult), Start(InStart), End(InEnd), TraceFlags(InTraceFlags)
	{
		const FMatrix& WorldToLocal = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal();
		// Move start and end to local space
		LocalStart = WorldToLocal.TransformFVector(Start);
		LocalEnd = WorldToLocal.TransformFVector(End);
		// Calculate the vector's direction in local space
		LocalDir = LocalEnd - LocalStart;
		// Build the one over dir
		LocalOneOverDir.X = LocalDir.X ? 1.f / LocalDir.X : 0.f;
		LocalOneOverDir.Y = LocalDir.Y ? 1.f / LocalDir.Y : 0.f;
		LocalOneOverDir.Z = LocalDir.Z ? 1.f / LocalDir.Z : 0.f;
		// Clear the closest hit time
		Result->Time = MAX_FLT;
	}

	/**
	 * Transforms the local hit normal into a world space normal using the transpose
	 * adjoint and flips the normal if need be
	 */
	FORCEINLINE FVector GetHitNormal(void)
	{
		// Transform the hit back into world space using the transpose adjoint
		FVector Normal = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetLocalToWorldTransposeAdjoint().TransformNormal(LocalHitNormal).SafeNormal();
		// Flip the normal if the triangle is inverted
		if (TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetDeterminant() < 0.f)
		{
			Normal = -Normal;
		}
		return Normal;
	}
};

/**
 * This struct holds the information used to do a box and/or point check
 * against the kDOP tree. It is used purely to gather multiple function
 * parameters into a single structure for smaller stack overhead.
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPBoxCollisionCheck :
	public TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>
{
	// Constant input vars
	const FVector& Extent;
	// Calculated vars
	FVector LocalExtent;
	FVector LocalBoxX;
	FVector LocalBoxY;
	FVector LocalBoxZ;

	/**
	 * Sets up the TkDOPBoxCollisionCheck structure for performing swept box checks
	 * against a kDOPTree. Initializes all of the variables that are used
	 * throughout the check.
	 *
	 * @param InStart -- The starting point of the trace
	 * @param InEnd -- The ending point of the trace
	 * @param InExtent -- The extent to check
	 * @param InTraceFlags -- The trace flags that might modify searches
	 * @param InCollDataProvider -- The struct that provides access to mesh/primitive
	 *		specific data, such as L2W, W2L, Vertices, and so on
	 * @param InResult -- The out param for hit result information
	 */
	TkDOPBoxCollisionCheck(const FVector& InStart,const FVector& InEnd,
		const FVector& InExtent,DWORD InTraceFlags,
		const COLL_DATA_PROVIDER& InCollDataProvider,FCheckResult* InResult) :
		TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>(InStart,InEnd,InTraceFlags,InCollDataProvider,InResult),
		Extent(InExtent)
	{
		// Move extent to local space
		LocalExtent = FBox(-Extent,Extent).TransformBy(TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal()).GetExtent();
		// Transform the PlaneNormals into local space.
		LocalBoxX = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal().TransformNormal(FkDOPPlanes::PlaneNormals[0]);
		LocalBoxY = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal().TransformNormal(FkDOPPlanes::PlaneNormals[1]);
		LocalBoxZ = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal().TransformNormal(FkDOPPlanes::PlaneNormals[2]);
	}
};

/**
 * This struct holds the information used to do a point check against the kDOP
 * tree. It is used purely to gather multiple function parameters into a
 * single structure for smaller stack overhead.
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPPointCollisionCheck :
	public TkDOPBoxCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>
{
	// Holds the minimum pentration distance for push out calculations
	FLOAT BestDistance;
	FVector LocalHitLocation;

	/**
	 * Sets up the TkDOPPointCollisionCheck structure for performing point checks
	 * (point plus extent) against a kDOPTree. Initializes all of the variables
	 * that are used throughout the check.
	 *
	 * @param InLocation -- The point to check for intersection
	 * @param InExtent -- The extent to check
	 * @param InCollDataProvider -- The struct that provides access to mesh/primitive
	 *		specific data, such as L2W, W2L, Vertices, and so on
	 * @param InResult -- The out param for hit result information
	 */
	TkDOPPointCollisionCheck(const FVector& InLocation,const FVector& InExtent,
		const COLL_DATA_PROVIDER& InCollDataProvider,FCheckResult* InResult) :
		TkDOPBoxCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>(InLocation,InLocation,InExtent,0,InCollDataProvider,InResult),
		BestDistance(100000.f)
	{
	}

	/**
	 * Returns the transformed hit location
	 */
	FORCEINLINE FVector GetHitLocation(void)
	{
		// Push out the hit location from the point along the hit normal and
		// convert into world units
		return TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetLocalToWorld().TransformFVector(
			TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::LocalStart + 
			TkDOPLineCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::LocalHitNormal * BestDistance);
	}
};

/**
 * Builds all of the data needed to find all triangles inside a sphere
 */
template <typename COLL_DATA_PROVIDER,typename KDOP_IDX_TYPE> struct TkDOPSphereQuery :
	public TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>
{
	FBox LocalBox;
	// Index of FkDOPCollisionTriangle in KDOPTree
	TArray<INT>& ReturnTriangles;

	/**
	 * Sets up the FkDOPSphereQuery structure for finding the set of triangles
	 * in the static mesh that overlap the give sphere.
	 *
	 * @param InSphere -- Sphere to query against 
	 * @param OutTriangles -- Array of collision triangles that overlap sphere
	 * @param InCollDataProvider -- The struct that provides access to mesh/primitive
	 *		specific data, such as L2W, W2L, Vertices, and so on
	 */
	TkDOPSphereQuery(const FSphere& InSphere,TArray<INT>& OutTriangles,
		const COLL_DATA_PROVIDER& InCollDataProvider) :
		TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>(InCollDataProvider),
		ReturnTriangles(OutTriangles)
	{
		// Find bounding box we are querying triangles against in local space.
		const FPlane& radiusVec(InSphere.W, InSphere.W, InSphere.W, 0);
		const FBox& WorldBox(InSphere - radiusVec, InSphere + radiusVec);
		LocalBox = WorldBox.TransformBy(TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal());
	}
};

/**
 * Sets up all of the data needed to find all triangles in a given frustum
 */
template <typename COLL_DATA_PROVIDER, typename KDOP_IDX_TYPE> struct TkDOPFrustumQuery :
	public TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>
{
	// frustum planes in local space
	TArray<FPlane> LocalFrustumPlanes;
	// indices for kdop node leaves that intersect the frustum
	TArray<KDOP_IDX_TYPE>& ReturnLeaves;
	// number of triangles in the ReturnLeaves
	INT ReturnNumTris;

	/**
	 * Sets up the TkDOPFrustumQuery structure for finding the set of nodes
	 * in the static mesh that overlap the given frustum planes.
	 *
	 * @param InFrustumPlanes -- planes to check against
	 * @param InNumFrustumPlanes -- number of FPlanes in InFrustumPlanes
	 * @param OutLeaves -- Array of indices to kdop node leaves intersected
	 * @param InCollDataProvider -- The struct that provides access to mesh/primitive
	 *		specific data, such as L2W, W2L, Vertices, and so on
	 */
	TkDOPFrustumQuery(FPlane* InFrustumPlanes,INT InNumFrustumPlanes,
		TArray<KDOP_IDX_TYPE>& OutLeaves,const COLL_DATA_PROVIDER& InCollDataProvider) :
		TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>(InCollDataProvider),
		ReturnLeaves( OutLeaves ),
		ReturnNumTris(0)
	{
		// matrices needed to go from world to local space
		const FMatrix& ToLocalMat = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetWorldToLocal();
		const FMatrix& ToLocalMatTA = ToLocalMat.TransposeAdjoint();
		const FLOAT Determinant = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::CollDataProvider.GetDeterminant();

		LocalFrustumPlanes.Add( InNumFrustumPlanes );
		for( INT PlaneIdx=0; PlaneIdx < InNumFrustumPlanes; PlaneIdx++ )
		{
			// the TransformByUsingAdjointT just transforms the plane normal (X,Y,Z) by using the
			// transpose adjoint of the w2l matrix (ToLocalMatTA).  Then it transforms a point on
			// the plane W*(X,Y,Z) by the w2l matrix (ToLocalMat). Then the new W (distance from 
			// plane to origin) can be derived.
			LocalFrustumPlanes(PlaneIdx) = InFrustumPlanes[PlaneIdx].TransformByUsingAdjointT( ToLocalMat, Determinant, ToLocalMatTA );        
		}
	}

	FORCEINLINE void AddLeafIndex( KDOP_IDX_TYPE LeafIndex )
	{
		const TkDOPNode<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>& Node = TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::Nodes(LeafIndex);
		checkSlow( Node.bIsLeaf );
		ReturnLeaves.AddItem( LeafIndex );
		ReturnNumTris += TkDOPCollisionCheck<COLL_DATA_PROVIDER,KDOP_IDX_TYPE>::Nodes(LeafIndex).t.NumTriangles;
	}
};
#endif
