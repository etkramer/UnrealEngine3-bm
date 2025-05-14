class SeqAct_ToggleHidden extends SeqAct_Toggle;

var() bool bToggleCollision;
var() bool bToggleBasedActors;
var() array< class<Actor> > IgnoreBasedClasses;

event bool IsValidUISequenceObject(optional UIScreenObject TargetObject)
{
    return false;
    //return ReturnValue;    
}

defaultproperties
{
    InputLinks[0]=(LinkDesc="Hide",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
    InputLinks[1]=(LinkDesc="UnHide",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
    InputLinks[2]=(LinkDesc="Toggle",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
}
