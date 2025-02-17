/*=============================================================================
	PhysXParticleSystem.cpp: PhysX Emitter Source.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "UnNovodexSupport.h"
#include "PhysXParticleSystem.h"
#include "PhysXVerticalEmitter.h"
#include "PhysXParticleSetMesh.h"

IMPLEMENT_CLASS(UPhysXParticleSystem)

void UPhysXParticleSystem::FinishDestroy()
{
	//check(!CascadeScene);
	RemovedFromScene();

#if WITH_NOVODEX
	if(GWorld && GWorld->RBPhysScene)
	{
		GWorld->RBPhysScene->PhysXEmitterManager->RemoveParticleSystem(this);
		UBOOL Disconnected = SyncDisconnect();
		check(Disconnected);
	}
#endif	//#if WITH_NOVODEX
	Super::FinishDestroy();
}

//Particle System Side Distribution
//static FILE* DistTestFile = NULL;

UBOOL UPhysXParticleSystem::SyncConnect()
{
#if WITH_NOVODEX
	check(!PSys);

	if(bSyncFailed) 
		return FALSE;

	PSys = new FPhysXParticleSystem(*this);
	check(PSys);

	if(!PSys->IsSyncedToSdk())
	{
		GWarn->Logf(NAME_Warning, TEXT("FPhysXParticleSystem %s failed to be created in PhysX Sdk!"), *GetName());
		delete PSys;
		PSys = NULL;
		bSyncFailed = TRUE;
		return FALSE;
	}

	bDestroy = FALSE;
	check(PSys);

	//if(DistTestFile)
	//{
	//	fclose(DistTestFile);
	//}
	//fopen_s(&DistTestFile, "DistTestFile.txt","w");
#endif	//#if WITH_NOVODEX

	return TRUE;
}

UBOOL UPhysXParticleSystem::SyncDisconnect()
{
#if WITH_NOVODEX
	if(!bDestroy)
		return FALSE;

	check(PSys);
	delete PSys;
	PSys = NULL;
	bSyncFailed = FALSE;
	bDestroy = FALSE;

	if(CascadeScene)
	{
		DestroyRBPhysScene(CascadeScene);
		CascadeScene = NULL;
	}
#endif	//#if WITH_NOVODEX
	return TRUE;
}

/**
Makes sure that a FPhysXParticleSystem is created and linked to an 
appropriate scene, and it's vertical emitter manager. Returns FALSE 
on failure, and TRUE on success. 
To which scene the FPhysXParticleSystem is connected, depends on running 
Cascade or Game. If an existing connection needs to be broken down, due 
to change from Cascade to Game or vice versa, FALSE is returned.
*/
UBOOL UPhysXParticleSystem::TryConnect()
{
#if WITH_NOVODEX
	FRBPhysScene* Scene = GetScene();

	if(!Scene)
	{
		return FALSE;
	}

	if(!PSys)
	{
		Scene->PhysXEmitterManager->AddParticleSystem(this);
		if(CascadeScene)
		{
			//Sync with PhysX objects
			Scene->PhysXEmitterManager->SyncPhysXData();
		}
		return FALSE;
	}

	if(CascadeScene && bIsInGame)
	{
		RemovedFromScene();
		return FALSE;
	}
#endif	//#if WITH_NOVODEX
	return TRUE;
}

FRBPhysScene* UPhysXParticleSystem::GetScene()
{
	FRBPhysScene* Scene = NULL;
#if WITH_NOVODEX
	if(GIsEditor == TRUE && GIsGame == FALSE && !bIsInGame) //cascade mode
	{
		if(!CascadeScene)
		{
			AWorldInfo *pInfo = (AWorldInfo*)AWorldInfo::StaticClass()->GetDefaultObject();
			check(pInfo);
			FVector Gravity(0, 0, pInfo->DefaultGravityZ * pInfo->RBPhysicsGravityScaling);
			CascadeScene = CreateRBPhysScene(Gravity);
			check(CascadeScene);

			NxPlaneShapeDesc PlaneShape;
			NxActorDesc Actor;
			PlaneShape.normal.set(0.0f, 0.0f, 1.0f);
			PlaneShape.d = -5.0f;
			FRBCollisionChannelContainer CollidesWith;
			CollidesWith.SetChannel(RBCC_Default, TRUE);
			PlaneShape.groupsMask = CreateGroupsMask(RBCC_Default, &CollidesWith);
			Actor.shapes.pushBack(&PlaneShape);
			NxScene* SceneNx = CascadeScene->GetNovodexPrimaryScene();
			check(SceneNx);
			SceneNx->createActor(Actor);
		}
		Scene = CascadeScene;
	
	}
	else if(GIsGame)
	{
		check(GWorld && GWorld->RBPhysScene);
		Scene = GWorld->RBPhysScene;

		//Mark scene for PIE over Cascade precedence
		bIsInGame = TRUE;
	}	
#endif	//#if WITH_NOVODEX

	return Scene;
}

void UPhysXParticleSystem::SyncPhysXData()
{
#if WITH_NOVODEX
	PSys->SyncPhysXData();
#endif	//#if WITH_NOVODEX
}

void UPhysXParticleSystem::Tick(float DeltaTime)
{
#if WITH_NOVODEX
	check(PSys);
	PSys->Tick(DeltaTime);
#endif	//#if WITH_NOVODEX
}

