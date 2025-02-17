//------------------------------------------------------------------------------
// The DLL wrapper for the graph layout algorithm.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "FxGraphLayout.h"
#include "FxFaceGraphNodeUserData.h"
#include "FxArray.h"
#include "FxUtil.h"
#include "FxSDK.h"

#ifdef __UNREAL__
#pragma pack(push, 8)
#endif // __UNREAL__
	#include "dotneato.h"
	#include "cdt.h"
#ifdef __UNREAL__
#pragma pack(pop)
#endif // __UNREAL__

#include <cstdlib>

BOOL 
APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	return TRUE;
}

// A little utility function to find a node in the array.
OC3Ent::Face::FxSize FindNode( const OC3Ent::Face::FxArray<Agnode_t*>& shadowNodes, const OC3Ent::Face::FxString& toFind )
{
	for( OC3Ent::Face::FxSize i = 0; i < shadowNodes.Length(); ++i )
	{
		if( OC3Ent::Face::FxString(shadowNodes[i]->name) == toFind ) return i;
	}
	return OC3Ent::Face::FxInvalidIndex;
}

// Finds a node in the face graph.
OC3Ent::Face::FxFaceGraphNode* FindNodeInGraph( const OC3Ent::Face::FxString& nodeName, OC3Ent::Face::FxFaceGraph* pGraph )
{
	OC3Ent::Face::FxFaceGraph::Iterator curr = pGraph->Begin(OC3Ent::Face::FxFaceGraphNode::StaticClass());
	OC3Ent::Face::FxFaceGraph::Iterator end  = pGraph->End(OC3Ent::Face::FxFaceGraphNode::StaticClass());
	for( ; curr != end; ++curr )
	{
		if( curr.GetNode()->GetNameAsString() == nodeName )
		{
			return curr.GetNode();
		}
	}
	return NULL;
}

