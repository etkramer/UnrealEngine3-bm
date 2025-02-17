/*=============================================================================
	SystemSettings.cpp: Unreal engine HW compat scalability system.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SYSTEMSETTINGS_H__
#define __SYSTEMSETTINGS_H__

/*-----------------------------------------------------------------------------
	System settings and scalability options.
-----------------------------------------------------------------------------*/

/** 
 * Split screen level can be one of these.
 * Add more if needed (4 view split screen for example)
 */
enum ESplitScreenLevel
{
	SPLITSCREENLEVEL_1 = 0,
	SPLITSCREENLEVEL_2,
	SPLITSCREENLEVEL_MAX
};

/** Friendly settings can be one of these levels */
enum EFriendlySettingsLevel
{
	FSL_Custom,
	FSL_Level1,
	FSL_Level2,
	FSL_Level3,
	FSL_Level4,
	FSL_Level5,
	FSL_LevelCount = FSL_Level5, // The number of friendly levels supported
};

/**
 * Class that handles the translation between System Settings and friendly settings
 */
 struct FSystemSettingsFriendly
 {
	 /** ctor */
	 FSystemSettingsFriendly()
		 :TextureDetail(FSL_Custom)
		 ,WorldDetail(FSL_Custom)
		 ,ShadowDetail(FSL_Custom)
		 ,bUseVSync(FALSE)
		 ,bUseMSAA(FALSE)
		 ,bFullScreen(FALSE)
		 ,ScreenPercentage(100)
		 ,bUpscaleScreenPercentage(TRUE)
		 ,ResX(0)
		 ,ResY(0)
	 {}

	 /** Friendly values */
	 EFriendlySettingsLevel TextureDetail;
	 EFriendlySettingsLevel WorldDetail;
	 EFriendlySettingsLevel ShadowDetail;
	 UBOOL bUseVSync;
	 UBOOL bUseMSAA;
	 UBOOL bFullScreen;
	 INT ScreenPercentage;
	 UBOOL bUpscaleScreenPercentage;
	 INT ResX;
	 INT ResY;
 };


/** Augments TextureLODSettings with access to TextureLODGroups. */
struct FExposedTextureLODSettings : public FTextureLODSettings
{
public:
	/** @return		A handle to the indexed LOD group. */
	FTextureLODGroup& GetTextureLODGroup(INT GroupIndex)
	{
		check( GroupIndex >= 0 && GroupIndex < TEXTUREGROUP_MAX );
		return TextureLODGroups[GroupIndex];
	}
	/** @return		A handle to the indexed LOD group. */
	const FTextureLODGroup& GetTextureLODGroup(INT GroupIndex) const
	{
		check( GroupIndex >= 0 && GroupIndex < TEXTUREGROUP_MAX );
		return TextureLODGroups[GroupIndex];
	}
};

/** Encapsulates all the system settings that make up World Detail friendly settings */
struct FSystemSettingsDataWorldDetail
{
	FSystemSettingsDataWorldDetail();

	friend bool operator==(const FSystemSettingsDataWorldDetail& LHS, const FSystemSettingsDataWorldDetail& RHS)
	{
		// WRH - 2007/09/22 - This relies on the fact that the various settings are binary comparable
		return appMemcmp(&LHS, &RHS, sizeof(LHS)) == 0;
	}

	friend bool operator!=(const FSystemSettingsDataWorldDetail& LHS, const FSystemSettingsDataWorldDetail& RHS)
	{
		return !(LHS==RHS);
	}

