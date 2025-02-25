/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleKillBox extends ParticleModuleKillBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

/** The lower left corner of the box. */
var(Kill)		rawdistributionvector	LowerLeftCorner;

/** The upper right corner of the box. */
var(Kill)		rawdistributionvector	UpperRightCorner;

/** If TRUE, the box coordinates are in world space./ */
var(Kill)		bool					bAbsolute;

/**
 *	If TRUE, particles INSIDE the box will be killed. 
 *	If FALSE (the default), particles OUTSIDE the box will be killed.
 */
var(Kill)		bool					bKillInside;

cpptext
{
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	Render3DPreview(FParticleEmitterInstance* Owner, const FSceneView* View,FPrimitiveDrawInterface* PDI);
}

defaultproperties
{
	bUpdateModule=true

	Begin Object Class=DistributionVectorConstant Name=DistributionLowerLeftCorner
	End Object
	LowerLeftCorner=(Distribution=DistributionLowerLeftCorner)

	Begin Object Class=DistributionVectorConstant Name=DistributionUpperRightCorner
	End Object
	UpperRightCorner=(Distribution=DistributionUpperRightCorner)

	bSupported3DDrawMode=true
}
