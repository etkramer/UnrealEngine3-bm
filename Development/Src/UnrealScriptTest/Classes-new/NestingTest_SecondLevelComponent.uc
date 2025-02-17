/**
 * Component for testing instancing nested subobjects.
 * This component serves as a leaf in a nesting chain - it will be at least two layers deep from the subobject root.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class NestingTest_SecondLevelComponent extends TestComponentsBase;

var() editconst float	TestFloat;

DefaultProperties
{
	TestFloat=2
}
