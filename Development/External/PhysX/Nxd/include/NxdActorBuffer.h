#ifndef NX_DOUBLEBUFFERED_ACTORBUFFER
#define NX_DOUBLEBUFFERED_ACTORBUFFER

#include "NxActor.h"
#include "NxdAllocateable.h"
#include "NxdObjArray.h"

class NxdScene;
class NxdActor;
class NxdShapeShared;

class NxdActorBuffer: public NxdAllocateable
{
public:

	// if the actor being created and needs a proxy
	NxdActorBuffer(NxdActor &, const NxActorDescBase *desc, NxScene &massPropsHackScene);
	~NxdActorBuffer();

	NxActor *		create();
	void			commitChanges();

	void			setGlobalPose(const NxMat34 &);
	NxMat34			getGlobalPose()							const;
	void			setGlobalPosition(const NxVec3 &);
	NxVec3			getGlobalPosition()						const;
	void			setGlobalOrientation(const NxMat33 &);
	NxMat33			getGlobalOrientation()					const;

	void			moveGlobalPose(const NxMat34 &);
	void			moveGlobalPosition(const NxVec3 &);
	void			moveGlobalOrientation(const NxMat33 &);

	void			setLinearVelocity(const NxVec3 &);
	NxVec3			getLinearVelocity()						const;
	void			setAngularVelocity(const NxVec3 &);
	NxVec3			getAngularVelocity()					const;

	void			setMass(NxReal);
	NxReal			getMass()								const;
	void			setMassSpaceInertiaTensor(const NxVec3 &);
	NxVec3			getMassSpaceInertiaTensor() 			const;
	NxMat33			getGlobalInertiaTensor()				const;
	NxMat33			getGlobalInertiaTensorInverse()			const;

	void            setCMassOffsetLocalPose(const NxMat34& mat);
	void            setCMassOffsetLocalPosition(const NxVec3& vec);
	void            setCMassOffsetLocalOrientation(const NxMat33& mat);

	NxMat34         getCMassLocalPose() const;
	NxVec3          getCMassLocalPosition() const; 
	NxMat33         getCMassLocalOrientation() const;

	NxVec3			getPointVelocity(const NxVec3 &p)		const;
	NxVec3			getLocalPointVelocity(const NxVec3 &p)	const;

	bool			isSleeping()							const;
	void			wakeUp(NxReal);
	void			putToSleep();

	void			raiseActorFlag(NxActorFlag flag);
	void			clearActorFlag(NxActorFlag flag);
	bool			readActorFlag(NxActorFlag flag);

	void			raiseBodyFlag(NxBodyFlag flag);
	void			clearBodyFlag(NxBodyFlag flag);
	bool			readBodyFlag(NxBodyFlag flag);

	void			addForce(const NxVec3&, NxForceMode, bool);
	void			addLocalForce(const NxVec3&, NxForceMode, bool);
	void			addForceAtPos(const NxVec3&, const NxVec3&, NxForceMode, bool );
	void			addForceAtLocalPos(const NxVec3&, const NxVec3&, NxForceMode, bool);
	void			addLocalForceAtPos(const NxVec3&, const NxVec3&, NxForceMode, bool);
	void			addLocalForceAtLocalPos(const NxVec3&, const NxVec3&, NxForceMode, bool);
	void			addTorque(const NxVec3& torque, NxForceMode, bool);
	void			addLocalTorque(const NxVec3& torque, NxForceMode, bool);
	
	void			setGroup(NxActorGroup actorGroup);
	NxActorGroup	getGroup() const;

	void            setLinearDamping(NxReal linDamp);
	NxReal          getLinearDamping() const;
	void            setAngularDamping(NxReal angDamp);
	NxReal          getAngularDamping() const;

	void            setMaxAngularVelocity(NxReal maxAngVel);
	NxReal          getMaxAngularVelocity() const; 

	void            setDominanceGroup(NxDominanceGroup group);
	NxDominanceGroup getDominanceGroup() const;

	void            setContactReportThreshold(NxReal threshold);
	NxReal			getContactReportThreshold() const;

	NxU32			getNbShapes();
	NxShape *const *getShapes();

	// CAUTION: these are not buffered - they're just here so that consistency
	// can be maintained between the Nxd actor's list of shapes and the Nx actor's
	// list of shapes

	NxShape *		createShape(const NxShapeDesc &shape);
	void			releaseShape(NxShape &shape);

private:

	friend class NxdActor;

	bool			hasBody()							const;
	void			addSpatialForce(const NxVec3 &force, const NxVec3 &torque, 
									NxForceMode mode, bool wakeup);
	NxReal			getInvMass()						const;

	NxShapeDesc *	cloneDesc(NxShapeDesc *shape);

	void			markShapeChanged()			{	mBufferFlags |= BUFFERED_SHAPE_CHANGE; }


	NxdActor &						mActor;
	NxActorDesc						mActorDesc;
	NxBodyDesc						mBodyDesc;
	NxdSimpleArray<NxShape*>		mShapes;



	NxVec3					mLinAcc, mAngAcc;
	NxVec3					mLinDeltaV, mAngDeltaV;
	NxVec3					mLinStepDeltaV, mAngStepDeltaV;
	NxU32					mBufferFlags;

	NxI32					mRaisedActorFlags;
	NxI32					mClearedActorFlags;

	NxI32					mRaisedBodyFlags;
	NxI32					mClearedBodyFlags;

	bool					mPositionMoved;
	bool					mOrientationMoved;
	
	enum BufferedFlags
	{
		BUFFERED_POSITION			= 1<<0,
		BUFFERED_ORIENTATION		= 1<<1,
		BUFFERED_LINVEL				= 1<<2,
		BUFFERED_ANGVEL				= 1<<3,
		BUFFERED_MASS				= 1<<4,
		BUFFERED_INERTIA			= 1<<5,
		BUFFERED_SLEEP				= 1<<6,
		BUFFERED_ACCELERATION		= 1<<7,
		BUFFERED_DELTAV				= 1<<8,
		BUFFERED_STEP_DELTAV		= 1<<9,
		BUFFERED_CMASS_POSITION		= 1<<10,
		BUFFERED_CMASS_ORIENTATION	= 1<<11,
		BUFFERED_ACTOR_GROUP		= 1<<12,
		BUFFERED_SHAPE_CHANGE		= 1<<13,
		BUFFERED_LINEAR_DAMPING		= 1<<14,
		BUFFERED_ANGULAR_DAMPING	= 1<<15,
		BUFFERED_MAXANGULARVELOCITY	= 1<<16,
		BUFFERED_DOMINANCE          = 1<<17,
		BUFFERED_CONTACT_REPORT_THRESHOLD  = 1<<18
	};

};

#endif
