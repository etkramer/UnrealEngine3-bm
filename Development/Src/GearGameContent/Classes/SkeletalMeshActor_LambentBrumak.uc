/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkeletalMeshActor_LambentBrumak extends SkeletalMeshActorMAT
	placeable;

struct native LambentBrumakPlayAnimInfo
{
	var	name		AnimName;
	var	byte		bNewData;
};

/** Struct used to hold state of anim we want to play on brumak - used for replication*/
var		LambentBrumakPlayAnimInfo	AnimRepInfo;

var		AnimNodeSlot				FullBodySlot;

var()	float	PlayAnimBlendInTime;
var()	float	PlayAnimBlendOutTime;

replication
{
	if(Role == ROLE_Authority)
		AnimRepInfo;
}

simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	Super.PostInitAnimTree(SkelComp);

	if(SkelComp == SkeletalMeshComponent)
	{
		FullBodySlot = AnimNodeSlot(SkelComp.FindAnimNode('SlotName'));
	}
}

/** Called from kismet to play an animation. (server only) */
function OnLambentBrumakPlayAnim(SeqAct_LambentBrumakPlayAnim Action)
{
	if(Action.InputLinks[0].bHasImpulse)
	{
		AnimRepInfo.AnimName = Action.AnimName;
		AnimRepInfo.bNewData = 1;
	}
	// 'Stop' is basically playing no anim
	else if(Action.InputLinks[1].bHasImpulse)
	{
		AnimRepInfo.AnimName = '';
		AnimRepInfo.bNewData = 1;
	}
}

simulated function Tick(float DeltaSeconds)
{
	Super.Tick(DeltaSeconds);

	// See if there is an animation to play
	if(AnimRepInfo.bNewData == 1)
	{
		if(AnimRepInfo.AnimName == '')
		{
			FullBodySlot.StopCustomAnim(PlayAnimBlendOutTime);
		}
		else
		{
			FullBodySlot.PlayCustomAnim(AnimRepInfo.AnimName, 1.0, PlayAnimBlendInTime, PlayAnimBlendOutTime, FALSE, TRUE);
		}

		AnimRepInfo.bNewData = 0;
	}
}


defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'Locust_Brumak_Lambent.Lambent_Brumak'
		PhysicsAsset=PhysicsAsset'Locust_Brumak_Lambent.Lambent_Brumak_Physics'
		AnimTreeTemplate=AnimTree'Locust_Brumak_Lambent.Lambent_Brumak_blisters'
		AnimSets(0)=AnimSet'Locust_Brumak_Lambent.Locust_Brumak_Lambent'
		MorphSets(0)=MorphTargetSet'Locust_Brumak_Lambent.Lambent_Brumak_Blister_Morphs'
		LightingChannels=(bInitialized=True,Unnamed_3=True)
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		bUpdateSkelWhenNotRendered=FALSE
	End Object

	DrawScale=0.8

	PlayAnimBlendInTime=0.2
	PlayAnimBlendOutTime=0.3

	bCollideActors=TRUE
	bBlockActors=FALSE

	RemoteRole=ROLE_SimulatedProxy
}
