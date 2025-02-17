/*=============================================================================
	UnParticleEmitterInstances.h: 
	Particle emitter instance definitions/ macros.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
/*-----------------------------------------------------------------------------
	Helper macros.
-----------------------------------------------------------------------------*/
//	Macro fun.

/*-----------------------------------------------------------------------------
	Forward declarations
-----------------------------------------------------------------------------*/
//	Emitter and module types
class UParticleEmitter;
class UParticleSpriteEmitter;
class UParticleModule;
// Data types
class UParticleModuleTypeDataMesh;
class UParticleModuleTypeDataTrail;
class UParticleModuleTypeDataBeam;
class UParticleModuleTypeDataBeam2;
class UParticleModuleTypeDataTrail2;

class UStaticMeshComponent;
class UParticleSystemComponent;

class UParticleModuleBeamSource;
class UParticleModuleBeamTarget;
class UParticleModuleBeamNoise;

class UParticleModuleTrailSource;
class UParticleModuleTrailSpawn;
class UParticleModuleTrailTaper;

class UParticleModuleOrientationAxisLock;

class UParticleLODLevel;

class FParticleSystemSceneProxy;
class FParticleDynamicData;
struct FDynamicBeam2EmitterData;
struct FDynamicTrail2EmitterData;

/*-----------------------------------------------------------------------------
	Particle Emitter Instance type
-----------------------------------------------------------------------------*/
struct FParticleEmitterInstanceType
{
	const TCHAR* Name;
	FParticleEmitterInstanceType* Super;

	/** Constructor */
	FParticleEmitterInstanceType(const TCHAR* InName, FParticleEmitterInstanceType* InSuper) :
		Name(InName),
		Super(InSuper)
	{
	}

	/** 
	 *	IsA 
	 *	@param	Type			The type to check for
	 *	@return UBOOL	TRUE	if the type is exactly or is derived from the given type.
	 *					FALSE	if not
	 */
	FORCEINLINE UBOOL IsA(FParticleEmitterInstanceType& Type)
	{
		FParticleEmitterInstanceType* CurrentSuper = this;
		while (CurrentSuper)
		{
			if (CurrentSuper == &Type)
			{
				return TRUE;
			}
			CurrentSuper = CurrentSuper->Super;
		}

		return FALSE;
	}
};

#define DECLARE_PARTICLEEMITTERINSTANCE_TYPE(TypeName, SuperType)	\
	typedef SuperType SuperResource;								\
	static FParticleEmitterInstanceType StaticType;					\
	virtual FParticleEmitterInstanceType* Type()					\
	{																\
		return &StaticType;											\
	}

