class InventoryManager extends Actor
    native
    notplaceable;

var Inventory InventoryChain;
var Weapon PendingWeapon;
var Weapon LastAttemptedSwitchToWeapon;
var bool bMustHoldWeapon;
var array<int> PendingFire;

event PostBeginPlay()
{
    super.PostBeginPlay();
    Instigator = Pawn(Owner);
    //return;    
}

// Export UInventoryManager::execInventoryActors(FFrame&, void* const)
native final iterator function InventoryActors(class<Inventory> BaseClass, out Inventory Inv);

simulated exec function DumpWeaponStats()
{
    local Weapon Weap;

    LogInternal(("+++" @ string(GetFuncName())) @ "+++");
    LogInternal((("Pawn.Weapon:" $ string(Instigator.Weapon)) @ "PendingWeapon:") $ string(PendingWeapon));
    // End:0x80
    foreach InventoryActors(Class'Weapon', Weap)
    {
        Weap.DumpWeaponDebugToLog();        
    }    
    //return;    
}

function SetupFor(Pawn P)
{
    Instigator = P;
    SetOwner(P);
    //return;    
}

event Destroyed()
{
    DiscardInventory();
    //return;    
}

function bool HandlePickupQuery(class<Inventory> ItemClass, Actor Pickup)
{
    local Inventory Inv;

    // End:0x0D
    if(InventoryChain == none)
    {
        return true;
    }
    // End:0x45
    foreach InventoryActors(Class'Inventory', Inv)
    {
        // End:0x44
        if(Inv.DenyPickupQuery(ItemClass, Pickup))
        {            
            return false;
        }        
    }    
    return true;
    //return ReturnValue;    
}

simulated event Inventory FindInventoryType(class<Inventory> DesiredClass, optional bool bAllowSubclass)
{
    local Inventory Inv;

    // End:0x40
    foreach InventoryActors(DesiredClass, Inv)
    {
        // End:0x3F
        if(bAllowSubclass || Inv.Class == DesiredClass)
        {            
            return Inv;
        }        
    }    
    return none;
    //return ReturnValue;    
}

simulated function Inventory CreateInventory(class<Inventory> NewInventoryItemClass, optional bool bDoNotActivate)
{
    local Inventory Inv;

    // End:0x101
    if(NewInventoryItemClass != none)
    {
        Inv = Spawn(NewInventoryItemClass, Owner,,,,, true);
        // End:0xB8
        if(Inv != none)
        {
            // End:0xB5
            if(!AddInventory(Inv, bDoNotActivate))
            {
                WarnInternal("InventoryManager::CreateInventory - Couldn't Add newly created inventory" @ string(Inv));
                Inv.Destroy();
                Inv = none;
            }            
        }
        else
        {
            WarnInternal("InventoryManager::CreateInventory - Couldn't spawn inventory" @ string(NewInventoryItemClass));
        }
    }
    return Inv;
    //return ReturnValue;    
}

simulated function bool AddInventory(Inventory NewItem, optional bool bDoNotActivate)
{
    local Inventory Item, LastItem;

    // End:0x19E
    if((NewItem != none) && !NewItem.bDeleteMe)
    {
        // End:0x3C
        if(InventoryChain == none)
        {
            InventoryChain = NewItem;            
        }
        else
        {
            Item = InventoryChain;
            J0x47:

            // End:0x86 [Loop If]
            if(Item != none)
            {
                // End:0x63
                if(Item == NewItem)
                {
                    return false;
                }
                LastItem = Item;
                Item = Item.Inventory;
                // [Loop Continue]
                goto J0x47;
            }
            LastItem.Inventory = NewItem;
        }
        LogInternal(((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "adding") @ string(NewItem)) @ "bDoNotActivate:") @ string(bDoNotActivate), 'Inventory');
        NewItem.SetOwner(Instigator);
        NewItem.Instigator = Instigator;
        NewItem.InvManager = self;
        NewItem.GivenTo(Instigator, bDoNotActivate);
        Instigator.TriggerEventClass(Class'SeqEvent_GetInventory', NewItem);
        return true;
    }
    return false;
    //return ReturnValue;    
}

