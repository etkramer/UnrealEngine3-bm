/*=============================================================================
	ParticleEmitterInstances.cpp: Particle emitter instance implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "LevelUtils.h"

IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteEmitterInstance);
IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteSubUVEmitterInstance);
IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleMeshEmitterInstance);

// Scale the particle bounds by appSqrt(2.0f) / 2.0f
#define PARTICLESYSTEM_BOUNDSSCALAR		0.707107f

/*-----------------------------------------------------------------------------
FParticlesStatGroup
-----------------------------------------------------------------------------*/
DECLARE_STATS_GROUP(TEXT("Particles"),STATGROUP_Particles);

DECLARE_DWORD_COUNTER_STAT(TEXT("Sprite Particles"),STAT_SpriteParticles,STATGROUP_Particles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Sprite Ptcl Render Calls"),STAT_SpriteParticlesRenderCalls,STATGROUP_Particles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Sprite Ptcls Spawned"),STAT_SpriteParticlesSpawned,STATGROUP_Particles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Sprite Ptcls Updated"),STAT_SpriteParticlesUpdated,STATGROUP_Particles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Sprite Ptcls Killed"),STAT_SpriteParticlesKilled,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Sort Time"),STAT_SortingTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Sprite Render Time"),STAT_SpriteRenderingTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Sprite Tick Time"),STAT_SpriteTickTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Sprite Spawn Time"),STAT_SpriteSpawnTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Sprite Update Time"),STAT_SpriteUpdateTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("PSys Comp Tick Time"),STAT_PSysCompTickTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Particle Tick Time"),STAT_ParticleTickTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Particle Render Time"),STAT_ParticleRenderingTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("SetTemplate Time"),STAT_ParticleSetTemplateTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Initialize Time"),STAT_ParticleInitializeTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Activate Time"),STAT_ParticleActivateTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("UpdateInstances Time"),STAT_ParticleUpdateInstancesTime,STATGROUP_Particles);

DECLARE_STATS_GROUP(TEXT("MeshParticles"),STATGROUP_MeshParticles);

DECLARE_DWORD_COUNTER_STAT(TEXT("Mesh Particles"),STAT_MeshParticles,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Mesh Render Time"),STAT_MeshRenderingTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Mesh Tick Time"),STAT_MeshTickTime,STATGROUP_Particles);
DECLARE_CYCLE_STAT(TEXT("Mesh Spawn Time"),STAT_MeshSpawnTime,STATGROUP_MeshParticles);
DECLARE_CYCLE_STAT(TEXT("Mesh Update Time"),STAT_MeshUpdateTime,STATGROUP_MeshParticles);

/*-----------------------------------------------------------------------------
	FParticleEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	ParticleEmitterInstance
 *	The base structure for all emitter instance classes
 */
FParticleEmitterInstanceType FParticleEmitterInstance::StaticType(TEXT("FParticleEmitterInstance"),NULL);

// Only update the PeakActiveParticles if the frame rate is 20 or better
const FLOAT FParticleEmitterInstance::PeakActiveParticleUpdateDelta = 0.05f;

/** Constructor	*/
FParticleEmitterInstance::FParticleEmitterInstance() :
	  SpriteTemplate(NULL)
    , Component(NULL)
    , CurrentLODLevelIndex(0)
    , CurrentLODLevel(NULL)
    , TypeDataOffset(0)
    , SubUVDataOffset(0)
	, DynamicParameterDataOffset(0)
	, OrbitModuleOffset(0)
    , KillOnDeactivate(0)
    , bKillOnCompleted(0)
	, bRequiresSorting(FALSE)
    , ParticleData(NULL)
    , ParticleIndices(NULL)
    , InstanceData(NULL)
    , InstancePayloadSize(0)
    , PayloadOffset(0)
    , ParticleSize(0)
    , ParticleStride(0)
    , ActiveParticles(0)
    , MaxActiveParticles(0)
    , SpawnFraction(0.0f)
    , SecondsSinceCreation(0.0f)
    , EmitterTime(0.0f)
    , LoopCount(0)
	, IsRenderDataDirty(0)
    , Module_AxisLock(NULL)
    , EmitterDuration(0.0f)
	, TrianglesToRender(0)
	, MaxVertexIndex(0)
	, CurrentMaterial(NULL)
	, bUseNxFluid(FALSE)
#if !FINAL_RELEASE
	, EventCount(0)
	, MaxEventCount(0)
#endif	//#if !FINAL_RELEASE
{
}

/** Destructor	*/
FParticleEmitterInstance::~FParticleEmitterInstance()
{
  	appFree(ParticleData);
    appFree(ParticleIndices);
    appFree(InstanceData);
	BurstFired.Empty();
}

/**
 *	Set the KillOnDeactivate flag to the given value
 *
 *	@param	bKill	Value to set KillOnDeactivate to.
 */
void FParticleEmitterInstance::SetKillOnDeactivate(UBOOL bKill)
{
	KillOnDeactivate = bKill;
}

/**
 *	Set the KillOnCompleted flag to the given value
 *
 *	@param	bKill	Value to set KillOnCompleted to.
 */
void FParticleEmitterInstance::SetKillOnCompleted(UBOOL bKill)
{
	bKillOnCompleted = bKill;
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	SpriteTemplate = CastChecked<UParticleSpriteEmitter>(InTemplate);
    Component = InComponent;
	SetupEmitterDuration();
}

/**
 *	Initialize the instance
 */
void FParticleEmitterInstance::Init()
{
	// This assert makes sure that packing is as expected.
	// Added FBaseColor...
	// Linear color change
	// Added Flags field
	check(sizeof(FBaseParticle) == 128);

	// Calculate particle struct size, size and average lifetime.
	ParticleSize = sizeof(FBaseParticle);
	INT	ReqBytes;
	INT ReqInstanceBytes = 0;
	INT TempInstanceBytes;

	UParticleLODLevel* HighLODLevel = SpriteTemplate->GetLODLevel(0);
	check(HighLODLevel);
	UParticleModule* TypeDataModule = HighLODLevel->TypeDataModule;
	if (TypeDataModule)
	{
		ReqBytes = TypeDataModule->RequiredBytes(this);
		if (ReqBytes)
		{
			TypeDataOffset	 = ParticleSize;
			ParticleSize	+= ReqBytes;
		}

		TempInstanceBytes = TypeDataModule->RequiredBytesPerInstance(this);
		if (TempInstanceBytes)
		{
			ReqInstanceBytes += TempInstanceBytes;
		}
	}

	// Set the current material
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	check(LODLevel->RequiredModule);
	CurrentMaterial = LODLevel->RequiredModule->Material;

	// NOTE: This code assumes that the same module order occurs in all LOD levels
	for (INT i = 0; i < LODLevel->Modules.Num(); i++)
	{
		UParticleModule* ParticleModule = LODLevel->Modules(i);
		check(ParticleModule);
		// We always use the HighModule as the look-up in the offset maps...
		UParticleModule* HighModule = HighLODLevel->Modules(i);
		check(HighModule);
		check(HighModule->GetClass() == ParticleModule->GetClass());

		if (ParticleModule->IsA(UParticleModuleTypeDataBase::StaticClass()) == FALSE)
		{
			ReqBytes	= ParticleModule->RequiredBytes(this);
			if (ReqBytes)
			{
				ModuleOffsetMap.Set(HighModule, ParticleSize);
				if (ParticleModule->IsA(UParticleModuleParameterDynamic::StaticClass()) && (DynamicParameterDataOffset == 0))
				{
					DynamicParameterDataOffset = ParticleSize;
				}
				ParticleSize	+= ReqBytes;
			}

			TempInstanceBytes = ParticleModule->RequiredBytesPerInstance(this);
			if (TempInstanceBytes)
			{
				ModuleInstanceOffsetMap.Set(HighModule, ReqInstanceBytes);
				ReqInstanceBytes += TempInstanceBytes;
			}
		}

		if (ParticleModule->IsA(UParticleModuleOrientationAxisLock::StaticClass()))
		{
			Module_AxisLock	= Cast<UParticleModuleOrientationAxisLock>(ParticleModule);
		}
	}

	if ((InstanceData == NULL) || (ReqInstanceBytes > InstancePayloadSize))
	{
		InstanceData = (BYTE*)(appRealloc(InstanceData, ReqInstanceBytes));
		InstancePayloadSize = ReqInstanceBytes;
	}

	appMemzero(InstanceData, InstancePayloadSize);

	for (INT i = 0; i < LODLevel->Modules.Num(); i++)
	{
		UParticleModule* ParticleModule = LODLevel->Modules(i);
		check(ParticleModule);
		BYTE* PrepInstData = this->GetModuleInstanceData(ParticleModule);
		if (PrepInstData)
		{
			ParticleModule->PrepPerInstanceBlock(this, (void*)PrepInstData);
		}
	}

	// Offset into emitter specific payload (e.g. TrailComponent requires extra bytes).
	PayloadOffset = ParticleSize;
	
	// Update size with emitter specific size requirements.
	ParticleSize += RequiredBytes();

	// Make sure everything is at least 16 byte aligned so we can use SSE for FVector.
	ParticleSize = Align(ParticleSize, 16);

	// E.g. trail emitters store trailing particles directly after leading one.
	ParticleStride			= CalculateParticleStride(ParticleSize);

	// Set initial values.
	SpawnFraction			= 0;
	SecondsSinceCreation	= 0;
	
	Location				= Component->LocalToWorld.GetOrigin();
	OldLocation				= Location;
	
	TrianglesToRender		= 0;
	MaxVertexIndex			= 0;

	if (ParticleData == NULL)
	{
		MaxActiveParticles	= 0;
		ActiveParticles		= 0;
	}

	ParticleBoundingBox.Init();
	check(LODLevel->RequiredModule);
	if (LODLevel->RequiredModule->RandomImageChanges == 0)
	{
		LODLevel->RequiredModule->RandomImageTime	= 1.0f;
	}
	else
	{
		LODLevel->RequiredModule->RandomImageTime	= 0.99f / (LODLevel->RequiredModule->RandomImageChanges + 1);
	}

	// Resize to sensible default.
	if (GIsGame == TRUE)
	{
		if ((LODLevel->PeakActiveParticles > 0) || (SpriteTemplate->InitialAllocationCount > 0))
		{
			// In-game... we assume the editor has set this properly, but still clamp at 100 to avoid wasting
			// memory.
			if (SpriteTemplate->InitialAllocationCount > 0)
			{
				Resize(Min( SpriteTemplate->InitialAllocationCount, 100 ));
			}
			else
			{
				Resize(Min( LODLevel->PeakActiveParticles, 100 ));
			}
		}
		else
		{
			// This is to force the editor to 'select' a value
			Resize(10);
		}
	}

	LoopCount = 0;

	// Propagate killon flags
	SetKillOnDeactivate(LODLevel->RequiredModule->bKillOnDeactivate);
	SetKillOnCompleted(LODLevel->RequiredModule->bKillOnCompleted);

	// Propagate sorting flag.
	bRequiresSorting = LODLevel->RequiredModule->bRequiresSorting;
	
	// Reset the burst lists
	if (BurstFired.Num() < SpriteTemplate->LODLevels.Num())
	{
		BurstFired.AddZeroed(SpriteTemplate->LODLevels.Num() - BurstFired.Num());
	}
	for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
	{
		LODLevel = SpriteTemplate->LODLevels(LODIndex);
		check(LODLevel);
		if (BurstFired(LODIndex).BurstFired.Num() < LODLevel->SpawnModule->BurstList.Num())
		{
			BurstFired(LODIndex).BurstFired.AddZeroed(LODLevel->SpawnModule->BurstList.Num() - BurstFired(LODIndex).BurstFired.Num());
		}
	}
	ResetBurstList();

	// Tag it as dirty w.r.t. the renderer
	IsRenderDataDirty	= 1;
}

/**
 *	Resize the particle data array
 *
 *	@param	NewMaxActiveParticles	The new size to use
 *
 *	@return	UBOOL					TRUE if the resize was successful
 */
