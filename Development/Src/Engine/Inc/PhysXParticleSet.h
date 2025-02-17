/*=============================================================================
	PhysXParticleSet.h: PhysX Emitter Source.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#ifndef __PHYSXPARTICLESET_H__
#define __PHYSXPARTICLESET_H__

#include "EngineParticleClasses.h"

#if WITH_NOVODEX

/**
Per particle data representing the particle in a FPhysXParticleSet.
The data contains lifetime information as well as links to the particle 
queue and SDK particle data. 
*/
struct PhysXRenderParticle
{	
	enum PhysXRenderParticleFlags
	{
		PXRP_DeathQueue		= (1 << 0),
		PXRP_EarlyReduction = (1 << 1),
	};

	UINT ParticleIndex;
	UINT Id;
	FLOAT OneOverMaxLifetime;
	FLOAT RelativeTime;
	UINT Flags;
	WORD QueueIndex;
};

/**
Class to manage a subset of particles of a PhysXParticleSystem. 
It is used to track particles for a given emitter effect, such as 
graphical representation. 
*/
class FPhysXParticleSet
{
private:
	FPhysXParticleSet();

protected:

	FPhysXParticleSet(INT ElementSize, FPhysXEmitterVerticalLodProperties& LodProperties, UPhysXParticleSystem*& InPhysXParSys);
	virtual ~FPhysXParticleSet();

public:

	virtual void AsyncUpdate(FLOAT DeltaTime, UBOOL bProcessSimulationStep) = 0;
	virtual void RemoveAllParticles() = 0;
	virtual INT RemoveParticle(INT RenderParticleIndex, bool bRemoveFromPSys);

	void SyncPhysXData();
	void SyncRemoveParticle(UINT Id);
	void SyncParticleReduction(UINT NumParticleReduction);

	FORCEINLINE FLOAT GetWeightForFifo() { return VerticalLod.WeightForFifo; }
	FORCEINLINE FLOAT GetWeightForSpawnLod() { return VerticalLod.WeightForSpawnLod; }

	//actually returns a particle with type specific payload
	FORCEINLINE PhysXRenderParticle* GetRenderParticle(INT Index) 
	{ 
		return (PhysXRenderParticle*)(RenderParticlesBuffer + ElementSize*Index); 
	}

	FORCEINLINE INT GetNumRenderParticles() { return RenderParticlesNum; } 
	FORCEINLINE INT GetMaxRenderParticles() { return RenderParticlesMax; } 
	
	FPhysXParticleSystem& GetPSys() { check(PhysXParSys && PhysXParSys->PSys); return *PhysXParSys->PSys; }

private:

	UBOOL MoveParticleFromLifeToDeath(INT RenderParticleIndex);
	
	INT							ElementSize;

	BYTE*						RenderParticlesBuffer;
	INT							RenderParticlesMax;
	INT							RenderParticlesNum;

	class FPhysXParticleQueue*	LifetimeQueue;	
	class FPhysXParticleQueue*	DeathQueue;
	

	INT							NumReducedParticles;
	FLOAT						ReductionPopMeasure;
	FLOAT						ReductionEarlyMeasure;
	FLOAT						QTarget;

protected: 

	void RemoveRenderParticle(INT RenderParticleIndex, INT& ParticleIdToFix);
	
	INT RemoveParticleFast(INT RenderParticleIndex, bool bRemoveFromPSys);
	
	//actually adds a particle with type specific payload
	void AddParticle(const PhysXRenderParticle* RenderParticle);

	void DeathRowManagment();
	void AsyncParticleReduction(FLOAT DeltaTime);
	void RemoveAllParticlesInternal();

	TArray<INT>					TmpRenderIndices;

	FPhysXEmitterVerticalLodProperties& VerticalLod;
	UPhysXParticleSystem*&		PhysXParSys;
};

#endif	//#if WITH_NOVODEX

#endif
