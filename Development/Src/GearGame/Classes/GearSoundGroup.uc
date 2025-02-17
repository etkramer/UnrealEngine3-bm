/**
 * GearSoundGroup
 * Defines a character's sound group.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearSoundGroup extends Object
	native(Sound);

/** Versioning for handling enum changes. */
var const int ClassVersion;
var const int ObjVersion;

/** Vocal "efforts".  Typically nonverbal, more like grunts and stuff.  For verbal stuff, see the GUDS system. */
enum GearVoiceEffortID
{
	GearEffort_None,

	GearEffort_PainSmall,
	GearEffort_PainMedium,
	GearEffort_PainLarge,
	GearEffort_PainHuge,
	GearEffort_BoloReleaseEffort,
	GearEffort_DeathBreathe,
	GearEffort_NearDeathBreathe,
	GearEffort_RoadieRunBreathe,
	GearEffort_ChainSawAttackEffort,
	GearEffort_CoverSlamEffort,
	GearEffort_CrouchEffort,
	GearEffort_DeathScream,
	GearEffort_SuddenDeathScream,
	GearEffort_DoorPullEffort,
	GearEffort_ShortFallEffort,
	GearEffort_KickEffort,
	GearEffort_LandLightEffort,
	GearEffort_MantleEffort,
	GearEffort_MeleeAttackLargeEffort,
	GearEffort_MeleeAttackMediumEffort,
	GearEffort_MeleeAttackSmallEffort,
	GearEffort_EvadeEffort,
	GearEffort_EvadeImpactEffort,

	GearEffort_LiftHeavyWeaponEffort,
	GearEffort_InSmokeCough,
	GearEffort_ImpactCough,
	GearEffort_InInkCough,
	GearEffort_OnFirePain,
	GearEffort_OnFireDeath,

	GearEffort_ChainsawDuel,

	GearEffort_BoltokLongExecutionEffort,
	GearEffort_TorquebowLongExecutionEffort,
	GearEffort_PunchFaceLongExecutionEffort,
	GearEffort_CurbStompLongExecutionEffort,
	GearEffort_SniperLongExecutionEffort,

	GearEffort_ShotgunQuickExecutionEffort,
	GearEffort_SniperQuickExecutionEffort,
	GearEffort_BoomshotQuickExecutionEffort,
	GearEffort_TorquebowQuickExecutionEffort,

	GearEffort_MeatbagExecutionEffort,
	GearEffort_GrapplingHookMantleOverEffort,
	GearEffort_FellOffGrapplingHook,
};

/** These are straight sound effects, entirely nonvocal. */
enum GearFoleyID
{
	GearFoley_None,
	GearFoley_EvadeImpactFX,
	GearFoley_BodyMovement_RunningFX,
	GearFoley_BodyMovement_RoadieRunFX,
	GearFoley_BodyTurnFX,
	GearFoley_EnterCoverLowFX,
	GearFoley_ExitCoverLowFX,
	GearFoley_EnterCoverHighFX,
	GearFoley_ExitCoverHighFX,
	GearFoley_ToCrouchFX,
	GearFoley_FromCrouchFX,
	GearFoley_BodyWallImpactFX,
	GearFoley_MeleeAttackHitFleshFX,
	GearFoley_MeleeAttackHitArmorFX,
	GearFoley_MeleeAttackFX,
	GearFoley_FastHeartbeat,
	GearFoley_SlowHeartbeat,
	GearFoley_RevivingFlatliningSound,

	GearFoley_Body_CoverSlip,
	GearFoley_Body_Mantle,
	GearFoley_Body_ClimbUp,
	GearFoley_Body_SwatTurn,

	GearFoley_Body_Popup,		// popping up from cover
	GearFoley_Body_Lean,		// leaning out of cover (might be same as popup)
	GearFoley_Body_Aim,			// targeting/raising weapon to shoulder
};

/** 
 * Allows for search hierarchies for audio, for easier maintenance.
 */
var() GearSoundGroup		Parent;

struct native GearFoleyEntry
{
	var() editconst GearFoleyID		ID;

