/**
 * Base class of any sequence object that can be executed, such
 * as SequenceAction, SequenceCondtion, etc.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SequenceOp extends SequenceObject
	native(Sequence)
	abstract;

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

/** Is this operation currently active? */
var bool bActive;

/** Does this op use latent execution (can it stay active multiple updates?) */
var const bool bLatentExecution;

/** indicates whether all output links should be activated when this op has finished executing */
var bool bAutoActivateOutputLinks;

/**
 * Represents an input link for a SequenceOp, that is
 * connected via another SequenceOp's output link.
 */
struct native SeqOpInputLink
{
	/** Text description of this link */
	// @fixme - localization
	var string LinkDesc;

	/**
	 * Indicates whether this input is ready to provide data to this sequence operation.
	 */
	var bool bHasImpulse;

	/** Number of activations received for this input when bHasImpulse == TRUE */
	var int QueuedActivations;

	/** Is this link disabled for debugging/testing? */
	var bool bDisabled;

	/** Is this link disabled for PIE? */
	var bool bDisabledPIE;

	/** Linked action that creates this input, for Sequences */
	var SequenceOp LinkedOp;

	// Temporary for drawing! Will think of a better way to do this! - James
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
};
var array<SeqOpInputLink>		InputLinks;

/**
 * Individual output link entry, for linking an output link
 * to an input link on another operation.
 */
struct native SeqOpOutputInputLink
{
	/** SequenceOp this is linked to */
	var SequenceOp LinkedOp;

	/** Index to LinkedOp's InputLinks array that this is linked to */
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
};

/**
 * Actual output link for a SequenceOp, containing connection
 * information to multiple InputLinks in other SequenceOps.
 */
struct native SeqOpOutputLink
{
	/** List of actual connections for this output */
	var array<SeqOpOutputInputLink> Links;

	/** Text description of this link */
	// @fixme - localization
	var string					LinkDesc;

	/**
	 * Indicates whether this link is pending activation.  If true, the SequenceOps attached to this
	 * link will be activated the next time the sequence is ticked
	 */
	var bool					bHasImpulse;

	/** Is this link disabled for debugging/testing? */
	var bool					bDisabled;

	/** Is this link disabled for PIE? */
	var bool					bDisabledPIE;

	/** Linked op that creates this output, for Sequences */
	var SequenceOp				LinkedOp;

	/** Delay applied before activating this output */
	var float					ActivateDelay;

	// Temporary for drawing! Will think of a better way to do this! - James
	var int						DrawY;
	var bool					bHidden;

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
};
var array<SeqOpOutputLink>		OutputLinks;

/**
 * Represents a variable linked to the operation for manipulation upon
 * activation.
 */
struct native SeqVarLink
{
	/** Class of variable that can be attached to this connector. */
	var class<SequenceVariable>	ExpectedType;

	/** SequenceVariables that we are linked to. */
	var array<SequenceVariable>	LinkedVariables;

	/** Text description of this variable's use with this op */
	// @fixme - localization
	var string					LinkDesc;

	/** Name of the linked external variable that creates this link, for sub-Sequences */
	var Name	LinkVar;

	/** Name of the property this variable is associated with */
	var Name	PropertyName;

	/** Is this variable written to by this op? */
	var bool	bWriteable;

	/** do the object(s) pointed to by this variable get modified by this op? (ignored if not an object variable) */
	var bool bModifiesLinkedObject;

	/** Should draw this connector in Kismet. */
	var bool	bHidden;

	/** Minimum number of variables that should be attached to this connector. */
	var int		MinVars;

	/** Maximum number of variables that should be attached to this connector. */
	var int		MaxVars;

	/** For drawing. */
	var int		DrawX;

	/** Cached property ref */
	var const	transient	Property	CachedProperty;

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
	ExpectedType=class'Engine.SequenceVariable'
	MinVars = 1
	MaxVars = 255
}
};

/** All variables used by this operation, both input/output. */
var array<SeqVarLink>			VariableLinks;

/**
 * Represents an event linked to the operation, similar to a variable link.  Necessary
 * only since SequenceEvent does not derive from SequenceVariable.
 * @todo native interfaces - could be avoided by using interfaces, but requires support for native interfaces
 */
struct native SeqEventLink
{
	var class<SequenceEvent>	ExpectedType;
	var array<SequenceEvent>	LinkedEvents;
	// @fixme - localization
	var string					LinkDesc;

	// Temporary for drawing! - James
	var int						DrawX;
	var bool					bHidden;

	structdefaultproperties
	{
		ExpectedType=class'Engine.SequenceEvent'
	}
};
var array<SeqEventLink>			EventLinks;

/**
 * The index [into the Engine.GamePlayers array] for the player that this action is associated with.  Currently only used in UI sequences.
 */
