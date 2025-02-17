/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnLinkedObjEditor.h"
#include "EngineAnimClasses.h"
#include "UnLinkedObjDrawUtils.h"
#include "AnimTreeEditor.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"

IMPLEMENT_CLASS(UAnimTreeEdSkelComponent);
IMPLEMENT_CLASS(UAnimNodeEditInfo);

static INT	DuplicateOffset = 30;

/*-----------------------------------------------------------------------------
	WxMBAnimTreeEdNewNode
-----------------------------------------------------------------------------*/

class WxMBAnimTreeEdNewNode : public wxMenu
{
public:
	WxMBAnimTreeEdNewNode(WxAnimTreeEditor* AnimTreeEd)
	{
		///////// Add animation classes.
		INT SeqClassIndex = INDEX_NONE;
		for(INT i=0; i<AnimTreeEd->AnimNodeClasses.Num(); i++)
		{
			// Add AnimNodeSequence option at bottom of list because its special.
			if( AnimTreeEd->AnimNodeClasses(i) == UAnimNodeSequence::StaticClass() )
			{
				SeqClassIndex = i;
			}
			else
			{
				Append( IDM_ANIMTREE_NEW_NODE_START+i, *AnimTreeEd->AnimNodeClasses(i)->GetDescription(), TEXT("") );
			}
		}
		check( SeqClassIndex != INDEX_NONE );
		AppendSeparator();
		Append( IDM_ANIMTREE_NEW_NODE_START+SeqClassIndex, *LocalizeUnrealEd("AnimSequencePlayer"), TEXT("") );

		///////// Add morph target classes.
		AppendSeparator();
		INT PoseClassIndex = INDEX_NONE;
		for(INT i=0; i<AnimTreeEd->MorphNodeClasses.Num(); i++)
		{
			// Add UMorphNodePose option at bottom of list because its special.
			if( AnimTreeEd->MorphNodeClasses(i) == UMorphNodePose::StaticClass() )
			{
				PoseClassIndex = i;
			}
			else
			{
				Append( IDM_ANIMTREE_NEW_MORPH_START+i, *AnimTreeEd->MorphNodeClasses(i)->GetDescription(), TEXT("") );
			}
		}
		check( PoseClassIndex != INDEX_NONE );
		AppendSeparator();
		Append( IDM_ANIMTREE_NEW_MORPH_START+PoseClassIndex, *LocalizeUnrealEd("MorphNodePose"), TEXT("") );


		////////// Add SkelControl classes
		AppendSeparator();
		for(INT i=0; i<AnimTreeEd->SkelControlClasses.Num(); i++)
		{
			Append( IDM_ANIMTREE_NEW_CONTROL_START+i, *AnimTreeEd->SkelControlClasses(i)->GetDescription(), TEXT("") );
		}
	}
};

/*-----------------------------------------------------------------------------
	WxMBAnimTreeEdNodeOptions
-----------------------------------------------------------------------------*/

class WxMBAnimTreeEdNodeOptions : public wxMenu
{
public:
	WxMBAnimTreeEdNodeOptions(WxAnimTreeEditor* AnimTreeEd)
	{
		INT NumSelected = AnimTreeEd->SelectedNodes.Num();
		if(NumSelected == 1)
		{
			// See if we adding another input would exceed max child nodes.
			UAnimNodeBlendBase* BlendNode = Cast<UAnimNodeBlendBase>( AnimTreeEd->SelectedNodes(0) );
			if( BlendNode && !BlendNode->bFixNumChildren )
			{
				Append( IDM_ANIMTREE_ADD_INPUT, *LocalizeUnrealEd("AddInput"), TEXT("") );
			}

			if( AnimTreeEd->SelectedNodes(0)->IsA(UAnimTree::StaticClass()) )
			{
				Append( IDM_ANIMTREE_ADD_CONTROLHEAD, *LocalizeUnrealEd("AddSkelControlChain") );
			}

			AppendSeparator();
		}

		Append( IDM_ANIMTREE_BREAK_ALL_LINKS, *LocalizeUnrealEd("BreakAllLinks"), TEXT("") );
		AppendSeparator();

		NumSelected += AnimTreeEd->SelectedControls.Num();
		NumSelected += AnimTreeEd->SelectedMorphNodes.Num();
		if( NumSelected == 1 )
		{
			Append( IDM_ANIMTREE_DELETE_NODE, *LocalizeUnrealEd("DeleteSelectedItem"), TEXT("") );
		}
		else
		{
			Append( IDM_ANIMTREE_DELETE_NODE, *LocalizeUnrealEd("DeleteSelectedItems"), TEXT("") );
		}
	}
};

/*-----------------------------------------------------------------------------
	WxMBAnimTreeEdConnectorOptions
-----------------------------------------------------------------------------*/

class WxMBAnimTreeEdConnectorOptions : public wxMenu
{
public:
	WxMBAnimTreeEdConnectorOptions(WxAnimTreeEditor* AnimTreeEd)
	{
		UAnimTree* Tree = Cast<UAnimTree>(AnimTreeEd->ConnObj);
		UAnimNodeBlendBase* BlendNode = Cast<UAnimNodeBlendBase>(AnimTreeEd->ConnObj);
		USkelControlBase* SkelControl = Cast<USkelControlBase>(AnimTreeEd->ConnObj);
		UMorphNodeWeightBase* MorphNode = Cast<UMorphNodeWeightBase>(AnimTreeEd->ConnObj);

		// Only display the 'Break Link' option if there is a link to break!
		UBOOL bHasNodeConnection = false;
		UBOOL bHasControlConnection = false;
		UBOOL bHasMorphConnection = false;
		if( AnimTreeEd->ConnType == LOC_OUTPUT )
		{
			if(Tree)
			{
				// Animation
				if(AnimTreeEd->ConnIndex == 0)
				{
					if( Tree->Children(0).Anim )
					{
						bHasNodeConnection = true;
					}
				}
				// Morph
				else if(AnimTreeEd->ConnIndex == 1)
				{
					if( Tree->RootMorphNodes.Num() > 0 )
					{
						bHasMorphConnection = true;
					}
				}
				// Controls
				else
				{
					if( Tree->SkelControlLists(AnimTreeEd->ConnIndex-2).ControlHead )
					{
						bHasControlConnection = true;
					}
				}
			}
			else if(BlendNode)
			{		
				if( BlendNode->Children(AnimTreeEd->ConnIndex).Anim )
				{
					bHasNodeConnection = true;
				}
			}
			else if(SkelControl)
			{
				if(SkelControl->NextControl)
				{
					bHasControlConnection = true;
				}
			}
			else if(MorphNode)
			{
				if(MorphNode->NodeConns(AnimTreeEd->ConnIndex).ChildNodes.Num() > 0)
				{
					bHasMorphConnection = true;
				}
			}
		}

		if(bHasNodeConnection || bHasControlConnection || bHasMorphConnection)
		{
			Append( IDM_ANIMTREE_BREAK_LINK, *LocalizeUnrealEd("BreakLink"), TEXT("") );
		}

		// If on an input that can be deleted, show option
		if( BlendNode && !Tree &&
			AnimTreeEd->ConnType == LOC_OUTPUT )
		{
			Append( IDM_ANIMTREE_NAME_INPUT, *LocalizeUnrealEd("NameInput"), TEXT("") );

			if( !BlendNode->bFixNumChildren )
			{
				if(bHasNodeConnection || bHasControlConnection)
				{
					AppendSeparator();
				}

				Append( IDM_ANIMTREE_DELETE_INPUT,	*LocalizeUnrealEd("DeleteInput"),	TEXT("") );
			}
		}

		if( Tree && AnimTreeEd->ConnIndex > 0)
		{
			if(bHasNodeConnection || bHasControlConnection)
			{
				AppendSeparator();
			}

			Append( IDM_ANIMTREE_CHANGEBONE_CONTROLHEAD,  *LocalizeUnrealEd("ChangeBone") );
			Append( IDM_ANIMTREE_DELETE_CONTROLHEAD, *LocalizeUnrealEd("DeleteSkelControlChain") );
		}
	}
};