UBOOL FParticleEmitterInstance::Resize(INT NewMaxActiveParticles, UBOOL bSetMaxActiveCount)
{
	if (GEngine->MaxParticleResize > 0)
	{
		if ((NewMaxActiveParticles < 0) || (NewMaxActiveParticles > GEngine->MaxParticleResize))
		{
			if ((NewMaxActiveParticles < 0) || (NewMaxActiveParticles > GEngine->MaxParticleResizeWarn))
			{
				warnf(TEXT("Emitter::Resize> Invalid NewMaxActive (%d) for Emitter in PSys %s"),
					NewMaxActiveParticles, 
					Component	? 
								Component->Template ? *(Component->Template->GetPathName()) 
													: *(Component->GetName()) 
								:
								TEXT("INVALID COMPONENT"));
			}

			return FALSE;
		}
	}

	if (NewMaxActiveParticles > MaxActiveParticles)
	{
		// Alloc (or realloc) the data array
		// Allocations > 16 byte are always 16 byte aligned so ParticleData can be used with SSE.
		// NOTE: We don't have to zero the memory here... It gets zeroed when grabbed later.
		ParticleData = (BYTE*) appRealloc(ParticleData, ParticleStride * NewMaxActiveParticles);
		check(ParticleData);

		// Allocate memory for indices.
		if (ParticleIndices == NULL)
		{
			// Make sure that we clear all when it is the first alloc
			MaxActiveParticles = 0;
		}
		ParticleIndices	= (WORD*) appRealloc(ParticleIndices, sizeof(WORD) * NewMaxActiveParticles);

		// Fill in default 1:1 mapping.
		for (INT i=MaxActiveParticles; i<NewMaxActiveParticles; i++)
		{
			ParticleIndices[i] = i;
		}

		// Set the max count
		MaxActiveParticles = NewMaxActiveParticles;
	}

	// Set the PeakActiveParticles
	if (bSetMaxActiveCount)
	{
		UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
		check(LODLevel);
		if (MaxActiveParticles > LODLevel->PeakActiveParticles)
		{
			LODLevel->PeakActiveParticles = MaxActiveParticles;
		}
	}

	return TRUE;
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleTickTime);
	SCOPE_CYCLE_COUNTER(STAT_SpriteTickTime);

	check(SpriteTemplate);
	check(SpriteTemplate->LODLevels.Num() > 0);

	// Grab the current LOD level
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);

	// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
	if(Component->bJustAttached)
	{
		Location	= Component->LocalToWorld.GetOrigin();
		OldLocation	= Location;
	}
	else
	{
		// Keep track of location for world- space interpolation and other effects.
		OldLocation	= Location;
		Location	= Component->LocalToWorld.GetOrigin();
	}

	// If this the FirstTime we are being ticked?
	UBOOL bFirstTime = (SecondsSinceCreation > 0.0f) ? FALSE : TRUE;
	SecondsSinceCreation += DeltaTime;

	// Update time within emitter loop.
	UBOOL bValidDuration = FALSE;
	EmitterTime = SecondsSinceCreation;
	if (EmitterDuration > KINDA_SMALL_NUMBER)
	{
		bValidDuration = TRUE;
		EmitterTime = appFmod(SecondsSinceCreation, EmitterDuration);
	}

	// Get the emitter delay time
	FLOAT EmitterDelay = LODLevel->RequiredModule->EmitterDelay;

	// Determine if the emitter has looped
	if (bValidDuration && ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration))
	{
		LoopCount++;
		ResetBurstList();
#if !FINAL_RELEASE
		// Reset the event count each loop...
		if (EventCount > MaxEventCount)
		{
			MaxEventCount = EventCount;
		}
		EventCount = 0;
#endif	//#if !FINAL_RELEASE

		if (LODLevel->RequiredModule->bDurationRecalcEachLoop == TRUE)
		{
			SetupEmitterDuration();
		}
	}

	// Don't delay unless required
	if ((LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && (LoopCount > 0))
	{
		EmitterDelay = 0;
	}

	// 'Reset' the emitter time so that the modules function correctly
	EmitterTime -= EmitterDelay;

	// Kill off any dead particles
	KillParticles();

	// If not suppressing spawning...
	if (!bSuppressSpawning && (EmitterTime >= 0.0f))
	{
		SCOPE_CYCLE_COUNTER(STAT_SpriteSpawnTime);
		// If emitter is not done - spawn at current rate.
		// If EmitterLoops is 0, then we loop forever, so always spawn.
		if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
			(LoopCount < LODLevel->RequiredModule->EmitterLoops) ||
			(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)) ||
			bFirstTime)
		{
            bFirstTime = FALSE;
			SpawnFraction = Spawn(DeltaTime);
		}
	}

	// Reset particle parameters.
	ResetParticleParameters(DeltaTime, STAT_SpriteParticlesUpdated);

	// Update the particles
	SCOPE_CYCLE_COUNTER(STAT_SpriteUpdateTime);

	CurrentMaterial = LODLevel->RequiredModule->Material;

	UParticleModuleTypeDataBase* pkBase = 0;
	if (LODLevel->TypeDataModule)
	{
		pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
		//@todo. Need to track TypeData offset into payload!
		pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Update existing particles (might respawn dying ones).
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
	for (INT ModuleIndex = 0; ModuleIndex < LODLevel->UpdateModules.Num(); ModuleIndex++)
	{
		UParticleModule* HighModule	= LODLevel->UpdateModules(ModuleIndex);
		if (HighModule && HighModule->bEnabled)
		{
			UParticleModule* OffsetModule = HighestLODLevel->UpdateModules(ModuleIndex);
			UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
			HighModule->Update(this, Offset ? *Offset : 0, DeltaTime);
		}
	}

	// Handle the TypeData module
	if (pkBase)
	{
		//@todo. Need to track TypeData offset into payload!
		pkBase->Update(this, TypeDataOffset, DeltaTime);
		pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Update the orbit data...
	UpdateOrbitData(DeltaTime);

	// Calculate bounding box and simulate velocity.
	UpdateBoundingBox(DeltaTime);

	// Invalidate the contents of the vertex/index buffer.
	IsRenderDataDirty = 1;

	// 'Reset' the emitter time so that the delay functions correctly
	EmitterTime += EmitterDelay;

	INC_DWORD_STAT_BY(STAT_SpriteParticles, ActiveParticles);
}

/**
 *	Rewind the instance.
 */
void FParticleEmitterInstance::Rewind()
{
	SecondsSinceCreation = 0;
	EmitterTime = 0;
	LoopCount = 0;
	ResetBurstList();
}

/**
 *	Retrieve the bounding box for the instance
 *
 *	@return	FBox	The bounding box
 */
FBox FParticleEmitterInstance::GetBoundingBox()
{ 
	return ParticleBoundingBox;
}

/**
 *	Update the bounding box for the emitter
 *
 *	@param	DeltaTime		The time slice to use
 */
void FParticleEmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
{
	if (Component)
	{
		// Take component scale into account
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}

		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);

		FLOAT	MaxSizeScale	= 1.0f;
		FVector	NewLocation;
		FLOAT	NewRotation;
		ParticleBoundingBox.Init();
		UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
		check(HighestLODLevel);
		// For each particle, offset the box appropriately 
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			// Do linear integrator and update bounding box
			// Do angular integrator, and wrap result to within +/- 2 PI
			Particle.OldLocation	= Particle.Location;
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)
			{
				if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
				{
					NewLocation	= Particle.Location + (DeltaTime * Particle.Velocity);
				}
				else
				{
					NewLocation	= Particle.Location;
				}
				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					NewRotation = (DeltaTime * Particle.RotationRate) + Particle.Rotation;
				}
				else
				{
					NewRotation	= Particle.Rotation;
				}
			}
			else
			{
				NewLocation	= Particle.Location;
				NewRotation	= Particle.Rotation;
			}

			FVector Size = Particle.Size * Scale;
			MaxSizeScale			= Max(MaxSizeScale, Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.

			Particle.Rotation	 = appFmod(NewRotation, 2.f*(FLOAT)PI);
			Particle.Location	 = NewLocation;

			if (Component->bWarmingUp == FALSE)
			{	
				if (LODLevel->OrbitModules.Num() > 0)
				{
					UParticleModuleOrbit* OrbitModule = HighestLODLevel->OrbitModules(LODLevel->OrbitModules.Num() - 1);
					if (OrbitModule)
					{
						UINT* OrbitOffsetIndex = ModuleOffsetMap.Find(OrbitModule);
						if (OrbitOffsetIndex)
						{
							INT CurrentOffset = *(OrbitOffsetIndex);
							const BYTE* ParticleBase = (const BYTE*)&Particle;
							PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
							const FLOAT Max = OrbitPayload.Offset.GetAbsMax();
							FVector OrbitOffset(Max, Max, Max);
							ParticleBoundingBox += (NewLocation + OrbitOffset);
							ParticleBoundingBox += (NewLocation - OrbitOffset);
						}
					}
				}
				else
				{
					ParticleBoundingBox += NewLocation;
				}
			}
		}

		if (Component->bWarmingUp == FALSE)
		{
			ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale * PARTICLESYSTEM_BOUNDSSCALAR);
			// Transform bounding box into world space if the emitter uses a local space coordinate system.
			if (LODLevel->RequiredModule->bUseLocalSpace) 
			{
				ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
			}
		}
	}
}

/**
 *	Retrieved the per-particle bytes that this emitter type requires.
 *
 *	@return	UINT	The number of required bytes for particles in the instance
 */
UINT FParticleEmitterInstance::RequiredBytes()
{
	// If ANY LOD level has subUV, the size must be taken into account.
	UINT uiBytes = 0;
	UBOOL bHasSubUV = FALSE;
	for (INT LODIndex = 0; (LODIndex < SpriteTemplate->LODLevels.Num()) && !bHasSubUV; LODIndex++)
	{
		// This code assumes that the module stacks are identical across LOD levevls...
		UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(LODIndex);
		
		if (LODLevel)
		{
			EParticleSubUVInterpMethod	InterpolationMethod	= (EParticleSubUVInterpMethod)LODLevel->RequiredModule->InterpolationMethod;
			if (LODIndex > 0)
			{
				if ((InterpolationMethod != PSUVIM_None) && (bHasSubUV == FALSE))
				{
					warnf(TEXT("Emitter w/ mismatched SubUV settings: %s"),
						Component ? 
							Component->Template ? 
								*(Component->Template->GetPathName()) :
								*(Component->GetFullName()) :
							TEXT("INVALID PSYS!"));
				}

				if ((InterpolationMethod == PSUVIM_None) && (bHasSubUV == TRUE))
				{
					warnf(TEXT("Emitter w/ mismatched SubUV settings: %s"),
						Component ? 
						Component->Template ? 
						*(Component->Template->GetPathName()) :
					*(Component->GetFullName()) :
					TEXT("INVALID PSYS!"));
				}
			}
			// Check for SubUV utilization, and update the required bytes accordingly
			if (InterpolationMethod != PSUVIM_None)
			{
				bHasSubUV = TRUE;
			}
		}
	}

	if (bHasSubUV)
	{
		SubUVDataOffset = PayloadOffset;
		uiBytes	= sizeof(FFullSubUVPayload);
	}

	return uiBytes;
}

/**
 *	Get the pointer to the instance data allocated for a given module.
 *
 *	@param	Module		The module to retrieve the data block for.
 *	@return	BYTE*		The pointer to the data
 */
BYTE* FParticleEmitterInstance::GetModuleInstanceData(UParticleModule* Module)
{
	// If there is instance data present, look up the modules offset
	if (InstanceData)
	{
		UINT* Offset = ModuleInstanceOffsetMap.Find(Module);
		if (Offset)
		{
			if (*Offset < (UINT)InstancePayloadSize)
			{
				return &(InstanceData[*Offset]);
			}
		}
	}
	return NULL;
}

/**
 *	Calculate the stride of a single particle for this instance
 *
 *	@param	ParticleSize	The size of the particle
 *
 *	@return	UINT			The stride of the particle
 */
UINT FParticleEmitterInstance::CalculateParticleStride(UINT ParticleSize)
{
	return ParticleSize;
}

/**
 *	Reset the burst list information for the instance
 */
