//------------------------------------------------------------------------------
// A "compiled" representation of a Face Graph that yields optimal in-game 
// performance over its easy-to-edit counterpart used in the tools.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxCompiledFaceGraph.h"

// For Face Graph decompilation.
#include "FxBonePoseNode.h"
#include "FxCombinerNode.h"
#include "FxGenericTargetNode.h"
#include "FxMorphTargetNode.h"
#include "FxCurrentTimeNode.h"
#include "FxDeltaNode.h"

#ifdef __UNREAL__
	#include "UnFaceFXNode.h"
	#include "UnFaceFXMaterialNode.h"
	#include "UnFaceFXMorphNode.h"
#endif

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxCompiledFaceGraphVersion 0

// The entries in this table must contain one (and only one) entry for each 
// valid node type in the FxCompiledFaceGraphNodeType enum.  It is very 
// important that the order here matches the order found in the 
// FxCompiledFaceGraphNodeType enum!
static const struct
{
	FxCompiledFaceGraphNodeType nodeType;
	const FxChar* nodeClass;
	FxClass::FxConstructObjectFunc constructNodeFunc;	
} FxNodeTypeLookupTable[] =
{
	{
		NT_Combiner,
		"FxCombinerNode",
		&FxCombinerNode::ConstructObject
	},
	{
		NT_Delta, 
		"FxDeltaNode",
		&FxDeltaNode::ConstructObject
	},
	{
		NT_CurrentTime,
		"FxCurrentTimeNode",
		&FxCurrentTimeNode::ConstructObject
	},
	{
		NT_GenericTarget,
		"FxGenericTargetNode",
		&FxGenericTargetNode::ConstructObject,
	},
	{
		NT_BonePose,
		"FxBonePoseNode",
		&FxBonePoseNode::ConstructObject
	},
	{
		NT_MorphTarget,
		"FxMorphTargetNode",
		&FxMorphTargetNode::ConstructObject
	},
	{
		NT_MaterialParameterUE3,
		"FUnrealFaceFXMaterialParameterNode",
#ifdef __UNREAL__
		&FUnrealFaceFXMaterialParameterNode::ConstructObject
#else
		NULL
#endif // __UNREAL__
	},
	{
		NT_MorphTargetUE3,
		"FUnrealFaceFXMorphNode",
#ifdef __UNREAL__
		&FUnrealFaceFXMorphNode::ConstructObject
#else
		NULL
#endif // __UNREAL__
	}
};

// The entries in this table must contain one (and only one) entry for each 
// valid node type in the FxCompiledFaceGraphNodeType enum.  It is very 
// important that the order here matches the order found in the 
// FxCompiledFaceGraphNodeType enum!
static const struct
{
	FxCompiledFaceGraphNodeType nodeType;
	FxBool bIsRelevantForCollapse;
} FxNodeRelevanceLookupTable[] =
{
	{
		NT_Combiner,
		FxFalse
	},
	{
		NT_Delta,
		FxFalse
	},
	{
		NT_CurrentTime,
		FxFalse
	},
	{
		NT_GenericTarget,
		FxFalse
	},
	{
		NT_BonePose,
		FxTrue
	},
	{
		NT_MorphTarget,
		FxTrue
	},
	{
		NT_MaterialParameterUE3,
		FxTrue
	},
	{
		NT_MorphTargetUE3,
		FxTrue
	}
};

FxCompiledFaceGraphLink::FxCompiledFaceGraphLink()
	: nodeIndex(FxInvalidIndex)
	, linkFnType(LFT_Invalid)
{
}

FxCompiledFaceGraphLink
::FxCompiledFaceGraphLink( FxSize iNodeIndex, FxLinkFnType iLinkFnType, 
						   const FxLinkFnParameters& iLinkFnParams )
						   : nodeIndex(iNodeIndex)
						   , linkFnType(iLinkFnType)
						   , linkFnParams(iLinkFnParams)
{
	FxAssert(FxInvalidIndex != nodeIndex);
	FxAssert(LFT_Invalid != linkFnType);
}

FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraphLink& link )
{
	FxInt32 tempLinkFnType = static_cast<FxInt32>(link.linkFnType);
	arc << link.nodeIndex << tempLinkFnType << link.linkFnParams;
	link.linkFnType = static_cast<FxLinkFnType>(tempLinkFnType);
	FxAssert(LFT_Invalid != link.linkFnType);
	if( LFT_Null == link.linkFnType )
	{
		link.linkFnParams.parameters.Clear();
	}
	return arc;
}

