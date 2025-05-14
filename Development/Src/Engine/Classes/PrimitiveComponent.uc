class PrimitiveComponent extends ActorComponent
    dependson(Scene)
    abstract
    native
    noexport;

enum ERBCollisionChannel
{
    RBCC_Default,                   // 0
    RBCC_Nothing,                   // 1
    RBCC_Pawn,                      // 2
    RBCC_Vehicle,                   // 3
    RBCC_Water,                     // 4
    RBCC_GameplayPhysics,           // 5
    RBCC_EffectPhysics,             // 6
    RBCC_Untitled1,                 // 7
    RBCC_Untitled2,                 // 8
    RBCC_Untitled3,                 // 9
    RBCC_Untitled4,                 // 10
    RBCC_Cloth,                     // 11
    RBCC_FluidDrain,                // 12
    RBCC_PawnRagdoll,               // 13
    RBCC_Rope,                      // 14
    RBCC_Cape,                      // 15
    RBCC_SoftBody,                  // 16
    RBCC_CinematicCape,             // 17
    RBCC_FracturedMeshPart,         // 18
    RBCC_BlockingVolume,            // 19
    RBCC_DeadPawn,                  // 20
    RBCC_PawnRagdollStrungUp,       // 21
    RBCC_Projectile,                // 22
    RBCC_VenomHenchmanThrow,        // 23
    RBCC_Grate,                     // 24
    RBCC_Destruction,               // 25
    RBCC_MAX                        // 26
};

enum ERadialImpulseFalloff
{
    RIF_Constant,                   // 0
    RIF_Linear,                     // 1
    RIF_MAX                         // 2
};

struct MaterialViewRelevance
{
    var bool bOpaque;
    var bool bTranslucent;
    var bool bDistortion;
    var bool bOneLayerDistortionRelevance;
    var bool bLit;
    var bool bUsesSceneColor;

    structdefaultproperties
    {
        bOpaque=false
        bTranslucent=false
        bDistortion=false
        bOneLayerDistortionRelevance=false
        bLit=false
        bUsesSceneColor=false
    }
};

struct RBCollisionChannelContainer
{
    var() const bool Default;
    var const bool Nothing;
    var() const bool Pawn;
    var() const bool Vehicle;
    var() const bool Water;
    var() const bool GameplayPhysics;
    var() const bool EffectPhysics;
    var() const bool Untitled1;
    var() const bool Untitled2;
    var() const bool Untitled3;
    var() const bool Untitled4;
    var() const bool Cloth;
    var() const bool FluidDrain;
    var() const bool PawnRagdoll;
    var() const bool Rope;
    var() const bool Cape;
    var() const bool SoftBody;
    var() const bool CinematicCape;
    var() const bool FracturedMeshPart;
    var() const bool BlockingVolume;
    var() const bool DeadPawn;
    var() const bool PawnRagdollStrungUp;
    var() const bool Projectile;
    var() const bool VenomHenchmanThrow;
    var() const bool Grate;
    var() const bool Destruction;

    structdefaultproperties
    {
        Default=false
        Nothing=false
        Pawn=false
        Vehicle=false
        Water=false
        GameplayPhysics=false
        EffectPhysics=false
        Untitled1=false
        Untitled2=false
        Untitled3=false
        Untitled4=false
        Cloth=false
        FluidDrain=false
        PawnRagdoll=false
        Rope=false
        Cape=false
        SoftBody=false
        CinematicCape=false
        FracturedMeshPart=false
        BlockingVolume=false
        DeadPawn=false
        PawnRagdollStrungUp=false
        Projectile=false
        VenomHenchmanThrow=false
        Grate=false
        Destruction=false
    }
};

