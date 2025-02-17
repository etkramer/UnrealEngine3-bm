/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DialogueManager extends Actor;

function bool TriggerDialogueEvent( class<SequenceEvent> InEventClass, Actor InInstigator, Actor InOriginator );

defaultproperties
{
	TickGroup=TG_DuringAsyncWork
	
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)
}
