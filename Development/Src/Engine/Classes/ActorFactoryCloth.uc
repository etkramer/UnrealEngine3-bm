// BM1
class ActorFactoryCloth extends ActorFactorySkeletalMesh
    native
    config(Editor)
    editinlinenew
    collapsecategories;

var() bool bAttachClothVertsToBaseBody;
var() bool bAutoFreezeClothWhenNotRendered;
var() bool bClothAwakeOnStartup;
var() bool bClothBaseVelClamp;
var() bool bClothFrozen;
var() bool bDisableClothCollision;
var() bool bEnableClothSimulation;
var() bool bClothUseCompartment;
var(Lighting) bool bEnableLightEnvironment;
var(Lighting) bool bCastDynamicShadow;
var(Lighting) bool bSelfShadowOnly;
var() float ClothAttachmentTearFactor;
var() Vector ClothBaseVelClampRange;
var() float ClothBlendWeight;
var() const Vector ClothExternalAcceleration;
var() float ClothForceScale;
var() const PrimitiveComponent.ERBCollisionChannel ClothRBChannel;
var() const RBCollisionChannelContainer ClothRBCollideWithChannels;
var() Vector ClothWind;
var(Lighting) const LightingChannelContainer LightingChannels;

defaultproperties
{
    bClothAwakeOnStartup=true
    bEnableClothSimulation=true
    bClothUseCompartment=true
    bEnableLightEnvironment=true
    bCastDynamicShadow=true
    ClothAttachmentTearFactor=-1.0000000
    ClothBlendWeight=1.0000000
    ClothRBChannel=RBCC_Cloth
    ClothRBCollideWithChannels=(Default=true,Nothing=false,Pawn=true,Vehicle=false,Water=false,GameplayPhysics=true,EffectPhysics=false,Untitled1=false,Untitled2=false,Untitled3=false,Untitled4=false,Cloth=false,FluidDrain=false,PawnRagdoll=false,Rope=false,Cape=false,SoftBody=false,CinematicCape=false,FracturedMeshPart=false,BlockingVolume=false,DeadPawn=false,PawnRagdollStrungUp=false,Projectile=true,VenomHenchmanThrow=false,Grate=false,Destruction=false)
    LightingChannels=(bInitialized=false,BSP=false,Static=false,Dynamic=false,CompositeDynamic=false,Skybox=false,Unnamed_1=false,Unnamed_2=false,Unnamed_3=false,Unnamed_4=false,PhysXDestruction=false,Cinematic_1=false,Cinematic_2=false,Cinematic_3=false,Cinematic_4=false,Cinematic_5=false,Cinematic_6=false,Cinematic_7=false,Cinematic_8=false,Cinematic_9=false,Cinematic_10=false,Gameplay_1=false,Gameplay_2=false,Gameplay_3=false,PhysXEffects=true,Crowd=false,Plant=false,Prop=false,Character=false,CinematicExclusive_1=false,CinematicExclusive_2=false,TVExclusive=false)
    MenuName="Add Cloth"
}