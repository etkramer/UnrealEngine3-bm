/**
 * Hammer of Dawn Beam.
 * An instance of a HOD beam, handles sequencing of a single shot
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class HOD_TurretBeam extends HOD_Beam
	config(Weapon)
	notplaceable;

var protected transient vector BeamStartLoc;



protected function DoBeamTargetTracking(float DeltaTime)
{
	// do nothing
}

simulated protected function StickToGround(out vector HitNormal, out Actor HitActor)
{
	// do nothing
	HitNormal = vect(0,0,1);
	HitActor = None;
}

simulated function BeginWarmup(vector TargetLoc, optional bool bShouldAutoFire)
{
	bActuallyFiring = FALSE;
	bAutoFire = bShouldAutoFire;

	if (PreWarmupTimeSec > 0.f)
	{
		SetTimer( PreWarmupTimeSec, FALSE, nameof(NotifyPreWarmupFinished) );
	}
	else
	{
		NotifyPreWarmupFinished();
	}
}


simulated final function SetEndpoints(vector StartPt, vector EndPt)
{
	BeamStartLoc = StartPt;
	PSC_MainBeam.SetVectorParameter('lock_on', StartPt);
	SetLocation(EndPt);
}

simulated protected function vector GetBeamDirection()
{
	return Normal(BeamStartLoc - Location);
}

simulated protected function float GetBeamLength()
{
	return VSize(BeamStartLoc - Location);
}


defaultproperties
{
	// main beam effects
	Begin Object Name=PSC_MainBeam0
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_Turret_Beam'
	End Object

	PS_TendrilBeam=None
	PS_TendrilImpact=None
}