/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactoryDecal extends ActorFactory
	config(Editor)
	native(Decal);

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor(FString& OutErrorMsg);
	virtual void AutoFillFields(class USelection* Selection);
	virtual FString GetMenuName();
}

var()	MaterialInterface	DecalMaterial;

defaultproperties
{
	MenuName="Add Decal"
	NewActorClass=class'Engine.DecalActor'
}
