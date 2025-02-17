
/**
 * GearWeap_InkGrenadeKantusCover
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_InkGrenadeKantusCover extends GearWeap_InkGrenade;


simulated function bool ShouldTryToThrowGrenade(GearAI AI, vector EnemyLocation, out int EnemiesInRange)
{
	EnemiesInRange = 1;
	return (AI.MyGearpawn.IsInCover() &&
		AI.bAllowCombatTransitions &&
		TimeSince( AI.LastGrenadeTime ) > AI_TimeTweenGrenade &&
		FRand() < (AI_ChanceToUseGrenade * AI.GrenadeChanceScale));
}

defaultproperties
{
	WeaponProjectiles(0)=class'GearProj_InkGrenadeKantus'
}
