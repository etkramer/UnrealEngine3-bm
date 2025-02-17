/*=============================================================================
	UILayerBrowser.cpp: UI Layer Browser window and control implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UILayerBrowser.h"

/* ==========================================================================================================
	WxUILayerTreeObjectWrapper
========================================================================================================== */

WxUILayerTreeObjectWrapper::WxUILayerTreeObjectWrapper(UObject* LayerObject, UUILayer* ParentLayer)
:	LayerParent(ParentLayer)
,	WxTreeObjectWrapper(LayerObject)
{

}

UBOOL WxUILayerTreeObjectWrapper::IsUILayerRoot()
{
	UBOOL bResult = FALSE;
	bResult = NULL != WxTreeObjectWrapper::GetObject<UUILayerRoot>();
	return bResult;
}

UBOOL WxUILayerTreeObjectWrapper::IsUILayer()
{
	UBOOL bResult = FALSE;
	bResult = WxTreeObjectWrapper::GetObject<UUILayer>() != NULL;
	return bResult;
}

UBOOL WxUILayerTreeObjectWrapper::IsUIObject()
{
	UBOOL bResult = FALSE;
	bResult = WxTreeObjectWrapper::GetObject<UUIObject>() != NULL;
	return bResult;
}

FUILayerNode* WxUILayerTreeObjectWrapper::GetObject()
{
	FUILayerNode* LayerNode = NULL;
	check(LayerParent);
	if(LayerParent)
	{
		LayerNode = LayerParent->FindChild(WxTreeObjectWrapper::GetObject<UObject>());
	}
	return LayerNode;
}

FString WxUILayerTreeObjectWrapper::GetObjectName()
{
	FString ObjectName;
	UUILayer* Layer = WxTreeObjectWrapper::GetObject<UUILayer>();
	if(Layer)
	{
		ObjectName = Layer->LayerName;
	}
	else
	{
		UUIObject* Widget = WxTreeObjectWrapper::GetObject<UUIObject>();
		if(Widget)
		{
			ObjectName = Widget->WidgetTag.ToString();
		}
	}
	return FString(ObjectName);
}

/* ==========================================================================================================
	WxUILayerTreeCtrl
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS (WxUILayerTreeCtrl, WxTreeCtrl)

BEGIN_EVENT_TABLE (WxUILayerTreeCtrl, WxTreeCtrl)
	EVT_TREE_BEGIN_DRAG(ID_UI_TREE_LAYER, WxUILayerTreeCtrl::OnBeginDrag)
	EVT_TREE_END_DRAG(ID_UI_TREE_LAYER, WxUILayerTreeCtrl::OnEndDrag)
	EVT_TREE_BEGIN_LABEL_EDIT(ID_UI_TREE_LAYER, WxUILayerTreeCtrl::OnBeginLabelEdit)
	EVT_TREE_END_LABEL_EDIT(ID_UI_TREE_LAYER, WxUILayerTreeCtrl::OnEndLabelEdit)
END_EVENT_TABLE()

inline DWORD GetTypeHash(FUILayerNode* Node)
{
	return PointerHash(Node);
}

/**
 * Constructor
 */
WxUILayerTreeCtrl::WxUILayerTreeCtrl()
:	UIEditor(NULL)
,	NumLayersCreated(0)
,	bEditingLabel(FALSE)
,	bInDrag(FALSE)
,	DragItemId()
{

}

/**
 * Destructor
 */
WxUILayerTreeCtrl::~WxUILayerTreeCtrl()
{

}

/**
 * Initialize this control.  Must be the first function called after creation.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 * @param	InEditor	pointer to the editor window that contains this control
 */
void WxUILayerTreeCtrl::Create(WxUIEditorBase* InEditor, wxWindow* InParent, wxWindowID InID)
{
	UIEditor = InEditor;

	const UBOOL bSuccess = WxTreeCtrl::Create
	(
		InParent,
		InID,
		new WxUILayerTreeContextMenu,
		wxTR_HAS_BUTTONS|wxTR_MULTIPLE|wxTR_LINES_AT_ROOT|wxTR_HIDE_ROOT|wxTR_EDIT_LABELS
	);

	check(bSuccess);
	if(bSuccess == FALSE)
	{
		return;
	}

	wxImageList *ImageList = new wxImageList(16, 15);
	check(ImageList);

	// These match order of UI_LAYER_ICON_ENUM
	const ANSICHAR *ImageNames[] = 
	{
		"UI_Layer_FolderOpen",
		"UI_Layer_FolderClosed",
		0
	};

	if(ImageList)
	{
		INT NameIdx;
		for(NameIdx = 0; ImageNames[NameIdx]; ++NameIdx)
		{
			ImageList->Add
			(
				WxBitmap(ImageNames[NameIdx]),
				wxColor(192, 192, 192)
			);
		}

		// Add icons for widgets.
		const INT MapCount = UIEditor->SceneManager->UIWidgetToolbarMaps.Num();
		INT IconIdx = NameIdx;
		for(INT MapIdx=0; MapIdx < MapCount; MapIdx++)
		{
			const FUIObjectToolbarMapping* Mapping = &UIEditor->SceneManager->UIWidgetToolbarMaps(MapIdx);
			ImageList->Add(WxBitmap(Mapping->IconName), wxColor (192, 192, 192));
			WidgetImageMap.Set(*Mapping->WidgetClassName, IconIdx);
			IconIdx++;
		}

		AssignImageList(ImageList);
	}
}

/**
 * Constructs a new UILayer object
 *
 * @param	LayerClass	the class to create an instance of; should be UILayer or a child class
 * @param	LayerOuter	the object to use as the Outer for the new layer
 * @param	LayerFlags	the object flags to use when creating the new layer
 * @param	LayerName	optional name to give the layer; usually only specified when creating a layer root.
 *
 * @return	a pointer to a new UILayer instance of the specified class.
 */
