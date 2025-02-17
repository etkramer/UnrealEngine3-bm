// Actor used by matinee (SeqAct_Interp) objects to replicate activation, playback, and other relevant flags to net clients
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
class MatineeActor extends Actor
	native
	nativereplication;

cpptext
{
	virtual INT* GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
	virtual void TickSpecial(FLOAT DeltaTime);
	virtual void PreNetReceive();
	virtual void PostNetReceive();
}

/** the SeqAct_Interp associated with this actor (this is set in C++ by the action that spawns this actor)
 *	on the client, the MatineeActor will tick this SeqAct_Interp and notify the actors it should be affecting
 */
var const SeqAct_Interp InterpAction;
/** properties that may change on InterpAction that we need to notify clients about, since the object's properties will not be replicated */
var bool bIsPlaying, bReversePlayback, bPaused;
var float PlayRate;
var float Position;

replication
{
	if (bNetInitial && Role == ROLE_Authority)
		InterpAction;

	if (bNetDirty && Role == ROLE_Authority)
		bIsPlaying, bReversePlayback, bPaused, PlayRate, Position;
}

/** called by InterpAction when significant changes occur. Updates replicated data. */
event Update()
{
	bIsPlaying = InterpAction.bIsPlaying;
	bReversePlayback = InterpAction.bReversePlayback;
	bPaused = InterpAction.bPaused;
	PlayRate = InterpAction.PlayRate;
	Position = InterpAction.Position;
	bForceNetUpdate = TRUE;

	if ( bIsPlaying && InterpAction != None &&
		//@HACK: workaround for LD changing playrate while matinee is running, which isn't simulated correctly
		(InterpAction.Name != 'SeqAct_Interp_5' || !(PathName(InterpAction) ~= "SP_Closure_04_S.TheWorld:PersistentLevel.Main_Sequence.SeqAct_Interp_5")) )
	{
		SetTimer(1.0, true, nameof(CheckPriorityRefresh));
	}
	else
	{
		ClearTimer(nameof(CheckPriorityRefresh));
	}
}

/** check if we should perform a network positional update of this matinee
 * to make sure it's in sync even if it hasn't had significant changes
 * because it's really important (e.g. a player is standing on it or being controlled by it)
 */
function CheckPriorityRefresh()
{
	local Controller C;
	local int i;

	if( InterpAction != None )
	{
		// check if it has a director group - if so, it's controlling the camera, so it's important
		for (i = 0; i < InterpAction.GroupInst.length; i++)
		{
			if (InterpGroupInstDirector(InterpAction.GroupInst[i]) != None)
			{
				bNetDirty = true;
				bForceNetUpdate = true;
				return;
			}
		}

		// check if it is controlling a player Pawn, or a platform a player Pawn is standing on
		foreach WorldInfo.AllControllers(class'Controller', C)
		{
			if ( C.bIsPlayer && C.Pawn != None &&
				( InterpAction.LatentActors.Find(C.Pawn) != INDEX_NONE ||
					(C.Pawn.Base != None && InterpAction.LatentActors.Find(C.Pawn.Base) != INDEX_NONE) ) )
			{
				bNetDirty = true;
				bForceNetUpdate = true;
				return;
			}
		}
	}
}

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	bSkipActorPropertyReplication=true
	bAlwaysRelevant=true
	bReplicateMovement=false
	bUpdateSimulatedPosition=false
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_SimulatedProxy
	NetPriority=2.7
	NetUpdateFrequency=1.0
	Position=-1.0
	PlayRate=1.0
}
