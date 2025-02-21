/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "Factories.h"
#include "PhAT.h"
#include "UnTerrain.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "EnginePrefabClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineSoundClasses.h"
#include "LensFlare.h"
#include "AnimSetViewer.h"
#include "UnLinkedObjEditor.h"
#include "SoundCueEditor.h"
#include "DlgSoundNodeWaveOptions.h"
#include "DialogueManager.h"
#include "AnimTreeEditor.h"
#include "PostProcessEditor.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UnrealEdPrivateClasses.h"			// required for declaration of UUILayerRoot
#include "GenericBrowser.h"
#include "GenericBrowserContextMenus.h"
#include "Cascade.h"
#include "StaticMeshEditor.h"
#include "NewMaterialEditor.h"
#include "MaterialInstanceConstantEditor.h"
#include "MaterialInstanceTimeVaryingEditor.h"
#include "Properties.h"
#include "DlgUISkinEditor.h"
#include "InterpEditor.h"
#include "LensFlareEditor.h"
#include "TrackableWindow.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "DlgNewPhysXDestructible.h"

#if WITH_FACEFX
	#include "../../../External/FaceFX/Studio/Main/Inc/FxStudioApp.h"
	#include "../FaceFX/FxRenderWidgetUE3.h"
#endif // WITH_FACEFX

#include "..\..\Launch\Resources\resource.h"

/*------------------------------------------------------------------------------
	Implementations.
------------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UGenericBrowserType)
IMPLEMENT_CLASS(UGenericBrowserType_All)
IMPLEMENT_CLASS(UGenericBrowserType_AnimTree)
IMPLEMENT_CLASS(UGenericBrowserType_Animation)
IMPLEMENT_CLASS(UGenericBrowserType_Archetype)
IMPLEMENT_CLASS(UGenericBrowserType_CameraAnim)
IMPLEMENT_CLASS(UGenericBrowserType_CurveEdPresetCurve)
IMPLEMENT_CLASS(UGenericBrowserType_Custom)
IMPLEMENT_CLASS(UGenericBrowserType_DecalMaterial)
IMPLEMENT_CLASS(UGenericBrowserType_FaceFXAnimSet)
IMPLEMENT_CLASS(UGenericBrowserType_FaceFXAsset)
IMPLEMENT_CLASS(UGenericBrowserType_Font)
IMPLEMENT_CLASS(UGenericBrowserType_FracturedStaticMesh)
IMPLEMENT_CLASS(UGenericBrowserType_LensFlare)
IMPLEMENT_CLASS(UGenericBrowserType_Material)
IMPLEMENT_CLASS(UGenericBrowserType_MaterialInstanceConstant)
IMPLEMENT_CLASS(UGenericBrowserType_MaterialInstanceTimeVarying)
IMPLEMENT_CLASS(UGenericBrowserType_MaterialLackingPhysMat)
IMPLEMENT_CLASS(UGenericBrowserType_MorphTargetSet)
IMPLEMENT_CLASS(UGenericBrowserType_MorphWeightSequence)
IMPLEMENT_CLASS(UGenericBrowserType_ParticleSystem)
IMPLEMENT_CLASS(UGenericBrowserType_PhysicalMaterial)
IMPLEMENT_CLASS(UGenericBrowserType_PhysXParticleSystem)
IMPLEMENT_CLASS(UGenericBrowserType_PhysXDestructible)
IMPLEMENT_CLASS(UGenericBrowserType_PhysicsAsset)
IMPLEMENT_CLASS(UGenericBrowserType_PostProcess);
IMPLEMENT_CLASS(UGenericBrowserType_Prefab)
IMPLEMENT_CLASS(UGenericBrowserType_RenderTexture)
IMPLEMENT_CLASS(UGenericBrowserType_Sequence)
IMPLEMENT_CLASS(UGenericBrowserType_SkeletalMesh)
IMPLEMENT_CLASS(UGenericBrowserType_SoundCue)
IMPLEMENT_CLASS(UGenericBrowserType_SoundMode)
IMPLEMENT_CLASS(UGenericBrowserType_SoundWave)
IMPLEMENT_CLASS(UGenericBrowserType_Sounds)
IMPLEMENT_CLASS(UGenericBrowserType_SpeechRecognition)
IMPLEMENT_CLASS(UGenericBrowserType_StaticMesh)
IMPLEMENT_CLASS(UGenericBrowserType_TerrainLayer)
IMPLEMENT_CLASS(UGenericBrowserType_TerrainMaterial)
IMPLEMENT_CLASS(UGenericBrowserType_Texture)
IMPLEMENT_CLASS(UGenericBrowserType_TextureWithAlpha)
IMPLEMENT_CLASS(UGenericBrowserType_UIArchetype)
IMPLEMENT_CLASS(UGenericBrowserType_UIScene)
IMPLEMENT_CLASS(UGenericBrowserType_UISkin)

/*-----------------------------------------------------------------------------
	WxDlgTextureProperties.
-----------------------------------------------------------------------------*/

class WxDlgTextureProperties : public WxTrackableDialog, FViewportClient, public FNotifyHook
{
public:
	WxDlgTextureProperties();
	~WxDlgTextureProperties();

	bool Show( const bool InShow, UTexture* InTexture );

	/////////////////////////
	// FViewportClient interface.

	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	/////////////////////////
	// FNotify interface.

	virtual void NotifyDestroy( void* Src );
	virtual void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );

private:
	UTexture* Texture;
	WxViewportHolder* ViewportHolder;
	FViewport* Viewport;
	WxPropertyWindow* PropertyWindow;

	void OnOK( wxCommandEvent& In );
	void OnSize( wxSizeEvent& In );

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxDlgTextureProperties, WxTrackableDialog)
	EVT_BUTTON( wxID_OK, WxDlgTextureProperties::OnOK )
	EVT_SIZE( WxDlgTextureProperties::OnSize )
END_EVENT_TABLE()

WxDlgTextureProperties::WxDlgTextureProperties()
	:	Viewport( NULL)
{
	PropertyWindow = NULL;
	ViewportHolder = NULL;

	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_TEXTURE_PROPERTIES") );
	check( bSuccess );

	// Property window for editing the texture

	wxWindow* win = FindWindow( XRCID( "ID_PROPERTY_WINDOW" ) );
	check( win != NULL );
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create(win, this);
	wxRect rc = win->GetRect();
	PropertyWindow->SetSize( 0, 0, rc.GetWidth(), rc.GetHeight() );
	win->Show();

	// Viewport to show the texture in

	ViewportHolder = new WxViewportHolder( (wxWindow*)this, -1, 0 );
	win = (wxWindow*)FindWindow( XRCID( "ID_VIEWPORT_WINDOW" ) );
	check( win != NULL );
	rc = win->GetRect();
	Viewport = GEngine->Client->CreateWindowChildViewport( this, (HWND)ViewportHolder->GetHandle(), rc.width, rc.height );
	Viewport->CaptureJoystickInput(false);
	ViewportHolder->SetViewport( Viewport );
	ViewportHolder->SetSize( rc );

	FWindowUtil::LoadPosSize( TEXT("DlgTextureProperties"), this );
	FLocalizeWindow( this );
}

WxDlgTextureProperties::~WxDlgTextureProperties()
{
	FWindowUtil::SavePosSize( TEXT("DlgTextureProperties"), this );

	GEngine->Client->CloseViewport(Viewport); 
	Viewport = NULL;
	delete PropertyWindow;
}

bool WxDlgTextureProperties::Show( bool InShow, UTexture* InTexture )
{
	Texture = InTexture;

	PropertyWindow->SetObject( Texture, 1,1,0 );

	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("TexturePropertiesCaption"), *Texture->GetPathName(), *Texture->GetDesc()) ) );

	return wxDialog::Show(InShow);
}

void WxDlgTextureProperties::OnOK( wxCommandEvent& In )
{
	PropertyWindow->FlushLastFocused();
	Destroy();
}

void WxDlgTextureProperties::OnSize( wxSizeEvent& In )
{
	// @fixme: We can't verify these FindWindow calls because OnSize is getting called before the ctor!!!!!
	wxWindow* win = (wxWindow*)FindWindow( XRCID( "ID_PROPERTY_WINDOW" ) );
	wxRect Rect;
	if( win && PropertyWindow )
	{
		Rect = win->GetRect();
		PropertyWindow->SetSize(0,0,Rect.GetWidth(),Rect.GetHeight());
	}
	// Size the viewport too.
	win = (wxWindow*)FindWindow( XRCID( "ID_VIEWPORT_WINDOW" ) );
	if (win && ViewportHolder)
	{
#if 0// @todo joeg ask a sizer guru what needs to change for this code to work
		// Get the current viewport size
		wxRect ViewportRect = ViewportHolder->GetRect();
		// Only change the bottom of the rect
		ViewportRect.SetBottom(Rect.GetBottom());
		// Tell it to update
		ViewportHolder->SetSize(ViewportRect);
		win->SetSize(ViewportRect);
#endif
		// Force a redraw
		Viewport->Invalidate();
	}
	In.Skip();
}

void WxDlgTextureProperties::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	Clear(Canvas, FColor(0,0,0) );
	// Get the rendering info for this object
	FThumbnailRenderingInfo* RenderInfo =
		GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Texture);
	// If there is an object configured to handle it, draw the thumbnail
	if (RenderInfo != NULL && RenderInfo->Renderer != NULL)
	{
		DWORD Width, Height;
		// Figure out the size we need
		RenderInfo->Renderer->GetThumbnailSize(Texture,
			TPT_Plane,1.f,Width,Height);
		// See if we need to uniformly scale it to fit in viewport
		DWORD ViewportW = Viewport->GetSizeX();
		DWORD ViewportH = Viewport->GetSizeY();
		if( Width > ViewportW )
		{
			Height = Height * ViewportW / Width;
			Width = ViewportW;
		}
		if( Height > ViewportH )
		{
			Width = Width * ViewportH / Height;
			Height = ViewportH;
		}
		// Now draw it
		RenderInfo->Renderer->Draw(Texture,TPT_Plane,0,0,Width,Height,
			Viewport,Canvas,TBT_None, FColor(0, 0, 0), FColor(0, 0, 0));
	}
}

void WxDlgTextureProperties::NotifyDestroy( void* Src )
{
	if( Src == PropertyWindow )
	{
		PropertyWindow = NULL;
	}
}

void WxDlgTextureProperties::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	Viewport->Invalidate();
}

/*------------------------------------------------------------------------------
	UGenericBrowserType
------------------------------------------------------------------------------*/

UBOOL UGenericBrowserType::Supports( UObject* InObject )
{
	check( InObject );
	for( INT x = 0 ; x < SupportInfo.Num() ; ++x )
	{
		if( SupportInfo(x).Supports(InObject) )
		{
			return TRUE;
		}
	}

	return 0;
}

FColor UGenericBrowserType::GetBorderColor( UObject* InObject )
{
	check( InObject );
	for( INT x = 0 ; x < SupportInfo.Num() ; ++x )
	{
		if( SupportInfo(x).Supports(InObject) )
		{
			return SupportInfo(x).BorderColor;
		}
	}

	return FColor(255,255,255);
}

WxMBGenericBrowserContextBase* UGenericBrowserType::GetContextMenu( USelection* Selection )
{
	check( Selection );

	// Find the first type that supports all selected objects and has a context menu.
	for( INT InfoIndex = 0 ; InfoIndex < SupportInfo.Num() ; ++InfoIndex )
	{
		UBOOL bSelectionMatchesThisType = TRUE;
		for ( USelection::TObjectIterator It( Selection->ObjectItor() ) ; It ; ++It )
		{
			UObject* Object = *It;
			if ( Object )
			{
				if( !SupportInfo(InfoIndex).Supports(Object) )
				{
					bSelectionMatchesThisType = FALSE;
				}
			}
		}

		if(bSelectionMatchesThisType && SupportInfo(InfoIndex).ContextMenu != NULL)
		{
			return SupportInfo(InfoIndex).ContextMenu;
		}
	}

	return (new WxMBGenericBrowserContext_Object);
}

/**
 * Factorizes out the creation of a new property window frame for the
 * UGenericBrowserType::ShowOBjectProperties(...) family of methods.
 */
static inline void GenericBrowser_CreateNewPropertyWindowFrame()
{
	if(!GApp->ObjectPropertyWindow)
	{
		GApp->ObjectPropertyWindow = new WxPropertyWindowFrame;
		GApp->ObjectPropertyWindow->Create( GApp->EditorFrame, -1, TRUE, GUnrealEd );
		GApp->ObjectPropertyWindow->SetSize( 64,64, 350,600 );
	}
}

/**
 * Generic implementation for opening a property window for an object.
 */
UBOOL UGenericBrowserType::ShowObjectProperties( UObject* InObject )
{
	GenericBrowser_CreateNewPropertyWindowFrame();
	GApp->ObjectPropertyWindow->SetObject( InObject, TRUE, TRUE, TRUE );
	GApp->ObjectPropertyWindow->Show();
	return TRUE;
}

/**
 * Generic implementation for opening a property window for a set of objects.
 */
