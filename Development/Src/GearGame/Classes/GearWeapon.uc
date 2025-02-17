/**
 * GearWeapon
 * Gear Weapon implementation
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeapon extends Weapon
	abstract
	config(Weapon)
	native(Weapon)
	nativereplication
	dependson(GearHUD_Base, GearPC, GearSkelCtrl_Recoil, GearTypes);

/** GoW global macros */

/** cached reference to GearAI controller */
var transient GearAI GearAIController;

/** If TRUE do not allow pickup factories to respawn if the weapon already exists in the level */
var config bool bUniqueSpawn;

/** Constant weapon ID, mainly used for FX lookup */
var const EWeaponClass WeaponID;
var const GearPerMapColorConfig GlobalDefaultColors;

/** Whether or not this weapon can be thrown  (e.g. GearWeap_Melee can't be thrown and that call chain is expensive 1+ ms expensive **/
var bool bCanThrowActiveWeapon;

var const config float Range_Melee;
/** if this is non-zero we will roll the dice when inside this range and maybe charge our enemy and melee (useful for chainsaws! )*/
var const config float Range_MeleeCharge;
var const config float Range_Short;
var const config float Range_Medium;
var const config float Range_Long;

/** FOV to acquire melee targets */
var const config Vector2D MeleeAcquireDotFOV;

/** Damage scalar based on range.  We interpolate these multipliers for actual ranges that lie between the defined ranges above. */
var config float DamageMod_MeleeRange; // point blank
var config float DamageMod_ShortRange;
var config float DamageMod_MediumRange;
var config float DamageMod_LongRange;

var config float Base_AccScale_BlindFire;

/** If TRUE, damage done by weapon is not modified by difficulty level (always use 'Normal' value) */
var	bool	bIgnoreDifficultyDamageScale;

/** AI Accuracy cone - by range */
var(AI)	config Vector2d AI_AccCone_Min,
						AI_AccCone_Max;
var(AI) config	float	AI_RateOfFire_Scale;
/** AI burst fire counts */
var(AI) config Vector2D AI_BurstFireCount;
var(AI) config Vector2D AI_BurstFireDelay;
/** number of shots left in this burst **/
var transient int RemainingBurstFireCount;
/** number of bursts we should fire this volley -1 indicates fire until out of ammo **/
var transient int RemainingBurstsToFire;

/** AI targeted shot delay */
var(AI) config Vector2D AI_AimDelay;

/** Variables for AI using hose weapons */
var(AI) config  Vector2D	AI_HoseInterpTimeRange;
var	transient	float		AI_HoseInterpSetTime, AI_HoseInterpFinishTime;
var transient	Rotator		AI_HoseDesiredOffset, AI_PreviousHoseOffset;


/** temp enum to define anim set to use on character */
enum EWeaponAnimType
{
	EWAT_AssaultRifle,
	EWAT_Shotgun,
	EWAT_SniperRifle,
	EWAT_Pistol
};
var const EWeaponAnimType	WeaponAnimType;

//
// Cheats
//

var()	bool	bInfiniteSpareAmmo;


//
// General
//

/** Force pawn to walk when targeting? */
var() const bool			bForceWalkWhenTargeting;

/** True if this weapon cannot target, false otherwise. */
var() const protected	bool	bPreventTargeting;
/** TRUE if reloading weapon takes out of TargetingMode, TRUE for most weapons. */
var() const				bool	bReloadingWeaponPreventsTargeting;

/** FOV to use in while in targeting mode with this weapon.	 <= zero means the weapon will not modify the FOV */
var()	config	float	TargetingFOV;

/** Adjustment to apply to fov when player wields this weapon in an "aiming" pose */
var()	config	float	AimingFOVAdjustment;

/** Is this weapon a slave weapon (used by remote clients) */
var				bool	bSlaveWeapon;
/** Weapon actually has alt fire instead of targeting */
var				bool	bAltFireWeapon;

/** Can weapon be fired in blind fire mode? */
var const		bool	bBlindFirable;
/** Setting this to FALSE, will prevent default blind firing animations from being played. */
var	const		bool	bPlayDefaultBlindFireStance;
/** TRUE if supports 360 aiming in cover animations. (Back to wall aiming ang firing). */
var				bool	bSupports360AimingInCover;
/** TRUE if we can RoadieRun using this weapon. */
var				bool	bAllowsRoadieRunning;
/** Character is allowed to play idle stances. (non firing, rest pose) */
var				bool	bAllowIdleStance;
/** Character is allowed to play aiming stances. (Firing from hips) */
var				bool	bAllowAimingStance;
/** Character is allowed to play downsights animations. (targeting mode) */
var				bool	bAllowDownsightsStance;

/** if TRUE, animation transitions won't delay weapon firing. */
var				bool	bNoAnimDelayFiring;
/**
 * if TRUE, disables Left Hand IK controller locked to IK Bones on skeleton.
 * Weapons like pistol and grenade do not want that.
 */
var				bool	bDisableLeftHandIK;
/** If TRUE, Pawn will play IK Recoil. */
var				bool	bPlayIKRecoil;
/**	If TRUE, meleeing a FSM with this weapons will cause fracture. */
var				bool	bAllowMeleeToFracture;

/** Animation node used to play custom animations */
var	transient	GearAnim_Slot	CustomAnimNode;
/** Pointer to AnimTree RootNode. */
var	transient	AnimTree		AnimTreeRootNode;

/** Animation to play on weapon */
var() protected Name	WeaponFireAnim;
var() protected Name	WeaponReloadAnim;
var() protected Name	WeaponReloadAnimFail;
var() protected Name	WeaponReloadAnimSuccess;

/** Screen Shake to play when firing weapon */
var() config	ScreenShakeStruct	FireCameraShake;
var() config	bool				bPlayFireCameraShake;
/** Scalar used to amplify/dampen firing CamShakes. */
var protected float					FireCameraShakeScale;

/** The controller vibration waveform for when firing this weapon **/
var ForceFeedbackWaveform WeaponFireWaveForm;

/** TRUE if camera needs to be super-steady during targeting */
var		bool	bSuperSteadyCamWhileTargeting;

/** Used By IsEffectRelevant as the cull distance for whether to spawn this effect or not **/
var config float ImpactRelevanceDistance;
/** Used by SpawnImpactDecal to determine min distance for spawning decals on skeletal meshes */
var config float DecalImpactRelevanceDistanceSkelMesh;

/** Shell case ejection particle system */
var()				ParticleSystemComponent		PSC_ShellEject;
/** After fire smoke effect */
var()				ParticleSystemComponent		PSC_ReloadBarrelSmoke;

// Fire modes

// Fire mode 0 is the default weapon firing.
const DEFAULT_FIREMODE			= 0;
/** arbitrary firemode for reloading */
const RELOAD_FIREMODE			= 1;
/** Firing mode used when doing a melee attack */
const MELEE_ATTACK_FIREMODE		= 2;
/** For weapons that have an alt fire instead of targeting */
const ALTFIRE_FIREMODE			= 3;
/** Shotgun Cock special fire mode */
const SHOTGUN_COCK_FIREMODE		= 4;
/** All purpose charging firing mode */
const FIREMODE_CHARGE			= 5;
/** All purpose failed firing mode */
const FIREMODE_FAILED			= 6;

/** Special Grenade fire modes */
const FIREMODE_GRENADESPIN = 7;
const FIREMODE_STOPGRENADESPIN = 8;

/** Active Reload fire modes */
const FIREMODE_FAILEDACTIVERELOAD		= 9;
const FIREMODE_ACTIVERELOADSUCCESS		= 10;
const FIREMODE_ACTIVERELOADSUPERSUCCESS	= 11;

/** Keep track of last firing mode used */
var	byte	PreviousFiringMode;

/** Weapon parameter definition */
struct native TWeaponParam
{
	var()	Array<float>	Value;
	var		Name			Name;
};

var() const config	float			WeaponDamage, WeaponDamageAI;
var() config	float				WeaponMagSize;
var() const config	float			WeaponMaxSpareAmmo;
var() const config	float			WeaponRecoil;
var() const config	vector2d		WeaponAimError;
var() const config	float			WeaponRateOfFire;

/** When blindfiring over cover, limit player aiming when using this weapon inside UpdateMeshBoneControllersNative to this pitch */
var() const protected int			MinBlindFireUpAimPitch;

//
// Weapon Recoil
//

/** Weapon Recoil Offset applied to player view. */
var				Rotator	WeaponRecoilOffset;
/** interpolation speed for weapon recoil. */
var()	config 	float	RecoilInterpSpeed;

/** Auto correcting weapon recoil applied to player view, diminishes over time */
var				Rotator	AutoCorrectWeaponRecoilOffset;
/** Percentage of recoil to auto correct */
var()	config	float	RecoilAutoCorrectPct;
/** Rotation units per second that auto correct recoil will converge on zero */
var()	config	float	RecoilAutoCorrectSpeed;
/** Delay after recoil is done to start auto correction */
var()	config	float	RecoilAutoCorrectDelay;
/** World time to start auto correction */
var				float	RecoilAutoCorrectTime;

//
// Ammo
//

/** Struct for storing ammo display */
struct native AmmoDrawData
{
	var int			DisplayCount;
	var CanvasIcon	AmmoIcon;
	var float		ULPerAmmo;
};
/** Ammo icon data */
var AmmoDrawData	HUDDrawData;
var AmmoDrawData	HUDDrawDataSuper;

/** Whether this weapon can be actively reloaded and thus can display a reload tutorial */
var bool bCanDisplayReloadTutorial;

/** This is the amount of ammo (min/max) that will be in the weapon if it is dropped from a foe**/
var config vector2d AmmoFromDrop;
/** class that should be acquired when picking up this weapon after being dropped
 * used to make enemy-only weapon classes become their player-wieldable counterpart on drop
 * if None, this object's Class is used
 */
var class<GearWeapon> DroppedWeaponClass;

/** Ammo used from current magazine */
var	repnotify int		AmmoUsedCount;
/** When reaching this amount, ammo becomes critical (warnings, flashing, beeping, whatever..). */
var()	config		int		CriticalAmmoCount;
/** Spare ammo, contained in extra magazines (outside of what's currently in the weapon) */
var	repnotify		int		SpareAmmoCount;
/** Number of magazines to start with */
var() 		config	int		InitialMagazines;
/** TRUE if weapon can be reloaded */
var() 				bool	bWeaponCanBeReloaded;

/** duration in seconds of reload sequence */
var		config		float	ReloadDuration;
/** Time this weapon last started a reload */
var 	transient	float	ReloadStartTime;


//
// Muzzle Flash
//

/** Muzzle socket name, to attach effects on */
var()	Name					MuzzleSocketName;
/** Muzzle flash staticmesh, attached to weapon mesh */
var()	MeshComponent			MuzzleFlashMesh;
/** dynamic light */
var()	PointLightComponent		MuzzleFlashLight;
/** Time in seconds the muzzle flashes */
var()				float		MuzzleLightDuration;

var()				bool		bLoopMuzzleFlashLight;
var()				float		MuzzleLightPulseFreq;
var()				float		MuzzleLightPulseExp;
var()				float		MuzzleLightPulseTime;

/** offset light is positioned from cannon */
var()				vector		MuzzleLightOffsetFromCannon;
/** Muzzle Flash particle system */
var()	editinline	ParticleSystemComponent		MuzFlashEmitter;
/** Affects how we turn it on/off. */
var()				bool						bMuzFlashEmitterIsLooping;

/** Time after muzzle flash was deactivated to hide particle system */
var					float						TimeToHideMuzzleFlashPS;

/** Muzzle Flash particle system for normal shooting */
var()	editinline	ParticleSystem		MuzFlashParticleSystem;
/** Muzzle Flash particle system for ActiveReload */
var()	editinline	ParticleSystem		MuzFlashParticleSystemActiveReload;

var()	editinline	ParticleSystem		MuzSmokeParticleSystem;


//
// Player animations
//

/**
 * Weapon Specific animation sets.
 * Animations in these will override the default assault rifle set.
 */
var()	Array<AnimSet>		CustomAnimSets;
/** Name of AimOffset profiles to use. Set in order. */
var	const Array<Name>		AimOffsetProfileNames;

var()	GearPawn.BodyStance	HolsterShoulderLeft;
var()	GearPawn.BodyStance	HolsterShoulderRight;
var()	GearPawn.BodyStance	EquipShoulderLeft;
var()	GearPawn.BodyStance	EquipShoulderRight;

var()	GearPawn.BodyStance	BS_PawnWeaponReload;
var()	GearPawn.BodyStance	BS_PawnWeaponReloadSuccess;
var()	GearPawn.BodyStance	BS_PawnWeaponReloadFail;

//
// Sounds
//

/** Fire sound to use when this weapon is fired by a local player.  Stereo, non-spatialized. */
var()			SoundCue	FireSound_Player;
/** Fire sound to use when this weapon is fired by everyone else.  Mono and spatialized. */
var()			SoundCue	FireSound;

/** Weapon melee hit sound*/
var()			SoundCue	MeleeImpactSound;

/** Weapon fire sound to play when firing and this weapon has no ammo */
var() const		SoundCue	FireNoAmmoSound;
/** Time between FireNoAmmoSound instances if trigger is held.  0.f for no repeating sound. */
var() const protected float		NoAmmoFireSoundDelay;

/**
* sound that is played when the bullets go whizzing past your head
*
* @see GearPC.CheckNearMiss
**/
var() const				SoundCue		WeaponWhipSound;
var() const				float			MinTimeBetweenBulletWhips;
var() const				float			BulletWhipChance;

/** Sound played when reloading weapon */
var() const protected	SoundCue		WeaponReloadSound;

/** Sound played when the player equips the weapon. (e.g. pulling it from his back or holster) **/
var() const protected	SoundCue		WeaponEquipSound;

/** Sound played when the player equips this weapon.  (e.g. stowing it away on his back or in his holster) **/
var() const protected	SoundCue		WeaponDeEquipSound;

/** Sound played when the player drops this weapon. **/
var() const				SoundCue		WeaponDropSound;

//
// HUD
//

/** Scale that defines how much the crosshair expands when being inaccurate */
var()		float						CrosshairExpandStrength;


//
// Inventory
//

enum EWeaponType
{
	WT_Normal,			// Carried on Shoulders
	WT_Holster,			// Only fits in holster slot
	WT_Item,			// Only fits in Item slot
	WT_Heavy,			// Can be carried only in hands, cannot be holstered
};

/** Type of weapon, will affect how it is managed in the inventory */
var()	EWeaponType			WeaponType;
/** Slot the weapon is attached to on the character */
var		GearPawn.EAttachSlot	CharacterSlot;

/** This is the ammo class that this weapon can get ammo from **/
var class<GearAmmoType> AmmoTypeClass;


//// Active Reload

/** this is our "mutex" that says whether or not we have attempted an AR on this specific reload **/
var		bool   bActiveReloadAttempted;

/** time the reload was started **/
var		float	AR_TimeReloadButtonWasPressed;

/** The time in which AR magic bullets will time out and become normal bullets again **/
var	config float	AR_MagicBulletsTimeoutDuration;

/**
 * Number of seconds before the AR_PossibleSuccessStartPoint which when the reload button
 * is pressed will result in a failed AR.
 **/
var()	config float	AR_PreReactionWindowDuration;

/**
 * Number of seconds AFTER hitting reload to start the active reload minigame
 * if -1 then we are just doing to use ReloadDuration / 2
 **/
var()	config float	AR_PossibleSuccessStartPoint;

/** Duration after the AR_PossibleSuccessStartPoint which the SuperSweetSpot is valid **/
var()	config float	AR_SuperSweetSpotDuration;

/**
 * Duration after the AR_PossibleSuccessStartPoint and SuperSweetSpot
 * which the SuperSweetSpot is valid
 **/
var()	config float	AR_SweetSpotDuration;


/** Whether or not the visual tracer effects are active **/
var bool bActiveReloadTracerVisualEffectsActive;


/** How many seconds after a successful active reload the bonus should last **/
//var() array<float> ActiveReload_BonusDurationTier;
/** Active reload damage percentage bonus for normal damage. X=Min bonus, Y=Max bonus */
var() config vector2d ActiveReload_DamageMultiplier;
/** Active reload damage multipler for explosive/radial damage.	X=Min bonus, Y=Max bonus */
var() config vector2d ActiveReload_ExploRadiusMultiplier;
/** Active reload rate-of-fire multipler. X=Min bonus, Y=Max bonus */
var() config vector2d ActiveReload_RateOfFireMultiplier;

/**
 * number of AR bullets you have in your gun currently
 **/
var repnotify int ActiveReload_NumBonusShots;

/** tmp var for how many shots this weapon has taken during a bonus active reload time period **/
var transient int ActiveReload_NumShotsTaken;

/** This holds the AR super sweet spot duration per tier for each weapon**/
const ACTIVE_RELOAD_TIERS = 3;
var config TWeaponParam ActiveReload_SuperSweetSpotDurations;

/**
 * If the weapon has a special Active Reload damage type this will be set.
 * @see SniperRifle for an example
 **/
var class<DamageType> ActiveReloadDamageType;

/** Audio for Active Reloads. */
var() protected const SoundCue AR_FailSound;
var() protected const SoundCue AR_SuccessSound;
var() protected const SoundCue AR_SuperSuccessSound;


/** This is how much stronger the last bullet shot is in SP only.  Makes it so you run out of ammo and last bullet KILLS the guy!!!!**/
var config float LastBulletStrongerPercent;

/** Max distance allow for friction */
var() config float MaxFrictionDistance;
/** Peak distance for friction */
var() config float PeakFrictionDistance;
/** Min distance for friction */
var() config float MinFrictionDistance;
/** Min/Max friction multiplier applied when target acquired */
var() config Vector2d FrictionMultiplierRange;
/** Amount of additional radius/height given to target cylinder when at peak distance */
var() config float PeakFrictionRadiusScale;
var() config float PeakFrictionHeightScale;
/** Offset to apply to friction target location (aim for the chest, etc) */
var() config vector FrictionTargetOffset;
/** Max time to attempt adhesion to the friction target */
var() config float MaxAdhesionTime;
/** Max distance to allow adhesion to still kick in */
var() config float MaxAdhesionDistance;
/** Max distance from edge of cylinder for adhesion to be valid */
var() config float MaxAdhesionAimDistY;
var() config float MaxAdhesionAimDistZ;
/** Min/Max amount to scale for adhesive purposes */
var() config Vector2d AdhesionScaleRange;
/** Min amount to scale for adhesive purposes */
var() config float MinAdhesionScaleAmount;
/** Require the target to be moving for adhesion to kick in? */
var() config float MinAdhesionTargetVelocity;
/** Require the player to be moving for adhesion to kick in? */
var() config float MinAdhesionPlayerVelocity;

/** If TRUE, call GetWeaponDistanceForEnemyTrace function on the weapon to get distance to do trace. */
var bool bOverrideDistanceForEnemyTrace;

/** Should use timer instead of anim notifies for melee hits? */
var bool bUseMeleeHitTimer;
/** This is the time that must pass between melee attacks **/
var config float MeleeAttackCoolDownInSeconds;

/** This locks us from responding to anim notifies that come through when we should not be able to attack **/
var bool bMeleeImpactNotifySemaphore;

/** Time (after melee starts) before traces for impact start occuring */
var config float MeleeInitialImpactDelay;
/** Time (after impact checks start) to retry impacts */
var config float MeleeImpactRetryDuration;
/** World time to stop retrying traces for melee impact */
var		   float MeleeImpactCompleteTime;

/** Melee Attack body stance for this weapon */
var()	array<BodyStance>	BS_MeleeAttack;

/** Name of camera anim to play on melee attack, if one is desired */
var()	array<CameraBoneAnimation>	MeleeAttackCameraBoneAnims;

/** For runtime decisions re whether to play melee attach camera anims. */
var transient bool					bSuppressMeleeAttackCameraBoneAnim;

/** The range at which this attack can be used **/
var config float MeleeAttackRange;

/** Extent to use when tracing melee attacks */
var vector MeleeTraceExtent;

/** The min amount of damage the melee attack does **/
var config float MeleeAttackDamageMin;

/** The max amount of damage the melee attack does **/
var config float MeleeAttackDamageMax;

/** True if this weapons melee attack should include some adhesion in the controller */
var() bool		bDoMeleeAdhesion;

/** This is the camera shake that we play when we hit a melee attack **/
var() ScreenShakeStruct MeleeImpactCamShake;

/** True to do camera and controller shakes on melee impact, false to skip them. */
var config bool bDoMeleeDamageShakes;

/** stores the wall being hit in a melee attack, so multiple sounds/effects aren't triggered.*/
var transient bool bHitWallThisAttack;

/**
 * This stores how many times we have attempted to attack with the melee attack.  We are doing a
 * a timed attack so you end up getting "Attacks" on the "return to idle" which is odd when you are
 * triggering a wall impact with it. so we will only trigger on the first k (3-4) and that will be the
 * down swing.
 **/
var transient int NumMeleeAttackAttempts;

/** The time stamp for when we started the melee special attack **/
var float TimeStartedSpecialMeleeAttack;

/** Weapon icon displayed on HUD */
var CanvasIcon	WeaponIcon;
var int			IconXOffset;
var int			IconYOffset;

/** Weapon icon displayed on HUD for Annex Command Point */
var CanvasIcon	AnnexWeaponIcon;

/** Crosshair icon */
var Texture2D	CrosshairIcon;
var Texture2D	CrosshairIconSecondary;

/** should the AI treat this weapon as a suppressive weapon? */
var bool		bIsSuppressive;
/** is this a long range sniping weapon? */
var bool		bSniping;
/** AI flag - can this weapon ignore (or completely destroy in one shot) a meat shield?
 * (used to know whether kidnapper can be engaged head on)
 */
var bool bCanNegateMeatShield;
/** AI flag indicating whether this weapon can kill DBNO pawns even when execution rules are enabled */
var bool bIgnoresExecutionRules;
/** is a hose type weapon - flame thrower, mulcher, etc */
var bool		bHose;

/** true if instigator's bWeaponDebug_Accuracy is true (set when firing) */
var bool		bDebuggingShots;

/** Start location of the last shot I fired.  Used for debug displays in multiplayer */
var repnotify vector DebugShotStartLoc;
/** Aim direction of the last shot I fired.	 Used for debug displays in multiplayer */
var rotator			 DebugShotAimRot;

/** Per-weapon scale factor for amount of aim assistance to apply. */
var() float AimAssistScale;
/** Same as AimAssistScale, except this is used while in targeting/zoom mode. */
var() float AimAssistScaleWhileTargeting;

/** Determines which tracer type to use for this weapon **/
var const protected EWeaponTracerType	TracerType;
/** These are the Distances for when to show tracers **/
var const protected config float		ShowTracerDistance;
/** Controls size of the tracers for this weapon */
var() const protected vector			TracerScale;
/** Effects to use for new tracers.	 Prototype for now, may replace old tracer system later. */
var() const protected ParticleSystem	TracerSmokeTrailEffect;
/** Effects to use for new tracers for active-reload bonus shots.  Prototype for now, may replace old tracer system later. */
var() const protected ParticleSystem	TracerSmokeTrailEffectAR;
/** Percentage chance that any given tracer will have a smoke trail. */
var() const protected config float		TracerSmokeTrailFrequency;

/** Animation Physics Recoil */
var() GearSkelCtrl_Recoil.RecoilDef	Recoil_Hand;
var() GearSkelCtrl_Recoil.RecoilDef	Recoil_Spine;

/** Can this weapon be selected when it is out of ammo? */
var const bool bCanSelectWithoutAmmo;

/** When this Weapon runs out of ammo, attempt auto-switch to another weapon. */
var protected const bool bAutoSwitchWhenOutOfAmmo;


/** true if this weapon is being temporarily holstered, false otherwise */
var transient bool bTemporaryHolster;

/** Are we allowed to swap this weapon? */
var bool bSwappable;
/** Are we forbidden to interact with anything while holding this weapon? */
var bool bNoInteractionWhileEquipped;

/** Whether or not muzzle flashes CastDynamicShadows **/
var bool bDynamicMuzzleFlashes;

/** Sound to play when your current clip is nearly empty, as a subtle reminder. */
var protected const SoundCue	NeedReloadNotifySound;
/** Play notify sound when this many rounds remain in the clip */
var() protected int				NeedReloadNotifyThreshold;

/** true if this weapon supports zoomed-in targeting, false otherwise */
var() bool		bUseTargetingCamera;

/** Keep track of where the weapon is attached to. To prevent multiple attachments to the same socket. */
var transient	Name	SocketAttachedTo;

/** Magazine Mesh */
var	StaticMeshComponent	MagazineMesh;

/** TRUE to suppress muzzle flash effects */
var bool bSuppressMuzzleFlash;
/** TRUE to suppress tracers for this weapon */
var bool bSuppressTracers;
/** TRUE to suppress impact effects for this weapon */
var bool bSuppressImpactFX;
/** TRUE to suppress all weapon audio */
var bool bSuppressAudio;
/** TRUE to suppress damage */
var bool bSuppressDamage;

/** TRUE if this is a weapon meant for dummyfire use. */
var protected bool bDummyFireWeapon;
/** Actor this weapon is attached to, if it's a dummyfire weapon (ignored otherwise) */
var protected Actor DummyFireParent;
/** Location of dummyfire target */
var protected Vector DummyFireTargetLoc;
/** Actor that we were dummy firing at. */
var protected Actor DummyFireTargetActor;

/**
  * This is the MIC which will be the weapons's "material" once it is spawned.	We will then
  * be able to modify the colors on the fly!
 **/
var transient MaterialInstanceConstant MIC_WeaponSkin;

/**
 * This weapon's default emissive color.  The issue is that we can not have a struct of structs
 * and have the composed struct get the correct defaultproperties.	So we need to have some pain
 * here where we check for the uninited LinearColor and then init it the real defaults.
 * Find RonP or MartinS for the details on this.  We are going to fix thise post Gears as it
 * is a behavior affecting change.
**/
var LinearColor LC_EmisDefaultCOG;
var LinearColor LC_EmisDefaultLocust;

/** This controls how much force is applied to the weapons when they spin/flyout as they are dropped **/
var config float WeaponDropAngularVelocity;

/** this weapon aims using the camera direction as determined BEFORE modifiers, such as camera shake or bone motion */
var() bool bUsePreModCameraRotForAiming;

/** Can this weapon be equipped while holding a shield? */
var() const bool bCanEquipWithShield;

/** True to spawn tracers on each shot, false for no tracrs. */
var() protected const bool bAllowTracers;

/** Damage type to use when sending death/pickup messages in the UI */
var class<GearDamageType> DamageTypeClassForUI;

/** activates client side hit detection for instant hit fire */
var config bool bClientSideInstantHit;
/** multiplier to hitbox size of an actor the client said it hit that the shot must be within for the server to agree */
var float ClientSideHitLeeway;

/** whether this weapon can parry melee attacks from other weapons' melee */
var bool bCanParryMelee;

// CQC Executions
var Name					CQC_Long_KillerAnim;
var Name					CQC_Long_VictimAnim;
var Array<CameraAnim>		CQC_Long_CameraAnims;
var bool					CQC_Long_bHolsterWeapon;
var float					CQC_Long_VictimDeathTime;
/** 0.f for instant */
var float					CQC_Long_VictimRotStartTime;
/** 0.f for instant */
var float					CQC_Long_VictimRotInterpSpeed;
var	class<GearDamageType>	CQC_Long_DamageType;
var GearVoiceEffortID		CQC_Long_EffortID;


/** Animation to play on Pawn for quick executions. */
var	Name					CQC_Quick_KillerAnim;
var	FLOAT					CQC_Quick_VictimDeathTime;
var GearVoiceEffortID		CQC_Quick_EffortID;
var	class<GearDamageType>	CQC_Quick_DamageType;

// if this is on GetPhysicalFireStartLoc will return GetMuzzleLoc
var bool					bUseMuzzleLocForPhysicalFireStartLoc;

/** TRUE if this weapon should do the barrel-heat functionality. */
var protected const bool			bSupportsBarrelHeat;
/** How hot the barrel is, from 0 to 1. */
var protected transient float		CurrentBarrelHeat;
var protected transient float		LastBarrelHeat;
var protected transient float		GoalBarrelHeat;
var protected transient float		MaxBarrelHeat;
var() protected const float			BarrelHeatInterpSpeed;
var() protected const float			BarrelHeatPerShot;
/** Time it takes for barrel to cool down from heat=1 to heat=0. */
var() protected const float			BarrelHeatCooldownTime;
var protected const Name			BarrelHeatMaterialParameterName;

/** @STATS - holds the stat index for the player who currently owns this weapon so we don't have to look it up each time a
    stat is sent. */
var int WeaponStatIndex;

/** Stats Mask Defines */
const STATS_LEVEL8	= 0x80;

cpptext
{
	// Networking
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual void TickSpecial( FLOAT DeltaSeconds );
}


simulated native function GetMuzzleLocAndRot(out vector Loc, out rotator Rot);

simulated native event vector GetPhysicalFireStartLoc(optional vector AimDir);

/**
* This function returns the world location for spawning the visual effects.
* Network: ALL
*/
simulated native event Vector GetMuzzleLoc();

//
// Network replication
//

replication
{
	// Things the server should send to local player.
	if ( bNetInitial && bNetOwner )
		InitialMagazines, WeaponMagSize, WeaponMaxSpareAmmo;

	if( bNetOwner )
		AmmoUsedCount, SpareAmmoCount, CharacterSlot,
		bInfiniteSpareAmmo, ActiveReload_NumBonusShots;

	if( bDebuggingShots && Role==ROLE_Authority )
		DebugShotStartLoc, DebugShotAimRot;
}


/** Check on various replicated data and act accordingly. */
simulated event ReplicatedEvent( name VarName )
{
	Super.ReplicatedEvent( VarName );

	Switch( VarName )
	{
		case 'DebugShotStartLoc' : DrawDebugShot(DebugShotStartLoc, DebugShotAimRot); break;
		case 'SpareAmmoCount' : AmmoUpdated(); break;
		case 'AmmoUsedCount' : AmmoUpdated(); break;
	}
}


/**
 * Called immediately before gameplay begins.
 */
event PreBeginPlay()
{
	super.PreBeginPlay();
	InitializeAmmo(); // setup starting ammo
}

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if( Mesh != none )
	{
		MIC_WeaponSkin = Mesh.CreateAndSetMaterialInstanceConstant(0);
	}

	// get the color values from the archetype
	if( MuzzleFlashLight != none )
	{
		MuzzleFlashLight.SetLightProperties( default.MuzzleFlashLight.Brightness, GetWeaponMuzzleFlashActiveReload() );
	}
}

simulated function ToggleCharging(bool bNewCharging);

/**
 * Fires a projectile.
 * Spawns the projectile, but also increment the flash count for remote client effects.
 * Network: Local Player and Server
 */
simulated function Projectile ProjectileFire()
{
	local Vector		ProjLoc, ProjDir;
	local Projectile	SpawnedProjectile;

	// tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	if( WorldInfo.Netmode != NM_Client )
	{
		GetProjectileFirePosition(ProjLoc, ProjDir);

		// Spawn projectile
		SpawnedProjectile = Spawn(GetProjectileClass(), Self,, ProjLoc);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( ProjDir );
			GearProjectile(SpawnedProjectile).SuppressAudio(bSuppressAudio);
		}

		// Return it up the line
		return SpawnedProjectile;
	}

	return None;
}


/**
 * Get Projectile Spawn Location and Direction.
 */
simulated function GetProjectileFirePosition(out vector out_ProjLoc, out vector out_ProjDir)
{
	local vector		StartTrace, EndTrace;
	local ImpactInfo	TestImpact;

	// This is where we would start an instant trace. (what CalcWeaponFire uses)
	StartTrace	= Instigator.GetWeaponStartTraceLocation();
	out_ProjDir	= Vector(GetAdjustedAim( StartTrace ));

	// this is the location where the projectile is spawned.
	out_ProjLoc	= GetPhysicalFireStartLoc(out_ProjDir);

	if (StartTrace != out_ProjLoc && Instigator != None && Instigator.IsHumanControlled())
	{
		// if projectile is spawned at different location of crosshair,
		// then simulate an instant trace where crosshair is aiming at, Get hit info.
		EndTrace	= StartTrace + out_ProjDir * GetTraceRange();
		TestImpact	= CalcWeaponFire(StartTrace, EndTrace);

		// make sure that can hit what StartTrace hit
		if( !FastTrace(TestImpact.HitLocation, out_ProjLoc) )
		{
			out_ProjLoc = StartTrace;
		}

		// Then we realign projectile aim direction to match where the crosshair did hit.
		out_ProjDir	= Normal(TestImpact.HitLocation - out_ProjLoc);
	}
}

