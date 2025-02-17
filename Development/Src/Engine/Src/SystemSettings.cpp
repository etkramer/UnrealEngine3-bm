/*=============================================================================
	ScalabilityOptions.cpp: Unreal engine HW compat scalability system.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "EngineSpeedTreeClasses.h"
#include "EngineDecalClasses.h"
#include "SceneRenderTargets.h"

/*-----------------------------------------------------------------------------
	FSystemSettings
-----------------------------------------------------------------------------*/

/** Global accessor */
FSystemSettings GSystemSettings;

/**
 * Helpers for writing to specific ini sections
 */
static const TCHAR* GIniSectionGame = TEXT("SystemSettings");
static const TCHAR* GIniSectionEditor = TEXT("SystemSettingsEditor");
static inline const TCHAR* GetSectionName(UBOOL bIsEditor)
{
	return bIsEditor ? GIniSectionEditor : GIniSectionGame;
}


/**
 * Helper for ConvertToFriendlySettings
 */
template <class FSystemSettingsDataSubClass>
EFriendlySettingsLevel FindMatchingSystemSetting( const FSystemSettingsData& Settings )
{
	// Search the Defaults in reverse order in case any settings are equal, we'll always return the larger value.
	// Note that identical default values for more than one level is not recommended because it makes makes the
	// mapping of system to friendly settings not one-to-one.
	for (INT IntLevel=FSL_Level5;IntLevel>=FSL_Level1;--IntLevel)
	{
		EFriendlySettingsLevel Level = (EFriendlySettingsLevel)IntLevel;
		const FSystemSettingsDataSubClass& DefaultSettings = GSystemSettings.GetDefaultSettings(Level);
		if (DefaultSettings == (const FSystemSettingsDataSubClass&)Settings)
		{
			return Level;
		}
	}
	return FSL_Custom;
}

/**
 * Uses the default settings mappings to return a friendly setting corresponding to a system setting.
 */
FSystemSettingsFriendly ConvertToFriendlySettings( const FSystemSettingsData& Settings )
{
	FSystemSettingsFriendly Result;

	// Texture Detail
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataTextureDetail>(Settings);
		Result.TextureDetail = Level;
	}

	// World Detail
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataWorldDetail>(Settings);
		Result.WorldDetail = Level;
	}

	// Shadow Detail
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataShadowDetail>(Settings);
		Result.ShadowDetail = Level;
	}

	// VSync
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataVSync>(Settings);
		const FSystemSettingsData* Data = &GSystemSettings;
		if (Level != FSL_Custom)
		{
			Data = &GSystemSettings.GetDefaultSettings(Level);
		}
		Result.bUseVSync = Data->bUseVSync;
	}

	// Screen Percentage
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataScreenPercentage>(Settings);
		const FSystemSettingsData* Data = &GSystemSettings;
		if (Level != FSL_Custom)
		{
			Data = &GSystemSettings.GetDefaultSettings(Level);
		}
		Result.ScreenPercentage = (INT)Data->ScreenPercentage;
		Result.bUpscaleScreenPercentage = Data->bUpscaleScreenPercentage;
	}

	// Resolution
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataResolution>(Settings);
		const FSystemSettingsData* Data = &GSystemSettings;
		if (Level != FSL_Custom)
		{
			Data = &GSystemSettings.GetDefaultSettings(Level);
		}
		Result.ResX = Data->ResX;
		Result.ResY = Data->ResY;
		Result.bFullScreen = Data->bFullscreen;
	}

	// MSAA
	{
		EFriendlySettingsLevel Level = FindMatchingSystemSetting<FSystemSettingsDataMSAA>(Settings);
		const FSystemSettingsData* Data = &GSystemSettings;
		if (Level != FSL_Custom)
		{
			Data = &GSystemSettings.GetDefaultSettings(Level);
		}
		Result.bUseMSAA = Data->MaxMultiSamples > 1;
	}

	return Result;
}

/**
 * Uses the default settings mappings to return a system setting corresponding to a friendly setting.
 * The SourceForCustomSettings parameter specifies how to initialize the data for friendly settings that are "Custom"
 * level. Since there's no direct translation, it just uses the existing settings from the supplied data.
 */
FSystemSettingsData ConvertToSystemSettings(const FSystemSettingsFriendly& FriendlySettings, const FSystemSettingsData& SourceForCustomSettings)
{
	FSystemSettingsData Result = SourceForCustomSettings;

	// Texture Detail
	{
		// Can only set these values if they are not customized!
		if (FriendlySettings.TextureDetail != FSL_Custom)
		{
			(FSystemSettingsDataTextureDetail&)Result = (const FSystemSettingsDataTextureDetail&)GSystemSettings.GetDefaultSettings(FriendlySettings.TextureDetail);
		}
	}

	// World Detail
	{
		// Can only set these values if they are not customized!
		if (FriendlySettings.WorldDetail != FSL_Custom)
		{
			(FSystemSettingsDataWorldDetail&)Result = (const FSystemSettingsDataWorldDetail&)GSystemSettings.GetDefaultSettings(FriendlySettings.WorldDetail);
		}
	}

	// Shadow Detail
	{
		// Can only set these values if they are not customized!
		if (FriendlySettings.ShadowDetail != FSL_Custom)
		{
			(FSystemSettingsDataShadowDetail&)Result = (const FSystemSettingsDataShadowDetail&)GSystemSettings.GetDefaultSettings(FriendlySettings.ShadowDetail);
		}
	}

	// VSync
	{
		Result.bUseVSync = FriendlySettings.bUseVSync;
	}

	// Screen Percentage
	{
		Result.ScreenPercentage = FriendlySettings.ScreenPercentage;
		Result.bUpscaleScreenPercentage = FriendlySettings.bUpscaleScreenPercentage;
	}

	// Resolution
	{
		Result.ResX = FriendlySettings.ResX;
		Result.ResY = FriendlySettings.ResY;
		Result.bFullscreen = FriendlySettings.bFullScreen;
	}

	// MSAA
	{
		Result.MaxMultiSamples = FriendlySettings.bUseMSAA ? 4 : 1;
	}

	return Result;
}