UUILayer* WxUILayerTreeCtrl::ConstructLayer( UClass* LayerClass, UObject* LayerOuter, FName LayerName/*=NAME_None*/, QWORD LayerFlags/*=0*/ ) const
{
	check(LayerClass);
	check(LayerClass->IsChildOf(UUILayer::StaticClass()));
	check(LayerOuter);

	// layers should never be marked RF_Public or RF_Standalone
	check((LayerFlags&(RF_Public|RF_Standalone)) == 0);

	// UILayers should never be loaded in the game
	LayerFlags |= RF_NotForClient|RF_NotForServer;

	return ConstructObject<UUILayer>( LayerClass, LayerOuter, LayerName, LayerFlags );
}

/**
 * Get the associated owner scene.
 *
 * @return	NULL if an owner scene could not be found
 */
UUIScene* WxUILayerTreeCtrl::GetOwnerScene() const
{
	UUIScene* Result = NULL;
	if(UIEditor != NULL && UIEditor->OwnerScene != NULL )
	{
		Result = UIEditor->OwnerScene;
	}
	return Result;
}

/**
 * Get the associated root layer from the tree control.
 *
 * @return	NULL if a root layer could not be found
 */
UUILayerRoot* WxUILayerTreeCtrl::GetLayerRootFromTree() const
{
	UUILayerRoot* LayerRoot = NULL;

	const wxTreeItemId RootItemId = GetRootItem();
	if(RootItemId.IsOk())
	{
		WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(RootItemId);
		if(ItemData)
		{
			LayerRoot = ItemData->GetLayerObject<UUILayerRoot>();
		}
	}

	return LayerRoot;
}

/**
 * Returns the UILayerRoot associated with the owner scene.
 *
 * @return	NULL if a root layer could not be found
 */
UUILayerRoot* WxUILayerTreeCtrl::GetLayerRootFromScene() const
{
	UUILayerRoot* LayerRoot = NULL;

	UUIScene* OwnerScene = GetOwnerScene();
	if( OwnerScene != NULL )
	{
		LayerRoot = Cast<UUILayerRoot>(OwnerScene->SceneLayerRoot);
	}

	return LayerRoot;
}

/**
 * Create the root layer.
 *
 * @return	NULL if a root layer could not be created
 */
UUILayerRoot* WxUILayerTreeCtrl::CreateLayerRoot() const
{
	UUILayerRoot* LayerRoot = NULL;

	UUIScene* OwnerScene = GetOwnerScene();
	if(OwnerScene)
	{
		LayerRoot = Cast<UUILayerRoot>(OwnerScene->SceneLayerRoot);
		if ( LayerRoot == NULL )
		{
			// The layer data is not required during gameplay, it is only needed within the editor
			LayerRoot = Cast<UUILayerRoot>(ConstructLayer(UUILayerRoot::StaticClass(), OwnerScene, TEXT("LayerRoot")));
			check(LayerRoot);

			OwnerScene->SceneLayerRoot = LayerRoot;
			LayerRoot->Scene = OwnerScene;
			LayerRoot->LayerName = TEXT("LayerRoot");

			// Create the default layer and populate it with all widgets currently placed within the OwnerScene
			const FString DefaultLayerName = FString::Printf(TEXT("%s 0"), *LocalizeUI(TEXT("UIEditor_LayerBrowser_DefaultLayerName")));
			UUILayer* DefaultLayer = ConstructLayer(UUILayer::StaticClass(), LayerRoot);

			check(DefaultLayer);
			if(DefaultLayer)
			{
 				DefaultLayer->LayerName = DefaultLayerName;
				FUILayerNode NewLayerNode(EC_EventParm);
				NewLayerNode.SetLayerObject(DefaultLayer);
				NewLayerNode.SetLayerParent(LayerRoot);
				LayerRoot->InsertNode(NewLayerNode);

				TArray<UUIObject*> Widgets;
				Widgets = LayerRoot->Scene->GetChildren(TRUE);
				for(INT i = 0; i < Widgets.Num(); i++)
				{
					// This widget is a member of another widget so it should not appear in the tree.
					// @fixme ronp - this needs to also check PRIVATE_TreeHiddenRecursive
					if( Widgets(i)->IsPrivateBehaviorSet(UCONST_PRIVATE_TreeHidden) )
						continue;

					FUILayerNode WidgetLayerNode(EC_EventParm);
					WidgetLayerNode.SetLayerObject(Widgets(i));
					WidgetLayerNode.SetLayerParent(DefaultLayer);
					DefaultLayer->InsertNode(WidgetLayerNode);
				}
			}
		}
	}

	return LayerRoot;
}

/**
 * Audit the root layer object contents.
 */
void WxUILayerTreeCtrl::AuditLayerRoot(class UUILayerRoot* LayerRoot) const
{
	if(LayerRoot)
	{
		// Audit the layer tree widget contents

		// Iterate through all Widgets within our LayerTree and verify
		// they exist within the OwnerScene and remove them if they don't.
		TArray<FUILayerNode*> LayerChildren;
		TArray<FUILayerNode*> MissingWidgetLayerNodes;
		if(LayerRoot->GetChildNodes(LayerChildren, TRUE))
		{
			for(INT i = 0; i < LayerChildren.Num(); i++)
			{
				FUILayerNode* LayerChild = LayerChildren(i);
				if(LayerChild)
				{
					UUIObject* LayerWidget = LayerChild->GetUIObject();
					if(LayerWidget)
					{
						// The widget is considered missing if it doesn't exist within the scene, or
						// it's private and somehow got into our layer tree (support for legacy data)
						// @fixme ronp - this needs to also check PRIVATE_TreeHiddenRecursive
						if( !LayerRoot->Scene->ContainsChild(LayerWidget) || LayerWidget->IsPrivateBehaviorSet(UCONST_PRIVATE_TreeHidden))
						{
							MissingWidgetLayerNodes.AddItem(LayerChild);
						}
					}
				}
			}

			for(INT i = 0; i < MissingWidgetLayerNodes.Num(); i++)
			{
				FUILayerNode* MissingWidget = MissingWidgetLayerNodes(i);
				if(MissingWidget)
				{
					UUILayer* ParentLayer = MissingWidget->GetParentLayer();
					if(ParentLayer)
					{
						ParentLayer->RemoveNode(*MissingWidget);
					}
				}
			}
		}

		// Iterate through all Widgets within our OwnerScene and verify
		// they exist within our LayerTree and add them if they don't exist.
		//
		// Widgets will be added to the LayerTree to the currently selected Layer.
		//
		UUILayer* SelectedLayer = LayerRoot->LayerNodes(0).GetUILayer();
		if(SelectedLayer == NULL)
		{
			return;
		}

		TArray<UUIObject*> Widgets;
		Widgets = LayerRoot->Scene->GetChildren(TRUE);
		for(INT i = 0; i < Widgets.Num(); i++)
		{
			UUIObject* CurrentWidget = Widgets(i);

			// This widget is a member of another widget so it should not appear in the tree.
			if( CurrentWidget->IsPrivateBehaviorSet(UCONST_PRIVATE_TreeHidden) )
				continue;

			if ( LayerRoot->FindChild(CurrentWidget, TRUE) == NULL )
			{
				FUILayerNode LayerNode(EC_EventParm);

				LayerNode.SetLayerObject(CurrentWidget);
				LayerNode.SetLayerParent(SelectedLayer);
				SelectedLayer->InsertNode(LayerNode);
			}
		}
	}
}