#define IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(TypeName)			\
	FParticleEmitterInstanceType TypeName::StaticType(				\
		TEXT(#TypeName), &TypeName::SuperResource::StaticType);

struct FLODBurstFired
{
    TArray<UBOOL> BurstFired;
};

/*-----------------------------------------------------------------------------
	FParticleEmitterInstance
-----------------------------------------------------------------------------*/
// This is the base-class 'IMPLEMENT' line
//FParticleEmitterInstanceType FParticleEmitterInstance::StaticType(TEXT("FParticleEmitterInstance"),NULL);
struct FParticleEmitterInstance
{
public:
	static FParticleEmitterInstanceType	StaticType;

	/** The maximum DeltaTime allowed for updating PeakActiveParticle tracking.
	 *	Any delta time > this value will not impact active particle tracking.
	 */
	static const FLOAT PeakActiveParticleUpdateDelta;

	/** The template this instance is based on.							*/
	UParticleSpriteEmitter* SpriteTemplate;
	/** The component who owns it.										*/
    UParticleSystemComponent* Component;
	/** The index of the currently set LOD level.						*/
    INT CurrentLODLevelIndex;
	/** The currently set LOD level.									*/
    UParticleLODLevel* CurrentLODLevel;
	/** The offset to the TypeDate payload in the particle data.		*/
    INT TypeDataOffset;
	/** The offset to the SubUV payload in the particle data.			*/
    INT SubUVDataOffset;
	/** The offset to the dynamic parameter payload in the particle data*/
    INT DynamicParameterDataOffset;
	/** The offset to the Orbit module payload in the particle data.	*/
	INT OrbitModuleOffset;
	/** The location of the emitter instance							*/
    FVector Location;
	/** If TRUE, kill this emitter instance when it is deactivated.		*/
    BITFIELD KillOnDeactivate:1;
	/** if TRUE, kill this emitter instance when it has completed.		*/
    BITFIELD bKillOnCompleted:1;
	/** Whether this emitter requires sorting as specified by artist.	*/
	BITFIELD bRequiresSorting:1;
	/** Pointer to the particle data array.								*/
    BYTE* ParticleData;
	/** Pointer to the particle index array.							*/
    WORD* ParticleIndices;
	/** Map module pointers to their offset into the particle data.		*/
    TMap<UParticleModule*,UINT> ModuleOffsetMap;
	/** Pointer to the instance data array.								*/
    BYTE* InstanceData;
	/** The size of the Instance data array.							*/
    INT InstancePayloadSize;
	/** Map module pointers to their offset into the instance data.		*/
    TMap<UParticleModule*,UINT> ModuleInstanceOffsetMap;
	/** The offset to the particle data.								*/
    INT PayloadOffset;
	/** The total size of a particle (in bytes).						*/
    INT ParticleSize;
	/** The stride between particles in the ParticleData array.			*/
    INT ParticleStride;
	/** The number of particles currently active in the emitter.		*/
    INT ActiveParticles;
	/** The maximum number of active particles that can be held in 
	 *	the particle data array.
	 */
    INT MaxActiveParticles;
	/** The fraction of time left over from spawning.					*/
    FLOAT SpawnFraction;
	/** The number of seconds that have passed since the instance was
	 *	created.
	 */
    FLOAT SecondsSinceCreation;
	/** */
    FLOAT EmitterTime;
	/** The previous location of the instance.							*/
    FVector OldLocation;
	/** The bounding box for the particles.								*/
    FBox ParticleBoundingBox;
	/** The BurstFire information.										*/
    TArray<struct FLODBurstFired> BurstFired;
	/** The number of loops completed by the instance.					*/
    INT LoopCount;
	/** Flag indicating if the render data is dirty.					*/
	INT IsRenderDataDirty;
	/** The AxisLock module - to avoid finding each Tick.				*/
    UParticleModuleOrientationAxisLock* Module_AxisLock;
	/** The current duration fo the emitter instance.					*/
    FLOAT EmitterDuration;
	/** The emitter duration at each LOD level for the instance.		*/
    TArray<FLOAT> EmitterDurations;

	/** The number of triangles to render								*/
	INT	TrianglesToRender;
	INT MaxVertexIndex;

	/** The material to render this instance with.						*/
	UMaterialInterface* CurrentMaterial;

	UBOOL bUseNxFluid;

#if !FINAL_RELEASE
	/** Number of events this emitter has generated... */
	INT EventCount;
	INT MaxEventCount;
#endif	//#if !FINAL_RELEASE

	/** Constructor	*/
	FParticleEmitterInstance();

	/** Destructor	*/
	virtual ~FParticleEmitterInstance();

	// Type interface
	virtual FParticleEmitterInstanceType* Type()			{	return &StaticType;		}

	//
    virtual void SetKillOnDeactivate(UBOOL bKill);
    virtual void SetKillOnCompleted(UBOOL bKill);
	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	virtual void Init();
	virtual UBOOL Resize(INT NewMaxActiveParticles, UBOOL bSetMaxActiveCount = TRUE);
	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);
	virtual void Rewind();
	virtual FBox GetBoundingBox();
	virtual void UpdateBoundingBox(FLOAT DeltaTime);
	virtual UINT RequiredBytes();
	virtual BYTE* GetModuleInstanceData(UParticleModule* Module);
	virtual UINT CalculateParticleStride(UINT ParticleSize);
	virtual void ResetBurstList();
	virtual FLOAT GetCurrentBurstRateOffset(FLOAT& DeltaTime, INT& Burst);
	virtual void ResetParticleParameters(FLOAT DeltaTime, DWORD StatId);
	void CalculateOrbitOffset(FOrbitChainModuleInstancePayload& Payload, 
		FVector& AccumOffset, FVector& AccumRotation, FVector& AccumRotationRate, 
		FLOAT DeltaTime, FVector& Result, FMatrix& RotationMat);
	virtual void UpdateOrbitData(FLOAT DeltaTime);
	virtual void ParticlePrefetch();

	/**
	 *	Spawn particles for this emitter instance
	 *
	 *	@param	DeltaTime		The time slice to spawn over
	 *
	 *	@return	FLOAT			The leftover fraction of spawning
	 */
	virtual FLOAT Spawn(FLOAT DeltaTime);
	/**
	 *	Spawn/burst the given particles...
	 *
	 *	@param	DeltaTime		The time slice to spawn over.
	 *	@param	InSpawnCount	The number of particles to forcibly spawn.
	 *	@param	InBurstCount	The number of particles to forcibly burst.
	 *	@param	InLocation		The location to spawn at.
	 *	@param	InVelocity		OPTIONAL velocity to have the particle inherit.
	 *
	 */
	virtual void ForceSpawn(FLOAT DeltaTime, INT InSpawnCount, INT InBurstCount, FVector& InLocation, FVector& InVelocity);
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(FBaseParticle* Particle);
	virtual UBOOL HasCompleted();
	virtual void PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime);
	virtual void KillParticles();
	/**
	 *	Kill the particle at the given instance
	 *
	 *	@param	Index		The index of the particle to kill.
	 */
	virtual void KillParticle(INT Index);
	virtual void KillParticlesForced();
	virtual void RemovedFromScene();
	virtual FBaseParticle* GetParticle(INT Index);

	/**
	 *	Calculates the emitter duration for the instance.
	 */
	void SetupEmitterDuration();
	
	/**
	 * Returns whether the system has any active particles.
	 *
	 * @return TRUE if there are active particles, FALSE otherwise.
	 */
	UBOOL HasActiveParticles()
	{
		return ActiveParticles > 0;
	}
	
	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected)
	{
		return NULL;
	}

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected)
	{
		// Base class does nothing...
		return FALSE;
	}

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData()
	{
		return NULL;
	}

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		InNum = 0;
		InMax = 0;
	}
	
	/**
	 * Init physics data associated with this emitter.
	 */
	virtual void InitPhysicsParticleData(DWORD NumParticles) {}
	/**
	 * Sync physics data associated with this emitter.
	 */
	virtual void SyncPhysicsData(void) {}

	/**
	 *	Process received events.
	 */
	virtual void ProcessEvents(FLOAT DeltaTime, UBOOL bSuppressSpawning);


protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );

};

/*-----------------------------------------------------------------------------
	Helper functions.
-----------------------------------------------------------------------------*/
/**
 * Safely casts a FParticleEmitterInstance to a specific type.
 *
 * @param 	template	type of result
 * @param	Src			instance to be cast
 * @return	Src	if instance is of the right type, NULL otherwise
 */
template<class T> T* CastEmitterInstance(FParticleEmitterInstance* Src)
{
	return Src && Src->Type()->IsA(T::StaticType) ? (T*)Src : NULL;
}

/**
 * Casts a FParticleEmitterInstance to a specific type and asserts if Src is 
 * not a subclass of the requested type.
 *
 * @param 	template	type of result
 * @param	Src			instance to be cast
 * @return	Src cast to the destination type
 */
template<class T, class U> T* CastEmitterInstanceChecked(U* Src)
{
	if (!Src || !Src->Type()->IsA(T::StaticType))
	{
		appErrorf(TEXT("Cast of %s to %s failed"), Src ? Src->Type()->Name : TEXT("NULL"), T::StaticType.Name);
	}
	return (T*)Src;
}

/*-----------------------------------------------------------------------------
	ParticleSpriteEmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleSpriteEmitterInstance : public FParticleEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteEmitterInstance, FParticleEmitterInstance);

	/** Constructor	*/
	FParticleSpriteEmitterInstance();

	/** Destructor	*/
	virtual ~FParticleSpriteEmitterInstance();

	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData();

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		INT Size = sizeof(FParticleSpriteEmitterInstance);
		INT ActiveParticleDataSize = ActiveParticles * ParticleStride;
		INT MaxActiveParticleDataSize = MaxActiveParticles * ParticleStride;
		INT ActiveParticleIndexSize = ActiveParticles * sizeof(WORD);
		INT MaxActiveParticleIndexSize = MaxActiveParticles * sizeof(WORD);

		InNum = ActiveParticleDataSize + ActiveParticleIndexSize + Size;
		InMax = MaxActiveParticleDataSize + MaxActiveParticleIndexSize + Size;

		// Take dynamic data into account as well
		Size = sizeof(FDynamicSpriteEmitterData);
		Size += MaxActiveParticleDataSize;								// Copy of the particle data on the render thread
		Size += MaxActiveParticleIndexSize;								// Copy of the particle indices on the render thread
		Size += MaxActiveParticles * sizeof(FParticleSpriteVertex);		// The vertex data array

		InNum += Size;
		InMax += Size;
	}


protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );

};

/*-----------------------------------------------------------------------------
	ParticleSpriteSubUVEmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleSpriteSubUVEmitterInstance : public FParticleEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteSubUVEmitterInstance, FParticleEmitterInstance);

	/** Constructor	*/
	FParticleSpriteSubUVEmitterInstance();

	/** Destructor	*/
	virtual ~FParticleSpriteSubUVEmitterInstance();

	virtual void KillParticles();

	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData();

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		INT Size = sizeof(FParticleSpriteSubUVEmitterInstance);
		INT ActiveParticleDataSize = ActiveParticles * ParticleStride;
		INT MaxActiveParticleDataSize = MaxActiveParticles * ParticleStride;
		INT ActiveParticleIndexSize = ActiveParticles * sizeof(WORD);
		INT MaxActiveParticleIndexSize = MaxActiveParticles * sizeof(WORD);

		InNum = ActiveParticleDataSize + ActiveParticleIndexSize + Size;
		InMax = MaxActiveParticleDataSize + MaxActiveParticleIndexSize + Size;

		// Take dynamic data into account as well
		Size = sizeof(FDynamicSubUVEmitterData);
		Size += MaxActiveParticleDataSize;									// Copy of the particle data on the render thread
		Size += MaxActiveParticleIndexSize;									// Copy of the particle indices on the render thread
		Size += MaxActiveParticles * sizeof(FParticleSpriteSubUVVertex);	// The vertex data array

		InNum += Size;
		InMax += Size;
	}


protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );
};

/*-----------------------------------------------------------------------------
	ParticleMeshEmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleMeshEmitterInstance : public FParticleEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleMeshEmitterInstance, FParticleEmitterInstance);

	UParticleModuleTypeDataMesh* MeshTypeData;
	INT MeshComponentIndex;
	UBOOL MeshRotationActive;
	INT MeshRotationOffset;
	UBOOL bIgnoreComponentScale;

	/** The materials to render this instance with.	*/
	TArray<UMaterialInterface*> CurrentMaterials;

	/** Constructor	*/
	FParticleMeshEmitterInstance();

	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	virtual void Init();
	virtual UBOOL Resize(INT NewMaxActiveParticles, UBOOL bSetMaxActiveCount = TRUE);
	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);
	virtual void UpdateBoundingBox(FLOAT DeltaTime);
	virtual UINT RequiredBytes();
	virtual void PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime);

	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData();

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		INT Size = sizeof(FParticleMeshEmitterInstance);
		INT ActiveParticleDataSize = ActiveParticles * ParticleStride;
		INT MaxActiveParticleDataSize = MaxActiveParticles * ParticleStride;
		INT ActiveParticleIndexSize = ActiveParticles * sizeof(WORD);
		INT MaxActiveParticleIndexSize = MaxActiveParticles * sizeof(WORD);

		InNum = ActiveParticleDataSize + ActiveParticleIndexSize + Size;
		InMax = MaxActiveParticleDataSize + MaxActiveParticleIndexSize + Size;

		// Take dynamic data into account as well
		Size = sizeof(FDynamicMeshEmitterData);
		Size += MaxActiveParticleDataSize;								// Copy of the particle data on the render thread
		Size += MaxActiveParticleIndexSize;								// Copy of the particle indices on the render thread

		InNum += Size;
		InMax += Size;
	}

protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );
};

#if WITH_NOVODEX
/*-----------------------------------------------------------------------------
	ParticleMeshPhysXEmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleMeshPhysXEmitterInstance : public FParticleMeshEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleMeshPhysXEmitterInstance, FParticleMeshEmitterInstance);

	FParticleMeshPhysXEmitterInstance(class UParticleModuleTypeDataMeshPhysX &TypeData);
	virtual ~FParticleMeshPhysXEmitterInstance();

	UParticleModuleTypeDataMeshPhysX	&PhysXTypeData;

	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);
	
	/** Does nothing for this emitter instance type. */
	virtual void ParticlePrefetch();

	virtual FLOAT Spawn(FLOAT DeltaTime);
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);

	virtual void RemovedFromScene();

	/** Forwards call to correspnding FParticleMeshPhysXEmitterRenderInstance object. */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/** Lifetime particle reduction is performed by FPhysXParticleSet */
	virtual void KillParticles() {}

	void RemoveParticles();

	/** Executes spawning of particles in sync with PhysX SDK */
	void SpawnSyncPhysX();

	/** Provides LOD relevant camera position. TODO: figure out how this should be done properly. */
	UBOOL GetLODOrigin(FVector& OutLODOrigin);
	
	/** Provides LOD relevant camera near plane. TODO: figure out how this should be done properly. */
	UBOOL GetLODNearClippingPlane(FPlane& OutClippingPlane);

	/** Returns smoothed estimate for rate*lifetime in [#particles]*/
	INT GetSpawnVolumeEstimate();

	/** Sets spawn budget for this emitter instance in [#particles] */
	void SetEmissionBudget(INT Budget);

	/** See ParticleModuleTypeDataPhysX.uc */
	FLOAT GetWeightForSpawnLod();


protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );


private:

	void CleanUp();
	UBOOL TryConnect();
	class FPhysXParticleSystem& GetPSys();
	FParticleSystemSceneProxy* GetSceneProxy();
	struct FRBVolumeFill* GetVolumeFill();
	void FillVolume( struct FRBVolumeFill & VolumeFill );
	void AsyncProccessSpawnedParticles(FLOAT DeltaTime);
	void SpawnEstimateUpdate(FLOAT DeltaTime, INT NumSpawnedParticles, FLOAT LifetimeSum);
	void OverwriteUnsupported();
	void RestoreUnsupported();

	INT NumSpawnedParticles;
	INT LodEmissionBudget;
	FLOAT LodEmissionRemainder;

	struct SpawnEstimateSample
	{
		FLOAT DeltaTime;
		INT NumSpawned;
		FLOAT LifetimeSum;
	};

	TArray<SpawnEstimateSample> SpawnEstimateSamples;
	FLOAT SpawnEstimateTime;
	FLOAT SpawnEstimateRate;
	FLOAT SpawnEstimateLife;

	UBOOL Stored_bUseLocalSpace;
};

