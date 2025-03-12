// BM1
class AnimationCompressionAlgorithm_AutoRemoveLinearKeys extends AnimationCompressionAlgorithm
    native(Anim);

var float MaxPosDiff;
var float MaxAngleDiff;
var float MaxEffectorDiff;
var float MinEffectorDiff;
var float ParentKeyScale;
var bool bRetarget;
var() float StartSpeedValue;
var() float EndSpeedValue;
var() float StartSpeedErrorMult;
var() float EndSpeedErrorMult;
var() float SpeedDeemsVerySlowAnim;
var() float VerySlowAnimErrorMult;
var() float FinalErrorMultiplier;
var() float OneBoneAnimsMultipler;
var string SpecialBonesNameList;
var() float SpecialBonesMultipler;
var() array<string> SpecialBonesNameArray;

defaultproperties
{
    MaxPosDiff=0.1500000
    MaxAngleDiff=0.0300000
    MaxEffectorDiff=0.2500000
    MinEffectorDiff=0.2500000
    ParentKeyScale=2.0000000
    bRetarget=true
    StartSpeedValue=10.0000000
    EndSpeedValue=150.0000000
    StartSpeedErrorMult=1.0000000
    EndSpeedErrorMult=8.0000000
    SpeedDeemsVerySlowAnim=50.0000000
    VerySlowAnimErrorMult=0.1275000
    FinalErrorMultiplier=1.0000000
    OneBoneAnimsMultipler=0.0010000
    SpecialBonesMultipler=0.2500000
    SpecialBonesNameArray[0]="Bip01_Head"
    Description="Auto Remove Linear Keys"
    bNeedsSkeleton=true
    RotationCompressionFormat=ACF_Fixed48Max
}
