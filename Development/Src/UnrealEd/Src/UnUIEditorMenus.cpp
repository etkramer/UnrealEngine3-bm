/*=============================================================================
	UnUIEditorMenus.cpp: UI editor menu class implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIDataStoreBrowser.h"

#define SUPPORTS_NESTED_PREFABS 0

/* ==========================================================================================================
	WxUIMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIMenu,wxMenu)

//==========================================================================================================

/**
 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
 */
WxUIMenu::WxUIMenu( wxWindow* InOwner/*=NULL*/ )
: wxMenu()
, NextStyleMenuID(IDM_UI_STYLECLASS_BEGIN), NextEditStyleMenuID(IDM_UI_STYLEREF_BEGIN)
, NextSetDataStoreBindingMenuID(IDM_UI_SET_DATASTOREBINDING_START)
, NextClearDataStoreBindingMenuID(IDM_UI_CLEAR_DATASTOREBINDING_START)
{
	if ( InOwner != NULL )
	{
		SetInvokingWindow(InOwner);
	}
}


WxUIEditorBase* WxUIMenu::GetUIEditor()
{
	WxUIEditorBase* Result = NULL;
	for ( wxWindow* ParentWindow = GetInvokingWindow(); ParentWindow; ParentWindow = ParentWindow->GetParent() )
	{
		Result = wxDynamicCast(ParentWindow,WxUIEditorBase);
		if ( Result != NULL )
		{
			break;
		}
	}

	return Result;
}

/**
 * Retrieves the top-level context menu
 */
WxUIContextMenu* WxUIMenu::GetMainContextMenu()
{
	wxMenu* Result = this;
	while ( Result && Result->GetParent() )
	{
		Result = Result->GetParent();
	}

	return wxDynamicCast(Result,WxUIContextMenu);
}

/**
 * Retrieves the top-level context menu, asserting if it is not a WxUIMenu.
 */
WxUIContextMenu* WxUIMenu::GetMainContextMenuChecked()
{
	WxUIContextMenu* Result = GetMainContextMenu();
	check(Result);
	return Result;
}

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 */
void WxUIMenu::Create()
{
	UIEditor = GetUIEditor();
	if ( UIEditor != NULL )
	{
		UIEditor->GetSelectedWidgets(SelectedWidgets);
		UIEditor->GetSelectedWidgets(SelectedParents, TRUE);

		if ( GetParent() == NULL )
		{
			UIEditor->SelectStyleMenuMap.Empty();
		}
	}
}

/**
 * Returns the next ID available for use in style selection menus from the owner WxUIContextMenu.
 *
 * @return	the next menu id available for context menu items that contain options for changing the style assigned
 *			to a widget's style property; value will be between IDM_UI_STYLECLASS_BEGIN and IDM_UI_STYLECLASS_END if there
 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
 *			is not a WxUIContextMenu
 */
INT WxUIMenu::GetNextStyleSelectionMenuID()
{
	// only the top-level menu should track menu ids for style selection; otherwise we end up with non-unique style ids
	WxUIContextMenu* MainContextMenu = GetMainContextMenuChecked();

	INT Result = INDEX_NONE;
	if ( MainContextMenu != this )
	{
		Result = MainContextMenu->GetNextStyleSelectionMenuID();
	}
	else if ( NextStyleMenuID < IDM_UI_STYLECLASS_END )
	{
		Result = NextStyleMenuID++;
	}

	return Result;
}

/**
 * Returns the next ID available for use in edit style menus from the owner WxUIContextMenu.
 *
 * @return	the next menu id available for context menu items that contain options for editing the style assigned
 *			to a widget's style property; value will be between IDM_UI_STYLEREF_BEGIN and IDM_UI_STYLEREF_END if there
 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
 *			is not a WxUIContextMenu.
 */
INT WxUIMenu::GetNextStyleEditMenuID()
{
	// only the top-level menu should track menu ids for edit-style selection; otherwise we end up with non-unique style ids
	WxUIContextMenu* MainContextMenu = GetMainContextMenuChecked();

	INT Result = INDEX_NONE;
	if ( MainContextMenu != this )
	{
		Result = MainContextMenu->GetNextStyleEditMenuID();
	}
	else if ( NextEditStyleMenuID < IDM_UI_STYLEREF_END )
	{
		Result = NextEditStyleMenuID++;
	}

	return Result;
}


/**
 * Returns the next ID available for use in set data binding menus from the owner WxUIContextMenu.
 *
 * @return	the next menu id available for context menu items that correspond to data store binding properties.
 *			value will be between IDM_UI_SET_DATASTOREBINDING_START and IDM_UI_SET_DATASTOREBINDING_END if there
 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
 *			is not a WxUIContextMenu
 */
INT WxUIMenu::GetNextSetDataBindingMenuId()
{
	// only the top-level menu should track menu ids for style selection; otherwise we end up with non-unique style ids
	WxUIContextMenu* MainContextMenu = GetMainContextMenuChecked();

	INT Result = INDEX_NONE;
	if ( MainContextMenu != this )
	{
		Result = MainContextMenu->GetNextSetDataBindingMenuId();
	}
	else if ( NextSetDataStoreBindingMenuID <= IDM_UI_SET_DATASTOREBINDING_END )
	{
		Result = NextSetDataStoreBindingMenuID++;
	}

	return Result;
}

/**
 * Returns the next ID available for use in clear data binding menus from the owner WxUIContextMenu.
 *
 * @return	the next menu id available for context menu items that correspond to data store binding properties.
 *			value will be between IDM_UI_SET_DATASTOREBINDING_START and IDM_UI_SET_DATASTOREBINDING_END if there
 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
 *			is not a WxUIContextMenu
 */
INT WxUIMenu::GetNextClearDataBindingMenuId()
{
	// only the top-level menu should track menu ids for style selection; otherwise we end up with non-unique style ids
	WxUIContextMenu* MainContextMenu = GetMainContextMenuChecked();

	INT Result = INDEX_NONE;
	if ( MainContextMenu != this )
	{
		Result = MainContextMenu->GetNextSetDataBindingMenuId();
	}
	else if ( NextClearDataStoreBindingMenuID <= IDM_UI_CLEAR_DATASTOREBINDING_END )
	{
		Result = NextClearDataStoreBindingMenuID++;
	}

	return Result;
}

/**
 * Retrieves a list of styles which are valid to be assigned to the specified style references.
 *
 * @param	StyleReferences		the list of style references to find valid styles for
 * @param	out_ValidStyles		receives the list of styles that can be assigned to all of the style references specified.
 */
void WxUIMenu::GetValidStyles( const TArray<FUIStyleReference*>& StyleReferences, TArray<UUIStyle*>& out_ValidStyles )
{
	out_ValidStyles.Empty();

	GetUIEditor();
	if ( UIEditor != NULL && UIEditor->SceneManager != NULL && UIEditor->SceneManager->ActiveSkin != NULL )
	{
		// get a list of all available styles
		TArray<UUIStyle*> AvailableStyles;
		UIEditor->SceneManager->ActiveSkin->GetAvailableStyles(AvailableStyles);

		// now determine which of the styles can be assigned to ALL of the style references passed in
		for ( INT StyleIndex = 0; StyleIndex < AvailableStyles.Num(); StyleIndex++ )
		{
			UUIStyle* Style = AvailableStyles(StyleIndex);
			UBOOL bValidStyle = TRUE;

			for ( INT StyleReferenceIndex = 0; StyleReferenceIndex < StyleReferences.Num(); StyleReferenceIndex++ )
			{
				FUIStyleReference* StyleRef = StyleReferences(StyleReferenceIndex);
				if( StyleRef->IsValidStyle(Style) == FALSE )
				{
					bValidStyle = FALSE;
					break;
				}
			}

			if ( bValidStyle )
			{
				out_ValidStyles.AddItem(Style);
			}
		}
	}
}

IMPLEMENT_COMPARE_CONSTPOINTER(UUIStyle,UnUIEditorMenus,
{
	INT Result = 0;
	if ( A->StyleGroupName.Len() > 0 || B->StyleGroupName.Len() > 0 )
	{
		if ( A->StyleGroupName.Len() > 0 && B->StyleGroupName.Len() > 0 )
		{
			Result = appStricmp(*A->StyleGroupName, *B->StyleGroupName);
		}
		else if ( A->StyleGroupName.Len() > 0 )
		{
			Result = -1;
		}
		else
		{
			Result = 1;
		}
	}

	if ( Result == 0 )
	{
		Result = appStricmp(*A->GetStyleName(), *B->GetStyleName()); 
	}

	return Result;
})

/**
 * Generates a style selection submenu for each of the style references specified and appends them to the ParentMenu specified.
 *
 * @param	ParentMenu				the menu to append the submenus to
 * @param	StyleReferenceGroup		the style reference property to build the submenus for
 * @param	ExtraData			optional additional data to associate with the menu items generated; this value will be set as the ExtraData
 *								member in all FUIStyleMenuPairs that are created
 *
 * @return	TRUE if any menu items were appended
 */
