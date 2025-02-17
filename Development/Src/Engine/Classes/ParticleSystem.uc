/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleSystem extends Object
	native(Particle)
	hidecategories(Object);

/**
 *	ParticleSystemUpdateMode
 *	Enumeration indicating the method by which the system should be updated
 */
enum EParticleSystemUpdateMode
{
	/** RealTime	- update via the delta time passed in				*/
	EPSUM_RealTime,
	/** FixedTime	- update via a fixed time step						*/
	EPSUM_FixedTime
};

var()	EParticleSystemUpdateMode		SystemUpdateMode;

/** UpdateTime_FPS	- the frame per second to update at in FixedTime mode		*/
var()	float							UpdateTime_FPS;

/** UpdateTime_Delta	- internal												*/
var		float							UpdateTime_Delta;

/** WarmupTime	- the time to warm-up the particle system when first rendered	*/
var()	float							WarmupTime;

/** Emitters	- internal - the array of emitters in the system				*/
var		editinline	export	array<ParticleEmitter>	Emitters;

/** The component used to preview the particle system in Cascade				*/
var	transient ParticleSystemComponent	PreviewComponent;

/** The angle to use when rendering the thumbnail image							*/
var		rotator	ThumbnailAngle;

/** The distance to place the system when rendering the thumbnail image			*/
var		float	ThumbnailDistance;

/** The time to warm-up the system for the thumbnail image						*/
var(Thumbnail)	float					ThumbnailWarmup;

/** Boolean to indicate whether the particle system can ignore lights or not	*/
var		bool						bLit;

/** Used for curve editor to remember curve-editing setup.						*/
var		export InterpCurveEdSetup	CurveEdSetup;

//
//	LOD
//
/**
 *	How often (in seconds) the system should perform the LOD distance check.
 */
var(LOD)					float					LODDistanceCheckTime;

/**
 *	ParticleSystemLODMethod
 *	Enumeration indicating the method by which the system should perform LOD determination
 */
enum ParticleSystemLODMethod
{
	/** Automatically set the LOD level			*/
	PARTICLESYSTEMLODMETHOD_Automatic,
	/** LOD level is directly set by the game	*/
	PARTICLESYSTEMLODMETHOD_DirectSet
};

/**
 *	The method of LOD level determination to utilize for this particle system
 */
var(LOD)					ParticleSystemLODMethod		LODMethod;

/**
 *	The array of distances for each LOD level in the system.
 *	Used when LODMethod is set to PARTICLESYSTEMLODMETHOD_Automatic.
 *
 *	Example: System with 3 LOD levels
 *		LODDistances(0) = 0.0
 *		LODDistances(1) = 2500.0
 *		LODDistances(2) = 5000.0
 *
 *		In this case, when the system is [   0.0 ..   2499.9] from the camera, LOD level 0 will be used.
 *										 [2500.0 ..   4999.9] from the camera, LOD level 1 will be used.
 *										 [5000.0 .. INFINITY] from the camera, LOD level 2 will be used.
 *
 */
var(LOD)	editfixedsize	array<float>			LODDistances;

/** LOD setting for intepolation (set by Cascade) Range [0..100]				*/
var			int													EditorLODSetting;

/**
 *	Internal value that tracks the regenerate LOD levels preference.
 *	If TRUE, when autoregenerating LOD levels in code, the low level will
 *	be a duplicate of the high.
 */
var			bool									bRegenerateLODDuplicate;

/** Whether to use the fixed relative bounding box or calculate it every frame. */
var(Bounds)	bool		bUseFixedRelativeBoundingBox;
/**	Fixed relative bounding box for particle system.							*/
var(Bounds)	box			FixedRelativeBoundingBox;
/**
 * Number of seconds of emitter not being rendered that need to pass before it
 * no longer gets ticked/ becomes inactive.
 */
var()		float		SecondsBeforeInactive;

//
//	Cascade 'floor' mesh information
//
var			string		FloorMesh;
var			vector		FloorPosition;
var			rotator		FloorRotation;
var			float		FloorScale;
var			vector		FloorScale3D;

/** EDITOR ONLY: Indicates that Cascade would like to have the PeakActiveParticles count reset */
var			bool		bShouldResetPeakCounts;

/** Set during load time to indicate that physics is used... */
var		transient			bool							bHasPhysics;

/** Inidicates the old 'real-time' thumbnail rendering should be used	*/
var(Thumbnail)	bool		bUseRealtimeThumbnail;
/** Internal: Indicates the PSys thumbnail image is out of date			*/
var				bool		ThumbnailImageOutOfDate;
/** Internal: The PSys thumbnail image									*/
var				Texture2D	ThumbnailImage;

/** Content tags for quick browsing in the editor */
var editoronly array<Name> ContentTags;

/** 
 *	When TRUE, do NOT perform the spawning limiter check.
 *	Intended for effects used in pre-rendered cinematics.
 */
var() bool bSkipSpawnCountCheck;

//
/** Return the currently set LOD method											*/
native function ParticleSystemLODMethod	GetCurrentLODMethod();
/** Return the number of LOD levels for this particle system					*/
native function	int					GetLODLevelCount();
/** Return the distance for the given LOD level									*/
native function	float				GetLODDistance(int LODLevelIndex);
/** Set the LOD method															*/
native function						SetCurrentLODMethod(ParticleSystemLODMethod InMethod);
/** Set the distance for the given LOD index									*/
native function bool				SetLODDistance(int LODLevelIndex, float InDistance);

//
cpptext
{
	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PreSave();
	virtual void PostLoad();

	void UpdateColorModuleClampAlpha(class UParticleModuleColorBase* ColorModule);

	/**
	 *	CalculateMaxActiveParticleCounts
	 *	Determine the maximum active particles that could occur with each emitter.
	 *	This is to avoid reallocation during the life of the emitter.
	 *
	 *	@return	TRUE	if the numbers were determined for each emitter
	 *			FALSE	if not be determined
	 */
	virtual UBOOL		CalculateMaxActiveParticleCounts();
	
	/**
	 *	Retrieve the parameters associated with this particle system.
	 *
	 *	@param	ParticleSysParamList	The list of FParticleSysParams used in the system
	 *	@param	ParticleParameterList	The list of ParticleParameter distributions used in the system
	 */
	void GetParametersUtilized(TArray<TArray<FString> >& ParticleSysParamList,
							   TArray<TArray<FString> >& ParticleParameterList);
}

//
defaultproperties
{
	ThumbnailDistance=200.0
	ThumbnailWarmup=1.0

	UpdateTime_FPS=60.0
	UpdateTime_Delta=1.0/60.0
	WarmupTime=0.0

	bLit=true

	EditorLODSetting=0
	FixedRelativeBoundingBox=(Min=(X=-1,Y=-1,Z=-1),Max=(X=1,Y=1,Z=1))

	LODMethod=PARTICLESYSTEMLODMETHOD_Automatic
	LODDistanceCheckTime=0.25

	bRegenerateLODDuplicate=false
	ThumbnailImageOutOfDate=true

	FloorMesh="EditorMeshes.AnimTreeEd_PreviewFloor"
	FloorPosition=(X=0.000000,Y=0.000000,Z=0.000000)
	FloorRotation=(Pitch=0,Yaw=0,Roll=0)
	FloorScale=1.000000
	FloorScale3D=(X=1.000000,Y=1.000000,Z=1.000000)
}
