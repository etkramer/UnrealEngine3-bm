/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactoryStaticMesh extends ActorFactory
	config(Editor)
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor(FString& OutErrorMsg);
	virtual void AutoFillFields(class USelection* Selection);
	virtual FString GetMenuName();
}

var()	StaticMesh		StaticMesh;
var()	vector			DrawScale3D;

defaultproperties
{
	DrawScale3D=(X=1,Y=1,Z=1)

	MenuName="Add StaticMesh"
	NewActorClass=class'Engine.StaticMeshActor'
}
