/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Emit_AnnexArc extends SpawnedGearEmitter;

/** The color of the effect */
var transient repnotify Color AnnexColor;
/** The value of the progression of the effect from 0.0f - 1.0f */
var transient repnotify float AnnexProgression;

replication
{
	if( Role == ROLE_Authority )
		AnnexColor, AnnexProgression;
}

simulated event ReplicatedEvent( name VarName )
{
	switch( VarName )
	{
		case 'AnnexColor':
			SetAnnexColor( AnnexColor );
			break;

		case 'AnnexProgression':
			SetAnnexProgression( AnnexProgression );
			break;
	}
}

simulated function SetAnnexColor( Color NewColor )
{
	local Vector VC;

	AnnexColor = NewColor;
	VC.X = float(AnnexColor.R) / 255.f;
	VC.Y = float(AnnexColor.G) / 255.f;
	VC.Z = float(AnnexColor.B) / 255.f;
	SetVectorParameter( 'AnnexColor', VC );
}

simulated function SetAnnexProgression( float NewProgression )
{
	AnnexProgression = NewProgression;
	SetFloatParameter( 'AnnexAlpha', AnnexProgression );
}

defaultproperties
{
	ParticleSystem=ParticleSystem'Gear_Annex_Effects.Effects.P_MP_Annex_Ring'
	bDestroyOnSystemFinish=FALSE
	bAlwaysRelevant=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=TRUE
	LifeSpan=0
}