simulated function RemoveFromInventory(Inventory ItemToRemove)
{
    local Inventory Item;
    local bool bFound;

    // End:0x17E
    if(ItemToRemove != none)
    {
        // End:0x3A
        if(InventoryChain == ItemToRemove)
        {
            bFound = true;
            InventoryChain = ItemToRemove.Inventory;            
        }
        else
        {
            Item = InventoryChain;
            J0x45:

            // End:0xAB [Loop If]
            if(Item != none)
            {
                // End:0x93
                if(Item.Inventory == ItemToRemove)
                {
                    bFound = true;
                    Item.Inventory = ItemToRemove.Inventory;
                    // [Explicit Break]
                    goto J0xAB;
                }
                Item = Item.Inventory;
                // [Loop Continue]
                goto J0x45;
            }
        }
        J0xAB:

        // End:0x154
        if(bFound)
        {
            LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "removed") @ string(ItemToRemove), 'Inventory');
            ItemToRemove.ItemRemovedFromInvManager();
            ItemToRemove.SetOwner(none);
            ItemToRemove.Inventory = none;
        }
        // End:0x17E
        if(ItemToRemove == Instigator.Weapon)
        {
            Instigator.Weapon = none;
        }
    }
    //return;    
}

simulated event DiscardInventory()
{
    local Inventory Inv;
    local Vector TossVelocity;
    local bool bBelowKillZ;

    LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "", 'Inventory');
    bBelowKillZ = (Instigator == none) || Instigator.Location.Z < WorldInfo.KillZ;
    // End:0x168
    foreach InventoryActors(Class'Inventory', Inv)
    {
        // End:0x15A
        if(Inv.bDropOnDeath && !bBelowKillZ)
        {
            TossVelocity = Vector(Instigator.GetViewRotation());
            TossVelocity = ((TossVelocity * ((Instigator.Velocity Dot TossVelocity) + 500.0000000)) + (250.0000000 * VRand())) + vect(0.0000000, 0.0000000, 250.0000000);
            Inv.DropFrom(Instigator.Location, TossVelocity);
            // End:0x167
            continue;
        }
        Inv.Destroy();        
    }    
    Instigator.Weapon = none;
    PendingWeapon = none;
    //return;    
}

function int ModifyDamage(int Damage, Controller InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType)
{
    return Damage;
    //return ReturnValue;    
}

simulated function OwnerEvent(name EventName)
{
    local Inventory Inv;

    // End:0x40
    foreach InventoryActors(Class'Inventory', Inv)
    {
        // End:0x3F
        if(Inv.bReceiveOwnerEvents)
        {
            Inv.OwnerEvent(EventName);
        }        
    }    
    //return;    
}

simulated function DrawHUD(HUD H)
{
    local Inventory Inv;

    // End:0x40
    foreach InventoryActors(Class'Inventory', Inv)
    {
        // End:0x3F
        if(Inv.bRenderOverlays)
        {
            Inv.RenderOverlays(H);
        }        
    }    
    // End:0x79
    if(Instigator.Weapon != none)
    {
        Instigator.Weapon.ActiveRenderOverlays(H);
    }
    //return;    
}

simulated function StartFire(byte FireModeNum)
{
    // End:0x38
    if(Instigator.Weapon != none)
    {
        Instigator.Weapon.StartFire(FireModeNum);
    }
    //return;    
}

simulated function StopFire(byte FireModeNum)
{
    // End:0x38
    if(Instigator.Weapon != none)
    {
        Instigator.Weapon.StopFire(FireModeNum);
    }
    //return;    
}

simulated function bool IsActiveWeapon(Weapon ThisWeapon)
{
    return ThisWeapon == Instigator.Weapon;
    //return ReturnValue;    
}

simulated function float GetWeaponRatingFor(Weapon W)
{
    local float Rating;

    // End:0x1F
    if(!W.HasAnyAmmo())
    {
        return -1.0000000;
    }
    // End:0xA2
    if(!Instigator.IsHumanControlled())
    {
        Rating = W.AIRating;
        // End:0x9F
        if(((IsActiveWeapon(W)) && Instigator.Controller != none) && Instigator.Controller.Enemy != none)
        {
            Rating += 0.2100000;
        }        
    }
    else
    {
        Rating = 1.0000000;
    }
    return Rating;
    //return ReturnValue;    
}

