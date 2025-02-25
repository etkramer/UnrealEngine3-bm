/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleTypeDataBase extends ParticleModule
	native(Particle)
	editinlinenew
	hidecategories(Object)
	abstract;

cpptext
{
	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent);
	virtual void	SetToSensibleDefaults();
	virtual void	PreSpawn(FParticleEmitterInstance* Owner, FBaseParticle* Particle)			{};
	virtual void	PreUpdate(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)		{};
	virtual void	PostUpdate(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)	{};

	virtual EModuleType	GetModuleType() const							{	return EPMT_TypeData;	}
	virtual UBOOL		SupportsSpecificScreenAlignmentFlags() const	{	return FALSE;			}
}

defaultproperties
{
	bSpawnModule=false
	bUpdateModule=false
}