FxCompiledFaceGraphNode::FxCompiledFaceGraphNode()
	: nodeType(NT_Invalid)
	, name(FxName::NullName)
	, nodeMin(0.0f)
	, oneOverNodeMin(0.0f)
	, nodeMax(0.0f)
	, oneOverNodeMax(0.0f)
	, trackValue(0.0f)
	, userValue(0.0f)
	, finalValue(0.0f)
	, inputOperation(IO_Invalid)
	, userValueOperation(VO_Invalid)
	, pUserData(NULL)
{
}

FxCompiledFaceGraphNode
::FxCompiledFaceGraphNode( const FxFaceGraphNode* pNode, 
 						   const FxCompiledFaceGraph* pCG )
{
	FxAssert(pNode);
	FxAssert(pCG);
	nodeType                 = _determineNodeType(pNode); 
	                           FxAssert(NT_Invalid != nodeType);
	name                     = pNode->GetName();
	nodeMin                  = pNode->GetMin();
	oneOverNodeMin           = (nodeMin > 0.0f || nodeMin < 0.0f) 
		                       ? 1.0f / nodeMin : 1.0f;
	nodeMax                  = pNode->GetMax();
	oneOverNodeMax           = (nodeMax > 0.0f || nodeMax < 0.0f) 
		                       ? 1.0f / nodeMax : 1.0f;
	trackValue               = userValue = finalValue = 0.0f;
	inputOperation           = pNode->GetNodeOperation(); 
	                           FxAssert(IO_Invalid != inputOperation);
	userValueOperation       = VO_None;
	pUserData                = NULL;
	FxSize numUserProperties = pNode->GetNumUserProperties();
	userProperties.Reserve(numUserProperties);
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		userProperties.PushBack(pNode->GetUserProperty(i));
	}
	FxSize numInputLinks = pNode->GetNumInputLinks();
	inputLinks.Reserve(numInputLinks);
	for( FxSize i = 0; i < numInputLinks; ++i )
	{
		const FxFaceGraphNodeLink& link = pNode->GetInputLink(i);
		inputLinks.PushBack(FxCompiledFaceGraphLink(
							pCG->FindNodeIndex(link.GetNodeName().GetAsCstr()),
							link.GetLinkFnType(),
							link.GetLinkFnParams()));
	}
}

FxFaceGraphNode* FxCompiledFaceGraphNode::Decompile( void ) const
{
	FxFaceGraphNode* pDecompiledNode = FxCast<FxFaceGraphNode>(FxNodeTypeLookupTable[nodeType].constructNodeFunc());
	FxAssert(pDecompiledNode);
	pDecompiledNode->SetName(name);
	pDecompiledNode->SetMin(nodeMin);
	pDecompiledNode->SetMax(nodeMax);
	pDecompiledNode->SetNodeOperation(inputOperation);
	FxSize numUserProperties = userProperties.Length();
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		pDecompiledNode->ReplaceUserProperty(userProperties[i]);
	}
	return pDecompiledNode;
}

FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraphNode& node )
{
	FxInt32 tempNodeType = static_cast<FxInt32>(node.nodeType);
	FxInt32 tempInputOp  = static_cast<FxInt32>(node.inputOperation);
	arc << tempNodeType << node.name << node.nodeMin << node.oneOverNodeMin 
		<< node.nodeMax << node.oneOverNodeMax << tempInputOp << node.inputLinks 
		<< node.userProperties;
	node.nodeType = static_cast<FxCompiledFaceGraphNodeType>(tempNodeType);
	node.inputOperation = static_cast<FxInputOp>(tempInputOp);

	return arc;
}

FxCompiledFaceGraphNodeType FxCompiledFaceGraphNode
::_determineNodeType( const FxFaceGraphNode* pNode ) const
{
	FxAssert(pNode);
	FxAssert(pNode->GetClassDesc());
	const FxName& nodeClassName = pNode->GetClassDesc()->GetName();
	for( int i = NT_First; i < NT_NumNodeTypes; ++i )
	{
		if( nodeClassName == FxNodeTypeLookupTable[i].nodeClass )
		{
			return FxNodeTypeLookupTable[i].nodeType;
		}
	}
	return NT_Invalid;
}

