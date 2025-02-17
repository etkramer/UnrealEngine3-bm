//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"
#include "UTGameUIClasses.h"
#include "UnPath.h"
#include "EngineInterpolationClasses.h"
#include "UTGameVehicleClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"

IMPLEMENT_CLASS(AUTPlayerController);
IMPLEMENT_CLASS(AUTConsolePlayerController);
IMPLEMENT_CLASS(UUTPlayerInput);
IMPLEMENT_CLASS(UUTConsolePlayerInput);

/** 
 * HearSound()
 * If sound is audible, calls eventClientHearSound() so local or remote player will hear it.
 * UTPlayerController implementation determines audibility based on the closest to the sound location of 
 * the viewtarget or the controller's pawn if they are not the same
 */
UBOOL AUTPlayerController::HearSound( USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed )
{
	if( SoundPlayer == this )
	{
		// always play sounds by the playercontroller
		eventClientHearSound(InSoundCue, SoundPlayer, (SoundPlayer->Location == SoundLocation) ? FVector(0.f) : SoundLocation, bStopWhenOwnerDestroyed, FALSE);
		return TRUE;
	}
	else if (bDedicatedServerSpectator)
	{
		// dedicated server spectator ignores sounds not played on self
		return FALSE;
	}

	// Get simple listener/speaker pair
	FVector ModifiedSoundLocation = SoundLocation;
	FVector HearLocation = Location;

	// Redirect through the view target if necessary
	if( ViewTarget )
	{
		HearLocation = ViewTarget->Location;

		if( Pawn && ( Pawn != ViewTarget ) && ( ( HearLocation - SoundLocation ).SizeSquared() > ( Pawn->Location - SoundLocation ).SizeSquared() ) )
		{
			// move sound location so that it's heard as if Pawn heard it.
			ModifiedSoundLocation = ModifiedSoundLocation + HearLocation - Pawn->Location;
		}
	}

	INT bIsOccluded = 0;

	if (!bAcuteHearing || (ModifiedSoundLocation - HearLocation).IsNearlyZero())
	{
		if( InSoundCue->IsAudible( ModifiedSoundLocation, HearLocation, SoundPlayer, bIsOccluded, bCheckSoundOcclusion ) )
		{
			ValidateSoundPlayer(SoundPlayer);
			eventClientHearSound( InSoundCue, SoundPlayer, (SoundPlayer != NULL && SoundPlayer->Location == ModifiedSoundLocation) ? FVector(0.f) : ModifiedSoundLocation, bStopWhenOwnerDestroyed, bIsOccluded );
			return TRUE;
		}
	}
	else
	{
		// play sound at closer location to increase volume
		const FVector SoundDir = ModifiedSoundLocation - HearLocation;
		const FLOAT SoundDist = SoundDir.Size();
		ModifiedSoundLocation = HearLocation + ::Min( 0.25f * SoundDist, 2000.f ) * SoundDir.SafeNormal();
		if( InSoundCue->IsAudible( ModifiedSoundLocation, HearLocation, SoundPlayer, bIsOccluded, bCheckSoundOcclusion ) )
		{
			ValidateSoundPlayer(SoundPlayer);
			eventClientHearSound( InSoundCue, NULL, ModifiedSoundLocation, bStopWhenOwnerDestroyed, FALSE );
			return TRUE;
		}
	}

	return FALSE;
}
 
UBOOL AUTPlayerController::Tick(FLOAT DeltaSeconds, ELevelTick TickType)
{
	if ( Super::Tick(DeltaSeconds,TickType) )
	{
#if FORCELOWGORE
		if ( WorldInfo->NetMode == NM_Client )
		{
			((AGameInfo *)(AGameInfo::StaticClass()->GetDefaultActor()))->GoreLevel = 1;
		}
#endif	
		if ( bPulseTeamColor )
		{
			PulseTimer -= DeltaSeconds;
			if (PulseTimer < 0.f)
			{
				bPulseTeamColor = false;
			}
		}

		if( bUsePhysicsRotation )
		{
			physicsRotation(DeltaSeconds);
		}
		if ( bBeaconPulseDir )
		{
			BeaconPulseScale += BeaconPulseRate * DeltaSeconds;
			if ( BeaconPulseScale > BeaconPulseMax )
			{
				BeaconPulseScale = BeaconPulseMax;
				bBeaconPulseDir = FALSE;
			}
		}
		else
		{
			BeaconPulseScale -= BeaconPulseRate * DeltaSeconds;
			if ( BeaconPulseScale < 1.f )
			{
				BeaconPulseScale = 1.f;
				bBeaconPulseDir = TRUE;
			}
		}

		return 1;
	}
	return 0;
}