var private native const transient Pointer SceneInfo{FPrimitiveSceneInfo};;
var native const transient Matrix LocalToWorld;
var native const transient Matrix CachedParentToWorld;
var private native const int DetachFence;
var native const transient float LocalToWorldDeterminant;
var native const transient int MotionBlurInfoIndex;
var private noimport native const array<Pointer> DecalList{class FDecalInteraction};
var private const export editinline transient array<export editinline DecalComponent> DecalsToReattach;
var native const transient int Tag;
var const export editinline PrimitiveComponent ShadowParent;
var const export editinline transient FogVolumeDensityComponent FogVolumeComponent;
var native const transient BoxSphereBounds Bounds;
var const export editinline LightEnvironmentComponent LightEnvironment;
var private const export editinline transient LightEnvironmentComponent PreviousLightEnvironment;
var(Rendering) float MinDrawDistance;
var(Rendering) private const noexport float MaxDrawDistance;
var(Rendering) editconst float CachedMaxDrawDistance;
var const float CullArea;
var(Rendering) const float CullAreaMultiplier;
var(Rendering) float MotionBlurScale;
var(Rendering) float SortBias;
var(Rendering) int TranslucencySortPriority;
var native const transient array<int> OctreeNodes;
var(Physics) const PhysicalMaterial PhysMaterialOverride;
var native const RB_BodyInstance BodyInstance;
var() const Vector Translation;
var() const Rotator Rotation;
var() const float Scale;
var() const Vector Scale3D;
var const transient float LastSubmitTime;
var transient float LastRenderTime;
var float ScriptRigidBodyCollisionThreshold;
var(Lighting) const LightingChannelContainer LightingChannels;
var(Collision) const RBCollisionChannelContainer RBCollideWithChannels;
var(Rendering) const bool bAllowCullDistanceVolume;
var(Rendering) const bool HiddenGame;
var(Rendering) const bool HiddenEditor;
var(Rendering) const bool bOwnerNoSee;
var(Rendering) const bool bOnlyOwnerSee;
var const bool bXrayNoSee;
var const bool bOnlyXraySee;
var const transient bool DontDrawThisFrame;
var(Rendering) const bool bDisableXboxMSAA;
var const bool bLevelHidden;
var(Rendering) const bool bIgnoreOwnerHidden;
var(Rendering) bool bUseAsOccluder;
var(Rendering) bool bUseAsOccluderAutomatic;
var(Rendering) bool bAllowOcclusionTesting;
var(Rendering) bool bAllowApproximateOcclusion;
var bool bFirstFrameOcclusion;
var bool bIgnoreNearPlaneIntersection;
var bool bSelectable;
var(Rendering) const bool bForceMipStreaming;
var(Rendering) const bool bAcceptsStaticDecals;
var(Rendering) const bool bAcceptsDynamicDecals;
var native const transient bool bIsRefreshingDecals;
var native transient bool bAllowDecalAutomaticReAttach;
var(Rendering) const bool bAcceptsFoliage;
var(Rendering) bool bContributesToLightEnvironmentBounds;
var(Lighting) bool CastShadow;
var(Lighting) const bool bForceDirectLightMap;
var(Lighting) bool bCastDynamicShadow;
var(Lighting) bool bSelfShadowOnly;
var(Lighting) bool bCastHiddenShadow;
var(Lighting) const bool bAcceptsLights;
var(Lighting) const bool bAcceptsDynamicLights;
var(Lighting) const bool bUsePrecomputedShadows;
var(Lighting) bool bCullModulatedShadowOnBackfaces;
var(Lighting) bool bCullModulatedShadowOnEmissive;
var(Lighting) bool bAllowAmbientOcclusion;
var const bool CollideActors;
var const bool AlwaysCheckCollision;
var const bool BlockActors;
var const bool BlockZeroExtent;
var const bool BlockNonZeroExtent;
var(Collision) const bool CanBlockCamera;
var(Collision) const bool BlockRigidBody;
var(Collision) const bool BlockRigidBodyPhysX;
var(Physics) const bool bDisableAllRigidBody;
var(Physics) const bool bSkipRBGeomCreation;
var(Physics) const bool bNotifyRigidBodyCollision;
var(Physics) const bool bEnableContactModificationCallback;
var(Physics) const bool bFluidDrain;
var(Physics) const bool bFluidTwoWay;
var(Physics) bool bIgnoreRadialImpulse;
var(Physics) bool bIgnoreRadialForce;
var(Physics) bool bIgnoreForceField;
var(Physics) const bool bUseCompartment;
var private const bool AlwaysLoadOnClient;
var private const bool AlwaysLoadOnServer;
var() bool bIgnoreHiddenActorsMembership;
var native const transient bool bWasSNFiltered;
var() const bool AbsoluteTranslation;
var() const bool AbsoluteRotation;
var() const bool AbsoluteScale;
var(Collision) const PrimitiveComponent.ERBCollisionChannel RBChannel;
var(Rendering) const Scene.ESceneDepthPriorityGroup DepthPriorityGroup;
var(Rendering) const Scene.EDetailMode DetailMode;
var(Physics) byte RBDominanceGroup;
var float MaxNearlyStillSpeed;

// Export UPrimitiveComponent::execSetDisableAllRigidBody(FFrame&, void* const)
native final function SetDisableAllRigidBody(bool bNewDisableAllRigidBody);

// Export UPrimitiveComponent::execAddImpulse(FFrame&, void* const)
native final function AddImpulse(Vector Impulse, optional Vector Position, optional name BoneName, optional bool bVelChange);

// Export UPrimitiveComponent::execAddRadialImpulse(FFrame&, void* const)
native final function AddRadialImpulse(Vector Origin, float Radius, float Strength, PrimitiveComponent.ERadialImpulseFalloff Falloff, optional bool bVelChange);

