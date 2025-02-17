//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "GearGame.h"
#include "UnNet.h"

#include "GearGameSequenceClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameWeaponClasses.h"


static inline UBOOL NEQ(const FCovPosInfo& A, const FCovPosInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return (A.Link != B.Link) || (A.LtSlotIdx != B.LtSlotIdx) || (A.RtSlotIdx != B.RtSlotIdx) || (A.LtToRtPct != B.LtToRtPct);
}

static inline UBOOL NEQ(const FTakeHitInfo& A, const FTakeHitInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return ( A.HitLocation != B.HitLocation || A.Momentum != B.Momentum || A.DamageType != B.DamageType || A.InstigatedBy != B.InstigatedBy ||
			A.HitBoneIndex != B.HitBoneIndex || A.PhysicalMaterial != B.PhysicalMaterial || A.Damage != B.Damage );
}

static inline UBOOL NEQ(const FSpeakLineParamStruct& A, const FSpeakLineParamStruct& B, UPackageMap* Map, UActorChannel* Channel)
{
	return (A.Addressee != B.Addressee) || (A.Audio != B.Audio) || (A.Priority != B.Priority) || (A.BroadcastFilter != B.BroadcastFilter) || (A.bNoHeadTrack != B.bNoHeadTrack) || (A.bSuppressSubtitle != B.bSuppressSubtitle);
}

static inline UBOOL NEQ(const FPhysicsImpulseInfo& A, const FPhysicsImpulseInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return (A.LinearVelocity != B.LinearVelocity || A.AngularVelocity != B.AngularVelocity);
}

static inline UBOOL NEQ(const FSMStruct& A, const FSMStruct& B, UPackageMap* Map, UActorChannel* Channel)
{
	return ( A.SpecialMove != B.SpecialMove || A.InteractionPawn != B.InteractionPawn || A.Flags != B.Flags );
}

static inline UBOOL NEQ(const FReplicatedWeatherData& A, const FReplicatedWeatherData& B, UPackageMap* Map, UActorChannel* Channel)
{
	return ( (A.WeatherType != B.WeatherType) || (A.bOverrideEmitterHeight != B.bOverrideEmitterHeight) || (A.EmitterHeight != B.EmitterHeight) );
}

static inline UBOOL NEQ(const FIdleBreakReplicationInfo& A, const FIdleBreakReplicationInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return ( A.Count != B.Count );
}

static inline UBOOL NEQ(const FWeaponRepInfo& A, const FWeaponRepInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return ( A.WeaponClass != B.WeaponClass );
}

static inline UBOOL NEQ(const FReplicatedRootPosInfo& A, const FReplicatedRootPosInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return ( (A.Position != B.Position) || (A.bNewData != B.bNewData) );
}