#define PROPERTY_TEXT_TO_ADDRESS_MAPPING											\
	/** Helper struct for all switches/ boolean config values. */					\
	struct { const TCHAR* Name; UBOOL* SwitchPtr; } Switches[] =					\
	{																				\
		{ TEXT("StaticDecals")				, &bAllowStaticDecals },				\
		{ TEXT("DynamicDecals")				, &bAllowDynamicDecals },				\
		{ TEXT("UnbatchedDecals")			, &bAllowUnbatchedDecals },				\
		{ TEXT("DynamicLights")				, &bAllowDynamicLights },				\
		{ TEXT("DynamicShadows")			, &bAllowDynamicShadows },				\
		{ TEXT("LightEnvironmentShadows")	, &bAllowLightEnvironmentShadows },		\
		{ TEXT("CompositeDynamicLights")	, &bUseCompositeDynamicLights },		\
		{ TEXT("DirectionalLightmaps")		, &bAllowDirectionalLightMaps },		\
		{ TEXT("MotionBlur")				, &bAllowMotionBlur },					\
		{ TEXT("DepthOfField")				, &bAllowDepthOfField },				\
		{ TEXT("AmbientOcclusion")			, &bAllowAmbientOcclusion },			\
		{ TEXT("Bloom")						, &bAllowBloom },						\
		{ TEXT("UseHighQualityBloom")		, &bUseHighQualityBloom },				\
		{ TEXT("Distortion")				, &bAllowDistortion },					\
		{ TEXT("DropParticleDistortion")	, &bAllowParticleDistortionDropping },	\
		{ TEXT("SpeedTreeLeaves")			, &bAllowSpeedTreeLeaves },				\
		{ TEXT("SpeedTreeFronds")			, &bAllowSpeedTreeFronds },				\
		{ TEXT("OnlyStreamInTextures")		, &bOnlyStreamInTextures },				\
		{ TEXT("LensFlares")				, &bAllowLensFlares },					\
		{ TEXT("FogVolumes")				, &bAllowFogVolumes },					\
		{ TEXT("FloatingPointRenderTargets"), &bAllowFloatingPointRenderTargets },	\
		{ TEXT("OneFrameThreadLag")			, &bAllowOneFrameThreadLag },			\
		{ TEXT("UseVsync")					, &bUseVSync},							\
		{ TEXT("UpscaleScreenPercentage")	, &bUpscaleScreenPercentage },			\
		{ TEXT("Fullscreen")				, &bFullscreen},						\
		{ TEXT("AllowD3D10")				, &bAllowD3D10 },						\
		{ TEXT("bEnableVSMShadows")			, &bEnableVSMShadows },					\
		{ TEXT("bEnableBranchingPCFShadows"), &bEnableBranchingPCFShadows },		\
		{ TEXT("bAllowBetterModulatedShadows"), &bAllowBetterModulatedShadows },	\
		{ TEXT("bEnableForegroundShadowsOnWorld"), &bEnableForegroundShadowsOnWorld },	\
		{ TEXT("bEnableForegroundSelfShadowing"), &bEnableForegroundSelfShadowing },	\
		{ TEXT("bAllowFracturedDamage")		, &bAllowFracturedDamage },				\
	};																				\
																					\
	/** Helper struct for all integer config values */								\
	struct { const TCHAR* Name; INT* IntValuePtr; } IntValues[] =					\
	{																				\
		{ TEXT("SkeletalMeshLODBias")		, &SkeletalMeshLODBias },				\
		{ TEXT("ParticleLODBias")			, &ParticleLODBias },					\
		{ TEXT("DetailMode")				, &DetailMode },						\
		{ TEXT("ShadowFilterQualityBias")	, &ShadowFilterQualityBias },			\
		{ TEXT("MaxAnisotropy")				, &MaxAnisotropy },						\
		{ TEXT("MaxMultisamples")			, &MaxMultiSamples },					\
		{ TEXT("MinShadowResolution")		, &MinShadowResolution },				\
		{ TEXT("MaxShadowResolution")		, &MaxShadowResolution },				\
		{ TEXT("ResX")						, &ResX },								\
		{ TEXT("ResY")						, &ResY },								\
		{ TEXT("ShadowFadeResolution")		, &ShadowFadeResolution },				\
	};																				\
																					\
	/** Helper struct for all float config values. */								\
	struct { const TCHAR* Name; FLOAT* FloatValuePtr; } FloatValues[] =				\
	{																				\
		{ TEXT("ScreenPercentage")					, &ScreenPercentage },					\
		{ TEXT("SceneCaptureStreamingMultiplier")	, &SceneCaptureStreamingMultiplier },	\
		{ TEXT("FoliageDrawRadiusMultiplier")		, &FoliageDrawRadiusMultiplier },		\
		{ TEXT("ShadowTexelsPerPixel")				, &ShadowTexelsPerPixel },				\
		{ TEXT("ShadowFilterRadius")				, &ShadowFilterRadius },				\
		{ TEXT("ShadowDepthBias")					, &ShadowDepthBias },					\
		{ TEXT("ShadowFadeExponent")				, &ShadowFadeExponent },				\
		{ TEXT("ShadowVolumeLightRadiusThreshold")	, &ShadowVolumeLightRadiusThreshold },	\
		{ TEXT("ShadowVolumePrimitiveScreenSpacePercentageThreshold"), &ShadowVolumePrimitiveScreenSpacePercentageThreshold },	\
		{ TEXT("NumFracturedPartsScale")			, &NumFracturedPartsScale },			\
		{ TEXT("FractureDirectSpawnChanceScale")	, &FractureDirectSpawnChanceScale },	\
		{ TEXT("FractureRadialSpawnChanceScale")	, &FractureRadialSpawnChanceScale },	\
		{ TEXT("FractureCullDistanceScale")			, &FractureCullDistanceScale },			\
		{ TEXT("DecalCullDistanceScale")			, &DecalCullDistanceScale },			\
	};

/**
 * ctor
 */
FSystemSettingsDataWorldDetail::FSystemSettingsDataWorldDetail()
{
}

/**
* ctor
*/
FSystemSettingsDataFracturedDetail::FSystemSettingsDataFracturedDetail()
{
}

/**
* ctor
*/
FSystemSettingsDataShadowDetail::FSystemSettingsDataShadowDetail()
{
}

/**
 * ctor
 */
FSystemSettingsDataTextureDetail::FSystemSettingsDataTextureDetail()
{
}

/**
 * ctor
 */
FSystemSettingsDataVSync::FSystemSettingsDataVSync()
{
}

/**
 * ctor
 */
FSystemSettingsDataScreenPercentage::FSystemSettingsDataScreenPercentage()
{
}

/**
 * ctor
 */
FSystemSettingsDataResolution::FSystemSettingsDataResolution()
{
}

/**
 * ctor
 */
FSystemSettingsDataMSAA::FSystemSettingsDataMSAA()
{
}

/**
 * ctor
 */
FSystemSettingsData::FSystemSettingsData()
{
}

/** Initializes and instance with the values from the given IniSection in the engine ini */
void FSystemSettingsData::LoadFromIni( const TCHAR* IniSection, const TCHAR* IniFilename, UBOOL bAllowMissingValues )
{
	PROPERTY_TEXT_TO_ADDRESS_MAPPING;

	// Read booleans from .ini.
	for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
	{
		const UBOOL bFoundValue = GConfig->GetBool( IniSection, Switches[SwitchIndex].Name, *Switches[SwitchIndex].SwitchPtr, IniFilename );
		checkf(bFoundValue || bAllowMissingValues, TEXT("Couldn't find BOOL system setting %s in Ini section %s in Ini file %s!"), Switches[SwitchIndex].Name, IniSection, IniFilename);
	}
	// Read int values from ini.
	for( INT IntValueIndex=0; IntValueIndex<ARRAY_COUNT(IntValues); IntValueIndex++ )
	{
		const UBOOL bFoundValue = GConfig->GetInt( IniSection, IntValues[IntValueIndex].Name, *IntValues[IntValueIndex].IntValuePtr, IniFilename );
		checkf(bFoundValue || bAllowMissingValues, TEXT("Couldn't find INT system setting %s in Ini section %s in Ini file %s!"), IntValues[IntValueIndex].Name, IniSection, IniFilename);
	}
	// Read float values from .ini.
	for( INT FloatValueIndex=0; FloatValueIndex<ARRAY_COUNT(FloatValues); FloatValueIndex++ )
	{
		const UBOOL bFoundValue = GConfig->GetFloat( IniSection, FloatValues[FloatValueIndex].Name, *FloatValues[FloatValueIndex].FloatValuePtr, IniFilename );
		checkf(bFoundValue || bAllowMissingValues, TEXT("Couldn't find FLOAT system setting %s in Ini section %s in Ini file %s!"), FloatValues[FloatValueIndex].Name, IniSection, IniFilename);
	}

#if CONSOLE
	// Always default to using VSYNC on consoles.
	GSystemSettings.bUseVSync = TRUE;
#endif

	// Disable VSYNC if -novsync is on the command line.
	bUseVSync = bUseVSync && !ParseParam(appCmdLine(), TEXT("novsync"));

	// Enable VSYNC if -vsync is on the command line.
	bUseVSync = bUseVSync || ParseParam(appCmdLine(), TEXT("vsync"));

	// Read the texture group LOD settings.
	TextureLODSettings.Initialize( IniFilename, IniSection );
}