void WxAnimTreeEditor::OpenNewObjectMenu()
{
	WxMBAnimTreeEdNewNode menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxAnimTreeEditor::OpenObjectOptionsMenu()
{
	WxMBAnimTreeEdNodeOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxAnimTreeEditor::OpenConnectorOptionsMenu()
{
	WxMBAnimTreeEdConnectorOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxAnimTreeEditor::DoubleClickedObject(UObject* Obj)
{
	UAnimNode* ClickedNode = Cast<UAnimNode>(Obj);
	if(ClickedNode)
	{
		UAnimNodeEditInfo* EditInfo = FindAnimNodeEditInfo(ClickedNode);
		if(EditInfo)
		{
			EditInfo->OnDoubleClickNode(ClickedNode, this);
		}
	}
}

void WxAnimTreeEditor::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{
	if (BackgroundTexture != NULL)
	{
		Clear(Canvas, FColor(161,161,161) );

		Canvas->PushAbsoluteTransform(FMatrix::Identity);

		const INT ViewWidth = LinkedObjVC->Viewport->GetSizeX();
		const INT ViewHeight = LinkedObjVC->Viewport->GetSizeY();

		// draw the texture to the side, stretched vertically
		FLinkedObjDrawUtils::DrawTile( Canvas, ViewWidth - BackgroundTexture->SizeX, 0,
			BackgroundTexture->SizeX, ViewHeight,
			0.f, 0.f,
			1.f, 1.f,
			FLinearColor::White,
			BackgroundTexture->Resource );

		// stretch the left part of the texture to fill the remaining gap
		if (ViewWidth > BackgroundTexture->SizeX)
		{
			FLinkedObjDrawUtils::DrawTile( Canvas, 0, 0,
				ViewWidth - BackgroundTexture->SizeX, ViewHeight,
				0.f, 0.f,
				0.1f, 0.1f,
				FLinearColor::White,
				BackgroundTexture->Resource );
		}

		Canvas->PopTransform();
	}

	{
	const DOUBLE StartTime = appSeconds();
	for(INT i=0; i<TreeNodes.Num(); i++)
	{
		UAnimNode* DrawNode = TreeNodes(i);
		const UBOOL bNodeSelected = SelectedNodes.ContainsItem( DrawNode );
		DrawNode->DrawAnimNode( Canvas, bNodeSelected, bShowNodeWeights, bDrawCurves );

		// If this is the 'viewed' node - draw icon above it.
		if(DrawNode == PreviewSkelComp->Animations)
		{
			FLinkedObjDrawUtils::DrawTile( Canvas, DrawNode->NodePosX,		DrawNode->NodePosY - 3 - 8, 10,	10, 0.f, 0.f, 1.f, 1.f, FColor(0,0,0) );
			FLinkedObjDrawUtils::DrawTile( Canvas, DrawNode->NodePosX + 1,	DrawNode->NodePosY - 2 - 8, 8,	8,	0.f, 0.f, 1.f, 1.f, FColor(255,215,0) );
		}
	}
	//debugf( TEXT("AnimTree::DrawObjects -- AnimNode(%lf msecs)"), 1000.0*(appSeconds() - StartTime) );
	}

	{
	const DOUBLE StartTime = appSeconds();
	// Draw SkelControls.
	for(INT i=0; i<TreeControls.Num(); i++)
	{
		USkelControlBase* SkelControl = TreeControls(i);
		const UBOOL bControlSelected = SelectedControls.ContainsItem( SkelControl );
		SkelControl->DrawSkelControl( Canvas, bControlSelected, bDrawCurves );
	}
	//debugf( TEXT("AnimTree::DrawObjects -- SkelControl(%lf msecs)"), 1000.0*(appSeconds() - StartTime) );
	}

	{
	const DOUBLE StartTime = appSeconds();
	// Draw MorphNodes
	for(INT i=0; i<TreeMorphNodes.Num(); i++)
	{
		UMorphNodeBase* DrawNode = TreeMorphNodes(i);
		const UBOOL bNodeSelected = SelectedMorphNodes.ContainsItem( DrawNode );
		DrawNode->DrawMorphNode( Canvas, bNodeSelected, bDrawCurves );
	}
	//debugf( TEXT("AnimTree::DrawObjects -- MorphNode(%lf msecs)"), 1000.0*(appSeconds() - StartTime) );
	}
}

void WxAnimTreeEditor::UpdatePropertyWindow()
{
	// If we have both controls and nodes selected - just show nodes in property window...
	if(SelectedNodes.Num() > 0)
	{
		PropertyWindow->SetObjectArray( SelectedNodes, 1,0,1 );
	}
	else if(SelectedControls.Num() > 0)
	{
		PropertyWindow->SetObjectArray( SelectedControls, 1,0,1 );
	}
	else
	{
		PropertyWindow->SetObjectArray( SelectedMorphNodes, 1,0,1 );
	}
}

void WxAnimTreeEditor::EmptySelection()
{
	SelectedNodes.Empty();
	SelectedControls.Empty();
	SelectedMorphNodes.Empty();
}

void WxAnimTreeEditor::AddToSelection( UObject* Obj )
{
	if( Obj->IsA(UAnimNode::StaticClass()) )
	{
		SelectedNodes.AddItem( (UAnimNode*)Obj );
	}
	else if( Obj->IsA(USkelControlBase::StaticClass()) )
	{
		SelectedControls.AddItem( (USkelControlBase*)Obj );
	}
	else
	{
		SelectedMorphNodes.AddItem( (UMorphNodeBase*)Obj );
	}	
}

void WxAnimTreeEditor::RemoveFromSelection( UObject* Obj )
{
	if( Obj->IsA(UAnimNode::StaticClass()) )
	{
		SelectedNodes.RemoveItem( (UAnimNode*)Obj );
	}
	else if( Obj->IsA(USkelControlBase::StaticClass()) )
	{
		SelectedControls.RemoveItem( (USkelControlBase*)Obj );
	}
	else
	{
		SelectedMorphNodes.RemoveItem( (UMorphNodeBase*)Obj );
	}	
}

UBOOL WxAnimTreeEditor::IsInSelection( UObject* Obj ) const
{
	if( Obj->IsA(UAnimNode::StaticClass()) )
	{
		return SelectedNodes.ContainsItem( (UAnimNode*)Obj );
	}
	else if( Obj->IsA(USkelControlBase::StaticClass()) )
	{
		return SelectedControls.ContainsItem( (USkelControlBase*)Obj );
	}
	else
	{
		return SelectedMorphNodes.ContainsItem( (UMorphNodeBase*)Obj );
	}
}

INT WxAnimTreeEditor::GetNumSelected() const
{
	return SelectedNodes.Num() + SelectedControls.Num() + SelectedMorphNodes.Num();
}

void WxAnimTreeEditor::SetSelectedConnector( FLinkedObjectConnector& Connector )
{
	ConnObj = Connector.ConnObj;
	ConnType = Connector.ConnType;
	ConnIndex = Connector.ConnIndex;
}

FIntPoint WxAnimTreeEditor::GetSelectedConnLocation(FCanvas* Canvas)
{
	if(ConnObj)
	{
		UAnimNode* AnimNode = Cast<UAnimNode>(ConnObj);
		if(AnimNode)
		{
			return AnimNode->GetConnectionLocation(ConnType, ConnIndex);
		}

		USkelControlBase* Control = Cast<USkelControlBase>(ConnObj);
		if(Control)
		{
			return Control->GetConnectionLocation(ConnType);
		}

		UMorphNodeBase* MorphNode = Cast<UMorphNodeBase>(ConnObj);
		if(MorphNode)
		{
			return MorphNode->GetConnectionLocation(ConnType, ConnIndex);
		}
	}

	return FIntPoint(0,0);
}

INT WxAnimTreeEditor::GetSelectedConnectorType()
{
	return ConnType;
}

FColor WxAnimTreeEditor::GetMakingLinkColor()
{
	if( ConnObj->IsA(USkelControlBase::StaticClass()) || (ConnObj->IsA(UAnimTree::StaticClass()) && ConnIndex > 1) )
	{
		return FColor(50,100,50);
	}
	else if( ConnObj->IsA(UMorphNodeBase::StaticClass()) || (ConnObj->IsA(UAnimTree::StaticClass()) && ConnIndex == 1) )
	{
		return FColor(50,50,100);
	}
	else if( ConnObj->IsA(UMorphNodeBase::StaticClass()) )
	{
		return FColor(100,50,50);
	}
	else
	{
		return FColor(0,0,0);
	}
}

static UBOOL CheckAnimNodeReachability(UAnimNode* Start, UAnimNode* TestDest)
{
	UAnimNodeBlendBase* StartBlend = Cast<UAnimNodeBlendBase>(Start);
	if(StartBlend)
	{
		// If we are starting from a blend - walk from this node and find all nodes we reach.
		TArray<UAnimNode*> Nodes;
		StartBlend->GetNodes(Nodes);

		// If our test destination is in that set, return true.
		return Nodes.ContainsItem(TestDest);
	}
	// If start is not a blend - no way we can reach anywhere.
	else
	{
		return false;
	}
}

static UBOOL CheckMorphNodeReachability(UMorphNodeBase* Start, UMorphNodeBase* TestDest)
{
	UMorphNodeWeightBase* StartWeight = Cast<UMorphNodeWeightBase>(Start);
	if(StartWeight)
	{
		// If we are starting from a blend - walk from this node and find all nodes we reach.
		TArray<UMorphNodeBase*> Nodes;
		StartWeight->GetNodes(Nodes);

		// If our test destination is in that set, return true.
		return Nodes.ContainsItem(TestDest);
	}
	// If start is not a blend - no way we can reach anywhere.
	else
	{
		return false;
	}
}

void WxAnimTreeEditor::MakeConnectionToConnector( FLinkedObjectConnector& Connector )
{
	// Avoid connections to yourself.
	if(!Connector.ConnObj || !ConnObj || Connector.ConnObj == ConnObj)
	{
		return;
	}

	UAnimNode* EndConnNode = Cast<UAnimNode>(Connector.ConnObj);
	USkelControlBase* EndConnControl = Cast<USkelControlBase>(Connector.ConnObj);
	UMorphNodeBase* EndConnMorph = Cast<UMorphNodeBase>(Connector.ConnObj);
	check(EndConnNode || EndConnControl || EndConnMorph);

	UAnimNode* ConnNode = Cast<UAnimNode>(ConnObj);
	USkelControlBase* ConnControl = Cast<USkelControlBase>(ConnObj);
	UMorphNodeBase* ConnMorph = Cast<UMorphNodeBase>(ConnObj);
	check(ConnNode || ConnControl || ConnMorph);

	// Control to...
	if(ConnControl)
	{
		// Control to control
		if(EndConnControl)
		{
			check(ConnIndex == 0 && Connector.ConnIndex == 0);

			// Determine which is the 'parent', which contains references to the child.
			if(ConnType == LOC_INPUT && Connector.ConnType == LOC_OUTPUT)
			{
				EndConnControl->NextControl = ConnControl;
			}
			else if(ConnType == LOC_OUTPUT && Connector.ConnType == LOC_INPUT)
			{
				ConnControl->NextControl = EndConnControl;
			}
		}
		// Control to Node. Node must be AnimTree. Make sure we are not connecting to Anim or Morph input.
		else if(EndConnNode && Connector.ConnIndex > 1)
		{
			check( ConnIndex == 0 );

			UAnimTree* Tree = Cast<UAnimTree>(EndConnNode);
			if(Tree)
			{
				Tree->SkelControlLists(Connector.ConnIndex-2).ControlHead = ConnControl;
			}
		}
	}
	// Morph to...
	else if(ConnMorph)
	{
		// Morph to Morph
		if(EndConnMorph)
		{
			// Determine which is the 'parent', to which you want to set a references to the child.
			UMorphNodeWeightBase* ParentNode = NULL;
			UMorphNodeBase* ChildNode = NULL;
			INT ChildIndex = INDEX_NONE;

			if(ConnType == LOC_INPUT && Connector.ConnType == LOC_OUTPUT)
			{
				ParentNode = CastChecked<UMorphNodeWeightBase>(EndConnMorph); // Only weight nodes can have LOC_OUTPUT connectors
				ChildIndex = Connector.ConnIndex;
				ChildNode = ConnMorph;
			}
			else if(ConnType == LOC_OUTPUT && Connector.ConnType == LOC_INPUT)
			{
				ParentNode = CastChecked<UMorphNodeWeightBase>(ConnMorph);
				ChildIndex = ConnIndex;
				ChildNode = EndConnMorph;
			}

			if(ParentNode)
			{
				// See if there is already a route from the child to the parent. If so - disallow connection.
				UBOOL bReachable = CheckMorphNodeReachability(ChildNode, ParentNode);
				if(bReachable)
				{
					appMsgf(AMT_OK, *LocalizeUnrealEd("Error_AnimTreeLoopDetected") );
				}
				else
				{
					check(ChildIndex < ParentNode->NodeConns.Num());
					ParentNode->NodeConns(ChildIndex).ChildNodes.AddUniqueItem(ChildNode);
				}
			}
		}
		// Morph to anim node. Node must be AnimTree.
		else if(EndConnNode)
		{
			check( ConnIndex == 0 );

			UAnimTree* Tree = Cast<UAnimTree>(EndConnNode);
			if(Tree)
			{
				Tree->RootMorphNodes.AddUniqueItem(ConnMorph);
			}
		}
	}
	// AnimNode to...
	else if(ConnNode)
	{
		// AnimNode to AnimNode
		if(EndConnNode)
		{
			// Determine which is the 'parent', to which you want to set a references to the child.
			UAnimNodeBlendBase* ParentNode = NULL;
			UAnimNode* ChildNode = NULL;
			INT ChildIndex = INDEX_NONE;

			if(ConnType == LOC_INPUT && Connector.ConnType == LOC_OUTPUT)
			{
				ParentNode = CastChecked<UAnimNodeBlendBase>(EndConnNode); // Only blend nodes can have LOC_OUTPUT connectors
				ChildIndex = Connector.ConnIndex;
				ChildNode = ConnNode;
			}
			else if(ConnType == LOC_OUTPUT && Connector.ConnType == LOC_INPUT)
			{
				ParentNode = CastChecked<UAnimNodeBlendBase>(ConnNode);
				ChildIndex = ConnIndex;
				ChildNode = EndConnNode;
			}

			if(ParentNode)
			{
				// See if there is already a route from the child to the parent. If so - disallow connection.
				UBOOL bReachable = CheckAnimNodeReachability(ChildNode, ParentNode);
				if(bReachable)
				{
					appMsgf(AMT_OK, *LocalizeUnrealEd("Error_AnimTreeLoopDetected") );
				}
				else
				{
					ParentNode->Children(ChildIndex).Anim = ChildNode;
				}
			}
		}
		// AnimNode to Control. AnimNode must be AnimTree.
		else if(EndConnControl)
		{
			check(Connector.ConnIndex == 0);

			UAnimTree* Tree = Cast<UAnimTree>(ConnNode);
			if(Tree && ConnIndex > 1)
			{
				Tree->SkelControlLists(ConnIndex-2).ControlHead = EndConnControl;
			}
		}
		// AnimNode to Morph. AnimNode must be AnimTree.
		else if(EndConnMorph)
		{
			check(Connector.ConnIndex == 0);
			check(ConnIndex == 1);

			UAnimTree* Tree = Cast<UAnimTree>(ConnNode);
			if(Tree)
			{
				Tree->RootMorphNodes.AddUniqueItem(EndConnMorph);
			}
		}
	}

	// Reinitialise the animation tree.
	ReInitAnimTree();

	// Mark the AnimTree's package as dirty.
	AnimTree->MarkPackageDirty();
}

void WxAnimTreeEditor::MakeConnectionToObject( UObject* EndObj )
{

}

/**
 * Called when the user releases the mouse over a link connector and is holding the ALT key.
 * Commonly used as a shortcut to breaking connections.
 *
 * @param	Connector	The connector that was ALT+clicked upon.
 */
void WxAnimTreeEditor::AltClickConnector(FLinkedObjectConnector& Connector)
{
	wxCommandEvent DummyEvent;
	OnBreakLink( DummyEvent );
}

void WxAnimTreeEditor::MoveSelectedObjects( INT DeltaX, INT DeltaY )
{
	for(INT i=0; i<SelectedNodes.Num(); i++)
	{
		UAnimNode* Node = SelectedNodes(i);
		
		Node->NodePosX += DeltaX;
		Node->NodePosY += DeltaY;
	}

	for(INT i=0; i<SelectedControls.Num(); i++)
	{
		USkelControlBase* Control = SelectedControls(i);

		Control->ControlPosX += DeltaX;
		Control->ControlPosY += DeltaY;
	}

	for(INT i=0; i<SelectedMorphNodes.Num(); i++)
	{
		UMorphNodeBase* MorphNode = SelectedMorphNodes(i);

		MorphNode->NodePosX += DeltaX;
		MorphNode->NodePosY += DeltaY;
	}

	// Mark the AnimTree's package as dirty.
	AnimTree->MarkPackageDirty();
}

void WxAnimTreeEditor::EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event)
{
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);

	if(Event == IE_Pressed)
	{
		if(Key == KEY_Delete)
		{
			DeleteSelectedObjects();
		}
		else if(Key == KEY_W && bCtrlDown)
		{
			DuplicateSelectedObjects();
		}
		else if(Key == KEY_SpaceBar)
		{
			wxCommandEvent DummyEvent;
			OnPreviewSelectedNode( DummyEvent );
		}
	}
}

/** The little button for doing automatic blending in/out of controls uses special index 1, so we look for it being clicked here. */
UBOOL WxAnimTreeEditor::SpecialClick( INT NewX, INT NewY, INT SpecialIndex, FViewport* Viewport )
{
	UBOOL bResult = 0;

	// Handle clicking on slider region jumping straight to that value.
	if(SpecialIndex == 0)
	{
		SpecialDrag(0, 0, NewX, NewY, SpecialIndex);
	}
	// Handle clicking on control button
	else if( SpecialIndex == 1 )
	{
		if( SelectedNodes.Num() == 0 )
		{
			UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);

			// If holding control (for multiple controls) or only have one selected control - toggle activate
			if( bCtrlDown || SelectedControls.Num() == 1 )
			{
				for( INT Idx = 0; Idx < SelectedControls.Num(); Idx++ )
				{
					USkelControlBase* Control = SelectedControls(Idx);

					// We basically use the mid-point as the deciding line between blending up or blending down when we press the button.
					if( Control->ControlStrength > 0.5f )
					{
						Control->SetSkelControlActive( FALSE );
					}
					else
					{
						Control->SetSkelControlActive( TRUE );
					}

					bResult = 1;
				}
			}
		}
	}

	return bResult;
}

