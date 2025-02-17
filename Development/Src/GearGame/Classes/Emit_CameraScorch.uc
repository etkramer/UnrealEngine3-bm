/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_CameraScorch extends Emit_CameraLensEffectBase
	config(Weapon);


var() MaterialInstanceConstant MIC_Scorch;

var() protected const float MaxFadeVal;

/** True if the scorch was turned on this frame. */
var transient protected bool bTriggered;

/** Current value of the material's fade parameter. */
var protected float				CurFadeVal;
var() protected const float		CurFadeRate;

/** How long to wait after being triggered before starting to fade out. */
var() protected const float		StartFadeDelay;
/** Internal.  Time after which we can start to fade. */
var transient protected float	StartFadeTime;

var() protected const float		FadeRampInTime;


simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	MIC_Scorch = new(outer) class'MaterialInstanceConstant';
	MIC_Scorch.SetParent( Material'COG_Flamethrower.Materials.M_Screen_Effect' );
	ParticleSystemComponent.SetMaterialParameter('HeatDistortionMat', MIC_Scorch);
	bTriggered = TRUE;
}

function Tick(float DeltaTime)
{
	super.Tick(DeltaTime);

	if (bTriggered)
	{
		CurFadeVal += MaxFadeVal * DeltaTime / FadeRampInTime;
		CurFadeVal = FMin(CurFadeVal, MaxFadeVal);
		StartFadeTime = WorldInfo.TimeSeconds + StartFadeDelay;
		bTriggered = FALSE;
	}
	else if (WorldInfo.TimeSeconds > StartFadeTime)
	{
		CurFadeVal -= CurFadeRate * DeltaTime;
		CurFadeVal = FMax(CurFadeVal, 0.f);
	}

	MIC_Scorch.SetScalarParameterValue('Fade', CurFadeVal);

	if (CurFadeVal <= 0.f)
	{
		// we're done
		Destroy();
	}

}

/** Called when this emitter is re-triggered, for bAllowMultipleInstances=FALSE emitters. */
function NotifyRetriggered()
{
	bTriggered = TRUE;
}


defaultproperties
{
	PS_CameraEffect=ParticleSystem'COG_Flamethrower.Effects.P_Camera_FX'
	PS_CameraEffectNonExtremeContent=ParticleSystem'COG_Flamethrower.Effects.P_Camera_FX'

	LifeSpan=0.f		// live until explicitly destroyed

	MaxFadeVal=0.1f
	CurFadeRate=0.15f
	StartFadeDelay=0.5f

	FadeRampInTime=1.f
}



