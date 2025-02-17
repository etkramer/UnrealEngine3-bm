/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Will trigger a new channel nudge when we see/hear an enemy at a far away location from where we last knew of them **/
class AIReactCond_EnemyMoved extends AIReactCond_Conduit_Base
	native;

/** if we haven't seen a guy for at least this long, trigger a channel nudge **/
var() float DistanceThreshold;

const ThreshPosBufferSize = 20;
struct native LastThreshPosPair
{
	var Pawn Enemy;
	var vector Position;
};

/** ring buffer of enemies to last threshold locations **/
var native array<LastThreshPosPair> EnemyThreshList;
/** current index into the ring buffer **/
var int RingBufIndex;

cpptext
{
	void AddNewRec(APawn* Pawn);
	INT FindRec(APawn* Pawn, FVector& outLoc);
}

/** private native function that will determine if the passed enemy's location is new, and if so update it **/
native function private bool IsEnemyInNewLocation( Pawn TestEnemy );

/** Activates if we have not seen the incoming dude in a long time (or haven't ever seen) **/
event bool ShouldActivate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	return (Super.ShouldActivate(EventInstigator, OriginatingChannel) && IsEnemyInNewLocation(Pawn(EventInstigator)));
}

defaultproperties
{
	AutoSubscribeChannels(0)=Sight
	AutoSubscribeChannels(1)=Hearing
	AutoSubscribeChannels(2)=Damage
	DistanceThreshold=480.0f
	OutputChannelname=EnemyMoved
}
