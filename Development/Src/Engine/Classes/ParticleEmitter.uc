//=============================================================================
// ParticleEmitter
// The base class for any particle emitter objects.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class ParticleEmitter extends Object
	native(Particle)
	dependson(ParticleLODLevel)
	hidecategories(Object)
	editinlinenew
	abstract;

//=============================================================================
//	General variables
//=============================================================================
/** The name of the emitter. */
var(Particle)							name						EmitterName;
var deprecated							bool						UseLocalSpace;
var	deprecated							bool						KillOnDeactivate;
var	deprecated							bool						bKillOnCompleted;

var	deprecated							rawdistributionfloat		SpawnRate;
var	deprecated							float						EmitterDuration;
var	deprecated							int							EmitterLoops; // 0 indicates loop continuously

//=============================================================================
//	Burst emissions
//=============================================================================
enum EParticleBurstMethod
{
	EPBM_Instant,
	EPBM_Interpolated
};

struct native ParticleBurst
{
	/** The number of particles to burst */
	var()				int		Count;
	/** If >= 0, use as a range [CountLow..Count] */
	var()				int		CountLow;
	/** The time at which to burst them (0..1: emitter lifetime) */
	var()				float	Time;

	structdefaultproperties
	{
		CountLow=-1		// Disabled by default...
	}
};

var	deprecated							EParticleBurstMethod		ParticleBurstMethod;
var	deprecated			export noclear	array<ParticleBurst>		BurstList;

//=============================================================================
//	SubUV-related
//=============================================================================
enum EParticleSubUVInterpMethod
{
	PSUVIM_None,
    PSUVIM_Linear,
    PSUVIM_Linear_Blend,
    PSUVIM_Random,
    PSUVIM_Random_Blend
};

var	deprecated							EParticleSubUVInterpMethod	InterpolationMethod;
var	deprecated							int							SubImages_Horizontal;
var	deprecated							int							SubImages_Vertical;
var	deprecated							bool						ScaleUV;
var	deprecated							float						RandomImageTime;
var	deprecated							int							RandomImageChanges;
var	deprecated							bool						DirectUV;
var	transient							int							SubUVDataOffset;

//=============================================================================
//	Cascade-related
//=============================================================================
enum EEmitterRenderMode
{
	ERM_Normal,
	ERM_Point,
	ERM_Cross,
	ERM_None
};

var	deprecated							EEmitterRenderMode				EmitterRenderMode;
var	deprecated							color							EmitterEditorColor;
var	deprecated							bool							bEnabled;

//=============================================================================
//	'Private' data - not required by the editor
//=============================================================================
var editinline export					array<ParticleLODLevel>			LODLevels;
var editinline export					array<ParticleModule>			Modules;

// Module<SINGULAR> used for emitter type "extension".
var export								ParticleModule					TypeDataModule;
// Modules used for initialization.
var native								array<ParticleModule>			SpawnModules;
// Modules used for ticking.
var native								array<ParticleModule>			UpdateModules;
var										bool							ConvertedModules;
var										int								PeakActiveParticles;

//=============================================================================
//	Performance/LOD Data
//=============================================================================

/**
 *	Initial allocation count - overrides calculated peak count if > 0
 */
var(Particle)							int								InitialAllocationCount;

//=============================================================================
//	C++
//=============================================================================
cpptext
{
	virtual void PreEditChange(UProperty* PropertyThatWillChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent);

	virtual void SetToSensibleDefaults() {}

	virtual void PostLoad();
	virtual void UpdateModuleLists();

	void SetEmitterName(FName Name);
	FName& GetEmitterName();
	virtual	void						SetLODCount(INT LODCount);

	// For Cascade
	void	AddEmitterCurvesToEditor(UInterpCurveEdSetup* EdSetup);
	void	RemoveEmitterCurvesFromEditor(UInterpCurveEdSetup* EdSetup);
	void	ChangeEditorColor(FColor& Color, UInterpCurveEdSetup* EdSetup);

	void	AutoPopulateInstanceProperties(UParticleSystemComponent* PSysComp);

	// LOD
	INT					CreateLODLevel(INT LODLevel, UBOOL bGenerateModuleData = TRUE);
	UBOOL				IsLODLevelValid(INT LODLevel);

	/** GetCurrentLODLevel
	*	Returns the currently set LODLevel. Intended for game-time usage.
	*	Assumes that the given LODLevel will be in the [0..# LOD levels] range.
	*	
	*	@return NULL if the requested LODLevel is not valid.
	*			The pointer to the requested UParticleLODLevel if valid.
	*/
	inline UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance)
	{
		// for the game (where we care about perf) we don't branch
		if (GIsGame == TRUE)
		{
			return Instance->CurrentLODLevel;
		}
		else
		{
			EditorUpdateCurrentLOD( Instance );
			return Instance->CurrentLODLevel;
		}

	}

	void EditorUpdateCurrentLOD(FParticleEmitterInstance* Instance);

	UParticleLODLevel*	GetLODLevel(INT LODLevel);
	
	virtual UBOOL		AutogenerateLowestLODLevel(UBOOL bDuplicateHighest = FALSE);
	
	/**
	 *	CalculateMaxActiveParticleCount
	 *	Determine the maximum active particles that could occur with this emitter.
	 *	This is to avoid reallocation during the life of the emitter.
	 *
	 *	@return	TRUE	if the number was determined
	 *			FALSE	if the number could not be determined
	 */
	virtual UBOOL		CalculateMaxActiveParticleCount();

	/**
	 *	Retrieve the parameters associated with this particle system.
	 *
	 *	@param	ParticleSysParamList	The list of FParticleSysParams used in the system
	 *	@param	ParticleParameterList	The list of ParticleParameter distributions used in the system
	 */
	void GetParametersUtilized(TArray<FString>& ParticleSysParamList,
							   TArray<FString>& ParticleParameterList);
}

//=============================================================================
//	Default properties
//=============================================================================
defaultproperties
{
	EmitterName="Particle Emitter"

	EmitterRenderMode=ERM_Normal

	ConvertedModules=true
	PeakActiveParticles=0

	EmitterDuration=1.0
	EmitterLoops=0

	EmitterEditorColor=(R=0,G=150,B=150)

	bEnabled=true
	
	Begin Object Class=DistributionFloatConstant Name=DistributionSpawnRate
	End Object
	SpawnRate=(Distribution=DistributionSpawnRate)

	InterpolationMethod=PSUVIM_None
	SubImages_Horizontal=1
	SubImages_Vertical=1
	RandomImageTime=1.0
	RandomImageChanges=0
	ScaleUV=true
	DirectUV=false
}
