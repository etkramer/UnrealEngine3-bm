/*=============================================================================
	UnParticleHelper.h: Particle helper definitions/ macros.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef HEADER_UNPARTICLEHELPER
#define HEADER_UNPARTICLEHELPER

#define _ENABLE_PARTICLE_LOD_INGAME_

#define PARTICLES_USE_DOUBLE_BUFFERING	0

/*-----------------------------------------------------------------------------
	Helper macros.
-----------------------------------------------------------------------------*/
//	Macro fun.
#define _PARTICLES_USE_PREFETCH_
#if defined(_PARTICLES_USE_PREFETCH_)
	#define	PARTICLE_PREFETCH(Index)					PREFETCH( ParticleData + ParticleStride * ParticleIndices[Index] )
	#define PARTICLE_INSTANCE_PREFETCH(Instance, Index)	PREFETCH( Instance->ParticleData + Instance->ParticleStride * Instance->ParticleIndices[Index] )
	#define	PARTICLE_OWNER_PREFETCH(Index)				PREFETCH( Owner->ParticleData + Owner->ParticleStride * Owner->ParticleIndices[Index] )
#else	//#if defined(_PARTICLES_USE_PREFETCH_)
	#define	PARTICLE_PREFETCH(Index)					
	#define	PARTICLE_INSTANCE_PREFETCH(Instance, Index)	
	#define	PARTICLE_OWNER_PREFETCH(Index)				
#endif	//#if defined(_PARTICLES_USE_PREFETCH_)

#define DECLARE_PARTICLE(Name,Address)		\
	FBaseParticle& Name = *((FBaseParticle*) (Address));

#define DECLARE_PARTICLE_PTR(Name,Address)		\
	FBaseParticle* Name = (FBaseParticle*) (Address);

#define BEGIN_UPDATE_LOOP																								\
	{																													\
		INT&			ActiveParticles = Owner->ActiveParticles;														\
		UINT			CurrentOffset	= Offset;																		\
		const BYTE*		ParticleData	= Owner->ParticleData;															\
		const UINT		ParticleStride	= Owner->ParticleStride;														\
		WORD*			ParticleIndices	= Owner->ParticleIndices;														\
		for(INT i=ActiveParticles-1; i>=0; i--)																			\
		{																												\
			const INT	CurrentIndex	= ParticleIndices[i];															\
			const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;									\
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);												\
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)															\
			{																											\

#define END_UPDATE_LOOP																									\
			}																											\
			CurrentOffset				= Offset;																		\
		}																												\
	}

#define CONTINUE_UPDATE_LOOP																							\
		CurrentOffset = Offset;																							\
		continue;

#define SPAWN_INIT																										\
	const INT		ActiveParticles	= Owner->ActiveParticles;															\
	const UINT		ParticleStride	= Owner->ParticleStride;															\
	const BYTE*		ParticleBase	= Owner->ParticleData + Owner->ParticleIndices[ActiveParticles] * ParticleStride;	\
	UINT			CurrentOffset	= Offset;																			\
	FBaseParticle&	Particle		= *((FBaseParticle*) ParticleBase);

#define PARTICLE_ELEMENT(Type,Name)																						\
	Type& Name = *((Type*)(ParticleBase + CurrentOffset));																\
	CurrentOffset += sizeof(Type);

#define KILL_CURRENT_PARTICLE																							\
	{																													\
		ParticleIndices[i]					= ParticleIndices[ActiveParticles-1];										\
		ParticleIndices[ActiveParticles-1]	= CurrentIndex;																\
		ActiveParticles--;																								\
	}

/*-----------------------------------------------------------------------------
	Helper functions.
-----------------------------------------------------------------------------*/

static FLinearColor ColorFromVector(const FVector& ColorVec, const FLOAT fAlpha);

/*-----------------------------------------------------------------------------
	Forward declarations
-----------------------------------------------------------------------------*/
//	Emitter and module types
class UParticleEmitter;
class UParticleSpriteEmitter;
class UParticleModule;
// Data types
class UParticleModuleTypeDataMesh;
class UParticleModuleTypeDataTrail;
class UParticleModuleTypeDataBeam;
class UParticleModuleTypeDataBeam2;
class UParticleModuleTypeDataTrail2;

class UStaticMeshComponent;

class UParticleSystem;
class UParticleSystemComponent;

class UParticleModuleBeamSource;
class UParticleModuleBeamTarget;
class UParticleModuleBeamNoise;
class UParticleModuleBeamModifier;

class UParticleModuleTrailSource;
class UParticleModuleTrailSpawn;
class UParticleModuleTrailTaper;

class UParticleModuleOrientationAxisLock;

class UParticleLODLevel;

class FParticleSystemSceneProxy;
class FParticleDynamicData;
struct FDynamicBeam2EmitterData;
struct FDynamicTrail2EmitterData;

struct FParticleSpriteEmitterInstance;
struct FParticleSpriteSubUVEmitterInstance;
struct FParticleMeshEmitterInstance;
struct FParticleTrailEmitterInstance;
struct FParticleBeamEmitterInstance;
struct FParticleBeam2EmitterInstance;
struct FParticleTrail2EmitterInstance;

void PS_DumpBeamDataInformation(TCHAR* Message, 
	UParticleSystemComponent* PSysComp, FParticleSystemSceneProxy* Proxy, 
	FParticleDynamicData* NewPSDynamicData, FParticleDynamicData* OldPSDynamicData, 
	FDynamicBeam2EmitterData* NewBeamData, FDynamicBeam2EmitterData* OldBeamData);
void PS_DumpTrailDataInformation(TCHAR* Message, 
	UParticleSystemComponent* PSysComp, FParticleSystemSceneProxy* Proxy, 
	FParticleDynamicData* NewPSDynamicData, FParticleDynamicData* OldPSDynamicData, 
	FDynamicTrail2EmitterData* NewTrailData, FDynamicTrail2EmitterData* OldTrailData);

// Special module indices...
#define INDEX_TYPEDATAMODULE	(INDEX_NONE - 1)
#define INDEX_REQUIREDMODULE	(INDEX_NONE - 2)
#define INDEX_SPAWNMODULE		(INDEX_NONE - 3)

/*-----------------------------------------------------------------------------
	FBaseParticle
-----------------------------------------------------------------------------*/
// Mappings for 'standard' particle data
// Only used when required.
struct FBaseParticle
{
	// 16 bytes
	FVector			OldLocation;			// Last frame's location, used for collision
	FLOAT			RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death)

	// 16 bytes
	FVector			Location;				// Current location
	FLOAT			OneOverMaxLifetime;		// Reciprocal of lifetime

	// 16 bytes
	FVector			BaseVelocity;			// Velocity = BaseVelocity at the start of each frame.
	FLOAT			Rotation;				// Rotation of particle (in Radians)

	// 16 bytes
	FVector			Velocity;				// Current velocity, gets reset to BaseVelocity each frame to allow 
	FLOAT			BaseRotationRate;		// Initial angular velocity of particle (in Radians per second)

	// 16 bytes
	FVector			BaseSize;				// Size = BaseSize at the start of each frame
	FLOAT			RotationRate;			// Current rotation rate, gets reset to BaseRotationRate each frame

	// 16 bytes
	FVector			Size;					// Current size, gets reset to BaseSize each frame
	INT				Flags;					// Flags indicating various particle states

	// 16 bytes
	FLinearColor	Color;					// Current color of particle.

	// 16 bytes
	FLinearColor	BaseColor;				// Base color of the particle
};

/*-----------------------------------------------------------------------------
	Particle State Flags
-----------------------------------------------------------------------------*/
enum EParticleStates
{
	/** Ignore updates to the particle						*/
	STATE_Particle_Freeze				= 0x00000001,
	/** Ignore collision updates to the particle			*/
	STATE_Particle_IgnoreCollisions		= 0x00000002,
	/**	Stop translations of the particle					*/
	STATE_Particle_FreezeTranslation	= 0x00000004,
	/**	Stop rotations of the particle						*/
	STATE_Particle_FreezeRotation		= 0x00000008,
	/** Delay collision updates to the particle				*/
	STATE_Particle_DelayCollisions		= 0x00000010,
	/** Flag indicating the particle has had at least one collision	*/
	STATE_Particle_CollisionHasOccurred	= 0x00000020,
};

