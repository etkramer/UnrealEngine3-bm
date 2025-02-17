/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


//-----------------------------------------------------------
//
//-----------------------------------------------------------
class InheritanceTestBase extends TestComponentsBase
	editinlinenew;

var() editconst	int									TestInt;
var() editconst	editinline NestingTest_SecondLevelComponent	TestComponent;

DefaultProperties
{
	TestInt=2

	Begin Object Class=NestingTest_SecondLevelComponent Name=NestingTestComponent
		TestFloat=5
	End Object
	TestComponent=NestingTestComponent
}
