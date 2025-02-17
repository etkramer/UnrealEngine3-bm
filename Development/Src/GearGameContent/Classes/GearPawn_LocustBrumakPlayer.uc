/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustBrumakPlayer extends GearPawn_LocustBrumakPlayerBase;

/** Amount to scale small arms fire for player brumak */
var config	float BallisticDamageScale;

/** Last time roar sound was played */
var			float LastRoarTime;
/** Last time melee damage sound was played */
var			float LastDamageTime;

/** List of Roar sound cues */
var array<SoundCue> RoarSound;
var() ScreenShakeStruct	RoarCameraShake;

simulated function UpdateMeshBoneControllers( float DeltaTime )
{
	local GearPC	PC;
	local Actor		HitActor;
	local Vector	HitLocation, HitNormal, CamLoc, StartTrace, EndTrace;
	local Rotator	DeltaRot, CamRot;
	local ImpactInfo Impact;
	local Vector2D	NewAimPct;

	//debug
	//FlushPersistentDebugLines();

	Super.UpdateMeshBoneControllers( DeltaTime );

	// Only update side gun aim locations on server - values replicated to clients
	if( Role == ROLE_Authority )
	{
		AimOffsetViewLoc = GetAimOffsetViewLocation();

		PC = GearPC(Controller);
		if( PC != None )
		{
			PC.GetPlayerViewPoint( CamLoc, CamRot );
			StartTrace	= CamLoc;
			EndTrace	= CamLoc + vector(CamRot)*10000;
			PC.CalcWeaponFire( StartTrace, EndTrace, HitLocation, HitNormal, HitActor, Impact );
		}

		LeftGunAimLocation  = HitLocation;
		RightGunAimLocation = HitLocation;
	}

	// Host or client side handles updating the rotation of the main gun
	PC = (HumanGunner != None) ? GearPC(HumanGunner.Controller) : None;
	if (PC == None || PC.bDeleteMe)
	{
		PC = GearPC(Controller);
	}

	if( PC != None )
	{
		PC.GetPlayerViewPoint( CamLoc, CamRot );
		StartTrace	= CamLoc;
		EndTrace	= CamLoc + vector(CamRot)*10000;
		PC.CalcWeaponFire( StartTrace, EndTrace, HitLocation, HitNormal, HitActor, Impact );

		MainGunAimLocation = HitLocation;
	}

/////// UPDATE ALL THE CONTROLLERS ON CLIENTS AND HOST ///////////
	if( LeftGunAimController != None && !IsZero(LeftGunAimLocation) && !IsDoingASpecialMove()  )
	{
		if( LeftGunAimController.LookAtAlphaTarget != 1.f )
		{
			LeftGunAimController.SetLookAtAlpha(  1.f, LeftGunAimController.BlendInTime );
		}

		LeftGunAimController.TargetLocationInterpSpeed = 15.f;
		LeftGunAimController.SetTargetLocation( LeftGunAimLocation );

		LeftGunAimController.InterpolateTargetLocation( DeltaTime );

		// Update AimOffset
		DeltaRot = Normalize(rotator(LeftGunAimLocation - AimOffsetViewLoc) - Rotation);
		NewAimPct.X = DeltaRot.Yaw	 / 16384.f;
		NewAimPct.Y = DeltaRot.Pitch / 16384.f;
		LeftGunAimPct.X = AimInterpTo(LeftGunAimPct.X, NewAimPct.X, DeltaTime, AimOffsetInterpSpeedHuman / 2.f);
		LeftGunAimPct.Y = AimInterpTo(LeftGunAimPct.Y, NewAimPct.Y, DeltaTime, AimOffsetInterpSpeedHuman / 2.f);

		// Update Recoil Controllers
		IKRecoilCtrl[0].Aim = LeftGunAimPct;

		//debug
		//DrawDebugSphere( LeftGunAimLocation, 20, 10, 255, 0, 0 );
	}

	if( RightGunAimController != None && !IsZero(RightGunAimLocation) && !IsDoingASpecialMove() )
	{
		if( RightGunAimController.LookAtAlphaTarget != 1.f )
		{
			RightGunAimController.SetLookAtAlpha(  1.f, RightGunAimController.BlendInTime );
		}

		RightGunAimController.TargetLocationInterpSpeed = 15.f;
		RightGunAimController.SetTargetLocation( RightGunAimLocation );

		RightGunAimController.InterpolateTargetLocation( DeltaTime );

		// Update AimOffset
		DeltaRot = Normalize(rotator(RightGunAimLocation - AimOffsetViewLoc) - Rotation);
		NewAimPct.X = DeltaRot.Yaw / 16384.f;
		NewAimPct.Y = DeltaRot.Pitch / 16384.f;
		RightGunAimPct.X = AimInterpTo(RightGunAimPct.X, NewAimPct.X, DeltaTime, AimOffsetInterpSpeedHuman / 2.f);
		RightGunAimPct.Y = AimInterpTo(RightGunAimPct.Y, NewAimPct.Y, DeltaTime, AimOffsetInterpSpeedHuman / 2.f);

		// Update Recoil Controllers
		IKRecoilCtrl[1].Aim = RightGunAimPct;
	}

	if( MainGunAimController1 != None && MainGunAimController2 != None && !IsZero(MainGunAimLocation ) && (HumanGunner != None || !IsDoingASpecialMove()) )
	{
		if( MainGunAimController1.LookAtAlphaTarget != 1.f )
		{
			MainGunAimController1.SetLookAtAlpha( 1.f, MainGunAimController1.BlendInTime );
			MainGunAimController2.SetLookAtAlpha( 1.f, MainGunAimController2.BlendInTime );
		}

		MainGunAimController1.TargetLocationInterpSpeed = 15.f;
		MainGunAimController2.TargetLocationInterpSpeed = 15.f;
		MainGunAimController1.SetTargetLocation( MainGunAimLocation );
		MainGunAimController2.SetTargetLocation( MainGunAimLocation );

		MainGunAimController1.InterpolateTargetLocation( DeltaTime );
		MainGunAimController2.InterpolateTargetLocation( DeltaTime );

		//debug
		//DrawDebugSphere( MainGunAimLocation, 20, 10, 0, 255, 0 );
	}
}

