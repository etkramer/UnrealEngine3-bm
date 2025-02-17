/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class ParticleModuleRequired extends ParticleModule
	native(Particle)
	editinlinenew
	hidecategories(Object,Cascade);

//=============================================================================
//	General
//=============================================================================
/** The material to utilize for the emitter at this LOD level.						*/
var(Emitter)						MaterialInterface			Material;
/** 
 *	The screen alignment to utilize for the emitter at this LOD level.
 *	One of the following:
 *	PSA_Square			- Uniform scale (via SizeX) facing the camera
 *	PSA_Rectangle		- Non-uniform scale (via SizeX and SizeY) facing the camera
 *	PSA_Velocity		- Orient the particle towards both the camera and the direction 
 *						  the particle is moving. Non-uniform scaling is allowed.
 *	PSA_TypeSpecific	- Use the alignment method indicated int he type data module.
 */
var(Emitter)						EParticleScreenAlignment	ScreenAlignment;

/** If TRUE, update the emitter in local space										*/
var(Emitter)						bool						bUseLocalSpace;
/** If TRUE, kill the emitter when the particle system is deactivated				*/
var(Emitter)						bool						bKillOnDeactivate;
/** If TRUE, kill the emitter when it completes										*/
var(Emitter)						bool						bKillOnCompleted;
/** Whether this emitter requires sorting as specified by artist.					*/
var(Emitter)						bool						bRequiresSorting;

/** 
 *	How long, in seconds, the emitter will run before looping.
 *	If set to 0, the emitter will never loop.
 */
var(Duration)						float						EmitterDuration;
/** 
 *	The low end of the emitter duration if using a range.
 */
var(Duration)						float						EmitterDurationLow;
/**
 *	If TRUE, select the emitter duration from the range 
 *		[EmitterDurationLow..EmitterDuration]
 */
var(Duration)						bool						bEmitterDurationUseRange;
/** 
 *	If TRUE, recalculate the emitter duration on each loop.
 */
var(Duration)						bool						bDurationRecalcEachLoop;

/** The number of times to loop the emitter.
 *	0 indicates loop continuously
 */
var(Duration)						int							EmitterLoops;

//=============================================================================
//	Spawn-related
//=============================================================================
/** The rate at which to spawn particles									*/
var							rawdistributionfloat		SpawnRate;

//=============================================================================
//	Burst-related
//=============================================================================
/** The method to utilize when burst-emitting particles						*/
var							EParticleBurstMethod		ParticleBurstMethod;

/** The array of burst entries.												*/
var		export noclear		array<ParticleBurst>		BurstList;

//=============================================================================
//	Delay-related
//=============================================================================
/**
 *	Indicates the time (in seconds) that this emitter should be delayed in the particle system.
 */
var(Delay)							float						EmitterDelay;
/**
 *	If TRUE, the emitter will be delayed only on the first loop.
 */
var(Delay)							bool						bDelayFirstLoopOnly;

//=============================================================================
//	SubUV-related
//=============================================================================
/** 
 *	The interpolation method to used for the SubUV image selection.
 *	One of the following:
 *	PSUVIM_None			- Do not apply SubUV modules to this emitter. 
 *	PSUVIM_Linear		- Smoothly transition between sub-images in the given order, 
 *						  with no blending between the current and the next
 *	PSUVIM_Linear_Blend	- Smoothly transition between sub-images in the given order, 
 *						  blending between the current and the next 
 *	PSUVIM_Random		- Pick the next image at random, with no blending between 
 *						  the current and the next 
 *	PSUVIM_Random_Blend	- Pick the next image at random, blending between the current 
 *						  and the next 
 */
var(SubUV)							EParticleSubUVInterpMethod	InterpolationMethod;

/** The number of sub-images horizontally in the texture							*/
var(SubUV)							int							SubImages_Horizontal;

/** The number of sub-images vertically in the texture								*/
var(SubUV)							int							SubImages_Vertical;

/** Whether to scale the UV or not - ie, the model wasn't setup with sub uvs		*/
var(SubUV)							bool						bScaleUV;

/**
 *	The amount of time (particle-relative, 0.0 to 1.0) to 'lock' on a random sub image
 *	    0.0 = change every frame
 *      1.0 = select a random image at spawn and hold for the life of the particle
 */
var									float						RandomImageTime;

/** The number of times to change a random image over the life of the particle.		*/
var(SubUV)							int							RandomImageChanges;

/** SUB-UV RELATIVE INTERNAL MEMBERS												*/
var									bool						bDirectUV;

/**
 *	If TRUE, use the MaxDrawCount to limit the number of particles rendered.
 *	NOTE: This does not limit the number spawned/updated, only what is drawn.
 */
var(Rendering)						bool						bUseMaxDrawCount;
/**
 *	The maximum number of particles to DRAW for this emitter.
 *	If set to 0, it will use whatever number are present.
 */
var(Rendering)						int							MaxDrawCount;

//=============================================================================
//	Cascade-related
//=============================================================================
/**
 *	How to render the emitter particles. Can be one of the following:
 *		ERM_Normal	- As the intended sprite/mesh
 *		ERM_Point	- As a 2x2 pixel block with no scaling and the color set in EmitterEditorColor
 *		ERM_Cross	- As a cross of lines, scaled to the size of the particle in EmitterEditorColor
 *		ERM_None	- Do not render
 */
var(Cascade)						EEmitterRenderMode			EmitterRenderMode;
/**
 *	The color of the emitter in the curve editor and debug rendering modes.
 */
var(Cascade)						color						EmitterEditorColor;

//=============================================================================
//	C++
//=============================================================================
cpptext
{
	virtual void	PostEditChange(UProperty* PropertyThatChanged);
	virtual void	PostLoad();

	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);

	/** 
	 *	Add all curve-editable Objects within this module to the curve editor.
	 *
	 *	@param	EdSetup		The CurveEd setup to use for adding curved.
	 */
	virtual	void	AddModuleCurvesToEditor(UInterpCurveEdSetup* EdSetup)
	{
		// Overide the base implementation to prevent old SpawnRate from being added...
	}

	/**
	 *	Retrieve the ModuleType of this module.
	 *
	 *	@return	EModuleType		The type of module this is.
	 */
	virtual EModuleType	GetModuleType() const	{	return EPMT_Required;	}

	virtual UBOOL	GenerateLODModuleValues(UParticleModule* SourceModule, FLOAT Percentage, UParticleLODLevel* LODLevel);
}

//=============================================================================
//	Default properties
//=============================================================================
defaultproperties
{
	bSpawnModule=true
	bUpdateModule=true

	EmitterDuration=1.0
	EmitterDurationLow=0.0
	bEmitterDurationUseRange=false
	EmitterLoops=0

	Begin Object Class=DistributionFloatConstant Name=RequiredDistributionSpawnRate
	End Object
	SpawnRate=(Distribution=RequiredDistributionSpawnRate)

	SubImages_Horizontal=1
	SubImages_Vertical=1

	bUseMaxDrawCount=true
	MaxDrawCount=500

	LODDuplicate=true

	IdenticalIgnoreProperties.Add("SpawnRate")
	IdenticalIgnoreProperties.Add("ParticleBurstMethod")
	IdenticalIgnoreProperties.Add("BurstList")
}
