class GearWeap_InkGrenadeSkorge extends GearWeap_InkGrenadeKantus
	config(Weapon);


simulated function bool ShouldTryToThrowGrenade(GearAI AI, vector EnemyLocation, out int EnemiesInRange)
{
	local GearAI_Skorge SAI;

	EnemiesInRange = 1;
	SAI = GearAI_Skorge(Instigator.Controller);
	return (SAI != None &&
				SAI.bPistolStrafeAttack &&
				TimeSince( SAI.LastGrenadeTime ) > AI_TimeTweenGrenade );
}


//// hax to throw a grenade without switching to it
simulated function float GetWeaponRating()
{
	local GearAI_Skorge GAI;
	local float SuperVal;

	GAI = GearAI_Skorge(Instigator.Controller);
	if(GAI.Enemy != none)
	{
		SuperVal = Super(GearWeap_InkGrenade).GetWeaponRating();
		if( SuperVal > 0.f )
		{
			ForceThrowGrenade(GAI);
		}
	}

	return -1.f;
}

function ForceThrowGrenade(GearAI GAI)
{
	local GearPawn_LocustSkorgeBase Skorge;

	// since we didn't go through state 'Active' these might not have been set
	AIController = GAI;
	GearAIController = GAI;

	if( !IsTimerActive('DelayedPlayFireEffects') )
	{
		// actually throw the grenade in a jiffy
		SetTimer( GrenadeThrowDelay*0.5f,,nameof(DelayedPlayFireEffects) );

		// notify the pawn right away so it can do whatever it needs to
		Skorge = GearPawn_LocustSkorgeBase(Instigator);
		if (Skorge != None)
		{
			Skorge.ThrowingInkGrenade(GrenadeThrowDelay);
		}
	}
}

defaultproperties
{
	BS_PawnThrowGrenade={(
		AnimName[BS_Std_Up]			    ="Pistol_Grenade_Toss",
		AnimName[BS_Std_Idle_Lower]	    ="Pistol_Grenade_Toss",
		AnimName[BS_CovMidBlindSd_Up]	="Pistol_Grenade_Toss",
		AnimName[BS_CovMidBlindUp_Up]	="Pistol_Grenade_Toss",
		AnimName[BS_CovMid_360_Upper]	="Pistol_Grenade_Toss",
		AnimName[BS_CovStdBlind_Up]		="Pistol_Grenade_Toss",
		AnimName[BS_CovStd_360_Upper]	="Pistol_Grenade_Toss",
	)}

}
