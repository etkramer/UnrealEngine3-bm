/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_CentaurSettings extends SequenceAction;

/** Controls spotlight on the vehicle */
var()	bool	bEnableSpotlight;

/** Controls snow effect attached to vehicle */
var()	bool	bEnableSnowEffects;

/** Enable / disable the cannon firing */
var()   bool    bSuppressCannonFire;

/** Enable boosting  */
var()	bool	bEnableBoost;

/** If TRUE, engine noise turns off */
var()	bool	bDisableEngineNoise;

/** Max speed of the centaur. */
var()	float	CentaurMaxSpeed;

/** Overall scaling for wheels longitudinal grip */
var()	float	CentaurLongGripScale;

/** Overall scaling for wheels lateral grip */
var()	float	CentaurLatGripScale;

/** Additional spinning to apply to graphics of wheels. */
var()	float	CentaurExtraWheelSpin;

/** What surface the centaur is currently on - for effects */
var()	ECentaurOnSurface	CentaurOnSurface;

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
	ObjName="Centaur Settings"
	ObjCategory="Gear"

	bEnableSpotlight=TRUE
	bEnableSnowEffects=TRUE
	bSuppressCannonFire=FALSE
	bEnableBoost=TRUE
	CentaurMaxSpeed=2000
	CentaurOnSurface=ECOS_Snow

	CentaurLongGripScale=1.0
	CentaurLatGripScale=1.0

	CentaurExtraWheelSpin=3.0
}
