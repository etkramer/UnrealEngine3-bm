/*=============================================================================
	GearEditorCookerHelper.h: Gear editor cokking helper class declarations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GEAREDITORCOOKERHELPER_H__
#define __GEAREDITORCOOKERHELPER_H__

// Forward declaration
class UGUDToC;

/**
 *	Data structure for tracking and removing FaceFX animations.
 */
struct FGUDSReferencedFaceFXAnimation
{
	/** The animation group. */
	FString AnimGroup;
	/** The animation name. */
	FString AnimName;

	FGUDSReferencedFaceFXAnimation()
	{
	}

	FGUDSReferencedFaceFXAnimation(const FGUDSReferencedFaceFXAnimation& Src)
	{
		AnimGroup = Src.AnimGroup;
		AnimName = Src.AnimName;
	}

	UBOOL operator==(const FGUDSReferencedFaceFXAnimation& Src) const 
	{
		return ((AnimGroup == Src.AnimGroup) && (AnimName == Src.AnimName));
	}

	/**
	 * Serialize function.
	 *
	 * @param	Ar	Archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Ar << AnimGroup;
		Ar << AnimName;
	}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FGUDSReferencedFaceFXAnimation& RefdFFXAnim)
	{
		return Ar << RefdFFXAnim.AnimGroup << RefdFFXAnim.AnimName;
	}
};

/** Helper typedef for referenced FaceFXAnimations. */
typedef TArray<FGUDSReferencedFaceFXAnimation> TGUDSReferencedFaceFXAnimations;

/** 
 *	Data structure for storing the Pawn-<GUD data/FaceFX> relationship.
 */
struct FGUDCueAndFaceFX
{
	/** The master GUD class(es) for the pawn. */
	TArray<FString>	GUDBankNames;
	/** The FaceFXAsset name that goes with it. */
	FString FaceFXAssetName;
	/** Additional FaceFX AnimSets (if required). */
	TArray<FString>	FaceFXAnimSets;

	FGUDCueAndFaceFX()
	{
	}

	FGUDCueAndFaceFX(const FGUDCueAndFaceFX& Src)
	{
		GUDBankNames = Src.GUDBankNames;
		FaceFXAssetName = Src.FaceFXAssetName;
		FaceFXAnimSets = Src.FaceFXAnimSets;
	}

	/**
	 * Serialize function.
	 *
	 * @param	Ar	Archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Ar << GUDBankNames;
		Ar << FaceFXAssetName;
		Ar << FaceFXAnimSets;
	}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FGUDCueAndFaceFX& GUDCueAndFaceFX)
	{
		return Ar << GUDCueAndFaceFX.GUDBankNames << GUDCueAndFaceFX.FaceFXAssetName << GUDCueAndFaceFX.FaceFXAnimSets;
	}
};

/** Data structure for accessing persistent cooker tracking data. */
struct FGUDPersistentCookerData
{
	/** Array of source GUD banks (that should be cleared). */
	TMap<FString, INT> SourceGUDBanks;
	/** Maps the pawn name to the GUDS and FaceFX data it requires. */
	TMap<FString, FGUDCueAndFaceFX> PawnNameToInfoRequired;
	/** Maps the source object to FaceFXAnimations that should be removed. */
	TMap<FString, TArray<FGUDSReferencedFaceFXAnimation>> ReferencedFaceFXAnimations;
	/** Array of sound cues that should be removed. */
	TMap<FString, TMap<FString, INT>> ReferencedSoundCues;
	/** Map of special GUDS package filenames to their 'true' package. */
	TMap<FString, FString> SpecialGUDSPackages;
	/** The standalone seekfree filename pairs that will be generated. */
	TArray<FPackageCookerInfo> StandaloneSeekfreeFilenamePairs;

	/** The TableOfContents for the GUDS */
	UGUDToC* TableOfContents;

	FGUDPersistentCookerData()
	{
	}

