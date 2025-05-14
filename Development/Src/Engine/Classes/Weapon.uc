class Weapon extends Inventory
    abstract
    native
    config(Game)
    notplaceable;

enum EWeaponFireType
{
    EWFT_InstantHit,                // 0
    EWFT_Projectile,                // 1
    EWFT_Custom,                    // 2
    EWFT_None,                      // 3
    EWFT_MAX                        // 4
};

var byte CurrentFireMode;
var array<name> FiringStatesArray;
var array<Weapon.EWeaponFireType> WeaponFireTypes;
var array< class<Projectile> > WeaponProjectiles;
var() array<float> FireInterval;
var() array<float> Spread;
var() array<float> InstantHitDamage;
var() array<float> InstantHitMomentum;
var array< class<DamageType> > InstantHitDamageTypes;
var() float EquipTime;
var() float PutDownTime;
var() Vector FireOffset;
var bool bWeaponPutDown;
var bool bCanThrow;
var bool bWasOptionalSet;
var bool bWasDoNotActivate;
var bool bInstantHit;
var bool bMeleeWeapon;
var bool bPendingClientWeaponSet;
var() float WeaponRange;
var() export editinline MeshComponent Mesh;
var() float DefaultAnimSpeed;
var databinding config float Priority;
var AIController AIController;
var array<byte> ShouldFireOnRelease;
var float AIRating;
var float CachedMaxRange;

simulated event Destroyed()
{
    DetachWeapon();
    super.Destroyed();
    //return;    
}

function ItemRemovedFromInvManager()
{
    LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
    GotoState('Inactive');
    ForceEndFire();
    DetachWeapon();
    ClientWeaponThrown();
    super.ItemRemovedFromInvManager();
    // End:0xAD
    if(IsActiveWeapon())
    {
        Instigator.Weapon = none;
    }
    //return;    
}

simulated function bool IsActiveWeapon()
{
    // End:0x21
    if(InvManager != none)
    {
        return InvManager.IsActiveWeapon(self);
    }
    return false;
    //return ReturnValue;    
}

function HolderDied()
{
    ServerStopFire(CurrentFireMode);
    //return;    
}

simulated function bool DoOverrideNextWeapon()
{
    return false;
    //return ReturnValue;    
}

simulated function bool DoOverridePrevWeapon()
{
    return false;
    //return ReturnValue;    
}

function DropFrom(Vector StartLocation, Vector StartVelocity)
{
    // End:0x11
    if(!CanThrow())
    {
        return;
    }
    GotoState('Inactive');
    ForceEndFire();
    DetachWeapon();
    super.DropFrom(StartLocation, StartVelocity);
    AIController = none;
    //return;    
}

simulated function bool CanThrow()
{
    return bCanThrow;
    //return ReturnValue;    
}

reliable client simulated function ClientWeaponThrown()
{
    LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
    GotoState('Inactive');
    // End:0x9E
    if((Instigator != none) && Instigator.Weapon == self)
    {
        Instigator.Weapon = none;
    }
    ForceEndFire();
    DetachWeapon();
    //return;    
}

simulated event bool IsFiring()
{
    return false;
    //return ReturnValue;    
}

simulated function bool DenyClientWeaponSet()
{
    return false;
    //return ReturnValue;    
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
    local array<string> DebugInfo;
    local int I;

    GetWeaponDebug(DebugInfo);
    HUD.Canvas.SetDrawColor(0, 255, 0);
    I = 0;
    J0x37:

    // End:0xAF [Loop If]
    if(I < DebugInfo.Length)
    {
        HUD.Canvas.DrawText("  " @ DebugInfo[I]);
        out_YPos += out_YL;
        HUD.Canvas.SetPos(4.0000000, out_YPos);
        I++;
        // [Loop Continue]
        goto J0x37;
    }
    //return;    
}

