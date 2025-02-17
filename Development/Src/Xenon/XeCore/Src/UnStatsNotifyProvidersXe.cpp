/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This file contains the implementations of the Xbox specific stats notify providers
 */

#include "EnginePrivate.h"

#include "UnStatsNotifyProvidersXe.h"

#if STATS

/**
 * Writes the stat data out to PIX
 *
 * @param StatId the id of the stat that is being written out
 * @param GroupId the id of the group the stat belongs to
 * @param ParentId the id of parent stat
 * @param InstanceId the instance id of the stat being written
 * @param ParentInstanceId the instance id of parent stat
 * @param ThreadId the thread this stat is for
 * @param Value the value of the stat to write out
 * @param CallsPerFrame the number of calls for this frame
 */
void FStatNotifyProvider_PIX::WriteStat(DWORD StatId,DWORD GroupId,DWORD ParentId,
	DWORD InstanceId,DWORD ParentInstanceId,DWORD ThreadId,DWORD Value,
	DWORD CallsPerFrame)
{
	// Format the counter in terms of milliseconds
	PIXAddNamedCounter((FLOAT)((DOUBLE)Value * 1000.0 * GSecondsPerCycle),"%i: %s",GroupId,TCHAR_TO_ANSI(GStatManager.GetStatName(StatId)));
}

/**
 * Function to write the stat out to the provider's data store
 *
 * @param StatId the id of the stat that is being written out
 * @param GroupId the id of the group the stat belongs to
 * @param Value the value of the stat to write out
 */
void FStatNotifyProvider_PIX::WriteStat(DWORD StatId,DWORD GroupId,FLOAT Value)
{
	PIXAddNamedCounter(Value,"%i: %s",GroupId,TCHAR_TO_ANSI(GStatManager.GetStatName(StatId)));
}

/**
 * Function to write the stat out to the provider's data store
 *
 * @param StatId the id of the stat that is being written out
 * @param GroupId the id of the group the stat belongs to
 * @param Value the value of the stat to write out
 */
void FStatNotifyProvider_PIX::WriteStat(DWORD StatId,DWORD GroupId,DWORD Value)
{
	PIXAddNamedCounter((FLOAT)Value,"%i: %s",GroupId,TCHAR_TO_ANSI(GStatManager.GetStatName(StatId)));
}

/**
 * Sends a named counter to PIX
 *
 * @param Stat the stat that needs to be written
 */
void FStatNotifyProvider_PIX::SendStat(const FCycleStat& Stat)
{
	// Format the counter in terms of milliseconds
	PIXAddNamedCounter((FLOAT)((DOUBLE)Stat.Cycles * 1000.0 * GSecondsPerCycle),"%i: %s",Stat.GroupId,TCHAR_TO_ANSI(Stat.CounterName));
}

#endif // STATS

