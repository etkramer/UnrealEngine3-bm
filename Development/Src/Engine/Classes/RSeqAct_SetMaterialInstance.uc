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
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Target",LinkVar="None",PropertyName="Targets",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[1]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="New Material",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=0,MaxVars=255,DrawX=0,CachedProperty=none)
}