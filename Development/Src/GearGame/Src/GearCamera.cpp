/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"

#include "GearGameCameraClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameSequenceClasses.h"

IMPLEMENT_CLASS(AGearPlayerCamera)

IMPLEMENT_CLASS(UGearCameraModifier)
IMPLEMENT_CLASS(UGearCamMod_CameraBone)
IMPLEMENT_CLASS(UGearCamMod_ScreenShake)

IMPLEMENT_CLASS(UGearCameraBase)
IMPLEMENT_CLASS(UGearSpectatorCamera)
IMPLEMENT_CLASS(UGearGameplayCamera)
IMPLEMENT_CLASS(UGearGameplayCameraMode)
IMPLEMENT_CLASS(UGameplayCam_Conversation)
IMPLEMENT_CLASS(UGameplayCam_Cover)
IMPLEMENT_CLASS(UGameplayCam_CoverTargeting)
IMPLEMENT_CLASS(UGameplayCam_Default)
IMPLEMENT_CLASS(UGameplayCam_Death)
IMPLEMENT_CLASS(UGameplayCam_Turret)
IMPLEMENT_CLASS(UGameplayCam_TurretTargeting)
IMPLEMENT_CLASS(UGameplayCam_Vehicle)
IMPLEMENT_CLASS(UGameplayCam_VehicleTurret)
IMPLEMENT_CLASS(UGameplayCam_VehicleTurretTargeting)
IMPLEMENT_CLASS(UGameplayCam_Vehicle_RideReaver)
IMPLEMENT_CLASS(UGameplayCam_CentaurDeath)
IMPLEMENT_CLASS(UGameplayCam_BrumakDeath)
IMPLEMENT_CLASS(UGameplayCam_MountedHeavyWeapon)
IMPLEMENT_CLASS(UGameplayCam_DeployedShield)
IMPLEMENT_CLASS(UGameplayCam_BrumakDriver)
IMPLEMENT_CLASS(UGameplayCam_BrumakTargeting)
IMPLEMENT_CLASS(UGameplayCam_TargetingGrenade)
IMPLEMENT_CLASS(UGameplayCam_CoverTargetingGrenade)
IMPLEMENT_CLASS(UGameplayCam_SniperZoom)

IMPLEMENT_CLASS(UFixedCam_AutoFraming)
IMPLEMENT_CLASS(USpectatorCam_AutoFraming)

IMPLEMENT_CLASS(ACameraVolume)
IMPLEMENT_CLASS(AAmbientShakeVolume);

/*-----------------------------------------------------------------------------
	Helpers
-----------------------------------------------------------------------------*/

static const FLOAT Rad2Deg = 180.f / PI;
static const FLOAT Deg2Rad = PI / 180.f;


static inline FVector TransformWorldToLocal(FVector const& WorldVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).Transpose().TransformNormal( WorldVect );
}

static inline FVector TransformLocalToWorld(FVector const& LocalVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).TransformNormal( LocalVect );
}

static inline FLOAT FPctByRange( FLOAT Value, FLOAT InMin, FLOAT InMax )
{
	return (Value - InMin) / (InMax - InMin);
}

static inline FLOAT DegToUnrRotatorUnits(FLOAT InDeg)
{
	return InDeg * 182.0444f;
}
static inline FLOAT RadToUnrRotatorUnits(FLOAT InDeg)
{
	return InDeg * 180.f / PI * 182.0444f;
}

/**
*	Returns world space angle (in radians) of given vector
*
*	@param	Dir		Vector to be converted into heading angle
*/
static FLOAT GetHeadingAngle( FVector const& Dir )
{
	FLOAT Angle = appAcos( Clamp( Dir.X, -1.f, 1.f ) );
	if( Dir.Y < 0.f )
	{
		Angle *= -1.f;
	}

	return Angle;
}

/**
* Just like the regular RInterpTo(), but with per axis interpolation speeds specification
*/
static FRotator RInterpToWithPerAxisSpeeds( FRotator const& Current, FRotator const& Target, FLOAT DeltaTime, FLOAT PitchInterpSpeed, FLOAT YawInterpSpeed, FLOAT RollInterpSpeed )
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if( DeltaTime == 0.f || Current == Target )
	{
		return Current;
	}

	// Delta Move, Clamp so we do not over shoot.
	FRotator DeltaMove = (Target - Current).GetNormalized();

	DeltaMove.Pitch *= appTrunc(Clamp(DeltaTime * PitchInterpSpeed, 0.f, 1.f));
	DeltaMove.Yaw *= appTrunc(Clamp(DeltaTime * YawInterpSpeed, 0.f, 1.f));
	DeltaMove.Roll *= appTrunc(Clamp(DeltaTime * RollInterpSpeed, 0.f, 1.f));

	return (Current + DeltaMove).GetNormalized();
}

/*-----------------------------------------------------------------------------
	UGearGameplayCameraMode
-----------------------------------------------------------------------------*/

FVector UGearGameplayCameraMode::GetViewOffset(class APawn* ViewedPawn, FLOAT DeltaTime, const FRotator& ViewRotation)
{
	FVector out_Offset(0.f);

	// figure out which viewport config we're in.  16:9 full is default to fall back on
	EGearCam_ViewportTypes ViewportConfig = CVT_16to9_Full;
	{
		// get viewport client
		UGameViewportClient* VPClient;
		{
			APlayerController* const PlayerOwner = Cast<APlayerController>(ViewedPawn->Controller);
			ULocalPlayer* const LP = (PlayerOwner != NULL) ? Cast<ULocalPlayer>(PlayerOwner->Player) : NULL;
			VPClient = (LP != NULL) ? LP->ViewportClient : NULL;
		}

		if ( VPClient )
		{
			// figure out if we are 16:9 or 4:3
			UBOOL bWideScreen = FALSE;
			{
				FVector2D ViewportSize;
				VPClient->GetViewportSize(ViewportSize);
			
				FLOAT Aspect =  ViewportSize.X / ViewportSize.Y;

				if ( (Aspect > (16.f/9.f-0.01f)) && (Aspect < (16.f/9.f+0.01f)) )
				{
					bWideScreen = TRUE;
				}
//				else if ( (Aspect < (4.f/3.f-0.01f)) || (Aspect > (4.f/3.f+0.01f)) )
//				{
//					debugf(TEXT("GetViewOffset: Unexpected aspect ratio %f, camera offsets may be incorrect."), Aspect);
//				}
			}

			// decide viewport configuration
			BYTE CurrentSplitType = VPClient->GetCurrentSplitscreenType();
			if (bWideScreen)
			{
				if (CurrentSplitType == eSST_2P_VERTICAL)
				{
					ViewportConfig = CVT_16to9_VertSplit;
				}
				else if (CurrentSplitType == eSST_2P_HORIZONTAL)
				{
					ViewportConfig = CVT_16to9_HorizSplit;
				}
				else
				{
					ViewportConfig = CVT_16to9_Full;
				}
			}
			else
			{
				if (CurrentSplitType == eSST_2P_VERTICAL)
				{
					ViewportConfig = CVT_4to3_VertSplit;
				}
				else if (CurrentSplitType == eSST_2P_HORIZONTAL)
				{
					ViewportConfig = CVT_4to3_HorizSplit;
				}
				else
				{
					ViewportConfig = CVT_4to3_Full;
				}
			}
		}
	}

	// find our 3 offsets
	FVector MidOffset(0.f), LowOffset(0.f), HighOffset(0.f);
	{
		GetBaseViewOffsets( ViewedPawn, ViewportConfig, DeltaTime, LowOffset, MidOffset, HighOffset );

		// apply viewport-config adjustments
		LowOffset += ViewOffset_ViewportAdjustments[ViewportConfig].OffsetLow;
		MidOffset += ViewOffset_ViewportAdjustments[ViewportConfig].OffsetMid;
		HighOffset += ViewOffset_ViewportAdjustments[ViewportConfig].OffsetHigh;
	}

	// calculate final offset based on camera pitch
	FLOAT Pitch = 0.f;
	if (!ViewedPawn->Base || !ViewedPawn->Base->IsA(AInterpActor_GearBasePlatform::StaticClass()))
	{
		Pitch = (FLOAT)FRotator::NormalizeAxis(ViewRotation.Pitch);
	}
	else
	{
		// want to use my pitch relative to dynamic base, instead of world origin (i.e. identity)
		FRotationMatrix const WorldViewRotMat(ViewRotation);
		FMatrix const BaseWorldToLocal = ViewedPawn->Base->LocalToWorld().Inverse();
		FRotator const ViewRotInBaseSpace = (WorldViewRotMat * BaseWorldToLocal).Rotator();
		Pitch = (FLOAT)FRotator::NormalizeAxis(ViewRotInBaseSpace.Pitch);
	}

	FLOAT Pct = 0.f;
	if( Pitch >= 0.f )
	{
		Pct			= Pitch / ViewedPawn->ViewPitchMax;
		out_Offset	= Lerp<FVector,FLOAT>( MidOffset, LowOffset, Pct );
	}
	else
	{
		Pct			= Pitch / ViewedPawn->ViewPitchMin;
		out_Offset	= Lerp<FVector,FLOAT>( MidOffset, HighOffset, Pct );
	}

	// note, this offset isn't really pawn-relative anymore, should
	// get folded into regular viewoffset stuff
	FVector const ExtraOffset = eventGetPawnRelativeOffset(ViewedPawn);
	out_Offset += ExtraOffset;

	return out_Offset;
}

/** Returns true if mode should lock camera to view target, false otherwise */
UBOOL UGearGameplayCameraMode::LockedToViewTarget(APawn* CameraTarget)
{
	return bLockedToViewTarget;
}


/** Returns true if mode should be using direct-look mode, false otherwise */
UBOOL UGearGameplayCameraMode::UseDirectLookMode(APawn* CameraTarget)
{
	return bDirectLook;
}

/**
* Returns true if this mode should do target following.  If true is returned, interp speeds are filled in.
* If false is returned, interp speeds are not altered.
*/
UBOOL UGearGameplayCameraMode::ShouldFollowTarget(APawn* CameraTarget, FLOAT& PitchInterpSpeed, FLOAT& YawInterpSpeed, FLOAT& RollInterpSpeed)
{
	return (bLockedToViewTarget) ? FALSE : bFollowTarget;
}


void UGearGameplayCameraMode::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	FVector StrafeOffset(0.f), RunOffset(0.f);

	// calculate strafe and running offsets
	FLOAT VelMag = ViewedPawn->Velocity.Size();

	if (VelMag > 0.f)
	{
		FVector X, Y, Z;
		FRotationMatrix(ViewedPawn->Rotation).GetAxes(X, Y, Z);
		FVector NormalVel = ViewedPawn->Velocity / VelMag;

		if (StrafeOffsetScalingThreshold > 0.f)
		{
			FLOAT YDot = Y | NormalVel;
			if (YDot < 0.f)
			{
				StrafeOffset = StrafeLeftAdjustment * -YDot;
			}
			else
			{
				StrafeOffset = StrafeRightAdjustment * YDot;
			}
			StrafeOffset *= Clamp(VelMag / StrafeOffsetScalingThreshold, 0.f, 1.f);
		}

		if (RunOffsetScalingThreshold > 0.f)
		{
			FLOAT XDot = X | NormalVel;
			if (XDot < 0.f)
			{
				RunOffset = RunBackAdjustment * -XDot;
			}
			else
			{
				RunOffset = RunFwdAdjustment * XDot;
			}
			RunOffset *= Clamp(VelMag / RunOffsetScalingThreshold, 0.f, 1.f);
		}
	}

	// interpolate StrafeOffset and RunOffset to avoid little pops
	FLOAT Speed = StrafeOffset.IsZero() ? StrafeOffsetInterpSpeedOut : StrafeOffsetInterpSpeedIn;
	StrafeOffset = VInterpTo(LastStrafeOffset, StrafeOffset, DeltaTime, Speed);
	LastStrafeOffset = StrafeOffset;

	Speed = RunOffset.IsZero() ? RunOffsetInterpSpeedOut : RunOffsetInterpSpeedIn;
	RunOffset = VInterpTo(LastRunOffset, RunOffset, DeltaTime, Speed);
	LastRunOffset = RunOffset;

	// Controllers are not valid for other players in MP mode
	FRotator CamRot;
	CamRot.Yaw = 0;
	CamRot.Pitch = 0;
	CamRot.Roll = 0;
	if( ViewedPawn->Controller )
	{
		FVector UnusedVec(0.0f);
		ViewedPawn->Controller->eventGetPlayerViewPoint(UnusedVec, CamRot);
	}
	// so just use the Pawn's data to determine where to place the camera's starting loc / rot
	else
	{
		CamRot = ViewedPawn->Rotation;
	}

	FVector TotalOffset = StrafeOffset + RunOffset;
	TotalOffset = ::TransformWorldToLocal(TotalOffset, ViewedPawn->Rotation);
	TotalOffset = ::TransformLocalToWorld(TotalOffset, CamRot);

	out_Low		= ViewOffset.OffsetLow;
	out_Mid 	= ViewOffset.OffsetMid;
	out_High	= ViewOffset.OffsetHigh;

	AGearPawn* const WP = Cast<AGearPawn>(ViewedPawn);
	if (WP && WP->bIsMirrored)
	{
		out_Low.Y = -out_Low.Y;
		out_Mid.Y = -out_Mid.Y;
		out_High.Y = -out_High.Y;
	}

	out_Low		+= TotalOffset;
	out_Mid 	+= TotalOffset;
	out_High	+= TotalOffset;
}

FVector UGearGameplayCameraMode::GetTargetRelativeOriginOffset(class APawn* TargetPawn)
{
	return TargetRelativeCameraOriginOffset;
}

void UGearGameplayCameraMode::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	AGearPawn* const GP = Cast<AGearPawn>(TargetPawn);

	// Rotation
	if ( TargetPawn && (GameplayCam->bResetCameraInterpolation || LockedToViewTarget(TargetPawn)) )
	{
		OriginRot = TargetPawn->eventGetViewRotation();
	}
	else
	{
		// use the camera's rotation
		OriginRot = GameplayCam->PlayerCamera->Rotation;
	}

	// Location
	if (GP)	
	{
		if (GP->SpecialMove == SM_MidLvlJumpOver)
		{
			// special move wants to handle it's own bidness
			UGSM_MidLvlJumpOver* const SM = CastChecked<UGSM_MidLvlJumpOver>(GP->SpecialMoves(SM_MidLvlJumpOver));
			if (SM)
			{
				OriginLoc = SM->GetIdealCameraOrigin();
			}
		}
		else
		{
			// standard way for GearPawns.  Note we aren't doing GetPawnViewLocation here, since that is attached to a bone,
			// which is good for AI and the like, but we don't want here.
			OriginLoc = TargetPawn->Location;
			OriginLoc.Z += TargetPawn->BaseEyeHeight;
			if (TargetPawn->bIsCrouched)
			{
				OriginLoc.Z += TargetPawn->CrouchHeight * 0.5f;
			}

			//GP->FlushPersistentDebugLines();
			//GP->DrawDebugBox( TargetPawn->Location, FVector(5,5,5), 255, 0, 0, TRUE );
			//debugf(TEXT("TP Loc %s EyeHeight %f Crouch %d Origin %s"), *TargetPawn->Location.ToString(), TargetPawn->BaseEyeHeight, TargetPawn->bIsCrouched, *OriginLoc.ToString());
		}
	}
	else
	{
		// for non-GearPawns, let the Pawn decide what it's viewlocation is.
		OriginLoc = TargetPawn->eventGetPawnViewLocation();
	}

	// apply any location offset
	OriginLoc += TransformLocalToWorld(GetTargetRelativeOriginOffset(TargetPawn), TargetPawn->Rotation);
}

/** Returns time to interpolate FOV changes. */
FLOAT UGearGameplayCameraMode::GetFOVBlendTime(class APawn* Pawn)
{
	return BlendTime;
}

/** Returns time to interpolate location/rotation changes. */
FLOAT UGearGameplayCameraMode::GetBlendTime(class APawn* Pawn)
{
	return BlendTime;
}

