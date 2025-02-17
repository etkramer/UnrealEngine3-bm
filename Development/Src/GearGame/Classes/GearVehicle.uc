/**
 * Gear note:  This is basically copied directly from UT, circa 12/13/07
 *
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearVehicle extends GearVehicleBase
	abstract
	native(Vehicle)
	nativereplication
	//notplaceable
	dependson(GearPC)
	dependson(GearPawn)
	config(Pawn);

/** If true, all passengers (inc. the driver) will be ejected if the vehicle flips over */
var	bool bEjectPassengersWhenFlipped;

/** true if vehicle must be upright to be entered */
var				bool		bMustBeUpright;

/** Use stick deflection to determine throttle magnitude (used for console controllers) */
var	bool bStickDeflectionThrottle;

/** When using bStickDeflectionThrottle, how far along Y do you pull stick before you actually reverse. */
var float DeflectionReverseThresh;

/** Whether or not the vehicle can auto center its view pitch.  Some vehicles (e.g. darkwalker) do not want this **/
var bool bShouldAutoCenterViewPitch;

/** The variable can be used to restrict this vehicle from being reset. */
var bool bNeverReset;

/** If true, any dead bodies that get ejected when the vehicle is flipped/destroy will be destroyed immediately */
var	bool bEjectKilledBodies;

/** Used to designate this vehicle as having light armor */
var	bool bLightArmor;

/** whether or not bots should leave this vehicle if they encounter enemies */
var bool bShouldLeaveForCombat;

/** whether or not to draw this vehicle's health on the HUD in addition to the driver's health */
var bool bDrawHealthOnHUD;

/** whether or not driver pawn should always cast shadow */
var bool bDriverCastsShadow;

/** Used natively to determine if the vehicle has been upside down */
var float LastCheckUpsideDownTime;

/** Used natively to give a little pause before kicking everyone out */
var	float FlippedCount;

/** The pawn's light environment */
var() const editconst DynamicLightEnvironmentComponent LightEnvironment;

/** Track different milestones (in terms of time) for this vehicle */
var float VehicleLostTime, PlayerStartTime;

/** How long vehicle takes to respawn */
var		float           RespawnTime;

/** How long to wait before spawning this vehicle when its factory becomes active */
var		float			InitialSpawnDelay;

/** If > 0, Link Gun secondary heals an amount equal to its damage times this */
var	float LinkHealMult;

/** hint for AI */
var float MaxDesireability;

/** if AI controlled and bot needs to trigger an objective not triggerable by vehicles, it will try to get out this far away */
var float ObjectiveGetOutDist;

/** Set internally by physics - if the contact force is pressing along vehicle forward direction. */
var const bool	bFrontalCollision;

/** If bFrontalCollision is true, this indicates the collision is with a fixed object (ie not PHYS_RigidBody) */
var const bool	bFrontalCollisionWithFixed;

/** Used to scale Z gravity applied to this vehicle. */
var() config float VehicleGravityZScale;

/*********************************************************************************************
Look Steering
********************************************************************************************* */

/** Cached value indicating what style of control we are currently using. */
var	transient bool bUsingLookSteer;

/** When using 'steer to left stick dir', this is a straight ahead dead zone which makes steering directly forwards easier. */
var(LookSteer) float LeftStickDirDeadZone;

/** When bLookToSteer is enabled, relates angle between 'looking' and 'facing' and steering angle. */
var(LookSteer) float LookSteerSensitivity;

var(LookSteer) float LookSteerDamping;

/** When error is more than this, turn on the handbrake. */
var(LookSteer) float LookSteerDeadZone;

/** Increases sensitivity of steering around middle region of controller. */
var(LookSteer) float ConsoleSteerScale;

/** Increases sensitivity of throttle around the middle region. */
var(LookSteer) float ConsoleThrottleScale;

/** [1,-1] Look angle relative to the front of the vehicle where 'forward' and 'reverse' throttle switch */
var() float ReverseIsForwardThreshold;

/** Whether the driver is allowed to exit the vehicle */
var		bool				bAllowedExit;

/** Whether to make sure exit position is near solid ground */
var		bool				bFindGroundExit;

/** Whether to do trace from actor location to camera focus location */
var		bool				bDoActorLocationToCamStartTrace;

/** If TRUE, and not bUsingLookSteer, camera inherits vehicle's yaw. */
var		bool				bVehicleSpaceCamera;
/** If TRUE, and not using looksteer, camera for passenger moves with vehicle */
var		bool				bPassengerVehicleSpaceCamera;

var		bool				bVehicleSpaceViewLimits;

/** If bVehicleSpaceCamera is TRUE, should we only take Yaw from the vehicle, or all rotation. */
var		bool				bOnlyInheritVehicleSpaceYaw;

/** How quickly camera moves to desired rotation */
var()	float				VehicleSpaceCamBlendTime;

/** If non-0, clamp camera view direction relative to vehicle to +/- this */
var()	config int			MaxVehicleSpaceCamYaw;

/** How quickly to center the camera to straight ahead if ShouldCenterCamSpaceCamera returns TRUE */
var()	config float		VehicleSpaceCamCenterSpeed;

/** Keep track of last view rot, so we can maintain it when vehicle dies. */
var		rotator				LastClampedViewRot;

/** Set to TRUE by view code if actively trying to move past view limit */
var		bool				bPushingAgainstViewLimit;

/** Saves if our current look direction would cause us to reverse */
var		bool				bLookSteerFacingReverse;

/** If this vehicle should show its weapon on the HUD. */
var		bool				bShowWeaponOnHUD;

/*********************************************************************************************
Vehicular Manslaughter / Hijacking
********************************************************************************************* */

/** The Damage type to use when something get's run over */
var class<DamageType> RanOverDamageType;

/** speed must be greater than this for running into someone to do damage */
var float MinRunOverSpeed;

/** Sound to play when someone is run over */
var SoundCue RanOverSound;

/** last time checked for pawns in front of vehicle and warned them of their impending doom. */
var	float LastRunOverWarningTime;

/** last time checked for pawns in front of vehicle and warned them of their impending doom. */
var	float MinRunOverWarningAim;

/** The human readable name of this vehicle */
var localized string VehicleNameString;

/*********************************************************************************************
Turret controllers / Aim Variables
********************************************************************************************* */
/*
Vehicles need to handle the replication of all variables required for the firing of a weapon.
Each vehicle needs to have a set of variables that begin with a common prefix and will be used
to line up the needed replicated data with that weapon.  The first weapon of the vehicle (ie: that
which is associated with the driver/seat 0 has no prefix.

<prefix>WeaponRotation 	- This defines the physical desired rotation of the weapon in the world.
<prefix>FlashLocation	- This defines the hit location when an instant-hit weapon is fired
<prefix>FlashCount		- This value is incremented after each shot
<prefix>FiringMode		- This is assigned the firemode the weapon is currently functioning in

Additionally, each seat can have any number of SkelControl_TurretConstrained controls associated with
it.  When a <prefix>WeaponRotation value is set or replicated, those controls will automatically be
updated.

FlashLocation, FlashCount and FiringMode (associated with seat 0) are predefined in PAWN.UC.  WeaponRotation
is defined below.  All "turret" variables must be defined "repnotify".  FlashLocation, FlashCount and FiringMode
variables should only be replicated to non-owning clients.
*/

/** rotation for the vehicle's main weapon */
var repnotify rotator WeaponRotation;

var repnotify bool bDisableShadows;

/** info on locations for weapon bonus effects (UDamage, etc) */
struct native WeaponEffectInfo
{
	/** socket to base on */
	var name SocketName;
	/** offset from the socket to place the effect */
	var vector Offset;
	/** Scaling for the effect */
	var vector Scale3D;
	/** reference to the component */
	var StaticMeshComponent Effect;

	structdefaultproperties
	{
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
	}
};

/**	The VehicleSeat struct defines each available seat in the vehicle. */
struct native VehicleSeat
{
	// ---[ Connections] ------------------------

	/** Who is sitting in this seat. */
	var() editinline  Pawn StoragePawn;

	/** Reference to the WeaponPawn if any */
	var() editinline  Vehicle SeatPawn;

	// ---[ Weapon ] ------------------------

	/** class of weapon for this seat */
	var() editconst class<GearVehicleWeapon> GunClass;

	/** Reference to the gun */
	var() editinline  GearVehicleWeapon Gun;

	/** Name of the socket to use for effects/spawning */
	var() array<name> GunSocket;

	/** Where to pivot the weapon */
	var() array<name> GunPivotPoints;

	var	int BarrelIndex;

	/** This is the prefix for the various weapon vars (WeaponRotation, FlashCount, etc)  */
	var() string TurretVarPrefix;

	/** list of locations for weapon bonus effects (UDamage, etc) and the component references if those effects are active */
	var array<WeaponEffectInfo> WeaponEffects;

	/** Cached names for this turret */

	var name	WeaponRotationName;
	var name	FlashLocationName;
	var name	FlashCountName;
	var name	FiringModeName;

	/** Cache pointers to the actual UProperty that is needed */

	var const pointer	WeaponRotationProperty;
	var const pointer	FlashLocationProperty;
	var const pointer	FlashCountProperty;
	var const pointer	FiringModeProperty;

	/** Holds a duplicate of the WeaponRotation value.  It's used to determine if a turret is turning */

	var rotator LastWeaponRotation;

	/** This holds all associated TurretInfos for this seat */
	var() array<name> TurretControls;

	/** Hold the actual controllers */
	var() editinline array<GearSkelCtrl_TurretConstrained> TurretControllers;

	/** Cached in ApplyWeaponRotation, this is the vector in the world where the player is currently aiming */
	var vector AimPoint;

	/** Cached in ApplyWeaponRotation, this is the actor the seat is currently aiming at (can be none) */
	var actor AimTarget;

	/** Z distance between weapon pivot and actual firing location - used to correct aiming rotation. */
	var float PivotFireOffsetZ;

	/** Disable adjustment to turret pitch based on PivotFireOffsetZ. */
	var bool bDisableOffsetZAdjust;

	// ---[ Camera ] ----------------------------------

	/** Name of the Bone/Socket to base the camera on */
	var() name CameraTag;

	/** Optional offset to add to the cameratag location, to determine base camera */
	var() vector CameraBaseOffset;

	/** how far camera is pulled back */
	var() float CameraOffset;

	/** View point offset for high player view pitch */
	var(Camera) vector CameraViewOffsetHigh;
	/** View point offset for medium (horizon) player view pitch */
	var(Camera) vector CameraViewOffsetMid;
	/** View point offset for low player view pitch */
	var(Camera) vector CameraViewOffsetLow;

	/** offset in vehicle-local space from vehicle origin that the camer worstcase position exists. replace with socket?? */
	var() vector WorstCameraLocOffset;

	/** The Eye Height for Weapon Pawns */
	var() float CameraEyeHeight;

	// ---[ View Limits ] ----------------------------------
	// - NOTE!! If ViewPitchMin/Max are set to 0.0f, the values associated with the host vehicle will be used

	/** Used for setting the ViewPitchMin on the Weapon pawn */
	var() float ViewPitchMin;

	/** Used for setting the ViewPitchMax on the Weapon pawn */
	var() float ViewPitchMax;

	// ---[  Pawn Visibility ] ----------------------------------

	/** Is this a visible Seat */
	var() bool bSeatVisible;

	/** Name of the Bone to use as an anchor for the pawn */
	var() name SeatBone;

	/** Offset from the origin to place the based pawn */
	var() vector SeatOffset;

	/** Any additional rotation needed when placing the based pawn */
	var() rotator SeatRotation;

	// ---[ Misc ] ----------------------------------

	/** damage to the driver is multiplied by this value */
	var() float DriverDamageMult;

	// ---[ Sounds ] ----------------------------------

	/** The sound to play when this seat is in motion (ie: turning) */
	var AudioComponent SeatMotionAudio;
};

/** information for each seat a player may occupy
* @note: this array is on clients as well, but SeatPawn and Gun will only be valid for the client in that seat
*/
var(Seats)	array<VehicleSeat> 	Seats;

/** This replicated property holds a mask of which seats are occupied.  */
var int SeatMask;

/*********************************************************************************************
Misc
********************************************************************************************* */

/** The Damage Type of the explosion when the vehicle is upside down */
var class<DamageType> ExplosionDamageType;

/** Is this vehicle dead */
var repnotify bool bDeadVehicle;

/** Natively used in determining when a bot should just out of the vehicle */
var float LastJumpOutCheck;

/** Templates used for explosions */
var ParticleSystem ExplosionTemplate;
/** Secondary explosions from vehicles.  (usually just dust when they are impacting something) **/
var ParticleSystem SecondaryExplosion;

/** socket to attach big explosion to (if 'None' it won't be attached at all) */
var name BigExplosionSocket;

/** Max dist for wheel sounds and particle effects */
var float MaxWheelEffectDistSq;

/** Water effect type name */
var name WaterEffectType;

//struct native BurnOutDatum
//{
	//var MaterialInstanceTimeVarying MITV;

	/**
	* We need to store the value of the MIC set param on a per material basis as we have some MICs where are
	* vastly different than the others.
	**/
	//var float CurrValue;
//};

/** The material instances and their data used when showing the burning hulk */
//var array<BurnOutDatum> BurnOutMaterialInstances;

/** How long does it take to burn out */
//var float BurnOutTime;

/** How long should the vehicle should last after being destroyed */
var float DeadVehicleLifeSpan;

/** Damage/Radius/Momentum parameters for dying explosions */
var float ExplosionDamage, ExplosionRadius, ExplosionMomentum;
/** If vehicle dies in the air, this is how much spin is given to it. */
var float ExplosionInAirAngVel;

