#ifndef NX_DOUBLEBUFFER_NXD_DISTANCEJOINT
#define NX_DOUBLEBUFFER_NXD_DISTANCEJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxdAllocateable.h"
#include "NxDistanceJoint.h"
#include "NxDistanceJointDesc.h"
#include "NxdJointHelper.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdScene;

class NxdDistanceJoint : public NxDistanceJoint, public NxdAllocateable
{
public:
	DECLARE_JOINT_FORWARDING_METHODS
	
	NxJointType getType()			{ return NX_JOINT_DISTANCE;	}

	void loadFromDesc(const NxDistanceJointDesc& desc);
	void saveToDesc(NxDistanceJointDesc& desc);

private:
	friend class NxdJointShared;

	NxdDistanceJoint(NxdScene &scene, NxDistanceJointDesc &desc);
	~NxdDistanceJoint() {}
	NxDistanceJoint *getNxJoint()	{	return static_cast<NxDistanceJoint *>(mShared.getNxJoint()); }
	NxdJointShared &getShared()				{	return mShared; }
	const NxdJointShared &getShared() const	{	return mShared; }

	void	commitChanges();

	NxdJointShared		mShared;
	NxDistanceJointDesc	mDesc;
	NxU32				mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_DESC = 1<<0
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxDistanceJoint)

#endif
