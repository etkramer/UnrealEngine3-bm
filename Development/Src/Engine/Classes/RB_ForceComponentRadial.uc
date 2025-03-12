// BM1
class RB_ForceComponentRadial extends RB_ForceComponent
    native
    editinlinenew
    collapsecategories;

var() export editinline DrawSphereComponent RenderComponentSphere;
var() interp float ForceStrength;
var() interp float ForceRadius;
var() interp float SwirlStrength;
var() interp float SpinTorque;
var() export PrimitiveComponent.ERadialImpulseFalloff ForceFalloff;

function RB_ForceComponent Clone()
{
    local RB_ForceComponentRadial NewForceComp;

    NewForceComp = new Class'RB_ForceComponentRadial';
    SetBaseCloneParameters(NewForceComp);
    NewForceComp.ForceStrength = ForceStrength;
    NewForceComp.ForceRadius = ForceRadius;
    NewForceComp.SwirlStrength = SwirlStrength;
    NewForceComp.SpinTorque = SpinTorque;
    NewForceComp.ForceFalloff = ForceFalloff;
    NewForceComp.RenderComponentSphere.SetHidden(RenderComponentSphere.HiddenGame);
    NewForceComp.RenderComponentSphere.SetRotation(RenderComponentSphere.Rotation);
    NewForceComp.RenderComponentSphere.SetTranslation(RenderComponentSphere.Translation);
    NewForceComp.RenderComponentSphere.SphereRadius = RenderComponentSphere.SphereRadius;
    return NewForceComp;
    //return ReturnValue;    
}

defaultproperties
{
    // Reference: DrawSphereComponent'Default__RB_ForceComponentRadial.RenderComponent0'
    begin object name=RenderComponent0 class=Class'DrawSphereComponent'
        SphereRadius=200.0000000
    end object
    RenderComponentSphere=RenderComponent0
    ForceStrength=10.0000000
    ForceRadius=200.0000000
    RenderComponent=RenderComponent0
}