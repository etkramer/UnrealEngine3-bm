class SeqAct_CrowdSpawner extends SeqAct_Latent
    native(Sequence);

struct native CrowdTargetActionInfo
{
    var() name AnimName;
    var() bool bFireEffects;

    structdefaultproperties
    {
        AnimName="None"
        bFireEffects=false
    }
};

struct native CrowdAttachmentInfo
{
    var() StaticMesh StaticMesh;
    var() float Chance;
    var() Vector Scale3D;

    structdefaultproperties
    {
        StaticMesh=none
        Chance=1.0000000
        Scale3D=(X=1.0000000,Y=1.0000000,Z=1.0000000)
    }
};

struct native CrowdAttachmentList
{
    var() name SocketName;
    var() array<CrowdAttachmentInfo> List;

    structdefaultproperties
    {
        SocketName="None"
        List=none
    }
};

var bool bSpawningActive;
var() bool bConformToBSP;
var() bool bConformToWorld;
var() bool bRespawnDeadAgents;
var() bool bLineSpawner;
var() bool bSpawnAtEdge;
var() bool bReduceNumInSplitScreen;
var bool bHasReducedNumberDueToSplitScreen;
var() bool bUseOnlyCrowdPaths;
var() bool bFlockScaleUniform;
var() bool bDrawDebugPathInfo;
var() bool bDrawDebugHitBox;
var() bool bDrawDebugMoveTarget;
var(Lighting) bool bEnableCrowdLightEnvironment;
var() float ConformTraceDist;
var() int ConformTraceInterval;
var() Vector CollisionBoxScaling;
var transient array<CrowdAttractor> AssignedMoveTargets;
var transient array<Actor> SpawnLocs;
var transient array<CrowdAttractor> AssignedActionTargets;
var() float SpawnRate;
var() int SpawnNum;
var() float Radius;
var() float SplitScreenNumReduction;
var float Remainder;
var class<CrowdAgent> AgentClass;
var() float AwareRadius;
var() int AwareUpdateInterval;
var() float AvoidOtherStrength;
var() float AvoidOtherRadius;
var() float MatchVelStrength;
var() float ToPathStrength;
var() float FollowPathStrength;
var() float PathDistance;
var() float ToAttractorStrength;
var() float MinVelDamping;
var() float MaxVelDamping;
var(Action) RawDistributionFloat ActionDuration;
var(Action) RawDistributionFloat ActionInterval;
var(Action) RawDistributionFloat TargetActionInterval;
var(Action) array<name> ActionAnimNames;
var array<name> TargetActionAnimNames;
var(Action) array<CrowdTargetActionInfo> TargetActions;
var(Action) float MaxEffectsPerSecond;
var transient float RemaingEffectsThisFrame;
var(Action) name SpawnAnimName;
var(Action) array<name> DeathAnimNames;
var(Action) float ActionBlendTime;
var(Action) float ReActionDelay;
var(Action) float RotateToTargetSpeed;
var() float SpeedBlendStart;
var() float SpeedBlendEnd;
var() float AnimVelRate;
var() float MaxSpeedBlendChangeSpeed;
var() name MoveSyncGroupName;
var() float MaxYawRate;
var SkeletalMesh FlockMesh;
var() array<SkeletalMesh> FlockMeshes;
var() array<MaterialInterface> RandomMaterials;
var() Vector FlockMeshMinScale3D;
var() Vector FlockMeshMaxScale3D;
var() array<AnimSet> FlockAnimSets;
var name WalkAnimName;
var() array<name> WalkAnimNames;
var name RunAnimName;
var() array<name> RunAnimNames;
var() AnimTree FlockAnimTree;
var() int Health;
var() ParticleSystem ExplosiveDeathEffect;
var() ParticleSystem ExplosiveDeathEffectNonExtremeContent;
var() float ExplosiveDeathEffectScale;
var array<CrowdAgent> SpawnedList;
var(Lighting) LightingChannelContainer FlockLighting;
var() array<CrowdAttachmentList> Attachments;
var() array< class<Actor> > ReportOverlapsWithClass;
var CrowdReplicationActor RepActor;

cpptext
{
	virtual void PostLoad();

	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT deltaTime);
	virtual void CleanUp();

	void UpdateAgent(class ACrowdAgent* Agent, FLOAT DeltaTime);
};