	FGUDPersistentCookerData(const FGUDPersistentCookerData& Src)
	{
		SourceGUDBanks = Src.SourceGUDBanks;
		PawnNameToInfoRequired = Src.PawnNameToInfoRequired;
		ReferencedFaceFXAnimations = Src.ReferencedFaceFXAnimations;
		ReferencedSoundCues = Src.ReferencedSoundCues;
		TableOfContents = Src.TableOfContents;
	}

	/**
	 *	Checks to see if GUDS need to be generated during the cook
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@return	UBOOL			TRUE if they should, FALSE if not
	 */
	UBOOL ShouldBeGenerated(class UCookPackagesCommandlet* Commandlet);

	/**
	 *	Checks to see if GUDS need to be cooked
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	bForceRecook	If TRUE, forcibly recook all guds
	 *	@param	GUDSCookDir		The destination directory for cooked GUDS
	 *	@param	OutGUDSToCook	Array that gets filled in with the GUD files to cook
	 *	@return	UBOOL			TRUE if they should, FALSE if not
	 */
	UBOOL ShouldBeCooked(
		class UCookPackagesCommandlet* Commandlet, 
		UBOOL bForceRecook, 
		FString& GUDSCookDir,
		TArray<FPackageCookerInfo>& OutGUDSToCook);

	/**
	 * Serialize function.
	 *
	 * @param	Ar	Archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar);

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FGUDPersistentCookerData& GUDPCD);
};

/**
 * Gears-specific persistent cooker data object...
 */
class UGearsPersistentCookerData : public UPersistentCookerData
{
	DECLARE_CLASS(UGearsPersistentCookerData, UPersistentCookerData,CLASS_Intrinsic,GearEditor);

	/**
	 * Create an instance of this class given a filename. First try to load from disk and if not found
	 * will construct object and store the filename for later use during saving.
	 *
	 * @param	Filename					Filename to use for serialization
	 * @param	bCreateIfNotFoundOnDisk		If FALSE, don't create if couldn't be found; return NULL.
	 * @return								instance of the container associated with the filename
	 */
	static UPersistentCookerData* CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk );
	static UGearsPersistentCookerData* CreateGearsInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk );
	
	/**
	 * Saves the data to disk.
	 */
	virtual void SaveToDisk();

	/**
	 * Serialize function.
	 *
	 * @param	Ar	Archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar);

	/**
	 *	Generate the table of contents object.\
	 *
	 *	@return	UBOOL		TRUE if successful, FALSE otherwise.
	 */
	UBOOL CreateTableOfContents();

protected:

public:
	FGUDPersistentCookerData	GUDPersistentData;
};

/** 
 *	Helper class for cooking GUDS audio
 */
class FGearGameGUDSCookerHelper
{
protected:
	/** 
	 *	Data structure to store off the information needed to split GUDs into banks.
	 */
	//struct FGearGUDSBankInfo
	//{
	//	/** The name of the GUDData_* class to use as the source. */
	//	FName			GUDSClassName;
	//	/** The FaceFXAnimSet name that goes with it. */
	//	FString			FaceFXAnimSetName;
	//	/** The source line indices to include. */
	//	TArray<INT>		LineIndices;
	//	/** The source line FaceFX animation (will match LineIndices). */
	//	TArray<FString>	LineFaceFXAnimationName;
	//	/** The action line indices to include, remapped to above array. */
	//	TArray<INT>		ActionLineIndices;
	//	/** The action combat line indices to include, remapped to above array. */
	//	TArray<INT>		ActionCombatLineIndices;
	//	/** The action combat line indices to include, remapped to above array. */
	//	TArray<INT>		ActionNonCombatLineIndices;
	//};

	/** The lines used for a given action. */
	struct FGUDSActionLineInfo
	{
		TArray<INT>		LineIndices;
		TArray<INT>		CombatLineIndices;
		TArray<INT>		NonCombatLineIndices;
	};

