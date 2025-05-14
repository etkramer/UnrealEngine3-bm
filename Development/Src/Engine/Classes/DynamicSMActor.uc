class DynamicSMActor extends Actor
    abstract
    native
    notplaceable;

var() const editconst export editinline StaticMeshComponent StaticMeshComponent;
var() const editconst export editinline DynamicLightEnvironmentComponent LightEnvironment;
var repnotify transient StaticMesh ReplicatedMesh;
var repnotify MaterialInterface ReplicatedMaterial;
var repnotify bool bForceStaticDecals;
var() bool bPawnCanBaseOn;
var() bool bSafeBaseIfAsleep;
var repnotify Vector ReplicatedMeshTranslation;
var repnotify Rotator ReplicatedMeshRotation;
var repnotify Vector ReplicatedMeshScale3D;

cpptext
{
	/**
	* Function that gets called from within Map_Check to allow this actor to check itself
	* for any potential errors and register them with map check dialog.
	*/
	virtual void CheckForErrors();

protected:
/**
     * This function actually does the work for the GetDetailInfo and is virtual.
     * It should only be called from GetDetailedInfo as GetDetailedInfo is safe to call on NULL object pointers
     **/
	virtual FString GetDetailedInfoInternal() const;
}

event PostBeginPlay()
{
    super.PostBeginPlay();
    ReplicatedMesh = StaticMeshComponent.StaticMesh;
    bForceStaticDecals = StaticMeshComponent.bForceStaticDecals;
    //return;    
}

simulated event ReplicatedEvent(name VarName)
{
    // End:0x53
    if(VarName == 'ReplicatedMesh')
    {
        LightEnvironment.bCastShadows = false;
        LightEnvironment.SetEnabled(true);
        StaticMeshComponent.SetStaticMesh(ReplicatedMesh);        
    }
    else
    {
        // End:0x83
        if(VarName == 'ReplicatedMaterial')
        {
            StaticMeshComponent.SetMaterial(0, ReplicatedMaterial);            
        }
        else
        {
            // End:0xB2
            if(VarName == 'ReplicatedMeshTranslation')
            {
                StaticMeshComponent.SetTranslation(ReplicatedMeshTranslation);                
            }
            else
            {
                // End:0xE1
                if(VarName == 'ReplicatedMeshRotation')
                {
                    StaticMeshComponent.SetRotation(ReplicatedMeshRotation);                    
                }
                else
                {
                    // End:0x110
                    if(VarName == 'ReplicatedMeshScale3D')
                    {
                        StaticMeshComponent.SetScale3D(ReplicatedMeshScale3D);                        
                    }
                    else
                    {
                        // End:0x13C
                        if(VarName == 'bForceStaticDecals')
                        {
                            StaticMeshComponent.SetForceStaticDecals(bForceStaticDecals);                            
                        }
                        else
                        {
                            super.ReplicatedEvent(VarName);
                        }
                    }
                }
            }
        }
    }
    //return;    
}

function OnSetStaticMesh(SeqAct_SetStaticMesh Action)
{
    local bool bForce;

    bForce = (Action.bIsAllowedToMove == StaticMeshComponent.bForceStaticDecals) || Action.bAllowDecalsToReattach;
    // End:0x15A
    if((Action.NewStaticMesh != none) && (Action.NewStaticMesh != StaticMeshComponent.StaticMesh) || bForce)
    {
        LightEnvironment.bCastShadows = false;
        LightEnvironment.SetEnabled(true);
        bForceStaticDecals = !Action.bIsAllowedToMove;
        StaticMeshComponent.SetForceStaticDecals(bForceStaticDecals);
        StaticMeshComponent.bAllowDecalAutomaticReAttach = Action.bAllowDecalsToReattach;
        StaticMeshComponent.SetStaticMesh(Action.NewStaticMesh, Action.bAllowDecalsToReattach);
        StaticMeshComponent.bAllowDecalAutomaticReAttach = true;
        ReplicatedMesh = Action.NewStaticMesh;
        ForceNetRelevant();
    }
    //return;    
}

