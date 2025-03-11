// BM1
class ApexDestructibleActor extends Actor
    native
    dependson(LightComponent)
    placeable;

struct native ApexDestructibleChunkLevelSettings
{
    var() bool bTakeImpactDamage;
    var() bool bIgnorePoseUpdates;
    var() bool bIgnoreRaycastCallbacks;
    var() bool bIgnoreContactCallbacks;
    var() bool bUserFlag0;
    var() bool bUserFlag1;
    var() bool bUserFlag2;
    var() bool bUserFlag3;

    structdefaultproperties
    {
        bTakeImpactDamage=false
        bIgnorePoseUpdates=false
        bIgnoreRaycastCallbacks=false
        bIgnoreContactCallbacks=false
        bUserFlag0=false
        bUserFlag1=false
        bUserFlag2=false
        bUserFlag3=false
    }
};

struct native ApexFractureBehavior
{
    var() float DamageFractureThreshold;
    var() float DamageDistanceMultiplier;
    var() float DamageMaximum;
    var() float DamageFromImpactFactor;
    var() bool bDamageAccumulates;
    var() bool bCrumbleSmallestChunks;
    var() float DeformationPercentPerDamage;
    var() float DeformationPercentLimit;
    var() float FractureImpulseScale;
    var() float ImpactVelocityThreshold;
    var() editfixedsize array<ApexDestructibleChunkLevelSettings> ChunkLevelSettings;
    var() bool bOverrideAssetFractureEffects;
    var() editfixedsize array<PhysicalMaterial> FractureEffects;

    structdefaultproperties
    {
        DamageFractureThreshold=1.0000000
        DamageDistanceMultiplier=0.1000000
        DamageMaximum=0.0000000
        DamageFromImpactFactor=0.0000000
        bDamageAccumulates=true
        bCrumbleSmallestChunks=true
        DeformationPercentPerDamage=0.0000000
        DeformationPercentLimit=1.0000000
        FractureImpulseScale=0.0000000
        ImpactVelocityThreshold=0.0000000
        bOverrideAssetFractureEffects=false
    }
};

struct native ApexDestructibleActorSettings
{
    var() float MaximumChunkSpeed;
    var() float MassScaleExponent;
    var() bool bUseValidBounds;
    var() bool bDisableGravity;
    var() Vector ValidBoundsMin;
    var() Vector ValidBoundsMax;
    var() float GrbVolumeThreshold;

    structdefaultproperties
    {
        MaximumChunkSpeed=0.0000000
        MassScaleExponent=0.5000000
        bUseValidBounds=false
        bDisableGravity=false
        ValidBoundsMin=(X=-500000.0000000,Y=-500000.0000000,Z=-500000.0000000)
        ValidBoundsMax=(X=500000.0000000,Y=500000.0000000,Z=500000.0000000)
        GrbVolumeThreshold=0.0000000
    }
};

var() const editconst export editinline ApexStaticDestructibleComponent StaticDestructibleComponent;
var() editinline ApexFractureBehavior FractureBehavior;
var() editinline ApexDestructibleActorSettings DestructibleActorSettings;
var init array<init byte> VisibilityFactors;
var float LastFractureTime;
var transient array<SoundCue> FractureSounds;
var transient array<ParticleSystem> FractureParticleEffects;
var transient array<float> FractureReFireDelays;

event SpawnFractureEmitter(ParticleSystem EmitterTemplate, Vector SpawnLocation)
{
    local ParticleSystemComponent PSC;
    local LightingChannelContainer Lights;

    PSC = WorldInfo.MyEmitterPool.SpawnEmitter(EmitterTemplate, SpawnLocation);
    Lights = PSC.LightingChannels;
    Lights.PhysXEffects = true;
    Lights.Dynamic = true;
    Lights.Prop = true;
    Lights.bInitialized = true;
    PSC.SetLightingChannels(Lights);
    //return;    
}

// Export UApexDestructibleActor::execCacheFractureEffects(FFrame&, void* const)
native function CacheFractureEffects();

simulated event PostBeginPlay()
{
    super.PostBeginPlay();
    CacheFractureEffects();
    //return;    
}

// Export UApexDestructibleActor::execTakeDamage(FFrame&, void* const)
native simulated function TakeDamage(int Damage, Controller EventInstigator, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser);

// Export UApexDestructibleActor::execTakeRadiusDamage(FFrame&, void* const)
native simulated function TakeRadiusDamage(Controller InstigatedBy, float BaseDamage, float DamageRadius, class<DamageType> DamageType, float Momentum, Vector HurtOrigin, bool bFullDamage, Actor DamageCauser);