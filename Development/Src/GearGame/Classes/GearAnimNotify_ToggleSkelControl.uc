
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnimNotify_ToggleSkelControl extends AnimNotify
	native(Anim);

struct native CtrlInfo
{
	var()	Name	ControlName;
	var()	Name	SeqNodeName;
};

var()	bool			bTurnOn;
var()	float			BlendTime;

var()	Array<CtrlInfo>	Controls;

cpptext
{
	// AnimNotify interface.
	virtual void Notify(class UAnimNodeSequence* NodeSeq);
}

defaultproperties
{
	BlendTime=0.2f
}