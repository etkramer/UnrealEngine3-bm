//------------------------------------------------------------------------------
// A "compiled" representation of a Face Graph that yields optimal in-game 
// performance over its easy-to-edit counterpart used in the tools.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCompiledFaceGraph_H__
#define FxCompiledFaceGraph_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxFaceGraphShared.h"
#include "FxFaceGraphNode.h"
#include "FxFaceGraph.h"
#include "FxLinkFn.h"
#include "FxStringHash.h"

namespace OC3Ent
{

namespace Face
{

/// A link between two nodes in a compiled face graph.
/// \ingroup faceGraph
struct FxCompiledFaceGraphLink
{
	/// Default constructor.
	FxCompiledFaceGraphLink();
	/// Constructor.
	FxCompiledFaceGraphLink( FxSize iNodeIndex, FxLinkFnType iLinkFnType, 
	                         const FxLinkFnParameters& iLinkFnParams );

	/// The index of the input node in the compiled face graph nodes array.
	FxSize nodeIndex;
	/// The type of link function used by the link.
	FxLinkFnType linkFnType;
	/// The link function parameters used by the link.
	FxLinkFnParameters linkFnParams;

	/// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, 
		                          FxCompiledFaceGraphLink& link );
};

struct FxCompiledFaceGraph;

/// A type of compiled face graph node.
/// \ingroup faceGraph
enum FxCompiledFaceGraphNodeType
{
	NT_Invalid = -1,						 ///< Invalid.
	NT_Combiner,							 ///< Combiner node.
	NT_Delta,								 ///< Delta node.
	NT_CurrentTime,							 ///< Current time node.
	NT_GenericTarget,						 ///< Generic target node.
	NT_BonePose,							 ///< Bone pose node.
	NT_MorphTarget,							 ///< Morph target node.
	NT_MaterialParameterUE3,				 ///< Unreal Engine specific.
	NT_MorphTargetUE3,						 ///< Unreal Engine specific.

	NT_First        = NT_Combiner,			 ///< First compiled node type.
	NT_Last         = NT_MorphTargetUE3,	 ///< Last compiled node type.
	NT_NumNodeTypes = NT_Last - NT_First + 1 ///< Number of compiled node types.
};

/// A node in a compiled face graph.
/// \ingroup faceGraph
struct FxCompiledFaceGraphNode
{
	/// Default constructor.
	FxCompiledFaceGraphNode();
	/// Constructor.
	FxCompiledFaceGraphNode( const FxFaceGraphNode* pNode, 
	                         const FxCompiledFaceGraph* pCG );

	/// Decompiles the compiled node.
	/// \return A pointer to a decompiled FxFaceGraphNode representation of the
	/// compiled node.
	FxFaceGraphNode* Decompile( void ) const;
	
	/// The node type.
	FxCompiledFaceGraphNodeType nodeType;
	/// The node name.
	FxName name;
	/// The minimum value the node can assume.
	FxReal nodeMin;
	/// Pre-computed 1.0f / nodeMin.
	FxReal oneOverNodeMin;
	/// The maximum value the node can assume.
	FxReal nodeMax;
	/// Pre-computed 1.0f / nodeMax.
	FxReal oneOverNodeMax;
	/// The value of any animation track that controls the node.
	FxReal trackValue;
	/// The user value of the node (e.g. register value).
	FxReal userValue;
	/// The final value of the node.
	FxReal finalValue;
	/// The type of input operation performed on incoming face graph links.
	FxInputOp inputOperation;
	/// The type of operation performed on the user value (e.g. register value).
	FxValueOp userValueOperation;
	/// User data that can be attached to the node.  Licensees may use this to
	/// attach custom, engine-specific data to any new or existing node types.
	/// \note You will need to set up the compilation call back functions so 
	/// that the user data can be properly cleaned up (see the example in
	/// FxRenderWidgetOC3.cpp).  There are 4 callbacks that licensees may hook 
	/// into, but the most useful one is the pre-compilation call back.  The 
	/// others may be set to NULL if overriding their functionality is not 
	/// required.
	void* pUserData;
	/// The input face graph links.
	FxArray<FxCompiledFaceGraphLink> inputLinks;
	/// The node user properties.
	FxArray<FxFaceGraphNodeUserProperty> userProperties;

	/// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, 
		                          FxCompiledFaceGraphNode& node );

private:
	/// Determines the type of the passed in FxFaceGraphNode and converts it to
	/// one of the FxCompiledFaceGraphNodeType values.
	/// \param pNode The node whose type is requested.
	/// \return The FxCompiledFaceGraphNodeType value that matches the node type
	/// of pNode.
	FxCompiledFaceGraphNodeType 
		_determineNodeType( const FxFaceGraphNode* pNode ) const;
};