/**
 * Repopulates the tree control with the layers in the scene
 */
void WxUILayerTreeCtrl::RefreshTree()
{
	// Get the root layer node from the tree control
	UUILayerRoot* RootLayer = GetLayerRootFromTree();
	if(RootLayer == NULL)
	{
		// If our tree control does not yet have a root layer,
		// attempt to load the layer root data.
		RootLayer = GetLayerRootFromScene();
		if(RootLayer == NULL)
		{
			return;
		}
	}

	SaveSelectionExpansionState();

	// Remove all existing items
	DeleteAllItems();
	LayerIDMap.Empty();

	// Add the RootLayer as the tree root (it will not be drawn, however [wxTR_HIDE_ROOT])
	const wxTreeItemId RootID = AddRoot
	(
		*RootLayer->LayerName,
		-1,
		-1,
		new WxUILayerTreeObjectWrapper(RootLayer, RootLayer)
	);

	check(RootID.IsOk() == TRUE);
	if(RootID.IsOk())
	{
		AddChildren(RootLayer, RootID);
		RestoreSelectionExpansionState();
		if(NumLayersCreated == 0)
		{
			NumLayersCreated = GetLayerCount();
		}
	}
}

/**
 * Recursively adds all children of the specified parent to the tree control.
 *
 * @param	ParentLayer	the layer containing the children that should be added
 * @param	ParentId	the tree id for the specified parent
 */
void WxUILayerTreeCtrl::AddChildren(UUILayer* ParentLayer, wxTreeItemId ParentId)
{
	check(ParentLayer);
	if(ParentLayer == NULL)
	{
		return;
	}

	TArray<FUILayerNode*> LayerChildren;
	if(ParentLayer->GetChildNodes(LayerChildren))
	{
		// Iterate through the children in the parent, adding them to the tree
		for(INT i = 0; i < LayerChildren.Num(); i++)
		{
			// Determine the Layer Object type and append the tree item,
			// UILayer and UIObject are the only valid types.
			FUILayerNode* LayerChild = LayerChildren(i);
			check(LayerChild);
			if(LayerChild == NULL)
			{
				continue;
			}

			UUILayer* SubLayer = LayerChild->GetUILayer();
			if(SubLayer)
			{
				const FString& NameString = SubLayer->LayerName;
				const wxTreeItemId ItemId = AppendItem
				(
					ParentId,
					*NameString,
					UI_LAYER_ICON_FOLDER_CLOSED,
					UI_LAYER_ICON_FOLDER_CLOSED,
					new WxUILayerTreeObjectWrapper(SubLayer, ParentLayer)
				);

				SetItemImage(ItemId, UI_LAYER_ICON_FOLDER_OPEN, wxTreeItemIcon_Expanded);

				LayerIDMap.Set(LayerChild, ItemId);

				AddChildren(SubLayer, ItemId);
			}
			else
			{
				UUIObject* LayerWidget = LayerChild->GetUIObject();
				if(LayerWidget == NULL)
				{
					continue;
				}

				// See if we can find a icon for this widget.
				INT ItemImage = 10; // use 10 as a default widget icon image id, jic we place a widget that doesn't exist in the toolbar code.
				INT* ImageId = WidgetImageMap.Find(LayerWidget->GetClass()->GetPathName());
				if(ImageId)
				{
					ItemImage = *ImageId;
				}

				const wxTreeItemId ItemId = AppendItem
				(
					ParentId,
					*LayerWidget->GetTag().ToString(),
					ItemImage,
					ItemImage,
					new WxUILayerTreeObjectWrapper(LayerWidget, ParentLayer)
				);

				LayerIDMap.Set(LayerChild, ItemId);
			}
		}
	}
}

/**
 * Method for retrieving the selected items, with the option of ignoring children of selected items.
 *
 * @param	Selections					array of wxTreeItemIds corresponding to the selected items
 * @param	bIgnoreChildrenOfSelected	if TRUE, selected items are only added to the array if the parent item is not selected
 */
void WxUILayerTreeCtrl::GetSelections(TArray<wxTreeItemId>& Selections, UBOOL bIgnoreChildrenOfSelected/*=FALSE*/)
{
	for(LayerTreeIterator It = TreeIterator(); It; ++It)
	{
		wxTreeItemId ItemId = It.GetId();
		if(IsSelected(ItemId))
		{
			Selections.AddItem(ItemId);
			if(bIgnoreChildrenOfSelected)
			{
				It.Skip();
			}
		}
	}
}

/**
 * Retrieves the layers associated with the currently selected items
 */