simulated function GetWeaponDebug(out array<string> DebugInfo)
{
    local string T;
    local int I;

    DebugInfo[DebugInfo.Length] = (((((("Weapon:" $ (GetItemName(string(self)))) @ "State:") $ string(GetStateName())) @ "Instigator:") $ string(Instigator)) @ "Owner:") $ string(Owner);
    DebugInfo[DebugInfo.Length] = (((("IsFiring():" $ string(IsFiring())) @ "CurrentFireMode:") $ string(CurrentFireMode)) @ "bWeaponPutDown:") $ string(bWeaponPutDown);
    // End:0x141
    if(Instigator != none)
    {
        DebugInfo[DebugInfo.Length] = (((("ShotCount:" $ string(Instigator.ShotCount)) @ "FlashCount:") $ string(Instigator.FlashCount)) @ "FlashLocation:") $ string(Instigator.FlashLocation);
    }
    T = "PendingFires:";
    I = 0;
    J0x15D:

    // End:0x1A4 [Loop If]
    if(I < InvManager.PendingFire.Length)
    {
        T = (T $ string(PendingFire(I))) $ " ";
        I++;
        // [Loop Continue]
        goto J0x15D;
    }
    DebugInfo[DebugInfo.Length] = T;
    // End:0x284
    if(Timers.Length > 0)
    {
        I = 0;
        J0x1C9:

        // End:0x284 [Loop If]
        if(I < Timers.Length)
        {
            DebugInfo[DebugInfo.Length] = (((("Timer" @ string(Timers[I].FuncName)) @ string(Timers[I].Count)) @ string(Timers[I].Rate)) @ string(int((Timers[I].Count / Timers[I].Rate) * float(100)))) $ "%";
            I++;
            // [Loop Continue]
            goto J0x1C9;
        }
    }
    //return;    
}