// Export UPrimitiveComponent::execAddForce(FFrame&, void* const)
native final function AddForce(Vector Force, optional Vector Position, optional name BoneName);

// Export UPrimitiveComponent::execAddRadialForce(FFrame&, void* const)
native final function AddRadialForce(Vector Origin, float Radius, float Strength, PrimitiveComponent.ERadialImpulseFalloff Falloff);

// Export UPrimitiveComponent::execAddTorque(FFrame&, void* const)
native final function AddTorque(Vector Torque, optional name BoneName);

// Export UPrimitiveComponent::execAddTorqueForce(FFrame&, void* const)
native final function AddTorqueForce(Vector Torgue, optional name BoneName);

// Export UPrimitiveComponent::execAddTorqueImpulse(FFrame&, void* const)
native final function AddTorqueImpulse(Vector Torgue, optional name BoneName);

// Export UPrimitiveComponent::execSetRBLinearVelocity(FFrame&, void* const)
native final function SetRBLinearVelocity(Vector NewVel, optional bool bAddToCurrent);

// Export UPrimitiveComponent::execGetRBLinearVelocity(FFrame&, void* const)
native final function Vector GetRBLinearVelocity();

// Export UPrimitiveComponent::execSetRBAngularVelocity(FFrame&, void* const)
native final function SetRBAngularVelocity(Vector NewAngVel, optional bool bAddToCurrent);

// Export UPrimitiveComponent::execRetardRBLinearVelocity(FFrame&, void* const)
native final function RetardRBLinearVelocity(Vector RetardDir, float VelScale);

// Export UPrimitiveComponent::execSetRBPosition(FFrame&, void* const)
native final function SetRBPosition(Vector NewPos, optional name BoneName);

// Export UPrimitiveComponent::execSetRBRotation(FFrame&, void* const)
native final function SetRBRotation(Rotator NewRot, optional name BoneName);

// Export UPrimitiveComponent::execWakeRigidBody(FFrame&, void* const)
native final function WakeRigidBody(optional name BoneName);

// Export UPrimitiveComponent::execPutRigidBodyToSleep(FFrame&, void* const)
native final function PutRigidBodyToSleep(optional name BoneName);

// Export UPrimitiveComponent::execRigidBodyIsFullyDynamic(FFrame&, void* const)
native function bool RigidBodyIsFullyDynamic(optional name BoneName);

// Export UPrimitiveComponent::execRigidBodyIsAwake(FFrame&, void* const)
native function bool RigidBodyIsAwake(optional name BoneName);

// Export UPrimitiveComponent::execRigidBodyIsNearlyStill(FFrame&, void* const)
native function bool RigidBodyIsNearlyStill(optional name BoneName);

// Export UPrimitiveComponent::execSetBlockRigidBody(FFrame&, void* const)
native final function SetBlockRigidBody(bool bNewBlockRigidBody);

// Export UPrimitiveComponent::execSetRBCollidesWithChannel(FFrame&, void* const)
native final function SetRBCollidesWithChannel(PrimitiveComponent.ERBCollisionChannel Channel, bool bNewCollides);

// Export UPrimitiveComponent::execSetRBCollisionChannels(FFrame&, void* const)
native final function SetRBCollisionChannels(RBCollisionChannelContainer Channels);

// Export UPrimitiveComponent::execSetRBChannel(FFrame&, void* const)
native final function SetRBChannel(PrimitiveComponent.ERBCollisionChannel Channel);

// Export UPrimitiveComponent::execSetNotifyRigidBodyCollision(FFrame&, void* const)
native final function SetNotifyRigidBodyCollision(bool bNewNotifyRigidBodyCollision);

// Export UPrimitiveComponent::execSetContactModification(FFrame&, void* const)
native final function SetContactModification(bool isEnableContactModificationCallback);

// Export UPrimitiveComponent::execInitRBPhys(FFrame&, void* const)
native final function InitRBPhys();

// Export UPrimitiveComponent::execSetPhysMaterialOverride(FFrame&, void* const)
native final function SetPhysMaterialOverride(PhysicalMaterial NewPhysMaterial);

// Export UPrimitiveComponent::execGetRootBodyInstance(FFrame&, void* const)
native final function RB_BodyInstance GetRootBodyInstance();

// Export UPrimitiveComponent::execSetRBDominanceGroup(FFrame&, void* const)
native final function SetRBDominanceGroup(byte InDomGroup);

// Export UPrimitiveComponent::execSetHidden(FFrame&, void* const)
native final function SetHidden(bool NewHidden);

// Export UPrimitiveComponent::execSetHiddenEditor(FFrame&, void* const)
native final function SetHiddenEditor(bool NewHidden);

