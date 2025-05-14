class PhysicsVolume extends Volume
    native
    nativereplication
    placeable;

struct CheckpointRecord
{
    var bool bPainCausing;

    structdefaultproperties
    {
        bPainCausing=false
    }
};

var() interp Vector ZoneVelocity;
var() bool bVelocityAffectsWalking;
var() bool bPainCausing;
var() bool bAIShouldIgnorePain;
var() bool bEntryPain;
var bool BACKUP_bPainCausing;
var() bool bDestructive;
var() bool bNoInventory;
var() bool bMoveProjectiles;
var() bool bBounceVelocity;
var() bool bNeutralZone;
var() bool bCrowdAgentsPlayDeathAnim;
var() bool bPhysicsOnContact;
var bool bWaterVolume;
var bool bWindVolume;
var bool bPerformTorque;
var() float GroundFriction;
var() float TerminalVelocity;
var() float DamagePerSec;
var() class<DamageType> DamageType;
var() int Priority;
var() float FluidFriction;
var() float PainInterval;
var() float RigidBodyDamping;
var() float MaxDampingForce;
var Info PainTimer;
var Controller DamageInstigator;
var PhysicsVolume NextPhysicsVolume;
var Vector ZoneTorque;

cpptext
{
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	void SetZone( UBOOL bTest, UBOOL bForceRefresh );
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	virtual UBOOL WillHurt(APawn *P);
	virtual void CheckForErrors();

	virtual FLOAT GetVolumeRBGravityZ() { return GetGravityZ(); }
}

// Export UPhysicsVolume::execGetGravityZ(FFrame&, void* const)
native function float GetGravityZ();

// Export UPhysicsVolume::execGetZoneVelocityForActor(FFrame&, void* const)
native function Vector GetZoneVelocityForActor(Actor TheActor);

simulated event PostBeginPlay()
{
    super(Actor).PostBeginPlay();
    BACKUP_bPainCausing = bPainCausing;
    // End:0x25
    if(Role < ROLE_Authority)
    {
        return;
    }
    PainTimer = Spawn(Class'VolumeTimer', self);
    //return;    
}

function Reset()
{
    bPainCausing = BACKUP_bPainCausing;
    bForceNetUpdate = true;
    //return;    
}

event PhysicsChangedFor(Actor Other)
{
    //return;    
}

event ActorEnteredVolume(Actor Other)
{
    //return;    
}

event ActorLeavingVolume(Actor Other)
{
    //return;    
}

event PawnEnteredVolume(Pawn Other)
{
    //return;    
}

event PawnLeavingVolume(Pawn Other)
{
    //return;    
}

simulated function OnToggle(SeqAct_Toggle inAction)
{
    // End:0x28
    if(!bStatic || RemoteRole > ROLE_None)
    {
        super.OnToggle(inAction);
    }
    // End:0x58
    if(inAction.InputLinks[0].bHasImpulse)
    {
        bPainCausing = BACKUP_bPainCausing;        
    }
    else
    {
        // End:0x83
        if(inAction.InputLinks[1].bHasImpulse)
        {
            bPainCausing = false;            
        }
        else
        {
            // End:0xBE
            if(inAction.InputLinks[2].bHasImpulse)
            {
                bPainCausing = !bPainCausing && BACKUP_bPainCausing;
            }
        }
    }
    //return;    
}

simulated event CollisionChanged()
{
    //return;    
}

function TimerPop(VolumeTimer T)
{
    local Actor A;

    // End:0x77
    if(T == PainTimer)
    {
        // End:0x76
        foreach TouchingActors(Class'Actor', A)
        {
            // End:0x3C
            if(!bPainCausing)
            {
                DamageApexDestructible(A);
                // End:0x75
                continue;
            }
            // End:0x75
            if(A.bCanBeDamaged && !A.bStatic)
            {
                CausePainTo(A);
            }            
        }        
    }
    //return;    
}

