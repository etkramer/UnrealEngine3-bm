/*=============================================================================
	ScopedDebugInfo.cpp: Scoped debug info implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

/** The TLS index for the debug info stack. */
static DWORD GThreadDebugInfoTLSID = appAllocTlsSlot();

FScopedDebugInfo::FScopedDebugInfo(INT InNumReplacedOuterCalls):
	NumReplacedOuterCalls(InNumReplacedOuterCalls),
	NextOuterInfo((FScopedDebugInfo*)appGetTlsValue(GThreadDebugInfoTLSID))
{
	// Set the this debug info as the current innermost debug info.
	appSetTlsValue(GThreadDebugInfoTLSID,this);
}

FScopedDebugInfo::~FScopedDebugInfo()
{
	FScopedDebugInfo* CurrentInnermostDebugInfo = (FScopedDebugInfo*)appGetTlsValue(GThreadDebugInfoTLSID);
	check(CurrentInnermostDebugInfo == this);

	// Set the next outermost link to the current innermost debug info.
	appSetTlsValue(GThreadDebugInfoTLSID,NextOuterInfo);
}

FScopedDebugInfo* FScopedDebugInfo::GetDebugInfoStack()
{
	return (FScopedDebugInfo*)appGetTlsValue(GThreadDebugInfoTLSID);
}
