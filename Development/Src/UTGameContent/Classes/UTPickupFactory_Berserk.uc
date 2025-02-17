/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTPickupFactory_Berserk extends UTPowerupPickupFactory;

defaultproperties
{
	InventoryType=class'UTBerserk'

    PickupStatName=PICKUPS_BERSERK

	BaseBrightEmissive=(R=50.0,G=1.0)
	BaseDimEmissive=(R=5.0,G=0.1)

	RespawnSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Berzerk_SpawnCue'

	Begin Object Class=UTParticleSystemComponent Name=BerserkParticles
		Template=ParticleSystem'Pickups.Berserk.Effects.P_Pickups_Berserk_Idle'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
		Translation=(X=0.0,Y=0.0,Z=+5.0)
	End Object
	ParticleEffects=BerserkParticles
	Components.Add(BerserkParticles)

	Begin Object Class=AudioComponent Name=BerserkReady
		SoundCue=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Berzerk_GroundLoopCue'
	End Object
	PickupReadySound=BerserkReady
	Components.Add(BerserkReady)

	bHasLocationSpeech=true
	LocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_HeadingForTheBerserk'
	LocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_HeadingForTheBerserk'
	LocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_HeadingForTheBerserk'
}

