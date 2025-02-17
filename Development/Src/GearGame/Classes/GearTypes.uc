/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* This will hold all of our enums and types and such that we need to
* use in multiple files where the enum can'y be mapped to a specific file.
*/
class GearTypes extends Object
	native
	abstract
	config(Game);

/** replicated information on a hit we've taken */
struct native TakeHitInfo
{
	/** the location of the hit */
	var vector				HitLocation;
	/** how much momentum was imparted */
	var vector				Momentum;
	/** the damage type we were hit with */
	var class<DamageType>	DamageType;
	/** the weapon that shot us */
	var Pawn				InstigatedBy;
	/** the bone that was hit on our Mesh (if any) */
	var byte				HitBoneIndex;
	/** the physical material that was hit on our Mesh (if any) */
	var PhysicalMaterial	PhysicalMaterial;
	/** how much damage was delivered */
	var float				Damage;
};

/** stores checkpoint information for held weapons */
struct InventoryRecord
{
	var string InventoryClassPath;
	var int SpareAmmoCount;
	var int AmmoUsedCount;
	var byte CharacterSlot;
	var bool bIsActiveWeapon;
};

struct native DecalData
{
	/** Used internally for validate the struct contains useful data */
	var bool bIsValid;

	/** Material and any other properties that are needed for impact decals */
	var() MaterialInterface DecalMaterial;

	/** Determines whether or not to use the fast decal attach path which WILL clip when hitting multiple BSP pieces.  Large decals should NOT use this as they will look really bad **/
	var() bool ClipDecalsUsingFastPath;

// 	/** Width of the decal**/
// 	var() RawDistributionFloat Width;
// 	/** Height of the decal**/
// 	var() RawDistributionFloat Height;
// 	/** Thickness of the decal**/
// 	var() RawDistributionFloat Thickness;

	/** Width of the decal **/
	var() float Width;
	/** Height of the decal **/
	var() float Height;
	/** Thickness of the decal (used to calculate the nearplane/farplane values) **/
	var() float Thickness;

	/** Whether decal should randomly alter rotation */
	var() bool bRandomizeRotation;

	/** Range of Percentages that the decal should be randomly scaled **/

	/** Range of Percentages that the decal should be randomly scaled **/
	var() vector2d RandomScalingRange;

	/**
	 * This is the range in units that that effect should be offset from the trace start.  (e.g. if we are tracing straight down this will
	 * cause the trace to start off center)
	 * This value should be the number of units for the radius of possible offset.
	 **/
	var() float RandomRadiusOffset;


	/** How long the decal should last before completely being gone (i.e. if we have a 10 second fade this needs to include that amount) **/
	var() float LifeSpan;

	/** Start/End blend range specified as an angle in degrees. Controls where to start blending out the decal on a surface */
	var() vector2d BlendRange;

	// we don't want to add the object overhead for using these :-(  And we don't want to add the rand() cost for every time when not all will use it.  And we don't want to add an if check each time also.   so we will ponder what to do
	// 		Begin Object Class=DistributionFloatConstant Name=DistributionWidth
	// 			Constant=6.0;
	// 		End Object
	// 		Width=(Distribution=DistributionWidth)
	//
	// 		Begin Object Class=DistributionFloatConstant Name=DistributionHeight
	// 		    Constant=6.0;
	// 		End Object
	// 		Height=(Distribution=DistributionHeight)
	//
	// 		Begin Object Class=DistributionFloatConstant Name=DistributionThickness
	// 			Constant=10.0;
	// 		End Object
	// 		Thickness=(Distribution=DistributionThickness)

	structdefaultproperties
	{
		ClipDecalsUsingFastPath=TRUE

		Width=6.0f
		Height=6.0f
		Thickness=10.0f

		bRandomizeRotation=TRUE

		RandomScalingRange=(X=1.0f,Y=1.2f)

		LifeSpan=15.0f

		BlendRange=(X=70,Y=89.5)
	}

	structcpptext
	{
		/** Constructors */
		FDecalData() 
		{}
		FDecalData(EEventParm)
		{
			appMemzero(this, sizeof(FDecalData));
		}
		FDecalData(ENativeConstructor)
		: ClipDecalsUsingFastPath(TRUE)
		, Width(6.0f)
		, Height(6.0f)
		, Thickness(10.0f)
		, bRandomizeRotation(TRUE)
		, RandomScalingRange(1.0f,1.2f)
		, LifeSpan(15.0f)
		, BlendRange(70.f,89.5f)
		{}
	}
};


/** These are the Tracer Types we utilize.  Named after what they look like. **/
enum EWeaponTracerType
{
	WTT_LongSkinny,
	WTT_ShortBullet,
	WTT_MinigunFastFiring,
	WTT_Hammerburst,
	WTT_Boltok,
	WTT_Brumak,
	WTT_Reaver,
	WTT_BrumakPlayer,
};


// list of all the game buttons for the sake of input handling
//@note - if you re-order this list at all you'll need to update GearPlayerInput.ButtonMappings and GearPlayerInput.UpdateJoystickInputs()
enum EGameButtons
{
	GB_A,
	GB_X,
	GB_Y,
	GB_B,
	GB_Start,
	GB_Back,
	GB_LeftTrigger,
	GB_LeftBumper,
	GB_RightTrigger,
	GB_RightBumper,
	GB_DPad_Up,
	GB_DPad_Left,
	GB_DPad_Down,
	GB_DPad_Right,
	GB_LeftStick_Push,
	GB_RightStick_Push,
	GB_LeftStick_Up,
	GB_LeftStick_Down,
	GB_LeftStick_Left,
	GB_LeftStick_Right,
	GB_RightStick_Up,
	GB_RightStick_Down,
	GB_RightStick_Left,
	GB_RightStick_Right,
	GB_Max,
};