simulated function Weapon GetBestWeapon(optional bool bForceADifferentWeapon)
{
    local Weapon W, BestWeapon;
    local float Rating, BestRating;

    // End:0x99
    foreach InventoryActors(Class'Weapon', W)
    {
        // End:0x98
        if(W.HasAnyAmmo())
        {
            // End:0x4C
            if(bForceADifferentWeapon && IsActiveWeapon(W))
            {
                continue;                
            }
            Rating = W.GetWeaponRating();
            // End:0x98
            if((BestWeapon == none) || Rating > BestRating)
            {
                BestWeapon = W;
                BestRating = Rating;
            }
        }        
    }    
    return BestWeapon;
    //return ReturnValue;    
}

simulated function SwitchToBestWeapon(optional bool bForceADifferentWeapon)
{
    local Weapon BestWeapon;

    LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "bForceADifferentWeapon:") @ string(bForceADifferentWeapon), 'Inventory');
    // End:0x119
    if((bForceADifferentWeapon || PendingWeapon == none) || AIController(Instigator.Controller) != none)
    {
        BestWeapon = GetBestWeapon(bForceADifferentWeapon);
        // End:0xD4
        if(BestWeapon == none)
        {
            return;
        }
        // End:0x119
        if(BestWeapon == Instigator.Weapon)
        {
            BestWeapon = none;
            PendingWeapon = none;
            Instigator.Weapon.Activate();
        }
    }
    Instigator.Controller.StopFiring();
    SetCurrentWeapon(BestWeapon);
    //return;    
}

simulated function PrevWeapon()
{
    local Weapon CandidateWeapon, StartWeapon, W;

    StartWeapon = Instigator.Weapon;
    // End:0x2B
    if(PendingWeapon != none)
    {
        StartWeapon = PendingWeapon;
    }
    // End:0x5C
    foreach InventoryActors(Class'Weapon', W)
    {
        // End:0x50
        if(W == StartWeapon)
        {
            // End:0x5C
            break;
        }
        CandidateWeapon = W;        
    }    
    // End:0x88
    if(CandidateWeapon == none)
    {
        // End:0x87
        foreach InventoryActors(Class'Weapon', W)
        {
            CandidateWeapon = W;            
        }        
    }
    // End:0xA3
    if(CandidateWeapon == Instigator.Weapon)
    {
        return;
    }
    SetCurrentWeapon(CandidateWeapon);
    //return;    
}

simulated function NextWeapon()
{
    local Weapon StartWeapon, CandidateWeapon, W;
    local bool bBreakNext;

    StartWeapon = Instigator.Weapon;
    // End:0x2B
    if(PendingWeapon != none)
    {
        StartWeapon = PendingWeapon;
    }
    // End:0x7A
    foreach InventoryActors(Class'Weapon', W)
    {
        // End:0x62
        if(bBreakNext || StartWeapon == none)
        {
            CandidateWeapon = W;
            // End:0x7A
            break;
        }
        // End:0x79
        if(W == StartWeapon)
        {
            bBreakNext = true;
        }        
    }    
    // End:0xA9
    if(CandidateWeapon == none)
    {
        // End:0xA8
        foreach InventoryActors(Class'Weapon', W)
        {
            CandidateWeapon = W;
            // End:0xA8
            break;            
        }        
    }
    // End:0xC4
    if(CandidateWeapon == Instigator.Weapon)
    {
        return;
    }
    SetCurrentWeapon(CandidateWeapon);
    //return;    
}

reliable client simulated function SetCurrentWeapon(Weapon DesiredWeapon, optional name MovementStance, optional name WeaponStance)
{
    InternalSetCurrentWeapon(DesiredWeapon, MovementStance, WeaponStance);
    // End:0x38
    if(Role < ROLE_Authority)
    {
        ServerSetCurrentWeapon(DesiredWeapon);
    }
    //return;    
}

reliable server function ServerSetCurrentWeapon(Weapon DesiredWeapon, optional name MovementStance, optional name WeaponStance)
{
    InternalSetCurrentWeapon(DesiredWeapon, MovementStance, WeaponStance);
    //return;    
}

