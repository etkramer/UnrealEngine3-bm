/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class CrowdAgent extends Actor
	native(Anim)
	placeable;

/** Pointer to action that spawned this agent. */
var	SeqAct_CrowdSpawner	Spawner;

/** Whether this agent is in the moving or idle state. */
var() enum EAgentMoveState
{
	EAMS_Move,
	EAMS_Idle
} AgentState;

/** When the currently active action will end. */
var		float	EndActionTime;
/** When the next action will begin */
var		float	NextActionTime;
/** Velocity damping applied to this specific agent */
var		float	VelDamping;

/** Rotation required to point agent at target (for target actions) */
var		rotator		ToTargetRot;
/** If we should rotate towards target during action */
var		bool	bRotateToTargetRot;
/** If we were near an action target last frame, to detect transitions */
var		bool	bHadNearbyTarget;
/** If TRUE, TargetZPos is valid and should be interpolated towards. */
var		bool	bTargetZPosInitialized;

/** Current position we are trying to move towards. */
var		vector	CurrentMoveTargetPos;

/** Additional force applied to this crowd agent externally - zeroed each frame. */
var		vector	ExternalForce;

/** If conforming to ground, this is how much to move the agent each frame between line-trace updates. */
var		float	InterpZTranslation;

/** SkeletalMeshComponent used for crowd member mesh */
var()	SkeletalMeshComponent			SkeletalMeshComponent;
/** Cached pointer to speed blend node */
var		AnimNodeBlend					SpeedBlendNode;
/** Cached pointer to action blend node */
var		AnimNodeBlend					ActionBlendNode;
/** Cached pointer to action animation player */
var		AnimNodeSequence				ActionSeqNode;
/** Cached pointer to walking animation player */
var		AnimNodeSequence				WalkSeqNode;
/** Cached pointer to running animation player */
var		AnimNodeSequence				RunSeqNode;
/** Cached pointer to AnimTree instance (SkeletalMeshComponent.Animations) */
var		AnimTree						AgentTree;

/** Current health of agent */
var		int								Health;

/** Pointer to LightEnvironment */
var() const editconst LightEnvironmentComponent LightEnvironment;

/** Used to count how many frames since the last conform trace. */
var		transient int	ConformTraceFrameCount;

/** Used to count how many frames since the last conform trace. */
var		transient int	AwareUpdateFrameCount;

/** Updated periodically (AwareUpdateInterval) using main Octree */
var		transient array<CrowdAgent>	NearbyAgents;

/** Update periodically (AwareUpdateInterval) using crowd octree. */
var		transient array<CrowdAttractor>	RelevantAttractors;

/** Update periodically (AwareUpdateInterval)  */
var		transient ReachSpec			NearestPath;

cpptext
{
	virtual void PreBeginPlay();
	virtual void PostScriptDestroyed();
	virtual void GetActorReferences(TArray<FActorReference*>& ActorRefs, UBOOL bIsRemovingLevel);

	virtual void performPhysics(FLOAT DeltaTime);
	void SetAgentMoveState(BYTE NewState);
	void DoAction(UBOOL bAtTarget, const FVector& TargetLoc);
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive, AActor *SourceActor, DWORD TraceFlags);

	/** Fired when an agent does his 'action' at a target, if 'bSpawnEffects' is TRUE */
	virtual void SpawnActionEffect(const FVector& ActionTarget) {}
}

/** Stop agent moving and pay death anim */
native function PlayDeath();

function TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Health -= DamageAmount;

	if(Health <= 0)
	{
		PlayDeath();
		SetCollision(FALSE, FALSE, FALSE); // Turn off all collision when dead.
	}
}

/** Called when crowd agent overlaps something in the ReportOverlapsWithClass list */
event OverlappedActorEvent(Actor A);

defaultproperties
{
	AgentState=EAMS_Move

	TickGroup=TG_DuringAsyncWork

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=FALSE
		InvisibleUpdateTime=5.0
		MinTimeBetweenFullUpdates=2.0
		TickGroup=TG_DuringAsyncWork
		bCastShadows=FALSE
	End Object
	Components.Add(MyLightEnvironment)
	LightEnvironment=MyLightEnvironment

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent0
		CollideActors=true
		bEnableLineCheckWithBounds=TRUE
		BlockActors=false
		BlockZeroExtent=true
		BlockNonZeroExtent=false
		BlockRigidBody=false
		LightEnvironment=MyLightEnvironment
		RBChannel=RBCC_GameplayPhysics
		bCastDynamicShadow=FALSE
		RBCollideWithChannels=(Default=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE)
		bUpdateSkelWhenNotRendered=FALSE
		bAcceptsDynamicDecals=FALSE // for crowds there are so many of them probably not going to notice not getting decals on them.  Each decal on them causes entire SkelMesh to be rerendered
	End Object
	SkeletalMeshComponent=SkeletalMeshComponent0
	Components.Add(SkeletalMeshComponent0)

	Physics=PHYS_Interpolating
	bStatic=false
	bCollideActors=true
	bBlockActors=false
	bWorldGeometry=false
	bCollideWorld=false
	bProjTarget=TRUE
	bUpdateSimulatedPosition=false
	bNoEncroachCheck=true

	RemoteRole=ROLE_None
	bNoDelete=false
}
