/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTActorFactoryMover extends ActorFactoryDynamicSM
	config(Editor)
	collapsecategories
	hidecategories(Object)
	native;

/** if true, also create a Kismet event and hook it up to the spawned actor */
var() bool bCreateKismetEvent;
/** the class of Kismet event to create if bCreateKismetEvent is true */
var class<SequenceEvent> EventClass;

cpptext
{
	virtual AActor* CreateActor(const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData);
}

defaultproperties
{
	MenuName="Add Mover"
	NewActorClass=class'InterpActor'

	bCreateKismetEvent=true
	EventClass=class'SeqEvent_Mover'
}
