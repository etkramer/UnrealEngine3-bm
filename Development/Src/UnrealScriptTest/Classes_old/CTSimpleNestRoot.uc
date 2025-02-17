/**
 * Used for testing and verifying actors which contain components that contain one layer of nesting; i.e. the component contains
 * a component or subobject, but the nested component or subobject does not contain any components.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CTSimpleNestRoot extends ComponentTestActorBase
	native;

var()	CTSimpleNestParent	UnmodifiedNestParent_SingleRef;
var()	CTSimpleNestParent	ModifiedNestParent_SingleRef;

var()	CTSimpleNestParent	UnmodifiedNestParent_MultipleRef;
var()	CTSimpleNestParent	ModifiedNestParent_MultipleRef;

var()	array<CTSimpleNestParent>	UnmodifiedNestParent_Array;
var()	array<CTSimpleNestParent>	ModifiedNestParent_Array;

DefaultProperties
{
	Begin Object Class=CTSimpleNestChild Name=UnmodifiedChild_NotDefined_SingleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object
	Begin Object Class=CTSimpleNestChild Name=ModifiedChild_NotDefined_SingleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object
	Begin Object Class=CTSimpleNestChild Name=UnmodifiedChild_NotDefined_MultipleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object
	Begin Object Class=CTSimpleNestChild Name=ModifiedChild_NotDefined_MultipleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object

	Begin Object Class=CTSimpleNestParent Name=UnmodifiedNestParent_SingleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
		UnmodifiedChild_NotDefined_SingleRef=UnmodifiedChild_NotDefined_SingleRefTemplate
		UnmodifiedChild_NotDefined_MultipleRef=UnmodifiedChild_NotDefined_MultipleRefTemplate
		UnmodifiedChild_Undefined_Array.Add(UnmodifiedChild_NotDefined_MultipleRefTemplate)
	End Object
	Begin Object Class=CTSimpleNestParent Name=ModifiedNestParent_SingleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
		ModifiedChild_NotDefined_SingleRef=ModifiedChild_NotDefined_SingleRefTemplate
		ModifiedChild_NotDefined_MultipleRef=ModifiedChild_NotDefined_MultipleRefTemplate
		ModifiedChild_Undefined_Array.Add(ModifiedChild_NotDefined_MultipleRefTemplate)
	End Object
	UnmodifiedNestParent_SingleRef=UnmodifiedNestParent_SingleRefTemplate
	ModifiedNestParent_SingleRef=ModifiedNestParent_SingleRefTemplate



	Begin Object Class=CTSimpleNestParent Name=UnmodifiedNestParent_MultipleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
		UnmodifiedChild_NotDefined_SingleRef=UnmodifiedChild_NotDefined_SingleRefTemplate
		UnmodifiedChild_NotDefined_MultipleRef=UnmodifiedChild_NotDefined_MultipleRefTemplate
		UnmodifiedChild_Undefined_Array.Add(UnmodifiedChild_NotDefined_MultipleRefTemplate)
	End Object
	Begin Object Class=CTSimpleNestParent Name=ModifiedNestParent_MultipleRefTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
		ModifiedChild_NotDefined_SingleRef=ModifiedChild_NotDefined_SingleRefTemplate
		ModifiedChild_NotDefined_MultipleRef=ModifiedChild_NotDefined_MultipleRefTemplate
		ModifiedChild_Undefined_Array.Add(ModifiedChild_NotDefined_MultipleRefTemplate)
	End Object



	UnmodifiedNestParent_MultipleRef=UnmodifiedNestParent_MultipleRefTemplate
	ModifiedNestParent_MultipleRef=ModifiedNestParent_MultipleRefTemplate
	UnmodifiedNestParent_Array.Add(UnmodifiedNestParent_MultipleRefTemplate)
	ModifiedNestParent_Array.Add(ModifiedNestParent_MultipleRefTemplate)
}