simulated function DumpWeaponDebugToLog()
{
    local array<string> DebugInfo;
    local int I;

    LogInternal(((((string(WorldInfo.TimeSeconds) @ string(self)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "");
    GetWeaponDebug(DebugInfo);
    I = 0;
    J0x4A:

    // End:0x86 [Loop If]
    if(I < DebugInfo.Length)
    {
        LogInternal("Weapondebuginfo " $ DebugInfo[I]);
        I++;
        // [Loop Continue]
        goto J0x4A;
    }
    //return;    
}

function ConsumeAmmo(byte FireModeNum)
{
    //return;    
}

function int AddAmmo(int Amount)
{
    //return ReturnValue;    
}

simulated function bool HasAmmo(byte FireModeNum, optional int Amount)
{
    return true;
    //return ReturnValue;    
}

simulated function bool HasAnyAmmo()
{
    return true;
    //return ReturnValue;    
}

simulated function bool PendingFire(int FireMode)
{
    // End:0x23
    if(InvManager != none)
    {
        return bool(InvManager.PendingFire[FireMode]);
    }
    return false;
    //return ReturnValue;    
}

simulated function SetPendingFire(int FireMode)
{
    // End:0x22
    if(InvManager != none)
    {
        InvManager.PendingFire[FireMode] = 1;
    }
    //return;    
}

simulated function ClearPendingFire(int FireMode)
{
    // End:0x22
    if(InvManager != none)
    {
        InvManager.PendingFire[FireMode] = 0;
    }
    //return;    
}

function class<Projectile> GetProjectileClass()
{
    return ((int(CurrentFireMode) < WeaponProjectiles.Length) ? WeaponProjectiles[int(CurrentFireMode)] : none);
    //return ReturnValue;    
}

simulated function Rotator AddSpread(Rotator BaseAim)
{
    local Vector X, Y, Z;
    local float CurrentSpread, RandY, RandZ;

    CurrentSpread = Spread[int(CurrentFireMode)];
    // End:0x29
    if(CurrentSpread == float(0))
    {
        return BaseAim;        
    }
    else
    {
        GetAxes(BaseAim, X, Y, Z);
        RandY = FRand() - 0.5000000;
        RandZ = Sqrt(0.5000000 - Square(RandY)) * (FRand() - 0.5000000);
        return Rotator((X + ((RandY * CurrentSpread) * Y)) + ((RandZ * CurrentSpread) * Z));
    }
    //return ReturnValue;    
}

simulated function float MaxRange()
{
    local int I;

    // End:0x13
    if(CachedMaxRange > float(0))
    {
        return CachedMaxRange;
    }
    // End:0x27
    if(bInstantHit)
    {
        CachedMaxRange = WeaponRange;
    }
    I = 0;
    J0x2E:

    // End:0x80 [Loop If]
    if(I < WeaponProjectiles.Length)
    {
        // End:0x76
        if(WeaponProjectiles[I] != none)
        {
            CachedMaxRange = FMax(CachedMaxRange, WeaponProjectiles[I].static.GetRange());
        }
        I++;
        // [Loop Continue]
        goto J0x2E;
    }
    return CachedMaxRange;
    //return ReturnValue;    
}

function float GetDamageRadius()
{
    local class<Projectile> CurrentProjectileClass;

    CurrentProjectileClass = GetProjectileClass();
    // End:0x21
    if(CurrentProjectileClass == none)
    {
        return 0.0000000;
    }
    return CurrentProjectileClass.default.DamageRadius;
    //return ReturnValue;    
}

function float GetAIRating()
{
    return AIRating;
    //return ReturnValue;    
}

function float RelativeStrengthVersus(Pawn P, float Dist)
{
    return 0.0000000;
    //return ReturnValue;    
}

simulated function float GetWeaponRating()
{
    // End:0x21
    if(InvManager != none)
    {
        return InvManager.GetWeaponRatingFor(self);
    }
    // End:0x36
    if(!HasAnyAmmo())
    {
        return -1.0000000;
    }
    return 1.0000000;
    //return ReturnValue;    
}

function bool RecommendRangedAttack()
{
    return false;
    //return ReturnValue;    
}

function bool FocusOnLeader(bool bLeaderFiring)
{
    return false;
    //return ReturnValue;    
}

function bool RecommendLongRangedAttack()
{
    return false;
    //return ReturnValue;    
}

function float RangedAttackTime()
{
    return 0.0000000;
    //return ReturnValue;    
}

function bool CanAttack(Actor Other)
{
    return true;
    //return ReturnValue;    
}

function float SuggestAttackStyle()
{
    return 0.0000000;
    //return ReturnValue;    
}

function float SuggestDefenseStyle()
{
    return 0.0000000;
    //return ReturnValue;    
}

function bool FireOnRelease()
{
    return (ShouldFireOnRelease.Length > 0) && int(ShouldFireOnRelease[int(CurrentFireMode)]) != 0;
    //return ReturnValue;    
}

simulated function AnimNodeSequence GetWeaponAnimNodeSeq()
{
    local AnimTree Tree;
    local AnimNodeSequence AnimSeq;
    local SkeletalMeshComponent SkelMesh;

    SkelMesh = SkeletalMeshComponent(Mesh);
    // End:0x8A
    if(SkelMesh != none)
    {
        Tree = AnimTree(SkelMesh.Animations);
        // End:0x6A
        if(Tree != none)
        {
            AnimSeq = AnimNodeSequence(Tree.Children[0].Anim);            
        }
        else
        {
            AnimSeq = AnimNodeSequence(SkelMesh.Animations);
        }
        return AnimSeq;
    }
    return none;
    //return ReturnValue;    
}

simulated function WeaponPlaySound(SoundCue Sound, optional float NoiseLoudness)
{
    // End:0x1B
    if((Sound == none) || Instigator == none)
    {
        return;
    }
    Instigator.PlaySound(Sound, false, true);
    //return;    
}

simulated function PlayWeaponAnimation(name Sequence, float fDesiredDuration, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{
    local AnimNodeSequence WeapNode;
    local AnimTree Tree;

    // End:0x1E
    if(WorldInfo.NetMode == NM_DedicatedServer)
    {
        return;
    }
    // End:0x39
    if(SkelMesh == none)
    {
        SkelMesh = SkeletalMeshComponent(Mesh);
    }
    // End:0x58
    if((SkelMesh == none) || (GetWeaponAnimNodeSeq()) == none)
    {
        return;
    }
    // End:0x8F
    if(fDesiredDuration > 0.0000000)
    {
        SkelMesh.PlayAnim(Sequence, fDesiredDuration, bLoop);        
    }
    else
    {
        Tree = AnimTree(SkelMesh.Animations);
        // End:0xDE
        if(Tree != none)
        {
            WeapNode = AnimNodeSequence(Tree.Children[0].Anim);            
        }
        else
        {
            WeapNode = AnimNodeSequence(SkelMesh.Animations);
        }
        WeapNode.SetAnim(Sequence);
        WeapNode.PlayAnim(bLoop, DefaultAnimSpeed);
    }
    //return;    
}

simulated function StopWeaponAnimation()
{
    local AnimNodeSequence AnimSeq;

    // End:0x1C
    if(WorldInfo.NetMode == NM_DedicatedServer)
    {
        return;
    }
    AnimSeq = GetWeaponAnimNodeSeq();
    // End:0x4B
    if(AnimSeq != none)
    {
        AnimSeq.StopAnim();
    }
    //return;    
}

simulated function PlayFireEffects(byte FireModeNum, optional Vector HitLocation)
{
    //return;    
}

simulated function StopFireEffects(byte FireModeNum)
{
    //return;    
}

simulated function float GetFireInterval(byte FireModeNum)
{
    return ((FireInterval[int(FireModeNum)] > float(0)) ? FireInterval[int(FireModeNum)] : 0.0100000);
    //return ReturnValue;    
}

simulated function TimeWeaponFiring(byte FireModeNum)
{
    // End:0x32
    if(!IsTimerActive('RefireCheckTimer'))
    {
        SetTimer(GetFireInterval(FireModeNum), true, 'RefireCheckTimer');
    }
    //return;    
}

simulated function RefireCheckTimer()
{
    //return;    
}

simulated function TimeWeaponPutDown()
{
    SetTimer(((PutDownTime > float(0)) ? PutDownTime : 0.0100000), false, 'WeaponIsDown');
    //return;    
}

simulated function TimeWeaponEquipping()
{
    SetTimer(((EquipTime > float(0)) ? EquipTime : 0.0100000), false, 'WeaponEquipped');
    //return;    
}

simulated function Activate()
{
    // End:0x1D
    if(!IsFiring())
    {
        GotoState('WeaponEquipping');
    }
    //return;    
}

simulated function PutDownWeapon()
{
    GotoState('WeaponPuttingDown');
    //return;    
}

function bool DenyPickupQuery(class<Inventory> ItemClass, Actor Pickup)
{
    // End:0x11
    if(ItemClass == Class)
    {
        return true;
    }
    return false;
    //return ReturnValue;    
}

simulated function WeaponEmpty()
{
    //return;    
}

simulated function IncrementFlashCount()
{
    // End:0x25
    if(Instigator != none)
    {
        Instigator.IncrementFlashCount(self, CurrentFireMode);
    }
    //return;    
}

simulated function ClearFlashCount()
{
    // End:0x20
    if(Instigator != none)
    {
        Instigator.ClearFlashCount(self);
    }
    //return;    
}

function SetFlashLocation(Vector HitLocation)
{
    // End:0x2A
    if(Instigator != none)
    {
        Instigator.SetFlashLocation(self, CurrentFireMode, HitLocation);
    }
    //return;    
}

function ClearFlashLocation()
{
    // End:0x20
    if(Instigator != none)
    {
        Instigator.ClearFlashLocation(self);
    }
    //return;    
}

simulated function AttachWeaponTo(SkeletalMeshComponent MeshCpnt, optional name SocketName)
{
    WarnInternal("AttachWeaponTo not allowed.");
    //return;    
}

simulated function DetachWeapon()
{
    //return;    
}

simulated function GetViewAxes(out Vector XAxis, out Vector YAxis, out Vector ZAxis)
{
    local Rotator AimRot;

    AimRot = Instigator.GetBaseAimRotation();
    GetAxes(AimRot, XAxis, YAxis, ZAxis);
    //return;    
}

simulated function float AdjustFOVAngle(float FOVAngle)
{
    return FOVAngle;
    //return ReturnValue;    
}

simulated function float GetZoomMagnification()
{
    return 1.0000000;
    //return ReturnValue;    
}

reliable client simulated function ClientGivenTo(Pawn NewOwner, bool bDoNotActivate)
{
    super.ClientGivenTo(NewOwner, bDoNotActivate);
    ClientWeaponSet(true, bDoNotActivate);
    //return;    
}

reliable client simulated function ClientWeaponSet(bool bOptionalSet, optional bool bDoNotActivate)
{
    LogInternal(((((((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "bOptionalSet:") @ string(bOptionalSet)) @ "bDoNotActivate:") @ string(bDoNotActivate)) @ "Instigator:") @ string(Instigator)) @ "InvManager:") @ string(InvManager), 'Inventory');
    bWasOptionalSet = bOptionalSet;
    bWasDoNotActivate = bDoNotActivate;
    // End:0x187
    if(Instigator == none)
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "Instigator == None, going to PendingClientWeaponSet", 'Inventory');
        GotoState('PendingClientWeaponSet');
        return;
    }
    // End:0x232
    if(InvManager == none)
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "InvManager == None, going to PendingClientWeaponSet", 'Inventory');
        GotoState('PendingClientWeaponSet');
        return;
    }
    InvManager.ClientWeaponSet(self, bOptionalSet, bDoNotActivate);
    //return;    
}

