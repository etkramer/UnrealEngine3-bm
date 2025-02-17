/*=============================================================================
	AsyncInputThread.cpp: Handles asynchronous input on the Xbox 360.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeDrv.h"
#include "Array.h"
#include "EngineUserInterfaceClasses.h"
#include "AsyncInputThread.h"


#if XBOX

// Only store MAXIMUM_PACKET_COUNT worth of data. Older packets get dropped.
#define MAXIMUM_PACKET_COUNT	15
#define POLL_FREQUENCY			16		// In msecs

// Singletons
FAsyncInputPoller*	FAsyncInputPoller::AsyncInputPoller = NULL;
FRunnableThread*	FAsyncInputPoller::AsyncInputThread = NULL;
/** Reference count used to determine whether Start/DestroyThread should perform work. */
INT					FAsyncInputPoller::RefCount			= 0;	


FAsyncInputPoller::FAsyncInputPoller()
:	Packets(MAXIMUM_PACKET_COUNT)
{
}

/**
 * Starts the input thread
 */
void FAsyncInputPoller::StartThread()
{
	if( RefCount++ == 0 )
	{
		check(!AsyncInputPoller && !AsyncInputThread);
		AsyncInputPoller = new FAsyncInputPoller();
		AsyncInputThread = GThreadFactory->CreateThread(AsyncInputPoller, TEXT("AsyncInputThread"), 0, 0, 16384);
		AsyncInputThread->SetProcessorAffinity(ASYNC_INPUT_HWTHREAD);
	}
}

/**
 * Shuts down the input thread, releases memory
 */
void FAsyncInputPoller::DestroyThread()
{
	if( --RefCount == 0 )
	{
		if (!AsyncInputPoller)
		{
			return;
		}

		AsyncInputPoller->bShouldKeepRunning = FALSE;
		
		AsyncInputThread->WaitForCompletion();
		delete AsyncInputThread;
		AsyncInputThread = NULL;

		delete AsyncInputPoller;
		AsyncInputPoller = NULL;
	}
}

/**
 * Initializes input thread
 *
 * @return TRUE if succeeded
 */
UBOOL FAsyncInputPoller::Init()
{
	bShouldKeepRunning = TRUE;
	CriticalSection = GSynchronizeFactory->CreateCriticalSection();
	return TRUE;
}

// Helper methods
static FORCEINLINE FLOAT Integrate(FLOAT X1, FLOAT T1, FLOAT X2, FLOAT T2)
{
	FLOAT dT = T2-T1;
	return ((X2+X1)*dT)/2.0f;
}

#define INTEGRATE(MemberName) Integrate((FLOAT)LastPacket->State[ControllerIndex].Gamepad.##MemberName, LastPacket->TimeStamp, \
										(FLOAT)Packet.State[ControllerIndex].Gamepad.##MemberName, Packet.TimeStamp)
// End helper methods

/**
 * Returns aggregate input
 *
 * @param OutPacket packet to fill out with data
 * @return TRUE if succeeded
 */
