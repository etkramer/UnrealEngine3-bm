/**
 * Mortar Heavy weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_HeavyMortarBase extends GearWeap_HeavyBase
	abstract;

/** Internal.  Percentage, range [0..1], representing where in the LaunchPitchRange we are aiming. */
var transient float			CurrentElevationPct;
var transient float			LastElevationPct;

/** Last shot elevation, used for HUD temporarily */
var transient float			LastLastElevationPct;

/** How many audio "beeps" or ticks to play while elevation is rising. */
var() protected const config int		LaunchElevationNumBeeps;


/** Starting and ending pitch (X,Y) for launcher */
var() protected const config vector2d	LaunchPitchRange;
/** Pitch to launch mortar when firing untargeted. */
var() protected const config float		LaunchPitchUntargeted;

/** How long it takes, in seconds, for elevation to interpolate from PitchRange.X to PitchRange.Y */
var() protected const config float		LaunchElevationTime;

var() protected const float				MortarRotInterpSpeed;
/** Pawn crank animation. */
var protected const BodyStance			BS_PawnCrank;

var protected const SoundCue			CrankSound;
var protected transient AudioComponent	CrankAC;

var protected const SoundCue			FireSoundUntargeted;
var protected const SoundCue			FireSoundUntargeted_Player;

var protected const SoundCue			MortarCrankingClickSound;

var SkeletalMeshComponent	MortarShell;
var BodyStance				BS_StandingFire, BS_MountedFire;
var Name					WeaponMountedReloadAnim, WeaponMountedReloadFailAnim;

var protected const class<Projectile> UntargetedProjectileClass;

var ForceFeedbackWaveForm MortarTickFF;

/**
 * State Active
 * When a weapon is in the active state, it's up, ready but not doing anything. (idle)
 */
simulated state Active
{
	simulated function BeginState( Name PreviousStateName )
	{
		local GearPawn	MyGearPawn;

		if( !HasAnyAmmo() )
		{
			// Get rid of Mortar if it's out of ammo.
			MyGearPawn = GearPawn(Instigator);
			MyGearPawn.CheckHeavyWeaponMounting();
			return;
		}

		Super.BeginState(PreviousStateName);
	}
}

simulated function Projectile ProjectileFire()
{
	local Projectile PB;

	PB = Super.ProjectileFire();
	if ( WorldInfo.NetMode == NM_Client )
	{
		return None;
	}

	if ( (Instigator != None) && IsMounted() )
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_MortarLaunched, Instigator, None, 1.f);
	}

	return PB;
}