// Export USeqAct_CrowdSpawner::execCacheSpawnerVars(FFrame&, void* const)
native simulated function CacheSpawnerVars();

// Export USeqAct_CrowdSpawner::execKillAgents(FFrame&, void* const)
native simulated function KillAgents();

// Export USeqAct_CrowdSpawner::execUpdateSpawning(FFrame&, void* const)
native simulated function UpdateSpawning(float DeltaSeconds);

simulated function CreateAttachments(CrowdAgent Agent)
{
    local int AttachIdx, InfoIdx, PickedInfoIdx;
    local float ChanceTotal, RandVal;
    local StaticMeshComponent StaticMeshComp;
    local bool bUseSocket, bUseBone;

    AttachIdx = 0;
    J0x07:

    // End:0x3CF [Loop If]
    if(AttachIdx < Attachments.Length)
    {
        // End:0x37
        if(Attachments[AttachIdx].List.Length == 0)
        {
            // [Explicit Continue]
            goto J0x3C5;
        }
        ChanceTotal = 0.0000000;
        InfoIdx = 0;
        J0x49:

        // End:0xA2 [Loop If]
        if(InfoIdx < Attachments[AttachIdx].List.Length)
        {
            ChanceTotal += Attachments[AttachIdx].List[InfoIdx].Chance;
            InfoIdx++;
            // [Loop Continue]
            goto J0x49;
        }
        RandVal = FRand() * ChanceTotal;
        ChanceTotal = 0.0000000;
        InfoIdx = 0;
        J0xC3:

        // End:0x139 [Loop If]
        if(InfoIdx < Attachments[AttachIdx].List.Length)
        {
            ChanceTotal += Attachments[AttachIdx].List[InfoIdx].Chance;
            // End:0x12F
            if(ChanceTotal >= RandVal)
            {
                PickedInfoIdx = InfoIdx;
                // [Explicit Break]
                goto J0x139;
            }
            InfoIdx++;
            // [Loop Continue]
            goto J0xC3;
        }
        J0x139:

        // End:0x3C5
        if(Attachments[AttachIdx].List[PickedInfoIdx].StaticMesh != none)
        {
            bUseSocket = Agent.SkeletalMeshComponent.GetSocketByName(Attachments[AttachIdx].SocketName) != none;
            bUseBone = Agent.SkeletalMeshComponent.MatchRefBone(Attachments[AttachIdx].SocketName) != -1;
            // End:0x32B
            if(bUseSocket || bUseBone)
            {
                StaticMeshComp = new (Agent) Class'StaticMeshComponent';
                StaticMeshComp.SetStaticMesh(Attachments[AttachIdx].List[PickedInfoIdx].StaticMesh);
                StaticMeshComp.SetActorCollision(false, false);
                StaticMeshComp.SetScale3D(Attachments[AttachIdx].List[PickedInfoIdx].Scale3D);
                StaticMeshComp.SetLightEnvironment(Agent.LightEnvironment);
                // End:0x2F0
                if(bUseSocket)
                {
                    Agent.SkeletalMeshComponent.AttachComponentToSocket(StaticMeshComp, Attachments[AttachIdx].SocketName);                    
                }
                else
                {
                    Agent.SkeletalMeshComponent.AttachComponent(StaticMeshComp, Attachments[AttachIdx].SocketName);
                }
                // [Explicit Continue]
                goto J0x3C5;
            }
            LogInternal(((("CrowdAgent: WARNING: Could not find socket or bone called '" $ string(Attachments[AttachIdx].SocketName)) $ "' for mesh '") @ string(Attachments[AttachIdx].List[PickedInfoIdx].StaticMesh)) $ "'");
        }
        J0x3C5:

        AttachIdx++;
        // [Loop Continue]
        goto J0x07;
    }
    //return;    
}