UBOOL WxUIMenu::BuildStyleSelectionSubmenus( wxMenu* ParentMenu, const FStylePropertyReferenceGroup& StyleReferenceGroup, INT ExtraData/*=INDEX_NONE*/ )
{
	UBOOL bAppendedItems = FALSE;

	check(ParentMenu);

	// first, determine the maximum number of style reference values contained by all widgets in the group
	INT MaxStyleReferenceArrayCount=0;
	for ( INT WidgetIndex = 0; WidgetIndex < StyleReferenceGroup.WidgetStyleReferences.Num(); WidgetIndex++ )
	{
		const FWidgetStyleReference& WidgetStyleReference = StyleReferenceGroup.WidgetStyleReferences(WidgetIndex);
		MaxStyleReferenceArrayCount = Max(MaxStyleReferenceArrayCount, WidgetStyleReference.References.Num());
	}

	INT StyleMenuID;
	for ( INT ArrayIndex = 0; ArrayIndex < MaxStyleReferenceArrayCount; ArrayIndex++ )
	{
		TArray<FUIStyleReference*> StyleReferences;
		for ( INT SelectionIndex = 0; SelectionIndex < StyleReferenceGroup.WidgetStyleReferences.Num(); SelectionIndex++ )
		{
			const FWidgetStyleReference& WidgetReferences = StyleReferenceGroup.WidgetStyleReferences(SelectionIndex);
			if ( ArrayIndex < WidgetReferences.References.Num() )
			{
				StyleReferences.AddItem(WidgetReferences.References(ArrayIndex));
			}
		}

		// build the list of valid styles for the currently selected widgets
		// ValidStyles is the list of styles that are valid to be assigned to all of style references (may be none if different types of widgets are selected)
		TArray<UUIStyle*> ValidStyles;
		GetValidStyles(StyleReferences, ValidStyles);
		if ( ValidStyles.Num() )
		{
			Sort<USE_COMPARE_CONSTPOINTER(UUIStyle,UnUIEditorMenus)>(&ValidStyles(0), ValidStyles.Num());

			// the currently selected style (only valid if all selected widgets have the same style)
			UUIStyle* SelectedStyle=NULL;

			// if all widgets are currently using the same style, then we can "check" that style's item in the list
			for ( INT SelectionIndex = 0; SelectionIndex < StyleReferences.Num(); SelectionIndex++ )
			{
				FUIStyleReference* StyleReference = StyleReferences(SelectionIndex);

				// if this is the first time through the loop, SelectedStyle will be NULL...just set it to the style of the widget
				if ( SelectedStyle == NULL )
				{
					SelectedStyle = StyleReference->GetResolvedStyle();
				}
				else if ( SelectedStyle != StyleReference->GetResolvedStyle() )
				{
					// if the SelectedStyle is different from this widget's current style, clear the SelectedStyle and stop
					SelectedStyle = NULL;
					break;
				}
			}

			wxMenu* AvailableStylesMenu = new wxMenu;

			// create sub-menus'es per style group
			TMap<FString, wxMenu*>	SubMenusByGroup;
			for ( INT StyleIndex = 0; StyleIndex < ValidStyles.Num(); StyleIndex++ )
			{
				UUIStyle* Style = ValidStyles(StyleIndex);
				if ( Style->StyleGroupName.Len() > 0 && !SubMenusByGroup.HasKey(Style->StyleGroupName) )
				{
					wxMenu* menuItem = new wxMenu;
					AvailableStylesMenu->Append( 0, *FString::Printf(TEXT("[%s]"), *Style->StyleGroupName ), menuItem );
					SubMenusByGroup.Set( *Style->StyleGroupName, menuItem );
					bAppendedItems = TRUE;
				}
			}

			INT SelectedIndex = INDEX_NONE;
			for ( INT StyleIndex = 0; StyleIndex < ValidStyles.Num(); StyleIndex++ )
			{
				UUIStyle* Style = ValidStyles(StyleIndex);

				// Make sure we don't add more IDs than we have room for.
				StyleMenuID = GetNextStyleSelectionMenuID();
				checkf(StyleMenuID != INDEX_NONE,TEXT("No more ids available for style menu items - might need to increase the number reserved by IDM_UI_STYLECLASS_END"));

				wxMenu* ParentMenu = AvailableStylesMenu;
				if ( Style->StyleGroupName.Len() > 0 )
				{
					wxMenu** SubMenu = SubMenusByGroup.Find(Style->StyleGroupName);
					if ( SubMenu != NULL )
					{
						ParentMenu = *SubMenu;
					}
				}

				ParentMenu->AppendCheckItem( StyleMenuID, *Style->GetStyleName() );
				bAppendedItems = TRUE;

				//  Add this style to the editor's style menu map so it can quickly assign the style when it receives a menu callback.
				UIEditor->SelectStyleMenuMap.Set(StyleMenuID, WxUIEditorBase::FUIStyleMenuPair(StyleReferenceGroup.StylePropertyRef, ArrayIndex, Style, ExtraData));

				// if this style is the style currently assigned to all selected widgets, "check" it
				if ( SelectedStyle != NULL &&  SelectedStyle == Style )
				{
					SelectedIndex = StyleMenuID;
				}
			}

			if ( SelectedIndex != INDEX_NONE )
			{
				AvailableStylesMenu->Check(SelectedIndex,true);
			}

			if(ParentMenu == this)
			{
				// if we're here, it means that we only have one valid style property and one widget selected
				ParentMenu->Append( IDM_UI_SELECTSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_SelectStyle")), AvailableStylesMenu );
				bAppendedItems = TRUE;
			}
			else
			{
				// here means that we either have multiple widgets selected or multiple style properties that can be assigned
				FString PropertyName;
				if ( MaxStyleReferenceArrayCount > 1 )
				{
					FString ArrayIndexString;
					UProperty* Property = StyleReferenceGroup.StylePropertyRef.StyleProperty;
					if ( Property == NULL || Property->ArraySizeEnum == NULL )
					{
						ArrayIndexString = FString::Printf(TEXT("[%i]"), ArrayIndex );
					}
					else
					{
						ArrayIndexString = FString::Printf(TEXT("[%s]"), *Property->ArraySizeEnum->GetEnum(ArrayIndex).ToString());
					}
					PropertyName = FString::Printf(TEXT("%s %s"), *StyleReferenceGroup.StylePropertyRef.GetStyleReferenceName(), *ArrayIndexString);
				}
				else
				{
					PropertyName = StyleReferenceGroup.StylePropertyRef.GetStyleReferenceName();
				}

				ParentMenu->Append(IDM_UI_SELECTSTYLE, *PropertyName, AvailableStylesMenu);
				bAppendedItems = TRUE;
			}
		}
	}

	return bAppendedItems;
}

/**
 * Appends menu options that are used for manipulating widgets
 */
void WxUIMenu::AppendWidgetOptions()
{
	AppendScreenObjectOptions();
	AppendSeparator();

	AppendStyleOptions();
	// separator will be appended by AppendStyleOptions, if any menu items are actually appended

	AppendDataStoreMenu();
	AppendSeparator();

	AppendPositionConversionMenu();
	AppendAlignMenu();
	AppendLayerOptions();
	AppendSeparator();

	AppendObjectOptions();
	AppendSeparator();

	Append(IDM_UI_DUMPPROPERTIES, TEXT("Dump Property Values"), TEXT("(Debug)"));
}

/**
 * Appends menu items that let the user align selected objects to the viewport.
 */
void WxUIMenu::AppendAlignMenu()
{
	wxMenu* AlignMenu = new wxMenu();
	{
		AlignMenu->Append(ID_UI_ALIGN_VIEWPORT_TOP, *LocalizeUI("UIEditor_Align_Top"));
		AlignMenu->Append(ID_UI_ALIGN_VIEWPORT_BOTTOM, *LocalizeUI("UIEditor_Align_Bottom"));
		AlignMenu->Append(ID_UI_ALIGN_VIEWPORT_LEFT, *LocalizeUI("UIEditor_Align_Left"));
		AlignMenu->Append(ID_UI_ALIGN_VIEWPORT_RIGHT, *LocalizeUI("UIEditor_Align_Right"));
		AlignMenu->Append(ID_UI_ALIGN_VIEWPORT_CENTER_HORIZONTALLY, *LocalizeUI("UIEditor_Align_CenterHorizontally"));
		AlignMenu->Append(ID_UI_ALIGN_VIEWPORT_CENTER_VERTICALLY, *LocalizeUI("UIEditor_Align_CenterVertically"));
	}
	Append(wxID_ANY, *LocalizeUI("UIEditor_AlignToViewport"), AlignMenu);
}

IMPLEMENT_COMPARE_CONSTPOINTER(UProperty,UnUIEditorMenus,{ return appStricmp(*A->GetFriendlyName(),*B->GetFriendlyName()); });

/**
 * Appends menu options that are used for manipulating widgets
 */
void WxUIMenu::AppendDataStoreMenu()
{
	WxUIEditorBase* UIEditor = GetUIEditor();

	UIEditor->SetDataStoreBindingMap.Empty();
	UIEditor->ClearDataStoreBindingMap.Empty();

	UBOOL bNeedsGeneralClearBindingItem = TRUE;
	// only display the data store options if the data store browser has been opened.
// 	if ( SelectedWidgets.Num() > 0 )
// 	{
	UBOOL bDataStoreManagerInitialized = UIEditor->SceneManager->AreDataStoresInitialized();
	UBOOL bTagSelected = FALSE;
	
	EUIDataProviderFieldType DataFieldType = DATATYPE_MAX;
	FString BindMenuString;
	
	if ( bDataStoreManagerInitialized )
	{
		WxDlgUIDataStoreBrowser* DataStoreBrowser = UIEditor->SceneManager->GetDataStoreBrowser();

		bTagSelected = DataStoreBrowser->IsTagSelected();
		if ( bTagSelected )
		{
			DataFieldType = DataStoreBrowser->GetSelectedTagType();
			BindMenuString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_BindDataStore_F")), *DataStoreBrowser->GetSelectedTagPath()));
		}
		else
		{
			BindMenuString = LocalizeUI(TEXT("UIEditor_BindDataStore"));
		}
	}
	else
	{
		Append(IDM_UI_DATASTOREBROWSER, *LocalizeUI(TEXT("UIEditor_MenuItem_DataStoreBrowser")));
	}
	
	if ( SelectedWidgets.Num() > 0 )
	{
		// Loop through all widgets and build a list of possible data store bindings.  We take a intersection of the set of FUIDataStoreBinding properties
		// in each selected widget.  So we only display data bindings that are available in all of the selected widgets.
		UUIObject* BaseWidget = SelectedWidgets(0);

		// get the list of data binding properties from the first selected widget
		TArray<UProperty*> CommonDataStoreProperties;
		BaseWidget->GetDataBindingProperties(CommonDataStoreProperties);
		for ( INT PropertyIndex = CommonDataStoreProperties.Num() - 1; PropertyIndex >= 0; PropertyIndex-- )
		{
			UProperty* Property = CommonDataStoreProperties(PropertyIndex);
			if ( !BaseWidget->AllowDataBindingProperty(Property) )
			{
				// this data binding shouldn't be shown in the context menu
				CommonDataStoreProperties.Remove(PropertyIndex);
			}
		}

		for ( INT SelectionIndex = 1; SelectionIndex < SelectedWidgets.Num() && CommonDataStoreProperties.Num() > 0; SelectionIndex++ )
		{
			UUIObject* Widget = SelectedWidgets(SelectionIndex);
			TArray<UProperty*> DataStoreProperties;
			Widget->GetDataBindingProperties(DataStoreProperties);

			for ( INT PropertyIndex = CommonDataStoreProperties.Num() - 1; PropertyIndex >= 0; PropertyIndex-- )
			{
				UProperty* Property = CommonDataStoreProperties(PropertyIndex);
				if ( !DataStoreProperties.ContainsItem(Property) || !Widget->AllowDataBindingProperty(Property) )
				{
					// this widget doesn't contain this data store binding property, so remove it from the list of common bindings
					CommonDataStoreProperties.Remove(PropertyIndex);
				}
			}
		}

		//@fixme ronp - array support
		// if any of these properties are array properties, we'll need to display a submenu regardless
/*
		UBOOL bHasArrayProperty=FALSE;
		for ( INT i = 0; i < CommonDataStoreProperties.Num(); i++ )
		{
			if ( CommonDataStoreProperties(i)->ArrayDim > 1 
			||	CommonDataStoreProperties(i)->IsA(UArrayProperty::StaticClass()) )
			{
				bHasArrayProperty = TRUE;
				break;
			}
		}
*/

		// If we have more than 1 valid data binding property we need to create a submenu which contains those properties.
		// If we have multiple widgets selected, then we should always show the intermediate menu so that we know exactly
		// which data binding will be modified.

		// Append the "Bind widget to data field" item.

		if ( /*bHasArrayStyleProperty || */CommonDataStoreProperties.Num() > 1
		||	(CommonDataStoreProperties.Num() > 0 && SelectedWidgets.Num() > 1) )
		{
			bNeedsGeneralClearBindingItem = FALSE;

			INT BindMenuId=0;
			wxMenu* BindParentMenu = bTagSelected ? new wxMenu : NULL;
			wxMenuItem* BindItem = bDataStoreManagerInitialized ? Append(IDM_UI_BINDWIDGETSTODATASTORE, *BindMenuString, BindParentMenu) : NULL;
			if ( BindItem != NULL )
			{
				BindItem->Enable(false);
			}

			wxMenu* ClearParentMenu = new wxMenu;	
			wxMenuItem* ClearItem = Append(IDM_UI_CLEARWIDGETSDATASTORE, *LocalizeUI(TEXT("UIEditor_ClearDataStore")), ClearParentMenu);
			INT ClearMenuId;

			Sort<USE_COMPARE_CONSTPOINTER(UProperty,UnUIEditorMenus)>( &CommonDataStoreProperties(0), CommonDataStoreProperties.Num() );

			// add an item for each data store property that has the exact same
			for ( INT PropertyIndex = 0; PropertyIndex < CommonDataStoreProperties.Num(); PropertyIndex++ )
			{
				UProperty* Property = CommonDataStoreProperties(PropertyIndex);
				FString PropertyName = Property->GetFriendlyName();

				//@fixme ronp - add support for data binding array properties
				if ( bTagSelected )
				{
					BindMenuId = GetNextSetDataBindingMenuId();
					checkf(BindMenuId != INDEX_NONE,TEXT("No more ids available for data binding menu items - might need to increase the number reserved by IDM_UI_SET_DATASTOREBINDING_END"));
					BindItem = BindParentMenu->Append(BindMenuId, *PropertyName);
				}

				ClearMenuId = GetNextClearDataBindingMenuId();
				checkf(ClearMenuId != INDEX_NONE,TEXT("No more ids available for clearing data binding menu items - might need to increase the number reserved by IDM_UI_CLEAR_DATASTOREBINDING_END"));
				ClearItem = ClearParentMenu->Append(ClearMenuId, *PropertyName);

				FDataBindingPropertyValueMap BindItemProperties;
				FDataBindingPropertyValueMap ClearItemProperties;

				// if this data store property isn't valid for at least one of the selected widgets, disable
				// the menu item
				UBOOL bFieldTypeSupported = FALSE;
				for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
				{
					UUIObject* BindableWidget = SelectedWidgets(SelectionIndex);
					if ( BindableWidget != NULL )
					{
						FWidgetDataBindingReference BindingRef(BindableWidget);
						FWidgetDataBindingReference ClearingRef(BindableWidget);

						TMultiMap<FName,FUIDataStoreBinding*> WidgetBindings;
						BindableWidget->GetDataBindings(WidgetBindings, Property->GetFName());
						for ( TMultiMap<FName,FUIDataStoreBinding*>::TIterator It(WidgetBindings); It; ++It )
						{
							FUIDataStoreBinding* Binding = It.Value();
							if ( bTagSelected && Binding->IsValidDataField(DataFieldType) )
							{
								BindingRef.Bindings.AddItem(Binding);
								bFieldTypeSupported = TRUE;
							}

							// if this data store property has a valid binding, allow it to be cleared
							if ( *(*Binding) )
							{
								ClearingRef.Bindings.AddItem(Binding);
							}
						}

						if ( BindingRef.Bindings.Num() > 0 )
						{
							BindItemProperties.Add(Property, BindingRef);
						}

						if ( ClearingRef.Bindings.Num() > 0 )
						{
							ClearItemProperties.Add(Property, ClearingRef);
						}
					}
				}

				if ( bFieldTypeSupported )
				{
					UIEditor->SetDataStoreBindingMap.Set(BindMenuId, BindItemProperties);
				}
				else if ( BindItem != NULL )
				{
					BindItem->Enable(false);
				}

				if ( ClearItemProperties.Num() > 0 )
				{
					UIEditor->ClearDataStoreBindingMap.Set(ClearMenuId, ClearItemProperties);
				}
				else
				{
					ClearItem->Enable(false);
				}
			}
		}
		else if ( bDataStoreManagerInitialized )
		{
			wxMenuItem* Item = Append(IDM_UI_BINDWIDGETSTODATASTORE, *BindMenuString);
			if ( !UIEditor->IsSelectedDataStoreTagValid(SelectedWidgets) )
			{
				Item->Enable(false);
			}
		}

	}
	else if ( bDataStoreManagerInitialized )
	{
		wxMenuItem* Item = Append(IDM_UI_BINDWIDGETSTODATASTORE, *BindMenuString);
		Item->Enable(false);
	}

	if ( bNeedsGeneralClearBindingItem )
	{
		bool bHasValidBinding = false;
		for ( INT SelectionIndex = 0; !bHasValidBinding && SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
		{
			UUIObject* BindableWidget = SelectedWidgets(SelectionIndex);
			if ( BindableWidget != NULL )
			{
				TMultiMap<FName,FUIDataStoreBinding*> WidgetBindings;
				BindableWidget->GetDataBindings(WidgetBindings);

				for ( TMultiMap<FName,FUIDataStoreBinding*>::TIterator It(WidgetBindings); !bHasValidBinding && It; ++It )
				{
					FUIDataStoreBinding* Binding = It.Value();
					if ( (*Binding) )
					{
						bHasValidBinding = true;
						break;
					}
				}
			}
		}

		wxMenuItem* Item = Append(IDM_UI_CLEARWIDGETSDATASTORE, *LocalizeUI(TEXT("UIEditor_ClearDataStore")));
		Item->Enable(bHasValidBinding);
	}
}

/**
 * Appends menu options which are applicable to both UIScenes and UIObjects
 */
void WxUIMenu::AppendScreenObjectOptions()
{
	if ( SelectedWidgets.Num() <= 1 )
	{
		Append( IDM_UI_KISMETEDITOR, *LocalizeUI(TEXT("UIEditor_MenuItem_KismetEditor")) );
		Append( IDM_UI_EDITEVENTS, *LocalizeUI(TEXT("UIEditor_MenuItem_EditEvents")) );
	}

	if(SelectedWidgets.Num() > 0)
	{
		Append( IDM_UI_DOCKINGEDITOR, *LocalizeUI(TEXT("UIEditor_MenuItem_DockingEditor")) );
		AppendSeparator();

		//@fixme ronp - make this data driven, i.e. wrap in check for if (SelectedWidgets->SupportsCustomContextMenuOptions())
		AppendCustomWidgetMenu();
		AppendPlacementOptions();
	}
	else
	{
		AppendSeparator();
		AppendPlacementOptions();
	}
}

/**
 * Appends a submenu containing context-sensitive menu options based on the currently selected widget
 */
void WxUIMenu::AppendCustomWidgetMenu()
{
	//@todo: This is a temp hack to support binding list elements.  Only append if we are working with a list, and it is the only widget selected.
	//@fixme ronp - we REALLY have to come up with a better way to do custom context menus for widgets.
	if ( SelectedWidgets.Num() == 1 )
	{
		UUIObject* ClickedWidget = SelectedWidgets(0);
		UUIList* List = Cast<UUIList>(ClickedWidget);
		if( List != NULL
		&&	List->CellDataComponent != NULL
		&&	List->CellDataComponent->IsA(UUIComp_ListPresenter::StaticClass()) )
		{
			if ( List->DataProvider )
			{
				FCellHitDetectionInfo ClickedCell;
				FVector LastClickLocation = UIEditor->ObjectVC->LastClick.GetLocation();
				FIntPoint HitLocation(appTrunc(LastClickLocation.X), appTrunc(LastClickLocation.Y));

				INT ClickedColumn = INDEX_NONE;
				if ( List->CalculateCellFromPosition(HitLocation, ClickedCell) )
				{
					ClickedColumn = ClickedCell.HitColumn;
				}
				WxUIListBindMenu* ListMenu = new WxUIListBindMenu(GetInvokingWindow());
				Append(new wxMenuItem(this, IDM_UI_LISTCELL_CHANGEBINDING, TEXT("UI List"), wxEmptyString, FALSE, ListMenu));
				ListMenu->Create(List, ClickedColumn);
			}

			return;
		}

		UUICalloutButton* CalloutButton = NULL;
		UUICalloutButtonPanel* CalloutPanel = NULL;
		for ( UUIObject* NextOwner = ClickedWidget; CalloutPanel == NULL && NextOwner; NextOwner = NextOwner->GetOwner() )
		{
			CalloutPanel = Cast<UUICalloutButtonPanel>(NextOwner);
			if ( CalloutButton == NULL )
			{
				CalloutButton = Cast<UUICalloutButton>(NextOwner);
			}
		}

		if ( CalloutPanel != NULL )
		{
			WxUICalloutButtonPanelMenu* PanelMenu = new WxUICalloutButtonPanelMenu(GetInvokingWindow());
			Append(new wxMenuItem(this, IDM_UI_CALLOUTBUTTON_ADD, TEXT("UICalloutButtonPanel"), wxEmptyString, FALSE, PanelMenu));
			PanelMenu->Create(CalloutPanel, CalloutButton);
			return;
		}


		UUITabControl* TabControl = NULL;
		for ( UUIObject* NextOwner = ClickedWidget; TabControl == NULL && NextOwner; NextOwner = NextOwner->GetOwner() )
		{
			TabControl = Cast<UUITabControl>(NextOwner);
		}

		if ( TabControl != NULL )
		{
			wxMenu* TabControlOptionsMenu = new wxMenu();
			{
				TabControlOptionsMenu->Append( IDM_UI_TABCONTROL_INSERTPAGE, *LocalizeUI(TEXT("UIEditor_MenuItem_InsertTabPage")), TEXT("") );

				//@todo - insert custom page using the actor browser (new page of a certain class) or the generic browser (instance a prefab)

				TabControlOptionsMenu->Append( IDM_UI_TABCONTROL_REMOVEPAGE, *LocalizeUI(TEXT("UIEditor_MenuItem_RemoveTabPage")), TEXT("") );
			}
			Append( wxID_ANY, *TabControl->GetClass()->GetDescription(), TabControlOptionsMenu);
			return;
		}
	}
}

/**
* Appends a menu for converting this widget's position into a different scale type.
*/
void WxUIMenu::AppendPositionConversionMenu()
{
	WxUIMenu* MainConversionMenu = new WxUIMenu(UIEditor);
	Append( IDM_UI_CONVERTPOSITION, *LocalizeUI(TEXT("UIEditor_MenuItem_ConvertPosition")), MainConversionMenu );

	MainConversionMenu->Create();

	INT MenuItemId = ID_CONVERT_POSITION_START;
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		// append an item for each face
		WxUIMenu* FaceConversionMenu = new WxUIMenu(UIEditor);
		MainConversionMenu->Append( IDM_UI_CONVERTPOSITION_FACE_START + FaceIndex, *LocalizeUI(*FString::Printf(TEXT("UIEditor_FaceText[%i]"), FaceIndex)), FaceConversionMenu );
		FaceConversionMenu->Create();

		for ( INT EvalIndex = 0; EvalIndex < EVALPOS_MAX; EvalIndex++ )
		{
			if ( EvalIndex > 0 )
			{
				FString MenuItemText = LocalizeUI(*FString::Printf(TEXT("UIEditor_PositionEvalText[%i]"), EvalIndex));
				FaceConversionMenu->Append(MenuItemId, *MenuItemText);
			}

			MenuItemId++;
		}
	}
}

/**
 * Appends menu options for changing or editing styles associated with the selected widgets.
 */
void WxUIMenu::AppendStyleOptions()
{
	UBOOL bItemsAppended = FALSE;
	
	if ( AppendStyleSelectionMenu() )
	{
		bItemsAppended = TRUE;
	}

	if ( AppendEditStyleMenu() )
	{
		bItemsAppended = TRUE;
	}

	if ( bItemsAppended )
	{
		AppendSeparator();
	}
}

IMPLEMENT_COMPARE_CONSTREF(FStyleReferenceId,UnUIEditorMenus,{ return appStricmp(*A.GetStyleReferenceName(),*B.GetStyleReferenceName()); })

/**
 * Appends a submenu containing a list of styles which are valid for the widget specified.
 *
 * @return	TRUE if at least one menu item was appended
 */
UBOOL WxUIMenu::AppendStyleSelectionMenu()
{
	UBOOL bAppendedItems = FALSE;

	TArray<FStyleReferenceId> ValidStyleReferences;

	// Loop through all widgets and build a list of possible style references.  We take a intersection of the set of FUIStyleReference properties
	// in each selected widget.  There should always be atleast 1 possible style reference (PrimaryStyle), the other ones are class specific and may not
	// be available in every widget class.  So we only display styles that are available in each of the selected widget classes.
	if(SelectedWidgets.Num() > 0)
	{
		UUIObject* BaseWidget = SelectedWidgets(0);

		// get the list of style references from the first selected widget
		BaseWidget->GetStyleReferenceProperties(ValidStyleReferences, INVALID_OBJECT);
		for ( INT SelectionIndex = 1; SelectionIndex < SelectedWidgets.Num() && ValidStyleReferences.Num() > 0; SelectionIndex++ )
		{
			UUIObject* Widget = SelectedWidgets(SelectionIndex);
			TArray<FStyleReferenceId> WidgetStyleReferences;
			Widget->GetStyleReferenceProperties(WidgetStyleReferences, INVALID_OBJECT);

			for ( INT PropertyIndex = ValidStyleReferences.Num() - 1; PropertyIndex >= 0; PropertyIndex-- )
			{
				FStyleReferenceId& StylePropertyId = ValidStyleReferences(PropertyIndex);
				if ( WidgetStyleReferences.FindItemIndex(StylePropertyId) == INDEX_NONE )
				{
					// this widget doesn't contain this style reference property, so remove it from the list of valid style property assignments
					ValidStyleReferences.Remove(PropertyIndex);
				}
			}
		}
	}

	Sort<USE_COMPARE_CONSTREF(FStyleReferenceId,UnUIEditorMenus)>( &ValidStyleReferences(0), ValidStyleReferences.Num() );

	// if any of these properties are array properties, we'll need to display the submenu regardless
	UBOOL bHasArrayStyleProperty=FALSE;
	for ( INT i = 0; i < ValidStyleReferences.Num(); i++ )
	{
		if ( ValidStyleReferences(i).StyleProperty->ArrayDim > 1 )
		{
			bHasArrayStyleProperty = TRUE;
			break;
		}
	}

	// If we have more than 1 valid style reference we need to create a submenu for each style reference.  So we need an intermediary 
	// menu to hold these submenus.  If we have multiple widgets selected, then we should always show the intermediate menu so that we
	// know exactly which style property we're changing.
	wxMenu* ParentMenu;
	if ( bHasArrayStyleProperty || ValidStyleReferences.Num() > 1 || (ValidStyleReferences.Num() > 0 && SelectedWidgets.Num() > 1) )
	{
		ParentMenu = new wxMenu;

		// Append the Select Style Submenu if the parent menu isnt this menu.
		Append( IDM_UI_SELECTSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_SelectStyle")), ParentMenu );
	}
	else
	{
		ParentMenu = this;
	}

	for( INT PropertyIndex = 0; PropertyIndex < ValidStyleReferences.Num(); PropertyIndex++ )
	{
		FStyleReferenceId& StyleRef = ValidStyleReferences(PropertyIndex);
		FStylePropertyReferenceGroup StyleReferenceGroup(StyleRef);
		for(INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++)
		{
			UUIObject* Widget = SelectedWidgets(SelectionIndex);

			FWidgetStyleReference* StylePropertyReference = new(StyleReferenceGroup.WidgetStyleReferences) FWidgetStyleReference(Widget);

			TMultiMap<FStyleReferenceId,FUIStyleReference*> StyleReferences;
			Widget->GetStyleReferences(StyleReferences, StyleRef, INVALID_OBJECT);
			bAppendedItems = bAppendedItems || StyleReferences.Num() > 0;

			// now add the references to the master list
			TArray<FUIStyleReference*> ReversedReferences;
			StyleReferences.MultiFind(StyleRef, ReversedReferences);

			for ( INT i = ReversedReferences.Num() - 1; i >= 0; i-- )
			{
				StylePropertyReference->References.AddItem(ReversedReferences(i));
			}
		}

		bAppendedItems = BuildStyleSelectionSubmenus(ParentMenu, StyleReferenceGroup) || bAppendedItems;
	}

	return bAppendedItems;
}

/**
 * Appends a submenu containing a list of style properties for the selected widgets, along with the styles which are valid for those properties.
 * Used for allowing the designer to open the style editor for a style assigned to a widget's style property
 *
 * @return	TRUE if at least one menu item was appended
 */
UBOOL WxUIMenu::AppendEditStyleMenu()
{
	UBOOL bAppendedItems = FALSE;

	// can only edit styles for one widget at a time
	if(SelectedWidgets.Num() == 1)
	{
		UUIObject* SelectedWidget = SelectedWidgets(0);

		TArray<FStyleReferenceId> ValidStyleReferences;

		// get the list of style reference properties from the selected widget from the first selected widget
		SelectedWidget->GetStyleReferenceProperties(ValidStyleReferences, INVALID_OBJECT);

		// if any of these properties are array properties, we'll need to display the submenu regardless
		UBOOL bHasArrayStyleProperty=FALSE;
		for ( INT i = 0; i < ValidStyleReferences.Num(); i++ )
		{
			if ( ValidStyleReferences(i).StyleProperty->ArrayDim > 1 )
			{
				bHasArrayStyleProperty = TRUE;
				break;
			}
		}

		// If we have more than 1 valid style reference we need to create a submenu for each style reference.  So we need an intermediary 
		// menu to hold these submenus.  If we have multiple widgets selected, then we should always show the intermediate menu so that we
		// know exactly which style property we're changing.
		wxMenu* ParentMenu;
		if ( bHasArrayStyleProperty || ValidStyleReferences.Num() > 1 )
		{
			ParentMenu = new wxMenu;
			Append( IDM_UI_EDITSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_EditStyle")), ParentMenu );
		}
		else
		{
			ParentMenu = this;
		}

		UIEditor->EditStyleMenuMap.Empty();

		Sort<USE_COMPARE_CONSTREF(FStyleReferenceId,UnUIEditorMenus)>( &ValidStyleReferences(0), ValidStyleReferences.Num() );
		for ( INT PropertyIndex = 0; PropertyIndex < ValidStyleReferences.Num(); PropertyIndex++ )
		{
			FStyleReferenceId& StylePropertyId = ValidStyleReferences(PropertyIndex);
			TArray<FUIStyleReference*> References;

			TMultiMap<FStyleReferenceId,FUIStyleReference*> CurrentStyleReferences;
			SelectedWidget->GetStyleReferences(CurrentStyleReferences, StylePropertyId, INVALID_OBJECT);

			// now add the references to the master list
			CurrentStyleReferences.MultiFind(StylePropertyId, References, TRUE);

			for ( INT ArrayIndex = 0; ArrayIndex < References.Num(); ArrayIndex++ )
			{
				UUIStyle* CurrentlyAssignedStyle = References(ArrayIndex)->GetResolvedStyle();
				if ( CurrentlyAssignedStyle != NULL )
				{
					if(ParentMenu == this)
					{
						// if we're here, it means that we only have one valid style property and one widget selected
						ParentMenu->Append( IDM_UI_EDITSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_EditStyle")) );
						UIEditor->EditStyleMenuMap.Set(IDM_UI_EDITSTYLE, WxUIEditorBase::FUIStyleMenuPair(StylePropertyId, ArrayIndex, CurrentlyAssignedStyle));
					}
					else
					{
						// Make sure we don't add more IDs than we have room for.
						FString MenuItemString;
						if ( References.Num() == 1 )
						{
							MenuItemString = FString::Printf(TEXT("%s (%s)"), *StylePropertyId.GetStyleReferenceName(), *CurrentlyAssignedStyle->GetStyleName());
						}
						else
						{
							FString ArrayIndexString;
							UProperty* Property = StylePropertyId.StyleProperty;
							if ( Property == NULL || Property->ArraySizeEnum == NULL )
							{
								ArrayIndexString = FString::Printf(TEXT("[%i]"), ArrayIndex );
							}
							else
							{
								ArrayIndexString = FString::Printf(TEXT("[%s]"), *Property->ArraySizeEnum->GetEnum(ArrayIndex).ToString());
							}
							MenuItemString = FString::Printf(TEXT("%s%s (%s)"), *StylePropertyId.GetStyleReferenceName(), *ArrayIndexString, *CurrentlyAssignedStyle->GetStyleName());
						}

						INT StyleMenuID = GetNextStyleEditMenuID();
						checkf(StyleMenuID != INDEX_NONE,TEXT("No more ids available for edit-style menu items - try increasing the number reserved by IDM_UI_STYLEREF_END"));

						UIEditor->EditStyleMenuMap.Set(StyleMenuID, WxUIEditorBase::FUIStyleMenuPair(StylePropertyId, ArrayIndex, CurrentlyAssignedStyle));
						ParentMenu->Append(StyleMenuID, *MenuItemString);
					}
				}
			}
		}
	}

	return bAppendedItems;
}

/**
 * Appends options which are normally associated with "Edit" menus
 * @param bRightClickMenu Whether or not this is a right click menu.
 */
void WxUIMenu::AppendEditOptions(UBOOL bRightClickMenu)
{
	if(bRightClickMenu == TRUE)
	{
		Append( IDM_CUT, *LocalizeUnrealEd(TEXT("Cut")), TEXT("") );
		Append( IDM_COPY, *LocalizeUnrealEd(TEXT("Copy")), TEXT("") );

		Append( IDM_UI_CLICK_PASTE, *LocalizeUnrealEd(TEXT("Paste")), TEXT("") );
		Append( IDM_UI_CLICK_PASTEASCHILD, *LocalizeUnrealEd(TEXT("PasteAsChild")), TEXT("") );

		// Disable menu items here because for some reason it doesn't disable them in the update UI call.
		FString PasteText = appClipboardPaste();
		UUIObject* SelectedWidget = GetUIEditor()->Selection->GetTop<UUIObject>();

		// Make sure that the clipboard has UI editor data.
		UBOOL bHasBeginScene = PasteText.InStr(TEXT("Begin Scene")) == 0;


		if(!bHasBeginScene)
		{
			Enable( IDM_UI_CLICK_PASTE, false );
			Enable( IDM_UI_CLICK_PASTEASCHILD, false );
		}
		else if(SelectedWidget == NULL)
		{
			Enable( IDM_UI_CLICK_PASTEASCHILD, false );
		}
	}
	else
	{
		Append( IDM_CUT, *LocalizeUnrealEd(TEXT("CutA")), TEXT("") );
		Append( IDM_COPY, *LocalizeUnrealEd(TEXT("CopyA")), TEXT("") );
		Append( IDM_PASTE, *LocalizeUnrealEd(TEXT("PasteA")), TEXT("") );
		Append( IDM_UI_PASTEASCHILD, *LocalizeUnrealEd(TEXT("PasteAsChildA")), TEXT("") );
	}
}

/**
 * Appends all options which are used for manipulating the Z-order placement of the scene's widgets
 */
void WxUIMenu::AppendLayerOptions()
{
	Append( IDM_UI_CHANGEPARENT, *LocalizeUI(TEXT("UIEditor_MenuItem_ChangeParent")), TEXT("") );
	
	wxMenu* ReorderMenu = new wxMenu();
	{
		ReorderMenu->Append( IDM_UI_MOVEUP, *LocalizeUI(TEXT("UIEditor_MenuItem_MoveUp")), TEXT("") );
		ReorderMenu->Append( IDM_UI_MOVEDOWN, *LocalizeUI(TEXT("UIEditor_MenuItem_MoveDown")), TEXT("") );
		ReorderMenu->Append( IDM_UI_MOVETOTOP, *LocalizeUI(TEXT("UIEditor_MenuItem_MoveToTop")), TEXT("") );
		ReorderMenu->Append( IDM_UI_MOVETOBOTTOM, *LocalizeUI(TEXT("UIEditor_MenuItem_MoveToBottom")), TEXT("") );
	}
	Append( wxID_ANY, *LocalizeUI("UIEditor_MenuItem_ReorderWidget"), ReorderMenu);
	
	// tracks whether the item will be enabled in the context menu... an item should only be enabled
	// if the action can be applied to all selected widgets
	UBOOL bMoveUpEnabled, bMoveDownEnabled, bMoveToTopEnabled, bMoveToBottomEnabled;
	bMoveUpEnabled=bMoveDownEnabled=bMoveToTopEnabled=bMoveToBottomEnabled=SelectedParents.Num() > 0;

	// this keeps track of which parents contain the widgets we'd be moving...if more than widget from the
	// same parent is selected, then the user is not allowed to use the Move To Top/Bottom items...
	TArray<UUIScreenObject*> WidgetParents;
	for ( INT i = 0; i < SelectedParents.Num(); i++ )
	{
		UUIObject* Widget = SelectedParents(i);
		UUIScreenObject* WidgetParent = Widget->GetParent();
		checkSlow(WidgetParent);

		UBOOL bMultipleChildrenSelected = WidgetParents.FindItemIndex(WidgetParent) != INDEX_NONE;
		if ( !bMultipleChildrenSelected )
		{
			WidgetParents.AddItem(WidgetParent);
		}

		INT ChildIndex = WidgetParent->Children.FindItemIndex(Widget);
		checkSlow(ChildIndex!=INDEX_NONE);

		UBOOL bAllowMoveDown		= ChildIndex > 0;
		UBOOL bAllowMoveUp			= ChildIndex < WidgetParent->Children.Num() - 1;
		UBOOL bAllowMoveToBottom	= bAllowMoveDown && !bMultipleChildrenSelected;
		UBOOL bAllowMoveToTop		= bAllowMoveUp && !bMultipleChildrenSelected;

		bMoveUpEnabled = bMoveUpEnabled && bAllowMoveUp;
		bMoveDownEnabled = bMoveDownEnabled && bAllowMoveDown;
		bMoveToBottomEnabled = bMoveToBottomEnabled && bAllowMoveToBottom;
		bMoveToTopEnabled = bMoveToTopEnabled && bAllowMoveToTop;
	}

	Enable( IDM_UI_MOVEUP, bMoveUpEnabled == TRUE );
	Enable( IDM_UI_MOVEDOWN, bMoveDownEnabled == TRUE );
	Enable( IDM_UI_MOVETOTOP, bMoveToTopEnabled == TRUE );
	Enable( IDM_UI_MOVETOBOTTOM, bMoveToBottomEnabled == TRUE );
}

/**
 * Appends all options which are common to all UObjects, such as rename, delete, etc.
 */
void WxUIMenu::AppendObjectOptions()
{

	AppendEditOptions(TRUE);
	Append( IDMN_ObjectContext_Rename, *LocalizeUnrealEd(TEXT("Rename")), TEXT("") );
	Append( IDMN_ObjectContext_Delete, *LocalizeUnrealEd(TEXT("Delete")), TEXT("") );
	Append( IDM_CREATEARCHETYPE, *LocalizeUnrealEd(TEXT("Archetype_Create")) );

	//@todo ronp nested prefabs: nesting prefabs isn't yet supported.
#if !SUPPORTS_NESTED_PREFABS
	// if our parent window is for a UIPrefab, disable the create archetype option.
	WxUIEditorBase* UIEditor = GetUIEditor();
	if ( UIEditor == NULL || UIEditor->OwnerScene->GetEditorDefaultParentWidget()->IsA(UUIPrefab::StaticClass()) )
	{
		Enable( IDM_CREATEARCHETYPE, false );
	}
#endif
}

void WxUIMenu::AppendPlacementOptions()
{
	WxUIEditorBase* UIEditor = GetUIEditor();
	if ( UIEditor != NULL )
	{
		check(UIEditor->SceneManager);

		WxUIMenu* ChooseClassSubmenu = new WxUIMenu;
		Append( IDM_UI_NEWOBJECT, *LocalizeUI(TEXT("UIEditor_MenuItem_PlaceWidget")), ChooseClassSubmenu);

		ChooseClassSubmenu->Create();

		TArray<UUIObject*> SelectedWidgets;
		UIEditor->GetSelectedWidgets(SelectedWidgets);
		UUIScreenObject* WidgetParent = SelectedWidgets.Num() == 1
			? SelectedWidgets(0)
			: UIEditor->OwnerScene->GetEditorDefaultParentWidget();

		TArray<FUIObjectResourceInfo>& WidgetResources = UIEditor->SceneManager->UIWidgetResources;
		for ( INT i = 0; i < WidgetResources.Num(); i++ )
		{
			FUIObjectResourceInfo& ClassInfo = WidgetResources(i);
			UUIObject* WidgetCDO = Cast<UUIObject>(ClassInfo.UIResource);
			if ( WidgetParent->IsA(WidgetCDO->GetClass()->ClassWithin) )
			{
				ChooseClassSubmenu->Append( IDM_UI_NEWOBJECT_START + i, *ClassInfo.FriendlyName, TEXT("") );
			}
		}

		// now add context-sensitive items based on the selected item from the generic browser
		USelection* SelectionSet = GEditor->GetSelectedObjects();

		// if a UIArchetype is selected in the generic browser, add a menu option for placing an instance.
		UUIPrefab* UIArchetype = SelectionSet->GetTop<UUIPrefab>();
		if( UIArchetype != NULL )
		{
			Append( IDM_UI_PLACEARCHETYPE, *FString::Printf(LocalizeSecure(LocalizeUI("UIEditor_MenuItem_PlaceArchetypef"), *UIArchetype->GetName())), TEXT("") );

#if !SUPPORTS_NESTED_PREFABS
			//@todo ronp nested prefabs: nesting prefabs isn't yet supported.
			// if our parent window is for a UIPrefab, disable the create archetype option.
			if ( UIEditor->OwnerScene->GetEditorDefaultParentWidget()->IsA(UUIPrefab::StaticClass()) )
			{
				Enable(IDM_UI_PLACEARCHETYPE, false);
			}
#endif
		}
	}
}

/* ==========================================================================================================
	WxUIContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIContextMenu,WxUIMenu)

/* ==========================================================================================================
	WxUISceneTreeContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUISceneTreeContextMenu,WxUIContextMenu)

/**
 * Initialization method used in two-step dynamic window creation
 * Appends the menu options
 */
void WxUISceneTreeContextMenu::Create( WxTreeObjectWrapper* InItemData  )
{
	check(InItemData);
	UUIObject* SelectedWidget = InItemData->GetObject<UUIObject>();

	WxUIContextMenu::Create(SelectedWidget);
	AppendWidgetOptions();
}

/* ==========================================================================================================
	WxUILayerTreeContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUILayerTreeContextMenu,WxUIContextMenu)


/**
 * Constructor
 * Appends the menu options
 */
WxUILayerTreeContextMenu::WxUILayerTreeContextMenu(wxWindow* InOwner/*=NULL*/)
:	WxUIContextMenu(InOwner)
{
	Append(IDM_UI_INSERTLAYER, *LocalizeUI(TEXT("UIEditor_MenuItem_InsertLayer")), TEXT(""));
	Append(IDM_UI_INSERTCHILDLAYER, *LocalizeUI(TEXT("UIEditor_MenuItem_InsertChildLayer")), TEXT(""));
	Append(IDM_UI_RENAMELAYER, *LocalizeUI(TEXT("UIEditor_MenuItem_RenameLayer")), TEXT(""));
	Append(IDM_UI_SELECTALLWIDGETSINLAYER, *LocalizeUI(TEXT("UIEditor_MenuItem_SelectAllWidgetsInLayer")), TEXT(""));
	Append(IDM_UI_DELETELAYER, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteLayer")), TEXT(""));
}

/* ==========================================================================================================
	WxUIStyleTreeContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIStyleTreeContextMenu,WxUIContextMenu)

/**
 * Constructor
 * Appends the menu options
 */
WxUIStyleTreeContextMenu::WxUIStyleTreeContextMenu( wxWindow* InOwner/*=NULL*/ )
: WxUIContextMenu(InOwner)
{
	Append( IDM_UI_EDITSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_EditStyle")), TEXT("") );
	Append( IDM_UI_CREATESTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_CreateStyle")), TEXT("") );
	Append( IDM_UI_CREATESTYLE_FROMTEMPLATE, *LocalizeUI(TEXT("UIEditor_MenuItem_CreateStyle_Selected")), TEXT("") );
	AppendSeparator();
	Append( IDM_UI_DELETESTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteStyle")), TEXT("") );
}

/* ==========================================================================================================
	WxUILoadSkinContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUILoadSkinContextMenu,WxUIContextMenu)

/**
 * Constructor
 * Appends the menu options
 */
WxUILoadSkinContextMenu::WxUILoadSkinContextMenu( wxWindow* InOwner/*=NULL*/ )
: WxUIContextMenu(InOwner)
{
	Append( IDM_UI_LOADSKIN, *LocalizeUI(TEXT("UIEditor_MenuItem_LoadSkin")), TEXT("") );
	Append( IDM_UI_CREATESKIN, *LocalizeUI(TEXT("UIEditor_MenuItem_CreateSkin")), TEXT("") );
}

/* ==========================================================================================================
	WxUIObjectNewContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIObjectNewContextMenu,WxUIContextMenu)

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 * Appends the menu options.
 *
 * @param	InActiveWidget	the widget that was clicked on to invoke this context menu
 */
void WxUIObjectNewContextMenu::Create( UUIObject* InActiveWidget )
{
	WxUIContextMenu::Create(InActiveWidget);
	AppendScreenObjectOptions();
	AppendSeparator();
	AppendEditOptions(TRUE);
}

/* ==========================================================================================================
	WxUIObjectOptionsContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIObjectOptionsContextMenu,WxUIContextMenu)

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 * Appends the menu options.
 *
 * @param	InActiveWidget	the widget that was clicked on to invoke this context menu
 */
void WxUIObjectOptionsContextMenu::Create( UUIObject* InActiveWidget )
{
	WxUIContextMenu::Create(InActiveWidget);
	AppendWidgetOptions();
}

/* ==========================================================================================================
	WxUIHandleContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIHandleContextMenu,WxUIContextMenu)

/**
* Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
*
* @param	InActiveWidget	the widget that was clicked on to invoke this context menu
* @param	InActiveFace	the dock handle that was clicked on to invoke this context menu
*/
void WxUIHandleContextMenu::Create(UUIObject* InActiveWidget, EUIWidgetFace InActiveFace)
{
	WxUIContextMenu::Create(InActiveWidget);
	ActiveFace = InActiveFace;
}

/* ==========================================================================================================
WxUIDockTargetContextMenu
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIDockTargetContextMenu,WxUIContextMenu)

/**
* Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
* @param NumOptions		Number of options to generate this menu with.
*/
void WxUIDockTargetContextMenu::Create( INT InNumOptions )
{
	NumOptions = InNumOptions;

	// Generate placeholder items for the menu, the labels will need to be changed later.
	INT ItemId = IDM_UI_DOCKTARGET_BEGIN;
	const INT MaxRange = IDM_UI_DOCKTARGET_END - IDM_UI_DOCKTARGET_BEGIN;

	if(NumOptions > MaxRange)
	{
		NumOptions = MaxRange;
	}

	for(INT ItemIdx = 0; ItemIdx < NumOptions; ItemIdx++)
	{
		Append( ItemId, TEXT("This should be replaced."));
		ItemId++;
	}
}
/** 
* Sets the label for a menu item.
*
* @param OptionIdx	Which menu item to set the label for (Must be between 0 and the number of options - 1).
* @param Label		New label for the item.
*/
void WxUIDockTargetContextMenu::SetMenuItemLabel(INT OptionIdx, const TCHAR* Label )
{
	check(OptionIdx < NumOptions);
	
	const INT ItemId = OptionIdx + IDM_UI_DOCKTARGET_BEGIN;

	SetLabel(ItemId, Label);
}


/* ==========================================================================================================
	WxUIEditorMenuBar
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxUIEditorMenuBar,wxMenuBar)

/** Constructor */
void WxUIEditorMenuBar::Create( WxUIEditorBase* EditorWindow )
{
	
	FileMenu = new wxMenu;
	FileMenu->Append(IDMN_UI_CLOSEWINDOW, *LocalizeUnrealEd(TEXT("Close")), TEXT(""), wxITEM_NORMAL);
	Append(FileMenu, *LocalizeUnrealEd(TEXT("File")));


	EditMenu = new WxUIMenu(NULL);
	{
		EditMenu->Append(IDM_UNDO, *LocalizeUnrealEd(TEXT("UndoA")), *LocalizeUnrealEd("UndoLastAction"));
		EditMenu->Append(IDM_REDO, *LocalizeUnrealEd(TEXT("RedoA")), *LocalizeUnrealEd("RedoLastAction"));

		EditMenu->AppendSeparator();
		EditMenu->AppendEditOptions(FALSE);
		EditMenu->AppendSeparator();
		EditMenu->Append( IDMN_ObjectContext_Rename, *LocalizeUI(TEXT("UIEditor_MenuItem_RenameSelectedWidgets")), *LocalizeUI(TEXT("UIEditor_HelpText_RenameSelectedWidgets")) );
		EditMenu->Append( IDMN_ObjectContext_Delete, *LocalizeUI(TEXT("UIEditor_MenuItem_DeleteSelectedWidgets")), *LocalizeUI(TEXT("UIEditor_HelpText_DeleteSelectedWidgets")) );
		EditMenu->AppendSeparator();
		EditMenu->Append(IDM_UI_BINDWIDGETSTODATASTORE, *LocalizeUI("UIEditor_BindDataStoreA"));
		EditMenu->AppendSeparator();
		EditMenu->Append(IDM_UI_DATASTOREBROWSER, *LocalizeUI("UIEditor_MenuItem_DataStoreBrowser"), *LocalizeUI("UIEditor_HelpText_DataStoreBrowser"), wxITEM_NORMAL);
		EditMenu->Append(IDM_UI_BINDUIEVENTKEYS, *LocalizeUI("UIEditor_MenuItem_BindUIEventKeys"), *LocalizeUI("UIEditor_HelpText_BindUIEventKeys"), wxITEM_NORMAL);
	}

	Append(EditMenu, *LocalizeUnrealEd(TEXT("Edit")));
	
	ViewMenu = new wxMenu;
	{

		// 	wxMenu* ViewportSizeSubmenu = new wxMenu;
		// 
		// 	//@todo - create a class for storing the user's preferences, then retrieve the values from there
		// 	ViewportSizeSubmenu->Append(ID_UITOOL_VIEWPORTSIZE, TEXT("Auto"), TEXT(""), wxITEM_RADIO);
		// 	ViewportSizeSubmenu->Append(ID_UITOOL_VIEWPORTSIZE+1, TEXT("640x480"), TEXT(""), wxITEM_RADIO);
		// 	ViewportSizeSubmenu->Append(ID_UITOOL_VIEWPORTSIZE+2, TEXT("800x600"), TEXT(""), wxITEM_RADIO);
		// 	ViewportSizeSubmenu->Check(ID_UITOOL_VIEWPORTSIZE, true);
		// 
		// 	ViewMenu->Append(wxID_ANY, *LocalizeUI(TEXT("UIEditor_MenuItem_ViewportSize")), ViewportSizeSubmenu);
		// 	ViewMenu->AppendSeparator();

		ViewMenu->Append(ID_UITOOL_VIEW_RESET, *LocalizeUI(TEXT("UIEditor_MenuItem_ViewReset")), *LocalizeUI(TEXT("UIEditor_HelpText_ViewReset")), wxITEM_NORMAL);
		ViewMenu->Append(ID_UI_CENTERONSELECTION, *LocalizeUI("UIEditor_MenuItem_CenterOnSelection"), *LocalizeUI("UIEditor_HelpText_CenterOnSelection"), wxITEM_NORMAL);
		ViewMenu->AppendSeparator();

		ViewMenu->Append(ID_UITOOL_VIEW_DRAWBKGND, *LocalizeUI(TEXT("UIEditor_MenuItem_ViewDrawBkgnd")), _T(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UITOOL_VIEW_DRAWGRID, *LocalizeUI(TEXT("UIEditor_MenuItem_ViewDrawGrid")), _T(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UITOOL_VIEW_WIREFRAME, *LocalizeUI(TEXT("UIEditor_MenuItem_ViewShowWireframe")), _T(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UI_SHOWVIEWPORTOUTLINE, *LocalizeUI(TEXT("UIEditor_MenuItem_ViewportOutline")), TEXT(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UI_SHOWCONTAINEROUTLINE, *LocalizeUI(TEXT("UIEditor_MenuItem_ContainerOutline")), TEXT(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UI_SHOWSELECTIONOUTLINE, *LocalizeUI(TEXT("UIEditor_MenuItem_SelectionOutline")), TEXT(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UI_SHOWSELECTIONHANDLES, *LocalizeUI(TEXT("UIEditor_ToolTip_SelectionHandles")), TEXT(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UI_SHOWSELECTIONOUTLINE_PERWIDGET, *LocalizeUI(TEXT("UIEditor_ToolTip_SelectionOutlinePerWidget")), TEXT(""), wxITEM_CHECK);
		ViewMenu->Append(ID_UITOOL_SHOWDOCKHANDLES, *LocalizeUI(TEXT("UIEditor_MenuItem_ShowDockHandles")), *LocalizeUI(TEXT("UIEditor_HelpText_ShowDockHandles")), wxITEM_CHECK);
		ViewMenu->Append(ID_UI_SHOWTITLESAFEREGION, *LocalizeUI("UIEditor_MenuItem_TitleSafeRegions"), *LocalizeUI("UIEditor_HelpText_TitleSafeRegions"), wxITEM_CHECK);

	}

	Append(ViewMenu, *LocalizeUnrealEd(TEXT("View")));

	// Skin Menu
	wxMenu* SkinMenu = new wxMenu;
	{
		SkinMenu->Append(IDM_UI_SAVESKIN, *LocalizeUI(TEXT("UIEditor_SaveSelectedSkin")), *LocalizeUI(TEXT("UIEditor_HelpText_SaveSelectedSkin")), wxITEM_NORMAL);
		SkinMenu->Append(IDM_UI_EDITSKIN, *LocalizeUI(TEXT("UIEditor_EditSelectedSkin")), *LocalizeUI(TEXT("UIEditor_HelpText_EditSelectedSkin")), wxITEM_NORMAL);
		SkinMenu->AppendSeparator();
		SkinMenu->Append( IDM_UI_LOADSKIN, *LocalizeUI(TEXT("UIEditor_MenuItem_LoadSkin")), *LocalizeUI(TEXT("UIEditor_HelpText_LoadExistingSkin")), wxITEM_NORMAL);
		SkinMenu->Append( IDM_UI_CREATESKIN, *LocalizeUI(TEXT("UIEditor_MenuItem_CreateSkin")), *LocalizeUI(TEXT("UIEditor_HelpText_CreateSkin")), wxITEM_NORMAL);
	}
	Append(SkinMenu, *LocalizeUI(TEXT("UIEditor_Skin")));

	SceneMenu = new WxUIMenu(NULL);
	{
		SceneMenu->Append(IDM_UI_FORCE_SCENE_REFRESH, *LocalizeUI(TEXT("UIEditor_MenuItem_RefreshScene")), *LocalizeUI(TEXT("UIEditor_HelpText_RefreshScene")), wxITEM_NORMAL);
	}
	Append( SceneMenu, *LocalizeUI(TEXT("UIEditor_Scene")) );


	EditorWindow->AppendWindowMenu(this);
}

/** Destructor */
WxUIEditorMenuBar::~WxUIEditorMenuBar()
{
}

/* ==========================================================================================================
	WxUIDragGridMenu
========================================================================================================== */

/**
 * Menu for controlling options related to the drag grid.
 */
WxUIDragGridMenu::WxUIDragGridMenu()
{
	AppendCheckItem( IDM_DRAG_GRID_TOGGLE, *LocalizeUI(TEXT("UIEditor_MenuItem_SnapToGrid")), *LocalizeUI(TEXT("UIEditor_ToolTip_SnapToGrid")) );
	AppendSeparator();
	AppendCheckItem( IDM_DRAG_GRID_1, *LocalizeUnrealEd("1"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_2, *LocalizeUnrealEd("2"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_4, *LocalizeUnrealEd("4"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_8, *LocalizeUnrealEd("8"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_16, *LocalizeUnrealEd("16"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_32, *LocalizeUnrealEd("32"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_64, *LocalizeUnrealEd("64"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_128, *LocalizeUnrealEd("128"), TEXT("") );
	AppendCheckItem( IDM_DRAG_GRID_256, *LocalizeUnrealEd("256"), TEXT("") );
}


/*-----------------------------------------------------------------------------
WxUIKismetNewObject.
-----------------------------------------------------------------------------*/

WxUIKismetNewObject::WxUIKismetNewObject(WxUIKismetEditor* SeqEditor)
{
	// create the top level menus
	ActionMenu = new wxMenu();
	Append( IDM_KISMET_NEW_ACTION, *LocalizeUnrealEd("NewAction"), ActionMenu );
	ConditionMenu = new wxMenu();
	VariableMenu = new wxMenu();
	EventMenu = new wxMenu();
	
	// set up map of sequence object classes to their respective menus
	TMap<UClass*,wxMenu*> MenuMap;
	MenuMap.Set(USequenceAction::StaticClass(),ActionMenu);
	MenuMap.Set(USequenceCondition::StaticClass(),ConditionMenu);
	MenuMap.Set(USequenceVariable::StaticClass(),VariableMenu);
	MenuMap.Set(USequenceEvent::StaticClass(),EventMenu);
	
	// list of categories for all the submenus
	TMap<FString,wxMenu*> ActionSubmenus;
	TMap<FString,wxMenu*> ConditionSubmenus;
	TMap<FString,wxMenu*> VariableSubmenus;
	TMap<FString,wxMenu*> EventSubmenus;
	
	// map all the submenus to their parent menu
	TMap<wxMenu*,TMap<FString,wxMenu*>*> SubmenuMap;
	SubmenuMap.Set(ActionMenu,&ActionSubmenus);
	SubmenuMap.Set(ConditionMenu,&ConditionSubmenus);
	SubmenuMap.Set(VariableMenu,&VariableSubmenus);
	SubmenuMap.Set(EventMenu,&EventSubmenus);

	// iterate through all known sequence object classes
	INT CommentIndex = INDEX_NONE;
	FString CategoryName, OpName;
	for(INT i=0; i<SeqEditor->SeqObjClasses.Num(); i++)
	{
		UClass *SeqClass = SeqEditor->SeqObjClasses(i);
		if (SeqClass != NULL)
		{
			// special cases
			if( SeqEditor->SeqObjClasses(i)->IsChildOf(USeqAct_Interp::StaticClass()) )
			{
				Append(IDM_NEW_SEQUENCE_OBJECT_START+i, *FString::Printf(TEXT("New %s"), *((USequenceObject*)SeqClass->GetDefaultObject())->ObjName), TEXT(""));
			}
			else if( SeqEditor->SeqObjClasses(i)->IsChildOf(USequenceFrame::StaticClass()) )
			{
				CommentIndex = i;
			}
			else
			{
				// grab the default object for reference
				USequenceObject* SeqObjDefault = (USequenceObject*)SeqClass->GetDefaultObject();
				// first look up the menu type
				wxMenu* Menu = NULL;
				for (TMap<UClass*,wxMenu*>::TIterator It(MenuMap); It && Menu == NULL; ++It)
				{
					UClass *TestClass = It.Key();
					if (SeqClass->IsChildOf(TestClass))
					{
						Menu = It.Value();
					}
				}
				if (Menu != NULL)
				{
					// check for a category
					if (GetSequenceObjectCategory(SeqObjDefault,CategoryName,OpName))
					{
						// look up the submenu map
						TMap<FString,wxMenu*>* Submenus = SubmenuMap.FindRef(Menu);
						if (Submenus != NULL)
						{
							wxMenu* Submenu = Submenus->FindRef(CategoryName);
							if (Submenu == NULL)
							{
								// create a new submenu for the category
								Submenu = new wxMenu();
								// add it to the map
								Submenus->Set(*CategoryName,Submenu);
								// and add it to the parent menu
								Menu->Append(IDM_KISMET_NEW_ACTION_CATEGORY_START+(Submenus->Num()),*CategoryName, Submenu);
							}
							// add the object to the submenu
							Submenu->Append(IDM_NEW_SEQUENCE_OBJECT_START+i,*OpName,TEXT(""));
						}
					}
					else
					{
						// otherwise add it to the parent menu
						Menu->Append(IDM_NEW_SEQUENCE_OBJECT_START+i,*OpName,TEXT(""));
					}
				}
			}
		}
	}

	Append( IDM_KISMET_NEW_CONDITION, *LocalizeUnrealEd("NewCondition"), ConditionMenu );
	Append( IDM_KISMET_NEW_VARIABLE, *LocalizeUnrealEd("NewVariable"), VariableMenu );
	Append( IDM_KISMET_NEW_EVENT, *LocalizeUnrealEd("NewEvent"), EventMenu );

	if(CommentIndex != INDEX_NONE)
	{
		AppendSeparator();
		Append( IDM_NEW_SEQUENCE_OBJECT_START+CommentIndex, *LocalizeUnrealEd("NewComment"), TEXT("") );
	}

	// Get a list of selected widgets.
	TArray<UUIObject*> SelectedWidgets;
	SeqEditor->UIEditor->GetSelectedWidgets(SelectedWidgets);
	
	// Add 'New Event Using' item.
	if(SelectedWidgets.Num() > 0)
	{
		// Don't add items, unless all actors belong to the sequence's level.
		AppendSeparator();
		ContextEventMenu = new wxMenu();

		FString NewEventString;
		FString NewObjVarString;

		FString FirstObjName = SelectedWidgets(0)->GetName();
		if( SeqEditor->NewObjActors.Num() > 1 )
		{
			NewEventString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewEventsUsing_F"), *FirstObjName) );
			NewObjVarString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewObjectVarsUsing_F"), *FirstObjName) );
		}
		else
		{
			NewEventString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewEventUsing_F"), *FirstObjName) );
			NewObjVarString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewObjectVarUsing_F"), *FirstObjName) );
		}

		Append( IDM_KISMET_NEW_VARIABLE_OBJ_CONTEXT, *NewObjVarString, TEXT("") );
		SeqEditor->bAttachVarsToConnector = false;

		/* @todo: See if the UI Kismet editor has support for this. - ASM 3/1/2006
		Append( IDM_KISMET_NEW_EVENT_CONTEXT, *NewEventString, ContextEventMenu );

		for(INT i=0; i<SeqEditor->NewEventClasses.Num(); i++)
		{
			USequenceEvent* SeqEvtDefault = (USequenceEvent*)SeqEditor->NewEventClasses(i)->GetDefaultObject();
			ContextEventMenu->Append( IDM_NEW_SEQUENCE_EVENT_START+i, *SeqEvtDefault->ObjName, TEXT("") );
		}
		*/
	}

	AppendSeparator();

	// Append option for pasting clipboard at current mouse location
	Append(IDM_KISMET_PASTE_HERE, *LocalizeUnrealEd("PasteHere"), TEXT(""));

}

/**
 * Looks for a category in the op's name, returns TRUE if one is found.
 */
UBOOL WxUIKismetNewObject::GetSequenceObjectCategory(USequenceObject *Op, FString &CategoryName, FString &OpName)
{
	OpName = Op->ObjName;
	if (Op->ObjCategory.Len() > 0)	
	{
		CategoryName = Op->ObjCategory;
		return TRUE;
	}
	return FALSE;
}

/* ==========================================================================================================
	WxUIKismet_CalloutProxyOptions
========================================================================================================== */
/** Constructor */
WxUIKismet_CalloutProxyOptions::WxUIKismet_CalloutProxyOptions( WxUIKismetEditor* KismetEditor )
: WxMBKismetObjectOptions(KismetEditor)
{
	// this menu should have only been invoked if the user clicked on a single UIEvent_CalloutButtonInputProxy
	// object, but double-check to be sure
	if ( KismetEditor != NULL && KismetEditor->SelectedSeqObjs.Num() == 1 )
	{
		UUIEvent_CalloutButtonInputProxy* InputProxy = Cast<UUIEvent_CalloutButtonInputProxy>(KismetEditor->SelectedSeqObjs(0));
		if ( InputProxy != NULL )
		{
			PrependSeparator();
			Prepend(IDM_UIKISMET_CALLOUT_ADD, *LocalizeUI(TEXT("UIKismet_MenuItem_AddCalloutButton")));
		}
	}
}

/* ==========================================================================================================
	WxUIKismet_CalloutProxyConnectorOptions
========================================================================================================== */
/** Constructor */
WxUIKismet_CalloutProxyConnectorOptions::WxUIKismet_CalloutProxyConnectorOptions( WxUIKismetEditor* KismetEditor )
: WxMBKismetConnectorOptions(KismetEditor)
{
	// this menu should have only been invoked if the user clicked on a single UIEvent_CalloutButtonInputProxy
	// object, but double-check to be sure
	if ( KismetEditor != NULL && KismetEditor->ConnSeqOp != NULL && KismetEditor->ConnType == LOC_OUTPUT )
	{
		UUIEvent_CalloutButtonInputProxy* InputProxy = Cast<UUIEvent_CalloutButtonInputProxy>(KismetEditor->ConnSeqOp);
		if ( InputProxy != NULL )
		{
			PrependSeparator();
			
			// determine which index was selected
			const INT ConnIndex = KismetEditor->ConnIndex;

			// add the item to allow the user to change the selected alias
			const FString ChangeAliasString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIKismet_MenuItem_ChangeCalloutAlias")), *InputProxy->OutputLinks(ConnIndex).LinkDesc));
			wxMenu* AliasSelectionMenu = new wxMenu;
			Prepend(IDM_UIKISMET_CALLOUT_CHANGE, *ChangeAliasString, AliasSelectionMenu);

			// populate the submenu with the list of aliases that aren't being used in the button panel
			//@fixme ronp - technically we shouldn't allow any aliases that are used in other button panels
			// from the same scene, as these are considered atomic...
			TArray<FName> InputAliases;
			InputProxy->ButtonPanel->GetAvailableCalloutButtonAliases(InputAliases);

			const size_t MaxAliases = IDM_UIKISMET_CALLOUT_CHANGE_END - IDM_UIKISMET_CALLOUT_CHANGE_START;
			size_t NumAliases = (size_t)InputAliases.Num();
			if ( NumAliases > MaxAliases )
			{
				appMsgf(AMT_OK, TEXT("Not enough menu ids reserved to display all available button callout aliases (max: %i, current: %i).  Increase the value of IDM_UIKISMET_CALLOUT_CHANGE_END"),
					MaxAliases, NumAliases);

				NumAliases = MaxAliases;
			}
			for ( size_t AliasIdx = 0; AliasIdx < NumAliases; AliasIdx++ )
			{
				AliasSelectionMenu->Append(IDM_UIKISMET_CALLOUT_CHANGE_START+AliasIdx, *InputAliases(AliasIdx).ToString());
			}

			// add the item to allow the user to completely remove this button
			const FString RemoveButtonString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIKismet_MenuItem_RemoveCalloutButton")), *InputProxy->OutputLinks(ConnIndex).LinkDesc));
			Prepend(IDM_UIKISMET_CALLOUT_REMOVE, *RemoveButtonString);
		}
	}
}


/* ==========================================================================================================
	WxUIListBindMenu
========================================================================================================== */
/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 * Appends the menu options.
 *
 * @param	InActiveWidget	the widget that was clicked on to invoke this context menu
 */
void WxUIListBindMenu::Create( UUIObject* InActiveWidget, INT ColumnIdx )
{
	WxUIContextMenu::Create(InActiveWidget);

	UUIList* OwnerList = CastChecked<UUIList>(ActiveWidget);
	UUIComp_ListPresenter* ReportPresenter = CastChecked<UUIComp_ListPresenter>(OwnerList->CellDataComponent);

	const INT NumColumns = OwnerList->CellDataComponent->GetSchemaCellCount();
	TArray<FName> ElementNames;
	TMap<FName,FString> AvailableCellBindings;

	if ( OwnerList->DataProvider )
	{
		TScriptInterface<IUIListElementCellProvider> CellProvider = OwnerList->DataProvider->GetElementCellSchemaProvider(OwnerList->DataSource.DataStoreField);
		if ( CellProvider )
		{
			CellProvider->GetElementCellTags(OwnerList->DataSource.DataStoreField,AvailableCellBindings);
			AvailableCellBindings.GenerateKeyArray(ElementNames);

			if ( NumColumns == 1 )
			{
				ColumnIdx = 0;
			}

			const UBOOL bIsComboBoxList = OwnerList->GetOuter()->IsA(UUIComboBox::StaticClass());

			// If there was a column index specified, then generate a menu only for that column.  Otherwise, generate a menu that
			// has submenus for each column.
			if(ColumnIdx > -1)
			{
				static UScriptStruct* UIListElementCellStruct = FindField<UScriptStruct>(UUIComp_ListPresenter::StaticClass(), TEXT("UIListElementCell"));
				static UStructProperty* CellStyleProperty = FindFieldWithFlag<UStructProperty,CASTCLASS_UStructProperty>(UIListElementCellStruct,TEXT("CellStyle"));

				// add a submenu to allow the designer to select the cell-specific styles for this column
				FStyleReferenceId CellStyleId(CellStyleProperty);
				FStylePropertyReferenceGroup CellStyleGroup(CellStyleId);
				FWidgetStyleReference* CellStyleReference = new(CellStyleGroup.WidgetStyleReferences) FWidgetStyleReference(OwnerList);
				for ( INT CellStateIndex = 0; CellStateIndex < ELEMENT_MAX; CellStateIndex++ )
				{
					FUIStyleReference* CustomCellStyleRef = OwnerList->CellDataComponent->GetCustomCellStyle(static_cast<EUIListElementState>(CellStateIndex), ColumnIdx);
					CellStyleReference->References.AddItem(CustomCellStyleRef);
				}

				wxMenu* StyleSelectionMenu = new wxMenu;
				Append( IDM_UI_SELECTSTYLE, *LocalizeUI(TEXT("UIEditor_MenuItem_SelectStyle")), StyleSelectionMenu );
				BuildStyleSelectionSubmenus(StyleSelectionMenu, CellStyleGroup, ColumnIdx);

				// Submenu to append a new column to the list.
				if ( !bIsComboBoxList )
				{
					wxMenu* AppendMenu = new wxMenu;
					CreateColumnMenu(OwnerList, ElementNames, AppendMenu, NumColumns);
					Append( IDM_UI_LISTCELL_APPEND, *LocalizeUI( TEXT("UIEditor_AppendColumn") ), AppendMenu );

					wxMenu* InsertMenu = new wxMenu;
					CreateColumnMenu(OwnerList, ElementNames, InsertMenu, NumColumns+1);
					Append( IDM_UI_LISTCELL_INSERT, *LocalizeUI(TEXT("UIEditor_InsertColumn")), InsertMenu );
				}

				// add the items for changing the cell binding for the selected column
				CreateColumnMenu(OwnerList, ElementNames, this, ColumnIdx);
			}
			else
			{
				if ( !bIsComboBoxList )
				{
					// Submenu to append a new column to the list.
					wxMenu* AppendMenu = new wxMenu;
					CreateColumnMenu(OwnerList, ElementNames, AppendMenu, NumColumns);
					Append(wxID_ANY, *LocalizeUI( TEXT("UIEditor_AppendColumn") ), AppendMenu );
				}

				for(INT ColumnIdx = 0; ColumnIdx < NumColumns; ColumnIdx++)
				{
					wxMenu* ColumnMenu = new wxMenu;
					CreateColumnMenu(OwnerList, ElementNames, ColumnMenu, ColumnIdx);

					const FString ColumnName = FString::Printf( LocalizeSecure(LocalizeUI(TEXT("UIEditor_ColumnF")), ColumnIdx));
					Append(wxID_ANY, *ColumnName, ColumnMenu);
				}
			}
		}
	}
}

/**
 * Generates a menu for the column index specified using all of the available data tags as options.
 *
 * @param InList			List to use to generate the menu.
 * @param ElementNames		Array of available data tags.
 * @param ColumnMenu		Menu to append items to.
 * @param ColumnIdx			Column that we will be generating a menu for.
 */
wxMenu* WxUIListBindMenu::CreateColumnMenu(UUIList* InList, const TArray<FName> &ElementNames, wxMenu* ColumnMenu, INT ColumnIdx)
{
	// Create a submenu using all of the available cell tags.  If the current column's data field already uses a tag, then check the menu item for that tag.
	FName ExistingCellBinding = InList->GetCellBinding(ColumnIdx);

	const UBOOL bIsComboBoxList = InList->GetOuter()->IsA(UUIComboBox::StaticClass());
	INT MenuIdx = IDM_UI_LISTELEMENTTAG_START + ColumnIdx * (ElementNames.Num() + (bIsComboBoxList ? 0 : 1));

	if ( !bIsComboBoxList && ExistingCellBinding != NAME_None )
	{
		ColumnMenu->Append(MenuIdx, *LocalizeUI(TEXT("UIEditor_MenuItem_ClearBinding")));
		ColumnMenu->AppendSeparator();
		MenuIdx++;
	}

	for(INT NameIdx = 0; NameIdx < ElementNames.Num(); NameIdx++)
	{
		const FName &ElementName = ElementNames(NameIdx);
		wxMenuItem* Item = ColumnMenu->AppendCheckItem(MenuIdx, *ElementName.ToString());

		if(ElementName == ExistingCellBinding && ExistingCellBinding != NAME_None)
		{
			Item->Check(true);
		}

		MenuIdx++;
	}

	return ColumnMenu;
}

/*-----------------------------------------------------------------------------
	WxUICalloutButtonPanelMenu
-----------------------------------------------------------------------------*/
/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 * Appends the menu options.
 *
 * @param	InActivePanel	the panel that was clicked on to invoke this context menu
 * @param	ActiveButton	the button the user clicked on; NULL if the user clicked on the panel itself
 */
void WxUICalloutButtonPanelMenu::Create( UUICalloutButtonPanel* InActiveWidget, UUICalloutButton* ActiveButton/*=NULL*/ )
{
	WxUIContextMenu::Create(InActiveWidget);

	Append(IDM_UI_CALLOUTBUTTON_ADD, *LocalizeUI(TEXT("UIEditor_MenuItem_AddCalloutButton")));
	if ( ActiveButton != NULL )
	{
		Append(IDM_UI_CALLOUTBUTTON_REMOVE, *LocalizeUI(TEXT("UIEditor_MenuItem_RemoveCalloutButton")));
	}

	if ( ActiveButton != NULL )
	{
		AppendSeparator();
		Append(IDM_UI_CALLOUTBUTTON_CHANGEALIAS, *LocalizeUI(TEXT("UIEditor_MenuItem_ChangeCalloutAlias")));
	}
}

// EOF

























