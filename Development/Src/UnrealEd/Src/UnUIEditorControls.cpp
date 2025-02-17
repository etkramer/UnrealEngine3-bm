/*=============================================================================
	UnUIEditorControls.cpp: UI editor custom control implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UnUIStyleEditor.h"
#include "GenericBrowser.h"
#include "ScopedTransaction.h"
#include "ScopedObjectStateChange.h"
#include "UnConsoleSupportContainer.h"

/* ==========================================================================================================
	WxUIKismetTreeControl
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS (WxUIKismetTreeControl, WxSequenceTreeCtrl)

BEGIN_EVENT_TABLE (WxUIKismetTreeControl, WxSequenceTreeCtrl)
	EVT_TREE_ITEM_EXPANDING( IDM_LINKEDOBJED_TREE, WxUIKismetTreeControl::OnTreeExpanding )
	EVT_TREE_ITEM_COLLAPSED( IDM_LINKEDOBJED_TREE, WxUIKismetTreeControl::OnTreeCollapsed )
	EVT_TREE_ITEM_ACTIVATED( IDM_LINKEDOBJED_TREE, WxUIKismetTreeControl::OnTreeItemActivated )
	EVT_TREE_ITEM_RIGHT_CLICK( IDM_LINKEDOBJED_TREE, WxUIKismetTreeControl::OnTreeItemRightClick )
END_EVENT_TABLE()

/** Default constructor for use in two-stage dynamic window creation */
WxUIKismetTreeControl::WxUIKismetTreeControl()
: UIKismetEditor(NULL)
{
}

WxUIKismetTreeControl::~WxUIKismetTreeControl()
{
}

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 * @param	InEditor	pointer to the editor window that contains this control
 */
void WxUIKismetTreeControl::Create( wxWindow* InParent, wxWindowID InID, class WxUIKismetEditor* InEditor )
{
	UIKismetEditor = InEditor;

	WxSequenceTreeCtrl::Create (InParent,InID,InEditor, wxTR_HAS_BUTTONS | wxTR_MULTIPLE );

	wxImageList *images = new wxImageList (16, 15);
	InEditor->TreeImages = images;

	// These match order of TREE_ICON enum
	char *names[] = 
	{
		"FolderClosed",
		"FolderOpen",
		"UI_KIS_Sequence",
		"UI_KIS_Sequence_State",
		"UI_KIS_Event",
		"UI_KIS_Event_Key",
		"UI_KIS_Action",
		"UI_KIS_Conditional",
		"UI_KIS_Variable",
		0
	};

	for (INT n = 0; names[n]; ++n)
	{
		images->Add (WxBitmap (names[n]), wxColor (192, 192, 192));
	}

	AssignImageList (images);
}

/**
 * One or more objects changed, so update the tree
 */
void WxUIKismetTreeControl::NotifyObjectsChanged()
{
	// Save the selection/expansion state of the tree, refresh it, then restore any expanded/selected items.
	SaveSelectionExpansionState();
	{
		RefreshTree();
	}
	RestoreSelectionExpansionState();
}

/**
 * Repopulates the tree control with the heirarchy of Sequences from the current level.
 */
void WxUIKismetTreeControl::RefreshTree()
{
	DeleteAllItems();
	TreeMap.Empty();

	wxTreeItemId RootId = AddRoot( *LocalizeUI(TEXT("UIEditor_UISequences")), TREE_ICON_FOLDER_CLOSED, -1, NULL );

	AddWidgetSequences( RootId, UIKismetEditor->SequenceOwner );
	Expand(RootId);
}

/**
 * Adds the sequences contained by the specified widget to the tree control.
 *
 * @param	ParentId	the tree item id to use as the parent node for the new tree items
 * @param	SequenceOwner	the widget to add the sequences for
 */
void WxUIKismetTreeControl::AddWidgetSequences( wxTreeItemId ParentId, UUIScreenObject* SequenceOwner )
{
	check(SequenceOwner);

	// add a tree item for the widget itself
	wxTreeItemId WidgetItemId = AppendItem( ParentId, *SequenceOwner->GetTag().ToString(), TREE_ICON_FOLDER_CLOSED, -1, new WxTreeObjectWrapper(SequenceOwner) );

	Expand (WidgetItemId);

	// add a tree item for the widget's main sequence
	UUISequence* GlobalSeq = SequenceOwner->EventProvider->EventContainer;
	wxTreeItemId GlobalId = AppendItem (WidgetItemId, *LocalizeUI(TEXT("UIEditor_GlobalSequence")), TREE_ICON_SEQUENCE, -1, new WxTreeObjectWrapper(GlobalSeq));

	// Set the extra images for the item.
	SetAllItemImages(GlobalId, TREE_ICON_SEQUENCE);

	TreeMap.Set (GlobalSeq, GlobalId);

	// Add all of the objects under the global sequence to the control.
	for (INT Idx = 0; Idx < GlobalSeq->SequenceObjects.Num(); Idx++)
	{
		USequenceObject *SeqObj = GlobalSeq->SequenceObjects (Idx);

		USequenceEvent *SeqEvent = Cast<USequenceEvent>(SeqObj);
		if (SeqEvent)
		{
			AddSequenceOp (GlobalId, SeqEvent);
		}
	}

	// Add all state sequences for this widget to the tree.
	if ( SequenceOwner->InactiveStates.Num() > 0 )
	{
		AddStateSequences (WidgetItemId, SequenceOwner);
	}

	// Recursively add all children for this tree to the control under a folder.
	if ( SequenceOwner->Children.Num() > 0 )
	{
		wxTreeItemId ChildItemId = AppendItem( WidgetItemId, *LocalizeUI(TEXT("UIEditor_Children")), TREE_ICON_FOLDER_CLOSED, -1, new WxTreeObjectWrapper(SequenceOwner) );

		// Set the extra images for the item.
		SetItemImage(ChildItemId, TREE_ICON_FOLDER_OPEN, wxTreeItemIcon_Expanded);
		SetItemImage(ChildItemId, TREE_ICON_FOLDER_OPEN, wxTreeItemIcon_SelectedExpanded);

		AddChildren(ChildItemId, SequenceOwner);
	}
}

/**
 * Adds the sequences for each of the states supported by the specified widget to the tree control.
 *
 * @param	ParentId		the tree item id to use as the parent node for all of the state sequences
 * @param	SequenceOwner	the widget to add the sequences for
 */
void WxUIKismetTreeControl::AddStateSequences( wxTreeItemId ParentId, UUIScreenObject* SequenceOwner )
{
	check(SequenceOwner);

	for (INT StateIndex = 0; StateIndex < SequenceOwner->InactiveStates.Num(); StateIndex++)
	{
		UUIState *State = SequenceOwner->InactiveStates(StateIndex);

		UUIStateSequence *Sequence = State->StateSequence;

		// Find a friendly name for the state sequenece.
		FString ItemString = UIKismetEditor->GetFriendlySequenceName(Sequence);

		wxTreeItemId StateId = AppendItem (ParentId, *ItemString, TREE_ICON_SEQUENCE_STATE, -1, new WxTreeObjectWrapper (Sequence));
		TreeMap.Set (Sequence, StateId);

		// Set the extra images for the item.
		SetAllItemImages(StateId, TREE_ICON_SEQUENCE_STATE);

		for (INT ObjIdx = 0; ObjIdx < Sequence->SequenceObjects.Num(); ObjIdx++)
		{
			USequenceObject *SeqObj = Sequence->SequenceObjects (ObjIdx);

			USequenceEvent *SeqEvent = Cast<USequenceEvent>(SeqObj);
			if (SeqEvent)
			{
				AddSequenceOp (StateId, SeqEvent);
			}
		}
	}

	SortChildren(ParentId);
}

/**
 * Add the sequence op to the tree control and any children
 *
 * @param	ParentId		the tree item id to use as the parent node
 * @param	Op				the sequence op we are adding
 */
void WxUIKismetTreeControl::AddSequenceOp( wxTreeItemId ParentId, USequenceOp *Op )
{
	check (Op);

	// Make sure the Op isn't already in the graph.
	if(TreeMap.Find(Op) == NULL)
	{
		wxTreeItemId OpId;

		FString Name = Op->ObjName;

		UUIEvent* Event = Cast<UUIEvent>(Op);
		if(Event && (Cast<UUIEvent_ProcessInput>(Op) == NULL))
		{
			UUIEvent_MetaObject *MetaObjectEvent = Cast<UUIEvent_MetaObject>(Op);
			if (MetaObjectEvent)
			{
				OpId = AppendItem (ParentId, *Name, TREE_ICON_EVENT, -1, new WxTreeObjectWrapper (Op));

				// Set the extra images for the item.
				SetAllItemImages(OpId, TREE_ICON_EVENT);

				// Add subobjects for all of the actions we currently support.  Their pointer will just be to the meta object.
				UUIState* State = MetaObjectEvent->GetOwnerState();

				if(State)
				{
					check(State->StateInputActions.Num() == Op->OutputLinks.Num());

					for(INT KeyIdx = 0; KeyIdx < State->StateInputActions.Num(); KeyIdx++)
					{
						FString ActionString;
						MetaObjectEvent->GenerateActionDescription(State->StateInputActions(KeyIdx), ActionString);
						wxTreeItemId EventId = AppendItem (OpId, *ActionString, TREE_ICON_EVENT_KEY, -1, new WxTreeObjectWrapper (Op));

						// Set the extra images for the item.
						SetAllItemImages(EventId, TREE_ICON_EVENT_KEY);

						// Add the operations connected to this event.
						FSeqOpOutputLink *OutLink = &Op->OutputLinks (KeyIdx);

						for (INT LinkIdx = 0; LinkIdx < OutLink->Links.Num(); LinkIdx++)
						{
							FSeqOpOutputInputLink *Link = &OutLink->Links (LinkIdx);

							USequenceOp *Operation = Link->LinkedOp;

							if(Operation)
							{
								AddSequenceOp (EventId, Operation);
							}
						}
					}
				}


				TreeMap.Set (Op, OpId);

				// Return early because we connected our own links.
				return;
			}
			else	// Normal event processing.
			{
				OpId = AppendItem (ParentId, *Name, TREE_ICON_EVENT, -1, new WxTreeObjectWrapper (Op));

				// Set the extra images for the item.
				SetAllItemImages(OpId, TREE_ICON_EVENT);

				TreeMap.Set (Op, OpId);
			}
		}

		

		USequenceAction *seqAction = Cast<USequenceAction>(Op);
		if (seqAction)
		{
			OpId = AppendItem (ParentId, *Name, TREE_ICON_ACTION, -1, new WxTreeObjectWrapper (Op));

			// Set the extra images for the item.
			SetAllItemImages(OpId, TREE_ICON_ACTION);

			TreeMap.Set (Op, OpId);
		}

		USequenceCondition *seqCondition = Cast<USequenceCondition>(Op);
		if (seqCondition)
		{
			OpId = AppendItem (ParentId, *Name, TREE_ICON_CONDITIONAL, -1, new WxTreeObjectWrapper (Op));

			// Set the extra images for the item.
			SetAllItemImages(OpId, TREE_ICON_CONDITIONAL);

			TreeMap.Set (Op, OpId);
		}

		if (OpId.IsOk())
		{
			for (INT Idx = 0; Idx < Op->VariableLinks.Num(); Idx++)
			{
				FSeqVarLink *varLink = &Op->VariableLinks (Idx);

				for (INT Idx = 0; Idx < varLink->LinkedVariables.Num(); Idx++)
				{
					USequenceVariable *Var = varLink->LinkedVariables (Idx);
					FString Name;

					if(Var->VarName == NAME_None)
					{
						Name = Var->GetName();
					}
					else
					{
						Name = Var->VarName.ToString();
					}
					
					wxTreeItemId VarId = AppendItem (OpId, *Name, TREE_ICON_VARIABLE, -1, new WxTreeObjectWrapper (Var));
					
					// Set the extra images for the item.
					SetAllItemImages(VarId, TREE_ICON_VARIABLE);

					TreeMap.Set (Var, VarId);
				}
			}

			for (INT Idx = 0; Idx < Op->OutputLinks.Num(); Idx++)
			{
				FSeqOpOutputLink *OutLinks = &Op->OutputLinks (Idx);

				for (INT Idx = 0; Idx < OutLinks->Links.Num(); Idx++)
				{
					FSeqOpOutputInputLink *NextLink = &OutLinks->Links (Idx);

					USequenceOp *NextOp = NextLink->LinkedOp;

					if(NextOp)
					{
						AddSequenceOp (OpId, NextOp);
					}
				}
			}
		}
	}
}

/**
 * Recursively adds all subsequences of the specified sequence to the tree control.
 */
void WxUIKismetTreeControl::AddChildren( wxTreeItemId ParentId, UUIScreenObject* ParentObj )
{
	check(ParentObj);

	TArray<UUIObject*> Children = ParentObj->GetChildren();

	for ( INT ChildIndex = Children.Num() - 1; ChildIndex >= 0 ; ChildIndex-- )
	{
		AddWidgetSequences( ParentId, Children(ChildIndex) );
	}
}

/**
 * De/selects the tree item corresponding to the specified object
 *
 * @param	SeqObj		the sequence object to select in the tree control
 * @param	bSelect		whether to select or deselect the sequence
 *
 * @return	True if the specified sequence object was found
 */
UBOOL WxUIKismetTreeControl::SelectObject( USequenceObject* SeqObj, UBOOL bSelect/*=TRUE*/ )
{
	wxTreeItemId* TreeItemId = TreeMap.Find( SeqObj );
	if ( TreeItemId != NULL )
	{
		if( ((UBOOL)IsSelected(*TreeItemId) != bSelect) )
		{
			SelectItem( *TreeItemId, bSelect == TRUE );
			if ( bSelect )
			{
				EnsureVisible( *TreeItemId );
				Expand( *TreeItemId );
			}
		}

		return TRUE;
	}

	return FALSE;
}

/**
 * Called when an item has been activated, either by double-clicking or keyboard. Calls the OnDoubleClick in the sequence object's
 * helper class.
 */