/*-----------------------------------------------------------------------------
	ParticleSpritePhysXEmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleSpritePhysXEmitterInstance : public FParticleSpriteSubUVEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpritePhysXEmitterInstance, FParticleSpriteSubUVEmitterInstance);
    
	FParticleSpritePhysXEmitterInstance(class UParticleModuleTypeDataPhysX &TypeData);
	virtual ~FParticleSpritePhysXEmitterInstance();

	class UParticleModuleTypeDataPhysX&		PhysXTypeData;

	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);

	virtual FLOAT Spawn(FLOAT DeltaTime);
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);

	virtual void RemovedFromScene(void);

	/** Executes method of base class, but forces bUseLocalSpace to FALSE */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/** Lifetime particle reduction is performed by FPhysXParticleSet */
	virtual void KillParticles() {}

	/** Particle reduction modules are currently not supported */
	virtual void KillParticle(INT Index) {}

	/** Removes all active particles*/
	void RemoveParticles();

	/** Fast active particle removal (replace with last) */
	void RemoveParticleFromActives(INT Index);

	/** Executes spawning of particles in sync with PhysX SDK */
	void SpawnSyncPhysX();

	/** Provides LOD relevant camera position. TODO: figure out how this should be done properly. */
	UBOOL GetLODOrigin(FVector& OutLODOrigin);

	/** Provides LOD relevant camera near plane. TODO: figure out how this should be done properly. */
	UBOOL GetLODNearClippingPlane(FPlane& OutClippingPlane);

	/** Returns smoothed estimate for rate*lifetime in [#particles]*/
	INT GetSpawnVolumeEstimate();

	/** Sets spawn budget for this emitter instance in [#particles] */
	void SetEmissionBudget(INT Budget);

	/** See ParticleModuleTypeDataPhysX.uc */
	FLOAT GetWeightForSpawnLod();

	FORCEINLINE void SetBounds(const FBox& InBounds) { ParticleBoundingBox = InBounds; }

	INT NumSpawnedParticles;

private:
	
	void CleanUp();
	UBOOL TryConnect();
	class FPhysXParticleSystem& GetPSys();
	FParticleSystemSceneProxy* GetSceneProxy();
	struct FRBVolumeFill* GetVolumeFill();
	void FillVolume( struct FRBVolumeFill & VolumeFill );
	void PostSpawnVerticalLod();
	void AsyncProccessSpawnedParticles(FLOAT DeltaTime, INT FirstIndex);
	void SpawnEstimateUpdate(FLOAT DeltaTime, INT NumSpawnedParticles, FLOAT LifetimeSum);
	void OverwriteUnsupported();
	void RestoreUnsupported();
	
	INT LodEmissionBudget;
	FLOAT LodEmissionRemainder;

	struct SpawnEstimateSample
	{
		FLOAT DeltaTime;
		INT NumSpawned;
		FLOAT LifetimeSum;
	};

	TArray<SpawnEstimateSample> SpawnEstimateSamples;
	FLOAT SpawnEstimateTime;
	FLOAT SpawnEstimateRate;
	FLOAT SpawnEstimateLife;

	UBOOL Stored_bUseLocalSpace;

	class FPhysXParticleSetSprite* PSet;
};
#endif	//#if WITH_NOVODEX

