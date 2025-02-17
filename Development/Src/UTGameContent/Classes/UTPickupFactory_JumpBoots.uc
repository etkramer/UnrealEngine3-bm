/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTPickupFactory_JumpBoots extends UTPowerupPickupFactory;

defaultproperties
{
	InventoryType=class'UTGameContent.UTJumpBoots'
	bIsSuperItem=FALSE

    PickupStatName=PICKUPS_JUMPBOOTS

	BaseBrightEmissive=(R=25.0,G=25.0,B=1.0)
	BaseDimEmissive=(R=1.0,G=1.0,B=0.01)
	PivotTranslation=(Y=20.0)

	Begin Object Class=AudioComponent Name=BootsReady
		SoundCue=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_JumpBoots_GroundLoopCue'
	End Object
	PickupReadySound=BootsReady
	Components.Add(BootsReady)
	RespawnSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_JumpBoots_SpawnCue'

	bHasLocationSpeech=true
	LocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_HeadingForTheJumpBoots'
	LocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_HeadingForTheJumpBoots'
	LocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_HeadingForTheJumpBoots'
}
