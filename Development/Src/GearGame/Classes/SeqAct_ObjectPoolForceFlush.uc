/**
 * This will force flush the object pool of all the transient cached objects it holds.
 * This idea behind this is that you sometimes don't need the data it has and want to jsut flush it all away
 * for a memory heavy cinematic or level transition.
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ObjectPoolForceFlush extends SequenceAction;

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


event Activated()
{
	local GearPC PC;

	// duplicated in SeqAct_CineCleanWorldPre
	// This will cause the object pool to flush and a GC will follow
	foreach GetWorldInfo().LocalPlayerControllers( class'GearPC', PC )
	{
		PC.ClientObjectPoolForceFlush();
	}
}



defaultproperties
{
	ObjName="Object Pool Force Flush"
	ObjCategory="Misc"

	VariableLinks.Empty
}