/*-----------------------------------------------------------------------------
	FParticlesStatGroup
-----------------------------------------------------------------------------*/
enum EParticleStats
{
	STAT_SpriteParticles = STAT_ParticlesFirstStat,
	STAT_SpriteParticlesRenderCalls,
	STAT_SpriteParticlesSpawned,
	STAT_SpriteParticlesUpdated,
	STAT_SpriteParticlesKilled,
	STAT_SortingTime,
	STAT_SpriteRenderingTime,
	STAT_SpriteResourceUpdateTime,
	STAT_SpriteTickTime,
	STAT_SpriteSpawnTime,
	STAT_SpriteUpdateTime,
	STAT_PSysCompTickTime,
	STAT_ParticleCollisionTime,
	STAT_ParticlePoolTime,
	STAT_ParticleTickTime,
	STAT_ParticleRenderingTime,
	STAT_ParticleSetTemplateTime,
	STAT_ParticleInitializeTime,
	STAT_ParticleActivateTime,
	STAT_ParticleUpdateInstancesTime,
};

enum EParticleMeshStats
{
	STAT_MeshParticles = STAT_MeshParticlesFirstStat,
	STAT_MeshRenderingTime,
	STAT_MeshTickTime,
	STAT_MeshSpawnTime,
	STAT_MeshUpdateTime
};

//	FParticleSpriteVertex
struct FParticleSpriteVertex
{
	/** The position of the particle					*/
	FVector			Position;
	/** The previous position of the particle			*/
	FVector			OldPosition;
	/** The size of the particle						*/
	FVector			Size;
	/** The rotation of the particle					*/
	FLOAT			Rotation;
	/** The color of the particle						*/
	FLinearColor	Color;
	/** The UV values of the particle					*/
	FLOAT			Tex_U;
	FLOAT			Tex_V;
};

//	FParticleSpriteVertexDynamicParameter
struct FParticleSpriteVertexDynamicParameter : public FParticleSpriteVertex
{
	/** The dynamic paramter of the particle					*/
	FVector4		DynamicValue;
};

//	FParticleSpriteSubUVVertex
struct FParticleSpriteSubUVVertex : public FParticleSpriteVertex
{
	/** The second UV set for the particle				*/
	FLOAT			Tex_U2;
	FLOAT			Tex_V2;
	/** The interpolation value							*/
	FLOAT			Interp;
	/** Padding...										*/
	FLOAT			Padding;
	/** The size of the sub-image						*/
	FLOAT			SizeU;
	FLOAT			SizeV;
};

//	FParticleSpriteSubUVVertexDynamicParameter
struct FParticleSpriteSubUVVertexDynamicParameter : public FParticleSpriteSubUVVertex
{
	/** The dynamic paramter of the particle					*/
	FVector4		DynamicValue;
};

//
//  Trail emitter flags and macros
//
// ForceKill: Indicates all the particles in the trail should be killed in the next KillParticles call.
#define TRAIL_EMITTER_FLAG_FORCEKILL	0x00000000
// DeadTrail: indicates that the particle is the start of a trail than should no longer spawn.
//			  It should just fade out as the particles die...
#define TRAIL_EMITTER_FLAG_DEADTRAIL	0x10000000
// Middle: indicates the particle is in the middle of a trail.
#define TRAIL_EMITTER_FLAG_MIDDLE       0x20000000
// Start: indicates the particle is the start of a trail.
#define TRAIL_EMITTER_FLAG_START        0x40000000
// End: indicates the particle is the end of a trail.
#define TRAIL_EMITTER_FLAG_END          0x80000000

//#define TRAIL_EMITTER_FLAG_ONLY	        (TRAIL_EMITTER_FLAG_START | TRAIL_EMITTER_FLAG_END)
#define TRAIL_EMITTER_FLAG_MASK         0xf0000000
#define TRAIL_EMITTER_PREV_MASK         0x0fffc000
#define TRAIL_EMITTER_PREV_SHIFT        14
#define TRAIL_EMITTER_NEXT_MASK         0x00003fff
#define TRAIL_EMITTER_NEXT_SHIFT        0

#define TRAIL_EMITTER_NULL_PREV			(TRAIL_EMITTER_PREV_MASK >> TRAIL_EMITTER_PREV_SHIFT)
#define TRAIL_EMITTER_NULL_NEXT			(TRAIL_EMITTER_NEXT_MASK >> TRAIL_EMITTER_NEXT_SHIFT)

// Helper macros
#define TRAIL_EMITTER_CHECK_FLAG(val, mask, flag)				((val & mask) == flag)
#define TRAIL_EMITTER_SET_FLAG(val, mask, flag)					((val & ~mask) | flag)
#define TRAIL_EMITTER_GET_PREVNEXT(val, mask, shift)			((val & mask) >> shift)
#define TRAIL_EMITTER_SET_PREVNEXT(val, mask, shift, setval)	((val & ~mask) | ((setval << shift) & mask))

// Start/end accessor macros
#define TRAIL_EMITTER_IS_START(index)       TRAIL_EMITTER_CHECK_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_START)
#define TRAIL_EMITTER_SET_START(index)      TRAIL_EMITTER_SET_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_START)

#define TRAIL_EMITTER_IS_END(index)			TRAIL_EMITTER_CHECK_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_END)
#define TRAIL_EMITTER_SET_END(index)		TRAIL_EMITTER_SET_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_END)

#define TRAIL_EMITTER_IS_MIDDLE(index)		TRAIL_EMITTER_CHECK_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_MIDDLE)
#define TRAIL_EMITTER_SET_MIDDLE(index)		TRAIL_EMITTER_SET_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_MIDDLE)

// Only is used for the first emission from the emitter
#define TRAIL_EMITTER_IS_ONLY(index)		TRAIL_EMITTER_CHECK_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_START)	&& \
											(TRAIL_EMITTER_GET_NEXT(index) == TRAIL_EMITTER_NULL_NEXT)
#define TRAIL_EMITTER_SET_ONLY(index)		TRAIL_EMITTER_SET_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_START);	\
											TRAIL_EMITTER_SET_START(index)

#define TRAIL_EMITTER_IS_FORCEKILL(index)	TRAIL_EMITTER_CHECK_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_FORCEKILL)
#define TRAIL_EMITTER_SET_FORCEKILL(index)	TRAIL_EMITTER_SET_FLAG(index, TRAIL_EMITTER_FLAG_MASK, TRAIL_EMITTER_FLAG_FORCEKILL)

// Prev/Next accessor macros
#define TRAIL_EMITTER_GET_PREV(index)       TRAIL_EMITTER_GET_PREVNEXT(index, TRAIL_EMITTER_PREV_MASK, TRAIL_EMITTER_PREV_SHIFT)
#define TRAIL_EMITTER_SET_PREV(index, prev) TRAIL_EMITTER_SET_PREVNEXT(index, TRAIL_EMITTER_PREV_MASK, TRAIL_EMITTER_PREV_SHIFT, prev)
#define TRAIL_EMITTER_GET_NEXT(index)       TRAIL_EMITTER_GET_PREVNEXT(index, TRAIL_EMITTER_NEXT_MASK, TRAIL_EMITTER_NEXT_SHIFT)
#define TRAIL_EMITTER_SET_NEXT(index, next) TRAIL_EMITTER_SET_PREVNEXT(index, TRAIL_EMITTER_NEXT_MASK, TRAIL_EMITTER_NEXT_SHIFT, next)

/**
 * Particle trail stats
 */
enum EParticleTrailStats
{
	STAT_TrailParticles = STAT_TrailParticlesFirstStat,
	STAT_TrailParticlesRenderCalls,
	STAT_TrailParticlesSpawned,
	STAT_TrailParticlesTickCalls,
	STAT_TrailParticlesUpdated,
	STAT_TrailParticlesKilled,
	STAT_TrailParticlesTrianglesRendered,
	STAT_TrailFillVertexTime,
	STAT_TrailFillIndexTime,
	STAT_TrailRenderingTime,
	STAT_TrailTickTime,
	STAT_TrailSpawnTime,
	STAT_TrailUpdateTime,
	STAT_TrailPSysCompTickTime
};

/**
 * Beam particle stats
 */
enum EBeamParticleStats
{
	STAT_BeamParticles = STAT_BeamParticlesFirstStat,
	STAT_BeamParticlesRenderCalls,
	STAT_BeamParticlesSpawned,
	STAT_BeamParticlesUpdateCalls,
	STAT_BeamParticlesUpdated,
	STAT_BeamParticlesKilled,
	STAT_BeamParticlesTrianglesRendered,
	STAT_BeamSpawnTime,
	STAT_BeamFillVertexTime,
	STAT_BeamFillIndexTime,
	STAT_BeamRenderingTime,
	STAT_BeamTickTime
};

