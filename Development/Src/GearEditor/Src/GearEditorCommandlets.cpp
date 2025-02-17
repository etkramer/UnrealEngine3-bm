/*=============================================================================
	GearEditorCommandlets.cpp: Gear editor comandlet definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "GearEditor.h"
#include "GearEditorClasses.h"
#include "GearEditorCommandlets.h"

#include "PackageHelperFunctions.h"

/** Implements FString sorting for GearEditorCommandlets.cpp */
IMPLEMENT_COMPARE_CONSTREF( FString, GearEditorCommandlets, { return appStricmp(*A,*B); } );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UAssembleMPSoundListCommandlet
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(UAssembleMPSoundListCommandlet);

/**
 * This will add all of the cues from this game type into the ReferencedCue array
 * @todo here is where we can link into the cooker stuff and make the game based packages
 **/
void AddCuesFromGameType( const FString& GearGameClassName, TArray<USoundCue*>& ReferencedCues )
{
	warnf( NAME_Log, TEXT("Loading game class: %s"), *GearGameClassName );

	UClass* GearGameMPClass = LoadClass<AGearGame>( NULL, *GearGameClassName, NULL, LOAD_None, NULL );
	if ( GearGameMPClass )
	{
		AGearGame* GearGameMPDefaultObject = Cast<AGearGame>( GearGameMPClass->GetDefaultObject( TRUE ) );
		if ( GearGameMPDefaultObject )
		{
			// Grab out all of the encouragement sounds
			for ( INT EncouragementIndex = 0 ; EncouragementIndex < ET_MAX; ++EncouragementIndex )
			{
				const FTeamEncouragementData& EncouragementData = GearGameMPDefaultObject->EncouragementData[EncouragementIndex];
				for ( INT CueIndex = 0 ; CueIndex < EncouragementData.COGSounds.Num() ; ++CueIndex )
				{
					USoundCue* Cue = EncouragementData.COGSounds(CueIndex);
					if ( Cue )
					{
						ReferencedCues.AddUniqueItem( Cue );
					}
				}
				for ( INT CueIndex = 0 ; CueIndex < EncouragementData.LocustSounds.Num() ; ++CueIndex )
				{
					USoundCue* Cue = EncouragementData.LocustSounds(CueIndex);
					if ( Cue )
					{
						ReferencedCues.AddUniqueItem( Cue );
					}
				}
			}

			// we also need to look at all of classes to see if they have special cues (e.g. MeatFlag does)

		}
		else
		{
			warnf( NAME_Log, TEXT("Couldn't create default object for class %s"), *GearGameMPClass->GetName() );
		}
	}
	else
	{
		warnf( NAME_Log, TEXT("Couldn't load class: %s"), *GearGameClassName );
	}
}


