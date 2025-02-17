/*=============================================================================
	ParticlePhysXMeshEmitterInstance.cpp: PhysX Emitter Source.
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
#include "PhysXParticleSetMesh.h"

IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleMeshPhysXEmitterInstance);

FParticleMeshPhysXEmitterInstance::FParticleMeshPhysXEmitterInstance(class UParticleModuleTypeDataMeshPhysX &TypeData) : 
	PhysXTypeData(TypeData), 
	NumSpawnedParticles(0),
	SpawnEstimateTime(0.0f)
{
	bUseNxFluid = TRUE;
	SpawnEstimateRate = 0.0f;
	SpawnEstimateLife = 0.0f;
	LodEmissionBudget = INT_MAX;
	LodEmissionRemainder = 0.0f;
}

FParticleMeshPhysXEmitterInstance::~FParticleMeshPhysXEmitterInstance()
{
	CleanUp();
}

void FParticleMeshPhysXEmitterInstance::CleanUp()
{
	PhysXTypeData.TryRemoveRenderInstance(this);
	
	if(PhysXTypeData.PhysXParSys)
	{
		PhysXTypeData.PhysXParSys->RemoveSpawnInstance(this);	
	}
}

void FParticleMeshPhysXEmitterInstance::RemovedFromScene()
{
	CleanUp();
}

FPhysXParticleSystem& FParticleMeshPhysXEmitterInstance::GetPSys()
{
	check(PhysXTypeData.PhysXParSys && PhysXTypeData.PhysXParSys->PSys);
	return *PhysXTypeData.PhysXParSys->PSys;
}

UBOOL FParticleMeshPhysXEmitterInstance::TryConnect()
{
	if(Component->bWarmingUp)
		return FALSE;

	if(!PhysXTypeData.PhysXParSys)
		return FALSE;

	if(!PhysXTypeData.PhysXParSys->TryConnect())
		return FALSE;

	return TRUE;
}

FRBVolumeFill* FParticleMeshPhysXEmitterInstance::GetVolumeFill()
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

void FParticleMeshPhysXEmitterInstance::AsyncProccessSpawnedParticles(FLOAT DeltaTime)
{
	FPhysXParticleSystem& PSys = GetPSys();
	FBaseParticle *pBaseParticle = (FBaseParticle*)ParticleData;
	FVector LODOrigin(0,0,0);
	BOOL bUseDistCulling = GetLODOrigin(LODOrigin);

	FLOAT LifetimeSum = 0.0f;

	//Filter particle for LOD and convert some data.
	INT LastParticleIndex = ActiveParticles-1;
	for(INT i=LastParticleIndex; i>=0; i--)
	{
		FBaseParticle* Particle = (FBaseParticle*)(((BYTE*)pBaseParticle) + ParticleStride*i);

		if(bUseDistCulling && PSys.GetLODDistanceSq(LODOrigin, Particle->Location) > PSys.VerticalPacketRadiusSq)
		{
			FBaseParticle* LastParticle = (FBaseParticle*)(((BYTE*)pBaseParticle) + ParticleStride*LastParticleIndex);
			FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)Particle + MeshRotationOffset);
			FMeshRotationPayloadData* LastPayloadData = (FMeshRotationPayloadData*)((BYTE*)LastParticle + MeshRotationOffset);
			*Particle = *LastParticle;
			*PayloadData = *LastPayloadData;
			LastParticleIndex--;
			continue;
		}
		*(NxVec3*)&Particle->Location.X = U2NPosition(Particle->Location);
		*(NxVec3*)&Particle->Velocity.X = U2NPosition(Particle->Velocity);
		LifetimeSum += (Particle->OneOverMaxLifetime > 0.0f)?(1.0f/Particle->OneOverMaxLifetime):0.0f;
	}
	NumSpawnedParticles = LastParticleIndex+1;
	SpawnEstimateUpdate(DeltaTime, NumSpawnedParticles, LifetimeSum);
}

void FParticleMeshPhysXEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	if(!TryConnect())
	{
		return;
	}

	PhysXTypeData.TryCreateRenderInstance(SpriteTemplate, this);

	FRBPhysScene* Scene = PhysXTypeData.PhysXParSys->GetScene();
	check(Scene);

	Scene->PhysXEmitterManager->Tick(DeltaTime);

	//This relies on NxCompartment::setTiming() be called befor this tick.
	DeltaTime = Scene->PhysXEmitterManager->GetEffectiveStepSize();

	FPhysXParticleSystem& PSys = GetPSys();
	PSys.AddSpawnInstance(this);
	ActiveParticles = 0;
	NumSpawnedParticles = 0;

	if(!GetVolumeFill())
	{
		OverwriteUnsupported();
		FParticleMeshEmitterInstance::Tick(DeltaTime, bSuppressSpawning);
		RestoreUnsupported();
		AsyncProccessSpawnedParticles(DeltaTime);
	}

	//Hack active particles in order get stuff rendered...
	ActiveParticles = PhysXTypeData.RenderInstance->ActiveParticles;

	//Pick up bounds from FPhysXParticleSystem
	ParticleBoundingBox = PSys.GetWorldBounds();
}

FParticleSystemSceneProxy* FParticleMeshPhysXEmitterInstance::GetSceneProxy()
{
	if(!Component)
		return NULL;

	return (FParticleSystemSceneProxy*)Scene_GetProxyFromInfo(Component->SceneInfo);
}

UBOOL FParticleMeshPhysXEmitterInstance::GetLODOrigin(FVector& OutLODOrigin)
{
	FParticleSystemSceneProxy* SceneProxy = GetSceneProxy();
	if(SceneProxy)
	{
		OutLODOrigin = SceneProxy->GetLODOrigin();
	}
	return SceneProxy != NULL;
}

UBOOL FParticleMeshPhysXEmitterInstance::GetLODNearClippingPlane(FPlane& OutClippingPlane)
{
	FParticleSystemSceneProxy* SceneProxy = GetSceneProxy();
	if(!SceneProxy)
	{
		return FALSE;
	}
	UBOOL HasCP = SceneProxy->GetNearClippingPlane(OutClippingPlane);
	return HasCP;
}

FDynamicEmitterDataBase* FParticleMeshPhysXEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	PhysXTypeData.TryCreateRenderInstance(SpriteTemplate, this);
	check(PhysXTypeData.RenderInstance);
	return PhysXTypeData.RenderInstance->GetDynamicData(bSelected);
}

/**
 *	Updates the dynamic data for the instance
 *
 *	@param	DynamicData		The dynamic data to fill in
 *	@param	bSelected		TRUE if the particle system component is selected
 */