/** Allow the weapon to do any special tweaks to the trace start location. */
simulated function vector AdjustStartTraceLocation(vector StartLoc, vector AimDir)
{
	return StartLoc;
}



/**
 * This is to allow the weapon to handle (eat) button presses.
 *
 * @see state Reload.HandleButtonPress for our ActiveReload functionality
 **/
simulated function bool HandleButtonPress( coerce Name ButtonName )
{
	return FALSE;
}

/**
 * This is to allow the weapon to handle (eat) button releases.
 **/
simulated function bool HandleButtonRelease( coerce Name ButtonName )
{
	return FALSE;
}

/** Whether the input passed in is asking for a zoom in or out */
simulated function bool InputCanZoom( coerce Name ButtonName )
{
	if ( ((ButtonName == 'ZoomIn') && !GearPawn(Instigator).bIsZoomed) ||
		 ((ButtonName == 'ZoomOut') && GearPawn(Instigator).bIsZoomed) ||
		 (ButtonName == 'R3') )
	{
		return true;
	}

	return false;
}

final simulated function SetZoomed(bool bNewZoomed, optional bool bToggleOwnerNoSee)
{
	GearPawn(Instigator).bIsZoomed = bNewZoomed;
	if (bToggleOwnerNoSee)
	{
		SetOwnerNoSee(GearPawn(Instigator).bIsZoomed);
	}
	if (WorldInfo.NetMode == NM_Client)
	{
		ServerSetZoomed(bNewZoomed);
	}
}

final reliable server function ServerSetZoomed(bool bNewZoomed)
{
	GearPawn(Instigator).bIsZoomed = bNewZoomed;
}

/**
 * Hide or unhide the owner of this weapon
 * Pawn is hidden while rifle is zoomed
 */
simulated function SetOwnerNoSee(bool bNewOwnerSee)
{
	local GearPawn_Infantry HelmetedInstigator;

	if ( Mesh != None )
	{
		Mesh.SetOwnerNoSee(bNewOwnerSee);
		if ( (Instigator != None) && (Instigator.Mesh != None) )
		{
			Instigator.Mesh.SetOwnerNoSee(bNewOwnerSee);
		}
		HelmetedInstigator = GearPawn_Infantry(Instigator);
		if ( HelmetedInstigator != None )
		{
			if( HelmetedInstigator.HeadSlotStaticMesh != none )
			{
				HelmetedInstigator.HeadSlotStaticMesh.SetOwnerNoSee(bNewOwnerSee);
			}

			if( HelmetedInstigator.HeadSlotStaticMesh2 != none )
			{
				HelmetedInstigator.HeadSlotStaticMesh2.SetOwnerNoSee(bNewOwnerSee);
			}
		}
	}

	// toggling this prevents an optimization where IK and stuff is turned off after a second
	// causing camera pops a second or so after the zoom (since camera is based on a bone)
	Instigator.Mesh.bIgnoreControllersWhenNotRendered = !bNewOwnerSee;
}

/**
 * @returns true if Weapon would like a first person camera to be used
 */
simulated function bool UseFirstPersonCamera()
{
	return false;
}

/**
 * Notification from pawn that it has been unpossessed.
 */
function NotifyUnpossessed();

/**
 * This function returns the damage for the current firing mode.
 * @note: to apply modifiers on base damage, prefer using ModifyDamage(). See below.
 */
simulated function float GetFireModeDamage( Vector HitLocation, optional Actor HitActor )
{
	local float ModifiedDamage;
	local GearAI AI;

	// Get base fire mode damage
	if (Instigator != None)
	{
		AI = GearAI(Instigator.Controller);
	}
	if (AI != None && GearAI_TDM(AI) == None) // allow MP bots to use player rules
	{
		ModifiedDamage = WeaponDamageAI;
		// double damage for AI during coop splits
		if (GearGame(WorldInfo.Game).bInCoopSplit &&
			GearAI_COGGear(AI) != None &&
			AI.Squad != None &&
			PlayerController(AI.GetSquadLeader()) == None)
		{
			ModifiedDamage *= 2.f;
		}
	}
	else
	{
		ModifiedDamage = WeaponDamage;
	}

	// Apply modifiers
	ModifyDamage( ModifiedDamage, HitLocation, HitActor );
	return ModifiedDamage;
}


/**
 * Apply modifiers to current firemode damage.
 * it checks to see if it should modify the damage for a shot
 * based on whether or not the ActiveReload minigame was completed successfully.
 */
simulated function ModifyDamage( out float BaseDamage, Vector HitLocation, optional Actor HitActor )
{
	local float Range;
	local GearPawn GP;
	local Actor ActorIter;
	local GearPRI PRI;

	// dummy fire weapons don't damage what they are attached to
	if ( (HitActor != None) && bDummyFireWeapon && (DummyFireParent != None) )
	{
		// search the attachment chain to see if we've hit something we should ignore
		ActorIter = DummyFireParent;
		do
		{
			if (ActorIter == HitActor)
			{
				BaseDamage = 0.f;
				return;
			}
			ActorIter = ActorIter.Base;
		} until (ActorIter == None);
	}

	if (Instigator != None)
	{
		Range = VSize(HitLocation - Instigator.Location);
	}
	else
	{
		// no instigator, measure shot from weapon itself
		Range = VSize(HitLocation - Location);
	}

	// Adjust damage for range.
	if( Range <= Range_Melee )
	{
		BaseDamage *= DamageMod_MeleeRange;
	}
	else if( Range <= Range_Short )
	{
		BaseDamage *= GetRangeValueByPct( vect2d(DamageMod_MeleeRange,DamageMod_ShortRange), ((Range-Range_Melee)/(Range_Short-Range_Melee)) );
	}
	else if( Range <= Range_Medium )
	{
		BaseDamage *= GetRangeValueByPct( vect2d(DamageMod_ShortRange,DamageMod_MediumRange),  ((Range-Range_Short)/(Range_Medium-Range_Short)) );
	}
	else if (Range <= Range_Long)
	{
		BaseDamage *= GetRangeValueByPct( vect2d(DamageMod_MediumRange,DamageMod_LongRange), ((Range-Range_Medium)/(Range_Long-Range_Medium)) );
	}
	else
	{
		BaseDamage *= DamageMod_LongRange;
	}

	if (Instigator != None)
	{
		// only humans get AR and last bullet bonus
		if( Instigator.IsHumanControlled() || GearAI_TDM(Instigator.Controller) != None)
		{
			BaseDamage *= GetActiveReloadDamageMultiplier();

			// if not a MP game
			if (!WorldInfo.GRI.IsMultiPlayerGame() || GearGRI(WorldInfo.GRI).IsCoopMultiplayerGame())
			{
				// check for last bullet bonus
				if ((GetMagazineSize() - AmmoUsedCount) == 0 && Range <= 512.f)
				{
					BaseDamage *= (1.0f + LastBulletStrongerPercent);
				}
				// also check for difficulty scaling
				PRI = GearPRI(Instigator.PlayerReplicationInfo);
				if (PRI != None)
				{
					if(bIgnoreDifficultyDamageScale)
					{
						BaseDamage *= class'DifficultySettings_Normal'.default.PlayerDamageMod;
					}
					else
					{
						BaseDamage *= PRI.Difficulty.default.PlayerDamageMod;
					}
				}
			}
		}
		// if we're a COG then use the friendly damage modifier
		else
		{
			PRI = GearPRI(Instigator.PlayerReplicationInfo);
			if (PRI != None && PRI.Team != None && PRI.Team.TeamIndex == 0)
			{
				BaseDamage *= PRI.Difficulty.default.FriendDamageMod;
			}
		}
	}

	// Multiply the super damage from the warpawn to the weapon.
	// This is initially done so that we can make AI buddies blow through levels on splits.
	GP = GearPawn(Instigator);
	if( GP != None )
	{
		BaseDamage *= GP.SuperDamageMultiplier;
	}
}


/**
 * Returns active-reload damage multiplier for this weapon.  Can be affected by active reload
 * (or anything else for that matter).
 **/
simulated function float GetActiveReloadDamageMultiplier()
{
	local float Retval;
	local GearPawn GP;

	GP = GearPawn(Instigator);

	// if we are in a AR bonus active state
	if( (GP != None) && GP.bActiveReloadBonusActive )
	{
		Retval = RandRange( ActiveReload_DamageMultiplier.X, ActiveReload_DamageMultiplier.Y );
	}
	else
	{
		Retval = 1.0f;
	}

	return Retval;
}

simulated function float GetExploRadiusMultiplier()
{
	local float Retval;
	local GearPawn GP;

	GP = GearPawn(Instigator);

	// if we are in a AR bonus active state
	if( (GP != None)  && GP.bActiveReloadBonusActive )
	{
		Retval = RandRange( ActiveReload_ExploRadiusMultiplier.X, ActiveReload_ExploRadiusMultiplier.Y );
	}
	else
	{
		Retval = 1.0f;
	}

	return Retval;

}


/** returns the size of the weapon's magazine */
simulated final function float GetMagazineSize()
{
	return WeaponMagSize;
}

/** Sets unlimited magazine size (= disables reloads) */
simulated final function SetInfiniteMagazineSize()
{
	WeaponMagSize = 0;
}

/** returns max amount of spare ammo */
simulated final function float GetMaxSpareAmmoSize()
{
	return WeaponMaxSpareAmmo;
}

/** Sets unlimited amount of spare ammo (= never runs out of ammo, but has to reload weapon */
simulated final function SetInfiniteSpareAmmo()
{
	bInfiniteSpareAmmo = TRUE;
}

/** Returns TRUE is weapon has unlimited spare ammo. */
simulated function bool HasInfiniteSpareAmmo()
{
	return bInfiniteSpareAmmo;
}

/** @return whether our remaining ammo is low (so draw red on HUD, etc) */
simulated function bool IsCriticalAmmoCount()
{
	return (SpareAmmoCount == 0);
}

/** returns rate of fire of weapon*/
simulated function float GetRateOfFire()
{
	local float RetVal;
	local GearPawn GP;

	RetVal = WeaponRateOfFire;

	if (Instigator != None)
	{
		if (Instigator.IsHumanControlled() || GearAI_TDM(Instigator.Controller) != None)
		{
			// check for active reload bonus (note AI doesn't get AR Bonuses)
			GP = GearPawn(Instigator);

			// if we are in a AR bonus active state
			if( (GP != None) && GP.bActiveReloadBonusActive )
			{
				RetVal *= RandRange( ActiveReload_RateOfFireMultiplier.X, ActiveReload_RateOfFireMultiplier.Y );
			}
		}
		else
		{
			// scale for AI
			RetVal *= AI_RateOfFire_Scale;
		}
	}

	return RetVal;
}


/************************************************************************************
 * Aim / View
 ***********************************************************************************/

/**
 * Called before firing weapon to adjust Trace/Projectile aiming
 * State scoped function. Override in proper state
 * Network: Server
 *
 * @param	StartFireLoc,	world location of weapon start trace or projectile spawning
 */

simulated function Rotator GetAdjustedAim( vector StartFireLoc )
{
	local Rotator	AimRot;
	local float		AimError;

	AimRot	= Instigator.GetAdjustedAimFor( Self, StartFireLoc );

	// If human player...
	// (Adjusted aim for AI handled by the controller)
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

	return AimRot;
}


/** Returns player aim error */
simulated function float GetPlayerAimError()
{
	return Lerp( WeaponAimError.X, WeaponAimError.Y, GearInventoryManager(InvManager).GetPlayerNormalizedAccuracy() );
}

/** returns accuracy cone for AI */
function GetAIAccuracyCone(out vector2D AccCone_Min, out vector2D AccCone_Max)
{
	AccCone_Min = AI_AccCone_Min;
	AccCone_Max = AI_AccCone_Max;
}

/**
 * Set Weapon Recoil effect, which modifies the player view rotation
 *
 * @param	PitchRecoil, Pitch offset added to player view rotation
 */
simulated protected function SetWeaponRecoil( int PitchRecoil )
{
	local int YawRecoil;

	YawRecoil = (0.5f - FRand()) * PitchRecoil;

	WeaponRecoilOffset.Pitch	+= PitchRecoil;
	WeaponRecoilOffset.Yaw		+= YawRecoil;
}

simulated function int GetWeaponRecoil()
{
	return WeaponRecoil;
}

/** @return the base rotation the AI should use for firing this weapon (before inaccuracy modifiers) */
function rotator GetBaseAIAimRotation(vector StartFireLoc, vector AimLoc)
{
	return rotator(AimLoc - StartFireLoc);
}

simulated function Rotator GetHumanAimRotationAdjustment();

/**
 * Called from PlayerController::UpdateRotation() -> PlayerController::ProcessViewRotation() ->
 * Pawn::ProcessViewRotation() -> GearInventoryManager::::ProcessViewRotation() to (pre)process player ViewRotation.
 * adds delta rot (player input), applies any limits and post-processing
 * returns the final ViewRotation set on PlayerController
 *
 * @param	DeltaTime, time since last frame
 * @param	ViewRotation, actual PlayerController view rotation
 * @input	out_DeltaRot, delta rotation to be applied on ViewRotation. Represents player's input.
 * @return	processed ViewRotation to be set on PlayerController.
 */

simulated function ProcessViewRotation( float DeltaTime, out rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	local Rotator	DeltaRecoil;
	local float		DeltaPitch, DeltaYaw;
	local GearPC	PC;

	// Add Weapon recoil offset to view rotation
	if( WeaponRecoilOffset != Rot(0,0,0) )
	{
		// add recoil offset smoothly
		DeltaRecoil.Pitch	= WeaponRecoilOffset.Pitch	- FInterpTo( WeaponRecoilOffset.Pitch,	0, DeltaTime, RecoilInterpSpeed );
		DeltaRecoil.Yaw		= WeaponRecoilOffset.Yaw	- FInterpTo( WeaponRecoilOffset.Yaw,	0, DeltaTime, RecoilInterpSpeed );

		WeaponRecoilOffset	-= DeltaRecoil;
		PC = GearPC(Instigator.Controller);
		if( PC != None )
		{
			DeltaRecoil = Normalize(PC.LimitViewRotation( out_ViewRotation + DeltaRecoil, Instigator.ViewPitchMin, Instigator.ViewPitchMax ) - out_ViewRotation);
		}
		out_DeltaRot		+= DeltaRecoil;

		if( DeltaRecoil == rot(0,0,0) )
		{
			WeaponRecoilOffset = rot(0,0,0);
		}

		AutoCorrectWeaponRecoilOffset += DeltaRecoil;
		RecoilAutoCorrectTime = WorldInfo.TimeSeconds + RecoilAutoCorrectDelay;
	}
	else
	if( AutoCorrectWeaponRecoilOffset != rot(0,0,0) &&
		WorldInfo.TimeSeconds > RecoilAutoCorrectTime )
	{
		DeltaPitch	= AutoCorrectWeaponRecoilOffset.Pitch	- FInterpTo( AutoCorrectWeaponRecoilOffset.Pitch,	0, DeltaTime, RecoilAutoCorrectSpeed );
		DeltaYaw	= AutoCorrectWeaponRecoilOffset.Yaw		- FInterpTo( AutoCorrectWeaponRecoilOffset.Yaw,		0, DeltaTime, RecoilAutoCorrectSpeed );

		out_DeltaRot.Pitch -= DeltaPitch * RecoilAutoCorrectPct;
		out_DeltaRot.Yaw   -= DeltaYaw   * RecoilAutoCorrectPct;
		AutoCorrectWeaponRecoilOffset.Pitch -= DeltaPitch;
		AutoCorrectWeaponRecoilOffset.Yaw   -= DeltaYaw;

		//`log( "Delta Pitch:"@out_DeltaRot.Pitch@"Delta Yaw"@out_DeltaRot.Yaw@"Sub"@DeltaPitch@DeltaYaw );

		if( Abs(DeltaPitch) <= 1.f && Abs(DeltaYaw) <= 1.f )
		{
			AutoCorrectWeaponRecoilOffset = rot(0,0,0);
		}
	}
}


/**
 *	Should Targeting Mode trigger the Zoom camera mode?
 */
simulated function bool ShouldTargetingModeZoomCamera()
{
	local GearPawn P;

	P = GearPawn(Instigator);
	if( P == None )
	{
		return FALSE;
	}

	return (bUseTargetingCamera && !IsReloading() && P.bIsTargeting);
}


/** Notification that Targeting Mode has changed. */
simulated function TargetingModeChanged(GearPawn P);

/** Return TRUE to prevent targeting */
simulated function bool ShouldPreventTargeting()
{
	return bPreventTargeting;
}

/** Return TRUE to force targeting */
simulated function bool ShouldForceTargeting()
{
	return FALSE;
}

/** Notification called when Pawn.bDoing360Aiming flag changes. */
simulated function On360AimingChangeNotify();


/*********************************************************************************************
 * Ammunition
 *********************************************************************************************/


/**
 * Initializes ammo counts, when weapon is spawned.
 */
function InitializeAmmo()
{
	AddAmmo(InitialMagazines * GetMagazineSize());
}

/**
 * @see Weapon::ConsumeAmmo
 * function also made simulated to ammo consumption is done instantly on client, before waiting for replication.
 * this fixes auto reloads not working on a local machine, but lag could had induced other bugs.
 * ammo is still replicated, so if it gets out of synch, client would still be corrected.
 */
simulated function ConsumeAmmo( byte FireModeNum )
{
	local GearPawn GP;

	if ( WorldInfo.Netmode != NM_Client )
	{
		AmmoUsedCount++;
	}

	if( ActiveReload_NumBonusShots <= 0 )
	{
		// and disable the bonus if we've run out of magic bullets
		GP = GearPawn(Instigator);
		if (GP != None && GP.bActiveReloadBonusActive)
		{
			TurnOffActiveReloadBonus();
			TurnOffActiveReloadBonus_VisualEffects();
		}
	}
	else
	{
		// consume bonus shot
		--ActiveReload_NumBonusShots;
	}

	// This is the "you are about to run out of ammo" sound better reload! (clickclick)
	if( ( (GetMagazineSize() - AmmoUsedCount) < NeedReloadNotifyThreshold ) && !HasInfiniteSpareAmmo() )
	{
		GearWeaponPlaySoundLocal(NeedReloadNotifySound,, TRUE);
	}
}

simulated function int GetDefaultAmmoAmount()
{
	return GetMagazineSize() * (1 + default.InitialMagazines);
}

simulated function int GetPickupAmmoAmount()
{
	return (GetMagazineSize() - AmmoUsedCount) + SpareAmmoCount;
}

simulated function int CopyAmmoAmountFromWeapon( GearWeapon InvWeapon )
{
	AmmoUsedCount	= InvWeapon.AmmoUsedCount;
	SpareAmmoCount	= InvWeapon.SpareAmmoCount;

	return SpareAmmoCount + (GetMagazineSize() - AmmoUsedCount);
}

/**
 *	Checks if weapon has room to pickup ANY ammo
 *	@return		TRUE if ammo will be accepted, FALSE otherwise
 */
simulated function bool CanPickupAmmo()
{
	if( CharacterSlot != EASlot_None &&
		GetMaxSpareAmmoSize() > 0 &&
		SpareAmmoCount < GetMaxSpareAmmoSize() )
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * Add ammo to weapon
 * @param	Amount to add.
 * @return	Amount actually added. (In case magazine is already full and some ammo is left
 */

function int AddAmmo( int Amount )
{
	local int	OldAmount, AmountAdded, MagazineSize, AmmoInCurrentMag, AmmoToReload;

	// If we can't accept spare ammo, then abort
	if( GetMaxSpareAmmoSize() <= 0 )
	{
		return 0;
	}

	MagazineSize		= GetMagazineSize();
	AmmoInCurrentMag	= MagazineSize - AmmoUsedCount;
	AmmoToReload		= AmmoUsedCount;
	OldAmount			= AmmoInCurrentMag + SpareAmmoCount;
	SpareAmmoCount		= SpareAmmoCount + Amount;

	// Take into account ammo that will be reloaded, so we cap to MaxSpareAmmoSize + AmmoToReload
	SpareAmmoCount		= Min(SpareAmmoCount, GetMaxSpareAmmoSize() + AmmoToReload);
	AmountAdded			= (SpareAmmoCount + AmmoInCurrentMag) - OldAmount;
	bForceNetUpdate		= TRUE;

	AmmoUpdated();

	return AmountAdded;
}

/** Notification called when ammo count has been updated. */
simulated function AmmoUpdated()
{
	// check to see if we should reload here as we may have had no ammo
	// in the active weapon and just picked up some ammo
	if( PendingFire(RELOAD_FIREMODE) || ShouldAutoReload() )
	{
		ForceReload();
	}
}

/** Send a message to the controller that ammo has been added */
function AddAmmoMessage( int AmountAdded )
{
	local GearPawn GP;
	local GearPC PC;

	GP = GearPawn(Instigator);
	if (GP != None)
	{
		// tell the hud to draw the ammo addition
		PC = GearPC(GP.Controller);
		if ( PC != None )
		{
			PC.ClientAddAmmoMessage(DamageTypeClassForUI, AmountAdded);
		}
	}
}


/** Returns true if spare ammo is available (outside of current magazine) */
simulated function bool HasSpareAmmo()
{
	return( HasInfiniteSpareAmmo() ||	// unlimited extra ammo
			SpareAmmoCount > 0 );		// spare ammo left
}


/** @see Weapon::HasAmmo */
simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
	local float MagazineSize;

	// we can always reload our weapon or do a melee attack
	if( FireModeNum == RELOAD_FIREMODE || FireModeNum == MELEE_ATTACK_FIREMODE )
	{
		return TRUE;
	}

	// Assume we ask for at least 1 bullet to be there.
	if( Amount == 0 )
	{
		Amount = 1;
	}
	MagazineSize = GetMagazineSize();

	// If we have fired more than Magazine capacity, then we need to reload
	if( MagazineSize > 0 &&
		AmmoUsedCount + Amount > MagazineSize )
	{
		return FALSE;
	}

	return TRUE;
}


/** Has weapon any ammo left? Regardless of firing mode and spare magazines. */
simulated function bool HasAnyAmmo()
{
	// we have just a single fire mode. So just check for this one.
	return (HasAmmo(0) || HasSpareAmmo());
}

function bool DenyPickupQuery(class<Inventory> ItemClass, Actor Pickup)
{
	// always accept pickups. GearDroppedPickup is responsible for adding inventory, or giving ammo instead.
	return FALSE;
}

/** Deprecated, do not use this one, see below. */
simulated function PlayWeaponAnimation( Name Sequence, float fDesiredDuration, optional bool bLoop, optional SkeletalMeshComponent SkelMesh);


/** Returns the AnimNodeSequence the weapon is using to play animations. */
simulated function AnimNodeSequence GetWeaponAnimNodeSeq()
{
	//`DLog("CustomAnimNode:" @ CustomAnimNode);
	if( CustomAnimNode != None )
	{
		//`DLog("SeqNode:" @ CustomAnimNode.GetCustomAnimNodeSeq());
		return CustomAnimNode.GetCustomAnimNodeSeq();
	}

	return Super.GetWeaponAnimNodeSeq();
}


/**
 * Play an animation on the weapon mesh.
 *
 * @param	AnimName		Name of animation to play.
 * @param	Rate			Rate animation should be played at.
 * @param	BlendInTime		Blend duration to play anim.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at AnimDuration - BlendOutTime seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop)
 * @param	bOverride		play same animation over again only if bOverride is set to true.
 *
 * @return	PlayBack length of animation.
 */
simulated function float PlayWeaponAnim
(
				name	AnimName,
				float	Rate,
	optional	float	BlendInTime,
	optional	float	BlendOutTime,
	optional	bool	bLooping,
	optional	bool	bOverride
)
{
	local AnimNodeSequence	AnimNode;

	if( CustomAnimNode != None )
	{
		return CustomAnimNode.PlayCustomAnim(AnimName, Rate, BlendInTime, BlendOutTime, bLooping, bOverride);
	}
	else
	{
		AnimNode = GetWeaponAnimNodeSeq();

		// Check if we can play animation
		if( AnimNode == None )
		{
			`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "GearWeapon::PlayWeaponAnim - no animations :( Trying to play:" @ AnimName);
			return 0.f;
		}

		// Set right animation sequence if needed
		if( AnimNode.AnimSeq == None || AnimNode.AnimSeq.SequenceName != AnimName )
		{
			AnimNode.SetAnim(AnimName);
		}

		if( AnimNode.AnimSeq == None )
		{
			`log("GearWeapon::PlayWeaponAnim - AnimSeq == None" @ Self @ "Instigator:" @ Instigator );
			return 0.f;
		}

		// Play Animation
		AnimNode.PlayAnim(bLooping, Rate);
		return AnimNode.GetAnimPlaybackLength();
	}
}


/**
 * Play an animation on the weapon mesh.
 *
 * @param	AnimName		Name of animation to play.
 * @param	Duration		duration in seconds the animation should be played.
 * @param	BlendInTime		Blend duration to play anim.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at AnimDuration - BlendOutTime seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop)
 * @param	bOverride		play same animation over again only if bOverride is set to true.
 */
simulated function PlayWeaponAnimByDuration
(
				name	AnimName,
				float	Duration,
	optional	float	BlendInTime,
	optional	float	BlendOutTime,
	optional	bool	bLooping,
	optional	bool	bOverride
)
{
	local SkeletalMeshComponent	SkelMesh;
	local float					Rate;

	if( CustomAnimNode != None )
	{
		CustomAnimNode.PlayCustomAnimByDuration(AnimName, Duration, BlendInTime, BlendOutTime, bLooping, bOverride);
	}
	else
	{
		SkelMesh = SkeletalMeshComponent(Mesh);

		// Check if we can play animation
		if( SkelMesh == None || AnimName == '' )
		{
			return;
		}

		Rate = SkelMesh.GetAnimRateByDuration(AnimName, Duration);
		PlayWeaponAnim(AnimName, Rate, BlendInTime, BlendOutTime, bLooping, bOverride);
	}
}


/**
 * Returns TRUE if weapon is playing a given animation.
 * @param	AnimName		Animation sequence name to test.
 * @param	bMustBeLooping	if animation is not looping, then FALSE will be returned.
 */
simulated function bool IsPlayingAnim(Name AnimName, bool bMustBeLooping)
{
	local AnimNodeSequence		AnimNode;

	if( CustomAnimNode != None )
	{
		return CustomAnimNode.bIsPlayingCustomAnim;
	}
	else
	{
		AnimNode = GetWeaponAnimNodeSeq();

		// Check if we can play animation
		if( AnimNode == None )
		{
			//`log( GetFuncName() @ "no mesh or no animations :(");
			return FALSE;
		}

		// test if animation is considered playing
		if( !AnimNode.bPlaying ||
			AnimNode.AnimSeq.SequenceName != AnimName ||
			bMustBeLooping && !AnimNode.bLooping )
		{
			return FALSE;
		}

		return TRUE;
	}
}


/** freeze weapon animation. */
simulated function FreezeWeaponAnim()
{
	local AnimNodeSequence		AnimNode;

	AnimNode = GetWeaponAnimNodeSeq();

	// Check if we have access to anim...
	if( AnimNode == None )
	{
		//`log( GetFuncName() @ "no animations :(");
		return;
	}

	AnimNode.bPlaying = FALSE;
}


/** Scale weapon animation play rate */
simulated function ScaleWeaponAnimPlayRate(float Scale)
{
	local AnimNodeSequence		AnimNode;

	AnimNode = GetWeaponAnimNodeSeq();

	// if there is no anim node we can't do anything
	if( AnimNode != None )
	{
		AnimNode.Rate *= Scale;
	}
}

/** Stop a weapon animation, and rewinds it */
simulated function StopWeaponAnim(optional float BlendOutTime)
{
	local AnimNodeSequence		AnimSeq;

	if( CustomAnimNode != None )
	{
		CustomAnimNode.StopCustomAnim(BlendOutTime);
	}
	else
	{
		AnimSeq = GetWeaponAnimNodeSeq();

		// if there is no skel mesh we can't do anything
		if( AnimSeq != None )
		{
			AnimSeq.StopAnim();
			AnimSeq.SetPosition(0.f, FALSE);
		}
	}
}


/** Set a new animation, overwriting current one */
simulated final function SetWeaponAnim(Name AnimName)
{
	local AnimNodeSequence	AnimSeq;

	AnimSeq = GetWeaponAnimNodeSeq();

	if( AnimSeq != None )
	{
		AnimSeq.SetAnim(AnimName);
	}
	else
	{
		`DLog("No weapon anim node seq!!");
	}
}


/** Stop a weapon animation, and rewinds it */
simulated final function SetWeaponAnimPosition(float NewPosition)
{
	local AnimNodeSequence		AnimSeq;

	AnimSeq = GetWeaponAnimNodeSeq();

	if( AnimSeq != None )
	{
		AnimSeq.SetPosition(NewPosition, FALSE);
	}
	else
	{
		`DLog("No weapon anim node seq!!");
	}
}


/** @see Weapon::GetFireInterval */
simulated function float GetFireInterval( byte FireModeNum )
{
	return 60.f / GetRateOfFire();
}


/************************************************************************************
 * Firing / Effects
 ***********************************************************************************/


/**
 * Event called when Pawn.FiringMode has been changed.
 * bViaReplication indicates if this was the result of a replication call.
 */
simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	local bool		bPrevIsReload, bNewIsReload;
	local GearPawn	GPOwner;
	local GearPC	PCOwner;

	// If melee is finished, send weapon to neutral state (remote clients)
	// and fire up MeleeAttackEnded() notification.
	if( bViaReplication && PreviousFiringMode == MELEE_ATTACK_FIREMODE )
	{
		MeleeAttackEnded();
	}

	// Handle reload aborts here.
	// If previous firing mode is for a reload
	bPrevIsReload = PreviousFiringMode == RELOAD_FIREMODE ||
					PreviousFiringMode == FIREMODE_FAILEDACTIVERELOAD ||
					PreviousFiringMode == FIREMODE_ACTIVERELOADSUCCESS ||
					PreviousFiringMode == FIREMODE_ACTIVERELOADSUPERSUCCESS;

	// If new firing mode is for a reload
	bNewIsReload = FiringMode == RELOAD_FIREMODE ||
					FiringMode == FIREMODE_FAILEDACTIVERELOAD ||
					FiringMode == FIREMODE_ACTIVERELOADSUCCESS ||
					FiringMode == FIREMODE_ACTIVERELOADSUPERSUCCESS;

	if( bPrevIsReload && !bNewIsReload )
	{
		// If weapon reload was aborted, then abort it...
		if( IsTimerActive('EndOfReloadTimer') )
		{
			AbortWeaponReload();
		}

		// Update Heavy weapon mouting status if needed
		PCOwner = GearPC(Instigator.Controller);
		if( PCOwner != None && PCOwner.IsLocalPlayerController() )
		{
			// First need to update Targeting status, to see if player is still targeting or not
			// This gets set to FALSE when reloading weapons
			PCOwner.UpdateTargetingStatus();

			// Then update heavy weapon status.
			GPOwner = GearPawn(Instigator);
			if( GPOwner != None )
			{
				GPOwner.CheckHeavyWeaponMounting();
			}
		}
	}

	// if we're reloading our weapon, play anim
	switch( FiringMode )
	{
		case RELOAD_FIREMODE					: PlayWeaponReloading();	break;
		case FIREMODE_FAILEDACTIVERELOAD		: PlayActiveReloadFailed(); break;
		case FIREMODE_ACTIVERELOADSUCCESS		: PlayActiveReloadSuccess(); break;
		case FIREMODE_ACTIVERELOADSUPERSUCCESS	: PlayActiveReloadSuperSuccess(); break;
		case MELEE_ATTACK_FIREMODE				: // For remote clients, fire up melee started notification
													if( bViaReplication )
													{
														MeleeAttackStarted();
													}
													break;
	}

	// Keep track of last firing mode
	PreviousFiringMode = FiringMode;
}


/**
 * Called by Pawn when FlashCount has been updated.
 * FlashCount > 0 denotes that weapon has fired.
 * FlashCount == 0 denotes that weapon stopped firing.
 * Network: ALL
 */
simulated function FlashCountUpdated( byte FlashCount, byte FiringMode, bool bViaReplication )
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "FlashCount:" @ FlashCount @ "bViaReplication:" @ bViaReplication );

	if( FlashCount > 0 )
	{
		WeaponFired( FiringMode );
	}
	else
	{
		WeaponStoppedFiring( FiringMode );
	}
}