/** Whether or not there is a turret explosion sequence on death */
var bool bHasTurretExplosion;
/** Name of the turret skel control to scale the turret to nothing*/
var name TurretScaleControlName;
/** Name of the socket location to spawn the explosion effect for the turret blowing up*/
var name TurretSocketName;
/** Explosion of the turret*/
//var array<DistanceBasedParticleTemplate> DistanceTurretExplosionTemplates;

/** The offset from the TurretSocketName to spawn the turret*/
var vector TurretOffset;
/** Reference to destroyed turret for death effects */
//var UTVehicleDeathPiece DestroyedTurret;
/** Class to spawn when turret destroyed */
var StaticMesh DestroyedTurretTemplate;
/** Force applied to the turret explosion */
var float TurretExplosiveForce;

/** sound for dying explosion */
var SoundCue ExplosionSound;
/** Sound for being hit by impact hammer*/
var SoundCue ImpactHitSound;
/** stores the time of the last death impact to stop spamming them from occurring*/
var float LastDeathImpactTime;

/** The sounds this vehicle will play based on impact force */
var SoundCue LargeChunkImpactSound, MediumChunkImpactSound, SmallChunkImpactSound;

/** How long until it's done burning */
var float RemainingBurn;

/** This vehicle is dead and burning */
var bool bIsBurning;

/** names of material parameters for burnout material effect */
//var name BurnTimeParameterName;

/** If true, this vehicle is scraping against something */
var bool bIsScraping;

/** Sound to play when the vehicle is scraping against something */
var(Sounds) AudioComponent ScrapeSound;

/** Sound to play from the tires */
var(Sounds) editconst const AudioComponent TireAudioComp;
//var(Sounds) array<MaterialSoundEffect> TireSoundList;
var name CurrentTireMaterial;

/** Overall scaling for wheels longitudinal grip */
var()	float	WheelLongGripScale;

/** Overall scaling for wheels lateral grip */
var()	float	WheelLatGripScale;

/** How much additional spin to apply to graphics of wheels */
var()	float	WheelExtraGraphicalSpin;

/** When extra spin is greater than actual spin, reduce grip */
var()	float	PeelOutFrictionScale;

/** This is used to determine a safe zone around the spawn point of the vehicle.  It won't spawn until this zone is clear of pawns */
var float	SpawnRadius;

/** Anim to play when a visible driver is driving */
var	name	DrivingAnim;

/** PRI of player in passenger turret */
var GearPRI PassengerPRI;

/** Burn out material (per team) */
//var MaterialInterface BurnOutMaterial[2];

/** multiplier to damage from colliding with other rigid bodies */
var float CollisionDamageMult;

/** last time we took collision damage, so we don't take collision damage multiple times in the same tick */
var float LastCollisionDamageTime;

/** if true, collision damage is reduced when the vehicle collided with something below it */
var bool bReducedFallingCollisionDamage;

var config array<PerDamageTypeMod> PerDamageTypeModifiers;

/** TRUE if this warpawn should block the camera, FALSE otherwise */
var() bool							bBlockCamera;
/** Whether the warpawn is not being rendered because the camera is inside of it */
var bool							bIsHiddenByCamera;

/*********************************************************************************************
Penetration destruction
********************************************************************************************* */

/** If a physics penetration of greater than this is detected, destroy vehicle. */
var() float DestroyOnPenetrationThreshold;

/** If we are over DestroyOnPenetrationThreshold for more than this (seconds), call RBPenetrationDestroy. */
var() float DestroyOnPenetrationDuration;

/** TRUE indicates vehicle is currently in penetration greater than DestroyOnPenetrationThreshold. */
var bool bIsInDestroyablePenetration;

/** How long the vehicle has been in penetration greater than DestroyOnPenetrationThreshold. */
var float TimeInDestroyablePenetration;

/*********************************************************************************************
Camera
********************************************************************************************* */

var(Seats)	float	SeatCameraScale;

/** If true, this will allow the camera to rotate under the vehicle which may obscure the view */
var(Camera)	bool bRotateCameraUnderVehicle;

/** If true, don't Z smooth lagged camera (for bumpier looking ride */
var bool bNoZSmoothing;

/** If true, make sure camera z stays above vehicle when looking up (to avoid clipping when flying vehicle going up) */
var bool bLimitCameraZLookingUp;

/** If true, don't change Z while jumping, for more dramatic looking jumps */
var bool bNoFollowJumpZ;

/** Used only if bNoFollowJumpZ=true.  True when Camera Z is being fixed. */
var bool bFixedCamZ;

/** Used only if bNoFollowJumpZ=true.  saves the Camera Z position from the previous tick. */
var float OldCamPosZ;

/** Smoothing scale for lagged camera - higher values = shorter smoothing time. */
var float CameraSmoothingFactor;

/** FOV to use when driving this vehicle */
var() config float DefaultFOV;

/** Saved Camera positions (for lagging camera) */
struct native TimePosition
{
	var vector Position;
	var float Time;
};
var array<TimePosition> OldPositions;

/** Amount of camera lag for this vehicle (in seconds */
var() float CameraLag;

/** Smoothed Camera Offset */
var vector CameraOffset;

/** How far forward to bring camera if looking over nose of vehicle */
var(Camera) float LookForwardDist;

/** hide vehicle if camera is too close */
var	float	MinCameraDistSq;

var bool bCameraNeverHidesVehicle;

/** If TRUE, always apply aim friction - not just when targetting */
var	bool bAlwaysViewFriction;

/** Stop death camera using OldCameraPosition if true */
var bool bStopDeathCamera;

/** OldCameraPosition saved when dead for use if fall below killz */
var vector OldCameraPosition;

/** for player controlled !bSeparateTurretFocus vehicles on console, this is updated to indicate whether the controller is currently turning
* (server and owning client only)
*/
var bool bIsConsoleTurning;

/** Whether to accept jump from UTWeaponPawns */
var bool bAcceptTurretJump;

/*********************************************************************************************/

var bool bShowDamageDebug;

/** Action displayed on the HUD */
var ActionInfo InteractAction;

/** If true, don't damp z component of vehicle velocity while it is in the air */
var bool bNoZDampingInAir;

/** If true, don't damp z component of vehicle velocity even when on the ground */
var bool bNoZDamping;

/** material specific wheel effects, applied to all attached UTVehicleWheels with bUseMaterialSpecificEffects set to true */
//var array<MaterialParticleEffect> WheelParticleEffects;

/** additional downward threshold for AI reach tests (for high hover vehicles) */
var float ExtraReachDownThreshold;

/** if part of a Horde wave, the index of this enemy in the Horde enemy list */
var int HordeEnemyIndex;

/** If AI_VehicleAimOffset is set on a GearAnim_AimOffset, this is used as input for that node  */
var	vector2D	VehicleAimOffset;

/** GUID for checkpoint saved GearPawns so that references to them can be saved even though they will be recreated */
var Guid MyGuid;

/** Ref to AI controller */
var GearAI MyGearAI;

/** This pawn should never be considered a valid enemy by AI */
var bool bNeverAValidEnemy;

/** inside this cylinder, camera's using this pawn as a target will turn off owner rendering for this pawn */
var() protected cylinder CameraNoRenderCylinder_High;
var() protected cylinder CameraNoRenderCylinder_Low;
var() protected cylinder CameraNoRenderCylinder_High_ViewTarget;
var() protected cylinder CameraNoRenderCylinder_Low_ViewTarget;
var() protected cylinder CameraNoRenderCylinder_FlickerBuffer;

/************************************************************************/
// Muzzle light support

/** Struct containing info for muzzle light */
struct native FGearVehicleMuzzleLightInfo
{
	/** Template that current MuzzleLight component is based on */
	var	PointLightComponent TemplateMuzzleLight;
	/** Active light component for muzzle */
	var	PointLightComponent	MuzzleLight;
	/** Initial brightness of muzzle light - will be ramped down from here */
	var	float				LightInitialBrightness;
	/** How long light takes to fade */
	var	float				LightFadeTime;
	/** How much longer before light brightness reaches zero */
	var	float				LightTimeRemaining;
};

/** Info for muzzle light on this vehicle */
var	FGearVehicleMuzzleLightInfo		MuzzleLightInfo;

replication
{
	if (bNetDirty && !bNetOwner)
		WeaponRotation;
	if (bNetDirty)
		bDeadVehicle, SeatMask, PassengerPRI;
	if(ROLE == ROLE_Authority)
		bDisableShadows;
}

cpptext
{
	virtual FGuid* GetGuid();
	virtual FVector GetDampingForce(const FVector& InForce);
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual UBOOL JumpOutCheck(AActor *GoalActor, FLOAT Distance, FLOAT ZDiff);
	virtual FLOAT GetMaxRiseForce();
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void ApplyWeaponRotation(INT SeatIndex, FRotator NewRotation);
	virtual void PreNetReceive();
	virtual void PostNetReceive();
	virtual UBOOL ReachThresholdTest(const FVector& TestPosition, const FVector& Dest, AActor* GoalActor, FLOAT UpThresholdAdjust, FLOAT DownThresholdAdjust, FLOAT ThresholdAdjust);
}

/*********************************************************************************************
Native Accessors for the WeaponRotation, FlashLocation, FlashCount and FiringMode
********************************************************************************************* */
native simulated function rotator 	SeatWeaponRotation	(int SeatIndex, optional rotator NewRot,	optional bool bReadValue);
native simulated function vector  	SeatFlashLocation	(int SeatIndex, optional vector  NewLoc,	optional bool bReadValue);
native simulated function byte		SeatFlashCount		(int SeatIndex, optional byte NewCount, 	optional bool bReadValue);
native simulated function byte		SeatFiringMode		(int SeatIndex, optional byte NewFireMode,	optional bool bReadValue);

native simulated function ForceWeaponRotation(int SeatIndex, Rotator NewRotation);
native simulated function vector GetSeatPivotPoint(int SeatIndex);
native simulated function int GetBarrelIndex(int SeatIndex);

native function bool IsValidEnemyTargetFor(const PlayerReplicationInfo PRI, bool bNoPRIisEnemy) const;

/** @return whether we are currently replicating to the Controller of the given seat
* this would be equivalent to checking bNetOwner on that seat,
* but bNetOwner is only valid during that Actor's replication, not during the base vehicle's
* not complex logic, but since it's for use in vehicle replication statements, the faster the better
*/
native(999) noexport final function bool IsSeatControllerReplicationViewer(int SeatIndex);

/**
* Returns camera "no render" cylinder.
* @param bViewTarget TRUE to return the cylinder for use when this pawn is the camera target, false to return the non-viewtarget dimensions
*/
final simulated native function GetCameraNoRenderCylinder(out float Radius, out float Height, bool bViewTarget, bool bHiddenLocally);

/**
* Initialization
*/
simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if (Role==ROLE_Authority)
	{
		// Setup the Seats array
		InitializeSeats();
	}
	else if (Seats.length > 0)
	{
		// Insure our reference to self is always setup
		Seats[0].SeatPawn = self;
	}

	CacheAnimNodes();

	PreCacheSeatNames();

	InitializeTurrets();		// Setup the turrets
}

simulated function CacheAnimNodes();

/** reattaches the mesh component, because settings were updated */
simulated function ReattachMesh()
{
	ReattachComponent(Mesh);
}

simulated function UpdateLookSteerStatus()
{
	bUsingLookSteer = FALSE;
}

