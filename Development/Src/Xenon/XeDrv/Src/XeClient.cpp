/*=============================================================================
	XeClient.cpp: UXenonClient code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeDrv.h"
#include "XeD3DDrvPrivate.h"
#include "EngineAudioDeviceClasses.h"

/*-----------------------------------------------------------------------------
	Class implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UXenonClient);

/*-----------------------------------------------------------------------------
	UXenonClient implementation.
-----------------------------------------------------------------------------*/

//
// UXenonClient constructor.
//
UXenonClient::UXenonClient()
{
	KeyMapVirtualToName.Set(VK_BACK,KEY_BackSpace);
	KeyMapVirtualToName.Set(VK_TAB,KEY_Tab);
	KeyMapVirtualToName.Set(VK_RETURN,KEY_Enter);
	KeyMapVirtualToName.Set(VK_PAUSE,KEY_Pause);

	KeyMapVirtualToName.Set(VK_CAPITAL,KEY_CapsLock);
	KeyMapVirtualToName.Set(VK_ESCAPE,KEY_Escape);
	KeyMapVirtualToName.Set(VK_SPACE,KEY_SpaceBar);
	KeyMapVirtualToName.Set(VK_PRIOR,KEY_PageUp);
	KeyMapVirtualToName.Set(VK_NEXT,KEY_PageDown);
	KeyMapVirtualToName.Set(VK_END,KEY_End);
	KeyMapVirtualToName.Set(VK_HOME,KEY_Home);

	KeyMapVirtualToName.Set(VK_LEFT,KEY_Left);
	KeyMapVirtualToName.Set(VK_UP,KEY_Up);
	KeyMapVirtualToName.Set(VK_RIGHT,KEY_Right);
	KeyMapVirtualToName.Set(VK_DOWN,KEY_Down);

	KeyMapVirtualToName.Set(VK_INSERT,KEY_Insert);
	KeyMapVirtualToName.Set(VK_DELETE,KEY_Delete);

	KeyMapVirtualToName.Set(0x30,KEY_Zero);
	KeyMapVirtualToName.Set(0x31,KEY_One);
	KeyMapVirtualToName.Set(0x32,KEY_Two);
	KeyMapVirtualToName.Set(0x33,KEY_Three);
	KeyMapVirtualToName.Set(0x34,KEY_Four);
	KeyMapVirtualToName.Set(0x35,KEY_Five);
	KeyMapVirtualToName.Set(0x36,KEY_Six);
	KeyMapVirtualToName.Set(0x37,KEY_Seven);
	KeyMapVirtualToName.Set(0x38,KEY_Eight);
	KeyMapVirtualToName.Set(0x39,KEY_Nine);

	KeyMapVirtualToName.Set(0x41,KEY_A);
	KeyMapVirtualToName.Set(0x42,KEY_B);
	KeyMapVirtualToName.Set(0x43,KEY_C);
	KeyMapVirtualToName.Set(0x44,KEY_D);
	KeyMapVirtualToName.Set(0x45,KEY_E);
	KeyMapVirtualToName.Set(0x46,KEY_F);
	KeyMapVirtualToName.Set(0x47,KEY_G);
	KeyMapVirtualToName.Set(0x48,KEY_H);
	KeyMapVirtualToName.Set(0x49,KEY_I);
	KeyMapVirtualToName.Set(0x4A,KEY_J);
	KeyMapVirtualToName.Set(0x4B,KEY_K);
	KeyMapVirtualToName.Set(0x4C,KEY_L);
	KeyMapVirtualToName.Set(0x4D,KEY_M);
	KeyMapVirtualToName.Set(0x4E,KEY_N);
	KeyMapVirtualToName.Set(0x4F,KEY_O);
	KeyMapVirtualToName.Set(0x50,KEY_P);
	KeyMapVirtualToName.Set(0x51,KEY_Q);
	KeyMapVirtualToName.Set(0x52,KEY_R);
	KeyMapVirtualToName.Set(0x53,KEY_S);
	KeyMapVirtualToName.Set(0x54,KEY_T);
	KeyMapVirtualToName.Set(0x55,KEY_U);
	KeyMapVirtualToName.Set(0x56,KEY_V);
	KeyMapVirtualToName.Set(0x57,KEY_W);
	KeyMapVirtualToName.Set(0x58,KEY_X);
	KeyMapVirtualToName.Set(0x59,KEY_Y);
	KeyMapVirtualToName.Set(0x5A,KEY_Z);

	KeyMapVirtualToName.Set(VK_NUMPAD0,KEY_NumPadZero);
	KeyMapVirtualToName.Set(VK_NUMPAD1,KEY_NumPadOne);
	KeyMapVirtualToName.Set(VK_NUMPAD2,KEY_NumPadTwo);
	KeyMapVirtualToName.Set(VK_NUMPAD3,KEY_NumPadThree);
	KeyMapVirtualToName.Set(VK_NUMPAD4,KEY_NumPadFour);
	KeyMapVirtualToName.Set(VK_NUMPAD5,KEY_NumPadFive);
	KeyMapVirtualToName.Set(VK_NUMPAD6,KEY_NumPadSix);
	KeyMapVirtualToName.Set(VK_NUMPAD7,KEY_NumPadSeven);
	KeyMapVirtualToName.Set(VK_NUMPAD8,KEY_NumPadEight);
	KeyMapVirtualToName.Set(VK_NUMPAD9,KEY_NumPadNine);

	KeyMapVirtualToName.Set(VK_MULTIPLY,KEY_Multiply);
	KeyMapVirtualToName.Set(VK_ADD,KEY_Add);
	KeyMapVirtualToName.Set(VK_SUBTRACT,KEY_Subtract);
	KeyMapVirtualToName.Set(VK_DECIMAL,KEY_Decimal);
	KeyMapVirtualToName.Set(VK_DIVIDE,KEY_Divide);

	KeyMapVirtualToName.Set(VK_F1,KEY_F1);
	KeyMapVirtualToName.Set(VK_F2,KEY_F2);
	KeyMapVirtualToName.Set(VK_F3,KEY_F3);
	KeyMapVirtualToName.Set(VK_F4,KEY_F4);
	KeyMapVirtualToName.Set(VK_F5,KEY_F5);
	KeyMapVirtualToName.Set(VK_F6,KEY_F6);
	KeyMapVirtualToName.Set(VK_F7,KEY_F7);
	KeyMapVirtualToName.Set(VK_F8,KEY_F8);
	KeyMapVirtualToName.Set(VK_F9,KEY_F9);
	KeyMapVirtualToName.Set(VK_F10,KEY_F10);
	KeyMapVirtualToName.Set(VK_F11,KEY_F11);
	KeyMapVirtualToName.Set(VK_F12,KEY_F12);

	KeyMapVirtualToName.Set(VK_NUMLOCK,KEY_NumLock);

	KeyMapVirtualToName.Set(VK_SCROLL,KEY_ScrollLock);

	KeyMapVirtualToName.Set(VK_OEM_1,KEY_Semicolon);
	KeyMapVirtualToName.Set(VK_OEM_PLUS,KEY_Equals);
	KeyMapVirtualToName.Set(VK_OEM_COMMA,KEY_Comma);
	KeyMapVirtualToName.Set(VK_OEM_MINUS,KEY_Underscore);
	KeyMapVirtualToName.Set(VK_OEM_PERIOD,KEY_Period);
	KeyMapVirtualToName.Set(VK_OEM_2,KEY_Slash);
	KeyMapVirtualToName.Set(VK_OEM_3,KEY_Tilde);
	KeyMapVirtualToName.Set(VK_OEM_4,KEY_LeftBracket);
	KeyMapVirtualToName.Set(VK_OEM_5,KEY_Backslash);
	KeyMapVirtualToName.Set(VK_OEM_6,KEY_RightBracket);
	KeyMapVirtualToName.Set(VK_OEM_7,KEY_Quote);

	for(UINT KeyIndex = 0;KeyIndex < 256;KeyIndex++) 
	{
		if(KeyMapVirtualToName.Find(KeyIndex)) 
		{
			KeyMapNameToVirtual.Set(KeyMapVirtualToName.FindRef(KeyIndex),KeyIndex);
		}
	}

	AudioDevice		= NULL;
}