/*-----------------------------------------------------------------------------
	UGearCamMod_CameraBone
-----------------------------------------------------------------------------*/
FLOAT UGearCamMod_CameraBone::GetTargetAlpha( class ACamera *Camera )
{
	FLOAT AlphaBonus=0.f, RetAlpha=0.f, CurSpeed=0.f, Scale=0.f;

	if (Super::GetTargetAlpha(Camera) != 0.f)
	{
		UBOOL bInCombat;
		AGearPC* WPC = Cast<AGearPC>(Pawn->Controller);

		if ( (WPC != NULL) && WPC->bCinematicMode )
		{
			return 0.f;
		}

		// apply an increased amount of camera bone motion if we've been recently shot at
		bInCombat = ( GWorld->GetTimeSeconds() - Pawn->LastShotAtTime < 5.0f ) || ( WPC != NULL && WPC->eventIsInCombat() );
		if (bInCombat && !bDisableAmbientCameraMotion)
		{
			AlphaBonus += InCombatAlphaBonus;
		}

		if( Pawn->SpecialMove == SM_RoadieRun )
		{
			// roadie run gets full monty, and isn't considered "ambient" camera motion
			RetAlpha = RoadieRunShakyCamDampeningFactor;
			Scale = 1.f;

			if (GEngine->IsSplitScreen())
			{
				AGearPlayerCamera* Cam = Cast<AGearPlayerCamera>(CameraOwner);
				if (Cam)
				{
					Scale *= Cam->SplitScreenShakeScale;
				}
			}
		}
		else if ( Pawn->eventIsFiringHOD() )
		{
			// firing HOD, be still
			RetAlpha = 0.f;
			Scale = 0.f;
			AlphaBonus = 0.f;
		}
		else if ( Pawn->eventIsPlayingCustomCameraAnim() )
		{
			if ( Pawn->CustomCameraAnimProperties.bApplyFullMotion )
			{
				// camera anims aren't considered "ambient", but are subject to scaling
				RetAlpha = 1.f;
				Scale = 1.f;
			}
			else 
			{
				// camera anims aren't considered "ambient", but are subject to scaling
				RetAlpha = 1.f;
				Scale = bOXMShakyCamDampening ? OXMShakyCamDampeningFactor : 1.f;

				// if not in combat and not pressing joystick
				if (!bInCombat && Pawn->Velocity.IsZero() && WPC != NULL && Abs(WPC->RemappedJoyRight) < WPC->DeadZoneThreshold && Abs(WPC->RemappedJoyUp) < WPC->DeadZoneThreshold)
				{
					// negate the idle shake entirely
					Scale = 0.f;
				}
				else if (Pawn->bIsTargeting)
				{
					// very steady when in targeting mode
					AGearWeapon * WW = Cast<AGearWeapon>(Pawn->Weapon);
					if ( (WW != NULL) && WW->bSuperSteadyCamWhileTargeting )
					{
						Scale = 0.f;
					}
					else
					{
						Scale *= 0.25f;
					}
					AlphaBonus *= 0.25f;
				}
			}
		}
		else if (Pawn->GroundSpeed != 0.f)
		{
			// set up base camera scaling factor.  may get situationally modified later
			Scale = bDisableAmbientCameraMotion ? 0.f : OXMShakyCamDampeningFactor;

			// we're moving, scale the camera motion with velocity
			CurSpeed = Pawn->Velocity.Size();

			if( Pawn->CoverType != CT_None )
			{
				RetAlpha = CurSpeed/(Pawn->GroundSpeed*Pawn->CoverMovementPct(Pawn->CoverType)) * (1.f - AlphaWhenIdle) + AlphaWhenIdle;
			}
			else
			{
				RetAlpha = CurSpeed/Pawn->GroundSpeed * (1.f - AlphaWhenIdle) + AlphaWhenIdle;
			}

			if (Pawn->bIsTargeting)
			{
				// very steady when in targeting mode
				AGearWeapon * WW = Cast<AGearWeapon>(Pawn->Weapon);
				if ( (WW != NULL) && WW->bSuperSteadyCamWhileTargeting )
				{
					// no motion on HOD
					Scale = 0.f;
				}
				else
				{
					Scale *= 0.25f;
				}
				AlphaBonus = 0.f;
			}
			else if (Pawn->eventIsInAimingPose())
			{
				// steadier when aiming
				Scale *= 0.5f;
			}
			else if (!bInCombat && Pawn->Velocity.IsZero() && WPC != NULL && Abs(WPC->RemappedJoyRight) < WPC->DeadZoneThreshold && Abs(WPC->RemappedJoyUp) < WPC->DeadZoneThreshold)
			{
				// negate the idle shake entirely
				Scale = 0.f;
			}
		}

		// calc final alpha
		RetAlpha = RetAlpha * Scale + AlphaBonus;
	}

	return RetAlpha;
}

FRotator UGearCamMod_CameraBone::CalcDeltaRot(FRotator CameraRootRot, FRotator CameraBoneRot, FRotator PawnRot)
{
	FVector X, Y, Z;

	// omfg this is a terrible hack, but it works and time is short
	// this is to overcome the fact that the camera bones aren't based on an 
	// identity orientation.  should be a general way around this.

	FRotationMatrix RootMat(CameraRootRot);
	RootMat.GetAxes(X, Y, Z);
	X = -X;
	RootMat.SetAxes(&Y, &X, &Z);
	FRotator FixedCameraRootRot = RootMat.Rotator();

	FRotationMatrix BoneMat(CameraBoneRot);
	BoneMat.GetAxes(X, Y, Z);
	X = -X;
	BoneMat.SetAxes(&Y, &X, &Z);
	FRotator FixedCameraBoneRot = BoneMat.Rotator();

	FRotator Delta = (FixedCameraBoneRot - FixedCameraRootRot).GetNormalized();
	return Delta;
}

UBOOL UGearCamMod_CameraBone::Initialize()
{
	if( CameraOwner != NULL &&
		CameraOwner->PCOwner != NULL &&
		Cast<AGearPawn>(CameraOwner->PCOwner->Pawn) != NULL &&
		CameraOwner->PCOwner->Pawn->Mesh != NULL )
	{
		Pawn = CastChecked<AGearPawn>(CameraOwner->PCOwner->Pawn);
		Mesh = Pawn->Mesh;
		return TRUE;
	}
	return FALSE;
}

FRotator UGearCamMod_CameraBone::FixCamBone(FRotator A)
{
	FVector X, Y, Z;
	FRotationMatrix R(A);
	X = R.GetAxis(0);
	Y = R.GetAxis(1);
	Z = R.GetAxis(2);

	X = -X;

	FMatrix M = FMatrix::Identity;
	M.SetAxis( 0, Y );
	M.SetAxis( 1, X );
	M.SetAxis( 2, Z );

	return M.Rotator();
	/*
	local vector X, Y, Z;
	GetAxes(R, X, Y, Z);
	X = -X;
	return OrthoRotation(Y, X, Z);

	*/
}

void UGearCamMod_CameraBone::UpdateAlpha(class ACamera* Camera,FLOAT DeltaTime)
{
	FLOAT PvsAlpha = Alpha;

	Super::UpdateAlpha(Camera, DeltaTime);

	Alpha = FInterpTo(PvsAlpha, Alpha, DeltaTime, AlphaInterpSpeed);
}

FVector VClamp(FVector A, FVector Min, FVector Max)
{
	A.X = Clamp<FLOAT>(A.X,Min.X,Max.X);
	A.Y = Clamp<FLOAT>(A.Y,Min.Y,Max.Y);
	A.Z = Clamp<FLOAT>(A.Z,Min.Z,Max.Z);
	return A;
}

UBOOL UGearCamMod_CameraBone::ModifyCamera(class ACamera* Camera,FLOAT DeltaTime,FTPOV& OutPOV)
{
	FRotator DeltaRot, CameraBoneWorldRot, CameraBoneRootWorldRot, FixedCameraRootRot;
	FVector Delta, NewBoneLocation, CameraBoneRootWorldPos, CameraBoneWorldPos;
	UBOOL bInitOK;

	// init every time, since it is possible for the camera target to change,
	// as is the case when getting in/out of vehicles.
	bInitOK = Initialize();
	if (!bInitOK)
	{
		return FALSE;
	}

	if( !Pawn || Pawn->IsA(AGearPawn_LocustBrumakBase::StaticClass()) != NULL )
	{
		return FALSE;
	}

	UpdateAlpha( Camera, DeltaTime );
	//debugf(TEXT("native alpha %f "), GetTargetAlpha(Camera));

	// Set camera bone motion scale on Pawn->
	// This is used by animations.
	Pawn->CameraBoneMotionScale = Alpha;

	if( Alpha > 0.f )
	{
		// If just using delta
		if( CameraAnimOption == CAO_TranslateDelta ||
			CameraAnimOption == CAO_TranslateRotateDelta )
		{
			// Note: Camera delta is derived from pos/orient of the camera bone, relative to the camera root
			CameraBoneWorldPos = Mesh->GetBoneLocation(CameraBoneName, 0);
			CameraBoneRootWorldPos = Mesh->GetBoneLocation(CameraBoneRootName, 0);
			CameraBoneRootWorldRot = FQuatRotationTranslationMatrix( Mesh->GetBoneQuaternion(CameraBoneRootName, 0), FVector(0.f) ).Rotator();
			FixedCameraRootRot = FixCamBone(CameraBoneRootWorldRot);

			// Get delta scaled by alpha, capped to the max translation box
			Delta = (CameraBoneWorldPos - CameraBoneRootWorldPos) * Alpha;
			Delta = VClamp(Delta, -MaxTranslation, MaxTranslation);

			// camera/pawn rotation can differ significantly from leg/cameraroot orient
			// so we correct for that here.
			Delta = FRotationMatrix(FixedCameraRootRot).Transpose().TransformNormal( Delta );
			//`log("Local space Delta is"@Delta);
			Delta = FRotationMatrix(Pawn->Rotation).TransformNormal( Delta );

			// Apply the change to camera location
			OutPOV.Location += Delta;

			if( CameraAnimOption == CAO_TranslateRotateDelta )
			{
				CameraBoneWorldRot = FQuatRotationTranslationMatrix( Mesh->GetBoneQuaternion(CameraBoneName, 0), FVector(0.f) ).Rotator();
				//CameraBoneRootWorldRot = QuatToRotator( Mesh.GetBoneQuaternion(CameraBoneRootName, 0) );		// already got it above

				DeltaRot = CalcDeltaRot(CameraBoneRootWorldRot, CameraBoneWorldRot, Pawn->Rotation);

				DeltaRot *= Alpha;

				//`log("CameraBoneWorldRot"@CameraBoneWorldRot@"CameraBoneRootWorldRot"@CameraBoneRootWorldRot@"Pawn->Rotation"@Pawn->Rotation);

				// apply change to camera rot
				OutPOV.Rotation += DeltaRot;
			}
		}
		else if( CameraAnimOption == CAO_Absolute )
		{
			// Grab new bone location
			NewBoneLocation = Mesh->GetBoneLocation( CameraBoneName, 0 );

			Delta = (NewBoneLocation - OutPOV.Location) * Alpha;
			OutPOV.Location += Delta;
		}
	}

	// If ModifyCamera returns true, exit loop
	// Allows high priority things to dictate if they are
	// the last modifier to be applied
	// Returning TRUE causes to stop adding another modifier! 
	// Returning FALSE is the right behavior since this is not high priority modifier.
	return FALSE;
}
/*-----------------------------------------------------------------------------
	AGearPlayerCamera
-----------------------------------------------------------------------------*/


/**
* Handles traces to make sure camera does not penetrate geometry and tries to find
* the best location for the camera.
* Also handles interpolating back smoothly to ideal/desired position.
*
* @param	WorstLocation		Worst location (Start Trace)
* @param	DesiredLocation		Desired / Ideal position for camera (End Trace)
* @param	DeltaTime			Time passed since last frame.
* @param	DistBlockedPct		percentage of distance blocked last frame, between WorstLocation and DesiredLocation. To interpolate out smoothly
* @param	CameraExtentScale	Scale camera extent. (box used for collision)
* @param	bSingleRayOnly		Only fire a single ray.  Do not send out extra predictive feelers.
*/
void UGearGameplayCamera::PreventCameraPenetration(APawn* P, const FVector& WorstLocation, FVector& DesiredLocation, FLOAT DeltaTime, FLOAT& DistBlockedPct, FLOAT CameraExtentScale, UBOOL bSingleRayOnly)
{	
	FLOAT HardBlockedPct = DistBlockedPct;
	FLOAT SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = DesiredLocation - WorstLocation;
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;
	BaseRayMatrix.GetAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	FLOAT CheckDist = BaseRay.Size();

	FLOAT DistBlockedPctThisFrame = 1.f;

	//FlushPersistentDebugLines();
	INT NumRaysToShoot = (bSingleRayOnly) ? Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num();

	for (INT RayIdx=0; RayIdx<NumRaysToShoot; ++RayIdx)
	{
		FMemMark Mark(GMainThreadMemStack);

		FPenetrationAvoidanceFeeler& Feeler = PenetrationAvoidanceFeelers(RayIdx);

		// calc ray target
		FVector RayTarget;
		{
			FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
			RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
			RayTarget = WorstLocation + RotatedRay;
		}

		// cast for world and pawn hits separately.  this is so we can safely ignore the 
		// camera's target pawn
		DWORD TraceFlags = (Feeler.PawnWeight > 0.f) ? TRACE_World | TRACE_Pawns : TRACE_World;
		FVector const CheckExtent = Feeler.Extent * CameraExtentScale;

		// on platforms, use complex collision.  this is a hack, since we increased the height of
		// the bounding walls to keep the player in
		if (P && P->Base)
		{
			AInterpActor_GearBasePlatform* BP = Cast<AInterpActor_GearBasePlatform>(P->Base);
			if (BP && BP->bDoComplexCameraCollision)
			{
				TraceFlags |= TRACE_ComplexCollision;
			}
		}

		// do multiline check to make sure we hits we throw out aren't
		// masking real hits behind (these are important rays).
		FCheckResult const* const pHitList = GWorld->MultiLineCheck(GMainThreadMemStack, RayTarget, WorstLocation, CheckExtent, TraceFlags, P);

		//DrawDebugLine(WorstLocation, RayTarget, Feeler.Weight*255, 255, 0, TRUE);
		//DrawDebugCoordinateSystem(WorstLocation, BaseRay.Rotation(), 32.f, TRUE);
		//DrawDebugLine(WorstLocation, WorstLocation + BaseRay, Feeler.Weight*255, 255, 0, TRUE);

		for( FCheckResult const* Hit = pHitList; Hit != NULL; Hit = Hit->GetNext() )
		{
			if (Hit->Actor != NULL)
			{
				FLOAT Weight;

				// experimenting with not colliding with pawns
				APawn* HitPawn = Hit->Actor->GetAPawn();
				if( HitPawn )
				{
					if ( P && ((P->DrivenVehicle == HitPawn) || (P == HitPawn->Base)) )
					{
						// ignore hits on the vehicle we're driving
						continue;
					}

					// * ignore ragdoll hits (can still his the collision cylinder, which is bad)
					// * also ignore zero-time pawn hits, which can happen since the raycast extent
					// box can poke out of the player's collision cylinder and register hits
					// even if the raycast goes away from the pawn
					// * also ignore warpawns that don't block the camera
					AGearPawn const* const WP = Cast<AGearPawn>(HitPawn);
					if ( WP && ( !WP->bBlockCamera || (WP->Physics == PHYS_RigidBody) || (Hit->Time == 0.f) ) )
					{
						continue;
					}
					else
					{
						AGearVehicle const* const GV = Cast<AGearVehicle>(HitPawn);
						if( GV != NULL && (!GV->bBlockCamera || Hit->Time == 0.f) )
						{
							continue;
						}

						// ignore hits that hit the vehicle for the warweaponpawn we're controlling
						AGearWeaponPawn const* const WWP = Cast<AGearWeaponPawn>(P);
						if ( WWP && (WWP->MyVehicle == HitPawn) )
						{
							continue;
						}
					}

					Weight = Feeler.PawnWeight;
				}
				else
				{
					// ignore KActorSpawnables, since they're only used for small, inconsequential things (for now anyway)
					if (Cast<AKActorSpawnable>(Hit->Actor))
					{
						continue;
					}

					// ignore BlockingVolumes with bBlockCamera=false
					ABlockingVolume* const BV = Cast<ABlockingVolume>(Hit->Actor);
					if (BV && !BV->bBlockCamera)
					{
						continue;
					}

					// if we're a zero extent trace and we hit something that doesn't take NZE hits
					// reject it.  this prevents situations where the predictive feeler traces hit something
					// that the main trace doesn't.
					// @fixme, too late in Gears 2 to try this, consider this for Gears 3.
					// LDs sometimes mark stuff "blockweapons" to let the player walk through, and use BVs
					// to contain the player, which can also block the camera.
					//if (Feeler.Extent.IsZero() && Hit->Component && !Hit->Component->BlockNonZeroExtent && !(TraceFlags & TRACE_ComplexCollision) )
					//{
					//	continue;
					//}

					if (Hit->Component && !Hit->Component->CanBlockCamera)
					{
						continue;
					}

					Weight = Feeler.WorldWeight;
				}

				FLOAT NewBlockPct	= Hit->Time;
				NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);
				DistBlockedPctThisFrame = Min(NewBlockPct, DistBlockedPctThisFrame);
			}
		}


		// collide with any extra collision planes the camera mode may want to use
		// @fixme, if this gets uncommented, move the eventGetAuxCollisionPlane() call out of the for loop
		//FVector PlaneNormal, PlaneOrigin;
		//if (CurrentCamMode->eventGetAuxCollisionPlane(P, PlaneOrigin, PlaneNormal))
		//{
		//	FVector PlaneIntersect = FLinePlaneIntersection(WorstLocation, DesiredLocation, PlaneOrigin, PlaneNormal);
		//	FLOAT NewBlockPct = (PlaneIntersect - WorstLocation).Size() / CheckDist;
		//	NewBlockPct += (1.f - NewBlockPct) * (1.f - Feeler.WorldWeight);
		//	FLOAT NewBlockPct2 = (PlaneIntersect - DesiredLocation).Size() / CheckDist;
		//	if( (NewBlockPct < DistBlockedPctThisFrame) && (NewBlockPct2 < 1.f) )
		//	{
		//		DistBlockedPctThisFrame = NewBlockPct;
		//	}
		//}

		if (RayIdx == 0)
		{
			// don't interpolate toward this one, snap to it
			HardBlockedPct = DistBlockedPctThisFrame;
		}
		else
		{
			SoftBlockedPct = DistBlockedPctThisFrame;
		}

		Mark.Pop();
	}

	if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out
		if (PenetrationBlendOutTime > DeltaTime)
		{
			DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
		}
		else
		{
			DistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	else
	{
		if (DistBlockedPct > HardBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in
			if (PenetrationBlendInTime > DeltaTime)
			{
				DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
			}
			else
			{
				DistBlockedPct = SoftBlockedPct;
			}
		}
	}

	DistBlockedPct = Clamp<FLOAT>(DistBlockedPct, 0.f, 1.f);

	if( DistBlockedPct < 1.f ) 
	{
		DesiredLocation	= WorstLocation + (DesiredLocation - WorstLocation) * DistBlockedPct;
	}
}


