//------------------------------------------------------------------------------
// The collapse face graph command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCollapseFaceGraphCommand.h"
//@todo This is included to get access to FxMsg() etc.  We might want to remove 
//      this later on if we actually implement the plug-in architecture.
#include "FxConsole.h"
#include "FxActor.h"
#include "FxFaceGraphBaker.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxCollapseFaceGraphCommand, 0, FxCommand);

FxCollapseFaceGraphCommand::FxCollapseFaceGraphCommand()
{
}

FxCollapseFaceGraphCommand::~FxCollapseFaceGraphCommand()
{
}

FxCommandSyntax FxCollapseFaceGraphCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-fn", "-forcenodes", CAT_StringArray));
	return newSyntax;
}

// Forward declaration.
FxBool _FG_Collapse( FxActor& srcActor, FxActor& dstActor, const FxArray<FxString>& forceNodes );

FxCommandError FxCollapseFaceGraphCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString filename;
	if( argList.GetArgument("-file", filename) )
	{
		FxString ext = filename.AfterLast('.');

		FxMsg("Loading %s...", filename.GetData());

		FxActor actor;
		if( ext == "fxa" || ext == "FXA" )
		{
			if( !FxLoadActorFromFile(actor, filename.GetData(), FxTrue) )
			{
				FxError("<b>ERROR:</b> file does not exist!");
				return CE_Failure;
			}
		}
		else
		{
			FxError("<b>ERROR:</b> file does not exist or is not an .fxa file!");
			return CE_Failure;
		}

		// Get the nodes that should be forced into the collapsed graph.
		FxArray<FxString> forceNodes;
		argList.GetArgument("-forcenodes", forceNodes);

		FxMsg("<i>Collapsing Face Graph...<i>");

		FxActor newActor;
		if( !_FG_Collapse(actor, newActor, forceNodes) )
		{
			FxError("<b>ERROR:</b> an error occurred collapsing the face graph!");
			return CE_Failure;
		}

		FxString newFilename = filename.BeforeLast('.');
		newFilename += "-FG_Collapsed.fxa";
		if( !FxSaveActorToFile(newActor, newFilename.GetData()) )
		{
			FxError("<b>ERROR:</b> failed to save actor!");
			return CE_Failure;
		}

		FxMsg("Face Graph collapse complete: <b>%s</b> saved to <b>%s</b>", filename.GetData(), newFilename.GetData());
		return CE_Success;
	}
	FxError("<b>ERROR:</b> invalid arguments (must specify -file)!");
	return CE_Failure;
}

