/*=============================================================================
	D3DUtil.h: D3D RHI utility implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI 

//
// Stat declarations.
//

DECLARE_STATS_GROUP(TEXT("RHI"),STATGROUP_XeRHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("CommandBuffer KB remaining"),STAT_CommandBufferBytesRemaining,STATGROUP_XeRHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("CommandBuffer KB used"),STAT_CommandBufferBytesUsed,STATGROUP_XeRHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("CommandBuffer Min KB remaining"),STAT_CommandBufferBytesMinRemaining,STATGROUP_XeRHI);
DECLARE_DWORD_COUNTER_STAT(TEXT("CommandBuffer Max KB used"),STAT_CommandBufferBytesMaxUsed,STATGROUP_XeRHI);

void VerifyD3DResult(HRESULT D3DResult,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line)
{
	if(FAILED(D3DResult))
	{
		FString ErrorCodeText( FString::Printf(TEXT("%08X"),(INT)D3DResult) );
		appErrorf(TEXT("%s failed at %s:%u with error %s"),ANSI_TO_TCHAR(Code),ANSI_TO_TCHAR(Filename),Line,*ErrorCodeText);
	}
}

/**
 * Adds a PIX event
 *
 * @param Color The color to draw the event as
 * @param Text The text displayed with the event
 */
void appBeginDrawEvent(const FColor& Color,const TCHAR* Text)
{
#if LINK_AGAINST_PROFILING_D3D_LIBRARIES
	PIXBeginNamedEvent(Color.DWColor(),TCHAR_TO_ANSI(Text));
#endif
}

/**
 * Ends the current PIX event
 */
void appEndDrawEvent(void)
{
#if LINK_AGAINST_PROFILING_D3D_LIBRARIES
	PIXEndNamedEvent();
#endif
}

/**
 * Platform specific function for setting the value of a counter that can be
 * viewed in PIX.
 */
void appSetCounterValue(const TCHAR* CounterName, FLOAT Value)
{
#if LINK_AGAINST_PROFILING_D3D_LIBRARIES
	PIXAddNamedCounter( Value, TCHAR_TO_ANSI(CounterName) );
#endif
}

#endif
