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
    CollideWithChannels=(Default=true,GameplayPhysics=true,EffectPhysics=true,Destruction=true)
    DestructibleActorSettings=(MassScaleExponent=0.5,ValidBoundsMin=(X=-500000.0,Y=-500000.0,Z=-500000.0),ValidBoundsMax=(X=500000.0,Y=500000.0,Z=500000.0),GrbVolumeThreshold=1.0)
    FractureBehavior=(DamageFractureThreshold=1.0,DamageDistanceMultiplier=0.1,bDamageAccumulates=true,bCrumbleSmallestChunks=true,DeformationPercentLimit=1.0)
    GameplayActorClass=Class'ApexDestructibleActorSpawnable'
    MenuName="Add ApexDestructibleActor"
    NewActorClass=Class'ApexDestructibleActor'
}