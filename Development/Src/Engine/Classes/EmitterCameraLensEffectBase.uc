/**
 * Base class for Camera Lens Effects.  Needed so we can have AnimNotifies be able to show camera effects
 * in a nice drop down
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class EmitterCameraLensEffectBase extends Emitter
	abstract
	dependson(Emitter)
	native(Particle);

/** Particle System to use */
var protected ParticleSystem PS_CameraEffect;

/** The effect to use for non extreme content **/
var protected ParticleSystem PS_CameraEffectNonExtremeContent;


simulated function PostBeginPlay()
{
	ParticleSystemComponent.SetDepthPriorityGroup(SDPG_Foreground);

	Super.PostBeginPlay();

	ActivateLensEffect();
}


/** This will actually activate the lens Effect.  We want this separated from PostBeginPlay so we can cache these emitters **/
simulated function ActivateLensEffect()
{
	local ParticleSystem PSToActuallySpawn;

	// only play the camera effect on clients
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// 
		if( WorldInfo.GRI.ShouldShowGore() )
		{
			PSToActuallySpawn = PS_CameraEffect;
		}
		else
		{
			PSToActuallySpawn = PS_CameraEffectNonExtremeContent;
		}

		if( PSToActuallySpawn != None )
		{
			SetTemplate( PS_CameraEffect, bDestroyOnSystemFinish );
		}
	}
}



DefaultProperties
{
	// makes sure I tick after the camera
	TickGroup=TG_PostAsyncWork

	bDestroyOnSystemFinish=TRUE
	bNetInitialRotation=TRUE
	bNoDelete=FALSE

	//bOnlyOwnerSee=TRUE
	Begin Object Name=ParticleSystemComponent0
		bOnlyOwnerSee=TRUE
	End Object

	Components.Remove(ArrowComponent0)
	Components.Remove(Sprite)
}