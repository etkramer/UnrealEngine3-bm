/**
 * Hammer of Dawn Manager Base.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class HOD_BeamManagerBase extends Actor
	abstract
	native
	config(Weapon)
	notplaceable;


/** array of references to actively firing beams */
var private array<HOD_BeamBase>		ActiveBeams;

/** Cache up the last FindActiveBeam result for fast access.  Simple caching mechanics because most calls tend to be clustered by Pawn */
var private transient Pawn			LastFABPawn;
var private transient HOD_BeamBase	LastFABBeam;

/** true if we can fire beams, false otherwise */
var bool				bEnabled;

/** Normalized vector pointing **from target to the beam origin** (satellite or whatever).*/
var() const vector		DirectionToDeathRayOriginNorm;

/** True if GUDEvent for disabling was fired, false otherwise.  Invalid when bEnabled is True */
var transient bool		bPlayerNotifiedOfDisabledStatus;

var protected const class<HOD_BeamBase> BeamClass;

replication
{
	if (Role == ROLE_Authority)
		bEnabled;
}

/**
 *
 */
simulated function Tick(float DeltaTime)
{
	local int		ShotIdx;
	local HOD_BeamBase	Beam;

	// clean up dead beams from our tracking array
	for (ShotIdx=0; ShotIdx<ActiveBeams.Length; ++ShotIdx)
	{
		Beam = ActiveBeams[ShotIdx];
		if ( (Beam == None) || (Beam.bDeleteMe) )
		{
			ActiveBeams.Remove(ShotIdx--, 1);
		}
	}

	// clear FindActiveBeam cache each Tick
	LastFABPawn = None;
}


/** Enable/disable beam firing (enable/disable orbital weapons platform). Server only. */
event SetEnabled(bool bNewEnabled, bool bSuppressAlert)
{
	local EGUDActionID GUDSAction;

	if (bEnabled != bNewEnabled)
	{
		bEnabled = bNewEnabled;

		// throw guds event
		if (!bSuppressAlert)
		{
			GUDSAction = bEnabled ? GUDAction_NotifyHODEnabled : GUDAction_NotifyHODDisabled;
			GearGame(WorldInfo.Game).Anya.PlayGUDSAction(GUDSAction);
		}

		bPlayerNotifiedOfDisabledStatus = bEnabled ? FALSE : !bSuppressAlert;
	}
}





/** Sets a new target position for an active beam instigated by the specified pawn. */
simulated function NotifyValidTarget(vector NewTargetPos, Pawn ShotInstigator)
{
	local HOD_BeamBase	Beam;
	local GearAI AI;

	if (bEnabled)
	{
		Beam = FindActiveBeam(ShotInstigator);
		if (Beam != None)
		{
			Beam.UpdateTarget(NewTargetPos);

			// only notify if firing, or half way through warmup
			if (Beam.bActuallyFiring)
			{
				// Notify each controller about HOD threat
				foreach WorldInfo.AllControllers( class'GearAI', AI )
				{
					AI.NotifyHODThreat( ShotInstigator, NewTargetPos );
				}
			}
		}
		else
		{
			BeginBeamWarmup(NewTargetPos, ShotInstigator, TRUE);
		}
	}
}


/** Notifies beam manager that the target for this pawn is no longer valid */
simulated function NotifyInvalidTarget(Pawn ShotInstigator)
{
	local HOD_BeamBase	Beam;

	Beam = FindActiveBeam(ShotInstigator);
	if (Beam != None)
	{
		// let beam know that it lost it's target so it can deal
		Beam.NotifyInvalidTarget();
	}

}


/** Tell Beam Manager that Instigator has requested current shot to be terminated. */
simulated function NotifyStopFire(Pawn ShotInstigator)
{
	local HOD_BeamBase	Beam;

	Beam = FindActiveBeam(ShotInstigator);
	if (Beam != None)
	{
		// let beam know that it must abort
		Beam.AbortFire();
	}
}




/** Returns reference to an active beam instigated by this pawn, or None if none exists */
simulated private function HOD_BeamBase FindActiveBeam(Pawn P)
{
	local int BeamIdx;

	if (P != None)
	{
		if (LastFABPawn == P)
		{
			// use cached result
			return LastFABBeam;
		}

		for (BeamIdx=0; BeamIdx<ActiveBeams.Length; ++BeamIdx)
		{
			if ( ActiveBeams[BeamIdx].WasInstigatedBy(P) && !ActiveBeams[BeamIdx].bDormant)
			{
				LastFABPawn = P;
				LastFABBeam = ActiveBeams[BeamIdx];
				return LastFABBeam;
			}
		}
	}


	return None;
}


/**
* Initiate a shot from the HoD satellite.
* Spawns beam, and begins warmup cycle.  Beam will nto fire until
* Returns true if shot began successfully, false if denied.
* Server only.
*/
function bool BeginBeamWarmup(vector InitTargetLoc, Pawn ShotInstigator, optional bool bAutoFire)
{
	local HOD_BeamBase	Beam;

	if (Role == ROLE_Authority)
	{
		// spawn a new beam and get it ready to shoot
		Beam = Spawn(BeamClass);
		if (Beam != None)
		{
			ActiveBeams[ActiveBeams.Length] = Beam;
			Beam.Init(ShotInstigator);
			Beam.BeginWarmup(InitTargetLoc, bAutoFire);
			return TRUE;
		}
	}

	return FALSE;
}


/**
* Causes this instigator's beam to actually fire the main beam.
* Returns true if it fired, false if it didn't.
*/
simulated function bool FireBeam(Pawn ShotInstigator)
{
	local HOD_BeamBase		Beam;

	Beam = FindActiveBeam(ShotInstigator);
	if (Beam != None)
	{
		return Beam.Fire();
	}

	return FALSE;
}


/** Returns TRUE if the beam for this pawn is actually firing */
simulated function bool IsBeamFiring(Pawn ShotInstigator)
{
	local HOD_BeamBase		Beam;

	Beam = FindActiveBeam(ShotInstigator);
	if (Beam != None)
	{
		return Beam.bActuallyFiring;
	}

	return FALSE;
}


/** Returns TRUE if the beam for this pawn is actually firing */
simulated function bool IsBeamCoolingDown(Pawn ShotInstigator)
{
	local HOD_BeamBase		Beam;

	Beam = FindActiveBeam(ShotInstigator);
	if (Beam != None)
	{
		return !Beam.bActuallyFiring && !Beam.IsWarmingUp();
	}

	return FALSE;
}

/** Does a hard reset of the HOD stuff.  Kills all beams, etc. */
simulated function ResetAll()
{
	local int BeamIdx;

	for (BeamIdx=0; BeamIdx<ActiveBeams.Length; ++BeamIdx)
	{
		ActiveBeams[BeamIdx].AbortFire();
	}

	ActiveBeams.Length = 0;
}




/**
* Returns true if the given impact info represents a valid target
* for the deathray of the gods.
*/
simulated native function bool IsAValidHODTarget(const out vector Loc);




defaultproperties
{
	// stuff from actor
	bMovable=false
	bCanBeDamaged=false
	bReplicateMovement=false
	bEnabled=true
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true

	DirectionToDeathRayOriginNorm=(X=0,Y=0,Z=1)
}