	/** The information needed to generate a GUDS package. */
	struct FGUDSGeneratedPackageInfo
	{
		/** The name of the package. */
		FString													PackageName;
		/** The source GUD bank. */
		FString													SourceGUDBank;
		/** The source pawn. */
		FString													SourcePawn;
		/** The FaceFXAsset, if present. */
		FString													FaceFXAssetName;
		/** The FaceFXAnimSets, if any. */
		TArray<FString>											FaceFXAnimSetNames;
		/** Whether it is the Root bank or not. */
		UBOOL													bIsRootBank;
		/** If Root bank, then this will have the number of lines to allocate. */
		INT														RootLineCount;
		/** The lines to incude from the source bank. */
		TArray<INT>												LinesIncluded;
		/** The actions supported, and their included lines. */
		TMap<INT, FGUDSActionLineInfo>							ActionMap;		
		/** The FaceFX asset/animset mapped to the animations needed from it. */
		TMap<FString, TArray<FGUDSReferencedFaceFXAnimation>>	FaceFxAnimations;

		FGUDSGeneratedPackageInfo()
		{
			appMemzero(this, sizeof(FGUDSGeneratedPackageInfo));
		}
	};

public:
	FGearGameGUDSCookerHelper():
		MyRandom(516945)
	{
		// EMPTY
	}

	/**
	 * Initialize the cooker helper and process any command line params
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	Tokens			Command line tokens parsed from app
	 *	@param	Switches		Command line switches parsed from app
	 */
	virtual void Init(
		class UCookPackagesCommandlet* Commandlet, 
		const TArray<FString>& Tokens, 
		const TArray<FString>& Switches);

	/** 
	 *	Generate the package list for audio-related files.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	Platform							The platform being cooked for
	 *	@param	ShaderPlatform						The shader platform being cooked for
	 *	@param	NotRequiredFilenamePairs			The package lists being filled in...
	 *	@param	RegularFilenamePairs				""
	 *	@param	MapFilenamePairs					""
	 *	@param	MPMapFilenamePairs					""
	 *	@param	ScriptFilenamePairs					""
	 *	@param	StartupFilenamePairs				""
	 *	@param	StandaloneSeekfreeFilenamePairs		""
	 *	
	 *	@return	UBOOL		TRUE if successfull, FALSE is something went wrong.
	 */
	virtual UBOOL GeneratePackageList( 
		class UCookPackagesCommandlet* Commandlet, 
		UE3::EPlatformType Platform,
		EShaderPlatform ShaderPlatform,
		TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
		TArray<FPackageCookerInfo>& RegularFilenamePairs,
		TArray<FPackageCookerInfo>& MapFilenamePairs,
		TArray<FPackageCookerInfo>& MPMapFilenamePairs,
		TArray<FPackageCookerInfo>& ScriptFilenamePairs,
		TArray<FPackageCookerInfo>& StartupFilenamePairs,
		TArray<FPackageCookerInfo>& StandaloneSeekfreeFilenamePairs);

	/**
	 * Cooks passed in object if it hasn't been already.
	 *
	 *	@param	Commandlet					The cookpackages commandlet being run
	 *	@param	Package						Package going to be saved
	 *	@param	Object						Object to cook
	 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 *
	 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
	 *										FALSE if the object should not be processed any further.
	 */
	virtual UBOOL CookObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage);

	/**
	 * Cooks the sound cue object if it hasn't been already.
	 *
	 *	@param	Commandlet					The cookpackages commandlet being run
	 *	@param	Package						Package going to be saved
	 *	@param	InSoundCue					Soundcue to cook
	 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 *
	 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
	 *										FALSE if the object should not be processed any further.
	 */
	virtual UBOOL CookSoundCue(class UCookPackagesCommandlet* Commandlet, UPackage* Package, USoundCue* InSoundCue, UBOOL bIsSavedInSeekFreePackage);

#if WITH_FACEFX
	/**
	 * Cooks FaceFX* object if it hasn't been already.
	 *
	 *	@param	Commandlet					The cookpackages commandlet being run
	 *	@param	Package						Package going to be saved
	 *	@param	Object						Object to cook
	 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 *
	 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
	 *										FALSE if the object should not be processed any further.
	 */
	virtual UBOOL CookFaceFXObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage);
