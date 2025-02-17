/**
 * SeqAct_ManagePOI - Action that drives a GearPointOfInterest
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ManagePOI extends SequenceAction
	Config(Game)
	native(Sequence);


/************************************************************************/
/* Constants, Enums, Structs, etc.										*/
/************************************************************************/

/** Enum of the possible inputs of this action */
enum EManagePOIInputType
{
	eMPOIINPUT_On,
	eMPOIINPUT_Off,
};

/** Enum of the possible outputs of this action */
enum EManagePOIOutputType
{
	eMPOIOUTPUT_Out,
	eMPOIOUTPUT_Expired,
	eMPOIOUTPUT_LookAt,
	eMPOIOUTPUT_LookAway,
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** -------Begin list of variables that mimic the properties in GearPointOfInterest------- */

/** The name of the lookup variable for finding the localized text in the [POIS] section of the GearGame.int file */
var() config string				POI_DisplayName;

/** Whether the POI is enabled or not (on, off) */
var bool						POI_bEnabled;

/** The length of time the Y-Look icon will show on the screen when this POI is enabled */
var() config float				POI_IconDuration;

/** Whether this POI will force the player to look at it or not */
var() config EPOIForceLookType	POI_ForceLookType;

/** The length of time the ForceLook will happen, if POI_ForceLookType is set - 0 is not supported */
var() config float				POI_ForceLookDuration;

/** Whether or not a line of sight check should occur when doing a ForceLook, if POI_ForceLookType is set */
var() config bool				POI_bForceLookCheckLineOfSight;

/** All POIs have a priority, and this is the LD's way of overriding that priority (should only be used when absolutely necessary) */
var() config int				POI_LookAtPriority;

/** Field of view to zoom to when the player looks at the POI */
var() config float				POI_DesiredFOV;

/** The number of times the POI will perform the POI_DesiredFOV zoom when the player looks at it - 0 means infinite */
var() config int				POI_FOVCount;

/** Whether or not to do a line of sight check when performing the POI_DesiredFOV zoom when the player looks at the POI */
var() config bool				POI_bFOVLineOfSightCheck;

/** Whether or not this POI should force all other active scripted POIs to disable themselves */
var() config bool				POI_bDisableOtherPOIs;

/** The length of time this POI will remain enabled - the POI will auto disable afterward - 0 means infinite */
var() config float				POI_EnableDuration;

/** If true, leaves player facing in the POIs direction after ending look, instead of swinging back to original facing direction. */
var() config bool				POI_bLeavePlayerFacingPOI;

/** -------End of list of variables that mimic the properties in GearPointOfInterest------- */


/** Whether this action is done or not */
var bool bIsDone;

/** The POI this action is referring to */
var GearPointOfInterest POI;

/************************************************************************/
/* C++ functions                                                        */
/************************************************************************/
cpptext
{
	virtual void Activated();

	/**
	 * Polls to see if the async action is done
	 * @param ignored
	 * @return TRUE if the operation has completed, FALSE otherwise
	 */
	UBOOL UpdateOp(FLOAT);

	/** Checks for input impulses and calls the proper function accordingly */
	void ProcessInputImpulses();
}

defaultproperties
{
	ObjName="POI Manager"
	ObjCategory="Gear"

	bAutoActivateOutputLinks=false
	bLatentExecution=true

	InputLinks(eMPOIINPUT_On)=(LinkDesc="On")
	InputLinks(eMPOIINPUT_Off)=(LinkDesc="Off")

	OutputLinks(eMPOIOUTPUT_Out)=(LinkDesc="Out")
	OutputLinks(eMPOIOUTPUT_Expired)=(LinkDesc="Expired")
	OutputLinks(eMPOIOUTPUT_LookAt)=(LinkDesc="Look At")
	OutputLinks(eMPOIOUTPUT_LookAway)=(LinkDesc="Look Away")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="POI",PropertyName=Targets)
}
