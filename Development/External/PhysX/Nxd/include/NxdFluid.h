#ifndef NX_DOUBLEBUFFER_NXD_FLUID
#define NX_DOUBLEBUFFER_NXD_FLUID

#include "fluids/NxFluid.h"
#include "NxdAllocateable.h"
#include "NxdObjArray.h"
#include "NxdFluidEmitter.h"

class NxdScene;

#if NX_USE_FLUID_API

class NxdFluid : public NxFluid, public NxdAllocateable
{
public:

/**
Non - SDK functions
*/


	static	NxdFluid*					getNxdFluid(NxFluid*);
	void								markDirty(NxdFluidEmitter &);

/**
SDK functions
*/

	//async
	virtual		NxScene&				getScene() const;

	//sync
	virtual		NxFluidEmitter*			createEmitter(const NxFluidEmitterDesc& desc);

	//no
	virtual		void					releaseEmitter(NxFluidEmitter& emitter);

	//async
	virtual		NxU32					getNbEmitters()	const;

	//async
	virtual		NxFluidEmitter**		getEmitters() const;

	//async
	virtual		NxU32 					addParticles(const NxParticleData& pData, bool appendIds = false);
	
	//async
	virtual		void					removeAllParticles();
	
	//sync
	virtual		void					updateParticles(const NxParticleUpdateData&);

	//sync
	virtual		void 					setParticlesWriteData(const NxParticleData& pData);
	
	//async
	virtual		NxParticleData 			getParticlesWriteData()	const;
	
	//sync
	virtual		void 					setParticleDeletionIdWriteData(const NxParticleIdData& iData);
	
	//async
	virtual		NxParticleIdData 		getParticleDeletionIdWriteData() const;
	
	//sync
	virtual		void 					setParticleCreationIdWriteData(const NxParticleIdData& iData);
	
	//async
	virtual		NxParticleIdData 		getParticleCreationIdWriteData() const;

	//sync
	virtual		void 					setFluidPacketData(const NxFluidPacketData& pData);

	//async
	virtual		NxFluidPacketData 		getFluidPacketData() const;

	//async
	virtual		NxU32					getSimulationMethod() const;
	
	//async
	virtual		NxU32 					getNumReserveParticles() const;
	
	//async
	virtual		void 					setNumReserveParticles(NxU32);

	//no
	virtual		NxU32 					getCurrentParticleLimit() const;

	//no
	virtual		void 					setCurrentParticleLimit(NxU32);

	//async
	virtual		void					setSimulationMethod(NxU32 simMethod);
	
	//async
	virtual		NxReal					getStiffness() const;
	
	//async
	virtual		void 					setStiffness(NxReal stiff);
	
	//async
	virtual		NxReal					getViscosity() const;
	
	//async
	virtual		void 					setViscosity(NxReal visc);
	
	//async
	virtual		NxReal					getSurfaceTension() const;

	//async
	virtual		void 					setSurfaceTension(NxReal surfaceTension);

	//async
	virtual		NxReal					getDamping() const;
	
	//async
	virtual		void 					setDamping(NxReal damp);
	
	//async
	virtual		NxReal					getFadeInTime() const;
	
	//async
	virtual		void 					setFadeInTime(NxReal fadeIn);
	
	//async
	virtual		NxVec3					getExternalAcceleration() const;
	
	//async
	virtual		void 					setExternalAcceleration(const NxVec3&acceleration);

	//async
	virtual		NxPlane					getProjectionPlane() const;

	//async
	virtual		void 					setProjectionPlane(const NxPlane& plane);
	
	//async
	virtual		NxU32					getCollisionMethod() const;
	
	//async
	virtual		void					setCollisionMethod(NxU32 collMethod);
	
	//async
	virtual		NxReal					getRestitutionForStaticShapes() const;
	
	//async
	virtual		void 					setRestitutionForStaticShapes(NxReal rest);
	
	//async
	virtual		NxReal					getDynamicFrictionForStaticShapes() const;
	
	//async
	virtual		void 					setDynamicFrictionForStaticShapes(NxReal friction);
	
	//async
	virtual		NxReal					getStaticFrictionForStaticShapes() const;
	
	//async
	virtual		void 					setStaticFrictionForStaticShapes(NxReal friction);

	//async
	virtual		NxReal					getAttractionForStaticShapes() const;
	
	//async
	virtual		void 					setAttractionForStaticShapes(NxReal attraction);

	//async
	virtual		NxReal					getRestitutionForDynamicShapes() const;
	
	//async
	virtual		void 					setRestitutionForDynamicShapes(NxReal rest);
	
	//async
	virtual		NxReal					getDynamicFrictionForDynamicShapes() const;
	
	//async
	virtual		void 					setDynamicFrictionForDynamicShapes(NxReal friction);
	
	//async
	virtual		NxReal					getStaticFrictionForDynamicShapes() const;
	
	//async
	virtual		void 					setStaticFrictionForDynamicShapes(NxReal friction);

	//async
	virtual		NxReal					getAttractionForDynamicShapes() const;
	
	//async
	virtual		void 					setAttractionForDynamicShapes(NxReal attraction);
	
	//async
	virtual		void					setCollisionResponseCoefficient(NxReal coefficient);
	
	//async
	virtual		NxReal					getCollisionResponseCoefficient() const;
		
	//async
	virtual		void					setFlag(NxFluidFlag flag, bool val);
		
	//async
	virtual		NX_BOOL					getFlag(NxFluidFlag flag) const;
			
	//async
	virtual		NxU32 					getMaxParticles() const;
		
	//async
	virtual		NxReal					getKernelRadiusMultiplier() const;
		
	//async
	virtual		NxReal					getMotionLimitMultiplier() const;
		
	//async
	virtual		NxReal					getCollisionDistanceMultiplier() const;
		