void WxUILayerTreeCtrl::GetSelectedLayers(TArray<FUILayerNode*>& SelectedLayers)
{
	TArray<wxTreeItemId> SelectedItems;
	WxUILayerTreeCtrl::GetSelections(SelectedItems);

	for(INT i = 0; i < SelectedItems.Num(); i++)
	{
		const wxTreeItemId ItemId = SelectedItems(i);
		WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(ItemId);
		if(ItemData)
		{
			if(ItemData->IsUILayer())
			{
				SelectedLayers.AddItem(ItemData->GetObject());
			}
		}
	}
}

/**
 * Retrieves the layer associated with the currently selected items
 */
void WxUILayerTreeCtrl::GetSelectedLayers(TArray<UUILayer*>& SelectedLayers)
{
	TArray<wxTreeItemId> SelectedItems;
	WxUILayerTreeCtrl::GetSelections(SelectedItems);

	for(INT i = 0; i < SelectedItems.Num(); i++)
	{
		const wxTreeItemId ItemId = SelectedItems(i);
		WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(ItemId);
		if(ItemData)
		{
			if(ItemData->IsUILayer())
			{
				SelectedLayers.AddItem(ItemData->GetLayerObject<UUILayer>());
			}
		}
	}
}

/**
 * Adds a new layer sibling to the currently selected layer within the tree control.
 */
void WxUILayerTreeCtrl::InsertLayer()
{
	TArray<FUILayerNode*> SelectedLayers;
	GetSelectedLayers(SelectedLayers);

	for(INT Index = 0; Index < SelectedLayers.Num(); Index++)
	{
		UUILayer* ParentLayer = SelectedLayers(Index)->GetParentLayer();
		if(ParentLayer)
		{
			INT SelectedLayerIndex = ParentLayer->FindNodeIndex(SelectedLayers(Index)->GetLayerObject());

			UUILayer* NewLayer = ConstructLayer(UUILayer::StaticClass(), ParentLayer);
			check(NewLayer);

			NewLayer->LayerName = CreateDefaultLayerName();
			FUILayerNode NewLayerNode(EC_EventParm);
			NewLayerNode.SetLayerObject(NewLayer);
			NewLayerNode.SetLayerParent(ParentLayer);
			ParentLayer->InsertNode(NewLayerNode, SelectedLayerIndex);
			RefreshTree();
		}
	}
}

/**
 * Adds a new child layer to the currently selected layer within the tree control.
 */
void WxUILayerTreeCtrl::InsertChildLayer()
{
	TArray<FUILayerNode*> SelectedLayers;
	GetSelectedLayers(SelectedLayers);

	for(INT Index = 0; Index < SelectedLayers.Num(); Index++)
	{
		UUILayer *SelectedLayer = SelectedLayers(Index)->GetUILayer();
		if(SelectedLayer)
		{
			UUILayer* NewLayer = ConstructLayer(UUILayer::StaticClass(), SelectedLayer);
			check(NewLayer);

			NewLayer->LayerName = CreateDefaultLayerName();
			FUILayerNode NewLayerNode(EC_EventParm);
			NewLayerNode.SetLayerObject(NewLayer);
			NewLayerNode.SetLayerParent(SelectedLayer);
			SelectedLayer->InsertNode(NewLayerNode, 0);
			RefreshTree();
		}
	}
}

/**
 * Selects all widgets within the currently selected layer within the tree control.
 */
void WxUILayerTreeCtrl::SelectAllWidgetsInLayer()
{
	TArray<FUILayerNode*> SelectedLayers;
	GetSelectedLayers(SelectedLayers);

	if(SelectedLayers.Num())
	{
		UnselectAll();
	}

	for(INT Index = 0; Index < SelectedLayers.Num(); Index++)
	{
		UUILayer *SelectedLayer = SelectedLayers(Index)->GetUILayer();
		if(SelectedLayer)
		{
			TArray<FUILayerNode*> LayerChildren;
			if(SelectedLayer->GetChildNodes(LayerChildren, TRUE))
			{
				for(INT i = 0; i < LayerChildren.Num(); i++)
				{
					if(LayerChildren(i)->IsUIObject())
					{
						const wxTreeItemId* ItemId = LayerIDMap.Find(LayerChildren(i));
						if(ItemId && ItemId->IsOk())
						{
							if(!IsSelected(*ItemId))
							{
								EnsureVisible(*ItemId);
								SelectItem(*ItemId);
							}
						}
					}
				}
			}
		}
	}
}

/**
 * Removes the currently selected layer from the tree control and re-parents any
 * children of the selected layer to the parent of the selected layer.
 */
void WxUILayerTreeCtrl::RemoveSelectedLayer()
{
	TArray<FUILayerNode*> SelectedLayers;
	GetSelectedLayers(SelectedLayers);

	for(INT Index = 0; Index < SelectedLayers.Num(); Index++)
	{
		UUILayer* SelectedLayerParent = SelectedLayers(Index)->GetParentLayer();
		if(SelectedLayerParent)
		{
			const INT SelectedLayerIndex = SelectedLayerParent->FindNodeIndex(SelectedLayers(Index)->GetLayerObject());
			if(SelectedLayerIndex != INDEX_NONE)
			{
				if(SelectedLayerParent->LayerNodes(SelectedLayerIndex).IsUILayer())
				{
					TArray<FUILayerNode*> LayerChildren;
					if(SelectedLayerParent->LayerNodes(SelectedLayerIndex).GetUILayer()->GetChildNodes(LayerChildren))
					{
						LayerIDMap.Remove(&SelectedLayerParent->LayerNodes(SelectedLayerIndex));
						SelectedLayerParent->LayerNodes.Remove(SelectedLayerIndex);

						UUILayer* NewParentLayer = SelectedLayerParent;
						if(Cast<UUILayerRoot>(SelectedLayerParent))
						{
							NewParentLayer = SelectedLayerParent->LayerNodes( SelectedLayerParent->LayerNodes.Num()-1 ).GetUILayer();
						}

						WxUILayerTreeObjectWrapper NewParentLayerObject(NewParentLayer, SelectedLayerParent);

						for(INT i = 0; i < LayerChildren.Num(); i++)
						{
							FUILayerNode* LayerChild = const_cast<FUILayerNode*>(LayerChildren(i));
							LayerChild->SetLayerParent(NewParentLayer);

							INT InsertIndex = INDEX_NONE;
							if(LayerChild->IsUILayer())
							{
								InsertIndex = 0;
							}

							NewParentLayer->InsertNode(*LayerChild, InsertIndex);
						}

						const wxTreeItemId* NewParentItemId = LayerIDMap.Find(NewParentLayerObject.GetObject());
						if(NewParentItemId && NewParentItemId->IsOk())
						{
							Expand(*NewParentItemId);
						}
						RefreshTree();
					}
					else
					{
						LayerIDMap.Remove(&SelectedLayerParent->LayerNodes(SelectedLayerIndex));
						SelectedLayerParent->LayerNodes.Remove(SelectedLayerIndex);
						RefreshTree();
					}
				}
			}
		}
	}
}

