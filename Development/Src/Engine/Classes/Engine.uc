class Engine extends Subsystem
    abstract
    transient
    native(GameEngine)
    config(Engine);

enum ETransitionType
{
    TT_None,                        // 0
    TT_Paused,                      // 1
    TT_Loading,                     // 2
    TT_Saving,                      // 3
    TT_Connecting,                  // 4
    TT_Precaching,                  // 5
    TT_MAX                          // 6
};

struct native StatColorMapEntry
{
    var globalconfig float In;
    var globalconfig Color Out;
};

struct native StatColorMapping
{
    var globalconfig string StatName;
    var globalconfig array<config StatColorMapEntry> ColorMap;
    var globalconfig bool DisableBlend;
};

struct native DropNoteInfo
{
    var Vector Location;
    var Rotator Rotation;
    var string Comment;
};

struct native TwistBoneFixer
{
    var int BaseBoneIndex;
    var int TwistBoneIndex;
    var float BaseYDotTwistY;
    var float BaseYDotTwistZ;
};

struct native TwistBoneFixers
{
    var bool Ok;
    var init array<init TwistBoneFixer> Fixers;
    var init array<init byte> DependentBoneIndices;
};

struct native ParentTwistBoneFixers
{
    var init array<init int> BoneIndices;
};

struct native BreathingFixerState
{
    var bool Enabled;
    var float Spine1Scale;
    var float Spine2Scale;
    var float Spine3Scale;
};

struct native BreathingFixer
{
    var bool Ok;
    var int Spine1Index;
    var int Spine2Index;
    var int Spine3Index;
};