FVector UGearGameplayCamera::GetEffectiveFocusLoc(const FVector& CamLoc, const FVector& FocusLoc, const FVector& ViewOffset)
{
	FLOAT YawDelta=0, PitchDelta=0, PitchDist=0, YawDist=0;
	const FVector Fwd(1.f,0.f,0.f);

	FVector const CamToFocus = FocusLoc - CamLoc;
	FLOAT const CamToFocusSize = CamToFocus.Size();

	// quick summary of what's going on here
	// what we want is for the effective focus loc to mirror the camera offset if the 
	// real camera was looking at the focus point.  This is basically  a parallelogram with the base 
	// camera position and the focusloc as one diagonal, and the ideal camera loc and the effective
	// loc forming the other diagonal.  
	// given the data we have, we need solve an SSA triangle to find the adjustment angle.
	// we do this twice, once for the yaw axis (XY plane) and once for pitch (XZ plane).

	// YAW
	{
		FVector ViewOffset3DNorm = ViewOffset;
		ViewOffset3DNorm.Z = 0.f;
		FLOAT ViewOffset3DSize = ViewOffset3DNorm.Size();
		ViewOffset3DNorm /= ViewOffset3DSize;	

		FLOAT DotProd = ViewOffset3DNorm | Fwd;
		if ( (DotProd < 0.999f) && (DotProd > -0.999f) )
		{
			FLOAT Alpha = PI - appAcos(DotProd);

			FLOAT SinTheta = ViewOffset3DSize * appSin(Alpha) / CamToFocusSize;
			FLOAT Theta = appAsin(SinTheta);

			YawDelta = RadToUnrRotatorUnits(Theta);
			if (ViewOffset.Y > 0.f)
			{
				YawDelta = -YawDelta;
			}

			FLOAT Phi = PI - Theta - Alpha;
			YawDist = ViewOffset3DSize * appSin(Phi) / SinTheta - CamToFocusSize;
		}
	}

	// PITCH
	{
		FVector ViewOffset3DNorm = ViewOffset;
		ViewOffset3DNorm.Y = 0.f;
		FLOAT ViewOffset3DSize = ViewOffset3DNorm.Size();
		ViewOffset3DNorm /= ViewOffset3DSize;	

		FLOAT DotProd = ViewOffset3DNorm | Fwd;
		if ( (DotProd < 0.999f) && (DotProd > -0.999f) )
		{
			FLOAT Alpha = PI - appAcos(DotProd);
			FLOAT SinTheta = ViewOffset3DSize * appSin(Alpha) / CamToFocusSize;
			FLOAT Theta = appAsin(SinTheta);

			PitchDelta = RadToUnrRotatorUnits(Theta);
			if (ViewOffset.Z > 0.f)
			{
				PitchDelta = -PitchDelta;
			}

			FLOAT Phi = PI - Theta - Alpha;
			PitchDist = ViewOffset3DSize * appSin(Phi) / appSin(Theta) - CamToFocusSize;
		}
	}

	FLOAT Dist = CamToFocusSize + PitchDist + YawDist;
	FRotator const CamToFocusRot = CamToFocus.Rotation();
	FRotationMatrix const M(CamToFocusRot);
	FVector X, Y, Z;
	M.GetAxes(X, Y, Z);

	FVector AdjustedCamVec = CamToFocus.RotateAngleAxis(appTrunc(YawDelta), Z);
	AdjustedCamVec = AdjustedCamVec.RotateAngleAxis(appTrunc(-PitchDelta), Y);
	AdjustedCamVec.Normalize();

	FVector EffectiveFocusLoc = CamLoc + AdjustedCamVec * Dist;

	//debugf(TEXT("effectivefocusloc (%f, %f, %f)"), EffectiveFocusLoc.X, EffectiveFocusLoc.Y, EffectiveFocusLoc.Z);

	return EffectiveFocusLoc;
}


/** 
* Given a horizontal FOV that assumes a 16:9 viewport, return an appropriately
* adjusted FOV for the viewport of the target Pawn->  Maintains constant vertical FOV.
* Used to correct for splitscreen.
*/
FLOAT AGearPlayerCamera::AdjustFOVForViewport(FLOAT inHorizFOV, APawn* CameraTargetPawn)
{
	FLOAT OutFOV = inHorizFOV;

	if (CameraTargetPawn)
	{
		APlayerController* const PlayerOwner = Cast<APlayerController>(CameraTargetPawn->Controller);
		ULocalPlayer* const LP = (PlayerOwner != NULL) ? Cast<ULocalPlayer>(PlayerOwner->Player) : NULL;
		UGameViewportClient* const VPClient = (LP != NULL) ? LP->ViewportClient : NULL;
		if ( VPClient && (VPClient->GetCurrentSplitscreenType() == eSST_2P_VERTICAL) )
		{
			FVector2D FullViewportSize(0,0);
			VPClient->GetViewportSize(FullViewportSize);

			FLOAT const BaseAspect =  FullViewportSize.X / FullViewportSize.Y;
			UBOOL bWideScreen = FALSE;
			if ( (BaseAspect > (16.f/9.f-0.01f)) && (BaseAspect < (16.f/9.f+0.01f)) )
			{
				bWideScreen = TRUE;
			}

			// find actual size of player's viewport
			FVector2D PlayerViewportSize;
			PlayerViewportSize.X = FullViewportSize.X * LP->Size.X;
			PlayerViewportSize.Y = FullViewportSize.Y * LP->Size.Y;

			// calculate new horizontal fov
			FLOAT NewAspectRatio = PlayerViewportSize.X / PlayerViewportSize.Y;
			OutFOV = (NewAspectRatio / BaseAspect /*AspectRatio16to9*/) * appTan(inHorizFOV * 0.5f * PI / 180.f);
			OutFOV = 2.f * appAtan(OutFOV) * 180.f / PI;
		}
	}

	return OutFOV;
}

// We'll update all of the world-space Last* vars used for interpolation here to add the 
// motion introduced by the motion of the base
void UGearGameplayCamera::UpdateForMovingBase(AActor* BaseActor)
{
	if ( BaseActor && BaseActor->IsA(AInterpActor_GearBasePlatform::StaticClass()) )
	{
		// this check handles the edge case of changing bases.  If this is false, LastTargetBaseRot/Loc
		// will be unreliable.  We will assume they are at the current base loc/rot, which means no change
		// and therefore no adjustment.
		if (PlayerCamera->LastTargetBase == BaseActor)
		{
			FMatrix OldBaseTM = PlayerCamera->LastTargetBaseTM;
			OldBaseTM.RemoveScaling();
			FMatrix NewBaseTM = BaseActor->LocalToWorld();
			NewBaseTM.RemoveScaling();

			FRotationTranslationMatrix const LastActualCameraOriginTM(LastActualCameraOriginRot, LastActualCameraOrigin);
			FMatrix RelTM = LastActualCameraOriginTM * OldBaseTM.Inverse();
			FMatrix UpdatedLastCameraOriginTM = RelTM * NewBaseTM;

			LastActualCameraOrigin = UpdatedLastCameraOriginTM.GetOrigin();
			LastActualCameraOriginRot = UpdatedLastCameraOriginTM.Rotator();
		}

		// @fixme, check focus point vars?
		// @fixme, LastPostCamTurnYaw?
	}
}

void UGearGameplayCamera::PlayerUpdateCameraNative(APawn* P, FLOAT DeltaTime, FTViewTarget& OutVT)
{
	// this can legitimately be NULL if the target isn't a GearPawn (such as a vehicle or turret)
	AGearPawn* WP = Cast<AGearPawn>(P);

	// "effective" WP is the GearPawn that's connected to the camera, be it 
	// in a vehicle or turret or whatever.
	AGearPawn* EffectiveWP = WP;
	if (!EffectiveWP)
	{
		AVehicle* VP = Cast<AVehicle>(P);
		if (VP)
		{
			EffectiveWP = Cast<AGearPawn>(VP->Driver);
		}
	}

	// Get pos/rot for camera origin (base location where offsets, etc are applied from)
	// handle a moving base, if necessary
	UpdateForMovingBase(P->Base);

	FRotator	IdealCameraOriginRot;
	FVector		IdealCameraOrigin;
	CurrentCamMode->GetCameraOrigin(P, IdealCameraOrigin, IdealCameraOriginRot);

	//debugf(TEXT("Ideal %s CamMode %s PawnLoc %s"), *IdealCameraOrigin.ToString(), *CurrentCamMode->GetName(), *P->Location.ToString());
//	if( P && P->Controller && P->Controller->IsLocalPlayerController() )
//	{
//		PlayerCamera->DrawDebugBox(IdealCameraOrigin, FVector(10.f,10.f,10.f), 255, 255, 0, FALSE);
//		PlayerCamera->DrawDebugBox(LastActualCameraOrigin, FVector(10.f,10.f,10.f), 255, 0, 0, FALSE);
//	}

	// First, update the camera origin.
	// This is the point in world space where camera offsets are applied.
	// We apply lazy cam on this location, so we can have a smooth / slow interpolation speed there,
	// And a different speed for offsets.
	FVector ActualCameraOrigin;
	{
		if( bResetCameraInterpolation || !CurrentCamMode->bInterpLocation )
		{
			// if this is the first time we update, then do not interpolate.
			ActualCameraOrigin = IdealCameraOrigin;
		}
		else
		{
			if (CurrentCamMode->bUsePerAxisLocInterp)
			{
				ActualCameraOrigin.X = FInterpTo(LastActualCameraOrigin.X,IdealCameraOrigin.X,DeltaTime,CurrentCamMode->PerAxisLocInterpSpeed.X);
				ActualCameraOrigin.Y = FInterpTo(LastActualCameraOrigin.Y,IdealCameraOrigin.Y,DeltaTime,CurrentCamMode->PerAxisLocInterpSpeed.Y);
				ActualCameraOrigin.Z = FInterpTo(LastActualCameraOrigin.Z,IdealCameraOrigin.Z,DeltaTime,CurrentCamMode->PerAxisLocInterpSpeed.Z);
			}
			else
			{
				// Apply lazy cam effect to the camera origin point
				ActualCameraOrigin	= VInterpTo(LastActualCameraOrigin, IdealCameraOrigin, DeltaTime, CurrentCamMode->InterpLocSpeed);
			}
		}
		LastActualCameraOrigin = ActualCameraOrigin;
	}

	// smooth out CameraOriginRot if necessary
	FRotator ActualCameraOriginRot;
	{
		AGearPC* const WPC = (PlayerCamera) ? Cast<AGearPC>(PlayerCamera->PCOwner) : NULL;

		if (WPC && WPC->eventIsSpectating() && !bResetCameraInterpolation)
		{
			// spectating a player, so we'll smooth the rot to help out with latency issues
			ActualCameraOriginRot = RInterpTo(LastActualCameraOriginRot, IdealCameraOriginRot, DeltaTime, SpectatorCameraRotInterpSpeed);
		}
		else if (bResetCameraInterpolation || !CurrentCamMode->bInterpRotation)
		{
			ActualCameraOriginRot = IdealCameraOriginRot;
		}
		else
		{
			ActualCameraOriginRot = RInterpTo(LastActualCameraOriginRot, IdealCameraOriginRot, DeltaTime, CurrentCamMode->InterpRotSpeed);
		}

		LastActualCameraOriginRot = ActualCameraOriginRot;
	}

	// do any pre-viewoffset focus point adjustment
	eventUpdateFocusPoint(P);

	// doing adjustment before view offset application in order to rotate around target
	// also doing before origin offset application to avoid pops in cover
	// using last viewoffset here, since we have a circular dependency with the data.  focus point adjustment
	// needs viewoffset, but viewoffset is dependent on results of adjustment.  this introduces a bit of error,
	// but I believe it won't be noticeable.
	AdjustToFocusPointKeepingTargetInView(P, DeltaTime, ActualCameraOrigin, ActualCameraOriginRot, LastViewOffset);

	// Get the camera-space offset from the camera origin
	FVector IdealViewOffset = CurrentCamMode->GetViewOffset(P, DeltaTime, ActualCameraOriginRot);
	//`CamDLog("IdealViewOffset out of GetViewOffset is "@IdealViewOffset);

	// get the desired FOV
	OutVT.POV.FOV = eventGetDesiredFOV(P);

	OutVT.POV.Rotation = ActualCameraOriginRot;

	// handle camera turns
	if ( bDoingACameraTurn )
	{
		TurnCurTime += DeltaTime;

		FLOAT TurnInterpPct = (TurnCurTime - TurnDelay) / TurnTotalTime;
		TurnInterpPct = Clamp(TurnInterpPct, 0.f, 1.f);
		if (TurnInterpPct == 1.f)
		{
			// turn is finished!
			EndTurn();
		}

		// swing as a square, feels better for 180s
		FLOAT TurnAngle = FInterpEaseInOut(TurnStartAngle, TurnEndAngle, TurnInterpPct, 2.f);

		// rotate view orient
		OutVT.POV.Rotation.Yaw += appTrunc(TurnAngle);
		LastPostCamTurnYaw = OutVT.POV.Rotation.Yaw;
	}

	//
	// View relative offset
	//

	// Interpolate FOV
	FLOAT FOVBlendTime = CurrentCamMode->GetFOVBlendTime(P);
	if( !bResetCameraInterpolation && FOVBlendTime > 0.f )
	{
		FLOAT InterpSpeed = 1.f / FOVBlendTime;
		OutVT.POV.FOV = FInterpTo(LastCamFOV, OutVT.POV.FOV, DeltaTime, InterpSpeed);
	}
	LastCamFOV = OutVT.POV.FOV;

	// View relative offset.
	FVector ActualViewOffset;
	{
		FLOAT BlendTime = CurrentCamMode->GetBlendTime(P);
		if( !bResetCameraInterpolation && BlendTime > 0.f )
		{
			FLOAT InterpSpeed = 1.f / BlendTime;
			ActualViewOffset = VInterpTo(LastViewOffset, IdealViewOffset, DeltaTime, InterpSpeed);
		}
		else
		{
			ActualViewOffset = IdealViewOffset;
		}
		LastViewOffset = ActualViewOffset;
		//`CamDLog("ActualViewOffset post interp is "@ActualViewOffset);
	}

	// dealing with special optional behaviors
	if (!bDoingACameraTurn)
	{
		// are we in direct look mode?
		UBOOL bDirectLook = CurrentCamMode->UseDirectLookMode(P);	
		if ( bDirectLook )
		{
			// the 50 is arbitrary, but any real motion is way above this
			UBOOL const bMoving = (P->Velocity.SizeSquared() > 50.f) ? TRUE : FALSE;
			FRotator BaseRot = (bMoving) ? P->Velocity.Rotation() : P->Rotation;

			if ( (DirectLookYaw != 0.f) || bDoingDirectLook )
			{
				// new goal rot
				BaseRot.Yaw = FRotator::NormalizeAxis(BaseRot.Yaw + DirectLookYaw);
				OutVT.POV.Rotation = RInterpTo(OutVT.POV.Rotation, BaseRot, DeltaTime, DirectLookInterpSpeed);

				if (DirectLookYaw == 0.f)
				{
					INT const StopDirectLookThresh = bMoving ? 1000 : 50;

					// interpolating out of direct look
					if ( Abs(OutVT.POV.Rotation.Yaw - BaseRot.Yaw) < StopDirectLookThresh )
					{
						// and we're done!
						bDoingDirectLook = FALSE;
					}
				}
				else
				{
					bDoingDirectLook = TRUE;
				}
			}
		}

		UBOOL bLockedToViewTarget = CurrentCamMode->LockedToViewTarget(P);
		if ( !bLockedToViewTarget )
		{
			FLOAT PitchInterpSpeed, YawInterpSpeed, RollInterpSpeed;
			// handle following if necessary
			if ( (P->Velocity.SizeSquared() > 50.f) &&
				CurrentCamMode->ShouldFollowTarget(P, PitchInterpSpeed, YawInterpSpeed, RollInterpSpeed) )
			{
				FLOAT Scale;
				if (CurrentCamMode->FollowingCameraVelThreshold > 0.f)
				{
					Scale = Min(1.f, (P->Velocity.Size() / CurrentCamMode->FollowingCameraVelThreshold));
				}
				else
				{
					Scale = 1.f;
				}

				PitchInterpSpeed *= Scale;
				YawInterpSpeed *= Scale;
				RollInterpSpeed *= Scale;

				FRotator const BaseRot = P->Velocity.Rotation();

				// doing this per-axis allows more aggressive pitch tracking, but looser yaw tracking
				OutVT.POV.Rotation = RInterpToWithPerAxisSpeeds(OutVT.POV.Rotation, BaseRot, DeltaTime, PitchInterpSpeed, YawInterpSpeed, RollInterpSpeed);
			}
		}
	}

	// apply viewoffset (in camera space)
	FVector DesiredCamLoc = ActualCameraOrigin + ::TransformLocalToWorld(ActualViewOffset, OutVT.POV.Rotation);

	// try to have a focus point in view
	AdjustToFocusPoint(P, DeltaTime, DesiredCamLoc, OutVT.POV.Rotation);

	// Set new camera position
	OutVT.POV.Location = DesiredCamLoc;

	// cache this up, for potential later use
	LastPreModifierCameraRot = OutVT.POV.Rotation;

	// apply post processing modifiers
	if (PlayerCamera)
	{
		PlayerCamera->ApplyCameraModifiers(DeltaTime, OutVT.POV);
	}

	//
	// find "worst" location, or location we will shoot the penetration tests from
	//
	FVector WorstLocation = CurrentCamMode->eventGetCameraWorstCaseLoc(P);

	// conv to local space for interpolation
	AGearWeaponPawn* WeapPawn = Cast<AGearWeaponPawn>(P);
	AActor* CamSpaceActor = (WeapPawn && WeapPawn->MyVehicle) ? WeapPawn->MyVehicle : P;
	FMatrix CamSpaceToWorld = FRotationTranslationMatrix(CamSpaceActor->Rotation, CamSpaceActor->Location);

	WorstLocation = CamSpaceToWorld.InverseTransformFVectorNoScale(WorstLocation);

	if (!bResetCameraInterpolation)
	{
		WorstLocation = VInterpTo(LastWorstLocationLocal, WorstLocation, DeltaTime, WorstLocInterpSpeed);
	}
	LastWorstLocationLocal = WorstLocation;

	// rotate back to world space
	WorstLocation = CamSpaceToWorld.TransformFVector(WorstLocation);

	//CLOCK_CYCLES(Time);

	//
	// test for penetration
	//
	//GWorld->LineBatcher->DrawPoint(WorstLocation, FColor(225,255,0), 16, SDPG_World);

	// adjust worst location origin to prevent any penetration
	if (CurrentCamMode->bValidateWorstLoc)
	{
		PreventCameraPenetration(P, IdealCameraOrigin, WorstLocation, DeltaTime, WorstLocBlockedPct, WorstLocPenetrationExtentScale, TRUE);
	}
	else
	{
		WorstLocBlockedPct = 0.f;
	}

	//DrawDebugSphere(WorstLocation, 16, 10, 255, 255, 0, FALSE);

	// adjust final desired camera location, to again, prevent any penetration
	if(!CurrentCamMode->bSkipCameraCollision)
	{
		UBOOL bSingleRayPenetrationCheck = ( !CurrentCamMode->bDoPredictiveAvoidance || ( WP && WP->CustomCameraAnimProperties.bSingleRayPenetrationOnly && WP->eventIsPlayingCustomCameraAnim()) ) ? TRUE : FALSE;
		PreventCameraPenetration(P, WorstLocation, OutVT.POV.Location, DeltaTime, PenetrationBlockedPct, PenetrationExtentScale, bSingleRayPenetrationCheck);
	}

	//UNCLOCK_CYCLES(Time);
	//`log("preventcamerapenetration time:"@Time);
	if ( PlayerCamera && PlayerCamera->PCOwner->IsLocalPlayerController() )
	{
		for (APawn *TempP = GWorld->GetWorldInfo()->PawnList; TempP != NULL; TempP = TempP->NextPawn)
		{
			AGearPawn* TempGP = Cast<AGearPawn>(TempP);
			AGearVehicle* TempGV = Cast<AGearVehicle>(TempP);
			if( TempGP != NULL || TempGV != NULL )
			{
				if( !TempP->IsAliveAndWell() )
				{
					// if not alive, we must'nt hide!
					PlayerCamera->RemoveGearPawnFromHiddenActorsArray(TempP);
				}
				else
				{
					UBOOL bInsideTargetCylinder = FALSE;
					FLOAT CylRadSq, HalfCylHeight;
					if( TempGP != NULL )
					{
						TempGP->GetCameraNoRenderCylinder(CylRadSq, HalfCylHeight, (TempGP==EffectiveWP), PlayerCamera->PCOwner->HiddenActors.ContainsItem(TempP));
					}
					else
					{
						TempGV->GetCameraNoRenderCylinder(CylRadSq, HalfCylHeight, FALSE, PlayerCamera->PCOwner->HiddenActors.ContainsItem(TempP));
					}

					
					CylRadSq *= CylRadSq;

					FVector TempPEffectiveLoc = TempP->Location;
					if( TempGP != NULL )
					{
						FVector PelvisWorldPos;

						switch (TempGP->CoverAction)
						{
							case CA_LeanRight:
							case CA_LeanLeft:
							// follow xy of pelvis
							{
								FVector PelvisWorldPos = TempGP->Mesh->GetBoneLocation(TempGP->NeckBoneName);
								TempPEffectiveLoc.X = PelvisWorldPos.X;
								TempPEffectiveLoc.Y = PelvisWorldPos.Y;
							}
						}

					}

					if ( (OutVT.POV.Location - TempPEffectiveLoc).SizeSquared2D() < CylRadSq )
					{
						FLOAT const CylMaxZ = TempPEffectiveLoc.Z + HalfCylHeight;
						FLOAT const CylMinZ = TempPEffectiveLoc.Z - HalfCylHeight;

						if ( (OutVT.POV.Location.Z < CylMaxZ) && (OutVT.POV.Location.Z > CylMinZ) )
						{
							bInsideTargetCylinder = TRUE;
						}
					}

					// only want to see this when transition state, not every frame
					if (bInsideTargetCylinder)
					{
						PlayerCamera->AddGearPawnToHiddenActorsArray(TempP);
					}
					else
					{
						PlayerCamera->RemoveGearPawnFromHiddenActorsArray(TempP);
					}

					// debug
					//{
					//	FVector CylTop = TempWPEffectiveLoc;
					//	CylTop.Z += HalfCylHeight;
					//	FVector CylBottom = TempWPEffectiveLoc;
					//	CylBottom.Z -= HalfCylHeight;
					//	DrawDebugCylinder(CylTop, CylBottom, appSqrt(CylRadSq), 16, 255, 255, 0, FALSE);
					//}
				}
			}
		}

		//`log("data"@WP@CylRadSq@HalfCylHeight@CylMaxZ@CylMinZ@"camloc"@OutVT.POV.Location@VSizeSq2D(OutVT.POV.Location - WP.Location));
	}
}