UBOOL AUTPlayerController::MoveWithInterpMoveTrack(UInterpTrackMove* MoveTrack, UInterpTrackInstMove* MoveInst, FLOAT CurTime, FLOAT DeltaTime)
{
	if (CameraAnimPlayer != NULL && CameraAnimPlayer->MoveTrack == MoveTrack)
	{
		// figure out the movement delta and put it in ShakeOffset and ShakeRot
		MoveTrack->GetKeyTransformAtTime(MoveInst, CurTime, ShakeOffset, ShakeRot);
		ShakeOffset *= CameraAnimPlayer->CurrentBlendWeight;
		ShakeRot *= CameraAnimPlayer->CurrentBlendWeight;
		return TRUE;
	}
	else
	{
		// use default behavior
		return Super::MoveWithInterpMoveTrack(MoveTrack, MoveInst, CurTime, DeltaTime);
	}
}

void AUTPlayerController::ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const
{
	Super::ModifyPostProcessSettings(PPSettings);

	if (CameraAnimPlayer != NULL && !CameraAnimPlayer->bFinished)
	{
		ACameraActor const* const DefaultCamActor = ACameraActor::StaticClass()->GetDefaultObject<ACameraActor>();
		if (DefaultCamActor) 
		{
			FLOAT const BlendWeight = CameraAnimPlayer->CurrentBlendWeight;
			FPostProcessSettings const& DefaultSettings = DefaultCamActor->CamOverridePostProcess;

			PPSettings.Bloom_Scale += BlendWeight * (CamOverridePostProcess.Bloom_Scale - DefaultSettings.Bloom_Scale);
			PPSettings.DOF_FalloffExponent += BlendWeight * (CamOverridePostProcess.DOF_FalloffExponent - DefaultSettings.DOF_FalloffExponent);
			PPSettings.DOF_BlurKernelSize += BlendWeight * (CamOverridePostProcess.DOF_BlurKernelSize - DefaultSettings.DOF_BlurKernelSize);
			PPSettings.DOF_MaxNearBlurAmount += BlendWeight * (CamOverridePostProcess.DOF_MaxNearBlurAmount - DefaultSettings.DOF_MaxNearBlurAmount);
			PPSettings.DOF_MaxFarBlurAmount += BlendWeight * (CamOverridePostProcess.DOF_MaxFarBlurAmount - DefaultSettings.DOF_MaxFarBlurAmount);
			PPSettings.DOF_FocusInnerRadius += BlendWeight * (CamOverridePostProcess.DOF_FocusInnerRadius - DefaultSettings.DOF_FocusInnerRadius);
			PPSettings.DOF_FocusDistance += BlendWeight * (CamOverridePostProcess.DOF_FocusDistance - DefaultSettings.DOF_FocusDistance);
			PPSettings.MotionBlur_MaxVelocity += BlendWeight * (CamOverridePostProcess.MotionBlur_MaxVelocity - DefaultSettings.MotionBlur_MaxVelocity);
			PPSettings.MotionBlur_Amount += BlendWeight * (CamOverridePostProcess.MotionBlur_Amount - DefaultSettings.MotionBlur_Amount);
			PPSettings.MotionBlur_CameraRotationThreshold += BlendWeight * (CamOverridePostProcess.MotionBlur_CameraRotationThreshold - DefaultSettings.MotionBlur_CameraRotationThreshold);
			PPSettings.MotionBlur_CameraTranslationThreshold += BlendWeight * (CamOverridePostProcess.MotionBlur_CameraTranslationThreshold - DefaultSettings.MotionBlur_CameraTranslationThreshold);
			PPSettings.Scene_Desaturation += BlendWeight * (CamOverridePostProcess.Scene_Desaturation - DefaultSettings.Scene_Desaturation);
			PPSettings.Scene_HighLights += BlendWeight * (CamOverridePostProcess.Scene_HighLights - DefaultSettings.Scene_HighLights);
			PPSettings.Scene_MidTones += BlendWeight * (CamOverridePostProcess.Scene_MidTones - DefaultSettings.Scene_MidTones);
			PPSettings.Scene_Shadows += BlendWeight * (CamOverridePostProcess.Scene_Shadows - DefaultSettings.Scene_Shadows);
		}
	}

	PPSettings.Bloom_Scale += PostProcessModifier.Bloom_Scale;
	PPSettings.DOF_FalloffExponent += PostProcessModifier.DOF_FalloffExponent;
	PPSettings.DOF_BlurKernelSize += PostProcessModifier.DOF_BlurKernelSize;
	PPSettings.DOF_MaxNearBlurAmount += PostProcessModifier.DOF_MaxNearBlurAmount;
	PPSettings.DOF_MaxFarBlurAmount += PostProcessModifier.DOF_MaxFarBlurAmount;
	PPSettings.DOF_FocusInnerRadius += PostProcessModifier.DOF_FocusInnerRadius;
	PPSettings.DOF_FocusDistance += PostProcessModifier.DOF_FocusDistance;
	PPSettings.MotionBlur_MaxVelocity += PostProcessModifier.MotionBlur_MaxVelocity;
	PPSettings.MotionBlur_Amount += PostProcessModifier.MotionBlur_Amount;
	PPSettings.MotionBlur_CameraRotationThreshold += PostProcessModifier.MotionBlur_CameraRotationThreshold;
	PPSettings.MotionBlur_CameraTranslationThreshold += PostProcessModifier.MotionBlur_CameraTranslationThreshold;
	PPSettings.Scene_Desaturation += PostProcessModifier.Scene_Desaturation;
	PPSettings.Scene_HighLights += PostProcessModifier.Scene_HighLights;
	PPSettings.Scene_MidTones += PostProcessModifier.Scene_MidTones;
	PPSettings.Scene_Shadows += PostProcessModifier.Scene_Shadows;
}