/**
* Console specific input modification
*/
simulated function SetInputs(float InForward, float InStrafe, float InUp)
{
	local bool bReverseThrottle;
	local PlayerController VehiclePC;
	local rotator SteerRot, VehicleRot;
	local vector SteerDir, VehicleDir, AngVel;
	local float VehicleHeading, SteerHeading, DeltaTargetHeading, Deflection;

	VehiclePC = PlayerController(Controller);
	if(VehiclePC != None && (VehiclePC.IsMoveInputIgnored() || VehiclePC.IsLookInputIgnored()))
	{
		Throttle = 0.0;
		Steering = 0.0;
		Rise = 0.0;
		return;
	}

	Throttle = InForward;
	Steering = InStrafe;
	Rise = InUp;

	bLookSteerFacingReverse = FALSE;

	//ConsolePC = UTConsolePlayerController(Controller);
	//if (ConsolePC != None)
	if(TRUE)
	{
		UpdateLookSteerStatus();

		// tank, wheeled / heavy vehicles will use this

		VehicleRot.Yaw = Rotation.Yaw;
		VehicleDir = vector(VehicleRot);

		SteerRot.Yaw = DriverViewYaw;
		SteerDir = vector(SteerRot);

		if (VehicleDir dot SteerDir < ReverseIsForwardThreshold)
		{
			bLookSteerFacingReverse = TRUE;
		}

		// If we desire 'look steering' on this vehicle, do it here.
		if (bUsingLookSteer && IsHumanControlled())
		{
			//If we are looking 'backwards', reverse controls
			if (bLookSteerFacingReverse)
			{
				Throttle = -InForward;
				VehicleDir = -VehicleDir;
			}

			if(bStickDeflectionThrottle)
			{
				// If there is a deflection, look at the angle that its point in.
				Deflection = Sqrt(Throttle*Throttle + Steering*Steering);

				// The region we consider 'reverse' is anything below DeflectionReverseThresh, or anything withing the triangle below the center position.
				bReverseThrottle = ((Throttle < DeflectionReverseThresh) || (Throttle < 0.0 && Abs(Steering) < -Throttle));
				Throttle = Deflection;

				if (bReverseThrottle)
				{
					Throttle *= -1;
				}
			}
			else
			{
				Throttle = FClamp(Throttle * ConsoleThrottleScale, -1.0, 1.0);
			}

			VehicleHeading = GetHeadingAngle(VehicleDir);
			SteerHeading = GetHeadingAngle(SteerDir);
			DeltaTargetHeading = FindDeltaAngle(SteerHeading, VehicleHeading);

			if (DeltaTargetHeading > LookSteerDeadZone)
			{
				Steering = FMin((DeltaTargetHeading - LookSteerDeadZone) * LookSteerSensitivity, 1.0);
			}
			else if (DeltaTargetHeading < -LookSteerDeadZone)
			{
				Steering = FMax((DeltaTargetHeading + LookSteerDeadZone) * LookSteerSensitivity, -1.0);
			}
			else
			{
				Steering = 0.0;
			}

			AngVel = Mesh.BodyInstance.GetUnrealWorldAngularVelocity();

			Steering = FClamp(Steering + (AngVel.Z * LookSteerDamping), -1.0, 1.0);

			// Reverse steering when reversing
			if (Throttle < 0.0 && ForwardVel < 0.0)
			{
				Steering = -1.0 * Steering;
			}
		}
		// flying hovering vehicles will use this
		else
		{
			VehicleRot.Yaw = Rotation.Yaw;
			VehicleDir = vector(VehicleRot);

			SteerRot.Yaw = DriverViewYaw;
			SteerDir = vector(SteerRot);

			//If we are looking 'backwards', reverse controls
			if (VehicleDir dot SteerDir < ReverseIsForwardThreshold)
			{
				Throttle = -InForward;
			}

			//`log( " flying hovering vehicle" );
			if (bStickDeflectionThrottle)
			{
				// The region we consider 'reverse' is anything below DeflectionReverseThresh, or anything withing the triangle below the center position.
				bReverseThrottle = ((Throttle < DeflectionReverseThresh) || (Throttle < 0.0 && Abs(Steering) < -Throttle));

				Deflection = Sqrt(Throttle*Throttle + Steering*Steering);
				Throttle = Deflection;

				if (bReverseThrottle)
				{
					Throttle *= -1;
				}
			}
			else
			{
				Throttle = FClamp(Throttle * ConsoleThrottleScale, -1.0, 1.0);
			}

			Steering = FClamp(Steering * ConsoleSteerScale, -1.0, 1.0);
		}

		//`log( "Throttle: " $ Throttle $ " Steering: " $ Steering );
	}
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	super.FellOutOfWorld(DmgType);
	bStopDeathCamera = true;
}

native function float GetGravityZ();


event SelfDestruct(Actor ImpactedActor);

/**
* JumpOutCheck()
* Check if bot wants to jump out of vehicle, which is currently descending towards its destination
*/
event JumpOutCheck();

/** @return whether the given vehicle pawn is in this vehicle's driver seat
* (usually seat 0, but some vehicles may give driver control of a different seat when deployed)
*/
function bool IsDriverSeat(Vehicle TestSeatPawn)
{
	return (Seats[0].SeatPawn == TestSeatPawn);
}

/**
* Returns rotation used for determining valid exit positions
*/
function Rotator ExitRotation()
{
	return Rotation;
}

/**
* FindAutoExit() Tries to find exit position on either side of vehicle, in back, or in front
* returns true if driver successfully exited.
*
* @param	ExitingDriver	The Pawn that is leaving the vehicle
*/
function bool FindAutoExit(Pawn ExitingDriver)
{
	local vector X, Y, Z;
	local float PlaceDist;

	GetAxes(ExitRotation(), X,Y,Z);
	Y *= -1;

	if ( ExitRadius == 0 )
	{
		ExitRadius = CylinderComponent.CollisionRadius + 2*ExitingDriver.GetCollisionRadius();
	}
	PlaceDist = ExitRadius + ExitingDriver.GetCollisionRadius();

	if ( Controller != None )
	{
		// use the controller's rotation as a hint
		if ( (Y dot vector(Controller.Rotation)) < 0 )
		{
			Y *= -1;
		}
	}

	if ( VSize(Velocity) > MinCrushSpeed )
	{
		//avoid running driver over by placing in direction away from velocity
		if ( (Velocity Dot X) < 0 )
			X *= -1;
		// check if going sideways fast enough
		if ( (Velocity Dot Y) > MinCrushSpeed )
			Y *= -1;
	}

	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) + (PlaceDist * Y), bFindGroundExit) )
		return true;
	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) - (PlaceDist * Y), bFindGroundExit) )
		return true;

	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) - (PlaceDist * X), false) )
		return true;
	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) + (PlaceDist * X), false) )
		return true;
	if ( !bFindGroundExit )
		return false;
	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) + (PlaceDist * Y), false) )
		return true;
	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) - (PlaceDist * Y), false) )
		return true;
	if ( TryExitPos(ExitingDriver, GetTargetLocation() + (ExitOffset >> Rotation) + (PlaceDist * Z), false) )
		return true;

	return false;
}

/**
* RanInto() called for encroaching actors which successfully moved the other actor out of the way
*
* @param	Other 		The pawn that was hit
*/
event RanInto(Actor Other)
{
	local vector Momentum;
	local float Speed;
	local GearPawn GP;

	GP = GearPawn(Other);

	if ( GP == None || !GP.bCanBeRunOver || Other == Instigator || Other.Role != ROLE_Authority )
		return;

	Speed = VSize(Velocity);
	if (Speed > MinRunOverSpeed)
	{
		Momentum = Velocity * 0.25 * GP.Mass;
		if ( RanOverSound != None )
			PlaySound(RanOverSound);

		if ( WorldInfo.GRI.OnSameTeam(self,Other) )
		{
			Momentum += Speed * 0.25 * Pawn(Other).Mass * Normal(Velocity cross vect(0,0,1));
		}
		else
		{
			Other.TakeDamage(int(Speed * 0.075), GetCollisionDamageInstigator(), Other.Location, Momentum, RanOverDamageType);
		}
	}
}

/**
* This function is called to see if radius damage should be applied to the driver.  It is called
* from SVehicle::TakeRadiusDamage().
*
* @param	DamageAmount		The amount of damage taken
* @param	DamageRadius		The radius that the damage covered
* @param	EventInstigator		Who caused the damage
* @param	DamageType			What type of damage
* @param	Momentum			How much force should be imparted
* @param	HitLocation			Where
*/
function DriverRadiusDamage( float DamageAmount, float DamageRadius, Controller EventInstigator,
class<DamageType> DamageType, float Momentum, vector HitLocation, Actor DamageCauser )
{
	local int i;
	local Vehicle V;

	if ( bDriverIsVisible )
	{
		Super.DriverRadiusDamage(DamageAmount, DamageRadius, EventInstigator, DamageType, Momentum, HitLocation, DamageCauser);
	}

	// pass damage to seats as well but skip seats[0] since that is us and was already handled by the Super

	for (i = 1; i < Seats.length; i++)
	{
		V = Seats[i].SeatPawn;
		if( ( V != none ) && ( V.bDriverIsVisible ) )
		{
			V.DriverRadiusDamage(DamageAmount, DamageRadius, EventInstigator, DamageType, Momentum, HitLocation, DamageCauser);
		}
	}

}

/**
* Called when the vehicle is destroyed.  Clean up the seats/effects/etc
*/
simulated function Destroyed()
{
	local int i;

	for(i=1;i<Seats.Length;i++)
	{
		if ( Seats[i].SeatPawn != None )
		{
			if (Seats[i].SeatPawn.Controller != None)
			{
				`Warn(self @ "destroying seat" @ i @ "still controlled by" @ Seats[i].SeatPawn.Controller @ Seats[i].SeatPawn.Controller.GetHumanReadableName());
			}
			Seats[i].SeatPawn.Destroy();
		}
	}

	SetTexturesToBeResident( FALSE );

	super.Destroyed();
}


/** This will set the textures to be resident or not **/
simulated function SetTexturesToBeResident( bool bActive )
{
	local int i, j;
	local array<texture> Textures;
	local int NumElems;
	local int NumTextures;
	local Texture2D Tex2d;
	local MaterialInterface Material;

	// reset all of the textures to not be resident
	NumElems = Mesh.GetNumElements();
	for (i = 0; i < NumElems; i++)
	{
		Material = Mesh.GetMaterial(i);
		if (Material != None)
		{
			Textures = Material.GetMaterial().GetTextures();
			NumTextures = Textures.Length;
			for( j = 0; j < NumTextures; ++j )
			{
				//`log( "Texture setting bForceMiplevelsToBeResident: " $ bActive $ " " $ Textures[j] );
				Tex2d = Texture2D(Textures[j]);
				if( Tex2d != none )
				{
					Tex2d.bForceMiplevelsToBeResident = bActive;
				}
			}
		}
	}
}

/**
* Given the variable prefix, find the seat index that is associated with it
*
* @returns the index if found or -1 if not found
*/

simulated function int GetSeatIndexFromPrefix(string Prefix)
{
	local int i;

	for (i=0; i < Seats.Length; i++)
	{
		if (Seats[i].TurretVarPrefix ~= Prefix)
		{
			return i;
		}
	}
	return -1;
}

/** used on console builds to set the value of bIsConsoleTurning on the server */
reliable server function ServerSetConsoleTurning(bool bNewConsoleTurning)
{
	bIsConsoleTurning = bNewConsoleTurning;
}

simulated function ProcessViewRotation(float DeltaTime, out rotator out_ViewRotation, out rotator out_DeltaRot)
{
	local int i, MaxDelta;
	local float MaxDeltaDegrees;

	if (WorldInfo.bUseConsoleInput)
	{
		if (!bSeparateTurretFocus && ShouldClamp())
		{
			if (out_DeltaRot.Yaw == 0)
			{
				if (bIsConsoleTurning)
				{
					// if controller stops rotating on a vehicle whose view rotation yaw gets clamped,
					// set the controller's yaw to where we got so that there's no control lag
					out_ViewRotation.Yaw = GetClampedViewRotation().Yaw;
					bIsConsoleTurning = false;
					ServerSetConsoleTurning(false);
				}
			}

			else if (!bIsConsoleTurning)
			{
				// don't allow starting a new turn if the view would already be clamped
				// because that causes nasty jerking
				if (GetClampedViewRotation().Yaw == Controller.Rotation.Yaw)
				{
					bIsConsoleTurning = true;
					ServerSetConsoleTurning(true);
				}
				else
				{
					// @fixme:  this should be setting to max turn rate so we actually do something when outside of the cone
					out_DeltaRot.Yaw = 0;
				}
			}

			// clamp player rotation to turret rotation speed
			for (i = 0; i < Seats[0].TurretControllers.length; i++)
			{
				MaxDeltaDegrees = FMax(MaxDeltaDegrees, Seats[0].TurretControllers[i].LagDegreesPerSecond);
			}

			if (MaxDeltaDegrees > 0.0)
			{
				MaxDelta = int(MaxDeltaDegrees * 182.0444 * DeltaTime);
				out_DeltaRot.Pitch = (out_DeltaRot.Pitch >= 0) ? Min(out_DeltaRot.Pitch, MaxDelta) : Max(out_DeltaRot.Pitch, -MaxDelta);
				out_DeltaRot.Yaw = (out_DeltaRot.Yaw >= 0) ? Min(out_DeltaRot.Yaw, MaxDelta) : Max(out_DeltaRot.Yaw, -MaxDelta);
				out_DeltaRot.Roll = (out_DeltaRot.Roll >= 0) ? Min(out_DeltaRot.Roll, MaxDelta) : Max(out_DeltaRot.Roll, -MaxDelta);
			}
		}
	}
	Super.ProcessViewRotation(DeltaTime, out_ViewRotation, out_DeltaRot);
}

simulated native function rotator GetClampedViewRotation();

simulated native function bool ShouldClamp();

simulated native event rotator GetViewRotation();

/**
* this function is called when a weapon rotation value has changed.  It sets the DesiredboneRotations for each controller
* associated with the turret.
*
* Network: Remote clients.  All other cases are handled natively
* FIXME: Look at handling remote clients natively as well
*
* @param	SeatIndex		The seat at which the rotation changed
*/

simulated function WeaponRotationChanged(int SeatIndex)
{
	local int i;

	if ( SeatIndex>=0 )
	{
		for (i=0;i<Seats[SeatIndex].TurretControllers.Length;i++)
		{
			Seats[SeatIndex].TurretControllers[i].DesiredBoneRotation = SeatWeaponRotation(SeatIndex,,true);
		}
	}
}

/**
* This event is triggered when a repnotify variable is received
*
* @param	VarName		The name of the variable replicated
*/

simulated event ReplicatedEvent(name VarName)
{
	local string VarString;
	local int SeatIndex;

	if (VarName == 'bDeadVehicle')
	{
		BlowupVehicle(None);
	}
	else
	{
		// Ok, some magic occurs here.  The turrets/seat use a prefix system to determine
		// which values to adjust. Here we decode those values and call the appropriate functions

		// First check for <xxxxx>weaponrotation

		VarString = ""$VarName;
		if( Varname == nameof(bDisableShadows) )
		{
			UpdateShadowSettings(!bDisableShadows);
		}
		else if ( Right(VarString, 14) ~= "weaponrotation" )
		{
			SeatIndex = GetSeatIndexFromPrefix( Left(VarString, Len(VarString)-14) );
			if (SeatIndex >= 0)
			{
				WeaponRotationChanged(SeatIndex);
			}
		}

		// Next, check for <xxxxx>flashcount

		else if ( Right(VarString, 10) ~= "flashcount" )
		{
			SeatIndex = GetSeatIndexFromPrefix( Left(VarString, Len(VarString)-10) );
			if ( SeatIndex>=0 )
			{
				Seats[SeatIndex].BarrelIndex++;

				if ( SeatFlashCount(SeatIndex,,true) > 0 )
				{
					VehicleWeaponFired(true, vect(0,0,0), SeatIndex);
				}
				else
				{
					VehicleWeaponStoppedFiring(true,SeatIndex);
				}
			}
		}

		// finally <xxxxxx>flashlocation

		else if ( Right(VarString, 13) ~= "flashlocation" )
		{
			SeatIndex = GetSeatIndexFromPrefix( Left(VarString, Len(VarString)-13) );
			if ( SeatIndex>=0 )
			{
				Seats[SeatIndex].BarrelIndex++;

				if ( !IsZero(SeatFlashLocation(SeatIndex,,true)) )
				{
					VehicleWeaponFired(true, SeatFlashLocation(SeatIndex,,true), SeatIndex);
				}
				else
				{
					VehicleWeaponStoppedFiring(true,SeatIndex);
				}
			}
		}
		else
		{
			super.ReplicatedEvent(VarName);
		}
	}
}

