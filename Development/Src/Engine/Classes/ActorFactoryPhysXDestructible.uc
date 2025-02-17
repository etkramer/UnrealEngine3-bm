/*=============================================================================
	ActorFactoryPhysXDestructible.uc: Destructible Vertical Component.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

class ActorFactoryPhysXDestructible extends ActorFactory
	config(Editor)
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor(FString& OutErrorMsg);
	virtual void AutoFillFields(class USelection* Selection);
	virtual FString GetMenuName();
}

var()	PhysXDestructible		PhysXDestructible;
var()	vector			DrawScale3D;

defaultproperties
{
	DrawScale3D=(X=1,Y=1,Z=1)

	MenuName="Add PhysXDestructibleActor"
	NewActorClass=class'Engine.PhysXDestructibleActor'
}