simulated function WeaponCalcCamera(float fDeltaTime, out Vector out_CamLoc, out Rotator out_CamRot)
{
    //return;    
}

simulated function StartFire(byte FireModeNum)
{
    // End:0x50
    if((Instigator == none) || !Instigator.bNoWeaponFiring)
    {
        // End:0x41
        if(Role < ROLE_Authority)
        {
            ServerStartFire(FireModeNum);
        }
        BeginFire(FireModeNum);
    }
    //return;    
}

reliable server function ServerStartFire(byte FireModeNum)
{
    // End:0x31
    if((Instigator == none) || !Instigator.bNoWeaponFiring)
    {
        BeginFire(FireModeNum);
    }
    //return;    
}

simulated function BeginFire(byte FireModeNum)
{
    LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "FireModeNum:") @ string(FireModeNum), 'Inventory');
    SetPendingFire(int(FireModeNum));
    //return;    
}

simulated function StopFire(byte FireModeNum)
{
    EndFire(FireModeNum);
    // End:0x2E
    if(Role < ROLE_Authority)
    {
        ServerStopFire(FireModeNum);
    }
    //return;    
}

reliable server function ServerStopFire(byte FireModeNum)
{
    EndFire(FireModeNum);
    //return;    
}

simulated function EndFire(byte FireModeNum)
{
    ClearPendingFire(int(FireModeNum));
    //return;    
}

