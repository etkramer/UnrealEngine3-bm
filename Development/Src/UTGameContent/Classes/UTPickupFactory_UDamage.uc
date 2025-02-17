/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTPickupFactory_UDamage extends UTPowerupPickupFactory;


simulated event InitPickupMeshEffects()
{
	Super.InitPickupMeshEffects();

	// Create a material instance for the pickup
	if (bDoVisibilityFadeIn && MeshComponent(PickupMesh) != None)
	{
		MIC_VisibilitySecondMaterial = MeshComponent(PickupMesh).CreateAndSetMaterialInstanceConstant(1);
		MIC_VisibilitySecondMaterial.SetScalarParameterValue(VisibilityParamName, bIsSuperItem ? 1.f : 0.f);
	}
}


simulated function SetResOut()
{
	Super.SetResOut();

	if (bDoVisibilityFadeIn && MIC_VisibilitySecondMaterial != None)
	{
		MIC_VisibilitySecondMaterial.SetScalarParameterValue(VisibilityParamName, 1.f);
	}
}



defaultproperties
{
	InventoryType=class'UTUDamage'

    PickupStatName=PICKUPS_UDAMAGE

	BaseBrightEmissive=(R=4.0,G=1.0,B=10.0)
	BaseDimEmissive=(R=1.0,G=0.25,B=2.5)

	RespawnSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_UDamage_SpawnCue'

	Begin Object Class=UTParticleSystemComponent Name=DamageParticles
		Template=ParticleSystem'Pickups.UDamage.Effects.P_Pickups_UDamage_Idle'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
		Translation=(X=0.0,Y=0.0,Z=+5.0)
	End Object
	ParticleEffects=DamageParticles
	Components.Add(DamageParticles)

	Begin Object Class=AudioComponent Name=DamageReady
		SoundCue=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_UDamage_GroundLoopCue'
	End Object
	PickupReadySound=DamageReady
	Components.Add(DamageReady)

	bHasLocationSpeech=true
	LocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_HeadingForTheUdamage'
	LocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_HeadingForTheUdamage'
	LocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_HeadingForTheUdamage'
}