void FX_CALL FxDefaultPreDestroyCallbackFunc   ( FxCompiledFaceGraph& FxUnused(cg) ) {}
void FX_CALL FxDefaultPreCompileCallbackFunc   ( FxCompiledFaceGraph& FxUnused(cg), FxFaceGraph& FxUnused(faceGraph) ) {}
void FX_CALL FxDefaultPostCompileCallbackFunc  ( FxCompiledFaceGraph& FxUnused(cg), FxFaceGraph& FxUnused(faceGraph) ) {}
void FX_CALL FxDefaultPreDecompileCallbackFunc ( FxCompiledFaceGraph& FxUnused(cg), FxFaceGraph& FxUnused(faceGraph) ) {}
void FX_CALL FxDefaultPostDecompileCallbackFunc( FxCompiledFaceGraph& FxUnused(cg), FxFaceGraph& FxUnused(faceGraph) ) {}

static FxCompiledFaceGraph::FxPreDestroyCallbackFunc  _preDestroy    = &FxDefaultPreDestroyCallbackFunc;
static FxCompiledFaceGraph::FxCompilationCallbackFunc _preCompile    = &FxDefaultPreCompileCallbackFunc;
static FxCompiledFaceGraph::FxCompilationCallbackFunc _postCompile   = &FxDefaultPostCompileCallbackFunc;
static FxCompiledFaceGraph::FxCompilationCallbackFunc _preDecompile  = &FxDefaultPreDecompileCallbackFunc;
static FxCompiledFaceGraph::FxCompilationCallbackFunc _postDecompile = &FxDefaultPostDecompileCallbackFunc;

FxCompiledFaceGraph::FxCompiledFaceGraph() {}

FxCompiledFaceGraph::~FxCompiledFaceGraph()
{
	FxAssert(_preDestroy);
	_preDestroy(*this);
	// Clear will also check for still valid custom node user data objects and
	// assert in debug mode.
	Clear();
}

void FX_CALL FxCompiledFaceGraph
::SetPreDestroyCallback( FxPreDestroyCallbackFunc preDestroy )
{
	if( preDestroy ) { _preDestroy = preDestroy; }
	FxAssert(_preDestroy);
}

void FX_CALL FxCompiledFaceGraph
::SetCompilationCallbacks( FxCompilationCallbackFunc preCompile,
						   FxCompilationCallbackFunc postCompile,
						   FxCompilationCallbackFunc preDecompile,
						   FxCompilationCallbackFunc postDecompile )
{
	if( preCompile    ) { _preCompile    = preCompile; }
	if( postCompile   ) { _postCompile   = postCompile; }
	if( preDecompile  ) { _preDecompile  = preDecompile; }
	if( postDecompile ) { _postDecompile = postDecompile; }
	FxAssert(_preCompile);
	FxAssert(_postCompile);
	FxAssert(_preDecompile);
	FxAssert(_postDecompile);
}

FxBool FX_CALL FxCompiledFaceGraph
::IsNodeTypeRelevantForCollapse( FxCompiledFaceGraphNodeType nodeType )
{
	return FxNodeRelevanceLookupTable[nodeType].bIsRelevantForCollapse;
}

void FxCompiledFaceGraph::Clear( void )
{
	_nodeHash.Clear();
#ifdef FX_DEBUG
	// Make sure no user data pointers are still valid.
	FxSize numNodes = nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxAssert(!nodes[i].pUserData);
	}
#endif // FX_DEBUG
	nodes.Clear();
}

