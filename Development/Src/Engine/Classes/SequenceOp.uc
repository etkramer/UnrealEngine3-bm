class SequenceOp extends SequenceObject
    abstract
    native(Sequence);

cpptext
{
	virtual void CheckForErrors();

	// USequenceOp interface
	virtual UBOOL UpdateOp(FLOAT deltaTime);
	virtual void Activated();
	virtual void DeActivated();
	/**
	 * Called after all the op has been deactivated and all linked variable values have been propagated to the next op
	 * in the sequence.
	 */
    virtual void PostDeActivated() {};

	/**
	 * Notification that an input link on this sequence op has been given impulse by another op.  Propagates the value of
	 * PlayerIndex from the ActivatorOp to this one.
	 *
	 * @param	ActivatorOp		the sequence op that applied impulse to this op's input link
	 * @param	InputLinkIndex	the index [into this op's InputLinks array] for the input link that was given impulse
	 */
	virtual void OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex );

	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 */
	virtual void InitializeLinkedVariableValues() {}

	// helper functions
	void GetBoolVars(TArray<UBOOL*> &outBools, const TCHAR *inDesc = NULL) const;
	void GetIntVars(TArray<INT*> &outInts, const TCHAR *inDesc = NULL) const;
	void GetByteVars(TArray<BYTE*>& out_Bytes, const TCHAR* inDesc=NULL ) const;
	void GetFloatVars(TArray<FLOAT*> &outFloats, const TCHAR *inDesc = NULL) const;
	void GetVectorVars(TArray<FVector*> &outVectors, const TCHAR *inDesc = NULL) const;
	void GetObjectVars(TArray<UObject**> &outObjects, const TCHAR *inDesc = NULL) const;
	void GetStringVars(TArray<FString*> &outStrings, const TCHAR *inDesc = NULL) const;
	/**
	 * Retrieve a list of FName values connected to this sequence op.
	 *
	 * @param	out_Names	receieves the list of name values
	 * @param	inDesc		if specified, only name values connected via a variable link that this name will be returned.
	 */
	void GetNameVars( TArray<FName*>& out_Names, const TCHAR* inDesc=NULL );
	/** Retrieve list of UInterpData objects connected to this sequence op. */
	void GetInterpDataVars(TArray<class UInterpData*> &outIData, const TCHAR *inDesc = NULL);

	/**
	 * Retrieve a list of UIRangeData values connected to this sequence op.
	 *
	 * @param	out_UIRanges	receieves the list of UIRangeData values
	 * @param	inDesc			if specified, only UIRangeData values connected via a variable link that this name will be returned.
	 */
	void GetUIRangeVars( TArray<struct FUIRangeData*>& out_UIRanges, const TCHAR* inDesc=NULL );

	/**
	 * Retrieve a list of UniqueNetId values connected to this sequence op.
	 *
	 * @param	out_NetIds	receieves the list of UniqueNetId values
	 * @param	inDesc		if specified, only UniqueNetId values connected via a variable link that this name will be returned.
	 */
	void GetUniqueNetIdVars( TArray<struct FUniqueNetId*>& out_NetIds, const TCHAR* inDesc=NULL );

	INT FindConnectorIndex(const FString& ConnName, INT ConnType);
	void CleanupConnections();

	/** Called via PostEditChange(), lets ops create/remove dynamic links based on data. */
	virtual void UpdateDynamicLinks() {}
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	// USequenceObject interface
	virtual void DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime);
	virtual FIntPoint	GetConnectionLocation(INT ConnType, INT ConnIndex);
	virtual FColor		GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex );

	FIntPoint GetLogicConnectorsSize(FCanvas* Canvas, INT* InputY=0, INT* OutputY=0);
	FIntPoint GetVariableConnectorsSize(FCanvas* Canvas);
	FColor GetVarConnectorColor(INT LinkIndex);

	virtual void DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter){}

	void DrawLogicConnectors(FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, INT MouseOverConnType, INT MouseOverConnIndex);
	void DrawVariableConnectors(FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, INT MouseOverConnType, INT MouseOverConnIndex, INT VarWidth);

	virtual void DrawLogicLinks(FCanvas* Canvas, UBOOL bCurves, TArray<USequenceObject*> &SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex);
	virtual void DrawVariableLinks(FCanvas* Canvas, UBOOL bCurves, TArray<USequenceObject*> &SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex);

	void MakeLinkedObjDrawInfo(struct FLinkedObjDrawInfo& ObjInfo, INT MouseOverConnType = -1, INT MouseOverConnIndex = INDEX_NONE);
	INT VisibleIndexToActualIndex(INT ConnType, INT VisibleIndex);

	/**
	 * Handles updating this sequence op when the ObjClassVersion doesn't match the ObjInstanceVersion, indicating that the op's
	 * default values have been changed.
	 */
	virtual void UpdateObject();

	/** Called after the object is loaded */
	virtual void PostLoad();
