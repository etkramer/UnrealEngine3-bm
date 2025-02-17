#ifndef NX_DOUBLEBUFFER_NXD_PULLEYJOINT
#define NX_DOUBLEBUFFER_NXD_PULLEYJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxdAllocateable.h"
#include "NxPulleyJoint.h"
#include "NxPulleyJointDesc.h"
#include "NxdJointHelper.h"
#include "NxdJointShared.h"
#include "NxdAllocateable.h"
#include "NxdCasts.h"

class NxdScene;

class NxdPulleyJoint : public NxPulleyJoint, public NxdAllocateable
{
public:
	DECLARE_JOINT_FORWARDING_METHODS
	
	NxJointType getType()			{ return NX_JOINT_PULLEY;	}

	void	loadFromDesc(const NxPulleyJointDesc& desc);
	void	saveToDesc(NxPulleyJointDesc& desc);
	void	setMotor(const NxMotorDesc &motorDesc);
	bool	getMotor(NxMotorDesc &motorDesc);
	void	setFlags(NxU32 flags);
	NxU32	getFlags();

private:
	friend class NxdJointShared;

	NxdPulleyJoint(NxdScene &scene, NxPulleyJointDesc &desc);
	~NxdPulleyJoint() {}
	NxPulleyJoint *getNxJoint()	{	return static_cast<NxPulleyJoint *>(mShared.getNxJoint()); }
	NxdJointShared &getShared()				{	return mShared; }
	const NxdJointShared &getShared() const	{	return mShared; }

	void	commitChanges();

	NxdJointShared		mShared;
	NxPulleyJointDesc	mDesc;
	NxU32				mBufferFlags;

	enum BufferedFlags
	{
		BUFFERED_DESC			= 1<<0,
		BUFFERED_FLAGS			= 1<<1,
		BUFFERED_MOTOR			= 1<<2,
	};


};

NXD_DECLARE_BUFFERED_CASTS(NxPulleyJoint)

#endif