var private Font TinyFont;
var globalconfig string TinyFontName;
var private Font SmallFont;
var globalconfig string SmallFontName;
var private Font MediumFont;
var globalconfig string MediumFontName;
var private Font LargeFont;
var globalconfig string LargeFontName;
var private Font SubtitleFont;
var globalconfig string SubtitleFontName;
var private array<Font> AdditionalFonts;
var globalconfig array<config string> AdditionalFontNames;
var class<Console> ConsoleClass;
var globalconfig string ConsoleClassName;
var class<GameViewportClient> GameViewportClientClass;
var globalconfig string GameViewportClientClassName;
var class<DataStoreClient> DataStoreClientClass;
var globalconfig string DataStoreClientClassName;
var class<LocalPlayer> LocalPlayerClass;
var config string LocalPlayerClassName;
var Material DefaultMaterial;
var globalconfig string DefaultMaterialName;
var Material DefaultDecalMaterial;
var globalconfig string DefaultDecalMaterialName;
var Texture DefaultTexture;
var globalconfig string DefaultTextureName;
var Material WireframeMaterial;
var globalconfig string WireframeMaterialName;
var Material EmissiveTexturedMaterial;
var globalconfig string EmissiveTexturedMaterialName;
var Material GeomMaterial;
var globalconfig string GeomMaterialName;
var Material DefaultFogVolumeMaterial;
var globalconfig string DefaultFogVolumeMaterialName;
var Material TickMaterial;
var globalconfig string TickMaterialName;
var Material CrossMaterial;
var globalconfig string CrossMaterialName;
var Material LevelColorationLitMaterial;
var globalconfig string LevelColorationLitMaterialName;
var Material LevelColorationUnlitMaterial;
var globalconfig string LevelColorationUnlitMaterialName;
var Material ShadedLevelColorationLitMaterial;
var globalconfig string ShadedLevelColorationLitMaterialName;
var Material ShadedLevelColorationUnlitMaterial;
var globalconfig string ShadedLevelColorationUnlitMaterialName;
var Material RemoveSurfaceMaterial;
var globalconfig string RemoveSurfaceMaterialName;
var Material VertexColorMaterial;
var globalconfig string VertexColorMaterialName;
var globalconfig array<config Color> LightComplexityColors;
var globalconfig array<config Color> ShaderComplexityColors;
var globalconfig bool bUsePixelShaderComplexity;
var globalconfig bool bUseAdditiveComplexity;
var(Settings) config bool bUseSound;
var(Settings) config bool bUseTextureStreaming;
var(Settings) config bool bUseBackgroundLevelStreaming;
var(Settings) config bool bSubtitlesEnabled;
var(Settings) config bool bSubtitlesForcedOff;
var(Settings) config bool bForceStaticTerrain;
var globalconfig bool bUseInvertedLeftStick;
var config bool bForceCPUSkinning;
var config bool bUsePostProcessEffects;
var config bool bOnScreenKismetWarnings;
var config bool bEnableKismetLogging;
var config bool bAllowMatureLanguage;
var config bool bRenderTerrainCollisionAsOverlay;
var globalconfig bool bPhysXuseGRB;
var config bool bPauseOnLossOfFocus;
var globalconfig bool bCheckParticleRenderSize;
var config bool bDisplayDebugAudio;
var config bool bDisplayDebugAI;
var config bool bDisplayDebugAnim;
var config bool bDisplayDebugPlayer;
var config bool bDisplayDebugEngine;
var config bool bDisplayDebugBoss;
var config bool bEnablePerfMemDump;
var const globalconfig bool bEnableColorClear;
var transient bool bUnboundActiveController;
var transient bool bPausedCheck;
var globalconfig float MaxPixelShaderAdditiveComplexityCount;
var globalconfig float MaxPixelShaderOpaqueComplexityCount;
var globalconfig float MaxVertexShaderComplexityCount;
var globalconfig float MinTextureDensity;
var globalconfig float IdealTextureDensity;
var globalconfig float MaxTextureDensity;
var globalconfig array<config StatColorMapping> StatColorMappings;
var Material EditorBrushMaterial;
var globalconfig string EditorBrushMaterialName;
var PhysicalMaterial DefaultPhysMaterial;
var globalconfig string DefaultPhysMaterialName;
var Material TerrainErrorMaterial;
var globalconfig string TerrainErrorMaterialName;
var globalconfig int TerrainMaterialMaxTextureCount;
var globalconfig int TerrainTessellationCheckCount;
var globalconfig float TerrainTessellationCheckDistance;
var class<OnlineSubsystem> OnlineSubsystemClass;
var globalconfig string DefaultOnlineSubsystemName;
var PostProcessChain DefaultPostProcess;
var config string DefaultPostProcessName;
var PostProcessChain ThumbnailSkeletalMeshPostProcess;
var config string ThumbnailSkeletalMeshPostProcessName;
var PostProcessChain ThumbnailParticleSystemPostProcess;
var config string ThumbnailParticleSystemPostProcessName;
var PostProcessChain ThumbnailMaterialPostProcess;
var config string ThumbnailMaterialPostProcessName;
var PostProcessChain DefaultUIScenePostProcess;
var config string DefaultUIScenePostProcessName;
var Material DefaultUICaretMaterial;
var globalconfig string DefaultUICaretMaterialName;
var Material SceneCaptureReflectActorMaterial;
var globalconfig string SceneCaptureReflectActorMaterialName;
var Material SceneCaptureCubeActorMaterial;
var globalconfig string SceneCaptureCubeActorMaterialName;
var Texture2D RandomAngleTexture;
var globalconfig string RandomAngleTextureName;
var Texture2D RandomNormalTexture;
var globalconfig string RandomNormalTextureName;
var Texture WeightMapPlaceholderTexture;
var globalconfig string WeightMapPlaceholderTextureName;
var TextureFlipBook LoadingIconTexture;
var SoundNodeWave DefaultSound;
var globalconfig string DefaultSoundName;
var(Settings) config float TimeBetweenPurgingPendingKillObjects;
var const Client Client;
var init array<init LocalPlayer> GamePlayers;
var const GameViewportClient GameViewport;
var init array<init string> DeferredCommands;
var int TickCycles;
var int GameCycles;
var int ClientCycles;
var const DebugManager DebugManager;
var native Pointer RemoteControlExec{class FRemoteControlExec};
var(Colors) Color C_WorldBox;
var(Colors) Color C_BrushWire;
var(Colors) Color C_AddWire;
var(Colors) Color C_SubtractWire;
var(Colors) Color C_SemiSolidWire;
var(Colors) Color C_NonSolidWire;
var(Colors) Color C_WireBackground;
var(Colors) Color C_ScaleBoxHi;
var(Colors) Color C_VolumeCollision;
var(Colors) Color C_BSPCollision;
var(Colors) Color C_OrthoBackground;
var(Colors) Color C_Volume;
var(Settings) float StreamingDistanceFactor;
var const config string ScoutClassName;
var Engine.ETransitionType TransitionType;
var string TransitionDescription;
var string TransitionGameType;
var config float MeshLODRange;
var config float CameraRotationThreshold;
var config float CameraTranslationThreshold;
var config float PrimitiveProbablyVisibleTime;
var config float PercentUnoccludedRequeries;
var config float MaxOcclusionPixelsFraction;
var globalconfig int PhysXLevel;
var globalconfig float PhysXgrbSpacing;
var config int MaxFluidNumVerts;
var config float FluidSimulationTimeLimit;
var config int MaxParticleResize;
var config int MaxParticleResizeWarn;
var config int MaxParticleVertexMemory;
var transient int MaxParticleSpriteCount;
var transient int MaxParticleSubUVCount;
var Material TerrainCollisionMaterial;
var globalconfig string TerrainCollisionMaterialName;
var config int BeginUPTryCount;
var transient array<DropNoteInfo> PendingDroppedNotes;
var globalconfig string DynamicCoverMeshComponentName;
var globalconfig float NetClientTicksPerSecond;
var globalconfig float LensFlareMaxOcclusionIncrement;
var globalconfig float LensFlareOcclusionStepSize;
var export editinline transient DirectionalLightComponent TempCollisionLight;
var export editinline transient DirectionalLightComponent TempCollisionBackLight;

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