private:
	static INT CurrentSearchTag;
	void GetLinkedObjectsInternal(TArray<USequenceObject*>& out_Objects, UClass* ObjectType, UBOOL bRecurse);
};

struct native SeqOpInputLink
{
    var string LinkDesc;
    var bool bHasImpulse;
    var int QueuedActivations;
    var bool bDisabled;
    var bool bDisabledPIE;
    var SequenceOp LinkedOp;
    var int DrawY;
    var bool bHidden;
    var float ActivateDelay;

structcpptext
{
     /** Constructors */
    FSeqOpInputLink() {}
    FSeqOpInputLink(EEventParm)
    {
		appMemzero(this, sizeof(FSeqOpInputLink));
    }

	/**
	 * Activates this output link if bDisabled is not true
	 */
	UBOOL ActivateInputLink()
	{
		if ( !bDisabled && !(bDisabledPIE && GIsEditor))
		{
			// if already active then mark in the queue, unless it's a latent op since those are handled uniquely currently
			if (bHasImpulse)
			{
				QueuedActivations++;
			}
			bHasImpulse = TRUE;
			return TRUE;
		}

		return FALSE;
	}
}

    structdefaultproperties
    {
        LinkDesc=""
        bHasImpulse=false
        QueuedActivations=0
        bDisabled=false
        bDisabledPIE=false
        LinkedOp=none
        DrawY=0
        bHidden=false
        ActivateDelay=0.0000000
    }
};

struct native SeqOpOutputInputLink
{
    var SequenceOp LinkedOp;
    var int InputLinkIdx;

	structcpptext
	{
		/** Default ctor */
		FSeqOpOutputInputLink() {}
		FSeqOpOutputInputLink(EEventParm) : LinkedOp(NULL), InputLinkIdx(0)
		{
		}
		FSeqOpOutputInputLink( USequenceOp* InOp, INT InLinkIdx=0 ) : LinkedOp(InOp), InputLinkIdx(InLinkIdx)
		{
		}

		/** Operators */
		/** native serialization operator */
		friend FArchive& operator<<( FArchive& Ar, FSeqOpOutputInputLink& OutputInputLink );

		/** Comparison operator */
		UBOOL operator==( const FSeqOpOutputInputLink& Other ) const;
		UBOOL operator!=( const FSeqOpOutputInputLink& Other ) const;
	}

    structdefaultproperties
    {
        LinkedOp=none
        InputLinkIdx=0
    }
};

struct native SeqOpOutputLink
{
    var array<SeqOpOutputInputLink> Links;
    var string LinkDesc;
    var bool bHasImpulse;
    var bool bDisabled;
    var bool bDisabledPIE;
    var SequenceOp LinkedOp;
    var float ActivateDelay;
    var int DrawY;
    var bool bHidden;

structcpptext
{
     /** Constructors */
    FSeqOpOutputLink() {}
    FSeqOpOutputLink(EEventParm)
    {
		appMemzero(this, sizeof(FSeqOpOutputLink));
    }

	/**
	 * Activates this output link if bDisabled is not true
	 */
	UBOOL ActivateOutputLink()
	{
		if ( !bDisabled && !(bDisabledPIE && GIsEditor))
		{
			bHasImpulse = TRUE;
			return TRUE;
		}
		return FALSE;
	}

	UBOOL HasLinkTo(USequenceOp *Op, INT LinkIdx = -1)
	{
		if (Op != NULL)
		{
			for (INT Idx = 0; Idx < Links.Num(); Idx++)
			{
				if (Links(Idx).LinkedOp == Op &&
					(LinkIdx == -1 || Links(Idx).InputLinkIdx == LinkIdx))
				{
					return TRUE;
				}
			}
		}
		return FALSE;
	}
}

    structdefaultproperties
    {
        Links=none
        LinkDesc=""
        bHasImpulse=false
        bDisabled=false
        bDisabledPIE=false
        LinkedOp=none
        ActivateDelay=0.0000000
        DrawY=0
        bHidden=false
    }
};

struct native SeqVarLink
{
    var class<SequenceVariable> ExpectedType;
    var array<SequenceVariable> LinkedVariables;
    var string LinkDesc;
    var name LinkVar;
    var name PropertyName;
    var bool bWriteable;
    var bool bModifiesLinkedObject;
    var bool bHidden;
    var int MinVars;
    var int MaxVars;
    var int DrawX;
    var const transient Property CachedProperty;

structcpptext
{
    /** Constructors */
    FSeqVarLink() {}
    FSeqVarLink(EEventParm)
    {
	appMemzero(this, sizeof(FSeqVarLink));
    }

	/**
	 * Determines whether this variable link can be associated with the specified sequence variable class.
	 *
	 * @param	SequenceVariableClass	the class to check for compatibility with this variable link; must be a child of SequenceVariable
	 * @param	bRequireExactClass		if FALSE, child classes of the specified class return a match as well.
	 *
	 * @return	TRUE if this variable link can be linked to the a SequenceVariable of the specified type.
	 */
	UBOOL SupportsVariableType( UClass* SequenceVariableClass, UBOOL bRequireExactClass=TRUE ) const;
}