/**
 * The whole FlashLocation usage in warfare...
 * FlashLocation is used to denote the location of an instant hit impact (or max range if an impact didn't occur).
 * It is replicated to all clients, including the owner of the weapon.
 *
 * Local Player
 * FlashLocation is only used to replicate the exact impact location of the hit.
 * The Weapon fire animations are played locally when the player initiated the firing sequence locally.
 *
 * Remote Clients
 * FlashLocation is used to replicate the exact impact location, but also initiate the firing sequence for remote clients.
 */

/** overridden to allow on client when bClientSideInstantHit is set */
simulated function SetFlashLocation( vector HitLocation )
{
	if (Instigator != None && (Role == ROLE_Authority || bClientSideInstantHit))
	{
		Instigator.SetFlashLocation( Self, CurrentFireMode, HitLocation );
	}
}

/**
 * Changed this function for warfare's weapon system.
 * WeaponFired() is called instantly on local clients, so there is no delay playing weapon firing effects.
 * remote clients do everything when FlashLocation is replicated.
 *
 * Network: LocalPlayer and Server.
 */
simulated function InstantFire()
{
	local GearPawn	PawnOwner;
	local vector			StartTrace, EndTrace;
	local Array<ImpactInfo>	ImpactList;
	local int				Idx;
	local ImpactInfo		RealImpact;

	PawnOwner = GearPawn(Instigator);

	// Servers proceed as normal
	if (WorldInfo.Netmode != NM_Client || (bClientSideInstantHit && Instigator != None && Instigator.IsLocallyControlled()))
	{
		// If InteractionPawn is Based on PawnOwner, then disable his collision when firing.
		// This is to prevent kidnapper from firing through the hostage
		if( PawnOwner != None && PawnOwner.InteractionPawn != None && PawnOwner.InteractionPawn.IsBasedOn(PawnOwner) )
		{
			PawnOwner.InteractionPawn.Mesh.SetTraceBlocking(FALSE, PawnOwner.InteractionPawn.Mesh.BlockNonZeroExtent);
		}

		// define range to use for CalcWeaponFire()
		StartTrace = Instigator.GetWeaponStartTraceLocation();
		EndTrace = StartTrace + vector(GetAdjustedAim(StartTrace)) * GetTraceRange();

		// Perform shot
		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);

		if (Role == ROLE_Authority || bClientSideInstantHit)
		{
			// Set flash location to trigger client side effects.
			// if HitActor == None, then HitLocation represents the end of the trace (maxrange)
			// Remote clients perform another trace to retrieve the remaining Hit Information (HitActor, HitNormal, HitInfo...)
			// Here, The final impact is replicated. More complex bullet physics (bounce, penetration...)
			// would probably have to run a full simulation on remote clients.
			SetFlashLocation(RealImpact.HitLocation);
		}

		// Process all Instant Hits on local player and server (gives damage, spawns any effects).
		for (Idx = 0; Idx < ImpactList.Length; Idx++)
		{
			ProcessInstantHit(CurrentFireMode, ImpactList[Idx]);
		}

		// Restore collision
		if( PawnOwner != None && PawnOwner.InteractionPawn != None && PawnOwner.InteractionPawn.IsBasedOn(PawnOwner) )
		{
			PawnOwner.InteractionPawn.Mesh.SetTraceBlocking(TRUE, PawnOwner.InteractionPawn.Mesh.BlockNonZeroExtent);
		}

`if(`notdefined(FINAL_RELEASE))
		if( PawnOwner != None )
		{
			bDebuggingShots = PawnOwner.bWeaponDebug_Accuracy;
			if( bDebuggingShots )
			{
				ClientFlushPersistentDebugLines();
				DebugShotStartLoc = Instigator.GetWeaponStartTraceLocation();
				DebugShotAimRot = GetAdjustedAim(DebugShotStartLoc);
				DrawDebugShot(DebugShotStartLoc, DebugShotAimRot);

				DrawDebugLine( DebugShotStartLoc+vect(0,0,5), DebugShotStartLoc + vector(DebugShotAimRot) * Range_Melee, 255, 1, 1, true );
				DrawDebugLine( DebugShotStartLoc+vect(0,0,5), DebugShotStartLoc + vector(DebugShotAimRot) * Range_Short, 255, 255, 1, true );
				DrawDebugLine( DebugShotStartLoc+vect(0,0,10), DebugShotStartLoc + vector(DebugShotAimRot) * Range_Medium, 1, 255, 1, true );
				DrawDebugLine( DebugShotStartLoc+vect(0,0,15), DebugShotStartLoc + vector(DebugShotAimRot) * Range_Long, 1, 1, 255, true );
			}
		}
`endif
	}
	else if( Instigator != None && Instigator.IsLocallyControlled() )
	{
		// local player simulates firing locally, to give instant response.
		// FlashLocation will only replicate the exact impact location, and play impact effects.
		WeaponFired( CurrentFireMode );
	}


}


/**
 * Changed this function for warfare's weapon system: It is now also called on LocalPlayer.
 * SetFlashLocation() is called only on servers, to replicate proper impact location.
 * However WeaponFired() is called instantly on local clients, so there is no delay playing weapon firing effects.
 * remote clients do everything when replicated.
 *
 * Network: LocalPlayer and Server.
 */

simulated function ClearFlashLocation()
{
	if( Instigator != None )
	{
		if( WorldInfo.Netmode != NM_Client )
		{
			Instigator.ClearFlashLocation( Self );
		}
		else if( Instigator.IsLocallyControlled() )
		{
			WeaponStoppedFiring( CurrentFireMode );
		}
	}
}


/**
 * Notification called when FlashLocation is updated.
 *
 * @param	FiringMode		that triggered the update.
 * @param	FlashLocation	replication location vector (typically used for the impact location)
 * @param	bViaReplication	if update has been triggered as a result of a replication call.
 */

simulated function FlashLocationUpdated( byte FiringMode, vector FlashLocation, bool bViaReplication )
{
	local bool		bValidFiringUpdate;

	// ignore FlashLocation replication if using client side hit location
	//@FIXME: should change Pawn to not send it
	if (bViaReplication && bClientSideInstantHit && Instigator != None && Instigator.IsLocallyControlled())
	{
		return;
	}

	// valid firing update (impact location) if FlashLocation is non zero.
	// Zero is used to denote a weapon stopping to fire.
	bValidFiringUpdate = !IsZero(FlashLocation);

	// we cannot post process any impact while reloading, FiringMode was lost.
	if( FiringMode == RELOAD_FIREMODE )
	{
		//`log( `location @ " FiringMode is same as reload firemode:" @ RELOAD_FIREMODE @ "Abort and don't trigger a firing update." @ self @ Instigator );
		bValidFiringUpdate = false;
	}

	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "FlashLocation:" $ FlashLocation @ "bViaReplication:" $ bViaReplication @ "bValidFiringUpdate:" $ bValidFiringUpdate @ "bSlaveWeapon:" $ bSlaveWeapon );

	// for server or remote clients, a FlashLocation update, triggers a weapon fire.
	// on the local client, the update is triggered instantly, not through replication.
	if (WorldInfo.Netmode != NM_Client || bClientSideInstantHit || bSlaveWeapon || WorldInfo.IsPlayingDemo())
	{
		if( bValidFiringUpdate )
		{
			WeaponFired( FiringMode, FlashLocation );
		}
		else
		{
			// stop the current fire mode in this case, rather than the passed in version
			WeaponStoppedFiring( CurrentFireMode /*FiringMode*/ );
		}
	}

	// if we've been replicated, then calculate remote impact effects.
	if( bValidFiringUpdate && bViaReplication )
	{
		CalcRemoteImpactEffects( FiringMode, FlashLocation, bViaReplication );
	}
}

/**
 * Called when the weapon has fired, to trigger any effects.
 * Network: ALL
 */
simulated event WeaponFired(byte FiringMode, optional vector HitLocation)
{
	local GearPawn GP;

	GP = GearPawn(Owner);

	//`log( `location @ `showvar(FiringMode) @ `showvar(HitLocation) );
	if (Instigator != None)
	{
		if (WorldInfo.NetMode != NM_Client)
		{
			//@STATS
			if(Instigator.PlayerReplicationInfo != none)
			{
				WeaponStatIndex = GearPRI(Instigator.PlayerReplicationInfo).AggWeaponFireStat(WeaponID,true, WeaponStatIndex);
			}
			`RecordStat(STATS_LEVEL8,'WeaponFired', Instigator.Controller, Class);
		}

		Instigator.WeaponFired(FALSE, HitLocation);
	}

	PlayFireEffects(FiringMode, HitLocation);

	if( GP != None )
	{
		// mark the last time the weapon was fired for AI use
		GP.LastWeaponStartFireTime = WorldInfo.TimeSeconds;

		// indicate we should draw the weapon indicator
		if ( GearPC(GP.Controller) != None )
		{
			GearPC(GP.Controller).SetLastWeaponInfoTime(WorldInfo.TimeSeconds);
		}
	}
//	`log( "ActiveReload_CurrTier: " $ GP.ActiveReload_CurrTier $ " SuperSweetSpotDuration: " $ ActiveReload_SuperSweetSpotDurations.Value[GP.ActiveReload_CurrTier] );

}


/**
 * Called when weapon stops firing.
 * @NOTE:	this is not a state scoped function (remote clients ignore the state the weapon is in)
 * Network: ALL
 *
 * @param	FireModeNum.
 */
simulated event WeaponStoppedFiring(byte FiringMode)
{
	if (Instigator != None)
	{
		Instigator.WeaponStoppedFiring(FALSE);
	}

	StopFireEffects(FiringMode);

	// turn off shell eject particle system
	if( PSC_ShellEject != None )
	{
		PSC_ShellEject.DeActivateSystem();
		SetTimer( 1.0f, FALSE, nameof(HidePSC_ShellEject) );
	}
}

/** this is used for client side hit detection */
reliable server function ServerNotifyHit(byte FiringMode, ImpactInfo Impact, bool bHitShield)
{
	local GearPawn GP;
	local Box HitBox;
	local vector BoxExtent, BoxCenter;

	// if HitActor didn't replicate it, try to get it from the hit component
	// this most commonly happens when hitting StaticMeshCollectionActors, as they are created by the cooker and so don't replicate
	if (Impact.HitActor == None && Impact.HitInfo.HitComponent != None)
	{
		Impact.HitActor = Impact.HitInfo.HitComponent.Owner;
	}

	// validate the shot
	// check that we're actually using this fire mode
	// and we're facing in that general direction
	if ( bClientSideInstantHit && Role == ROLE_Authority && Instigator != None && Impact.HitActor != None &&
		IsFiring() && CurrentFireMode == FiringMode &&
		vector(Instigator.GetViewRotation()) dot Normal(Impact.HitLocation - Instigator.GetWeaponStartTraceLocation()) > 0.7 ) // ~45 degrees
	{
		// assume it told the truth about static things because the hit doesn't have significant gameplay implications
		if (Impact.HitActor.bStatic)
		{
			ProcessInstantHit_Internal(FiringMode, Impact);
		}
		else
		{
			// and the target is really there
			Impact.HitActor.GetComponentsBoundingBox(HitBox);
			// increase both a bit because the hitlocation is usually on the edge of the cylinder
			// and because we want to give a reasonable leeway to the client's results
			BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
			BoxExtent *= ClientSideHitLeeway;
			// if we're in co-op, allow as long as it's in the ballpark
			if (GearGameSP_Base(WorldInfo.Game) != None)
			{
				BoxExtent *= 3.0;
			}
			BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;
			if ( Abs(Impact.HitLocation.Z - BoxCenter.Z) < BoxExtent.Z &&
				Abs(Impact.HitLocation.X - BoxCenter.X) < BoxExtent.X &&
				Abs(Impact.HitLocation.Y - BoxCenter.Y) < BoxExtent.Y )
			{
				if (bHitShield)
				{
					GP = GearPawn(Impact.HitActor);
					if (GP.EquippedShield != None && GP.EquippedShield.Mesh != None)
					{
						Impact.HitInfo.HitComponent = GP.EquippedShield.Mesh;
					}
				}
				ProcessInstantHit_Internal(FiringMode, Impact);
			}
		}
	}
}

/*
 * Process Instant Hit is called to process a Hit.
 * It can be called from InstantFire() (LocalPlayer and Server) or CalcRemoteImpactEffects() (remote clients).
 * Using that same path for server/local/remote is specific to warfare.
 */
simulated function ProcessInstantHit(byte FiringMode, ImpactInfo Impact)
{
	local GearPawn GP;

	if (bClientSideInstantHit && Instigator != None && Instigator.Controller != None)
	{
		if (Instigator.IsLocallyControlled())
		{
			if (WorldInfo.NetMode == NM_Client && Impact.HitActor != None && Impact.HitActor.Role < ROLE_Authority)
			{
				GP = GearPawn(Impact.HitActor);
				ServerNotifyHit(FiringMode, Impact, GP != None && GP.ComponentIsAnEquippedShield(Impact.HitInfo.HitComponent));
			}
		}
		else if ( Impact.HitActor != None
			&& ( Impact.HitActor.RemoteRole == ROLE_SimulatedProxy || Impact.HitActor.RemoteRole == ROLE_AutonomousProxy ||
				Impact.HitActor.bStatic || Impact.HitActor.bNoDelete ) )
		{
			// client will tell us if it hit this thing
			return;
		}
	}

	ProcessInstantHit_Internal(FiringMode, Impact);
}

simulated function bool ShouldRegisterDamage( Actor A )
{
	local GearPawn HitPawn;

	if( A != None && !bSuppressDamage )
	{
		if( WorldInfo.NetMode != NM_Client )
		{
			return TRUE;
		}

		HitPawn = GearPawn(A);
		if( HitPawn != None && HitPawn.bTearOff )
		{
			return TRUE;
		}

		// Acceptable client side hit actors
		if( FracturedStaticMeshActor(A) != None ||
			Hydra_Base(A)				!= None ||
			FluidSurfaceActor(A)		!= None ||
			CrowdAgent(A)				!= None )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** handles the effects of weapon firing. Separated out so that clientside hit detection can just call this part
 * for valid client hits
 */
simulated final private function ProcessInstantHit_Internal(byte FiringMode, const out ImpactInfo Impact)
{
	local Controller		CheckPlayer;
	local GearPawn			GP, HitPawn;
	local GearPC			GearPC;
	local class<DamageType> DamageTypeToUse;
	local GearAI			AI;

	GP = GearPawn(Instigator);
	HitPawn = GearPawn(Impact.HitActor);

	//debug
	`log( self@GetFuncName()@FiringMode@Impact.HitActor@Impact.HitInfo.HitComponent, bDebug );

	//`log(GetFuncName()@HitPawn@HitPawn.Health@HitPawn.bTearOff@GP);
	// substitute the active reload damage type if applicable

	if( ( (GP != None) && GP.bActiveReloadBonusActive ) && ( ActiveReloadDamageType != None ) )
	{
		DamageTypeToUse = ActiveReloadDamageType;
	}
	else
	{
		// otherwise use the default corresponding to our fire mode
		DamageTypeToUse = InstantHitDamageTypes[FiringMode];
	}

	// if we hit something on the server, then deal damage to it.
	// We also accept dead pawns (torn off) on clients for local impacts and rag doll fun
	if( ShouldRegisterDamage( Impact.HitActor ) )
	{
		Impact.HitActor.TakeDamage(	GetFireModeDamage( Impact.HitLocation, Impact.HitActor ),
									(Instigator != None ? Instigator.Controller : None),
									Impact.HitLocation,
									InstantHitMomentum[FiringMode] * (Instigator != None ? Normal(Impact.HitLocation - Instigator.Location) : Impact.RayDir), //Impact.RayDir, // @fixme: This is not correct. RayDir is from crosshair. should probably fix to be Normal(ImpactLoc - GunBarrel)
									DamageTypeToUse,
									Impact.HitInfo,
									self );
	}

	// Skip impacts for triggers and trigger volumes. We just want to forward damage to these, nothing else.
	if( Trigger(Impact.HitActor) != None ||
		TriggerVolume(Impact.HitActor) != None ||
		GrenadeBlockingVolume(Impact.HitActor) != None )
	{
		return;
	}

	// notify nearby players of a miss. This assumes currently one impact point. If we end up having several (reflection, penetration) we have to revise this.
	if( WorldInfo.Netmode != NM_Client )
	{
		// if we didn't hit a player
		if (Instigator != None && Instigator.Controller != None)
		{
			// notify any nearby players of the near miss
			foreach WorldInfo.AllControllers(class'Controller', CheckPlayer)
			{
				if( GearPawn(CheckPlayer.Pawn) == None || CheckPlayer.Pawn == Instigator || WorldInfo.GRI.OnSameTeam(Instigator, CheckPlayer.Pawn) )
				{
					continue;
				}

				GearPC = GearPC(CheckPlayer);
				if( GearPC != None )
				{
					GearPC.CheckNearMiss(Instigator, Self, Impact, DamageTypeToUse);
				}
				else
				{
					AI = GearAI(CheckPlayer);
					if( AI != None )
					{
						AI.CheckNearMiss(Instigator, Self, Impact, DamageTypeToUse);
					}
				}
			}
		}
	}

	// if a visual client, is relevant, hit something, and didn't hit a player (FX handled via TakeDamage->DoDamageEffects)
	if( WorldInfo.NetMode != NM_DedicatedServer &&
		Impact.HitActor != None &&
		( HitPawn == None || HitPawn.ComponentIsAnEquippedShield(Impact.HitInfo.HitComponent) )
		)
	{
		SpawnImpactFX(class<GearDamageType>(DamageTypeToUse),Impact,FALSE,FALSE,FALSE);
	}
}

/**
 * Kicks off sound/particles/decals based on an ImpactInfo struct, called on all visual clients either simulated locally or
 * replicated through GearPawn.ReplicatedPlayTakeHitEffects().
 */
final simulated function SpawnImpactFX(const class<GearDamageType> GearDmgType, const out ImpactInfo Impact, bool bFatalShot, bool bMeleeHit, bool bPlayExplosiveRadialDamageEffects)
{
	local PhysicalMaterial PhysMaterial;
	local GearPawn GP;
	local GearGRI GRI;


	// we probably want to also move up the FrameRate Dependent checking to here instead of calling the function and then just returning nothing
	// so it is really this:
	// if the framerate allows us to spawn effect<type> then we need to go and look and see if it is relevant to us
	// (for all relevant effects we have to check framerate shizzle so might as ALWAYS just have a Framerate check cost
	// before doing the more expensive relevancy checks / or going through and trying to spawn stuff
	//

	if( GearDmgType.default.bSuppressImpactFX == FALSE )
	{
		// grab the physical material to get things rolling
		PhysMaterial = class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterial(Impact);

		GRI = GearGRI(WorldInfo.GRI);
		GP = GearPawn(Impact.HitActor);
		//`log( "SpawnImpactFX" @ self );
		// only Spawn impact effects if the Pawn has not been dead for too long
		if( ( GP == None) || ( ( WorldInfo.TimeSeconds - GP.TimeOfDeath < 20.0f ) || (!GP.bTearOff) ) )
		{
			if( GRI.IsEffectRelevant( Instigator, Impact.HitLocation, 4000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
			{
				// play the impact effect(s)
				SpawnImpactEffects( GearDmgType, Impact, PhysMaterial, bFatalShot, bMeleeHit, bPlayExplosiveRadialDamageEffects );
				//`log( " SpawnImpactEffects" @ self );
			}
		 }
		// only Spawn impact decals if the Pawn has not been dead for too long
		if( ( GP == None) || ( ( WorldInfo.TimeSeconds - GP.TimeOfDeath < 5.0f ) || (!GP.bTearOff) ) )
		{
			if( GRI.IsEffectRelevant( Instigator, Impact.HitLocation, 4000, FALSE, GRI.CheckEffectDistance_SpawnBehindIfNear ) )
			{
				// leave a decal
				SpawnImpactDecal( GearDmgType, Impact, PhysMaterial );
				//`log( " SpawnImpactDecal" );
			}
		}
		// how far can you hear impact effects from weapons?  Explosions are different  (According to the soundcue up to 6000 units!)
		if( GRI.IsEffectRelevant( Instigator, Impact.HitLocation, 3000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistance ) )
		{
			// play an impact sound
			SpawnImpactSounds( GearDmgType, Impact, PhysMaterial );
			//`log( " SpawnImpactSounds" );
		}
	}
}

/**
 * This will spawn a PhysicalMaterial based effect.	 It looks up the PhysicalMaterial
 * tree until it finds a valid effect to Spawn.	 If it does not find one then
 * it defaults the effect coded in to the .uc file for that specific weapon
 */
simulated function SpawnImpactDecal(const class<GearDamageType> GearDmgType, const out ImpactInfo Impact, PhysicalMaterial PhysMaterial)
{
	local DecalData DecalData;
	local GearPawn GP;
	local SkeletalMeshComponent SkelMeshHitComponent;

	GP = GearPawn(Impact.HitActor);

	// if we are NOT a head bone then we can leave a decal
	if( ( GP == None) || ( GP.TookHeadShot(Impact.HitInfo.BoneName, Impact.HitLocation, Impact.RayDir) == FALSE ) )
	{
		SkelMeshHitComponent = SkeletalMeshComponent(Impact.HitInfo.HitComponent);
		if( (SkelMeshHitComponent == None)
			|| ( (SkelMeshHitComponent != None) && (Instigator != None) && (VSize(Impact.HitLocation - Instigator.Location) < DecalImpactRelevanceDistanceSkelMesh) )
			&& ( ClassIsChildOf( GearDmgType, class'GDT_Explosive' ) == FALSE )
			)
		{
			//`LogExt("Instigator:"@Instigator@"PhysMaterial:"@PhysMaterial@"HitActor:"@Impact.HitActor);
			DecalData = class'GearPhysicalMaterialProperty'.static.DetermineImpactDecalData( PhysMaterial, GearDmgType, GearPawn(Instigator) != None ? GearPawn(Instigator).bActiveReloadBonusActive : FALSE, WorldInfo, Instigator );

			//`log( "SpawnImpactDecal PhysMaterial: " $ PhysMaterial @ DecalData.DecalMaterial );

			// create the actual decal
			if( DecalData.bIsValid )
			{
				class'GearDecal'.static.StaticAttachFromImpact( class'GearDecal_WeaponImpact', GearPawn(Instigator), Impact, DecalData );
			}
		}

		// so now check to see if we hit a pawn and need to do a exit wound splatter decal (50% of the time)
		if( ( Frand() < 0.50f ) && ( GP != None ) && GP.bSpawnHitEffectDecals )
		{
			//`LogExt("Instigator:"@Instigator@"PhysMaterial:"@PhysMaterial@"HitActor:"@Impact.HitActor);
			DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Impact.HitInfo ), GDTT_ExitWoundSplatter, WorldInfo );

			//`log( "SpawnImpactDecal PhysMaterial: " $ PhysMaterial @ DecalData.DecalMaterial );

			// create the actual decal
			if( DecalData.bIsValid )
			{
				class'GearDecal'.static.StaticAttachToSurface( class'GearDecal_WeaponImpact', GearPawn(Instigator), Impact.HitLocation, Impact.RayDir, 128.0f, Impact, DecalData );
			}
		}
	}
}

simulated function ParticleSystemComponent GetImpactParticleSystemComponent( ParticleSystem PS_Type )
{
	return GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( PS_Type );
}


simulated function Emitter GetImpactEmitter( ParticleSystem PS_Type, vector SpawnLocation, rotator SpawnRotation )
{
	return GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_Type, SpawnLocation, SpawnRotation );

}


simulated function SpawnImpactEffects(const class<GearDamageType> GearDmgType, ImpactInfo Impact, PhysicalMaterial PhysMaterial, bool bWasFatalShot, bool bWasMeleeAttack, bool bPlayExplosiveRadialDamageEffects )
{
	local SkeletalMeshComponent SMC;
	local vector DirectionToSprayParticles, BoneRelativeLocation;
	local rotator BoneRelativeRotation;
	local GearPawn HitPawn;
	local ParticleSystem ImpactPS;
	local ParticleSystemComponent ImpactPSC;
	local Emitter ImpactEmitter;
	local name AttachToBoneName;

	local vector HitLocation, HitNormal;
	local vector ExitWoundStart,ExitWoundEnd;

	//`LogExt("Hit:"@Impact.HitActor@"HitComp"@Impact.HitInfo.HitComponent@"Fatal:"@bWasFatalShot@"Melee:"@bWasMeleeAttack@"PhysMat:"@PhysMaterial@WeaponID@GearDmgType@GearDmgType.default.bSuppressImpactFX);

	// abort early if FX are turned off (dummy fire)
	if( bSuppressImpactFX || GearDmgType.default.bSuppressImpactFX )
	{
		return;
	}

	// check to see if we're dealing with a pawn
	HitPawn = GearPawn(Impact.HitActor);

	// if it's a fatal shot,we will spawn GoreExplosion blood there!

	// if it's a melee hit,
	if (bWasMeleeAttack)
	{
		if( WorldInfo.GRI.ShouldShowGore() && HitPawn != None )
		{
			ImpactPS = HitPawn.PS_MeleeImpactEffect;
		}
		// we don't show any impact effects atm for melee hits when extreme content is off
		else
		{
			return;
		}
	}
	else if( bPlayExplosiveRadialDamageEffects )
	{
		if( WorldInfo.GRI.ShouldShowGore() )
		{
			ImpactPS = HitPawn.PS_RadialDamageEffect;
		}
		// we don't show any impact effects atm for melee hits when extreme content is off
		else
		{
			return;
		}
	}
	else
	// check for a pawn override
	if (HitPawn != None && HitPawn.HasImpactOverride())
	{
		ImpactPS = HitPawn.GetImpactOverrideParticleSystem();
	}
	// otherwise look to the physical material
	else
	{
		ImpactPS = class'GearPhysicalMaterialProperty'.static.DetermineImpactParticleSystem( PhysMaterial, GearDmgType, GearPawn(Instigator) != None ? GearPawn(Instigator).bActiveReloadBonusActive : FALSE, WorldInfo, Instigator );
	}

	//`LogExt("- ImpactPS:"@ImpactPS);

	// if we ended up with a particle system
	if (ImpactPS != None)
	{
		SMC = SkeletalMeshComponent(Impact.HitInfo.HitComponent);
		DirectionToSprayParticles = Impact.HitNormal;
		// attach to the skeletal mesh component if possible
		if (SMC != None)
		{
			// Make sure we always have a valid bone name (e.g. ink grenade damage)
			AttachToBoneName = (Impact.HitInfo.BoneName == '') ? HitPawn.MeleeDamageBoneName : Impact.HitInfo.BoneName;

			// get a particle system component for this system
			ImpactPSC = GetImpactParticleSystemComponent(ImpactPS);
			`assert(ImpactPSC != None);
			// and transform/attach to the mesh
			SMC.TransformToBoneSpace(AttachToBoneName, Impact.HitLocation, Rotator(DirectionToSprayParticles), BoneRelativeLocation, BoneRelativeRotation);
			SMC.AttachComponent(ImpactPSC, AttachToBoneName, BoneRelativeLocation, BoneRelativeRotation);
			ImpactPSC.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(ImpactPSC.Template, Impact.HitLocation));
			ImpactPSC.ActivateSystem();

			//`log( "ImpactInfo: " $ AttachToBoneName @ Impact.HitLocation @ BoneRelativeLocation @ BoneRelativeRotation @ ImpactPSC.Template );
			//DrawDebugCoordinateSystem( Impact.HitLocation, Rotator(DirectionToSprayParticles), 3.0f, TRUE );

			// now check for exit wound capability
			// only do this 20% of the time
			if( ( Frand() < 0.20f ) ) // &&( PhysMatProp.ExitWoundPS != none ) )
			{
				//`log( "wooot Exit Wound" );
				ImpactPS = class'GearPhysicalMaterialProperty'.static.DetermineExitWoundParticleSystem( PhysMaterial, GearDmgType, GearPawn(Instigator) != None ? GearPawn(Instigator).bActiveReloadBonusActive : FALSE, WorldInfo );
				if( ImpactPS != None )
				{
					ImpactPSC = GetImpactParticleSystemComponent(ImpactPS);

					ExitWoundEnd = Impact.HitLocation;
					ExitWoundStart = ExitWoundEnd + (-Impact.HitNormal * 32.0f);

					// note this will break if you have a shot -> body |air| body  then the trace back will be through the second body
					TraceComponent( HitLocation, HitNormal, SMC, ExitWoundEnd, ExitWoundStart,, );

					SMC.TransformToBoneSpace(AttachToBoneName, HitLocation, Rotator(HitNormal), BoneRelativeLocation, BoneRelativeRotation);

					SMC.AttachComponent(ImpactPSC, AttachToBoneName, BoneRelativeLocation, BoneRelativeRotation);
					ImpactPSC.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(ImpactPSC.Template, Impact.HitLocation));
					ImpactPSC.ActivateSystem();

					//DrawDebugCoordinateSystem( HitLocation, Rotator(HitNormal), 3.0f, TRUE );
					//DrawDebugLine( ExitWoundStart, ExitWoundEnd , 255, 1, 1, TRUE );
				}

				// when ever we do an exit wound we also want to do a little blood splash
				// unless we already did one due to being really hurt
				if (HitPawn != none && HitPawn.Health >= 0.40 * HitPawn.DefaultHealth)
				{
					HitPawn.SpawnABloodTrail_HitByABullet();
				}
			}
		}
		// otherwise grab an ol' fashioned emitter
		else
		{
			ImpactEmitter = GetImpactEmitter(ImpactPS,Impact.HitLocation,rotator(DirectionToSprayParticles));
			`assert(ImpactEmitter != None);
			//`log( "SetBase: " $ Impact.HitActor );
			ImpactEmitter.SetBase(Impact.HitActor);
			ImpactEmitter.ParticleSystemComponent.ActivateSystem();
		}
	}
	else
	{
		`warn("No impact particle system found:"@self@Impact.HitActor@PhysMaterial@GearDmgType);
	}
}


simulated function PlayMeleeScreenShake( GearPawn Victim )
{
	local GearPC PC;

	PC = GearPC(Instigator.Controller);
	if( PC != None )
	{
		PC.ClientPlayCameraShake( MeleeImpactCamShake, FALSE ); // we already do a forcefeedback elsewhere
	}

	if(Victim != None)
	{
		PC = GearPC(Victim.Controller);
		if( PC != None )
		{
			PC.ClientPlayCameraShake( MeleeImpactCamShake, FALSE ); // we already do a forcefeedback elsewhere
		}
	}

}

/**
 * This will spawn a PhysicalMaterial based sound.	It looks up the PhysicalMaterial
 * tree until it finds a valid sound to Spawn.	If it does not find one then
 * it defaults the sound coded in to the .uc file for that specific weapon
 *
 **/
simulated function SpawnImpactSounds( const class<GearDamageType> GearDmgType, ImpactInfo Impact, PhysicalMaterial PhysMaterial )
{
	local SoundCue ImpactCue;

	ImpactCue = class'GearPhysicalMaterialProperty'.static.DetermineImpactSound( PhysMaterial, GearDmgType, GearPawn(Instigator) != None ? GearPawn(Instigator).bActiveReloadBonusActive : FALSE, WorldInfo );

	if( ImpactCue != None )
	{
		// play the sound
		PlaySound( ImpactCue, TRUE, TRUE, FALSE, Impact.HitLocation, TRUE );
		//PlaySound(ImpactCue,TRUE,,TRUE,Impact.HitLocation);
	}
}

simulated function Actor GetTraceOwner()
{
	local Actor A;

	// For dummy fire weapon, try to trace from skeletal mesh actor
	if( bDummyFireWeapon )
	{
		A = DummyFireParent;
		while( A != None && !A.IsA('SkeletalMeshActor') )
		{
			A = A.Base;
		}
		if( A != None )
		{
			return A;
		}
	}

	return Super.GetTraceOwner();
}

simulated function SetCurrentFireMode(byte FiringModeNum)
{
	// slave weapons receive their firing mode through replication of the Pawn property
	if (!bSlaveWeapon)
	{
		Super.SetCurrentFireMode(FiringModeNum);
	}
}

simulated event DummyFire(byte FireModeNum, vector TargetLoc, optional Actor AttachedTo, optional float AimErrorDeg, optional Actor TargetActor)
{
	local ImpactInfo InstantHitImpact;
	local vector StartLoc, EndLoc;
	local rotator AimRot;
	local float AimErrorUnr;

	DummyFireParent = AttachedTo;
	bDummyFireWeapon = TRUE;
	DummyFireTargetLoc	= TargetLoc;
	DummyFireTargetActor = TargetActor;

	SetCurrentFireMode(FireModeNum);

	// impact effects
	switch (WeaponFireTypes[FireModeNum])
	{
	case EWFT_Projectile:
		// spit out a projectile
		ProjectileFireSimple(AimErrorDeg);
		break;
	case EWFT_InstantHit:
		// simulate instant hit weapon
		StartLoc = GetPhysicalFireStartLoc();

		AimRot = rotator(DummyFireTargetLoc - StartLoc);

		// modify Aim to add player aim error
		// and convert aim error from degrees to Unreal Units ( * 65536 / 360 )
		if (AimErrorDeg != 0.f)
		{
			AimErrorUnr = AimErrorDeg * 182.044;
			AimRot.Pitch += AimErrorUnr * (0.5 - FRand());
			AimRot.Yaw	 += AimErrorUnr * (0.5 - FRand());
		}

		EndLoc = StartLoc + vector(AimRot) * 30000;		// way out there

		InstantHitImpact = CalcWeaponFire(StartLoc, EndLoc);
		if (InstantHitImpact.HitActor != None)
		{
			WeaponFired(FireModeNum, InstantHitImpact.HitLocation);
			CalcRemoteImpactEffects(FireModeNum, InstantHitImpact.HitLocation, FALSE);
		}
		else
		{
			WeaponFired(FireModeNum, EndLoc);
		}
		break;
	case EWFT_Custom:
		// individual weapon subclass should overload DummyFire in this case
	default:
		break;
	}
}

simulated function bool PassThroughDamage(Actor HitActor)
{
	return Super.PassThroughDamage(HitActor) || HitActor.IsA('GrenadeBlockingVolume');
}

simulated function static bool PassThroughDamageStatic(Actor HitActor)
{
	return ((!HitActor.bBlockActors && (HitActor.IsA('Trigger') || HitActor.IsA('TriggerVolume')))) || HitActor.IsA('GrenadeBlockingVolume');
}

simulated function static ImpactInfo CalcRemoteWeaponFireStatic(Vector StartTrace, vector EndTrace, Actor TraceOwner)
{
	local vector			HitLocation, HitNormal;
	local Actor				HitActor;
	local TraceHitInfo		HitInfo;
	local ImpactInfo		CurrentImpact;
	local bool bToggledBlockActors;

	// Perform trace to retrieve hit info
	HitActor = TraceOwner.Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE, vect(0,0,0), HitInfo, TRACEFLAG_Bullet);

	// If we didn't hit anything, then set the HitLocation as being the EndTrace location
	if( HitActor == None )
	{
		HitLocation	= EndTrace;
	}

	// check to see if we've hit a trigger.
	// In this case, we want to add this actor to the list so we can give it damage, and then continue tracing through.
	if ( (HitActor != None) && PassThroughDamageStatic(HitActor) )
	{
		// disable collision temporarily for the trigger so that we can catch anything inside the trigger
		HitActor.bProjTarget = FALSE;
		if (HitActor.bBlockActors)
		{
			HitActor.SetCollision(HitActor.bCollideActors, false);
			bToggledBlockActors = true;
		}
		// recurse another trace
		CurrentImpact = CalcRemoteWeaponFireStatic(HitLocation, EndTrace,TraceOwner);
		// and re-enable collision for the trigger
		HitActor.bProjTarget = TRUE;
		if (bToggledBlockActors)
		{
			HitActor.SetCollision(HitActor.bCollideActors, true);
		}
	}
	else
	{
		// Convert Trace Information to ImpactInfo type.
		CurrentImpact.HitActor		= HitActor;
		CurrentImpact.HitLocation	= HitLocation;
		CurrentImpact.HitNormal		= HitNormal;
		CurrentImpact.RayDir		= Normal(EndTrace-StartTrace);
		CurrentImpact.HitInfo		= HitInfo;
	}

	return CurrentImpact;
}
/** basically same as CalcWeaponFire, but a pure query, no side effects
 * @note: make sure the special cases stay mirrored (PassThroughDamage, etc)
 */
simulated function ImpactInfo CalcRemoteWeaponFire(vector StartTrace, vector EndTrace)
{
	local vector			HitLocation, HitNormal;
	local Actor				HitActor;
	local TraceHitInfo		HitInfo;
	local ImpactInfo		CurrentImpact;
	local bool bToggledBlockActors;

	// Perform trace to retrieve hit info
	HitActor =GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE, vect(0,0,0), HitInfo, TRACEFLAG_Bullet);

	// If we didn't hit anything, then set the HitLocation as being the EndTrace location
	if( HitActor == None )
	{
		HitLocation	= EndTrace;
	}

	// check to see if we've hit a trigger.
	// In this case, we want to add this actor to the list so we can give it damage, and then continue tracing through.
	if ( (HitActor != None) && PassThroughDamage(HitActor) )
	{
		// disable collision temporarily for the trigger so that we can catch anything inside the trigger
		HitActor.bProjTarget = FALSE;
		if (HitActor.bBlockActors)
		{
			HitActor.SetCollision(HitActor.bCollideActors, false);
			bToggledBlockActors = true;
		}
		// recurse another trace
		CurrentImpact = CalcRemoteWeaponFire(HitLocation, EndTrace);
		// and re-enable collision for the trigger
		HitActor.bProjTarget = TRUE;
		if (bToggledBlockActors)
		{
			HitActor.SetCollision(HitActor.bCollideActors, true);
		}
	}
	else
	{
		// Convert Trace Information to ImpactInfo type.
		CurrentImpact.HitActor		= HitActor;
		CurrentImpact.HitLocation	= HitLocation;
		CurrentImpact.HitNormal		= HitNormal;
		CurrentImpact.RayDir		= Normal(EndTrace-StartTrace);
		CurrentImpact.HitInfo		= HitInfo;
	}

	return CurrentImpact;
}


/**
 * Calculate weapon impacts for remote clients.
 * Remote clients may only get a HitLocation replicated,
 * so we need to figure out the HitNormal and HitActor to play proper effects.
 */

simulated function CalcRemoteImpactEffects( byte FireModeNum, vector GivenHitLocation, bool bViaReplication )
{
	local vector TraceOffset, AimDir, WeaponLoc;
	local ImpactInfo	TestImpact;
	local float			HitDistance;

	// if no instigator (some system is manipulating weapons by hand), measure from fire start location
	WeaponLoc = (Instigator != None) ? Instigator.Location : Location;

	// if we should spawn a tracer effect for remote clients, let's do so
	if( bViaReplication && ShouldSpawnTracerFX() )
	{
		HitDistance = VSize(GivenHitLocation - WeaponLoc);
		SpawnTracerEffect( GivenHitLocation, HitDistance );
	}

	// do another trace to retrieve Hit Info
	AimDir		= Normal(GivenHitLocation - WeaponLoc);
	TraceOffset	= AimDir * 16.f;

	// Only trace slightly before and after our HitLocation
	TestImpact = CalcRemoteWeaponFire(GivenHitLocation-TraceOffset, GivenHitLocation+TraceOffset);

	// We didn't hit anything, use GivenHitLocation as our HitLocation
	if( TestImpact.HitActor == None )
	{
		TestImpact.HitLocation = GivenHitLocation;
	}

	// Process Instant Hit. We use same path as local clients and server.
	ProcessInstantHit_Internal( FireModeNum, TestImpact );
}


/** return true if should spawn tracer effect for instant trace hits */
simulated function bool ShouldSpawnTracerFX()
{
	return (bAllowTracers && !bSuppressTracers);
}

simulated function GearProj_BulletTracer GetTracer( vector SpawnLocation, rotator SpawnRotation )
{
	return GearGRI(WorldInfo.GRI).GOP.GetTracer( TracerType, 0, SpawnLocation, SpawnRotation );
}


/**
 * Spawn tracer effect for instant hit shots.
 * Network: All Clients
 *
 * @param HitLocation	Location instant trace did hit
 & @param HitDistance	Distance to the hit location
 */

simulated function GearProj_BulletTracer SpawnTracerEffect( vector HitLocation, float HitDistance )
{
	//
	// note: new prototype tracers for now, just spawn the effect.
	// move to a ObjectPool approach later?
	//

	local ParticleSystem	TracerFX;
	local GearPawn			InstigatorGP;
	local vector			SpawnLoc;
	local Emitter		    SpawnedTracerEmitter;
	local rotator           SpawnRot;
	local GearProj_BulletTracer	SpawnedProjectile;
	local vector			FinalTracerScale;

	SpawnLoc = GetMuzzleLoc();

	// NOTE we want the tracers to ALWAYS be relevant as there are lots of cases where people are shooting things far far away
	// and maybe have been hidden somewhere for more than 2 seconds.
	// and in MP seeing tracers from some hidden spot is vital
	// and in SP the AI often hides behind you so you can get into the case where they are shooting but you see not tracers

	// is this an AR shot?
	InstigatorGP = GearPawn(Instigator);

	// spawn a tracer smoke trail only 50% of the time
	if( FRand() > TracerSmokeTrailFrequency )
	{
		TracerFX = ( (InstigatorGP != None) && (InstigatorGP.bActiveReloadBonusActive) && (TracerSmokeTrailEffectAR != None) ) ? TracerSmokeTrailEffectAR : TracerSmokeTrailEffect;
		if(TracerFX != None)
		{
			SpawnedTracerEmitter = GetImpactEmitter( TracerFX, SpawnLoc, Rotation );
			SpawnedTracerEmitter.ParticleSystemComponent.SetVectorParameter('SnipeEnd', HitLocation);
			SpawnedTracerEmitter.ParticleSystemComponent.ActivateSystem();
			//`log("***"@self@"spawned tracer"@TracerFX@SpawnLoc@HitLocation);
		}
	}

	// always spawn a tracer
	// old-style projectile tracers.  The smoke trail is a beam particle effect but FX artists want this type of tracer
	if( HitDistance > ShowTracerDistance )
	{
		SpawnLoc = GetMuzzleLoc();
		SpawnedProjectile = GetTracer( SpawnLoc, SpawnRot );

		if ( (SpawnedProjectile != None) && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Instigator = Instigator;

			// set tracer scale
			FinalTracerScale = TracerScale;
			SpawnedProjectile.SetDrawScale3D(FinalTracerScale);
			//`log("scaled tracer"@FinalTracerScale);

			// init!
			SpawnedProjectile.InitTracer(SpawnLoc, HitLocation);
		}
	}

	return SpawnedProjectile;

}