void FSystemSettingsData::SaveToIni( const TCHAR* IniSection )
{
	PROPERTY_TEXT_TO_ADDRESS_MAPPING;

	// Save the BOOLs.
	for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
	{
		GConfig->SetBool( IniSection, Switches[SwitchIndex].Name, *Switches[SwitchIndex].SwitchPtr, GEngineIni );
	}
	// Save the INTs.
	for( INT IntValueIndex=0; IntValueIndex<ARRAY_COUNT(IntValues); IntValueIndex++ )
	{
		GConfig->SetInt( IniSection, IntValues[IntValueIndex].Name, *IntValues[IntValueIndex].IntValuePtr, GEngineIni );
	}
	// Save the FLOATs.
	for( INT FloatValueIndex=0; FloatValueIndex<ARRAY_COUNT(FloatValues); FloatValueIndex++ )
	{
		GConfig->SetFloat( IniSection, FloatValues[FloatValueIndex].Name, *FloatValues[FloatValueIndex].FloatValuePtr, GEngineIni );
	}

	// Save the texture group LOD settings.
	WriteTextureLODGroupsToIni( IniSection );

	GConfig->Flush( FALSE, GEngineIni );
}

/**
 * Writes all texture group LOD settings to the specified ini.
 *
 * @param	IniSection			The .ini section to save to.
 */
void FSystemSettingsData::WriteTextureLODGroupsToIni(const TCHAR* IniSection)
{
	WriteTextureLODGroupToIni( TEXTUREGROUP_World				, TEXT("TEXTUREGROUP_World")				, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_WorldNormalMap		, TEXT("TEXTUREGROUP_WorldNormalMap")		, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_WorldSpecular		, TEXT("TEXTUREGROUP_WorldSpecular")		, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_Character			, TEXT("TEXTUREGROUP_Character")			, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_CharacterNormalMap	, TEXT("TEXTUREGROUP_CharacterNormalMap")	, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_CharacterSpecular	, TEXT("TEXTUREGROUP_CharacterSpecular")	, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_Weapon				, TEXT("TEXTUREGROUP_Weapon")				, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_WeaponNormalMap		, TEXT("TEXTUREGROUP_WeaponNormalMap")		, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_WeaponSpecular		, TEXT("TEXTUREGROUP_WeaponSpecular")		, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_Vehicle				, TEXT("TEXTUREGROUP_Vehicle")				, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_VehicleNormalMap	, TEXT("TEXTUREGROUP_VehicleNormalMap")		, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_VehicleSpecular		, TEXT("TEXTUREGROUP_VehicleSpecular")		, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_Cinematic			, TEXT("TEXTUREGROUP_Cinematic")			, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_Effects				, TEXT("TEXTUREGROUP_Effects")				, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_EffectsNotFiltered	, TEXT("TEXTUREGROUP_EffectsNotFiltered")	, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_Skybox				, TEXT("TEXTUREGROUP_Skybox")				, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_UI					, TEXT("TEXTUREGROUP_UI")					, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_LightAndShadowMap	, TEXT("TEXTUREGROUP_LightAndShadowMap")	, IniSection );
	WriteTextureLODGroupToIni( TEXTUREGROUP_RenderTarget		, TEXT("TEXTUREGROUP_RenderTarget")			, IniSection );
}

/**
 * Writes the specified texture group LOD settings to the specified ini.
 *
 * @param	TextureGroupID		Index/enum of the group to parse
 * @param	GroupName			String representation of the texture group, to be used as the ini key.
 * @param	IniSection			The .ini section to save to.
 */
void FSystemSettingsData::WriteTextureLODGroupToIni(TextureGroup TextureGroupID, const TCHAR* GroupName, const TCHAR* IniSection)
{
	const FExposedTextureLODSettings::FTextureLODGroup& Group = TextureLODSettings.GetTextureLODGroup(TextureGroupID);

	const INT MinLODSize = 1 << Group.MinLODMipCount;
	const INT MaxLODSize = 1 << Group.MaxLODMipCount;

	const FString Entry( FString::Printf( TEXT("(MinLODSize=%i,MaxLODSize=%i,LODBias=%i)"), MinLODSize, MaxLODSize, Group.LODBias ) );
	GConfig->SetString( IniSection, GroupName, *Entry, GEngineIni );
}

/**
 * Dump settings to log
 */
void FSystemSettingsData::Dump( const TCHAR* DumpHeading )
{
	PROPERTY_TEXT_TO_ADDRESS_MAPPING;

	debugf(NAME_Init, DumpHeading);
	debugf(NAME_Init, TEXT("System Settings:"));
	// Save the BOOLs.
	for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
	{
		debugf(NAME_Init, TEXT("\t%s=%s"), Switches[SwitchIndex].Name, *Switches[SwitchIndex].SwitchPtr ? TEXT("true") : TEXT("false"));
	}
	// Save the INTs.
	for( INT IntValueIndex=0; IntValueIndex<ARRAY_COUNT(IntValues); IntValueIndex++ )
	{
		debugf(NAME_Init, TEXT("\t%s=%d"), IntValues[IntValueIndex].Name, *IntValues[IntValueIndex].IntValuePtr );
	}
	// Save the FLOATs.
	for( INT FloatValueIndex=0; FloatValueIndex<ARRAY_COUNT(FloatValues); FloatValueIndex++ )
	{
		debugf(NAME_Init, TEXT("\t%s=%.3f")	, FloatValues[FloatValueIndex].Name, *FloatValues[FloatValueIndex].FloatValuePtr );
	}

	// Save the TextureLODSettings
	DumpTextureLODGroups();

	// write out friendly equivalents (make sure to use current values and not values on disk).
	FSystemSettingsFriendly FriendlySettings = ConvertToFriendlySettings(*this);

	struct ToString
	{
		const TCHAR* operator()(EFriendlySettingsLevel Level) const
		{
			switch(Level)
			{
			case FSL_Level1: return TEXT("Level1");
			case FSL_Level2: return TEXT("Level2");
			case FSL_Level3: return TEXT("Level3");
			case FSL_Level4: return TEXT("Level4");
			case FSL_Level5: return TEXT("Level5");
				// no default
			}
			return TEXT("Custom");
		}
	};

	debugf(NAME_Init, TEXT("Friendly System Settings:"));
	debugf(NAME_Init, TEXT("\tTextureDetail=%s"), ToString()(FriendlySettings.TextureDetail));
	debugf(NAME_Init, TEXT("\tWorldDetail=%s"), ToString()(FriendlySettings.WorldDetail));
	debugf(NAME_Init, TEXT("\tShadowDetail=%s"), ToString()(FriendlySettings.ShadowDetail));
	debugf(NAME_Init, TEXT("\tbUseVSync=%d"), FriendlySettings.bUseVSync);
	debugf(NAME_Init, TEXT("\tbUseMSAA=%d"), FriendlySettings.bUseMSAA);
	debugf(NAME_Init, TEXT("\tScreenPercentage=%3d"), FriendlySettings.ScreenPercentage);
	debugf(NAME_Init, TEXT("\tUpscaleScreenPercentage=%d"), FriendlySettings.bUpscaleScreenPercentage);
	debugf(NAME_Init, TEXT("\tResX=%4d"), FriendlySettings.ResX);
	debugf(NAME_Init, TEXT("\tResY=%4d"), FriendlySettings.ResY);
	debugf(NAME_Init, TEXT("\tFullscreen=%d"), FriendlySettings.bFullScreen);
}

