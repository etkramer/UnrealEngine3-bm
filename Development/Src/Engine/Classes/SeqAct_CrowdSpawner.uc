/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class SeqAct_CrowdSpawner extends SeqAct_Latent
	native(Sequence);

/** Set by kismet action inputs - controls whether we are actively spawning agents. */
var		bool	bSpawningActive;

/** Do extra line checks to place agent on BSP surface as they move - faster than bConformToWorld. */
var()	bool	bConformToBSP;

/** Do extra line checks to place agent on any world surface as they move. */
var()	bool	bConformToWorld;

/** How far to trace to conform agent to the bsp/world. */
var()	float	ConformTraceDist;

/** Every how many frames the 'conform' line check is done. */
var()	int		ConformTraceInterval;

/** Scale the agent mesh bounds, used for weapon collision */
var()	vector	CollisionBoxScaling;

/** Cached set of assigned move targets. */
var transient array<CrowdAttractor>		AssignedMoveTargets;
/** Cached set of spawn locations. */
var	transient array<Actor>				SpawnLocs;
/** Cached set of assigned action-at target. */
var transient array<CrowdAttractor>		AssignedActionTargets;

// Spawner stuff

/** How many agents per second will be spawned at the target actor(s).  */
var()	float	SpawnRate;
/** The maximum number of agents alive at one time. If agents are destroyed, more will spawn to meet this number. */
var()	int		SpawnNum;
/** If TRUE, agents that are totally removed (ie blown up) are respawned */
var()	bool	bRespawnDeadAgents;
/** Radius around target actor(s) to spawn agents. */
var()	float	Radius;
/** Spawn in a line rather than in a circle. */
var()	bool	bLineSpawner;
/** Whether to spawn agents only at the edge of the circle, or at any point within the circle. */
var()	bool	bSpawnAtEdge;

/** If TRUE, reduce the number of crowd members we spawn in splitscreen */
var()	bool	bReduceNumInSplitScreen;
/** Whether we have already done the number reduction */
var		bool	bHasReducedNumberDueToSplitScreen;
/** How much to reduce number by in splitscreen */
var()	float	SplitScreenNumReduction;

/** Used by spawning code to accumulate partial spawning */
var		float	Remainder;
/** Class of agent spawned by this action */
var		class<CrowdAgent>	AgentClass;

// Agent force stuff

/** Controls how far around an agent the system looks when finding the average speed. */
var()	float	AwareRadius;
/** Every how many frames the agents 'awareness' is updated (attractors and other agents */
var()	int		AwareUpdateInterval;


/** How hard agents are pushed apart when they overlap. */
var()	float	AvoidOtherStrength;
/** The radius used to check overlap between agents (basically how big an agent is). */
var()	float	AvoidOtherRadius;
/** How aggressively an agent matches the speed of its neighbours. */
var()	float	MatchVelStrength;

/** How hard agents are pushed back when they are too far from a path.  */
var()	float	ToPathStrength;
/** How strongly agents are encouraged to 'flow' along the path direction (towards  their target). */
var()	float	FollowPathStrength;
/** How close agents will stay to a path. */
var()	float	PathDistance;
/** If TRUE, only use paths with nodes flagged as bCrowdPath */
var()	bool	bUseOnlyCrowdPaths;

/** The constant force applied to agents to move them towards their target. */
var()	float	ToAttractorStrength;


/** When an agent is spawned, its individual damping is randomly chosen from this range. In this way, there is variety in the speed that agents move. */
var()	float	MinVelDamping;
var()	float	MaxVelDamping;

// Agent animation stuff

/** How long a agent should stop when performing an action anim */
var(Action)		RawDistributionFloat	ActionDuration;
/** What the time between 'ambient' actions should be.   */
var(Action)		RawDistributionFloat	ActionInterval;
/** What the time between 'target' actions should be */
var(Action)		RawDistributionFloat	TargetActionInterval;
/** Array of names of action animations to play. If array is empty, actions will not occur */
var(Action)		array<name>				ActionAnimNames;

/** DEPRECATED */
var				array<name>				TargetActionAnimNames;

