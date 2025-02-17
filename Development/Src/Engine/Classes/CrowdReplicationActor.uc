/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class CrowdReplicationActor extends Actor
	native
	nativereplication;

/** Pointer to crowd spawning action we are replicating. */
var	repnotify	SeqAct_CrowdSpawner	Spawner;
/** If crowd spawning is active. */
var	repnotify	bool				bSpawningActive;
/** Use to replicate when we want to destroy all crowd agents. */
var repnotify	int					DestroyAllCount;

replication
{
	if(Role == Role_Authority)
		Spawner, bSpawningActive, DestroyAllCount;
}

cpptext
{
	virtual INT* GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
};

simulated event ReplicatedEvent(name VarName)
{
	if(VarName == 'Spawner' || VarName == 'bSpawningActive')
	{
		if(Spawner != None)
		{
			Spawner.bSpawningActive = bSpawningActive;

			// Cache spawner kismet vars on client
			if(bSpawningActive)
			{
				Spawner.CacheSpawnerVars();
			}
		}
	}
	else if(VarName == 'DestroyAllCount')
	{
		Spawner.KillAgents();

		Spawner.bSpawningActive = FALSE;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated event Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);

	if(Role < ROLE_Authority && Spawner != None && Spawner.bSpawningActive)
	{
		Spawner.UpdateSpawning(DeltaTime);
	}
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	bSkipActorPropertyReplication=true
	bAlwaysRelevant=true
	bReplicateMovement=false
	bUpdateSimulatedPosition=false
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_SimulatedProxy
	NetPriority=2.7
	NetUpdateFrequency=1.0
}