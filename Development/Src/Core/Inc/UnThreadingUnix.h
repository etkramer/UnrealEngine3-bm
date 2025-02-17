/**
 * UnThreadingUnix.h -- Contains all Unix platform-specific definitions
 * of interfaces and concrete classes for multithreading support in the Unreal
 * engine.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _UNTHREADING_UNIX_H
#define _UNTHREADING_UNIX_H

#include <pthread.h>

#if PLATFORM_MACOSX
#include <libkern/OSAtomic.h>  // This is only available in 10.4 and later.
#endif

// Just for sanity checking...this is a big performance hit.
#define UNTHREADINGUNIX_ATOMIC_BY_MUTEXING 0
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
extern pthread_mutex_t UnixAtomicMutex;
#endif

/**
 * Interlocked style functions for threadsafe atomic operations
 */

/**
 * Atomically increments the value pointed to and returns that to the caller
 */
FORCEINLINE INT appInterlockedIncrement(volatile INT* Value)
{
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
    pthread_mutex_lock(&UnixAtomicMutex);
    *Value = *Value + 1;
    INT RetVal = *Value;
    pthread_mutex_unlock(&UnixAtomicMutex);
    return RetVal;
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
	INT retval;
	__asm__ __volatile__("lock ; xaddl %0, (%1) \n\t"
	                        : "=r" (retval)
	                        : "r" (Value), "0" (1)
	                        : "memory");
	return retval+1;  // have to add here, since we get back the original val.
#elif PLATFORM_MACOSX  // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	return (INT) OSAtomicIncrement32Barrier((int32_t *) Value);
#else
#	error Please define your platform.
#endif
}
/**
 * Atomically decrements the value pointed to and returns that to the caller
 */
FORCEINLINE INT appInterlockedDecrement(volatile INT* Value)
{
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
    pthread_mutex_lock(&UnixAtomicMutex);
    *Value = *Value - 1;
    INT RetVal = *Value;
    pthread_mutex_unlock(&UnixAtomicMutex);
    return RetVal;
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
	INT retval;
	__asm__ __volatile__("lock ; xaddl %0, (%1) \n\t"
	                        : "=r" (retval)
	                        : "r" (Value), "0" (-1)
	                        : "memory");
	return retval-1; // have to sub here, since we get back the original val.
#elif PLATFORM_MACOSX  // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	return (INT) OSAtomicDecrement32Barrier((int32_t *) Value);
#else
#	error Please define your platform.
#endif
}
/**
 * Atomically adds the amount to the value pointed to and returns the old
 * value to the caller
 */
FORCEINLINE INT appInterlockedAdd(volatile INT* Value,INT Amount)
{
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
    pthread_mutex_lock(&UnixAtomicMutex);
    *Value += Amount;
    INT RetVal = *Value;
    pthread_mutex_unlock(&UnixAtomicMutex);
    return RetVal;
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
	INT retval;
	__asm__ __volatile__("lock ; xaddl %0, (%1) \n\t"
	                        : "=r" (retval)
	                        : "r" (Value), "0" (Amount)
	                        : "memory");
	return retval;
#elif PLATFORM_MACOSX  // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	return (INT) OSAtomicAdd32Barrier((int32_t) Amount, (int32_t *) Value);
#else
#	error Please define your platform.
#endif
}
/**
 * Atomically swaps two values returning the original value to the caller
 */
FORCEINLINE INT appInterlockedExchange(volatile INT* Value,INT Exchange)
{
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
    pthread_mutex_lock(&UnixAtomicMutex);
    INT RetVal = *Value;
    *Value = Exchange;
    pthread_mutex_unlock(&UnixAtomicMutex);
    return RetVal;
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
	INT retval;
	__asm__ __volatile__("lock ; xchgl %0, (%1) \n\t"
	                        : "=r" (retval)
	                        : "r" (Value), "0" (Exchange)
	                        : "memory");
	return retval;
#elif PLATFORM_MACOSX  // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	INT RetVal;
	do
	{
		RetVal = *Value;
	} while (!OSAtomicCompareAndSwap32Barrier(RetVal, Exchange, (int32_t *) Value));
	return RetVal;
#else
#	error Please define your platform.
#endif
}
/**
 * Atomically compares the value to comperand and replaces with the exchange
 * value if they are equal and returns the original value
 */
