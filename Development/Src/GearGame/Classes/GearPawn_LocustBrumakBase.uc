
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GearPawn_LocustBrumakBase extends GearPawn
	native(Pawn)
	config(Pawn)
	native
	abstract;

cpptext
{
	virtual FVector GetIdealCameraOrigin()
	{
		if( Driver )
		{
			return Driver->GetIdealCameraOrigin();
		}
		return Super::GetIdealCameraOrigin();
	}

	virtual void	TickSpecial(FLOAT DeltaSeconds);

	virtual UBOOL	IgnoreBlockingBy( const AActor* Other ) const;
}

/////// RIDER VARIABLES /////////
/** GearPawn that is the driver */
var repnotify GearPawn	Driver;
var			  GearPawn	BackupDriver;
/** GearPawn class to spawn for Driver */
var class<GearPawn>		DriverClass;
/** Name of the socket driver should be attached to */
var Name				DriverSocket;
var AnimTree			DriverAnimTree;
var AnimSet				DriverAnimSet;
/** Actor main gun is currently aiming at */
var Actor				MainGunTarget;
/** Location main gun is aiming */
var Vector				MainGunAimLocation;
/** Controllers used to aim main gun toward target */
var SkelControlLookAt	MainGunAimController1,
						MainGunAimController2;
/** How much damage the legs can take before exposing the driver */
var	config	float		LegHealth;
/** How long it takes legs to fully heal */
var config	float		LegHealTime;
/** Last time the driver was exposed */
var			float		LastExposeDriverTime;
/** Min time before driver can be exposed after the last time */
var	config	float		MinDriverSafeTime;
/** The spawned POI for the driver */
var GearPointOfInterest	DriverPOI;
/** Whether the driver POI has enable yet or not - so that it isn't enable again */
var bool				bDriverPOIEnabled;
/** The spawned POI for the legs */
var GearPointOfInterest	LegPOI;
/** Whether the leg POI has enable yet or not - so that it isn't enable again */
var bool				bLegPOIEnabled;
/** Flinching b/c of kismet control */
var	bool				bFlinching;

/** Damage head can take, starts affecting accuracy */
var config float		HeadHealth;
/** How long it takes head to fully heal */
var config float		HeadHealTime;
/** Main Gun Health */
var config float		MainGunHealth;

/** Brumak is capable of firing rockets */
var bool				bCanFireRockets;

var repnotify	byte	RoarCount;

/////// SIDE GUN VARIABLES ///////
var class<Pawn>					SideGunClass;
var Pawn						RightGunPawn,
								LeftGunPawn;
var GearPawn					HumanGunner;
/** How much damage the side guns can continue to take */
var config	int					LeftGunHealth,
								RightGunHealth;
/** Name of sockets for weapon muzzle flash */
var			Name				LeftMuzzleSocketName,
								RightMuzzleSocketName;
/** Controllers used to aim side guns toward targets */
var SkelControlLookAt			LeftGunAimController,
								RightGunAimController;
/** Actors guns are currently aiming at */
var Actor						LeftGunTarget,
								RightGunTarget;
/** Locations that side guns are aiming */
var Vector						LeftGunAimLocation,
								RightGunAimLocation;
var Vector						AimOffsetViewLoc;
/** Number of times left before to be "hurt" before guns are destroyed */
var config Byte					LeftGunLives,
								RightGunLives;
/** Amount of damage to deal to Brumak when a side gun is destroyed */
var config int					SideGunDestroyedHitPointPenalty;
/** Name of state for player controller to be in while gunning */
var Name						GunnerState;
/** AimPct */
var Vector2D					LeftGunAimPct;
var	Vector2D					RightGunAimPct;

var Array<GearSkelCtrl_IKRecoil>	IKRecoilCtrl;
/** Blend nodes to enable/disable arm aim offset nodes */
var AnimNodeBlend				LeftArmAimOffsetBlend, RightArmAimOffsetBlend;
var GearAnim_BlendByRefRelative	MainGunFireAdditiveBlend;

