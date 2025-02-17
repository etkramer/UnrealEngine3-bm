/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTActorFactoryVehicle extends ActorFactoryVehicle
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
}

/** whether the vehicle starts out locked and can only be used by the owning team */
var() bool bTeamLocked;
/** the number of the team that may use this vehicle */
var() byte TeamNum;
/** if set, force vehicle to be a key vehicle (displayed on map and considered more important by AI) */
var() bool bKeyVehicle;

defaultproperties
{
	VehicleClass=class'UTVehicle'
	bTeamLocked=true
}
