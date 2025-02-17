/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTCTFGame_Content extends UTCTFGame;

defaultproperties
{
	HUDType=class'UTGame.UTCTFHUD'
	TranslocatorClass=class'UTWeap_Translocator_Content'

	AnnouncerMessageClass=class'UTCTFMessage'
 	TeamScoreMessageClass=class'UTGameContent.UTTeamScoreMessage'
}
