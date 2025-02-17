/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearGameMessage extends GameMessage
	abstract;

static function ClientReceive( PlayerController P, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1,
				optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	local GearPC PC;

	// no messages in the menu level
	if (class'WorldInfo'.static.IsMenuLevel())
	{
		return;
	}

	// only display leave and join messages on HUD
	if (Switch == 4 || Switch == 1)
	{
		PC = GearPC(P);

		// only show join messages during the match
		if (Switch == 1 && (GearGRI(P.WorldInfo.GRI).GameStatus == GS_PreMatch || GearGRI(P.WorldInfo.GRI).GameStatus == GS_None || GearGRI(P.WorldInfo.GRI).GameStatus == GS_WaitingForHost))
		{
			return;
		}

		if (PC != None && PC.MyGearHUD != None)
		{
			PC.MyGearHUD.AddPlayerJoinMessage(RelatedPRI_1, static.GetString(Switch, false, RelatedPRI_1, RelatedPRI_2, OptionalObject));
		}
	}
}