UBOOL UGenericBrowserType::ShowObjectProperties( const TArray<UObject*>& InObjects )
{
	GenericBrowser_CreateNewPropertyWindowFrame();
	GApp->ObjectPropertyWindow->SetObjectArray( InObjects, TRUE, TRUE, TRUE );
	GApp->ObjectPropertyWindow->Show();
	return TRUE;
}

/**
 * Invokes the editor for all selected objects.
 */
UBOOL UGenericBrowserType::ShowObjectEditor()
{
	UBOOL bAbleToShowEditor = FALSE;

	// Loop through all of the selected objects and see if any of our support classes support this object.
	// If one of them does, show the editor specific to this browser type and continue on to the next object.
	for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
	{
		UObject* Object = *It;
		
		for( INT SupportIdx = 0 ; SupportIdx < SupportInfo.Num() ; ++SupportIdx )
		{
			if( Object && SupportInfo(SupportIdx).Supports( Object ) && ShowObjectEditor( Object ) )
			{
				bAbleToShowEditor =  TRUE;
				break;
			}
		}
	}

	return bAbleToShowEditor;
}

/**
 * Displays the object properties window for all selected objects that this GenericBrowserType supports.
 */
UBOOL UGenericBrowserType::ShowObjectProperties()
{
	TArray<UObject*> SelectedObjects;
	GEditor->GetSelectedObjects()->GetSelectedObjects(SelectedObjects);
	return ShowObjectProperties(SelectedObjects);
}

void UGenericBrowserType::InvokeCustomCommand(INT InCommand)
{
	for( INT i = 0 ; i < SupportInfo.Num() ; ++i )
	{
		for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
		{
			UObject* Object = *It;
			if ( Object && SupportInfo(i).Supports( Object ) )
			{
				InvokeCustomCommand( InCommand, Object );
			}
		}
	}
}

void UGenericBrowserType::DoubleClick()
{
	for( INT i = 0 ; i < SupportInfo.Num() ; ++i )
	{
		for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
		{
			UObject* Object = *It;
			if ( Object && SupportInfo(i).Supports( Object ) )
			{
				DoubleClick( Object );
			}
		}
	}
}

void UGenericBrowserType::DoubleClick( UObject* InObject )
{
	// This is what most types want to do with a double click so we
	// handle this in the base class.  Override this function to
	// implement a custom behavior.

	if ( Supports(InObject) )
	{
		ShowObjectEditor( InObject );
	}
}

void UGenericBrowserType::GetSelectedObjects( TArray<UObject*>& Objects )
{
	for ( INT i = 0; i < SupportInfo.Num(); i++ )
	{
		for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
		{
			UObject* Object = *It;
			if ( Object && SupportInfo(i).Supports( Object ) )
			{
				Objects.AddUniqueItem( Object );
			}
		}
	}
}

/**
 * Determines whether the specified package is allowed to be saved.
 */
UBOOL UGenericBrowserType::IsSavePackageAllowed( UPackage* PackageToSave )
{
	UBOOL bResult = TRUE;

	if ( PackageToSave != NULL )
	{
		TArray<UObject*> StandaloneObjects;
		for ( TObjectIterator<UObject> It; It; ++It )
		{
			if ( It->IsIn(PackageToSave) && It->HasAnyFlags(RF_Standalone) && It->GetClass() != UObjectRedirector::StaticClass() )
			{
				StandaloneObjects.AddItem(*It);
			}
		}
		
		for( INT SupportIdx = 0 ; SupportIdx < SupportInfo.Num() ; ++SupportIdx )
		{
			UGenericBrowserType* InfoType = SupportInfo(SupportIdx).BrowserType;
			if ( InfoType != NULL && !InfoType->IsSavePackageAllowed(PackageToSave, StandaloneObjects) )
			{
				bResult = FALSE;
				break;
			}
		}
	}

	return bResult;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_All
------------------------------------------------------------------------------*/

/**
 * Does any initial set up that the type requires.
 */
void UGenericBrowserType_All::Init()
{
	for( TObjectIterator<UGenericBrowserType> It ; It ; ++It )
	{
		UGenericBrowserType* BrowserType = *It;

		if( !Cast<UGenericBrowserType_All>(BrowserType) && !Cast<UGenericBrowserType_Custom>(BrowserType) )
		{
			for( INT SupportIdx = 0 ; SupportIdx < BrowserType->SupportInfo.Num() ; ++SupportIdx )
			{
				const FGenericBrowserTypeInfo &TypeInfo = BrowserType->SupportInfo(SupportIdx);
				const UClass* ResourceClass = TypeInfo.Class;
				INT InsertIdx = -1;
				UBOOL bAddItem = TRUE;

				// Loop through all the existing classes checking for duplicate classes and making sure the point of insertion for a class
				// is before any of its parents.
				for(INT ClassIdx = 0; ClassIdx < SupportInfo.Num(); ClassIdx++)
				{
					UClass* PotentialParent = SupportInfo(ClassIdx).Class;

					if(PotentialParent == ResourceClass)
					{
						bAddItem = FALSE;
						break;
					}

					if(ResourceClass->IsChildOf(PotentialParent) == TRUE)
					{
						InsertIdx = ClassIdx;
						break;
					}
				}

				if(bAddItem == TRUE)
				{
					if(InsertIdx == -1)
					{
						SupportInfo.AddItem( TypeInfo );
					}
					else
					{
						SupportInfo.InsertItem( TypeInfo, InsertIdx );
					}
				}

			}
		}
	}
}

/*------------------------------------------------------------------------------
UGenericBrowserType_Custom
------------------------------------------------------------------------------*/

/**
 * Invokes the editor for all selected objects.
 */
UBOOL UGenericBrowserType_Custom::ShowObjectEditor()
{
	UBOOL bAbleToShowEditor = FALSE;

	// Loop through all of the selected objects and see if any of our support classes support this object.
	// If one of them does, show the editor specific to this browser type and continue on to the next object.
	for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
	{
		UObject* Object = *It;

		if ( Supports(Object) )
		{
			for( INT SupportIdx = 0 ; SupportIdx < SupportInfo.Num() ; ++SupportIdx )
			{
				FGenericBrowserTypeInfo& Info = SupportInfo(SupportIdx);

				if( Info.Supports(Object) )
				{
					if(Info.BrowserType == NULL)
					{
						for( TObjectIterator<UGenericBrowserType> It ; It ; ++It )
						{
							UGenericBrowserType* BrowserType = *It;

							if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(Object) )
							{
								bAbleToShowEditor = BrowserType->ShowObjectEditor(Object);
								break;
							}
						}
					}
					else
					{
						UGenericBrowserType* BrowserType = Info.BrowserType;
						if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(Object) )
						{
							bAbleToShowEditor = BrowserType->ShowObjectEditor(Object);
						}
					}

					break;
				}
			}
		}
	}

	return bAbleToShowEditor;
}

/**
 * Invokes the editor for an object.  The default behaviour is to
 * open a property window for the object.  Dervied classes can override
 * this with eg an editor which is specialized for the object's class.
 *
 * This version loops through all of the supported classes for the custom type and 
 * calls the appropriate implementation of the function.
 *
 * @param	InObject	The object to invoke the editor for.
 */
UBOOL UGenericBrowserType_Custom::ShowObjectEditor( UObject* InObject )
{
	UBOOL bAbleToShowEditor = FALSE;

	if ( Supports(InObject) == TRUE )
	{
		for( INT SupportIdx = 0 ; SupportIdx < SupportInfo.Num() ; ++SupportIdx )
		{
			FGenericBrowserTypeInfo& Info = SupportInfo(SupportIdx);

			if( Info.Supports(InObject) )
			{
				if(Info.BrowserType == NULL)
				{
					for( TObjectIterator<UGenericBrowserType> It ; It ; ++It )
					{
						UGenericBrowserType* BrowserType = *It;

						if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(InObject) )
						{
							bAbleToShowEditor = BrowserType->ShowObjectEditor(InObject);
							break;
						}
					}
				}
				else
				{
					UGenericBrowserType* BrowserType = Info.BrowserType;
					if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(InObject) )
					{
						bAbleToShowEditor = BrowserType->ShowObjectEditor(InObject);
					}
				}

				break;
			}
		}
	}
	
	return bAbleToShowEditor;
}

/**
 * Opens a property window for the specified objects.  By default, GEditor's
 * notify hook is used on the property window.  Derived classes can override
 * this method in order to eg provide their own notify hook.
 *
 * @param	InObjects	The objects to invoke the property window for.
 */
UBOOL UGenericBrowserType_Custom::ShowObjectProperties( const TArray<UObject*>& InObjects )
{
	UBOOL bShowedProperties = FALSE;
	const INT NumObjects = InObjects.Num();

	for( TObjectIterator<UGenericBrowserType> It ; It ; ++It )
	{
		UGenericBrowserType* BrowserType = *It;
		TArray<UObject*> Objects;

		if( !Cast<UGenericBrowserType_Custom>(BrowserType) )
		{
			for(INT ObjectIdx = 0; ObjectIdx < NumObjects; ObjectIdx++)
			{
				UObject* Object = InObjects(ObjectIdx);

				if(Object != NULL && Supports(Object) && BrowserType->Supports(Object))
				{
					Objects.AddUniqueItem(Object);
				}
			}
		}

		if( Objects.Num() > 0 && BrowserType->ShowObjectProperties(Objects) )
		{
			bShowedProperties = TRUE;
		}
	}

	return bShowedProperties;
}

/**
 * Invokes a custom menu item command for every selected object
 * of a supported class.
 *
 * @param InCommand		The command to execute
 */
void UGenericBrowserType_Custom::InvokeCustomCommand( INT InCommand )
{
	// Loop through all of the selected objects and see if any of our support classes support this object.
	// If one of them does, call the invoke function for the object and exit out.
	for ( USelection::TObjectIterator ObjectIt( GEditor->GetSelectedObjects()->ObjectItor() ) ; ObjectIt ; ++ObjectIt )
	{
		UObject* Object = *ObjectIt;

		if ( Supports(Object) )
		{
			for( INT SupportIdx = 0 ; SupportIdx < SupportInfo.Num() ; ++SupportIdx )
			{
				FGenericBrowserTypeInfo& Info = SupportInfo(SupportIdx);

				if( Info.Supports(Object) )
				{
					if(Info.BrowserType == NULL)
					{
						for( TObjectIterator<UGenericBrowserType> It ; It ; ++It )
						{
							UGenericBrowserType* BrowserType = *It;

							if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(Object) )
							{
								BrowserType->InvokeCustomCommand(InCommand, Object);
								break;
							}
						}
					}
					else
					{
						UGenericBrowserType* BrowserType = Info.BrowserType;

						if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(Object) )
						{
							BrowserType->InvokeCustomCommand(InCommand, Object);
						}
					}

					break;
				}
			}
		}
	}
}

/**
 * Calls the virtual "DoubleClick" function for each object
 * of a supported class.
 */
void UGenericBrowserType_Custom::DoubleClick()
{
	// Loop through all of the selected objects and see if any of our support classes support this object.
	// If one of them does, call the double click function for the object and continue on.
	for ( USelection::TObjectIterator ObjectIt( GEditor->GetSelectedObjects()->ObjectItor() ) ; ObjectIt ; ++ObjectIt )
	{
		UObject* Object = *ObjectIt;

		if ( Supports(Object) == TRUE )
		{
			for( INT SupportIdx = 0 ; SupportIdx < SupportInfo.Num() ; ++SupportIdx )
			{
				FGenericBrowserTypeInfo& Info = SupportInfo(SupportIdx);

				if( Info.Supports(Object) )
				{
					if(Info.BrowserType == NULL)
					{
						for( TObjectIterator<UGenericBrowserType> It ; It ; ++It )
						{
							UGenericBrowserType* BrowserType = *It;

							if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(Object) )
							{
								BrowserType->DoubleClick(Object);
								break;
							}
						}
					}
					else
					{
						UGenericBrowserType* BrowserType = Info.BrowserType;
						if( !BrowserType->IsA(UGenericBrowserType_Custom::StaticClass()) && BrowserType->Supports(Object) )
						{
							BrowserType->DoubleClick(Object);
						}
					}

					break;
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_Animation
------------------------------------------------------------------------------*/

void UGenericBrowserType_Animation::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UAnimSet::StaticClass(), FColor(192,128,255), new WxMBGenericBrowserContext_AnimSet , 0, this) );
}

UBOOL UGenericBrowserType_Animation::ShowObjectEditor( UObject* InObject )
{
	UAnimSet* AnimSet = Cast<UAnimSet>(InObject);

	if( !AnimSet )
	{
		return 0;
	}

	WxAnimSetViewer* AnimSetViewer = new WxAnimSetViewer( (wxWindow*)GApp->EditorFrame, -1, NULL, AnimSet, NULL );
	AnimSetViewer->Show(1);
	
	return 0;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_AnimTree
------------------------------------------------------------------------------*/

void UGenericBrowserType_AnimTree::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UAnimTree::StaticClass(), FColor(255,128,192), new WxMBGenericBrowserContext_AnimTree, 0, this ) );
}

