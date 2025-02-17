/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _REFCOUNTEDTARGET_H_
#define _REFCOUNTEDTARGET_H_

#include <WinSock2.h>
#include "Common.h"

template <class T>
class FReferenceCountPtr;

/**
 * Base class for reference counted targets.
 */
template <class T>
class FRefCountedTarget
{
	friend class FReferenceCountPtr<T>;

private:
	volatile LONG ReferenceCount;

	/**
	* Increments the reference count.
	*
	* @return	The new reference count.
	*/
	inline LONG IncrementReferenceCount()
	{
		return InterlockedIncrement(&ReferenceCount);
	}

	/**
	* Decrements the reference count.
	*
	* @return	The new reference count.
	*/
	inline LONG DecrementReferenceCount()
	{
		return InterlockedDecrement(&ReferenceCount);
	}

	/**
	* Returns the current reference count.
	*/
	inline LONG GetReferenceCount()
	{
		return InterlockedAnd(&ReferenceCount, ~0);
	}

public:
	FRefCountedTarget() : ReferenceCount(0)
	{

	}

	virtual ~FRefCountedTarget()
	{

	}
};

/**
 * Smart pointer to reference counted targets.
 */
template <class T>
class FReferenceCountPtr
{
private:
	FRefCountedTarget<T>* Target;

	/**
	* Increments the reference count.
	*
	* @return	The new reference count.
	*/
	LONG IncrementReferenceCount()
	{
		LONG Count = 0;

		if(Target)
		{
			Count = Target->IncrementReferenceCount();
		}

		return Count;
	}

	/**
	* Decrements the reference count.
	*
	* @return	The new reference count.
	*/
	LONG DecrementReferenceCount()
	{
		LONG Count = 0;

		if(Target)
		{
			Count = Target->DecrementReferenceCount();

			if(Count <= 0)
			{
				delete Target;
			}

			Target = NULL;
		}

		return Count;
	}

public:
	FReferenceCountPtr() : Target(NULL)
	{

	}

	FReferenceCountPtr(T* NewTarget) : Target(NewTarget)
	{
		IncrementReferenceCount();
	}

	FReferenceCountPtr(const FReferenceCountPtr<T>& Ptr) : Target(NULL)
	{
		operator=(Ptr);
	}

	~FReferenceCountPtr()
	{
		DecrementReferenceCount();
	}

	inline TARGETHANDLE GetHandle()
	{
		return (TARGETHANDLE)Target;
	}

	inline T* operator*()
	{
		return (T*)Target;
	}

	inline T* operator->()
	{
		return (T*)Target;
	}

	inline operator bool() const
	{
		return Target != NULL;
	}

	FReferenceCountPtr<T>& operator=(T* Ptr)
	{
		DecrementReferenceCount();
		Target = Ptr;
		IncrementReferenceCount();

		return *this;
	}

	FReferenceCountPtr<T>& operator=(const FReferenceCountPtr<T>& Ptr)
	{
		DecrementReferenceCount();
		Target = Ptr.Target;
		IncrementReferenceCount();

		return *this;
	}

	inline bool operator==(const FReferenceCountPtr<T>& Ptr) const
	{
		return Target == Ptr.Target;
	}
};

#endif
