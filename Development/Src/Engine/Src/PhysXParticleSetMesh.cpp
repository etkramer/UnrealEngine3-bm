/*=============================================================================
	PhysXParticleSetMesh.cpp: PhysX Emitter Source.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EnginePhysicsClasses.h"

#if WITH_NOVODEX

#include "PhysXVerticalEmitter.h"
#include "PhysXParticleSystem.h"
#include "PhysXParticleSetMesh.h"

FPhysXParticleSetMesh::FPhysXParticleSetMesh(FPhysXMeshInstance& InMeshInstance) :
	FPhysXParticleSet(sizeof(PhysXRenderParticleMesh), InMeshInstance.PhysXTypeData.VerticalLod, InMeshInstance.PhysXTypeData.PhysXParSys),
	PhysXTypeData(InMeshInstance.PhysXTypeData),
	MeshInstance(InMeshInstance)
{
	check(InMeshInstance.PhysXTypeData.PhysXParSys);
}

FPhysXParticleSetMesh::~FPhysXParticleSetMesh()
{
}

void FPhysXParticleSetMesh::AsyncUpdate(FLOAT DeltaTime, UBOOL bProcessSimulationStep)
{
	FPhysXParticleSystem& PSys = GetPSys();
	FillInVertexBuffer(DeltaTime, PhysXTypeData.PhysXRotationMethod, PhysXTypeData.FluidRotationCoefficient);

	if(bProcessSimulationStep)
	{
		DeathRowManagment();
		AsyncParticleReduction(DeltaTime);
	}
}

void FPhysXParticleSetMesh::RemoveAllParticles()
{
	RemoveAllParticlesInternal();
	MeshInstance.ActiveParticles = 0;
}

/**
Here we assume that the particles of the render instance emitter have been updated.
*/
void FPhysXParticleSetMesh::FillInVertexBuffer(FLOAT DeltaTime, BYTE FluidRotationMethod, FLOAT FluidRotationCoefficient)
{
	FPhysXParticleSystem& PSys = GetPSys();
	MeshInstance.ActiveParticles = GetNumRenderParticles();
	TmpRenderIndices.Empty();

	if(GetNumRenderParticles() == 0)
		return;

	FParticleMeshPhysXEmitterInstance* SpawnInstance = MeshInstance.GetSpawnInstance();
	if(!SpawnInstance)
		return;

	FDynamicMeshEmitterData::FParticleInstancedMeshInstance *InstanceWriteBuffer = MeshInstance.PrepareInstanceWriteBuffer(GetMaxRenderParticles());

	if(!InstanceWriteBuffer)
		return;

	UParticleModuleSizeMultiplyLife* SizeMultiplyLifeModule = MeshInstance.GetSizeMultiplyLifeModule();
	PhysXParticle* SdkParticles = PSys.ParticlesSdk;
	PhysXParticleEx* SdkParticlesEx = PSys.ParticlesEx;
	
	FLOAT Temp = NxMath::clamp(PhysXTypeData.VerticalLod.RelativeFadeoutTime, 1.0f, 0.0f);
	FLOAT TimeUntilFadeout = 1.0f - Temp;

	if(SpawnInstance->MeshRotationActive)
	{
		const NxReal MaxRotPerStep = 2.5f;
		const NxReal Epsilon = 0.001f;
		
		// Use the rotation coefficient as the particle size variable.
		const NxReal InvParticleSize = FluidRotationCoefficient > Epsilon ? (1.0 / FluidRotationCoefficient) : 1.0f;
		
		// Retrieve global up axis.
		NxVec3 UpVector(0.0f, 0.0f, 1.0f);
		PSys.GetGravity(UpVector);
		UpVector = -UpVector;
		UpVector.normalize();
	
		NxVec3* SdkContacts = PSys.ParticleContactsSdk;
		check(SdkContacts);

		for (INT i=0; i<GetNumRenderParticles(); i++)
		{
			PhysXRenderParticleMesh& NParticle = *(PhysXRenderParticleMesh*)GetRenderParticle(i);
			PhysXParticle& SdkParticle = SdkParticles[NParticle.ParticleIndex];

			NParticle.RelativeTime += NParticle.OneOverMaxLifetime * DeltaTime;
			if(NParticle.RelativeTime >= TimeUntilFadeout && (NParticle.Flags & PhysXRenderParticleMesh::PXRP_DeathQueue) == 0)
				TmpRenderIndices.Push(i);

			NxVec3 Size = *reinterpret_cast<NxVec3*>(&NParticle.Size.X);
			if (SizeMultiplyLifeModule)
			{
				FVector SizeScale = SizeMultiplyLifeModule->LifeMultiplier.GetValue(NParticle.RelativeTime, SpawnInstance->Component);

				if (SizeMultiplyLifeModule->MultiplyX)
					Size.x *= SizeScale.X;

				if (SizeMultiplyLifeModule->MultiplyY)
					Size.y *= SizeScale.Y;

				if (SizeMultiplyLifeModule->MultiplyZ)
					Size.z *= SizeScale.Z;
			}

			FVector	Pos = N2UPosition(SdkParticle.Pos);

			switch(FluidRotationMethod)
			{
				case PMRM_Spherical:
				{
					NxVec3             vel  = SdkParticle.Vel;
					NxU32              id   = NParticle.Id;
					vel.z = 0; // Project onto xy plane.
					NxReal velmag = vel.magnitude();
					if(velmag > Epsilon)
					{
						NxVec3 avel;
						avel.cross(vel, UpVector);
						NxReal avelm = avel.normalize();
						if(avelm > Epsilon)
						{
							// Set magnitude to magnitude of linear motion (later maybe clamp)
							avel *= -velmag;

							NxReal w = velmag;
							NxReal v = (NxReal)DeltaTime*w*FluidRotationCoefficient;
							NxReal q = NxMath::cos(v);
							NxReal s = NxMath::sin(v)/w;

							NxQuat& Rot = *reinterpret_cast<NxQuat*>(&NParticle.Rot.X);
							Rot.multiply(NxQuat(avel*s,q), Rot);
							Rot.normalize();
						}
					}
				}
				break;

				case PMRM_Box:
				case PMRM_LongBox:
				case PMRM_FlatBox:
				{
					const NxVec3& Vel = *reinterpret_cast<NxVec3*>(&SdkParticle.Vel);
					NxQuat& Rot       = *reinterpret_cast<NxQuat*>(&NParticle.Rot.X);
					NxVec3& AngVel    = *reinterpret_cast<NxVec3*>(&NParticle.AngVel.X);

					const NxVec3 &Contact = SdkContacts[NParticle.ParticleIndex];

					NxReal VelMagSqr  = Vel.magnitudeSquared();
					NxReal LimitSqr   = VelMagSqr * InvParticleSize * InvParticleSize;


					NxVec3 PoseCorrection(0.0f);		// Rest pose correction.

					// Check bounce...
					if(Contact.magnitudeSquared() > Epsilon)
					{
						NxVec3 UpVector = Contact; // So we can rest on a ramp.
						UpVector.normalize();
						NxVec3 t = Vel - UpVector * UpVector.dot(Vel);
						AngVel = -t.cross(UpVector) * InvParticleSize;

						NxMat33 rot33;
						rot33.fromQuat(Rot);
						NxVec3  Up(0.0f);

						switch(FluidRotationMethod)
						{
							case PMRM_FlatBox:
							{
								Up = rot33.getColumn(2);
								if(Up.z < 0)
								{
									Up = -Up;
								}
								break;
							}
							default:
							{
								NxReal Best = 0;
								for(int j = (FluidRotationMethod == PMRM_LongBox ? 1 : 0); j < 3; j++)
								{
									NxVec3 tmp = rot33.getColumn(j);
									NxReal d   = tmp.dot(UpVector);
									if(d > Best)
									{
										Up   = tmp;
										Best = d;
									}
									if(-d > Best)
									{
										Up   = -tmp;
										Best = -d;
									}
								}
								break;
							}
						}

						PoseCorrection = Up.cross(UpVector);
						NxReal Mag = PoseCorrection.magnitude();
						NxReal MaxMag = 0.5f / (1.0f + NxMath::sqrt(LimitSqr));
						if(Mag > MaxMag)
						{
							PoseCorrection *= MaxMag / Mag;
						}
					}

					// Limit angular velocity.
					NxReal MagSqr = AngVel.magnitudeSquared();
					if(MagSqr > LimitSqr)
					{
						AngVel *= NxMath::sqrt(LimitSqr) / NxMath::sqrt(MagSqr);
					}
					
					// Integrate rotation.
					NxVec3 DeltaRot = AngVel * (NxReal)DeltaTime;
					
					// Apply combined rotation.
					NxVec3 Axis  = DeltaRot + PoseCorrection;
					NxReal Angle = Axis.normalize();
					if(Angle > Epsilon)
					{
						if(Angle > MaxRotPerStep)
						{
							Angle = MaxRotPerStep;
						}
						NxQuat TempRot;
						TempRot.fromAngleAxisFast(Angle, Axis);
						Rot = TempRot * Rot;
					}
				}
				break;
			}

			FRotationMatrix kRotMat(NParticle.Rot);

			InstanceWriteBuffer[i].Location = Pos;
			InstanceWriteBuffer[i].XAxis = FVector(Size.x * kRotMat.M[0][0], Size.x * kRotMat.M[0][1], Size.x * kRotMat.M[0][2]);
			InstanceWriteBuffer[i].YAxis = FVector(Size.y * kRotMat.M[1][0], Size.y * kRotMat.M[1][1], Size.y * kRotMat.M[1][2]);
			InstanceWriteBuffer[i].ZAxis = FVector(Size.z * kRotMat.M[2][0], Size.z * kRotMat.M[2][1], Size.z * kRotMat.M[2][2]);
		}
	}
	else
	{
		//Debug
		for (INT i=0; i<GetNumRenderParticles(); i++)
		{
			PhysXRenderParticleMesh& NParticle = *(PhysXRenderParticleMesh*)GetRenderParticle(i);
			PhysXParticle& SdkParticle = SdkParticles[NParticle.ParticleIndex];

			NParticle.RelativeTime += NParticle.OneOverMaxLifetime * DeltaTime;
			if(NParticle.RelativeTime >= TimeUntilFadeout && (NParticle.Flags & PhysXRenderParticleMesh::PXRP_DeathQueue) == 0)
				TmpRenderIndices.Push(i);

			NxVec3	Size = *reinterpret_cast<NxVec3*>(&NParticle.Size.X);
			if (SizeMultiplyLifeModule)
			{
				FVector SizeScale = SizeMultiplyLifeModule->LifeMultiplier.GetValue(NParticle.RelativeTime, SpawnInstance->Component);

				if (SizeMultiplyLifeModule->MultiplyX)
					Size.x *= SizeScale.X;

				if (SizeMultiplyLifeModule->MultiplyY)
					Size.y *= SizeScale.Y;

				if (SizeMultiplyLifeModule->MultiplyZ)
					Size.z *= SizeScale.Z;
			}

			InstanceWriteBuffer[i].Location = N2UPosition(SdkParticle.Pos);
			InstanceWriteBuffer[i].XAxis = FVector(Size.x, 0.0f, 0.0f);
			InstanceWriteBuffer[i].YAxis = FVector(0.0f, Size.y, 0.0f);
			InstanceWriteBuffer[i].ZAxis = FVector(0.0f, 0.0f, Size.z);
		}
	}
}

