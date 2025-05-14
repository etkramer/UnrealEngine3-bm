class SeqAct_AttachToActor extends SequenceAction;

var() bool bDetach;
var() bool bHardAttach;
var() bool bUseRelativeOffset;
var() bool bUseRelativeRotation;
var() bool bIgnoreBaseRotation;
var() bool bUseTickGroup;
var() name BoneName;
var() Vector RelativeOffset;
var() Rotator RelativeRotation;
var() Object.ETickingGroup TickGroup;

static event int GetObjClassVersion()
{
    return super(SequenceObject).GetObjClassVersion() + 1;
    //return ReturnValue;    
}

defaultproperties
{
    bHardAttach=true
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Target",LinkVar="None",PropertyName="Targets",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[1]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Attachment",LinkVar="None",PropertyName="None",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
}