simulated event Touch(Actor Other, PrimitiveComponent OtherComp, Vector HitLocation, Vector HitNormal)
{
    super(Actor).Touch(Other, OtherComp, HitLocation, HitNormal);
    // End:0x3C
    if((Other == none) || Other.bStatic)
    {
        return;
    }
    // End:0x85
    if((bNoInventory && DroppedPickup(Other) != none) && Other.Owner == none)
    {
        Other.LifeSpan = 1.5000000;
        return;
    }
    // End:0x14B
    if(bMoveProjectiles && ZoneVelocity != vect(0.0000000, 0.0000000, 0.0000000))
    {
        // End:0xDA
        if(Other.Physics == 6)
        {
            Other.Velocity += ZoneVelocity;            
        }
        else
        {
            // End:0x14B
            if(((Other.Base == none) && Other.IsA('Emitter')) && Other.Physics == 0)
            {
                Other.SetPhysics(6);
                Other.Velocity += ZoneVelocity;
            }
        }
    }
    // End:0x1AE
    if(bPainCausing)
    {
        // End:0x17E
        if(Other.bDestroyInPainVolume)
        {
            Other.VolumeBasedDestroy(self);
            return;
        }
        // End:0x1AB
        if(bEntryPain && Other.bCanBeDamaged)
        {
            CausePainTo(Other);
        }        
    }
    else
    {
        DamageApexDestructible(Other);
    }
    //return;    
}

function CausePainTo(Actor Other)
{
    // End:0xDD
    if(DamagePerSec > float(0))
    {
        // End:0x3E
        if(WorldInfo.bSoftKillZ && Other.Physics != 1)
        {
            return;
        }
        // End:0x9A
        if((DamageType == none) || DamageType == Class'DamageType')
        {
            LogInternal((("No valid damagetype (" $ string(DamageType)) $ ") specified for ") $ PathName(self));
        }
        Other.TakeDamage(int(DamagePerSec * PainInterval), DamageInstigator, Location, vect(0.0000000, 0.0000000, 0.0000000), DamageType,, self);        
    }
    else
    {
        Other.HealDamage(int(-DamagePerSec * PainInterval), DamageInstigator, DamageType);
    }
    //return;    
}

function DamageApexDestructible(Actor TheActor)
{
    local Box Bounds;

    // End:0x7C
    if(ApexDestructibleActor(TheActor) != none)
    {
        GetComponentsBoundingBox(Bounds);
        TheActor.TakeRadiusDamage(DamageInstigator, DamagePerSec * PainInterval, VSize(Bounds.Max - Bounds.Min) / 2.0000000, DamageType, 0.0000000, Location, false, self);
    }
    //return;    
}

function ModifyPlayer(Pawn PlayerPawn)
{
    //return;    
}

function NotifyPawnBecameViewTarget(Pawn P, PlayerController PC)
{
    //return;    
}

function OnSetDamageInstigator(SeqAct_SetDamageInstigator Action)
{
    DamageInstigator = Action.GetController(Action.DamageInstigator);
    //return;    
}

function bool ShouldSaveForCheckpoint()
{
    return bPainCausing != BACKUP_bPainCausing;
    //return ReturnValue;    
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
    Record.bPainCausing = bPainCausing;
    //return;    
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
    bPainCausing = Record.bPainCausing;
    //return;    
}

defaultproperties
{
    bVelocityAffectsWalking=true
    bEntryPain=true
    GroundFriction=8.0000000
    TerminalVelocity=3500.0000000
    DamageType=Class'DamageType'
    FluidFriction=0.3000000
    PainInterval=1.0000000
    MaxDampingForce=1000000.0000000
    // Reference: BrushComponent'Default__PhysicsVolume.BrushComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'BrushComponent0'
    // Archetype: BrushComponent'Default__Volume.BrushComponent0'
    begin object name="BrushComponent0"
        BlockZeroExtent=true
        bDisableAllRigidBody=false
    end object
    BrushComponent=BrushComponent0
    bAlwaysRelevant=true
    bOnlyDirtyReplication=true
    bForceAllowKismetModification=true
    Components[0]=BrushComponent0
    CollisionComponent=BrushComponent0
}
