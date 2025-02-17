/**
 * Class: GUDBank
 * A GUDSBank is a
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GUDBank extends Object
	dependson(GUDTypes)
	native(Sound);

struct native GUDLine
{
//	var string		Text;			// can we remove this?  used only for debugging
	var	SoundCue	Audio;
	var	EGUDRole	Addressee;
	/** Used mainly in responses, to handle a line directed at one person, but referring to another. */
	var EGUDRole	ReferringTo;
	/** TRUE if this line should be in the "core" streaming package for this character, which is always loaded. */
	var byte		bAlwaysLoad;
	/** Used by cooker. Fits in the padding, shouldn't eat memory. */
	var byte		bCookerProcessed;
	/** GUDEvent(s) to trigger upon completion of this action.  One will be chosen randomly from the list. */
	var init array<EGUDEventID>	ResponseEventIDs;

	structdefaultproperties
	{
		Addressee=GUDRole_None
	}
};
var array<GUDLine>		GUDLines;

struct native GUDAction
{
	/** Line will be chosen randomly from these if in combat, otherwise it will default to LineIndices */
	var array<int>	CombatOnlyLineIndices;
	var array<int>	NonCombatOnlyLineIndices;
	/** Line will be chosen randomly from these */
	var array<int>	LineIndices;
};
var array<GUDAction>	GUDActions;

/** for cooked GUDBank objects, the class path to the GUDBank subclass that was used to generate it */
var string SourceGUDBankPath;

defaultproperties
{
}
