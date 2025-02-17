/*=============================================================================
	ParticlePhysXSpriteEmitterInstance.cpp: PhysX Emitter Source.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "LevelUtils.h"

#if WITH_NOVODEX

#include "UnNovodexSupport.h"
#include "PhysXParticleSystem.h"
#include "PhysXVerticalEmitter.h"
#include "PhysXParticleSetSprite.h"

IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpritePhysXEmitterInstance);

FParticleSpritePhysXEmitterInstance::FParticleSpritePhysXEmitterInstance(class UParticleModuleTypeDataPhysX &TypeData) : 
	PhysXTypeData(TypeData), 
	NumSpawnedParticles(0),
	SpawnEstimateTime(0.0f),
	PSet(NULL)
{
	SpawnEstimateRate = 0.0f;
	SpawnEstimateLife = 0.0f;
	LodEmissionBudget = INT_MAX;
	LodEmissionRemainder = 0.0f;
}

FParticleSpritePhysXEmitterInstance::~FParticleSpritePhysXEmitterInstance()
{
	CleanUp();
}

void FParticleSpritePhysXEmitterInstance::CleanUp()
{
	if(PhysXTypeData.PhysXParSys)
	{
		UPhysXParticleSystem& ParSys = *PhysXTypeData.PhysXParSys;
		ParSys.RemoveSpawnInstance(this);
		if(ParSys.PSys && PSet)
		{
			ParSys.PSys->RemoveParticleSet(PSet);
		}
	}

	if(PSet)
	{
		delete PSet;
		PSet = NULL;
	}
	ActiveParticles = 0;
}

void FParticleSpritePhysXEmitterInstance::RemovedFromScene()
{
	CleanUp();
}

FPhysXParticleSystem& FParticleSpritePhysXEmitterInstance::GetPSys()
{
	check(PhysXTypeData.PhysXParSys && PhysXTypeData.PhysXParSys->PSys);
	return *PhysXTypeData.PhysXParSys->PSys;
}

UBOOL FParticleSpritePhysXEmitterInstance::TryConnect()
{
	if(Component->bWarmingUp)
		return FALSE;

	if(!PhysXTypeData.PhysXParSys)
		return FALSE;

	if(!PhysXTypeData.PhysXParSys->TryConnect())
		return FALSE;

	if(!PSet)
	{
		PSet = new FPhysXParticleSetSprite(*this);
	}
	return TRUE;
}

FRBVolumeFill* FParticleSpritePhysXEmitterInstance::GetVolumeFill()
{
	if(!Component)
		return NULL;
	
	AActor* Owner = Component->GetOwner();
	if(!Owner)
		return NULL;

	APhysXEmitterSpawnable* Emitter = Cast<APhysXEmitterSpawnable>(Owner);
	if(!Emitter)
		return NULL;

	return Emitter->VolumeFill;
}

void FParticleSpritePhysXEmitterInstance::AsyncProccessSpawnedParticles(FLOAT DeltaTime, INT FirstIndex)
{
	FPhysXParticleSystem& PSys = GetPSys();
	FVector LODOrigin(0,0,0);
	BOOL bUseDistCulling = GetLODOrigin(LODOrigin);

	FLOAT LifetimeSum = 0.0f;

	//Filter particle for LOD and convert some data.
	INT LastParticleIndex = ActiveParticles-1;
	for(INT i=LastParticleIndex; i>=FirstIndex; i--)
	{
		FBaseParticle *Particle = (FBaseParticle*)((BYTE*)ParticleData + ParticleStride*ParticleIndices[i]);

		if(bUseDistCulling && PSys.GetLODDistanceSq(LODOrigin, Particle->Location) > PSys.VerticalPacketRadiusSq)
		{
			RemoveParticleFromActives(i);
			LastParticleIndex--;
			continue;
		}
		LifetimeSum += (Particle->OneOverMaxLifetime > 0.0f)?(1.0f/Particle->OneOverMaxLifetime):0.0f;
	}
	NumSpawnedParticles = LastParticleIndex + 1 - FirstIndex;
	SpawnEstimateUpdate(DeltaTime, NumSpawnedParticles, LifetimeSum);
}

void FParticleSpritePhysXEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	if(!TryConnect())
	{
		return;
	}

	FRBPhysScene* Scene = PhysXTypeData.PhysXParSys->GetScene();
	check(Scene);

	//The timestep passed into here should result in the right compartment timimg, we query after, in editor.
	Scene->PhysXEmitterManager->Tick(DeltaTime);
	
	//This relies on NxCompartment::setTiming() be called befor this tick.
	DeltaTime = Scene->PhysXEmitterManager->GetEffectiveStepSize();

	FPhysXParticleSystem& PSys = GetPSys();
	PSys.AddSpawnInstance(this);

	NumSpawnedParticles = 0;

	if(!GetVolumeFill())
	{
		INT ActiveParticlesOld = ActiveParticles;

		OverwriteUnsupported();
		FParticleSpriteSubUVEmitterInstance::Tick(DeltaTime, bSuppressSpawning);
		RestoreUnsupported();

		AsyncProccessSpawnedParticles(DeltaTime, ActiveParticlesOld);
		//plotf( TEXT("DEBUG_VE_PLOT numSpawnedEmitterRangeClamp %d"), NumSpawnedParticles);
	}
}

FParticleSystemSceneProxy* FParticleSpritePhysXEmitterInstance::GetSceneProxy()
{
	if(!Component)
		return NULL;

	return (FParticleSystemSceneProxy*)Scene_GetProxyFromInfo(Component->SceneInfo);
}

UBOOL FParticleSpritePhysXEmitterInstance::GetLODOrigin(FVector& OutLODOrigin)
{
	FParticleSystemSceneProxy* SceneProxy = GetSceneProxy();
	if(SceneProxy)
	{
		OutLODOrigin = SceneProxy->GetLODOrigin();
	}
	return SceneProxy != NULL;
}

UBOOL FParticleSpritePhysXEmitterInstance::GetLODNearClippingPlane(FPlane& OutClippingPlane)
{
	FParticleSystemSceneProxy* SceneProxy = GetSceneProxy();
	if(!SceneProxy)
	{
		return FALSE;
	}
	UBOOL HasCP = SceneProxy->GetNearClippingPlane(OutClippingPlane);
	return HasCP;
}

/**
Assumes the base class emitter instance has spawned particles, which now need to be added to the 
FPhysXParticleSystem and to the RenderInstance.

UE Particle lifetimes are converted to PhysX SDK lifetimes.
Adds particles to the PhysXParticle system and creates entries in the RenderInstance 
with SDK particle Ids.

Sets appropriate initial sizes and max lifetimes for the new particles in the RenderInstance.
*/
void FParticleSpritePhysXEmitterInstance::SpawnSyncPhysX()
{
	FRBVolumeFill* VolumeFill = GetVolumeFill();
	if(VolumeFill)
	{
		FillVolume(*VolumeFill);
	}
	else
	{
		PostSpawnVerticalLod();
	}
	
	if(NumSpawnedParticles == 0)
	{
		return;
	}

	FPhysXParticleSystem& PSys = GetPSys();
	INT FirstSpawnedIndex = ActiveParticles - NumSpawnedParticles;

	//At least temporarily do local allocation:
	struct FTempParticle
	{
		NxVec3 Pos;
		NxVec3 Vel;
	};
	FTempParticle* TempParticleBuffer = (FTempParticle*)appAlloca(NumSpawnedParticles*sizeof(FTempParticle));

	for(INT i = 0; i<NumSpawnedParticles; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[FirstSpawnedIndex+i]);
		FTempParticle& Dest = TempParticleBuffer[i]; 			
		Dest.Pos = U2NPosition(Particle->Location);
		Dest.Vel = U2NPosition(Particle->Velocity);
	}

	NxParticleData particleData;
	particleData.bufferPos = (NxF32*)&TempParticleBuffer[0].Pos.x;
	particleData.bufferVel = (NxF32*)&TempParticleBuffer[0].Vel.x;
	particleData.numParticlesPtr = (NxU32*)&NumSpawnedParticles;
	particleData.bufferPosByteStride = sizeof(FTempParticle);
	particleData.bufferVelByteStride = sizeof(FTempParticle);
	check(particleData.isValid());

	check(PSet);
	
	INT SdkIndex = PSys.NumParticlesSdk;
	INT NumCreated = PSys.AddParticles(particleData, PSet);

	INT NumFailed = NumSpawnedParticles - NumCreated;
	for(INT i=0; i<NumFailed; i++)
	{
		RemoveParticleFromActives(ActiveParticles-1);
	}

	//Creation for FPhysXParticleSet
	for(INT i=0; i<NumCreated; i++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[FirstSpawnedIndex+i]);
		PhysXParticle& SdkParticle = PSys.ParticlesSdk[SdkIndex];
		PhysXRenderParticleSprite RenderParticle;
		
		RenderParticle.Id = SdkParticle.Id;
		RenderParticle.ParticleIndex = SdkIndex;
		RenderParticle.OneOverMaxLifetime = Particle.OneOverMaxLifetime;
		RenderParticle.RelativeTime = 0.0f;
		check(FirstSpawnedIndex + i == PSet->GetNumRenderParticles());
		PSet->AddParticle(RenderParticle);
		SdkIndex++;
	}

	NumSpawnedParticles = 0;
}