FORCEINLINE INT appInterlockedCompareExchange(volatile INT* Dest,INT Exchange,INT Comperand)
{
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
    pthread_mutex_lock(&UnixAtomicMutex);
    INT RetVal = *Dest;
    if (*Dest == Comperand)
        *Dest = Exchange;
    pthread_mutex_unlock(&UnixAtomicMutex);
    return RetVal;
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
	INT retval;
	__asm__ __volatile__("lock ; cmpxchgl %2,(%1) \n\t"
	                        : "=a" (retval)
	                        : "r" (Dest), "r" (Exchange), "0" (Comperand)
	                        : "memory");
	return retval;
#elif PLATFORM_MACOSX  // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	INT RetVal;
	do
	{
		RetVal = *Dest;
	} while ( (!OSAtomicCompareAndSwap32Barrier(Comperand, Exchange, (int32_t *) Dest)) && (RetVal == Comperand) );
	return RetVal;
#else
#	error Please define your platform.
#endif
}
/**
 * Atomically compares the pointer to comperand and replaces with the exchange
 * pointer if they are equal and returns the original value
 */
FORCEINLINE void* appInterlockedCompareExchangePointer(void** Dest,void* Exchange,void* Comperand)
{
#if UNTHREADINGUNIX_ATOMIC_BY_MUTEXING
    pthread_mutex_lock(&UnixAtomicMutex);
    void *RetVal = *Dest;
    if (*Dest == Comperand)
        *Dest = Exchange;
    pthread_mutex_unlock(&UnixAtomicMutex);
    return RetVal;
#elif (defined(__GNUC__) && defined(__i386__))
	void *retval;
	__asm__ __volatile__("lock ; cmpxchgl %2,(%1) \n\t"
	                        : "=a" (retval)
	                        : "r" (Dest), "r" (Exchange), "0" (Comperand)
	                        : "memory");
	return retval;
#elif (defined(__GNUC__) && defined(__x86_64__))
	void *retval;
	__asm__ __volatile__("lock ; cmpxchgq %2,(%1) \n\t"
	                        : "=a" (retval)
	                        : "r" (Dest), "r" (Exchange), "0" (Comperand)
	                        : "memory");
	return retval;
#elif PLATFORM_MACOSX && PLATFORM_32BITS // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	void *RetVal;
	do
	{
		RetVal = *Dest;
	} while ( (!OSAtomicCompareAndSwap32Barrier((int32_t) Comperand, (int32_t) Exchange, (int32_t *) Dest)) && (RetVal == Comperand) );
	return RetVal;
#elif PLATFORM_MACOSX && PLATFORM_64BITS // This is only used on PowerPC, since the x86 version gets used on Intel Mac.
	void *RetVal;
	do
	{
		RetVal = *Dest;
	} while ( (!OSAtomicCompareAndSwap64Barrier((int64_t) Comperand, (int64_t) Exchange, (int64_t *) Dest)) && (RetVal == Comperand) );
	return RetVal;
#else
#	error Please define your platform.
#endif
}

/**
 * Returns a pseudo-handle to the currently executing thread.
 */
FORCEINLINE pthread_t appGetCurrentThread(void)
{
	return pthread_self();
}

/**
 * Returns the currently executing thread's id
 */
FORCEINLINE DWORD appGetCurrentThreadId(void)
{
	// !!! FIXME: this is a 64-bit value on linux/amd64!
	return (DWORD) pthread_self();
}

/**
 * Sets the preferred processor for a thread.
 *
 * @param	ThreadHandle		handle for the thread to set affinity for
 * @param	PreferredProcessor	zero-based index of the processor that this thread prefers
 *
 * @return	the number of the processor previously preferred by the thread, MAXIMUM_PROCESSORS
 *			if the thread didn't have a preferred processor, or (DWORD)-1 if the call failed.
 */
FORCEINLINE DWORD appSetThreadAffinity( pthread_t ThreadHandle, DWORD PreferredProcessor )
{
	// !!! FIXME
	// The Linux API for this is a little flakey right now...apparently
	//  the default scheduling is quite good, though!
	STUBBED("Processor affinity");
	return (DWORD)-1;
}


/**
 * Allocates a thread local store slot
 */
FORCEINLINE DWORD appAllocTlsSlot(void)
{
    // !!! FIXME: pthread_key_t is an "unsigned int" on Linux, but
    // !!! FIXME:  this really shouldn't count on it being anything
    // !!! FIXME:  but sizeof (void *)...
    // !!! FIXME: Calling code seems to assume this can't fail...
    pthread_key_t retval = 0;
    if (pthread_key_create(&retval, NULL) != 0)
        retval = 0xFFFFFFFF;  // matches the Windows TlsAlloc() retval.
	return (DWORD) retval;
}

/**
 * Sets a value in the specified TLS slot
 *
 * @param SlotIndex the TLS index to store it in
 * @param Value the value to store in the slot
 */
