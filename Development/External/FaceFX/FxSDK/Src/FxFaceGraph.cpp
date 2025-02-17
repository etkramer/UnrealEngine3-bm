//------------------------------------------------------------------------------
// This class implements the FaceFx face graph (Directed Acyclic Graph).
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxFaceGraph.h"
#include "FxCurrentTimeNode.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxFaceGraphVersion 1

FX_IMPLEMENT_CLASS(FxFaceGraph, kCurrentFxFaceGraphVersion, FxObject)

FxFaceGraph::FxFaceGraphNodeJumpList::
FxFaceGraphNodeJumpList( const FxClass* nodeType, FxFaceGraphNode* pNode )
	: _nodeType(nodeType)
{
	if( pNode )
	{
		_jumpList.PushBack(pNode);
	}
}

FxFaceGraph::FxFaceGraphNodeJumpList::~FxFaceGraphNodeJumpList()
{
}

void FxFaceGraph::
FxFaceGraphNodeJumpList::SetNodeType( const FxClass* nodeType )
{
	_nodeType = nodeType;
}

FxBool FxFaceGraph::FxFaceGraphNodeJumpList::
FindNode( FxFaceGraphNode* pNode ) const
{
	if( FxInvalidIndex != _jumpList.Find(pNode) )
	{
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxFaceGraph::
FxFaceGraphNodeJumpList::AddNode( FxFaceGraphNode* pNode )
{
	FxBool retval = FxFalse;
	FxAssert(pNode);
	// Only add the node if it is of the same type as all of the other nodes
	// in the jump list.
	if( pNode->IsA(_nodeType) )
	{
		// Only add the node if it isn't already in the jump list.
		if( !FindNode(pNode) )
		{
			_jumpList.PushBack(pNode);
			retval = FxTrue;
		}
	}
	else
	{
		FxAssert(!"FxFaceGraphNodeJumpList node type mismatch!");
	}
	return retval;
}

FxBool FxFaceGraph::
FxFaceGraphNodeJumpList::RemoveNode( FxFaceGraphNode* pNode )
{
	FxAssert(pNode);
	// Only remove the node if it is of the same type as all of the other 
	// nodes in the jump list.
	if( pNode->IsA(_nodeType) )
	{
		FxSize index = _jumpList.Find(pNode);
		if( FxInvalidIndex != index )
		{
			_jumpList.Remove(index);
			return FxTrue;
		}
	}
	else
	{
		FxAssert(!"FxFaceGraphNodeJumpList node type mismatch!");
	}
	return FxFalse;
}

void FxFaceGraph::
FxFaceGraphNodeJumpList::Reserve( FxSize numNodes )
{
	_jumpList.Clear();
	_jumpList.Reserve(numNodes);
}

FxFaceGraph::FxFaceGraph()
{
	_pNodeHash = NULL;
}

FxFaceGraph::FxFaceGraph( const FxFaceGraph& other )
	: Super(other)
{
	_pNodeHash = NULL;
	_clone(other);
}

FxFaceGraph& FxFaceGraph::operator=( const FxFaceGraph& other )
{
	if( this == &other ) return *this;
 	Super::operator=(other);
	if( _pNodeHash )
	{
		delete _pNodeHash;
	}
	_pNodeHash = NULL;
 	_clone(other);
	return *this;
}

FxFaceGraph::~FxFaceGraph()
{
	Clear();

	// Clean up the node hash if it exists.
	if( _pNodeHash )
	{
		delete _pNodeHash;
		_pNodeHash = NULL;
	}
}

void FxFaceGraph::Clear( void )
{
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxFaceGraphNode* pNode = _nodes[i];
		FxAssert(pNode);
		FxSize size = pNode->GetClassDesc()->GetSize();
		pNode->~FxFaceGraphNode();
		FxFree(pNode, size);
		pNode = NULL;
	}
	_nodes.Clear();
	if( _pNodeHash )
	{
		_pNodeHash->Clear();
	}
	_jumpLists.Clear();
}

FxBool FxFaceGraph::AddNode( FxFaceGraphNode* pNode )
{
	if( !_pNodeHash )
	{
		_pNodeHash = new FxStringHash<FxFaceGraphNode*, 11>();
	}
	FxAssert(_pNodeHash);
	FxBool retval = FxFalse;
	if( pNode )
	{
		FxFaceGraphNode* alreadyInFaceGraph = _findNode(pNode->GetName());
		if( !alreadyInFaceGraph )
		{
			_nodes.PushBack(pNode);
			_pNodeHash->Insert(pNode->GetNameAsCstr(), pNode);
			_addNodeType(pNode);
			FxSize jumpListIndex = _findNodeType(pNode->GetClassDesc());
			if( FxInvalidIndex != jumpListIndex )
			{
				_jumpLists[jumpListIndex].AddNode(pNode);
			}
			retval = FxTrue;
		}
	}
	return retval;
}

FxBool FxFaceGraph::RemoveNode( const FxName& name )
{
	FxAssert(_pNodeHash);
	FxBool retval = FxFalse;
	FxFaceGraphNode* nodeToDelete = _findNode(name);
	if( nodeToDelete )
	{
		_preRemoveNode(nodeToDelete);
		_pNodeHash->Remove(nodeToDelete->GetNameAsCstr());
		FxSize index = _nodes.Find(nodeToDelete);
		FxSize size = nodeToDelete->GetClassDesc()->GetSize();
		nodeToDelete->~FxFaceGraphNode();
		FxFree(nodeToDelete, size);
		nodeToDelete = NULL;
		_nodes.Remove(index);
		retval = FxTrue;
	}
	return retval;
}

FxFaceGraphNode* FxFaceGraph::FindNode( const FxName& name )
{
	return _findNode(name);
}

FxBool FxFaceGraph::RenameNode( const FxName& currName, const FxName newName )
{
	FxFaceGraphNode* pCurrNode = _findNode(currName);
	FxFaceGraphNode* pDesiredNode = _findNode(newName);
	
	// Abort if either the current node doesn't exist in the graph or if the
	// desired node already exists in the graph.
	if( !pCurrNode || pDesiredNode )
	{
		return FxFalse;
	}

	// Remove the node from the hash, set it's new name, and rehash it.
	_pNodeHash->Remove(pCurrNode->GetNameAsCstr());
	pCurrNode->SetName(newName);
	_pNodeHash->Insert(pCurrNode->GetNameAsCstr(), pCurrNode);

	// Patch up any outputs.
	FxSize numOutputs = pCurrNode->GetNumOutputs();
	for( FxSize i = 0; i < numOutputs; ++i )
	{
		FxFaceGraphNode* outputNode = pCurrNode->GetOutput(i);
		FxSize numLinks = outputNode->GetNumInputLinks();
		for( FxSize j = 0; j < numLinks; ++j )
		{
			FxFaceGraphNodeLink link = outputNode->GetInputLink(j);
			if( link.GetNodeName() == currName )
			{
				// Update the name stored in the link.
				link.SetNode(pCurrNode);
				outputNode->ModifyInputLink(j, link);
			}
		}
	}

	return FxTrue;
}

FxSize FxFaceGraph::CountNodes( const FxClass* fgNodeClass )
{
	if( fgNodeClass == FxFaceGraphNode::StaticClass() )
	{
		return _nodes.Length();
	}
	else
	{
		FxSize jumpListIndex = _findNodeType(fgNodeClass);
		if( FxInvalidIndex != jumpListIndex )
		{
			return _jumpLists[jumpListIndex].GetNumNodes();
		}
	}
	return 0;
}

FxLinkErrorCode 
FxFaceGraph::Link( const FxName& toNode, const FxName& fromNode, 
		           const FxName& linkFn, const FxLinkFnParameters& linkFnParams )
{
	// Make sure both nodes are in the graph.
	FxFaceGraphNode* pToNode   = _findNode(toNode);
	FxFaceGraphNode* pFromNode = _findNode(fromNode);
	// Make sure the link function is found.
	const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunction(linkFn);
	if( pToNode && pFromNode && pLinkFn )
	{
		// Prevent self linkage.
		if( pToNode == pFromNode )
		{
			return LEC_Cycle;
		}
		// Prevent duplicate input links (even if they have different
		// link functions).  Allowing such links would complicate the code and
		// the operation duplicate links would accomplish can also be
		// accomplished by adding an intermediate node.
		FxBool okToAddLink = FxTrue;
		FxSize numInputLinks = pToNode->GetNumInputLinks();
		for( FxSize i = 0; i < numInputLinks; ++i )
		{
			const FxFaceGraphNodeLink& link = pToNode->GetInputLink(i);
			if( link.GetNode() == pFromNode )
			{
				okToAddLink = FxFalse;
				break;
			}
		}
		if( !okToAddLink )
		{
			// Could not add the link because it is a duplicate.
			return LEC_Duplicate;
		}
		else
		{
			FxFaceGraphNodeLink link(fromNode, pFromNode, linkFn, pLinkFn->GetType(), linkFnParams);
			if( FxFalse == pToNode->AddInputLink(link) )
			{
				// The toNode refused to add another link.
				return LEC_Refused;
			}
			pFromNode->_outputs.PushBack(pToNode);
			// If pToNode has no outputs or pFromNode has no inputs, then a 
			// cycle can't possibly be created so don't check for them for 
			// increased performance on large and complicated face graphs.
			if( (pToNode->_outputs.Length() > 0) && (pFromNode->_inputs.Length() > 0) )
			{
				if( pToNode->_hasCycles() )
				{
					// If the link creates cycles in the graph, remove it and 
					// return the error.
					pToNode->RemoveInputLink(pToNode->GetNumInputLinks()-1);
					pFromNode->_outputs.Remove(pFromNode->_outputs.Length()-1);
					return LEC_Cycle;
				}
			}
			// The link was added with no errors.
			return LEC_None;
		}
	}
	// The link is invalid.
	return LEC_Invalid;
}

FxBool FxFaceGraph::Unlink( const FxName& firstNodeName, const FxName& secondNodeName )
{
	FxFaceGraphNode* pInputNode  = NULL;
	FxFaceGraphNode* pOutputNode = NULL;
	DetermineLinkDirection(firstNodeName, secondNodeName, pInputNode, pOutputNode);
	if( pInputNode && pOutputNode )
	{
		// Try the first direction.
		FxSize numInputLinks = pInputNode->GetNumInputLinks();
		for( FxSize i = 0; i < numInputLinks; ++i )
		{
			const FxFaceGraphNodeLink& link = pInputNode->GetInputLink(i);
			// If the pointers are equal, this is the correct link to remove.
			if( link.GetNode() == pOutputNode )
			{
				pInputNode->RemoveInputLink(i);
				// Find the output in pOutputNode and remove it.
				FxSize numOutputs = pOutputNode->_outputs.Length();
				for( FxSize j = 0; j < numOutputs; ++j )
				{
					if( pOutputNode->_outputs[j] == pInputNode )
					{
						pOutputNode->_outputs.Remove(j);
						break;
					}
				}
				return FxTrue;
			}
		}
	}
	return FxFalse;
}


FxBool FxFaceGraph::GetLink( const FxName& firstNodeName, const FxName& secondNodeName, 
						     FxFaceGraphNodeLink& linkInfo )
{
	FxBool retval = FxFalse;
	FxFaceGraphNode* pInputNode  = NULL;
	FxFaceGraphNode* pOutputNode = NULL;
	DetermineLinkDirection(firstNodeName, secondNodeName, pInputNode, pOutputNode);
	if( pInputNode && pOutputNode )
	{
		// Find the link from input to output.
		FxSize numInputLinks = pInputNode->GetNumInputLinks();
		for( FxSize i = 0; i < numInputLinks; ++i )
		{
			if( pInputNode->GetInputLink(i).GetNode() == pOutputNode )
			{
				// Fill out the structure to return.
				linkInfo = pInputNode->GetInputLink(i);
				retval = FxTrue;
				break;
			}
		}
	}
	return retval;
}

FxBool FxFaceGraph::SetLink( const FxName& firstNodeName, const FxName& secondNodeName, 
						     const FxFaceGraphNodeLink& linkInfo )
{
	FxBool retval = FxFalse;
	FxFaceGraphNode* pInputNode  = NULL;
	FxFaceGraphNode* pOutputNode = NULL;
	DetermineLinkDirection(firstNodeName, secondNodeName, pInputNode, pOutputNode);
	if( pInputNode && pOutputNode )
	{
		// Find the link from input to output.
		FxSize numInputLinks = pInputNode->GetNumInputLinks();
		for( FxSize i = 0; i < numInputLinks; ++i )
		{
			if( pInputNode->GetInputLink(i).GetNode() == pOutputNode )
			{
				pInputNode->ModifyInputLink(i, linkInfo);
				retval = FxTrue;
				break;
			}
		}
	}
	return retval;
}

FxBool FxFaceGraph::
DetermineLinkDirection( const FxName& firstNodeName, const FxName& secondNodeName,
						FxFaceGraphNode*& pInputNode, FxFaceGraphNode*& pOutputNode )
{
	FxBool retval = FxFalse;
	pInputNode  = NULL;
	pOutputNode = NULL;

	// Make sure both nodes are in the graph.
	FxFaceGraphNode* pFirstNode   = _findNode(firstNodeName);
	FxFaceGraphNode* pSecondNode  = _findNode(secondNodeName);
	if( pFirstNode && pSecondNode )
	{
		// Go through the inputs of the first node and see if it links to the 
		// second node.
		FxSize numInputLinks = pFirstNode->GetNumInputLinks();
		for( FxSize i = 0; i < numInputLinks; ++i )
		{
			if( pFirstNode->GetInputLink(i).GetNode() == pSecondNode )
			{
				pInputNode  = pFirstNode;
				pOutputNode = pSecondNode;
				break;
			}
		}
		numInputLinks = pSecondNode->GetNumInputLinks();
		for( FxSize i = 0; i < numInputLinks; ++i )
		{
			if( pSecondNode->GetInputLink(i).GetNode() == pFirstNode )
			{
				pInputNode  = pSecondNode;
				pOutputNode = pFirstNode;
				break;
			}
		}
	}
	return retval;
}

void FxFaceGraph::Serialize( FxArchive& arc )
{
	if( arc.IsLoading() )
	{
		Clear();
	}

	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxFaceGraph");

	arc << _nodes;

	if( arc.IsLoading() )
	{
		FxSize numNodes = _nodes.Length();
		if( numNodes > 0 ) 
		{
			_pNodeHash = new FxStringHash<FxFaceGraphNode*, 11>();
		}
		FxAssert(_pNodeHash);
		for( FxSize i = 0; i < numNodes; ++i )
		{
			FxAssert(_nodes[i]);
			_pNodeHash->Insert(_nodes[i]->GetNameAsCstr(), _nodes[i]);
		}
	}

	if( version >= 1 )
	{
		FxSize numJumpLists = _jumpLists.Length();
		arc << numJumpLists;

		if( arc.IsSaving() )
		{
			for( FxSize i = 0; i < numJumpLists; ++i )
			{
				FxFaceGraphNodeJumpList& jumpList = _jumpLists[i];
				FxSize numNodes = jumpList.GetNumNodes();
				arc << numNodes;
			}
		}
		else if( arc.IsLoading() )
		{
			_jumpLists.Reserve(numJumpLists);
			for( FxSize i = 0; i < numJumpLists; ++i )
			{
				FxSize numNodes = 0;
				arc << numNodes;
				_jumpLists.PushBack(FxFaceGraphNodeJumpList());
				_jumpLists.Back().Reserve(numNodes);
			}
		}
	}

	if( arc.IsLoading() )
	{
		// Relink the face graph.
		_relink(version >= 1 ? FxFalse : FxTrue);
		// Rebuild the jump lists.
		_createJumpLists(version >= 1 ? FxFalse : FxTrue);
	}
}

void FxFaceGraph::
ChangeNodeType( const FxName& nodeToChangeTypeOf, const FxClass* pNewTypeClassDesc )
{
	FxAssert(_pNodeHash);
	if( pNewTypeClassDesc && pNewTypeClassDesc->IsKindOf(FxFaceGraphNode::StaticClass()) )
	{
		FxFaceGraphNode* pNode = _findNode(nodeToChangeTypeOf);
		if( pNode )
		{
			FxFaceGraphNode* pNewNode = FxCast<FxFaceGraphNode>(pNewTypeClassDesc->ConstructObject());
			if( pNewNode )
			{
				// Copy the node data from the old node type to the new node type.
				pNode->CopyData(pNewNode);
				pNewNode->_inputs = pNode->_inputs;
				pNewNode->_userData = pNode->_userData;
				// Delete the old node type and replace it in the _nodes array
				// with the new node type.
				FxSize numNodes = _nodes.Length();
				for( FxSize i = 0; i < numNodes; ++i )
				{
					if( _nodes[i] == pNode )
					{
						FxSize size = pNode->GetClassDesc()->GetSize();
						_pNodeHash->Remove(pNode->GetNameAsCstr());
						pNode->~FxFaceGraphNode();
						FxFree(pNode, size);
						pNode = NULL;
						_nodes[i] = pNewNode;
						_pNodeHash->Insert(pNewNode->GetNameAsCstr(), pNewNode);
					}
				}
				// Relink the face graph.
				_relink();
				// Rebuild the jump lists.
				_createJumpLists();
			}
		}
	}
}

void FxFaceGraph::NotifyRenamed( const char* oldName, const char* newName )
{
	FxAssert(_pNodeHash);
	// This is called after a rename has taken place, so a node might have been
	// renamed out from under the hash. Update the hash accordingly.
	FxFaceGraphNode* pNode = NULL;

	// Can't use _findNode since that uses the node hash.
	// Do it the old-fashioned way.
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( _nodes[i]->GetNameAsString() == newName )
		{
			pNode = _nodes[i];
			break;
		}
	}

	if( pNode )
	{
		_pNodeHash->Remove(oldName);
		_pNodeHash->Insert(newName, pNode);
	}
}

