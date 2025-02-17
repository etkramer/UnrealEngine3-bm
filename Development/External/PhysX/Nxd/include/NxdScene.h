#ifndef NX_DOUBLEBUFFER_NXD_SCENE
#define NX_DOUBLEBUFFER_NXD_SCENE

#include "NxScene.h"
#include "NxSceneDesc.h"
#include "NxdObjArray.h"
#include "NxdActor.h"
#include "NxdShapeShared.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCallbacks.h"
#include "NxdCasts.h"
#include "NxdPairFlagHash.h"
#include "NxdSpringAndDamperEffector.h"
#include "NxdMaterial.h"

#if NX_USE_FLUID_API
#include "NxdFluid.h"
#endif

#if NX_USE_CLOTH_API
#include "NxdCloth.h"
#endif

#if NX_USE_SOFTBODY_API
#include "NxdSoftBody.h"
#endif

class NxdDoubleBufferReport
{
public:
	virtual void actorInitializationFailed(NxActor *actor) = 0;
	virtual void jointInitializationFailed(NxJoint *joint) = 0;
	virtual void springAndDamperEffectorInitializationFailed(NxSpringAndDamperEffector *) = 0;
	virtual void materialInitializationFailed(NxMaterial *) = 0;
};


class NxdScene : public NxScene, public NxdAllocateable
{
public:
	static NxdScene *			create(NxPhysicsSDK &sdk, const NxSceneDesc &desc);
	static void					release(NxScene *scene);
	NxScene &					getNxScene() const	{	return mScene; }


	void						setDoubleBufferReport(NxdDoubleBufferReport *);
	NxdDoubleBufferReport *		getDoubleBufferReport();

	bool                        saveToDesc(NxSceneDesc& desc) const;
	NxU32						getFlags() const;