simulated event DummyFire(byte FireModeNumm, vector TargetLoc, optional Actor AttachedTo, optional float AimErrorDeg, optional Actor TargetActor)
{
	local vector SuggestedInitVel, StartLoc;
	local class<Projectile> ProjClass;
	local Projectile SpawnedProjectile;
	local bool bFoundVel;
	local float SpeedMult;

	// fire a projectile without an instigator.	 nothing fancy, just straight
	if( WorldInfo.Netmode != NM_Client )
	{
		ProjClass = GetProjectileClass();
		StartLoc = GetPhysicalFireStartLoc();

		// keep trying until we find a speed that can hit the target.  capped, of course.
		do
		{
			SpeedMult += 1.f;
			bFoundVel = SuggestTossVelocity(SuggestedInitVel, TargetLoc, StartLoc, SpeedMult * ProjClass.default.Speed,, 0.9f);
		} until ( bFoundVel || (SpeedMult >= 5.f) );

		if (bFoundVel)
		{
			SpawnedProjectile = Spawn(ProjClass, Self,, StartLoc);
			if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
			{
				SpawnedProjectile.Speed *= SpeedMult;
				SpawnedProjectile.Init( Normal(SuggestedInitVel) );
				SpawnedProjectile.Velocity = SuggestedInitVel;
				GearProjectile(SpawnedProjectile).bSuppressAudio = bSuppressAudio;
			}
		}
		else
		{
			`log("DummyFire Weapon"@self@"Could not calculate trajectory.  Are source and target too far apart?");
		}
	}
}

/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease()
{
	//local SkeletalMeshComponent	SkelMesh;
	//local Vector				MagLoc;
	//local Rotator				MagRot;

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Hide Ammo part on weapon mesh
	SetWeaponAmmoBoneDisplay( FALSE );

	// Spawn physics magazine instead
	//SkelMesh = SkeletalMeshComponent(Mesh);
	//SkelMesh.GetSocketWorldLocationAndRotation('Magazine', MagLoc, MagRot);

	// Spawn physics Magazine
	//SpawnPhysicsMagazine(MagLoc, MagRot);
}


///** notification fired from Pawn animation when player puts new magazine in weapon. */
//simulated function Notify_AmmoReload()
//{
//	// Skip on dedicated servers, below is just cosmetic
//	if( WorldInfo.NetMode == NM_DedicatedServer )
//	{
//		return;
//	}
//
//	// Detach magazine mesh from instigator's left
//	SetPawnAmmoAttachment(FALSE);
//
//	// Show Ammo part on weapon mesh.
//	SetWeaponAmmoBoneDisplay(TRUE);
//}

simulated function LaunchMortar()
{
	FireAmmunition();
}

simulated function Tick(float DeltaTime)
{
	//local int NewPitch;
	//local rotator TmpRot
	//local rotator  GoalRot;
	//local vector2d HackedPitchRange;
	local GearPawn InstigatorGP;
	local int LastBeepSegment, CurBeepSegment;

	// for debug line hack below
	//	local vector ProjLoc, ProjDir;

	InstigatorGP = GearPawn(Instigator);

	if( InstigatorGP != None )
	{
		if( (Role == ROLE_Authority) && !bSlaveWeapon )
		{
			if ( !IsFiring() &&
				(CurrentElevationPct > 0.f) &&
				( !IsMounted() || IsReloading() )
				)
			{
				// figure out elevation pct
				CurrentElevationPct -= DeltaTime / GetFireInterval(0);
				CurrentElevationPct = FMax(CurrentElevationPct, 0.f);
				InstigatorGP.MortarElevationPct = CurrentElevationPct;
			}
		}
		else
		{
			// get from replicated var
			CurrentElevationPct = InstigatorGP.MortarElevationPct;
		}


		// keep raising weapon
		//if ( InstigatorGP.IsDoingSpecialMove(SM_TargetMortar) )
		//{
		//	// need to adjust pitchrange due to the placeholder Yaw=180 model flip
		//	HackedPitchRange.X = 2 * 16384 - LaunchPitchRange.X;
		//	HackedPitchRange.Y = 2 * 16384 - LaunchPitchRange.Y;
		//	NewPitch = HackedPitchRange.X + CurrentElevationPct * (HackedPitchRange.Y - HackedPitchRange.X);
		//	GoalRot = Mesh.Rotation;
		//	if (Instigator.Controller != None)
		//	{
		//		GoalRot.Yaw = default.Mesh.Rotation.Yaw + (Instigator.Controller.Rotation.Yaw - Instigator.Rotation.Yaw);
		//	}
		//	GoalRot.Pitch = NewPitch;
		//}
		//else
		//{
		//	GoalRot = default.Mesh.Rotation;
		//}

		//TmpRot = RInterpTo(Mesh.Rotation, GoalRot, DeltaTime, MortarRotInterpSpeed);
		//Mesh.SetRotation(TmpRot);

		//`log("elev time"@CurrentElevationTime@TmpRot@NewPitch@default.Mesh.Rotation.Pitch@FMin(CurrentElevationTime / LaunchElevationTime, 1.f));

		// play the click/beep when crossing certain evenly-spaced elevation thresholds.
		LastBeepSegment = int(LaunchElevationNumBeeps * LastElevationPct);
		CurBeepSegment = int(LaunchElevationNumBeeps * CurrentElevationPct);
		if ( (CurBeepSegment > LastBeepSegment) || ( (LastElevationPct == 0.f) && (CurrentElevationPct != 0.f) ) )
		{
			PlayElevatingBeepAudio();
		}
		LastElevationPct = CurrentElevationPct;
	}
	else
	{
		LastElevationPct = 0.f;
	}

	Super.Tick(DeltaTime);
}

simulated protected function PlayElevatingBeepAudio()
{
	local GearPC PC;
	WeaponPlaySound(MortarCrankingClickSound);
	PC = GearPC(Instigator.Controller);
	if (PC != None && PC.IsLocalPlayerController())
	{
		PC.ClientPlayForceFeedbackWaveform(MortarTickFF);
	}
}

simulated function AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	local vector NewScale3D;

	super.AttachMuzzleEffectsComponents(SkelMesh);

	// this is to offset the temporary mesh scaling -- we want the muzzle flash to look normal.
	// REMOVE when real mesh is in place
	NewScale3D.X = 1.f/SkelMesh.Scale3D.X;
	NewScale3D.Y = 1.f/SkelMesh.Scale3D.Y;
	NewScale3D.Z = 1.f/SkelMesh.Scale3D.Z;
	MuzFlashEmitter.SetScale3D(NewScale3D);

	MuzFlashEmitter.SetScale(1.f/SkelMesh.Scale);
}