FxFaceGraphNode* FxFaceGraph::
_findNode( const FxName& name )
{
	FxFaceGraphNode* pRetNode = NULL;
	if( _pNodeHash )
	{
		// If the node isn't found in the hash, pRetNode is still NULL.
		_pNodeHash->Find(name.GetAsCstr(), pRetNode);
	}
	return pRetNode;
}

void FxFaceGraph::_preRemoveNode( FxFaceGraphNode* nodeToDelete )
{
	if( nodeToDelete )
	{
		// Remove any links to nodeToDelete:
		// First go through all nodes that input to nodeToDelete and remove
		// any references to nodeToDelete in their outputs.
		for( FxSize i = 0; i < nodeToDelete->_inputs.Length(); ++i )
		{
			const FxFaceGraphNode* inputNode = nodeToDelete->_inputs[i].GetNode();
			if( inputNode )
			{
				for( FxSize j = 0; j < inputNode->_outputs.Length(); ++j )
				{
					if( inputNode->_outputs[j] )
					{
						// In this case, there should only be one entry in
						// inputNode's _outputs array corresponding to 
						// nodeToDelete.  The code loops through them all,
						// though.
						if( FxTrue == Unlink(inputNode->GetName(), 
											 nodeToDelete->GetName()) )
						{
							// Back up because removing the link shifted elements
							// in _outputs.
							j--;
						}
					}
				}
			}
			// Back up because removing the link shifted elements in _inputs.
			i--;
		}
		FxAssert( nodeToDelete->_inputs.Length() == 0 );
		// Next go through all outputs of nodeToDelete and remove any references
		// to nodeToDelete in their inputs.
		for( FxSize i = 0; i < nodeToDelete->_outputs.Length(); ++i )
		{
			if( nodeToDelete->_outputs[i] )
			{
				if( FxTrue == Unlink(nodeToDelete->_outputs[i]->GetName(), 
					                 nodeToDelete->GetName()) )
				{
					// Back up because removing the link shifted elements
					// in _outputs.
					i--;
				}
			}
		}
		FxAssert( nodeToDelete->_outputs.Length() == 0 );
		// Remove the node from the jump list.
		FxSize jumpListIndex = _findNodeType(nodeToDelete->GetClassDesc());
		_jumpLists[jumpListIndex].RemoveNode(nodeToDelete);
		if( _jumpLists[jumpListIndex].GetNumNodes() == 0 )
		{
			_jumpLists.Remove(jumpListIndex);
		}
	}
}

