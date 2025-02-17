#ifndef NXD_ALLOCATEABLE_H
#define NXD_ALLOCATEABLE_H

class NxdScene;

#include <new>
#include "PhysXLoader.h"

class NxdAllocateable
{
public:
	void* operator new(size_t size)
	{
		return NxGetPhysicsSDKAllocator()->malloc(size);
	}

	void  operator delete(void* p)
	{
		NxGetPhysicsSDKAllocator()->free(p);
	}

};


class NxdUnallocateable	// things which definitively should not be new'd or deleted!!!
{
private:
	void* operator new(size_t size)
	{
		return NxGetPhysicsSDKAllocator()->malloc(size);
	}

	void  operator delete(void* p)
	{
		NxGetPhysicsSDKAllocator()->free(p);
	}

};


#define NXD_MALLOC(size) (NxGetPhysicsSDKAllocator()->malloc(size))
#define NXD_FREE(_ptr) NxGetPhysicsSDKAllocator()->free(_ptr)

template <class T>
T *NxdReplicate(void *x)
{
	T *d = static_cast<T*>(NXD_MALLOC(sizeof(T)));
	*d = *(static_cast<T*>(x));
}

#endif