void WxUIKismetTreeControl::OnTreeItemActivated( wxTreeEvent& Event )
{
	WxTreeObjectWrapper *LeafData = (WxTreeObjectWrapper *)GetItemData(Event.GetItem());
	if (LeafData != NULL)
	{
		UObject* Obj = LeafData->GetObject<UObject>();
		USequenceObject* SeqObj = Cast<USequenceObject>(Obj);

		if(SeqObj != NULL)
		{
			UClass* Class = LoadObject<UClass>( NULL, *SeqObj->GetEdHelperClassName(), NULL, LOAD_None, NULL );
			if ( Class != NULL )
			{
				USequenceObjectHelper* NodeHelper = CastChecked<USequenceObjectHelper>(Class->GetDefaultObject());

				if( NodeHelper )
				{
					// We need to default the mouse click position to the corner of the viewport since the new object menu uses the last
					// click position from the viewport as a point of creation for new objects.
					const INT ViewportOffset = 20;
					KismetEditor->LinkedObjVC->NewX = ViewportOffset;
					KismetEditor->LinkedObjVC->NewY = ViewportOffset;


					NodeHelper->OnDoubleClick(UIKismetEditor, SeqObj);
				}
			}
		}
	}
}


/**
 * Get the tree object that goes with the tree id
 *
 * @param	TreeId	the tree id
 * @return			the sequence object we found or NULL
 */
USequenceObject *WxUIKismetTreeControl::GetTreeObject( wxTreeItemId TreeId )
{
	USequenceObject *seqObj = NULL;

	if (TreeId.IsOk())
	{
		WxTreeObjectWrapper *leafData = (WxTreeObjectWrapper *) GetItemData (TreeId);
		if (leafData)
		{
			seqObj = leafData->GetObject<USequenceObject>();
		}
	}

	return seqObj;
}

/** Spawn a Context Menu depending on the selected object type. */
void WxUIKismetTreeControl::OnTreeItemRightClick(wxTreeEvent& In)
{
	WxTreeObjectWrapper *LeafData = (WxTreeObjectWrapper *)GetItemData(In.GetItem());
	if (LeafData != NULL)
	{
		UObject* Obj = LeafData->GetObject<UObject>();
		USequenceObject* SeqObj = Cast<USequenceObject>(Obj);

		if(SeqObj != NULL)
		{
			UClass* Class = LoadObject<UClass>( NULL, *SeqObj->GetEdHelperClassName(), NULL, LOAD_None, NULL );
			if ( Class != NULL )
			{
				USequenceObjectHelper* NodeHelper = CastChecked<USequenceObjectHelper>(Class->GetDefaultObject());

				if( NodeHelper )
				{
					// We need to default the mouse click position to the corner of the viewport since the new object menu uses the last
					// click position from the viewport as a point of creation for new objects.
					const INT ViewportOffset = 20;
					KismetEditor->LinkedObjVC->NewX = ViewportOffset;
					KismetEditor->LinkedObjVC->NewY = ViewportOffset;

					NodeHelper->ShowContextMenu(UIKismetEditor);
				}
			}
		}
	}
}

/* ==========================================================================================================
	WxSceneTreeCtrl
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxSceneTreeCtrl,WxTreeCtrl)

BEGIN_EVENT_TABLE( WxSceneTreeCtrl, WxTreeCtrl )
	EVT_TREE_KEY_DOWN( ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnKeyDown )
	EVT_TREE_ITEM_ACTIVATED( ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnItemActivated ) 
	EVT_TREE_BEGIN_DRAG(ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnBeginDrag)
	EVT_TREE_END_DRAG(ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnEndDrag)
	EVT_TREE_BEGIN_LABEL_EDIT(ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnBeginLabelEdit)
	EVT_TREE_END_LABEL_EDIT( ID_UI_TREE_SCENE,  WxSceneTreeCtrl::OnEndLabelEdit )


	// doesn't work correctly...numerous erroneous events are sent when multi-selection is enabled
	EVT_TREE_SEL_CHANGED( ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnSelectionChanged )
	EVT_TREE_SEL_CHANGING( ID_UI_TREE_SCENE, WxSceneTreeCtrl::OnSelectionChanging )
END_EVENT_TABLE()

/** Constructor */
WxSceneTreeCtrl::WxSceneTreeCtrl()
: UIEditor(NULL), bProcessingTreeSelection(FALSE), bEditingLabel(FALSE), bSynchronizingSelection(FALSE), bInDrag(FALSE)
{
	GCallbackEvent->Register(CALLBACK_UIEditor_ModifiedRenderOrder,this);
}

/** Destructor */
WxSceneTreeCtrl::~WxSceneTreeCtrl()
{
	GCallbackEvent->UnregisterAll(this);
}

/**
 * Initialize this control.  Must be the first function called after creation.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 * @param	InEditor	pointer to the editor window that contains this control
 */
void WxSceneTreeCtrl::Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor )
{
	UIEditor = InEditor;
	const UBOOL bCreateSuccesful = WxTreeCtrl::Create(InParent,InID,NULL,wxTR_HAS_BUTTONS|wxTR_MULTIPLE|wxTR_EXTENDED|wxTR_EDIT_LABELS);
	check( bCreateSuccesful );

	// Generate a image list with all of the bitmaps for each widget.
	wxImageList *ImageList = new wxImageList (16, 15);
	const INT MapCount = UIEditor->SceneManager->UIWidgetToolbarMaps.Num();

	// Add a icon for the scene.
	INT IconIdx = 0;
	ImageList->Add(WxBitmap (TEXT("UI_Scene")), wxColor (192, 192, 192));
	IconIdx++;

	// Add icons for widgets.
	for (INT MapIdx=0; MapIdx < MapCount; MapIdx++)
	{
		const FUIObjectToolbarMapping* Mapping = &UIEditor->SceneManager->UIWidgetToolbarMaps(MapIdx);
		ImageList->Add(WxBitmap (Mapping->IconName), wxColor (192, 192, 192));
		WidgetImageMap.Set(*Mapping->WidgetClassName, IconIdx);
		IconIdx++;
	}

	AssignImageList (ImageList);
}

/**
 * Repopulates the tree control with the widgets in the current scene
 */
void WxSceneTreeCtrl::RefreshTree()
{
	// remember the selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	// remember the expanded widgets
	TArray<UUIObject*> ExpandedWidgets = GetExpandedWidgets();

	// remove all existing items
	DeleteAllItems();
	WidgetIDMap.Empty();

	UUIScreenObject* RootObject = UIEditor->OwnerScene;
	check(RootObject);

	FName RootObjectName = RootObject->GetTag();
	if ( RootObjectName == NAME_None )
	{
		RootObjectName = RootObject->GetFName();
	}
	wxTreeItemId RootID = AddRoot( *RootObjectName.ToString(), 0, 0, new WxTreeObjectWrapper( RootObject ) );

	AddChildren( RootObject, RootID );
	Expand( RootID );

	// restore the selected widgets
	SetSelectedWidgets(SelectedWidgets);

	// restore the expanded widgets
	SetExpandedWidgets(ExpandedWidgets);
}


/**
 * Recursively adds all children of the specified parent to the tree control.
 *
 * @param	Parent		the widget containing the children that should be added
 * @param	ParentId	the tree id for the specified parent
 */
void WxSceneTreeCtrl::AddChildren( UUIScreenObject* Parent, wxTreeItemId ParentId )
{
	check(Parent);

	// iterate through this widget's children, adding them to the tree
	// we do this in reverse order so that the widget which would be rendered last is the first item in this node
	for ( INT i = Parent->Children.Num() - 1; i >= 0; i-- )
	{
		UUIObject* Widget = Parent->Children(i);

		// this widget is a member of another widget so it should not appear in the tree.
		if( Widget->IsPrivateBehaviorSet(UCONST_PRIVATE_TreeHiddenRecursive) )
		{
			continue;
		}

		if ( !Widget->IsPrivateBehaviorSet(UCONST_PRIVATE_TreeHidden) )
		{
			// See if we can find a icon for this widget.
			INT ItemImage = -1;
			INT* ImageId = WidgetImageMap.Find(Widget->GetClass()->GetPathName());

			if(ImageId != NULL)
			{
				ItemImage = *ImageId;
			}

			FName ItemName = Widget->GetTag();
			if ( ItemName == NAME_None )
			{
				ItemName = Widget->GetFName();
			}
			wxTreeItemId id = AppendItem( ParentId, *ItemName.ToString(), ItemImage, ItemImage, new WxTreeObjectWrapper(Widget) );

			// add this widget to the lookup table
			WidgetIDMap.Set(Widget, id);

			AddChildren(Widget, id);
		}
		else
		{
			// we can't show this widget, but we can show its children.  So add this widget's children as a
			// child item of this widget's parent item - in the tree, it will appear that this widget's children
			// are direct children of this widget's parent.
			AddChildren(Widget, ParentId);
		}
	}
}

/**
 * Expands the tree items corresponding to the specified widgets.
 *
 * @param	WidgetsToExpand		the widgets that should be expanded
 */
void WxSceneTreeCtrl::SetExpandedWidgets( TArray<UUIObject*> WidgetsToExpand )
{
	for ( INT i = 0; i < WidgetsToExpand.Num(); i++ )
	{
		UUIObject* Widget = WidgetsToExpand(i);
		wxTreeItemId* pItemId = WidgetIDMap.Find(Widget);
		if ( pItemId != NULL )
		{
			Expand(*pItemId);
		}
	}
}

/**
 * Get the list of widgets corresponding to the expanded items in the tree control
 */
TArray<UUIObject*> WxSceneTreeCtrl::GetExpandedWidgets()
{
	TArray<UUIObject*> ExpandedWidgets;
	for ( SceneTreeIterator It = TreeIterator(); It; ++It )
	{
		wxTreeItemId ItemId = It.GetId();
		if ( IsExpanded(ItemId) )
		{
			UUIObject* Widget = Cast<UUIObject>(It.GetObject());
			if ( Widget != NULL )
			{
				ExpandedWidgets.AddItem(Widget);
			}
		}
	}

	return ExpandedWidgets;
}

/**
 * Synchronizes the selected widgets across all windows.
 * If source is the editor window, iterates through the tree control, making sure that the selection state of each tree
 * item matches the selection state of the corresponding widget in the editor window.
 *
 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
 *					used as the authoritative selection set.
 */
void WxSceneTreeCtrl::SynchronizeSelectedWidgets( FSelectionInterface* Source )
{
	if ( !bSynchronizingSelection )
	{
		bSynchronizingSelection = TRUE;
		if ( Source == this )
		{
			UIEditor->SynchronizeSelectedWidgets(this);
		}
		else if ( Source == UIEditor )
		{
			TArray<UUIObject*> SelectedWidgets;
			UIEditor->GetSelectedWidgets(SelectedWidgets);

			SetSelectedWidgets(SelectedWidgets);
		}
		bSynchronizingSelection = FALSE;
	}
}


/**
 * Changes the selection set to the widgets specified.
 *
 * @return	TRUE if the selection set was accepted
 */
UBOOL WxSceneTreeCtrl::SetSelectedWidgets( TArray<UUIObject*> SelectionSet )
{
	UBOOL bResult = FALSE;
	for ( SceneTreeIterator Itor = TreeIterator(); Itor; ++Itor )
	{
		UUIObject* Widget = Cast<UUIObject>(Itor.GetObject());
		if ( Widget != NULL )
		{
			UBOOL bIsSelected = SelectionSet.ContainsItem(Widget);
			wxTreeItemId Id = Itor.GetId();

			SelectItem(Id, bIsSelected == TRUE);
			if ( bIsSelected )
			{
				EnsureVisible( Id );
			}
			bResult = TRUE;
		}
		else
		{
			wxTreeItemId Id = Itor.GetId();
			SelectItem(Id, false);
			bResult = TRUE;
		}
	}

	return bResult;
}


/**
* Retrieves the widgets corresonding to the selected items in the tree control
*
* @param	OutWidgets					Storage array for the currently selected widgets.
* @param	bIgnoreChildrenOfSelected	if TRUE, widgets will not be added to the list if their parent is in the list
*/
void WxSceneTreeCtrl::GetSelectedWidgets( TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected /*= FALSE*/ )
{
	TArray<wxTreeItemId> SelectedItems;
	GetSelections(SelectedItems,bIgnoreChildrenOfSelected);

	for ( INT i = 0; i < SelectedItems.Num(); i++ )
	{
		wxTreeItemId ItemId = SelectedItems(i);
		WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)GetItemData(ItemId);
		if ( ItemData )
		{
			UUIScreenObject* Obj = ItemData->GetObject<UUIScreenObject>();
			if ( Obj != NULL )
			{
				UUIObject* Widget = Cast<UUIObject>(Obj);
				if ( Widget != NULL )
				{
					OutWidgets.AddItem(Widget);
				}
			}
		}
	}
}

/**
 * Overloaded method for retrieving the selected items, with the option of ignoring children of selected items.
 *
 * @param	Selections					array of wxTreeItemIds corresponding to the selected items
 * @param	bIgnoreChildrenOfSelected	if TRUE, selected items are only added to the array if the parent item is not selected
 */
void WxSceneTreeCtrl::GetSelections( TArray<wxTreeItemId>& Selections, UBOOL bIgnoreChildrenOfSelected/*=FALSE*/ )
{
	for ( SceneTreeIterator It = TreeIterator(); It; ++It )
	{
		wxTreeItemId ItemId = It.GetId();
		if ( IsSelected(ItemId) )
		{
			Selections.AddItem(ItemId);
			if ( bIgnoreChildrenOfSelected )
			{
				It.Skip();
			}
		}
	}
}

/**
 * Activates the context menu associated with this tree control.
 *
 * @param	ItemData	the data associated with the item that the user right-clicked on
 */
void WxSceneTreeCtrl::ShowPopupMenu( WxTreeObjectWrapper* ItemData )
{
	SynchronizeSelectedWidgets(this);

	// Update viewport with new selections. 
	UIEditor->ObjectVC->Viewport->Draw();

	WxUISceneTreeContextMenu Menu(this);

	FTrackPopupMenu PopupTracker( this, &Menu );
	Menu.Create(ItemData);
	PopupTracker.Show();
}

/**
 * Called when a widget is selected/de-selected in the editor.  Synchronizes the "selected" state of the tree item corresponding to the specified widget.
 * @fixme - currently not hooked up, since the wx tree control doesn't send selection events correctly, so we must do all or nothing synchronization
 */