	/** Current detail mode; determines whether components of actors should be updated/ ticked.	*/
	INT		DetailMode;
	/** Whether to allow rendering of SpeedTree leaves.					*/
	UBOOL	bAllowSpeedTreeLeaves;
	/** Whether to allow rendering of SpeedTree fronds.					*/
	UBOOL	bAllowSpeedTreeFronds;
	/** Whether to allow static decals.									*/
	UBOOL	bAllowStaticDecals;
	/** Whether to allow dynamic decals.								*/
	UBOOL	bAllowDynamicDecals;
	/** Whether to allow decals that have not been placed in static draw lists and have dynamic view relevance */
	UBOOL	bAllowUnbatchedDecals;
	/** Scale factor for distance culling decals						*/
	FLOAT	DecalCullDistanceScale;
	/** Whether to allow dynamic lights.								*/
	UBOOL	bAllowDynamicLights;
	/** Whether to composte dynamic lights into light environments.		*/
	UBOOL	bUseCompositeDynamicLights;
	/**  Whether to allow directional lightmaps, which use the material's normal and specular. */
	UBOOL	bAllowDirectionalLightMaps;
	/** Whether to allow motion blur.									*/
	UBOOL	bAllowMotionBlur;
	/** Whether to allow depth of field.								*/
	UBOOL	bAllowDepthOfField;
	/** Whether to allow ambient occlusion.								*/
	UBOOL	bAllowAmbientOcclusion;
	/** Whether to allow bloom.											*/
	UBOOL	bAllowBloom;
	/** Whether to use high quality bloom or fast versions.				*/
	UBOOL	bUseHighQualityBloom;
	/** Whether to allow distortion.									*/
	UBOOL	bAllowDistortion;
	/** Whether to allow dropping distortion on particles based on WorldInfo::bDropDetail. */
	UBOOL	bAllowParticleDistortionDropping;
	/** Whether to allow rendering of LensFlares.						*/
	UBOOL	bAllowLensFlares;
	/** Whether to allow fog volumes.									*/
	UBOOL	bAllowFogVolumes;
	/** Whether to allow floating point render targets to be used.		*/
	UBOOL	bAllowFloatingPointRenderTargets;
	/** Whether to allow the rendering thread to lag one frame behind the game thread.	*/
	UBOOL	bAllowOneFrameThreadLag;
	/** LOD bias for skeletal meshes.									*/
	INT		SkeletalMeshLODBias;
	/** LOD bias for particle systems.									*/
	INT		ParticleLODBias;
	/** Whether to use D3D10 when it's available.						*/
	UBOOL	bAllowD3D10;	
};

/** Encapsulates all the system settings involving fractured static meshes. */
struct FSystemSettingsDataFracturedDetail
{
	FSystemSettingsDataFracturedDetail();

	friend bool operator==(const FSystemSettingsDataFracturedDetail& LHS, const FSystemSettingsDataFracturedDetail& RHS)
	{
		// WRH - 2007/09/22 - This relies on the fact that the various settings are binary comparable
		return appMemcmp(&LHS, &RHS, sizeof(LHS)) == 0;
	}
	friend bool operator!=(const FSystemSettingsDataFracturedDetail& LHS, const FSystemSettingsDataFracturedDetail& RHS)
	{
		return !(LHS==RHS);
	}

	/** Whether to allow fractured meshes to take damage.				*/
	UBOOL	bAllowFracturedDamage;
	/** Scales the game-specific number of fractured physics objects allowed.	*/
	FLOAT	NumFracturedPartsScale;
	/** Percent chance of a rigid body spawning after a fractured static mesh is damaged directly.  [0-1] */
	FLOAT	FractureDirectSpawnChanceScale;
	/** Percent chance of a rigid body spawning after a fractured static mesh is damaged by radial blast.  [0-1] */
	FLOAT	FractureRadialSpawnChanceScale;
	/** Distance scale for whether a fractured static mesh should actually fracture when damaged */
	FLOAT	FractureCullDistanceScale;
};


/** Encapsulates all the system settings that make up Shadow Detail friendly settings */
struct FSystemSettingsDataShadowDetail
{
	FSystemSettingsDataShadowDetail();

	friend bool operator==(const FSystemSettingsDataShadowDetail& LHS, const FSystemSettingsDataShadowDetail& RHS)
	{
		// WRH - 2007/09/22 - This relies on the fact that the various settings are binary comparable
		return appMemcmp(&LHS, &RHS, sizeof(LHS)) == 0;
	}

	friend bool operator!=(const FSystemSettingsDataShadowDetail& LHS, const FSystemSettingsDataShadowDetail& RHS)
	{
		return !(LHS==RHS);
	}

