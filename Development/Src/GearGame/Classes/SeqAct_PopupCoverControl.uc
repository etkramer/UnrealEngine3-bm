class SeqAct_PopupCoverControl extends SeqAct_Latent
	native(Sequence);

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void Initialize();
	virtual void Activated();
	virtual void DeActivated();
	virtual UBOOL UpdateOp( FLOAT DeltaTime );

	virtual void PrePathBuild(  AScout* Scout );
	virtual void PostPathBuild( AScout* Scout );

	virtual void OnDelete();

	void ResolveMatineeNames();
	void SetCoverState( UBOOL bUp );
};

/** Starting state of the cover */
var() bool	bStartPoppedUp;
/** Current state of the cover */
var	  bool	bCurrentPoppedUp;
/** Desired state of cover */
var	  bool	bDesiredPoppedUp;
/** Auto activate input on matinee objects */
var() bool	bAutoPlayMatinee;
/** Popup should check encroachment w/ blocking volumes */
var() bool	bCheckEncroachOnPopUp;
/** Should continue to try to change popup after fail */
var() bool	bRetryOnFailPopUp;
/** If TRUE, do extra work (slow!) to fix cover slipping for nodes. */ 
var() bool bAutoAdjustCover;
/** Next time should retry popup */
var	  float NextRetryTime;
/** Number of times to retry */
var() byte	NumRetry;
/** Current retry count */
var	  byte	RetryCount;

/** Volumes to disable when the level loads b/c they were used only for cover builidng */
var() array<BlockingVolume> BlockingVolumes;
/** Cover links w/ slots touching the popup cover */
var() array<CoverLink>	CoverLinks;
var deprecated array<LinkSlotHelper>	CoverList;
/** Delay before adjusting cover on up/down */
var() float						DelayAdjustCoverUp;
var() float						DelayAdjustCoverDown;
/** List of cover slot markers affected by popup cover */
var() editconst array<CoverSlotMarker> MarkerList;

/** List of names of matinee objects */
var() array<String>		MatineeObjList;
/** Ptr to the matinee actor that controls this cover */
var array<SeqAct_Interp> MatineeList;

var duplicatetransient PopupCoverControlHelper Helper;


/** script tick interface
 * the action deactivates when this function returns false and LatentActors is empty
 * @return whether the action needs to keep ticking
 */
event bool Update( float DeltaTime )
{
	// If not matching the desired state
	if( CanRetryPopup() )
	{
		return TRUE;
	}
	return FALSE;
}

native function bool CanRetryPopup();

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
	return Super.GetObjClassVersion() + 4;
}

defaultproperties
{
	ObjName="Popup Cover Control"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Up")
	InputLinks(1)=(LinkDesc="Down")
	InputLinks(2)=(LinkDesc="Toggle")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Blocking Volumes",PropertyName=BlockingVolumes)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Cover Links",PropertyName=CoverLinks)

	OutputLinks(0)=(LinkDesc="Up - Out")
	OutputLinks(1)=(LinkDesc="Down - Out")
	OutputLinks(2)=(LinkDesc="Failed")


	bCallHandler=FALSE
	bCheckEncroachOnPopUp=TRUE
	bRetryOnFailPopUp=FALSE
}

