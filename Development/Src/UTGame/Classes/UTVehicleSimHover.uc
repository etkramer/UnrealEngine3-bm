/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVehicleSimHover extends UTVehicleSimChopper
	native(Vehicle);

var		bool	bDisableWheelsWhenOff;
var		bool	bRepulsorCollisionEnabled;
var		bool	bCanClimbSlopes;
var		bool	bUnPoweredDriving;

cpptext
{
	virtual void UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime);
	FLOAT GetEngineOutput(ASVehicle* Vehicle);
	virtual void GetRotationAxes(ASVehicle* Vehicle, FVector &DirX, FVector &DirY, FVector &DirZ);
}

defaultproperties
{
	bDisableWheelsWhenOff=true
	bRepulsorCollisionEnabled=true
}
