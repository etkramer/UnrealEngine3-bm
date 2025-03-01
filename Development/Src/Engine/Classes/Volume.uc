//=============================================================================
// Volume:  a bounding volume
// touch() and untouch() notifications to the volume as actors enter or leave it
// enteredvolume() and leftvolume() notifications when center of actor enters the volume
// pawns with bIsPlayer==true  cause playerenteredvolume notifications instead of actorenteredvolume()
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Volume extends Brush
	native
	nativereplication;

/** Should pawns be forced to walk when inside this volume? */
var() bool bForcePawnWalk;
/** Should process all actors within this volume */
var() bool bProcessAllActors;


cpptext
{
	INT Encompasses(FVector point);
	void SetVolumes();
	virtual void SetVolumes(const TArray<class AVolume*>& Volumes);
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	virtual UBOOL IsAVolume() const {return TRUE;}
	virtual AVolume* GetAVolume() { return this; }
	virtual INT* GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
}

native noexport function bool Encompasses(Actor Other); // returns true if center of actor is within volume

/**	Handling Toggle event from Kismet. */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if (Action.InputLinks[0].bHasImpulse)
	{
		if(!bCollideActors)
		{
			SetCollision(true, bBlockActors);
		}
	}
	// Turn OFF
	else if (Action.InputLinks[1].bHasImpulse)
	{
		if(bCollideActors)
		{
			SetCollision(false, bBlockActors);
		}
	}
	// Toggle
	else if (Action.InputLinks[2].bHasImpulse)
	{
		SetCollision(!bCollideActors, bBlockActors);
	}

	ForceNetRelevant();

	SetForcedInitialReplicatedProperty(Property'Engine.Actor.bCollideActors', (bCollideActors == default.bCollideActors));
}

simulated event CollisionChanged()
{
	// rigid body collision should match Unreal collision
	CollisionComponent.SetBlockRigidBody(bCollideActors && bBlockActors);
}

event ProcessActorSetVolume( Actor Other );

cpptext
{
	virtual void PostEditImport();
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=true
		bAcceptsLights=true
		LightingChannels=(Dynamic=TRUE,bInitialized=TRUE)
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=true
		BlockRigidBody=false
		AlwaysLoadOnClient=True
		AlwaysLoadOnServer=True
	End Object

	bCollideActors=True
	bSkipActorPropertyReplication=true
}