private final simulated function InternalSetCurrentWeapon(Weapon DesiredWeapon, optional name MovementStance, optional name WeaponStance)
{
    local Weapon PrevWeapon;

    PrevWeapon = Instigator.Weapon;
    LogInternal(((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "PrevWeapon:") @ string(PrevWeapon)) @ "DesiredWeapon:") @ string(DesiredWeapon), 'Inventory');
    SetPendingWeapon(DesiredWeapon, MovementStance, WeaponStance);
    // End:0x1D5
    if(((PrevWeapon != none) && DesiredWeapon == PrevWeapon) && !PrevWeapon.IsInState('WeaponPuttingDown'))
    {
        // End:0x1D5
        if(!DesiredWeapon.IsInState('Inactive') && !DesiredWeapon.IsInState('PendingClientWeaponSet'))
        {
            LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "DesiredWeapon == PrevWeapon - abort") @ string(DesiredWeapon.GetStateName()), 'Inventory');
            PrevWeapon.bWeaponPutDown = false;
            return;
        }
    }
    // End:0x300
    if((((PrevWeapon != none) && PrevWeapon != DesiredWeapon) && !PrevWeapon.bDeleteMe) && !PrevWeapon.IsInState('Inactive'))
    {
        LogInternal((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "Try to put down previous weapon first.", 'Inventory');
        // End:0x2E9
        if((DesiredWeapon != none) && PrevWeapon.Class == DesiredWeapon.Class)
        {
            PrevWeapon.Destroy();            
        }
        else
        {
            PrevWeapon.TryPutDown();
        }        
    }
    else
    {
        ChangedWeapon();
    }
    //return;    
}

simulated function SetPendingWeapon(Weapon DesiredWeapon, name MovementStance, name WeaponStance)
{
    LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "SetPendingWeapon to") @ string(DesiredWeapon), 'Inventory');
    PendingWeapon = DesiredWeapon;
    //return;    
}

simulated function bool CancelWeaponChange()
{
    // End:0x0D
    if(!bMustHoldWeapon)
    {
        return false;
    }
    // End:0x2D
    if(PendingWeapon == none)
    {
        PendingWeapon = Instigator.Weapon;
    }
    return false;
    //return ReturnValue;    
}

simulated function ChangedWeapon()
{
    local Weapon OldWeapon;

    OldWeapon = Instigator.Weapon;
    // End:0x55
    if((PendingWeapon == none) && bMustHoldWeapon)
    {
        // End:0x55
        if(OldWeapon != none)
        {
            OldWeapon.Activate();
            PendingWeapon = OldWeapon;
        }
    }
    LogInternal(((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "switch from") @ string(OldWeapon)) @ "to") @ string(PendingWeapon), 'Inventory');
    Instigator.Weapon = PendingWeapon;
    OwnerEvent('ChangedWeapon');
    Instigator.PlayWeaponSwitch(OldWeapon, PendingWeapon);
    // End:0x186
    if(PendingWeapon != none)
    {
        PendingWeapon.Instigator = Instigator;
        // End:0x16B
        if(WorldInfo.Game != none)
        {
            Instigator.MakeNoise(0.1000000, 'ChangedWeapon');
        }
        PendingWeapon.Activate();
        PendingWeapon = none;
    }
    // End:0x1CD
    if(Instigator.Controller != none)
    {
        Instigator.Controller.NotifyChangedWeapon(OldWeapon, Instigator.Weapon);
    }
    //return;    
}

