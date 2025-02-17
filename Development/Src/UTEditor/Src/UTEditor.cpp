//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

#include "UnrealEd.h"
#include "Properties.h"
#include "EngineMaterialClasses.h"
#include "UTEditor.h"
#include "UTEditorFactories.h"
#include "UTGameUIClasses.h"
#include "SourceControlIntegration.h"
#include "FConfigCacheIni.h"
#include "BusyCursor.h"
#include "ScopedTransaction.h"

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName UTEDITOR_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "UTEditorClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "UTEditorClasses.h"
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
void AutoInitializeRegistrantsUTEditor( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_UTEDITOR;
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesUTEditor()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) UTEDITOR_##name = FName(TEXT(#name));
	#include "UTEditorClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

IMPLEMENT_CLASS(UUTMapMusicInfoFactoryNew);
IMPLEMENT_CLASS(UGenericBrowserType_UTMapMusicInfo);
IMPLEMENT_CLASS(UUTEditorEngine);
IMPLEMENT_CLASS(UUTUnrealEdEngine);

/*------------------------------------------------------------------------------
     UUTMapMusicInfoFactoryNew.
------------------------------------------------------------------------------*/

void UUTMapMusicInfoFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UUTMapMusicInfoFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UUTMapMusicInfo::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("UT Map Music");
}

UObject* UUTMapMusicInfoFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

void UGenericBrowserType_UTMapMusicInfo::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UUTMapMusicInfo::StaticClass(), FColor(200,128,128), NULL, 0, this));
}

/** checks if the given file is read only. If so, attempts to check it out to make it not read only.
 * @return whether the file is still read only
 */