UBOOL FParticleMeshPhysXEmitterInstance::UpdateDynamicData(FDynamicEmitterDataBase* DynamicData, UBOOL bSelected)
{
	checkf(0, TEXT("PhysXEmitters not supported in DoubleBuffering!"));
	return FALSE;
}

void SetRotation(PhysXRenderParticleMesh& RenderParticle, const FMeshRotationPayloadData& PayloadData)
{
	//This still seems kinda broken, but less.
	FRotator Rotator;
	Rotator = FRotator::MakeFromEuler(PayloadData.Rotation);
	RenderParticle.Rot = Rotator.Quaternion();
	RenderParticle.Rot.Normalize();

	Rotator = FRotator::MakeFromEuler(PayloadData.RotationRateBase);

	//x:roll, y:pitch, z:yaw
	//Wx = rollrate - yawrate * sin(pitch)
	//Wy = pitchrate * cos(roll) + yawrate * sin(roll) * cos(pitch) 
	//Wz = yawrate * cos(roll) * cos(pitch) - pitchrate * sin(roll)

	FVector Rot = PayloadData.Rotation*(2*PI)/360.0f;
	FVector Rate = PayloadData.RotationRateBase*(2*PI)/360.0f;

	RenderParticle.AngVel.X = Rate.X - Rate.Z*appSin(Rot.Y);
	RenderParticle.AngVel.Y = Rate.Y * appCos(Rot.X) + Rate.Z * appSin(Rot.X) * appCos(Rot.Y);
	RenderParticle.AngVel.Z = Rate.Z * appCos(Rot.X) * appCos(Rot.Y) - Rate.Y*appSin(Rot.X);
}

