/*=============================================================================
	ParticleModules_Spawn.cpp: Particle spawn-related module implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "EngineParticleClasses.h"

/*-----------------------------------------------------------------------------
	Abstract base modules used for categorization.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UParticleModuleSpawnBase);

/*-----------------------------------------------------------------------------
	UParticleModuleSpawn implementation.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UParticleModuleSpawn);

void UParticleModuleSpawn::PostEditChange(UProperty* PropertyThatChanged)
{
	for (INT Index = 0; Index < BurstList.Num(); Index++)
	{
		FParticleBurst* Burst = &(BurstList(Index));

		// Clamp them to positive numbers...
		Burst->Count = Max<INT>(0, Burst->Count);
		if (Burst->CountLow > -1)
		{
			Burst->CountLow = Min<INT>(Burst->Count, Burst->CountLow);
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 *	Retrieve the spawn amount this module is contributing.
 *	Note that if multiple Spawn-specific modules are present, if any one
 *	of them ignores the SpawnRate processing it will be ignored.
 *
 *	@param	Owner		The particle emitter instance that is spawning.
 *	@param	Offset		The offset into the particle payload for the module.
 *	@param	OldLeftover	The bit of timeslice left over from the previous frame.
 *	@param	DeltaTime	The time that has expired since the last frame.
 *	@param	Number		The number of particles to spawn. (OUTPUT)
 *	@param	Rate		The spawn rate of the module. (OUTPUT)
 *
 *	@return	UBOOL		FALSE if the SpawnRate should be ignored.
 *						TRUE if the SpawnRate should still be processed.
 */
UBOOL UParticleModuleSpawn::GetSpawnAmount(FParticleEmitterInstance* Owner, 
	INT Offset, FLOAT OldLeftover, FLOAT DeltaTime, INT& Number, FLOAT& Rate)
{
	check(Owner);
	return FALSE;
}

UBOOL UParticleModuleSpawn::GenerateLODModuleValues(UParticleModule* SourceModule, FLOAT Percentage, UParticleLODLevel* LODLevel)
{
	// Convert the module values
	UParticleModuleSpawn* SpawnSource = Cast<UParticleModuleSpawn>(SourceModule);
	if (!SpawnSource)
	{
		return FALSE;
	}

	UBOOL bResult	= TRUE;
#if !CONSOLE
	//SpawnRate
	// @GEMINI_TODO: Make sure these functions are never called on console, or when the UDistributions are missing
	if (ConvertFloatDistribution(Rate.Distribution, SpawnSource->Rate.Distribution, Percentage) == FALSE)
	{
		bResult	= FALSE;
	}

	//ParticleBurstMethod
	//BurstList
	check(BurstList.Num() == SpawnSource->BurstList.Num());
	for (INT BurstIndex = 0; BurstIndex < SpawnSource->BurstList.Num(); BurstIndex++)
	{
		FParticleBurst* SourceBurst	= &(SpawnSource->BurstList(BurstIndex));
		FParticleBurst* Burst		= &(BurstList(BurstIndex));

		Burst->Time = SourceBurst->Time;
		// Don't drop below 1...
		if (Burst->Count > 0)
		{
			Burst->Count = appTrunc(SourceBurst->Count * (Percentage / 100.0f));
			if (Burst->Count == 0)
			{
				Burst->Count = 1;
			}
		}
	}
#endif	//#if !CONSOLE
	return bResult;
}

/*-----------------------------------------------------------------------------
	UParticleModuleSpawnPerUnit implementation.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UParticleModuleSpawnPerUnit);

/**
 *	Called on a particle that is freshly spawned by the emitter.
 *	
 *	@param	Owner		The FParticleEmitterInstance that spawned the particle.
 *	@param	Offset		The modules offset into the data payload of the particle.
 *	@param	SpawnTime	The time of the spawn.
 */
void UParticleModuleSpawnPerUnit::Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime)
{
	SPAWN_INIT;
	{
		PARTICLE_ELEMENT(FParticleSpawnPerUnitInstancePayload, SPUPayload);
		SPUPayload.CurrentDistanceTravelled = 0.0f;
	}
}

/**
 *	Returns the number of bytes the module requires in the emitters 'per-instance' data block.
 *	
 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
 *
 *	@return UINT		The number fo bytes the module needs per emitter instance.
 */
UINT UParticleModuleSpawnPerUnit::RequiredBytesPerInstance(FParticleEmitterInstance* Owner)
{
	return sizeof(FParticleSpawnPerUnitInstancePayload);
}

/**
 *	Retrieve the spawn amount this module is contributing.
 *	Note that if multiple Spawn-specific modules are present, if any one
 *	of them ignores the SpawnRate processing it will be ignored.
 *
 *	@param	Owner		The particle emitter instance that is spawning.
 *	@param	Offset		The offset into the particle payload for the module.
 *	@param	OldLeftover	The bit of timeslice left over from the previous frame.
 *	@param	DeltaTime	The time that has expired since the last frame.
 *	@param	Number		The number of particles to spawn. (OUTPUT)
 *	@param	Rate		The spawn rate of the module. (OUTPUT)
 *
 *	@return	UBOOL		FALSE if the SpawnRate should be ignored.
 *						TRUE if the SpawnRate should still be processed.
 */
UBOOL UParticleModuleSpawnPerUnit::GetSpawnAmount(FParticleEmitterInstance* Owner, 
	INT Offset, FLOAT OldLeftover, FLOAT DeltaTime, INT& Number, FLOAT& Rate)
{
	check(Owner);

	UBOOL bMoved = FALSE;
	FParticleSpawnPerUnitInstancePayload* SPUPayload = NULL;
	FLOAT NewTravelLeftover = 0.0f;

	FLOAT ParticlesPerUnit = SpawnPerUnit.GetValue(Owner->EmitterTime, Owner->Component) / UnitScalar;
	// Allow for PPU of 0.0f to allow for 'turning off' an emitter when moving
	if (ParticlesPerUnit >= 0.0f)
	{
		FLOAT LeftoverTravel = 0.0f;
		BYTE* InstData = Owner->GetModuleInstanceData(this);
		if (InstData)
		{
			SPUPayload = (FParticleSpawnPerUnitInstancePayload*)InstData;
			LeftoverTravel = SPUPayload->CurrentDistanceTravelled;
		}
		
		// Calculate movement delta over last frame, include previous remaining delta
		FVector TravelDirection = Owner->Location - Owner->OldLocation;
		// Calculate distance traveled
		FLOAT TravelDistance = TravelDirection.Size();

		if (TravelDistance > 0.0f)
		{
			if (TravelDistance > (MovementTolerance * UnitScalar))
			{
				bMoved = TRUE;
			}

			// Normalize direction for use later
			TravelDirection.Normalize();

			// Calculate number of particles to emit
			FLOAT NewLeftover = (TravelDistance + LeftoverTravel) * ParticlesPerUnit;
			Number = appFloor(NewLeftover);
			Rate = Number / DeltaTime;
			NewTravelLeftover = (TravelDistance + LeftoverTravel) - (Number * UnitScalar);
			if (SPUPayload)
			{
				SPUPayload->CurrentDistanceTravelled = Max<FLOAT>(0.0f, NewTravelLeftover);
			}

		}
		else
		{
			Number = 0;
			Rate = 0.0f;
		}
	}
	else
	{
		Number = 0;
		Rate = 0.0f;
	}

	if (bIgnoreSpawnRateWhenMoving == TRUE)
	{
		if (bMoved == TRUE)
		{
			return FALSE;
		}
		return TRUE;
	}

	return bProcessSpawnRate;
}