void FParticleEmitterInstance::ResetBurstList()
{
	for (INT BurstIndex = 0; BurstIndex < BurstFired.Num(); BurstIndex++)
	{
		FLODBurstFired* CurrBurstFired = &(BurstFired(BurstIndex));
		for (INT FiredIndex = 0; FiredIndex < CurrBurstFired->BurstFired.Num(); FiredIndex++)
		{
			CurrBurstFired->BurstFired(FiredIndex) = FALSE;
		}
	}
}

/**
 *	Get the current burst rate offset (delta time is artifically increased to generate bursts)
 *
 *	@param	DeltaTime	The time slice (In/Out)
 *	@param	Burst		The number of particles to burst (Output)
 *
 *	@return	FLOAT		The time slice increase to use
 */
FLOAT FParticleEmitterInstance::GetCurrentBurstRateOffset(FLOAT& DeltaTime, INT& Burst)
{
	FLOAT SpawnRateInc = 0.0f;

	// Grab the current LOD level
	UParticleLODLevel* LODLevel	= SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->SpawnModule->BurstList.Num() > 0)
    {
		// For each burst in the list
        for (INT i = 0; i < LODLevel->SpawnModule->BurstList.Num(); i++)
        {
            FParticleBurst* BurstEntry = &(LODLevel->SpawnModule->BurstList(i));
			// If it hasn't been fired
			if ((LODLevel->Level < BurstFired.Num()) && (i < BurstFired(LODLevel->Level).BurstFired.Num()))
			{
				if (BurstFired(LODLevel->Level).BurstFired(i) == FALSE)
				{
					// If it is time to fire it
					if (EmitterTime >= BurstEntry->Time)
					{
						// Make sure there is a valid time slice
						if (DeltaTime < 0.00001f)
						{
							DeltaTime = 0.00001f;
						}
						// Calculate the increase time slice
						INT Count = BurstEntry->Count;
						if (BurstEntry->CountLow > -1)
						{
							Count = BurstEntry->CountLow + appRound(appSRand() * (FLOAT)(BurstEntry->Count - BurstEntry->CountLow));
						}
						SpawnRateInc += Count / DeltaTime;
						Burst += Count;
						BurstFired(LODLevel->Level).BurstFired(i)	= TRUE;
					}
				}
			}
        }
   }

	return SpawnRateInc;
}

/**
 *	Reset the particle parameters
 */
void FParticleEmitterInstance::ResetParticleParameters(FLOAT DeltaTime, DWORD StatId)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
	check(HighestLODLevel);

	INT OrbitCount = LODLevel->OrbitModules.Num();
	for (INT ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
		Particle.Velocity		= Particle.BaseVelocity;
		Particle.Size			= Particle.BaseSize;
		Particle.RotationRate	= Particle.BaseRotationRate;
		Particle.Color			= Particle.BaseColor;
		Particle.RelativeTime	+= Particle.OneOverMaxLifetime * DeltaTime;

		if (OrbitCount > 0)
		{
			for (INT OrbitIndex = 0; OrbitIndex < OrbitCount; OrbitIndex++)
			{
				UParticleModuleOrbit* OrbitModule = HighestLODLevel->OrbitModules(OrbitIndex);
				if (OrbitModule)
				{
					UINT* OrbitOffset = ModuleOffsetMap.Find(OrbitModule);
					if (OrbitOffset)
					{
						INT CurrentOffset = *(OrbitOffset);
						const BYTE* ParticleBase = (const BYTE*)&Particle;
						PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
						if (OrbitIndex < (OrbitCount - 1))
						{
							OrbitPayload.PreviousOffset = OrbitPayload.Offset;
						}
						OrbitPayload.Offset = OrbitPayload.BaseOffset;
						OrbitPayload.RotationRate = OrbitPayload.BaseRotationRate;
					}
				}
			}
		}
	}
	INC_DWORD_STAT_BY(StatId, ActiveParticles);
}

/**
 *	Calculate the orbit offset data.
 */
void FParticleEmitterInstance::CalculateOrbitOffset(FOrbitChainModuleInstancePayload& Payload, 
	FVector& AccumOffset, FVector& AccumRotation, FVector& AccumRotationRate, 
	FLOAT DeltaTime, FVector& Result, FMatrix& RotationMat)
{
	AccumRotation += AccumRotationRate * DeltaTime;
	Payload.Rotation = AccumRotation;
	if (AccumRotation.IsNearlyZero() == FALSE)
	{
		FVector RotRot = RotationMat.TransformNormal(AccumRotation);
		FVector ScaledRotation = RotRot * 360.0f;
		FRotator Rotator = FRotator::MakeFromEuler(ScaledRotation);
		FMatrix RotMat = FRotationMatrix(Rotator);

		RotationMat *= RotMat;

		Result = RotationMat.TransformFVector(AccumOffset);
	}
	else
	{
		Result = AccumOffset;
	}

	AccumOffset.X = 0.0f;;
	AccumOffset.Y = 0.0f;;
	AccumOffset.Z = 0.0f;;
	AccumRotation.X = 0.0f;;
	AccumRotation.Y = 0.0f;;
	AccumRotation.Z = 0.0f;;
	AccumRotationRate.X = 0.0f;;
	AccumRotationRate.Y = 0.0f;;
	AccumRotationRate.Z = 0.0f;;
}

void FParticleEmitterInstance::UpdateOrbitData(FLOAT DeltaTime)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
	check(HighestLODLevel);

	INT ModuleCount = LODLevel->OrbitModules.Num();
	if (ModuleCount > 0)
	{
		TArray<FVector> Offsets;
		Offsets.AddZeroed(ModuleCount + 1);

		TArray<INT> ModuleOffsets;
		ModuleOffsets.AddZeroed(ModuleCount + 1);
		for (INT ModOffIndex = 0; ModOffIndex < ModuleCount; ModOffIndex++)
		{
			UParticleModuleOrbit* HighestOrbitModule = HighestLODLevel->OrbitModules(ModOffIndex);
			check(HighestOrbitModule);

			UINT* ModuleOffset = ModuleOffsetMap.Find(HighestOrbitModule);
			if (ModuleOffset == NULL)
			{
				ModuleOffsets(ModOffIndex) = 0;
			}
			else
			{
				ModuleOffsets(ModOffIndex) = (INT)(*ModuleOffset);
			}
		}

		for(INT i=ActiveParticles-1; i>=0; i--)
		{
			INT OffsetIndex = 0;
			const INT	CurrentIndex	= ParticleIndices[i];
			const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)
			{
				FVector AccumulatedOffset = FVector(0.0f);
				FVector AccumulatedRotation = FVector(0.0f);
				FVector AccumulatedRotationRate = FVector(0.0f);

				FOrbitChainModuleInstancePayload* LocalOrbitPayload = NULL;
				FOrbitChainModuleInstancePayload* PrevOrbitPayload = NULL;
				BYTE PrevOrbitChainMode = 0;
				FMatrix AccumRotMatrix;
				AccumRotMatrix.SetIdentity();

				INT CurrentAccumCount = 0;

				for (INT OrbitIndex = 0; OrbitIndex < ModuleCount; OrbitIndex++)
				{
					INT CurrentOffset = ModuleOffsets(OrbitIndex);
					UParticleModuleOrbit* OrbitModule = LODLevel->OrbitModules(OrbitIndex);
					check(OrbitModule);

					if (CurrentOffset == 0)
					{
						continue;
					}

					PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);

					// The last orbit module holds the last final offset position
					UBOOL bCalculateOffset = FALSE;
					if (OrbitIndex == (ModuleCount - 1))
					{
						LocalOrbitPayload = &OrbitPayload;
						bCalculateOffset = TRUE;
					}

					// Determine the offset, rotation, rotationrate for the current particle
					if (OrbitModule->ChainMode == EOChainMode_Add)
					{
						if (OrbitModule->bEnabled == TRUE)
						{
							AccumulatedOffset += OrbitPayload.Offset;
							AccumulatedRotation += OrbitPayload.Rotation;
							AccumulatedRotationRate += OrbitPayload.RotationRate;
						}
					}
					else
					if (OrbitModule->ChainMode == EOChainMode_Scale)
					{
						if (OrbitModule->bEnabled == TRUE)
						{
							AccumulatedOffset *= OrbitPayload.Offset;
							AccumulatedRotation *= OrbitPayload.Rotation;
							AccumulatedRotationRate *= OrbitPayload.RotationRate;
						}
					}
					else
					if (OrbitModule->ChainMode == EOChainMode_Link)
					{
						if ((OrbitIndex > 0) && (PrevOrbitChainMode == EOChainMode_Link))
						{
							// Calculate the offset with the current accumulation
							FVector ResultOffset;
							CalculateOrbitOffset(*PrevOrbitPayload, 
								AccumulatedOffset, AccumulatedRotation, AccumulatedRotationRate, 
								DeltaTime, ResultOffset, AccumRotMatrix);
							if (OrbitModule->bEnabled == FALSE)
							{
								AccumulatedOffset = FVector(0.0f);
								AccumulatedRotation = FVector(0.0f);
								AccumulatedRotationRate = FVector(0.0f);
							}
							Offsets(OffsetIndex++) = ResultOffset;
						}

						if (OrbitModule->bEnabled == TRUE)
						{
							AccumulatedOffset = OrbitPayload.Offset;
							AccumulatedRotation = OrbitPayload.Rotation;
							AccumulatedRotationRate = OrbitPayload.RotationRate;
						}
					}

					if (bCalculateOffset == TRUE)
					{
						// Push the current offset into the array
						FVector ResultOffset;
						CalculateOrbitOffset(OrbitPayload, 
							AccumulatedOffset, AccumulatedRotation, AccumulatedRotationRate, 
							DeltaTime, ResultOffset, AccumRotMatrix);
						Offsets(OffsetIndex++) = ResultOffset;
					}

					if (OrbitModule->bEnabled)
					{
						PrevOrbitPayload = &OrbitPayload;
						PrevOrbitChainMode = OrbitModule->ChainMode;
					}
				}

				if (LocalOrbitPayload != NULL)
				{
					LocalOrbitPayload->Offset = FVector(0.0f);
					for (INT AccumIndex = 0; AccumIndex < OffsetIndex; AccumIndex++)
					{
						LocalOrbitPayload->Offset += Offsets(AccumIndex);
					}

					appMemzero(Offsets.GetData(), sizeof(FVector) * (ModuleCount + 1));
				}
			}
		}
	}
}