/**
 * Called when a shot is fired in the following situations:
 * 1) On local player and server.
 * 2) On remote clients if Pawn.FlashCount or Pawn.FlashLocation is incremented on the server.
 *
 * @NOTE:	this is not a state scoped function (non local clients ignore the state the weapon is in)
 *			use the supplied FireModeNum variable.
 *
 * Network: ALL
 *
 * @param	FireModeNum.
 * @param	HitLocation	optional, just sent when replication occured through SetFlashLocation()
 */
simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	// Start muzzle flash effect
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// only play the shell eject if we are a player controller
		if (Instigator != None && Instigator.IsHumanControlled() && Instigator.IsLocallyControlled())
		{
			PlayShellCaseEject();
			PlayerController(Instigator.Controller).ClientPlayForceFeedbackWaveform(WeaponFireWaveForm);
		}

		// Play muzzle flash effect
		PlayMuzzleFlashEffect();

		// play weapon fire sound
		PlayFireSound();
	}

	// Play fire effects for owned player
	if (WorldInfo.Netmode != NM_Client || (Instigator != None && Instigator.IsLocallyControlled()) || bDummyFireWeapon)
	{
		PlayOwnedFireEffects( FireModeNum, HitLocation );
	}
}


/**
 * Called when the weapon stops firing, allows for stoppage of
 * looping effects.
 */
simulated function StopFireEffects(byte FireModeNum)
{
	if( bLoopMuzzleFlashLight )
	{
		// turn off muzzle flash light
		if( MuzzleFlashLight != None )
		{
			MuzzleFlashLight.SetEnabled(FALSE);
		}
	}
}

simulated function PlayShellCaseEject()
{
	local GearPawn	GP;
	local Rotator	NewRot;

	// shell eject particle system
	if( PSC_ShellEject != None )
	{
		GP = GearPawn(Instigator);

		// If pawn is mirrored, then mirror shell ejection Y axis as well
		if( GP != None )
		{
			// we need to manually flip the meshes on the emitters so they are not backwards
			if( GP.bIsMirrored )
			{
				PSC_ShellEject.SetVectorParameter( 'MeshOrientation', vect(1,1,1) );
			}
			else
			{
				PSC_ShellEject.SetVectorParameter( 'MeshOrientation', vect(0,0,0) );
			}

			// we don't want to mirror the shell if we are using the muzzle flash socket as the weapon mesh already is scaled
			if( !ShouldUseMuzzleFlashSocketForShellEjectAttach() )
			{
				// LAURENT: forced offsets, because Component.default.Property doesn't work ATM.
				if( GP.bIsMirrored )
				{
					NewRot.Yaw = 16384;
				}
				else
				{
					NewRot.Yaw = -16384;
				}

				if( PSC_ShellEject.Rotation != NewRot )
				{
					PSC_ShellEject.SetRotation(NewRot);
				}
			}
		}

		ClearTimer( 'HidePSC_ShellEject' );
		PSC_ShellEject.SetHidden( FALSE );
	    //@SAS. Set the bJustAttched flag when activating...
		PSC_ShellEject.ActivateSystem(TRUE);
	}
}

/**
 * Play fire effects for owned player
 * Network: Local Player and Server
 */
simulated function PlayOwnedFireEffects(byte FireModeNum, vector HitLocation)
{
	// if we should spawn a tracer effect for the local player, let's do so
	if( WorldInfo.NetMode != NM_DedicatedServer &&
		ShouldSpawnTracerFX() &&
		!IsZero(HitLocation)
		)
	{
		if (Instigator != None)
		{
			SpawnTracerEffect( HitLocation, VSize(HitLocation - Instigator.Location) );
		}
		else
		{
			// if no instigator (some system is manipulating weapons by hand), measure from fire start location
			SpawnTracerEffect( HitLocation, VSize(HitLocation - GetMuzzleLoc()) );
		}
	}

	if( (Instigator != None) && Instigator.IsHumanControlled() )
	{
		// Add weapon recoil effect to player's viewrotation
		SetWeaponRecoil( GetWeaponRecoil() );
		PlayFireCameraEffects();
	}
}

/** Overrideable for special firing audio behavior. */
simulated protected function PlayFireSound()
{
	GearWeaponPlaySoundLocal( FireSound, FireSound_Player,, 1.f );
}

/** Return the screenshake struct to play for this weapon */
simulated function ScreenShakeStruct GetFireViewShake()
{
	return FireCameraShake;
}

/** Play Weapon Camera Shake, etc */
simulated function PlayFireCameraEffects( )
{
	local ScreenShakeStruct FCS;
	local float			ARScalar;
	local GearPC		GPC;
	local GearPawn		GP;

	if( Instigator != None )
	{
		GP = GearPawn(Instigator);
		if( Instigator.Controller != None )
		{
			GPC = GearPC(Instigator.Controller);
		}
	}

	if( bPlayFireCameraShake &&	(GPC != None) )
	{
		FCS = GetFireViewShake();

		// maybe scale camera shake based on active reload success
		if( GP != None && !GP.bActiveReloadBonusActive )
		{
			ARScalar = GP.ActiveReload_CurrTier * 0.05f + 3.0f;
		}
		else
		{
			ARScalar = 1.f;
		}

		// note: FireCameraShakeScale applied to amplitudes
		FCS.FOVAmplitude *= FireCameraShakeScale * ARScalar;
		FCS.FOVFrequency *= ARScalar;
		FCS.LocAmplitude *= FireCameraShakeScale;
		FCS.RotAmplitude *= FireCameraShakeScale;

		GPC.ClientPlayCameraShake( FCS );
	}
}

/************************************************************************************
 * Muzzle Flash
 ***********************************************************************************/


/** Returns TRUE, if muzzle flash effects should be played. */
simulated function bool IsMuzzleFlashRelevant()
{
	local float MuzzleFlashRadius;
	local vector MuzzleLoc;

	MuzzleLoc = GetMuzzleLoc();

	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, MuzzleLoc, 5000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
	{
		return FALSE;
	}

	// Always should muzzle flashes on the player we're controlling or if the instigator is None (dummy fire)
	if( Instigator == None || (Instigator.IsHumanControlled() && Instigator.IsLocallyControlled()) )
	{
		return TRUE;
	}

	// If we have a muzzle flash light, use its radius as an indication
	MuzzleFlashRadius = MuzzleFlashLight == None ? 256.f : MuzzleFlashLight.Radius + 60.f;

	// if frame rate is really bad and Pawn hasn't been rendered since last second, then don't display effects
	if( WorldInfo.bAggressiveLOD && WorldInfo.TimeSeconds - Instigator.LastRenderTime > 1.f)
	{
		return FALSE;
	}

	// If Instigator hasn't been rendered for 2 seconds and camera isn't really close to the instigator, then don't play muzzle flash effects
	if (WorldInfo.TimeSeconds - Instigator.LastRenderTime > 2.f && !IsCameraWithinRadius(Instigator.Location, MuzzleFlashRadius))
	{
		return FALSE;
	}

	return TRUE;
}

final simulated function bool IsCameraWithinRadius(const out Vector TestLocation, float Radius)
{
	local PlayerController		PC;
	local Vector	CamLoc;
	local Rotator	CamRot;

	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return FALSE;
	}

	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		PC.GetPlayerViewPoint(CamLoc, CamRot);

		if( VSize(TestLocation - CamLoc) <= Radius )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Start muzzle flash effect
 * Network: Local Player and Clients
 */
simulated function PlayMuzzleFlashEffect()
{
	local vector		SpawnLoc;
	local Emitter	SpawnedSmokeEmitter;

	// Play muzzle flash effects only if they are relevant
	if( !bSuppressMuzzleFlash && IsMuzzleFlashRelevant() )
	{
		// turn on muzzle flash mesh
		//`log("playing muzzle flash"@MuzzleFlashMesh@MuzFlashEmitter@MuzzleFlashLight);
		if( MuzzleFlashMesh != None )
		{
			MuzzleFlashMesh.SetHidden(FALSE);

			// Randomize muzzle flash mesh size
			RandomizeMuzzleFlash(MuzzleFlashMesh);
		}

		// activate muzzle flash particle system
		if( MuzFlashEmitter != None )
		{
			ClearTimer( 'HideMuzzleFlashEmitter' );
			MuzFlashEmitter.SetHidden( FALSE );

			if (bMuzFlashEmitterIsLooping)
			{
				MuzFlashEmitter.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(MuzFlashEmitter.Template, GetMuzzleLoc()));
				MuzFlashEmitter.SetActive(TRUE);
			}
			else
			{
				MuzFlashEmitter.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(MuzFlashEmitter.Template, GetMuzzleLoc()));
				MuzFlashEmitter.ActivateSystem();
			}
		}

		if( MuzzleFlashLight != None
			&& !GearGRI(WorldInfo.GRI).ShouldDisableEffectsDueToFramerate(Instigator)
			&& (!bLoopMuzzleFlashLight || !MuzzleFlashLight.bEnabled)
			)
		{
			MuzzleFlashLight.CastDynamicShadows = bDynamicMuzzleFlashes;
			MuzzleFlashLight.SetEnabled(TRUE);

			// Reset Pulse time to zero, so we start at the beginning.
			MuzzleLightPulseTime = 0.f;
		}

		// maybe spawn lingering muzzle smoke
		if( (MuzSmokeParticleSystem != None)
			&& !GearGRI(WorldInfo.GRI).ShouldDisableEffectsDueToFramerate(Instigator)
			)
		{
			SpawnLoc = GetMuzzleLoc();
			SpawnedSmokeEmitter = GetImpactEmitter( MuzSmokeParticleSystem, SpawnLoc, rot(0,0,0) );
			SpawnedSmokeEmitter.ParticleSystemComponent.ActivateSystem();
		}

		SetTimer( MuzzleLightDuration, FALSE, nameof(StopMuzzleFlashEffect) );
	}
}



/**
 * Stop muzzle flash effect
 * Network: Local Player and Clients
 */
simulated function StopMuzzleFlashEffect()
{
	// turn off muzzle flash mesh
	if( MuzzleFlashMesh != None )
	{
		MuzzleFlashMesh.SetHidden( TRUE );
	}

	if( MuzFlashEmitter != None )
	{
		MuzFlashEmitter.SetActive(FALSE);
		SetTimer( TimeToHideMuzzleFlashPS, FALSE, nameof(HideMuzzleFlashEmitter) );
	}

	// turn off muzzle flash light only if it's non looping
	// Looping ones are turned off when the weapon stops firing
	// Used by rapid fire weapons
	if( !bLoopMuzzleFlashLight && MuzzleFlashLight != None )
	{
		MuzzleFlashLight.SetEnabled( FALSE );
	}
}


simulated function HideMuzzleFlashEmitter()
{
	MuzFlashEmitter.SetHidden( TRUE );
}

simulated function HidePSC_ShellEject()
{
	PSC_ShellEject.SetHidden( TRUE );
	PSC_ShellEject.RewindEmitterInstances();
}

simulated function HidePSC_ReloadBarrelSmoke()
{
	PSC_ReloadBarrelSmoke.SetHidden( TRUE );
}


/**
 * Randomizes muzzle flash mesh transform component. to give a random aspect to it.
 */

simulated function RandomizeMuzzleFlash( PrimitiveComponent MuzzleFlash )
{
	local Vector	V;
	local Rotator	R;

	// Randomize muzzle flash mesh size
	V.X = MuzzleFlash.default.Scale3D.X * ( 0.8 + 0.4*FRand() );
	V.Y = MuzzleFlash.default.Scale3D.Y * ( 0.8 + 0.4*FRand() );
	V.Z = MuzzleFlash.default.Scale3D.Z * ( 0.8 + 0.4*FRand() );

	MuzzleFlash.SetScale3D( V );

	// randomize muzzle flash mesh roll angle
	R = MuzzleFlash.Rotation;
	R.Roll = FRand() * 65536;
	MuzzleFlash.SetRotation( R );
}


/************************************************************************************
 * HUD
 ***********************************************************************************/

/**
 * Access to HUD and Canvas.
 * Event always called when the InventoryManager considers this Inventory Item currently "Active"
 * (for example active weapon)
 *
 * @param	HUD H
 */

simulated function ActiveRenderOverlays( HUD H )
{
	//DrawWeaponCrosshair( H );
	//TNT
	`if(`isdefined(_MGSTEST_))
		chkIfAimingAtEnemy();
	`endif
}

/**
 * list important Weapon variables on canvas.  HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */

simulated function GetWeaponDebug( out Array<String> DebugInfo )
{
	super.GetWeaponDebug( DebugInfo );

	DebugInfo[DebugInfo.Length] = "AmmoUsedCount:" $ AmmoUsedCount @ "MagazineSize:" $ GetMagazineSize();
	DebugInfo[DebugInfo.Length] = "SpareAmmoCount:" $ SpareAmmoCount @ "SpareAmmoSize:" $ GetMaxSpareAmmoSize();
	DebugInfo[DebugInfo.Length] = "bDisableLeftHandIK:" $ bDisableLeftHandIK;
}


unreliable client function ClientFlushPersistentDebugLines()
{
	FlushPersistentDebugLines();
}


/** Returns TRUE if should draw crosshair */
simulated function bool ShouldDrawCrosshair()
{
	local GearPawn GP;
	local GearVehicle GV;
	local PlayerController PC;
	GP = GearPawn(Instigator);
	if (GP != None && GP.bIsTargeting)
	{
		return TRUE;
	}

	GV = GearVehicle(Instigator);
	if(GV != None)
	{
		PC = PlayerController(GV.Controller);
	}
	if (PC != None && GV.WantsCrosshair(PC))
	{
		return TRUE;
	}

	return FALSE;
}

/** Returns the value to scale the cross hair by */
simulated function float GetCrossHairScale()
{
	return 0.7f;
}

simulated function Texture2D GetCrosshairIcon(GearPC PC, out float YOffset)
{
	return CrosshairIcon;
}

/** returns whether or not the passed state name is from a state that actually fires **/
protected function bool IsFireState( name StateName )
{
	return (FiringStatesArray.Find(StateName) != INDEX_NONE);
}

/** if we just reloaded and then consumed ammo, we might have gone to 0 and back to
 * the same value we had before in one tick
 * in this case, variable replication will think nothing has changed,
 * but the client only simulates the ammo counter reset, not the change from firing
 * this function forces an update regardless of the previous value
 */
reliable client final function ClientForceUpdateAmmoUsed(int NewAmmoUsed)
{
	AmmoUsedCount = NewAmmoUsed;
}

simulated function AutoSwitchToNewWeapon()
{
	GearInventoryManager(InvManager).AutoSwitchWeapon();
}

simulated function bool ShouldPreventWeaponSwitch()
{
	return FALSE;
}


/**
 * State Active
 * When a weapon is in the active state, it's up, ready but not doing anything. (idle)
 */
simulated state Active
{
	simulated function BeginState( Name PreviousStateName )
	{
		local int i;
		local bool bOutOfAmmo;

		if( Role == ROLE_Authority )
		{
			// Cache a reference to the AI controller
			GearAIController = GearAI(Instigator.Controller);
			AIController = AIController(Instigator.Controller);
		}

		bOutOfAmmo = !HasAnyAmmo();
		`LogInv("PreviousStateName:" @ PreviousStateName @ "bOutOfAmmo:" @ bOutOfAmmo);

		if (bOutOfAmmo)
		{
			PlayNeedsAmmoChatter();
		}

		if( bWeaponPutDown )
		{
			// put weapon down first
			PutDownWeapon();
		}
		else if( (PendingFire(RELOAD_FIREMODE) || ShouldAutoReload()) && FiringStatesArray[RELOAD_FIREMODE] != '' )
		{
			// then check for weapon reloading
			BeginFire(RELOAD_FIREMODE);
		}
		else if( InvManager != None )
		{
			// if out of ammo, and can't have weapon selected w/o ammo,
			if( bOutOfAmmo &&
				 ( !bCanSelectWithoutAmmo ||
					(bAutoSwitchWhenOutOfAmmo && IsFireState(PreviousStateName) && (FiringStatesArray[MELEE_ATTACK_FIREMODE] != PreviousStateName) )
				 )
				)
			{
				AutoSwitchToNewWeapon();
			}
			else
			{
				// if either of the fire modes are pending, perform them
				for( i=0; i<InvManager.PendingFire.Length; i++ )
				{
					if( PendingFire(i) )
					{
						BeginFire(i);
						// keep going if the BeginFire() didn't do anything
						// (e.g. holding down fire while clip is empty)
						if (GetStateName() != 'Active')
						{
							break;
						}
					}
				}
				//@see: ClientForceUpdateAmmoUsed()
				if (PreviousStateName == 'Reloading' && AmmoUsedCount > 0)
				{
					ClientForceUpdateAmmoUsed(AmmoUsedCount);
				}
			}
		}
	}

	simulated function EndState(Name NextStateName)
	{
		`LogInv("NextStateName:" @ NextStateName);
		super.EndState(NextStateName);
		ClearTimer('PlayNoAmmoFireSound');
	}

	/** Override BeginFire so that it will enter the firing state right away. */
	simulated function BeginFire(byte FireModeNum)
	{
		if( !bDeleteMe && Instigator != None )
		{
			Global.BeginFire(FireModeNum);

			// in the active state, fire right away if we have the ammunition
			if( PendingFire(FireModeNum) )
			{
				if( HasAmmo(FireModeNum) )
				{
					SendToFiringState(FireModeNum);
				}
				else if( FireModeNum == 0 )
				{
					if( !HasAnyAmmo() )
					{
						/** The weapon is out of ammo and this is the sound it makes when firing with no ammo **/
						//`log("no ammo fire sound!");
						PlayNoAmmoFireSound();
						if (NoAmmoFireSoundDelay > 0.f)
						{
							SetTimer( NoAmmoFireSoundDelay, TRUE, nameof(PlayNoAmmoFireSound) );
						}
					}
					else
					{
						// if failed to fire because out of ammo in current clip, then try to reload weapon instead.
						ForceReload();
					}
				}
			}
		}
	}

	simulated function EndFire(byte FireModeNum)
	{
		Global.EndFire(FireModeNum);
		ClearTimer( 'PlayNoAmmoFireSound' );
	}


}

/** The weapon is out of ammo and this is the sound it makes when firing with no ammo **/
simulated function PlayNoAmmoFireSound()
{
	WeaponPlaySound(FireNoAmmoSound);
}

simulated state WeaponFiring
{
	simulated event BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		if( AIController != None )
		{
			SetHoseOffset();
		}
	}

	simulated event EndState( Name NextStateName )
	{
		Super.EndState( NextStateName );

		ClearTimer( 'NotifyWeaponRefireDelayExpired' );
	}
}

reliable server function ServerStartFire(byte FireModeNum)
{
	local GearPawn GP;

	// validate that we're actually able to fire
	GP = GearPawn(Instigator);
	if (GP == None || !GP.IsDoingASpecialMove() || GP.SpecialMoves[GP.SpecialMove].bCanFireWeapon)
	{
		Super.ServerStartFire(FireModeNum);
	}
}