	//async
	virtual		NxU32					getPacketSizeMultiplier() const;
		
	//async
	virtual		NxReal					getRestParticlesPerMeter() const;
		
	//async
	virtual		NxReal					getRestDensity() const;
		
	//async
	virtual		NxReal					getRestParticleDistance() const;
		
	//async
	virtual		NxReal					getParticleMass() const;
		
	//async
	virtual		void					setGroup(NxCollisionGroup collisionGroup);
		
	//async
	virtual		NxCollisionGroup		getGroup() const;
		
	//async
	virtual		void					setGroupsMask(const NxGroupsMask& groupsMask);
		
	//async
	virtual		const NxGroupsMask		getGroupsMask() const;
		
	//async
	virtual		void					getWorldBounds(NxBounds3& dest) const;
		
	//sync
	virtual		bool					loadFromDesc(const NxFluidDescBase& desc);
		
	//async
	virtual		bool					saveToDesc(NxFluidDescBase &desc) const;
	
	//no
	virtual		NxU32					getPPUTime() const;

	//async
	virtual		void					setName(const char* name);
		
	//async
	virtual		const char*				getName() const;
		
	//async
	virtual		NxCompartment*			getCompartment() const;

	// async
	virtual		NxForceFieldMaterial	getForceFieldMaterial() const;

	// sync
	virtual		void					setForceFieldMaterial(NxForceFieldMaterial);




	class DirtyIndexer
	{
	public:
		static NxU32 &index(NxdFluid &f)		{	return f.mDirtyIndex;		}
	};

	class SceneIndexer
	{
	public:
		static NxU32 &index(NxdFluid &f)		{	return f.mSceneIndex;		}
	};

	class ParticleUpdate
	{
	public:
		NxVec3	force;
		NxU32	flags;
		NxU32	id;
	};

private:

/**
Non - SDK functions
*/
	friend class NxdScene;
	friend class NxdFluidEmitter;

	NxdFluid(NxScene& scene, const NxFluidDescBase& desc);
	NxdFluid(NxScene& scene, NxFluid* fluid);
	virtual ~NxdFluid(void);

	NxFluid*				getNxFluid()		const	{ return mNxFluid; }
	NxdScene&				getNxdScene()		const	{ return mNxdScene; }

	void					commitChanges();
	void					commitFlaggedChanges();

	bool					writable()							const;
	void					checkWrite(const char *)			const;

	NxReal					getUpdateParticlesUnitMult(const NxParticleUpdateData& ud) const;


	NxFluid*				mNxFluid;
	NxdScene&				mNxdScene;

	NxU32					mSceneIndex;
	NxU32					mDirtyIndex;

	//Buffered data
	NxArray<ParticleUpdate> mParticlesUpdateBuffer;

	NxdObjArray<NxdFluidEmitter, NxdFluidEmitter::FluidIndexer>
							mEmitters;

	NxU32					mBufferFlags;
	NxFluidDescBase			mFluidDesc;
	NxI32					mRaisedFlags;
	NxI32					mClearedFlags;

	//ttp 7191;  there is no mCurrentParticleLimit in NxFluidDescBase, add here 
	NxU32					mCurrentParticleLimit;

	enum BufferedFlags
	{
		BUFFERED_STIFFNESS                   		= (1 << 0),
		BUFFERED_VISCOSITY                   		= (1 << 1),
		BUFFERED_DAMPING  							= (1 << 2),
		BUFFERED_FADE_IN_TIME						= (1 << 3),
		BUFFERED_EXTERNAL_ACCELERATION				= (1 << 4),
		BUFFERED_RESTITUTION_FOR_STATIC_SHAPES		= (1 << 5),
		BUFFERED_DYNAMIC_FRICTION_FOR_STATIC_SHAPES	= (1 << 6),
		BUFFERED_STATIC_FRICTION_FOR_STATIC_SHAPES	= (1 << 7),
		BUFFERED_ATTRACTION_FOR_STATIC_SHAPES		= (1 << 8),
		BUFFERED_RESTITUTION_FOR_DYNAMIC_SHAPES		= (1 << 9),
		BUFFERED_DYNAMIC_FRICTION_FOR_DYNAMIC_SHAPES= (1 << 10),
		BUFFERED_STATIC_FRICTION_FOR_DYNAMIC_SHAPES	= (1 << 11),
		BUFFERED_ATTRACTION_FOR_DYNAMIC_SHAPES		= (1 << 12),
		BUFFERED_COLLISION_RESPONSE_COEFFICIENT		= (1 << 13),
		BUFFERED_SIMULATION_METHOD            		= (1 << 14),
		BUFFERED_COLLISION_METHOD             		= (1 << 15),
		BUFFERED_COLLISION_GROUP					= (1 << 16),
		BUFFERED_GROUPS_MASK						= (1 << 17),
		/*
		BUFFERED_PARTICLES_WRITE_DATA				= (1 << 18),	not used
		BUFFERED_PARTICLE_DELETION_ID_WRITE_DATA	= (1 << 19),	not used	
		BUFFERED_PARTICLE_CREATION_ID_WRITE_DATA	= (1 << 20),	not used	
		BUFFERED_FLUID_PACKET_DATA					= (1 << 21),	not used	
		*/
		BUFFERED_FLAGS								= (1 << 22),
		BUFFERED_NUM_RESERVE_PARTICLES				= (1 << 23),
		BUFFERED_NAME								= (1 << 24),
		BUFFERED_PROJECTION_PLANE					= (1 << 25),
		BUFFERED_SURFACE_TENSION               		= (1 << 26),
		BUFFERED_CURRENT_PARTICLE_LIMIT             = (1 << 27),
	};	
};

#endif //NX_USE_FLUID_API

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