void WxSceneTreeCtrl::OnSelectWidget( UUIObject* Widget, UBOOL bSelected )
{
	wxTreeItemId* pItemId = WidgetIDMap.Find(Widget);
	if ( pItemId != NULL )
	{
		SelectItem(*pItemId, bSelected == TRUE);
		if ( bSelected )
		{
			EnsureVisible(*pItemId);
		}
	}
}

/**
 * Called when an item has been activated, either by double-clicking or keyboard. Synchronizes the selected widgets
 * between the editor window and the scene tree.
 */
void WxSceneTreeCtrl::OnItemActivated( wxTreeEvent& Event )
{
	SynchronizeSelectedWidgets(this);
}

/**
* Called when the selected item is changing
*/
void WxSceneTreeCtrl::OnSelectionChanging( wxTreeEvent& Event )
{
	if ( IsInSetFocus() )
	{
		Event.Veto();
	}
	else
	{
		SynchronizeSelectedWidgets(this);
	}
}

/**
 * Called when the selected item changes
 */
void WxSceneTreeCtrl::OnSelectionChanged( wxTreeEvent& Event )
{
	Event.Skip();
}

/**
 * Called when an item is dragged.
 */
void WxSceneTreeCtrl::OnBeginDrag( wxTreeEvent& Event )
{
	DragItem = Event.GetItem();

	if(bInDrag == FALSE && DragItem.IsOk() && bEditingLabel == FALSE)
	{
		WxTreeObjectWrapper* SourceItemData = (WxTreeObjectWrapper*)GetItemData(DragItem);
		if(SourceItemData != NULL)
		{
			UUIScreenObject* SourceObject = SourceItemData->GetObject<UUIScreenObject>();
			const UBOOL bIsAScene = (Cast<UUIObject>(SourceObject) == NULL);
			if(bIsAScene == FALSE)
			{
				bInDrag = TRUE;
				Event.Allow();
			}
		}
	}
}

/**
 * Called when an item is done being dragged.
 */
void WxSceneTreeCtrl::OnEndDrag( wxTreeEvent& Event )
{
	if(bInDrag == TRUE)
	{
		bInDrag = FALSE;

		// Reparent the widget depending on where we ended our drag.
		wxTreeItemId DropItem = Event.GetItem();

		if(DropItem.IsOk() && DragItem.IsOk() && DropItem != DragItem)
		{
			UBOOL bReparentItem = TRUE;

			WxTreeObjectWrapper* SourceItemData = (WxTreeObjectWrapper*)GetItemData(DragItem);
			WxTreeObjectWrapper* DropItemData = (WxTreeObjectWrapper*)GetItemData(DropItem);

			if(SourceItemData != NULL && DropItemData != NULL)
			{
				UUIScreenObject* SourceObject = SourceItemData->GetObject<UUIScreenObject>();
				UUIScreenObject* DropObject = DropItemData->GetObject<UUIScreenObject>();

				// Make sure that the drop item isnt a parent of the drag item.
				UUIScreenObject* ItemParent = Cast<UUIScreenObject>(DropObject->GetParent());

				while(ItemParent != NULL)
				{
					if(ItemParent == SourceObject)
					{
						bReparentItem = FALSE;
						break;
					}

					ItemParent = ItemParent->GetParent();
				}

				// Reparent the widget and refresh the tree.
				if(bReparentItem == TRUE)
				{
					FScopedTransaction Transaction( *LocalizeUI(TEXT("TransChangeParent")) );
					const UBOOL bSuccess = UIEditor->ReparentWidget(Cast<UUIObject>(SourceObject), DropObject);

					if(bSuccess == TRUE)
					{
						RefreshTree();
					}
					else
					{
						Transaction.Cancel();
					}
				}
			}
		}
	}
}

/**
 * Called when the user presses a key while the tree control has focus.
 */
void WxSceneTreeCtrl::OnKeyDown(wxTreeEvent& Event)
{
	const wxKeyEvent &KeyEvent = Event.GetKeyEvent();

	// Use key codes to make it easy to reorder a widget within its parent.  
	if(KeyEvent.ControlDown() && KeyEvent.ShiftDown())
	{
		switch(Event.GetKeyCode())
		{
			case ']':
				{
					wxCommandEvent Event(wxEVT_COMMAND_MENU_SELECTED, IDM_UI_MOVETOTOP);
					Event.SetEventObject(this);

					UIEditor->AddPendingEvent(Event);
				}
				break;

			case '[':
				{
					wxCommandEvent Event(wxEVT_COMMAND_MENU_SELECTED, IDM_UI_MOVETOBOTTOM);
					Event.SetEventObject(this);

					UIEditor->AddPendingEvent(Event);
				}
				break;

			default:
				break;
		}
	}
	else if(KeyEvent.ControlDown())
	{
		switch(Event.GetKeyCode())
		{
			case ']': 
				{
					wxCommandEvent Event(wxEVT_COMMAND_MENU_SELECTED, IDM_UI_MOVEUP);
					Event.SetEventObject(this);

					UIEditor->AddPendingEvent(Event);
				}
				break;

			case '[':
				{
					wxCommandEvent Event(wxEVT_COMMAND_MENU_SELECTED, IDM_UI_MOVEDOWN);
					Event.SetEventObject(this);

					UIEditor->AddPendingEvent(Event);
				}
				break;

			default:
				break;
		}
	}


			

	Event.Veto();
}

/**
 * Called when the user begins editing a label.
 */
void WxSceneTreeCtrl::OnBeginLabelEdit(wxTreeEvent &Event )
{
	if(bEditingLabel == FALSE)
	{
		bEditingLabel = TRUE;
		Event.Allow();
	}
	else
	{
		Event.Veto();
	}
}

/**
 * Called when the user finishes editing a label.
 */
void WxSceneTreeCtrl::OnEndLabelEdit( wxTreeEvent &Event )
{
	if(bEditingLabel == TRUE)
	{
		UBOOL bAcceptChange = FALSE;
		wxTreeItemId TreeItem = Event.GetItem();

		if(TreeItem.IsOk() && !Event.IsEditCancelled())
		{
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)GetItemData(TreeItem);

			if(ItemData != NULL)
			{
				UUIScreenObject* SourceObject = ItemData->GetObject<UUIScreenObject>();
				FString NewName = FString(Event.GetLabel());

				FString Reason(LocalizeUnrealEd(TEXT("Error_InvalidInput")));
				if ( NewName.Len() > 0 && FIsValidObjectName(*NewName, Reason) )
				{
					// Try to cast this object to a UIObject and a UIScene and call the appropriate rename function, otherwise veto the rename.
					UUIScene* Scene = Cast<UUIScene>(SourceObject);

					if(Scene != NULL)
					{
						bAcceptChange = UIEditor->RenameObject(Scene, *NewName);
					}
					else
					{
						UUIObject* Widget = Cast<UUIObject>(SourceObject);
						if(Widget != NULL)
						{
							bAcceptChange = UIEditor->RenameObject(Widget, *NewName);
						}
					}
				}
				else
				{
					appMsgf(AMT_OK, *Reason);
				}
			}
		}

		// Cancel the edit if the user entered an invalid name,
		if( !bAcceptChange )
		{
			Event.Veto();
		}

		bEditingLabel = FALSE;
	}
}

/** FCallbackEventDevice interface */
void WxSceneTreeCtrl::Send( ECallbackEventType InType )
{
	switch ( InType )
	{
	case CALLBACK_UIEditor_ModifiedRenderOrder:
		RefreshTree();
		break;
	}
}

void WxSceneTreeCtrl::Send( ECallbackEventType InType, UObject* Obj )
{
	switch ( InType )
	{
	case CALLBACK_UIEditor_ModifiedRenderOrder:
		RefreshTree();
		break;
	}
}

/* ==========================================================================================================
	WxUIStyleTreeCtrl
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIStyleTreeCtrl,WxTreeCtrl)

/** Default constructor for use in two-stage dynamic window creation */
WxUIStyleTreeCtrl::WxUIStyleTreeCtrl()
: UIEditor(NULL)
{
	// we want to know about undo events
	GCallbackEvent->Register(CALLBACK_Undo, this);
//	GCallbackEvent->Register(CALLBACK_SelectObject,this);
}

/** Destructor */
WxUIStyleTreeCtrl::~WxUIStyleTreeCtrl()
{
	// remove ourselves from the list of registered callback observers
	GCallbackEvent->UnregisterAll(this);
}

/**
 * Initialize this control.  Must be the first function called after creation.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 * @param	InEditor	pointer to the editor window that contains this control
 */
void WxUIStyleTreeCtrl::Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor )
{
	UIEditor = InEditor;
	const UBOOL bSuccess = WxTreeCtrl::Create(InParent,InID,new WxUIStyleTreeContextMenu,wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_HIDE_ROOT);
	check( bSuccess );

	wxImageList *ImageList = new wxImageList (16, 15);

	// These match order of STYLE_ICON_ENUM
	char *ImageNames[] = 
	{
		"Style_Text",
		"Style_Image",
		"Style_Combo",
		0
	};

	for (INT NameIdx = 0; ImageNames[NameIdx]; ++NameIdx)
	{
		ImageList->Add (WxBitmap (ImageNames[NameIdx]), wxColor (192, 192, 192));
	}

	AssignImageList(ImageList);
}

/**
 * Retrieves the UISkin that is currently set as the root of this tree control
 */
UUISkin* WxUIStyleTreeCtrl::GetCurrentSkin()
{
	UUISkin* Result = NULL;

	if ( GetRootItem().IsOk() )
	{
		WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)GetItemData(GetRootItem());
		if ( ItemData != NULL )
		{
			Result = ItemData->GetObject<UUISkin>();
		}
	}

	return Result;
}

/**
 * Retrieves the UIStyle associated with the currently selected item
 */
UUIStyle* WxUIStyleTreeCtrl::GetSelectedStyle()
{
	UUIStyle* Result = NULL;
	if ( GetRootItem().IsOk() )
	{
		wxTreeItemId SelectedItemId = GetSelection();
		if ( SelectedItemId.IsOk() )
		{
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)GetItemData(SelectedItemId);
			if ( ItemData != NULL )
			{
				Result = ItemData->GetObject<UUIStyle>();
			}
		}
	}

	return Result;
}

/**
 * Repopulates the tree control with the styles in the active skin
 */
void WxUIStyleTreeCtrl::RefreshTree( UUISkin* CurrentlyActiveSkin )
{
	check(CurrentlyActiveSkin);

	// remove all existing items
	DeleteAllItems();
	StyleIDMap.Empty();

	// add the skin as the root (it will not be drawn, however)
	wxTreeItemId RootID = AddRoot( *CurrentlyActiveSkin->GetPathName(), -1, -1, new WxTreeObjectWrapper(CurrentlyActiveSkin) );

	TArray<UUIStyle*> SkinStyles;
	CurrentlyActiveSkin->GetAvailableStyles(SkinStyles, FALSE);

	// iterate through the styles in the skin, adding them to the tree
	for ( INT i = 0; i < SkinStyles.Num(); i++ )
	{
		UUIStyle* Style = SkinStyles(i);
		if ( StyleIDMap.Find(Style) == NULL )
		{
			AddStyle(Style);
		}
	}
}

/**
 * Appends a tree item for the specified style.  If the style is based on another style, ensures that
 * the style's template has already been added (if the template exists in the current skin), and adds
 * the style as a child of its template.  Otherwise, the style is added as a child of the root item.
 *
 * @param	Style	the style to add to the tree control
 *
 * @return	the tree id for the style
 */
wxTreeItemId WxUIStyleTreeCtrl::AddStyle( UUIStyle* Style )
{
	checkSlow(Style);

	// skip it if this style has been added (which might occur if another style was based on this one)
	wxTreeItemId* pItemId = StyleIDMap.Find(Style);
	if ( pItemId != NULL )
	{
		return *pItemId;
	}

	wxTreeItemId ParentId;
	UUIStyle* StyleTemplate = CastChecked<UUIStyle>(Style->GetArchetype());

	// if this style is not based on any other style, add it to the tree as a child of root
	if ( StyleTemplate->HasAnyFlags(RF_ClassDefaultObject) )
	{
		ParentId = GetRootItem();
	}
	else
	{
		// if this style's archetype is contained within the same skin, make sure that skin is added first
		if ( GetCurrentSkin()->ContainsStyle(StyleTemplate) )
		{
			ParentId = AddStyle(StyleTemplate);
		}
		else
		{
			// otherwise, we'll add this item to the root (for now...maybe we should do something else here)
			ParentId = GetRootItem();
		}
	}
	
	// Find a icon for this style depending on its type. 
	INT StyleIcon = -1;

	if(Style->StyleDataClass->IsChildOf(UUIStyle_Combo::StaticClass()) == TRUE)
	{
		StyleIcon = SI_Combo;
	}
	else if(Style->StyleDataClass->IsChildOf(UUIStyle_Image::StaticClass()) == TRUE)
	{
		StyleIcon = SI_Image;
	}
	else if(Style->StyleDataClass->IsChildOf(UUIStyle_Text::StaticClass()) == TRUE)
	{
		StyleIcon = SI_Text;
	}

	wxTreeItemId ItemId = AppendItem( ParentId, *Style->GetStyleName(), StyleIcon, StyleIcon, new WxTreeObjectWrapper(Style) );
	StyleIDMap.Set(Style, ItemId);

	return ItemId;
}

/**
 * Callback for wxWindows RMB events. Default behaviour is to show the popup menu if a WxTreeObjectWrapper
 * is attached to the tree item that was clicked.
 */
void WxUIStyleTreeCtrl::OnRightButtonDown( wxMouseEvent& In )
{
	if(ContextMenu != NULL)
	{
		const wxTreeItemId TreeId = HitTest( wxPoint( In.GetX(), In.GetY() ) );

		if( TreeId.IsOk() == true )
		{
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)GetItemData( TreeId );
			if( ItemData )
			{
				ContextMenu->Enable(IDM_UI_DELETESTYLE, TRUE);
				ContextMenu->Enable(IDM_UI_CREATESTYLE_FROMTEMPLATE, TRUE);
				ContextMenu->Enable(IDM_UI_EDITSTYLE, TRUE);

				ShowPopupMenu(ItemData);		
			}
		}
		else
		{
			ContextMenu->Enable(IDM_UI_DELETESTYLE, FALSE);
			ContextMenu->Enable(IDM_UI_CREATESTYLE_FROMTEMPLATE, FALSE);
			ContextMenu->Enable(IDM_UI_EDITSTYLE, FALSE);

			ShowPopupMenu(NULL);
		}
	}

}