#define _BEAM2_USE_MODULES_

/** Structure for multiple beam targets.								*/
struct FBeamTargetData
{
	/** Name of the target.												*/
	FName		TargetName;
	/** Percentage chance the target will be selected (100 = always).	*/
	FLOAT		TargetPercentage;
};

//
//	Helper structures for payload data...
//

//
//	SubUV-related payloads
//
struct FFullSubUVPayload
{
	FLOAT	RandomImageTime;
	// Sprite:	ImageH, ImageV, Interpolation
	// Mesh:	UVOffset
	FVector	ImageHVInterp_UVOffset;
	// Sprite:	Image2H, Image2V
	// Mesh:	UV2Offset
	FVector	Image2HV_UV2Offset;
};

//
//	AttractorParticle
//
struct FAttractorParticlePayload
{
	INT			SourceIndex;
	UINT		SourcePointer;
	FVector		SourceVelocity;
};

//
//	TypeDataBeam2 payload
//
#define BEAM2_TYPEDATA_LOCKED_MASK					0x80000000
#define	BEAM2_TYPEDATA_LOCKED(x)					((x & BEAM2_TYPEDATA_LOCKED_MASK) != 0)
#define	BEAM2_TYPEDATA_SETLOCKED(x, Locked)			(x = Locked ? (x | BEAM2_TYPEDATA_LOCKED_MASK) : (x & ~BEAM2_TYPEDATA_LOCKED_MASK))

#define BEAM2_TYPEDATA_NOISEMAX_MASK				0x40000000
#define	BEAM2_TYPEDATA_NOISEMAX(x)					((x & BEAM2_TYPEDATA_NOISEMAX_MASK) != 0)
#define	BEAM2_TYPEDATA_SETNOISEMAX(x, Max)			(x = Max ? (x | BEAM2_TYPEDATA_NOISEMAX_MASK) : (x & ~BEAM2_TYPEDATA_NOISEMAX_MASK))

#define BEAM2_TYPEDATA_NOISEPOINTS_MASK				0x00000fff
#define	BEAM2_TYPEDATA_NOISEPOINTS(x)				(x & BEAM2_TYPEDATA_NOISEPOINTS_MASK)
#define BEAM2_TYPEDATA_SETNOISEPOINTS(x, Count)		(x = (x & ~BEAM2_TYPEDATA_NOISEPOINTS_MASK) | Count)

#define BEAM2_TYPEDATA_FREQUENCY_MASK				0x00fff000
#define BEAM2_TYPEDATA_FREQUENCY_SHIFT				12
#define	BEAM2_TYPEDATA_FREQUENCY(x)					((x & BEAM2_TYPEDATA_FREQUENCY_MASK) >> BEAM2_TYPEDATA_FREQUENCY_SHIFT)
#define BEAM2_TYPEDATA_SETFREQUENCY(x, Freq)		(x = ((x & ~BEAM2_TYPEDATA_FREQUENCY_MASK) | (Freq << BEAM2_TYPEDATA_FREQUENCY_SHIFT)))

struct FBeam2TypeDataPayload
{
	/** The source of this beam											*/
	FVector		SourcePoint;
	/** The source tangent of this beam									*/
	FVector		SourceTangent;
	/** The stength of the source tangent of this beam					*/
	FLOAT		SourceStrength;

	/** The target of this beam											*/
	FVector		TargetPoint;
	/** The target tangent of this beam									*/
	FVector		TargetTangent;
	/** The stength of the Target tangent of this beam					*/
	FLOAT		TargetStrength;

	/** Target lock, extreme max, Number of noise points				*/
	INT			Lock_Max_NumNoisePoints;

	/** Number of segments to render (steps)							*/
	INT			InterpolationSteps;

	/** Direction to step in											*/
	FVector		Direction;
	/** StepSize (for each segment to be rendered)						*/
	FLOAT		StepSize;
	/** Number of segments to render (steps)							*/
	INT			Steps;
	/** The 'extra' amount to travel (partial segment)					*/
	FLOAT		TravelRatio;

	/** The number of triangles to render for this beam					*/
	INT			TriangleCount;

	/**
	 *	Type and indexing flags
	 * 3               1              0
	 * 1...|...|...|...5...|...|...|..0
	 * TtPppppppppppppppNnnnnnnnnnnnnnn
	 * Tt				= Type flags --> 00 = Middle of Beam (nothing...)
	 * 									 01 = Start of Beam
	 * 									 10 = End of Beam
	 * Ppppppppppppppp	= Previous index
	 * Nnnnnnnnnnnnnnn	= Next index
	 * 		INT				Flags;
	 * 
	 * NOTE: These values DO NOT get packed into the vertex buffer!
	 */
	INT			Flags;
};

/**	Particle Source/Target Data Payload									*/
struct FBeamParticleSourceTargetPayloadData
{
	INT			ParticleIndex;
};

/**	Particle Source Branch Payload										*/
struct FBeamParticleSourceBranchPayloadData
{
	INT			NoiseIndex;
};

/** Particle Beam Modifier Data Payload */
struct FBeamParticleModifierPayloadData
{
	BITFIELD	bModifyPosition:1;
	BITFIELD	bScalePosition:1;
	BITFIELD	bModifyTangent:1;
	BITFIELD	bScaleTangent:1;
	BITFIELD	bModifyStrength:1;
	BITFIELD	bScaleStrength:1;
	FVector		Position;
	FVector		Tangent;
	FLOAT		Strength;

	// Helper functions
	FORCEINLINE void UpdatePosition(FVector& Value)
	{
		if (bModifyPosition == TRUE)
		{
			if (bScalePosition == FALSE)
			{
				Value += Position;
			}
			else
			{
				Value *= Position;
			}
		}
	}

	FORCEINLINE void UpdateTangent(FVector& Value, UBOOL bAbsolute)
	{
		if (bModifyTangent == TRUE)
		{
			FVector ModTangent = Tangent;

			if (bAbsolute == FALSE)
			{
				// Transform the modified tangent so it is relative to the real tangent
				FQuat RotQuat = FQuatFindBetween(FVector(1.0f, 0.0f, 0.0f), Value);
				FMatrix RotMat = FQuatRotationTranslationMatrix(RotQuat, FVector(0.0f));

				ModTangent = RotMat.TransformNormal(Tangent);
			}

			if (bScaleTangent == FALSE)
			{
				Value += ModTangent;
			}
			else
			{
				Value *= ModTangent;
			}
		}
	}

	FORCEINLINE void UpdateStrength(FLOAT& Value)
	{
		if (bModifyStrength == TRUE)
		{
			if (bScaleStrength == FALSE)
			{
				Value += Strength;
			}
			else
			{
				Value *= Strength;
			}
		}
	}
};

//
//	Trail2 payload data
//
struct FTrail2TypeDataPayload
{
	/**
	 *	Type and indexing flags
	 * 3               1              0
	 * 1...|...|...|...5...|...|...|..0
	 * TtPppppppppppppppNnnnnnnnnnnnnnn
	 * Tt				= Type flags --> 00 = Middle of Beam (nothing...)
	 * 									 01 = Start of Beam
	 * 									 10 = End of Beam
	 * Ppppppppppppppp	= Previous index
	 * Nnnnnnnnnnnnnnn	= Next index
	 * 		INT				Flags;
	 * 
	 * NOTE: These values DO NOT get packed into the vertex buffer!
	 */
	INT			Flags;

	/** The trail index - START only							*/
	INT			TrailIndex;
	/** The number of triangle in the trail	- START only		*/
	INT			TriangleCount;
	/** The velocity of the particle - to allow moving trails	*/
	FVector		Velocity;
	/**	Tangent for the trail segment							*/
	FVector		Tangent;
};

/**	Particle Source Data Payload									*/
struct FTrailParticleSourcePayloadData
{
	INT			ParticleIndex;
};

/** Mesh rotation data payload										*/
struct FMeshRotationPayloadData
{
	FVector  Rotation;
	FVector  RotationRate;
	FVector  RotationRateBase;
};

/** ModuleLocationEmitter instance payload							*/
struct FLocationEmitterInstancePayload
{
	INT		LastSelectedIndex;
};

/**
 *	Chain-able Orbit module instance payload
 */