/**
 * Enumeration of the various WeaponClass used mainly for indexing the FXInfo array in GearPhysicalMaterialProperty at the moment
 * we are going to use this for random explosions also so it is not just weapons
 *
 * NOTE:  If any new types are added they must be added at the end as the the GearPhysicalMaterialProperty indexes into an array with these enums
 */
enum EWeaponClass
{
	WC_Boltock, // Pistol Locust
	WC_Boomshot,
	WC_BoomerFlail,
	WC_BrumakMainGun,
	WC_BrumakSideGun,
	WC_BrumakSideGunOther,
	WC_FragGrenade,
	WC_GasGrenade,
	WC_Gnasher, // Shotgun
	WC_HOD,
	WC_Hammerburst,
	WC_InkGrenade,
	WC_KingRavenTurret,
	WC_Lancer,  // COG Assault Rifle
	WC_Longshot, // Sniper
	WC_Minigun, // Heavy Weapon
	WC_Mortar,  // Heavy Weapon
	WC_Scorcher, // Flamethrower
	WC_SmokeGrenade,
	WC_Snub,  // Pistol COG
	WC_TorqueBow,
	WC_Troika,
	WC_TyroTurrent,
	WC_WretchMeleeSlash,
	WC_FragGrenadeExplosion,		// explosions
	WC_TorqueBowArrowExplosion,
	WC_MortarExplosion,
	WC_BoomshotExplosion,
	WC_BrumakRocketExplosion,
	WC_LocustBurstPistol,
	WC_CentaurCannon,
	WC_ReaverRocket,
	WC_RocketLauncherTurret,
	WC_Shield,
	// !!!! add NEW entries below here !!!!
};



enum EImpactTypeBallistic
{
	ITB_None,
	ITB_Boltock, // Pistol Locust
	ITB_BrumakSideGun,
	ITB_BrumakSideGunOther,
	ITB_Gnasher, // Shotgun
	ITB_Hammerburst,
	ITB_KingRavenTurret,
	ITB_Lancer,  // COG Assault Rifle
	ITB_LocustBurstPistol,
	ITB_Longshot, // Sniper
	ITB_Minigun, // Heavy Weapon
	ITB_Scorcher, // Flamethrower
	ITB_Snub,  // Pistol COG
	ITB_TorqueBow,
	ITB_Troika,
	ITB_TyroTurrent,
	ITB_WretchMeleeSlash,
	ITB_WretchMelee,
	ITB_BrumakSideGunPlayer,
	// !!!! add NEW entries below here !!!!
};


enum EImpactTypeExplosion
{
	ITE_None,
	ITE_BoomerFlail,
	ITE_Boomshot,
	ITE_BrumakMainGun,
	ITE_BrumakRocket,
	ITE_CentaurCannon,
	ITE_GrenadeFrag,
	ITE_GrenadeGas,
	ITE_GrenadeInk,
	ITE_GrenadeSmoke,
	ITE_HOD,
	ITE_Mortar,  // Heavy Weapon
	ITE_ReaverRocket,
	ITE_RocketLauncherTurret,
	ITE_TorqueBowArrow,
	ITE_BrumakMainGunPlayer,
	// !!!! add NEW entries below here !!!!
};




/** Enumeration of the various weapons used mainly for indexing the FXInfo array in GearPhysicalMaterialProperty at the moment */
enum ECharacterFootStepType
{
	CFST_Generic,
	CFST_COG_Dom,
	CFST_COG_Marcus,
	CFST_Locust_Drone,
	CFST_Locust_Ticker,
	CFST_Locust_Wretch,
	CFST_Locust_RideableBrumak,
	// !!!! add NEW entries below here !!!!
};

enum EGearDecalType
{
	GDTT_Blood_GenericSplat,
	GDTT_Chainsaw_Ground,
	GDTT_Chainsaw_Wall,
	GDTT_DBNO_BodyHittingFloor,
	GDTT_DBNO_Smear,
	GDTT_ExitWoundSplatter,
	GDTT_GibExplode_Ceiling,
	GDTT_GibExplode_Ground,
	GDTT_GibExplode_Wall,
	GDTT_GibImpact,
	GDTT_GibTrail,
	GDTT_LimbBreak,
	GDTT_MeatBag_BloodSplatter,
	GDTT_MeatBag_FirstGrabbing,
	GDTT_MeatBag_HeelScuff,
	GDTT_NeckSpurt,
	GDTT_Wall_SlammingIntoCover,
	GDTT_Wall_Smear,
	GDTT_Wall_Smear_Mirrored,
	GDTT_GibExplode_Ground_SmallSplat,
	GDTT_BloodPool,
	GDTT_PawnIsReallyHurtSmallSplat,
	GDTT_HitByBulletSmallSplat,
	GDTT_ExecutionLong_PunchFace,
	GDTT_ExecutionLong_Sniper,
	GDTT_ExecutionLong_TorqueBow,
	GDTT_ExecutionLong_PistolWhip,
	GDTT_FlameThrower_StartFlameBlast,
	GDTT_FlameThrower_FlameIsBurning,
	GDDT_ReaverGibExplode,
	// !!!! add NEW entries below here !!!!
};



enum EActionType
{
	/** Default value of the action system */
	AT_None,
	/** Kismet induced button icon **/
	AT_KismetButton,
	/** Reviving and need to stay alive **/
	AT_StayingAlive,
	/** Reviving and want to blow self up **/
	AT_SuicideBomb,
	/** Knocked down by explosion / close range shot **/
	AT_KnockedDown,
	/** Downed Teammate tooltip */
	AT_DownedTeammate,
	/** Looking at something */
	AT_ForceLook,
	/** Available force looks, etc */
	AT_Scripted,
	/** Looking at a player */
	AT_Player,
	/** When performing an action that puts us in a different state, like ValveTurning */
	AT_StateAction,
	/** Vehicles, turrets, etc */
	AT_Vehicle,
	/** Special moves */
	AT_SpecialMove,
	/** Doors, buttons, etc */
	AT_Trigger,
	/** Pickups */
	AT_Pickup,
	/** Movable Objects */
	AT_MovableObject,
	/** vehicle controls */
	AT_VehicleControl
};

