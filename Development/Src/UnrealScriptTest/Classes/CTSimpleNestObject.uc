/**
 * Used for testing and verifying actors which contain instanced subobjects which contain components that do not themselves contain components.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CTSimpleNestObject extends TestClassBase
	native;

var()	CTSimpleNestChild		InnerComponent;

DefaultProperties
{
	Begin Object Class=CTSimpleNestChild Name=InnerComponentTemplate
		UnmodifiedInt=20
		ModifiedFloat=20.f
	End Object
	InnerComponent=InnerComponentTemplate
}
