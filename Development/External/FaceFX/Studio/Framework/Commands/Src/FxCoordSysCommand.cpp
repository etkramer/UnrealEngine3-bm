//------------------------------------------------------------------------------
// The coordsys command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCoordSysCommand.h"
#include "FxSessionProxy.h"
//@todo This is included to get access to FxMsg() etc.  We might want to remove 
//      this later on if we actually implement the plug-in architecture.
#include "FxConsole.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxCoordSysCommand, 0, FxCommand);

FxCoordSysCommand::FxCoordSysCommand()
{
	_isUndoable = FxTrue;
}

FxCoordSysCommand::~FxCoordSysCommand()
{
}

FxCommandSyntax FxCoordSysCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-sp", "-sourcepackage", CAT_String));
	return newSyntax;
}

FxCommandError FxCoordSysCommand::Execute( const FxCommandArgumentList& argList )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		if( argList.GetArgument("-sourcepackage", _sourcePackage) )
		{
			return Redo();
		}
		FxError("<b>ERROR:</b> invalid arguments (must specify -sourcepackage)!");
		return CE_Failure;
	}
	FxError("<b>ERROR:</b> no actor loaded!");
	return CE_Failure;
}

FxCommandError FxCoordSysCommand::Undo( void )
{
	// The inverse of a coordsys operation is simply to redo the command with
	// the same arguments.
	return Redo();
}

FxCommandError FxCoordSysCommand::Redo( void )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	FxAssert(pActor);
	if( _sourcePackage == "maya" || _sourcePackage == "MAYA" )
	{
		FxMsg("Converting from Maya coordinate system...");
		_convertMayaCoordSys(pActor);
	}
	else if( _sourcePackage == "max" || _sourcePackage == "MAX" )
	{
		FxMsg("Converting from Max coordinate system...");
		_convertMaxCoordSys(pActor);
	}
	else if( _sourcePackage == "xsi" || _sourcePackage == "XSI" )
	{
		FxMsg("Converting from XSI coordinate system...");
		_convertXsiCoordSys(pActor);
	}
	else
	{
		FxError("<b>ERROR:</b> invalid source package (must be maya or max)!");
		return CE_Failure;
	}
	FxSessionProxy::RefreshControls();
	FxMsg("Conversion complete.");
	return CE_Success;
}

void FxCoordSysCommand::_convertMayaCoordSys( FxActor* pActor )
{
	FxAssert(pActor);
	FxArray<FxBone> refBones;
	FxSize numRefBones = pActor->GetMasterBoneList().GetNumRefBones();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		FxBone refBone = pActor->GetMasterBoneList().GetRefBone(i);
		FxVec3 refPos  = refBone.GetPos();
		FxQuat refRot  = refBone.GetRot();
		refPos.x       = -refPos.x;
		refRot.x       = -refRot.x;
		refBone.SetPos(refPos);
		refBone.SetRot(refRot);
		refBones.PushBack(refBone);
	}

	// Clear the master bone list.
	pActor->GetMasterBoneList().Clear();

	// Recreate the master bone list.
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		pActor->GetMasterBoneList().AddRefBone(refBones[i]);
	}

	// Update the bone poses.
	FxFaceGraph::Iterator bonePoseIter    = pActor->GetDecompiledFaceGraph().Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator bonePoseIterEnd = pActor->GetDecompiledFaceGraph().End(FxBonePoseNode::StaticClass());
	for( ; bonePoseIter != bonePoseIterEnd; ++bonePoseIter )
	{
		FxBonePoseNode* pNode = FxCast<FxBonePoseNode>(bonePoseIter.GetNode());
		if( pNode )
		{
			// Cache the bones in the node.
			FxArray<FxBone> bonesInNode;
			FxSize numBonesInNode = pNode->GetNumBones();
			for( FxSize i = 0; i < numBonesInNode; ++i )
			{
				bonesInNode.PushBack(pNode->GetBone(i));
			}

			// Clear them.
			pNode->RemoveAllBones();

			// Transform the coordinate system and re-add them.
			for( FxSize i = 0; i < numBonesInNode; ++i )
			{
				FxVec3 pos = bonesInNode[i].GetPos();
				FxQuat rot = bonesInNode[i].GetRot();
				pos.x      = -pos.x;
				rot.x      = -rot.x;
				bonesInNode[i].SetPos(pos);
				bonesInNode[i].SetRot(rot);

				pNode->AddBone(bonesInNode[i]);
			}
		}
	}

	// Pull the bones from the face graph into the master bone list.
	pActor->GetMasterBoneList().PullBones(pActor->GetDecompiledFaceGraph(), pActor->GetCompiledFaceGraph());
	pActor->SetShouldClientRelink(FxTrue);
}

void FxCoordSysCommand::_convertMaxCoordSys( FxActor* pActor )
{
	FxAssert(pActor);
	FxArray<FxBone> refBones;
	FxSize numRefBones = pActor->GetMasterBoneList().GetNumRefBones();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		FxBone refBone = pActor->GetMasterBoneList().GetRefBone(i);
		FxVec3 refPos  = refBone.GetPos();
		FxQuat refRot  = refBone.GetRot();
		FxReal temp    = refPos.y;
		refPos.y       = refPos.z;
		refPos.z       = temp;
		temp           = refRot.y;
		refRot.y       = refRot.z;
		refRot.z       = temp;
		refBone.SetPos(refPos);
		refBone.SetRot(refRot);
		refBones.PushBack(refBone);
	}

	// Clear the master bone list.
	pActor->GetMasterBoneList().Clear();

	// Recreate the master bone list.
    for( FxSize i = 0; i < numRefBones; ++i )
	{
		pActor->GetMasterBoneList().AddRefBone(refBones[i]);
	}

	// Update the bone poses.
	FxFaceGraph::Iterator bonePoseIter    = pActor->GetDecompiledFaceGraph().Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator bonePoseIterEnd = pActor->GetDecompiledFaceGraph().End(FxBonePoseNode::StaticClass());
	for( ; bonePoseIter != bonePoseIterEnd; ++bonePoseIter )
	{
		FxBonePoseNode* pNode = FxCast<FxBonePoseNode>(bonePoseIter.GetNode());
		if( pNode )
		{
			// Cache the bones in the node.
			FxArray<FxBone> bonesInNode;
			FxSize numBonesInNode = pNode->GetNumBones();
			for( FxSize i = 0; i < numBonesInNode; ++i )
			{
				bonesInNode.PushBack(pNode->GetBone(i));
			}

			// Clear them.
			pNode->RemoveAllBones();

			// Transform the coordinate system and re-add them.
			for( FxSize i = 0; i < numBonesInNode; ++i )
			{
				FxVec3 pos  = bonesInNode[i].GetPos();
				FxQuat rot  = bonesInNode[i].GetRot();
				FxReal temp = pos.y;
				pos.y       = pos.z;
				pos.z       = temp;
				temp        = rot.y;
				rot.y       = rot.z;
				rot.z       = temp;
				bonesInNode[i].SetPos(pos);
				bonesInNode[i].SetRot(rot);
				pNode->AddBone(bonesInNode[i]);
			}
		}
	}

	// Pull the bones from the face graph into the master bone list.
	pActor->GetMasterBoneList().PullBones(pActor->GetDecompiledFaceGraph(), pActor->GetCompiledFaceGraph());
	pActor->SetShouldClientRelink(FxTrue);
}

void FxCoordSysCommand::_convertXsiCoordSys( FxActor* pActor )
{
	_convertMayaCoordSys(pActor);
}

} // namespace Face

} // namespace OC3Ent