/** Contains info about action to perform at a 'target'. */
struct native CrowdTargetActionInfo
{
	/** Name of animation */
	var()	name	AnimName;
	/** If TRUE, call SpawnActionEffect when this action happens */
	var()	bool	bFireEffects;
};
/** Actions to perform at an Attractor with bActionAtThisAttractor set. */
var(Action)		array<CrowdTargetActionInfo>	TargetActions;

/** Maximum number of times that SpawnActionEffect can be called each second. */
var(Action)		float							MaxEffectsPerSecond;
/** How many more times SpawnActionEffect can get called this frame. */
var				transient float					RemaingEffectsThisFrame;

/** Optional animation to play when agent is spawned. Root motion is enabled during this animation so you can, for example, have an agent crawl from a hole */
var(Action)		name					SpawnAnimName;
/** Set of possible animation names to play when agent dies */
var(Action)		array<name>				DeathAnimNames;
/** How quickly to blend to and from an action animation when it plays */
var(Action)		float					ActionBlendTime;
/** Minimum time between actions occuring */
var(Action)		float					ReActionDelay;
/** When a 'target' action occurs, agent will rotate to face the CrowdAttractor. This controls how fast that turn happens */
var(Action)		float					RotateToTargetSpeed;

/** Below this speed, the walking animation is used */
var()	float	SpeedBlendStart;
/** Above this speed, the running animation is used. Between this and SpeedBlendStart the animations are blended */
var()	float	SpeedBlendEnd;

/** This controls how the animation playback rate changes based on the speed of the agent */
var()	float	AnimVelRate;
/** Limits how quickly blending between running and walking can happen. */
var()	float	MaxSpeedBlendChangeSpeed;
/** Name of sync group for movement, whose rate is scaled */
var()	name	MoveSyncGroupName;

/** Crowd agents rotate to face the direction they are travelling. This value limits how quickly they turn to do this, to avoid them spinning too quickly */
var()	float	MaxYawRate;

/** DEPRECATD */
var		SkeletalMesh		FlockMesh;
/** The SkeletalMeshes to use for flock members */
var()	Array<SkeletalMesh>	FlockMeshes;

/** Optional array of materials to pick randomly from and apply upon spawn. */
var()	array<MaterialInterface>	RandomMaterials;
/** Min 3D drawscale to apply to the agent mesh */
var()	vector			FlockMeshMinScale3D;
/** Max 3D drawscale to apply to the agent mesh */
var()	vector			FlockMeshMaxScale3D;
/** Locks scaling to a linear interpolation between FlockMeshMinScale3D and FlockMeshMaxScale3D */
var()	bool			bFlockScaleUniform;

/** The AnimSets to use for each flock member */
var()	array<AnimSet>	FlockAnimSets;

/** DEPRECATED */
var		name			WalkAnimName;
/** The names of the animation loops to use when moving slowly */
var()	array<name>		WalkAnimNames;
/** DEPRECATED */
var		name			RunAnimName;
/** The name of the animations to use when moving more quickly */
var()	array<name>		RunAnimNames;

/** The AnimTree to use as a template for each agent */
var()	AnimTree		FlockAnimTree;
/** How much damage an agent needs to take before it dies */
var()	int				Health;
/** Not currently used at engine level, but can be used in your own CrowdAgent subclasses for explosive-type damage deaths */
var()	ParticleSystem	ExplosiveDeathEffect;
var()	ParticleSystem	ExplosiveDeathEffectNonExtremeContent;

/** Again, not currently used, but could be used in custom CrowdAgent subclasses */
var()	float			ExplosiveDeathEffectScale;

/** Used to keep track of currently spawned crowd members. */
var array<CrowdAgent> SpawnedList;

/** If TRUE, draw debug information about agents and paths they are trying to stay near. */
var()	bool			bDrawDebugPathInfo;
/** If TRUE, draw debug info showing hit box for weapons. */
var()	bool			bDrawDebugHitBox;
/** If TRUE, draw debug info showing hit desired move target. */
var()	bool			bDrawDebugMoveTarget;