/**
 * Dump helpers
 */
void FSystemSettingsData::DumpTextureLODGroups()
{
	DumpTextureLODGroup( TEXTUREGROUP_World					, TEXT("TEXTUREGROUP_World") );
	DumpTextureLODGroup( TEXTUREGROUP_WorldNormalMap		, TEXT("TEXTUREGROUP_WorldNormalMap") );
	DumpTextureLODGroup( TEXTUREGROUP_WorldSpecular			, TEXT("TEXTUREGROUP_WorldSpecular") );
	DumpTextureLODGroup( TEXTUREGROUP_Character				, TEXT("TEXTUREGROUP_Character") );
	DumpTextureLODGroup( TEXTUREGROUP_CharacterNormalMap	, TEXT("TEXTUREGROUP_CharacterNormalMap") );
	DumpTextureLODGroup( TEXTUREGROUP_CharacterSpecular		, TEXT("TEXTUREGROUP_CharacterSpecular") );
	DumpTextureLODGroup( TEXTUREGROUP_Weapon				, TEXT("TEXTUREGROUP_Weapon") );
	DumpTextureLODGroup( TEXTUREGROUP_WeaponNormalMap		, TEXT("TEXTUREGROUP_WeaponNormalMap") );
	DumpTextureLODGroup( TEXTUREGROUP_WeaponSpecular		, TEXT("TEXTUREGROUP_WeaponSpecular") );
	DumpTextureLODGroup( TEXTUREGROUP_Vehicle				, TEXT("TEXTUREGROUP_Vehicle") );
	DumpTextureLODGroup( TEXTUREGROUP_VehicleNormalMap		, TEXT("TEXTUREGROUP_VehicleNormalMap") );
	DumpTextureLODGroup( TEXTUREGROUP_VehicleSpecular		, TEXT("TEXTUREGROUP_VehicleSpecular") );
	DumpTextureLODGroup( TEXTUREGROUP_Cinematic				, TEXT("TEXTUREGROUP_Cinematic") );
	DumpTextureLODGroup( TEXTUREGROUP_Effects				, TEXT("TEXTUREGROUP_Effects") );
	DumpTextureLODGroup( TEXTUREGROUP_EffectsNotFiltered	, TEXT("TEXTUREGROUP_EffectsNotFiltered") );
	DumpTextureLODGroup( TEXTUREGROUP_Skybox				, TEXT("TEXTUREGROUP_Skybox") );
	DumpTextureLODGroup( TEXTUREGROUP_UI					, TEXT("TEXTUREGROUP_UI") );
	DumpTextureLODGroup( TEXTUREGROUP_LightAndShadowMap		, TEXT("TEXTUREGROUP_LightAndShadowMap") );
	DumpTextureLODGroup( TEXTUREGROUP_RenderTarget			, TEXT("TEXTUREGROUP_RenderTarget") );
}

/**
 * Dump helpers
 */
void FSystemSettingsData::DumpTextureLODGroup(TextureGroup TextureGroupID, const TCHAR* GroupName)
{
	const FExposedTextureLODSettings::FTextureLODGroup& Group = TextureLODSettings.GetTextureLODGroup(TextureGroupID);

	const INT MinLODSize = 1 << Group.MinLODMipCount;
	const INT MaxLODSize = 1 << Group.MaxLODMipCount;

	debugf(TEXT("\t%s: (MinLODSize=%4i,MaxLODSize=%4i,LODBias=%1i)"), GroupName, MinLODSize, MaxLODSize, Group.LODBias );
}

/**
 * Constructor, initializing all member variables.
 */
FSystemSettings::FSystemSettings() :
	bIsEditor( FALSE ),
	CurrentSplitScreenLevel(SPLITSCREENLEVEL_1)
{
	// there should only ever be one of these
	static INT InstanceCount = 0;
	++InstanceCount;
	check(InstanceCount == 1);

	// Renderthread state
	RenderThreadSettings.bAllowMotionBlur		= bAllowMotionBlur;
	RenderThreadSettings.bAllowAmbientOcclusion	= bAllowAmbientOcclusion;
	RenderThreadSettings.bAllowDynamicShadows	= bAllowDynamicShadows;
	RenderThreadSettings.bAllowFogVolumes		= bAllowFogVolumes;
	RenderThreadSettings.MaxMultiSamples		= MaxMultiSamples;
	RenderThreadSettings.MinShadowResolution	= MinShadowResolution;
	RenderThreadSettings.MaxShadowResolution	= MaxShadowResolution;
	RenderThreadSettings.bAllowUnbatchedDecals	= bAllowUnbatchedDecals;
}

/**
 * Initializes system settings and included texture LOD settings.
 *
 * @param bSetupForEditor	Whether to initialize settings for Editor
 */
void FSystemSettings::Initialize( UBOOL bSetupForEditor )
{
	// Since System Settings is called into before GIsEditor is set, we must cache this value.
	bIsEditor = bSetupForEditor;

	// Load the settings that will be the default for every other compat level, and the editor, and the other split screen levels.
	FSystemSettingsData DefaultSettings;
	DefaultSettings.LoadFromIni(GetSectionName(FALSE), GEngineIni, FALSE);

	// read the compat level settings from the ini
	for (INT Level=0;Level<ARRAY_COUNT(Defaults);++Level)
	{
		Defaults[Level][0] = DefaultSettings;
		FString Section = FString::Printf(TEXT("AppCompatBucket%d"), Level+1);
		// use global SystemSettings if default sections don't exist.
		if (GConfig->GetSectionPrivate(*Section, FALSE, TRUE, GCompatIni) != NULL)
		{
			Defaults[Level][0].LoadFromIni(*Section, GCompatIni);
		}
		else
		{
			Defaults[Level][0].LoadFromIni(GetSectionName(bIsEditor));
		}
	}

	// Initialize all split screen level settings to the default split screen level (SPLITSCREENLEVEL_1)
	for (INT Level=0;Level<ARRAY_COUNT(Defaults);++Level)
	{
		for (INT SSLevel=1;SSLevel<ARRAY_COUNT(Defaults[0]);++SSLevel)
		{
			FString SSSection = FString::Printf(TEXT("SystemSettingsSplitScreen%d"), SSLevel+1);
			Defaults[Level][SSLevel] = DefaultSettings;
			// Override defaults with the subset of settings that are actually specified for this split screen level
			Defaults[Level][SSLevel].LoadFromIni(bIsEditor ? GIniSectionEditor : *SSSection);
		}
	}

	(FSystemSettingsData&)(*this) = DefaultSettings;
	LoadFromIni();

#if CONSOLE
	// Overwrite resolution from Ini with resolution from the console RHI
	ResX = GScreenWidth;
	ResY = GScreenHeight;
#else
	// Dump(TEXT("Startup System Settings:"));
#endif

	// Apply settings to render thread values.
	ApplySystemSettingsToRenderThread();
}