/**
* AI Hint
* @returns true if there is an occupied turret
*/

function bool HasOccupiedTurret()
{
	local int i;

	for (i = 1; i < Seats.length; i++)
	{
		if( ( Seats[i].SeatPawn != none )
			&& ( Seats[i].SeatPawn.Controller != None )
			)
		{
			return true;
		}
	}

	return false;
}

/**
* This function is called when the driver's status has changed.
*/
simulated function DrivingStatusChanged()
{
	// turn parking friction on or off
	bUpdateWheelShapes = true;

	if ( bDriving )
	{
		VehiclePlayEnterSound();
	}
	else if ( Health > 0 )
	{
		VehiclePlayExitSound();
	}

	bBlocksNavigation = !bDriving;

	if (!bDriving)
	{
		StopFiringWeapon();

		SetTexturesToBeResident(false);
	}

	super.DrivingStatusChanged();
}

/**
* @Returns true if a seat is not occupied
*/
function bool SeatAvailable(int SeatIndex)
{
	return Seats[SeatIndex].SeatPawn == none || Seats[SeatIndex].SeatPawn.Controller == none;
}

/**
* @return true if there is a seat
*/
function bool AnySeatAvailable()
{
	local int i;
	for (i=0;i<Seats.Length;i++)
	{
		if( ( Seats[i].SeatPawn != none )
			&& ( Seats[i].SeatPawn.Controller==none )
			)
		{
			return true;
		}
	}
	return false;
}

/**
* @returns the Index for this Controller's current seat or -1 if there isn't one
*/
simulated function int GetSeatIndexForController(controller ControllerToMove)
{
	local int i;
	for (i=0;i<Seats.Length;i++)
	{
		if (Seats[i].SeatPawn.Controller != none && Seats[i].SeatPawn.Controller == ControllerToMove )
		{
			return i;
		}
	}
	return -1;
}

/**
* @returns the controller of a given seat.  Can be none if the seat is empty
*/
function controller GetControllerForSeatIndex(int SeatIndex)
{
	return Seats[SeatIndex].SeatPawn.Controller;
}

/**
request change to adjacent vehicle seat
*/
reliable server function ServerAdjacentSeat(int Direction, Controller C)
{
	local int CurrentSeat, NewSeat;

	CurrentSeat = GetSeatIndexForController(C);
	if (CurrentSeat != INDEX_NONE)
	{
		NewSeat = CurrentSeat;
		do
		{
			NewSeat += Direction;
			if (NewSeat < 0)
			{
				NewSeat = Seats.Length - 1;
			}
			else if (NewSeat == Seats.Length)
			{
				NewSeat = 0;
			}
			if (NewSeat == CurrentSeat)
			{
				// no available seat
				return;
			}
		} until (SeatAvailable(NewSeat) || (Seats[NewSeat].SeatPawn != None && AIController(Seats[NewSeat].SeatPawn.Controller) != None));

		// change to the seat we found
		ChangeSeat(C, NewSeat);
	}
}

/**
* Called when a client is requesting a seat change
*
* @network	Server-Side
*/
reliable server function ServerChangeSeat(int RequestedSeat)
{
	if ( RequestedSeat == -1 )
		DriverLeave(false);
	else
		ChangeSeat(Controller, RequestedSeat);
}

/**
* This function looks at 2 controllers and decides if one as priority over the other.  Right now
* it looks to see if a human is against a bot but it could be extended to use rank/etc.
*
* @returns	ture if First has priority over second
*/
function bool HasPriority(controller First, controller Second)
{
	if ( First != Second && PlayerController(First) != none && PlayerController(Second) == none)
		return true;
	else
		return false;
}

/**
* ChangeSeat, this controller to change from it's current seat to a new one if (A) the new
* set is empty or (B) the controller looking to move has Priority over the controller already
* there.
*
* If the seat is filled but the new controller has priority, the current seat holder will be
* bumped and swapped in to the seat left vacant.
*
* @param	ControllerToMove		The Controller we are trying to move
* @param	RequestedSeat			Where are we trying to move him to
*
* @returns true if successful
*/
function bool ChangeSeat(Controller ControllerToMove, int RequestedSeat)
{
	local int OldSeatIndex;
	local Pawn OldPawn, BumpPawn;
	local Controller BumpController;

	// Make sure we are looking to switch to a valid seat
	if ( (RequestedSeat >= Seats.Length) || (RequestedSeat < 0) )
	{
		return false;
	}

	// get the seat index of the pawn looking to move.
	OldSeatIndex = GetSeatIndexForController(ControllerToMove);
	if (OldSeatIndex == -1)
	{
		// Couldn't Find the controller, should never happen
		`Warn("[Vehicles] Attempted to switch" @ ControllerToMove @ "to a seat in" @ self @ " when he is not already in the vehicle");
		return false;
	}

	// If someone is in the seat, see if we can bump him
	if (!SeatAvailable(RequestedSeat))
	{
		// Get the Seat holder's controller and check it for Priority
		BumpController = GetControllerForSeatIndex(RequestedSeat);
		if (BumpController == none)
		{
			`warn("[Vehicles]" @ ControllertoMove @ "Attempted to bump a phantom Controller in seat in" @ RequestedSeat @ " (" $ Seats[RequestedSeat].SeatPawn $ ")");
			return false;
		}

		if ( !HasPriority(ControllerToMove,BumpController) )
		{
			// Nope, same or great priority on the seat holder, deny the move
			return false;
		}

		// If we are bumping someone, free their seat.
		if (BumpController != None)
		{
			BumpPawn = Seats[RequestedSeat].StoragePawn;
			Seats[RequestedSeat].SeatPawn.DriverLeave(true);

			// Handle if we bump the driver
			if (RequestedSeat == 0)
			{
				// Reset the controller's AI if needed
				if (BumpController.RouteGoal == self)
				{
					BumpController.RouteGoal = None;
				}
				if (BumpController.MoveTarget == self)
				{
					BumpController.MoveTarget = None;
				}
			}
		}
	}

	OldPawn = Seats[OldSeatIndex].StoragePawn;

	// Leave the current seat and take over the new one
	Seats[OldSeatIndex].SeatPawn.DriverLeave(true);
	if (OldSeatIndex == 0)
	{
		// Reset the controller's AI if needed
		if (ControllerToMove.RouteGoal == self)
		{
			ControllerToMove.RouteGoal = None;
		}
		if (ControllerToMove.MoveTarget == self)
		{
			ControllerToMove.MoveTarget = None;
		}
	}

	if (RequestedSeat == 0)
	{
		DriverEnter(OldPawn);
	}
	else
	{
		PassengerEnter(OldPawn, RequestedSeat);
	}


	// If we had to bump a pawn, seat them in this controller's old seat.
	if (BumpPawn != None)
	{
		if (OldSeatIndex == 0)
		{
			DriverEnter(BumpPawn);
		}
		else
		{
			PassengerEnter(BumpPawn, OldSeatIndex);
		}
	}
	return true;
}

/**
* This event is called when the pawn is torn off
*/
simulated event TornOff()
{
	`warn(self @ "Torn off");
}

/**
* See Pawn::Died()
*/
function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	local int i;
	local GearPawn APawn;

	// handle unlimited health cheat
	if (Health <= 0)
	{
		for (i=0; i<Seats.length; i++)
		{
			APawn = GearPawn(Seats[i].StoragePawn);
			if(APawn != None && APawn.bUnlimitedHealth)
			{
				return false;
			}
		}
	}

	if ( Super(Vehicle).Died(Killer, DamageType, HitLocation) )
	{
		HitDamageType = DamageType; // these are replicated to other clients
		TakeHitLocation = HitLocation;
		BlowupVehicle(Killer);

		HandleDeadVehicleDriver();

		for (i = 1; i < Seats.Length; i++)
		{
			if (Seats[i].SeatPawn != None)
			{
				// kill the WeaponPawn with the appropriate killer, etc for kill credit and death messages
				Seats[i].SeatPawn.Died(Killer, DamageType, HitLocation);
			}
		}

		return true;
	}
	return false;
}

/**
* Call this function to blow up the vehicle
*/
simulated function BlowupVehicle(Controller Killer)
{
	bCanBeBaseForPawns = false;
	GotoState('DyingVehicle');
	AddVelocity(TearOffMomentum, TakeHitLocation, HitDamageType);
	bDeadVehicle = true;
	bStayUpright = false;

	if ( StayUprightConstraintInstance != None )
	{
		StayUprightConstraintInstance.TermConstraint();
	}
}


simulated function PlayerReplicationInfo GetSeatPRI(int SeatNum)
{
	if ( Role == ROLE_Authority )
	{
		return Seats[SeatNum].SeatPawn.PlayerReplicationInfo;
	}
	else
	{
		return (SeatNum==0) ? PlayerReplicationInfo : PassengerPRI;
	}
}

/**
* CanEnterVehicle()
* @return true if Pawn P is allowed to enter this vehicle
*/
simulated function bool CanEnterVehicle(Pawn P)
{
	local int i;
	local bool bSeatAvailable, bIsHuman;
	local PlayerReplicationInfo SeatPRI;

	if (P.DrivenVehicle != None || (P.Controller == None) || !P.Controller.bIsPlayer || Health <= 0 || bDeleteMe)
	{
		return false;
	}

	// check for available seat, and no enemies in vehicle
	// allow humans to enter if full but with bots (TryToDrive() will kick one out if possible)
	bIsHuman = P.IsHumanControlled();
	bSeatAvailable = false;
	for (i=0;i<Seats.Length;i++)
	{
		SeatPRI = GetSeatPRI(i);
		if (SeatPRI == None)
		{
			bSeatAvailable = true;
		}
		// ignore team differences for humans, just to reduce failure chances in co-op
		else if (!bIsHuman && !WorldInfo.GRI.OnSameTeam(P, SeatPRI))
		{
			return false;
		}
		else if (bIsHuman && SeatPRI.bBot)
		{
			bSeatAvailable = true;
		}
	}

	return bSeatAvailable;
}

/**
* The pawn Driver has tried to take control of this vehicle
*
* @param	P		The pawn who wants to drive this vehicle
*/
function bool TryToDrive(Pawn P)
{
	local vector X,Y,Z;
	local bool bFreedSeat;
	local bool bEnteredVehicle;

	// Does the vehicle need to be uprighted?
	if ( bIsInverted && bMustBeUpright && !bVehicleOnGround && VSize(Velocity) <= 5.0f )
	{
		if ( bCanFlip )
		{
			bIsUprighting = true;
			UprightStartTime = WorldInfo.TimeSeconds;
			GetAxes(Rotation,X,Y,Z);
			bFlipRight = ((P.Location - Location) dot Y) > 0;
		}
		return false;
	}

	if ( !CanEnterVehicle(P) || (Vehicle(P) != None) )
	{
		return false;
	}

	// Check vehicle Locking....
	// Must be a non-disabled same team (or no team game) vehicle
	//if(!WorldInfo.Game.bTeamGame || WorldInfo.GRI.OnSameTeam(self,P))
	//{
		if (!AnySeatAvailable())
		{
			if (WorldInfo.GRI.OnSameTeam(self, P))
			{
				// kick out the first bot in the vehicle to make way for this driver
				bFreedSeat = KickOutBot();
			}

			if (!bFreedSeat)
			{
				// we were unable to kick a bot out
				return false;
			}
		}

		// Look to see if the driver seat is open
		bEnteredVehicle = (Driver == None) ? DriverEnter(P) : PassengerEnter(P, GetFirstAvailableSeat());

		if( bEnteredVehicle )
		{
			SetTexturesToBeResident( TRUE );
		}

		return bEnteredVehicle;
	//}

	return false;
}

/**
* kick out the first bot in the vehicle to make way for human driver
*/
function bool KickOutBot()
{
	local int i;
	local AIController B;

	for (i = 0; i < Seats.length; i++)
	{
		B = AIController(Seats[i].SeatPawn.Controller);
		if (B != None && Seats[i].SeatPawn.DriverLeave(false))
		{
			return true;
		}
	}
	return false;
}

/**
* Check to see if Other is too close to attack
*
* @param	Other		Actor to check against
* @returns true if he's too close
*/

function bool TooCloseToAttack(Actor Other)
{
	local int NeededPitch, i;
	local bool bControlledWeaponPawn;

	if (VSize(Location - Other.Location) > 2500.0)
	{
		return false;
	}

	if (Weapon == None)
	{
		if (Seats.length < 2)
		{
			return false;
		}
		for (i = 0; i < Seats.length; i++)
		{
			if (Seats[i].SeatPawn != None && Seats[i].SeatPawn.Controller != None)
			{
				bControlledWeaponPawn = true;
				if (!Seats[i].SeatPawn.TooCloseToAttack(Other))
				{
					return false;
				}
			}
		}

		return bControlledWeaponPawn;
	}

	NeededPitch = rotator(Other.GetTargetLocation(self) - Weapon.GetPhysicalFireStartLoc()).Pitch & 65535;
	return CheckTurretPitchLimit(NeededPitch, 0);
}

/** checks if the given pitch would be limited by the turret controllers, i.e. we cannot possibly fire in that direction
* @return whether the pitch would be constrained
*/
function bool CheckTurretPitchLimit(int NeededPitch, int SeatIndex)
{
	local int i;

	if (SeatIndex >= 0)
	{
		if (Seats[SeatIndex].TurretControllers.length > 0)
		{
			for (i = 0; i < Seats[SeatIndex].TurretControllers.Length; i++ )
			{
				if (!Seats[SeatIndex].TurretControllers[i].WouldConstrainPitch(NeededPitch, Mesh))
				{
					return false;
				}
			}

			return true;
		}
		else if (Seats[SeatIndex].Gun != None)
		{
			return (Cos(Abs(NeededPitch - (Rotation.Pitch & 65535)) / 182.0444) > Seats[SeatIndex].Gun.GetMaxFinalAimAdjustment());
		}
	}

	return false;
}

/**
* UpdateControllerOnPossess() override Pawn.UpdateControllerOnPossess() to keep from changing controller's rotation
*
* @param	bVehicleTransition	Will be true if this the pawn is entering/leaving a vehicle
*/
function UpdateControllerOnPossess(bool bVehicleTransition);

/**
* @returns the number of passengers in this vehicle
*/
simulated function int NumPassengers()
{
	local int i, Num;

	for (i = 0; i < Seats.length; i++)
	{
		if( (Seats[i].SeatPawn != None)
			&& (Seats[i].SeatPawn.Controller != None)
			)
		{
			Num++;
		}
	}

	return Num;
}

/**
* Called when a pawn enters the vehicle
*
* @Param P		The Pawn entering the vehicle
*/
function bool DriverEnter(Pawn P)
{
	P.StopFiring();

	if (Seats[0].Gun != none)
	{
		InvManager.SetCurrentWeapon(Seats[0].Gun);
	}

	Instigator = self;

	if ( !Super.DriverEnter(P) )
		return false;

	SetSeatStoragePawn(0,P);
	//	Seats[0].StoragePawn = P;

	if ( PlayerController(Controller) != None )
	{
		VehicleLostTime = 0;
	}
	StuckCount = 0;

	return true;
}

/**
* @returns the first available passenger seat, or -1 if there are none available
*/
function int GetFirstAvailableSeat()
{
	local int i;

	for (i = 1; i < Seats.Length; i++)
	{
		if (SeatAvailable(i))
		{
			return i;
		}
	}

	return -1;
}

/**
* Called when a passenger enters the vehicle
*
* @param P				The Pawn entering the vehicle
* @param SeatIndex		The seat where he is to sit
*/

function bool PassengerEnter(Pawn P, int SeatIndex)
{
	if (SeatIndex <= 0 || SeatIndex >= Seats.Length)
	{
		`warn("Attempted to add a passenger to unavailable passenger seat" @ SeatIndex);
		return false;
	}

	if ( !Seats[SeatIndex].SeatPawn.DriverEnter(p) )
	{
		return false;
	}

	SetSeatStoragePawn(SeatIndex,P);

	return true;
}

