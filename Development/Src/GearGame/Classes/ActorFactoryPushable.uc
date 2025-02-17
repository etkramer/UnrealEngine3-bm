/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class ActorFactoryPushable extends ActorFactoryMover
	config(Editor)
	collapsecategories
	hidecategories(Object)
	native;

defaultproperties
{
	MenuName="Add InterpActor_Pushable"
	NewActorClass=class'GearGame.InterpActor_Pushable'
}
