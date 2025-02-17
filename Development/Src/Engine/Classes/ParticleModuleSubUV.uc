/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleSubUV extends ParticleModuleSubUVBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

/**
 *	The index of the sub-image that should be used for the particle.
 *	The value is retrieved using the RelativeTime of the particles.
 */
var(SubUV) rawdistributionfloat	SubImageIndex;

cpptext
{
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	SpawnSprite(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	SpawnMesh(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	UpdateSprite(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	UpdateMesh(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	
			UBOOL	DetermineSpriteImageIndex(FParticleEmitterInstance* Owner, FBaseParticle* Particle, 
						EParticleSubUVInterpMethod eMethod, FFullSubUVPayload& SubUVPayload, INT& ImageIndex, FLOAT& Interp, FLOAT DeltaTime);
			UBOOL	DetermineMeshImageIndex(FParticleEmitterInstance* Owner, FBaseParticle* Particle, 
						EParticleSubUVInterpMethod eMethod, FFullSubUVPayload& SubUVPayload, INT& ImageIndex, FLOAT& Interp, FLOAT DeltaTime);
		

	/**
	 *	Called when the module is created, this function allows for setting values that make
	 *	sense for the type of emitter they are being used in.
	 *
	 *	@param	Owner			The UParticleEmitter that the module is being added to.
	 */
	virtual void SetToSensibleDefaults(UParticleEmitter* Owner);
}

defaultproperties
{
	bSpawnModule=true
	bUpdateModule=true

	Begin Object Class=DistributionFloatConstant Name=DistributionSubImage
	End Object
	SubImageIndex=(Distribution=DistributionSubImage)
}
