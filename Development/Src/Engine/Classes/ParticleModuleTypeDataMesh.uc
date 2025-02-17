/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleTypeDataMesh extends ParticleModuleTypeDataBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

/** The static mesh to render at the particle positions */
var(Mesh)	StaticMesh				Mesh;
/** If TRUE, has the meshes cast shadows */
var(Mesh)	bool					CastShadows;
/** UNUSED (the collision module dictates doing collisions) */
var(Mesh)	bool					DoCollisions;

enum EMeshScreenAlignment
{
    PSMA_MeshFaceCameraWithRoll,
    PSMA_MeshFaceCameraWithSpin,
    PSMA_MeshFaceCameraWithLockedAxis
};

/** 
 *	The alignment to use on the meshes emitted.
 *	The RequiredModule->ScreenAlignment MUST be set to PSA_TypeSpecific to use.
 *	One of the following:
 *	PSMA_MeshFaceCameraWithRoll
 *		Face the camera allowing for rotation around the mesh-to-camera vector 
 *		(amount provided by the standard particle sprite rotation).  
 *	PSMA_MeshFaceCameraWithSpin
 *		Face the camera allowing for the mesh to rotate about the tangential axis.  
 *	PSMA_MeshFaceCameraWithLockedAxis
 *		Face the camera while maintaining the up vector as the locked direction.  
 */
var(Mesh)	EMeshScreenAlignment	MeshAlignment;

/**
 *	If TRUE, use the emitter material when rendering rather than the one applied 
 *	to the static mesh model.
 */
var(Mesh)	bool					bOverrideMaterial;

cpptext
{
	virtual void						PostEditChange(UProperty* PropertyThatChanged);
	virtual FParticleEmitterInstance*	CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent);
	virtual void						SetToSensibleDefaults();

	virtual UBOOL	SupportsSpecificScreenAlignmentFlags() const	{	return TRUE;	}
}

defaultproperties
{
	CastShadows=false
	DoCollisions=false
	MeshAlignment=PSMA_MeshFaceCameraWithRoll
}