/*-----------------------------------------------------------------------------
	ParticleBeam2EmitterInstance
-----------------------------------------------------------------------------*/
struct FParticleBeam2EmitterInstance : public FParticleEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleBeam2EmitterInstance, FParticleEmitterInstance);

	UParticleModuleTypeDataBeam2*	BeamTypeData;
	UParticleModuleBeamSource*		BeamModule_Source;
	UParticleModuleBeamTarget*		BeamModule_Target;
	UParticleModuleBeamNoise*		BeamModule_Noise;
	UParticleModuleBeamModifier*	BeamModule_SourceModifier;
	INT								BeamModule_SourceModifier_Offset;
	UParticleModuleBeamModifier*	BeamModule_TargetModifier;
	INT								BeamModule_TargetModifier_Offset;

	UBOOL							FirstEmission;
	INT								LastEmittedParticleIndex;
	INT								TickCount;
	INT								ForceSpawnCount;
	/** The method to utilize when forming the beam.							*/
	INT								BeamMethod;
	/** How many times to tile the texture along the beam.						*/
	TArray<INT>						TextureTiles;
	/** The number of live beams												*/
	INT								BeamCount;
	/** The actor to get the source point from.									*/
	AActor*							SourceActor;
	/** The emitter to get the source point from.								*/
	FParticleEmitterInstance*		SourceEmitter;
	/** User set Source points of each beam - primarily for weapon effects.		*/
	TArray<FVector>					UserSetSourceArray;
	/** User set Source tangents of each beam - primarily for weapon effects.	*/
	TArray<FVector>					UserSetSourceTangentArray;
	/** User set Source strengths of each beam - primarily for weapon effects.	*/
	TArray<FLOAT>					UserSetSourceStrengthArray;
	/** The distance of each beam, if utilizing the distance method.			*/
	TArray<FLOAT>					DistanceArray;
	/** The target point of each beam, when using the end point method.			*/
	TArray<FVector>					TargetPointArray;
	/** The target tangent of each beam, when using the end point method.		*/
	TArray<FVector>					TargetTangentArray;
	/** User set Target strengths of each beam - primarily for weapon effects.	*/
	TArray<FLOAT>					UserSetTargetStrengthArray;
	/** The actor to get the target point from.									*/
	AActor*							TargetActor;
	/** The emitter to get the Target point from.								*/
	FParticleEmitterInstance*		TargetEmitter;
	/** The target point sources of each beam, when using the end point method.	*/
	TArray<FName>					TargetPointSourceNames;
	/** User set target points of each beam - primarily for weapon effects.		*/
	TArray<FVector>					UserSetTargetArray;
	/** User set target tangents of each beam - primarily for weapon effects.	*/
	TArray<FVector>					UserSetTargetTangentArray;

	/** The number of vertices and triangles, for rendering						*/
	INT								VertexCount;
	INT								TriangleCount;
	TArray<INT>						BeamTrianglesPerSheet;

	/** Constructor	*/
	FParticleBeam2EmitterInstance();

	/** Destructor	*/
	virtual ~FParticleBeam2EmitterInstance();

	// Accessors
    virtual void SetBeamType(INT NewMethod);
    virtual void SetTessellationFactor(FLOAT NewFactor);
    virtual void SetEndPoint(FVector NewEndPoint);
    virtual void SetDistance(FLOAT Distance);
    virtual void SetSourcePoint(FVector NewSourcePoint,INT SourceIndex);
    virtual void SetSourceTangent(FVector NewTangentPoint,INT SourceIndex);
    virtual void SetSourceStrength(FLOAT NewSourceStrength,INT SourceIndex);
    virtual void SetTargetPoint(FVector NewTargetPoint,INT TargetIndex);
    virtual void SetTargetTangent(FVector NewTangentPoint,INT TargetIndex);
    virtual void SetTargetStrength(FLOAT NewTargetStrength,INT TargetIndex);

	//
	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	virtual void Init();
	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);
	virtual void UpdateBoundingBox(FLOAT DeltaTime);
	virtual UINT RequiredBytes();
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(FBaseParticle* Particle);
	virtual void KillParticles();
			void SetupBeamModules();
			void SetupBeamModifierModules();
			void ResolveSource();
			void ResolveTarget();
			void DetermineVertexAndTriangleCount();

	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData();

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		INT Size = sizeof(FParticleBeam2EmitterInstance);
		INT ActiveParticleDataSize = ActiveParticles * ParticleStride;
		INT MaxActiveParticleDataSize = MaxActiveParticles * ParticleStride;
		INT ActiveParticleIndexSize = 0;
		INT MaxActiveParticleIndexSize = 0;

		InNum = ActiveParticleDataSize + ActiveParticleIndexSize + Size;
		InMax = MaxActiveParticleDataSize + MaxActiveParticleIndexSize + Size;

		// Take dynamic data into account as well
		Size = sizeof(FDynamicBeam2EmitterData);
		Size += MaxActiveParticleDataSize;								// Copy of the particle data on the render thread
		Size += MaxActiveParticles * sizeof(FParticleSpriteVertex);		// The vertex data array

		InNum += Size;
		InMax += Size;
	}


protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );

};

/*-----------------------------------------------------------------------------
	OLD - Get rid of these...
-----------------------------------------------------------------------------*/
struct FParticleBeamEmitterInstance : public FParticleEmitterInstance
{
};

