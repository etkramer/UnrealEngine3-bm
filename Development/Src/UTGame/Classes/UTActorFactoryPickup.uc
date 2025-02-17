/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/** creates a pickup (NOT pickup factory) in the world */
class UTActorFactoryPickup extends ActorFactory
	native;

var() class<Inventory> InventoryClass;

cpptext
{
	virtual AActor* CreateActor(const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData);
}

defaultproperties
{
	NewActorClass=class'UTDroppedPickup'
	bPlaceable=false
}