UBOOL UGenericBrowserType_AnimTree::ShowObjectEditor( UObject* InObject )
{
	UAnimTree* AnimTree = Cast<UAnimTree>(InObject);
	if(AnimTree)
	{
		if(AnimTree->bBeingEdited)
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_AnimTreeAlreadyBeingEdited") );
		}
		else
		{
			WxAnimTreeEditor* AnimTreeEditor = new WxAnimTreeEditor( (wxWindow*)GApp->EditorFrame, -1, AnimTree );
			AnimTreeEditor->Show(1);

			return 1;
		}
	}

	
	return 0;
}

/*------------------------------------------------------------------------------
UGenericBrowserType_PostProcess
------------------------------------------------------------------------------*/

void UGenericBrowserType_PostProcess::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UPostProcessChain::StaticClass(), FColor(255,128,192), new WxMBGenericBrowserContext_PostProcess, 0, this ) );
}

UBOOL UGenericBrowserType_PostProcess::ShowObjectEditor( UObject* InObject )
{
	UPostProcessChain* PostProcess = Cast<UPostProcessChain>(InObject);
	if(PostProcess)
	{
		WxPostProcessEditor* PostProcessEditor = new WxPostProcessEditor( (wxWindow*)GApp->EditorFrame, -1, PostProcess );
		PostProcessEditor->Show(1);

		return 1;
	}


	return 0;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_LensFlare
------------------------------------------------------------------------------*/
    //## BEGIN PROPS GenericBrowserType_LensFlare
    //## END PROPS GenericBrowserType_LensFlare

void UGenericBrowserType_LensFlare::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( ULensFlare::StaticClass(), FColor(0,255,0), new WxMBGenericBrowserContext_LensFlare, 0, this ) );
}

UBOOL UGenericBrowserType_LensFlare::ShowObjectEditor( UObject* InObject )
{
	ULensFlare* LensFlare = Cast<ULensFlare>(InObject);
	if (LensFlare == NULL)
	{
		return FALSE;
	}

	check(GApp->EditorFrame);
	const wxWindowList& ChildWindows = GApp->EditorFrame->GetChildren();
    // Find targets in splitter window and send the event to them
    wxWindowList::compatibility_iterator node = ChildWindows.GetFirst();
    while (node)
    {
        wxWindow* child = (wxWindow*)node->GetData();
        if (child->IsKindOf(CLASSINFO(wxFrame)))
        {
			if (appStricmp(child->GetName(), *(LensFlare->GetPathName())) == 0)
			{
				debugf(TEXT("LensFlareEditor already open for %s"), *(LensFlare->GetPathName()));
				child->Show(1);
				child->Raise();
				//@todo. If minimized, we should restore it...
				return FALSE;
			}
        }
        node = node->GetNext();
    }
	WxLensFlareEditor* LensFlareEditor = new WxLensFlareEditor(GApp->EditorFrame, -1, LensFlare);
	check(LensFlareEditor);
	LensFlareEditor->Show(1);

	return TRUE;
}

/*------------------------------------------------------------------------------
	Helper functions for instancing a material editor.
------------------------------------------------------------------------------*/

namespace {

/**
 * Opens the specified material in the material editor.
 */
static void OpenMaterialInMaterialEditor(UMaterial* MaterialToEdit)
{
	if ( MaterialToEdit )
	{
		wxFrame* MaterialEditor = new WxMaterialEditor( (wxWindow*)GApp->EditorFrame,-1,MaterialToEdit );
		MaterialEditor->Show();
	}
}

} // namespace

/*------------------------------------------------------------------------------
	UGenericBrowserType_Material
------------------------------------------------------------------------------*/

void UGenericBrowserType_Material::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( 
		UMaterial::StaticClass(), 
		FColor(0,255,0), 
		new WxMBGenericBrowserContext_Material, 
		0, 
		this,
		UGenericBrowserType_Material::ShouldDisplayCallback )
	);
}

/**
* Callback to register whether or not the object should be displayed
* @param InObject - object that will be displayed in the GB
* @return TRUE if should be displayed
*/ 
UBOOL UGenericBrowserType_Material::ShouldDisplayCallback( UObject* InObject )
{
	UMaterial* InMaterial = Cast<UMaterial>(InObject);
	// Hide fallback materials to discourage direct references to them, since only their parent material should be referenced.
	return InMaterial && !InMaterial->bIsFallbackMaterial;
}

UBOOL UGenericBrowserType_Material::ShowObjectEditor( UObject* InObject )
{
	OpenMaterialInMaterialEditor( CastChecked<UMaterial>(InObject) );
	return TRUE;
}

static void CreateNewMaterialInstanceConstant(UObject* InObject)
{
	// Create a new material containing a texture material expression referring to the texture.
	const FString Package	= InObject->GetOutermost()->GetName();
	const FString Group		= InObject->GetOuter()->GetOuter() ? InObject->GetFullGroupName( 1 ) : TEXT("");
	const FString Name		= FString::Printf( TEXT("%s_INST"), *InObject->GetName() );

	WxDlgPackageGroupName dlg;
	dlg.SetTitle( *LocalizeUnrealEd("CreateNewMaterialInstanceConstant") );

	if( dlg.ShowModal( Package, Group, Name ) == wxID_OK )
	{
		FString Pkg;
		// Was a group specified?
		if( dlg.GetGroup().Len() > 0 )
		{
			Pkg = FString::Printf(TEXT("%s.%s"),*dlg.GetPackage(),*dlg.GetGroup());
		}
		else
		{
			Pkg = FString::Printf(TEXT("%s"),*dlg.GetPackage());
		}
		UObject* ExistingPackage = UObject::FindPackage(NULL, *Pkg);
		FString Reason;

		// Verify the package an object name.
		if(!dlg.GetPackage().Len() || !dlg.GetObjectName().Len())
		{
			appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
		}
// 		// Verify the object name.
// 		else if( !FIsValidObjectName( *dlg.GetObjectName(), Reason ))
// 		{
// 			appMsgf( AMT_OK, *Reason );
// 		}
		// Verify the object name is unique withing the package.
		else if (ExistingPackage && !FIsUniqueObjectName(*dlg.GetObjectName(), ExistingPackage, Reason))
		{
			appMsgf(AMT_OK, *Reason);
		}
		else
		{
			UMaterialInterface* InMaterialParent = Cast<UMaterialInterface>(InObject);
			check(InMaterialParent);

			UMaterialInstanceConstant* MaterialInterface = ConstructObject<UMaterialInstanceConstant>( UMaterialInstanceConstant::StaticClass(), UObject::CreatePackage(NULL,*Pkg), *dlg.GetObjectName(), RF_Public|RF_Transactional|RF_Standalone );
			check(MaterialInterface);

			MaterialInterface->SetParent(InMaterialParent);
			GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );

			// Show the editor with the new material instance.
			wxFrame* MaterialInstanceEditor = new WxMaterialInstanceConstantEditor( (wxWindow*)GApp->EditorFrame,-1, MaterialInterface );
			MaterialInstanceEditor->Show();
		}
	}
}


static void CreateNewMaterialInstanceTimeVarying(UObject* InObject)
{
	// Create a new material containing a texture material expression referring to the texture.
	const FString Package	= InObject->GetOutermost()->GetName();
	const FString Group		= InObject->GetOuter()->GetOuter() ? InObject->GetFullGroupName( 1 ) : TEXT("");
	const FString Name		= FString::Printf( TEXT("%s_INST"), *InObject->GetName() );

	WxDlgPackageGroupName dlg;
	dlg.SetTitle( *LocalizeUnrealEd("CreateNewMaterialInstanceTimeVarying") );

	if( dlg.ShowModal( Package, Group, Name ) == wxID_OK )
	{
		FString Pkg;
		// Was a group specified?
		if( dlg.GetGroup().Len() > 0 )
		{
			Pkg = FString::Printf(TEXT("%s.%s"),*dlg.GetPackage(),*dlg.GetGroup());
		}
		else
		{
			Pkg = FString::Printf(TEXT("%s"),*dlg.GetPackage());
		}
		UObject* ExistingPackage = UObject::FindPackage(NULL, *Pkg);
		FString Reason;

		// Verify the package an object name.
		if(!dlg.GetPackage().Len() || !dlg.GetObjectName().Len())
		{
			appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
		}
// 		// Verify the object name.
// 		else if( !FIsValidObjectName( *dlg.GetObjectName(), Reason ))
// 		{
// 			appMsgf( AMT_OK, *Reason );
// 		}
		// Verify the object name is unique withing the package.
		else if (ExistingPackage && !FIsUniqueObjectName(*dlg.GetObjectName(), ExistingPackage, Reason))
		{
			appMsgf(AMT_OK, *Reason);
		}
		else
		{
			UMaterialInterface* InMaterialParent = Cast<UMaterialInterface>(InObject);
			check(InMaterialParent);

			UMaterialInstanceTimeVarying* MaterialInterface = ConstructObject<UMaterialInstanceTimeVarying>( UMaterialInstanceTimeVarying::StaticClass(), UObject::CreatePackage(NULL,*Pkg), *dlg.GetObjectName(), RF_Public|RF_Transactional|RF_Standalone );
			check(MaterialInterface);

			MaterialInterface->SetParent(InMaterialParent);
			GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );

			// Show the editor with the new material instance.
			wxFrame* MaterialInstanceEditor = new WxMaterialInstanceTimeVaryingEditor( (wxWindow*)GApp->EditorFrame,-1, MaterialInterface );
			MaterialInstanceEditor->Show();
		}
	}
}


void UGenericBrowserType_Material::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if ( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceConstant )
	{
		CreateNewMaterialInstanceConstant( InObject );
	}
	else if ( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceTimeVarying )
	{
		CreateNewMaterialInstanceTimeVarying( InObject );
	}

}


/*------------------------------------------------------------------------------
  GenericBrowserType_MaterialLackingPhysMat
------------------------------------------------------------------------------*/
/** This will check the material to see if it has a physical material assigned or not. **/
UBOOL DoesNotHasPhysicalMaterial( UObject* Object )
{
	UMaterial* Material = Cast<UMaterial>(Object);
	if( Material != NULL )
	{
		if( Material->PhysMaterial == NULL )
		{
			return TRUE;
		}
	}

	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>( Object );
	if( MaterialInstance != NULL )
	{
		if( MaterialInstance->PhysMaterial == NULL )
		{
			return TRUE;
		}
	}

	return FALSE;
}


void UGenericBrowserType_MaterialLackingPhysMat::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UMaterialInterface::StaticClass(), FColor(128,128,200), NULL, 0, this, DoesNotHasPhysicalMaterial));
}


/*------------------------------------------------------------------------------
	UGenericBrowserType_DecalMaterial
------------------------------------------------------------------------------*/

void UGenericBrowserType_DecalMaterial::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UDecalMaterial::StaticClass(), FColor(192,255,192), new WxMBGenericBrowserContext_Material, 0, this ) );
}

UBOOL UGenericBrowserType_DecalMaterial::ShowObjectEditor( UObject* InObject )
{
	OpenMaterialInMaterialEditor( CastChecked<UDecalMaterial>(InObject) );
	return TRUE;
}

void UGenericBrowserType_DecalMaterial::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if ( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceConstant)
	{
		CreateNewMaterialInstanceConstant( InObject );
	}
	else if ( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceTimeVarying )
	{
		CreateNewMaterialInstanceTimeVarying( InObject );
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_MaterialInstance
------------------------------------------------------------------------------*/

void UGenericBrowserType_MaterialInstanceConstant::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UMaterialInstanceConstant::StaticClass(), FColor(0,128,0), new WxMBGenericBrowserContext_MaterialInstanceConstant, 0, this ) );
}

UBOOL UGenericBrowserType_MaterialInstanceConstant::ShowObjectEditor( UObject* InObject )
{
	// Show the material instance editor.
	UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(InObject);

	if(MaterialInterface)
	{
		wxFrame* MaterialInstanceEditor = new WxMaterialInstanceConstantEditor( (wxWindow*)GApp->EditorFrame,-1, MaterialInterface );
		MaterialInstanceEditor->Show();
	}

	return TRUE;
}

void UGenericBrowserType_MaterialInstanceConstant::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceConstant )
	{
		CreateNewMaterialInstanceConstant( InObject );
	}
	else if ( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceTimeVarying )
	{
		CreateNewMaterialInstanceTimeVarying( InObject );
	}
}


/*------------------------------------------------------------------------------
	UGenericBrowserType_MaterialInstance
------------------------------------------------------------------------------*/

void UGenericBrowserType_MaterialInstanceTimeVarying::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UMaterialInstanceTimeVarying::StaticClass(), FColor(0,128,0), new WxMBGenericBrowserContext_MaterialInstanceTimeVarying, 0, this ) );
}

UBOOL UGenericBrowserType_MaterialInstanceTimeVarying::ShowObjectEditor( UObject* InObject )
{
	// Show the material instance editor.
	UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(InObject);

	if(MaterialInterface)
	{
		wxFrame* MaterialInstanceEditor = new WxMaterialInstanceTimeVaryingEditor( (wxWindow*)GApp->EditorFrame,-1, MaterialInterface );
		MaterialInstanceEditor->Show();
	}

	return TRUE;
}

void UGenericBrowserType_MaterialInstanceTimeVarying::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if( InCommand == IDMN_ObjectContext_CreateNewMaterialInstanceTimeVarying )
	{
		CreateNewMaterialInstanceTimeVarying( InObject );
	}
}