/**
 * Exec handler implementation.
 *
 * @param Cmd	Command to parse
 * @param Ar	Output device to log to
 *
 * @return TRUE if command was handled, FALSE otherwise
 */
UBOOL FSystemSettings::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	PROPERTY_TEXT_TO_ADDRESS_MAPPING;

	// Keep track of previous detail mode and lightmap type.
	const INT OldDetailMode = DetailMode;
	const UBOOL bOldAllowDirectionalLightMaps = bAllowDirectionalLightMaps;
	const UBOOL bOldAllowDynamicDecals = bAllowDynamicDecals;
	const FLOAT OldDecalCullDistanceScale = DecalCullDistanceScale;

	// Keep track whether the command was handled or not.
	UBOOL bHandledCommand = FALSE;

	if( ParseCommand(&Cmd,TEXT("SCALE")) )
	{
		// Some of these settings are used in both threads so we need to stop the rendering thread before changing them.
		FlushRenderingCommands();

		if( ParseCommand(&Cmd,TEXT("DUMP")) )
		{
			Dump(TEXT("System settings Dump:"));
			return TRUE;
		}
		if( ParseCommand(&Cmd,TEXT("DUMPINI")) )
		{
			FSystemSettingsData IniData;
			IniData.LoadFromIni(GetSectionName(bIsEditor));
			IniData.Dump(TEXT("System settings Dump (from ini):"));
			return TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("LEVEL")) )
		{
			FString Str(ParseToken(Cmd, 0));
			if (Str.Len() > 0)
			{
				INT Level = appAtoi(*Str);
				if (Level >= 1 && Level <= 5)
				{
					// don't put us into fullscreen if we're not already in it.
					FSystemSettingsData NewData = GetDefaultSettings(EFriendlySettingsLevel(Level));
					NewData.bFullscreen = bFullscreen && NewData.bFullscreen;
					ApplyNewSettings(NewData, FALSE); 
					Ar.Logf(TEXT("Applying Default system settings for compat level: %d"), Level);
				}
			}
			return TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("LOWEND")) )
		{
			bAllowStaticDecals				= FALSE;
			bAllowDynamicDecals				= FALSE;
			bAllowUnbatchedDecals			= TRUE;
			DecalCullDistanceScale			= 1.0f;
			bAllowDynamicLights				= FALSE;
			bAllowDynamicShadows			= FALSE;
			bAllowLightEnvironmentShadows	= FALSE;
			bUseCompositeDynamicLights		= TRUE;
			//bAllowDirectionalLightMaps		= FALSE;		// Can't be toggled at run-time
			bAllowMotionBlur				= FALSE;
			bAllowDepthOfField				= TRUE;
			bAllowAmbientOcclusion			= FALSE;
			bAllowBloom						= TRUE;
			bUseHighQualityBloom			= FALSE;
			bAllowDistortion				= FALSE;
			bAllowParticleDistortionDropping= TRUE;
			bAllowSpeedTreeLeaves			= FALSE;
			bAllowSpeedTreeFronds			= FALSE;
			bOnlyStreamInTextures			= FALSE;
			bAllowLensFlares				= FALSE;
			bAllowFogVolumes				= FALSE;
			bAllowFloatingPointRenderTargets= FALSE;
			//bAllowOneFrameThreadLag			= TRUE;			// Only toggle explicitly
			bUseVSync						= FALSE;
			bUpscaleScreenPercentage		= TRUE;
			SkeletalMeshLODBias				= 1;
			ParticleLODBias					= INT_MAX;
			DetailMode						= DM_Low;
			ShadowFilterQualityBias			= -1;
			MaxAnisotropy					= 0;
			ScreenPercentage				= 100.f;
			SceneCaptureStreamingMultiplier	= 0.5f;
			FoliageDrawRadiusMultiplier		= 0.0f;
			MaxMultiSamples					= 1;
			MinShadowResolution				= 32;
			MaxShadowResolution				= 512;
			ShadowTexelsPerPixel			= 2.0f;
			bAllowFracturedDamage			= FALSE;
			NumFracturedPartsScale			= 0;
			FractureDirectSpawnChanceScale	= 0;
			FractureRadialSpawnChanceScale	= 0;
			FractureCullDistanceScale		= 0.25f;

			bHandledCommand					= TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("HIGHEND")) )
		{
			bAllowStaticDecals				= TRUE;
			bAllowDynamicDecals				= TRUE;
			bAllowUnbatchedDecals			= TRUE;
			DecalCullDistanceScale			= 1.0f;
			bAllowDynamicLights				= TRUE;
			bAllowDynamicShadows			= TRUE;
			bAllowLightEnvironmentShadows	= TRUE;
			bUseCompositeDynamicLights		= FALSE;
			//bAllowDirectionalLightMaps		= TRUE;			// Can't be toggled at run-time
			bAllowMotionBlur				= TRUE;
			bAllowDepthOfField				= TRUE;
			bAllowAmbientOcclusion			= TRUE;
			bAllowBloom						= TRUE;
			bUseHighQualityBloom			= TRUE;
			bAllowDistortion				= TRUE;
			bAllowParticleDistortionDropping= FALSE;
			bAllowSpeedTreeLeaves			= TRUE;
			bAllowSpeedTreeFronds			= TRUE;
			bOnlyStreamInTextures			= FALSE;
			bAllowLensFlares				= TRUE;
			bAllowFogVolumes				= TRUE;
			bAllowFloatingPointRenderTargets= TRUE;
			//bAllowOneFrameThreadLag			= TRUE;			// Only toggle explicitly
			bUseVSync						= TRUE;
			bUpscaleScreenPercentage		= TRUE;
			SkeletalMeshLODBias				= 0;
			ParticleLODBias					= 0;
			DetailMode						= DM_High;
			ShadowFilterQualityBias			= INT_MAX;
			MaxAnisotropy					= 16;
			ScreenPercentage				= 100.f;
			SceneCaptureStreamingMultiplier	= 1.0f;
			FoliageDrawRadiusMultiplier		= 1.0f;
			MaxMultiSamples					= 4;
			MinShadowResolution				= 32;
			MaxShadowResolution				= 512;
			ShadowTexelsPerPixel			= 2.0f;
			bAllowFracturedDamage			= TRUE;
			NumFracturedPartsScale			= 1;
			FractureDirectSpawnChanceScale	= 1;
			FractureRadialSpawnChanceScale	= 1;
			FractureCullDistanceScale		= 1;

			bHandledCommand					= TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("RESET")) )
		{
			// Reset values to defaults from ini.
			LoadFromIni();
			bHandledCommand = TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("SET")) )
		{
			// Search for a specific boolean
			for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
			{
				if( ParseCommand(&Cmd,Switches[SwitchIndex].Name) )
				{
					UBOOL bNewValue = ParseCommand(&Cmd,TEXT("TRUE"));
					*Switches[SwitchIndex].SwitchPtr = bNewValue;
					Ar.Logf(TEXT("Switch %s set to %u"), Switches[SwitchIndex].Name, bNewValue);
					bHandledCommand	= TRUE;

					debugf(TEXT("%s %s"),Switches[SwitchIndex].Name,bNewValue ? TEXT("TRUE") : TEXT("FALSE"));
				}
			}

			// Search for a specific int value.
			for( INT IntValueIndex=0; IntValueIndex<ARRAY_COUNT(IntValues); IntValueIndex++ )
			{
				if( ParseCommand(&Cmd,IntValues[IntValueIndex].Name) )
				{
					INT NewValue = appAtoi( Cmd );
					*IntValues[IntValueIndex].IntValuePtr = NewValue;
					Ar.Logf(TEXT("Int %s set to %u"), IntValues[IntValueIndex].Name, NewValue);
					bHandledCommand	= TRUE;

					debugf(TEXT("%s %d"),IntValues[IntValueIndex].Name,NewValue);
				}
			}

			// Search for a specific float value.
			for( INT FloatValueIndex=0; FloatValueIndex<ARRAY_COUNT(FloatValues); FloatValueIndex++ )
			{
				if( ParseCommand(&Cmd,FloatValues[FloatValueIndex].Name) )
				{
					FLOAT NewValue = appAtof( Cmd );
					*FloatValues[FloatValueIndex].FloatValuePtr = NewValue;
					Ar.Logf(TEXT("Float %s set to %f"), FloatValues[FloatValueIndex].Name, NewValue);
					bHandledCommand	= TRUE;

					debugf(TEXT("%s %0.3f"),FloatValues[FloatValueIndex].Name,NewValue);
				}
			}
		}
		else if( ParseCommand(&Cmd,TEXT("TOGGLE")) )
		{
			// Search for a specific boolean
			for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
			{
				if( ParseCommand(&Cmd,Switches[SwitchIndex].Name) )
				{
					*Switches[SwitchIndex].SwitchPtr = !*Switches[SwitchIndex].SwitchPtr;
					Ar.Logf(TEXT("Switch %s toggled, new value %u"), Switches[SwitchIndex].Name, *Switches[SwitchIndex].SwitchPtr);
					bHandledCommand	= TRUE;
				}
			}
		}

		if (!bHandledCommand)
		{
			Ar.Logf(TEXT("Unrecognized system setting"));
		}

		// Write the new settings to the INI.
		SaveToIni();
	}

	// We don't support switching lightmap type at run-time.
	if( bAllowDirectionalLightMaps != bOldAllowDirectionalLightMaps )
	{
		debugf(TEXT("Can't change lightmap type at run-time."));
		bAllowDirectionalLightMaps = bOldAllowDirectionalLightMaps;
	}

	// Reattach components if world-detail settings have changed.
	if( OldDetailMode != DetailMode )
	{
		// decals should always reattach after all other primitives have been attached
		TArray<UClass*> ExcludeComponents;
		ExcludeComponents.AddItem(UDecalComponent::StaticClass());
		ExcludeComponents.AddItem(UAudioComponent::StaticClass());

		FGlobalComponentReattachContext PropagateDetailModeChanges(ExcludeComponents);
	}	
	// Reattach decal components if needed 
	if( OldDetailMode != DetailMode )
	{
		TComponentReattachContext<UDecalComponent> PropagateDecalComponentChanges;
	}

	// Activate certain system settings on the renderthread
	GSystemSettings.ApplySystemSettingsToRenderThread();

	// Dump settings to the log so we know what values are being used.
	if (bHandledCommand)
	{
#if !CONSOLE
		Dump(TEXT("System settings changed by exec command:"));
#endif
	}

	return bHandledCommand;
}