enum EFontType
{
	/** WarfareFonts.Fonts.WarfareFonts_Chrom20 */
	FONT_Chrom20,
	/** WarfareFonts.Fonts.WarfareFonts_Chrom24 */
	FONT_Chrom24,
	/** WarfareFonts.Fonts.WarfareFonts_Euro20 */
	FONT_Euro20,
	/** WarfareFonts.Fonts.WarfareFonts_Euro24 */
	FONT_Euro24,
	/** WarfareFonts.WarfareFonts_GameOVer2 */
	FONT_GameOver2,
	/** Warfare_HUD.WarfareHUD_font */
	FONT_HUD_GearHUD_font,
	/** Warfare_HUD.Xbox360_18pt */
	FONT_Xbox360_18pt,
};

/**
 *	Structure to hold arrays since we can't have arrays of arrays
 */
struct native ActionIconData
{
	var array<CanvasIcon> ActionIcons;
};

/**
 * Information for drawing an action.
 */
struct native ActionInfo
{
	/** Type of action for reference/prioritization */
	var transient EActionType ActionType;
	/** Identifier for this action */
	var Name ActionName;
	/** List of icons drawn left->right, centered based on total width */
	var array<ActionIconData> ActionIconDatas;
	/** Spacing to add between icons */
	var float IconSpacing;

	/** Is it currently active? */
	var transient bool bActive;
	/** Current blend percent */
	var transient float BlendPct;
	/** Cached draw info */
	var transient bool bCached;
	var transient float SizeX, SizeY;

	/** Text (tooltip) drawn at center based on total width like the icons */
	var string TooltipText;
	/** Index into the engine's AdditionalFonts array for the font to use. */
	var int FontIndex;

	/** Duration before this action automatically turns off */
	var float AutoDeActivateTimer;
	/** Time at which this action was activated */
	var transient float ActivateTime;

	/** Whether to mirror the icons or not */
	var bool bMirror;

	/** speed the icons will switch for animating */
	var float IconAnimationSpeed;

	// structdefaultproperties used to have:
	//		TooltipFont=Font'Warfare_HUD.HUD.UIfont-chromosome-heavy-big'
	// Now, we replace with WarfareFonts.Fonts.WarfareFonts_Chrom24
	structdefaultproperties
	{
		AutoDeActivateTimer=1.f
		FontIndex=FONT_Chrom24
		IconAnimationSpeed=0.0f
	}
};

/** Types of button displays supported by the SeqAct_ToggleButtonMash kismet action */
enum eGearKismetIconType
{
	eGEARBUTTON_A,
	eGEARBUTTON_A_MASH,
	eGEARBUTTON_B,
	eGEARBUTTON_B_MASH,
	eGEARBUTTON_X,
	eGEARBUTTON_Y,
	eGEARBUTTON_ChainsawMash,
};

// Don't reorder these as that will confuse Live
enum EDifficultyLevel
{
	DL_Casual,
	DL_Normal,
	DL_Hardcore,
	DL_Insane,
};

/** Enum of all of the multiplayer game modes */
enum EGearMPTypes
{
	eGEARMP_Warzone,
	eGEARMP_Execution,
	eGEARMP_KTL,
	eGEARMP_CombatTrials,
	eGEARMP_Annex,
	eGEARMP_Wingman,
	eGEARMP_KOTH,
	eGEARMP_CTM,
};

/** The different modes the campaign lobby can have */
enum EGearCampaignLobbyMode
{
	eGCLOBBYMODE_Error,
	eGCLOBBYMODE_Host,
	eGCLOBBYMODE_Join,
	eGCLOBBYMODE_Split,
	eGCLOBBYMODE_SoloPath,
};

/** Enum of all of the races in Gears */
enum EGearRaceTypes
{
	eGEARRACE_COG,
	eGEARRACE_Locust,
};

/** Enum of the different auto-initialized tutorial modes supported */
Enum EGearAutoInitTutorialTypes
{
	eGAIT_None,
	eGAIT_SP,
	eGAIT_MP,
	eGAIT_Train1,
	eGAIT_Train2,
	eGAIT_Train3,
	eGAIT_Train4,
	eGAIT_Train5,
};