/**
* Adjust Camera location and rotation, to try to have a FocusPoint (point in world space) in view.
* Also supports blending in and out from that adjusted location/rotation.
*
* @param	P			Gear Pawn currently being viewed.
* @param	DeltaTime	seconds since last rendered frame.
* @param	CamLoc		(out) cam location
* @param	CamRot		(out) cam rotation
*/
void UGearGameplayCamera::AdjustToFocusPoint(APawn* P, FLOAT DeltaTime, FVector& CamLoc, FRotator& CamRot)
{
	UBOOL bProcessedFocusPoint = FALSE;

	AWorldInfo* const WorldInfo = GWorld->GetWorldInfo();

	if (FocusPoint.bAdjustCamera)
	{
		if (LeftoverPitchAdjustment > 0.f)
		{
			CamRot.Pitch += appTrunc(LeftoverPitchAdjustment);
		}
		LeftoverPitchAdjustment = 0.f;

		// this function is only for use if this is false
		return;
	}

	// Adjust Blend interpolation speed
	FLOAT InterpSpeed;
	if( bFocusPointSet )
	{
		// Keep track if focus point changes, to slow down interpolation speed
		if( (ActualFocusPointWorldLoc - LastFocusPointLoc).SizeSquared() > 1.f )
		{
			LastFocusChangeTime = WorldInfo->TimeSeconds;
			LastFocusPointLoc	= ActualFocusPointWorldLoc;
		}

		// Blend from min to max speed, since focus point last moved, over Focus_FastAdjustKickInTime seconds.
		if( (WorldInfo->TimeSeconds - LastFocusChangeTime) > Focus_FastAdjustKickInTime )
		{
			InterpSpeed = FocusPoint.InterpSpeedRange.Y;
		}
		else
		{
			InterpSpeed = Lerp(FocusPoint.InterpSpeedRange.X, FocusPoint.InterpSpeedRange.Y, FPctByRange(WorldInfo->TimeSeconds-LastFocusChangeTime, 0, Focus_FastAdjustKickInTime));
		}
	}
	else
	{
		// Blend out at max speed
		InterpSpeed = FocusPoint.InterpSpeedRange.Y;
	}

	// If we have a valid focus point, try to have in view.
	if( bFocusPointSet &&
		(FocusPoint.bAlwaysFocus || ( (CamRot.Vector() | (ActualFocusPointWorldLoc - CamLoc)) > 0.f )) )
	{
		INT nbTries = 0;
		FLOAT HeightOffset = 0.f;

		if( !FocusPoint.bIgnoreTrace )
		{
			// Move camera up and back to try to have focus point in view
			for( nbTries = 0; nbTries < Focus_MaxTries; nbTries++ )
			{
				// step up and back, step by step, and trace to see if any geometry is blocking our view
				HeightOffset	= Focus_StepHeightAdjustment * nbTries;
				FVector tmpLoc	= CamLoc;
				tmpLoc.Z		+= HeightOffset;

				FVector LocalOffset(0.f);
				LocalOffset.X = -HeightOffset * Focus_BackOffStrength;
				tmpLoc += ::TransformLocalToWorld(LocalOffset, CamRot);				// note: was worldtolocal, bug?

				// basically a fasttrace
				FCheckResult Hit(1.f);
				GWorld->SingleLineCheck( Hit, P, ActualFocusPointWorldLoc, tmpLoc, TRACE_World|TRACE_StopAtAnyHit, FVector(0.f) );

				if (Hit.Actor == NULL)
				{
					break;
				}
			}
		}

		// if we can successfully view focus point, then adjust Camera so focus point is in FOV
		if( nbTries < Focus_MaxTries )
		{
			bProcessedFocusPoint = TRUE;

			// Camera Location
			LastHeightAdjustment = FInterpTo( LastHeightAdjustment, HeightOffset, DeltaTime, InterpSpeed );
			CamLoc.Z += LastHeightAdjustment;
			FVector LocalOffset(0.f);
			LocalOffset.X = -LastHeightAdjustment * Focus_BackOffStrength;
			CamLoc += ::TransformLocalToWorld(LocalOffset, CamRot);

			// ADJUST YAW
			// Get the look direction, ignoring Z
			FVector LookDir	= CamRot.Vector();
			LookDir.Z = 0.f;
			LookDir.Normalize();

			// Save final camera location as current location
			FVector FinalCamLoc = CamLoc;

			FVector DesiredLookDir = ActualFocusPointWorldLoc - FinalCamLoc;
			DesiredLookDir.Z = 0.f;
			DesiredLookDir.Normalize();

			// Get world space angles for both look direction and desired direction
			FLOAT const CurrentHeading = GetHeadingAngle( LookDir );
			FLOAT const DesiredHeading = GetHeadingAngle( DesiredLookDir );

			// Get the change in angles between current and target and convert the radians angle to Unreal Rotator Units
			FLOAT DeltaTarget = int(RadToUnrRotatorUnits(FindDeltaAngle( CurrentHeading, DesiredHeading )));

			// If not within the FOV
			if( Abs(DeltaTarget) > DegToUnrRotatorUnits(FocusPoint.InFocusFOV.X) )
			{
				// Snap Delta to FOV limit
				DeltaTarget = DeltaTarget - DeltaTarget * Abs(DegToUnrRotatorUnits(FocusPoint.InFocusFOV.X) / DeltaTarget);
				// Interpolate from last update position to delta target - saving the new adjustment for next time
				if (!bResetCameraInterpolation)
				{
					LastYawAdjustment = FInterpTo( LastYawAdjustment, DeltaTarget, DeltaTime, InterpSpeed );
				}
				else
				{
					LastYawAdjustment = DeltaTarget;
				}
			}
			// Adjust camera rotation
			CamRot.Yaw += appTrunc(LastYawAdjustment);

			// ADJUST PITCH
			// (Different because we don't have to worry about camera wrapping around)
			// Find change in pitch
			//FLOAT DesiredPitch = (FLOAT)(FRotator::NormalizeAxis((ActualFocusPointWorldLoc - CamLoc).Rotation().Pitch));
			INT DesiredPitch = (ActualFocusPointWorldLoc - CamLoc).Rotation().Pitch + appTrunc(DegToUnrRotatorUnits(FocusPoint.FocusPitchOffsetDeg));
			DeltaTarget = (FLOAT) FRotator::NormalizeAxis(DesiredPitch - CamRot.Pitch);
			if( Abs(DeltaTarget) > DegToUnrRotatorUnits(FocusPoint.InFocusFOV.Y) )
			{
				// Snap Delta to FOV limit
				DeltaTarget = DeltaTarget - DeltaTarget * Abs(DegToUnrRotatorUnits(FocusPoint.InFocusFOV.Y) / DeltaTarget);
				// Interpolate from last pitch adjustment to desired delta - saving the new adjustment for next time
				if (!bResetCameraInterpolation)
				{
					LastPitchAdjustment	= FInterpTo( LastPitchAdjustment, DeltaTarget, DeltaTime, InterpSpeed );
				}
				else
				{
					LastPitchAdjustment = DeltaTarget;
				}
			}

			// Adjust camera rotation
			CamRot.Pitch += appTrunc(LastPitchAdjustment);
		}
	}

	// if we're not viewing a focus point, blend out smoothly at Max interp speed.
	if( !bProcessedFocusPoint )
	{
		FLOAT Zero = 0.f;

		// blend out of vertical offset adjustment
		if( LastHeightAdjustment != 0 )
		{
			if (!bResetCameraInterpolation)
			{
				LastHeightAdjustment	 = FInterpTo( LastHeightAdjustment, Zero, DeltaTime, InterpSpeed );

				CamLoc.Z += LastHeightAdjustment;

				FVector LocalOffset(0.f);
				LocalOffset.X = -LastHeightAdjustment * Focus_BackOffStrength;
				CamLoc += ::TransformLocalToWorld(LocalOffset, CamRot);
			}
			else
			{
				LastHeightAdjustment = 0;
			}
		}

		// blend out of pitch adjustment
		if( LastPitchAdjustment != 0 )
		{
			if (!bResetCameraInterpolation)
			{
				LastPitchAdjustment  = FInterpTo( LastPitchAdjustment, Zero, DeltaTime, InterpSpeed );
				CamRot.Pitch		+= appTrunc(LastPitchAdjustment);
			}
			else
			{
				LastPitchAdjustment = 0;
			}
		}

		// blend out of yaw adjustment
		if( LastYawAdjustment != 0 )
		{
			if (!bResetCameraInterpolation)
			{
				LastYawAdjustment  = FInterpTo( LastYawAdjustment, Zero, DeltaTime, InterpSpeed );
				CamRot.Yaw		+= appTrunc(LastYawAdjustment);
			}
			else
			{
				LastYawAdjustment = 0;
			}
		}
	}

	bFocusPointSuccessful = bProcessedFocusPoint;
}

