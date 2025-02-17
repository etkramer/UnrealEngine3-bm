//------------------------------------------------------------------------------
// This class implements the FaceFx Face Graph (Directed Acyclic Graph).
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraph_H__
#define FxFaceGraph_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxObject.h"
#include "FxFaceGraphNode.h"
#include "FxBonePoseNode.h"
#include "FxCombinerNode.h"
#include "FxGenericTargetNode.h"
#include "FxStringHash.h"

namespace OC3Ent
{

namespace Face
{

/// Several error codes that can be returned by FxFaceGraph::Link().
enum FxLinkErrorCode
{
	LEC_None      = 0, ///< No Error.
	LEC_Invalid   = 1, ///< The link is invalid.
	LEC_Duplicate = 2, ///< The link would result in a duplicate link in the %Face Graph.
	LEC_Cycle     = 3, ///< The link would result in a cycle in the %Face Graph.
	LEC_Refused   = 4  ///< The link was refused by the toNode.
};

/// A directed acyclic graph consisting of %Face Graph nodes and links.
/// \ingroup faceGraph
class FxFaceGraph : public FxObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxFaceGraph, FxObject)
public:
	// Forward declarations.
	class Iterator;

	/// Constructor.
	FxFaceGraph();
	/// Copy constructor.
	FxFaceGraph( const FxFaceGraph& other );
	/// Assignment operator.
	FxFaceGraph& operator=( const FxFaceGraph& other );
	/// Destructor.
	virtual ~FxFaceGraph();

	/// Removes all nodes from the %Face Graph and frees all associated memory.
	void Clear( void );

	/// The start of iteration over nodes in the %Face Graph.
	/// \param fgNodeClass The type of %Face Graph node over which to iterate.
	/// Use FxFaceGraphNode::StaticClass() to iterate over all nodes
	/// in the %Face Graph.
	/// \return An iterator to the first node of type fgNodeClass in the face 
	/// graph.
	/// \note fgNodeClass must be equal in matching calls to Begin() and End().
	/// You should never perform operations that may modify the %Face Graph 
	/// nodes (such as AddNode() or RemoveNode()) during a %Face Graph 
	/// iteration loop because those operations could invalidate the %Face Graph
	/// iterators.
	Iterator Begin( const FxClass* fgNodeClass ) const;
	/// The end of iteration over nodes in the %Face Graph.
	/// \param fgNodeClass The type of %Face Graph node over which to iterate.
	/// Use FxFaceGraphNode::StaticClass() to iterate over all nodes in the %Face Graph.
	/// \return An iterator representing the last node of fgNodeClass in the 
	/// %Face Graph.  There is no valid node in the "end" iterator, so calling 
	/// GetNode() on it is undefined and dereferencing the result of GetNode()
	/// will most likely crash.  
	/// \note fgNodeClass must be equal in matching calls to Begin() and End().
	/// You should never perform operations that may modify the %Face Graph 
	/// nodes (such as AddNode() or RemoveNode()) during a %Face Graph 
	/// iteration loop because those operations could invalidate the %Face Graph
	/// iterators.
	Iterator End( const FxClass* fgNodeClass ) const;

	/// Adds a node to the %Face Graph.
	/// \param pNode A pointer to the node to add to the %Face Graph.  
	/// This pointer \b must be created on the heap with operator \p new and 
	/// the instance of FxFaceGraph assumes control of the memory.
	/// \return \p FxFalse if a node with the same name already exists in the graph.  
	/// Otherwise the node is added and \p FxTrue is returned. 
	FxBool AddNode( FxFaceGraphNode* pNode );
	/// Removes a node from the %Face Graph.
	/// \param name The name of the node to remove.
	/// \return \p FxFalse if the node was not in the graph and wasn't removed, \p FxTrue 
	/// if the node was in the graph and removed.
	FxBool RemoveNode( const FxName& name );

	/// Finds the node named 'name' (of any type) and returns a pointer to it
	/// or \p NULL if the node is not found.
	FxFaceGraphNode* FindNode( const FxName& name );

	/// Renames a node in the %Face Graph.
	/// \param currName The current name of the node.
	/// \param newName The desired new name of the node.
	/// \return \p FxFalse if a node with \a currName does not exist in the 
	/// graph, or if a node with \a newName already exists in the graph. 
	/// Otherwise, the node is renamed and FxTrue is returned.
	FxBool RenameNode( const FxName& currName, const FxName newName );