void FParticleEmitterInstance::ParticlePrefetch()
{
	for (INT ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
	{
		PARTICLE_INSTANCE_PREFETCH(this, ParticleIndex);
	}
}

/**
 *	Spawn particles for this emitter instance
 *
 *	@param	DeltaTime		The time slice to spawn over
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleEmitterInstance::Spawn(FLOAT DeltaTime)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	check(LODLevel->RequiredModule);

	// For beams, we probably want to ignore the SpawnRate distribution,
	// and focus strictly on the BurstList...
	FLOAT SpawnRate = 0.0f;
	INT SpawnCount = 0;
	INT BurstCount = 0;
	FLOAT SpawnRateDivisor = 0.0f;
	FLOAT OldLeftover = SpawnFraction;

	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);

	UBOOL bProcessSpawnRate = TRUE;
	UBOOL bProcessBurstList = TRUE;

	// Process all Spawning modules that are present in the emitter.
	for (INT SpawnModIndex = 0; SpawnModIndex < LODLevel->SpawningModules.Num(); SpawnModIndex++)
	{
		UParticleModuleSpawnBase* SpawnModule = LODLevel->SpawningModules(SpawnModIndex);
		if (SpawnModule && SpawnModule->bEnabled)
		{
			UParticleModule* OffsetModule = HighestLODLevel->SpawningModules(SpawnModIndex);
			UINT* Offset = ModuleOffsetMap.Find(OffsetModule);

			// Update the spawn rate
			INT Number = 0;
			FLOAT Rate = 0.0f;
			if (SpawnModule->GetSpawnAmount(this, Offset ? *Offset : 0, OldLeftover, DeltaTime, Number, Rate) == FALSE)
			{
				bProcessSpawnRate = FALSE;
			}

			SpawnCount += Number;
			SpawnRate += Rate;
			// Update the burst list
			INT BurstNumber = 0;
			if (SpawnModule->GetBurstCount(this, Offset ? *Offset : 0, OldLeftover, DeltaTime, BurstNumber) == FALSE)
			{
				bProcessBurstList = FALSE;
			}

			BurstCount += BurstNumber;
		}
	}

	// Figure out spawn rate for this tick.
	if (bProcessSpawnRate)
	{
		FLOAT RateScale = LODLevel->SpawnModule->RateScale.GetValue(EmitterTime, Component);
		SpawnRate += LODLevel->SpawnModule->Rate.GetValue(EmitterTime, Component) * Clamp<FLOAT>(RateScale, 0.0f, RateScale);
	}

	// Take Bursts into account as well...
	if (bProcessBurstList)
	{
		INT Burst = 0;
		FLOAT BurstTime = GetCurrentBurstRateOffset(DeltaTime, Burst);
		BurstCount += Burst;
	}

	// Spawn new particles...
	if ((SpawnRate > 0.f) || (BurstCount > 0))
	{
		FLOAT SafetyLeftover = OldLeftover;
		// Ensure continous spawning... lots of fiddling.
		FLOAT	NewLeftover = OldLeftover + DeltaTime * SpawnRate;
		INT		Number		= appFloor(NewLeftover);
		FLOAT	Increment	= (SpawnRate > 0.0f) ? (1.f / SpawnRate) : 0.0f;
		FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
		NewLeftover			= NewLeftover - Number;

		// Handle growing arrays.
		UBOOL bProcessSpawn = TRUE;
		INT NewCount = ActiveParticles + Number + BurstCount;

#if 0
		//@todo.SAS. Check something similar to MaxDrawCount here...
		if (LODLevel->RequiredModule && LODLevel->RequiredModule->bUseMaxDrawCount)
		{
			if (NewCount > LODLevel->RequiredModule->MaxDrawCount)
			{
				NewCount = LODLevel->RequiredModule->MaxDrawCount;
			}
		}
#endif
		if ((GEngine->MaxParticleVertexMemory > 0) && Component && (Component->bSkipSpawnCountCheck == FALSE))
		{
			INT MaxCount;
			if (LODLevel->RequiredModule->InterpolationMethod == PSUVIM_None)
			{
				check(GEngine->MaxParticleSpriteCount > 0);
				MaxCount = GEngine->MaxParticleSpriteCount;
			}
			else
			{
				check(GEngine->MaxParticleSubUVCount > 0);
				MaxCount = GEngine->MaxParticleSubUVCount;
			}
			if (NewCount > MaxCount)
			{
#if !FINAL_RELEASE
				INT SizeScalar;
				if (LODLevel->RequiredModule->InterpolationMethod == PSUVIM_None)
				{
					SizeScalar = sizeof(FParticleSpriteVertex);
				}
				else
				{
					SizeScalar = sizeof(FParticleSpriteSubUVVertex);
				}
				AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo() : NULL;
				if (WorldInfo)
				{
					FString ErrorMessage = 
						FString::Printf(TEXT("Emitter spawn vertices: %10d (%8.3f kB of verts), clamp to %10d (%8.3f kB): %s"),
							NewCount, 
							(FLOAT)(NewCount * 4 * SizeScalar) / 1024.0f,
							MaxCount, 
							(FLOAT)(MaxCount * 4 * SizeScalar) / 1024.0f,
							Component ? 
								Component->Template ? 
									*(Component->Template->GetName()) :
									TEXT("No template") :
								TEXT("No component"));
					FColor ErrorColor(255,255,0);
					if (WorldInfo->OnScreenDebugMessageExists(0x8000000 | (INT)this) == FALSE)
					{
						debugf(*ErrorMessage);
					}
					WorldInfo->AddOnScreenDebugMessage(0x8000000 | (INT)this, 5.0f, ErrorColor,ErrorMessage);
				}
#endif	//#if !FINAL_RELEASE
				NewCount = MaxCount;
				INT NewParticleCount = MaxCount - ActiveParticles;
				// Burst gets priority
				if ((NewParticleCount > 0) && (BurstCount > 0))
				{
					BurstCount = Min<INT>(NewParticleCount, BurstCount);
					BurstCount = Clamp<INT>(BurstCount, 0, NewParticleCount);
					NewParticleCount -= BurstCount;
				}
				else
				{
					BurstCount = 0;
				}
				if ((NewParticleCount > 0) && (Number > 0))
				{
					Number = Min<INT>(NewParticleCount, Number);
					Number = Clamp<INT>(Number, 0, NewParticleCount);
				}
				else
				{
					Number = 0;
				}
			}
		}

		if (NewCount >= MaxActiveParticles)
		{
			if (DeltaTime < PeakActiveParticleUpdateDelta)
			{
				bProcessSpawn = Resize(NewCount + appTrunc(appSqrt(appSqrt((FLOAT)NewCount)) + 1));
			}
			else
			{
				bProcessSpawn = Resize((NewCount + appTrunc(appSqrt(appSqrt((FLOAT)NewCount)) + 1)), FALSE);
			}
		}

		if (bProcessSpawn == TRUE)
		{
			FParticleEventInstancePayload* EventPayload = NULL;
			if (LODLevel->EventGenerator)
			{
				EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
				if (EventPayload && (EventPayload->bSpawnEventsPresent == FALSE))
				{
					EventPayload = NULL;
				}
			}

			// Spawn particles.
			UParticleLODLevel* HighestLODLevel2 = SpriteTemplate->LODLevels(0);
			for (INT i=0; i<Number; i++)
			{
				check(ActiveParticles <= MaxActiveParticles);
				DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);

				FLOAT SpawnTime = StartTime - i * Increment;

				PreSpawn(Particle);

				if (LODLevel->TypeDataModule)
				{
					UParticleModuleTypeDataBase* pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
					pkBase->Spawn(this, TypeDataOffset, SpawnTime);
				}

				for (INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
				{
					UParticleModule* SpawnModule	= LODLevel->SpawnModules(ModuleIndex);
					if (SpawnModule->bEnabled)
					{
						UParticleModule* OffsetModule	= HighestLODLevel2->SpawnModules(ModuleIndex);
						UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
						SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
					}
				}
				PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

				ActiveParticles++;

				if (EventPayload)
				{
					LODLevel->EventGenerator->HandleParticleSpawned(this, EventPayload, Particle);
				}

				INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
			}

			// Burst particles.
			for (INT BurstIndex = 0; BurstIndex < BurstCount; BurstIndex++)
			{
				check(ActiveParticles <= MaxActiveParticles);
				DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);

				FLOAT SpawnTime = 0.0f;

				PreSpawn(Particle);

				if (LODLevel->TypeDataModule)
				{
					UParticleModuleTypeDataBase* pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
					pkBase->Spawn(this, TypeDataOffset, SpawnTime);
				}

				for (INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
				{
					UParticleModule* SpawnModule	= LODLevel->SpawnModules(ModuleIndex);
					if (SpawnModule->bEnabled)
					{
						UParticleLODLevel* HighestLODLevel2 = SpriteTemplate->LODLevels(0);
						UParticleModule* OffsetModule	= 	HighestLODLevel2->SpawnModules(ModuleIndex);
						UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
						SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
					}
				}
				PostSpawn(Particle, 0.0f, SpawnTime);

				ActiveParticles++;

				if (EventPayload)
				{
					LODLevel->EventGenerator->HandleParticleSpawned(this, EventPayload, Particle);
				}

				INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
			}

			return NewLeftover;
		}
		return SafetyLeftover;
	}

	return SpawnFraction;
}

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
void FParticleEmitterInstance::ForceSpawn(FLOAT DeltaTime, INT InSpawnCount, INT InBurstCount, 
	FVector& InLocation, FVector& InVelocity)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);

	// For beams, we probably want to ignore the SpawnRate distribution,
	// and focus strictly on the BurstList...
	INT SpawnCount = InSpawnCount;
	INT BurstCount = InBurstCount;
	FLOAT SpawnRateDivisor = 0.0f;
	FLOAT OldLeftover = 0.0f;

	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);

	UBOOL bProcessSpawnRate = TRUE;
	UBOOL bProcessBurstList = TRUE;

	// Spawn new particles...
	if ((SpawnCount > 0) || (BurstCount > 0))
	{
		INT		Number		= SpawnCount;
		FLOAT	Increment	= (SpawnCount > 0) ? (DeltaTime / SpawnCount) : 0;
		FLOAT	StartTime	= DeltaTime;
		
		// Handle growing arrays.
		UBOOL bProcessSpawn = TRUE;
		INT NewCount = ActiveParticles + Number + BurstCount;
		if (NewCount >= MaxActiveParticles)
		{
			if (DeltaTime < PeakActiveParticleUpdateDelta)
			{
				bProcessSpawn = Resize(NewCount + appTrunc(appSqrt(appSqrt((FLOAT)NewCount)) + 1));
			}
			else
			{
				bProcessSpawn = Resize((NewCount + appTrunc(appSqrt(appSqrt((FLOAT)NewCount)) + 1)), FALSE);
			}
		}

		if (bProcessSpawn == TRUE)
		{
/***
			//@todo.SAS. If we are allowing events-->process-->events, then this goes back in!
			FParticleEventInstancePayload* EventPayload = NULL;
			if (LODLevel->EventGenerator)
			{
				EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
				if (EventPayload && (EventPayload->bSpawnEventsPresent == FALSE))
				{
					EventPayload = NULL;
				}
			}
***/
			// Spawn particles.
			UParticleLODLevel* HighestLODLevel2 = SpriteTemplate->LODLevels(0);
			for (INT i=0; i<Number; i++)
			{
				check(ActiveParticles <= MaxActiveParticles);
				DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);

				FLOAT SpawnTime = StartTime - i * Increment;

				PreSpawn(Particle);

				// Override the location and velocity!
				Particle->Location = InLocation;
				Particle->BaseVelocity = InVelocity;
				Particle->Velocity = InVelocity;

				if (LODLevel->TypeDataModule)
				{
					UParticleModuleTypeDataBase* pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
					pkBase->Spawn(this, TypeDataOffset, SpawnTime);
				}

				for (INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
				{
					UParticleModule* SpawnModule	= LODLevel->SpawnModules(ModuleIndex);
					UParticleModule* OffsetModule	= HighestLODLevel2->SpawnModules(ModuleIndex);
					UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
					if (SpawnModule->bEnabled)
					{
						SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
					}
				}
				PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

				ActiveParticles++;

/***
			//@todo.SAS. If we are allowing events-->process-->events, then this goes back in!
				if (EventPayload)
				{
					LODLevel->EventGenerator->HandleParticleSpawned(this, EventPayload, Particle);
				}
***/
				INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
			}

			// Burst particles.
			for (INT BurstIndex = 0; BurstIndex < BurstCount; BurstIndex++)
			{
				check(ActiveParticles <= MaxActiveParticles);
				DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);

				FLOAT SpawnTime = 0.0f;

				PreSpawn(Particle);	

				// Override the location and velocity!
				Particle->Location = InLocation;
				Particle->BaseVelocity = InVelocity;
				Particle->Velocity = InVelocity;

				if (LODLevel->TypeDataModule)
				{
					UParticleModuleTypeDataBase* pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
					pkBase->Spawn(this, TypeDataOffset, SpawnTime);
				}

				for (INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
				{
					UParticleModule* SpawnModule	= LODLevel->SpawnModules(ModuleIndex);

					UParticleLODLevel* HighestLODLevel2 = SpriteTemplate->LODLevels(0);
					UParticleModule* OffsetModule	= 	HighestLODLevel2->SpawnModules(ModuleIndex);
					UINT* Offset = ModuleOffsetMap.Find(OffsetModule);

					if (SpawnModule->bEnabled)
					{
						SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
					}
				}
				PostSpawn(Particle, 0.0f, SpawnTime);

				ActiveParticles++;

/***
			//@todo.SAS. If we are allowing events-->process-->events, then this goes back in!
				if (EventPayload)
				{
					LODLevel->EventGenerator->HandleParticleSpawned(this, EventPayload, Particle);
				}
***/
				INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
			}
		}
	}
}

