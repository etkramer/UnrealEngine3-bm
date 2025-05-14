class KActor extends DynamicSMActor
    native(Physics)
    nativereplication
    placeable
    showcategories(Navigation);

cpptext
{
	// UObject interface
	virtual void PostLoad();

	// AActor interface
	virtual void physRigidBody(FLOAT DeltaTime);
	virtual INT* GetOptimizedRepList(BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData);
	UBOOL ShouldTrace(UPrimitiveComponent* Primitive, AActor *SourceActor, DWORD TraceFlags);

	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();

	virtual void TickSpecial(FLOAT DeltaSeconds);
}

var() bool bDamageAppliesImpulse;
var() repnotify bool bWakeOnLevelStart;
var bool bCurrentSlide;
var bool bSlideActive;
var() bool bDontBlockActors;
var(StayUprightSpring) bool bEnableStayUprightSpring;
var() bool bLimitMaxPhysicsVelocity;
var transient bool bNeedsRBStateReplication;
var bool bDisableClientSidePawnInteractions;
var export editinline ParticleSystemComponent ImpactEffectComponent;
var export editinline AudioComponent ImpactSoundComponent;
var export editinline AudioComponent ImpactSoundComponent2;
var float LastImpactTime;
var PhysEffectInfo ImpactEffectInfo;
var export editinline RB_ForceComponent ImpactForceComponent;
var export editinline ParticleSystemComponent SlideEffectComponent;
var export editinline AudioComponent SlideSoundComponent;
var float LastSlideTime;
var PhysEffectInfo SlideEffectInfo;
var(StayUprightSpring) float StayUprightTorqueFactor;
var(StayUprightSpring) float StayUprightMaxTorque;
var() float MaxPhysicsVelocity;
var native const RigidBodyState RBState;
var native const float AngErrorAccumulator;
var repnotify Vector ReplicatedDrawScale3D;
var transient Vector InitialLocation;
var transient Rotator InitialRotation;

// Export UKActor::execGetKActorPhysMaterial(FFrame&, void* const)
native final function PhysicalMaterial GetKActorPhysMaterial();

// Export UKActor::execResolveRBState(FFrame&, void* const)
native final function ResolveRBState();

simulated event PostBeginPlay()
{
    super.PostBeginPlay();
    // End:0x23
    if(bWakeOnLevelStart)
    {
        StaticMeshComponent.WakeRigidBody();        
    }
    else
    {
        bNeedsRBStateReplication = !bNoDelete;
    }
    ReplicatedDrawScale3D = DrawScale3D * 1000.0000000;
    // End:0x61
    if(StaticMeshComponent.bNotifyRigidBodyCollision)
    {
        SetPhysicalCollisionProperties();
    }
    InitialLocation = Location;
    InitialRotation = Rotation;
    // End:0x86
    if(bDontBlockActors)
    {
        SetCollision(,, false);
    }
    // End:0xB4
    if(bDisableClientSidePawnInteractions && Role != ROLE_Authority)
    {
        StaticMeshComponent.SetRBCollidesWithChannel(2, false);
    }
    //return;    
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
    ShutDown();
    super(Actor).FellOutOfWorld(dmgType);
    //return;    
}

event Destroyed()
{
    // End:0x50
    if(ImpactEffectInfo.Sound != none)
    {
        // End:0x33
        if(ImpactSoundComponent != none)
        {
            ImpactSoundComponent.bAutoDestroy = true;
        }
        // End:0x50
        if(ImpactSoundComponent2 != none)
        {
            ImpactSoundComponent2.bAutoDestroy = true;
        }
    }
    // End:0x78
    if(SlideEffectInfo.Sound != none)
    {
        SlideSoundComponent.bAutoDestroy = true;
    }
    super(Actor).Destroyed();
    //return;    
}

