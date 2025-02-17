/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineAudioDeviceClasses.h"
#include "GenericBrowserContextMenus.h"

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContextBase.
-----------------------------------------------------------------------------*/

/** Enables/disables menu items supported by the specified selection set. */
void WxMBGenericBrowserContextBase::SetObjects(const USelection* Selection)
{
	// Iterate over the selection set looking for a cooked object.
	UBOOL bFoundCookedObject = FALSE;
	for ( USelection::TObjectConstIterator It( Selection->ObjectConstItor() ) ; It ; ++It )
	{
		const UObject* Object = *It;
		if ( Object )
		{
			const UPackage* ObjectPackage = Object->GetOutermost();
			if ( ObjectPackage->PackageFlags & PKG_Cooked )
			{
				bFoundCookedObject = TRUE;
				break;
			}
		}
	}

	ToggleItemsForCookedContent( bFoundCookedObject );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext::WxMBGenericBrowserContext()
	:	ExportItem( NULL )
	,	DuplicateItem( NULL )
	,	RenameItem( NULL )
	,	DeleteItem( NULL )
	,	DeleteWithReferencesItem( NULL )
{}

void WxMBGenericBrowserContext::AppendObjectMenu()
{
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	ExportItem = Append(IDMN_ObjectContext_Export,*LocalizeUnrealEd("ExportFileE"),TEXT(""));
	AppendSeparator();
	DuplicateItem = Append(IDMN_ObjectContext_DuplicateWithRefs,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	RenameItem = Append(IDMN_ObjectContext_RenameWithRefs,*LocalizeUnrealEd("Rename"),TEXT(""));
	DeleteItem = Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	DeleteWithReferencesItem = Append(IDMN_ObjectContext_DeleteWithRefs,*LocalizeUnrealEd("DeleteWithReferences"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
	Append(IDMN_ObjectContext_ShowRefObjs, *LocalizeUnrealEd("ShowReferences"),TEXT(""));
}

void WxMBGenericBrowserContext::AppendLocObjectMenu()
{
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	ExportItem = Append(IDMN_ObjectContext_Export,*LocalizeUnrealEd("ExportFileE"),TEXT(""));
	AppendSeparator();
	DuplicateItem = Append(IDMN_ObjectContext_DuplicateWithRefs,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	RenameItem = Append(IDMN_ObjectContext_RenameWithRefs,*LocalizeUnrealEd("Rename"),TEXT(""));
	RenameLocItem = Append(IDMN_ObjectContext_RenameLocWithRefs,*LocalizeUnrealEd("RenameLoc"),TEXT(""));
	DeleteItem = Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	DeleteWithReferencesItem = Append(IDMN_ObjectContext_DeleteWithRefs,*LocalizeUnrealEd("DeleteWithReferences"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
	Append(IDMN_ObjectContext_ShowRefObjs, *LocalizeUnrealEd("ShowReferences"),TEXT(""));
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	if ( ExportItem ) Enable( ExportItem->GetId(), !bSomeObjectsAreCooked );
	if ( DuplicateItem ) Enable( DuplicateItem->GetId(), !bSomeObjectsAreCooked );
	if ( RenameItem ) Enable( RenameItem->GetId(), !bSomeObjectsAreCooked );
	if ( DeleteItem ) Enable( DeleteItem->GetId(), !bSomeObjectsAreCooked );
	if ( DeleteWithReferencesItem ) Enable( DeleteWithReferencesItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Object.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Object::WxMBGenericBrowserContext_Object()
{
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Archetype.
-----------------------------------------------------------------------------*/
WxMBGenericBrowserContext_Archetype::WxMBGenericBrowserContext_Archetype()
{
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Material.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Material::WxMBGenericBrowserContext_Material()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("MaterialEditor"),TEXT(""));
	Append(IDMN_ObjectContent_AddTag,*LocalizeUnrealEd("AddTag"),TEXT(""));
	CreateNewMaterialItem = Append(IDMN_ObjectContext_CreateNewMaterialInstanceConstant,*LocalizeUnrealEd("CreateNewMaterialInstanceConstant"),TEXT(""));
	CreateNewMaterialTimeVaryingItem = Append(IDMN_ObjectContext_CreateNewMaterialInstanceTimeVarying,*LocalizeUnrealEd("CreateNewMaterialInstanceTimeVarying"),TEXT(""));
	AppendObjectMenu();
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_Material::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( CreateNewMaterialItem->GetId(), !bSomeObjectsAreCooked );
	Enable( CreateNewMaterialTimeVaryingItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_MaterialInstanceTimeVarying
-----------------------------------------------------------------------------*/
WxMBGenericBrowserContext_MaterialInstanceTimeVarying::WxMBGenericBrowserContext_MaterialInstanceTimeVarying()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("MaterialInstanceTimeVaryingEditor"),TEXT(""));
	CreateNewMaterialTimeVaryingItem = Append(IDMN_ObjectContext_CreateNewMaterialInstanceTimeVarying,*LocalizeUnrealEd("CreateNewMaterialInstanceTimeVarying"),TEXT(""));
	AppendObjectMenu();
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_MaterialInstanceTimeVarying::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( CreateNewMaterialTimeVaryingItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Texture.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Texture::WxMBGenericBrowserContext_Texture()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("TextureViewer"),TEXT(""));
	ReimportTextItem = Append(IDMN_ObjectContext_Reimport,*LocalizeUnrealEd(TEXT("ReimportTexture")),TEXT(""));
	CreateNewMaterialItem = Append(IDMN_ObjectContext_CreateNewMaterial,*LocalizeUnrealEd(TEXT("CreateNewMaterial")),TEXT(""));
	AppendCheckItem(IDMN_TextureContext_ShowStreamingBounds,*LocalizeUnrealEd(TEXT("ShowStreamingBounds")),TEXT(""));
	AppendObjectMenu();
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_Texture::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( ReimportTextItem->GetId(), !bSomeObjectsAreCooked );
	Enable( CreateNewMaterialItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_RenderTexture.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_RenderTexture::WxMBGenericBrowserContext_RenderTexture()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("TextureViewer"),TEXT(""));
	CreateStaticTextureItem = Append(IDMN_ObjectContext_RTCreateStaticTexture,*LocalizeUnrealEd(TEXT("CreateStaticTextureE")),TEXT(""));
	AppendObjectMenu();
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_RenderTexture::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( CreateStaticTextureItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_StaticMesh.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_StaticMesh::WxMBGenericBrowserContext_StaticMesh()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("StaticMeshEditor"),TEXT(""));
	Append(IDMN_ObjectContent_AddTag,*LocalizeUnrealEd("AddTag"),TEXT(""));

	AppendObjectMenu();
}


/** Enables/disables menu items supported by the specified selection set. */
void WxMBGenericBrowserContext_StaticMesh::SetObjects(const USelection* Selection)
{
	// Clear any items we may have added earlier.  We want to rebuild every time the user summons it.
	if( FindItem( IDMN_StaticMeshContext_SimplifyMesh ) != NULL )
	{
		Delete( IDMN_StaticMeshContext_SimplifyMesh );
	}

	WxMBGenericBrowserContextBase::SetObjects( Selection );

	UBOOL bHaveNonSimplifiedStaticMesh = FALSE;
	UBOOL bHaveSimplifiedStaticMesh = FALSE;

	INT NumSelectedObjects = 0;
	for( FSelectionIterator SelIter( const_cast< USelection& >( *Selection ) ); SelIter != NULL; ++SelIter )
	{
		UObject* SelObject = *SelIter;
		if( SelObject->IsA( UStaticMesh::StaticClass() ) )
		{
			UStaticMesh* StaticMesh = CastChecked<UStaticMesh>( SelObject );

			// Check to see if the mesh is already simplified.  If it has a source mesh name we can
			// assume that it is.
			if( StaticMesh->HighResSourceMeshName.Len() == 0 )
			{
				bHaveNonSimplifiedStaticMesh = TRUE;
			}
			else
			{
				bHaveSimplifiedStaticMesh = TRUE;
			}
		}

		++NumSelectedObjects;
	}


	// Only show this option when there is a single static mesh selected
	if( NumSelectedObjects == 1 )
	{
		// Insert our item right after the 'Static Mesh Editor' option
		const INT ItemInsertPos = 1;

		// Check to see if we have any meshes that can be simplified
		if( bHaveNonSimplifiedStaticMesh )
		{
			Insert( ItemInsertPos, IDMN_StaticMeshContext_SimplifyMesh, *LocalizeUnrealEd( "GenericBrowserContext_SimplifyMesh" ), TEXT( "" ) );
		}
		else if( bHaveSimplifiedStaticMesh )
		{
			Insert( ItemInsertPos, IDMN_StaticMeshContext_SimplifyMesh, *LocalizeUnrealEd( "GenericBrowserContext_ResimplifyMesh" ), TEXT( "" ) );
		}
	}
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FracturedStaticMesh.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_FracturedStaticMesh::WxMBGenericBrowserContext_FracturedStaticMesh()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("StaticMeshEditor"),TEXT(""));
	Append(IDMN_ObjectContext_CreateNewPhysXDestructible,*LocalizeUnrealEd("CreateNewPhysXDestructible"),TEXT("")); 
	Append(IDMN_ObjectContext_ResliceFracturedMeshes,*LocalizeUnrealEd("ResliceFracturedMeshes"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Sound.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Sound::WxMBGenericBrowserContext_Sound()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("SoundNodeWavePreview"),TEXT(""));
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Sound_Play,*LocalizeUnrealEd("Play"),TEXT(""));
	Append(IDMN_ObjectContext_Sound_Stop,*LocalizeUnrealEd("Stop"),TEXT(""));
	AppendLocObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SoundCue.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_SoundCue::WxMBGenericBrowserContext_SoundCue()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("SoundCueEditor"),TEXT(""));
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	EditNodesItem = Append(IDMN_ObjectContext_EditNodes,*LocalizeUnrealEd("EditNodesE"),TEXT(""));
	AppendSeparator();	
	Append(IDMN_ObjectContext_Sound_Play,*LocalizeUnrealEd("Play"),TEXT(""));
	Append(IDMN_ObjectContext_Sound_Stop,*LocalizeUnrealEd("Stop"),TEXT(""));
	AppendSeparator();

	// Add sound group menu & submenu.
	wxMenu*			SoundGroupMenu	= new wxMenu();

	UEnum* SoundClassNames = FindObject<UEnum>( NULL, TEXT( "Engine.AudioDevice.ESoundClassName" ) );
	if( SoundClassNames )
	{
		for( INT NameIndex = 0; NameIndex < SoundClassNames->NumEnums() - 1; NameIndex++ )
		{
			// Add sound group to submenu.
			FName SoundClassName = SoundClassNames->GetEnum( NameIndex );
			SoundGroupMenu->Append( IDMN_ObjectContext_SoundCue_SoundGroups_START + NameIndex, *SoundClassName.ToString() );
		}

		// Add menu.
		Append( IDMN_ObjectContext_SoundCue_SoundGroups, *LocalizeUnrealEd( "SoundGroups" ), SoundGroupMenu );
	}

	AppendObjectMenu();
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_SoundCue::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( EditNodesItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SoundMode
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_SoundMode::WxMBGenericBrowserContext_SoundMode()
{
	Append( IDMN_ObjectContext_Properties, *LocalizeUnrealEd( "PropertiesE" ), TEXT( "" ) );
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SpeechRecognition.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_SpeechRecognition::WxMBGenericBrowserContext_SpeechRecognition()
{
	Append( IDMN_ObjectContext_Properties, *LocalizeUnrealEd( "PropertiesE" ), TEXT( "" ) );
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_PhysicsAsset.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_PhysicsAsset::WxMBGenericBrowserContext_PhysicsAsset()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("PhysicsAssetEditor"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FaceFXAsset.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_FaceFXAsset::WxMBGenericBrowserContext_FaceFXAsset()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("FaceFXStudio"),TEXT(""));
    AppendSeparator();
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendSeparator();
	CreateFaceFXAnimSetItem = Append(IDMN_ObjectContext_CreateNewFaceFXAnimSet,*LocalizeUnrealEd("CreateFaceFXAnimSet"),TEXT(""));
	AppendSeparator();
	ImportFromFXAItem = Append(IDMN_ObjectContext_ImportFaceFXAsset,*LocalizeUnrealEd("ImportFromFXA"),TEXT(""));
	ExporttoFXAItem = Append(IDMN_ObjectContext_ExportFaceFXAsset,*LocalizeUnrealEd("ExportToFXA"),TEXT(""));
	// Don't want to show the "Export to File..." option as it may be confusing
	// since there is already an "Export to ..." option, so rather than call 
	// AppendObjectMenu() simply do it in-place.
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	AppendSeparator();
	DuplicateItem = Append(IDMN_ObjectContext_DuplicateWithRefs,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	RenameItem = Append(IDMN_ObjectContext_RenameWithRefs,*LocalizeUnrealEd("Rename"),TEXT(""));
	DeleteItem = Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_FaceFXAsset::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( CreateFaceFXAnimSetItem->GetId(), !bSomeObjectsAreCooked );
	Enable( ImportFromFXAItem->GetId(), !bSomeObjectsAreCooked );
	Enable( ExporttoFXAItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FaceFXAnimSet.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_FaceFXAnimSet::WxMBGenericBrowserContext_FaceFXAnimSet()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("FaceFXStudio"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	// Don't want to show the "Export to File..." option as it may be confusing
	// since there is already an "Export to ..." option, so rather than call 
	// AppendObjectMenu() simply do it in-place.
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	AppendSeparator();
	DuplicateItem = Append(IDMN_ObjectContext_DuplicateWithRefs,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	RenameItem = Append(IDMN_ObjectContext_RenameWithRefs,*LocalizeUnrealEd("Rename"),TEXT(""));
	DeleteItem = Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_Skeletal.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Skeletal::WxMBGenericBrowserContext_Skeletal()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimSetViewer"),TEXT("")); 
	CreateNewPhysicsAssetItem = Append(IDMN_ObjectContext_CreateNewPhysicsAsset,*LocalizeUnrealEd("CreateNewPhysicsAsset"),TEXT("")); 
#if WITH_FACEFX
	CreateNewFaceFXAssetItem = Append(IDMN_ObjectContext_CreateNewFaceFXAsset, *LocalizeUnrealEd("CreateNewFaceFXAsset"), TEXT(""));
#endif // WITH_FACEFX
	AppendObjectMenu();
}

/** Enables/disables menu items based on whether the selection set contains cooked objects. */
void WxMBGenericBrowserContext_Skeletal::ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked)
{
	Super::ToggleItemsForCookedContent( bSomeObjectsAreCooked );

	Enable( CreateNewPhysicsAssetItem->GetId(), !bSomeObjectsAreCooked );
	Enable( CreateNewFaceFXAssetItem->GetId(), !bSomeObjectsAreCooked );
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_LensFlare
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_LensFlare::WxMBGenericBrowserContext_LensFlare()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("EditorLensFlare"),TEXT("")); 
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_ParticleSystem.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_ParticleSystem::WxMBGenericBrowserContext_ParticleSystem()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("EditorCascade"),TEXT("")); 
	Append(IDMN_ObjectContent_AddTag, *LocalizeUnrealEd("AddTag"),TEXT(""));
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyParticleParameters,*LocalizeUnrealEd("ParticleCopyParameters"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_PhysXParticleSystem.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_PhysXParticleSystem::WxMBGenericBrowserContext_PhysXParticleSystem()
{
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_AnimSet.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_AnimSet::WxMBGenericBrowserContext_AnimSet()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimSetViewer"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_MorphTargetSet.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_MorphTargetSet::WxMBGenericBrowserContext_MorphTargetSet()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimSetViewer"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	XMBGenericBrowserContext_AnimTree.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_AnimTree::WxMBGenericBrowserContext_AnimTree()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimTreeEditor"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	XMBGenericBrowserContext_AnimTree.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_PostProcess::WxMBGenericBrowserContext_PostProcess()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("PostProcessEditor"),TEXT(""));
	AppendObjectMenu();
}