void AUTPlayerController::PreSave()
{
	if (IsTemplate())
	{
		// force PostProcessModifier to be zero in UTPlayerController defaults
		UProperty* Prop = FindField<UProperty>(GetClass(), FName(TEXT("PostProcessModifier")));
		checkSlow(Prop != NULL);
		Prop->ClearValue((BYTE*)this + Prop->Offset);
	}

	Super::PreSave();
}

/** @return The index of the PC in the game player's array. */
INT AUTPlayerController::GetUIPlayerIndex()
{
	ULocalPlayer* LP = NULL;
	INT Result = INDEX_NONE;

	LP = Cast<ULocalPlayer>(Player);

	if(LP)
	{	
		Result = UUIInteraction::GetPlayerIndex(LP);
	}

	return Result;
}

/** @return Returns whether or not we're in a epic internal build. */
UBOOL AUTPlayerController::IsEpicInternal()
{
	UBOOL bResult = FALSE;

#if EPIC_INTERNAL
	bResult = TRUE;
#endif

	return bResult;
}

/** Resets the profile to its default state. */
void AUTPlayerController::ResetProfileToDefault(class UOnlineProfileSettings* Profile)
{
	if(Profile != NULL)
	{
		Profile->eventSetToDefaults();
	}
}

/** Sets the display gamma. */
void AUTPlayerController::SetGamma(FLOAT GammaValue)
{
	extern void SetDisplayGamma(FLOAT Gamma);

	// map value between 1.0 - 3.0
	GammaValue = GammaValue*2.0f + 1.0f;
	SetDisplayGamma(GammaValue);
}