function OnSetMaterial(SeqAct_SetMaterial Action)
{
    StaticMeshComponent.SetMaterial(Action.MaterialIndex, Action.NewMaterial);
    // End:0x66
    if(Action.MaterialIndex == 0)
    {
        ReplicatedMaterial = Action.NewMaterial;
        ForceNetRelevant();
    }
    //return;    
}

function OnSetMaterialInstance(RSeqAct_SetMaterialInstance Action)
{
    StaticMeshComponent.SetMaterial(Action.MaterialIndex, Action.NewMaterial);
    //return;    
}

function SetStaticMesh(StaticMesh NewMesh, optional Vector NewTranslation, optional Rotator NewRotation, optional Vector NewScale3D)
{
    StaticMeshComponent.SetStaticMesh(NewMesh);
    StaticMeshComponent.SetTranslation(NewTranslation);
    StaticMeshComponent.SetRotation(NewRotation);
    // End:0x80
    if(!IsZero(NewScale3D))
    {
        StaticMeshComponent.SetScale3D(NewScale3D);
        ReplicatedMeshScale3D = NewScale3D;
    }
    ReplicatedMesh = NewMesh;
    ReplicatedMeshTranslation = NewTranslation;
    ReplicatedMeshRotation = NewRotation;
    ForceNetRelevant();
    //return;    
}

simulated function bool CanBasePawn(Pawn P)
{
    // End:0x3F
    if(bPawnCanBaseOn || (bSafeBaseIfAsleep && StaticMeshComponent != none) && !StaticMeshComponent.RigidBodyIsAwake())
    {
        return true;
    }
    return false;
    //return ReturnValue;    
}

event Attach(Actor Other)
{
    local Pawn P;

    super.Attach(Other);
    // End:0x35
    if(bSafeBaseIfAsleep)
    {
        P = Pawn(Other);
        // End:0x35
        if(P != none)
        {
            SetPhysics(0);
        }
    }
    //return;    
}

event Detach(Actor Other)
{
    local int Idx;
    local Pawn P, Test;
    local bool bResetPhysics;

    super.Detach(Other);
    P = Pawn(Other);
    // End:0x9B
    if(P != none)
    {
        bResetPhysics = true;
        Idx = 0;
        J0x35:

        // End:0x8C [Loop If]
        if(Idx < Attached.Length)
        {
            Test = Pawn(Attached[Idx]);
            // End:0x82
            if((Test != none) && Test != P)
            {
                bResetPhysics = false;
                // [Explicit Break]
                goto J0x8C;
            }
            Idx++;
            // [Loop Continue]
            goto J0x35;
        }
        J0x8C:

        // End:0x9B
        if(bResetPhysics)
        {
            SetPhysics(10);
        }
    }
    //return;    
}

final simulated function SetLightEnvironmentToNotBeDynamic()
{
    // End:0x1D
    if(LightEnvironment != none)
    {
        LightEnvironment.bDynamic = false;
    }
    //return;    
}

defaultproperties
{
    // Reference: StaticMeshComponent'Default__DynamicSMActor.StaticMeshComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'StaticMeshComponent0'
    begin object name="StaticMeshComponent0" class=Class'StaticMeshComponent'
        LightEnvironment=DynamicLightEnvironmentComponent'Default__DynamicSMActor.MyLightEnvironment'
        bCastDynamicShadow=false
        BlockRigidBody=false
    end object
    StaticMeshComponent=StaticMeshComponent0
    // Reference: DynamicLightEnvironmentComponent'Default__DynamicSMActor.MyLightEnvironment'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'MyLightEnvironment'
    begin object name="MyLightEnvironment" class=Class'DynamicLightEnvironmentComponent'
        bEnabled=false
    end object
    LightEnvironment=MyLightEnvironment
    bPawnCanBaseOn=true
    bStasis=true
    bGameRelevant=true
    bEdShouldSnap=true
    bPathColliding=true
    Components[0]=MyLightEnvironment
    Components[1]=StaticMeshComponent0
    RemoteRole=ROLE_SimulatedProxy
    CollisionComponent=StaticMeshComponent0
}
