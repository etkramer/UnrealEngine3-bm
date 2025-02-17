/*=============================================================================
	StaticMeshDrawList.h: Static mesh draw list definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __STATICMESHDRAWLIST_H__
#define __STATICMESHDRAWLIST_H__

/**
 * A set of static meshs, each associated with a mesh drawing policy of a particular type.
 * @param DrawingPolicyType - The drawing policy type used to draw mesh in this draw list.
 * @param HashSize - The number of buckets to use in the drawing policy hash.
 */
template<typename DrawingPolicyType>
class TStaticMeshDrawList
{
public:
	typedef typename DrawingPolicyType::ElementDataType ElementPolicyDataType;

private:

	/** A handle to an element in the draw list.  Used by FStaticMesh to keep track of draw lists containing the mesh. */
	class FElementHandle : public FStaticMesh::FDrawListElementLink
	{
	public:

		/** Initialization constructor. */
		FElementHandle(TStaticMeshDrawList* InStaticMeshDrawList,FSetElementId InSetId,INT InElementIndex):
		  StaticMeshDrawList(InStaticMeshDrawList)
		  ,SetId(InSetId)
		  ,ElementIndex(InElementIndex)
		{
		}

		// FAbstractDrawListElementLink interface.
		virtual void Remove();

	private:
		TStaticMeshDrawList* StaticMeshDrawList;
		FSetElementId SetId;
		INT ElementIndex;
	};

	/**
	 * This structure stores the info needed for visibility culling a static mesh element.
	 * Stored separately to avoid bringing the other info about non-visible meshes into the cache.
	 */
	struct FElementCompact
	{
		FRelativeBitReference VisibilityBitReference;
		FElementCompact() {}
		FElementCompact(INT MeshId)
		: VisibilityBitReference(MeshId)
		{}
	};

	struct FElement
	{
		FStaticMesh* Mesh;
		ElementPolicyDataType PolicyData;
		TRefCountPtr<FElementHandle> Handle;

		/** Default constructor. */
		FElement():
			Mesh(NULL)
		{}

		/** Minimal initialization constructor. */
		FElement(
			FStaticMesh* InMesh,
			const ElementPolicyDataType& InPolicyData,
			TStaticMeshDrawList* StaticMeshDrawList,
			FSetElementId SetId,
			INT ElementIndex
			):
			Mesh(InMesh),
			PolicyData(InPolicyData),
			Handle(new FElementHandle(StaticMeshDrawList,SetId,ElementIndex))
		{}

		/** Destructor. */
		~FElement()
		{
			if(Mesh)
			{
				Mesh->UnlinkDrawList(Handle);
			}
		}
	};

	/** A set of draw list elements with the same drawing policy. */
	struct FDrawingPolicyLink
	{
		TArray<FElementCompact>		CompactElements;    // The elements array and the compact elements array are always synchronized
		TArray<FElement>			Elements;
		DrawingPolicyType			DrawingPolicy;
		FBoundShaderStateRHIRef		BoundShaderState;

		/** The id of this link in the draw list's set of drawing policy links. */
		FSetElementId SetId;

		TStaticMeshDrawList* DrawList;

		/** Initialization constructor. */
		FDrawingPolicyLink(TStaticMeshDrawList* InDrawList,const DrawingPolicyType& InDrawingPolicy):
			DrawingPolicy(InDrawingPolicy),
			DrawList(InDrawList)
		{
			BoundShaderState = DrawingPolicy.CreateBoundShaderState();
		}
	};

	/** Functions to extract the drawing policy from FDrawingPolicyLink as a key for TSet. */
	struct FDrawingPolicyKeyFuncs : BaseKeyFuncs<FDrawingPolicyLink,DrawingPolicyType>
	{
		static const DrawingPolicyType& GetSetKey(const FDrawingPolicyLink& Link)
		{
			return Link.DrawingPolicy;
		}

		static UBOOL Matches(const DrawingPolicyType& A,const DrawingPolicyType& B)
		{
			return A.Matches(B);
		}

		static DWORD GetKeyHash(const DrawingPolicyType& DrawingPolicy)
		{
			return DrawingPolicy.GetTypeHash();
		}
	};

	/**
	* Submits the draw calls.  Used by Draw() after the mesh has been determined to be visible
	* @param View - The view of the meshes to render.
	* @param Element - The mesh element
	* @param DrawingPolicyLink - the drawing policy link
	* @param bDrawnShared - determines whether to draw shared 
	*/
	void SubmitDrawCall(const FViewInfo& View, const FElement& Element, const FDrawingPolicyLink* DrawingPolicyLink, UBOOL &bDrawnShared) const;

public:

	/**
	 * Adds a mesh to the draw list.
	 * @param Mesh - The mesh to add.
	 * @param PolicyData - The drawing policy data for the mesh.
	 * @param InDrawingPolicy - The drawing policy to use to draw the mesh.
	 */
	void AddMesh(
		FStaticMesh* Mesh,
		const ElementPolicyDataType& PolicyData,
		const DrawingPolicyType& InDrawingPolicy
		);

	/**
	 * Removes all meshes from the draw list.
	 */
	void RemoveAllMeshes();

	/**
	 * Draws all elements of the draw list.
	 * @param CI - The command interface to the execute the draw commands on.
	 * @param View - The view of the meshes to render.
	 * @return True if any static meshes were drawn.
	 */
	UBOOL DrawAll(const FViewInfo& View) const;

	/**
	 * Draws only the static meshes which are in the visibility map.
	 * @param CI - The command interface to the execute the draw commands on.
	 * @param View - The view of the meshes to render.
	 * @param StaticMeshVisibilityMap - An map from FStaticMesh::Id to visibility state.
	 * @return True if any static meshes were drawn.
	 */
	UBOOL DrawVisible(const FViewInfo& View, const TBitArray<SceneRenderingBitArrayAllocator>& StaticMeshVisibilityMap) const;

	/**
	 * @return total number of meshes in all draw policies
	 */
	INT NumMeshes() const;

private:
	/** All drawing policies in the draw list, in rendering order. */
    TArray<FSetElementId> OrderedDrawingPolicies;

	/** All drawing policy element sets in the draw list, hashed by drawing policy. */
	TSet<FDrawingPolicyLink,FDrawingPolicyKeyFuncs> DrawingPolicySet;
};

#include "StaticMeshDrawList.inl"

#endif