#endif // WITH_FACEFX

	/** 
	 *	LoadPackageForCookingCallback
	 *	This function will be called in LoadPackageForCooking, allowing the cooker
	 *	helper to handle the package creation as they wish.
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	Filename		The name of the package to load.
	 *
	 *	@return	UPackage*		The package generated/loaded
	 *							NULL if the commandlet should load the package normally.
	 */
	virtual UPackage* LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename);

	/** 
	 *	PostLoadPackageForCookingCallback
	 *	This function will be called in LoadPackageForCooking, prior to any
	 *	operations occurring on the contents...
	 *
	 *	@param	Commandlet	The cookpackages commandlet being run
	 *	@param	Package		The package just loaded.
	 *
	 *	@return	UBOOL		TRUE if the package should be processed further.
	 *						FALSE if the cook of this package should be aborted.
	 */
	virtual UBOOL PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage);

	/**
	 *	Return TRUE if the sound cue should be ignored when generating persistent FaceFX list.
	 *
	 *	@param	Commandlet		The commandlet being run
	 *	@param	InSoundCue		The sound cue of interest
	 *
	 *	@return	UBOOL			TRUE if the sound cue should be ignored, FALSE if not
	 */
	virtual UBOOL ShouldSoundCueBeIgnoredForPersistentFaceFX(class UCookPackagesCommandlet* Commandlet, const USoundCue* InSoundCue);

	/**
	 *	Dump out stats specific to the game cooker helper.
	 */
	virtual void DumpStats();

protected:
	/**
	 *	Generate the GUD bank packages...
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	GearsPCD		The cooker persistent data to fill in...
	 *	
	 *	@return	UBOOL			TRUE if successful.
	 */
	UBOOL GenerateGUDSPackages(
		class UCookPackagesCommandlet* Commandlet, 
		UGearsPersistentCookerData* GearsPCD);

	/**
	 *	Generate the information that is stored in the persistent cooker data.
	 *	This is done to properly cook stuff out, even when not processing GUDS. 
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	GearsPCD		The cooker persistent data to fill in...
	 *	
	 *	@return	UBOOL			TRUE if successful.
	 */
	UBOOL GeneratePersistentCookerData(
		class UCookPackagesCommandlet* Commandlet, 
		UGearsPersistentCookerData* GearsPCD);

	/**
	 *	Fill in the PawnNameToInfoRequired map.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	GearsPCD							The persistent cooker data
	 *	@param	InOutPawnNameToInfoRequired			Where to fill in the info
	 *	
	 *	@return	UBOOL		TRUE if successful.
	 */
	UBOOL FillPawnNameToInfoRequiredMap(
		class UCookPackagesCommandlet* Commandlet, 
		UGearsPersistentCookerData* GearsPCD,
		TMap<FString, FGUDCueAndFaceFX>* InOutPawnNameToInfoRequired
		);

	/** 
	 *	Generate the package information for the GUDS.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	InGUDInfo							The GUD info to generate the packages from
	 *	@param	GearsPCD							The persistent cooker data
	 *	
	 *	@return	UBOOL		TRUE if successful.
	 */
	virtual UBOOL GenerateGUDSPackageInfo( 
		class UCookPackagesCommandlet* Commandlet, 
		FString& PawnName, 
		FGUDCueAndFaceFX& InGUDInfo,
		UGearsPersistentCookerData* GearsPCD);

	/** 
	 *	Generate the package(s) for the GUDS.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	InGUDInfo							The GUD info to generate the packages from
	 *	@param	GearsPCD							The persistent cooker data
	 *	
	 *	@return	UBOOL		TRUE if successful.
	 */
	virtual UBOOL GenerateGUDSPackage( 
		class UCookPackagesCommandlet* Commandlet, 
		FString& PawnName, 
		FGUDCueAndFaceFX& InGUDInfo,
		FString& PackageName,
		UGearsPersistentCookerData* GearsPCD);

	/** 
	 *	Generate the package(s) for the GUDS.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	InGUDInfo							The GUD info to generate the packages from
	 *	@param	GearsPCD							The persistent cooker data
	 *	
	 *	@return	UBOOL		TRUE if successful.
	 */
	UPackage* GenerateGUDSPackage( 
		class UCookPackagesCommandlet* Commandlet, 
		FGUDCueAndFaceFX& InGUDInfo,
		FGUDSGeneratedPackageInfo& GenPkgInfo,
		UGearsPersistentCookerData* GearsPCD);

	/**
	 *	Get the various names that will be generated from the given GUD class.
	 *
	 *	@param	InGUDClass					The GUD class to generate names for.
	 *	@param	OutGeneratedGUDName			The resulting generated GUD name.
	 *	@param	OutGeneratedGUDFaceFXName	The resulting generated FaceFX name.
	 *
	 *	@return	UBOOL						TRUE if successful.
	 */
	virtual UBOOL GenerateGUDNames(
		const UClass* InGUDClass,
		FString& OutGeneratedGUDName,
		FString& OutGeneratedGUDFaceFXName
		);

