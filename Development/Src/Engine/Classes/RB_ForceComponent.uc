class RB_ForceComponent extends PrimitiveComponent
    abstract
    native
    editinlinenew
    collapsecategories;

enum EForceModeType
{
    FMT_Constant,                   // 0
    FMT_Impulse,                    // 1
    FMT_MAX                         // 2
};

var() bool bUseNxForcefield;
var() bool bForceActive;
var() bool bDetachWhenInactive;
var() bool bForceApplyToCloth;
var() bool bForceApplyToFluid;
var() bool bForceApplyToRigidBodies;
var() bool bForceApplyToProjectiles;
var() RB_ForceComponent.EForceModeType ForceMode;
var() float Duration;
var() RBCollisionChannelContainer CollideWithChannels;
var float ElapsedTime;
var export editinline PrimitiveComponent RenderComponent;
var native const transient Pointer LinearKernel;
var native const transient Pointer TheNxForceField;

function RB_ForceComponent Clone()
{
    //return ReturnValue;    
}

final function SetBaseCloneParameters(RB_ForceComponent ForceComp)
{
    ForceComp.SetTranslation(Translation);
    ForceComp.SetRotation(Rotation);
    ForceComp.Duration = Duration;
    ForceComp.bForceActive = bForceActive;
    ForceComp.bDetachWhenInactive = bDetachWhenInactive;
    ForceComp.ForceMode = ForceMode;
    ForceComp.bForceApplyToCloth = bForceApplyToCloth;
    ForceComp.bForceApplyToFluid = bForceApplyToFluid;
    ForceComp.bForceApplyToRigidBodies = bForceApplyToRigidBodies;
    ForceComp.bForceApplyToProjectiles = bForceApplyToProjectiles;
    ForceComp.CollideWithChannels = CollideWithChannels;
    ForceComp.bUseNxForcefield = bUseNxForcefield;
    //return;    
}

final event AttachCloneToActor(Actor NewOwner)
{
    NewOwner.AttachComponent(Clone());
    //return;    
}

defaultproperties
{
    bForceApplyToCloth=true
    bForceApplyToFluid=true
    CollideWithChannels=(Default=true,Nothing=false,Pawn=true,Vehicle=true,Water=true,GameplayPhysics=true,EffectPhysics=true,Untitled1=true,Untitled2=true,Untitled3=true,Untitled4=false,Cloth=true,FluidDrain=true,PawnRagdoll=false,Rope=false,Cape=false,SoftBody=false,CinematicCape=false,FracturedMeshPart=false,BlockingVolume=false,DeadPawn=false,PawnRagdollStrungUp=false,Projectile=false,VenomHenchmanThrow=false,Grate=false,Destruction=false)
    RBChannel=RBCC_Nothing
    TickGroup=TG_PreAsyncWork
}