/*------------------------------------------------------------------------------
	UGenericBrowserType_ParticleSystem
------------------------------------------------------------------------------*/

void UGenericBrowserType_ParticleSystem::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UParticleSystem::StaticClass(), FColor(255,255,255), new WxMBGenericBrowserContext_ParticleSystem, 0, this ) );
}

UBOOL UGenericBrowserType_ParticleSystem::ShowObjectEditor( UObject* InObject )
{
	UParticleSystem* PSys = Cast<UParticleSystem>(InObject);
	if (PSys == NULL)
	{
		return FALSE;
	}

	check(GApp->EditorFrame);
	const wxWindowList& ChildWindows = GApp->EditorFrame->GetChildren();
    // Find targets in splitter window and send the event to them
    wxWindowList::compatibility_iterator node = ChildWindows.GetFirst();
    while (node)
    {
        wxWindow* child = (wxWindow*)node->GetData();
        if (child->IsKindOf(CLASSINFO(wxFrame)))
        {
			if (appStricmp(child->GetName(), *(PSys->GetPathName())) == 0)
			{
				debugf(TEXT("Cascade already open for PSys %s"), *(PSys->GetPathName()));
				child->Show(1);
				child->Raise();
				//@todo. If minimized, we should restore it...
				return FALSE;
			}
        }
        node = node->GetNext();
    }
	WxCascade* ParticleEditor = new WxCascade(GApp->EditorFrame, -1, PSys);
	ParticleEditor->Show(1);

	return TRUE;
}

void UGenericBrowserType_ParticleSystem::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if (InCommand == IDMN_ObjectContext_CopyParticleParameters)
	{
		UParticleSystem* PSys = Cast<UParticleSystem>(InObject);
		if (PSys)
		{
			debugf(TEXT("Copy particle parameters - %s"), *(PSys->GetPathName()));
			TArray<TArray<FString>> ParticleSysParamList;
			TArray<TArray<FString>> ParticleParameterList;
			PSys->GetParametersUtilized(ParticleSysParamList, ParticleParameterList);

			FString ClipboardString;
			ClipboardString = FString::Printf(TEXT("ParticleSystem parameters for %s\n"), *(PSys->GetPathName()));
			for (INT EmitterIndex = 0; EmitterIndex < PSys->Emitters.Num(); EmitterIndex++)
			{
				ClipboardString += FString::Printf(TEXT("\tEmitter %2d - "), EmitterIndex);
				UParticleEmitter* Emitter = PSys->Emitters(EmitterIndex);
				if (Emitter)
				{
					ClipboardString += FString::Printf(TEXT("%s\n"), *(Emitter->GetEmitterName().ToString()));
				}
				else
				{
					ClipboardString += FString(TEXT("* EMPTY *\n"));
				}

				TArray<FString>& ParticleSysParams = ParticleSysParamList(EmitterIndex);
				for (INT PSPIndex = 0; PSPIndex < ParticleSysParams.Num(); PSPIndex++)
				{
					if (PSPIndex == 0)
					{
						ClipboardString += FString(TEXT("\t\tParticleSysParam List\n"));
					}
					ClipboardString += FString::Printf(TEXT("\t\t\t%s"), *(ParticleSysParams(PSPIndex)));
				}

				TArray<FString>& ParticleParameters = ParticleParameterList(EmitterIndex);
				for (INT PPIndex = 0; PPIndex < ParticleParameters.Num(); PPIndex++)
				{
					if (PPIndex == 0)
					{
						ClipboardString += FString(TEXT("\t\tParticleParameter List\n"));
					}
					ClipboardString += FString::Printf(TEXT("\t\t\t%s"), *(ParticleParameters(PPIndex)));
				}
			}

			appClipboardCopy(*ClipboardString);
		}
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_PhysXParticleSystem
------------------------------------------------------------------------------*/
void UGenericBrowserType_PhysXParticleSystem::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UPhysXParticleSystem::StaticClass(), FColor(200,192,128), new WxMBGenericBrowserContext_PhysXParticleSystem, 0, this ) );
}

UBOOL UGenericBrowserType_PhysXParticleSystem::ShowObjectEditor( UObject* InObject )
{
	WxPropertyWindowFrame* Properties = new WxPropertyWindowFrame;
	Properties->Create( GApp->EditorFrame, -1, 0 );

	Properties->AllowClose();
	Properties->SetObject( InObject, 1,1,1 );
	Properties->SetTitle( *FString::Printf( TEXT("PhysXParticleSystem") ) );
	Properties->Show();

	return 1;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_PhysicsAsset
------------------------------------------------------------------------------*/

void UGenericBrowserType_PhysicsAsset::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UPhysicsAsset::StaticClass(), FColor(255,192,128), new WxMBGenericBrowserContext_PhysicsAsset, 0, this ) );
}

UBOOL UGenericBrowserType_PhysicsAsset::ShowObjectEditor( UObject* InObject )
{
	WxPhAT* AssetEditor = new WxPhAT(GApp->EditorFrame, -1, CastChecked<UPhysicsAsset>(InObject));
	AssetEditor->Show(1);

	return 1;
}

/*------------------------------------------------------------------------------
UGenericBrowserType_PhysXDestructible
------------------------------------------------------------------------------*/
void UGenericBrowserType_PhysXDestructible::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UPhysXDestructible::StaticClass(), FColor(255,192,128), 0, 0, this ) );
}

UBOOL UGenericBrowserType_PhysXDestructible::ShowObjectEditor( UObject* InObject )
{
	class FPhysXDestructiblePropertyNotify : public FNotifyHook
	{
	public:
		FPhysXDestructiblePropertyNotify( UPhysXDestructible * InPhysXDestructible ) :
			PhysXDestructible( InPhysXDestructible ) {}

		UPhysXDestructible * PhysXDestructible;

		void NotifyDestroy( void* Src )
		{
			if( PhysXDestructible )
			{
				PhysXDestructible->ApplyCookingScalesToAssets();
			}
		}
	};

	WxPropertyWindowFrame* Properties = new WxPropertyWindowFrame;
	Properties->Create( GApp->EditorFrame, -1, 0, new FPhysXDestructiblePropertyNotify( Cast<UPhysXDestructible>( InObject ) ) );

	Properties->AllowClose();
	Properties->SetObject( InObject, 1,1,1 );
	Properties->Show();

	return 1;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_Sequence
------------------------------------------------------------------------------*/

void UGenericBrowserType_Sequence::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USequence::StaticClass(), FColor(255,255,255), NULL, 0, this, UGenericBrowserType_Sequence::IsSequenceTypeSupported ) );
}

/**
 * Determines whether the specified object is a USequence class that should be handled by this generic browser type.
 *
 * @param	Object	a pointer to a USequence object.
 *
 * @return	TRUE if this generic browser type supports to object specified.
 */
UBOOL UGenericBrowserType_Sequence::IsSequenceTypeSupported( UObject* Object )
{
	USequence* SequenceObj = Cast<USequence>(Object);

	if ( SequenceObj != NULL )
	{
		// we don't support exporting/importing UI Sequences, so disallow these guys
		// UISequences have the RF_Public flag (and thus would otherwise be displayed in the generic browser)
		// if they are contained within a UIPrefab
		if ( SequenceObj->IsA(UUISequence::StaticClass()) )
		{
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_SkeletalMesh
------------------------------------------------------------------------------*/

void UGenericBrowserType_SkeletalMesh::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USkeletalMesh::StaticClass(), FColor(255,0,255), new WxMBGenericBrowserContext_Skeletal, 0, this ) );
}

UBOOL UGenericBrowserType_SkeletalMesh::ShowObjectEditor( UObject* InObject )
{
	WxAnimSetViewer* AnimSetViewer = new WxAnimSetViewer( (wxWindow*)GApp->EditorFrame, -1, CastChecked<USkeletalMesh>(InObject), NULL, NULL );
	AnimSetViewer->Show();

	return 1;
}

void UGenericBrowserType_SkeletalMesh::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if( InCommand == IDMN_ObjectContext_CreateNewPhysicsAsset )
	{
		USkeletalMesh* SkelMesh = Cast<USkeletalMesh>(InObject);
		if( SkelMesh )
		{
			// Create a new physics asset from the selected skeletal mesh. Defaults to being in the same package/group as the skeletal mesh.

			FString DefaultPackage = SkelMesh->GetOutermost()->GetName();
			FString DefaultGroup = SkelMesh->GetOuter()->GetOuter() ? SkelMesh->GetFullGroupName( TRUE ) : TEXT("");

			// Make default name by appending '_Physics' to the SkeletalMesh name.
			FString DefaultAssetName = FString( FString::Printf( TEXT("%s_Physics"), *SkelMesh->GetName() ) );

			// First of all show the dialog for choosing a new package file, group and asset name:
			WxDlgPackageGroupName PackageDlg;
			if( PackageDlg.ShowModal(DefaultPackage, DefaultGroup, DefaultAssetName) == wxID_OK )
			{
				// Get the full name of where we want to create the physics asset.
				FString PackageName;
				if(PackageDlg.GetGroup().Len() > 0)
				{
					PackageName = PackageDlg.GetPackage() + TEXT(".") + PackageDlg.GetGroup();
				}
				else
				{
					PackageName = PackageDlg.GetPackage();
				}

				// Then find/create it.
				UPackage* Package = UObject::CreatePackage(NULL, *PackageName);
				check(Package);

				// Now show the 'asset creation' options dialog
				WxDlgNewPhysicsAsset AssetDlg;
				if( AssetDlg.ShowModal( SkelMesh, false ) == wxID_OK )
				{			
					UPhysicsAsset* NewAsset = ConstructObject<UPhysicsAsset>( UPhysicsAsset::StaticClass(), Package, *PackageDlg.GetObjectName(), RF_Public|RF_Standalone|RF_Transactional );
					if(NewAsset)
					{
						// Do automatic asset generation.
						UBOOL bSuccess = NewAsset->CreateFromSkeletalMesh( SkelMesh, AssetDlg.Params );
						if(bSuccess)
						{
							if(AssetDlg.bOpenAssetNow)
							{
								WxPhAT* AssetEditor = new WxPhAT(GApp->EditorFrame, -1, NewAsset);
								AssetEditor->SetSize(256,256,1024,768);
								AssetEditor->Show(1);
							}
						}
						else
						{
							appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_FailedToCreatePhysAssetFromSM"), *PackageDlg.GetObjectName(), *SkelMesh->GetName()) );
							NewAsset->ClearFlags( RF_Public| RF_Standalone );
						}
					}
					else
					{
						appMsgf( AMT_OK, *LocalizeUnrealEd("Error_FailedToCreateNewPhysAsset") );
					}
				}
			}
		}
	}
	else if( InCommand == IDMN_ObjectContext_CreateNewFaceFXAsset )
	{
		USkeletalMesh* SkelMesh = Cast<USkeletalMesh>(InObject);
		if( SkelMesh )
		{
			if( SkelMesh->FaceFXAsset )
			{
				//@todo Localize this string.
				wxMessageBox(wxString(TEXT("That Skeletal Mesh already has a valid FaceFX Asset!  Please remove the existing one first.")), wxString(TEXT("FaceFX")));
			}
			else
			{
				// Create a new FaceFX asset from the selected skeletal mesh. 
				// Defaults to being in the same package/group as the skeletal mesh.
				FString DefaultPackage = SkelMesh->GetOutermost()->GetName();
				FString DefaultGroup = SkelMesh->GetOuter()->GetOuter() ? SkelMesh->GetFullGroupName(TRUE) : TEXT("");

				// Make default name by appending '_FaceFX' to the SkeletalMesh name.
				FString DefaultAssetName = FString(FString::Printf( TEXT("%s_FaceFX"), *SkelMesh->GetName()));

				// First of all show the dialog for choosing a new package file, group and asset name:
				WxDlgPackageGroupName PackageDlg;
				if( PackageDlg.ShowModal(DefaultPackage, DefaultGroup, DefaultAssetName) == wxID_OK )
				{
					// Get the full name of where we want to create the FaceFX asset.
					FString PackageName;
					if( PackageDlg.GetGroup().Len() > 0 )
					{
						PackageName = PackageDlg.GetPackage() + TEXT(".") + PackageDlg.GetGroup();
					}
					else
					{
						PackageName = PackageDlg.GetPackage();
					}

					// Then find/create it.
					UPackage* Package = UObject::CreatePackage(NULL, *PackageName);
					check(Package);

					FString ExistingAssetPath = PackageName;
					ExistingAssetPath += ".";
					ExistingAssetPath += *PackageDlg.GetObjectName();
					UFaceFXAsset* ExistingAsset = LoadObject<UFaceFXAsset>(NULL, *ExistingAssetPath, NULL, LOAD_NoWarn, NULL);
					if( ExistingAsset )
					{
						//@todo Localize this string.
						wxMessageBox(wxString(TEXT("That FaceFXAsset already exists!  Please remove the existing one first.")), wxString(TEXT("FaceFX")));
					}
					else
					{
						UFaceFXAsset* NewAsset = ConstructObject<UFaceFXAsset>(UFaceFXAsset::StaticClass(), Package, *PackageDlg.GetObjectName(), RF_Public|RF_Standalone|RF_Transactional);
						if( NewAsset )
						{
							NewAsset->CreateFxActor(SkelMesh);
							NewAsset->MarkPackageDirty();
							GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
						}
					}
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_Sounds
------------------------------------------------------------------------------*/

void UGenericBrowserType_Sounds::Init( void )
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USoundCue::StaticClass(), FColor( 0, 175, 255 ), new WxMBGenericBrowserContext_SoundCue, 0, this ) );
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USoundNodeWave::StaticClass(), FColor( 0, 0, 255 ), new WxMBGenericBrowserContext_Sound, 0, this ) );
}

