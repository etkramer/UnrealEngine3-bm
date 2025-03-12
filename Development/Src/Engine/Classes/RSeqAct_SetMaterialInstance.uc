// BM1
class RSeqAct_SetMaterialInstance extends SequenceAction
    native(Sequence);

var() MaterialInstance MaterialToInstance;
var() int MaterialIndex;
var MaterialInstanceConstant NewMaterial;

event CreateInstance()
{
    NewMaterial = new (none) Class'MaterialInstanceConstant';
    NewMaterial.SetParent(MaterialToInstance);
    //return;    
}

defaultproperties
{
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkDesc="Target",LinkVar="None",PropertyName="Targets",MinVars=1,MaxVars=255)
    VariableLinks[1]=(ExpectedType=Class'SeqVar_Object',LinkDesc="New Material",LinkVar="None",PropertyName="None",bWriteable=true,MinVars=0,MaxVars=255)
}