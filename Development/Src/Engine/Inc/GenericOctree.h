/*=============================================================================
	GenericOctree.h: Generic octree definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GENERIC_OCTREE_H__
#define __GENERIC_OCTREE_H__

/** A concise iteration over the children of an octree node. */
#define FOREACH_OCTREE_CHILD_NODE(ChildRef) \
	for(FOctreeChildNodeRef ChildRef(0);!ChildRef.IsNULL();ChildRef.Advance())

/** An unquantized bounding box. */
class FBoxCenterAndExtent
{
public:
	FVector4 Center;
	FVector4 Extent;

	/** Default constructor. */
	FBoxCenterAndExtent() {}

	/** Initialization constructor. */
	FBoxCenterAndExtent(const FVector& InCenter,const FVector& InExtent)
	:	Center(InCenter,0)
	,	Extent(InExtent,0)
	{}

	/** FBox conversion constructor. */
	FBoxCenterAndExtent(const FBox& Box)
	{
		Box.GetCenterAndExtents((FVector&)Center,(FVector&)Extent);
		Center.W = Extent.W = 0;
	}

	/** FBoxSphereBounds conversion constructor. */
	FBoxCenterAndExtent(const FBoxSphereBounds& BoxSphere)
	{
		Center = BoxSphere.Origin;
		Extent = BoxSphere.BoxExtent;
		Center.W = Extent.W = 0;
	}

	/** Converts to a FBox. */
	FBox GetBox() const
	{
		return FBox(Center - Extent,Center + Extent);
	}

	/**
	 * Determines whether two boxes intersect.
	 * @return TRUE if the boxes intersect, or FALSE.
	 */
	friend FORCEINLINE UBOOL Intersect(const FBoxCenterAndExtent& A,const FBoxCenterAndExtent& B)
	{
		// CenterDifference is the vector between the centers of the bounding boxes.
		const VectorRegister CenterDifference = VectorAbs(VectorSubtract(VectorLoadAligned(&A.Center),VectorLoadAligned(&B.Center)));

		// CompositeExtent is the extent of the bounding box which is the convolution of A with B.
		const VectorRegister CompositeExtent = VectorAdd(VectorLoadAligned(&A.Extent),VectorLoadAligned(&B.Extent));

		// For each axis, the boxes intersect on that axis if the projected distance between their centers is less than the sum of their
		// extents.  If the boxes don't intersect on any of the axes, they don't intersect.
		return VectoryAnyGreaterThan(CenterDifference,CompositeExtent) == FALSE;
	}
};

/** A reference to a child of an octree node. */
class FOctreeChildNodeRef
{
public:

	union
	{
		struct
		{
			BITFIELD X : 1;
			BITFIELD Y : 1;
			BITFIELD Z : 1;
			BITFIELD bNULL : 1;
		};
		BITFIELD Index : 3;
	};

	/** Initialization constructor. */
	FOctreeChildNodeRef(INT InX,INT InY,INT InZ)
	:	X(InX)
	,	Y(InY)
	,	Z(InZ)
	,	bNULL(FALSE)
	{}

	/** Initialized the reference with a child index. */
	FOctreeChildNodeRef(INT InIndex = 0)
	:	bNULL(FALSE)
	,	Index(InIndex)
	{}

	/** Advances the reference to the next child node.  If this was the last node remain, sets bInvalid=TRUE. */
	FORCEINLINE void Advance()
	{
		if(Index < 7)
		{
			++Index;
		}
		else
		{
			bNULL = TRUE;
		}
	}

	/** @return TRUE if the reference isn't set. */
	FORCEINLINE UBOOL IsNULL() const
	{
		return bNULL;
	}
};

/** A subset of an octree node's children that intersect a bounding box. */
class FOctreeChildNodeSubset
{
public:

	union
	{
		struct 
		{
			BITFIELD bPositiveX : 1;
			BITFIELD bPositiveY : 1;
			BITFIELD bPositiveZ : 1;
			BITFIELD bNegativeX : 1;
			BITFIELD bNegativeY : 1;
			BITFIELD bNegativeZ : 1;
		};

