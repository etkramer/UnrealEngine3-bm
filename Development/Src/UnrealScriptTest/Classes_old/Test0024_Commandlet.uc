/**
 * Driver commandlet for testing delegate boolean comparison functionality.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0024_Commandlet extends TestCommandletBase;

delegate CommandletDelegate( int i )
{
	`log("here i am in the commandlet delegate!");
}

event int Main( string Parms )
{
	local Test0024_DelegateComparison TestObj;

	TestObj = new class'Test0024_DelegateComparison';

	TestObj.DelegateExample(Self);
	return 0;
}

function CommandletDelegateFunction( int Param1 )
{
	`log("CommandletDelegateFunction - Param1:" $ Param1);
}

DefaultProperties
{
	LogToConsole=true
}