	/** Whether to allow dynamic shadows.								*/
	UBOOL	bAllowDynamicShadows;
	/** Whether to allow dynamic light environments to cast shadows.	*/
	UBOOL	bAllowLightEnvironmentShadows;
	/** Quality bias for projected shadow buffer filtering.	 Higher values use better quality filtering.		*/
	INT		ShadowFilterQualityBias;
	/** min dimensions (in texels) allowed for rendering shadow subject depths */
	INT		MinShadowResolution;
	/** max square dimensions (in texels) allowed for rendering shadow subject depths */
	INT		MaxShadowResolution;
	/** The ratio of subject pixels to shadow texels.					*/
	FLOAT	ShadowTexelsPerPixel;
	/** Toggle VSM (Variance Shadow Map) usage for projected shadows */
	UBOOL	bEnableVSMShadows;
	/** Toggle Branching PCF implementation for projected shadows */
	UBOOL	bEnableBranchingPCFShadows;
	/** Toggle extra geometry pass needed for culling shadows on emissive and backfaces */
	UBOOL	bAllowBetterModulatedShadows;
	/** hack to allow for foreground DPG objects to cast shadows on the world DPG */
	UBOOL	bEnableForegroundShadowsOnWorld;
	/** Whether to allow foreground DPG self-shadowing */
	UBOOL	bEnableForegroundSelfShadowing;
	/** Radius, in shadowmap texels, of the filter disk */
	FLOAT	ShadowFilterRadius;
	/** Depth bias that is applied in the depth pass for all types of projected shadows except VSM */
	FLOAT	ShadowDepthBias;
	/** Resolution in texel below which shadows are faded out. */
	INT		ShadowFadeResolution;
	/** Controls the rate at which shadows are faded out. */
	FLOAT	ShadowFadeExponent;
	/** Lights with radius below threshold will not cast shadow volumes. */
	FLOAT	ShadowVolumeLightRadiusThreshold;
	/** Primitives with screen space percantage below threshold will not cast shadow volumes. */
	FLOAT	ShadowVolumePrimitiveScreenSpacePercentageThreshold;	
};


/** Encapsulates all the system settings that make up Texture Detail friendly settings */
struct FSystemSettingsDataTextureDetail
{
	FSystemSettingsDataTextureDetail();

	friend bool operator==(const FSystemSettingsDataTextureDetail& LHS, const FSystemSettingsDataTextureDetail& RHS)
	{
		// WRH - 2007/09/22 - This relies on the fact that the various settings are binary comparable
		return appMemcmp(&LHS, &RHS, sizeof(LHS)) == 0;
	}
	friend bool operator!=(const FSystemSettingsDataTextureDetail& LHS, const FSystemSettingsDataTextureDetail& RHS)
	{
		return !(LHS==RHS);
	}

	/** Global texture LOD settings.									*/
	FExposedTextureLODSettings TextureLODSettings;

	/** If enabled, texture will only be streamed in, not out.			*/
	UBOOL	bOnlyStreamInTextures;
	/** Maximum level of anisotropy used.								*/
	INT		MaxAnisotropy;
	/** Scene capture streaming texture update distance scalar.			*/
	FLOAT	SceneCaptureStreamingMultiplier;
	/** Foliage draw distance scalar. 									*/
	FLOAT	FoliageDrawRadiusMultiplier;
};

/** Encapsulates all the system settings that make up VSync friendly settings */
struct FSystemSettingsDataVSync
{
	FSystemSettingsDataVSync();

	friend bool operator==(const FSystemSettingsDataVSync& LHS, const FSystemSettingsDataVSync& RHS)
	{
		return LHS.bUseVSync == RHS.bUseVSync;
	}
	friend bool operator!=(const FSystemSettingsDataVSync& LHS, const FSystemSettingsDataVSync& RHS)
	{
		return !(LHS==RHS);
	}

	/** Whether to use VSync or not.									*/
	UBOOL	bUseVSync;
};

/** Encapsulates all the system settings that make up screen percentage friendly settings */
struct FSystemSettingsDataScreenPercentage
{
	FSystemSettingsDataScreenPercentage();

	friend bool operator==(const FSystemSettingsDataScreenPercentage& LHS, const FSystemSettingsDataScreenPercentage& RHS)
	{
		return LHS.ScreenPercentage == RHS.ScreenPercentage;
	}
	friend bool operator!=(const FSystemSettingsDataScreenPercentage& LHS, const FSystemSettingsDataScreenPercentage& RHS)
	{
		return !(LHS==RHS);
	}

	/** Percentage of screen main view should take up.					*/
	FLOAT	ScreenPercentage;
	UBOOL	bUpscaleScreenPercentage;
};

/** Encapsulates all the system settings that make up resolution friendly settings */
struct FSystemSettingsDataResolution
{
	FSystemSettingsDataResolution();

	friend bool operator==(const FSystemSettingsDataResolution& LHS, const FSystemSettingsDataResolution& RHS)
	{
		return LHS.ResX == RHS.ResX && LHS.ResY == RHS.ResY && LHS.bFullscreen == RHS.bFullscreen;
	}
	friend bool operator!=(const FSystemSettingsDataResolution& LHS, const FSystemSettingsDataResolution& RHS)
	{
		return !(LHS==RHS);
	}

	/** Screen X resolution */
	INT ResX;
	/** Screen Y resolution */
	INT ResY;
	/** Fullscreen */
	UBOOL bFullscreen;
};