UBOOL FAsyncInputPoller::GetInterpolatedState(FAsyncInputPacket &OutPacket)
{
	UINT PacketCount;
	FScopeLock Lock(AsyncInputPoller->CriticalSection);

	// Poll input, make sure we have up-to-date data
	AsyncInputPoller->ReadInput();

	PacketCount = AsyncInputPoller->Packets.Num();
	if (PacketCount == 0)
	{
		return FALSE;
	}

	FAsyncInputPacket* LastPacket = &AsyncInputPoller->Packets.Get();
	OutPacket = *LastPacket;

	if (PacketCount == 1)
	{
		return TRUE;
	}

	// Initialize accumulation buffers, copy over digital button states
	FLOAT LeftTrigger[FXenonViewport::MAX_NUM_CONTROLLERS] = {0};
	FLOAT RightTrigger[FXenonViewport::MAX_NUM_CONTROLLERS] = {0};
	FLOAT ThumbLX[FXenonViewport::MAX_NUM_CONTROLLERS] = {0};
	FLOAT ThumbLY[FXenonViewport::MAX_NUM_CONTROLLERS] = {0};
	FLOAT ThumbRX[FXenonViewport::MAX_NUM_CONTROLLERS] = {0};
	FLOAT ThumbRY[FXenonViewport::MAX_NUM_CONTROLLERS] = {0};

	FLOAT StartTime = LastPacket->TimeStamp;

	// Integrate each input packet
	while(AsyncInputPoller->Packets.Num())
	{
		FAsyncInputPacket& Packet = AsyncInputPoller->Packets.Get();

		for( UINT ControllerIndex = 0;ControllerIndex < FXenonViewport::MAX_NUM_CONTROLLERS; ControllerIndex++ )
		{
			OutPacket.ControllerPresent[ControllerIndex] |= LastPacket->ControllerPresent[ControllerIndex];
			OutPacket.State[ControllerIndex].Gamepad.wButtons |= Packet.State[ControllerIndex].Gamepad.wButtons;

			LeftTrigger[ControllerIndex]  += INTEGRATE(bLeftTrigger);
			RightTrigger[ControllerIndex] += INTEGRATE(bRightTrigger);
			ThumbLX[ControllerIndex] += INTEGRATE(sThumbLX);
			ThumbLY[ControllerIndex] += INTEGRATE(sThumbLY);
			ThumbRX[ControllerIndex] += INTEGRATE(sThumbRX);
			ThumbRY[ControllerIndex] += INTEGRATE(sThumbRY);
		}

		LastPacket = &Packet;
	}

	// Scale accumulated packets by elapsed time
	FLOAT EndTime = LastPacket->TimeStamp;
	FLOAT DeltaTime = EndTime - StartTime;

	// If all (two) packets have the same timestamp, just return the last packet.
	if ( DeltaTime <= 0.0f )
	{
		OutPacket = *LastPacket;
		return TRUE;
	}

	OutPacket.TimeStamp = EndTime;
	for( UINT ControllerIndex = 0;ControllerIndex < FXenonViewport::MAX_NUM_CONTROLLERS; ControllerIndex++ )
	{
		OutPacket.State[ControllerIndex].Gamepad.bLeftTrigger	= (BYTE)  Clamp<INT>( LeftTrigger[ControllerIndex]  / DeltaTime, 0, 255 );
		OutPacket.State[ControllerIndex].Gamepad.bRightTrigger	= (BYTE)  Clamp<INT>( RightTrigger[ControllerIndex] / DeltaTime, 0, 255 );
		OutPacket.State[ControllerIndex].Gamepad.sThumbLX		= (SHORT) Clamp<INT>( ThumbLX[ControllerIndex] / DeltaTime, -32768, 32767 );
		OutPacket.State[ControllerIndex].Gamepad.sThumbRX		= (SHORT) Clamp<INT>( ThumbRX[ControllerIndex] / DeltaTime, -32768, 32767 );
		OutPacket.State[ControllerIndex].Gamepad.sThumbLY		= (SHORT) Clamp<INT>( ThumbLY[ControllerIndex] / DeltaTime, -32768, 32767 );
		OutPacket.State[ControllerIndex].Gamepad.sThumbRY		= (SHORT) Clamp<INT>( ThumbRY[ControllerIndex] / DeltaTime, -32768, 32767 );
	}

	return TRUE;
}

/**
 * This is defined in the XeLaunch.cpp as indiv games may need to override it for automation
 * purposes
 */
extern DWORD UnrealXInputGetState( DWORD ControllerIndex, PXINPUT_STATE CurrentInput );

/**
 * Internal method; performs actual controller polling. Do not call directly
 */
void FAsyncInputPoller::ReadInput()
{
	FScopeLock Lock(CriticalSection);
	FAsyncInputPacket Packet;

	Packet.TimeStamp = appSeconds();
	for( UINT ControllerIndex = 0;ControllerIndex < FXenonViewport::MAX_NUM_CONTROLLERS; ControllerIndex++ )
	{
		//@todo xenon: handle insertion, removal, merging input from several controllers and all the input mojo from the XDK sample
		//@todo xenon: Add splitscreen support, arbitrary controller choices. None of this code remotely passes cert
		if (UnrealXInputGetState( ControllerIndex, &Packet.State[ControllerIndex] ) == ERROR_SUCCESS)
		{
			Packet.ControllerPresent[ControllerIndex] = TRUE;
		}
		else
		{
			Packet.ControllerPresent[ControllerIndex] = FALSE;
		}
	}

	Packets.Add(Packet);
}

/**
 * Internal method - Thread body; do not call directly
 *
 * @return 0 if succeeded
 */
DWORD FAsyncInputPoller::Run()
{
	HANDLE TimerEvent;
	LARGE_INTEGER DueTime;
	DueTime.QuadPart = -1;

	TimerEvent = CreateWaitableTimer(NULL, FALSE, NULL);
	SetWaitableTimer(TimerEvent, &DueTime, POLL_FREQUENCY, NULL, NULL, FALSE);

	while(bShouldKeepRunning)
	{
		DWORD WaitResult;

		WaitResult = WaitForSingleObject(TimerEvent, POLL_FREQUENCY*2);
		if (WaitResult != WAIT_OBJECT_0)
		{
			// Wait failed
			continue;
		}
		else
		{
			ReadInput();
		}
	}

	CancelWaitableTimer(TimerEvent);
	CloseHandle(TimerEvent);
	return 0;
}

#endif	//XBOX
