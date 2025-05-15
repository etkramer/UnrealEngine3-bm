class DirectionalLightComponent extends LightComponent
    native
    editinlinenew
    collapsecategories;

var(AdvancedLighting) float TraceDistance;

cpptext
{
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual FVector4 GetPosition() const;
	virtual ELightComponentType GetLightType() const;
}

function OnUpdatePropertyLightColor()
{
    UpdateColorAndBrightness();
    //return;    
}

function OnUpdatePropertyBrightness()
{
    UpdateColorAndBrightness();
    //return;    
}

defaultproperties
{
    TraceDistance=100000.0000000
}