/*-----------------------------------------------------------------------------
	FPhysXMeshInstance implementation.
-----------------------------------------------------------------------------*/

FParticleMeshPhysXEmitterInstance* FPhysXMeshInstance::GetSpawnInstance()
{
	if(SpawnInstances.Num() == 0)
		return NULL;

	check(SpawnInstances(0));
	FParticleMeshPhysXEmitterInstance* SpawnInstance = SpawnInstances(0);
	return SpawnInstance;
}

FPhysXMeshInstance::FPhysXMeshInstance(class UParticleModuleTypeDataMeshPhysX &TypeData) :
	PhysXTypeData(TypeData),
	ActiveParticles(0),
	DynamicEmitter(NULL),
	InstBufDesc(NULL)
{
	PSet = new FPhysXParticleSetMesh(*this);
	check(PSet);
}

FPhysXMeshInstance::~FPhysXMeshInstance()
{
	PhysXTypeData.RenderInstance = NULL;
	check(PSet);

	if(PhysXTypeData.PhysXParSys && PhysXTypeData.PhysXParSys->PSys)
		PhysXTypeData.PhysXParSys->PSys->RemoveParticleSet(PSet);
		
	if(InstBufDesc)
	  InstBufDesc->DecRefCount();

	delete PSet;
	PSet = NULL;
}

