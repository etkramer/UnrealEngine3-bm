class StaticMeshComponent extends MeshComponent
    native
    editinlinenew
    hidecategories(Object)
    noexport;

struct StaticMeshComponentLODInfo
{
    var private const array<ShadowMap2D> ShadowMaps;
    var private const array<Object> ShadowVertexBuffers;
    var private native const Pointer LightMap{FLightMap};

    structdefaultproperties
    {
        ShadowMaps=none
        ShadowVertexBuffers=none
    }
};

var() byte ForcedLodModel;
var byte PreviousLODLevel;
var() const StaticMesh StaticMesh;
var() Color WireframeColor;
var() const int OverriddenLightMapResolution;
var(AdvancedLighting) const int SubDivisionStepSize;
var(AdvancedLighting) const int MinSubDivisions;
var(AdvancedLighting) const int MaxSubDivisions;
var const array<Guid> IrrelevantLights;
var private native const array<StaticMeshComponentLODInfo> LODData;
var() bool bIgnoreInstanceForTextureStreaming;
var() const bool bOverrideLightMapResolution;
var(AdvancedLighting) const bool bUseSubDivisions;
var const transient bool bForceStaticDecals;

// Export UStaticMeshComponent::execSetStaticMesh(FFrame&, void* const)
native simulated function bool SetStaticMesh(StaticMesh NewMesh, optional bool bForce);

// Export UStaticMeshComponent::execDisableRBCollisionWithSMC(FFrame&, void* const)
native simulated function DisableRBCollisionWithSMC(StaticMeshComponent OtherSMC, bool bDisabled);

// Export UStaticMeshComponent::execSetForceStaticDecals(FFrame&, void* const)
native final function SetForceStaticDecals(bool bInForceStaticDecals);

// Export UStaticMeshComponent::execGetUseSimpleBoxCollision(FFrame&, void* const)
native function bool GetUseSimpleBoxCollision();

// Export UStaticMeshComponent::execGetUseSimpleLineCollision(FFrame&, void* const)
native function bool GetUseSimpleLineCollision();

defaultproperties
{
    WireframeColor=(R=0,G=255,B=255,A=255)
    SubDivisionStepSize=16
    MinSubDivisions=2
    MaxSubDivisions=8
    bOverrideLightMapResolution=true
    bUseSubDivisions=true
    bAcceptsStaticDecals=true
    bAcceptsDynamicDecals=true
    CollideActors=true
    BlockActors=true
    BlockZeroExtent=true
    BlockNonZeroExtent=true
    BlockRigidBody=true
    TickGroup=TG_PreAsyncWork
}
