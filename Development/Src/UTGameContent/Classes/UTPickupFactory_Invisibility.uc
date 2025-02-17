/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTPickupFactory_Invisibility extends UTPowerupPickupFactory;

defaultproperties
{
	InventoryType=class'UTGameContent.UTInvisibility'

    PickupStatName=PICKUPS_INVISIBILITY

	BaseBrightEmissive=(R=4.0,G=4.0,B=3.0)
	BaseDimEmissive=(R=0.5,G=0.5,B=0.25)

	RespawnSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Invisibility_SpawnCue'

	Begin Object Class=AudioComponent Name=InvisReady
		SoundCue=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Invisibility_GroundLoopCue'
	End Object
	PickupReadySound=InvisReady
	Components.Add(InvisReady)

	Begin Object Class=UTParticleSystemComponent Name=InvisParticles
		Template=ParticleSystem'Pickups.Invis.Effects.P_Pickups_Invis_Idle'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
		Translation=(X=0.0,Y=0.0,Z=-20.0)
	End Object
	ParticleEffects=InvisParticles
	Components.Add(InvisParticles)

	bHasLocationSpeech=true
	LocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_HeadingForTheInvisibility'
	LocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_HeadingForTheInvisibility'
	LocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_HeadingForTheInvisibility'
}


