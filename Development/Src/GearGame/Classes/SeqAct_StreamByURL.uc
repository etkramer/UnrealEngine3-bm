/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_StreamByURL extends SeqAct_Latent
	native(Sequence);

cpptext
{
	UBOOL UpdateOp(FLOAT DeltaTime);
};

/** What stage are we in in the latent action */
var int Stage;

defaultproperties
{
	ObjName="Stream Level From URL"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Stream")
	OutputLinks(0)=(LinkDesc="Finished")
	
	VariableLinks.Empty
} 
