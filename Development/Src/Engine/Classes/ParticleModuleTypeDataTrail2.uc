/**
 *	ParticleModuleTypeDataTrail2
 *	Provides the base data for trail emitters.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleTypeDataTrail2 extends ParticleModuleTypeDataBase
	native(Particle)
	editinlinenew
	hidecategories(Object);

//*************************************************************************************************
// General Trail Variables
//*************************************************************************************************
/** The tesselation amount to use for each trail				*/
var(Trail)		int		TessellationFactor;

/** The distance between particles for full TessellationFactor	*/
var				float	TessellationFactorDistance;

/** The strength to apply to the tangents						*/
var(Trail)		float	TessellationStrength;

/** The number of times to tile the texture along each trail	*/
var(Trail)		int		TextureTile;

/** The number of sheets to render								*/
var				int		Sheets;

/** The number of live trails									*/
var(Trail)		int		MaxTrailCount;

/** Max particles per trail										*/
var(Trail)		int		MaxParticleInTrailCount;

//*************************************************************************************************
// Trail Rendering Variables
//*************************************************************************************************
var(Rendering)	bool	RenderGeometry;
var(Rendering)	bool	RenderDirectLine;
var(Rendering)	bool	RenderLines;
var(Rendering)	bool	RenderTessellation;

//*************************************************************************************************
// C++ Text
//*************************************************************************************************
cpptext
{
	virtual void	PreSpawn(FParticleEmitterInstance* Owner, FBaseParticle* Particle);
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	PreUpdate(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual UINT	RequiredBytes(FParticleEmitterInstance* Owner = NULL);

	virtual void	SetToSensibleDefaults();
	virtual void	PostEditChange(UProperty* PropertyThatChanged);

	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent);

	// Trail
	virtual void	GetDataPointers(FParticleEmitterInstance* Owner, const BYTE* ParticleBase, 
			INT& CurrentOffset, FTrail2TypeDataPayload*& TrailData, FLOAT*& TaperValues);
	virtual void	GetDataPointerOffsets(FParticleEmitterInstance* Owner, const BYTE* ParticleBase, 
			INT& CurrentOffset, INT& TrailDataOffset, INT& TaperValuesOffset);
}

//*************************************************************************************************
// Default properties
//*************************************************************************************************
defaultproperties
{
	TessellationFactor=1
	TessellationFactorDistance=0.0
	TessellationStrength=25.0
	TextureTile=1
	Sheets=1
	MaxTrailCount=1
	MaxParticleInTrailCount=0

	RenderGeometry=true
	RenderDirectLine=false
	RenderLines=false
	RenderTessellation=false
}
