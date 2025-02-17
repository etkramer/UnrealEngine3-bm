/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_AIGrapple extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void	Activated();
	virtual UBOOL	UpdateOp(FLOAT DeltaTime);
	virtual void	DeActivated();

	virtual void PostLoad();

	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/** This will allow use to put the grapple hook into the GearGameContent package and then set it only in packages that are using this kismet **/
	void SetTheHardReferences();
}

var() array<Actor> GrappleTargets;

/** The name of the GrappleRope class **/
var private string GrappleRopeClassName;

var class<GrappleRopeBase> GrappleRopeClass;

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
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	ObjName="Grapple To"
	ObjCategory="AI"

	bLatentExecution=TRUE

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Grapple To",PropertyName=GrappleTargets)
	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Finished")


	GrappleRopeClassName="GearGameContent.GrappleRope"
}