		struct
		{
			/** Only the bits for the children on the positive side of the splits. */
			BITFIELD PositiveChildBits : 3;

			/** Only the bits for the children on the negative side of the splits. */
			BITFIELD NegativeChildBits : 3;
		};

		/** All the bits corresponding to the child bits. */
		BITFIELD ChildBits : 6;

		/** All the bits used to store the subset. */
		BITFIELD AllBits;
	};

	/** Initializes the subset to be empty. */
	FOctreeChildNodeSubset()
	:	AllBits(0)
	{}

	/** Initializes the subset to contain a single node. */
	FOctreeChildNodeSubset(FOctreeChildNodeRef ChildRef)
	:	AllBits(0)
	{
		// The positive child bits correspond to the child index, and the negative to the NOT of the child index.
		PositiveChildBits = ChildRef.Index;
		NegativeChildBits = ~ChildRef.Index;
	}

	/** Determines whether the subset contains a specific node. */
	UBOOL Contains(FOctreeChildNodeRef ChildRef) const;
};

/** The context of an octree node, derived from the traversal of the tree. */
class FOctreeNodeContext
{
public:

	/** The node bounds are expanded by their extent divided by LoosenessDenominator. */
	enum { LoosenessDenominator = 16 };

	/** The bounds of the node. */
	FBoxCenterAndExtent Bounds;

	/** The extent of the node's children. */
	FLOAT ChildExtent;

	/** The offset of the childrens' centers from the center of this node. */
	FLOAT ChildCenterOffset;

	/** Default constructor. */
	FOctreeNodeContext()
	{}

	/** Initialization constructor. */
	FOctreeNodeContext(const FBoxCenterAndExtent& InBounds)
	:	Bounds(InBounds)
	{
		// A child node's tight extents are half its parent's extents, and its loose extents are expanded by 1/LoosenessDenominator.
		const FLOAT TightChildExtent = Bounds.Extent.X * 0.5f;
		const FLOAT LooseChildExtent = TightChildExtent * (1.0f + 1.0f / (FLOAT)LoosenessDenominator);

		ChildExtent = LooseChildExtent;
		ChildCenterOffset = Bounds.Extent.X - LooseChildExtent;
	}

	/** Child node initialization constructor. */
	FORCEINLINE FOctreeNodeContext GetChildContext(FOctreeChildNodeRef ChildRef) const
	{
		return FOctreeNodeContext(FBoxCenterAndExtent(
				FVector(
					Bounds.Center.X + ChildCenterOffset * (-1.0f + 2 * ChildRef.X),
					Bounds.Center.Y + ChildCenterOffset * (-1.0f + 2 * ChildRef.Y),
					Bounds.Center.Z + ChildCenterOffset * (-1.0f + 2 * ChildRef.Z)
					),
				FVector(
					ChildExtent,
					ChildExtent,
					ChildExtent
					)
				));
	}
	
	/**
	 * Determines which of the octree node's children intersect with a bounding box.
	 * @param BoundingBox - The bounding box to check for intersection with.
	 * @return A subset of the children's nodes that intersect the bounding box.
	 */
	FOctreeChildNodeSubset GetIntersectingChildren(const FBoxCenterAndExtent& BoundingBox) const;

	/**
	 * Determines which of the octree node's children contain the whole bounding box, if any.
	 * @param BoundingBox - The bounding box to check for intersection with.
	 * @return The octree's node that the bounding box is farthest from the outside edge of, or an invalid node ref if it's not contained
	 *			by any of the children.
	 */
	FOctreeChildNodeRef GetContainingChild(const FBoxCenterAndExtent& BoundingBox) const;
};

/** An octree. */
template<typename ElementType,typename OctreeSemantics>
class TOctree
{
public:

	typedef TArray<ElementType,TInlineAllocator<OctreeSemantics::MaxElementsPerLeaf> > ElementArrayType;
	typedef typename ElementArrayType::TConstIterator ElementConstIt;

	/** A node in the octree. */
	class FNode
	{
	public:

		friend class TOctree;

