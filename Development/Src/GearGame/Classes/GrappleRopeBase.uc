/** 
 * Grapple rope object for grappling enemies.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GrappleRopeBase extends Actor;


/** Hack variable for prototype tweaking. */
var() const vector TmpStartClimbOffset;

var() protected const float				FlightTime;

var() protected StaticMeshComponent		HookMesh;
//var() const SkeletalMeshComponent	RopeMesh;

var transient protected Actor	LaunchOriginActor;
var transient Actor				LaunchTargetActor;
var transient protected float	FlightPct;
var transient protected bool	bFlying;

function LaunchTo(Actor OriginActor, Actor TargetActor)
{
	LaunchOriginActor = OriginActor;
	LaunchTargetActor = TargetActor;
	FlightPct = 0.f;
	bFlying = TRUE;
}

simulated function Tick(float DeltaTime)
{
	local vector StartLoc, DestLoc;

	//DrawDebugSphere(Location, 16, 10, 255, 255, 0, FALSE);
	if (bFlying)
	{
		FlightPct += DeltaTime / FlightTime;
		if (FlightPct > 1.f)
		{
			FlightPct = 1.f;
			bFlying = FALSE;		// done
		}

		StartLoc = LaunchOriginActor.Location;
		DestLoc = LaunchTargetActor.Location;
		SetLocation(StartLoc + FlightPct * (DestLoc - StartLoc));

		// finished!
		if ( !bFlying )
		{
			if (LaunchTargetActor.Base != None)
			{
				SetHardAttach(TRUE);
				SetBase(LaunchTargetActor.Base);
			}
			if (GearPawn(Owner) != None)
			{
				GearPawn(Owner).NotifyGrappleRopeIsAttached(self);
			}	

			PlaySound(SoundCue'Ambient_NonLoop.Bell.Bell_Impact_Cue');
		}
	}
	else
	{
		if (LaunchTargetActor != None)
		{
			SetLocation(LaunchTargetActor.Location);
		}
		else
		{
			// the actor we're attached to is gone, we can go away too
			Destroy();
		}
	}

	if ( (LaunchTargetActor != None) && (LaunchOriginActor != None) )
	{
		DrawDebugLine(LaunchOriginActor.Location, Location, 200, 200, 200, FALSE);
	}
}

function bool HookIsAttached()
{
	return ( !bFlying && (LaunchOriginActor != None) );
}

function DetachFromOriginActor()
{
	LaunchOriginActor = None;
	bFlying = FALSE;
}

defaultproperties
{

}