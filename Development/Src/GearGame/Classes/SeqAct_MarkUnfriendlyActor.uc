/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_MarkUnfriendlyActor extends SequenceAction;

/** List of targets to choose from. */
var() array<Actor> UnfriendlyList;

/** Replace the existing targets? */
var() bool bOverwriteExisting;

defaultproperties
{
    ObjName="Mark Unfriendly Actors"
    ObjCategory="Gear"

    bOverwriteExisting=TRUE

    VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Unfriendly",PropertyName=UnfriendlyList)
}