// Export UEngine::execGetCurrentWorldInfo(FFrame&, void* const)
native static final function WorldInfo GetCurrentWorldInfo();

// Export UEngine::execGetBuildDate(FFrame&, void* const)
native static final function string GetBuildDate();

// Export UEngine::execGetTinyFont(FFrame&, void* const)
native static final function Font GetTinyFont();

// Export UEngine::execGetSmallFont(FFrame&, void* const)
native static final function Font GetSmallFont();

// Export UEngine::execGetMediumFont(FFrame&, void* const)
native static final function Font GetMediumFont();

// Export UEngine::execGetLargeFont(FFrame&, void* const)
native static final function Font GetLargeFont();

// Export UEngine::execGetSubtitleFont(FFrame&, void* const)
native static final function Font GetSubtitleFont();

// Export UEngine::execGetAdditionalFont(FFrame&, void* const)
native static final function Font GetAdditionalFont(int AdditionalFontIndex);

// Export UEngine::execIsSplitScreen(FFrame&, void* const)
native static final function bool IsSplitScreen();

// Export UEngine::execGetAudioDevice(FFrame&, void* const)
native static final function AudioDevice GetAudioDevice();

// Export UEngine::execGetLastMovieName(FFrame&, void* const)
native static final function string GetLastMovieName();

// Export UEngine::execPlayLoadMapMovie(FFrame&, void* const)
native static final function bool PlayLoadMapMovie();

// Export UEngine::execStopMovie(FFrame&, void* const)
native static final function StopMovie(bool bDelayStopUntilGameHasRendered);

// Export UEngine::execRemoveAllOverlays(FFrame&, void* const)
native static final function RemoveAllOverlays();

// Export UEngine::execAddOverlay(FFrame&, void* const)
native static final function AddOverlay(Font Font, string Text, float X, float Y, float ScaleX, float ScaleY, bool bIsCentered);

// Export UEngine::execAddOverlayWrapped(FFrame&, void* const)
native static final function AddOverlayWrapped(Font Font, string Text, float X, float Y, float ScaleX, float ScaleY, float WrapWidth);

// Export UEngine::execGetPhysXLevel(FFrame&, void* const)
native static final function int GetPhysXLevel();

// Export UEngine::execGetPhysXuseGRB(FFrame&, void* const)
native static final function bool GetPhysXuseGRB();

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