	/** Sound to play in this slot. Will be chosen randomly. */
	var() array<SoundCue>	Sounds;
};
var() array<GearFoleyEntry> FoleySoundFX;


struct native GearVoiceEffortEntry
{
	var() editconst GearVoiceEffortID	ID;

	/** Sound to play in this slot. Will be chosen randomly. */
	var() array<SoundCue>	Sounds;
};
var() array<GearVoiceEffortEntry> VoiceEfforts;

/** last played cue idx, per-effort.  parallel array to VoiceEfforts.  Sized properly in PostLoad. */
var protected transient array<int> LastEffortIdx;



cpptext
{
protected:
	void ValidateData();

public:
	virtual void PostLoad();
	virtual void PostEditChange( class FEditPropertyChain& PropertyThatChanged );
}



/** Internal. Returns a cue (randomly from available cues) */
native function protected SoundCue FindEffortCue(GearVoiceEffortID EffortID);

/** Internal. Returns a cue (randomly from available cues) */
native function protected SoundCue FindFoleyCue(GearFoleyID FoleyID) const;


/** Play specified effort on specified pawn. */
function PlayEffort(GearPawn GP, GearVoiceEffortID ID, optional bool bClientSide, optional bool bDeathEffort)
{
	local ESpeechPriority Priority;
	local SoundCue Cue;

	//`log("Attempting to play effort"@ID@VoiceEfforts[ID].Sounds.length);

	if ( (GP.Health > 0) || GP.IsDBNO() || bDeathEffort )
	{
		Cue = FindEffortCue(ID);
		if (Cue != None)
		{
			Priority = bDeathEffort ? Speech_Immediate : Speech_Effort;
			//`log("   ... acutally playing"@Cue);
			GP.SpeakLine(None, Cue, "", 0.f, Priority, SIC_IfHigher,,,,, bClientSide);
		}
	}
}

/** Doesn't use normal Speakline pipeline, use very selectively (e.g. breathing sounds). */
function AudioComponent PlayEffortEx(GearPawn GP, GearVoiceEffortID ID, optional float FadeInTimeSec = 0.3f)
{
	local SoundCue Cue;
	local AudioComponent AC;

	if ( (GP.Health > 0) || GP.IsDBNO() )
	{
		Cue = FindEffortCue(ID);
		if (Cue != None)
		{
			AC = GP.CreateAudioComponent( Cue, FALSE, TRUE );
			if (AC != None)
			{
				AC.bUseOwnerLocation = TRUE;
				AC.bAutoDestroy = TRUE;
				GP.AttachComponent( AC );
				AC.FadeIn(FadeInTimeSec, 1.f);
			}
		}
	}

	return AC;
}

/** Plays specified foley on specified pawn. */
function PlayFoleySound(GearPawn GP, GearFoleyID ID, optional bool bReplicate)
{
	local SoundCue Cue;

	Cue = FindFoleyCue(ID);
	if (Cue != None)
	{
		GP.PlaySound(Cue, !bReplicate);
	}
}


function AudioComponent PlayFoleySoundEx(GearPawn GP, GearFoleyID ID, optional float FadeInTimeSec = 0.3f)
{
	local SoundCue Cue;
	local AudioComponent AC;

	Cue = FindFoleyCue(ID);
	if (Cue != None)
	{
		AC = GP.CreateAudioComponent( Cue, FALSE, TRUE );
		if (AC != None)
		{
			AC.bUseOwnerLocation = TRUE;
			AC.bAutoDestroy = TRUE;
			AC.bStopWhenOwnerDestroyed = TRUE;
			GP.AttachComponent( AC );
			AC.FadeIn(FadeInTimeSec, 1.f);
		}
	}

	return AC;
}


