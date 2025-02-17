#ifndef TestFxFaceGraph_H__
#define TestFxFaceGraph_H__

#include "FxFaceGraph.h"
#include "FxCompiledFaceGraph.h"
using namespace OC3Ent::Face;

UNITTEST( FxFaceGraph, Usage )
{
	FxBonePoseNode* pBonePoseNode = new FxBonePoseNode();
	pBonePoseNode->SetName("bonePoseNode1");
	FxBonePoseNode* pBonePoseNode2 = new FxBonePoseNode();
	pBonePoseNode2->SetName("bonePoseNode2");
	FxFaceGraph faceGraph;
	faceGraph.AddNode(pBonePoseNode);
	faceGraph.AddNode(pBonePoseNode2);
	CHECK( 2 == faceGraph.GetNumNodes() );
	FxCompiledFaceGraph cg;
	cg.Compile(faceGraph);
	CHECK( 2 == cg.nodes.Length() );

	FxMasterBoneList mbl;

	FxBone refBone("refBone", FxVec3(5.05f,2.23f,0.43f),
		            FxQuat(1.0f,0.456f,0.567f,0.523f).Normalize(),
		            FxVec3(1.0f,1.0f,1.0f));
	FxBone refBoneCopy("refBone", FxVec3(0.0f,1.0f,1.0f),
		                FxQuat(0.5f,0.245f,0.324f,0.134f ).Normalize(),
		                FxVec3(1.0f,1.0f,1.0f));
	FxBone refBoneNew("refBone", FxVec3(0.0f,1.0f,1.0f),
		               FxQuat(0.5f,0.245f,0.324f,0.134f ).Normalize(),
		               FxVec3(1.0f,1.0f,1.0f));

	mbl.AddRefBone(refBone);
	CHECK( 1 == mbl.GetNumRefBones() );

	pBonePoseNode->AddBone(refBoneCopy);
	pBonePoseNode2->AddBone(refBoneNew);
	CHECK( 1 == pBonePoseNode->GetNumBones() );
	CHECK( 1 == pBonePoseNode2->GetNumBones() );

	mbl.Prune(faceGraph);
	CHECK( 1 == pBonePoseNode->GetNumBones() );
	CHECK( 1 == pBonePoseNode2->GetNumBones() );

	mbl.PullBones(faceGraph, cg);
	FxVec3 bonePos, boneScale;
	FxQuat boneRot;
	FxReal boneWeight;
	mbl.GetBlendedBone(0, cg, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == refBone.GetPos() );
	CHECK( boneRot   == refBone.GetRot() );
	CHECK( boneScale == refBone.GetScale() );

	faceGraph.Clear();
}

class FxNewTestNode : public FxFaceGraphNode
{
	// Declare the class.
	FX_DECLARE_CLASS(FxNewTestNode, FxFaceGraphNode);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxNewTestNode);
public:
	// Constructor.
	FxNewTestNode() {};
	// Destructor.
	virtual ~FxNewTestNode() {};

	virtual FxNewTestNode* Clone( void )
	{
		FxNewTestNode* pNode = new FxNewTestNode();
		CopyData(pNode);
		return pNode;
	}

	virtual void CopyData( FxNewTestNode* pOther )
	{
		if( pOther )
		{
			Super::CopyData(pOther);
		}
	}
};

// Implement the class.
FX_IMPLEMENT_CLASS(FxNewTestNode,0,FxFaceGraphNode);

class FxAnotherNewTestNode : public FxFaceGraphNode
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnotherNewTestNode, FxFaceGraphNode);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxAnotherNewTestNode);
public:
	// Constructor.
	FxAnotherNewTestNode() {};
	// Destructor.
	virtual ~FxAnotherNewTestNode() {};

	virtual FxAnotherNewTestNode* Clone( void )
	{
		FxAnotherNewTestNode* pNode = new FxAnotherNewTestNode();
		CopyData(pNode);
		return pNode;
	}

	virtual void CopyData( FxAnotherNewTestNode* pOther )
	{
		if( pOther )
		{
			Super::CopyData(pOther);
		}
	}
};

// Implement the class.
FX_IMPLEMENT_CLASS(FxAnotherNewTestNode,0,FxFaceGraphNode);

