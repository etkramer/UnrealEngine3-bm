/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Leviathan_Jaw extends SeqAct_Latent;

/** List of valid targets inside the mouth */
var array<Actor> Victims;
var Trigger		 InnerTrigger;

/** Duration that jaw/mouth is open */
var	float		 Duration;


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
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Leviathan: Jaws"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Open Jaws")
	
	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failure")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Leviathan",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Victims",PropertyName=Victims)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Inner Trig",PropertyName=InnerTrigger)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Float',LinkDesc="Duration",PropertyName=Duration)

	Duration=10.f
}