//
// Static init.
//
void UXenonClient::StaticConstructor()
{
	new(GetClass(),TEXT("AudioDeviceClass")	,RF_Public)UClassProperty(CPP_PROPERTY(AudioDeviceClass)	,TEXT("Audio")	,CPF_Config,UAudioDevice::StaticClass());

	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UXenonClient, Engine ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UXenonClient, AudioDevice ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UXenonClient, AudioDeviceClass ) );
}

//
// Initialize the platform-specific viewport manager subsystem.
// Must be called after the Unreal object manager has been initialized.
// Must be called before any viewports are created.
//
void UXenonClient::Init( UEngine* InEngine )
{
	Engine = InEngine;

	// Initialize the audio device.
	if( GEngine->bUseSound )
	{		
		AudioDevice = ConstructObject<UAudioDevice>( AudioDeviceClass );
		if( !AudioDevice->Init() )
		{
			AudioDevice = NULL;
		}
	}

	// remove bulk data if no sounds were initialized
	if( AudioDevice == NULL )
	{		
		appSoundNodeRemoveBulkData();
	}

	// Success.
	debugf( NAME_Init, TEXT("Client initialized") );
}

//
//	UXenonClient::Serialize - Make sure objects the client reference aren't garbage collected.
//
void UXenonClient::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << Engine << AudioDeviceClass << AudioDevice;
}

