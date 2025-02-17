/*=============================================================================
	RenderingThread.cpp: Rendering thread implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

//
// Definitions
//

/** The size of the rendering command buffer, in bytes. */
#define RENDERING_COMMAND_BUFFER_SIZE	(256*1024)

/** comment in this line to display the average amount of data per second processed by the command buffer */
//#define RENDERING_COMMAND_BUFFER_STATS	1

//
// Globals
//

FRingBuffer GRenderCommandBuffer(RENDERING_COMMAND_BUFFER_SIZE, Max<UINT>(16,sizeof(FSkipRenderCommand)));
UBOOL GIsThreadedRendering = FALSE;
UBOOL GUseThreadedRendering = FALSE;
FMemStack GRenderingThreadMemStack(65536);

/** If the rendering thread has been terminated by an unhandled exception, this contains the error message. */
FString GRenderingThreadError;

/**
 * Polled by the game thread to detect crashes in the rendering thread.
 * If the rendering thread crashes, it sets this variable to FALSE.
 */
UBOOL GIsRenderingThreadHealthy = TRUE;

UBOOL GGameThreadWantsToSuspendRendering = FALSE;

/**
 * If the rendering thread is in its idle loop (which ticks rendering tickables
 */
UBOOL GIsRenderingThreadIdling = FALSE;

/** Whether the rendering thread is suspended (not even processing the tickables) */
volatile INT GIsRenderingThreadSuspended = 0;

/**
 * Maximum rate the rendering thread will tick tickables when idle (in Hz)
 */
FLOAT GRenderingThreadMaxIdleTickFrequency = 40.f;

/**
 *	Constructor that flushes and suspends the renderthread
 *	@param bRecreateThread	- Whether the rendering thread should be completely destroyed and recreated, or just suspended.
 */
FSuspendRenderingThread::FSuspendRenderingThread( UBOOL bInRecreateThread )
{
	bRecreateThread = bInRecreateThread;
	bUseRenderingThread = GUseThreadedRendering;
	bWasRenderingThreadRunning = GIsThreadedRendering;
	if ( bRecreateThread )
	{
		GUseThreadedRendering = FALSE;
		StopRenderingThread();
		appInterlockedIncrement( &GIsRenderingThreadSuspended );
	}
	else
	{
		// First tell the render thread to finish up all pending commands and then suspend its activities.
		ENQUEUE_UNIQUE_RENDER_COMMAND( ScopedSuspendRendering, { RHISuspendRendering(); appInterlockedIncrement( &GIsRenderingThreadSuspended ); } );

		// Block until the flag is set on the render-thread.
		while ( !GIsRenderingThreadSuspended )
		{
			appSleep( 0.0f );
		}

		// Then immediately queue up a command to resume rendering. This command won't be processed until GIsRenderingThreadSuspended == FALSE.
		// This command must be the very next thing the rendering thread executes.
		ENQUEUE_UNIQUE_RENDER_COMMAND( ScopedResumeRendering, { RHIResumeRendering(); } );
	}
}

/** Destructor that starts the renderthread again */
FSuspendRenderingThread::~FSuspendRenderingThread()
{
	if ( bRecreateThread )
	{
		GUseThreadedRendering = bUseRenderingThread;
		appInterlockedDecrement( &GIsRenderingThreadSuspended );
		if ( bUseRenderingThread && bWasRenderingThreadRunning )
		{
			StartRenderingThread();
		}
	}
	else
	{
		// Resume the render thread again. It should immediately process the ScopedResumeRendering command queued up from before.
		appInterlockedDecrement( &GIsRenderingThreadSuspended );
	}
}


/**
 * Tick all rendering thread tickable objects
 */
void TickRenderingTickables()
{
	static DOUBLE LastTickTime = appSeconds();

	// calc how long has passed since last tick
	DOUBLE CurTime = appSeconds();
	FLOAT DeltaSeconds = CurTime - LastTickTime;
	
	// don't let idle ticks happen too fast
	UBOOL bShouldDoTick = !GIsRenderingThreadIdling || DeltaSeconds > (1.f/GRenderingThreadMaxIdleTickFrequency);

	if (!bShouldDoTick || GIsRenderingThreadSuspended)
	{
		return;
	}

	UINT ObjectsThatResumedRendering = 0;

	// tick any rendering thread tickables
	for (INT ObjectIndex = 0; ObjectIndex < FTickableObject::RenderingThreadTickableObjects.Num(); ObjectIndex++)
	{
		FTickableObject* TickableObject = FTickableObject::RenderingThreadTickableObjects(ObjectIndex);
		// make sure it wants to be ticked and the rendering thread isn't suspended
		if (TickableObject->IsTickable())
		{
			if (GGameThreadWantsToSuspendRendering && TickableObject->NeedsRenderingResumedForRenderingThreadTick())
			{
				RHIResumeRendering();
				ObjectsThatResumedRendering++;
			}
			TickableObject->Tick(DeltaSeconds);
		}
	}
	// update the last time we ticked
	LastTickTime = CurTime;

	// if no ticked objects resumed rendering, make sure we're suspended if game thread wants us to be
	if (ObjectsThatResumedRendering == 0 && GGameThreadWantsToSuspendRendering)
	{
		RHISuspendRendering();
	}
}

