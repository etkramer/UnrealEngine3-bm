
/**
 * GearWeap_InkGrenadeKantus
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_InkGrenadeKantus extends GearWeap_InkGrenade
	config(Weapon);


/** delay enforced before a grenade is thrown (so that you don't have him firing and throwing at the same instant) */
var config float GrenadeThrowDelay;

simulated function bool ShouldTryToThrowGrenade(GearAI AI, vector EnemyLocation, out int EnemiesInRange)
{
	//debug
	//`log(GetFuncName()@"TimeSince:"@TimeSince( AI.LastGrenadeTime )@"("$AI_TimeTweenGrenade$")"@ AI_ChanceToUseGrenade @ AI.GrenadeChanceScale);

	EnemiesInRange = 1;
	return (!AI.MyGearpawn.IsInCover() &&
		AI.bAllowCombatTransitions &&
		!AI.MyGearPawn.IsDoingASpecialMove() &&
		!AI.MyGearPawn.IsReloadingWeapon() &&
		AI.MyGearPawn.CanFireWeapon() &&
		TimeSince( AI.LastGrenadeTime ) > AI_TimeTweenGrenade &&
		FRand() < (AI_ChanceToUseGrenade * AI.GrenadeChanceScale) &&
		AI.CanFireAt(AI.FireTarget, AI.MyGearPawn.GetWeaponStartTraceLocation(),,self));
}

//// hax to throw a grenade without switching to it
simulated function float GetWeaponRating()
{
	local GearAI_Kantus GAI;
	local float SuperVal;

	GAI = GearAI_Kantus(Instigator.Controller);
	//`log(GetFuncname()@self@"--------->>> Instigator:"@Instigator@" Instigator.Enemy:"@GAI.Enemy);
	if(GAI.Enemy != none)
	{
		SuperVal = Super.GetWeaponRating();
		//`log(GetFuncname()@self@"------>>> SuperVal:"@SuperVal);
		if(SuperVal > 0.f && TimeSince(GAI.LastSuccessfulKnockdownTime) >= GAI.MinTimeBetweenKnockDownScreams)
		{
			GAI.ForceThrowGrenade();
		}
	}

	return -1.f;
}

function ForceThrowGrenade(GearAI GAI)
{
	local GearPawn_LocustKantusBase Kantus;

	// since we didn't go through state 'Active' these might not have been set
	AIController = GAI;
	GearAIController = GAI;

	StartGrenadeSpin();

	GAI.LastGrenadeTime = WorldInfo.TimeSeconds;
	// notify the pawn right away so it can do whatever it needs to
	Kantus = GearPawn_LocustKantusBase(Instigator);
	if (Kantus != None)
	{
		Kantus.ThrowingInkGrenade(GrenadeThrowDelay);
	}
}

simulated function ThrowingOffHandGrenade();

/** Start the grenade spinning animation */
simulated function StartGrenadeSpin()
{
	local FLOAT		PlayLength;
	local GearPawn	P;
	local GearPawn_LocustKantusBase Kantus;

	P = GearPawn(Instigator);

	P.BS_Play(BS_PawnPrepare, GlobalPlayRate, 0.3f, -1.f, FALSE, FALSE);
	PlayLength	= PlayWeaponAnim(WeapAnimPrepare, GlobalPlayRate, 0.3f, -1.f, FALSE, FALSE);
	SetCurrentFireMode(FIREMODE_GRENADESPIN);

	// Dummy timer to track the grenade being thrown
	SetTimer( 100.f, FALSE, nameof(ThrowingOffHandGrenade) );
	SetTimer( PlayLength, FALSE, nameof(PreparedNotify) );

	bSpinningGrenade = TRUE;

	if( PSC_SpinningParticleEffect == none )
	{
		if( SpinningParticleEffectTemplate != none )
		{
			PSC_SpinningParticleEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( SpinningParticleEffectTemplate );
			SkeletalMeshComponent(Mesh).AttachComponentToSocket(  PSC_SpinningParticleEffect, SpinningEffectSocketName );
		}
	}
	// so now after possibly spawning any effect we check to see if we actually have one to activate
	if( PSC_SpinningParticleEffect != none )
	{
		PSC_SpinningParticleEffect.ActivateSystem();
	}

	Kantus = GearPawn_LocustKantusBase(Instigator);
	if ( (Kantus != None) && (GearAI(Instigator.Controller) != None) )
	{
		Kantus.TelegraphGrenadeThrow();
	}
}

/** Notification called when prepared animation is done playing */
simulated function PreparedNotify()
{
	local GearPawn	P;
	local float		ExcessTime, NewPlayRate;

	P = GearPawn(Instigator);
	if( P != None )
	{
		// Find out by how much we went over the ideal time.
		// By correcting the new animation, this makes a seamless transition.
		ExcessTime = GetTimerCount('PreparedNotify') - GetTimerRate('PreparedNotify');

		// Play looping animation
		P.BS_Play(BS_PawnLoop, GlobalPlayRate, 0.1f, 0.25f, TRUE, FALSE);
		NewPlayRate = P.BS_GetPlayRate(BS_PawnLoop);
		P.BS_SetPosition(BS_PawnLoop, ExcessTime * NewPlayRate);

		PlayWeaponAnim(WeapAnimLoop, GlobalPlayRate, 0.1f, 0.f, TRUE, FALSE);
		SetWeaponAnimPosition(ExcessTime * NewPlayRate);

		bSpinningGrenade = TRUE;

		SetTimer( 0.8f, FALSE, nameof(DelayedPlayFireEffects) );
	}
}