/** Encapsulates all the system settings that make up MSAA settings */
struct FSystemSettingsDataMSAA
{
	FSystemSettingsDataMSAA();

	friend bool operator==(const FSystemSettingsDataMSAA& LHS, const FSystemSettingsDataMSAA& RHS)
	{
		return LHS.MaxMultiSamples == RHS.MaxMultiSamples;
	}
	friend bool operator!=(const FSystemSettingsDataMSAA& LHS, const FSystemSettingsDataMSAA& RHS)
	{
		return !(LHS==RHS);
	}


	/** The maximum number of MSAA samples to use.						*/
	INT		MaxMultiSamples;
};

/** 
 * Struct that holds the actual data for the system settings. 
 *
 * Uses the insane derivation for backwards compatibility purposes only. Would be cleaner to use composition.
 */
struct FSystemSettingsData 
	: public FSystemSettingsDataWorldDetail
	, public FSystemSettingsDataTextureDetail
	, public FSystemSettingsDataVSync
	, public FSystemSettingsDataScreenPercentage
	, public FSystemSettingsDataResolution
	, public FSystemSettingsDataMSAA
	, public FSystemSettingsDataShadowDetail
	, public FSystemSettingsDataFracturedDetail
{
	/** ctor */
	FSystemSettingsData();

	/** loads settings from the given section in the given ini */
	void LoadFromIni(const TCHAR* IniSection, const TCHAR* IniFilename = GEngineIni, UBOOL bAllowMissingValues = TRUE);

	/** saves settings to the given section in the engine ini */
	void SaveToIni(const TCHAR* IniSection);

	/** Dumps the settings to the log file */
	void Dump(const TCHAR* DumpHeading);

private:
	void WriteTextureLODGroupsToIni(const TCHAR* IniSection);
	void WriteTextureLODGroupToIni(TextureGroup TextureGroupID, const TCHAR* GroupName, const TCHAR* IniSection);

	void DumpTextureLODGroups();
	void DumpTextureLODGroup(TextureGroup TextureGroupID, const TCHAR* GroupName);
};


/**
 * Class that actually holds the current system setttings
 *
 * Derive from FSystemSettingsData instead of holding one purely for backwards
 * compatibility reasons (each data element used to be a member of the class).
 */
class FSystemSettings : public FSystemSettingsData, public FExec, private FNoncopyable
{
public:
	/** Constructor, initializing all member variables. */
	FSystemSettings();

	/**
	 * Initializes system settings and included texture LOD settings.
	 *
	 * @param bSetupForEditor	Whether to initialize settings for Editor
	 */
	void Initialize( UBOOL bSetupForEditor );

	/**
	 * Sets new system settings (optionally writes out to the ini).
	 */
	void ApplyNewSettings(const FSystemSettingsData& NewSettings, UBOOL bWriteToIni);

	/**
	 * Returns the current friendly settings as defined by the current system settings.
	 *
	 * We read from the ini because some settings are deferred until the next reboot.
	 */
	FSystemSettingsFriendly ConvertToFriendlySettings(bool bReadSettingsFromIni = TRUE);

	/**
	 * Sets the system settings using the given friendly settings (optionally writes out to the ini).
	 */
	void ApplyFriendlySettings(const FSystemSettingsFriendly& FriendlySettings, UBOOL bWriteToIni);

	/** 
	 * Ensures that the correct settings are being used based on split screen type.
	 */
	void UpdateSplitScreenSettings(ESplitScreenType NewSplitScreenType);

	/** 
	 * Sets the resolution and writes the values to Ini if changed but does not apply the changes (eg resize the viewport).
	 */
	void SetResolution(INT InSizeX, INT InSizeY, UBOOL InFullscreen);

	const FSystemSettingsData& GetDefaultSettings(EFriendlySettingsLevel Level);

	/**
	 * Exec handler implementation.
	 *
	 * @param Cmd	Command to parse
	 * @param Ar	Output device to log to
	 *
	 * @return TRUE if command was handled, FALSE otherwise
	 */
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

