/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_RideReaverSetOptions extends SequenceAction;

/** Disable spotlight */
var()	bool	bDisableSpotlight;
/** Disable dodging */
var()	bool	bDisableDodging;
/** Disable vertical dodging */
var()	bool	bDisableVerticalDodging;

defaultproperties
{
	ObjName="RideReaver Set Options"
	ObjCategory="Gear"
}