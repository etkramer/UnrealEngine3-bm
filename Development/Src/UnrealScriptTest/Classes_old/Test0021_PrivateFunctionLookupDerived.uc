/**
 * Test calling a function which has the same name as a private function in a base class.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0021_PrivateFunctionLookupDerived extends Test0021_PrivateFunctionLookupBase;

// The derived version this function has a different number of parameters
function DifferentNumberOfParams( float DeltaTime )
{
	`log(GetFuncName()@`showvar(DeltaTime));
}

DefaultProperties
{

}
