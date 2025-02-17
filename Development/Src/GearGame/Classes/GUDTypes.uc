/**
 * Class: GUDTypes
 * Various enums and decalarations universal to the GUDS system.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GUDTypes extends Object
	abstract
	native(Sound);


enum EGUDEventID
{
	GUDEvent_None,
	GUDEvent_EnteredCombat,
	GUDEvent_LostTarget,
	GUDEvent_FoundTarget,
	GUDEvent_KilledEnemyGeneric,
	GUDEvent_KilledEnemyChainsaw,
	GUDEvent_KilledEnemyHeadShot,
	GUDEvent_KilledEnemyMelee,
	GUDEvent_KilledEnemyCurbstomp,
	GUDEvent_KilledEnemyExecution,
	GUDEvent_KilledEnemyHOD,
	GUDEvent_DamagedEnemy,
	GUDEvent_DamagedEnemyHeavy,
	GUDEvent_DamageTeammate,
	GUDEvent_CauseEnemyStumble,
	GUDEvent_TransToRetreatAI,			// really means "fallback"
	GUDEvent_TransToHoldAI,				// unused
	GUDEvent_TransToAdvanceAI,
	GUDEvent_TransToKamikazeAI,
	GUDEvent_TransToMeleeAI,
	GUDEvent_TransToMeleeAIOffscreen,
	GUDEvent_TransToFlankAI,			// unused
	GUDEvent_TransToFleeHODAI,			// unused
	GUDEvent_ThrowingSmokeGrenade,
	GUDEvent_ThrowingFragGrenade,
	GUDEvent_FailedActiveReload,
	GUDEvent_SucceededActiveReload,
	GUDEvent_Reloaded,
	GUDEvent_PickedUpAmmo,
	GUDEvent_PickedUpNewWeapon,
	GUDEvent_PickedUpCollectible,
	GUDEvent_PickedUpGrenades,
	GUDEvent_PlayerHasntMoved,
	GUDEvent_NeedAmmo,
	GUDEvent_NeedAmmoGrenade,

	GUDEvent_GenericWentDown,
	GUDEvent_MarcusWentDown,
	GUDEvent_DomWentDown,
	GUDEvent_BairdWentDown,
	GUDEvent_MinhWentDown,
	GUDEvent_GusWentDown,
	GUDEvent_HoffmanWentDown,
	GUDEvent_CarmineWentDown,

	GUDEvent_GenericNeedsRevived,
	GUDEvent_MarcusNeedsRevived,
	GUDEvent_DomNeedsRevived,
	GUDEvent_BairdNeedsRevived,
	GUDEvent_MinhNeedsRevived,
	GUDEvent_GusNeedsRevived,
	GUDEvent_HoffmanNeedsRevived,
	GUDEvent_CarmineNeedsRevived,

	GUDEvent_Revived,
	GUDEvent_Attack,
	GUDEvent_NoticedManyEnemies,
	GUDEvent_NoticedDrone,
	GUDEvent_NoticedBoomer,
	GUDEvent_NoticedTheron,
	GUDEvent_NoticedPalaceGuard,
	GUDEvent_NoticedWretch,
	GUDEvent_NoticedHunter,
	GUDEvent_NoticedKantus,
	GUDEvent_NoticedMauler,			// flail boomer
	GUDEvent_NoticedGrinder,		// gatling boomer
	GUDEvent_NoticedBloodmounts,
	GUDEvent_NoticedTickers,
	GUDEvent_NoticedSnipers,
	GUDEvent_NoticedEnemyGeneric,
	GUDEvent_NoticedReinforcements,

	GUDEvent_CombatLull,

	GUDEvent_EHoleOpened,
	GUDEvent_EHoleOpenReminder,
	GUDEvent_EHoleClosedWithGrenade,
	GUDEvent_EHoleGrenadeMissed,

	GUDEvent_VersusRoundBegunCOG,				// one per team to ensure both teams say something
	GUDEvent_VersusRoundBegunLocust,

	// response event to a noticed-enemy callout, can be something like "Where?" or "I see him!"
	// instigator will be the teammate that said the callout, recipient is the enemy that was spotted
	GUDEvent_TeammateAnnouncedEnemySpotted,
	GUDEvent_TeammateAnnouncedEnemyDir,

	GUDEvent_EnteredCover,						// Instigator entered cover, no recipient, role is always enemywitness

	// a teammate requested the location of an enemy who was recently spotted (e.g., said "where is he?")
	// instigator is the teammate making the request, recipient is the enemy in question
	// can be directional, or location-specific, e.g. "up high!" or "in the window!"
	GUDEvent_TeammateRequestedEnemyLocDesc,		// OBSOLETE
	GUDEvent_TeammateRequestedEnemyDir,			// OBSOLETE
	GUDEvent_DecidedToAnounceEnemyLocDesc,		// OBSOLETE. shares actions with GUDEvent_TeammateRequestedEnemyLocDesc
	GUDEvent_DecidedToAnounceEnemyDir,			// OBSOLETE.  shares actions with GUDEvent_TeammateRequestedEnemyDir

	// Meatflag-specific events
	GUDEvent_MeatFlagWentDBNO,					// MeatFlag is recipient, killer is Instigator.
	GUDEvent_MeatFlagGrabbedByCOG,				// Meatflag is recipient, grabber is Instigator.
	GUDEvent_MeatFlagGrabbedByLocust,
	GUDEvent_MeatFlagStillHeldByCOG,			// Meatflag is recipient, captor is Instigator.
	GUDEvent_MeatFlagStillHeldByLocust,
	GUDEvent_MeatFlagReleasedByCOG,				// Meatflag is recipient, former captor is Instigator.
	GUDEvent_MeatFlagReleasedByLocust,	
	GUDEvent_MeatFlagCapturedByCOG,				// Meatflag is recipient, captor is Instigator.
	GUDEvent_MeatFlagCapturedByLocust,
	GUDEvent_MeatFlagKilledFormerCOGCaptor,		// Meatflag is Instigator, dead guy is recipient.
	GUDEvent_MeatFlagKilledFormerLocustCaptor,

	GUDEvent_PickedUpMeatShield,				// Instigator picked up Recipient as MeatShield
	GUDEvent_KilledEnemyFlamethrower,			// Instigator killed Recipient with FlameThrower
	GUDEvent_PickedUpFlamethrower,				// Instigator picked up Flamethrower weapon, no recipient

	GUDEvent_GatlingOverheated,					// Instigator's gatling gun overheated.  No recipient.
	GUDEvent_MortarLaunched,					// Instigator launched a mortar, no recipient.
	GUDEvent_ThrowingInkGrenade,				// Instigator threw ink grenade.
	GUDEvent_StuckExplosiveToEnemy,				// Instigator stuck Recipient with explosive (e.g. grenade, arrow)
	GUDEvent_GrenadeTrapSet,					// Instigator set a sticky grenade trap to the world, no recipient. 
	GUDEvent_ExecutedMeatShield,				// Instigator executed Recipient, who was his meatshield at the time

	GUDEvent_DBNOPawnCrawlingAway,				// Instigator is DBNO Pawn, no recipient
};


/**
 * To avoid confusion re who is the instigator and whatnot, think of it like so:
 *		"Event happened to RECIPIENT, caused by INSTIGATOR, observed by WITNESS"
 *
 * Which role an action is designed for is denoted by a suffix on the action name:
 *		I = Instigator
 *		R = Recipient
 *		TW = Teammate Witness (same team as Instigator)
 *		EW = Enemy Witness (different team as Instigator)
 *
 * Note that an "action" conceptually is essentially a bundle of lines.  Actions need
 * not be associated with an event, although events do play actions.
 */
