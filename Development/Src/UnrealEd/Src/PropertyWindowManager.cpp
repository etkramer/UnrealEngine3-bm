/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "PropertyWindowManager.h"
#include "Properties.h"
#include "BusyCursor.h"

IMPLEMENT_CLASS(UPropertyWindowManager);	

///////////////////////////////////////////////////////////////////////////////
//
// InitPropertySubSystem.
//
///////////////////////////////////////////////////////////////////////////////

/**
 * Allocates and initializes GPropertyWindowManager.  Called by WxLaunchApp::OnInit() to initialize.  Not rentrant.
 */
void InitPropertySubSystem()
{
	check( GPropertyWindowManager == NULL );
	GPropertyWindowManager = new UPropertyWindowManager;
	GPropertyWindowManager->AddToRoot();
	const UBOOL bWasInitializationSuccessful = GPropertyWindowManager->Initialize();
	check( bWasInitializationSuccessful );
}

///////////////////////////////////////////////////////////////////////////////
//
// UPropertyWindowManager -- property window manager.
//
///////////////////////////////////////////////////////////////////////////////

UPropertyWindowManager::UPropertyWindowManager()
	:	bInitialized( FALSE )
{
	GConfig->GetBool(TEXT("EditorFrame"), TEXT("ShowAllPropertyItemButtons"), bShowAllItemButtons, GEditorUserSettingsIni);
	GConfig->GetBool(TEXT("EditorFrame"), TEXT("ShowOnlyModifiedProperties"), bShowOnlyModifiedItems, GEditorUserSettingsIni);
}

/**
 * Loads common bitmaps.  Safely reentrant.
 *
 * @return		TRUE on success, FALSE on fail.
 */
UBOOL UPropertyWindowManager::Initialize()
{
	if ( !bInitialized )
	{
		if ( !ArrowDownB.Load( TEXT("DownArrowLarge") ) )					return FALSE;
		if ( !ArrowRightB.Load( TEXT("RightArrowLarge") ) )					return FALSE;
		if ( !CheckBoxOnB.Load( TEXT("CheckBox_On") ) )						return FALSE;
		if ( !CheckBoxOffB.Load( TEXT("CheckBox_Off") ) )					return FALSE;
		if ( !CheckBoxUnknownB.Load( TEXT("CheckBox_Unknown") ) )			return FALSE;
		if ( !Prop_AddNewItemB.Load( TEXT("Prop_Add") ) )					return FALSE;
		if ( !Prop_RemoveAllItemsFromArrayB.Load( TEXT("Prop_Empty") ) )	return FALSE;
		if ( !Prop_InsertNewItemHereB.Load( TEXT("Prop_Insert") ) )			return FALSE;
		if ( !Prop_DeleteItemB.Load( TEXT("Prop_Delete") ) )				return FALSE;
		if ( !Prop_ShowGenericBrowserB.Load( TEXT("Prop_Browse") ) )		return FALSE;
		if ( !Prop_UseMouseToPickColorB.Load( TEXT("Prop_Pick") ) )			return FALSE;
		if ( !Prop_ClearAllTextB.Load( TEXT("Prop_Clear") ) )				return FALSE;
		if ( !Prop_UseMouseToPickActorB.Load( TEXT("Prop_Find") ) )			return FALSE;
		if ( !Prop_UseCurrentBrowserSelectionB.Load( TEXT("Prop_Use") ) )	return FALSE;
		if ( !Prop_NewObjectB.Load( TEXT("Prop_NewObject") ) )				return FALSE;
		if ( !Prop_DuplicateB.Load( TEXT("Prop_Duplicate") ) )				return FALSE;
		if ( !Prop_ResetToDefaultB.Load( TEXT("Prop_ResetToDefault") ) )			return FALSE;

		bInitialized = TRUE;
	}

	return TRUE;
}

