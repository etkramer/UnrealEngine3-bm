/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _CRITICALSECTION_H_
#define _CRITICALSECTION_H_

#include <WinSock2.h>

/**
* This is the Windows version of a critical section. It uses an aggregate
* CRITICAL_SECTION to implement its locking.
*/
class FCriticalSection
{
	/**
	* The windows specific critical section
	*/
	CRITICAL_SECTION CriticalSection;

public:
	/**
	* Constructor that initializes the aggregated critical section
	*/
	FORCEINLINE FCriticalSection(void)
	{
		InitializeCriticalSection(&CriticalSection);
	}

	/**
	* Destructor cleaning up the critical section
	*/
	FORCEINLINE ~FCriticalSection(void)
	{
		DeleteCriticalSection(&CriticalSection);
	}

	/**
	* Locks the critical section
	*/
	FORCEINLINE void Lock(void)
	{
		EnterCriticalSection(&CriticalSection);
	}

	/**
	* Releases the lock on the critical seciton
	*/
	FORCEINLINE void Unlock(void)
	{
		LeaveCriticalSection(&CriticalSection);
	}
};

#endif
