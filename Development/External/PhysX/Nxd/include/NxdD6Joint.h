#ifndef NX_DOUBLEBUFFER_NXD_D6JOINT
#define NX_DOUBLEBUFFER_NXD_D6JOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxdAllocateable.h"
#include "NxD6Joint.h"
#include "NxD6JointDesc.h"
#include "NxdJointHelper.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdScene;

class NxdD6Joint : public NxD6Joint, public NxdAllocateable
{
public:

	DECLARE_JOINT_FORWARDING_METHODS

	NxJointType getType()		{ return NX_JOINT_D6;	}

	void loadFromDesc(const NxD6JointDesc& desc); 
	void saveToDesc(NxD6JointDesc& desc);
 	void setDrivePosition(const NxVec3 &position);
 	void setDriveOrientation(const NxQuat &orientation); 
 	void setDriveLinearVelocity(const NxVec3 &linVel);
 	void setDriveAngularVelocity(const NxVec3 &angVel);

private:
	friend class NxdJointShared;


	NxD6Joint *getNxJoint()		{	return static_cast<NxD6Joint *>(mShared.getNxJoint()); }
	NxdD6Joint(NxdScene &scene, NxD6JointDesc &desc);
	~NxdD6Joint() {}

	NxdJointShared &getShared()				{	return mShared; }
	const NxdJointShared &getShared() const	{	return mShared; }

	void	commitChanges();


	NxdJointShared		mShared;
	NxD6JointDesc		mDesc;
	NxU32				mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_DESC			= 1<<0,
		BUFFERED_DRIVE_LINPOS	= 1<<1,
		BUFFERED_DRIVE_LINVEL	= 1<<2,
		BUFFERED_DRIVE_ANGPOS	= 1<<3,
		BUFFERED_DRIVE_ANGVEL	= 1<<4
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxD6Joint)

#endif
