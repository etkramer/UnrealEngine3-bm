/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AIReactCond_EnemyCloseToFireLine extends AIReactCond_Conduit_Base;

var() float DistFromFireLineThresh;

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	local float DistFromFireLine;
	local pawn PawnInst;

	if (!Super.ShouldActivate(EventInstigator,OriginatingChannel))
	{
		return false;
	}

	// extra checking here because security turrets (which use this) don't have PRI's, and are on special neutral teams so this would otherwise get tripped by other turrets
	PawnInst = Pawn(EventInstigator);
	if(PawnInst == none || (PawnInst.PlayerReplicationInfo == none && Pawn.PlayerReplicationInfo == none) || PawnInst.Health <= 0)
	{
		return false;
	}

	DistFromFireLine = PointDistToLine(EventInstigator.Location,vector(Pawn.GetBaseAimRotation()),Pawn.Weapon.GetMuzzleLoc());
	if(DistFromFireLine < DistFromFireLineThresh)
	{
		return true;
	}

	return false;
}

DefaultProperties
{
	AutoSubscribeChannels(0)=Sight

	DistFromFireLineThresh=64.f
}
