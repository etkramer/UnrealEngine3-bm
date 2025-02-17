/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleAttractorPoint extends ParticleModuleAttractorBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

/**	The position of the point attractor from the source of the emitter.		*/
var(Attractor)				rawdistributionvector	Position;

/**	The radial range of the attractor.										*/
var(Attractor)				rawdistributionfloat	Range;

/**	The strength of the point attractor.									*/
var(Attractor)				rawdistributionfloat	Strength;

/**	The strength curve is a function of distance or of time.				*/
var(Attractor) bool									StrengthByDistance;

/**	If TRUE, the velocity adjustment will be applied to the base velocity.	*/
var(Attractor) bool									bAffectBaseVelocity;

/**	If TRUE, set the velocity.												*/
var(Attractor) bool									bOverrideVelocity;

cpptext
{
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	Render3DPreview(FParticleEmitterInstance* Owner, const FSceneView* View,FPrimitiveDrawInterface* PDI);
}

defaultproperties
{
	bUpdateModule=true

	Begin Object Class=DistributionVectorConstant Name=DistributionPosition
	End Object
	Position=(Distribution=DistributionPosition)

	Begin Object Class=DistributionFloatConstant Name=DistributionRange
	End Object
	Range=(Distribution=DistributionRange)

	Begin Object Class=DistributionFloatConstant Name=DistributionStrength
	End Object
	Strength=(Distribution=DistributionStrength)
	
	StrengthByDistance=true
	bAffectBaseVelocity=false
	bOverrideVelocity=false

	bSupported3DDrawMode=true
}
