#ifndef NX_DOUBLEBUFFER_NXD_ACTOR
#define NX_DOUBLEBUFFER_NXD_ACTOR

#include "NxActor.h"
#include "NxdAllocateable.h"
#include "NxdObjArray.h"
#include "NxdCasts.h"

class NxdActorBuffer;
class NxdScene;
class NxdShapeShared;

class NxdActor : public NxActor, public NxdAllocateable
{
public:
	virtual   NxScene&        getScene() const;
	virtual   void            saveToDesc(NxActorDescBase& desc);
	virtual   void            setName(const char* name);
	virtual   const char*     getName() const;
	virtual   void            setGlobalPose(const NxMat34& mat);
	virtual   void            setGlobalPosition(const NxVec3& vec);
	virtual   void            setGlobalOrientation(const NxMat33& mat);
	virtual   void            setGlobalOrientationQuat(const NxQuat& mat);
	virtual   NxMat34         getGlobalPose() const;
	virtual   NxVec3          getGlobalPosition() const;
	virtual   NxMat33         getGlobalOrientation() const; 
	virtual   NxQuat          getGlobalOrientationQuat()const;
	virtual   void            moveGlobalPose(const NxMat34& mat);
	virtual   void            moveGlobalPosition(const NxVec3& vec);
	virtual   void            moveGlobalOrientation(const NxMat33& mat);
	virtual   void            moveGlobalOrientationQuat(const NxQuat& quat);
	virtual   NxShape*        createShape(const NxShapeDesc& desc);
	virtual   void            releaseShape(NxShape& shape);
	virtual   NxU32           getNbShapes() const;
	virtual   NxShape*const*  getShapes() const;
	virtual   void            setGroup(NxActorGroup actorGroup);
	virtual   NxActorGroup    getGroup() const;
	virtual	  void			  setDominanceGroup(NxDominanceGroup dominanceGroup);	
	virtual	  NxDominanceGroup	
							  getDominanceGroup() const;
	virtual	  void			  setContactReportThreshold(NxReal threshold);	
	virtual	  NxReal		  getContactReportThreshold() const;
	virtual	  void			  setContactReportFlags(NxU32 flags);
	virtual	  NxU32			  getContactReportFlags() const;