simulated function ForceEndFire()
{
    local int I;

    // End:0x59
    if(InvManager != none)
    {
        I = 0;
        J0x12:

        // End:0x59 [Loop If]
        if(I < InvManager.PendingFire.Length)
        {
            // End:0x4F
            if(PendingFire(I))
            {
                EndFire(byte(I));
            }
            I++;
            // [Loop Continue]
            goto J0x12;
        }
    }
    //return;    
}

simulated function SendToFiringState(byte FireModeNum)
{
    // End:0x84
    if(int(FireModeNum) >= FiringStatesArray.Length)
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "Invalid FireModeNum", 'Inventory');
        return;
    }
    // End:0xBB
    if((FiringStatesArray[int(FireModeNum)] == 'None') || int(WeaponFireTypes[int(FireModeNum)]) == 3)
    {
        return;
    }
    SetCurrentFireMode(FireModeNum);
    LogInternal((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ string(FireModeNum)) @ "Sending to state:") @ string(FiringStatesArray[int(FireModeNum)]), 'Inventory');
    GotoState(FiringStatesArray[int(FireModeNum)]);
    //return;    
}

simulated function SetCurrentFireMode(byte FiringModeNum)
{
    CurrentFireMode = FiringModeNum;
    // End:0x2F
    if(Instigator != none)
    {
        Instigator.SetFiringMode(FiringModeNum);
    }
    //return;    
}

simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
    //return;    
}

simulated function FireAmmunition()
{
    ConsumeAmmo(CurrentFireMode);
    switch(WeaponFireTypes[int(CurrentFireMode)])
    {
        // End:0x31
        case 0:
            InstantFire();
            // End:0x58
            break;
        // End:0x43
        case 1:
            ProjectileFire();
            // End:0x58
            break;
        // End:0x55
        case 2:
            CustomFire();
            // End:0x58
            break;
        // End:0xFFFF
        default:
            break;
    }
    NotifyWeaponFired(CurrentFireMode);
    //return;    
}

simulated function Rotator GetAdjustedAim(Vector StartFireLoc)
{
    local Rotator R;

    // End:0x2B
    if(Instigator != none)
    {
        R = Instigator.GetAdjustedAimFor(self, StartFireLoc);
    }
    return AddSpread(R);
    //return ReturnValue;    
}

simulated event float GetTraceRange()
{
    return WeaponRange;
    //return ReturnValue;    
}

simulated function Actor GetTraceOwner()
{
    return ((Instigator != none) ? Instigator : self);
    //return ReturnValue;    
}

