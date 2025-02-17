//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"
#include "EngineMaterialClasses.h"
#include "EngineAnimClasses.h"
#include "UnPath.h"

#include "FConfigCacheIni.h"

// These are not included in UTGame.h, so included here.
#include "UTGameAnimationClasses.h"
#include "UTGameSequenceClasses.h"
#include "UTGameUIClasses.h"
#include "UTGameAIClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameUIFrontEndClasses.h"
#include "UTDownloadableContent.h"
#include "UTPatchHelper.h"

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName UTGAME_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "UTGameClasses.h"
#include "UTGameAnimationClasses.h"
#include "UTGameSequenceClasses.h"
#include "UTGameUIClasses.h"
#include "UTGameAIClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameUIFrontEndClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "UTGameClasses.h"
#include "UTGameAnimationClasses.h"
#include "UTGameSequenceClasses.h"
#include "UTGameUIClasses.h"
#include "UTGameAIClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameUIFrontEndClasses.h"
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
void AutoInitializeRegistrantsUTGame( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_UTGAME;
	AUTO_INITIALIZE_REGISTRANTS_UTGAME_ANIMATION;
	AUTO_INITIALIZE_REGISTRANTS_UTGAME_SEQUENCE;
	AUTO_INITIALIZE_REGISTRANTS_UTGAME_UI;
	AUTO_INITIALIZE_REGISTRANTS_UTGAME_AI;
	AUTO_INITIALIZE_REGISTRANTS_UTGAME_VEHICLE;
	AUTO_INITIALIZE_REGISTRANTS_UTGAME_UIFRONTEND;
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesUTGame()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) UTGAME_##name = FName(TEXT(#name));
	#include "UTGameClasses.h"
	#include "UTGameAnimationClasses.h"
	#include "UTGameSequenceClasses.h"
	#include "UTGameUIClasses.h"
	#include "UTGameAIClasses.h"
	#include "UTGameVehicleClasses.h"
	#include "UTGameUIFrontEndClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}


IMPLEMENT_CLASS(UUTTypes);
IMPLEMENT_CLASS(AUTEmitter);
IMPLEMENT_CLASS(AUTReplicatedEmitter);
IMPLEMENT_CLASS(UUTCheatManager);
IMPLEMENT_CLASS(AUTPickupFactory);
IMPLEMENT_CLASS(AUTAmmoPickupFactory);
IMPLEMENT_CLASS(AUTPowerupPickupFactory);
IMPLEMENT_CLASS(AUTWeaponLocker);
IMPLEMENT_CLASS(AUTGameObjective);
IMPLEMENT_CLASS(UUTActorFactoryVehicle);
IMPLEMENT_CLASS(UUTSkeletalMeshComponent);
IMPLEMENT_CLASS(AUTCarriedObject);
IMPLEMENT_CLASS(AUTPlayerReplicationInfo);
IMPLEMENT_CLASS(AUTLinkedReplicationInfo);
IMPLEMENT_CLASS(UUTPhysicalMaterialProperty);
IMPLEMENT_CLASS(UUTActorFactoryMover);
IMPLEMENT_CLASS(UUTActorFactoryPickup);
IMPLEMENT_CLASS(AUTProj_Grenade);
IMPLEMENT_CLASS(AUTKActor);
IMPLEMENT_CLASS(AUTInventory);
IMPLEMENT_CLASS(AUTTimedPowerup);
IMPLEMENT_CLASS(UUTActorFactoryAI);
IMPLEMENT_CLASS(AUTTeamInfo);
IMPLEMENT_CLASS(UUTMapInfo);
IMPLEMENT_CLASS(AUTDroppedPickup);
IMPLEMENT_CLASS(AUTItemPickupFactory);
IMPLEMENT_CLASS(AUTGame);
IMPLEMENT_CLASS(UUTMapMusicInfo);
IMPLEMENT_CLASS(UUTParticleSystemComponent);
IMPLEMENT_CLASS(AUTHUD);
IMPLEMENT_CLASS(AUTCTFFlag);
IMPLEMENT_CLASS(AUTInventoryManager);
IMPLEMENT_CLASS(AForcedDirVolume);
IMPLEMENT_CLASS(AUTMutator);
IMPLEMENT_CLASS(AUTGameReplicationInfo);
IMPLEMENT_CLASS(AUTEntryPlayerController);
IMPLEMENT_CLASS(AUTSlowVolume);
IMPLEMENT_CLASS(AUTEmitterPool);
IMPLEMENT_CLASS(AUTVoteCollector);
IMPLEMENT_CLASS(AUTVoteReplicationInfo);
IMPLEMENT_CLASS(UUTGameViewportClient);
IMPLEMENT_CLASS(AUTTeleporterBase);
IMPLEMENT_CLASS(AUTArmorPickupFactory);
IMPLEMENT_CLASS(AUTHealthPickupFactory);
IMPLEMENT_CLASS(AUTPickupLight);
IMPLEMENT_CLASS(AUTArmorPickupLight);
IMPLEMENT_CLASS(AUTHealthPickupLight);
IMPLEMENT_CLASS(AUTWeaponPickupLight);
IMPLEMENT_CLASS(AUTWeaponLockerPickupLight);
IMPLEMENT_CLASS(AUTPickupFactory_HealthVial);
IMPLEMENT_CLASS(UUTProfileSettings);
IMPLEMENT_CLASS(UUTSeqVar_Character);
IMPLEMENT_CLASS(UUTSeqAct_UnlockChar);
IMPLEMENT_CLASS(UUTSeqEvent_TouchStatus);
IMPLEMENT_CLASS(UUTDemoRecDriver);
IMPLEMENT_CLASS(AUTVehicleFactory);

#define CustomCharPartsKey TEXT("CustomParts")
#define CustomCharactersKey TEXT("CustomChars")

UBOOL AUTGame::IsLowGoreVersion()
{
	return FALSE; //GIsLowGoreVersion;
}

/** @return		TRUE if any packages in the specified bundles are maps. */
static UBOOL BundleContainsMaps(const FDLCBundle& Bundle)
{
	for( INT PackageFileIndex = 0 ; PackageFileIndex < Bundle.PackageFiles.Num() ; ++PackageFileIndex )
	{
		const FFilename FileName( Bundle.PackageFiles(PackageFileIndex) );
		if( FileName.GetExtension() == TEXT("ut3") )
		{
			//debugf(TEXT("Bundle: %s DOES contain maps!"), *Bundle.Directory);
			return TRUE;
		}
	}
	//debugf(TEXT("Bundle: %s DOES NOT contain maps!"), *Bundle.Directory);
	return FALSE;
}

/**
 * Game-specific code to handle DLC being added or removed
 * 
 * @param bContentWasInstalled TRUE if DLC was installed, FALSE if it was removed
 */
void FUTDownloadableContent::OnDownloadableContentChanged(UBOOL bContentWasInstalled)
{
	// refresh the data provider
	UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();
	if (DataStoreClient != NULL)
	{
		UUIDataStore_GameResource* DataStore = (UUIDataStore_GameResource*)DataStoreClient->FindDataStore(FName(TEXT("UTMenuItems")));
		if (DataStore)
		{
			INT OldNumMutators = 0;
			INT OldNumMaps = 0;
			if (!bContentWasInstalled)
			{
				TArray<UUIResourceDataProvider*> CurrentProviders;
				DataStore->ListElementProviders.MultiFind(TEXT("Mutators"), CurrentProviders);
				OldNumMutators = CurrentProviders.Num();

				CurrentProviders.Reset();
				DataStore->ListElementProviders.MultiFind(TEXT("Maps"), CurrentProviders);
				OldNumMaps = CurrentProviders.Num();
			}

			// reparse the .ini files for the PerObjectConfig objects
			DataStore->InitializeListElementProviders();

			if (!bContentWasInstalled)
			{
				// clear out the mutator and map cycle lists when content is removed as the indices may now be invalid
				UUTUIDataStore_MenuItems* MenuStore = Cast<UUTUIDataStore_MenuItems>(DataStore);
				if (MenuStore != NULL)
				{
					TArray<UUIResourceDataProvider*> CurrentProviders;
					DataStore->ListElementProviders.MultiFind(TEXT("Mutators"), CurrentProviders);
					if (OldNumMutators != CurrentProviders.Num())
					{
						MenuStore->EnabledMutators.Empty();
					}

					CurrentProviders.Reset();
					DataStore->ListElementProviders.MultiFind(TEXT("Maps"), CurrentProviders);
					if (OldNumMaps != CurrentProviders.Num())
					{
						MenuStore->MapCycle.Empty();
					}
				}
			}

			// make ay currently active widgets using the data get refreshed
			DataStore->eventRefreshSubscribers();
		}
	}
}

/** Generates any missing per-map .ini files for the specified DLC bundle. */
void FUTDownloadableContent::GenerateMissingCustomMapINIsForBundle(const FDLCBundle& Bundle)
{
#if !CONSOLE
	// Find and integrate .ini files.

	// Map of map packages and a bool indicating whether or not the map has a .ini
	TMap<FString, UBOOL> Maps;

	// Find all of the map packages
	for(INT PackageFileIndex = 0; PackageFileIndex < Bundle.PackageFiles.Num(); ++PackageFileIndex)
	{
		const FFilename FileName( Bundle.PackageFiles(PackageFileIndex) );

		// NOTE: We skip packages that appear to be localization-related, since we never want to generate INIs
		//    for those files
		if( FileName.GetExtension() == TEXT("ut3") && ( FileName.InStr(LOCALIZED_SEEKFREE_SUFFIX) == INDEX_NONE ) )
		{
			Maps.Set(*FileName.GetBaseFilename(FALSE), FALSE);
		}
	}

	// Find all of the .ini's
	for(INT NonPackageIndex = 0; NonPackageIndex < Bundle.NonPackageFiles.Num(); ++NonPackageIndex)
	{
		const FFilename FileName( Bundle.NonPackageFiles(NonPackageIndex) );
		if( FileName.GetExtension() == TEXT("ini") )
		{
			const FString BaseIniName = FileName.GetBaseFilename(FALSE);
			UBOOL *Value = Maps.Find(BaseIniName);
			if(Value)
			{
				*Value = TRUE;
			}
		}
	}

	// Generate any missing map .ini's with defaults
	TArray<FString> *Inis = const_cast<TArray<FString>*>(&Bundle.NonPackageFiles);
	for(TMap<FString, UBOOL>::TIterator Iter(Maps); Iter; ++Iter)
	{
		if(!Iter.Value())
		{
			const FString NewConfig = Iter.Key() + TEXT(".ini");
			const FString MapName = FFilename(Iter.Key()).GetBaseFilename();

			// The 'FriendlyName' is the name that will be displayed in the menus.
			// Strip the gametype from the front (e.g. 'DM-Blah' becomes 'Blah').
			FString FriendlyName = MapName;
			FString MapPrefix;
			const INT DashPos = FriendlyName.InStr( TEXT("-") );
			if (DashPos != -1)
			{
				MapPrefix = FriendlyName.Left( DashPos );
				FriendlyName = FriendlyName.Right( (FriendlyName.Len() - DashPos) - 1);
			}
			// dm ctf vctf war
			const FString Section = FString::Printf(TEXT("[%s UTUIDataProvider_MapInfo]\r\nMapName=%s\r\nFriendlyName=%s\r\nPreviewImageMarkup=<Images:UI_FrontEnd_Art.GameTypes.DeathMatch>\r\nDescription=None\r\n"), *MapName, *MapName, *FriendlyName);
			appSaveStringToFile(Section, *NewConfig);

			Inis->AddItem(NewConfig);
		}
	}
#endif
}

