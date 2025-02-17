/*=============================================================================
	UnUIEditor.cpp: UI editor class implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"
#include "CanvasScene.h"
#include "..\..\Engine\Src\SceneRenderTargets.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"

#include "ImageUtils.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIEditorStatusBar.h"
#include "UnLinkedObjDrawUtils.h"
#include "UIWidgetTool.h"
#include "UIDragTools.h"
#include "UIDataStoreBrowser.h"

#include "BusyCursor.h"
#include "ScopedObjectStateChange.h"

// Registration
IMPLEMENT_CLASS(UUISceneManager);
IMPLEMENT_CLASS(UEditorUISceneClient);
IMPLEMENT_CLASS(UUIEditorOptions);
IMPLEMENT_CLASS(UUIPreviewString);

IMPLEMENT_CLASS(UUISceneFactory);
	IMPLEMENT_CLASS(UUISceneFactoryNew);
	IMPLEMENT_CLASS(UUISceneFactoryText);

IMPLEMENT_CLASS(UUISceneExporter);
IMPLEMENT_CLASS(UUILayer);
	IMPLEMENT_CLASS(UUILayerRoot);

/**
 * Loads the specified key from the UIEditor section of the UnrealEd .int file
 * 
 * @param	Key			the key to load
 * @param	bOptional	TRUE to ignore missing localization values
 *
 * @return	the value corresponding to the key specified.  For keys that can't be found, if
 * bOptional is FALSE, the result will be a localization error string indicating which key was missing.
 * If bOptional is TRUE, the result will be an empty string.
 */
FString LocalizeUI( const TCHAR* Key, UBOOL bOptional )
{
	return Localize( TEXT("UIEditor"), Key, TEXT("UnrealEd"), NULL, bOptional );
}

/**
 * Loads the specified key from the UIEditor section of the UnrealEd .int file
 * 
 * @param	Key			the key to load
 * @param	bOptional	TRUE to ignore missing localization values
 *
 * @return	the value corresponding to the key specified.  For keys that can't be found, if
 * bOptional is FALSE, the result will be a localization error string indicating which key was missing.
 * If bOptional is TRUE, the result will be an empty string.
 */
FString LocalizeUI( const ANSICHAR* Key, UBOOL bOptional )
{
	return LocalizeUI(ANSI_TO_TCHAR(Key), bOptional );
}


/* ==========================================================================================================
	UUISceneManager
========================================================================================================== */
FUIObjectResourceInfo::FUIObjectResourceInfo( UUIObject* InResource )
: FUIResourceInfo(InResource)
{
}

FUIStyleResourceInfo::FUIStyleResourceInfo( UUIStyle_Data* InResource )
: FUIResourceInfo(InResource)
{
}

FUIStateResourceInfo::FUIStateResourceInfo( UUIState* InResource )
: FUIResourceInfo(InResource)
{
}

/**
 * Initializes the singleton data store client that will manage the global data stores.
 */
void UUISceneManager::InitializeGlobalDataStore()
{
	if ( DataStoreManager == NULL )
	{
		DataStoreManager = CreateGlobalDataStoreClient(this);
		DataStoreManager->InitializeDataStores();
	}
}

/**
 * Returns the data store client singleton used by the editor.
 */
UDataStoreClient* GetEditorDataStoreClient()
{
	UDataStoreClient* DSClient = NULL;

	if ( GUnrealEd != NULL && GUnrealEd->GetBrowserManager() != NULL
	&&	GUnrealEd->GetBrowserManager()->UISceneManager != NULL )
	{
		DSClient = GUnrealEd->GetBrowserManager()->UISceneManager->DataStoreManager;
	}
	return DSClient;
}

/**
 * Performs all initialization for the UI editor system.
 */
void UUISceneManager::Initialize()
{
	// Initialize the lookup maps for UIEvent Keys
	UUIInteraction* DefaultUIInteraction = UUIRoot::GetDefaultUIController();
	DefaultUIInteraction->InitializeUIInputAliasNames();
	DefaultUIInteraction->InitializeInputAliasLookupTable();

	// create the global data store manager
	InitializeGlobalDataStore();

	// set the scene factory's pointer back to us.  by setting this on the scene factory's default object,
	// this should cause it to be set correctly for all instances of the scene factory
	UUISceneFactoryNew* SceneFactory = UUISceneFactoryNew::StaticClass()->GetDefaultObject<UUISceneFactoryNew>();
	SceneFactory->SetSceneManager(this);
	InitUIResources();

	// if we don't have an active skin, create one it now
	if ( ActiveSkin == NULL )
	{
		SetActiveSkin(LoadInitialSkin());
	}

	if ( GCallbackEvent != NULL )
	{
		GCallbackEvent->Register(CALLBACK_UIEditor_SkinLoaded, this);
	}
}

/**
 * Loads the initial UISkin to use for rendering scenes
 *
 * @return	a pointer to the UISkin object corresponding to the default UI Skin
 */
UUISkin* UUISceneManager::LoadInitialSkin() const
{
	// the initial UI skin is located in the UIInteraction class, which is game-specific, so we need to use the accessor so that we get the correct class
	UUIInteraction* DefaultUIController = UUIRoot::GetDefaultUIController();

	// now load the configured initial skin
	UUISkin* InitialSkin = LoadObject<UUISkin>(NULL, *DefaultUIController->UISkinName, NULL, 0, NULL);
	check(InitialSkin);

	return InitialSkin;
}

/**
 * Changes the active skin to the skin specified
 *
 * @param	NewActiveSkin	the skin that should now be active
 *
 * @return	TRUE if the active skin was successfully changed
 */