simulated function ImpactInfo CalcWeaponFire(Vector StartTrace, Vector EndTrace, optional out array<ImpactInfo> ImpactList, optional Vector Extent)
{
    local Vector HitLocation, HitNormal, Dir;
    local Actor HitActor;
    local TraceHitInfo HitInfo;
    local ImpactInfo CurrentImpact;
    local PortalTeleporter Portal;
    local float HitDist;
    local bool bToggledBlockActors;

    HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, true, Extent, HitInfo, 1);
    // End:0x50
    if(HitActor == none)
    {
        HitLocation = EndTrace;
    }
    CurrentImpact.HitActor = HitActor;
    CurrentImpact.HitLocation = HitLocation;
    CurrentImpact.HitNormal = HitNormal;
    CurrentImpact.RayDir = Normal(EndTrace - StartTrace);
    CurrentImpact.StartTrace = StartTrace;
    CurrentImpact.HitInfo = HitInfo;
    ImpactList[ImpactList.Length] = CurrentImpact;
    // End:0x2A5
    if(HitActor != none)
    {
        // End:0x1DB
        if(PassThroughDamage(HitActor))
        {
            HitActor.bProjTarget = !HitActor.bProjTarget;
            // End:0x169
            if(HitActor.bBlockActors)
            {
                HitActor.SetCollision(HitActor.bCollideActors, false);
                bToggledBlockActors = true;
            }
            CurrentImpact = CalcWeaponFire(HitLocation, EndTrace, ImpactList, Extent);
            HitActor.bProjTarget = !HitActor.bProjTarget;
            // End:0x1D8
            if(bToggledBlockActors)
            {
                HitActor.SetCollision(HitActor.bCollideActors, true);
            }            
        }
        else
        {
            Portal = PortalTeleporter(HitActor);
            // End:0x2A5
            if((Portal != none) && Portal.SisterPortal != none)
            {
                Dir = EndTrace - StartTrace;
                HitDist = VSize(HitLocation - StartTrace);
                StartTrace = Portal.TransformHitLocation(HitLocation);
                EndTrace = StartTrace + Portal.TransformVectorDir(Normal(Dir) * (VSize(Dir) - HitDist));                
                CalcWeaponFire(StartTrace, EndTrace, ImpactList, Extent);
            }
        }
    }
    return CurrentImpact;
    //return ReturnValue;    
}

simulated function bool PassThroughDamage(Actor HitActor)
{
    return !HitActor.bBlockActors && HitActor.IsA('Trigger') || HitActor.IsA('TriggerVolume');
    //return ReturnValue;    
}

simulated function InstantFire()
{
    local Vector StartTrace, EndTrace;
    local array<ImpactInfo> ImpactList;
    local int Idx;
    local ImpactInfo RealImpact;

    StartTrace = Instigator.GetWeaponStartTraceLocation();
    EndTrace = StartTrace + (Vector(GetAdjustedAim(StartTrace)) * (GetTraceRange()));
    RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);
    // End:0x8F
    if(Role == ROLE_Authority)
    {
        SetFlashLocation(RealImpact.HitLocation);
    }
    Idx = 0;
    J0x96:

    // End:0xCA [Loop If]
    if(Idx < ImpactList.Length)
    {
        ProcessInstantHit(CurrentFireMode, ImpactList[Idx]);
        Idx++;
        // [Loop Continue]
        goto J0x96;
    }
    //return;    
}

simulated function ProcessInstantHit(byte FiringMode, ImpactInfo Impact)
{
    // End:0xA0
    if(Impact.HitActor != none)
    {
        Impact.HitActor.TakeDamage(int(InstantHitDamage[int(CurrentFireMode)]), Instigator.Controller, Impact.HitLocation, InstantHitMomentum[int(FiringMode)] * Impact.RayDir, InstantHitDamageTypes[int(FiringMode)], Impact.HitInfo, self);
    }
    //return;    
}

simulated function Projectile ProjectileFire()
{
    local Vector StartTrace, EndTrace, RealStartLoc, AimDir;
    local ImpactInfo TestImpact;
    local Projectile SpawnedProjectile;

    IncrementFlashCount();
    // End:0x12A
    if(Role == ROLE_Authority)
    {
        StartTrace = Instigator.GetWeaponStartTraceLocation();
        AimDir = Vector(GetAdjustedAim(StartTrace));
        RealStartLoc = GetPhysicalFireStartLoc(AimDir);
        // End:0xC9
        if(StartTrace != RealStartLoc)
        {
            EndTrace = StartTrace + (AimDir * (GetTraceRange()));
            TestImpact = CalcWeaponFire(StartTrace, EndTrace);
            AimDir = Normal(TestImpact.HitLocation - RealStartLoc);
        }
        SpawnedProjectile = Spawn(GetProjectileClass(), self,, RealStartLoc);
        // End:0x124
        if((SpawnedProjectile != none) && !SpawnedProjectile.bDeleteMe)
        {
            SpawnedProjectile.Init(AimDir);
        }
        return SpawnedProjectile;
    }
    return none;
    //return ReturnValue;    
}