/** 
 * Sets whether or not hardware physics are enabled.
 *
 * @param bEnabled	Whether to enable the physics or not.
 */
void AUTPlayerController::SetHardwarePhysicsEnabled(UBOOL bEnabled)
{
	GEngine->bDisablePhysXHardwareSupport = (bEnabled==FALSE);
}

/** Determines whether this Pawn can be used for Adhesion or Friction **/
UBOOL AUTConsolePlayerController::IsValidTargetAdhesionFrictionTarget( const APawn* P, FLOAT MaxDistance ) const
{
	UBOOL Retval = FALSE;

	// check for self before we do any computation
	if( ( P != NULL ) && ( P != Pawn ) ) 
	{
		const FVector ToTargetNorm = P->Location - Pawn->Location;
		const FLOAT DistToTarget = ToTargetNorm.Size();

		//warnf( TEXT( "IsValidTargetAdhesionFrictionTarget: %s %i" ), *GP->GetName(), GP->PlayerReplicationInfo->eventGetTeamNum() );

		Retval = ( 
			 ( P->IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) == TRUE )
			&& ( P->bCanBeAdheredTo == TRUE )
			&& ( P->bCanBeFrictionedTo == TRUE )
			&& ( DistToTarget < MaxDistance )
			&& ( P->Health > 0 ) 
			&& ( P->bHidden == FALSE )
			&& ( P->bDeleteMe == FALSE )
			&& ( const_cast<AUTConsolePlayerController*>(this)->LineOfSightTo( P ) == TRUE )
			);
	}

	return Retval;
}

/** 
 * This will look to see if the player is aiming directly at a projectile which does not want to
 * be adhered to. 
 **/
UBOOL IsDirectlyAimingAtAProjectile( const AUTConsolePlayerController* C, const FRotator& CamRot )
{
	UBOOL RetVal = FALSE;

	AActor*	ViewTarg = const_cast<AUTConsolePlayerController*>(C)->GetViewTarget();

	if( C->Pawn != NULL )
	{
		const FVector StartPoint = ViewTarg->Location + FVector(0,0,C->Pawn->BaseEyeHeight);
		const FVector EndPoint = StartPoint + CamRot.Vector() * 3000.0f; // @todo get this from the weapon

		FCheckResult HitProjectile;
		GWorld->SingleLineCheck( HitProjectile, const_cast<AUTConsolePlayerController*>(C), EndPoint, StartPoint, TRACE_Actors );

		//const_cast<AUTConsolePlayerController*>(C)->FlushPersistentDebugLines();
		//const_cast<AUTConsolePlayerController*>(C)->DrawDebugLine( StartPoint, EndPoint, 0, 255, 0, 1 ); 

		if( (HitProjectile.Actor != NULL )
			&& ( HitProjectile.Actor->GetAProjectile() != NULL )
			&& ( (HitProjectile.Actor->bCanBeAdheredTo == FALSE ) || (HitProjectile.Actor->bCanBeFrictionedTo == FALSE ) )

			)
		{
			//warnf( TEXT("aiming at a non adhere-to and non friction-to Projectile!!!" ) );
			RetVal = TRUE;
		}

	}

	return RetVal;
}



/**
 * This will find the best AdhesionFriction target based on the params passed in.
 **/