/**
* Called when the driver leaves the vehicle
*
* @param	bForceLeave		Is true if the driver was forced out
*/

event bool DriverLeave(bool bForceLeave)
{
	local bool bResult;
	local Pawn OldDriver;

	if (!bForceLeave && !bAllowedExit)
	{
		return false;
	}

	OldDriver = Driver;
	bResult = Super.DriverLeave(bForceLeave);
	if (bResult)
	{
		SetSeatStoragePawn(0,None);
		//		Seats[0].StoragePawn = None;
		// set Instigator to old driver so if vehicle continues on and runs someone over, the appropriate credit is given
		Instigator = OldDriver;
	}

	return bResult;
}

/**
* Called when a passenger leaves the vehicle
*
* @param	SeatIndex		Leaving from which seat
*/

function PassengerLeave(int SeatIndex)
{
	SetSeatStoragePawn(SeatIndex, None);
}

/**
*  AI code
*/
function bool Occupied()
{
	local int i;

	if ( Controller != None )
		return true;

	for ( i=0; i<Seats.Length; i++ )
		if ( !SeatAvailable(i) )
			return true;

	return false;
}

/**
* OpenPositionFor() returns true if there is a seat available for P
*
* @param P		The Pawn to test for
* @returns true if open
*/
function bool OpenPositionFor(Pawn P)
{
	local int i;

	if ( Controller == None )
		return true;

	if ( !WorldInfo.GRI.OnSameTeam(Controller,P) )
		return false;

	for ( i=0; i<Seats.Length; i++ )
		if ( SeatAvailable(i) )
			return true;

	return false;
}


/** Optionally support a horn sound */
function PlayHorn();

/* epic ===============================================
* ::StopsProjectile()
*
* returns true if Projectiles should call ProcessTouch() when they touch this actor
*/
simulated function bool StopsProjectile(Projectile P)
{
	// Don't block projectiles fired from this vehicle
	if ( P.Instigator == self )
		return false;

	// Don't block projectiles fired from turret on this vehicle
	return ( (P.Instigator == None) || (P.Instigator.Base != self) || !P.Instigator.IsA('GearWeaponPawn') );
}

function PlayHit(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo)
{
	Super.PlayHit(Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);
}

function NotifyTakeHit(Controller InstigatedBy, vector HitLocation, int Damage, class<DamageType> damageType, vector Momentum)
{
	local int i;
	local GearAI AIController;
	local TraceHitInfo DummyHitInfo;

	AIController = GearAI(Controller);
	if (AIController != None)
	{
		AIController.GearAINotifyTakeHit(InstigatedBy, HitLocation, Damage, damageType, Momentum, DummyHitInfo);
	}
	else
	{
		Super.NotifyTakeHit(InstigatedBy, HitLocation, Damage, DamageType, Momentum);
	}

	// notify anyone in turrets
	for (i = 1; i < Seats.length; i++)
	{
		if (Seats[i].SeatPawn != None)
		{
			AIController = GearAI(Seats[i].SeatPawn.Controller);
			if (AIController != None)
			{
				AIController.GearAINotifyTakeHit(InstigatedBy, HitLocation, Damage, damageType, Momentum, DummyHitInfo);
			}
			else
			{
				Seats[i].SeatPawn.NotifyTakeHit(InstigatedBy, HitLocation, Damage, DamageType, Momentum);
			}
		}
	}
}

/*********************************************************************************************
* Vehicle Weapons, Drivers and Passengers
*********************************************************************************************/

