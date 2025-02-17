#ifndef NX_DOUBLEBUFFER_NXD_CALLBACKS
#define NX_DOUBLEBUFFER_NXD_CALLBACKS
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxScene.h"
#include "NxdObjArray.h"
#include "NxUserNotify.h"
#include "NxUserContactReport.h"


class NxdUserNotify: public NxUserNotify
{
public:
	NxdUserNotify(): mCallback(0) {}
	void setCallback(NxUserNotify &callback)	{	mCallback = &callback;	}
	NxUserNotify *getCallback()	const			{	return mCallback;		}

	bool onJointBreak(NxReal, NxJoint &);
	void onWake(NxActor **actors, NxU32 count);
	void onSleep(NxActor **actors, NxU32 count);

private:

	void filter(NxActor **input, NxU32 count);

	NxUserNotify *mCallback;
	NxdSimpleArray<NxActor *> mFilteredActors;
};

class NxdUserContactModify: public NxUserContactModify
{
public:
	NxdUserContactModify(): mCallback(0) {}
	void setCallback(NxUserContactModify &callback) {	mCallback = &callback;	}
	NxUserContactModify *getCallback() const		{	return mCallback;		}

	virtual bool onContactConstraint(
		NxU32& changeFlags, 
		const NxShape* shape0, 
		const NxShape* shape1, 
		const NxU32 featureIndex0, 
		const NxU32 featureIndex1,
		NxContactCallbackData& data);

private:
	NxUserContactModify *mCallback;
};

class NxdUserTriggerReport: public NxUserTriggerReport
{
public:
	NxdUserTriggerReport(): mCallback(0) {}
	void setCallback(NxUserTriggerReport &callback) {	mCallback = &callback;	}
	NxUserTriggerReport *getCallback()	const		{	return mCallback;		}

	void onTrigger(NxShape &triggerShape, NxShape &otherShape, NxTriggerFlag status);
private:
	NxUserTriggerReport *mCallback;
};

class NxdUserContactReport: public NxUserContactReport
{
public:
	NxdUserContactReport(): mCallback(0) {}
	void setCallback(NxUserContactReport &callback) { mCallback = &callback;	}
	NxUserContactReport *getCallback()	const		{ return mCallback;			}

	void onContactNotify(NxContactPair &, NxU32);
private:
	NxUserContactReport * mCallback;

};

class NxdUserActorPairFiltering: public NxUserActorPairFiltering
{
public:
	NxdUserActorPairFiltering(): mCallback(0) {}
	void setCallback(NxUserActorPairFiltering &callback)	{ mCallback = &callback; }
	NxUserActorPairFiltering *getCallback()	const			{ return mCallback; }

	void onActorPairs(NxActorPairFilter* filterArray, NxU32 arraySize);
private:
	NxdSimpleArray<NxActorPairFilter> mFilteredArray;
	NxdSimpleArray<NxU32> mRemapArray;
	NxUserActorPairFiltering *mCallback;
};

#if 0

// We could wrap some of the raycast stuff like this, but it's not a
// viable approach for all raycasts, and a partial implementation would
./
class NxdUserRaycastReport: public NxUserRaycastReport
{
public:
	NxdUserRaycastReport(NxUserRaycastReport &callback) : mCallback(callback) {}

	bool onHit(const NxRaycastHit& hits);
private:
	NxUserRaycastReport &mCallback;
};



class NxdUserEntityReportShape: public NxUserEntityReport<NxShape *>
{
public:
	typedef NxUserEntityReport<NxShape *> Callback;
	NxdUserEntityReportShape() : mCallback(0) {}
	void setCallback(Callback &callback) { mCallback = &callback; }

	bool onEvent(NxU32 nbEntities, NxShape * entities);
private:
	Callback *					mCallback;
	NxdSimpleArray<NxShape *>	mFilterArray;
};

#endif

#endif