/**
 *	Spawn particles for this instance
 *
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleEmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	FLOAT SafetyLeftover = OldLeftover;
	// Ensure continous spawning... lots of fiddling.
	FLOAT	NewLeftover = OldLeftover + DeltaTime * Rate;
	INT		Number		= appFloor(NewLeftover);
	FLOAT	Increment	= 1.f / Rate;
	FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
	NewLeftover			= NewLeftover - Number;

	// If we have calculated less than the burst count, force the burst count
	if (Number < Burst)
	{
		Number = Burst;
	}

	// Take the burst time fakery into account
	if (BurstTime > 0.0f)
	{
		NewLeftover -= BurstTime / Burst;
		NewLeftover	= Clamp<FLOAT>(NewLeftover, 0, NewLeftover);
	}

	// Handle growing arrays.
	UBOOL bProcessSpawn = TRUE;
	INT NewCount = ActiveParticles + Number;
	if (NewCount >= MaxActiveParticles)
	{
		if (DeltaTime < PeakActiveParticleUpdateDelta)
		{
			bProcessSpawn = Resize(NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1);
		}
		else
		{
			bProcessSpawn = Resize((NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1), FALSE);
		}
	}

	if (bProcessSpawn == TRUE)
	{
		UParticleLODLevel* LODLevel	= SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);
		// Spawn particles.
		for (INT i=0; i<Number; i++)
		{
			check(ActiveParticles <= MaxActiveParticles);
			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);

			FLOAT SpawnTime = StartTime - i * Increment;
		
			PreSpawn(Particle);

			if (LODLevel->TypeDataModule)
			{
				UParticleModuleTypeDataBase* pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
				pkBase->Spawn(this, TypeDataOffset, SpawnTime);
			}

			for (INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
			{
				UParticleModule* SpawnModule	= LODLevel->SpawnModules(ModuleIndex);

				UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
				UParticleModule* OffsetModule	= 	HighestLODLevel->SpawnModules(ModuleIndex);
				UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
				
				if (SpawnModule->bEnabled)
				{
					SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
				}
			}
			PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

			ActiveParticles++;
		}
		INC_DWORD_STAT_BY(STAT_SpriteParticlesSpawned,Number);

		return NewLeftover;
	}

	return SafetyLeftover;
}

/**
 *	Handle any pre-spawning actions required for particles
 *
 *	@param	Particle	The particle being spawned.
 */
void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle)
{
	check(Particle);
	// This isn't a problem w/ the appMemzero call - it's a problem in general!
	check(ParticleSize > 0);

	// By default, just clear out the particle
	appMemzero(Particle, ParticleSize);
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
	{
		// If not using local space, initialize the particle location
		Particle->Location = Location;
	}
}

/**
 *	Has the instance completed it's run?
 *
 *	@return	UBOOL	TRUE if the instance is completed, FALSE if not
 */
UBOOL FParticleEmitterInstance::HasCompleted()
{
	// Validity check
	if (SpriteTemplate == NULL)
	{
		return TRUE;
	}

	// If it hasn't finished looping or if it loops forever, not completed.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
		(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)))
	{
		return FALSE;
	}

	// If there are active particles, not completed
	if (ActiveParticles > 0)
	{
		return FALSE;
	}

	return TRUE;
}

/**
 *	Handle any post-spawning actions required by the instance
 *
 *	@param	Particle					The particle that was spawned
 *	@param	InterpolationPercentage		The percentage of the time slice it was spawned at
 *	@param	SpawnTIme					The time it was spawned at
 */
void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime)
{
	// Interpolate position if using world space.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
	{
		if (FDistSquared(OldLocation, Location) > 1.f)
		{
			Particle->Location += InterpolationPercentage * (OldLocation - Location);	
		}
	}

	// Offset caused by any velocity
	Particle->OldLocation = Particle->Location;
	Particle->Location   += SpawnTime * Particle->Velocity;
}

/**
 *	Kill off any dead particles. (Remove them from the active array)
 */
void FParticleEmitterInstance::KillParticles()
{
	if (ActiveParticles > 0)
	{
		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);
		FParticleEventInstancePayload* EventPayload = NULL;
		if (LODLevel->EventGenerator)
		{
			EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
			if (EventPayload && (EventPayload->bDeathEventsPresent == FALSE))
			{
				EventPayload = NULL;
			}
		}

		// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
		// move them to the 'end' of the active particle list.
		for (INT i=ActiveParticles-1; i>=0; i--)
		{
			const INT	CurrentIndex	= ParticleIndices[i];
			const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
			if (Particle.RelativeTime > 1.0f)
			{
				if (EventPayload)
				{
					LODLevel->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
				}
				// Move it to the 'back' of the list
				ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
				ParticleIndices[ActiveParticles-1]	= CurrentIndex;
				ActiveParticles--;

				INC_DWORD_STAT(STAT_SpriteParticlesKilled);
			}
		}
	}
}

/**
 *	Kill the particle at the given instance
 *
 *	@param	Index		The index of the particle to kill.
 */
void FParticleEmitterInstance::KillParticle(INT Index)
{
	if (Index < ActiveParticles)
	{
		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);
		FParticleEventInstancePayload* EventPayload = NULL;
		if (LODLevel->EventGenerator)
		{
			EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
			if (EventPayload && (EventPayload->bDeathEventsPresent == FALSE))
			{
				EventPayload = NULL;
			}
		}

		INT KillIndex = ParticleIndices[Index];

		// Handle the kill event, if needed
		if (EventPayload)
		{
			const BYTE* ParticleBase	= ParticleData + KillIndex * ParticleStride;
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
			LODLevel->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
		}

		// Move it to the 'back' of the list
		for (INT i=Index; i < ActiveParticles - 1; i++)
		{
			ParticleIndices[i] = ParticleIndices[i+1];
		}
		ParticleIndices[ActiveParticles-1] = KillIndex;
		ActiveParticles--;

		INC_DWORD_STAT(STAT_SpriteParticlesKilled);
	}
}

/**
 *	This is used to force "kill" particles irrespective of their duration.
 *	Basically, this takes all particles and moves them to the 'end' of the 
 *	particle list so we can insta kill off trailed particles in the level.
 */
void FParticleEmitterInstance::KillParticlesForced()
{
	// Loop over the active particles and kill them.
	// Move them to the 'end' of the active particle list.
	for (INT i=ActiveParticles-1; i>=0; i--)
	{
		const INT	CurrentIndex	= ParticleIndices[i];
		ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
		ParticleIndices[ActiveParticles-1]	= CurrentIndex;
		ActiveParticles--;

		INC_DWORD_STAT(STAT_SpriteParticlesKilled);
	}
}

/**
 *	Called when the instance if removed from the scene
 *	Perform any actions required, such as removing components, etc.
 */
void FParticleEmitterInstance::RemovedFromScene()
{
}

/**
 *	Retrieve the particle at the given index
 *
 *	@param	Index			The index of the particle of interest
 *
 *	@return	FBaseParticle*	The pointer to the particle. NULL if not present/active
 */
FBaseParticle* FParticleEmitterInstance::GetParticle(INT Index)
{
	// See if the index is valid. If not, return NULL
	if ((Index >= ActiveParticles) || (Index < 0))
	{
		return NULL;
	}

	// Grab and return the particle
	DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[Index]);
	return Particle;
}

/**
 *	Calculates the emitter duration for the instance.
 */
void FParticleEmitterInstance::SetupEmitterDuration()
{
	// Validity check
	if (SpriteTemplate == NULL)
	{
		return;
	}

	// Set up the array for each LOD level
	INT EDCount = EmitterDurations.Num();
	if ((EDCount == 0) || (EDCount != SpriteTemplate->LODLevels.Num()))
	{
		EmitterDurations.Empty();
		EmitterDurations.Insert(0, SpriteTemplate->LODLevels.Num());
	}

	// Calculate the duration for each LOD level
	for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels(LODIndex);
		UParticleModuleRequired* RequiredModule = TempLOD->RequiredModule;
		if (RequiredModule->bEmitterDurationUseRange)
		{
			FLOAT	Rand		= appFrand();
			FLOAT	Duration	= RequiredModule->EmitterDurationLow + 
				((RequiredModule->EmitterDuration - RequiredModule->EmitterDurationLow) * Rand);
			EmitterDurations(TempLOD->Level) = Duration + RequiredModule->EmitterDelay;
		}
		else
		{
			EmitterDurations(TempLOD->Level) = RequiredModule->EmitterDuration + RequiredModule->EmitterDelay;
		}

		if ((LoopCount == 1) && (RequiredModule->bDelayFirstLoopOnly == TRUE) && 
			((RequiredModule->EmitterLoops == 0) || (RequiredModule->EmitterLoops > 1)))
		{
			EmitterDurations(TempLOD->Level) -= RequiredModule->EmitterDelay;
		}
	}

	// Set the current duration
	EmitterDuration	= EmitterDurations(CurrentLODLevelIndex);
}

/**
 *	Process received events.
 */
void FParticleEmitterInstance::ProcessEvents(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->EventReceiverModules.Num() > 0)
	{
		for (INT EventModIndex = 0; EventModIndex < LODLevel->EventReceiverModules.Num(); EventModIndex++)
		{
			INT EventIndex;
			UParticleModuleEventReceiverBase* EventRcvr = LODLevel->EventReceiverModules(EventModIndex);
			check(EventRcvr);

			if (EventRcvr->WillProcessEvent(EPET_Spawn) && (Component->SpawnEvents.Num() > 0))
			{
				for (EventIndex = 0; EventIndex < Component->SpawnEvents.Num(); EventIndex++)
				{
					EventRcvr->ProcessEvent(this, Component->SpawnEvents(EventIndex), DeltaTime);
				}
			}

			if (EventRcvr->WillProcessEvent(EPET_Death) && (Component->DeathEvents.Num() > 0))
			{
				for (EventIndex = 0; EventIndex < Component->DeathEvents.Num(); EventIndex++)
				{
					EventRcvr->ProcessEvent(this, Component->DeathEvents(EventIndex), DeltaTime);
				}
			}

			if (EventRcvr->WillProcessEvent(EPET_Collision) && (Component->CollisionEvents.Num() > 0))
			{
				for (EventIndex = 0; EventIndex < Component->CollisionEvents.Num(); EventIndex++)
				{
					EventRcvr->ProcessEvent(this, Component->CollisionEvents(EventIndex), DeltaTime);
				}
			}

			if (EventRcvr->WillProcessEvent(EPET_Kismet) && (Component->KismetEvents.Num() > 0))
			{
				for (EventIndex = 0; EventIndex < Component->KismetEvents.Num(); EventIndex++)
				{
					EventRcvr->ProcessEvent(this, Component->KismetEvents(EventIndex), DeltaTime);
				}
			}
		}
	}
}


/**
 * Captures dynamic replay data for this particle system.
 *
 * @param	OutData		[Out] Data will be copied here
 *
 * @return Returns TRUE if successful
 */
UBOOL FParticleEmitterInstance::FillReplayData( FDynamicEmitterReplayDataBase& OutData )
{
	// NOTE: This the base class implementation that should ONLY be called by derived classes' FillReplayData()!

	// Make sure there is a template present
	if (!SpriteTemplate)
	{
		return FALSE;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles <= 0)
	{
		return FALSE;
	}
	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return FALSE;
	}

	// Make sure we will not be allocating enough memory
	check(MaxActiveParticles >= ActiveParticles);

	// Must be filled in by implementation in derived class
	OutData.eEmitterType = DET_Unknown;

	OutData.ActiveParticleCount = ActiveParticles;
	OutData.ParticleStride = ParticleStride;
	OutData.bRequiresSorting = bRequiresSorting;

	// Take scale into account
	OutData.Scale = FVector(1.0f, 1.0f, 1.0f);
	if (Component)
	{
		OutData.Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			OutData.Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}
	}


