/**
 * Test cases for accessing struct members in different ways.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0022_Commandlet extends TestCommandletBase;

event int Main( string Parms )
{
	local int TestId;
	local Test0022_StructMemberAccess TestObject;

	if ( Parms == "" )
	{
		`log("You must specify the id (0 - 4) of the test to run!");
		return 1;
	}

	TestId = int(Parms);
	TestObject = new class'Test0022_StructMemberAccess';

	switch ( TestId )
	{
		case 0:
			TestObject.SimpleStructAccess_NonCtorTest();
			break;
		case 1:
			TestObject.SimpleStructAccess_CtorTest();
			break;
		case 2:
			TestObject.ArrayStructAccessTest();
			break;
		case 3:
			TestObject.ReturnValueAccessTest();
			break;
		case 4:
			TestObject.InlineStructAccessTest();
			break;
		case 5:
			TestObject.TestInvalidArrayExpansion();
			break;
//		case 6:
//			TestObject.
//			break;
	}

	return 0;
}

DefaultProperties
{
	LogToConsole=true
}