void FUTDownloadableContent::InstallDLCBundles(TArray<FDLCBundle>& Bundles)
{
	// The 'Characters' property; we'll use this to parse data from DLC .ini's.
	const UProperty* CharactersProp = FindObjectChecked<UProperty>( ANY_PACKAGE, TEXT("UTGame.UTCharInfo.Characters.Characters") );

	// Temporarily allow the GConfigCache to perform file operations if they were off.
	const UBOOL bWereFileOpsDisabled = GConfig->AreFileOperationsDisabled();
	GConfig->EnableFileOperations();

	for ( INT BundleIndex = 0 ; BundleIndex < Bundles.Num() ; ++BundleIndex )
	{
		FDLCBundle& Bundle = Bundles(BundleIndex);

		// Handle maps
		if ( BundleContainsMaps( Bundle ) )
		{
			GenerateMissingCustomMapINIsForBundle( Bundle );
		}

		// Install the bundle.
		InstallDownloadableContent( *Bundle.Directory, Bundle.PackageFiles, Bundle.NonPackageFiles );
	}

	// Re-disable file ops if they were before.
	if ( bWereFileOpsDisabled )
	{
		GConfig->DisableFileOperations();
	}
}

/**
 * Allows a game to handle special ini sections in a DLC ini file
 *
 * @param NewConfigPath Pathname to the config that can be used with GConfig
 * @param IniFilename Filename that can be used to read in the ini file manually
 */
void FUTDownloadableContent::HandleExtraIniSection(const FString& NewConfigPath, const FString& IniFilename)
{
	// attempt to cast the engine to a GameEngine
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);

	// Handle mutators specially to not have them always loaded
	const TCHAR* LoadForAllGameTypesIniSection = TEXT("LoadForAllGameTypes");
	const TMultiMap<FString,FString>* AllGameTypeList = GConfig->GetSectionPrivate( LoadForAllGameTypesIniSection, FALSE, TRUE, *NewConfigPath );
	if ( LoadForAllGameTypesIniSection && AllGameTypeList )
	{
		TArray<FName> PackageNames;
		for( TMultiMap<FString,FString>::TConstIterator It(*AllGameTypeList) ; It ; ++It )
		{
			const FString& PackageName = It.Value();
			PackageNames.AddItem(FName(*PackageName));
			//debugf(TEXT("----------- Adding %s"), *PackageName);
		}
		GameEngine->AddPackagesToFullyLoad( FULLYLOAD_Game_PostLoadClass, TEXT("LoadForAllGameTypes"), PackageNames, TRUE );

		FConfigFile IniFile;
		IniFile.Read(*IniFilename);

		// iterate over the sections in the file
		for (TMap<FString, FConfigSection>::TIterator It(IniFile); It; ++It)
		{
			INT Space = It.Key().InStr(TEXT(" "));
			if (Space != -1)
			{
				// get the bit after the string
				FString ObjectType = It.Key().Right(It.Key().Len() - (Space + 1));

				// look for allowed types
				if (ObjectType == TEXT("UTUIDataProvider_Mutator"))
				{
					FString* ClassName = It.Value().Find(TEXT("ClassName"));
					if (ClassName)
					{
						debugf(TEXT("Adding mutator packages (0 = %s) to load for (%s)"), *PackageNames(0).ToString(), *(*ClassName));
						GameEngine->AddPackagesToFullyLoad( FULLYLOAD_Mutator, *ClassName, PackageNames, TRUE );
					}
				}
			}
		}
	}

}

/**
 * Checks if the game will allow this file to be merged into the GConfigCache
 *
 * @param FullFilename Full path to the file 
 * @param BaseFilename Name of the actual ini file that would be merged with
 * @param ExistingConfigFiles A list of the existing config files
 *
 * @return TRUE if the file is valid and can be added to GConfig
 */
UBOOL FUTDownloadableContent::IsIniOrLocFileValid(const FFilename& FullFilename, const FFilename& BaseFilename, const TArray<FFilename>& ExistingConfigFiles)
{
	// read the file into a temp config file, without touching GConfig at all
	// (yes, this will result in reading it twice, but it will always be from HD)
	FConfigFile IniFile;
	IniFile.Read(*FullFilename);

	// default to allowed
	UBOOL bIsFileAllowed = TRUE;

	// look for an existing ini that matches
	FConfigFile* ExistingConfig = NULL;
	for (INT ConfigFileIndex = 0; ConfigFileIndex < ExistingConfigFiles.Num(); ConfigFileIndex++)
	{
		// does the config file (without path, but with extension) match the ini file?
		if (ExistingConfigFiles(ConfigFileIndex).GetCleanFilename() == BaseFilename)
		{
			ExistingConfig = GConfig->FindConfigFile(*ExistingConfigFiles(ConfigFileIndex));
		}
	}

	// we don't allow updating existing loc data
	if (ExistingConfig && BaseFilename.GetExtension() == UObject::GetLanguage())
	{
		return FALSE;
	}

	// iterate over the sections in the file
	for (TMap<FString, FConfigSection>::TIterator It(IniFile); It; ++It)
	{
#if EPIC_INTERNAL
		// look for special magic which will allow any file
		if (appAnsiStrCrc(TCHAR_TO_ANSI(*It.Key())) == ALLOW_DLC_INI_MAGIC_CRC)
		{
			bIsInstallingSpecialDLC = TRUE;
			// if we see this, we allow any file, no matter the contents
			bIsFileAllowed = TRUE;
			break;
		}
#endif
		// check if the section is allowed
		UBOOL bIsValidSection = FALSE;

		// character pieces are okay
		if (It.Key() == CustomCharPartsKey)
		{
			bIsValidSection = TRUE;
		}
		// DLC package loading are needed and fine
		else if (It.Key() == TEXT("Engine.PackagesToFullyLoadForDLC"))
		{
			bIsValidSection = TRUE;
		}
		else if (It.Key() == TEXT("LoadForAllGameTypes"))
		{
			bIsValidSection = TRUE;
		}
		// look for some per-object config types
		else
		{
			INT Space = It.Key().InStr(TEXT(" "));
			if (Space != -1)
			{
				// get the bit after the string
				FString ObjectType = It.Key().Right(It.Key().Len() - (Space + 1));

				// look for allowed types
				if (ObjectType == TEXT("UTUIDataProvider_GameModeInfo") ||
					ObjectType == TEXT("UTUIDataProvider_MapInfo") ||
					ObjectType == TEXT("UTUIDataProvider_Mutator"))
				{
					bIsValidSection = TRUE;

					// see if the existing config has the same section, in which case don't allow it
					if (ExistingConfig && ExistingConfig->Find(It.Key()) != NULL)
					{
						bIsValidSection = FALSE;
						break;
					}
				}
			}
		}

		// if the section is not valid, deny the entire file
		if (!bIsValidSection)
		{
			bIsFileAllowed = FALSE;
		}
	}

	return bIsFileAllowed;
}

void AUTDroppedPickup::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);
	if(PickupMesh && WorldInfo->NetMode != NM_DedicatedServer )
	{
		if ( (bFadeOut || bRotatingPickup) && (WorldInfo->TimeSeconds - LastRenderTime < 0.2f) )
		{
			PickupMesh->Rotation.Yaw += appRound(DeltaSeconds * YawRotationRate);
			PickupMesh->BeginDeferredUpdateTransform();

			if(PickupParticles)
			{
				PickupParticles->Rotation.Yaw += appRound(DeltaSeconds * YawRotationRate);
				PickupParticles->BeginDeferredUpdateTransform();				
			}
		}
		if ( bFadeOut )
		{
			PickupMesh->Scale = ::Max(0.01f, PickupMesh->Scale - StartScale * DeltaSeconds);
			PickupMesh->BeginDeferredUpdateTransform();
		}
	}
}

void AUTPickupFactory::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);
	
	//@note: we don't check LastRenderTime here because otherwise the pickups always look like they just spawned in on clients
	if (BaseMesh && BaseMaterialInstance && BaseMaterialParamName != NAME_None && WorldInfo->NetMode != NM_DedicatedServer) 
	{
		if(Glow != NULL && BasePulseTime > 0.0f)
		{
			Glow->SetFloatParameter(GlowEmissiveParam,BaseEmissive.A);
		}

		// if this pickup wants to fade in when respawning
		if( bIsRespawning && bDoVisibilityFadeIn )
		{
			if(MIC_Visibility != NULL)
			{
				FLOAT ResVal = 0.0f;
				MIC_Visibility->GetScalarParameterValue(VisibilityParamName,ResVal);
				if (ResVal != 0.0f)
				{
					ResVal *= 1.5;
					if (ResVal < 0.0f) //clamp
					{
						ResVal = 0.0f;
					}
					MIC_Visibility->SetScalarParameterValue(VisibilityParamName, (ResVal-DeltaSeconds)/1.5f  );
				}
			}

			if(MIC_VisibilitySecondMaterial != NULL)
			{
				FLOAT ResVal = 0.0f;
				MIC_VisibilitySecondMaterial->GetScalarParameterValue(VisibilityParamName,ResVal);
				if (ResVal != 0.0f)
				{
					ResVal *= 1.5;
					if (ResVal < 0.0f) //clamp
					{
						ResVal = 0.0f;
					}
					MIC_VisibilitySecondMaterial->SetScalarParameterValue(VisibilityParamName, (ResVal-DeltaSeconds)/1.5f  );
				}
			}
		}
	}

	if ( bUpdatingPickup && PickupMesh && !PickupMesh->HiddenGame && WorldInfo->TimeSeconds - LastRenderTime < 0.2f )
	{
		if ( bRotatingPickup )
		{
			PickupMesh->Rotation.Yaw += appRound(DeltaSeconds * YawRotationRate);

			// @TODO FIXMESTEVE Better way to change pivot point?
			FRotationMatrix R(PickupMesh->Rotation);
			PickupMesh->Translation = PivotTranslation.X * R.GetAxis(0) + PivotTranslation.Y * R.GetAxis(1) + FVector(0,0,PickupMesh->Translation.Z);
		}

		if ( bFloatingPickup )
		{
			BobTimer += DeltaSeconds;
			const FLOAT Offset = BobOffset * appSin(BobTimer * BobSpeed);
			PickupMesh->Translation.Z = BobBaseOffset + Offset;
		}
		// this is just visual, so we don't need to update it immediately
		PickupMesh->BeginDeferredUpdateTransform();
	}

	// Pulse
	if ( BaseMesh && BaseMaterialInstance && WorldInfo->NetMode != NM_DedicatedServer && BaseMaterialParamName != NAME_None )
	{
		//@note: we don't check LastRenderTime here so that the base doesn't wait to start the pulse until it becomes visible
		// which would defeat the purpose of it indicating when the pickup will respawn
		if (BasePulseTime > 0.0)
		{
			BaseEmissive.R += (BaseTargetEmissive.R - BaseEmissive.R) * (DeltaSeconds / BasePulseTime);
			BaseEmissive.G += (BaseTargetEmissive.G - BaseEmissive.G) * (DeltaSeconds / BasePulseTime);
			BaseEmissive.B += (BaseTargetEmissive.B - BaseEmissive.B) * (DeltaSeconds / BasePulseTime);
			BaseEmissive.A += (BaseTargetEmissive.A - BaseEmissive.A) * (DeltaSeconds / BasePulseTime);
			
			BasePulseTime -= DeltaSeconds;
			if ( bPulseBase && BasePulseTime <= 0.0f )
			{
				if ( BaseTargetEmissive == BaseDimEmissive )
				{
					BaseTargetEmissive = BaseBrightEmissive;
				}
				else
				{
					BaseTargetEmissive = BaseDimEmissive;
				}

				BasePulseTime = BasePulseRate;
			}
		}
		else
		{
			BaseEmissive = BaseTargetEmissive;
			BasePulseTime = 0.0f;
		}

		if (WorldInfo->TimeSeconds - LastRenderTime < 0.2f)
		{
			BaseMaterialInstance->SetVectorParameterValue(BaseMaterialParamName, BaseEmissive);
		}
	}
}

