/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTHeroMessage extends UTLocalMessage
	abstract;

var localized string TooManyHeroesString, HeroNoFitString;
var localized string HeroKillString;
var SoundNodeWave HeroKillSound;

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if ( Switch == 0 )
		return default.TooManyHeroesString;
	else if ( Switch == 2 )
		return default.HeroKillString;
	
	return default.HeroNoFitString;
}

static simulated function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);

	if ( Switch == 2 )
	{
		UTPlayerController(P).PlayAnnouncement(Default.class,Switch );
		UTPlayerController(P).ClientMusicEvent(10);
	}
}

static function SoundNodeWave AnnouncementSound(int MessageIndex, Object OptionalObject, PlayerController PC)
{
	return (MessageIndex == 2) ? default.HeroKillSound : None;
}

defaultproperties
{
	bIsUnique=True
	Lifetime=6
	DrawColor=(R=255,G=0,B=0,A=255)
	 FontSize=3
	 bBeep=False
	 HeroKillSound=SoundNodeWave'A_Announcer_Reward.Rewards.A_RewardAnnouncer_Assassin'
	MessageArea=3
	AnnouncementPriority=8
}
