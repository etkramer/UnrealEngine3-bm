/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactoryVehicle extends ActorFactory
	config(Editor)
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );

	virtual AActor* GetDefaultActor();
};

var() class<Vehicle>	VehicleClass;

defaultproperties
{
	VehicleClass=class'Vehicle'
	bPlaceable=false
}