void AUTPowerupPickupFactory::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	if (WorldInfo->NetMode != NM_DedicatedServer && Spinner != NULL && !Spinner->HiddenGame && WorldInfo->TimeSeconds - LastRenderTime < 0.2f)
	{
		Spinner->Rotation.Yaw += appRound(DeltaSeconds * YawRotationRate);
		// this is just visual, so we don't need to update it immediately
		Spinner->BeginDeferredUpdateTransform();
	}

	if ( ParticleEffects != NULL && !ParticleEffects->HiddenGame && WorldInfo->NetMode != NM_DedicatedServer && WorldInfo->TimeSeconds - LastRenderTime < 0.2f)
	{
		if ( PickupMesh )
		{
			ParticleEffects->Rotation = PickupMesh->Rotation;
		}
		// this is just visual, so we don't need to update it immediately
		ParticleEffects->BeginDeferredUpdateTransform();
	}
}

void AUTAmmoPickupFactory::TransformAmmoType(UClass* NewAmmoClass)
{
	if (NewAmmoClass == NULL)
	{
		debugf(NAME_Warning, TEXT("NewAmmoClass of None passed to UTAmmoPickupFactory::TransformAmmoType()"));
	}
	else
	{
		UObject* OldDefault = GetClass()->GetDefaultObject();
		UObject* NewDefault = NewAmmoClass->GetDefaultObject();

		// create the instancing graph for copying components
		FObjectInstancingGraph InstancingGraph;
		InstancingGraph.SetDestinationRoot(this, NewDefault);

		ClearComponents();
		
		// copy non-native, non-duplicatetransient properties from this class and superclasses
		// skip editable properties that have been modified
		// only copy component properties from classes above PickupFactory
		// on client, also don't copy replicated properties (assume server sent correct value)
		for (UProperty* Property = StaticClass()->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
		{
			if ( !(Property->PropertyFlags & CPF_Native) && !(Property->PropertyFlags & CPF_DuplicateTransient) &&
				(!(Property->PropertyFlags & CPF_Edit) || Property->Identical((BYTE*)this + Property->Offset, (BYTE*)OldDefault + Property->Offset)) &&
				(Property->GetOwnerClass()->IsChildOf(APickupFactory::StaticClass()) || (Property->PropertyFlags & CPF_Component)) &&
				(Role == ROLE_Authority || !(Property->PropertyFlags & CPF_Net)) )
			{
				Property->CopyCompleteValue((BYTE*)this + Property->Offset, (BYTE*)NewDefault + Property->Offset, NULL, this, &InstancingGraph);
			}
		}

		// instance components we copied references to
		StaticClass()->InstanceComponentTemplates((BYTE*)this, (BYTE*)NewDefault, NewDefault->GetClass()->GetPropertiesSize(), this, &InstancingGraph);

		ForceUpdateComponents(FALSE, FALSE);

		TransformedClass = NewAmmoClass;

		eventInitPickupMeshEffects();
	}
}

void AUTWeaponLocker::CheckForErrors()
{
	Super::CheckForErrors();

	for (INT i = 0; i < Weapons.Num(); i++)
	{
		if (Weapons(i).WeaponClass != NULL && Weapons(i).WeaponClass->GetDefaultObject<AUTWeapon>()->bWarnIfInLocker)
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, this, *FString::Printf(TEXT("%s should not be in weapon lockers"), *Weapons(i).WeaponClass->GetName()));
		}
	}

	// Check for a pickup light being nearby

	UBOOL bHasLight = FALSE;

	for( FActorIterator It; It; ++It )
	{
		AActor* actor = Cast<AActor>( *It );

		if( Cast<AUTWeaponLockerPickupLight>( actor ) && FPointsAreNear( actor->Location, Location, 16.0f ) )
		{
			bHasLight = TRUE;
		}
	}

	if( !bHasLight )
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("%s doesn't have a pickup light near it"), *GetName()));
	}
}
void AUTArmorPickupFactory::CheckForErrors()
{
	Super::CheckForErrors();

	// Check for a pickup light being nearby

	UBOOL bHasLight = FALSE;

	for( FActorIterator It; It; ++It )
	{
		AActor* actor = Cast<AActor>( *It );

		if( Cast<AUTArmorPickupLight>( actor ) && FPointsAreNear( actor->Location, Location, 16.0f ) )
		{
			bHasLight = TRUE;
		}
	}

	if( !bHasLight )
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("%s doesn't have a pickup light near it"), *GetName()));
	}
}

void AUTHealthPickupFactory::CheckForErrors()
{
	Super::CheckForErrors();

	// Health vials don't have lights so ignore them for this check

	if( !Cast<AUTPickupFactory_HealthVial>(this) )
	{
		// Check for a pickup light being nearby

		UBOOL bHasLight = FALSE;

		for( FActorIterator It; It; ++It )
		{
			AActor* actor = Cast<AActor>( *It );

			if( Cast<AUTHealthPickupLight>( actor ) && FPointsAreNear( actor->Location, Location, 16.0f ) )
			{
				bHasLight = TRUE;
			}
		}

		if( !bHasLight )
		{
			GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("%s doesn't have a pickup light near it"), *GetName()));
		}
	}
}

void AUTWeaponLocker::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	if ( bIsActive && (WorldInfo->NetMode != NM_DedicatedServer) )
	{
		if ( bPlayerNearby && bScalingUp )
		{
			CurrentWeaponScaleX = CurrentWeaponScaleX + (ScaleRate * DeltaTime);
			if ( CurrentWeaponScaleX >= 1.f )
			{
				bScalingUp = false;
				CurrentWeaponScaleX = 1.f;
			}
			for (INT i = 0; i < Weapons.Num(); i++)
			{
				if ( Weapons(i).PickupMesh )
				{
					Weapons(i).PickupMesh->Scale3D.X = CurrentWeaponScaleX;
					// this is just visual, so we don't need to update it immediately
					Weapons(i).PickupMesh->BeginDeferredUpdateTransform();
				}
			}
		}

		if ( WorldInfo->TimeSeconds > NextProximityCheckTime )
		{
			NextProximityCheckTime = WorldInfo->TimeSeconds + 0.15f + 0.1f * appFrand();
			UBOOL bNewPlayerNearby = FALSE;
			APlayerController *NearbyPC = NULL;
			for( INT iPlayers=0; iPlayers<GEngine->GamePlayers.Num(); iPlayers++ )
			{
				if ( GEngine->GamePlayers(iPlayers) && GEngine->GamePlayers(iPlayers)->Actor )
				{
					APlayerController *PC = GEngine->GamePlayers(iPlayers)->Actor;
					if ( PC->Pawn && ((Location - PC->Pawn->Location).SizeSquared()< ProximityDistanceSquared) )
					{
						bNewPlayerNearby = TRUE;
						NearbyPC = PC;
						break;
					}
				}
			}
			if ( bNewPlayerNearby != bPlayerNearby )
			{
				eventSetPlayerNearby(NearbyPC, bNewPlayerNearby, true);
			}
		}
	}

}

void AUTVehicleFactory::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);
	
	if (RespawnProgress > 0.f)
	{
		RespawnProgress -= DeltaSeconds * RespawnRateModifier;
		if (RespawnProgress <= 0.f)
		{
			eventSpawnVehicle();
		}
	}
}

void AUTVehicleFactory::CheckForErrors()
{
	Super::CheckForErrors();

	// throw an error if multiple enabled vehicle factories overlap
	UBOOL bFoundThis = FALSE;
	for (FActorIterator It; It; ++It)
	{
		// don't start testing until we've found ourselves - avoids getting two errors for each pair of factories that overlap
		if (*It == this)
		{
			bFoundThis = TRUE;
		}
		else if (bFoundThis)
		{
			AUTVehicleFactory* OtherFactory = Cast<AUTVehicleFactory>(*It);
			// if they overlap and they'll both spawn vehicles for the same team
			if ( OtherFactory != NULL && (OtherFactory->Location - Location).SizeSquared() < Square(CylinderComponent->CollisionRadius + OtherFactory->CylinderComponent->CollisionRadius) &&
					(TeamSpawningControl == TS_All || OtherFactory->TeamSpawningControl == TS_All || TeamSpawningControl == OtherFactory->TeamSpawningControl) )
			{
				// check if they can both be active simultaneously
				UBOOL bActiveSimultaneously = (!bDisabled && !OtherFactory->bDisabled);
				FName ActiveLinkSetup = NAME_None;

				if (bActiveSimultaneously)
				{
					if (ActiveLinkSetup != NAME_None)
					{
						GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Vehicle factory is too close to other factory '%s' that can be active simultaneously in link setup '%s'"), *OtherFactory->GetName(), *ActiveLinkSetup.ToString()));
					}
					else
					{
						GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Vehicle factory is too close to other factory '%s' that can be active simultaneously"), *OtherFactory->GetName()));
					}
				}
			}
		}
	}
}