/**
 * Property window management.  Registers a property window with the manager.
 *
 * @param	InPropertyWindow		The property window to add to managing control.  Must be non-NULL.
 */
void UPropertyWindowManager::RegisterWindow(WxPropertyWindow* InPropertyWindow)
{
	check( InPropertyWindow );
	PropertyWindows.AddItem( InPropertyWindow );
}

/**
 * Property window management.  Unregisters a property window from the manager.
 *
 * @param	InPropertyWindow		The property window to remove from managing control.
 */
void UPropertyWindowManager::UnregisterWindow(WxPropertyWindow* InPropertyWindow)
{
	PropertyWindows.RemoveItem( InPropertyWindow );
}

/**
 * Dissociates all set objects and hides windows.
 */
void UPropertyWindowManager::ClearReferencesAndHide()
{
	for( INT WindowIndex=0; WindowIndex<PropertyWindows.Num(); WindowIndex++ )
	{
		// Only hide if we are flagged as being able to be hidden.
		WxPropertyWindow* PropertyWindow = PropertyWindows(WindowIndex);
		check(PropertyWindow);

		const UBOOL bHideable = PropertyWindow->GetCanBeHiddenByPropertyWindowManager();
		if( bHideable )
		{
			PropertyWindow->SetObject( NULL, FALSE, TRUE, TRUE, FALSE );
			PropertyWindow->ClearLastFocused();
		}
	}
}


/**
 * Serializes all managed property windows to the specified archive.
 *
 * @param		Ar		The archive to read/write.
 */
void UPropertyWindowManager::Serialize(FArchive& Ar)
{
	Super::Serialize( Ar );

	for( INT WindowIndex = 0 ; WindowIndex < PropertyWindows.Num() ; ++WindowIndex )
	{
		WxPropertyWindow* PropertyWindow = PropertyWindows(WindowIndex);
		PropertyWindow->Serialize( Ar );
	}
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UPropertyWindowManager::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	Super::AddReferencedObjects( ObjectArray );
	
	// Collect object references...
	TArray<UObject*> CollectedReferences;
	FArchiveObjectReferenceCollector ObjectReferenceCollector( &CollectedReferences );
	Serialize( ObjectReferenceCollector );
	
	// ... and add them.
	for( INT ObjectIndex=0; ObjectIndex<CollectedReferences.Num(); ObjectIndex++ )
	{
		UObject* Object = CollectedReferences(ObjectIndex);
		AddReferencedObject( ObjectArray, Object );
	}
}

/**
* Toggles whether or not the buttons for each property item are shown
* regardless of whether or not they currently have focus.
*
* @param	bShowButtons	TRUE if all buttons are to be shown.
*/
void UPropertyWindowManager::SetShowAllItemButtons(const UBOOL bShowButtons)
{
	if(bShowAllItemButtons == bShowButtons)
	{
		return;
	}

	bShowAllItemButtons = bShowButtons;

	wxCommandEvent Event;
	Event.SetEventType(ID_TOGGLE_SHOW_ALL_BUTTONS);

	FScopedBusyCursor Cursor;
	for(int Index = 0; Index < PropertyWindows.Num(); ++Index)
	{
		Event.SetEventObject(PropertyWindows(Index));
		PropertyWindows(Index)->GetEventHandler()->ProcessEvent(Event);
	}
}

/**
 * Toggles whether or not only property items that have been modified are shown.
 *
 * @param	bShowModifiedOnly	TRUE if only modified property items are to be shown.
 */
void UPropertyWindowManager::SetShowOnlyModifiedItems(const UBOOL bShowModifiedOnly)
{
	if(bShowOnlyModifiedItems != bShowModifiedOnly)
	{
		bShowOnlyModifiedItems = bShowModifiedOnly;

		FScopedBusyCursor Cursor;
		for(int Index = 0; Index < PropertyWindows.Num(); ++Index)
		{
			PropertyWindows(Index)->Rebuild();
		}
	}
}
