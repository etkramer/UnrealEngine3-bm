#ifndef NX_DOUBLEBUFFER_NXD_JOINTHELPER
#define NX_DOUBLEBUFFER_NXD_JOINTHELPER

#include "NxdJointShared.h"
#include "NxdError.h"

#define DECLARE_JOINT_FORWARDING_METHODS										\
void	getActors(NxActor** actor1, NxActor** actor2)							\
{																				\
    mShared.getActors(actor1,actor2);											\
}																				\
																				\
void	setGlobalAnchor(const NxVec3 &vec)										\
{																				\
	mShared.setGlobalAnchor(vec);												\
}																				\
																				\
void	setGlobalAxis(const NxVec3 &vec)										\
{																				\
	mShared.setGlobalAxis(vec);													\
}																				\
																				\
NxVec3	getGlobalAnchor()	const												\
{																				\
	return mShared.getGlobalAnchor();											\
}																				\
																				\
NxVec3	getGlobalAxis()	const													\
{																				\
	return mShared.getGlobalAxis();												\
}																				\
																				\
NxJointState getState()															\
{																				\
	return mShared.getState();													\
}																				\
void setBreakable(NxReal maxForce, NxReal maxTorque)							\
{																				\
	mShared.setBreakable(maxForce,maxTorque);									\
}																				\
																				\
void getBreakable(NxReal & maxForce, NxReal & maxTorque)						\
{																				\
	return mShared.getBreakable(maxForce,maxTorque);							\
}																				\
																				\
void setLimitPoint(const NxVec3 & point, bool pointIsOnActor2 = true)			\
{																				\
	mShared.setLimitPoint(point, pointIsOnActor2);								\
}																				\
																				\
bool getLimitPoint(NxVec3 & worldLimitPoint)									\
{																				\
	return mShared.getLimitPoint(worldLimitPoint);								\
}																				\
																				\
bool addLimitPlane(const NxVec3 & normal, const NxVec3 & pointInPlane, NxReal restitution = 0.0f)\
{																				\
	return mShared.addLimitPlane(normal,pointInPlane,restitution);				\
}																				\
																				\
void purgeLimitPlanes()															\
{																				\
	mShared.purgeLimitPlanes();													\
}																				\
																				\
void resetLimitPlaneIterator()													\
{																				\
	mShared.resetLimitPlaneIterator();											\
}																				\
																				\
bool hasMoreLimitPlanes()														\
{																				\
	return mShared.hasMoreLimitPlanes();										\
}																				\
																				\
bool getNextLimitPlane(NxVec3 & planeNormal, NxReal & planeD, NxReal * restitution = NULL)	\
{																				\
	return mShared.getNextLimitPlane(planeNormal,planeD,restitution);			\
}																				\
																				\
NxJointType  getType() const													\
{																				\
	return mShared.getType();													\
}																				\
																				\
void* is(NxJointType type)														\
{																				\
	return mShared.is(type);														\
}																				\
																				\
void setName(const char* name)													\
{																				\
	mShared.setName(name);														\
}																				\
																				\
const char*	getName() const														\
{																				\
	return mShared.getName();													\
}																				\
																				\
NxScene&		getScene()	const												\
{																				\
	return static_cast<NxScene &>(mShared.getScene());							\
}																				\


#define DEFINE_JOINT_CASTS(Type)																	\
Nx##Type##Joint *NxdGetBuffered(Nx##Type##Joint *a)													\
{																									\
	return static_cast<Nx##Type##Joint *>(NxdGetBuffered(static_cast<NxJoint *>(a)));				\
}																									\
																									\
const Nx##Type##Joint *NxdGetBuffered(const Nx##Type##Joint *a)										\
{																									\
	return static_cast<const Nx##Type##Joint *>(NxdGetBuffered(static_cast<const NxJoint *>(a)));	\
}																									\
																									\
Nx##Type##Joint *NxdGetUnbuffered(Nx##Type##Joint *a)												\
{																									\
	return static_cast<Nx##Type##Joint *>(NxdGetUnbuffered(static_cast<NxJoint *>(a)));				\
}																									\
																									\
const Nx##Type##Joint *NxdGetUnbuffered(const Nx##Type##Joint *a)									\
{																									\
	return static_cast<const Nx##Type##Joint *>(NxdGetUnbuffered(static_cast<const NxJoint *>(a)));	\
}																									\

#endif
