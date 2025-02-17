/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleSpawnPerUnit extends ParticleModuleSpawnBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

//=============================================================================
//	Properties
//=============================================================================
/** 
 *	The scalar to apply to the distance traveled.
 *	The value from SpawnPerUnit is divided by this value to give the actual
 *	number of particles per unit. 
 */
var(Spawn)							float						UnitScalar;
 
/** 
 *	The amount to spawn per meter distribution.
 *	The value is retrieved using the EmitterTime.
 */
var(Spawn)							rawdistributionfloat		SpawnPerUnit;

/**
 *	If TRUE, process the default spawn rate when not moving...
 *	When not moving, skip the default spawn rate.
 *	If FALSE, return the bProcessSpawnRate setting.
 */
var(Spawn)							bool						bIgnoreSpawnRateWhenMoving;

/**
 *	The tolerance for moving vs. not moving w.r.t. the bIgnoreSpawnRateWhenMoving flag.
 *	Ie, if (DistanceMoved < (UnitScalar * MovementTolerance)) then consider it not moving.
 */
var(Spawn)							float						MovementTolerance;

//=============================================================================
//	C++
//=============================================================================
cpptext
{
	/**
	 *	Called on a particle that is freshly spawned by the emitter.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that spawned the particle.
	 *	@param	Offset		The modules offset into the data payload of the particle.
	 *	@param	SpawnTime	The time of the spawn.
	 */
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	
	/**
	 *	Returns the number of bytes the module requires in the emitters 'per-instance' data block.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *
	 *	@return UINT		The number fo bytes the module needs per emitter instance.
	 */
	virtual UINT	RequiredBytesPerInstance(FParticleEmitterInstance* Owner = NULL);

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
	virtual UBOOL GetSpawnAmount(FParticleEmitterInstance* Owner, INT Offset, FLOAT OldLeftover, 
		FLOAT DeltaTime, INT& Number, FLOAT& Rate);
}

//=============================================================================
//	Default properties
//=============================================================================
defaultproperties
{
	bSpawnModule=true
	bUpdateModule=false

	UnitScalar=50.0
	Begin Object Class=DistributionFloatConstant Name=RequiredDistributionSpawnPerUnit
		Constant=0.0
	End Object
	SpawnPerUnit=(Distribution=RequiredDistributionSpawnPerUnit)
	
	MovementTolerance=0.1
}