function SetDriver( GearPawn NewDriver )
{
	local GearPawn OldDriver;

	//debug
 	`DLog("Driver:" @ Driver @ "NewDriver:" @ NewDriver);

	OldDriver = Driver;
	// Assign new driver
	Driver = NewDriver;
	bForceNetUpdate = TRUE;

	if( Driver != None )
	{
		Driver.bCollideWorld = FALSE;
		Driver.SetCollision( FALSE, FALSE, TRUE );
		Health = DefaultHealth;

		Driver.SetPhysics( PHYS_None );
		Driver.SetOwner( self );
		Driver.SetBase( None );
		Driver.SetBase( self,, Mesh, DriverSocket );

		Driver.Mesh.SetShadowParent(Mesh);
		Driver.Mesh.SetLightEnvironment(LightEnvironment);
		Driver.LightEnvironment.SetEnabled(FALSE);

		Driver.Mesh.bUpdateSkelWhenNotRendered = FALSE;
		Driver.Mesh.MinDistFactorForKinematicUpdate = 10.0; // Stop physics updates even when seen

		Driver.InvManager.DiscardInventory();
		InvManager.DiscardInventory();
		CreateInventory( class'GearGameContent.GearWeap_BrumakSideGun_Player' );

		DoDriverAttachment(Driver);

		LeftGunAimController.SetLookAtAlpha(  1.f, 0.1f );
		RightGunAimController.SetLookAtAlpha( 1.f, 0.1f );
		MainGunAimController1.SetLookAtAlpha( 1.f, 0.1f );
		MainGunAimController2.SetLookAtAlpha( 1.f, 0.1f );
	}
	else
	{
		LeftGunAimController.SetLookAtAlpha(  0.f, 0.1f );
		RightGunAimController.SetLookAtAlpha( 0.f, 0.1f );

		if( OldDriver != None )
		{
			OldDriver.SetOwner( None );
			OldDriver.SetBase( None );

			OldDriver.Mesh.SetShadowParent(None);
			OldDriver.Mesh.SetLightEnvironment(OldDriver.LightEnvironment);
			OldDriver.LightEnvironment.SetEnabled(TRUE);

			OldDriver.Mesh.bUpdateSkelWhenNotRendered = TRUE;
			OldDriver.Mesh.MinDistFactorForKinematicUpdate = OldDriver.default.Mesh.MinDistFactorForKinematicUpdate;

			OldDriver.InvManager.DiscardInventory();
			OldDriver.AddDefaultInventory();

			DoDriverDetatch( OldDriver );
		}
	}
}

function SetGunner( Pawn NewGunner, optional int Side )
{
	local GearPawn OldGunner;

	if( Side != 0 )
		return;

	if( HumanGunner != None )
	{
		OldGunner = HumanGunner;
	}

	//debug
	`DLog("LeftGunPawn:" @ LeftGunPawn @ "RightGunPawn:" @ RightGunPawn @ "NewGunner:" @ NewGunner @ "Side:" @ Side @ "HumanGunner:" @ HumanGunner @ "OldGunner:" @ OldGunner);

	if( NewGunner != None && NewGunner.IsHumanControlled() )
	{
		SetHumanGunner( GearPawn(NewGunner) );
	}
	else
	{
		SetHumanGunner( None );
	}

	RightGunTarget	= None;
	LeftGunTarget	= None;

	LeftGunPawn  = NewGunner;
	RightGunPawn = NewGunner;

	if( NewGunner != None )
	{
		`DLog("Attach NewGunner:" @ NewGunner @ "HumanGunner:" @ HumanGunner @ "OldGunner:" @ OldGunner);
		NewGunner.SetPhysics( PHYS_None );
		NewGunner.SetBase( None );
		NewGunner.SetBase( self,, Mesh, DriverSocket );
		NewGunner.bCollideWorld = FALSE;
		NewGunner.SetCollision( TRUE, FALSE, TRUE );
		NewGunner.SetHidden( TRUE );

		NewGunner.Mesh.SetShadowParent(Mesh);
		NewGunner.Mesh.SetLightEnvironment(LightEnvironment);
		GearPawn(NewGunner).LightEnvironment.SetEnabled(FALSE);

		NewGunner.Mesh.bUpdateSkelWhenNotRendered = FALSE;
		NewGunner.Mesh.bNoSkeletonUpdate = TRUE;

		NewGunner.InvManager.DiscardInventory();
		NewGunner.CreateInventory( class'GearGameContent.GearWeap_BrumakMainGun_Player' );

		MainGunAimController1.SetLookAtAlpha( 1.f, 0.1f );
		MainGunAimController2.SetLookAtAlpha( 1.f, 0.1f );
	}
	else
	{
		`DLog("Detaching OldGunner:" @ OldGunner);

		if( OldGunner != None )
		{
			OldGunner.SetBase( None );
			OldGunner.SetHidden( FALSE );

			OldGunner.Mesh.SetShadowParent(None);
			OldGunner.Mesh.SetLightEnvironment(OldGunner.LightEnvironment);
			OldGunner.LightEnvironment.SetEnabled(TRUE);

			OldGunner.Mesh.bUpdateSkelWhenNotRendered = TRUE;
			OldGunner.Mesh.bNoSkeletonUpdate = FALSE;

			OldGunner.InvManager.DiscardInventory();
			OldGunner.AddDefaultInventory();

			OldGunner = None;
		}

		MainGunAimController1.SetLookAtAlpha( 0.f, 0.1f );
		MainGunAimController2.SetLookAtAlpha( 0.f, 0.1f );
	}
}