	/// Returns the number of total nodes in the %Face Graph.  Considerably faster
	/// than CountNodes(), since the type can be ignored, and the very base list 
	/// simply queried for the number of objects it holds.
	FX_INLINE FxSize GetNumNodes( void ) const { return _nodes.Length(); }
	/// Counts a certain type of node in the %Face Graph.  
	/// This will use an iterator to count the nodes each time, so it could 
	/// become slow and as such should not be used frequently, especially in tight loops.  
	/// If no fgNodeClass is specified, all nodes will be counted, resulting in 
	/// the same result GetNumNodes() returns.
	/// \param fgNodeClass The node type to count
	/// \return The number of nodes of type \a fgNodeClass in the %Face Graph.
	FxSize CountNodes( const FxClass* fgNodeClass = FxFaceGraphNode::StaticClass() );
	
	/// Adds an input link to toNode from fromNode using the specified link function.
	/// \param toNode The node which receives input from \a fromNode
	/// \param fromNode The node which delivers output to \a toNode
	/// \param linkFn The name of the link function to modify the value going 
	/// from \a fromNode to \a toNode.
	/// \param linkFnParams The parameters to \a linkFn.
	/// \return Face::LEC_None if the link was made, Face::LEC_Invalid if
	/// the link could not be made because either the toNode or fromNode could
	/// not be found in the %Face Graph or the linkFn is invalid, Face::LEC_Duplicate
	/// if there already exists a link from toNode to fromNode, Face::LEC_Cycle
	/// if the link would result in a cycle being created in the %Face Graph, or 
	/// Face::LEC_Refused if the link would cause \a toNode to have too many inputs.
	FxLinkErrorCode Link( const FxName& toNode, const FxName& fromNode, 
		                  const FxName& linkFn, 
						  const FxLinkFnParameters& linkFnParams );

	/// Removes the link between the two nodes, if it exists.
	/// \param firstNodeName The name of the node on one side of a link.
	/// \param secondNodeName The name of the node on the other side of a link.
	/// \return \p FxTrue if the link existed and was removed or \p FxFalse if 
	/// the link did not exist, and thus could not be removed.
	FxBool Unlink( const FxName& firstNodeName, const FxName& secondNodeName );

	/// Gets information for the link in the between the given nodes. 
	/// \param[in] firstNodeName The name of the node on one side of a link.
	/// \param[in] secondNodeName The name of the node on the other side of a link.
	/// \param[out] linkInfo The link information for the link between the two nodes.
	/// \return FxTrue if a link exists, and FxFalse if a link doesn't
	/// exist, in which case the data in \a linkInfo should be ignored.
	/// \note The names may be passed in any order.  Since it is an acyclic graph,
	/// there may only by a single link between two nodes, and the direction may be
	/// determined given the names of the two nodes.
	FxBool GetLink( const FxName& firstNodeName, const FxName& secondNodeName, 
					FxFaceGraphNodeLink& linkInfo );
	/// Sets the information for the link in the %Face Graph between the given nodes.
	/// \param firstNodeName The name of the node on one side of a link.
	/// \param secondNodeName The name of the node on the other side of a link.
	/// \param linkInfo The link information for the link between the two nodes.
	/// \return \p FxTrue if a link exists and the information was set, 
	/// and \p FxFalse if a link doesn't exist.
	/// \note The names may be passed in any order.  Since it is an acyclic graph,
	/// there may only by a single link between two nodes, and the direction may be
	/// determined given the names of the two nodes.
	FxBool SetLink( const FxName& firstNodeName, const FxName& secondNodeName, 
		            const FxFaceGraphNodeLink& linkInfo );
	/// Determines the direction of a link between two nodes.
	/// Given two node names, fills out the two pointers specifying the input 
	/// node and the output node
	/// \param[in] firstNodeName The name of the node on one side of a link.
	/// \param[in] secondNodeName The name of the node on the other side of a link.
	/// \param[out] pInputNode A pointer to the node on the input side of the link.
	/// \param[out] pOutputNode A pointer to the node on the output side of the link.
	/// \return \p FxTrue if the results are valid, \p FxFalse if they should be ignored.
	/// \note The names may be passed in any order.  Since it is an acyclic graph,
	/// there may only by a single link between two nodes, and the direction may be
	/// determined given the names of the two nodes.
	FxBool DetermineLinkDirection( const FxName& firstNodeName,
								   const FxName& secondNodeName,
								   FxFaceGraphNode*& pInputNode,
								   FxFaceGraphNode*& pOutputNode );

	/// Returns the number of unique node types in the %Face Graph.
	FX_INLINE FxSize GetNumNodeTypes( void ) const { return _jumpLists.Length(); }
	/// Returns the node type at index.
	FX_INLINE const FxClass* GetNodeType( FxSize index ) const { return _jumpLists[index].GetNodeType(); }
	
	/// Serializes an FxFaceGraph to an archive.
	virtual void Serialize( FxArchive& arc );

