/*=============================================================================
AsyncInputPoller.h: Handles asynchronous input on the Xbox 360.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef ASYNCINPUTPOLLER_H
#define ASYNCINPUTPOLLER_H

template<typename Type>
class TRingArray
{
protected:
	Type *Data;
	UINT ReadIndex;
	UINT WriteIndex;
	UINT Count;
	UINT MaxSize;

public:
	TRingArray(UINT InMaxSize)
	{
		ReadIndex = 0;
		WriteIndex = 0;
		MaxSize = InMaxSize+1;
		Count = 0;
		Data = new Type[MaxSize];
	}

	~TRingArray()
	{
		delete[]Data;
	}

	UINT Num()
	{
		return Count;
	}

	Type& Get()
	{
		check(ReadIndex != WriteIndex);
		Type &RetValue = Data[ReadIndex];
		Count--;
		ReadIndex++;
		ReadIndex%=MaxSize;
		return RetValue;
	}

	void Add(Type &Item)
	{
		Data[WriteIndex] = Item;
		WriteIndex++;
		WriteIndex%=MaxSize;
		if (WriteIndex == ReadIndex)
		{
			ReadIndex++;
			ReadIndex%=MaxSize;
		}
		else
		{
			Count++;
		}
	}
};

/**
 * Async input packet used to encapsulate controller state.
 */
class FAsyncInputPacket
{
public:
	FAsyncInputPacket()
	{
		appMemzero( this, sizeof(FAsyncInputPacket) );
	}
	FLOAT TimeStamp;
	UBOOL ControllerPresent[FXenonViewport::MAX_NUM_CONTROLLERS];
	XINPUT_STATE State[FXenonViewport::MAX_NUM_CONTROLLERS];
};

/**
 * Async input polling thread being run at a higher framerate than the game. Used to
 * aggregate/ interpolate data for passing it back to the game thread. This is not needed
 * on platforms that don't require polling unbuffered input state.
 */
class FAsyncInputPoller : public FRunnable
{
protected:
	TRingArray<FAsyncInputPacket> Packets;
	UBOOL bShouldKeepRunning;
	FCriticalSection *CriticalSection;

	static FAsyncInputPoller *AsyncInputPoller;
	static FRunnableThread	*AsyncInputThread;
	/** Reference count used to determine whether Start/DestroyThread should perform work. */
	static INT RefCount;

	void ReadInput();

public:
	FAsyncInputPoller();

	virtual UBOOL Init();
	virtual DWORD Run();
	virtual void Stop() { }
	virtual void Exit() { }

	static UBOOL GetInterpolatedState(FAsyncInputPacket &Packet);
	static void StartThread();
	static void DestroyThread();
};


#endif	//ASYNCINPUTPOLLER_H