UBOOL UGenericBrowserType_Sounds::ShowObjectEditor( UObject* InObject )
{
	USoundCue*	SoundCue = Cast<USoundCue>( InObject );
	USoundNodeWave* SoundNodeWave = Cast<USoundNodeWave>( InObject );
	if( SoundCue )
	{
		WxSoundCueEditor* SoundCueEditor = new WxSoundCueEditor( GApp->EditorFrame, -1, CastChecked<USoundCue>( InObject ) );
		SoundCueEditor->InitEditor();
		SoundCueEditor->Show( true );
	}
	else if( SoundNodeWave )
	{
		if( !SoundNodeWave->bUseTTS )
		{
			// Create an instance of the Wave Node options dialog.
			WxDlgSoundNodeWaveOptions* DlgSoundNodeWave = new WxDlgSoundNodeWaveOptions( GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT( "GenericBrowser" ) ), SoundNodeWave );
		}
		else
		{
			appMsgf( AMT_OK, TEXT( "There are no available quality settings for TextToSpeech (TTS)." ) );
		}
	}

	return TRUE;
}

void UGenericBrowserType_Sounds::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	USoundCue* SoundCue = Cast<USoundCue>( InObject );
	USoundNodeWave* SoundNodeWave = Cast<USoundNodeWave>( InObject );

	if( InCommand == IDMN_ObjectContext_Sound_Play )
	{
		if( SoundCue )
		{
			Play( SoundCue );
		}
		else if( SoundNodeWave )
		{
			Play( SoundNodeWave );
		}
	}
	else if( InCommand == IDMN_ObjectContext_Sound_Stop )
	{
		Stop();
	}
	else if( SoundCue && InCommand >= IDMN_ObjectContext_SoundCue_SoundGroups_START && InCommand < IDMN_ObjectContext_SoundCue_SoundGroups_END )
	{
		INT		SoundClassIndex	= InCommand - IDMN_ObjectContext_SoundCue_SoundGroups_START;
		FName	SoundClassName	= NAME_None;

		// Retrieve group name from index.
		UEnum* SoundClassNames = FindObject<UEnum>( NULL, TEXT( "Engine.AudioDevice.ESoundClassName" ) );
		if( SoundClassNames )
		{
			SoundClassName = SoundClassNames->GetEnum( SoundClassIndex );
		}

		// Refresh the main display
		GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
	}
}

void UGenericBrowserType_Sounds::DoubleClick( UObject* InObject )
{
	USoundCue*	SoundCue = Cast<USoundCue>( InObject );
	USoundNode* SoundNodeWave = Cast<USoundNodeWave>( InObject );
	if( SoundCue )
	{
		Play( SoundCue );
	}
	else if( SoundNodeWave )
	{
		Play( SoundNodeWave );
	}
}

void UGenericBrowserType_Sounds::Play( USoundNode* InSound )
{
	UAudioComponent* AudioComponent = GEditor->GetPreviewAudioComponent( NULL, InSound );
	if( AudioComponent )
	{
		AudioComponent->Stop();

		// Create the ogg vorbis or TTS data for this file if it doesn't exist
		if( AudioComponent->SoundCue->ValidateData() )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
		}

		AudioComponent->bUseOwnerLocation = FALSE;
		AudioComponent->bAutoDestroy = FALSE;
		AudioComponent->Location = FVector( 0.0f, 0.0f, 0.0f );
		AudioComponent->bIsUISound = TRUE;
		AudioComponent->bAllowSpatialization = FALSE;
		AudioComponent->bNoReverb = TRUE;

		AudioComponent->Play();	
	}
}

void UGenericBrowserType_Sounds::Play( USoundCue* InSound )
{
	UAudioComponent* AudioComponent = GEditor->GetPreviewAudioComponent( InSound, NULL );
	if( AudioComponent )
	{
		AudioComponent->Stop();

		// Create the ogg vorbis data for this file if it doesn't exist
		if( AudioComponent->SoundCue->ValidateData() )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
		}

		AudioComponent->SoundCue = InSound;
		AudioComponent->bUseOwnerLocation = FALSE;
		AudioComponent->bAutoDestroy = FALSE;
		AudioComponent->Location = FVector( 0.0f, 0.0f, 0.0f );
		AudioComponent->bIsUISound = TRUE;
		AudioComponent->bAllowSpatialization	= FALSE;
		AudioComponent->bNoReverb = TRUE;

		AudioComponent->Play();	
	}
}

void UGenericBrowserType_Sounds::Stop( void )
{
	UAudioComponent* AudioComponent = GEditor->GetPreviewAudioComponent( NULL, NULL );
	if( AudioComponent )
	{
		AudioComponent->Stop();
		AudioComponent->SoundCue = NULL;
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_SoundCue
------------------------------------------------------------------------------*/

void UGenericBrowserType_SoundCue::Init( void )
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USoundCue::StaticClass(), FColor( 0, 175, 255 ), new WxMBGenericBrowserContext_SoundCue, 0, this ) );
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_SoundMode
------------------------------------------------------------------------------*/

void UGenericBrowserType_SoundMode::Init( void )
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USoundMode::StaticClass(), FColor( 175, 0, 255 ), new WxMBGenericBrowserContext_SoundMode, 0, this ) );
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_SoundWave
------------------------------------------------------------------------------*/

void UGenericBrowserType_SoundWave::Init( void )
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USoundNodeWave::StaticClass(), FColor( 0, 0, 255 ), new WxMBGenericBrowserContext_Sound, 0, this ) );
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_SpeechRecognition
------------------------------------------------------------------------------*/

void UGenericBrowserType_SpeechRecognition::Init( void )
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( USpeechRecognition::StaticClass(), FColor( 0, 0, 255 ), new WxMBGenericBrowserContext_SpeechRecognition, 0, this ) );
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_StaticMesh
------------------------------------------------------------------------------*/

void UGenericBrowserType_StaticMesh::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UFracturedStaticMesh::StaticClass(), FColor(96,200,255), new WxMBGenericBrowserContext_FracturedStaticMesh, 0, this ) );
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UStaticMesh::StaticClass(), FColor(0,255,255), new WxMBGenericBrowserContext_StaticMesh, 0, this ) );
}

UBOOL UGenericBrowserType_StaticMesh::ShowObjectEditor( UObject* InObject )
{
	WxStaticMeshEditor* StaticMeshEditor = new WxStaticMeshEditor( GApp->EditorFrame, -1, CastChecked<UStaticMesh>(InObject), FALSE );
	StaticMeshEditor->Show(1);

	return 1;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_FracuredStaticMesh
------------------------------------------------------------------------------*/

void UGenericBrowserType_FracturedStaticMesh::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UFracturedStaticMesh::StaticClass(), FColor(96,200,255), new WxMBGenericBrowserContext_FracturedStaticMesh, 0, this ) );
}

void UGenericBrowserType_FracturedStaticMesh::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if( InCommand == IDMN_ObjectContext_CreateNewPhysXDestructible )
	{
		// Create a PhysXDestructible from the selected FracturedStaticMesh. 
		// Defaults to being in the same package/group as the static mesh.

		const FString Package	= InObject->GetOutermost()->GetName();
		const FString Group		= InObject->GetOuter()->GetOuter() ? InObject->GetFullGroupName( 1 ) : TEXT("");
		const FString Name		= FString::Printf( TEXT("%s_PHYSX"), *InObject->GetName() );

		WxDlgPackageGroupName dlg;
		dlg.SetTitle( *LocalizeUnrealEd("CreateNewPhysXDestructible") );

		if( dlg.ShowModal( Package, Group, Name ) == wxID_OK )
		{
			FString Pkg;
			// Was a group specified?
			if( dlg.GetGroup().Len() > 0 )
			{
				Pkg = FString::Printf(TEXT("%s.%s"),*dlg.GetPackage(),*dlg.GetGroup());
			}
			else
			{
				Pkg = FString::Printf(TEXT("%s"),*dlg.GetPackage());
			}
			UPackage* Package = UObject::CreatePackage(NULL, *Pkg);
			FString Reason;

			// Verify the package and object name.
			if(!dlg.GetPackage().Len() || !dlg.GetObjectName().Len())
			{
				appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
			}
			// Verify the object name.
			else if( !FIsValidObjectName( *dlg.GetObjectName(), Reason ))
			{
				appMsgf( AMT_OK, *Reason );
			}
			// Verify the object name is unique withing the package.
			else if (Package && !FIsUniqueObjectName(*dlg.GetObjectName(), Package, Reason))
			{
				appMsgf(AMT_OK, *Reason);
			}
			else
			{
				UFracturedStaticMesh* FracturedStaticMesh = CastChecked<UFracturedStaticMesh>(InObject);
				WxDlgNewPhysXDestructible* dlg2 = new WxDlgNewPhysXDestructible();
				dlg2->Show(FracturedStaticMesh, Package, *dlg.GetObjectName());
			}
		}
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_TerrainLayer
------------------------------------------------------------------------------*/

void UGenericBrowserType_TerrainLayer::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UTerrainLayerSetup::StaticClass(), FColor(128,192,255), NULL, 0, this ) );
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_TerrainMaterial
------------------------------------------------------------------------------*/

void UGenericBrowserType_TerrainMaterial::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UTerrainMaterial::StaticClass(), FColor(192,255,192), NULL, 0, this ) );
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_TextureWithAlpha
------------------------------------------------------------------------------*/

void UGenericBrowserType_TextureWithAlpha::Init()
{	
	SupportInfo.AddItem( FGenericBrowserTypeInfo( 
							UTexture::StaticClass(), 
							FColor(255,0,0), 
							new WxMBGenericBrowserContext_Texture, 
							0,
							this,
							UGenericBrowserType_TextureWithAlpha::IsTextureWithAlpha) );	
}

/**
 * Returns TRUE if passed in UObject is a texture using an alpha channel.
 *
 * @param	Object	Object to check whether it's a texture utilizing an alpha channel
 * @return TRUE if passed in UObject is a texture using an alpha channel, FALSE otherwise
 */
UBOOL UGenericBrowserType_TextureWithAlpha::IsTextureWithAlpha( UObject* Object )
{
	UTexture2D* Texture2D = Cast<UTexture2D>(Object);
	if( Texture2D && 
		(Texture2D->Format == PF_DXT3 || Texture2D->Format == PF_DXT5) &&
		UGenericBrowserType_Texture::ShouldDisplayCallback(Object) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_Texture
------------------------------------------------------------------------------*/

void UGenericBrowserType_Texture::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( 
		UTexture::StaticClass(), 
		FColor(255,0,0), 
		new WxMBGenericBrowserContext_Texture, 
		0, 
		this,
		UGenericBrowserType_Texture::ShouldDisplayCallback) );	
}

/**
* Callback to register whether or not the object should be displayed
* @param InObject - object that will be displayed in the GB
* @return TRUE if should be displayed
*/ 
UBOOL UGenericBrowserType_Texture::ShouldDisplayCallback( UObject* InObject )
{
	// don't display texture if its outer is a UFont
	UBOOL bResult = !(Cast<UTexture>(InObject) && Cast<UFont>(InObject->GetOuter()));
	return bResult;
}

UBOOL UGenericBrowserType_Texture::ShowObjectEditor( UObject* InObject )
{
	WxDlgTextureProperties* dlg = new WxDlgTextureProperties();
	dlg->Show( 1, Cast<UTexture>(InObject) );

	return 1;
}

