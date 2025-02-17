/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearSpawner extends Actor
	native;

/** Is this spawner active? */
var() bool bActive;

/** Should this spawner automatically de-activate when factories are complete? */
var() bool bAutoDeActivate;

/** List of factories using this spawner */
var array<SeqAct_AIFactory> Factories;

enum EHoleEmergeAnim
{
	EHEA_Random,
	EHEA_CrawlUp,
	EHEA_Jump,
};


struct native SpawnerSlot
{
	var() bool bEnabled;
	/** Pawn that is currently using this slot */
	var GearPawn SpawningPawn;
	/** Offsets to use when spawning */
	var() vector LocationOffset;
	var() rotator RotationOffset;
	/** The emerge anim to do **/
	var() EHoleEmergeAnim EmergeAnim;

	structdefaultproperties
	{
		EmergeAnim=EHEA_Random
		bEnabled=TRUE
	}
};
var() editinline array<SpawnerSlot> SpawnSlots;

/** Actor that killed the spawner */
var Actor Killer;

struct CheckpointRecord
{
	var array<string> FactoryPathNames;
};

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	bActive = true;

	Super.Reset();
}

/**
 * Called when a new pawn is spawned using this spawner,
 * use to handle any special behaviors, such as playing
 * an animation, etc.
 */
event HandleSpawn(GearPawn NewSpawn, int SlotIdx)
{
	SpawnSlots[SlotIdx].SpawningPawn = NewSpawn;
}

/**
 * Returns the first available spawn slot index, or -1 if
 * none are available.
 */
event bool GetSpawnSlot(out int out_SpawnSlotIdx, out vector out_SpawnLocation, out rotator out_SpawnRotation)
{
	local int BaseIdx, Idx;
	local array<int> Indices;
	if (bActive)
	{
		// do a quick shuffle of the slots
		Indices[Indices.Length] = RandRange(0,SpawnSlots.Length-1);
		BaseIdx = 0;
		Idx = Indices[BaseIdx];
		while (--Idx >= 0)
		{
			if (FRand() > 0.5f)
			{
				Indices.Insert(BaseIdx+1,1);
				Indices[BaseIdx+1] = Idx;
			}
			else
			{
				Indices.Insert(BaseIdx,1);
				Indices[BaseIdx] = Idx;
				BaseIdx++;
			}
		}
		Idx = Indices[BaseIdx];
		while (++Idx < SpawnSlots.Length)
		{
			if (FRand() > 0.5f)
			{
				Indices.Insert(BaseIdx+1,1);
				Indices[BaseIdx+1] = Idx;
			}
			else
			{
				Indices.Insert(BaseIdx,1);
				Indices[BaseIdx] = Idx;
				BaseIdx++;
			}
		}
		// and then pick the first available one
		for (BaseIdx = 0; BaseIdx < Indices.Length; BaseIdx++)
		{
			Idx = Indices[BaseIdx];
			if (SpawnSlots[Idx].bEnabled && (SpawnSlots[Idx].SpawningPawn == None || SpawnSlots[Idx].SpawningPawn.Health <= 0 || !SpawnSlots[Idx].SpawningPawn.bSpawning))
			{
				out_SpawnLocation = Location + (SpawnSlots[Idx].LocationOffset >> Rotation);
				out_SpawnRotation = Rotation + SpawnSlots[Idx].RotationOffset;
				out_SpawnSlotIdx = Idx;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
 * Simple toggle of this spawner, also handles notifying any
 * associated Kismet AI factories that were using this spawner.
 */
function OnToggle(SeqAct_Toggle Action)
{
	local bool bWasActive;
	bWasActive = bActive;
	if (Action.InputLinks[0].bHasImpulse)
	{
		bActive = TRUE;
	}
	else
	if (Action.InputLinks[1].bHasImpulse)
	{
		bActive = FALSE;
	}
	else
	if (Action.InputLinks[2].bHasImpulse)
	{
		bActive = !bActive;
	}
	// send activated/deactivated notification
	if (bActive && !bWasActive)
	{
		Activated();
	}
	else
	if (!bActive && bWasActive)
	{
		DeActivated();
	}
}

/**
 * Called when this spawner is activated (via Toggle).
 */
function Activated()
{
}

/**
 * Called when this spawner is deactivated (via Toggle, or auto deactivate).
 */
function DeActivated()
{
	local int Idx;
	for (Idx = 0; Idx < Factories.Length; Idx++)
	{
		if (Factories[Idx] != None)
		{
			Factories[Idx].NotifySpawnerDisabled(self);
		}
	}
}

/**
 * Register a factory with this spawner so that notifications
 * can be sent in case of being disabled.
 */
event RegisterFactory(SeqAct_AIFactory Factory)
{
	local int Idx;
	Idx = Factories.Find(Factory);
	if (Idx == -1)
	{
		Factories[Factories.Length] = Factory;
	}
}

/**
 * Unregister a factory from this spawner.
 */
event UnRegisterFactory(SeqAct_AIFactory Factory)
{
	local int Idx;
	Idx = Factories.Find(Factory);
	if (Idx != -1)
	{
		Factories.Remove(Idx,1);
	}
	// if set to autodeactivate
	if (bAutoDeActivate)
	{
		// then check to see if all factories are off
		if (Factories.Length == 0)
		{
			bActive = FALSE;
			DeActivated();
		}
	}
}

/**
 * This is called when a spawner is DYING (like a Seeder or an E-Hole).
 * We fire an event to let LDs know when the spawner is DYING.
 */
function StartingToDie()
{
	TriggerEventClass( class'SeqEvt_SpawnerDying', Killer, 0 );
}

/**
 * This is called when a spawner is actually DEAD (like a Seeder or an E-Hole).
 * We fire an event to let LDs know when the spawner is finally DEAD.
 */
function FinishedDying()
{
	TriggerEventClass( class'SeqEvt_SpawnerDying', Killer, 1 );
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	local int i;

	for (i = 0; i < Factories.length; i++)
	{
		Record.FactoryPathNames[i] = PathName(Factories[i]);
	}
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	local int i;

	for (i = 0; i < Record.FactoryPathNames.length; i++)
	{
		Factories[i] = SeqAct_AIFactory(FindObject(Record.FactoryPathNames[i], class'SeqAct_AIFactory'));
	}
}

defaultproperties
{
	bActive=TRUE
	bAutoDeActivate=TRUE
	SpawnSlots(0)=(LocationOffset=(X=0,Y=0,Z=0))

	SupportedEvents.Add(class'SeqEvt_SpawnerDying')

	bNoDelete=TRUE
}