APawn* AUTConsolePlayerController::GetTargetAdhesionFrictionTarget( FLOAT MaxDistance, const FVector& CamLoc, const FRotator& CamRot ) const
{
	APawn* BestTarget = NULL;
	
	// how to deal with other objects that are not GamePawns  (e.g. shock ball, magic objects)
	// a shock ball should "always?" be the best

	const UBOOL bDirectlyAimingAtAProjectile = IsDirectlyAimingAtAProjectile( this, CamRot );

	if( bDirectlyAimingAtAProjectile == TRUE )
	{
		return NULL;
	}

	
	const ULocalPlayer* LocalPlayer = ConstCast<ULocalPlayer>(Player); // do this cast only once
 
	if( LocalPlayer != NULL )
	{
		FLOAT BestFrictionTargetScore = 0.0f;

		for( APawn* TempP = GWorld->GetWorldInfo()->PawnList; TempP != NULL; TempP = TempP->NextPawn )
		{
			// check for visibility here as we need a LocalPlayer and don't want to just pass it in to then do the same check
			if( ( LocalPlayer->GetActorVisibility( TempP ) == TRUE )
				&& ( IsValidTargetAdhesionFrictionTarget( TempP, MaxDistance ) == TRUE )
				)
			{
				//warnf( TEXT("PossibleTarget: %s"), *GamePawn->GetName() );
				// score all potential targets and find best one.
				const FLOAT TmpScore = ScoreTargetAdhesionFrictionTarget( TempP, MaxDistance, CamLoc, CamRot );

				// track best visible target
				if( TmpScore > BestFrictionTargetScore )
				{
					BestFrictionTargetScore = TmpScore;
					BestTarget = TempP;
					//warnf( TEXT("BestAdhesionFrictionTarget: %s at %f"), *BestTarget->GetName(), BestFrictionTargetScore );
				}
			}
		}
	}

	return BestTarget;
}


/** 
 * This will score both Adhesion and Friction targets.  We want the same scoring function as we
 * don't want the two different systems fighting over targets that are close.
 **/
FLOAT AUTConsolePlayerController::ScoreTargetAdhesionFrictionTarget( const APawn* P, FLOAT MaxDistance, const FVector& CamLoc, const FRotator& CamRot ) const
{
	FLOAT Score = 0.0f;

	if( P != NULL && Pawn != NULL )
	{
		// Initial Score based on how much we're aiming at them
		FVector ToTargetNorm = P->Location - Pawn->Location;
		const FLOAT DistToTarget = ToTargetNorm.Size();
		ToTargetNorm /= DistToTarget;

		Score = ToTargetNorm | CamRot.Vector();

		// If they're in front and within adhesion/friction range
		if( (Score > 0.f) && (DistToTarget < MaxDistance) )
		{
			// Adjust Score based on distance, so that closer targets are slightly favored
			Score += (1.f - (DistToTarget/MaxDistance)) * Score * 0.65f;
		}
	}

	return Score;
}

void AUTPlayerController::UpdateHiddenActors(FVector ViewLocation)
{
	// hide orb if player camera clipping into it
	for( INT i=0; i< PotentiallyHiddenActors.Num(); i++ )
	{
		if ( (PotentiallyHiddenActors(i) == NULL) || PotentiallyHiddenActors(i)->bDeleteMe )
		{
			if ( PotentiallyHiddenActors(i) != NULL )
			{
				HiddenActors.RemoveItem(PotentiallyHiddenActors(i));
			}
			PotentiallyHiddenActors.Remove(i);
			i--;
		}
		else
		{
			// hide if too close
			if ( PotentiallyHiddenActors(i)->ShouldHideActor(ViewLocation) )
			{
				HiddenActors.AddUniqueItem(PotentiallyHiddenActors(i));
			}
			else
			{
				HiddenActors.RemoveItem(PotentiallyHiddenActors(i));
			}
		}
	}
}




UBOOL AUTConsolePlayerController::IsControllerTiltActive() const
{
	UBOOL Retval = FALSE;

	//ULocalPlayer* LP = Cast<ULocalPlayer>(Pawn->Controller);

	// const ULocalPlayer* LocalPlayer = ConstCast<ULocalPlayer>(Player); // do this cast only once
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::IsControllerTiltActive calling!" ) );
		Retval = LP->ViewportClient->Viewport->IsControllerTiltActive( LP->ControllerId );
	}

	return Retval;
}


void AUTConsolePlayerController::SetControllerTiltDesiredIfAvailable( UBOOL bActive )
{
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::SetControllerTiltDesiredIfAvailable calling!" ) );
		LP->ViewportClient->Viewport->SetControllerTiltDesiredIfAvailable( LP->ControllerId, bActive );
	}
}