simulated function CustomFire()
{
    //return;    
}

simulated event Vector GetMuzzleLoc()
{
    // End:0x3E
    if(Instigator != none)
    {
        return Instigator.GetPawnViewLocation() + (FireOffset >> Instigator.GetViewRotation());
    }
    return Location;
    //return ReturnValue;    
}

// Export UWeapon::execGetPhysicalFireStartLoc(FFrame&, void* const)
native simulated event Vector GetPhysicalFireStartLoc(optional Vector AimDir);

simulated function bool TryPutDown()
{
    bWeaponPutDown = true;
    return true;
    //return ReturnValue;    
}

simulated function HandleFinishedFiring()
{
    GotoState('Active');
    //return;    
}

function NotifyWeaponFired(byte FireMode)
{
    // End:0x25
    if(AIController != none)
    {
        AIController.NotifyWeaponFired(self, FireMode);
    }
    //return;    
}

function NotifyWeaponFinishedFiring(byte FireMode)
{
    // End:0x25
    if(AIController != none)
    {
        AIController.NotifyWeaponFinishedFiring(self, FireMode);
    }
    //return;    
}

simulated function bool ShouldRefire()
{
    // End:0x17
    if(!HasAmmo(CurrentFireMode))
    {
        return false;
    }
    return StillFiring(CurrentFireMode);
    //return ReturnValue;    
}

simulated function bool StillFiring(byte FireMode)
{
    return PendingFire(int(FireMode));
    //return ReturnValue;    
}

simulated function WeaponIsDown()
{
    //return;    
}

simulated function CacheAIController()
{
    AIController = AIController(Instigator.Controller);
    //return;    
}

auto simulated state Inactive
{
    reliable server function ServerStartFire(byte FireModeNum)
    {
        global.ServerStartFire(FireModeNum);
        WarnInternal((string(WorldInfo.TimeSeconds) @ string(Instigator)) @ "received ServerStartFire in Inactive State!!!");
        // End:0xD2
        if((Instigator != none) && Instigator.Weapon == self)
        {
            WarnInternal(" - I'm the current weapon, so gotostate active and start firing");
            GotoState('Active');            
        }
        else
        {
            // End:0x296
            if((InvManager != none) && InvManager.PendingWeapon == self)
            {
                // End:0x191
                if(Instigator.Weapon.IsInState('WeaponPuttingDown'))
                {
                    WarnInternal(" - I'm the pending weapon, and current weapon is being put down, so force switch now");
                    Instigator.Weapon.WeaponIsDown();                    
                }
                else
                {
                    WarnInternal(" - I'm the pending weapon, but current weapon is NOT being put down, so resync client and server");
                    InvManager.SetCurrentWeapon(self);
                    InvManager.ServerSetCurrentWeapon(self);
                    // End:0x293
                    if(((Instigator.Weapon != self) && InvManager.PendingWeapon == self) && Instigator.Weapon.IsInState('WeaponPuttingDown'))
                    {
                        Instigator.Weapon.WeaponIsDown();
                    }
                }                
            }
            else
            {
                // End:0x37C
                if(Instigator != none)
                {
                    WarnInternal(" - I'm just in the inventory, so resync client and server");
                    InvManager.SetCurrentWeapon(self);
                    InvManager.ServerSetCurrentWeapon(self);
                    // End:0x37C
                    if(((Instigator.Weapon != self) && InvManager.PendingWeapon == self) && Instigator.Weapon.IsInState('WeaponPuttingDown'))
                    {
                        Instigator.Weapon.WeaponIsDown();
                    }
                }
            }
        }
        //return;        
    }

    reliable server function ServerStopFire(byte FireModeNum)
    {
        ClearPendingFire(int(FireModeNum));
        //return;        
    }

    simulated function bool TryPutDown()
    {
        return false;
        //return ReturnValue;        
    }
    stop;    
}