	NxSimulationType            getSimType() const;
	void *                      getInternal(void);
	void                        setGravity(const NxVec3& vec);
	void                        getGravity(NxVec3& vec);
	NxActor*                    createActor(const NxActorDescBase& desc);
	void                        releaseActor(NxActor& actor);
	NxJoint *                   createJoint(const NxJointDesc &jointDesc);
	void                        releaseJoint(NxJoint &joint);
	NxSpringAndDamperEffector*  createSpringAndDamperEffector(const NxSpringAndDamperEffectorDesc& springDesc);
	NxEffector*					createEffector(const NxEffectorDesc& desc);
	void                        releaseEffector(NxEffector& effector);
	NxForceField*				createForceField(const NxForceFieldDesc& forceFieldDesc);
	void						releaseForceField(NxForceField& forceField);
	NxForceFieldLinearKernel*	createForceFieldLinearKernel(const NxForceFieldLinearKernelDesc& kernelDesc);
	void						releaseForceFieldLinearKernel(NxForceFieldLinearKernel& kernel);
	NxU32						getNbForceFieldLinearKernels() const; 
	void						resetForceFieldLinearKernelsIterator(); 
	NxForceFieldLinearKernel*	getNextForceFieldLinearKernel(); 
	NxForceFieldShapeGroup*		createForceFieldShapeGroup(const NxForceFieldShapeGroupDesc& desc);
	void						releaseForceFieldShapeGroup(NxForceFieldShapeGroup &group);
	NxU32						getNbForceFieldShapeGroups() const;
	void						resetForceFieldShapeGroupsIterator(); 
	NxForceFieldShapeGroup*		getNextForceFieldShapeGroup(); 
	NxForceFieldVariety			createForceFieldVariety();
	NxForceFieldVariety			getHighestForceFieldVariety() const;
	void						releaseForceFieldVariety(NxForceFieldVariety var);
	NxForceFieldMaterial		createForceFieldMaterial();
	NxForceFieldMaterial		getHighestForceFieldMaterial() const;
	void						releaseForceFieldMaterial(NxForceFieldMaterial mat);
	NxReal						getForceFieldScale(NxForceFieldVariety var, NxForceFieldMaterial mat);
	void						setForceFieldScale(NxForceFieldVariety var, NxForceFieldMaterial mat, NxReal val);
	NxU32						getNbForceFields() const;
	NxForceField**				getForceFields();
	NxMaterial *                createMaterial(const NxMaterialDesc &matDesc); 
	void                        releaseMaterial(NxMaterial &material);
	NxCompartment *             createCompartment(const NxCompartmentDesc &compDesc);
	NxU32                       getNbCompartments() const;
	NxU32                       getCompartmentArray(NxCompartment ** userBuffer, NxU32 bufferSize, NxU32 & usersIterator) const;
	void                        setActorPairFlags(NxActor& actorA, NxActor& actorB, NxU32 nxContactPairFlag);
	NxU32                       getActorPairFlags(NxActor& actorA, NxActor& actorB) const;
	void                        setShapePairFlags(NxShape& shapeA, NxShape& shapeB, NxU32 nxContactPairFlag);
	NxU32                       getShapePairFlags(NxShape& shapeA, NxShape& shapeB) const;
	NxU32                       getNbPairs() const;
	NxU32                       getPairFlagArray(NxPairFlag* userArray, NxU32 numPairs) const;
	void                        setGroupCollisionFlag(NxCollisionGroup group1, NxCollisionGroup group2, bool enable);
	bool                        getGroupCollisionFlag(NxCollisionGroup group1, NxCollisionGroup group2) const;
	void						setDominanceGroupPair(NxDominanceGroup group1, NxDominanceGroup group2, NxConstraintDominance & dominance);
	NxConstraintDominance		getDominanceGroupPair(NxDominanceGroup group1, NxDominanceGroup group2) const;
	void                        setActorGroupPairFlags(NxActorGroup group1, NxActorGroup group2, NxU32 flags);
	NxU32                       getActorGroupPairFlags(NxActorGroup group1, NxActorGroup group2) const;
	NxU32                       getNbActorGroupPairs() const;
	NxU32                       getActorGroupPairArray(NxActorGroupPair * userBuffer, NxU32 bufferSize, NxU32 & userIterator) const;
	void                        setFilterOps(NxFilterOp op0, NxFilterOp op1, NxFilterOp op2);
	void                        setFilterBool(bool flag);
	void                        setFilterConstant0(const NxGroupsMask& mask);
	void                        setFilterConstant1(const NxGroupsMask& mask);
	void                        getFilterOps(NxFilterOp& op0, NxFilterOp& op1, NxFilterOp& op2)const;
	bool                        getFilterBool() const;
	NxGroupsMask                getFilterConstant0() const;
	NxGroupsMask                getFilterConstant1() const;
	NxU32                       getNbActors()   const;
	NxActor**                   getActors();
	NxActiveTransform*          getActiveTransforms(NxU32 &nbTransformsOut);
	NxU32                       getNbStaticShapes() const;
	NxU32                       getNbDynamicShapes() const;
	NxU32                       getTotalNbShapes() const;
	NxU32                       getNbJoints() const;
	void                        resetJointIterator();
	NxJoint *                   getNextJoint() ;
	NxU32                       getNbEffectors() const;
	void                        resetEffectorIterator();
	NxEffector *                getNextEffector();
	NxU32                       getBoundForIslandSize(NxActor& actor);
	NxU32                       getIslandArrayFromActor(NxActor& actor, NxActor** userBuffer, NxU32 bufferSize, NxU32& userIterator);
	NxU32                       getNbMaterials() const;
	NxU32                       getMaterialArray(NxMaterial ** userBuffer, NxU32 bufferSize, NxU32 & usersIterator);
	NxMaterialIndex             getHighestMaterialIndex() const;
	NxMaterial *                getMaterialFromIndex(NxMaterialIndex matIndex);
	void                        flushStream();
	void                        setTiming(NxReal maxTimestep=1.0f/60.0f, NxU32 maxIter=8, NxTimeStepMethod method=NX_TIMESTEP_FIXED);
	void                        getTiming(NxReal& maxTimestep, NxU32& maxIter, NxTimeStepMethod& method, NxU32* numSubSteps=NULL) const;
	const NxDebugRenderable*    getDebugRenderable();
	NxPhysicsSDK&               getPhysicsSDK();
	void                        getStats(NxSceneStats& stats) const;
	const NxSceneStats2 *		getStats2() const;
	void                        getLimits(NxSceneLimits& limits) const;
	void						setMaxCPUForLoadBalancing(NxReal cpuFraction);
	NxReal						getMaxCPUForLoadBalancing();
	void                        setUserNotify(NxUserNotify* callback);
	void                        setUserContactModify(NxUserContactModify* callback);
	NxUserNotify*               getUserNotify() const;

#if NX_USE_FLUID_API
	void						setFluidUserNotify(NxFluidUserNotify* callback);
	NxFluidUserNotify*			getFluidUserNotify() const;
#endif // NX_USE_FLUID_API