// Export UPrimitiveComponent::execSetOwnerNoSee(FFrame&, void* const)
native final function SetOwnerNoSee(bool bNewOwnerNoSee);

// Export UPrimitiveComponent::execSetOnlyOwnerSee(FFrame&, void* const)
native final function SetOnlyOwnerSee(bool bNewOnlyOwnerSee);

// Export UPrimitiveComponent::execSetXrayNoSee(FFrame&, void* const)
native final function SetXrayNoSee(bool bNewXrayNoSee);

// Export UPrimitiveComponent::execSetOnlyXraySee(FFrame&, void* const)
native final function SetOnlyXraySee(bool bNewOnlyXraySee);

// Export UPrimitiveComponent::execSetDontDrawThisFrame(FFrame&, void* const)
native final function SetDontDrawThisFrame(bool bNewDontDrawThisFrame);

// Export UPrimitiveComponent::execSetLevelHidden(FFrame&, void* const)
native final function SetLevelHidden(bool bNewLevelHidden);

// Export UPrimitiveComponent::execSetIgnoreOwnerHidden(FFrame&, void* const)
native final function SetIgnoreOwnerHidden(bool bNewIgnoreOwnerHidden);

// Export UPrimitiveComponent::execSetShadowParent(FFrame&, void* const)
native final function SetShadowParent(PrimitiveComponent NewShadowParent);

// Export UPrimitiveComponent::execSetLightEnvironment(FFrame&, void* const)
native final function SetLightEnvironment(LightEnvironmentComponent NewLightEnvironment);

// Export UPrimitiveComponent::execSetCullDistance(FFrame&, void* const)
native final function SetCullDistance(float NewCullDistance);

// Export UPrimitiveComponent::execSetLightingChannels(FFrame&, void* const)
native final function SetLightingChannels(LightingChannelContainer NewLightingChannels);

// Export UPrimitiveComponent::execSetDepthPriorityGroup(FFrame&, void* const)
native final function SetDepthPriorityGroup(Scene.ESceneDepthPriorityGroup NewDepthPriorityGroup);

// Export UPrimitiveComponent::execSetTraceBlocking(FFrame&, void* const)
native final function SetTraceBlocking(bool NewBlockZeroExtent, bool NewBlockNonZeroExtent);

// Export UPrimitiveComponent::execSetActorCollision(FFrame&, void* const)
native final function SetActorCollision(bool NewCollideActors, bool NewBlockActors, optional bool NewAlwaysCheckCollision);

// Export UPrimitiveComponent::execSetTranslation(FFrame&, void* const)
native function SetTranslation(Vector NewTranslation);

// Export UPrimitiveComponent::execSetRotation(FFrame&, void* const)
native function SetRotation(Rotator NewRotation);

// Export UPrimitiveComponent::execSetScale(FFrame&, void* const)
native function SetScale(float NewScale);

// Export UPrimitiveComponent::execSetScale3D(FFrame&, void* const)
native function SetScale3D(Vector NewScale3D);

// Export UPrimitiveComponent::execSetAbsolute(FFrame&, void* const)
native function SetAbsolute(optional bool NewAbsoluteTranslation, optional bool NewAbsoluteRotation, optional bool NewAbsoluteScale);

final function Vector GetPosition()
{
    local Vector Position;

    Position.X = LocalToWorld.WPlane.X;
    Position.Y = LocalToWorld.WPlane.Y;
    Position.Z = LocalToWorld.WPlane.Z;
    return Position;
    //return ReturnValue;    
}

// Export UPrimitiveComponent::execGetRotation(FFrame&, void* const)
native final function Rotator GetRotation();

// Export UPrimitiveComponent::execGetHasFallenOutOfWorld(FFrame&, void* const)
native function bool GetHasFallenOutOfWorld();

// Export UPrimitiveComponent::execGetPhysicalMaterial(FFrame&, void* const)
native function PhysicalMaterial GetPhysicalMaterial(optional int BodyIndex);

defaultproperties
{
    CullArea=10000000000000.0000000
    CullAreaMultiplier=1.0000000
    MotionBlurScale=1.0000000
    Scale=1.0000000
    Scale3D=(X=1.0000000,Y=1.0000000,Z=1.0000000)
    bAllowCullDistanceVolume=true
    bUseAsOccluderAutomatic=true
    bAllowOcclusionTesting=true
    bSelectable=true
    bAcceptsFoliage=true
    bContributesToLightEnvironmentBounds=true
    bCastDynamicShadow=true
    bAcceptsDynamicLights=true
    bAllowAmbientOcclusion=true
    CanBlockCamera=true
    AlwaysLoadOnClient=true
    AlwaysLoadOnServer=true
    DepthPriorityGroup=SDPG_World
    RBDominanceGroup=15
    MaxNearlyStillSpeed=15.0000000
}
