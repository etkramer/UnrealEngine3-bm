#ifndef NX_DOUBLEBUFFER_NXD_FLUIDEMITTER
#define NX_DOUBLEBUFFER_NXD_FLUIDEMITTER

#include "fluids/NxFluidEmitter.h"
#include "NxdFluidEmitter.h"
#include "NxdAllocateable.h"

class NxdFluid;

#if NX_USE_FLUID_API

class NxdFluidEmitter : public NxFluidEmitter, public NxdAllocateable
{
public:
	virtual		NxFluid&		getFluid() const;
	virtual		void			setGlobalPose(const NxMat34& mat);
	virtual		void			setGlobalPosition(const NxVec3& vec);
	virtual		void			setGlobalOrientation(const NxMat33& mat);
	virtual		NxMat34			getGlobalPoseVal() const;
	virtual		NxVec3			getGlobalPositionVal() const;
	virtual		NxMat33			getGlobalOrientationVal() const;
	virtual		void			setLocalPose(const NxMat34& mat);
	virtual		void			setLocalPosition(const NxVec3& vec);
	virtual		void			setLocalOrientation(const NxMat33& mat);
	virtual		NxMat34			getLocalPoseVal() const;
	virtual		NxVec3			getLocalPositionVal() const;
	virtual		NxMat33			getLocalOrientationVal() const;
	virtual		void 			setFrameShape(NxShape* shape);
	virtual		NxShape* 		getFrameShape() const;
	virtual		NxReal 			getDimensionX() const;
	virtual		NxReal 			getDimensionY() const;
	virtual		void 			setRandomPos(NxVec3 disp);
	virtual		NxVec3 			getRandomPos() const;
	virtual		void 			setRandomAngle(NxReal angle);
	virtual		NxReal 			getRandomAngle() const;
	virtual		void 			setFluidVelocityMagnitude(NxReal vel);
	virtual		NxReal 			getFluidVelocityMagnitude() const;
	virtual		void 			setRate(NxReal rate);
	virtual		NxReal 			getRate() const;
	virtual		void 			setParticleLifetime(NxReal life);
	virtual		NxReal 			getParticleLifetime() const;
	virtual		void			setRepulsionCoefficient(NxReal coefficient);
	virtual		NxReal			getRepulsionCoefficient() const;
	virtual		void			resetEmission(NxU32 maxParticles);
	virtual		NxU32 			getMaxParticles() const;
	virtual		NxU32			getNbParticlesEmitted() const;
	virtual		void			setFlag(NxFluidEmitterFlag flag, bool val);
	virtual		NX_BOOL			getFlag(NxFluidEmitterFlag flag) const;
	virtual		NX_BOOL			getShape(NxEmitterShape shape) const;
	virtual		NX_BOOL			getType(NxEmitterType type) const;
	virtual		bool			loadFromDesc(const NxFluidEmitterDesc& desc);
	virtual		bool			saveToDesc(NxFluidEmitterDesc &desc) const;
	virtual		void			setName(const char* name);
	virtual		const char*		getName() const;

	class FluidIndexer
	{
	public:
		static NxU32 &index(NxdFluidEmitter &e)					{return e.mFluidIndex; }
	};

private:
	friend class NxdFluid;

	NxdFluidEmitter(NxFluid& fluid, NxFluidEmitter *emitter);
	virtual	~NxdFluidEmitter(void);

	NxFluidEmitter*			getNxFluidEmitter()						const	{ return mNxFluidEmitter; }
	NxdFluid&				getNxdFluid()							const	{ return mNxdFluid; }

	bool					commitChanges();

	bool					instanced()								const;
	bool					writable()								const;
	void					checkWrite(const char *)				const;

	void					computeGlobalPose(NxMat34& globalPose)	const;
	void					updateLocalPose(const NxMat34& globalPose);


	// friends
	NxFluidEmitter*			mNxFluidEmitter;
	NxdFluid&				mNxdFluid;

	// index for EmitterArray in NxdFluid
	NxU32					mFluidIndex;

	// buffered data
	NxU32					mBufferFlags;
	NxFluidEmitterDesc		mFluidEmitterDesc;
	NxI32					mRaisedFlags;
	NxI32					mClearedFlags;

	enum BufferedFlags
	{
		BUFFERED_FRAME_SHAPE	= (1 << 0),
		BUFFERED_ORIENTATION	= (1 << 1),
		BUFFERED_POSITION		= (1 << 2),
		BUFFERED_RANDOM_POS		= (1 << 3),
		BUFFERED_RANDOM_ANGLE	= (1 << 4),
		BUFFERED_VEL_MAGNITUDE	= (1 << 5),
		BUFFERED_RATE			= (1 << 6),
		BUFFERED_LIFETIME		= (1 << 7),
		BUFFERED_REPULSION		= (1 << 8),
		BUFFERED_FLAGS			= (1 << 9),
		BUFFERED_RESET_EMISSION = (1 << 10)
	};
};

#endif // NX_USE_FLUID_API

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