	virtual   void            raiseActorFlag(NxActorFlag actorFlag);
	virtual   void            clearActorFlag(NxActorFlag actorFlag);
	virtual   bool            readActorFlag(NxActorFlag actorFlag) const;
	virtual   bool            isDynamic() const;
	virtual   void            setCMassOffsetLocalPose(const NxMat34& mat);
	virtual   void            setCMassOffsetLocalPosition(const NxVec3& vec);
	virtual   void            setCMassOffsetLocalOrientation(const NxMat33& mat);
	virtual   void            setCMassOffsetGlobalPose(const NxMat34& mat);
	virtual   void            setCMassOffsetGlobalPosition(const NxVec3& vec);
	virtual   void            setCMassOffsetGlobalOrientation(const NxMat33& mat);
	virtual   void            setCMassGlobalPose(const NxMat34& mat);
	virtual   void            setCMassGlobalPosition(const NxVec3& vec);
	virtual   void            setCMassGlobalOrientation(const NxMat33& mat);
	virtual   NxMat34         getCMassLocalPose() const;
	virtual   NxVec3          getCMassLocalPosition() const; 
	virtual   NxMat33         getCMassLocalOrientation() const;
	virtual   NxMat34         getCMassGlobalPose() const;
	virtual   NxVec3          getCMassGlobalPosition() const;
	virtual   NxMat33         getCMassGlobalOrientation() const;
	virtual   void            setMass(NxReal mass);
	virtual   NxReal          getMass() const;
	virtual   void            setMassSpaceInertiaTensor(const NxVec3& m);
	virtual   NxVec3          getMassSpaceInertiaTensor() const;
	virtual   NxMat33         getGlobalInertiaTensor() const;
	virtual   NxMat33         getGlobalInertiaTensorInverse() const;
	virtual   bool            updateMassFromShapes(NxReal density, NxReal totalMass);
	virtual   void            setLinearDamping(NxReal linDamp);
	virtual   NxReal          getLinearDamping() const;
	virtual   void            setAngularDamping(NxReal angDamp);
	virtual   NxReal          getAngularDamping() const;
	virtual   void            setLinearVelocity(const NxVec3& linVel);
	virtual   void            setAngularVelocity(const NxVec3& angVel);
	virtual   NxVec3          getLinearVelocity() const;
	virtual   NxVec3          getAngularVelocity() const;
	virtual   void            setMaxAngularVelocity(NxReal maxAngVel);
	virtual   NxReal          getMaxAngularVelocity() const; 
	virtual   void            setCCDMotionThreshold(NxReal thresh);
	virtual   NxReal          getCCDMotionThreshold() const; 
	virtual   void            setLinearMomentum(const NxVec3& linMoment);
	virtual   void            setAngularMomentum(const NxVec3& angMoment);
	virtual   NxVec3          getLinearMomentum() const;
	virtual   NxVec3          getAngularMomentum() const;
	virtual   void            addForceAtPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode, bool wakeup);
	virtual   void            addForceAtLocalPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode, bool wakeup);
	virtual   void            addLocalForceAtPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode, bool wakeup);
	virtual   void            addLocalForceAtLocalPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode, bool wakeup);
	virtual   void            addForce(const NxVec3& force, NxForceMode mode, bool wakeup);
	virtual   void            addLocalForce(const NxVec3& force, NxForceMode mode, bool wakeup);
	virtual   void            addTorque(const NxVec3& torque, NxForceMode mode, bool wakeup);
	virtual   void            addLocalTorque(const NxVec3& torque, NxForceMode mode, bool wakeup);
	virtual   NxReal          computeKineticEnergy() const;
	virtual   NxVec3          getPointVelocity(const NxVec3& point) const;
	virtual   NxVec3          getLocalPointVelocity(const NxVec3& point)  const;
	virtual   bool            isGroupSleeping() const;
	virtual   bool            isSleeping() const;
	virtual   NxReal          getSleepLinearVelocity() const;
	virtual   void            setSleepLinearVelocity(NxReal threshold);
	virtual   NxReal          getSleepAngularVelocity() const;
	virtual   void            setSleepAngularVelocity(NxReal threshold);
	virtual   NxReal          getSleepEnergyThreshold() const;
	virtual   void            setSleepEnergyThreshold(NxReal threshold);
	virtual   void            wakeUp(NxReal wakeCounterValue);
	virtual   void            putToSleep();
	virtual   void            raiseBodyFlag(NxBodyFlag bodyFlag);
	virtual   void            clearBodyFlag(NxBodyFlag bodyFlag);
	virtual   bool            readBodyFlag(NxBodyFlag bodyFlag) const;
	virtual   bool            saveBodyToDesc(NxBodyDesc& bodyDesc);
	virtual   void            setSolverIterationCount(NxU32 iterCount);
	virtual   NxU32           getSolverIterationCount() const;
	virtual   void			  resetUserActorPairFiltering();
#if NX_SUPPORT_SWEEP_API
	virtual   NxU32           linearSweep(const NxVec3& motion, NxU32 flags, void* userData, NxU32 nbShapes, NxSweepQueryHit* shapes, NxUserEntityReport<NxSweepQueryHit>* callback, const NxSweepCache* sweepCache);
#endif
	virtual   NxCompartment*		getCompartment() const;
	virtual   NxForceFieldMaterial	getForceFieldMaterial() const;
	virtual   void					setForceFieldMaterial(NxForceFieldMaterial);

	bool			isAwaitingDelete()	const;
	NxActor *		getNxActor()		const	{ return mNxActor; }
	
private:
	friend class NxdScene;
	friend class NxdActorBuffer;
	friend class NxdJoint;
	friend class NxdShapeShared;

	class SceneIndexer
	{
	public:
		static NxU32 &index(NxdActor &a)		{	return a.mSceneIndex;		}
	};

	class DirtyIndexer
	{
	public:
		static NxU32 &index(NxdActor &a)		{	return a.mDirtyIndex;		}
	};



	NxdActor(NxScene &scene, const NxActorDescBase &desc, NxScene &massPropsHackScene);
	virtual ~NxdActor(void);

	// returns false if change was buffered and creation failed

	NxdScene &				getNxdScene()		const	{ return mNxdScene; }

	bool					instance();
	bool					commitChanges();
	void					markShapeChanged();

	NxdActorBuffer *		buffer()							const;
	bool					instanced()							const;
	bool					writable()							const;
	bool					isInstanced()						const;
	void					checkWrite(const char *)			const;
	void					checkInstancedWrite(const char *)	const;
	void					checkInstanced(const char *)		const;


	NxMat33 				id33()								const;
	NxMat34 				id34()								const;

	NxActor *				mNxActor;
	NxdScene &				mNxdScene;
	mutable NxdActorBuffer	*		
							mBuffer;

	NxU32					mSceneIndex;
	NxU32					mDirtyIndex;

};

NXD_DECLARE_BUFFERED_CASTS(NxActor)

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
