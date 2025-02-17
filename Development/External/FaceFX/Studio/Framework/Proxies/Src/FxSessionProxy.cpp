//------------------------------------------------------------------------------
// The session proxy gives internal and external commands a clean and 
// consistent interface to the current FaceFX Studio session.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxSessionProxy.h"
#include "FxStudioSession.h"
#include "FxVM.h"
#include "FxFaceGraphNodeGroupManager.h"
#include "FxFaceGraphNodeGroup.h"
#include "FxPhonWordList.h"
#include "FxFaceGraphNodeUserData.h"
#include "FxPhonemeInfo.h"
#include "FxAnimController.h"
#include "FxCurveUserData.h"
#include "FxAnimSetManager.h"

namespace OC3Ent
{

namespace Face
{

static FxStudioSession* _pSession = NULL;
static FxPhonWordList*  _pPhonWordList = NULL;
static FxBool _noActorChangedMessages = FxFalse;
static FxBool _noRefreshMessages = FxFalse;

//------------------------------------------------------------------------------
// Application functions.
//------------------------------------------------------------------------------
void FxSessionProxy::ExitApplication( void )
{
	if( _pSession )
	{
		_pSession->Exit();
	}
}

void FxSessionProxy::ToggleWidgetMessageDebugging( void )
{
	if( _pSession )
	{
		_pSession->_debugWidgetMessages = !_pSession->_debugWidgetMessages;
	}
}

void FxSessionProxy::QueryLanguageInfo( FxArray<FxLanguageInfo>& languages )
{
	if( _pSession )
	{
		_pSession->OnQueryLanguageInfo(NULL, languages);
	}
	else
	{
		languages.Clear();
	}
}

void FxSessionProxy::QueryRenderWidgetCaps( FxRenderWidgetCaps& renderWidgetCaps )
{
	if( _pSession )
	{
		_pSession->OnQueryRenderWidgetCaps(NULL, renderWidgetCaps);
	}
}

//------------------------------------------------------------------------------
// Actor functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::CreateActor( void )
{
	if( _pSession )
	{
		if( _pSession->CreateActor() )
		{
			FxActor* pActor = _pSession->GetActor();
			if( pActor )
			{
				pActor->SetName("NewActor");
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::LoadActor( const FxString& filename )
{
	if( _pSession )
	{
		return _pSession->LoadActor(filename);
	}
	return FxFalse;
}

FxBool FxSessionProxy::CloseActor( void )
{
	if( _pSession )
	{
		return _pSession->CloseActor();
	}
	return FxFalse;
}

FxBool FxSessionProxy::SaveActor( const FxString& filename )
{
	if( _pSession )
	{
		return _pSession->SaveActor(filename);
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetActorName( FxString& actorName )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			actorName = pActor->GetNameAsString();
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetActor( FxActor** ppActor )
{
	if( _pSession )
	{
		*ppActor = _pSession->GetActor();
		if( *ppActor )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetCompiledFaceGraph( FxCompiledFaceGraph** ppCG )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			*ppCG = &pActor->GetCompiledFaceGraph();
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetActorInstance( FxActorInstance** ppActorInstance )
{
	if( _pSession )
	{
		*ppActorInstance = _pSession->GetActorInstance();
		if( *ppActorInstance )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetRegister( const FxName& regName, FxReal value, FxRegisterOp regOp )
{
	if( _pSession )
	{
		FxActorInstance* pActorInstance = _pSession->GetActorInstance();
		if( pActorInstance )
		{
			return pActorInstance->SetRegister(regName, value, regOp);
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetRegister( const FxName& regName, FxReal& regValue )
{
	if( _pSession )
	{
		FxActorInstance* pActorInstance = _pSession->GetActorInstance();
		if( pActorInstance )
		{
			regValue = pActorInstance->GetRegister(regName);
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetOriginalRegisterValue( const FxName& regName, FxReal& regValue )
{
	if( _pSession )
	{
		FxActorInstance* pActorInstance = _pSession->GetActorInstance();
		if( pActorInstance )
		{
			regValue = pActorInstance->GetOriginalRegisterValue(regName);
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::DisableAllRegisters( void )
{
	if( _pSession )
	{
		FxActorInstance* pActorInstance = _pSession->GetActorInstance();
		if( pActorInstance )
		{
			pActorInstance->SetAllRegisters(RO_None, 0.0f);
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::UpdateActorInstance( FxBool useDummyRegisters )
{
	if( _pSession )
	{
		if( useDummyRegisters )
		{
			FxActor* pActor = _pSession->GetActor();
			if( pActor )
			{
				FxCompiledFaceGraph& cg = pActor->GetCompiledFaceGraph();
				//@todo Look into this in more detail.  hasBeenTicked 'should' be FxFalse for
				// the first Tick and then FxTrue for each subsequent Tick, but due to the way
				// the curve widget draws itself it ends up being FxFalse the entire time
				// if it is initialized to FxFalse.  I think we will just have to live with
				// this, because it works to leave it as FxTrue because of the way things are
				// initialized.  It's much easier to leave this slightly ambiguous with an
				// explanation rather than restructuring how FaceFX Studio evaluates the
				// face graph to draw curves.
				FxBool hasBeenTicked = FxTrue;
				cg.Tick(0.0f, _pSession->_dummyRegisters, hasBeenTicked);
				return FxTrue;
			}
		}
		else
		{
			FxActorInstance* pActorInstance = _pSession->GetActorInstance();
			if( pActorInstance )
			{
				pActorInstance->Tick(0.0f, -1.0f);
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

#ifdef __UNREAL__
FxBool FxSessionProxy::GetFaceFXAsset( UFaceFXAsset** ppFaceFXAsset )
{
	if( _pSession )
	{
		*ppFaceFXAsset = _pSession->GetFaceFXAsset();
		if( *ppFaceFXAsset )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}
#endif // __UNREAL__

FxBool FxSessionProxy::SetNoActorChangedMessages( FxBool noActorChangedMessages )
{
	if( _pSession )
	{
		_noActorChangedMessages = noActorChangedMessages;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::ActorDataChanged( FxActorDataChangedFlag flags )
{
	if( _pSession )
	{
		if( !_noActorChangedMessages )
		{
			_pSession->OnActorRenamed(NULL);
			if( flags & ADCF_FaceGraphChanged )
			{
				CompileFaceGraph();
			}
			_pSession->OnActorInternalDataChanged(NULL, flags);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::IsForcedDirty( FxBool& isForcedDirty )
{
	if( _pSession )
	{
		isForcedDirty = _pSession->IsForcedDirty();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetIsForcedDirty( FxBool isForcedDirty )
{
	if( _pSession )
	{
		_pSession->SetIsForcedDirty(isForcedDirty);
		return FxTrue;
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Time functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::GetSessionTime( FxReal& sessionTime )
{
	if( _pSession )
	{
		sessionTime = _pSession->_currentTime;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSessionTime( FxReal newTime )
{
	if( _pSession )
	{
		_pSession->OnTimeChanged(NULL, newTime);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSessionFrame( FxInt32 newFrame )
{
	FxReal frameRate = 1.0f / FxStudioOptions::GetFrameRate();
	return SetSessionTime(newFrame * frameRate);
}

//------------------------------------------------------------------------------
// Selection functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::GetSelectedAnimGroup( FxString& animGroup )
{
	if( _pSession )
	{
		animGroup = _pSession->_selAnimGroupName.GetAsString();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetSelectedAnim( FxString& anim )
{
	if( _pSession )
	{
		anim = _pSession->_selAnimName.GetAsString();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetSelectedAnimCurves( FxArray<FxString>& animCurves )
{
	if( _pSession )
	{
		animCurves.Clear();
		for( FxSize i = 0; i < _pSession->_selAnimCurveNames.Length(); ++i )
		{
			animCurves.PushBack(_pSession->_selAnimCurveNames[i].GetAsString());
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::IsAnimCurveSelected( const FxString& curve )
{
	if( _pSession )
	{
		FxArray<FxString> selectedCurves;
		if( GetSelectedAnimCurves(selectedCurves) )
		{
			for( FxSize i = 0; i < selectedCurves.Length(); ++i )
			{
				if( selectedCurves[i] == curve )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetSelectedFaceGraphNodeGroup( FxString& nodeGroup )
{
	if( _pSession )
	{
		nodeGroup = _pSession->_selNodeGroup.GetAsString();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetSelectedFaceGraphNodes( FxArray<FxString>& nodes )
{
	if( _pSession )
	{
		nodes.Clear();
		for( FxSize i = 0; i < _pSession->_selFaceGraphNodeNames.Length(); ++i )
		{
			nodes.PushBack(_pSession->_selFaceGraphNodeNames[i].GetAsString());
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::IsFaceGraphNodeSelected( const FxString& node )
{
	if( _pSession )
	{
		FxArray<FxString> selectedNodes;
		if( GetSelectedFaceGraphNodes(selectedNodes) )
		{
			for( FxSize i = 0; i < selectedNodes.Length(); ++i )
			{
				if( selectedNodes[i] == node )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetSelectedFaceGraphNodeLink( FxString& node1, FxString& node2 )
{
	if( _pSession )
	{
		node1 = _pSession->_inputNode.GetAsString();
		node2 = _pSession->_outputNode.GetAsString();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSelectedAnimGroup( const FxString& animGroup )
{
	if( _pSession )
	{
		_pSession->OnAnimChanged(NULL, animGroup.GetData(), NULL);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSelectedAnim( const FxString& anim )
{
	if( _pSession )
	{
		FxAnim* pAnim = NULL;
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize groupIndex = pActor->FindAnimGroup(_pSession->_selAnimGroupName);
			if( FxInvalidIndex != groupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(groupIndex);
				FxSize animIndex = group.FindAnim(anim.GetData());
				if( FxInvalidIndex != animIndex )
				{
					pAnim = group.GetAnimPtr(animIndex);
				}
			}
		}
		_pSession->OnAnimChanged(NULL, _pSession->_selAnimGroupName, pAnim);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSelectedAnimCurves( const FxArray<FxString>& animCurves )
{
	if( _pSession )
	{
		FxArray<FxName> animCurveNames;
		for( FxSize i = 0; i < animCurves.Length(); ++i )
		{
			animCurveNames.PushBack(animCurves[i].GetData());
		}
		_pSession->OnAnimCurveSelChanged(NULL, animCurveNames);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddAnimCurveToSelection( const FxString& curve )
{
	if( _pSession )
	{
		if( !IsAnimCurveSelected(curve) )
		{
			FxArray<FxString> selectedCurves;
			if( GetSelectedAnimCurves(selectedCurves) )
			{
				selectedCurves.PushBack(curve);
				if( SetSelectedAnimCurves(selectedCurves) )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveAnimCurveFromSelection( const FxString& curve )
{
	if( _pSession )
	{
		if( IsAnimCurveSelected(curve) )
		{
			FxArray<FxString> selectedCurves;
			if( GetSelectedAnimCurves(selectedCurves) )
			{
				FxArray<FxString> newSelection;
				for( FxSize i = 0; i < selectedCurves.Length(); ++i )
				{
					if( selectedCurves[i] != curve )
					{
						newSelection.PushBack(selectedCurves[i]);
					}
				}
				if( SetSelectedAnimCurves(newSelection) )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSelectedFaceGraphNodeGroup( const FxString& nodeGroup )
{
	if( _pSession )
	{
		_pSession->OnFaceGraphNodeGroupSelChanged(NULL, nodeGroup.GetData());
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSelectedFaceGraphNodes( const FxArray<FxString>& nodes, FxBool zoomToSelection )
{
	if( _pSession )
	{
		FxArray<FxName> nodeNames;
		for( FxSize i = 0; i < nodes.Length(); ++i )
		{
			nodeNames.PushBack(nodes[i].GetData());
		}
		_pSession->OnFaceGraphNodeSelChanged(NULL, nodeNames, zoomToSelection);

		if( nodeNames.Length() == 0 )
		{
			_pSession->OnLinkSelChanged(NULL, FxName::NullName, FxName::NullName);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddFaceGraphNodeToSelection( const FxString& node )
{
	if( _pSession )
	{
		if( !IsFaceGraphNodeSelected(node) )
		{
			FxArray<FxString> selectedNodes;
			if( GetSelectedFaceGraphNodes(selectedNodes) )
			{
				selectedNodes.PushBack(node);
				if( SetSelectedFaceGraphNodes(selectedNodes, FxFalse) )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveFaceGraphNodeFromSelection( const FxString& node )
{
	if( _pSession )
	{
		if( IsFaceGraphNodeSelected(node) )
		{
			FxArray<FxString> selectedNodes;
			if( GetSelectedFaceGraphNodes(selectedNodes) )
			{
				FxArray<FxString> newSelection;
				for( FxSize i = 0; i < selectedNodes.Length(); ++i )
				{
					if( selectedNodes[i] != node )
					{
						newSelection.PushBack(selectedNodes[i]);
					}
				}
				if( SetSelectedFaceGraphNodes(newSelection, FxFalse) )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetSelectedFaceGraphNodeLink( const FxString& node1, const FxString& node2 )
{
	if( _pSession )
	{
		_pSession->OnLinkSelChanged(NULL, node1.GetData(), node2.GetData());
		return FxTrue;
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Face graph functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::LayoutFaceGraph( void )
{
	if( _pSession )
	{
		_pSession->OnLayoutFaceGraph(NULL);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetFaceGraphNodePos( const FxString& node, FxInt32& x, FxInt32& y )
{
	x = 0;
	y = 0;
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxFaceGraphNode* pNode = NULL;
			pNode = pActor->GetDecompiledFaceGraph().FindNode(node.GetData());
			if( pNode )
			{
				if( pNode->GetUserData() )
				{
					x = reinterpret_cast<FxNodeUserData*>(pNode->GetUserData())->bounds.GetPosition().x;
					y = reinterpret_cast<FxNodeUserData*>(pNode->GetUserData())->bounds.GetPosition().y;
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetFaceGraphNodePos( const FxString& node, FxInt32 x, FxInt32 y )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxFaceGraphNode* pNode = NULL;
			pNode = pActor->GetDecompiledFaceGraph().FindNode(node.GetData());
			if( pNode )
			{
				if( pNode->GetUserData() )
				{
					reinterpret_cast<FxNodeUserData*>(pNode->GetUserData())->bounds.SetPosition(FxPoint(x, y));
					RefreshControls();
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddFaceGraphNode( FxFaceGraphNode* pNode )
{
	if( _pSession && pNode )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			if( pActor->GetDecompiledFaceGraph().FindNode(pNode->GetName()) )
			{
				return FxFalse;
			}
			else
			{
				FxNodeUserData* pUserData = new FxNodeUserData();
				pNode->SetUserData(pUserData);
				pActor->GetDecompiledFaceGraph().AddNode(pNode);
				_pSession->OnAddFaceGraphNode(NULL, pNode);
                FaceGraphChanged();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveFaceGraphNode( FxFaceGraphNode* pNode )
{
	if( _pSession && pNode )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			if( !pActor->GetDecompiledFaceGraph().FindNode(pNode->GetName()) )
			{
				return FxFalse;
			}
			else
			{
				_pSession->OnRemoveFaceGraphNode(NULL, pNode);
				pActor->GetDecompiledFaceGraph().RemoveNode(pNode->GetName());
                FaceGraphChanged();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetFaceGraphNode( const FxString& node, FxFaceGraphNode** ppNode )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			*ppNode = pActor->GetDecompiledFaceGraph().FindNode(node.GetData());
			if( *ppNode )
			{
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool 
FxSessionProxy::AddFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
								      FxFaceGraphNode* pToNode, 
									  const FxLinkFn* pLinkFn, 
									  const FxLinkFnParameters& linkFnParams )
{
	if( _pSession && pFromNode && pToNode && pLinkFn )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxLinkErrorCode linkError = 
				pActor->GetDecompiledFaceGraph().Link(pToNode->GetName(), pFromNode->GetName(), 
				pLinkFn->GetName(), linkFnParams);
			switch( linkError )
			{
			case LEC_Invalid:
				FxVM::DisplayError("invalid link!");
				return FxFalse;
				break;
			case LEC_Duplicate:
				FxVM::DisplayError("duplicate link!");
				return FxFalse;
				break;
			case LEC_Cycle:
				FxVM::DisplayError("link would create cycle!");
				return FxFalse;
				break;
			case LEC_Refused:
				FxVM::DisplayError("link refused!");
				return FxFalse;
				break;
			case LEC_None:
				return FxTrue;
				break;
			default:
				FxVM::DisplayError("unknown link error!");
				return FxFalse;
				break;
			}
		}
	}
	return FxFalse;
}

FxBool 
FxSessionProxy::GetFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
									  FxFaceGraphNode* pToNode, FxString& linkFn, 
								      FxLinkFnParameters& linkFnParams )
{
	if( _pSession && pFromNode && pToNode )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxFaceGraphNodeLink linkInfo;
			if( pActor->GetDecompiledFaceGraph().GetLink(pFromNode->GetName(), pToNode->GetName(), linkInfo) )
			{
				linkFn = linkInfo.GetLinkFnName().GetAsString();
				linkFnParams = linkInfo.GetLinkFnParams();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool 
FxSessionProxy::RemoveFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
										 FxFaceGraphNode* pToNode )
{
	if( _pSession && pFromNode && pToNode )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			return pActor->GetDecompiledFaceGraph().Unlink(pFromNode->GetName(), pToNode->GetName());
		}
	}
	return FxFalse;
}

FxBool 
FxSessionProxy::EditFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
									   FxFaceGraphNode* pToNode, 
									   const FxLinkFn* pLinkFn, 
									   const FxLinkFnParameters& linkFnParams )
{
	if( _pSession && pFromNode && pToNode && pLinkFn )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxFaceGraphNodeLink linkInfo;
			linkInfo.SetLinkFn(pLinkFn);
			linkInfo.SetLinkFnParams(linkFnParams);
			linkInfo.SetNode(pFromNode);
			return pActor->GetDecompiledFaceGraph().SetLink(pFromNode->GetName(), pToNode->GetName(), linkInfo);
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetFaceGraphNodeName( FxName oldName, FxName newName )
{
	if( _pSession )
	{
		// Get a pointer to the node.
		FxFaceGraphNode* pNode = NULL;
		if( FxSessionProxy::GetFaceGraphNode(oldName.GetAsString(), &pNode) )
		{
			FxActor* pActor = _pSession->GetActor();
			FxBool success = FxFalse;
			if( pActor )
			{
				success = pActor->GetDecompiledFaceGraph().RenameNode(oldName, newName);
			}

			if( success )
			{
				FxArray<FxString> selNodes;
				selNodes.PushBack(newName.GetAsString());
				SetSelectedFaceGraphNodes(selNodes, FxFalse);
				
				FaceGraphChanged();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::FaceGraphChanged( void )
{
	if( _pSession )
	{
		if( !_noActorChangedMessages )
		{
			_pSession->OnActorInternalDataChanged(NULL, ADCF_FaceGraphChanged);
			CompileFaceGraph();
			// Reset the current time, to update the preview window, face graph
			// value viz mode, and anything else that needs it.
			_pSession->OnTimeChanged(NULL, _pSession->_currentTime);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::CompileFaceGraph( void )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
#ifdef __UNREAL__
			// Prevent a potential attachment issue.  NULL out the actor
			// instance in the session because it could be destroyed by
			// being detached - but in the UE3 render widget update the 
			// actor instance will be reset after waiting on the rendering
			// thread to finish processing the potential re-attachment.
			_pSession->SetActorInstance(NULL);
#endif // __UNREAL__
			pActor->CompileFaceGraph();
			SetNumDummyRegisters(pActor->GetCompiledFaceGraph().nodes.Length());
			// Reset the current time, to update the preview window, face graph
			// value viz mode, and anything else that needs it.
			_pSession->OnActorInternalDataChanged(NULL, ADCF_FaceGraphChanged);
			_pSession->OnTimeChanged(NULL, _pSession->_currentTime);
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetFaceGraph( FxFaceGraph*& pFaceGraph )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			pFaceGraph = &pActor->GetDecompiledFaceGraph();
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetFaceGraph( const FxFaceGraph& faceGraph )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			// Cache the app's state.
			FxArray<FxString> selectedFGNodes;
			FxSessionProxy::GetSelectedFaceGraphNodes(selectedFGNodes);
			FxString selectedAnimGroup;
			FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
			FxString selectedAnim;
			FxSessionProxy::GetSelectedAnim(selectedAnim);
			FxArray<FxString> selectedCurves;
			FxSessionProxy::GetSelectedAnimCurves(selectedCurves);
			FxReal currentTime;
			FxSessionProxy::GetSessionTime(currentTime);
			FxPhonemeMap phonemeMap;
			FxSessionProxy::GetPhonemeMap(phonemeMap);

			// Clear the app's state.
			_pSession->_deselectAll();

			// Set the Face Graph.
			pActor->GetDecompiledFaceGraph() = faceGraph;
			// Compile.
			pActor->CompileFaceGraph();
			
			// Reset the app's state.
			_pSession->OnActorChanged(NULL, pActor);
			FxSessionProxy::SetSelectedFaceGraphNodes(selectedFGNodes, FxFalse);
			FxSessionProxy::SetSelectedAnimGroup(selectedAnimGroup);
			FxSessionProxy::SetSelectedAnim(selectedAnim);
			FxSessionProxy::SetSelectedAnimCurves(selectedCurves);
			FxSessionProxy::SetSessionTime(currentTime);
			FxSessionProxy::SetPhonemeMap(phonemeMap);
			_pSession->OnActorInternalDataChanged(NULL, ADCF_FaceGraphChanged);
			return FxTrue;
		}
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Node group functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::CreateNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes )
{
	if( !FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData()) )
	{
		FxFaceGraphNodeGroupManager::AddGroup(nodeGroup.GetData());
		FxFaceGraphNodeGroup* group = FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData());
		if( group )
		{
			for( FxSize i = 0; i < nodes.Length(); ++i )
			{
				group->AddNode(nodes[i].GetData());
			}
		}
		FxSessionProxy::ActorDataChanged(ADCF_NodeGroupsChanged);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveNodeGroup( const FxString& nodeGroup )
{
	if( FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData()) )
	{
		FxFaceGraphNodeGroupManager::RemoveGroup(nodeGroup.GetData());
		FxSessionProxy::ActorDataChanged(ADCF_NodeGroupsChanged);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetNodeGroup( const FxString& nodeGroup, FxArray<FxString>& nodes )
{
	FxFaceGraphNodeGroup* group = FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData());
	if( group )
	{
		for( FxSize i = 0; i < group->GetNumNodes(); ++i )
		{
			nodes.PushBack(group->GetNode(i).GetAsString());
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes )
{
	// Reset the node group to no members.
	if( FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData()) )
	{
		FxFaceGraphNodeGroupManager::RemoveGroup(nodeGroup.GetData());
		FxFaceGraphNodeGroupManager::AddGroup(nodeGroup.GetData());
	}

	FxFaceGraphNodeGroup* group = FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData());
	if( group )
	{
		for( FxSize i = 0; i < nodes.Length(); ++i )
		{
			group->AddNode(nodes[i].GetData());
		}
		FxSessionProxy::ActorDataChanged(ADCF_NodeGroupsChanged);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddToNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes )
{
	FxFaceGraphNodeGroup* group = FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData());
	if( group )
	{
		for( FxSize i = 0; i < nodes.Length(); ++i )
		{
			group->AddNode(nodes[i].GetData());
		}
		FxSessionProxy::ActorDataChanged(ADCF_NodeGroupsChanged);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveFromNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes )
{
	FxFaceGraphNodeGroup* group = FxFaceGraphNodeGroupManager::GetGroup(nodeGroup.GetData());
	if( group )
	{
		for( FxSize i = 0; i < nodes.Length(); ++i )
		{
			group->RemoveNode(nodes[i].GetData());
		}
		FxSessionProxy::ActorDataChanged(ADCF_NodeGroupsChanged);
		return FxTrue;
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Phoneme list functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::GetPhonWordList( FxPhonWordList* phonWordList )
{
	if( _pPhonWordList )
	{
		*phonWordList = *_pPhonWordList;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetPhonWordList( FxPhonWordList* newPhonWordList )
{
	if( _pPhonWordList && newPhonWordList )
	{
		*_pPhonWordList = *newPhonWordList;

		if( _pSession )
		{
			FxAnim* pAnim = NULL;
			FxActor* pActor = _pSession->GetActor();
			if( pActor )
			{
				FxSize groupIndex = pActor->FindAnimGroup(_pSession->_selAnimGroupName);
				if( FxInvalidIndex != groupIndex )
				{
					FxAnimGroup& group = pActor->GetAnimGroup(groupIndex);
					FxSize animIndex = group.FindAnim(_pSession->_selAnimName);
					if( FxInvalidIndex != animIndex )
					{
						pAnim = group.GetAnimPtr(animIndex);
					}
				}
			}
			_pSession->OnAnimPhonemeListChanged(NULL, pAnim, _pPhonWordList);
			return FxTrue;
		}
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Mapping functions.
//------------------------------------------------------------------------------
static FxBool batchMapping = FxFalse;

FxBool StringToPhoneme( const FxString& phoneString, FxPhoneme& phoneme )
{
	FxPhoneticAlphabet alphabet = _pSession->GetPhoneticAlphabet();
	for( FxSize i = 0; i < NUM_PHONS; ++i )
	{
		wxString wxPhoneString(phoneString.GetData(), wxConvLibc);
		FxPhoneme currentPhoneme = static_cast<FxPhoneme>(i);
		const FxPhonExtendedInfo& phonInfo = FxGetPhonemeInfo(currentPhoneme);
		switch(alphabet)
		{
		default:
		case PA_DEFAULT:
			if( phonInfo.talkback == wxPhoneString )
			{
				phoneme = currentPhoneme;
				return FxTrue;
			}
			break;
		case PA_IPA:
		case PA_SAMPA:
			if( phonInfo.sampa == wxPhoneString )
			{
				phoneme = currentPhoneme;
				return FxTrue;
			}
			break;
		};
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddTrackToMapping( const FxString& track )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		_pSession->_pPhonemeMap->AddTarget(track.GetData());
		if( !batchMapping )
		{
			_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveTrackFromMapping( const FxString& track )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		_pSession->_pPhonemeMap->RemoveTarget(track.GetData());
		if( !batchMapping )
		{
			_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::IsTrackInMapping( const FxString& track )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		for( FxSize i = 0; i < _pSession->_pPhonemeMap->GetNumTargets(); ++i )
		{
			if( track == _pSession->_pPhonemeMap->GetTargetName(i).GetAsString() )
			{
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetTrackNamesInMapping( FxArray<FxName>& trackNames )
{
	trackNames.Clear();
	if( _pSession && _pSession->_pPhonemeMap )
	{
		for( FxSize i = 0; i < _pSession->_pPhonemeMap->GetNumTargets(); ++i )
		{
			trackNames.PushBack(_pSession->_pPhonemeMap->GetTargetName(i));
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetMappingEntry( const FxString& phoneme, const FxString& track, FxReal& value )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		FxPhoneme phone;
		if( StringToPhoneme(phoneme, phone) )
		{
			value = _pSession->_pPhonemeMap->GetMappingAmount(phone, track.GetData());
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetMappingEntry( const FxString& phoneme, const FxString& track, const FxReal& value )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		FxPhoneme phone;
		if( StringToPhoneme(phoneme, phone) )
		{
			_pSession->_pPhonemeMap->SetMappingAmount(phone, track.GetData(), value);
			if( !batchMapping )
			{	
				_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
			}
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::ClearMapping( void )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		_pSession->_pPhonemeMap->Clear();
		if( !batchMapping )
		{
			_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::ClearPhonemeMapping( const FxString& phoneme )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		FxPhoneme phone;
		if( StringToPhoneme(phoneme, phone) )
		{
			_pSession->_pPhonemeMap->RemovePhonemeMappingEntries(phone);
			if( !batchMapping )
			{
				_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
			}
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::ClearTrackMapping( const FxString& track )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		_pSession->_pPhonemeMap->RemoveTarget(track.GetData());
		if( !batchMapping )
		{
			_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetPhonemeMap( FxPhonemeMap& phonemeMap )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		phonemeMap = *_pSession->_pPhonemeMap;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetPhonemeMap( const FxPhonemeMap& phonemeMap )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		*_pSession->_pPhonemeMap = phonemeMap;
		_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
		FxSessionProxy::SetIsForcedDirty(FxTrue);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetPhonemeMapping( const FxString& phoneme, FxArray<FxString>& tracks, FxArray<FxReal>& values )
{
	if( _pSession && _pSession->_pPhonemeMap )
	{
		FxPhoneme phone;
		if( StringToPhoneme(phoneme, phone) )
		{
			tracks.Clear();
			values.Clear();
			FxPhonemeMap* pMap = _pSession->_pPhonemeMap;
			for( FxSize i = 0; i < pMap->GetNumTargets(); ++i )
			{
				tracks.PushBack(pMap->GetTargetName(i).GetAsCstr());
				values.PushBack(pMap->GetMappingAmount(phone, i));
			}
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetTrackMapping( const FxString& track, FxArray<FxString>& phonemes, FxArray<FxReal>& values )
{
	if( _pSession && _pSession->_pPhonemeMap && _pSession->_pPhonemeMap->GetTargetIndex(track.GetData()) != FxInvalidIndex )
	{
		phonemes.Clear();
		values.Clear();
		FxPhoneticAlphabet alphabet = _pSession->GetPhoneticAlphabet();
		for( FxSize i = 0; i < NUM_PHONS; ++i )
		{
			if( alphabet == PA_DEFAULT )
			{
				phonemes.PushBack(FxString(FxGetPhonemeInfo(static_cast<FxPhoneme>(i)).talkback.mb_str(wxConvLibc)));
			}
			else
			{
				phonemes.PushBack(FxString(FxGetPhonemeInfo(static_cast<FxPhoneme>(i)).sampa.mb_str(wxConvLibc)));
			}
			values.PushBack(_pSession->_pPhonemeMap->GetMappingAmount(static_cast<FxPhoneme>(i), track.GetData()));
		}
		return FxTrue;
	}
	return FxFalse;
}

void FxSessionProxy::BeginMappingBatch( void )
{
	batchMapping = FxTrue;
}

void FxSessionProxy::EndMappingBatch( void )
{
	batchMapping = FxFalse;
	if( _pSession )
	{
		_pSession->OnPhonemeMappingChanged(NULL, _pSession->_pPhonemeMap);
	}
}

void FxSessionProxy::EndMappingBatchNoRefresh( void )
{
	batchMapping = FxFalse;
}

//------------------------------------------------------------------------------
// Animation group functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::CreateAnimGroup( const FxString& animGroup )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex == animGroupIndex )
			{
				// Adding a new animation group can potentially cause a reallocation, so
				// give the widgets a NULL animation in the default group to start with.
				FxSessionProxy::SetSelectedAnim("");
				FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());
				// Add the new group.
				pActor->AddAnimGroup(animGroup.GetData());
				FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveAnimGroup( const FxString& animGroup )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(animGroupIndex);
				// Only empty groups and non-mounted groups can be removed.
				if( 0 == group.GetNumAnims() && 
					FxInvalidIndex == FxAnimSetManager::FindMountedAnimSet(group.GetName()) )
				{
					_pSession->OnDeleteAnimGroup(NULL, animGroup.GetData());
					FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetAnimGroup( const FxString& animGroup, FxAnimGroup** ppAnimGroup )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(animGroupIndex);
				*ppAnimGroup = &group;
				if( *ppAnimGroup )
				{
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Animation set functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::MountAnimSet( const FxString& animSetPath )
{
	if( _pSession )
	{
		return _pSession->MountAnimSet(animSetPath);
	}
	return FxFalse;
}

FxBool FxSessionProxy::UnmountAnimSet( const FxName& animSetName )
{
	if( _pSession )
	{
		return _pSession->UnmountAnimSet(animSetName);
	}
	return FxFalse;
}

FxBool FxSessionProxy::ImportAnimSet( const FxString& animSetPath )
{
	if( _pSession )
	{
		return _pSession->ImportAnimSet(animSetPath);
	}
	return FxFalse;
}

FxBool FxSessionProxy::ExportAnimSet( const FxName& animSetName, const FxString& animSetPath )
{
	if( _pSession )
	{
		return _pSession->ExportAnimSet(animSetName, animSetPath);
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Animation functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::AddAnim( const FxString& animGroup, const FxString& anim )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = group.FindAnim(anim.GetData());
				if( FxInvalidIndex == animIndex )
				{
					// Adding a new animation can potentially cause a reallocation,
					// so select nothing to begin with.
					FxString selAnimGroup;
					FxString selAnim;
					FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);
					FxSessionProxy::GetSelectedAnim(selAnim);
					_pSession->OnAnimChanged(NULL, FxAnimGroup::Default, NULL);
					FxAnim newAnim;
					newAnim.SetName(anim.GetData());
					// Set up the user data.
                    FxAnimUserData* pAnimUserData = _pSession->_findAnimUserData(animGroup.GetData(), anim.GetData());
					if( !pAnimUserData )
					{
						pAnimUserData = new FxAnimUserData();
						pAnimUserData->SetNames(animGroup, anim);
						pAnimUserData->SetIsSafeToUnload(FxFalse);
						_pSession->_addAnimUserData(pAnimUserData);
					}
					// If there is no phoneme word list, create one and fill it with silence by default.
					FxPhonWordList* pPhonemeWordList = pAnimUserData->GetPhonemeWordListPointer();
					if( !pPhonemeWordList )
					{
						pPhonemeWordList = new FxPhonWordList();
						pPhonemeWordList->AppendPhoneme(PHON_SIL, newAnim.GetStartTime(), newAnim.GetEndTime());
						pAnimUserData->SetPhonemeWordList(pPhonemeWordList);
						delete pPhonemeWordList;
						pPhonemeWordList = NULL;
					}
					pPhonemeWordList = pAnimUserData->GetPhonemeWordListPointer();
					if( pPhonemeWordList )
					{
						if( 0 == pPhonemeWordList->GetNumPhonemes() )
						{
							pPhonemeWordList->AppendPhoneme(PHON_SIL, newAnim.GetStartTime(), newAnim.GetEndTime());
						}
					}
                    newAnim.SetUserData(pAnimUserData);
					// Add the animation.
					group.AddAnim(newAnim);
					FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
					// Select the new animation.
					FxSessionProxy::SetSelectedAnimGroup(animGroup);
					FxSessionProxy::SetSelectedAnim(anim);
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddAnim( const FxString& animGroup, FxAnim& anim )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = group.FindAnim(anim.GetName());
				if( FxInvalidIndex == animIndex )
				{
					// Adding a new animation can potentially cause a reallocation,
					// so select nothing to begin with.
					FxString selAnimGroup;
					FxString selAnim;
					FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);
					FxSessionProxy::GetSelectedAnim(selAnim);
					_pSession->OnAnimChanged(NULL, FxAnimGroup::Default, NULL);

					// Set up the user data.
					// If the animation has no user data, attempt to attach user data
					// from the session and as a final resort create new user data.
					if( !anim.GetUserData() )
					{
						FxAnimUserData* pAnimUserData = _pSession->_findAnimUserData(animGroup.GetData(), anim.GetName());
						if( !pAnimUserData )
						{
							pAnimUserData = new FxAnimUserData();
							pAnimUserData->SetNames(animGroup, anim.GetNameAsString());
							pAnimUserData->SetIsSafeToUnload(FxFalse);
							_pSession->_addAnimUserData(pAnimUserData);
						}
						anim.SetUserData(pAnimUserData);
					}
					else
					{
						// If the animation has user data, remove any user data of the same name
						// from the session if the pointers do not match.
						FxAnimUserData* pSessionAnimUserData = _pSession->_findAnimUserData(animGroup.GetData(), anim.GetName());
						FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(anim.GetUserData());
						if( pSessionAnimUserData )
						{
							if( pSessionAnimUserData != pAnimUserData )
							{
								_pSession->_removeAnimUserData(animGroup.GetData(), anim.GetName());
								if( pAnimUserData )
								{
									pAnimUserData->SetIsSafeToUnload(FxFalse);
									_pSession->_addAnimUserData(pAnimUserData);
								}
							}
						}
						else
						{
							if( pAnimUserData )
							{
								pAnimUserData->SetIsSafeToUnload(FxFalse);
								_pSession->_addAnimUserData(pAnimUserData);
							}
						}
					}
					group.AddAnim(anim);
					FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
					// Select the new animation.
					FxSessionProxy::SetSelectedAnimGroup(selAnimGroup);
					FxSessionProxy::SetSelectedAnim(selAnim);
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveAnim( const FxString& animGroup, const FxString& anim )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = group.FindAnim(anim.GetData());
				if( FxInvalidIndex != animIndex )
				{
					_pSession->OnDeleteAnim(NULL, animGroup.GetData(), anim.GetData());
					FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
					return FxTrue;
				}
			}
		}
	}
	return FxFalse;
}


FxBool FxSessionProxy::GetAnim( const FxString& animGroup, const FxString& anim, FxAnim** ppAnim )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize animGroupIndex = pActor->FindAnimGroup(animGroup.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = group.FindAnim(anim.GetData());
				if( FxInvalidIndex != animIndex )
				{
					*ppAnim = group.GetAnimPtr(animIndex);
					if( *ppAnim )
					{
						if( !(*ppAnim)->GetUserData() )
						{
							(*ppAnim)->SetUserData(_pSession->_findAnimUserData(animGroup.GetData(), anim.GetData()));
						}
						return FxTrue;
					}
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::MoveAnim( const FxString& fromGroup, const FxString& toGroup, const FxString& anim )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize fromGroupIndex = pActor->FindAnimGroup(fromGroup.GetData());
			FxSize toGroupIndex   = pActor->FindAnimGroup(toGroup.GetData());
			if( FxInvalidIndex != fromGroupIndex &&
				FxInvalidIndex != toGroupIndex )
			{
				FxAnimGroup& from = pActor->GetAnimGroup(fromGroupIndex);
				FxAnimGroup& to   = pActor->GetAnimGroup(toGroupIndex);
				FxSize animIndexInFromGroup = from.FindAnim(anim.GetData());
				FxSize animIndexInToGroup   = to.FindAnim(anim.GetData());
				if( FxInvalidIndex != animIndexInFromGroup &&
					FxInvalidIndex == animIndexInToGroup )
				{
					// Adding and removing animations can potentially cause a
					// reallocation, so any selected animation has to be deselected
					// here.
					FxString selAnimGroup;
					FxString selAnim;
					FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);
					FxSessionProxy::GetSelectedAnim(selAnim);
					_pSession->OnAnimChanged(NULL, FxAnimGroup::Default, NULL);
					FxAnim tempAnim = from.GetAnim(animIndexInFromGroup);
					from.RemoveAnim(anim.GetData());
					to.AddAnim(tempAnim);
					// Update the user data.
					FxAnimUserData* pUserData = _pSession->_findAnimUserData(fromGroup.GetData(), anim.GetData());
					if( pUserData )
					{
						FxAnimUserData* pNewUserData = new FxAnimUserData(*pUserData);
						_pSession->_removeAnimUserData(fromGroup.GetData(), anim.GetData());
						pNewUserData->SetNames(toGroup, anim);
						pNewUserData->SetIsSafeToUnload(FxFalse);
						_pSession->_addAnimUserData(pNewUserData);
						FxSize animIndexInTo = to.FindAnim(anim.GetData());
						if( FxInvalidIndex != animIndexInTo )
						{
							FxAnim* pToAnim = to.GetAnimPtr(animIndexInTo);
							if( pToAnim )
							{
								pToAnim->SetUserData(pNewUserData);
							}
						}
					}
                    FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
					// If the moved animation was the currently selected animation,
					// update the animation selection.  Otherwise retain the selection.
					if( fromGroup == selAnimGroup && anim == selAnim )
					{
						FxSessionProxy::SetSelectedAnimGroup(toGroup);
						FxSessionProxy::SetSelectedAnim(anim);
					}
					else
					{
						FxSessionProxy::SetSelectedAnimGroup(selAnimGroup);
						FxSessionProxy::SetSelectedAnim(selAnim);
					}
					return FxTrue;
				}
				else
				{
					FxVM::DisplayError("either the animation does not exist in the source group or already exists in the destination group!");
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::CopyAnim( const FxString& fromGroup, const FxString& toGroup, const FxString& anim )
{
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxSize fromGroupIndex = pActor->FindAnimGroup(fromGroup.GetData());
			FxSize toGroupIndex   = pActor->FindAnimGroup(toGroup.GetData());
			if( FxInvalidIndex != fromGroupIndex &&
				FxInvalidIndex != toGroupIndex )
			{
				FxAnimGroup& from = pActor->GetAnimGroup(fromGroupIndex);
				FxAnimGroup& to   = pActor->GetAnimGroup(toGroupIndex);
				FxSize animIndexInFromGroup = from.FindAnim(anim.GetData());
				FxSize animIndexInToGroup   = to.FindAnim(anim.GetData());
				if( FxInvalidIndex != animIndexInFromGroup &&
					FxInvalidIndex == animIndexInToGroup )
				{
					// Adding and removing animations can potentially cause a
					// reallocation, so any selected animation has to be deselected
					// here.
					FxString selAnimGroup;
					FxString selAnim;
					FxArray<FxString> selCurves;
					FxArray<FxSize> selCurveIndices, selKeyIndices;
					FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);
					FxSessionProxy::GetSelectedAnim(selAnim);
					FxSessionProxy::GetSelectedAnimCurves(selCurves);
					FxSessionProxy::GetSelectedKeys(selCurveIndices, selKeyIndices);
					_pSession->OnAnimChanged(NULL, FxAnimGroup::Default, NULL);
					to.AddAnim(from.GetAnim(animIndexInFromGroup));
					// Update the user data.
					FxAnimUserData* pUserData = _pSession->_findAnimUserData(fromGroup.GetData(), anim.GetData());
					if( pUserData )
					{
						FxAnimUserData* pNewUserData = new FxAnimUserData(*pUserData);
						pNewUserData->SetNames(toGroup, anim);
						pNewUserData->SetIsSafeToUnload(FxFalse);
						_pSession->_addAnimUserData(pNewUserData);
						FxSize animIndexInTo = to.FindAnim(anim.GetData());
						if( FxInvalidIndex != animIndexInTo )
						{
							FxAnim* pToAnim = to.GetAnimPtr(animIndexInTo);
							if( pToAnim )
							{
								pToAnim->SetUserData(pNewUserData);
							}
						}
					}
					FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
					// Always retain the selection.
					FxSessionProxy::SetSelectedAnimGroup(selAnimGroup);
					FxSessionProxy::SetSelectedAnim(selAnim);
					FxSessionProxy::SetSelectedAnimCurves(selCurves);
					FxSessionProxy::SetSelectedKeys(selCurveIndices, selKeyIndices);

					return FxTrue;
				}
				else
				{
					FxVM::DisplayError("either the animation does not exist in the source group or already exists in the destination group!");
				}
			}
		}
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Playback functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::PlayAnimation( FxReal startTime, FxReal endTime, FxBool loop )
{
	if( _pSession )
	{
		_pSession->OnPlayAnimation(NULL, startTime, endTime, loop);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSessionProxy::StopAnimation( void )
{
	if( _pSession )
	{
		_pSession->OnStopAnimation(NULL);
		return FxTrue;
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Analysis functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::
AnalyzeDirectory( const FxString& directory, const FxString& animGroupName, 
				  const FxString& language, const FxString& coarticulationConfig,
				  const FxString& gestureConfig, FxBool generateSpeechGestures,
				  FxBool overwrite, FxBool recurse, FxBool notext )
{
	FxBool result = FxFalse;
	if( _pSession )
	{
		result = _pSession->AnalyzeDirectory(directory, animGroupName, language, 
			coarticulationConfig, gestureConfig, generateSpeechGestures, 
			overwrite, recurse, notext);
		FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
	}
	return result;
}

FxBool FxSessionProxy::
AnalyzeFile( const FxString& audioFile, const FxWString& optionalText, const FxString& animGroupName, 
			 const FxString& animName, const FxString& language, const FxString& coarticulationConfig,
			 const FxString& gestureConfig, FxBool generateSpeechGestures, FxBool overwrite )
{
	FxBool result = FxFalse;
	if( _pSession )
	{
		result = _pSession->AnalyzeFile(audioFile, optionalText, animGroupName, 
			animName, language, coarticulationConfig, gestureConfig, 
			generateSpeechGestures, overwrite);
		FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
	}
	return result;
}

FxBool FxSessionProxy::
AnalyzeRawAudio( FxAudio* pAudio, const FxWString& optionalText, const FxString& animGroupName, 
				 const FxString& animName, const FxString& language, const FxString& coarticulationConfig, 
				 const FxString& gestureConfig, FxBool generateSpeechGestures, FxBool overwrite )
{
	FxBool result = FxFalse;
	if( _pSession )
	{
		result = _pSession->AnalyzeRawAudio(pAudio, optionalText, animGroupName,
			animName, language, coarticulationConfig, gestureConfig, 
			generateSpeechGestures, overwrite);
		FxSessionProxy::ActorDataChanged(ADCF_AnimationsChanged);
	}
	return result;
}

FxBool FxSessionProxy::ChangeAudioMinMax( FxReal audioMin, FxReal audioMax )
{
	if( _pSession )
	{
		_pSession->OnAudioMinMaxChanged(NULL, audioMin, audioMax);
		return FxTrue;
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Curve functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::AddCurve( const FxString& group, const FxString& anim, const FxString& curve, FxSize index )
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			FxAnimCurve newCurve;
			newCurve.SetName(curve.GetData());
			// If the curve is going to the currently selected animation, the
			// user data needs to be created here.
			FxString selectedAnimGroup;
			FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
			FxString selectedAnim;
			FxSessionProxy::GetSelectedAnim(selectedAnim);
			if( group == selectedAnimGroup && anim == selectedAnim )
			{
				newCurve.SetUserData(new FxCurveUserData());
			}
			// Remember the selected curves.
			FxArray<FxString> selectedAnimCurves;
			FxSessionProxy::GetSelectedAnimCurves(selectedAnimCurves);
			// Adding a new animation curve could cause a reallocation, so
			// clear the selection and then reselect the curves after the add
			// operation.
			FxSessionProxy::SetSelectedAnimCurves(FxArray<FxString>());
			if( pAnim->InsertAnimCurve(newCurve, index) )
			{
				FxSessionProxy::SetSelectedAnimCurves(selectedAnimCurves);
				FxSessionProxy::ActorDataChanged(ADCF_CurvesChanged);
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::AddCurve( const FxString& group, const FxString& anim, FxAnimCurve& curve, FxSize index )
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			// If the curve is going to the currently selected animation, the
			// user data needs to be created here.
			FxString selectedAnimGroup;
			FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
			FxString selectedAnim;
			FxSessionProxy::GetSelectedAnim(selectedAnim);
			if( group == selectedAnimGroup && anim == selectedAnim )
			{
				if( !curve.GetUserData() )
				{
					curve.SetUserData(new FxCurveUserData());
				}
				for( FxSize i = 0; i < curve.GetNumKeys(); ++i )
				{
					FxAnimKey& key = curve.GetKeyM(i);
					key.SetUserData(new FxKeyUserData());
				}
			}
			// Remember the selected curves.
			FxArray<FxString> selectedAnimCurves;
			FxSessionProxy::GetSelectedAnimCurves(selectedAnimCurves);
			// Adding a new animation curve could cause a reallocation, so
			// clear the selection and then reselect the curves after the add
			// operation.
			FxSessionProxy::SetSelectedAnimCurves(FxArray<FxString>());
			if( pAnim->InsertAnimCurve(curve, index) )
			{
				// After adding the curve, set the curve reference passed in
				// to have NULL user data because it was copied into the actual
				// curve added to the animation in the above code.
				curve.SetUserData(NULL);
				for( FxSize i = 0; i < curve.GetNumKeys(); ++i )
				{
					FxAnimKey& key = curve.GetKeyM(i);
					key.SetUserData(NULL);
				}
				FxSessionProxy::SetSelectedAnimCurves(selectedAnimCurves);
				FxSessionProxy::ActorDataChanged(ADCF_CurvesChanged);
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::RemoveCurve( const FxString& group, const FxString& anim, const FxString& curve, FxSize& oldIndex )
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			oldIndex = pAnim->FindAnimCurve(curve.GetData());
			// Remember the selected curves.
			FxArray<FxString> selectedAnimCurves;
			FxSessionProxy::GetSelectedAnimCurves(selectedAnimCurves);
			// Removing an animation curve could cause a reallocation, so
			// clear the selection and then reselect the curves after the remove
			// operation.
			FxSessionProxy::SetSelectedAnimCurves(FxArray<FxString>());
			// If the curve is coming from the currently selected animation, the
			// user data needs to be destroyed here or it will leak.
			FxString selectedAnimGroup;
			FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
			FxString selectedAnim;
			FxSessionProxy::GetSelectedAnim(selectedAnim);
			if( group == selectedAnimGroup && anim == selectedAnim )
			{
				if( FxInvalidIndex != oldIndex )
				{
					FxAnimCurve& curveToRemove = pAnim->GetAnimCurveM(oldIndex);
					for( FxSize i = 0; i < curveToRemove.GetNumKeys(); ++i )
					{
						FxAnimKey& key = curveToRemove.GetKeyM(i);
						delete reinterpret_cast<FxKeyUserData*>(key.GetUserData());
						key.SetUserData(NULL);
					}
					delete reinterpret_cast<FxCurveUserData*>(curveToRemove.GetUserData());
					curveToRemove.SetUserData(NULL);
				}
			}
			if( pAnim->RemoveAnimCurve(curve.GetData()) )
			{
				FxSessionProxy::SetSelectedAnimCurves(selectedAnimCurves);
				ActorDataChanged(ADCF_CurvesChanged);
				return FxTrue;
			}
			else
			{
				// We need to reset the curve selection or we leave the app in
				// an inconsistent state.
				FxSessionProxy::SetSelectedAnimCurves(selectedAnimCurves);
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetCurveOwner( const FxString& group, const FxString& anim, const FxString& curve, FxBool& isOwnedByAnalysis)
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			FxAnimUserData* userData = reinterpret_cast<FxAnimUserData*>(pAnim->GetUserData());
			if( userData )
			{
				isOwnedByAnalysis = userData->IsCurveOwnedByAnalysis(curve.GetData());
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetCurveOwner( const FxString& group, const FxString& anim, const FxString& curve, FxBool isOwnedByAnalysis)
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			FxAnimUserData* userData = reinterpret_cast<FxAnimUserData*>(pAnim->GetUserData());
			if( userData )
			{
				userData->SetCurveOwnedByAnalysis(curve.GetData(), isOwnedByAnalysis);
				ActorDataChanged(ADCF_CurvesChanged);
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetCurveInterp( const FxString& group, const FxString& anim, const FxString& curve, FxInterpolationType& interpolator )
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			FxSize curveIndex = pAnim->FindAnimCurve(curve.GetData());
			if( curveIndex != FxInvalidIndex )
			{
				interpolator = pAnim->GetAnimCurveM(curveIndex).GetInterpolator();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::SetCurveInterp( const FxString& group, const FxString& anim, const FxString& curve, FxInterpolationType interpolator )
{
	FxAnim* pAnim = NULL;
	if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
	{
		if( pAnim )
		{
			FxSize curveIndex = pAnim->FindAnimCurve(curve.GetData());
			if( curveIndex != FxInvalidIndex )
			{
				pAnim->GetAnimCurveM(curveIndex).SetInterpolator(interpolator);
				RefreshControls();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxSessionProxy::GetCurve( const FxString& group, const FxString& anim, const FxString& curve, FxAnimCurve** ppCurve )
{
	if( ppCurve )
	{
		FxAnim* pAnim = NULL;
		if( FxSessionProxy::GetAnim(group, anim, &pAnim) )
		{
			if( pAnim )
			{
				FxSize curveIndex = pAnim->FindAnimCurve(curve.GetData());
				if( curveIndex != FxInvalidIndex )
				{
					*ppCurve = &pAnim->GetAnimCurveM(curveIndex);
					return FxTrue;
				}
			}
		}
		*ppCurve = NULL;
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Key functions.
//------------------------------------------------------------------------------
FxBool FxSessionProxy::GetSelectedKeys( FxArray<FxSize>& curveIndices, FxArray<FxSize>& keyIndices )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		FxSize numSelectedKeys = pAnimController->GetNumSelectedKeys();
		curveIndices.Clear();
		keyIndices.Clear();
		curveIndices.Reserve(numSelectedKeys);
		keyIndices.Reserve(numSelectedKeys);
		FxSize numCurves = pAnimController->GetNumCurves();
		for( FxSize i = 0; i < numCurves; ++i )
		{
			if( pAnimController->IsCurveVisible(i) )
			{
				FxSize numKeys = pAnimController->GetNumKeys(i);
				for( FxSize j = 0; j < numKeys; ++j )
				{
					if( pAnimController->IsKeySelected(i, j) && !pAnimController->IsKeyTemporary(i, j) )
					{
						curveIndices.PushBack(i);
						keyIndices.PushBack(j);
					}
				}
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

// Sets the selected keys from parallel arrays of curve/key indices.
FxBool FxSessionProxy::SetSelectedKeys( const FxArray<FxSize>& curveIndices, const FxArray<FxSize>& keyIndices )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( pAnimController && curveIndices.Length() == keyIndices.Length() )
		{
			pAnimController->ClearSelection();
			FxSize num = curveIndices.Length();
			for( FxSize i = 0; i < num; ++i )
			{
				pAnimController->SetKeySelected(curveIndices[i], keyIndices[i], FxTrue);
			}
			pAnimController->RecalcPivotKey();
			RefreshControls();
			return FxTrue;
		}
	}
	return FxFalse;
}

// Clears the current key selection
FxBool FxSessionProxy::ClearSelectedKeys( void )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		_pSession->GetAnimController()->ClearSelection();
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Applies a selection to the keys based on various criteria.
FxBool FxSessionProxy::
ApplyKeySelection( FxReal minTime, FxReal maxTime, FxReal minValue, FxReal maxValue, FxBool toggle )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( !toggle )
		{
			pAnimController->ClearSelection();
		}

		FxReal time, value, slopeIn, slopeOut;
		FxSize numCurves = pAnimController->GetNumCurves();
		for( FxSize i = 0; i < numCurves; ++i )
		{
			if( pAnimController->IsCurveVisible(i) && !pAnimController->IsCurveOwnedByAnalysis(i) )
			{
				FxSize numKeys = pAnimController->GetNumKeys(i);
				for( FxSize j = 0; j < numKeys; ++j )
				{
					if( !pAnimController->IsKeyTemporary(i, j) )
					{
						pAnimController->GetKeyInfo(i, j, time, value, slopeIn, slopeOut);
						if( (minTime != FxInvalidValue && time < minTime) ||
							(maxTime != FxInvalidValue && time > maxTime) ||
							(minValue != FxInvalidValue && value < minValue) ||
							(maxValue != FxInvalidValue && value > maxValue) )
						{
							continue;
						}
						if( toggle )
						{
							FxBool isSelected = pAnimController->IsKeySelected(i, j);
							pAnimController->SetKeySelected(i, j, !isSelected);
						}
						else
						{
							pAnimController->SetKeySelected(i, j, FxTrue);
						}
					}
				}
			}
		}
		pAnimController->RecalcPivotKey();
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Transforms the selected keys by a time or value delta.
FxBool FxSessionProxy::TransformSelectedKeys( FxReal timeDelta, FxReal valueDelta )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( timeDelta == FxInvalidValue ) timeDelta = 0.0f;
		if( valueDelta == FxInvalidValue ) valueDelta = 0.0f;
		pAnimController->TransformSelection(timeDelta, valueDelta);
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Sets all selected keys's slopes to specific values.
FxBool FxSessionProxy::SetSelectedKeysSlopes( FxReal slope )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( slope == FxInvalidValue ) slope = 0.0f;
		pAnimController->TransformSelection(0.0f, 0.0f, slope);
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Gets the pivot curve index and key index.
FxBool FxSessionProxy::GetPivotKey( FxSize& curveIndex, FxSize& keyIndex )
{
	if( _pSession && _pSession->GetAnimController())
	{
		_pSession->GetAnimController()->GetPivotKey(curveIndex, keyIndex);
		return FxTrue;
	}
	return FxFalse;
}

// Sets the pivot curve index and key index.
FxBool FxSessionProxy::SetPivotKey( FxSize curveIndex, FxSize keyIndex )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		pAnimController->SetPivotKey(curveIndex, keyIndex);
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Deletes the selection.
FxBool FxSessionProxy::RemoveKeySelection( void )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		_pSession->GetAnimController()->DeleteSelection();
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Returns the information for the selected keys as parallel arrays.
FxBool FxSessionProxy::
GetSelectedKeyInfos( FxArray<FxReal>& times, FxArray<FxReal>& values, 
					 FxArray<FxReal>& incDerivs, FxArray<FxReal>& outDerivs, 
					 FxArray<FxInt32>& flags )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		FxSize numSelectedKeys = pAnimController->GetNumSelectedKeys();
		times.Clear();
		values.Clear();
		incDerivs.Clear();
		outDerivs.Clear();
		flags.Clear();
		times.Reserve(numSelectedKeys);
		values.Reserve(numSelectedKeys);
		incDerivs.Reserve(numSelectedKeys);
		outDerivs.Reserve(numSelectedKeys);
		flags.Reserve(numSelectedKeys);
		FxReal time, value, slopeIn, slopeOut;
		FxSize numCurves = pAnimController->GetNumCurves();
		for( FxSize i = 0; i < numCurves; ++i )
		{
			if( pAnimController->IsCurveVisible(i) )
			{
				FxSize numKeys = pAnimController->GetNumKeys(i);
				for( FxSize j = 0; j < numKeys; ++j )
				{
					if( pAnimController->IsKeySelected(i, j) )
					{
						pAnimController->GetKeyInfo(i, j, time, value, slopeIn, slopeOut);
						times.PushBack(time);
						values.PushBack(value);
						incDerivs.PushBack(slopeIn);
						outDerivs.PushBack(slopeOut);
						flags.PushBack(KeyIsSelected |
									   (pAnimController->IsKeyPivot(i,j) ? KeyIsPivot : 0) |
									   (pAnimController->IsTangentLocked(i,j) ? TangentsLocked : 0));
					}
				}
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

// Inserts keys into curves.
FxBool FxSessionProxy::
InsertKeys( const FxArray<FxSize>& curveIndicies, const FxArray<FxReal>& times, 
		    const FxArray<FxReal>& values, const FxArray<FxReal>& incDerivs, 
			const FxArray<FxReal>& outDerivs, const FxArray<FxInt32>& flags )
{
	FxSize numKeys = curveIndicies.Length();
	if( _pSession && _pSession->GetAnimController() && 
		numKeys == times.Length() && numKeys == values.Length() &&
		numKeys == incDerivs.Length() && numKeys == outDerivs.Length() )
	{
		for( FxSize i = 0; i < numKeys; ++i )
		{
			_pSession->GetAnimController()->InsertKey(curveIndicies[i], times[i], values[i], incDerivs[i], outDerivs[i], flags[i]);
		}
		RefreshControls();
		return FxTrue;
	}
	return FxFalse;
}

// Gets the information for a single key.
FxBool FxSessionProxy::GetKeyInfo( FxSize curveIndex, FxSize keyIndex, FxReal& time, FxReal& value, FxReal& slopeIn, FxReal& slopeOut )
{
	if( _pSession && _pSession->GetAnimController())
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( pAnimController->GetNumCurves() > curveIndex && 
			pAnimController->GetNumKeys(curveIndex) > keyIndex )
		{
			pAnimController->GetKeyInfo(curveIndex, keyIndex, time, value, slopeIn, slopeOut);
			return FxTrue;
		}
	}
	return FxFalse;
}

// Edits the information for a single key.
FxBool FxSessionProxy::EditKeyInfo( FxSize curveIndex, FxSize& keyIndex, FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( pAnimController->GetNumCurves() > curveIndex && 
			pAnimController->GetNumKeys(curveIndex) > keyIndex &&
			!pAnimController->IsCurveOwnedByAnalysis(curveIndex) )
		{
			// Cache whether the key had tangents locked
			FxBool isTangentsLocked = pAnimController->IsTangentLocked(curveIndex, keyIndex);
			pAnimController->RemoveKey(curveIndex, keyIndex);
			keyIndex = pAnimController->InsertKey(curveIndex, time, value, slopeIn, slopeOut, 0);
			pAnimController->SetTangentLocked(curveIndex, keyIndex, isTangentsLocked);
			pAnimController->SetKeySelected(curveIndex, keyIndex);
			pAnimController->SetPivotKey(curveIndex, keyIndex);
			RefreshControls();
			return FxTrue;
		}
	}
	return FxFalse;
}

// Inserts a single key and fills out the index where it was placed.
FxBool FxSessionProxy::InsertKey( FxSize curveIndex, FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut, FxSize& keyIndex, FxBool clearSelection, FxBool autoSlope )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( !pAnimController->IsCurveOwnedByAnalysis(curveIndex) )
		{
			if( clearSelection )
			{
				pAnimController->ClearSelection();
			}
			keyIndex = pAnimController->InsertKey(curveIndex, time, value, slopeIn, slopeOut, TangentsLocked, autoSlope);
			pAnimController->SetKeySelected(curveIndex, keyIndex);
			pAnimController->SetPivotKey(curveIndex, keyIndex);
			RefreshControls();
			return FxTrue;
		}
		else
		{
			FxVM::DisplayWarning("Selected curve is owned by analysis.");
		}
	}
	return FxFalse;
}

// Removes a single key.
FxBool FxSessionProxy::RemoveKey( FxSize curveIndex, FxSize keyIndex )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		FxAnimController* pAnimController = _pSession->GetAnimController();
		if( !pAnimController->IsCurveOwnedByAnalysis(curveIndex) && 
			!pAnimController->IsKeyTemporary(curveIndex, keyIndex) )
		{
			pAnimController->RemoveKey(curveIndex, keyIndex);
			RefreshControls();
			return FxTrue;
		}
	}
	return FxFalse;
}

// Inserts a key into all selected curves, or the single visible curve.
FxBool FxSessionProxy::DefaultInsertKey( FxReal time, FxReal value )
{
	if( _pSession && _pSession->GetAnimController() )
	{
		if( _pSession->GetAnimController()->InsertNewKey(time, value) )
		{
			_pSession->GetAnimController()->RecalcPivotKey();
			RefreshControls();
			return FxTrue;
		}
		else
		{
			FxVM::DisplayWarning("All selected curves are owned by analysis.");
		}
	}
	return FxFalse;
}

// Disables sending refresh messages.
void FxSessionProxy::SetNoRefreshMessages( FxBool state )
{
	_noRefreshMessages = state;
}

// Tries to send a refresh message.
void FxSessionProxy::RefreshControls( void )
{
	if( _pSession && !_noRefreshMessages )
	{
		_pSession->OnRefresh(NULL);
	}
}

//------------------------------------------------------------------------------
// Internal use only.
//------------------------------------------------------------------------------
void FxSessionProxy::SetSession( void* pSession )
{
	_pSession = reinterpret_cast<FxStudioSession*>(pSession);
}

void FxSessionProxy::GetSession( void** ppSession )
{
	*ppSession = _pSession;
}

void FxSessionProxy::SetNumDummyRegisters( FxSize numDummyRegisters )
{
	if( _pSession )
	{
		_pSession->_dummyRegisters.Clear();
		if( numDummyRegisters > 0 )
		{
			_pSession->_dummyRegisters.Reserve(numDummyRegisters);
			for( FxSize i = 0; i < numDummyRegisters; ++i )
			{
				_pSession->_dummyRegisters.PushBack(FxRegister());
			}
		}
	}
}

void FxSessionProxy::GetDummyRegister( const FxName& regName, FxReal& regValue )
{
	regValue = 0.0f;
	if( _pSession )
	{
		FxActor* pActor = _pSession->GetActor();
		if( pActor )
		{
			FxCompiledFaceGraph& cg = pActor->GetCompiledFaceGraph();
			FxSize regIndex = cg.FindNodeIndex(regName);
			if( FxInvalidIndex != regIndex )
			{
				regValue = _pSession->_dummyRegisters[regIndex].value;
			}
		}
	}
}

void FxSessionProxy::SetPhonemeList( void* pPhonemeList )
{
	if( pPhonemeList )
	{
		_pPhonWordList = reinterpret_cast<FxPhonWordList*>(pPhonemeList);
	}
}

void FxSessionProxy::RefreshUndoRedoMenu( void )
{
	if( _pSession )
	{
		_pSession->RefreshUndoRedoMenu();
	}
}

void FxSessionProxy::PreRemoveAnim( const FxString& animGroup, const FxString& anim )
{
	if( _pSession )
	{
		// If the animation to be deleted is also selected remove any user data.
		FxString selAnimGroup;
		FxString selAnim;
		FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);
		FxSessionProxy::GetSelectedAnim(selAnim);
		if( animGroup == selAnimGroup && anim == selAnim )
		{
			if( _pSession->_pAnimController )
			{
				_pSession->_pAnimController->DestroyUserData();
			}
		}
	}
}

} // namespace Face

} // namespace OC3Ent