/**
Assumes the base class emitter instance has spawned particles, which now need to be added to the 
FPhysXParticleSystem and to the RenderInstance.

UE Particle lifetimes are converted to PhysX SDK lifetimes.
Adds particles to the PhysXParticle system and creates entries in the RenderInstance 
with SDK particle Ids.

Sets appropriate initial sizes and max lifetimes for the new particles in the RenderInstance.
*/
void FParticleMeshPhysXEmitterInstance::SpawnSyncPhysX()
{
	ActiveParticles = 0;
	PhysXTypeData.TryCreateRenderInstance(SpriteTemplate, this);

	FRBVolumeFill * VolumeFill = GetVolumeFill();
	if(VolumeFill)
	{
		FillVolume(*VolumeFill);
	}
	else
	{
		FLOAT LodEmissionRate = 0.0f;
		FLOAT LodEmissionLife = 0.0f;

		//Clamp emission with LOD
		if(LodEmissionBudget < INT_MAX)
		{
			FLOAT ParamBias = NxMath::clamp(PhysXTypeData.VerticalLod.SpawnLodRateVsLifeBias, 1.0f, 0.0f);
			FLOAT Alpha = ((FLOAT)LodEmissionBudget)/(SpawnEstimateRate*SpawnEstimateLife);
			
			LodEmissionRate = SpawnEstimateRate * NxMath::pow(Alpha, ParamBias);
			LodEmissionLife = SpawnEstimateLife * NxMath::pow(Alpha, 1.0f - ParamBias);

			LodEmissionRemainder += LodEmissionRate*GDeltaTime;
			
			FLOAT RemainderInt = NxMath::floor(LodEmissionRemainder);

			if(NumSpawnedParticles > RemainderInt)
			{
				NumSpawnedParticles = (INT)RemainderInt;
				LodEmissionRemainder -= RemainderInt;
			}
			else
			{
				LodEmissionRemainder -= (FLOAT)NumSpawnedParticles;
			}
		}
		else
		{
			LodEmissionRemainder = 0.0f;
		}

		//plotf( TEXT("DEBUG_VE_PLOT numSpawnedEmitterRateClamp %d"), NumSpawnedParticles);

		if(NumSpawnedParticles == 0)
			return;

		FPhysXParticleSystem& PSys = GetPSys();
		FBaseParticle *pBaseParticle = (FBaseParticle*)ParticleData;

		NxParticleData particleData;
  		particleData.bufferPos = (NxF32*)&pBaseParticle->Location.X;
  		particleData.bufferVel = (NxF32*)&pBaseParticle->Velocity.X;
   		particleData.numParticlesPtr = (NxU32*)&NumSpawnedParticles;
  		particleData.bufferPosByteStride = ParticleStride;
  		particleData.bufferVelByteStride = ParticleStride;
   		check(particleData.isValid());

		check(PhysXTypeData.RenderInstance->PSet);
		FPhysXParticleSetMesh* PSet = PhysXTypeData.RenderInstance->PSet;
		
		INT SdkIndex = PSys.NumParticlesSdk;
		INT NumCreated = PSys.AddParticles(particleData, PSet);

		FLOAT OneOverLodEmissionLife = (LodEmissionLife > 0.0f) ? 1.0f/LodEmissionLife : 0.0f;
		//plotf( TEXT("DEBUG_VE_PLOT LodEmissionLife %f"), LodEmissionLife/GDeltaTime);

		//Creation for RenderInstance
		FRotator Rotator; 
		for(INT i=0; i<NumCreated; i++)
		{
			FBaseParticle& Particle = *(FBaseParticle*)(((BYTE*)pBaseParticle) + ParticleStride*i);
			PhysXParticle& SdkParticle = PSys.ParticlesSdk[SdkIndex];
			PhysXRenderParticleMesh RenderParticle;
			
			RenderParticle.Id = SdkParticle.Id;
			RenderParticle.ParticleIndex = SdkIndex;
			RenderParticle.Size = Particle.BaseSize;

			if(Particle.OneOverMaxLifetime < OneOverLodEmissionLife)
			{
				RenderParticle.OneOverMaxLifetime = OneOverLodEmissionLife;
			}
			else
			{
				RenderParticle.OneOverMaxLifetime = Particle.OneOverMaxLifetime;
			}

			RenderParticle.RelativeTime = 0.0f;


			if(MeshRotationActive)
			{
				FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
				SetRotation(RenderParticle, *PayloadData);
			}
			else
			{
				RenderParticle.Rot = FQuat::Identity;
				RenderParticle.AngVel = FVector(0,0,0);
			}

			PSet->AddParticle(RenderParticle);
			SdkIndex++;
		}

		//debugf( TEXT("DEBUG_VE SpawnSyncPhysX(): PSys.NumParticles %d, PSys.GetNumParticles() %d, NumCreated %d"), PSys.NumParticles, PSys.GetNumParticles(), NumCreated);

		NumSpawnedParticles = 0;
	}

	ActiveParticles = 0;
}