simulated event SetPhysicalCollisionProperties()
{
    local PhysicalMaterial PhysMat;

    PhysMat = GetKActorPhysMaterial();
    ImpactEffectInfo = PhysMat.FindPhysEffectInfo(0);
    SlideEffectInfo = PhysMat.FindPhysEffectInfo(1);
    // End:0x61
    if(ImpactEffectComponent != none)
    {
        DetachComponent(ImpactEffectComponent);
        ImpactEffectComponent = none;
    }
    // End:0x7E
    if(ImpactSoundComponent != none)
    {
        DetachComponent(ImpactSoundComponent);
        ImpactSoundComponent = none;
    }
    // End:0x9B
    if(ImpactSoundComponent2 != none)
    {
        DetachComponent(ImpactSoundComponent2);
        ImpactSoundComponent2 = none;
    }
    // End:0xB8
    if(ImpactForceComponent != none)
    {
        DetachComponent(ImpactForceComponent);
        ImpactForceComponent = none;
    }
    // End:0xD5
    if(SlideEffectComponent != none)
    {
        DetachComponent(SlideEffectComponent);
        SlideEffectComponent = none;
    }
    // End:0xF2
    if(SlideSoundComponent != none)
    {
        DetachComponent(SlideSoundComponent);
        SlideSoundComponent = none;
    }
    // End:0x159
    if(ImpactEffectInfo.Effect != none)
    {
        ImpactEffectComponent = new (Outer) Class'ParticleSystemComponent';
        AttachComponent(ImpactEffectComponent);
        ImpactEffectComponent.bAutoActivate = false;
        ImpactEffectComponent.SetTemplate(ImpactEffectInfo.Effect);
    }
    // End:0x1ED
    if(ImpactEffectInfo.Sound != none)
    {
        ImpactSoundComponent = new (Outer) Class'AudioComponent';
        AttachComponent(ImpactSoundComponent);
        ImpactSoundComponent.SoundCue = ImpactEffectInfo.Sound;
        ImpactSoundComponent2 = new (Outer) Class'AudioComponent';
        AttachComponent(ImpactSoundComponent2);
        ImpactSoundComponent2.SoundCue = ImpactEffectInfo.Sound;
    }
    // End:0x233
    if(ImpactEffectInfo.Force != none)
    {
        ImpactForceComponent = ImpactEffectInfo.Force.Clone();
        AttachComponent(ImpactForceComponent);
    }
    // End:0x29A
    if(SlideEffectInfo.Effect != none)
    {
        SlideEffectComponent = new (Outer) Class'ParticleSystemComponent';
        AttachComponent(SlideEffectComponent);
        SlideEffectComponent.bAutoActivate = false;
        SlideEffectComponent.SetTemplate(SlideEffectInfo.Effect);
    }
    // End:0x2EF
    if(SlideEffectInfo.Sound != none)
    {
        SlideSoundComponent = new (Outer) Class'AudioComponent';
        AttachComponent(SlideSoundComponent);
        SlideSoundComponent.SoundCue = SlideEffectInfo.Sound;
    }
    //return;    
}

simulated event SpawnedByKismet()
{
    // End:0x1D
    if(StaticMeshComponent.bNotifyRigidBodyCollision)
    {
        SetPhysicalCollisionProperties();
    }
    InitialLocation = Location;
    InitialRotation = Rotation;
    //return;    
}

simulated event ReplicatedEvent(name VarName)
{
    local Vector NewDrawScale3D;

    // End:0x30
    if(VarName == 'bWakeOnLevelStart')
    {
        // End:0x2D
        if(bWakeOnLevelStart)
        {
            StaticMeshComponent.WakeRigidBody();
        }        
    }
    else
    {
        // End:0x63
        if(VarName == 'ReplicatedDrawScale3D')
        {
            NewDrawScale3D = ReplicatedDrawScale3D / 1000.0000000;
            SetDrawScale3D(NewDrawScale3D);            
        }
        else
        {
            super.ReplicatedEvent(VarName);
        }
    }
    //return;    
}

function ApplyImpulse(Vector ImpulseDir, float ImpulseMag, Vector HitLocation, optional TraceHitInfo HitInfo)
{
    local Vector ApplyImpulse;

    ImpulseDir = Normal(ImpulseDir);
    ApplyImpulse = ImpulseDir * ImpulseMag;
    // End:0x6F
    if(HitInfo.HitComponent != none)
    {
        HitInfo.HitComponent.AddImpulse(ApplyImpulse, HitLocation, HitInfo.BoneName);        
    }
    else
    {
        CollisionComponent.AddImpulse(ApplyImpulse, HitLocation);
    }
    //return;    
}

event TakeDamage(int Damage, Controller EventInstigator, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
    local Vector ApplyImpulse;

    super(Actor).TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
    // End:0x110
    if(bDamageAppliesImpulse && DamageType.default.KDamageImpulse > float(0))
    {
        // End:0x87
        if(VSize(Momentum) < 0.0010000)
        {
            LogInternal("Zero momentum to KActor.TakeDamage");
            return;
        }
        ApplyImpulse = Normal(Momentum) * DamageType.default.KDamageImpulse;
        // End:0xF4
        if(HitInfo.HitComponent != none)
        {
            HitInfo.HitComponent.AddImpulse(ApplyImpulse, HitLocation, HitInfo.BoneName);            
        }
        else
        {
            CollisionComponent.AddImpulse(ApplyImpulse, HitLocation);
        }
    }
    //return;    
}