void AUTConsolePlayerController::SetControllerTiltActive( UBOOL bActive )
{
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::SetControllerTiltActive calling!" ) );

		//warnf( TEXT( "0" ) );
		LP->ViewportClient->Viewport->SetControllerTiltActive( LP->ControllerId, bActive );
		//warnf( TEXT( "1" ) );
		//LP->ViewportClient->ViewportFrame->SetControllerTiltActive( bActive );
		//warnf( TEXT( "2" ) );
		//LP->ViewportClient->SetControllerTiltActive( bActive );
	}
}



void AUTConsolePlayerController::SetOnlyUseControllerTiltInput( UBOOL bActive )
{
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::SetOnlyUseControllerTiltInput calling!" ) );

		//warnf( TEXT( "0" ) );
		LP->ViewportClient->Viewport->SetOnlyUseControllerTiltInput( LP->ControllerId, bActive );
		//warnf( TEXT( "1" ) );
		//LP->ViewportClient->ViewportFrame->SetControllerTiltActive( bActive );
		//warnf( TEXT( "2" ) );
		//LP->ViewportClient->SetControllerTiltActive( bActive );
	}
}

void AUTConsolePlayerController::SetUseTiltForwardAndBack( UBOOL bActive )
{
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::SetOnlyUseControllerTiltInput calling!" ) );

		//warnf( TEXT( "0" ) );
		LP->ViewportClient->Viewport->SetUseTiltForwardAndBack( LP->ControllerId, bActive );
		//warnf( TEXT( "1" ) );
		//LP->ViewportClient->ViewportFrame->SetControllerTiltActive( bActive );
		//warnf( TEXT( "2" ) );
		//LP->ViewportClient->SetControllerTiltActive( bActive );
	}
}



/** @return whether or not this Controller has a keyboard available to be used **/
UBOOL AUTPlayerController::IsKeyboardAvailable() const
{
	UBOOL Retval = FALSE;

	//ULocalPlayer* LP = Cast<ULocalPlayer>(Pawn->Controller);

	// const ULocalPlayer* LocalPlayer = ConstCast<ULocalPlayer>(Player); // do this cast only once
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::IsControllerTiltActive calling!" ) );
		Retval = LP->ViewportClient->Viewport->IsKeyboardAvailable( LP->ControllerId );
	}

	return Retval;
}

/** @return whether or not this Controller has a mouse available to be used **/
UBOOL AUTPlayerController::IsMouseAvailable() const
{
	UBOOL Retval = FALSE;

	//ULocalPlayer* LP = Cast<ULocalPlayer>(Pawn->Controller);

	// const ULocalPlayer* LocalPlayer = ConstCast<ULocalPlayer>(Player); // do this cast only once
	ULocalPlayer* LP = Cast<ULocalPlayer>(Player);

	if( ( LP != NULL )
		&& ( LP->ViewportClient != NULL )
		&& ( LP->ViewportClient->Viewport != NULL )
		)
	{
		//warnf( TEXT( "AUTConsolePlayerController::IsControllerTiltActive calling!" ) );
		Retval = LP->ViewportClient->Viewport->IsMouseAvailable( LP->ControllerId );
	}

	return Retval;
}


/** Unlocks a specific character. */
void AUTPlayerController::UnlockChar(const FString& CharName)
{
	// Get custom char database
	UUTCharInfo* Data = CastChecked<UUTCharInfo>(UUTCharInfo::StaticClass()->GetDefaultObject());

	if( OnlinePlayerData && OnlinePlayerData->ProfileProvider )
	{
		UUTProfileSettings* Profile = Cast<UUTProfileSettings>(OnlinePlayerData->ProfileProvider->Profile);
		if(Profile)
		{
			INT UnlockInfo = 0;
			Profile->GetProfileSettingValueInt(UCONST_UTPID_UnlockedCharacters, UnlockInfo);

			if( !Data->CharIsUnlocked(CharName, UnlockInfo) )
			{
				UnlockInfo = Data->SetCharLockedStatus(CharName, FALSE, UnlockInfo);
				Profile->SetProfileSettingValueInt(UCONST_UTPID_UnlockedCharacters, UnlockInfo);

				debugf(TEXT("Char '%s' Unlocked!"), *CharName);

				eventOnCharacterUnlocked();
			}
			else
			{
				debugf(TEXT("Char '%s' already unlocked."), *CharName);
			}
		}
	}
}