/** FOVs for brumak guns */
var() float						SideGunConeAngle;
var() Vector					SideGunOriginOffset;
var() float						MainGunConeAngle;
var() Vector					MainGunOriginOffset;

/** Mesh components for attached pieces */
var StaticMeshComponent LeftGunComponent, RightGunComponent;
var StaticMeshComponent MaskComponent;

/** Turn in place player */
var GearAnim_TurnInPlace_Player	TurnPlayer;

// Damage dealing limbs
struct native DmgDealingLimbStruct
{
	/** Bone Name/Socket Name */
	var()				Name	BoneName;
	/** Extent, for bounding box size */
	var()				float	BoxSize;
	/** Last known location. */
	var		transient	vector	LastLocation;
};

/** Array of damage dealing limbs. */
var() Array<DmgDealingLimbStruct>	DmgDealingLimbs;

/** The PS to play when the brumak is moving.  Played at each foot step **/
var ParticleSystem PS_FootstepDust;
var SoundCue FootSound;
var ParticleSystem PS_BreathEffect;
var ParticleSystemComponent PSC_BreathEffect;


var SoundCue DyingSound;

var vector	OldCamPosition;
var	rotator	OldCamRotation;

//////////////////////////////////////////////////////////////////////////
// CENTAUR REPULSION

var()	bool	bEnableCentaurRepulsion;
var()	bool	bShowCentaurRepulsion;
var()	float	CentaurRepulsionRadius;
var()	float	CentaurRepulsionStrength;

replication
{
	if( Role == ROLE_Authority )
		Driver, bCanFireRockets, HumanGunner, AimOffsetViewLoc, LeftGunAimLocation, RightGunAimLocation, RoarCount;
}

simulated native event vector GetPhysicalFireStartLoc( vector FireOffset );

simulated event Vector GetWeaponStartTraceLocation(optional Weapon CurrentWeapon)
{
	return GetPhysicalFireStartLoc(vect(0,0,0));
}

simulated function PostBeginPlay()
{
	local int i, NumMaterials;

	Super.PostBeginPlay();

	// Create MICs for each material, so we can modify them for gore
	NumMaterials = Mesh.SkeletalMesh.Materials.length;
	for(i=0; i<NumMaterials; i++)
	{
		Mesh.CreateAndSetMaterialInstanceConstant(i);
	}

	// Increase foot steps volume
	FootSound.VolumeMultiplier = 5.0;
}

simulated function Destroyed()
{
	super.Destroyed();

	if( BackupDriver != None )
	{
		DoDriverDetatch( BackupDriver );
		BackupDriver = None;
	}

	// Clean up
	if( Driver != None )
	{
		Driver.Destroy();
		Driver = None;
	}
	if( LeftGunPawn != None )
	{
		LeftGunPawn.Destroy();
		LeftGunPawn = None;
	}
	if( RightGunPawn != None )
	{
		RightGunPawn.Destroy();
		RightGunPawn = None;
	}
}

simulated event ReplicatedEvent( Name VarName )
{
	if( VarName == 'Driver' )
	{
		if( Driver != None )
		{
			DoDriverAttachment(Driver);
			BackupDriver = Driver;
		}
		else
		{
			DoDriverDetatch( BackupDriver );
			BackupDriver = None;
		}
	}
	else
	if( VarName == 'RoarCount' )
	{
		if( RoarCount != 0 )
		{
			PlayRoar();
		}
	}
	else
	{
		Super.ReplicatedEvent( VarName );
	}
}