void FParticleMeshPhysXEmitterInstance::FillVolume(FRBVolumeFill & VolumeFill)
{
	INT NumParticles = VolumeFill.Positions.Num();
	if(!NumParticles)
	{
		return;
	}

	if(!FParticleMeshEmitterInstance::Resize(NumParticles, FALSE))
		return;

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
	
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	const FLOAT SpawnTime = 0;

	OverwriteUnsupported();

	check(ActiveParticles == 0);
	FBaseParticle *pBaseParticle = (FBaseParticle*)ParticleData;
	for(INT i=0; i<NumParticles; i++)
	{
		FBaseParticle* Particle = (FBaseParticle*)(((BYTE*)pBaseParticle) + ParticleStride*i);
		memset(Particle, 0, sizeof(FBaseParticle));
		FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)Particle + MeshRotationOffset);

		if(MeshRotationActive)
		{
			memset(PayloadData, 0, sizeof(FMeshRotationPayloadData));
		}

		//Execute spawn modules
		INT NumSpawnModules = LODLevel->SpawnModules.Num();
		for(INT ModIndex=0; ModIndex<NumSpawnModules; ModIndex++)
		{
			UParticleModule *Module = LODLevel->SpawnModules(ModIndex);
			check(Module);
			if(Module && Module->bEnabled)
			{
				Module->Spawn(this, 0, SpawnTime);
			}
		}

		Particle->Location = VolumeFill.Positions(i);
		*(NxVec3*)&Particle->Velocity.X = U2NPosition(Particle->Velocity);

		if( i == RBStateIndex )
		{
			FIndexedRBState IRBS = VolumeFill.RBStates(RBStateNum++);
			RBStateIndex = RBStateNum < VolumeFill.RBStates.Num() ? VolumeFill.RBStates(RBStateNum).Index : -1;
			COM = IRBS.CenterOfMass;
			LinVel = IRBS.LinearVelocity;
			AngVel = IRBS.AngularVelocity;
			bHaveRBState = TRUE;
		}
		if( bHaveRBState )
		{
			FVector V = LinVel + (AngVel^(Particle->Location - COM));
			*(NxVec3*)&Particle->Velocity.X += U2NPosition(V);
		}		

		ActiveParticles++;
	}

	RestoreUnsupported();

	NxParticleData particleData;
	particleData.bufferPos = (NxF32*)&pBaseParticle->Location.X;
	particleData.bufferVel = (NxF32*)&pBaseParticle->Velocity.X;
	particleData.numParticlesPtr = (NxU32*)&NumParticles;
	particleData.bufferPosByteStride = ParticleStride;
	particleData.bufferVelByteStride = ParticleStride;
	check(particleData.isValid());

	check(PhysXTypeData.RenderInstance->PSet);
	FPhysXParticleSetMesh* PSet = PhysXTypeData.RenderInstance->PSet;
	FPhysXParticleSystem& PSys = GetPSys();

	INT SdkIndex = PSys.NumParticlesSdk;
	INT NumCreated = PSys.AddParticles(particleData, PSet);

	//Creation for RenderInstance
	FRotator Rotator; 
	for(INT i=0; i<NumCreated; i++)
	{
		FBaseParticle& Particle = *(FBaseParticle*)(((BYTE*)pBaseParticle) + ParticleStride*i);
		PhysXParticle& SdkParticle = PSys.ParticlesSdk[SdkIndex];

		PhysXRenderParticleMesh RenderParticle;
		
		RenderParticle.Id = SdkParticle.Id;
		RenderParticle.ParticleIndex = SdkIndex;
		
		RenderParticle.Size = Particle.BaseSize;
		RenderParticle.OneOverMaxLifetime = Particle.OneOverMaxLifetime;
		RenderParticle.RelativeTime = 0.0f;

		if(MeshRotationActive)
		{
			FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
			SetRotation(RenderParticle, *PayloadData);
		}
		else
		{
			RenderParticle.Rot = FQuat::Identity;
			RenderParticle.AngVel = FVector(0,0,0);
		}

		PSet->AddParticle(RenderParticle);
		SdkIndex++;
	}

	VolumeFill.Positions.Empty();
	VolumeFill.RBStates.Empty();
}

