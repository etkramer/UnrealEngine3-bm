/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleEventReceiverSpawn extends ParticleModuleEventReceiverBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

/** The type of event that will generate the spawn. */
var(Source)		EParticleEventType			EventGeneratorType;

/** The name of the emitter of interest for generating the event. */
var(Source)		name						EventName;

/** The number of particles to spawn. */
var(Spawn)		rawdistributionfloat		SpawnCount;

/** 
 *	For Death-based event receiving, if this is TRUE, it indicates that the 
 *	ParticleTime of the event should be used to look-up the SpawnCount.
 *	Otherwise (and in all other events received), use the emitter time of 
 *	the event.
 */
var(Spawn)		bool						bUseParticleTime;

/**
 *	If TRUE, use the location of the particle system component for spawning.
 *	if FALSE (default), use the location of the particle event.
 */
var(Location)	bool						bUsePSysLocation;

/**
 *	If TRUE, use the velocity of the dying particle as the start velocity of 
 *	the spawned particle.
 */
var(Velocity)	bool						bInheritVelocity;

/**
 *	If bInheritVelocity is TRUE, scale the velocity with this.
 */
var(Velocity)	rawdistributionvector		InheritVelocityScale;

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
	 *	Called on a particle that is being updated by its emitter.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *	@param	Offset		The modules offset into the data payload of the particle.
	 *	@param	DeltaTime	The time since the last update.
	 */
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);

	/**
	 *	Returns the number of bytes that the module requires in the particle payload block.
	 *
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *
	 *	@return	UINT		The number of bytes the module needs per particle.
	 */
	virtual UINT	RequiredBytes(FParticleEmitterInstance* Owner = NULL);
	/**
	 *	Returns the number of bytes the module requires in the emitters 'per-instance' data block.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *
	 *	@return UINT		The number fo bytes the module needs per emitter instance.
	 */
	virtual UINT	RequiredBytesPerInstance(FParticleEmitterInstance* Owner = NULL);
	/**
	 *	Allows the module to prep its 'per-instance' data block.
	 *	
	 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
	 *	@param	InstData	Pointer to the data block for this module.
	 */
	virtual UINT	PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData);

	// Cascade
	/** Set the module to sensible default values - called on creation. */
	virtual void	SetToSensibleDefaults();
	/**
	 *	Called when the properties change in the property window.
	 *
	 *	@param	PropertyThatChanged		The property that was edited...
	 */
	virtual void	PostEditChange(UProperty* PropertyThatChanged);

	// Event Receiver functionality
	/**
	 *	Is the module interested in events of the given type?
	 *
	 *	@param	InEventType		The event type to check
	 *
	 *	@return	UBOOL			TRUE if interested.
	 */
	virtual UBOOL WillProcessEvent(EParticleEventType InEventType);

	/**
	 *	Process the event...
	 *
	 *	@param	Owner		The FParticleEmitterInstance this module is contained in.
	 *	@param	InEvent		The FParticleEventData that occurred.
	 *	@param	DeltaTime	The time slice of this frame.
	 *
	 *	@return	UBOOL		TRUE if the event was processed; FALSE if not.
	 */
	virtual UBOOL ProcessEvent(FParticleEmitterInstance* Owner, FParticleEventData& InEvent, FLOAT DeltaTime);
}

defaultproperties
{
	bSpawnModule=true
	bUpdateModule=true

	Begin Object Class=DistributionFloatConstant Name=RequiredDistributionSpawnCount
		Constant=0.0
	End Object
	SpawnCount=(Distribution=RequiredDistributionSpawnCount)

	Begin Object Class=DistributionVectorConstant Name=RequiredDistributionInheritVelocityScale
		Constant=(X=1.0,Y=1.0,Z=1.0)
	End Object
	InheritVelocityScale=(Distribution=RequiredDistributionInheritVelocityScale)

}