		/** Initialization constructor. */
		FNode(const FNode* InParent)
		:	Parent(InParent)
		,	InclusiveNumElements(0)
		,	bIsLeaf(TRUE)
		{
			FOREACH_OCTREE_CHILD_NODE(ChildRef)
			{
				Children[ChildRef.Index] = NULL;
			}
		}

		/** Destructor. */
		~FNode()
		{
			FOREACH_OCTREE_CHILD_NODE(ChildRef)
			{
				delete Children[ChildRef.Index];
			}
		}

		// Accessors.
		FORCEINLINE ElementConstIt GetElementIt() const { return ElementConstIt(Elements); }
		FORCEINLINE UBOOL IsLeaf() const { return bIsLeaf; }
		FORCEINLINE UBOOL HasChild(FOctreeChildNodeRef ChildRef) const
		{
			return Children[ChildRef.Index] != NULL && Children[ChildRef.Index]->InclusiveNumElements > 0;
		}
		FORCEINLINE FNode* GetChild(FOctreeChildNodeRef ChildRef) const
		{
			return Children[ChildRef.Index];
		}
		FORCEINLINE INT GetElementCount() const
		{
			return Elements.Num();
		}

	private:

		/** The elements in this node. */
		mutable ElementArrayType Elements;

		/** The parent of this node. */
		const FNode* Parent;

		/** The children of the node. */
		mutable FNode* Children[8];

		/** The number of elements contained by the node and its child nodes. */
		mutable BITFIELD InclusiveNumElements : 31;

		/** TRUE if the meshes should be added directly to the node, rather than subdividing when possible. */
		mutable BITFIELD bIsLeaf : 1;
	};



	/** A reference to an octree node, its context, and a read lock. */
	class FNodeReference
	{
	public:

		const FNode* Node;
		FOctreeNodeContext Context;

		/** Default constructor. */
		FNodeReference():
			Node(NULL),
			Context()
		{}

		/** Initialization constructor. */
		FNodeReference(const FNode* InNode,const FOctreeNodeContext& InContext):
			Node(InNode),
			Context(InContext)
		{}
	};	

	/** The default iterator allocator gives the stack enough inline space to contain a path and its siblings from root to leaf. */
	typedef TInlineAllocator<7 * (14 - 1) + 8> DefaultStackAllocator;

	/** An octree node iterator. */
	template<typename StackAllocator = DefaultStackAllocator>
	class TConstIterator
	{
	public:

		/** Pushes a child of the current node onto the stack of nodes to visit. */
		void PushChild(FOctreeChildNodeRef ChildRef)
		{
			NodeStack.AddItem(
				FNodeReference(
					CurrentNode.Node->GetChild(ChildRef),
					CurrentNode.Context.GetChildContext(ChildRef)
					));
		}

		/** Iterates to the next node. */
		void Advance()
		{
			if(NodeStack.Num())
			{
				CurrentNode = NodeStack(NodeStack.Num() - 1);
				NodeStack.Remove(NodeStack.Num() - 1);
			}
			else
			{
				CurrentNode = FNodeReference();
			}
		}

		/** Checks if there are any nodes left to iterate over. */
		UBOOL HasPendingNodes() const
		{
			return CurrentNode.Node != NULL;
		}

		/** Starts iterating at the root of an octree. */
		TConstIterator(const TOctree& Tree)
		:	CurrentNode(FNodeReference(&Tree.RootNode,Tree.RootNodeContext))
		{}

		/** Starts iterating at a particular node of an octree. */
		TConstIterator(const FNode& Node,const FOctreeNodeContext& Context):
			CurrentNode(FNodeReference(&Node,Context))
		{}

		// Accessors.
		const FNode& GetCurrentNode() const
		{
			return *CurrentNode.Node;
		}
		const FOctreeNodeContext& GetCurrentContext() const
		{
			return CurrentNode.Context;
		}

	private:

		/** The node that is currently being visited. */
		FNodeReference CurrentNode;
	
		/** The nodes which are pending iteration. */
		TArray<FNodeReference,StackAllocator> NodeStack;
	};