void AUTVehicleFactory::Spawned()
{
#if !CONSOLE
	if (VehicleClassPath != TEXT(""))
	{
		VehicleClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), NULL, *VehicleClassPath, NULL, LOAD_NoWarn, NULL));
	}
#endif

	Super::Spawned();
}

void AUTVehicleFactory::PostLoad()
{
#if !CONSOLE
	if (GetLinkerVersion() < VER_UTVEHICLEFACTORY_USE_STRING_CLASS && VehicleClassPath != TEXT(""))
	{
		VehicleClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), NULL, *VehicleClassPath, NULL, LOAD_NoWarn, NULL));
	}
#endif

	Super::PostLoad();
}

void AUTVehicleFactory::StripData(UE3::EPlatformType TargetPlatform)
{
	if (bIgnoreOnPS3 && TargetPlatform == UE3::PLATFORM_PS3)
	{
		VehicleClass = NULL;
		VehicleClassPath = TEXT("");
		ClearComponents();
		Components.Empty();
	}

	Super::StripData(TargetPlatform);
}

//======================================================================
// Commandlet for level error checking
IMPLEMENT_CLASS(UUTLevelCheckCommandlet)

INT UUTLevelCheckCommandlet::Main( const FString& Params )
{
	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// Iterate over all level packages.
	FString MapExtension = GConfig->GetStr(TEXT("URL"), TEXT("MapExt"), GEngineIni);
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		if (Filename.GetExtension() == MapExtension)
		{
			warnf(NAME_Log, TEXT("------------------------------------"));
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);
			warnf(NAME_Log, TEXT("...................................."));

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage*	Package = CastChecked<UPackage>(UObject::LoadPackage( NULL, *Filename, 0 ));

			// We have to use GetPackageLinker as GetLinker() returns NULL for top level packages.
			BeginLoad();
			ULinkerLoad* Linker	= UObject::GetPackageLinker( Package, NULL, 0, NULL, NULL );
			EndLoad();

			warnf(NAME_Log, TEXT("Checking Map..."), *Filename);

			// Check all actors for Errors

			for( TObjectIterator<AActor> It; It; ++It )
			{
				AActor* Actor = *It;
				if (Actor)
				{
					Actor->CheckForErrors();
				}
					
			}

			const UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			if ( World )
			{
				AWorldInfo* WorldInfo = World->GetWorldInfo();
				if ( WorldInfo )
					warnf(NAME_Log,TEXT("Gravity %f"),WorldInfo->GlobalGravityZ);
			}

			warnf(NAME_Log, TEXT("...................................."));
			warnf(NAME_Log, TEXT("Process Completed"));
			warnf(NAME_Log, TEXT("------------------------------------"));
			warnf(NAME_Log, TEXT(""));

			// Garbage collect to restore clean plate.
			UObject::CollectGarbage(RF_Native);
		}
	}
	return 0;
}

//======================================================================
// Commandlet for replacing one kind of actor with another kind of actor, copying changed properties from the most-derived common superclass
IMPLEMENT_CLASS(UUTReplaceActorCommandlet)

