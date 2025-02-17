/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "UnTerrain.h"
#include "EngineAIClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineForceFieldClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineFogVolumeClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EnginePrefabClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineFoliageClasses.h"
#include "EngineSpeedTreeClasses.h"
#include "EngineFluidClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "LensFlare.h"
#include "UnStatChart.h"
#include "UnNet.h"
#include "UnCodecs.h"
#include "RemoteControl.h"
#include "FFileManagerGeneric.h"
#include "DemoRecording.h"
#include "PerfMem.h"
#include "Database.h"

#if !CONSOLE && defined(_MSC_VER)
#include "..\..\UnrealEd\Inc\DebugToolExec.h"
#include "..\Debugger\UnDebuggerCore.h"
#endif

/*-----------------------------------------------------------------------------
	Static linking.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName ENGINE_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "EngineGameEngineClasses.h"
#include "EngineClasses.h"
#include "EngineAIClasses.h"
#include "EngineMaterialClasses.h"
#include "EngineTerrainClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineForceFieldClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineFogVolumeClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EnginePrefabClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineSceneClasses.h"
#include "EngineFoliageClasses.h"
#include "EngineSpeedTreeClasses.h"
#include "EngineLensFlareClasses.h"
#include "EngineFluidClasses.h"
#include "EngineAudioDeviceClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "EngineGameEngineClasses.h"
#include "EngineClasses.h"
#include "EngineAIClasses.h"
#include "EngineMaterialClasses.h"
#include "EngineTerrainClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineForceFieldClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineFogVolumeClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EnginePrefabClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineSceneClasses.h"
#include "EngineFoliageClasses.h"
#include "EngineSpeedTreeClasses.h"
#include "EngineLensFlareClasses.h"
#include "EngineFluidClasses.h"
#include "EngineAudioDeviceClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsEngine( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_GAMEENGINE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_AI
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_ANIM
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_DECAL
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_FOGVOLUME
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_MESH
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_INTERPOLATION
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_MATERIAL
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_PARTICLE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_PHYSICS
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_FORCEFIELD
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_PREFAB
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_SEQUENCE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_SOUND
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_TERRAIN
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_USERINTERFACE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_UIPRIVATE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_UISEQUENCE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_SCENE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_FOLIAGE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_FLUID
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_SPEEDTREE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_LENSFLARE
	AUTO_INITIALIZE_REGISTRANTS_ENGINE_AUDIODEVICE
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesEngine()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define AUTOGENERATE_NAME(name) ENGINE_##name = FName(TEXT(#name));
	#include "EngineGameEngineClasses.h"
	#include "EngineClasses.h"
	#include "EngineAIClasses.h"
	#include "EngineMaterialClasses.h"
	#include "EngineTerrainClasses.h"
	#include "EnginePhysicsClasses.h"
	#include "EngineForceFieldClasses.h"
	#include "EngineSequenceClasses.h"
	#include "EngineSoundClasses.h"
	#include "EngineInterpolationClasses.h"
	#include "EngineParticleClasses.h"
	#include "EngineAnimClasses.h"
	#include "EngineDecalClasses.h"
	#include "EngineFogVolumeClasses.h"
	#include "UnFracturedStaticMesh.h"
	#include "EngineMeshClasses.h"
	#include "EnginePrefabClasses.h"
	#include "EngineUserInterfaceClasses.h"
	#include "EngineUIPrivateClasses.h"
	#include "EngineUISequenceClasses.h"
	#include "EngineSceneClasses.h"
	#include "EngineFoliageClasses.h"
	#include "EngineSpeedTreeClasses.h"
	#include "EngineLensFlareClasses.h"
	#include "EngineFluidClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

// Register input keys.
#define DEFINE_KEY(Name, Unused) FName KEY_##Name;
#include "UnKeys.h"
#undef DEFINE_KEY

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

/** Global engine pointer. */
UEngine*		GEngine			= NULL;
/** TRUE if we allow shadow volumes. FALSE will strip out any vertex data needed for shadow volumes (less memory) */
UBOOL			GAllowShadowVolumes = TRUE;

#if !FINAL_RELEASE
/** TRUE if we debug material names with SCOPED_DRAW_EVENT. Toggle with "ShowMaterialDrawEvents" console command. */
UBOOL			GShowMaterialDrawEvents = FALSE;
#endif

/** Global GamePatchHelper object that a game can override */
FGamePatchHelper* GGamePatchHelper = NULL;

IMPLEMENT_COMPARE_CONSTREF( FString, UnEngine, { return appStricmp(*A,*B); } )

/*-----------------------------------------------------------------------------
	Object class implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UEngine);
IMPLEMENT_CLASS(UGameEngine);
IMPLEMENT_CLASS(UDebugManager);
IMPLEMENT_CLASS(USaveGameSummary);
IMPLEMENT_CLASS(UObjectReferencer);

/*-----------------------------------------------------------------------------
	Engine init and exit.
-----------------------------------------------------------------------------*/

//
// Construct the engine.
//
UEngine::UEngine()
{
}

//
// Init class.
//
void UEngine::StaticConstructor()
{
}

//
// Initialize the engine.
//
void UEngine::Init()
{
	// Add input key names.
	#define DEFINE_KEY(Name, Unused) KEY_##Name = FName(TEXT(#Name));
	#include "UnKeys.h"
	#undef DEFINE_KEY

	// Subsystems.
	FURL::StaticInit();
	
	// Initialize random number generator.
	if( GIsBenchmarking || ParseParam(appCmdLine(),TEXT("FIXEDSEED")) )
	{
		appRandInit( 0 );
		appSRandInit( 0 );
	}
	else
	{
		appRandInit( appCycles() );
		appSRandInit( appCycles() );
	}

	// Add to root.
	AddToRoot();

	if( !GIsUCCMake )
	{	
		// Make sure the engine is loaded, because some classes need GEngine->DefaultMaterial to be valid
		LoadObject<UClass>(UEngine::StaticClass()->GetOuter(), *UEngine::StaticClass()->GetName(), NULL, LOAD_Quiet|LOAD_NoWarn, NULL );
		// This reads the Engine.ini file to get the proper DefaultMaterial, etc.
		LoadConfig();
		// Initialize engine's object references.
		InitializeObjectReferences();

		// Ensure all native classes are loaded.
		UObject::BeginLoad();
		for( TObjectIterator<UClass> It; It; ++It )
		{
			UClass* Class = *It;
			if( !Class->GetLinker() )
			{
				LoadObject<UClass>( Class->GetOuter(), *Class->GetName(), NULL, LOAD_Quiet|LOAD_NoWarn, NULL );
			}
		}
		UObject::EndLoad();

		for ( TObjectIterator<UClass> It; It; ++It )
		{
			It->ConditionalLink();
		}

		// Create debug manager object
		DebugManager = ConstructObject<UDebugManager>( UDebugManager::StaticClass() );
#if !CONSOLE && defined(_MSC_VER)
		// Create debug exec.	
		GDebugToolExec = new FDebugToolExec;
#endif		

#if USING_REMOTECONTROL
		// Create RemoteControl.
		if ( !GIsUCC )
		{
			RegisterCoreRemoteControlPages();
			RemoteControlExec = new FRemoteControlExec;
		}
#endif

		// Create the global chart-drawing object.
		GStatChart = new FStatChart();
	}

	if( GIsEditor && !GIsUCCMake )
	{
		UWorld::CreateNew();
	}

	debugf( NAME_Init, TEXT("UEngine initialized") );
}

/**
 * Loads a special material and verifies that it is marked as a special material (some shaders
 * will only be compiled for materials marked as "special engine material")
 *
 * @param MaterialName Fully qualified name of a material to load/find
 * @param Material Reference to a material object pointer that will be filled out
 * @param bCheckUsage Check if the material has been marked to be used as a special engine material
 */
void LoadSpecialMaterial(const FString& MaterialName, UMaterial*& Material, UBOOL bCheckUsage)
{
	// only bother with materials that aren't already loaded
	if (Material == NULL)
	{
		// find or load the object
		Material = LoadObject<UMaterial>(NULL, *MaterialName, NULL, LOAD_None, NULL);	

		if (!Material)
		{
#if CONSOLE
			debugf(TEXT("ERROR: Failed to load special material '%s'. This will probably have bad consequences (depending on its use)"), *MaterialName);
#else
			appErrorf(TEXT("Failed to load special material '%s'"), *MaterialName);
#endif
		}
		// if the material wasn't marked as being a special engine material, then not all of the shaders 
		// will have been compiled on it by this point, so we need to compile them and alert the use
		// to set the bit
		else if (!Material->bUsedAsSpecialEngineMaterial && bCheckUsage) 
		{
#if CONSOLE
			// consoles must have the flag set properly in the editor
			appErrorf(TEXT("The special material (%s) was not marked with bUsedAsSpecialEngineMaterial. Make sure this flag is set in the editor, save the package, and compile shaders for this platform"), *MaterialName);
#else
			Material->bUsedAsSpecialEngineMaterial = TRUE;
			Material->MarkPackageDirty();

			// make sure all necessary shaders for the default are compiled, now that the flag is set
			Material->CacheResourceShaders(GRHIShaderPlatform);

			appMsgf(AMT_OK, TEXT("The special material (%s) has not been marked with bUsedAsSpecialEngineMaterial.\nThis will prevent shader precompiling properly, so the flag has been set automatically.\nMake sure to save the package and distribute to everyone using this material."), *MaterialName);
#endif
		}
	}
}
/**
 * Loads all Engine object references from their corresponding config entries.
 */