event CrowdAgent SpawnAgent(Actor SpawnLoc)
{
    local Rotator Rot;
    local Vector SpawnPos, SpawnLine, AgentScale3D;
    local CrowdAgent Agent;
    local float RandScale;

    // End:0x9C
    if(bLineSpawner)
    {
        RandScale = -1.0000000 + (2.0000000 * FRand());
        SpawnLine = vect(0.0000000, 1.0000000, 0.0000000) >> SpawnLoc.Rotation;
        SpawnPos = SpawnLoc.Location + ((RandScale * SpawnLine) * Radius);
        Rot.Yaw = SpawnLoc.Rotation.Yaw;        
    }
    else
    {
        Rot = RotRand(false);
        Rot.Pitch = 0;
        // End:0xF7
        if(bSpawnAtEdge)
        {
            SpawnPos = SpawnLoc.Location + ((vect(1.0000000, 0.0000000, 0.0000000) * Radius) >> Rot);            
        }
        else
        {
            SpawnPos = SpawnLoc.Location + (((vect(1.0000000, 0.0000000, 0.0000000) * FRand()) * Radius) >> Rot);
        }
    }
    Agent = SpawnLoc.Spawn(AgentClass,,, SpawnPos, Rot);
    // End:0x180
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
    // End:0x28F
    if(FlockMeshes.Length > 0)
    {
        Agent.SkeletalMeshComponent.SetSkeletalMesh(FlockMeshes[Rand(FlockMeshes.Length)]);
    }
    Agent.SkeletalMeshComponent.SetAnimTreeTemplate(FlockAnimTree);
    Agent.SkeletalMeshComponent.SetLightingChannels(FlockLighting);
    Agent.SkeletalMeshComponent.LineCheckBoundsScale = CollisionBoxScaling;
    // End:0x313
    if(bEnableCrowdLightEnvironment)
    {
        Agent.LightEnvironment.SetEnabled(true);        
    }
    else
    {
        Agent.DetachComponent(Agent.LightEnvironment);
    }
    // End:0x36B
    if(RandomMaterials.Length > 0)
    {
        Agent.SkeletalMeshComponent.SetMaterial(0, RandomMaterials[Rand(RandomMaterials.Length)]);
    }
    CreateAttachments(Agent);
    Agent.SpeedBlendNode = AnimNodeBlend(Agent.SkeletalMeshComponent.FindAnimNode('SpeedBlendNode'));
    Agent.ActionBlendNode = AnimNodeBlend(Agent.SkeletalMeshComponent.FindAnimNode('ActionBlendNode'));
    Agent.ActionSeqNode = AnimNodeSequence(Agent.SkeletalMeshComponent.FindAnimNode('ActionSeqNode'));
    Agent.WalkSeqNode = AnimNodeSequence(Agent.SkeletalMeshComponent.FindAnimNode('WalkSeqNode'));
    Agent.RunSeqNode = AnimNodeSequence(Agent.SkeletalMeshComponent.FindAnimNode('RunSeqNode'));
    Agent.AgentTree = AnimTree(Agent.SkeletalMeshComponent.Animations);
    // End:0x501
    if(Agent.WalkSeqNode != none)
    {
        Agent.WalkSeqNode.SetAnim(WalkAnimNames[Rand(WalkAnimNames.Length)]);
    }
    // End:0x542
    if(Agent.RunSeqNode != none)
    {
        Agent.RunSeqNode.SetAnim(RunAnimNames[Rand(RunAnimNames.Length)]);
    }
    // End:0x573
    if(Agent.ActionSeqNode != none)
    {
        Agent.ActionSeqNode.bZeroRootTranslation = true;
    }
    Agent.VelDamping = MinVelDamping + (FRand() * (MaxVelDamping - MinVelDamping));
    Agent.Health = Health;
    Agent.Spawner = self;
    return Agent;
    //return ReturnValue;    
}

static event int GetObjClassVersion()
{
    return super(SequenceObject).GetObjClassVersion() + 1;
    //return ReturnValue;    
}