enum EGUDActionID
{
	// this is going to be a long list, but ok
	GUDAction_None,

	GUDAction_EnteredCombat_I,
	GUDAction_LostTarget_I,
	GUDAction_FoundTarget_I,
	GUDAction_FoundTarget_R,
	GUDAction_KilledEnemyGeneric_I,
	GUDAction_KilledEnemyGeneric_TW,
	GUDAction_KilledEnemyChainsaw_I,
	GUDAction_KilledEnemyChainsaw_TW,
	GUDAction_KilledEnemyHeadShot_I,
	GUDAction_KilledEnemyHeadShot_TW,
	GUDAction_KilledEnemyMelee_I,
	GUDAction_KilledEnemyMelee_TW,
	GUDAction_KilledEnemyCurbstomp_I,
	GUDAction_KilledEnemyExecution_I,
	GUDAction_KilledEnemyHOD_I,
	GUDAction_KilledEnemyHOD_TW,
	GUDAction_DamagedEnemy_I,
	GUDAction_DamagedEnemy_R,
	GUDAction_DamagedEnemyHeavy_I,
	GUDAction_DamagedEnemyHeavy_R,
	GUDAction_DamageTeammate_I,
	GUDAction_DamageTeammate_R,
	GUDAction_CauseEnemyStumble_I,
	GUDAction_CauseEnemyStumble_R,
	GUDAction_TransToFleeAI_I,
	GUDAction_TransToFleeAI_EW,
	GUDAction_TransToRetreatAI_I,
	GUDAction_TransToRetreatAI_EW,
	GUDAction_TransToHoldAI_I,
	GUDAction_TransToHoldAI_EW,
	GUDAction_TransToAdvanceAI_I,
	GUDAction_TransToAdvanceAI_EW,
	GUDAction_TransToKamikazeAI_I,
	GUDAction_TransToKamikazeAI_EW,
	GUDAction_TransToMeleeAI_I,
	GUDAction_TransToMeleeAI_EW,
	GUDAction_TransToFlankAI_I,
	GUDAction_TransToFlankAI_EW,
	GUDAction_TransToFleeHODAI_I,
	GUDAction_TransToFleeHODAI_EW,
	GUDAction_ThrowingSmokeGrenade_I,
	GUDAction_ThrowingSmokeGrenade_EW,
	GUDAction_ThrowingFragGrenade_I,
	GUDAction_ThrowingFragGrenade_EW,
	GUDAction_BlockingMovement_I,
	GUDAction_BlockingMovement_R,
	GUDAction_BlockingMovement_TW,
	GUDAction_NoRecentDamage_I,
	GUDAction_FailedActiveReload_I,
	GUDAction_SucceededActiveReload_I,
	GUDAction_Reloaded_I,
	GUDAction_PickedUpAmmo_I,
	GUDAction_PickedUpNewWeapon_I,
	GUDAction_PickedUpCollectible_I,
	GUDAction_PickedUpGrenades_I,
	GUDAction_PlayerHasntMoved_I,
	GUDAction_PlayerHasntMoved_TW,
	GUDAction_MysteriousEvent_TW,
	GUDAction_SurprisingEvent_TW,
	GUDAction_DangerousEvent_TW,
	GUDAction_NeedAmmo_I,
	GUDAction_NeedAmmoGrenade_I,