/** Handler for dragging a 'special' item in the LinkedObjEditor. Index 0 is for slider controls, and thats what we are handling here. */
void WxAnimTreeEditor::SpecialDrag( INT DeltaX, INT DeltaY, INT NewX, INT NewY, INT SpecialIndex )
{
	// Only works if only 1 thing is selected.
	if(GetNumSelected() != 1)
	{
		return;
	}

	// Anim node slider
	if( SelectedNodes.Num() == 1 )
	{
		UAnimNode* Node = SelectedNodes(0);

		INT SliderWidth, SliderHeight;

		if(ST_2D == Node->GetSliderType(SpecialIndex))
		{
			SliderWidth = LO_SLIDER_HANDLE_HEIGHT;
			SliderHeight = LO_SLIDER_HANDLE_HEIGHT;
		}
		else
		{
			check(ST_1D == Node->GetSliderType(SpecialIndex));
			SliderWidth = LO_SLIDER_HANDLE_WIDTH;
			SliderHeight = LO_SLIDER_HANDLE_HEIGHT;
		}

		INT SliderRangeX = (Node->DrawWidth - 4 - SliderWidth);
		INT SlideStartX = Node->NodePosX + (2 + (0.5f * SliderWidth));
		INT SlideEndX = Node->NodePosX + Node->DrawWidth - (2 + (0.5f * SliderWidth));

		FLOAT HandleValX = (FLOAT)(NewX - SlideStartX)/(FLOAT)SliderRangeX;

		Node->HandleSliderMove(SpecialIndex, 0, ::Clamp(HandleValX, 0.f, 1.f) );

		if(ST_2D == Node->GetSliderType(SpecialIndex))
		{
			INT SliderPosY = Node->NodePosY + Node->DrawHeight;

			// compute slider starting position
			for(INT i = 0; i < SpecialIndex; ++i)
			{
				if(ST_1D == Node->GetSliderType(i))
				{
					SliderPosY += FLinkedObjDrawUtils::ComputeSliderHeight(Node->DrawWidth);
				}
				else
				{
					check(ST_2D == Node->GetSliderType(i));
					SliderPosY += FLinkedObjDrawUtils::Compute2DSliderHeight(Node->DrawWidth);
				}
			}

			INT TotalSliderHeight = FLinkedObjDrawUtils::Compute2DSliderHeight(Node->DrawWidth);
			INT SliderRangeY = (TotalSliderHeight - 4 - SliderHeight);
			INT SlideStartY = SliderPosY + (2 + (0.5f * SliderHeight));
			INT SlideEndY = SliderPosY + TotalSliderHeight - (2 + (0.5f * SliderHeight));

			FLOAT HandleValY = (FLOAT)(NewY - SlideStartY)/(FLOAT)SliderRangeY;

			Node->HandleSliderMove(SpecialIndex, 1, ::Clamp(HandleValY, 0.f, 1.f) );
		}
	}
	// SkelControl slider
	else if( SelectedControls.Num() == 1 )
	{
		if( SpecialIndex == 0 )
		{
			USkelControlBase* Control = SelectedControls(0);

			INT SliderRange = (Control->DrawWidth - 4 - LO_SLIDER_HANDLE_WIDTH - 15);
			INT SlideStart = Control->ControlPosX + (2 + (0.5f * LO_SLIDER_HANDLE_WIDTH));
			INT SlideEnd = Control->ControlPosX + Control->DrawWidth - (2 + (0.5f * LO_SLIDER_HANDLE_WIDTH)) - 15;
			FLOAT HandleVal = (FLOAT)(NewX - SlideStart)/(FLOAT)SliderRange;
			Control->HandleControlSliderMove( ::Clamp(HandleVal, 0.f, 1.f) );
		}
	}
	// MorphNode slider
	else if( SelectedMorphNodes.Num() == 1 )
	{
		if( SpecialIndex == 0 )
		{
			UMorphNodeBase* MorphNode = SelectedMorphNodes(0);

			INT SliderRange = (MorphNode->DrawWidth - 4 - LO_SLIDER_HANDLE_WIDTH);
			INT SlideStart = MorphNode->NodePosX + (2 + (0.5f * LO_SLIDER_HANDLE_WIDTH));
			INT SlideEnd = MorphNode->NodePosX + MorphNode->DrawWidth - (2 + (0.5f * LO_SLIDER_HANDLE_WIDTH));
			FLOAT HandleVal = (FLOAT)(NewX - SlideStart)/(FLOAT)SliderRange;
			MorphNode->HandleSliderMove( ::Clamp(HandleVal, 0.f, 1.f) );
		}
	}
}