/** Pawn taking damage sound */
function PlayTakeHit( Pawn P, int DamageTakenAmount, float PercentOfTotalHealthRemaining )
{
//	local SoundCue SC;
	local GearVoiceEffortID	SoundID;
	local float			DamageTakenPct;
	local GearPawn		WP;
	local float			Rnd;

	WP = GearPawn(P);

	if( WP != None )
	{
		DamageTakenPct = DamageTakenAmount / float(WP.DefaultHealth);
	}
	else
	{
		DamageTakenPct = DamageTakenAmount / float(P.HealthMax);
	}

	Rnd = FRand();

	// check for a huge hit.  we don't want to play this every time
	// so it has more impact
	// maybe we should have a min-time-between these as well?
	if( (DamageTakenPct > 0.4f) && (Rnd < 0.3f) )
	{
		SoundID = GearEffort_PainHuge;
	}
	else if( PercentOfTotalHealthRemaining > 0.70 )
	{
		SoundID = GearEffort_PainSmall;
	}
	else if( PercentOfTotalHealthRemaining > 0.35 )
	{
		SoundID = GearEffort_PainMedium;
	}
	else
	{
		SoundID = GearEffort_PainLarge;
	}

	PlayEffort(WP, SoundID, true);
}

simulated native function DumpMemoryUsage(optional bool bDetailed);

