class BlockingVolume extends Volume
    native
    placeable;

var() bool bClampFluid;
var() bool bBlockCamera;
var Color DebugRenderingColor;

cpptext
{
	UBOOL IgnoreBlockingBy( const AActor *Other ) const;
}

simulated function OnToggle(SeqAct_Toggle Action)
{
    // End:0x34
    if(Action.InputLinks[0].bHasImpulse)
    {
        CollisionComponent.SetBlockRigidBody(true);        
    }
    else
    {
        // End:0x68
        if(Action.InputLinks[1].bHasImpulse)
        {
            CollisionComponent.SetBlockRigidBody(false);            
        }
        else
        {
            // End:0xAB
            if(Action.InputLinks[2].bHasImpulse)
            {
                CollisionComponent.SetBlockRigidBody(!CollisionComponent.BlockRigidBody);
            }
        }
    }
    super.OnToggle(Action);
    //return;    
}

defaultproperties
{
    bClampFluid=true
    bBlockCamera=true
    // Reference: BrushComponent'Default__BlockingVolume.BrushComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'BrushComponent0'
    // Archetype: BrushComponent'Default__Volume.BrushComponent0'
    begin object name="BrushComponent0"
        BlockActors=true
        BlockRigidBody=true
        RBChannel=RBCC_BlockingVolume
    end object
    BrushComponent=BrushComponent0
    bWorldGeometry=true
    bBlockActors=true
    Components[0]=BrushComponent0
    CollisionComponent=BrushComponent0
}