	GUDAction_GenericWentDown_I,
	GUDAction_GenericWentDown_TW,
	GUDAction_MarcusWentDown_I,
	GUDAction_MarcusWentDown_TW,
	GUDAction_DomWentDown_I,
	GUDAction_DomWentDown_TW,
	GUDAction_BairdWentDown_I,
	GUDAction_BairdWentDown_TW,
	GUDAction_MinhWentDown_I,
	GUDAction_MinhWentDown_TW,
	GUDAction_GusWentDown_I,
	GUDAction_GusWentDown_TW,
	GUDAction_HoffmanWentDown_I,
	GUDAction_HoffmanWentDown_TW,
	GUDAction_CarmineWentDown_I,
	GUDAction_CarmineWentDown_TW,

	GUDAction_GenericNeedsRevived_I,
	GUDAction_GenericNeedsRevived_TW,
	GUDAction_MarcusNeedsRevived_I,
	GUDAction_MarcusNeedsRevived_TW,
	GUDAction_DomNeedsRevived_I,
	GUDAction_DomNeedsRevived_TW,
	GUDAction_BairdNeedsRevived_I,
	GUDAction_BairdNeedsRevived_TW,
	GUDAction_MinhNeedsRevived_I,
	GUDAction_MinhNeedsRevived_TW,
	GUDAction_GusNeedsRevived_I,
	GUDAction_GusNeedsRevived_TW,
	GUDAction_HoffmanNeedsRevived_I,
	GUDAction_HoffmanNeedsRevived_TW,
	GUDAction_CarmineNeedsRevived_I,
	GUDAction_CarmineNeedsRevived_TW,

	GUDAction_Revived_I,
	GUDAction_Revived_R,
	GUDAction_Attack_I,


	/** Enemy spotted notifications.  Instigator is None, Recipient is the enemy that was spotted. */
	GUDAction_NoticedTwoEnemies_TW,
	GUDAction_NoticedThreeEnemies_TW,
	GUDAction_NoticedFourEnemies_TW,
	GUDAction_NoticedManyEnemies_TW,
	GUDAction_NoticedDrone_TW,
	GUDAction_NoticedBoomer_TW,
	GUDAction_NoticedTheron_TW,
	GUDAction_NoticedPalaceGuard_TW,
	GUDAction_NoticedWretch_TW,
	GUDAction_NoticedHunter_TW,
	GUDAction_NoticedKantus_TW,
	GUDAction_NoticedMauler_TW,			// flail boomer
	GUDAction_NoticedGrinder_TW,		// gatling boomer
	GUDAction_NoticedBloodmounts_TW,
	GUDAction_NoticedTickers_TW,
	GUDAction_NoticedSnipers_TW,
	GUDAction_NoticedEnemyGeneric_TW,
	GUDAction_NoticedReinforcements_TW,