void UEngine::InitializeObjectReferences()
{
	if (DefaultMaterialName.Len() == 0)
	{
		appErrorf(TEXT("Invalid DefaultMaterialName! Make sure that script was compiled sucessfully, especially if running with -unattended."));
	}
	if (DefaultDecalMaterialName.Len() == 0)
	{
		appErrorf(TEXT("Invalid DefaultDecalMaterialName! Make sure that script was compiled sucessfully, especially if running with -unattended."));
	}

	// initialize the special engine/editor materials
	LoadSpecialMaterial(DefaultMaterialName, DefaultMaterial, TRUE);
	LoadSpecialMaterial(DefaultDecalMaterialName, DefaultDecalMaterial, TRUE);
	LoadSpecialMaterial(WireframeMaterialName, WireframeMaterial, TRUE);
	LoadSpecialMaterial(EmissiveTexturedMaterialName, EmissiveTexturedMaterial, FALSE);
	LoadSpecialMaterial(LevelColorationLitMaterialName, LevelColorationLitMaterial, TRUE);
	LoadSpecialMaterial(LevelColorationUnlitMaterialName, LevelColorationUnlitMaterial, TRUE);
	LoadSpecialMaterial(ShadedLevelColorationLitMaterialName, ShadedLevelColorationLitMaterial, TRUE);
	LoadSpecialMaterial(ShadedLevelColorationUnlitMaterialName, ShadedLevelColorationUnlitMaterial, TRUE);
	LoadSpecialMaterial(SceneCaptureReflectActorMaterialName, SceneCaptureReflectActorMaterial, FALSE);
	LoadSpecialMaterial(SceneCaptureCubeActorMaterialName, SceneCaptureCubeActorMaterial, FALSE);
	LoadSpecialMaterial(TerrainCollisionMaterialName, TerrainCollisionMaterial, TRUE);
	LoadSpecialMaterial(TerrainErrorMaterialName, TerrainErrorMaterial, TRUE);
	LoadSpecialMaterial(DefaultFogVolumeMaterialName, DefaultFogVolumeMaterial, FALSE);
	LoadSpecialMaterial(DefaultUICaretMaterialName, DefaultUICaretMaterial, FALSE);
	LoadSpecialMaterial(VertexColorMaterialName, VertexColorMaterial, FALSE);

	if (GIsEditor == TRUE)
	{
		LoadSpecialMaterial(GeomMaterialName, GeomMaterial, FALSE);
		LoadSpecialMaterial(TickMaterialName, TickMaterial, FALSE);
		LoadSpecialMaterial(CrossMaterialName, CrossMaterial, FALSE);
		LoadSpecialMaterial(EditorBrushMaterialName, EditorBrushMaterial, FALSE);

		if ( !RemoveSurfaceMaterial )
		{
			RemoveSurfaceMaterial = LoadObject<UMaterial>(NULL, *RemoveSurfaceMaterialName, NULL, LOAD_None, NULL);	
		}
	}
	else
	{
		LoadSpecialMaterial(DefaultMaterialName, GeomMaterial, FALSE);
		LoadSpecialMaterial(DefaultMaterialName, TickMaterial, FALSE);
		LoadSpecialMaterial(DefaultMaterialName, CrossMaterial, FALSE);
		LoadSpecialMaterial(DefaultMaterialName, EditorBrushMaterial, FALSE);
	}

	if( DefaultTexture == NULL )
	{
		DefaultTexture = LoadObject<UTexture2D>(NULL, *DefaultTextureName, NULL, LOAD_None, NULL);	
	}

	if( RandomAngleTexture == NULL )
	{
		RandomAngleTexture = LoadObject<UTexture2D>(NULL, *RandomAngleTextureName, NULL, LOAD_None, NULL);	
	}

	if( RandomNormalTexture == NULL )
	{
		RandomNormalTexture = LoadObject<UTexture2D>(NULL, *RandomNormalTextureName, NULL, LOAD_None, NULL);	
	}
	
	if( WeightMapPlaceholderTexture == NULL )
	{
		WeightMapPlaceholderTexture = LoadObject<UTexture2D>(NULL, *WeightMapPlaceholderTextureName, NULL, LOAD_None, NULL);	
	}

	if ( DefaultPhysMaterial == NULL )
	{
		DefaultPhysMaterial = LoadObject<UPhysicalMaterial>(NULL, *DefaultPhysMaterialName, NULL, LOAD_None, NULL);	
	}
	if ( ConsoleClass == NULL )
	{
		ConsoleClass = LoadClass<UConsole>(NULL, *ConsoleClassName, NULL, LOAD_None, NULL);
	}

	if ( GameViewportClientClass == NULL )
	{
		GameViewportClientClass = LoadClass<UGameViewportClient>(NULL, *GameViewportClientClassName, NULL, LOAD_None, NULL);
	}

	if ( LocalPlayerClass == NULL )
	{
		LocalPlayerClass = LoadClass<ULocalPlayer>(NULL, *LocalPlayerClassName, NULL, LOAD_None, NULL);
	}

	if ( DataStoreClientClass == NULL )
	{
		DataStoreClientClass = LoadClass<UDataStoreClient>(NULL, *DataStoreClientClassName, NULL, LOAD_None, NULL);
	}

#if STORAGE_MANAGER_IMPLEMENTED
	if ( StorageDeviceManagerClass == NULL )
	{
		StorageDeviceManagerClass = LoadClass<UStorageDeviceManager>(NULL, *StorageDeviceManagerClassName, NULL, LOAD_None, NULL);
	}
#endif

#if WITH_UE3_NETWORKING
	if (OnlineSubsystemClass == NULL && DefaultOnlineSubsystemName.Len())
	{
		OnlineSubsystemClass = LoadClass<UOnlineSubsystem>(NULL, *DefaultOnlineSubsystemName, NULL, LOAD_None, NULL);
	}
#endif	//#if WITH_UE3_NETWORKING

	// Load the default engine post process chain used for the game and main editor view
	if( DefaultPostProcess == NULL && DefaultPostProcessName.Len() )
	{
		DefaultPostProcess = LoadObject<UPostProcessChain>(NULL,*DefaultPostProcessName,NULL,LOAD_None,NULL);
	}

	if (GIsEditor == TRUE)
	{
		// Load the post process chain used for skeletal mesh thumbnails
		if( ThumbnailSkeletalMeshPostProcess == NULL && ThumbnailSkeletalMeshPostProcessName.Len() )
		{
			ThumbnailSkeletalMeshPostProcess = LoadObject<UPostProcessChain>(NULL,*ThumbnailSkeletalMeshPostProcessName,NULL,LOAD_None,NULL);
		}
		// Load the post process chain used for particle system thumbnails
		if( ThumbnailParticleSystemPostProcess == NULL && ThumbnailParticleSystemPostProcessName.Len() )
		{
			ThumbnailParticleSystemPostProcess = LoadObject<UPostProcessChain>(NULL,*ThumbnailParticleSystemPostProcessName,NULL,LOAD_None,NULL);
		}
		// Load the post process chain used for material thumbnails
		if( ThumbnailMaterialPostProcess == NULL && ThumbnailMaterialPostProcessName.Len() )
		{
			ThumbnailMaterialPostProcess = LoadObject<UPostProcessChain>(NULL,*ThumbnailMaterialPostProcessName,NULL,LOAD_None,NULL);
		}
	}
	else
	{
		// Load the post process chain used for skeletal mesh thumbnails
		if( ThumbnailSkeletalMeshPostProcess == NULL && DefaultPostProcessName.Len() )
		{
			ThumbnailSkeletalMeshPostProcess = LoadObject<UPostProcessChain>(NULL,*DefaultPostProcessName,NULL,LOAD_None,NULL);
		}
		// Load the post process chain used for particle system thumbnails
		if( ThumbnailParticleSystemPostProcess == NULL && DefaultPostProcessName.Len() )
		{
			ThumbnailParticleSystemPostProcess = LoadObject<UPostProcessChain>(NULL,*DefaultPostProcessName,NULL,LOAD_None,NULL);
		}
		// Load the post process chain used for material thumbnails
		if( ThumbnailMaterialPostProcess == NULL && DefaultPostProcessName.Len() )
		{
			ThumbnailMaterialPostProcess = LoadObject<UPostProcessChain>(NULL,*DefaultPostProcessName,NULL,LOAD_None,NULL);
		}
	}

	// Load the default UI post process chain
	if( DefaultUIScenePostProcess == NULL && DefaultUIScenePostProcessName.Len() )
	{
		DefaultUIScenePostProcess = LoadObject<UPostProcessChain>(NULL,*DefaultUIScenePostProcessName,NULL,LOAD_None,NULL);
	}

	if( DefaultSound == NULL && DefaultSoundName.Len() )
	{
		DefaultSound = LoadObject<USoundNodeWave>( NULL, *DefaultSoundName, NULL, LOAD_None, NULL );
	}

	// set the font object pointers
	if( TinyFont == NULL && TinyFontName.Len() )
	{
		TinyFont = LoadObject<UFont>(NULL,*TinyFontName,NULL,LOAD_None,NULL);
	}
	if( SmallFont == NULL && SmallFontName.Len() )
	{
		SmallFont = LoadObject<UFont>(NULL,*SmallFontName,NULL,LOAD_None,NULL);
	}
	if( MediumFont == NULL && MediumFontName.Len() )
	{
		MediumFont = LoadObject<UFont>(NULL,*MediumFontName,NULL,LOAD_None,NULL);
	}
	if( LargeFont == NULL && LargeFontName.Len() )
	{
		LargeFont = LoadObject<UFont>(NULL,*LargeFontName,NULL,LOAD_None,NULL);
	}
	if( SubtitleFont == NULL && SubtitleFontName.Len() )
	{
		SubtitleFont = LoadObject<UFont>(NULL,*SubtitleFontName,NULL,LOAD_None,NULL);
	}

	// Additional fonts.
	AdditionalFonts.Empty( AdditionalFontNames.Num() );
	for ( INT FontIndex = 0 ; FontIndex < AdditionalFontNames.Num() ; ++FontIndex )
	{
		const FString& FontName = AdditionalFontNames(FontIndex);
		UFont* NewFont = NULL;
		if( FontName.Len() )
		{
			NewFont = LoadObject<UFont>(NULL,*FontName,NULL,LOAD_None,NULL);
		}
		AdditionalFonts.AddItem( NewFont );
	}
}

//
// Exit the engine.
//
void UEngine::FinishDestroy()
{
	// Remove from root.
	RemoveFromRoot();

	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		// Clean up debug tool.
		delete GDebugToolExec;
		GDebugToolExec		= NULL;

#if USING_REMOTECONTROL
		// Clean up RemoteControl.
		delete RemoteControlExec;
		RemoteControlExec		= NULL;
#endif

		// Shut down all subsystems.
		Client				= NULL;
		GEngine				= NULL;
		FURL::StaticExit();

		delete GStatChart;
		GStatChart			= NULL;
	}
	Super::FinishDestroy();
}

//
// Progress indicator.
//
void UEngine::SetProgress( EProgressMessageType MessageType, const FString& Title, const FString& Message )
{
}