/** Overridden to handle AI weapon firing */
simulated function StartFire( byte FireModeNum )
{
	// If AI cannot fire weapon right now, abort
	if(  AIController != None )
	{
		//debug
		`AILog_Ext( GetFuncName()@FireModeNum@AIController.CanFireWeapon( self, FireModeNum ), 'Weapon', GearAI(AIController) );

		if (!HasAnyAmmo() || !AIController.CanFireWeapon(self, FireModeNum))
		{
			return;
		}

		// Init weapon firing variables for AI
		SetupWeaponFire( FireModeNum );
	}

	Super.StartFire( FireModeNum );
}

/**
 *	AIFunc - Function for AI to setup weapon firing variables
 */
function SetupWeaponFire( byte FireModeNum )
{
	// Setup the burst fire count if applicable
	RemainingBurstFireCount = (FireModeNum == 0) ? GetBurstFireCount()  : -1;
	RemainingBurstsToFire	= (FireModeNum == 0) ? GetBurstsToFire()	: -1;

	//debug
	`AILog_Ext( self@GetFuncName()@FireModeNum@RemainingBurstFireCount@RemainingBurstsToFire, 'Weapon', GearAI(AIController) );
}

/**
 *	AIFunc - Function to get the next burst fire count for AI
 */
function int GetBurstFireCount()
{
	local int AmmoRemaining, BurstCount;

	AmmoRemaining = (GetMagazineSize() == 0) ? 65535 : int(GetMagazineSize() - AmmoUsedCount);
	// Setup the burst fire count if applicable, clamped by how much ammo is left in the clip
	BurstCount = Min( AmmoRemaining, GetRangeValueByPct(AI_BurstFireCount,FRand()) );
	// If no burst is specified then default to the full clip
	if( BurstCount == 0 )
	{
		BurstCount = AmmoRemaining;
	}

	return BurstCount;
}

// Default to firing bursts until ammo is gone
function int GetBurstsToFire()
{
	if(GearAIController != none && GearAIController.DesiredBurstsToFire > 0)
	{
		return GearAIController.DesiredBurstsToFire;
	}
	return -1;
}

/** @return how close AI should get to an enemy before attempting melee
 * this doesn't affect the weapon's rules, just the AI behavior
 * so for correct behavior this should return a value equal or less than GetMeleeAttackRange()
 */
function float GetAIMeleeAttackRange()
{
	return MeleeAttackRange;
}

/** @see Weapon::ShouldRefire() */
simulated function bool ShouldRefire()
{
	local GearPawn GP;

	// If AI controlled...
	if( AIController != None )
	{
		// If AI doesn't want to refire - fail
		if( !AIController.CanFireWeapon( self, CurrentFireMode ) )
		{
			StopFire(CurrentFireMode);
			return FALSE;
		}
		// If AI is burst firing weapon and fired a full burst - fail
		if( RemainingBurstFireCount == 0 )
		{
			return FALSE;
		}
	}

	// If Pawn is not allowed to fire, then abort firing.
	GP = GearPawn(Instigator);
	if( GP != None )
	{
		if( !GP.CanFireWeapon() || GP.ShouldDelayFiring() )
		{
			return FALSE;
		}
	}

	// If melee attack is pending, then abort firing.
	if( PendingFire(MELEE_ATTACK_FIREMODE) )
	{
		return FALSE;
	}

	return Super.ShouldRefire();
}

/**
 *	Called from RefireCheckTimer when weapon should not refire
 *	Give weapon/AI a chance to do something other than return to the Active state
 */
simulated function HandleFinishedFiring()
{
	local float RefireDelay;

	// If AI controlled...
	if( AIController != None )
	{
		if(RemainingBurstFireCount < 1)
		{
			// Reset the burst fire count
			RemainingBurstFireCount = GetBurstFireCount();

			// If firing in bursts...
			if( RemainingBurstsToFire > 0 )
			{

				// Reduce remaining bursts
				RemainingBurstsToFire--;
				if( RemainingBurstsToFire > 0 )
				{
					// Clear refire check until we restart it again
					ClearTimer( 'RefireCheckTimer' );

					// Set the timer to check for the next burst
					RefireDelay = FMax( 0.01f, GetRangeValueByPct( AI_BurstFireDelay, FRand() ) );
					SetTimer( RefireDelay, FALSE, nameof(NotifyWeaponRefireDelayExpired) );
					return;
				}
				else
				{
					StopFire( CurrentFireMode );
					AIController.StopFiring();
				}
			}
		}
	}

	super.HandleFinishedFiring();
}


simulated function FireAmmunition()
{
	Super.FireAmmunition();
	GoalBarrelHeat += BarrelHeatPerShot;
	GoalBarrelHeat = FMin(GoalBarrelHeat, MaxBarrelHeat);
}


/**
 *	Called from FireAmmunition (usually for a single shot)
 *	Updates AI burst logic
 */
function NotifyWeaponFired( byte FireMode )
{
	if( AIController != None )
	{
		// If firing in bursts...
		if( RemainingBurstFireCount > 0 )
		{
			// Reduce the number of shots in the current burst
			RemainingBurstFireCount--;
		}
	}

	Super.NotifyWeaponFired( FireMode );
}

/**
 *	Called when AI refire delay (used to handle bursts) expires
 */
function NotifyWeaponRefireDelayExpired()
{
	if( ShouldRefire() )
	{
		// Start firing again
		FireAmmunition();
		TimeWeaponFiring( CurrentFireMode );
	}
	else
	{
		// Reset the timer to try again
		SetTimer( 0.2f, FALSE, nameof(NotifyWeaponRefireDelayExpired) );
	}
}

/** @return whether the this weapon can hit the given target */
function bool CanHit(vector ViewPt, vector TestLocation, rotator ViewRotation)
{
	return ( GearAIController != none
		&& GearAIController.CanSeeByPoints(ViewPt, TestLocation, ViewRotation)
		&& !GearAIController.IsBlockedByDeployedShield(ViewPt, TestLocation) );
}

function SetHoseOffset()
{
	AI_PreviousHoseOffset		= AI_HoseDesiredOffset;

	AI_HoseDesiredOffset.Pitch  = RandRange( AI_AccCone_Min.X, AI_AccCone_Max.X ) * 182.044;
	AI_HoseDesiredOffset.Yaw	= RandRange( AI_AccCone_Min.Y, AI_AccCone_Max.Y ) * 182.044;
	AI_HoseDesiredOffset.Pitch *= (0.5 - FRand());
	AI_HoseDesiredOffset.Yaw   *= (0.5 - FRand());

	AI_HoseInterpSetTime	= WorldInfo.TimeSeconds;
	AI_HoseInterpFinishTime = WorldInfo.TimeSeconds + RandRange( AI_HoseInterpTimeRange.X, AI_HoseInterpTimeRange.Y );
}

event Rotator GetHoseOffset()
{
	local Rotator Offset;
	local float InterpPct, InterpTime;

	InterpTime = AI_HoseInterpFinishTime - AI_HoseInterpSetTime;
	if( InterpTime > 0.f )
	{
		InterpPct = FClamp( (WorldInfo.TimeSeconds - AI_HoseInterpSetTime) / InterpTime, 0.f, 1.f );
	}

	if( InterpPct == 1.f )
	{
		SetHoseOffset();
		InterpPct = 0.f;
	}

	Offset.Yaw	 = GetRangeValueByPct( vect2d(AI_PreviousHoseOffset.Yaw,   AI_HoseDesiredOffset.Yaw),	InterpPct );
	Offset.Pitch = GetRangeValueByPct( vect2d(AI_PreviousHoseOffset.Pitch, AI_HoseDesiredOffset.Pitch), InterpPct );

	return Offset;
}

/************************************************************************************
 * Weapon Reloading
 ***********************************************************************************/

/**
 * Called by Player to force current weapon to be reloaded
 * Network: LocalPlayer
 */

simulated function ForceReload()
{
	// Make sure it's called for a locally controlled pawn.
	if( !Instigator.IsLocallyControlled() )
	{
		return;
	}

	// if we can reload,
	if( CanReload() )
	{
		StartFire(RELOAD_FIREMODE);

		// Trigger the reload delegates in the GearPC
		FireReloadDelegates();
	}
}

simulated function FireReloadDelegates()
{
	local GearPC MyGearPC;

	if ( (Instigator != None) && (Instigator.Controller != None) )
	{
		MyGearPC = GearPC(Instigator.Controller);
		if ( MyGearPC != None )
		{
			MyGearPC.TriggerGearEventDelegates( eGED_Reload );
		}
	}
}

function DropFrom(vector StartLocation, vector StartVelocity)
{
	local GearDroppedPickup DP;
	local bool bForceDestroy;
	local GearPawn	MyGearPawn;
	local GearGame GG;
//	local vector InitialMeshLoc;
//	local rotator InitialMeshRot;

	if( !CanThrow() )
	{
		return;
	}

	// if the weapon has no ammo, just destroy it
	if( !HasAmmo(0) && !HasSpareAmmo() )
	{
		bForceDestroy = TRUE;
	}

	// If we're not allowed to drop the weapon, just destroy it.
	MyGearPawn = GearPawn(Instigator);
	GG = GearGame(WorldInfo.Game);
	if( MyGearPawn == None || !MyGearPawn.bAllowInventoryDrops || GG == None || !GG.CanDropWeapon(Self, Instigator.Location) )
	{
		bForceDestroy = TRUE;
	}

	// turn off infinite ammo for AI dropped weapons
	if( bInfiniteSpareAmmo && Instigator != None && GearAI(Instigator.Controller) != None )
	{
		bInfiniteSpareAmmo = FALSE;
		SpareAmmoCount = SpareAmmoCount * RandRange(0.4f, 0.8f);
	}

	// check to see if an AI dropped then and then remove some of the ammo so we are
	// not always giving 100% ammo
	if( GearAI(Instigator.Controller) != None && GearAI_TDM(Instigator.Controller) == None )
	{
		SpareAmmoCount = GetDropFromAIAmmoCount();
	}

	if( !bForceDestroy && DroppedPickupMesh != None && SkeletalMeshComponent(DroppedPickupMesh).PhysicsAsset == None )
	{
		`log( GetFuncName() @ Self @ "has no physics asset! doing a non physics drop...");
		Super.DropFrom(StartLocation,StartVelocity);
		return;
	}

//	InitialMeshRot = Mesh.GetRotation();
//	InitialMeshLoc = Mesh.GetPosition();

	GotoState('Inactive');

	StopMuzzleFlashEffect();
	// somebody more knowledgeable about how this should work should figure out the actual problem :)
	ForceEndFire();
	// pseudo-hack: for whatever reason the host is not calling StopFireEffects via ForceEndFire(), so we add a redundant call
	StopFireEffects(CurrentFireMode);
	// Detach weapon components from instigator
	DetachWeapon();

	// remove it from our inventory
	if( Instigator != None && Instigator.InvManager != None )
	{
		Instigator.InvManager.RemoveFromInventory(self);
	}

	// if cannot spawn a pickup, then destroy and quit
	if( bForceDestroy || DroppedPickupClass == None || DroppedPickupMesh == None )
	{
		Instigator = None;
		AIController = None;
		Destroy();
		return;
	}

	DP = GearDroppedPickup(Spawn(DroppedPickupClass,,, StartLocation));
	if( DP == None )
	{
		Instigator = None;
		AIController = None;
		Destroy();
		return;
	}

	// the dropped pickup mesh is usually the same as the held mesh
	// so make sure to get rid of any mirroring
	if( DroppedPickupMesh != None )
	{
		DroppedPickupMesh.SetScale3D(default.DroppedPickupMesh.Scale3D);
	}

	DP.Inventory	= self;
	DP.InventoryClass = (DroppedWeaponClass != None) ? DroppedWeaponClass : Class;
	DP.Instigator = Instigator;
	DP.SetPickupMesh(DP.InventoryClass.default.DroppedPickupMesh);
	DP.CollisionComponent.SetRBAngularVelocity(VRand() * RandRange(-WeaponDropAngularVelocity,WeaponDropAngularVelocity),TRUE);
	DP.CollisionComponent.SetRBLinearVelocity(StartVelocity);

	//`log( "WorldInfo.GRI.IsMultiPlayerGame() == TRUE" @ GearPawn(Instigator).GetMPWeaponColorBasedOnClass() @ GearPawn(Instigator) @ GearPawn(Owner) @ Owner );
	DP.SetEmissiveColor( GearPawn(Instigator).GetMPWeaponColorBasedOnClass() );

	DP.CheckForWeaponFadeOut();

	// align pickup mesh with current real mesh pos, for continuity
	// @fixme, why isn't this working?
	//	DP.CollisionComponent.SetRBPosition(InitialMeshLoc);
	//	DP.CollisionComponent.SetRBRotation(InitialMeshRot);

	Instigator = None;
	AIController = None;
}

function int GetDropFromAIAmmoCount()
{
	// this is the actual number of bullets you get back
	return RandRange( AmmoFromDrop.X,AmmoFromDrop.Y );
}

/**
 * Returns true if weapon can potentially be reloaded
 *
 * @return	true if weapon can be reloaded
 */
simulated function bool CanReload()
{
	local GearPawn	P;
	local bool		bInstigatorCanReload;

	P = GearPawn(Instigator);

	// Cannot reload weapon when doing a special move
	// as this conflicts with the body stance animation
	bInstigatorCanReload = (P != None && P.CanReloadWeapon());

	return(	bWeaponCanBeReloaded	&&	// Weapon can be reloaded
			bInstigatorCanReload	&&	// Instigator can reload weapon
			AmmoUsedCount > 0		&&	// we fired at least a shot
			HasSpareAmmo()				// and we have more ammo to fill current magazine
			);
}


/**
 * returns true if the weapon's magazine is empty and needs to be reloaded
 *
 * @return	true if weapon needs to reload
 */
simulated function bool ShouldAutoReload()
{
	// If AI controlled...
	if( AIController != None )
	{
		// If can't reload - fail
		if( !CanReload() )
		{
			return FALSE;
		}
		// If AI doesn't want to reload - fail
		if( GearAIController != None && !GearAIController.ShouldAutoReload() )
		{
			return FALSE;
		}
		// If clip is full or no ammo to reload with - fail
		if( AmmoUsedCount == 0 || !HasAnyAmmo() )
		{
			return FALSE;
		}

		// If we have ammo in our clip
		if( GetMagazineSize() - AmmoUsedCount > 0 )
		{
			if( GearAIController != None &&
				GearAIController.MyGearPawn != None &&
				GearAIController.MyGearPawn.CoverType != CT_None &&
				GearAIController.MyGearPawn.CoverAction == CA_Default)
			{
				// increase the chance the more ammo we've used
				return FRand() < (AmmoUsedCount/GetMagazineSize());
			}
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		return CanReload() && GetMagazineSize() > 0 && AmmoUsedCount >= GetMagazineSize();
	}
}


/** Is the weapon currently being reloaded? */
simulated function bool IsReloading()
{
	local GearPawn InstigatorGP;
	InstigatorGP = GearPawn(Instigator);
	return (InstigatorGP != None) && InstigatorGP.IsReloadingWeapon();
}


/**
 * Returns duration in seconds of reloading sequence.
 * This doesn't take in account active reloading.
 */
simulated function float GetReloadDuration()
{
	return ReloadDuration;
}


/**
 * Play weapon reloading effects.
 * Called when weapon enters reloading state.
 * Network: ALL
 */
simulated function PlayWeaponReloading()
{
	local GearPawn	GP;
	local Pawn		P;
	local float		ReloadTime;

	GP = GearPawn(Instigator);
	P = (Instigator);

	//`log(GetFuncName()@self@GetStateName());
	if( P == None )
	{
		`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Ouch no Pawn, so can't play animation!");
		EndOfReloadTimer();
		return;
	}

	// Tell pawn to play weapon reload animation
	if( GP != none )
	{
		ReloadTime = PlayPawnReloadingAnimation(GP);
	}

	if( ReloadTime > 0.f )
	{
		ReloadDuration = ReloadTime;
	}

	if( ReloadDuration > 0.f )
	{
		SetTimer( ReloadDuration, FALSE, nameof(EndOfReloadTimer) );
	}
	else
	{
		`warn(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "ReloadDuration is zero!! Triggering EndOfReload now!! Instigator:" @ Instigator @ "ReloadTime:" @ ReloadTime @ "ReloadDuration:" @ ReloadDuration);
		EndOfReloadTimer();
	}

	// Have weapon mesh play a synchronized reload animation as well
	if( WeaponReloadAnim != '' )
	{
		PlayWeaponAnimByDuration(WeaponReloadAnim, ReloadDuration);
	}

	// Play weapon reloading sound
	if( WeaponReloadSound != None && WorldInfo.NetMode != NM_DedicatedServer )
	{
		Instigator.PlaySound(WeaponReloadSound, true);
	}

	// only start the mini game on the client (active reload is client authoritative)
	if( P.IsLocallyControlled() )
	{
		`LogInfo( 'Gear_MiniGames', "Trying to start a StartAReloadMiniGame" );
		StartActiveReload();
	}

	// turn on after fire smoke effect
	if( PSC_ReloadBarrelSmoke != None && Instigator.IsHumanControlled() )
	{
		if( GP == None || TimeSince(GP.LastWeaponEndFireTime) < 2.f )
		{
			ClearTimer( 'PSC_ReloadBarrelSmoke' );
			PSC_ReloadBarrelSmoke.SetHidden( FALSE );
			PSC_ReloadBarrelSmoke.ActivateSystem();
			SetTimer( 3.0f, FALSE, nameof(HidePSC_ReloadBarrelSmoke) );
		}
	}
}

simulated function float PlayPawnReloadingAnimation(GearPawn GP)
{
	local FLOAT ReloadAnimTime;
	local INT	i;

	// Make sure Group starts from begining.
	GP.AnimTreeRootNode.ForceGroupRelativePosition('WeaponReload', 0.f);
	ReloadAnimTime = GP.BS_Play(BS_PawnWeaponReload, 1.f, 0.25f, 0.33f, FALSE, TRUE, 'WeaponReload');
	GP.BS_SetEarlyAnimEndNotify(BS_PawnWeaponReload, FALSE);

	// uh oh problem
	if( ReloadAnimTime == 0.f || !GP.BS_IsPlaying(BS_PawnWeaponReload) )
	{
		`DLog("Failed to play reload animation for" @ GP @ "ReloadAnimTime:" @ ReloadAnimTime @ "IsPlaying?" @ GP.BS_IsPlaying(BS_PawnWeaponReload) @ "AnimTree:" @ GP.Mesh.Animations @ "Template:" @ GP.Mesh.AnimTreeTemplate @ "FullBodyNode:" @ GP.FullBodyNode);
		for(i=0; i<BS_PawnWeaponReload.AnimName.Length; i++)
		{
			if( BS_PawnWeaponReload.AnimName[i] != '' )
			{
				`Log(" ["$i$"]" @ BS_PawnWeaponReload.AnimName[i] @ GP.BodyStanceNodes[i]);
			}
		}
	}

	return ReloadAnimTime;
}


/** needed to be visible at the GearWeapon scope.  Only ever used in state Reloading **/
simulated function EndOfReloadTimer()
{
	// End reload animations.
	EndReloadAnimations();
}

/** End Reload Animations */
simulated function EndReloadAnimations()
{
	local GearPawn	P;

	// Make sure below is set in case notifies couldn't be played
	// (doing a special move while reloading)
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// Show Ammo part on weapon mesh.
		SetWeaponAmmoBoneDisplay(TRUE);

		// Detach magazine mesh from instigator's hand
		SetPawnAmmoAttachment(FALSE);
	}

	// Stop weapon animations
	StopWeaponAnim(0.33f);

	P = GearPawn(Instigator);
	if( P == None )
	{
		return;
	}

	// Stop pawn reload animations
	P.BS_Stop(BS_PawnWeaponReload, 0.33f);
	P.BS_Stop(BS_PawnWeaponReloadSuccess, 0.33f);
	P.BS_Stop(BS_PawnWeaponReloadFail, 0.33f);
}

/** Put down current weapon. */
simulated function PutDownWeapon()
{
	// When putting down weapon, abort reload if reload was still in progress
	if( IsTimerActive('EndOfReloadTimer') )
	{
		AbortWeaponReload();
	}

	Super.PutDownWeapon();
}


/**
 * Function called to abort weapon reload.
 * Can be called when starting a SpecialMove, in that case it is called on all clients.
 * Animation is stopped before special moves starts.
 * Reloading State also sends the weapon to the active state.
 */
simulated function AbortWeaponReload()
{
	if( (CurrentFireMode == FIREMODE_ACTIVERELOADSUPERSUCCESS) || (CurrentFireMode == FIREMODE_ACTIVERELOADSUCCESS) )
	{
		// we've successfully hit an AR and are in the process of finishing it up.
		// in this case, we will still award the ammo
		PerformReload();
	}

	// Clear reload timer.
	ClearTimer('EndOfReloadTimer');

	// End reload animations
	EndReloadAnimations();
}


/** Active Reload Super Success. This is the "sweet spot" for the best reward. */
simulated function PlayActiveReloadSuperSuccess()
{
	local GearPawn	GP;
	local Pawn P;
	local float		SpeedUpPercent, NewTimerRate;

	GP = GearPawn(Instigator);
	P = (Instigator);
	// Need a Pawn doing a reload to proceed
	if( P == None || !IsTimerActive('EndOfReloadTimer') )
	{
		return;
	}

	// speed up by 250%
	SpeedUpPercent = 2.50f;
	if( GP != None )
	{
		// If we have a player reload success animation, play it
		if( BS_PawnWeaponReloadSuccess.AnimName.Length > 0 )
		{
			// Override normal reload animation with success one
			GP.BS_Override(BS_PawnWeaponReloadSuccess);
			NewTimerRate = GP.BS_GetTimeLeft(BS_PawnWeaponReloadSuccess);
		}
		// otherwise, scale up default reload animation
		else
		{
			// Scale player reload animation
			GP.BS_ScalePlayRate(BS_PawnWeaponReload, SpeedUpPercent);
			NewTimerRate = GP.BS_GetTimeLeft(BS_PawnWeaponReload);
		}
	}

	// Adjust end of reload timer based on new animation
	if( NewTimerRate > 0.f )
	{
		SetTimer( NewTimerRate, FALSE, nameof(EndOfReloadTimer) );
	}
	else
	{
		`warn(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Instigator:" @ Instigator @ "NewTimerRate:" @ NewTimerRate @ "!!! Calling EndOfReloadTimer() right away.");
		EndOfReloadTimer();
	}

	// Now, do the same for the weapon animation
	if( WeaponReloadAnimSuccess != '' )
	{
		// Set animation on weapon.
		SetWeaponAnim(WeaponReloadAnimSuccess);
	}
	else
	{
		// Scale weapon reload animation
		ScaleWeaponAnimPlayRate(SpeedUpPercent);
	}
}


/** Active Reload Success. Not the best reward, but still... */
simulated function PlayActiveReloadSuccess()
{
	local GearPawn	GP;
	local Pawn      P;
	local float		SpeedUpPercent, NewTimerRate;

	GP = GearPawn(Instigator);
	P = (Instigator);
	// Need a Pawn doing a reload to proceed
	if( P == None || !IsTimerActive('EndOfReloadTimer') )
	{
		return;
	}

	// speed up by 166%
	SpeedUpPercent = 1.66f;
	if( GP != None )
	{
		// If we have a player reload success animation, play it
		if( BS_PawnWeaponReloadSuccess.AnimName.Length > 0 )
		{
			// Override normal reload animation with success one
			GP.BS_Override(BS_PawnWeaponReloadSuccess);
			NewTimerRate = GP.BS_GetTimeLeft(BS_PawnWeaponReloadSuccess);
		}
		// otherwise, scale up default reload animation
		else
		{
			// Scale player reload animation
			GP.BS_ScalePlayRate(BS_PawnWeaponReload, SpeedUpPercent);
			NewTimerRate = GP.BS_GetTimeLeft(BS_PawnWeaponReload);
		}
	}

	// Adjust end of reload timer based on new animation
	if( NewTimerRate > 0.f )
	{
		SetTimer( NewTimerRate, FALSE, nameof(EndOfReloadTimer) );
	}
	else
	{
		`warn(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Instigator:" @ Instigator @ "NewTimerRate:" @ NewTimerRate @ "!!! Calling EndOfReloadTimer() right away.");
		EndOfReloadTimer();
	}

	// Now, do the same for the weapon animation
	if( WeaponReloadAnimSuccess != '' )
	{
		// Set animation on weapon.
		SetWeaponAnim(WeaponReloadAnimSuccess);
	}
	else
	{
		// Scale weapon reload animation
		ScaleWeaponAnimPlayRate(SpeedUpPercent);
	}
}


/** Active Reload failed, ouch... penalty follows. */
simulated function PlayActiveReloadFailed()
{
	local GearPawn	GP;

	GP = GearPawn(Instigator);
	// Need a Pawn doing a reload to proceed
	if( Instigator == None || !IsTimerActive('EndOfReloadTimer') )
	{
		return;
	}

	PlayPawnActiveReloadFailAnimation(GP);

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// Play the failed animation on the weapon
		if( WeaponReloadAnimFail != '' )
		{
			SetWeaponAnim(WeaponReloadAnimFail);
		}

		// Play facial Angry animation
		Instigator.PlayActorFaceFXAnim(None, "Emotions", "Angry", None);
	}
}

simulated function PlayPawnActiveReloadFailAnimation(GearPawn GP)
{
	local float NewTimerRate;

	// Play active reload failed animation
	if( BS_PawnWeaponReloadFail.AnimName.Length > 0 && GP != None && GP.BS_IsPlaying(BS_PawnWeaponReload) )
	{
		GP.BS_Override(BS_PawnWeaponReloadFail);
		NewTimerRate = GP.BS_GetTimeLeft(BS_PawnWeaponReloadFail);

		// Adjust end of reload timer based on new animation
		if( NewTimerRate > 0.f )
		{
			SetTimer( NewTimerRate, FALSE, nameof(EndOfReloadTimer) );
		}
		else
		{
			`warn(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Instigator:" @ Instigator @ "NewTimerRate:" @ NewTimerRate @ "!!! Calling EndOfReloadTimer() right away.");
			EndOfReloadTimer();
		}
	}
	else
	{
		`warn(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Instigator:" @ Instigator @ "BS_PawnWeaponReloadFail.AnimName.Length:" @ BS_PawnWeaponReloadFail.AnimName.Length @ "GP.BS_IsPlaying(BS_PawnWeaponReload):" @ GP.BS_IsPlaying(BS_PawnWeaponReload) @ "!!! Calling EndOfReloadTimer() right away.");
		EndOfReloadTimer();
	}
}

/** Performs actual ammo reloading */
simulated function PerformReload()
{
	local float	Amount;

	// reload weapon here
	if( HasInfiniteSpareAmmo() )
	{
		AmmoUsedCount = 0;
	}
	else if( GetMaxSpareAmmoSize() > 0 && SpareAmmoCount > 0 )
	{
		Amount			= SpareAmmoCount - FMax(SpareAmmoCount - AmmoUsedCount, 0);
		AmmoUsedCount	= AmmoUsedCount - Amount;
		SpareAmmoCount	= SpareAmmoCount - Amount;
	}
	// right now some config values are not being read in correctly.
	// so if we ever get a MaxSpareAmmoSize of 0 we just reload it
	// this is what is occurring with grenades atm
	else if( GetMaxSpareAmmoSize() == 0 )
	{
		`log( "something terrible has gone wrong with the config loading of MaxSpareAmmo " );

		Amount			= SpareAmmoCount - FMax(SpareAmmoCount - AmmoUsedCount, 0);
		AmmoUsedCount	= AmmoUsedCount - Amount;
		SpareAmmoCount	= SpareAmmoCount - Amount;
	}
}


/**
 * State Reloading
 * State the weapon is in when it is being reloaded (current magazine replaced with a new one, related animations and effects played).
 */
simulated state Reloading
{
	ignores ForceReload;

	simulated function BeginState(Name PreviousStateName)
	{
		local GearAI AI;
		local float PreReactionWindowDuration, ARStartTime, SuperSweetSpotDuration, SweetSpotDuration;
		local bool bSweet, bJam;

		`LogInv("PreviousStateName:" @ PreviousStateName);

		ReloadStartTime = WorldInfo.TimeSeconds;

		bActiveReloadAttempted = FALSE;

		// animation is played when receiving the updated firing mode.
		SetCurrentFireMode(RELOAD_FIREMODE);

		// tell guds
		if (Role == ROLE_Authority && Instigator.Controller.IsInCombat())
		{
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_Reloaded, Instigator);
		}

		AI = GearAI(Instigator.Controller);
		if (AI != None && FRand() < AI.ActiveReloadAttemptPct)
		{
			bSweet = (FRand() < AI.ActiveReloadSweetPct);
			bJam   = (FRand() < AI.ActiveReloadJamPct);

			GetActiveReloadValues(ARStartTime, PreReactionWindowDuration, SuperSweetSpotDuration, SweetSpotDuration);
			if (bSweet != bJam)
			{
				if (bSweet)
				{
					SetTimer(ARStartTime, false, nameof(AISuperSweetReload));
				}
				else
				{
					`Log(`location @ "JAM" @ `showobj(Instigator));
					LocalFailedActiveReload(true);
				}
			}
			else if (GearAI_TDM(AI) != None) // bots can also do the faster reload only AR
			{
				SetTimer(ARStartTime + SuperSweetSpotDuration + SweetSpotDuration, false, nameof(AISweetReload));
			}
		}
	}

	/** called on a timer when AI decides it should succeed AR */
	function AISweetReload()
	{
		LocalActiveReloadSuccess(false);
	}
	function AISuperSweetReload()
	{
		LocalActiveReloadSuccess(true);
	}

	simulated function EndState(Name NextStateName)
	{
		`LogInfo('Gear_MiniGames', "Reloading EndState()");
		`LogInv("NextStateName:" @ NextStateName);

		// Was reload aborted??
		if( IsTimerActive('EndOfReloadTimer') )
		{
			// Call global version, so we're not forced to go to the Active state
			Global.AbortWeaponReload();
			ClearTimer('EndOfReloadTimer');
		}
		ClearTimer(nameof(AISuperSweetReload));
		ClearTimer(nameof(AISweetReload));

		EndFire( RELOAD_FIREMODE );

		// switch back to default firing mode.
		SetCurrentFireMode( 0 );

		// do nothing for now
//		// if we did not attempt an ActiveReload then reduce our AR level by one.
//		if( ( bActiveReloadAttempted == FALSE )
//			&& ( ActiveReload_CurrTier != 0 )
//			)
//		{
//			ActiveReload_CurrTier--;
//		}

		//`log( "ActiveReload_CurrTier: " $ ActiveReload_CurrTier );
	}

	/** @see Weapon::TryPutDown() */
	simulated function bool TryPutDown()
	{
		// Put the weapon down right away, and abort weapon reloading
		PutDownWeapon();
		return TRUE;
	}

	/** Abort weapon reload... */
	simulated function AbortWeaponReload()
	{
		Global.AbortWeaponReload();

		// leave state and go back to active without reloading weapon
		GotoState('Active');
	}

	simulated function EndOfReloadTimer()
	{
		Global.EndOfReloadTimer();

		ClearTimer('EndOfReloadTimer');

		// Reload weapon
		PerformReload();

		// we're done, leave state and go back to active
		GotoState('Active');
	}


	//// ActiveReload Functionality

	simulated function bool HandleButtonPress( coerce Name ButtonName )
	{
		// check to see if we have pressed the reload button
		if( ( bActiveReloadAttempted == FALSE )
			&& ( ButtonName == 'R1' )
			)
		{
			DetermineActiveReloadSuccessOrFailure();
			bActiveReloadAttempted = TRUE;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	simulated function BeginFire(byte FireModeNum)
	{
		Global.BeginFire(FireModeNum);

		// Melee aborts weapon reloads
		if( FireModeNum == MELEE_ATTACK_FIREMODE && PendingFire(MELEE_ATTACK_FIREMODE) )
		{
			if( IsTimerActive('EndOfReloadTimer') )
			{
				// Call global, because we don't want to go to the active state
				Global.AbortWeaponReload();
			}
			SendToFiringState(FireModeNum);
		}
	}

	simulated function bool IsFiring()
	{
		return FALSE;
	}

}


/**
 * This function will determine if the player has pressed the reload button fast enough to get
 * an AR success.  Or if they have failed.	This function then sends events back up to the Weapon
 * to tell it what has occurred.
 *
 **/
native function DetermineActiveReloadSuccessOrFailure();

native function GetActiveReloadValues(out float ActiveReloadStartTime, out float PreReactionWindowDuration, out float SuperSweetSpotDuration, out float SweetSpotDuration);

/**
 * Resets the ActiveReload variables in the case of a failure
 * @Network: Local Player.
 */
simulated event LocalFailedActiveReload( bool bFailedActiveReload )
{
	if( Role < Role_Authority )
	{
		ServerFailedActiveReload( bFailedActiveReload );
	}

	FailedActiveReload( bFailedActiveReload );

	// play the failure sound and show failure message
	if (bFailedActiveReload == TRUE)
	{
		GearWeaponPlaySoundLocal(AR_FailSound,, TRUE);
	}
}


/**
 * Event called when player failed to perform active reload.
 * @Network: Dedicated Server, or Listen Server.
 */
unreliable server function ServerFailedActiveReload( bool bFailedActiveReload )
{
	FailedActiveReload( bFailedActiveReload );
}


/**
 * Event called when player failed to perform active reload.
 * @param bFailedActiveReload - TRUE if the attempted the AR and failed.  FALSE if they did not attempt the AR
 * @Network: ALL
 */
simulated function FailedActiveReload(bool bFailedActiveReload)
{
	local GearPawn	GP;

	GP = GearPawn(Instigator);
	if ( (GP == None) && (Vehicle(Instigator) != None) )
	{
		GP = GearPawn(Vehicle(Instigator).Driver);
	}

	//debug
	`LogInfo( 'Gear_ActiveReload', "FailedActiveReload() bFailedActiveReload:" @ bFailedActiveReload );

	ActiveReload_NumBonusShots	= 0;
	if(GP != none)
	{
		GP.ActiveReload_CurrTier		= 0;
		GP.bActiveReloadBonusActive	= FALSE;
	}

	// if we actually failed not because time expired, play failed animation
	if( bFailedActiveReload == TRUE && ShouldPlayFailedActiveReload() )
	{
		SetCurrentFireMode(FIREMODE_FAILEDACTIVERELOAD);
	}

	// Notify Pawn
	if (GP != none)
	{
		GP.ActiveReloadFail(bFailedActiveReload);
	}

}

/** Have a chance to not play the active reload failed animation */
simulated function bool ShouldPlayFailedActiveReload()
{
	return TRUE;
}

/**
 * @Network: Local Player.
 */
simulated event LocalActiveReloadSuccess(bool bDidSuperSweetReload)
{
	local GearPC PC;
	//local SoundCue SoundToPlay;

	`LogInfo( 'Gear_ActiveReload', "LocalActiveReloadSuccess bDidSuperSweetReload:" @ bDidSuperSweetReload );

	if( Role < Role_Authority )
	{
		ServerActiveReloadSuccess(bDidSuperSweetReload);
	}


	PC = GearPC(Pawn(Owner).Controller);
	if( PC != None )
	{
		if( bDidSuperSweetReload == TRUE )
		{
			PC.ClientPlayForceFeedbackWaveform( class'GearWaveForms'.default.ActiveReloadSuperSuccess );
		}
		else
		{
			PC.ClientPlayForceFeedbackWaveform( class'GearWaveForms'.default.ActiveReloadSuccess );
		}
	}

	ActiveReloadSuccess( bDidSuperSweetReload );
}


/**
 * Event called when player succeeded to perform active reload.
 * @Network: Dedicated Server, or Listen Server.
 */
unreliable server function ServerActiveReloadSuccess(bool bDidSuperSweetReload)
{
	ActiveReloadSuccess(bDidSuperSweetReload);
}


/**
 * Event called when player succeeded to perform active reload.
 * @Network: ALL
 */
simulated function ActiveReloadSuccess(bool bDidSuperSweetReload)
{
	local GearPawn GP;
	`LogInfo( 'Gear_ActiveReload', "ActiveReloadSuccess bDidSuperSweetReload:" @ bDidSuperSweetReload );

	GP = GearPawn(Instigator);
	if ( (GP == None) && (Vehicle(Instigator) != None) )
	{
		GP = GearPawn(Vehicle(Instigator).Driver);
	}
	if (GP != none)
	{
		GP.ActiveReloadSuccess(bDidSuperSweetReload);
	}

	SetActiveReloadBonusActive(bDidSuperSweetReload);

	if( bDidSuperSweetReload )
	{
		SetCurrentFireMode(FIREMODE_ACTIVERELOADSUPERSUCCESS);
	}
	else
	{
		SetCurrentFireMode(FIREMODE_ACTIVERELOADSUCCESS);
	}
}


/**
 * This will start a reload mini game for the weapon reloading.
 * Additionally, this function will either be overridden or
 * will use the default properties for each weapon TYPE.
 */
simulated function StartActiveReload()
{
	local GearPC PC;

	AR_TimeReloadButtonWasPressed = WorldInfo.TimeSeconds;

	// reset the hud for the active reload
	if (Instigator != None)
	{
		// tell the hud to reset the active reload data
		PC = GearPC(Instigator.Controller);
		if ( (PC != None) && (PC.myHUD != None) )
		{
			GearHUD_Base(PC.myHUD).ResetActiveReload();
		}
	}
}


/**
* Turn on the visual effects from an active reload
**/
simulated function TurnOnActiveReloadBonus_VisualEffects()
{
	local Vector Scalar;
	local GearPawn GP;
	local int CurTier;

	`LogInfo( 'Gear_ActiveReload', "TurnOnActiveReloadBonus()" );

	GP = GearPawn(Owner);
	if(GP != none)
	{
		CurTier = GP.ActiveReload_CurrTier;
	}
	else
	{
		CurTier = 0;
	}

	if( MuzFlashEmitter != None )
	{
		if( MuzFlashParticleSystemActiveReload != None )
		{
			MuzFlashEmitter.SetTemplate( MuzFlashParticleSystemActiveReload );
			MuzFlashEmitter.SetLODLevel( GearGRI(WorldInfo.GRI).GetLODLevelToUse(MuzFlashParticleSystemActiveReload, GetMuzzleLoc()) );
		}
		else
		{
			// scale based on which tier of active reload we are in
			Scalar.X = 1.0f + (CurTier * 0.05f);
			Scalar.Y = 1.0f + (CurTier * 0.05f);
			Scalar.Z = 1.0f + (CurTier * 0.05f);
			MuzFlashEmitter.SetScale3D( default.MuzFlashEmitter.Scale3D * Scalar * 2.0 );
		}
	}

	if( MuzzleFlashLight != None )
	{
		// scale based on which tier of active reload we are in
		MuzzleFlashLight.Radius = default.MuzzleFlashLight.Radius * (1.0f + (CurTier * 0.05f));
		MuzzleFlashLight.SetLightProperties( default.MuzzleFlashLight.Brightness * (1.0f + (CurTier * 0.05f)), GetWeaponMuzzleFlashActiveReload() );
	}

	bActiveReloadTracerVisualEffectsActive = TRUE;
}


/**
* Turn off the visual effects from an active reload
**/
simulated function TurnOffActiveReloadBonus_VisualEffects()
{
	`LogInfo( 'Gear_ActiveReload', "TurnOffActiveReloadBonus()" );

	// Reset the scale of the various muzzle flashy stuff
	if( MuzFlashEmitter != None )
	{
		if( MuzFlashParticleSystemActiveReload != None )
		{
			MuzFlashEmitter.SetTemplate( MuzFlashParticleSystem );
			MuzFlashEmitter.SetLODLevel( GearGRI(WorldInfo.GRI).GetLODLevelToUse(MuzFlashParticleSystem, GetMuzzleLoc()) );
			//MuzFlashEmitter.ActivateSystem();
		}
		else
		{
			MuzFlashEmitter.SetScale3D( default.MuzFlashEmitter.Scale3D );
		}
	}

	if( MuzzleFlashLight != None )
	{
		// Reset light brightness and radius
		MuzzleFlashLight.SetLightProperties( default.MuzzleFlashLight.Brightness, GetWeaponMuzzleFlashNormal() );
		MuzzleFlashLight.Radius = default.MuzzleFlashLight.Radius;
	}

	bActiveReloadTracerVisualEffectsActive = FALSE;
	if (GearPawn(Owner) != None)
	{
		GearPawn(Owner).ActiveReload_CurrTier = 0;	// for now just turn off the AR tier if dam bonus ends
	}
}


/**
 * This will turn off the active reload bonus
 */
simulated function TurnOffActiveReloadBonus()
{
	`LogInfo( 'Gear_ActiveReload', "TurnOffActiveReloadBonus()" );
	if (GearPawn(Instigator) != None)
	{
		GearPawn(Instigator).bActiveReloadBonusActive = FALSE;
	}
	ActiveReload_NumBonusShots = 0;

	PlaySound( SoundCue'MiniGames.Sounds.ActiveReload_DamageBonusEnd' );
}


/**
 * This will turn on the active reload bonus which will modify:
 *	-damage of hits
 *
 * Additionally, it will set a timer to turn off the active reload bonus.
 */
simulated function SetActiveReloadBonusActive(bool bSetSuperSweetSpotReward)
{
	local GearPawn GP;

	// check to make certain we have used most of the clip before giving
	// out the damage bonus
	if (IsApplicableForARDamageBonus())
	{
		// only go up to the max avail tiers.
		/*
		if( ActiveReload_CurrTier < ActiveReload_BonusDurationTier.length-1 )
		{
			if( !bSetSuperSweetSpotReward )
			{
				// do nothing for a non super sweet reload
				//IncreaseActiveReloadCurrTier();
			}
			// we are in super swweet reload
			else
			{
				ActiveReload_CurrTier = ActiveReload_BonusDurationTier.length-1; // index is zero based while length is 1 based
			}
		}
		*/

		GP = GearPawn(Instigator);

		//`LogInfo( 'Gear_ActiveReload', "PRE SetActiveReloadBonusActive() ActiveReload_CurrTier: " $ GP.ActiveReload_CurrTier@bSetSuperSweetSpotReward );

		// play the correct sound
		if (bSetSuperSweetSpotReward)
		{
			if( GP != none)
			{
				GP.bActiveReloadBonusActive = TRUE;
				if( GP.ActiveReload_CurrTier < ACTIVE_RELOAD_TIERS-1 )
				{
					GP.ActiveReload_CurrTier++;
				}
			}

			// set the number of bonus bullets to the number of bullets reloaded
			ActiveReload_NumBonusShots = ActiveReload_NumBonusShots + SpareAmmoCount - FMax(SpareAmmoCount - AmmoUsedCount, 0);

			// set timer to disable AR bonus
			SetTimer( AR_MagicBulletsTimeoutDuration, FALSE, nameof(TurnOffActiveReloadBonus) );
			SetTimer( AR_MagicBulletsTimeoutDuration, FALSE, nameof(TurnOffActiveReloadBonus_VisualEffects) );

			// turn off the muzzle flash in case it's still active
			// since active reload can replace the muzzle flash and cause
			// a strange particle pop
			if (MuzFlashEmitter != None)
			{
				HideMuzzleFlashEmitter();
			}

			// only turn on visuals for supersweet
			TurnOnActiveReloadBonus_VisualEffects();
			GearWeaponPlaySoundLocal(AR_SuperSuccessSound,, TRUE);
		}
		else
		{

			ActiveReload_NumBonusShots = 0;
			if(GP != none)
			{
				GP.bActiveReloadBonusActive = FALSE;
				GP.ActiveReload_CurrTier = 0;
			}

			GearWeaponPlaySoundLocal(AR_SuccessSound,, TRUE);
		}

		if(GP != none)
		{
			`LogInfo( 'Gear_ActiveReload', "ActiveReload_CurrTier: " $ GP.ActiveReload_CurrTier );
		}


	}
	// no damage bonus given
	else
	{
		`LogDebug( 'Gear_ActiveReload', "Not eligible" );
	}
}


/**
 * This can be over ridden by specific weapons that have different behavior than the norm.
 */
simulated function bool IsApplicableForARDamageBonus()
{
	return (AmmoUsedCount > 0);
}


// Reload Ammo Effect

/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease();
/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease2();

/** notification fired from Pawn animation when player throws ammo */
simulated function Notify_AmmoThrow();

/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab();

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload();
/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload2();

/**
 * Show or Hide weapon ammo bone.
 * Done by using a bone scaling controller.
 */
simulated function SetWeaponAmmoBoneDisplay(bool bShow, optional Name InControlName)
{
	local SkelControlBase		Control;
	local SkeletalMeshComponent	SkelMesh;

	SkelMesh = SkeletalMeshComponent(Mesh);
	if( SkelMesh != None )
	{
		if( InControlName == '' )
		{
			InControlName = 'AmmoControl';
		}

		Control = SkelMesh.FindSkelControl(InControlName);
		if( Control != None )
		{
			Control.SetSkelControlActive( !bShow );
		}
	}
}


/** Attach or Detach MagazineMesh on Pawn's left hand */
simulated function SetPawnAmmoAttachment(bool bAttach, optional StaticMeshComponent InMagazineMesh, optional Name InBoneName, optional Rotator InRelativeRotation)
{
	local SkeletalMeshSocket	Socket;
	local Vector				NewScale3D;

	if( InMagazineMesh == None )
	{
		InMagazineMesh = MagazineMesh;
	}

	if( InMagazineMesh == None )
	{
		return;
	}

	if( bAttach )
	{
		// Set Shadow parent and light environment.
		InMagazineMesh.SetShadowParent(Instigator.Mesh);
		InMagazineMesh.SetLightEnvironment(GearPawn(Instigator).LightEnvironment);

		if( InBoneName != '' )
		{
			Instigator.Mesh.AttachComponent(InMagazineMesh, InBoneName,, InRelativeRotation);
		}
		else
		{
			Socket = Instigator.Mesh.GetSocketByName(GearPawn(Instigator).GetLeftHandSocketName());
			Instigator.Mesh.AttachComponent(InMagazineMesh, Socket.BoneName, Socket.RelativeLocation, Socket.RelativeRotation + InRelativeRotation, Socket.RelativeScale);
		}

		// Mirror weapon
		if( ShouldWeaponBeMirrored(GearPawn(Instigator)) )
		{
			NewScale3D = InMagazineMesh.default.Scale3D;
			NewScale3D.Y = -NewScale3D.Y;
			InMagazineMesh.SetScale3D( NewScale3D );
		}
		else
		{
			InMagazineMesh.SetScale3D(InMagazineMesh.default.Scale3D);
		}
	}
	else
	{
		// Set Shadow parent and light environment.
		InMagazineMesh.SetShadowParent(None);
		InMagazineMesh.SetLightEnvironment(None);

		// detach mesh from instigator
		if( Instigator.Mesh != None && Instigator.Mesh.IsComponentAttached(InMagazineMesh) )
		{
			Instigator.Mesh.DetachComponent(InMagazineMesh);
		}
	}
}


/** Spawn physics Magazine */
simulated function KActorSpawnable SpawnPhysicsMagazine(Vector SpawnLoc, Rotator SpawnRot)
{
	local KActorSpawnable	MagActor;
	local Vector			X, Y, Z;
	local GearGRI			GearGRI;

	if( MagazineMesh == None || Instigator == None || ( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, SpawnLoc, 1024.0f, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE ) )
	{
		return None;
	}

	GearGRI = GearGRI(WorldInfo.GRI);
	// Get a cached KActor from our object pool
	MagActor = GearGRI.GOP.GetKActorSpawnable(SpawnLoc, SpawnRot);

	if( MagActor != None )
	{
		MagActor.StaticMeshComponent.SetStaticMesh(MagazineMesh.StaticMesh);
		MagActor.StaticMeshComponent.WakeRigidBody();
		MagActor.SetCollision(FALSE, FALSE);
		MagActor.StaticMeshComponent.SetPhysMaterialOverride(PhysicalMaterial'WeaponPhysics.Physmats.WeaponPhysMat');
		MagActor.StaticMeshComponent.SetNotifyRigidBodyCollision(true);
		MagActor.SetPhysicalCollisionProperties();
		// Add a small impulse to have the clip slightly spinning
		GetAxes(Instigator.Rotation, X, Y, Z);
		MagActor.ApplyImpulse(Y + X*FRand(), 1.f + 2.f * FRand(), MagActor.Location);

		// Don't replicate this one
		MagActor.RemoteRole = Role_None;

		// Keep around for only 25 seconds.
		MagActor.SetTimer( 25.f, FALSE, nameof(MagActor.Recycle) );
		// turn off updating the DLE after a second
		MagActor.SetTimer( 1.0f, FALSE, nameof(MagActor.SetLightEnvironmentToNotBeDynamic) );
	}

	return MagActor;
}

/** Default Socket this weapon is attached to */
simulated event Name GetDefaultHandSocketName(GearPawn P)
{
	if( P != None )
	{
		return P.GetRightHandSocketName();
	}

	return '';
}

/**
 * Notification that Pawn just mirrored.
 * Swap weapon attachments...
 */
simulated function AdjustWeaponDueToMirror(GearPawn P)
{
	local Name	HandSocketName;

	HandSocketName = GetDefaultHandSocketName(P);
	if( SocketAttachedTo != HandSocketName && P.Mesh.IsComponentAttached(Mesh) )
	{
		// Detach weapon from current hand socket.
		DetachWeapon();

		// reattach weapon, now to inverted hand socket
		AttachWeaponTo(P.Mesh, HandSocketName);
		UpdateMagazineDueToMirror(P);
	}
}

simulated function UpdateMagazineDueToMirror(GearPawn P)
{
	// If player hapened to be in the middle of a weapon reload animation
	// handle case where clip was attached to left hand
	if( MagazineMesh != None && P.Mesh != None && P.Mesh.IsComponentAttached(MagazineMesh) )
	{
		SetPawnAmmoAttachment(FALSE);
		SetPawnAmmoAttachment(TRUE);
	}
}


/************************************************************************************
 * Melee Attacks
 * Code for weapon melee attacks
 ***********************************************************************************/

simulated function bool IsMeleeing()
{
	return FALSE;
}

/**
 * State MeleeAttacking
 * State the weapon is in when doing a melee attack
 */
simulated state MeleeAttacking
{
	simulated function bool IsMeleeing() { return TRUE; }

	simulated function BeginState(Name PreviousStateName)
	{
		//`LogInv("");

		// Change the weapon firing mode to MELEE_ATTACK_FIREMODE
		// This is used to synchronize clients to the new firing mode
		// MeleeAttackStarted() and MeleeAttackEnded() is called on clients based on the replication
		// of that firing mode.
		SetCurrentFireMode(MELEE_ATTACK_FIREMODE);

		// Melee attack is starting!
		MeleeAttackStarted();
	}

	simulated function EndState(Name NextStateName)
	{
		NotifyWeaponFinishedFiring( MELEE_ATTACK_FIREMODE );

		// Switch back to default firing mode.
		// This will call MeleeAttackEnded() on remote clients.
		SetCurrentFireMode(0);
		MeleeAttackEnded();
	}

	simulated function EndOfMeleeAttack()
	{
		// If player is still holding melee button
		if( PendingFire(MELEE_ATTACK_FIREMODE) )
		{
			// Clear flag to avoid going back to this state
			EndFire(MELEE_ATTACK_FIREMODE);
		}

		// we're done, leave state and go back to active
		GotoState('Active');
	}
}

/**
 * Some weapons can do SpecialMeleeAttacks which will end up killing the enemy in a glorious way
 * This function will check to see if that SpecialMeleeAttack is avail.	 (e.g. some might not be able
 * to be activated if you have been shot recently, or if you don't have the magic tokens)
 **/
simulated function bool CanDoSpecialMeleeAttack()
{
	return TRUE;
}


/**
 * Event called when melee attack begins.
 * This means when weapon is entering melee attacking state on local player and server.
 * When Pawn.FiringMode == MELEE_ATTACK_FIREMODE is replicated on remote clients.
 */
simulated function MeleeAttackStarted()
{
	// Forward event to pawn owner
	GearPawn(Instigator).MeleeAttackStarted(Self);

	// Play melee attack animation.
	// This also sets the timer to call EndOfMeleeAttack.
	PlayMeleeAttack();

	// set the timer to check for hits
	if( bUseMeleeHitTimer )
	{
		// Clear complete time so it is intialized properly
		MeleeImpactCompleteTime = -1.f;
		SetTimer( MeleeInitialImpactDelay, FALSE, nameof(CheckMeleeAttackCollision) );
	}
}

/**
 *	Function handles checking for all types of melee impacts
 */
simulated function bool CheckMeleeAttackCollision()
{
	local bool bResult;

	// If this is the start first check of a melee attack
	if( MeleeImpactCompleteTime < 0.f )
	{
		// Update the window of time to repeat checks for impact
		MeleeImpactCompleteTime = WorldInfo.TimeSeconds + MeleeImpactRetryDuration;
	}
	// If window has expired - FAIL
	if( WorldInfo.TimeSeconds > MeleeImpactCompleteTime )
	{
		return FALSE;
	}

	// Check impacts for players first
	bResult = MeleeAttackImpact();
	// If no impact with player occurs
	if( !bResult && Role == ROLE_Authority && !bSlaveWeapon )
	{
		// Check for impact with world/destructibles
		bResult = MeleeAttackDestructibles();
	}

	// If no impact has occured
	if( !bResult )
	{
		// Set timer to retry impact
		SetTimer( 0.1, FALSE, nameof(CheckMeleeAttackCollision) );
	}

	return bResult;
}

/**
 * Event called when melee attack ends.
 * This means when weapon leaves melee attacking state on local player and server.
 * When Pawn.FiringMode != MELEE_ATTACK_FIREMODE is replicated on remote clients, and previous firing mode was MELEE_ATTACK_FIREMODE.
 */
simulated function MeleeAttackEnded()
{
	if (GearPawn(Instigator) != None)
	{
		// Forward event to pawn owner
		GearPawn(Instigator).MeleeAttackEnded(Self);
	}

	bHitWallThisAttack = FALSE;
	NumMeleeAttackAttempts = 0;

	ClearTimer( 'CheckMeleeAttackCollision' );
}

/**
 * Play Melee Attack
 * This function plays the melee attack animation
 * and sets a timer to call EndOfMeleeAttack(), which will trigger this end of the move
 * (and state transition).
 */
simulated function PlayMeleeAttack()
{
	local GearPC		GPC;
	local GearPawn	GP;
	local float		AnimDuration;

	GP = GearPawn(Instigator);
	if( GP == None )
	{
		return;
	}

	// Play melee attack animation.
	AnimDuration = PlayMeleeAttackAnimation(GP);
	TimeStartedSpecialMeleeAttack = WorldInfo.TimeSeconds;

	if( !bSuppressMeleeAttackCameraBoneAnim && MeleeAttackCameraBoneAnims.Length > 0 )
	{
		GPC = GearPC(GP.Controller);
		if (GPC != None)
		{
			GPC.PlayRandomCameraBoneAnim(MeleeAttackCameraBoneAnims,, 0.1f, 0.1f,,, TRUE, TRUE, TRUE);
		}
	}

	// Set Timer to end melee attack state
	if( AnimDuration > 0.f )
	{
		SetTimer( AnimDuration, FALSE, nameof(EndOfMeleeAttack) );
	}
	else
	{
		`Warn("PlayMeleeAttack" @ Self @ Instigator @ "AnimDuration is zero!!, ending melee attack now");
		EndOfMeleeAttack();
	}

	// play melee attack sound
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		GP.SoundGroup.PlayFoleySound(GP, GearFoley_MeleeAttackFX);
	}
}


/**
 * Play melee attack animation.
 * Returns the duration in seconds of the animation.
 */
simulated function float PlayMeleeAttackAnimation(GearPawn P)
{
	local float	AnimTime;
	local int	Idx;

	// Does Pawn want to override the melee animation with his own?
	if( P.OverridePlayMeleeAttackAnim(AnimTime) )
	{
		return AnimTime;
	}

	// pick random attack
	Idx = Rand(BS_MeleeAttack.Length);

	return P.BS_Play(BS_MeleeAttack[Idx], 1.f, 0.1f, 0.25f, FALSE, TRUE);
}


/**
 * Called by timer from PlayMeleeAttack()
 * Overloaded in state to end attack.
 */
simulated function EndOfMeleeAttack();


/** Notification called from Pawn animation, by GearAnimNotify_MeleeImpact. */
simulated function MeleeImpactNotify(GearAnimNotify_MeleeImpact Notify)
{
	if( !bUseMeleeHitTimer && !bMeleeImpactNotifySemaphore )
	{
		// Give damage
		MeleeAttackImpact();
		bMeleeImpactNotifySemaphore = TRUE;
		SetTimer( MeleeAttackCoolDownInSeconds, FALSE, nameof(ClearMeleeImpactNotifySemaphore) );
	}
}

/** This resets the bMeleeImpactNotifySemaphore **/
simulated function ClearMeleeImpactNotifySemaphore()
{
	bMeleeImpactNotifySemaphore = FALSE;
}


/**
 * Returns the location that should be the start of traces for MeleeAttackImpact()
 */
simulated function vector GetMeleeStartTraceLocation()
{
	// we need to do it from the pawn's lower body so we can hit pawns that are "down but not out"
	return Instigator.Location - vect(0,0,32); // GetPhysicalFireStartLoc();
}

/**
 * Separate check for destructible objects, fixes issues with notifies/delays/etc.	Only
 * attack an object once per swing, etc.
 */
function bool MeleeAttackDestructibles()
{
	local Actor HitActor;
	local GearPawn GP;
	local vector StartTrace, EndTrace, HitLoc, HitNorm;
	local TraceHitInfo HitInfo;
	local bool bHitSomething;
	local GearPC PC;
	local Trigger_ChainsawInteraction Trigger;
	local Emitter AnEmitter;
	GP = GearPawn(Instigator);
	StartTrace	= GetMeleeStartTraceLocation();
	EndTrace	= StartTrace + ( vector(GP.GetBaseAimRotation()) * MeleeAttackRange );
	bHitSomething = FALSE;

	// check for normal melee against chainsaw triggers that also accept melee
	foreach GP.TouchingActors(class'Trigger_ChainsawInteraction',Trigger)
	{
		// skip over triggers being used by other players
		if (Trigger.CurrentActivator != None && Trigger.CurrentActivator != Instigator)
		{
			continue;
		}
		if (Trigger.NumberOfMeleeHitsNeeded > 0 && Normal(Trigger.Location - GP.Location) dot vector(GP.Rotation) > 0.2f && (Trigger.TriggerEventClass(class'SeqEvt_ChainsawInteraction',GP,0,TRUE) || Trigger.bNoKismet))
		{
			bHitSomething = TRUE;
			if (Trigger.NumberOfMeleeHitsNeeded == Trigger.default.NumberOfMeleeHitsNeeded)
			{
				Trigger.bActivatedByMelee = TRUE;
				Trigger.TriggerEventClass(class'SeqEvt_ChainsawInteraction',GP,2,FALSE);
				Trigger.CurrentActivator = Instigator;
				Trigger.HitByMelee( GearPawn(Instigator) );
			}
			Trigger.NumberOfMeleeHitsNeeded--;
			if (Trigger.NumberOfMeleeHitsNeeded <= 0)
			{
				Trigger.TriggerEventClass(class'SeqEvt_ChainsawInteraction',GP,3,FALSE);
				Trigger.CurrentActivator = None;
				Trigger.TriggerEndChainsaw(GP);
			}
			AnEmitter	= GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'COG_Assaultrifle.Effects.P_COG_AssaultRifle_Impact_Melee', Trigger.Location, rotator(Instigator.Location - Trigger.Location) );
			AnEmitter.ParticleSystemComponent.ActivateSystem();

			// Sound
			WeaponPlaySound(MeleeImpactSound); // hit wall
			PlayMeleeScreenShake(None);
			break;
		}
	}

	if (!bHitSomething)
	{
		// attempt to hit an object in the world (e.g. destructible object)
		foreach GP.TraceActors(class'Actor', HitActor, HitLoc, HitNorm, EndTrace, StartTrace, MeleeTraceExtent, HitInfo)
		{
			if(GearDestructibleObject(HitActor) != None || GearMeleeTarget(HitActor) != None)
			{
				HitActor.TakeDamage(150.f,GP.Controller,HitLoc,HitNorm * -1.f,InstantHitDamageTypes[MELEE_ATTACK_FIREMODE],HitInfo);
				bHitSomething = TRUE;
			}
		}

		// If we didn't hit something low down, try up a bit higher (need this for grapples)
		if(!bHitSomething)
		{
			foreach GP.TraceActors(class'Actor', HitActor, HitLoc, HitNorm, EndTrace + vect(0,0,64), StartTrace + vect(0,0,64), MeleeTraceExtent, HitInfo)
			{
				if(GearDestructibleObject(HitActor) != None || GearMeleeTarget(HitActor) != None)
				{
					HitActor.TakeDamage(150.f,GP.Controller,HitLoc,HitNorm * -1.f,InstantHitDamageTypes[MELEE_ATTACK_FIREMODE],HitInfo);
					bHitSomething = TRUE;
				}
			}
		}
	}

	if( bHitSomething )
	{
		PC = GearPC(Instigator.Controller);
		if( PC != None )
		{
			PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
		}
		return TRUE;
	}

	return FALSE;
}

/** Trace, but ignore volumes and triggers */
simulated function Actor TraceNoVolumesOrTriggers(Pawn TraceOwner, vector End, vector Start, vector Extent, out vector HitLocation)
{
	local Actor HitActor;
	local vector HitLoc, HitNorm;

	// Iterate over each actor along trace...
	foreach TraceOwner.TraceActors(class'Actor', HitActor, HitLoc, HitNorm, End, Start, Extent)
	{
		// .. if it's not a trigger or a volume, use it!
		if(Volume(HitActor) != None && Trigger(HitActor) != None)
		{
			HitLocation = HitLoc;
			return HitActor;
		}
	}

	// Found nothing non-volume or -trigger like :(
	return None;
}

/**
 * Melee Damage
 * @param	InDamage	Damage given to override the default RandRange(MeleeAttackDamageMin, MeleeAttackDamageMax)
 */
simulated function bool MeleeAttackImpact( optional int InDamage, optional out GearPawn OutVictimPawn, optional out TraceHitInfo OutHitInfo, optional bool bTest )
{
	local GearPawn		GP, TestGP;
	local vector		CheckDir, DirToVictim, CheckDir2D, DirToVictim2D, HitLoc, EndTrace, StartTrace, TestGPLoc;
	local float			Damage;
	local Actor			HitActor;
	local GearPC		 PC;
	local TraceHitInfo	HitInfo;
	local Emitter		AnEmitter;
	local GearPawn		CurrWinner;
	local float			CurrClosestDistance;
	local float			DistanceToCheckAgainst;
	local ImpactInfo	TestImpact;
	local float			AttackRange;
	local GearAI		AI;
	local FracturedStaticMeshActor FracActor;
	`if(`notdefined(FINAL_RELEASE))
	local bool			bShowDebugging;
	`endif

	GP = GearPawn(Instigator);
	`if(`notdefined(FINAL_RELEASE))
	bShowDebugging = GP != None && GP.bWeaponDebug_Accuracy && Instigator.IsLocallyControlled();
	`endif

	// Make sure Pawn is allowed to melee. Pawn could have enterred a state preventing that like going DBNO.
	if( GP != None && !GP.CanEngageMelee() )
	{
		return FALSE;
	}

//	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName());

	// Limit to 4 checks
	if( NumMeleeAttackAttempts >= 4 )
	{
//		`log("rejected NumMeleeAttackAttempts");
		return FALSE;
	}


	CheckDir = vector(GP.GetBaseAimRotation());
	CurrClosestDistance = 99999999;

	// First do a check to see if we hit any Pawns, not on slave weapons
	if( !bSlaveWeapon )
	{
		AttackRange = MeleeAttackRange;
		AI = GearAI(Instigator.Controller);
		if( AI != None )
		{
			AttackRange = AI.EnemyDistance_Melee;
		}

		// Debugging
		`if(`notdefined(FINAL_RELEASE))
		if( bShowDebugging )
		{
			if( NumMeleeAttackAttempts == 0 )
			{
				FlushPersistentDebugLines();
			}
			DrawDebugCone(GP.Location, CheckDir, AttackRange, Acos(MeleeAcquireDotFOV.X), Acos(MeleeAcquireDotFOV.Y), 16, MakeColor(255,0,0,255), TRUE);
		}
		`endif

		foreach WorldInfo.AllPawns(class'GearPawn', TestGP)
		{
			// First make sure Pawn is relevant (ie not dead)
			if( TestGP != GP && !TestGP.bDeleteMe && !TestGP.bTearOff )
			{
//				`log(" Considering:" @ TestGP);
				TestGPLoc = TestGP.GetMeleeHitTestLocation();
				DistanceToCheckAgainst = VSize(TestGPLoc - GP.Location);

				// Then make sure Pawn is within melee range
				if( DistanceToCheckAgainst > AttackRange || DistanceToCheckAgainst > CurrClosestDistance ||
					Abs(TestGPLoc.Z - GP.Location.Z) > GP.CylinderComponent.CollisionHeight * 0.8f )
				{
//					`log("rejected within melee range");
					continue;
				}

				// Then Peform other relevancy checks
				if( !GP.TargetIsValidMeleeTarget(TestGP, TRUE) )
				{
//					`log("rejected TargetIsValidMeleeTarget");
					continue;
				}

				// skip held meatshields
				if ( TestGP.IsAHostage() && Instigator == TestGP.InteractionPawn )
				{
					continue;
				}

				// Check that pawn is within FOV
				DirToVictim = Normal(TestGPLoc - GP.Location);

				if(GP.CarriedCrate == none )
				{
					// If our check FOV is a cone, then perform a simple dot product
					if( MeleeAcquireDotFOV.X == MeleeAcquireDotFOV.Y )
					{
						// Make sure victim is in FOV
						if( DirToVictim dot CheckDir < MeleeAcquireDotFOV.X )
						{
	//						`log("rejected:"@TestGP@"dot:"@DirToVictim dot CheckDir);
							continue;
						}
					}
					// Not a cone, so do separate FOV checks for horizontal and vertical planes
					else
					{
						// First check for horizontal plane -- argh, why can't we use Vect(CheckDir.X, CheckDir.Y, 0.f) ????
						CheckDir2D.X	= CheckDir.X;
						CheckDir2D.Y	= CheckDir.Y;
						CheckDir2D.Z	= 0.f;
						CheckDir2D		= Normal(CheckDir2D);

						DirToVictim2D.X	= DirToVictim.X;
						DirToVictim2D.Y	= DirToVictim.Y;
						DirToVictim2D.Z	= 0.f;
						DirToVictim2D	= Normal(DirToVictim2D);

						// Debugging
						`if(`notdefined(FINAL_RELEASE))
						if( bShowDebugging )
						{
							DrawDebugLine(GP.Location, GP.Location + CheckDir2D * AttackRange, 0, 255, 0, TRUE);
							DrawDebugLine(GP.Location, GP.Location + DirToVictim2D * AttackRange, 0, 255, 0, TRUE);
						}
						`endif

						// Make sure victim is in horizontal FOV
						if( DirToVictim2D dot CheckDir2D < MeleeAcquireDotFOV.X )
						{
	//						`log("rejected:"@TestGP@"horiz dot:"@DirToVictim2D dot CheckDir2D);
							continue;
						}

						// Then, check for vertical plane
						DirToVictim2D.X	= DirToVictim dot CheckDir2D;
						DirToVictim2D.Y	= 0.f;
						DirToVictim2D.Z	= DirToVictim.Z;
						DirToVictim2D	= Normal(DirToVictim2D);

						CheckDir2D		= Vect(1.f, 0.f, 0.f);

						// Debugging
						`if(`notdefined(FINAL_RELEASE))
						if( bShowDebugging )
						{
							DrawDebugLine(GP.Location, GP.Location + CheckDir2D * AttackRange, 0, 0, 255, TRUE);
							DrawDebugLine(GP.Location, GP.Location + DirToVictim2D * AttackRange, 0, 0, 255, TRUE);
						}
						`endif

						// Make sure victim is in vertical FOV
						if( DirToVictim2D dot CheckDir2D < MeleeAcquireDotFOV.Y )
						{
	//						`log("rejected:"@TestGP@"vert dot:"@DirToVictim2D dot CheckDir2D);
							continue;
						}
					}
				}

				// Force Start/EndTrace to be inside the pawns cylinders...
				// Fixes melee animations would cause the neck bone to go through thin geometry to the other side
				// and allow meleeing through walls
				StartTrace	 = GP.Location;
				StartTrace.Z = GP.GetPawnViewLocation().Z;
				EndTrace	 = TestGPLoc;
				EndTrace.Z	 = TestGP.GetPawnViewLocation().Z;

				// Finally trace to make sure there are no obstructions. ie acquiring someone through a wall
				//@note - can't use fasttrace since that'll collide against trigger volumes
				// @note2 - won't this hit triggers too?
				//  @note3 - jag: changed to avoid triggers etc
				//HitActor = Instigator.Trace(HitLoc,HitNorm,EndTrace,StartTrace,TRUE,vect(8,8,32));
				HitActor = TraceNoVolumesOrTriggers(Instigator, EndTrace, StartTrace, vect(8,8,32), HitLoc);

				if (HitActor != None && HitActor.bBlockActors && HitActor != TestGP)
				{
//					`log("rejected Trace"@HitActor);
					continue;
				}

				// We found someone!
				CurrWinner			= TestGP;
				CurrClosestDistance = DistanceToCheckAgainst;
			}
		}
	}

	TestGP = CurrWinner;
	if( TestGP != None )
	{
		if( WorldInfo.NetMode != NM_Client )
		{
			if( !bTest )
			{
				// If no damage override, use defaults
				if( InDamage == 0 )
				{
					if( !GP.GetMeleeDamageOverride(Damage) )
					{
						Damage = RandRange(MeleeAttackDamageMin, MeleeAttackDamageMax);
					}
				}
				// otherwise, use damage override
				else
				{
					Damage = InDamage;
				}

				// If attacker is AI controlled
				if( !GP.IsHumanControlled() )
				{
					Damage *= GP.AI_MeleeDamageScale;
				}

//					`log("Damage:" @ Damage @ "for" @ TestGP);

				if( Damage > 0 )
				{
					// Force kill DBNO players.
					if( TestGP.IsDBNO() )
					{
						Damage = TestGP.DefaultHealth - TestGP.DBNODeathThreshold + 1;
//							`log("DBNO ForceKill Damage:" @ Damage);
					}
					// Give Damage
					GiveMeleeDamageTo(TestGP, Damage);

					// and make them stumble
					if( !TestGP.bPlayedDeath && !TestGP.InGodMode() )
					{
						TestGP.DoStumbleFromMeleeSpecialMove();
					}
				}

				// do melee hit effects...
				PC = GearPC(TestGP.Controller);
				if( PC != None )
				{
					PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
				}
			}

			OutHitInfo		= HitInfo;
			OutVictimPawn	= TestGP;
			//`log("-"@self@WorldInfo.TimeSeconds@"hit"@TestGP@HitInfo.HitComponent@HitInfo.BoneName);
		}

		// client-side nonreplicated effects
		if (!bTest)
		{
			// randomly choose between flesh and armor damage sounds
			if( 0.5f > FRand() )
			{
				TestGP.SoundGroup.PlayFoleySound( TestGP, GearFoley_MeleeAttackHitFleshFX );
			}
			else
			{
				TestGP.SoundGroup.PlayFoleySound( TestGP, GearFoley_MeleeAttackHitArmorFX );
			}
		}

		return TRUE; // we hit someone!
	}
	// If we haven't hit anybody see if we hit a wall
	else
	{
		//`log( "HitActor: " $ HitActor  );
		if( !bHitWallThisAttack )
		{
			// only do smoke if you are close to the wall
			StartTrace	= GetMeleeStartTraceLocation();
			EndTrace	= StartTrace + CheckDir * 96.f;

			TestImpact	= CalcWeaponFire(StartTrace, EndTrace);

			// Debugging
			`if(`notdefined(FINAL_RELEASE))
			if( bShowDebugging )
			{
				DrawDebugLine(StartTrace, EndTrace, 255, 0, 255, TRUE);
				`log(WorldInfo.TimeSeconds @ GetFuncName() @ "HitWallCheck, HitActor:" @ TestImpact.HitActor);
			}
			`endif

			// if we hit world geometry
			if( TestImpact.HitActor != None && (TestImpact.HitActor == WorldInfo || TestImpact.HitActor.bBlockActors) )
			{
				bHitWallThisAttack = TRUE;

				if( !bTest )
				{
					// Particle
					HitLoc		= TestImpact.HitLocation;
					HitLoc.Z	= HitLoc.Z + 100; // ?
					AnEmitter	= GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'COG_Assaultrifle.Effects.P_COG_AssaultRifle_Impact_Melee', HitLoc, rotator(TestImpact.HitNormal) );
					AnEmitter.ParticleSystemComponent.ActivateSystem();

					// Sound
					WeaponPlaySound(MeleeImpactSound); // hit wall
					PlayMeleeScreenShake(None);

					PC = GearPC(Instigator.Controller);
					if( PC != None )
					{
						PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
					}

					// Fracture meshes if we hit them
					if(bAllowMeleeToFracture)
					{
						FracActor = FracturedStaticMeshActor(TestImpact.HitActor);
						if(FracActor != None)
						{
							FracActor.BreakOffPartsInRadius(HitLoc - (TestImpact.HitNormal * 15.0), 35.0, 100.0, TRUE);
							//DrawDebugSphere(HitLoc - (TestImpact.HitNormal * 15.0), 35.0, 16.0, 255,0,0, TRUE);
						}
					}
				}
			}
		}
	}

	if( !bTest )
	{
		NumMeleeAttackAttempts++;
		OutVictimPawn = None;
	}

	return FALSE;
}