/**
* Create all of the vehicle weapons
*/
function InitializeSeats()
{
	local int i;
	if (Seats.Length==0)
	{
		`log("WARNING: Vehicle ("$self$") **MUST** have at least one seat defined");
		destroy();
		return;
	}

	for(i=0;i<Seats.Length;i++)
	{
		// Seat 0 = Driver Seat.  It doesn't get a WeaponPawn

		if (i>0)
		{
			Seats[i].SeatPawn = Spawn(class'GearWeaponPawn', self);
			Seats[i].SeatPawn.SetBase(self);
			if(Seats[i].GunClass != None)
			{
				Seats[i].Gun = GearVehicleWeapon(Seats[i].SeatPawn.InvManager.CreateInventory(Seats[i].GunClass));
				Seats[i].Gun.SetBase(self);
			}
			Seats[i].SeatPawn.EyeHeight = Seats[i].SeatPawn.BaseEyeheight;
			GearWeaponPawn(Seats[i].SeatPawn).MyVehicleWeapon = Seats[i].Gun;
			GearWeaponPawn(Seats[i].SeatPawn).MyVehicle = self;
			GearWeaponPawn(Seats[i].SeatPawn).MySeatIndex = i;

			if ( Seats[i].ViewPitchMin != 0.0f )
			{
				GearWeaponPawn(Seats[i].SeatPawn).ViewPitchMin = Seats[i].ViewPitchMin;
			}
			else
			{
				GearWeaponPawn(Seats[i].SeatPawn).ViewPitchMin = ViewPitchMin;
			}


			if ( Seats[i].ViewPitchMax != 0.0f )
			{
				GearWeaponPawn(Seats[i].SeatPawn).ViewPitchMax = Seats[i].ViewPitchMax;
			}
			else
			{
				GearWeaponPawn(Seats[i].SeatPawn).ViewPitchMax = ViewPitchMax;
			}
		}
		else
		{
			Seats[i].SeatPawn = self;
			if(Seats[i].GunClass != None)
			{
				Seats[i].Gun = GearVehicleWeapon(InvManager.CreateInventory(Seats[i].GunClass));
				Seats[i].Gun.SetBase(self);
			}
		}

		Seats[i].SeatPawn.DriverDamageMult = Seats[i].DriverDamageMult;
		Seats[i].SeatPawn.bDriverIsVisible = Seats[i].bSeatVisible;

		if (Seats[i].Gun!=none)
		{
			Seats[i].Gun.SeatIndex = i;
			Seats[i].Gun.MyVehicle = self;
		}

		// Cache the names used to access various variables
	}
}

simulated function PreCacheSeatNames()
{
	local int i;
	for (i=0;i<Seats.Length;i++)
	{
		Seats[i].WeaponRotationName	= NAME( Seats[i].TurretVarPrefix$"WeaponRotation" );
		Seats[i].FlashLocationName	= NAME( Seats[i].TurretVarPrefix$"FlashLocation" );
		Seats[i].FlashCountName		= NAME( Seats[i].TurretVarPrefix$"FlashCount" );
		Seats[i].FiringModeName		= NAME( Seats[i].TurretVarPrefix$"FiringMode" );
	}
}

simulated function InitializeTurrets()
{
	local int Seat, i;
	local GearSkelCtrl_TurretConstrained Turret;
	local vector PivotLoc, MuzzleLoc;

	if (Mesh == None)
	{
		`warn("No Mesh for" @ self);
	}
	else
	{
		for (Seat = 0; Seat < Seats.Length; Seat++)
		{
			for (i = 0; i < Seats[Seat].TurretControls.Length; i++)
			{
				Turret = GearSkelCtrl_TurretConstrained( Mesh.FindSkelControl(Seats[Seat].TurretControls[i]) );
				if ( Turret != none )
				{
					Turret.AssociatedSeatIndex = Seat;
					Seats[Seat].TurretControllers[i] = Turret;

					// Initialize turrets to vehicle rotation.
					Turret.InitTurret(Rotation, Mesh);
				}
				else
				{
					`warn("Failed to find skeletal controller named" @ Seats[Seat].TurretControls[i] @ "(Seat "$Seat$") for" @ self @ "in AnimTree" @ Mesh.AnimTreeTemplate);
				}
			}

			if(Role == ROLE_Authority)
			{
				SeatWeaponRotation(Seat, Rotation, FALSE);
			}

			// Calculate Z distance between weapon pivot and muzzle
			PivotLoc = GetSeatPivotPoint(Seat);
			GetBarrelLocationAndRotation(Seat, MuzzleLoc);

			Seats[Seat].PivotFireOffsetZ = MuzzleLoc.Z - PivotLoc.Z;
		}
	}
}

function PossessedBy(Controller C, bool bVehicleTransition)
{
	local GearWeapon GearWeap;

	super.PossessedBy(C,bVehicleTransition);

	if (Seats[0].Gun!=none)
		Seats[0].Gun.ClientWeaponSet(false);

	// Make sure to cache GearAIController pointer
	GearWeap = GearWeapon(Weapon);
	if(GearWeap != None)
	{
		GearWeap.GearAIController = GearAI(Controller);
		GearWeap.AIController	  = AIController(Controller);
	}

	MyGearAI = GearAI(Controller);
}

simulated function SetFiringMode(byte FiringModeNum)
{
	SeatFiringMode(0, FiringModeNum, false);
	if(Seats[0].Gun != none)
	{
		Seats[0].Gun.FireModeUpdated(FiringModeNum,FALSE);
	}
}

simulated function ClearFlashCount(Weapon Who)
{
	local GearVehicleWeapon VWeap;

	VWeap = GearVehicleWeapon(Who);
	if (VWeap != none)
	{
		VehicleAdjustFlashCount(VWeap.SeatIndex, SeatFiringMode(VWeap.SeatIndex,,true), true);
	}

}

simulated function IncrementFlashCount(Weapon Who, byte FireModeNum)
{
	local GearVehicleWeapon VWeap;

	VWeap = GearVehicleWeapon(Who);
	if (VWeap != none)
	{
		VehicleAdjustFlashCount(VWeap.SeatIndex, FireModeNum, false);
	}
}

function SetFlashLocation( Weapon Who, byte FireModeNum, vector NewLoc )
{
	local GearVehicleWeapon VWeap;

	VWeap = GearVehicleWeapon(Who);
	if (VWeap != none)
	{
		VehicleAdjustFlashLocation(VWeap.SeatIndex, FireModeNum, NewLoc,  false);
	}
}

/**
* Reset flash location variable. and call stop firing.
* Network: Server only
*/
function ClearFlashLocation( Weapon Who )
{
	local GearVehicleWeapon VWeap;

	VWeap = GearVehicleWeapon(Who);
	if (VWeap != none)
	{
		VehicleAdjustFlashLocation(VWeap.SeatIndex, SeatFiringMode(VWeap.SeatIndex,,true), Vect(0,0,0),  true);
	}
}

simulated native function GetBarrelLocationAndRotation(int SeatIndex, out vector SocketLocation, optional out rotator SocketRotation);
simulated native function vector GetEffectLocation(int SeatIndex);

simulated native event Vector GetPhysicalFireStartLoc(GearWeapon ForWeapon);

/** Do trace for weapon, ignoring hits between camera and vehicle, and things we don't want to aim at. */
simulated native final function Actor DoTurretTrace(const vector Start, const vector End, int SeatIndex, out vector OutHitLocation);


/**
* This function returns the aim for the weapon
*/
function rotator GetWeaponAim(GearVehicleWeapon VWeapon)
{
	local vector SocketLocation, CameraLocation, RealAimPoint, DesiredAimPoint, HitLocation, DirA, DirB;
	local rotator CameraRotation, SocketRotation, ControllerAim, AdjustedAim;
	local float DiffAngle, MaxAdjust;
	local Controller C;
	local PlayerController PC;
	local Quat Q;

	if ( VWeapon != none )
	{
		C = Seats[VWeapon.SeatIndex].SeatPawn.Controller;

		PC = PlayerController(C);
		if (PC != None)
		{
			PC.GetPlayerViewPoint(CameraLocation, CameraRotation);
			DesiredAimPoint = CameraLocation + Vector(CameraRotation) * VWeapon.GetTraceRange();

			if (DoTurretTrace(CameraLocation, DesiredAimPoint, VWeapon.SeatIndex, HitLocation) != None)
			{
				DesiredAimPoint = HitLocation;
			}
		}
		else if (C != None)
		{
			DesiredAimPoint = C.GetFocalPoint();
		}

		if ( Seats[VWeapon.SeatIndex].GunSocket.Length>0 )
		{
			GetBarrelLocationAndRotation(VWeapon.SeatIndex, SocketLocation, SocketRotation);
			if(VWeapon.bIgnoreSocketPitchRotation || ((DesiredAimPoint.Z - Location.Z)<0 && VWeapon.bIgnoreDownwardPitch))
			{
				SocketRotation.Pitch = Rotator(DesiredAimPoint - Location).Pitch;
			}
		}
		else
		{
			SocketLocation = Location;
			SocketRotation = Rotator(DesiredAimPoint - Location);
		}

		RealAimPoint = SocketLocation + Vector(SocketRotation) * VWeapon.GetTraceRange();
		DirA = normal(DesiredAimPoint - SocketLocation);
		DirB = normal(RealAimPoint - SocketLocation);
		DiffAngle = ( DirA dot DirB );
		MaxAdjust = VWeapon.GetMaxFinalAimAdjustment();
		if ( DiffAngle >= MaxAdjust )
		{
			// bit of a hack here to make bot aiming and single player autoaim work
			ControllerAim = (C != None) ? C.Rotation : Rotation;
			AdjustedAim = VWeapon.GetAdjustedAim(SocketLocation);
			if (AdjustedAim == VWeapon.Instigator.GetBaseAimRotation() || AdjustedAim == ControllerAim)
			{
				// no adjustment
				return rotator(DesiredAimPoint - SocketLocation);
			}
			else
			{
				// FIXME: AdjustedAim.Pitch = Instigator.LimitPitch(AdjustedAim.Pitch);
				return AdjustedAim;
			}
		}
		else
		{
			Q = QuatFromAxisAndAngle(Normal(DirB cross DirA), ACos(MaxAdjust));
			return Rotator( QuatRotateVector(Q,DirB));
		}
	}
	else
	{
		return Rotation;
	}
}

/** Gives the vehicle an opportunity to override the functionality of the given fire mode, called on both the owning client and the server
@return false to allow the vehicle weapon to use its behavior, true to override it */
simulated function bool OverrideBeginFire(byte FireModeNum);
simulated function bool OverrideEndFire(byte FireModeNum);

/**
* GetWeaponViewAxes should be subclassed to support returningthe rotator of the various weapon points.
*/
simulated function GetWeaponViewAxes( GearWeapon WhichWeapon, out vector xaxis, out vector yaxis, out vector zaxis )
{
	GetAxes( Controller.Rotation, xaxis, yaxis, zaxis );
}

simulated function WeaponFired( bool bViaReplication, optional vector HitLocation)
{
	VehicleWeaponFired(bViaReplication, HitLocation, 0);
}

/**
* Vehicle will want to override WeaponFired and pass off the effects to the proper Seat
*/
simulated function VehicleWeaponFired( bool bViaReplication, vector HitLocation, int SeatIndex )
{
	// Trigger any vehicle Firing Effects
	if ( WorldInfo.NetMode != NM_DedicatedServer )
	{
		VehicleWeaponFireEffects(HitLocation, SeatIndex);

		if (SeatIndex == 0)
		{
			Seats[SeatIndex].Gun = GearVehicleWeapon(Weapon);
		}
	}
}

simulated function WeaponStoppedFiring( bool bViaReplication )
{
	VehicleWeaponStoppedFiring(bViaReplication, 0);
}

simulated function VehicleWeaponStoppedFiring( bool bViaReplication, int SeatIndex )
{

}

/**
* This function should be subclassed and manage the different effects
*/
simulated function VehicleWeaponFireEffects(vector HitLocation, int SeatIndex)
{

}

/**
* This function is here so that children vehicles can get access to the retrace to get the hitnormal.  See the Dark Walker
*/

simulated function actor FindWeaponHitNormal(out vector HitLocation, out Vector HitNormal, vector End, vector Start, out TraceHitInfo HitInfo)
{
	return Trace(HitLocation, HitNormal, End, Start, true,, HitInfo, TRACEFLAG_Bullet);
}


/**
* These two functions needs to be subclassed in each weapon
*/
simulated function VehicleAdjustFlashCount(int SeatIndex, byte FireModeNum, optional bool bClear)
{
	if (bClear)
	{
		SeatFlashCount( SeatIndex, 0 );
		VehicleWeaponStoppedFiring( false, SeatIndex );
	}
	else
	{
		SeatFiringMode(SeatIndex,FireModeNum);
		SeatFlashCount( SeatIndex, SeatFlashCount(SeatIndex,,true)+1 );
		
		if(ROLE==ROLE_Authority) // if we're a client, then the flash count is going to get repped right back to us.. so don't play this now
		{
			VehicleWeaponFired( false, vect(0,0,0), SeatIndex );
		}
		Seats[SeatIndex].BarrelIndex++;
	}

	bForceNetUpdate = TRUE;	// Force replication
}

simulated function VehicleAdjustFlashLocation(int SeatIndex, byte FireModeNum, vector NewLocation, optional bool bClear)
{
	if (bClear)
	{
		SeatFlashLocation( SeatIndex, Vect(0,0,0) );
		VehicleWeaponStoppedFiring( false, SeatIndex );
	}
	else
	{
		// Make sure 2 consecutive flash locations are different, for replication
		if( NewLocation == SeatFlashLocation(SeatIndex,,true) )
		{
			NewLocation += vect(0,0,1);
		}

		// If we are aiming at the origin, aim slightly up since we use 0,0,0 to denote
		// not firing.
		if( NewLocation == vect(0,0,0) )
		{
			NewLocation = vect(0,0,1);
		}

		SeatFiringMode(SeatIndex,FireModeNum);
		SeatFlashLocation( SeatIndex, NewLocation );
		VehicleWeaponFired( false, NewLocation, SeatIndex );
		Seats[SeatIndex].BarrelIndex++;
	}


	bForceNetUpdate = TRUE;	// Force replication
}

/** Used by PlayerController.FindGoodView() in RoundEnded State */
simulated function FindGoodEndView(PlayerController PC, out Rotator GoodRotation)
{
	local vector cameraLoc;
	local rotator cameraRot, ViewRotation;
	local int tries;
	local float bestdist, newdist, FOVAngle;

	ViewRotation = GoodRotation;
	ViewRotation.Pitch = 56000;
	tries = 0;
	bestdist = 0.0;
	for (tries=0; tries<16; tries++)
	{
		cameraLoc = Location;
		cameraRot = ViewRotation;
		CalcCamera( 0, cameraLoc, cameraRot, FOVAngle );
		newdist = VSize(cameraLoc - Location);
		if (newdist > bestdist)
		{
			bestdist = newdist;
			GoodRotation = cameraRot;
		}
		ViewRotation.Yaw += 4096;
	}
}

/** Return offsets to use for vehicle camera */
simulated event GetVehicleViewOffsets(int SeatIndex, bool bSplitScreen, out vector out_Low, out vector out_Mid, out vector out_High)
{
	out_Low		= Seats[SeatIndex].CameraViewOffsetLow;
	out_Mid 	= Seats[SeatIndex].CameraViewOffsetMid;
	out_High	= Seats[SeatIndex].CameraViewOffsetHigh;
}

/**
* returns the camera focus position (without camera lag)
*/
simulated function vector GetCameraFocus(int SeatIndex)
{
	local vector CamStart, HitLocation, HitNormal;
	local actor HitActor;
	local bool bFoundSocket;

	//  calculate camera focus
	if ( !bDeadVehicle && Seats[SeatIndex].CameraTag != '' )
	{
		// See if we have a socket with this name - if not use bone
		bFoundSocket = (Mesh.GetSocketByName(Seats[SeatIndex].CameraTag) != None);
		if(bFoundSocket)
		{
			Mesh.GetSocketWorldLocationAndRotation(Seats[SeatIndex].CameraTag, CamStart);
		}
		else
		{
			CamStart = Mesh.GetBoneLocation(Seats[SeatIndex].CameraTag, 0);
		}

		if(bDoActorLocationToCamStartTrace)
		{
			// Do a line check from actor location to this socket. If we hit the world, use that location instead.
			HitActor = Trace(HitLocation, HitNormal, CamStart, Location, FALSE, vect(12,12,12));
			if( HitActor != None )
			{
				CamStart = HitLocation;
			}
		}
	}
	else
	{
		CamStart = Location;
	}
	CamStart += (Seats[SeatIndex].CameraBaseOffset >> Rotation);
	//DrawDebugSphere(CamStart, 8, 10, 0, 255, 0, FALSE);
	//DrawDebugSphere(Location, 8, 10, 255, 255, 0, FALSE);
	return CamStart;
}

simulated event float GetCameraFOV(int SeatIndex)
{
	return DefaultFOV;
}

/**
* returns the camera focus position (adjusted for camera lag)
*/
simulated event vector GetCameraStart(int SeatIndex)
{
	local int i, len, obsolete;
	local vector CamStart;
	local float OriginalCamZ;
	local TimePosition NewPos, PrevPos;
	local float DeltaTime;

	// If we've already updated the cameraoffset, just return it
	len = OldPositions.Length;
	if (len > 0 && SeatIndex == 0 && OldPositions[len-1].Time == WorldInfo.TimeSeconds)
	{
		return CameraOffset + Location;
	}

	CamStart = GetCameraFocus(SeatIndex);
	OriginalCamZ = CamStart.Z;
	if (CameraLag == 0)
	{
		return CamStart;
	}

	// cache our current location
	NewPos.Time = WorldInfo.TimeSeconds;
	NewPos.Position = CamStart;
	OldPositions[len] = NewPos;

	// if no old locations saved, return offset
	if ( len == 0 )
	{
		CameraOffset = CamStart - Location;
		return CamStart;
	}
	DeltaTime = (len > 1) ? (WorldInfo.TimeSeconds - OldPositions[len-2].Time) : 0.0;

	len = OldPositions.Length;
	obsolete = 0;
	for ( i=0; i<len; i++ )
	{
		if ( OldPositions[i].Time < WorldInfo.TimeSeconds - CameraLag )
		{
			PrevPos = OldPositions[i];
			obsolete++;
		}
		else
		{
			if ( Obsolete > 0 )
			{
				// linear interpolation to maintain same distance in past
				if ( (i == 0) || (OldPositions[i].Time - PrevPos.Time > 0.2) )
				{
					CamStart = OldPositions[i].Position;
				}
				else
				{
					CamStart = PrevPos.Position + (OldPositions[i].Position - PrevPos.Position)*(WorldInfo.TimeSeconds - CameraLag - PrevPos.Time)/(OldPositions[i].Time - PrevPos.Time);
				}
				if ( Obsolete > 1)
					OldPositions.Remove(0, obsolete-1);
			}
			else
			{
				CamStart = OldPositions[i].Position;
			}
			// need to smooth camera to vehicle distance, since vehicle update rate not synched with frame rate
			if ( DeltaTime > 0 )
			{
				DeltaTime *= CameraSmoothingFactor;
				CameraOffset = (CamStart - Location)*DeltaTime + CameraOffset*(1-DeltaTime);
				if ( bNoZSmoothing )
				{
					// don't smooth z - want it bouncy
					CameraOffset.Z = CamStart.Z - Location.Z;
				}
			}
			else
			{
				CameraOffset = CamStart - Location;
			}
			CamStart = CameraOffset + Location;
			if ( bLimitCameraZLookingUp )
			{
				CamStart.Z = LimitCameraZ(CamStart.Z, OriginalCamZ, SeatIndex);
			}
			return CamStart;
		}
	}
	CamStart = OldPositions[len-1].Position;
	if ( bLimitCameraZLookingUp )
	{
		CamStart.Z = LimitCameraZ(CamStart.Z, OriginalCamZ, SeatIndex);
	}
	return CamStart;
}


/**
* returns the camera focus position (adjusted for camera lag)
*/
simulated function float LimitCameraZ(float CurrentCamZ, float OriginalCamZ, int SeatIndex)
{
	local rotator CamRot;
	local float Pct;

	CamRot = Seats[SeatIndex].SeatPawn.GetViewRotation();
	CamRot.Pitch = CamRot.Pitch & 65535;
	if ( (CamRot.Pitch < 32768) )
	{
		Pct = FClamp(float(CamRot.Pitch)*0.00025, 0.0, 1.0);
		CurrentCamZ = OriginalCamZ*Pct + CurrentCamZ*(1.0-Pct);
	}
	return CurrentCamZ;
}

/**
* Returns "worst case" location for camera.  This should
* be a point inside the bounding region of the vehicle, but somewhere
* that the camera could peacefully exist (ie not inside the
* mesh)
*/
simulated function vector GetCameraWorstCaseLoc(int SeatIndex)
{
	//@fixme, choose proper seat!
	if ( (SeatIndex >= 0) && (SeatIndex < Seats.Length) )
	{
		return Location + (Seats[SeatIndex].WorstCameraLocOffset >> Rotation);
	}
	else
	{
		`log("GearVehicle.GetCameraWorstCaseLoc : Invalid Seat"@SeatIndex);
		return vect(0,0,0);
	}
}


/** turns off collision on the vehicle when it's almost fully burned out */
simulated function DisableCollision()
{
	SetCollision(false);
	Mesh.SetBlockRigidBody(false);
}


simulated state DyingVehicle
{
	ignores Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling, BreathTimer, FellOutOfWorld;

	simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon) {}
	simulated function PlayNextAnimation() {}
	singular event BaseChange() {}
	event Landed(vector HitNormal, Actor FloorActor) {}

	function bool Died(Controller Killer, class<DamageType> damageType, vector HitLocation);

	simulated event PostRenderFor(PlayerController PC, Canvas Canvas, vector CameraPosition, vector CameraDir) {}

	simulated function BlowupVehicle(Controller Killer) {}


	/** This does the secondary explosion of the vehicle (e.g. from reserve fuel tanks finally blowing / ammo blowing up )**/

	simulated event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		if (DamageType != None)
		{
			Damage *= DamageType.static.VehicleDamageScalingFor(self);

			Health -= Damage;
			AddVelocity(Momentum, HitLocation, DamageType, HitInfo);
		}
	}

	simulated function CheckDestroy()
	{
		if (!PlayerCanSeeMe())
		{
			Destroy();
		}
	}

	simulated function BeginState(name PreviousStateName)
	{
		local int i;

		StopVehicleSounds();

		LastCollisionSoundTime = WorldInfo.TimeSeconds;

		//for(i=0; i<DamageSkelControls.length; i++)
		//{
		//	DamageSkelControls[i].HealthPerc = 0.f;
		//}

		if (WorldInfo.NetMode != NM_DedicatedServer)
		{
			PerformDeathEffects();
		}
		//SetBurnOut();
		if (WorldInfo.GRI.IsMultiplayerGame())
		{
			SetTimer( 2.0, true, nameof(CheckDestroy) );
		}

		if (Controller != None)
		{
			if (Controller.bIsPlayer)
			{
				DetachFromController();
			}
			else
			{
				Controller.Destroy();
			}
		}

		for (i = 0; i < Attached.length; i++)
		{
			if (Attached[i] != None)
			{
				Attached[i].PawnBaseDied();
			}
		}
	}

	simulated function PerformDeathEffects()
	{
		if (bHasTurretExplosion)
		{
			TurretExplosion();
		}
	}


}

simulated function TurretExplosion()
{
	local vector SpawnLoc;
	local rotator SpawnRot;
	//local SkelControlBase SK;
	//local vector Force;

	Mesh.GetSocketWorldLocationAndRotation(TurretSocketName,SpawnLoc,SpawnRot);

	//WorldInfo.MyEmitterPool.SpawnEmitter( class'UTEmitter'.static.GetTemplateForDistance(DistanceTurretExplosionTemplates, SpawnLoc, WorldInfo),Location, Rotation );

/*
	DestroyedTurret = Spawn(class'UTVehicleDeathPiece',self,,SpawnLoc+TurretOffset,SpawnRot,,true);
	if(DestroyedTurret != none)
	{
		StaticMeshComponent(DestroyedTurret.GibMeshComp).SetStaticMesh(DestroyedTurretTemplate);
		//DestroyedTurret.SetCollision( FALSE, FALSE, TRUE );
		DestroyedTurret.GibMeshComp.SetBlockRigidBody( FALSE );
		// @todo make a RBChannelContainer function to do all this
		DestroyedTurret.GibMeshComp.SetRBChannel( RBCC_Nothing ); // nothing will request to collide with us
		DestroyedTurret.GibMeshComp.SetRBCollidesWithChannel( RBCC_Default, FALSE );
		DestroyedTurret.GibMeshComp.SetRBCollidesWithChannel( RBCC_Pawn, FALSE );
		DestroyedTurret.GibMeshComp.SetRBCollidesWithChannel( RBCC_Vehicle, FALSE );
		DestroyedTurret.GibMeshComp.SetRBCollidesWithChannel( RBCC_GameplayPhysics, FALSE );
		DestroyedTurret.GibMeshComp.SetRBCollidesWithChannel( RBCC_EffectPhysics, FALSE );

		DestroyedTurret.SetTimer( 0.100f, FALSE, nameof(DestroyedTurret.TurnOnCollision) );

		SK = Mesh.FindSkelControl(TurretScaleControlName);
		if(SK != none)
		{
			SK.boneScale = 0.0f;
		}
		Force = Vect(0,0,1);
		Force *= TurretExplosiveForce;
		// Let's at least try and go off in some direction
		Force.X = FRand()*1000.0f + 400.0f;
		Force.Y = FRand()*1000.0f + 400.0f;
		DestroyedTurret.GibMeshComp.AddImpulse(Force);
		DestroyedTurret.GibMeshComp.SetRBAngularVelocity(VRand()*500.0f);
	}
*/
}

simulated function StopVehicleSounds()
{
	local int seatIdx;
	Super.StopVehicleSounds();
	for(seatIdx=0;seatIdx < Seats.Length; ++seatIdx)
	{
		if(Seats[seatIdx].SeatMotionAudio != none)
		{
			Seats[seatIdx].SeatMotionAudio.Stop();
		}
	}
}

simulated function AttachDriver( Pawn P )
{
	local GearPawn WP;

	// reset vehicle camera
	OldPositions.remove(0,OldPositions.Length);
	Eyeheight = BaseEyeheight;

	if( !bAttachDriver )
	{
		return;
	}

	WP = GearPawn(P);
	if (WP!=none)
	{
		WP.SetCollision( false, false);
		WP.bCollideWorld = false;
		WP.SetBase(none);
		WP.SetHardAttach(true);
		WP.SetLocation( Location );
		WP.SetPhysics( PHYS_None );
		WP.Mesh.SetBlockRigidBody(FALSE);
		WP.Mesh.SetHasPhysicsAssetInstance(FALSE);

		SitDriver( WP, 0);
	}
}

simulated function DetachDriver( Pawn P )
{
    local GearPawn WP;

    Super.DetachDriver(P);

    WP = GearPawn(P);
	if (WP != none)
	{
	   WP.Mesh.SetBlockRigidBody(TRUE);
	   WP.Mesh.SetHasPhysicsAssetInstance(TRUE);

	   // Ensure mesh translation is back to normal
	   WP.Mesh.SetTranslation(WP.default.Mesh.Translation);

	   WP.Mesh.bUpdateKinematicBonesFromAnimation = TRUE;
	   WP.Mesh.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
	   WP.Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, Mesh);
	   WP.bDisableMeshTranslationChanges = FALSE;

	   // set the DLE back to self
	   WP.Mesh.SetShadowParent(None);
	}
}