void FxFaceGraph::_relink( FxBool shoudClearOutputs )
{
	FxSize numNodes = _nodes.Length();

	// Clear out all the outputs.
	if( shoudClearOutputs )
	{
		for( FxSize i = 0; i < numNodes; ++i )
		{
			FxFaceGraphNode* pNode = _nodes[i];
			if( pNode )
			{
				pNode->_outputs.Clear();
			}
		}
	}

	// Relink the graph.
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxFaceGraphNode* pNode = _nodes[i];
		if( pNode )
		{
			FxSize numInputLinks = pNode->GetNumInputLinks();
			for( FxSize j = 0; j < numInputLinks; ++j )
			{
				FxFaceGraphNode* fromNode = _findNode(pNode->_inputs[j].GetNodeName());
				pNode->_inputs[j].SetNode(fromNode);
				// Keep track of the outputs.
				if( fromNode )
				{
					fromNode->_outputs.PushBack(pNode);
				}
			}
		}
	}
}

void FxFaceGraph::_createJumpLists( FxBool shouldClearJumpLists )
{
	if( shouldClearJumpLists )
	{
		_jumpLists.Clear();
	}
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxFaceGraphNode* pNode = _nodes[i];
		FxAssert(pNode);
		_addNodeType(pNode);
		FxSize jumpListIndex = _findNodeType(pNode->GetClassDesc());
		if( FxInvalidIndex != jumpListIndex )
		{
			_jumpLists[jumpListIndex].AddNode(pNode);
		}
	}
}