/** Enum of all of the tutorials in the game */
enum EGearTutorialType
{
	GEARTUT_Objectives,
	GEARTUT_Cover,
	GEARTUT_Fire,
	GEARTUT_Target,
	GEARTUT_ChangeWeapon,
	GEARTUT_Reload,
	GEARTUT_ActiveReload,
	GEARTUT_PointOfInterest,
	GEARTUT_Mantle1,
	GEARTUT_Mantle2,
	GEARTUT_Evade,
	GEARTUT_CoverSlip,
	GEARTUT_SwatTurn,
	GEARTUT_Use1,
	GEARTUT_Use2,
	GEARTUT_Grenades1,
	GEARTUT_Grenades2,
	GEARTUT_PlayerDamage,
	GEARTUT_Revive,
	GEARTUT_ChainsawMelee,
	GEARTUT_Ladder,
	GEARTUT_RoadieRun,
	GEARTUT_BlindFire,
	GEARTUT_AimFromCover,
	GEARTUT_Movement,
	GEARTUT_Executions,
	GEARTUT_MeatShield,
	GEARTUT_Crawling,
	GEARTUT_WeapMortar,
	GEARTUT_WeapLongshot,
	GEARTUT_Shield,
	GEARTUT_WeapHOD,
	GEARTUT_WeapBow,
	GEARTUT_Centaur,
	GEARTUT_CentaurCoop,
	GEARTUT_CentaurLights,
	GEARTUT_Reaver,
	GEARTUT_Reaver2,
	GEARTUT_Brumak,
	GEARTUT_WeapSmokeGrenade,
	GEARTUT_WeapShotgun,
	GEARTUT_WeapPistol,
	GEARTUT_WeapMinigun,
	GEARTUT_WeapInkGrenade,
	GEARTUT_WeapHammerburst,
	GEARTUT_WeapFragGrenade,
	GEARTUT_WeapFlameThrower,
	GEARTUT_WeapBurstPistol,
	GEARTUT_WeapBoomshot,
	GEARTUT_WeapBoltok,
	GEARTUT_WeapAssaultRifle,
	GEARTUT_ReloadSimple,
	GEARTUT_UnlimitedAmmo,
	GEARTUT_Sneak,
	GEARTUT_ObjectivesReminder,
	GEARTUT_WeapTurret,
	GEARTUT_TRAIN_WarWelcome,
	GEARTUT_TRAIN_SudDeath,
	GEARTUT_TRAIN_Revive,
	GEARTUT_TRAIN_Crawl,
	GEARTUT_TRAIN_GrenCrawl,
	GEARTUT_TRAIN_ExeWelcome,
	GEARTUT_TRAIN_ExeRule,
	GEARTUT_TRAIN_Exe,
	GEARTUT_TRAIN_GuarWelcome,
	GEARTUT_TRAIN_KSpawn,
	GEARTUT_TRAIN_DSpawn,
	GEARTUT_TRAIN_ExeLead,
	GEARTUT_TRAIN_UDed,
	GEARTUT_TRAIN_LeDed,
	GEARTUT_TRAIN_Assas,
	GEARTUT_TRAIN_HiScor,
	GEARTUT_TRAIN_AnxWelcome,
	GEARTUT_TRAIN_NmyCap,
	GEARTUT_TRAIN_TeamCap,
	GEARTUT_TRAIN_Defend,
	GEARTUT_TRAIN_DrainRng,
	GEARTUT_TRAIN_WinRnd,
	GEARTUT_TRAIN_LoseRnd,
	GEARTUT_TRAIN_MeatWelcome,
	GEARTUT_TRAIN_MeatPick,
	GEARTUT_TRAIN_MeatRng,
	GEARTUT_TRAIN_MeatNmy,
	GEARTUT_TRAIN_MeatScore,
};

enum EGearAct
{
	GEARACT_I,
	GEARACT_II,
	GEARACT_III,
	GEARACT_IV,
	GEARACT_V,
	GEARACT_DLC1,	// Potential DLC Act
	GEARACT_DLC2,	// Potential DLC Act
	GEARACT_DLC3,	// Potential DLC Act
};

enum EChapterPoint
{
	CHAP_Tutorial_Welcome,
	CHAP_Hospital_Desperation,
	CHAP_Assault_Thunder,
	CHAP_Assault_Push,
	CHAP_Landown_Roadblocks,
	CHAP_Landown_Digging,
	CHAP_Intervention_Scattered,
	CHAP_Intervention_Creatures,
	CHAP_Intervention_Revelations,
	CHAP_Rescue_Feeling,
	CHAP_Rescue_Captivity,
	CHAP_Riftworm_Belly,
	CHAP_Outpost_Secret,
	CHAP_Outpost_Origins,
	CHAP_Outpost_Awakening,
	CHAP_MountKadar_Ascension,
	CHAP_Leviathan_Displacement,
	CHAP_Leviathan_Beast,
	CHAP_Maria_Priorities,
	CHAP_Maria_Answers,
	CHAP_Nexis_Nest,
	CHAP_Nexis_NoBack,
	CHAP_Palace_Plans,
	CHAP_Palace_Inquisition,
	CHAP_Escape_Escape,
	CHAP_Jacinto_Stand,
	CHAP_Sinkhole_Parking,
	CHAP_Sinkhole_Footing,
	CHAP_Closure_Closure,
	CHAP_Gameover, // This chapter does not exist and is here so that code knows if CHAP_Closure_Closure was beaten
	CHAP_DLC_1,	// From here down is reserved for potential DLC
	CHAP_DLC_2,
	CHAP_DLC_3,
	CHAP_DLC_4,
	CHAP_DLC_5,
	CHAP_DLC_6,
	CHAP_DLC_7,
	CHAP_DLC_8,
	CHAP_DLC_9,
	CHAP_DLC_10,
	CHAP_DLC_11,
	CHAP_DLC_12,
	CHAP_DLC_13,
	CHAP_DLC_14,
	CHAP_DLC_15,
	CHAP_DLC_16,
	CHAP_DLC_17,
	CHAP_DLC_18,
};

/**
 * The kinds of progress that a player can make towards various goals (these are rewarded with toasts).
 */
enum EProgressType
{
	ePROG_Achievement,
	ePROG_Collectible, //A.K.A Discoverable
	ePROG_Unlockable
};

