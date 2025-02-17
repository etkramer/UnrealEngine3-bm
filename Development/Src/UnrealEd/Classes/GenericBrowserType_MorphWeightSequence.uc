/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//-----------------------------------------------------------
// Browser type for morph weight sequence
//-----------------------------------------------------------
class GenericBrowserType_MorphWeightSequence extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}

DefaultProperties
{
	Description="MorphWeights"
}
