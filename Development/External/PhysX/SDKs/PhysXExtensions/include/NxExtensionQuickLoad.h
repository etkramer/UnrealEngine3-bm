#ifndef NX_EXTENSION_QUICK_LOAD_H
#define NX_EXTENSION_QUICK_LOAD_H

#include "NxExtensions.h"

#define NX_EXT_QUICK_LOAD_VERSION 100

class NxConvexMeshDesc;
class NxStream;

/**
An extension that accelerates the cooking and loading of convexes through improved memory management.
*/
class NxExtensionQuickLoad : public NxExtension
	{
	protected:
	NX_INLINE NxExtensionQuickLoad(){}
	virtual ~NxExtensionQuickLoad(){}

	public:
	/**
	Retrieves the NxQuickLoadAllocator

	You must use this allocator with the SDK if you are going to use the NxQuickLoad library.
	It wraps the user's allocator.  wrapAllocator() can only be returned once.  Subsequent calls return NULL.
	Alignment must be a power of two.

	Usage:
	1) create your own user allocator
	2) call wrapAllocator
	3) create the NxPhysicsSDK using the wrapped allocator
	4) set the SDK using the setSDK() call below
	*/
	virtual NxUserAllocator * wrapAllocator(NxUserAllocator & userAllocator, NxU32 alignment = 4) = 0;

	/**
	Registers the SDK and cooker libraries with the QuickLoad object.
	This needs to be called before calls below.  Can only be called once.
	NxInitCooking() should already have been called on the cooking interface.

	Pass NX_PHYSICS_SDK_VERSION as interfaceVersion.
	Returns false on a version mismatch or if wrapAllocator() has not yet been called.
	*/
	virtual bool initialize(NxPhysicsSDK &, NxCookingInterface *, NxU32 interfaceVersion = NX_PHYSICS_SDK_VERSION) = 0;

	/**
	Replacement for the SDK's own cookConvexMesh
	*/
	virtual bool cookConvexMesh(const NxConvexMeshDesc& desc, NxStream& stream) = 0;
	/**
	Replacement for the SDK's own createConvexMesh
	*/
	virtual NxConvexMesh * createConvexMesh(NxStream& stream) = 0;
	/**
	Replacement for the SDK's own releaseConvexMesh
	*/
	virtual void releaseConvexMesh(NxConvexMesh& mesh) = 0;
	};


#endif // #ifndef NX_EXTENSION_QUICK_LOAD_H