simulated function DelayedPlayFireEffects()
{
	local GearPawn	P;

	P = GearPawn(Instigator);
	if (P != none && !P.IsDoingASpecialMove())
	{
		PlayFireEffects(0);
	}
	else
	{
		if (Role == ROLE_Authority)
		{
			// reset the timer so we try again soon
			P.MyGearAI.LastGrenadeTime = 0;
		}
		StopWeaponAnim(0.25f);
	}

	SetCurrentFireMode(0);
}

/** Overriden from GearWeap_GrenadeBase, so we do blend out the release animation. */
simulated function PlayFireEffects(byte FireModeNum, optional vector HitLocation)
{
	local GearPawn	P;
	local float Delay;

	P = GearPawn(Owner);
	if( P == None )
	{
		return;
	}

	if( bSpinningGrenade )
	{
		// Clear timer to play looping animation if it hasn't been triggered yet
		ClearTimer('PreparedNotify');

		// Play Release animation
		ThrowAnimLength = P.BS_Play(BS_PawnRelease, GlobalPlayRate, 0.1f, 0.33f, FALSE, FALSE);
		PlayWeaponAnim(WeapAnimRelease, GlobalPlayRate, 0.1f, 0.1f, FALSE, FALSE);

		// Set Timer to throw grenade.
		SetTimer( 0.9f, FALSE, nameof(AnimNotifyThrow) );

		bSpinningGrenade	= FALSE;
		bThrownBoloStyle	= TRUE;
	}
	else
	{
		// Blind fire
		bThrownBoloStyle	= FALSE;

		// Play Throwing animation
		ThrowAnimLength = P.BS_Play(BS_PawnThrowGrenade, 1.f, 0.1f, 0.33f, FALSE);

		// Set Timer to throw grenade.
		switch (P.CoverAction)
		{
		case CA_BlindUp:
			Delay = BlindFireUpThrowDelay;
			break;
		case CA_BlindLeft:
		case CA_BlindRight:
			Delay = BlindFireSideThrowDelay;
			break;
		default:
			// standard underhand throw
			Delay = UntargetedThrowDelay;
		}

		SetTimer( Delay, FALSE, nameof(AnimNotifyThrow) );
		SetTimer( Delay*0.5f, FALSE, nameof(PlayGrenadeThrowEffort) );
	}

	// Adjust dummy timer to the exact throw duration
	if( ThrowAnimLength > 0.f )
	{
		SetTimer( ThrowAnimLength, FALSE, nameof(ThrowingOffHandGrenade) );
	}
	else
	{
		`DLog("ThrowAnimLength == 0.f. Calling ThrowingOffHandGrenade() right away. Instigator:" @ Instigator);
		ThrowingOffHandGrenade();
	}
}

simulated function bool HasAnyAmmo()
{
	return true;
}

/** Off hand grenade attached to left hand. */
simulated event Name GetDefaultHandSocketName(GearPawn P)
{
	return P.GetLeftHandSocketName();
}

defaultproperties
{
	AIRating=1.f

	WeaponProjectiles(0)=class'GearProj_InkGrenadeKantus'

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		AnimSets(0)=AnimSet'Locust_PoisonGrenade.Locust_Poinson_Grenade'
    End Object

	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_Camskel_BurstPistol')
	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Grenade')

	BS_PawnPrepare={(
		AnimName[BS_Std_Up]				="kantus_grenade_throw_start",
		AnimName[BS_Std_Idle_Lower]		="kantus_grenade_throw_start",
		AnimName[BS_CovStdLean_Up]		="kantus_grenade_throw_start",
		AnimName[BS_CovMidLean_Up]		="kantus_grenade_throw_start",
		AnimName[BS_CovStd_360_Upper]	="kantus_grenade_throw_start",
		AnimName[BS_CovMid_360_Upper]	="kantus_grenade_throw_start"
	)}
	BS_PawnLoop={(
		AnimName[BS_Std_Up]				="kantus_grenade_throw_loop",
		AnimName[BS_Std_Idle_Lower]		="kantus_grenade_throw_loop",
		AnimName[BS_CovStdLean_Up]		="kantus_grenade_throw_loop",
		AnimName[BS_CovMidLean_Up]		="kantus_grenade_throw_loop",
		AnimName[BS_CovStd_360_Upper]	="kantus_grenade_throw_loop",
		AnimName[BS_CovMid_360_Upper]	="kantus_grenade_throw_loop"
	)}
	BS_PawnRelease={(
		AnimName[BS_Std_Up]				="kantus_grenade_throw_end",
		AnimName[BS_Std_Idle_Lower]		="kantus_grenade_throw_end",
		AnimName[BS_CovStdLean_Up]		="kantus_grenade_throw_end",
		AnimName[BS_CovMidLean_Up]		="kantus_grenade_throw_end",
		AnimName[BS_CovStd_360_Upper]	="kantus_grenade_throw_end",
		AnimName[BS_CovMid_360_Upper]	="kantus_grenade_throw_end"
	)}
	BS_PawnThrowGrenade={(
		AnimName[BS_Std_Up]			    ="kantus_side_throw",
		AnimName[BS_Std_Idle_Lower]	    ="kantus_side_throw",
		AnimName[BS_CovMidBlindSd_Up]	="kantus_side_throw",
		AnimName[BS_CovMidBlindUp_Up]	="kantus_side_throw",
		AnimName[BS_CovMid_360_Upper]	="kantus_side_throw",
		AnimName[BS_CovStdBlind_Up]		="kantus_side_throw",
		AnimName[BS_CovStd_360_Upper]	="kantus_side_throw",
	)}

	WeapAnimPrepare="kantus_grenade_intro"
	WeapAnimLoop="kantus_grenade_loop"
	WeapAnimRelease="kantus_grenade_end"
}
