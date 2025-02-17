#ifndef NX_DOUBLEBUFFER_NXD_SOFTBODY
#define NX_DOUBLEBUFFER_NXD_SOFTBODY

#include "NxdAllocateable.h"
#include "softbody/NxSoftBody.h"

class NxdScene;

class NxdSoftBody : public NxSoftBody, public NxdAllocateable
{
	friend class NxdScene;

public:
	// getter functions [ double buffering supported ]
	virtual NxReal					getAttachmentResponseCoefficient() const;
	virtual NxReal					getAttachmentTearFactor() const;
	virtual NxCompartment*			getCompartment() const;
	virtual NxReal					getCollisionResponseCoefficient() const;
	virtual NxReal					getDampingCoefficient() const;
	virtual NxReal					getDensity() const;
	virtual NxVec3					getExternalAcceleration() const;
	virtual NxU32					getFlags() const;
	virtual NxReal					getFriction() const;
	virtual NxReal					getFromFluidResponseCoefficient() const;
	virtual NxCollisionGroup		getGroup() const;
	virtual const NxGroupsMask		getGroupsMask() const;
	virtual NxU32					getNumberOfParticles();
	virtual NxReal					getParticleRadius() const;
	virtual NxVec3					getPosition(NxU32 vertexId) const;
	virtual void					getPositions(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual NxReal					getRelativeGridSpacing() const;
	virtual NxScene&				getScene() const;
    virtual NxReal					getSleepLinearVelocity() const;
	virtual	NxSoftBodyMesh*			getSoftBodyMesh() const;
	virtual NxU32					getSolverIterations() const;
	virtual NxReal					getStretchingStiffness() const;
	virtual NxReal					getTearFactor() const;
	virtual NxReal					getToFluidResponseCoefficient() const;
	virtual void					getValidBounds(NxBounds3& validBounds) const;
	virtual NxVec3					getVelocity(NxU32 vertexId) const;
	virtual void					getVelocities(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual NxReal					getVolumeStiffness() const;
	virtual void					getWorldBounds(NxBounds3& bounds) const;
	virtual	NxForceFieldMaterial	getForceFieldMaterial() const;

	// getter functions [ double buffering *NOT* supported yet ]
	virtual NxMeshData			getMeshData();
	virtual NxSoftBodySplitPairData	getSplitPairData();
	virtual const char*			getName() const;
	virtual NxReal				getMinAdhereVelocity() const;

	// setter functions  [ double buffering supported ]
	virtual void				setAttachmentResponseCoefficient(NxReal coefficient);
	virtual void				setAttachmentTearFactor(NxReal factor);
	virtual void				setCollisionResponseCoefficient(NxReal coefficient);
	virtual void				setDampingCoefficient(NxReal coefficient);
	virtual void				setExternalAcceleration(NxVec3 acceleration);
	virtual void				setFlags(NxU32 flags);
	virtual void				setFriction(NxReal friction);
	virtual void				setFromFluidResponseCoefficient(NxReal coefficient);
	virtual void				setGroup(NxCollisionGroup collisionGroup);
	virtual void				setGroupsMask(const NxGroupsMask& groupsMask);
	virtual void				setParticleRadius(NxReal particleRadius);
	virtual void				setPosition(const NxVec3& position, NxU32 vertexId);
	virtual void				setPositions(void* buffer, NxU32 byteStride = sizeof(NxVec3));
    virtual void				setSleepLinearVelocity(NxReal threshold);
	virtual void				setSolverIterations(NxU32 iterations);
	virtual void				setStretchingStiffness(NxReal stiffness);
	virtual void				setTearFactor(NxReal factor);
	virtual void				setToFluidResponseCoefficient(NxReal coefficient);
	virtual void				setValidBounds(const NxBounds3& validBounds);
	virtual void				setVelocity(const NxVec3& velocity, NxU32 vertexId);
	virtual void				setVelocities(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual void				setVolumeStiffness(NxReal stiffness);

	// setter functions [ double buffering *NOT* supported yet ]
	virtual void				setMeshData(NxMeshData& meshData);
	virtual void				setSplitPairData(NxSoftBodySplitPairData& splitPairData);
	virtual void				setName(const char* name);
	virtual void				setMinAdhereVelocity(NxReal velocity);
	virtual	void				setForceFieldMaterial(NxForceFieldMaterial);

	// attachments [ double buffering *NOT* supported yet ]
	virtual void				attachToCollidingShapes(NxU32 attachmentFlags);
	virtual void				attachToShape(const NxShape* shape, NxU32 attachmentFlags);
	virtual void				attachVertexToGlobalPosition(const NxU32 vertexId, const NxVec3& pos);
	virtual void				attachVertexToShape(NxU32 vertexId, const NxShape* shape, const NxVec3& localPos, NxU32 attachmentFlags);
	virtual void				detachFromShape(const NxShape* shape);
	virtual void				freeVertex(const NxU32 vertexId);

	// sleeping [ double buffering supported ]
	virtual bool				isSleeping() const;
	virtual void				putToSleep();
	virtual void				wakeUp(NxReal wakeCounterValue = NX_SLEEP_INTERVAL);

	// interaction [ double buffering supported ]
	virtual	void				addForceAtPos(const NxVec3& position, NxReal magnitude, NxReal radius, NxForceMode mode = NX_FORCE);
	virtual	void				addForceAtVertex(const NxVec3& force, NxU32 vertexId, NxForceMode mode = NX_FORCE);
	virtual bool				tearVertex(const NxU32 vertexId, const NxVec3& normal);

	// serialization [ double buffering *NOT* supported yet ]
	virtual void				getShapePointers(NxShape** shapePointers, NxU32* flags = 0);
	virtual NxU32				getStateByteSize();
	virtual void				loadStateFromStream(NxStream& stream);
	virtual NxU32				queryShapePointers();
	virtual void				saveStateToStream(NxStream& stream, bool permute = false);
	virtual	bool				saveToDesc(NxSoftBodyDesc& desc) const;
	virtual void				setShapePointers(NxShape** shapePointers, unsigned int numShapes);

	// queries [ double buffering *NOT* supported yet ]
	virtual	bool				overlapAABBTetrahedra(const NxBounds3& bounds, NxU32& nb, const NxU32*& indices) const;
	virtual bool				raycast(const NxRay& worldRay, NxVec3& hit, NxU32& vertexId);

	class SceneIndexer
	{
	public:
		static NxU32&			index(NxdSoftBody& c) { return c.mSceneIndex; }
	};

	class DirtyIndexer
	{
	public:
		static NxU32&			index(NxdSoftBody& c) { return c.mDirtyIndex; }
	};

private:
	// internal data structures
	struct BufferedState
	{
		enum DirtyFlags
		{
			DIRTY_NONE									= 0,
			DIRTY_SLEEPING								= (1<<0),
			DIRTY_ATTACHMENT_RESPONSE_COEFFICIENT		= (1<<1),
			DIRTY_ATTACHMENT_TEAR_FACTOR				= (1<<2),	
			DIRTY_COLLISION_RESPONSE_COEFFICIENT		= (1<<3),
			DIRTY_DAMPING_COEFFICIENT					= (1<<4),
			DIRTY_EXTERNAL_ACCELERATION					= (1<<5),
			DIRTY_FLAGS									= (1<<6),
			DIRTY_FRICTION								= (1<<7),
			DIRTY_FROM_FLUID_RESPONSE_COEFFICIENT		= (1<<8),
			DIRTY_GROUP									= (1<<9),
			DIRTY_GROUPS_MASK							= (1<<10),
			//DIRTY_MESH_DATA								= (1<<11),
			//DIRTY_NAME									= (1<<12),
			DIRTY_PARTICLE_RADIUS						= (1<<13),			
			DIRTY_POSITIONS								= (1<<14),			
			DIRTY_SLEEP_LINEAR_VELOCITY					= (1<<15),
			DIRTY_SOLVER_ITERATIONS						= (1<<16),
			DIRTY_STRETCHING_STIFFNESS					= (1<<17),
			DIRTY_TEAR_FACTOR							= (1<<18),			
			DIRTY_TO_FLUID_RESPONSE_COEFFICIENT			= (1<<19),
			DIRTY_VALID_BOUNDS							= (1<<20),
			DIRTY_VOLUME_STIFFNESS						= (1<<21),
			DIRTY_VELOCITIES							= (1<<22),
			DIRTY_MIN_ADHERE_VELOCITY					= (1<<23),

			DIRTY_ALL									= 0xffffffff,
		};

		// general
		NxU32					dirtyFlags;

		// getter/setter functions
		NxReal					attachmentResponseCoefficient;
		NxReal					attachmentTearFactor;
		NxReal					collisionResponseCoefficient;
		NxReal					dampingCoefficient;
		NxVec3					externalAcceleration;
		NxU32					flags;
		NxReal					friction;
		NxReal					fromFluidResponseCoefficient;
		NxCollisionGroup		group;
		NxGroupsMask			groupsMask;
		//NxMeshData				meshData;
		//const char*				name;
		NxReal					particleRadius;		
		//void*						positions;		
		NxReal					sleepLinearVelocity;
		NxU32					solverIterations;
		NxReal					stretchingStiffness;
		NxReal					tearFactor;
		NxReal					toFluidResponseCoefficient;
		NxBounds3				validBounds;
		NxReal					volumeStiffness;
		//void*						velocities;
		NxReal					minAdhereVelocity;

		// sleeping
		bool					sleeping;

		// interaction
	};

	union BufferedUpdate
	{
		enum Type
		{
			TYPE_NONE								= 0,

			// getter/setter functions
			TYPE_ATTACHMENT_RESPONSE_COEFFICIENT,
			TYPE_ATTACHMENT_TEAR_FACTOR,
			TYPE_COLLISION_RESPONSE_COEFFICIENT,
			TYPE_DAMPING_COEFFICIENT,
			TYPE_EXTERNAL_ACCELERATION,
			TYPE_FLAGS,
			TYPE_FRICTION,
			TYPE_FROM_FLUID_RESPONSE_COEFFICIENT,
			TYPE_GROUP,
			TYPE_GROUPS_MASK,
			// TYPE_MESH_DATA,
			// TYPE_NAME,
			TYPE_PARTICLE_RADIUS,
			TYPE_POSITION,			
			TYPE_SLEEP_LINEAR_VELOCITY,
			TYPE_SOLVER_ITERATIONS,
			TYPE_STRETCHING_STIFFNESS,
			TYPE_TEAR_FACTOR,
			TYPE_TO_FLUID_RESPONSE_COEFFICIENT,
			TYPE_VALID_BOUNDS,
			TYPE_VELOCITY,
			TYPE_MIN_ADHERE_VELOCITY,
			TYPE_VOLUME_STIFFNESS,
			// sleeping
			TYPE_PUT_TO_SLEEP,
			TYPE_WAKE_UP,

			// interaction
			TYPE_ADD_FORCE_AT_POS,
			TYPE_ADD_FORCE_AT_VERTEX,
			TYPE_TEAR_VERTEX,
		};

		struct Common
		{
			NxU32				type;
		} common;

		// getter/setter functions
		struct AttachmentResponseCoefficient
		{
			NxU32				type;
			NxReal				coefficient;
		} attachmentResponseCoefficient;

		struct AttachmentTearFactor
		{
			NxU32				type;
			NxReal				factor;
		} attachmentTearFactor;

		struct CollisionResponseCoefficient
		{
			NxU32				type;
			NxReal				coefficient;
		} collisionResponseCoefficient;

		struct DampingCoefficient
		{
			NxU32				type;
			NxReal				coefficient;
		} dampingCoefficient;

		struct ExternalAcceleration
		{
			NxU32				type;
			NxReal				acceleration[3];
		} externalAcceleration;

		struct Flags
		{
			NxU32				type;
			NxU32				flags;
		} flags;

		struct Friction
		{
			NxU32				type;
			NxReal				friction;
		} friction;

		struct FromFluidResponseCoefficient
		{
			NxU32				type;
			NxReal				coefficient;
		} fromFluidResponseCoefficient;

		struct Group
		{
			NxU32				type;
			NxCollisionGroup	collisionGroup;
		} group;

		struct GroupsMask
		{
			NxU32				type;
			NxU32				groupsMask[4];
		} groupsMask;

		struct ParticleRadius
		{
			NxU32				type;
			NxReal				particleRadius;
		} particleRadius;

		struct Position
		{
			NxU32				type;
			NxReal				position[3];
			NxU32				vertexId;
		} position;

		struct SleepLinearVelocity
		{
			NxU32				type;
			NxReal				threshold;
		} sleepLinearVelocity;

		struct SolverIterations
		{
			NxU32				type;
			NxU32				iterations;
		} solverIterations;

		struct StretchingStiffness
		{
			NxU32				type;
			NxReal				stiffness;
		} stretchingStiffness;

		struct TearFactor
		{
			NxU32				type;
			NxReal				factor;
		} tearFactor;

		struct ToFluidResponseCoefficient
		{
			NxU32				type;
			NxReal				coefficient;
		} toFluidResponseCoefficient;

		struct ValidBounds
		{
			NxU32				type;
			NxReal				validBounds[2][3];
		} validBounds;

		struct Velocity
		{
			NxU32				type;
			NxReal				velocity[3];
			NxU32				vertexId;
		} velocity;

		struct VolumeStiffness
		{
			NxU32				type;
			NxReal				stiffness;
		} volumeStiffness;

		// sleeping
		struct PutToSleep
		{
			NxU32				type;
		} putToSleep;

		struct WakeUp
		{
			NxU32				type;
			NxReal				wakeCounterValue;
		} wakeUp;

		// interaction
		struct AddForceAtPos
		{
			NxU32				type;
			NxReal				position[3];
			NxReal				magnitude;
			NxReal				radius;
			NxForceMode			mode;
		} addForceAtPos;

		struct AddForceAtVertex
		{
			NxU32				type;
			NxReal				force[3];
			NxU32				vertexId;
			NxForceMode			mode;
		} addForceAtVertex;

		struct TearVertex
		{
			NxU32				type;
			NxU32				vertexId;
			NxReal				normal[3];
		} tearVertex;

		struct MinAdhereVelocity
		{
			NxU32				type;
			NxReal				velocity;
		} minAdhereVelocity;
	};

	// create/release [ double buffering *NOT* supported yet ]
								NxdSoftBody(NxScene& scene, NxSoftBody* softbody);
	virtual						~NxdSoftBody();

	// double-buffering
	static NxdSoftBody*			getNxdSoftBody(NxSoftBody* c);

	void						checkWrite(const char* fn) const;
	void						commitChanges();
	bool						writable() const;

	NxSoftBody*					mNxSoftBody;
	NxdScene&					mNxdScene;
	NxU32						mSceneIndex;	
	NxU32						mDirtyIndex;

	BufferedState				mBufferedState;
	NxArray<BufferedUpdate>		mBufferedUpdates;

	NxVec3*						mBufferedPositions;
	NxVec3*						mBufferedVelocities;
	NxU32						mBufferedNumberOfParticles;
};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