INT* AGearPawn::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		if (!bNetOwner || bDemoRecording)
		{
			if (!bTearOff)
			{
				DOREP(GearPawn,RemoteWeaponRepInfo);
				DOREP(GearPawn,AttachSlotClass_Holster);
				DOREP(GearPawn,AttachSlotClass_Belt);
				DOREP(GearPawn,AttachSlotClass_LeftShoulder);
				DOREP(GearPawn,AttachSlotClass_RightShoulder);
			}
			DOREP(GearPawn,CoverType);
			DOREP(GearPawn,CoverAction);
			DOREP(GearPawn,AcquiredCoverInfo);
			DOREP(GearPawn,bWantsToBeMirrored);
			DOREP(GearPawn,bIsTargeting);
			DOREP(GearPawn,bIsInStationaryCover);
			DOREP(GearPawn,ReplicatedAimOffsetPct);
			DOREP(GearPawn,ReplicatedSpecialMoveStruct);
			DOREP(GearPawn,bIsZoomed);
			DOREP(GearPawn,CrawlSpeedModifier);
			DOREP(GearPawn,bRaiseHandWhileCrawling);
			DOREP(GearPawn,CurrentSlotDirection);
		}
		if (Physics == PHYS_RigidBody && !bTearOff)
		{
			DOREP(GearPawn,ReplicationRootBodyPos);
		}
		DOREP(GearPawn,bIsBleedingOut);
		DOREP(GearPawn,AttachClass_Shield);
		DOREP(GearPawn,bChargingBow);
		DOREP(GearPawn,ReplicatedForcedAimTime);
		DOREP(GearPawn,bSpawnABloodPool);
		DOREP(GearPawn,SpecialMoveLocation);
		DOREP(GearPawn,bActiveReloadBonusActive);
		DOREP(GearPawn,bWantsToMelee);
		if (bNetOwner)
		{
			DOREP(GearPawn,bCanPickupFactoryWeapons);
		}
		if (WorldInfo->TimeSeconds < LastTakeHitTimeout)
		{
			DOREP(GearPawn,LastTakeHitInfo);
		}
		DOREP(GearPawn,bIsInCombat);

		DOREP(GearPawn,KnockdownImpulse);
		DOREP(GearPawn,bWantToConverse);
		DOREP(GearPawn,bWantToUseCommLink);
		DOREP(GearPawn,ReplicatedSpeakLineParams);

		DOREP(GearPawn,bCannotRoadieRun);
		DOREP(GearPawn,bCoveringHead);
		DOREP(GearPawn,EngageTrigger);
		DOREP(GearPawn,LadderTrigger);

		if ( bNetInitial )
		{
			DOREP(GearPawn,DefaultGroundSpeed);
			DOREP(GearPawn,bDisableShadows);
		}

		if ( bTearOff )
		{
			DOREP(GearPawn,DurationBeforeDestroyingDueToNotBeingSeen);
			DOREP(GearPawn,bEnableEncroachCheckOnRagdoll);
			DOREP(GearPawn,KilledByPawn);
		}

		DOREP(GearPawn,MortarElevationPct);
		DOREP(GearPawn,ReplicatedOnFireState);
		DOREP(GearPawn,MutatedClass);

		DOREP(GearPawn,CarriedCrate);

		DOREP(GearPawn,ShotAtCount);
		DOREP(GearPawn,CringeCount);

		DOREP(GearPawn,CurrentGrapplingMarker);

		DOREP(GearPawn,DesiredKantusFadeVal);
		DOREP(GearPawn,KantusReviver);
		DOREP(GearPawn,bLastHitWasHeadShot);

		DOREPARRAY(GearPawn,KismetAnimSets);
		DOREPARRAY(GearPawn,ReplicatedFaceFXAnimSets);
		DOREP(GearPawn,ReplicatedMaterial);
		DOREP(GearPawn,BloodOpacity);
		DOREP(GearPawn,bScriptedWalking);

		DOREP(GearPawn,DefaultHealth);
		DOREP(GearPawn,ClampedBase);

		if( SpecialMove == SM_Hostage )
		{
			DOREP(GearPawn,HostageHealth);
		}

		if( bInDuelingMiniGame )
		{
			DOREP(GearPawn,DuelingMiniGameButtonPresses);
		}
	}

	return Ptr;
}

INT* AGearPawn_Infantry::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		DOREP(GearPawn_Infantry,RemoveHelmet);
		DOREP(GearPawn_Infantry,RemoveHelmet2);
		DOREP(GearPawn_Infantry,bHasHelmetOn);
		DOREP(GearPawn_Infantry,RemoveShoulderPadLeft);
		DOREP(GearPawn_Infantry,RemoveShoulderPadRight);
		DOREP(GearPawn_Infantry,ReplicatedIdleBreakInfo);
		DOREP(GearPawn_Infantry,bDisableIdleBreaks);
	
		if ( bNetInitial )
		{
			DOREP(GearPawn_Infantry,HelmetTypeReplicated);
			DOREP(GearPawn_Infantry,ShoulderPadLeftTypeReplicated);
			DOREP(GearPawn_Infantry,ShoulderPadRightTypeReplicated);
		}

	}
	return Ptr;
}

INT* AGearPC::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		if ( bNetOwner )
		{
			DOREP(GearPC,InteractEvent);
			DOREP(GearPC,ServerDictatedCover);
			DOREP(GearPC,bWaitingToRespawn);
			DOREPARRAY(GearPC,UnfriendlyActorList);
		}
	}
	return Ptr;
}

