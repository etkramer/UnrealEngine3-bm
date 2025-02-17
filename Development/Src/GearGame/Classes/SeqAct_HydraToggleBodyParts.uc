/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_HydraToggleBodyParts extends SequenceAction;

enum ETogglePartsInfo
{
	TPI_Helmet,
	TPI_Butt,
	TPI_Mouth01,
	TPI_Mouth02,
	TPI_Mouth03,
	TPI_FrontLeftTentacle,
	TPI_BackLeftTentacle,
	TPI_FrontRightTentacle,
	TPI_BackRightTentacle,
};

var() array<ETogglePartsInfo>	Parts;
/** Force the body part to fall off when toggled ON */
var() bool						bForceFallOff;

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
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Hydra: Toggle Body Parts"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Turn On")
	InputLinks(1)=(LinkDesc="Turn Off")
	InputLinks(2)=(LinkDesc="Toggle")
}