simulated function SitDriver( GearPawn WP, int SeatIndex)
{
	if (Seats[SeatIndex].SeatBone != '')
	{
		WP.SetBase( Self, , Mesh, Seats[SeatIndex].SeatBone);
	}
	else
	{
		WP.SetBase( Self );
	}

	// Shut down physics when getting in vehicle.
	if(WP.Mesh.PhysicsAssetInstance != None)
	{
		WP.Mesh.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
	}
	WP.Mesh.bUpdateKinematicBonesFromAnimation = FALSE;
	WP.Mesh.PhysicsWeight = 0.0;
	WP.bDisableMeshTranslationChanges = TRUE;

	if ( Seats[SeatIndex].bSeatVisible )
	{
		if ( (WP.Mesh != None) && (Mesh != None) )
		{
			WP.Mesh.SetShadowParent(Mesh);
		}
		WP.Mesh.SetOwnerNoSee(FALSE);
		WP.SetRelativeLocation( Seats[SeatIndex].SeatOffset );
		WP.SetRelativeRotation( Seats[SeatIndex].SeatRotation );
		WP.Mesh.SetTranslation(vect(0,0,0));
		WP.SetHidden(FALSE);
	}
	else
	{
		WP.SetHidden(TRUE);
	}
}

simulated function String GetHumanReadableName()
{
	if (VehicleNameString == "")
	{
		return ""$Class;
	}
	else
	{
		return VehicleNameString;
	}
}


event OnPropertyChange(name PropName)
{
	local int i;

	for (i=0;i<Seats.Length;i++)
	{
		if ( Seats[i].bSeatVisible )
		{
			Seats[i].StoragePawn.SetRelativeLocation( Seats[i].SeatOffset );
			Seats[i].StoragePawn.SetRelativeRotation( Seats[i].SeatRotation );
		}
	}
}

simulated function int GetHealth(int SeatIndex)
{
	return Health;
}

function float GetCollisionDamageModifier(const out array<RigidBodyContactInfo> ContactInfos)
{
	local float Angle;
	local vector X, Y, Z;

	if (bReducedFallingCollisionDamage)
	{
		GetAxes(Rotation, X, Y, Z);
		Angle = ContactInfos[0].ContactNormal Dot Z;
		return (Angle < 0.f) ? Square(CollisionDamageMult * (1.0+Angle)) : Square(CollisionDamageMult);
	}
	else
	{
		return Square(CollisionDamageMult);
	}
}

simulated event RigidBodyCollision( PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent,
								   const out CollisionImpactData Collision, int ContactIndex )
{
	local int Damage;
	local GearVehicle V;
	local Controller InstigatorController;

	if (LastCollisionDamageTime != WorldInfo.TimeSeconds)
	{
		Super.RigidBodyCollision(HitComponent, OtherComponent, Collision, ContactIndex);

		if (OtherComponent != None && GearPawn(OtherComponent.Owner) != None && OtherComponent.Owner.Physics == PHYS_RigidBody)
		{
			RanInto(OtherComponent.Owner);
		}
		else if(Mesh != None)
		{
			// take impact damage
			Damage = int(VSizeSq(Mesh.GetRootBodyInstance().PreviousVelocity) * GetCollisionDamageModifier(Collision.ContactInfos));
			if (Damage > 1)
			{
				// if rammed other vehicle, give that vehicle's Instigator credit for the damage
				if (OtherComponent != None)
				{
					V = GearVehicle(OtherComponent.Owner);
					if (V != None)
					{
						InstigatorController = V.GetCollisionDamageInstigator();
					}
				}
				if (InstigatorController == None)
				{
					InstigatorController = GetCollisionDamageInstigator();
				}
				TakeDamage(Damage, InstigatorController, Collision.ContactInfos[0].ContactPosition, vect(0,0,0), class'DmgType_Crushed');
				LastCollisionDamageTime = WorldInfo.TimeSeconds;
			}
		}
	}
}

/** Called when a contact with a large penetration occurs. */
event RBPenetrationDestroy()
{
	if (Health > 0)
	{
		//`log("Penetration Death:"@self@Penetration);
		TakeDamage(10000, GetCollisionDamageInstigator(), Location, vect(0,0,0), class'DmgType_Crushed');
	}
}

/** called when the client receives a change to Health
* if LastTakeHitInfo changed in the same received bunch, always called *after* PlayTakeHitEffects()
* (this is so we can use the damage info first for more accurate modelling and only use the direct health change for corrections)
*/
simulated event ReceivedHealthChange(int OldHealth)
{

}


/**
* We extend GetSVehicleDebug to include information about the seats array
*
* @param	DebugInfo		We return the text to display here
*/

simulated function GetSVehicleDebug( out Array<String> DebugInfo )
{
	local int i;

	Super.GetSVehicleDebug(DebugInfo);

	if (GearVehicleSimCar(SimObj) != None)
	{
		DebugInfo[DebugInfo.Length] = "ActualThrottle: "$GearVehicleSimCar(SimObj).ActualThrottle;
	}

	DebugInfo[DebugInfo.Length] = "";
	DebugInfo[DebugInfo.Length] = "----Seats----: ";
	for (i=0;i<Seats.Length;i++)
	{
		DebugInfo[DebugInfo.Length] = "Seat"@i$":"@Seats[i].Gun @ "Rotation" @ SeatWeaponRotation(i,,true) @ "FiringMode" @ SeatFiringMode(i,,true) @ "Barrel" @ Seats[i].BarrelIndex;
		if (Seats[i].Gun != None)
		{
			DebugInfo[DebugInfo.length - 1] @= "IsAimCorrect" @ Seats[i].Gun.IsAimCorrect();
		}
	}
}

function SetSeatStoragePawn(int SeatIndex, Pawn PawnToSit)
{
	local int Mask;

	Seats[SeatIndex].StoragePawn = PawnToSit;
	if ( (SeatIndex == 1) && (Role == ROLE_Authority) )
	{
		PassengerPRI = (PawnToSit == None) ? None : Seats[SeatIndex].SeatPawn.PlayerReplicationInfo;
	}

	Mask = 1 << SeatIndex;

	if ( PawnToSit != none )
	{
		SeatMask = SeatMask | Mask;
	}
	else
	{
		if ( (SeatMask & Mask) > 0)
		{
			SeatMask = SeatMask ^ Mask;
		}
	}

}

