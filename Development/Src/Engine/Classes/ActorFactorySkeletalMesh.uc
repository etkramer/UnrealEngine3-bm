/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactorySkeletalMesh extends ActorFactory
	config(Editor)
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor(FString& OutErrorMsg);
	virtual void AutoFillFields(class USelection* Selection);
	virtual FString GetMenuName();
}

var()	SkeletalMesh	SkeletalMesh;
var()	AnimSet			AnimSet;
var()	name			AnimSequenceName;

defaultproperties
{
	MenuName="Add SkeletalMesh"
	NewActorClass=class'Engine.SkeletalMeshActor'
	GameplayActorClass=class'Engine.SkeletalMeshActorSpawnable'
}