var	transient	noimport	int		PlayerIndex;

/**
 * The ControllerId for the player that generated this action; generally only relevant in UI sequences.
 */
var	transient	noimport	byte	GamepadID;

/** Number of times that this Op has had Activate called on it. Used for finding often-hit ops and optimising levels. */
var transient int				ActivateCount;

/** used when searching for objects to avoid unnecessary recursion */
var transient duplicatetransient const protected{protected} int SearchTag;

/**
 * Determines whether this sequence op is linked to any other sequence ops through its variable, output, event or (optionally)
 * its input links.
 *
 * @param	bConsiderInputLinks		specify TRUE to check this sequence ops InputLinks array for linked ops as well
 *
 * @return	TRUE if this sequence op is linked to at least one other sequence op.
 */
native final function bool HasLinkedOps( optional bool bConsiderInputLinks ) const;

/**
 * Gets all SequenceObjects that are contained by this SequenceObject.
 *
 * @param	out_Objects		will be filled with all ops that are linked to this op via
 *							the VariableLinks, OutputLinks, or InputLinks arrays. This array is NOT cleared first.
 * @param	ObjectType		if specified, only objects of this class (or derived) will
 *							be added to the output array.
 * @param	bRecurse		if TRUE, recurse into linked ops and add their linked ops to
 *							the output array, recursively.
 */
native final function GetLinkedObjects( out array<SequenceObject> out_Objects, optional class<SequenceObject> ObjectType, optional bool bRecurse );

/**
 * Returns all the objects linked via SeqVar_Object, optionally specifying the
 * link to filter by.
 * @fixme - localization
 */
native noexport final function GetObjectVars(out array<Object> objVars,optional string inDesc) const;
/** Retrieve list of UInterpData objects connected to this sequence op. */
native noexport final function GetInterpDataVars(out array<InterpData> outIData,optional string inDesc);

// @fixme - localization
native noexport final function GetBoolVars(out array<BYTE> boolVars,optional string inDesc) const;

/** returns all linked variables that are of the specified class or a subclass
 * @param VarClass the class of variable to return
 * @param OutVariable (out) the returned variable for each iteration
 * @param InDesc (optional) if specified, only variables connected to the link with the given description are returned
 @fixme - localization
 */
native noexport final iterator function LinkedVariables(class<SequenceVariable> VarClass, out SequenceVariable OutVariable, optional string InDesc);

/**
 *	Activates an output link by index
 *	@param OutputIdx output index to set impulse on (if it's not disabled)
 */
native final function bool ActivateOutputLink( int OutputIdx );

/**
 * Activates an output link by searching for the one with a matching LinkDesc.
 *
 * @param	LinkDesc	the string used as the value for LinkDesc of the output link to activate.
 *
 * @return	TRUE if the link was found and activated.
 */
native final function bool ActivateNamedOutputLink( string LinkDesc );

/**
 * Called when this event is activated.
 */
event Activated();

/**
 * Called when this event is deactivated.
 */
event Deactivated();

/**
 * Called when the version is updated, in case any special handling is desired script-side.
 */
event VersionUpdated(int OldVersion, int NewVersion);

/**
 * Copies the values from member variables contained by this sequence op into any VariableLinks attached to that member variable.
 */
native final virtual function PopulateLinkedVariableValues();	// ApplyPropertiesToVariables

/**
 * Copies the values from all VariableLinks to the member variable [of this sequence op] associated with that VariableLink.
 */
native final virtual function PublishLinkedVariableValues();	// ApplyVariablesToProperties

/* Reset() - reset to initial state - used when restarting level without reloading */
function Reset();

/** utility to try to get a Pawn out of the given Actor (tries looking for a Controller if necessary) */
function Pawn GetPawn(Actor TheActor)
{
	local Pawn P;
	local Controller C;

	P = Pawn(TheActor);
	if (P != None)
	{
		return P;
	}
	else
	{
		C = Controller(TheActor);
		return (C != None) ? C.Pawn : None;
	}
}

/** utility to try to get a Controller out of the given Actor (tries looking for a Pawn if necessary) */
function Controller GetController(Actor TheActor)
{
	local Pawn P;
	local Controller C;

	C = Controller(TheActor);
	if (C != None)
	{
		return C;
	}
	else
	{
		P = Pawn(TheActor);
		return (P != None) ? P.Controller : None;
	}
}

native final function ForceActivateInput(int InputIdx);

defaultproperties
{
    // define the base input link required for this op to be activated
	InputLinks(0)=(LinkDesc="In")
	// define the base output link that this action generates (always assumed to generate at least a single output)
	OutputLinks(0)=(LinkDesc="Out")

	bAutoActivateOutputLinks=true

	PlayerIndex=-1
	GamepadID=255
}