/** Turn off physics collision for dom/marcus when attached to brumak */
simulated event Attach(Actor Other)
{
	local GearPawn_Infantry GP;

	Super.Attach(Other);

	GP = GearPawn_Infantry(Other);
	if(GP != None)
	{
		GP.Mesh.SetBlockRigidBody(FALSE);
	}
}

/** Turn physics collision back on when detaching marcus/dom */
simulated event Detach(Actor Other)
{
	local GearPawn_Infantry GP;

	Super.Detach(Other);

	GP = GearPawn_Infantry(Other);
	if(GP != None)
	{
		GP.Mesh.SetBlockRigidBody(GP.default.Mesh.BlockRigidBody);
	}
}

event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local GearPawn GP;

	GP = GearPawn(Other);
	// the extra failsafes are to (a) make sure we don't catch COG in the middle of spawning, before their team is assigned, and
	// (b) to make sure we don't disrupt checkpoint loads, which need to spawn player pawns before mounting them to the brumak
	if ( GP != None && GP.Base != self && !IsSameTeam(GP) && GearPawn_COGGear(GP) == None &&
		(GearGameSP_Base(WorldInfo.Game) == None || !GearGameSP_Base(WorldInfo.Game).bCheckpointLoadInProgress) )
	{
		GP.Died( Controller, class'GDT_Melee', GP.Location );
	}
	else
	{
		Super.Touch( Other, OtherComp, HitLocation, HitNormal );
	}
}