FORCEINLINE void appSetTlsValue(DWORD SlotIndex,void* Value)
{
    // !!! FIXME: Calling code seems to assume this can't fail...
    pthread_setspecific((pthread_key_t) SlotIndex, Value);
}

/**
 * Reads the value stored at the specified TLS slot
 *
 * @return the value stored in the slot
 */
FORCEINLINE void* appGetTlsValue(DWORD SlotIndex)
{
	return pthread_getspecific((pthread_key_t) SlotIndex);
}

/**
 * Frees a previously allocated TLS slot
 *
 * @param SlotIndex the TLS index to store it in
 */
FORCEINLINE void appFreeTlsSlot(DWORD SlotIndex)
{
    pthread_key_delete((pthread_key_t) SlotIndex);
}


/**
 * This is the Unix version of a critical section.
 */
class FCriticalSection :
	public FSynchronize
{
	/**
	 * The pthread-specific critical section
	 */
	pthread_mutex_t mutex;

public:
	/**
	 * Constructor that initializes the aggregated critical section
	 */
	FORCEINLINE FCriticalSection(void)
	{
		pthread_mutex_init(&mutex, NULL);
	}

	/**
	 * Destructor cleaning up the critical section
	 */
	FORCEINLINE ~FCriticalSection(void)
	{
		pthread_mutex_destroy(&mutex);
	}

	/**
	 * Locks the critical section
	 */
	FORCEINLINE void Lock(void)
	{
        pthread_mutex_lock(&mutex);
	}

	/**
	 * Releases the lock on the critical seciton
	 */
	FORCEINLINE void Unlock(void)
	{
		pthread_mutex_unlock(&mutex);
	}
};

/**
 * This is the Unix version of an event
 */
class FEventUnix : public FEvent
{
	// This is a little complicated, in an attempt to match Win32 Event semantics...
    typedef enum
    {
        TRIGGERED_NONE,
        TRIGGERED_ONE,
        TRIGGERED_ALL,
        TRIGGERED_PULSE,
    } TriggerType;

    inline void LockEventMutex();
    inline void UnlockEventMutex();
	UBOOL bInitialized;
	UBOOL bIsManualReset;
	volatile TriggerType Triggered;
	volatile INT WaitingThreads;
    pthread_mutex_t Mutex;
    pthread_cond_t Condition;

public:
	/**
	 * Constructor that zeroes the handle
	 */
	FEventUnix(void);

	/**
	 * Cleans up the event handle if valid
	 */
	virtual ~FEventUnix(void);

	/**
	 * Waits for the event to be signaled before returning
	 */
	virtual void Lock(void);

	/**
	 * Triggers the event so any waiting threads are allowed access
	 */
	virtual void Unlock(void);

	/**
	 * Creates the event. Manually reset events stay triggered until reset.
	 * Named events share the same underlying event.
	 *
	 * @param bIsManualReset Whether the event requires manual reseting or not
	 * @param InName Whether to use a commonly shared event or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns TRUE if the event was created, FALSE otherwise
	 */
	virtual UBOOL Create(UBOOL bIsManualReset = FALSE,const TCHAR* InName = NULL);

	/**
	 * Triggers the event so any waiting threads are activated
	 */
	virtual void Trigger(void);

	/**
	 * Resets the event to an untriggered (waitable) state
	 */
	virtual void Reset(void);

	/**
	 * Triggers the event and resets the triggered state NOTE: This behaves
	 * differently for auto-reset versus manual reset events. All threads
	 * are released for manual reset events and only one is for auto reset
	 */
	virtual void Pulse(void);

	/**
	 * Waits for the event to be triggered
	 *
	 * @param WaitTime Time in milliseconds to wait before abandoning the event
	 * (DWORD)-1 is treated as wait infinite
	 *
	 * @return TRUE if the event was signaled, FALSE if the wait timed out
	 */
	virtual UBOOL Wait(DWORD WaitTime = (DWORD)-1);
};

/**
 * This is the Unix factory for creating various synchronization objects.
 */
class FSynchronizeFactoryUnix : public FSynchronizeFactory
{
public:
	/**
	 * Zeroes its members
	 */
	FSynchronizeFactoryUnix(void);

	/**
	 * Creates a new critical section
	 *
	 * @return The new critical section object or NULL otherwise
	 */
	virtual FCriticalSection* CreateCriticalSection(void);

	/**
	 * Creates a new event
	 *
	 * @param bIsManualReset Whether the event requires manual reseting or not
	 * @param InName Whether to use a commonly shared event or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns the new event object if successful, NULL otherwise
	 */
	virtual FEvent* CreateSynchEvent(UBOOL bIsManualReset = FALSE,const TCHAR* InName = NULL);