FLOAT FParticleMeshPhysXEmitterInstance::Spawn(FLOAT DeltaTime)
{
	FLOAT result = FParticleMeshEmitterInstance::Spawn(DeltaTime);
	return result;
}

FLOAT FParticleMeshPhysXEmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	FLOAT result = FParticleMeshEmitterInstance::Spawn(OldLeftover, Rate, DeltaTime, Burst, BurstTime);
	return result;
}

void FParticleMeshPhysXEmitterInstance::RemoveParticles()
{
	ActiveParticles = 0;
	NumSpawnedParticles = 0;
}

void FParticleMeshPhysXEmitterInstance::ParticlePrefetch() {}

#define PHYSX_SPAWN_ESTIMATE_MAX 8

void FParticleMeshPhysXEmitterInstance::SpawnEstimateUpdate(FLOAT DeltaTime, INT NumSpawnedParticles, FLOAT LifetimeSum)
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

INT FParticleMeshPhysXEmitterInstance::GetSpawnVolumeEstimate()
{
	return appFloor(SpawnEstimateRate * SpawnEstimateLife);
}

void FParticleMeshPhysXEmitterInstance::SetEmissionBudget(INT Budget)
{
	LodEmissionBudget = Budget;
}

FLOAT FParticleMeshPhysXEmitterInstance::GetWeightForSpawnLod()
{
	check(PhysXTypeData.RenderInstance->PSet);
	return PhysXTypeData.RenderInstance->PSet->GetWeightForSpawnLod();
}

void FParticleMeshPhysXEmitterInstance::OverwriteUnsupported()
{
	//Not supporting bUseLocalSpace:
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	Stored_bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
	LODLevel->RequiredModule->bUseLocalSpace = FALSE;	
}

void FParticleMeshPhysXEmitterInstance::RestoreUnsupported()
{
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	LODLevel->RequiredModule->bUseLocalSpace = Stored_bUseLocalSpace;
}


/**
 * Captures dynamic replay data for this particle system.
 *
 * @param	OutData		[Out] Data will be copied here
 *
 * @return Returns TRUE if successful
 */
UBOOL FParticleMeshPhysXEmitterInstance::FillReplayData( FDynamicEmitterReplayDataBase& OutData )
{
	// Call parent implementation first to fill in common particle source data
	// NOTE: We skip the Mesh implementation since we're replacing it with our own here
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


	OutData.eEmitterType = DET_Mesh;

	FDynamicMeshEmitterReplayData* NewReplayData =
		static_cast< FDynamicMeshEmitterReplayData* >( &OutData );


	// Get the material instance. If none is present, use the DefaultMaterial
	UMaterialInterface* MatInterface = NULL;
	if (LODLevel->TypeDataModule)
	{
		UParticleModuleTypeDataMesh* MeshTD = CastChecked<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
		if (MeshTD->bOverrideMaterial == TRUE)
		{
			if (CurrentMaterial)
			{
				MatInterface = CurrentMaterial;
			}
		}
	}
	NewReplayData->MaterialInterface = MatInterface;


	// If there are orbit modules, add the orbit module data
	if (LODLevel->OrbitModules.Num() > 0)
	{
		UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
		UParticleModuleOrbit* LastOrbit = HighestLODLevel->OrbitModules(LODLevel->OrbitModules.Num() - 1);
		check(LastOrbit);

		UINT* LastOrbitOffset = ModuleOffsetMap.Find(LastOrbit);
		NewReplayData->OrbitModuleOffset = *LastOrbitOffset;
	}



	// Mesh settings
	NewReplayData->bScaleUV = LODLevel->RequiredModule->bScaleUV;
	NewReplayData->SubUVInterpMethod = LODLevel->RequiredModule->InterpolationMethod;
	NewReplayData->SubUVDataOffset = SubUVDataOffset;
	NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
	NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
	NewReplayData->MeshRotationOffset = MeshRotationOffset;
	NewReplayData->bMeshRotationActive = MeshRotationActive;
	NewReplayData->MeshAlignment = MeshTypeData->MeshAlignment;


	// Never use 'local space' for PhysX meshes
	NewReplayData->bUseLocalSpace = FALSE;


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
		NewReplayData->Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			NewReplayData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
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


#endif	//#if WITH_NOVODEX