INT* AGearPRI::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		DOREP(GearPRI,PlayerStatus);
		DOREP(GearPRI,EndOfRoundPlayerStatus);
		DOREP(GearPRI,bIsDead);
		DOREP(GearPRI,bIsLeader);
		DOREP(GearPRI,PawnClass);
		DOREP(GearPRI,InitialWeaponType);
		DOREP(GearPRI,SelectedCharacterProfileId);
		DOREP(GearPRI,Difficulty);
		DOREP(GearPRI,bReceivesTeamScoringBonus);
		DOREP(GearPRI,LastToDBNOMePRI);
		DOREP(GearPRI,LastToKillMePRI);
		DOREP(GearPRI,LastIKilledPRI);
		DOREP(GearPRI,DamageTypeToKillMe);

		DOREP(GearPRI,Score_Kills);
		DOREP(GearPRI,Score_Takedowns);
		DOREP(GearPRI,Score_Revives);
		DOREP(GearPRI,Score_GameSpecific1);
		DOREP(GearPRI,Score_GameSpecific2);
		DOREP(GearPRI,Score_GameSpecific3);

		DOREP(GearPRI,KOTHRingPointsToBeginRound);
		DOREP(GearPRI,DLCFlag);

		DOREP(GearPRI,SquadName);
		DOREP(GearPRI,bForceShowInTaccom);
		DOREP(GearPRI,bIsMeatFlag);
	}
	return Ptr;
}

INT* AGearWeapon::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		if ( bNetOwner )
		{
			DOREP(GearWeapon,AmmoUsedCount);
			DOREP(GearWeapon,SpareAmmoCount);
			DOREP(GearWeapon,CharacterSlot);
			DOREP(GearWeapon,bInfiniteSpareAmmo);
			DOREP(GearWeapon,ActiveReload_NumBonusShots);
			if ( bNetInitial )
			{
				DOREP(GearWeapon,InitialMagazines);
				DOREP(GearWeapon,WeaponMagSize);
				DOREP(GearWeapon,WeaponMaxSpareAmmo);
			}
		}
		DOREP(GearWeapon,DebugShotStartLoc);
		DOREP(GearWeapon,DebugShotAimRot);
	}
	return Ptr;
}

INT* AGearGRI::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		if ( bNetInitial )
		{
			DOREP(GearGRI,AutoStartDelay);
			DOREP(GearGRI,RoundDuration);
			DOREP(GearGRI,EndOfRoundDelay);
			DOREP(GearGRI,EndOfGameDelay);
			DOREP(GearGRI,InitialRevivalTime);
			DOREP(GearGRI,bGameIsExecutionRules);
			DOREP(GearGRI,bAnnexIsKOTHRules);
			DOREP(GearGRI,InvasionNumTotalWaves);
			DOREP(GearGRI,EnemyDifficulty);
			DOREP(GearGRI,AnnexResourceGoal);
		}
		DOREP(GearGRI,bOverTime);
		DOREP(GearGRI,HODBeamManager);
		DOREP(GearGRI,GameStatus);
		DOREP(GearGRI,RoundCount);
		DOREP(GearGRI,RoundTime);
		DOREP(GearGRI,bIsCoop);
		DOREP(GearGRI,bHasEnoughPlayers);
		DOREP(GearGRI,FinalKiller);
		DOREP(GearGRI,FinalVictim);
		DOREP(GearGRI,EndTimeForCountdown);
		DOREPARRAY(GearGRI,LastStandingOnTeamName);
		DOREPARRAY(GearGRI,TeamIndexLossOrder);
		DOREP(GearGRI,bFinalFriendlyKill);
		DOREP(GearGRI,bAllowFriendlyFire);
		DOREP(GearGRI,InvasionCurrentWaveIndex);
		DOREP(GearGRI,EnemiesLeftThisRound);
		DOREP(GearGRI,TotalNumTeamRespawns);
		DOREPARRAY(GearGRI,NumTeamRespawns);
		DOREPARRAY(GearGRI,GameScore);
		DOREP(GearGRI,CPControlTeam);
		DOREP(GearGRI,CommandPoint);
		DOREP(GearGRI,CPControlPct);
		DOREP(GearGRI,CPResourceLeft);
		DOREP(GearGRI,RespawnTime);
		DOREP(GearGRI,bInfiniteRoundDuration);
		DOREP(GearGRI,RoundEndTime);
		DOREP(GearGRI,NumSecondsUntilNextRound);
		DOREP(GearGRI,MeatflagKidnapper);
		DOREP(GearGRI,MeatflagPawn);
		DOREP(GearGRI,ReplicatedWeather);
		DOREP(GearGRI,KillerOfCOGLeaderPRI);
		DOREP(GearGRI,KillerOfLocustLeaderPRI);
		DOREP(GearGRI,DefaultSpectatingState);
		DOREP(GearGRI,ExtendedRestartCount);
		DOREP(GearGRI,PlaylistId);
		DOREPARRAY(GearGRI,WingmanClassIndexes);
		DOREP(GearGRI,WavePointsAlivePct);
		DOREP(GearGRI,StatsGameplaySessionID);
	}
	return Ptr;
}