struct FParticleTrailEmitterInstance : public FParticleEmitterInstance
{
};

/*-----------------------------------------------------------------------------
	ParticleTrail2EmitterInstance.
-----------------------------------------------------------------------------*/
struct FParticleTrail2EmitterInstance : public FParticleEmitterInstance
{
	DECLARE_PARTICLEEMITTERINSTANCE_TYPE(FParticleTrail2EmitterInstance, FParticleEmitterInstance);

    UParticleModuleTypeDataTrail2*	TrailTypeData;
    UParticleModuleTrailSource*		TrailModule_Source;
    INT								TrailModule_Source_Offset;
    UParticleModuleTrailSpawn*		TrailModule_Spawn;
    INT								TrailModule_Spawn_Offset;
    UParticleModuleTrailTaper*		TrailModule_Taper;
    INT								TrailModule_Taper_Offset;

	BITFIELD FirstEmission:1;
    INT LastEmittedParticleIndex;
    INT LastSelectedParticleIndex;
    INT TickCount;
    INT ForceSpawnCount;
    INT VertexCount;
    INT TriangleCount;
    INT Tessellation;
    TArray<INT> TextureTiles;
    INT TrailCount;
    INT MaxTrailCount;
    TArray<FLOAT> TrailSpawnTimes;
    TArray<FVector> SourcePosition;
    TArray<FVector> LastSourcePosition;
    TArray<FVector> CurrentSourcePosition;
    TArray<FVector> LastSpawnPosition;
    TArray<FVector> LastSpawnTangent;
    TArray<FLOAT> SourceDistanceTravelled;
    AActor* SourceActor;
    TArray<FVector> SourceOffsets;
    FParticleEmitterInstance* SourceEmitter;
    INT ActuallySpawned;

	/** Constructor	*/
	FParticleTrail2EmitterInstance();

	/** Destructor	*/
	virtual ~FParticleTrail2EmitterInstance();

	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	virtual void Init();
	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);

	virtual void UpdateBoundingBox(FLOAT DeltaTime);

	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(FBaseParticle* Particle);
	virtual void KillParticles();
	
			void SetupTrailModules();
			void ResolveSource();
			void UpdateSourceData(FLOAT DeltaTime);
			void DetermineVertexAndTriangleCount();

	/**
	 *	Retrieves the dynamic data for the emitter
	 */
	virtual FDynamicEmitterDataBase* GetDynamicData(UBOOL bSelected);

	/**
	 *	Updates the dynamic data for the instance
	 *
	 *	@param	DynamicData		The dynamic data to fill in
	 *	@param	bSelected		TRUE if the particle system component is selected
	 */
	virtual UBOOL UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected);

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	virtual FDynamicEmitterReplayDataBase* GetReplayData();

	virtual void GetAllocatedSize(INT& InNum, INT& InMax)
	{
		INT Size = sizeof(FParticleTrail2EmitterInstance);
		INT ActiveParticleDataSize = ActiveParticles * ParticleStride;
		INT MaxActiveParticleDataSize = MaxActiveParticles * ParticleStride;
		INT ActiveParticleIndexSize = ActiveParticles * sizeof(WORD);
		INT MaxActiveParticleIndexSize = MaxActiveParticles * sizeof(WORD);

		InNum = ActiveParticleDataSize + ActiveParticleIndexSize + Size;
		InMax = MaxActiveParticleDataSize + MaxActiveParticleIndexSize + Size;

		// Take dynamic data into account as well
		Size = sizeof(FDynamicTrail2EmitterData);
		Size += MaxActiveParticleDataSize;								// Copy of the particle data on the render thread
		Size += MaxActiveParticleIndexSize;								// Copy of the particle indices on the render thread
		Size += MaxActiveParticles * sizeof(FParticleSpriteVertex);		// The vertex data array

		InNum += Size;
		InMax += Size;
	}


protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns TRUE if successful
	 */
	virtual UBOOL FillReplayData( FDynamicEmitterReplayDataBase& OutData );

};