UNITTEST( FxFaceGraph, IteratorsAndCounting )
{
	FxFaceGraph faceGraph;
	FxCombinerNode* combinerNode0 = new FxCombinerNode();
	combinerNode0->SetName("testCombinerNode0");
	FxBonePoseNode* bonePoseNode0 = new FxBonePoseNode();
	bonePoseNode0->SetName("testTargetNode0");
	FxCombinerNode* combinerNode1 = new FxCombinerNode();
	combinerNode1->SetName("testCombinerNode1");
	FxNewTestNode* newTestNode0 = new FxNewTestNode();
	newTestNode0->SetName("testNewTestNode0");
	FxBonePoseNode* bonePoseNode1 = new FxBonePoseNode();
	bonePoseNode1->SetName("testTargetNode1");
	FxCombinerNode* combinerNode2 = new FxCombinerNode();
	combinerNode2->SetName("testCombinerNode2");
	FxNewTestNode* newTestNode1 = new FxNewTestNode();
	newTestNode1->SetName("testNewTestNode1");
	CHECK( faceGraph.AddNode(combinerNode0) == FxTrue );
	CHECK( faceGraph.AddNode(bonePoseNode0) == FxTrue );
	CHECK( faceGraph.AddNode(combinerNode1) == FxTrue );
	CHECK( faceGraph.AddNode(newTestNode0) == FxTrue );
	CHECK( faceGraph.AddNode(bonePoseNode1) == FxTrue );
	CHECK( faceGraph.AddNode(combinerNode2) == FxTrue );
	CHECK( faceGraph.AddNode(newTestNode1) == FxTrue );
	CHECK( 7 == faceGraph.GetNumNodes() );
	CHECK( 7 == faceGraph.CountNodes() );

	FxFaceGraph::Iterator combinerNodeIter    = faceGraph.Begin(FxCombinerNode::StaticClass());
	FxFaceGraph::Iterator combinerNodeEndIter = faceGraph.End(FxCombinerNode::StaticClass());
	FxSize numCombinerNodes = 0;
	for( ; combinerNodeIter != combinerNodeEndIter; ++combinerNodeIter )
	{
		numCombinerNodes++;
	}
	CHECK( 3 == numCombinerNodes );

	FxFaceGraph::Iterator bonePoseNodeIter    = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator bonePoseNodeEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	FxSize numTargetNodes = 0;
	for( ; bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
	{
		numTargetNodes++;
	}
	CHECK( 2 == numTargetNodes );

	FxFaceGraph::Iterator newTestNodeIter    = faceGraph.Begin(FxNewTestNode::StaticClass());
	FxFaceGraph::Iterator newTestNodeEndIter = faceGraph.End(FxNewTestNode::StaticClass());
	FxSize numNewTestNodes = 0;
	for( ; newTestNodeIter != newTestNodeEndIter; ++newTestNodeIter )
	{
		numNewTestNodes++;
	}
	CHECK( 2 == numNewTestNodes );

	faceGraph.RemoveNode("testNewTestNode1");
	newTestNodeIter    = faceGraph.Begin(FxNewTestNode::StaticClass());
	newTestNodeEndIter = faceGraph.End(FxNewTestNode::StaticClass());
	numNewTestNodes = 0;
	for( ; newTestNodeIter != newTestNodeEndIter; ++newTestNodeIter )
	{
		numNewTestNodes++;
	}
	CHECK( 1 == numNewTestNodes );

	CHECK( numCombinerNodes == faceGraph.CountNodes(FxCombinerNode::StaticClass()) );
	CHECK( numTargetNodes   == faceGraph.CountNodes(FxBonePoseNode::StaticClass()) );
	CHECK( numNewTestNodes  == faceGraph.CountNodes(FxNewTestNode::StaticClass()) );
	CHECK( 0 == faceGraph.CountNodes(FxAnotherNewTestNode::StaticClass()) );

	// Try to iterate over non-existent node types.
	FxFaceGraph::Iterator anotherNewTestNodeIter    = faceGraph.Begin(FxAnotherNewTestNode::StaticClass());
	FxFaceGraph::Iterator anotherNewTestNodeEndIter = faceGraph.End(FxAnotherNewTestNode::StaticClass());
	FxSize numAnotherNewTestNodes = 0;
	for( ; anotherNewTestNodeIter != anotherNewTestNodeEndIter; ++anotherNewTestNodeIter )
	{
		numAnotherNewTestNodes++;
	}
	CHECK( 0 == numAnotherNewTestNodes );

	// Test going backwards.
	// Go forward through the graph.
	FxFaceGraph::Iterator combinerNodeIterBackwards    = faceGraph.Begin(FxCombinerNode::StaticClass());
	FxFaceGraph::Iterator combinerNodeIterBackwardsEnd = faceGraph.End(FxCombinerNode::StaticClass());
	for( ; combinerNodeIterBackwards != combinerNodeIterBackwardsEnd; ++combinerNodeIterBackwards )
	{
	}
	// Now go backward checking each combiner node.
	FxSize currentCombinerNode = 3;
	// Make sure the iterator is actually on a combiner node.
	--combinerNodeIterBackwards;
	for( ; ; --combinerNodeIterBackwards )
	{
		FxFaceGraphNode* pNode = combinerNodeIterBackwards.GetNode();
		if( pNode )
		{
			if( currentCombinerNode == 3 )
			{
				CHECK( pNode->GetName() == combinerNode2->GetName() );
			}
			else if( currentCombinerNode == 2 )
			{
				CHECK( pNode->GetName() == combinerNode1->GetName() );
			}
			else if( currentCombinerNode == 1 )
			{
				CHECK( pNode->GetName() == combinerNode0->GetName() );
				break;
			}
			currentCombinerNode--;
		}
	}

	faceGraph.Clear();
}

UNITTEST( FxFaceGraph, Clear )
{
	//@todo compiled
	/*FxFaceGraph faceGraph;
	FxBonePoseNode* bonePoseNode = new FxBonePoseNode();
	bonePoseNode->SetName("testTargetNode");
	CHECK( faceGraph.AddNode(bonePoseNode) == FxTrue );
	FxCombinerNode* combinerNode = new FxCombinerNode();
	combinerNode->SetName("testCombinerNode");
	CHECK( faceGraph.AddNode(combinerNode) == FxTrue );
	
	faceGraph.ClearValues();
	
	// No real good way to test this because user value and track value are
	// hidden in the FxFaceGraphNode interface, and GetValue() will Clamp() the result
	// to the min / max of the node.  In the default case min is zero and max is one, 
	// so an actual GetValue() call should produce zero if the node was cleared.  
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator combinerNodeIter = faceGraph.Begin(FxCombinerNode::StaticClass());
	FxFaceGraphNode* pBonePoseNode = bonePoseNodeIter.GetNode();
	FxFaceGraphNode* pCombinerNode = combinerNodeIter.GetNode();
	CHECK( pBonePoseNode );
	CHECK( pCombinerNode );
	CHECK( pBonePoseNode->GetValue() == 0.0f );
	CHECK( pCombinerNode->GetValue() == 0.0f );

	faceGraph.Clear();*/
}

UNITTEST( FxFaceGraph, AddNode )
{
	FxFaceGraph faceGraph;
	FxBonePoseNode* bonePoseNode = new FxBonePoseNode();
	bonePoseNode->SetName("testTargetNode");
	CHECK( faceGraph.AddNode(bonePoseNode) == FxTrue );
	CHECK( faceGraph.GetNumNodes() == 1 );
	CHECK( faceGraph.AddNode(bonePoseNode) == FxFalse );

	faceGraph.Clear();
}

UNITTEST( FxFaceGraph, RemoveNode )
{
	FxFaceGraph faceGraph;
	FxBonePoseNode* bonePoseNode = new FxBonePoseNode();
	bonePoseNode->SetName("testTargetNode");
	CHECK( faceGraph.AddNode(bonePoseNode) == FxTrue );
	CHECK( faceGraph.GetNumNodes() == 1 );
	CHECK( faceGraph.AddNode(bonePoseNode) == FxFalse );
	CHECK( faceGraph.RemoveNode("testTargetNode") == FxTrue );
	CHECK( faceGraph.GetNumNodes() == 0 );
	CHECK( faceGraph.RemoveNode("noneExistantNode") == FxFalse );

	faceGraph.Clear();
}

UNITTEST( FxFaceGraph, GetNumNodes )
{
	FxFaceGraph faceGraph;
	FxBonePoseNode* bonePoseNode = new FxBonePoseNode();
	bonePoseNode->SetName("testTargetNode");
	CHECK( faceGraph.AddNode(bonePoseNode) == FxTrue );
	CHECK( faceGraph.GetNumNodes() == 1 );
	FxCombinerNode* combinerNode = new FxCombinerNode();
	combinerNode->SetName("testCombinerNode");
	CHECK( faceGraph.AddNode(combinerNode) == FxTrue );
	CHECK( faceGraph.GetNumNodes() == 2 );

	faceGraph.Clear();
}

UNITTEST( FxFaceGraph, Linkage )
{
	FxFaceGraph faceGraph;
	FxBonePoseNode* pBonePoseNode = new FxBonePoseNode();
	pBonePoseNode->SetName("testTargetNode");
	CHECK( faceGraph.AddNode(pBonePoseNode) == FxTrue );
	CHECK( 1 == faceGraph.GetNumNodes() );
	FxCombinerNode* pCombinerNode = new FxCombinerNode();
	pCombinerNode->SetName("testCombinerNode");
	CHECK( faceGraph.AddNode(pCombinerNode) == FxTrue );
	CHECK( 2 == faceGraph.GetNumNodes() );

	FxLinkFnParameters params;
	params.parameters.PushBack(1);

	// Check for links to or from nodes that do not exist in the Face Graph.
	CHECK( LEC_Invalid == faceGraph.Link("testNotANode", "testCombinerNode",
		                                 "linear", params) );
	CHECK( LEC_Invalid == faceGraph.Link("testCombinerNode", "testNotANode",
		                                 "linear", params) );

	// Insert a valid link.
	CHECK( LEC_None == faceGraph.Link("testTargetNode", "testCombinerNode",
		                              "linear", params) );
	CHECK( 1 == pBonePoseNode->GetNumInputLinks() );
	CHECK( pBonePoseNode->GetInputLink(0).GetNode() == pCombinerNode );
	CHECK( 0 == pCombinerNode->GetNumInputLinks() );

	// Check inserting a duplicate link.
	CHECK( LEC_Duplicate == faceGraph.Link("testTargetNode", "testCombinerNode",
		                                   "linear", params) );
	CHECK( 1 == pBonePoseNode->GetNumInputLinks() );
	CHECK( pBonePoseNode->GetInputLink(0).GetNode() == pCombinerNode );
	CHECK( 0 == pCombinerNode->GetNumInputLinks() );

	// Check removing an invalid link.
	CHECK( FxFalse == faceGraph.Unlink("testNotANode", 
		                               "testTargetNode") );

	// Check order-independant un-linkage.
	CHECK( FxTrue == faceGraph.Unlink("testCombinerNode",
					                  "testTargetNode") );

	CHECK( LEC_None == faceGraph.Link("testTargetNode", "testCombinerNode",
		                              "linear", params) );

	// Remove a valid link.
	CHECK( FxTrue == faceGraph.Unlink("testTargetNode",
		                              "testCombinerNode") );
	CHECK( 0 == pBonePoseNode->GetNumInputLinks() );
	CHECK( 0 == pCombinerNode->GetNumInputLinks() );

	CHECK( LEC_None == faceGraph.Link("testTargetNode", "testCombinerNode",
		                              "linear", params) );

	// Modify a link
	FxFaceGraphNodeLink linkInfo;
	CHECK( FxTrue == faceGraph.GetLink("testTargetNode", 
						               "testCombinerNode", 
						               linkInfo) );
	linkInfo.SetLinkFn(FxLinkFn::FindLinkFunction("corrective"));
	CHECK( FxTrue == faceGraph.SetLink("testTargetNode", 
						               "testCombinerNode", 
						               linkInfo) );

	faceGraph.Clear();
}

UNITTEST( FxFaceGraph, CopyConstructionAndAssignment )
{
	// Check assignment and copy construction.
	FxFaceGraph faceGraph1;
	FxFaceGraph faceGraph2;
	FxBonePoseNode* bonePoseNode = new FxBonePoseNode();
	bonePoseNode->SetName("testTargetNode");
	CHECK( faceGraph1.AddNode(bonePoseNode) == FxTrue );
	CHECK( faceGraph1.GetNumNodes() == 1 );
	FxCombinerNode* combinerNode = new FxCombinerNode();
	combinerNode->SetName("testCombinerNode");
	CHECK( faceGraph1.AddNode(combinerNode) == FxTrue );
	CHECK( faceGraph1.GetNumNodes() == 2 );
	CHECK( faceGraph2.GetNumNodes() == 0 );

	CHECK( faceGraph2.AddNode(new FxBonePoseNode()) == FxTrue );
	CHECK( faceGraph2.GetNumNodes() == 1);
	CHECK( faceGraph2.CountNodes(FxBonePoseNode::StaticClass()) == 1 );

	// Insert a valid link.
	FxLinkFnParameters params;
	params.parameters.PushBack(1);
	CHECK( LEC_None == faceGraph1.Link("testTargetNode", "testCombinerNode",
		"linear", params) );
	CHECK( 1 == bonePoseNode->GetNumInputLinks() );
	CHECK( bonePoseNode->GetInputLink(0).GetNode() == combinerNode );
	CHECK( 0 == combinerNode->GetNumInputLinks() );

	faceGraph2 = faceGraph1;
	FxFaceGraph faceGraph2Copy(faceGraph2);

	CHECK( faceGraph2.GetNumNodes() == 2 );
	CHECK( faceGraph2Copy.GetNumNodes() == 2 );
	CHECK( faceGraph2.GetNumNodeTypes() == 2 );
	CHECK( faceGraph2Copy.GetNumNodeTypes() == 2 );
	CHECK( faceGraph2.CountNodes(FxBonePoseNode::StaticClass()) == 1 );
	CHECK( faceGraph2Copy.CountNodes(FxBonePoseNode::StaticClass()) == 1 );
	CHECK( faceGraph2.CountNodes(FxCombinerNode::StaticClass()) == 1 );
	CHECK( faceGraph2Copy.CountNodes(FxCombinerNode::StaticClass()) == 1 );
	CHECK( faceGraph2.FindNode(bonePoseNode->GetName()) != NULL );
	CHECK( faceGraph2Copy.FindNode(bonePoseNode->GetName()) != NULL );
	CHECK( faceGraph2.FindNode(combinerNode->GetName()) != NULL );
	CHECK( faceGraph2Copy.FindNode(combinerNode->GetName()) != NULL );
	FxFaceGraphNode* faceGraph2BonePoseNode = faceGraph2.FindNode(bonePoseNode->GetName());
	FxFaceGraphNode* faceGraph2CopyBonePoseNode = faceGraph2Copy.FindNode(bonePoseNode->GetName());
	FxFaceGraphNode* faceGraph2CombinerNode = faceGraph2.FindNode(combinerNode->GetName());
	FxFaceGraphNode* faceGraph2CopyCombinerNode = faceGraph2Copy.FindNode(combinerNode->GetName());
	CHECK( faceGraph2BonePoseNode );
	CHECK( faceGraph2CopyBonePoseNode );
	CHECK( faceGraph2CombinerNode );
	CHECK( faceGraph2CopyCombinerNode );
	CHECK( 1 == faceGraph2BonePoseNode->GetNumInputLinks() );
	CHECK( 1 == faceGraph2CopyBonePoseNode->GetNumInputLinks() );
	CHECK( 0 == faceGraph2CombinerNode->GetNumInputLinks() );
	CHECK( 0 == faceGraph2CopyCombinerNode->GetNumInputLinks() );
	CHECK( faceGraph2BonePoseNode->GetInputLink(0).GetNode() == faceGraph2CombinerNode );
	CHECK( faceGraph2CopyBonePoseNode->GetInputLink(0).GetNode() == faceGraph2CopyCombinerNode );
	CHECK( faceGraph2BonePoseNode->GetInputLink(0).GetLinkFnName() == "linear" );
	CHECK( faceGraph2CopyBonePoseNode->GetInputLink(0).GetLinkFnName() == "linear" );

	faceGraph1.Clear();
	faceGraph2.Clear();
	faceGraph2Copy.Clear();
}

UNITTEST( FxFaceGraph, Serialization )
{
	//@todo Implement this test.
}

#endif