void FParticleSpritePhysXEmitterInstance::PostSpawnVerticalLod()
{
	FLOAT LodEmissionRate = 0.0f;
	FLOAT LodEmissionLife = 0.0f;

	//Clamp emission number with LOD
	if(LodEmissionBudget < INT_MAX)
	{
		FLOAT ParamBias = NxMath::clamp(PhysXTypeData.VerticalLod.SpawnLodRateVsLifeBias, 1.0f, 0.0f);
		
		FLOAT Alpha = ((FLOAT)LodEmissionBudget)/(SpawnEstimateRate*SpawnEstimateLife);
		
		LodEmissionRate = SpawnEstimateRate * NxMath::pow(Alpha, ParamBias);
		LodEmissionLife = SpawnEstimateLife * NxMath::pow(Alpha, 1.0f - ParamBias);

		LodEmissionRemainder += LodEmissionRate*GDeltaTime;
		
		FLOAT RemainderInt = NxMath::floor(LodEmissionRemainder);

		while(NumSpawnedParticles > RemainderInt)
		{
			RemoveParticleFromActives(ActiveParticles-1);
			NumSpawnedParticles--;
		}
		LodEmissionRemainder -= (FLOAT)NumSpawnedParticles;
	}
	else
	{
		LodEmissionRemainder = 0.0f;
	}

	//Clamp emission lifetime with LOD
	if (LodEmissionLife > 0.0f)
	{
		FLOAT OneOverLodEmissionLife = 1.0f/LodEmissionLife;
		INT FirstSpawnedIndex = ActiveParticles - NumSpawnedParticles;
		for(INT i = 0; i<NumSpawnedParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[FirstSpawnedIndex+i]);
			if(Particle.OneOverMaxLifetime < OneOverLodEmissionLife)
			{
				Particle.OneOverMaxLifetime = OneOverLodEmissionLife;
			}
		}
	}
}