/** FCallbackEventDevice interface */
void WxUIStyleTreeCtrl::Send( ECallbackEventType InType )
{
	switch ( InType )
	{
	// user performed undo - refresh the tree to purge any stale styles
	case CALLBACK_Undo:
		{
			// save the currently selected style
			UUIStyle* CurrentlySelectedStyle = GetSelectedStyle();

			RefreshTree(GetCurrentSkin());

			if ( CurrentlySelectedStyle != NULL )
			{
				// if the style that was previously selected is still in the list, reselect it
				wxTreeItemId* pItemId = StyleIDMap.Find(CurrentlySelectedStyle);
				if ( pItemId != NULL )
				{
					SelectItem(*pItemId,true);
					EnsureVisible(*pItemId);
				}
			}

			break;
		}
	}
}

void WxUIStyleTreeCtrl::Send( ECallbackEventType InType, UObject* Obj )
{
	switch ( InType )
	{
	case CALLBACK_SelectObject:
		break;
	}
}
#if 0

/* ==========================================================================================================
	WxUISequenceTreeCtrl
========================================================================================================== */
/** Default constructor for use in two-stage dynamic window creation */
WxUISequenceTreeCtrl::WxUISequenceTreeCtrl()
: UIEditor(NULL)
{
	GCallbackEvent->Register(CALLBACK_SelChange,this);
	GCallbackEvent->Register(CALLBACK_SelectObject,this);
}

/** Destructor */
WxUISequenceTreeCtrl::~WxUISequenceTreeCtrl()
{
	GCallbackEvent->UnregisterAll(this);
}

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 * @param	InEditor	pointer to the editor window that contains this control
 */
void WxUISequenceTreeCtrl::Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor )
{
	UIEditor = InEditor;
	const UBOOL bSuccess = WxTreeCtrl::Create(InParent,InID,NULL,wxTR_HAS_BUTTONS);
	check( bSuccess );
}

/**
 * Repopulates the tree control with the sequence objects for the selected widgets
 *
 * @param	Filter	if specified, only the sequence objects contained by the
 */
void WxUISequenceTreeCtrl::RefreshTree( IUIEventContainer* Filter/*=NULL*/ )
{
	// remove all existing items
	DeleteAllItems();
	WidgetIDMap.Empty();

	TArray<UUIScreenObject*> SelectedObjects = UIEditor->GetSelectedWidgets();

	if ( Filter == NULL )
	{
		// for each of the selected widgets

			// add a "Global" node, which will be the parent for all events in the widget's sequence,
			// saving the tree id to the map for later lookup

			// for each of the states supported by this widget, add a node that will be the parent for the
			// the events specific to that state, saving the tree id to the map for later lookup


			// for each of the event containers that were added

				// add an "Events" node

				// add node for each of the events contained by this container

				// for each



		// if the container is a UIState, add an InputKeys node
		
		// add a node for each of the events contained by the widge's sequence
	}
	else
	{
		// figure out which type of container this is
		UObject* Obj = Filter->GetUObjectInterfaceUIInterface();
		check(Obj);

		UUISequence* Sequence = Cast<UUISequence>(Obj);
		if ( Sequence != NULL )
		{
			AddEvents(Sequence);
		}
		else
		{
			UUIState* State = Cast<UUIState>(Obj);
			if ( State != NULL )
			{
				AddEvents(State);
				AddInputKeys(State);
			}
			else
			{
				appMsgf(AMT_OK, TEXT("Unhandled event container type '%s'"), *Obj->GetName());
			}
		}
	}
}

/** FCallbackEventDevice interface */
void WxUISequenceTreeCtrl::Send( ECallbackEventType InType )
{
	switch ( InType )
	{
	case CALLBACK_SelChange:
// 		if ( !bProcessingTreeSelection )
// 		{
// 			SynchronizeSelectedWidgets();
// 		}
		break;
	}
}

void WxUISequenceTreeCtrl::Send( ECallbackEventType InType, UObject* Obj )
{
	switch ( InType )
	{
	case CALLBACK_SelectObject:
// 		if ( !bProcessingTreeSelection )
// 		{
// 			if ( Cast<UUIObject>(Obj) != NULL )
// 			{
// 				OnSelectWidget( Cast<UUIObject>(Obj), Obj->IsSelected() );
// 			}
// 		}
		break;
	}
}
#endif

/* ==========================================================================================================
	WxStyleBrowser
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxStyleBrowser,wxPanel)

BEGIN_EVENT_TABLE(WxStyleBrowser,wxPanel)
	EVT_SIZE(WxStyleBrowser::OnSize)
	EVT_TREE_ITEM_ACTIVATED(ID_UI_TREE_STYLE, WxStyleBrowser::OnDoubleClickStyle)
	EVT_BUTTON(IDB_UI_LOADSKIN, WxStyleBrowser::OnButton_LoadSkin)
	EVT_MENU(IDM_UI_CREATESTYLE, WxStyleBrowser::OnContext_CreateStyle)
	EVT_MENU(IDM_UI_CREATESTYLE_FROMTEMPLATE, WxStyleBrowser::OnContext_CreateStyle)
	EVT_MENU(IDM_UI_EDITSTYLE, WxStyleBrowser::OnContext_EditStyle)
	EVT_MENU(IDM_UI_DELETESTYLE, WxStyleBrowser::OnContext_DeleteStyle)
	EVT_CHOICE(ID_UI_COMBO_CHOOSESKIN,WxStyleBrowser::OnChangeActiveSkin)
	EVT_UPDATE_UI(IDM_UI_DELETESTYLE, WxStyleBrowser::OnUpdateUI)

END_EVENT_TABLE()

/** Constructor */
WxStyleBrowser::WxStyleBrowser()
: ActiveSkinCombo(NULL), LoadSkinButton(NULL), StyleTree(NULL), UIEditor(NULL)
{
}

/**
 * Initialize this panel and its children.  Must be the first function called after creating the panel.
 *
 * @param	InEditor	the ui editor that contains this panel
 * @param	InParent	the window that owns this panel (may be different from InEditor)
 * @param	InID		the ID to use for this panel
 */
void WxStyleBrowser::Create( WxUIEditorBase* InEditor, wxWindow* InParent, wxWindowID InID/*=ID_UI_PANEL_SKINBROWSER*/ )
{
	UIEditor = InEditor;
	const bool bSuccess = wxPanel::Create(InParent,InID,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);
	check( bSuccess );

	CreateControls();

	// initialize the value of the skin selectin combobox
	UUISkin* ActiveSkin = UIEditor->SceneManager->ActiveSkin;
	ActiveSkinCombo->SetStringSelection(*ActiveSkin->GetPathName());

	// initialize the style tree with the currently active skin
	StyleTree->RefreshTree(ActiveSkin);

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
}

/**
 * Creates the controls and sizers for this dialog.
 */