void UGenericBrowserType_Texture::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if( InCommand == IDMN_ObjectContext_Reimport )
	{
		FReimportManager::Instance()->Reimport(InObject);
	}
	else if( InCommand == IDMN_ObjectContext_CreateNewMaterial )
	{
		// Create a new material containing a texture material expression referring to the texture.
		const FString Package	= InObject->GetOutermost()->GetName();
		const FString Group		= InObject->GetOuter()->GetOuter() ? InObject->GetFullGroupName( 1 ) : TEXT("");
		const FString Name		= FString::Printf( TEXT("%s_Mat"), *InObject->GetName() );

		WxDlgPackageGroupName dlg;
		dlg.SetTitle( *LocalizeUnrealEd("CreateNewMaterial") );

		if( dlg.ShowModal( Package, Group, Name ) == wxID_OK )
		{
			FString Pkg;
			// Was a group specified?
			if( dlg.GetGroup().Len() > 0 )
			{
				Pkg = FString::Printf(TEXT("%s.%s"),*dlg.GetPackage(),*dlg.GetGroup());
			}
			else
			{
				Pkg = FString::Printf(TEXT("%s"),*dlg.GetPackage());
			}
			UObject* ExistingPackage = UObject::FindPackage(NULL, *Pkg);
			FString Reason;

			// Verify the package an object name.
			if(!dlg.GetPackage().Len() || !dlg.GetObjectName().Len())
			{
				appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
			}
// 			// Verify the object name.
// 			else if( !FIsValidObjectName( *dlg.GetObjectName(), Reason ))
// 			{
// 				appMsgf( AMT_OK, *Reason );
// 			}
			// Verify the object name is unique withing the package.
			else if (ExistingPackage && !FIsUniqueObjectName(*dlg.GetObjectName(), ExistingPackage, Reason))
			{
				appMsgf(AMT_OK, *Reason);
			}
			else
			{
				UMaterialFactoryNew* Factory = new UMaterialFactoryNew;
				UMaterial* Material = (UMaterial*)Factory->FactoryCreateNew( UMaterial::StaticClass(), UObject::CreatePackage(NULL,*Pkg), *dlg.GetObjectName(), RF_Public|RF_Transactional|RF_Standalone, NULL, GWarn );
				check( Material );
				UMaterialExpressionTextureSample* Expression = ConstructObject<UMaterialExpressionTextureSample>( UMaterialExpressionTextureSample::StaticClass(), Material );
				Material->Expressions.AddItem( Expression );
				Expression->Texture = CastChecked<UTexture>( InObject );
				Expression->PostEditChange( NULL );
				GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
			}
		}
	}
}

/*------------------------------------------------------------------------------
UGenericBrowserType_RenderTexture
------------------------------------------------------------------------------*/

void UGenericBrowserType_RenderTexture::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UTextureRenderTarget2D::StaticClass(), FColor(255,0,0), new WxMBGenericBrowserContext_RenderTexture, 0, this ) );
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UTextureRenderTargetCube::StaticClass(), FColor(255,0,0), new WxMBGenericBrowserContext_RenderTexture, 0, this ) );		
}

static void Handle_RTCreateStaticTexture( UObject* InObject )
{
	if( !InObject &&
		!(InObject->IsA(UTextureRenderTarget2D::StaticClass()) || InObject->IsA(UTextureRenderTargetCube::StaticClass())) )
	{
		return;
	}

	// create dialog for naming the new object and its package
	WxDlgPackageGroupName dlg;
	dlg.SetTitle( *LocalizeUnrealEd("CreateStaticTextureE") );
	FString Package = InObject->GetOutermost()->GetName();
	FString Group = InObject->GetOuter()->GetOuter() ? InObject->GetFullGroupName( 1 ) : TEXT("");
	FString Name = FString::Printf( TEXT("%s_Tex"), *InObject->GetName() );

	// show the dialog
	if( dlg.ShowModal( Package, Group, Name ) == wxID_OK )
	{
		FString Pkg,Reason;
		// parse the package and group name
		if( dlg.GetGroup().Len() > 0 )
		{
			Pkg = FString::Printf(TEXT("%s.%s"),*dlg.GetPackage(),*dlg.GetGroup());
		}
		else
		{
			Pkg = FString::Printf(TEXT("%s"),*dlg.GetPackage());
		}
		// check for an exsiting package
		UObject* ExistingPackage = UObject::FindPackage(NULL, *Pkg);

		// make sure package and object names were specified
		if(!dlg.GetPackage().Len() || !dlg.GetObjectName().Len())
		{
			appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
		}
// 		// check for a valid object name for the new static texture
// 		else if( !FIsValidObjectName( *dlg.GetObjectName(), Reason ))
// 		{
// 			appMsgf( AMT_OK, *Reason );
// 		}
		// check for a duplicate object
		else if (ExistingPackage && !FIsUniqueObjectName(*dlg.GetObjectName(), ExistingPackage, Reason))
		{
			appMsgf(AMT_OK, *Reason);
		}
		else
		{
			UObject* NewObj = NULL;
			UTextureRenderTarget2D* TexRT = Cast<UTextureRenderTarget2D>(InObject);
			UTextureRenderTargetCube* TexRTCube = Cast<UTextureRenderTargetCube>(InObject);
			if( TexRTCube )
			{
				// create a static cube texture as well as its 6 faces
				NewObj = TexRTCube->ConstructTextureCube( UObject::CreatePackage(NULL,*Pkg), dlg.GetObjectName(), InObject->GetMaskedFlags(~0) );
			}
			else if( TexRT )
			{
				// create a static 2d texture
				NewObj = TexRT->ConstructTexture2D( UObject::CreatePackage(NULL,*Pkg), dlg.GetObjectName(), InObject->GetMaskedFlags(~0) );
			}

			if( NewObj )
			{
				// package needs saving
				NewObj->MarkPackageDirty();
				// refresh GB
				GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
			}
		}
	}
}

void UGenericBrowserType_RenderTexture::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	// create a static texture from a dynamic render target texture
	if( InCommand == IDMN_ObjectContext_RTCreateStaticTexture )
	{
		Handle_RTCreateStaticTexture(InObject);        
	}
}


/*------------------------------------------------------------------------------
	UGenericBrowserType_Font
------------------------------------------------------------------------------*/

#include "FontPropertiesDlg.h"

/**
 * Adds the font information to the support info
 */
void UGenericBrowserType_Font::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UFont::StaticClass(), FColor(255,0,0), NULL, 0, this ) );
}

/**
 * Displays the font properties window for editing & importing/exporting of
 * font pages
 *
 * @param InObject the object being edited
 */
UBOOL UGenericBrowserType_Font::ShowObjectEditor(UObject* InObject)
{
	// Cast to the font so we have access to the texture data
	UFont* Font = Cast<UFont>(InObject);
	if (Font != NULL)
	{
		WxFontPropertiesDlg* dlg = new WxFontPropertiesDlg();
		dlg->Show(1,Font);
	}
	return 1;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_PhysicalMaterial
------------------------------------------------------------------------------*/

void UGenericBrowserType_PhysicalMaterial::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UPhysicalMaterial::StaticClass(), FColor(200,192,128), NULL, 0, this ) );
}

UBOOL UGenericBrowserType_PhysicalMaterial::ShowObjectEditor( UObject* InObject )
{
	WxPropertyWindowFrame* Properties = new WxPropertyWindowFrame;
	Properties->Create( GApp->EditorFrame, -1, 0 );

	Properties->AllowClose();
	Properties->SetObject( InObject, 1,1,1 );
	Properties->SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("PhysicalMaterial_F")), *InObject->GetPathName()) ) );
	Properties->Show();

	return 1;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_Archetype
------------------------------------------------------------------------------*/
void UGenericBrowserType_Archetype::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UObject::StaticClass(), FColor(255,0,0), new WxMBGenericBrowserContext_Archetype, RF_ArchetypeObject, this, UGenericBrowserType_Archetype::IsArchetypeSupported) );
}

/**
 * Determines whether the specified object is an archetype that should be handled by this generic browser type.
 *
 * @param	Object	a pointer to a object with the RF_ArchetypeObject flag
 *
 * @return	TRUE if this generic browser type supports to object specified.
 */
UBOOL UGenericBrowserType_Archetype::IsArchetypeSupported( UObject* Object )
{
	return Object != NULL && Object->HasAllFlags(RF_ArchetypeObject) && !Object->IsAPrefabArchetype();
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_Prefab
------------------------------------------------------------------------------*/
void UGenericBrowserType_Prefab::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UPrefab::StaticClass(), FColor(255,128,128), NULL, 0, this ) );
}

UBOOL UGenericBrowserType_Prefab::ShowObjectEditor( UObject* InObject )
{
	// Do nothing...
	return true;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_MorphTargetSet
------------------------------------------------------------------------------*/

void UGenericBrowserType_MorphTargetSet::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UMorphTargetSet::StaticClass(), FColor(192,128,0), new WxMBGenericBrowserContext_MorphTargetSet, 0, this ) );
}

UBOOL UGenericBrowserType_MorphTargetSet::ShowObjectEditor( UObject* InObject )
{
	// open AnimSetViewer

	UMorphTargetSet* MorphSet = Cast<UMorphTargetSet>(InObject);
	if( !MorphSet )
	{
		return 0;
	}

	WxAnimSetViewer* AnimSetViewer = new WxAnimSetViewer( (wxWindow*)GApp->EditorFrame, -1, NULL, NULL, MorphSet );
	AnimSetViewer->Show(1);

	return 1;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_MorphWeightSequence
------------------------------------------------------------------------------*/

void UGenericBrowserType_MorphWeightSequence::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UMorphWeightSequence::StaticClass(), FColor(128,192,0), NULL, 0, this ) );
}

UBOOL UGenericBrowserType_MorphWeightSequence::ShowObjectEditor( UObject* InObject )
{
	// Do nothing...
	return true;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_UIScene
------------------------------------------------------------------------------*/
static UUISceneManager* GetUISceneManager()
{
	UBrowserManager* BrowserManager = GUnrealEd->GetBrowserManager();
	if ( BrowserManager->UISceneManager == NULL )
	{
		BrowserManager->UISceneManager = ConstructObject<UUISceneManager>(UUISceneManager::StaticClass());
		BrowserManager->UISceneManager->Initialize();
	}

	return BrowserManager->UISceneManager;
}

/**
 * Initialize the supported classes for this browser type.
 */
void UGenericBrowserType_UIScene::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UUIScene::StaticClass(), FColor(30,170,200), NULL, 0, this ) );

	UBrowserManager* BrowserManager = GUnrealEd->GetBrowserManager();
	if ( BrowserManager->UISceneManager == NULL )
	{
		BrowserManager->UISceneManager = ConstructObject<UUISceneManager>(UUISceneManager::StaticClass());
		BrowserManager->UISceneManager->Initialize();
	}

	SceneManager = BrowserManager->UISceneManager;
}

/**
 * Display the editor for the object specified.
 *
 * @param	InObject	the object to edit.  this should always be a UIScene object.
 */
UBOOL UGenericBrowserType_UIScene::ShowObjectEditor( UObject* Object )
{
	UBOOL bResult = FALSE;
	UUIScene* Scene = Cast<UUIScene>(Object);

	if ( Scene != NULL )
	{
		bResult = SceneManager->OpenScene(Scene);
	}

	return bResult;
}

/**
 * Called when the user chooses to delete objects from the generic browser.  Gives the resource type the opportunity
 * to perform any special logic prior to the delete.
 *
 * @param	ObjectToDelete	the object about to be deleted.
 *
 * @return	TRUE to allow the object to be deleted, FALSE to prevent the object from being deleted.
 */
UBOOL UGenericBrowserType_UIScene::NotifyPreDeleteObject( UObject* ObjectToDelete )
{
	UUIScene* Scene = CastChecked<UUIScene>(ObjectToDelete);
	SceneManager->NotifySceneDeletion(Scene);

	// remove from rootset scene packages are often in the list of startup packages so
	// they're always fully loaded and marked with RF_RootSet, making it impossible to delete the scene
	Scene->RemoveFromRoot();

	//@fixme ronp - need to restore this flag in postdeleteobject if delete is unsuccessful
// 	Scene->ClearFlags(RF_Standalone);

	return TRUE;
}

/**
 * Determines whether the specified package is allowed to be saved.
 *
 * @param	PackageToSave		the package that is about to be saved
 * @param	StandaloneObjects	a list of objects from PackageToSave which were marked RF_Standalone
 */