/** Lighting channels to put the agents in. */
var(Lighting)	LightingChannelContainer	FlockLighting;
/** Whether to enable the light environment on crowd members. */
var(Lighting)	bool						bEnableCrowdLightEnvironment;

/** Info about mesh we might want to use as an attachment. */
struct native CrowdAttachmentInfo
{
	/** Pointer to mesh to attach */
	var()	StaticMesh		StaticMesh;
	/** Chance of choosing this attachment. */
	var()	float			Chance;
	/** Scaling applied to mesh when attached */
	var()	vector			Scale3D;

	structdefaultproperties
	{
		Chance=1.0
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
	}
};

/** Info about things you can attach to one socket. */
struct native CrowdAttachmentList
{
	/** Name of socket to attach mesh to */
	var()	name SocketName;
	/** List of possible meshes to attach to this socket. */
	var()	array<CrowdAttachmentInfo>	List;
};

/** List of sets of meshes to attach to agent.  */
var() array<CrowdAttachmentList>	Attachments;

/** OverlappedActorEvent will be called on the CrowdAgent if it overlaps an actor of these */
var() array< class<Actor> >			ReportOverlapsWithClass<AllowAbstract>;


/** Used for replicating crowd inputs to clients. */
var		CrowdReplicationActor		RepActor;

cpptext
{
	virtual void PostLoad();

	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT deltaTime);
	virtual void CleanUp();

	void UpdateAgent(class ACrowdAgent* Agent, FLOAT DeltaTime);
};

/** Cache SpawnLocs and AssignedMoveTargets from attached Kismet vars. */
native simulated function CacheSpawnerVars();
/** Immediately destroy all agents spawned by this action. */
native simulated function KillAgents();
/** Manually update spwaning (for use on clients where action does not become active) */
native simulated function UpdateSpawning(float DeltaSeconds);