/*-----------------------------------------------------------------------------
	WxAnimTreeEditorToolBar.
-----------------------------------------------------------------------------*/

class WxAnimTreeEditorToolBar : public wxToolBar
{
public:
	WxAnimTreeEditorToolBar( wxWindow* InParent, wxWindowID InID );

private:
	WxMaskedBitmap TickTreeB;
	WxMaskedBitmap PreviewNodeB;
	WxMaskedBitmap ShowNodeWeightB;
	WxMaskedBitmap ShowBonesB;
	WxMaskedBitmap ShowBoneNamesB;
	WxMaskedBitmap ShowWireframeB;
	WxMaskedBitmap ShowFloorB;
	WxMaskedBitmap CurvesB;
};

WxAnimTreeEditorToolBar::WxAnimTreeEditorToolBar( wxWindow* InParent, wxWindowID InID )
	:	wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	// create the return to parent sequence button
	TickTreeB.Load(TEXT("AnimTree_TickTree"));
	PreviewNodeB.Load(TEXT("AnimTree_PrevNode"));
	ShowNodeWeightB.Load(TEXT("AnimTree_ShowNodeWeight"));
	ShowBonesB.Load(TEXT("AnimTree_ShowBones"));
	ShowBoneNamesB.Load(TEXT("AnimTree_ShowBoneNames"));
	ShowWireframeB.Load(TEXT("AnimTree_ShowWireframe"));
	ShowFloorB.Load(TEXT("AnimTree_ShowFloor"));
	CurvesB.Load(TEXT("KIS_DrawCurves"));

	SetToolBitmapSize( wxSize( 16, 16 ) );

	AddSeparator();
	AddCheckTool(IDM_ANIMTREE_TOGGLETICKTREE, *LocalizeUnrealEd("PauseAnimTree"), TickTreeB, wxNullBitmap, *LocalizeUnrealEd("PauseAnimTree"));
	AddSeparator();
	AddTool(IDM_ANIMTREE_PREVIEWSELECTEDNODE, PreviewNodeB, *LocalizeUnrealEd("PreviewSelectedNode"));
	AddCheckTool(IDM_ANIMTREE_SHOWNODEWEIGHT, *LocalizeUnrealEd("ShowNodeWeight"), ShowNodeWeightB, wxNullBitmap, *LocalizeUnrealEd("ShowNodeWeight"));
	AddSeparator();
	AddCheckTool(IDM_ANIMTREE_SHOWHIERARCHY, *LocalizeUnrealEd("ShowSkeleton"), ShowBonesB, wxNullBitmap, *LocalizeUnrealEd("ShowSkeleton"));
	AddCheckTool(IDM_ANIMTREE_SHOWBONENAMES, *LocalizeUnrealEd("ShowBoneNames"), ShowBoneNamesB, wxNullBitmap, *LocalizeUnrealEd("ShowBoneNames"));
	AddCheckTool(IDM_ANIMTREE_SHOWWIREFRAME, *LocalizeUnrealEd("ShowWireframe"), ShowWireframeB, wxNullBitmap, *LocalizeUnrealEd("ShowWireframe"));
	AddCheckTool(IDM_ANIMTREE_SHOWFLOOR, *LocalizeUnrealEd("ShowFloor"), ShowFloorB, wxNullBitmap, *LocalizeUnrealEd("ShowFloor"));
	AddCheckTool(IDM_ANIMTREE_SHOWCURVES, *LocalizeUnrealEd("ToggleCurvedConnections"), CurvesB, wxNullBitmap, *LocalizeUnrealEd("ToggleCurvedConnections"));

	Realize();
}

/*-----------------------------------------------------------------------------
	WxAnimTreeEditor
-----------------------------------------------------------------------------*/

/** return how many times the class was derived */
static INT CountDerivations(const UClass* cl)
{
	INT derivations = 0;
	while (cl != NULL)
	{
		++derivations;
		cl = cl->GetSuperClass();
	}
	return derivations;
}

/** compare two UAnimNodeEditInfo by how often their AnimNodeClass is derived (most derived will be in front) */
IMPLEMENT_COMPARE_POINTER(UAnimNodeEditInfo, AnimTreeEditor, \
{ \
	const INT derivationsA = CountDerivations(A->AnimNodeClass); \
	const INT derivationsB = CountDerivations(B->AnimNodeClass); \
	return (derivationsA < derivationsB) ? +1 \
	: ((derivationsA > derivationsB) ? -1 : 0); \
} )


/** this function will sort the given array of anim node edit infos by the derivation of their AnimNodeClass, so that most derived will be in front */
void SortAnimNodeEditInfosByDerivation(TArray<class UAnimNodeEditInfo*>& AnimNodeEditInfos)
{
	Sort<USE_COMPARE_POINTER(UAnimNodeEditInfo,
		AnimTreeEditor)>( AnimNodeEditInfos.GetTypedData(),
		AnimNodeEditInfos.Num() );
}


UBOOL				WxAnimTreeEditor::bAnimClassesInitialized = false;
TArray<UClass*>		WxAnimTreeEditor::AnimNodeClasses;
TArray<UClass*>		WxAnimTreeEditor::SkelControlClasses;
TArray<UClass*>		WxAnimTreeEditor::MorphNodeClasses;