INT UAssembleMPSoundListCommandlet::Main(const FString& Params)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Look to the .ini for the list of pawn classes used in MP.
	TMultiMap<FString,FString>* IniPawnClassList = GConfig->GetSectionPrivate( TEXT("Engine.MPPawnClasses"), FALSE, TRUE, GEngineIni );
	TArray<FString> PawnClassList;
	if ( IniPawnClassList )
	{
		for( TMultiMap<FString,FString>::TConstIterator It(*IniPawnClassList) ; It ; ++It )
		{
			const FString& ClassName = It.Value();
			PawnClassList.AddUniqueItem( ClassName );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load the requested classes.
	TArray<UClass*> PawnClasses;
	for ( INT ClassIndex = 0 ; ClassIndex < PawnClassList.Num() ; ++ClassIndex )
	{
		const FString& ClassName = PawnClassList(ClassIndex);
		UClass* LoadedClass = LoadClass<AGearPawn>( NULL, *ClassName, NULL, LOAD_None, NULL );
		if ( !LoadedClass )
		{
			warnf( NAME_Log, TEXT("Couldn't load class: %s"), *ClassName );
		}
		else
		{
			warnf( NAME_Log, TEXT("Loaded class: %s"), *ClassName );
			PawnClasses.AddUniqueItem( LoadedClass );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// For each class, assemble the list of referenced cues.
	TArray<USoundCue*> ReferencedCues;
	for ( INT ClassIndex = 0 ; ClassIndex < PawnClasses.Num() ; ++ClassIndex )
	{
		UClass* PawnClass = PawnClasses(ClassIndex);
		AGearPawn* ClassDefaultObject = Cast<AGearPawn>( PawnClass->GetDefaultObject( TRUE ) );
		if ( ClassDefaultObject )
		{
			// @todo hook up to look at the SoundGroup which are: GearSoundGroup'GearSoundGroupArchetypes

			////////////////////////////////////////
			// Sound group
			UGearSoundGroup const* const SoundGroup = ClassDefaultObject->SoundGroup;
			if ( SoundGroup )
			{
				for ( INT Index = 0 ; Index < SoundGroup->VoiceEfforts.Num() ; ++Index )
				{
					const FGearVoiceEffortEntry& EffortEntry = SoundGroup->VoiceEfforts(Index);

					for (INT CueIdx=0; CueIdx<EffortEntry.Sounds.Num(); CueIdx++)
					{
						USoundCue* const Cue = EffortEntry.Sounds(CueIdx);
						if ( Cue )
						{
							ReferencedCues.AddUniqueItem( Cue );
						}
					}
				}

				for ( INT Index = 0 ; Index < SoundGroup->FoleySoundFX.Num() ; ++Index )
				{
					const FGearFoleyEntry& FoleyEntry = SoundGroup->FoleySoundFX(Index);

					for (INT CueIdx=0; CueIdx<FoleyEntry.Sounds.Num(); CueIdx++)
					{
						USoundCue* const Cue = FoleyEntry.Sounds(CueIdx);
						if ( Cue )
						{
							ReferencedCues.AddUniqueItem( Cue );
						}
					}
				}
			}
			else
			{
				warnf( NAME_Log, TEXT("SoundGroup is NULL for class %s"), *PawnClass->GetName() );
			}

			////////////////////////////////////////
			// GUDS
/***
			UClass* MasterGUDBankClass = ClassDefaultObject->MasterGUDBankClass;
			if ( MasterGUDBankClass )
			{
				UGUDBank* MasterGUDBankClassDefaultObject = Cast<UGUDBank>( MasterGUDBankClass->GetDefaultObject( TRUE ) );
				if ( MasterGUDBankClassDefaultObject )
				{
					for ( INT Index = 0 ; Index < MasterGUDBankClassDefaultObject->GUDLines.Num() ; ++Index )
					{
						const FGUDLine& GUDLine = MasterGUDBankClassDefaultObject->GUDLines(Index);
						USoundCue* Cue = GUDLine.Audio;
						if ( Cue )
						{
							ReferencedCues.AddUniqueItem( Cue );
						}
					}
				}
				else
				{
					warnf( NAME_Log, TEXT("Couldn't create default object for class %s"), *MasterGUDBankClassDefaultObject->GetName() );
				}
			}
			else
			{
				warnf( NAME_Log, TEXT("MasterGUDBankClass is NULL for class %s"), *PawnClass->GetName() );
			}
***/
			////////////////////////////////////////
			// FaceFX of the pawn's skeletal mesh.
			USkeletalMeshComponent* Mesh = ClassDefaultObject->Mesh;
			if ( Mesh && Mesh->SkeletalMesh )
			{
				UFaceFXAsset* FaceFXAsset = Mesh->SkeletalMesh->FaceFXAsset;
				if ( FaceFXAsset )
				{
					for ( INT Index = 0 ; Index < FaceFXAsset->ReferencedSoundCues.Num() ; ++Index )
					{
						USoundCue* Cue = FaceFXAsset->ReferencedSoundCues(Index);
						if ( Cue )
						{
							ReferencedCues.AddUniqueItem( Cue );
						}
					}
				}
				else
				{
					warnf( NAME_Log, TEXT("Skeletal mesh's FaceFX is NULL for class %s"), *PawnClass->GetName() );
				}
			}
			else
			{
				warnf( NAME_Log, TEXT("Skeletal mesh is NULL for class %s"), *PawnClass->GetName() );
			}

			////////////////////////////////////////  keep this here as an example of what we need
			// do for looking at faceFX stuff.   If we need to reuse the FaceFX external animsets ever
			// Efforts FaceFX.
			for (INT FASIdx=0; FASIdx<ClassDefaultObject->FAS_Efforts.Num(); ++FASIdx)
			{
				UFaceFXAnimSet* FaceFXAnimSet = ClassDefaultObject->FAS_Efforts(FASIdx);
				if ( FaceFXAnimSet )
				{
					for ( INT Index = 0 ; Index < FaceFXAnimSet->ReferencedSoundCues.Num() ; ++Index )
					{
						USoundCue* Cue = FaceFXAnimSet->ReferencedSoundCues(Index);
						if ( Cue )
						{
							ReferencedCues.AddUniqueItem( Cue );
						}
					}
				}
				else
				{
					warnf( NAME_Log, TEXT("FaceFXAnimSet FAS_Efforts has NULL entry for class %s"), *PawnClass->GetName() );
				}
			}
		}
		else
		{
			warnf( NAME_Log, TEXT("Couldn't create default object for class %s"), *PawnClass->GetName() );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Assemble cues referenced by GearGameMP.
	const FString GearGameClassName( TEXT("GearGameContent.GearGameTDM") );
	AddCuesFromGameType( GearGameClassName, ReferencedCues );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Assemble the list of referenced packages.
	TArray<UPackage*> ReferencedPackages;
	for ( INT CueIndex = 0 ; CueIndex < ReferencedCues.Num() ; ++CueIndex )
	{
		USoundCue* Cue = ReferencedCues(CueIndex);
		UPackage* CuePackage = Cue->GetOutermost();
		ReferencedPackages.AddUniqueItem( CuePackage );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Empty out any existing ini data.
	const TCHAR* CueSectionName = TEXT("Cooker.MPSoundCues");
	GConfig->EmptySection( CueSectionName, GEditorIni );

	const TCHAR* PackageSectionName = TEXT("Cooker.MPSoundCuePackages");
	GConfig->EmptySection( PackageSectionName, GEditorIni );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Write references to ini.
	for ( INT CueIndex = 0 ; CueIndex < ReferencedCues.Num() ; ++CueIndex )
	{
		USoundCue* Cue = ReferencedCues(CueIndex);
		const FString CuePath = Cue->GetPathName();
		//warnf( NAME_Log, TEXT("%s"), *CuePath );
		GConfig->SetString( CueSectionName, *FString::Printf(TEXT("Cue%i"),CueIndex), *CuePath, GEditorIni );
	}

	for ( INT PackageIndex = 0 ; PackageIndex < ReferencedPackages.Num() ; ++PackageIndex )
	{
		UPackage* Package = ReferencedPackages(PackageIndex);
		const FString PackageName = Package->GetName();
		GConfig->SetString( PackageSectionName, *FString::Printf(TEXT("Package%i"),PackageIndex), *PackageName, GEditorIni );
	}

	warnf( NAME_Log, TEXT("Wrote %i cues to %s in %s"), ReferencedCues.Num(), CueSectionName, GEditorIni );
	warnf( NAME_Log, TEXT("Wrote %i packages to %s in %s"), ReferencedPackages.Num(), PackageSectionName, GEditorIni );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UAnalyzeLocalizedSoundCommandlet
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(UAnalyzeLocalizedSoundCommandlet);

/**
 * Assembles a localized package name from the specified base package name and language extension.
 *
 * @return		A string of the form BasePackageName_LanguageExt, or BasePackageName if LanguageExt is "int".
 */
static FString MakeLocalizedPackageName(const TCHAR* BasePackageName, const TCHAR* LanguageExt)
{
	check( BasePackageName );
	check( LanguageExt );

	// Only append the language if one was supplied and it's not int.
	const FString Result = appStricmp( LanguageExt, TEXT("INT") ) != 0 ?
		FString::Printf(TEXT("%s_%s"), BasePackageName, LanguageExt ) :
	FString( BasePackageName );

	return Result;
}

class FSoundSrcData
{
public:
	FLOAT Duration;
	FLOAT CookedDataSizeK;
};

/**
 * Closes the specified named file archive.  Generates a warning if the archive wasn't open.
 */
static void CloseCSVFile(FArchive*& CSVFile, const TCHAR* CSVFilename)
{
	check( CSVFilename );
	if ( CSVFile )
	{
		CSVFile->Close();
		delete CSVFile;
		CSVFile = NULL;
	}
	else
	{
		warnf( NAME_Warning, TEXT("Could not create CSV file %s for writing."), CSVFilename );
	}
}

typedef TMap<const FString,FSoundSrcData> SoundDurationList;
typedef SoundDurationList::TConstIterator SoundDurationListConstItor;

/**
 * Compares the 'tgt' package against the 'ref' package to determine the set of sounds that are
 * longer, missing, or superfluous.
 *
 * @param	Ref					The reference package (package to compare against).
 * @param	Tgt					The target package (package to compare).
 * @param	OutLongerSounds		[out] Sounds which are in both packages, and which are longer in the tgt package.
 * @param	OutMissingSounds	[out] Sounds which are in the ref package but not in the tgt package.
 * @param	OutExtraSounds		[out] Sounds which are in the tgt package but not in the ref package.
 */
static void CompareSounds(const SoundDurationList& Ref,
						  const SoundDurationList& Tgt,
						  TArray<FString>& OutLongerSounds,
						  TArray<FString>& OutMissingSounds,
						  TArray<FString>& OutExtraSounds)
{
	// Zero inputs.
	OutLongerSounds.Empty();
	OutMissingSounds.Empty();
	OutExtraSounds.Empty();

	// Look for sounds which:
	//    1) missing from the tgt package;
	//    2) have a longer duration in the tgt package.
	for ( SoundDurationListConstItor It( Ref ) ; It ; ++It )
	{
		const FString& RefSound = It.Key();
		const FSoundSrcData* TgtData= Tgt.Find( RefSound );
		if ( TgtData )
		{
			// The sound exists in the tgt package.  See if it has a duration that is 1/30th sec longer than the ref sound.
			const FSoundSrcData& RefData = It.Value();
			if ( TgtData->Duration > RefData.Duration + 0.25f )
			{
				OutLongerSounds.AddItem( RefSound );
			}
		}
		else
		{
			// The sound does not exist in the tgt package.
			OutMissingSounds.AddItem( RefSound );
		}
	}

	// Look for sounds in the tgt package that aren't present in the ref package.
	for ( SoundDurationListConstItor It( Tgt ) ; It ; ++It )
	{
		const FString& TgtSound = It.Key();
		if ( !Ref.Find( TgtSound ) )
		{
			// The sound exists in the tgt package but not in the ref package.
			OutExtraSounds.AddItem( TgtSound );
		}
	}

	// Alphabetically sort missing sound list.
	if ( OutMissingSounds.Num() > 0 )
	{
		Sort<USE_COMPARE_CONSTREF(FString, GearEditorCommandlets)>( OutMissingSounds.GetTypedData(), OutMissingSounds.Num() );
	}
	// Alphabetically extra sound list.
	if ( OutExtraSounds.Num() > 0 )
	{
		Sort<USE_COMPARE_CONSTREF(FString, GearEditorCommandlets)>( OutExtraSounds.GetTypedData(), OutExtraSounds.Num() );
	}
}

class FSoundInfo
{
public:
	FString SoundName;

	FLOAT RefMem;
	FLOAT TgtMem;
	FLOAT MemRatio;
	FLOAT MemDifference;

	FLOAT RefDuration;
	FLOAT TgtDuration;
	FLOAT Ratio;
	FLOAT Difference;

	enum SortType
	{
		ST_Name,

		ST_RefMem,
		ST_TgtMem,
		ST_MemRatio,
		ST_MemDifference,

		ST_RefDuration,
		ST_TgtDuration,
		ST_Ratio,
		ST_Difference,
	};
	static SortType SortMethod;
};

FSoundInfo::SortType FSoundInfo::SortMethod = ST_Name;

IMPLEMENT_COMPARE_CONSTREF( FSoundInfo, GearEditorCommandlets,
{
	if ( FSoundInfo::SortMethod == FSoundInfo::ST_Name )
	{
		return appStricmp( *A.SoundName, *B.SoundName );
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_RefMem )
	{
		return B.RefMem - A.RefMem > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_TgtMem )
	{
		return B.TgtMem - A.TgtMem > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_MemRatio )
	{
		return B.MemRatio - A.MemRatio > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_MemDifference )
	{
		return B.MemDifference - A.MemDifference > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_RefDuration )
	{
		return B.RefDuration - A.RefDuration > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_TgtDuration )
	{
		return B.TgtDuration - A.TgtDuration > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_Ratio )
	{
		return B.Ratio - A.Ratio > 0.f ? 1 : -1;
	}
	else if ( FSoundInfo::SortMethod == FSoundInfo::ST_Difference )
	{
		return B.Difference - A.Difference > 0.f ? 1 : -1;
	}
	else
	{
		return 0;
	}
})

/**
 * Outputs the results of a sound comparison.
 *
 * @param	Ref				The reference package (package to compare against).
 * @param	Tgt				The target package (package to compare).
 * @param	LongerSounds	Sounds which are in both packages, and which are longer in the tgt package.
 * @param	MissingSounds	Sounds which are in the ref package but not in the tgt package.
 * @param	ExtraSounds		Sounds which are in the tgt package but not in the ref package.
 * @param	CSVLongerFile
 * @param	CSVMissingFile
 * @param	CSVExtraFile
 */
static void OutputComparison(const TCHAR* RefPackageName,
							 const TCHAR* TgtPackageName,
							 const SoundDurationList& Ref,
							 const SoundDurationList& Tgt,
							 const TArray<FString>& LongerSounds,
							 const TArray<FString>& MissingSounds,
							 const TArray<FString>& ExtraSounds,
							 FArchive* CSVLongerFile,
							 FArchive* CSVMissingFile,
							 FArchive* CSVExtraFile
							 )
{
	warnf( TEXT("Comparing package %s to %s:"), TgtPackageName, RefPackageName );

	///////////////////////////////////////////////
	// Output missing sounds.
	if ( MissingSounds.Num() > 0 )
	{
		SET_WARN_COLOR( COLOR_RED );
		warnf( TEXT("========== MISSING SOUNDS ==========") );
		CLEAR_WARN_COLOR();
		for ( INT SoundIndex = 0 ; SoundIndex < MissingSounds.Num() ; ++SoundIndex )
		{
			const FString& Sound = MissingSounds(SoundIndex);
			warnf( TEXT("    %s"), *Sound );

			if ( CSVMissingFile )
			{
				const FString Row( FString::Printf(TEXT("%s%s"), *Sound, LINE_TERMINATOR) );
				CSVMissingFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
			}
		}
	}

	///////////////////////////////////////////////
	// Output extra sounds.
	if ( ExtraSounds.Num() > 0 )
	{
		SET_WARN_COLOR( COLOR_YELLOW );
		warnf( TEXT("========== EXTRA SOUNDS ==========") );
		CLEAR_WARN_COLOR();
		for ( INT SoundIndex = 0 ; SoundIndex < ExtraSounds.Num() ; ++SoundIndex )
		{
			const FString& Sound = ExtraSounds(SoundIndex);
			warnf( TEXT("    %s"), *Sound );

			if ( CSVExtraFile )
			{
				const FString Row( FString::Printf(TEXT("%s%s"), *Sound, LINE_TERMINATOR) );
				CSVExtraFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
			}
		}
	}

	///////////////////////////////////////////////
	// Output longer sounds.

	// Assemble data.
	TArray<FSoundInfo> SoundInfo;
	SoundInfo.Empty( LongerSounds.Num() );
	SoundInfo.AddZeroed( LongerSounds.Num() );

	for ( INT SoundIndex = 0 ; SoundIndex < LongerSounds.Num() ; ++SoundIndex )
	{
		const FString& Sound = LongerSounds(SoundIndex);
		const FSoundSrcData* RefData = Ref.Find( Sound );
		const FSoundSrcData* TgtData = Tgt.Find( Sound );
		check( RefData );
		check( TgtData );
		check( TgtData->Duration > RefData->Duration );

		const FLOAT Ratio = TgtData->Duration / RefData->Duration;
		const FLOAT Difference = TgtData->Duration - RefData->Duration;
		const FLOAT MemRatio = RefData->CookedDataSizeK > 0.f ? TgtData->CookedDataSizeK / RefData->CookedDataSizeK : 9999.f;
		const FLOAT MemDifference = TgtData->CookedDataSizeK - RefData->CookedDataSizeK;

		FSoundInfo& Info = SoundInfo(SoundIndex);
		Info.SoundName = Sound;
		Info.RefMem = RefData->CookedDataSizeK;
		Info.TgtMem = TgtData->CookedDataSizeK;
		Info.MemRatio = MemRatio;
		Info.MemDifference = MemDifference;
		Info.RefDuration = RefData->Duration;
		Info.TgtDuration = TgtData->Duration;
		Info.Ratio = Ratio;
		Info.Difference = Difference;
	}

	// Sort.
	FSoundInfo::SortMethod = FSoundInfo::ST_Difference;
	Sort<USE_COMPARE_CONSTREF(FSoundInfo, GearEditorCommandlets)>( SoundInfo.GetTypedData(), SoundInfo.Num() );

	// Output.
	warnf( TEXT("==================================================== LONGER SOUNDS ====================================================") );
	warnf( TEXT("%64s: %6s %6s %6s %6s %6s %6s %6s %6s"),
		TEXT("Sound Name"), TEXT("Ref(K)"), TEXT("Tgt(K)"), TEXT("RatioK"), TEXT("Dif(K)"), TEXT("RefDur"), TEXT("TgtDur"), TEXT("Ratio"), TEXT("DifDur") );

	if ( CSVLongerFile )
	{
		const FString Header = FString::Printf( TEXT("%s,%s,%s,%s,%s,%s,%s,%s,%s%s"),
			TEXT("Sound Name"), TEXT("RefSize(K)"), TEXT("TgtSize(K)"), TEXT("SizeRatio"), TEXT("SizeDiff (K)"), TEXT("RefDuration(sec)"), TEXT("TgtDuration(sec)"), TEXT("RatioDuartion"), TEXT("DurationDiff(sec)"), LINE_TERMINATOR );
		CSVLongerFile->Serialize( TCHAR_TO_ANSI( *Header ), Header.Len() );
	}

	if ( SoundInfo.Num() > 0 )
	{
		SET_WARN_COLOR( COLOR_RED );
		warnf( TEXT("-------------------------------------------  Sounds with way too much slop --------------------------------------------") );
		CLEAR_WARN_COLOR();
	}

	UBOOL bDrewMedLargeSeparator = FALSE;
	UBOOL bDrewSmallMedSeparator = FALSE;

	for ( INT SoundIndex = 0 ; SoundIndex < SoundInfo.Num() ; ++SoundIndex )
	{
		const FSoundInfo& Info = SoundInfo(SoundIndex);
		if ( FSoundInfo::SortMethod == FSoundInfo::ST_Difference)
		{
			// Set the log color and draw spacers based on the difference in sound durations.
			const UBOOL Small = Info.Difference < 0.4f;
			const UBOOL Med = !Small && Info.Difference < 0.75f;
			const UBOOL Large = !Med;
			if ( !Large && !bDrewMedLargeSeparator )
			{
				SET_WARN_COLOR( COLOR_YELLOW );
				warnf( TEXT("---------------------------------------------  Sounds with some slop ---------------------------------------------------") );
				bDrewMedLargeSeparator = TRUE;
			}
			if ( Small && !bDrewSmallMedSeparator )
			{
				SET_WARN_COLOR( COLOR_GREEN );
				warnf( TEXT("--------------------------------------------  Sounds with a bit of slop ------------------------------------------------") );
				bDrewSmallMedSeparator = TRUE;
			}
			SET_WARN_COLOR( Small ? COLOR_GREEN : (Med ? COLOR_YELLOW : COLOR_RED) );
		}

		const FString RowLog = FString::Printf( TEXT("%64s: %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"),
			*Info.SoundName, Info.RefMem, Info.TgtMem, Info.MemRatio, Info.MemDifference, Info.RefDuration, Info.TgtDuration, Info.Ratio, Info.Difference );
		warnf( TEXT("%s"), *RowLog );

		if ( CSVLongerFile )
		{
			const FString RowFile = FString::Printf( TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f%s"),
				*Info.SoundName, Info.RefMem, Info.TgtMem, Info.MemRatio, Info.MemDifference, Info.RefDuration, Info.TgtDuration, Info.Ratio, Info.Difference, LINE_TERMINATOR );
			CSVLongerFile->Serialize( TCHAR_TO_ANSI( *RowFile ), RowFile.Len() );
		}

		CLEAR_WARN_COLOR();
	}
	warnf( TEXT("%i Total"), SoundInfo.Num() );
}

/**
 * Data for a specific language version of a sound package.
 */
class FSoundPackageInfo
{
public:
	/** The language-invariant package name. */
	FString PackageBaseName;
	/** The langauge version for the package associated with these stats. */
	FString	LanguageExt;
	/** The number of USoundNodeWave objects in this package. */
	INT NumSounds;
	/** Sum of the size of the cooked data associated with all USoundNodeWave objects in this package, in KB. */
	FLOAT TotalCookedDataSizeK;
};

static TArray<FSoundPackageInfo> GSoundPackageInfo;

static void AssembleSoundStatsForPackage(const TCHAR* BasePackageName, const TCHAR* LanguageExt, SoundDurationList& OutSoundDurations)
{
	check( BasePackageName );
	check( LanguageExt );
	OutSoundDurations.Empty();

	// Load the package.
	const FString PackageName = MakeLocalizedPackageName( BasePackageName, LanguageExt );
	UObject* Package = UObject::LoadPackage( NULL, *PackageName, LOAD_None );
	if ( !Package )
	{
		warnf( NAME_Warning, TEXT("Failed to load package %s"), *PackageName );
	}
	else
	{
		FSoundPackageInfo SoundPackageInfo;
		SoundPackageInfo.PackageBaseName = BasePackageName;
		SoundPackageInfo.LanguageExt = LanguageExt;
		SoundPackageInfo.NumSounds = 0;
		SoundPackageInfo.TotalCookedDataSizeK = 0.f;

		// Find all waves in the package and record their data.
		for ( TObjectIterator<USoundNodeWave> It ; It ; ++It )
		{
			if ( It->GetOutermost() == Package )
			{
				USoundNodeWave* Wave = static_cast<USoundNodeWave*>( *It );
				const FString WaveName = Wave->GetPathName( Package );

				// Record wave data.
				FSoundSrcData WaveSrcData;
				WaveSrcData.Duration = Wave->GetDuration();
				//@fixme - gearsetup
				//WaveSrcData.CookedDataSizeK = static_cast<FLOAT>( Wave->CachedCookedXbox360Data.GetBulkDataSizeOnDisk() ) / 1024.f;
				OutSoundDurations.Set( WaveName, WaveSrcData );

				// Add wave data to package stats.
				SoundPackageInfo.NumSounds++;
				SoundPackageInfo.TotalCookedDataSizeK += WaveSrcData.CookedDataSizeK;
			}
		}

		GSoundPackageInfo.AddItem( SoundPackageInfo );

		// Unload the package.
		UObject::CollectGarbage( RF_Native );
	}
}

static void OutputPackageComparisons(const TCHAR* CSVDirectory)
{
	const FString	CSVSummaryFilename	= FString::Printf(TEXT("%sPackageComparisons.csv"), CSVDirectory );
	FArchive*		CSVSummaryFile		= GFileManager->CreateFileWriter( *CSVSummaryFilename );

	if ( CSVSummaryFile )
	{
		// Print out a header string for the csv file.
		const FString Header = FString::Printf( TEXT("PackageName,Language,NumSounds,CookedDataSizeDiff (K)%s"), LINE_TERMINATOR );
		CSVSummaryFile->Serialize( TCHAR_TO_ANSI( *Header ), Header.Len() );
	}

	for ( INT IntPackageIndex = 0 ; IntPackageIndex < GSoundPackageInfo.Num() ; ++IntPackageIndex )
	{
		const FSoundPackageInfo& IntPackage = GSoundPackageInfo(IntPackageIndex);
		if ( IntPackage.LanguageExt == TEXT("INT") )
		{
			for ( INT NonIntPackageIndex = 0 ; NonIntPackageIndex < GSoundPackageInfo.Num() ; ++NonIntPackageIndex )
			{
				if ( IntPackageIndex != NonIntPackageIndex )
				{
					const FSoundPackageInfo& NonIntPackage = GSoundPackageInfo(NonIntPackageIndex);
					if ( IntPackage.PackageBaseName == NonIntPackage.PackageBaseName )
					{
						if ( CSVSummaryFile )
						{
							const FString RowFile = FString::Printf( TEXT("%s,%s,%i,%.2f%s"),
								*NonIntPackage.PackageBaseName
								,*NonIntPackage.LanguageExt
								,NonIntPackage.NumSounds - IntPackage.NumSounds
								,NonIntPackage.TotalCookedDataSizeK - IntPackage.TotalCookedDataSizeK
								,LINE_TERMINATOR
								);
							CSVSummaryFile->Serialize( TCHAR_TO_ANSI( *RowFile ), RowFile.Len() );
						}
					}
				}
			}
		}
	}
	CloseCSVFile( CSVSummaryFile, *CSVSummaryFilename );
}

INT UAnalyzeLocalizedSoundCommandlet::Main(const FString& Params)
{
	const TCHAR* PackageNames[] = {
		TEXT("Human_Anya_Chatter"),
		TEXT("Human_Anya_Dialog"),
		TEXT("Human_Baird_Chatter"),
		TEXT("Human_Baird_Dialog"),
		TEXT("Human_Dom_Chatter"),
		TEXT("Human_Dom_Dialog"),
		TEXT("Human_Gus_Chatter"),
		TEXT("Human_Gus_Dialog"),
		TEXT("Human_Hoffman_Chatter"),
		TEXT("Human_Hoffman_Dialog"),
		TEXT("Human_Marcus_Chatter"),
		TEXT("Human_Marcus_Dialog"),
		TEXT("Human_Mihn_Chatter"),
		TEXT("Human_Minh_Dialog"),
		TEXT("Human_Myrrah_Chatter"),
		TEXT("Human_Myrrah_Dialog"),
		TEXT("Human_Pilot_Dialog"),
		TEXT("Human_Redshirt_Chatter"),
		TEXT("Human_Redshirt_Dialog"),
		TEXT("Human_Stranded_Chatter"),
		TEXT("Human_Stranded_Dialog"),
		TEXT("Locust_Boomer_Chatter"),
		TEXT("Locust_Drone_Chatter"),
		TEXT("Locust_Raam_Chatter"),
		TEXT("Locust_Theron_Chatter"),
		NULL
	};

	const TCHAR* LangExts[] = {
		TEXT("deu"),
		TEXT("esm"),
		TEXT("esn"),
		TEXT("fra"),
		TEXT("ita"),
		TEXT("jpn"),
		NULL
	};

	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	const FString CurrentTime = FString::Printf(TEXT("%i_%02i_%02i-%02i_%02i_%02i"), Year, Month, Day, Hour, Min, Sec );

	// Re-used helper variables for writing to CSV file.
	FString		CSVDirectory		= appGameLogDir() + TEXT("AnalyzeLocalizedSound-") + CurrentTime + PATH_SEPARATOR;
	FString		CSVMissingDirectory	= CSVDirectory + TEXT("Missing") + PATH_SEPARATOR;
	FString		CSVExtraDirectory	= CSVDirectory + TEXT("Extra") + PATH_SEPARATOR;
	FString		CSVLongerFilename	= TEXT("");
	FString		CSVMissingFilename	= TEXT("");
	FString		CSVExtraFilename	= TEXT("");
	FArchive*	CSVLongerFile		= NULL;
	FArchive*	CSVMissingFile		= NULL;
	FArchive*	CSVExtraFile		= NULL;

	GFileManager->MakeDirectory( *CSVDirectory );
	GFileManager->MakeDirectory( *CSVMissingDirectory );
	GFileManager->MakeDirectory( *CSVExtraDirectory );

	for ( INT PackageIndex = 0 ; PackageNames[PackageIndex] ; ++PackageIndex )
	{
		SoundDurationList RefSoundDurations;
		AssembleSoundStatsForPackage( PackageNames[PackageIndex], TEXT("INT"), RefSoundDurations );
		if ( RefSoundDurations.Num() > 0 )
		{
			// Compare against each localized language version of that package.
			for ( INT LangIndex = 0 ; LangExts[LangIndex] ; ++LangIndex )
			{
				SoundDurationList TgtSoundDurations;
				AssembleSoundStatsForPackage( PackageNames[PackageIndex], LangExts[LangIndex], TgtSoundDurations );
				if ( TgtSoundDurations.Num() > 0 )
				{
					// Compare sounds.
					TArray<FString> LongerSounds;
					TArray<FString> MissingSounds;
					TArray<FString> ExtraSounds;
					CompareSounds( RefSoundDurations, TgtSoundDurations, LongerSounds, MissingSounds, ExtraSounds );

					// Create files.
					CSVLongerFilename	= FString::Printf(TEXT("%s%s_%s_Longer.csv"), *CSVDirectory, LangExts[LangIndex], PackageNames[PackageIndex] );
					CSVLongerFile		= GFileManager->CreateFileWriter( *CSVLongerFilename );

					CSVMissingFilename	= FString::Printf(TEXT("%s%s_%s_Missing.csv"), *CSVMissingDirectory, LangExts[LangIndex], PackageNames[PackageIndex] );
					CSVMissingFile		= GFileManager->CreateFileWriter( *CSVMissingFilename );

					CSVExtraFilename	= FString::Printf(TEXT("%s%s_%s_Extra.csv"), *CSVExtraDirectory, LangExts[LangIndex], PackageNames[PackageIndex] );
					CSVExtraFile		= GFileManager->CreateFileWriter( *CSVExtraFilename );

					// Output the comparisons.
					OutputComparison(
						*MakeLocalizedPackageName( PackageNames[PackageIndex], TEXT("INT") )
						,*MakeLocalizedPackageName( PackageNames[PackageIndex], LangExts[LangIndex] )
						,RefSoundDurations
						,TgtSoundDurations
						,LongerSounds
						,MissingSounds
						,ExtraSounds
						, CSVLongerFile
						, CSVMissingFile
						, CSVExtraFile
							);

					// Close files.
					CloseCSVFile( CSVLongerFile, *CSVLongerFilename );
					CloseCSVFile( CSVMissingFile, *CSVMissingFilename );
					CloseCSVFile( CSVExtraFile, *CSVExtraFilename );
				}
			} // for LangExts
		}
	} // for PackageNames

	// Finally, output package comparisons.
	OutputPackageComparisons( *CSVDirectory );

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UAnalyzeGUDSoundsCommandlet
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(UAnalyzeGUDSoundsCommandlet);

INT UAnalyzeGUDSoundsCommandlet::Main(const FString& Params)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Look to the .ini for the list of pawn classes.
	// Check each pawn
	const TCHAR* PawnIniSection = TEXT("Cooker.GearPawns");
	TMultiMap<FString,FString>* IniPawnsAndRemotesList = GConfig->GetSectionPrivate(PawnIniSection, FALSE, TRUE, GEditorIni);
	if (IniPawnsAndRemotesList)
	{
		TMap<FString, TArray<FString>> PawnToGUDBankNames;

		// Split up the remotes and the pawns...
		TArray<FString> RemotesList;
		TArray<FString> PawnsList;

		for (TMultiMap<FString,FString>::TIterator It(*IniPawnsAndRemotesList); It; ++It)
		{
			FString TypeString = It.Key();
			FString ValueString = It.Value();

			if (TypeString == TEXT("RemoteList"))
			{
				RemotesList.AddUniqueItem(ValueString);
			}
			else
			{
				PawnsList.AddUniqueItem(ValueString);
			}
		}

		// Now, handle each type (GearPawn vs RemoteSpeaker) grabbing their MasterGUDClassName(s)
		for (INT RemoteIndex = 0; RemoteIndex < RemotesList.Num(); RemoteIndex++)
		{
			FString RemoteName = RemotesList(RemoteIndex);
			debugf(TEXT("Loading remote speaker for GUD check: %s"), *RemoteName);

			// Load the class...
			UClass* RemoteClass = LoadObject<UClass>(NULL, *RemoteName, NULL, LOAD_None, NULL);
			if (RemoteClass)
			{
				AGearRemoteSpeaker* RemoteSpeaker = Cast<AGearRemoteSpeaker>(RemoteClass->GetDefaultObject());
				if (RemoteSpeaker)
				{
					for (INT GUDIndex = 0; GUDIndex < RemoteSpeaker->MasterGUDBankClassNames.Num(); GUDIndex++)
					{
						TArray<FString>* GUDNames = PawnToGUDBankNames.Find(RemoteName);
						if (GUDNames)
						{
							// Already had some GUDs...
							(*GUDNames).AddUniqueItem(RemoteSpeaker->MasterGUDBankClassNames(GUDIndex));
						}
						else
						{
							TArray<FString> TempList;
							TArray<FString>& NewList = PawnToGUDBankNames.Set(RemoteName, TempList);
							NewList.AddUniqueItem(RemoteSpeaker->MasterGUDBankClassNames(GUDIndex));
						}
					}
				}
			}
		}

		// Now, handle each type (GearPawn vs PawnSpeaker) grabbing their MasterGUDClassName(s)
		for (INT PawnIndex = 0; PawnIndex < PawnsList.Num(); PawnIndex++)
		{
			FString PawnName = PawnsList(PawnIndex);
			debugf(TEXT("Loading pawn           for GUD check: %s"), *PawnName);
			// Load the class...
			UClass* PawnClass = LoadObject<UClass>(NULL, *PawnName, NULL, LOAD_None, NULL);
			if (PawnClass)
			{
				AGearPawn* Pawn = Cast<AGearPawn>(PawnClass->GetDefaultObject());
				if (Pawn)
				{
					for (INT GUDIndex = 0; GUDIndex < Pawn->MasterGUDBankClassNames.Num(); GUDIndex++)
					{
						TArray<FString>* GUDNames = PawnToGUDBankNames.Find(PawnName);
						if (GUDNames)
						{
							// Already had some GUDs...
							(*GUDNames).AddUniqueItem(Pawn->MasterGUDBankClassNames(GUDIndex));
						}
						else
						{
							TArray<FString> TempList;
							TArray<FString>& NewList = PawnToGUDBankNames.Set(PawnName, TempList);
							NewList.AddUniqueItem(Pawn->MasterGUDBankClassNames(GUDIndex));
						}
					}
				}
			}
		}

		// Process the GUDS.
		TArray<FString> ProcessedGUDBanks;

		static const INT A2L_NUM_DISTRIBUTION_BUCKETS = 50;
		struct FActionToLinesInfo
		{
			FString PawnOrRemoteSpeakerName;
			FString GUDBankSource;
			INT NonCombatLines_ActionsWith;
			INT NonCombatLines_Total;
			INT NonCombatLines_High;
			INT NonCombatLines_Low;
			TArray<INT> NonCombatLines_Distribution;
			INT CombatLines_ActionsWith;
			INT CombatLines_Total;
			INT CombatLines_High;
			INT CombatLines_Low;
			TArray<INT> CombatLines_Distribution;
			INT Lines_ActionsWith;
			INT Lines_Total;
			INT Lines_High;
			INT Lines_Low;
			TArray<INT> Lines_Distribution;
			INT EmptyActions;
			INT TotalResourceSize360;

			FActionToLinesInfo() :
				  NonCombatLines_ActionsWith(0)
				, NonCombatLines_Total(0)
				, NonCombatLines_High(0)
				, NonCombatLines_Low(5000)
				, CombatLines_ActionsWith(0)
				, CombatLines_Total(0)
				, CombatLines_High(0)
				, CombatLines_Low(5000)
				, Lines_ActionsWith(0)
				, Lines_Total(0)
				, Lines_High(0)
				, Lines_Low(5000)
				, EmptyActions(0)
				, TotalResourceSize360(0)
			{
				appMemzero(&NonCombatLines_Distribution, sizeof(TArray<INT>));
				NonCombatLines_Distribution.AddZeroed(A2L_NUM_DISTRIBUTION_BUCKETS);
				appMemzero(&CombatLines_Distribution, sizeof(TArray<INT>));
				CombatLines_Distribution.AddZeroed(A2L_NUM_DISTRIBUTION_BUCKETS);
				appMemzero(&Lines_Distribution, sizeof(TArray<INT>));
				Lines_Distribution.AddZeroed(A2L_NUM_DISTRIBUTION_BUCKETS);
			};
		};

		TArray<FActionToLinesInfo> ActionToLinesInfo;

		for (TMap<FString, TArray<FString>>::TIterator It(PawnToGUDBankNames); It; ++It)
		{
			FString& PawnOrRemoteSpeaker = It.Key();
			TArray<FString>& GUDBanks = It.Value();

			debugf(TEXT("Processing Pawn/RemoteSpeaker: %s"), *PawnOrRemoteSpeaker);
			for (INT GUDIndex = 0; GUDIndex < GUDBanks.Num(); GUDIndex++)
			{
				FString GUDBankName = GUDBanks(GUDIndex);
				INT DummyIndex;
				if (ProcessedGUDBanks.FindItem(GUDBanks(GUDIndex), DummyIndex) == FALSE)
				{
					debugf(TEXT("\t%s"), *GUDBankName);

					// Check the relative numbers from this bank...
					UClass* GUDSClass = LoadObject<UClass>(NULL, *GUDBankName, NULL, LOAD_None, NULL);
					if (GUDSClass)
					{
						// Loaded object!!!!
						debugf(TEXT("\t\tLoaded GUDS class data: %s"), *(GUDSClass->GetName()));

						UGUDBank* DefaultBank = Cast<UGUDBank>(GUDSClass->GetDefaultObject());
						if (DefaultBank)
						{
							// Process the actions...
							FActionToLinesInfo* A2LInfo = new(ActionToLinesInfo)FActionToLinesInfo();
							check(A2LInfo);

							A2LInfo->PawnOrRemoteSpeakerName = PawnOrRemoteSpeaker;
							A2LInfo->GUDBankSource = GUDBankName;

							for (INT ActionIndex = 0; ActionIndex < DefaultBank->GUDActions.Num(); ActionIndex++)
							{
								const FGUDAction& CheckAction = DefaultBank->GUDActions(ActionIndex);
		
								// combat-only lines
								INT TrueCLCount = CheckAction.CombatOnlyLineIndices.Num();
								INT CombatLineCount = 0;
								INT CombatLineALCount = 0;
								for (INT CLIndex = 0; CLIndex < TrueCLCount; CLIndex++)
								{
									FGUDLine& Line = DefaultBank->GUDLines(CheckAction.CombatOnlyLineIndices(CLIndex));
									if (Line.bAlwaysLoad == FALSE)
									{
										CombatLineCount++;
									}
									else
									{
										CombatLineALCount++;
									}
								}
								if (CombatLineCount > 0)
								{
									INT const DistributionBucket = Min(CombatLineCount, A2L_NUM_DISTRIBUTION_BUCKETS);
									A2LInfo->CombatLines_Distribution(DistributionBucket) = A2LInfo->CombatLines_Distribution(DistributionBucket) + 1;
									A2LInfo->CombatLines_Total += CombatLineCount;
									if (CombatLineCount > A2LInfo->CombatLines_High)
									{
										A2LInfo->CombatLines_High = CombatLineCount;
									}
									if (CombatLineCount < A2LInfo->CombatLines_Low)
									{
										A2LInfo->CombatLines_Low = CombatLineCount;
									}
									A2LInfo->CombatLines_ActionsWith++;
								}
								if (CombatLineALCount > 0)
								{
									A2LInfo->CombatLines_Distribution(0) = A2LInfo->CombatLines_Distribution(0) + CombatLineALCount;
								}

								// non-combat-only lines
								INT TrueNCLCount = CheckAction.NonCombatOnlyLineIndices.Num();
								INT NonCombatLineCount = 0;
								INT NonCombatLineALCount = 0;
								for (INT NCLIndex = 0; NCLIndex < TrueNCLCount; NCLIndex++)
								{
									FGUDLine& Line = DefaultBank->GUDLines(CheckAction.NonCombatOnlyLineIndices(NCLIndex));
									if (Line.bAlwaysLoad == FALSE)
									{
										NonCombatLineCount++;
									}
									else
									{
										NonCombatLineALCount++;
									}
								}
								if (NonCombatLineCount > 0)
								{
									INT const DistributionBucket = Min(NonCombatLineCount, A2L_NUM_DISTRIBUTION_BUCKETS);
									A2LInfo->NonCombatLines_Distribution(DistributionBucket) = A2LInfo->NonCombatLines_Distribution(DistributionBucket) + 1;
									A2LInfo->NonCombatLines_Total += NonCombatLineCount;
									if (NonCombatLineCount > A2LInfo->NonCombatLines_High)
									{
										A2LInfo->CombatLines_High = NonCombatLineCount;
									}
									if (NonCombatLineCount < A2LInfo->NonCombatLines_Low)
									{
										A2LInfo->NonCombatLines_Low = NonCombatLineCount;
									}
									A2LInfo->NonCombatLines_ActionsWith++;
								}
								if (NonCombatLineALCount > 0)
								{
									A2LInfo->NonCombatLines_Distribution(0) = A2LInfo->NonCombatLines_Distribution(0) + NonCombatLineALCount;
								}



								// regular/always-appropriate lines
								INT TrueLineCount = CheckAction.LineIndices.Num();
								INT LineCount = 0;
								INT LineALCount = 0;
								for (INT LIndex = 0; LIndex < TrueLineCount; LIndex++)
								{
									FGUDLine& Line = DefaultBank->GUDLines(CheckAction.LineIndices(LIndex));
									if (Line.bAlwaysLoad == FALSE)
									{
										LineCount++;
									}
									else
									{
										LineALCount++;
									}
								}
								if (LineCount > 0)
								{
									INT const DistributionBucket = Min(LineCount, A2L_NUM_DISTRIBUTION_BUCKETS);
									A2LInfo->Lines_Distribution(DistributionBucket) = A2LInfo->Lines_Distribution(DistributionBucket) + 1;
									A2LInfo->Lines_Total += LineCount;
									if (LineCount > A2LInfo->Lines_High)
									{
										A2LInfo->Lines_High = LineCount;
									}
									if (LineCount < A2LInfo->Lines_Low)
									{
										A2LInfo->Lines_Low = LineCount;
									}
									A2LInfo->Lines_ActionsWith++;
								}
								if (LineALCount > 0)
								{
									A2LInfo->Lines_Distribution(0) = A2LInfo->Lines_Distribution(0) + LineALCount;
								}

								// empty?
								if ((TrueLineCount == 0) && (TrueCLCount == 0) && (TrueNCLCount == 0))
								{
									A2LInfo->EmptyActions++;
								}
							}

							// Handle cases where there were no lines at all!
							if (A2LInfo->NonCombatLines_Low == 5000)	{	A2LInfo->NonCombatLines_Low = 0;	}
							if (A2LInfo->CombatLines_Low == 5000)		{	A2LInfo->CombatLines_Low = 0;		}
							if (A2LInfo->Lines_Low == 5000)				{	A2LInfo->Lines_Low = 0;				}

							// Now, process the audio...
							for (INT LinesIndex = 0; LinesIndex < DefaultBank->GUDLines.Num(); LinesIndex++)
							{
								const FGUDLine& GUDLine = DefaultBank->GUDLines(LinesIndex);
								if (GUDLine.Audio)
								{
									A2LInfo->TotalResourceSize360 += GUDLine.Audio->GetResourceSize(UE3::PLATFORM_Xenon);
								}
							}
						}
						else
						{
							debugf(TEXT("\t\t\tFAILED TO GET DEFAULT OBJECT!"));
						}
					}
					else
					{
						debugf(TEXT("\t\tFAILED TO LOAD CLASS!"));
					}

					ProcessedGUDBanks.AddUniqueItem(GUDBankName);
				}
				else
				{
					debugf(TEXT("\t[ALREADY PROCESSED] %s"), *GUDBankName);
				}
			}
		}

		INT A2LIndex;
		debugf(TEXT("Pawn/RemoteSpeaker,GUDBank Source,Actions w/ CombatLines,Total CombatLines,High CombatLines,Low CombatLines,Actions w/ NonCombatLines,Total NonCombatLines,High NonCombatLines,Low NonCombatLines,Actions w/ Lines,Total Line,High Lines,Low Lines,Empty Actions,360Size"));
		for (A2LIndex = 0; A2LIndex < ActionToLinesInfo.Num(); A2LIndex++)
		{
			FActionToLinesInfo& A2LInfo = ActionToLinesInfo(A2LIndex);
			debugf(TEXT("%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), 
				*(A2LInfo.PawnOrRemoteSpeakerName),
				*(A2LInfo.GUDBankSource),
				A2LInfo.CombatLines_ActionsWith,
				A2LInfo.CombatLines_Total,
				A2LInfo.CombatLines_High,
				A2LInfo.CombatLines_Low,
				A2LInfo.NonCombatLines_ActionsWith,
				A2LInfo.NonCombatLines_Total,
				A2LInfo.NonCombatLines_High,
				A2LInfo.NonCombatLines_Low,
				A2LInfo.Lines_ActionsWith,
				A2LInfo.Lines_Total,
				A2LInfo.Lines_High,
				A2LInfo.Lines_Low,
				A2LInfo.EmptyActions,
				A2LInfo.TotalResourceSize360);
		}
		
		FString CombatLineDistHeader = FString(TEXT("Pawn/RemoteSpeaker,GUDBank Source"));
		FString NonCombatLineDistHeader = FString(TEXT("Pawn/RemoteSpeaker,GUDBank Source"));
		FString LineDistHeader = FString(TEXT("Pawn/RemoteSpeaker,GUDBank Source"));
		for (INT TempIndex = 0; TempIndex < 20; TempIndex++)
		{
			CombatLineDistHeader += FString::Printf(TEXT(",CL %d"), TempIndex);
			NonCombatLineDistHeader += FString::Printf(TEXT(",NCL %d"), TempIndex);
			LineDistHeader += FString::Printf(TEXT(",L %d"), TempIndex);
		}

		debugf(*CombatLineDistHeader);
		for (A2LIndex = 0; A2LIndex < ActionToLinesInfo.Num(); A2LIndex++)
		{
			FActionToLinesInfo& A2LInfo = ActionToLinesInfo(A2LIndex);
			FString DistOut = FString::Printf(TEXT("%s,%s"), 
				*(A2LInfo.PawnOrRemoteSpeakerName),
				*(A2LInfo.GUDBankSource));
			for (INT CLDIndex = 0; CLDIndex < A2LInfo.CombatLines_Distribution.Num(); CLDIndex++)
			{
				DistOut += FString::Printf(TEXT(",%d"), A2LInfo.CombatLines_Distribution(CLDIndex));
			}
			debugf(*DistOut);
		}

		debugf(*NonCombatLineDistHeader);
		for (A2LIndex = 0; A2LIndex < ActionToLinesInfo.Num(); A2LIndex++)
		{
			FActionToLinesInfo& A2LInfo = ActionToLinesInfo(A2LIndex);
			FString DistOut = FString::Printf(TEXT("%s,%s"), 
				*(A2LInfo.PawnOrRemoteSpeakerName),
				*(A2LInfo.GUDBankSource));
			for (INT NCLDIndex = 0; NCLDIndex < A2LInfo.NonCombatLines_Distribution.Num(); NCLDIndex++)
			{
				DistOut += FString::Printf(TEXT(",%d"), A2LInfo.NonCombatLines_Distribution(NCLDIndex));
			}
			debugf(*DistOut);
		}

		debugf(*LineDistHeader);
		for (A2LIndex = 0; A2LIndex < ActionToLinesInfo.Num(); A2LIndex++)
		{
			FActionToLinesInfo& A2LInfo = ActionToLinesInfo(A2LIndex);
			FString DistOut = FString::Printf(TEXT("%s,%s"), 
				*(A2LInfo.PawnOrRemoteSpeakerName),
				*(A2LInfo.GUDBankSource));
			for (INT LDIndex = 0; LDIndex < A2LInfo.Lines_Distribution.Num(); LDIndex++)
			{
				DistOut += FString::Printf(TEXT(",%d"), A2LInfo.Lines_Distribution(LDIndex));
			}
			debugf(*DistOut);
		}
	}

	return 0;
}


IMPLEMENT_CLASS(UWeaponDistributionAcrossMapsCommandlet);


/** 
 * This will look in each package that is passed into it and then it will count the number OF  GearWeapons that exist in that package
 * 
 **/

typedef TMap< FString, UINT > StringToUINTType;
typedef TMap< FString, StringToUINTType > StringToStringToUINTType;
 
StringToStringToUINTType AllWeapons;
StringToStringToUINTType AllMaps;

struct WeaponDistributionAcrossMapsFunctor
{

	void UpdateStringWithNumInstances( StringToUINTType* TheMapCountType, const FString& MapName )
	{
		UINT* CurrCount = TheMapCountType->Find( MapName );
		if( CurrCount != NULL )
		{
			(*CurrCount)++;
		}
		else
		{
			TheMapCountType->Set( MapName, 1 );
		}

	}

	void UpdateAllWeapons( const FString& WeaponName, const FString& MapName )
	{
		StringToUINTType* MapData = AllWeapons.Find( WeaponName );

		// if exists update
		if( MapData != NULL )
		{
			UpdateStringWithNumInstances( MapData, MapName );
		}
		// so if this ref does not exist
		else
		{
			StringToUINTType NewData;
			AllWeapons.Set( WeaponName, NewData );
			StringToUINTType* MapData = AllWeapons.Find( WeaponName );
			UpdateStringWithNumInstances( MapData, MapName );
		}
	}


	void UpdateAllMaps( const FString& WeaponName, const FString& MapName )
	{
		StringToUINTType* MapData = AllMaps.Find( MapName );

		// if exists update
		if( MapData != NULL )
		{
			UpdateStringWithNumInstances( MapData, WeaponName );
		}
		// so if this ref does not exist
		else
		{
			StringToUINTType NewData;
			AllMaps.Set( MapName, NewData );
			StringToUINTType* MapData = AllMaps.Find( MapName );
			UpdateStringWithNumInstances( MapData, WeaponName );
		}
	}




	template< typename OBJECTYPE >
	void DoIt( UPackage* Package, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		const FString PackageName = Package->GetName();
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			// we want to skip all maps except MP_ and SP_
			if( PackageName.InStr( TEXT( "MP_" ) ) != INDEX_NONE 
				|| PackageName.InStr( TEXT( "SP_" ) ) != INDEX_NONE 
				)
			{
				// do nothing as we want to process this map
			}
			else
			{
				continue;
			}

			const OBJECTYPE* const TheGearWeaponClass = *It;

			if( TheGearWeaponClass->IsChildOf( AGearWeapon::StaticClass() ) == TRUE
				&& TheGearWeaponClass->HasAnyClassFlags(CLASS_Abstract) == FALSE
				&& TheGearWeaponClass->GetOuter()->GetName() != TEXT( "GearGame" )
				)
			{
				const FString WeaponName = TheGearWeaponClass->GetName();
				warnf( TEXT( "Weapon: %s" ), *WeaponName );

				UpdateAllWeapons( WeaponName, PackageName );
				UpdateAllMaps( WeaponName, PackageName );
			}
		}

	}
};


INT UWeaponDistributionAcrossMapsCommandlet::Main(const FString& Params)
{
	DoActionToAllPackages<UClass, WeaponDistributionAcrossMapsFunctor>(Params);

	// now print out everything
	for( StringToStringToUINTType::TIterator Itr(AllWeapons); Itr; ++Itr )
	{
		const FString& WeaponName = Itr.Key();
		warnf( TEXT( "%s" ), *WeaponName );

		StringToUINTType& MapCountEntry = Itr.Value();

		for( StringToUINTType::TIterator Jtr(MapCountEntry); Jtr; ++Jtr )
		{
			warnf( TEXT( "   %s %d" ), *Jtr.Key(), Jtr.Value() );
		}
	}

	// now print out everything
	for( StringToStringToUINTType::TIterator Itr(AllMaps); Itr; ++Itr )
	{
		const FString& WeaponName = Itr.Key();
		warnf( TEXT( "%s" ), *WeaponName );

		StringToUINTType& MapCountEntry = Itr.Value();

		for( StringToUINTType::TIterator Jtr(MapCountEntry); Jtr; ++Jtr )
		{
			warnf( TEXT( "   %s %d" ), *Jtr.Key(), Jtr.Value() );
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UListDialogueCommandlet
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(UListDialogueCommandlet);

INT UListDialogueCommandlet::Main(const FString& Params)
{
	TArray<FString> Tokens, Switches;
	ParseCommandLine(*Params, Tokens, Switches);

	UBOOL bSkipGUDS = Switches.ContainsItem(TEXT("SKIPGUDS"));
	UBOOL bSkipSoundGroup = Switches.ContainsItem(TEXT("SKIPSOUNDGROUP"));

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Find list of remotespeakers and pawns that have guds

	TArray<FString> RemoteSpeakerClassList;
	TArray<FString> PawnClassList;	
	{
		// Look to the .ini for the list of pawn classes.
		const TCHAR* PawnIniSection = TEXT("Cooker.GearPawns");
		TMultiMap<FString,FString>* IniPawnsAndRemotesList = GConfig->GetSectionPrivate(PawnIniSection, FALSE, TRUE, GEditorIni);

		if (IniPawnsAndRemotesList)
		{
			// Split up the remotes and the pawns...
			for (TMultiMap<FString,FString>::TIterator It(*IniPawnsAndRemotesList); It; ++It)
			{
				FString TypeString = It.Key();
				FString ValueString = It.Value();

				if (TypeString == TEXT("RemoteList"))
				{
					RemoteSpeakerClassList.AddUniqueItem(ValueString);
				}
				else
				{
					PawnClassList.AddUniqueItem(ValueString);
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load the requested classes.
	TArray<UClass*> PawnClasses;
	for ( INT ClassIndex = 0 ; ClassIndex < PawnClassList.Num() ; ++ClassIndex )
	{
		const FString& ClassName = PawnClassList(ClassIndex);
		UClass* LoadedClass = LoadClass<AGearPawn>( NULL, *ClassName, NULL, LOAD_None, NULL );
		if ( !LoadedClass )
		{
			warnf( NAME_Log, TEXT("Couldn't load class: %s"), *ClassName );
		}
		else
		{
			warnf( NAME_Log, TEXT("Loaded class: %s"), *ClassName );
			PawnClasses.AddUniqueItem( LoadedClass );
		}
	}

	TArray<UClass*> RemoteSpeakerClasses;
	for ( INT ClassIndex = 0 ; ClassIndex < RemoteSpeakerClassList.Num() ; ++ClassIndex )
	{
		const FString& ClassName = RemoteSpeakerClassList(ClassIndex);
		UClass* LoadedClass = LoadClass<AGearRemoteSpeaker>( NULL, *ClassName, NULL, LOAD_None, NULL );
		if ( !LoadedClass )
		{
			warnf( NAME_Log, TEXT("Couldn't load class: %s"), *ClassName );
		}
		else
		{
			warnf( NAME_Log, TEXT("Loaded class: %s"), *ClassName );
			RemoteSpeakerClasses.AddUniqueItem( LoadedClass );
		}
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// For each class, assemble the list of referenced cues.

	TMap<FString, TArray<USoundCue*>> PawnToReferencedCues;

	for ( INT ClassIndex = 0 ; ClassIndex < PawnClasses.Num() ; ++ClassIndex )
	{
		UClass* const PawnClass = PawnClasses(ClassIndex);
		AGearPawn* ClassDefaultObject = Cast<AGearPawn>( PawnClass->GetDefaultObject( TRUE ) );

		TArray<USoundCue*> TempList;
		TArray<USoundCue*>& ReferencedCues = PawnToReferencedCues.Set(PawnClass->GetName(), TempList);

		if ( ClassDefaultObject )
		{
			// @todo hook up to look at the SoundGroup which are: GearSoundGroup'GearSoundGroupArchetypes

			////////////////////////////////////////
			// Sound group
			if (bSkipSoundGroup == FALSE)
			{
				UGearSoundGroup const* const SoundGroup = ClassDefaultObject->SoundGroup;
				if ( SoundGroup )
				{
					for ( INT Index = 0 ; Index < SoundGroup->VoiceEfforts.Num() ; ++Index )
					{
						const FGearVoiceEffortEntry& EffortEntry = SoundGroup->VoiceEfforts(Index);

						for (INT CueIdx=0; CueIdx<EffortEntry.Sounds.Num(); CueIdx++)
						{
							USoundCue* const Cue = EffortEntry.Sounds(CueIdx);
							if ( Cue )
							{
								ReferencedCues.AddUniqueItem( Cue );
							}
						}
					}

					for ( INT Index = 0 ; Index < SoundGroup->FoleySoundFX.Num() ; ++Index )
					{
						const FGearFoleyEntry& FoleyEntry = SoundGroup->FoleySoundFX(Index);

						for (INT CueIdx=0; CueIdx<FoleyEntry.Sounds.Num(); CueIdx++)
						{
							USoundCue* const Cue = FoleyEntry.Sounds(CueIdx);
							if ( Cue )
							{
								ReferencedCues.AddUniqueItem( Cue );
							}
						}
					}
				}
				else
				{
					warnf( NAME_Log, TEXT("SoundGroup is NULL for class %s"), *PawnClass->GetName() );
				}
			}

			////////////////////////////////////////
			// GUDS
			if (bSkipGUDS == FALSE)
			{
				for (INT MasterBankIdx=0; MasterBankIdx<ClassDefaultObject->MasterGUDBankClassNames.Num(); ++MasterBankIdx)
				{
					UClass* MasterGUDBankClass = LoadClass<UGUDBank>( NULL, *ClassDefaultObject->MasterGUDBankClassNames(MasterBankIdx), NULL, LOAD_None, NULL );
					if ( MasterGUDBankClass )
					{
						UGUDBank* MasterGUDBankClassDefaultObject = Cast<UGUDBank>( MasterGUDBankClass->GetDefaultObject( TRUE ) );
						if ( MasterGUDBankClassDefaultObject )
						{
							for ( INT Index = 0 ; Index < MasterGUDBankClassDefaultObject->GUDLines.Num() ; ++Index )
							{
								const FGUDLine& GUDLine = MasterGUDBankClassDefaultObject->GUDLines(Index);
								USoundCue* Cue = GUDLine.Audio;
								if ( Cue )
								{
									ReferencedCues.AddUniqueItem( Cue );
								}
							}
						}
						else
						{
							warnf( NAME_Log, TEXT("Couldn't create default object for class %s"), *MasterGUDBankClassDefaultObject->GetName() );
						}
					}
					else
					{
						warnf( NAME_Log, TEXT("MasterGUDBankClass is NULL for class %s"), *PawnClass->GetName() );
					}
				}
			}

			////////////////////////////////////////
			// FaceFX of the pawn's skeletal mesh.
			//USkeletalMeshComponent* Mesh = ClassDefaultObject->Mesh;
			//if ( Mesh && Mesh->SkeletalMesh )
			//{
			//	UFaceFXAsset* FaceFXAsset = Mesh->SkeletalMesh->FaceFXAsset;
			//	if ( FaceFXAsset )
			//	{
			//		for ( INT Index = 0 ; Index < FaceFXAsset->ReferencedSoundCues.Num() ; ++Index )
			//		{
			//			USoundCue* Cue = FaceFXAsset->ReferencedSoundCues(Index);
			//			if ( Cue )
			//			{
			//				ReferencedCues.AddUniqueItem( Cue );
			//			}
			//		}
			//	}
			//	else
			//	{
			//		warnf( NAME_Log, TEXT("Skeletal mesh's FaceFX is NULL for class %s"), *PawnClass->GetName() );
			//	}
			//}
			//else
			//{
			//	warnf( NAME_Log, TEXT("Skeletal mesh is NULL for class %s"), *PawnClass->GetName() );
			//}

			//////////////////////////////////////////  keep this here as an example of what we need
			//// do for looking at faceFX stuff.   If we need to reuse the FaceFX external animsets ever
			//// Efforts FaceFX.
			//UFaceFXAnimSet* FaceFXAnimSet = ClassDefaultObject->FAS_Efforts;
			//if ( FaceFXAnimSet )
			//{
			//	for ( INT Index = 0 ; Index < FaceFXAnimSet->ReferencedSoundCues.Num() ; ++Index )
			//	{
			//		USoundCue* Cue = FaceFXAnimSet->ReferencedSoundCues(Index);
			//		if ( Cue )
			//		{
			//			ReferencedCues.AddUniqueItem( Cue );
			//		}
			//	}
			//}
			//else
			//{
			//	warnf( NAME_Log, TEXT("FaceFXAnimSet FAS_Efforts is NULL for class %s"), *PawnClass->GetName() );
			//}
		}
		else
		{
			warnf( NAME_Log, TEXT("Couldn't create default object for class %s"), *PawnClass->GetName() );
		}
	}

	// output per-pawn list, build master list
	TArray<USoundCue*> MasterCueList;
	for (TMap<FString, TArray<USoundCue*>>::TIterator It(PawnToReferencedCues); It; ++It)
	{
		FString& PawnName = It.Key();

		warnf(TEXT("\nPawn: %s"), *PawnName);

		TArray<USoundCue*>& Cues = It.Value();

		for (INT Idx=0; Idx<Cues.Num(); ++Idx)
		{
			warnf(TEXT("  %s"), *Cues(Idx)->GetFullName());
			MasterCueList.AddUniqueItem(Cues(Idx));
		}
	}

	// output master list
	warnf(TEXT("\nUNIQUE CUE LIST"));
	for (INT Idx=0; Idx<MasterCueList.Num(); ++Idx)
	{
		warnf(TEXT("  %s"), *MasterCueList(Idx)->GetFullName());
	}


	return 0;
}