struct FOrbitChainModuleInstancePayload
{
	/** The base offset of the particle from it's tracked location	*/
	FVector	BaseOffset;
	/** The offset of the particle from it's tracked location		*/
	FVector	Offset;
	/** The rotation of the particle at it's offset location		*/
	FVector	Rotation;
	/** The base rotation rate of the particle offset				*/
	FVector	BaseRotationRate;
	/** The rotation rate of the particle offset					*/
	FVector	RotationRate;
	/** The offset of the particle from the last frame				*/
	FVector	PreviousOffset;
};

/**
 *	Payload for instances which use the SpawnPerUnit module.
 */
struct FParticleSpawnPerUnitInstancePayload
{
	FLOAT	CurrentDistanceTravelled;
};

/**
 *	Collision module particle payload
 */
struct FParticleCollisionPayload
{
	FVector	UsedDampingFactor;
	FVector	UsedDampingFactorRotation;
	INT		UsedCollisions;
	FLOAT	Delay;
};

/**
 *	General event instance payload.
 */
struct FParticleEventInstancePayload
{
	/** Spawn event instance payload. */
	UBOOL bSpawnEventsPresent;
	INT SpawnTrackingCount;
	/** Death event instance payload. */
	UBOOL bDeathEventsPresent;
	INT DeathTrackingCount;
	/** Collision event instance payload. */
	UBOOL bCollisionEventsPresent;
	INT CollisionTrackingCount;
};

/**
 *	DynamicParameter particle payload.
 */
struct FEmitterDynamicParameterPayload
{
	/** The float4 value to assign to the dynamic parameter. */
	FVector4 DynamicParameterValue;
};

/*-----------------------------------------------------------------------------
	Particle Sorting Helper
-----------------------------------------------------------------------------*/
struct FParticleOrder
{
	INT		ParticleIndex;
	FLOAT	Z;
	
	FParticleOrder(INT InParticleIndex,FLOAT InZ):
		ParticleIndex(InParticleIndex),
		Z(InZ)
	{}
};

/*-----------------------------------------------------------------------------
	Particle Dynamic Data
-----------------------------------------------------------------------------*/
/**
 *	The information required for rendering sprite-based particles
 */
struct FParticleSpriteData
{
	/** Current location of the particle.			*/
	FVector			Location;
	/** Last frame's location of the particle.		*/
	FVector			OldLocation;
	/** Rotation of the particle (in Radians).		*/
	FLOAT			Rotation;
	/** Current size of the particle.				*/
	FVector			Size;
	/** Current color of the particle.				*/
	FLinearColor	Color;
};



/**
 * Dynamic particle emitter types
 *
 * NOTE: These are serialized out for particle replay data, so be sure to update all appropriate
 *    when changing anything here.
 */
enum EDynamicEmitterType
{
	DET_Unknown = 0,
	DET_Sprite,
	DET_SubUV,
	DET_Mesh,
	DET_Beam,
	DET_Beam2,
	DET_Trail,
	DET_Trail2,
	DET_Custom
};




/** Source data base class for all emitter types */
struct FDynamicEmitterReplayDataBase
{
	/**	The type of emitter. */
	EDynamicEmitterType	eEmitterType;

	/**	The number of particles currently active in this emitter. */
	INT ActiveParticleCount;

	INT ParticleStride;
	TArray< BYTE > ParticleData;
	TArray< WORD > ParticleIndices;
	FVector Scale;

	/** Whether this emitter requires sorting as specified by artist.	*/
	UBOOL bRequiresSorting;


	/** Constructor */
	FDynamicEmitterReplayDataBase()
		: eEmitterType( DET_Unknown ),
		  ActiveParticleCount( 0 ),
		  ParticleStride( 0 ),
		  Scale( FVector( 1.0f ) ),
		  bRequiresSorting( FALSE )
	{
	}


	/** Serialization */
	virtual void Serialize( FArchive& Ar )
	{
		INT EmitterTypeAsInt = eEmitterType;
		Ar << EmitterTypeAsInt;
		eEmitterType = static_cast< EDynamicEmitterType >( EmitterTypeAsInt );

		Ar << ActiveParticleCount;
		Ar << ParticleStride;
		Ar << ParticleData;
		Ar << ParticleIndices;
		Ar << Scale;
		Ar << bRequiresSorting;
	}

};



/** Base class for all emitter types */
struct FDynamicEmitterDataBase
{
	FDynamicEmitterDataBase()
		: bSelected(FALSE)
		, SceneProxy(NULL)
	{
	}
	
	virtual ~FDynamicEmitterDataBase()
	{
	}


	/**
	* @return TRUE if the particle system uses FMeshElement.UseDynamicData=TRUE
	*/
	virtual UBOOL HasDynamicMeshElementData()
	{
		// by default all emitters assumed to use dynamic vertex data
		return TRUE;
	}

	// Render thread only draw call
	virtual void Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex) = 0;
	virtual const FMaterialRenderProxy*		GetMaterialRenderProxy() = 0;

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	Proxy			The 'owner' particle system scene proxy
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber) {}

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

	/** Release the resource for the data */
	virtual void ReleaseResource() {}

	BITFIELD	bSelected : 1;
	BITFIELD	bValid : 1;

	/** The scene proxy - only used during rendering!					*/
	FParticleSystemSceneProxy* SceneProxy;
};



/** Source data base class for Sprite emitters */
struct FDynamicSpriteEmitterReplayDataBase
	: public FDynamicEmitterReplayDataBase
{
	BYTE						ScreenAlignment;
	UBOOL						bUseLocalSpace;
	UBOOL						bLockAxis;
	BYTE						LockAxisFlag;
	INT							MaxDrawCount;
	INT							EmitterRenderMode;
	INT							OrbitModuleOffset;
	INT							DynamicParameterDataOffset;
	UMaterialInterface*			MaterialInterface;


	/** Constructor */
	FDynamicSpriteEmitterReplayDataBase()
		: ScreenAlignment(0)
		, bUseLocalSpace(FALSE)
		, bLockAxis(FALSE)
		, LockAxisFlag(0)
		, MaxDrawCount(0)
		, OrbitModuleOffset(0)
		, DynamicParameterDataOffset(0)
		, MaterialInterface( NULL )
	{
	}

	/** Serialization */
	virtual void Serialize( FArchive& Ar );

};



/** Base class for Sprite emitters and other emitter types that share similar features. */
struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
	FDynamicSpriteEmitterDataBase()
		: MaterialResource( NULL ),
		  bUsesDynamicParameter( FALSE )
	{
	}

	virtual ~FDynamicSpriteEmitterDataBase()
	{
	}

	const FMaterialRenderProxy* GetMaterialRenderProxy() 
	{ 
		return MaterialResource; 
	}
	
	virtual void RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses);

	const FMaterialRenderProxy*	MaterialResource;
	UBOOL						bUsesDynamicParameter;
};




/** Source data for Sprite emitters */
struct FDynamicSpriteEmitterReplayData
	: public FDynamicSpriteEmitterReplayDataBase
{
	// Nothing needed, yet


	/** Constructor */
	FDynamicSpriteEmitterReplayData()
	{
	}


	/** Serialization */
	virtual void Serialize( FArchive& Ar )
	{
		// Call parent implementation
		FDynamicSpriteEmitterReplayDataBase::Serialize( Ar );

		// ...
	}

};



/** Dynamic emitter data for sprite emitters */
struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
	FDynamicSpriteEmitterData()
		: PrimitiveCount( 0 ),
		  ParticleOrder(),
		  VertexFactory( NULL )
	{
	}

	virtual ~FDynamicSpriteEmitterData()
	{
		if(VertexFactory)
		{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
			VertexFactory->ReleaseResource();
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
			delete VertexFactory;
		}
	}

	/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
	void Init( UBOOL bInSelected );

	virtual UBOOL GetVertexAndIndexData(void* VertexData, void* FillIndexData, TArray<FParticleOrder>* ParticleOrder);

	// Render thread only draw call
	virtual void Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const
	{
		return Source;
	}

	/** Release the resource for the data */
	virtual void ReleaseResource()
	{
#if PARTICLES_USE_DOUBLE_BUFFERING
		if (VertexFactory)
		{
			BeginReleaseResource(VertexFactory);
		}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	}

	/** The frame source data for this particle system.  This is everything needed to represent this
	    this particle system frame.  It does not include any transient rendering thread data.  Also, for
		non-simulating 'replay' particle systems, this data may have come straight from disk! */
	FDynamicSpriteEmitterReplayData Source;


	/**	The sprite particle data.										*/
	INT							PrimitiveCount;

	TArray<FParticleOrder>		ParticleOrder;

	FParticleVertexFactory*		VertexFactory;		// RENDER-THREAD USAGE ONLY!!!

};