/** Accumulates how many cycles the renderthread has been idle. It's defined in RenderingThread.cpp. */
DWORD GRenderThreadIdle = 0;
/** How many cycles the renderthread used (excluding idle time). It's set once per frame in FViewport::Draw. */
DWORD GRenderThreadTime = 0;

/** The rendering thread main loop */
void RenderingThreadMain()
{
#if RENDERING_COMMAND_BUFFER_STATS
	static UINT		RenderingCommandByteCount = 0;
	static DOUBLE	Then = 0.0;
#endif

	void* ReadPointer = NULL;
	UINT NumReadBytes = 0;

	while(GIsThreadedRendering)
	{
		// Command processing loop.
		while ( GRenderCommandBuffer.BeginRead(ReadPointer,NumReadBytes) )
		{
			// Process one render command.
			{
				SCOPE_CYCLE_COUNTER(STAT_RenderingBusyTime);
				FRenderCommand* Command = (FRenderCommand*)ReadPointer;
				UINT CommandSize = Command->Execute();
				Command->~FRenderCommand();
				GRenderCommandBuffer.FinishRead(CommandSize);
#if RENDERING_COMMAND_BUFFER_STATS
				RenderingCommandByteCount += CommandSize;
#endif
			}

			// Suspending loop.
			{
				while ( GIsRenderingThreadSuspended )
				{
					if (GHandleDirtyDiscError)
					{
						appHandleIOFailure( NULL );
					}

					// Just sleep a little bit.
					appSleep( 0.001f );
				}
			}
		}

		// Idle loop:
		{
			SCOPE_CYCLE_COUNTER(STAT_RenderingIdleTime);
			GIsRenderingThreadIdling = TRUE;
			DWORD IdleStart = appCycles();
			while ( GIsThreadedRendering && !GRenderCommandBuffer.BeginRead(ReadPointer,NumReadBytes) )
			{
				if (GHandleDirtyDiscError)
				{
					appHandleIOFailure( NULL );
				}

				// Suspend rendering thread while we're trace dumping to avoid running out of memory due to holding
				// an IRQ for too long.
				while( GShouldSuspendRenderingThread )
				{
					appSleep( 1 );
				}

				// Wait for up to 16ms for rendering commands.
				GRenderCommandBuffer.WaitForRead(16);

				GRenderThreadIdle += appCycles() - IdleStart;

				// tick tickable objects when there are no commands to process, so if there are
				// no commands for a long time, we don't want to starve the tickables
				TickRenderingTickables();
				IdleStart = appCycles();

#if RENDERING_COMMAND_BUFFER_STATS
				DOUBLE Now = appSeconds();
				if( Now - Then > 2.0 )
				{
					debugf( TEXT( "RenderCommands: %7d bytes per second" ), UINT( RenderingCommandByteCount / ( Now - Then ) ) );
					Then = Now;
					RenderingCommandByteCount = 0;
				}
#endif
			}
			GIsRenderingThreadIdling = FALSE;
			GRenderThreadIdle += appCycles() - IdleStart;
		}
	};

	// Advance and reset the rendering thread stats before returning from the thread.
	// This is necessary to prevent the stats for the last frame of the thread from persisting.
	STAT(GStatManager.AdvanceFrameForThread());
}

/** The rendering thread runnable object. */
class FRenderingThread : public FRunnable
{
public:

	// FRunnable interface.
	virtual UBOOL Init(void) 
	{ 
		FLightPrimitiveInteraction::InitializeMemoryPool();
#if _XBOX && USE_XeD3D_RHI
		//Acquire ownership of D3D
		if( GIsRHIInitialized )
		{
			extern IDirect3DDevice9* GDirect3DDevice;
			GDirect3DDevice->AcquireThreadOwnership();
		}
#endif
		return TRUE; 
	}
	virtual void Exit(void) 
	{
#if _XBOX && USE_XeD3D_RHI
		//Don't relinquish the D3D device until we are completely out of the thread loop
		if( GIsRHIInitialized )
		{
			extern IDirect3DDevice9* GDirect3DDevice;
			GDirect3DDevice->ReleaseThreadOwnership();
		}
#endif
	}
	virtual void Stop(void) {}
	virtual DWORD Run(void)
	{
#if _MSC_VER && !XBOX
		extern INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );
		if(!appIsDebuggerPresent())
		{
			__try
			{
				RenderingThreadMain();
			}
			__except( CreateMiniDump( GetExceptionInformation() ) )
			{
				GRenderingThreadError = GErrorHist;

				// Use a memory barrier to ensure that the game thread sees the write to GRenderingThreadError before
				// the write to GIsRenderingThreadHealthy.
				appMemoryBarrier();

				GIsRenderingThreadHealthy = FALSE;
			}
		}
		else
#endif
		{
			RenderingThreadMain();
		}

		return 0;
	}
};

/** Thread used for rendering */
FRunnableThread* GRenderingThread = NULL;
FRunnable* GRenderingThreadRunnable = NULL;

void StartRenderingThread()
{
	check(!GIsThreadedRendering && GUseThreadedRendering);

	// Turn on the threaded rendering flag.
	GIsThreadedRendering = TRUE;

	// Create the rendering thread.
	GRenderingThreadRunnable = new FRenderingThread();

	EThreadPriority RenderingThreadPrio = TPri_Normal;
#if PS3
	// below normal, so streaming doesn't get blocked
	RenderingThreadPrio = TPri_BelowNormal;
#endif
#if _XBOX && USE_XeD3D_RHI
	//Have to give up control of the D3D device here
	if( GIsRHIInitialized )
	{
		// transfer ownership of D3D device to rendering thread
		extern IDirect3DDevice9* GDirect3DDevice;
		GDirect3DDevice->ReleaseThreadOwnership();
	}
#endif  //_XBOX && USE_XeD3D_RHI

	GRenderingThread = GThreadFactory->CreateThread(GRenderingThreadRunnable, TEXT("RenderingThread"), 0, 0, 0, RenderingThreadPrio);
#if _XBOX
	check(GRenderingThread);
	// Assign the rendering thread to the specified hwthread
	GRenderingThread->SetProcessorAffinity(RENDERING_HWTHREAD);
#endif
}

void StopRenderingThread()
{
	if(GIsThreadedRendering)
	{
		// Get the list of objects which need to be cleaned up when the rendering thread is done with them.
		FPendingCleanupObjects* PendingCleanupObjects = GetPendingCleanupObjects();

		// Make sure we're not in the middle of streaming textures.
		(*GFlushStreamingFunc)();

		// Wait for the rendering thread to finish executing all enqueued commands.
		FlushRenderingCommands();

		// Turn off the threaded rendering flag.
		GIsThreadedRendering = FALSE;

		// Wait for the rendering thread to return.
		GRenderingThread->WaitForCompletion();

		// Destroy the rendering thread objects.
		GThreadFactory->Destroy(GRenderingThread);
		GRenderingThread = NULL;
		delete GRenderingThreadRunnable;
		GRenderingThreadRunnable = NULL;

		// Delete the pending cleanup objects which were in use by the rendering thread.
		delete PendingCleanupObjects;

#if _XBOX && USE_XeD3D_RHI 
        //Take control of the D3D device here
		if( GIsRHIInitialized )
		{
			// transfer ownership of D3D device to rendering thread
			extern IDirect3DDevice9* GDirect3DDevice;
			GDirect3DDevice->AcquireThreadOwnership();   
		}
#endif  //_XBOX && USE_XeD3D_RHI

		FDynamicMeshEmitterData::FInstanceBufferDesc::EmptyGarbage();
	}
}

void CheckRenderingThreadHealth()
{
	if(!GIsRenderingThreadHealthy)
	{
		GErrorHist[0] = 0;
		GIsCriticalError = FALSE;
		GError->Logf(TEXT("Rendering thread exception:\r\n%s"),*GRenderingThreadError);
	}
	
	GLog->FlushThreadedLogs();

	// Process pending windows messages, which is necessary to the rendering thread in some rare cases where DX10
	// sends window messages (from IDXGISwapChain::Present) to the main thread owned viewport window.
	// Only process sent messages to minimize the opportunity for re-entry in the editor, since wx messages are not deferred.
	#if _WINDOWS
		appWinPumpSentMessages();
	#endif
}

UBOOL IsInRenderingThread()
{
	return !GRenderingThread || (appGetCurrentThreadId() == GRenderingThread->GetThreadID());
}