void FxCompiledFaceGraph
::Tick( const FxReal time, FxArray<FxRegister>& registers, FxBool& hasBeenTicked )
{
	nodes.Prefetch();
	registers.Prefetch();

	FxSize numNodes = nodes.Length();
	FxAssert(numNodes == registers.Length());
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxCompiledFaceGraphNode& node  = nodes[i];

		// Only do register processing if the register is not in the shut off
		// state.
		if( VO_None != registers[i].firstRegOp )
		{
			if( !registers[i].isInterpolating )
			{
				registers[i].interpStartTime = time;
				registers[i].isInterpolating = FxTrue;
			}
			if( FxRealEquality(registers[i].interpInverseDuration, 0.0f) )
			{
				registers[i].interpLastValue = registers[i].interpEndValue;
				// If there is a "next" interpolation operation and it is time to
				// start it, promote the "next" interpolation parameters up to the
				// "current" interpolation parameters.
				if( (time - registers[i].interpStartTime) > 0.0f )
				{
					if( RO_Invalid != registers[i].nextRegOp )
					{
						registers[i].interpStartTime          = time;
						registers[i].isInterpolating          = FxTrue;
						registers[i].interpStartValue         = (RO_LoadAdd      == registers[i].nextRegOp || 
							                                     RO_LoadMultiply == registers[i].nextRegOp || 
							                                     RO_LoadReplace  == registers[i].nextRegOp) 
							                                     ? registers[i].value 
							                                     : registers[i].interpEndValue;
						registers[i].interpEndValue            = registers[i].interpNextEndValue;
						registers[i].interpInverseDuration     = registers[i].interpNextInverseDuration;
						registers[i].firstRegOp                = FxRegister::GetUserOpForRegOp(registers[i].nextRegOp);
						registers[i].nextRegOp                 = RO_Invalid;
						registers[i].interpNextEndValue        = 0.0f;
						registers[i].interpNextInverseDuration = 0.0f;
					}
				}
			}
			else
			{
				FxReal parametricTime = (time - registers[i].interpStartTime) * 
					                    registers[i].interpInverseDuration;
				// If there is a "next" interpolation operation and it is time to
				// start it, promote the "next" interpolation parameters up to the
				// "current" interpolation parameters.
				if( RO_Invalid != registers[i].nextRegOp && parametricTime >= 1.0f )
				{
					registers[i].interpStartTime          = time;
					registers[i].isInterpolating          = FxTrue;
					registers[i].interpStartValue         = (RO_LoadAdd      == registers[i].nextRegOp || 
						                                     RO_LoadMultiply == registers[i].nextRegOp || 
						                                     RO_LoadReplace  == registers[i].nextRegOp) 
						                                     ? registers[i].value 
						                                     : registers[i].interpEndValue;
					registers[i].interpEndValue            = registers[i].interpNextEndValue;
					registers[i].interpInverseDuration     = registers[i].interpNextInverseDuration;
					registers[i].firstRegOp                = FxRegister::GetUserOpForRegOp(registers[i].nextRegOp);
					registers[i].nextRegOp                 = RO_Invalid;
					registers[i].interpNextEndValue		   = 0.0f;
					registers[i].interpNextInverseDuration = 0.0f;
					parametricTime                         = 0.0f;
				}
				registers[i].interpLastValue = FxHermiteInterpolate(registers[i].interpStartValue, 
					                                                registers[i].interpEndValue, 
					                                                parametricTime);
			}
		}

		node.userValueOperation = registers[i].firstRegOp;
		node.userValue          = registers[i].interpLastValue;
		
		switch( node.nodeType )
		{
		case NT_CurrentTime:
			{
				FxReal value = time;

				// Operate with the user-defined value.
				switch( node.userValueOperation )
				{
				case VO_None:
					break;
				case VO_Multiply:
					value *= node.userValue;
					break;
				case VO_Add:
					value += node.userValue;
					break;
				case VO_Replace:
					value = node.userValue;
					break;
				case VO_Invalid:
				default:
					FxAssert(!"Invalid value operation");
					break;
				}
				node.userValue  = 0.0f;
				node.finalValue = value;
			}
			break;
		case NT_Delta:
			{
				node.finalValue = 0.0f;
				if( node.inputLinks.Length() > 0 )
				{
					const FxCompiledFaceGraphLink& inputLink = node.inputLinks[0];
					const FxCompiledFaceGraphNode& inputNode = nodes[inputLink.nodeIndex];
					
					FxReal currentValue = FxLinkFnEval(inputLink.linkFnType, 
								                       inputNode.finalValue, 
							                           inputLink.linkFnParams);
					if( hasBeenTicked )
					{
						node.finalValue = currentValue - registers[i].scratch;
					}
					registers[i].scratch = currentValue;
				}
			}
			break;
		case NT_Combiner:
		case NT_GenericTarget:
		case NT_BonePose:
		case NT_MorphTarget:
		case NT_MaterialParameterUE3:
		case NT_MorphTargetUE3:
		default:
			{
				// Initialize value.  Use 0.0 as a starting point for summing 
				// and 1.0 as a starting point for multiplying.
				static const FxReal initialValueLookup[] = {0.0f, 1.0f, -FX_REAL_MAX, FX_REAL_MAX};
				FxAssert(IO_Invalid != node.inputOperation);
				
				FxReal value = 0.0f;

				switch( node.inputLinks.Length() )
				{
				case 0:
					// If there are zero inputs, do nothing.
					break;
				default:
					// If there are inputs, look up the initial value for the
					// node.
					value = initialValueLookup[node.inputOperation];
					break;
				}

				FxReal correction = 0.0f;
				node.inputLinks.Prefetch();
				FxSize numInputs = node.inputLinks.Length();
				for( FxSize j = 0; j < numInputs; ++j )
				{
					const FxCompiledFaceGraphLink& inputLink = node.inputLinks[j];
					const FxCompiledFaceGraphNode& inputNode = nodes[inputLink.nodeIndex];
					
					switch( inputLink.linkFnType )
					{
					default:
						{
							FxReal transformedValue = FxLinkFnEval(inputLink.linkFnType, 
																   inputNode.finalValue, 
																   inputLink.linkFnParams);
							switch( node.inputOperation )
							{
							case IO_Sum:
								value += transformedValue;
								break;
							case IO_Multiply:
								value *= transformedValue;
								break;
							case IO_Max:
								value = FxMax(transformedValue, value);
								break;
							case IO_Min:
								value = FxMin(transformedValue, value);
								break;
							case IO_Invalid:
							default:
								FxAssert(!"Invalid input operation");
								break;
							}
						}
						break;
					case LFT_Corrective:
						{
							FxReal correctedValueRight = inputNode.finalValue  * inputNode.oneOverNodeMax;
							FxReal correctedValueLeft  = -inputNode.finalValue * inputNode.oneOverNodeMin;
							FxReal correctedValue = FxMax(correctedValueRight, correctedValueLeft);
							correction = FxMin(1.0f, correction + (correctedValue * inputLink.linkFnParams.parameters[0]));
						}
						break;
					}
				}

				// Add in the offset track value.
				value += node.trackValue;
				node.trackValue = 0.0f;

				// Operate with the user-defined value.
				switch( node.userValueOperation )
				{
				case VO_None:
					break;
				case VO_Multiply:
					value *= node.userValue;
					break;
				case VO_Add:
					value += node.userValue;
					break;
				case VO_Replace:
					value = node.userValue;
					break;
				case VO_Invalid:
				default:
					FxAssert(!"Invalid value operation");
					break;
				}
				node.userValue = 0.0f;
				
				// Correct the value.
				value *= (1.0f - correction);
					
				// Clamp the value.
				node.finalValue = FxClamp(node.nodeMin, value, node.nodeMax);
				registers[i].value = node.finalValue;
			}
			break;
		}
		registers[i].value = node.finalValue;
	}
	hasBeenTicked = FxTrue;
}