void UEngine::CleanupGameViewport()
{
	// Clean up the viewports that have been closed.
	for(FPlayerIterator It(this);It;++It)
	{
		if(It->ViewportClient && !It->ViewportClient->Viewport)
		{
			It->ViewportClient = NULL;
			It.RemoveCurrent();
		}
	}

	if ( GameViewport != NULL && GameViewport->Viewport == NULL )
	{
		GameViewport->DetachViewportClient();
		GameViewport = NULL;
	}
}

FViewport* UEngine::GetAViewport()
{
	if(GameViewport)
	{
		return GameViewport->Viewport;
	}

	return NULL;
}

/**
 * Returns a pointer to the current world.
 */
AWorldInfo* UEngine::GetCurrentWorldInfo()
{
	return GWorld->GetWorldInfo();
}

/**
 * Returns version info from the engine
 */
FString UEngine::GetBuildDate( void )
{
	FString BuildDate = ANSI_TO_TCHAR( __DATE__ );
	return BuildDate;
}

/**
 * Returns the engine's default tiny font
 */
UFont* UEngine::GetTinyFont()
{
	return GEngine->TinyFont;
}

/**
 * Returns the engine's default small font
 */
UFont* UEngine::GetSmallFont()
{
	return GEngine->SmallFont;
}

/**
 * Returns the engine's default medium font
 */
UFont* UEngine::GetMediumFont()
{
	return GEngine->MediumFont;
}

/**
 * Returns the engine's default large font
 */
UFont* UEngine::GetLargeFont()
{
	return GEngine->LargeFont;
}

/**
 * Returns the engine's default subtitle font
 */
UFont* UEngine::GetSubtitleFont()
{
	return GEngine->SubtitleFont;
}

/**
 * Returns the specified additional font.
 *
 * @param	AdditionalFontIndex		Index into the AddtionalFonts array.
 */
UFont* UEngine::GetAdditionalFont(INT AdditionalFontIndex)
{
	return GEngine->AdditionalFonts(AdditionalFontIndex);
}

/** @return whether we're currently running in splitscreen (more than one local player) */
UBOOL UEngine::IsSplitScreen()
{
	return (GEngine->GamePlayers.Num() > 1);
}

/** @return the audio device (will be None if sound is disabled) */
UAudioDevice* UEngine::GetAudioDevice()
{
	return (GEngine->Client != NULL) ? GEngine->Client->GetAudioDevice() : NULL;
}

/** @return Returns the name of the last movie that was played. */
FString UEngine::GetLastMovieName()
{
	if(GFullScreenMovie)
	{
		return GFullScreenMovie->GameThreadGetLastMovieName();
	}

	return "";
}

/**
 * Play one of the LoadMap loading movies as configured by ini file
 */
UBOOL UEngine::PlayLoadMapMovie()
{
	// don't try to load a movie if one is already going
	UBOOL bStartedLoadMapMovie=FALSE;
	if (GFullScreenMovie && !GFullScreenMovie->GameThreadIsMoviePlaying(TEXT("")))
	{
		// potentially load a movie here
		TMultiMap<FString,FString>* MovieIni = GConfig->GetSectionPrivate(TEXT("FullScreenMovie"), FALSE, TRUE, GEngineIni);
		if (MovieIni)
		{
			TArray<FString> LoadMapMovies;
			// find all the loadmap movie possibilities
			for (TMultiMap<FString,FString>::TIterator It(*MovieIni); It; ++It)
			{
				if (It.Key() == TEXT("LoadMapMovies"))
				{
					LoadMapMovies.AddItem(It.Value());
				}
			}

			// load a random mobvie from the list, if there were any
			if (LoadMapMovies.Num() != 0)
			{
				GFullScreenMovie->GameThreadPlayMovie(MM_LoopFromMemory, *LoadMapMovies(appRand() % LoadMapMovies.Num()));
				// keep track of starting playback for loadmap so it can be stopped
				bStartedLoadMapMovie=TRUE;
			}
		}
	}

	// return if we started or not
	return bStartedLoadMapMovie;
}

/**
 * Stops the current movie
 *
 * @param bDelayStopUntilGameHasRendered If TRUE, the engine will delay stopping the movie until after the game has rendered at least one frame
 */
void UEngine::StopMovie(UBOOL bDelayStopUntilGameHasRendered)
{
	// delay if desired
	if (bDelayStopUntilGameHasRendered)
	{
		GFullScreenMovie->GameThreadRequestDelayedStopMovie();
	}
	else
	{
		GFullScreenMovie->GameThreadStopMovie(0, FALSE, TRUE);
	}
}

/**
 * Removes all overlays from displaying
 */
void UEngine::RemoveAllOverlays()
{
	GFullScreenMovie->GameThreadRemoveAllOverlays();
}

/**
 * Adds a text overlay to the movie
 *
 * @param Font Font to use to display (must be in the root set so this will work during loads)
 * @param Text Text to display
 * @param X X location in resolution-independent coordinates (ignored if centered)
 * @param Y Y location in resolution-independent coordinates
 * @param ScaleX Text horizontal scale
 * @param ScaleY Text vertical scale
 * @param bIsCentered TRUE if the text should be centered
 */
void UEngine::AddOverlay( UFont* Font, const FString& Text, FLOAT X, FLOAT Y, FLOAT ScaleX, FLOAT ScaleY, UBOOL bIsCentered )
{
	GFullScreenMovie->GameThreadAddOverlay( Font, Text, X, Y, ScaleX, ScaleY, bIsCentered, FALSE, 0 );
}


/**
 * Adds a wrapped text overlay to the movie
 *
 * @param Font Font to use to display (must be in the root set so this will work during loads)
 * @param Text Text to display
 * @param X X location in resolution-independent coordinates (ignored if centered)
 * @param Y Y location in resolution-independent coordinates
 * @param ScaleX Text horizontal scale
 * @param ScaleY Text vertical scale
 * @param WrapWidth Width before text is wrapped to the next line
 */
void UEngine::AddOverlayWrapped( UFont* Font, const FString& Text, FLOAT X, FLOAT Y, FLOAT ScaleX, FLOAT ScaleY, FLOAT WrapWidth )
{
	GFullScreenMovie->GameThreadAddOverlay( Font, Text, X, Y, ScaleX, ScaleY, FALSE, TRUE, WrapWidth );
}

/*-----------------------------------------------------------------------------
	Input.
-----------------------------------------------------------------------------*/
/**
 * Helper structure for sorting textures by relative cost.
 */
struct FSortedTexture 
{
 	INT		OrigSizeX;
 	INT		OrigSizeY;
	INT		CookedSizeX;
	INT		CookedSizeY;
	INT		CurSizeX;
	INT		CurSizeY;
 	INT		LODBias;
	INT		MaxSize;
 	INT		CurrentSize;
	FString Name;
 	INT		LODGroup;
 	UBOOL	bIsStreaming;

	/** Constructor, initializing every member variable with passed in values. */
	FSortedTexture(	INT InOrigSizeX, INT InOrigSizeY, INT InCookedSizeX, INT InCookedSizeY, INT InCurSizeX, INT InCurSizeY, INT InLODBias, INT InMaxSize, INT InCurrentSize, const FString& InName, INT InLODGroup, UBOOL bInIsStreaming )
	:	OrigSizeX( InOrigSizeX )
	,	OrigSizeY( InOrigSizeY )
	,	CookedSizeX( InCookedSizeX )
	,	CookedSizeY( InCookedSizeY )
	,	CurSizeX( InCurSizeX )
	,	CurSizeY( InCurSizeY )
 	,	LODBias( InLODBias )
	,	MaxSize( InMaxSize )
	,	CurrentSize( InCurrentSize )
	,	Name( InName )
 	,	LODGroup( InLODGroup )
 	,	bIsStreaming( bInIsStreaming )
	{}
};
static UBOOL bAlphaSort = FALSE;
IMPLEMENT_COMPARE_CONSTREF( FSortedTexture, UnPlayer, { return bAlphaSort ? appStricmp(*A.Name,*B.Name) : B.MaxSize - A.MaxSize; } )

/** Helper struct for sorting anim sets by size */
struct FSortedAnimSet
{
	FString Name;
	INT		Size;

	FSortedAnimSet( const FString& InName, INT InSize )
	:	Name(InName)
	,	Size(InSize)
	{}
};
IMPLEMENT_COMPARE_CONSTREF( FSortedAnimSet, UnPlayer, { return bAlphaSort ? appStricmp(*A.Name,*B.Name) : ( A.Size > B.Size ) ? -1 : 1; } );


/** Helper compare function for the SHOWHOTKISMET exec */
IMPLEMENT_COMPARE_POINTER(USequenceOp, UnPlayer, { return(B->ActivateCount - A->ActivateCount); } );

/** Sort actors by name. */
IMPLEMENT_COMPARE_POINTER( AActor, UnPlayer, { return( appStricmp( *A->GetName(), *B->GetName() ) ); } );


static void ShowSubobjectGraph( FOutputDevice& Ar, UObject* CurrentObject, const FString& IndentString )
{
	if ( CurrentObject == NULL )
	{
		Ar.Logf(TEXT("%sX NULL"), *IndentString);
	}
	else
	{
		TArray<UObject*> ReferencedObjs;
		FArchiveObjectReferenceCollector RefCollector(&ReferencedObjs, CurrentObject, TRUE, FALSE, FALSE, FALSE);
		CurrentObject->Serialize(RefCollector);

		if ( ReferencedObjs.Num() == 0 )
		{
			Ar.Logf(TEXT("%s. %s"), *IndentString, IndentString.Len() == 0 ? *CurrentObject->GetPathName() : *CurrentObject->GetName());
		}
		else
		{
			Ar.Logf(TEXT("%s+ %s"), *IndentString, IndentString.Len() == 0 ? *CurrentObject->GetPathName() : *CurrentObject->GetName());
			for ( INT ObjIndex = 0; ObjIndex < ReferencedObjs.Num(); ObjIndex++ )
			{
				ShowSubobjectGraph(Ar, ReferencedObjs(ObjIndex), IndentString + TEXT("|\t"));
			}
		}
	}
}

//
// This always going to be the last exec handler in the chain. It
// handles passing the command to all other global handlers.
//
UBOOL UEngine::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	// See if any other subsystems claim the command.
	if( GSys				&&	GSys->Exec			(Cmd,Ar) ) return TRUE;
	if(	UObject::StaticExec							(Cmd,Ar) ) return TRUE;
	if( Client				&&	Client->Exec		(Cmd,Ar) ) return TRUE;
	if( GDebugToolExec		&&	GDebugToolExec->Exec(Cmd,Ar) ) return TRUE;
	if( GStatChart			&&	GStatChart->Exec	(Cmd,Ar) ) return TRUE;
	if( GMalloc				&&	GMalloc->Exec		(Cmd,Ar) ) return TRUE;
	if(	GObjectPropagator->Exec						(Cmd,Ar) ) return TRUE;
	if( GSystemSettings.Exec						(Cmd,Ar) ) return TRUE;