simulated function DoDriverAttachment(GearPawn InDriver)
{
	local int i;

// 	`DLog("InDriver:" @ InDriver);

	// Attach the driver to the correct socket
	InDriver.SetBase(Self,, Mesh, DriverSocket );

	// fix me
	InDriver.Mesh.SetTranslation( vect(0,0,0) );
	InDriver.bDisableMeshTranslationChanges = TRUE;

	if( DriverAnimTree != None && DriverAnimTree != InDriver.Mesh.AnimTreeTemplate )
	{
		// Backup his original Animation Tree.
		InDriver.RecoverFromRagdollAnimTree = InDriver.Mesh.AnimTreeTemplate;
		// Set driver AnimTree
		InDriver.Mesh.SetAnimTreeTemplate(DriverAnimTree);
		// Update AnimNodes
		InDriver.ClearAnimNodes();
		InDriver.CacheAnimNodes();

		// Add necessary AnimSets
		for (i = 0; i < ArrayCount(InDriver.KismetAnimSets); i++)
		{
			if (InDriver.KismetAnimSets[i] == None)
			{
				InDriver.KismetAnimSets[i] = DriverAnimSet;
				break;
			}
		}
		InDriver.UpdateAnimSetList();
	}
}

simulated function DoDriverDetatch( GearPawn OldDriver )
{
	local int i;

// 	`DLog("OldDriver:" @ OldDriver);

	OldDriver.Mesh.SetTranslation( OldDriver.default.Mesh.Translation );
	OldDriver.bDisableMeshTranslationChanges = FALSE;

	// Restore driver AnimTree
	if( OldDriver.RecoverFromRagdollAnimTree != None &&
		OldDriver.Mesh.AnimTreeTemplate != OldDriver.RecoverFromRagdollAnimTree )
	{
		OldDriver.Mesh.SetAnimTreeTemplate( OldDriver.RecoverFromRagdollAnimTree );
		OldDriver.RecoverFromRagdollAnimTree = None;

		// Update AnimNodes
		OldDriver.ClearAnimNodes();
		OldDriver.CacheAnimNodes();
	}

	// Remove Driver AnimSets
	for(i = 0; i < ArrayCount(OldDriver.KismetAnimSets); i++)
	{
		if( OldDriver.KismetAnimSets[i] == DriverAnimSet )
		{
			OldDriver.KismetAnimSets[i] = None;
			break;
		}
	}

	OldDriver.UpdateAnimSetList();
}

function SetHumanGunner( GearPawn GP )
{
	HumanGunner = GP;
}

/** Find lookat controllers for side guns */
simulated function CacheAnimNodes()
{
	local AnimNode	Node;

	Super.CacheAnimNodes();

	foreach Mesh.AllAnimNodes(class'AnimNode', Node)
	{
		if( Node.NodeName == 'LeftArmAimOffsetBlend' )
		{
			LeftArmAimOffsetBlend = AnimNodeBlend(Node);
		}
		else if( Node.NodeName == 'RightArmAimOffsetBlend' )
		{
			RightArmAimOffsetBlend = AnimNodeBlend(Node);
		}
		else if( Node.NodeName == 'MainGunFireAdditiveBlend' )
		{
			MainGunFireAdditiveBlend = GearAnim_BlendByRefRelative(Node);
			MainGunFireAdditiveBlend.SetBlendTarget(0.f, 0.f);
		}
		else if( Node.NodeName == 'TurnPlayer' )
		{
			TurnPlayer = GearAnim_TurnInPlace_Player(Node);
		}
	}

	LeftGunAimController  = SkelControlLookAt(Mesh.FindSkelControl('LeftGunAimController'));
	RightGunAimController = SkelControlLookAt(Mesh.FindSkelControl('RightGunAimController'));
	MainGunAimController1 = SkelControlLookAt(Mesh.FindSkelControl('MainGunAimController1'));
	MainGunAimController2 = SkelControlLookAt(Mesh.FindSkelControl('MainGunAimController2'));

	LeftGunAimController.SetSkelControlActive( TRUE );
	LeftGunAimController.SetLookAtAlpha( 0, 0 );
	RightGunAimController.SetSkelControlActive( TRUE );
	RightGunAimController.SetLookAtAlpha( 0, 0 );
	MainGunAimController1.SetSkelControlActive( TRUE );
	MainGunAimController1.SetLookAtAlpha( 0, 0 );
	MainGunAimController2.SetSkelControlActive( TRUE );
	MainGunAimController2.SetLookAtAlpha( 0, 0 );

	IKRecoilCtrl[0] = GearSkelCtrl_IKRecoil(Mesh.FindSkelControl( 'LeftGunRecoil' ));
	IKRecoilCtrl[1] = GearSkelCtrl_IKRecoil(Mesh.FindSkelControl( 'RightGunRecoil' ));
}