//
//	UXenonClient::Destroy - Shut down the platform-specific viewport manager subsystem.
//
void UXenonClient::FinishDestroy()
{
	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		// Close all viewports.
		for(INT ViewportIndex = 0;ViewportIndex < Viewports.Num();ViewportIndex++)
		{
			delete Viewports(ViewportIndex);
		}
		Viewports.Empty();

		debugf( NAME_Exit, TEXT("Xenon client shut down") );
	}
	else
	{
		check(Viewports.Num()==0);
		check(AudioDevice==NULL);
	}

	Super::FinishDestroy();
}

//
//	UXenonClient::Tick - Perform timer-tick processing on all visible viewports.
//
void UXenonClient::Tick( FLOAT DeltaTime )
{
	// Process input.
	SCOPE_CYCLE_COUNTER(STAT_InputTime);

	GInputLatencyTimer.GameThreadTick();

	// Process input.
	for(UINT ViewportIndex = 0;ViewportIndex < (UINT)Viewports.Num();ViewportIndex++)
	{
		Viewports(ViewportIndex)->ProcessInput( DeltaTime );
	}
}

/** Function to immediately stop any force feedback vibration that might be going on this frame. */
void UXenonClient::ForceClearForceFeedback()
{
	for(UINT ViewportIndex = 0;ViewportIndex < (UINT)Viewports.Num();ViewportIndex++)
	{
		if(Viewports(ViewportIndex) && Viewports(ViewportIndex)->GetClient())
		{
			for( INT ControllerIndex = 0; ControllerIndex < FXenonViewport::MAX_NUM_CONTROLLERS; ControllerIndex++ )
			{
				UForceFeedbackManager* Manager = Viewports(ViewportIndex)->GetClient()->GetForceFeedbackManager(ControllerIndex);
				if(Manager)
				{
					Manager->ForceClearWaveformData(ControllerIndex);
				}
			}
		}
	}
}

// @hack DB: Gears-specific hack for movie testing.
typedef void (*TestMovieCallbackType)(const FString& MovieName);
TestMovieCallbackType TestMovieCallback = NULL;

#if USE_XeD3D_RHI
extern UINT GTilingMode;
extern const INT GNumTilingModes;
extern const TCHAR *GTilingDesc[];
#endif // USE_XeD3D_RHI

/**
 * Xenon specific exec handling
 *
 * @param Cmd - exec command string
 * @param Ar - output archive
 */
