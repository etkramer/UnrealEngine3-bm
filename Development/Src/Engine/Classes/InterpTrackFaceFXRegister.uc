// BM1
class InterpTrackFaceFXRegister extends InterpTrackFloatBase
    native(Interpolation)
    collapsecategories;

var() string Register;
var() SkeletalMeshComponent.EFaceFXRegOp Operation;

defaultproperties
{
    TrackInstClass=Class'InterpTrackInstFaceFXRegister'
    TrackTitle="FaceFX Register"
}