INT UUTReplaceActorCommandlet::Main(const FString& Params)
{
	const TCHAR* Parms = *Params;

	// get the specified filename/wildcard
	FString PackageWildcard;
	if (!ParseToken(Parms, PackageWildcard, 0))
	{
		warnf(TEXT("Syntax: ucc utreplaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
		return 1;
	}
	
	// find all the files matching the specified filename/wildcard
	TArray<FString> FilesInPath;
	GFileManager->FindFiles(FilesInPath, *PackageWildcard, 1, 0);
	if (FilesInPath.Num() == 0)
	{
		warnf(NAME_Error, TEXT("No packages found matching %s!"), *PackageWildcard);
		return 2;
	}
	// get the directory part of the filename
	INT ChopPoint = Max(PackageWildcard.InStr(TEXT("/"), 1) + 1, PackageWildcard.InStr(TEXT("\\"), 1) + 1);
	if (ChopPoint < 0)
	{
		ChopPoint = PackageWildcard.InStr( TEXT("*"), 1 );
	}
	FString PathPrefix = (ChopPoint < 0) ? TEXT("") : PackageWildcard.Left(ChopPoint);

	// get the class to remove and the class to replace it with
	FString ClassName;
	if (!ParseToken(Parms, ClassName, 0))
	{
		warnf(TEXT("Syntax: ucc utreplaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
		return 1;
	}
	UClass* ClassToReplace = (UClass*)StaticLoadObject(UClass::StaticClass(), NULL, *ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	if (ClassToReplace == NULL)
	{
		warnf(NAME_Error, TEXT("Invalid class to remove: %s"), *ClassName);
		return 4;
	}
	else
	{
		ClassToReplace->AddToRoot();
	}
	if (!ParseToken(Parms, ClassName, 0))
	{
		warnf(TEXT("Syntax: ucc utreplaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
		return 1;
	}
	UClass* ReplaceWithClass = (UClass*)StaticLoadObject(UClass::StaticClass(), NULL, *ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	if (ReplaceWithClass == NULL)
	{
		warnf(NAME_Error, TEXT("Invalid class to replace with: %s"), *ClassName);
		return 5;
	}
	else
	{
		ReplaceWithClass->AddToRoot();
	}

	// find the most derived superclass common to both classes
	UClass* CommonSuperclass = NULL;
	for (UClass* BaseClass1 = ClassToReplace; BaseClass1 != NULL && CommonSuperclass == NULL; BaseClass1 = BaseClass1->GetSuperClass())
	{
		for (UClass* BaseClass2 = ReplaceWithClass; BaseClass2 != NULL && CommonSuperclass == NULL; BaseClass2 = BaseClass2->GetSuperClass())
		{
			if (BaseClass1 == BaseClass2)
			{
				CommonSuperclass = BaseClass1;
			}
		}
	}
	checkSlow(CommonSuperclass != NULL);

	for (INT i = 0; i < FilesInPath.Num(); i++)
	{
		const FString& PackageName = FilesInPath(i);
		// get the full path name to the file
		FString FileName = PathPrefix + PackageName;
		// skip if read-only
		if (GFileManager->IsReadOnly(*FileName))
		{
			warnf(TEXT("Skipping %s (read-only)"), *FileName);
		}
		else
		{
			// clean up any previous world
			if (GWorld != NULL)
			{
				GWorld->CleanupWorld();
				GWorld->RemoveFromRoot();
				GWorld = NULL;
			}
			// load the package
			warnf(TEXT("Loading %s..."), *PackageName); 
			UPackage* Package = LoadPackage(NULL, *PackageName, LOAD_None);

			// load the world we're interested in
			GWorld = FindObject<UWorld>(Package, TEXT("TheWorld"));
			if (GWorld == NULL)
			{
				warnf(TEXT("Skipping %s (not a map)"));
			}
			else
			{
				// add the world to the root set so that the garbage collection to delete replaced actors doesn't garbage collect the whole world
				GWorld->AddToRoot();
				// initialize the levels in the world
				GWorld->Init();
				GWorld->GetWorldInfo()->PostEditChange(NULL);
				// iterate through all the actors in the world, looking for matches with the class to replace (must have exact match, not subclass)
				for (FActorIterator It; It; ++It)
				{
					AActor* OldActor = *It;
					if (OldActor->GetClass() == ClassToReplace)
					{
						// replace an instance of the old actor
						warnf(TEXT("Replacing actor %s"), *OldActor->GetName());
						// make sure we spawn the new actor in the same level as the old
						//@warning: this relies on the outer of an actor being the level
						GWorld->CurrentLevel = OldActor->GetLevel();
						checkSlow(GWorld->CurrentLevel != NULL);
						// spawn the new actor
						AActor* NewActor = GWorld->SpawnActor(ReplaceWithClass, NAME_None, OldActor->Location, OldActor->Rotation, NULL, TRUE);
						// copy non-native non-transient properties common to both that were modified in the old actor to the new actor
						for (UProperty* Property = CommonSuperclass->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
						{
							//@note: skipping properties containing components - don't have a reasonable way to deal with them and the instancing mess they create
                            // to  hack around this for now you can do:
                           // (!(Property->PropertyFlags & CPF_Component) || Property->GetFName() == FName(TEXT("Weapons"))) &&
							if ( !(Property->PropertyFlags & CPF_Native) && !(Property->PropertyFlags & CPF_Transient) &&
								 !(Property->PropertyFlags & CPF_Component) &&
								 !Property->Identical((BYTE*)OldActor + Property->Offset, (BYTE*)OldActor->GetClass()->GetDefaultObject() + Property->Offset) )
							{
								Property->CopyCompleteValue((BYTE*)NewActor + Property->Offset, (BYTE*)OldActor + Property->Offset);
							}
						}
						// destroy the old actor
						//@warning: must do this before replacement so the new Actor doesn't get the old Actor's entry in the level's actor list (which would cause it to be in there twice)
						GWorld->DestroyActor(OldActor);
						check(OldActor->IsValid()); // make sure DestroyActor() doesn't immediately trigger GC since that'll break the reference replacement
						// check for any references to the old Actor and replace them with the new one
						TMap<AActor*, AActor*> ReplaceMap;
						ReplaceMap.Set(OldActor, NewActor);
						FArchiveReplaceObjectRef<AActor> ReplaceAr(GWorld, ReplaceMap, FALSE, FALSE, FALSE);
					}
					else
					{
						// check for any references to the old class and replace them with the new one
						TMap<UClass*, UClass*> ReplaceMap;
						ReplaceMap.Set(ClassToReplace, ReplaceWithClass);
						FArchiveReplaceObjectRef<UClass> ReplaceAr(*It, ReplaceMap, FALSE, FALSE, FALSE);
						if (ReplaceAr.GetCount() > 0)
						{
							warnf(TEXT("Replaced %i class references in actor %s"), ReplaceAr.GetCount(), *It->GetName());
						}
					}
				}
				// replace Kismet references to the class
				USequence* Sequence = GWorld->GetGameSequence();
				if (Sequence != NULL)
				{
					TMap<UClass*, UClass*> ReplaceMap;
					ReplaceMap.Set(ClassToReplace, ReplaceWithClass);
					FArchiveReplaceObjectRef<UClass> ReplaceAr(Sequence, ReplaceMap, FALSE, FALSE, FALSE);
					if (ReplaceAr.GetCount() > 0)
					{
						warnf(TEXT("Replaced %i class references in Kismet"), ReplaceAr.GetCount());
					}
				}

				// collect garbage to delete replaced actors and any objects only referenced by them (components, etc)
				GWorld->PerformGarbageCollection();

				// save the world
				warnf(TEXT("Saving %s..."), *PackageName);
				SavePackage(Package, GWorld, 0, *FileName, GWarn);
				// clear GWorld by removing it from the root set and replacing it with a new one
				GWorld->CleanupWorld();
				GWorld->RemoveFromRoot();
				GWorld = NULL;
			}
		}
		
		// get rid of the loaded world
		warnf(TEXT("Cleaning up..."));
		CollectGarbage(RF_Native);
	}

	// UEditorEngine::FinishDestroy() expects GWorld to exist
	UWorld::CreateNew();

	return 0;
}

AActor* UUTActorFactoryVehicle::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* Actor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	AUTVehicle* Vehicle = Cast<AUTVehicle>(Actor);
	if (Vehicle)
	{
		if ( Vehicle->bIsNecrisVehicle )
		{
			// check whether necris locked (for campaign)
			AUTGame* Game = Cast<AUTGame>(Vehicle->WorldInfo->Game);
			if ( Game && Game->bNecrisLocked )
			{
				bTeamLocked = TRUE;
				TeamNum = 1;
				Vehicle->bEnteringUnlocks = FALSE;
			}
		}
		Vehicle->eventSetTeamNum(TeamNum);
		Vehicle->bTeamLocked = bTeamLocked;
		if (bKeyVehicle)
		{
			Vehicle->eventSetKeyVehicle();
		}
		// actor factories could spawn the vehicle anywhere, so make sure it's awake so it doesn't end up floating or something
		if (Vehicle->Mesh != NULL)
		{
			Vehicle->Mesh->WakeRigidBody();
		}
	}
	return Actor;
}

void AUTCarriedObject::PostNetReceiveBase(AActor* NewBase)
{	
	APawn* P = NewBase ? NewBase->GetAPawn() : NULL;
	if ( !P )
	{
		Super::PostNetReceiveBase(NewBase);
		return;
	}
	if ( NewBase != Base )
	{
		if (P->IsA(AUTPawn::StaticClass()))
		{
			((AUTPawn*)P)->eventHoldGameObject(this);
		}
		else if (P->IsA(AUTVehicleBase::StaticClass()))
		{
			((AUTVehicleBase*)P)->eventHoldGameObject(this);
		}
	}
	bJustTeleported = 0;
}

void AUTGame::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

#if FORCELOWGORE
	((AGameInfo *)(AGameInfo::StaticClass()->GetDefaultActor()))->GoreLevel = 1;
#endif	
}

void AUTGame::AddSupportedGameTypes(AWorldInfo* Info, const TCHAR* WorldFilename) const
{
	// match the map prefix
	FString BaseFilename = FFilename(WorldFilename).GetBaseFilename();
	TArray<FGameTypePrefix> Prefixes = DefaultMapPrefixes;
	Prefixes += CustomMapPrefixes;
	for (INT i = 0; i < Prefixes.Num(); i++)
	{
		if (BaseFilename.StartsWith(Prefixes(i).Prefix))
		{
			UClass* GameClass = StaticLoadClass(AGameInfo::StaticClass(), NULL, *Prefixes(i).GameType, NULL, LOAD_None, NULL);
			if (GameClass != NULL)
			{
				Info->GameTypesSupportedOnThisMap.AddUniqueItem(GameClass);
			}
			for (INT j = 0; j < Prefixes(i).AdditionalGameTypes.Num(); j++)
			{
				GameClass = StaticLoadClass(AGameInfo::StaticClass(), NULL, *Prefixes(i).AdditionalGameTypes(j), NULL, LOAD_None, NULL);
				if (GameClass != NULL)
				{
					Info->GameTypesSupportedOnThisMap.AddUniqueItem(GameClass);
				}
			}
			break;
		}
	}
}

AActor* UUTActorFactoryMover::CreateActor(const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData)
{
	AActor* Actor = Super::CreateActor(Location, Rotation, ActorFactoryData);

	if (bCreateKismetEvent && Actor != NULL && Actor->SupportedEvents.FindItemIndex(EventClass) != INDEX_NONE)
	{
		USequence* GameSequence = GWorld->GetGameSequence();
		if (GameSequence != NULL)
		{
			GameSequence->Modify();

			// create a new sequence to put the new event in
			USequence* NewSequence = ConstructObject<USequence>(USequence::StaticClass(), GameSequence, FName(*Actor->GetName()), RF_Transactional);
			NewSequence->ParentSequence = GameSequence;
			GameSequence->SequenceObjects.AddItem(NewSequence);
			NewSequence->ObjName = NewSequence->GetName();
			NewSequence->OnCreated();
			NewSequence->Modify();

			// now create the event
			USequenceEvent* NewEvent = ConstructObject<USequenceEvent>(EventClass, NewSequence, NAME_None, RF_Transactional);
			NewEvent->ObjName = FString::Printf(TEXT("%s %s"), *Actor->GetName(), *NewEvent->ObjName);
			NewEvent->Originator = Actor;
			NewEvent->ParentSequence = NewSequence;
			NewSequence->SequenceObjects.AddItem(NewEvent);
			NewEvent->OnCreated();
			NewEvent->Modify();
		}
	}

	return Actor;
}

AActor* UUTActorFactoryPickup::CreateActor(const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData)
{
	ADroppedPickup* Pickup = NULL;
	// spawn the inventory actor
	AInventory* NewInventory = Cast<AInventory>(GWorld->SpawnActor(InventoryClass, NAME_None, *Location, *Rotation));
	if (NewInventory != NULL)
	{
		// if it has its own DroppedPickupClass, use it, otherwise stick with the default
		if (NewInventory->DroppedPickupClass != NULL)
		{
			NewActorClass = NewInventory->DroppedPickupClass;
		}
		Pickup = Cast<ADroppedPickup>(Super::CreateActor(Location, Rotation, ActorFactoryData));
		if (Pickup == NULL)
		{
			GWorld->DestroyActor(NewInventory);
		}
		else
		{
			Pickup->setPhysics(PHYS_Falling);
			Pickup->Inventory = NewInventory;
			Pickup->InventoryClass = InventoryClass;
			Pickup->eventSetPickupMesh(Pickup->Inventory->DroppedPickupMesh);
		}
	}
	return Pickup;
}
		  
/** 
 * @See AActor::physicRotation
 */
void AUTProj_Grenade::physicsRotation(FLOAT deltaTime)
{
	// Accumulate a desired new rotation.
	FRotator NewRotation = Rotation + (RotationRate * deltaTime);

	// Set the new rotation.
	// fixedTurn() returns denormalized results so we must convert Rotation to prevent negative values in Rotation from causing unnecessary MoveActor() calls
	if (NewRotation != Rotation.GetDenormalized())
	{
		FCheckResult Hit(1.f);
		GWorld->MoveActor( this, FVector(0,0,0), NewRotation, 0, Hit );
	}
}

void AUTKActor::physRigidBody(FLOAT DeltaTime)
{
	Super::physRigidBody(DeltaTime);
	FRigidBodyState OutState;

	if ( bDamageOnEncroachment && bResetDOEWhenAsleep && ( !GetCurrentRBState(OutState) || ( DOEResetThreshold>0 && Velocity.Size() < DOEResetThreshold ) ) )
	{
		bDamageOnEncroachment = FALSE;
	}
}

void AUTTimedPowerup::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	if (Owner != NULL && TimeRemaining > 0.0f)
	{
		CustomTimeDilation = Owner->CustomTimeDilation;
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			eventTimeExpired();
		}
	}
	else
	{
		CustomTimeDilation = 1.f;
	}
}

AActor* UUTActorFactoryAI::CreateActor(const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData)
{
	APawn* NewPawn = (APawn*)Super::CreateActor(Location, Rotation, ActorFactoryData);
	if (bForceDeathmatchAI && NewPawn != NULL)
	{
		AUTBot* Bot = Cast<AUTBot>(NewPawn->Controller);
		if (Bot != NULL)
		{
			Bot->Squad = (AUTSquadAI*)GWorld->SpawnActor(AUTSquadAI::StaticClass());
			if (Bot->Squad != NULL)
			{
				if (Bot->PlayerReplicationInfo != NULL)
				{
					Bot->Squad->Team = Cast<AUTTeamInfo>(Bot->PlayerReplicationInfo->Team);
				}
				Bot->Squad->eventSetLeader(Bot);
			}
		}
	}

	// Set the 'use hardware for physics' flag as desired on the spawned Pawn.
	if(bUseCompartment && NewPawn && NewPawn->Mesh)
	{
		NewPawn->Mesh->bUseCompartment = TRUE;
	}

	return NewPawn;
}


void AUTCTFFlag::PostNetReceiveLocation()
{
	Super::PostNetReceiveLocation();
	//debugf(TEXT("POST RECEIVE FLAG"));
	SkelMesh->ResetClothVertsToRefPose();
}

void AForcedDirVolume::PostEditChange(UProperty* Property)
{
	ArrowDirection = Arrow->Rotation.Vector();
}

UBOOL AForcedDirVolume::IgnoreBlockingBy( const AActor *Other ) const
{
	return (!bBlockPawns 
			|| (Other->GetAProjectile() != NULL)
			|| (Other->IsA(TypeToForce))); // ignore everything if we're not blocking pawns; if we are blocking them ignore projectiles and whatever we force in a direction.
}

void AForcedDirVolume::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// push any vehicles inside that aren't moving out of the volume fast enough
	for (INT i=0; i<TouchingVehicles.Num(); i++)
	{
		AUTVehicle* V = TouchingVehicles(i);
		if ( !V || V->bDeleteMe )
		{
			TouchingVehicles.RemoveItem(V);
			i--;
		}
		else
		{
			FVector AdjustedVelocity = V->Velocity - (1100.f * ArrowDirection);
			FLOAT AdjVelMag = AdjustedVelocity.Size();
			if((AdjVelMag > KINDA_SMALL_NUMBER) && ((AdjustedVelocity | ArrowDirection) < 0.f))
			{
				V->Mesh->AddForce(ArrowDirection * AdjVelMag);
			}
		}
	}
}

/** Helper class to sort online player scores used in SortPlayerScores. Must be outside of the function or gcc can't compile it */
class OnlinePlayerScoreSorter
{
public:
	static inline INT Compare(const FOnlinePlayerScore& A,const FOnlinePlayerScore& B)
	{
		// Sort descending...
		return B.Score - A.Score;
	}
};

/**
 * Sorts the scores and assigns relative positions to the players
 *
 * @param PlayerScores the raw scores before sorting and relative position setting
 */
void AUTGame::SortPlayerScores(TArray<FOnlinePlayerScore>& PlayerScores)
{
	Sort<FOnlinePlayerScore,OnlinePlayerScoreSorter>(PlayerScores.GetTypedData(),PlayerScores.Num());
	// Now that the items are sorted, assign relative positions
	for (INT Idx = 0; Idx < PlayerScores.Num(); Idx++)
	{
		// Total players minus position equals their relative position, ie 1st is highest
		PlayerScores(Idx).Score = PlayerScores.Num() - Idx;
	}
}

void UUTMapInfo::CheckForErrors()
{
	Super::CheckForErrors();

	if( MapMusicInfo == NULL )
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, NULL, TEXT("Map does not have a MapMusicInfo set."));
	}
	for (FActorIterator It; It; ++It)
	{
		if (It->GetClass() == ATeleporter::StaticClass())
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, *It, TEXT("Maps should use UTTeleporters instead of Teleporters"), MCACTION_DELETE);
		}
	}
}
/**
 * Updates the Map Location for a given world location
 */