/**
A lot of this code should be shared with the mesh emitter FillVolume, and should 
probably be made into a module.
*/
void FParticleSpritePhysXEmitterInstance::FillVolume(FRBVolumeFill & VolumeFill)
{
	NumSpawnedParticles = 0;
	
	INT Number = VolumeFill.Positions.Num();
	if(Number == 0)
	{
		return;
	}

	//Init velocities based on destructible buffers.
	INT RBStateIndex = -1;
	UBOOL bHaveRBState = FALSE;
	FVector COM(0,0,0);
	FVector LinVel(0,0,0);
	FVector AngVel(0,0,0);
	INT RBStateNum = 0;
	if( VolumeFill.RBStates.Num() )
	{
		RBStateIndex = VolumeFill.RBStates(0).Index;
	}

	OverwriteUnsupported();

	//This is almost a copy from  FLOAT FParticleEmitterInstance::Spawn(FLOAT DeltaTime)
	//const FLOAT SpawnTime = 0;

	// Handle growing arrays.
	UBOOL bProcessSpawn = TRUE;
	INT NewCount = ActiveParticles + Number;
	if (NewCount >= MaxActiveParticles)
	{
		bProcessSpawn = Resize((NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1), FALSE);
	}

	if (bProcessSpawn)
	{
		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);

		for(INT i=0; i<Number; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);
			const FLOAT SpawnTime = 0;

			PreSpawn(Particle);

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

			//execute FillVolume "module"
			Particle->Location = VolumeFill.Positions(i)*P2UScale;

			if (i == RBStateIndex)
			{
				FIndexedRBState IRBS = VolumeFill.RBStates(RBStateNum++);
				RBStateIndex = RBStateNum < VolumeFill.RBStates.Num() ? VolumeFill.RBStates(RBStateNum).Index : -1;
				COM = IRBS.CenterOfMass;
				LinVel = IRBS.LinearVelocity;
				AngVel = IRBS.AngularVelocity;
				bHaveRBState = TRUE;
			}
			if (bHaveRBState)
			{
				FVector V = LinVel + (AngVel^(Particle->Location - COM));
				Particle->Velocity += V;
			}		

			PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);
			
			ActiveParticles++;
			INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
		}
	}

	RestoreUnsupported();

	VolumeFill.Positions.Empty();
	VolumeFill.RBStates.Empty();

	NumSpawnedParticles = Number;
}

