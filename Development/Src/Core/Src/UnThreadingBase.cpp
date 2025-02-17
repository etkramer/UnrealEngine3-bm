/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "CorePrivate.h"


/**
 * Clean up the synch objects
 */
FQueuedThreadPoolBase::~FQueuedThreadPoolBase(void)
{
	if (SynchQueue != NULL)
	{
		GSynchronizeFactory->Destroy(SynchQueue);
	}
}

/**
 * Creates the synchronization object for locking the queues
 *
 * @return Whether the pool creation was successful or not
 */
UBOOL FQueuedThreadPoolBase::CreateSynchObjects(void)
{
	check(SynchQueue == NULL);
	// Create them with the factory
	SynchQueue = GSynchronizeFactory->CreateCriticalSection();
	// If it is valid then we succeeded
	return SynchQueue != NULL;
}

/**
 * Tells the pool to clean up all background threads
 */
void FQueuedThreadPoolBase::Destroy(void)
{
	{
		FScopeLock Lock(SynchQueue);
		// Clean up all queued objects
		for (INT Index = 0; Index < QueuedWork.Num(); Index++)
		{
			QueuedWork(Index)->Abandon();
		}
		// Empty out the invalid pointers
		QueuedWork.Empty();
	}
	{
		FScopeLock Lock(SynchQueue);
		// Now tell each thread to die and delete those
		for (INT Index = 0; Index < QueuedThreads.Num(); Index++)
		{
			// Wait for the thread to die and have it delete itself using
			// whatever malloc it should
			QueuedThreads(Index)->Kill(TRUE,INFINITE,TRUE);
		}
		// All the pointers are invalid so clean up
		QueuedThreads.Empty();
	}
}

/**
 * Checks to see if there is a thread available to perform the task. If not,
 * it queues the work for later. Otherwise it is immediately dispatched.
 *
 * @param InQueuedWork The work that needs to be done asynchronously
 */
void FQueuedThreadPoolBase::AddQueuedWork(FQueuedWork* InQueuedWork)
{
	check(InQueuedWork != NULL);
	FQueuedThread* Thread = NULL;
	// Check to see if a thread is available. Make sure no other threads
	// can manipulate the thread pool while we do this.
	check(SynchQueue && "Did you forget to call Create()?");
	FScopeLock sl(SynchQueue);
	if (QueuedThreads.Num() > 0)
	{
		// Figure out which thread is available
		INT Index = QueuedThreads.Num() - 1;
		// Grab that thread to use
		Thread = QueuedThreads(Index);
		// Remove it from the list so no one else grabs it
		QueuedThreads.Remove(Index);
	}
	// Was there a thread ready?
	if (Thread != NULL)
	{
		// We have a thread, so tell it to do the work
		Thread->DoWork(InQueuedWork);
	}
	else
	{
		// There were no threads available, queue the work to be done
		// as soon as one does become available
		QueuedWork.AddItem(InQueuedWork);
	}
}

/**
 * Places a thread back into the available pool if there is no pending work
 * to be done. If there is, the thread is put right back to work without it
 * reaching the queue.
 *
 * @param InQueuedThread The thread that is ready to be pooled
 */
void FQueuedThreadPoolBase::ReturnToPool(FQueuedThread* InQueuedThread)
{
	check(InQueuedThread != NULL);
	FQueuedWork* Work = NULL;
	// Check to see if there is any work to be done
	FScopeLock sl(SynchQueue);
	if (QueuedWork.Num() > 0)
	{
		// Grab the oldest work in the queue. This is slower than
		// getting the most recent but prevents work from being
		// queued and never done
		Work = QueuedWork(0);
		// Remove it from the list so no one else grabs it
		QueuedWork.Remove(0);
	}
	// Was there any work to be done?
	if (Work != NULL)
	{
		// Rather than returning this thread to the pool, tell it to do
		// the work instead
		InQueuedThread->DoWork(Work);
	}
	else
	{
		// There was no work to be done, so add the thread to the pool
		QueuedThreads.AddItem(InQueuedThread);
	}
}
