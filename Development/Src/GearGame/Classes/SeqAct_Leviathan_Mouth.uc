/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Leviathan_Mouth extends SeqAct_Latent
	native(Sequence);

cpptext
{
	virtual UBOOL UpdateOp( FLOAT DeltaTime );
};

/** List of valid targets inside the mouth */
var array<Actor> Victims;

/** Duration that jaw/mouth is open */
var	float		 Duration;

/** Number of tentacles to hit before garbage can opens */
var int			 NumTentaclesToHurtToOpenMouth;


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
	return Super.GetObjClassVersion() + 9;
}

defaultproperties
{
	ObjName="Leviathan: Mouth"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Mouth Battle")
	InputLinks(1)=(LinkDesc="Abort Battle")
	
	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failure")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Leviathan",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Victims",PropertyName=Victims)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Duration",PropertyName=Duration)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Num Tentacles",PropertyName=NumTentaclesToHurtToOpenMouth)

	NumTentaclesToHurtToOpenMouth=3
	Duration=10.f
}