	/**
	 * Cleans up the specified synchronization object using the correct heap
	 *
	 * @param InSynchObj The synchronization object to destroy
	 */
	virtual void Destroy(FSynchronize* InSynchObj);
};

/**
 * This is the Unix class used for all poolable threads
 */
class FQueuedThreadUnix : public FQueuedThread
{
	/**
	 * The event that tells the thread there is work to do
	 */
	FEvent* DoWorkEvent;

	/**
	 * The thread handle to clean up. Must be closed or this will leak resources
	 */
	pthread_t ThreadHandle;

	/**
	 * If true, thread was created.
	 */
	UBOOL ThreadCreated;

	/**
	 * If true, the thread should exit
	 */
	volatile UBOOL TimeToDie;

	/**
	 * If true, the thread is ready to be joined.
	 */
	volatile UBOOL ThreadHasTerminated;

	/**
	 * The work this thread is doing
	 */
	FQueuedWork* QueuedWork;

	/**
	 * The synchronization object for the work member
	 */
	FCriticalSection* QueuedWorkSynch;

	/**
	 * The pool this thread belongs to
	 */
	FQueuedThreadPool* OwningThreadPool;

	/**
	 * The real thread entry point. It waits for work events to be queued. Once
	 * an event is queued, it executes it and goes back to waiting.
	 */
	void Run(void);

	/**
	 * Bridge between Pthread entry point and Unreal's.
	 */
	static void *_ThreadProc(void *pThis);

public:
	/**
	 * Zeros any members
	 */
	FQueuedThreadUnix(void);

	/**
	 * Deletes any allocated resources. Kills the thread if it is running.
	 */
	virtual ~FQueuedThreadUnix(void);

	/**
	 * Creates the thread with the specified stack size and creates the various
	 * events to be able to communicate with it.
	 *
	 * @param InPool The thread pool interface used to place this thread
	 *		  back into the pool of available threads when its work is done
	 * @param ProcessorMask The processor set to run the thread on
	 * @param InStackSize The size of the stack to create. 0 means use the
	 *		  current thread's stack size
	 *
	 * @return True if the thread and all of its initialization was successful, false otherwise
	 */
	virtual UBOOL Create(class FQueuedThreadPool* InPool,DWORD ProcessorMask,
		DWORD InStackSize = 0);
	
	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @param bShouldWait If true, the call will wait for the thread to exit
	 * @param MaxWaitTime The amount of time to wait before killing it. It
	 * defaults to inifinite.
	 * @param bShouldDeleteSelf Whether to delete ourselves upon completion
	 *
	 * @return True if the thread exited gracefull, false otherwise
	 */
	virtual UBOOL Kill(UBOOL bShouldWait = FALSE,DWORD MaxWaitTime = INFINITE,
		UBOOL bShouldDeleteSelf = FALSE);

	/**
	 * Tells the thread there is work to be done. Upon completion, the thread
	 * is responsible for adding itself back into the available pool.
	 *
	 * @param InQueuedWork The queued work to perform
	 */
	virtual void DoWork(FQueuedWork* InQueuedWork);
};

/**
 * This class fills in the platform specific features that the parent
 * class doesn't implement. The parent class handles all common, non-
 * platform specific code, while this class provides all of the Unix
 * specific methods. It handles the creation of the threads used in the
 * thread pool.
 */
class FQueuedThreadPoolUnix : public FQueuedThreadPoolBase
{
public:
	/**
	 * Cleans up any threads that were allocated in the pool
	 */
	virtual ~FQueuedThreadPoolUnix(void);

	/**
	 * Creates the thread pool with the specified number of threads
	 *
	 * @param InNumQueuedThreads Specifies the number of threads to use in the pool
	 * @param ProcessorMask Specifies which processors should be used by the pool
	 * @param StackSize The size of stack the threads in the pool need (32K default)
	 *
	 * @return Whether the pool creation was successful or not
	 */
	virtual UBOOL Create(DWORD InNumQueuedThreads,DWORD ProcessorMask = 0,
		DWORD StackSize = (32 * 1024));
};

/**
 * This is the base interface for all runnable thread classes. It specifies the
 * methods used in managing its life cycle.
 */
class FRunnableThreadUnix : public FRunnableThread
{
	/**
	 * The thread handle to clean up. Must be closed or this will leak resources
	 */
	pthread_t ThreadHandle;

	/**
	 * If true, thread was created.
	 */
	UBOOL ThreadCreated;

	/**
	 * The runnable object to execute on this thread
	 */
	FRunnable* Runnable;

