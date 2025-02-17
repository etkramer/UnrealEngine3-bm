/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactoryFogVolumeLinearHalfspaceDensityInfo extends ActorFactory
	config(Editor)
	native(FogVolume);

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual void AutoFillFields(class USelection* Selection);
}

var	MaterialInterface SelectedMaterial;

defaultproperties
{
	MenuName="Add FogVolumeLinearHalfspaceDensityInfo"
	NewActorClass=class'Engine.FogVolumeLinearHalfspaceDensityInfo'
}
