
/** 
 * GearAnim_BlendList
 * Base warfare blendlist, adds the following:
 * - Default slider implementation for AnimTree previewing.
 * - per child specific blend in time.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendList extends AnimNodeBlendList
	native(Anim);

cpptext
{
	// AnimTree editor interface	
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
}

// Custom blend in time for each child
var()	Array<float>	ChildBlendInTime;
/** 
 * Blend to DesiredChild index, only if node is not relevant in tree.
 * This is used to blend until a transition animation is played.
 * So typical scenario is:
 *	- Special Move DBNO is set, we have an node triggering when that happens.
 *	- But we want to play a transition first, and wait before the DBNO node is triggered.
 *	- transition animation is played on an AnimNodeSlot who make the DBNO node not relevant
 *	- when DBNO node becomes relevant again, then he is triggered, once transition is done playing.
 */
var()	Array<bool>		BlockSetActiveChildWhenRelevant;

native function SetActiveChild(INT ChildIndex, FLOAT BlendTime);