#if PARTICLES_USE_DOUBLE_BUFFERING
	// Allocate particle memory
	INT TotalVertexSize = MaxActiveParticles * ParticleStride;
	if (OutData.ParticleData.Num() < TotalVertexSize)
	{
		OutData.ParticleData.Empty( TotalVertexSize );
		OutData.ParticleData.Add( TotalVertexSize );
	}
	appMemcpy(OutData.ParticleData.GetData(), ParticleData, TotalVertexSize);

	// Allocate particle index memory
	if (OutData.ParticleIndices.Num() < MaxActiveParticles)
	{
		OutData.ParticleIndices.Empty( MaxActiveParticles );
		OutData.ParticleIndices.Add( MaxActiveParticles );
	}
	appMemcpy(OutData.ParticleIndices.GetData(), ParticleIndices, MaxActiveParticles * sizeof(WORD));
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	// Allocate particle memory
	OutData.ParticleData.Empty( MaxActiveParticles * ParticleStride );
	OutData.ParticleData.Add( MaxActiveParticles * ParticleStride );
	appMemcpy(OutData.ParticleData.GetData(), ParticleData, MaxActiveParticles * ParticleStride);

	// Allocate particle index memory
	OutData.ParticleIndices.Empty( MaxActiveParticles );
	OutData.ParticleIndices.Add( MaxActiveParticles );
	appMemcpy(OutData.ParticleIndices.GetData(), ParticleIndices, MaxActiveParticles * sizeof(WORD));
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

	// All particle emitter types derived from sprite emitters, so we can fill that data in here too!
	{
		FDynamicSpriteEmitterReplayDataBase* NewReplayData =
			static_cast< FDynamicSpriteEmitterReplayDataBase* >( &OutData );

		NewReplayData->MaterialInterface = NULL;	// Must be set by derived implementation

		NewReplayData->MaxDrawCount =
			(LODLevel->RequiredModule->bUseMaxDrawCount == TRUE) ? LODLevel->RequiredModule->MaxDrawCount : -1;
		NewReplayData->ScreenAlignment	= LODLevel->RequiredModule->ScreenAlignment;
		NewReplayData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
		NewReplayData->EmitterRenderMode = LODLevel->RequiredModule->EmitterRenderMode;
		NewReplayData->DynamicParameterDataOffset = DynamicParameterDataOffset;


		NewReplayData->bLockAxis = FALSE;
		if (Module_AxisLock && (Module_AxisLock->bEnabled == TRUE))
		{
			NewReplayData->LockAxisFlag = Module_AxisLock->LockAxisFlags;
			if (Module_AxisLock->LockAxisFlags != EPAL_NONE)
			{
				NewReplayData->bLockAxis = TRUE;
			}
		}

		// If there are orbit modules, add the orbit module data
		if (LODLevel->OrbitModules.Num() > 0)
		{
			UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
			UParticleModuleOrbit* LastOrbit = HighestLODLevel->OrbitModules(LODLevel->OrbitModules.Num() - 1);
			check(LastOrbit);

			UINT* LastOrbitOffset = ModuleOffsetMap.Find(LastOrbit);
			NewReplayData->OrbitModuleOffset = *LastOrbitOffset;
		}

	}

#if !FINAL_RELEASE
	if (GEngine->bCheckParticleRenderSize)
	{
		INT CheckSize = (ActiveParticles * 4 * sizeof(FParticleSpriteVertex));
		//@todo.SAS. This matches the 360 limiter... Make this an ini setting.
		if (CheckSize > GEngine->MaxParticleVertexMemory)
		{
			AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo() : NULL;
			if (WorldInfo)
			{
				FString ErrorMessage = 
					FString::Printf(TEXT("Emitter with too many vertices: %4d verts for %6d kB: %s"),
						ActiveParticles * 4, CheckSize / 1024, 
						Component ? 
							Component->Template ? 
								*(Component->Template->GetName()) :
								TEXT("No template") :
							TEXT("No component"));
				FColor ErrorColor(255,0,0);
				WorldInfo->AddOnScreenDebugMessage((INT)this, 5.0f, ErrorColor,ErrorMessage);
				debugf(*ErrorMessage);
			}
		}
	}
#endif	//#if !FINAL_RELEASE


	return TRUE;
}



/*-----------------------------------------------------------------------------
	ParticleSpriteEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	ParticleSpriteEmitterInstance
 *	The structure for a standard sprite emitter instance.
 */
/** Constructor	*/
FParticleSpriteEmitterInstance::FParticleSpriteEmitterInstance() :
	FParticleEmitterInstance()
{
}

/** Destructor	*/
FParticleSpriteEmitterInstance::~FParticleSpriteEmitterInstance()
{
}



/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	if (ActiveParticles <= 0)
	{
		return NULL;
	}
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return NULL;
	}
	// Allocate the dynamic data
	FDynamicSpriteEmitterData* NewEmitterData = ::new FDynamicSpriteEmitterData();

	// Now fill in the source data
	if( !FillReplayData( NewEmitterData->Source ) )
	{
		delete NewEmitterData;
		return NULL;
	}

	// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
	NewEmitterData->Init( bSelected );

#if PARTICLES_USE_DOUBLE_BUFFERING
	// Create the vertex factory...
	//@todo. Cache these??
	if (NewEmitterData->VertexFactory == NULL)
	{
		if (NewEmitterData->bUsesDynamicParameter == FALSE)
		{
			NewEmitterData->VertexFactory = new FParticleVertexFactory();
		}
		else
		{
			NewEmitterData->VertexFactory = new FParticleDynamicParameterVertexFactory();
		}
		check(NewEmitterData->VertexFactory);
		BeginInitResource(NewEmitterData->VertexFactory);
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

	return NewEmitterData;
}

/**
 *	Updates the dynamic data for the instance
 *
 *	@param	DynamicData		The dynamic data to fill in
 *	@param	bSelected		TRUE if the particle system component is selected
 */
UBOOL FParticleSpriteEmitterInstance::UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected)
{
	checkf((DynamicData->GetSource().eEmitterType == DET_Sprite), TEXT("Sprite::UpdateDynamicData> Invalid DynamicData type!"));

	if (ActiveParticles <= 0)
	{
		return FALSE;
	}

	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return FALSE;
	}
	FDynamicSpriteEmitterData* SpriteDynamicData = (FDynamicSpriteEmitterData*)DynamicData;
	// Now fill in the source data
	if( !FillReplayData( SpriteDynamicData->Source ) )
	{
		return FALSE;
	}

	// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
	SpriteDynamicData->Init( bSelected );

#if PARTICLES_USE_DOUBLE_BUFFERING
	// Create the vertex factory...
	//@todo. Cache these??
	if (SpriteDynamicData->VertexFactory == NULL)
	{
		if (SpriteDynamicData->bUsesDynamicParameter == FALSE)
		{
			SpriteDynamicData->VertexFactory = new FParticleVertexFactory();
		}
		else
		{
			SpriteDynamicData->VertexFactory = new FParticleDynamicParameterVertexFactory();
		}
		check(SpriteDynamicData->VertexFactory);
		BeginInitResource(SpriteDynamicData->VertexFactory);
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

	return TRUE;
}

/**
 *	Retrieves replay data for the emitter
 *
 *	@return	The replay data, or NULL on failure
 */
FDynamicEmitterReplayDataBase* FParticleSpriteEmitterInstance::GetReplayData()
{
	if (ActiveParticles <= 0)
	{
		return NULL;
	}

	FDynamicEmitterReplayDataBase* NewEmitterReplayData = ::new FDynamicSpriteEmitterReplayData();
	check( NewEmitterReplayData != NULL );

	if( !FillReplayData( *NewEmitterReplayData ) )
	{
		delete NewEmitterReplayData;
		return NULL;
	}

	return NewEmitterReplayData;
}



/**
 * Captures dynamic replay data for this particle system.
 *
 * @param	OutData		[Out] Data will be copied here
 *
 * @return Returns TRUE if successful
 */
UBOOL FParticleSpriteEmitterInstance::FillReplayData( FDynamicEmitterReplayDataBase& OutData )
{
	if (ActiveParticles <= 0)
	{
		return FALSE;
	}

	// Call parent implementation first to fill in common particle source data
	if( !FParticleEmitterInstance::FillReplayData( OutData ) )
	{
		return FALSE;
	}

	OutData.eEmitterType = DET_Sprite;

	FDynamicSpriteEmitterReplayData* NewReplayData =
		static_cast< FDynamicSpriteEmitterReplayData* >( &OutData );

	// Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
	UMaterialInterface* MaterialInst = CurrentMaterial;
	if ((MaterialInst == NULL) || (MaterialInst->CheckMaterialUsage(MATUSAGE_ParticleSprites) == FALSE))
	{
		MaterialInst = GEngine->DefaultMaterial;
	}
	NewReplayData->MaterialInterface = MaterialInst;

	return TRUE;
}


/*-----------------------------------------------------------------------------
	ParticleSpriteSubUVEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	ParticleSpriteSubUVEmitterInstance
 *	Structure for SubUV sprite instances
 */
/** Constructor	*/
FParticleSpriteSubUVEmitterInstance::FParticleSpriteSubUVEmitterInstance() :
	FParticleEmitterInstance()
{
}

/** Destructor	*/
FParticleSpriteSubUVEmitterInstance::~FParticleSpriteSubUVEmitterInstance()
{
}

/**
 *	Kill off any dead particles. (Remove them from the active array)
 */
void FParticleSpriteSubUVEmitterInstance::KillParticles()
{
	if (ActiveParticles > 0)
	{
		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);
		FParticleEventInstancePayload* EventPayload = NULL;
		if (LODLevel->EventGenerator)
		{
			EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
			if (EventPayload && (EventPayload->bDeathEventsPresent == FALSE))
			{
				EventPayload = NULL;
			}
		}

		// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
		// move them to the 'end' of the active particle list.
		for (INT i=ActiveParticles-1; i>=0; i--)
		{
			const INT	CurrentIndex	= ParticleIndices[i];
			const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
			if (Particle.RelativeTime > 1.0f)
			{
				FLOAT* pkFloats = (FLOAT*)((BYTE*)&Particle + PayloadOffset);
				pkFloats[0] = 0.0f;
				pkFloats[1] = 0.0f;
				pkFloats[2] = 0.0f;
				pkFloats[3] = 0.0f;
				pkFloats[4] = 0.0f;

				if (EventPayload)
				{
					LODLevel->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
				}

				ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
				ParticleIndices[ActiveParticles-1]	= CurrentIndex;
				ActiveParticles--;
			}
		}
	}
}



/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleSpriteSubUVEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	if (ActiveParticles <= 0)
	{
		return NULL;
	}

	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return NULL;
	}
	// Allocate the dynamic data
	FDynamicSubUVEmitterData* NewEmitterData = ::new FDynamicSubUVEmitterData();

	// Now fill in the source data
	if( !FillReplayData( NewEmitterData->Source ) )
	{
		delete NewEmitterData;
		return NULL;
	}

	// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
	NewEmitterData->Init( bSelected );

#if PARTICLES_USE_DOUBLE_BUFFERING
	// Create the vertex factory...
	//@todo. Cache these??
	if (NewEmitterData->VertexFactory == NULL)
	{
		if (NewEmitterData->bUsesDynamicParameter == FALSE)
		{
			NewEmitterData->VertexFactory = new FParticleSubUVVertexFactory();
		}
		else
		{
			NewEmitterData->VertexFactory = new FParticleSubUVDynamicParameterVertexFactory();
		}
		check(NewEmitterData->VertexFactory);
		BeginInitResource(NewEmitterData->VertexFactory);
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

	return NewEmitterData;
}

/**
 *	Updates the dynamic data for the instance
 *
 *	@param	DynamicData		The dynamic data to fill in
 *	@param	bSelected		TRUE if the particle system component is selected
 */
UBOOL FParticleSpriteSubUVEmitterInstance::UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected)
{
	checkf((DynamicData->GetSource().eEmitterType == DET_SubUV), TEXT("SubUV::UpdateDynamicData> Invalid DynamicData type!"));

	if (ActiveParticles <= 0)
	{
		return FALSE;
	}
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return FALSE;
	}
	FDynamicSubUVEmitterData* SubUVDynamicData = (FDynamicSubUVEmitterData*)DynamicData;
	// Now fill in the source data
	if( !FillReplayData( SubUVDynamicData->Source ) )
	{
		return FALSE;
	}

	// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
	SubUVDynamicData->Init( bSelected );

