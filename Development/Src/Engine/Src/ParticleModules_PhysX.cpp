/*=============================================================================
	ParticleModules_PhysX.cpp: PhysX Emitter Source.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EnginePhysicsClasses.h"
#include "UnNovodexSupport.h"
#include "PhysXParticleSystem.h"
#include "PhysXParticleSetMesh.h"

IMPLEMENT_CLASS(UParticleModuleTypeDataMeshPhysX);

#if WITH_NOVODEX
void UParticleModuleTypeDataMeshPhysX::TryCreateRenderInstance(UParticleEmitter *InEmitterParent, FParticleMeshPhysXEmitterInstance *InSpawnEmitterInstance)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	if(!RenderInstance)
	{
		RenderInstance = new FPhysXMeshInstance(*this);
		check(RenderInstance);
	}
	RenderInstance->SpawnInstances.AddUniqueItem(InSpawnEmitterInstance);
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void UParticleModuleTypeDataMeshPhysX::TryRemoveRenderInstance(FParticleMeshPhysXEmitterInstance *InSpawnEmitterInstance)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	if(RenderInstance)
	{
		RenderInstance->SpawnInstances.RemoveItem(InSpawnEmitterInstance);
		
		if(RenderInstance->SpawnInstances.Num() == 0)
		{
			delete RenderInstance;
			RenderInstance = NULL;
		}
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}
#endif	//#if WITH_NOVODEX

FParticleEmitterInstance *UParticleModuleTypeDataMeshPhysX::CreateInstance(UParticleEmitter *InEmitterParent, UParticleSystemComponent *InComponent)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	return NULL;
#else//#if PARTICLES_USE_DOUBLE_BUFFERING
#if WITH_NOVODEX
	SetToSensibleDefaults();

	FParticleEmitterInstance* Instance = new FParticleMeshPhysXEmitterInstance(*this);
	check(Instance);
	Instance->InitParameters(InEmitterParent, InComponent);

	TryCreateRenderInstance(InEmitterParent, (FParticleMeshPhysXEmitterInstance*)Instance);

	return Instance;
#else	//#if WITH_NOVODEX
	return NULL;
#endif	//#if WITH_NOVODEX
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void UParticleModuleTypeDataMeshPhysX::SetToSensibleDefaults()
{
	Super::SetToSensibleDefaults();
	if(PhysXParSys == NULL) // If fluid not set, set the default one
		PhysXParSys = (UPhysXParticleSystem*)UObject::StaticLoadObject(UPhysXParticleSystem::StaticClass(), NULL, TEXT("EngineResources.DefaultPhysXParSys"), NULL, LOAD_None, NULL);
}

void UParticleModuleTypeDataMeshPhysX::PreEditChange(UProperty* PropertyAboutToChange)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	Super::PreEditChange(PropertyAboutToChange);
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
#if WITH_NOVODEX
	if(RenderInstance)
	{
		if(RenderInstance->PSet)
			RenderInstance->PSet->RemoveAllParticles();

		while(RenderInstance && RenderInstance->SpawnInstances.Num())
			TryRemoveRenderInstance(RenderInstance->SpawnInstances(RenderInstance->SpawnInstances.Num()-1));
	}

	if(PhysXParSys)
		PhysXParSys->RemovedFromScene();
#endif	//#if WITH_NOVODEX

	Super::PreEditChange(PropertyAboutToChange);
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}


void UParticleModuleTypeDataMeshPhysX::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

#if WITH_NOVODEX
	if(PhysXParSys)
		PhysXParSys->RemovedFromScene();
#endif	//#if WITH_NOVODEX
}

void UParticleModuleTypeDataMeshPhysX::FinishDestroy()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else//#if PARTICLES_USE_DOUBLE_BUFFERING
#if WITH_NOVODEX
	if(RenderInstance)
	{
		check(RenderInstance->SpawnInstances.Num() == 0);
		delete RenderInstance;
		RenderInstance = NULL;
	}
#endif	//#if WITH_NOVODEX
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	Super::FinishDestroy();
}

/////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(UParticleModuleTypeDataPhysX);

FParticleEmitterInstance *UParticleModuleTypeDataPhysX::CreateInstance(UParticleEmitter *InEmitterParent, UParticleSystemComponent *InComponent)
{
#if WITH_NOVODEX
	SetToSensibleDefaults();

	FParticleEmitterInstance* Instance = new FParticleSpritePhysXEmitterInstance(*this);
	check(Instance);
	Instance->InitParameters(InEmitterParent, InComponent);

	return Instance;
#else	//#if WITH_NOVODEX
	return NULL;
#endif	//#if WITH_NOVODEX
}

void UParticleModuleTypeDataPhysX::SetToSensibleDefaults()
{
	Super::SetToSensibleDefaults();
	if(PhysXParSys == NULL) // If fluid not set, set the default one
		PhysXParSys = (UPhysXParticleSystem*)UObject::StaticLoadObject(UPhysXParticleSystem::StaticClass(), NULL, TEXT("EngineResources.DefaultPhysXParSys"), NULL, LOAD_None, NULL);
}

void UParticleModuleTypeDataPhysX::PreEditChange(UProperty* PropertyAboutToChange)
{
	if(PhysXParSys)
		PhysXParSys->RemovedFromScene();

	Super::PreEditChange(PropertyAboutToChange);
}

void UParticleModuleTypeDataPhysX::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if(PhysXParSys)
		PhysXParSys->RemovedFromScene();
}

void UParticleModuleTypeDataPhysX::FinishDestroy()
{
	Super::FinishDestroy();
}