void WxStyleBrowser::CreateControls()
{
	wxBoxSizer* PanelVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(PanelVSizer);

	wxBoxSizer* SkinChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
	PanelVSizer->Add(SkinChoiceSizer, 0, wxGROW|wxTOP, 5);

	ActiveSkinCombo = new wxChoice( this, ID_UI_COMBO_CHOOSESKIN, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	PopulateSkinCombo();

	ActiveSkinCombo->SetHelpText(*LocalizeUI(TEXT("StyleBrowser_HelpText_SkinCombo")));
	ActiveSkinCombo->SetToolTip(*LocalizeUI(TEXT("StyleBrowser_ToolTip_SkinCombo")));
	SkinChoiceSizer->Add(ActiveSkinCombo, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

	wxBitmap LoadSkinButtonBitmap( *FString::Printf(TEXT("%swxRes\\ASV_NewNotify.bmp"), appBaseDir()), wxBITMAP_TYPE_BMP );
	LoadSkinButton = new wxBitmapButton( this, IDB_UI_LOADSKIN, LoadSkinButtonBitmap, wxDefaultPosition, wxSize(18, 18), wxBU_AUTODRAW );
	LoadSkinButton->SetToolTip(*LocalizeUI(TEXT("StyleBrowser_ToolTip_SkinButton")));
	SkinChoiceSizer->Add(LoadSkinButton, 0, wxALIGN_TOP|wxALL, 3);

	wxBoxSizer* StyleTreeSizer = new wxBoxSizer(wxHORIZONTAL);
	PanelVSizer->Add(StyleTreeSizer, 1, wxGROW|wxBOTTOM, 5);

	StyleTree = new WxUIStyleTreeCtrl;
	StyleTree->Create( this, ID_UI_TREE_STYLE, UIEditor );
	StyleTreeSizer->Add(StyleTree, 1, wxGROW|wxALL, 2);
}

/**
 * Populates the skin selection combobox with the loaded UISkins
 */
void WxStyleBrowser::PopulateSkinCombo()
{
	Freeze();

	// clear all existing items
	ActiveSkinCombo->Clear();

	for ( TObjectIterator<UUISkin> It; It; ++It )
	{
		UUISkin* Skin = *It;

		// add this skin to the list
		if ( !Skin->IsTemplate() )
		{
			ActiveSkinCombo->Append( *Skin->GetPathName(), Skin );
		}
	}

	Thaw();
}

/**
 * Called when the user has changed the active skin.  Notifies all other editor windows about the change.
 *
 * @param	NewActiveSkin			the skin that will now be the active skin
 * @param	PreviouslyActiveSkin	the skin that was previously the active skin
 *
 * @todo - notify the ui editor so that it can reapply all styles from the new skin
 */
void WxStyleBrowser::NotifyActiveSkinChanged( UUISkin* NewActiveSkin, UUISkin* PreviouslyActiveSkin )
{
	if ( UIEditor->SceneManager->SetActiveSkin(NewActiveSkin) )
	{
		// refill the style browser with the styles from this skin
		StyleTree->RefreshTree(NewActiveSkin);
	}
}

/**
 * Displays the "edit style" dialog to allow the user to modify the specified style.
 */
void WxStyleBrowser::DisplayEditStyleDialog( UUIStyle* StyleToEdit )
{
	//@todo - support editing multiple styles at once
	//@todo - while editing a style, highight all widgets that are currently using that style
	WxDlgEditUIStyle* dlg = new WxDlgEditUIStyle;
	dlg->Create(StyleToEdit, this);
	dlg->Show();
}

/**
 * Rebuilds the combo list and sets the appropriate skin as the selected item.
 *
 * @param	SkinToSelect	the skin to make the selected item in the combo after it's refreshed.  If not specified, attempts to
 *							preserve the currently selected item, or if there is none, uses the currently active skin.
 */
void WxStyleBrowser::RefreshAvailableSkinCombo( UUISkin* SkinToSelect/*=NULL*/ )
{
	check(ActiveSkinCombo);

	// save the name of the currently selected skin, since calling PopulateSkinCombo might change the position of the selected skin in the combo
	wxString CurrentSelection;
	if ( ActiveSkinCombo->GetCount() > 0 )
	{
		CurrentSelection = ActiveSkinCombo->GetStringSelection();
	}

	PopulateSkinCombo();

	INT SelectionIndex=wxNOT_FOUND;
	// if a skin was specified, attempt to find the position of the skin in the combo
	if ( SkinToSelect != NULL )
	{
		SelectionIndex = ActiveSkinCombo->FindString(*SkinToSelect->GetPathName());
	}
	// if we don't have a valid selection index yet, attempt to preserve the currently selected item
	if ( SelectionIndex == wxNOT_FOUND && CurrentSelection.Len() > 0 )
	{
		SelectionIndex = ActiveSkinCombo->FindString(CurrentSelection);
	}
	// if we still don't have a valid selection index, attempt to select the item corresponding to the currently active skin
	if ( SelectionIndex == wxNOT_FOUND )
	{
		UUISkin* CurrentlyActiveSkin = StyleTree->GetCurrentSkin();
		if ( CurrentlyActiveSkin == NULL )
		{
			CurrentlyActiveSkin = UIEditor->SceneManager->ActiveSkin;
		}

		if ( CurrentlyActiveSkin != NULL )
		{
			SelectionIndex = ActiveSkinCombo->FindString(*CurrentlyActiveSkin->GetPathName());
		}
	}
	// and if we still don't have a valid selection index, just select the first one
	if ( SelectionIndex == wxNOT_FOUND )
	{
		SelectionIndex = 0;
	}

	// now figure out if the skin we're going to select is the one that is currently active; if not, send a notification
	UUISkin* SelectedSkin = static_cast<UUISkin*>(ActiveSkinCombo->GetClientData(SelectionIndex));
	UUISkin* CurrentSkin = StyleTree->GetCurrentSkin();

	ActiveSkinCombo->SetSelection(SelectionIndex);
	if ( CurrentSkin != SelectedSkin )
	{
		NotifyActiveSkinChanged(SelectedSkin, CurrentSkin);
	}
}

void WxStyleBrowser::OnSize( wxSizeEvent& Event )
{
	Event.Skip();
}

void WxStyleBrowser::OnButton_LoadSkin( wxCommandEvent& Event )
{
	WxUILoadSkinContextMenu menu(this);

	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

/** Called when the user double-clicks or presses enter on a style in the tree */
void WxStyleBrowser::OnDoubleClickStyle( wxTreeEvent& Event )
{
	UUIStyle* StyleToEdit = StyleTree->GetSelectedStyle();
	if ( StyleToEdit != NULL )
	{
		DisplayEditStyleDialog(StyleToEdit);
	}
	else
	{
		Event.Skip();
	}
}

/** Called when the user changes the value of the skin combo */
void WxStyleBrowser::OnChangeActiveSkin( wxCommandEvent& Event )
{
	UUISkin* PreviousSkin = StyleTree->GetCurrentSkin();
	UUISkin* NewSkin = (UUISkin*)Event.GetClientData();

	// send off a notification to the rest of the editor that the skin has been changed
	NotifyActiveSkinChanged(NewSkin,PreviousSkin);
}

/** Called when the user selects the "Create Style" context menu option */
void WxStyleBrowser::OnContext_CreateStyle( wxCommandEvent& Event )
{
	WxDlgCreateUIStyle dlg;
	dlg.Create(this);

	UUIStyle* StyleTemplate = Event.GetId() == IDM_UI_CREATESTYLE_FROMTEMPLATE
		? StyleTree->GetSelectedStyle()
		: NULL;

	if ( dlg.ShowModal(TEXT(""),TEXT(""), StyleTemplate) == wxID_OK )
	{
		// create the new style
		UClass* StyleClass = dlg.GetStyleClass();
		UUIStyle* StyleTemplate = dlg.GetStyleTemplate();
		FString StyleFriendlyName = dlg.GetFriendlyName();
		FString StyleTag = dlg.GetStyleTag();
		UUISkin* CurrentSkin = StyleTree->GetCurrentSkin();

		check(CurrentSkin);
		check(StyleClass);

		CurrentSkin->SetFlags(RF_Transactional);

		FScopedTransaction Transaction( *LocalizeUI(TEXT("TransCreateStyle")) );

		UUIStyle* NewStyle = CurrentSkin->CreateStyle(StyleClass, *StyleTag, StyleTemplate, TRUE);
		if ( NewStyle != NULL )
		{
			NewStyle->StyleName = StyleFriendlyName;

			// refresh the style tree so that the new style appears in the list
			StyleTree->RefreshTree(CurrentSkin);

			// get the wxTreeItemId associated with the new style
			wxTreeItemId* pItemId = StyleTree->StyleIDMap.Find(NewStyle);
			check(pItemId);

			// ensure it's visible in the style browser
			StyleTree->EnsureVisible(*pItemId);

			// then display the style editor to allow the user to customize this style
			DisplayEditStyleDialog(NewStyle);
		}
		else
		{
			Transaction.Cancel();
		}
	}
}

/** Called when the user selects the "Edit Style" context menu option */
void WxStyleBrowser::OnContext_EditStyle( wxCommandEvent& Event )
{
	UUIStyle* StyleToEdit = StyleTree->GetSelectedStyle();
	if ( StyleToEdit != NULL )
	{
		DisplayEditStyleDialog(StyleToEdit);
	}
	else
	{
		Event.Skip();
	}
}

/** Called when the user selects the "Delete Style" context menu option */
void WxStyleBrowser::OnContext_DeleteStyle( wxCommandEvent& Event )
{

	UUISkin* Skin = StyleTree->GetCurrentSkin();
	UUIStyle* StyleToEdit = StyleTree->GetSelectedStyle();

	if(Skin && StyleToEdit)
	{
		const UBOOL bImplementedStyle = StyleToEdit->GetOuter() == Skin;
		wxTreeItemId StyleItemId = StyleTree->GetSelection();

		if(StyleItemId && bImplementedStyle == TRUE)
		{			

			// Make sure that we should delete this style by checking to see if any widgets are currently referencing this style.
			// If so, display a dialog and let the user confirm that they want to delete the style.
			const UUISceneManager *SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
			const UBOOL bDeleteStyle = SceneManager->ShouldDeleteStyle(StyleToEdit, FALSE);

			if(bDeleteStyle == TRUE)
			{
				FScopedTransaction Transaction(*LocalizeUI(TEXT("UIEditor_MenuItem_DeleteStyle")));
				if(Skin->DeleteStyle(StyleToEdit))
				{
					// Since we deleted a style, we need to tell widgets to reinitialize their styles.
					UUISceneManager *SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
					UUISkin *ActiveSkin = SceneManager->ActiveSkin;
					for ( INT SceneIndex = 0; SceneIndex < SceneManager->SceneClients.Num(); SceneIndex++ )
					{
						UEditorUISceneClient* Client = SceneManager->SceneClients(SceneIndex);
						Client->OnActiveSkinChanged();
					}

					// remove the item from the style tree
					StyleTree->SaveSelectionExpansionState();
					{
						StyleTree->RefreshTree(Skin);
					}
					StyleTree->RestoreSelectionExpansionState();
				}
				else
				{
					Transaction.Cancel();
				}
			}
		}
	}
}

/** Called when wx wants to update the UI for a widget. */
void WxStyleBrowser::OnUpdateUI(wxUpdateUIEvent &Event)
{
	switch(Event.GetId())
	{
	case IDM_UI_DELETESTYLE:
		{
			UUIStyle* SelectedStyle = StyleTree->GetSelectedStyle();

			if(SelectedStyle != NULL && SelectedStyle->IsDefaultStyle() == FALSE)
			{
				Event.Enable(true);
			}
			else
			{
				Event.Enable(false);
			}
		}
		break;
	}
}

/* ==========================================================================================================
	WxUIEditorToolBar
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIEditorToolBar,wxToolBar)

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 */
void WxUIEditorToolBar::Create( wxWindow* InParent, wxWindowID InID, long ToolbarStyle, WxUIEditorBase* InEditorWindow )
{
	UIEditor = InEditorWindow;
	const bool bSuccess = wxToolBar::Create(InParent,InID, wxDefaultPosition, wxDefaultSize, ToolbarStyle);
	check( bSuccess );
}

/** Destructor */
WxUIEditorToolBar::~WxUIEditorToolBar()
{
}

/* ==========================================================================================================
	WxUIEditorMainToolBar
========================================================================================================== */
IMPLEMENT_DYNAMIC_CLASS(WxUIEditorMainToolBar,WxUIEditorToolBar)

/** Default constructor for use in two-stage dynamic window creation */
WxUIEditorMainToolBar::WxUIEditorMainToolBar()
{
	Init();
}

/** Destructor */
WxUIEditorMainToolBar::~WxUIEditorMainToolBar()
{
}

/**
 * Initializes the values of this window's members.
 */
void WxUIEditorMainToolBar::Init()
{
	cmb_ViewportSize = NULL;
	cmb_SSPreviewState = NULL;
	cmb_WidgetPreviewState = NULL;
	cmb_PreviewPlatform = NULL;
	spin_ViewportGutter = NULL;
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 */
void WxUIEditorMainToolBar::Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditorWindow )
{
	WxUIEditorToolBar::Create(InParent,InID,wxTB_FLAT|wxTB_HORIZONTAL,InEditorWindow); 

	LoadButtonResources();
	CreateControls();
}

void WxUIEditorMainToolBar::CreateControls()
{
	AddCheckTool( ID_UI_SHOWVIEWPORTOUTLINE, TEXT(""), ViewportOutlineB, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_ToolTip_ViewportOutline")) );
	AddCheckTool( ID_UI_SHOWCONTAINEROUTLINE, TEXT(""), ContainerOutlineB, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_ToolTip_ContainerOutline")) );
	AddCheckTool( ID_UI_SHOWSELECTIONOUTLINE, TEXT(""), SelectionOutlineB, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_ToolTip_SelectionOutline")) );
	AddCheckTool( ID_UI_SHOWSELECTIONHANDLES, TEXT(""), SelectionHandleB, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_ToolTip_SelectionHandles")) );
	AddCheckTool( ID_UI_SHOWSELECTIONOUTLINE_PERWIDGET, TEXT(""), MultiSelectionOutlineB, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_ToolTip_SelectionOutlinePerWidget")) );
	AddCheckTool( ID_UITOOL_SHOWDOCKHANDLES, TEXT(""), ShowDockHandlesB, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_ToolTip_ShowDockHandles")) );
	AddSeparator();
	
	{
		//@fixme - localize/use variable
		wxArrayString cmb_ViewportSizeStrings;
		cmb_ViewportSizeStrings.Add(TEXT("Auto"));
		cmb_ViewportSizeStrings.Add(TEXT("640x480"));
		cmb_ViewportSizeStrings.Add(TEXT("720x480"));
		cmb_ViewportSizeStrings.Add(TEXT("800x600"));
		cmb_ViewportSizeStrings.Add(TEXT("960x720"));
		cmb_ViewportSizeStrings.Add(TEXT("1024x768"));
		cmb_ViewportSizeStrings.Add(TEXT("1152x720"));
		cmb_ViewportSizeStrings.Add(TEXT("1280x720"));
		cmb_ViewportSizeStrings.Add(TEXT("1280x960"));
		cmb_ViewportSizeStrings.Add(TEXT("1280x1024"));
		cmb_ViewportSizeStrings.Add(TEXT("1600x1200"));
		cmb_ViewportSizeStrings.Add(TEXT("1680x1050"));
		cmb_ViewportSizeStrings.Add(TEXT("1920x1080"));
		cmb_ViewportSizeStrings.Add(TEXT("1920x1200"));

		cmb_ViewportSize = new WxComboBox( this, ID_UITOOL_VIEW_SIZE, TEXT("Auto"), wxDefaultPosition, wxDefaultSize, cmb_ViewportSizeStrings, wxCB_DROPDOWN );

		// initialize the virtual viewport size combo with the scene's viewport size
		FVector2D SceneViewportSize = UIEditor->OwnerScene->CurrentViewportSize;
		FString ViewportSizeString = FString::Printf(TEXT("%ix%i"), appTrunc(SceneViewportSize.X), appTrunc(SceneViewportSize.Y));
		INT SelectionIndex = cmb_ViewportSize->FindString(*ViewportSizeString);
		if ( SelectionIndex == wxNOT_FOUND )
		{
			// use auto
			SelectionIndex = 0;
		}
		else
		{
			UIEditor->EditorOptions->VirtualSizeX = SceneViewportSize.X;
			UIEditor->EditorOptions->VirtualSizeY = SceneViewportSize.Y;
		}

		cmb_ViewportSize->SetSelection(SelectionIndex);
		cmb_ViewportSize->SetHelpText(*LocalizeUI(TEXT("UIEditor_HelpText_ViewportSize")));
		cmb_ViewportSize->SetToolTip(*LocalizeUI(TEXT("UIEditor_ToolTip_ViewportSize")));
		AddControl(cmb_ViewportSize);

		spin_ViewportGutter = new wxSpinCtrl( this, ID_UITOOL_VIEW_GUTTER, TEXT("0"), wxDefaultPosition, wxSize(75, -1), wxSP_ARROW_KEYS, 0, 100, 10 );
		spin_ViewportGutter->SetHelpText(*LocalizeUI(TEXT("UIEditor_HelpText_ViewportGutter")));
		spin_ViewportGutter->SetToolTip(*LocalizeUI(TEXT("UIEditor_ToolTip_ViewportGutter")));
		AddControl(spin_ViewportGutter);
	}

	AddSeparator();

	// Center on selection
	{
		WxMaskedBitmap Bitmap;
		Bitmap.Load( TEXT("UI_CENTERONSELECTION") );

		AddTool(ID_UI_CENTERONSELECTION, *LocalizeUI("UIEditor_MenuItem_CenterOnSelection"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_MenuItem_CenterOnSelection"), *LocalizeUI("UIEditor_HelpText_CenterOnSelection"));
	}

	AddSeparator();
	
	// Align items
	{
		WxMaskedBitmap Bitmap;

		Bitmap.Load( TEXT("ALIGN_TOP") );
		AddTool(ID_UI_ALIGN_VIEWPORT_TOP, *LocalizeUI("UIEditor_ToolTip_AlignToViewport_Top"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_AlignToViewport_Top"), *LocalizeUI("UIEditor_HelpText_AlignToViewport_Top"));

		Bitmap.Load( TEXT("ALIGN_CENTER_VERTICAL") );
		AddTool(ID_UI_ALIGN_VIEWPORT_CENTER_VERTICALLY, *LocalizeUI("UIEditor_ToolTip_AlignToViewport_CenterVertically"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_AlignToViewport_CenterVertically"), *LocalizeUI("UIEditor_HelpText_AlignToViewport_CenterVertically"));

		Bitmap.Load( TEXT("ALIGN_BOTTOM") );
		AddTool(ID_UI_ALIGN_VIEWPORT_BOTTOM, *LocalizeUI("UIEditor_ToolTip_AlignToViewport_Bottom"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_AlignToViewport_Bottom"), *LocalizeUI("UIEditor_HelpText_AlignToViewport_Bottom"));

		Bitmap.Load( TEXT("ALIGN_LEFT") );
		AddTool(ID_UI_ALIGN_VIEWPORT_LEFT, *LocalizeUI("UIEditor_ToolTip_AlignToViewport_Left"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_AlignToViewport_Left"), *LocalizeUI("UIEditor_HelpText_AlignToViewport_Left"));

		Bitmap.Load( TEXT("ALIGN_CENTER_HORIZONTAL") );
		AddTool(ID_UI_ALIGN_VIEWPORT_CENTER_HORIZONTALLY, *LocalizeUI("UIEditor_ToolTip_AlignToViewport_CenterHorizontally"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_AlignToViewport_CenterHorizontally"), *LocalizeUI("UIEditor_HelpText_AlignToViewport_CenterHorizontally"));

		Bitmap.Load( TEXT("ALIGN_RIGHT") );
		AddTool(ID_UI_ALIGN_VIEWPORT_RIGHT, *LocalizeUI("UIEditor_ToolTip_AlignToViewport_Right"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_AlignToViewport_Right"), *LocalizeUI("UIEditor_HelpText_AlignToViewport_Right"));
	}

	AddSeparator();

	// Align Text items
	{
		WxMaskedBitmap Bitmap;

		Bitmap.Load( TEXT("TEXT_ALIGN_LEFT") );
		AddTool(ID_UI_TEXT_ALIGN_LEFT, *LocalizeUI("UIEditor_ToolTip_TextAlign_Left"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_TextAlign_Left"), *LocalizeUI("UIEditor_HelpText_TextAlign_Left"));

		Bitmap.Load( TEXT("TEXT_ALIGN_CENTER") );
		AddTool(ID_UI_TEXT_ALIGN_CENTER, *LocalizeUI("UIEditor_ToolTip_TextAlign_CenterHorizontally"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_TextAlign_Center"), *LocalizeUI("UIEditor_HelpText_TextAlign_Center"));

		Bitmap.Load( TEXT("TEXT_ALIGN_RIGHT") );
		AddTool(ID_UI_TEXT_ALIGN_RIGHT, *LocalizeUI("UIEditor_ToolTip_TextAlign_Right"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_TextAlign_Right"), *LocalizeUI("UIEditor_HelpText_TextAlign_Right"));

		AddSeparator();

		Bitmap.Load( TEXT("TEXT_ALIGN_TOP") );
		AddTool(ID_UI_TEXT_ALIGN_TOP, *LocalizeUI("UIEditor_ToolTip_TextAlign_Top"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_TextAlign_Top"), *LocalizeUI("UIEditor_HelpText_TextAlign_Top"));

		Bitmap.Load( TEXT("TEXT_ALIGN_MIDDLE") );
		AddTool(ID_UI_TEXT_ALIGN_MIDDLE, *LocalizeUI("UIEditor_ToolTip_TextAlign_CenterVertically"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_TextAlign_Middle"), *LocalizeUI("UIEditor_HelpText_TextAlign_Middle"));

		Bitmap.Load( TEXT("TEXT_ALIGN_BOTTOM") );
		AddTool(ID_UI_TEXT_ALIGN_BOTTOM, *LocalizeUI("UIEditor_ToolTip_TextAlign_Bottom"), Bitmap, wxNullBitmap, wxITEM_NORMAL, 
			*LocalizeUI("UIEditor_ToolTip_TextAlign_Bottom"), *LocalizeUI("UIEditor_HelpText_TextAlign_Bottom"));
	}

	AddSeparator();

	CreateToolModeButtons();

	AddSeparator();

	// Add widget preview state combo.
	cmb_WidgetPreviewState = new WxComboBox(this, ID_UITOOL_WIDGET_PREVIEW_STATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_SORT | wxCB_READONLY);
	if(cmb_WidgetPreviewState)
	{
		cmb_WidgetPreviewState->Disable();
		cmb_WidgetPreviewState->SetHelpText(*LocalizeUI(TEXT("UIEditor_HelpText_WidgetPreviewState")));
		cmb_WidgetPreviewState->SetToolTip(*LocalizeUI(TEXT("UIEditor_ToolTip_WidgetPreviewState")));
		AddControl(cmb_WidgetPreviewState);
	}

	// Add the platform preview combobox
	{
		// first, generate the list of available platforms
		wxArrayString PlatformStrings;

		wxString SelectedPlatform = *UIEditor->EditorOptions->CurrentPreviewPlatform;

		// if we have any console plugins, add them to the list of selectable platforms
		const INT NumSupportedConsoles = FConsoleSupportContainer::GetConsoleSupportContainer()->GetNumConsoleSupports();
		if ( NumSupportedConsoles > 0 )
		{
			// loop through all consoles (only support 20 consoles)
			INT ConsoleIndex = 0;
			for (FConsoleSupportIterator It; It && ConsoleIndex < 20; ++It, ConsoleIndex++)
			{
				PlatformStrings.Add(It->GetConsoleName());
			}
		}

		// make sure we have at least one platform
		if ( PlatformStrings.GetCount() == 0 )
		{
			PlatformStrings.Add(CONSOLESUPPORT_NAME_PC);
		}

		// make sure we have a valid platform selected
		if ( SelectedPlatform.Len() == 0 || PlatformStrings.Index(SelectedPlatform) == wxNOT_FOUND )
		{
			SelectedPlatform = CONSOLESUPPORT_NAME_PC;
		}

		cmb_PreviewPlatform = new WxComboBox(this, ID_UI_COMBO_CHOOSEPLATFORM, SelectedPlatform, wxDefaultPosition, wxDefaultSize, PlatformStrings, wxCB_DROPDOWN|wxCB_SORT|wxCB_READONLY);
		if ( cmb_PreviewPlatform )
		{
			cmb_PreviewPlatform->SetHelpText(*LocalizeUI(TEXT("UIEditor_HelpText_PreviewPlatform")));
			cmb_PreviewPlatform->SetToolTip(*LocalizeUI(TEXT("UIEditor_ToolTip_PreviewPlatform")));
			AddControl(cmb_PreviewPlatform);
		}
	}

	{
		wxArrayString SSPreviewModeStrings;

		static UEnum* SplitscreenLayoutEnum = FindField<UEnum>(UGameViewportClient::StaticClass(), TEXT("ESplitScreenType"));
		checkf(SplitscreenLayoutEnum,TEXT("Couldn't find ESplitScreenType enum in GameViewportClientClass!"));

		FString InitialSplitscreenModeString;
		for ( INT ValueIdx = 0; ValueIdx < SplitscreenLayoutEnum->NumEnums() - 1; ValueIdx++ )
		{
			FString EnumValueString = SplitscreenLayoutEnum->GetMetaData(TEXT("DisplayName"), ValueIdx);
			if ( EnumValueString.Len() == 0 )
			{
				EnumValueString = SplitscreenLayoutEnum->GetEnum(ValueIdx).ToString();
			}
			SSPreviewModeStrings.Add(*EnumValueString);

			if ( ValueIdx == UIEditor->EditorOptions->CurrentSSPreviewMode )
			{
				InitialSplitscreenModeString = EnumValueString;
			}
		}

		cmb_SSPreviewState = new WxComboBox(this, ID_UITOOL_SS_PREVIEW_MODE, *InitialSplitscreenModeString, wxDefaultPosition, wxDefaultSize, SSPreviewModeStrings, wxCB_DROPDOWN|wxCB_READONLY);
		cmb_SSPreviewState->SetHelpText(*LocalizeUI(TEXT("UIEditor_HelpText_SSPreviewMode")));
		cmb_SSPreviewState->SetToolTip(*LocalizeUI(TEXT("UIEditor_ToolTip_SSPreviewMode")));
		AddControl(cmb_SSPreviewState);
	}
}

/**
 * Handles the selected widget preview state activation.
 */
void WxUIEditorMainToolBar::SetWidgetPreviewStateSelection(INT SelectionIndex)
{
	if(cmb_WidgetPreviewState != NULL)
	{
		const FString StateName(cmb_WidgetPreviewState->GetString(SelectionIndex));
		TArray<UUIObject*> Selected;
		UIEditor->GetSelectedWidgets(Selected);
		for(INT Index = 0; Index < Selected.Num(); Index++)
		{
			UUIScreenObject* SelectedItem = Selected(Index);
			if(SelectedItem)
			{
				UUIState* State = NULL;
				const INT NumStates = SelectedItem->InactiveStates.Num();
				for ( INT StateIndex = 0; StateIndex < NumStates; StateIndex++ )
				{
					UUIState* StateNode = SelectedItem->InactiveStates(StateIndex);
					const FString Name = StateNode->GetClass()->GetDescription();
					if ( Name == StateName )
					{
						State = StateNode;
						break;
					}
				}

				if ( State != NULL )
				{
					SelectedItem->ActivatePreviewState(State);
				}
			}
		}
	}
}

/**
 * Re-populates the Widget Preview State combo with a common subset
 * of states valid for all currently selected widgets within the scene.
 */
void WxUIEditorMainToolBar::UpdateWidgetPreviewStates()
{
	if(cmb_WidgetPreviewState)
	{
		cmb_WidgetPreviewState->Clear();
	}

	TArray<UUIObject*> Selected;
	UIEditor->GetSelectedWidgets(Selected);

	const INT NumSelected = Selected.Num();
	if(NumSelected == 0)
	{
		cmb_WidgetPreviewState->Disable();
		return;
	}

	TArray<class UClass*> PotentialStates;
	TArray<class UClass*> CommonStates;
	INT StateCount = 0;

	UUIScreenObject* SelectedItem = Selected(0);
	if(SelectedItem)
	{
		const INT NumStates = SelectedItem->InactiveStates.Num();
		for(INT StateIndex = 0; StateIndex < NumStates; StateIndex++)
		{
			UUIState *State = SelectedItem->InactiveStates(StateIndex);
			if(State && !PotentialStates.ContainsItem(State->GetClass()))
			{
				PotentialStates.InsertItem(State->GetClass(), StateCount);
				StateCount++;
			}
		}
	}

	const INT NumPotentialStates = PotentialStates.Num();

	CommonStates = PotentialStates;

	for(INT Index = 0; Index < NumSelected; Index++)
	{
		UUIScreenObject* SelectedItem = Selected(Index);
		if(SelectedItem)
		{
			for(INT StateIndex = 0; StateIndex < NumPotentialStates; StateIndex++)
			{
				if(!SelectedItem->DefaultStates.ContainsItem(PotentialStates(StateIndex)))
				{
					CommonStates.RemoveItem(PotentialStates(StateIndex));
				}
			}
		}
	}

	cmb_WidgetPreviewState->Enable();

	for(INT StateIndex = 0; StateIndex < CommonStates.Num(); StateIndex++)
	{
		UClass *StateClass = CommonStates(StateIndex);
		cmb_WidgetPreviewState->Append(*StateClass->GetDescription(), SelectedItem);
	}

	UBOOL bSameClass = TRUE;
	UClass* CommonStateClass = NULL;
	UUIState* CommonState = Selected(0)->GetCurrentState();
	if(CommonState)
	{
		CommonStateClass = CommonState->GetClass();
	}

	for(INT Index = 1; Index < Selected.Num(); Index++)
	{
		UUIScreenObject* SelectedItem = Selected(Index);
		if(SelectedItem)
		{
			UUIState* CurrentState = SelectedItem->GetCurrentState();
			if(CurrentState)
			{
				if(CurrentState->GetClass() != CommonStateClass)
				{
					bSameClass = FALSE;
					break;
				}
			}
		}
	}

	if(bSameClass && CommonStateClass)
	{
		cmb_WidgetPreviewState->SetValue(*CommonStateClass->GetDescription());
	}
	else
	{
		cmb_WidgetPreviewState->SetValue(TEXT(""));
	}
}

/**
 * Creates the radio buttons for all of the various UI tool modes.
 */
void WxUIEditorMainToolBar::CreateToolModeButtons()
{

	// Create the "Select Arrow" tool button first because it is a special case.
	{
		WxMaskedBitmap WidgetBitmap;

		WidgetBitmap.Load( TEXT("UI_MODE_SELECT") );

		AddRadioTool(ID_UITOOL_SELECTION_MODE, *LocalizeUI(TEXT("UIEditor_Widget_SelectionTool")), WidgetBitmap, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_Widget_SelectionTool")),
			*LocalizeUI(TEXT("UIEditor_HelpText_SelectionTool")));
	}

	// Create the "Focus Chain" tool button first because it is a special case.
	{
		WxMaskedBitmap WidgetBitmap;

		WidgetBitmap.Load( TEXT("UI_MODE_FOCUSCHAIN") );

		AddRadioTool(ID_UITOOL_FOCUSCHAIN_MODE, *LocalizeUI(TEXT("UIEditor_Widget_FocusChain")), WidgetBitmap, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_Widget_FocusChain")),
			*LocalizeUI(TEXT("UIEditor_HelpText_FocusChain")));
	}

	// Create the "Edit Widget Mode" button
	{
		WxMaskedBitmap EditWidgetModeBitmap;
		EditWidgetModeBitmap.Load(TEXT("EditWidgetMode"));

		AddRadioTool(ID_UITOOL_EDITWIDGET_MODE, *LocalizeUI(TEXT("UIEditor_EditWidgetMode_Tool")), EditWidgetModeBitmap, wxNullBitmap, *LocalizeUI(TEXT("UIEditor_EditWidgetMode_Tool")),
			*LocalizeUI(TEXT("UIEditor_HelpText_EditWidgetMode")));
	}

	AddSeparator();

	// Dynamically generate widget toolbar buttons using the UIWidgetToolbarMaps property of UISceneManager.

	//@todo ronp - need to add a dynamic button that can be used for entering a tool mode which allows the user to
	// place instances of UI archetypes using the mouse
	INT WidgetToolButtonID = ID_UITOOL_CUSTOM_MODE_START;
	const INT MapCount = UIEditor->SceneManager->UIWidgetToolbarMaps.Num();
	const INT WidgetCount = UIEditor->SceneManager->UIWidgetResources.Num();

	for (INT MapIdx=0; MapIdx < MapCount; MapIdx++)
	{
		const FUIObjectToolbarMapping* Mapping = &UIEditor->SceneManager->UIWidgetToolbarMaps(MapIdx);

		// Loop through all of our widgets and try to find a match for this mapping.
		for( INT WidgetIdx = 0; WidgetIdx < WidgetCount; WidgetIdx++)
		{
			const FUIObjectResourceInfo* WidgetInfo = &UIEditor->SceneManager->UIWidgetResources(WidgetIdx);
			const UBOOL bStringMatch = (WidgetInfo->UIResource->GetClass()->GetPathName() == Mapping->WidgetClassName);

			// Found a match, add a radio tool using this widget's info.
			if( bStringMatch == TRUE)
			{
				WxMaskedBitmap WidgetBitmap;

				WidgetBitmap.Load(Mapping->IconName);

				const FString TooltipStr = LocalizeUI(*Mapping->ToolTip);
				const FString StatusBarStr = LocalizeUI(*Mapping->HelpText);

				AddRadioTool(WidgetToolButtonID, *TooltipStr, WidgetBitmap, wxNullBitmap, *TooltipStr, *StatusBarStr);

				// Add the widgets IDX to our ToolModes list.
				WidgetButtonResourceIDs.AddUniqueItem(WidgetIdx);

				WidgetToolButtonID++;
				break;
			}
		}
	}
}

/**
 * Loads the bmp/tga files and assigns them to the WxMaskedBitmaps that are used by this toolbar.
 */
void WxUIEditorMainToolBar::LoadButtonResources()
{
	ViewportOutlineB.Load(TEXT("UI_ShowViewportOutline"));
	ContainerOutlineB.Load(TEXT("UI_ShowContainerOutline"));
	SelectionOutlineB.Load(TEXT("UI_ShowSelectionOutline"));
	SelectionHandleB.Load(TEXT("UI_ShowSelectionHandle"));
	MultiSelectionOutlineB.Load(TEXT("UI_ShowMultiSelectOutline"));
	ShowDockHandlesB.Load(TEXT("UI_ShowDockHandles"));
}


/**
 * Gets the desired viewport size.
 *
 * @param	out_ViewportSizeX	the X size selected by the user
 * @param	out_ViewportSizeY	the Y size selected by the user
 *
 * @return	TRUE if the user selected an explicit size, FALSE if the viewport size should be the same size of the editor window.
 */
UBOOL WxUIEditorMainToolBar::GetViewportSize( INT& out_ViewportSizeX, INT& out_ViewportSizeY ) const
{
	UBOOL bResult = FALSE;
	if ( cmb_ViewportSize != NULL )
	{
		INT SelectedItem = cmb_ViewportSize->GetSelection();

		// item 0 is the "Auto" item, which indicates that the viewport should fill the window
		if ( SelectedItem > 0 )
		{
			FString StringValue = cmb_ViewportSize->GetString(SelectedItem).c_str();
			FString SizeXString, SizeYString;

			if ( StringValue.Split(TEXT("x"), &SizeXString, &SizeYString) )
			{
				out_ViewportSizeX = Max(appAtoi(*SizeXString), 640);
				out_ViewportSizeY = Max(appAtoi(*SizeYString), 480);
				bResult = TRUE;
			}
		}
		else
		{
			// the value of the viewport combobox is a user-entered value
			FString StringValue = cmb_ViewportSize->GetValue().c_str();
			FString SizeXString, SizeYString;

			if ( StringValue.Split(TEXT("x"), &SizeXString, &SizeYString) )
			{
				out_ViewportSizeX = Max(appAtoi(*SizeXString), 640);
				out_ViewportSizeY = Max(appAtoi(*SizeYString), 480);
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
* Gets the configured gutter size
*
* @return the value of the Gutter Size spin control
*/
INT	WxUIEditorMainToolBar::GetGutterSize() const
{
	return spin_ViewportGutter != NULL
		? spin_ViewportGutter->GetValue()
		: 10;	// default
}

/**
 * Changes the value of the gutter size spin control.  If the input value is out of range, it will be clamped.
 *
 * @param	NewGutterSize	a string representing an integer value between 0 and 100
 */
void WxUIEditorMainToolBar::SetGutterSizeValue( const FString& NewGutterSize )
{
	INT NewValue = Clamp(appAtoi(*NewGutterSize),0,100);
	spin_ViewportGutter->SetValue(NewValue);
}

/**
* Finds out which WxWidgets ToolBar button ID corresponds to the Widget with the ID passed in.
*
* @param InToolID		WxWidget's ID, we will map this to a widget id
* @param OutWidgetID	Stores the Widget ID, if we were able to find one.
* @return				Whether or not we could find the Widget's ID
*/
UBOOL WxUIEditorMainToolBar::GetWidgetIDFromToolID(INT InToolId, INT& OutWidgetID)
{
	INT ArrayIdx = (InToolId - ID_UITOOL_CUSTOM_MODE_START);

	// Make sure the array idx is within our array bounds.
	if(ArrayIdx < 0 || ArrayIdx >= WidgetButtonResourceIDs.Num())
	{
		return FALSE;
	}
	else
	{
		OutWidgetID = WidgetButtonResourceIDs(ArrayIdx);

		return TRUE;
	}
}

/**
 * Changes the currently selected item in the preview platform combobox.
 */
UBOOL WxUIEditorMainToolBar::SetPreviewPlatformSelection( const FString& NewPlatformName )
{
	UBOOL bResult = FALSE;

	if ( cmb_PreviewPlatform != NULL && NewPlatformName.Len() > 0 )
	{
		bResult = cmb_PreviewPlatform->SetStringSelection(*NewPlatformName);
	}

	return bResult;
}

/**
 * Returns the preview platform combo's currently selected item.
 */
FString WxUIEditorMainToolBar::GetCurrentPreviewPlatform() const
{
	FString Result;

	if ( cmb_PreviewPlatform != NULL )
	{
		Result = cmb_PreviewPlatform->GetValue().c_str();
	}

	return Result;
}

/* ==========================================================================================================
	WxUIEditorWidgetToolBar
========================================================================================================== */
IMPLEMENT_DYNAMIC_CLASS(WxUIEditorWidgetToolBar,WxUIEditorToolBar)

/** Destructor */
WxUIEditorWidgetToolBar::~WxUIEditorWidgetToolBar()
{
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 */
void WxUIEditorWidgetToolBar::Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditorWindow )
{
	WxUIEditorToolBar::Create(InParent, InID, wxTB_FLAT|wxTB_VERTICAL|wxSIMPLE_BORDER|wxCLIP_CHILDREN, InEditorWindow);
	CreateControls();
}

void WxUIEditorWidgetToolBar::CreateControls()
{
}

/* ==========================================================================================================
	WxUIPositionPanel
========================================================================================================== */

namespace
{
	static UBOOL FloatEquals(const FLOAT F1, const FLOAT F2, FLOAT Tolerance=KINDA_SMALL_NUMBER)
	{
		return Abs(F1-F2) < Tolerance;
	}
};


BEGIN_EVENT_TABLE(WxUIPositionPanel, wxPanel)
	EVT_COMBOBOX(IDC_UI_SCALETYPE_LEFT,		WxUIPositionPanel::OnComboScaleChanged)
	EVT_COMBOBOX(IDC_UI_SCALETYPE_RIGHT,		WxUIPositionPanel::OnComboScaleChanged)
	EVT_COMBOBOX(IDC_UI_SCALETYPE_TOP,		WxUIPositionPanel::OnComboScaleChanged)
	EVT_COMBOBOX(IDC_UI_SCALETYPE_BOTTOM,	WxUIPositionPanel::OnComboScaleChanged)

	EVT_TEXT_ENTER(IDT_UI_FACE_LEFT,		WxUIPositionPanel::OnTextEntered)
	EVT_TEXT_ENTER(IDT_UI_FACE_TOP,			WxUIPositionPanel::OnTextEntered)
	EVT_TEXT_ENTER(IDT_UI_FACE_RIGHT,		WxUIPositionPanel::OnTextEntered)
	EVT_TEXT_ENTER(IDT_UI_FACE_BOTTOM,		WxUIPositionPanel::OnTextEntered)
	EVT_TEXT_ENTER(IDT_UI_WIDTH,			WxUIPositionPanel::OnTextEntered)
	EVT_TEXT_ENTER(IDT_UI_HEIGHT,			WxUIPositionPanel::OnTextEntered)
END_EVENT_TABLE()

WxUIPositionPanel::WxUIPositionPanel(WxUIEditorBase* InParent) : 
wxScrolledWindow(InParent),
Parent(InParent)
{
	wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// Create a value/scale pair for each row.
		for(INT FaceIdx = 0; FaceIdx < 4; FaceIdx++)
		{
			FString FaceNameTag = FString::Printf(TEXT("UIEditor_FaceText[%i]"), FaceIdx);

			wxSizer* FaceSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				// Face Label
				wxStaticText *FaceLabel = new wxStaticText(this, wxID_ANY, *LocalizeUI(*FaceNameTag));
				{
					wxFont Font = FaceLabel->GetFont();
					Font.SetWeight(wxBOLD);
					FaceLabel->SetFont(Font);
					FaceLabel->SetMinSize(wxSize(50, FaceLabel->GetSize().y));
				}
				FaceSizer->Add(FaceLabel, 0, wxALIGN_CENTER_VERTICAL);

				// Scale Combo
				wxSizer* ScaleSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI("UIEditor_ScaleType"));
				{
					ComboScale[FaceIdx] = new wxComboBox(this, IDC_UI_SCALETYPE_LEFT+FaceIdx, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
					ScaleSizer->Add(ComboScale[FaceIdx], 0, wxEXPAND | wxALL, 2);
				}
				FaceSizer->Add(ScaleSizer, 0, wxEXPAND | wxRIGHT, 2);

				// Value Text
				wxSizer* ValueSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI("UIEditor_FaceValue"));
				{
					TextValue[FaceIdx] = new wxTextCtrl(this, IDT_UI_FACE_LEFT+FaceIdx, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC) );
					ValueSizer->Add(TextValue[FaceIdx], 0, wxEXPAND | wxALL, 2);
				}
				FaceSizer->Add(ValueSizer, 1, wxEXPAND | wxLEFT, 2);
			}
			MainSizer->Add(FaceSizer, 0, wxEXPAND | wxALL, 3);
		}

		// Create a horizontal divider
		{
			wxStaticLine* Separator = new wxStaticLine( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
			MainSizer->Add(Separator, 0, wxGROW|wxALL, 3);
		}

		// Width
		wxSizer* HSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxSizer* WidthSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				// Face Label
				wxStaticText *FaceLabel = new wxStaticText(this, wxID_ANY, *LocalizeUI("UIEditor_Width"));
				{
					wxFont Font = FaceLabel->GetFont();
					Font.SetWeight(wxBOLD);
					FaceLabel->SetFont(Font);
					FaceLabel->SetMinSize(wxSize(50, FaceLabel->GetSize().y));
				}
				WidthSizer->Add(FaceLabel, 0, wxALIGN_CENTER_VERTICAL);

				// Value Text
				wxSizer* ValueSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI("UIEditor_Value"));
				{
					TextWidth = new wxTextCtrl(this, IDT_UI_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC) );
					ValueSizer->Add(TextWidth, 0, wxEXPAND | wxALL, 2);
				}
				WidthSizer->Add(ValueSizer, 1, wxEXPAND | wxLEFT, 2);
			}
			HSizer->Add(WidthSizer, 1, wxEXPAND | wxRIGHT, 3);


			// Height
			wxSizer* HeightSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				// Face Label
				wxStaticText *FaceLabel = new wxStaticText(this, wxID_ANY, *LocalizeUI("UIEditor_Height"));
				{
					wxFont Font = FaceLabel->GetFont();
					Font.SetWeight(wxBOLD);
					FaceLabel->SetFont(Font);
					FaceLabel->SetMinSize(wxSize(50, FaceLabel->GetSize().y));
				}
				HeightSizer->Add(FaceLabel, 0, wxALIGN_CENTER_VERTICAL);

				// Value Text
				wxSizer* ValueSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI("UIEditor_Value"));
				{
					TextHeight = new wxTextCtrl(this, IDT_UI_HEIGHT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC) );
					ValueSizer->Add(TextHeight, 0, wxEXPAND | wxALL, 2);
				}
				HeightSizer->Add(ValueSizer, 1, wxEXPAND | wxLEFT, 2);
			}
			HSizer->Add(HeightSizer, 1, wxEXPAND | wxLEFT, 3);
		}
		MainSizer->Add(HSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 3);
	}
	SetSizer(MainSizer);

	// Set a scrollbar for this panel.
	EnableScrolling(false, true);
	SetScrollRate(0,1);
	
	// Build a list of possible scale type enum values.
	wxArrayString EnumTypes;
	static UEnum* WidgetFaceEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPositionEvalType"), TRUE);
	if ( WidgetFaceEnum != NULL )
	{
		for( INT EnumIdx=0; EnumIdx<WidgetFaceEnum->NumEnums() - 1; EnumIdx++ )
		{
			EnumTypes.Add(*WidgetFaceEnum->GetEnum(EnumIdx).ToString());
		}
	}


	for(INT FaceIdx = 0; FaceIdx < UIFACE_MAX; FaceIdx++)
	{
		ComboScale[FaceIdx]->Append(EnumTypes);
	}

	

}

/** 
 * Seralizes the array of objects that we are storing. 
 *
 * @param Ar	Archive to serialize to.
 */
void WxUIPositionPanel::Serialize( FArchive& Ar )
{
	Ar << Objects;
}

/** 
 * Sets a object to modify, replaces the existing objects we were modifying.
 *
 * @param	Object	New object to start modifying.
 */
void WxUIPositionPanel::SetObject(UUIScreenObject* Object)
{
	Objects.Empty();
	Objects.AddItem(Object);

	RefreshControls();
}

/** 
 * Sets a array of objects to modify, replaces the existing objects we were modifying.
 *
 * @param	Objects		New objects to start modifying.
 */
void WxUIPositionPanel::SetObjectArray(const TArray<UUIObject*> &ObjectsArray)
{
	Objects.Empty();

	for(INT ObjectIdx = 0; ObjectIdx < ObjectsArray.Num(); ObjectIdx++)
	{
		Objects.AddItem(ObjectsArray(ObjectIdx));
	}
	
	RefreshControls();
}

/** Refreshes the panel's values using the currently selected widgets. */
void WxUIPositionPanel::RefreshControls()
{
	if(Objects.Num() == 0)
	{
		Enable(false);		
	}
	else
	{
		Enable(true);

		// Read values for each of the widgets and diff them, if we have any items in our struct that have differing
		// values, then we blank out the text for that widget.
		for(INT FaceIdx = 0; FaceIdx < UIFACE_MAX; FaceIdx++)
		{
			EUIWidgetFace Face = (EUIWidgetFace)FaceIdx;

			UUIScreenObject* InitialObject = Objects(0);

			const FUIScreenValue_Bounds CommonPosition	= InitialObject->Position;
			const FLOAT CommonPositionValue				= CommonPosition.GetPositionValue(InitialObject, Face);
			const INT CommonPositionScale				= CommonPosition.GetScaleType(Face);

			UBOOL bScaleValuesMatch = TRUE;
			UBOOL bPositionValuesMatch = TRUE;
		
			UUIObject* InitialUIObject = Cast<UUIObject>(InitialObject);
			UBOOL bFaceDocked = InitialUIObject != NULL && InitialUIObject->DockTargets.IsDocked(Face);

			for ( INT ObjectIdx = 1; ObjectIdx < Objects.Num() && (bPositionValuesMatch || bScaleValuesMatch || !bFaceDocked); ObjectIdx++)
			{
				UUIScreenObject* Object = Objects(ObjectIdx);
				const FUIScreenValue_Bounds& Bounds = Object->Position;

				bPositionValuesMatch = bPositionValuesMatch && FloatEquals(Bounds.GetPositionValue(Object, Face), CommonPositionValue);
				bScaleValuesMatch = bScaleValuesMatch && Bounds.GetScaleType(Face) == CommonPositionScale;
				if ( !bFaceDocked )
				{
					UUIObject* UIObj = Cast<UUIObject>(Object);
					if ( UIObj != NULL && UIObj->DockTargets.IsDocked(Face) )
					{
						bFaceDocked = TRUE;
					}
				}
			}

			if( bPositionValuesMatch )
			{
				TextValue[FaceIdx]->SetValue( *FString::Printf(TEXT("%f"), CommonPositionValue) );
			}
			else
			{
				//@todo - change TextValue to be a WxUIText so it can display the multiple value string
				TextValue[FaceIdx]->SetValue( wxEmptyString );
			}

			if ( bScaleValuesMatch )
			{
				static UEnum* WidgetFaceEnum = FindObject<UEnum>(UUIRoot::StaticClass(), TEXT("EPositionEvalType"), TRUE);
				check(WidgetFaceEnum);

				ComboScale[FaceIdx]->SetValue( *WidgetFaceEnum->GetEnum(CommonPositionScale).ToString() );
			}
			else
			{
				//@todo - change ComboScale to be a WxUIChoice so it can display the multiple value string
				ComboScale[FaceIdx]->SetValue( wxEmptyString );
			}

			TextValue[FaceIdx]->Enable(!bFaceDocked);
			ComboScale[FaceIdx]->Enable(!bFaceDocked);
		}

		RefreshWidthAndHeight();
	}
}

/** Updates the width and height fields using the values of the faces. */
void WxUIPositionPanel::RefreshWidthAndHeight()
{
	// Make sure that the scene client viewport has been initialized and we have a object selected.
	const UUISceneClient* SceneClient = Parent->OwnerScene->SceneClient;

	if ( Objects.Num() > 0 && SceneClient != NULL && SceneClient->RenderViewport != NULL )
	{
		// Diff the width and height, if we get different values then blank out the text boxes.
		UUIScreenObject* InitialObject = Objects(0);
		const FLOAT CommonWidth = InitialObject->Position.GetBoundsExtent(InitialObject, UIORIENT_Horizontal, EVALPOS_PixelViewport);
		const FLOAT CommonHeight = InitialObject->Position.GetBoundsExtent(InitialObject, UIORIENT_Vertical, EVALPOS_PixelViewport);

		UBOOL bWidthValuesMatch = TRUE;
		UBOOL bHeightValuesMatch = TRUE;
		UBOOL bWidthLocked = FALSE;
		UBOOL bHeightLocked = FALSE;

		UUIObject* InitialUIObj = Cast<UUIObject>(InitialObject);
		if ( InitialUIObj != NULL )
		{
			bWidthLocked = InitialUIObj->DockTargets.IsDocked(UIFACE_Left) && InitialUIObj->DockTargets.IsDocked(UIFACE_Right);
			bHeightLocked = InitialUIObj->DockTargets.IsDocked(UIFACE_Top) && InitialUIObj->DockTargets.IsDocked(UIFACE_Bottom);
		}

		for(INT ObjectIdx = 1; ObjectIdx < Objects.Num() && (bWidthValuesMatch || bHeightValuesMatch || !bWidthLocked || !bHeightLocked); ObjectIdx++)
		{
			UUIScreenObject* Object = Objects(ObjectIdx);
			const FLOAT Width = Object->Position.GetBoundsExtent(Object, UIORIENT_Horizontal, EVALPOS_PixelViewport);
			const FLOAT Height = Object->Position.GetBoundsExtent(Object, UIORIENT_Vertical, EVALPOS_PixelViewport);

			bWidthValuesMatch = bWidthValuesMatch && FloatEquals(Width, CommonWidth);
			bHeightValuesMatch = bHeightValuesMatch && FloatEquals(Height, CommonHeight);

			if ( !bWidthLocked || !bHeightLocked )
			{
				UUIObject* UIObj = Cast<UUIObject>(InitialObject);
				if ( UIObj != NULL )
				{
					bWidthLocked = bWidthLocked || (UIObj->DockTargets.IsDocked(UIFACE_Left) && UIObj->DockTargets.IsDocked(UIFACE_Right));
					bHeightLocked = bHeightLocked || (UIObj->DockTargets.IsDocked(UIFACE_Top) && UIObj->DockTargets.IsDocked(UIFACE_Bottom));
				}
			}
		}

		if ( bWidthValuesMatch )
		{
			TextWidth->SetValue( *FString::Printf(TEXT("%f"), CommonWidth) );
		}
		else
		{
			TextWidth->SetValue( wxEmptyString );
		}

		if( bHeightValuesMatch )
		{
			TextHeight->SetValue( *FString::Printf(TEXT("%f"), CommonHeight) );
		}
		else
		{
			TextHeight->SetValue( wxEmptyString );
		}

		TextWidth->Enable(!bWidthLocked);
		TextHeight->Enable(!bHeightLocked);
	}
}

/** Applies the values of the panel to selected widgets. */
void WxUIPositionPanel::ApplyValuesToSelectedWidgets() const
{
	// Go through all faces and send any non-blank values to the objects we are modifying.
	FScopedTransaction Transaction(*LocalizeUI("TransResizeWidget"));
	UBOOL bObjectsChanged = FALSE;

	TArray<FScopedObjectStateChange> PositionChangeNotifiers;
	for ( INT ObjIndex = 0; ObjIndex < Objects.Num(); ObjIndex++ )
	{
		new(PositionChangeNotifiers) FScopedObjectStateChange(Objects(ObjIndex));
	}

	for(INT FaceIdx = 0; FaceIdx < UIFACE_MAX; FaceIdx++)
	{
		wxString Text;

		// Text Value
		Text = TextValue[FaceIdx]->GetValue();
		if(Text.Length())
		{
			bObjectsChanged = TRUE;
			FLOAT Value = appAtof(Text);

			for(INT ObjectIdx = 0; ObjectIdx < Objects.Num(); ObjectIdx++)
			{
				UUIScreenObject* Object = Objects(ObjectIdx);

				FUIScreenValue_Bounds& Bounds = Object->Position;
				Bounds.SetRawPositionValue(FaceIdx,Value);

				//@fixme ronp - this doesn't work correctly if width or height is locked; need to update RenderBounds directly in that case.
			}
		}


		// Scale Combo
		Text = ComboScale[FaceIdx]->GetValue();

		if(Text.Length())
		{
			bObjectsChanged = TRUE;
			INT ScaleType = ComboScale[FaceIdx]->GetSelection();

			for(INT ObjectIdx = 0; ObjectIdx < Objects.Num(); ObjectIdx++)
			{
				UUIScreenObject* Object = Objects(ObjectIdx);

				FUIScreenValue_Bounds& Bounds = Object->Position;
				Bounds.SetRawScaleType(FaceIdx, (EPositionEvalType)ScaleType);
			}
		}
	}

	// If we changed the objects, then we need to refresh their positions.  Since any widgets which are docked to these, either directly or indirectly,
	// may need to reformat strings/images, we'll request a global reformat as well.
	if( bObjectsChanged )
	{
		for( INT ObjectIdx = 0; ObjectIdx < Objects.Num(); ObjectIdx++ )
		{
			UUIScreenObject* Object = Objects(ObjectIdx);
			Object->RefreshPosition();
		}

		Parent->OwnerScene->RequestFormattingUpdate();
	}
	else
	{
		for ( INT ObjIndex = 0; ObjIndex < PositionChangeNotifiers.Num(); ObjIndex++ )
		{
			PositionChangeNotifiers(ObjIndex).CancelEdit();
		}

		Transaction.Cancel();
	}
}

/** Called when the user changes the scale type combo box. */
void WxUIPositionPanel::OnComboScaleChanged(wxCommandEvent& Event)
{
	INT FaceIdx = Event.GetId() - IDC_UI_SCALETYPE_LEFT;

	if(FaceIdx >=0 && FaceIdx < UIFACE_MAX)
	{
		INT ScaleType = Event.GetInt();

		FScopedTransaction Transaction(*LocalizeUI(TEXT("TransModifyScaleType")));

		// Loop through all modifable objects and set the new scale type.
		for(INT ObjectIdx = 0; ObjectIdx < Objects.Num(); ObjectIdx++)
		{
			UUIScreenObject* Object = Objects(ObjectIdx);

			FScopedObjectStateChange ScaleTypeChangeNotifier(Object);
			Object->Position.ChangeScaleType(Object, (EUIWidgetFace)FaceIdx, (EPositionEvalType)ScaleType);
		}
	}

	RefreshControls();
}

/** Called when the user presses the enter key on one of the textboxes. */
void WxUIPositionPanel::OnTextEntered(wxCommandEvent& Event)
{
	switch(Event.GetId())
	{
	case IDT_UI_FACE_LEFT:
	case IDT_UI_FACE_TOP:
	case IDT_UI_FACE_RIGHT:
	case IDT_UI_FACE_BOTTOM:
		ApplyValuesToSelectedWidgets();
		break;

	case IDT_UI_WIDTH:
		{
			FScopedTransaction Transaction(*LocalizeUI("TransResizeWidget"));

			// Loop through all objects and reposition their right face to have the widget have the correct width.
			wxString Str = TextWidth->GetValue();
			if(Str.Len())
			{
				FLOAT Width = appAtof(Str);
				for(INT ObjectIdx = 0; ObjectIdx < Objects.Num(); ObjectIdx++)
				{
					UUIScreenObject* Object = Objects(ObjectIdx);

					FScopedObjectStateChange PositionChangeNotifier(Object);
					//Object->Modify();


					UUIObject* UIObject = Cast<UUIObject>(Object);
					if ( UIObject != NULL && UIObject->DockTargets.IsDocked(UIFACE_Right) && !UIObject->DockTargets.IsDocked(UIFACE_Left) )
					{
						const FLOAT NewLeft = Object->GetPosition(UIFACE_Right,EVALPOS_PixelViewport) - Width;
						Object->SetPosition(NewLeft, UIFACE_Left, EVALPOS_PixelViewport);

						if ( UIObject->DockTargets.IsWidthLocked() )
						{
							// have to manually update the RenderBounds if width is locked, since the widget's extent will be used to
							// determine whether the left face should be
							UIObject->RenderBounds[UIFACE_Left] = NewLeft;
						}
					}
					else
					{
						const FLOAT Left = Object->GetPosition(UIFACE_Left, EVALPOS_PixelViewport, TRUE);
						const FLOAT NewRight = Left + Width;
						Object->SetPosition(NewRight, UIFACE_Right, EVALPOS_PixelViewport);
					}
				}
			}
		}
		break;

	case IDT_UI_HEIGHT:
		{
			FScopedTransaction Transaction(*LocalizeUI("TransResizeWidget"));

			// Loop through all objects and reposition their bottom face to have the widget have the correct height.
			wxString Str = TextHeight->GetValue();

			if(Str.Len())
			{
				FLOAT Height = appAtof(Str);
				for(INT ObjectIdx = 0; ObjectIdx < Objects.Num(); ObjectIdx++)
				{
					UUIScreenObject* Object = Objects(ObjectIdx);
					FScopedObjectStateChange PositionChangeNotifier(Object);
					//Object->Modify();

					UUIObject* UIObject = Cast<UUIObject>(Object);
					if ( UIObject != NULL && UIObject->DockTargets.IsDocked(UIFACE_Right) && !UIObject->DockTargets.IsDocked(UIFACE_Top) )
					{
						const FLOAT NewTop = Object->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) - Height;
						Object->SetPosition(NewTop, UIFACE_Top, EVALPOS_PixelViewport);

						if ( UIObject->DockTargets.IsHeightLocked() )
						{
							// have to manually update the RenderBounds if width is locked, since the widget's extent will be used to
							// determine whether the left face should be
							UIObject->RenderBounds[UIFACE_Top] = NewTop;
						}
					}
					else
					{
						const FLOAT Top = Object->GetPosition(UIFACE_Top, EVALPOS_PixelViewport, TRUE);
						const FLOAT NewBottom = Top + Height;
						Object->SetPosition(NewBottom, UIFACE_Bottom, EVALPOS_PixelViewport);
					}
				}
			}
		}
		break;
	default:
		break;
	}

	// any widgets which are docked (either directly or indirectly) to this one may need to reformat their strings/images
	Parent->OwnerScene->RequestFormattingUpdate();
	Parent->OwnerScene->RequestSceneUpdate(FALSE, TRUE);

	// Force a refresh of widget positions before populating our fields.
	Parent->OwnerScene->UpdateScene();

	RefreshControls();
}

/* ==========================================================================================================
	WxUIButton
========================================================================================================== */

/**
 *  Control unique handling of displaying multiple values 
 */
void WxUIButton::DisplayMultipleValues()
{
	SetBackgroundColour(wxColour(255,255,255));
	SetLabel(MultipleValuesString);
}

/**
 *  Clear the control changes done by the DisplayMultipleValues method 
 */
void WxUIButton::ClearMultipleValues()
{
	SetLabel(TEXT(""));
}

/* ==========================================================================================================
	WxUITextCtrl
========================================================================================================== */

/**
*  Control unique handling of displaying multiple values 
*/
void WxUITextCtrl::DisplayMultipleValues()
{
	SetValue(MultipleValuesString);
}

/**
*  Clear the control changes done by the DisplayMultipleValues method 
*/
void WxUITextCtrl::ClearMultipleValues()
{
	SetValue(TEXT(""));
}


/* ==========================================================================================================
	WxUIChoice
========================================================================================================== */

/**
*  Control unique handling of displaying multiple values 
*/
void WxUIChoice::DisplayMultipleValues()
{
	// clear existing multiple values items
	ClearMultipleValues();

	INT IndexValue = Append(MultipleValuesString);
	Select(IndexValue);
}

/**
*  Clear the control changes done by the DisplayMultipleValues method 
*/
void WxUIChoice::ClearMultipleValues()
{
	INT IndexValue = FindString(MultipleValuesString);
	if(IndexValue != wxNOT_FOUND)
	{
		Delete(IndexValue);
	}
}

/* ==========================================================================================================
	WxUIChoice
========================================================================================================== */
/**
 *   Control unique handling of displaying multiple values 
 */
void WxUISpinCtrl::DisplayMultipleValues()
{
	ValidatorStringValue = MultipleValuesString;

	wxTextValidator* TextValidator = wxDynamicCast(TextCtrl->GetValidator(),wxTextValidator);
	check(TextValidator);

	TextValidator->SetStyle(wxFILTER_NONE);
	TextCtrl->SetValue(ValidatorStringValue);
}

/**
 *  Clear the control changes done by the DisplayMultipleValues method 
 */
void WxUISpinCtrl::ClearMultipleValues()
{
	ValidatorStringValue.Clear();

	wxTextValidator* TextValidator = wxDynamicCast(TextCtrl->GetValidator(),wxTextValidator);
	check(TextValidator);

	TextValidator->SetStyle(wxFILTER_NUMERIC);
	TextCtrl->SetValue(ValidatorStringValue);
}

