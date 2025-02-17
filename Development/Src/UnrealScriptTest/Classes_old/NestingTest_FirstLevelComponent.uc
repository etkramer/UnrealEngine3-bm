/**
 * Component for testing instancing nested subobjects.
 * This component contains its own instanced subobjects and components.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class NestingTest_FirstLevelComponent extends TestComponentsBase;

var	instanced NestingTest_SecondLevelSubobject		NestedSubobject;

var NestingTest_SecondLevelComponent				NestedComponent;

DefaultProperties
{
	Begin Object Class=NestingTest_SecondLevelSubobject Name=Component_NestedSubobjectTemplate
	End Object
	NestedSubobject=Component_NestedSubobjectTemplate

	Begin Object Class=NestingTest_SecondLevelComponent Name=Component_NestedComponentTemplate
	End Object
	NestedComponent=Component_NestedComponentTemplate
}