#if USING_REMOTECONTROL
	if( RemoteControlExec	&&	RemoteControlExec->Exec	(Cmd,Ar) ) return TRUE;
#endif

	// Handle engine command line.
	if( ParseCommand(&Cmd,TEXT("SHOWLOG")) )
	{
		// Toggle display of console log window.
		if( GLogConsole )
		{
			GLogConsole->Show( !GLogConsole->IsShown() );
		}
		return 1;
	}

	else if ( ParseCommand(&Cmd,TEXT("PerfMem_Memory")) )
	{
		//PerfMem MemoryDatum( FVector(), FRotator() );
		//MemoryDatum.AddMemoryStatsForLocationRotation();
		//warnf( TEXT("%s"), *MemoryDatum.GetAlterTableColumnsSQL_StatSceneRendering() );
		//warnf( TEXT("%s"), *MemoryDatum.GetStoredProcedureText() );

		//PerfMem PerfDatum( FVector(), FRotator() );
		//PerfDatum.WriteInsertSQLToBatFile();
		//PerfDatum.AddPerfStatsForLocationRotation();


		return TRUE;
	}

	else if( ParseCommand(&Cmd, TEXT("GAMEVER")) ||  ParseCommand(&Cmd, TEXT("GAMEVERSION")))
	{
		Ar.Logf( TEXT("GEngineVersion:  %d  GBuiltFromChangeList:  %d"), GEngineVersion, GBuiltFromChangeList );
		return 1;
	}
	else if( ParseCommand(&Cmd,TEXT("CRACKURL")) )
	{
		FURL URL(NULL,Cmd,TRAVEL_Absolute);
		if( URL.Valid )
		{
			Ar.Logf( TEXT("     Protocol: %s"), *URL.Protocol );
			Ar.Logf( TEXT("         Host: %s"), *URL.Host );
			Ar.Logf( TEXT("         Port: %i"), URL.Port );
			Ar.Logf( TEXT("          Map: %s"), *URL.Map );
			Ar.Logf( TEXT("   NumOptions: %i"), URL.Op.Num() );
			for( INT i=0; i<URL.Op.Num(); i++ )
				Ar.Logf( TEXT("     Option %i: %s"), i, *URL.Op(i) );
			Ar.Logf( TEXT("       Portal: %s"), *URL.Portal );
			Ar.Logf( TEXT("       String: '%s'"), *URL.String() );
		}
		else Ar.Logf( TEXT("BAD URL") );
		return 1;
	}
	else if( ParseCommand(&Cmd,TEXT("DEFER")) )
	{
		new(DeferredCommands)FString(Cmd);
		return 1;
	}
	else if( ParseCommand(&Cmd,TEXT("ToggleRenderingThread")) )
	{
		if(GIsThreadedRendering)
		{
			StopRenderingThread();
			GUseThreadedRendering = FALSE;
		}
		else
		{
			GUseThreadedRendering = TRUE;
			StartRenderingThread();
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("RecompileShaders")) )
	{
		class FTestTimer
		{
		public:
			FTestTimer(const TCHAR* InInfoStr=TEXT("Test"))
				: InfoStr(InInfoStr),
				bAlreadyStopped(FALSE)
			{
                StartTime = appSeconds();
			}

			FTestTimer(FString InInfoStr)
				: InfoStr(InInfoStr),
				bAlreadyStopped(FALSE)
			{
				StartTime = appSeconds();
			}

			void Stop(UBOOL DisplayLog = TRUE)
			{
				if (!bAlreadyStopped)
				{
					bAlreadyStopped = TRUE;
					EndTime = appSeconds();
					TimeElapsed = EndTime-StartTime;
					if (DisplayLog)
					{
						warnf(TEXT("		[%s] took [%.4f] s"),*InfoStr,TimeElapsed);
					}
				}
			}

			~FTestTimer()
			{
				Stop(TRUE);
			}

			DOUBLE StartTime,EndTime;
			DOUBLE TimeElapsed;
			FString InfoStr;
			UBOOL bAlreadyStopped;

		};

		FString FlagStr(ParseToken(Cmd, 0));
		if( FlagStr.Len() > 0 )
		{
			FlushShaderFileCache();
			FlushRenderingCommands();

			if( appStricmp(*FlagStr,TEXT("Changed"))==0)
			{
				TArray<FShaderType*> OutdatedShaderTypes;
				TArray<FVertexFactoryType*> OutdatedVFTypes;
				
				{
					FTestTimer SearchTimer(TEXT("Searching for changed files"));
					FShaderType::GetOutdatedTypes(OutdatedShaderTypes);
					FVertexFactoryType::GetOutdatedTypes(OutdatedVFTypes);
				}

				if (OutdatedShaderTypes.Num() > 0 || OutdatedVFTypes.Num() > 0)
				{
					FTestTimer TestTimer(TEXT("RecompileShaders Changed"));
					UMaterial::UpdateMaterialShaders(OutdatedShaderTypes, OutdatedVFTypes);
					RecompileGlobalShaders(OutdatedShaderTypes);
				}
				else
				{
					warnf(TEXT("No Shader changes found."));
				}
			}
			else if( appStricmp(*FlagStr,TEXT("Shadow"))==0)
			{
				FTestTimer TestTimer(TEXT("RecompileShaders Shadow"));
				RecompileGlobalShaderGroup(SRG_GLOBAL_MISC_SHADOW);
			}
			else if( appStricmp(*FlagStr,TEXT("BPCF"))==0)
			{
				FTestTimer TestTimer(TEXT("RecompileShaders BPCF"));
				RecompileGlobalShaderGroup(SRG_GLOBAL_BPCF_SHADOW_LOW);
			}
			else if( appStricmp(*FlagStr,TEXT("GlobalMisc"))==0)
			{
				FTestTimer TestTimer(TEXT("RecompileShaders GlobalMisc"));
				RecompileGlobalShaderGroup(SRG_GLOBAL_MISC);
			}
			else if( appStricmp(*FlagStr,TEXT("Global"))==0)
			{
				FTestTimer TestTimer(TEXT("RecompileShaders Global"));
				RecompileGlobalShaders();
			}
			else if( appStricmp(*FlagStr,TEXT("VF"))==0)
			{
				FString RequestedVertexFactoryName(ParseToken(Cmd, 0));

				FVertexFactoryType* FoundType = FVertexFactoryType::GetVFByName(RequestedVertexFactoryName);
				
				if (!FoundType)
				{
					warnf( TEXT("Couldn't find Vertex Factory %s! \nExisting Vertex Factories:"), *RequestedVertexFactoryName);
					for(TLinkedList<FVertexFactoryType*>::TIterator It(FVertexFactoryType::GetTypeList()); It; It.Next())
					{
						const TCHAR* VertexFactoryTypeName = It->GetName();
						warnf( TEXT("%s"), VertexFactoryTypeName);
					}
					return 1;
				}

				FTestTimer TestTimer(TEXT("RecompileShaders VertexFactory"));
				TArray<FShaderType*> OutdatedShaderTypes;
				TArray<FVertexFactoryType*> OutdatedVFTypes;
				OutdatedVFTypes.Push(FoundType);

				UMaterial::UpdateMaterialShaders(OutdatedShaderTypes, OutdatedVFTypes);
			}
			else if( appStricmp(*FlagStr,TEXT("Material"))==0)
			{
				FString RequestedMaterialName(ParseToken(Cmd, 0));
				FTestTimer TestTimer(FString::Printf(TEXT("Recompile Material %s"), *RequestedMaterialName));
				UBOOL bMaterialFound = FALSE;
				for( TObjectIterator<UMaterial> It; It; ++It )
				{
					UMaterial* Material = *It;
					if( Material && Material->GetName() == RequestedMaterialName)
					{
						bMaterialFound = TRUE;
						// <Pre/Post>EditChange will force a re-creation of the resource,
						// in turn recompiling the shader.
						Material->PreEditChange(NULL);
						Material->PostEditChange(NULL);
						break;
					}
				}

				if (!bMaterialFound)
				{
					TestTimer.Stop(FALSE);
					warnf(TEXT("Couldn't find Material %s!"), *RequestedMaterialName);
				}
			}
			else if( appStricmp(*FlagStr,TEXT("MaterialShaderType"))==0)
			{
				FString RequestedShaderTypeName(ParseToken(Cmd, 0));

				FShaderType* FoundType = FMeshMaterialShaderType::GetTypeByName(RequestedShaderTypeName);

				if (!FoundType)
				{
					FoundType = FMaterialShaderType::GetTypeByName(RequestedShaderTypeName);
				}

				if (!FoundType)
				{
					warnf( TEXT("Couldn't find Shader Type %s! \nExisting Shader Types:"), *RequestedShaderTypeName);
					for(TLinkedList<FShaderType*>::TIterator It(FShaderType::GetTypeList()); It; It.Next())
					{
						FMaterialShaderType* MaterialShaderType = It->GetMaterialShaderType();
						FMeshMaterialShaderType* MeshMaterialShaderType = It->GetMeshMaterialShaderType();
						if (MaterialShaderType || MeshMaterialShaderType)
						{
							const TCHAR* ShaderTypeName = It->GetName();
							warnf( TEXT("%s"), ShaderTypeName);
						}
					}
					return 1;
				}

				FTestTimer TestTimer(TEXT("RecompileShaders ShaderType"));

				TArray<FShaderType*> OutdatedShaderTypes;
				OutdatedShaderTypes.Push(FoundType);
				TArray<FVertexFactoryType*> OutdatedVFTypes;

				UMaterial::UpdateMaterialShaders(OutdatedShaderTypes, OutdatedVFTypes);
			} 
			else if( appStricmp(*FlagStr,TEXT("All"))==0)
			{
				FTestTimer TestTimer(TEXT("RecompileShaders"));
				RecompileGlobalShaders();
				for( TObjectIterator<UMaterial> It; It; ++It )
				{
					UMaterial* Material = *It;
					if( Material )
					{
						debugf(TEXT("recompiling [%s]"),*Material->GetFullName());

						// <Pre/Post>EditChange will force a re-creation of the resource,
						// in turn recompiling the shader.
						Material->PreEditChange(NULL);
						Material->PostEditChange(NULL);
					}
				}
			}
			
			return 1;
		}

		warnf( TEXT("Invalid parameter. Options are: \n'Changed', 'Shadow', 'BPCF', 'GlobalMisc', 'Global', \n'VF [name]', 'Material [name]', 'MaterialShaderType [name]', 'All'"));
		
		return 1;
	}
	else if( ParseCommand(&Cmd,TEXT("RecompileGlobalShaders")) )
	{
		RecompileGlobalShaders();
		return 1;
	}
	else if( ParseCommand(&Cmd,TEXT("DUMPSHADERSTATS")) )
	{
		DumpShaderStats();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("DISTFACTORSTATS")) )
	{
		DumpDistanceFactorChart();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEAO")) )
	{
		extern UBOOL GRenderAmbientOcclusion;
		FlushRenderingCommands();
		GRenderAmbientOcclusion = !GRenderAmbientOcclusion;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLESCENE")) )
	{
		extern UBOOL GAOCombineWithSceneColor;
		FlushRenderingCommands();
		GAOCombineWithSceneColor = !GAOCombineWithSceneColor;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("rebuilddistro")) )
	{
		for (TObjectIterator<UDistributionFloat> It; It; ++It)
		{
			It->bIsDirty = TRUE;
		}
		for (TObjectIterator<UDistributionVector> It; It; ++It)
		{
			It->bIsDirty = TRUE;
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("DUMPDYNAMICLIGHTSHADOWINTERACTIONS")) )
	{
		GWorld->Scene->DumpDynamicLightShadowInteractions( TRUE );
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("QUERYPERFDB")) )
	{
		// Check whether we have an active DB connection for perf tracking and try to use it if we do.
		FDataBaseConnection* DataBaseConnection = GTaskPerfTracker->GetConnection();
		if( DataBaseConnection )
		{
			// Query string, calling stored procedure on DB that lists aggregrated duration of tasks on this machine
			// grouped by task, sorted by duration.
			FString QueryString = FString(TEXT("EXEC MachineSummary @MachineName=")) + appComputerName();
		
			// Execute the Query. If successful it will fill in the RecordSet and we're responsible for 
			// deleting it.
			FDataBaseRecordSet* RecordSet = NULL;
			if( DataBaseConnection->Execute( *QueryString, RecordSet ) )
			{
				// Success! Iterate over all rows in record set and log them.
				debugf(TEXT("Aggregate duration of tasks tracked on this machine."));
				for( FDataBaseRecordSet::TIterator It( RecordSet ); It; ++It )
				{
					FLOAT	DurationSum = It->GetFloat( TEXT("DurationSum") );
					FString Task		= It->GetString( TEXT("Task") );
					debugf(TEXT("%10.2f min for '%s'"), DurationSum / 60, *Task);
				}

				// Clean up record set now that we are done.
				delete RecordSet;
				RecordSet = NULL;
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TestRemoteDBProxy")) )
	{
		// Check whether we have an active DB connection for perf tracking and try to use it if we do.
		FDataBaseConnection* DataBaseConnection = GTaskPerfMemDatabase->GetConnection();
		if( DataBaseConnection != NULL )
		{
			DataBaseConnection->Open( TEXT("172.23.2.8"), NULL, NULL );
			// Query string, calling stored procedure on DB that lists aggregrated duration of tasks on this machine
			// grouped by task, sorted by duration.
			const FString QueryString = FString (TEXT("EXEC dbo.[AddMapLocationData_Perf] @Platform='Xenon', @LevelName='msewTest2', @BuiltFromChangelist=218869, @RunResult='Passed', @LocX=-26953.707031, @LocY=142715.671875, @LocZ=-10642.504883, @RotYaw=49152, @RotPitch=0, @RotRoll=0, @AverageFPS=75.12, @AverageMS=13.31, @FrameTime=13.29, @Game_thread_time=12.03, @Render_thread_time=1.98, @GPU_time=12.92, @Culled_primitives=0.00, @Occluded_primitives=0.00, @Occlusion_queries=0.00, @Projected_shadows=0.00, @Visible_static_mesh_elements=0.00, @Visible_dynamic_primitives=2.00, @Draw_events=0.00, @Streaming_Textures_Size=19564.00, @Textures_Max_Size=63892.00, @Intermediate_Textures_Size=0.00, @Request_Size_Current_Frame=0.00, @Request_Size_Total=211420.00, @Streaming_fudge_factor=667.00" ) );
			
			DataBaseConnection->Execute( *QueryString );

			warnf( TEXT( "TestRemoteDBProxy EXECUTED!!" ) );
		}

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("RESETFPSCHART")) )
	{
		GSentinelRunID = -1;  // we can do this here as the dumpfpschart / resetfpschart is the process QA uses to get new sets of data
		ResetFPSChart();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("DUMPFPSCHART")) )
	{
		DumpFPSChart(TRUE);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("LISTDYNAMICLEVELS")) )
	{
		debugf(TEXT("Listing dynamic streaming levels for %s"),*GWorld->GetOutermost()->GetName());
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel && !StreamingLevel->bIsFullyStatic ) 
			{
				debugf(TEXT("   %s"),*StreamingLevel->PackageName.ToString());
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TEXTUREDENSITY")) )
	{
		FString MinDensity(ParseToken(Cmd, 0));
		FString IdealDensity(ParseToken(Cmd, 0));
		FString MaxDensity(ParseToken(Cmd, 0));
		GEngine->MinTextureDensity   = ( MinDensity.Len()   > 0 ) ? appAtof(*MinDensity)   : GEngine->MinTextureDensity;
		GEngine->IdealTextureDensity = ( IdealDensity.Len() > 0 ) ? appAtof(*IdealDensity) : GEngine->IdealTextureDensity;
		GEngine->MaxTextureDensity   = ( MaxDensity.Len()   > 0 ) ? appAtof(*MaxDensity)   : GEngine->MaxTextureDensity;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHADERCOMPLEXITY")) )
	{
		FString FlagStr(ParseToken(Cmd, 0));
		if( FlagStr.Len() > 0 )
		{
			if( appStricmp(*FlagStr,TEXT("TOGGLEADDITIVE"))==0)
			{
				GEngine->bUseAdditiveComplexity = !GEngine->bUseAdditiveComplexity;
			}
			else if( appStricmp(*FlagStr,TEXT("TOGGLEPIXEL"))==0)
			{
				GEngine->bUsePixelShaderComplexity = !GEngine->bUsePixelShaderComplexity;
			}
			else if( appStricmp(*FlagStr,TEXT("MAX"))==0)
			{
				FLOAT NewMax = appAtof(Cmd);
				if (NewMax > 0.0f)
				{
					if (GEngine->bUsePixelShaderComplexity)
					{
						if (GEngine->bUseAdditiveComplexity)
						{
							GEngine->MaxPixelShaderAdditiveComplexityCount = NewMax;
						}
						else
						{
							GEngine->MaxPixelShaderOpaqueComplexityCount = NewMax;
						}
					}
					else
					{
						GEngine->MaxVertexShaderComplexityCount = NewMax;
					}
				}
			}
			else
			{
				Ar.Logf( TEXT("Format is 'shadercomplexity [toggleadditive] [togglepixel] [max $int]"));
				return TRUE;
			}

			FLOAT CurrentMax = 0.0f;
			if (GEngine->bUsePixelShaderComplexity)
			{
				CurrentMax = GEngine->bUseAdditiveComplexity ? GEngine->MaxPixelShaderAdditiveComplexityCount : GEngine->MaxPixelShaderOpaqueComplexityCount;
			}
			else
			{
				CurrentMax = GEngine->MaxVertexShaderComplexityCount;
			}

			Ar.Logf( TEXT("New ShaderComplexity Settings: Pixel Complexity = %u, Additive = %u, Max = %f"), GEngine->bUsePixelShaderComplexity, GEngine->bUseAdditiveComplexity, CurrentMax);
		} 
		else
		{
			Ar.Logf( TEXT("Format is 'shadercomplexity [toggleadditive] [togglepixel] [max $int]"));
		}
		return TRUE; 
	}
	else if( ParseCommand(&Cmd, TEXT("TOGGLECOLLISIONOVERLAY")) )
	{
		GEngine->bRenderTerrainCollisionAsOverlay = !(GEngine->bRenderTerrainCollisionAsOverlay);
		debugf(TEXT("Render terrain collision as overlay = %s"),
			GEngine->bRenderTerrainCollisionAsOverlay ? TEXT("TRUE") : TEXT("FALSE"));
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("LISTMISSINGPHYSICALMATERIALS")) )
	{
		// Gather all material (instances) without a physical material association.
		TArray<FString> MaterialNames;
		for(  TObjectIterator<UMaterialInterface> It; It; ++It )
		{
			UMaterialInterface* MaterialInterface = *It;
			if( MaterialInterface->GetPhysicalMaterial() == NULL )
			{
				new(MaterialNames)FString(MaterialInterface->GetFullName());
			}
		}
		if( MaterialNames.Num() )
		{
			// Sort the list lexigraphically.
			Sort<USE_COMPARE_CONSTREF(FString,UnEngine)>( MaterialNames.GetTypedData(), MaterialNames.Num() );
			// Log the names.
			debugf(TEXT("Materials with no associated physical material:"));
			for( INT i=0; i<MaterialNames.Num(); i++ )
			{
				debugf(TEXT("%s"),*MaterialNames(i));
			}
		}
		else
		{
			debugf(TEXT("All materials have physical materials associated."));
		}
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("FULLMOTIONBLUR")))
	{
		extern INT GMotionBlurFullMotionBlur;
		FString Parameter(ParseToken(Cmd, 0));
		GMotionBlurFullMotionBlur = (Parameter.Len() > 0) ? Clamp(appAtoi(*Parameter),-1,1) : -1;
		warnf( TEXT("Motion Blur FullMotionBlur is now set to: %s"),
			(GMotionBlurFullMotionBlur < 0 ? TEXT("DEFAULT") : (GMotionBlurFullMotionBlur > 0 ? TEXT("TRUE") : TEXT("FALSE"))) );
		return TRUE;
	}