FX_GRAPH_LAYOUT_API void DoLayout( OC3Ent::Face::FxFaceGraph* pFaceGraph )
{
	OC3Ent::Face::FxMemoryAllocationPolicy defaultAllocationPolicy;
	OC3Ent::Face::FxSDKStartup(defaultAllocationPolicy);
	if( pFaceGraph )
	{
		Agraph_t* graph;
		OC3Ent::Face::FxArray<Agnode_t*> nodes;
		OC3Ent::Face::FxArray<Agedge_t*> edges;

		aginit();

		graph = agopen("ShadowGraph", AGDIGRAPHSTRICT);

		agnodeattr(graph, "height", "0.25");
		agnodeattr(graph, "width", "0.40");
		agnodeattr(graph, "shape", "record");
		agnodeattr(graph, "label", "");
		agraphattr(graph, "nodesep", "0.00");
		OC3Ent::Face::FxReal numNodes = static_cast<OC3Ent::Face::FxReal>(pFaceGraph->GetNumNodes());
		OC3Ent::Face::FxReal count    = 0.0f;
		OC3Ent::Face::FxFaceGraph::Iterator curr = pFaceGraph->Begin(OC3Ent::Face::FxFaceGraphNode::StaticClass());
		OC3Ent::Face::FxFaceGraph::Iterator end  = pFaceGraph->End(OC3Ent::Face::FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() )
			{
				unsigned int tempLen = curr.GetNode()->GetNameAsString().Length()+1;
				char* temp = new char[tempLen];
				OC3Ent::Face::FxStrcpy(temp, tempLen, curr.GetNode()->GetNameAsCstr());
				nodes.PushBack(agnode(graph, temp));
				char* height = new char[255];
#if _MSC_VER >= 1400
				sprintf_s(height, 255, "%f", GetNodeUserData(curr.GetNode())->bounds.GetHeight()/50.0f);
#else
				sprintf(height, "%f", GetNodeUserData(curr.GetNode())->bounds.GetHeight()/50.0f);
#endif
				agset(nodes.Back(), "height", height);
				delete [] height;
				char* width = new char[255];
#if _MSC_VER >= 1400
				sprintf_s(width, 255, "%f", GetNodeUserData(curr.GetNode())->bounds.GetWidth()/50.0f );
#else
				sprintf(width, "%f", GetNodeUserData(curr.GetNode())->bounds.GetWidth()/50.0f );
#endif
				agset(nodes.Back(), "width", width);
				delete [] width;
				OC3Ent::Face::FxInt32 size = OC3Ent::Face::FxMax(curr.GetNode()->GetNumInputLinks(), curr.GetNode()->GetNumOutputs()) + 1;
				OC3Ent::Face::FxString label = "";
				for( OC3Ent::Face::FxInt32 i = 0; i < size; ++i )
				{
					char* buf = new char[255];
#if _MSC_VER >= 1400
					sprintf_s(buf, 255, "<p%d> Port%d", i, i);
#else
					sprintf(buf, "<p%d> Port%d", i, i);
#endif
					label = OC3Ent::Face::FxString::Concat(label, buf);
					if( i != size - 1 ) label =  OC3Ent::Face::FxString::Concat(label, "|");
					delete [] buf;
				}
				delete [] temp;
				unsigned int tempLen2 = label.Length() + 1;
				temp = new char[tempLen2];
				OC3Ent::Face::FxStrcpy(temp, tempLen2, label.GetData());
				agset(nodes.Back(), "label", temp);
				delete [] temp;
				temp = NULL;
			}
			++count;
		}

		agedgeattr(graph, "headport", "center");
		agedgeattr(graph, "tailport", "center");
		curr = pFaceGraph->Begin(OC3Ent::Face::FxFaceGraphNode::StaticClass());
		end  = pFaceGraph->End(OC3Ent::Face::FxFaceGraphNode::StaticClass());
		count = 0.0f;
		for( ; curr != end; ++curr )
		{
			OC3Ent::Face::FxFaceGraphNode* currNode = curr.GetNode();
			if( currNode )
			{
				for( OC3Ent::Face::FxSize i = 0; i < currNode->GetNumInputLinks(); ++i )
				{
					edges.PushBack(agedge(graph, 
						nodes[FindNode(nodes, currNode->GetInputLink(i).GetNodeName().GetAsString())],
						nodes[FindNode(nodes, currNode->GetNameAsString())]));
					char* temp = new char[16];
#if _MSC_VER >= 1400
					sprintf_s(temp, 16, "p%d", i);
#else
					sprintf(temp, "p%d", i);
#endif
					agset(edges.Back(), "headport", temp);
					delete [] temp;

					// Some magic to find the tail's port.
					OC3Ent::Face::FxFaceGraphNode* otherNode = const_cast<OC3Ent::Face::FxFaceGraphNode*>(currNode->GetInputLink(i).GetNode());
					OC3Ent::Face::FxArray<OC3Ent::Face::FxLinkEnd*>& links = GetNodeUserData(otherNode)->outputLinkEnds;
					temp = new char[16];
					for( OC3Ent::Face::FxSize j = 0; j < links.Length(); ++j )
					{
						if( FindNodeInGraph(links[j]->otherNode.GetAsString(), pFaceGraph) == currNode )
						{
#if _MSC_VER >= 1400
							sprintf_s(temp, 16, "p%d", j);
#else
							sprintf(temp, "p%d", j);
#endif
						}
					}

					agset(edges.Back(), "tailport", temp);
					delete [] temp;
				}
			}
			++count;
		}

		agraphattr(graph, "clusterrank", "local");
		agraphattr(graph, "rankdir", "LR");
		agraphattr(graph, "ranksep", "2.0");

		dot_layout(graph);

		for( OC3Ent::Face::FxSize i = 0; i < nodes.Length(); ++i )
		{
			OC3Ent::Face::FxFaceGraphNode* pNode = FindNodeInGraph(OC3Ent::Face::FxString(nodes[i]->name), pFaceGraph);
			if( pNode )
			{
				GetNodeUserData(pNode)->bounds.SetPosition(OC3Ent::Face::FxPoint(ND_coord_i(nodes[i]).x, -ND_coord_i(nodes[i]).y));
			}
		}

		dot_cleanup(graph);
		agclose(graph);
	}
	OC3Ent::Face::FxSDKShutdown();
}
