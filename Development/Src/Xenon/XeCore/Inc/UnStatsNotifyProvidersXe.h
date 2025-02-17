/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This file contains the Xbox specific notifier that use's PIX to notify
 * counter changes
 */

#if STATS

#ifndef _STATS_NOTIFY_PROVIDER_XE_H
#define _STATS_NOTIFY_PROVIDER_XE_H


/**
 * This provider adds common members/functions for file based stats writing
 */
class FStatNotifyProvider_PIX : public FStatNotifyProvider
{
public:
	/**
	 * Default constructor. Zeros pointer and passes the provider name
	 */
	FStatNotifyProvider_PIX(void) :
		FStatNotifyProvider(TEXT("PIXNamedCounterProvider"),TEXT("PIXStats"))
	{
	}

	/**
	 * Does nothing as we have no internal state
	 */
	virtual UBOOL Init(void)
	{
		return TRUE;
	}

	/**
	 * Does nothing as we have no internal state
	 */
	virtual void Destroy(void)
	{
	}

	/**
	 * Tells the provider that we are starting to supply it with descriptions
	 * for all of the stats/groups.
	 */
	virtual void StartDescriptions(void)
	{
		// Ignored
	}

	/**
	 * Tells the provider that we are finished sending descriptions for all of
	 * the stats/groups.
	 */
	virtual void EndDescriptions(void)
	{
		// Ignored
	}

	/**
	 * Tells the provider that we are starting to supply it with group descriptions
	 */
	virtual void StartGroupDescriptions(void)
	{
		// Ignored
	}

	/**
	 * Tells the provider that we are finished sending stat descriptions
	 */
	virtual void EndGroupDescriptions(void)
	{
		// Ignored
	}

	/**
	 * Tells the provider that we are starting to supply it with stat descriptions
	 */
	virtual void StartStatDescriptions(void)
	{
		// Ignored
	}

	/**
	 * Tells the provider that we are finished sending group descriptions
	 */
	virtual void EndStatDescriptions(void)
	{
		// Ignored
	}

	/**
	 * Adds a stat to the list of descriptions. Used to allow custom stats to
	 * report who they are, parentage, etc. Prevents applications that consume
	 * the stats data from having to change when stats information changes
	 *
	 * @param StatId the id of the stat
	 * @param StatName the name of the stat
	 * @param StatType the type of stat this is
	 * @param GroupId the id of the group this stat belongs to
	 */
	virtual void AddStatDescription(DWORD StatId,const TCHAR* StatName,DWORD StatType,DWORD GroupId)
	{
		// Ignored
	}

	/**
	 * Adds a group to the list of descriptions
	 *
	 * @param GroupId the id of the group being added
	 * @param GroupName the name of the group
	 */
	virtual void AddGroupDescription(DWORD GroupId,const TCHAR* GroupName)
	{
		// Ignored
	}

	/**
	 * Function to write the stat out to the provider's data store
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
	virtual void WriteStat(DWORD StatId,DWORD GroupId,DWORD ParentId,DWORD InstanceId,
		DWORD ParentInstanceId,DWORD ThreadId,DWORD Value,
		DWORD CallsPerFrame);

	/**
	 * Function to write the stat out to the provider's data store
	 *
	 * @param StatId the id of the stat that is being written out
	 * @param GroupId the id of the group the stat belongs to
	 * @param Value the value of the stat to write out
	 */
	virtual void WriteStat(DWORD StatId,DWORD GroupId,FLOAT Value);

	/**
	 * Function to write the stat out to the provider's data store
	 *
	 * @param StatId the id of the stat that is being written out
	 * @param GroupId the id of the group the stat belongs to
	 * @param Value the value of the stat to write out
	 */
	virtual void WriteStat(DWORD StatId,DWORD GroupId,DWORD Value);

	/**
	 * Sends a named counter to PIX
	 *
	 * @param Stat the stat that needs to be written
	 */
	virtual void SendStat(const FCycleStat& Stat);

	/**
	 * We're always sending data to PIX.
	 */
	virtual UBOOL IsAlwaysOn()
	{
		return TRUE;
	}
};

#endif // _STATS_NOTIFY_PROVIDER_XE_H

#endif // STATS