simulated function ClientWeaponSet(Weapon NewWeapon, bool bOptionalSet, optional bool bDoNotActivate)
{
    local Weapon OldWeapon;

    LogInternal(((((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "NewWeapon:") @ string(NewWeapon)) @ "bOptionalSet:") @ string(bOptionalSet)) @ "bDoNotActivate:") @ string(bDoNotActivate), 'Inventory');
    // End:0x573
    if(!bDoNotActivate)
    {
        OldWeapon = Instigator.Weapon;
        // End:0x1B7
        if(((OldWeapon == none) || OldWeapon.bDeleteMe) || OldWeapon.IsInState('Inactive'))
        {
            LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "OldWeapon == None or Inactive - Set new weapon right away") @ string(NewWeapon), 'Inventory');
            SetCurrentWeapon(NewWeapon);
            return;
        }
        // End:0x321
        if(OldWeapon == NewWeapon)
        {
            // End:0x29B
            if(NewWeapon.IsInState('PendingClientWeaponSet'))
            {
                LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "OldWeapon == NewWeapon - but in PendingClientWeaponSet, so reset.") @ string(NewWeapon), 'Inventory');
                SetCurrentWeapon(NewWeapon);                
            }
            else
            {
                LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "OldWeapon == NewWeapon - abort") @ string(NewWeapon), 'Inventory');
            }
            return;
        }
        // End:0x434
        if(bOptionalSet)
        {
            // End:0x434
            if(OldWeapon.DenyClientWeaponSet() || Instigator.IsHumanControlled() && PlayerController(Instigator.Controller).bNeverSwitchOnPickup)
            {
                LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "bOptionalSet && (DenyClientWeaponSet() || bNeverSwitchOnPickup) - abort") @ string(NewWeapon), 'Inventory');
                LastAttemptedSwitchToWeapon = NewWeapon;
                return;
            }
        }
        // End:0x573
        if(((PendingWeapon == none) || !PendingWeapon.HasAnyAmmo()) || PendingWeapon.GetWeaponRating() < NewWeapon.GetWeaponRating())
        {
            // End:0x573
            if(!Instigator.Weapon.HasAnyAmmo() || Instigator.Weapon.GetWeaponRating() < NewWeapon.GetWeaponRating())
            {
                LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "Switch to new weapon:") @ string(NewWeapon), 'Inventory');
                SetCurrentWeapon(NewWeapon);
                return;
            }
        }
    }
    LogInternal(((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "Send to inactive state") @ string(NewWeapon), 'Inventory');
    NewWeapon.GotoState('Inactive');
    //return;    
}

reliable client simulated function ClientSyncWeapon(Weapon NewWeapon)
{
    local Weapon OldWeapon;

    // End:0x7D
    if(NewWeapon == Instigator.Weapon)
    {
        LogInternal(((((string(self) @ "(Owned by") @ string(Owner)) @ ") is trying to Sync to the currently active weapon (") $ string(NewWeapon)) $ ")");
        return;
    }
    LogInternal(((((((((((string(WorldInfo.TimeSeconds) @ "Self:") @ string(self)) @ "Instigator:") @ string(Instigator)) @ string(GetStateName())) $ "::") $ string(GetFuncName())) @ "Forcing weapon:") @ string(NewWeapon)) @ "from:") @ string(Instigator.Weapon), 'Inventory');
    OldWeapon = Instigator.Weapon;
    Instigator.Weapon = NewWeapon;
    OwnerEvent('ChangedWeapon');
    Instigator.PlayWeaponSwitch(OldWeapon, NewWeapon);
    // End:0x1E1
    if(NewWeapon != none)
    {
        Instigator.Weapon.Instigator = Instigator;
        // End:0x1C3
        if(WorldInfo.Game != none)
        {
            Instigator.MakeNoise(0.1000000, 'ChangedWeapon');
        }
        Instigator.Weapon.Activate();
    }
    // End:0x228
    if(Instigator.Controller != none)
    {
        Instigator.Controller.NotifyChangedWeapon(OldWeapon, Instigator.Weapon);
    }
    //return;    
}

simulated function UpdateController()
{
    local Inventory Item;
    local Weapon Weap;

    Item = InventoryChain;
    J0x0B:

    // End:0x5D [Loop If]
    if(Item != none)
    {
        Weap = Weapon(Item);
        // End:0x45
        if(Weap != none)
        {
            Weap.CacheAIController();
        }
        Item = Item.Inventory;
        // [Loop Continue]
        goto J0x0B;
    }
    //return;    
}

defaultproperties
{
    bHidden=true
    bOnlyRelevantToOwner=true
    bReplicateInstigator=true
    bReplicateMovement=false
    bOnlyDirtyReplication=true
    RemoteRole=ROLE_SimulatedProxy
    TickGroup=TG_DuringAsyncWork
}