#if PARTICLES_USE_DOUBLE_BUFFERING
	// Create the vertex factory...
	//@todo. Cache these??
	if (SubUVDynamicData->VertexFactory == NULL)
	{
		if (SubUVDynamicData->bUsesDynamicParameter == FALSE)
		{
			SubUVDynamicData->VertexFactory = new FParticleSubUVVertexFactory();
		}
		else
		{
			SubUVDynamicData->VertexFactory = new FParticleSubUVDynamicParameterVertexFactory();
		}
		check(SubUVDynamicData->VertexFactory);
		BeginInitResource(SubUVDynamicData->VertexFactory);
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING


	return TRUE;
}

/**
 *	Retrieves replay data for the emitter
 *
 *	@return	The replay data, or NULL on failure
 */
FDynamicEmitterReplayDataBase* FParticleSpriteSubUVEmitterInstance::GetReplayData()
{
	if (ActiveParticles <= 0)
	{
		return NULL;
	}

	FDynamicEmitterReplayDataBase* NewEmitterReplayData = ::new FDynamicSubUVEmitterReplayData();
	check( NewEmitterReplayData != NULL );

	if( !FillReplayData( *NewEmitterReplayData ) )
	{
		delete NewEmitterReplayData;
		return NULL;
	}

	return NewEmitterReplayData;
}



/**
 * Captures dynamic replay data for this particle system.
 *
 * @param	OutData		[Out] Data will be copied here
 *
 * @return Returns TRUE if successful
 */
UBOOL FParticleSpriteSubUVEmitterInstance::FillReplayData( FDynamicEmitterReplayDataBase& OutData )
{
	// Call parent implementation first to fill in common particle source data
	if( !FParticleEmitterInstance::FillReplayData( OutData ) )
	{
		return FALSE;
	}

	// Grab the LOD level
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return FALSE;
	}

	OutData.eEmitterType = DET_SubUV;

	FDynamicSubUVEmitterReplayData* NewReplayData =
		static_cast< FDynamicSubUVEmitterReplayData* >( &OutData );

	// Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
	UMaterialInterface* MaterialInst = CurrentMaterial;
	if ((MaterialInst == NULL) || (MaterialInst->CheckMaterialUsage(MATUSAGE_ParticleSubUV) == FALSE))
	{
		MaterialInst = GEngine->DefaultMaterial;
	}

	NewReplayData->MaterialInterface = MaterialInst;


	NewReplayData->SubUVDataOffset = SubUVDataOffset;
	NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
	NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
	NewReplayData->bDirectUV = LODLevel->RequiredModule->bDirectUV;


	return TRUE;
}


/*-----------------------------------------------------------------------------
	ParticleMeshEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	Structure for mesh emitter instances
 */

/** Constructor	*/
FParticleMeshEmitterInstance::FParticleMeshEmitterInstance() :
	  FParticleEmitterInstance()
	, MeshTypeData(NULL)
	, MeshComponentIndex(-1)
	, MeshRotationActive(FALSE)
	, MeshRotationOffset(0)
	, bIgnoreComponentScale(FALSE)
{
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleMeshEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	FParticleEmitterInstance::InitParameters(InTemplate, InComponent, bClearResources);

	// Get the type data module
	UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
	check(LODLevel);
	MeshTypeData = CastChecked<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
	check(MeshTypeData);

    // Grab the MeshRotationRate module offset, if there is one...
    MeshRotationActive = FALSE;
	if (LODLevel->RequiredModule->ScreenAlignment == PSA_Velocity)
	{
		MeshRotationActive = TRUE;
	}
	else
	{
	    for (INT i = 0; i < LODLevel->Modules.Num(); i++)
	    {
	        if (LODLevel->Modules(i)->IsA(UParticleModuleMeshRotationRate::StaticClass())	||
	            LODLevel->Modules(i)->IsA(UParticleModuleMeshRotation::StaticClass())		||
				LODLevel->Modules(i)->IsA(UParticleModuleUberRainImpacts::StaticClass())	||
				LODLevel->Modules(i)->IsA(UParticleModuleUberRainSplashA::StaticClass())
				)
	        {
	            MeshRotationActive = TRUE;
	            break;
	        }
		}
    }
}

/**
 *	Initialize the instance
 */
void FParticleMeshEmitterInstance::Init()
{
	FParticleEmitterInstance::Init();

	// If there is a mesh present (there should be!)
	if (MeshTypeData->Mesh && (MeshTypeData->Mesh->LODModels.Num() > 0))
	{
		const FStaticMeshRenderData& MeshLODModel = MeshTypeData->Mesh->LODModels(0);

		AEmitterPool* EmitterPool = NULL;
		if (GWorld && GWorld->GetWorldInfo())
		{
			EmitterPool = GWorld->GetWorldInfo()->MyEmitterPool;
		}

		UStaticMeshComponent* MeshComponent = NULL;

		// If the index is set, try to retrieve it from the component
		if (MeshComponentIndex == -1)
		{
			// See if the SMC we are interested in is already present...
			for (INT SMIndex = 0; SMIndex < Component->SMComponents.Num(); SMIndex++)
			{
				UStaticMeshComponent* SMComp = Component->SMComponents(SMIndex);
				if (SMComp)
				{
					if (SMComp->StaticMesh == MeshTypeData->Mesh)
					{
						MeshComponentIndex = SMIndex;
						break;
					}
				}
			}
		}
		if (MeshComponentIndex != -1)
		{
			if (MeshComponentIndex < Component->SMComponents.Num())
			{
				MeshComponent = Component->SMComponents(MeshComponentIndex);
			}

			if (MeshComponent && (MeshComponent->StaticMesh != MeshTypeData->Mesh))
			{
				// Incorrect mesh!
				// We can't assume that another emitter in the PSystem isn't use it,
				// so simply regrab another one...
				// This should be safe as the system will release all of them.
				MeshComponent = NULL;
			}

			// If it wasn't retrieved, force it to get recreated
			if (MeshComponent == NULL)
			{
				MeshComponentIndex = -1;
			}
		}

		if (MeshComponentIndex == -1)
		{
			// try to find a free component instead of creating one
			if (EmitterPool)
			{
				MeshComponent = EmitterPool->GetFreeStaticMeshComponent();
			}

			// create the component if necessary
			if (MeshComponent == NULL)
			{
				// If the pool did not return one to us, create one w/ the PSysComponent as the outer.
				MeshComponent = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(), Component);
				MeshComponent->bAcceptsStaticDecals = FALSE;
				MeshComponent->bAcceptsDynamicDecals = FALSE;
				MeshComponent->CollideActors		= FALSE;
				MeshComponent->BlockActors			= FALSE;
				MeshComponent->BlockZeroExtent		= FALSE;
				MeshComponent->BlockNonZeroExtent	= FALSE;
				MeshComponent->BlockRigidBody		= FALSE;
			}

			// allocate space for material instance constants
			INT Diff = MeshComponent->Materials.Num() - MeshLODModel.Elements.Num();
			if (Diff > 0)
			{
				MeshComponent->Materials.Remove(MeshComponent->Materials.Num() - Diff - 1, Diff);
			}
			else if (Diff < 0)
			{
				MeshComponent->Materials.AddZeroed(-Diff);
			}

			check(MeshComponent->Materials.Num() == MeshLODModel.Elements.Num());

			MeshComponent->StaticMesh		= MeshTypeData->Mesh;
			MeshComponent->CastShadow		= MeshTypeData->CastShadows;
			MeshComponent->bAcceptsLights	= Component->bAcceptsLights;

			for (INT SlotIndex = 0; SlotIndex < Component->SMComponents.Num(); SlotIndex++)
			{
				if (Component->SMComponents(SlotIndex) == NULL)
				{
					MeshComponentIndex = SlotIndex;
					Component->SMComponents(SlotIndex) = MeshComponent;
				}
			}
			if (MeshComponentIndex == -1)
			{
				MeshComponentIndex = Component->SMComponents.AddItem(MeshComponent);
			}
		}
		check(MeshComponent);
		check(MeshComponent->Materials.Num() >= MeshLODModel.Elements.Num());

		// Constructing MaterialInstanceConstant for each mesh instance is done so that
		// particle 'vertex color' can be set on each individual mesh.
		// They are tagged as transient so they don't get saved in the package.
		for (INT MatIndex = 0; MatIndex < MeshComponent->Materials.Num(); MatIndex++)
		{
			const FStaticMeshElement* MeshElement = &(MeshLODModel.Elements(MatIndex));
			if (MeshElement == NULL)
			{
				// This should not happen...
				continue;
			}

			UMaterialInterface* Parent = NULL;

			// Determine what the material should be...

			// First check the emitter instance Materials array, which is set from the 
			// MeshMaterial module...
			if (CurrentMaterials.Num() > MatIndex)
			{
				Parent = CurrentMaterials(MatIndex);
			}

			if (Parent == NULL)
			{
				// Not set yet, so check for the bOverrideMaterial flag
				if (MeshTypeData->bOverrideMaterial == TRUE)
				{
					Parent = CurrentLODLevel->RequiredModule->Material;
				}
			}

			if (Parent == NULL)
			{
				// Not set yet, so use the StaticMesh material that is assigned.
				Parent = MeshElement->Material;
			}

			if (Parent == NULL)
			{
				// This is really bad... no material found.
				// But, we don't want to crash, so use the Default one.
				Parent = GEngine->DefaultMaterial;
			}

			check(Parent);

			UMaterialInstanceConstant* MatInst = NULL;
			if (MeshComponent->Materials.Num() > MatIndex)
			{
				MatInst = Cast<UMaterialInstanceConstant>(MeshComponent->Materials(MatIndex));
			}

			if (MatInst == NULL)
			{
				if (EmitterPool)
				{
					MatInst = EmitterPool->GetFreeMatInstConsts();
				}

				if (MatInst == NULL)
				{
					// create the instance constant if necessary
					MatInst = ConstructObject<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), MeshComponent);
				}

				if (MeshComponent->Materials.Num() > MatIndex)
				{
					MeshComponent->Materials(MatIndex) = MatInst;
				}
				else
				{
					INT CheckIndex = MeshComponent->Materials.AddItem(MatInst);
					check(CheckIndex == MatIndex);
				}
			}

			check(MatInst);

			MatInst->SetParent(Parent);
			MatInst->SetFlags(RF_Transient);
		}
	}
}

/**
 *	Resize the particle data array
 *
 *	@param	NewMaxActiveParticles	The new size to use
 *
 *	@return	UBOOL					TRUE if the resize was successful
 */
UBOOL FParticleMeshEmitterInstance::Resize(INT NewMaxActiveParticles, UBOOL bSetMaxActiveCount)
{
	INT OldMaxActiveParticles = MaxActiveParticles;
	if (FParticleEmitterInstance::Resize(NewMaxActiveParticles, bSetMaxActiveCount) == TRUE)
	{
		if (MeshRotationActive)
		{
			for (INT i = OldMaxActiveParticles; i < NewMaxActiveParticles; i++)
			{
				DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
				FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
				PayloadData->RotationRateBase			= FVector(0.0f);
			}
		}

		return TRUE;
	}

	return FALSE;
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleMeshEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleTickTime);
	SCOPE_CYCLE_COUNTER(STAT_MeshTickTime);

	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	// See if we are handling mesh rotation
    if (MeshRotationActive)
    {
		// Update the rotation for each particle
        for (INT i = 0; i < ActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
			PayloadData->RotationRate				= PayloadData->RotationRateBase;
			if (LODLevel->RequiredModule->ScreenAlignment == PSA_Velocity)
			{
				// Determine the rotation to the velocity vector and apply it to the mesh
				FVector	NewDirection	= Particle.Velocity;
				NewDirection.Normalize();
				FVector	OldDirection(1.0f, 0.0f, 0.0f);

				FQuat Rotation	= FQuatFindBetween(OldDirection, NewDirection);
				FVector Euler	= Rotation.Euler();
				PayloadData->Rotation.X	= Euler.X;
				PayloadData->Rotation.Y	= Euler.Y;
				PayloadData->Rotation.Z	= Euler.Z;
			}
	    }
    }

	// Call the standard tick
	FParticleEmitterInstance::Tick(DeltaTime, bSuppressSpawning);

	// Apply rotation if it is active
    if (MeshRotationActive)
    {
        for (INT i = 0; i < ActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
			{
	            FMeshRotationPayloadData* PayloadData	 = (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
				PayloadData->Rotation					+= DeltaTime * PayloadData->RotationRate;
			}
        }
    }

	// Do we need to tick the mesh instances or will the engine do it?
	if ((ActiveParticles == 0) & bSuppressSpawning)
	{
		RemovedFromScene();
	}

	// Remove from the Sprite count... happens because we use the Super::Tick
	DEC_DWORD_STAT_BY(STAT_SpriteParticles, ActiveParticles);
	INC_DWORD_STAT_BY(STAT_MeshParticles, ActiveParticles);
}

