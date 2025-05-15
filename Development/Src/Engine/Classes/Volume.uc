class Volume extends Brush
    native
    nativereplication
    notplaceable;

var() bool bForcePawnWalk;
var() bool bProcessAllActors;

cpptext
{
	INT Encompasses(FVector point);
	void SetVolumes();
	virtual void SetVolumes(const TArray<class AVolume*>& Volumes);
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	virtual UBOOL IsAVolume() const {return TRUE;}
	virtual AVolume* GetAVolume() { return this; }
	virtual INT* GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
}

// Export UVolume::execEncompasses(FFrame&, void* const)
native noexport function bool Encompasses(Actor Other);

// Export UVolume::execEncompassesPoint(FFrame&, void* const)
native function bool EncompassesPoint(Vector Point);

simulated function OnToggle(SeqAct_Toggle Action)
{
    // End:0x39
    if(Action.InputLinks[0].bHasImpulse)
    {
        // End:0x36
        if(!bCollideActors)
        {
            SetCollision(true, bBlockActors);
        }        
    }
    else
    {
        // End:0x70
        if(Action.InputLinks[1].bHasImpulse)
        {
            // End:0x6D
            if(bCollideActors)
            {
                SetCollision(false, bBlockActors);
            }            
        }
        else
        {
            // End:0xA3
            if(Action.InputLinks[2].bHasImpulse)
            {
                SetCollision(!bCollideActors, bBlockActors);
            }
        }
    }
    ForceNetRelevant();
    SetForcedInitialReplicatedProperty(BoolProperty'bCollideActors', bCollideActors == default.bCollideActors);
    //return;    
}

simulated event CollisionChanged()
{
    CollisionComponent.SetBlockRigidBody(bCollideActors && bBlockActors);
    //return;    
}

event ProcessActorSetVolume(Actor Other)
{
    //return;    
}

cpptext
{
	virtual void PostEditImport();
}

defaultproperties
{
    // Reference: BrushComponent'Default__Volume.BrushComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'BrushComponent0'
    // Archetype: BrushComponent'Default__Brush.BrushComponent0'
    begin object name="BrushComponent0"
        LightingChannels=(bInitialized=true,Dynamic=true)
        bAcceptsLights=true
        CollideActors=true
        BlockNonZeroExtent=true
        bDisableAllRigidBody=true
        AlwaysLoadOnClient=true
        AlwaysLoadOnServer=true
    end object
    BrushComponent=BrushComponent0
    bSkipActorPropertyReplication=true
    bCollideActors=true
    Components[0]=BrushComponent0
    CollisionComponent=BrushComponent0
}