/** Source data for SubUV emitters */
struct FDynamicSubUVEmitterReplayData
	: public FDynamicSpriteEmitterReplayDataBase
{
	INT								SubUVDataOffset;
	INT								SubImages_Horizontal;
	INT								SubImages_Vertical;
	UBOOL							bDirectUV;


	/** Constructor */
	FDynamicSubUVEmitterReplayData()
		: SubUVDataOffset( 0 ),
		  SubImages_Horizontal( 0 ),
		  SubImages_Vertical( 0 ),
		  bDirectUV( FALSE )
	{
	}


	/** Serialization */
	virtual void Serialize( FArchive& Ar )
	{
		// Call parent implementation
		FDynamicSpriteEmitterReplayDataBase::Serialize( Ar );

		Ar << SubUVDataOffset;
		Ar << SubImages_Horizontal;
		Ar << SubImages_Vertical;
		Ar << bDirectUV;
	}

};



/** Dynamic emitter data for SubUV emitters */
struct FDynamicSubUVEmitterData : public FDynamicSpriteEmitterDataBase
{
	FDynamicSubUVEmitterData()
		: PrimitiveCount( 0 ),
		  ParticleOrder(),
		  VertexFactory( NULL )
	{
	}

	virtual ~FDynamicSubUVEmitterData()
	{
		if(VertexFactory)
		{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
			VertexFactory->ReleaseResource();
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
			delete VertexFactory;
		}
	}

	/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
	void Init( UBOOL bInSelected );

	virtual UBOOL GetVertexAndIndexData(void* VertexData, void* FillIndexData, TArray<FParticleOrder>* ParticleOrder);

	// Render thread only draw call
	virtual void Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const
	{
		return Source;
	}

	/** Release the resource for the data */
	virtual void ReleaseResource()
	{
#if PARTICLES_USE_DOUBLE_BUFFERING
		if (VertexFactory)
		{
			BeginReleaseResource(VertexFactory);
		}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	}

	/** The frame source data for this particle system.  This is everything needed to represent this
	    this particle system frame.  It does not include any transient rendering thread data.  Also, for
		non-simulating 'replay' particle systems, this data may have come straight from disk! */
	FDynamicSubUVEmitterReplayData Source;


	/**	The sprite particle data.										*/
	INT								PrimitiveCount;

	TArray<FParticleOrder>			ParticleOrder;

	FParticleSubUVVertexFactory*	VertexFactory;		// RENDER-THREAD USAGE ONLY!!!
};

/** */
class UStaticMesh;
class UMaterialInstanceConstant;

// The resource used to render a UMaterialInstanceConstant.
class FMeshEmitterMaterialInstanceResource : public FMaterialRenderProxy
{
public:
	FMeshEmitterMaterialInstanceResource() : 
	  FMaterialRenderProxy()
		  , Parent(NULL)
	  {
	  }

	  FMeshEmitterMaterialInstanceResource(FMaterialRenderProxy* InParent) : 
	  FMaterialRenderProxy()
		  , Parent(InParent)
	  {
	  }

	  virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue, const FMaterialRenderContext& Context) const
	  {
		  if (ParameterName == NAME_MeshEmitterVertexColor)
		  {
			  *OutValue = Param_MeshEmitterVertexColor;
			  return TRUE;
		  }
		  else
			  if (ParameterName == NAME_TextureOffsetParameter)
			  { 
				  *OutValue = Param_TextureOffsetParameter;
				  return TRUE;
			  }
			  else
				  if (ParameterName == NAME_TextureScaleParameter)
				  {
					  *OutValue = Param_TextureScaleParameter;
					  return TRUE;
				  }

				  if (Parent == NULL)
				  {
					  return FALSE;
				  }

				  return Parent->GetVectorValue(ParameterName, OutValue, Context);
	  }

	  UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
	  {
		  return Parent->GetScalarValue(ParameterName, OutValue, Context);
	  }

	  UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
	  {
		  return Parent->GetTextureValue(ParameterName, OutValue, Context);
	  }

	  virtual const FMaterial* GetMaterial() const
	  {
		  return Parent->GetMaterial();
	  }

	  FMaterialRenderProxy* Parent;
	  FLinearColor Param_MeshEmitterVertexColor;
	  FLinearColor Param_TextureOffsetParameter;
	  FLinearColor Param_TextureScaleParameter;
};



/** Source data for Mesh emitters */
struct FDynamicMeshEmitterReplayData
	: public FDynamicSpriteEmitterReplayDataBase
{
	INT					SubUVInterpMethod;
	INT					SubUVDataOffset;
	INT					SubImages_Horizontal;
	INT					SubImages_Vertical;
	UBOOL				bScaleUV;
	INT					MeshRotationOffset;
	BYTE				MeshAlignment;
	UBOOL				bMeshRotationActive;
	FVector				LockedAxis;


	/** Constructor */
	FDynamicMeshEmitterReplayData()
		: SubUVInterpMethod( 0 ),
		  SubUVDataOffset( 0 ),
		  SubImages_Horizontal( 0 ),
		  SubImages_Vertical( 0 ),
		  bScaleUV( FALSE ),
		  MeshRotationOffset( 0 ),
		  MeshAlignment( 0 ),
		  bMeshRotationActive( FALSE ),
		  LockedAxis( 0.0f, 0.0f, 0.0f )
	{
	}


	/** Serialization */
	virtual void Serialize( FArchive& Ar )
	{
		// Call parent implementation
		FDynamicSpriteEmitterReplayDataBase::Serialize( Ar );

		Ar << SubUVInterpMethod;
		Ar << SubUVDataOffset;
		Ar << SubImages_Horizontal;
		Ar << SubImages_Vertical;
		Ar << bScaleUV;
		Ar << MeshRotationOffset;
		Ar << MeshAlignment;
		Ar << bMeshRotationActive;
		Ar << LockedAxis;
	}

};



/** Dynamic emitter data for Mesh emitters */
struct FDynamicMeshEmitterData : public FDynamicSpriteEmitterDataBase
{
	FDynamicMeshEmitterData();

	virtual ~FDynamicMeshEmitterData();

	/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
	void Init( UBOOL bInSelected,
			   const FParticleMeshEmitterInstance* InEmitterInstance,
			   UStaticMesh* InStaticMesh,
			   const UStaticMeshComponent* InStaticMeshComponent,
			   UBOOL UseNxFluid );


	/**
	* @return TRUE if the particle system uses FMeshElement.UseDynamicData=TRUE
	*/
	virtual UBOOL HasDynamicMeshElementData()
	{
		// mesh emitters don't set any dynamic mesh element data
		return FALSE;
	}