defaultproperties
{
    bRespawnDeadAgents=true
    bReduceNumInSplitScreen=true
    bFlockScaleUniform=true
    ConformTraceDist=75.0000000
    ConformTraceInterval=10
    CollisionBoxScaling=(X=1.0000000,Y=1.0000000,Z=1.0000000)
    SpawnRate=10.0000000
    SpawnNum=100
    Radius=200.0000000
    SplitScreenNumReduction=0.5000000
    AgentClass=Class'CrowdAgent'
    AwareRadius=200.0000000
    AwareUpdateInterval=15
    AvoidOtherStrength=1500.0000000
    AvoidOtherRadius=100.0000000
    MatchVelStrength=0.6000000
    ToPathStrength=200.0000000
    FollowPathStrength=30.0000000
    PathDistance=300.0000000
    ToAttractorStrength=50.0000000
    MinVelDamping=0.0010000
    MaxVelDamping=0.0030000
    ActionDuration=(Distribution=// Reference: DistributionFloatUniform'Default__SeqAct_CrowdSpawner.DistributionActionDuration'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'DistributionActionDuration'
    begin object name="DistributionActionDuration" class=Class'DistributionFloatUniform'
        Min=0.8000000
        Max=1.2000000
    end object
    Distribution=DistributionActionDuration,Type=0,Op=2,LookupTableNumElements=2,LookupTableChunkSize=2,LookupTable=LookupTable[0]=0.8000000
    LookupTable[1]=1.2000000
    LookupTable[2]=0.8000000
    LookupTable[3]=1.2000000
    LookupTable[4]=0.8000000
    LookupTable[5]=1.2000000,LookupTableTimeScale=0.0000000,LookupTableStartTime=0.0000000)
    ActionInterval=(Distribution=// Reference: DistributionFloatUniform'Default__SeqAct_CrowdSpawner.DistributionActionInterval'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'DistributionActionInterval'
    begin object name="DistributionActionInterval" class=Class'DistributionFloatUniform'
        Min=10.0000000
        Max=20.0000000
    end object
    Distribution=DistributionActionInterval,Type=0,Op=2,LookupTableNumElements=2,LookupTableChunkSize=2,LookupTable=LookupTable[0]=10.0000000
    LookupTable[1]=20.0000000
    LookupTable[2]=10.0000000
    LookupTable[3]=20.0000000
    LookupTable[4]=10.0000000
    LookupTable[5]=20.0000000,LookupTableTimeScale=0.0000000,LookupTableStartTime=0.0000000)
    TargetActionInterval=(Distribution=// Reference: DistributionFloatUniform'Default__SeqAct_CrowdSpawner.DistributionTargetActionInterval'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'DistributionTargetActionInterval'
    begin object name="DistributionTargetActionInterval" class=Class'DistributionFloatUniform'
        Min=1.0000000
        Max=5.0000000
    end object
    Distribution=DistributionTargetActionInterval,Type=0,Op=2,LookupTableNumElements=2,LookupTableChunkSize=2,LookupTable=LookupTable[0]=1.0000000
    LookupTable[1]=5.0000000
    LookupTable[2]=1.0000000
    LookupTable[3]=5.0000000
    LookupTable[4]=1.0000000
    LookupTable[5]=5.0000000,LookupTableTimeScale=0.0000000,LookupTableStartTime=0.0000000)
    MaxEffectsPerSecond=10.0000000
    ActionBlendTime=0.1000000
    ReActionDelay=1.0000000
    RotateToTargetSpeed=0.1000000
    SpeedBlendStart=150.0000000
    SpeedBlendEnd=180.0000000
    AnimVelRate=0.0098000
    MaxSpeedBlendChangeSpeed=2.0000000
    MoveSyncGroupName="MoveGroup"
    MaxYawRate=40000.0000000
    FlockMeshMinScale3D=(X=1.0000000,Y=1.0000000,Z=1.0000000)
    FlockMeshMaxScale3D=(X=1.0000000,Y=1.0000000,Z=1.0000000)
    Health=100
    ExplosiveDeathEffectScale=1.0000000
    FlockLighting=(bInitialized=true,BSP=false,Static=false,Dynamic=false,CompositeDynamic=false,Skybox=false,Unnamed_1=false,Unnamed_2=false,Unnamed_3=false,Unnamed_4=false,PhysXDestruction=false,Cinematic_1=false,Cinematic_2=false,Cinematic_3=false,Cinematic_4=false,Cinematic_5=false,Cinematic_6=false,Cinematic_7=false,Cinematic_8=false,Cinematic_9=false,Cinematic_10=false,Gameplay_1=false,Gameplay_2=false,Gameplay_3=false,PhysXEffects=false,Crowd=true,Plant=false,Prop=false,Character=false,CinematicExclusive_1=false,CinematicExclusive_2=false,TVExclusive=false)
    InputLinks[0]=(LinkDesc="Start",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
    InputLinks[1]=(LinkDesc="Stop",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
    InputLinks[2]=(LinkDesc="Destroy All",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
    OutputLinks=none
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Target",LinkVar="None",PropertyName="Targets",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[1]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Attractors",LinkVar="None",PropertyName="None",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
}
