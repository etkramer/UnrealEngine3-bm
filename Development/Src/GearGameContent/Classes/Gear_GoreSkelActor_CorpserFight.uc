/**
*	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class Gear_GoreSkelActor_CorpserFight extends Gear_GoreSkelActor_Corpser
	placeable;

/** Slot node used to play various animations */
var()	AnimNodeSlot	FullBodySlot;

// Names of anims to play
var()	name	IdleAnim;
var()	name	StartAttackAnim;
var()	name	HoldAttackAnim;
var()	name	StrikeAttackAnim;
var()	name	RetreatAnim;
var()	name	DeathAnim;
var()	float	DeathAnimStartTime;

// Sounds to play
var(Sounds)	SoundCue	PainSound;
var(Sounds)	SoundCue	StartAttackSound;
var(Sounds)	SoundCue	StrikeSound;


/** Keeps track of how many times we have been successfully hurt */
var		int		HurtCount;

/** Centaur we are currently targetting to attack */
var		Vehicle_Centaur_Base TargetCentaur;

/** Used to replicate playing attack effects on client */
var		repnotify int AttackEffectCount;

/** Position (relative to corpser) of trigger for centaur  */
var()	vector	CentaurTriggerPos;
/** Radius of trigger for centaur */
var()	float 	CentaurTriggerRadius;
/** Turns on debug drawing of trigger */
var()	bool	bDrawCentaurTrigger;

/** How long to pause befor attacking - where corpser can be damaged */
var()	float	HoldAttackTime;

/** When the next attack is allowed */
var		float	NextAllowedAttackTime;

/** How much damage to do to centaur if attack is successful */
var()	int		CentaurDamage;
/** Bones to spawn attack effects at */
var()	array<name>	ClawTipBones;
/** Effect to spawn at ClawTipBones */
var()	ParticleSystem AttackHitEffect;
/** Cam anim to play when centaur gets attacked */
var()	CameraAnim	CentaurHitCamAnim;
/** Minimum time between corpser attacks */
var()	vector2d	MinAttackInterval;

enum ECorpserState
{
	ECS_Idle,
	ECS_StartingAttack,
	ECS_HoldingAttack,
	ECS_Strike,
	ECS_Retreat,
	ECS_DeathAnim
};

/** Current state of corpser animation */
var	repnotify	ECorpserState	CorpserState;

replication
{
	if(Role == ROLE_Authority)
		CorpserState, TargetCentaur, AttackEffectCount;
};

/** */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	Super.PostInitAnimTree(SkelComp);

	if(SkelComp == SkeletalMeshComponent)
	{
		FullBodySlot = AnimNodeSlot(SkelComp.FindAnimNode('Slot_FullBody'));
	}

	SkelComp.AnimSets = default.SkeletalMeshComponent.AnimSets;
	SkelComp.UpdateAnimations();

	// Start looping idle anim
	PlayAnimForState(ECS_Idle);
}