	/**
	 * Whether we should delete ourselves on thread exit
	 */
	UBOOL bShouldDeleteSelf;

	/**
	 * Whether we should delete the runnable on thread exit
	 */
	UBOOL bShouldDeleteRunnable;

	/**
	 * The priority to run the thread at
	 */
	EThreadPriority ThreadPriority;

	/**
	 * If true, the thread is ready to be joined.
	 */
	volatile UBOOL ThreadHasTerminated;

	/**
	 * The real thread entry point. It calls the Init/Run/Exit methods on
	 * the runnable object
	 */
	DWORD Run(void);

	/**
	 * Bridge between Pthread entry point and Unreal's.
	 */
	static void *_ThreadProc(void *pThis);

public:
	/**
	 * Zeroes members
	 */
	FRunnableThreadUnix(void);

	/**
	 * Cleans up any resources
	 */
	~FRunnableThreadUnix(void);

	/**
	 * Creates the thread with the specified stack size and thread priority.
	 *
	 * @param InRunnable The runnable object to execute
	 * @param ThreadName Name of the thread
	 * @param bAutoDeleteSelf Whether to delete this object on exit
	 * @param bAutoDeleteRunnable Whether to delete the runnable object on exit
	 * @param InStackSize The size of the stack to create. 0 means use the
	 * current thread's stack size
	 * @param InThreadPri Tells the thread whether it needs to adjust its
	 * priority or not. Defaults to normal priority
	 *
	 * @return True if the thread and all of its initialization was successful, false otherwise
	 */
	UBOOL Create(FRunnable* InRunnable, const TCHAR* ThreadName,
		UBOOL bAutoDeleteSelf = 0,UBOOL bAutoDeleteRunnable = 0,DWORD InStackSize = 0,
		EThreadPriority InThreadPri = TPri_Normal);
	
	/**
	 * Changes the thread priority of the currently running thread
	 *
	 * @param NewPriority The thread priority to change to
	 */
	virtual void SetThreadPriority(EThreadPriority NewPriority);

	/**
	 * Tells the OS the preferred CPU to run the thread on. NOTE: Don't use
	 * this function unless you are absolutely sure of what you are doing
	 * as it can cause the application to run poorly by preventing the
	 * scheduler from doing its job well.
	 *
	 * @param ProcessorNum The preferred processor for executing the thread on
	 */
	virtual void SetProcessorAffinity(DWORD ProcessorNum);

	/**
	 * Tells the thread to either pause execution or resume depending on the
	 * passed in value.
	 *
	 * @param bShouldPause Whether to pause the thread (true) or resume (false)
	 */
	virtual void Suspend(UBOOL bShouldPause = 1);

	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @param bShouldWait If true, the call will wait for the thread to exit
	 * @param MaxWaitTime The amount of time to wait before killing it.
	 * Defaults to inifinite.
	 *
	 * @return True if the thread exited gracefull, false otherwise
	 */
	virtual UBOOL Kill(UBOOL bShouldWait = 0,DWORD MaxWaitTime = 0);

	/**
	 * Halts the caller until this thread is has completed its work.
	 */
	virtual void WaitForCompletion(void);

	/**
	* Thread ID for this thread 
	*
	* @return ID that was set by CreateThread
	*/
	virtual DWORD GetThreadID(void);
};

/**
 * This is the factory interface for creating threads on Unix
 */
class FThreadFactoryUnix : public FThreadFactory
{
public:
	/**
	 * Creates the thread with the specified stack size and thread priority.
	 *
	 * @param InRunnable The runnable object to execute
	 * @param ThreadName Name of the thread
	 * @param bAutoDeleteSelf Whether to delete this object on exit
	 * @param bAutoDeleteRunnable Whether to delete the runnable object on exit
	 * @param InStackSize The size of the stack to create. 0 means use the
	 * current thread's stack size
	 * @param InThreadPri Tells the thread whether it needs to adjust its
	 * priority or not. Defaults to normal priority
	 *
	 * @return The newly created thread or NULL if it failed
	 */
	virtual FRunnableThread* CreateThread(FRunnable* InRunnable, const TCHAR* ThreadName,
		UBOOL bAutoDeleteSelf = 0,UBOOL bAutoDeleteRunnable = 0,
		DWORD InStackSize = 0,EThreadPriority InThreadPri = TPri_Normal);

	/**
	 * Cleans up the specified thread object using the correct heap
	 *
	 * @param InThread The thread object to destroy
	 */
	virtual void Destroy(FRunnableThread* InThread);
};

#endif  // define _UNTHREADING_UNIX_H