#if !FINAL_RELEASE
	else if( ParseCommand(&Cmd,TEXT("FREEZERENDERING")) )
	{
		ProcessToggleFreezeCommand();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("FREEZESTREAMING")) )
	{
		ProcessToggleFreezeStreamingCommand();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("FREEZEALL")) )
	{
		ProcessToggleFreezeCommand();
		ProcessToggleFreezeStreamingCommand();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHOWMATERIALDRAWEVENTS")) )
	{
		GShowMaterialDrawEvents = !GShowMaterialDrawEvents;
		warnf( TEXT("Show material names in SCOPED_DRAW_EVENT: %s"), GShowMaterialDrawEvents ? TEXT("TRUE") : TEXT("FALSE") );
		return TRUE;
	}
#endif
#if TRACK_FILEIO_STATS
	else if( ParseCommand(&Cmd,TEXT("DUMPFILEIOSTATS")) )
	{
		GetFileIOStats()->DumpStats();
		return TRUE;
	}
#endif
	else if( ParseCommand(&Cmd, TEXT("FLUSHIOMANAGER")) )
	{
		GIOManager->Flush();
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("MOVIETEST")) )
	{
		if( GFullScreenMovie )
		{
			FString TestMovieName( ParseToken( Cmd, 0 ) );
			if( TestMovieName.Len() == 0 )
			{
				TestMovieName = TEXT("AttractMode");
			}
			// Play movie and block on playback.
			GFullScreenMovie->GameThreadPlayMovie(MM_PlayOnceFromStream, *TestMovieName);
			GFullScreenMovie->GameThreadWaitForMovie();
			GFullScreenMovie->GameThreadStopMovie();
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("AVAILABLETEXMEM")) )
	{
		FlushRenderingCommands();
		warnf(TEXT("Available texture memory as reported by the RHI: %dMB"), RHIGetAvailableTextureMemory());
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("DUMPAVAILABLERESOLUTIONS")))
	{
		debugf(TEXT("DumpAvailableResolutions"));
		
		FScreenResolutionArray ResArray;
		if (RHIGetAvailableResolutions(ResArray, FALSE))
		{
			for (INT ModeIndex = 0; ModeIndex < ResArray.Num(); ModeIndex++)
			{
				FScreenResolutionRHI& ScreenRes = ResArray(ModeIndex);
				debugf(TEXT("DefaultAdapter - %4d x %4d @ %d"), 
					ScreenRes.Width, ScreenRes.Height, ScreenRes.RefreshRate);
			}
		}
		else
		{
			debugf(TEXT("Failed to get available resolutions!"));
		}
		return TRUE;
	}
	// START: These commands were previously located in LocalPlayer::Exec...
	else if(ParseCommand(&Cmd,TEXT("LISTTEXTURES")))
	{
		const UBOOL bShouldOnlyListStreaming = ParseCommand(&Cmd, TEXT("STREAMING"));
		const UBOOL bShouldOnlyListNonStreaming = ParseCommand(&Cmd, TEXT("NONSTREAMING"));
		bAlphaSort = ParseParam( Cmd, TEXT("ALPHASORT") );

		Ar.Logf( TEXT("Listing %s textures."), bShouldOnlyListNonStreaming ? TEXT("non streaming") : bShouldOnlyListStreaming ? TEXT("streaming") : TEXT("all")  );

		// Traverse streamable list, creating a map of all streamable textures for fast lookup.
		TMap<UTexture2D*,UBOOL> StreamableTextureMap;
		for( TLinkedList<UTexture2D*>::TIterator It(UTexture2D::GetStreamableList()); It; It.Next() )
		{	
			UTexture2D* Texture	= *It;
			StreamableTextureMap.Set( Texture, TRUE );
		}

		// Collect textures.
		TArray<FSortedTexture> SortedTextures;
		for( TObjectIterator<UTexture2D> It; It; ++It )
		{
			UTexture2D*		Texture				= *It;
			INT				LODGroup			= Texture->LODGroup;
			INT				LODBias				= Texture->GetCachedLODBias();
			INT				NumMips				= Texture->Mips.Num();	
			INT				MaxMips				= Max( 1, Min( NumMips - Texture->GetCachedLODBias(), GMaxTextureMipCount ) );
			INT				OrigSizeX			= Texture->SizeX;
			INT				OrigSizeY			= Texture->SizeY;
			INT				CookedSizeX			= Texture->SizeX >> LODBias;
			INT				CookedSizeY			= Texture->SizeY >> LODBias;
			INT				DroppedMips			= Texture->Mips.Num() - Texture->ResidentMips;
			INT				CurSizeX			= Texture->SizeX >> DroppedMips;
			INT				CurSizeY			= Texture->SizeY >> DroppedMips;
			UBOOL			bIsStreamingTexture = StreamableTextureMap.Find( Texture ) != NULL;
			INT				MaxSize				= 0;
			INT				CurrentSize			= 0;

			for( INT MipIndex=Max(0,NumMips-MaxMips); MipIndex<NumMips; MipIndex++ )
			{
				const FTexture2DMipMap& Mip = Texture->Mips(MipIndex);
				MaxSize += Mip.Data.GetBulkDataSize();
			}
			for( INT MipIndex=NumMips-Texture->ResidentMips; MipIndex<NumMips; MipIndex++ )
			{
				const FTexture2DMipMap& Mip = Texture->Mips(MipIndex);
				CurrentSize += Mip.Data.GetBulkDataSize();
			}

			if( (bShouldOnlyListStreaming && bIsStreamingTexture) 
			||	(bShouldOnlyListNonStreaming && !bIsStreamingTexture) 
			||	(!bShouldOnlyListStreaming && !bShouldOnlyListNonStreaming) )
			{
				new(SortedTextures) FSortedTexture( 
										OrigSizeX, 
										OrigSizeY, 
										CookedSizeX,
										CookedSizeY,
										CurSizeX,
										CurSizeY,
										LODBias, 
										MaxSize / 1024, 
										CurrentSize / 1024, 
										Texture->GetPathName(), 
										LODGroup, 
										bIsStreamingTexture );
			}
		}

		// Sort textures by cost.
		Sort<USE_COMPARE_CONSTREF(FSortedTexture,UnPlayer)>(SortedTextures.GetTypedData(),SortedTextures.Num());

		// Retrieve mapping from LOD group enum value to text representation.
		TArray<FString> TextureGroupNames = FTextureLODSettings::GetTextureGroupNames();

		// Display.
		INT TotalMaxSize		= 0;
		INT TotalCurrentSize	= 0;
		Ar.Logf( TEXT(",Authored Width,Authored Height,Cooked Width,Cooked Height,Current Width,Current Height,Max Size,Current Size,LODBias,LODGroup,Name,Streaming") );
		for( INT TextureIndex=0; TextureIndex<SortedTextures.Num(); TextureIndex++ )
 		{
 			const FSortedTexture& SortedTexture = SortedTextures(TextureIndex);
			Ar.Logf( TEXT(",%i,%i,%i,%i,%i,%i,%i,%i,%i,%s,%s,%s"),
 				SortedTexture.OrigSizeX,
 				SortedTexture.OrigSizeY,
				SortedTexture.CookedSizeX,
				SortedTexture.CookedSizeY,
				SortedTexture.CurSizeX,
				SortedTexture.CurSizeY,
				SortedTexture.MaxSize,
 				SortedTexture.CurrentSize,
				SortedTexture.LODBias,
 				*TextureGroupNames(SortedTexture.LODGroup),
				*SortedTexture.Name,
				SortedTexture.bIsStreaming ? TEXT("YES") : TEXT("NO") );
			
			TotalMaxSize		+= SortedTexture.MaxSize;
			TotalCurrentSize	+= SortedTexture.CurrentSize;
 		}

		Ar.Logf(TEXT("Total size: Max= %d  Current= %d"), TotalMaxSize, TotalCurrentSize );
		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("LISTANIMSETS")))
	{
		bAlphaSort = ParseParam( Cmd, TEXT("ALPHASORT") );

		TArray<FSortedAnimSet> SortedSets;
		for( TObjectIterator<UAnimSet> It; It; ++It )
		{
			UAnimSet* Set = *It;
			new(SortedSets) FSortedAnimSet(Set->GetPathName(), Set->GetResourceSize());
		}

		// Sort anim sets by cost
		Sort<USE_COMPARE_CONSTREF(FSortedAnimSet,UnPlayer)>(SortedSets.GetTypedData(),SortedSets.Num());

		// Now print them out.
		Ar.Logf(TEXT("Loaded AnimSets:"));
		INT TotalSize = 0;
		for(INT i=0; i<SortedSets.Num(); i++)
		{
			FSortedAnimSet& SetInfo = SortedSets(i);
			TotalSize += SetInfo.Size;
			Ar.Logf(TEXT("Size: %d\tName: %s"), SetInfo.Size, *SetInfo.Name);
		}
		Ar.Logf(TEXT("Total Size:%d"), TotalSize);
		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("ANIMSEQSTATS")))
	{
		extern void GatherAnimSequenceStats(FOutputDevice& Ar);
		GatherAnimSequenceStats( Ar );
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHOWHOTKISMET")) )
	{
		// First make array of all USequenceOps
		TArray<USequenceOp*> AllOps;
		for( TObjectIterator<USequenceOp> It; It; ++It )
		{
			USequenceOp* Op = *It;
			AllOps.AddItem(Op);
		}

		// Then sort them
		Sort<USE_COMPARE_POINTER(USequenceOp, UnPlayer)>(&AllOps(0), AllOps.Num());

		// Then print out the top 10
		INT TopNum = ::Min(10, AllOps.Num());
		Ar.Logf( TEXT("----- TOP 10 KISMET SEQUENCEOPS ------") );
		for(INT i=0; i<TopNum; i++)
		{
			Ar.Logf( TEXT("%6d: %s (%d)"), i, *(AllOps(i)->GetPathName()), AllOps(i)->ActivateCount );
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("DLE")) )
	{
		Ar.Logf( TEXT( "Looking for disabled LightEnvironmentComponents" ) );
		INT Num = 0;
		for( TObjectIterator<ULightEnvironmentComponent	> It; It; ++It )
		{
			ULightEnvironmentComponent* LE = *It;
			if(!LE->IsTemplate() && !LE->IsEnabled())
			{
				Ar.Logf(TEXT("  Disabled DLE: %d %s"), Num, *LE->GetPathName());
				Num++;
			}
		}
		Ar.Logf( TEXT( "Total Disabled found: %i" ), Num );

		return TRUE;
	}
#if !CONSOLE && defined(_MSC_VER)
	else if( ParseCommand( &Cmd, TEXT("TOGGLEDEBUGGER") ) )
	{
		if ( GDebugger == NULL )
		{
			GDebugger = new UDebuggerCore();
		}
		UDebuggerCore* ScriptDebugger = static_cast<UDebuggerCore*>(GDebugger);
		ScriptDebugger->AttachScriptDebugger(!ScriptDebugger->IsDebuggerAttached());
		return TRUE;
	}
#endif
	else if ( ParseCommand(&Cmd, TEXT("DEBUGPREFAB")) )
	{
		FString ObjectName;
		if ( ParseToken(Cmd,ObjectName,TRUE) )
		{
			UObject* TargetObject = FindObject<UObject>(ANY_PACKAGE,*ObjectName);
			if ( TargetObject != NULL )
			{
				UBOOL bParsedCommand = FALSE;
				if ( ParseCommand(&Cmd, TEXT("SHOWMAP")) )
				{
					bParsedCommand = TRUE;

					TMap<UObject*,UObject*>* ArcInstMap=NULL;
					APrefabInstance* ActorPrefab = Cast<APrefabInstance>(TargetObject);
					if ( ActorPrefab != NULL )
					{
						ArcInstMap = &ActorPrefab->ArchetypeToInstanceMap;
					}
					else
					{
						UUIPrefabInstance* UIPrefab = Cast<UUIPrefabInstance>(TargetObject);
						if ( UIPrefab != NULL )
						{
							ArcInstMap = &UIPrefab->ArchetypeToInstanceMap;
						}
					}

					if ( ArcInstMap != NULL )
					{
						INT ElementIndex = 0;
						INT IndexPadding = appItoa(ArcInstMap->Num()).Len();
						Ar.Logf(TEXT("Archetype mappings for %s  (%i elements)"), *TargetObject->GetFullName(), ArcInstMap->Num());
						for ( TMap<UObject*,UObject*>::TConstIterator It(*ArcInstMap); It; ++It )
						{
							UObject* Arc = It.Key();
							UObject* Inst = It.Value();
							UObject* RealArc = Inst != NULL ? Inst->GetArchetype() : NULL;

							if ( Arc != NULL && Inst != NULL )
							{
								if ( Arc == RealArc )
								{
									Ar.Logf(TEXT(" +  %*i) %s ==> %s"), IndexPadding, ElementIndex++, *Inst->GetPathName(), *Arc->GetPathName());
								}
								else
								{
									Ar.Logf(TEXT(" .  %*i) %s ==> %s (%s)"), IndexPadding, ElementIndex++, *Inst->GetPathName(), *Arc->GetPathName(), *RealArc->GetPathName());
								}
							}
							else if ( Inst == NULL )
							{
								Ar.Logf(TEXT(" ?  %*i) %s ==> %s"), IndexPadding, ElementIndex++, *Inst->GetPathName(), *Arc->GetPathName());
							}
							else if ( Arc == NULL )
							{
								Ar.Logf(TEXT(" !  %*i) %s ==> %s"), IndexPadding, ElementIndex++, *Inst->GetPathName(), *RealArc->GetPathName());
							}
						}
					}
					else
					{
						Ar.Logf(TEXT("'%s' is not a prefab instance"), *TargetObject->GetFullName());
					}
				}

				if ( ParseCommand(&Cmd, TEXT("GRAPH")) )
				{
					bParsedCommand = TRUE;
					ShowSubobjectGraph( Ar, TargetObject, TEXT("") );
				}

				if ( !bParsedCommand )
				{
					Ar.Logf(TEXT("SYNTAX:  DEBUGPREFAB <PathNameForPrefabInstance> <COMMAND>"));
				}
			}
			else
			{
				Ar.Logf(TEXT("No object found using '%s'"), *ObjectName);
			}
		}
		else
		{
			Ar.Logf(TEXT("No object name specified - syntax: DEBUGPREFAB PathNameForPrefabInstance COMMAND"));
		}

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEMINDISTORT")) )
	{
		extern UBOOL GRenderMinimalDistortion;
		FlushRenderingCommands();
		GRenderMinimalDistortion = !GRenderMinimalDistortion;
		debugf(TEXT("Resolve minimal screen distortion rectangle = %s"),
			GRenderMinimalDistortion ? TEXT("TRUE") : TEXT("FALSE"));
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEMINTRANSLUCENT")) )
	{
		extern UBOOL GRenderMinimalTranslucency;
		FlushRenderingCommands();
		GRenderMinimalTranslucency = !GRenderMinimalTranslucency;
		debugf(TEXT("Resolve minimal translucency rectangle = %s"),
			GRenderMinimalTranslucency ? TEXT("TRUE") : TEXT("FALSE"));
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("LISTSPAWNEDACTORS")) )
	{
		if( GWorld && GamePlayers.Num() )
		{
			// Cached invariants.
			const FLOAT	TimeSeconds		    = GWorld->GetTimeSeconds();
			const FVector PlayerLocation	= GamePlayers(0)->LastViewLocation;

			// Create alphanumerically sorted list of actors in persistent level.
			TArray<AActor*> SortedActorList = GWorld->PersistentLevel->Actors;
			Sort<USE_COMPARE_POINTER( AActor, UnPlayer )>( SortedActorList.GetTypedData(), SortedActorList.Num() );

			// Iterate over all non-static actors and log detailed information.
			Ar.Logf(TEXT("Listing spawned actors in persistent level:"));
			Ar.Logf(TEXT("Total: %d" ), SortedActorList.Num());
			Ar.Logf(TEXT("TimeUnseen,TimeAlive,Distance,Class,Name,Owner"));
			for( INT ActorIndex=0; ActorIndex<SortedActorList.Num(); ActorIndex++ )
			{
				AActor* Actor = SortedActorList(ActorIndex);
				if( Actor && !Actor->bStatic )
				{
					// Calculate time actor has been alive for. Certain actors can be spawned before TimeSeconds is valid
					// so we manually reset them to the same time as TimeSeconds.
					FLOAT TimeAlive	= TimeSeconds - Actor->CreationTime;
					if( TimeAlive < 0 )
					{
						TimeAlive = TimeSeconds;
					}
					const FLOAT TimeUnseen = TimeSeconds - Actor->LastRenderTime;
					const FLOAT DistanceToPlayer = FDist( Actor->Location, PlayerLocation );
					Ar.Logf(TEXT("%6.2f,%6.2f,%8.0f,%s,%s,%s"),TimeUnseen,TimeAlive,DistanceToPlayer,*Actor->GetClass()->GetName(),*Actor->GetName(),*Actor->Owner->GetName());
				}
			}
		}
		else
		{
			Ar.Logf(TEXT("LISTSPAWNEDACTORS failed."));
		}
		return TRUE;
	}

	return FALSE;
}

/**
 * Computes a color to use for property coloration for the given object.
 *
 * @param	Object		The object for which to compute a property color.
 * @param	OutColor	[out] The returned color.
 * @return				TRUE if a color was successfully set on OutColor, FALSE otherwise.
 */
UBOOL UEngine::GetPropertyColorationColor(UObject* Object, FColor& OutColor)
{
	return FALSE;
}

/** Uses StatColorMappings to find a color for this stat's value. */
UBOOL UEngine::GetStatValueColoration(const FString& StatName, FLOAT Value, FColor& OutColor)
{
	for(INT i=0; i<StatColorMappings.Num(); i++)
	{
		const FStatColorMapping& Mapping = StatColorMappings(i);
		if(StatName == Mapping.StatName)
		{
			const INT NumPoints = Mapping.ColorMap.Num();

			// If no point in curve, return the Default value we passed in.
			if( NumPoints == 0 )
			{
				return FALSE;
			}

			// If only one point, or before the first point in the curve, return the first points value.
			if( NumPoints < 2 || (Value <= Mapping.ColorMap(0).In) )
			{
				OutColor = Mapping.ColorMap(0).Out;
				return TRUE;
			}

			// If beyond the last point in the curve, return its value.
			if( Value >= Mapping.ColorMap(NumPoints-1).In )
			{
				OutColor = Mapping.ColorMap(NumPoints-1).Out;
				return TRUE;
			}

			// Somewhere with curve range - linear search to find value.
			for( INT i=1; i<NumPoints; i++ )
			{	
				if( Value < Mapping.ColorMap(i).In )
				{
					if (Mapping.DisableBlend)
					{
						OutColor = Mapping.ColorMap(i).Out;
					}
					else
					{
						const FLOAT Diff = Mapping.ColorMap(i).In - Mapping.ColorMap(i-1).In;
						const FLOAT Alpha = (Value - Mapping.ColorMap(i-1).In) / Diff;

						FLinearColor A(Mapping.ColorMap(i-1).Out);
						FVector AV(A.R, A.G, A.B);

						FLinearColor B(Mapping.ColorMap(i).Out);
						FVector BV(B.R, B.G, B.B);

						FVector OutColorV = Lerp( AV, BV, Alpha );
						OutColor = FLinearColor(OutColorV.X, OutColorV.Y, OutColorV.Z);
					}

					return TRUE;
				}
			}

			OutColor = Mapping.ColorMap(NumPoints-1).Out;
			return TRUE;
		}
	}

	// No entry for this stat name
	return FALSE;
}

void UEngine::OnLostFocusPause(UBOOL EnablePause)
{
	if( bPauseOnLossOfFocus )
	{
		// Iterate over all players and pause / unpause them
		// Note: pausing / unpausing the player is done via their HUD pausing / unpausing
		for(INT PlayerIndex = 0;PlayerIndex < GamePlayers.Num();PlayerIndex++)
		{
			ULocalPlayer* Player = GamePlayers(PlayerIndex);
			if(Player && Player->Actor && Player->Actor->myHUD)
			{
				Player->Actor->myHUD->eventOnLostFocusPause(EnablePause);
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	UServerCommandlet.
-----------------------------------------------------------------------------*/

INT UServerCommandlet::Main( const FString& Params )
{
	GIsRunning = 1;
	GIsRequestingExit = FALSE;

#if !CONSOLE && !__GNUC__// Windows only
	/**
	 * Used by the .com wrapper to notify that the Ctrl-C handler was triggered.
	 * This shared event is checked each tick so that the log file can be
	 * cleanly flushed
	 */
	FEvent* ComWrapperShutdownEvent = GSynchronizeFactory->CreateSynchEvent(TRUE,TEXT("ComWrapperShutdown"));
#endif

	// Main loop.
	while( GIsRunning && !GIsRequestingExit )
	{
		// Update GCurrentTime/ GDeltaTime while taking into account max tick rate.
 		extern void appUpdateTimeAndHandleMaxTickRate();
		appUpdateTimeAndHandleMaxTickRate();

		// Tick the engine.
		GEngine->Tick( GDeltaTime );
	
#if STATS
		// Write all stats for this frame out
		GStatManager.AdvanceFrame();
#endif

#if !CONSOLE && !__GNUC__
		// See if the Ctrl-C handler of the .com wrapper asked us to exit
		if (ComWrapperShutdownEvent->Wait(0) == TRUE)
		{
			// inform GameInfo that we're exiting
			if (GWorld != NULL && GWorld->GetWorldInfo() != NULL && GWorld->GetWorldInfo()->Game != NULL)
			{
				GWorld->GetWorldInfo()->Game->eventGameEnding();
			}
			// It wants us to exit
			GIsRequestingExit = TRUE;
		}
#endif
	}

#if !CONSOLE && !__GNUC__
	// Create an event that is shared across apps and is manual reset
	GSynchronizeFactory->Destroy(ComWrapperShutdownEvent);
#endif

	GIsRunning = 0;
	return 0;
}

IMPLEMENT_CLASS(UServerCommandlet)


INT SmokeTest_CheckNativeClassSizes( const TCHAR* Parms );
INT SmokeTest_RunServer( const TCHAR* Parms );

/**
 * smoke test to check CheckNativeClassSizes  this doesn't work just yet
 *
 *
 * Note:  probably need to move the init() call to below and then set our IsFoo=1 for the type of 
 * test we want
 *
 */
INT USmokeTestCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	INT Retval = 0;

	// put various smoke test testing code in here before moving off to their
	// own commandlet

	if( ParseParam(appCmdLine(),TEXT("SERVER")) == TRUE )
	{
		Retval = SmokeTest_RunServer( Parms );
	}
	else if( ParseParam(appCmdLine(),TEXT("CHECK_NATIVE_CLASS_SIZES")) == TRUE )
	{
		Retval = SmokeTest_CheckNativeClassSizes( Parms );
	}
	// other tests we want are to launch the editor and then quit
	// launch the game and then quit
	else
	{
		// exit with no error
		Retval = 0;
	}

	GIsRequestingExit = TRUE; // so CTRL-C will exit immediately


	return Retval;
}
IMPLEMENT_CLASS(USmokeTestCommandlet)

/**
 * Run the server for one tick and then quit
 */
INT SmokeTest_RunServer( const TCHAR* Parms )
{
	INT Retval = 0;

	GIsRunning = 1;

	// Main loop.
	while( GIsRunning && !GIsRequestingExit )
	{
		// Update GCurrentTime/ GDeltaTime while taking into account max tick rate.
 		extern void appUpdateTimeAndHandleMaxTickRate();
		appUpdateTimeAndHandleMaxTickRate();
		
		// Tick the engine.
		GEngine->Tick( GDeltaTime );

		GIsRequestingExit = TRUE; // so CTRL-C will exit immediately
	}

	GIsRunning = 0;

	return Retval;
}


/**
 * test to check CheckNativeClassSizes 
 */
INT SmokeTest_CheckNativeClassSizes( const TCHAR* Parms )
{
	GIsRequestingExit = TRUE; // so CTRL-C will exit immediately

	LoadAllNativeScriptPackages( FALSE ); // after this we can verify them

	// Verify native class sizes and member offsets.
	CheckNativeClassSizes();

	return 0;
}




/*-----------------------------------------------------------------------------
	Actor iterator implementations.
-----------------------------------------------------------------------------*/

/**
 * Returns the actor count.
 *
 * @param total actor count
 */
INT FActorIteratorBase::GetProgressDenominator()
{
	return GetActorCount();
}
/**
 * Returns the actor count.
 *
 * @param total actor count
 */
INT FActorIteratorBase::GetActorCount()
{
	INT TotalActorCount = 0;
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		TotalActorCount += Level->Actors.Num();
	}
	return TotalActorCount;
}
/**
 * Returns the dynamic actor count.
 *
 * @param total dynamic actor count
 */
INT FActorIteratorBase::GetDynamicActorCount()
{
	INT TotalActorCount = 0;
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		TotalActorCount += Level->Actors.Num() - Level->iFirstDynamicActor;
	}
	return TotalActorCount;
}
/**
 * Returns the net relevant actor count.
 *
 * @param total net relevant actor count
 */
INT FActorIteratorBase::GetNetRelevantActorCount()
{
	INT TotalActorCount = 0;
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		TotalActorCount += Level->Actors.Num() - Level->iFirstNetRelevantActor;
	}
	return TotalActorCount;
}

/**
 * Stats objects for Engine
 */
DECLARE_STATS_GROUP(TEXT("Engine"),STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("FrameTime"),STAT_FrameTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("GameEngine Tick"),STAT_GameEngineTick,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("GameViewport Tick"),STAT_GameViewportTick,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("RedrawViewports"),STAT_RedrawViewports,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Update Level Streaming"),STAT_UpdateLevelStreaming,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("RHI Game Tick"),STAT_RHITickTime,STATGROUP_Engine);

DECLARE_STATS_GROUP(TEXT("Script"),STATGROUP_Script);

/** Terrain stats */
DECLARE_CYCLE_STAT(TEXT("Terrain Foliage Time"),STAT_TerrainFoliageTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Foliage Render Time"),STAT_FoliageRenderTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Foliage Tick Time"),STAT_FoliageTickTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Terrain Smooth Time"),STAT_TerrainSmoothTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Terrain Render Time"),STAT_TerrainRenderTime,STATGROUP_Engine);
DECLARE_DWORD_COUNTER_STAT(TEXT("Terrain Triangles"),STAT_TerrainTriangles,STATGROUP_Engine);
DECLARE_DWORD_COUNTER_STAT(TEXT("Foliage Instances"),STAT_TerrainFoliageInstances,STATGROUP_Engine);

