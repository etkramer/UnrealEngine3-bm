class Vehicle extends Pawn
    abstract
    native
    nativereplication
    config(Game)
    placeable;

defaultproperties
{
    // Reference: CylinderComponent'Default__Vehicle.CollisionCylinder'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'CollisionCylinder'
    // Archetype: CylinderComponent'Default__Pawn.CollisionCylinder'
    begin object name="CollisionCylinder"
    end object
    CylinderComponent=CollisionCylinder
    Components[0]=none
    Components[1]=CollisionCylinder
    Components[2]=none
    CollisionComponent=CollisionCylinder
}