function AdjustPawnDamage
(
	out	int					Damage,
	Pawn				InstigatedBy,
	Vector				HitLocation,
	out Vector				Momentum,
	class<GearDamageType>	GearDamageType,
	optional	out	TraceHitInfo		HitInfo
	)
{
	//	`log( self@GetFuncName()@GearDamageType@GearDamageType.default.WeaponID@Damage );
	//	ScriptTrace();


	if( GearDamageType != None )
	{
		switch( GearDamageType.default.WeaponID )
		{
		case WC_Lancer:
		case WC_Snub:
		case WC_Longshot:
		case WC_Hammerburst:
		case WC_Boltock:
		case WC_LocustBurstPistol:
		case WC_Gnasher:
		case WC_Scorcher:
		case WC_Minigun:
			Damage *= BallisticDamageScale;
			break;
		}
	}

	//	`log( "..."@Damage );

	Super.AdjustPawnDamage( Damage, InstigatedBy, HitLocation, Momentum, GearDamageType, HitInfo );
}

simulated function bool ShouldPlayFootStepWaveform() 
{
	return FALSE;
}

simulated function PlayRoar()
{
	local GearPC PC;

	if( TimeSince(LastRoarTime) > 3.f )
	{
		LastRoarTime = WorldInfo.TimeSeconds;

		if( PSC_BreathEffect == None )
		{
			PSC_BreathEffect = new(self) class'ParticleSystemComponent';
			PSC_BreathEffect.SetTemplate( PS_BreathEffect );
			PSC_BreathEffect.SetScale( DrawScale );
			PSC_BreathEffect.bAutoActivate = FALSE;
			Mesh.AttachComponent( PSC_BreathEffect, 'head', vect(0,0,0),,);
		}
		PSC_BreathEffect.SetHidden( FALSE );
		PSC_BreathEffect.ActivateSystem();

		PlaySound( RoarSound[Rand(RoarSound.Length)], TRUE );

		foreach LocalPlayerControllers( class'GearPC', PC )
		{
			PC.ClientPlayCameraShake(RoarCameraShake);
		}
	}
}

function NotifyBrumakSmashCollision( Actor Other, Vector HitLocation )
{
	// Only do melee damage to world/pawns/movers
	if( !Other.bStatic &&
		!Other.IsA( 'Pawn' ) &&
		!Other.IsA( 'InterpActor' ) &&
		!Other.IsA( 'FracturedStaticMeshActor' ) )
	{
		return;
	}

	Super.NotifyBrumakSmashCollision( Other, HitLocation );

	if( IsDoingSpecialMove( GSM_Brumak_OverlayLftArmSwing ) ||
		IsDoingSpecialMove( GSM_Brumak_OverlayRtArmSwing  ) )
	{
		if( TimeSince(LastDamageTime) > 3.f )
		{
			LastDamageTime = WorldInfo.TimeSeconds;
			PlaySound( MeleeImpactSound );
		}
	}
}

