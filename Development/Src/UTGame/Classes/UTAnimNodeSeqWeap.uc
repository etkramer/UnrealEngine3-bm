/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *	AnimNodeSequence which changes its animation based on the weapon anim type.
 */

class UTAnimNodeSeqWeap extends UTAnimNodeSequence
	native(Animation);

var()	name	DefaultAnim;
var()	name	DualPistolAnim;
var()	name	SinglePistolAnim;
var()	name	ShoulderRocketAnim;
var()	name	StingerAnim;

cpptext
{
	virtual FString GetNodeTitle();

	void WeapTypeChanged(FName NewAimProfileName);
}