/**
* This is a simplified version of AdjustToFocusPoint that keeps the camera target in frame.
* CamLoc passed in should be the camera origin, before the view offset is applied.
*/
void UGearGameplayCamera::AdjustToFocusPointKeepingTargetInView(APawn* P, FLOAT DeltaTime, FVector& CamLoc, FRotator& CamRot, const FVector& ViewOffset)
{
	// this function is only for use if this is true
	if (P && FocusPoint.bAdjustCamera)
	{
		FVector EffectiveFocusLoc;
		UBOOL bProcessedFocusPoint = FALSE;

		AWorldInfo* const WorldInfo = GWorld->GetWorldInfo();

		APlayerController* const PC = Cast<APlayerController>(P->Controller);

		CamRot = CamRot.GetNormalized();
		FRotator const OriginalCamRot = CamRot;

		// Adjust Blend interpolation speed
		FLOAT InterpSpeed;
		if( bFocusPointSet )
		{
			// this fudges the focus point to counteract the camera view offset
			// so we end up with the camera pointed at the target, and not the pawn
			// (which doesn't look right, since the camera is laterally offset)
			EffectiveFocusLoc = GetEffectiveFocusLoc(CamLoc, ActualFocusPointWorldLoc, ViewOffset);

			// Keep track if focus point changes, to slow down interpolation speed
			if( (EffectiveFocusLoc - LastFocusPointLoc).SizeSquared() > 1.f )
			{
				LastFocusChangeTime = WorldInfo->TimeSeconds;
				LastFocusPointLoc	= EffectiveFocusLoc;
			}

			// Blend from min to max speed, since focus point last moved, over Focus_FastAdjustKickInTime seconds.
			if( WorldInfo->TimeSeconds - LastFocusChangeTime > Focus_FastAdjustKickInTime )
			{
				InterpSpeed = FocusPoint.InterpSpeedRange.Y;
			}
			else
			{
				InterpSpeed = Lerp(FocusPoint.InterpSpeedRange.X, FocusPoint.InterpSpeedRange.Y, FPctByRange(WorldInfo->TimeSeconds-LastFocusChangeTime, 0, Focus_FastAdjustKickInTime));
			}
		}
		else
		{
			// Blend out at max speed
			InterpSpeed = FocusPoint.InterpSpeedRange.Y;
		}

		// If we have a valid focus point, try to have in view.
		if( bFocusPointSet &&
			(FocusPoint.bAlwaysFocus || ((CamRot.Vector() | (EffectiveFocusLoc - CamLoc)) > 0.f)) )
		{
			bProcessedFocusPoint = TRUE;

			// ADJUST YAW
			// Get the look direction, ignoring Z
			FVector LookDir	= CamRot.Vector();
			LookDir.Z	= 0.f;
			LookDir.Normalize();

			FVector DesiredLookDir = EffectiveFocusLoc - CamLoc;
			DesiredLookDir.Z	= 0;
			DesiredLookDir.Normalize();

			// Get world space angles for both look direction and desired direction
			FLOAT const CurrentHeading = GetHeadingAngle( LookDir );
			FLOAT const DesiredHeading = GetHeadingAngle( DesiredLookDir );

			//`log("calc heading; focus"@ActualFocusPointWorldLoc@"effocus"@EffectiveFocusLoc@"camloc"@CamLoc@"headings"@CurrentHeading@DesiredHeading@"camrot"@CamRot@"LastYP"@LastYawAdjustment@LastPitchAdjustment@LastPostAdjustmentCamRot@"XYZ"@X@Y@Z);
			//DrawDebugSphere(EffectiveFocusLoc, 32, 16, 255, 255, 255);
			//DrawDebugSphere(ActualFocusPointWorldLoc, 32, 16, 255, 255, 128);

			// Get the change in angles between current and target and convert the radians angle to Unreal Rotator Units
			FLOAT DeltaTarget = int(RadToUnrRotatorUnits(FindDeltaAngle(CurrentHeading, DesiredHeading)));

			//`log("DeltaTarget"@DeltaTarget@"headings cur/des"@CurrentHeading@DesiredHeading@"LastYawAdj"@LastYawAdjustment);

			// If not within the FOV
			if( Abs(DeltaTarget) > DegToUnrRotatorUnits(FocusPoint.InFocusFOV.X) )
			{
				// Snap Delta to FOV limit
				DeltaTarget = DeltaTarget - DeltaTarget * Abs(DegToUnrRotatorUnits(FocusPoint.InFocusFOV.X) / DeltaTarget);

				// make sure we're interpolating the shortest way around the circle.
				FLOAT Diff = LastYawAdjustment - DeltaTarget;
				if (Diff > 32768.f)
				{
					LastYawAdjustment -= 65536.f;
				}
				else if (Diff < -32768.f)
				{
					LastYawAdjustment += 65536.f;
				}

				// Interpolate from last update position to delta target - saving the new adjustment for next time
				if (!bResetCameraInterpolation)
				{
					LastYawAdjustment = FInterpTo( LastYawAdjustment, DeltaTarget, DeltaTime, InterpSpeed );
				}
				else
				{
					LastYawAdjustment = DeltaTarget;
				}
				//`log("DeltaTarget adjusted to"@DeltaTarget@"LastYawAdj"@LastYawAdjustment@FocusPoint.InFocusFOV.X);
			}
			// Adjust camera rotation
			CamRot.Yaw += appTrunc(LastYawAdjustment);

			// ADJUST PITCH
			// (Different because we don't have to worry about camera wrapping around)
			// Find change in pitch
			DeltaTarget = FLOAT(FRotator::NormalizeAxis((EffectiveFocusLoc - CamLoc).Rotation().Pitch - CamRot.Pitch));
			if( Abs(DeltaTarget) > DegToUnrRotatorUnits(FocusPoint.InFocusFOV.Y) )
			{
				// Snap Delta to FOV limit
				DeltaTarget = DeltaTarget - DeltaTarget * Abs(DegToUnrRotatorUnits(FocusPoint.InFocusFOV.Y) / DeltaTarget);
				// Interpolate from last pitch adjustment to desired delta - saving the new adjustment for next time
				if (!bResetCameraInterpolation)
				{
					LastPitchAdjustment	= FInterpTo( LastPitchAdjustment, DeltaTarget, DeltaTime, InterpSpeed );
				}
				else
				{
					LastPitchAdjustment = DeltaTarget;
				}
			}

			// Adjust camera rotation
			CamRot.Pitch += appTrunc(LastPitchAdjustment);

			// done adjusting, now clamp rotation appropriately
			if (PC)
			{
				CamRot = PC->eventLimitViewRotation( CamRot, P->ViewPitchMin, /*P->ViewPitchMax*/Max((FLOAT)OriginalCamRot.Pitch, 5000.f) );
			}

			// recalc Last adjustment vars in case LimitViewRotation changed something.
			LeftoverPitchAdjustment = LastPitchAdjustment - (CamRot.Pitch - OriginalCamRot.Pitch);
			LastYawAdjustment = CamRot.Yaw - OriginalCamRot.Yaw;
		}

		// if we're not viewing a focus point, blend out smoothly at Max interp speed.
		if( !bProcessedFocusPoint )
		{
			FLOAT Zero = 0.f;

			// blend out of pitch adjustment
			if( LastPitchAdjustment != 0 )
			{
				if (!bResetCameraInterpolation)
				{
					LastPitchAdjustment  = FInterpTo( LastPitchAdjustment, Zero, DeltaTime, InterpSpeed );
					CamRot.Pitch		+= appTrunc(LastPitchAdjustment);
				}
				else
				{
					LastPitchAdjustment = 0.f;
				}

				if (PC)
				{
					CamRot = PC->eventLimitViewRotation( CamRot, P->ViewPitchMin, /*P->ViewPitchMax*/Max((FLOAT)OriginalCamRot.Pitch, 5000.f) );
				}
				LeftoverPitchAdjustment = LastPitchAdjustment - (CamRot.Pitch - OriginalCamRot.Pitch);
			}

			// blend out of yaw adjustment
			if( LastYawAdjustment != 0.f )
			{
				if (!bResetCameraInterpolation)
				{
					LastYawAdjustment  = FInterpTo( LastYawAdjustment, Zero, DeltaTime, InterpSpeed );
					CamRot.Yaw		+= appTrunc(LastYawAdjustment);
				}
				else
				{
					LastYawAdjustment = 0.f;
				}
			}
		}

		bFocusPointSuccessful = bProcessedFocusPoint;
	}
}

/**
* Stops a camera rotation.
*/
void UGearGameplayCamera::EndTurn()
{
	bDoingACameraTurn = FALSE;

	// maybe align
	if (bTurnAlignTargetWhenFinished)
	{
		if (PlayerCamera && PlayerCamera->PCOwner)
		{
			FRotator TmpRot = PlayerCamera->PCOwner->Rotation;
			TmpRot.Yaw = LastPostCamTurnYaw;
			PlayerCamera->PCOwner->SetRotation(TmpRot);
		}
	}
}

/**
* Returns desired camera fov.
*/
//FLOAT AGearPlayerCamera::GetGearCamFOV(APawn* CameraTargetPawn)
//{
//	FLOAT FOV = bUseForcedCamFOV ? ForcedCamFOV : FOV = CurrentCamMode->eventGetDesiredFOV(CameraTargetPawn);
//
//	// adjust FOV for splitscreen, 4:3, whatever
//	FOV = AdjustFOVForViewport(FOV, CameraTargetPawn);
//
//	return FOV;
//}

/** Add a pawn from the hidden actor's array */
void AGearPlayerCamera::AddGearPawnToHiddenActorsArray( APawn *PawnToHide )
{
	if ( PawnToHide )
	{
		PCOwner->HiddenActors.AddItem(PawnToHide);
		AGearPawn* GP = Cast<AGearPawn>(PawnToHide);
		if( GP != NULL )
		{
			GP->bIsHiddenByCamera = TRUE;
		}
		else
		{
			AGearVehicle* GV = Cast<AGearVehicle>(PawnToHide);
			if( GV != NULL )
			{
				GV->bIsHiddenByCamera = TRUE;
			}
		}		
	}
}

/** Remove a pawn from the hidden actor's array */
void AGearPlayerCamera::RemoveGearPawnFromHiddenActorsArray( APawn *PawnToShow )
{
	if ( PawnToShow )
	{
		PCOwner->HiddenActors.RemoveItem(PawnToShow);
		AGearPawn* GP = Cast<AGearPawn>(PawnToShow);
		if( GP != NULL )
		{
			GP->bIsHiddenByCamera = FALSE;
		}
		else
		{
			AGearVehicle* GV = Cast<AGearVehicle>(PawnToShow);
			if( GV != NULL )
			{
				GV->bIsHiddenByCamera = FALSE;
			}
		}		
	}
}


void AGearPlayerCamera::ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const
{
	Super::ModifyPostProcessSettings(PPSettings);

	// allow the current gearcamera to do what it wants
	if (CurrentCamera)
	{
		CurrentCamera->eventModifyPostProcessSettings(PPSettings);
	}
}

// in native code so we can use LocalToWorld()
void AGearPlayerCamera::CacheLastTargetBaseInfo(AActor* Base)
{
	LastTargetBase = Base;
	if (Base)
	{
		LastTargetBaseTM = Base->LocalToWorld();
	}
}



/*-----------------------------------------------------------------------------
	UGearCamMod_ScreenShake
-----------------------------------------------------------------------------*/

void UGearCamMod_ScreenShake::UpdateScreenShake(FLOAT DeltaTime, FScreenShakeStruct& Shake, FTPOV& OutPOV)
{
	Shake.TimeToGo -= DeltaTime;

	// Do not update screen shake if not needed
	if( Shake.TimeToGo <= 0.f )
	{
		return;
	}

	// Smooth fade out
	FLOAT ShakePct = Clamp<FLOAT>(Shake.TimeToGo / Shake.TimeDuration, 0.f, 1.f);
	FLOAT Scale = Alpha;

	
	if( CameraOwner && CameraOwner->PCOwner)
	{
		AGearPawn* WP = Cast<AGearPawn>(CameraOwner->PCOwner->Pawn);
		if( WP )
		{
			// is alpha getting overridden here?
			if( Shake.bOverrideTargetingDampening && WP->bIsTargeting )
			{
				Scale = Shake.TargetingDampening;
			}

			// If on a Brumak, dampen the scale if it hasn't been already
			if( Cast<AGearPawn_LocustBrumakBase>(WP)		!= NULL ||
				Cast<AGearPawn_LocustBrumakBase>(WP->Base)	!= NULL )
			{
				Scale = ::Min( Scale, 0.5f );
			}			
		}
	}	

	ShakePct = Scale * ShakePct*ShakePct*(3.f - 2.f*ShakePct);

	// do not update if percentage is null
	if( ShakePct <= 0.f )
	{
		return;
	}

	// View Offset, Compute sin wave value for each component
	if( !Shake.LocAmplitude.IsZero() )
	{
		FVector	LocOffset = FVector(0);
		if( Shake.LocAmplitude.X != 0.0 ) 
		{
			Shake.LocSinOffset.X += DeltaTime * Shake.LocFrequency.X;
			LocOffset.X = Shake.LocAmplitude.X * appSin(Shake.LocSinOffset.X) * ShakePct;
		}
		if( Shake.LocAmplitude.Y != 0.0 ) 
		{
			Shake.LocSinOffset.Y += DeltaTime * Shake.LocFrequency.Y;
			LocOffset.Y = Shake.LocAmplitude.Y * appSin(Shake.LocSinOffset.Y) * ShakePct;
		}
		if( Shake.LocAmplitude.Z != 0.0 ) 
		{
			Shake.LocSinOffset.Z += DeltaTime * Shake.LocFrequency.Z;
			LocOffset.Z = Shake.LocAmplitude.Z * appSin(Shake.LocSinOffset.Z) * ShakePct;
		}

		// Offset is relative to camera orientation
		FRotationMatrix CamRotMatrix(OutPOV.Rotation);
		OutPOV.Location += LocOffset.X * CamRotMatrix.GetAxis(0) + LocOffset.Y * CamRotMatrix.GetAxis(1) + LocOffset.Z * CamRotMatrix.GetAxis(2);
	}

	// View Rotation, compute sin wave value for each component
	if( !Shake.RotAmplitude.IsZero() )
	{
		FVector	RotOffset = FVector(0);
		if( Shake.RotAmplitude.X != 0.0 ) 
		{
			Shake.RotSinOffset.X += DeltaTime * Shake.RotFrequency.X;
			RotOffset.X = Shake.RotAmplitude.X * appSin(Shake.RotSinOffset.X);
		}
		if( Shake.RotAmplitude.Y != 0.0 ) 
		{
			Shake.RotSinOffset.Y += DeltaTime * Shake.RotFrequency.Y;
			RotOffset.Y = Shake.RotAmplitude.Y * appSin(Shake.RotSinOffset.Y);
		}
		if( Shake.RotAmplitude.Z != 0.0 ) 
		{
			Shake.RotSinOffset.Z += DeltaTime * Shake.RotFrequency.Z;
			RotOffset.Z = Shake.RotAmplitude.Z * appSin(Shake.RotSinOffset.Z);
		}
		RotOffset				*= ShakePct;
		OutPOV.Rotation.Pitch	+= appTrunc(RotOffset.X);
		OutPOV.Rotation.Yaw		+= appTrunc(RotOffset.Y);
		OutPOV.Rotation.Roll	+= appTrunc(RotOffset.Z);
	}

	// Compute FOV change
	if( Shake.FOVAmplitude != 0.0 ) 
	{
		Shake.FOVSinOffset	+= DeltaTime * Shake.FOVFrequency;
		OutPOV.FOV			+= ShakePct * Shake.FOVAmplitude * appSin(Shake.FOVSinOffset);
	}
}

UBOOL UGearCamMod_ScreenShake::ModifyCamera(class ACamera* Camera,FLOAT DeltaTime,FTPOV& OutPOV)
{
	// Update the alpha
	UpdateAlpha(Camera, DeltaTime);

	// Call super where modifier may be disabled
	Super::ModifyCamera(Camera, DeltaTime, OutPOV);

	// If no alpha, exit early
	if( Alpha <= 0.f )
	{
		return FALSE;
	}

	// Update Screen Shakes array
	if( Shakes.Num() > 0 )
	{
		for(INT i=0; i<Shakes.Num(); i++)
		{
			UpdateScreenShake(DeltaTime, Shakes(i), OutPOV);
		}

		// Delete any obsolete shakes
		for(INT i=Shakes.Num()-1; i>=0; i--)
		{
			if( Shakes(i).TimeToGo <= 0 )
			{
				Shakes.Remove(i,1);
			}
		}
	}
	// Update Test Shake
	UpdateScreenShake(DeltaTime, TestShake, OutPOV);

	if (Camera && Camera->ViewTarget.Target)
	{
		// Update Ambient Shake Volumes
		for (INT i = 0; i < Camera->ViewTarget.Target->Touching.Num(); ++i)
		{
			AActor* TestActor = Camera->ViewTarget.Target->Touching(i);
			if(	TestActor &&
				!TestActor->bDeleteMe &&
				TestActor->IsA(AAmbientShakeVolume::StaticClass()) )
			{
				AAmbientShakeVolume * ShakeVolume = CastChecked<AAmbientShakeVolume>(Camera->ViewTarget.Target->Touching(i));
				if( ShakeVolume->bEnableShake )
				{
					// Set TimeToGo, so it ends up scaling shake by 1x
					ShakeVolume->AmbientShake.TimeToGo = ShakeVolume->AmbientShake.TimeDuration + DeltaTime;
					// Update ambient shake
					UpdateScreenShake(DeltaTime, ShakeVolume->AmbientShake, OutPOV);
				}
			}
		}
	}


	// If ModifyCamera returns true, exit loop
	// Allows high priority things to dictate if they are
	// the last modifier to be applied
	// Returning TRUE causes to stop adding another modifier! 
	// Returning FALSE is the right behavior since this is not high priority modifier.
	return FALSE;
}

FLOAT UGearCamMod_ScreenShake::GetTargetAlpha(class ACamera * Camera)
{

	FLOAT SuperAlpha = Super::GetTargetAlpha(Camera);
	FLOAT ThisAlpha = 1.f;

	if (SuperAlpha > 0.f)
	{
		AGearPawn* WP = Cast<AGearPawn>(CameraOwner->PCOwner->Pawn);
		if  (WP != NULL)
		{
			if (WP->bIsTargeting)
			{
				ThisAlpha = TargetingAlpha;
			}
		}
	}

	return Min<FLOAT>(SuperAlpha, ThisAlpha);
}

/*-----------------------------------------------------------------------------
	UGameplayCam_Vehicle
-----------------------------------------------------------------------------*/

/** Returns true if mode should lock camera to view target, false otherwise */
UBOOL UGameplayCam_Vehicle::LockedToViewTarget(APawn* CameraTarget)
{
	if (CameraTarget)
	{
		AGearPC* const WPC = Cast<AGearPC>(CameraTarget->Controller);

		if ( WPC && WPC->bAssessMode )
		{
			// lock to target when using tac/com
			return TRUE;
		}
	}
	
	return bLockedToViewTarget;
}


