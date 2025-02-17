/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ReaverLand extends SequenceAction;

/** If TRUE, request Reaver to take off instead of land. */
var()	bool	bTakeOff;
/** If TRUE, reaver will not fire rockets when on the ground */
var()	bool	bSuppressRocketsOnLand;
/** Min time between rockets firing */
var()	float	MinTimeBetweenRockets;

defaultproperties
{
	ObjName="Reaver Land"
	ObjCategory="Gear"
}