BEGIN_EVENT_TABLE( WxAnimTreeEditor, WxTrackableFrame )
	EVT_CLOSE( WxAnimTreeEditor::OnClose )
	EVT_MENU_RANGE( IDM_ANIMTREE_NEW_NODE_START, IDM_ANIMTREE_NEW_NODE_END, WxAnimTreeEditor::OnNewAnimNode )
	EVT_MENU_RANGE( IDM_ANIMTREE_NEW_CONTROL_START, IDM_ANIMTREE_NEW_CONTROL_END, WxAnimTreeEditor::OnNewSkelControl )
	EVT_MENU_RANGE( IDM_ANIMTREE_NEW_MORPH_START, IDM_ANIMTREE_NEW_MORPH_END, WxAnimTreeEditor::OnNewMorphNode )
	EVT_MENU( IDM_ANIMTREE_DELETE_NODE, WxAnimTreeEditor::OnDeleteObjects )
	EVT_MENU( IDM_ANIMTREE_BREAK_LINK, WxAnimTreeEditor::OnBreakLink )
	EVT_MENU( IDM_ANIMTREE_BREAK_ALL_LINKS, WxAnimTreeEditor::OnBreakAllLinks )
	EVT_MENU( IDM_ANIMTREE_ADD_INPUT, WxAnimTreeEditor::OnAddInput )
	EVT_MENU( IDM_ANIMTREE_DELETE_INPUT, WxAnimTreeEditor::OnRemoveInput )
	EVT_MENU( IDM_ANIMTREE_NAME_INPUT, WxAnimTreeEditor::OnNameInput )
	EVT_MENU( IDM_ANIMTREE_ADD_CONTROLHEAD, WxAnimTreeEditor::OnAddControlHead )
	EVT_MENU( IDM_ANIMTREE_DELETE_CONTROLHEAD, WxAnimTreeEditor::OnRemoveControlHead )
	EVT_MENU( IDM_ANIMTREE_CHANGEBONE_CONTROLHEAD,WxAnimTreeEditor::OnChangeBoneControlHead )
	EVT_TOOL( IDM_ANIMTREE_TOGGLETICKTREE, WxAnimTreeEditor::OnToggleTickAnimTree )
	EVT_TOOL( IDM_ANIMTREE_PREVIEWSELECTEDNODE, WxAnimTreeEditor::OnPreviewSelectedNode )
	EVT_TOOL( IDM_ANIMTREE_SHOWNODEWEIGHT, WxAnimTreeEditor::OnShowNodeWeights )
	EVT_TOOL( IDM_ANIMTREE_SHOWHIERARCHY, WxAnimTreeEditor::OnShowSkeleton )
	EVT_TOOL( IDM_ANIMTREE_SHOWBONENAMES, WxAnimTreeEditor::OnShowBoneNames )
	EVT_TOOL( IDM_ANIMTREE_SHOWWIREFRAME, WxAnimTreeEditor::OnShowWireframe )
	EVT_TOOL( IDM_ANIMTREE_SHOWFLOOR, WxAnimTreeEditor::OnShowFloor )
	EVT_TOOL( IDM_ANIMTREE_SHOWCURVES, WxAnimTreeEditor::OnButtonShowCurves)
END_EVENT_TABLE()

IMPLEMENT_COMPARE_POINTER( UClass, AnimTreeEditor, { return appStricmp( *A->GetName(), *B->GetName() ); } )

// Static functions that fills in array of all available USoundNode classes and sorts them alphabetically.
void WxAnimTreeEditor::InitAnimClasses()
{
	if(bAnimClassesInitialized)
		return;

	// Construct list of non-abstract gameplay sequence object classes.
	for(TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;

		if( !(Class->ClassFlags & CLASS_Abstract) && !(Class->ClassFlags & CLASS_Hidden) && !(Class->ClassFlags & CLASS_Deprecated) )
		{
			if( (Class->IsChildOf(UAnimNode::StaticClass())) && !(Class->IsChildOf(UAnimTree::StaticClass())) )
			{
				AnimNodeClasses.AddItem(Class);
			}
			else if( Class->IsChildOf(USkelControlBase::StaticClass()) )
			{
				SkelControlClasses.AddItem(Class);
			}
			else if( Class->IsChildOf(UMorphNodeBase::StaticClass()) )
			{
				MorphNodeClasses.AddItem(Class);
			}
		}
	}

	Sort<USE_COMPARE_POINTER(UClass,AnimTreeEditor)>( &AnimNodeClasses(0), AnimNodeClasses.Num() );

	bAnimClassesInitialized = true;
}


