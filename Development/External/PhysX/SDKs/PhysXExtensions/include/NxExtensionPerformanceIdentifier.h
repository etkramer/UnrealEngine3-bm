#ifndef NX_EXTENSION_PERFORMANCE_IDENTIFIER_H
#define NX_EXTENSION_PERFORMANCE_IDENTIFIER_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup physics
  @{
*/

#include "NxExtensions.h"

#define NX_EXT_PERFORMANCE_IDENTIFIER_VERSION 100

enum NxPerformanceTestName
{
	NX_PERFTEST_RIGIDBODY_1,
	NX_PERFTEST_FLUID_1,
	NX_PERFTEST_CLOTH_1,
	NX_PERFTEST_NUM_TESTS
};

enum NxPerformanceTestResult
{
	NX_PERFTEST_RESULT_SUCCESS,
	NX_PERFTEST_RESULT_UNIMPLEMENTED,
	NX_PERFTEST_RESULT_UNSUPPORTED,
	NX_PERFTEST_RESULT_STARTUP_FAILED,
	NX_PERFTEST_RESULT_WRONG_VERSION,
	NX_PERFTEST_RESULT_SIMULATION_FAILED,
};

class NxExtensionPerformanceIdentifier : public NxExtension
{
protected:
	NX_INLINE NxExtensionPerformanceIdentifier(){}
	virtual ~NxExtensionPerformanceIdentifier(){}

public:
	/**
	Registers the SDK and cooker libraries with the extension object.
	This needs to be called before calls below.  Can only be called once.
	NxInitCooking() should already have been called on the cooking interface.

	Pass NX_PHYSICS_SDK_VERSION as interfaceVersion.
	Returns false on a version mismatch.
	*/
	virtual bool initialize(NxPhysicsSDK &, NxCookingInterface &, NxU32 interfaceVersion = NX_PHYSICS_SDK_VERSION) = 0;
	/**
	Call this to perform the indicated performance test.
	*/
	virtual NxPerformanceTestResult measurePerformance(NxPerformanceTestName test, NxReal & scoreOut) = 0;
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