	NxUserContactModify*        getUserContactModify() const;
	void                        setUserTriggerReport(NxUserTriggerReport* callback);
	NxUserTriggerReport*        getUserTriggerReport() const;
	void                        setUserContactReport(NxUserContactReport* callback);
	NxUserContactReport*        getUserContactReport() const;
	void						setUserActorPairFiltering(NxUserActorPairFiltering* callback);
	NxUserActorPairFiltering*	getUserActorPairFiltering() const;
	bool                        raycastAnyBounds(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups, NxReal maxDist, const NxGroupsMask* groupsMask) const;
	bool						raycastAnyShape(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, const NxGroupsMask* groupsMask=NULL, NxShape** cache=NULL) const;
	NxU32                       raycastAllBounds(const NxRay& worldRay, NxUserRaycastReport& report, NxShapesType shapesType, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask* groupsMask) const;
	NxU32                       raycastAllShapes(const NxRay& worldRay, NxUserRaycastReport& report, NxShapesType shapesType, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask* groupsMask) const;
	NxShape*                    raycastClosestBounds(const NxRay& worldRay, NxShapesType shapeType, NxRaycastHit& hit, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask* groupsMask) const;
	NxShape*					raycastClosestShape(const NxRay& worldRay, NxShapesType shapeType, NxRaycastHit& hit, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask* groupsMask, NxShape** cache=NULL) const;
	NxU32                       overlapSphereShapes(const NxSphere& worldSphere, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask, bool accurateCollision);
	NxU32                       overlapAABBShapes(const NxBounds3& worldBounds, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask, bool accurateCollision);
	NxU32                       overlapOBBShapes(const NxBox& worldBox, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask, bool accurateCollision);
	NxU32                       overlapCapsuleShapes(const NxCapsule& worldCapsule, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask, bool accurateCollision);
#if NX_SUPPORT_SWEEP_API
	NxSweepCache*               createSweepCache();
	void                        releaseSweepCache(NxSweepCache* cache);
	NxU32                       linearOBBSweep(const NxBox& worldBox, const NxVec3& motion, NxU32 flags, void* userData, NxU32 nbShapes, NxSweepQueryHit* shapes, NxUserEntityReport<NxSweepQueryHit>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask);
	NxU32                       linearCapsuleSweep(const NxCapsule& worldCapsule, const NxVec3& motion, NxU32 flags, void* userData, NxU32 nbShapes, NxSweepQueryHit* shapes, NxUserEntityReport<NxSweepQueryHit>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask);
#endif
	NxU32                       cullShapes(NxU32 nbPlanes, const NxPlane* worldPlanes, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask);
	bool                        checkOverlapSphere(const NxSphere& worldSphere, NxShapesType shapeType, NxU32 activeGroups, const NxGroupsMask* groupsMask);
	bool                        checkOverlapAABB(const NxBounds3& worldBounds, NxShapesType shapeType, NxU32 activeGroups, const NxGroupsMask* groupsMask);
	bool                        checkOverlapOBB(const NxBox& worldBox, NxShapesType shapeType, NxU32 activeGroups, const NxGroupsMask* groupsMask);
	bool                        checkOverlapCapsule(const NxCapsule& worldCapsule, NxShapesType shapeType, NxU32 activeGroups, const NxGroupsMask* groupsMask);
#if NX_USE_FLUID_API
	NxFluid*                    createFluid(const NxFluidDescBase& fluidDesc);
	void                        releaseFluid(NxFluid& fluid);
	NxU32                       getNbFluids() const;
	NxFluid**                   getFluids();
	bool						cookFluidMeshHotspot(const NxBounds3& bounds, NxU32 packetSizeMultiplier, NxReal restParticlesPerMeter, NxReal kernelRadiusMultiplier, NxReal motionLimitMultiplier, NxReal collisionDistanceMultiplier, NxCompartment* compartment = NULL, bool forceStrictCookingFormat = false);
#endif
#if NX_USE_IMPLICIT_SCREEN_SURFACE_API 
	NxImplicitScreenMesh*       createImplicitScreenMesh(const NxImplicitScreenMeshDesc& surfaceDesc);
	void                        releaseImplicitScreenMesh(NxImplicitScreenMesh& implicitScreenMesh);
	NxU32                       getNbImplicitScreenMeshes() const;
	NxImplicitScreenMesh**      getImplicitScreenMeshes();
#endif
#if NX_USE_CLOTH_API
	// create/release [ double buffering *NOT* supported yet ]
	NxCloth*                    createCloth(const NxClothDesc& clothDesc);
	void                        releaseCloth(NxCloth& cloth);
	NxU32                       getNbCloths() const;
	NxCloth**                   getCloths();
	void						setClothUserNotify(NxClothUserNotify* callback);
	NxClothUserNotify*			getClothUserNotify() const;
#endif
#if NX_USE_SOFTBODY_API
	// create/release [ double buffering *NOT* supported yet ]
	NxSoftBody*					createSoftBody(const NxSoftBodyDesc& softBodyDesc);
	void						releaseSoftBody(NxSoftBody& softBody);
	NxU32						getNbSoftBodies() const;
	NxSoftBody**				getSoftBodies();
	void						setSoftBodyUserNotify(NxSoftBodyUserNotify* callback);
	NxSoftBodyUserNotify*		getSoftBodyUserNotify() const;
#endif