FxBool FxFaceGraph::_hasCycles( void )
{
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxAssert(_nodes[i]);
		_nodes[i]->_hasBeenVisited = FxFalse;
	}
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxAssert(_nodes[i]);
		if( _nodes[i]->_hasCycles() )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

FxSize FxFaceGraph::_findNodeType( const FxClass* nodeType ) const
{
	if( nodeType )
	{
		FxSize numJumpLists = _jumpLists.Length();
		for( FxSize i = 0; i < numJumpLists; ++i )
		{
			if( _jumpLists[i].GetNodeType() == nodeType )
			{
				return i;
			}
		}
	}
	return FxInvalidIndex;
}

void FxFaceGraph::_addNodeType( FxFaceGraphNode* pNode )
{
	FxAssert(pNode);
	const FxClass* nodeType = pNode->GetClassDesc();
	if( FxInvalidIndex == _findNodeType(nodeType) )
	{
		FxSize numJumpLists = _jumpLists.Length();
		for( FxSize i = 0; i < numJumpLists; ++i )
		{
			FxFaceGraphNodeJumpList& jumpList = _jumpLists[i];
			if( !jumpList.GetNodeType() )
			{
				jumpList.SetNodeType(nodeType);
				return;
			}
		}
		_jumpLists.PushBack(FxFaceGraphNodeJumpList(pNode->GetClassDesc()));
	}
}

void FxFaceGraph::_clone( const FxFaceGraph& other )
{
	Clear();
	// Clone all of the nodes.
	FxSize numNodes = other._nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxAssert(other._nodes[i]);
		FxFaceGraphNode* pClone = other._nodes[i]->Clone();
		AddNode(pClone);
	}
	// Clone all of the links.
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxFaceGraphNode* pNode = other._nodes[i];
		FxAssert(pNode);
		FxSize numInputLinks = pNode->GetNumInputLinks();
		for( FxSize j = 0; j < numInputLinks; ++j )
		{
			const FxFaceGraphNodeLink& link = pNode->GetInputLink(j);
			Link(pNode->GetName(), link.GetNodeName(), link.GetLinkFnName(), link.GetLinkFnParams());
		}
	}
	// Relink the face graph.
	_relink();
	// Rebuild the jump lists.  This is needed to find the new FxCurrentTimeNode
	// jump list index.
	_createJumpLists();
}

} // namespace Face

} // namespace OC3Ent
