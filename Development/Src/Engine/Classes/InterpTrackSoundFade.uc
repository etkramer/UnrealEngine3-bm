// BM1
class InterpTrackSoundFade extends InterpTrackFloatBase
    native(Interpolation)
    collapsecategories;

var() bool ResetVolume;

defaultproperties
{
    ResetVolume=true
    TrackInstClass=Class'InterpTrackInstSoundFade'
    TrackTitle="Sound Fade"
    bOnePerGroup=true
    bDirGroupOnly=true
}