static UBOOL CheckReadOnly(const FString& Filename, UBOOL bSkipCheckout, UBOOL bAutoCheckout)
{
	if (GFileManager->IsReadOnly(*Filename))
	{
#if HAVE_SCC
		if (!bSkipCheckout && (bAutoCheckout || GWarn->YesNof(LocalizeSecure(LocalizeQuery(TEXT("CheckoutPerforce"),TEXT("Core")), *Filename))))
		{
			// Attempt to check out the file
			FSourceControlIntegration* SCC = new FSourceControlIntegration;
			SCC->CheckOut(*(GFileManager->GetCurrentDirectory() + Filename));
			delete SCC;
		}
#endif // HAVE_SCC
		if (GFileManager->IsReadOnly(*Filename))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** gets the full path of the .ini file we should use for autoexporting the specified class's menu data */
static FString GetIniPath(UUTUIResourceDataProvider* Provider, UClass* TheClass)
{
	FString IniPath = appGameConfigDir();
	if (Provider->IniName != TEXT(""))
	{
		IniPath += Provider->IniName + TEXT(".ini");
	}
	else if (TheClass->GetOutermost()->GetName().StartsWith(TEXT("UT")))
	{
		IniPath += FString::Printf(TEXT("%s.ini"), *TheClass->GetOutermost()->GetName());
	}
	else
	{
		IniPath += FString::Printf(TEXT("UT%s.ini"), *TheClass->GetOutermost()->GetName());
	}
	return IniPath;
}

void UUTEditorEngine::PostScriptCompile()
{
	if (ParseParam(appCmdLine(), TEXT("NOINIEXPORT")))
	{
		return;
	}

	// load up all of the providers in .ini files, so that if a mutator has an entry in a different .ini than we expect, we can still find it
	UUTUIDataStore_MenuItems* DefaultDataStore = UUTUIDataStore_MenuItems::StaticClass()->GetDefaultObject<UUTUIDataStore_MenuItems>();
	TArray<UUTUIResourceDataProvider*> Providers;
	DefaultDataStore->GetAllResourceDataProviders(UUTUIDataProvider_Mutator::StaticClass(), Providers);
	Providers.Reset();
	DefaultDataStore->GetAllResourceDataProviders(UUTUIDataProvider_GameModeInfo::StaticClass(), Providers);
	Providers.Reset();
	DefaultDataStore->GetAllResourceDataProviders(UUTUIDataProvider_Weapon::StaticClass(), Providers);

	// find all concrete, exportable Mutator, GameInfo, and Weapon classes
	// also get all Ammo classes, used when exporting weapons
	TArray<UClass*> MutatorClasses;
	TArray<UClass*> GameInfoClasses;
	TArray<UClass*> WeaponClasses;
	TArray<UClass*> AmmoClasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (!(It->ClassFlags & CLASS_Abstract))
		{
			if (It->IsChildOf(AUTMutator::StaticClass()))
			{
				if (It->GetDefaultObject<AUTMutator>()->bExportMenuData)
				{
					MutatorClasses.AddItem(*It);
				}
			}
			else if (It->IsChildOf(AUTGame::StaticClass()))
			{
				if (It->GetDefaultObject<AUTGame>()->bExportMenuData)
				{
					GameInfoClasses.AddItem(*It);
				}
			}
			else if (It->IsChildOf(AUTWeapon::StaticClass()))
			{
				if (It->GetDefaultObject<AUTWeapon>()->bExportMenuData)
				{
					WeaponClasses.AddItem(*It);
				}
			}
			else if (It->IsChildOf(AUTAmmoPickupFactory::StaticClass()))
			{
				AmmoClasses.AddItem(*It);
			}
		}
	}

	const UBOOL bSkipCheckout = (ParseParam(appCmdLine(), TEXT("SILENTBUILD")) || ParseParam(appCmdLine(), TEXT("UNATTENDED")));
	const UBOOL bAutoCheckout = ParseParam(appCmdLine(), TEXT("auto"));

	// for each class found, find its resource provider entry in the .ini and update it if necessary (creating it if one does not yet exist)

	// Mutators
	for (INT i = 0; i < MutatorClasses.Num(); i++)
	{
		AMutator* DefaultMut = MutatorClasses(i)->GetDefaultObject<AMutator>();
		FString MutatorGroupString;
		for (INT j = 0; j < DefaultMut->GroupNames.Num(); j++)
		{
			if (j > 0)
			{
				MutatorGroupString += TEXT("|");
			}
			MutatorGroupString += DefaultMut->GroupNames(j);
		}

		UUTUIDataProvider_Mutator* Provider = FindObject<UUTUIDataProvider_Mutator>(GetTransientPackage(), *MutatorClasses(i)->GetName(), TRUE);
		UBOOL bNeedToSave;
		if (Provider != NULL)
		{
			bNeedToSave = (Provider->GroupNames != MutatorGroupString);
		}
		else
		{
			Provider = ConstructObject<UUTUIDataProvider_Mutator>(UUTUIDataProvider_Mutator::StaticClass(), GetTransientPackage(), MutatorClasses(i)->GetFName());
			bNeedToSave = TRUE;
		}

		if (bNeedToSave)
		{
			FString IniPath = GetIniPath(Provider, MutatorClasses(i));
			bNeedToSave = !CheckReadOnly(IniPath, bSkipCheckout, bAutoCheckout);
			if (bNeedToSave)
			{
				warnf(TEXT("Updating %s's menu information in %s"), *MutatorClasses(i)->GetName(), *FFilename(IniPath).GetCleanFilename());
				Provider->ClassName = MutatorClasses(i)->GetPathName();
				Provider->GroupNames = MutatorGroupString;
				if (Provider->FriendlyName == TEXT(""))
				{
					Provider->FriendlyName = MutatorClasses(i)->GetName();
				}
				Provider->SaveConfig(0, *IniPath);
			}
			else
			{
				warnf(TEXT("Unable to update %s's menu information in %s because the file is read-only"), *MutatorClasses(i)->GetName(), *FFilename(IniPath).GetCleanFilename());
			}
		}
	}
	// GameInfos
	for (INT i = 0; i < GameInfoClasses.Num(); i++)
	{
		AUTGame* DefaultGame = GameInfoClasses(i)->GetDefaultObject<AUTGame>();
		FString MapPrefixesString;
		for (INT j = 0; j < DefaultGame->MapPrefixes.Num(); j++)
		{
			if (j > 0)
			{
				MapPrefixesString += TEXT("|");
			}
			MapPrefixesString += DefaultGame->MapPrefixes(j);
		}

		UUTUIDataProvider_GameModeInfo* Provider = FindObject<UUTUIDataProvider_GameModeInfo>(GetTransientPackage(), *GameInfoClasses(i)->GetName(), TRUE);
		UBOOL bNeedToSave;
		if (Provider != NULL)
		{
			bNeedToSave = (MapPrefixesString != Provider->Prefixes);
		}
		else
		{
			Provider = ConstructObject<UUTUIDataProvider_GameModeInfo>(UUTUIDataProvider_GameModeInfo::StaticClass(), GetTransientPackage(), GameInfoClasses(i)->GetFName());
			bNeedToSave = TRUE;
		}

		if (bNeedToSave)
		{
			FString IniPath = GetIniPath(Provider, GameInfoClasses(i));
			bNeedToSave = !CheckReadOnly(IniPath, bSkipCheckout, bAutoCheckout);
			if (bNeedToSave)
			{
				warnf(TEXT("Updating %s's menu information in %s"), *GameInfoClasses(i)->GetName(), *FFilename(IniPath).GetCleanFilename());
				Provider->GameMode = GameInfoClasses(i)->GetPathName();
				Provider->Prefixes = MapPrefixesString;
				
				if (Provider->FriendlyName == TEXT(""))
				{
					Provider->FriendlyName = GameInfoClasses(i)->GetName();
				}
				if (Provider->GameSettingsClass == TEXT(""))
				{
					Provider->GameSettingsClass = TEXT("UTGameSettingsDM");
				}
				if (Provider->OptionSet == TEXT(""))
				{
					Provider->OptionSet = TEXT("DM");
				}
				Provider->SaveConfig(0, *IniPath);
			}
			else
			{
				warnf(TEXT("Unable to update %s's menu information in %s because the file is read-only"), *GameInfoClasses(i)->GetName(), *FFilename(IniPath).GetCleanFilename());
			}
		}
	}
	// Weapons
	for (INT i = 0; i < WeaponClasses.Num(); i++)
	{
		// get the ammo class used by this weapon (if any)
		UClass* MyAmmoClass = NULL;
		for (INT j = 0; j < AmmoClasses.Num(); j++)
		{
			if (AmmoClasses(j)->GetDefaultObject<AUTAmmoPickupFactory>()->TargetWeapon == WeaponClasses(i))
			{
				MyAmmoClass = AmmoClasses(j);
				break;
			}
		}
		UUTUIDataProvider_Weapon* Provider = FindObject<UUTUIDataProvider_Weapon>(GetTransientPackage(), *WeaponClasses(i)->GetName(), TRUE);
		UBOOL bNeedToSave;
		if (Provider != NULL)
		{
			// multiple ammo types could point to the same weapon, so only re-export if the current value is completely invalid
			bNeedToSave = (Provider->AmmoClassPath != TEXT("")) ? (FindObject<UClass>(NULL, *Provider->AmmoClassPath, TRUE) == NULL) : (MyAmmoClass != NULL);
		}
		else
		{
			Provider = ConstructObject<UUTUIDataProvider_Weapon>(UUTUIDataProvider_Weapon::StaticClass(), GetTransientPackage(), WeaponClasses(i)->GetFName());
			bNeedToSave = TRUE;
		}

		if (bNeedToSave)
		{
			FString IniPath = GetIniPath(Provider, WeaponClasses(i));
			bNeedToSave = !CheckReadOnly(IniPath, bSkipCheckout, bAutoCheckout);
			if (bNeedToSave)
			{
				warnf(TEXT("Updating %s's menu information in %s"), *WeaponClasses(i)->GetName(), *FFilename(IniPath).GetCleanFilename());
				Provider->ClassName = WeaponClasses(i)->GetPathName();
				
				if (Provider->FriendlyName == TEXT(""))
				{
					Provider->FriendlyName = WeaponClasses(i)->GetName();
				}
				Provider->AmmoClassPath = (MyAmmoClass != NULL) ? MyAmmoClass->GetPathName() : TEXT("");
				Provider->SaveConfig(0, *IniPath);
			}
			else
			{
				warnf(TEXT("Unable to update %s's menu information in %s because the file is read-only"), *WeaponClasses(i)->GetName(), *FFilename(IniPath).GetCleanFilename());
			}
		}
	}
}

static FString ConvertUnicodeFStringToAnsi(FString UniCodeString)
{
	if ( UniCodeString == TEXT("") || appIsPureAnsi(*UniCodeString) )
	{
		return UniCodeString;
	}

	for (INT i=0;i<UniCodeString.Len();i++)
	{
		INT Ord = INT( UniCodeString[i] );
		if ( Ord > 255 )
		{
			if ( Ord == 8217 )
			{
				UniCodeString[i] = '\'';
			}
			else if (Ord == 8220 || Ord == 8221)
			{
				UniCodeString[i] = '\"';
			}
			else
			{
				UniCodeString[i] = '?';
			}
		}
	}

	FString Result( TCHAR_TO_ANSI(*UniCodeString) );
	if ( !appIsPureAnsi(*Result) )
	{
		debugf(TEXT("Converting [%s] Failed"),*UniCodeString);
	}
	else
	{
		debugf(TEXT("Converted [%s]"),*Result);
	}

	return Result;
}

/** version of the string entry dialog that takes pre-localized strings */
class WxUTDlgGenericStringEntry : public WxDlgGenericStringEntry
{
public:
	int ShowModal(const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString)
	{
		SetTitle(DialogTitle);
		GetStringCaption().SetLabel(Caption);

		GetStringEntry().SetValue(DefaultString);

		return wxDialog::ShowModal();
	}
};

/** UT-specific editor console commands. */
UBOOL UUTUnrealEdEngine::Exec( const TCHAR* Str, FOutputDevice& Ar )
{
	if( ParseCommand(&Str,TEXT("ADDPICKUPLIGHTS")) )
	{
		const FScopedBusyCursor BusyCursor;
		const FScopedTransaction Transaction( TEXT("Add Pickup Lights") );

		UBOOL bLevelWasModified = FALSE;

		// Delete all existing lights first, while at the same time making a list of pickup factories.
		TArray<AUTPickupFactory*> PickupFactories;
		for( FActorIterator It ; It ; ++It )
		{
			if( It->IsA( AUTPickupLight::StaticClass() ) )
			{
				GWorld->EditorDestroyActor( *It, TRUE );
				bLevelWasModified = TRUE;
			}
			else if ( It->IsA( AUTPickupFactory::StaticClass() ) )
			{
				PickupFactories.AddItem( (AUTPickupFactory*)*It );
			}
		}

		// Loop over pickups, creating lightings for those that need them.
		for( INT FactoryIndex = 0 ; FactoryIndex < PickupFactories.Num() ; ++FactoryIndex )
		{
			AUTPickupFactory* PickupFactory = PickupFactories(FactoryIndex);

			const UBOOL bIsArmorPickup			= PickupFactory->IsA( AUTArmorPickupFactory::StaticClass() );
			const UBOOL bIsHealthPickup			= PickupFactory->IsA( AUTHealthPickupFactory::StaticClass() ) && !PickupFactory->IsA( AUTPickupFactory_HealthVial::StaticClass() );
			const UBOOL bIsWeaponPickup			= PickupFactory->IsA( AUTWeaponPickupFactory::StaticClass() );
			const UBOOL bIsWeaponLockerPickup	= PickupFactory->IsA( AUTWeaponLocker::StaticClass() );

			FVector NewLocation( PickupFactory->Location );
			UClass* LightClass = NULL;
			if( bIsArmorPickup )
			{
				NewLocation += FVector(0,0,-12);
				LightClass = AUTArmorPickupLight::StaticClass();
			}
			else if( bIsHealthPickup )
			{
				NewLocation += FVector(0,0,-12);
				LightClass = AUTHealthPickupLight::StaticClass();
			}
			else if( bIsWeaponPickup )
			{
				NewLocation += FVector(0,0,-12);
				LightClass = AUTWeaponPickupLight::StaticClass();
			}
			else if( bIsWeaponLockerPickup )
			{
				LightClass = AUTWeaponLockerPickupLight::StaticClass();
			}

			if ( LightClass )
			{
				AActor* NewLight = GWorld->SpawnActor( LightClass, NAME_None, NewLocation );
				NewLight->InvalidateLightingCache();
				NewLight->PostEditMove( TRUE );
				NewLight->MarkPackageDirty();
				bLevelWasModified = TRUE;
			}
		}

		if ( bLevelWasModified )
		{
			RedrawLevelEditingViewports();
			GCallbackEvent->Send( CALLBACK_LevelDirtied );
			GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
		}
		return TRUE;
	}

	const UBOOL bProcessed = Super::Exec(Str, Ar);
	return bProcessed;
}

/** Util for finding and replacing a substring within another string. */
static FString ReplaceSubstring(const FString& InString, const FString& Substring, const FString& ReplaceWith)
{
	INT SubstringPos = InString.InStr(Substring);
	if(SubstringPos == INDEX_NONE)
	{
		return TEXT("");
	}

	FString PreSubstring = InString.Left(SubstringPos);
	FString PostSubstring = InString.Mid(SubstringPos+4);
	FString WithSubstring = PreSubstring + ReplaceWith + PostSubstring;

	return WithSubstring;
}