/** Enum of all the discoverable in gears */
enum EGearDiscoverableType
{
	eDISC_None,
	eDISC_Tut_Newspaper,
	eDISC_Tut_AmbulanceLog,
	eDISC_Hosp_MedicalFile,
	eDISC_Hosp_DrNote,
	eDISC_Hosp_COGLetterhead,
	eDISC_Hosp_COGTag,
	eDISC_Hosp_MartialLaw,
	eDISC_Assault_GrindSpec,
	eDISC_Assault_AmmoForm,
	eDISC_Landown_Newspaper,
	eDISC_Landown_COGTag,
	eDISC_Landown_PackSlip,
	eDISC_Landown_Tombstone,
	eDISC_Interv_GrindJournal,
	eDISC_Interv_KantusScroll,
	eDISC_Interv_LocustNeck,
	eDISC_Rescue_HelpAd,
	eDISC_Rescue_COGTag,
	eDISC_Rescue_Journal,
	eDISC_Riftworm_MagCover,
	eDISC_Riftworm_COGTag,
	eDISC_Outpost_MedFile,
	eDISC_Outpost_Memo,
	eDISC_Outpost_CompReadout,
	eDISC_Outpost_Journal,
	eDISC_Outpost_CaptMarks,
	eDISC_Leviathan_Journal,
	eDISC_Maria_Jornal,
	eDISC_Maria_PrisCatalog,
	eDISC_Highway_JailSched,
	eDISC_Highway_FingerNeck,
	eDISC_Nexis_LocCalendar,
	eDISC_Nexis_DefPlans,
	eDISC_Palace_LocInvMap,
	eDISC_Palace_LocWormStat,
	eDISC_Palace_Tablets,
	eDISC_Jacinto_ReconRep,
	eDISC_Jacinto_COGTag,
	eDISC_Sinkhole_Newspaper,
	eDISC_Sinkhole_COGTag,
	eDISC_Sinkhole_Journal,
};

/** List of unlockables in Gears2 */
enum EGearUnlockable
{
	eUNLOCK_InsaneDifficulty,
	eUNLOCK_Character_Dizzy,
	eUNLOCK_Character_Kantus,
	eUNLOCK_Character_Tai,
	eUNLOCK_Character_PalaceGuard, // Actually unlocks the flame grenadier
	eUNLOCK_Character_Skorge,
	eUNLOCK_Character_Carmine1,
	eUNLOCK_Character_Raam,
	eUNLOCK_Character_Minh,
	eUNLOCK_Character_DLC1,
	eUNLOCK_Character_DLC2,
	eUNLOCK_Character_DLC3,
	eUNLOCK_Character_DLC4,
	eUNLOCK_Character_DLC5,
	eUNLOCK_Character_None, // Used for datastores to determine if a character is/is not unlockable
};

/** All of the achievements in the game */
enum EGearAchievement
{
	eGA_Invalid,
	eGA_GreenAsGrass,				// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_ItsATrap,					// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_EscortService,				// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_GirlAboutTown,				// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_ThatSinkingFeeling,			// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_Freebaird,					// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_HeartBroken,				// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_LongitudeAndAttitude,		// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_TanksForTheMemories,		// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_WaterSports,				// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_SavedMaria,					// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_WrappedInBeacon,			// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_HaveFunStormingTheCastle,	// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_AndTheHorseYourRodeInOn,	// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_TheyJustKeepComing,			// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_BrumakRodeo,				// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DoesThisLookInfectedToYou,	// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_Reservist,					// Handled by: UnlockChapterByDifficulty() in GearProfileSettings
	eGA_NonCommissionedOfficer,		// Handled by: UnlockChapterByDifficulty() in GearProfileSettings
	eGA_CommissionedOfficer,		// Handled by: UnlockChapterByDifficulty() in GearProfileSettings
	eGA_Commander,					// Handled by: UnlockChapterByDifficulty() in GearProfileSettings
	eGA_Tourist,					// Handled by: UpdateDiscoverableProgression() in GearProfileSettings (Uses Progression UI)
	eGA_PackRat,					// Handled by: UpdateDiscoverableProgression() in GearProfileSettings (Uses Progression UI)
	eGA_Completionist,				// Handled by: UpdateDiscoverableProgression() in GearProfileSettings (Uses Progression UI)
	eGA_DomCurious,					// Handled by: UpdateCoopChapterCompleteProgression()
	eGA_Domination,					// Handled by: UpdateCoopChapterCompleteProgression() in GearProfileSettings (Uses Progression UI)
	eGA_ICantQuitYouDom,			// Handled by: UpdateCoopChapterCompleteProgression() in GearProfileSettings (Uses Progression UI)
	eGA_CrossedSwords,
	eGA_PoundOfFlesh,				// Handled by: SavedByAMeatshieldCallback() in GearPawn (Uses Progression UI)
	eGA_DIYTurret,					// Handled by: CheckForMinigunAchievement() in GearPC (Uses Progression UI)
	eGA_ShockAndAwe,				// Handled by: CheckForMortarAchievement() in GearPC (Uses Progression UI)
	eGA_SaidTheSpiderToTheFly,		// Handled by: CheckForStickyAchievement() in GearPC (Uses Progression UI)
	eGA_PeekABoo,					// Handled by: CheckForBoomShieldKillAchievement() in GearPC (Uses Progression UI)
	eGA_ShakeAndBake,				// Handled by: CheckForFlameThrowerAchievement() in GearPC (Uses Progression UI)
	eGA_ActiveReload,				// Handled by: ActiveReloadSuccess of GearHUD_Base
	eGA_TickerLicker,				// Handled by: TakeDamage of GearPawn_LocustTickerBase
	eGA_VarietyIsTheSpiceOfDeath,	// Handled by: UpdateKillingProgression() in GearProfileSettings (Uses Progression UI)
	eGA_MultitalentedExecutioner,	// Handled by: UpdateExecutionProgression() in GearProfileSettings (Uses Progression UI)
	eGA_Seriously2,					// Handled by: UpdateKillingProgression() in GearProfileSettings (Uses Progression UI)
	eGA_Photojournalist,			// Handled by: UploadScreenshotComplete() in GearPC
	eGA_ItTakesTwo,					// Handled by: GameStatusHasReplicated() in GearUISceneEndOfRound (Uses Progression UI)
	eGA_TheOldBallAndChain,			// Handled by: GameStatusHasReplicated() in GearUISceneEndOfRound (Uses Progression UI)
	eGA_ItsGoodToBeTheKing,			// Handled by: GameStatusHasReplicated() in GearUISceneEndOfRound (Uses Progression UI)
	eGA_YouGoAheadIllBeFine,		// Handled by: GameStatusHasReplicated() in GearUISceneEndOfRound (Uses Progression UI)
	eGA_BackToBasic,				// Handled by: UpdateTrainingProgression() in GearProfileSettings (Uses Progression UI)
	eGA_Martyr,						// Handled by: UpdateKillingProgression() in GearProfileSettings (Uses Progression UI)
	eGA_Party1999,					// Handled by: GameStatusHasReplicated() in GearUISceneEndOfRound (Uses Progression UI)
	eGA_AroundTheWorld,				// Handled by: UpdateMPMatchWonProgression() in GearProfileSettings (Uses Progression UI)
	eGA_SurvivedToTen,				// Handled by: UnlockHordeAchievements() in GearGameHorde_Base (Uses Progression UI)
	eGA_SurvivedToFifty,			// Handled by: UnlockHordeAchievements() in GearGameHorde_Base (Uses Progression UI)
	eGA_DLC1,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC2,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC3,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC4,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC5,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC6,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC7,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC8,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC9,						// Handled by: SeqAct_GearAchievementUnlock kismet action
	eGA_DLC10,						// Handled by: SeqAct_GearAchievementUnlock kismet action
};