UBOOL UGenericBrowserType_UIScene::IsSavePackageAllowed( UPackage* PackageToSave, TArray<UObject*>& StandaloneObjects )
{
	TArray<UUIScreenObject*> DeprecatedChildren;

	for ( INT ObjIndex = 0; ObjIndex < StandaloneObjects.Num(); ObjIndex++ )
	{
		UUIScene* SceneObject = Cast<UUIScene>(StandaloneObjects(ObjIndex));
		if ( SceneObject != NULL )
		{
			// find any children in this scene whose class is deprecated
			SceneObject->FindDeprecatedWidgets(DeprecatedChildren);
		}
	}

	if ( DeprecatedChildren.Num() > 0 )
	{
		warnf(TEXT("Unable to save package '%s' because the following widgets have a deprecated class.  They must be deleted before this package can be saved:"), *PackageToSave->GetName());

		FStringOutputDevice ErrorAr;
		ErrorAr.Logf(TEXT("Unable to save package '%s' because the following widgets have a deprecated class.  They must be deleted before this package can be saved:") LINE_TERMINATOR,
			*PackageToSave->GetName());

		for ( INT ChildIndex = 0; ChildIndex < DeprecatedChildren.Num(); ChildIndex++ )
		{
			ErrorAr.Logf(TEXT("%s") LINE_TERMINATOR, *DeprecatedChildren(ChildIndex)->GetFullName());
			warnf(TEXT("%s"), *DeprecatedChildren(ChildIndex)->GetFullName());
		}

		// show user a dialog asking what they'd like to do
		//@todo ronp - for now it's just a simple message box...later we'll make it a more complex box
		appMsgf(AMT_OK, TEXT("%s"), *ErrorAr);
		return FALSE;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_UIArchetype
------------------------------------------------------------------------------*/
/**
 * Initialize the supported classes for this browser type.
 */
void UGenericBrowserType_UIArchetype::Init()
{
	//@todo - context menu for this browser type
	SupportInfo.AddItem( FGenericBrowserTypeInfo(UUIPrefab::StaticClass(), FColor(30,170,200), NULL, RF_ArchetypeObject, this) );

	UBrowserManager* BrowserManager = GUnrealEd->GetBrowserManager();
	if ( BrowserManager->UISceneManager == NULL )
	{
		BrowserManager->UISceneManager = ConstructObject<UUISceneManager>(UUISceneManager::StaticClass());
		BrowserManager->UISceneManager->Initialize();
	}

	SceneManager = BrowserManager->UISceneManager;
}

/**
 * Display the editor for the object specified.
 *
 * @param	InObject	the object to edit.  this should always be a UIObject which has the RF_ArchetypeObject flag set.
 */
UBOOL UGenericBrowserType_UIArchetype::ShowObjectEditor( UObject* InObject )
{
	UBOOL bResult = FALSE;

	UUIPrefab* ArchetypeWrapper = Cast<UUIPrefab>(InObject);
	if ( ArchetypeWrapper != NULL )
	{
		check(ArchetypeWrapper->HasAnyFlags(RF_ArchetypeObject));

		// get the transient scene that will be used to contain the archetype wrapper for the purposes of rendering and input processing
		UUIScene* WrapperScene = ArchetypeWrapper->GetScene();
		if ( WrapperScene == NULL )
		{
			FName SceneTitle = FName(*(ArchetypeWrapper->GetName() + TEXT("_") + UUIScene::StaticClass()->GetDescription()));

			// don't call SceneManager->CreateScene because it does alot of initialization and validation thatwe don't need to do
			WrapperScene = ConstructObject<UUIPrefabScene>(UUIPrefabScene::StaticClass(), ArchetypeWrapper->GetOutermost(), SceneTitle, RF_Transactional);
			WrapperScene->SceneTag = SceneTitle;
			WrapperScene->InsertChild(ArchetypeWrapper);
		}

		bResult = SceneManager->OpenScene(WrapperScene);
	}

	return bResult;
}

/**
 * Called when the user chooses to delete objects from the generic browser.  Gives the resource type the opportunity
 * to perform any special logic prior to the delete.
 *
 * @param	ObjectToDelete	the object about to be deleted.
 *
 * @return	TRUE to allow the object to be deleted, FALSE to prevent the object from being deleted.
 */
UBOOL UGenericBrowserType_UIArchetype::NotifyPreDeleteObject( UObject* ObjectToDelete )
{
	UUIPrefab* ArchetypeWrapper = Cast<UUIPrefab>(ObjectToDelete);
	if ( ArchetypeWrapper != NULL )
	{
		UUIScene* WrapperScene = ArchetypeWrapper->GetScene();
		if ( WrapperScene != NULL )
		{
			//@fixme - only call NotifySceneDeletion if the wrapper is the only child in the scene
			SceneManager->NotifySceneDeletion(WrapperScene);
		}
	}

	return TRUE;
}

/**
 * Determines whether the specified package is allowed to be saved.
 *
 * @param	PackageToSave		the package that is about to be saved
 * @param	StandaloneObjects	a list of objects from PackageToSave which were marked RF_Standalone
 */
UBOOL UGenericBrowserType_UIArchetype::IsSavePackageAllowed( UPackage* PackageToSave, TArray<UObject*>& StandaloneObjects )
{
	TArray<UUIScreenObject*> DeprecatedChildren;
	for ( INT ObjIndex = 0; ObjIndex < StandaloneObjects.Num(); ObjIndex++ )
	{
		UUIPrefab* ArchetypeWrapper = Cast<UUIPrefab>(StandaloneObjects(ObjIndex));
		if ( ArchetypeWrapper != NULL )
		{
			// find any children in this archetype whose class is deprecated
			ArchetypeWrapper->FindDeprecatedWidgets(DeprecatedChildren);
		}
	}

	if ( DeprecatedChildren.Num() > 0 )
	{
		warnf(TEXT("Unable to save package '%s' because the following widgets have a deprecated class.  They must be deleted before this package can be saved:"), *PackageToSave->GetName());

		FStringOutputDevice ErrorAr;
		ErrorAr.Logf(TEXT("Unable to save package '%s' because the following widgets have a deprecated class.  They must be deleted before this package can be saved:") LINE_TERMINATOR,
			*PackageToSave->GetName());

		for ( INT ChildIndex = 0; ChildIndex < DeprecatedChildren.Num(); ChildIndex++ )
		{
			ErrorAr.Logf(TEXT("%s") LINE_TERMINATOR, *DeprecatedChildren(ChildIndex)->GetFullName());
			warnf(TEXT("%s"), *DeprecatedChildren(ChildIndex)->GetFullName());
		}

		// show user a dialog asking what they'd like to do
		//@todo ronp - for now it's just a simple message box...later we'll make it a more complex box
		appMsgf(AMT_OK, TEXT("%s"), *ErrorAr);
		return FALSE;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
UGenericBrowserType_UISkin
------------------------------------------------------------------------------*/

void UGenericBrowserType_UISkin::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UUISkin::StaticClass(), FColor(15,125,150), NULL, 0, this ) );
}

UBOOL UGenericBrowserType_UISkin::ShowObjectEditor( UObject* Object )
{
	UBOOL bResult = FALSE;
	UUISkin* Skin = Cast<UUISkin>(Object);

	if ( Skin != NULL )
	{
		WxDlgUISkinEditor* Dialog = new WxDlgUISkinEditor;
		Dialog->Create((wxWindow*)GApp->EditorFrame, Skin );
		Dialog->Show();

		bResult = TRUE;
	}

	return bResult;
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_CurveEdPresetCurve
------------------------------------------------------------------------------*/

void UGenericBrowserType_CurveEdPresetCurve::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UCurveEdPresetCurve::StaticClass(), FColor(200,128,128), NULL, 0, this));
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_FaceFXAsset
------------------------------------------------------------------------------*/

void UGenericBrowserType_FaceFXAsset::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UFaceFXAsset::StaticClass(), FColor(200,128,128), new WxMBGenericBrowserContext_FaceFXAsset, 0, this));
}

#if WITH_FACEFX
extern OC3Ent::Face::FxStudioMainWin* GFaceFXStudio;
#endif // WITH_FACEFX

UBOOL UGenericBrowserType_FaceFXAsset::ShowObjectEditor( UObject* InObject )
{
#if WITH_FACEFX
	UFaceFXAsset* FaceFXAsset = Cast<UFaceFXAsset>(InObject);
	if( FaceFXAsset )
	{
		//@todo Replace these strings with localized strings at some point.
		USkeletalMesh* SkelMesh = FaceFXAsset->DefaultSkelMesh;
		OC3Ent::Face::FxActor* Actor = FaceFXAsset->GetFxActor();
		if( SkelMesh && Actor )
		{
			OC3Ent::Face::FxBool bShouldSetActor = FxTrue;
			OC3Ent::Face::FxStudioMainWin* FaceFXStudio = 
				static_cast<OC3Ent::Face::FxStudioMainWin*>(OC3Ent::Face::FxStudioApp::GetMainWindow());
			// This is only here to force the FaceFX Studio libraries to link.
			GFaceFXStudio = FaceFXStudio;
			if( !FaceFXStudio )
			{
				OC3Ent::Face::FxStudioApp::CheckForSafeMode();
				FaceFXStudio = new OC3Ent::Face::FxStudioMainWin(FxFalse, GApp->EditorFrame);
				OC3Ent::Face::FxStudioApp::SetMainWindow(FaceFXStudio);
				FaceFXStudio->Show();
				FaceFXStudio->Layout();
				FaceFXStudio->LoadOptions();
			}
			else
			{
				if( wxNO == wxMessageBox(wxString(TEXT("FaceFX Studio is already open and only one window may be open at once.  Do you want to use the existing window?")), 
					wxString(TEXT("FaceFX")), wxYES_NO) )
				{
					bShouldSetActor = FxFalse;
				}
			}
			if( FaceFXStudio && bShouldSetActor )
			{
				if( FaceFXStudio->GetSession().SetActor(Actor, FaceFXAsset) )
				{
					OC3Ent::Face::FxRenderWidgetUE3* RenderWidgetUE3 = static_cast<OC3Ent::Face::FxRenderWidgetUE3*>(FaceFXStudio->GetRenderWidget());
					if( RenderWidgetUE3 )
					{
						RenderWidgetUE3->SetSkeletalMesh(SkelMesh);
					}
				}
				FaceFXStudio->SetFocus();
			}
			return 1;
		}
		else
		{
			wxMessageBox(wxString(TEXT("The FaceFX Asset does not reference a Skeletal Mesh.  Use the AnimSet Viewer to set a Skeletal Mesh's FaceFXAsset property.")), wxString(TEXT("FaceFX")));
		}
	}
#endif // WITH_FACEFX
	return 0;
}

UBOOL UGenericBrowserType_FaceFXAsset::ShowObjectProperties( UObject* InObject )
{
#if WITH_FACEFX
	WxPropertyWindowFrame* ObjectPropertyWindow = NULL;
	OC3Ent::Face::FxStudioMainWin* FaceFXStudio = 
		static_cast<OC3Ent::Face::FxStudioMainWin*>(OC3Ent::Face::FxStudioApp::GetMainWindow());
	if( FaceFXStudio )
	{
		OC3Ent::Face::FxRenderWidgetUE3* RenderWidgetUE3 = static_cast<OC3Ent::Face::FxRenderWidgetUE3*>(FaceFXStudio->GetRenderWidget());
		if( RenderWidgetUE3 )
		{
			ObjectPropertyWindow = new WxPropertyWindowFrame;
			ObjectPropertyWindow->Create(GApp->EditorFrame, -1, TRUE, RenderWidgetUE3);
		}
	}
	else
	{
		ObjectPropertyWindow = new WxPropertyWindowFrame;
		ObjectPropertyWindow->Create(GApp->EditorFrame, -1, TRUE, GUnrealEd);
	}
#else
	WxPropertyWindowFrame* ObjectPropertyWindow = new WxPropertyWindowFrame;
	ObjectPropertyWindow->Create(GApp->EditorFrame, -1, TRUE, GUnrealEd);
#endif // WITH_FACEFX
	if( ObjectPropertyWindow )
	{
		ObjectPropertyWindow->SetSize(64,64, 350,600);
		ObjectPropertyWindow->SetObject(InObject, TRUE, TRUE, TRUE);
		ObjectPropertyWindow->Show();
	}
	return TRUE;
}

UBOOL UGenericBrowserType_FaceFXAsset::ShowObjectProperties( const TArray<UObject*>& InObjects )
{
#if WITH_FACEFX
    WxPropertyWindowFrame* ObjectPropertyWindow = NULL;
	OC3Ent::Face::FxStudioMainWin* FaceFXStudio = 
		static_cast<OC3Ent::Face::FxStudioMainWin*>(OC3Ent::Face::FxStudioApp::GetMainWindow());
	if( FaceFXStudio )
	{
		OC3Ent::Face::FxRenderWidgetUE3* RenderWidgetUE3 = static_cast<OC3Ent::Face::FxRenderWidgetUE3*>(FaceFXStudio->GetRenderWidget());
		if( RenderWidgetUE3 )
		{
			ObjectPropertyWindow = new WxPropertyWindowFrame;
			ObjectPropertyWindow->Create(GApp->EditorFrame, -1, TRUE, RenderWidgetUE3);
		}
	}
	else
	{
		ObjectPropertyWindow = new WxPropertyWindowFrame;
		ObjectPropertyWindow->Create(GApp->EditorFrame, -1, TRUE, GUnrealEd);
	}
#else
	WxPropertyWindowFrame* ObjectPropertyWindow = new WxPropertyWindowFrame;
	ObjectPropertyWindow->Create(GApp->EditorFrame, -1, TRUE, GUnrealEd);
#endif // WITH_FACEFX
	if( ObjectPropertyWindow )
	{
		ObjectPropertyWindow->SetSize(64,64, 350,600);
		ObjectPropertyWindow->SetObjectArray(InObjects, TRUE, TRUE, TRUE);
		ObjectPropertyWindow->Show();
	}
	return TRUE;
}

