/**
 * Non-component counterpart for NestingTest_FirstLevelComponent
 * This object contains its own instanced subobjects and components.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class NestingTest_FirstLevelSubobject extends TestClassBase;

struct NestedObjectStruct
{
	var() instanced NestingTest_SecondLevelSubobject StructObject;
	var() instanced array<NestingTest_SecondLevelSubobject> StructObjectArray;
};

var()	instanced NestingTest_SecondLevelSubobject		NestedSubobject;

var() NestingTest_SecondLevelComponent				NestedComponent;

var()	NestedObjectStruct							NestedStruct;

DefaultProperties
{
	Begin Object Class=NestingTest_SecondLevelSubobject Name=Subobject_NestedSubobjectTemplate
	End Object
	NestedSubobject=Subobject_NestedSubobjectTemplate

	Begin Object Class=NestingTest_SecondLevelComponent Name=Subobject_NestedComponentTemplate
	End Object
	NestedComponent=Subobject_NestedComponentTemplate

	NestedStruct={(
				StructObject=Subobject_NestedSubobjectTemplate,
				StructObjectArray=(Subobject_NestedSubobjectTemplate,Subobject_NestedSubobjectTemplate)
				)}
}
