/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTActorFactoryAI extends ActorFactoryAI
	native;

cpptext
{
	virtual AActor* CreateActor(const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData);
};

var() bool bForceDeathmatchAI;

/** Try and use physics hardware for this spawned object. */
var() bool bUseCompartment;

defaultproperties
{
	ControllerClass=class'UTBot'
}