simulated function PlayShoulderMeleeCameraAnim()
{
	SpecialMoves[SpecialMove].PlayCameraAnim( self, CameraAnim'Locust_Brumak.Camera_Anims.Smash_Melee_Cam',,, 0.2f, 0.2f );
}

/** @hack: for some reason the brumak triggers OutsideWorldBounds in the first frame only in final release
 * this code basically causes it to be ignored
 */
final function ClosureHack()
{
	SetCollision(true, true);
	SetPhysics(PHYS_Falling);
}

simulated singular event OutsideWorldBounds()
{
	SetTimer(0.01, false, nameof(ClosureHack));
}

defaultproperties
{
	bCanBeBaseForPawns=TRUE
	bNoEncroachCheck=TRUE
	bBlockCamera=FALSE

	SpecialMoveClasses(GSM_Brumak_OverlayLftArmSwing)	=class'GSM_Brumak_MeleeDoor_Player'
	SpecialMoveClasses(GSM_Brumak_OverlayRtArmSwing)	=class'GSM_Brumak_Melee_Player'

	ControllerClass=None
	CharacterFootStepType=CFST_Locust_RideableBrumak

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Brumak.BrumakMesh'
		PhysicsAsset=PhysicsAsset'Locust_Brumak.BrumakMesh_Physics'
		AnimTreeTemplate=AnimTree'Locust_Brumak.Brumak_AnimTree_Player'
		AnimSets(0)=AnimSet'Locust_Brumak.Brumak_Animset'
		bHasPhysicsAssetInstance=TRUE
		bEnableFullAnimWeightBodies=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		Translation=(X=-128,Y=0,Z=-404)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=450.0
		CollisionHeight=400.0
		BlockActors=FALSE
		CollideActors=FALSE
		BlockNonZeroExtent=FALSE
		BlockZeroExtent=FALSE
	End Object

	Begin Object Class=StaticMeshComponent Name=RightGunSMC
		StaticMesh=StaticMesh'Locust_Brumak.right_gun'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	RightGunComponent=RightGunSMC
	Components.Add(RightGunSMC)

	Begin Object Class=StaticMeshComponent Name=LeftGunSMC
		StaticMesh=StaticMesh'Locust_Brumak.left_gun'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	LeftGunComponent=LeftGunSMC
	Components.Add(LeftGunSMC)

	Begin Object Class=StaticMeshComponent Name=MaskSMC
		StaticMesh=StaticMesh'Locust_Brumak.Helmet'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	MaskComponent=MaskSMC
	Components.Add(MaskSMC)


	DriverAnimTree=AnimTree'Locust_Brumak.BrumakRider_AnimTree'
	DriverAnimSet=AnimSet'Locust_Brumak.Rider_animset'
	DriverSocket="Driver"
	LeftMuzzleSocketName=LeftMuzzle
	RightMuzzleSocketName=RightMuzzle
	NeckBoneName=head

	PeripheralVision=-0.1
	SightBoneName="head"

	LandMovementState=PlayerBrumakDriver
	GunnerState=PlayerBrumakGunner


	PS_FootstepDust=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Foot_Effect'
	PS_BreathEffect=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Breath_Effect'

	FootSound=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_FootstepPlayer_Cue'
	RoarSound(0)=SoundCue'Locust_Brumak_Efforts.BrumakEfforts.BrumakVocalScreamButton_Cue'
	MeleeImpactSound=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_ArmImpact_Cue'

	RoarCameraShake=(FOVAmplitude=1.5,FOVFrequency=100,FOVParam=ESP_OffsetZero,LocAmplitude=(X=1,Y=1,Z=1),LocFrequency=(X=5,Y=2,Z=75),RotAmplitude=(X=50,Y=5,Z=100),RotFrequency=(X=100,Y=5,Z=50),TimeDuration=2.0)

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustBrumak'

	// maybe this should be increased?
//	MaxStepHeight=35.0
	MaxStepHeight=150

//	bDebug=TRUE
}