/** Clear lookat controllers for side guns */
simulated function ClearAnimNodes()
{
	Super.ClearAnimNodes();

	LeftArmAimOffsetBlend		= None;
	RightArmAimOffsetBlend		= None;
	MainGunFireAdditiveBlend	= None;

	LeftGunAimController  = None;
	RightGunAimController = None;
	MainGunAimController1 = None;
	MainGunAimController2 = None;
}

/** Set the actors the side guns should aim at */
final simulated function SetSideGunAimTarget( Actor NewTarget, bool bLeftGun )
{
	if( bLeftGun )
	{
		//debug
		`log( GetFuncName()@"LEFT"@LeftGunTarget@NewTarget@IsLeftGunDestroyed(), bDebug);

		LeftGunTarget = (!IsLeftGunDestroyed()) ? NewTarget : None;
	}
	else
	{
		//debug
		`log( GetFuncName()@"RIGHT"@RightGunTarget@NewTarget@IsRightGunDestroyed(),bDebug);

		RightGunTarget = (!IsRightGunDestroyed()) ? NewTarget : None;
	}
}

/** Set the actor the main gun should aim at */
final simulated function SetMainGunAimTarget( Actor NewTarget )
{
	//debug
	`log( GetFuncName()@"MAIN"@MainGunTarget@NewTarget@Driver,bDebug);

	MainGunTarget =  (Driver != None) ? NewTarget : None;
}

/** Overriden to bypass all the drone specific rules, and use straight known location interpolation */
simulated function Vector GetAIAimOffsetFireTargetLocation(GearAI AI)
{
	return AI.GetFireTargetLocation(LT_InterpVisibility);
}

/** Given a target to aim at, get the location to aim at */
simulated function vector GetGunLookTargetLocation( Actor TargActor, SkelControlLookAt SkelController )
{
	local GearAI_Brumak	AI;
	local Pawn			TargetPawn;

	if( TargActor != None )
	{
		AI = GearAI_Brumak(Controller);
		TargetPawn = Pawn(TargActor);
		if( AI != None && TargetPawn != None )
		{
			if( TargetPawn.IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
			{
				return AI.GetEnemyLocation( TargetPawn, LT_InterpVisibility );
			}
			else
			{
				SkelController.SetLookAtAlpha( 0.f, SkelController.BlendOutTime );
				return SkelController.DesiredTargetLocation;
			}
		}

		return TargActor.Location;
	}

	return SkelController.DesiredTargetLocation;
}

/** Returns TRUE if main gun is disabled */
final function bool IsMainGunDestroyed()
{
	return (Driver == None);
}

/** Returns TRUE if left gun is disabled */
final function bool IsLeftGunDestroyed()
{
	return (LeftGunHealth <= 0);
}

/** Returns TRUE if right gun is disabled */
final function bool IsRightGunDestroyed()
{
	return (RightGunHealth <= 0);
}

function float GetHeadDamageInaccuracyScale();

function AddDefaultInventory();

simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	MyGearWeapon = GearWeapon(Weapon);
}

/** Stub functions */
function SetDriver( GearPawn NewDriver );
function SetGunner( Pawn NewGunner, optional int Side );
function NotifyDriverDied( Pawn DeadPawn, Controller Killer );
function Pawn GetOtherGunEnemy( GearPawn_LocustBrumak_SideGun ThisGun );

/** Brumak can't be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

simulated function SetAttachmentVisibility(EAttachSlot Slot, class<Inventory> AttachClass);

/** Returns ViewLocation to be used by AimOffset, for AI characters, when tracking a Target. */
simulated function vector GetAimOffsetViewLocation()
{
	local Vector RoughHeadLocation, ViewLocation;

	// Get rough head location
	// We prefer Spine2 instead of Head or Neck, because it's about the same height,
	// but it doesn't move as much around as these other bones when aimoffsets change.
	RoughHeadLocation = Mesh.GetBoneLocation('Spine2');

	// RoughHeadLocation could be outside of the collision cylinder, so we just use its height as an indication.
	// We need something stable for AimOffsets, and it doesn't have to be that precise.
	ViewLocation	= Location;
	ViewLocation.Z	= RoughHeadLocation.Z;

	// Set BaseEyeHeight for LineOfSight checks
	BaseEyeHeight = ViewLocation.Z - Location.Z;

	return ViewLocation;
}

event BaseChange()
{
	local Pawn BasePawn;

	BasePawn = Pawn(Base);
	if( BasePawn != None )
	{
		BasePawn.Died( Controller, class'GDT_Melee', BasePawn.Location );
		SetPhysics( PHYS_Falling );
		SetBase( None );
	}
	else
	{
		Super.BaseChange();
	}
}

simulated event Bump( Actor Other, PrimitiveComponent OtherComp, Vector HitNormal )
{
	local Pawn OtherPawn;

	OtherPawn = Pawn(Other);
	if( !IsZero(Velocity) &&
		OtherPawn != None &&
		!OtherPawn.IsSameTeam(self) )
	{
		OtherPawn.TakeDamage( 50, Controller, OtherPawn.Location, vect(0,0,0), class'GDT_Melee' );
	}

	Super.Bump( Other, OtherComp, HitNormal );
}

event RanInto( Actor Other )
{
	local Vehicle_Centaur_Base V;
	local Vector	VectToVehicle;
	local float		VDotRot, VDotVel;

	V = Vehicle_Centaur_Base(Other);
	if( V != None && !V.IsSameTeam( self ) )
	{
		VectToVehicle = Normal(V.Location - Location);
		VDotRot = VectToVehicle DOT Vector(Rotation);
		VDotVel = VectToVehicle DOT V.Velocity;

		V.TakeDamage( 25, Controller, V.Location, vect(0,0,0), class'GDT_Melee' );
		TakeDamage( 250, V.Controller, Location, -VectToVehicle, class'GDT_Melee' );

		if( Health > 0 && VDotRot > 0.3f && VDotVel < -0.3 )
		{
			PlayStumbleAnim( FALSE );
		}
	}
}

function bool PlayStumbleAnim( bool bLatent )
{
	bFlinching = bLatent;
	if( !IsDoingSpecialMove( SM_StumbleGoDown ) )
	{
		DoSpecialMove( SM_StumbleGoDown, TRUE );
		return TRUE;
	}
	return FALSE;
}

function NotifyBrumakSmashCollision( Actor Other, Vector HitLocation )
{
	local Controller InstigatedBy;
	local FracturedStaticMeshActor FracActor;

	//debug
	`log( GetFuncName()@Other@SpecialMove@Controller, bDebug );

	if( IsDoingSpecialMove( GSM_Brumak_MeleeAttack		  ) ||
		IsDoingSpecialMove( GSM_Brumak_OverlayLftArmSwing ) ||
		IsDoingSpecialMove( GSM_Brumak_OverlayRtArmSwing  ) )
	{
		//debug
		`log("doing damage to"@Other,bDebug);

		InstigatedBy = Controller;
		if( LeftGunPawn != None && LeftGunPawn.IsHumanControlled() )
		{
			InstigatedBy = LeftGunPawn.Controller;
		}

		Other.TakeDamage( 1000.f, InstigatedBy, Other.Location, vect(0,0,0), class'GDT_Brumak_Smash' );

		FracActor = FracturedStaticMeshActor(Other);
		if( FracActor != None && FracActor.Physics == PHYS_None && FracActor.IsFracturedByDamageType(class'GDT_Brumak_Smash') )
		{
			FracActor.BreakOffPartsInRadius( HitLocation, 512.f, 600.f, TRUE );
		}
	}
}

function NotifyBrumakBiteCollision( Actor Other, Vector HitLocation )
{
	local Controller InstigatedBy;

	//debug
	`log( GetFuncName()@Other@SpecialMove,bDebug);

	if( IsDoingSpecialMove( GSM_Brumak_OverlayBite ) )
	{
		`log("doing damage to"@Other,bDebug);

		InstigatedBy = Controller;
		if( LeftGunPawn != None && LeftGunPawn.IsHumanControlled() )
		{
			InstigatedBy = LeftGunPawn.Controller;
		}

		Other.TakeDamage( 1000.f, InstigatedBy, Other.Location, vect(0,0,0), class'GDT_Brumak_Bite' );

//		PlaySound(SoundCue'Foley_BodyMoves.BodyMoves.BerserkerArmImpactFlesh_Cue'); //replace
	}
}

function NotifyBrumakRoar()
{
	local GearPawn	EnemyPawn;
	local float		DistSq, DotP, DamageScale, Damage;
	local Controller InstigatedBy;

	//debug
	`log( GetFuncName()@SpecialMove,bDebug );

	if( IsDoingSpecialMove( GSM_Brumak_Roar ) )
	{
		InstigatedBy = Controller;
		if( LeftGunPawn != None && LeftGunPawn.IsHumanControlled() )
		{
			InstigatedBy = LeftGunPawn.Controller;
		}

		foreach WorldInfo.AllPawns( class'GearPawn', EnemyPawn )
		{
			if( !InstigatedBy.Pawn.IsSameTeam(EnemyPawn) )
			{
				DistSq = VSizeSq(EnemyPawn.Location-Location);
				DotP = Normal(EnemyPawn.Location-Location) DOT vector(Rotation);
				if( DistSq <= (768*768) && DotP > 0.85f )
				{
					DamageScale = DistSq / (768*768);
					Damage		= 100.f * DamageScale;

					//debug
					`log( ">>>> Damage"@EnemyPawn@DamageScale@Damage,bDebug );

					EnemyPawn.TakeDamage(Damage, InstigatedBy, EnemyPawn.Location, vect(0,0,0), class'GDT_Brumak_Roar' );
				}
			}
		}
	}
}

final function GetGunFOVParameters(vector InOffset, float InDegAngle, out vector OutOrigin, out vector OutOrientation, out float OutRadAngle, optional bool bLeftSide)
{
	local vector X, Y, Z;

	GetAxes(Rotation, X, Y, Z);
	OutOrigin		= Location + InOffset.X * X + InOffset.Y * Y * (bLeftSide ? -1.f : 1.f) + InOffset.Z * Z;
	OutOrientation	= Vector(Rotation);
	OutRadAngle		= InDegAngle * DegToRad;
}

final function bool IsWithinGunFOV(vector InOffset, float InDegAngle, vector TestLocation, optional bool bLeftSide)
{
	local Vector	OutOrigin, OutOrientation;
	local float		OutRadAngle, DotAngle;

	// Get FOV parameters.
	GetGunFOVParameters(InOffset, InDegAngle, OutOrigin, OutOrientation, OutRadAngle, bLeftSide);

	// Turn rad angle to dot angle
	DotAngle = Cos(OutRadAngle);

	// See if point is contained within cone
	return ((Normal(TestLocation - OutOrigin) dot OutOrientation) >= DotAngle);
}


final function bool IsWithinLeftGunFOV(vector TestLocation)
{
	return IsWithinGunFOV(SideGunOriginOffset, SideGunConeAngle, TestLocation, TRUE);
}

final function bool IsWithinRightGunFOV(vector TestLocation)
{
	return IsWithinGunFOV(SideGunOriginOffset, SideGunConeAngle, TestLocation);
}

final function bool IsWithinMainGunFOV(vector TestLocation)
{
	return IsWithinGunFOV(MainGunOriginOffset, MainGunConeAngle, TestLocation);
}

function HandleStoppingPower(class<GearDamageType> GearDamageType, Vector Momentum, float DmgDistance);

/** Notification called when a damage dealing limb collided with an Actor. */
event DmgDealingLimbsCollision(Actor HitActor, Vector HitLocation, Vector HitNormal)
{
	//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "HitActor:" @ HitActor);
	if( HitActor.IsA('GearDestructibleObject') || HitActor.IsA('InterpActor') )
	{
		// using GDT_BrumakCannon instead of _Smash, because destructible pipes don't recognize the latter.
		HitActor.TakeDamage(10, Controller, HitLocation, HitNormal, class'GDT_BrumakBullet',, Self);
	}
}

/** Disable Drone-like melee attacks for Brumak. */
simulated function StartMeleeAttack();

simulated function StartFire( byte FireModeNum )
{
	if( FireModeNum == class'GearWeapon'.const.MELEE_ATTACK_FIREMODE )
	{
		LocalDoSpecialMove( GSM_Brumak_OverlayBite );
	}
	else
	{
		Super.StartFire( FireModeNum );
	}
}


/**
 *  Event from c++ land that tells us to play a footstep
 **/
simulated event PlayFootStepSound( int FootDown )
{
	//local ParticleSystemComponent PSC_Dust;
	//	local GearCamMod_ScreenShake.ScreenShakeStruct Shake;
	//	local float DistanceMod;
	//	local GearPC WPC;
	local PlayerController PC;
	local Emitter WE;
	local vector SpawnLocation;
	local rotator SpawnRotator;

	if( FootDown == 1 )
	{
		Mesh.GetSocketWorldLocationAndRotation( 'DustRightFoot', SpawnLocation, SpawnRotator );
	}
	else
	{
		Mesh.GetSocketWorldLocationAndRotation( 'DustLeftFoot', SpawnLocation, SpawnRotator );
	}

	WE = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_FootstepDust, SpawnLocation, SpawnRotator );
	WE.SetDrawScale( DrawScale );

	//	`log("FootSound Volume: " @ FootSound.VolumeMultiplier);
	PlaySound(FootSound);

	// Tell AI that brumak has stepped
	if( Role == ROLE_Authority || IsLocallyControlled() )
	{
		MakeNoise( 1.0, 'NOISETYPE_FootStep_Brumak' );
	}

	if( ShouldPlayFootStepWaveform() )
	{
		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			if( PC.Pawn != None && PC.Pawn != self )
			{
				PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeMediumShort);
			}
		}
	}

	WE.ParticleSystemComponent.ActivateSystem();
}

simulated function bool ShouldPlayFootStepWaveform()
{
	return TRUE;
}

function PlayExposeDriver();
simulated function PlayRoar();

function UpdateRoarCount()
{
	RoarCount++;
	if( RoarCount == 0 )
	{
		RoarCount = 1;
	}

	PlayRoar();
}


function PlayDyingSound()
{
	PlaySound( DyingSound );
}

simulated function SetCinematicMode( bool bInCinematicMode )
{
	Super.SetCinematicMode( bInCinematicMode );
	Driver.SetHidden( bHidden );
}

simulated event TornOff()
{
	local GearPC PC;

	//@HACK: Big hack for Assault to make the already dead, torn off brumak go away for the cinematic
	// in the future should probably make Kismet not work on bTearOff stuff so LDs don't do things like this
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		if (PC.bInMatinee)
		{
			Destroy();
			return;
		}
	}

	Super.TornOff();
}

simulated function OnToggleHidden(SeqAct_ToggleHidden Action)
{
	local GearPC PC;

	Super.OnToggleHidden(Action);

	//@HACK: Big hack for Assault to make the already dead, torn off brumak go away for the cinematic
	// in the future should probably make Kismet not work on bTearOff stuff so LDs don't do things like this
	if (bHidden && bTearOff)
	{
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			if (LocalPlayer(PC.Player) == None)
			{
				PC.ClientDestroyDeadBrumaks();
			}
		}
	}
}

defaultproperties
{
	bCanDo360AimingInCover=FALSE
	bRespondToExplosions=FALSE
	bCanDBNO=FALSE
	bCanBeBaseForPawns=TRUE
	bNoEncroachCheck=TRUE

	SpecialMoveClasses(SM_DeathAnim)					=class'GSM_DeathAnim'
	SpecialMoveClasses(SM_RoadieRun)					=None
	SpecialMoveClasses(SM_StumbleGoDown)				=class'GSM_Brumak_StumbleBack'
	SpecialMoveClasses(GSM_Brumak_Roar)					=class'GSM_Brumak_Roar'
	SpecialMoveClasses(GSM_Brumak_RtGunHit)				=class'GSM_Brumak_RtGunHit'
	SpecialMoveClasses(GSM_Brumak_LtGunHit)				=class'GSM_Brumak_LtGunHit'
	SpecialMoveClasses(GSM_Brumak_CannonHit)			=class'GSM_Brumak_CannonHit'
	SpecialMoveClasses(GSM_Brumak_CannonFire)			=class'GSM_Brumak_CannonFire'
	SpecialMoveClasses(GSM_Brumak_MeleeAttack)			=class'GSM_Brumak_MeleeAttack'
	SpecialMoveClasses(GSM_Brumak_OverlayLftArmSwing)	=class'GSM_Brumak_OverlayLftArmSwing'
	SpecialMoveClasses(GSM_Brumak_OverlayRtArmSwing)	=class'GSM_Brumak_OverlayRtArmSwing'
	SpecialMoveClasses(GSM_Brumak_OverlayBite)			=class'GSM_Brumak_OverlayBite'
	SpecialMoveClasses(GSM_Brumak_Scream)				=class'GSM_Brumak_Scream'
	SpecialMoveClasses(GSM_Brumak_LtGunPain)			=class'GSM_Brumak_LtGunPain'
	SpecialMoveClasses(GSM_Brumak_RtGunPain)			=class'GSM_Brumak_RtGunPain'
	SpecialMoveClasses(GSM_Brumak_ExposeDriver)			=class'GSM_Brumak_ExposeDriver'


	SideGunConeAngle=85.f
	SideGunOriginOffset=(Z=200,Y=250)
	MainGunConeAngle=100.f
	MainGunOriginOffset=(X=100,Z=600)

	DmgDealingLimbs(0)=(BoneName="LeftHand",BoxSize=75.f)
	DmgDealingLimbs(1)=(BoneName="RightHand",BoxSize=75.f)
	DmgDealingLimbs(2)=(BoneName="DustLeftFoot",BoxSize=75.f)
	DmgDealingLimbs(3)=(BoneName="DustRightFoot",BoxSize=75.f)

//	RightFootBoneName="DustRightFoot"
//	LeftFootBoneName="DustLeftFoot"
	RightFootBoneName="RightBall"
	LeftFootBoneName="LeftBall"
	PelvisBoneName=None

	bCanFireRockets=TRUE
	bCanBeRunOver=FALSE
	bPushedByEncroachers=FALSE
	bEnableFloorRotConform=TRUE

	bEnableCentaurRepulsion=TRUE
	CentaurRepulsionRadius=512.0
	CentaurRepulsionStrength=4000.0

	DeathAnimHighFwd(0)=death_backward
	DeathAnimHighBwd(0)=death_backward
	DeathAnimStdFwd(0)=death_backward
	DeathAnimStdBwd(0)=death_backward
	DeathAnimStdLt(0)=death_crumple_left
	DeathAnimStdRt(0)=death_crumple_left
}