simulated function TakeRadiusDamage(Controller InstigatedBy, float BaseDamage, float DamageRadius, class<DamageType> DamageType, float Momentum, Vector HurtOrigin, bool bFullDamage, Actor DamageCauser)
{
    local int Idx;
    local SeqEvent_TakeDamage DmgEvt;

    Idx = 0;
    J0x07:

    // End:0x6D [Loop If]
    if(Idx < GeneratedEvents.Length)
    {
        DmgEvt = SeqEvent_TakeDamage(GeneratedEvents[Idx]);
        // End:0x63
        if(DmgEvt != none)
        {
            DmgEvt.HandleDamage(self, InstigatedBy, DamageType, int(BaseDamage), HurtOrigin);
        }
        Idx++;
        // [Loop Continue]
        goto J0x07;
    }
    // End:0xDC
    if((bDamageAppliesImpulse && DamageType.default.RadialDamageImpulse > float(0)) && Role == ROLE_Authority)
    {
        CollisionComponent.AddRadialImpulse(HurtOrigin, DamageRadius, DamageType.default.RadialDamageImpulse, 1, DamageType.default.bRadialDamageVelChange);
    }
    //return;    
}

simulated function OnToggle(SeqAct_Toggle Action)
{
    // End:0x31
    if(Action.InputLinks[0].bHasImpulse)
    {
        StaticMeshComponent.WakeRigidBody();
    }
    //return;    
}

simulated function OnTeleport(SeqAct_Teleport inAction)
{
    local Vector Loc;
    local Rotator Rot;

    // End:0x52
    if(inAction.GetDestination(Loc, Rot))
    {
        // End:0x4A
        if(Physics == 10)
        {
            StaticMeshComponent.SetRBRotation(Rot);            
        }
        else
        {
            SetRotation(Rot);
        }
    }
    // End:0x7B
    if(Physics == 10)
    {
        StaticMeshComponent.SetRBPosition(Loc);        
    }
    else
    {
        SetLocation(Loc);
    }
    PlayTeleportEffect(false, true);
    //return;    
}

simulated function Reset()
{
    StaticMeshComponent.SetRBLinearVelocity(vect(0.0000000, 0.0000000, 0.0000000));
    StaticMeshComponent.SetRBAngularVelocity(vect(0.0000000, 0.0000000, 0.0000000));
    StaticMeshComponent.SetRBPosition(InitialLocation);
    StaticMeshComponent.SetRBRotation(InitialRotation);
    // End:0x87
    if(!bWakeOnLevelStart)
    {
        StaticMeshComponent.PutRigidBodyToSleep();        
    }
    else
    {
        StaticMeshComponent.WakeRigidBody();
    }
    ResolveRBState();
    bForceNetUpdate = true;
    super(Actor).Reset();
    //return;    
}

event DestroyIfFallenOutOfWorld()
{
    // End:0x49
    if(StaticMeshComponent.GetHasFallenOutOfWorld())
    {
        LogInternal(string(self) @ "fell out of world so destroying self.");
        Destroy();
    }
    //return;    
}

defaultproperties
{
    bDamageAppliesImpulse=true
    bNeedsRBStateReplication=true
    bDisableClientSidePawnInteractions=true
    StayUprightTorqueFactor=1000.0000000
    StayUprightMaxTorque=1500.0000000
    MaxPhysicsVelocity=350.0000000
    ReplicatedDrawScale3D=(X=1000.0000000,Y=1000.0000000,Z=1000.0000000)
    // Reference: StaticMeshComponent'Default__KActor.StaticMeshComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'StaticMeshComponent0'
    // Archetype: StaticMeshComponent'Default__DynamicSMActor.StaticMeshComponent0'
    begin object name="StaticMeshComponent0"
        WireframeColor=(R=0,G=255,B=128,A=255)
        LightEnvironment=DynamicLightEnvironmentComponent'Default__KActor.MyLightEnvironment'
        RBCollideWithChannels=(Default=true,GameplayPhysics=true,EffectPhysics=true,BlockingVolume=true)
        BlockRigidBody=true
        RBChannel=RBCC_GameplayPhysics
    end object
    StaticMeshComponent=StaticMeshComponent0
    // Reference: DynamicLightEnvironmentComponent'Default__KActor.MyLightEnvironment'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'MyLightEnvironment'
    // Archetype: DynamicLightEnvironmentComponent'Default__DynamicSMActor.MyLightEnvironment'
    begin object name="MyLightEnvironment"
    end object
    LightEnvironment=MyLightEnvironment
    bPawnCanBaseOn=false
    bSafeBaseIfAsleep=true
    bNoDelete=true
    bAlwaysRelevant=true
    bUpdateSimulatedPosition=true
    bNetInitialRotation=true
    bBlocksNavigation=true
    bCollideActors=true
    bBlockActors=true
    bProjTarget=true
    bBlocksTeleport=true
    bNoEncroachCheck=true
    Components[0]=MyLightEnvironment
    Components[1]=StaticMeshComponent0
    Physics=PHYS_RigidBody
    TickGroup=TG_PostAsyncWork
    CollisionComponent=StaticMeshComponent0
    SupportedEvents[0]=Class'SeqEvent_Touch'
    SupportedEvents[1]=Class'SeqEvent_Destroyed'
    SupportedEvents[2]=Class'SeqEvent_TakeDamage'
    SupportedEvents[3]=Class'SeqEvent_RigidBodyCollision'
}
