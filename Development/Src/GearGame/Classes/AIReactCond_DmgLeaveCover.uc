/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* will trip cause a guy to leave his cover after taking so much damage/almost taking so much damage
*/
class AIReactCond_DmgLeaveCover extends AIReactCondition_Base
	within GearAI_Cover;

/** Percent of our health we take at one cover slot (within DamgaeDecayTime) before bailing and finding a new one */
var() float DamageThreshPct;
/** Amount of time it takes for our current damage value to decay back to 0 */
var() float DamageDecayTime;
/** if set, don't trigger if our enemy has less health than we do */
var() bool bSkipIfBeatingEnemy;

/** Damage we take at one cover slot (within DamgaeDecayTime) before bailing and finding a new one */
var float DamageThresh;
/** The last time we took damage in our current cover slot */
var float LastDamageTime;
/** the current amount value of our damage bail checks */
var float DamageTaken;
/** the cover slot we were at last time we checked stuff out */
var CoverInfo LastCover;

function float GetDamageThresh()
{

	if(DamageThresh < 0.f && MyGearPawn != none)
	{
		DamageThresh = MyGearPawn.default.DefaultHealth * DamageThreshPct;
	}

	return DamageThresh;
}
event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan)
{
	local AIReactChan_Damage DmgChan;
	local float ElapsedTimeSinceLastDamage;

	// super pwns, also if we're tethered to a spot, don't leave it!
	if(!Super.ShouldActivate(EventInstigator,OrigChan) || !AllowedToMove())
	{
		return false;
	}

	DmgChan = AIReactChan_Damage(OrigChan);
	if(DmgChan != none && MyGearPawn.IsInCover())
	{
		// check to see if we are in a new cover slot
		if(LastCover.Link != Cover.Link || LastCover.SlotIdx != Cover.SlotIdx)
		{
			// reset the counter since the cover has changed since last we took damage
			DamageTaken = 0.f;
			LastCover = Cover;
			LastDamageTime = WorldInfo.TimeSeconds;
		}

		// decay damage based on how much time has elapsed
		ElapsedTimeSinceLastDamage = WorldInfo.TimeSeconds - LastDamageTime;
		DamageTaken = Max(0.f, DamageTaken - clamp(ElapsedTimeSinceLastDamage/DamageDecayTime,0.f,1.f) * GetDamageThresh());
		LastDamageTime = WorldInfo.TimeSeconds;

		// add in the damage we just now took --

		// if this was a near miss use the pretend damage value from the GDT instead
		if(!DmgChan.bDirectDamage)
		{
			DamageTaken += DmgChan.DamageType.default.NearMissPretentDamageValue;
		}
		else
		{
			DamageTaken += DmgChan.DamageAmt;
		}

		//MessagePlayer(GetFuncName()@self@"DmgTaken:"@DamageTaken@"InDmg:"@DmgChan.DamageAmt@"Elapsed:"@ElapsedTimeSinceLastDamage@"Thresh:"@DamageThresh@"Headshotgib?"@DmgChan.damageType.default.bAllowHeadShotGib);


		// check to see if the amount of damage we've sustained at this cover slot exceeds the threshold
		if(DamageTaken > GetDamageThresh() && (!bSkipIfBeatingEnemy || Enemy == None || Enemy.Health > Pawn.Health))
		{
			return true;
		}
	}

	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel)
{
	//MessagePlayer(GetFuncName()@self@outer);
	DamageTaken = 0.f;
	InvalidateCoverFromDamage(EventInstigator);
}


defaultproperties
{
	AutoSubscribeChannels(0)=Damage

	DamageThreshPct=0.25f
	DamageDecayTime=3.0f
	DamageThresh=-1.f
}
