// BM1
class ApexComponentBase extends MeshComponent
    native
    editinlinenew;

var protected native const transient Pointer ComponentBaseResources;
var protected native const transient RenderCommandFence ReleaseResourcesFence;
var() const ApexAsset Asset;
var() Color WireframeColor;
var const bool bAssetChanged;

defaultproperties
{
    WireframeColor=(R=255,G=128,B=64,A=255)
    CollideActors=true
    BlockActors=true
    BlockZeroExtent=true
    BlockNonZeroExtent=true
    BlockRigidBody=true
    TickGroup=TG_PreAsyncWork
}