/**
 *	Update the bounding box for the emitter
 *
 *	@param	DeltaTime		The time slice to use
 */
void FParticleMeshEmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
{
	//@todo. Implement proper bound determination for mesh emitters.
	// Currently, just 'forcing' the mesh size to be taken into account.
	if (Component)
	{
		// Take scale into account
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		if (!bIgnoreComponentScale)
		{
			Scale *= Component->Scale * Component->Scale3D;
		}
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}

		// Get the static mesh bounds
		FBoxSphereBounds MeshBound;
		if (Component->bWarmingUp == FALSE)
		{	
			if (MeshTypeData->Mesh)
			{
				MeshBound = MeshTypeData->Mesh->Bounds;
			}
			else
			{
				warnf(TEXT("MeshEmitter with no mesh set?? - %s"), Component->Template ? *(Component->Template->GetPathName()) : TEXT("??????"));
				MeshBound = FBoxSphereBounds();
			}
		}
		else
		{
			// This isn't used anywhere if the bWarmingUp flag is false, but GCC doesn't like it not touched.
			appMemzero(&MeshBound, sizeof(FBoxSphereBounds));
		}

		if (ActiveParticles > 0)
		{
			DECLARE_PARTICLE_PTR(NextParticle, ParticleData + ParticleStride * ParticleIndices[0]);
			PREFETCH(NextParticle);
		}

		FLOAT MaxSizeScale	= 1.0f;
		FVector	NewLocation;
		FLOAT	NewRotation;
		ParticleBoundingBox.Init();
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			if (i + 1 < ActiveParticles)
			{
				const INT NextIndex = ParticleIndices[i+1];
				DECLARE_PARTICLE_PTR(NextParticle, ParticleData + ParticleStride * NextIndex);
				PREFETCH(NextParticle);
			}

			// Do linear integrator and update bounding box
			Particle.OldLocation = Particle.Location;
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)
			{
				if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
				{
					NewLocation	= Particle.Location + DeltaTime * Particle.Velocity;
				}
				else
				{
					NewLocation = Particle.Location;
				}
				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					NewRotation	= Particle.Rotation + DeltaTime * Particle.RotationRate;
				}
				else
				{
					NewRotation = Particle.Rotation;
				}
			}
			else
			{
				// Don't move it...
				NewLocation = Particle.Location;
				NewRotation = Particle.Rotation;
			}

			// Do angular integrator, and wrap result to within +/- 2 PI
			FVector Size = Particle.Size * Scale;
			MaxSizeScale = Max(MaxSizeScale, Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.

			Particle.Rotation = appFmod(NewRotation, 2.f*(FLOAT)PI);
			Particle.Location = NewLocation;
			if (Component->bWarmingUp == FALSE)
			{	
				ParticleBoundingBox += (Particle.Location + MeshBound.SphereRadius * Size);
				ParticleBoundingBox += (Particle.Location - MeshBound.SphereRadius * Size);
			}
		}

		if (Component->bWarmingUp == FALSE)
		{	
			ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale * PARTICLESYSTEM_BOUNDSSCALAR);
			// Transform bounding box into world space if the emitter uses a local space coordinate system.
			UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
			check(LODLevel);
			if (LODLevel->RequiredModule->bUseLocalSpace) 
			{
				ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
			}
		}
	}
}

/**
 *	Retrieved the per-particle bytes that this emitter type requires.
 *
 *	@return	UINT	The number of required bytes for particles in the instance
 */
UINT FParticleMeshEmitterInstance::RequiredBytes()
{
	UINT uiBytes = FParticleEmitterInstance::RequiredBytes();
	MeshRotationOffset	= PayloadOffset + uiBytes;
	uiBytes += sizeof(FMeshRotationPayloadData);
	return uiBytes;
}

/**
 *	Handle any post-spawning actions required by the instance
 *
 *	@param	Particle					The particle that was spawned
 *	@param	InterpolationPercentage		The percentage of the time slice it was spawned at
 *	@param	SpawnTIme					The time it was spawned at
 */
void FParticleMeshEmitterInstance::PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime)
{
	FParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if (LODLevel->RequiredModule->ScreenAlignment == PSA_Velocity)
	{
		// Determine the rotation to the velocity vector and apply it to the mesh
		FVector	NewDirection	= Particle->Velocity;
		NewDirection.Normalize();
		FVector	OldDirection(1.0f, 0.0f, 0.0f);

		FQuat Rotation	= FQuatFindBetween(OldDirection, NewDirection);
		FVector Euler	= Rotation.Euler();

		FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)Particle + MeshRotationOffset);
		PayloadData->Rotation.X	+= Euler.X;
		PayloadData->Rotation.Y	+= Euler.Y;
		PayloadData->Rotation.Z	+= Euler.Z;
		//
	}
}



/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	if (ActiveParticles <= 0)
	{
		return NULL;
	}

	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return NULL;
	}
	if ((MeshComponentIndex == -1) || (MeshComponentIndex >= Component->SMComponents.Num()))
	{
		// Not initialized?
		return NULL;
	}

	UStaticMeshComponent* MeshComponent = Component->SMComponents(MeshComponentIndex);
	if (MeshComponent == NULL)
	{
		// The mesh component has been GC'd?
		return NULL;
	}

	// Allocate the dynamic data
	FDynamicMeshEmitterData* NewEmitterData = ::new FDynamicMeshEmitterData();

	// Now fill in the source data
	if( !FillReplayData( NewEmitterData->Source ) )
	{
		delete NewEmitterData;
		return NULL;
	}


	// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
	NewEmitterData->Init(
		bSelected,
		this,
		MeshTypeData->Mesh,
		MeshComponent,
		FALSE );		// Use Nx fluid?

	return NewEmitterData;
}

/**
 *	Updates the dynamic data for the instance
 *
 *	@param	DynamicData		The dynamic data to fill in
 *	@param	bSelected		TRUE if the particle system component is selected
 */
UBOOL FParticleMeshEmitterInstance::UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected)
{
	if (ActiveParticles <= 0)
	{
		return FALSE;
	}

	if ((MeshComponentIndex == -1) || (MeshComponentIndex >= Component->SMComponents.Num()))
	{
		// Not initialized?
		return FALSE;
	}

	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return FALSE;
	}
	UStaticMeshComponent* MeshComponent = Component->SMComponents(MeshComponentIndex);
	if (MeshComponent == NULL)
	{
		// The mesh component has been GC'd?
		return FALSE;
	}

	checkf((DynamicData->GetSource().eEmitterType == DET_Mesh), TEXT("Mesh::UpdateDynamicData> Invalid DynamicData type!"));

	FDynamicMeshEmitterData* MeshDynamicData = (FDynamicMeshEmitterData*)DynamicData;
	// Now fill in the source data
	if( !FillReplayData( MeshDynamicData->Source ) )
	{
		return FALSE;
	}

	// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
	MeshDynamicData->Init(
		bSelected,
		this,
		MeshTypeData->Mesh,
		MeshComponent,
		FALSE );		// Use Nx fluid?

	return TRUE;
}

/**
 *	Retrieves replay data for the emitter
 *
 *	@return	The replay data, or NULL on failure
 */
FDynamicEmitterReplayDataBase* FParticleMeshEmitterInstance::GetReplayData()
{
	if (ActiveParticles <= 0)
	{
		return NULL;
	}

	FDynamicEmitterReplayDataBase* NewEmitterReplayData = ::new FDynamicMeshEmitterReplayData();
	check( NewEmitterReplayData != NULL );

	if( !FillReplayData( *NewEmitterReplayData ) )
	{
		delete NewEmitterReplayData;
		return NULL;
	}

	return NewEmitterReplayData;
}



/**
 * Captures dynamic replay data for this particle system.
 *
 * @param	OutData		[Out] Data will be copied here
 *
 * @return Returns TRUE if successful
 */
UBOOL FParticleMeshEmitterInstance::FillReplayData( FDynamicEmitterReplayDataBase& OutData )
{
	// Call parent implementation first to fill in common particle source data
	if( !FParticleEmitterInstance::FillReplayData( OutData ) )
	{
		return FALSE;
	}

	// Grab the LOD level
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == FALSE))
	{
		return FALSE;
	}

	CurrentMaterial = LODLevel->RequiredModule->Material;

	OutData.eEmitterType = DET_Mesh;

	FDynamicMeshEmitterReplayData* NewReplayData =
		static_cast< FDynamicMeshEmitterReplayData* >( &OutData );

	// Don't need this for meshes
	NewReplayData->MaterialInterface = NULL;

	// Mesh settings
	NewReplayData->bScaleUV = LODLevel->RequiredModule->bScaleUV;
	NewReplayData->SubUVInterpMethod = LODLevel->RequiredModule->InterpolationMethod;
	NewReplayData->SubUVDataOffset = SubUVDataOffset;
	NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
	NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
	NewReplayData->MeshRotationOffset = MeshRotationOffset;
	NewReplayData->bMeshRotationActive = MeshRotationActive;
	NewReplayData->MeshAlignment = MeshTypeData->MeshAlignment;

	// Scale needs to be handled in a special way for meshes.  The parent implementation set this
	// itself, but we'll recompute it here.
	NewReplayData->Scale = FVector(1.0f, 1.0f, 1.0f);
	if (Component)
	{
		check(SpriteTemplate);
		UParticleLODLevel* LODLevel2 = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel2);
		check(LODLevel2->RequiredModule);
		// Take scale into account
		if (LODLevel2->RequiredModule->bUseLocalSpace == FALSE)
		{
			if (!bIgnoreComponentScale)
			{
				NewReplayData->Scale *= Component->Scale * Component->Scale3D;
			}
			AActor* Actor = Component->GetOwner();
			if (Actor && !Component->AbsoluteScale)
			{
				NewReplayData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
			}
		}
	}

	if (Module_AxisLock && (Module_AxisLock->bEnabled == TRUE))
	{
		NewReplayData->LockAxisFlag = Module_AxisLock->LockAxisFlags;
		if (Module_AxisLock->LockAxisFlags != EPAL_NONE)
		{
			NewReplayData->bLockAxis = TRUE;
			switch (Module_AxisLock->LockAxisFlags)
			{
			case EPAL_X:
				NewReplayData->LockedAxis = FVector(1,0,0);
				break;
			case EPAL_Y:
				NewReplayData->LockedAxis = FVector(0,1,0);
				break;
			case EPAL_NEGATIVE_X:
				NewReplayData->LockedAxis = FVector(-1,0,0);
				break;
			case EPAL_NEGATIVE_Y:
				NewReplayData->LockedAxis = FVector(0,-1,0);
				break;
			case EPAL_NEGATIVE_Z:
				NewReplayData->LockedAxis = FVector(0,0,-1);
				break;
			case EPAL_Z:
			case EPAL_NONE:
			default:
				NewReplayData->LockedAxis = FVector(0,0,1);
				break;
			}
		}
	}


	return TRUE;
}




/** FDynamicSpriteEmitterReplayDataBase Serialization */
void FDynamicSpriteEmitterReplayDataBase::Serialize( FArchive& Ar )
{
	// Call parent implementation
	FDynamicEmitterReplayDataBase::Serialize( Ar );

	Ar << ScreenAlignment;
	Ar << bUseLocalSpace;
	Ar << bLockAxis;
	Ar << LockAxisFlag;
	Ar << MaxDrawCount;
	Ar << EmitterRenderMode;
	Ar << OrbitModuleOffset;
	Ar << DynamicParameterDataOffset;

	Ar << MaterialInterface;
}
