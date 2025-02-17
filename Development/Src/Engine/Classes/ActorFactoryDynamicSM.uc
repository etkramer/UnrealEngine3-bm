/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactoryDynamicSM extends ActorFactory
	config(Editor)
	native
	abstract;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor(FString& OutErrorMsg);
	virtual void AutoFillFields(class USelection* Selection);
	virtual FString GetMenuName();
	virtual void PostLoad();
}

var()	StaticMesh		StaticMesh;
var()	vector			DrawScale3D;
var()	bool			bNoEncroachCheck;
var()	bool			bNotifyRigidBodyCollision;
var()	ECollisionType		CollisionType;

/** Try and use physics hardware for this spawned object. */
var()	bool			bUseCompartment;

/** If false, primitive does not cast dynamic shadows. */
var()	bool			bCastDynamicShadow;

defaultproperties
{
	DrawScale3D=(X=1,Y=1,Z=1)
	CollisionType=COLLIDE_NoCollision
	bCastDynamicShadow=true
}
