// BM1
class ActorFactorySpotLight extends ActorFactory
    native
    config(Editor)
    editinlinenew
    collapsecategories;

var export editinline SpotLightComponent LightComponent;

defaultproperties
{
    MenuName="Add Light (Spot)"
    NewActorClass=Class'SpotLight'
    UseActorSelection=true
}