/**
 * Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
 *
 * @param X - in/out X screen offset
 * @param Y - in/out Y screen offset
 * @param SizeX - in/out X screen size
 * @param SizeY - in/out Y screen size
 */
void FSystemSettings::ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY )
{
	// Take screen percentage option into account if percentage != 100.
	if( GSystemSettings.ScreenPercentage != 100.f && !bIsEditor )
	{
		// Clamp screen percentage to reasonable range.
		FLOAT ScaleFactor = Clamp( GSystemSettings.ScreenPercentage / 100.f, 0.0f, 1.f );

		INT	OrigX = X;
		INT OrigY = Y;
		UINT OrigSizeX = SizeX;
		UINT OrigSizeY = SizeY;

		// Scale though make sure we're at least covering 1 pixel.
		SizeX = Max(1,appTrunc(ScaleFactor * OrigSizeX));
		SizeY = Max(1,appTrunc(ScaleFactor * OrigSizeY));

		// Center scaled view.
		X = OrigX + (OrigSizeX - SizeX) / 2;
		Y = OrigY + (OrigSizeY - SizeY) / 2;
	}
}

/**
 * Reverses the scale and offset done by ScaleScreenCoords() 
 * if the screen percentage is not 100% and upscaling is allowed.
 *
 * @param OriginalX - out X screen offset
 * @param OriginalY - out Y screen offset
 * @param OriginalSizeX - out X screen size
 * @param OriginalSizeY - out Y screen size
 * @param InX - in X screen offset
 * @param InY - in Y screen offset
 * @param InSizeX - in X screen size
 * @param InSizeY - in Y screen size
 */
void FSystemSettings::UnScaleScreenCoords( 
	INT &OriginalX, INT &OriginalY, 
	UINT &OriginalSizeX, UINT &OriginalSizeY, 
	FLOAT InX, FLOAT InY, 
	FLOAT InSizeX, FLOAT InSizeY)
{
	if (NeedsUpscale())
	{
		FLOAT ScaleFactor = Clamp( GSystemSettings.ScreenPercentage / 100.f, 0.0f, 1.f );

		//undo scaling
		OriginalSizeX = appTrunc(InSizeX / ScaleFactor);
		OriginalSizeY = appTrunc(InSizeY / ScaleFactor);

		//undo centering
		OriginalX = appTrunc(InX - (OriginalSizeX - InSizeX) / 2.0f);
		OriginalY = appTrunc(InY - (OriginalSizeY - InSizeY) / 2.0f);
	}
	else
	{
		OriginalSizeX = appTrunc(InSizeX);
		OriginalSizeY = appTrunc(InSizeY);

		OriginalX = appTrunc(InX);
		OriginalY = appTrunc(InY);
	}
}

/** Indicates whether upscaling is needed */
UBOOL FSystemSettings::NeedsUpscale() const
{
	return bUpscaleScreenPercentage && ScreenPercentage < 100.0f && !bIsEditor;
}

