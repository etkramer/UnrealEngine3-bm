/**
 * Used for testing and verifying actors which contain components that do not themselves contain components or subobjects.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CTNoNestRoot extends ComponentTestActorBase
	native;

var()		CTNoNesting			UnmodifiedComponent_SingleRef;
var()		CTNoNesting			ModifiedComponent_SingleRef;

var()		CTNoNesting			UnmodifiedComponent_MultipleRef;
var()		CTNoNesting			ModifiedComponent_MultipleRef;

var()		array<CTNoNesting>	UnmodifiedComponent_Array;
var()		array<CTNoNesting>	ModifiedComponent_Array;

DefaultProperties
{
	Begin Object Class=CTNoNesting Name=UnmodifiedComponent_SingleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object
	Begin Object Class=CTNoNesting Name=ModifiedComponent_SingleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object

	UnmodifiedComponent_SingleRef=UnmodifiedComponent_SingleRefTemplate
	ModifiedComponent_SingleRef=ModifiedComponent_SingleRefTemplate


	Begin Object Class=CTNoNesting Name=UnmodifiedComponent_MultipleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object
	Begin Object Class=CTNoNesting Name=ModifiedComponent_MultipleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object

	UnmodifiedComponent_MultipleRef=UnmodifiedComponent_MultipleRefTemplate
	ModifiedComponent_MultipleRef=ModifiedComponent_MultipleRefTemplate
	UnmodifiedComponent_Array.Add(UnmodifiedComponent_MultipleRefTemplate)
	ModifiedComponent_Array.Add(ModifiedComponent_MultipleRefTemplate)
}
