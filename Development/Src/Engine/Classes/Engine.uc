//=============================================================================
// Engine: The base class of the global application object classes.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Engine extends Subsystem
	native(GameEngine)
	abstract
	config(Engine)
	transient;

// Fonts.
var private Font	TinyFont;
var globalconfig string TinyFontName;

var private Font	SmallFont;
var globalconfig string SmallFontName;

var private Font	MediumFont;
var globalconfig string MediumFontName;

var private Font	LargeFont;
var globalconfig string LargeFontName;

var private Font	SubtitleFont;
var globalconfig string SubtitleFontName;

/** Any additional fonts that script may use without hard-referencing the font. */
var private array<Font>			AdditionalFonts;
var globalconfig array<string>	AdditionalFontNames;

/** The class to use for the game console. */
var class<Console> ConsoleClass;
var globalconfig string ConsoleClassName;

/** The class to use for the game viewport client. */
var class<GameViewportClient> GameViewportClientClass;
var globalconfig string GameViewportClientClassName;

/** The class to use for managing the global data stores */
var	class<DataStoreClient> DataStoreClientClass;
var	globalconfig string DataStoreClientClassName;

`if(`isdefined(STORAGE_MANAGER_IMPLEMENTED))
var	class<StorageDeviceManager> StorageDeviceManagerClass;
var	globalconfig string	StorageDeviceManagerClassName;
`endif

/** The class to use for local players. */
var class<LocalPlayer> LocalPlayerClass;
var config string LocalPlayerClassName;

/** The material used when no material is explicitly applied. */
var Material	DefaultMaterial;
var globalconfig string DefaultMaterialName;

/** The decal material used for fallback case of decals */
var Material	DefaultDecalMaterial;
var globalconfig string DefaultDecalMaterialName;

/** A global default texture. */
var Texture	DefaultTexture;
var globalconfig string DefaultTextureName;

/** The material used to render wireframe meshes. */
var Material	WireframeMaterial;
var globalconfig string WireframeMaterialName;

/** A textured material with an instance parameter for the texture. */
var Material EmissiveTexturedMaterial;
var globalconfig string EmissiveTexturedMaterialName;

/** A translucent material used to render things in geometry mode. */
var Material	GeomMaterial;
var globalconfig string GeomMaterialName;

/** The default fog volume material */
var Material	DefaultFogVolumeMaterial;
var globalconfig string DefaultFogVolumeMaterialName;

/** Material used for drawing a tick mark. */
var Material	TickMaterial;
var globalconfig string TickMaterialName;

/** Material used for drawing a cross mark. */
var Material	CrossMaterial;
var globalconfig string CrossMaterialName;

/** Material used for visualizing level membership in lit viewport modes. */
var Material	LevelColorationLitMaterial;
var globalconfig string LevelColorationLitMaterialName;

/** Material used for visualizing level membership in unlit viewport modes. */
var Material	LevelColorationUnlitMaterial;
var globalconfig string LevelColorationUnlitMaterialName;

/** Material used for visualizing level membership in lit viewport modes. Uses shading to show axis directions. */
var Material	ShadedLevelColorationLitMaterial;
var globalconfig string ShadedLevelColorationLitMaterialName;

/** Material used for visualizing level membership in unlit viewport modes.  Uses shading to show axis directions. */
var Material	ShadedLevelColorationUnlitMaterial;
var globalconfig string ShadedLevelColorationUnlitMaterialName;

/** Material used to indicate that the associated BSP surface should be removed. */
var Material	RemoveSurfaceMaterial;
var globalconfig string RemoveSurfaceMaterialName;

/** Material that renders vertex colour as emissive. */
var Material	VertexColorMaterial;
var globalconfig string VertexColorMaterialName;

/** The colors used to render light complexity. */
var globalconfig array<color> LightComplexityColors;

/** The colors used to render shader complexity. */
var globalconfig array<color> ShaderComplexityColors;

/** When true, pixel shader complexity is shown, otherwise vertex shader complexity is shown in the shader complexity viewmode. */
var globalconfig bool bUsePixelShaderComplexity;

/**
* When true, pixel shader complexity is cumulative, otherwise only the last pixel drawn contributes complexity.
* Has no effect if bUsePixelShaderComplexity is false
*/
var globalconfig bool bUseAdditiveComplexity;

/**
* Complexity limits for the various complexity viewmode combinations.
* These limits are used to map instruction counts to ShaderComplexityColors.
*/
var globalconfig float MaxPixelShaderAdditiveComplexityCount;
var globalconfig float MaxPixelShaderOpaqueComplexityCount;
var globalconfig float MaxVertexShaderComplexityCount;

/** Range for the texture density viewmode. */
var globalconfig float MinTextureDensity;
var globalconfig float IdealTextureDensity;
var globalconfig float MaxTextureDensity;

struct native StatColorMapEntry
{
	var globalconfig float	In;
	var globalconfig color	Out;
};

struct native StatColorMapping
{
	var globalconfig string	StatName;
	var globalconfig array<StatColorMapEntry> ColorMap;
	var globalconfig bool DisableBlend;
};

var globalconfig array<StatColorMapping>	StatColorMappings;

/** A material used to render the sides of the builder brush/volumes/etc. */
var Material	EditorBrushMaterial;
var globalconfig string EditorBrushMaterialName;

/** PhysicalMaterial to use if none is defined for a particular object. */
var	PhysicalMaterial	DefaultPhysMaterial;
var globalconfig string DefaultPhysMaterialName;

/** The material used when terrain compilation is too complex. */
var Material	TerrainErrorMaterial;
var globalconfig string TerrainErrorMaterialName;
var globalconfig int TerrainMaterialMaxTextureCount;

/** This is the number of frames that are used between terrain tessellation re-calculations */
var globalconfig int TerrainTessellationCheckCount;
/**
 *	The radius from the view origin that terrain tessellation checks should be performed.
 *	If 0.0, every component will be checked for tessellation changes each frame.
 */
var globalconfig float TerrainTessellationCheckDistance;

/** OnlineSubsystem class to use for netplay */
var	class<OnlineSubsystem> OnlineSubsystemClass;
var globalconfig string DefaultOnlineSubsystemName;

/** Default engine post process chain used for the game and main editor view */
var PostProcessChain DefaultPostProcess;
var config string DefaultPostProcessName;

/** post process chain used for skeletal mesh thumbnails */
var PostProcessChain ThumbnailSkeletalMeshPostProcess;
var config string ThumbnailSkeletalMeshPostProcessName;

/** post process chain used for particle system thumbnails */
var PostProcessChain ThumbnailParticleSystemPostProcess;
var config string ThumbnailParticleSystemPostProcessName;

/** post process chain used for material thumbnails */
var PostProcessChain ThumbnailMaterialPostProcess;
var config string ThumbnailMaterialPostProcessName;

/** post process chain used for rendering the UI */
var PostProcessChain DefaultUIScenePostProcess;
var config string DefaultUIScenePostProcessName;

/** Material used for drawing meshes when their collision is missing. */
var Material	DefaultUICaretMaterial;
var globalconfig string DefaultUICaretMaterialName;

/** Material used for visualizing the reflection scene captures on a surface */
var Material	SceneCaptureReflectActorMaterial;
var globalconfig string SceneCaptureReflectActorMaterialName;

/** Material used for visualizing the cube map scene captures on a mesh */
var Material	SceneCaptureCubeActorMaterial;
var globalconfig string SceneCaptureCubeActorMaterialName;

/** Texture used to get random angles per-pixel by the Branching PCF implementation */
var Texture2D RandomAngleTexture;
var globalconfig string RandomAngleTextureName;

/** Texture used to get random normals per-pixel */
var Texture2D RandomNormalTexture;
var globalconfig string RandomNormalTextureName;

/** Texture used as a placeholder for terrain weight-maps to give the material the correct texture format. */
var Texture	WeightMapPlaceholderTexture;
var globalconfig string WeightMapPlaceholderTextureName;

/** White noise sound */
var SoundNodeWave DefaultSound;
var globalconfig string DefaultSoundName;

/** Time in seconds (game time) we should wait between purging object references to objects that are pending kill */
var(Settings) config float TimeBetweenPurgingPendingKillObjects;

// Variables.

/** Abstract interface to platform-specific subsystems */
var const client							Client;

/** Viewports for all players in all game instances (all PIE windows, for example) */
var init array<LocalPlayer>					GamePlayers;

/** the viewport representing the current game instance */
var const GameViewportClient				GameViewport;

/** Array of deferred command strings/ execs that get executed at the end of the frame */
var init array<string>	DeferredCommands;

var int TickCycles, GameCycles, ClientCycles;
var(Settings) config bool bUseSound;

/** Whether to use texture streaming. */
var(Settings) config bool bUseTextureStreaming;

/** Whether to allow background level streaming. */
var(Settings) config bool bUseBackgroundLevelStreaming;

/** Flag for completely disabling subtitles for localized sounds. */
var(Settings) config bool bSubtitlesEnabled;

/** Flag for forcibly disabling subtitles even if you try to turn them back on they will be off */
var(Settings) config bool bSubtitlesForcedOff;

/**
 *	Flag for forcing terrain to be 'static' (MinTessellationLevel = MaxTesselationLevel)
 *	Game time only...
 */
var(Settings) config bool bForceStaticTerrain;

/** Global debug manager helper object that stores configuration and state used during development */
var const DebugManager			DebugManager;

/** Entry point for RemoteControl, the in-game UI for the exec system. */
var native pointer				RemoteControlExec{class FRemoteControlExec};

// Color preferences.
var(Colors) color
	C_WorldBox,
	C_BrushWire,
	C_AddWire,
	C_SubtractWire,
	C_SemiSolidWire,
	C_NonSolidWire,
	C_WireBackground,
	C_ScaleBoxHi,
	C_VolumeCollision,
	C_BSPCollision,
	C_OrthoBackground,
	C_Volume;

/** Fudge factor for tweaking the distance based miplevel determination */
var(Settings)	float			StreamingDistanceFactor;

/** Class name of the scout to use for path building */
var const config string ScoutClassName;

/**
 * A transition type.
 */
enum ETransitionType
{
	TT_None,
	TT_Paused,
	TT_Loading,
	TT_Saving,
	TT_Connecting,
	TT_Precaching
};

/** The current transition type. */
var ETransitionType TransitionType;

/** The current transition description text. */
var string TransitionDescription;

/** The gametype for the destination map */
var string TransitionGameType;

/** Level of detail range control for meshes */
var config		float					MeshLODRange;
/** Force to CPU skinning only for skeletal mesh rendering */
var	config		bool					bForceCPUSkinning;
/** Whether to use post processing effects or not */
var	config		bool					bUsePostProcessEffects;
/** whether to send Kismet warning messages to the screen (via PlayerController::ClientMessage()) */
var config bool bOnScreenKismetWarnings;
/** whether kismet logging is enabled. */
var config bool bEnableKismetLogging;
/** whether mature language is allowed **/
var config bool bAllowMatureLanguage;
/** camera rotation (deg) beyond which occlusion queries are ignored from previous frame (because they are likely not valid) */
var config float CameraRotationThreshold;
/** camera movement beyond which occlusion queries are ignored from previous frame (because they are likely not valid) */
var config float CameraTranslationThreshold;
/** The amount of time a primitive is considered to be probably visible after it was last actually visible. */
var config float PrimitiveProbablyVisibleTime;
/** The percent of previously unoccluded primitives which are requeried every frame. */
var config float PercentUnoccludedRequeries;
/** Max screen pixel fraction where retesting when unoccluded is worth the GPU time. */
var config float MaxOcclusionPixelsFraction;

/** Terrain collision viewing - If TRUE, overlay collion level else render it and overlay terrain. */
var config bool bRenderTerrainCollisionAsOverlay;

/** Do not use Ageia PhysX hardware */
var config bool bDisablePhysXHardwareSupport;

/** Whether to pause the game if focus is lost. */
var config bool bPauseOnLossOfFocus;

/** The most vertices a fluid surface can have.  The number of verts is clamped to avoid running out of memory and exposing driver bugs. */
var config int MaxFluidNumVerts;

/**
 *	Time limit (in milliseconds) for a fluid simulation update, to avoid spiraling into a bad
 *	feedback-loop with slower and slower framerate. This value is doubled in debug builds.
 */
var config float FluidSimulationTimeLimit;

/**
 *	The maximum allowed size to a ParticleEmitterInstance::Resize call.
 *	If larger, the function will return without resizing.
 */
var config int MaxParticleResize;
/**
*	If the resize request is larger than this, spew out a warning to the log
*/
var config int MaxParticleResizeWarn;
/**
 *	If TRUE, then perform particle size checks in non FINAL_RELEASE builds.
 */
var globalconfig bool bCheckParticleRenderSize;
/** The maximum amount of memory any single emitter is allowed to take for its vertices */
var config int MaxParticleVertexMemory;
var transient int MaxParticleSpriteCount;
var transient int MaxParticleSubUVCount;

/** Material used for visualizing terrain collision. */
var Material TerrainCollisionMaterial;
var globalconfig string TerrainCollisionMaterialName;

/** The number of times to attempt the Begin*UP call before assuming the GPU is hosed	*/
var config int BeginUPTryCount;

/** Info about one note dropped in the map during PIE. */
struct native DropNoteInfo
{
	/** Location to create Note actor in edited level. */
	var vector	Location;
	/** Rotation to create Note actor in edited level. */
	var rotator	Rotation;
	/** Text to assign to Note actor in edited level. */
	var string	Comment;
};

/**  */
var transient array<DropNoteInfo>	PendingDroppedNotes;

/** Overridable class for cover mesh rendering in-game, used to get around the editoronly restrictions needed by the base CoverMeshComponent */
var globalconfig string DynamicCoverMeshComponentName;

/**
 * By default, each frame's initial scene color clear is disabled.
 * This flag can be toggled at runtime to enable clearing for development.
 */
var globalconfig const bool			bEnableColorClear;

/** Number of times to tick each client per second */
var globalconfig float				NetClientTicksPerSecond;

/**
 *	The largest step-size allowed for lens flare occlusion results
 *	before using the incremental step method.
 */
var globalconfig float				LensFlareMaxOcclusionIncrement;
/**
 *	The incremental step size for the above.
 */
var globalconfig float				LensFlareOcclusionStepSize;

cpptext
{
	// Constructors.
	UEngine();
	void StaticConstructor();

	// UObject interface.
	virtual void FinishDestroy();

	// UEngine interface.
	virtual void Init();

	/**
	 * Called at shutdown, just before the exit purge.
	 */
	virtual void PreExit() {}

	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Out=*GLog );
	virtual void Tick( FLOAT DeltaSeconds ) PURE_VIRTUAL(UEngine::Tick,);
	virtual void SetClientTravel( const TCHAR* NextURL, ETravelType TravelType ) PURE_VIRTUAL(UEngine::SetClientTravel,);
	virtual FLOAT GetMaxTickRate( FLOAT /*DeltaTime*/, UBOOL bAllowFrameRateSmoothing = TRUE ) { return 0.0f; }
	virtual void SetProgress( EProgressMessageType MessageType, const FString& Title, const FString& Message );

	/**
	 * Ticks the FPS chart.
	 *
	 * @param DeltaSeconds	Time in seconds passed since last tick.
	 */
	virtual void TickFPSChart( FLOAT DeltaSeconds );

	/**
	* Ticks the Memory chart.
	*
	* @param DeltaSeconds	Time in seconds passed since last tick.
	*/
	virtual void TickMemoryChart( FLOAT DeltaSeconds );

	/**
	 * Pauses / unpauses the game-play when focus of the game's window gets lost / gained.
	 * @param EnablePause TRUE to pause; FALSE to unpause the game
	 */
	virtual void OnLostFocusPause( UBOOL EnablePause );

	/**
	 * Resets the FPS chart data.
	 */
	virtual void ResetFPSChart();

	/**
	 * Dumps the FPS chart information to the passed in archive.
	 *
	 * @param	bForceDump	Whether to dump even if FPS chart info is not enabled.
	 */
	virtual void DumpFPSChart( UBOOL bForceDump = FALSE );

	/** Dumps info on DistanceFactor used for rendering SkeletalMeshComponents during the game. */
	virtual void DumpDistanceFactorChart();

	/**
 	 * Resets the Memory chart data.
	 */
	virtual void ResetMemoryChart();

	/**
	 * Dumps the Memory chart information to various places.
	 */
	virtual void DumpMemoryChart();


private:
	/**
	 * Dumps the FPS chart information to HTML.
	 */
	virtual void DumpFPSChartToHTML( FLOAT TotalTime, FLOAT DeltaTime, INT NumFrames, UBOOL bOutputToGlobalLog );

	/**
	 * Dumps the FPS chart information to the log.
	 */
	virtual void DumpFPSChartToLog( FLOAT TotalTime, FLOAT DeltaTime, INT NumFrames );

	/**
	 * Dumps the FPS chart information to the special stats log file.
	 */
	virtual void DumpFPSChartToStatsLog( FLOAT TotalTime, FLOAT DeltaTime, INT NumFrames );

	/**
	 * Dumps the Memory chart information to HTML.
	 */
	virtual void DumpMemoryChartToHTML( FLOAT TotalTime, FLOAT DeltaTime, INT NumFrames, UBOOL bOutputToGlobalLog );

	/**
	 * Dumps the Memory chart information to the log.
	 */
	virtual void DumpMemoryChartToLog( FLOAT TotalTime, FLOAT DeltaTime, INT NumFrames );

	/**
	 * Dumps the Memory chart information to the special stats log file.
	 */
	virtual void DumpMemoryChartToStatsLog( FLOAT TotalTime, FLOAT DeltaTime, INT NumFrames );

public:

	/**
	 * Spawns any registered server actors
	 */
	virtual void SpawnServerActors(void)
	{
	}

	/**
	 * Loads all Engine object references from their corresponding config entries.
	 */
	void InitializeObjectReferences();

	/**
	 * Clean up the GameViewport
	 */
	void CleanupGameViewport();

	/** Get some viewport. Will be GameViewport in game, and one of the editor viewport windows in editor. */
	virtual FViewport* GetAViewport();

	/**
	 * Allows the editor to accept or reject the drawing of wireframe brush shapes based on mode and tool.
	 */
	virtual UBOOL ShouldDrawBrushWireframe( class AActor* InActor ) { return TRUE; }

	/**
	 * Looks at all currently loaded packages and prompts the user to save them
	 * if their "bDirty" flag is set.
	 *
	 * @param	bShouldSaveMap				TRUE if the function should save the map first before other packages.
	 * @param	bForcePackagesToFullyLoad	If TRUE, fully load dirty packages without first prompting the user.
	 * @return								TRUE on success, FALSE on fail.
	 */
	virtual UBOOL SaveDirtyPackages(UBOOL bShouldSaveMap, UBOOL bForcePackagesToFullyLoad) { return TRUE; }

	/**
	 * Issued by code reuqesting that decals be reattached.
	 */
	virtual void IssueDecalUpdateRequest() {}

	/**
	 * Returns whether or not the map build in progressed was cancelled by the user.
	 */
	virtual UBOOL GetMapBuildCancelled() const
	{
		return FALSE;
	}

	/**
	 * Sets the flag that states whether or not the map build was cancelled.
	 *
	 * @param InCancelled	New state for the cancelled flag.
	 */
	virtual void SetMapBuildCancelled( UBOOL InCancelled )
	{
		// Intentionally empty.
	}

	/**
	 * Computes a color to use for property coloration for the given object.
	 *
	 * @param	Object		The object for which to compute a property color.
	 * @param	OutColor	[out] The returned color.
	 * @return				TRUE if a color was successfully set on OutColor, FALSE otherwise.
	 */
	virtual UBOOL GetPropertyColorationColor(class UObject* Object, FColor& OutColor);

	/** Uses StatColorMappings to find a color for this stat's value. */
	UBOOL GetStatValueColoration(const FString& StatName, FLOAT Value, FColor& OutColor);


	/**
	 * @return TRUE if we allow shadow volume resources to be loaded/rendered
	 */
	static FORCEINLINE UBOOL ShadowVolumesAllowed()
	{
		return( (GIsEditor && !(GCookingTarget & UE3::PLATFORM_Console)) || (GAllowShadowVolumes && (GIsGame || (GCookingTarget & UE3::PLATFORM_Console))) );
	}

protected:
	/**
	 * Handles freezing/unfreezing of rendering
	 */
	virtual void ProcessToggleFreezeCommand()
	{
		// Intentionally empty.
	}

	/**
	 * Handles frezing/unfreezing of streaming
	 */
	 virtual void ProcessToggleFreezeStreamingCommand()
	 {
		// Intentionally empty.
	 }
}

/**
 * Returns a pointer to the current world.
 */
native static final function WorldInfo GetCurrentWorldInfo();

/**
 * Returns version info from the engine
 */
native static final function string GetBuildDate();

/**
 * Returns the engine's default tiny font
 */
native static final function Font GetTinyFont();

/**
 * Returns the engine's default small font
 */
native static final function Font GetSmallFont();

/**
 * Returns the engine's default medium font
 */
native static final function Font GetMediumFont();

/**
 * Returns the engine's default large font
 */
native static final function Font GetLargeFont();

/**
 * Returns the engine's default subtitle font
 */
native static final function Font GetSubtitleFont();

/**
 * Returns the specified additional font.
 *
 * @param	AdditionalFontIndex		Index into the AddtionalFonts array.
 */
native static final function Font GetAdditionalFont(int AdditionalFontIndex);

/** @return whether we're currently running in splitscreen (more than one local player) */
native static final function bool IsSplitScreen();

/** @return the audio device (will be None if sound is disabled) */
native static final function AudioDevice GetAudioDevice();

/** @return Returns the name of the last movie that was played. */
native static final function string GetLastMovieName();


/**
 * Play one of the LoadMap loading movies as configured by ini file
 *
 * @return TRUE if a movie was played
 */
native static final function bool PlayLoadMapMovie();

/**
 * Stops the current movie
 *
 * @param bDelayStopUntilGameHasRendered If TRUE, the engine will delay stopping the movie until after the game has rendered at least one frame
 */
native static final function StopMovie(bool bDelayStopUntilGameHasRendered);

/**
 * Removes all overlays from displaying
 */
native static final function RemoveAllOverlays();

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
native static final function AddOverlay( Font Font, string Text, float X, float Y, float ScaleX, float ScaleY, bool bIsCentered );

/**
 * Adds a wrapped text overlay to the movie
 *
 * @param Font Font to use to display (must be in the root set so this will work during loads)
 * @param Text Text to display
 * @param X X location in resolution-independent coordinates (ignored if centered)
 * @param Y Y location in resolution-independent coordinates
 * @param ScaleX Text horizontal scale
 * @param ScaleY Text vertical scale
 * @param WrapWidth Number of pixels before text should wrap
 */
native static final function AddOverlayWrapped( Font Font, string Text, float X, float Y, float ScaleX, float ScaleY, float WrapWidth );


defaultproperties
{
	C_WorldBox=(R=0,G=0,B=40,A=255)
	C_BrushWire=(R=192,G=0,B=0,A=255)
	C_AddWire=(R=127,G=127,B=255,A=255)
	C_SubtractWire=(R=255,G=192,B=63,A=255)
	C_SemiSolidWire=(R=127,G=255,B=0,A=255)
	C_NonSolidWire=(R=63,G=192,B=32,A=255)
	C_WireBackground=(R=0,G=0,B=0,A=255)
	C_ScaleBoxHi=(R=223,G=149,B=157,A=255)
	C_VolumeCollision=(R=149,G=223,B=157,A=255)
	C_BSPCollision=(R=149,G=157,B=223,A=255)
	C_OrthoBackground=(R=163,G=163,B=163,A=255)
	C_Volume=(R=255,G=196,B=255,A=255)
}