/** Enum of all the maps the game will ship with (needed for achievement tracking) */
enum EGearMapsShipped
{
	eGEARMAP_None,
	eGEARMAP_MP1,
	eGEARMAP_MP2,
	eGEARMAP_MP3,
	eGEARMAP_MP4,
	eGEARMAP_MP5,
	eGEARMAP_MP6,
	eGEARMAP_MP7,
	eGEARMAP_MP8,
	eGEARMAP_MP9,
	eGEARMAP_MP10,
};

/**
 * Enum of all the weapons in the game (at least the ones we shipped with)
 * NOTE: the Disabled value is required for UI so we can disable weapons in the pickup factories
 *			and should remain at the bottom of the list
 */
enum EGearWeaponType
{
	eGEARWEAP_FragGrenade,
	eGEARWEAP_InkGrenade,
	eGEARWEAP_SmokeGrenade,
	eGEARWEAP_Boomshot,
	eGEARWEAP_Flame,
	eGEARWEAP_Sniper,
	eGEARWEAP_Bow,
	eGEARWEAP_Mulcher,
	eGEARWEAP_Mortar,
	eGEARWEAP_HOD,
	eGEARWEAP_Gorgon,
	eGEARWEAP_Boltok,
	eGEARWEAP_Pistol,
	eGEARWEAP_Lancer,
	eGEARWEAP_Hammerburst,
	eGEARWEAP_Shotgun,
	eGEARWEAP_Shield,
	eGEARWEAP_Disabled,
	eGEARWEAP_GoldLancer, // Must be after because you can't swap it in MP
	eGEARWEAP_GoldHammer, // Must be after because you can't swap it in MP
};

/** Enum of all the execution types in the game (needed for achievement tracking) */
enum EGearExecutionType
{
	eGET_Curbstomp,
	eGET_MeatShield,
	eGET_ShieldBash,
	eGET_QuickGeneric,
	eGET_QuickBow,
	eGET_QuickShotgun,
	eGET_QuickSniper,
	eGET_LongGeneric,
	eGET_LongBoltok,
	eGET_LongBow,
	eGET_LongSniper,
};

/** Enum of all the training scenarios we have in Training Grounds */
enum EGearTrainingType
{
	eGEARTRAIN_Basic,
	eGEARTRAIN_Execution,
	eGEARTRAIN_Respawn,
	eGEARTRAIN_Objective,
	eGEARTRAIN_Meatflag,
};

/**
* prioritization of different types of speech.
* used to determine what interrupts what.	lowest to highest.
*/
enum ESpeechPriority
{
	Speech_None,
	Speech_Effort,
	Speech_GUDS,
	Speech_Scripted,
	Speech_Immediate,
};

enum ESpeechInterruptCondition
{
	/** interrupt if request is higher pri than current (this is the default, keep it at zero) */
	SIC_IfHigher,
	/** never interrupt */
	SIC_Never,
	/** interrupt if request is same or higher priority than current */
	SIC_IfSameOrHigher,
	/** always interrupt */
	SIC_Always
};

enum ESpeakLineBroadcastFilter
{
	/** No filtering, everyone can hear the speech. */
	SLBFilter_None,
	/** Only the speaking players can hear. */
	SLBFilter_SpeakerOnly,
	/** Only the speaking player and his teammates can hear. */
	SLBFilter_SpeakerTeamOnly,
	/** Only the speaking player and the addressee can hear. */
	SLBFilter_SpeakerAndAddresseeOnly,
};

/** Enum of the inputs for this action */
enum EManageTutorialInputs
{
	eMTINPUT_AddTutorial,
	eMTINPUT_RemoveTutorial,
	eMTINPUT_StartTutorial,
	eMTINPUT_StopTutorial,
	eMTINPUT_CompleteTutorial,
	eMTINPUT_SystemOn,
	eMTINPUT_SystemOff,
	eMTINPUT_AutosOn,
	eMTINPUT_AutosOff,
};

/** Defines a camera-animation-driven screenshake. */
struct native ScreenShakeAnimStruct
{
	var() CameraAnim	Anim;

	/** If TRUE, code will choose which anim to play based on relative location to the player.  Anim is treated as "front" in this case. */
	var() bool			bUseDirectionalAnimVariants;
	var() CameraAnim	Anim_Left;
	var() CameraAnim	Anim_Right;
	var() CameraAnim	Anim_Rear;

	var() float			AnimPlayRate;
	var() float			AnimScale;
	var() float			AnimBlendInTime;
	var() float			AnimBlendOutTime;