	/// Changes the node named nodeToChangeTypeOf to be of the type specified by
	/// pNewTypeClassDesc.  FxActor::Link() should always be called on the actor
	/// that owns this %Face Graph after executing this function.
	/// \note Internal use only.  Licensees should never call this function.
	void ChangeNodeType( const FxName& nodeToChangeTypeOf, const FxClass* pNewTypeClassDesc );

	/// Notifies the %Face Graph that a rename has taken place.
	/// \note Internal use only. Licensees should never call this function.
	void NotifyRenamed( const char* oldName, const char* newName );

	/// An iterator over nodes of a specific type in an FxFaceGraph.
	/// \note You should never perform operations that may modify the %Face Graph 
	/// nodes (such as AddNode() or RemoveNode()) during a %Face Graph 
	/// iteration loop because those operations could invalidate the %Face Graph
	/// iterators.
	class Iterator
	{
	public:
		/// Constructor.
		Iterator( FxArray<FxFaceGraphNode*>::ConstIterator fgNodeListIter )
			: _fgNodeListIter(fgNodeListIter) {}

		/// Advance the iterator.
		FX_INLINE Iterator& operator++( void ) 
		{
			// Go to the next node.
			++_fgNodeListIter;
			return *this;
		}
		/// Move the iterator backwards.
		FX_INLINE Iterator& operator--( void )
		{
			// Go to the previous node.
			--_fgNodeListIter;
			return *this;
		}

		/// Returns a pointer to the node associated with the iterator.
		FX_INLINE FxFaceGraphNode* GetNode( void ) 
		{ 
			FxAssert((*_fgNodeListIter));
			return (*_fgNodeListIter);
		}

		/// Tests for iterator equality.
		FX_INLINE FxBool operator==( const Iterator& other ) { return _fgNodeListIter == other._fgNodeListIter; }
		/// Tests for iterator inequality.
		FX_INLINE FxBool operator!=( const Iterator& other ) { return _fgNodeListIter != other._fgNodeListIter; }

	private:
		/// The iterator into the %Face Graph's node list (or a jump list).
		FxArray<FxFaceGraphNode*>::ConstIterator _fgNodeListIter;
	};

private:
	/// The list of nodes stored in the %Face Graph.
	FxArray<FxFaceGraphNode*> _nodes;
	/// The node hash.  This contains 2048 (2^11) hash bins.
	FxStringHash<FxFaceGraphNode*, 11>* _pNodeHash;
	
	/// A structure to support super-fast %Face Graph iterators and %Face Graph 
	/// node class management.  This isn't a "traditional" jump list.
	/// It is a supplementary structure used in parallel with the main list
	/// of nodes (_nodes) to provide a quickly indexed organizational scheme
	/// based on node type.  Conceptually, it is akin to jumping directly from
	/// one node to the next node of that type directly in the main \a _nodes
	/// list but without the complexity of encoding this information directly
	/// in the main _nodes list.  Hence "jump" list.
	class FxFaceGraphNodeJumpList
	{
	public:
		/// Constructor.
		FxFaceGraphNodeJumpList( const FxClass* nodeType = NULL, 
			FxFaceGraphNode* pNode = NULL );
		/// Destructor.
		~FxFaceGraphNodeJumpList();

		/// Returns the %Face Graph node class associated with the jump list.
		FX_INLINE const FxClass* GetNodeType( void ) const { return _nodeType; }
		/// Sets the %Face Graph node class associated with the jump list.
		/// \param nodeType The node class associated with the jump list.
		/// \note Internal use only.
		void SetNodeType( const FxClass* nodeType );

		/// Returns the number of %Face Graph nodes in the jump list.
		FX_INLINE FxSize GetNumNodes( void ) const { return _jumpList.Length(); }
		/// Returns the %Face Graph node at index in the jump list.
		FX_INLINE FxFaceGraphNode* GetNode( FxSize index ) { return _jumpList[index]; }
		/// Finds pNode in the jump list and returns FxTrue if it was found
		/// or FxFalse if it wasn't.
		FxBool FindNode( FxFaceGraphNode* pNode ) const;
		/// Adds pNode to the jump list and returns FxTrue if it was added and
		/// FxFalse if it couldn't be added.  pNode is only added if it is of 
		/// the same type as the jump list type and if it is not already in the 
		/// jump list.
		FxBool AddNode( FxFaceGraphNode* pNode );
		/// Removes pNode from the jump list and returns FxTrue if it was
		/// removed and FxFalse if it couldn't be removed.  pNode is only 
		/// removed if it is of the same type as the jump list type and if it
		/// is actually in the jump list.
		FxBool RemoveNode( FxFaceGraphNode* pNode );

