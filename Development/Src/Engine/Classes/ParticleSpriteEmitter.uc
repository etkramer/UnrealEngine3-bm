/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleSpriteEmitter extends ParticleEmitter
	native(Particle)
	collapsecategories		
	hidecategories(Object)
	editinlinenew;

enum EParticleScreenAlignment
{
	PSA_Square,
	PSA_Rectangle,
	PSA_Velocity,
	PSA_TypeSpecific
};

cpptext
{
	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent);
	virtual void SetToSensibleDefaults();
}