	/** Iterates over the elements in the octree that intersect a bounding box. */
	template<typename StackAllocator = DefaultStackAllocator>
	class TConstElementBoxIterator
	{
	public:

		/** Iterates to the next element. */
		void Advance()
		{
			++ElementIt;
			AdvanceToNextIntersectingElement();
		}

		/** Checks if there are any elements left to iterate over. */
		UBOOL HasPendingElements() const
		{
			return NodeIt.HasPendingNodes();
		}

		/** Initialization constructor. */
		TConstElementBoxIterator(const TOctree& Tree,const FBoxCenterAndExtent& InBoundingBox)
		:	IteratorBounds(InBoundingBox)
		,	NodeIt(Tree)
		,	ElementIt(Tree.RootNode.GetElementIt())
		{
			ProcessChildren();
			AdvanceToNextIntersectingElement();
		}

		// Accessors.
		const ElementType& GetCurrentElement() const
		{
			return *ElementIt;
		}

	private:

		/** The bounding box to check for intersection with. */
		FBoxCenterAndExtent IteratorBounds;

		/** The octree node iterator. */
		TConstIterator<StackAllocator> NodeIt;

		/** The element iterator for the current node. */
		ElementConstIt ElementIt;

		/** Processes the children of the current node. */
		void ProcessChildren()
		{
			// Add the child nodes that intersect the bounding box to the node iterator's stack.
			const FNode& CurrentNode = NodeIt.GetCurrentNode();
			const FOctreeNodeContext& Context = NodeIt.GetCurrentContext();
			const FOctreeChildNodeSubset IntersectingChildSubset = Context.GetIntersectingChildren(IteratorBounds);
			FOREACH_OCTREE_CHILD_NODE(ChildRef)
			{
				if(IntersectingChildSubset.Contains(ChildRef) && CurrentNode.HasChild(ChildRef))
				{
					NodeIt.PushChild(ChildRef);
				}
			}
		}

		/** Advances the iterator to the next intersecting primitive, starting at a primitive in the current node. */
		void AdvanceToNextIntersectingElement()
		{
			// Keep trying elements until we find one that intersects or run out of elements to try.
			while(NodeIt.HasPendingNodes())
			{
				// Check if we've advanced past the elements in the current node.
				if(ElementIt)
				{
					// Check if the current element intersects the bounding box.
					if(Intersect(OctreeSemantics::GetBoundingBox(*ElementIt),IteratorBounds))
					{
						// If it intersects, break out of the advancement loop.
						break;
					}
					else
					{
						// If it doesn't intersect, skip to the next element.
						++ElementIt;
					}
				}
				else
				{
					// Advance to the next node.
					NodeIt.Advance();
					if(NodeIt.HasPendingNodes())
					{
						ProcessChildren();

						// The element iterator can't be assigned to, but it can be replaced by Move.
						Move(ElementIt,NodeIt.GetCurrentNode().GetElementIt());
					}
				}
			};
		}
	};

	/**
	 * Adds an element to the octree.
	 * @param Element - The element to add.
	 * @return An identifier for the element in the octree.
	 */
	void AddElement(typename TContainerTraits<ElementType>::ConstInitType Element);

	/**
	 * Removes an element from the octree.
	 * @param ElementId - The element to remove from the octree.
	 */
	void RemoveElement(FOctreeElementId ElementId);

	/** Accesses an octree element by ID. */
	ElementType& GetElementById(FOctreeElementId ElementId);

	/** Writes stats for the octree to the log. */
	void DumpStats();

	/** Initialization constructor. */
	TOctree(const FVector& InOrigin,FLOAT InExtent);

private:

	/** The octree's root node. */
	FNode RootNode;

	/** The octree's root node's context. */
	FOctreeNodeContext RootNodeContext;

	/** The extent of a leaf at the maximum allowed depth of the tree. */
	FLOAT MinLeafExtent;

	/** Adds an element to a node or its children. */
	void AddElementToNode(
		typename TContainerTraits<ElementType>::ConstInitType Element,
		const FNode& InNode,
		const FOctreeNodeContext& InContext
		);
};

#include "GenericOctree.inl"

#endif