#if WITH_FACEFX
	/** 
	 *	Move the animation at the given index from the source group to the dest group.
	 *
	 *	@param	InSourceSoundCue	The sound cue that matches the animation.
	 *	@param	InAnimIndex			The index of the animation.
	 *	@param	InAnimName			The name of the animation.
	 *	@param	SrcFFXAnimGroup		The AnimGroup the animation is in.
	 *	@param	DestFFXAnimGroup	The AnimGroup to copy the animation to.
	 *	
	 *	@return	UBOOL		TRUE if successful.
	 */
	UBOOL CopyAnimationToAnimGroup(
		USoundCue* InSourceSoundCue, 
		OC3Ent::Face::FxSize InAnimIndex,
		OC3Ent::Face::FxName InAnimName,
		OC3Ent::Face::FxAnimGroup* SrcFFXAnimGroup,
		OC3Ent::Face::FxAnimGroup* DestFFXAnimGroup);
#endif // WITH_FACEFX

	/**
	 *	Retrieve the referenced animation list for the given object.
	 */
	TGUDSReferencedFaceFXAnimations* GetReferencedAnimationsForObject(class UCookPackagesCommandlet* Commandlet, UObject* InFaceFXObject, UBOOL bCreateIfNotFound = TRUE);

	/**
	 *	Checks to see if GUDS need to be generated during the cook
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	GearsPCD		The UGearsPersistentCookerData instance
	 *	@return	UBOOL			TRUE if they should, FALSE if not
	 */
	UBOOL ShouldBeGenerated(
		class UCookPackagesCommandlet* Commandlet, 
		UGearsPersistentCookerData* GearsPCD);

	/**
	 *	Checks to see if GUDS need to be cooked
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	bForceRecook	If TRUE, forcibly recook all guds
	 *	@param	GUDSCookDir		The destination directory for cooked GUDS
	 *	@param	GearsPCD		The UGearsPersistentCookerData instance
	 *	@param	OutGUDSToCook	Array that gets filled in with the GUD files to cook
	 *	@return	UBOOL			TRUE if they should, FALSE if not
	 */
	UBOOL ShouldBeCooked(
		class UCookPackagesCommandlet* Commandlet, 
		UBOOL bForceRecook, 
		FString& GUDSCookDir,
		UGearsPersistentCookerData* GearsPCD,
		TArray<FPackageCookerInfo>& OutGUDSToCook);

	/**
	 *	Get the generation tracking filename
	 *
	 *	@param	bStart		If TRUE, the start file is requested, otherwise the end one.
	 */
	FString GetGenerationTrackingFilename(UBOOL bStart);

	/**
	 *	Touch the 'start generation' and 'end generation' files, creating them if necessary.
	 *	These are used to determine if GUDS were successfully generated.
	 *
	 *	@param	bStart		If TRUE, the start file is to be touched, otherwise the end one.
	 */
	void TouchGenerationTrackingFile(UBOOL bStart);

	/**
	 *	Get the timestamp for the generation tracking file.
	 *
	 *	@param	bStart		If TRUE, check the start file, otherwise the end one.
	 *	@return	DOUBLE		The timestamp for the file
	 */
	DOUBLE GetGenerationTrackingFileTimestamp(UBOOL bStart);

	/**
	 *	Check the timestamp of the given file vs. the generation tracking file requested.
	 *
	 *	@param	CheckFilename	The filename to check against.
	 *	@param	bStart			If TRUE, check the start file, otherwise the end one.
	 *
	 *	@return	INT				-1 if the given file is older, 1 if it is newer, 0 if they are the same
	 */
	INT CheckFileVersusGenerationTrackingFile(const TCHAR* CheckFilename, UBOOL bStart);

	/**
	 *	Clears out the generated GUDS folder...
	 */
	void ClearGeneratedGUDSFiles();

	// Members...
	/** Our own random number generator */
	FRandomStream MyRandom;
	/** If TRUE then dependency checking is disabled when cooking the GUDS packages */
	UBOOL bForceRecookSeekfreeGUDS;
	/** If TRUE then the guds must be regenerated */
	UBOOL bForceRegenerateGUDS;
	/** If TRUE, then the initialization failed and GUDS should not be cooked. */
	UBOOL bAbortGUDSCooking;
	/** If TRUE, a special GUDs package is being cooked... */
	UBOOL bCookingSpecialGUDSPackage;
	/** If TRUE, when encountering a GUDS sound in non-guds package, list references to it. */
	UBOOL bListGUDSSoundCueReferences;
	/** The package being cooked... */
	FString PackageBeingCooked;
	/** Map of special GUDS package filenames to their 'true' package. */
