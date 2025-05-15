class SequenceEvent extends SequenceOp
    abstract
    native(Sequence);

cpptext
{
	// USequenceObject interface
	virtual void DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime);
	virtual FIntRect GetSeqObjBoundingBox();
	FIntPoint GetCenterPoint(FCanvas* Canvas);

	virtual UBOOL CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest=FALSE, TArray<INT>* ActivateIndices = NULL, UBOOL bPushTop = FALSE);

	/**
	 * Adds an error message to the map check dialog if this SequenceEvent's EventActivator is bStatic
	 */
	virtual void CheckForErrors();

	/**
	 * This is a debug version of ActivateEvent which can be used by automated testing tools to Activate
	 * an event for testing purposes.
	 **/
	virtual void DebugActivateEvent(AActor *InOriginator, AActor *InInstigator, TArray<INT> *ActivateIndices = NULL);

	virtual UBOOL RegisterEvent();

	/**
	 * Called after all the op has been deactivated and all linked variable values have been propagated to the next op
	 * in the sequence.
	 *
	 * This version handles cases where the event is activated multiple times in a single frame.
	 */
	virtual void PostDeActivated();

	/**
	 * Fills in the value of the "Instigator" VariableLink
	 */
	virtual void InitializeLinkedVariableValues();

	virtual void OnExport()
	{
		Super::OnExport();
		Originator = NULL;
		Instigator = NULL;
	}

	/**
	 * Returns whether this SequenceObject can exist in a sequence without being linked to anything else (i.e. does not require
	 * another sequence object to activate it)
	 */
	virtual UBOOL IsStandalone() const { return TRUE; }

	virtual FString GetDisplayTitle() const;

protected:
	virtual void ActivateEvent(AActor *InOriginator, AActor *InInstigator, TArray<INT> *ActivateIndices = NULL, UBOOL bPushTop = FALSE, UBOOL bFromQueued = FALSE);
}

struct native QueuedActivationInfo
{
    var Actor InOriginator;
    var Actor InInstigator;
    var array<int> ActivateIndices;
    var bool bPushTop;

    structdefaultproperties
    {
        InOriginator=none
        InInstigator=none
        bPushTop=false
    }
};

var transient array<SequenceEvent> DuplicateEvts;
var Actor Originator;
var Actor Instigator;
var float ActivationTime;
var int TriggerCount;
var() int MaxTriggerCount;
var() float ReTriggerDelay;
var() bool bEnabled;
var() bool bPlayerOnly;
var transient bool bRegistered;
var() const bool bClientSideOnly;
var() byte Priority;
var int MaxWidth;
var array<QueuedActivationInfo> QueuedActivations;

event RegisterEvent()
{
    //return;    
}

// Export USequenceEvent::execCheckActivate(FFrame&, void* const)
native noexport final function bool CheckActivate(Actor InOriginator, Actor InInstigator, optional bool bTest, const optional out array<int> ActivateIndices, optional bool bPushTop);

function Reset()
{
    ActivationTime = 0.0000000;
    TriggerCount = 0;
    Instigator = none;
    //return;    
}

event Toggled()
{
    //return;    
}

defaultproperties
{
    MaxTriggerCount=1
    bEnabled=true
    bPlayerOnly=true
    bAutoActivateOutputLinks=false
    InputLinks=none
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Instigator",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
}
