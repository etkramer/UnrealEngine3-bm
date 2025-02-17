/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTProj_FlakShardMain extends UTProj_FlakShard;

/** max bonus damage when (this) center flak shard hits enemy pawn */
var float CenteredDamageBonus;

/** max bonus momentum when (this) center flak shard hits enemy pawn */
var float CenteredMomentumBonus;

/** max time when any bonus still applies */
var float MaxBonusTime;

/** 
  * Increase damage to UTPawns based on how centered this shard is on target.  If it is within the time MaxBonusTime time period.
  * e.g. point blank shot with the flak cannon you will do mega damage.  Once MaxBonusTime passes then this shard becomes a normal shard.
  */
simulated function float GetDamage(Actor Other, vector HitLocation)
{
	local float MaxRadius;

	if ( (LifeSpan < default.LifeSpan - MaxBonusTime) || (UTPawn(Other) == None) )
	{
		return Damage;
	}

	MaxRadius = Pawn(Other).CylinderComponent.CollisionRadius;
	return Damage + CenteredDamageBonus * (LifeSpan - Default.LifeSpan + MaxBonusTime) 
			* FMax(0, 2*MaxRadius - PointDistToLine(Other.Location, normal(Velocity), HitLocation))/MaxRadius;
}

/** 
  * Increase momentum imparted based on how recently this shard was fired
  */
simulated function float GetMomentumTransfer()
{
	if ( LifeSpan < default.LifeSpan - MaxBonusTime )
	{
		return MomentumTransfer;
	}
	return MomentumTransfer + CenteredMomentumBonus * (LifeSpan - Default.LifeSpan + MaxBonusTime);
}

defaultproperties
{
	CenteredMomentumBonus=90000
	CenteredDamageBonus=100.0
	MaxBonusTime=0.2
	Bounces=3
}
