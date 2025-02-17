/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/** this class handles replicating SeqAct_DummyWeaponFire activation. */
class DummyWeaponFireActor extends Actor
	native;

/** owner action */
var SeqAct_DummyWeaponFire FireAction;
/** origin and target actors for the weapon firing */
var Actor OriginActor, TargetActor;
/** incremented after every shot as notification to play effects clientside */
var repnotify int ShotCount;

replication
{
	if (bNetDirty)
		FireAction, OriginActor, TargetActor, ShotCount;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'ShotCount')
	{
		if (FireAction != None && OriginActor != None && TargetActor != None)
		{
			if (FireAction.SpawnedWeapon == None)
			{
				FireAction.SpawnDummyWeapon(OriginActor, TargetActor);
			}
			FireAction.AlignWeaponMuzzleToActor(OriginActor, TargetActor);
			FireAction.SpawnedWeapon.DummyFire(FireAction.FiringMode, TargetActor.Location, OriginActor, FireAction.InaccuracyDegrees, TargetActor);
		}
	}
}

simulated event Tick(float DeltaTime)
{
	if ( WorldInfo.NetMode == NM_Client && FireAction != None && FireAction.SpawnedWeapon != None &&
		OriginActor != None && TargetActor != None )
	{
		// make sure weapon stays aligned so attached effects are in the right place
		FireAction.AlignWeaponMuzzleToActor(OriginActor, TargetActor);
	}
}

simulated event Destroyed()
{
	if (FireAction != None && FireAction.SpawnedWeapon != None)
	{
		FireAction.SpawnedWeapon.WeaponStoppedFiring(FireAction.FiringMode);
		FireAction.SpawnedWeapon.Destroy();
	}

	Super.Destroyed();
}

/** called to set up replication of a shot fired */
event NotifyShotFired(Actor InOriginActor, Actor InTargetActor)
{
	// make sure all necessary properties are up to date
	OriginActor = InOriginActor;
	TargetActor = InTargetActor;
	// trigger the shot
	ShotCount++;
	// make sure we replicate immediately
	bForceNetUpdate = TRUE;
}

defaultproperties
{
	bSkipActorPropertyReplication=true
	bAlwaysRelevant=true
	bReplicateMovement=false
	bUpdateSimulatedPosition=false
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_SimulatedProxy
	NetPriority=2.7
	NetUpdateFrequency=1.0
}