/** Returns true if mode should be using direct-look mode, false otherwise */
UBOOL UGameplayCam_Vehicle::UseDirectLookMode(APawn* CameraTarget)
{
	if (CameraTarget)
	{
		AGearPC* const WPC = Cast<AGearPC>(CameraTarget->Controller);

		if ( WPC && WPC->bAssessMode )
		{
			// lock to target when using tac/com
			return FALSE;
		}
	}

	return bDirectLook;
}


/**
* Returns true if this mode should do target following.  If true is returned, interp speeds are filled in.
* If false is returned, interp speeds are not altered.
*/
UBOOL UGameplayCam_Vehicle::ShouldFollowTarget(APawn* CameraTarget, FLOAT& PitchInterpSpeed, FLOAT& YawInterpSpeed, FLOAT& RollInterpSpeed)
{
	if (LockedToViewTarget(CameraTarget))
	{
		// no following when locked
		return FALSE;
	}

	AGearPC* const WPC = Cast<AGearPC>(CameraTarget->Controller);
	if ( WPC && WPC->bAssessMode )
	{
		// no following when using tac/com
		return FALSE;
	}

	if (bFollowTarget)
	{
		PitchInterpSpeed = FollowingInterpSpeed_Pitch;
		YawInterpSpeed = FollowingInterpSpeed_Yaw;
		RollInterpSpeed = FollowingInterpSpeed_Roll;
		return TRUE;
	}

	return FALSE;
}

/**  */
void UGameplayCam_Vehicle::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	INT SeatIndex = 0;
	AGearVehicle* WV = Cast<AGearVehicle>(TargetPawn);
	if (WV == NULL && TargetPawn != NULL)
	{
		AGearWeaponPawn* WP = Cast<AGearWeaponPawn>(TargetPawn);
		if (WP != NULL)
		{
			WV = WP->MyVehicle;
			SeatIndex = WP->MySeatIndex;
		}
	}
	if (WV != NULL)
	{
		OriginLoc = WV->eventGetCameraStart(SeatIndex);
	}
}


/** returns View relative offsets */
void UGameplayCam_Vehicle::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	INT SeatIndex = 0;
	AGearVehicle* WV = Cast<AGearVehicle>(ViewedPawn);
	if (WV == NULL && ViewedPawn != NULL)
	{
		// Get seat index
		AGearWeaponPawn* WP = Cast<AGearWeaponPawn>(ViewedPawn);
		if(WP)
		{
			SeatIndex = WP->MySeatIndex;
		}

		WV = Cast<AGearVehicle>(ViewedPawn->GetVehicleBase());
	}

	UBOOL bSplitscreen = (ViewedPawn && ViewedPawn->PlayerReplicationInfo && ViewedPawn->PlayerReplicationInfo->SplitscreenIndex != -1);

	if (WV != NULL)
	{
		// return the parameters stored in the vehicle itself
		check(SeatIndex < WV->Seats.Num());
		WV->eventGetVehicleViewOffsets(SeatIndex, bSplitscreen, out_Low, out_Mid, out_High);
	}
	else
	{
		// shouldn't get here, but fallback to base functionality just in case
		debugf(TEXT("could not retrieve vehicle camera data, falling back to defaults"));
		out_Low		= ViewOffset.OffsetLow;
		out_Mid		= ViewOffset.OffsetMid;
		out_High	= ViewOffset.OffsetHigh;
	}
}

/*-----------------------------------------------------------------------------
	UGameplayCam_Turret
-----------------------------------------------------------------------------*/

void UGameplayCam_Turret::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	AGearTurret* const WT = Cast<AGearTurret>(ViewedPawn);

	if (WT)
	{
		// return the parameters stored in the turret itself
		out_Low		= WT->CameraViewOffsetLow;
		out_Mid 	= WT->CameraViewOffsetMid;
		out_High	= WT->CameraViewOffsetHigh;
	}
	else
	{
		// can get here if we're in turret mode but not attached to a turret (e.g heavy weapons)
		out_Low		= ViewOffset.OffsetLow;
		out_Mid		= ViewOffset.OffsetMid;
		out_High	= ViewOffset.OffsetHigh;
	}
}

/*-----------------------------------------------------------------------------
	UGameplayCam_TurretTargeting
-----------------------------------------------------------------------------*/


/** Returns View relative offsets */
void UGameplayCam_TurretTargeting::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	AGearTurret* const WT = Cast<AGearTurret>(ViewedPawn);

	if (WT)
	{
		// return the parameters stored in the turret itself
		out_Low		= WT->CameraTargetingViewOffsetLow;
		out_Mid 	= WT->CameraTargetingViewOffsetMid;
		out_High	= WT->CameraTargetingViewOffsetHigh;
	}
	else
	{
		// fall back to nontargeting version
		Super::GetBaseViewOffsets(ViewedPawn, ViewportConfig, DeltaTime, out_Low, out_Mid, out_High);
	}
}

/*-----------------------------------------------------------------------------
	UGameplayCam_VehicleTurret
-----------------------------------------------------------------------------*/

/** Returns View relative offsets */
void UGameplayCam_VehicleTurret::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	AGearWeaponPawn* const WWP = Cast<AGearWeaponPawn>(ViewedPawn);
	AGearVehicle* const WV = WWP ? WWP->MyVehicle : NULL;

	if (WWP && WWP->MyVehicle)
	{
		// return the parameters stored in the vehicle itself
		// @fixme, hardcoded seat, this works but is brittle and eeeevil.
		FVehicleSeat& Seat = WWP->MyVehicle->Seats(1);
		out_Low		= Seat.CameraViewOffsetLow;
		out_Mid 	= Seat.CameraViewOffsetMid;
		out_High	= Seat.CameraViewOffsetHigh;
	}
	else
	{
		// shouldn't get here, but fallback to base functionality just in case
		debugf(TEXT("could not retrieve vehicle camera data, falling back to defaults"));
		out_Low		= ViewOffset.OffsetLow;
		out_Mid		= ViewOffset.OffsetMid;
		out_High	= ViewOffset.OffsetHigh;
	}
}


/*-----------------------------------------------------------------------------
	UGameplayCam_VehicleTurretTargeting
-----------------------------------------------------------------------------*/

void UGameplayCam_VehicleTurretTargeting::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	// @note: it would be better, longer term, to store these offsets in the vehicle or the warweaponPawn-> 
	// but for gears 1, since there's only 1 vehicle, I'm using these for expediency
	out_Low		= ViewOffset.OffsetLow;
	out_Mid		= ViewOffset.OffsetMid;
	out_High	= ViewOffset.OffsetHigh;
}

/*-----------------------------------------------------------------------------
	UGameplayCam_Vehicle_RideReaver
-----------------------------------------------------------------------------*/

FLOAT UGameplayCam_Vehicle_RideReaver::GetFOVBlendTime(class APawn* Pawn)
{
	return FOVBlendTime;
}

/** Returns time to interpolate location/rotation changes. */
FLOAT UGameplayCam_Vehicle_RideReaver::GetBlendTime(class APawn* Pawn)
{
	if(Pawn)
	{
		AVehicle_RideReaver_Base* Reaver = Cast<AVehicle_RideReaver_Base>(Pawn);
		if(!Reaver)
		{
			AGearWeaponPawn* WP = Cast<AGearWeaponPawn>(Pawn);
			if(WP)
			{
				Reaver = Cast<AVehicle_RideReaver_Base>(WP->MyVehicle);
			}
		}

		if(Reaver)
		{
			FLOAT Amount = Reaver->WeapTransitionRemaining/Reaver->WeaponTransitionCamBlendDuration;
			UGameplayCam_Vehicle_RideReaver* Default = CastChecked<UGameplayCam_Vehicle_RideReaver>(GetClass()->GetDefaultObject());
			FLOAT CurrentBlendTime = Lerp(Default->BlendTime, Reaver->WeaponTransitionCamTranslationBlendTime, Amount);

			return CurrentBlendTime;
		}
	}

	return BlendTime;
}

void UGameplayCam_Vehicle_RideReaver::GetBaseViewOffsets( APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High )
{
	Super::GetBaseViewOffsets( ViewedPawn, ViewportConfig, DeltaTime, out_Low, out_Mid, out_High );

	AVehicle_RideReaver_Base* RR = Cast<AVehicle_RideReaver_Base>(ViewedPawn);
	if (!RR)
	{
		AGearWeaponPawn* WP = Cast<AGearWeaponPawn>(ViewedPawn);
		if (WP)
		{
			RR = Cast<AVehicle_RideReaver_Base>(WP->MyVehicle);
		}
	}

	if( RR != NULL && RR->bPlayLanding )
	{
		out_Low.Z  += OnLandAdjustZ;
		out_Mid.Z  += OnLandAdjustZ;
		out_High.Z += OnLandAdjustZ;
	}
}

void UGameplayCam_Vehicle_RideReaver::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin( TargetPawn, OriginLoc, OriginRot );

	AVehicle_RideReaver_Base* RR = Cast<AVehicle_RideReaver_Base>(TargetPawn);
	if (!RR)
	{
		AGearWeaponPawn* WP = Cast<AGearWeaponPawn>(TargetPawn);
		if (WP)
		{
			RR = Cast<AVehicle_RideReaver_Base>(WP->MyVehicle);
		}
	}

	if( RR != NULL && !RR->bPlayLanding && !RR->LastOrigInterpLocation.IsNearlyZero() )
	{
		FVector FullMoveLoc = RR->LastOrigInterpLocation + (OriginLoc - RR->Location);
		OriginLoc = Lerp(OriginLoc, FullMoveLoc, RR->DodgeCamAmount);
	}
}

/*-----------------------------------------------------------------------------
	UGameplayCam_CoverTargeting
-----------------------------------------------------------------------------*/

void UGameplayCam_CoverTargeting::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	UGameplayCam_CoverTargeting* DefaultClass = CastChecked<UGameplayCam_CoverTargeting>(GetClass()->GetDefaultObject());

	// Reset to default
	if( DefaultClass )
	{
		InterpLocSpeed = DefaultClass->InterpLocSpeed;
	}
	
	AGearPawn* GP = Cast<AGearPawn>(TargetPawn);
	if( GP )
	{
		// Transition from 360 aiming to leaning
		if( GP->IsDoing360ToLeaningTransition() || GP->IsDoingLeaningTo360Transition() )
		{
			InterpLocSpeed = (1.f / LeanTo360AimingBlendTime);
		}
	}
}

FLOAT UGameplayCam_CoverTargeting::GetBlendTime(class APawn* Pawn)
{
	FLOAT NewBlendTime = Super::GetBlendTime(Pawn);

	AGearPawn* GP = Cast<AGearPawn>(Pawn);
	if( GP )
	{
		if( GP->IsDoing360ToLeaningTransition() || GP->IsDoingLeaningTo360Transition() )
		{
			NewBlendTime = LeanTo360AimingBlendTime;
		}
	}

	return NewBlendTime;
}

FVector UGameplayCam_CoverTargeting::GetTargetRelativeOriginOffset(class APawn* TargetPawn)
{
	if (TargetPawn)
	{
		AGearPawn* const WP = Cast<AGearPawn>(TargetPawn);
		if ( WP && ((WP->CoverAction == CA_LeanRight) || (WP->CoverAction == CA_LeanLeft)) )
		{
			if (!WP->bIsMirrored)
			{
				return (WP->CoverType == CT_MidLevel) ? LeanOriginOffsetLow : LeanOriginOffset;
			}
			else
			{
				// just need to invert the Y
				FVector Offset = (WP->CoverType == CT_MidLevel) ? LeanOriginOffsetLow : LeanOriginOffset;
				Offset.Y = -Offset.Y;
				return Offset;
			}
		}
	}

	return FVector(0.f,0.f,0.f);
}


/*-----------------------------------------------------------------------------
	UGameplayCam_Conversation
-----------------------------------------------------------------------------*/
void UGameplayCam_Conversation::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	//// move origin out in front of targetpawn's face a little
	//static FVector LocalOffset(60.f, 0.f, 0.f);
	//OriginLoc += TransformLocalToWorld(LocalOffset, /*OriginRot*/ TargetPawn->Rotation);

	// the goal here is to find a point at given radius from the TargetPawn, such that a line from 
	// the speaker target to this point is tangent to the circle described by that radius.

	// IOW, imagine a circle around the TargetPawm.  The CameraOrigin will always be on that circle,
	// such that the apparent distance from the pawn is maximized.

	/** This code is set to be deleted due to design changes in the system.  Will check in for perforce posterity though, since it seemed to work fairly well. */

#if 0
	AActor* const FocusActor = GameplayCam->FocusPoint.FocusActor;
	if (FocusActor && TargetPawn)
	{
		FVector FocusActorToTargetPawn = TargetPawn->Location - FocusActor->Location;
		FocusActorToTargetPawn.Z = 0.f;
		FLOAT const FocusActorToTargetPawnMag = FocusActorToTargetPawn->Size();

		FLOAT const Radius = Min(TargetRelativeCameraOriginOffset.X, FocusActorToTargetPawnMag*0.5f);		// just using X for now
		FLOAT const RadiusSq = Square(Radius);

		if (FocusActorToTargetPawnMag > KINDA_SMALL_NUMBER)
		{
			FVector const FocusActorToTargetPawnNorm = FocusActorToTargetPawn / FocusActorToTargetPawnMag;

			FLOAT const b = RadiusSq / FocusActorToTargetPawnMag;
			FLOAT const B = FocusActorToTargetPawnMag - b;
			FLOAT const A = appSqrt(RadiusSq - Square(b));
			check( (RadiusSq - Square(b)) >= 0.f );

			// perp can be either (-y, x) or (y, -x).  just choosing one for now
			// no need to renormalize, since z=0 in FocusActorToTargetPawnNorm and we're just swapping elements
			FVector FocusActorToTargetPawnPerpNorm(-FocusActorToTargetPawnNorm.Y, FocusActorToTargetPawnNorm.X, 0.f);

			FVector NewOriginLoc = FocusActor->Location + (FocusActorToTargetPawnNorm * B) + (FocusActorToTargetPawnPerpNorm * A);
			NewOriginLoc.Z = OriginLoc.Z;

			static UBOOL bNewOriginCode = TRUE;
			if (bNewOriginCode)
			{
				OriginLoc = NewOriginLoc;
			}

			FocusActor->DrawDebugSphere(OriginLoc, 12, 10, 255, 255, 0);
		}
	}
#endif
}


/*-----------------------------------------------------------------------------
	UGameplayCam_MountedHeavyWeapon
-----------------------------------------------------------------------------*/

void UGameplayCam_MountedHeavyWeapon::GetBaseViewOffsets(APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	AGearWeap_HeavyBase* const HeavyWeap = ViewedPawn ? Cast<AGearWeap_HeavyBase>(ViewedPawn->Weapon) : NULL;

	if (HeavyWeap)
	{
		HeavyWeap->eventGetMountedCameraOffsets(out_Low, out_Mid, out_High);
	}
	else
	{
		// can get here if we're in turret mode but not attached to a turret (e.g heavy weapons)
		out_Low		= ViewOffset.OffsetLow;
		out_Mid		= ViewOffset.OffsetMid;
		out_High	= ViewOffset.OffsetHigh;
	}

	AGearPawn* const GP = Cast<AGearPawn>(ViewedPawn);
	if (GP && GP->bIsMirrored)
	{
		out_Low.Y = -out_Low.Y;
		out_Mid.Y = -out_Mid.Y;
		out_High.Y = -out_High.Y;
	}
}


/*-----------------------------------------------------------------------------
	UGameplayCam_BrumakDriver
-----------------------------------------------------------------------------*/

void UGameplayCam_BrumakDriver::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	// FInd the brumak we are watching
	AGearPawn_LocustBrumakBase* Brumak = Cast<AGearPawn_LocustBrumakBase>(TargetPawn);
	if( Brumak == NULL )
	{
		Brumak = Cast<AGearPawn_LocustBrumakBase>(TargetPawn->Base);
	}

	// If its dead, use backed up location
	if( Brumak != NULL && Brumak->Health <= 0 )
	{
		OriginLoc = Brumak->OldCamPosition;
		OriginRot = Brumak->OldCamRotation;
		return;
	}

	Super::GetCameraOrigin( Brumak, OriginLoc, OriginRot );

	if( GameplayCam != NULL && GameplayCam->PlayerCamera != NULL && GameplayCam->PlayerCamera->PCOwner != NULL )
	{
		AGearPC* PC = Cast<AGearPC>(GameplayCam->PlayerCamera->PCOwner);
		if( PC != NULL && 
			PC->Pawn != NULL && 
			PC->Pawn != Brumak )
		{
			if( GameplayCam->bResetCameraInterpolation || LockedToViewTarget(PC->Pawn) )
			{
				OriginRot = PC->Pawn->eventGetViewRotation();
			}
			else
			{
				OriginRot = GameplayCam->PlayerCamera->Rotation;
			}
		}
	}

	// If brumak is alive, save off this camera info
	if( Brumak != NULL && Brumak->Health > 0 )
	{
		Brumak->OldCamPosition = OriginLoc;
		Brumak->OldCamRotation = OriginRot;
	}
}