	GUDAction_EnemiesReducedToZero_TW,
	GUDAction_EnemiesReducedToOne_TW,
	GUDAction_EnemiesReducedToTwo_TW,
	GUDAction_EnemiesReducedToThree_TW,
	GUDAction_EnemiesReducedToFour_TW,
	GUDAction_CombatLull_TW,
	GUDAction_CombatLull_EW,
	GUDAction_EHoleOpened_TW,
	GUDAction_EHoleOpenReminder_TW,
	GUDAction_EHoleClosedWithGrenade_I,
	GUDAction_EHoleClosedWithGrenade_TW,
	GUDAction_EHoleGrenadeMissed_I,
	GUDAction_EHoleGrenadeMissed_TW,
	GUDAction_VersusRoundBegunCOG_TW,
	GUDAction_VersusRoundBegunLocust_EW,

	GUDAction_EnemySpottedResponse_TW,
	GUDAction_EnemyDirAnnouncedResponse_TW,

	GUDAction_EnemyLocationCallout_Above,
	GUDAction_EnemyLocationCallout_Below,
	GUDAction_EnemyLocationCallout_Right,
	GUDAction_EnemyLocationCallout_Left,
	GUDAction_EnemyLocationCallout_Ahead,
	GUDAction_EnemyLocationCallout_Behind,

	GUDAction_EnemyLocationCallout_InWindow,
	GUDAction_EnemyLocationCallout_InDoorway,
	GUDAction_EnemyLocationCallout_BehindCar,
	GUDAction_EnemyLocationCallout_BehindTruck,
	GUDAction_EnemyLocationCallout_OnTruck,
	GUDAction_EnemyLocationCallout_BehindBarrier,
	GUDAction_EnemyLocationCallout_BehindColumn,
	GUDAction_EnemyLocationCallout_BehindCrate,
	GUDAction_EnemyLocationCallout_BehindWall,
	GUDAction_EnemyLocationCallout_BehindStatue,
	GUDAction_EnemyLocationCallout_BehindSandbags,
	GUDAction_EnemyLocationCallout_InTheOpen,

	// meatflag actions
	GUDAction_MeatflagWentDBNO_R,
	GUDAction_MeatFlagGrabbedByCOG_R,
	GUDAction_MeatFlagGrabbedByLocust_R,
	GUDAction_MeatFlagStillHeldByCOG_R,
	GUDAction_MeatFlagStillHeldByLocust_R,
	GUDAction_MeatFlagReleasedByCOG_R,
	GUDAction_MeatFlagReleasedByLocust_R,
	GUDAction_MeatFlagCapturedByCOG_R,
	GUDAction_MeatFlagCapturedByLocust_R,
	GUDAction_MeatFlagKilledFormerCOGCaptor_I,
	GUDAction_MeatFlagKilledFormerLocustCaptor_I,

	GUDAction_PickedUpMeatShield_I,
	GUDAction_PickedUpMeatShield_R,

	GUDAction_KilledEnemyFlamethrower_I,
	GUDAction_KilledEnemyFlamethrower_TW,
	GUDAction_PickedUpFlamethrower_I,

	GUDAction_GatlingOverheated_I,

	GUDAction_MortarLaunched_EW,
	GUDAction_ThrowingInkGrenade_I,
	GUDAction_ThrowingInkGrenade_EW,
	GUDAction_StuckExplosiveToEnemy_I,
	GUDAction_StuckExplosiveToEnemy_R,
	GUDAction_GrenadeTrapSet_I,
	GUDAction_ExecutedMeatShield_I,

	GUDAction_DBNOPawnCrawlingAway_EW,

	// actions that aren't associated with an event, triggered manually
	GUDAction_NotifyHODEnabled,
	GUDAction_NotifyHODDisabled,
	GUDAction_NoticedACollectible,
	GUDAction_MeatFlagRoundBegun,
};

enum EGUDRole
{
	GUDRole_None,
	GUDRole_Instigator,			// the instigator CAUSED the event
	GUDRole_Recipient,			// the event HAPPENED TO the recipient 
	GUDRole_EnemyWitness,		// the EW witnessed the event and IS NOT on the instigators team
	GUDRole_TeammateWitness,	// the TW witnessed the event and IS on the instigators team
	GUDRole_ReferencedPawn,		// the event happened in reference to the ReferencedPawn (used mainly for responses)
};


defaultproperties
{
}