	/**
	 * Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
	 *
	 * @param X - in/out X screen offset
	 * @param Y - in/out Y screen offset
	 * @param SizeX - in/out X screen size
	 * @param SizeY - in/out Y screen size
	 */
	void ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY );

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
	void UnScaleScreenCoords( 
		INT &OriginalX, INT &OriginalY, 
		UINT &OriginalSizeX, UINT &OriginalSizeY, 
		FLOAT InX, FLOAT InY, 
		FLOAT InSizeX, FLOAT InSizeY);

	/** 
	 * Indicates whether upscaling is needed 
	 */
	UBOOL NeedsUpscale() const;
	
	/** 
	 * Container for renderthread version of FSystemSettings 
	 */
	struct FRenderThreadSettings
	{
		UBOOL	bAllowMotionBlur;
		UBOOL	bAllowAmbientOcclusion;
		UBOOL	bAllowDynamicShadows;
		UBOOL	bAllowFogVolumes;
		INT		MaxMultiSamples;
		INT		MinShadowResolution;
		INT		MaxShadowResolution;
		INT		bAllowUnbatchedDecals;
	};

	/** 
	 * Current renderthread state 
	 */
	FRenderThreadSettings	RenderThreadSettings;

private:
	/** Since System Settings is called into before GIsEditor is set, we must cache this value. */
	UBOOL bIsEditor;

	/** Split screen level that is currently being used. */
	ESplitScreenLevel CurrentSplitScreenLevel;

	/**
	 * These are the default system settings for each supported compatibility level.
	 * They are initialized from engine ini.
	 */ 
	FSystemSettingsData Defaults[FSL_LevelCount][SPLITSCREENLEVEL_MAX];

	/**
	 * Loads settings from the ini. (purposely override the inherited name so people can't accidentally call it.)
	 */
	void LoadFromIni();

	/**
	 * Saves current settings to the ini. (purposely override the inherited name so people can't accidentally call it.)
	 */
	void SaveToIni();

	/**
	 * Reads a single entry and parses it into the group array.
	 *
	 * @param	TextureGroupID		Index/enum of group to parse
	 * @param	MinLODSize			Minimum size, in pixels, below which the code won't bias.
	 * @param	MaxLODSize			Maximum size, in pixels, above which the code won't bias.
	 * @param	LODBias				Group LOD bias.
	 */
	void SetTextureLODGroup(TextureGroup TextureGroupID, int MinLODSize, INT MaxLODSize, INT LODBias);

	/**
	 * Writes all texture group LOD settings to the specified ini.
	 *
	 * @param	IniFilename			The .ini file to save to.
	 * @param	IniSection			The .ini section to save to.
	 */
	void WriteTextureLODGroupsToIni(const TCHAR* IniSection);

	/**
	 * Writes the specified texture group LOD settings to the specified ini.
	 *
	 * @param	TextureGroupID		Index/enum of the group to parse
	 * @param	GroupName			String representation of the texture group, to be used as the ini key.
	 * @param	IniSection			The .ini section to save to.
	 */
	void WriteTextureLODGroupToIni(TextureGroup TextureGroupID, const TCHAR* GroupName, const TCHAR* IniSection);

	/**
	 * Recreates texture resources and drops mips.
	 *
	 * @return		TRUE if the settings were applied, FALSE if they couldn't be applied immediately.
	 */
	UBOOL UpdateTextureStreaming();

	/**
	 * Writes current settings to the logfile
	 // WRH - 2007/08/29 - Can't be const due to peculiar use of macros
	 */
	void DumpCurrentSettings(const TCHAR* DumpHeading);

	/**
	 * Helper for ApplyNewSettings when the engine is running. Applies the changes needed for the runtime system.
	 *
	 * We can assume the game is running if this code is called.
	 */
	void ApplySettingsAtRuntime(const FSystemSettingsData& NewSettings, UBOOL bWriteToInii);

	/** 
	 * Command run on the rendering thread which sets RT specific system settings
	 */
	void ApplySystemSettings_RenderingThread(const FSystemSettings::FRenderThreadSettings& NewSettings);

	/** 
	 * Makes System Settings take effect on the rendering thread 
	 */
	void ApplySystemSettingsToRenderThread();
};

/**
 * Returns the friendly settings that correspond to the given system settings.
 */
FSystemSettingsFriendly ConvertToFriendlySettings(const FSystemSettingsData& Settings);
/**
 * Returns the system settings that correspond to the given friendly settings.
 * The SourceForCustomSettings parameter specifies how to initialize the data for friendly settings that are "Custom"
 * level. Since there's no direct translation, it just uses the existing settings from the supplied data.
 */
FSystemSettingsData ConvertToSystemSettings(const FSystemSettingsFriendly& FriendlySettings, const FSystemSettingsData& SourceForCustomSettings = FSystemSettingsData());


/**
 * Global system settings accessor
 */
extern FSystemSettings GSystemSettings;

#endif // __SYSTEMSETTINGS_H__
