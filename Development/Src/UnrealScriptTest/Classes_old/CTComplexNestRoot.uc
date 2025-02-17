/**
 * Used for testing and verifying actors which contain components that contain more than one layer of nesting; i.e. the component contains
 * a component or subobject, and the nested component or subobject contains any components.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CTComplexNestRoot extends ComponentTestActorBase
	native;

struct native ComplexNestStruct
{
	var() instanced CTSimpleNestObject			ComplexNestStructObject;
	var() instanced array<CTSimpleNestObject>	ComplexNestStructObjectArray;
};

var()	instanced	CTSimpleNestObject		SimpleNestObject;

var()	instanced	ComplexNestStruct		SimpleNestObjectStruct;

DefaultProperties
{
	Begin Object Class=CTSimpleNestObject Name=SimpleNestObjectTemplate
	End Object
	SimpleNestObject=SimpleNestObjectTemplate

	Begin Object Class=CTSimpleNestObject Name=StructObjectTemplate
	End Object
	Begin Object Class=CTSimpleNestObject Name=StructArrayObjectTemplateA
	End Object
	Begin Object Class=CTSimpleNestObject Name=StructArrayObjectTemplateB
	End Object

	SimpleNestObjectStruct={(
							ComplexNestStructObject=StructObjectTemplate,
							ComplexNestStructObjectArray=(StructArrayObjectTemplateA,StructArrayObjectTemplateB)
							)}
}
