// BM1
class RB_ForceComponentCylindrical extends RB_ForceComponent
    native
    editinlinenew
    collapsecategories;

var() export editinline DrawCylinderComponent RenderComponentCylinder;
var() interp float RadialStrength;
var() interp float RotationalStrength;
var() interp float LiftStrength;
var() interp float LiftFalloffHeight;
var() interp float EscapeVelocity;
var() interp float ForceRadius;
var() interp float ForceTopRadius;
var() interp float ForceHeight;
var() interp float HeightOffset;

function RB_ForceComponent Clone()
{
    local RB_ForceComponentCylindrical NewForceComp;

    NewForceComp = new Class'RB_ForceComponentCylindrical';
    SetBaseCloneParameters(NewForceComp);
    NewForceComp.RadialStrength = RadialStrength;
    NewForceComp.RotationalStrength = RotationalStrength;
    NewForceComp.LiftStrength = LiftStrength;
    NewForceComp.LiftFalloffHeight = LiftFalloffHeight;
    NewForceComp.EscapeVelocity = EscapeVelocity;
    NewForceComp.ForceRadius = ForceRadius;
    NewForceComp.ForceTopRadius = ForceTopRadius;
    NewForceComp.ForceHeight = ForceHeight;
    NewForceComp.HeightOffset = HeightOffset;
    NewForceComp.RenderComponentCylinder.SetHidden(RenderComponentCylinder.HiddenGame);
    NewForceComp.RenderComponentCylinder.SetRotation(RenderComponentCylinder.Rotation);
    NewForceComp.RenderComponentCylinder.SetTranslation(RenderComponentCylinder.Translation);
    NewForceComp.RenderComponentCylinder.CylinderRadius = RenderComponentCylinder.CylinderRadius;
    NewForceComp.RenderComponentCylinder.CylinderTopRadius = RenderComponentCylinder.CylinderTopRadius;
    NewForceComp.RenderComponentCylinder.CylinderHeight = RenderComponentCylinder.CylinderHeight;
    return NewForceComp;
    //return ReturnValue;    
}

defaultproperties
{
    // Reference: DrawCylinderComponent'Default__RB_ForceComponentCylindrical.RenderComponent0'
    begin object name=RenderComponent0 class=Class'DrawCylinderComponent'
        CylinderRadius=200.0000000
        CylinderTopRadius=200.0000000
        CylinderHeight=200.0000000
    end object
    RenderComponentCylinder=RenderComponent0
    EscapeVelocity=10000.0000000
    ForceRadius=200.0000000
    ForceTopRadius=200.0000000
    ForceHeight=200.0000000
    RenderComponent=RenderComponent0
}