/** Create any attachments */
simulated function CreateAttachments(CrowdAgent Agent)
{
	local int		AttachIdx, InfoIdx, PickedInfoIdx;
	local float		ChanceTotal, RandVal;
	local StaticMeshComponent	StaticMeshComp;
	local bool		bUseSocket, bUseBone;

	// Iterate over each list/attachment point.
	for(AttachIdx=0; AttachIdx < Attachments.length; AttachIdx++ )
	{
		// Skip over empty lists
		if(Attachments[AttachIdx].List.length == 0)
		{
			continue;
		}

		// We need to choose one from he list, using the 'Chance' values.
		// First we need to total of all of them
		ChanceTotal = 0.0;
		for(InfoIdx=0; InfoIdx < Attachments[AttachIdx].List.length; InfoIdx++)
		{
			ChanceTotal += Attachments[AttachIdx].List[InfoIdx].Chance;
		}
		// Now pick a value between 0.0 and ChanceTotal
		RandVal = FRand() * ChanceTotal;

		// Now go over list again - when we pass RandVal, that is our attachment
		ChanceTotal = 0.0;
		for(InfoIdx=0; InfoIdx < Attachments[AttachIdx].List.length; InfoIdx++)
		{
			ChanceTotal += Attachments[AttachIdx].List[InfoIdx].Chance;
			if(ChanceTotal >= RandVal)
			{
				PickedInfoIdx = InfoIdx;
				break;
			}
		}

		// Ok, so now we know what we want to attach.
		if( Attachments[AttachIdx].List[PickedInfoIdx].StaticMesh != None )
		{
			// See if name is a socket or a bone (if both, favours socket)
			bUseSocket = (Agent.SkeletalMeshComponent.GetSocketByName(Attachments[AttachIdx].SocketName) != None);
			bUseBone = (Agent.SkeletalMeshComponent.MatchRefBone(Attachments[AttachIdx].SocketName) != INDEX_NONE);

			// See if we found valid attachment point
			if(bUseSocket || bUseBone)
			{
				// Actually create the StaticMeshComponent
				StaticMeshComp = new(Agent) class'StaticMeshComponent';
				StaticMeshComp.SetStaticMesh( Attachments[AttachIdx].List[PickedInfoIdx].StaticMesh );
				StaticMeshComp.SetActorCollision(FALSE, FALSE);
				StaticMeshComp.SetScale3D( Attachments[AttachIdx].List[PickedInfoIdx].Scale3D );
				StaticMeshComp.SetLightEnvironment(Agent.LightEnvironment);

				// Attach it to socket or bone
				if(bUseSocket)
				{
					Agent.SkeletalMeshComponent.AttachComponentToSocket(StaticMeshComp, Attachments[AttachIdx].SocketName);
				}
				else
				{
					Agent.SkeletalMeshComponent.AttachComponent(StaticMeshComp, Attachments[AttachIdx].SocketName);		
				}
			}
			else
			{
				`log("CrowdAgent: WARNING: Could not find socket or bone called '"$Attachments[AttachIdx].SocketName$"' for mesh '"@Attachments[AttachIdx].List[PickedInfoIdx].StaticMesh$"'");
			}
		}
	}
}

/** Called fro C++ to actually create a new CrowdAgent actor, and initialise it */
event CrowdAgent SpawnAgent(Actor SpawnLoc)
{
	local rotator	Rot;
	local vector	SpawnPos, SpawnLine, AgentScale3D;
	local CrowdAgent	Agent;
	local float		RandScale;

	// LINE SPAWN
	if(bLineSpawner)
	{
		// Scale between -1.0 and 1.0
		RandScale = -1.0 + (2.0 * FRand());
		// Get line along which to spawn.
		SpawnLine = vect(0,1,0) >> SpawnLoc.Rotation;
		// Now make the position
		SpawnPos = SpawnLoc.Location + (RandScale * SpawnLine * Radius);

		// Always face same way as spawn location
		Rot.Yaw = SpawnLoc.Rotation.Yaw;
	}
	// CIRCLE SPAWN
	else
	{
		Rot = RotRand(false);
		Rot.Pitch = 0;

		if(bSpawnAtEdge)
		{
			SpawnPos = SpawnLoc.Location + ((vect(1,0,0) * Radius) >> Rot);
		}
		else
		{
			SpawnPos = SpawnLoc.Location + ((vect(1,0,0) * FRand() * Radius) >> Rot);
		}
	}


	Agent = SpawnLoc.Spawn( AgentClass,,,SpawnPos,Rot);

	// Choose a random range
	if(bFlockScaleUniform)
	{
		AgentScale3D = FlockMeshMinScale3D + (FRand() * (FlockMeshMaxScale3D - FlockMeshMinScale3D));
	}
	else
	{
		AgentScale3D.X = RandRange(FlockMeshMinScale3D.X, FlockMeshMaxScale3D.X);
		AgentScale3D.Y = RandRange(FlockMeshMinScale3D.Y, FlockMeshMaxScale3D.Y);
		AgentScale3D.Z = RandRange(FlockMeshMinScale3D.Z, FlockMeshMaxScale3D.Z);
	}
	Agent.SetDrawScale3D(AgentScale3D);

	Agent.SkeletalMeshComponent.AnimSets = FlockAnimSets;
	Agent.SkeletalMeshComponent.SetSkeletalMesh(FlockMeshes[Rand(FlockMeshes.length)]);
	Agent.SkeletalMeshComponent.SetAnimTreeTemplate(FlockAnimTree);
	Agent.SkeletalMeshComponent.SetLightingChannels(FlockLighting);
	Agent.SkeletalMeshComponent.LineCheckBoundsScale = CollisionBoxScaling;

	// If desired, enable light env
	if(bEnableCrowdLightEnvironment)
	{
		Agent.LightEnvironment.SetEnabled(TRUE);
	}
	// If not, detach to stop it even getting updated
	else
	{
		Agent.DetachComponent(Agent.LightEnvironment);
	}

	// Apply random material if some are set
	if(RandomMaterials.length > 0)
	{
		Agent.SkeletalMeshComponent.SetMaterial(0, RandomMaterials[Rand(RandomMaterials.length)]);
	}

	// Do attachments
	CreateAttachments(Agent);

	// Cache pointers to anim nodes
	Agent.SpeedBlendNode = AnimNodeBlend(Agent.SkeletalMeshComponent.Animations.FindAnimNode('SpeedBlendNode'));
	Agent.ActionBlendNode = AnimNodeBlend(Agent.SkeletalMeshComponent.Animations.FindAnimNode('ActionBlendNode'));
	Agent.ActionSeqNode = AnimNodeSequence(Agent.SkeletalMeshComponent.Animations.FindAnimNode('ActionSeqNode'));
	Agent.WalkSeqNode = AnimNodeSequence(Agent.SkeletalMeshComponent.Animations.FindAnimNode('WalkSeqNode'));
	Agent.RunSeqNode = AnimNodeSequence(Agent.SkeletalMeshComponent.Animations.FindAnimNode('RunSeqNode'));
	Agent.AgentTree = AnimTree(Agent.SkeletalMeshComponent.Animations);

	// Assign random walk/run cycle
	if(Agent.WalkSeqNode != None)
	{
		Agent.WalkSeqNode.SetAnim(WalkAnimNames[Rand(WalkAnimNames.length)]);
	}

	if(Agent.RunSeqNode != None)
	{
		Agent.RunSeqNode.SetAnim(RunAnimNames[Rand(RunAnimNames.length)]);
	}

	if(Agent.ActionSeqNode != None)
	{
		Agent.ActionSeqNode.bZeroRootTranslation = TRUE;
	}

	Agent.VelDamping = MinVelDamping + (FRand() * (MaxVelDamping - MinVelDamping));
	Agent.Health = Health;
	Agent.Spawner = self;

	return Agent;
}

static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Crowd Spawner"
	ObjCategory="Crowd"

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")
	InputLinks(2)=(LinkDesc="Destroy All")

	OutputLinks.Empty

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Attractors")

	AgentClass=class'Engine.CrowdAgent'

	FlockLighting=(Crowd=TRUE,bInitialized=TRUE)

	Radius=200
	SpawnRate=10
	SpawnNum=100
	bRespawnDeadAgents=TRUE

	bReduceNumInSplitScreen=TRUE
	SplitScreenNumReduction=0.5

	Health=100

	ConformTraceDist=75.0
	ConformTraceInterval=10

	AvoidOtherStrength=1500.0
	AvoidOtherRadius=100.0

	MatchVelStrength=0.6

	AwareRadius=200.0
	AwareUpdateInterval=15

	FollowPathStrength=30.0
	ToPathStrength=200.0
	PathDistance=300.0

	ToAttractorStrength=50.0

	ExplosiveDeathEffectScale=1.0

	FlockMeshMinScale3D=(X=1.0,Y=1.0,Z=1.0)
	FlockMeshMaxScale3D=(X=1.0,Y=1.0,Z=1.0)
	bFlockScaleUniform=TRUE

	CollisionBoxScaling=(X=1.0,Y=1.0,Z=1.0)

	Begin Object Class=DistributionFloatUniform Name=DistributionActionDuration
		Min=0.8
		Max=1.2
	End Object
	ActionDuration=(Distribution=DistributionActionDuration)

	Begin Object Class=DistributionFloatUniform Name=DistributionTargetActionInterval
		Min=1.0
		Max=5.0
	End Object
	TargetActionInterval=(Distribution=DistributionTargetActionInterval)

	Begin Object Class=DistributionFloatUniform Name=DistributionActionInterval
		Min=10.0
		Max=20.0
	End Object
	ActionInterval=(Distribution=DistributionActionInterval)

	MaxEffectsPerSecond=10.0

	MinVelDamping=0.001
	MaxVelDamping=0.003

	ActionBlendTime=0.1
	ReActionDelay=1.0
	RotateToTargetSpeed=0.1

	SpeedBlendStart=150.0
	SpeedBlendEnd=180.0

	AnimVelRate=0.0098
	MaxSpeedBlendChangeSpeed=2.0
	MoveSyncGroupName=MoveGroup
	MaxYawRate=40000.0
}