/** (client only) */
simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'CorpserState')
	{
		PlayAnimForState(CorpserState);
	}
	else if(VarName == 'AttackEffectCount')
	{
		ClientCorpserAttackEffects();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Plays anim when state changes (client + server) */
simulated function PlayAnimForState(ECorpserState NewState)
{
	local AnimNodeSequence SeqNode;

	if(NewState == ECS_Idle)
	{
		FullBodySlot.PlayCustomAnim(IdleAnim, 1.0, 1.0, 0.0, TRUE, TRUE);
	}
	else if(NewState == ECS_StartingAttack)
	{
		FullBodySlot.PlayCustomAnim(StartAttackAnim, 1.0, 0.2, -1.0, FALSE, TRUE);
		FullBodySlot.SetActorAnimEndNotification(TRUE);
		PlaySound(StartAttackSound, TRUE, TRUE, TRUE, Location, TRUE);
	}
	else if(NewState == ECS_HoldingAttack)
	{
		FullBodySlot.PlayCustomAnim(HoldAttackAnim, 1.0, 0.2, 0.0, TRUE, TRUE);
	}
	else if(NewState == ECS_Strike)
	{
		FullBodySlot.PlayCustomAnim(StrikeAttackAnim, 1.5, 0.1, -1.0, FALSE, TRUE);
		FullBodySlot.SetActorAnimEndNotification(TRUE);
	}
	else if(NewState == ECS_Retreat)
	{
		FullBodySlot.PlayCustomAnim(RetreatAnim, 1.0, 0.1, -1.0, FALSE, TRUE);
		FullBodySlot.SetActorAnimEndNotification(TRUE);
	}
	else if(NewState == ECS_DeathAnim)
	{
		FullBodySlot.PlayCustomAnim(DeathAnim, 1.0, 0.1, -1.0, FALSE, TRUE);
		// Jump forward in anim
		SeqNode = FullBodySlot.GetCustomAnimNodeSeq();
		SeqNode.SetPosition(DeathAnimStartTime, FALSE);
	}

}

/** Used to look for centaurs (server only) */
simulated function Tick(float DeltaTime)
{
	local vector TriggerPos;
	local Vehicle_Centaur_Base Centaur;

	Super.Tick(DeltaTime);

	if(Role == ROLE_Authority)
	{
		// When idling, and long enough after last attack, look for a centaur to stab
		if(CorpserState == ECS_Idle && (WorldInfo.TimeSeconds > NextAllowedAttackTime))
		{
			TargetCentaur = None;

			TriggerPos = Location + (CentaurTriggerPos >> Rotation);

			// Draw sphere to show where check is happening
			if(bDrawCentaurTrigger)
			{
				DrawDebugSphere(TriggerPos, CentaurTriggerRadius, 16, 255,255,0, FALSE);
			}

			// Look for a centaur
			ForEach OverlappingActors(class'GearGame.Vehicle_Centaur_Base', Centaur, CentaurTriggerRadius, TriggerPos)
			{
				if(Centaur.Health > 0)
				{
					TargetCentaur = Centaur;
				}
			}

			// Start the attack
			if(TargetCentaur != None)
			{
				CorpserState = ECS_StartingAttack;
				PlayAnimForState(ECS_StartingAttack);
			}
		}
	}

}

/** (server only) */
function TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int idx;
	local SeqEvent_TakeDamage dmgEvent;

	// server only!
	if(Role != ROLE_Authority || HurtCount >= 3)
	{
		return;
	}

	// Only vulnerable when preparing to strike
	if(CorpserState == ECS_HoldingAttack)
	{
		HurtCount++;

		// Play either retreat or death
		if(HurtCount == 3)
		{
			CorpserState = ECS_DeathAnim;
			PlayAnimForState(ECS_DeathAnim);
		}
		else
		{
			CorpserState = ECS_Retreat;
			PlayAnimForState(ECS_Retreat);
		}

		// Abort any attacking
		ClearTimer('StrikeAttack');

		NextAllowedAttackTime = WorldInfo.TimeSeconds + (MinAttackInterval.X + (FRand() * (MinAttackInterval.Y - MinAttackInterval.X)));

		// Play pain sound
		PlaySound(PainSound, TRUE, TRUE, TRUE, Location, TRUE);

		// search for any damage events
		for (idx = 0; idx < GeneratedEvents.Length; idx++)
		{
			dmgEvent = SeqEvent_TakeDamage(GeneratedEvents[idx]);
			if (dmgEvent != None)
			{
				// notify the event of the damage received
				dmgEvent.HandleDamage(self, EventInstigator, DamageType, Damage);
			}
		}
	}
}

