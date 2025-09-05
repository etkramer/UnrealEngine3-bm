class RCinematicCameraActor extends CameraActor;

var transient array<AnimSet> AnimSets;
var() Actor RelativeTo;
var transient name BoneName;

defaultproperties
{
    DrawFrustum=none
    MeshComp=none
    Components[0]=none
    Components[1]=none
}
