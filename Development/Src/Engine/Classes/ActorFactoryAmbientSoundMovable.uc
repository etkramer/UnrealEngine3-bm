/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactoryAmbientSoundMovable extends ActorFactory
	config( Editor )
	collapsecategories
	hidecategories( Object )
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor( FString& OutErrorMsg );
	virtual void AutoFillFields( class USelection* Selection );
	virtual FString GetMenuName( void );
}

var()	SoundCue		AmbientSoundCue;

defaultproperties
{
	MenuName="Add AmbientSoundMovable"
	NewActorClass=class'Engine.AmbientSoundMovable'
}
