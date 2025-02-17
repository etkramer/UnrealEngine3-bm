/**
 * Driver commandlet for tests which verify the behavior calling function which have the same name as private
 * functions in a base class.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0021_Commandlet extends TestCommandletBase;

event int main( string Params )
{
	local int Retval;

	`log( " --= Test0021_Commandlet =--" );
	Retval = Test0021();

	return Retval;
}

function int Test0021()
{
	local int Result;
	local Test0021_PrivateFunctionLookupDerived TestObject;

	Result = 0;

	`log( "Running:  Test0003" );
	TestObject = new(self) class'Test0021_PrivateFunctionLookupDerived';
	TestObject.Test0021();

	return Result;
}

DefaultProperties
{

}
