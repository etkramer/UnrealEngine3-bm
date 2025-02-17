#ifndef NX_DOUBLEBUFFER_NXD_FIXEDJOINT
#define NX_DOUBLEBUFFER_NXD_FIXEDJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxdAllocateable.h"
#include "NxFixedJoint.h"
#include "NxFixedJointDesc.h"
#include "NxdJointHelper.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdScene;

class NxdFixedJoint : public NxFixedJoint, public NxdAllocateable
{
public:
	DECLARE_JOINT_FORWARDING_METHODS
	
	NxJointType getType()			{ return NX_JOINT_FIXED;	}

	void loadFromDesc(const NxFixedJointDesc& desc);
	void saveToDesc(NxFixedJointDesc& desc);

private:
	friend class NxdJointShared;

	NxdFixedJoint(NxdScene &scene, NxFixedJointDesc &desc);
	~NxdFixedJoint() {}
	NxFixedJoint *getNxJoint()	{	return static_cast<NxFixedJoint *>(mShared.getNxJoint()); }
	NxdJointShared &getShared()				{	return mShared; }
	const NxdJointShared &getShared() const	{	return mShared; }

	void	commitChanges();

	NxdJointShared		mShared;
	NxFixedJointDesc	mDesc;
	NxU32				mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_DESC = 1<<0
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxFixedJoint)

#endif