function bool CanAttack(Actor Other)
{
	local float MaxRange;
	local Weapon W;

	if ( bShouldLeaveForCombat && Driver != None && Driver.InvManager != None && Controller != None &&
		!WorldInfo.GRI.OnSameTeam(self, Other) && !IsHumanControlled() )
	{
		// return whether can attack with handheld weapons (assume bot will leave if it actually decides to attack)
		foreach Driver.InvManager.InventoryActors(class'Weapon', W)
		{
			MaxRange = FMax(MaxRange, W.MaxRange());
		}
		return (VSize(Location - Other.Location) <= MaxRange && Controller.LineOfSightTo(Other));
	}
	else
	{
		return Super.CanAttack(Other);
	}
}

/** Default input */
simulated function RemapPlayerInput(GearPC GPC, out FLOAT OutForward, out FLOAT OutStrafe, out FLOAT OutUp)
{
	local GearPlayerInput_Base GPI;

	OutForward = GPC.PlayerInput.RawJoyUp;
	OutStrafe = GPC.PlayerInput.RawJoyRight;

	GPI = GearPlayerInput_Base(GPC.PlayerInput);
	OutUp = GPI.IsButtonActive(GB_A) ? 1.0 : 0.0;
}

simulated function bool WantsCrosshair(PlayerController PC)
{
	return TRUE;
}

simulated function int GetVehicleDefaultHealth()
{
	return 0;
}

/** Hook for Kismet instigated player death. */
final event VehicleScriptedDeath()
{
	Died(Controller,class'GDT_ScriptedGib',Location+vect(0,0,32));
}

function SetMovementPhysics();

simulated function Tick(float DeltaTime)
{
	local float NewBrightness;

	Super.Tick(DeltaTime);

	if (AIController(Controller) != None && Weapon != None)
	{
		if (Controller.bFire != 0)
		{
			if (!Weapon.IsFiring())
			{
				Weapon.StartFire(0);
			}
		}
		else if (Weapon.IsFiring())
		{
			Weapon.StopFire(0);
		}
	}

	// If we have some muzzle light time remaining - update brightness
	if(MuzzleLightInfo.LightTimeRemaining > 0.0)
	{
		MuzzleLightInfo.LightTimeRemaining -= DeltaTime;

		// Light has stopped - disable it
		if(MuzzleLightInfo.LightTimeRemaining <= 0.0)
		{
			MuzzleLightInfo.MuzzleLight.SetEnabled(FALSE);
		}
		// Light is still on - update brightness
		else
		{
			NewBrightness = MuzzleLightInfo.LightInitialBrightness * (MuzzleLightInfo.LightTimeRemaining/MuzzleLightInfo.LightFadeTime);
			MuzzleLightInfo.MuzzleLight.SetLightProperties(NewBrightness);
		}
	}
}

event bool ContinueOnFoot();

simulated function OnTeleport(SeqAct_Teleport Action)
{
	local array<Object> objVars;
	local int idx;
	local Actor destActor;
	local Controller C;

	// find the first supplied actor
	Action.GetObjectVars(objVars,"Destination");
	for (idx = 0; idx < objVars.Length && destActor == None; idx++)
	{
		destActor = Actor(objVars[idx]);

		// If its a player variable, teleport to the Pawn not the Controller.
		C = Controller(destActor);
		if(C != None && C.Pawn != None)
		{
			destActor = C.Pawn;
		}
	}

	if (GearAI(Controller) != None)
	{
		`AILog_Ext(self $ "::" $ GetFuncName() @ "teleporting to" @ destActor,, GearAI(Controller));
	}

	// and set to that actor's location
	if (destActor != None && SetLocation(destActor.Location))
	{
		Mesh.SetRBPosition(destActor.Location);
		PlayTeleportEffect(false, true);
		if (Action.bUpdateRotation)
		{
			SetRotation(destActor.Rotation);
			Mesh.SetRBRotation(destActor.Rotation);
		}
	}
	else
	{
		`warn("Unable to teleport to"@destActor);
	}

	if( Controller != None && Driver == self )
	{
		Controller.OnTeleport( None );
	}
}

/** Called for cars to see if they can boost. */
simulated event bool CanBoost()
{
	return FALSE;
}

/** Called to force the vehicle to boost. */
simulated event bool ForceBoost()
{
	return FALSE;
}

simulated event bool ShouldDoTankSteer()
{
	return FALSE;
}

/** Called to force vehicle-space cam to center (back to vehicle forward) */
simulated function bool ShouldCenterCamSpaceCamera()
{
	return FALSE;
}

/** Return the rotation of the vehicle used as basis for vehicle-space camera */
simulated function rotator GetVehicleSpaceCamRotation(float DeltaTime, bool bPassenger)
{
	return Rotation;
}

/** Called when a passenger presses X */
simulated function PassengerPressedX(Controller C);

/** Trigger a muzzle light */
simulated function TriggerVehicleMuzzleLight(PointLightComponent Template, Name MuzzleSocketName, FLOAT FadeTime)
{
	// See if the light we currently have is based on the supplied template
	if(Template == MuzzleLightInfo.TemplateMuzzleLight)
	{
		// If so, just reset brightness (no need to create new component)
		MuzzleLightInfo.MuzzleLight.SetLightProperties(Template.Brightness);
	}
	else
	{
		// Check if we already have a component- kill if so
		if(MuzzleLightInfo.MuzzleLight != None)
		{
			MuzzleLightInfo.MuzzleLight.SetEnabled(FALSE);
			MuzzleLightInfo.MuzzleLight.DetachFromAny();
			MuzzleLightInfo.MuzzleLight = None;
		}

		// Create new component, based on template
		MuzzleLightInfo.MuzzleLight = new(self) class'PointLightComponent' (Template);
		MuzzleLightInfo.TemplateMuzzleLight = Template;
	}

	// If we have a light, attach it and save params to fade it out
	if (MuzzleLightInfo.MuzzleLight != None)
	{
		Mesh.AttachComponentToSocket(MuzzleLightInfo.MuzzleLight, MuzzleSocketName);
		MuzzleLightInfo.MuzzleLight.SetEnabled(TRUE);
		MuzzleLightInfo.LightFadeTime = FadeTime;
		MuzzleLightInfo.LightTimeRemaining = FadeTime;
		MuzzleLightInfo.LightInitialBrightness = Template.Brightness;
	}
}

/** Overridden to provide PerDamageTypeModifier fuctionality. */
function TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int Index;

	if (! WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.GRI.IsCoopMultiplayerGame())
	{
		if ( (AIController(Controller) != None) && (GearAI_TDM(Controller) == None) )
		{
			Index = PerDamageTypeModifiers.Find('DamageTypeName', DamageType.Name);
			if (Index != INDEX_NONE)
			{
				Damage *= PerDamageTypeModifiers[Index].Multiplier;
			}
		}
	}

	super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
}

simulated function OnSetPawnBaseHealth(SeqAct_SetPawnBaseHealth Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		Health = Action.NewHealth;
		HealthMax = FMax(Health,HealthMax);
	}
}

/** This will update the shadow settings for this pawn's mesh **/
simulated event UpdateShadowSettings( bool bInWantShadow )
{
	local bool bNewCastShadow;
	local bool bNewCastDynamicShadow;

	if( Mesh != None )
	{
		bNewCastShadow = default.Mesh.CastShadow && bInWantShadow;
		bNewCastDynamicShadow = default.Mesh.bCastDynamicShadow && bInWantShadow;

		if( (bNewCastShadow != Mesh.CastShadow) || (bNewCastDynamicShadow != Mesh.bCastDynamicShadow) )
		{
			// if there is a pending Attach then this will set the shadow immediately as the flags have changed an a reattached has occurred
			Mesh.CastShadow = bNewCastShadow;
			Mesh.bCastDynamicShadow = bNewCastDynamicShadow;

			ReattachMesh();
		}
	}
}

simulated function bool ShouldShowWeaponOnHUD(GearPC PC)
{
	return bShowWeaponOnHUD;
}

/** Called when the driver speaks a line */
simulated event DriverSpeaking();

defaultproperties
{

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		LightShadowMode=LightShadow_Modulate
	End Object
	LightEnvironment=MyLightEnvironment
	Components.Add(MyLightEnvironment)

	Begin Object Name=SVehicleMesh
		CastShadow=true
		bCastDynamicShadow=true
		LightEnvironment=MyLightEnvironment
		bOverrideAttachmentOwnerVisibility=true
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object

	Begin Object Name=CollisionCylinder
		BlockNonZeroExtent=false
		BlockZeroExtent=false
		BlockActors=false
		BlockRigidBody=false
		CollideActors=false
	End Object

	InventoryManagerClass=class'GearInventoryManager'
	bEjectKilledBodies=true
	LinkHealMult=0.35
	VehicleLostTime=0.0
	MaxDesireability=0.5
	bEjectPassengersWhenFlipped=true
	MinRunOverSpeed=250.0
	MinCrushSpeed=100.0
	LookForwardDist=0.0
	MomentumMult=2.0
	bNoZDampingInAir=true

	LookSteerSensitivity=2.0
	ConsoleSteerScale=1.5
	ConsoleThrottleScale=1.5

	ReverseIsForwardThreshold=-1.0

	WheelLongGripScale=1.0
	WheelLatGripScale=1.0
	PeelOutFrictionScale=1.0

	bCanFlip=false
	RanOverDamageType=class'GDT_Explosive'
	CrushedDamageType=class'GDT_Explosive'
	MinRunOverWarningAim=0.88
	ObjectiveGetOutDist=1000.0
	//ExplosionTemplate=ParticleSystem'FX_VehicleExplosions.Effects.P_FX_GeneralExplosion'
	//BigExplosionTemplates[0]=(Template=ParticleSystem'FX_VehicleExplosions.Effects.P_FX_VehicleDeathExplosion')
	//SecondaryExplosion=ParticleSystem'Envy_Effects.VH_Deaths.P_VH_Death_Dust_Secondary'

	bDoActorLocationToCamStartTrace=TRUE
	SeatCameraScale=1.0

	SpawnRadius=320.0
	//BurnOutTime=2.5
	DeadVehicleLifeSpan=9.0
	//BurnTimeParameterName=BurnTime
	//VehicleDrowningDamType=class'UTGame.UTDmgType_Drowned'

	BaseEyeheight=30
	Eyeheight=30

	//DrivingAnim=Manta_Idle_Sitting
	bShouldAutoCenterViewPitch=TRUE

	bDrawHealthOnHUD=FALSE

	CollisionDamageMult=0.002
	CameraLag=0.12
	ViewPitchMin=-15000
	MinCameraDistSq=1.0

	bNoZSmoothing=true
	CameraSmoothingFactor=2.0

	RespawnTime=30.0
	InitialSpawnDelay=+0.0

	ExplosionDamage=100.0
	ExplosionRadius=300.0
	ExplosionMomentum=60000
	ExplosionInAirAngVel=1.5
	//ExplosionLightClass=class'UTGame.UTTankShellExplosionLight'
	//MaxExplosionLightDistance=+4000.0
	ExplosionDamageType=class'GDT_Explosive'
	//VehiclePieceClass=class'UTGib_VehiclePiece'

	bAllowedExit=true

	//InteractAction={(
	//	ActionName=DriveVehicle,
	//	ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
	//	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=132,V=421,UL=149,VL=90)))	),
	//	)}

	//LargeChunkImpactSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleImpact_MetalLargeCue'
	//MediumChunkImpactSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleImpact_MetalMediumCue'
	//SmallChunkImpactSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleImpact_MetalSmallCue'
	//ImpactHitSound=SoundCue'A_Weapon_ImpactHammer.ImpactHammer.A_Weapon_ImpactHammer_FireImpactVehicle_Cue'

	bPushedByEncroachers=false
	bAlwaysRelevant=true
	TurretScaleControlName=TurretScale
	TurretSocketName=VH_Death
	TurretOffset=(X=0.0,Y=0.0,Z=200.0);
	//DistanceTurretExplosionTemplates[0]=(Template=ParticleSystem'Envy_Effects.VH_Deaths.P_VH_Death_SpecialCase_1_Base_Near',MinDistance=1500.0)
	//DistanceTurretExplosionTemplates[1]=(Template=ParticleSystem'Envy_Effects.VH_Deaths.P_VH_Death_SpecialCase_1_Base_Far',MinDistance=0.0)
	TurretExplosiveForce=10000.0f

	//VehicleSounds(0)=(SoundStartTag=DamageSmoke,SoundEndTag=NoDamageSmoke,SoundTemplate=SoundCue'A_Vehicle_Generic.Vehicle.Vehicle_Damage_FireLoop_Cue')

	DestroyOnPenetrationThreshold=50.0
	DestroyOnPenetrationDuration=1.0

	bFindGroundExit=true

	MaxWheelEffectDistSq=16000000.0
	WaterEffectType=Water

	PathSearchType=PST_Constraint

	bShowWeaponOnHUD=TRUE

	CameraNoRenderCylinder_High_ViewTarget=(Radius=40,Height=86)
	CameraNoRenderCylinder_Low_ViewTarget=(Radius=40,Height=36)
	CameraNoRenderCylinder_High=(Radius=40,Height=88)
	CameraNoRenderCylinder_Low=(Radius=40,Height=44)
	CameraNoRenderCylinder_FlickerBuffer=(Radius=4,Height=4)
	bBlockCamera=TRUE

	HordeEnemyIndex=INDEX_NONE
}