//	TMap<FString, FString> SpecialGUDSPackages;
	/** GUDBanks that have been processed already. */
	TMap<FString, INT> ProcessedGUDBanks;
	/** GUDBanks that have been processed already. */
	TMap<FString, INT> ProcessedGUDBanks2;
	/** Maps the source filename to the information to build the bank. */
	//TMap<FString, FGearGUDSBankInfo> SrcFilenameToGUDSBankInfoMap;
	/** The source GUD bank mapped to the generation information. */
	TMap<FString, FGUDSGeneratedPackageInfo>	GUDFileNameToGUDSGenerationInfoMap;
	/** The standalone seekfree filename pairs that will be generated. */
//	TArray<FPackageCookerInfo>					GUDStandaloneSeekfreeFilenamePairs;

	/** Package generation settings... */
	INT VarietyLineCount;
	/** If TRUE, generate the root bank (ie only the AlwaysLoad sounds) */
	UBOOL bGenerateRootBankOnly;
	UBOOL bLogGUDSGeneration;

	/** The output directory for generated GUDS files... */
	FString GUDSOutDir;

	/** The TableOfContents object. */
	UGUDToC* GUDToC;
	/** The filename for the ToC. */
	FFilename ToCSrcFilename;
};

/**
 *	Helper class to handle game-specific cooking. Each game can 
 *	subclass this to handle special-case cooking needs.
 */
class FGearGameCookerHelper : public FGameCookerHelper
{
public:

	/**
	 *	Initialize the cooker helpr and process any command line params
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	Tokens			Command line tokens parsed from app
	 *	@param	Switches		Command line switches parsed from app
	 */
	virtual void Init(
		class UCookPackagesCommandlet* Commandlet, 
		const TArray<FString>& Tokens, 
		const TArray<FString>& Switches );

	/**
	 *	Create an instance of the persistent cooker data given a filename. 
	 *	First try to load from disk and if not found will construct object and store the 
	 *	filename for later use during saving.
	 *
	 *	The cooker will call this first, and if it returns NULL, it will use the standard
	 *		UPersistentCookerData::CreateInstance function. 
	 *	(They are static hence the need for this)
	 *
	 * @param	Filename					Filename to use for serialization
	 * @param	bCreateIfNotFoundOnDisk		If FALSE, don't create if couldn't be found; return NULL.
	 * @return								instance of the container associated with the filename
	 */
	virtual UPersistentCookerData* CreateInstance( const TCHAR* Filename, UBOOL bCreateIfNotFoundOnDisk );