void UPhysXParticleSystem::PostEditChange(UProperty* PropertyThatChanged)
{
	if (PropertyThatChanged)
	{
		//Collision
		if (PropertyThatChanged->GetFName() == FName(TEXT("MaxParticles")))
		{
			MaxParticles = Clamp(MaxParticles, 1, 32767);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("CollisionDistance")))
		{	
			//Dependency!
			CollisionDistance = Clamp(CollisionDistance, (FLOAT)KINDA_SMALL_NUMBER, BIG_NUMBER);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("RestitutionWithStaticShapes")))
		{
			RestitutionWithStaticShapes = Clamp(RestitutionWithStaticShapes, 0.0f, 1.0f);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("RestitutionWithDynamicShapes")))
		{
			RestitutionWithDynamicShapes = Clamp(RestitutionWithDynamicShapes, 0.0f, 1.0f);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("FrictionWithStaticShapes")))
		{
			FrictionWithStaticShapes = Clamp(FrictionWithStaticShapes, 0.0f, 1.0f);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("FrictionWithDynamicShapes")))
		{
			FrictionWithDynamicShapes = Clamp(FrictionWithDynamicShapes, 0.0f, 1.0f);
		}

		//Dynamics
		else if (PropertyThatChanged->GetFName() == FName(TEXT("MaxMotionDistance")))
		{
			MaxMotionDistance = Clamp(MaxMotionDistance, 0.0f, BIG_NUMBER);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("Damping")))
		{
			Damping = Clamp(Damping, 0.0f, BIG_NUMBER);
		}
		//Advanced. With dependencies
		else if (PropertyThatChanged->GetFName() == FName(TEXT("PacketSizeMultiplier")))
		{
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("RestParticleDistance")))
		{
			RestParticleDistance = Clamp(RestParticleDistance, (FLOAT)KINDA_SMALL_NUMBER, BIG_NUMBER);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("KernelRadiusMultiplier")))
		{
			KernelRadiusMultiplier = Clamp(KernelRadiusMultiplier, 0.5f, BIG_NUMBER);
		}

		//Other Advanced
		else if (PropertyThatChanged->GetFName() == FName(TEXT("RestDensity")))
		{	
			RestDensity = Clamp(RestDensity, (FLOAT)KINDA_SMALL_NUMBER, BIG_NUMBER);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("Stiffness")))
		{	
			Stiffness = Clamp(Stiffness, (FLOAT)KINDA_SMALL_NUMBER, BIG_NUMBER);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("Viscosity")))
		{	
			Viscosity = Clamp(Viscosity, (FLOAT)KINDA_SMALL_NUMBER, BIG_NUMBER);
		}
		else if (PropertyThatChanged->GetFName() == FName(TEXT("CollisionResponseCoefficient")))
		{	
			CollisionResponseCoefficient = Clamp(CollisionResponseCoefficient, 0.0f, BIG_NUMBER);
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

void UPhysXParticleSystem::PreEditChange(UProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	FlushRenderingCommands();
	RemovedFromScene();
#if WITH_NOVODEX
	if(PSys && PSys->PhysScene)
	{
		PSys->PhysScene->PhysXEmitterManager->RemoveParticleSystem(this);
	}
#endif	//#if WITH_NOVODEX
}

void UPhysXParticleSystem::RemovedFromScene()
{
#if WITH_NOVODEX
	if(!CascadeScene)
	{
		bIsInGame = FALSE;
	}

	bSyncFailed = FALSE;
	
	if(!PSys)
		return;

	bDestroy = TRUE;

	PSys->RemovedFromScene();
	
	if(CascadeScene)
	{
		SyncDisconnect();
		check(!CascadeScene);
	}
#endif	//#if WITH_NOVODEX
}

void UPhysXParticleSystem::RemoveSpawnInstance(struct FParticleEmitterInstance* SpawnInstance)
{
#if WITH_NOVODEX
	if(!PSys)
		return;

	check(SpawnInstance);

	PSys->RemoveSpawnInstance(SpawnInstance);
	if(PSys->GetSpawnInstanceRefsNum() == 0)
		RemovedFromScene();
#endif	//#if WITH_NOVODEX
}

////////////////////////////////////////////////////////////////////////////////////
#if WITH_NOVODEX

FPhysXParticleSystem::FPhysXParticleSystem(UPhysXParticleSystem& InParams):
	Fluid(NULL),
	ParticlesSdk(NULL),
	ParticlesEx(NULL),
	ParticleForceUpdates(NULL),
	ParticleFlagUpdates(NULL),
	ParticleContactsSdk(NULL),
	ParticleCreationIDsSdk(NULL),
	ParticleDeletionIDsSdk(NULL),
	PacketsSdk(NULL),
	Params(InParams),
	Packets(NULL),
	Distributer(NULL)
{
	FRBPhysScene* InRBPhysScene = Params.GetScene();
	check(InRBPhysScene);

	PhysScene = InRBPhysScene;


	NxScene* Scene = PhysScene->GetNovodexPrimaryScene();
	NxCompartment* Compartment = PhysScene->GetNovodexFluidCompartment(); 
	
	if(!Scene || !Scene->isWritable() || !Compartment)
	{
		return;
	}

	ParticlesSdk = (PhysXParticle*)appMalloc(Params.MaxParticles * sizeof(PhysXParticle));
	check(ParticlesSdk);
	ParticlesEx = (PhysXParticleEx*)appMalloc(Params.MaxParticles * sizeof(PhysXParticleEx));
	check(ParticlesEx);
	ParticleContactsSdk = (NxVec3*)appMalloc(Params.MaxParticles * sizeof(NxVec3));
	check(ParticleContactsSdk);
	ParticleCreationIDsSdk = (UINT*)appMalloc(Params.MaxParticles * sizeof(UINT));
	check(ParticleCreationIDsSdk);
	ParticleDeletionIDsSdk = (UINT*)appMalloc(Params.MaxParticles * sizeof(UINT));
	check(ParticleDeletionIDsSdk);

	UINT MaxPackets = appFloor(GNovodexSDK->getParameter(NX_CONSTANT_FLUID_MAX_PACKETS));
	PacketsSdk = (NxFluidPacket*)appMalloc(MaxPackets * sizeof(NxFluidPacket));
	check(PacketsSdk);
	Packets = (Packet*)appMalloc(MaxPackets * sizeof(Packet));
	check(Packets);

	ParticleForceUpdates = (PhysXParticleForceUpdate*)appMalloc(Params.MaxParticles * sizeof(PhysXParticleForceUpdate));
	check(ParticleForceUpdates);
	ParticleFlagUpdates = (PhysXParticleFlagUpdate*)appMalloc(Params.MaxParticles * sizeof(PhysXParticleFlagUpdate));
	check(ParticleFlagUpdates);

	InitState();

	NxFluidDesc FluidDesc = CreateFluidDesc(Compartment);
	if(!FluidDesc.isValid())
		return;

	Fluid = Scene->createFluid(FluidDesc);
	check(Fluid);

	Distributer = new FPhysXDistribution();
}

void FPhysXParticleSystem::InitState()
{
	NumParticlesSdk = 0;
	NumParticlesCreatedSdk = 0;
	//This is used to mark, that fetch results wasn't executed.
	NumParticlesDeletedSdk = PHYSX_NUM_DEL_NOT_WRITTEN;
	NumPacketsSdk = 0;
	NumPackets = 0;

	NumParticleForceUpdates = 0;
	NumParticleFlagUpdates = 0;

	VerticalPacketLimit = appFloor(GNovodexSDK->getParameter(NX_CONSTANT_FLUID_MAX_PACKETS));
	VerticalPacketRadiusSq = FLT_MAX;

	DebugNumSDKDel = 0;

	bSdkDataSynced = FALSE;
	bProcessSimulationStep = FALSE;
	bLocationSortDone = FALSE;
	bCylindricalPacketCulling = GWorld->GetWorldInfo()->VerticalProperties.Emitters.bApplyCylindricalPacketCulling;
}

UBOOL FPhysXParticleSystem::IsSyncedToSdk()
{
	return Fluid != NULL;
}

FPhysXParticleSystem::~FPhysXParticleSystem()
{
	check(ParticleSetRefs.Num() == 0);
	check(SpawnInstanceRefs.Num() == 0);
	
	if(Fluid)
	{
		NxScene *Scene = &Fluid->getScene();
		check(Scene);
		Scene->releaseFluid(*Fluid);
		Fluid = NULL;
	}

	if(ParticlesSdk)
	{
		appFree(ParticlesSdk);
		ParticlesSdk = NULL;
	}

	if(ParticlesEx)
	{
		appFree(ParticlesEx);
		ParticlesEx = NULL;
	}

	if(ParticleContactsSdk)
	{
		appFree(ParticleContactsSdk);
		ParticleContactsSdk = NULL;
	}

	if(ParticleCreationIDsSdk)
	{
		appFree(ParticleCreationIDsSdk);
		ParticleCreationIDsSdk = NULL;
	}

	if(ParticleDeletionIDsSdk)
	{
		appFree(ParticleDeletionIDsSdk);
		ParticleDeletionIDsSdk = NULL;
	}

	if(PacketsSdk)
	{
		appFree(PacketsSdk);
		PacketsSdk = NULL;
	}

	if(Packets)
	{
		appFree(Packets);
		Packets = NULL;
	}

	if(ParticleForceUpdates)
	{
		appFree(ParticleForceUpdates);
		ParticleForceUpdates = NULL;
	}

	if(Distributer)
	{
		delete Distributer;
	}
}

void FPhysXParticleSystem::RemovedFromScene()
{
	RemoveAllParticleSets();
	RemoveAllSpawnInstances();

	//Init Sdk state variables.
	if (Fluid)
	{
		Fluid->removeAllParticles();
	}
	InitState();
}

void FPhysXParticleSystem::SyncParticleForceUpdates()
{
	//Remove unused and obsolete updates	
	for(INT i=NumParticleForceUpdates-1; i>=0;i--)
	{
		PhysXParticleForceUpdate& ParticleForceUpdate = ParticleForceUpdates[i];

		UBOOL Unused = (ParticleForceUpdate.Vec.isZero());
		if(Unused || ParticlesEx[ParticleForceUpdate.Id].Index == 0xffff)
		{
			ParticleForceUpdate = ParticleForceUpdates[NumParticleForceUpdates-1];
			NumParticleForceUpdates--;
		}
	}

	if (NumParticleForceUpdates > 0)
	{
		//debugf( TEXT("DEBUG_VE SyncParticleUpdates() NumDels %d"), NumDels);
		NxParticleUpdateData UpdateData;
		UpdateData.forceMode = NX_ACCELERATION;
		UpdateData.bufferId = &ParticleForceUpdates[0].Id;
		UpdateData.bufferForce = &ParticleForceUpdates[0].Vec.x;
		UpdateData.bufferIdByteStride = sizeof(PhysXParticleForceUpdate);
		UpdateData.bufferForceByteStride = sizeof(PhysXParticleForceUpdate);
		UpdateData.numUpdates = NumParticleForceUpdates;
		Fluid->updateParticles(UpdateData);
		NumParticleForceUpdates = 0;
	}
}

void FPhysXParticleSystem::SyncParticleFlagUpdates()
{
	if (NumParticleFlagUpdates == 0)
		return;

#ifdef _DEBUG
	for(INT i=NumParticleFlagUpdates-1; i>=0;i--)
	{
		PhysXParticleFlagUpdate& ParticleUpdate = ParticleFlagUpdates[i];
		check(ParticleUpdate.Flags > 0);
		check(ParticlesEx[ParticleUpdate.Id].PSet == NULL);
		check(ParticlesEx[ParticleUpdate.Id].Index != 0xffff);

//		if(Removed)
//		{
//			ParticleUpdate = ParticleFlagUpdates[NumParticleFlagUpdates-1];
//			NumParticleFlagUpdates--;
//		}
//		else
//		{
//#if PHYSX_PARTICLE_SYSTEM_DEBUG_IDS
//			INT Blabla;
//			check(!DebugDelSdkIds.FindItem(ParticleUpdate.Id,Blabla));	
//			DebugDelSdkIds.AddItem(ParticleUpdate.Id);
//#endif
//		}
	}
#endif

	//This assumes, the flag updates are actually set to particle deletions
	DebugNumSDKDel += NumParticleFlagUpdates;
	
	NxParticleUpdateData UpdateData;
	UpdateData.bufferId = &ParticleFlagUpdates[0].Id;
	UpdateData.bufferFlag = &ParticleFlagUpdates[0].Flags;
	UpdateData.bufferIdByteStride = sizeof(PhysXParticleFlagUpdate);
	UpdateData.bufferFlagByteStride = sizeof(PhysXParticleFlagUpdate);
	UpdateData.numUpdates = NumParticleFlagUpdates;
	Fluid->updateParticles(UpdateData);
	NumParticleFlagUpdates = 0;
}

/**
Checks whether there are pending SDK particle removals. If yes, the simulation 
didn't execute, usually because of too small timesteps.
*/
UBOOL FPhysXParticleSystem::SyncCheckSimulationStep()
{
	return !(DebugNumSDKDel > 0 && NumParticlesDeletedSdk == 0) && NumParticlesDeletedSdk != PHYSX_NUM_DEL_NOT_WRITTEN;
}

/**
update particle extensions, render instances and async particle updates, 
given the notification of deleted Ids from the SDK.
*/
void FPhysXParticleSystem::SyncProcessSDKParticleRemovals()
{
	check(bProcessSimulationStep);
	INT DebugNumSdkInternalDel = 0;

	for (INT i = 0; i < NumParticlesDeletedSdk; i++)
	{
		UINT Id = ParticleDeletionIDsSdk[i];
		PhysXParticleEx& ParticleEx = ParticlesEx[Id];
		FPhysXParticleSet* PSet = ParticleEx.PSet;

#if PHYSX_PARTICLE_SYSTEM_DEBUG_IDS
		INT Num = DebugDelSdkIds.Num();
		DebugDelSdkIds.RemoveItem(Id);
		check(Num == DebugDelSdkIds.Num() + 1);	
#endif

		//SDK side deletions!
		if (PSet != NULL)
		{
			INT FixId;
			INT RenderIndex = ParticleEx.RenderIndex;
			FixId = PSet->RemoveParticle(RenderIndex, false);
			ParticlesEx[FixId].RenderIndex = RenderIndex;
			check(ParticlesEx[FixId].PSet == PSet);
			ParticleEx.PSet = NULL;
			ParticleEx.RenderIndex = 0xffff;

			PhysXParticleForceUpdate& ParticleForceUpdate = ParticleForceUpdates[ParticleEx.Index];
			check(ParticleForceUpdate.Id == 0 || ParticleForceUpdate.Id == Id);
			appMemset(&ParticleForceUpdate, 0, sizeof(PhysXParticleForceUpdate));
			DebugNumSdkInternalDel++;
		}
		ParticleEx.Index = 0xffff;
	}

	DebugNumSDKDel += DebugNumSdkInternalDel;
	DebugNumSDKDel -= NumParticlesDeletedSdk;
	check(DebugNumSDKDel == 0);

#if PHYSX_PARTICLE_SYSTEM_DEBUG_IDS
	check(DebugDelSdkIds.Num() == 0);
#endif

	INT NumEmitterParticles = GetNumParticles();
	check(NumEmitterParticles + NumParticleFlagUpdates == NumParticlesSdk);
	
	NumParticlesDeletedSdk = 0;
}

void FPhysXParticleSystem::SyncSdkData()
{
	if(bSdkDataSynced)
		return;

	bSdkDataSynced = TRUE;
	if(SyncCheckSimulationStep())
	{
		bProcessSimulationStep = TRUE;
		SyncProcessSDKParticleRemovals();
		SyncProcessParticles();
	}
	else
	{
		bProcessSimulationStep = FALSE;
	}
	NumParticlesDeletedSdk = PHYSX_NUM_DEL_NOT_WRITTEN;
}

//update extension table, clean up async updates 
//update per packet mean particle age
void FPhysXParticleSystem::SyncProcessParticles()
{
	if(!bProcessSimulationStep)
	{
		return;
	}

	FVector LODOrigin;
	FPlane LODPlane;
	UBOOL bApplyPacketDistanceLOD = GetLODOrigin(LODOrigin);
	UBOOL bApplyPacketClippingLOD = GetLODNearClippingPlane(LODPlane);
	UBOOL bApplyLOD = bApplyPacketDistanceLOD || bApplyPacketClippingLOD;

	NumPackets = NumPacketsSdk;
	for(INT i = 0; i < NumPacketsSdk; i++)
	{
		Packet& P = Packets[i];
		NxFluidPacket& SdkPacket = PacketsSdk[i];
		P.SdkPacket = &SdkPacket;
		
		//Particle loop
		P.MeanParticleAge = 0.0f;
		for( UINT j = 0; j < SdkPacket.numParticles; ++j )
		{
			UINT ParticleIndex = SdkPacket.firstParticleIndex + j;
			PhysXParticle& Particle = ParticlesSdk[ParticleIndex];
			INT Id = Particle.Id;
			PhysXParticleEx& ParticleEx = ParticlesEx[Id];

			FPhysXParticleSet* PSet = ParticleEx.PSet;
			if(PSet != NULL)
			{
				INT RenderIndex = ParticleEx.RenderIndex;
				PhysXRenderParticle* RenderParticle = PSet->GetRenderParticle(RenderIndex);
				RenderParticle->ParticleIndex = ParticleIndex;			
				P.MeanParticleAge += RenderParticle->RelativeTime;				
			}

			ParticleEx.Index = ParticleIndex;
		}
		P.MeanParticleAge /= SdkPacket.numParticles;
		if(bApplyLOD)
		{
			NxVec3 nxCenter;
			SdkPacket.aabb.getCenter(nxCenter);
			FVector Center = N2UPosition(nxCenter);
			P.DistanceSq = bApplyPacketDistanceLOD ? GetLODDistanceSq(LODOrigin, Center) : P.DistanceSq = 0.0f;
			P.IsInFront	= bApplyPacketClippingLOD ? LODPlane.PlaneDot(Center) < 0.0f : TRUE;
		}
		else
		{
			P.DistanceSq = 0.0f;
			P.IsInFront = TRUE;
		}
	}
}

/**
Executes SDK particle spawning
*/
void FPhysXParticleSystem::SyncPhysXData()
{
	SyncSdkData();
	bSdkDataSynced = FALSE;

	if(bProcessSimulationStep)
	{
		SyncParticleFlagUpdates();
		SyncParticleForceUpdates();
	}

	for(INT i=0; i<ParticleSetRefs.Num(); i++)
		ParticleSetRefs(i)->SyncPhysXData();

	for(INT i=0; i<SpawnInstanceRefs.Num(); i++)
	{
		FParticleEmitterInstance* SpawnEmitter = SpawnInstanceRefs(i);
		if(SpawnEmitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
		{
			FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)SpawnEmitter;
			SpawnSpriteEmitter->SpawnSyncPhysX();
		}
		else if(SpawnEmitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
		{
			FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)SpawnEmitter;
			SpawnMeshEmitter->SpawnSyncPhysX();
		}
	}

	if(bProcessSimulationStep)
	{
		NumParticleForceUpdates = NumParticlesSdk;
		appMemset(ParticleForceUpdates, 0, NumParticleForceUpdates * sizeof(PhysXParticleForceUpdate));

		check(GetNumParticles() + DebugNumSDKDel + NumParticleFlagUpdates == NumParticlesSdk);
		bLocationSortDone = FALSE;
	}
}

void FPhysXParticleSystem::AssignReduction(INT InNumReduction)
{
	check(InNumReduction > 0);

	TArray<FPhysXDistribution::Input>& InputBuffer = Distributer->GetInputBuffer(ParticleSetRefs.Num());
	for(INT i=0; i<ParticleSetRefs.Num(); i++)
	{
		FPhysXParticleSet& PSet = *ParticleSetRefs(i);
		InputBuffer.AddItem(FPhysXDistribution::Input(PSet.GetNumRenderParticles(), PSet.GetWeightForFifo()));
	}
	
	const TArray<FPhysXDistribution::Result>& ResultBuffer = Distributer->GetResult(InNumReduction);

	for(INT i=0; i<ResultBuffer.Num(); i++)
	{
		FPhysXParticleSet& PSet = *ParticleSetRefs(i);
		const FPhysXDistribution::Result& R = ResultBuffer(i);
		PSet.SyncParticleReduction(R.Piece);
	}
}

void FPhysXParticleSystem::SyncParticleReduction(INT InNumReduction)
{
	if(!bProcessSimulationStep)
	{
		return;
	}

	if(InNumReduction > 0)
		SyncRemoveHidden(InNumReduction);

	if(InNumReduction > 0)
	{
		AssignReduction(InNumReduction);
		SyncParticleFlagUpdates();
	}
	//plotf( TEXT("DEBUG_VE_PLOT numParticles %d"), NumParticles);
}

void FPhysXParticleSystem::SyncRemoveHidden(INT& InNumReduction)
{
	check(bProcessSimulationStep);
	appQsort( Packets, NumPackets, sizeof(Packet), (QSORT_COMPARE)ComparePacketLocation );
	bLocationSortDone = TRUE;

	INT i=0; 
	while(i < NumPackets && InNumReduction > 0)
	{
		Packet& P = Packets[i];
		NxFluidPacket& SdkPacket = *P.SdkPacket;
		
		if(!P.IsInFront && SdkPacket.packetID != 0xffff)
		{
			for( UINT j = 0; j < SdkPacket.numParticles && InNumReduction > 0; ++j )
			{
				UINT ParticleIndex = SdkPacket.firstParticleIndex + j;
				PhysXParticle& Particle = ParticlesSdk[ParticleIndex];
				PhysXParticleEx& ParticleEx = ParticlesEx[Particle.Id];

				if(ParticleEx.PSet == NULL)
					continue;

				ParticleEx.PSet->SyncRemoveParticle(Particle.Id);
				
				InNumReduction--;
			}	
		}
		i++;
	}
}

void FPhysXParticleSystem::Tick(FLOAT DeltaTime)
{
	for(INT i=0; i<ParticleSetRefs.Num(); i++)
	{
		ParticleSetRefs(i)->AsyncUpdate(DeltaTime, bProcessSimulationStep);
	}
	
	if(bProcessSimulationStep)
	{
		AsyncPacketCulling();
	}
}

void FPhysXParticleSystem::RemoveAllParticleSets()
{
	while(ParticleSetRefs.Num())
	{
		RemoveParticleSet(ParticleSetRefs.Last());
	}
}

void FPhysXParticleSystem::AddParticleSet(FPhysXParticleSet* InParticleSet)
{
	if(Params.bDestroy)
		return;

	ParticleSetRefs.AddUniqueItem(InParticleSet);
}

void FPhysXParticleSystem::RemoveParticleSet(FPhysXParticleSet* InParticleSet)
{
	InParticleSet->RemoveAllParticles();
	
	INT OldNum = ParticleSetRefs.Num();
	ParticleSetRefs.RemoveItem(InParticleSet);
	
	if(ParticleSetRefs.Num() == OldNum)
	{
		return;
	}

	if(!ParticlesEx)
		return;

	for (INT i = 0; i < Params.MaxParticles; i++)
	{
		if (ParticlesEx[i].PSet == InParticleSet)
			ParticlesEx[i].PSet = NULL;
	}
}

void FPhysXParticleSystem::RemoveAllSpawnInstances()
{
	while(SpawnInstanceRefs.Num())
	{
		RemoveSpawnInstance(SpawnInstanceRefs.Last());
	}
}

void FPhysXParticleSystem::AddSpawnInstance(FParticleEmitterInstance* InSpawnInstance)
{
	if(Params.bDestroy)
		return;
	//INT OldNum = SpawnInstanceRefs.Num();
	SpawnInstanceRefs.AddUniqueItem(InSpawnInstance);
	
	//if(SpawnInstanceRefs.Num() > OldNum)
}

void FPhysXParticleSystem::RemoveSpawnInstance(FParticleEmitterInstance* InSpawnInstance)
{
	INT NumRemoved = SpawnInstanceRefs.RemoveItem(InSpawnInstance);

	if(InSpawnInstance->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
	{
		FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)InSpawnInstance;
		SpawnSpriteEmitter->RemoveParticles();
	}
	else if(InSpawnInstance->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
	{
		FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)InSpawnInstance;
		SpawnMeshEmitter->RemoveParticles();
	}
}

INT FPhysXParticleSystem::AddParticles(NxParticleData& InParticleBuffer, FPhysXParticleSet* InParticleSet)
{
	AddParticleSet(InParticleSet);
	UINT RIOffset =  InParticleSet->GetNumRenderParticles();

	UINT NumOld = NumParticlesSdk;
	Fluid->addParticles(InParticleBuffer);
	check(NumOld + NumParticlesCreatedSdk == NumParticlesSdk);
	for (INT i = 0; i < NumParticlesCreatedSdk; i++)
	{
		UINT Id = ParticleCreationIDsSdk[i];
		PhysXParticleEx& ParticleEx = ParticlesEx[Id];
		ParticleEx.PSet = InParticleSet;
		ParticleEx.RenderIndex = RIOffset + i;
		ParticleEx.Index = NumOld + i;
	}
	return NumParticlesCreatedSdk;
}

void FPhysXParticleSystem::GetGravity(NxVec3& gravity)
{
	check(Fluid);
	Fluid->getScene().getGravity(gravity);
}

NxFluidDesc FPhysXParticleSystem::CreateFluidDesc(NxCompartment* Compartment)
{
	NxFluidDesc fluidDesc;
	fluidDesc.setToDefault();
	fluidDesc.maxParticles = Params.MaxParticles;
	fluidDesc.numReserveParticles = 0;
	fluidDesc.restParticlesPerMeter = 1.0f/(Params.RestParticleDistance*U2PScale);
	fluidDesc.restDensity = Params.RestDensity;
	fluidDesc.kernelRadiusMultiplier = Params.KernelRadiusMultiplier;
	fluidDesc.motionLimitMultiplier = Params.MaxMotionDistance/Params.RestParticleDistance;
	fluidDesc.collisionDistanceMultiplier = Params.CollisionDistance/Params.RestParticleDistance;
	fluidDesc.stiffness = Params.Stiffness;
	fluidDesc.viscosity = Params.Viscosity;
	fluidDesc.damping = Params.Damping;
	fluidDesc.restitutionForStaticShapes  = Params.RestitutionWithStaticShapes;
	fluidDesc.dynamicFrictionForStaticShapes = Params.FrictionWithStaticShapes;
	fluidDesc.restitutionForDynamicShapes = Params.RestitutionWithDynamicShapes;
	fluidDesc.dynamicFrictionForDynamicShapes = Params.FrictionWithDynamicShapes;
	fluidDesc.collisionResponseCoefficient = Params.CollisionResponseCoefficient;
	fluidDesc.externalAcceleration = U2NPosition(Params.ExternalAcceleration);
	switch (Params.PacketSizeMultiplier)
	{
	case EPSM_4:
			fluidDesc.packetSizeMultiplier = 4;
		break;
	case EPSM_8:
			fluidDesc.packetSizeMultiplier = 8;
		break;
	case EPSM_16:
			fluidDesc.packetSizeMultiplier = 16;
		break;
	case EPSM_32:
			fluidDesc.packetSizeMultiplier = 32;
		break;
	case EPSM_64:
			fluidDesc.packetSizeMultiplier = 64;
		break;
	case EPSM_128:
			fluidDesc.packetSizeMultiplier = 128;
		break;
	}
	switch (Params.SimulationMethod)
	{
	case ESM_SPH:
		fluidDesc.simulationMethod = NX_F_SPH;
		break;
	case ESM_NO_PARTICLE_INTERACTION:
		fluidDesc.simulationMethod = NX_F_NO_PARTICLE_INTERACTION;
		break;
	case ESM_MIXED_MODE:
		fluidDesc.simulationMethod = NX_F_MIXED_MODE;
		break;
	}
	if (Params.bStaticCollision)
		fluidDesc.collisionMethod |= NX_F_STATIC;
	else
		fluidDesc.collisionMethod &= ~NX_F_STATIC;
	if (Params.bDynamicCollision)
		fluidDesc.collisionMethod |= NX_F_DYNAMIC;
	else
		fluidDesc.collisionMethod &= ~NX_F_DYNAMIC;
	fluidDesc.compartment = Compartment;
	fluidDesc.flags = NX_FF_VISUALIZATION | NX_FF_ENABLED;
	if (Params.bTwoWayCollision)
		fluidDesc.flags |= NX_FF_COLLISION_TWOWAY;
	else
		fluidDesc.flags &= ~NX_FF_COLLISION_TWOWAY;
	if (fluidDesc.numReserveParticles > 0)
		fluidDesc.flags |= NX_FF_PRIORITY_MODE;
	else
		fluidDesc.flags &= ~NX_FF_PRIORITY_MODE;
	if(IsPhysXHardwarePresent() && Compartment->getDeviceCode() != NX_DC_CPU)
		fluidDesc.flags |= NX_FF_HARDWARE;
	else
		fluidDesc.flags &= ~NX_FF_HARDWARE;
	if (Params.bDisableGravity)
		fluidDesc.flags |= NX_FF_DISABLE_GRAVITY;
	else
		fluidDesc.flags &= ~NX_FF_DISABLE_GRAVITY;

	FRBCollisionChannelContainer collidesWith;
	collidesWith.SetChannel(RBCC_Default, TRUE);
	collidesWith.SetChannel(RBCC_GameplayPhysics, TRUE);
	collidesWith.SetChannel(RBCC_FluidDrain, TRUE);
	fluidDesc.groupsMask = CreateGroupsMask(RBCC_EffectPhysics, &collidesWith);
	fluidDesc.collisionGroup = UNX_GROUP_DEFAULT;

	NxParticleData ParticlesWriteData;
	ParticlesWriteData.numParticlesPtr = (NxU32*)&NumParticlesSdk;
	ParticlesWriteData.bufferPos = &ParticlesSdk[0].Pos.x;
	ParticlesWriteData.bufferVel = &ParticlesSdk[0].Vel.x;
	ParticlesWriteData.bufferId = (NxU32*)&ParticlesSdk[0].Id;
	ParticlesWriteData.bufferCollisionNormal = (NxF32*)&ParticleContactsSdk[0].x;
	ParticlesWriteData.bufferPosByteStride = sizeof(PhysXParticle);
	ParticlesWriteData.bufferVelByteStride = sizeof(PhysXParticle);
	ParticlesWriteData.bufferIdByteStride = sizeof(PhysXParticle);
	ParticlesWriteData.bufferCollisionNormalByteStride = sizeof(NxVec3);	
	fluidDesc.particlesWriteData = ParticlesWriteData;
	
	fluidDesc.particleCreationIdWriteData.numIdsPtr = (NxU32*)&NumParticlesCreatedSdk;
	fluidDesc.particleCreationIdWriteData.bufferId = (NxU32*)ParticleCreationIDsSdk;
	fluidDesc.particleCreationIdWriteData.bufferIdByteStride = sizeof(UINT);
	
	fluidDesc.particleDeletionIdWriteData.numIdsPtr = (NxU32*)&NumParticlesDeletedSdk;
	fluidDesc.particleDeletionIdWriteData.bufferId = (NxU32*)ParticleDeletionIDsSdk;
	fluidDesc.particleDeletionIdWriteData.bufferIdByteStride = sizeof(UINT);

	fluidDesc.fluidPacketData.bufferFluidPackets = PacketsSdk;
	fluidDesc.fluidPacketData.numFluidPacketsPtr = (NxU32*)&NumPacketsSdk;

	return fluidDesc;
}

FLOAT FPhysXParticleSystem::GetWeightForFifo()
{
	FLOAT TotalLodWeight = 0.0f;
	for(INT i=0; i<ParticleSetRefs.Num(); i++)
	{
		FPhysXParticleSet& PSet = *ParticleSetRefs(i);
		TotalLodWeight += PSet.GetWeightForFifo();
	}
	return TotalLodWeight;
}

FLOAT FPhysXParticleSystem::GetWeightForSpawnLod()
{
	FLOAT TotalLodWeight = 0.0f;
	for(INT i=0; i<ParticleSetRefs.Num(); i++)
	{
		FPhysXParticleSet& PSet = *ParticleSetRefs(i);
		TotalLodWeight += PSet.GetWeightForSpawnLod();
	}
	return TotalLodWeight;
}

INT FPhysXParticleSystem::GetSpawnVolumeEstimate()
{
	INT SpawnVolumeSum = 0;
	for(INT i=0; i<SpawnInstanceRefs.Num(); i++)
	{
		FParticleEmitterInstance* Emitter = SpawnInstanceRefs(i);
		
		if(Emitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
		{
			FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)Emitter;
			SpawnVolumeSum += SpawnSpriteEmitter->GetSpawnVolumeEstimate();
		}
		else if(Emitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
		{
			FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)Emitter;
			SpawnVolumeSum += SpawnMeshEmitter->GetSpawnVolumeEstimate();
		}
	}
	return SpawnVolumeSum;
}

void FPhysXParticleSystem::SetEmissionBudget(INT EmissionBudget)
{
	if(EmissionBudget == INT_MAX)
	{
		for(INT i=0; i<SpawnInstanceRefs.Num(); i++)
		{
			FParticleEmitterInstance* Emitter = SpawnInstanceRefs(i);

			if(Emitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
			{
				FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)Emitter;
				SpawnSpriteEmitter->SetEmissionBudget(INT_MAX);
			}
			else if(Emitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
			{
				FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)Emitter;
				SpawnMeshEmitter->SetEmissionBudget(INT_MAX);
			}
		}
		return;
	}

	TArray<FPhysXDistribution::Input>& InputBuffer = Distributer->GetInputBuffer(SpawnInstanceRefs.Num());
	for(INT i=0; i<SpawnInstanceRefs.Num(); i++)
	{
		FParticleEmitterInstance* Emitter = SpawnInstanceRefs(i);
		INT SpawnVolumeEstimate = 0;
		FLOAT WeightForSpawnLod = 0.0f;

		if(Emitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
		{
			FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)Emitter;
			SpawnVolumeEstimate = SpawnSpriteEmitter->GetSpawnVolumeEstimate();
			WeightForSpawnLod = SpawnSpriteEmitter->GetWeightForSpawnLod();
		}
		else if(Emitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
		{
			FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)Emitter;
			SpawnVolumeEstimate = SpawnMeshEmitter->GetSpawnVolumeEstimate();
			WeightForSpawnLod = SpawnMeshEmitter->GetWeightForSpawnLod();
		}
		
		InputBuffer.AddItem(FPhysXDistribution::Input(SpawnVolumeEstimate, WeightForSpawnLod));
	}
	
	const TArray<FPhysXDistribution::Result>& ResultBuffer = Distributer->GetResult(EmissionBudget);

	for(INT i=0; i<ResultBuffer.Num(); i++)
	{
		const FPhysXDistribution::Result& R = ResultBuffer(i);		
		
		FParticleEmitterInstance* Emitter = SpawnInstanceRefs(i);

		if(Emitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
		{
			FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)Emitter;
			SpawnSpriteEmitter->SetEmissionBudget(R.Piece);
		}
		else if(Emitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
		{
			FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)Emitter;
			SpawnMeshEmitter->SetEmissionBudget(R.Piece);
		}
	}
}

INT FPhysXParticleSystem::GetNumParticles()
{
	//return NumParticles;
	INT NumEmitterParticles = 0;
	for(INT i=0; i<ParticleSetRefs.Num(); i++)
	{
		FPhysXParticleSet& PSet = *ParticleSetRefs(i);
		NumEmitterParticles += PSet.GetNumRenderParticles();
	}

	return NumEmitterParticles;
}

FBox FPhysXParticleSystem::GetWorldBounds()
{
	FBox Bounds;
	Bounds.Init();
	if(!Fluid)
		return Bounds;

	NxBounds3 NxBounds;
	Fluid->getWorldBounds(NxBounds);
	if(NxBounds.isEmpty())
		return Bounds;

	Bounds.Min =  N2UPosition(NxBounds.min);
	Bounds.Max =  N2UPosition(NxBounds.max);
	//Bounds = Bounds.ExpandBy(1.0f);
	Bounds.IsValid = TRUE;
	return Bounds;
}


QSORT_RETURN
FPhysXParticleSystem::ComparePacketSizes( const Packet* A, const Packet* B )
{ 
	return A->SdkPacket->numParticles > B->SdkPacket->numParticles ? 1 : A->SdkPacket->numParticles < B->SdkPacket->numParticles? -1 : 0; 
}

QSORT_RETURN
FPhysXParticleSystem::ComparePacketAges( const Packet* A, const Packet* B )
{ 
	return B->MeanParticleAge > A->MeanParticleAge ? 1 : B->MeanParticleAge < A->MeanParticleAge? -1 : 0; 
}

QSORT_RETURN
FPhysXParticleSystem::ComparePacketLocation( const Packet* A, const Packet* B )
{
	if(B->IsInFront < A->IsInFront)
		return 1;

	return B->DistanceSq > A->DistanceSq ? 1 : B->DistanceSq < A->DistanceSq? -1 : 0; 
}

void FPhysXParticleSystem::AsyncPacketCulling()
{
	check(bProcessSimulationStep);

	if(VerticalPacketLimit >= NumPackets)
	{
		VerticalPacketRadiusSq *= 1.01f;
		return;
	}

	INT VerticalPacketReduction = (NumPackets > VerticalPacketLimit) ? NumPackets - VerticalPacketLimit : 0;

#if VERTICAL_LOD_USE_PACKET_AGE_CULLING
	//sort according to mean age
	appQsort( Packets, NumParticlePackets, sizeof(Packet), (QSORT_COMPARE)ComparePacketAges );	
#else
	//sort according to distance/clipping plane
	if (!bLocationSortDone)
	{
		appQsort( Packets, NumPackets, sizeof(Packet), (QSORT_COMPARE)ComparePacketLocation );
	}
#endif
	
	//sort selection according to packet size
	INT NumPacketSortForSize = Min(NumPackets/2, VerticalPacketReduction*2);
	appQsort( Packets, NumPacketSortForSize, sizeof(Packet), (QSORT_COMPARE)ComparePacketSizes );

	FLOAT MeanRadiusSq = 0.0f;

	//mark particles as deleted
	UINT RemovalCount = 0;
	for(INT i = 0; i < VerticalPacketReduction; ++i )
	{
		Packet& P = Packets[i];
		NxFluidPacket& SdkPacket = *P.SdkPacket;
		
		MeanRadiusSq += P.DistanceSq;

		for( UINT j = 0; j < SdkPacket.numParticles; ++j )
		{
			UINT ParticleIndex = SdkPacket.firstParticleIndex + j;
			PhysXParticle& Particle = ParticlesSdk[ParticleIndex];
			PhysXParticleEx& ParticleEx = ParticlesEx[Particle.Id];

			if(ParticleEx.PSet == NULL)
				continue;

			check(ParticleEx.Index != 0xffff);
			ParticleEx.PSet->RemoveParticle(ParticleEx.RenderIndex, true);		
			RemovalCount++;
		}
	}
	//plotf( TEXT("DEBUG_VE_PLOT numASyncRemovalsPacketCulling %d"), RemovalCount);
		
	MeanRadiusSq /= VerticalPacketReduction;

	//Some continuity...
	if(VerticalPacketRadiusSq > 2*MeanRadiusSq)
	{
		VerticalPacketRadiusSq = MeanRadiusSq;
	}
	else
	{
		VerticalPacketRadiusSq *= 0.99f;
		if(VerticalPacketRadiusSq < 1.0f)
		{
			VerticalPacketRadiusSq = 1.0f;
		}
	}
}

BOOL FPhysXParticleSystem::GetLODOrigin(FVector& OutLODOrigin)
{
	if(SpawnInstanceRefs.Num() == 0)
		return FALSE;

	FParticleEmitterInstance* Emitter = SpawnInstanceRefs(0);

	if(Emitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
	{
		FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)Emitter;
		return SpawnSpriteEmitter->GetLODOrigin(OutLODOrigin);
	}
	else if(Emitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
	{
		FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)Emitter;
		return SpawnMeshEmitter->GetLODOrigin(OutLODOrigin);
	}

	return FALSE;
}	

BOOL FPhysXParticleSystem::GetLODNearClippingPlane(FPlane& OutNearClippingPlane)
{
	if(SpawnInstanceRefs.Num() == 0)
		return FALSE;

	FParticleEmitterInstance* Emitter = SpawnInstanceRefs(0);

	if(Emitter->Type()->IsA(FParticleSpritePhysXEmitterInstance::StaticType))
	{
		FParticleSpritePhysXEmitterInstance* SpawnSpriteEmitter = (FParticleSpritePhysXEmitterInstance*)Emitter;
		return SpawnSpriteEmitter->GetLODNearClippingPlane(OutNearClippingPlane);
	}
	else if(Emitter->Type()->IsA(FParticleMeshPhysXEmitterInstance::StaticType))
	{
		FParticleMeshPhysXEmitterInstance* SpawnMeshEmitter = (FParticleMeshPhysXEmitterInstance*)Emitter;
		return SpawnMeshEmitter->GetLODNearClippingPlane(OutNearClippingPlane);
	}

	return FALSE;
}

#endif	//#if WITH_NOVODEX

///////////////////////////////////////////////////////////////////////////////////
#if WITH_NOVODEX

void FPhysXDistribution::ComputeResult(INT Cake)
{
	FLOAT ProductSum = 0.0f;
	for(INT i=0; i<InputBuffer.Num(); i++)
	{
		Input& In = InputBuffer(i);
		Temp Tmp;
		Tmp.Num = In.Num;
		Tmp.InputIndex = i;
		Tmp.Product = In.Num * In.Weight;
		Tmp.Piece = 0;
		TempBuffer.AddItem(Tmp);
		ProductSum += Tmp.Product;
	}
	if(ProductSum == 0.0f)
	{
		for(INT i=0; i<TempBuffer.Num(); i++)
		{
			Temp& Tmp = TempBuffer(i);
			Tmp.Product = Tmp.Num;
			ProductSum += Tmp.Product;
		}
	}

	//Bubblesort, In order to prevent round up errors
	if(TempBuffer.Num())
	{
		UBOOL Bubbled = TRUE;
		UINT N = TempBuffer.Num()-1;
		while(Bubbled)
		{
			Bubbled = FALSE;
			for(UINT i=0; i<N; i++)
			{
				Temp& Tmp0 = TempBuffer(i);
				Temp& Tmp1 = TempBuffer(i+1);

				if(Tmp0.Product < Tmp1.Product)
				{
					TempBuffer.SwapItems(i, i+1);
					Bubbled = TRUE;
				}
			}
			N = N-1;
		}
	}

	//Distribute Sum according to the product of Num and Weight.
	FLOAT NFactor = 1.0f / ProductSum;
	INT Leftover = Cake;
	INT NewParticleSum = 0;
	for(INT i=0; i<TempBuffer.Num(); i++)
	{
		Temp& Tmp = TempBuffer(i);

		INT Piece = (INT)NxMath::ceil(Cake*Tmp.Product*NFactor);
		Piece = Min<UINT>(Piece, Leftover);
		Piece = Min<UINT>(Piece, Tmp.Num);
		Leftover -= Piece;
		Tmp.Num -= Piece;
		Tmp.Piece = Piece;
		NewParticleSum += Tmp.Num;
	}

	if(Leftover > 0)
	{

		//Distribute rest proportional to particles left
		NFactor = 1.0f / NewParticleSum;
		Cake = Leftover;
		for(INT i=0; i<TempBuffer.Num(); i++)
		{
			Temp& Tmp = TempBuffer(i); 

			INT Piece = (INT)NxMath::ceil(Cake*Tmp.Num*NFactor);
			Piece = Min<UINT>(Piece, Leftover);
			Piece = Min<UINT>(Piece, Tmp.Num);
			Leftover -= Piece;
			Tmp.Num -= Piece;
			Tmp.Piece += Piece;
		}
	}
	check(Leftover == 0);

	ResultBuffer.Add(TempBuffer.Num());
	for(INT i=0; i<TempBuffer.Num(); i++)
	{
		Temp& Tmp = TempBuffer(i);
		Result& Result = ResultBuffer(Tmp.InputIndex);
		Result.Piece = Tmp.Piece;
	}
}

#endif	//#if WITH_NOVODEX