UBOOL IsInGameThread()
{
	// if the game thread is uninitialized, then we are calling this VERY early before other threads will have started up, so it will be the game thread
	return !GIsGameThreadIdInitialized || appGetCurrentThreadId() == GGameThreadId;
}

void FRenderCommandFence::BeginFence()
{
	appInterlockedIncrement((INT*)&NumPendingFences);

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		FenceCommand,
		FRenderCommandFence*,Fence,this,
		{
			// Use a memory barrier to ensure that memory writes made by the rendering thread are visible to the game thread before the
			// NumPendingFences decrement.
			appMemoryBarrier();

			appInterlockedDecrement((INT*)&Fence->NumPendingFences);
		});
}

UINT FRenderCommandFence::GetNumPendingFences() const
{
	CheckRenderingThreadHealth();
	return NumPendingFences;
}

/** Accumulates how many cycles the gamethread has been idle. */
DWORD GGameThreadIdle = 0;
/** How many cycles the gamethread used (excluding idle time). It's set once per frame in FViewport::Draw. */
DWORD GGameThreadTime = 0;

/**
 * Waits for pending fence commands to retire.
 * @param NumFencesLeft	- Maximum number of fence commands to leave in queue
 */
void FRenderCommandFence::Wait( UINT NumFencesLeft/*=0*/ ) const
{
	check(IsInGameThread());

	SCOPE_CYCLE_COUNTER(STAT_GameIdleTime);
	DWORD IdleStart = appCycles();
	while(NumPendingFences > NumFencesLeft)
	{
		// Check that the rendering thread is still running.
		CheckRenderingThreadHealth();

		// Yield CPU time while waiting.
		appSleep(0);
	};
	GGameThreadIdle += appCycles() - IdleStart;
}

/**
 * Waits for all deferred deleted objects to be cleaned up. Should only be used from the game thread.
 */
void FlushDeferredDeletion()
{
#if XBOX && USE_XeD3D_RHI
	// make sure all textures are freed up after GCing
	FlushRenderingCommands();
	extern void DeleteUnusedXeResources();
	ENQUEUE_UNIQUE_RENDER_COMMAND( BlockUntilGPUIdle, { RHIBlockUntilGPUIdle(); } );
	for( INT i=0; i<2; i++ )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND( DeleteResources, { DeleteUnusedXeResources(); } );
	}
	FlushRenderingCommands();
#elif PS3 && USE_PS3_RHI
	// make sure all textures are freed up after GCing
	FlushRenderingCommands();
	extern void DeleteUnusedPS3Resources();
	ENQUEUE_UNIQUE_RENDER_COMMAND( BlockUntilGPUIdle, { RHIBlockUntilGPUIdle(); } );
	for( INT i=0; i<2; i++ )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND( DeleteResources, { DeleteUnusedPS3Resources(); } );
	}
	FlushRenderingCommands();
#endif
}

/**
 * Waits for the rendering thread to finish executing all pending rendering commands.  Should only be used from the game thread.
 */
void FlushRenderingCommands()
{
	// Find the objects which may be cleaned up once the rendering thread command queue has been flushed.
	FPendingCleanupObjects* PendingCleanupObjects = GetPendingCleanupObjects();

	// Issue a fence command to the rendering thread and wait for it to complete.
	FRenderCommandFence Fence;
	Fence.BeginFence();
	Fence.Wait();

	// Delete the objects which were enqueued for deferred cleanup before the command queue flush.
	delete PendingCleanupObjects;
}

/** The set of deferred cleanup objects which are pending cleanup. */
FPendingCleanupObjects* GPendingCleanupObjects = NULL;

FPendingCleanupObjects::~FPendingCleanupObjects()
{
	for(INT ObjectIndex = 0;ObjectIndex < Num();ObjectIndex++)
	{
		(*this)(ObjectIndex)->FinishCleanup();
	}
}

void BeginCleanup(FDeferredCleanupInterface* CleanupObject)
{
	if(GIsThreadedRendering)
	{
		// If no pending cleanup set exists, create a new one.
		if(!GPendingCleanupObjects)
		{
			GPendingCleanupObjects = new FPendingCleanupObjects();
		}

		// Add the specified object to the pending cleanup set.
		GPendingCleanupObjects->AddItem(CleanupObject);
	}
	else
	{
		CleanupObject->FinishCleanup();
	}
}

FPendingCleanupObjects* GetPendingCleanupObjects()
{
	FPendingCleanupObjects* OldPendingCleanupObjects = GPendingCleanupObjects;
	GPendingCleanupObjects = NULL;

	return OldPendingCleanupObjects;
}