FxBool _FG_Collapse( FxActor& srcActor, FxActor& dstActor, const FxArray<FxString>& forceNodes )
{
	// Make sure the actor names match.
	dstActor.SetName(srcActor.GetName());
	FxMsg("Processing actor <b>%s</b>...", dstActor.GetNameAsCstr());

	// Construct the master bone list in dstActor.
	FxMasterBoneList& srcMBL = srcActor.GetMasterBoneList();
	FxMasterBoneList& dstMBL = dstActor.GetMasterBoneList();

	FxSize numRefBones = srcMBL.GetNumRefBones();
	FxMsg("Processing <b>%d</b> ref. bones...", numRefBones);
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		const FxBone& refBone = srcMBL.GetRefBone(i);
		FxMsg("&nbsp;&nbsp;&nbsp;&nbsp;<b>%s</b>", refBone.GetNameAsCstr());
		dstMBL.AddRefBone(refBone);
	}

	// Construct the phoneme mapping in dstActor.
	FxMsg("Constructing phoneme map...");
	dstActor.SetPhonemeMap(srcActor.GetPhonemeMap());

	// Build a list of all node names relevant to dstActor.
	FxArray<FxName> relevantNodes;
	FxCompiledFaceGraph& srcCG = srcActor.GetCompiledFaceGraph();
	FxSize numSrcNodes = srcCG.nodes.Length();
	// Prevent reallocation overhead.
	relevantNodes.Reserve(numSrcNodes);
	FxMsg("Processing <b>%d</b> src nodes...", numSrcNodes);
	for( FxSize i = 0; i < numSrcNodes; ++i )
	{
		if( FxCompiledFaceGraph::IsNodeTypeRelevantForCollapse(srcCG.nodes[i].nodeType) ||
			FxInvalidIndex != forceNodes.Find(srcCG.nodes[i].name.GetAsString()) )
		{
			relevantNodes.PushBack(srcCG.nodes[i].name);
		}
	}
	FxSize numRelevantNodes = relevantNodes.Length();
	FxMsg("Gathered <b>%d</b> relevant nodes", numRelevantNodes);

	// Make sure that the bones are valid in each bone pose node.
	srcActor.DecompileFaceGraph();
	FxFaceGraph& srcFG = srcActor.GetDecompiledFaceGraph();

	FxMsg("Constructing collapsed face graph...");
	// Put the relevant nodes into dstActor.
	FxFaceGraph& dstFG = dstActor.GetDecompiledFaceGraph();
	for( FxSize i = 0; i < numRelevantNodes; ++i )
	{
		FxFaceGraphNode* pNode = srcFG.FindNode(relevantNodes[i]);
		if( !pNode )
		{
			return FxFalse;
		}
		FxFaceGraphNode* pClone = pNode->Clone();
		if( !dstFG.AddNode(pClone) )
		{
			return FxFalse;
		}
	}
	dstActor.CompileFaceGraph();

	// Add and bake all animations for each relevant node.
	FxFaceGraphBaker baker;
	baker.SetActor(&srcActor);

	FxStringHash<FxSize> processedCurves;
	FxSize numProcessedAnims = 0;
	FxSize numSrcAnimGroups = srcActor.GetNumAnimGroups();
	for( FxSize i = 0; i < numSrcAnimGroups; ++i )
	{
		FxAnimGroup& srcGroup = srcActor.GetAnimGroup(i);
		FxSize numSrcAnims = srcGroup.GetNumAnims();
		FxMsg("Processing group <b>%s</b> containing <b>%d</b> anims...", srcGroup.GetNameAsCstr(), numSrcAnims);
		for( FxSize j = 0; j < numSrcAnims; ++j )
		{
			const FxAnim& srcAnim = srcGroup.GetAnim(j);
			baker.SetAnim(srcGroup.GetName(), srcAnim.GetName());
			if( !baker.Bake(relevantNodes) )
			{
				return FxFalse;
			}
			FxAnim dstAnim;
			dstAnim.SetName(srcAnim.GetName());
			dstAnim.SetBlendInTime(srcAnim.GetBlendInTime());
			dstAnim.SetBlendOutTime(srcAnim.GetBlendOutTime());
			FxSize numSrcBoneWeights = srcAnim.GetNumBoneWeights();
			for( FxSize k = 0; k < numSrcBoneWeights; ++k )
			{
				dstAnim.AddBoneWeight(srcAnim.GetBoneWeight(k));
			}
			FxSize numBakedCurves = baker.GetNumResultAnimCurves();
			if( numBakedCurves > 0 )
			{
				for( FxSize k = 0; k < numBakedCurves; ++k )
				{
					const FxAnimCurve& bakedCurve = baker.GetResultAnimCurve(k);
					if( bakedCurve.GetNumKeys() > 0 )
					{
						processedCurves.Insert(bakedCurve.GetNameAsCstr(), 0);
						dstAnim.AddAnimCurve(baker.GetResultAnimCurve(k));
					}
				}
				dstActor.AddAnim(srcGroup.GetName(), dstAnim);
				numProcessedAnims++;
				FxMsg("&nbsp;&nbsp;&nbsp;&nbsp;Processed anim <b>%s</b> with <b>%d</b> curves", dstAnim.GetNameAsCstr(), dstAnim.GetNumAnimCurves());
			}
		}
	}
	FxMsg("Processed <b>%d</b> animations", numProcessedAnims);

	// Remove any unused nodes (only if they are not in the forceNodes array).
	dstActor.DecompileFaceGraph();
	FxCompiledFaceGraph& dstCG = dstActor.GetCompiledFaceGraph();
	FxSize numUnusedNodes = 0;
	FxSize numNodesInDstCG = dstCG.nodes.Length();
	for( FxSize i = 0; i < numNodesInDstCG; ++i )
	{
		if( !processedCurves.Find(dstCG.nodes[i].name.GetAsCstr()) &&
			FxInvalidIndex == forceNodes.Find(dstCG.nodes[i].name.GetAsString()) )
		{
			if( !dstFG.RemoveNode(dstCG.nodes[i].name) )
			{
				return FxFalse;
			}
			numUnusedNodes++;
		}
	}
	dstActor.CompileFaceGraph();

	FxMsg("Removed <b>%d</b> unused nodes leaving <b>%d</b> total nodes", numUnusedNodes, numRelevantNodes - numUnusedNodes);

	return FxTrue;
}

} // namespace Face

} // namespace OC3Ent
