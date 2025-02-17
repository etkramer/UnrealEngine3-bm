/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkeletalMeshActor_TortureBarge_C extends SkelMeshActor_Unfriendly
	placeable;

/** RearLeft, RearRight, FrontLeft, FrontRight */
var	AnimNodeSlot	LegSlot[4];

var AnimNodeSlot	OverlaySlot;

var AnimNodeSlot	FullBodySlot;

var	AnimNodeSlot	BreathSlot;

/** How often the small hurt anim can be played */
var()	float		MinHurtAnimInterval;
var		float		NextHurtAnimTime;

var() name			LegTopName[4];
var() name			LegHideBoneName[4];

var repnotify byte	LegHurtState[4];
var byte			OldLegHurtState[4];

/** Health of leg */
var() int			LegHealth[4];
/** If barge is currently vulnerable to damage */
var() bool			bVulnerable;

var() int			LegsHurtBeforeDeath;

replication
{
	if(Role == ROLE_Authority)
		LegHurtState;
}

/** Used to grab nodes we will need */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	Super.PostInitAnimTree(SkelComp);

	if(SkelComp == SkeletalMeshComponent)
	{
		LegSlot[0] = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('RearLeftLeg'));
		LegSlot[1] = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('RearRightLeg'));
		LegSlot[2] = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('FrontLeftLeg'));
		LegSlot[3] = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('FrontRightLeg'));

		BreathSlot = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('BreathSlot'));

		OverlaySlot = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('OverlaySlot'));
		FullBodySlot = AnimNodeSlot(SkeletalMeshComponent.FindAnimNode('FullBody'));
	}
}

