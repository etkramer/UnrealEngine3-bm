#ifndef NX_DOUBLEBUFFER_NXD_REVOLUTEJOINT
#define NX_DOUBLEBUFFER_NXD_REVOLUTEJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxdAllocateable.h"
#include "NxRevoluteJoint.h"
#include "NxdJointHelper.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdScene;

class NxdRevoluteJoint : public NxRevoluteJoint, public NxdAllocateable
{
public:

	DECLARE_JOINT_FORWARDING_METHODS
	
	NxJointType getType()			{ return NX_JOINT_REVOLUTE; }

	void loadFromDesc(const NxRevoluteJointDesc& desc);
	void saveToDesc(NxRevoluteJointDesc& desc);
	void setLimits(const NxJointLimitPairDesc & pair);
	bool getLimits(NxJointLimitPairDesc & pair);
	void setMotor(const NxMotorDesc &motorDesc);
	bool getMotor(NxMotorDesc &motorDesc);
	void setSpring(const NxSpringDesc &springDesc);
	bool getSpring(NxSpringDesc &springDesc);
	NxReal getAngle();
	NxReal getVelocity();
	void setFlags(NxU32 flags);
	NxU32 getFlags();
	void setProjectionMode(NxJointProjectionMode projectionMode);
	NxJointProjectionMode getProjectionMode();

private:
	friend class NxdJointShared;

	NxdRevoluteJoint(NxdScene &scene, NxRevoluteJointDesc &desc);
	~NxdRevoluteJoint() {}
	NxRevoluteJoint *getNxJoint()	{	return static_cast<NxRevoluteJoint *>(mShared.getNxJoint()); }
	NxdJointShared &getShared()				{	return mShared; }
	const NxdJointShared &getShared() const	{	return mShared; }

	void	commitChanges();

	NxdJointShared		mShared;
	NxRevoluteJointDesc mDesc;
	NxU32				mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_DESC = 1<<0
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxRevoluteJoint)

#endif