INT* AGearPointOfInterest::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		DOREP(GearPointOfInterest,bEnabled);
		DOREP(GearPointOfInterest,AttachedToActor);
		DOREP(GearPointOfInterest,IconDuration);
		DOREP(GearPointOfInterest,DisplayName);
		DOREP(GearPointOfInterest,ForceLookType);
		DOREP(GearPointOfInterest,DesiredFOV);
		DOREP(GearPointOfInterest,FOVCount);
		DOREP(GearPointOfInterest,bDoTraceForFOV);
		DOREP(GearPointOfInterest,ForceLookDuration);
		DOREP(GearPointOfInterest,LookAtPriority);
		DOREP(GearPointOfInterest,bForceLookCheckLineOfSight);
	}
	return Ptr;
}

INT* AGearPointOfInterest_Meatflag::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		DOREP(GearPointOfInterest_Meatflag,KidnapperTeamIndex);
	}
	return Ptr;
}

/** Get variables to replicate. */
INT* AGearDroppedPickup::GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel)
{
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);

	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	DOREP(GearDroppedPickup,RBState);
	if( bNetDirty )
	{
		DOREP(GearDroppedPickup,DroppedEmissiveColorTeam);
		DOREP(GearDroppedPickup,ClaimedBy);
	}
	return Ptr;
}

/** Get variables to replicate. */
INT* AGearPickupFactory::GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel)
{
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);

	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	if( bNetDirty )
	{
		DOREP(GearPickupFactory,ClaimedBy);
		// need to replicate collision for HUD icons and such
		if (bOnlyReplicateHidden)
		{
			DOREP(Actor,bCollideActors);
		}
	}
	return Ptr;
}

INT* AGearPawn_CarryCrate_Base::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);

	// force full update every frame for crate
	Channel->LastFullUpdateTime = -1.f;

	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	if(bNetDirty)
	{
		DOREP(GearPawn_CarryCrate_Base, MarcusPawn);
		DOREP(GearPawn_CarryCrate_Base, DomPawn);
	}

	DOREP(GearPawn_CarryCrate_Base, YawVelocity);
	DOREP(GearPawn_CarryCrate_Base, RepVelocityX);
	DOREP(GearPawn_CarryCrate_Base, RepVelocityY);
	DOREP(GearPawn_CarryCrate_Base, RepVelocityZ);

	return Ptr;
}

static inline UBOOL NEQ(const FCraneSpringState& A, const FCraneSpringState& B, UPackageMap* Map, UActorChannel* Channel)
{
	return	((A.SpringPos != B.SpringPos) || (A.SpringVelX != B.SpringVelX) || (A.SpringVelY != B.SpringVelY) || (A.SpringVelZ != B.SpringVelZ) || (A.bNewData != B.bNewData));
}

INT* AVehicle_Crane_Base::GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel)
{
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);

	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	DOREP(Vehicle_Crane_Base, RaiseAngVel);
	DOREP(Vehicle_Crane_Base, CurrentRaiseAng);
	DOREP(Vehicle_Crane_Base, YawAngVel);
	DOREP(Vehicle_Crane_Base, CurrentYawAng);
	DOREP(Vehicle_Crane_Base, SpringState);
	DOREP(Vehicle_Crane_Base, bForceAbove);
	DOREP(Vehicle_Crane_Base, ForceAbovePitch);

	return Ptr;
}