UBOOL UXenonClient::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	if( Super::Exec(Cmd,Ar) )
	{
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("TILING")) )
	{
		// toggle Tiling

		extern UBOOL GUseTilingCode;

		if( GScreenWidth == 1280 )
		{
			FString AmtStr(ParseToken(Cmd, 0));
			GUseTilingCode = !GUseTilingCode;

			debugf( TEXT("GUseTilingCode=%d"),GUseTilingCode );
		}
		else
		{
			debugf( TEXT( "Tiling mode not allowed in 4:3 mode" ), GUseTilingCode );
			GUseTilingCode = FALSE;
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("TILINGMODE")) )
	{
#if USE_XeD3D_RHI
		// toggle Tiling
		if( GUseTilingCode )
		{
			FString AmtStr(ParseToken(Cmd, 0));
			if( appStricmp(*AmtStr,TEXT("")) == 0 )
			{
				GTilingMode = (GTilingMode + 1) % GNumTilingModes;
			}
			else
			{
				// explicit threshold
				GTilingMode = Clamp(appAtoi(*AmtStr),0,GNumTilingModes - 1);
			}

			debugf( TEXT("TilingMode=%s"),GTilingDesc[GTilingMode] );
		}
		else
		{
			debugf( TEXT( "Tiling not enabled" ), GTilingDesc[GTilingMode] );
		}
#endif // USE_XeD3D_RHI

		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("TRACE")) )
	{
		GCurrentTraceName = FName( Cmd, FNAME_Add );
		debugf(TEXT("Requesting a trace of '%s'"),*GCurrentTraceName.ToString());
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("VSYNC")) )
	{
		// toggle VSYNC

		// global that controls D3DRS_PRESENTIMMEDIATETHRESHOLD
		extern DWORD GPresentImmediateThreshold;

		FString AmtStr(ParseToken(Cmd, 0));
		if( appStricmp(*AmtStr,TEXT("")) == 0 )
		{            
			if( GPresentImmediateThreshold == 100 )
			{
				// enabled
				GPresentImmediateThreshold = 0;
			}
			else
			{
				// disabled
				GPresentImmediateThreshold = 100;
			}
		}
		else
		{
			// explicit threshold
			GPresentImmediateThreshold = Clamp(appAtoi(*AmtStr),0,100);
		}

		debugf( TEXT("GPresentImmediateThreshold=%d"),GPresentImmediateThreshold );

		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("XMEMDUMP")) )
	{
#if USE_XeD3D_RHI
		extern void XMemDumpAllocationInfo( FOutputDevice& Ar );
		XMemDumpAllocationInfo( Ar );
#endif // USE_XeD3D_RHI
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("GPURESOURCEDUMP")) || ParseCommand(&Cmd, TEXT("DUMPGPURESOURCES")) )
	{
		// Figure out whether we want a detailed split or not.
		UBOOL bSummaryOnly = TRUE;
		if( ParseToken( Cmd, 0 ) == TEXT("DETAILED") )
		{
			bSummaryOnly = FALSE;
		}
#if USE_XeD3D_RHI
		// Dump resource information.
		extern void DumpXeGPUResources( UBOOL bSummaryOnly, FOutputDevice& Ar );
		DumpXeGPUResources( bSummaryOnly, Ar );
#endif // USE_XeD3D_RHI
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEGAMMACORRECT")) )
	{
#if USE_XeD3D_RHI
		GUseCorrectiveGammaRamp = !GUseCorrectiveGammaRamp;
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			SetGammaRamp,
			UBOOL, bUseCorrectiveGammaRamp, GUseCorrectiveGammaRamp,
		{
			XeSetCorrectiveGammaRamp(bUseCorrectiveGammaRamp);
		});
#endif
		return TRUE;
	}
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	else if( ParseCommand(&Cmd, TEXT("LISTTHREADS")) )
	{
		const INT NUM_HW_THREADS = 6;
		// Handles to HW thread related perf counters that can be queried.
		HANDLE PerfCountersIdle[NUM_HW_THREADS];	// Handle to counter for percentage of time idle
		HANDLE PerfCountersISR[NUM_HW_THREADS];		// Handle to counter for percentage of time in interrupt service requests (ISR)
		HANDLE PerfCountersDPC[NUM_HW_THREADS];		// Handle to counter for percentage of time in deferred procedure calls (DPC)
		// Open performance counters for thread idle/ISR/DPC percentages.
		for( INT HWThreadIndex=0; HWThreadIndex<NUM_HW_THREADS; HWThreadIndex++ )
		{
			verify( SUCCEEDED( DmOpenPerformanceCounter( TCHAR_TO_ANSI(*FString::Printf(TEXT("%%Thread%i Idle"),HWThreadIndex)), &PerfCountersIdle[HWThreadIndex] ) ) );
			verify( SUCCEEDED( DmOpenPerformanceCounter( TCHAR_TO_ANSI(*FString::Printf(TEXT("%%Thread%i ISR"),HWThreadIndex)), &PerfCountersISR[HWThreadIndex] ) ) );
			verify( SUCCEEDED( DmOpenPerformanceCounter( TCHAR_TO_ANSI(*FString::Printf(TEXT("%%Thread%i DPC"),HWThreadIndex)), &PerfCountersDPC[HWThreadIndex] ) ) );
		}

		// Perf counter values for each HW thread.
		FLOAT PercentageIdle[NUM_HW_THREADS];		// Percentage of time idle
		FLOAT PercentageISR[NUM_HW_THREADS];		// Percentage of time in interrupt service requests (ISR)
		FLOAT PercentageDPC[NUM_HW_THREADS];		// Percentage of time in deferred procedure calls (DPC)
		// Query perf counters and close handles.
		for( INT HWThreadIndex=0; HWThreadIndex<NUM_HW_THREADS; HWThreadIndex++ )
		{	
			// Query data.
			DM_COUNTDATA IdleData;
			DM_COUNTDATA ISRData;
			DM_COUNTDATA DPCData;
			verify( SUCCEEDED( DmQueryPerformanceCounterHandle( PerfCountersIdle[HWThreadIndex], DMCOUNT_PRATIO, &IdleData ) ) );
			verify( SUCCEEDED( DmQueryPerformanceCounterHandle( PerfCountersISR[HWThreadIndex], DMCOUNT_PRATIO, &ISRData ) ) );
			verify( SUCCEEDED( DmQueryPerformanceCounterHandle( PerfCountersDPC[HWThreadIndex], DMCOUNT_PRATIO, &DPCData ) ) );

			// Convert data to percentages.
			PercentageIdle[HWThreadIndex]	= (FLOAT)((DOUBLE) IdleData.CountValue.QuadPart / IdleData.RateValue.QuadPart) * 100.f;
			PercentageISR[HWThreadIndex]	= (FLOAT)((DOUBLE) ISRData.CountValue.QuadPart / ISRData.RateValue.QuadPart) * 100.f;
			PercentageDPC[HWThreadIndex]	= (FLOAT)((DOUBLE) DPCData.CountValue.QuadPart / DPCData.RateValue.QuadPart) * 100.f;

			// Close handles.
			verify( SUCCEEDED( DmClosePerformanceCounter( PerfCountersIdle[HWThreadIndex] ) ) );
			verify( SUCCEEDED( DmClosePerformanceCounter( PerfCountersISR[HWThreadIndex] ) ) );
			verify( SUCCEEDED( DmClosePerformanceCounter( PerfCountersDPC[HWThreadIndex] ) ) );
		}

		// Get list of thread ids to query.
		DWORD ThreadIds[256];
		DWORD NumThreadIds = ARRAY_COUNT(ThreadIds);
		verify( SUCCEEDED( DmGetThreadList( ThreadIds, &NumThreadIds ) ) );

		// Iterate over all HW threads and spit out idle status and associated threads.
		debugf(TEXT("Listing HW threads and associated SW threads:"));
		for( INT HWThreadIndex=0; HWThreadIndex<NUM_HW_THREADS; HWThreadIndex++ )
		{
			debugf(TEXT("HW thread %i - %6.2f%% Idle, %6.2f%% ISR, %6.2f%% DPC"),HWThreadIndex,PercentageIdle[HWThreadIndex],PercentageISR[HWThreadIndex],PerfCountersDPC[HWThreadIndex]);
			// Iterate over all SW threads and list matching ones.
			for( DWORD ThreadIndex=0; ThreadIndex<NumThreadIds; ThreadIndex++ )
			{
				// Query thread info for SW thread.
				DM_THREADINFOEX ThreadInfo;
				ThreadInfo.Size = sizeof(ThreadInfo);
				verify( SUCCEEDED( DmGetThreadInfoEx( ThreadIds[ThreadIndex], &ThreadInfo ) ) );

				// Only list threads running on this HW thread.
				if( ThreadInfo.CurrentProcessor == HWThreadIndex )
				{
					debugf(TEXT("   Name: %30s   Priority: %2i   Stack size: %4i KByte   Suspend count: %i   ThreadId: %x"),
						ANSI_TO_TCHAR(ThreadInfo.ThreadNameAddress),
						ThreadInfo.Priority,
						// Stack grows downwards so it's Start - End
						((BYTE*)ThreadInfo.StackBase - (BYTE*)ThreadInfo.StackLimit) / 1024,
						ThreadInfo.SuspendCount,
						ThreadIds[ThreadIndex]);
				}
			}
		}
	}