UBOOL UUISceneManager::SetActiveSkin( UUISkin* NewActiveSkin )
{
	checkSlow(DataStoreManager);

	UBOOL bResult = FALSE;

	UUIDataStore* StylesDataStore = DataStoreManager->FindDataStore(TEXT("Styles"));
	if ( StylesDataStore != NULL )
	{
		DataStoreManager->UnregisterDataStore(StylesDataStore);
	}

	//@todo - hmm, if the previously active style was unregistered, but the new skin failed to register, the scene manager now has no active skin
	// what should I do in that case....perhaps re-register the old skin?

	if ( DataStoreManager->RegisterDataStore(NewActiveSkin) )
	{
		ActiveSkin = NewActiveSkin;

		//@todo - hmmm, should we always do this?
		ActiveSkin->Initialize();

		for ( INT SceneIndex = 0; SceneIndex < SceneClients.Num(); SceneIndex++ )
		{
			UEditorUISceneClient* Client = SceneClients(SceneIndex);
			Client->SetActiveSkin(NewActiveSkin);
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Create a new scene client and associates it with the specified scene.
 *
 * @param	Scene	the scene to be associated with the new scene client.
 */
UEditorUISceneClient* UUISceneManager::CreateSceneClient( UUIScene* Scene )
{
	check(Scene);

	// if we don't have an active skin, create one it now
	if ( ActiveSkin == NULL )
	{
		SetActiveSkin(LoadInitialSkin());
	}

	UEditorUISceneClient* SceneClient = ConstructObject<UEditorUISceneClient>(UEditorUISceneClient::StaticClass(), this, NAME_None, RF_Transient);
	SceneClient->SceneManager = this;
	SceneClient->Scene = Scene;
	SceneClient->DataStoreManager = DataStoreManager;
	Scene->SceneClient = SceneClient;

	SceneClient->InitializeClient(ActiveSkin);
	SceneClients.AddUniqueItem(SceneClient);

	if ( Scene->HasAnyFlags(RF_Public) )
	{
		// if the scene isn't marked public, then this scene is a UIPrefabScene
		check(!Scene->IsA(UUIPrefabScene::StaticClass()));
		check(!Scene->HasAnyFlags(RF_Transient));

		for ( TObjectIterator<UObject> It; It; ++It )
		{
			if ( It->HasAnyFlags(RF_Public) && It->IsIn(Scene) )
			{
				It->Modify(TRUE);
				It->ClearFlags(RF_Public);
			}
		}
	}

	return SceneClient;
}

/**
 * Creates a new UIScene in the package specified.
 *
 * @param	SceneTemplate	the template to use for the new scene
 * @param	InOuter			the outer for the scene
 * @param	SceneTag		if specified, the scene will be given this tag when created
 *
 * @return	a pointer to the new UIScene that was created.
 */
UUIScene* UUISceneManager::CreateScene( UUIScene* SceneTemplate, UObject* InOuter, FName SceneTag /*=NAME_None*/ )
{
	UUIScene* CreatedScene = NULL;
	check(SceneTemplate!=NULL);

	UClass* SceneClass = SceneTemplate->GetClass();
	if ( SceneTag == NAME_None )
	{
		SceneTag = SceneTemplate->SceneTag;
	}
	FObjectDuplicationParameters Parameters(SceneTemplate, InOuter);
	Parameters.DestName = SceneTag;
	Parameters.DestClass = SceneClass;
	Parameters.FlagMask = ~(RF_RootSet|RF_DisregardForGC|RF_ArchetypeObject);
	Parameters.ApplyFlags = RF_Transactional;
	Parameters.bMigrateArchetypes = TRUE;

	CreatedScene = Cast<UUIScene>(StaticDuplicateObjectEx(Parameters));
	CreatedScene->SetFlags(RF_Public|RF_Standalone);

	// the Scene's SceneTag will be set to the name of the scene (which is now something like FooScene_2) in UUIScene::PostDuplicate;
	// so change it back to match the scene resource's tag manually
	CreatedScene->SceneTag = SceneTag;
	CreatedScene->Created(NULL);

	return CreatedScene;
}

IMPLEMENT_COMPARE_CONSTREF( FUIResourceInfo, UnUIEditor, { return appStricmp( *A.FriendlyName, *B.FriendlyName ); } )

/**
 * Builds the list of available widget, style, and state resources.
 *
 * @param	bRefresh	if FALSE, only builds the list if the list of resources is currently empty.
 *						if TRUE, clears the existing list of resources first
 */
void UUISceneManager::InitUIResources( UBOOL bRefresh/*=FALSE*/ )
{
	if ( bRefresh )
	{
		ClearUIResources();
	}

	if ( UIWidgetResources.Num() == 0 )
	{
		for ( FObjectIterator It; It; ++It )
		{
			if ( It->IsA(UUIObject::StaticClass()) )
			{
				UUIObject* Resource = (UUIObject*)(*It);
				if ( IsValidUIResource(Resource) )
				{
					new(UIWidgetResources) FUIObjectResourceInfo(Resource);
				}
			}
			else if ( It->IsA(UUIStyle_Data::StaticClass()) )
			{
				UUIStyle_Data* Resource = (UUIStyle_Data*)(*It);
				if ( IsValidStyleResource(Resource) )
				{
					new(UIStyleResources) FUIStyleResourceInfo(Resource);
				}
			}
			else if ( It->IsA(UUIState::StaticClass()) )
			{
				UUIState* Resource = (UUIState*)(*It);
				if ( IsValidStateResource(Resource) )
				{
					FUIStateResourceInfo* StateInfo = new(UIStateResources) FUIStateResourceInfo(Resource);

					UIStateResourceInfoMap.Set(Resource->GetClass(), StateInfo);
				}
			}
		}

		Sort<USE_COMPARE_CONSTREF(FUIResourceInfo,UnUIEditor)>( &UIWidgetResources(0), UIWidgetResources.Num() );
		Sort<USE_COMPARE_CONSTREF(FUIResourceInfo,UnUIEditor)>( &UIStyleResources(0), UIStyleResources.Num() );
		Sort<USE_COMPARE_CONSTREF(FUIResourceInfo,UnUIEditor)>( &UIStateResources(0), UIStateResources.Num() );
	}
}


/**
 * Cleans up the memory allocated for the UIClasses list.
 */
void UUISceneManager::ClearUIResources()
{
	UIWidgetResources.Empty(UIWidgetResources.Num());
	UIStyleResources.Empty(UIStyleResources.Num());
	UIStateResources.Empty(UIStateResources.Num());
	UIStateResourceInfoMap.Empty();
}

/**
 * Determines whether the specified UI resource should be included in the list of placeable widgets.
 *
 * @param	UIResource		a UIObject archetype
 */
UBOOL UUISceneManager::IsValidUIResource( UUIObject* UIResource ) const
{
	UBOOL bValidResource
		=  UIResource != NULL
		&& (UIResource->HasAnyFlags(RF_ClassDefaultObject) || UIResource->IsA(UUIPrefab::StaticClass()))
		&& UIResource->GetClass()->HasAnyClassFlags(CLASS_Placeable)
		&& !UIResource->GetClass()->HasAnyClassFlags(CLASS_Abstract);

	return bValidResource;
}

/**
 * Determines whether the specified style data should be included in the list of useable styles.
 *
 * @param	StyleResource	a UUIStyle_Data archetype
 */
UBOOL UUISceneManager::IsValidStyleResource( UUIStyle_Data* StyleResource ) const
{
	UBOOL bValidResource
		=  StyleResource != NULL
		&& StyleResource->HasAnyFlags(RF_ClassDefaultObject)
		&& !StyleResource->GetClass()->HasAnyClassFlags(CLASS_Abstract);

	return bValidResource;
}

/**
 * Determines whether the specified state should be included in the list of available UI states.
 *
 * @param	StateResource	a UIState archetype
 */
UBOOL UUISceneManager::IsValidStateResource( UUIState* StateResource ) const
{
	UBOOL bValidResource
		=  StateResource != NULL
		&& StateResource->HasAnyFlags(RF_ClassDefaultObject)
		&& !StateResource->GetClass()->HasAnyClassFlags(CLASS_Abstract);

	return bValidResource;
}

/**
 * Retrieves the list of UIStates which are supported by the specified widget.
 *
 * @param	out_SupportedStates		the list of UIStates supported by the specified widget class.
 * @param	WidgetClass				if specified, only those states supported by this class will be returned.  If not
 *									specified, all states will be returned.
 */
void UUISceneManager::GetSupportedUIStates( TArray<FUIStateResourceInfo>& out_SupportedStates, UClass* WidgetClass/*=NULL*/ ) const
{
	out_SupportedStates.Empty();

	for ( INT StateIndex = 0; StateIndex < UIStateResources.Num(); StateIndex++ )
	{
		const FUIStateResourceInfo& StateInfo = UIStateResources(StateIndex);
		UUIState* MenuState = Cast<UUIState>(StateInfo.UIResource);
		if ( WidgetClass == NULL || MenuState->eventIsWidgetClassSupported(WidgetClass) )
		{
			new(out_SupportedStates) FUIStateResourceInfo(StateInfo);
		}
	}
}

/**
 * Checks to see if any widgets in any of the scenes are currently referencing the passed in style.  If so,
 * it displays a message box asking the user if they still want to go through with the action.
 *
 * @param	StyleToDelete	Style that we want to delete/replace.
 * @param	bIsReplace		Whether or not this is a replacement operation, if so, display a different message to the user.
 *
 * @return Whether or not we should proceed with the delete/replace operation.
 */
UBOOL UUISceneManager::ShouldDeleteStyle(UUIStyle* StyleToDelete, UBOOL bIsReplace) const
{
	UBOOL bDeleteStyle = TRUE;

	TArray<UPackage*> NonFullyLoadedPackages;

	// first, make sure that all packages which contain UIScenes are fully loaded so that we can be as reliable as possible when determining
	// whether widgets reference this style
	for ( TObjectIterator<UUIRoot> It; It; ++It )
	{
		if ( It->IsA(UUIObject::StaticClass()) || It->IsA(UUIStyle::StaticClass()) )
		{
			UPackage* ScenePackage = It->GetOutermost();
			if ( !ScenePackage->IsFullyLoaded() )
			{
				// this package isn't fully loaded, so warn the user about this
				NonFullyLoadedPackages.AddUniqueItem(ScenePackage);
			}
		}
	}

	if ( NonFullyLoadedPackages.Num() > 0 )
	{
		// if not all packages were fully loaded, do not allow the delete/replace
		FString ErrorMsg = LocalizeUI(TEXT("UIEditor_MustFullyLoadPackages"));
		ErrorMsg += TEXT("\n\n");

		for ( INT PackageIndex = 0; PackageIndex < NonFullyLoadedPackages.Num(); PackageIndex++ )
		{
			ErrorMsg += NonFullyLoadedPackages(PackageIndex)->GetName();
			ErrorMsg += LINE_TERMINATOR;
		}

		ErrorMsg += LINE_TERMINATOR;
		appMsgf(AMT_OK, *ErrorMsg);
		bDeleteStyle = FALSE;
	}
	else
	{
		// search for widgets that are using this style
		TArray<const UUIObject*> BoundWidgets;
		for ( TObjectIterator<UUIObject> It; It; ++It )
		{
			UUIObject* Widget = *It;
			if ( Widget->GetOutermost() != UObject::GetTransientPackage() && Widget->UsesStyle(StyleToDelete) )
			{
				BoundWidgets.AddItem(Widget);
			}
		}

		TArray<const UUIStyle*> ReferencingStyles;
		for ( TObjectIterator<UUIStyle> It; It; ++It )
		{
			UUIStyle* CurrentStyle = *It;
			if ( CurrentStyle != StyleToDelete && CurrentStyle->ReferencesStyle(StyleToDelete) )
			{
				ReferencingStyles.AddUniqueItem(CurrentStyle);
			}
		}

		if( BoundWidgets.Num() > 0 || ReferencingStyles.Num() > 0 )
		{
			FString ErrorMsg = FString::Printf(LocalizeSecure(LocalizeUI("UIEditor_StyleReferenced"), *StyleToDelete->StyleName));

			ErrorMsg += TEXT("\n\n");

			for(INT WidgetIdx = 0; WidgetIdx<BoundWidgets.Num();WidgetIdx++)
			{
				ErrorMsg += BoundWidgets(WidgetIdx)->GetWidgetPathName();
				ErrorMsg += LINE_TERMINATOR;
			}

			for ( INT StyleIndex = 0; StyleIndex < ReferencingStyles.Num(); StyleIndex++ )
			{
				ErrorMsg += *ReferencingStyles(StyleIndex)->GetPathName();
				ErrorMsg += LINE_TERMINATOR;
			}

			ErrorMsg += LINE_TERMINATOR;

			// Switch up the text depending on whether this is a replace or delete operation.
			if(bIsReplace == TRUE)
			{
				ErrorMsg += *LocalizeUI("UIEditor_DoYouWantToReplaceStyle");
			}
			else
			{
				ErrorMsg += *LocalizeUI("UIEditor_DoYouWantToDeleteStyle");
			}
			
			bDeleteStyle = appMsgf(AMT_YesNo, *ErrorMsg);
		}
	}

	return bDeleteStyle;
}

/**
 * Find the position for the scene client window containing the scene specified.
 *
 * @param	Scene	the scene to search for
 *
 * @return	the index into array of SceneClients arry for the scene client associated with the scene specified,
 *			or INDEX_NONE if that scene has never been edited
 */
INT UUISceneManager::FindSceneIndex( UUIScene* Scene ) const
{
	INT Result = INDEX_NONE;
	for ( INT i = 0; i < SceneClients.Num(); i++ )
	{
		UEditorUISceneClient* SceneClient = SceneClients(i);
		if ( SceneClient->Scene == Scene )
		{
			Result = i;
			break;
		}
	}

	return Result;
}

/**
 * Retrieves the set of selected widgets for scene specified.
 *
 * @param	Scene	the scene to get the selected widgets for
 * @param	out_SelectedWidgets		will be filled with the selected widgets from the specified scene
 *
 * @return	TRUE if out_SelectedWidgets was successfully filled with widgets from the specified scene, or 
 *			FALSE if the scene specified isn't currently being edited.
 */
UBOOL UUISceneManager::GetSelectedSceneWidgets( UUIScene* Scene, TArray<UUIObject*>& out_SelectedWidgets ) const
{
	UBOOL bResult = FALSE;

	if ( Scene != NULL )
	{
		// find the scene client for the scene specified
		INT SceneIndex = FindSceneIndex(Scene);
		if ( SceneClients.IsValidIndex(SceneIndex) )
		{
			UEditorUISceneClient* SceneClient = SceneClients(SceneIndex);

			// get the selection set from this scene's window
			SceneClient->SceneWindow->GetSelectedWidgets(out_SelectedWidgets,TRUE);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Sets the selected widgets for the scene editor associated with the scene specified
 *
 * @param	Scene				the scene to get the selected widgets for
 * @param	SelectedWidgets		the list of widgets to mark as selected
 *
 * @return	TRUE if the selection set was accepted
 */
UBOOL UUISceneManager::SetSelectedSceneWidgets( class UUIScene* Scene, TArray<class UUIObject*>& SelectedWidgets )
{
	UBOOL bResult = FALSE;

	if ( Scene != NULL )
	{
		// find the scene client for the scene specified
		INT SceneIndex = FindSceneIndex(Scene);
		if ( SceneClients.IsValidIndex(SceneIndex) )
		{
			UEditorUISceneClient* SceneClient = SceneClients(SceneIndex);

			// set the selection set for this scene's window
			bResult = SceneClient->SceneWindow->SetSelectedWidgets(SelectedWidgets);
		}
	}

	return bResult;
}

/**
 * Called when the user requests to edit a UIScene.  Creates a new scene client (or finds an existing scene client, if
 * this isn't the first time the scene has been edited during this session) to handle initialization and de-initialization
 * of the scene, and passes the edit scene request to the scene client.
 *
 * @param	Scene	the scene to open
 *
 * @return	TRUE if the scene was successfully opened and initialized
 */
UBOOL UUISceneManager::OpenScene( UUIScene* Scene )
{
	UEditorUISceneClient* SceneClient = NULL;

	INT SceneClientIndex = FindSceneIndex(Scene);
	if ( SceneClientIndex == INDEX_NONE )
	{
		// this scene has never been opened for edit - create a new scene client that will handle initialization and rendering for this scene
		SceneClient = CreateSceneClient(Scene);
	}
	else
	{
		SceneClient = SceneClients(SceneClientIndex);
	}

	return SceneClient->OpenScene(Scene);
}

/**
 * Called when the editor window for the specified scene is closed.  Passes the notification to the appropriate
 * scene client for further processing.
 *
 * @param	Scene	the scene to deactivate
 */
void UUISceneManager::SceneClosed( UUIScene* Scene )
{
	INT SceneClientIndex = FindSceneIndex(Scene);
	if ( SceneClientIndex != INDEX_NONE )
	{
		SceneClients(SceneClientIndex)->CloseScene(Scene);
	}
}

/**
 * Called when the user selects to delete a scene in the generic browser.
 */
void UUISceneManager::NotifySceneDeletion( UUIScene* Scene )
{
	INT SceneClientIndex = FindSceneIndex(Scene);
	if ( SceneClientIndex != INDEX_NONE )
	{
		SceneClients(SceneClientIndex)->NotifyDeleteScene();
		SceneClients.Remove(SceneClientIndex);
	}
}

/** @return Returns the pointer to the datastore browser dialog. */
WxDlgUIDataStoreBrowser* UUISceneManager::GetDataStoreBrowser()
{
	if(DlgUIDataStoreBrowser == NULL)
	{
		FScopedBusyCursor BusyCursor;
		GWarn->BeginSlowTask(*LocalizeUI(TEXT("UIEditor_SlowTask_DataStoreBrowser")), TRUE);

		// Create a instance of the datastore browser dialog.
		DlgUIDataStoreBrowser = new WxDlgUIDataStoreBrowser(GApp->EditorFrame, this);

		GWarn->EndSlowTask();
	}

	return DlgUIDataStoreBrowser;
}


/* ==============================================
	FExec interface
============================================== */
UBOOL UUISceneManager::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	if ( ParseCommand(&Cmd, TEXT("CLEANUPEVENTPROVIDERS")) )
	{
		TArray<UUIScene*> CurrentlyOpenScenes;
		for ( INT SceneClientIndex = 0; SceneClientIndex < SceneClients.Num(); SceneClientIndex++ )
		{
			UEditorUISceneClient* SceneClient = SceneClients(SceneClientIndex);
			if ( SceneClient->SceneWindow != NULL && SceneClient->Scene != NULL )
			{
				CurrentlyOpenScenes.AddUniqueItem(SceneClient->Scene);
			}
		}

		for ( TObjectIterator<UUIComp_Event> It; It; ++It )
		{
			UUIComp_Event* EventComponent = *It;
			UUIScreenObject* EventOwner = EventComponent->GetOwner();
			if ( EventOwner != NULL )
			{
				// make sure this event component is contained within an open scene, since this is the only case where
				// the owning widget should definitely have a reference to the owning scene
				for ( INT OpenSceneIndex = 0; OpenSceneIndex < CurrentlyOpenScenes.Num(); OpenSceneIndex++ )
				{
					if ( EventOwner->IsIn(CurrentlyOpenScenes(OpenSceneIndex)) )
					{
						if ( EventOwner->GetScene() == NULL )
						{
							debugf(TEXT("Cleaning up corrupted event provider '%s'"), *EventComponent->GetPathName());

							// if the widget that owns this event component doesn't have a valid reference to its owning scene, it's a widget that was renamed or deleted
							// but for some reason its EventProvider was never removed from its owner's global sequence, which is the only reference to that widget - kill it now
							EventComponent->CleanupEventProvider();
							CurrentlyOpenScenes(OpenSceneIndex)->MarkPackageDirty();
						}
						break;
					}
				}
			}
		}

		return TRUE;
	}
	else if ( ParseCommand(&Cmd, TEXT("CLEANUPLAYERS")) )
	{
		for ( INT SceneClientIndex = 0; SceneClientIndex < SceneClients.Num(); SceneClientIndex++ )
		{
			UEditorUISceneClient* SceneClient = SceneClients(SceneClientIndex);
			if ( SceneClient->SceneWindow != NULL && SceneClient->Scene != NULL )
			{
				Ar.Logf(TEXT("Please close all UI editor windows and fully load all UI packages before running this command!"));
				return TRUE;
			}
		}

		// remove any layers that were created using the old code
		for ( TObjectIterator<UUILayer> It; It; ++It )
		{
			UUILayer* CurrentLayer = *It;
			if ( CurrentLayer->GetOuter()->IsA(UPackage::StaticClass()) )
			{
				UUILayerRoot* RootLayer = Cast<UUILayerRoot>(CurrentLayer);
				if ( RootLayer != NULL )
				{
					Ar.Logf(TEXT("Deleting old root layer for %s (%s)"), RootLayer->Scene ? TEXT("NULL") : *RootLayer->Scene->GetName(), *RootLayer->GetFullName());
				}
				else
				{
					Ar.Logf(TEXT("Deleting old child layer %s (%s)"), *CurrentLayer->LayerName, *CurrentLayer->GetFullName());
				}
				CurrentLayer->Modify(TRUE);
				CurrentLayer->ClearFlags(RF_Standalone);
			}
		}

		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		return TRUE;
	}
// 	else if ( ParseCommand(&Cmd, TEXT("DUMPRENDERMODIFIERS")) )
// 	{
// 		for ( INT SceneClientIndex = 0; SceneClientIndex < SceneClients.Num(); SceneClientIndex++ )
// 		{
// 			UEditorUISceneClient* Client = SceneClients(SceneClientIndex);
// 			if ( Client != NULL && Client->SceneWindow != NULL && Client->Scene != NULL )
// 			{
// 				FEditorUIObjectViewportClient* ObjectVC = Client->SceneWindow->ObjectVC;
// 				if ( ObjectVC != NULL )
// 				{
// 					for ( INT ModIndex = 0; ModIndex < ObjectVC->RenderModifiers.Num(); ModIndex++ )
// 					{
// 						
// 					}
// 				}
// 			}
// 		}
// 		return TRUE;
// 	}
	for ( INT SceneClientIndex = 0; SceneClientIndex < SceneClients.Num(); SceneClientIndex++ )
	{
		if ( SceneClients(SceneClientIndex)->Exec(Cmd, Ar) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* === FCallbackEventDevice interface === */
/**
 * Called when a package containing a UISKin is loaded.
 *
 * @param	LoadedSkin	the skin that was loaded.
 */
void UUISceneManager::Send( ECallbackEventType InType, UObject* LoadedSkin )
{
	UUISkin* Skin = Cast<UUISkin>(LoadedSkin);
	if ( Skin != NULL )
	{
		// if this skin is the ActiveSkin, or is in the archetype of the ActiveSkin, re-initialize it if it has been reloaded from disk
		// We must re-initialize each one (even though UUISkin::Initialize() calls Initialize on its archetype) because the archetype might
		// be loaded AFTER the ActiveSkin, meaning that the archetype's styles have been recreated but still need to be re-initialized
		for ( UUISkin* CurrentSkin = ActiveSkin; CurrentSkin && !CurrentSkin->HasAnyFlags(RF_ClassDefaultObject); CurrentSkin = CurrentSkin->GetArchetype<UUISkin>() )
		{
			if ( CurrentSkin == Skin )
			{
				Skin->Initialize();
				break;
			}
		}
	}
}

/* ==========================================================================================================
	UEditorUISceneClient
========================================================================================================== */
/** Default constructor */
UEditorUISceneClient::UEditorUISceneClient()
: ClientCanvasScene(NULL)
{
}

/**
 * Called to finish destroying the object.
 */
void UEditorUISceneClient::FinishDestroy()
{
	DetachUIPrimitiveScene();
	Super::FinishDestroy();
}

/**
 * Retrieves the virtual offset for the viewport that renders the specified scene.  Only relevant in the UI editor.
 * Non-zero when the user has panned or zoomed the UI editor such that the 0,0 viewport position is no longer the same
 * as the 0,0 canvas location.
 *
 * @param	out_ViewportOffset	[out] will be filled in with the delta between the viewport's actual origin and virtual origin.
 *
 * @return	TRUE if the viewport origin was successfully retrieved
 */
UBOOL UEditorUISceneClient::GetViewportOffset( const UUIScene* Scene, FVector2D& out_ViewportOffset )
{
	UBOOL bResult = FALSE;

	out_ViewportOffset = FVector2D(0,0);
	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized && SceneWindow->ObjectVC != NULL )
	{
		out_ViewportOffset = SceneWindow->ObjectVC->Origin2D;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Retrieves the scale factor for the viewport that renders the specified scene.  Only relevant in the UI editor.
 */
FLOAT UEditorUISceneClient::GetViewportScale( const UUIScene* Scene ) const
{
	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized && SceneWindow->ObjectVC != NULL )
	{
		return SceneWindow->ObjectVC->Zoom2D;
	}

	return Super::GetViewportScale(Scene);
}


/**
 * Retrieves the virtual point of origin for the viewport that renders the specified scene
 *
 * In the game, this will be non-zero if Scene is for split-screen and isn't for the first player.
 * In the editor, this will be equal to the value of the gutter region around the viewport.
 *
 * @param	out_ViewportOrigin	[out] will be filled in with the position of the virtual origin point of the viewport.
 *
 * @return	TRUE if the viewport origin was successfully retrieved
 */
UBOOL UEditorUISceneClient::GetViewportOrigin( const UUIScene* Scene, FVector2D& out_ViewportOrigin )
{
	UBOOL bResult = FALSE;

	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized )
	{
		check(SceneWindow->EditorOptions);
		out_ViewportOrigin.X = out_ViewportOrigin.Y = SceneWindow->EditorOptions->ViewportGutterSize;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Retrieves the size of the viewport for the scene specified.
 *
 * @param	out_ViewportSize	[out] will be filled in with the width & height that the scene should use as the viewport size
 *
 * @return	TRUE if the viewport size was successfully retrieved
 */
UBOOL UEditorUISceneClient::GetViewportSize( const UUIScene* Scene, FVector2D& out_ViewportSize )
{
	UBOOL bResult = FALSE;

	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized )
	{
		check(SceneWindow->EditorOptions);

		SceneWindow->EditorOptions->GetVirtualViewportSize(RenderViewport, out_ViewportSize.X, out_ViewportSize.Y);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Recalculates the matrix used for projecting local coordinates into screen (normalized device)
 * coordinates.  This method should be called anytime the viewport size or origin changes.
 */
void UEditorUISceneClient::UpdateCanvasToScreen()
{
	FLOAT SizeX, SizeY;

	if ( RenderViewport != NULL )
	{
		SizeX = RenderViewport->GetSizeX();
		SizeY = RenderViewport->GetSizeY();
	}
	else
	{
		SizeX = UCONST_DEFAULT_SIZE_X;
		SizeY = UCONST_DEFAULT_SIZE_Y;
	}

	// local space to world space to normalized device coord.
	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized && SceneWindow->ObjectVC != NULL )
	{
		const FLOAT Zoom2D = SceneWindow->ObjectVC->Zoom2D;
		const FVector2D Origin2D = SceneWindow->ObjectVC->Origin2D;
		CanvasToScreen = 
			FScaleMatrix(Zoom2D) * 
			FTranslationMatrix(FVector(Origin2D.X,Origin2D.Y,0)) *
			FCanvas::CalcBaseTransform3D( SizeX, SizeY, 90, NEAR_CLIPPING_PLANE );
	}
	else
	{
		CanvasToScreen = FCanvas::CalcBaseTransform3D( SizeX, SizeY, 90, NEAR_CLIPPING_PLANE );
	}

	InvCanvasToScreen = CanvasToScreen.Inverse();
}

/**
 * Changes the active skin to the skin specified, initializes the skin and performs all necessary cleanup and callbacks.
 * This method should only be called from script.
 *
 * @param	NewActiveScene	The skin to activate
 *
 * @return	TRUE if the skin was successfully changed.
 */
UBOOL UEditorUISceneClient::ChangeActiveSkin( UUISkin* NewActiveSkin )
{
	UBOOL bResult = FALSE;

	// in the editor, the active skin must be updated via the scene manager so that everything remains synchronized
	// (i.e. all open scene clients are notified of the change, the scene manager has the right skin, etc. etc.)
	if ( SceneManager != NULL )
	{
		bResult = SceneManager->SetActiveSkin(NewActiveSkin);
	}

	return bResult;
}

/**
 * Refreshes all existing UI elements with the styles from the currently active skin.
 */
void UEditorUISceneClient::OnActiveSkinChanged()
{
	if ( Scene != NULL && Scene->IsInitialized() )
	{
		Scene->NotifyActiveSkinChanged();
	}
}


/**
 * Called when user opens the specified scene for editing.  Creates the editor window, initializes the scene's state,
 * and converts it into "editing" mode.
 *
 * @param	Scene			the scene to open
 *
 * @return TRUE if the scene was successfully opened
 */
UBOOL UEditorUISceneClient::OpenScene( UUIScene* Scene, ULocalPlayer*, UUIScene**, BYTE)
{
	Scene->SceneClient = this;

	// if we already have an editor window open for this scene, stop here
	if ( SceneWindow == NULL )
	{
		// this scene has never been opened for edit - create a new scene client that will handle initialization and rendering for this scene
		SceneWindow = new WxUIEditorBase( GApp->EditorFrame, -1, SceneManager, Scene );
		SceneWindow->InitEditor();
		SetRenderViewport(SceneWindow->ObjectVC->Viewport);
	}

	// register this scene client to receive notifications when the viewport is resized
	GCallbackEvent->Register(CALLBACK_ViewportResized, this);

	SceneWindow->Show(TRUE);
	return TRUE;
}

void UEditorUISceneClient::GenerateSceneThumbnail( UUIScene* Scene)
{
#if !CONSOLE

	// Construct a thumbnail from a scaled down version of the viewport image.  We will do this by reading the array of pixels from the viewport,
	// then resizing the image down to a 256x256 image.  Then finally, we will construct a texture that is only loaded in the editor but still saved
	// in the scene's package.
	const INT Width = 256;
	const INT Height = 256;

	//@todo - resize the viewport to match the size of the scene

	// Read the array of pixels from the viewport.
	TArray<FColor> OutputBuffer;
	RenderViewport->ReadPixels(OutputBuffer);

	// Create a array of colors to store our resized viewport image.
	TArray<FColor> SurfData;
	FImageUtils::ImageResize(RenderViewport->GetSizeX(), RenderViewport->GetSizeY(), OutputBuffer, Width, Height, SurfData);

	// Create a texture using our resized data.
	EObjectFlags Flags = RF_NotForServer | RF_NotForClient;
	FString NewTexName = FString::Printf(TEXT("%s_Preview"), *Scene->GetTag().ToString());

	Scene->ScenePreview = FImageUtils::ConstructTexture2D(Width, Height, SurfData, Scene, NewTexName, Flags);
	Scene->ScenePreview->LODGroup = TEXTUREGROUP_UI;
#endif
}

/**
 * Called when a scene editing window is closed. Returns the scene to a stable "resource-only" state.
 *
 * @param	Scene	the scene to deactivate
 *
 * @return true if the scene was successfully uninitialized
 */
UBOOL UEditorUISceneClient::CloseScene( UUIScene* Scene,UBOOL,UBOOL)
{
	check(Scene);
	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized )
	{
		// if bWindowInitialized is still TRUE, then something called CloseScene directly (rather than calling
		// WxUIEditorBase::Close()), so do that now so that everything is cleaned up properly
		SceneWindow->Close();
		return TRUE;
	}

	// this scene should no longer receive notifications when the viewport is resized
	GCallbackEvent->Unregister(CALLBACK_ViewportResized, this);

	//@hack - UIPrefabScene needs to update the prefab's original size values, so we need to allow it to resolve positions
	const UBOOL bIsPrefabScene = Scene->IsA(UUIPrefabScene::StaticClass());
	if ( bIsPrefabScene )
	{
		SceneWindow->bWindowInitialized = TRUE;
	}
	Scene->Deactivate();
	if ( bIsPrefabScene )
	{
		SceneWindow->bWindowInitialized = FALSE;
	}
	// reset this var so that the next time we open the scene, it will call PreInitialSceneUpdate at the beginning of the first tick
	Scene->bPerformedInitialUpdate = FALSE;

	GenerateSceneThumbnail(Scene);

	// de-initialize all scene properties that aren't needed anymore
	DetachUIPrimitiveScene();

	SceneWindow = NULL;

	SetRenderViewport(NULL);

	return TRUE;
}

/**
 * Called when a scene is about to be deleted in the generic browser.  Closes any open editor windows for this scene
 * and clears all references.
 *
 * @param	Scene	the scene being deleted
 */
void UEditorUISceneClient::NotifyDeleteScene()
{
	// this scene should no longer receive notifications when the viewport is resized
	GCallbackEvent->Unregister(CALLBACK_ViewportResized, this);

	if ( SceneWindow != NULL )
	{
		SceneWindow->Close(TRUE);
	}

	DetachUIPrimitiveScene();
	Scene = NULL;
}

/**
 * Renders this scene's widgets.
 *
 * @param	RI		the interface for drawing to the screen
 */
void UEditorUISceneClient::RenderScenes( FCanvas* Canvas )
{
	check(Canvas);

	// if the user reloads the packages containing this scene while it's open in a UI editor window, it
	// will wipe out the SceneClient reference, so we'll set it each render to ensure the scene has a valid
	// reference to it 
	Scene->SceneClient = this;
	Scene->Tick(-1.f);

	if( UsesUIPrimitiveScene() )
	{
		// init the UI scene for 3d primitive rendering
		if( NeedsInitUIPrimitiveScene() )
		{
			if ( ClientCanvasScene == NULL )
			{
				ClientCanvasScene = new FCanvasScene;
			}
			InitializePrimitives(ClientCanvasScene);
		}

		// update the 3d primitive scene
		UpdateActivePrimitives(ClientCanvasScene);
	}

	// render background ui post process pass
	Render_Scene_PostProcess(Canvas,Scene,UIPostProcess_Background);

	// detect active ui scene with primitives
	UBOOL bUISceneHasPrimitives = UsesUIPrimitiveScene() && GetUIPrimitiveScene() && GetUIPrimitiveScene()->GetNumPrimitives() > 0;
	// turn on depth testing against the primitive ui scene if depth testing is enabled on the scene
	Canvas->SetDepthTestingEnabled(bUISceneHasPrimitives && Scene->bEnableSceneDepthTesting);
	
	Render_Scene(Canvas, Scene, UIPostProcess_None);

	// render foreground ui post process pass
	Render_Scene_PostProcess(Canvas,Scene,UIPostProcess_Foreground);

	// restore depth test to disabled
	Canvas->SetDepthTestingEnabled(FALSE);

	// Check for a feedback loop
	const INT MaxFeedbackLoopItersBeforeWarning = 32;
	if( Scene->UpdateSceneFeedbackLoopCount > MaxFeedbackLoopItersBeforeWarning )
	{
		Canvas->PushAbsoluteTransform( FMatrix::Identity );

		DrawShadowedString(
			Canvas,
			20,
			20,
			*FString::Printf( TEXT( "WARNING: 'Update Scene' feedback loop detected in UI scene!  (%i iters)" ), Scene->UpdateSceneFeedbackLoopCount ),
			GEngine->SmallFont,
			FLinearColor( 1.0f, 0.2f, 0.2f, 1.0f ) );

		Canvas->PopTransform();
	}
}


/**
 * Returns if this UI requires a CanvasScene for rendering 3D primitives
 *
 * @return TRUE if 3D primitives are used
 */
UBOOL UEditorUISceneClient::UsesUIPrimitiveScene() const
{
	return IsUIActive(UUISceneClient::SCENEFILTER_PrimitiveUsersOnly);
}

/**
 * Returns the internal CanvasScene that may be used by this UI
 *
 * @return canvas scene or NULL
 */
FCanvasScene* UEditorUISceneClient::GetUIPrimitiveScene()
{
	return ClientCanvasScene;	
}

/**
 * Detaches and cleans up the ClientCanvasScene.
 */
void UEditorUISceneClient::DetachUIPrimitiveScene()
{
	if( ClientCanvasScene )
	{
		ClientCanvasScene->DetachAllComponents();
		ClientCanvasScene->Release();
		ClientCanvasScene = NULL;
	}
}

/**
 * Determine if the canvas scene for primitive rendering needs to be initialized
 *
 * @return TRUE if InitUIPrimitiveScene should be called
 */
UBOOL UEditorUISceneClient::NeedsInitUIPrimitiveScene()
{
	return !bIsUIPrimitiveSceneInitialized || ClientCanvasScene == NULL;
}

/**
 * Re-initializes all primitives in the specified scene.  Will occur on the next tick.
 *
 * @param	Sender	the scene to re-initialize primitives for.
 */
void UEditorUISceneClient::RequestPrimitiveReinitialization( UUIScene* Sender )
{
	bIsUIPrimitiveSceneInitialized = FALSE;
}

/**
 * Gives all UIScenes a chance to create, attach, and/or initialize any primitives contained in the UIScene.
 *
 * @param	CanvasScene		the scene to use for attaching any 3D primitives
 */
void UEditorUISceneClient::InitializePrimitives( FCanvasScene* CanvasScene )
{
	check(CanvasScene);

	// mark as initialized
	bIsUIPrimitiveSceneInitialized = TRUE;

	// initialize scene
	CanvasScene->DetachAllComponents();

	// attach components to the scene.
	if ( Scene != NULL && Scene->bUsesPrimitives )
	{
		Scene->InitializePrimitives(CanvasScene);
	}
}

/**
 * Updates 3D primitives for all active scenes
 *
 * @param	CanvasScene		the scene to use for attaching any 3D primitives
 */
void UEditorUISceneClient::UpdateActivePrimitives( FCanvasScene* CanvasScene )
{
	check(CanvasScene);
	if ( Scene != NULL )
	{
		Scene->UpdateScenePrimitives(CanvasScene);
	}
}

/**
 * Returns true if there is an unhidden fullscreen UI active
 *
 * @param	Flags	modifies the logic which determines whether the UI is active
 *
 * @return TRUE if the UI is currently active
 */
UBOOL UEditorUISceneClient::IsUIActive( DWORD Flags/*=0*/ ) const
{
	UBOOL bResult = FALSE;
	if ( Scene != NULL && SceneWindow != NULL && SceneWindow->bWindowInitialized )
	{
		if ( (Flags&SCENEFILTER_PrimitiveUsersOnly) != 0 && Scene->bUsesPrimitives )
		{
			bResult = TRUE;
		}

		else if ( (Flags&SCENEFILTER_UsesPostProcessing) != 0 
			&& Scene->bEnableScenePostProcessing 
			&& Scene->IsVisible()
			&& Scene->Opacity > 1.0f / 255.0f )
		{
			const UBOOL bHasPostProcessChain = UIScenePostProcess || Scene->GetPostProcessChain(UIPostProcess_Background) || Scene->GetPostProcessChain(UIPostProcess_Foreground);
			if( bHasPostProcessChain )
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Returns whether the specified scene has been fully initialized.  Different from UUIScene::IsInitialized() in that this
 * method returns true only once all objects related to this scene have been created and initialized (e.g. in the UI editor
 * only returns TRUE once the editor window for this scene has finished creation).
 *
 * @param	Scene	the scene to check.
 */
UBOOL UEditorUISceneClient::IsSceneInitialized( const UUIScene* Scene ) const
{
	//@fixme ronp - this check fails if I open one window while another is still in the process of closing!!
	return SceneWindow != NULL && SceneWindow->bWindowInitialized && Scene != NULL && Scene->IsInitialized();
}

/**
 * Provides the scene client with a way to apply a platform input type other than the actual input type being used.
 * Primarily for simulating platforms in the editor.
 *
 * @param	OwningPlayer		the player to use for determining the real platform input type, if necessary.
 * @param	SimulatedPlatform	receives the value of the platform that should be used.
 *
 * @return	TRUE if the scene client wants to override the current platform input type.
 */
UBOOL UEditorUISceneClient::GetSimulatedPlatformInputType( BYTE& SimulatedPlatform ) const
{
	UBOOL bResult = FALSE;

	if ( SceneWindow != NULL && SceneWindow->bWindowInitialized && SceneWindow->MainToolBar != NULL )
	{
		const FString CurrentPlatform = SceneWindow->MainToolBar->GetCurrentPreviewPlatform();
		if ( CurrentPlatform.Len() > 0 )
		{
			if ( CurrentPlatform == CONSOLESUPPORT_NAME_PC )
			{
				SimulatedPlatform = IPT_PC;
				bResult = TRUE;
			}
			else if ( CurrentPlatform == CONSOLESUPPORT_NAME_360 )
			{
				SimulatedPlatform = IPT_360;
				bResult = TRUE;
			}
			else if ( CurrentPlatform == CONSOLESUPPORT_NAME_PS3 )
			{
				SimulatedPlatform = IPT_PS3;
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Check a key event received by the viewport.
 *
 * @param	Viewport - The viewport which the key event is from.
 * @param	ControllerId - The controller which the key event is from.
 * @param	Key - The name of the key which an event occured for.
 * @param	Event - The type of event which occured.
 * @param	AmountDepressed - For analog keys, the depression percent.
 * @param	bGamepad - input came from gamepad (ie xbox controller)
 *
 * @return	True to consume the key event, false to pass it on.
 */
UBOOL UEditorUISceneClient::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.f*/,UBOOL bGamepad/*=FALSE*/)
{
	UBOOL bResult = FALSE;

	// @todo

	return bResult;
}

/**
 * Check an axis movement received by the viewport.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Key - The name of the axis which moved.
 * @param	Delta - The axis movement delta.
 * @param	DeltaTime - The time since the last axis update.
 *
 * @return	True to consume the axis movement, false to pass it on.
 */
UBOOL UEditorUISceneClient::InputAxis(INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	UBOOL bResult = FALSE;

	// @todo

	return bResult;
}

/**
 * Check a character input received by the viewport.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Character - The character.
 *
 * @return	True to consume the character, false to pass it on.
 */
UBOOL UEditorUISceneClient::InputChar (INT ControllerId, TCHAR Character)
{
	UBOOL bResult = FALSE;

	return bResult;
}

/* ==============================================
	FExec interface
============================================== */
UBOOL UEditorUISceneClient::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	const TCHAR* Str = Cmd;

	UBOOL bResult = FALSE;
	if ( SceneWindow != NULL && SceneWindow->OwnerScene != NULL )
	{
		if( ParseCommand(&Str, TEXT("DUMPSTRINGNODES")) )
		{
			for ( TObjectIterator<UUIString> It; It; ++It )
			{
				if ( It->IsIn(SceneWindow->OwnerScene) )
				{
					UUIString* StringToDump = *It;
					Ar.Logf(TEXT("%s: %i nodes  (%.2f,%.2f)"), *StringToDump->GetFullName(), StringToDump->Nodes.Num(), StringToDump->StringExtent.X, StringToDump->StringExtent.Y);
					for ( INT NodeIndex = 0; NodeIndex < StringToDump->Nodes.Num(); NodeIndex++ )
					{
						FUIStringNode* Node = StringToDump->Nodes(NodeIndex);
						Ar.Logf(TEXT("\t%i) (%.2f,%.2f)  SourceText:%s\t\tRenderText:%s\t\t"), NodeIndex, Node->Extent.X, Node->Extent.Y, *Node->SourceText, Node->IsTextNode() ? *((FUIStringNode_Text*)Node)->RenderedText : TEXT("N/A"));
					}

					bResult = TRUE;
				}
			}
		}
		if ( ParseCommand(&Str,TEXT("ADDSCENEDATAFIELD")) )
		{
			UUIScene* SceneToModify=NULL;
			if ( ParseObject<UUIScene>(Cmd, TEXT("SCENE="), SceneToModify, ANY_PACKAGE) && SceneToModify == SceneWindow->OwnerScene )
			{
				FString FieldName, FieldValue;
				if ( Parse(Cmd, TEXT("NAME="), FieldName) && Parse(Cmd, TEXT("VALUE="), FieldValue) )
				{
					FUIProviderScriptFieldValue ValueStruct(EC_EventParm);
					ValueStruct.PropertyTag = *FieldName;
					ValueStruct.PropertyType = DATATYPE_Property;

					if ( FieldValue.Left(1) == TEXT("(") && FieldValue.Right(1) == TEXT(")") )
					{
						ValueStruct.PropertyType = DATATYPE_RangeProperty;

						// chop off the parenthesis
						FieldValue = FieldValue.Mid(1, FieldValue.Len() - 2);

						FLOAT CurrentValue, MinValue, MaxValue, NudgeValue;
						UBOOL bIntValues;

						// first, parse off the required fields
						if ( Parse(*FieldValue, TEXT("MinValue="), MinValue) )
						{
							ValueStruct.RangeValue.MinValue = MinValue;
						}
						else
						{
							Ar.Logf(TEXT("%s (%s): MinValue not supplied for range data.  Syntax: AddSceneDataField Scene=SceneName Name=DataFieldName Value=(CurrentValue=x,MinValue=x,MaxValue=x[,NudgeValue=x,bIntValues=true/false])"),
								*SceneToModify->GetPathName(),
								*FieldName);
						}
						if ( Parse(*FieldValue, TEXT("MaxValue="), MaxValue) )
						{
							ValueStruct.RangeValue.MaxValue = MaxValue;
						}
						else
						{
							Ar.Logf(TEXT("%s (%s): MaxValue not supplied for range data.  Syntax: AddSceneDataField Scene=SceneName Name=DataFieldName Value=(CurrentValue=x,MinValue=x,MaxValue=x[,NudgeValue=x,bIntValues=true/false])"),
								*SceneToModify->GetPathName(),
								*FieldName);
						}
						if ( Parse(*FieldValue, TEXT("CurrentValue="), CurrentValue) )
						{
							ValueStruct.RangeValue.SetCurrentValue(CurrentValue);
						}
						else
						{
							Ar.Logf(TEXT("%s (%s): CurrentValue not supplied for range data.  Syntax: AddSceneDataField Scene=SceneName Name=DataFieldName Value=(CurrentValue=x,MinValue=x,MaxValue=x[,NudgeValue=x,bIntValues=true/false])"),
								*SceneToModify->GetPathName(),
								*FieldName);
						}

						// these fields are optional
						if ( Parse(*FieldValue, TEXT("NudgeValue="), NudgeValue) )
						{
							ValueStruct.RangeValue.SetNudgeValue(NudgeValue);
						}
						if ( ParseUBOOL(*FieldValue, TEXT("bIntValues="), bIntValues) )
						{
							ValueStruct.RangeValue.bIntRange = bIntValues;
						}
					}
					else
					{
						USurface* ImageValue=NULL;
						if ( (ImageValue=FindObject<USurface>(ANY_PACKAGE, *FieldValue)) != NULL )
						{
							ValueStruct.ImageValue = ImageValue;
						}
						else
						{
							ValueStruct.StringValue = FieldValue;
						}

						UUIDynamicFieldProvider* SceneDataProvider = SceneToModify->SceneData->SceneDataProvider;
						if ( SceneDataProvider != NULL )
						{
							if ( SceneDataProvider->AddField(ValueStruct.PropertyTag, ValueStruct.PropertyType) )
							{
								if ( SceneDataProvider->SetFieldValue(FieldName, ValueStruct) )
								{
									Ar.Logf(TEXT("%s: successfully added new data field: %s (%s)"), *SceneToModify->GetPathName(), *FieldName, *FieldValue);
								}
								else
								{
									Ar.Logf(TEXT("%s: failed to set the value for data field added to scene data store: %s (%s)"), *SceneToModify->GetPathName(), *FieldName, *FieldValue);
								}
							}
							else
							{
								Ar.Logf(TEXT("%s: failed to add new data field to scene data store: %s (%s)"), *SceneToModify->GetPathName(), *FieldName, *FieldValue);
							}
						}
						else
						{
							Ar.Logf(TEXT("%s's data store doesn't have a valid data provider"), *SceneToModify->GetPathName());
						}
					}
				}
				else
				{
					Ar.Logf(TEXT("Insufficient parameters.  Syntax: AddSceneDataField Scene=SceneName Name=DataFieldName Value=DataFieldValue"));
				}

				bResult = TRUE;
			}
		}
		if ( ParseCommand(&Str,TEXT("REMOVESCENEDATAFIELD")) )
		{
			UUIScene* SceneToModify=NULL;
			if ( ParseObject<UUIScene>(Cmd, TEXT("SCENE="), SceneToModify, ANY_PACKAGE) && SceneToModify == SceneWindow->OwnerScene )
			{
				FName FieldName;
				if ( Parse(Cmd, TEXT("NAME="), FieldName) )
				{
					UUIDynamicFieldProvider* SceneDataProvider = SceneToModify->SceneData->SceneDataProvider;
					if ( SceneDataProvider != NULL )
					{
						if ( SceneDataProvider->RemoveField(FieldName) )
						{
							Ar.Logf(TEXT("%s: successfully removed data field from scene data store: %s"), *SceneToModify->GetPathName(), *FieldName.ToString());
						}
						else
						{
							Ar.Logf(TEXT("%s: failed to remove data field from scene data store: %s"), *SceneToModify->GetPathName(), *FieldName.ToString());
						}
					}
					else
					{
						Ar.Logf(TEXT("%s's data store doesn't have a valid data provider"), *SceneToModify->GetPathName());
					}
				}
				else
				{
					Ar.Logf(TEXT("Insufficient parameters.  Syntax: RemoveSceneDataField Scene=SceneName Name=DataFieldName"));
				}

				bResult = TRUE;
			}
		}
		else if ( ParseCommand(&Cmd,TEXT("DEBUGUIPREFAB")) )
		{
			FString ObjectName;
			UUIPrefabInstance* PrefInst = NULL;
			if ( ParseToken(Cmd,ObjectName,1) )
			{
				PrefInst = FindObject<UUIPrefabInstance>(ANY_PACKAGE,*ObjectName);
			}

			if ( PrefInst != NULL )
			{
				Ar.Logf(TEXT("Property values for %s"), *PrefInst->GetFullName());
				Ar.Logf(TEXT("\tArchetypeToInstanceMap (%i elements)"), PrefInst->ArchetypeToInstanceMap.Num());
				for ( TMap<UObject*, UObject*>::TConstIterator It(PrefInst->ArchetypeToInstanceMap); It; ++It )
				{
					UObject* Arc = It.Key();
					UObject* Inst = It.Value();
					Ar.Logf(TEXT("\t\tArc {%s}\t Inst {%s}"), *Arc->GetFullName(), *Inst->GetFullName());
				}

				Ar.Logf(TEXT("\tPI_CompleteObjects (%i elements)"), PrefInst->PI_CompleteObjects.Num());
				for ( INT ObjIndex = 0; ObjIndex < PrefInst->PI_CompleteObjects.Num(); ObjIndex++ )
				{
					UObject* CurrentObject = PrefInst->PI_CompleteObjects(ObjIndex);
					Ar.Logf(TEXT("\t\t%i) %s"), ObjIndex, *CurrentObject->GetFullName());
				}
			}
			else
			{
				Ar.Logf(TEXT("No UIPrefabInstances found using the name '%s'"), *ObjectName);
			}

			bResult = TRUE;
		}
		else if ( ParseCommand(&Cmd, TEXT("CHECKNULLCHILDREN")) )
		{
			for ( TObjectIterator<UUIScreenObject> It; It; ++It )
			{
				UUIScreenObject* Widget = *It;
				if ( Widget->Children.ContainsItem(NULL) )
				{
					Ar.Logf(TEXT("%s has NULL entries in the Children array"), *Widget->GetFullName());
				}
			}

			bResult = TRUE;
		}

		bResult = bResult || Super::Exec(Cmd, Ar);
	}

	return bResult;
}

/**
 * Called when the viewport has been resized.
 */
void UEditorUISceneClient::Send(ECallbackEventType InType, FViewport* InViewport, UINT InMessage)
{
	if ( InType == CALLBACK_ViewportResized && Scene != NULL )
	{
		if ( RenderViewport == InViewport && RenderViewport != NULL )
		{
			UpdateCanvasToScreen();

			FVector2D NewViewportSize;
			if ( !GetViewportSize(Scene, NewViewportSize) )
			{
				NewViewportSize.X = InViewport->GetSizeX();
				NewViewportSize.Y = InViewport->GetSizeY();
			}
			const FVector2D OriginalSceneViewportSize = Scene->CurrentViewportSize;

			Scene->CurrentViewportSize = NewViewportSize;
			Scene->NotifyResolutionChanged(OriginalSceneViewportSize, NewViewportSize);
		}
	}
}

/* ==========================================================================================================
	UUIPreviewString
========================================================================================================== */
/**
 * Changes the current menu state for this UIPreviewString.
 */
void UUIPreviewString::SetCurrentMenuState( UUIState* NewMenuState )
{
	CurrentMenuState = NewMenuState;
}

/**
 * Retrieves the UIState that should be used for applying style data.
 */
UUIState* UUIPreviewString::GetCurrentMenuState() const
{
	return CurrentMenuState;
}
	
/**
 * Gets the size of the viewport.
 *
 * @param	out_ViewportSize	receives the viewport size.
 *
 * @return	TRUE if the viewport size was retrieved successfully.
 */
UBOOL UUIPreviewString::GetViewportSize( FVector2D& out_ViewportSize ) const
{
	out_ViewportSize = PreviewViewportSize;
	return TRUE;
}

/* ==========================================================================================================
	FEditorUIObjectViewportClient
========================================================================================================== */

/** Constructor */
FEditorUIObjectViewportClient::FEditorUIObjectViewportClient( WxUIEditorBase* InUIEditor )
: FEditorObjectViewportClient(InUIEditor), UIEditor(InUIEditor)
{
	// Scrolling not allowed in the UI editor
	bAllowScroll = FALSE;
	ViewportOutline = NULL;
}

/** Destructor */
FEditorUIObjectViewportClient::~FEditorUIObjectViewportClient()
{
	RemoveRenderModifier(ViewportOutline);

	delete ViewportOutline;
	ViewportOutline = NULL;

	delete MouseTool;
	MouseTool = NULL;

	while ( WidgetRenderModifiers.Num() > 0 )
	{
		FRenderModifier_CustomWidget* RenderModifier = WidgetRenderModifiers.Pop();
		RemoveRenderModifier(RenderModifier);
		delete RenderModifier;
	}
}

/** Initialize this client's workers */
void FEditorUIObjectViewportClient::InitializeClient()
{
	ViewportOutline = new FRenderModifier_ViewportOutline(UIEditor);
	AddRenderModifier(ViewportOutline, 0);

	FEditorObjectViewportClient::InitializeClient();
}


/**
 * @return	TRUE if the UIScene associated with this viewport is open and initialized
 */
FORCEINLINE UBOOL FEditorUIObjectViewportClient::HasValidUIScene() const
{
	return UIEditor != NULL
		&& UIEditor->OwnerScene != NULL
		&& UIEditor->OwnerScene->SceneClient != NULL
		&& UIEditor->OwnerScene->SceneClient->IsSceneInitialized(UIEditor->OwnerScene);
}

/**
 * Determine whether the required buttons are pressed to begin a box selection.
 */
UBOOL FEditorUIObjectViewportClient::ShouldBeginBoxSelect() const
{
	// If we have the mouse tool selected then we should be box selecting on drag.
	const UBOOL bProperModifiersPressed = (Input->IsAltPressed() && Input->IsCtrlPressed());

	if ( bProperModifiersPressed && Selector->ActiveHandle == HANDLE_None )
	{
		return TRUE;
	}

	return FALSE;
}

/**
* Called by FEditorObjectViewportClient so we can process keys
*/
UBOOL FEditorUIObjectViewportClient::InputKeyUser (FName Key, EInputEvent Event)
{
	if (Event == IE_Pressed)
	{
		FLOAT move = Input->IsShiftPressed() ? 10 : 1;

		if (Key == KEY_A)
		{
			// @todo
		}
		else if (Key == KEY_Up)
		{
			UIEditor->MoveSelectedObjects (HANDLE_Outline, FVector (0, -move, 0));
		}
		else if (Key == KEY_Down)
		{
			UIEditor->MoveSelectedObjects (HANDLE_Outline, FVector (0, move, 0));
		}
		else if (Key == KEY_Left)
		{
			UIEditor->MoveSelectedObjects (HANDLE_Outline, FVector (-move, 0, 0));
		}
		else if (Key == KEY_Right)
		{
			UIEditor->MoveSelectedObjects (HANDLE_Outline, FVector (move, 0, 0));
		}
		else if(Key == KEY_Delete)
		{
			UIEditor->DeleteSelectedWidgets();
		}
		else
		{
			FName ToolModeKeyMappings[] = { KEY_One, KEY_Two, KEY_Three, KEY_Four, KEY_Five, KEY_Six, KEY_Seven, KEY_Eight, KEY_Nine, KEY_Zero };

			UBOOL bToolModeKey = FALSE;
			for ( INT i = 0; i < ARRAY_COUNT(ToolModeKeyMappings); i++ )
			{
				if ( Key == ToolModeKeyMappings[i] )
				{
					UIEditor->SetToolMode( ID_UITOOL_MODES_START + i );
					bToolModeKey = TRUE;
					break;
				}
			}

			return bToolModeKey;
		}

		return TRUE;		// Processed
	}

	return FALSE;
}

/**
* Check a key event received by the viewport.
* If the viewport client uses the event, it should return true to consume it.
* @param	Viewport - The viewport which the key event is from.
* @param	ControllerId - The controller which the key event is from.
* @param	Key - The name of the key which an event occured for.
* @param	Event - The type of event which occured.
* @param	AmountDepressed - For analog keys, the depression percent.
* @return	True to consume the key event, false to pass it on.
*/
UBOOL FEditorUIObjectViewportClient::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL /*Gamepad*/)
{
	UBOOL bPassInputToSuperclass = TRUE;

	if(Event == IE_Pressed)
	{
		if ( Key == KEY_Delete )
		{
			UIEditor->DeleteSelectedWidgets();
			bPassInputToSuperclass = FALSE;
		}
		else if ( Key == KEY_Home )
		{
			UIEditor->CenterOnSelectedWidgets();
			bPassInputToSuperclass = FALSE;
		}
		else if ( (Key == KEY_MouseScrollDown || Key == KEY_MouseScrollUp) && Input->IsCtrlPressed() )
		{
			// get the list of widgets selected in the tree control
			TArray<UUIObject*> SelectedWidgets;
			UIEditor->GetSelectedWidgets(SelectedWidgets);

			UUIScreenObject* DefaultParent = UIEditor->OwnerScene->GetEditorDefaultParentWidget();
			if ( SelectedWidgets.Num() > 1 )
			{
				if ( Key == KEY_MouseScrollDown )
				{
					UUIScreenObject* CommonParent = WxUIEditorBase::FindCommonParent(DefaultParent, SelectedWidgets);
					
					UIEditor->Selection->BeginBatchSelectOperation();
					UIEditor->EmptySelection();
					UIEditor->AddToSelection(CommonParent);
					UIEditor->Selection->EndBatchSelectOperation();
				}
				else
				{
					// nothing?
				}
			}
			else
			{
				if ( Key == KEY_MouseScrollUp )
				{
					UUIObject* NextChildToFocus = NULL;
					if ( SelectedWidgets.Num() == 0 )
					{
						if ( DefaultParent->Children.Num() > 0 )
						{
							UIEditor->AddToSelection(DefaultParent->Children(0));
						}
					}
					else if ( SelectedWidgets(0)->Children.Num() > 0 )
					{
						//@todo ronp - we could make this logic much better; like, traverse all the way down to the bottom of the hierarchy
						// then start back at the sibling of the originally selected widget.
						UUIObject* NextSelectedWidget = SelectedWidgets(0)->Children(0);

						UIEditor->Selection->BeginBatchSelectOperation();
						ObjectEditor->EmptySelection();
						ObjectEditor->AddToSelection(NextSelectedWidget);
						ObjectEditor->Selection->EndBatchSelectOperation();
					}
				}
				else if ( SelectedWidgets.Num() == 1 )
				{
					UUIScreenObject* Parent = SelectedWidgets(0)->GetParent();
					if ( Parent != NULL )
					{
						UIEditor->Selection->BeginBatchSelectOperation();
						UIEditor->EmptySelection();
						UIEditor->AddToSelection(Parent);
						UIEditor->Selection->EndBatchSelectOperation();
					}
				}
			}

			bPassInputToSuperclass = FALSE;
		}
	}

	if ( bPassInputToSuperclass )
	{
		bPassInputToSuperclass = !UIEditor->WidgetTool || !UIEditor->WidgetTool->InputKey(Viewport, ControllerId, Key, Event, AmountDepressed);
	}

	if ( bPassInputToSuperclass )
	{
		return FEditorObjectViewportClient::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed);
	}

	return TRUE;
}

/**
* Called when the user clicks anywhere in the viewport.
*
* @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
* @param	Click		contains data about this mouse click
*/
void FEditorUIObjectViewportClient::ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click)
{
	UBOOL bHandledClick = FALSE;

	if(UIEditor->WidgetTool && UIEditor->WidgetTool->ProcessClick(HitProxy, Click))
	{
		bHandledClick = TRUE;
	}


	if( bHandledClick == FALSE)
	{
		FEditorObjectViewportClient::ProcessClick (HitProxy, Click);
	}
}

/**
 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
 */
UBOOL FEditorUIObjectViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	if(UIEditor->WidgetTool)
	{
		UIEditor->WidgetTool->InputAxis(Viewport, ControllerId, Key, Delta, DeltaTime);
	}
	

	// Accumulate and snap the mouse movement since the last mouse button click.
	MouseTracker->AddDelta(this, Key, Delta, FALSE);
	
	if(UIEditor->EditorOptions->bSnapToGrid)
	{
		MouseTracker->Snap(UIEditor->GetGridSize(), UIEditor->GetGridOrigin());
	}

	// If we are using a drag tool, paint the viewport so we can see it update.
	if ( MouseTracker->DragTool != NULL )
	{
		RefreshViewport();
	}

	return TRUE;
}

/**
* Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
*/
void FEditorUIObjectViewportClient::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	// If a MouseTool is active then, let it handle the mouse move event.
	if(MouseTool != NULL)
	{
		MouseTool->MouseMove(Viewport, X, Y);

		if(UIEditor->WidgetTool)
		{
			UIEditor->WidgetTool->MouseMove(Viewport, X, Y);
		}

		// Update status bar
		FString DeltaStatus;
		MouseTool->GetDeltaStatusString(DeltaStatus);
		UIEditor->StatusBar->UpdateMouseDelta(*DeltaStatus);
		UIEditor->StatusBar->UpdateMousePosition(this, X, Y);

		// Update viewport.
		Viewport->Invalidate();
	}
	else
	{
		UIEditor->StatusBar->UpdateMousePosition(this, X, Y);
		if(UIEditor->WidgetTool)
		{
			UIEditor->WidgetTool->MouseMove(Viewport, X, Y);
		}
		
		// Update Status Bar, Since there is no mouse tool active, we set the status to nothing.
		UIEditor->StatusBar->UpdateMouseDelta(TEXT(""));

		FEditorObjectViewportClient::MouseMove(Viewport, X, Y);
	}
}

/**
 * Create the appropriate drag tool.  Called when the user has dragged the mouse at least the minimum number of pixels.
 *
 * @return	a pointer to the appropriate drag tool given the currently pressed buttons, or NULL if no drag tool should be active
 */
FObjectDragTool* FEditorUIObjectViewportClient::CreateCustomDragTool()
{
	return NULL;
}

/**
 * Creates or cleans up custom widget render modifiers based on the currently selected widgets.
 */
void FEditorUIObjectViewportClient::UpdateWidgetRenderModifiers()
{
	for ( INT WidgetIdx = WidgetRenderModifiers.Num() - 1; WidgetIdx >= 0; WidgetIdx-- )
	{
		FRenderModifier_CustomWidget* RenderModifier = WidgetRenderModifiers(WidgetIdx);
		if ( RenderModifier->SelectedWidget == NULL
		||	!UIEditor->Selection->IsSelected(RenderModifier->SelectedWidget)
		||	RenderModifier->SelectedWidget->IsPendingKill() )
		{
			RemoveRenderModifier(RenderModifier);
			WidgetRenderModifiers.Remove(WidgetIdx);
			delete RenderModifier;
		}
	}

	TArray<UUIObject*> SelectedWidgets;
	UIEditor->GetSelectedWidgets(SelectedWidgets,FALSE);

	for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
	{
		UUIObject* SelectedWidget = SelectedWidgets(SelectionIndex);
		if ( FindCustomWidgetRenderModifierIndex(SelectedWidget) == INDEX_NONE )
		{
			//@fixme ronp - ok, this is really gay; better would be data driven or something
			if ( SelectedWidget->IsA(UUIList::StaticClass()) )
			{
				FRenderModifier_UIListCellBoundary* ResizeCellRenderModifier = new FRenderModifier_UIListCellBoundary(UIEditor, static_cast<UUIList*>(SelectedWidget));
				AddRenderModifier(ResizeCellRenderModifier);
				WidgetRenderModifiers.AddItem(ResizeCellRenderModifier);
			}
		}
	}
}

/**
 * Returns the index of the custom widget render modifier associated with the specified widget, or INDEX_NONE if there are none.
 */
INT FEditorUIObjectViewportClient::FindCustomWidgetRenderModifierIndex( UUIObject* SearchWidget )
{
	INT Result = INDEX_NONE;

	for ( INT ModifierIndex = 0; ModifierIndex < WidgetRenderModifiers.Num(); ModifierIndex++ )
	{
		if ( WidgetRenderModifiers(ModifierIndex)->SelectedWidget == SearchWidget )
		{
			Result = ModifierIndex;
			break;
		}
	}

	return Result;
}

/**
 *	Creates a mouse tool depending on the current state of the editor and what key modifiers are held down.
 *  @param HitProxy	Pointer to the hit proxy currently under the mouse cursor.
 *  @return Returns a pointer to the newly created FMouseTool.
 */
FMouseTool* FEditorUIObjectViewportClient::CreateMouseTool(const HHitProxy* HitProxy)
{
	const INT CurrentMode = UIEditor->EditorOptions->ToolMode;
	FMouseTool* MouseTool = NULL;

	// allow the currently active widget tool to have the first opportunity to provide a MouseTool
	if( UIEditor->WidgetTool != NULL )
	{
		MouseTool = UIEditor->WidgetTool->CreateMouseTool();
	}

	if( MouseTool == NULL && ObjectEditor->HaveObjectsSelected() )
	{
		UBOOL bCreateMouseTool = FALSE;
		const UBOOL bObjectOrBackgroundHighlighted = ( HitProxy == NULL || HitProxy->IsA(HUIHitProxy::StaticGetType()));
		const UBOOL bCtrlOnly = Input->IsCtrlPressed() && !Input->IsAltPressed() && !Input->IsShiftPressed();
		const UBOOL bShiftOnly = !Input->IsCtrlPressed() && !Input->IsAltPressed() && Input->IsShiftPressed();
		const UBOOL bMoveObjectMode = bCtrlOnly && bObjectOrBackgroundHighlighted;
		const UBOOL bRotateObjectMode = bShiftOnly && bObjectOrBackgroundHighlighted;

		// If the user has their mouse over a handle, OR, if they have only control held down, then create a drag selection mouse tool.
		// If they have control held down, we force the selected handle to the outline so that they can move the selected objects without
		// having to actually click and drag the selection outline.
		if(Selector->ActiveHandle != HANDLE_None)
		{
			bCreateMouseTool = TRUE;
		}
		else if( bMoveObjectMode )
		{
			Selector->ActiveHandle = HANDLE_Outline;
			bCreateMouseTool = TRUE;
		}
		else if( bRotateObjectMode )
		{
			Selector->ActiveHandle = HANDLE_Rotation;
			bCreateMouseTool = TRUE;
		}

		if(bCreateMouseTool == TRUE)
		{
			MouseTool = new FMouseTool_DragSelection(this);

			MouseTool->SetGridOrigin(UIEditor->GetGridOrigin());
			MouseTool->SetGridSize(UIEditor->GetGridSize());
			MouseTool->SetUseSnapping(UIEditor->GetUseSnapping());
		}
		else if ( HitProxy && HitProxy->IsA(HUICellBoundaryHitProxy::StaticGetType()) )
		{
			MouseTool = new FMouseTool_ResizeListCell(this, UIEditor, ((HUICellBoundaryHitProxy*)HitProxy)->SelectedList, ((HUICellBoundaryHitProxy*)HitProxy)->ResizeCell);
		}
	}

	if(MouseTool == NULL)
	{
		MouseTool = FEditorObjectViewportClient::CreateMouseTool(HitProxy);
	}

	return MouseTool;
}

/**
 * Returns the scene to use for rendering primitives in this viewport.
 */
FSceneInterface* FEditorUIObjectViewportClient::GetScene()
{
	FSceneInterface* Result = NULL;
	if ( UIEditor != NULL && UIEditor->OwnerScene != NULL )
	{
		if ( UIEditor->OwnerScene->SceneClient != NULL )
		{
			Result = CastChecked<UEditorUISceneClient>(UIEditor->OwnerScene->SceneClient)->GetUIPrimitiveScene();
		}
	}

	if ( Result == NULL )
	{
		Result = FEditorObjectViewportClient::GetScene();
	}

	return Result;
}


/**
* Renders 3D primitives into the viewport.
*
* @param Viewport - current editor viewport to render to
* @param Canvas - current canvas with render target to use
* @return TRUE if anything was rendered
* 
*/
UBOOL FEditorUIObjectViewportClient::DrawPrimitives( FViewport* Viewport, FCanvas* Canvas )
{
	UBOOL bRendered = FALSE;
	FSceneInterface* RenderScene = GetScene();
	if ( RenderScene != GWorld->Scene )
	{
		FCanvasScene* CanvasScene = static_cast<FCanvasScene*>(RenderScene);
		if ( CanvasScene->GetNumPrimitives() > 0 )
		{
			FLOAT TimeSeconds = appSeconds();

			QWORD SceneShowFlags = ShowFlags;
			// toggle wireframe mode
			if( UIEditor && 
				UIEditor->EditorOptions && 
				UIEditor->EditorOptions->bViewShowWireframe )
			{
				SceneShowFlags &= ~SHOW_ViewMode_Mask;
				SceneShowFlags |= SHOW_ViewMode_Wireframe;
			}

			// Create a FSceneViewFamilyContext for the canvas scene
			FSceneViewFamilyContext CanvasSceneViewFamily(
				Canvas->GetRenderTarget(),
				CanvasScene,
				SceneShowFlags,
				TimeSeconds,
				0.0f,
				TimeSeconds, 
				GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, 
				FALSE,
				FALSE,
				FALSE,	
				Canvas->GetHitProxyConsumer()!=NULL,	// assume that the scene color gets cleared by the grid rendering
				Canvas->GetHitProxyConsumer()!=NULL		// disable the final copy to the back buffer since it will occur during FEditorUIObjectViewportClient::DrawPostProcess
				);

			// scale/offset applied directly to the primitive transforms, see UUIRoot::GetPrimitiveTransform
			const FVector2D ViewSizeScale( 1.0f, 1.0f );
			const FVector2D OriginPercent( 0.0f, 0.0f );

			// Generate the view for the canvas scene			
			CanvasScene->CalcSceneView(
				&CanvasSceneViewFamily,
				OriginPercent,
				ViewSizeScale,
				Viewport,
				NULL,
				NULL
				);

			// Render the canvas scene
			BeginRenderingViewFamily(Canvas,&CanvasSceneViewFamily);

			bRendered=TRUE;
		}
	}
	return bRendered;
}

/**
* Renders the post process pass for the UI editor and copies the results to the Viewport render target
*/
void FEditorUIObjectViewportClient::DrawPostProcess( FViewport* Viewport, FCanvas* Canvas, UPostProcessChain* UIPostProcessChain )
{
	FLOAT TimeSeconds = appSeconds();

	QWORD PPViewShowFlags = ShowFlags;
	if( UIPostProcessChain )
	{ 
		PPViewShowFlags |= SHOW_PostProcess;
	}

	// Create a FSceneViewFamilyContext for rendering the post process
	FSceneViewFamilyContext PostProcessViewFamily(
		Viewport,
		NULL,
		PPViewShowFlags,
		TimeSeconds,
		0.0f,
		TimeSeconds, 
		NULL, 
		IsRealtime(),
		TRUE,
		FALSE,			
		FALSE,	// maintain scene color from player scene rendering
		TRUE	// enable the final copy to the viewport RT
		);

	// Compute the view's screen rectangle.
	INT X = 0;
	INT Y = 0;
	UINT SizeX = Viewport->GetSizeX();
	UINT SizeY = Viewport->GetSizeY();

	const FLOAT fFOV = 90.0f;
	// Take screen percentage option into account if percentage != 100.
	GSystemSettings.ScaleScreenCoords(X,Y,SizeX,SizeY);

	// add the new view to the scene
	FSceneView* View = new FSceneView(
		&PostProcessViewFamily,
		NULL,
		-1,
		NULL,
		NULL,
		NULL,
		UIPostProcessChain,
		NULL,
		NULL,
		NULL,
		X,
		Y,
		SizeX,
		SizeY,
		FCanvas::CalcViewMatrix(SizeX,SizeY,fFOV),
		FCanvas::CalcProjectionMatrix(SizeX,SizeY,fFOV,NEAR_CLIPPING_PLANE),
		FLinearColor::Black,
		FLinearColor(0,0,0,0),
		FLinearColor::White,
		TSet<UPrimitiveComponent*>()
		);
	PostProcessViewFamily.Views.AddItem(View);
	// Render the post process pass
	BeginRenderingViewFamily(Canvas,&PostProcessViewFamily);
}

/**
 *	Rendering loop call for this viewport.
 */
void FEditorUIObjectViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	UBOOL bIsValidUIScene = HasValidUIScene();

	if ( !bIsValidUIScene )
	{
		return;
	}

	// create a proxy of the scene color buffer to render to
	static FSceneRenderTargetProxy SceneColorTarget;
	SceneColorTarget.SetSizes(Viewport->GetSizeX(),Viewport->GetSizeY());
	if( !Canvas->GetHitProxyConsumer() )
	{
		// set the scene color render target
		Canvas->SetRenderTarget(&SceneColorTarget);
	}	

	// Render the background while we're still using a 2D base matrix
	{
		Canvas->PushAbsoluteTransform(FScaleMatrix(Zoom2D) * FTranslationMatrix(FVector(Origin2D.X,Origin2D.Y,0)));
		// in order to ensure that the background grid always renders behind all widgets, make sure that it's DepthSortKey is the max value
		Canvas->PushDepthSortKey(MAXINT - 1);
		UIEditor->DrawBackground(Viewport,Canvas);
		Canvas->PopDepthSortKey();
		Canvas->PopTransform();
	}

	// Set 3D Projection
	Canvas->SetBaseTransform(FCanvas::CalcBaseTransform3D(Viewport->GetSizeX(),
		Viewport->GetSizeY(),90,NEAR_CLIPPING_PLANE));

	// Render the scene and all tools / overlays
	Canvas->PushAbsoluteTransform(FScaleMatrix(Zoom2D) * FTranslationMatrix(FVector(Origin2D.X,Origin2D.Y,0)));
	{
		// allow the scene to render any 3D objects
		DrawPrimitives(Viewport, Canvas);
		
		// draw the 2D objects in this viewport
		UIEditor->DrawObjects(Viewport,Canvas);

		// we just rendered the UIScene.  It's possible (though very bad practice) that the scene we just rendered closed
		// itself during the render call, so we'll need to protect ourselves against that
		bIsValidUIScene = HasValidUIScene();
		if ( bIsValidUIScene )
		{
			if( !Canvas->GetHitProxyConsumer() )
			{
				// Restore to using the viewport RT
				Canvas->SetRenderTarget(Viewport);
				// post process and copy to the viewport render target		
				UPostProcessChain* UIPostProcessChain = NULL;
				if( UIEditor->OwnerScene &&
					UIEditor->OwnerScene->SceneClient &&
					UIEditor->OwnerScene->SceneClient->UsesPostProcess() )
				{
					UIPostProcessChain = UIEditor->OwnerScene->SceneClient->UIScenePostProcess;
				}
				// always need this iot copy back to viewport RT even if no post processing is actually used
				DrawPostProcess(Viewport, Canvas, UIPostProcessChain);
			}

			// draw the viewport modifiers
			for ( INT i = 0; i < RenderModifiers.Num(); i++ )
			{
				FObjectViewportRenderModifier* Modifier = RenderModifiers(i);
				Modifier->Render(this,Canvas);
			}

			if ( MouseTracker->DragTool )
			{
				MouseTracker->DragTool->Render(Canvas);
			}

			// Let the selected tool render if it wants to, make sure to draw this last so it has a chance to draw on top
			// of everything else.
			if ( MouseTool )
			{
				MouseTool->Render(Canvas);
			}
		}
	}

	// If Enabled, render the title safe region outline.
	if ( bIsValidUIScene && UIEditor->EditorOptions->bRenderTitleSafeRegionOutline )
	{
		FVector2D StartPos, EndPos;
		GetClientRenderRegion(StartPos,EndPos,FALSE);

		const FLOAT TitlePercentage=UIEditor->SceneManager->TitleRegions.RecommendedPercentage;
		const FLOAT MaxPercentage=UIEditor->SceneManager->TitleRegions.MaxPercentage;
		const FVector2D Gap = EndPos-StartPos;
		const FColor MaxColor = FColor(255,0,0);
		const FColor TitleColor = FColor(255,255,0);

		// Draw Max Region
		{
			const FVector2D Offset = Gap * (MaxPercentage + (1.0f - MaxPercentage) * 0.5f);	
			DrawBox2D(
				Canvas,
				FVector2D(StartPos.X + Offset.X,StartPos.Y + Offset.Y),
				FVector2D(EndPos.X - Offset.X,EndPos.Y - Offset.Y),
				MaxColor
				);
		}

		// Draw Title Region
		{
			const FVector2D Offset = Gap * (TitlePercentage + (1.0f - TitlePercentage) * 0.5f);
			DrawBox2D(
				Canvas,
				FVector2D(StartPos.X + Offset.X,StartPos.Y + Offset.Y),
				FVector2D(EndPos.X - Offset.X,EndPos.Y - Offset.Y),
				TitleColor
				);
		}
	}

	Canvas->PopTransform();

	// Restore 2D Projection
	Canvas->SetBaseTransform(FCanvas::CalcBaseTransform2D(Viewport->GetSizeX(),
		Viewport->GetSizeY()));
}

/**
 * @return Returns a mouse cursor depending on where the mouse is in the viewport and if a mouse tool is active.
 */
EMouseCursor FEditorUIObjectViewportClient::GetCursor(FViewport* Viewport,INT X,INT Y)
{
	if(MouseTool != NULL)
	{
		return MouseTool->GetCursor();
	}
	else
	{
		return FEditorObjectViewportClient::GetCursor(Viewport, X, Y);
	}
}

/** Creates the object selection tool */
void FEditorUIObjectViewportClient::CreateSelectionTool()
{
	if ( Selector == NULL )
	{
		Selector = new FUISelectionTool;
	}
}

/**
 * Retrieve the vectors marking the region within the viewport available for the object editor to render objects. 
 *
 * @param	StartPos					receives the value of the origin of the rendering region
 * @param	EndPos						receives the value for the end point of the rendering region
 * @param	bIncludeVirtualAdjustments	specify FALSE to ignore any gutter or offsets which may be in use by the viewport
 */
void FEditorUIObjectViewportClient::GetClientRenderRegion( FVector2D& StartPos, FVector2D& EndPos, UBOOL bIncludeVirtualAdjustments/*=TRUE*/ )
{
	FLOAT ViewportSizeX, ViewportSizeY;
	UIEditor->EditorOptions->GetVirtualViewportSize(Viewport, ViewportSizeX, ViewportSizeY, !bIncludeVirtualAdjustments);

	StartPos.X = UIEditor->EditorOptions->ViewportGutterSize;
	StartPos.Y = UIEditor->EditorOptions->ViewportGutterSize;
	EndPos.X = StartPos.X + ViewportSizeX;
	EndPos.Y = StartPos.Y + ViewportSizeY;
}

/* ==========================================================================================================
	FRenderModifier_TargetTooltip
========================================================================================================== */
FRenderModifier_TargetTooltip::FRenderModifier_TargetTooltip(class WxUIEditorBase* InEditor)
: FUIEditor_RenderModifier(InEditor)
{
	
}

/**
 * Renders a tooltip box with each of the strings in our TooltipStrings list displayed in order from top to bottom.  The box
 * is drawn at a position offset from the mouse cursor and is checked against screen bounds to make sure that it is visible.
 */
void FRenderModifier_TargetTooltip::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	const INT NumStrings = TooltipStrings.Num();

	if(NumStrings > 0)
	{
		// Loop through all strings and calculate a size for the box.
		UFont* Font = GEngine->SmallFont;
		
		INT BoxHeight = 0;
		INT BoxWidth = 0;
		INT LineHeight = 0;

		for(INT StringIdx = 0; StringIdx < NumStrings; StringIdx++)
		{
			INT StrWidth;
			INT StrHeight;

			StringSize(Font, StrWidth, StrHeight, *TooltipStrings(StringIdx));

			if(StrWidth > BoxWidth)
			{
				BoxWidth = StrWidth;
			}

			if(StrHeight > LineHeight)
			{
				LineHeight = StrHeight;
			}
		}

		BoxHeight = LineHeight * NumStrings;
	
		const INT Padding = 2;

		BoxWidth += Padding * 2;
		BoxHeight += Padding * 2;

		Canvas->PushDepthSortKey(-MAXINT + 1);
		Canvas->PushAbsoluteTransform(FMatrix::Identity);
		{
			const INT ViewportX = ObjectVC->Viewport->GetSizeX();
			const INT ViewportY = ObjectVC->Viewport->GetSizeY();
			const INT MouseX = ObjectVC->Viewport->GetMouseX();
			const INT MouseY = ObjectVC->Viewport->GetMouseY();

			FIntPoint Offset(20, 20);

			FIntPoint BoxPosition(MouseX, MouseY);

			BoxPosition += Offset;

			if(BoxPosition.X + BoxWidth > ViewportX)
			{
				BoxPosition.X -= Offset.X * 2 + BoxWidth;
			}

			if(BoxPosition.Y + BoxHeight > ViewportY)
			{
				BoxPosition.Y -= Offset.Y * 2 + BoxHeight;
			}
			
			// Draw the box
			const INT Border = 2;
			DrawTile(Canvas,BoxPosition.X - Border, BoxPosition.Y - Border, BoxWidth + Border*2, BoxHeight + Border*2, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor(0.0f,0.0f,0.0f,0.5f));
			DrawTile(Canvas,BoxPosition.X, BoxPosition.Y, BoxWidth, BoxHeight, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor(0.0f,0.0f,0.25f,0.25f));

			// Draw all of the strings
			FIntPoint StringPos(Padding, Padding);
			StringPos += BoxPosition;

			for(INT StringIdx = 0; StringIdx < NumStrings; StringIdx++)
			{
				DrawString(Canvas,StringPos.X, StringPos.Y, *TooltipStrings(StringIdx), Font, FLinearColor(1.0f,1.0f,1.0f,1.0f));

				StringPos.Y += LineHeight;
			}
		}
		Canvas->PopTransform();
		Canvas->PopDepthSortKey();
	}
}


/* ==========================================================================================================
	FRenderModifier_ContainerOutline
========================================================================================================== */
/**
 * Constructor
 *
 * @param	InEditor	the editor window that holds the container to render the outline for
 */
FRenderModifier_ContainerOutline::FRenderModifier_ContainerOutline( WxUIEditorBase* InEditor )
: FUIEditor_RenderModifier(InEditor), OutlineColor(0,255,0,255)	// green
{
}

/**
 * Renders the outline for the top-level object of a UI editor window.
 */
void FRenderModifier_ContainerOutline::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	if ( UIEditor != NULL && UIEditor->ShouldRenderContainerOutline() )
	{
		FVector2D StartPos, EndPos;
		StartPos.X = UIEditor->OwnerScene->GetPosition( UIFACE_Left, EVALPOS_PixelViewport, TRUE);
		StartPos.Y = UIEditor->OwnerScene->GetPosition( UIFACE_Top, EVALPOS_PixelViewport, TRUE);
		EndPos.X = UIEditor->OwnerScene->GetPosition( UIFACE_Right, EVALPOS_PixelViewport, TRUE);
		EndPos.Y = UIEditor->OwnerScene->GetPosition( UIFACE_Bottom, EVALPOS_PixelViewport, TRUE);

		DrawBox2D(Canvas, StartPos, EndPos, OutlineColor );
	}
}

/* ==========================================================================================================
	FRenderModifier_ViewportOutline
========================================================================================================== */
/**
 * Constructor
 *
 * @param	InEditor	the editor window that holds the container to render the outline for
 */
FRenderModifier_ViewportOutline::FRenderModifier_ViewportOutline( WxUIEditorBase* InEditor )
: FUIEditor_RenderModifier(InEditor), OutlineColor(0,0,255,255)	// blue
{
}

/**
 * Renders an outline indicating the size of the viewport as it is reported to the scene being edited.
 */
void FRenderModifier_ViewportOutline::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	if ( ObjectVC != NULL && UIEditor != NULL && UIEditor->ShouldRenderViewportOutline() )
	{
		if ( UIEditor->EditorOptions->CurrentSSPreviewMode != eSST_NONE || UIEditor->EditorOptions->ViewportGutterSize > 0 )
		{
			FVector2D FullViewportStart(EC_EventParm), FullViewportEnd(EC_EventParm);
			FVector2D BoundingRegionStart(EC_EventParm), BoundingRegionEnd(EC_EventParm);
			
			ObjectVC->GetClientRenderRegion(FullViewportStart, FullViewportEnd, FALSE);
			BoundingRegionStart.X = FullViewportStart.X - UIEditor->EditorOptions->ViewportGutterSize;
			BoundingRegionStart.Y = FullViewportStart.Y - UIEditor->EditorOptions->ViewportGutterSize;
			BoundingRegionEnd.X = FullViewportEnd.X + UIEditor->EditorOptions->ViewportGutterSize;
			BoundingRegionEnd.Y = FullViewportEnd.Y + UIEditor->EditorOptions->ViewportGutterSize;

			// draw the outline of the full-screen viewport, without gutter or splitscreen adjustments
			FLinearColor FullViewportOutlineColor(OutlineColor);
			FullViewportOutlineColor.B = 0.878f;	// 224 / 255
			DrawBox2D(Canvas, BoundingRegionStart, BoundingRegionEnd, FullViewportOutlineColor);
			
			if ( UIEditor->EditorOptions->ViewportGutterSize > 0 )
			{
				// draw the outline of the full-screen viewport, with gutter values applied
				DrawBox2D( Canvas, FullViewportStart, FullViewportEnd, OutlineColor );
			}

			if ( UIEditor->EditorOptions->CurrentSSPreviewMode != eSST_NONE )
			{
				FVector2D SSViewportStart(EC_EventParm), SSViewportEnd(EC_EventParm);
				ObjectVC->GetClientRenderRegion(SSViewportStart, SSViewportEnd);
				FullViewportOutlineColor.B = 1.122f;	// 286 / 255 - bloom?

				DrawBox2D( Canvas, SSViewportStart, SSViewportEnd, FullViewportOutlineColor );
			}
		}
		else
		{
			FVector2D StartPos(EC_EventParm), EndPos(EC_EventParm);
			ObjectVC->GetClientRenderRegion(StartPos, EndPos);
			
			DrawBox2D(Canvas, StartPos, EndPos, OutlineColor );
		}
	}
}

/* ==========================================================================================================
	FRenderModifier_UIListColumnBoundary
========================================================================================================== */
/**
 * Constructor
 *
 * @param	InEditor	the editor window that holds the container to render the outline for
 */
FRenderModifier_UIListCellBoundary::FRenderModifier_UIListCellBoundary( WxUIEditorBase* InEditor, UUIList* InSelectedList )
: FRenderModifier_CustomWidget(InEditor, InSelectedList), SelectedList(InSelectedList)
{
}

/**
 * Renders an outline indicating the size of the viewport as it is reported to the scene being edited.
 */
void FRenderModifier_UIListCellBoundary::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	if ( ObjectVC != NULL && SelectedList != NULL )
	{
		if ((SelectedList->CellLinkType == LINKED_Columns && SelectedList->ColumnAutoSizeMode != CELLAUTOSIZE_None)
		||	(SelectedList->CellLinkType == LINKED_Rows && SelectedList->RowAutoSizeMode != CELLAUTOSIZE_None) )
		{
			return;
		}
		if ( SelectedList->CellDataComponent != NULL && SelectedList->CellDataComponent->GetSchemaCellCount() > 0 )
		{
			const INT SchemaCellCount = SelectedList->CellDataComponent->GetSchemaCellCount();
			Canvas->PushDepthSortKey(appTrunc(SelectedList->GetZDepth()));
			Canvas->PushRelativeTransform(SelectedList->GenerateTransformMatrix());

			const FLOAT BoundarySize = Canvas->IsHitTesting() ? UCONST_ResizeBufferPixels : 2.f;
			if ( SelectedList->CellLinkType == LINKED_Columns )
			{
				const FLOAT LineY = SelectedList->RenderBounds[UIFACE_Top];
				const FLOAT LineYL = SelectedList->GetRowHeight(INDEX_NONE, TRUE) + SelectedList->HeaderElementSpacing.GetValue(SelectedList);

				for ( INT CellIndex = 0; CellIndex < SchemaCellCount; CellIndex++ )
				{
					const FLOAT CellWidth = SelectedList->GetColumnWidth(CellIndex);
					const FLOAT LineX = Max<FLOAT>(0.f, SelectedList->CellDataComponent->GetSchemaCellPosition(CellIndex)) + CellWidth - (BoundarySize * 0.5f);
					if ( LineX > SelectedList->RenderBounds[UIFACE_Right] )
					{
						break;
					}

					// draw the line
					if ( Canvas->IsHitTesting() ) Canvas->SetHitProxy(new HUICellBoundaryHitProxy(SelectedList, CellIndex));
					DrawTile(Canvas, LineX, LineY, BoundarySize, LineYL, 0, 0, 0, 0, FLinearColor(1.f,1.f,1.f,1.f));
					if ( Canvas->IsHitTesting() ) Canvas->SetHitProxy(NULL);
				}
			}
			else
			{
				FLOAT LineX = SelectedList->RenderBounds[UIFACE_Left];
				FLOAT LineXL = SelectedList->GetColumnWidth(INDEX_NONE, TRUE) + SelectedList->HeaderElementSpacing.GetValue(SelectedList);

				for ( INT CellIndex = 0; CellIndex < SchemaCellCount; CellIndex++ )
				{
					const FLOAT CellHeight = SelectedList->GetRowHeight(CellIndex);
					const FLOAT LineY = Max<FLOAT>(0.f, SelectedList->CellDataComponent->GetSchemaCellPosition(CellIndex)) + CellHeight - (BoundarySize * 0.5f);
					if ( LineY > SelectedList->RenderBounds[UIFACE_Bottom] )
					{
						break;
					}

					// draw the line
					if ( Canvas->IsHitTesting() ) Canvas->SetHitProxy(new HUICellBoundaryHitProxy(SelectedList, CellIndex));
					DrawTile(Canvas, LineX, LineY, LineXL, BoundarySize, 0, 0, 0, 0, FLinearColor(1.f,1.f,1.f,1.f));
					if ( Canvas->IsHitTesting() ) Canvas->SetHitProxy(NULL);
				}
			}

			Canvas->PopTransform();
			Canvas->PopDepthSortKey();
		}
	}
}

/* ==========================================================================================================
	FWidgetCollection
========================================================================================================== */

/** Constructors */
FWidgetCollector::FWidgetCollector()
: FilterFlags(RF_AllFlags)
{
}

/**
 * Adds the specified flag/s to the collection filter.  Only widgets with the specified ObjectFlags will be included in the results.
 */
void FWidgetCollector::ApplyFilter( EObjectFlags RequiredFlags )
{
	FilterFlags |= RequiredFlags;
}

/**
 * Adds the specified style to the collection filter.   Only widgets with this style (or a style based on this one) will be included in the results.
 *
 * @param	RequiredStyle	the style to filter for
 * @param	bIncludeDerived	if TRUE, styles which are based on RequiredStyle will also be included
 */
void FWidgetCollector::ApplyFilter( UUIStyle* RequiredStyle, UBOOL bIncludeDerived/*=TRUE*/ )
{
	if ( RequiredStyle != NULL )
	{
		// add this style to the list
        FilterStyles.AddUniqueItem(RequiredStyle);

		if ( bIncludeDerived )
		{
			// also add any styles which are based on this one.
			TArray<UUIStyle*> AvailableStyles;
			UUISkin* ActiveSkin = GUnrealEd->GetBrowserManager()->UISceneManager->ActiveSkin;
			check(ActiveSkin);

			ActiveSkin->GetAvailableStyles(AvailableStyles);
			for ( INT i = 0; i < AvailableStyles.Num(); i++ )
			{
				UUIStyle* Style = AvailableStyles(i);
				if ( Style->IsBasedOnArchetype(RequiredStyle) )
				{
					FilterStyles.AddUniqueItem(Style);
				}
			}
		}
	}
}

/**
 * Builds the list of widgets which match the filters for this collector.
 *
 * @param	Container	the screen object to use to start the recursion.  Container will NOT be included in the results.
 */
void FWidgetCollector::Process( UUIScreenObject* Container )
{
	for ( INT i = 0; i < Container->Children.Num(); i++ )
	{
		UUIObject* Widget = Container->Children(i);
		Integrate(Widget);
	}
}

/**
 * Adds the widget specified widget to the list if it passes the current filters, then recurses through all children of the widget.
 */
void FWidgetCollector::Integrate( UUIObject* Widget )
{
	UBOOL bShouldAdd = TRUE;

	// see if this widget passes the requirements to be added to the list
	if ( !Widget->HasAnyFlags(FilterFlags) )
	{
		bShouldAdd = FALSE;
	}

	if ( bShouldAdd && FilterStyles.Num() > 0 )
	{
		bShouldAdd = FALSE;
		for ( INT i = 0; i < FilterStyles.Num(); i++ )
		{
			UUIStyle* Style = FilterStyles(i);
			if ( Widget->UsesStyle(Style) )
			{
				bShouldAdd = TRUE;
				break;
			}
		}
	}

	if ( bShouldAdd )
	{
		AddUniqueItem(Widget);
	}

	for ( INT i = 0; i < Widget->Children.Num(); i++ )
	{
		UUIObject* Child = Widget->Children(i);
		Integrate(Child);
	}
}

/* ============================================
    UUIEditorOptions
   ==========================================*/
/**
 * Retrieves the size that the UI should use as the size of the viewport when resolving positions.
 *
 * @param	RenderViewport	if the VirtualSize is 'Auto', the output values are filled using the size of this viewport.
 * @param	out_SizeX		receives the horizontal size of the viewport
 * @param	out_SizeY		receives the vertical size of the viewport
 * @param	bIgnoreSSLayout	specify TRUE to ignore the current split-screen layout preview mode and assume fullscreen
 * @param	bIgnoreGutter	specify TRUE to ignore any gutter value being applied.
 *
 * @return	TRUE if a virtual viewport size has been set, FALSE if the virtual viewport size is "Auto"
 */
UBOOL UUIEditorOptions::GetVirtualViewportSize( FViewport* RenderViewport, FLOAT& out_SizeX, FLOAT& out_SizeY, UBOOL bIgnoreSSLayout/*=FALSE*/, UBOOL bIgnoreGutter/*=FALSE*/ ) const
{
	check(RenderViewport);

	out_SizeX = VirtualSizeX;
	out_SizeY = VirtualSizeY;
	if ( VirtualSizeX == UCONST_AUTOEXPAND_VALUE )
	{
		out_SizeX = RenderViewport->GetSizeX();
	}
	if ( VirtualSizeY == UCONST_AUTOEXPAND_VALUE )
	{
		out_SizeY = RenderViewport->GetSizeY();
	}

	if ( !bIgnoreSSLayout )
	{
		UGameViewportClient* ViewportClientCDO = GEngine->GameViewportClientClass->GetDefaultObject<UGameViewportClient>();

		if (CurrentSSPreviewMode < ViewportClientCDO->SplitscreenInfo.Num()
		&&	ViewportClientCDO->SplitscreenInfo(CurrentSSPreviewMode).PlayerData.Num() > 0 )
		{
			out_SizeX *= ViewportClientCDO->SplitscreenInfo(CurrentSSPreviewMode).PlayerData(0).SizeX;
			out_SizeY *= ViewportClientCDO->SplitscreenInfo(CurrentSSPreviewMode).PlayerData(0).SizeY;
		}
	}

	if ( !bIgnoreGutter )
	{
		BYTE SSLayout = bIgnoreSSLayout ? eSST_NONE : CurrentSSPreviewMode;
		switch( SSLayout )
		{
		case eSST_NONE:
			out_SizeX -= ViewportGutterSize * 2;
			out_SizeY -= ViewportGutterSize * 2;
			break;

		case eSST_2P_HORIZONTAL:
		case eSST_3P_FAVOR_TOP:
			out_SizeX -= ViewportGutterSize * 2;
			out_SizeY -= ViewportGutterSize;
			break;

		case eSST_2P_VERTICAL:
			out_SizeX -= ViewportGutterSize;
			out_SizeY -= ViewportGutterSize * 2;
			break;

		case eSST_3P_FAVOR_BOTTOM:
		case eSST_4P:
			out_SizeX -= ViewportGutterSize;
			out_SizeY -= ViewportGutterSize;
			break;
		}
	}

	return VirtualSizeX != UCONST_AUTOEXPAND_VALUE || VirtualSizeY != UCONST_AUTOEXPAND_VALUE;
}

/**
 * Returns a string representation of the virtual viewport size, in the format:
 * 640x480
 * (assuming VirtualSizeX is 640 and VirtualSizeY is 480)
 *
 * @return	a string representing the configured viewport size, or an empty string to indicate that the viewport
 *			should take up the entire window (auto-expand)
 */
FString UUIEditorOptions::GetViewportSizeString() const
{
	if ( VirtualSizeX != UCONST_AUTOEXPAND_VALUE && VirtualSizeY != UCONST_AUTOEXPAND_VALUE )
	{
		return FString::Printf(TEXT("%ix%i"), VirtualSizeX, VirtualSizeY);
	}

	return TEXT("");
}

/**
 * Sets the values for VirtualSizeX and VirtualSizeY from a string in the format:
 * 640x480
 * (assuming you wanted to set VirtualSizeX to 640 and VirtualSizeY to 480).  To indicate
 * that the viewport should take up the entire window (auto-expand), pass in an empty string.
 *
 * @return	TRUE if the input string was successfully parsed into a SizeX and SizeY value.
 */
UBOOL UUIEditorOptions::SetViewportSize( const FString& ViewportSizeString )
{
	if ( ViewportSizeString.Len() == 0 )
	{
		VirtualSizeX = VirtualSizeY = UCONST_AUTOEXPAND_VALUE;
		return TRUE;
	}

	// search for the x...InStr is case-sensitive, so search for both versions
	INT DelimPos = ViewportSizeString.InStr(TEXT("x"));
	if ( DelimPos == INDEX_NONE )
	{
		DelimPos = ViewportSizeString.InStr(TEXT("X"));
	}

	if ( DelimPos != INDEX_NONE )
	{
		// found the delimiter, separate it out into individual strings
		FString SizeXString = ViewportSizeString.Left(DelimPos).Trim();
		FString SizeYString = ViewportSizeString.Mid(DelimPos+1).Trim();

		// if either side was a zero length string, bail out
		if ( SizeXString.Len() == 0 || SizeYString.Len() == 0 )
		{
			return FALSE;
		}

		// convert the strings into integers
		INT NewSizeX = appAtoi(*SizeXString);
		INT NewSizeY = appAtoi(*SizeYString);

		// if either of the strings couldn't be converted, bail out
		if ( NewSizeX == 0 || NewSizeY == 0 )
		{
			return FALSE;
		}

		VirtualSizeX = NewSizeX;
		VirtualSizeY = NewSizeY;
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================================================
	FUILayerNode
========================================================================================================== */

/**
 * Changes the object associated with this layer node.
 *
 * @param	InObject	the object to associate with this layer node; must be of type UIObject or UILayer
 */
UBOOL FUILayerNode::SetLayerObject( UObject* InObject )
{
	UBOOL bResult = FALSE;
	
	// if passing in a null object, clear our value
	if ( InObject == NULL )
	{
		LayerObject = NULL;
		bResult = TRUE;
	}
	
	// only allow UIObject and UILayer objects to be assigned to UILayerNodes
	else if ( Cast<UUIObject>(InObject) != NULL || Cast<UUILayer>(InObject) != NULL )
	{
		LayerObject = InObject;
		bResult = TRUE;
	}
	
	return bResult;
}

/**
 * Returns TRUE if the object associated with this layer node is of type UILayer.
 */
UBOOL FUILayerNode::IsUILayer() const
{
	return Cast<UUILayer>(LayerObject) != NULL;
}

/**
 * Returns TRUE if the object associated with this layer node is of type UIObject.
 */
UBOOL FUILayerNode::IsUIObject() const
{
	return Cast<UUIObject>(LayerObject) != NULL;
}

/**
 * Gets the object associated with this layer node, casted to a UILayer.
 */
UUILayer* FUILayerNode::GetUILayer() const
{
	return Cast<UUILayer>(LayerObject);
}

/**
 * Gets the object associated with this layer node, casted to a UIObject.
 */
UUIObject* FUILayerNode::GetUIObject() const
{
	return Cast<UUIObject>(LayerObject);
}

/* ==========================================================================================================
	UUILayer
========================================================================================================== */
/** Standard constructors */
FUILayerNode::FUILayerNode( UUILayer* InLayer, UUILayer* InParentLayer )
: bLocked(FALSE), bVisible(TRUE), LayerObject(InLayer), ParentLayer(InParentLayer)
{}
FUILayerNode::FUILayerNode( UUIObject* InWidget, UUILayer* InParentLayer )
: bLocked(FALSE), bVisible(TRUE), LayerObject(InWidget), ParentLayer(InParentLayer)
{}

/**
 * Inserts the specified node at the specified location
 *
 * @param	NodeToInsert	the layer node that should be inserted into this UILayer's LayerNodes array
 * @param	InsertIndex		if specified, the index where the new node should be inserted into the LayerNodes array. if not specified
 *							the new node will be appended to the end of the array.
 *
 * @return	TRUE if the node was successfully inserted into this UILayer's list of child nodes.
 */
UBOOL UUILayer::InsertNode( const FUILayerNode& NodeToInsert, INT InsertIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( InsertIndex == INDEX_NONE )
	{
		InsertIndex = LayerNodes.Num();
	}

	if ( InsertIndex >= 0 && InsertIndex <= LayerNodes.Num() )
	{
		new(LayerNodes, InsertIndex) FUILayerNode(NodeToInsert);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Removes the specified node
 *
 * @param	ExistingNode	the layer node that should be removed from this UILayer's LayerNodes array
 *
 * @return	TRUE if the node was successfully removed from this UILayer's list of child nodes.
 */
UBOOL UUILayer::RemoveNode(const struct FUILayerNode& ExistingNode)
{
	UBOOL bResult = FALSE;

	INT NodeIndex = INDEX_NONE;

	NodeIndex = FindNodeIndex(ExistingNode.GetLayerObject());

	if(NodeIndex != INDEX_NONE)
	{
		LayerNodes.Remove(NodeIndex);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Finds the index [into the LayerNodes array] for a child node that has a name matching the string specified.
 *
 * @param	NodeTitle	the name of the layer node to search for.  If searching for a UILayer node, should be the UILayer's LayerName;
 *						if searching for a UIObject node type, should be the WidgetTag of the UIObject.
 *
 * @return	the index into the LayerNodes array for the child node which has a name matching the string specified, or INDEX_NONE if no
 *			child nodes were found with that name.
 */
INT UUILayer::FindNodeIndex(const UObject* NodeObject) const
{
	INT Result = INDEX_NONE;

	for(INT NodeIndex = 0; NodeIndex < LayerNodes.Num(); NodeIndex++)
	{
		if(NodeObject == LayerNodes(NodeIndex).GetLayerObject())
		{
			Result = NodeIndex;
			break;
		}
	}
	return Result;
}

/**
 * Retrieves the list of child nodes for this UILayer.
 *
 * @param	out_ChildNodes	receives the list of children of this UILayerNode
 * @param	bRecurse	if FALSE, will only add children of this UILayerNode; if TRUE, will add children of this UILayerNode,
 *						along with the children of those nodes, recursively.
 *
 * @return	TRUE if this nodes were added to the out_ChildNodes array, FALSE otherwise.
 */
UBOOL UUILayer::GetChildNodes( TArray<FUILayerNode*>& out_ChildNodes, UBOOL bRecurse/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

	for ( INT NodeIndex = 0; NodeIndex < LayerNodes.Num(); NodeIndex++ )
	{
		out_ChildNodes.AddItem(&LayerNodes(NodeIndex));
		bResult = TRUE;
	}

	if ( bRecurse == TRUE )
	{
		for ( INT NodeIndex = 0; NodeIndex < LayerNodes.Num(); NodeIndex++ )
		{
			UUILayer* NodeLayer = LayerNodes(NodeIndex).GetUILayer();

			// only nodes associated with UILayers have child nodes
			if ( NodeLayer != NULL )
			{
				bResult = NodeLayer->GetChildNodes(out_ChildNodes,bRecurse) || bResult;
			}
		}
	}

	return bResult;
}

/**
 * Returns the child layer node that contains the specified object as its layer object.
 *
 * @param	NodeObject	the child layer object to look for
 * @param	bRecurse	if TRUE, searches all children of this object recursively
 *
 * @return	a pointer to a node contained by this object that has the specified object, or
 *			NULL if no node with that specified object was found
 */
FUILayerNode* UUILayer::FindChild( UObject* NodeObject, UBOOL bRecurse )
{
	FUILayerNode* Result = NULL;

	TArray<FUILayerNode*> LayerChildren;
	if(GetChildNodes(LayerChildren, bRecurse))
	{
		for(INT i = 0; i < LayerChildren.Num(); i++)
		{
			if(NodeObject == LayerChildren(i)->GetLayerObject())
			{
				Result = LayerChildren(i);
				break;
			}
		}
	}

	return Result;
}

/**
 * Find any child nodes with the specified name
 *
 * @param	NodeTitle	the name of the child to find
 * @param	out_ChildNodes	receives the list of children
 * @param	bRecurse	if TRUE, searches all children of this object recursively
 *
 * @return	number of children added to out_ChildNodes
 */
INT UUILayer::FindChildNodes( const FString& NodeTitle, TArray<FUILayerNode*>& out_ChildNodes, UBOOL bRecurse )
{
	INT NumFound = 0;

	for ( INT NodeIndex = 0; NodeIndex < LayerNodes.Num(); NodeIndex++ )
	{
		UUILayer* NodeLayer = LayerNodes(NodeIndex).GetUILayer();
		if ( NodeLayer != NULL )
		{
			if ( NodeLayer->LayerName == NodeTitle )
			{
				out_ChildNodes.AddItem(&LayerNodes(NodeIndex));
				NumFound++;
			}

			if ( bRecurse == TRUE )
			{
				NumFound += NodeLayer->FindChildNodes(NodeTitle,out_ChildNodes,bRecurse);
			}
		}
		else
		{
			UUIObject* LayerWidget = LayerNodes(NodeIndex).GetUIObject();
			if ( LayerWidget != NULL && LayerWidget->WidgetTag == *NodeTitle )
			{
				out_ChildNodes.AddItem(&LayerNodes(NodeIndex));
				NumFound++;
			}
		}
	}

	return NumFound;
}

/**
 * Returns whether this layer contains the specified child in its list of children.
 *
 * @param	NodeTitle	the name of the child layer to look for
 * @param	bRecurse	if TRUE, searches all children of this object recursively
 *
 * @return	TRUE if the child layer is contained by this layer object
 */
UBOOL UUILayer::ContainsChild( const FString& NodeTitle, UBOOL bRecurse )
{
	UBOOL bResult = FALSE;

	TArray<FUILayerNode*> LayerChildren;
	if(GetChildNodes(LayerChildren, bRecurse))
	{
		for(INT i = 0; i < LayerChildren.Num(); i++)
		{
			const FUILayerNode* LayerNode = LayerChildren(i);

			UUILayer* NodeLayer = LayerNode->GetUILayer();
			if ( NodeLayer != NULL )
			{
				if ( NodeLayer->LayerName == NodeTitle )
				{
					bResult = TRUE;
					break;
				}
			}
			else
			{
				UUIObject* LayerWidget = LayerNode->GetUIObject();
				if ( LayerWidget != NULL && LayerWidget->WidgetTag == *NodeTitle )
				{
					bResult = TRUE;
					break;
				}
			}
		}
	}

	return bResult;
}

/* === UObject interface === */
/**
 * Prior to 06-28, UILayer objects used to have the RF_Standalone flag.  Clear the flag if it is still set on this UILayer.
 */
void UUILayer::PostLoad()
{
	Super::PostLoad();

	ClearFlags(RF_Standalone);
}