UParticleModuleSizeMultiplyLife* FPhysXMeshInstance::GetSizeMultiplyLifeModule()
{
	FParticleMeshPhysXEmitterInstance* SpawnInstance = GetSpawnInstance();
	if(SpawnInstance == NULL) 
		return NULL;

	UParticleModuleSizeMultiplyLife* SMLM = NULL;
	UParticleLODLevel* LODLevel = SpawnInstance->SpriteTemplate->GetCurrentLODLevel(SpawnInstance);
	for (INT ModuleIndex = 0; ModuleIndex < LODLevel->UpdateModules.Num(); ModuleIndex++)
	{
		UParticleModule* HighModule	= LODLevel->UpdateModules(ModuleIndex);
		if (HighModule && HighModule->bEnabled && HighModule->IsSizeMultiplyLife())
		{
			SMLM = (UParticleModuleSizeMultiplyLife *)HighModule;
			break;
		}
	}
	return SMLM;
}

FDynamicMeshEmitterData::FParticleInstancedMeshInstance *FPhysXMeshInstance::PrepareInstanceWriteBuffer(INT CurrentMaxParticles)
{
	if(!InstBufDesc)
	{
		InstBufDesc = new FDynamicMeshEmitterData::FInstanceBufferDesc;
		InstBufDesc->IncRefCount();	
	
		BeginInitResource( InstBufDesc );	
	}

	FDynamicMeshEmitterData::FParticleInstancedMeshInstance *InstanceWriteBuffer = NULL;
	AllocateDynamicEmitter(CurrentMaxParticles);

	if (!DynamicEmitter)
		return NULL;

	if ((DynamicEmitter->Source.EmitterRenderMode == ERM_Normal) &&
		InstBufDesc->GetNextBuffer(ActiveParticles,
			(FVertexBufferRHIRef **)&(DynamicEmitter->InstBuf),
			(void **)&InstanceWriteBuffer))
	{
		DynamicEmitter->bInstBufIsVB = TRUE;
	}
	else
	{
		DynamicEmitter->InstBuf = appMalloc(ActiveParticles * sizeof(FDynamicMeshEmitterData::FParticleInstancedMeshInstance));
		DynamicEmitter->bInstBufIsVB = FALSE;
		InstanceWriteBuffer = (FDynamicMeshEmitterData::FParticleInstancedMeshInstance *)DynamicEmitter->InstBuf;
	}
	DynamicEmitter->NumInst = ActiveParticles;
	return InstanceWriteBuffer;
}

void FPhysXMeshInstance::AllocateDynamicEmitter(INT CurrentMaxParticles)
{
	// (JPB) The value of bSelected (selected in the editor) was provided by GetDynamicData(),
	// but since we are allocating the emitter earlier, we don't have this information.
	// For now, just set to FALSE.
	UBOOL bSelected = FALSE;

	FParticleMeshPhysXEmitterInstance* SpawnInstance = GetSpawnInstance();
	check(SpawnInstance);

	DynamicEmitter = static_cast< FDynamicMeshEmitterData* >( SpawnInstance->GetDynamicData( bSelected ) );
	if( DynamicEmitter != NULL )
	{
		DynamicEmitter->bUseNxFluid = TRUE;
	
		DynamicEmitter->InstBufDesc = InstBufDesc;
		InstBufDesc->IncRefCount();
	}
}


/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FPhysXMeshInstance::GetDynamicData(UBOOL bSelected)
{
	FDynamicMeshEmitterData* NewEmitterData = DynamicEmitter;
	DynamicEmitter = NULL;

	return NewEmitterData;
}

#endif	//#if WITH_NOVODEX