	// Render thread only draw call
	virtual void Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);
	
	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	Proxy			The 'owner' particle system scene proxy
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber);

	/** Render using hardware instancing. */
	void RenderInstanced(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	/** Render NxFluid using hardware instancing, with pre-filled VB's */
	void RenderNxFluidInstanced(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	/** Render NxFluid without hardware instancing, with pre-filled VB's */
	void RenderNxFluidNonInstanced(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	/** Debug rendering for NxFluid */
	void RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses);
	
	/** Initialized the vertex factory for a specific number of instances. */
	void InitInstancedResources(UINT NumInstances);

	/** Information used by the proxy about a single LOD of the mesh. */
	class FLODInfo
	{
	public:

		/** Information about an element of a LOD. */
		struct FElementInfo
		{
			UMaterialInterface* MaterialInterface;
		};
		TArray<FElementInfo> Elements;

		FLODInfo(const UStaticMeshComponent* InStaticMeshComponent, const FParticleMeshEmitterInstance* MeshEmitInst, INT LODIndex, UBOOL bSelected);
		
		void Update(const UStaticMeshComponent* InStaticMeshComponent, const FParticleMeshEmitterInstance* MeshEmitInst, INT LODIndex, UBOOL bSelected);
	};
	
	struct FParticleInstancedMeshInstance
	{
		FVector Location;
		FVector XAxis;
		FVector YAxis;
		FVector ZAxis;
	};
	
	class FParticleInstancedMeshInstanceBuffer : public FVertexBuffer
	{
	public:

		/** Initialization constructor. */
		FParticleInstancedMeshInstanceBuffer(const FDynamicMeshEmitterData& InRenderResources):
			RenderResources(InRenderResources)
		{}

		// FRenderResource interface.
		virtual void ReleaseDynamicRHI()
		{
			VertexBufferRHI.SafeRelease();
		}

		virtual FString GetFriendlyName() const { return TEXT("Instanced Particle Mesh Instances"); }

		void* CreateAndLockInstances(const UINT NumInstances);
		void UnlockInstances();

	private:
		const FDynamicMeshEmitterData &RenderResources;
	};

	// This structure is filled by the render thread, and is used to pass
	// pointers to VB's to the game thread to fill with xform matrices.

#define F_INSTANCE_BUFFER_MAX 2

	struct FInstanceBufferDesc : public FRenderResource
	{
		FInstanceBufferDesc() : NextNumInstances(0), RefCount(0)
		{
			for (int i = 0; i < F_INSTANCE_BUFFER_MAX; i++)
			{
				NextVertexBuffer[i] = NULL;
				NextInstanceBuffer[i] = NULL;
			}
		}
		~FInstanceBufferDesc()
		{
		}
		inline void Lock()
		{
			CritSect.Lock();
		}
		inline void UnLock()
		{
			CritSect.Unlock();
		}
		void IncRefCount();
		void DecRefCount();

		// A function to allocate a vertex buffer
		inline FVertexBufferRHIRef *AllocateVB(UINT InParticleCount)
		{
			UINT Size = InParticleCount * sizeof(FParticleInstancedMeshInstance);
			FVertexBufferRHIRef *VertexBufferRHI = ::new FVertexBufferRHIRef(RHICreateVertexBuffer(Size,NULL,RUF_Dynamic));
			return VertexBufferRHI;
		}

		inline BYTE *LockVB(FVertexBufferRHIRef* Buf, int Num)
		{
			return (BYTE*)RHILockVertexBuffer(*Buf, 0, Num * sizeof(FParticleInstancedMeshInstance), FALSE);
		}

		inline void UnlockVB(FVertexBufferRHIRef* Buf)
		{
			RHIUnlockVertexBuffer(*Buf);
		}
		inline void ReleaseVB(FVertexBufferRHIRef *Buf)
		{
			delete Buf;
		}

		void AllocateAll(UINT InParticleCount);
		void ReleaseAll();

		virtual void ReleaseDynamicRHI();
		virtual void InitDynamicRHI();

		static struct FInstanceBufferDesc *GetNextGarbage();
		static void EmptyGarbage();

		UBOOL GetNextBuffer(UINT NumInst, FVertexBufferRHIRef** vb, void** ptr);

		static TArray<struct FInstanceBufferDesc *>	Garbage;
		static FCriticalSection						GarbageCritSect;

		UINT					NextNumInstances;		// The size (# particles) of the NextInstanceBuffer's
		FVertexBufferRHIRef*	NextVertexBuffer[F_INSTANCE_BUFFER_MAX];	// The next (locked) VB's for the game thread to fill
		void*					NextInstanceBuffer[F_INSTANCE_BUFFER_MAX];	// The pointer to the locked data of the NextVertexBuffer's
		FCriticalSection		CritSect;
		UINT					RefCount;
	};

	// A function to update (refill) the FInstanceBufferDesc *FInstanceBufferDesc
	void UpdateInstBufDesc(UINT NewNumInst);

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const
	{
		return Source;
	}


	/** The frame source data for this particle system.  This is everything needed to represent this
	    this particle system frame.  It does not include any transient rendering thread data.  Also, for
		non-simulating 'replay' particle systems, this data may have come straight from disk! */
	FDynamicMeshEmitterReplayData Source;


	INT					LastFramePreRendered;

	UStaticMesh*		StaticMesh;
	TArray<FLODInfo>	LODs;
	
	/** The instanced rendering supporting material to use on the particles. */
	UMaterialInterface                        *InstancedMaterialInterface;
	
	/** The vertex buffer used to hold the instance data. */
	FParticleInstancedMeshInstanceBuffer      *InstanceBuffer;
	
	/** The vertex factory used to render the instanced meshes. */
	class FParticleInstancedMeshVertexFactory *InstancedVertexFactory;

	UBOOL				bUseNxFluid;		// True if we are using the optimization where the game thread fills VB's
	UBOOL				bInstBufIsVB;		// True if InstBuf is a FVertexBufferRHIRef, false if it is raw data
	void*				InstBuf;			// Pointer to instance buffer (maybe a VB, maybe not) - Rendering thread always deletes
	UINT				NumInst;			// Number of particles in InstBuf
	FInstanceBufferDesc* InstBufDesc;		// Pointer to descriptor that holds locked VB's for game thread to fill

	TArray<FMeshEmitterMaterialInstanceResource> MEMatInstRes;
};



/** Source data for Beam emitters */
struct FDynamicBeam2EmitterReplayData
	: public FDynamicSpriteEmitterReplayDataBase
{
	INT									VertexCount;
	INT									IndexCount;
	INT									IndexStride;

	TArray<INT>							TrianglesPerSheet;
	INT									UpVectorStepSize;

	// Offsets to particle data
	INT									BeamDataOffset;
	INT									InterpolatedPointsOffset;
	INT									NoiseRateOffset;
	INT									NoiseDeltaTimeOffset;
	INT									TargetNoisePointsOffset;
	INT									NextNoisePointsOffset;
	INT									TaperValuesOffset;
	INT									NoiseDistanceScaleOffset;

	UBOOL								bLowFreqNoise_Enabled;
	UBOOL								bHighFreqNoise_Enabled;
	UBOOL								bSmoothNoise_Enabled;
	UBOOL								bUseSource;
	UBOOL								bUseTarget;
	UBOOL								bTargetNoise;
	INT									Sheets;
	INT									Frequency;
	INT									NoiseTessellation;
	FLOAT								NoiseRangeScale;
	FLOAT								NoiseTangentStrength;
	FVector								NoiseSpeed;
	FLOAT								NoiseLockTime;
	FLOAT								NoiseLockRadius;
	FLOAT								NoiseTension;

	INT									TextureTile;
	FLOAT								TextureTileDistance;
	BYTE								TaperMethod;
	INT									InterpolationPoints;

	/** Debugging rendering flags												*/
	UBOOL								bRenderGeometry;
	UBOOL								bRenderDirectLine;
	UBOOL								bRenderLines;
	UBOOL								bRenderTessellation;

	/** Constructor */
	FDynamicBeam2EmitterReplayData()
		: VertexCount(0)
		, IndexCount(0)
		, IndexStride(0)
		, TrianglesPerSheet()
		, UpVectorStepSize(0)
		, BeamDataOffset(-1)
		, InterpolatedPointsOffset(-1)
		, NoiseRateOffset(-1)
		, NoiseDeltaTimeOffset(-1)
		, TargetNoisePointsOffset(-1)
		, NextNoisePointsOffset(-1)
		, TaperValuesOffset(-1)
		, NoiseDistanceScaleOffset(-1)
		, bLowFreqNoise_Enabled( FALSE )
		, bHighFreqNoise_Enabled( FALSE )
		, bSmoothNoise_Enabled( FALSE )
		, bUseSource( FALSE )
		, bUseTarget( FALSE )
		, bTargetNoise( FALSE )
		, Sheets(1)
		, Frequency(1)
		, NoiseTessellation(1)
		, NoiseRangeScale(1)
		, NoiseTangentStrength( 0.0f )
		, NoiseSpeed( 0.0f, 0.0f, 0.0f )
		, NoiseLockTime( 0.0f )
		, NoiseLockRadius( 0.0f )
		, NoiseTension( 0.0f )
		, TextureTile(0)
		, TextureTileDistance(0)
		, TaperMethod(0)
		, InterpolationPoints(0)
		, bRenderGeometry(TRUE)
		, bRenderDirectLine(FALSE)
		, bRenderLines(FALSE)
		, bRenderTessellation(FALSE)
	{
	}


	/** Serialization */
	virtual void Serialize( FArchive& Ar )
	{
		// Call parent implementation
		FDynamicSpriteEmitterReplayDataBase::Serialize( Ar );

		Ar << VertexCount;
		Ar << IndexCount;
		Ar << IndexStride;

		Ar << TrianglesPerSheet;
		Ar << UpVectorStepSize;
		Ar << BeamDataOffset;
		Ar << InterpolatedPointsOffset;
		Ar << NoiseRateOffset;
		Ar << NoiseDeltaTimeOffset;
		Ar << TargetNoisePointsOffset;
		Ar << NextNoisePointsOffset;
		Ar << TaperValuesOffset;
		Ar << NoiseDistanceScaleOffset;

		Ar << bLowFreqNoise_Enabled;
		Ar << bHighFreqNoise_Enabled;
		Ar << bSmoothNoise_Enabled;
		Ar << bUseSource;
		Ar << bUseTarget;
		Ar << bTargetNoise;
		Ar << Sheets;
		Ar << Frequency;
		Ar << NoiseTessellation;
		Ar << NoiseRangeScale;
		Ar << NoiseTangentStrength;
		Ar << NoiseSpeed;
		Ar << NoiseLockTime;
		Ar << NoiseLockRadius;
		Ar << NoiseTension;

		Ar << TextureTile;
		Ar << TextureTileDistance;
		Ar << TaperMethod;
		Ar << InterpolationPoints;

		Ar << bRenderGeometry;
		Ar << bRenderDirectLine;
		Ar << bRenderLines;
		Ar << bRenderTessellation;
	}

};



/** Dynamic emitter data for Beam emitters */
struct FDynamicBeam2EmitterData : public FDynamicSpriteEmitterDataBase 
{
	static const UINT MaxBeams = 2 * 1024;
	static const UINT MaxInterpolationPoints = 250;
	static const UINT MaxNoiseFrequency = 250;

	FDynamicBeam2EmitterData()
		: 
		  VertexCount(0)
		, VertexData(NULL)
		, IndexCount(0)
		, IndexData(NULL)
		, VertexFactory(NULL)
		, TrianglesToRender(0)
		, TrianglesToRender_Index(0)
		, LastFramePreRendered(-1)
	{
	}

	virtual ~FDynamicBeam2EmitterData()
	{
		appFree(VertexData);
		appFree(IndexData);
		if(VertexFactory)
		{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
			VertexFactory->ReleaseResource();
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
			delete VertexFactory;
		}
	}

	/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
	void Init( UBOOL bInSelected );

	// Render thread only draw call
	virtual void Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	Proxy			The 'owner' particle system scene proxy
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber);

	// Debugging functions
	virtual void RenderDirectLine(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);
	virtual void RenderLines(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);

	virtual void RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses);

	// Data fill functions
	INT FillIndexData(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	INT FillVertexData_NoNoise(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	INT FillData_Noise(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	INT FillData_InterpolatedNoise(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const
	{
		return Source;
	}

	/** Release the resource for the data */
	virtual void ReleaseResource()
	{
#if PARTICLES_USE_DOUBLE_BUFFERING
		if (VertexFactory)
		{
			BeginReleaseResource(VertexFactory);
		}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	}

	/** The frame source data for this particle system.  This is everything needed to represent this
	    this particle system frame.  It does not include any transient rendering thread data.  Also, for
		non-simulating 'replay' particle systems, this data may have come straight from disk! */
	FDynamicBeam2EmitterReplayData Source;

	/**	The sprite particle data.										*/
	INT									VertexCount;		// RENDER-THREAD USAGE ONLY!!!
	FParticleSpriteVertex*				VertexData;			// RENDER-THREAD USAGE ONLY!!!
	INT									IndexCount;			// RENDER-THREAD USAGE ONLY!!!
	void*								IndexData;			// RENDER-THREAD USAGE ONLY!!!
	FParticleBeamTrailVertexFactory*	VertexFactory;		// RENDER-THREAD USAGE ONLY!!!

	INT									TrianglesToRender;
	INT									TrianglesToRender_Index;
	INT									LastFramePreRendered;
};




/** Source data for Trail emitters */
struct FDynamicTrail2EmitterReplayData
	: public FDynamicSpriteEmitterReplayDataBase
{
	INT									PrimitiveCount;
	INT									VertexCount;
	INT									IndexCount;
	INT									IndexStride;

	// Payload offsets
	INT									TrailDataOffset;
	INT									TaperValuesOffset;
	INT									ParticleSourceOffset;

	INT									TrailCount;
	INT									Sheets;
	INT									TessFactor;
	INT									TessStrength;
	FLOAT								TessFactorDistance;

    TArray<FLOAT>						TrailSpawnTimes;
    TArray<FVector>						SourcePosition;
    TArray<FVector>						LastSourcePosition;
    TArray<FVector>						CurrentSourcePosition;
    TArray<FVector>						LastSpawnPosition;
    TArray<FVector>						LastSpawnTangent;
    TArray<FLOAT>						SourceDistanceTravelled;
    TArray<FVector>						SourceOffsets;


	/** Constructor */
	FDynamicTrail2EmitterReplayData()
		: PrimitiveCount(0)
		, VertexCount(0)
		, IndexCount(0)
		, IndexStride(0)
		, TrailDataOffset(-1)
		, TaperValuesOffset(-1)
		, ParticleSourceOffset(-1)
		, TrailCount(1)
		, Sheets(1)
		, TessFactor(1)
		, TessStrength(1)
		, TessFactorDistance(0.0f)
		, TrailSpawnTimes()
		, SourcePosition()
		, LastSourcePosition()
		, CurrentSourcePosition()
		, LastSpawnPosition()
		, LastSpawnTangent()
		, SourceDistanceTravelled()
		, SourceOffsets()
	{
	}


	/** Serialization */
	virtual void Serialize( FArchive& Ar )
	{
		// Call parent implementation
		FDynamicSpriteEmitterReplayDataBase::Serialize( Ar );

		Ar << PrimitiveCount;
		Ar << VertexCount;
		Ar << IndexCount;
		Ar << IndexStride;

		Ar << TrailDataOffset;
		Ar << TaperValuesOffset;
		Ar << ParticleSourceOffset;

		Ar << TrailCount;
		Ar << Sheets;
		Ar << TessFactor;
		Ar << TessStrength;
		Ar << TessFactorDistance;

		Ar << TrailSpawnTimes;
		Ar << SourcePosition;
		Ar << LastSourcePosition;
		Ar << CurrentSourcePosition;
		Ar << LastSpawnPosition;
		Ar << LastSpawnTangent;
		Ar << SourceDistanceTravelled;
		Ar << SourceOffsets;
	}

};



/** Dynamic emitter data for Trail emitters */
struct FDynamicTrail2EmitterData : public FDynamicSpriteEmitterDataBase 
{
	FDynamicTrail2EmitterData()
		: 
		  VertexCount(0)
		, VertexData(NULL)
		, IndexCount(0)
		, IndexData(NULL)
		, VertexFactory(NULL)
		, TriCountIndex(0)
		, LastFramePreRendered(-1)
	{
	}

	virtual ~FDynamicTrail2EmitterData()
	{
		appFree(VertexData);
		appFree(IndexData);
		if(VertexFactory)
		{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
			VertexFactory->ReleaseResource();
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
			delete VertexFactory;
		}
	}

	/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
	void Init( UBOOL bInSelected );

	// Render thread only draw call
	virtual void Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	Proxy			The 'owner' particle system scene proxy
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber);

	virtual void RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses);

	// Data fill functions
	INT FillIndexData(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);
	INT FillVertexData(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex);

	/** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const
	{
		return Source;
	}

	/** Release the resource for the data */
	virtual void ReleaseResource()
	{
#if PARTICLES_USE_DOUBLE_BUFFERING
		if (VertexFactory)
		{
			BeginReleaseResource(VertexFactory);
		}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	}

	/** The frame source data for this particle system.  This is everything needed to represent this
	    this particle system frame.  It does not include any transient rendering thread data.  Also, for
		non-simulating 'replay' particle systems, this data may have come straight from disk! */
	FDynamicTrail2EmitterReplayData Source;

	/**	The sprite particle data.										*/
	INT									VertexCount;		// RENDER-THREAD USAGE ONLY!!!
	FParticleSpriteVertex*				VertexData;			// RENDER-THREAD USAGE ONLY!!!
	INT									IndexCount;			// RENDER-THREAD USAGE ONLY!!!
	void*								IndexData;			// RENDER-THREAD USAGE ONLY!!!
	FParticleBeamTrailVertexFactory*	VertexFactory;		// RENDER-THREAD USAGE ONLY!!!
	INT									TriCountIndex;
	INT									LastFramePreRendered;

};

/*-----------------------------------------------------------------------------
 *	Particle dynamic data
 *	This is a copy of the particle system data needed to render the system in
 *	another thread.
 ----------------------------------------------------------------------------*/
class FParticleDynamicData
{
public:
	FParticleDynamicData()
		: DynamicEmitterDataArray()
		, bNeedsLODDistanceUpdate(FALSE)
	{
	}

	virtual ~FParticleDynamicData()
	{
		ClearEmitterDataArray();
	}

	void ClearEmitterDataArray()
	{
		for (INT Index = 0; Index < DynamicEmitterDataArray.Num(); Index++)
		{
			FDynamicEmitterDataBase* Data =	DynamicEmitterDataArray(Index);
			delete Data;
			DynamicEmitterDataArray(Index) = NULL;
		}
		DynamicEmitterDataArray.Empty();
	}

	/**
	 *	Called when a dynamic data is being deleted when double-buffering.
	 *	This function is responsible for releasing all non-render resource data.
	 *	The complete deletion must be deferred to ensure the render thread is not
	 *	utilizing any of the resources.
	 */
	void CleanupDataForDeferredDeletion()
	{
		for (INT Index = 0; Index < DynamicEmitterDataArray.Num(); Index++)
		{
			FDynamicEmitterDataBase* Data =	DynamicEmitterDataArray(Index);
			if (Data)
			{
				Data->ReleaseResource();
			}
		}
	}

	DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + DynamicEmitterDataArray.GetAllocatedSize() ); }

	// Variables
	TArray<FDynamicEmitterDataBase*>	DynamicEmitterDataArray;
	UBOOL bNeedsLODDistanceUpdate;
	volatile INT RenderFlag;
};