	/**
	* If TRUE, play a random snippet of the animation of length RandomSegmentDuration.  Implies bLoop and bRandomStartTime = TRUE.
	* If FALSE, play the full anim once, non-looped.
	*/
	var() bool			bRandomSegment;
	var() float			RandomSegmentDuration;

	/** TRUE to only allow a single instance of the specified anim to play at any given time. */
	var() bool			bSingleInstance;

	structdefaultproperties
	{
		AnimPlayRate=1.f
		AnimScale=1.f
		AnimBlendInTime=0.2f
		AnimBlendOutTime=0.2f
	}
};


/** Shake start offset parameter */
enum EShakeParam
{
	ESP_OffsetRandom,	// Start with random offset (default)
	ESP_OffsetZero,		// Start with zero offset
};

/** Shake vector params */
struct native ShakeParams
{
	var() EShakeParam	X, Y, Z;

	var transient const byte Padding;
};

/** Defines a code-driven (sinusoidal) screenshake */
struct native ScreenShakeStruct
{
	/** Time in seconds to go until current screen shake is finished */
	var()	float	TimeToGo;
	/** Duration in seconds of current screen shake */
	var()	float	TimeDuration;

	/** view rotation amplitude */
	var()	vector	RotAmplitude;
	/** view rotation frequency */
	var()	vector	RotFrequency;
	/** view rotation Sine offset */
	var		vector	RotSinOffset;
	/** rotation parameters */
	var()	ShakeParams	RotParam;

	/** view offset amplitude */
	var()	vector	LocAmplitude;
	/** view offset frequency */
	var()	vector	LocFrequency;
	/** view offset Sine offset */
	var		vector	LocSinOffset;
	/** location parameters */
	var()	ShakeParams	LocParam;

	/** FOV amplitude */
	var()	float	FOVAmplitude;
	/** FOV frequency */
	var()	float	FOVFrequency;
	/** FOV Sine offset */
	var		float	FOVSinOffset;
	/** FOV parameters */
	var()	EShakeParam	FOVParam;

	/**
	 * Unique name for this shake.  Only 1 instance of a shake with a particular
	 * name can be playing at once.  Subsequent calls to add the shake will simply
	 * restart the existing shake with new parameters.  This is useful for animating
	 * shake parameters.
	 */
	var()	Name		ShakeName;

	/** True to use TargetingDampening multiplier while player is targeted, False to use global defaults (see TargetingAlpha). */
	var()	bool		bOverrideTargetingDampening;

	/** Amplitude multiplier to apply while player is targeting.  Ignored if bOverrideTargetingDampening == FALSE */
	var()	float		TargetingDampening;

	structdefaultproperties
	{
		TimeDuration=1.f
		RotAmplitude=(X=100,Y=100,Z=200)
		RotFrequency=(X=10,Y=10,Z=25)
		LocAmplitude=(X=0,Y=3,Z=5)
		LocFrequency=(X=1,Y=10,Z=20)
		FOVAmplitude=2
		FOVFrequency=5
		ShakeName=""
	}
};

/** struct to store the date and time the checkpoint was created */
struct native CheckpointTime
{
	var int SecondsSinceMidnight;
	var int Day;
	var int Month;
	var int Year;
};

/**
 * This stores a 64-bit DateTime value for use by screenshots.
 * qword cannot be used due to it's variables being declared as const.
 */
struct native DateTime
{
	var int A, B;
};

/**
 * This is the info that is stored for each player in the game when a screenshot is taken.
 * If the contents of this struct changes, the two SerializeInfo() functions in GearScreenshotIO.cpp must be updated.
 */
struct native ScreenshotPlayerInfo
{
	/** The nickname for bots only. */
	var string Nick;
	/** The player's XUID.  For bot's this is set to zero. */
	var UniqueNetId Xuid;
	/** If true, the player is visible within the screenshot. */
	var bool IsVisible;
	/**
	 * If IsVisible is true, then this contains the coordinates for the player's screen-space bounding box surrounding.
	 * Only the X and Y coordinates will be valid.
	 */
	var box ScreenSpaceBoundingBox;
	/** The location of the player in world-space. */
	var vector Location;
};

/**
 * This is the info that is stored for each screenshot when it is taken.
 * If the contents of this struct changes, the two SerializeInfo() functions in GearScreenshotIO.cpp must be updated.
 */
struct native ScreenshotInfo
{
	/** The time at which the screenshot was taken, based on GWorld->GetTimeSeconds(). */
	var float WorldTime;
	/** An optional description of the screenshot. */
	var string Description;
	/** The screenshot's calculated rating. */
	var int Rating;
	/** The gametype being played when the screenshot was taken. */
	/** DEPRECATED: don't use anymore. */
	var string GameType;
	/** The friendly name of the gametype being played when the screenshot was taken. */
	var string GameTypeFriendly;
	/** The EGearMPTypes value for the gametype. */
	var int GameTypeId;
	/** The eScreenshotMatchType value for this shot. */
	var int MatchType;
	/** The name of the map being played when the screenshot was taken. */
	var string MapName;
	/** The friendly name of the map being played when the screenshot was taken. */
	var string MapNameFriendly;
	/** The "real world" time at which the screenshot was taken. */
	var DateTime Realtime;
	/** The stats ID for this match. */
	var guid MatchId;
	/** The camera's location when the shot was taken. */
	var vector CamLoc;
	/** The camera's rotation when the shot was taken. */
	var rotator CamRot;
	/** Screenshot width. */
	var int Width;
	/** Screenshot height. */
	var int Height;
	/** Info for all the players (including bots) in the screenshot. */
	var array<ScreenshotPlayerInfo> Players;
};

