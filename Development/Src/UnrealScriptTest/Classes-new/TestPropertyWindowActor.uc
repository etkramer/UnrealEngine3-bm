/**
 * Used for testing property window functionality.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class TestPropertyWindowActor extends Actor
	HideCategories(Object,Actor,Advanced,Attachment,Collision,Debug,Physics)
	placeable;

struct NestedArrayStruct
{
	var() int NormalIntVar;
	var() editinline array<string> NestedStringArray;
};

var()   class<Actor>                            ActorClassVar;
var()   editinline  array<int>                  IntArrayVar;

var()	NestedArrayStruct						StructVar;
var()	editinline	array<NestedArrayStruct>	StructArrayVar;

var()	editinline instanced array<Object>		ObjectArray;
var()	instanced object						ObjectStaticArray[3];

var()	InheritanceTestDerived					RuntimeComponent;


var()	array<InheritanceTestDerived>			ComponentDynArray;

var()	InheritanceTestDerived					ComponentStaticArray[2];


DefaultProperties
{
	IntArrayVar(0)=10
	IntArrayVar(1)=20
	IntArrayVar(2)=30

	StructVar=(NormalIntVar=10,NestedStringArray=("StringA","StringB"))
	StructArrayVar(0)=(NormalIntVar=10,NestedStringArray=("StringA","StringB"))
	StructArrayVar(1)=(NormalIntVar=20,NestedStringArray=("StringZ","StringY"))

	Begin Object Class=ScriptedTexture Name=ObjectA
	End Object

	Begin Object Class=NestingTest_FirstLevelSubobject Name=ObjectB
	End Object


	ObjectArray(0)=ObjectA
	ObjectArray(1)=ObjectB

	ObjectStaticArray(1)=ObjectA
	ObjectStaticArray(2)=ObjectB

	Begin Object Class=InheritanceTestDerived Name=TestComp
		TestInt=5
	End Object
	Components.Add(TestComp)
	RuntimeComponent=TestComp

	Begin Object Class=InheritanceTestDerived Name=TestComp2
		TestInt=10
	End Object
	ComponentDynArray.Add(TestComp2)

	ComponentStaticArray(0)=TestComp
	ComponentStaticArray(1)=TestComp2
}