/** Utility function to give melee damage */
simulated function GiveMeleeDamageTo(GearPawn Victim, float Damage)
{
	local Vector			EndTrace, MomentumScale;
	local ImpactInfo		Impact;
	local ScreenShakeStruct	Shake;
	local class<DamageType>  DT;
	local Gearpawn			Gp;

	if( Damage < 1.f || WorldInfo.NetMode == NM_Client || bSlaveWeapon )
	{
		return;
	}

	GP = GearPawn(Owner);

	if ( Victim != None )
	{
		// wtf is this?	 should be rolled into PlayMeleeScreenShake functionality.

		if (bDoMeleeDamageShakes)
		{
			Shake.TimeDuration=0.7;
			Shake.RotAmplitude=Vect(3,4,5);
			Shake.RotFrequency=Vect(1,1,1);
			Shake.LocAmplitude=Vect(2,1,1);
			Shake.LocFrequency=Vect(2,1,1);
			Shake.FOVAmplitude=0;
			//Shake.FOVFrequency=20;
			// Shake screen of Victim
			if(GearPC(Victim.Controller) != None)
			{
				GearPC(Victim.Controller).ClientPlayForcefeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
				GearPC(Victim.Controller).ClientPlayCameraShake(Shake,false);
			}
			// Shake screen of Hitter
			if(GP != None)
			{
				if(GearPC(GP.Controller) != None)
				{
					GearPC(GP.Controller).ClientPlayForcefeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
					GearPC(GP.Controller).ClientPlayCameraShake(Shake,false);
				}
			}
		}

		// Trace to neck for maximum blood in your face
		EndTrace = Victim.Mesh.GetBoneLocation(Victim.MeleeDamageBoneName);

		// Do a trace to get HitInfo
		Instigator.TraceComponent(Impact.HitLocation, Impact.HitNormal, Victim.Mesh, EndTrace, Instigator.Location, Vect(0,0,0), Impact.HitInfo);

		MomentumScale		= Impact.HitNormal * -1.f;
		Impact.HitActor		= Victim;
		Impact.RayDir		= Normal(Impact.HitLocation - Instigator.Location);


		if(GP.IsCarryingShield())
		{
			DT = class'GDT_ShieldBash';
		}
		else
		{
			DT = InstantHitDamageTypes[MELEE_ATTACK_FIREMODE];
		}
		Victim.TakeDamage(Damage, Instigator.Controller, Impact.HitLocation, MomentumScale, DT , Impact.HitInfo);

		if( ( WorldInfo.NetMode != NM_DedicatedServer )
			// melee impacts don't need to spawn effects and such when they are really far away
			&& ( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Impact.HitLocation, 2048.0f, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ))
			)
		{
			// Play Physics Body Impact.
			Victim.PlayPhysicsBodyImpact(Impact.HitLocation, MomentumScale, InstantHitDamageTypes[MELEE_ATTACK_FIREMODE], Impact.HitInfo);

			// Back up HitLocation a bit to spawn impact effects
			Impact.HitLocation = Impact.HitLocation + Impact.HitNormal * 3.f;

			SpawnImpactEffects(class'GDT_Melee',Impact,class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterial(Impact),FALSE,TRUE,FALSE);

			if (bDoMeleeDamageShakes)
			{
				PlayMeleeScreenShake( Victim );
			}
		}
	}
	else
	{
		`log(WorldInfo.TimeSeconds @ GetFuncName() @ "SpecialMeleeAttackVictim == None");
	}
}


/************************************************************************************
 * Inventory / Slots / Attachments
 ***********************************************************************************/

reliable client function ClientWeaponSet(bool bOptionalSet, optional bool bDoNotActivate)
{
	local GearPawn	GP;

	`LogInv(Self @ "bOptionalSet:" @ bOptionalSet @ "CharacterSlot:" @ CharacterSlot);

	GP = GearPawn(Instigator);

	// Make sure we have the replicated CharacterSlot before attaching the weapon.
	if( Instigator != None && GP != None && CharacterSlot == EASlot_None && WeaponType != WT_Heavy )
	{
		`LogInv("CharacterSlot == EASlot_None, going to PendingClientWeaponSet");
		GotoState('PendingClientWeaponSet');
		return;
	}

	Super.ClientWeaponSet(bOptionalSet, bDoNotActivate);

	if( GP != None )
	{
		// Set attachment on local client player
		// When the weapon is initially replicated to the local player, it may not become the active weapon.
		// GearPawn::PlayWeaponSwitch() takes care of updating the slot attachments on the local player.
		// That function will only be called if the weapon becomes the active one.
		// So we force update the attachments here, so they are set for the local player.
		if( GP.Weapon != Self )
		{
			GP.AttachWeaponToSlot(Self);
		}
	}
}


/** @see Inventory::GivenTo */
function GivenTo(Pawn thisPawn, optional bool bDoNotActivate)
{
	local GearPawn.EAttachSlot	DesiredSlot;

	// if we're given a weapon, find an available slot for it
	if( GearInventoryManager(InvManager) != None && WorldInfo.NetMode != NM_Client )
	{
		// if checkpoint data mandates our slot, use that
		if ( CharacterSlot != EASlot_None && GearGameSP_Base(WorldInfo.Game) != None &&
			GearGameSP_Base(WorldInfo.Game).bCheckpointLoadInProgress )
		{
			DesiredSlot = CharacterSlot;
			CharacterSlot = EASlot_None; // avoids early out check
		}
		else
		{
			DesiredSlot = GearInventoryManager(InvManager).FindFreeSlotForInventoryClass(class);
		}
		AssignToSlot(DesiredSlot);
	}

	Super(Inventory).GivenTo(thisPawn, bDoNotActivate);
	`LogInv(Self @ "thisPawn:" @ thisPawn @ "bDoNotActivate:" @ bDoNotActivate @ "CharacterSlot:" @ CharacterSlot);
}


/** @see Inventory::ItemRemovedFromInvManager */
function ItemRemovedFromInvManager()
{
	local GearPawn	GP;

	// if weapon was attached, then remove it
	RemoveFromSlot();

	GP = GearPawn(Instigator);
	if( GP != None && GP.MyGearWeapon == Self )
	{
		GP.MyGearWeapon = None;
	}

	Super.ItemRemovedFromInvManager();
}


/**
 * Assign weapon to a given slot.
 * Triggers replication and visual updates through GearPawn.AttachWeaponToSlot()
 * Network: Server.
 */
function AssignToSlot(GearPawn.EAttachSlot Slot)
{
	local GearPawn	GP;

	GP = GearPawn(Instigator);

	if( GP != None )
	{
		`LogInv(Slot);
		if( Slot != EASlot_None )
		{
			if( CharacterSlot != Slot )
			{
				// force replication
				bForceNetUpdate = TRUE;
				// Assign slot
				CharacterSlot = Slot;
			}

			// trigger replication and visual updates
			GP.AttachWeaponToSlot(Self);
		}
	}
}


/**
 * Remove weapon from its slot. And set it to EASlot_None.
 * Triggers replication and visual updates through GearPawn.DetachWeaponFromSlot()
 * Network: Server.
 */
function RemoveFromSlot()
{
	local GearPawn	GP;

	GP = GearPawn(Instigator);
	if( GP != None )
	{
		`LogInv(CharacterSlot);

		if( CharacterSlot != EASlot_None )
		{
			// force replication
			bForceNetUpdate = TRUE;
			// trigger replication and visual updates
			GP.DetachWeaponFromSlot(Self);
			// clear slot
			CharacterSlot = EASlot_None;
		}
	}
}

simulated event Destroyed()
{
	local GearPawn	GP;

	Super.Destroyed();

	// make sure we got detached correctly
	if (CharacterSlot != EASlot_None)
	{
		GP = GearPawn(Instigator);
		if (GP != None)
		{
			GP.DetachWeaponFromSlot(self);
			CharacterSlot = EASlot_None;
		}
	}
}

/** Sets up effects components that are attached to the weapon mesh. */
simulated event AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	// we allowed attachment on dedicated servers, but we don't need stuff after this
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Attach muzzle flash related effects
	if( SkelMesh != None && MuzzleSocketName != '' )
	{
		// if we are pending kill then don't attach things
		if( SkelMesh.IsPendingKill() )
		{
			//ScriptTrace();
			return;
		}

		if (Owner != None)
		{
			SetMaterialBasedOnTeam( SkelMesh, GearPawn(Owner) != none ? GearPawn(Owner).GetMPWeaponColorBasedOnClass() : Owner.GetTeamNum() );
			MIC_WeaponSkin.SetScalarParameterValue( 'Weap_SpecMult', 1.0f );
			MIC_WeaponSkin.SetScalarParameterValue( 'Weap_DroppedGlow', 0.0f );
			MIC_WeaponSkin.SetScalarParameterValue( 'Weap_DropGlowAlpha', 0.0f );
		}

		// Muzzle Flash Mesh
		if( MuzzleFlashMesh != None && !SkelMesh.IsComponentAttached(MuzzleFlashMesh) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashMesh, MuzzleSocketName);
		}

		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			SkelMesh.AttachComponentToSocket(MuzFlashEmitter, MuzzleSocketName);
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			MuzzleFlashLight.SetTranslation(MuzzleLightOffsetFromCannon);
			SkelMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleSocketName);
		}

		// attach after fire particle system
		if( PSC_ReloadBarrelSmoke != None && !SkelMesh.IsComponentAttached(PSC_ReloadBarrelSmoke) )
		{
			SkelMesh.AttachComponentToSocket(PSC_ReloadBarrelSmoke, MuzzleSocketName);
			HidePSC_ReloadBarrelSmoke();
		}
	}
}