/** Cheat that unlocks all possible characters. */
void UUTCheatManager::UnlockAllChars()
{
	// Get custom char database
	UUTCharInfo* Data = CastChecked<UUTCharInfo>(UUTCharInfo::StaticClass()->GetDefaultObject());

	AUTPlayerController* UTPC = Cast<AUTPlayerController>(GetOuter());
	if(UTPC)
	{
		// Iterate over all possible unlockable chars
		for(INT i=0; i<Data->UnlockableChars.Num(); i++)
		{
			UTPC->UnlockChar( Data->UnlockableChars(i) );
		}
	}
}

/** Will return the BindName based on the BindCommand */
FString UUTPlayerInput::GetUTBindNameFromCommand(const FString& BindCommand)
{
	FString NameSearch = TEXT("");
	FString CommandToFind = BindCommand;
	UBOOL bGamepad = bUsingGamepad;

	// Get the bind command using the Mapped FieldName as the key
	if ( CommandToFind.Len() > 0 )
	{
		// We have a potential 2nd pass check in case we are looking for a gamepad binding
		// and we didn't find one.  The reason is that we may have appeneded "_Gamepad" to the end for
		// the special cases where a bind was modified to work special on the gamepad.
		INT NumAttempts = bGamepad ? 2 : 1;
		for ( INT AttemptIndex = 0; AttemptIndex < NumAttempts; AttemptIndex++ )
		{
			INT BindIndex = -1;

			// If this is the 2nd attempt it is because we were looking for a gamepad bind and
			// could not find one, so append "_Gamepad" and see if this was an altered command.
			if ( AttemptIndex > 0 )
			{
				CommandToFind += TEXT("_Gamepad");
			}

			// Loop for bind names until we get a match that is dependent on whether a controller or keyboard/mouse are used.
			do 
			{
				// Get bind name
				NameSearch = GetBindNameFromCommand( CommandToFind, &BindIndex );
				// See if it starts with the controller prefix.
				if ( NameSearch.StartsWith(TEXT("XboxTypeS")) )
				{
					// Is a controller prefix so if we are using the gamepad break and return this bind name.
					if ( bGamepad )
					{
						break;
					}
				}
				else
				{
					// Is not a controller prefix so if we are not using the gamepad break and return this bind name.
					if ( !bGamepad )
					{
						break;
					}
				}

				// Decrement the index.
				BindIndex--;

			} while( BindIndex >= 0 );

			// If we found a match break out.
			if ( NameSearch.Len() > 0 )
			{
				break;
			}
		}
	}

	return NameSearch;
}

void UUTPlayerInput::ForceFlushOfInput()
{
	FlushPressedKeys();
}

/*
void AUTPlayerController::GiveAllDamageTypes()
{
	debugf(TEXT("Testing all damagetypes"));
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		if (It->IsChildOf(UDamageType::StaticClass()))
		{
			UDamageType* DmgObj = It->GetDefaultObject<UDamageType>();
			eventPrintDamageStats(DmgObj);
		}
	}
}
*/


UBOOL UUTPlayerInput::InputKey(INT ControllerId, FName Key, enum EInputEvent Event, FLOAT AmountDepressed, UBOOL bGamePad)
{
#if CONSOLE
	if (!bGamePad)
	{
		AUTGameReplicationInfo* GRI = Cast<AUTGameReplicationInfo>(((AActor*)GetOuter())->WorldInfo->GRI);
		if (GRI != NULL && !GRI->bAllowKeyboardAndMouse)
		{
			return FALSE;
		}
	}
#endif

	return Super::InputKey(ControllerId, Key, Event, AmountDepressed, bGamePad);
}
