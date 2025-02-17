#ifndef NX_EXTENSION_SCREEN_SURFACE_H
#define NX_EXTENSION_SCREEN_SURFACE_H
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

#define NX_EXT_SCREEN_SURFACE_VERSION 100

class NxExtensionScreenSurface : public NxExtension
{
public:

  virtual NxExtensionType getExtensionType(void)    const { return NX_EXT_SCREEN_SURFACE; };
  virtual unsigned int    getExtensionVersion(void) const { return NX_EXT_SCREEN_SURFACE_VERSION; };


	virtual	void  setProjectionParameters(NxReal *projection44, NxReal width, NxReal height) = 0;
	virtual	void  setSpacing(NxReal spacing) = 0;
	virtual	void  setParticleRadius(NxReal particleRadius) =0;
	virtual	void  setDepthSmoothing(NxU32 smoothing) = 0;
	virtual	void  setSilhouetteSmoothing(NxU32 smoothing) = 0;
	virtual	void  setHierarchyLevels(NxU32 hierarchyLevels) = 0;
	virtual	void  setTriangleWinding(bool positive) = 0;
	virtual	void  buildSurface(	NxU32 numParticles,const NxF32 *particles, NxU32 particleByteStride,const NxF32 *densities, NxU32 densityByteStride,const NxF32 *lifetimes, NxU32 lifetimeStride) =0;
	virtual const NxArray<NxVec3, NxAllocatorDefault>& getVertices(void) = 0;
	virtual const NxArray<NxVec3, NxAllocatorDefault>& getNormals(void) = 0;
	virtual	const NxArray<NxU32, NxAllocatorDefault>&  getTriangles(void) = 0;

private:
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