/**
 * Returns TRUE if weapon should be mirrored when attached.
 * Uses MirrorNode.bPendingIsMirrored instead of PawnOwner.bIsMirrored, in case weapon attach is done before mirror has hapened.
 * This can happen with Heavy Weapons, as they use anim notifies, and are not synchronized with pawn mesh mirroring.
 */
simulated final function bool ShouldWeaponBeMirrored(GearPawn MyGearPawn)
{
	return MyGearPawn != None && MyGearPawn.MirrorNode != None && MyGearPawn.MirrorNode.bPendingIsMirrored;
}

/**
 * Attach Weapon Mesh to player's hand. This is not a slot attachment on the back,
 * but a full functioning weapon mesh in the player's hand to use.
 *
 * @param	SkeletalMeshComponent where to attach weapon
 * @param	SocketName of Socket to use in provided SkeletalMeshComponent
 * @return	TRUE if attach was successful
 */
simulated function AttachWeaponTo(SkeletalMeshComponent MeshCpnt, optional Name SocketName)
{
	// Already attached there, skip.
	if( SocketName != '' && SocketAttachedTo == SocketName )
	{
		`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Already attached to socket" @ SocketName @ ". Skipping attachment.");
		return;
	}

	SocketAttachedTo = SocketName;
	PerformWeaponAttachment(MeshCpnt, SocketName);
}

simulated function UpdateAnimNodeReferences()
{
	local SkeletalMeshComponent WeaponSkelMesh;

	WeaponSkelMesh = SkeletalMeshComponent(Mesh);
	if( WeaponSkelMesh == None )
	{
		`DLog("Warning:" @ Self @ "has no SkeletalMesh");
		return;
	}

	// See if for whatever reason our weapon didn't instance our AnimTree
	if( WeaponSkelMesh.Animations == None && WeaponSkelMesh.AnimTreeTemplate != None )
	{
		`DLog("Warning: AnimTreeTemplate not instanced!!!" @ WeaponSkelMesh.AnimTreeTemplate @ "Instigator" @ Instigator @ "Controller:" @ Instigator.Controller);
		ScriptTrace();
 		WeaponSkelMesh.SetAnimTreeTemplate(WeaponSkelMesh.AnimTreeTemplate);
	}

	if( WeaponSkelMesh.Animations == None )
	{
		`DLog("Warning:" @ Self @ "has no Animations. AnimTreeTemplate:" @ WeaponSkelMesh.AnimTreeTemplate);
		return;
	}

	// Update our references
	CustomAnimNode = GearAnim_Slot(WeaponSkelMesh.FindAnimNode('CustomAnim'));
	AnimTreeRootNode = AnimTree(WeaponSkelMesh.Animations);
}

/** Notification called when one of our meshes gets his AnimTree updated */
simulated event AnimTreeUpdated(SkeletalMeshComponent SkelMesh)
{
// 	`DLog(SkelMesh.AnimTreeTemplate);
	if( SkelMesh == Mesh )
	{
		UpdateAnimNodeReferences();
	}

	Super.AnimTreeUpdated(SkelMesh);
}

/** Called by AttachWeaponTo to perform weapon attachment after we're sure it's ok to do so. */
simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name SocketName)
{
	local SkeletalMeshComponent	SkelMesh;
	local GearPawn				P;
	local vector				NewScale3D;

	// Add weapon specific animation sets
	// And remove unused ones.
	P = GearPawn(Instigator);
	SkelMesh = SkeletalMeshComponent(Mesh);

	`LogInv("MeshCpnt:" @ MeshCpnt @ "SocketName:" @ SocketName);

	// Attach Weapon mesh to player skelmesh
	if( Mesh != None && !MeshCpnt.IsComponentAttached(Mesh,SocketName) )
	{
		// Mirror weapon
		if( ShouldWeaponBeMirrored(P) )
		{
			NewScale3D = default.Mesh.Scale3D;
			NewScale3D.Y = -NewScale3D.Y;
			Mesh.SetScale3D( NewScale3D );
		}
		else
		{
			Mesh.SetScale3D(default.Mesh.Scale3D);
		}

		// Set Shadow parent and light environment.
		Mesh.SetShadowParent(MeshCpnt);
		Mesh.SetLightEnvironment(GearPawn(Instigator).LightEnvironment);

		MeshCpnt.AttachComponentToSocket(Mesh, SocketName);
	}

	if( SkelMesh != None )
	{
		if( SkelMesh.Animations != None )
		{
			UpdateAnimNodeReferences();
		}
		else
		{
			// AnimTree is not instanced right away, so set timer to cache nodes a little later.
			SetTimer(0.1f, FALSE, 'UpdateAnimNodeReferences');
		}
	}

	AttachMuzzleEffectsComponents(SkelMesh);

	// attach shell ejection particle system
	//@todo. Should this not check the SkelMesh if
	//    ShouldUseMuzzleFlashSocketForShellEjectAttach() == false???
	if( PSC_ShellEject != None && !MeshCpnt.IsComponentAttached(PSC_ShellEject) )
	{
		if( !ShouldUseMuzzleFlashSocketForShellEjectAttach() )
		{
			MeshCpnt.AttachComponentToSocket(PSC_ShellEject, SocketName);
		}
		else
		{
			SkelMesh.AttachComponentToSocket(PSC_ShellEject, MuzzleSocketName);
		}

		PSC_ShellEject.bJustAttached = true;

		HidePSC_ShellEject();
	}
}

/**
 * Some weapons end up in odd places when the do their reload.	As the PSC_ShellEject gets
 * attached to the hand bone (usually) we get crazy rotations when the shell eject actually happens
 * This allows us to override that behavior and attach to the muzzleFlash Socket which will be correct
 * orientation
 **/
simulated function bool ShouldUseMuzzleFlashSocketForShellEjectAttach()
{
	return FALSE;
}

/**
 * Detach weapon components from Instigator. Perform any clean up.
 */
simulated function DetachWeapon()
{
	local SkeletalMeshComponent	PawnMesh;
	local Pawn OwnerPawn;

	// Clear socket name, so we can attach again later.
	SocketAttachedTo = '';

	// Clear node references so AnimTree can be garbage collected.
	CustomAnimNode = None;
	AnimTreeRootNode = None;

	if( Instigator != None )
	{
		PawnMesh = Instigator.Mesh;
	}
	else
	{
		// if instigator is None but attachment is somehow still in place,
		// detach from whatever the weapon mesh thinks it's attached to
		OwnerPawn = Pawn(Owner);
		if (OwnerPawn != None)
		{
			PawnMesh = OwnerPawn.Mesh;
		}
	}

	if( Mesh != None )
	{
		// detach mesh from instigator
		if( PawnMesh != None && PawnMesh.IsComponentAttached(Mesh) )
		{
			PawnMesh.DetachComponent(Mesh);
		}

		// remove shadow parent and light environment, as this causes a nice little crash when garbage collection is performed.
		Mesh.SetShadowParent(None);
		Mesh.SetLightEnvironment(None);
	}

	if( PawnMesh != None )
	{
		// shell eject particle system
		if( PSC_ShellEject != None )
		{
			if( PawnMesh.IsComponentAttached(PSC_ShellEject) )
			{
				PawnMesh.DetachComponent(PSC_ShellEject);
			}

			if( SkeletalMeshComponent(Mesh).IsComponentAttached(PSC_ShellEject) )
			{
				SkeletalMeshComponent(Mesh).DetachComponent(PSC_ShellEject);
			}
		}
	}
}

simulated function HolsterWeaponTemporarily(bool bHolster)
{
	`LogInv("bHolster" @ bHolster);

	if( bHolster && !bTemporaryHolster )
	{
		bTemporaryHolster = TRUE;
		PutDownWeapon();
	}
	else if( !bHolster && bTemporaryHolster )
	{
		Activate();
		bTemporaryHolster = FALSE;
	}
}


simulated state WeaponPuttingDown
{
	simulated function bool CanReload()
	{
		// no reloading when switching weapons
		return FALSE;
	}

	simulated function bool TryPutDown()
	{
		local GearPawn P;

		P = GearPawn(Instigator);
		if( P != None && P.IsDoingSpecialMove(SM_WeaponPickup) )
		{
			GSM_Pickup(P.SpecialMoves[SM_WeaponPickup]).TryToPutDownNotify();
		}
		return FALSE;
	}

	/**
	 * Override WeaponIsDown() to not call DetachWeapon().
	 *
	 * We delay that to when the weapon pawn has played his weapon switching animation.
	 * When we switch weapons we lose our ActiveReload bonuses.
	 */
	simulated function WeaponIsDown()
	{
		local GearPawn P;

		P = GearPawn(Instigator);

		`LogInv("");

		// when we are being scorched and switch weapons we need to turn off the burn value otherwise it will be there when we get it back
		if( MIC_WeaponSkin != none )
		{
			MIC_WeaponSkin.SetScalarParameterValue( 'Heat', 0.0f );
			MIC_WeaponSkin.SetScalarParameterValue( 'Burn', 0.0f );
		}

		if( bTemporaryHolster )
		{
			// tell pawn so he can detach and all that
			P.SetWeaponTemporarilyHolstered(TRUE);
		}
		else
		{
			if( InvManager.CancelWeaponChange() )
			{
				return;
			}

			// cancel the ActiveReload effects on this weapon
			ActiveReload_NumBonusShots = 0;
			P.ActiveReload_CurrTier = 0;
			P.bActiveReloadBonusActive = FALSE;

			// switch to pending weapon
			InvManager.ChangedWeapon();
		}

		// Put weapon to sleep, except if instigator reselected the same weapon!
		if( Instigator.Weapon != Self )
		{
			GotoState('Inactive');
		}
	}
}


/**
 * Sets the timing for putting a weapon down.  The WeaponIsDown event is trigged when expired
 */
simulated function TimeWeaponPutDown()
{
	local GearPawn	P;
	local GearPC	PC;

	P = GearPawn(Instigator);

	if( P == None )
	{
		Super.TimeWeaponPutDown();
		return;
	}

	// If not a temporary holster, notify GearInventoryManager that we're putting our weapon down
	if( !bTemporaryHolster )
	{
		GearInventoryManager(InvManager).PuttingWeaponDown();
	}

	P.bSwitchingWeapons = TRUE;
	P.UpdateBoneLeftHandIK();

	// Set Timer for switch
	SetTimer( PutDownTime, FALSE, nameof(WeaponIsDown) );
	`LogInv("PutDownTime" @ PutDownTime);

	PC = GearPC(P.Controller);

	if( !P.bQuietWeaponEquipping
		&& ((WorldInfo.Game == None) || !WorldInfo.Game.bWaitingToStartMatch)
		&& (( PC != None && !PC.bCinematicMode) || ( PC == None ))
		)
	{
		if (WeaponDeEquipSound != None)
		{
			//`log( " EQ bQuietWeaponEquipping: " $ P.bQuietWeaponEquipping );
			//`log( "Hidden: " $ P.bHidden $ " PC.bCinematicMode: " $ PC.bCinematicMode	 );
			// Play putting down animation on Pawn
			P.PlaySound(WeaponDeEquipSound, TRUE);
		}
	}

	// First abort weapon related animations currently playing
	EndReloadAnimations();

	// Then play put down animation.
	PlayHolsterAnimation();

}

simulated function PlayHolsterAnimation()
{
	local GearPawn	P;

	P = GearPawn(Instigator);

	// Left Shoulder slot
	if( P.IsSlotLeftShoulder(CharacterSlot) )
	{
		P.BS_PlayByDuration(HolsterShoulderLeft, PutDownTime, 0.25f, -1.f);
	}
	// Right Shoulder Slot
	else if( P.IsSlotRightShoulder(CharacterSlot) )
	{
		P.BS_PlayByDuration(HolsterShoulderRight, PutDownTime, 0.25f, -1.f);
	}
	// Unsupported slot
	else
	{
		P.BS_PlayByDuration(HolsterShoulderRight, PutDownTime, 0.25f, -1.f);
	}
}

/** Stop Holster animations */
simulated function StopHolsterAnims()
{
	local GearPawn	P;

	P = GearPawn(Instigator);

	P.BS_Stop(HolsterShoulderLeft, 0.25f);
	P.BS_Stop(HolsterShoulderRight, 0.25f);
}

/**
 * Sets the timing for equipping a weapon.
 * The WeaponEquipped event is trigged when expired
 */
simulated function TimeWeaponEquipping()
{
	local GearPawn	P;
	local GearPC	PC;
	local int		i;
	local float		BlendInTime;

	P = GearPawn(Instigator);
	//`log(WorldInfo.TimeSeconds @ GetFuncName() );

	if( P == None )
	{
		SetTimer( 0.0001f, FALSE, 'WeaponEquipped' );
		return;
	}

	// We're switching weapons
	P.bSwitchingWeapons = TRUE;
	// Reset when we get a new weapon
	P.bTargetingNodeIsInIdleChannel = P.default.bTargetingNodeIsInIdleChannel;

	// Disable IK on Left Hand IK Bone if needed
	// Weapon such as grenades and pistol, don't want IK on Left Hand.
	P.UpdateBoneLeftHandIK();

	// Update AimOffset profiles.
	for(i=0; i<AimOffsetProfileNames.Length; i++)
	{
		P.SetAimOffsetNodesProfile(AimOffsetProfileNames[i]);
	}

	PC = GearPC(P.Controller);

	if( (!P.bQuietWeaponEquipping)
		&& ((WorldInfo.Game == None) || !WorldInfo.Game.bWaitingToStartMatch)
		&& ( ( PC != None && !PC.bCinematicMode) || ( PC == None ))
		)
	{
		if (WeaponEquipSound != None)
		{
			//`log( "De EQ bQuietWeaponEquipping: " $ P.bQuietWeaponEquipping $ " WorldInfo.Game: " $ WorldInfo.Game );
			//`log( "Hidden: " $ P.bHidden $ " PC.bCinematicMode: " $ PC.bCinematicMode	 );
			P.PlaySound(WeaponEquipSound, TRUE);
		}
	}

	SetTimer( EquipTime, FALSE, 'WeaponEquipped' );
	`LogInv("EquipTime" @ EquipTime);

	if( bTemporaryHolster )
	{
		// After a temporary holster, add a blend in time, because we're not perfectly transitioning from
		// the putdown animation.
		BlendInTime = 0.2f;
		SetTimer( BlendInTime, FALSE, nameof(EquipWeaponAfterTemporaryHolster) );
	}

	// Stop Holster Animations if they were playing.
	StopHolsterAnims();
	// End reloading animations
	EndReloadAnimations();

	PlayEquipAnimation(BlendInTime);

	// Attach weapon
	P.AttachWeapon();
}

reliable client function ClientWeaponThrown()
{
	local GearPawn	GP;
	local INT		i;

	GP = GearPawn(Instigator);

	Super.ClientWeaponThrown();

	// If we've thrown our currently held weapon, then we need to clear some things!
	// Like reset custom weapon animations.
	// This can cause bugs for ex.. if being chainsawed while carrying a heavy weapon.
	// Animations are not compatible, so we need to reset to default when weapon is dropped.
	if( GP != None && GP.MyGearWeapon == Self )
	{
		`LogInv("Updating animations.");
		// Clear reference to MyGearWeapon
		GP.MyGearWeapon = None;
		GP.Weapon = None;

		// Update animations to reflect this change.
		GP.bSwitchingWeapons = FALSE;
		GP.bEquipTransition = FALSE;
		// Update left hand IK
		GP.UpdateBoneLeftHandIK();

		// Update AimOffset profiles.
		for(i=0; i<class'GearWeapon'.default.AimOffsetProfileNames.Length; i++)
		{
			GP.SetAimOffsetNodesProfile(class'GearWeapon'.default.AimOffsetProfileNames[i]);
		}

		// Update the animation sets related to the current weapon
		// Adding or removing weapon specific sets.
		GP.UpdateAnimSetList();
		// Force Pawn mesh to be updated.
		GP.Mesh.ForceSkelUpdate();
	}
	RemoveFromSlot();
	Instigator = None;
}

simulated function PlayEquipAnimation(float BlendInTime)
{
	local GearPawn			P;
	local GearPawn_Infantry	InfantryPawn;

	P = GearPawn(Instigator);
	InfantryPawn = GearPawn_Infantry(Instigator);
	if( InfantryPawn != None )
	{
		InfantryPawn.StopIdleBreakAnim();
	}

	if( !P.IsDoingSpecialMove(SM_WeaponPickup) )
	{
		SetTimer(EquipTime * 0.5f, FALSE, 'TurnOnEquipTransition');

		// Left Shoulder slot
		if( P.IsSlotLeftShoulder(CharacterSlot) )
		{
			P.BS_PlayByDuration(EquipShoulderLeft, EquipTime, BlendInTime, EquipTime * 0.5f);
		}
		// Right Shoulder Slot
		else if( P.IsSlotRightShoulder(CharacterSlot) )
		{
			P.BS_PlayByDuration(EquipShoulderRight, EquipTime, BlendInTime, EquipTime * 0.5f);
		}
		// Unsupported slot
		else
		{
			P.BS_PlayByDuration(EquipShoulderRight, EquipTime, BlendInTime, EquipTime * 0.5f);
		}
	}
}

/** We use this to be able to turn on the IK sooner, so it blends in nicely */
simulated function TurnOnEquipTransition()
{
	local GearPawn	P;

	P = GearPawn(Instigator);
	if( P != None )
	{
		P.bEquipTransition = TRUE;
		P.UpdateBoneLeftHandIK();
	}
}

simulated function EquipWeaponAfterTemporaryHolster()
{
	GearPawn(Instigator).SetWeaponTemporarilyHolstered(FALSE);
}


/**
 * State WeaponEquipping
 * The Weapon is in this state while transitioning from Inactive to Active state.
 * Typically, the weapon will remain in this state while its selection animation is being played.
 * While in this state, the weapon cannot be fired.
 */