FVector UUTMapInfo::UpdateHUDLocation( FVector InLocation )
{
    FVector ScreenLocation = InLocation - ActualMapCenter;

	if ( ScreenLocation.SizeSquared() > Square(0.55f*RadarRange) )
	{
		// draw on circle if extends past edge
		ScreenLocation = 0.55f*RadarRange * ScreenLocation.SafeNormal();
	}
	FVector NewHUDLocation;
	FLOAT Scaling = RadarWidth/RadarRange;
	NewHUDLocation.X = CenterPos.X + (ScreenLocation | MapRotX) * Scaling;
	NewHUDLocation.Y = CenterPos.Y + (ScreenLocation | MapRotY) * Scaling;
	NewHUDLocation.Z = 0.f;

	return NewHUDLocation;
}

/**
 * Returns the index of a given map in the array or INDEX_None if it doesn't exist
 *
 * @Param MapId		The ID to look up.
 */
INT AUTVoteReplicationInfo::GetMapIndex(INT MapId)
{
	for (INT i=0;i<Maps.Num(); i++)
	{
		if (Maps(i).MapId == MapId)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

/**
 * Look for the setting of the owner 
 */
void AUTVoteReplicationInfo::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);
	if (OldOwner != Owner && Owner && GWorld->GetWorldInfo()->NetMode == NM_Client)
	{
		eventClientHasOwner();
	}
	OldOwner = Owner;
}


/**
 * Returns the index of a given map in the array or INDEX_None if it doesn't exist
 *
 */
INT AUTVoteCollector::GetMapIndex(INT MapId)
{
	for (INT i=0;i<Votes.Num(); i++)
	{
		if (Votes(i).MapId == MapId)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void AUTEmitterPool::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	INT i = 0;
	while (i < RelativeExplosionLights.Num())
	{
		if (RelativeExplosionLights(i).Light == NULL || RelativeExplosionLights(i).Base == NULL || RelativeExplosionLights(i).Base->bDeleteMe)
		{
			RelativeExplosionLights.Remove(i, 1);
		}
		else
		{
			FVector NewTranslation = RelativeExplosionLights(i).Base->Location + RelativeExplosionLights(i).RelativeLocation;
			if (RelativeExplosionLights(i).Light->Translation != NewTranslation)
			{
				RelativeExplosionLights(i).Light->Translation = NewTranslation;
				RelativeExplosionLights(i).Light->BeginDeferredUpdateTransform();
			}
			i++;
		}
	}
}

BYTE AUTGameObjective::GetTeamNum()
{
	return DefenderTeamIndex;
}


/**
 * Locates a random localized hint message string for the specified two game types.  Usually the first game type
 * should always be "DM", since you always want those strings included regardless of game type
 *
 * @param GameType1Name Name of the first game type we're interested in
 * @param GameType2Name Name of the second game type we're interested in
 *
 * @return Returns random hint string for the specified game types
 */
FString UUTGameViewportClient::LoadRandomLocalizedHintMessage( const FString& GameType1Name, const FString& GameType2Name )
{
	// Loc package
	const FString LocFileName = TEXT( "UTGameUI" );

	// This string in the loc file will specify the total number of hint messages in this category.  We'll read
	// in its value string and convert it to an integer for our tests
	const FString HintCountVarName = TEXT( "Hint_Count" );

	// This is the prefix before the category name for hints (e.g. LoadingHints_UTDeathmatch)
	const FString HintCategoryPrefix = TEXT( "LoadingHints_" );

	// Prefix before each hint message, usually followed by a number (index of that hint)
	const FString HintMessageVarPrefix = TEXT( "Hint_" );


	// How many games are we dealing with?
	const INT MaxGameTypesForHints = 2;
	INT GameTypeHintCounts[ MaxGameTypesForHints ];


	// Figure out how many total hints we have available to us for the specified game types
	INT TotalHintCount = 0;
	for( INT GameIndex = 0; GameIndex < MaxGameTypesForHints; ++GameIndex )
	{
		// Setup category name
		const FString CurGameTypeName = ( GameIndex == 0 ) ? GameType1Name : GameType2Name;
		const FString HintCategoryName = HintCategoryPrefix + CurGameTypeName;

		// Load the number of available hints for this category
		FString HintCountValueString = Localize( *HintCategoryName, *HintCountVarName, *LocFileName );
		INT HintCountValue = 0;
		if( HintCountValueString.Len() > 0 )
		{
			// Convert the hint count from a string
			HintCountValue = appAtoi( *HintCountValueString );
		}
		else
		{
			warnf(
				TEXT( "Hint System: Unable to locate hint count [%s] for category [%s] in loc file [%s]." ),
				*HintCountVarName,
				*HintCategoryName,
				*LocFileName );
		}

		// Store hint count
		GameTypeHintCounts[ GameIndex ] = HintCountValue;

		// Also, total everything up!
		TotalHintCount += HintCountValue;
	}

	
	// OK, now choose a random hint!
	INT RandomHintIndex = TotalHintCount > 0 ? appRand() % TotalHintCount : 0;


	// Figure out which hint category we landed in and
	FString HintMessage;
	for( INT GameIndex = 0; GameIndex < MaxGameTypesForHints; ++GameIndex )
	{
		if( RandomHintIndex < GameTypeHintCounts[ GameIndex ] )
		{
			// Found it!  Select our hint!
			FString HintMessageVarName = HintMessageVarPrefix + appItoa( RandomHintIndex );

			// Setup category name
			const FString CurGameTypeName = ( GameIndex == 0 ) ? GameType1Name : GameType2Name;
			const FString HintCategoryName = HintCategoryPrefix + CurGameTypeName;

			HintMessage = Localize( *HintCategoryName, *HintMessageVarName, *LocFileName );
			if( HintMessage.Len() == 0 )
			{
				warnf(
					TEXT( "Hint System: Unable to locate hint message [%s] for category [%s] in loc file [%s]." ),
					*HintMessageVarName,
					*HintCategoryName,
					*LocFileName );
			}

			break;
		}

		RandomHintIndex -= GameTypeHintCounts[ GameIndex ];
	}


	// Done!  Return the hint message we found.
	return HintMessage;
}


void UUTGameViewportClient::SetDropDetail(FLOAT DeltaSeconds)
{
	Super::SetDropDetail(DeltaSeconds);

	// reduce detail if any local player is in a vehicle that requests it
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	if (WorldInfo->GetDetailMode() < DM_Medium)
	{
		for (FPlayerIterator It(GEngine); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->ViewTarget != NULL)
			{
				AUTVehicle* V = Cast<AUTVehicle>(It->Actor->ViewTarget);
				if (V == NULL && It->Actor->ViewTarget->GetAPawn() != NULL)
				{
					V = Cast<AUTVehicle>(((APawn*)It->Actor->ViewTarget)->GetVehicleBase());
				}
				if (V != NULL && V->bDropDetailWhenDriving)
				{
					WorldInfo->bDropDetail = TRUE;
					break;
				}
			}
		}
	}
}

/** utility for portals to update the VisiblePortals on Controllers */
static void CheckPortalVisible(AActor* Source, AActor* Destination, USceneCaptureComponent* PortalCaptureComponent)
{
	FVisiblePortalInfo PortalInfo(Source, Destination);
	for (AController* C = Source->WorldInfo->ControllerList; C != NULL; C = C->NextController)
	{
		if (C->SightCounter < 0.0f)
		{
			AActor* ViewTarget = C->GetViewTarget();
			if ((ViewTarget->Location - Source->Location).SizeSquared() <= Square(PortalCaptureComponent->MaxUpdateDist))
			{
				FCheckResult Hit(1.0f);
				if (GWorld->SingleLineCheck(Hit, Source, Source->Location, ViewTarget->Location, TRACE_World | TRACE_StopAtAnyHit | TRACE_ComplexCollision))
				{
					// we are visible to C
					C->VisiblePortals.AddUniqueItem(PortalInfo);
				}
				else
				{
					C->VisiblePortals.RemoveItem(PortalInfo);
				}
			}
		}
	}
}

void AUTTeleporterBase::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

#if !PS3
	if (PortalCaptureComponent != NULL && PortalViewTarget != NULL)
	{
		CheckPortalVisible(this, PortalViewTarget, PortalCaptureComponent);
	}
#endif
}

/**
 * Resets the current keybindings for the specified playerowner to the defaults specified in the INI.
 *
 * @param InPlayerOwner	Player to get the default keybindings for.
 */
void UUTProfileSettings::ResetKeysToDefault(ULocalPlayer* InPlayerOwner)
{
	const FString ConfigName = TEXT( "Input" );

	if( GUseSeekFreeLoading && CONSOLE )
	{
		const FString IniPrefix = "";
		const FString DefaultIniFilename= appGameConfigDir() + FString(GGameName) + FString::Printf( TEXT( "%s%s.ini"), *ConfigName, TEXT("Defaults") );

		FConfigFile* ExistingConfigFile = GConfig->FindConfigFile(*DefaultIniFilename);
		check(ExistingConfigFile);
		// need to copy off the config file, as the SetFile below may reallocate memory that the ExistingConfigFile is inside of
		FConfigFile ConfigFileCopy = *ExistingConfigFile;

		GConfig->SetFile( TEXT("PlatformInput.ini"), &ConfigFileCopy);
		//warnf( TEXT( "UUTUIKeyBindingList DefaultIniFilename: %s" ), *DefaultIniFilename );
	}
	else
	{
		const FString IniPrefix = PC_DEFAULT_INI_PREFIX;
		const FString DefaultIniFilename = appGameConfigDir() * FString::Printf( TEXT( "%s%s.ini"), *IniPrefix, *ConfigName );

		// build a new .ini file for the specified platform
		FConfigFile PlatformWeaponIni;
		PlatformWeaponIni.NoSave = TRUE;
		LoadAnIniFile(*DefaultIniFilename, PlatformWeaponIni, FALSE);

		// add it to the config cache so that LoadConfig() can find it
		static_cast<FConfigCacheIni*>(GConfig)->Set(TEXT("PlatformInput.ini"), PlatformWeaponIni);

		//warnf( TEXT( "UUTUIKeyBindingList ConfigNameToBeReplaced: %s" ), *DefaultIniFilename );
	}

	// If no player owner was specified, just use player 0.
	if(InPlayerOwner==NULL && GEngine && GEngine->GamePlayers.Num()>0)
	{
		InPlayerOwner=GEngine->GamePlayers(0);
	}

	if(InPlayerOwner != NULL)
	{
		AUTPlayerController* UTPC = Cast<AUTPlayerController>(InPlayerOwner->Actor);

		if(UTPC != NULL && UTPC->PlayerInput != NULL)
		{
			UProperty* BindingProperty = Cast<UProperty>(UTPC->PlayerInput->FindObjectField(TEXT("Bindings")));
			UTPC->PlayerInput->ReloadConfig(NULL, TEXT("PlatformInput.ini"), UE3::LCPF_None,BindingProperty);

			/* Debug output of current state of input bindings */
			if (0)
			{
				for (INT i=0;i<UTPC->PlayerInput->Bindings.Num(); i++)
				{
					const FKeyBind& KeyBind = UTPC->PlayerInput->Bindings(i);
					debugf(TEXT("%s = %s"), *KeyBind.Name.ToString(), *KeyBind.Command);
				}
			}
		}
	}
}