		/// Returns an iterator to the first element of the jump list.
		FX_INLINE FxArray<FxFaceGraphNode*>::ConstIterator Begin( void ) const { return _jumpList.Begin(); }
		/// Returns an iterator to the last element of the jump list.
		FX_INLINE FxArray<FxFaceGraphNode*>::ConstIterator End( void ) const { return _jumpList.End(); }
		
		/// Reserves space in the jump list for numNodes nodes.
		/// \param numNodes The number of nodes to reserve in the jump list.
		/// \note Internal use only.
		void Reserve( FxSize numNodes );

	protected:
		/// The %Face Graph node type stored in the jump list.
		const FxClass* _nodeType;
		/// The jump list.
		FxArray<FxFaceGraphNode*> _jumpList;
	};
	/// The jump list of %Face Graph nodes.
	FxArray<FxFaceGraphNodeJumpList> _jumpLists;
	
	/// Finds the node named 'name' (of any type) and returns a pointer to it
	/// or NULL if the node is not found.
	FxFaceGraphNode* _findNode( const FxName& name );

	/// Performs operations in preparation for deleting the node.
	void _preRemoveNode( FxFaceGraphNode* nodeToDelete );

	/// Relinks the %Face Graph.
	/// \param shouldClearOutputs If FxTrue, clears the outputs array on each
	/// node prior to linking up the outputs.  If FxFalse, the outputs array
	/// on each node is not cleared prior to linking up the outputs and it is
	/// assumed that each node's outputs array is the exact size that it needs
	/// to be to contain all of the node's outputs.  That will only be the case
	/// during load serialization of the %Face Graph.
	void _relink( FxBool shoudClearOutputs = FxTrue );

	/// Creates the internal jump list structures.
	/// \param shouldClearJumpLists If FxTrue, clears the jump lists array prior
	/// to re-creating the jump lists.  If FxFalse, the jump lists array is not 
	/// cleared prior to re-creating the jump lists and it is assumed that each 
	/// jump list's internal array is the exact size that it needs
	/// to be to contain all of the nodes in the jump list.  That will only be 
	/// the case during load serialization of the %Face Graph.
	void _createJumpLists( FxBool shouldClearJumpLists = FxTrue );

	/// Returns FxTrue if there are any cycles in the %Face Graph or FxFalse
	/// otherwise.
	FxBool _hasCycles( void );

	/// Returns the index of the node type in the jump list of node types.  If 
	/// the node type does not exist in the jump list, FxInvalidIndex is 
	/// returned.
	FxSize _findNodeType( const FxClass* nodeType ) const;

	/// Adds the node type of pNode to the node types list if it does not
	/// already exist in the list.
	void _addNodeType( FxFaceGraphNode* pNode );

	/// Clones the other Face Graph into this Face Graph.
	void _clone( const FxFaceGraph& other );
};

FX_INLINE FxFaceGraph::Iterator FxFaceGraph::Begin( const FxClass* fgNodeClass ) const
{
	if( fgNodeClass != FxFaceGraphNode::StaticClass() )
	{
		FxSize jumpListIndex = _findNodeType(fgNodeClass);
		if( FxInvalidIndex != jumpListIndex )
		{
			FxArray<FxFaceGraphNode*>::ConstIterator fgNodeIter = 
				_jumpLists[jumpListIndex].Begin();
			return FxFaceGraph::Iterator(fgNodeIter);
		}
		else
		{
			FxArray<FxFaceGraphNode*>::ConstIterator fgNodeIter = _nodes.End();
			return FxFaceGraph::Iterator(fgNodeIter);
		}
	}
	FxArray<FxFaceGraphNode*>::ConstIterator fgNodeIter = _nodes.Begin();
	return FxFaceGraph::Iterator(fgNodeIter);
}

FX_INLINE FxFaceGraph::Iterator FxFaceGraph::End( const FxClass* fgNodeClass ) const
{
	if( fgNodeClass != FxFaceGraphNode::StaticClass() )
	{
		FxSize jumpListIndex = _findNodeType(fgNodeClass);
		if( FxInvalidIndex != jumpListIndex )
		{
			FxArray<FxFaceGraphNode*>::ConstIterator fgNodeIter = 
				_jumpLists[jumpListIndex].End();
			return FxFaceGraph::Iterator(fgNodeIter);
		}
		else
		{
			FxArray<FxFaceGraphNode*>::ConstIterator fgNodeIter = _nodes.End();
			return FxFaceGraph::Iterator(fgNodeIter);
		}
	}
	FxArray<FxFaceGraphNode*>::ConstIterator fgNodeIter = _nodes.End();
	return FxFaceGraph::Iterator(fgNodeIter);
}

} // namespace Face

} // namespace OC3Ent

#endif
