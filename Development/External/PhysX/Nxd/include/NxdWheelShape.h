#ifndef NX_DOUBLEBUFFER_NXD_WHEELSHAPE
#define NX_DOUBLEBUFFER_NXD_WHEELSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxWheelShape.h"
#include "NxWheelShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdWheelShape : public NxWheelShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	void	saveToDesc(NxWheelShapeDesc& desc)		const;

	void	setRadius(NxReal radius);	
	NxReal	getRadius() const;	

	void	setSuspensionTravel(NxReal travel);
	NxReal	getSuspensionTravel() const;

	void	setSteerAngle(NxReal angle);
	NxReal	getSteerAngle() const;

	void	setWheelFlags(NxU32 flags);
	NxU32	getWheelFlags() const;

	void	setSuspension(NxSpringDesc spring);	
	NxSpringDesc getSuspension() const;	

	void	setMotorTorque(NxReal torque);
	NxReal	getMotorTorque() const;

	void	setBrakeTorque(NxReal torque);
	NxReal	getBrakeTorque() const;

	void	setLongitudalTireForceFunction(NxTireFunctionDesc tireFunc);	
	NxTireFunctionDesc getLongitudalTireForceFunction() const;

	void	setLateralTireForceFunction(NxTireFunctionDesc tireFunc);	
	NxTireFunctionDesc getLateralTireForceFunction() const;

	void	setInverseWheelMass(NxReal invMass);	
	NxReal	getInverseWheelMass() const;

	void	setAxleSpeed(NxReal speed);
	NxReal getAxleSpeed() const;

	void							setUserWheelContactModify(NxUserWheelContactModify* callback);
	NxUserWheelContactModify*		getUserWheelContactModify();

	NxShape * getContact(NxWheelContactData & dest) const;	

private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdWheelShape(NxdActor &, const NxWheelShapeDesc &);
	~NxdWheelShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_WHEEL;	}
	NxWheelShape *			getShape()	const	{ return static_cast<NxWheelShape *>(mShared.getNxShape()); }
	bool					isWritable() const;

	void					commitChanges();
	void					markShapeChanged();
	void					mark(NxU32 flag) {	mBufferedFlags |= flag; markShapeChanged(); }


	NxdShapeShared			mShared;
	NxWheelShapeDesc		mDesc;

	NxU32					mBufferedFlags;
	NxReal					mAxleSpeed;
	NxUserWheelContactModify* 
							mUserWheelContactModify;

	enum BufferedFlags
	{
		BUFFERED_RADIUS														= 1<<0,
		BUFFERED_SUSPENSIONTRAVEL											= 1<<1,
		BUFFERED_STEERANGLE													= 1<<2,
		BUFFERED_WHEELFLAGS													= 1<<3,
		BUFFERED_SUSPENSION													= 1<<4,
		BUFFERED_MOTORTORQUE												= 1<<5,
		BUFFERED_BRAKETORQUE												= 1<<6,
		BUFFERED_LONGITUDALTIREFORCEFUNCTIONBLAHBLAHBLAH					= 1<<7,
		BUFFERED_LATERALTIREFORCEFUNCTION									= 1<<8,
		BUFFERED_INVERSEWHEELMASS											= 1<<9,
		BUFFERED_AXLESPEED													= 1<<10,
		BUFFERED_USERWHEELCONTACTMODIFY										= 1<<11
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxWheelShape)

#endif