void UGenericBrowserType_FaceFXAsset::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
#if WITH_FACEFX
	WxGenericBrowser* GenericBrowser = static_cast<WxGenericBrowser*>(GUnrealEd->GetBrowserManager()->GetBrowserPane(TEXT("GenericBrowser")));
	UFaceFXAsset* FaceFXAsset = Cast<UFaceFXAsset>(InObject);
	if( GenericBrowser && FaceFXAsset )
	{
		OC3Ent::Face::FxActor* Actor = FaceFXAsset->GetFxActor();
		if( Actor )
		{
			if( Actor->IsOpenInStudio() )
			{
				//@todo Replace these strings with localized strings at some point.
				wxMessageBox(wxString(TEXT("The FaceFX Asset is currently open in FaceFX Studio.  Please close FaceFX Studio and try again.")), wxString(TEXT("FaceFX")));
			}
			else
			{
				if( InCommand == IDMN_ObjectContext_CreateNewFaceFXAnimSet )
				{
					UFaceFXAsset* FaceFXAsset = Cast<UFaceFXAsset>(InObject);
					if( FaceFXAsset )
					{
						// Create a new FaceFXAnimSet from the selected FaceFXAsset. 
						// Defaults to being in the same package/group as the FaceFXAsset.
						FString DefaultPackage = *FaceFXAsset->GetOutermost()->GetName();
						FString DefaultGroup = FaceFXAsset->GetOuter()->GetOuter() ? FaceFXAsset->GetFullGroupName(TRUE) : TEXT("");

						// Make default name by appending '_AnimSet' to the FaceFXAsset name.
						FString DefaultAnimSetName = FString(FString::Printf( TEXT("%s_AnimSet"), *FaceFXAsset->GetName()));

						// First of all show the dialog for choosing a new package file, group and asset name:
						WxDlgPackageGroupName PackageDlg;
						if( PackageDlg.ShowModal(DefaultPackage, DefaultGroup, DefaultAnimSetName) == wxID_OK )
						{
							// Get the full name of where we want to create the FaceFXAnimSet.
							FString PackageName;
							if( PackageDlg.GetGroup().Len() > 0 )
							{
								PackageName = PackageDlg.GetPackage() + TEXT(".") + PackageDlg.GetGroup();
							}
							else
							{
								PackageName = PackageDlg.GetPackage();
							}

							// Then find/create it.
							UPackage* Package = UObject::CreatePackage(NULL, *PackageName);
							check(Package);

							FString ExistingAnimSetPath = PackageName;
							ExistingAnimSetPath += ".";
							ExistingAnimSetPath += *PackageDlg.GetObjectName();
							UFaceFXAnimSet* ExistingAnimSet = LoadObject<UFaceFXAnimSet>(NULL, *ExistingAnimSetPath, NULL, LOAD_NoWarn, NULL);
							if( ExistingAnimSet )
							{
								//@todo Localize this string.
								wxMessageBox(wxString(TEXT("That FaceFXAnimSet already exists!  Please remove the existing one first.")), wxString(TEXT("FaceFX")));
								return;
							}

							UFaceFXAnimSet* NewAnimSet = ConstructObject<UFaceFXAnimSet>(UFaceFXAnimSet::StaticClass(), Package, *PackageDlg.GetObjectName(), RF_Public|RF_Standalone|RF_Transactional);
							if( NewAnimSet )
							{
								NewAnimSet->CreateFxAnimSet(FaceFXAsset);
								NewAnimSet->MarkPackageDirty();
								GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
							}
						}
					}
				}
				else if( InCommand == IDMN_ObjectContext_ImportFaceFXAsset )
				{
					WxFileDialog OpenFileDialog(GenericBrowser, 
						*LocalizeUnrealEd("Import"),
						*GenericBrowser->LastImportPath,
						TEXT(""),
						wxT("FaceFX Actor (*.fxa)|*.fxa|All Files (*.*)|*.*"),
						wxOPEN|wxFILE_MUST_EXIST);

					if( OpenFileDialog.ShowModal() == wxID_OK )
					{
                        // Import the actor.
						OC3Ent::Face::FxString ActorFilename(OpenFileDialog.GetPath().mb_str(wxConvLibc));
						if( OC3Ent::Face::FxLoadActorFromFile(*Actor, ActorFilename.GetData(), FxTrue) )
						{
							// Update the RawFaceFXActorBytes.
							OC3Ent::Face::FxByte* ActorMemory = NULL;
							OC3Ent::Face::FxSize  NumActorMemoryBytes = 0;
							if( !OC3Ent::Face::FxSaveActorToMemory(*Actor, ActorMemory, NumActorMemoryBytes) )
							{
								warnf(TEXT("FaceFX: Failed to save actor for %s"), *(FaceFXAsset->GetPathName()));
							}
							else
							{
								FaceFXAsset->RawFaceFXActorBytes.Empty();
								FaceFXAsset->RawFaceFXActorBytes.Add(NumActorMemoryBytes);
								appMemcpy(FaceFXAsset->RawFaceFXActorBytes.GetData(), ActorMemory, NumActorMemoryBytes);
								OC3Ent::Face::FxFree(ActorMemory, NumActorMemoryBytes);
								FaceFXAsset->ReferencedSoundCues.Empty();
								FaceFXAsset->FixupReferencedSoundCues();
								Actor->SetShouldClientRelink(FxTrue);
								// The package has changed so mark it dirty.
								FaceFXAsset->MarkPackageDirty();
								GCallbackEvent->Send(CALLBACK_RefreshEditor_GenericBrowser);
							}
						}
						else
						{
							warnf(TEXT("FaceFX: Failed to import %s for %s"), ANSI_TO_TCHAR(ActorFilename.GetData()), *(FaceFXAsset->GetPathName()));
						}
					}
				}
				else if( InCommand == IDMN_ObjectContext_ExportFaceFXAsset )
				{
					WxFileDialog SaveFileDialog(GenericBrowser, 
						*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Save_F"), *FaceFXAsset->GetName())),
						*GenericBrowser->LastExportPath,
						*FaceFXAsset->GetName(),
						wxT("FaceFX Actor (*.fxa)|*.fxa|All Files (*.*)|*.*"),
						wxSAVE|wxOVERWRITE_PROMPT);

					if( SaveFileDialog.ShowModal() == wxID_OK )
					{
						// Save the actor.
						OC3Ent::Face::FxString ActorFilename(SaveFileDialog.GetPath().mb_str(wxConvLibc));
						if( !OC3Ent::Face::FxSaveActorToFile(*Actor, ActorFilename.GetData()) )
						{
							warnf(TEXT("FaceFX: Failed to export %s for %s"), ANSI_TO_TCHAR(ActorFilename.GetData()), *(FaceFXAsset->GetPathName()));
						}
					}
				}
			}
		}
	}
#endif // WITH_FACEFX
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_FaceFXAnimSet
------------------------------------------------------------------------------*/

void UGenericBrowserType_FaceFXAnimSet::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UFaceFXAnimSet::StaticClass(), FColor(200,128,128), new WxMBGenericBrowserContext_FaceFXAnimSet, 0, this));
}

UBOOL UGenericBrowserType_FaceFXAnimSet::ShowObjectEditor( UObject* InObject )
{
#if WITH_FACEFX
	UFaceFXAnimSet* FaceFXAnimSet = Cast<UFaceFXAnimSet>(InObject);
	if( FaceFXAnimSet )
	{
		UFaceFXAsset* FaceFXAsset = FaceFXAnimSet->DefaultFaceFXAsset;
		if( FaceFXAsset )
		{
			//@todo Replace these strings with localized strings at some point.
			USkeletalMesh* SkelMesh = FaceFXAsset->DefaultSkelMesh;
			OC3Ent::Face::FxActor* Actor = FaceFXAsset->GetFxActor();
			if( SkelMesh && Actor )
			{
				OC3Ent::Face::FxBool bShouldSetActor = FxTrue;
				OC3Ent::Face::FxStudioMainWin* FaceFXStudio = 
					static_cast<OC3Ent::Face::FxStudioMainWin*>(OC3Ent::Face::FxStudioApp::GetMainWindow());
				// This is only here to force the FaceFX Studio libraries to link.
				GFaceFXStudio = FaceFXStudio;
				if( !FaceFXStudio )
				{
					OC3Ent::Face::FxStudioApp::CheckForSafeMode();
					FaceFXStudio = new OC3Ent::Face::FxStudioMainWin(FxFalse, GApp->EditorFrame);
					OC3Ent::Face::FxStudioApp::SetMainWindow(FaceFXStudio);
					FaceFXStudio->Show();
					FaceFXStudio->Layout();
					FaceFXStudio->LoadOptions();
				}
				else
				{
					if( wxNO == wxMessageBox(wxString(TEXT("FaceFX Studio is already open and only one window may be open at once.  Do you want to use the existing window?")), 
						wxString(TEXT("FaceFX")), wxYES_NO) )
					{
						bShouldSetActor = FxFalse;
					}
				}
				if( FaceFXStudio && bShouldSetActor )
				{
					if( FaceFXStudio->GetSession().SetActor(Actor, FaceFXAsset, FaceFXAnimSet) )
					{
						OC3Ent::Face::FxRenderWidgetUE3* RenderWidgetUE3 = static_cast<OC3Ent::Face::FxRenderWidgetUE3*>(FaceFXStudio->GetRenderWidget());
						if( RenderWidgetUE3 )
						{
							RenderWidgetUE3->SetSkeletalMesh(SkelMesh);
						}
					}
					FaceFXStudio->SetFocus();
				}
				return 1;
			}
			else
			{
				wxMessageBox(wxString(TEXT("The FaceFX Asset does not reference a Skeletal Mesh.  Use the AnimSet Viewer to set a Skeletal Mesh's FaceFXAsset property.")), wxString(TEXT("FaceFX")));
			}
		}
		else
		{
			wxMessageBox(wxString(TEXT("The FaceFX AnimSet does not reference a FaceFX Asset.  Right click the FaceFX AnimSet and use the property editor to set the FaceFXAsset property.")), wxString(TEXT("FaceFX")));
		}
	}
#endif // WITH_FACEFX
	return 0;
}

void UGenericBrowserType_FaceFXAnimSet::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
#if WITH_FACEFX
	WxGenericBrowser* GenericBrowser = static_cast<WxGenericBrowser*>(GUnrealEd->GetBrowserManager()->GetBrowserPane(TEXT("GenericBrowser")));
	UFaceFXAnimSet* FaceFXAnimSet = Cast<UFaceFXAnimSet>(InObject);
	if( GenericBrowser && FaceFXAnimSet )
	{
		//@todo Implement this!
	}
#endif // WITH_FACEFX
}

/*------------------------------------------------------------------------------
	UGenericBrowserType_CameraAnim
------------------------------------------------------------------------------*/

void UGenericBrowserType_CameraAnim::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UCameraAnim::StaticClass(), FColor(200,128,128), NULL, 0, this));
}

UBOOL UGenericBrowserType_CameraAnim::ShowObjectEditor( UObject* InObject )
{
	UCameraAnim* InCamAnim = Cast<UCameraAnim>(InObject);

	if (InCamAnim)
	{
		// get a Kismet window
		WxKismet::OpenKismet(NULL, FALSE, (wxWindow*)GApp->EditorFrame);
		WxKismet* KismetWindow = GApp->KismetWindows.Last();
		// @todo: minimize/hide this temp window?  might not be a temp window, though.

		// construct a temporary SeqAct_Interp object.
		USeqAct_Interp* TempInterp = (USeqAct_Interp*)KismetWindow->NewSequenceObject(USeqAct_Interp::StaticClass(), 0, 0, TRUE);
		TempInterp->SetFlags(RF_Transient);
		TempInterp->ObjComment = TEXT("TEMP FOR CAMERAANIM EDITING, DO NOT EDIT BY HAND");

		// @todo, find a way to mark TempInterp and attached InterpData so they don't 
		// get drawn int he window and cannot be edited by hand

		UInterpGroup* NewInterpGroup = InCamAnim->CameraInterpGroup;
		check(NewInterpGroup);

		// attach temp interpgroup to the interpdata object
		UInterpData* InterpData = TempInterp->FindInterpDataFromVariable();
		if (InterpData)
		{
			InterpData->SetFlags(RF_Transient);
			InterpData->InterpLength = InCamAnim->AnimLength;

			if (NewInterpGroup)
			{
				InterpData->InterpGroups.AddItem(NewInterpGroup);
			}
		}

		// create a CameraActor and connect it to the Interp.  will create this at the perspective viewport's location and rotation
		ACameraActor* TempCameraActor = NULL;
		{
			FVector ViewportCamLocation(0.f);
			FRotator ViewportCamRotation;
			{
				for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
				{
					// find perspective window and note the camera location
					const WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->GetViewport(i).ViewportWindow;
					if(LevelVC->ViewportType == LVT_Perspective)
					{
						ViewportCamLocation = LevelVC->ViewLocation;
						ViewportCamRotation = LevelVC->ViewRotation;
						break;
					}
				}
			}

			TempCameraActor = (ACameraActor*)GWorld->SpawnActor( ACameraActor::StaticClass(), NAME_None, ViewportCamLocation, ViewportCamRotation, NULL, TRUE );
			check(TempCameraActor);
		}

		// set up the group actor
		TempInterp->InitGroupActorForGroup(NewInterpGroup, TempCameraActor);

		// this will create the instances for everything
		TempInterp->InitInterp();

		// open up the matinee window in camera anim mode
		{
			// If already in Matinee mode, exit out before going back in with new Interpolation.
			if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
			{
				GEditorModeTools().SetCurrentMode( EM_Default );
			}

			GEditorModeTools().SetCurrentMode( EM_InterpEdit );

			FEdModeInterpEdit* InterpEditMode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();

			WxCameraAnimEd* const CamAnimEd = InterpEditMode->InitCameraAnimMode( TempInterp );

			// start out looking through the camera
			CamAnimEd->LockCamToGroup(NewInterpGroup);
		}

		// all good
		return TRUE;
	}

	return Super::ShowObjectEditor(InObject);
}