/** Called when various anims end - moves on to next state */
simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	local vector TriggerPos;

	// Server only
	if(Role != ROLE_Authority)
	{
		return;
	}

	// Starting attack -> Hold Attack
	if(CorpserState == ECS_StartingAttack)
	{
		CorpserState = ECS_HoldingAttack;
		PlayAnimForState(ECS_HoldingAttack);

		SetTimer( HoldAttackTime, FALSE, nameof(StrikeAttack) );
	}
	// Strike -> Idle
	else if(CorpserState == ECS_Strike)
	{
		ClientCorpserAttackEffects();
		AttackEffectCount++; // replicate to client

		// Transition back to idle
		CorpserState = ECS_Idle;
		PlayAnimForState(ECS_Idle);

		// Strike finished, actually do damage (if still close enough)
		TriggerPos = Location + (CentaurTriggerPos >> Rotation);
		if(VSize(TargetCentaur.Location - TriggerPos) < CentaurTriggerRadius*1.5)
		{
			TargetCentaur.TakeDamage(CentaurDamage, None, TargetCentaur.Location, Normal(TargetCentaur.Location - Location), class'GDT_Corpser_Smash');
		}
		else
		{
			`log("CENTAUR EVADED CORPSER");
		}

		//Delay before attacking again
		NextAllowedAttackTime = WorldInfo.TimeSeconds + (MinAttackInterval.X + (FRand() * (MinAttackInterval.Y - MinAttackInterval.X)));
	}
	// Retreat -> Idle
	else if(CorpserState == ECS_Retreat)
	{
		CorpserState = ECS_Idle;
		PlayAnimForState(ECS_Idle);
	}
}

/** Play effects that result from corpser attack */
simulated function ClientCorpserAttackEffects()
{
	local int i;
	local vector BoneLoc;
	local Emitter ExplodeEffect;

	PlaySound(StrikeSound, TRUE, TRUE, TRUE, Location, TRUE);

	// Spawn particle effects at each claw location
	for(i=0; i<ClawTipBones.length; i++)
	{
		BoneLoc = SkeletalMeshComponent.GetBoneLocation(ClawTipBones[i]);
		ExplodeEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(AttackHitEffect, BoneLoc, rotator(vect(0,0,1)));
		ExplodeEffect.ParticleSystemComponent.ActivateSystem();
	}

	// Play cam anim
	TargetCentaur.PlayCamAnimOnAllControllers(CentaurHitCamAnim);

}

/** Holding attack -> Strike (server only) */
function StrikeAttack()
{
	CorpserState = ECS_Strike;
	PlayAnimForState(ECS_Strike);

	//Tell Kismet the strike is happening
	TriggerEventClass(class'SeqEvt_CorpserAttack', self);
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		AnimSets(0)=AnimSet'Locust_Corpser.Cavern_AH_AnimSet'
		AnimSets(1)=AnimSet'Locust_Corpser.Cinematic'
		bUpdateSkelWhenNotRendered=TRUE
	End Object

	IdleAnim="Cavern_Idle_Hunkered"
	StartAttackAnim="StabAttack_Beginning_Start"
	HoldAttackAnim="StabAttack_Beginning_Hold"
	StrikeAttackAnim="StabAttack_Strike"
	RetreatAnim="Retreat_Start"
	DeathAnim="Emerge_Scream_6sec_B"
	DeathAnimStartTime=2.6

	StartAttackSound=SoundCue'Locust_Corpser_Efforts.Corpser.CorpserArmAttackRaiseCue'
	PainSound=SoundCue'Locust_Corpser_Efforts.CorpserEfforts.ScreamCue'
	StrikeSound=SoundCue'Locust_Corpser_Efforts.Corpser.CorpserArmAttackSlamCue'

	CentaurTriggerPos=(X=1200.0, Z=100.0)
	CentaurTriggerRadius=512.0
	//bDrawCentaurTrigger=TRUE

	CorpserState=ECS_Idle

	HoldAttackTime=1.0
	MinAttackInterval=(X=3.0,Y=6.0)

	CentaurDamage=4000

	//ClawTipBones=("Rt_Clawtip1","Rt_Clawtip2","Rt_Clawtip3","Rt_Clawtip4","Lft_clawtip1","Lft_Clawtip2","Lft_Clawtip3","Lft_Clawtip4");
	ClawTipBones=("Rt_Clawtip3","Rt_Clawtip4","Lft_Clawtip3","Lft_Clawtip4");
	AttackHitEffect=ParticleSystem'Locust_Reaver.Particles.P_Reaver_Ground_Hit'
	CentaurHitCamAnim=CameraAnim'Effects_Camera.Explosions.CA_Reaver_Tentacle_Strike'

	SupportedEvents.Add(class'SeqEvt_CorpserAttack')

	RemoteRole=ROLE_SimulatedProxy
}