/** 
 * Sets the specified profile id back to its default value.
 * 
 * @param ProfileId	Profile setting to reset to default.
 */
void UUTProfileSettings::ResetToDefault(int ProfileId)
{
	for(INT ProfileIdx=0; ProfileIdx<ProfileSettings.Num(); ProfileIdx++)
	{
		if(ProfileSettings(ProfileIdx).ProfileSetting.PropertyId==ProfileId)
		{
			if(DefaultSettings.Num()>ProfileIdx && DefaultSettings(ProfileIdx).ProfileSetting.PropertyId==ProfileId)
			{
				ProfileSettings(ProfileIdx)=DefaultSettings(ProfileIdx);
			}

			break;
		}
	}
}

/**
* Increments the value for "mix it up" (win on each gametype)
*
* @param GameType the game type that was just won on
*
* @return true if all have been beaten, false otherwise
*/
UBOOL UUTProfileSettings::IncrementMixItUp(INT GameType)
{
	//Only support DM,TDM,DUEL,CTF,VCTF,WAR
	UBOOL bShouldUnlock = FALSE;

	if (GameType < 6)
	{   
		DWORD GameTypeMask = (0xF << GameType);
		INT CurrentValue = 0;
		
		// Read the value so we can or the match type in
		if (GetAchievementValue(UCONST_UTAID_ACHIEVEMENT_RANKED_MixItUp, CurrentValue) == TRUE)
		{
			CurrentValue |= GameTypeMask;
			// Now replace the value with the new one
			bShouldUnlock = UpdateAchievementBitMask(UCONST_UTAID_ACHIEVEMENT_RANKED_MixItUp, CurrentValue);
		}
		else
		{
			debugf(NAME_Error,TEXT("Internal badness in mix it up achievement"));
		}
	}	

	return bShouldUnlock;
}

/**
* Updates the around the world achievement with the map that was just won
* on. If all maps have been won upon, the achievement is complete
*
* @param MapContextId the map context id (see WarPC.DetermineMapStringId)
*
* @return true if the achievement should be granted, false otherwise
*/
UBOOL UUTProfileSettings::UpdateAroundTheWorld(INT MapContextId)
{
	DWORD MapMask = 1 << MapContextId;
	INT CurrentValue = 0;
	UBOOL bShouldUnlock = FALSE;

	// Read the value so we can OR the map bit in
	if (GetAchievementValue(UCONST_UTAID_ACHIEVEMENT_VERSUS_AroundTheWorld, CurrentValue) == TRUE)
	{
		CurrentValue |= MapMask;
		// Now replace the value with the new one
		bShouldUnlock = UpdateAchievementBitMask(UCONST_UTAID_ACHIEVEMENT_VERSUS_AroundTheWorld, CurrentValue);
	}
	else
	{
		debugf(NAME_Error,TEXT("Internal badness in 'around the world' achievement"));
	}
	// If all bits for the map list are set, then it's unlocked
	return bShouldUnlock;
}

/**
* Updates the around the world achievement with the map that was just won
* on. If all maps have been won upon, the achievement is complete
*
* @param MapContextId the map context id
*
* @return true if the achievement should be granted, false otherwise
*/
UBOOL UUTProfileSettings::UpdateGetItOn(INT MapContextId)
{
	DWORD MapMask = 1 << MapContextId;
	INT CurrentValue = 0;
	UBOOL bShouldUnlock = FALSE;

	// Read the value so we can OR the map bit in
	if (GetAchievementValue(UCONST_UTAID_ACHIEVEMENT_VERSUS_GetItOn, CurrentValue) == TRUE)
	{
		CurrentValue |= MapMask;
		// Now replace the value with the new one
		bShouldUnlock = UpdateAchievementBitMask(UCONST_UTAID_ACHIEVEMENT_VERSUS_GetItOn, CurrentValue);
	}
	else
	{
		debugf(NAME_Error,TEXT("Internal badness in 'get it on' achievement"));
	}
	// If all bits for the map list are set, then it's unlocked
	return bShouldUnlock;
}


// Update an achievement int32 by by treating it as a bitmask
// for stats like "complete all campaign", etc
// compares this value against a bitmask and returns if the achievement should be awarded
UBOOL UUTProfileSettings::UpdateAchievementBitMask(INT AchievementId, INT BitMask)
{
	UBOOL Unlocked = FALSE;
	INT Value;

	if (GetAchievementValue(AchievementId, Value))
	{
		Value |= BitMask;
		if (SetAchievementValue(AchievementId, Value))
		{
			INT UnlockCriteria;
			if (GetAchievementUnlockCriteria(AchievementId, UnlockCriteria))
			{
				Unlocked = (Value & UnlockCriteria);
			}
			else
			{
				warnf(TEXT("Failed to find achievement %d in achievements array"), AchievementId);
			}
		}
		else
		{
			warnf(TEXT("Failed to set achievement count for %d"), AchievementId);
		}
	}
	else
	{
		warnf(TEXT("Failed to get achievement count for %d"), AchievementId);
	}

	return Unlocked;
}

// Increment the particular achievement by Count
// basically maintains a count of how many times this achievement was accomplished
// compares this number against a total count and returns if the achievement should be awarded
UBOOL UUTProfileSettings::UpdateAchievementCount(INT AchievementId, INT Count)
{
	UBOOL Unlocked = FALSE;
	INT Value = 0;

	if (GetAchievementValue(AchievementId, Value))
	{
		Value += Count;
		if (SetAchievementValue(AchievementId, Value))
		{
			INT UnlockCriteria = 0;
			if (GetAchievementUnlockCriteria(AchievementId, UnlockCriteria))
			{
				Unlocked = (Value >= UnlockCriteria);
			}
			else
			{
				warnf(TEXT("Failed to find achievement %d in achievements array"),  AchievementId);
			}
		}
		else
		{
			warnf(TEXT("Failed to set achievement count for %d"), AchievementId);
		}
	}
	else
	{
		warnf(TEXT("Failed to get achievement count for %d"), AchievementId);
	}

	return Unlocked;
}

// Returns the value stored with this particular achievement
UBOOL UUTProfileSettings::GetAchievementValue(INT AchievementId, INT& Value)
{
	UBOOL Success = FALSE;

	INT AchievementIdx = UCONST_UTAID_Achievement_Start + AchievementId;
	if (UCONST_UTAID_Achievement_Start < AchievementIdx && AchievementIdx < UCONST_UTAID_Achievement_End)
	{
		Success = GetProfileSettingValueInt(AchievementIdx, Value);
	}
	else
	{
		warnf(TEXT("Invalid achievement ID: %d"), AchievementId);
	}

	return Success;
}



// Set the value for this particular achievement
UBOOL UUTProfileSettings::SetAchievementValue(INT AchievementId, INT Value)
{
	UBOOL Success = FALSE;

	INT AchievementIdx = UCONST_UTAID_Achievement_Start + AchievementId;
	if (UCONST_UTAID_Achievement_Start < AchievementIdx && AchievementIdx < UCONST_UTAID_Achievement_End)
	{
		Success = SetProfileSettingValueInt(AchievementIdx, Value);
	}
	else
	{
		warnf(TEXT("Invalid achievement ID: %d"), AchievementId);
	}

	return Success;
}

// Return the value (bitmask or count) of the achievement unlock criteria
UBOOL UUTProfileSettings::GetAchievementUnlockCriteria(INT AchievementId, INT& UnlockCriteria)
{
	UBOOL Success = FALSE;
	for (INT Idx = 0; Idx < AchievementsArray.Num(); Idx++)
	{
		if (AchievementId == AchievementsArray(Idx).Id)
		{
			UnlockCriteria = AchievementsArray(Idx).UnlockCriteria;
			Success = TRUE;
			break;
		}
	}

	return Success;
}

UBOOL UUTSeqVar_Character::MatchCharacter(AController* C)
{
	AUTPlayerReplicationInfo* PRI = Cast<AUTPlayerReplicationInfo>(C->PlayerReplicationInfo);
	if (PRI != NULL)
	{
		if (appStricmp(*PRI->PlayerName, *CharacterName) == 0)
		{
			return TRUE;
		}
		else if (PRI->SinglePlayerCharacterIndex >= 0)
		{
			AUTGameReplicationInfo* GRI = Cast<AUTGameReplicationInfo>(GWorld->GetWorldInfo()->GRI);
			return ( GRI != NULL && PRI->SinglePlayerCharacterIndex < ARRAY_COUNT(GRI->SinglePlayerBotNames) &&
				appStricmp(*GRI->SinglePlayerBotNames[PRI->SinglePlayerCharacterIndex], *CharacterName) == 0 );
		}
	}

	return FALSE;
}

UObject** UUTSeqVar_Character::GetObjectRef(INT Idx)
{
	if (Idx == 0)
	{
		if (GWorld != NULL)
		{
			AController* C = Cast<AController>(ObjValue);
			if (C == NULL || !MatchCharacter(C))
			{
				ObjValue = NULL;
				for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
				{
					if (MatchCharacter(C))
					{
						ObjValue = C;
						break;
					}
				}
			}
		}
	}

	return Super::GetObjectRef(Idx);
}

void UUTSeqAct_UnlockChar::Activated()
{
	Super::Activated();

	AWorldInfo* WI = GWorld->GetWorldInfo();
	for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
	{
		AUTPlayerController* UTPC = Cast<AUTPlayerController>(C);
		if(UTPC)
		{
			UTPC->UnlockChar(Char);
		}
	}
}

