/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_DoorControl extends SeqAct_Latent
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
	void SetDoorState( UBOOL bOpen );

};

/** Starting state of the door */
var() bool	bStartOpen;
/** Current state of the door */
var	  bool	bCurrentOpen;
/** Desired state of the door */
var	  bool	bDesiredOpen;
/** Auto activate input on matinee objects */
var() bool	bAutoPlayMatinee;
/** Door should check encroachment w/ blocking volumes */
var() bool	bCheckEncroachOnMove;
/** Should continue to try to move after fail */
var() bool	bRetryOnFail;
/** If TRUE, do extra work (slow!) to fix cover slipping for nodes. */ 
var() bool bAutoAdjustCover;
/** Next time should retry door */
var	  float NextRetryTime;
/** Number of times to retry */
var() byte	NumRetry;
/** Current retry count */
var	  byte	RetryCount;
/** Number of times to trigger before considered completely open */
var() byte	NumStepsToOpen;
/** Current step in the open process */
var	  byte	CurStepToOpen;

/** Volumes to disable when the level loads b/c they were used only for cover building */
var() array<BlockingVolume>		BlockingVolumes;
/** Cover links w/ slots touching the door */
var() array<CoverLink>			CoverLinks;
/** Delay before adjusting cover on open/shut */
var() float						DelayAdjustCoverOpen;
var() float						DelayAdjustCoverShut;
/** Nav points to force paths between */
var() array<NavigationPoint>	NavPoints;

/** List of names of matinee objects */
var() array<String>		MatineeObjList;
/** Ptr to the matinee actor that controls this door */
var array<SeqAct_Interp> MatineeList;

var duplicatetransient DoorControlHelper Helper;

/** script tick interface
 * the action deactivates when this function returns false and LatentActors is empty
 * @return whether the action needs to keep ticking
 */
event bool Update( float DeltaTime )
{
	// If not matching the desired state
	if( CanRetryDoor() )
	{
		return TRUE;
	}
	return FALSE;
}

native function bool CanRetryDoor();

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
	return Super.GetObjClassVersion() + 2;
}

defaultproperties
{
	ObjName="Door Control"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Open")
	InputLinks(1)=(LinkDesc="Shut")
	InputLinks(2)=(LinkDesc="Toggle")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Blocking Volumes",PropertyName=BlockingVolumes)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Cover Links",PropertyName=CoverLinks)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Nav Points",PropertyName=NavPoints)

	OutputLinks(0)=(LinkDesc="Open - Out")
	OutputLinks(1)=(LinkDesc="Shut - Out")
	OutputLinks(2)=(LinkDesc="Failed")


	bCallHandler=FALSE
	bCheckEncroachOnMove=FALSE
	bRetryOnFail=FALSE

	NumStepsToOpen=1
}