FLOAT FParticleSpritePhysXEmitterInstance::Spawn(FLOAT DeltaTime)
{
	FLOAT result = FParticleSpriteSubUVEmitterInstance::Spawn(DeltaTime);
	return result;
}

FLOAT FParticleSpritePhysXEmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	FLOAT result = FParticleSpriteSubUVEmitterInstance::Spawn(OldLeftover, Rate, DeltaTime, Burst, BurstTime);
	return result;
}

void FParticleSpritePhysXEmitterInstance::RemoveParticles()
{
	ActiveParticles = 0;
	NumSpawnedParticles = 0;
}

#define PHYSX_SPAWN_ESTIMATE_MAX 8

void FParticleSpritePhysXEmitterInstance::SpawnEstimateUpdate(FLOAT DeltaTime, INT NumSpawnedParticles, FLOAT LifetimeSum)
{	
	SpawnEstimateRate = 0.0f;
	SpawnEstimateLife = 0.0f;

	SpawnEstimateTime += DeltaTime;
	
	//A ringbuffer would be nice here...
	if(NumSpawnedParticles > 0)
	{
		if(SpawnEstimateSamples.Num() == PHYSX_SPAWN_ESTIMATE_MAX)
		{
			SpawnEstimateSamples.Pop();
		}

		if(SpawnEstimateSamples.Num() > 0)
		{
			SpawnEstimateSamples(0).DeltaTime = SpawnEstimateTime;
		}

		SpawnEstimateSamples.Insert(0);
		SpawnEstimateSample& NewSample = SpawnEstimateSamples(0);
		NewSample.DeltaTime = 0.0f;
		NewSample.NumSpawned = NumSpawnedParticles;
		NewSample.LifetimeSum = LifetimeSum;
		
		SpawnEstimateTime = 0.0f;
	}

	if(SpawnEstimateSamples.Num() > 0)
	{
		SpawnEstimateSamples(0).DeltaTime = SpawnEstimateTime;
		
		//Find mean sample time. (Don't include latest sample)
		FLOAT MeanDeltaTime = 0.0f;
		for(INT i=1; i<SpawnEstimateSamples.Num(); i++)
		{
			SpawnEstimateSample& Sample = SpawnEstimateSamples(i);
			MeanDeltaTime += Sample.DeltaTime;
		}
		if(MeanDeltaTime > 0.0f)
		{
			MeanDeltaTime /= (SpawnEstimateSamples.Num()-1);
		}


		INT StartIndex = 1;
		if(SpawnEstimateTime > MeanDeltaTime) //include current measurment
		{
			StartIndex = 0;
		}

		FLOAT WeightSum = 0.0f;
		FLOAT TimeSum = 0.0f;
		for(INT i=StartIndex; i<SpawnEstimateSamples.Num(); i++)
		{
			SpawnEstimateSample& Sample = SpawnEstimateSamples(i);
			TimeSum += Sample.DeltaTime;
			FLOAT Weight = 1.0f/(TimeSum);
			WeightSum += Weight;

			SpawnEstimateRate += Weight*Sample.NumSpawned;
			SpawnEstimateLife += Weight*Sample.LifetimeSum/Sample.NumSpawned;
		}

		if(WeightSum > 0.0f)
		{
			SpawnEstimateRate/=(WeightSum*MeanDeltaTime);
			SpawnEstimateLife/=WeightSum;
		}

	}

	//plotf( TEXT("DEBUG_VE_PLOT %x_SpawnEstimateRate %f"), this, SpawnEstimateRate);
	//plotf( TEXT("DEBUG_VE_PLOT %x_SpawnEstimateLife %f"), this, SpawnEstimateLife);
	//plotf( TEXT("DEBUG_VE_PLOT %x_NumSpawnedParticles %d"), this, NumSpawnedParticles);
	//plotf( TEXT("DEBUG_VE_PLOT %x_DeltaTime %f"), this, DeltaTime);
}

