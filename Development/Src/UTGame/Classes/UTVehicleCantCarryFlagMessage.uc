/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVehicleCantCarryFlagMessage extends UTLocalMessage;

var localized string FlagMessage;
var SoundNodeWave FlagAnnouncement;
var localized string OrbMessage;
var SoundNodeWave OrbAnnouncement;

static simulated function ClientReceive( PlayerController P, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1,
						optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);

	UTPlayerController(P).PlayAnnouncement(default.class, Switch);
}

static function SoundNodeWave AnnouncementSound(int MessageIndex, Object OptionalObject, PlayerController PC)
{
	return (MessageIndex == 0) ? default.FlagAnnouncement : default.OrbAnnouncement;
}

static function byte AnnouncementLevel(byte MessageIndex)
{
	return 2;
}

static function string GetString( optional int Switch, optional bool bPRI1HUD, optional PlayerReplicationInfo RelatedPRI_1,
					optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	return (Switch == 0) ? default.FlagMessage : default.OrbMessage;
}

defaultproperties
{
	FlagAnnouncement=SoundNodeWave'A_Announcer_Status.Status.A_StatusAnnouncer_YouCannotCarryTheFlagInThisVehicle'
	OrbAnnouncement=SoundNodeWave'A_Announcer_Status.Status.A_StatusAnnouncer_YouCannotCarryTheOrbInThisVehicle'

	bIsUnique=false
	FontSize=1
	MessageArea=2
	bBeep=false
	DrawColor=(R=0,G=160,B=255,A=255)
}
