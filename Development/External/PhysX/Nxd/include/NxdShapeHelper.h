#ifndef NX_DOUBLEBUFFER_NXD_SHAPEHELPER
#define NX_DOUBLEBUFFER_NXD_SHAPEHELPER

#include "NxdShapeShared.h"
#include "NxdScene.h"
#include "NxdError.h"

class NxBoxShape;
class NxPlaneShape;
class NxSphereShape;
class NxCapsuleShape;
class NxConvexShape;
class NxTriangleMeshShape;
class NxHeightFieldShape;


#define DECLARE_SHAPE_FORWARDING_METHODS						\
NxActor& getActor() const										\
{																\
	return mShared.getActor();									\
}																\
																\
void setGroup(NxCollisionGroup collisionGroup)					\
{																\
	mShared.setGroup(collisionGroup);							\
}																\
																\
NxCollisionGroup getGroup() const								\
{																\
	return mShared.getGroup();									\
}																\
																\
void getWorldBounds(NxBounds3& dest) const						\
{																\
	return mShared.getWorldBounds(dest);						\
}																\
																\
void setFlag(NxShapeFlag flag, bool value)						\
{																\
	mShared.setFlag(flag, value);								\
}																\
																\
NX_BOOL getFlag(NxShapeFlag flag) const							\
{																\
	return mShared.getFlag(flag);								\
}																\
																\
void setLocalPose(const NxMat34& mat)							\
{																\
	mShared.setLocalPose(mat);									\
}																\
																\
void setLocalPosition(const NxVec3& vec)						\
{																\
	mShared.setLocalPosition(vec);								\
}																\
																\
void setLocalOrientation(const NxMat33& mat)					\
{																\
	mShared.setLocalOrientation(mat);							\
}																\
																\
NxMat34 getLocalPose() const									\
{																\
	return mShared.getLocalPose();								\
}																\
																\
NxVec3 getLocalPosition() const									\
{																\
	return mShared.getLocalPosition();							\
}																\
																\
NxMat33 getLocalOrientation() const								\
{																\
	return mShared.getLocalOrientation();						\
}																\
																\
void setGlobalPose(const NxMat34& mat)							\
{																\
	mShared.setLocalPose(mat);									\
}																\
																\
void setGlobalPosition(const NxVec3& vec)						\
{																\
	mShared.setGlobalPosition(vec);								\
}																\
																\
void setGlobalOrientation(const NxMat33& mat)					\
{																\
	mShared.setGlobalOrientation(mat);							\
}																\
																\
NxMat34 getGlobalPose() const									\
{																\
	return mShared.getGlobalPose();								\
}																\
																\
NxVec3 getGlobalPosition() const								\
{																\
	return mShared.getGlobalPosition();							\
}																\
																\
NxMat33 getGlobalOrientation() const							\
{																\
	return mShared.getGlobalOrientation();						\
}																\
																\
void setMaterial(NxMaterialIndex matIndex)						\
{																\
	mShared.setMaterial(matIndex);								\
}																\
																\
NxMaterialIndex getMaterial() const								\
{																\
	return mShared.getMaterial();								\
}																\
																\
void setSkinWidth(NxReal skinWidth)								\
{																\
	mShared.setSkinWidth(skinWidth);							\
}																\
																\
NxReal getSkinWidth() const										\
{																\
	return mShared.getSkinWidth();								\
}																\
																\
void setCCDSkeleton(NxCCDSkeleton *ccdSkel)						\
{																\
	mShared.setCCDSkeleton(ccdSkel);							\
}																\
																\
NxCCDSkeleton * getCCDSkeleton() const							\
{																\
	return mShared.getCCDSkeleton();							\
}																\
																\
void setName(const char* name)									\
{																\
	mShared.setName(name);										\
}																\
																\
const char* getName() const										\
{																\
	return mShared.getName();									\
}																\
																\
bool raycast(const NxRay& worldRay, NxReal maxDist, NxU32 hintFlags, NxRaycastHit& hit, bool firstHit) const \
{																\
	return mShared.raycast(worldRay, maxDist, hintFlags, hit, firstHit); \
}																\
																\
bool checkOverlapSphere(const NxSphere& worldSphere)  const		\
{																\
	return mShared.checkOverlapSphere(worldSphere);				\
}																\
																\
bool checkOverlapOBB(const NxBox& worldBox)  const				\
{																\
return mShared.checkOverlapOBB(worldBox);						\
}																\
																\
bool checkOverlapAABB(const NxBounds3& worldBounds)  const		\
{																\
	return mShared.checkOverlapAABB(worldBounds);				\
}																\
																\
bool checkOverlapCapsule(const NxCapsule& worldCapsule)  const	\
{																\
	return mShared.checkOverlapCapsule(worldCapsule);			\
}																\
																\
void setGroupsMask(const NxGroupsMask& mask)					\
{																\
	mShared.setGroupsMask(mask);								\
}																\
																\
const NxGroupsMask getGroupsMask() const						\
{																\
	return mShared.getGroupsMask();								\
}																\
																\
void setNonInteractingCompartmentTypes(NxU32 compartmentTypes)	\
{																\
	mShared.setNonInteractingCompartmentTypes(compartmentTypes);\
}																\
																\
NxU32 getNonInteractingCompartmentTypes() const					\
{																\
	return mShared.getNonInteractingCompartmentTypes();			\
}																\
																\
void* is(NxShapeType type)										\
{																\
	return mShared.is(type);									\
}																\
																\
const void*	is(NxShapeType type) const							\
{																\
	return mShared.is(type);									\
}																\

#define DEFINE_SHAPE_CASTS(Type)														\
Nx##Type##Shape *NxdGetBuffered(Nx##Type##Shape *a)										\
{																						\
	return static_cast<Nx##Type##Shape *>(NxdGetBuffered(static_cast<NxShape *>(a)));	\
}																						\
																						\
const Nx##Type##Shape *NxdGetBuffered(const Nx##Type##Shape *a)							\
{																						\
	return static_cast<const Nx##Type##Shape *>(NxdGetBuffered(static_cast<const NxShape *>(a)));						\
}																						\
																						\
Nx##Type##Shape *NxdGetUnbuffered(Nx##Type##Shape *a)									\
{																						\
	return static_cast<Nx##Type##Shape *>(NxdGetUnbuffered(static_cast<NxShape *>(a)));							\
}																						\
																						\
const Nx##Type##Shape *NxdGetUnbuffered(const Nx##Type##Shape *a)						\
{																						\
	return static_cast<const Nx##Type##Shape *>(NxdGetUnbuffered(static_cast<const NxShape *>(a)));					\
}																						\

#endif
