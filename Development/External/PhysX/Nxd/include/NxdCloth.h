#ifndef NX_DOUBLEBUFFER_NXD_CLOTH
#define NX_DOUBLEBUFFER_NXD_CLOTH

#include "NxdAllocateable.h"
#include "cloth/NxCloth.h"

class NxdScene;

class NxdCloth : public NxCloth, public NxdAllocateable
{
	friend class NxdScene;

public:
	// getter functions [ double buffering supported ]
	virtual NxReal					getAttachmentResponseCoefficient() const;
	virtual NxReal					getAttachmentTearFactor() const;
	virtual NxReal					getBendingStiffness() const;
	virtual	NxClothMesh*			getClothMesh() const;
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
	virtual NxMeshData				getMeshData();
	virtual NxU32					getNumberOfParticles();
	virtual NxVec3					getPosition(NxU32 vertexId) const;
	virtual void					getPositions(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual NxReal					getPressure() const;
	virtual NxReal					getRelativeGridSpacing() const;
	virtual NxScene&				getScene() const;
    virtual NxReal					getSleepLinearVelocity() const;
	virtual NxU32					getSolverIterations() const;
	virtual NxReal					getStretchingStiffness() const;
	virtual NxReal					getTearFactor() const;
	virtual NxReal					getThickness() const;
	virtual NxReal					getToFluidResponseCoefficient() const;
	virtual void					getValidBounds(NxBounds3& validBounds) const;
	virtual NxVec3					getVelocity(NxU32 vertexId) const;
	virtual void					getVelocities(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual NxVec3					getWindAcceleration() const;
	virtual void					getWorldBounds(NxBounds3& bounds) const;
	virtual NxForceFieldMaterial	getForceFieldMaterial() const;

	// getter functions [ double buffering *NOT* supported yet ]
	virtual const char*			getName() const;
	virtual NxVec3				getVertexAttachmentPosition(NxU32 vertexId) const;
	virtual NxShape*			getVertexAttachmentShape(NxU32 vertexId) const;
	virtual NxClothVertexAttachmentStatus
		getVertexAttachmentStatus(NxU32 vertexId) const;

	virtual NxReal				getMinAdhereVelocity() const;
	virtual	NxU32			    getPPUTime() const;

	// setter functions  [ double buffering supported ]
	virtual void				setAttachmentResponseCoefficient(NxReal coefficient);
	virtual void				setAttachmentTearFactor(NxReal factor);
	virtual void				setBendingStiffness(NxReal stiffness);
	virtual void				setCollisionResponseCoefficient(NxReal coefficient);
	virtual void				setDampingCoefficient(NxReal coefficient);
	virtual void				setExternalAcceleration(NxVec3 acceleration);
	virtual void				setFlags(NxU32 flags);
	virtual void				setFriction(NxReal friction);
	virtual void				setFromFluidResponseCoefficient(NxReal coefficient);
	virtual void				setGroup(NxCollisionGroup collisionGroup);
	virtual void				setGroupsMask(const NxGroupsMask& groupsMask);
	virtual void				setPosition(const NxVec3& position, NxU32 vertexId);
	virtual void				setPositions(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual void				setPressure(NxReal pressure);
    virtual void				setSleepLinearVelocity(NxReal threshold);
	virtual void				setSolverIterations(NxU32 iterations);
	virtual void				setStretchingStiffness(NxReal stiffness);
	virtual void				setTearFactor(NxReal factor);
	virtual void				setThickness(NxReal thickness);
	virtual void				setToFluidResponseCoefficient(NxReal coefficient);
	virtual void				setValidBounds(const NxBounds3& validBounds);
	virtual void				setVelocity(const NxVec3& velocity, NxU32 vertexId);
	virtual void				setVelocities(void* buffer, NxU32 byteStride = sizeof(NxVec3));
	virtual void				setWindAcceleration(NxVec3 acceleration);

	// setter functions [ double buffering *NOT* supported yet ]
	virtual void				setMeshData(NxMeshData& meshData);
	virtual void				setName(const char* name);
	virtual void				setMinAdhereVelocity(NxReal velocity);
	virtual	void				setForceFieldMaterial(NxForceFieldMaterial);

	// attachments [ double buffering supported ]
	virtual void				attachVertexToGlobalPosition(const NxU32 vertexId, const NxVec3& pos);
	virtual void				freeVertex(const NxU32 vertexId);

	// attachments [ double buffering *NOT* supported yet ]
	virtual void				attachToCollidingShapes(NxU32 attachmentFlags);
	virtual void				attachToCore(NxActor* actor, NxReal impulseThreshold, NxReal penetrationDepth = 0.0f, NxReal maxDeformationDistance = 0.0f);
	virtual void				attachToShape(const NxShape* shape, NxU32 attachmentFlags);
	virtual void				attachVertexToShape(NxU32 vertexId, const NxShape* shape, const NxVec3& localPos, NxU32 attachmentFlags);
	virtual void				detachFromShape(const NxShape* shape);

	// sleeping [ double buffering supported ]
	virtual bool				isSleeping() const;
	virtual void				putToSleep();
	virtual void				wakeUp(NxReal wakeCounterValue = NX_SLEEP_INTERVAL);

	// interaction [ double buffering supported ]
	virtual	void				addDirectedForceAtPos(const NxVec3& position, const NxVec3& force, NxReal radius, NxForceMode mode = NX_FORCE);
	virtual	void				addForceAtPos(const NxVec3& position, NxReal magnitude, NxReal radius, NxForceMode mode = NX_FORCE);
	virtual	void				addForceAtVertex(const NxVec3& force, NxU32 vertexId, NxForceMode mode = NX_FORCE);
	virtual void				dominateVertex(NxU32 vertexId, NxReal expirationTime, NxReal dominanceWeight);
	virtual bool				tearVertex(const NxU32 vertexId, const NxVec3& normal);

	// serialization [ double buffering *NOT* supported yet ]
	virtual void				getShapePointers(NxShape** shapePointers, NxU32* flags = 0);
	virtual NxU32				getStateByteSize();
	virtual void				loadStateFromStream(NxStream& stream);
	virtual NxU32				queryShapePointers();
	virtual void				saveStateToStream(NxStream& stream, bool permute = false);
	virtual	bool				saveToDesc(NxClothDesc& desc) const;
	virtual void				setShapePointers(NxShape** shapePointers, unsigned int numShapes);

	// queries [ double buffering supported ]
	virtual	bool				overlapAABBTriangles(const NxBounds3& bounds, NxU32& nb, const NxU32*& indices) const;
	virtual bool				raycast(const NxRay& worldRay, NxVec3& hit, NxU32& vertexId);

	class SceneIndexer
	{
	public:
		static NxU32&			index(NxdCloth& c) { return c.mSceneIndex; }
	};

	class DirtyIndexer
	{
	public:
		static NxU32&			index(NxdCloth& c) { return c.mDirtyIndex; }
	};

private:
	// internal data structures
	struct BufferedState
	{
		enum DirtyFlags
		{
			DIRTY_NONE									= 0x00000000,
			DIRTY_SLEEPING								= 0x00000001,
			DIRTY_ATTACHMENT_RESPONSE_COEFFICIENT		= 0x00000002,
			DIRTY_ATTACHMENT_TEAR_FACTOR				= 0x00000004,
			DIRTY_BENDING_STIFFNESS						= 0x00000008,
			DIRTY_COLLISION_RESPONSE_COEFFICIENT		= 0x00000010,
			DIRTY_DAMPING_COEFFICIENT					= 0x00000020,
			DIRTY_EXTERNAL_ACCELERATION					= 0x00000040,
			DIRTY_FLAGS									= 0x00000080,
			DIRTY_FRICTION								= 0x00000100,
			DIRTY_FROM_FLUID_RESPONSE_COEFFICIENT		= 0x00000200,
			DIRTY_GROUP									= 0x00000400,
			DIRTY_GROUPS_MASK							= 0x00000800,
			//DIRTY_MESH_DATA								= 0x00001000,
			//DIRTY_NAME									= 0x00002000,
			DIRTY_POSITIONS								= 0x00004000,
			DIRTY_PRESSURE								= 0x00008000,
			DIRTY_SLEEP_LINEAR_VELOCITY					= 0x00010000,
			DIRTY_SOLVER_ITERATIONS						= 0x00020000,
			DIRTY_STRETCHING_STIFFNESS					= 0x00040000,
			DIRTY_TEAR_FACTOR							= 0x00080000,
			DIRTY_THICKNESS								= 0x00100000,
			DIRTY_TO_FLUID_RESPONSE_COEFFICIENT			= 0x00200000,
			DIRTY_VALID_BOUNDS							= 0x00400000,
			DIRTY_VELOCITIES							= 0x00800000,
			DIRTY_WIND_ACCELERATION						= 0x01000000,
			DIRTY_MIN_ADHERE_VELOCITY					= 0x02000000,

			DIRTY_ALL									= 0xffffffff,
		};

		// general
		NxU32					dirtyFlags;

		// getter/setter functions
		NxReal					attachmentResponseCoefficient;
		NxReal					attachmentTearFactor;
		NxReal					bendingStiffness;
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
		//void*						positions;
		NxReal					pressure;
		NxReal					sleepLinearVelocity;
		NxU32					solverIterations;
		NxReal					stretchingStiffness;
		NxReal					tearFactor;
		NxReal					thickness;
		NxReal					toFluidResponseCoefficient;
		NxBounds3				validBounds;
		//void*						velocities;
		NxVec3					windAcceleration;
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
			TYPE_BENDING_STIFFNESS,
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
			// TYPE_POSITIONS,
			TYPE_POSITION,
			TYPE_PRESSURE,
			TYPE_SLEEP_LINEAR_VELOCITY,
			TYPE_SOLVER_ITERATIONS,
			TYPE_STRETCHING_STIFFNESS,
			TYPE_TEAR_FACTOR,
			TYPE_THICKNESS,
			TYPE_TO_FLUID_RESPONSE_COEFFICIENT,
			TYPE_VALID_BOUNDS,
			TYPE_VELOCITY,
			TYPE_WIND_ACCELERATION,
			TYPE_MIN_ADHERE_VELOCITY,

			// attachments
			TYPE_ATTACH_VERTEX_TO_GLOBAL_POSITION,
			TYPE_FREE_VERTEX,

			// sleeping
			TYPE_PUT_TO_SLEEP,
			TYPE_WAKE_UP,

			// interaction
			TYPE_ADD_DIRECTED_FORCE_AT_POS,
			TYPE_ADD_FORCE_AT_POS,
			TYPE_ADD_FORCE_AT_VERTEX,
			TYPE_DOMINATE_VERTEX,
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

		struct BendingStiffness
		{
			NxU32				type;
			NxReal				stiffness;
		} bendingStiffness;

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

		struct Position
		{
			NxU32				type;
			NxReal				position[3];
			NxU32				vertexId;
		} position;

		struct Pressure
		{
			NxU32				type;
			NxReal				pressure;
		} pressure;

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

		struct Thickness
		{
			NxU32				type;
			NxReal				thickness;
		} thickness;

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

		// attachments
		struct AttachVertexToGlobalPosition
		{
			NxU32				type;
			NxU32				vertexId;
			NxReal				position[3];
		} attachVertexToGlobalPosition;

		struct FreeVertex
		{
			NxU32				type;
			NxU32				vertexId;
		} freeVertex;

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
		struct AddDirectedForceAtPos
		{
			NxU32				type;
			NxReal				position[3];
			NxReal				force[3];
			NxReal				radius;
			NxForceMode			mode;
		} addDirectedForceAtPos;

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

		struct DominateVertex
		{
			NxU32				type;
			NxU32				vertexId;
			NxReal				expirationTime;
			NxReal				dominanceWeight;
		} dominateVertex;

		struct TearVertex
		{
			NxU32				type;
			NxU32				vertexId;
			NxReal				normal[3];
		} tearVertex;

		struct WindAcceleration
		{
			NxU32				type;
			NxReal				acceleration[3];
		} windAcceleration;

		struct MinAdhereVelocity
		{
			NxU32				type;
			NxReal				velocity;
		} minAdhereVelocity;
	};

	// create/release [ double buffering *NOT* supported yet ]
								NxdCloth(NxScene& scene, NxCloth* cloth);
	virtual						~NxdCloth();

	// double-buffering
	static NxdCloth*			getNxdCloth(NxCloth* c);

	void						checkWrite(const char* fn) const;
	void						commitChanges();
	bool						writable() const;

	NxCloth*					mNxCloth;
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
