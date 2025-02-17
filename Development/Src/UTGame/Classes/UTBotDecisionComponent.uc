/**
 * when this component gets triggered, it calls its owner UTBot's ExecuteWhatToDoNext() in its next tick
 * this is so the AI's state code, timers, etc happen in TG_PreAsyncWork while the decision logic happens
 * in TG_DuringAsyncWork
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTBotDecisionComponent extends ActorComponent
	native(AI);

var bool bTriggered;

cpptext
{
	virtual void Tick(FLOAT DeltaTime);
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork
}
