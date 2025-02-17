/**
 * Demonstrates and verifies boolean comparison functionality for delegates.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0024_DelegateComparison extends TestClassBase;

var Test0024_Commandlet CommandletOwner;

delegate ComparisonDelegate( int Param1 )
{
	`log("Executing the default body for ComparisonDelegate");
}

function TestDelegateParam( delegate<ComparisonDelegate> MyDelegate )
{
	`log("Here we are in TestDelegateParm");

	if ( MyDelegate != None )
	{
		`log("MyDelegate has a value");
	}

	if ( ComparisonDelegate != None )
	{
		`log("ComparisonDelegate is assigned a value!");
	}

	if ( MyDelegate == ComparisonDelegate )
	{
		`log("MyDelegate value is same as ComparisonDelegate");
	}
	else if ( MyDelegate == DelegateFunction )
	{
		`log("MyDelegate assigned to DelegateFunction");
	}
//	else if ( MyDelegate == SomeOtherDelegate )
//	{
//		`log("This should be a compiler error");
//	}

	if ( MyDelegate == CommandletOwner.CommandletDelegateFunction )
	{
		`log("MyDelegate is assigned to CommandletOwner's CommandletDelegateFunction");
	}

	MyDelegate( 10 );
}

function DelegateFunction( int Param1 )
{
	`log("DelegateFunction - Param1:" $ Param1);
}

function DelegateExample( Test0024_Commandlet Owner )
{
	CommandletOwner = Owner;

	`log("First call ======================");
	TestDelegateParam(DelegateFunction);

	`log("");
	`log("Second Call ======================");
	TestDelegateParam(None);

	CommandletOwner.CommandletDelegate = CommandletOwner.CommandletDelegateFunction;

	`log("");
	`log("Third Call =====================");
	TestDelegateParam(CommandletOwner.CommandletDelegateFunction);
}

delegate SomeOtherDelegate( bool bFoo );

DefaultProperties
{
	ComparisonDelegate=DelegateFunction
}