simulated state WeaponEquipping
{
	simulated function BeginState(Name PreviousStateName)
	{
		`LogInv("");
		Super.BeginState(PreviousStateName);
	}

	simulated function EndState(Name NextStateName)
	{
		local GearPawn P;
		local GearPC PC;

		P = GearPawn(Instigator);
		if( P != None )
		{
			P.bSwitchingWeapons = FALSE;
			P.bEquipTransition = FALSE;
			// Update left hand IK
			P.UpdateBoneLeftHandIK();

			// Trigger the weapon equip delegates in the GearPC
			PC = GearPC(P.Controller);
			if ( PC != None )
			{
				PC.TriggerGearWeaponEquipDelegates( Class );
			}
		}

		Super.EndState(NextStateName);
	}

	simulated function WeaponEquipped()
	{
		if( bTemporaryHolster )
		{
			// tell pawn so he can detach and all that
			GearPawn(Instigator).SetWeaponTemporarilyHolstered(FALSE);
		}

		// If not a temporary holster, notify GearInventoryManager that we've equipped a weapon
		if( !bTemporaryHolster )
		{
			GearInventoryManager(InvManager).WeaponEquipped();
		}

		Super.WeaponEquipped();
	}
}


simulated function DrawDebugShot(vector StartLocation, rotator AimRot)
{
	DrawDebugLine( StartLocation, StartLocation + vector(AimRot) * 1024, 255, 255, 255, true );
}

/**
 * Returns true and sets StartTraceLoc if the weapon has a custom start trace location.
 * Returns false and leaves StartTraceLoc unaltered if not.
 */
simulated function bool GetCustomStartTraceLocation(out vector StartTraceLoc)
{
	return FALSE;
}

/**
 * Returns the weapon-specific targeting FOV, or BaseFOV if the weapon
 * has no desire to specify a targeting FOV.
 */
simulated function float GetTargetingFOV(float BaseFOV)
{
	if (TargetingFOV <= 0)
	{
		return BaseFOV;
	}
	else
	{
		return TargetingFOV;
	}
}

/** Weapon can make situational FOV adjustments if it so chooses */
simulated function float GetAdjustedFOV(float BaseFOV)
{
	//local GearPawn	GP;
	//GP = GearPawn(Instigator);

	//if (GP != None)
	//{
	//	if (GP.IsInAimingPose())
	//	{
	//		return BaseFOV + AimingFOVAdjustment;
	//	}
	//}

	return BaseFOV;
}

/** Returns the amount of time AI should pause before firing first shot */
final function float GetAIAimDelay()
{
	return RandRange( AI_AimDelay.X, AI_AimDelay.Y );
}


//@see GearPawn.SetAttachmentVisibility
//@see GearShield for duplicated version of this as it has a different hierarchy
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum )
{
	local LinearColor LC;

	//`log( "SetMaterialBasedOnTeam" @ self );
	if( (TheMesh != None) && ( MIC_WeaponSkin != None ) )
	{
		TeamNum = GearPawn(Owner).GetMPWeaponColorBasedOnClass();
		//`log( "MP TeamNum" @ TeamNum );

		// COG
		if( TeamNum == 0 )
		{
			LC = GetWeaponEmisColor_COG( WorldInfo, WeaponID );
		}
		else
		{
			LC = GetWeaponEmisColor_Locust( WorldInfo, WeaponID );
		}

		MIC_WeaponSkin.SetVectorParameterValue( 'Weap_EmisColor', LC );
	}
}

/** This will return the COG Emissive Color for this weapon for this map **/
static simulated function LinearColor GetWeaponEmisColor_COG( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local GearMapSpecificInfo GSI;
	local LinearColor LC;

	GSI = GearMapSpecificInfo(TheWorldInfo.GetMapInfo());
	if( ( GSI != none ) && ( GSI.ColorConfig != none ) )
	{
		LC = GSI.ColorConfig.WeaponColors[WeapID].EmissiveCOG;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using EmissiveCOG " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}

		// so try default if there was no weapon specific setting
		LC = GSI.ColorConfig.DefaultWeaponColor.EmissiveCOG;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using global EmissiveCOG " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}
	}
	//`log( "using GlobalDefaultColors.WeaponColors[WeapID].EmissiveCOG" );

	LC = default.GlobalDefaultColors.WeaponColors[WeapID].EmissiveCOG;
	if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
	{
		//`log( "using EmissiveCOG " $ LC.A @ LC.R @ LC.G @ LC.B );
		return LC;
	}

	//`log( "using default.EmissiveCOG" );
	return default.GlobalDefaultColors.DefaultWeaponColor.EmissiveCOG;
}

/** This will return the Locust Emissive Color for this weapon for this map **/
static simulated function LinearColor GetWeaponEmisColor_Locust( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local GearMapSpecificInfo GSI;
	local LinearColor LC;

	GSI = GearMapSpecificInfo(TheWorldInfo.GetMapInfo());
	if( ( GSI != none ) && ( GSI.ColorConfig != none ) )
	{
		LC = GSI.ColorConfig.WeaponColors[WeapID].EmissiveLocust;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using EmissiveLocust " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}

		// so try default if there was no weapon specific setting
		LC = GSI.ColorConfig.DefaultWeaponColor.EmissiveLocust;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using global EmissiveLocust " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}
	}
	//`log( "using GlobalDefaultColors.WeaponColors[WeapID].EmissiveLocust" );

	LC = default.GlobalDefaultColors.WeaponColors[WeapID].EmissiveLocust;
	if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
	{
		//`log( "using EmissiveCOG " $ LC.A @ LC.R @ LC.G @ LC.B );
		return LC;
	}


	//`log( "using default.LC_EmisDefaultLocust" );
	return default.GlobalDefaultColors.DefaultWeaponColor.EmissiveLocust;
}


/** This will return the MuzzleFlashlight for this map **/
simulated function Color GetWeaponMuzzleFlashNormal()
{
	local GearMapSpecificInfo GSI;
	local Color LC;

	GSI = GearMapSpecificInfo(WorldInfo.GetMapInfo());
	if( ( GSI != none ) && ( GSI.ColorConfig != none ) )
	{
		LC = GSI.ColorConfig.WeaponColors[WeaponID].MuzzleFlashLightColor;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using GetWeaponMuzzleFlashNormal " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}

		// so try default if there was no weapon specific setting
		LC = GSI.ColorConfig.DefaultWeaponColor.MuzzleFlashLightColor;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using global GetWeaponMuzzleFlashActiveReload " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}
	}

	//`log( "using default.MuzzleFlashLight.LightColor" );
	return default.GlobalDefaultColors.DefaultWeaponColor.MuzzleFlashLightColor;
}


/** This will return the MuzzleFlashlight for this map **/
simulated function Color GetWeaponMuzzleFlashActiveReload()
{
	local GearMapSpecificInfo GSI;
	local Color LC;

	GSI = GearMapSpecificInfo(WorldInfo.GetMapInfo());
	if( ( GSI != none ) && ( GSI.ColorConfig != none ) )
	{
		LC = GSI.ColorConfig.WeaponColors[WeaponID].MuzzleFlashLightColor_AR;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using GetWeaponMuzzleFlashActiveReload " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}

		// so try default if there was no weapon specific setting
		LC = GSI.ColorConfig.DefaultWeaponColor.MuzzleFlashLightColor_AR;
		if( ( LC.A != 0 ) || ( LC.R != 0 ) || ( LC.G != 0 ) || ( LC.B != 0 ) )
		{
			//`log( "using global GetWeaponMuzzleFlashActiveReload " $ LC.A @ LC.R @ LC.G @ LC.B );
			return LC;
		}
	}
	//`log( "using  AR default.MuzzleFlashLight.LightColor" );
	return default.GlobalDefaultColors.DefaultWeaponColor.MuzzleFlashLightColor_AR;
}



/**
 * Causes this weapon to play the given audio. Will replicate to all non-owning clients.
 *
 * @param Sound			SoundCue to play.
 * @param NoiseLoudness	Optional intensity of AI noise.
 */
simulated function WeaponPlaySound( SoundCue Sound, optional float NoiseLoudness )
{
	if( (Sound == None) || bSuppressAudio )
	{
		return;
	}

	if ( Instigator != None )
	{
		// play spatialized, no replication
		PlaySound(Sound,, TRUE,, Instigator.Location);

		// Maybe alert AI of the noise
		if ( (NoiseLoudness > 0.f) && (WorldInfo.Netmode != NM_Client) )
		{
			Instigator.MakeNoise( NoiseLoudness, 'WeaponPlaySound' );
		}
	}
	else
	{
		// @check this, dummy fire weapons should replicate audio?
		PlaySound(Sound,, TRUE);
	}
}



/**
 * Similar to GearWeaponPlaySound, except it returns you the AudioComponent.  Useful for looping sounds, sounds you want greater control over, etc.
 * Does not replicate.
 */
simulated function protected AudioComponent GearWeaponPlaySoundLocalEx(SoundCue Sound, optional SoundCue AltLocalSound, optional AudioComponent AC, optional float FadeInTime)
{
	local GearPC GPC;
	local Actor SoundOwner;

	// @todo, test this with dummyfire (e.g. gatling)

	if( (Sound == None) || bSuppressAudio )
	{
		return None;
	}

//	`log("GearWeaponPlaySoundLocalEx()"@Sound@AltLocalSound);

	// handles dummyfire, where instgator & owner are none
	SoundOwner = (Owner == None) ? self : Owner;

	// handle non-spatialized looping sounds
	if ( (AltLocalSound != None) && (Instigator != None) )
	{
		GPC = GearPC(Instigator.Controller);
		if ( (GPC != None) && GPC.IsLocalPlayerController() && GPC.IsViewingTarget(Instigator) )
		{
			// play LocalSound non-spatialized
			if ( (AC == None) || AC.IsPendingKill() )
			{
				AC = SoundOwner.CreateAudioComponent(AltLocalSound, FALSE, TRUE);
			}
			if (AC != None)
			{
				AC.bUseOwnerLocation = TRUE;
				AC.bAllowSpatialization = FALSE;
				AC.bAutoDestroy = TRUE;
				AC.FadeIn(FadeInTime, 1.f);

				// done, bail
				return AC;
			}
		}
	}

	if( Sound != None )
	{
		if ( (AC == None) || AC.IsPendingKill() )
		{
			AC = SoundOwner.CreateAudioComponent(Sound, FALSE, TRUE);
			if( AC != None )
			{
				AC.bUseOwnerLocation	= TRUE;
				AC.bAutoDestroy			= TRUE;
				SoundOwner.AttachComponent( AC );
			}
		}
		if (AC != None)
		{
			AC.FadeIn(FadeInTime, 1.f);
		}
	}

	return AC;
}


 /**
  * Causes this weapon to play the given audio. Does not replicate, plays only on this machine.
  *
  * @param Sound			SoundCue to play, must be valid.
  * @param AltLocalSound	Optional alternate SoundCue to play for local players.  Implies bNonSpatialized=TRUE for local players.  If none, base sound is played for all.
  * @param bNonSpatialized	TRUE to play the sound nonspatialized.  Will only play for locally controlled players.
  * @param NoiseLoudness	Optional intensity of AI noise.
  */
simulated protected function GearWeaponPlaySoundLocal( SoundCue Sound, optional SoundCue AltLocalSound, optional bool bNonSpatialized, optional float NoiseLoudness )
{
	local AudioComponent AC;
	local GearPC GPC;
	local SoundCue Cue;

	if( bSuppressAudio )
	{
		return;
	}

	//`log("GearWeaponPlaySoundLocal()"@Sound@AltLocalSound@bNonSpatialized@NoiseLoudness);

	// which cue to play
	if (AltLocalSound != None)
	{
		GPC = (Instigator != None) ? GearPC(Instigator.Controller) : None;
		if ( (GPC != None) && GPC.IsLocalPlayerController() && GPC.IsViewingTarget(Instigator) )
		{
			Cue = AltLocalSound;
			bNonSpatialized = TRUE;
		}
		else
		{
			Cue = Sound;
		}
	}
	else
	{
		Cue = Sound;
	}

	if( Cue == None )
	{
		return;
	}

	// now play
	if ( Instigator != None )
	{
		if (bNonSpatialized)
		{
			if ( (Instigator.Controller != None) && Instigator.Controller.IsLocalPlayerController() )
			{
				AC = Instigator.CreateAudioComponent( Cue, FALSE, TRUE );
				if (AC != None)
				{
					AC.bUseOwnerLocation = TRUE;
					AC.bAllowSpatialization = FALSE;
					AC.bAutoDestroy = TRUE;
					AC.Play();
				}
			}
		}
		else
		{
			// play spatialized, no replication
			PlaySound(Cue, TRUE,,, Instigator.Location);
		}

		// Maybe alert AI of the noise
		if ( (NoiseLoudness > 0.f) && (WorldInfo.Netmode != NM_Client) )
		{
			Instigator.MakeNoise( NoiseLoudness,'GearWeaponPlaySoundLocal' );
		}
	}
	else
	{
		// play spatialized, no replication
		PlaySound(Sound, TRUE);
	}
}



/**
 * simplified version of ProjectileFire(), useful in situations where the weapon
 * has no Instigator and we need a simpler firing model (see DummyFire kismet action)
 */
simulated event Projectile ProjectileFireSimple(optional float AimErrorDeg)
{
	local vector RealStartLoc, AimDir;
	local Projectile SpawnedProjectile;
	local rotator AimRot;
	local float AimErrorUnr;

	// fire a projectile without an instigator.	 nothing fancy, just straight
	if( WorldInfo.Netmode != NM_Client )
	{
		AimRot = Rotation;

		// modify Aim to add player aim error
		// and convert aim error from degrees to Unreal Units ( * 65536 / 360 )
		if (AimErrorDeg != 0.f)
		{
			AimErrorUnr = AimErrorDeg * 182.044;
			AimRot.Pitch += AimErrorUnr * (0.5 - FRand());
			AimRot.Yaw	 += AimErrorUnr * (0.5 - FRand());
		}
		AimDir = vector(AimRot);
		RealStartLoc = GetPhysicalFireStartLoc();

		SpawnedProjectile = Spawn(GetProjectileClass(), Self,, RealStartLoc);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( AimDir );
			GearProjectile(SpawnedProjectile).bSuppressAudio = bSuppressAudio;
		}

		// Return it up the line
		return SpawnedProjectile;
	}

	return None;
}

/** get the special melee attack victim from weapons that can do SMAs**/
simulated function GearPawn GetSpecialMeleeAttackVictim();

/** This weapon was denied the opportunity to fire because it could not blind fired.  Give any desired feedback to the player. */
simulated function DoCannotBlindfireFeedback();


/** overloaded to handle None instigators -- these are special weapons (eg dummyfire) and are always relevant */
simulated function bool EffectIsRelevant(vector SpawnLocation, bool bForceDedicated, optional float CullDistance )
{
	local bool bResult;
	local PlayerController P;

	`warn( "EffectIsRelevant has been deprecated in favor of GRI.IsEffectRelevant()" );
	ScriptTrace();

	if (Instigator == None)
	{
		return TRUE;
	}
	else
	{
		if ( WorldInfo.NetMode == NM_DedicatedServer )
		{
			return bForceDedicated;
		}

		if ( (WorldInfo.NetMode == NM_ListenServer) && (WorldInfo.Game.NumPlayers > 1) )
		{
			if ( bForceDedicated )
				return true;
			if ( (Instigator != None) && Instigator.IsHumanControlled() && Instigator.IsLocallyControlled() )
				return true;
		}
		else if ( (Instigator != None) && Instigator.IsHumanControlled() )
		{
			return true;
		}

		bResult = (SpawnLocation != Location || WorldInfo.TimeSeconds - Instigator.LastRenderTime < 0.5);
		if (bResult)
		{
			bResult = false;
			foreach LocalPlayerControllers(class'PlayerController', P)
			{
				if ( P.ViewTarget != None )
				{
					if ( (P.Pawn == Instigator) && (Instigator != None) )
					{
						return true;
					}
					else
					{
						bResult = CheckMaxEffectDistance(P, SpawnLocation, CullDistance);
						break;
					}
				}
			}
		}
		return bResult;
	}
}

static function PrimitiveComponent GetPickupMesh(PickupFactory Factory)
{
	return default.DroppedPickupMesh;
}

function CreateCheckpointRecord(out InventoryRecord Record)
{
	Record.InventoryClassPath = PathName(Class);
	Record.AmmoUsedCount = AmmoUsedCount;
	Record.SpareAmmoCount = SpareAmmoCount;
	Record.CharacterSlot = CharacterSlot;
	Record.bIsActiveWeapon = (Instigator.Weapon == self);
}

function ApplyCheckpointRecord(const out InventoryRecord Record)
{
	AmmoUsedCount = Record.AmmoUsedCount;
	SpareAmmoCount = Record.SpareAmmoCount;
	CharacterSlot = EAttachSlot(Record.CharacterSlot);
}

simulated function bool IsFiring()
{
	return IsFireState(GetStateName());
}

simulated function GetRotationControlScale(out float TurnScale, out float LookUpScale)
{
	TurnScale = 1.f;
	LookUpScale = 1.f;
}

function PlayNeedsAmmoChatter()
{
	if (WorldInfo.NetMode != NM_Client)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_NeedAmmo, Instigator, None);
	}
}

/** Returns the number of ammo icons this weapon will display in a clip */
simulated function float GetAmmoIconCount()
{
	return HUDDrawData.DisplayCount;
}

/** Returns the pixel width of a single ammo for the HUD */
simulated function float GetPixelWidthOfAmmo()
{
	return HUDDrawData.ULPerAmmo;
}

simulated function DebugDrawFireLocation()
{
	local Vector V, D;
	local byte OFM;

	OFM = CurrentFireMode;
	CurrentFireMode = ALTFIRE_FIREMODE;

	GetProjectileFirePosition( V, D );
	DrawDebugBox( V, vect(10,10,10), 255, 0, 0 );
	DrawDebugLine( V, V + D * 128, 255, 0, 0 );
	CurrentFireMode = OFM;
}

/** the MP bot AI uses this to determine if it should delay its current action and pick this up when it gets close
 * currently the return value is used as a bool so just return 0 or 1
 */
static function float DetourWeight(Pawn Other, float PathWeight)
{
	local GearAI_TDM AI;
	local GearWeapon CompareWeapon;
	local GearInventoryManager GearInvManager;

	AI = GearAI_TDM(Other.Controller);
	if (!AI.IsUnderHeavyFire() && AI.GetHealthPercentage() ~= 1.0)
	{
		// compare base rating to our current weapon in the slot, if any
		GearInvManager = GearInventoryManager(Other.InvManager);
		switch (default.WeaponType)
		{
			case WT_Normal:
				CompareWeapon = GearWeapon(GearInvManager.GetInventoryInSlot(EASlot_LeftShoulder));
				// avoid assault rifle
				if ( CompareWeapon != None &&
					(CompareWeapon.IsA('GearWeap_AssaultRifle') || CompareWeapon.IsA('GearWeap_LocustAssaultRifle')) )
				{
					CompareWeapon = GearWeapon(GearInvManager.GetInventoryInSlot(EASlot_RightShoulder));
				}
				break;
			case WT_Holster:
				CompareWeapon = GearWeapon(GearInvManager.GetInventoryInSlot(EASlot_Holster));
				break;
			case WT_Item:
				CompareWeapon = GearWeapon(GearInvManager.GetInventoryInSlot(EASlot_Belt));
			case WT_Heavy:
				break;
			default:
				`Warn("Unknown WeaponType" @ default.WeaponType @ "for" @ default.Class);
				break;
		}

		return (CompareWeapon == None || !CompareWeapon.HasAnyAmmo() || CompareWeapon.default.AIRating < default.AIRating) ? 1 : 0;
	}
	else
	{
		return 0;
	}
}

/** Returns TRUE if weapon wants to define focus loc, false otherwise. */
simulated function bool GetCameraDOFFocusLocation(out vector FocusLoc)
{
	return FALSE;
}

// when an AI wants to evade it will stop firing, then pause for however long this function tells it to before triggering an evade
function float AIDelayBeforeEvade(GearAI AI)
{
	return 0.f;
}

simulated protected native event UpdateBarrelHeatMaterial();
/** Returns the Barrel heat value if this weapons has one**/
simulated function float GetCurrentBarrelHeat()
{
	return CurrentBarrelHeat;
}
simulated function bool IsHeatLimited();

/** If bOverrideDistanceForEnemyTrace is TRUE, this function will be called to provide distance to trace for enemies */
simulated function float GetWeaponDistanceForEnemyTrace()
{
	return class'GearGame.GearPC'.default.DistanceForEnemyTrace;
}

/** TRUE if reloading weapon takes out of TargetingMode, TRUE for most weapons. */
simulated event bool ShouldReloadingPreventTargeting()
{
	return bReloadingWeaponPreventsTargeting;
}

defaultproperties
{
	MinTimeBetweenBulletWhips=2.f
	BulletWhipChance=0.5f

	GlobalDefaultColors=GearPerMapColorConfig'GearMapColorArchetypes.GlobalDefault'
	bKillDuringLevelTransition=FALSE

    //Setting this to TRUE uses shadow volumes, which won't work on Xbox and is overridden by bCastCompositeShadows=TRUE So only on PC will this have a visual afffect
	bDynamicMuzzleFlashes=FALSE
	MuzzleSocketName="Muzzle"

	bCanThrowActiveWeapon=TRUE
	WeaponType=WT_Normal
	CharacterSlot=EASlot_None
	DroppedPickupClass=class'GearDroppedPickup'
	AmmoTypeClass=class'GearAmmoType' // this will make a weapon that has not defined any specific type get ammo from anything

	bAllowsRoadieRunning=TRUE
	bPlayIKRecoil=TRUE
	bPlayDefaultBlindFireStance=TRUE
	bBlindFirable=TRUE
	bSupports360AimingInCover=TRUE
	bAllowIdleStance=TRUE
	bAllowAimingStance=TRUE
	bAllowDownsightsStance=TRUE
	CrosshairExpandStrength=0.25

	MinBlindFireUpAimPitch=-65535

	// MAIN FIREMODE
	FiringStatesArray(0)="WeaponFiring"
	WeaponFireTypes(0)=EWFT_InstantHit
	InstantHitDamageTypes(0)=class'GearDamageType'
	InstantHitMomentum(0)=1
	InstantHitMomentum(1)=1

	// RELOAD
	FiringStatesArray(RELOAD_FIREMODE)="Reloading"
	WeaponFireTypes(RELOAD_FIREMODE)=EWFT_InstantHit	// this is not actually a firing mode, but this avoids an access None log spam

	// MELEE_ATTACK_FIREMODE
	FiringStatesArray(MELEE_ATTACK_FIREMODE)="MeleeAttacking"
	WeaponFireTypes(MELEE_ATTACK_FIREMODE)=EWFT_InstantHit
	InstantHitDamageTypes(MELEE_ATTACK_FIREMODE)=class'GDT_Melee'

	AimOffsetProfileNames(0)="Default"

	BS_PawnWeaponReload={(
		AnimName[BS_Std_Up]				="AR_Reload",
		AnimName[BS_CovStdIdle_Up]		="AR_Cov_Std_Reload",
		AnimName[BS_CovMidIdle_Up]		="AR_Cov_Mid_Reload"
	)}

	// No success animations, just speed up the normal one.
	BS_PawnWeaponReloadSuccess={()}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_Std_Up]				="AR_Reload_Fail",
		AnimName[BS_CovStdIdle_Up]		="AR_Cov_Std_Reload_Fail",
		AnimName[BS_CovMidIdle_Up]		="AR_Cov_Mid_Reload_Fail"
	)}

	HolsterShoulderLeft={(
		AnimName[BS_Std_Upper_Harsh]	="AR_Holster_Lt",
		AnimName[BS_Std_Idle_Lower]		="AR_Holster_Lt",
		AnimName[BS_CovStdIdle_Up]		="AR_Cov_Std_Holster_Lt",
		AnimName[BS_CovMidIdle_Up]		="AR_Cov_Mid_Holster_Lt"
	)}

	HolsterShoulderRight={(
		AnimName[BS_Std_Upper_Harsh]	="AR_Holster_Rt",
		AnimName[BS_Std_Idle_Lower]		="AR_Holster_Rt",
		AnimName[BS_CovStdIdle_Up]		="AR_Cov_Std_Holster_Rt",
		AnimName[BS_CovMidIdle_Up]		="AR_Cov_Mid_Holster_Rt"
	)}

	EquipShoulderLeft={(
		AnimName[BS_Std_Upper_Harsh]	="AR_Equip_Lt",
		AnimName[BS_Std_Idle_Lower]		="AR_Equip_Lt",
		AnimName[BS_CovStdIdle_Up]		="AR_Cov_Std_Equip_Lt",
		AnimName[BS_CovMidIdle_Up]		="AR_Cov_Mid_Equip_Lt"
	)}

	EquipShoulderRight={(
		AnimName[BS_Std_Upper_Harsh]	="AR_Equip_Rt",
		AnimName[BS_Std_Idle_Lower]		="AR_Equip_Rt",
		AnimName[BS_CovStdIdle_Up]		="AR_Cov_Std_Equip_Rt",
		AnimName[BS_CovMidIdle_Up]		="AR_Cov_Mid_Equip_Rt"
	)}

	BS_MeleeAttack(0)=(AnimName[BS_Std_Up]="AR_Melee_Smack_A",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_A")
	BS_MeleeAttack(1)=(AnimName[BS_Std_Up]="AR_Melee_Smack_C",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_C")
	BS_MeleeAttack(2)=(AnimName[BS_Std_Up]="AR_Melee_Smack_B",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_B")
	bDoMeleeAdhesion=TRUE
	MeleeImpactCompleteTime=-1.f

	WeaponReloadSound=None

	FireSound=None
	FireNoAmmoSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleFireEmptyCue'
	WeaponEquipSound=SoundCue'Weapon_AssaultRifle.Actions.CogRifleRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_AssaultRifle.Actions.CogRifleLowerCue'

	PickupSound=SoundCue'Weapon_AssaultRifle.Actions.CogRiflePickupCue'
	WeaponDropSound=SoundCue'Weapon_Sniper.Actions.CogSniperDropCue'

	AR_FailSound=SoundCue'Interface_Audio.Interface.ActiveReloadFailCue'
	AR_SuccessSound=SoundCue'Interface_Audio.Interface.ActiveReloadSuccessCue'
	AR_SuperSuccessSound=SoundCue'Interface_Audio.Interface.ActiveReloadSuperSuccessCue'

//	MuzSmokeParticleSystem=ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_BarrelSmoke'

	MuzzleLightPulseFreq=50
	MuzzleLightPulseExp=1.5

	MuzzleLightDuration=0.05f
	MuzzleLightOffsetFromCannon=(X=10,Y=5,Z=10)	// position so it lights better the character when firing
	TimeToHideMuzzleFlashPS=2.f

//	ActiveReload_BonusDurationTier=(3.0f,4.0f,5.0f)

	bIsSuppressive=TRUE

	AimAssistScale=1.0
	AimAssistScaleWhileTargeting=1.0

	TracerType=WTT_LongSkinny
	TracerScale=(X=1.f,Y=1.f,Z=1.f)
	TracerSmokeTrailEffect=ParticleSystem'War_BulletEffects.FX.FX_P_BulletTrail_Normal'
	TracerSmokeTrailEffectAR=ParticleSystem'War_BulletEffects.FX.FX_P_BulletTrail_HighPower'

	bCanDisplayReloadTutorial=FALSE
	DamageTypeClassForUI=class'GDT_AssaultRifle'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=222,V=100,UL=143,VL=46)
	IconXOffset = 0
	IconYOffset = 0

	AnnexWeaponIcon=(U=0,V=0,UL=128,VL=40)

	Recoil_Hand={(
					TimeDuration=0.33f,
					RotAmplitude=(X=1500,Y=200,Z=0),
					RotFrequency=(X=10,Y=10,Z=0),
					RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
					LocAmplitude=(X=-4,Y=0,Z=1),
					LocFrequency=(X=10,Y=0,Z=10),
					LocParams=(X=ERS_Zero,Y=ERS_Zero,Z=ERS_Random)
					)}

	Recoil_Spine={(
					TimeDuration=0.67f,
					RotAmplitude=(X=750,Y=100,Z=0),
					RotFrequency=(X=10,Y=10,Z=0),
					RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero)
					)}


	bCanSelectWithoutAmmo=TRUE
	bAutoSwitchWhenOutOfAmmo=FALSE
	bSwappable=TRUE

	NeedReloadNotifySound=SoundCue'Interface_Audio.Interface.NeedsReloadNotifyCue'

	bUseTargetingCamera=TRUE
	EquipTime=0.67f
	PutDownTime=0.50f

	HUDDrawData			= (DisplayCount=26,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=240,UL=106,VL=7),ULPerAmmo=4)
	HUDDrawDataSuper	= (DisplayCount=26,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=248,UL=106,VL=7),ULPerAmmo=4)

	bUseMeleeHitTimer=TRUE
	RespawnTime=30.0
	CrosshairIcon=Texture2D'Warfare_HUD.WarfareHUD_XHairs_assaultrifle'

	// Weapon Mesh
	Begin Object Class=SkeletalMeshComponent Name=WeaponMesh
		CollideActors=FALSE
		bCastDynamicShadow=TRUE
		MotionBlurScale=0.0
		bUpdateSkelWhenNotRendered=FALSE
		bAcceptsDynamicDecals=FALSE // Each decal on them causes entire SkelMesh to be rerendered
	End Object

	// muzzle flash emitter
	Begin Object Class=ParticleSystemComponent Name=PSC_WeaponMuzzleMuzFlashComp
	    bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	//MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp 	// each subclass should set this if they want one

	// Muzzle Flash point light
	Begin Object Class=PointLightComponent Name=WeaponMuzzleFlashLightComp
	    bEnabled=FALSE
		Brightness=10
		Radius=256
		bAffectCompositeShadowDirection=FALSE // do not affect light env shadow direction
	End Object
	//MuzzleFlashLight=WeaponMuzzleFlashLightComp  // each subclass should set this if they want one


	// shell case ejection emitter
	Begin Object Class=ParticleSystemComponent Name=PSC_WeaponShellCaseComp
	    bAutoActivate=FALSE
		Rotation=(Yaw=-16384)
		TickGroup=TG_PostUpdateWork
	End Object
	//PSC_ShellEject=PSC_WeaponShellCaseComp  // each subclass should set this if they want one

	// reload barrel smoke
	Begin Object Class=ParticleSystemComponent Name=PSC_WeaponReloadBarrelSmokeComp
	    bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	//PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp  // each subclass should set this if they want one


	bHitWallThisAttack = false;
	MeleeTraceExtent=(X=8,Y=8,Z=8)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting0
		Samples(0)=(LeftAmplitude=20,RightAmplitude=20,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.100)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting0

	bReloadingWeaponPreventsTargeting=TRUE
	bForceWalkWhenTargeting=TRUE

	FireCameraShakeScale=1.f
	MeleeImpactCamShake=(TimeDuration=0.500000,RotAmplitude=(X=500,Y=500,Z=200),RotFrequency=(X=100,Y=100,Z=150),LocAmplitude=(X=0,Y=3,Z=5),LocFrequency=(X=1,Y=10,Z=10),FOVAmplitude=2,FOVFrequency=5)

	bAllowTracers=FALSE
	bCanParryMelee=true

	// CQC Long Executions
	CQC_Long_KillerAnim="CTRL_PunchFace"
	CQC_Long_VictimAnim="DBNO_PunchFace"
	CQC_Long_CameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_PunchFace_Cam01'
	CQC_Long_CameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_PunchFace_Cam02'
	CQC_Long_CameraAnims(2)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_PunchFace_Cam03'
	CQC_Long_CameraAnims(3)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_PunchFace_Cam04'
	CQC_Long_bHolsterWeapon=TRUE
	CQC_Long_VictimDeathTime=2.61f
	CQC_Long_VictimRotStartTime=0.5f
	CQC_Long_VictimRotInterpSpeed=8.f
	CQC_Long_DamageType=class'GDT_Execution_PunchFace'
	CQC_Long_EffortID=GearEffort_PunchFaceLongExecutionEffort

	// CQC Quick Executions
	CQC_Quick_KillerAnim="CTRL_Quick_Boomshot"
	CQC_Quick_VictimDeathTime=0.68f
	CQC_Quick_EffortID=GearEffort_BoomshotQuickExecutionEffort
	CQC_Quick_DamageType=class'GDT_QuickExecution'

	bMuzFlashEmitterIsLooping=TRUE

	ClientSideHitLeeway=1.4

	bAllowMeleeToFracture=TRUE

    WeaponStatIndex=-1

	//bDebug=TRUE
}

