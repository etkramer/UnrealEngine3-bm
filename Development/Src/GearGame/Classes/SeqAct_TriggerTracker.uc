class SeqAct_TriggerTracker extends SeqAct_Latent
	native(Sequence);

cpptext
{
	virtual void Initialize();
	virtual void OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex );
	virtual void DeActivated();
	virtual UBOOL UpdateOp( FLOAT DeltaTime );

	void SetTracking( UBOOL bTrack );
	void SetupTrackingList();
	void ClearTrackingList();
};

/** Trigger volumes to track */
var TriggerVolume	BackTrigVol;
var TriggerVolume	FrontTrigVol;

/** Action is tracking touch events or not */
var bool bTracking;
/** Helper actor for associated touches to the volumes */
var duplicatetransient TriggerTrackerHelper Helper;

struct native TriggerTrackerInfo
{
	var()	Actor	Target;
	var()	bool	bTouchedFrontTrigger;
};

var array<TriggerTrackerInfo> InfoList;

/** called by the TriggerTrackerHelper when the associated volumes are touched */
function NotifyTouch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local int Idx;
	local TriggerVolume TV;

	Idx = InfoList.Find( 'Target', Other );
	if( Idx >= 0 )
	{
		foreach Other.TouchingActors( class'TriggerVolume', TV )
		{
			if( TV == BackTrigVol )
			{
				InfoList[Idx].bTouchedFrontTrigger = FALSE;
			}
			if( TV == FrontTrigVol )
			{
				InfoList[Idx].bTouchedFrontTrigger = TRUE;
			}
		}
	}
}

/**
* Return the version number for this class.  Child classes should increment this method by calling Super then adding
* a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
* link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
* Super.GetObjClassVersion() should be incremented by 1.
*
* @return	the version number for this specific class.
*/
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Trigger Tracker"
	ObjCategory="Gear"
	bCallHandler=FALSE

	InputLinks(0)=(LinkDesc="Activate")
	InputLinks(1)=(LinkDesc="DeActivate")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Targets",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Back Vol",PropertyName=BackTrigVol,MaxVars=1)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Front Vol",PropertyName=FrontTrigVol,MaxVars=1)
}