void FxCompiledFaceGraph::Compile( FxFaceGraph& faceGraph )
{
	FxAssert(_preCompile);
	_preCompile(*this, faceGraph);

	Clear();

	FxSize numNodes = faceGraph.GetNumNodes();
	
	FxArray<const FxFaceGraphNode*> zeroOutputNodes;
	zeroOutputNodes.Reserve(numNodes);
	FxFaceGraph::Iterator iter = faceGraph.Begin(FxFaceGraphNode::StaticClass());
	FxFaceGraph::Iterator iterEnd = faceGraph.End(FxFaceGraphNode::StaticClass());
	for( ; iter != iterEnd; ++iter )
	{
		if( 0 == iter.GetNode()->GetNumOutputs() )
		{
			zeroOutputNodes.PushBack(iter.GetNode());
		}
	}

	nodes.Reserve(numNodes);
	FxSize numZeroOutputNodes = zeroOutputNodes.Length();

	// First pass: add all nodes with zero inputs and zero outputs.
	for( FxSize i = 0; i < numZeroOutputNodes; ++i )
	{
		FxAssert(zeroOutputNodes[i]);
		if( 0 == zeroOutputNodes[i]->GetNumInputLinks() )
		{
			// No need to check for duplicates in this pass.
			nodes.PushBack(FxCompiledFaceGraphNode(zeroOutputNodes[i], this));
			_nodeHash.Insert(zeroOutputNodes[i]->GetNameAsCstr(), nodes.Length()-1);
		}
	}

	// Second pass: add all other nodes in optimal traversal order.
	for( FxSize i = 0; i < numZeroOutputNodes; ++i )
	{
		FxAssert(zeroOutputNodes[i]);
		FxArray<const FxFaceGraphNode*> traversal;
		FxStringHash<FxSize, 11> traversalHash;
		_compileNode(zeroOutputNodes[i], traversal, traversalHash);
		FxSize numTraversed = traversal.Length();
		for( FxSize j = 0; j < numTraversed; ++j )
		{
			FxAssert(traversal[j]);
			// Make sure there are no duplicates.
			if( FxInvalidIndex == FindNodeIndex(traversal[j]->GetName()) )
			{
				// When we get here all of this node's inputs should have
				// already been compiled so the constructor should be
				// able to process all of them.
				nodes.PushBack(FxCompiledFaceGraphNode(traversal[j], this));
				_nodeHash.Insert(traversal[j]->GetNameAsCstr(), nodes.Length()-1);
			}
		}
	}

	FxAssert(numNodes == nodes.Length());

	FxAssert(_postCompile);
	_postCompile(*this, faceGraph);
}