    structdefaultproperties
    {
        ExpectedType=Class'SequenceVariable'
        LinkedVariables=none
        LinkDesc=""
        LinkVar="None"
        PropertyName="None"
        bWriteable=false
        bModifiesLinkedObject=false
        bHidden=false
        MinVars=1
        MaxVars=255
        DrawX=0
        CachedProperty=none
    }
};

struct native SeqEventLink
{
    var class<SequenceEvent> ExpectedType;
    var array<SequenceEvent> LinkedEvents;
    var string LinkDesc;
    var int DrawX;
    var bool bHidden;

    structdefaultproperties
    {
        ExpectedType=Class'SequenceEvent'
        LinkedEvents=none
        LinkDesc=""
        DrawX=0
        bHidden=false
    }
};

var bool bActive;
var const bool bLatentExecution;
var bool bAutoActivateOutputLinks;
var array<SeqOpInputLink> InputLinks;
var array<SeqOpOutputLink> OutputLinks;
var array<SeqVarLink> VariableLinks;
var array<SeqEventLink> EventLinks;
var noimport transient int PlayerIndex;
var noimport transient byte GamepadID;
var transient int ActivateCount;
var protected{protected} duplicatetransient const transient int SearchTag;

// Export USequenceOp::execHasLinkedOps(FFrame&, void* const)
native final function bool HasLinkedOps(optional bool bConsiderInputLinks);

// Export USequenceOp::execGetLinkedObjects(FFrame&, void* const)
native final function GetLinkedObjects(out array<SequenceObject> out_Objects, optional class<SequenceObject> ObjectType, optional bool bRecurse);

// Export USequenceOp::execGetVectorVars(FFrame&, void* const)
native final function GetVectorVars(out array<Vector> vecVars, optional string inDesc);

// Export USequenceOp::execGetObjectVars(FFrame&, void* const)
native final function GetObjectVars(out array<Object> ObjVars, optional string inDesc);

// Export USequenceOp::execGetInterpDataVars(FFrame&, void* const)
native final function GetInterpDataVars(out array<InterpData> outIData, optional string inDesc);

// Export USequenceOp::execGetBoolVars(FFrame&, void* const)
native final function GetBoolVars(out array<byte> boolVars, optional string inDesc);

// Export USequenceOp::execGetFloatVars(FFrame&, void* const)
native final function GetFloatVars(out array<float> floatVars, optional string inDesc);

// Export USequenceOp::execLinkedVariables(FFrame&, void* const)
native final iterator function LinkedVariables(class<SequenceVariable> VarClass, out SequenceVariable OutVariable, optional string inDesc);

// Export USequenceOp::execActivateOutputLink(FFrame&, void* const)
native final function bool ActivateOutputLink(int OutputIdx);

// Export USequenceOp::execActivateNamedOutputLink(FFrame&, void* const)
native final function bool ActivateNamedOutputLink(string LinkDesc);

event Activated()
{
    //return;    
}

event Deactivated()
{
    //return;    
}

event VersionUpdated(int OldVersion, int NewVersion)
{
    //return;    
}

// Export USequenceOp::execPopulateLinkedVariableValues(FFrame&, void* const)
native final function PopulateLinkedVariableValues();

// Export USequenceOp::execPublishLinkedVariableValues(FFrame&, void* const)
native final function PublishLinkedVariableValues();

event Reset()
{
    //return;    
}

function Pawn GetPawn(Actor TheActor)
{
    local Pawn P;
    local Controller C;

    P = Pawn(TheActor);
    // End:0x24
    if(P != none)
    {
        return P;        
    }
    else
    {
        C = Controller(TheActor);
        return ((C != none) ? C.Pawn : none);
    }
    //return ReturnValue;    
}

function Controller GetController(Actor TheActor)
{
    local Pawn P;
    local Controller C;

    C = Controller(TheActor);
    // End:0x24
    if(C != none)
    {
        return C;        
    }
    else
    {
        P = Pawn(TheActor);
        return ((P != none) ? P.Controller : none);
    }
    //return ReturnValue;    
}

// Export USequenceOp::execForceActivateInput(FFrame&, void* const)
native final function ForceActivateInput(int InputIdx);

defaultproperties
{
    bAutoActivateOutputLinks=true
    InputLinks[0]=(LinkDesc="In",bHasImpulse=false,QueuedActivations=0,bDisabled=false,bDisabledPIE=false,LinkedOp=none,DrawY=0,bHidden=false,ActivateDelay=0.0000000)
    OutputLinks[0]=(Links=none,LinkDesc="Out",bHasImpulse=false,bDisabled=false,bDisabledPIE=false,LinkedOp=none,ActivateDelay=0.0000000,DrawY=0,bHidden=false)
    PlayerIndex=-1
    GamepadID=255
}