/**
 * Reads a single entry and parses it into the group array.
 *
 * @param	TextureGroupID		Index/enum of group to parse
 * @param	MinLODSize			Minimum size, in pixels, below which the code won't bias.
 * @param	MaxLODSize			Maximum size, in pixels, above which the code won't bias.
 * @param	LODBias				Group LOD bias.
 */
void FSystemSettings::SetTextureLODGroup(TextureGroup TextureGroupID, int MinLODSize, INT MaxLODSize, INT LODBias)
{
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).MinLODMipCount	= appCeilLogTwo( MinLODSize );
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).MaxLODMipCount	= appCeilLogTwo( MaxLODSize );
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).LODBias			= LODBias;
}

/**
* Recreates texture resources and drops mips.
*
* @return		TRUE if the settings were applied, FALSE if they couldn't be applied immediately.
*/
UBOOL FSystemSettings::UpdateTextureStreaming()
{
	if ( GStreamingManager )
	{
		// Make sure textures can be streamed out so that we can unload current mips.
		const UBOOL bOldOnlyStreamInTextures = bOnlyStreamInTextures;
		bOnlyStreamInTextures = FALSE;

		for( TObjectIterator<UTexture2D> It; It; ++It )
		{
			UTexture* Texture = *It;

			// Update cached LOD bias.
			Texture->CachedCombinedLODBias = TextureLODSettings.CalculateLODBias( Texture );
		}

		// Make sure we iterate over all textures by setting it to high value.
		GStreamingManager->SetNumIterationsForNextFrame( 100 );
		// Update resource streaming with updated texture LOD bias/ max texture mip count.
		GStreamingManager->UpdateResourceStreaming( 0 );
		// Block till requests are finished.
		GStreamingManager->BlockTillAllRequestsFinished();

		// Restore streaming out of textures.
		bOnlyStreamInTextures = bOldOnlyStreamInTextures;
	}

	return TRUE;
}

void FSystemSettings::ApplySystemSettings_RenderingThread(const FSystemSettings::FRenderThreadSettings& NewSettings)
{
	// Should we create or release certain rendertargets?
	const UBOOL bUpdateRenderTargets =
		(GSystemSettings.RenderThreadSettings.bAllowMotionBlur		!= NewSettings.bAllowMotionBlur) ||
		(GSystemSettings.RenderThreadSettings.bAllowAmbientOcclusion!= NewSettings.bAllowAmbientOcclusion) ||
		(GSystemSettings.RenderThreadSettings.bAllowDynamicShadows	!= NewSettings.bAllowDynamicShadows) ||
		(GSystemSettings.RenderThreadSettings.bAllowFogVolumes		!= NewSettings.bAllowFogVolumes) ||
		(GSystemSettings.RenderThreadSettings.MaxMultiSamples		!= NewSettings.MaxMultiSamples) ||
		(GSystemSettings.RenderThreadSettings.MinShadowResolution	!= NewSettings.MinShadowResolution) ||
		(GSystemSettings.RenderThreadSettings.MaxShadowResolution	!= NewSettings.MaxShadowResolution);

	GSystemSettings.RenderThreadSettings.bAllowMotionBlur		= NewSettings.bAllowMotionBlur;
	GSystemSettings.RenderThreadSettings.bAllowAmbientOcclusion	= NewSettings.bAllowAmbientOcclusion;
	GSystemSettings.RenderThreadSettings.bAllowDynamicShadows	= NewSettings.bAllowDynamicShadows;
	GSystemSettings.RenderThreadSettings.bAllowFogVolumes		= NewSettings.bAllowFogVolumes;
	GSystemSettings.RenderThreadSettings.MaxMultiSamples		= NewSettings.MaxMultiSamples;
	GSystemSettings.RenderThreadSettings.MinShadowResolution	= NewSettings.MinShadowResolution;
	GSystemSettings.RenderThreadSettings.MaxShadowResolution	= NewSettings.MaxShadowResolution;
	GSystemSettings.RenderThreadSettings.bAllowUnbatchedDecals	= NewSettings.bAllowUnbatchedDecals;

// Shouldn't be reallocating render targets on console
#if !CONSOLE
	if(bUpdateRenderTargets)
	{
		GSceneRenderTargets.UpdateRHI();
	}
#endif
}

/** Makes System Settings take effect on the renderthread */
void FSystemSettings::ApplySystemSettingsToRenderThread()
{
	FSystemSettings::FRenderThreadSettings NewSettings;
	NewSettings.bAllowMotionBlur		= bAllowMotionBlur;
	NewSettings.bAllowAmbientOcclusion	= bAllowAmbientOcclusion;
	NewSettings.bAllowDynamicShadows	= bAllowDynamicShadows;
	NewSettings.bAllowFogVolumes		= bAllowFogVolumes;
	NewSettings.MaxMultiSamples			= MaxMultiSamples;
	NewSettings.MinShadowResolution		= MinShadowResolution;
	NewSettings.MaxShadowResolution		= MaxShadowResolution;
	NewSettings.bAllowUnbatchedDecals	= bAllowUnbatchedDecals;

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ActivateSystemSettings,
		FSystemSettings*,SystemSettings,this,
		FSystemSettings::FRenderThreadSettings,NewSettings,NewSettings,
	{
		SystemSettings->ApplySystemSettings_RenderingThread(NewSettings);
	});
}

/*-----------------------------------------------------------------------------
	Resolution.
-----------------------------------------------------------------------------*/

/** 
 * Sets the resolution and writes the values to Ini if changed but does not apply the changes (eg resize the viewport).
 */
void FSystemSettings::SetResolution(INT InSizeX, INT InSizeY, UBOOL InFullscreen)
{
	if (!bIsEditor)
	{
		const UBOOL bResolutionChanged = 
			ResX != InSizeX ||
			ResY != InSizeY ||
			bFullscreen != InFullscreen;

		if (bResolutionChanged)
		{
			ResX = InSizeX;
			ResY = InSizeY;
			bFullscreen = InFullscreen;
			SaveToIni();
		}
	}
}

/**
 * Overriden function that selects the proper ini section to write to
 */
void FSystemSettings::LoadFromIni()
{
	FSystemSettingsData::LoadFromIni(GetSectionName(bIsEditor));
}

void FSystemSettings::SaveToIni()
{
	// don't write changes in the editor
	if (bIsEditor)
	{
		debugf(TEXT("Can't save system settings to ini in an editor mode"));
		return;
	}
	FSystemSettingsData::SaveToIni(GetSectionName(bIsEditor));
}

/**
 * Returns the current friendly settings as defined by the current system settings.
 *
 * Normally we read from the ini because some settings are deferred until the next reboot
 * so are only in the ini.
 */
FSystemSettingsFriendly FSystemSettings::ConvertToFriendlySettings( bool bReadSettingsFromIni /*= TRUE*/ )
{
	if (bReadSettingsFromIni)
	{
		// use the values in the ini file
		FSystemSettingsData IniData;
		IniData.LoadFromIni(GetSectionName(bIsEditor));
		return ::ConvertToFriendlySettings(IniData);
	}

	return ::ConvertToFriendlySettings(*this);
}

/**
 * applies the system settings using the given friendly settings
 */