#endif
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	else if( ParseCommand(&Cmd, TEXT("TEXTUREMEMDUMP")) )
	{
		appDumpTextureMemoryStats(TEXT(""));
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("ToggleDefrag")))
	{
		extern UBOOL GEnableTextureMemoryDefragmentation;
		GEnableTextureMemoryDefragmentation = !GEnableTextureMemoryDefragmentation;
		Ar.Logf( TEXT("Texture memory defragmentation: %s"), GEnableTextureMemoryDefragmentation ? TEXT("ENABLED") : TEXT("DISABLED") );
		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("ToggleAOFog")))
	{
		extern UBOOL GAOCombineWithHeightFog;
		GAOCombineWithHeightFog = !GAOCombineWithHeightFog;
		Ar.Logf( TEXT("AO Fog: %s"), GAOCombineWithHeightFog ? TEXT("ENABLED") : TEXT("DISABLED") );
		return TRUE;
	}
#endif

	return FALSE;
}

//
// UXenonClient::CreateViewportFrame
//
FViewportFrame* UXenonClient::CreateViewportFrame(FViewportClient* ViewportClient,const TCHAR* InName,UINT SizeX,UINT SizeY,UBOOL Fullscreen)
{
	return new FXenonViewport(this,ViewportClient,InName,SizeX,SizeY,Fullscreen);
}

//
// UXenonClient::CreateWindowChildViewport
//
FViewport* UXenonClient::CreateWindowChildViewport(FViewportClient* ViewportClient,void* ParentWindow,UINT SizeX,UINT SizeY,INT InPosX,INT InPosY)
{
	check(0);
	return NULL;
}

//
// UXenonClient::CloseViewport
//
void UXenonClient::CloseViewport(FViewport* Viewport)
{
	FXenonViewport* XenonViewport = (FXenonViewport*)Viewport;
	delete XenonViewport;
}

/**
 * Retrieves the name of the key mapped to the specified character code.
 *
 * @param	KeyCode	the key code to get the name for; should be the key's ANSI value
 */
FName UXenonClient::GetVirtualKeyName( INT KeyCode ) const
{
	if ( KeyCode < 255 )
	{
		const FName* VirtualName = KeyMapVirtualToName.Find((BYTE)KeyCode);
		if ( VirtualName != NULL )
		{
			return *VirtualName;
		}
	}

	return NAME_None;
}

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsXeDrv( INT& Lookup )
{
	UXenonClient::StaticClass();
	UXnaForceFeedbackManager::StaticClass();
}

/**
 * Auto generates names.
 */
void AutoRegisterNamesXeDrv()
{
}