UBOOL AUTGame::IsPureGame()
{
	// campaigns always considered pure
	if ( SinglePlayerMissionID != INDEX_NONE )
	{
		return TRUE;
	}

	AUTGameReplicationInfo *UTGRI = Cast<AUTGameReplicationInfo>(GameReplicationInfo);
	if ( UTGRI && UTGRI->bStoryMode )
	{
		return TRUE;
	}

	// no mutators allowed
	if ( BaseMutator )
	{
		return FALSE;
	}

	// @TODO FIXMESTEVE - only official maps?
	// no modified scripts?
	// no listen server on PC?

	// only official gametypes
	debugf(TEXT("GAME PATH IS %s"), *GetClass()->GetName());
	if ( GetClass()->GetName() == TEXT("UTVehicleCTFGame_Content") )
	{
		return TRUE;
	}
	if ( GetClass()->GetName() == TEXT("UTCTFGame_Content") )
	{
		return TRUE;
	}
	if ( GetClass()->GetName() == TEXT("UTDeathmatch") )
	{
		return TRUE;
	}
	if ( GetClass()->GetName() == TEXT("UTTeamGame") )
	{
		return TRUE;
	}
	if ( GetClass()->GetName() == TEXT("UTDuelGame") )
	{
		return TRUE;
	}
	if ( GetClass()->GetName() == TEXT("UTGreedGame") )
	{
		return TRUE;
	}

	return FALSE;
}

void UUTSeqEvent_TouchStatus::ActivateEvent(AActor* InOriginator, AActor* InInstigator, TArray<INT>* ActivateIndices, UBOOL bPushTop, UBOOL bFromQueued)
{
	if (ActivateIndices != NULL && ActivateIndices->Num() > 0)
	{
		// only activate "touch" if first to touch; only activate "untouch" if last to untouch
		if (((*ActivateIndices)(0) == 0 && TouchedList.Num() == 0 || ((*ActivateIndices)(0) == 1 && TouchedList.Num() == 1)))
		{
			Super::ActivateEvent(InOriginator, InInstigator, ActivateIndices, bPushTop, bFromQueued);
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("Invalid ActivateIndices for UTSeqEvent_TouchStatus activation"));
	}
}

UBOOL UUTDemoRecDriver::InitListen(FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error)
{
	if (Super::InitListen(InNotify, ConnectURL, Error))
	{
		// try to add a UI entry for the new demo file
		UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
		if ( GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL && GEngine->GameViewport != NULL && DSClient != NULL )
		{
			UUTUIDataStore_MenuItems* MenuItemStore = Cast<UUTUIDataStore_MenuItems>(DSClient->FindDataStore(FName(TEXT("UTMenuItems")), GEngine->GamePlayers(0)));
			if (MenuItemStore != NULL)
			{
				FString DemoFilename = FFilename(ConnectURL.Map).GetCleanFilename();
				UBOOL bAlreadyInList = FALSE;
				TArray<UUIResourceDataProvider*> CurrentProviders;
				MenuItemStore->ListElementProviders.MultiFind(TEXT("DemoFiles"), CurrentProviders);
				for (INT i = 0; i < CurrentProviders.Num(); i++)
				{
					UUTUIDataProvider_DemoFile* Provider = Cast<UUTUIDataProvider_DemoFile>(CurrentProviders(i));
#if PLATFORM_UNIX
					if (Provider != NULL && appStrcmp(*Provider->Filename, *DemoFilename) == 0)
#else
					if (Provider != NULL && appStricmp(*Provider->Filename, *DemoFilename) == 0)
#endif
					{
						bAlreadyInList = TRUE;
						break;
					}
				}

				if (!bAlreadyInList)
				{
					UUTUIDataProvider_DemoFile* Provider = ConstructObject<UUTUIDataProvider_DemoFile>(UUTUIDataProvider_DemoFile::StaticClass(), MenuItemStore);
					if (Provider != NULL)
					{
						Provider->Filename = DemoFilename;
						MenuItemStore->ListElementProviders.Add(TEXT("DemoFiles"), Provider);
					}
				}
			}
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
 * Fix up the world in code however necessary
 *
 * @param World UWorld object for the world to fix
 */
void FUTPatchHelper::FixupWorld(UWorld* World)
{
	FName PackageName = World->GetOutermost()->GetFName();
	ULevel* PersistentLevel = World->PersistentLevel;

	if (PackageName == FName(TEXT("VCTF-Suspense")))
	{
		debugf(TEXT("Fixing up %s..."), *PackageName.ToString());
		// set bBlockedForVehicles on some additional navigation points
		TCHAR* BlockForVehiclesNames[] = { TEXT("UTAmmo_LinkGun_34"), TEXT("PathNode_243"), TEXT("UTAmmo_FlakCannon_1"),
										TEXT("UTAmmo_LinkGun_35"), TEXT("UTAmmo_FlakCannon_8"), TEXT("PathNode_241") };
		for (INT i = 0; i < ARRAY_COUNT(BlockForVehiclesNames); i++)
		{
			ANavigationPoint* Nav = FindObject<ANavigationPoint>(PersistentLevel, BlockForVehiclesNames[i], FALSE);
			if (Nav != NULL)
			{
				Nav->bBlockedForVehicles = TRUE;
			}
			else
			{
				debugf(NAME_Warning, TEXT("Failed to find %s for fixup"), BlockForVehiclesNames[i]);
			}
		}
	}
	else if (PackageName == FName(TEXT("DM-Deimos")))
	{
		debugf(TEXT("Fixing up %s..."), *PackageName.ToString());
		// set bStopOnEncroach to FALSE for a mover whose Kismet doesn't actually stop/return when it hits stuff
		AInterpActor* BrokenMover = FindObject<AInterpActor>(PersistentLevel, TEXT("InterpActor_2"), FALSE);
		if (BrokenMover != NULL)
		{
			BrokenMover->bStopOnEncroach = FALSE;
		}
		else
		{
			debugf(NAME_Warning, TEXT("Failed to find InterpActor_2 for fixup"));
		}
	}
	else if (PackageName == FName(TEXT("DM-Gateway")))
	{
		debugf(TEXT("Fixing up %s..."), *PackageName.ToString());
		// fix the TranslucentSortPriority on some meshes that have it set too high
		TCHAR* BrokenMeshNames[] = { TEXT("StaticMeshActor_1563"), TEXT("StaticMeshActor_1673"), TEXT("StaticMeshActor_1345") };
		for (INT i = 0; i < ARRAY_COUNT(BrokenMeshNames); i++)
		{
			AStaticMeshActor* BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, BrokenMeshNames[i], FALSE);
			if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
			{
				FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
				BrokenMesh->StaticMeshComponent->TranslucencySortPriority = -1 - i;
			}
			else
			{
				debugf(NAME_Warning, TEXT("Failed to find %s for fixup"), BrokenMeshNames[i]);
			}
		}
	}
	else if (PackageName == FName(TEXT("CTF-OmicronDawn")))
	{
		debugf(TEXT("Fixing up %s..."), *PackageName.ToString());
		// add some extra blocking volumes near a hole where players can fall through the world
		FVector BlockerLocations[] = { FVector(-8505.0f, 114.0f, 233.0f), FVector(-10949.0f, -5756.0, 233.0f) };
		for (INT i = 0; i < ARRAY_COUNT(BlockerLocations); i++)
		{
			AKeypoint* Blocker = Cast<AKeypoint>(GWorld->SpawnActor(FindObject<UClass>(NULL, TEXT("Engine.TargetPoint")), NAME_None, BlockerLocations[i], FRotator(0,0,0), NULL, TRUE));
			if (Blocker != NULL)
			{
				UCylinderComponent* NewCylinder = ConstructObject<UCylinderComponent>(UCylinderComponent::StaticClass(), Blocker);
				NewCylinder->CollisionRadius = 150.0f;
				NewCylinder->CollisionHeight = 300.0f;
				NewCylinder->BlockZeroExtent = FALSE;
				NewCylinder->BlockNonZeroExtent = TRUE;
				NewCylinder->CollideActors = TRUE;
				NewCylinder->BlockActors = TRUE;
				Blocker->AttachComponent(NewCylinder);
				Blocker->bProjTarget = FALSE;
				Blocker->bWorldGeometry = TRUE;
				Blocker->SetCollision(TRUE, TRUE, TRUE);
			}
		}
	}
	else if (PackageName == FName(TEXT("WAR-Islander")) || PackageName == FName(TEXT("WAR-Islander_Necris")))
	{
		debugf(TEXT("Fixing up %s..."), *PackageName.ToString());
		// fix hole where players can get stuck
		AKeypoint* Blocker = Cast<AKeypoint>(GWorld->SpawnActor(FindObject<UClass>(NULL, TEXT("Engine.TargetPoint")), NAME_None, FVector(-5484.0f, -8546.0f, 527.4f), FRotator(0,0,0), NULL, TRUE));
		if (Blocker != NULL)
		{
			UCylinderComponent* NewCylinder = ConstructObject<UCylinderComponent>(UCylinderComponent::StaticClass(), Blocker);
			NewCylinder->CollisionRadius = 100.0f;
			NewCylinder->CollisionHeight = 50.0f;
			NewCylinder->BlockZeroExtent = FALSE;
			NewCylinder->BlockNonZeroExtent = TRUE;
			NewCylinder->CollideActors = TRUE;
			NewCylinder->BlockActors = TRUE;
			Blocker->AttachComponent(NewCylinder);
			Blocker->bProjTarget = FALSE;
			Blocker->bWorldGeometry = TRUE;
			Blocker->bMovable = FALSE;
			Blocker->SetCollision(TRUE, TRUE, TRUE);
		}
	}
	else if (PackageName == FName(TEXT("WAR-Torlan_Leviathan")))
	{
		debugf(TEXT("Fixing up %s..."), *PackageName.ToString());
		// set bPreferredVehiclePath on some additional navigation points to help Leviathan navigation
		TCHAR* BlockForVehiclesNames[] = { TEXT("PathNode_646"), TEXT("PathNode_649"), TEXT("PathNode_30"),
										TEXT("PathNode_31"), TEXT("PathNode_32"), TEXT("PathNode_184"),
										TEXT("PathNode_185"), TEXT("PathNode_210"), TEXT("PathNode_149"),
										TEXT("PathNode_33"), TEXT("PathNode_34") };
		for (INT i = 0; i < ARRAY_COUNT(BlockForVehiclesNames); i++)
		{
			ANavigationPoint* Nav = FindObject<ANavigationPoint>(PersistentLevel, BlockForVehiclesNames[i], FALSE);
			if (Nav != NULL)
			{
				Nav->bPreferredVehiclePath = TRUE;
			}
			else
			{
				debugf(NAME_Warning, TEXT("Failed to find %s for fixup"), BlockForVehiclesNames[i]);
			}
		}
	}
	else if (PackageName == FName(TEXT("DM-Deck")))
	{
		AStaticMeshActor* BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_673"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
	}
	else if (PackageName == FName(TEXT("CTF-Coret")))
	{
		AStaticMeshActor* BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1943"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1417"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1828"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1829"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1831"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1830"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1832"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1833"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1835"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
		BrokenMesh = FindObject<AStaticMeshActor>(PersistentLevel, TEXT("StaticMeshActor_1834"), FALSE);
		if (BrokenMesh != NULL && BrokenMesh->StaticMeshComponent != NULL)
		{
			FComponentReattachContext Context(BrokenMesh->StaticMeshComponent);
			BrokenMesh->StaticMeshComponent->BlockNonZeroExtent = TRUE;
		}
	}
}


