// BM1
class AirNode extends PathNode
    native
    placeable;

defaultproperties
{
    bFlyingPreferred=true
    bBlockedForVehicles=false
    // Reference: CylinderComponent'Default__AirNode.CollisionCylinder'
    begin object name=CollisionCylinder
    end object
    CylinderComponent=CollisionCylinder
    GoodSprite=none
    BadSprite=none
    Components[0]=none
    Components[1]=none
    Components[2]=none
    Components[3]=CollisionCylinder
    Components[4]=none
    CollisionComponent=CollisionCylinder
}