defaultproperties
{
	ClassVersion=3

	FoleySoundFX(GearFoley_None)=(ID=GearFoley_None)
	FoleySoundFX(GearFoley_EvadeImpactFX)=(ID=GearFoley_EvadeImpactFX)
	FoleySoundFX(GearFoley_BodyMovement_RunningFX)=(ID=GearFoley_BodyMovement_RunningFX)
	FoleySoundFX(GearFoley_BodyMovement_RoadieRunFX)=(ID=GearFoley_BodyMovement_RoadieRunFX)
	FoleySoundFX(GearFoley_BodyTurnFX)=(ID=GearFoley_BodyTurnFX)
	FoleySoundFX(GearFoley_EnterCoverLowFX)=(ID=GearFoley_EnterCoverLowFX)
	FoleySoundFX(GearFoley_ExitCoverLowFX)=(ID=GearFoley_ExitCoverLowFX)
	FoleySoundFX(GearFoley_EnterCoverHighFX)=(ID=GearFoley_EnterCoverHighFX)
	FoleySoundFX(GearFoley_ExitCoverHighFX)=(ID=GearFoley_ExitCoverHighFX)
	FoleySoundFX(GearFoley_ToCrouchFX)=(ID=GearFoley_ToCrouchFX)
	FoleySoundFX(GearFoley_FromCrouchFX)=(ID=GearFoley_FromCrouchFX)
	FoleySoundFX(GearFoley_BodyWallImpactFX)=(ID=GearFoley_BodyWallImpactFX)
	FoleySoundFX(GearFoley_MeleeAttackHitFleshFX)=(ID=GearFoley_MeleeAttackHitFleshFX)
	FoleySoundFX(GearFoley_MeleeAttackHitArmorFX)=(ID=GearFoley_MeleeAttackHitArmorFX)
	FoleySoundFX(GearFoley_MeleeAttackFX)=(ID=GearFoley_MeleeAttackFX)
	FoleySoundFX(GearFoley_FastHeartbeat)=(ID=GearFoley_FastHeartbeat)
	FoleySoundFX(GearFoley_SlowHeartbeat)=(ID=GearFoley_SlowHeartbeat)
	FoleySoundFX(GearFoley_RevivingFlatliningSound)=(ID=GearFoley_RevivingFlatliningSound)

	VoiceEfforts(GearEffort_None)=(ID=GearEffort_None)
	VoiceEfforts(GearEffort_PainSmall)=(ID=GearEffort_PainSmall)
	VoiceEfforts(GearEffort_PainMedium)=(ID=GearEffort_PainMedium)
	VoiceEfforts(GearEffort_PainLarge)=(ID=GearEffort_PainLarge)
	VoiceEfforts(GearEffort_PainHuge)=(ID=GearEffort_PainHuge)
	VoiceEfforts(GearEffort_BoloReleaseEffort)=(ID=GearEffort_BoloReleaseEffort)
	VoiceEfforts(GearEffort_DeathBreathe)=(ID=GearEffort_DeathBreathe)
	VoiceEfforts(GearEffort_NearDeathBreathe)=(ID=GearEffort_NearDeathBreathe)
	VoiceEfforts(GearEffort_RoadieRunBreathe)=(ID=GearEffort_RoadieRunBreathe)
	VoiceEfforts(GearEffort_ChainSawAttackEffort)=(ID=GearEffort_ChainSawAttackEffort)
	VoiceEfforts(GearEffort_CoverSlamEffort)=(ID=GearEffort_CoverSlamEffort)
	VoiceEfforts(GearEffort_CrouchEffort)=(ID=GearEffort_CrouchEffort)
	VoiceEfforts(GearEffort_DeathScream)=(ID=GearEffort_DeathScream)
	VoiceEfforts(GearEffort_SuddenDeathScream)=(ID=GearEffort_SuddenDeathScream)
	VoiceEfforts(GearEffort_DoorPullEffort)=(ID=GearEffort_DoorPullEffort)
	VoiceEfforts(GearEffort_ShortFallEffort)=(ID=GearEffort_ShortFallEffort)
	VoiceEfforts(GearEffort_KickEffort)=(ID=GearEffort_KickEffort)
	VoiceEfforts(GearEffort_LandLightEffort)=(ID=GearEffort_LandLightEffort)
	VoiceEfforts(GearEffort_MantleEffort)=(ID=GearEffort_MantleEffort)
	VoiceEfforts(GearEffort_MeleeAttackLargeEffort)=(ID=GearEffort_MeleeAttackLargeEffort)
	VoiceEfforts(GearEffort_MeleeAttackMediumEffort)=(ID=GearEffort_MeleeAttackMediumEffort)
	VoiceEfforts(GearEffort_MeleeAttackSmallEffort)=(ID=GearEffort_MeleeAttackSmallEffort)
	VoiceEfforts(GearEffort_EvadeEffort)=(ID=GearEffort_EvadeEffort)
	VoiceEfforts(GearEffort_EvadeImpactEffort)=(ID=GearEffort_EvadeImpactEffort)
	VoiceEfforts(GearEffort_LiftHeavyWeaponEffort)=(ID=GearEffort_LiftHeavyWeaponEffort)
	VoiceEfforts(GearEffort_InSmokeCough)=(ID=GearEffort_InSmokeCough)
	VoiceEfforts(GearEffort_ImpactCough)=(ID=GearEffort_ImpactCough)
	VoiceEfforts(GearEffort_InInkCough)=(ID=GearEffort_InInkCough)
	VoiceEfforts(GearEffort_OnFirePain)=(ID=GearEffort_OnFirePain)
	VoiceEfforts(GearEffort_OnFireDeath)=(ID=GearEffort_OnFireDeath)

	VoiceEfforts(GearEffort_ChainsawDuel)=(ID=GearEffort_ChainsawDuel)

	VoiceEfforts(GearEffort_BoltokLongExecutionEffort)=(ID=GearEffort_BoltokLongExecutionEffort)
	VoiceEfforts(GearEffort_TorquebowLongExecutionEffort)=(ID=GearEffort_TorquebowLongExecutionEffort)
	VoiceEfforts(GearEffort_PunchFaceLongExecutionEffort)=(ID=GearEffort_PunchFaceLongExecutionEffort)
	VoiceEfforts(GearEffort_CurbStompLongExecutionEffort)=(ID=GearEffort_CurbStompLongExecutionEffort)
	VoiceEfforts(GearEffort_SniperLongExecutionEffort)=(ID=GearEffort_SniperLongExecutionEffort)

	VoiceEfforts(GearEffort_ShotgunQuickExecutionEffort)=(ID=GearEffort_ShotgunQuickExecutionEffort)
	VoiceEfforts(GearEffort_SniperQuickExecutionEffort)=(ID=GearEffort_SniperQuickExecutionEffort)
	VoiceEfforts(GearEffort_BoomshotQuickExecutionEffort)=(ID=GearEffort_BoomshotQuickExecutionEffort)
	VoiceEfforts(GearEffort_TorquebowQuickExecutionEffort)=(ID=GearEffort_TorquebowQuickExecutionEffort)

	VoiceEfforts(GearEffort_MeatbagExecutionEffort)=(ID=GearEffort_MeatbagExecutionEffort)
	VoiceEfforts(GearEffort_GrapplingHookMantleOverEffort)=(ID=GearEffort_GrapplingHookMantleOverEffort)
}
