#ifndef NX_DOUBLEBUFFER_NXD_SHAPE
#define NX_DOUBLEBUFFER_NXD_SHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxShape.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdActor;
class NxShapeDesc;

class NxdShapeShared: public NxdUnallocateable
{
public:
	NxActor&					getActor() const;

	void						setGroup(NxCollisionGroup collisionGroup);
	NxCollisionGroup			getGroup() const;

	void						getWorldBounds(NxBounds3& dest) const;	

	void						setFlag(NxShapeFlag flag, bool value);
	NX_BOOL						getFlag(NxShapeFlag flag) const;

	void						setLocalPose(const NxMat34& mat);
	void						setLocalPosition(const NxVec3& vec);

	void						setLocalOrientation(const NxMat33& mat);
	NxMat34						getLocalPose()					const;

	NxVec3						getLocalPosition()				const;
	NxMat33						getLocalOrientation()			const;

	void						setGlobalPose(const NxMat34& mat);
	void						setGlobalPosition(const NxVec3& vec);

	void						setGlobalOrientation(const NxMat33& mat);
	NxMat34						getGlobalPose()					const;

	NxVec3						getGlobalPosition()				const;
	NxMat33						getGlobalOrientation()			const;

	void						setMaterial(NxMaterialIndex matIndex);
	NxMaterialIndex				getMaterial() const;

	void						setSkinWidth(NxReal skinWidth);
	NxReal						getSkinWidth() const;

	NxShapeType					getType() const;

	void						setCCDSkeleton(NxCCDSkeleton *ccdSkel);
	NxCCDSkeleton *				getCCDSkeleton() const;

	void*						is(NxShapeType type);
	const void*					is(NxShapeType type) const;

	void						setName(const char* name);
	const char*					getName()			const;

	bool						raycast(const NxRay& worldRay, NxReal maxDist, NxU32 hintFlags, NxRaycastHit& hit, bool firstHit)	const;
	bool						checkOverlapSphere(const NxSphere& worldSphere)														const;
	bool						checkOverlapOBB(const NxBox& worldBox)																const;
	bool						checkOverlapAABB(const NxBounds3& worldBounds)														const;
	bool						checkOverlapCapsule(const NxCapsule& worldCapsule)														const;

	void						setGroupsMask(const NxGroupsMask& mask);
	const NxGroupsMask			getGroupsMask()	const;

	void						setNonInteractingCompartmentTypes(NxU32 compartmentTypes);
	NxU32						getNonInteractingCompartmentTypes() const;

	//////////////////////////// internal to Nxd layer

	static NxdShapeShared *	createFromDesc(NxdActor &, const NxShapeDesc &);
	static NxdShapeShared * getShared(NxShape *);
	static const NxdShapeShared *NxdShapeShared::getShared(const NxShape *s);

	void					destroy();

	NxdShapeShared(NxdActor &);

	bool					isAwaitingDelete()					const;
	NxShape *				getNxShape()						const	{	return mNxShape; }
	NxShapeDesc *			getDesc()							const	{	return mDesc;	 }
	NxShape *				getOwner()							const	{	return mOwner;	 }

	void					checkInstanced(const char *)		const;
	void					checkInstancedWrite(const char *)	const;
	void					checkWrite(const char *)			const;

	void					setPointers(NxShape *owner, NxShapeDesc *desc);
	void					setNxShape(NxShape *shape);

	void					commitChanges();
	void					markShapeChanged();

private:

	NxdActor &			mActor;
	NxShape *			mOwner;
	NxShape *			mNxShape;
	NxShapeDesc *		mDesc;

	NxI32				mRaisedFlags;
	NxI32				mClearedFlags;

	NxU32				mBufferedFlags;

	enum
	{
		BUFFERED_GROUPSMASK = 1<<0,
		BUFFERED_MATERIAL = 1<<1,
		BUFFERED_NONINTCOMPARTMENTTYPES = 1<<2,
	};
};


NXD_DECLARE_BUFFERED_CASTS(NxShape)
#endif