	bool						isWritable();
	bool                        isWritable() const;
	void                        simulate(NxReal elapsedTime);
	bool                        checkResults(NxSimulationStatus status, bool block);
	bool                        fetchResults(NxSimulationStatus status, bool block, NxU32 *errorState);
	void                        flushCaches();
	const NxProfileData*        readProfileData(bool clearData);
	NxThreadPollResult          pollForWork(NxThreadWait waitType);
	void                        resetPollForWork();
	NxThreadPollResult          pollForBackgroundWork(NxThreadWait waitType);
	void                        shutdownWorkerThreads();
	void                        lockQueries();
	void                        unlockQueries();
	NxSceneQuery*				createSceneQuery(const NxSceneQueryDesc& desc);
	bool						releaseSceneQuery(NxSceneQuery& query);

	void						setSolverBatchSize(NxU32 solverBatchSize);
	NxU32						getSolverBatchSize() const;


// internal to Nxd

	void								markDirty(NxdActor &);
	void								markDirty(NxdJointShared &);
	void								markDirty(NxdSpringAndDamperEffector &);
	void								markDirty(NxdMaterial &);

#if NX_USE_FLUID_API
	void								markDirty(NxdFluid &);
#endif
#if NX_USE_CLOTH_API
	void								markDirty(NxdCloth &);
#endif
#if NX_USE_SOFTBODY_API
	void								markDirty(NxdSoftBody &);
#endif
	void								scheduleJointForDeletion(NxJoint &joint);


private:
	friend bool NxdIsBuffered(const NxScene *s);

	NxdScene(NxPhysicsSDK &sdk, NxScene &scene);
	~NxdScene(void);

	void			commitUpdates(void);
	void			commitDeletes(void);
	void			checkWrite(const char *fn) const;


	NxPhysicsSDK &					mSDK;
	NxScene &						mScene;
	bool							mLocked;
	NxScene *						mMassPropsHackScene;

	NxdObjArray<NxdActor, NxdActor::SceneIndexer>
									mActors;
	NxdObjArray<NxdActor, NxdActor::DirtyIndexer>
									mDirtyActors;
	NxdSimpleArray<NxdActor*>		mDeadActors;


	NxdObjArray<NxdJointShared, NxdJointShared::SceneIndexer>
									mJoints;
	NxdObjArray<NxdJointShared,	NxdJointShared::DirtyIndexer>
									mDirtyJoints;
	NxdSimpleArray<NxdJointShared*>	mDeadJoints;
	NxU32							mJointIterator;


	NxdObjArray<NxdSpringAndDamperEffector, NxdSpringAndDamperEffector::SceneIndexer>
									mEffectors;
	NxdObjArray<NxdSpringAndDamperEffector,	NxdSpringAndDamperEffector::DirtyIndexer>
									mDirtyEffectors;
	NxU32							mEffectorIterator;
	NxdSimpleArray<NxdSpringAndDamperEffector*>	
									mDeadEffectors;

	NxdSimpleArray<NxdMaterial *>	mMaterials;
	NxdSimpleArray<NxMaterialIndex>	mMaterialFreeIndices;
	NxU32							mMaterialCount;
	NxdObjArray<NxdMaterial, NxdMaterial::DirtyIndexer>		
									mDirtyMaterials;
	NxdSimpleArray<NxdMaterial *>	mDeadMaterials;

	NxVec3							mGravity;
	NxU32							mSolverBatchSize;
	NxU32							mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_GRAVITY			= 1<<0,
		BUFFERED_SOLVER_BATCH_SIZE	= 1<<1
	};


	NxdPairFlagHash<NxdActor *>		mActorPairFlagHash;
	NxdPairFlagHash<NxdShapeShared *>	
									mShapePairFlagHash;

	NxdDoubleBufferReport *			mReport;
	NxdUserNotify					mUserNotify;
	NxdUserContactModify			mUserContactModify;
	NxdUserTriggerReport			mUserTriggerReport;
	NxdUserContactReport			mUserContactReport;
	NxdUserActorPairFiltering		mUserActorPairFiltering;


#if NX_USE_FLUID_API
	NxdObjArray<NxdFluid, NxdFluid::SceneIndexer>
								mFluids;
	NxdObjArray<NxdFluid, NxdFluid::DirtyIndexer>
								mDirtyFluids;
#endif
#if NX_USE_CLOTH_API
	NxdObjArray<NxdCloth, NxdCloth::SceneIndexer>
								mCloths;
	NxdObjArray<NxdCloth, NxdCloth::DirtyIndexer>
								mDirtyCloths;	
#endif
#if NX_USE_SOFTBODY_API
	NxdObjArray<NxdSoftBody, NxdSoftBody::SceneIndexer>
								mSoftBodies;
	NxdObjArray<NxdSoftBody, NxdSoftBody::DirtyIndexer>
								mDirtySoftBodies;	
#endif


};

NXD_DECLARE_BUFFERED_CASTS(NxScene)

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
