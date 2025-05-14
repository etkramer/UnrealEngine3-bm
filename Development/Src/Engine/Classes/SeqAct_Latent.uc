class SeqAct_Latent extends SequenceAction
    abstract
    native(Sequence);

cpptext
{
	virtual void PreActorHandle(AActor *inActor);
	virtual UBOOL UpdateOp(FLOAT deltaTime);
	virtual void Activated();
	virtual void DeActivated();
};

var array<Actor> LatentActors;
var bool bAborted;

// Export USeqAct_Latent::execAbortFor(FFrame&, void* const)
native function AbortFor(Actor latentActor);

event bool Update(float DeltaTime)
{
    //return ReturnValue;    
}

defaultproperties
{
    bLatentExecution=true
    OutputLinks[0]=(Links=none,LinkDesc="Finished",bHasImpulse=false,bDisabled=false,bDisabledPIE=false,LinkedOp=none,ActivateDelay=0.0000000,DrawY=0,bHidden=false)
    OutputLinks[1]=(Links=none,LinkDesc="Aborted",bHasImpulse=false,bDisabled=false,bDisabledPIE=false,LinkedOp=none,ActivateDelay=0.0000000,DrawY=0,bHidden=false)
}
