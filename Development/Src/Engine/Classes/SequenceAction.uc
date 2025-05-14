class SequenceAction extends SequenceOp
    abstract
    native(Sequence);

cpptext
{
	virtual void Activated();

	/** Called before the handler function is called on a target actor. */
	virtual void PreActorHandle(AActor *inActor) {}
}

var name HandlerName;
var bool bCallHandler;
var() array<Object> Targets;

defaultproperties
{
    bCallHandler=true
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Target",LinkVar="None",PropertyName="Targets",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
}