void FSystemSettings::ApplyFriendlySettings( const FSystemSettingsFriendly& FriendlySettings, UBOOL bWriteToIni )
{
	ApplyNewSettings(ConvertToSystemSettings(FriendlySettings, *this), bWriteToIni);
}

/** 
 * Ensures that the correct settings are being used based on split screen type.
 */
void FSystemSettings::UpdateSplitScreenSettings(ESplitScreenType NewSplitScreenType)
{
	ESplitScreenLevel NewLevel = NewSplitScreenType == eSST_NONE ? SPLITSCREENLEVEL_1 : SPLITSCREENLEVEL_2;
	if (NewLevel != CurrentSplitScreenLevel)
	{
		CurrentSplitScreenLevel = NewLevel;
		//@todo - Don't assume level 5 is the current settings
		FSystemSettingsData NewData = GetDefaultSettings(FSL_Level5);
		if( GEngine && GEngine->GameViewport )
		{
			GEngine->GameViewport->OverrideSplitscreenSettings(NewData,NewSplitScreenType);
		}

		ApplyNewSettings(NewData, FALSE); 
	}
}

const FSystemSettingsData& FSystemSettings::GetDefaultSettings( EFriendlySettingsLevel Level )
{
	check(Level>=FSL_Level1 && Level<=FSL_Level5);
	return Defaults[Level-1][CurrentSplitScreenLevel];
}

/**
 * Helper for ApplyNewSettings when the engine is running. Applies the changes needed for the runtime system.
 *
 * We can assume the game is running if this code is called.
 */
void FSystemSettings::ApplySettingsAtRuntime(const FSystemSettingsData& NewSettings, UBOOL bWriteToIni)
{
	// Some of these settings are shared between threads, so we
	// must flush the rendering thread before changing anything.
	FlushRenderingCommands();

	// Track settings we might have to put back
	FExposedTextureLODSettings InMemoryTextureLODSettings = TextureLODSettings;

	// Read settings from .ini.  This is necessary because settings which need to wait for a restart
	// will be on disk but may not be in memory.  Therefore, we read from disk before capturing old
	// values to revert to.
	LoadFromIni();

	// see what settings are actually changing.
	// Ugly casts because system settings is multi-inherited from all the consituent classes for backwards compatibility

	// Texture Detail
	UBOOL bTextureDetailChanged = (const FSystemSettingsDataTextureDetail&)(*this) != (const FSystemSettingsDataTextureDetail&)NewSettings;
	// World Detail
	UBOOL bWorldDetailChanged = (const FSystemSettingsDataWorldDetail&)(*this) != (const FSystemSettingsDataWorldDetail&)NewSettings;
	// Shadow Detail
	UBOOL bShadowDetailChanged = (const FSystemSettingsDataShadowDetail&)(*this) != (const FSystemSettingsDataShadowDetail&)NewSettings;
	// VSync
	UBOOL bVSyncChanged = (const FSystemSettingsDataVSync&)(*this) != (const FSystemSettingsDataVSync&)NewSettings;
	// Screen Percentage
	UBOOL bScreenPercentageChanged = (const FSystemSettingsDataScreenPercentage&)(*this) != (const FSystemSettingsDataScreenPercentage&)NewSettings;
	// Resolution
	UBOOL bResolutionChanged = (const FSystemSettingsDataResolution&)(*this) != (const FSystemSettingsDataResolution&)NewSettings;
	// MSAA
	UBOOL bMSAAChanged = (const FSystemSettingsDataMSAA&)(*this) != (const FSystemSettingsDataMSAA&)NewSettings;
	// special case changes we need to look out for
	UBOOL bDetailModeChanged = DetailMode != NewSettings.DetailMode;
	UBOOL bAllowDirectionalLightMapsChanged = bAllowDirectionalLightMaps != NewSettings.bAllowDirectionalLightMaps;
	// change in decal detail mode
	const UBOOL bDecalCullDistanceScaleChanged = DecalCullDistanceScale != NewSettings.DecalCullDistanceScale;
	// change in mode for dynamic path for decals
	const UBOOL bAllowUnbatchedDecalsChanged = bAllowUnbatchedDecals != NewSettings.bAllowUnbatchedDecals;

	// Set new settings. Would look prettier if we didn't derive from the Data class...
	(FSystemSettingsData&)(*this) = NewSettings;

	// apply any runtime changes that need to be made
	UBOOL bUpdateTextureStreamingSucceeded = FALSE;
	if (bTextureDetailChanged)
	{
		bUpdateTextureStreamingSucceeded = GSystemSettings.UpdateTextureStreaming();
	}

	if (bResolutionChanged)
	{
		if ( GEngine
			&& GEngine->GameViewport
			&& GEngine->GameViewport->ViewportFrame )
		{
			GEngine->GameViewport->ViewportFrame->Resize(ResX, ResY, bFullscreen);
		}
	}

	// If requested, save the settings to ini.
	if ( bWriteToIni )
	{
		SaveToIni();
	}

	// We don't support switching lightmap type at run-time.
	if( bAllowDirectionalLightMapsChanged )
	{
		debugf(TEXT("Can't change lightmap type at run-time."));
		bAllowDirectionalLightMaps = !bAllowDirectionalLightMaps;
	}

	// If texture detail settings couldn't be applied because we're loading seekfree,
	// revert the new settings to their previous in-memory values.
	if ( bTextureDetailChanged && !bUpdateTextureStreamingSucceeded )
	{
		TextureLODSettings = InMemoryTextureLODSettings;
	}

	// Reattach components if world-detail settings have changed.
	if( bDetailModeChanged )
	{
		// decals should always reattach after all other primitives have been attached
		TArray<UClass*> ExcludeComponents;
		ExcludeComponents.AddItem(UDecalComponent::StaticClass());
		ExcludeComponents.AddItem(UAudioComponent::StaticClass());

		FGlobalComponentReattachContext PropagateDetailModeChanges(ExcludeComponents);
	}	
	// Reattach decal components if needed
	if( bDetailModeChanged )
	{
		TComponentReattachContext<UDecalComponent> PropagateDecalComponentChanges;
	}
}

/**
 * Sets new system settings (optionally writes out to the ini). 
 */
void FSystemSettings::ApplyNewSettings( const FSystemSettingsData& NewSettings, UBOOL bWriteToIni )
{
	// we can set any setting before the engine is initialized so don't bother restoring values.
	UBOOL bEngineIsInitialized = GEngine != NULL;

	// if the engine is running, there are certain values we can't set immediately
	if (bEngineIsInitialized)
	{
		// apply settings to the runtime system.
		ApplySettingsAtRuntime(NewSettings, bWriteToIni);
	}
	else
	{
		// if the engine is not initialized we don't need to worry about all the deferred settings etc. 
		// as we do above.
		// Set new settings. Would look prettier if we didn't derive from the Data class...
		(FSystemSettingsData&)(*this) = NewSettings;

		// If requested, save the settings to ini.
		if ( bWriteToIni )
		{
			SaveToIni();
		}
	}

	// Activate certain system settings on the renderthread
	GSystemSettings.ApplySystemSettingsToRenderThread();

	// Dump settings to the log so we know what values are being used.
	if (bWriteToIni && !bIsEditor)
	{
#if !CONSOLE
		Dump(TEXT("System Settings changed using SetFriendlyGraphicsSettings:"));
#endif
	}
}