/** Input stat */
DECLARE_CYCLE_STAT(TEXT("Input Time"),STAT_InputTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Input Latency"),STAT_InputLatencyTime,STATGROUP_Engine);

/** HUD stat */
DECLARE_CYCLE_STAT(TEXT("HUD Time"),STAT_HudTime,STATGROUP_Engine);

/** Shadow volume stats */
DECLARE_DWORD_COUNTER_STAT(TEXT("Shadow Volume Tris"),STAT_ShadowVolumeTriangles,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Shadow Extrusion Time"),STAT_ShadowExtrusionTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Shadow Volume Render Time"),STAT_ShadowVolumeRenderTime,STATGROUP_Engine);

/** Static mesh tris rendered */
DECLARE_DWORD_COUNTER_STAT(TEXT("Static Mesh Tris"),STAT_StaticMeshTriangles,STATGROUP_Engine);

/** Skeletal stats */
DECLARE_CYCLE_STAT(TEXT("Skel Skin Time"),STAT_SkinningTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Update Cloth Verts Time"),STAT_UpdateClothVertsTime,STATGROUP_Engine);
DECLARE_CYCLE_STAT(TEXT("Update SoftBody Verts Time"),STAT_UpdateSoftBodyVertsTime,STATGROUP_Engine);
DECLARE_DWORD_COUNTER_STAT(TEXT("Skel Mesh Tris"),STAT_SkelMeshTriangles,STATGROUP_Engine);
DECLARE_DWORD_COUNTER_STAT(TEXT("Skel Mesh Draw Calls"),STAT_SkelMeshDrawCalls,STATGROUP_Engine);
DECLARE_DWORD_COUNTER_STAT(TEXT("Skel Verts CPU Skin"),STAT_CPUSkinVertices,STATGROUP_Engine);
DECLARE_DWORD_COUNTER_STAT(TEXT("Skel Verts GPU Skin"),STAT_GPUSkinVertices,STATGROUP_Engine);

/** Fracture */
DECLARE_DWORD_COUNTER_STAT(TEXT("Fracture Part Pool Used"),STAT_FracturePartPoolUsed,STATGROUP_Engine);

/** Frame chart stats */
DECLARE_STATS_GROUP(TEXT("FPSChart"),STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("00 - 05 FPS"), STAT_FPSChart_0_5,		STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("05 - 10 FPS"), STAT_FPSChart_5_10,		STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("10 - 15 FPS"), STAT_FPSChart_10_15,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("15 - 20 FPS"), STAT_FPSChart_15_20,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("20 - 25 FPS"), STAT_FPSChart_20_25,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("25 - 30 FPS"), STAT_FPSChart_25_30,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("30 - 35 FPS"), STAT_FPSChart_30_35,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("35 - 40 FPS"), STAT_FPSChart_35_40,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("40 - 45 FPS"), STAT_FPSChart_40_45,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("45 - 50 FPS"), STAT_FPSChart_45_50,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("50 - 55 FPS"), STAT_FPSChart_50_55,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("55 - 60 FPS"), STAT_FPSChart_55_60,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("60 - .. FPS"), STAT_FPSChart_60_INF,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("30+ FPS"), STAT_FPSChart_30Plus,	STATGROUP_FPSChart);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Unaccounted time"), STAT_FPSChart_UnaccountedTime,	STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Frame count"),	STAT_FPSChart_FrameCount, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("5.0s  - ....  hitches"), STAT_FPSChart_Hitch_5000_Plus, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("2.5s  - 5.0s  hitches"), STAT_FPSChart_Hitch_2500_5000, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("2.0s  - 2.5s  hitches"), STAT_FPSChart_Hitch_2000_2500, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("1.5s  - 2.0s  hitches"), STAT_FPSChart_Hitch_1500_2000, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("1.0s  - 1.5s  hitches"), STAT_FPSChart_Hitch_1000_1500, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("0.75s - 1.0s  hitches"), STAT_FPSChart_Hitch_750_1000, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("0.5s  - 0.75s hitches"), STAT_FPSChart_Hitch_500_750, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("0.3s  - 0.5s  hitches"), STAT_FPSChart_Hitch_300_500, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("0.2s  - 0.3s  hitches"), STAT_FPSChart_Hitch_200_300, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("0.15s - 0.2s  hitches"), STAT_FPSChart_Hitch_150_200, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("0.1s  - 0.15s hitches"), STAT_FPSChart_Hitch_100_150, STATGROUP_FPSChart);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Total hitches"), STAT_FPSChart_TotalHitchCount, STATGROUP_FPSChart);



