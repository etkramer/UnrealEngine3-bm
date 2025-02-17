/**
 *  To run this do the following:  run UnrealScriptTest.Test0020_Commandlet
 *
 * Part of the unrealscript execution and compilation regression framework.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0020_Commandlet extends TestCommandletBase;

event int Main( string Parms )
{
	PerformStructSerializationTest();

	return 0;
}


final function PerformStructSerializationTest()
{
	local Test0020_StructDefaults TestObj;

	TestObj = new(self) class'Test0020_StructDefaults';

	TestObj.LogPropertyValues();
}

DefaultProperties
{
	LogToConsole=True
}
