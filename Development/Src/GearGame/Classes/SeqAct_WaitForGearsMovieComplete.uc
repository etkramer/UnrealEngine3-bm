/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_WaitForGearsMovieComplete extends SeqAct_Latent
	native(Sequence);

cpptext
{
	UBOOL UpdateOp(FLOAT DeltaTime);
};

/** The name of the movie to wait on */
var() string MovieName;

defaultproperties
{
	ObjName="Wait For Gears Movie"
	ObjCategory="Cinematic"

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="MovieName",PropertyName=MovieName)
}
