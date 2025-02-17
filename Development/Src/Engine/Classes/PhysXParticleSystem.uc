/*=============================================================================
	PhysXParticleSystem.uc: PhysX Emitter Source.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

class PhysXParticleSystem extends Object
	native(Particle)
	hidecategories(Object);

enum ESimulationMethod
{
	ESM_SPH,
	ESM_NO_PARTICLE_INTERACTION,
	ESM_MIXED_MODE
};

enum EPacketSizeMultiplier
{
	EPSM_4,
	EPSM_8,
	EPSM_16,
	EPSM_32,
	EPSM_64,
	EPSM_128
};

//=============================================================================
//	PhysX SDK Parameters (Basic parameters for colliding particles) 
//=============================================================================

var(Buffer) int MaxParticles;
var(Collision) float CollisionDistance;
var(Collision) float RestitutionWithStaticShapes;
var(Collision) float RestitutionWithDynamicShapes;
var(Collision) float FrictionWithStaticShapes;
var(Collision) float FrictionWithDynamicShapes;
var(Collision) bool bDynamicCollision;
var(Dynamics) float MaxMotionDistance;
var(Dynamics) float Damping;
var(Dynamics) vector ExternalAcceleration;
var(Dynamics) bool bDisableGravity;

//=============================================================================
//	More PhysX SDK Params. (SPH particles and parallelization settings, ect...) 
//=============================================================================

var(SdkExpert) bool bStaticCollision;
var(SdkExpert) bool bTwoWayCollision;
var(SdkExpert) ESimulationMethod SimulationMethod;
var(SdkExpert) EPacketSizeMultiplier PacketSizeMultiplier;
var(SdkExpert) float RestParticleDistance;
var(SdkExpert) float RestDensity;
var(SdkExpert) float KernelRadiusMultiplier;
var(SdkExpert) float Stiffness;
var(SdkExpert) float Viscosity;
var(SdkExpert) float CollisionResponseCoefficient;

//=============================================================================
//	Non-exposed state 
//=============================================================================

var transient bool bDestroy;
var transient bool bSyncFailed;
var transient bool bIsInGame;

var native pointer CascadeScene {class FRBPhysScene};
var native pointer PSys {class FPhysXParticleSystem};

cpptext
{
    virtual void FinishDestroy();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PreEditChange(UProperty* PropertyAboutToChange);

    void Tick(FLOAT deltaTime);
    void TickEditor(FLOAT deltaTime);
    void RemovedFromScene();
    void RemoveSpawnInstance(struct FParticleEmitterInstance*);
    UBOOL SyncConnect();
    UBOOL SyncDisconnect();
    UBOOL TryConnect();
	void SyncPhysXData();
	FRBPhysScene* GetScene();
}

defaultproperties
{
    //user properties
    MaxParticles = 32767
    FrictionWithStaticShapes = 0.05f
    FrictionWithDynamicShapes = 0.5f
    RestitutionWithStaticShapes = 0.5f
    RestitutionWithDynamicShapes = 0.5f
    bDynamicCollision = true
    CollisionDistance = 10.0f
    bDisableGravity = false
    ExternalAcceleration = (X=0,Y=0,Z=0)
    Damping = 0.0f
    MaxMotionDistance = 64.0f

	//sdk expert
    bStaticCollision = true
    bTwoWayCollision = false
    SimulationMethod = ESM_NO_PARTICLE_INTERACTION
    PacketSizeMultiplier = EPSM_16
    RestParticleDistance = 64.0f
    RestDensity = 1000.0f
    KernelRadiusMultiplier = 2.0f
    Stiffness = 20.0f
    Viscosity = 6.0f
    CollisionResponseCoefficient = 0.2f

    //non gui
    bDestroy = false
    bSyncFailed = false
    bIsInGame = false

}