/** Handle replicated events */
simulated function ReplicatedEvent(name VarName)
{
	local bool bPlayPain;
	local int i;

	if(VarName == 'LegHurtState')
	{
		for(i=0; i<4; i++)
		{
			if(LegHurtState[i] != OldLegHurtState[i])
			{
				LegStateChanged(i);
				OldLegHurtState[i] = LegHurtState[i];
				bPlayPain = TRUE; // Also play the pain anim
			}
		}

		if(bPlayPain)
		{
			PlayHitPain();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Use toggle action to control if it can be hurt */
simulated function OnToggle(SeqAct_Toggle action)
{
	if (action.InputLinks[0].bHasImpulse)
	{
		bVulnerable = TRUE;
	}
	else if (action.InputLinks[1].bHasImpulse)
	{
		bVulnerable = FALSE;
	}
	else if (action.InputLinks[2].bHasImpulse)
	{
		bVulnerable = !bVulnerable;
	}
}

/** Util to see if a bone it relevant to a particular damage bone */
simulated function bool BoneForDamage(name Bone, name Parent)
{
	return (Bone == Parent || SkeletalMeshComponent.BoneIsChildOf(Bone, Parent));
}

/** See if area we are aiming at should make crosshair red. */
simulated function bool HitAreaIsUnfriendly(TraceHitInfo HitInfo)
{
	local int i;

	for(i=0; i<4; i++)
	{
		if(BoneForDamage(HitInfo.BoneName, LegTopName[i]))
		{
			return TRUE;
		}

	}

	return FALSE;
}

/** Take damage, and fire leg anims as necessary */
event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int i, DeadLegCount;
	local bool bPlayPain;

	Super.TakeDamage( DamageAmount, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );

	if(ROLE == ROLE_Authority && bVulnerable)
	{
		for(i=0; i<4; i++)
		{
			if((LegHurtState[i] == 0) && BoneForDamage(HitInfo.BoneName, LegTopName[i]) && (LegHealth[i] > 0))
			{
				LegHealth[i] -= DamageAmount;
				if(LegHealth[i] <= 0)
				{
					LegHurtState[i] = 1;
					LegStateChanged(i);

					// Fire kismet event
					TriggerEventClass( class'SeqEvt_TortureBargeLegHurt', self, i );

					bPlayPain = TRUE;
				}
			}

			if(LegHurtState[i] == 1)
			{
				DeadLegCount++;
			}
		}

		if(bPlayPain)
		{
			PlayHitPain();
		}

		// Fire death event once 3 legs are knocked out
		if(DeadLegCount >= LegsHurtBeforeDeath)
		{
			TriggerEventClass( class'SeqEvent_Death', self, 0 );
		}
	}

	// Play random pain anim if you hit it anywhere
	if(WorldInfo.TimeSeconds > NextHurtAnimTime && (FRand() < 0.2))
	{
		BreathSlot.PlayCustomAnim('TB_BreathingHitreact', 1.0, 0.2, 0.2, FALSE, TRUE);
		NextHurtAnimTime = WorldInfo.TimeSeconds + MinHurtAnimInterval;
	}
}


/** Called when an anim ends */
simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	local AnimNodeSlot EndedSlot;

	// See if it was a leg slot that finished
	EndedSlot = AnimNodeSlot(SeqNode.ParentNodes[0]);
	if(EndedSlot != None && (EndedSlot == LegSlot[0] || EndedSlot == LegSlot[1] || EndedSlot == LegSlot[2] || EndedSlot == LegSlot[3]))
	{
		// If it is - start the hurt idle anim
		EndedSlot.PlayCustomAnim('TB_CL_LegsDamagedIdle', 1.0, 0.1, -1.0, TRUE, TRUE);
	}
}

/** When leg is hurt, play anim */
simulated function LegStateChanged(INT LegIndex)
{
	local int HideBoneIndex;

	// Play anim!
	LegSlot[LegIndex].PlayCustomAnim('TB_CL_LegsHitReact', 1.0, 0.2, 0.1, FALSE, TRUE);
	LegSlot[LegIndex].SetActorAnimEndNotification(TRUE);

	// Hide leg!
	HideBoneIndex = SkeletalMeshComponent.MatchRefBone(LegHideBoneName[LegIndex]);
	if(HideBoneIndex != INDEX_NONE)
	{
		SkeletalMeshComponent.HideBone(HideBoneIndex, FALSE);
	}
}

/** Play a pain anim */
simulated function PlayHitPain()
{
	OverlaySlot.PlayCustomAnim('ADD_TB_CL_BodyHitReact', 1.0, 0.1, 0.2, FALSE, TRUE);
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'Locust_TortureBarge.Mesh.Locust_TortureBarge_SK'
		PhysicsAsset=PhysicsAsset'Locust_TortureBarge.Mesh.Locust_TortureBarge_SK_Physics'
		AnimTreeTemplate=AnimTree'Locust_TortureBarge.AnimTrees.Locust_Torture_Barge_Closure_AnimTree'
		AnimSets.Add(AnimSet'Locust_TortureBarge.Anims.Locust_Torture_Barge_Animset_Closure')

		CastShadow=TRUE
		bCastDynamicShadow=TRUE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE

		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockRigidBody=TRUE
		BlockNonzeroExtent=TRUE
		CollideActors=TRUE
	End Object

	LegTopName[0]="TB_RearLeftLeg02"
	LegTopName[1]="TB_RearRightLeg02"
	LegTopName[2]="TB_FrontLeftLeg02"
	LegTopName[3]="TB_FrontRightLeg02"

	LegHideBoneName[0]="TB_RearLeftLeg03"
	LegHideBoneName[1]="TB_RearRightLeg03"
	LegHideBoneName[2]="TB_FrontLeftLeg03"
	LegHideBoneName[3]="TB_FrontRightLeg03"

	LegHealth[0]=2000
	LegHealth[1]=2000
	LegHealth[2]=2000
	LegHealth[3]=2000

	MinHurtAnimInterval=2.0

	LegsHurtBeforeDeath=3

	bVulnerable=TRUE

	SupportedEvents.Add(class'SeqEvent_Death')
	SupportedEvents.Add(class'SeqEvt_TortureBargeLegHurt')

	RemoteRole=ROLE_SimulatedProxy
}