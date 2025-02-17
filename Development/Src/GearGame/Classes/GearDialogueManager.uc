/**
 * Class: GearDialogueManager
 * Takes incoming dialogue events and filters them the next tick to decide what should be played
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDialogueManager extends DialogueManager;

//
// I believe this is rendered obsolete by GUDS, but holding on to it until that's all complete...
//

/**
 * Storage struct for pending list
 */
//struct DialogueEvent
//{
//	var Actor	Instigator;
//	var Actor	Originator;
//	var class<SequenceEvent> EventClass;
//};
//var array<DialogueEvent> PendingList;

/** Place incoming events into pending list to be evaluated next tick */
function bool TriggerDialogueEvent( class<SequenceEvent> InEventClass, Actor InInstigator, Actor InOriginator )
{
	//local DialogueEvent Event;

	//Event.Instigator = InInstigator;
	//Event.Originator = InOriginator;
	//Event.EventClass = InEventClass;

	//PendingList[PendingList.Length] = Event;

	//return true;
}
//
//function Tick( float DeltaTime )
//{
//	local int EventIdx;
//	local GearPawn Orig;
//
//	// Discard all pending events, that overlap or are somehow undesireable
//	if( PendingList.Length > 0 )
//	{
//		FilterPendingList();
//	}
//
//	// Go through each pending event and activate
//	for( EventIdx = 0; EventIdx < PendingList.Length; EventIdx++ )
//	{
//		Orig = GearPawn(PendingList[EventIdx].Originator);
//		if( Orig != None )
//		{
//			Orig.ActivateEventClass( PendingList[EventIdx].EventClass, PendingList[EventIdx].Instigator, Orig.DialogueEvents );
//		}
//	}
//	PendingList.Length = 0;
//}
//
///** Remove unwanted items from the pending list */
//function FilterPendingList()
//{
//	FilterByOriginator();
//	FilterByEvent();
//}
//
//function FilterByOriginator()
//{
//	local int PendingIdx;
//	local array<DialogueEvent>	WorkList, KeepList;
//	local Actor					EventOriginator;
//
////	`log( "Filter By Originator" );
//
//	// Originator can only say one thing at a time...
//	while( PendingList.Length > 0 )
//	{
//		WorkList.Length = 0;
//		EventOriginator = None;
//
//		// Get a list of all messages for the same originator
//		for( PendingIdx = 0; PendingIdx < PendingList.Length; PendingIdx++ )
//		{
////			`log( "Pending"@EventOriginator@PendingList[PendingIdx].Originator@PendingList[PendingIdx].EventClass );
//
//			if( EventOriginator == None )
//			{
//				EventOriginator = PendingList[PendingIdx].Originator;
//			}
//
//			if( PendingList[PendingIdx].Originator == EventOriginator )
//			{
//				WorkList[WorkList.Length] = PendingList[PendingIdx];
//				PendingList.Remove( PendingIdx--, 1 );
//			}
//		}
//
//		// Sort messages by priority (highest priority first)
//		GetByPriority( WorkList );
//
//		// Keep only one of the items remaining in the work list
//		if( WorkList.Length > 0 )
//		{
//			KeepList[KeepList.Length] = WorkList[Rand(WorkList.Length)];
//		}
//	}
//
//	// New pending list
//	PendingList = KeepList;
//
///*	for( PendingIdx = 0; PendingIdx < PendingList.Length; PendingIdx++ )
//	{
//		`log( "KEEP:"@PendingList[PendingIdx].Originator@PendingList[PendingIdx].EventClass@PendingList[PendingIdx].Instigator );
//	}*/
//}
//
//final function FilterByEvent()
//{
//	local int PendingIdx;
//	local array<DialogueEvent>	WorkList, KeepList;
//	local Actor					EventInstigator;
//	local class<SequenceEvent>	EventClass;
//
////	`log( "Filter By Instigator/Event" );
//
//	// Filter out same events seen by multiple originators (ie 4 ppl see same enemy go down)
//	while( PendingList.Length > 0 )
//	{
//		WorkList.Length = 0;
//		EventInstigator = None;
//		EventClass		= None;
//
//		// Get a list of all messages for the same originator
//		for( PendingIdx = 0; PendingIdx < PendingList.Length; PendingIdx++ )
//		{
////			`log( "Pending"@EventInstigator@PendingList[PendingIdx].Instigator@PendingList[PendingIdx].EventClass );
//
//			if( EventInstigator == None )
//			{
//				EventInstigator = PendingList[PendingIdx].Instigator;
//				EventClass		= PendingList[PendingIdx].EventClass;
//			}
//
//			if( PendingList[PendingIdx].Instigator == EventInstigator &&
//				PendingList[PendingIdx].EventClass == EventClass )
//			{
//				WorkList[WorkList.Length] = PendingList[PendingIdx];
//				PendingList.Remove( PendingIdx--, 1 );
//			}
//		}
//
//		// Keep only one of the items remaining in the work list
//		if( WorkList.Length > 0 )
//		{
//			KeepList[KeepList.Length] = WorkList[Rand(WorkList.Length)];
//		}
//	}
//
//	// New pending list
//	PendingList = KeepList;
//
///*
//	for( PendingIdx = 0; PendingIdx < PendingList.Length; PendingIdx++ )
//	{
//		`log( "KEEP:"@PendingList[PendingIdx].Originator@PendingList[PendingIdx].EventClass@PendingList[PendingIdx].Instigator );
//	}*/
//}
//
///** Sort input list by priority */
//final function SortByPriority( out array<DialogueEvent> out_List )
//{
//	local int i, j;
//	local Byte Highest;
//	local DialogueEvent Swap;
//
//	for( i = 0; i < out_List.Length; i++ )
//	{
//		for( j = i + 1; j < out_List.Length; j++ )
//		{
//			if( out_List[j].EventClass.default.Priority > Highest )
//			{
//				Highest = out_List[j].EventClass.default.Priority;
//				Swap = out_List[i];
//				out_List[i] = out_List[j];
//				out_List[j] = Swap;
//			}
//		}
//	}
//}
//
///** Returns a list of events with the highest priority from the input list */
//final function GetByPriority( out array<DialogueEvent> out_List )
//{
//	local int i;
//	local Byte Highest;
//
//	for( i = 0; i < out_List.Length; i++ )
//	{
//		if( out_List[i].EventClass.default.Priority > Highest )
//		{
//			Highest = out_List[i].EventClass.default.Priority;
//			out_List.Remove( 0, i );
//			i = 0;
//		}
//	}
//}

defaultproperties
{
}
