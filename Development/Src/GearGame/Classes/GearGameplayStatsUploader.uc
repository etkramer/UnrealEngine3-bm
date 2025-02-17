/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This interface deals with the online games. It creates, destroys, performs
 * searches for online games. This interface is overloaded to provide custom
 * matchmaking services
 */
class GearGameplayStatsUploader extends GearEventsInterface;

/**
 * Uploads the data contained within the object to the Vince server
 *
 * @param PlayerNum the index of the player uploading the stats
 * @param StatsObject the object to convert to Vince data
 *
 * @return true if successful uploading, false otherwise
 */
function bool UploadStats(GearStatsObject StatsObject)
{
	local int PlayerEvtIdx, GameplayEvtIdx;
	local int PlayerIdx, TargetPlayerIdx;
	local int EventNameIdx;
    local vector EventLocation;
    local float EventTime;
	BeginLog();

    for (GameplayEvtIdx = 0; GameplayEvtIdx < StatsObject.GameplayEvents.Length; GameplayEvtIdx++)
    {
		PlayerEvtIdx = StatsObject.GameplayEvents[GameplayEvtIdx].PlayerEventAndTarget >> 16;
		TargetPlayerIdx = StatsObject.GameplayEvents[GameplayEvtIdx].PlayerEventAndTarget & 0xffff;
		EventNameIdx = StatsObject.GameplayEvents[GameplayEvtIdx].EventNameAndDesc >> 16;

		PlayerIdx = StatsObject.PlayerEvents[PlayerEvtIdx].PlayerIndexAndYaw >> 16;
        EventLocation = StatsObject.PlayerEvents[PlayerEvtIdx].EventLocation;
        EventTime = StatsObject.PlayerEvents[PlayerEvtIdx].EventTime;
		
   		BeginEvent(string(StatsObject.EventNames[EventNameIdx]));
		AddParamFloat("EventLocationX", EventLocation.X);
		AddParamFloat("EventLocationY", EventLocation.Y);
		AddParamFloat("EventLocationZ", EventLocation.Z);
		AddParamInt("PlayerIndex", PlayerIdx);
		AddParamInt("TargetPlayerIndex", TargetPlayerIdx);
		AddParamString("PlayerName", StatsObject.PlayerList[PlayerIdx].PlayerName);
		AddParamString("PlayerControllerName", StatsObject.PlayerList[PlayerIdx].ControllerName);
		AddParamString("TargetName", StatsObject.PlayerList[TargetPlayerIdx].PlayerName);
		AddParamString("TargetControllerName", StatsObject.PlayerList[TargetPlayerIdx].ControllerName);
		AddParamFloat("Time", EventTime);
		EndEvent();
    }

	EndLog();
	return true;
}
