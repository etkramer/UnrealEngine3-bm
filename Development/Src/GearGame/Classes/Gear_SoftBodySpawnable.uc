/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Gear_SoftBodySpawnable extends SkeletalMeshActor
	notplaceable;

/** Automatically clean up actor after a certain time */
var()	bool	bAutoRemove;
/** Actor will be removed after this time if it cannot be seen */
var()	float	RemoveAfterIfNotSeen;
/** Actor will be removed after this time even if it can be seen */
var()	float	ForceRemoveAfter;

/** Soft body physics should start frozen */
var()	bool	bStartFrozen;



/** Used to set timers */
simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Set timers to clean it up
	if(bAutoRemove)
	{
		SetTimer( RemoveAfterIfNotSeen, FALSE, nameof(RemoveIfNotSeen) );
		SetTimer( ForceRemoveAfter, FALSE, nameof(ForceRemove) );
	}

	// Apply bStartFrozen
	SkeletalMeshComponent.SetSoftBodyFrozen(bStartFrozen);
}

/** Will clean up if not seen - otherwise will wait 0.5 secs and try again */
simulated function RemoveIfNotSeen()
{
	// Not seen - clean up now
	if(SkeletalMeshComponent.LastRenderTime < WorldInfo.TimeSeconds - 0.5)
	{
		LifeSpan=0.01;
		ClearTimer('ForceRemove');
	}
	// Is being seen - check again in 0.5 secs
	else
	{
		SetTimer( 0.5, FALSE, nameof(RemoveIfNotSeen) );
	}
}

/** Will force actor to be destroyed */
simulated function ForceRemove()
{
	LifeSpan=0.01;
	ClearTimer('RemoveIfNotSeen');
}

/** If this KAsset receives a Toggle ON event from Kismet, wake the physics up. */
simulated function OnToggle(SeqAct_Toggle action)
{
	local bool bFrozen;

	if(action.InputLinks[0].bHasImpulse)
	{
		bFrozen = FALSE;
	}
	else if(action.InputLinks[1].bHasImpulse)
	{
		bFrozen = TRUE;
	}
	else if(action.InputLinks[2].bHasImpulse)
	{
		bFrozen = !SkeletalMeshComponent.bSoftBodyFrozen;
	}

	// Change frozen state
	SkeletalMeshComponent.SetSoftBodyFrozen(bFrozen);

	// Wake up now if not frozen
	if(!bFrozen)
	{
		SkeletalMeshComponent.WakeSoftBody();
	}
}

defaultproperties
{
	bDamageAppliesImpulse=TRUE
	bNoDelete=FALSE

	bAutoRemove=TRUE

	Begin Object Name=MyLightEnvironment
	    bEnabled=TRUE
	End Object

	Begin Object Name=SkeletalMeshComponent0
		bEnableSoftBodySimulation=TRUE
		BlockRigidBody=TRUE
		SoftBodyRBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE)
	End Object

	RemoveAfterIfNotSeen=5.0
	ForceRemoveAfter=15.0
}