/** Returns true if mode should lock camera to view target, false otherwise */
UBOOL UGameplayCam_BrumakDriver::LockedToViewTarget( APawn* CameraTarget )
{
	if( CameraTarget )
	{
		AGearPC* const WPC = Cast<AGearPC>(CameraTarget->Controller);

		if( WPC && WPC->bAssessMode )
		{
			// lock to target when using tac/com
			return TRUE;
		}
	}

	return bLockedToViewTarget;
}

/** Returns true if mode should be using direct-look mode, false otherwise */
UBOOL UGameplayCam_BrumakDriver::UseDirectLookMode( APawn* CameraTarget )
{
	if( CameraTarget )
	{
		AGearPC* const WPC = Cast<AGearPC>(CameraTarget->Controller);

		if( WPC && WPC->bAssessMode )
		{
			// lock to target when using tac/com
			return FALSE;
		}
	}

	return bDirectLook;
}

/**
* Returns true if this mode should do target following.  If true is returned, interp speeds are filled in.
* If false is returned, interp speeds are not altered.
*/
UBOOL UGameplayCam_BrumakDriver::ShouldFollowTarget( APawn* CameraTarget, FLOAT& PitchInterpSpeed, FLOAT& YawInterpSpeed, FLOAT& RollInterpSpeed )
{
	if( LockedToViewTarget( CameraTarget ) )
	{
		// no following when locked
		return FALSE;
	}

	AGearPC* const WPC = Cast<AGearPC>(CameraTarget->Controller);
	if( WPC && WPC->bAssessMode )
	{
		// no following when using tac/com
		return FALSE;
	}

	if( bFollowTarget )
	{
		PitchInterpSpeed = FollowingInterpSpeed_Pitch;
		YawInterpSpeed	 = FollowingInterpSpeed_Yaw;
		RollInterpSpeed  = FollowingInterpSpeed_Roll;

		return TRUE;
	}

	return FALSE;
}

/** returns View relative offsets */
void UGameplayCam_BrumakDriver::GetBaseViewOffsets( APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High )
{
	out_Low		= ViewOffset.OffsetLow;
	out_Mid		= ViewOffset.OffsetMid;
	out_High	= ViewOffset.OffsetHigh;
}

/*-----------------------------------------------------------------------------
	UGameplayCam_TargetingGrenade
-----------------------------------------------------------------------------*/

void UGameplayCam_TargetingGrenade::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	// clamp pitch
	OriginRot.Pitch = appTrunc( Clamp( (FLOAT) FRotator::NormalizeAxis(OriginRot.Pitch), GrenadeCamPitchLimit.X, GrenadeCamPitchLimit.Y ) );
}

/*-----------------------------------------------------------------------------
	UGameplayCam_CoverTargetingGrenade
-----------------------------------------------------------------------------*/

void UGameplayCam_CoverTargetingGrenade::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	// clamp pitch
	OriginRot.Pitch = appTrunc( Clamp( (FLOAT) FRotator::NormalizeAxis(OriginRot.Pitch), GrenadeCamPitchLimit.X, GrenadeCamPitchLimit.Y ) );
}


/*-----------------------------------------------------------------------------
	UFixedCam_AutoFraming
-----------------------------------------------------------------------------*/


void UFixedCam_AutoFraming::UpdateCamera(class ACameraActor* CamActor, class APawn* TargetPawn, FLOAT DeltaTime, struct FTViewTarget& OutVT)
{
	// @fixme, writeme
#if 0
	local GearPawn GP;
	local array<GearPawn> VisibleTeammateList;
	//	local GearSpeechManager GSM;

	local int Idx;
	local float RelWeight, TotalWeight;
	local array<float> RelativeWeights;
	local vector WeightedLookat;


	local vector BoundingSphereCenter, BoxMin, BoxMax;
	local float	BoundingSphereRadius;

	local vector CamX, CamY, CamZ, LeftExtent, RightExtent, CamToExtentNorm;
	local float Angle, MaxSphereExtentFOV;

	if (CamActor == NULL)
	{
		// not much we can do here, return values unchanged
		return;
	}
	if (TargetPawn == NULL)
	{
		// just look through the 
		OutVT.POV.Location = CamActor->Location;
		OutVT.POV.Rotation = CamActor->Rotation;
		OutVT.POV.FOV = CamActor->FOVAngle;
	}

	struct VisibleTeammate
	{
		AGearPawn* GP;
		FLOAT RelativeWeight;
	};

	// always at the camactor loc
	OutVT.POV.Location = CamActor->Location;

	// this is just simple TargetPawn tracking
	//local GearPawn TrackPawn;
	//TrackPawn = GearPawn(TargetPawn);
	//OutVT.POV.FOV = CamActor.FOVAngle;
	//OutVT.POV.Rotation = rotator(TrackPawn->Location - CamActor.Location);
	//OutVT.POV.Location = CamActor.Location;



	// autoframing rules
	// try to keep all teammates onscreen
	// favor the speaker
	// strongly favor the TargetPawn
	// only consider visible pawns (trace check)
	// set FOV (nearly) as tight as possible

	// @todo: do this in native code?


	// get list of pawns we'll try and keep onscreen.


	foreach CamActor.WorldInfo.AllPawns(class'GearPawn', GP)
	{
		if ( GP.IsSameTeam(TargetPawn) && GP.FastTrace(GP.Location, CamActor.Location) )
		{
			VisibleTeammateList.AddItem(GP);

			// figure and store the relative weighting of this teammate
			if (GP == TargetPawn)
			{
				RelWeight = RelativeWeight_TargetPawn;
			}
			//@fixme, include GUDS here?
			else if ( (GP.CurrentlySpeakingLine != None) && (GP.CurrentSpeakLineParams.Priority >= Speech_GUDS) )
			{
				RelWeight = RelativeWeight_Speaker;
			}
			else
			{
				RelWeight = RelativeWeight_Normal;
			}

			RelativeWeights.AddItem(RelWeight);
			TotalWeight += RelWeight;
		}
	}

	//	GSM = GearGRI(CamActor.WorldInfo.GRI).SpeechManager;

	// no one visible, just use default camera orientation and bail
	if (VisibleTeammateList.length == 0)
	{
		OutVT.POV.Rotation = CamActor.Rotation;
		OutVT.POV.FOV = CamActor.FOVAngle;
		return;
	}


	// normalize relative weights and calc weighted lookat point
	for (Idx=0; Idx<RelativeWeights.length; ++Idx)
	{
		RelativeWeights[Idx] /= TotalWeight;
		WeightedLookat += VisibleTeammateList[Idx].Location * RelativeWeights[Idx];

		//BoundingSphereCenter += VisibleTeammateList[Idx].Location / float(RelativeWeights.length);
	}

	if (!FixedCam.bResetCameraInterpolation)
	{
		// interpolate for smoothness
		WeightedLookat = VInterpTo(LastLookat, WeightedLookat, DeltaTime, LookatInterpSpeed);
	}

	OutVT.POV.FOV = CamActor.FOVAngle;
	OutVT.POV.Rotation = rotator(WeightedLookat - CamActor.Location);

	CamActor.DrawDebugSphere(WeightedLookat, 16, 10, 255, 255, 0);

	LastLookat = WeightedLookat;



	// try to set an optimal fov




	BoxMin = VisibleTeammateList[0].Location;
	BoxMax = VisibleTeammateList[0].Location;
	foreach VisibleTeammateList(GP)
	{
		// @fixme, do this in above loop
		GrowBox(BoxMin, BoxMax, GP.Location);
	}

	BoundingSphereCenter = (BoxMin + BoxMax) * 0.5f;
	BoundingSphereRadius = FMax( (BoxMax.X-BoxMin.X), (BoxMax.Y-BoxMin.Y) );
	BoundingSphereRadius = FMax( BoundingSphereRadius, (BoxMax.Z-BoxMin.Z) );
	BoundingSphereRadius *= 0.5f;
	BoundingSphereRadius = FMax(400, BoundingSphereRadius);		// to prevent it getting to small.  what's a sensible limit here?  could get tight on faces, which might be cool

	CamActor.DrawDebugSphere(BoundingSphereCenter, BoundingSphereRadius, 12, 255, 255, 255);

	// make sure hfov is large enough to see entire sphere
	// @todo, maybe make sure vfov is large enough as well?  only if losing guys off bottom/top is a big problem.
	GetAxes(OutVT.POV.Rotation, CamX, CamY, CamZ);

	LeftExtent = BoundingSphereCenter - CamY * BoundingSphereRadius;
	RightExtent = BoundingSphereCenter + CamY * BoundingSphereRadius;

	// left
	CamToExtentNorm = Normal(LeftExtent - OutVT.POV.Location);
	Angle = Abs(ACos(CamToExtentNorm dot CamX) * RadToDeg);
	`log("left angle"@Angle);
	MaxSphereExtentFOV = FMax(MaxSphereExtentFOV, Angle);

	// right
	CamToExtentNorm = Normal(RightExtent - OutVT.POV.Location);
	Angle = Abs(ACos(CamToExtentNorm dot CamX) * RadToDeg);
	`log("   ...right angle"@Angle);
	MaxSphereExtentFOV = FMax(MaxSphereExtentFOV, Angle);

	// convert to full angles
	MaxSphereExtentFOV *= 2.f;

	if (!FixedCam.bResetCameraInterpolation)
	{
		MaxSphereExtentFOV = FInterpTo(LastFOV, MaxSphereExtentFOV, DeltaTime, FOVInterpSpeed);
	}
	if (bDoFOVAdj)
	{
		OutVT.POV.FOV = MaxSphereExtentFOV;
	}
	LastFOV = MaxSphereExtentFOV;

#endif
}

//
// USpectatorCam_AutoFraming
// "Battle Cam"
//

FAutoframingWatchedPawn* USpectatorCam_AutoFraming::FindRelevantPawnEntry(AGearPawn const* GP)
{
	INT const NumRelevantPawns = RelevantPawnList.Num();
	
	for (INT Index=0; Index<NumRelevantPawns; Index++)
	{
		FAutoframingWatchedPawn* const Entry = &RelevantPawnList(Index);
		if (Entry->GP == GP)
		{
			return Entry;
		}
	}

	return NULL;
}


void USpectatorCam_AutoFraming::UpdateRelevantPawnList(FVector& OutCentroid, FVector const& BaseLoc, FRotator const& BaseRot, UBOOL bInterpolating)
{
	// Special Case:  At end of round/match, just freeze the centroid and relevant pawn lists.
	AGearGRI* const GRI = Cast<AGearGRI>(GWorld->GetWorldInfo()->GRI);
	if ( GRI && (GRI->GameStatus > GS_RoundInProgress) )
	{
		OutCentroid = LastLookat;
		return;
	}

	// clear for accumulation
	OutCentroid = FVector(0.f);	
	
	// Special Case 2: if interpolating with 0 relevant pawns, don't allow new relevant pawns.
	// this prevents nasty pop, due to the faster interpolation while moving.
	if ( bInterpolating && (RelevantPawnList.Num() == 0) )
	{
		return;
	}

	FVector const AutoframingBaseFacingVector = BaseRot.Vector();

	FLOAT const CurrentTime = GWorld->GetWorldInfo()->TimeSeconds;

	FVector BaseX, BaseY, BaseZ;
	FRotationMatrix(BaseRot).GetAxes(BaseX, BaseY, BaseZ);

	FVector PawnLookatLoc(0.f);

	// check for input
	FVector SelectionPlaneNormal(0.f);
	FLOAT SelectionPlaneIntensity = 0.f;
#if 0
	{
		// construct a plane...
		FRotationMatrix const RotMat(BaseRot);

		FVector const RVec = RotMat.GetAxis(1);
		FVector const UVec = RotMat.GetAxis(2);

		static UBOOL bTest = 0;
		if (bTest)
			SpectatorCam->ControlInfo_LookRight = -1.f;
		
		SelectionPlaneNormal = (RVec * SpectatorCam->ControlInfo_LookRight) + (UVec * -SpectatorCam->ControlInfo_LookUp);
		SelectionPlaneIntensity = SelectionPlaneNormal.Size();
		if (SelectionPlaneIntensity > 0.f)
		{
			SelectionPlaneNormal /= SelectionPlaneIntensity;
		}
	}
#endif

	// walk the WorldInfo.Pawnlist
	for (APawn *P=GWorld->GetWorldInfo()->PawnList; P!=NULL; P=P->NextPawn)
	{
		AGearPawn* const GP = Cast<AGearPawn>(P);
		FAutoframingWatchedPawn* const ExistingEntry = FindRelevantPawnEntry(GP);
		UBOOL bIsRelevant = FALSE;

		// valid pawn for consideration?
		if ( GP && GP->Mesh && 
			 ( (GP->Health > 0) || GP->IsDBNO() ) &&
			 !GP->bDeleteMe
			 )
		{
			// base lookat is biased for velocity
			if(GP->NeckBoneName != NAME_None)
			{
				PawnLookatLoc = GP->Mesh->GetBoneLocation(GP->NeckBoneName);
			}
			else
			{
				PawnLookatLoc = GP->Location;
			}

			PawnLookatLoc += (GP->Velocity * VelBiasFactor);
			
			FVector PawnTraceLoc = PawnLookatLoc;

			// maybe bias the trace point.  we rotate offsets around the pawns to prevent
			// problems like visibility ping-ponging.
			if (ExistingEntry)
			{
				// advance trace point
				if (++ExistingEntry->LastTracePoint == PawnTrace_MAX)
				{
					ExistingEntry->LastTracePoint = PawnTrace_Head;
				}

				// calc new trace point
				switch (ExistingEntry->LastTracePoint)
				{
				case PawnTrace_Left:
					PawnTraceLoc -= BaseY * PawnTraceRadius;
					break;
				case PawnTrace_Right:
					PawnTraceLoc += BaseY * PawnTraceRadius;
					break;
				case PawnTrace_Above:
					PawnTraceLoc += BaseZ * PawnTraceRadius;
					break;
				}
			}

			// quick out for pawns behind the camera
			FLOAT const Dot = AutoframingBaseFacingVector | (PawnTraceLoc - BaseLoc);
			if (Dot > 0.f)
			{
				DWORD const TraceFlags = TRACE_World|TRACE_StopAtAnyHit;
				FCheckResult Hit(1.f);
				GWorld->SingleLineCheck( Hit, GP /*???*/, PawnTraceLoc, BaseLoc, TraceFlags );
				
				if (Hit.Actor == NULL)
				{
					bIsRelevant = TRUE;
				}
			}
		}

		if (bIsRelevant)
		{
			if (ExistingEntry)
			{
				// update existing entry
				ExistingEntry->LookatLoc = PawnLookatLoc;
				ExistingEntry->LastRelevantTime = CurrentTime;
				ExistingEntry->bNoRelevanceLag = FALSE;
			}
			else
			{
				// add new entry
				FAutoframingWatchedPawn WatchedPawn;
				WatchedPawn.GP = GP;
				WatchedPawn.LookatLoc = PawnLookatLoc;
				WatchedPawn.LastRelevantTime = CurrentTime;
				WatchedPawn.LastTracePoint = 0;
				WatchedPawn.NormalizedWeight = 0.0f;
				WatchedPawn.bNoRelevanceLag = FALSE;
				RelevantPawnList.AddItem(WatchedPawn);
			}
		}
	}

	// expire any no-longer-relevant entries, handle weighting, and update centroid
	FLOAT TotalWeight = 0.f;
	for (INT Idx=0; Idx<RelevantPawnList.Num(); ++Idx)
	{
		FLOAT Weight = 1.f;

		FAutoframingWatchedPawn* const RelevantPawn = &RelevantPawnList(Idx);

		// if interpolating, just expire folks right away
		{
			FLOAT const TimeSinceLastRelevance = CurrentTime - RelevantPawn->LastRelevantTime;

			if ( bInterpolating && (TimeSinceLastRelevance > 0.f) )
			{
			 	RelevantPawn->bNoRelevanceLag = TRUE;
			}

			// calculate weight based on relevance fading
			FLOAT WeightScale = RelevantPawn->bNoRelevanceLag ?
								(1.f - (TimeSinceLastRelevance/RelevanceFadeTime))  :
								1.f - ( (TimeSinceLastRelevance - RelevanceLagTime) / RelevanceFadeTime );
			WeightScale = Clamp(WeightScale, 0.f, 1.f);


			// when interpolation stops, clear irrelevant dudes
			if (!bInterpolating && bWasInterpolating && TimeSinceLastRelevance > 0.f)
			{
				WeightScale = 0.f;
			}


			Weight *= WeightScale;
		}

		// adjust weight based on player input
		if (SelectionPlaneIntensity > 0.f)
		{
			FLOAT DistToPlane = FPointPlaneDist(RelevantPawnList(Idx).LookatLoc, BaseLoc, SelectionPlaneNormal);
			if (DistToPlane > 0.f)
			{
				Weight *= DirSelectionWeightMultiplier_Good * SelectionPlaneIntensity;
			}
			else
			{
				Weight *= DirSelectionWeightMultiplier_Bad * SelectionPlaneIntensity;
			}
		}

		if (Weight > 0.f)
		{
			// store weight
			RelevantPawnList(Idx).NormalizedWeight = Weight;

			OutCentroid += RelevantPawnList(Idx).LookatLoc * Weight;
			TotalWeight += Weight;
		}
		else
		{
			// fully faded, expire
			RelevantPawnList.Remove(Idx, 1);
			--Idx;
		}
	}
	if (TotalWeight > 0.f)
	{
		OutCentroid /= TotalWeight;
	}

	// optional debug rendering
	if (bDebugShowWatchedPawns)
	{
		for (INT Idx=0; Idx<RelevantPawnList.Num(); ++Idx)
		{
			const FLOAT Weight = RelevantPawnList(Idx).NormalizedWeight;
			GWorld->GetWorldInfo()->DrawDebugBox(RelevantPawnList(Idx).LookatLoc, Weight * FVector(30.f,30.f,30.f), 255, 255, 0);
		}
		GWorld->GetWorldInfo()->DrawDebugBox(OutCentroid, FVector(6.f,6.f,6.f), 255, 255, 255);
	}

	// @todo, remove positional outliers, esp far away from camera?
	// @todo, weight pawns closer to camera?
	// @todo, weight pawns that have recently fired?
}