/// A compiled face graph.
/// \ingroup faceGraph
struct FxCompiledFaceGraph
{
	/// The nodes in the compiled face graph.
	FxArray<FxCompiledFaceGraphNode> nodes;

	/// Default constructor.
	FxCompiledFaceGraph();
	/// Destructor.
	~FxCompiledFaceGraph();

	/// The function signature of the pre-destruction callback.
	typedef void (FX_CALL *FxPreDestroyCallbackFunc)( FxCompiledFaceGraph& cg );
	/// The function signature of the face graph compilation callbacks.
	typedef void (FX_CALL *FxCompilationCallbackFunc)( FxCompiledFaceGraph& cg, 
													   FxFaceGraph& faceGraph );

	/// Sets the pre-destroy callback function.  Use this to properly 
	/// destroy and clean up any attached custom node user data objects.
	/// \param preDestroy Is called just before destruction.
	static void FX_CALL 
		SetPreDestroyCallback( FxPreDestroyCallbackFunc preDestroy );

	/// Sets the compilation callback functions.  Use these to properly destroy
	/// and clean up any attached custom node user data objects.
	/// \param preCompile Is called before compilation begins.
	/// \param postCompile Is called after compilation is completed.
	/// \param preDecompile Is called before decompilation begins.
	/// \param postDecompile Is called after decompilation is completed.
	static void FX_CALL 
		SetCompilationCallbacks( FxCompilationCallbackFunc preCompile,
					             FxCompilationCallbackFunc postCompile,
								 FxCompilationCallbackFunc preDecompile,
								 FxCompilationCallbackFunc postDecompile );

	/// Checks to see if \a nodeType is a "relevant" node type during 
	/// %Face Graph collapse.
	/// \param nodeType The node type to check.
	/// \return FxTrue is \a nodeType is relevant or FxFalse if it is not.
	/// \note Internal use only.
	static FxBool FX_CALL 
		IsNodeTypeRelevantForCollapse( FxCompiledFaceGraphNodeType nodeType );

	/// Clears the compiled face graph and reclaims all dynamic memory.
	void Clear( void );

	/// Ticks the compiled face graph.  The final values of all nodes in the
	/// compiled face graph are computed and the registers are updated.
	/// \param time The current application time.
	/// \param registers The array of face graph registers to be used and 
	/// updated.
	/// \param hasBeenTicked A boolean signifying that the compiled face graph
	/// has been ticked at least once.  This is used to properly initialize any
	/// ::NT_Delta nodes that may be in the compiled graph.
	void Tick( const FxReal time, FxArray<FxRegister>& registers, 
		       FxBool& hasBeenTicked );

	/// Compiles the FxFaceGraph into the FxCompiledFaceGraph representation.
	/// \param faceGraph The face graph to compile.
	void Compile( FxFaceGraph& faceGraph );
	/// Decompiles the FxCompiledFaceGraph into the FxFaceGraph representation.
	/// \param faceGraph The face graph to decompile into.
	void Decompile( FxFaceGraph& faceGraph );

	/// Finds the node named \a name in the compiled face graph and returns its
	/// index in the nodes array.
	/// \param name The name of the node to find.
	/// \return The index of the node in the nodes array.
	FxSize FindNodeIndex( const FxName& name ) const;

	/// Counts the number of nodes of the specified type.
	/// \param nodeType The type of node to count.
	/// \return The number of nodes of type \a nodeType in the compiled face 
	/// graph.
	FxSize CountNodesOfType( const FxCompiledFaceGraphNodeType nodeType ) const;

	/// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraph& cg );

private:
	/// The hash of node names.  This maps node names to indices in the nodes
	/// array for very fast node lookup.  This contains 2048 (2^11) hash bins.
	FxStringHash<FxSize, 11> _nodeHash;

	/// Compiles a single FxFaceGraphNode and inserts it and its data into the
	/// compiled face graph.  Note that this function is recursive.
	/// \param pNode The FxFaceGraphNode to compile.
	/// \param traversal The traversal path through the original FxFaceGraph 
	/// from pNode through the 'end' of the graph.  When this function returns,
	/// traversal is the complete path through the FxFaceGraph used to compute
	/// pNode's final value.
	/// \param traversalHash A hash structure that is used to prevent duplicate
	/// nodes from being added to the traversal path.  This greatly improves
	/// the performance of compilation on graphs containing large numbers of 
	/// nodes and links that form complicated networks.  It contains 2048 (2^11) 
	/// hash bins.
	void _compileNode( const FxFaceGraphNode* pNode, 
		               FxArray<const FxFaceGraphNode*>& traversal,
					   FxStringHash<FxSize, 11>& traversalHash );
};

} // namespace Face

} // namespace OC3Ent

#endif