/**
 *	Used for double-buffering particle system dynamic data...
 *	The template member is used to determine what the dynamic data was initialized for.
 */
struct FParticleDynamicBufferedData
{
	UParticleSystem* Template;
	INT LODLevel;
	FParticleDynamicData* DynamicData;
	UBOOL bForceTemplateReset;

	FParticleDynamicBufferedData() :
		  Template(NULL)
		, LODLevel(-1)
		, DynamicData(NULL)
		, bForceTemplateReset(FALSE)
	{
	}

	~FParticleDynamicBufferedData()
	{
		delete DynamicData;
	}

	void UpdateTemplate(UParticleSystemComponent* InPSysComp);
	void ReleaseResources();
};

/** Used to defer release of rendering resources (vertex factories) */
struct FParticleDeferredReleaseResources
{
	/** The release fence that must pass */
	FRenderCommandFence* ReleaseFence;
	/** The dynamic data to delete after above fence clears */
	FParticleDynamicData* DynamicData;
	/** The emitter dynamic data to delete after above fence clears */
	FDynamicEmitterDataBase* EmitterDynamicData;

	FParticleDeferredReleaseResources() :
		  ReleaseFence(NULL)
		, DynamicData(NULL)
		, EmitterDynamicData(NULL)
	{
	}
};