WxAnimTreeEditor::WxAnimTreeEditor( wxWindow* InParent, wxWindowID InID, class UAnimTree* InAnimTree )
: WxTrackableFrame( InParent, InID, TEXT(""), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR ),
  FDockingParent(this)
{
	bShowSkeleton = FALSE;
	bShowBoneNames = FALSE;
	bShowWireframe = FALSE;
	bShowFloor = FALSE;
	bDrawCurves = TRUE;
	bShowNodeWeights = FALSE;
	bTickAnimTree = TRUE;
	bEditorClosing = FALSE;

	AnimTree = InAnimTree;
	AnimTree->bBeingEdited = TRUE;

	// Set the anim tree editor window title to include the anim tree being edited.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("AnimTreeEditor_F"), *AnimTree->GetPathName()) ) );

	// Initialise list of AnimNode classes.
	InitAnimClasses();

	SetSize(1024, 768);
	// Load the desired window position from .ini file
	FWindowUtil::LoadPosSize(TEXT("AnimTreeEditor"), this, 256, 256, 1024, 768);

	// Create property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );

	// Create linked-object tree window
	WxLinkedObjVCHolder* TreeWin = new WxLinkedObjVCHolder( this, -1, this );
	LinkedObjVC = TreeWin->LinkedObjVC;

	// Create 3D preview window
	WxAnimTreePreview* PreviewWin = new WxAnimTreePreview( this, -1, this, AnimTree->PreviewCamPos, AnimTree->PreviewCamRot );
	PreviewVC = PreviewWin->AnimTreePreviewVC;

	// Add docking windows.
	{
		AddDockingWindow( PreviewWin, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PreviewCaption_F"), *AnimTree->GetPathName())), *LocalizeUnrealEd("Preview") );
		AddDockingWindow( PropertyWindow, FDockingParent::DH_Bottom, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PropertiesCaption_F"), *AnimTree->GetPathName())), *LocalizeUnrealEd("Properties") );
		AddDockingWindow( TreeWin, FDockingParent::DH_None, NULL );

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();
	}

	wxMenuBar* MenuBar = new wxMenuBar;
	AppendWindowMenu(MenuBar);
	SetMenuBar(MenuBar);

	ToolBar = new WxAnimTreeEditorToolBar( this, -1 );
	SetToolBar(ToolBar);

	// Shift origin so origin is roughly in the middle. Would be nice to use Viewport size, but doesnt seem initialised here...
	LinkedObjVC->Origin2D.X = 150;
	LinkedObjVC->Origin2D.Y = 300;

	BackgroundTexture = LoadObject<UTexture2D>(NULL, TEXT("EditorMaterials.AnimTreeBackGround"), NULL, LOAD_None, NULL);

	// Build list of all AnimNodes in the tree.
	AnimTree->GetNodes(TreeNodes);
	AnimTree->GetSkelControls(TreeControls);
	AnimTree->GetMorphNodes(TreeMorphNodes);

	// Check for nodes that have been deprecated. Warn user if any has been found!
	{
		UBOOL bFoundDeprecatedNode = FALSE;

		for(INT i=0; i<TreeNodes.Num(); i++)
		{
			if( TreeNodes(i)->GetClass()->ClassFlags & CLASS_Deprecated )
			{
				bFoundDeprecatedNode = TRUE;
				break;
			}
		}

		if( !bFoundDeprecatedNode )
		{
			for(INT i=0; i<TreeControls.Num(); i++)
			{
				if( TreeControls(i)->GetClass()->ClassFlags & CLASS_Deprecated )
				{
					bFoundDeprecatedNode = TRUE;
					break;
				}
			}
		}

		if( !bFoundDeprecatedNode )
		{
			for(INT i=0; i<TreeMorphNodes.Num(); i++)
			{
				if( TreeMorphNodes(i)->GetClass()->ClassFlags & CLASS_Deprecated )
				{
					bFoundDeprecatedNode = TRUE;
					break;
				}
			}
		}

		// If we've found a deprecated node, warn the user that he should be careful when saving the tree
		if( bFoundDeprecatedNode )
		{
			appMsgf(AMT_OK, TEXT("This AnimTree contains DEPRECATED nodes.\nSaving the Tree will set these references to NULL!!\nMake sure you update or remove these nodes before saving.\nThey are shown with a bright RED background."));
		}
	}

	// Initialise the preview SkeletalMeshComponent.
	PreviewSkelComp->Animations = AnimTree;
	PreviewSkelComp->AnimSets = AnimTree->PreviewAnimSets;
	PreviewSkelComp->MorphSets = AnimTree->PreviewMorphSets;
	PreviewSkelComp->SetSkeletalMesh(AnimTree->PreviewSkelMesh);

	// Instance one of each AnimNodeEditInfos. Allows AnimNodes to have specific editor functionality.
	for(TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if( !(Class->ClassFlags & CLASS_Abstract) && !(Class->ClassFlags & CLASS_Hidden) )
		{
			if( Class->IsChildOf(UAnimNodeEditInfo::StaticClass()) )
			{
				UAnimNodeEditInfo* NewEditInfo = ConstructObject<UAnimNodeEditInfo>(Class);
				AnimNodeEditInfos.AddItem(NewEditInfo);
			}
		}
	}

	SortAnimNodeEditInfosByDerivation(AnimNodeEditInfos);

	// make sure SocketComponent is not initialized with garbage.
	SocketComponent = NULL;
	RecreateSocketPreview();
}

WxAnimTreeEditor::~WxAnimTreeEditor()
{
	FWindowUtil::SavePosSize(TEXT("AnimTreeEditor"), this);
	
	SaveDockingLayout();

	// Do this here as well, as OnClose doesn't get called when closing the editor.
	PreviewSkelComp->Animations = NULL;
	for(INT i=0; i<TreeNodes.Num(); i++)
	{
		TreeNodes(i)->SkelComponent = NULL;
	}

	for(INT i=0; i<TreeMorphNodes.Num(); i++)
	{
		TreeMorphNodes(i)->SkelComponent = NULL;
	}
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxAnimTreeEditor::OnSelected()
{
	Raise();
}

void WxAnimTreeEditor::OnClose( wxCloseEvent& In )
{
	AnimTree->bBeingEdited = false;

	// Call OnCloseAnimTreeEditor on all EditInfos
	for(INT i=0; i<AnimNodeEditInfos.Num(); i++)
	{
		AnimNodeEditInfos(i)->OnCloseAnimTreeEditor();
	}

	// Detach AnimTree from preview Component so it doesn't get deleted when preview SkeletalMeshComponent does, 
	// and there are no pointers to component we are about to destroy.
	PreviewSkelComp->Animations = NULL;

	for(INT i=0; i<TreeNodes.Num(); i++)
	{
		TreeNodes(i)->SkelComponent = NULL;
	}

	for(INT i=0; i<TreeMorphNodes.Num(); i++)
	{
		TreeMorphNodes(i)->SkelComponent = NULL;
	}

	// Remember the preview viewport camera position/rotation.
	AnimTree->PreviewCamPos = PreviewVC->ViewLocation;
	AnimTree->PreviewCamRot = PreviewVC->ViewRotation;

	// Remember the floor position/rotation
	AnimTree->PreviewFloorPos = FloorComp->LocalToWorld.GetOrigin();
	FRotator FloorRot = FloorComp->LocalToWorld.Rotator();
	AnimTree->PreviewFloorYaw = FloorRot.Yaw;

	ClearSocketPreview();

	bEditorClosing = true;

	this->Destroy();
}

void WxAnimTreeEditor::Serialize(FArchive& Ar)
{
	PreviewVC->Serialize(Ar);

	// Make sure we don't garbage collect nodes/control that are not currently linked into the tree.
	if(!Ar.IsLoading() && !Ar.IsSaving())
	{
		for(INT i=0; i<TreeNodes.Num(); i++)
		{
			Ar << TreeNodes(i);
		}

		for(INT i=0; i<TreeControls.Num(); i++)
		{
			Ar << TreeControls(i);
		}

		for(INT i=0; i<TreeMorphNodes.Num(); i++)
		{
			Ar << TreeMorphNodes(i);
		}

		for(INT i=0; i<AnimNodeEditInfos.Num(); i++)
		{
			Ar << AnimNodeEditInfos(i);
		}
	}
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxAnimTreeEditor::GetDockingParentName() const
{
	return TEXT("AnimTreeEditor");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxAnimTreeEditor::GetDockingParentVersion() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////
// Menu Handlers
//////////////////////////////////////////////////////////////////

void WxAnimTreeEditor::OnNewAnimNode(wxCommandEvent& In)
{
	INT NewNodeClassIndex = In.GetId() - IDM_ANIMTREE_NEW_NODE_START;
	check( NewNodeClassIndex >= 0 && NewNodeClassIndex < AnimNodeClasses.Num() );

	UClass* NewNodeClass = AnimNodeClasses(NewNodeClassIndex);
	check( NewNodeClass->IsChildOf(UAnimNode::StaticClass()) );

	UAnimNode* NewNode = ConstructObject<UAnimNode>( NewNodeClass, AnimTree, NAME_None);

	NewNode->NodePosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
	NewNode->NodePosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

	TreeNodes.AddItem(NewNode);

	RefreshViewport();
}

void WxAnimTreeEditor::OnNewSkelControl(wxCommandEvent& In)
{
	INT NewControlClassIndex = In.GetId() - IDM_ANIMTREE_NEW_CONTROL_START;
	check( NewControlClassIndex >= 0 && NewControlClassIndex < SkelControlClasses.Num() );

	UClass* NewControlClass = SkelControlClasses(NewControlClassIndex);
	check( NewControlClass->IsChildOf(USkelControlBase::StaticClass()) );

	USkelControlBase* NewControl = ConstructObject<USkelControlBase>( NewControlClass, AnimTree, NAME_None);
	NewControl->ControlPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
	NewControl->ControlPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

	TreeControls.AddItem(NewControl);

	RefreshViewport();
}

void WxAnimTreeEditor::OnNewMorphNode( wxCommandEvent& In )
{
	INT NewNodeClassIndex = In.GetId() - IDM_ANIMTREE_NEW_MORPH_START;
	check( NewNodeClassIndex >= 0 && NewNodeClassIndex < MorphNodeClasses.Num() );

	UClass* NewNodeClass = MorphNodeClasses(NewNodeClassIndex);
	check( NewNodeClass->IsChildOf(UMorphNodeBase::StaticClass()) );

	UMorphNodeBase* NewNode = ConstructObject<UMorphNodeBase>( NewNodeClass, AnimTree, NAME_None);

	NewNode->NodePosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
	NewNode->NodePosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

	TreeMorphNodes.AddItem(NewNode);

	RefreshViewport();
}


void WxAnimTreeEditor::OnBreakLink(wxCommandEvent& In)
{
	UAnimTree* Tree = Cast<UAnimTree>(ConnObj);
	UAnimNodeBlendBase* BlendNode = Cast<UAnimNodeBlendBase>(ConnObj);
	USkelControlBase* SkelControl = Cast<USkelControlBase>(ConnObj);
	UMorphNodeWeightBase* MorphNode = Cast<UMorphNodeWeightBase>(ConnObj);

	if( ConnType == LOC_OUTPUT )
	{
		if(Tree)
		{
			if(ConnIndex == 0)
			{
				Tree->Children(0).Anim = NULL;
			}
			else if(ConnIndex == 1)
			{
				// @todo - Let you select WHICH connection to break
				Tree->RootMorphNodes.Empty();
			}
			else
			{
				Tree->SkelControlLists(ConnIndex-2).ControlHead = NULL;
			}
		}
		else if(BlendNode)
		{		
			BlendNode->Children(ConnIndex).Anim = NULL;
		}
		else if(SkelControl)
		{
			SkelControl->NextControl = NULL;
		}
		else if(MorphNode)
		{
			// @todo - Let you select WHICH connection to break
			MorphNode->NodeConns(ConnIndex).ChildNodes.Empty();
		}
	}

	ReInitAnimTree();
	RefreshViewport();

	AnimTree->MarkPackageDirty();
}

/** Break all links going to or from the selected node(s). */
void WxAnimTreeEditor::OnBreakAllLinks(wxCommandEvent& In)
{
	if (appMsgf(AMT_YesNo, *LocalizeUnrealEd("ConfirmBreakAllLinks")))
	{
		for(INT i=0; i<SelectedNodes.Num(); i++)
		{
			UAnimTree* Tree = Cast<UAnimTree>(SelectedNodes(i));
			UAnimNode* Node = Cast<UAnimNodeBlendBase>(SelectedNodes(i));

			if(Tree)
			{
				Tree->Children(0).Anim = NULL;

				for(INT j=0; j<Tree->SkelControlLists.Num(); j++)
				{
					Tree->SkelControlLists(j).ControlHead = NULL;
				}
			}
			else if(Node)
			{		
				UAnimNodeBlendBase* BlendNode = Cast<UAnimNodeBlendBase>(Node);
				if(BlendNode)
				{
					for(INT j=0; j<BlendNode->Children.Num(); j++)
					{
						BlendNode->Children(j).Anim = NULL;
					}
				}

				BreakLinksToNode(Node);
			}
		}
	}
}

void WxAnimTreeEditor::OnAddInput(wxCommandEvent& In)
{
	const INT NumSelected = SelectedNodes.Num();
	if(NumSelected == 1)
	{
		UAnimNodeBlendBase* BlendNode = Cast<UAnimNodeBlendBase>(SelectedNodes(0));
		if(BlendNode && !BlendNode->bFixNumChildren)
		{
			const INT NewChildIndex = BlendNode->Children.AddZeroed();
			BlendNode->OnAddChild(NewChildIndex);
		}
	}

	ReInitAnimTree();
	RefreshViewport();
	UpdatePropertyWindow();

	AnimTree->MarkPackageDirty();
}

void WxAnimTreeEditor::OnRemoveInput(wxCommandEvent& In)
{
	if(!ConnObj || ConnType != LOC_OUTPUT)
		return;

	// Only blend nodes have outputs..
	UAnimNodeBlendBase* BlendNode = CastChecked<UAnimNodeBlendBase>(ConnObj);

	// Do nothing if not allowed to modify connectors
	if(BlendNode->bFixNumChildren)
		return;

	check(ConnIndex >= 0 || ConnIndex < BlendNode->Children.Num() );

	BlendNode->Children.Remove(ConnIndex);
	BlendNode->OnRemoveChild(ConnIndex);

	ReInitAnimTree();
	RefreshViewport();

	AnimTree->MarkPackageDirty();
}

void WxAnimTreeEditor::OnNameInput(wxCommandEvent& In)
{
	if( !ConnObj || ConnType != LOC_OUTPUT )
	{
		return;
	}

	// Only blend nodes have outputs...
	UAnimNodeBlendBase* BlendNode = CastChecked<UAnimNodeBlendBase>(ConnObj);
	check(ConnIndex >= 0 || ConnIndex < BlendNode->Children.Num() );

	// Prompt the user for the name of the input connector
	WxDlgGenericStringEntry dlg;
	if( dlg.ShowModal( *LocalizeUnrealEd("Rename"), *LocalizeUnrealEd("Rename"), TEXT("") ) == wxID_OK )
	{
		FString	newName = dlg.GetEnteredString();
		BlendNode->Children(ConnIndex).Name = FName( *newName, FNAME_Add );

		ReInitAnimTree();
		RefreshViewport();

		AnimTree->MarkPackageDirty();
	}
}

void WxAnimTreeEditor::OnAddControlHead(wxCommandEvent& In)
{
	// If we have only the AnimTree selected, and we have a preview mesh...
	if( SelectedNodes.Num() == 1 && 
		SelectedNodes(0) == AnimTree &&
		AnimTree->PreviewSkelMesh )
	{
		// Make list of all bone names in preview skeleton, and let user pick which one to attach controllers to.
		TArray<FString> BoneNames;
		for(INT i=0; i<AnimTree->PreviewSkelMesh->RefSkeleton.Num(); i++)
		{
			FName BoneName = AnimTree->PreviewSkelMesh->RefSkeleton(i).Name;

			// Check there is not already a chain for this bone...
			UBOOL bAlreadyHasChain = false;
			for(INT j=0; j<AnimTree->SkelControlLists.Num() && !bAlreadyHasChain; j++)
			{
				if(AnimTree->SkelControlLists(j).BoneName == BoneName)
				{
					bAlreadyHasChain = true;
				}
			}

			// If not - add to combo box.
			if(!bAlreadyHasChain)
			{
				BoneNames.AddZeroed();
				BoneNames(BoneNames.Num()-1) = BoneName.ToString();
			}
		}

		// Display dialog and let user pick which bone to create a SkelControl Chain for.
		WxDlgGenericComboEntry dlg;
		if( dlg.ShowModal( TEXT("NewSkelControlChain"), TEXT("BoneName"), BoneNames, 0, TRUE ) == wxID_OK )
		{
			FName BoneName = FName( *dlg.GetSelectedString() );
			if(BoneName != NAME_None)
			{
				FSkelControlListHead ListHead;
				ListHead.BoneName = BoneName;
				ListHead.ControlHead = NULL;

				AnimTree->SkelControlLists.AddItem(ListHead);

				// Re-init the SkelControls list.
				PreviewSkelComp->InitSkelControls();
				RefreshViewport();
				AnimTree->MarkPackageDirty();
			}
		}
	}
}

void WxAnimTreeEditor::OnRemoveControlHead(wxCommandEvent& In)
{
	if( ConnObj == AnimTree )
	{
		check(ConnIndex > 0);
		AnimTree->SkelControlLists.Remove( ConnIndex-2 );

		// Update control table.
		PreviewSkelComp->InitSkelControls();
		RefreshViewport();
		AnimTree->MarkPackageDirty();
	}
}

void WxAnimTreeEditor::OnChangeBoneControlHead(wxCommandEvent& In)
{
	// If we have a preview mesh...
	if( AnimTree->PreviewSkelMesh )
	{
		// Make list of all bone names in preview skeleton, and let user pick which
		// one to attach controllers to.
		TArray<FString> BoneNames;
		for(INT i=0; i<AnimTree->PreviewSkelMesh->RefSkeleton.Num(); i++)
		{
			FName BoneName = AnimTree->PreviewSkelMesh->RefSkeleton(i).Name;

			// Check there is not already a chain for this bone...
			UBOOL bAlreadyHasChain = FALSE;

			for(INT j=0; j<AnimTree->SkelControlLists.Num() && !bAlreadyHasChain; j++)
			{
				if( AnimTree->SkelControlLists(j).BoneName == BoneName )
				{
					bAlreadyHasChain = TRUE;
				}
			}

			// If not - add to combo box.
			if( !bAlreadyHasChain )
			{
				BoneNames.AddZeroed();
				BoneNames(BoneNames.Num()-1) = BoneName.ToString();
			}
		}

		// Display dialog and let user pick which bone shall replace the
		// current bone in the SkelControl Chain.
		WxDlgGenericComboEntry dlg;
		if( dlg.ShowModal( TEXT("NewSkelControlChain"), TEXT("BoneName"), BoneNames, 0, TRUE ) == wxID_OK )
		{
			FName BoneName = FName( *dlg.GetSelectedString() );
			if( BoneName != NAME_None )
			{
				check(ConnIndex > 0);
				FSkelControlListHead& ListHead = AnimTree->SkelControlLists(ConnIndex-2);
				ListHead.BoneName = BoneName;

				// Update control table.
				PreviewSkelComp->InitSkelControls();
				RefreshViewport();
				AnimTree->MarkPackageDirty();
			}
		}

	}
}

void WxAnimTreeEditor::OnDeleteObjects(wxCommandEvent& In)
{
	DeleteSelectedObjects();
}

void WxAnimTreeEditor::OnToggleTickAnimTree( wxCommandEvent& In )
{
	bTickAnimTree = !bTickAnimTree;
}

void WxAnimTreeEditor::OnPreviewSelectedNode( wxCommandEvent& In )
{
	if(SelectedNodes.Num() == 1)
	{
		SetPreviewNode( SelectedNodes(0) );
	}
}

/** Toggle showing total weights numerically above each  */
void WxAnimTreeEditor::OnShowNodeWeights( wxCommandEvent& In )
{
	bShowNodeWeights = !bShowNodeWeights;
}

/** Toggle drawing of skeleton. */
void WxAnimTreeEditor::OnShowSkeleton( wxCommandEvent& In )
{
	bShowSkeleton = !bShowSkeleton;
}

/** Toggle drawing of bone names. */
void WxAnimTreeEditor::OnShowBoneNames( wxCommandEvent& In )
{
	bShowBoneNames = !bShowBoneNames;
}

/** Toggle drawing skeletal mesh in wireframe. */
void WxAnimTreeEditor::OnShowWireframe( wxCommandEvent& In )
{
	bShowWireframe = !bShowWireframe;
}

/** Toggle drawing the floor (and allowing collision with it). */
void WxAnimTreeEditor::OnShowFloor( wxCommandEvent& In )
{
	bShowFloor = !bShowFloor;
}

/** Toggle drawing logic connections as straight lines or curves. */
void WxAnimTreeEditor::OnButtonShowCurves(wxCommandEvent &In)
{
	bDrawCurves = In.IsChecked();
	RefreshViewport();
}

//////////////////////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////////////////////

void WxAnimTreeEditor::RefreshViewport()
{
	LinkedObjVC->Viewport->Invalidate();
}

/** Clear all references from any node to the given UAnimNode. */
void WxAnimTreeEditor::BreakLinksToNode(UAnimNode* InNode)
{
	for(INT i=0; i<TreeNodes.Num(); i++)
	{
		UAnimNodeBlendBase* BlendNode = Cast<UAnimNodeBlendBase>(TreeNodes(i));
		if(BlendNode)
		{
			for(INT j=0; j<BlendNode->Children.Num(); j++)
			{
				if(BlendNode->Children(j).Anim == InNode)
				{
					BlendNode->Children(j).Anim = NULL;
				}
			}
		}
	}
}

/** Clear all references from any SkelControl (or the root AnimTree) to the given USkelControlBase. */
void WxAnimTreeEditor::BreakLinksToControl(USkelControlBase* InControl)
{
	// Clear any references in the AnimTree to this SkelControl.
	for( INT i=0; i<AnimTree->SkelControlLists.Num(); i++)
	{
		if(AnimTree->SkelControlLists(i).ControlHead == InControl)
		{
			AnimTree->SkelControlLists(i).ControlHead = NULL;
		}
	}

	// Clear any references to this SkelControl from other SkelControls.
	for(INT i=0; i<TreeControls.Num(); i++)
	{
		if(TreeControls(i)->NextControl == InControl)
		{
			TreeControls(i)->NextControl = NULL;
		}
	}
}

/** Clear all references from any MorphNode (or the root AnimTree) to the given UMorphNodeBase. */
void WxAnimTreeEditor::BreakLinksToMorphNode(UMorphNodeBase* InNode)
{
	// Clear any references in the AnimTree to this node.
	AnimTree->RootMorphNodes.RemoveItem(InNode);

	// Clear any references to this MorphNode from other MorphNode.
	for(INT i=0; i<TreeMorphNodes.Num(); i++)
	{
		// Only MorphNodeWeightBase classes keep references.
		UMorphNodeWeightBase* WeightNode = Cast<UMorphNodeWeightBase>( TreeMorphNodes(i) );
		if(WeightNode)
		{
			// Iterate over each connector
			for(INT j=0; j<WeightNode->NodeConns.Num(); j++)
			{
				// Remove from array connector struct.
				FMorphNodeConn& Conn = WeightNode->NodeConns(j);
				Conn.ChildNodes.RemoveItem(InNode);
			}
		}
	}
}

void WxAnimTreeEditor::DeleteSelectedObjects()
{
	// DElete selected AnimNodes
	for(INT i=0; i<SelectedNodes.Num(); i++)
	{
		UAnimNode* DelNode = SelectedNodes(i);

		// Don't allow AnimTree to be deleted.
		if(DelNode != AnimTree)
		{
			BreakLinksToNode(DelNode);

			// Clear reference to preview SkeletalMeshComponent.
			DelNode->SkelComponent = NULL;

			// Remove this node from the list of all Nodes (whether in the tree or not).
			check( TreeNodes.ContainsItem(DelNode) );
			TreeNodes.RemoveItem(DelNode);
		}
	}
	SelectedNodes.Empty();

	// Delete selected SkelControls
	for(INT i=0; i<SelectedControls.Num(); i++)
	{
		USkelControlBase* DelControl = SelectedControls(i);

		BreakLinksToControl(DelControl);

		// We just remove it from the array - don't actually delete object in editor (GC will take care of it when necessary).
		check( TreeControls.ContainsItem(DelControl) );
		TreeControls.RemoveItem(DelControl);
	}
	SelectedControls.Empty();

	// Delete selected MorphNodes
	for(INT i=0; i<SelectedMorphNodes.Num(); i++)
	{
		UMorphNodeBase* MorphNode = SelectedMorphNodes(i);
		BreakLinksToMorphNode(MorphNode);

		MorphNode->SkelComponent = NULL;

		check(TreeMorphNodes.ContainsItem(MorphNode));
		TreeMorphNodes.RemoveItem(MorphNode);
	}
	SelectedMorphNodes.Empty();

	UpdatePropertyWindow();
	ReInitAnimTree();
	RefreshViewport();
	AnimTree->MarkPackageDirty();
}

void WxAnimTreeEditor::DuplicateSelectedObjects()
{
	// Never allow you to duplicate the AnimTree itself!
	SelectedNodes.RemoveItem(AnimTree);

	// Duplicate the AnimNodes	
	TArray<UAnimNode*> DestNodes; // Array of newly created nodes.
	TMap<UAnimNode*,UAnimNode*> SrcToDestNodeMap; // Mapping table from src node to newly created node.
	UAnimTree::CopyAnimNodes(SelectedNodes, AnimTree, DestNodes, SrcToDestNodeMap);

	// Duplicate the SkelControls
	TArray<USkelControlBase*> DestControls; // Array of new skel controls.
	TMap<USkelControlBase*, USkelControlBase*> SrcToDestControlMap; // Map from src control to newly created one.
	UAnimTree::CopySkelControls(SelectedControls, AnimTree, DestControls, SrcToDestControlMap);

	// Duplicate the MorphNodes
	TArray<UMorphNodeBase*> DestMorphNodes; // Array of new morph nodes.
	TMap<UMorphNodeBase*, UMorphNodeBase*> SrcToDestMorphNodeMap; // Map from src node to newly created one.
	UAnimTree::CopyMorphNodes(SelectedMorphNodes, AnimTree, DestMorphNodes, SrcToDestMorphNodeMap);


	// We set the selection to being the newly created nodes.
	EmptySelection();

	// Offset the newly created nodes.
	for(INT i=0; i < DestNodes.Num(); i++)
	{
		UAnimNode* Node = DestNodes(i);
		Node->NodePosX += DuplicateOffset;
		Node->NodePosY += DuplicateOffset;

		TreeNodes.AddItem(Node);
		AddToSelection(Node);
	}


	// Offset the newly created controls.
	for(INT i=0; i < DestControls.Num(); i++)
	{
		USkelControlBase* Control = DestControls(i);
		Control->ControlPosX += DuplicateOffset;
		Control->ControlPosY += DuplicateOffset;
	
		TreeControls.AddItem(Control);
		AddToSelection(Control);
	}

	// Offset the newly created morph nodes.
	for(INT i=0; i < DestMorphNodes.Num(); i++)
	{
		UMorphNodeBase* MorphNode = DestMorphNodes(i);
		MorphNode->NodePosX += DuplicateOffset;
		MorphNode->NodePosY += DuplicateOffset;

		TreeMorphNodes.AddItem(MorphNode);
		AddToSelection(MorphNode);
	}
}

/** 
 *	Update the SkelComponent of all nodes. Will set to NULL any that are no longer in the actual tree.
 *	Also prevents you from viewing a node that is no longer in the tree.
 */
void WxAnimTreeEditor::ReInitAnimTree()
{
	PreviewSkelComp->InitAnimTree();

	// If we are previewing a node which is no longer in the tree, reset to root.
	if( PreviewSkelComp->Animations->SkelComponent == NULL )
	{
		PreviewSkelComp->Animations = AnimTree;
	}
}

UAnimNodeEditInfo* WxAnimTreeEditor::FindAnimNodeEditInfo(UAnimNode* InNode)
{
	for(INT i=0; i<AnimNodeEditInfos.Num(); i++)
	{
		UAnimNodeEditInfo* EditInfo = AnimNodeEditInfos(i);
		if( EditInfo->AnimNodeClass && InNode->IsA(EditInfo->AnimNodeClass) )
		{
			return EditInfo;
		}
	}

	return NULL;
}


/** Update anything (eg the skeletal mesh) in the preview window. */
void WxAnimTreeEditor::TickPreview(FLOAT DeltaSeconds)
{
	if( !bEditorClosing && PreviewSkelComp->IsAttached() )
	{
		// Take into account preview play rate
		const FLOAT UpdateDeltaSeconds = DeltaSeconds * Clamp<FLOAT>(AnimTree->PreviewPlayRate, 0.1f, 10.f);

		// Used to ensure nodes/controls are not ticked multiple times if there are multiple routes to one (ie they are shared).
		PreviewSkelComp->TickTag++;

		// Don't tick the anim tree if animation is paused
		if( bTickAnimTree )
		{
			PreviewSkelComp->TickAnimNodes(UpdateDeltaSeconds);
		}

		// update the instanced influence weights if needed
		if( PreviewSkelComp->bNeedsInstanceWeightUpdate )
		{
			PreviewSkelComp->UpdateInstanceVertexWeights();
		}

		// Do keep ticking controls though.
		PreviewSkelComp->TickSkelControls(UpdateDeltaSeconds);

		// Update the bones and skeletal mesh.
		PreviewSkelComp->UpdateSkelPose();
		PreviewSkelComp->ConditionalUpdateTransform();
		RefreshViewport();
	}
}


/** Change the node that we are previewing in the 3D window. */
void WxAnimTreeEditor::SetPreviewNode(class UAnimNode* NewPreviewNode)
{
	// First we want to check that this node is in the 'main' tree - connected to the AnimTree root.
	TArray<UAnimNode*> TestNodes;
	AnimTree->GetNodes(TestNodes);

	if( TestNodes.ContainsItem(NewPreviewNode) )
	{
		PreviewSkelComp->Animations = NewPreviewNode;		
	}
}


///////////////////////////////////////////////////////////////////////////////////////
// Properties window NotifyHook stuff

void WxAnimTreeEditor::NotifyDestroy( void* Src )
{

}

void WxAnimTreeEditor::NotifyPreChange( void* Src, UProperty* PropertyAboutToChange )
{
	GEditor->BeginTransaction( *LocalizeUnrealEd("EditLinkedObj") );

	for ( WxPropertyWindow::TObjectIterator Itor( PropertyWindow->ObjectIterator() ) ; Itor ; ++Itor )
	{
		(*Itor)->Modify();
	}
}

void WxAnimTreeEditor::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	GEditor->EndTransaction();

	// If we are editing the AnimTree (root) and know what changed.
	if(PropertyThatChanged && SelectedNodes.ContainsItem(AnimTree))
	{
		// If it was the SkeletalMesh, update the preview SkeletalMeshComponent
		if( PropertyThatChanged->GetFName() == FName(TEXT("PreviewSkelMesh")) )
		{
			PreviewSkelComp->SetSkeletalMesh(AnimTree->PreviewSkelMesh);
		}

		// If it was the PreviewAnimSets array, update the preview SkelMeshComponent and re-init tree (to pick up new AnimSequences).
		if( PropertyThatChanged->GetFName() == FName(TEXT("PreviewAnimSets")) )
		{
			PreviewSkelComp->AnimSets = AnimTree->PreviewAnimSets;
			AnimTree->InitAnim(PreviewSkelComp, NULL);
		}

		// If it was the PreviewMorphSets array, update the preview SkelMeshComponent and re-init tree (to pick up new MorphTargetSets).
		if( PropertyThatChanged->GetFName() == FName(TEXT("PreviewMorphSets")) )
		{
			PreviewSkelComp->MorphSets = AnimTree->PreviewMorphSets;
			AnimTree->InitTreeMorphNodes(PreviewSkelComp);
		}

		if( PropertyThatChanged->GetFName() == FName(TEXT("SocketSkelMesh")) || 
			PropertyThatChanged->GetFName() == FName(TEXT("SocketStaticMesh")) ||
			PropertyThatChanged->GetFName() == FName(TEXT("SocketName")) )
		{
			RecreateSocketPreview();
		}
	}

	RefreshViewport();
	AnimTree->MarkPackageDirty();
}

void WxAnimTreeEditor::NotifyExec( void* Src, const TCHAR* Cmd )
{
	GUnrealEd->NotifyExec(Src, Cmd);
}


void WxAnimTreeEditor::ClearSocketPreview()
{
	if( PreviewSkelComp && SocketComponent )
	{
		PreviewSkelComp->DetachComponent(SocketComponent);

		// delete is evil... and I trusted James ;)
		//delete SocketComponent;
		SocketComponent = NULL;
	}
}


/** Update the Skeletal and StaticMesh Components used to preview attachments in the editor. */
void WxAnimTreeEditor::RecreateSocketPreview()
{
	ClearSocketPreview();

	if( AnimTree && PreviewSkelComp && PreviewSkelComp->SkeletalMesh )
	{
		if( PreviewSkelComp->SkeletalMesh->FindSocket(AnimTree->SocketName) )
		{
			if( AnimTree->SocketSkelMesh )
			{
				// Create SkeletalMeshComponent and fill in mesh and scene.
				USkeletalMeshComponent* NewSkelComp = ConstructObject<USkeletalMeshComponent>(USkeletalMeshComponent::StaticClass());
				NewSkelComp->SetSkeletalMesh(AnimTree->SocketSkelMesh);

				// Attach component to this socket.
				PreviewSkelComp->AttachComponentToSocket(NewSkelComp, AnimTree->SocketName);

				// And keep track of it.
				SocketComponent = NewSkelComp;
			}

			if( AnimTree->SocketStaticMesh )
			{
				// Create StaticMeshComponent and fill in mesh and scene.
				UStaticMeshComponent* NewMeshComp = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass());
				NewMeshComp->SetStaticMesh(AnimTree->SocketStaticMesh);

				// Attach component to this socket.
				PreviewSkelComp->AttachComponentToSocket(NewMeshComp, AnimTree->SocketName);

				// And keep track of it
				SocketComponent = NewMeshComp;
			}
		}
	}
}