	/** 
	 *	Generate the package list that is specific for the game being cooked.
	 *
	 *	@param	Commandlet							The cookpackages commandlet being run
	 *	@param	Platform							The platform being cooked for
	 *	@param	ShaderPlatform						The shader platform being cooked for
	 *	@param	NotRequiredFilenamePairs			The package lists being filled in...
	 *	@param	RegularFilenamePairs				""
	 *	@param	MapFilenamePairs					""
	 *	@param	MPMapFilenamePairs					""
	 *	@param	ScriptFilenamePairs					""
	 *	@param	StartupFilenamePairs				""
	 *	@param	StandaloneSeekfreeFilenamePairs		""
	 *	
	 *	@return	UBOOL		TRUE if successfull, FALSE is something went wrong.
	 */
	virtual UBOOL GeneratePackageList( 
		class UCookPackagesCommandlet* Commandlet, 
		UE3::EPlatformType Platform,
		EShaderPlatform ShaderPlatform,
		TArray<FPackageCookerInfo>& NotRequiredFilenamePairs,
		TArray<FPackageCookerInfo>& RegularFilenamePairs,
		TArray<FPackageCookerInfo>& MapFilenamePairs,
		TArray<FPackageCookerInfo>& MPMapFilenamePairs,
		TArray<FPackageCookerInfo>& ScriptFilenamePairs,
		TArray<FPackageCookerInfo>& StartupFilenamePairs,
		TArray<FPackageCookerInfo>& StandaloneSeekfreeFilenamePairs);

	/**
	 * Cooks passed in object if it hasn't been already.
	 *
	 *	@param	Commandlet					The cookpackages commandlet being run
	 *	@param	Package						Package going to be saved
	 *	@param	Object						Object to cook
	 *	@param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 *
	 *	@return	UBOOL						TRUE if the object should continue the 'normal' cooking operations.
	 *										FALSE if the object should not be processed any further.
	 */
	virtual UBOOL CookObject(class UCookPackagesCommandlet* Commandlet, UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage);

	/** 
	 *	LoadPackageForCookingCallback
	 *	This function will be called in LoadPackageForCooking, allowing the cooker
	 *	helper to handle the package creation as they wish.
	 *
	 *	@param	Commandlet		The cookpackages commandlet being run
	 *	@param	Filename		The name of the package to load.
	 *
	 *	@return	UPackage*		The package generated/loaded
	 *							NULL if the commandlet should load the package normally.
	 */
	virtual UPackage* LoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, const TCHAR* Filename);

	/** 
	 *	PostLoadPackageForCookingCallback
	 *	This function will be called in LoadPackageForCooking, prior to any
	 *	operations occurring on the contents...
	 *
	 *	@param	Commandlet	The cookpackages commandlet being run
	 *	@param	Package		The package just loaded.
	 *
	 *	@return	UBOOL		TRUE if the package should be processed further.
	 *						FALSE if the cook of this package should be aborted.
	 */
	virtual UBOOL PostLoadPackageForCookingCallback(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage);

	/**
	 *	Clean up the kismet for the given level...
	 *	Remove 'danglers' - sequences that don't actually hook up to anything, etc.
	 *
	 *	@param	Commandlet	The cookpackages commandlet being run
	 *	@param	Package		The package being cooked.
	 */
	virtual void CleanupKismet(class UCookPackagesCommandlet* Commandlet, UPackage* InPackage);

	/**
	 *	Return TRUE if the sound cue should be ignored when generating persistent FaceFX list.
	 *
	 *	@param	Commandlet		The commandlet being run
	 *	@param	InSoundCue		The sound cue of interest
	 *
	 *	@return	UBOOL			TRUE if the sound cue should be ignored, FALSE if not
	 */
	virtual UBOOL ShouldSoundCueBeIgnoredForPersistentFaceFX(class UCookPackagesCommandlet* Commandlet, const USoundCue* InSoundCue);

	/**
	 *	Dump out stats specific to the game cooker helper.
	 */
	virtual void DumpStats();

private:
	static class FGenerateGameContentSeekfree* GetGameContentSeekfreeHelper();
	static class FGearGameGUDSCookerHelper* GetGUDSCooker();
};

#endif