void FxCompiledFaceGraph::Decompile( FxFaceGraph& faceGraph )
{
	FxAssert(_preDecompile);
	_preDecompile(*this, faceGraph);

	faceGraph.Clear();

	// First pass: add all of the nodes.
	FxSize numNodes = nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		faceGraph.AddNode(nodes[i].Decompile());
	}

	// Second pass: add all of the links.
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxSize numInputs = nodes[i].inputLinks.Length();
		for( FxSize j = 0; j < numInputs; ++j )
		{
			FxSize fromNodeIndex = nodes[i].inputLinks[j].nodeIndex;
			const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunctionByType(nodes[i].inputLinks[j].linkFnType);
			FxAssert(pLinkFn);
			const FxLinkFnParameters& linkFnParams = nodes[i].inputLinks[j].linkFnParams;
			FxLinkErrorCode ec = LEC_Invalid;
			ec = faceGraph.Link(nodes[i].name, nodes[fromNodeIndex].name, pLinkFn->GetName(), linkFnParams);
			FxAssert(LEC_None == ec);
		}
	}

	FxAssert(_postDecompile);
	_postDecompile(*this, faceGraph);
}

FxSize FxCompiledFaceGraph::FindNodeIndex( const FxName& name ) const
{
	FxSize nodeIndex = FxInvalidIndex;
	if( _nodeHash.Find(name.GetAsCstr(), nodeIndex) )
	{
#ifdef FX_XBOX_360
		// It is a safe assumption that the node will be accessed pretty soon 
		// after this, so go ahead and bring it into L1 and L2.
		const FxCompiledFaceGraphNode* FX_RESTRICT pNodes = nodes.GetData();
		__dcbt(nodeIndex*sizeof(FxCompiledFaceGraphNode), pNodes);
#endif // FX_XBOX_360
	}
	return nodeIndex;
}

FxSize FxCompiledFaceGraph
::CountNodesOfType( const FxCompiledFaceGraphNodeType nodeType ) const
{
	FxSize numNodesOfType = 0;
	FxSize numNodes = nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( nodeType == nodes[i].nodeType )
		{
			numNodesOfType++;
		}
	}
	return numNodesOfType;
}

FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraph& cg )
{
	arc.SerializeClassVersion("FxCompiledFaceGraph", FxTrue, kCurrentFxCompiledFaceGraphVersion);
	arc << cg.nodes << cg._nodeHash;
	return arc;
}

void FxCompiledFaceGraph
::_compileNode( const FxFaceGraphNode* pNode, 
			    FxArray<const FxFaceGraphNode*>& traversal,
				FxStringHash<FxSize, 11>& traversalHash )
{
	FxAssert(pNode);
	if( traversalHash.Find(pNode->GetNameAsCstr()) )
	{
		return;
	}
	FxSize numInputs = pNode->GetNumInputLinks();
	if( numInputs > 0 )
	{
		for( FxSize i = 0; i < numInputs; ++i )
		{
			_compileNode(pNode->GetInputLink(i).GetNode(), traversal, traversalHash);
		}
		traversal.PushBack(pNode);
		traversalHash.Insert(pNode->GetNameAsCstr(), traversal.Length()-1);
	}
	else
	{
		traversal.PushBack(pNode);
		traversalHash.Insert(pNode->GetNameAsCstr(), traversal.Length()-1);
	}
}

} // namespace Face

} // namespace OC3Ent
