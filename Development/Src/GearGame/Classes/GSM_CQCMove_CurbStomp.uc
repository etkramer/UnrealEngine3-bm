
/**
 * CQC Move (Close Quarter Combat) CurbStomp
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_CQCMove_CurbStomp extends GSM_CQC_Killer_Base;

/** Screen Shake play on impact */
var	ScreenShakeStruct	ImpactScreenShake;

var Array<CameraAnim>	CameraAnims;

function PlayExecution()
{
	// Play execution Camera Animation.
	if( CameraAnims.Length > 0 )
	{
		PlayExecutionCameraAnim(CameraAnims);
	}

	// Play Face FX Emotion
	if( !PawnOwner.IsActorPlayingFaceFXAnim() )
	{
		PawnOwner.PlayActorFaceFXAnim(None, "Emotions", "Strained", None);
	}

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_CurbStompLongExecutionEffort, true);

	Super.PlayExecution();
}

/** Separate function, so other executions can implement their variations. */
function SetVictimRotation()
{
	// Override rotation.
	VictimDesiredYaw = NormalizeRotAxis(Rotator(-DirToVictim).Yaw + 16384);
	Super.SetVictimRotation();
}

/** Called on a timer when doing a curb stomp. */
function KillVictim()
{
	if( PCOwner != None )
	{
		// Play Force Feed Back effect
		PCOwner.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);

		// Play Screen Shake effect
		//PCOwner.ClientPlayCameraShake(ImpactScreenShake);
	}

	Super.KillVictim();
}

defaultproperties
{
	BS_KillerAnim=(AnimName[BS_FullBody]="CTRL_SmashFace")
	BS_VictimAnim=(AnimName[BS_FullBody]="DBNO_SmashFace")
	VictimDeathTime=1.31f
	VictimRotStartTime=0.42f
	VictimRotInterpSpeed=8.f
	MarkerRelOffset=(X=84.57,Y=27.19,Z=0.0)

	CameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_SmashFace_Cam01'
	CameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_SmashFace_Cam02'
	CameraAnims(2)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_SmashFace_Cam03'
	CameraAnims(3)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_SmashFace_Cam04'

	ImpactScreenShake={(TimeDuration=0.25f,
						RotAmplitude=(X=1500,Y=500,Z=250),
						RotFrequency=(X=10,Y=10,Z=10),
						RotParam=(X=ESP_OffsetRandom,Y=ESP_OffsetRandom,Z=ESP_OffsetRandom),
						LocAmplitude=(X=-10,Y=2,Z=2),
						LocFrequency=(X=10,Y=10,Z=10),
						LocParam=(X=ESP_OffsetZero,Y=ESP_OffsetRandom,Z=ESP_OffsetRandom)
						)}
}