simulated function float GetWeaponRating()
{
	local GearAI GAI;
	GAI = GearAI(Instigator.Controller);
	
	// put away if enemy is too close
	if ( GAI != None && Instigator.Controller.Enemy != None &&
		VSize(Instigator.Controller.Enemy.Location - Instigator.Location) < 1024.0 
		&& GAI.IsEnemyVisible(GAI.Enemy))
	{
		`AILog_Ext(GetFuncName()@self@"enemy within range:"@GAI.Enemy@"Dumping mortar",,GAI);

		return -1.0;
	}
	else
	{
		`AILog_Ext(GetFuncName()@self@"enemy wasn't within range",,GAI);

		// don't switch from this ever unless enemy is within 1024
		if(GAI != none && GearAI_TDM(GAI) == none)
		{
			`AILog_Ext(GetFuncName()@self@"returning 10!",,GAI);

			return 10.f;
		}	
	}

	return Super.GetWeaponRating();
}

/** @return whether the target location can be hit with the unmounted firemode */
function bool CanHitWithUnmountedFire(vector TargetLoc)
{
	local vector Start;

	if (VSize(TargetLoc - Instigator.Location) > 1500.0)
	{
		// too far away
		return false;
	}
	else
	{
		// trace from feet to target, since the unmounted shot fires very low
		Start = Instigator.Location;
		Start.Z -= Instigator.GetCollisionHeight() * 0.75;
		return FastTrace(TargetLoc, Start);
	}
}

simulated function StartFire(byte FireModeNum)
{
	local bool bOkToFire;

	if (AIController != None && GearAI_TDM(AIController) == None)
	{
		// AI only fires while mounted
		if (IsMounted())
		{
			bOkToFire = TRUE;
		}
	}
	// reject fire attempts while in the process of unmounting
	// prevent AI from using unmounted fire while enemy is out of range
	else if ( !IsBeingUnMounted() &&
		(GearAI_TDM(GearAIController) == None || IsMounted() || CanHitWithUnmountedFire(GearAIController.GetEnemyLocation())) )
	{
		bOkToFire = TRUE;
	}

	if (bOkToFire)
	{
		super.StartFire(FireModeNum);
	}
}


simulated function SetMounted(bool bNowMounted)
{
	if ( !bNowMounted && (CrankAC != None) )
	{
		CrankAC.FadeOut(0.3f, 0.f);
	}

	super.SetMounted(bNowMounted);
}


simulated state Elevating
{
	simulated protected function ElevatingFinished()
	{
		LastLastElevationPct = LastElevationPct;
		SetCurrentFireMode(0);
		GotoState('WeaponFiring');
	}

	simulated event bool IsFiring()
	{
		return TRUE;
	}

	simulated function SetMounted(bool bNowMounted)
	{
		// make sure the AI finishes firing before unmounting
		if (!bNowMounted && AIController != None)
		{
			ElevatingFinished();
		}
		Super.SetMounted(bNowMounted);
	}

	/** Starts the elevating process.  Triggers crank animation, elevation changes, etc. */
	simulated protected function BeginElevating()
	{
		local GearPawn InstigatorGP;

		// start the elevating
		SetCurrentFireMode(FIREMODE_CHARGE);

		// "ready" stance
		InstigatorGP = GearPawn(Instigator);
		if (InstigatorGP != None)
		{
			InstigatorGP.SetWeaponAlert(LaunchElevationTime);
		}
	}

	simulated function BeginState(Name PreviousStateName)
	{
		CurrentElevationPct = 0.f;

		if (IsMounted())
		{
			BeginElevating();
		}
		else if (!IsBeingMounted())
		{
			// not in the process of mounting, not mounted, go straight to firing
			// to do an "untargeted" launch
			GotoState('WeaponFiring');
		}
	}

	simulated function EndFire(byte FireModeNum)
	{
		Global.EndFire(FireModeNum);

		// if fire is released, elevating phase is finished
		if ( IsMounted() )
		{
			ElevatingFinished();
		}
		else
		{
			GotoState('Active');
		}
	}

	simulated function Tick(float DeltaTime)
	{
		local GearPawn InstigatorGP;

		Global.Tick(DeltaTime);

		if ( (CurrentFireMode != FIREMODE_CHARGE) && IsMounted() )
		{
			BeginElevating();
		}

		// figure out elevation pct
		InstigatorGP = GearPawn(Instigator);
		if ( (Role == ROLE_Authority) && (!bSlaveWeapon) )
		{
			if (IsMounted())
			{
				CurrentElevationPct += DeltaTime / LaunchElevationTime;
				CurrentElevationPct = FMin(CurrentElevationPct, 1.f);
				InstigatorGP.MortarElevationPct = CurrentElevationPct;
			}
		}
		else
		{
			// get from replicated var
			if (InstigatorGP != None)
			{
				CurrentElevationPct = InstigatorGP.MortarElevationPct;
			}
		}

		if (CurrentElevationPct >= 1.f)
		{
			// done
			ElevatingFinished();
		}
	}

};

simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	if( FiringMode == FIREMODE_CHARGE )
	{
		CrankStarted();
	}
	else if( PreviousFiringMode == FIREMODE_CHARGE )
	{
		CrankEnded();
	}

	Super.FireModeUpdated(FiringMode, bViaReplication);
}

simulated function CrankStarted()
{
	local GearPawn	MyGearPawn;

	// Play Pawn crank animation.
	MyGearPawn = GearPawn(Instigator);
	if( MyGearPawn != None )
	{
		MyGearPawn.BS_Play(BS_PawnCrank, 1.f, 0.2f, -1.f);
	}

	// Play weapon crank animation.
	PlayWeaponAnim('AR_Mortar_Deployed_Crank', 1.f, 0.f, -1.f);

	// Start crank audio
	CrankAC = GearWeaponPlaySoundLocalEx(CrankSound,, CrankAC, 0.15f);
}

simulated function CrankEnded()
{
	local GearPawn	MyGearPawn;

	// Stop Pawn crank animation
	MyGearPawn = GearPawn(Instigator);
	if( MyGearPawn != None )
	{
		if( MyGearPawn.BS_IsPlaying(BS_PawnCrank) )
		{
			MyGearPawn.BS_SetPlayingFlag(BS_PawnCrank, FALSE);
		}
	}

	// Stop weapon crank animation
	FreezeWeaponAnim();

	if (CrankAC != None)
	{
		CrankAC.FadeOut(0.15f, 0.0f);
	}
}

/** Returns proper set of camera offsets for the current situation. */
simulated event GetMountedCameraOffsets(out vector OffsetLow, out vector OffsetMid, out vector OffsetHigh)
{
	local GearPawn InstigatorGP;

	InstigatorGP = GearPawn(Instigator);

	if( InstigatorGP != None && InstigatorGP.IsPoppingUp() )
	{
		OffsetLow = MountedCameraViewOffsetsLow_OnLowCover;
		OffsetMid = MountedCameraViewOffsetsMid_OnLowCover;
		OffsetHigh = MountedCameraViewOffsetsHigh_OnLowCover;
	}
	else
	{
		OffsetLow = MountedCameraViewOffsetsLow;
		OffsetMid = MountedCameraViewOffsetsMid;
		OffsetHigh = MountedCameraViewOffsetsHigh;
	}
}

/** Returns TRUE if gun is being mounted. Overridden from GearWeap_HeavyBase. */
simulated function bool IsBeingMounted()
{
	local GearPawn	MyGearPawn;
	MyGearPawn = GearPawn(Instigator);

	if (MyGearPawn != None)
	{
		// doing the mount SM but not yet fully mounted
		if ( !bIsMounted && MyGearPawn.IsDoingSpecialMove(SM_TargetMortar) )
		{
			return TRUE;
		}

		// in cover and leaning or popping up
		// this check covers edge cases where the SM hasn't kicked in yet but the fire button registers
		if ( (MyGearPawn.CoverType != CT_None) && MyGearPawn.IsLeaning() )
		{
			return TRUE;
		}
	}

	return FALSE;


}

/** Returns TRUE if gun is being unmounted. Overridden from GearWeap_HeavyBase. */
simulated function bool IsBeingUnMounted()
{
	local GearPawn	MyGearPawn;
	MyGearPawn = GearPawn(Instigator);
	return (MyGearPawn != None) && MyGearPawn.IsDoingSpecialMove(SM_UnMountMortar);
}

simulated function float GetSlottedElevationPct(float Pct)
{
	local float SlottedPct;
	SlottedPct = FFloor(Pct * LaunchElevationNumBeeps) / (float(LaunchElevationNumBeeps) - 1.f);
	return FMin(SlottedPct, 1.f);		// Pct of 1.f can trigger this
}


/** Overridden to distinguish between normal and from-the-hip launches. */
simulated protected function PlayFireSound()
{
	if (IsMounted())
	{
		GearWeaponPlaySoundLocal( FireSound, FireSound_Player,, 1.f );
	}
	else
	{
		GearWeaponPlaySoundLocal( FireSoundUntargeted, FireSoundUntargeted_Player,, 1.f );
	}
}

/** Make sure given fire position is in a good spot.  Returns a guaranteed valid pos. */
simulated protected final function vector ValidateProjectileFirePosition(vector ProjLoc)
{
	local vector StartTrace, EndTrace, HitNorm, HitLoc;
	local Actor HitActor;

	if (Instigator != None)
	{
		StartTrace = Instigator.Location;
		EndTrace = ProjLoc;

		HitActor = Trace(HitLoc, HitNorm, EndTrace, StartTrace, TRUE);
		if (HitActor != None)
		{
			return HitLoc + HitNorm * 8.f;
		}
	}

	return ProjLoc;
}


simulated function GetProjectileFirePosition(out vector out_ProjLoc, out vector out_ProjDir)
{
	local rotator AimRot;
	local float AimError, NewPitch, c;
	local vector TossVelocity;
	local class<GearProj_ClusterMortarBase> ProjClass;

	SkeletalMeshComponent(Mesh).GetSocketWorldLocationAndRotation(MuzzleSocketName, out_ProjLoc, AimRot);

	if (GearAIController != None)
	{
		// figure out what aim the AI wants
		AimRot = GetAdjustedAim(out_ProjLoc);
		if (IsMounted())
		{
			ProjClass = class<GearProj_ClusterMortarBase>(GetProjectileClass());
			Instigator.SuggestTossVelocity( TossVelocity, GearAI(AIController).GetFireTargetLocation(), out_ProjLoc,
				ProjClass.default.MortarLaunchSpeed, 0.0, 0.95,,, Instigator.PhysicsVolume.GetGravityZ() * ProjClass.default.GravityScale * 0.5 );
			// SuggestTossVelocity() doesn't always return the exact velocity requested.
			// Add the extra to Z, figuring that firing a little higher up with a mortar is probably a safe bet
			if (ProjClass.default.MortarLaunchSpeed > VSize(TossVelocity))
			{
				c = VSizeSq(TossVelocity) - Square(ProjClass.default.MortarLaunchSpeed);
				TossVelocity.Z += (-(TossVelocity.Z * 2.0) + Sqrt(Square(TossVelocity.Z * 2.0) - 4.0 * c)) / 2.0;
			}

			AimRot.Pitch = rotator(TossVelocity).Pitch + GearAIController.GetAccuracyRotationModifier(self, 0.25 * FRand()).Pitch;
			AimRot.Pitch = Clamp(rotator(TossVelocity).Pitch, LaunchPitchRange.Y, LaunchPitchRange.X);
		}
		else
		{
			AimRot.Pitch = LaunchPitchUntargeted;
		}
	}
	else
	{
		if (IsMounted())
		{
			NewPitch = GetRangeValueByPct(LaunchPitchRange, GetSlottedElevationPct(CurrentElevationPct));
			AimRot = Instigator.Controller.Rotation;
			AimRot.Pitch = NewPitch;
		}
		else
		{
			// check that muzzle rot retrieved above isn't buried in/through geometry
			out_ProjLoc = ValidateProjectileFirePosition(out_ProjLoc);
		}
	}

	if( Instigator.IsHumanControlled() )
	{
		// modify Aim to add player aim error
		// and convert aim error from degrees to Unreal Units ( * 65536 / 360 )
		AimError = GetPlayerAimError() * 182.044;
		if( AimError > 0 )
		{
			AimRot.Pitch += AimError * (0.5 - FRand());
			AimRot.Yaw	 += AimError * (0.5 - FRand());
		}
	}

	out_ProjDir = vector(AimRot);
}


function bool CanHit(vector ViewPt, vector TestLocation, rotator ViewRotation)
{
	local vector TossVelocity;
	local class<GearProj_ClusterMortarBase> ProjClass;

	ProjClass = class<GearProj_ClusterMortarBase>(WeaponProjectiles[0]);
	return Instigator.SuggestTossVelocity( TossVelocity, TestLocation, ViewPt, ProjClass.default.MortarLaunchSpeed,
		0.0, 0.95,,, Instigator.PhysicsVolume.GetGravityZ() * ProjClass.default.GravityScale * 0.5 );
	return true;
}

/** overridden to handle the untargetted mortar shells */
function class<Projectile> GetProjectileClass()
{
	if( !IsMounted() && (Instigator != None) )
	{
		// Instigator == None is dummyfire
		return UntargetedProjectileClass;
	}

	return super.GetProjectileClass();
}


simulated function PlayWeaponReloading()
{
	WeaponReloadAnim = '';

	Super.PlayWeaponReloading();

	WeaponReloadAnim = (IsMounted() || IsBeingMounted()) ? default.WeaponMountedReloadAnim : default.WeaponReloadAnim;
	if( WeaponReloadAnim != '' )
	{
		PlayWeaponAnimByDuration(WeaponReloadAnim, ReloadDuration, 0.25f, -1.f);
	}
}

simulated function PlayActiveReloadFailed()
{
	WeaponReloadAnimFail = (IsMounted() || IsBeingMounted()) ? default.WeaponMountedReloadFailAnim : default.WeaponReloadAnimFail;
	Super.PlayActiveReloadFailed();
}

/** We use a magic trick, requiring animations to freeze on last frame. So we can't blend out. Sorry. */
simulated function EndReloadAnimations()
{
	// Detach magazine mesh
	Notify_AmmoReload();
}

simulated event WeaponFired(byte FiringMode, optional vector HitLocation)
{
	local GearPawn		MyGearPawn;

	MyGearPawn = GearPawn(Instigator);
	if( IsMounted() )
	{
		PlayWeaponAnim(WeaponFireAnim, 1.f, 0.f, -1.f, FALSE);
		MyGearPawn.BS_Play(BS_MountedFire, 1.f, 0.2f, -1.f);
	}
	else
	{
		MyGearPawn.BS_Play(BS_StandingFire, 1.f, 0.2f, 0.33f);
	}

	Super.WeaponFired(FiringMode, HitLocation);
}

/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab()
{
	local SkeletalMeshSocket	Socket;

	// Set Shadow parent and light environment.
	MortarShell.SetShadowParent(Instigator.Mesh);
	MortarShell.SetLightEnvironment(GearPawn(Instigator).LightEnvironment);
	Socket = Instigator.Mesh.GetSocketByName(GearPawn(Instigator).GetLeftHandSocketName());
	Instigator.Mesh.AttachComponent(MortarShell, Socket.BoneName, Socket.RelativeLocation, Socket.RelativeRotation, Socket.RelativeScale);
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload()
{
	// Set Shadow parent and light environment.
	MortarShell.SetShadowParent(None);
	MortarShell.SetLightEnvironment(None);

	// detach mesh from instigator
	if( Instigator.Mesh != None && Instigator.Mesh.IsComponentAttached(MortarShell) )
	{
		Instigator.Mesh.DetachComponent(MortarShell);
	}
}



defaultproperties
{
	FiringStatesArray(0)="Elevating"

	bCanSelectWithoutAmmo=FALSE

	bReloadingWeaponPreventsTargeting=FALSE
	bNoAnimDelayFiring=TRUE
	bWeaponCanBeReloaded=TRUE
	bIsSuppressive=false

	WeaponFireTypes(0)=EWFT_Projectile
	InstantHitDamageTypes(0)=class'GDT_Mortar'
	AmmoTypeClass=class'GearAmmoType_Boomer'

	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	BS_PawnCrank=(AnimName[BS_MortarMounted]="ADD_HW_Mortar_Deployed_Crank")
	BS_StandingFire=(AnimName[BS_Std_Up]="HW_Mortar_Fire")
	BS_MountedFire=(AnimName[BS_MortarMounted]="ADD_HW_Mortar_Deployed_Fire")
	WeaponFireAnim="AR_Mortar_Deployed_Fire"

	BS_PawnWeaponReload={(
		AnimName[BS_MortarMounted]	="ADD_HW_Mortar_Deployed_Reload",
		AnimName[BS_Std_Up]			="HW_Mortar_Reload",
		AnimName[BS_CovStdIdle_Up]	="HW_Mortar_Cov_Std_Reload",
		AnimName[BS_CovMidIdle_Up]	="HW_Mortar_Cov_Mid_Reload"
	)}

	// No success animations, just speed up the normal one.
	BS_PawnWeaponReloadSuccess={()}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_MortarMounted]	="ADD_HW_Mortar_Deployed_Reload_Fail",
		AnimName[BS_Std_Up]			="HW_Mortar_Reload_Fail",
		AnimName[BS_CovStdIdle_Up]	="HW_Mortar_Cov_Std_Reload_Fail",
		AnimName[BS_CovMidIdle_Up]	="HW_Mortar_Cov_Mid_Reload_Fail"
	)}

	WeaponMountedReloadAnim="HW_Mortar_Deployed_Reload"
	WeaponMountedReloadFailAnim="HW_Mortar_Deployed_Reload_Fail"
	WeaponReloadAnim="HW_Mortar_Reload"
	WeaponReloadAnimFail="HW_Mortar_Reload_Fail"

	// Weapon Mesh Transform
	Begin Object Name=WeaponMesh
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	FireOffset=(X=68,Y=0,Z=9)

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(B=35,G=185,R=255,A=255)
	End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.4f
	MuzzleLightPulseFreq=10
	MuzzleLightPulseExp=1.5
	MuzSmokeParticleSystem=None

	LC_EmisDefaultCOG=(R=0.5,G=1.0,B=2.0,A=1.0)
	LC_EmisDefaultLocust=(R=60.0,G=1.0,B=0.1,A=1.0)

	MortarRotInterpSpeed=10.f

	bIgnoresExecutionRules=true

	WeaponID=WC_Mortar

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformMortarTick1
		Samples(0)=(LeftAmplitude=75,RightAmplitude=75,LeftFunction=WF_LinearIncreasing,RightFunction=WF_LinearIncreasing,Duration=0.3)
	End Object
	MortarTickFF=ForceFeedbackWaveformMortarTick1
}
