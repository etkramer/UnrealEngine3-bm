// BM1
class ActorFactoryApexDestructible extends ActorFactory
    native
    config(Editor)
    editinlinenew
    collapsecategories;

var() bool bStartAwake;
var() PrimitiveComponent.ERBCollisionChannel RBChannel;
var() const RBCollisionChannelContainer CollideWithChannels;
var() editinline ApexDestructibleActorSettings DestructibleActorSettings;
var() editinline ApexFractureBehavior FractureBehavior;
var() ApexDestructibleAsset DestructibleAsset;

defaultproperties
{
    bStartAwake=true
    RBChannel=RBCC_Destruction
    CollideWithChannels=(Default=true,Nothing=false,Pawn=false,Vehicle=false,Water=false,GameplayPhysics=true,EffectPhysics=true,Untitled1=false,Untitled2=false,Untitled3=false,Untitled4=false,Cloth=false,FluidDrain=false,PawnRagdoll=false,Rope=false,Cape=false,SoftBody=false,CinematicCape=false,FracturedMeshPart=false,BlockingVolume=true,DeadPawn=false,PawnRagdollStrungUp=false,Projectile=false,VenomHenchmanThrow=false,Grate=false,Destruction=true)
    DestructibleActorSettings=(MaximumChunkSpeed=0.0000000,MassScaleExponent=0.5000000,bUseValidBounds=false,bDisableGravity=false,ValidBoundsMin=(X=-500000.0000000,Y=-500000.0000000,Z=-500000.0000000),ValidBoundsMax=(X=500000.0000000,Y=500000.0000000,Z=500000.0000000),GrbVolumeThreshold=1.0000000)
    FractureBehavior=(DamageFractureThreshold=1.0000000,DamageDistanceMultiplier=0.1000000,DamageMaximum=0.0000000,DamageFromImpactFactor=0.0000000,bDamageAccumulates=true,bCrumbleSmallestChunks=true,DeformationPercentPerDamage=0.0000000,DeformationPercentLimit=1.0000000,FractureImpulseScale=0.0000000,ImpactVelocityThreshold=0.0000000,ChunkLevelSettings=none,bOverrideAssetFractureEffects=false,FractureEffects=none)
    GameplayActorClass=Class'ApexDestructibleActorSpawnable'
    MenuName="Add ApexDestructibleActor"
    NewActorClass=Class'ApexDestructibleActor'
}