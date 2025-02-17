/**
 *
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class Test0025_NativeIssues extends TestClassBase
	native;

enum EBoolOrderEnum
{
	BOOLORDER_One,
	BOOLORDER_Two,
	BOOLORDER_Three,
};

var	EBoolOrderEnum	EnumBeforeBoolTestVar;

var	bool			bBoolAfterEnumTestVar;


native final function ShowNativeBoolAssignmentTestResults();

function RunTest()
{
	EnumBeforeBoolTestVar = BOOLORDER_Three;
	bBoolAfterEnumTestVar = true;

	`log(`showvar(EnumBeforeBoolTestVar));
	`log(`showvar(bBoolAfterEnumTestVar));
	ShowNativeBoolAssignmentTestResults();


	bBoolAfterEnumTestVar = FALSE;
	`log("");
	ShowNativeBoolAssignmentTestResults();
	`log(`showvar(EnumBeforeBoolTestVar));
	`log(`showvar(bBoolAfterEnumTestVar));
}

DefaultProperties
{

}
