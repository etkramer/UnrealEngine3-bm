#ifndef NX_DOUBLEBUFFER_NXD_SPHERICALJOINT
#define NX_DOUBLEBUFFER_NXD_SPHERICALJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxdAllocateable.h"
#include "NxSphericalJoint.h"
#include "NxdJointHelper.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdScene;

class NxdSphericalJoint : public NxSphericalJoint, public NxdAllocateable
{
public:

	DECLARE_JOINT_FORWARDING_METHODS
	
	NxJointType getType()				{ return NX_JOINT_SPHERICAL; }

	void	loadFromDesc(const NxSphericalJointDesc &desc);
	void	saveToDesc(NxSphericalJointDesc &desc);
	void	setFlags(NxU32 flags);
	NxU32	getFlags();
	void	setProjectionMode(NxJointProjectionMode projectionMode);
	NxJointProjectionMode getProjectionMode();

private:
	friend class NxdJointShared;

	NxdSphericalJoint(NxdScene &scene, NxSphericalJointDesc &desc);
	~NxdSphericalJoint() {}
	NxSphericalJoint *getNxJoint()	{	return static_cast<NxSphericalJoint *>(mShared.getNxJoint()); }
	NxdJointShared &getShared()				{	return mShared; }
	const NxdJointShared &getShared() const	{	return mShared; }

	void	commitChanges();

	NxdJointShared			mShared;
	NxSphericalJointDesc	mDesc;
	NxU32					mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_DESC = 1<<0
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxSphericalJoint)

#endif