/** Returns in radians. */
FLOAT USpectatorCam_AutoFraming::CalcHFovFromVFov(FLOAT VFovRad) const
{
	// assume 16:9 if for some reason the below fails
	FLOAT AspectRatio = 16.f / 9.f;

	// try to find the real aspect ratio of our viewport
	if (SpectatorCam && SpectatorCam->PlayerCamera && SpectatorCam->PlayerCamera->PCOwner)
	{
		ULocalPlayer* const LP = Cast<ULocalPlayer>(SpectatorCam->PlayerCamera->PCOwner->Player);
		UGameViewportClient* const VPClient = LP ? LP->ViewportClient : NULL;
		if ( VPClient && (VPClient->GetCurrentSplitscreenType() == eSST_2P_VERTICAL) )
		{
			FVector2D FullViewportSize(0,0);
			VPClient->GetViewportSize(FullViewportSize);
			AspectRatio = FullViewportSize.X / FullViewportSize.Y;
		}
	}

	return appAtan( AspectRatio * appTan(VFovRad) );
}

/** Returns in radians. */
FLOAT USpectatorCam_AutoFraming::CalcVFovFromHFov(FLOAT HFovRad) const
{
	// assume 16:9 if for some reason the below fails
	FLOAT InvAspectRatio = 9.f / 16.f;

	// try to find the real aspect ratio of our viewport
	if (SpectatorCam && SpectatorCam->PlayerCamera && SpectatorCam->PlayerCamera->PCOwner)
	{
		ULocalPlayer* const LP = Cast<ULocalPlayer>(SpectatorCam->PlayerCamera->PCOwner->Player);
		UGameViewportClient* const VPClient = LP ? LP->ViewportClient : NULL;
		if ( VPClient && (VPClient->GetCurrentSplitscreenType() == eSST_2P_VERTICAL) )
		{
			FVector2D FullViewportSize(0,0);
			VPClient->GetViewportSize(FullViewportSize);
			InvAspectRatio = FullViewportSize.Y / FullViewportSize.X;
		}
	}

	return appAtan( InvAspectRatio * appTan(HFovRad) );
}

FLOAT USpectatorCam_AutoFraming::FindOptimalFOV(TArray<FAutoframingWatchedPawn> const& VisiblePawnList, FVector const& BaseLoc, FRotator const& BaseRot) const
{
	FRotationMatrix BaseRotMat(BaseRot);
	FVector CamX, CamY, CamZ;
	BaseRotMat.GetAxes(CamX, CamY, CamZ);

	// @todo, find min acceptablevertdot as well?
	FLOAT const MinAcceptableHorizDot = appCos(Deg2Rad * AcceptableFOVRange.Y);
	FLOAT const MinAcceptableVertDot = appCos(CalcVFovFromHFov(Deg2Rad * AcceptableFOVRange.Y));

	// find dot extents.  min dot is farthest from center (center is 1.f)
	FLOAT MinHorizDot=1.f, MinVertDot=1.f;
	for (INT Idx=0; Idx<VisiblePawnList.Num(); ++Idx)
	{
		FVector const ToLookat = VisiblePawnList(Idx).LookatLoc - BaseLoc;

		// see FVector.ProjectOnto
		FVector ToLookatXZ =  ToLookat - ToLookat.ProjectOnTo(CamY);
		// handle the zoom control by scaling the camera-perpendicular element of the lookat vector
		ToLookatXZ += ToLookatXZ.ProjectOnTo(CamZ) * (-SpectatorCam->ControlInfo_Zoom * ZoomFOVAdjustmentMag);
		ToLookatXZ.Normalize();

		FLOAT const VertDot = (ToLookatXZ | CamX);
		if (VertDot > MinAcceptableVertDot)
		{
			MinVertDot = Min( MinVertDot, VertDot );
		}

		FVector ToLookatXY = ToLookat - ToLookat.ProjectOnTo(CamZ);
		// handle the zoom control by scaling the camera-perpendicular element of the lookat vector
		ToLookatXY += ToLookatXY.ProjectOnTo(CamY) * (-SpectatorCam->ControlInfo_Zoom * ZoomFOVAdjustmentMag);
		ToLookatXY.Normalize();
		FLOAT const HorizDot = ToLookatXY | CamX;
		// if we're outsize the acceptable FOV range, ignore this pawn
		// altogether for FOV computation.  helps prevent some crazy outlier
		// from pegging the FOV at the max.
		if (HorizDot > MinAcceptableHorizDot)
		{
			MinHorizDot = Min( MinHorizDot, HorizDot );
		}
	}

	//
	// get the desired hfov, the hfov required to satisfy the desired vfov, and choose the max of the 2
	// There's some extra gymnastics here to provide a buffer at the screen edge, to make it look 
	// nicer.
	//

	// equation for edge buffer calculations:
	//		BufferAngle = FOV - ( atan( (1.f - BufferScreenSpacePct) * tan(FOV) ) )
	// @note the extra 2.f factor on BufferScreenSpacePct applied below.  that didn't come up 
	// in my equations, but I clearly found that it's needed to work as expected.  hey, it works. :)

	FLOAT HBufferPct = BorderBufferPercentage_Horizontal * (1.f + ZoomBufferAdjustmentMag * -SpectatorCam->ControlInfo_Zoom);
	FLOAT VBufferPct = BorderBufferPercentage_Vertical * (1.f + ZoomBufferAdjustmentMag * -SpectatorCam->ControlInfo_Zoom);

	FLOAT const HFOVFromHorizExtentsRad = appAcos(MinHorizDot);
	FLOAT const HFovBufferRad = HFOVFromHorizExtentsRad - ( appAtan( (1.f - 2.f*HBufferPct) * appTan(HFOVFromHorizExtentsRad) ) );
	FLOAT const DesiredHFOVRad = (HFOVFromHorizExtentsRad + HFovBufferRad);

	FLOAT const VFOVFromVertExtentsRad = appAcos(MinVertDot);
	FLOAT const VFovBufferRad = VFOVFromVertExtentsRad - ( appAtan( (1.f - 2.f*VBufferPct) * appTan(VFOVFromVertExtentsRad) ) );
	FLOAT const DesiredHFOVFromVFOVRad = CalcHFovFromVFov(VFOVFromVertExtentsRad + VFovBufferRad);

	// choose largest and then double, since we've been dealing with half angles up until now
	FLOAT OutHFOVDeg = Rad2Deg * Max(DesiredHFOVRad, DesiredHFOVFromVFOVRad) * 2.f;

	// clamp to limits
	OutHFOVDeg = Clamp(OutHFOVDeg, AcceptableFOVRange.X, AcceptableFOVRange.Y);

	return OutHFOVDeg;
};

void USpectatorCam_AutoFraming::UpdateCamera(class ACameraActor* CamActor, FLOAT DeltaTime, struct FTViewTarget& OutVT)
{
	// autoframing rules
	// @WIP
	// try to keep "the action" onscreen
	// only consider visible pawns (trace check)
	// set FOV (nearly) as tight as possible
	//
	// @NOTE
	// at the moment, we're just doing "try to fit everyone onscreen"

	// sanity
	if (!SpectatorCam || !CamActor)
	{
		return;
	}

	// whether or not the camera is moving around (e.g. between path stops)
	UBOOL const bInterpolating = LastCamActorLoc != CamActor->Location;

	// Location is easy, just stick to the CamActor, always.
	FVector const DesiredLoc = CamActor->Location;
	FRotator DesiredRot(0,0,0);
	FLOAT DesiredFOV = 0.f;

	// build the list of pawns we care about keeping on screen
	FVector RelevantPawnCentroid(0.f);
	UpdateRelevantPawnList(RelevantPawnCentroid, CamActor->Location, CamActor->Rotation, bInterpolating);

	// no one visible
	if (RelevantPawnList.Num() == 0)
	{
		// centroid code might update this, but by default use the camactor's rot
		DesiredRot = CamActor->Rotation;

		if (bInterpolating || SpectatorCam->bResetCameraInterpolation || (LastFOV <= 0.f) )
		{
			DesiredFOV = CamActor->FOVAngle;
		}
		else
		{
			DesiredFOV = LastFOV;
		}
	}
	else
	{
		// camera looks at centroid
		DesiredRot = (RelevantPawnCentroid - CamActor->Location).Rotation();

		if (!bDebugSkipFOVAdj)
		{
			// try to set an optimal fov
			DesiredFOV = FindOptimalFOV(RelevantPawnList, CamActor->Location, DesiredRot);
		}
		else
		{
			DesiredFOV = CamActor->FOVAngle;
		}
	}

	//
	// now set final values, interpolating as necessary
	//

	// always at the camactor's actual loc
	OutVT.POV.Location = DesiredLoc;

	FRotator InterpedDeltaRot(0,0,0);

	if (!SpectatorCam->bResetCameraInterpolation)
	{
		// interp the centroid...
		{
			if (RelevantPawnList.Num() > 0)
			{
				FLOAT const InterpSpeed = bInterpolating ? LookatInterpSpeed_MovingCamera : LookatInterpSpeed;

				// only interpolate if we've got a good last lookat pos
				if (bLastLookatIsValid)
				{
					RelevantPawnCentroid = VInterpTo(LastLookat, RelevantPawnCentroid, DeltaTime, InterpSpeed);
				}

				// new rot based on new centroid
				DesiredRot = (RelevantPawnCentroid - CamActor->Location).Rotation();
				bLastLookatIsValid = TRUE;

				// debug support
				if (bDebugShowWatchedPawns)
				{
					CamActor->DrawDebugBox(RelevantPawnCentroid, FVector(6.f,6.f,6.f), 128, 128, 255);
				}
			}
			else if (bInterpolating && bLastLookatIsValid)
			{
				RelevantPawnCentroid = LastLookat;

				// new rot based on new centroid
				DesiredRot = (RelevantPawnCentroid - CamActor->Location).Rotation();
				bLastLookatIsValid = TRUE;
			}
			else
			{
				bLastLookatIsValid = FALSE;
			}

		}

		//@fixme - hacky way of applying the battle cam offsets, Jeff probably should rework this and remove my 'fix'
		DesiredRot.Yaw += appTrunc(SpectatorCam->ControlInfo_LookRight * 4096.f);
		DesiredRot.Pitch += appTrunc(SpectatorCam->ControlInfo_LookUp * -4096.f);

		// interp rotational offset
		// we do the delta and not the straight rotation so the camera can rotate via Matinee and
		// we don't lag for that portion of the motion.
		{
			FRotator const DesiredDeltaRot = (DesiredRot - CamActor->Rotation).GetNormalized();
			FLOAT const InterpSpeed = bInterpolating ? RotInterpSpeed_MovingCamera : RotInterpSpeed;
			InterpedDeltaRot = RInterpTo(LastDeltaRot, DesiredDeltaRot, DeltaTime, InterpSpeed);
			DesiredRot = CamActor->Rotation + InterpedDeltaRot;
		}

		// interp FOV
		{
			// an FOV of 0 is invalid so use the FOV passed in. This fixes popping due to interpolating from 0 to 90 at the beginning of a multiplayer match.
			if(LastFOV == 0.0f)
			{
				LastFOV = OutVT.POV.FOV;
			}

			FLOAT const InterpSpeed = bInterpolating ? FOVInterpSpeed_MovingCamera : FOVInterpSpeed;
			DesiredFOV = FInterpTo(LastFOV, DesiredFOV, DeltaTime, InterpSpeed);
		}
	}
	else
	{
		bLastLookatIsValid = FALSE;
	}

	OutVT.POV.FOV = DesiredFOV;
	OutVT.POV.Rotation = DesiredRot;

	LastLookat = RelevantPawnCentroid;
	LastDeltaRot = InterpedDeltaRot;
	LastFOV = OutVT.POV.FOV;
	LastCamActorLoc = CamActor->Location;
	bWasInterpolating = bInterpolating;
}

/*-----------------------------------------------------------------------------
	UGameplayCam_SniperZoom
-----------------------------------------------------------------------------*/

void UGameplayCam_SniperZoom::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	AGearPawn* const TargetGP = Cast<AGearPawn>(TargetPawn);
	if (TargetGP)
	{
		AGearWeapon* const Weap = Cast<AGearWeapon>(TargetGP->Weapon);
		OriginLoc = TargetGP->eventGetWeaponStartTraceLocation(Weap);

		FVector UnusedLoc(0.0f);
		TargetGP->eventGetActorEyesViewPoint(UnusedLoc, OriginRot);
	}
}


/*-----------------------------------------------------------------------------
	UGameplayCam_CentaurDeath
-----------------------------------------------------------------------------*/

void UGameplayCam_CentaurDeath::GetBaseViewOffsets(class APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	// skip _Vehicle and use the normal offsets
	UGearGameplayCameraMode::GetBaseViewOffsets(ViewedPawn, ViewportConfig, DeltaTime, out_Low, out_Mid, out_High);
}

void UGameplayCam_BrumakDeath::GetBaseViewOffsets(class APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High)
{
	// skip _Vehicle and use the normal offsets
	UGearGameplayCameraMode::GetBaseViewOffsets(ViewedPawn, ViewportConfig, DeltaTime, out_Low, out_Mid, out_High);
}
FLOAT UGearCameraModifier::GetTargetAlpha(class ACamera* Camera )
{
	if( bPendingDisable )
	{
		return 0.0f;
	}
	return 1.0f;		
}

UBOOL UGearCameraModifier::ModifyCamera(class ACamera* Camera,FLOAT DeltaTime,FTPOV& OutPOV)
{
	// If pending disable and fully alpha'd out, truly disable this modifier
	if( bPendingDisable && Alpha <= 0.0 )
	{
		Super::eventDisableModifier();
	}

	return FALSE;
}

void UGearCameraModifier::UpdateAlpha(class ACamera* Camera,FLOAT DeltaTime)
{
	FLOAT Time;

	TargetAlpha = GetTargetAlpha( Camera );

	// Alpha out
	if( TargetAlpha == 0.0 )
	{
		Time = AlphaOutTime;
	}
	else
	{
		// Otherwise, alpha in
		Time = AlphaInTime;
	}

	if( Time <= 0.0 )
	{
		Alpha = TargetAlpha;
	}
	else if( Alpha > TargetAlpha )
	{
		Alpha = Max<FLOAT>( Alpha - (DeltaTime * (1.0 / Time)), TargetAlpha );
	}
	else
	{
		Alpha = Min<FLOAT>( Alpha + (DeltaTime * (1.0 / Time)), TargetAlpha );
	}
}


/*-----------------------------------------------------------------------------
	UGameplayCam_Default
-----------------------------------------------------------------------------*/

void UGameplayCam_Default::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	// a kludgy way of interpolating out of grenade cameras
	// @fixme, need a better way to manage transitions between modes
	if (bTemporaryOriginRotInterp)
	{
		FRotator BaseOriginRot = OriginRot;
		OriginRot = RInterpTo(GameplayCam->LastActualCameraOriginRot, BaseOriginRot, TemporaryOriginRotInterpSpeed, GWorld->GetWorldInfo()->DeltaSeconds);
		if (OriginRot == BaseOriginRot)
		{
			// arrived, stop interp
			bTemporaryOriginRotInterp = FALSE;
		}
	}
}

void UGameplayCam_Death::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	AGearPawn* GP = Cast<AGearPawn>(TargetPawn);
	if( GP != NULL )
	{
		OriginRot = GP->RotationWhenKilled;
		OriginLoc = GP->LocationWhenKilled;
	}
	else
	{
		Super::GetCameraOrigin( TargetPawn, OriginLoc, OriginRot );
	}
}


/*-----------------------------------------------------------------------------
	UGameplayCam_Cover
-----------------------------------------------------------------------------*/

void UGameplayCam_Cover::GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot)
{
	Super::GetCameraOrigin(TargetPawn, OriginLoc, OriginRot);

	// a kludgy way of interpolating out of grenade cameras
	// @fixme, need a better way to manage transitions between modes
	if (bTemporaryOriginRotInterp)
	{
		FRotator BaseOriginRot = OriginRot;
		OriginRot = RInterpTo(GameplayCam->LastActualCameraOriginRot, BaseOriginRot, TemporaryOriginRotInterpSpeed, GWorld->GetWorldInfo()->DeltaSeconds);
		if (OriginRot == BaseOriginRot)
		{
			// arrived, stop interp
			bTemporaryOriginRotInterp = FALSE;
		}
	}
}