/** Container to support multiple groups... */
struct FParticleDeferredReleases
{
	TArray<FParticleDeferredReleaseResources> DeferredReleases;
};

//
//	Scene Proxies
//

class FParticleSystemSceneProxy : public FPrimitiveSceneProxy
{
public:
	/** Initialization constructor. */
	FParticleSystemSceneProxy(const UParticleSystemComponent* Component);
	virtual ~FParticleSystemSceneProxy();

	// FPrimitiveSceneProxy interface.
	
	/** 
	 * Draw the scene proxy as a dynamic element
	 *
	 * @param	PDI - draw interface to render to
	 * @param	View - current view
	 * @param	DPGIndex - current depth priority 
	 * @param	Flags - optional set of flags from EDrawDynamicElementFlags
	 */
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	 *	Helper function for determining the LOD distance for a given view.
	 *
	 *	@param	View			The view of interest.
	 *	@param	FrameNumber		The frame number being rendered.
	 */
	void ProcessPreRenderView(const FSceneView* View, INT FrameNumber);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber);

	/**
	 *	Called when the rendering thread adds the proxy to the scene.
	 *	This function allows for generating renderer-side resources.
	 */
	virtual UBOOL CreateRenderThreadResources();

	/**
	 *	Called when the rendering thread removes the dynamic data from the scene.
	 */
	virtual UBOOL ReleaseRenderThreadResources();

	void UpdateData(FParticleDynamicData* NewDynamicData);
	void UpdateData_RenderThread(FParticleDynamicData* NewDynamicData);
	void UpdateViewRelevance(FMaterialViewRelevance& NewViewRelevance);
	void UpdateViewRelevance_RenderThread(FMaterialViewRelevance& NewViewRelevance);

	void DummyFunction()
	{
		// This is for debugging purposes!
		INT Dummy = 0;
	}

	FParticleDynamicData* GetDynamicData()
	{
		return DynamicData;
	}

	FParticleDynamicData* GetLastDynamicData()
	{
		return LastDynamicData;
	}

	void SetLastDynamicData(FParticleDynamicData* InLastDynamicData)
	{
		LastDynamicData  = InLastDynamicData;
	}

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocPSSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const 
	{ 
		DWORD AdditionalSize = FPrimitiveSceneProxy::GetAllocatedSize();
		if( DynamicData )
		{
			AdditionalSize = DynamicData->GetMemoryFootprint();
		}

		return( AdditionalSize ); 
	}

	void DetermineLODDistance(const FSceneView* View, INT FrameNumber);

	// While this isn't good OO design, access to everything is made public.
	// This is to allow custom emitter instances to easily be written when extending the engine.

	FPrimitiveSceneInfo* GetPrimitiveSceneInfo() const	{	return PrimitiveSceneInfo;	}

	FMatrix& GetLocalToWorld()			{	return LocalToWorld;			}
	FMatrix GetWorldToLocal()			{	return LocalToWorld.Inverse();	}
	FLOAT GetLocalToWorldDeterminant()	{	return LocalToWorldDeterminant;	}
	AActor* GetOwner()					{	return Owner;					}
	UBOOL GetSelected()					{	return bSelected;				}
	FLOAT GetCullDistance()				{	return CullDistance;			}
	UBOOL GetCastShadow()				{	return bCastShadow;				}
	const FMaterialViewRelevance& GetMaterialViewRelevance() const
	{
		return MaterialViewRelevance;
	}
	FLOAT GetPendingLODDistance()		{	return PendingLODDistance;		}
	FVector GetLODOrigin()				{	return LODOrigin;				}
	UBOOL GetNearClippingPlane(FPlane& OutNearClippingPlane) const;
	INT GetLODMethod()					{	return LODMethod;				}

	FColoredMaterialRenderProxy* GetSelectedWireframeMatInst()		{	return &SelectedWireframeMaterialInstance;		}
	FColoredMaterialRenderProxy* GetDeselectedWireframeMatInst()	{	return &DeselectedWireframeMaterialInstance;	}

	void GetAxisLockValues(FDynamicSpriteEmitterDataBase* DynamicData, FVector& CameraUp, FVector& CameraRight);

protected:
	AActor* Owner;

	UBOOL bSelected;

	FLOAT CullDistance;

	BITFIELD bCastShadow : 1;
	
	FMaterialViewRelevance MaterialViewRelevance;

	FParticleDynamicData* DynamicData;			// RENDER THREAD USAGE ONLY
	FParticleDynamicData* LastDynamicData;		// RENDER THREAD USAGE ONLY

	FColoredMaterialRenderProxy SelectedWireframeMaterialInstance;
	FColoredMaterialRenderProxy DeselectedWireframeMaterialInstance;

	INT LODMethod;
	FLOAT PendingLODDistance;

	FVector LODOrigin;
	BOOL LODHasNearClippingPlane;
	FPlane LODNearClippingPlane;

	INT LastFramePreRendered;

	friend struct FDynamicSpriteEmitterDataBase;
};

/*-----------------------------------------------------------------------------
 *	ParticleDataManager
 *	Handles the collection of ParticleSystemComponents that are to be 
 *	submitted to the rendering thread for display.
 ----------------------------------------------------------------------------*/
struct FParticleDataManager
{
protected:
	/** The particle system components that need to be sent to the rendering thread */
	TMap<UParticleSystemComponent*, UBOOL>	PSysComponents;

public:
	/**
	 *	Update the dynamic data for all particle system componets
	 */
	virtual void UpdateDynamicData();
	
	/**
	 *	Add a particle system component to the list.
	 *
	 *	@param		InPSysComp		The particle system component to add.
	 */
	void AddParticleSystemComponent(UParticleSystemComponent* InPSysComp);

	/**
	 *	Remove a particle system component to the list.
	 *
	 *	@param		InPSysComp		The particle system component to remove.
	 */
	void RemoveParticleSystemComponent(UParticleSystemComponent* InPSysComp);

	/**
	 *	Clear all pending components from the queue.
	 */
	void Clear();
};

#endif
