/**
 * Test calling a function which has the same name as a private function in a base class.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0021_PrivateFunctionLookupBase extends TestClassBase;

function Test0021()
{
	local rotator Rotation;

	Rotation.Pitch = 10;
	Rotation.Yaw = 20;
	Rotation.Roll = 30;

	DifferentNumberOfParams(10.f, Rotation);
}


// The derived version this function has a different number of parameters
private function DifferentNumberOfParams(float DeltaTime, rotator CurrentRotation)
{
	`log(GetFuncName()@`showvar(DeltaTime)@"Rotation:" $ CurrentRotation);
}

DefaultProperties
{

}