INT FParticleSpritePhysXEmitterInstance::GetSpawnVolumeEstimate()
{
	return appFloor(SpawnEstimateRate * SpawnEstimateLife);
}

void FParticleSpritePhysXEmitterInstance::SetEmissionBudget(INT Budget)
{
	LodEmissionBudget = Budget;
}

FLOAT FParticleSpritePhysXEmitterInstance::GetWeightForSpawnLod()
{
	check(PSet);
	return PSet->GetWeightForSpawnLod();
}

void FParticleSpritePhysXEmitterInstance::RemoveParticleFromActives(INT Index)
{
	if (Index < ActiveParticles)
	{
		// Swap the last particle for this one.
		WORD ParticleIndex = ParticleIndices[Index];
		ParticleIndices[Index] = ParticleIndices[ActiveParticles-1];
		ParticleIndices[ActiveParticles-1] = ParticleIndex;
		ActiveParticles--;
		INC_DWORD_STAT(STAT_SpriteParticlesKilled);
	}
}

void FParticleSpritePhysXEmitterInstance::OverwriteUnsupported()
{
	//Not supporting bUseLocalSpace:
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	Stored_bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
	LODLevel->RequiredModule->bUseLocalSpace = FALSE;	
}

void FParticleSpritePhysXEmitterInstance::RestoreUnsupported()
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	LODLevel->RequiredModule->bUseLocalSpace = Stored_bUseLocalSpace;
}

FDynamicEmitterDataBase* FParticleSpritePhysXEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	OverwriteUnsupported();
	FDynamicEmitterDataBase* Ret = FParticleSpriteSubUVEmitterInstance::GetDynamicData(bSelected);
	RestoreUnsupported();
	return Ret;
}

/**
 *	Updates the dynamic data for the instance
 *
 *	@param	DynamicData		The dynamic data to fill in
 *	@param	bSelected		TRUE if the particle system component is selected
 */
UBOOL FParticleSpritePhysXEmitterInstance::UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected)
{
	OverwriteUnsupported();
	UBOOL bResult = FParticleSpriteSubUVEmitterInstance::UpdateDynamicData(DynamicData, bSelected);
	RestoreUnsupported();
	return bResult;
}

#endif	//#if WITH_NOVODEX