/**
 * Displays the rename layer dialog for the currently selected layers.
 */
void WxUILayerTreeCtrl::RenameSelectedLayers()
{
	UBOOL bRefresh = FALSE;
	
	// Try to rename any widgets that are selected.
	TArray<UUILayer*> SelectedLayers;
	GetSelectedLayers(SelectedLayers);

	if(SelectedLayers.Num() > 0)
	{
		bRefresh = DisplayRenameDialog(SelectedLayers);
	}

	if(bRefresh)
	{
		RefreshTree();
	}
}

/**
 * Tries to rename a UILayer, if unsuccessful, will return FALSE and display a error messagebox.
 *
 * @param Layer		Layer to rename.
 * @param NewName	New name for the object.
 *
 * @return	Returns whether or not the object was renamed successfully.
 */
UBOOL WxUILayerTreeCtrl::RenameLayer(UUILayer* Layer, const TCHAR* NewName)
{
	UBOOL bSuccess = FALSE;
	FName NewTag = NewName;
	checkSlow(Layer);
	if(NewTag == NAME_None)
	{
		wxMessageBox(*LocalizeUI(TEXT("DlgRenameLayer_Error_EmptyName")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);
	}
	else
	{
		Layer->LayerName = NewTag.ToString();
		bSuccess = TRUE;
	}

	return bSuccess;
}

/**
 * Displays the rename dialog for each of the layers specified.
 *
 * @param	Layers		the list of layers to rename
 *
 * @return	TRUE if at least one of the specified layers was renamed successfully
 */
UBOOL WxUILayerTreeCtrl::DisplayRenameDialog(TArray<UUILayer*> Layers)
{
	UBOOL bSuccess = FALSE;

	for(INT Index = 0; Index < Layers.Num(); Index++)
	{
		UUILayer* Layer = Layers(Index);
		wxString NameString = *Layer->LayerName;

		wxTextEntryDialog Dlg
		(
			this,
			*LocalizeUI(TEXT("DlgRenameLayer_Label")),
			*LocalizeUI(TEXT("DlgRenameLayer_Title")),
			*Layer->LayerName
		);

		if(Dlg.ShowModal() == wxID_OK)
		{
			bSuccess = RenameLayer(Layer, Dlg.GetValue().c_str());
		}
	}

	return bSuccess;
}

/**
 * Retrieves the current total number of layers.
 */
INT WxUILayerTreeCtrl::GetLayerCount() const
{
	UUILayerRoot* LayerRoot = GetLayerRootFromTree();
	if( LayerRoot == NULL )
	{
		return 0;
	}

	INT NumLayers = 0;

	UUILayer* ParentLayer = LayerRoot;
	TArray<FUILayerNode*> LayerNodes;
	if(ParentLayer->GetChildNodes(LayerNodes, TRUE))
	{
		for(INT i = 0; i < LayerNodes.Num(); i++)
		{
			FUILayerNode* LayerNode = LayerNodes(i);
			if(LayerNode)
			{
				if(LayerNode->IsUILayer())
				{
					NumLayers++;
				}
			}
		}
	}

	return NumLayers;
}

/**
 * Returns a string to use as the default LayerName for inserted layers.
 * This method will also increment the num layers created count.
 */
FString WxUILayerTreeCtrl::CreateDefaultLayerName()
{
	FString LayerName = FString::Printf(TEXT("%s %d"), *LocalizeUI(TEXT("UIEditor_LayerBrowser_DefaultLayerName")), NumLayersCreated++);
	return FString(LayerName);
}

/**
 * Synchronizes the selected widgets across all windows.
 * If source is the editor window, iterates through the tree control, making sure that the selection state of each tree
 * item matches the selection state of the corresponding widget in the editor window.
 *
 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
 *					used as the authoritative selection set.
 */
void WxUILayerTreeCtrl::SynchronizeSelectedWidgets(FSelectionInterface* Source)
{
	if(Source == this)
	{
		UIEditor->SynchronizeSelectedWidgets(this);
	}
	else if(Source == UIEditor)
	{
		TArray<UUIObject*> SelectedWidgets;
		UIEditor->GetSelectedWidgets(SelectedWidgets);
		SetSelectedWidgets(SelectedWidgets);
	}
}

/**
 * Changes the selection set to the widgets specified.
 *
 * @return	TRUE if the selection set was accepted
 */
UBOOL WxUILayerTreeCtrl::SetSelectedWidgets(TArray<UUIObject*> SelectionSet)
{
	UBOOL bResult = FALSE;
	for(LayerTreeIterator It = TreeIterator(); It; ++It)
	{
		UUIObject* Widget = It.GetLayerObject<UUIObject>();
		if(Widget)
		{
			UBOOL bIsSelected = SelectionSet.ContainsItem(Widget);
			const wxTreeItemId Id = It.GetId();

			SelectItem(Id, bIsSelected == TRUE);
			if(bIsSelected)
			{
				EnsureVisible(Id);
			}
			bResult = TRUE;
		}
		else
		{
			const wxTreeItemId Id = It.GetId();
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
void WxUILayerTreeCtrl::GetSelectedWidgets(TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected /*= FALSE*/)
{
	TArray<wxTreeItemId> SelectedItems;
	WxUILayerTreeCtrl::GetSelections(SelectedItems);

	for(INT i = 0; i < SelectedItems.Num(); i++)
	{
		const wxTreeItemId ItemId = SelectedItems(i);
		WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(ItemId);
		if(ItemData)
		{
			if(ItemData->IsUIObject())
			{
				OutWidgets.AddItem(ItemData->GetLayerObject<UUIObject>());
			}
		}
	}
}

/**
 * Callback for wxWindows RMB events. Default behaviour is to show the popup menu if a WxUILayerTreeObjectWrapper
 * is attached to the tree item that was clicked.
 */
void WxUILayerTreeCtrl::OnRightButtonDown(wxMouseEvent& In)
{
	if(ContextMenu)
	{
		ContextMenu->Enable(IDM_UI_INSERTLAYER, FALSE);
		ContextMenu->Enable(IDM_UI_INSERTCHILDLAYER, FALSE);
		ContextMenu->Enable(IDM_UI_RENAMELAYER, FALSE);
		ContextMenu->Enable(IDM_UI_SELECTALLWIDGETSINLAYER, FALSE);
		ContextMenu->Enable(IDM_UI_DELETELAYER, FALSE);

		TArray<FUILayerNode*> SelectedLayers;
		GetSelectedLayers(SelectedLayers);
		if(SelectedLayers.Num() == 1)
		{
			const wxTreeItemId TreeId = HitTest(wxPoint(In.GetX(), In.GetY()));
			if(TreeId.IsOk())
			{
				WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(TreeId);
				if(ItemData)
				{
					UBOOL bShowDelete = TRUE;
					FUILayerNode* LayerNode = ItemData->GetObject();
					UUILayer* ParentLayer = LayerNode->GetParentLayer();
					if(LayerNode->IsUIObject() || (Cast<UUILayerRoot>(ParentLayer) && ParentLayer->LayerNodes.Num() < 2))
					{
						bShowDelete = FALSE;
					}

					if(LayerNode->IsUILayer())
					{
						ContextMenu->Enable(IDM_UI_INSERTLAYER, TRUE);
						ContextMenu->Enable(IDM_UI_INSERTCHILDLAYER, TRUE);
						ContextMenu->Enable(IDM_UI_RENAMELAYER, TRUE);
					}

					ContextMenu->Enable(IDM_UI_SELECTALLWIDGETSINLAYER, TRUE);

					if(bShowDelete)
					{
						ContextMenu->Enable(IDM_UI_DELETELAYER, TRUE);
					}
				}
			}
		}

		FTrackPopupMenu tpm(GetParent(), ContextMenu);
		tpm.Show();
	}
}

/**
 * Called when the user begins editing a label.
 */
void WxUILayerTreeCtrl::OnBeginLabelEdit(wxTreeEvent &Event)
{
	if(bEditingLabel == FALSE)
	{
		const wxTreeItemId TreeItem = Event.GetItem();
		if(TreeItem.IsOk())
		{
			WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(TreeItem);
			if(ItemData)
			{
				if(ItemData->IsUILayer())
				{
					bEditingLabel = TRUE;
					Event.Allow();
				}
			}
		}

		if(bEditingLabel == FALSE)
		{
			Event.Veto();
		}
	}
	else
	{
		Event.Veto();
	}
}

/**
 * Called when the user finishes editing a label.
 */
void WxUILayerTreeCtrl::OnEndLabelEdit(wxTreeEvent &Event)
{
	if(bEditingLabel)
	{
		UBOOL bAcceptChange = FALSE;
		const wxTreeItemId TreeItem = Event.GetItem();
		if(TreeItem.IsOk())
		{
			WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)GetItemData(TreeItem);
			if(ItemData)
			{
				const FString NewName = FString(Event.GetLabel());
				if(NewName.Len() > 0)
				{
					UUILayer* Layer = ItemData->GetLayerObject<UUILayer>();
					if(Layer)
					{
						Layer->LayerName = NewName;
						bAcceptChange = TRUE;
					}
				}	
			}
		}

		// Cancel the edit if the user entered an invalid name
		if(bAcceptChange == FALSE)
		{
			Event.Veto();
		}

		bEditingLabel = FALSE;
	}
}

/**
 * Called when an item is dragged.
 */
void WxUILayerTreeCtrl::OnBeginDrag(wxTreeEvent& Event)
{
	DragItemId = Event.GetItem();
	if(bInDrag == FALSE && DragItemId.IsOk() && bEditingLabel == FALSE && UIEditor->ObjectVC->Viewport->KeyState(KEY_LeftMouseButton))
	{
		WxUILayerTreeObjectWrapper* SourceItemData = (WxUILayerTreeObjectWrapper*)GetItemData(DragItemId);
		if(SourceItemData)
		{
			bInDrag = TRUE;
			Event.Allow();
		}
	}
}

/**
 * Called when an item is done being dragged.
 */
void WxUILayerTreeCtrl::OnEndDrag(wxTreeEvent& Event)
{
	if(bInDrag == TRUE)
	{
		bInDrag = FALSE;

		UBOOL bSuccess = FALSE;

		// Reparent the widget or layer.
		const wxTreeItemId DropItemId = Event.GetItem();
		if(DropItemId.IsOk() && DragItemId.IsOk() && DropItemId != DragItemId)
		{
			UBOOL bReparentItem = TRUE;
			WxUILayerTreeObjectWrapper* SourceItemData = (WxUILayerTreeObjectWrapper*)GetItemData(DragItemId);
			WxUILayerTreeObjectWrapper* DropItemData = (WxUILayerTreeObjectWrapper*)GetItemData(DropItemId);
			if(SourceItemData && DropItemData)
			{
				INT DropIndex = INDEX_NONE;
				FUILayerNode* SourceLayerNode = const_cast<FUILayerNode*>(SourceItemData->GetObject());
				FUILayerNode* DropLayerNode = DropItemData->GetObject();
				if(SourceLayerNode && DropLayerNode)
				{
					UUILayer* SourceParentLayer = SourceLayerNode->GetParentLayer();
					UUILayer* DropParentLayer = DropLayerNode->GetParentLayer();
					if(SourceParentLayer && DropParentLayer)
					{
						if(DropLayerNode->IsUIObject())
						{
							return;
						}

						if(DropLayerNode->IsUILayer())
						{
							if(DropLayerNode->GetUILayer() == SourceParentLayer)
							{
								return;
							}
						}

						if(SourceLayerNode->IsUILayer())
						{
							UUILayer* SourceLayer = SourceLayerNode->GetUILayer();
							if(SourceLayer->ContainsChild(DropLayerNode->GetUILayer()->LayerName, TRUE))
							{
								return;
							}
						}

						INT InsertIndex = INDEX_NONE;
						if(SourceLayerNode->IsUILayer())
						{
							InsertIndex = 0;
						}

						UUILayer* NewParent = DropParentLayer;
						if(DropLayerNode->IsUILayer())
						{
							NewParent = DropLayerNode->GetUILayer();
						}

						SourceLayerNode->SetLayerParent(NewParent);
						NewParent->InsertNode(*SourceLayerNode, InsertIndex);
						SourceParentLayer->RemoveNode(*SourceLayerNode);
						bSuccess = TRUE;
					}
				}
			}
		}

		if(bSuccess == TRUE)
		{
			RefreshTree();
			Expand(DropItemId);
		}
	}
}

/** 
 * Loops through all of the elements of the tree and adds selected items to the selection set,
 * and expanded items to the expanded set.
 */
void WxUILayerTreeCtrl::SaveSelectionExpansionState()
{
	SavedSelections.Empty();
	SavedExpansions.Empty();

	// Recursively traverse and tree and store selection states based on the client data.
	wxTreeItemId Root = GetRootItem();
	if(Root.IsOk())
	{
		SaveSelectionExpansionStateRecurse(Root);
	}
}

/** 
 * Loops through all of the elements of the tree and sees if the client data of the item is in the 
 * selection or expansion set, and modifies the item accordingly.
 */
void WxUILayerTreeCtrl::RestoreSelectionExpansionState()
{
	// Recursively traverse and tree and restore selection states based on the client data.
	Freeze();
	{
		wxTreeItemId Root = GetRootItem();
		if(Root.IsOk())
		{
			RestoreSelectionExpansionStateRecurse(Root);
		}
	}
	Thaw();

	SavedSelections.Empty();
	SavedExpansions.Empty();
}


/** 
 * Recursion function that loops through all of the elements of the tree item provided and saves their select/expand state. 
 * 
 * @param Item Item to use for the root of this recursion.
 */
void WxUILayerTreeCtrl::SaveSelectionExpansionStateRecurse(wxTreeItemId& Item)
{
	const UBOOL bIsRoot = (Item == GetRootItem());
	const UBOOL bVirtualRoot = ((GetWindowStyle() & wxTR_HIDE_ROOT) == wxTR_HIDE_ROOT);
	const UBOOL bProcessItem = (bIsRoot == FALSE) || (bVirtualRoot == FALSE);

	if( bProcessItem )
	{
		// Expand and select this item
		WxUILayerTreeObjectWrapper* ObjectWrapper = static_cast<WxUILayerTreeObjectWrapper*>(GetItemData(Item));
		if(ObjectWrapper != NULL)
		{
			FUILayerNode* ObjectPointer = ObjectWrapper->GetObject();
			if(IsSelected(Item))
			{
				SavedSelections.Set(ObjectPointer, ObjectPointer);
			}

			if(IsExpanded(Item))
			{
				SavedExpansions.Set(ObjectPointer, ObjectPointer);
			}
		}
	}


	// Loop through all of the item's children and store their state.
	wxTreeItemIdValue Cookie;
	wxTreeItemId ChildItem = GetFirstChild(Item, Cookie);
	while(ChildItem.IsOk())
	{
		SaveSelectionExpansionStateRecurse(ChildItem);
		ChildItem = GetNextChild(Item, Cookie);
	}
}


/** 
 * Recursion function that loops through all of the elements of the tree item provided and restores their select/expand state. 
 * 
 * @param Item Item to use for the root of this recursion.
 */
void WxUILayerTreeCtrl::RestoreSelectionExpansionStateRecurse(wxTreeItemId& Item)
{
	const UBOOL bIsRoot = (Item == GetRootItem());
	const UBOOL bVirtualRoot = ((GetWindowStyle() & wxTR_HIDE_ROOT) == wxTR_HIDE_ROOT);
	const UBOOL bProcessItem = (bIsRoot == FALSE) || (bVirtualRoot == FALSE);

	if( bProcessItem )
	{
		// Expand and select this item
		WxUILayerTreeObjectWrapper* ObjectWrapper = static_cast<WxUILayerTreeObjectWrapper*>(GetItemData(Item));
		if(ObjectWrapper != NULL)
		{
			FUILayerNode* ObjectPointer = ObjectWrapper->GetObject();
			const UBOOL bItemSelected = SavedSelections.Find(ObjectPointer) != NULL;
			const UBOOL bItemExpanded = SavedExpansions.Find(ObjectPointer) != NULL;

			if(bItemSelected == TRUE)
			{
				SelectItem(Item);
			}

			if(bItemExpanded == TRUE)
			{
				Expand(Item);
			}
		}
	}

	// Loop through all of the item's children and select/expand them.
	wxTreeItemIdValue Cookie;
	wxTreeItemId ChildItem = GetFirstChild(Item, Cookie);

	while(ChildItem.IsOk())
	{
		RestoreSelectionExpansionStateRecurse(ChildItem);
		ChildItem = GetNextChild(Item, Cookie);
	}
}

/* ==========================================================================================================
	WxUILayerBrowser
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUILayerBrowser,wxPanel)

BEGIN_EVENT_TABLE(WxUILayerBrowser,wxPanel)
	EVT_PAINT(WxUILayerBrowser::OnPaint)
	EVT_SIZE(WxUILayerBrowser::OnSize)
	EVT_MENU(IDM_UI_INSERTLAYER, WxUILayerBrowser::OnContext_InsertLayer)
	EVT_MENU(IDM_UI_INSERTCHILDLAYER, WxUILayerBrowser::OnContext_InsertChildLayer)
	EVT_MENU(IDM_UI_RENAMELAYER, WxUILayerBrowser::OnContext_RenameLayer)
	EVT_MENU(IDM_UI_SELECTALLWIDGETSINLAYER, WxUILayerBrowser::OnContext_SelectAllWidgetsInLayer)
	EVT_MENU(IDM_UI_DELETELAYER, WxUILayerBrowser::OnContext_DeleteLayer)
END_EVENT_TABLE()

/** Constructor */
WxUILayerBrowser::WxUILayerBrowser()
:	UIEditor(NULL)
,	LayerTree(NULL)
{

}

/**
 * Initialize this panel and its children.  Must be the first function called after creating the panel.
 *
 * @param	InEditor	the ui editor that contains this panel
 * @param	InParent	the window that owns this panel (may be different from InEditor)
 * @param	InID		the ID to use for this panel
 */
void WxUILayerBrowser::Create(WxUIEditorBase* InEditor, wxWindow* InParent, wxWindowID InID)
{
	UIEditor = InEditor;

	const bool bSuccess = wxPanel::Create
	(
		InParent,
		InID,
		wxDefaultPosition,
		wxDefaultSize,
		wxCLIP_CHILDREN
	);

	check(bSuccess == TRUE);
	if(bSuccess == FALSE)
	{
		return;
	}

	CreateControls();

	InitializeLayers();
}

/**
 * Creates the controls and sizers for this dialog.
 */
void WxUILayerBrowser::CreateControls()
{
	wxBoxSizer* PanelVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(PanelVSizer);

	wxBoxSizer* LayerTreeSizer = new wxBoxSizer(wxHORIZONTAL);
	PanelVSizer->Add(LayerTreeSizer, 1, wxGROW|wxBOTTOM, 5);

	LayerTree = new WxUILayerTreeCtrl;
	LayerTree->Create(UIEditor, this, ID_UI_TREE_LAYER);
	LayerTreeSizer->Add(LayerTree, 1, wxGROW|wxALL, 2);

	wxSizer* Sizer = GetSizer();
	if(Sizer)
	{
		Sizer->Fit(this);
		Sizer->SetSizeHints(this);
		Centre();
	}
}

/**
 * Populates the Layer Tree
 */
void WxUILayerBrowser::InitializeLayers()
{
	check(LayerTree);
	if(LayerTree == NULL)
	{
		return;
	}

	UUILayerRoot* LayerRoot = LayerTree->GetLayerRootFromScene();

	// If no layer data yet exists for the OwnerScene then we create it here
	if(LayerRoot == NULL)
	{
		LayerRoot = LayerTree->CreateLayerRoot();
	}
	else
	{
		LayerTree->AuditLayerRoot(LayerRoot);
	}


	// Initialize the layer tree with the currently active scene
	LayerTree->RefreshTree();
}

/**
 * Refresh the referenced browser resources.
 */
void WxUILayerBrowser::Refresh()
{
	InitializeLayers();
}

/**
 * Synchronizes the selected widgets across all windows.
 *
 * If source is the editor window, iterates through the tree control, making sure that the selection state of each tree
 * item matches the selection state of the corresponding widget in the editor window.
 *
 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
 *					used as the authoritative selection set.
 */
void WxUILayerBrowser::SynchronizeSelectedWidgets(FSelectionInterface* Source)
{
	if(LayerTree)
	{
		LayerTree->SynchronizeSelectedWidgets(Source);
	}
}

void WxUILayerBrowser::OnPaint(wxPaintEvent& Event)
{
	wxPaintDC dc(this);

	// TODO: Add implementation
}

/** Sizing event handler. */
void WxUILayerBrowser::OnSize(wxSizeEvent& Event)
{
	Event.Skip();
}

/** Called when the user selects the "Insert Layer" context menu option */
void WxUILayerBrowser::OnContext_InsertLayer(wxCommandEvent& Event)
{
	UBOOL bSkip = TRUE;
	check(LayerTree);
	if(LayerTree)
	{
		LayerTree->InsertLayer();
		bSkip = FALSE;
	}

	if(bSkip)
	{
		Event.Skip();
	}
}

/** Called when the user selects the "Insert Child Layer" context menu option */
void WxUILayerBrowser::OnContext_InsertChildLayer(wxCommandEvent& Event)
{
	UBOOL bSkip = TRUE;
	check(LayerTree);
	if(LayerTree)
	{
		LayerTree->InsertChildLayer();
		bSkip = FALSE;
	}

	if(bSkip)
	{
		Event.Skip();
	}
}

/** Called when the user selects the "Rename Layer" context menu option */
void WxUILayerBrowser::OnContext_RenameLayer(wxCommandEvent& Event)
{
	UBOOL bSkip = TRUE;
	check(LayerTree);
	if(LayerTree)
	{
		LayerTree->RenameSelectedLayers();
		bSkip = FALSE;
	}

	if(bSkip)
	{
		Event.Skip();
	}
}

/** Called when the user selects the "Select All Widgets In Layer" context menu option */
void WxUILayerBrowser::OnContext_SelectAllWidgetsInLayer(wxCommandEvent& Event)
{
	UBOOL bSkip = TRUE;
	check(LayerTree);
	if(LayerTree)
	{
		LayerTree->SelectAllWidgetsInLayer();
		UIEditor->SynchronizeSelectedWidgets(LayerTree);
		bSkip = FALSE;
	}

	if(bSkip)
	{
		Event.Skip();
	}
}

/** Called when the user selects the "Delete Layer" context menu option */
void WxUILayerBrowser::OnContext_DeleteLayer(wxCommandEvent& Event)
{
	UBOOL bSkip = TRUE;
	check(LayerTree);
	if(LayerTree)
	{
		LayerTree->RemoveSelectedLayer();
		bSkip = FALSE;
	}

	if(bSkip)
	{
		Event.Skip();
	}
}