/** Represents screenshot info for a shot that has been saved to disk. */
struct native SavedScreenshotInfo
{
	/** The unique ID for this screenshot on disk. */
	var guid ID;
	/** The device on which this screenshot exists. */
	var int DeviceID;
	/** The name of the map being played when the screenshot was taken. */
	var string MapName;
	/** The gametype being played when the screenshot was taken. */
	var string GameType;
	/** The screenshot's rating. */
	var int Rating;
	/** The date the screenshot was taken. */
	var string Date;
	/** The parsed date, to speed up comparisons. */
	var int Year;
	var byte Month;
	var byte Day;
};

/** Mapping of GearDamageType class to EGearWeaponType */
var const array<class<GearDamageType> > DamageToWeaponMap;

/** Mapping of GearDamageType class to EGearExecutionType */
var const array<class<GearDamageType> > DamageToExecutionMap;

/**
 * This is a list of Physical Materials that are in the vast majority of maps.
 * By referencing them here we will not have to duplicate them into all of the maps and will save diskpace
 **/
var const array<PhysicalMaterial> AlwaysReferencedPhysMaterials;

/** Per-damagetype damage modifiers. Campaign/Horde only. */
struct native PerDamageTypeMod
{
	var config name DamageTypeName;
	var config float Multiplier;
};

/** base "p" level for each chapter point */
var const config array<name> ChapterLevelNames;

/** Sets WeaponType to the EGearWeaponType it corresponds to in the DamageToWeaponMap via a GearDamageType */
static final function bool GetWeaponType( class<GearDamageType> DmgType, out EGearWeaponType WeaponType )
{
	local int Idx;

	if (DmgType != None)
	{
		// Find the index of the damage type in the map
		for ( Idx = 0; Idx < class'GearTypes'.default.DamageToWeaponMap.length; Idx++ )
		{
			if ( class'GearTypes'.default.DamageToWeaponMap[Idx] == DmgType )
			{
				WeaponType = EGearWeaponType(Idx);
				return true;
			}
		}
	}
	return false;
}

/** Sets ExecutionType to the EGearExecutionType it corresponds to in the DamageToExecutionMap via a GearDamageType */
static final function bool GetExecutionType( class<GearDamageType> DmgType, out EGearExecutionType ExecutionType )
{
	local int Idx;

	if ( ClassIsChildOf(DmgType, class'GDT_Execution_Base') )
	{
		// Find the index of the damage type in the map
		for ( Idx = 0; Idx < class'GearTypes'.default.DamageToExecutionMap.length; Idx++ )
		{
			if ( class'GearTypes'.default.DamageToExecutionMap[Idx] == DmgType )
			{
				ExecutionType = EGearExecutionType(Idx);
				return true;
			}
		}
	}

	return false;
}

defaultproperties
{
	AlwaysReferencedPhysMaterials.Add( PhysicalMaterial'GearPhysMats.Stone' )
	AlwaysReferencedPhysMaterials.Add( PhysicalMaterial'GearPhysMats.Wood' )

	DamageToWeaponMap(eGEARWEAP_FragGrenade)=class'GDT_FragGrenade'
	DamageToWeaponMap(eGEARWEAP_InkGrenade)=class'GDT_InkGrenade'
	DamageToWeaponMap(eGEARWEAP_SmokeGrenade)=class'GDT_SmokeGrenade'
	DamageToWeaponMap(eGEARWEAP_Boomshot)=class'GDT_Boomshot'
	DamageToWeaponMap(eGEARWEAP_Flame)=class'GDT_FlamethrowerSpray'
	DamageToWeaponMap(eGEARWEAP_Sniper)=class'GDT_SniperRifle'
	DamageToWeaponMap(eGEARWEAP_Bow)=class'GDT_TorqueBow_Explosion'
	DamageToWeaponMap(eGEARWEAP_Mulcher)=class'GDT_Mulcher'
	DamageToWeaponMap(eGEARWEAP_Mortar)=class'GDT_Mortar'
	DamageToWeaponMap(eGEARWEAP_HOD)=class'GDT_HOD'
	DamageToWeaponMap(eGEARWEAP_Gorgon)=class'GDT_LocustBurstPistol'
	DamageToWeaponMap(eGEARWEAP_Boltok)=class'GDT_LocustPistol'
	DamageToWeaponMap(eGEARWEAP_Pistol)=class'GDT_COGPistol'
	DamageToWeaponMap(eGEARWEAP_Lancer)=class'GDT_AssaultRifle'
	DamageToWeaponMap(eGEARWEAP_Hammerburst)=class'GDT_LocustAssaultRifle'
	DamageToWeaponMap(eGEARWEAP_Shotgun)=class'GDT_Shotgun'
	DamageToWeaponMap(eGEARWEAP_GoldHammer)=class'GDT_LocustAssaultRifle'
	DamageToWeaponMap(eGEARWEAP_GoldLancer)=class'GDT_AssaultRifle'

	DamageToExecutionMap(eGET_Curbstomp)=class'GDT_CurbStomp'
	DamageToExecutionMap(eGET_MeatShield)=class'GDT_NeckBreak'
	DamageToExecutionMap(eGET_ShieldBash)=class'GDT_ShieldExecute'
	DamageToExecutionMap(eGET_QuickGeneric)=class'GDT_QuickExecution'
	DamageToExecutionMap(eGET_QuickBow)=class'GDT_QuickExecution_Bow'
	DamageToExecutionMap(eGET_QuickShotgun)=class'GDT_QuickExecution_Shotgun'
	DamageToExecutionMap(eGET_QuickSniper)=class'GDT_QuickExecution_Sniper'
	DamageToExecutionMap(eGET_LongGeneric)=class'GDT_Execution_PunchFace'
	DamageToExecutionMap(eGET_LongBoltok)=class'GDT_LongExecution_Boltok'
	DamageToExecutionMap(eGET_LongBow)=class'GDT_LongExecution_Bow'
	DamageToExecutionMap(eGET_LongSniper)=class'GDT_LongExecution_Sniper'
}