simulated state Active
{
    simulated event BeginState(name PreviousStateName)
    {
        local int I;

        // End:0x1A
        if(Role == ROLE_Authority)
        {
            CacheAIController();
        }
        // End:0x30
        if(bWeaponPutDown)
        {
            PutDownWeapon();            
        }
        else
        {
            // End:0x4C
            if(!HasAnyAmmo())
            {
                WeaponEmpty();                
            }
            else
            {
                I = 0;
                J0x53:

                // End:0x9D [Loop If]
                if(I < InvManager.PendingFire.Length)
                {
                    // End:0x93
                    if(PendingFire(I))
                    {
                        BeginFire(byte(I));
                        // [Explicit Break]
                        goto J0x9D;
                    }
                    I++;
                    // [Loop Continue]
                    goto J0x53;
                }
            }
        }
        J0x9D:

        //return;        
    }

    simulated function BeginFire(byte FireModeNum)
    {
        // End:0x5F
        if(!bDeleteMe && Instigator != none)
        {
            global.BeginFire(FireModeNum);
            // End:0x5F
            if((PendingFire(int(FireModeNum))) && HasAmmo(FireModeNum))
            {
                SendToFiringState(FireModeNum);
            }
        }
        //return;        
    }

    simulated function bool ReadyToFire(bool bFinished)
    {
        return true;
        //return ReturnValue;        
    }

    simulated function bool TryPutDown()
    {
        PutDownWeapon();
        return true;
        //return ReturnValue;        
    }
    stop;    
}

simulated state WeaponFiring
{
    simulated event bool IsFiring()
    {
        return true;
        //return ReturnValue;        
    }

    simulated function RefireCheckTimer()
    {
        // End:0x15
        if(bWeaponPutDown)
        {
            PutDownWeapon();
            return;
        }
        // End:0x2E
        if(ShouldRefire())
        {
            FireAmmunition();
            return;
        }
        HandleFinishedFiring();
        //return;        
    }

    simulated event BeginState(name PreviousStateName)
    {
        LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "PreviousStateName:") @ string(PreviousStateName), 'Inventory');
        FireAmmunition();
        TimeWeaponFiring(CurrentFireMode);
        //return;        
    }

    simulated event EndState(name NextStateName)
    {
        LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "NextStateName:") @ string(NextStateName), 'Inventory');
        ClearFlashCount();
        ClearFlashLocation();
        ClearTimer('RefireCheckTimer');
        NotifyWeaponFinishedFiring(CurrentFireMode);
        //return;        
    }
    stop;    
}

simulated state WeaponEquipping
{
    simulated event BeginState(name PreviousStateName)
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
        TimeWeaponEquipping();
        bWeaponPutDown = false;
        //return;        
    }

    simulated event EndState(name NextStateName)
    {
        ClearTimer('WeaponEquipped');
        //return;        
    }

    simulated function WeaponEquipped()
    {
        // End:0x15
        if(bWeaponPutDown)
        {
            PutDownWeapon();
            return;
        }
        GotoState('Active');
        //return;        
    }
    stop;    
}

simulated state WeaponPuttingDown
{
    simulated event BeginState(name PreviousStateName)
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
        TimeWeaponPutDown();
        bWeaponPutDown = false;
        ForceEndFire();
        //return;        
    }

    simulated function WeaponIsDown()
    {
        // End:0x19
        if(InvManager.CancelWeaponChange())
        {
            return;
        }
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
        DetachWeapon();
        GotoState('Inactive');
        InvManager.ChangedWeapon();
        //return;        
    }

    simulated function bool TryPutDown()
    {
        return false;
        //return ReturnValue;        
    }

    reliable client simulated function ClientWeaponThrown()
    {
        WeaponIsDown();
        global.ClientWeaponThrown();
        //return;        
    }

    simulated event EndState(name NextStateName)
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
        ClearTimer('WeaponIsDown');
        //return;        
    }
    stop;    
}

state PendingClientWeaponSet
{
    simulated function PendingWeaponSetTimer()
    {
        ClientWeaponSet(bWasOptionalSet, bWasDoNotActivate);
        //return;        
    }

    simulated event BeginState(name PreviousStateName)
    {
        bPendingClientWeaponSet = true;
        SetTimer(0.0300000, true, 'PendingWeaponSetTimer');
        //return;        
    }

    simulated event EndState(name NextStateName)
    {
        bPendingClientWeaponSet = false;
        ClearTimer('PendingWeaponSetTimer');
        //return;        
    }
    stop;    
}

defaultproperties
{
    EquipTime=0.3300000
    PutDownTime=0.3300000
    bCanThrow=true
    WeaponRange=16384.0000000
    DefaultAnimSpeed=1.0000000
    Priority=-1.0000000
    AIRating=0.5000000
    ItemName="Weapon"
    RespawnTime=30.0000000
    bReplicateInstigator=true
    bOnlyDirtyReplication=false
    Components=none
}
