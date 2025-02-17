/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "GearGame.h"
#include "EngineMaterialClasses.h"

#include "GearGameAnimClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameWeaponClasses.h"


// Base Anim Nodes
IMPLEMENT_CLASS(UGearAnim_AimOffset)

IMPLEMENT_CLASS(UGearAnim_BaseBlendNode)
IMPLEMENT_CLASS(UGearAnim_BlendBySpeed)
IMPLEMENT_CLASS(UGearAnim_BlendBySpeedDirection)
IMPLEMENT_CLASS(UGearAnim_BlendByTransition)
IMPLEMENT_CLASS(UGearAnim_BlendByCoverType)
IMPLEMENT_CLASS(UGearAnim_BlendByWeaponFire)
IMPLEMENT_CLASS(UGearAnim_BlendByWeaponType)
IMPLEMENT_CLASS(UGearAnim_BlendByWeaponClass)
IMPLEMENT_CLASS(UGearAnim_BlendByDamage)
IMPLEMENT_CLASS(UGearAnim_BlendByAngularVelocity)
IMPLEMENT_CLASS(UGearAnim_BlendByAngularVelocity3)
IMPLEMENT_CLASS(UGearAnim_BlendByHeavyWeaponMount)
IMPLEMENT_CLASS(UGearAnim_BlendByShieldExpand)
IMPLEMENT_CLASS(UGearAnim_BlendByTargetingMode)
IMPLEMENT_CLASS(UGearAnim_CoverBlend)
IMPLEMENT_CLASS(UGearAnim_CoverActionToggle)
IMPLEMENT_CLASS(UGearAnim_BlendByBlindUpStance)
IMPLEMENT_CLASS(UGearAnim_BlendList)
IMPLEMENT_CLASS(UGearAnim_BlendBySpecialMove)
IMPLEMENT_CLASS(UGearAnim_BlendByRefRelative)
IMPLEMENT_CLASS(UGearAnim_BlendByReaverLean)

IMPLEMENT_CLASS(UGearAnim_BlendByCoverDirection)
IMPLEMENT_CLASS(UGearAnim_Mirror_Master)
IMPLEMENT_CLASS(UGearAnim_Mirror_TransitionBlend)

IMPLEMENT_CLASS(UGearAnim_Corpser_BlendClaw)
IMPLEMENT_CLASS(UGearAnim_Wretch_BaseBlendNode)
IMPLEMENT_CLASS(UGearAnim_Wretch_JumpAttack)

IMPLEMENT_CLASS(UGearAnim_DirectionalMove2Idle)
IMPLEMENT_CLASS(UGearAnim_MovementNode)

IMPLEMENT_CLASS(UGearAnim_TurnInPlace)
IMPLEMENT_CLASS(UGearAnim_TurnInPlace_Rotator)
IMPLEMENT_CLASS(UGearAnim_TurnInPlace_Player)

// AnimNodeSequences
IMPLEMENT_CLASS(UGearAnim_ReverseByDirection)
IMPLEMENT_CLASS(UGearAnim_BlendAnimsByAim)
IMPLEMENT_CLASS(UGearAnim_BlendAnimsByDirection)

IMPLEMENT_CLASS(UGearAnim_UpperBodyIKHack)

IMPLEMENT_CLASS(UGearAnim_Slot)
IMPLEMENT_CLASS(UGearAnim_BlendPerBone)
IMPLEMENT_CLASS(UGearAnim_AdditiveBlending)

// Skorge
IMPLEMENT_CLASS(UGearAnim_Skorge_BlendByStage)
IMPLEMENT_CLASS(UGearAnim_Skorge_BlockingBullets)

// rockworm 
IMPLEMENT_CLASS(UGearAnim_RockwormTail_SynchToPrev)

// bone Controllers
IMPLEMENT_CLASS(UGearSkelCtrl_Recoil)
IMPLEMENT_CLASS(UGearSkelCtrl_TurretConstrained)
IMPLEMENT_CLASS(UGearSkelCtrl_Trail)
IMPLEMENT_CLASS(UGearSkelCtrl_CorpserIK)
IMPLEMENT_CLASS(UGearSkelCtrl_Prune)
IMPLEMENT_CLASS(UGearSkelCtrl_IKRecoil)
IMPLEMENT_CLASS(UGearSkelCtrl_FootPlanting)
IMPLEMENT_CLASS(UGearSkelCtrl_CCD_IK)
IMPLEMENT_CLASS(UGearSkelCtrl_Flamethrower)
IMPLEMENT_CLASS(UGearSkelCtrl_FlamethrowerScaling)
IMPLEMENT_CLASS(UGearSkelCtrl_Copy)
IMPLEMENT_CLASS(UGearSkelCtrl_Spring)

// Anim notifies
IMPLEMENT_CLASS(UGearAnimNotify_ToggleSkelControl)

// Morph Targets
IMPLEMENT_CLASS(UGearMorph_WeightByBoneAngle)

static FLOAT GetFloatAverage(FLOAT* History, INT NumElements)
{
	FLOAT Total = 0.f;
	for(INT i=0; i<NumElements; i++)
	{
		Total += History[i];
	}
	return Total/(FLOAT(NumElements));
}

/************************************************************************************
 * UGearAnim_BlendList
 ***********************************************************************************/


void UGearAnim_BlendList::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	SliderPosition	= NewSliderValue;

	const INT TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	if( ActiveChildIndex != TargetChannel )
	{
		FLOAT BlendInTime = 0.f;

		if( TargetChannel < ChildBlendInTime.Num() )
		{
			BlendInTime = ChildBlendInTime(TargetChannel);
		}

		SetActiveChild(TargetChannel, BlendInTime);
	}
}

void UGearAnim_BlendList::SetActiveChild(INT ChildIndex, FLOAT BlendTime)
{
	// Prevent switching when BlockSetActiveChildWhenRelevant is set and node is relevant.
	// Except if bJustBecameRelevant, that's valid!
	if( ChildIndex < BlockSetActiveChildWhenRelevant.Num() && BlockSetActiveChildWhenRelevant(ChildIndex) && 
		!bJustBecameRelevant && bRelevant && GIsGame )
	{
		return;
	}

	Super::SetActiveChild(ChildIndex, BlendTime);
}

/************************************************************************************
 * UGearAnim_BaseBlendNode
 ***********************************************************************************/


void UGearAnim_BaseBlendNode::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}


/**
 * Overridden to look at the GearPawn owner and activate the 
 * third child if evading, second child if using cover, and finally
 * the base child if neither.
 * 
 * @param		DeltaSeconds - time since last update
 */
void UGearAnim_BaseBlendNode::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT DesiredChildIdx = ActiveChildIndex;

	// Freeze when dead, so pawn retains his DBNO animations.
	if( GearPawnOwner && !GearPawnOwner->bPlayedDeath ) 
	{
		// Default channel
		DesiredChildIdx = 0;

		// Pawn is in cover.
		if( GearPawnOwner->CoverType != CT_None )
		{
			// If doing a run 2 cover special move, force into standard movement animations. Blend to cover animations once we've reached cover.
			// Otherwise just trigger cover animations
			if( (GearPawnOwner->SpecialMove != SM_Run2MidCov && GearPawnOwner->SpecialMove != SM_Run2StdCov) || 
				((GearPawnOwner->AcquiredCoverInfo.Location - GearPawnOwner->Location).Size2D() / (GearPawnOwner->GroundSpeed * GearPawnOwner->GetMaxSpeedModifier())) <= ChildBlendInTime(1) * 0.5f )
			{
				DesiredChildIdx = 1;
			}
			// Else default channel.
		}
		else 
		{
			// If delaying for a mirror transition, force Pawn to stay in cover until blend out animation is started.
			if( ActiveChildIndex == 1 && GearPawnOwner->SpecialMove != SM_None && GearPawnOwner->SpecialMoves(GearPawnOwner->SpecialMove) && 
				GearPawnOwner->SpecialMoves(GearPawnOwner->SpecialMove)->bMirrorTransitionSafeNotify )
			{
				DesiredChildIdx = 1;
			}
			// Force animations to remain in cover if doing a transition out of cover.
			// Once we're fully blended in, we can change that.
			else if( ActiveChildIndex == 1 && GearPawnOwner->bDoingMirrorTransition && GearPawnOwner->MirrorNode && !GearPawnOwner->MirrorNode->bToggledMirrorStatus )
			{
				DesiredChildIdx = 1;
			}
			else
			{
				// Set child based on special move
				switch( GearPawnOwner->SpecialMove )
				{
					case SM_Kidnapper_Execution		: 
					case SM_Kidnapper				: DesiredChildIdx = 2;	break;
					case SM_Hostage					: DesiredChildIdx = 3;	break;
					case SM_DBNO					:
					case SM_CQC_Victim				: DesiredChildIdx = 4;	break;
					case SM_ChainsawDuel_Leader		: DesiredChildIdx = 5;	break;
					case SM_ChainsawDuel_Follower	: DesiredChildIdx = 6;	break;
					case SM_DeployShield			: DesiredChildIdx = 7;	break;
				}
			}
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		DesiredChildIdx = appRound( SliderPosition*(Children.Num() - 1) );
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		SetActiveChild(DesiredChildIdx, DesiredChildIdx < ChildBlendInTime.Num() ? ChildBlendInTime(DesiredChildIdx) : 0.f);
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_BlendByWeaponFire
 ***********************************************************************************/

void UGearAnim_BlendByWeaponFire::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}

/**
 * Overridden to check the state of the weapon, and to activate
 * the second child if currently firing.
 * 
 * @param		DeltaSeconds - time since last update
 */
void UGearAnim_BlendByWeaponFire::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if( GearPawnOwner )
	{
		// This is how we can tell a weapon is firing on all clients.
		const UBOOL bIsWeaponFiring = GearPawnOwner->ShotCount > 0;

		if( bIsWeaponFiring )
		{
			// make sure the 2nd child is active
			if( Child2WeightTarget != MaxFireBlendWeight )
			{
				SetBlendTarget(MaxFireBlendWeight, BlendInTime);
			}
		}
		else
		{
			// not firing, make sure the 1st child is active
			if( Child2WeightTarget != 0.f )
			{
				SetBlendTarget(0.f, BlendOutTime);
			}
		}
	}

	// Update AnimNodeBlend
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
* UGearAnim_BlendByTargetingMode
***********************************************************************************/

void UGearAnim_BlendByTargetingMode::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}


/**
 * Check if Pawn is targeting and blend accordingly
 * 
 * @param		DeltaSeconds - time since last update
 */
void UGearAnim_BlendByTargetingMode::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT TargetChannel = ActiveChildIndex;

	if( GearPawnOwner )
	{
		// Make sure weapon is up to date.
		if( Weapon != GearPawnOwner->Weapon )
		{	
			Weapon = Cast<AGearWeapon>(GearPawnOwner->Weapon);
		}

		// Which animations the character is allowed to play.
		const UBOOL bAllowIdle			= !Weapon || Weapon->bAllowIdleStance;
		const UBOOL bAllowAiming		= Weapon && Weapon->bAllowAimingStance;
		const UBOOL bAllowDownsights	= Weapon && Weapon->bAllowDownsightsStance && (GearPawnOwner->EquippedShield == NULL);

		const UBOOL	bIsReloadingWeapon	= GearPawnOwner->IsReloadingWeapon();
		const UBOOL	bIsFiring = !bIsReloadingWeapon && GearPawnOwner->ShotCount > 0;

		// Downsights
		const UBOOL bIsTargeting	= bAllowDownsights && GearPawnOwner->bIsTargeting && !bIsReloadingWeapon;
		const UBOOL bForceTargeting	= bAllowDownsights && !bAllowAiming && !bAllowIdle;

		// Downsights channel
		if( !bMergeDownSightsIntoAim && (bIsTargeting || bForceTargeting) )
		{
			TargetChannel = (bIsFiring && !bMergeDownSightsFireIntoIdle) ? 4 : 3;
		}
		else
		{
			const FLOAT	CurrentTime	= GWorld->GetTimeSeconds();

			// Aiming
			const UBOOL	bIsAiming =	bAllowAiming && !bIsReloadingWeapon && 
									( CurrentTime < GearPawnOwner->ForcedAimTime || 
										(CurrentTime - GearPawnOwner->LastWeaponEndFireTime) < GearPawnOwner->AimTimeAfterFiring ||
										GearPawnOwner->FiringMode == UCONST_FIREMODE_CHARGE ||
										GearPawnOwner->bPendingFire);
			const UBOOL bForceAiming = bAllowAiming && !bAllowIdle;

			if( bIsAiming || bForceAiming || (bIsFiring && bMergeAimFiringIntoIdle) || (bIsTargeting && bMergeDownSightsIntoAim) )
			{
				TargetChannel = (bIsFiring && !bMergeAimFiringIntoIdle) ? 2 : (bMergeAimIdleIntoIdleReady ? 0 : 1);
			}
			else
			{
				TargetChannel = 0;
			}
		}
	}
	
	// Update channel
	if( TargetChannel != ActiveChildIndex )
	{
		// If blending from aim/target to idle, then use this special blend time
		const UBOOL BlendFromAimToIdle		= (TargetChannel == 0 && ActiveChildIndex > 0);
		const UBOOL	BlendFromTargetToAim	= (TargetChannel == 1 || TargetChannel == 2) && (ActiveChildIndex >= 3);
		if( BlendFromAimToIdle || BlendFromTargetToAim )
		{
			SetActiveChild(TargetChannel, Aim2IdleBlendOutTime);
		}
		else
		{
			const FLOAT BlendInTime = (TargetChannel < ChildBlendInTime.Num()) ? ChildBlendInTime(TargetChannel) : 0.f;
			SetActiveChild(TargetChannel, BlendInTime);
		}

// 		debugf(TEXT("Tick [%s] ActiveChildIndex: %i"), *GetFName().ToString(), ActiveChildIndex);
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);

	if( GearPawnOwner && bRelevant )
	{
		// We consider ourselves fully blended a little earlier
		const FLOAT FullyBlendedThreshold = 0.9f;
		const UBOOL bIsInIdleChannel = (Children(0).Weight > (1.f - FullyBlendedThreshold - ZERO_ANIMWEIGHT_THRESH));
		const UBOOL bNodeIsRelevant = (NodeTotalWeight > (1.f - ZERO_ANIMWEIGHT_THRESH));

// 		debugf(TEXT("Tick [%s] bIsInIdleChannel: %d, bNodeIsRelevant: %d"), *GetFName().ToString(), bIsInIdleChannel, bNodeIsRelevant);
		GearPawnOwner->UpdateTargetingNodeIsInIdleChannel(NodeTickTag, bIsInIdleChannel && bNodeIsRelevant);
	}
}

/** Get notification that this node is no longer relevant for the final blend. ie TotalWeight is now == 0 */
void UGearAnim_BlendByTargetingMode::OnCeaseRelevant()
{
	Super::OnCeaseRelevant();

	if( GearPawnOwner )
	{
// 		debugf(TEXT("OnCeaseRelevant [%s] FALSE"), *GetFName().ToString());
		GearPawnOwner->UpdateTargetingNodeIsInIdleChannel(NodeTickTag, FALSE);
	}
}

/************************************************************************************
* UGearAnim_BlendByCoverType
***********************************************************************************/


void UGearAnim_BlendByCoverType::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}


void UGearAnim_BlendByCoverType::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT DesiredChildIdx = ActiveChildIndex;

	if( GearPawnOwner && GearPawnOwner->CoverType != CT_None )
	{
		DesiredChildIdx = GearPawnOwner->CoverType - 1;
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// Support for AnimTree editor
		DesiredChildIdx = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		SetActiveChild(DesiredChildIdx, ChildBlendInTime(DesiredChildIdx));
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_Skorge_BlendByStage
 ***********************************************************************************/

void UGearAnim_Skorge_BlendByStage::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( SkorgeOwner != MeshComp->GetOwner() )
	{
		SkorgeOwner = Cast<AGearPawn_LocustSkorgeBase>(MeshComp->GetOwner());
	}
}

void UGearAnim_Skorge_BlendByStage::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT DesiredChildIdx = SkorgeOwner ? SkorgeOwner->Stage : ActiveChildIndex;

#if !CONSOLE
	if( GIsEditor && !GIsGame )
	{
		// Support for AnimTree editor
		DesiredChildIdx = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		SetActiveChild(DesiredChildIdx, ChildBlendInTime(DesiredChildIdx));
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/************************************************************************************
 * UGearAnim_Skorge_BlockingBullets
 ***********************************************************************************/

void UGearAnim_Skorge_BlockingBullets::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( SkorgeOwner != MeshComp->GetOwner() )
	{
		SkorgeOwner = Cast<AGearPawn_LocustSkorgeBase>(MeshComp->GetOwner());
	}
}

void UGearAnim_Skorge_BlockingBullets::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT DesiredChildIdx = ActiveChildIndex;

	if( SkorgeOwner )
	{
		DesiredChildIdx = SkorgeOwner->bBlockingBullets ? 1 : 0;
	}

#if !CONSOLE
	if( GIsEditor && !GIsGame )
	{
		// Support for AnimTree editor
		DesiredChildIdx = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		SetActiveChild(DesiredChildIdx, ChildBlendInTime(DesiredChildIdx));
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/************************************************************************************
 * UGearAnim_BlendBySpecialMove
 ***********************************************************************************/

void UGearAnim_BlendBySpecialMove::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}


void UGearAnim_BlendBySpecialMove::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT DesiredChildIdx = ActiveChildIndex;

	// Freeze when dead, so pawn retains his DBNO animations.
	if( GearPawnOwner && !GearPawnOwner->IsDoingDeathAnimSpecialMove() ) 
	{
		// Reset to default if Pawn is not dead
		if( !GearPawnOwner->bPlayedDeath )
		{
			DesiredChildIdx = 0;
		}

		// Set child based on special move
		switch( GearPawnOwner->SpecialMove )
		{
			case SM_Kidnapper_Execution		:
			case SM_Kidnapper				: DesiredChildIdx = 1;	break;
			case SM_Hostage					: DesiredChildIdx = 2;	break;
			case SM_DBNO					:
			case SM_CQC_Victim				: DesiredChildIdx = 3;	break;
			case SM_ChainsawDuel_Leader		: DesiredChildIdx = 4;	break;
			case SM_ChainsawDuel_Follower	: DesiredChildIdx = 5;	break;
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// Support for AnimTree editor
		DesiredChildIdx = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		SetActiveChild(DesiredChildIdx, DesiredChildIdx < ChildBlendInTime.Num() ? ChildBlendInTime(DesiredChildIdx) : 0.f);
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
* UGearAnim_BlendByCoverDirection
***********************************************************************************/

void UGearAnim_BlendByCoverDirection::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}


void UGearAnim_BlendByCoverDirection::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT	DesiredChildIdx	= 0;	// default to idle

	if( GearPawnOwner )
	{
		if( GearPawnOwner->CurrentSlotDirection == CD_Right )
		{
			DesiredChildIdx = 1;
		}
		else if( GearPawnOwner->CurrentSlotDirection == CD_Left )
		{
			DesiredChildIdx = 2;
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		DesiredChildIdx = appRound( SliderPosition*(Children.Num() - 1) );
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
			SetActiveChild(DesiredChildIdx, DesiredChildIdx < ChildBlendInTime.Num() ? ChildBlendInTime(DesiredChildIdx) : 0.f);
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_Mirror_Master
 ***********************************************************************************/

#if 0 && !FINAL_RELEASE
	#define DEBUGMIRRORING(x) { ##x }
#else
	#define DEBUGMIRRORING(x)
#endif

void UGearAnim_Mirror_Master::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}

	// Cache RootNode
	if( RootNode != MeshComp->Animations )
	{
		RootNode = Cast<UAnimTree>(MeshComp->Animations);
	}

	// Cache transition nodes
	// Find synchronization node
	// Cache blend nodes
	TransitionNodes.Reset();

	TArray<UAnimNode*> Nodes;
	MeshComp->Animations->GetNodes(Nodes);
	for (INT i = 0; i < Nodes.Num(); i++)
	{
		UGearAnim_Mirror_TransitionBlend* TransitionNode = Cast<UGearAnim_Mirror_TransitionBlend>(Nodes(i));
		if (TransitionNode != NULL)
		{
			TransitionNodes.AddItem(TransitionNode);
		}
	}

	// Cache skel controls
	Drive_SkelControls.Reset();
	for(INT i=0; i<Drive_SkelControlNames.Num(); i++)
	{
		USkelControlBase* SkelCtrl = MeshComp->FindSkelControl( Drive_SkelControlNames(i) );
		if( SkelCtrl )
		{
			Drive_SkelControls.AddUniqueItem(SkelCtrl);
		}
	}
}

static inline UGearAnim_Mirror_TransitionBlend* FindMTransNodeByName(TArray<UGearAnim_Mirror_TransitionBlend*>& MTransList, FName NodeName)
{
	for( INT i=0; i<MTransList.Num(); i++ )
	{
		//DEBUGMIRRORING( debugf(TEXT("%3.2f FindMTransNodeByName Checking: %s"), GWorld->GetTimeSeconds(), *MTransList(i)->NodeName.ToString()); )
		if( MTransList(i)->NodeName == NodeName )
		{
			return MTransList(i);
		}
	}

	return NULL;
}

/** 
 * Get most relevant TransitionBlend node 
 */
UGearAnim_Mirror_TransitionBlend* UGearAnim_Mirror_Master::GetMostRelevantTransitionNode()
{
	DEBUGMIRRORING(debugf(TEXT("%3.3f GetMostRelevantTransitionNode"), GWorld->GetTimeSeconds());)

	UGearAnim_Mirror_TransitionBlend* TransitionNode = NULL;

	// Gameplay version.
	// Allows for rules that are hard to express in the AnimTree.
	if( GearPawnOwner && GearPawnOwner->CoverType != CT_None )
	{
		if( GearPawnOwner->CoverType == CT_MidLevel )
		{
			if( GearPawnOwner->bDoing360Aiming && GearPawnOwner->SpecialMove != SM_CoverRun )
			{
				TransitionNode = FindMTransNodeByName(TransitionNodes, FName(TEXT("MTrans_MidCov_AimBack")));
			}
			else
			{
				TransitionNode = FindMTransNodeByName(TransitionNodes, FName(TEXT("MTrans_MidCov")));
			}
		}
		else
		{
			if( GearPawnOwner->bDoing360Aiming && GearPawnOwner->SpecialMove != SM_CoverRun )
			{
				TransitionNode = FindMTransNodeByName(TransitionNodes, FName(TEXT("MTrans_StdCov_AimBack")));
			}
			else
			{
				TransitionNode = FindMTransNodeByName(TransitionNodes, FName(TEXT("MTrans_StdCov")));
			}
		}
	}

	// If not found, try to look for most relevant weight wise.
	if( !TransitionNode )
	{
		FLOAT HightestWeight = 0.f;

		for (INT i=0; i<TransitionNodes.Num(); i++)
		{
			UGearAnim_Mirror_TransitionBlend* Candidate = TransitionNodes(i);
			if( Candidate && Candidate->bRelevant )
			{
				if( TransitionNode == NULL || 
					(Candidate->NodeTotalWeight > HightestWeight && !TransitionNode->IsChildOf(Candidate)) || 
					Candidate->IsChildOf(TransitionNode) )
				{

					DEBUGMIRRORING
					( 
						if( Candidate->SeqNodes.Num() > 0 && Candidate->SeqNodes(0) )
						{
							debugf(TEXT("%3.3f  Picked: %s"), GWorld->GetTimeSeconds(), *Candidate->SeqNodes(0)->AnimSeqName.ToString());
						}
						else
						{
							debugf(TEXT("%3.3f  Picked: NO NAME"), GWorld->GetTimeSeconds());
						}
					)

					TransitionNode	= Candidate;
					HightestWeight	= Candidate->NodeTotalWeight;
				}
			}
		}
	}

	DEBUGMIRRORING( debugf(TEXT("%3.3f  Picked %s"), GWorld->GetTimeSeconds(), TransitionNode ? *TransitionNode->NodeName.ToString() : TEXT("None")); )
	return TransitionNode;
}


/** Utility function to find out the most relevant body stance animation being played */
static inline UAnimNodeSequence* GetMostRelevantBodyStanceNode(TArray<UGearAnim_Slot*>& BodyStanceNodes)
{
	FLOAT				BestWeight	= 0.f;
	UAnimNodeSequence*	BestNode	= NULL;

	for(INT i=0; i<BodyStanceNodes.Num(); i++)
	{
		if( BodyStanceNodes(i) )
		{
			UAnimNodeSequence* Candidate = BodyStanceNodes(i)->GetCustomAnimNodeSeq();

			if( Candidate )
			{
				if( BestNode == NULL || Candidate->NodeTotalWeight > BestNode->NodeTotalWeight )
				{
					BestNode = Candidate;
				}
			}
		}
	}

	return BestNode;
}

/** Turn OFF bone controllers */
void UGearAnim_Mirror_Master::ForceDrivenNodesOff()
{
	for(INT i=0; i<Drive_SkelControls.Num(); i++)
	{
		Drive_SkelControls(i)->BlendOutTime = BlendInTime;
		Drive_SkelControls(i)->SetSkelControlActive(FALSE);
	}
}

/** Make sure that driven nodes are fully turned off */
UBOOL UGearAnim_Mirror_Master::AreDrivenNodesTurnedOff()
{
	// Then check for BoneControllers
	for(INT i=0; i<Drive_SkelControls.Num(); i++)
	{
		// Bone Controller is still relevant, so we're not ready!
		if( Drive_SkelControls(i)->ControlStrength > ZERO_ANIMWEIGHT_THRESH )
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


void UGearAnim_Mirror_Master::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Update child weight now.
	Super::TickAnim(DeltaSeconds, TotalWeight);

	const UBOOL bForceNonMirrorOutsideOfCover = GearPawnOwner && GearPawnOwner->CoverType == CT_None && !GearPawnOwner->bIsTargeting 
		&& (GearPawnOwner->SpecialMove == SM_None || !GearPawnOwner->SpecialMoves(GearPawnOwner->SpecialMove)->bCoverExitMirrorTransition);

	// If player is mirrored outside of cover, and not doing a cover exit mirror transition 
	// then force player to trigger a transition to the non mirrored state.
	if( bForceNonMirrorOutsideOfCover && GearPawnOwner && GearPawnOwner->bWantsToBeMirrored )
	{
		GearPawnOwner->eventSetMirroredSide(FALSE);
	}	

	// If not playing a transition, see if we should be playing one
	if( !bPlayingTransition )
	{
		// Figure out if we should be mirrored or not
		const UBOOL bWantsToBeMirrored = GearPawnOwner ? GearPawnOwner->bWantsToBeMirrored : bEditorMirrored;

		// See if we'd like to transition
		if( bIsMirrored != bWantsToBeMirrored )
		{
			DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] bWantsToBeMirrored: %d"), GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bWantsToBeMirrored); )

			// We'd like to transition, see if we can find a node to play this transition...
			UGearAnim_Mirror_TransitionBlend* TransNode	= GetMostRelevantTransitionNode();
			const UBOOL	bTransitionNodeRelevant = TransNode && TransNode->bRelevant;

			// Get animation node leading the transition
			UAnimNodeSequence* MasterNode = RootNode ? RootNode->GetGroupSynchMaster(GroupName) : NULL;
			UAnimNodeSequence* BodyStanceMaster = NULL;

			// If Body Stances are driving the mirror transition, then these override 'normal' transitions.
			// This is a hacky workaround for when a SpecialMove is started on top of an existing transition.
			// We do not want the first transition to abort the special move too early.
			if( BodyStanceNodes.Num() > 0 )
			{
				MasterNode = GetMostRelevantBodyStanceNode(BodyStanceNodes);
				BodyStanceMaster = MasterNode;
			}

			// If transition is being driven by a valid master node.
			const UBOOL	bMasterNodeRelevant = MasterNode && MasterNode->AnimSeq && MasterNode->bRelevant;

			// In Cover, wait for a transition to happen, outside of cover, Mirror right away.
			const UBOOL	bInstantMirror = bForceNonMirrorOutsideOfCover && !bWantsToBeMirrored;

			// Start the transition
			if( bInstantMirror || bMasterNodeRelevant || bTransitionNodeRelevant )
			{
				// Update new status
				bPendingIsMirrored		= bWantsToBeMirrored;
				bToggledMirrorStatus	= FALSE;
				bPlayingTransition		= TRUE;

				// Tell Pawn that we are transitioning
				if( GearPawnOwner )
				{
					GearPawnOwner->bDoingMirrorTransition = TRUE;
				}

				DEBUGMIRRORING(debugf(TEXT("%3.3f Mirror_Master [%s] bPlayingTransition. bTransitionNodeRelevant: %d, bMasterNodeRelevant: %d, bInstantMirror: %d"), 
					GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bTransitionNodeRelevant, bMasterNodeRelevant, bInstantMirror);)

				// If we're not doing this transition through a BodyStance animation, then start the normal AnimTree transitions
				if( !BodyStanceMaster && TransNode )
				{
					// Reset transition animation group to start position.
					// Do this only if animation is not visible.
					if( RootNode )
					{
						RootNode->ForceGroupRelativePosition(GroupName, 0.f);
					}

					// Start transition node.
					TransNode->StartTransition(BlendInTime);
				}

				// Call event telling that transition has started!
				TransitionStarted();
			}
			else
			{
				DEBUGMIRRORING(debugf(TEXT("%3.3f Mirror_Master [%s] Delay... bPlayingTransition. bTransitionNodeRelevant: %d, bMasterNodeRelevant: %d, bInstantMirror: %d"), 
					GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bTransitionNodeRelevant, bMasterNodeRelevant, bInstantMirror);)
			}
		}
	}
	// If playing a transition, track end of it
	else
	{
		// In Cover, wait for a transition to happen, Outside of cover, Mirror right away.
		const UBOOL	bInstantMirror = bForceNonMirrorOutsideOfCover && !bPendingIsMirrored;

		// Find the Master Node: Node leading the transition (because it's the most relevant to the final animation)
		UAnimNodeSequence* MasterNode = RootNode ? RootNode->GetGroupSynchMaster(GroupName) : NULL;

		// If Body Stances are driving the mirror transition, then these override 'normal' transitions.
		// This is a hacky workaround for when a SpecialMove is started on top of an existing transition.
		// We do not want the first transition to abort the special move too early.
		if( BodyStanceNodes.Num() > 0 )
		{
			MasterNode = GetMostRelevantBodyStanceNode(BodyStanceNodes);
		}

		// If transition is being driven by a valid master node.
		const UBOOL	bHasValidMaster = MasterNode && MasterNode->AnimSeq && MasterNode->bRelevant;

		// If we're delaying until we have a valid master node
		if( bDelayUntilMasterRelevant && !bInstantMirror )
		{
			// Wait until either we have a valid master node, or timeout has elapsed.
			if( bHasValidMaster )
			{
				bDelayUntilMasterRelevant = FALSE;
			}
			// Smaller time out outside of cover. In that case we're driven by SpecialMoves and BodyStances. We should never wait long here.
			else if( GearPawnOwner && GearPawnOwner->CoverType == CT_None && (GWorld->GetWorldInfo()->TimeSeconds - TransitionStartedTime) > 0.15f)
			{
				bDelayUntilMasterRelevant = FALSE;
			}
			// If we're in cover, we can sometimes get taken over by weapon reloads and get stuck. Longer time out here.
			else if( GearPawnOwner && GearPawnOwner->CoverType != CT_None && (GWorld->GetWorldInfo()->TimeSeconds - TransitionStartedTime) > 0.5f)
			{
				bDelayUntilMasterRelevant = FALSE;
			}
			else
			{
				DEBUGMIRRORING(debugf(TEXT("%3.3f Mirror_Master [%s] bDelayUntilMasterRelevant"), GWorld->GetTimeSeconds(), *GearPawnOwner->GetName());)
				// delay
				return;
			}
		}

		// If the Master Node the only node providing animation?
		// If that's the case, we can toggle mirroring and do some magic safely
		const UBOOL bMasterIsFullWeight	= bHasValidMaster && (MasterNode->NodeTotalWeight >= (1.f - ZERO_ANIMWEIGHT_THRESH));

		// Get remaining play time of transition played by master node.
		// If no transition being played, then not ready to blend out.
		const FLOAT RemainingPlayTime	= bHasValidMaster && MasterNode->bPlaying ? MasterNode->GetTimeLeft() : 0.f;

		// Is the transition animation finished?
		const UBOOL	bFinishedTransition	= !bHasValidMaster || bInstantMirror || (RemainingPlayTime < KINDA_SMALL_NUMBER) || !MasterNode->bPlaying;

		// We haven't toggled mirroring, see if we can do it now
		// We try to do this as soon as possible. So if transition gets aborted for whatever reason, mirroring will be done 
		// and weapon sockets exchanged.
		if( !bToggledMirrorStatus ) 
		{
			// Wait for driven nodes to be completely off, otherwise mirror toggle may create a pop.
			if( (bInstantMirror || AreDrivenNodesTurnedOff()) && (bMasterIsFullWeight || bFinishedTransition) )
			{
				// mirror trick now!
				ToggleMirrorStatus();

				DEBUGMIRRORING
				(
					debugf(TEXT("%3.3f Mirror_Master [%s] bToggledMirrorStatus. bIsMirrored: %d, bMasterIsFullWeight: %d, bFinishedTransition: %d, bInstantMirror: %d"), 
						GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bIsMirrored, bMasterIsFullWeight, bFinishedTransition, bInstantMirror);
					
					if( !bHasValidMaster || !bMasterIsFullWeight )
					{
						debugf(TEXT("%3.3f Mirror_Master [%s] Ugly Mirroring Hapening :( bMasterIsFullWeight: %d. bHasValidMaster: %d"), 
							GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bMasterIsFullWeight, bHasValidMaster);
					}
				)
			}
			else
			{
				DEBUGMIRRORING
				( 
					debugf(TEXT("%3.3f Mirror_Master [%s] Delay for bToggledMirrorStatus. bInstantMirror: %d, AreDrivenNodesTurnedOff: %d, bMasterIsFullWeight: %d, bFinishedTransition: %d"), 
						GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bInstantMirror, AreDrivenNodesTurnedOff(), bMasterIsFullWeight, bFinishedTransition);
					if( bHasValidMaster )
					{
						debugf(TEXT("    Master AnimName: %s, RemainingPlayTime: %3.3f"), *MasterNode->AnimSeqName.ToString(), RemainingPlayTime); 
					}
				)
			}
		}

		if( bToggledMirrorStatus ) 
		{
			// Are we ready to blend out from the transition?
			// Either we can blend out early (if toggle has been made), or we've reached the end of the animation.
			// If we reach the end and toggle hasn't been made, then something went wrong.
			const UBOOL	bReadyToBlendOut = bHasValidMaster && (RemainingPlayTime <= BlendOutTime);

			// If we're ready to blend out, do it
			if( !bLockBlendOut && !bBlendingOut && (bReadyToBlendOut || bFinishedTransition) )
			{
				BlendOutTimeToGo = !bHasValidMaster  ? BlendOutTime :  RemainingPlayTime;
				DEBUGMIRRORING(debugf(TEXT("%3.3f Mirror_Master [%s] BlendingOut. bReadyToBlendOut: %d, bInstantMirror: %d, bHasValidMaster: %d, BlendOutTimeToGo: %f"), 
					GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bReadyToBlendOut, bInstantMirror, bHasValidMaster, BlendOutTimeToGo);)
				// Event called when blending out starts
				TransitionBlendingOut(BlendOutTimeToGo);
			}

			// If transition is finished, then we're ready to play another one.
			if( !bLockBlendOut && bFinishedTransition )
			{
				DEBUGMIRRORING(debugf(TEXT("%3.3f Mirror_Master [%s] Transition Finished. bFinishedTransition: %d, bInstantMirror: %d, bHasValidMaster: %d. Turn ON BoneControllers: %f"), 
					GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bFinishedTransition, bInstantMirror, bHasValidMaster, BlendOutTime);)
				TransitionFinished();
			}
		}
	}
}

/** Called when mirror transition is starting */
void UGearAnim_Mirror_Master::TransitionStarted()
{
	DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] TransitionStarted. Turn SkelControls OFF %f"), 
		GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), BlendInTime); )

	// Reset this flag when a new transition is started.
	bBlendingOut = FALSE;
	// Animation may take a little while to become relevant, so delay mirroring until then.
	bDelayUntilMasterRelevant = TRUE;
	// Save time when we started this transition.
	TransitionStartedTime = GWorld->GetWorldInfo()->TimeSeconds;

	// Turn OFF bone controllers
	ForceDrivenNodesOff();
}

/** Called when we're ready to toggle the mirror flag. */
void UGearAnim_Mirror_Master::ToggleMirrorStatus()
{
	DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] ToggleMirrorStatus"), 
		GWorld->GetTimeSeconds(), *GearPawnOwner->GetName()); )

	bToggledMirrorStatus = TRUE;

	// Counter mirror transition animation.
	// So it blends a non mirrored transition animation to a mirrored source animation.
	// Start blending out of transition
	for (INT i=0; i<TransitionNodes.Num(); i++)
	{
		TransitionNodes(i)->Children(1).bMirrorSkeleton = TRUE;
	}

	// Body stances doing transitions... Toggle mirroring flag...
	for(INT i=0; i<BodyStanceNodes.Num(); i++)
	{
		const INT ChildIndex = BodyStanceNodes(i)->CustomChildIndex;
		BodyStanceNodes(i)->Children(ChildIndex).bMirrorSkeleton = !BodyStanceNodes(i)->Children(ChildIndex).bMirrorSkeleton;
	}

	// Toggle node's mirroring flag
	Children(0).bMirrorSkeleton = bPendingIsMirrored;

	// Set node's status
	bIsMirrored	= bPendingIsMirrored;

	// Notify Pawn that mirroring has been toggled
	if( GearPawnOwner )
	{
		if( GearPawnOwner->bIsMirrored != bPendingIsMirrored )
		{
			GearPawnOwner->bIsMirrored = bPendingIsMirrored;
			GearPawnOwner->eventOnMirroredNotify();
		}
	}
}

/** Called when mirror transition is starting to blend out */
void UGearAnim_Mirror_Master::TransitionBlendingOut(FLOAT BlendOutTimeToGo)
{
	DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] TransitionBlendingOut. BlendOutTime: %f"), 
		GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), BlendOutTime); )

	bBlendingOut = TRUE;

	// Start blending out transition nodes
	for (INT i=0; i<TransitionNodes.Num(); i++)
	{
		TransitionNodes(i)->SetBlendTarget(0.f, BlendOutTimeToGo);
	}
}

/** Called when transition is finished */
void UGearAnim_Mirror_Master::TransitionFinished()
{
	DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] TransitionFinished"), 
		GWorld->GetTimeSeconds(), *GearPawnOwner->GetName()); )

	bPlayingTransition	= FALSE;
	bBlendingOut		= FALSE;

	// Turn ON bone controllers
	for(INT i=0; i<Drive_SkelControls.Num(); i++)
	{
		Drive_SkelControls(i)->BlendInTime = BlendOutTime;
		Drive_SkelControls(i)->SetSkelControlActive(TRUE);
	}

	// Tell Pawn that transition is finished
	if( GearPawnOwner )
	{
		GearPawnOwner->bDoingMirrorTransition = FALSE;
		
		// Needs to happen after skel controls are turned back on, in case gameplay code, wants to force some of them off.
		GearPawnOwner->eventOnMirrorBlendOutNotify();

		// Force an update of 360 aiming in cover.
		// If mirror transition was done to enter 360 aiming in cover, then update it now.
		// This way GearPC.UpdatePlayerPosture() knows about it.
		// Otherwise it can create conflicts and trigger several mirroring transitions in a row.
		if( GearPawnOwner->bCanDo360AimingInCover )
		{
			GearPawnOwner->UpdateAimOffset(GearPawnOwner->AimOffsetPct, 0.f);
		}
	}
}

/** Special case mirroring. When a BodyStance animation does a mirror transition. */
void UGearAnim_Mirror_Master::MirrorBodyStanceNode(class UGearAnim_Slot* SlotNode, UBOOL bBeginTransition, UBOOL bMirrorAnimation)
{
	if( !SlotNode )
	{
		return;
	}

	DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] MirrorBodyStanceNode, bBeginTransition: %d, bMirrorAnimation: %d"), 
		GWorld->GetTimeSeconds(), *GearPawnOwner->GetName(), bBeginTransition, bMirrorAnimation); )

	// Starting mirror transition
	if( bBeginTransition )
	{
		// Set mirroring on children
		SlotNode->Children(SlotNode->CustomChildIndex).bMirrorSkeleton = bMirrorAnimation;

		// If mirroring, add body stance node to our cache
		BodyStanceNodes.AddUniqueItem(SlotNode);

		if( bPlayingTransition )
		{
			DEBUGMIRRORING( debugf(TEXT("%3.3f Mirror_Master [%s] MirrorBodyStanceNode, Taking over from an previous transition."), 
				GWorld->GetTimeSeconds(), *GearPawnOwner->GetName()); )

			// Call event telling that transition has started!
			// This will make sure SkelControllers are shut down.
			TransitionStarted();

			// Figure out if we should be mirrored or not
			const UBOOL bWantsToBeMirrored = GearPawnOwner ? GearPawnOwner->bWantsToBeMirrored : bEditorMirrored;

			// If we're in the middle of a different transition, abort it.
			if( bPendingIsMirrored != bWantsToBeMirrored )
			{
				bPendingIsMirrored		= bWantsToBeMirrored;
				bToggledMirrorStatus	= FALSE;
			}
		}
	}
	else
	{
		// If done mirroring, clear up references
		BodyStanceNodes.RemoveItem(SlotNode);

		// If we were playing a transition, and we just lost our animation node, see if we should abort the transition.
		if( bPlayingTransition )
		{
			if( !bToggledMirrorStatus ) 
			{
				ToggleMirrorStatus();
			}
			if( !bLockBlendOut && !bBlendingOut )
			{
				// Event called when blending out starts
				TransitionBlendingOut(0.f);
			}
			if( !bLockBlendOut )
			{
				TransitionFinished();
			}
		}
	}
}


/************************************************************************************
 * UGearAnim_Mirror_TransitionBlend
 ***********************************************************************************/

void UGearAnim_Mirror_TransitionBlend::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache transition node to avoid expensive casting every tick.
	SeqNodes.Empty();
	if( Children(1).Anim )
	{
		Children(1).Anim->GetAnimSeqNodes(SeqNodes);
	}
}

void UGearAnim_Mirror_TransitionBlend::StartTransition(FLOAT BlendInTime)
{
	// Play transition non mirrored.
	Children(1).bMirrorSkeleton = FALSE;

	for(INT i=0; i<SeqNodes.Num(); i++)
	{
		// Start transition animation
		SeqNodes(i)->PlayAnim(FALSE, SeqNodes(i)->Rate, 0.f);
	}

	// Blend to transition child
	SetBlendTarget(1.f, BlendInTime);
}


/************************************************************************************
 * UGearAnim_CoverBlend
 ***********************************************************************************/

#if 0 && !FINAL_RELEASE
	#define DEBUGCOVERBLEND(x) { ##x }
#else
	#define DEBUGCOVERBLEND(x)
#endif

void UGearAnim_CoverBlend::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}

/** Get notification that this node is no longer relevant for the final blend. ie TotalWeight is now == 0 */
void UGearAnim_CoverBlend::OnCeaseRelevant()
{
	Super::OnCeaseRelevant();

	// If node leaves relevancy, make sure we reset the bDoingCoverActionAnimTransition flag
	if( bUpdatePawnActionFiringFlag && GearPawnOwner )
	{
		GearPawnOwner->UpdateCoverActionAnimTransition(NodeTickTag, FALSE);
	}

	// If Pawn is not in cover, then reset cover anim action
	if( bUpdatePawnActionFiringFlag && GearPawnOwner && GearPawnOwner->CoverType == CT_None )
	{
		GearPawnOwner->CoverActionAnim		= CA_Default;
		GearPawnOwner->LastCoverActionAnim	= CA_Default;
		AnimCoverAction						= CA_Default;
	}	
}

void UGearAnim_CoverBlend::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT	DesiredChildIdx		= ActiveChildIndex;
	BYTE PickedCoverAction	= AnimCoverAction;

	if( GearPawnOwner && GearPawnOwner->CoverType != CT_None )
	{
		// Default
		PickedCoverAction = CA_Default;

		// Update weapon information if we care about that
		if( bLockForHeavyWeapons && CachedWeapon != GearPawnOwner->Weapon )
		{
			CachedWeapon = GearPawnOwner->Weapon;
			bCarryingHeavyWeapon = CachedWeapon && CachedWeapon->IsA(AGearWeap_HeavyBase::StaticClass());
		}

		// If Pawn can do 360 aiming, then we have to check for that...
		// Use Idle Stance when reloading or switching weapons
		if( GearPawnOwner->bDoing360Aiming && !GearPawnOwner->IsReloadingWeapon() && !GearPawnOwner->bSwitchingWeapons 
			&& GearPawnOwner->FiringMode != UCONST_SHOTGUN_COCK_FIREMODE )
		{
			DesiredChildIdx = 1;
		}
		// No action or cover run, force into the idle channel.
		else if( GearPawnOwner->CoverAction == CA_Default || GearPawnOwner->SpecialMove == SM_CoverRun || 
			(bLockForHeavyWeapons && bCarryingHeavyWeapon) ||
			// If AI is reloading, force into idle channel until they are done
			(!GearPawnOwner->IsHumanControlled() && GearPawnOwner->IsReloadingWeapon()) )
		{
			DesiredChildIdx = 0;
		}
		else
		{
			// Update channel based on cover action. 
			// That doesn't take into account 360 aiming, it's done below.
			switch( GearPawnOwner->CoverAction )
			{
				case CA_BlindRight	:	
				case CA_BlindLeft	: DesiredChildIdx = 2;	PickedCoverAction = GearPawnOwner->CoverAction;	break;

				case CA_BlindUp		: DesiredChildIdx = 3;	PickedCoverAction = GearPawnOwner->CoverAction;	break;

				case CA_PopUp		: DesiredChildIdx = 4;	PickedCoverAction = GearPawnOwner->CoverAction;	break;

				case CA_LeanRight	:	
				case CA_LeanLeft	: DesiredChildIdx = 5;	PickedCoverAction = GearPawnOwner->CoverAction;	break;

				case CA_PeekRight	:
				case CA_PeekLeft	: DesiredChildIdx = 7;	PickedCoverAction = GearPawnOwner->CoverAction;	break;

				case CA_PeekUp		:	// Peek up can only be called from mid level cover!
										if( GearPawnOwner->CoverType == CT_MidLevel )
										{
											DesiredChildIdx = 8;
											PickedCoverAction = GearPawnOwner->CoverAction;
										}
										break;

				default				: DesiredChildIdx = 0;	break;
			}
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// Support for AnimTree editor
		DesiredChildIdx = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx || AnimCoverAction != PickedCoverAction )
	{
		UBOOL bCanTransition = TRUE;

		// check if we're allowed to blend out of current node
		if( !bJustBecameRelevant && Children(ActiveChildIndex).Anim )
		{
			// transition node is requesting a delay until we can blend out of active node
			bCanTransition = Children(ActiveChildIndex).Anim->CanBlendOutFrom();
			DEBUGCOVERBLEND( debugf(TEXT(" UGearAnim_CoverBlend CanBlendOutFrom %d, Active:%d, Desired:%d"), bCanTransition, ActiveChildIndex, DesiredChildIdx); )
		}

		// Check if we're allowed to blend to desired node
		if( bCanTransition && Children(DesiredChildIdx).Anim )
		{
			bCanTransition = Children(DesiredChildIdx).Anim->CanBlendTo();
			DEBUGCOVERBLEND( debugf(TEXT("UGearAnim_CoverBlend CanBlendTo %d, Active:%d, Desired:%d"), bCanTransition, ActiveChildIndex, DesiredChildIdx); )
		}

		// if we can transition, then do it
		if( bCanTransition )
		{
			// activate new child
			AnimCoverAction = PickedCoverAction;
			FLOAT BlendTime = DesiredChildIdx < ChildBlendInTime.Num() ? ChildBlendInTime(DesiredChildIdx) : 0.f;
			// Increase blendtime for 360 <-> leaning transitions because we're skipping transitions there.
			if( GearPawnOwner && (GearPawnOwner->IsDoing360ToLeaningTransition() || GearPawnOwner->IsDoingLeaningTo360Transition()) )
			{
				BlendTime *= 1.5f;
			}
			SetActiveChild(DesiredChildIdx, BlendTime);

			DEBUGCOVERBLEND( debugf(TEXT("%3.2f UGearAnim_CoverBlend SetActiveChild %d %3.2f"), GWorld->GetTimeSeconds(), DesiredChildIdx, BlendInTime); )
		}
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);

	// Update bDoingCoverActionAnimTransition flag
	if( bUpdatePawnActionFiringFlag && GearPawnOwner )
	{
		if( TotalWeight >= (1.f - ZERO_ANIMWEIGHT_THRESH) )
		{
			// Update LastCoverActionAnim with what the animations are playing.
			if( GearPawnOwner->CoverActionAnim != AnimCoverAction )
			{
				GearPawnOwner->LastCoverActionAnim	= GearPawnOwner->CoverActionAnim;
				GearPawnOwner->CoverActionAnim		= AnimCoverAction;
			}

			// Set transition status flag, This is to find out if we are blending between states
			const UBOOL bDoingTransition = (Children(ActiveChildIndex).Weight < (1.f - ZERO_ANIMWEIGHT_THRESH));
			const UBOOL bInIdleChannel = (Children(0).Weight > ZERO_ANIMWEIGHT_THRESH);

			// Update transition flag.
			GearPawnOwner->UpdateCoverActionAnimTransition(NodeTickTag, (bDoingTransition || bInIdleChannel));

#if 0 // DEBUG
			debugf(TEXT("%3.2f bDoingCoverActionAnimTransition: %d, %s"), GWorld->GetTimeSeconds(), GearPawnOwner->bDoingCoverActionAnimTransition, *GetName());
			
			if( bActiveChildIsFiringFriendly )
			{
				for(INT i=0; i<Children.Num(); i++)
				{
					if( Children(i).Weight > ZERO_ANIMWEIGHT_THRESH )
					{
						if( i == ActiveChildIndex )
						{
							debugf(TEXT("* %d Weight: %2.3f"), i, Children(i).Weight);
						}
						else
						{
							debugf(TEXT("  %d Weight: %2.3f"), i, Children(i).Weight);
						}
					}
				}
			}
#endif
		}
	}
	
}

/************************************************************************************
 * UGearAnim_CoverActionToggle
 ***********************************************************************************/

void UGearAnim_CoverActionToggle::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}

void UGearAnim_CoverActionToggle::OnBecomeRelevant()
{
	Super::OnBecomeRelevant();

	if( GearPawnOwner )
	{
		CachedPrevCoverAction = GearPawnOwner->PreviousCoverAction;
		CachedCoverAction = GearPawnOwner->CoverAction;
	}
}

void UGearAnim_CoverActionToggle::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	UBOOL bToggleOn = FALSE;
	if( GearPawnOwner )
	{
		if( PrevCoverAction.Num() > 0 )
		{
			for(INT i=0; i<PrevCoverAction.Num(); i++ )
			{
				if( PrevCoverAction(i) == CachedPrevCoverAction )
				{
					bToggleOn = TRUE;
					break;
				}
			}
		}

		if( !bToggleOn && CoverAction.Num() > 0 )
		{
			for(INT i=0; i<CoverAction.Num(); i++ )
			{
				if( CoverAction(i) == CachedCoverAction )
				{
					bToggleOn = TRUE;
					break;
				}
			}
		}
	}

	const INT DesiredChildIndex = bToggleOn ? 1 : 0;
	if( DesiredChildIndex != ActiveChildIndex )
	{
		UBOOL bCanTransition = TRUE;

		// check if we're allowed to blend out of current node
		if( !bJustBecameRelevant && Children(ActiveChildIndex).Anim )
		{
			// transition node is requesting a delay until we can blend out of active node
			bCanTransition = Children(ActiveChildIndex).Anim->CanBlendOutFrom();
#if 0
			debugf(TEXT(" UGearAnim_CoverActionToggle CanBlendOutFrom %d, Active:%d, Desired:%d"), bCanTransition, ActiveChildIndex, DesiredChildIndex);
#endif
		}

		// Check if we're allowed to blend to desired node
		if( bCanTransition && Children(DesiredChildIndex).Anim )
		{
			bCanTransition = Children(DesiredChildIndex).Anim->CanBlendTo();
#if 0
			debugf(TEXT("UGearAnim_CoverActionToggle CanBlendTo %d, Active:%d, Desired:%d"), bCanTransition, ActiveChildIndex, DesiredChildIndex);
#endif
		}

		// if we can transition, then do it
		if( bCanTransition )
		{
			// Use custom blend in time.
			const FLOAT BlendInTime = (DesiredChildIndex < ChildBlendInTime.Num()) ? ChildBlendInTime(DesiredChildIndex) : 0.f;

			// activate new child
			SetActiveChild(DesiredChildIndex, BlendInTime);

#if 0 // DEBUG
			debugf(TEXT("%3.2f UGearAnim_CoverActionToggle SetActiveChild %d %3.2f"), GWorld->GetTimeSeconds(), DesiredChildIndex, BlendInTime);
#endif
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/** Parent node is requesting a blend out. Give node a chance to delay that. */
UBOOL UGearAnim_CoverActionToggle::CanBlendOutFrom()
{
	// See if any of our relevant children is requesting a delay.
	if( bRelevant )
	{
		for(INT i=0; i<Children.Num(); i++)
		{
			if( Children(i).Anim && Children(i).Anim->bRelevant && !Children(i).Anim->CanBlendOutFrom() )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


/** parent node is requesting a blend in. Give node a chance to delay that. */
UBOOL UGearAnim_CoverActionToggle::CanBlendTo()
{
	// See if any of our relevant children is requesting a delay.
	if( bRelevant )
	{
		for(INT i=0; i<Children.Num(); i++)
		{
			if( Children(i).Anim && Children(i).Anim->bRelevant && !Children(i).Anim->CanBlendTo() )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

/************************************************************************************
 * UGearAnim_BlendByBlindUpStance
 ***********************************************************************************/

void UGearAnim_BlendByBlindUpStance::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}

void UGearAnim_BlendByBlindUpStance::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Only relevant if PawnOwner is blind firing up.
	if( GearPawnOwner && GearPawnOwner->CoverType != CT_None && 
		(GearPawnOwner->CoverAction == CA_BlindUp || GearPawnOwner->CoverAction == CA_BlindRight || GearPawnOwner->CoverAction == CA_BlindLeft) )
	{
		// Update Weapon reference if necessary
		if( GearPawnOwner->Weapon != Weapon )
		{
			Weapon = Cast<AGearWeapon>(GearPawnOwner->Weapon);
		}

		// If weapon doesn't want to play default blind up stance, then play default idle animation.
		if( !Weapon || !Weapon->bPlayDefaultBlindFireStance )
		{
			if( ActiveChildIndex != 1 )
			{
				SetActiveChild(1, 0.f);
			}
		}
		// Otherwise re route to default blind up stance
		else
		{
			if( ActiveChildIndex != 0 )
			{
				SetActiveChild(0, 0.f);
			}
		}
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * GearAnim_BlendByTransition
 ***********************************************************************************/

#if 0 && !FINAL_RELEASE
	#define DEBUGBLENDBYTRANSITION(x) { ##x }
#else
	#define DEBUGBLENDBYTRANSITION(x)
#endif

void UGearAnim_BlendByTransition::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}


/** Node has become relevant to the final blend, play intro. */
void UGearAnim_BlendByTransition::OnBecomeRelevant()
{
	Super::OnBecomeRelevant();

	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::OnBecomeRelevant() %f"), GWorld->GetTimeSeconds()); )

	// Reset flags
	bPlayingIntro	= FALSE;
	bPlayingOutro	= FALSE;
	bPlayedOutro	= FALSE;

	// See if we should bypass intro
	UBOOL bBypassIntro = FALSE;
	if( GearPawnOwner )
	{
		if( PrevCoverActionBypassIntro.Num() > 0 )
		{
			for(INT i=0; i<PrevCoverActionBypassIntro.Num(); i++ )
			{
				if( PrevCoverActionBypassIntro(i) == GearPawnOwner->PreviousCoverAction )
				{
					bBypassIntro = TRUE;
					break;
				}
			}
		}

		if( !bBypassIntro && b360AimingBypassIntro && GearPawnOwner->bWasDoing360Aiming )
		{
			bBypassIntro = TRUE;
		}
	}

	// if cannot play intro, fall back to playing main channel
	if( bBypassIntro || !Children(0).Anim || !PlayIntro() )
	{
		PlayMain();
	}
}


/** Node has ceased to be relevant to the final blend */
void UGearAnim_BlendByTransition::OnCeaseRelevant()
{
	Super::OnCeaseRelevant();

	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::OnCeaseRelevant() %f"), GWorld->GetTimeSeconds()); )

	// Reset flags
	bPlayingIntro	= FALSE;
	bPlayingOutro	= FALSE;
	bPlayedOutro	= FALSE;

	// stop all intro anim nodes
	StopIntro();
	
	// stop all outro anim nodes
	StopOutro();

	if( bUpdatePawnActionFiringFlag && GearPawnOwner )
	{
		GearPawnOwner->UpdateCoverActionAnimTransition(NodeTickTag, FALSE);
	}
}


/** 
 * Start up intro animation 
 * Returns TRUE if it was able to start up at least one animation node.
 */	
UBOOL UGearAnim_BlendByTransition::PlayIntro()
{
	bPlayingIntro	= FALSE;
	bPlayingOutro	= FALSE;
	bPlayedOutro	= FALSE;

	DEBUGBLENDBYTRANSITION( debugf(TEXT("%3.2f UGearAnim_BlendByTransition::PlayIntro()"), GWorld->GetTimeSeconds()); )

	if( Children(0).Anim )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(0).Anim->GetAnimSeqNodes(SeqNodes);

		// If controlled by a Human Player, get Pawn
		APawn*	P = GearPawnOwner ? GearPawnOwner->GetPlayerPawn() : NULL;

		for(INT i=0; i<SeqNodes.Num(); i++)
		{
			// For human players, scale transitions by 1.5x
			const FLOAT PlayRate = P ? 1.2f : 1.f;

			SeqNodes(i)->PlayAnim(FALSE, PlayRate, 0.f);
			bPlayingIntro = TRUE;
		}

		if( bPlayingIntro )
		{
			// Make sure intro node is relevant.
			SetActiveChild(0, ChildBlendInTime(0) * (1.f - Children(0).Weight));
			return TRUE;
		}
	}

	return FALSE;
}


/** Play main animation */
void UGearAnim_BlendByTransition::PlayMain()
{
	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::PlayMain() %f"), GWorld->GetTimeSeconds()); )
	bPlayingIntro	= FALSE;
	bPlayingOutro	= FALSE;
	bPlayedOutro	= FALSE;

	// transition to main animation
	SetActiveChild(1, ChildBlendInTime(1) * (1.f - Children(1).Weight));
}


/** Play Outro animation */
UBOOL UGearAnim_BlendByTransition::PlayOutro()
{
	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::PlayOutro() %f"), GWorld->GetTimeSeconds()); )
	bPlayingIntro	= FALSE;
	bPlayingOutro	= FALSE;
	bPlayedOutro	= FALSE;

	if( Children(2).Anim )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(2).Anim->GetAnimSeqNodes(SeqNodes);

		for(INT i=0; i<SeqNodes.Num(); i++)
		{
			SeqNodes(i)->PlayAnim(FALSE, SeqNodes(i)->Rate, 0.0f);
			bPlayingOutro = TRUE;
		}

		if( bPlayingOutro )
		{
			// Make sure outro node is relevant.
			SetActiveChild(2, ChildBlendInTime(2) * (1.f - Children(2).Weight));
			return TRUE;
		}
	}

	return FALSE;
}


/** Stop all intro anim nodes */
void UGearAnim_BlendByTransition::StopIntro()
{
	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::StopIntro() %f"), GWorld->GetTimeSeconds()); )
	if( Children(0).Anim )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(0).Anim->GetAnimSeqNodes(SeqNodes);

		for(INT i=0; i<SeqNodes.Num(); i++)
		{
			SeqNodes(i)->StopAnim();
		}
	}
}


/** stop all outro anim nodes */
void UGearAnim_BlendByTransition::StopOutro()
{
	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::StopOutro() %f"), GWorld->GetTimeSeconds()); )
	if( Children(2).Anim )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(2).Anim->GetAnimSeqNodes(SeqNodes);

		for(INT i=0; i<SeqNodes.Num(); i++)
		{
			SeqNodes(i)->StopAnim();
		}
	}
}


/** Returns TRUE if Child is an intro node */
UBOOL UGearAnim_BlendByTransition::IsAnIntroNode(UAnimNodeSequence* Child)
{
	if( Children(0).Anim )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(0).Anim->GetAnimSeqNodes(SeqNodes);

		for(INT i=0; i<SeqNodes.Num(); i++)
		{
			if( Child == SeqNodes(i) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


/** Returns TRUE if Child is an outro node */
UBOOL UGearAnim_BlendByTransition::IsAnOutroNode(UAnimNodeSequence* Child)
{
	if( Children(2).Anim )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(2).Anim->GetAnimSeqNodes(SeqNodes);

		for(INT i=0; i<SeqNodes.Num(); i++)
		{
			if( Child == SeqNodes(i) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


/** Check if intro or outro finished playing */
void UGearAnim_BlendByTransition::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child, PlayedTime, ExcessTime);

	DEBUGBLENDBYTRANSITION( debugf(TEXT("UGearAnim_BlendByTransition::OnChildAnimEnd() %f"), GWorld->GetTimeSeconds()); )
	if( bRelevant && Child->bRelevant )
	{
		// Intro finished playing
		if( bPlayingIntro && IsAnIntroNode(Child) )
		{
			// stop intro nodes from propagating more anim end notifications
			StopIntro();
			PlayMain();
		}
		else if( bPlayingOutro && IsAnOutroNode(Child) )
		{
			// stop outro nodes from propagating more anim end notifications
			StopOutro();
			bPlayingOutro	= FALSE;
			bPlayedOutro	= TRUE;
		}
	}
}


/** parent node is requesting a blend in */
UBOOL UGearAnim_BlendByTransition::CanBlendTo()
{
	DEBUGBLENDBYTRANSITION(
		debugf(TEXT("UGearAnim_BlendByTransition::CanBlendTo() bPlayingIntro:%d, bPlayingOutro:%d, bPlayedOutro:%d"), bPlayingIntro, bPlayingOutro, bPlayedOutro);
		debugf(TEXT("    NodeTotalWeight:%f, Children(0).Weight:%f, Children(1).Weight:%f, Children(2).Weight:%f"), NodeTotalWeight, Children(0).Weight, Children(1).Weight, Children(2).Weight);
		)

	// Cannot blend to this if it is already relevant
	// this forces the event OnBecomeRelevant() to be called again to start the intro
	if( !bPlayedOutro && bRelevant )
	{
		return FALSE;
	}

	return TRUE;
}


/** 
 * Parent node is requesting a blend out.
 * This is our chance to delay this until we have finished our transitions.
 */
UBOOL UGearAnim_BlendByTransition::CanBlendOutFrom()
{
	DEBUGBLENDBYTRANSITION(
		debugf(TEXT("UGearAnim_BlendByTransition::CanBlendOutFrom() bPlayingIntro:%d, bPlayingOutro:%d, bPlayedOutro:%d"), bPlayingIntro, bPlayingOutro, bPlayedOutro);
		debugf(TEXT("    NodeTotalWeight:%f, Children(0).Weight:%f, Children(1).Weight:%f, Children(2).Weight:%f"), NodeTotalWeight, Children(0).Weight, Children(1).Weight, Children(2).Weight);
		)

	// if played outro, then we're ready to blend out
	if( bPlayedOutro )
	{
		return TRUE;
	}
	// if playing intro wait
	else if( bPlayingIntro )
	{
		return FALSE;
	}
	// if playing outro, wait unless we're at the end and we can blend out.
	else if( bPlayingOutro )
	{
		if( Children(2).Anim )
		{
			TArray<UAnimNodeSequence*> SeqNodes;
			Children(2).Anim->GetAnimSeqNodes(SeqNodes);
			UBOOL bFoundPlayingNode = FALSE;
			for(INT i=0; i<SeqNodes.Num(); i++)
			{			
				UAnimNodeSequence *OutroSeqNode = SeqNodes(i);
				if( OutroSeqNode && OutroSeqNode->AnimSeq && OutroSeqNode->bPlaying )
				{
					bFoundPlayingNode = TRUE;
					// If we're at the end, then we can blend out
					if( OutroSeqNode->bRelevant && OutroSeqNode->GetTimeLeft() < ChildBlendInTime(1) )
					{
						return TRUE;
					}
				}
			}

			// If no nodes are playing, then we can bail.
			if( !bFoundPlayingNode )
			{
				return TRUE;
			}
		}

		// delay for outro animation.
		return FALSE;
	}
	// if not playing outro, then trigger it
	else if( !bPlayingOutro )
	{
		// See if we should bypass outro
		UBOOL bBypassOutro = FALSE;
		if( GearPawnOwner )
		{
			if( CoverActionBypassOutro.Num() > 0 )
			{
				for(INT i=0; i<CoverActionBypassOutro.Num(); i++ )
				{
					if( CoverActionBypassOutro(i) == GearPawnOwner->CoverAction )
					{
						bBypassOutro = TRUE;
						break;
					}
				}
			}

			if( !bBypassOutro && b360AimingBypassOutro && GearPawnOwner->bDoing360Aiming )
			{
				bBypassOutro = TRUE;
			}
		}

		if( !bBypassOutro && Children(2).Anim )
		{
			// delay if we can play outro animation
			return !PlayOutro();
		}
		// no outro, don't need to wait
		else
		{
			return TRUE;
		}
	}
	
	// by default we can transition out
	return TRUE;
}

/** Overriden, to be able to blend before animation finishes */
void UGearAnim_BlendByTransition::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if( bPlayingIntro )
	{
		TArray<UAnimNodeSequence*> SeqNodes;
		Children(0).Anim->GetAnimSeqNodes(SeqNodes);
		UBOOL bFoundPlayingNode = FALSE;
		for(INT i=0; i<SeqNodes.Num(); i++)
		{			
			UAnimNodeSequence *SeqNodeIntro = SeqNodes(i);
			if( SeqNodeIntro && SeqNodeIntro->AnimSeq && SeqNodeIntro->bPlaying )
			{
				bFoundPlayingNode = TRUE;
				if( SeqNodeIntro->bRelevant && SeqNodeIntro->GetTimeLeft() <= ChildBlendInTime(1) )
				{
					PlayMain();
					break;
				}
			}
		}

		if( !bFoundPlayingNode )
		{
			PlayMain();
		}
	}
	else if( bPlayedOutro )
	{
		// finished playing outro, see if we can replay the intro
		if( Children(0).Anim )
		{
			// we're still active, then play intro all over again
			if( TotalWeight >= (1.f - ZERO_ANIMWEIGHT_THRESH) )
			{
				bPlayedOutro = FALSE;
				PlayIntro();
			}
		}
	}

	// Update GearAnim_BlendList
	Super::TickAnim(DeltaSeconds, TotalWeight);

	// Update UpdateCoverActionAnimTransition flag
	if( bUpdatePawnActionFiringFlag && GearPawnOwner )
	{
		GearPawnOwner->UpdateCoverActionAnimTransition(NodeTickTag, bRelevant && (NodeTotalWeight * Children(1).Weight < (1.f - ZERO_ANIMWEIGHT_THRESH)) );
	}
}


/************************************************************************************
 * UGearAnim_Corpser_BlendClaw
 ***********************************************************************************/

UAnimNodeSequence* UGearAnim_Corpser_BlendClaw::PlayAttackAnim( int LegIdx, float InTargetWeight, FName AnimSeqName )
{
	INT ChildIdx = LegIdx + 1;

	if( BlendTimeToGo.Num() == 0 )
	{
		BlendTimeToGo.AddZeroed( ChildBlendInTime.Num() );
		TargetWeight.AddZeroed( ChildBlendInTime.Num() );
	}

	BlendTimeToGo(ChildIdx) = ChildBlendInTime(ChildIdx);
	TargetWeight(ChildIdx)	= InTargetWeight;

	UAnimNodeSequence* Seq = Cast<UAnimNodeSequence>(Children(ChildIdx).Anim);
	if( Seq )
	{
		Seq->SetAnim( AnimSeqName );
		Seq->PlayAnim( Seq->bLooping, Seq->Rate );
	}

	return Seq;
}


void UGearAnim_Corpser_BlendClaw::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	for( INT Idx = 0; Idx < Children.Num(); Idx++ )
	{
		UAnimNodeSequence* Seq = Cast<UAnimNodeSequence>(Children(Idx).Anim);
		if( Seq )
		{
			if( Idx == 0 )
			{
				Children(Idx).Weight = 1.f;
			}
			else if( Seq->bPlaying )
			{
				if( Children(Idx).Weight < TargetWeight(Idx) )
				{
					if( BlendTimeToGo(Idx) < DeltaSeconds )
					{
						Children(Idx).Weight = TargetWeight(Idx);
						BlendTimeToGo(Idx) = ChildBlendOutTime(Idx);
					}
					else
					{
						// Amount left to blend
						FLOAT BlendDelta = TargetWeight(Idx) - Children(Idx).Weight; 
						Children(Idx).Weight += (BlendDelta / BlendTimeToGo(Idx)) * DeltaSeconds;
					}
				}
			}
			else
			{
				if( Children(Idx).Weight > 0.f )
				{
					if( BlendTimeToGo(Idx) < DeltaSeconds )
					{
						Children(Idx).Weight = 0.f;
					}
					else
					{
						Children(Idx).Weight -= (Children(Idx).Weight/ BlendTimeToGo(Idx)) * DeltaSeconds;
					}
				}
			}
		}
	}

	Super::TickAnim( DeltaSeconds, TotalWeight );
}


/************************************************************************************
 * UGearAnim_Wretch_BaseBlendNode
 ***********************************************************************************/

void UGearAnim_Wretch_BaseBlendNode::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT	DesiredChildIdx	= 0;

	if( GearPawnOwner )
	{
		DesiredChildIdx	= 0;

		// Otherwise, if already on wall/ceiling
		if( GearPawnOwner->Physics == PHYS_Spider )
		{
			// If on ceiling
			DesiredChildIdx = 2;
		}
		else
		if( GearPawnOwner->CoverType != CT_None ) // next check if we are using cover
		{
			DesiredChildIdx = 3;
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		DesiredChildIdx = appRound( SliderPosition*(Children.Num() - 1) );
	}
#endif

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		SetActiveChild(DesiredChildIdx, DesiredChildIdx < ChildBlendInTime.Num() ? ChildBlendInTime(DesiredChildIdx) : 0.f);
	}

	// and call the parent version to perform the actual interpolation
	UGearAnim_BlendList::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_Wretch_JumpAttack
 ***********************************************************************************/

void UGearAnim_Wretch_JumpAttack::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	INT DesiredChildIdx = AttackState;

	if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		DesiredChildIdx = appRound( SliderPosition*(Children.Num() - 1) );
	}

	// Child needs to be updated
	if( ActiveChildIndex != DesiredChildIdx )
	{
		FLOAT BlendInTime = 0.f;

		if( ActiveChildIndex != -1 )
		{
			if( DesiredChildIdx < ChildBlendInTime.Num() )
			{
				BlendInTime = ChildBlendInTime(DesiredChildIdx);
			}
		}

		SetActiveChild( DesiredChildIdx, BlendInTime );

		UAnimNodeSequence *Seq = Cast<UAnimNodeSequence>(Children(DesiredChildIdx).Anim);
		if( Seq )
		{
			Seq->PlayAnim( Seq->bLooping, Seq->Rate, 0.f );
		}
	}

	Super::TickAnim( DeltaSeconds, TotalWeight );
}

void UGearAnim_Wretch_JumpAttack::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime) 
{
	Super::OnChildAnimEnd(Child, PlayedTime, ExcessTime);

	if( Child == Children(0).Anim && AttackState == WAS_Init )
	{
		AttackState = WAS_AirLoop;
	}
}


/************************************************************************************
 * UGearAnim_BlendBySpeed
 ***********************************************************************************/

FLOAT UGearAnim_BlendBySpeed::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	FLOAT MaxSpeed = Constraints(Constraints.Num() - 1);
	return Speed / MaxSpeed;
}

void UGearAnim_BlendBySpeed::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	FLOAT MaxSpeed = Constraints(Constraints.Num() - 1);
	Speed = NewSliderValue * MaxSpeed;
}

FString UGearAnim_BlendBySpeed::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf(TEXT("%3.2f"), Speed);
}


/**
 * Blend animations based on an Owner's velocity.
 *
 * @param DeltaSeconds	Time since last tick in seconds.
 */
void UGearAnim_BlendBySpeed::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT		NumChannels				= Children.Num();
	UBOOL	bSufficientChannels		= NumChannels >= 2;
	UBOOL	bSufficientConstraints	= Constraints.Num() >= NumChannels;

	if( bSufficientChannels && bSufficientConstraints )
	{
		INT	TargetChannel = 0;

		// Get the speed we should use for the blend.
		Speed = CalcSpeed();

		// Find appropriate channel for current speed with "Constraints" containing an upper speed bound.
		while( (TargetChannel < NumChannels-1) && (Speed > Constraints(TargetChannel)) )
		{
			TargetChannel++;
		}

		// See if we need to blend down.
		if( TargetChannel > 0 )
		{
			FLOAT SpeedRatio = (Speed - Constraints(TargetChannel-1)) / (Constraints(TargetChannel) - Constraints(TargetChannel-1));
			if( SpeedRatio <= BlendDownPerc )
			{
				TargetChannel--;
			}
		}

		if( TargetChannel != LastChannel  )
		{
			if( TargetChannel < LastChannel )
			{
				SetActiveChild(TargetChannel, BlendDownTime);
			}
			else
			{
				SetActiveChild(TargetChannel, BlendUpTime);
			}
			LastChannel = TargetChannel;
		}

		// Scale the rate of the nodes based the ratio between the speed and their current constraint
		for( INT i=0; i<NumChannels; i++ )
		{
			if( Children(i).Anim )
			{
				TArray<UAnimNodeSequence*> SeqNodes;
				Children(i).Anim->GetAnimSeqNodes(SeqNodes);

				FLOAT Rate = 1.f;
				if( Constraints(i) > 0 )
				{
					Rate = Speed / Constraints(i);
				}

				for( INT i=0; i<SeqNodes.Num(); i++ )
				{
					SeqNodes(i)->Rate = Rate;
				}
			}
		}
	}
	else if( !bSufficientChannels )
	{
		debugf(TEXT("UAnimNodeBlendBySpeed::TickAnim - Need at least two children"));
	}
	else if( !bSufficientConstraints )
	{
		debugf(TEXT("UAnimNodeBlendBySpeed::TickAnim - Number of constraints (%i) is lower than number of children! (%i)"), Constraints.Num(), NumChannels);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);				
}


/** 
 *	Function called to calculate the speed that should be used for this node. 
 *	Allows subclasses to easily modify the speed used.
 */
FLOAT UGearAnim_BlendBySpeed::CalcSpeed()
{
	AActor* Owner = SkelComponent ? SkelComponent->GetOwner() : NULL;
	if( Owner )
	{
		return Owner->Velocity.Size();
	}
	else
	{
		return Speed;
	}
}


/************************************************************************************
 * GearAnim_BlendByWeaponType
 ***********************************************************************************/

void UGearAnim_BlendByWeaponType::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}

void UGearAnim_BlendByWeaponType::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT TargetChannel = ActiveChildIndex;

	if( GearPawnOwner )
	{
		AGearWeapon* W = Cast<AGearWeapon>(GearPawnOwner->Weapon);

		if( W )
		{
			switch( W->WeaponAnimType )
			{
				case EWAT_Shotgun		: TargetChannel = 1; break;
				case EWAT_SniperRifle	: TargetChannel = 2; break;
				case EWAT_Pistol		: TargetChannel = 3; break;
				default					: TargetChannel = 0; break;
			}
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	if( ActiveChildIndex != TargetChannel )
	{
		SetActiveChild(TargetChannel, 0.f);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_BlendByHeavyWeaponMount
 ***********************************************************************************/

void UGearAnim_BlendByHeavyWeaponMount::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	AActor*	Owner		= SkelComponent->GetOwner();
	APawn*	MyPawn		= Owner ? Owner->GetAPawn() : NULL;

	// Below is support for weapon animtree owned by pawns.
	if( Owner && !MyPawn )
	{
		MyPawn = Owner->Instigator;
	}
	
	// Update GearPawn ref if it changed.
	if( MyGearPawn != MyPawn )
	{
		MyGearPawn = MyPawn ? Cast<AGearPawn>(MyPawn) : NULL;
	}
}

void UGearAnim_BlendByHeavyWeaponMount::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT TargetChannel = 0;

	if( MyGearPawn && (MyGearPawn->SpecialMove == SM_TargetMinigun || MyGearPawn->SpecialMove == SM_TargetMortar) )
	{
		TargetChannel = 1;
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	if( TargetChannel != ActiveChildIndex )
	{
		SetActiveChild(TargetChannel, bJustBecameRelevant ? 0.f : ChildBlendInTime(TargetChannel));
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/************************************************************************************
 * UGearAnim_BlendByShieldExpand
 ***********************************************************************************/

void UGearAnim_BlendByShieldExpand::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	AActor*	Owner		= SkelComponent->GetOwner();
	APawn*	MyPawn		= Owner ? Owner->GetAPawn() : NULL;

	// Below is support for weapon animtree owned by pawns.
	if( Owner && !MyPawn )
	{
		MyPawn = Owner->Instigator;
	}
	
	// Update GearPawn ref if it changed.
	if( MyGearPawn != MyPawn )
	{
		MyGearPawn = MyPawn ? Cast<AGearPawn>(MyPawn) : NULL;
	}
}

void UGearAnim_BlendByShieldExpand::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT TargetChannel = 0;

	if( MyGearPawn && MyGearPawn->EquippedShield && MyGearPawn->EquippedShield->bExpanded )
	{
		TargetChannel = 1;
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
			// use SliderPosition as we might be in the AnimTree editor
			TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	if( TargetChannel != ActiveChildIndex )
	{
		SetActiveChild(TargetChannel, bJustBecameRelevant ? 0.f : ChildBlendInTime(TargetChannel));
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_BlendByWeaponClass
 ***********************************************************************************/

void UGearAnim_BlendByWeaponClass::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	AActor* Owner = MeshComp->GetOwner();
	if( MyGearPawn != Owner )
	{
		MyGearPawn = Cast<AGearPawn>(Owner);
	}

	// WeaponClassList and Children arrays do not match, argh!
	if( (WeaponClassList.Num() + 1) != Children.Num() )
	{
		WeaponClassList.Reset();
		if( Children.Num() > 1 )
		{
			WeaponClassList.AddZeroed(Children.Num() - 1);
			RenameChildConnectors();
		}
	}
}

void UGearAnim_BlendByWeaponClass::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT TargetChannel = ActiveChildIndex;

	// Track weapon changes
	if( MyGearPawn && MyGearWeapon != MyGearPawn->MyGearWeapon )
	{
		// Default to zero, unless we can find it in our list.
		TargetChannel = 0;

		MyGearWeapon = MyGearPawn->MyGearWeapon;

		if( MyGearWeapon )
		{
			// New weapon set, look up through our list, if we have it.
			for(INT i=0; i<WeaponClassList.Num(); i++)
			{		
				if( WeaponClassList(i) && MyGearWeapon->IsA(WeaponClassList(i)) )
				{
					TargetChannel = i + 1;
					break;
				}
			}
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	if( TargetChannel != ActiveChildIndex )
	{
		SetActiveChild(TargetChannel, 0.f);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/** A child has been added, update Mask accordingly */
void UGearAnim_BlendByWeaponClass::OnAddChild(INT ChildNum)
{
	Super::OnAddChild(ChildNum);

	// Update Mask to match Children array
	if( ChildNum > 0 )
	{
		INT ListIndex = ChildNum - 1;

		if( ListIndex < WeaponClassList.Num() )
		{
			WeaponClassList.InsertZeroed(ListIndex, 1);
		}
		else
		{
			ListIndex = WeaponClassList.AddZeroed(1);
		}
	}
}


/** A child has been removed, update Mask accordingly */
void UGearAnim_BlendByWeaponClass::OnRemoveChild(INT ChildNum)
{
	Super::OnRemoveChild(ChildNum);

	INT ListIndex = ChildNum > 0 ? ChildNum-1 : 0;
	if( ListIndex < WeaponClassList.Num() )
	{
		// Update list to match Children array
		WeaponClassList.Remove(ListIndex);
	}
}

/** Rename all child nodes upon Add/Remove, so they match their position in the array. */
void UGearAnim_BlendByWeaponClass::RenameChildConnectors()
{
	const INT NumChildren = Children.Num();

	if( NumChildren > 0 )
	{
		Children(0).Name = FName(TEXT("Default"));

		for(INT i=1; i<NumChildren; i++)
		{
			Children(i).Name = FName(*FString::Printf(TEXT("%s"), *WeaponClassList(i-1)->GetName()));
		}
	}
}

/** Track Changes, and trigger updates */
void UGearAnim_BlendByWeaponClass::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if( PropertyThatChanged->GetFName() == FName(TEXT("WeaponClassList")) )
	{
		RenameChildConnectors();
	}
}

/************************************************************************************
 * UGearAnim_BlendByDamage
 ***********************************************************************************/

void UGearAnim_BlendByDamage::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	AActor* Owner = MeshComp->GetOwner();
	if( MyGearPawn != Owner )
	{
		MyGearPawn = Cast<AGearPawn>(Owner);
	}
}

void UGearAnim_BlendByDamage::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	INT TargetChannel = ActiveChildIndex;

	if( MyGearPawn )
	{
		if( bRequiresStoppingPower )
		{
			if( MyGearPawn->GetResultingStoppingPower() > 0.f )
			{
				TargetChannel = 1;
			}
			else
			{
				TargetChannel = 0;
			}
		}
		else
		{
			// If we've been hit since Duration time.
			// We check this wide time window in case node just becomes relevant after being hit. So we still blend accordingly.
			if( MyGearPawn->LastTookDamageTime >= (GWorld->GetWorldInfo()->TimeSeconds - Duration) )
			{
				TimeToGo = Duration - (GWorld->GetWorldInfo()->TimeSeconds - MyGearPawn->LastTookDamageTime);
			}

			if( TimeToGo > 0.f )
			{
				TimeToGo -= DeltaSeconds;
				TargetChannel = 1;
			}
			else
			{
				TargetChannel = 0;
			}
		}
	}
#if !CONSOLE
	else if( GIsEditor && !GIsGame )
	{
		// use SliderPosition as we might be in the AnimTree editor
		TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	}
#endif

	if( TargetChannel != ActiveChildIndex )
	{
		SetActiveChild(TargetChannel, TargetChannel == 0 ? BlendOutTime : BlendInTime);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/************************************************************************************
 * UGearAnim_BlendByAngularVelocity
 ***********************************************************************************/

void UGearAnim_BlendByAngularVelocity::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	AActor* Owner = SkelComponent->GetOwner();
	if( Owner )
	{
		INT NewYaw = Owner->Rotation.Yaw;

		// Use base angular velocity if there is some
		if( bUseBaseVelocity && Owner->Base )
		{
			NewYaw += Owner->Base->Rotation.Yaw;
		}

		// If just became relevant, then update Last Yaw. We only update it when this node is ticked.
		if( bJustBecameRelevant )
		{
			LastYaw = NewYaw;
		}

		// Convert to Yaw Angular Velocity
		INT DeltaYaw = NewYaw - LastYaw;
		if(DeltaYaw > 32768)
		{
			DeltaYaw -= 65536;
		}
		else if(DeltaYaw < -32768)
		{
			DeltaYaw += 65536;
		}

		FLOAT YawVel = (FLOAT)(DeltaYaw) / DeltaSeconds;
		LastYaw = NewYaw;

		// We smooth out the blending
		YawVelHistory[YawVelSlot] = YawVel;
		if(++YawVelSlot >= 10)
		{
			YawVelSlot = 0;
		}
		FLOAT SmoothYawVel = GetFloatAverage(YawVelHistory, 10);

		// If moving backwards - invert angular velocity used
		if(bInvertWhenMovingBackwards)
		{
			FVector ForwardDir = Owner->LocalToWorld().GetAxis(0);
			UBOOL bBackwards = (ForwardDir | Owner->Velocity) < 0.1f;
			if(bBackwards)
			{
				SmoothYawVel *= -1.f;
			}
		}

		FLOAT ScaleValue = 1.f;
		if( AIScale != 1.f )
		{
			APawn* PawnOwner = Cast<APawn>(Owner);
			if( PawnOwner && (!PawnOwner->IsHumanControlled() || !PawnOwner->IsLocallyControlled()) )
			{
				ScaleValue = AIScale;
			}
		}

		// Limit how quickly it can change
		FLOAT MaxDelta = DeltaSeconds * AngVelAimOffsetChangeSpeed * ScaleValue;

		FLOAT TargetYawOffset = 0.5f + Clamp(SmoothYawVel * AngVelAimOffsetScale * ScaleValue, -0.5f, 0.5f);
		FLOAT Delta = Clamp(TargetYawOffset - Child2Weight, -MaxDelta, MaxDelta);
		Child2Weight += Delta;
		Child2WeightTarget = Child2Weight;

// 		debugf(TEXT("YawVel: %f, MaxDelta: %f, TargetYawOffset: %f, Child2Weight: %f"), YawVel, MaxDelta, TargetYawOffset, Child2Weight);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/************************************************************************************
 * UGearAnim_BlendByAngularVelocity3
 ***********************************************************************************/

void UGearAnim_BlendByAngularVelocity3::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	// Cache SeqNodes per movement channel
	if( Children(0).Anim )
	{
		AnimSeqNodesRight.Reset();
		Children(0).Anim->GetAnimSeqNodes(AnimSeqNodesLeft, ScaleBySpeedSynchGroupName);
		AnimSeqNodesRight.Shrink();
	}

	if( Children(2).Anim )
	{
		AnimSeqNodesRight.Reset();
		Children(2).Anim->GetAnimSeqNodes(AnimSeqNodesRight, ScaleBySpeedSynchGroupName);
		AnimSeqNodesRight.Shrink();
	}

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}

	Super::InitAnim(MeshComp, Parent);
}

void UGearAnim_BlendByAngularVelocity3::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	AGearPawn* WorkingGearPawn = GearPawnOwner;
	if( bUseInteractionPawn && WorkingGearPawn )
	{
		WorkingGearPawn = WorkingGearPawn->InteractionPawn;
	}

	if( WorkingGearPawn )
	{
		INT NewYaw = WorkingGearPawn->Rotation.Yaw;

		// Use base angular velocity if there is some
		if( bUseBaseVelocity && WorkingGearPawn->Base )
		{
			NewYaw += WorkingGearPawn->Base->Rotation.Yaw;
		}

		// If just became relevant, then update Last Yaw. We only update it when this node is ticked.
		if( bJustBecameRelevant )
		{
			LastYaw = NewYaw;
		}

		// Convert to Yaw Angular Velocity
		INT DeltaYaw = NewYaw - LastYaw;
		if(DeltaYaw > 32768)
		{
			DeltaYaw -= 65536;
		}
		else if(DeltaYaw < -32768)
		{
			DeltaYaw += 65536;
		}

		FLOAT YawAngularVelDegrees = (FLOAT)(DeltaYaw) / (360.f * DeltaSeconds);
		LastYaw = NewYaw;

		// We smooth out the blending
		YawVelHistory[YawVelSlot] = YawAngularVelDegrees;
		if(++YawVelSlot >= 10)
		{
			YawVelSlot = 0;
		}
		FLOAT SmoothYawVel = GetFloatAverage(YawVelHistory, 10);

		// If moving backwards - invert angular velocity used
		if( bInvertWhenMovingBackwards )
		{
			FVector ForwardDir = WorkingGearPawn->LocalToWorld().GetAxis(0);
			UBOOL bBackwards = (ForwardDir | WorkingGearPawn->Velocity) < 0.1f;
			if( bBackwards )
			{
				SmoothYawVel *= -1.f;
			}
		}

		INT DesiredChildIndex = ActiveChildIndex;
		if( Abs(SmoothYawVel) < AngularVelDeadZoneThreshold )
		{
			DesiredChildIndex = 1;
		}
		else 
		{
			if( SmoothYawVel > 0.f )
			{
				DesiredChildIndex = 2;
				for(INT i=0; i<AnimSeqNodesRight.Num(); i++)
				{
					if( AnimSeqNodesRight(i) )
					{
						AnimSeqNodesRight(i)->Rate = Min(Abs(SmoothYawVel) * YawAngularVelSpeedScale, MaxRateScale);
					}
				}
			}
			else
			{
				DesiredChildIndex = 0;
				for(INT i=0; i<AnimSeqNodesLeft.Num(); i++)
				{
					if( AnimSeqNodesLeft(i) )
					{
						AnimSeqNodesLeft(i)->Rate = Min(Abs(SmoothYawVel) * YawAngularVelSpeedScale, MaxRateScale);
					}
				}
			}
		}

		if( ActiveChildIndex != DesiredChildIndex )
		{
			if( ActiveChildIndex == 1 )
			{
				UAnimTree* RootNode = SkelComponent->Animations->GetAnimTree();
				if( RootNode )
				{
					UAnimNodeSequence* Master = RootNode->GetGroupSynchMaster(ScaleBySpeedSynchGroupName);
					if( Master->NodeTotalWeight <= ZERO_ANIMWEIGHT_THRESH )
					{
						RootNode->ForceGroupRelativePosition(ScaleBySpeedSynchGroupName, StartRelPose);
					}
				}
			}
			SetActiveChild(DesiredChildIndex, ChildBlendInTime(DesiredChildIndex));
		}
// 		debugf(TEXT("YawVel: %f, MaxDelta: %f, TargetYawOffset: %f, Child2Weight: %f"), YawVel, MaxDelta, TargetYawOffset, Child2Weight);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/************************************************************************************
 * UGearAnim_BlendBySpeedDirection
 ***********************************************************************************/

void UGearAnim_BlendBySpeedDirection::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}
}

void UGearAnim_BlendBySpeedDirection::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if( GearPawnOwner )
	{
		// Defaults to Idle
		INT DesiredChildIdx = 0;

		const FVector Accel = GearPawnOwner->Acceleration;

		if( !Accel.IsNearlyZero() )
		{
			const FLOAT DotDir = Accel.SafeNormal() | GearPawnOwner->Rotation.Vector();
			if( DotDir >= 0.f )
			{
				DesiredChildIdx = 1; // Forward
			}
			else
			{
				DesiredChildIdx = 2; // Backward
			}
		}

		if( DesiredChildIdx != LastChildIndex )
		{
			// See if children are ready for blend
			if( (!Children(DesiredChildIdx).Anim || Children(DesiredChildIdx).Anim->CanBlendTo()) &&
				(bJustBecameRelevant || !Children(LastChildIndex).Anim || Children(LastChildIndex).Anim->CanBlendOutFrom()) )
			{
				SetActiveChild(DesiredChildIdx, ChildBlendInTime(DesiredChildIdx));
				LastChildIndex = DesiredChildIdx;
			}
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/************************************************************************************
 * UGearAnim_MovementNode
 ***********************************************************************************/

FLOAT UGearAnim_MovementNode::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	const FLOAT MaxSpeed = Movements(Movements.Num()-1).BaseSpeed * 1.5f;
	return Speed / MaxSpeed;
}

void UGearAnim_MovementNode::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	const FLOAT MaxSpeed = Movements(Movements.Num()-1).BaseSpeed * 1.5f;
	Speed = NewSliderValue * MaxSpeed;
}

FString UGearAnim_MovementNode::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf(TEXT("%3.2f"), Speed);
}


void UGearAnim_MovementNode::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}

	// Cast Root Node just once
	if( RootNode != MeshComp->Animations )
	{
		RootNode = Cast<UAnimTree>(MeshComp->Animations);
	}

	InitializeNode();
}

/** Get WorkingPawn */
AGearPawn* UGearAnim_MovementNode::GetWorkingPawn()
{
	if( OwnerType == EOT_Owner )
	{
		return GearPawnOwner;
	}

	if( OwnerType == EOT_Base && GearPawnOwner )
	{
		if( GearPawnOwner->Base != CachedWorkingGearPawn )
		{
			CachedWorkingGearPawn = Cast<AGearPawn>(GearPawnOwner->Base);
		}

		return CachedWorkingGearPawn;
	}

	return NULL;
}

/** Perform node initialization. */
void UGearAnim_MovementNode::InitializeNode()
{
	if( SkelComponent && SkelComponent->Animations )
	{
		TArray<UAnimNode*> Nodes;

		// Find transition nodes
		TransitionNodes.Empty();
		if( Children(EMTC_Transition).Anim != NULL )
		{
			Nodes.Reset();
			Children(EMTC_Transition).Anim->GetNodesByClass(Nodes, UGearAnim_DirectionalMove2Idle::StaticClass());
			for(INT i=0; i<Nodes.Num(); i++)
			{
				UGearAnim_DirectionalMove2Idle* Node = Cast<UGearAnim_DirectionalMove2Idle>(Nodes(i));
				if( Node )
				{
					TransitionNodes.AddItem(Node);
				}
			}
		}

		const UBOOL bFoundTransitionNode = TransitionNodes.Num() > 0;

		// Cache SeqNodes per movement channel
		for(INT i=EMTC_Walk; i<Movements.Num(); i++)
		{
			Movements(i).SeqNodes.Reset();
			if( i < Children.Num() && Children(i).Anim )
			{
				Children(i).Anim->GetAnimSeqNodes(Movements(i).SeqNodes, Name_SynchGroupName);
			}
		}

		/*
		if( bFoundTransitionNode )
		{
			debugf(TEXT("Couldn't initialized walktest. bFoundTransitionNode:%d"), bFoundTransitionNode);
		}
		*/

		//SetActiveChild(0, 0.f);
		GroupRelPos	= RootNode && Name_SynchGroupName != NAME_None ? RootNode->GetGroupRelativePosition(Name_SynchGroupName) : 0.f;
	}
}


/** 
 * See if movement has reached a transition point in the given cycle, 
 * If that's the case, the transition information is returned. Otherwise NULL is returned.
 */
FTransInfo* UGearAnim_MovementNode::GetTransitionInfo(TArray<FTransInfo> &CycleTransInfoList)
{
	UAnimNodeSequence*	MasterNode = RootNode ? RootNode->GetGroupSynchMaster(Name_SynchGroupName) : NULL;

	// Get the relative position of the animation playing on the master node.
	const FLOAT NewPos	= MasterNode ? MasterNode->FindNormalizedPositionFromGroupRelativePosition(GroupRelPos) : 0.f;
	
	// First Pass, see if we are within the range of a transition
	for(INT i=0; i<CycleTransInfoList.Num(); i++)
	{
		FTransInfo &Info = CycleTransInfoList(i);

		// Does the range wrap over?
		const UBOOL	bWrappedRange	= Info.Range.X > Info.Range.Y;

		// Is the current position within the range?
		const UBOOL bIsWithinRange	= bWrappedRange ? 
			(NewPos >= Info.Range.X || NewPos <= Info.Range.Y) :
			(NewPos >= Info.Range.X && NewPos <= Info.Range.Y);

		if( bIsWithinRange )
		{
#if 0 //DEBUG
			debugf(TEXT("%3.2f GetTransitionInfo, bIsWithinRange. Name: %s, NewPos: %3.2f"), GWorld->GetTimeSeconds(), *Info.TransName, NewPos);
#endif
			return &Info;
		}
	}

	const FLOAT OldPos	= MasterNode ? MasterNode->FindNormalizedPositionFromGroupRelativePosition(PrevGroupRelPos) : 0.f;
	
	// Relative position of animation, did we loop?
	const UBOOL	bHasLooped = OldPos > NewPos;

	// Second pass, see if we skipped a transition during this tick
	for(INT i=0; i<CycleTransInfoList.Num(); i++)
	{
		FTransInfo &Info = CycleTransInfoList(i);

		// Have we just skipped the begining of the transition?
		const UBOOL bSkipStart	= bHasLooped ? 
			(Info.Range.X >= OldPos || Info.Range.X <= NewPos) :
			(Info.Range.X >= OldPos && Info.Range.X <= NewPos);

		// Have we just skipped the end of it?
		const UBOOL bSkipEnd	= bHasLooped ? 
			(Info.Range.Y >= OldPos || Info.Range.Y <= NewPos) :
			(Info.Range.Y >= OldPos && Info.Range.Y <= NewPos);

		if( bSkipStart || bSkipEnd )
		{
#if 0 //DEBUG
			debugf(TEXT("%3.2f GetTransitionInfo, skipped. Name: %s, NewPos: %3.2f"), GWorld->GetTimeSeconds(), *Info.TransName, NewPos);
#endif
			return &Info;
		}
	}

	return NULL;
}


void UGearAnim_MovementNode::StartTransition(FTransInfo* Info)
{
	check(Info);

	// Playing transition, disable auto channel adjustement by speed, lock on transition track
	bPlayingTransitionToIdle	= TRUE;
	// Last transition played
	LastTransInfo				= Info;

	SetActiveChild(EMTC_Transition, Info->BlendTime * (1.f - Children(EMTC_Transition).Weight));

	// See how far behind we were from the ideal transition point
	// We can then adjust the transition animation
	FLOAT PosAdjust = 0.f;
	if( Info->bAdjustAnimPos )
	{
		UAnimNodeSequence*	MasterNode = RootNode ? RootNode->GetGroupSynchMaster(Name_SynchGroupName) : NULL;

		if( MasterNode )
		{
			PosAdjust = MasterNode->GetAnimPlaybackLength() * appFmod(MasterNode->FindNormalizedPositionFromGroupRelativePosition(GroupRelPos) - Info->Range.X, 1.f);
		}
	}

	// Start transitions
	for( INT i=0; i<TransitionNodes.Num(); i++ )
	{
		UGearAnim_DirectionalMove2Idle* Node = TransitionNodes(i);
		if( Node )
		{
			Node->SetTransition(Info->TransName, PosAdjust);
		}
	}

	// Turn on root motion mode
	if( SkelComponent )
	{
		SkelComponent->RootMotionMode = RMM_Velocity;
	}

	// If transition requires the player to be in aiming pose.
	if( Info->bForcePlayerToAim && GearPawnOwner )
	{
		GearPawnOwner->eventSetWeaponAlert(GearPawnOwner->AimTimeAfterFiring);
	}
}



void UGearAnim_MovementNode::ResumeTransition(FTransInfo* Info)
{
	check(Info);
	
	// Playing transition, disable auto channel adjustement by speed, lock on transition track
	bPlayingTransitionToIdle = TRUE;

	SetActiveChild(EMTC_Transition, Info->BlendTime * (1.f - Children(EMTC_Transition).Weight));

	// Turn on root motion mode
	if( SkelComponent )
	{
		SkelComponent->RootMotionMode = RMM_Velocity;
	}

	// If transition requires the player to be in aiming pose.
	if( Info->bForcePlayerToAim && GearPawnOwner )
	{
		GearPawnOwner->eventSetWeaponAlert(GearPawnOwner->AimTimeAfterFiring);
	}
}


UBOOL UGearAnim_MovementNode::IsTransitionFinished(FLOAT DeltaTime)
{
	for( INT i=0; i<TransitionNodes.Num(); i++ )
	{
		UGearAnim_DirectionalMove2Idle* Node = TransitionNodes(i);
		if( Node && Node->IsTransitionFinished(DeltaTime) )
		{ 
			return TRUE;
		}
	}

	return FALSE;
}


void UGearAnim_MovementNode::StopTransition(UBOOL bAbortTransition)
{
#if 0 //DEBUG
	debugf(TEXT("%3.2f StopTransition. bAbortTransition: %d"), GWorld->GetTimeSeconds(), bAbortTransition);
#endif

	for( INT i=0; i<TransitionNodes.Num(); i++ )
	{
		UGearAnim_DirectionalMove2Idle* Node = TransitionNodes(i);
		if( Node )
		{
			Node->StopTransition();
		}
	}

	if( GearPawnOwner )
	{
		// Tell Pawn that transition is finished
		GearPawnOwner->eventMove2IdleTransitionFinished();

		// If not aborting a transition, then clear physics, so another transition is not triggered right away
		if( !bAbortTransition )
		{
			GearPawnOwner->Acceleration	= FVector(0.f);
			GearPawnOwner->Velocity		= FVector(0.f);
		}
	}

	// If transition finished properly, we won't be able to resume it.
	if( !bAbortTransition )
	{
		LastTransInfo = NULL;
		SetActiveChild(EMTC_Idle, TransitionBlendOutTime * (1.f - Children(EMTC_Idle).Weight));
	}

	bPlayingTransitionToIdle = FALSE;
	// Root motion will be turned off in special move, for proper transitioning.
}


/** 
 * Returns the Pawn's current speed.
 * Takes into account the SpeedType.
 */
FLOAT UGearAnim_MovementNode::GetCurrentSpeed()
{
	// See which Pawn we should use to get the aim from
	AGearPawn*	WorkingGearPawn = GetWorkingPawn();

	if( !WorkingGearPawn )
	{
		return Speed;
	}

	if( SpeedType == EST_Velocity )
	{
		const FVector BaseVelocity = (WorkingGearPawn->bAnimForceVelocityFromBase && WorkingGearPawn->Base != NULL) ? WorkingGearPawn->Base->Velocity : FVector(0.f);
		return (WorkingGearPawn->Velocity + BaseVelocity).Size2D();
	}
	else if( SpeedType == EST_RootMotion && WorkingGearPawn->Mesh )
	{
		return WorkingGearPawn->Mesh->RootMotionVelocity.Size2D();
	}
	else if( SpeedType == EST_AccelAndMaxSpeed && !WorkingGearPawn->Acceleration.IsNearlyZero() )
	{
		// Acceleration tells if the pawn is moving or not (idle channel or movement channel)
		// MaxSpeed tells which movement channel to play.
		return WorkingGearPawn->GroundSpeed * WorkingGearPawn->MaxSpeedModifier();
	}

	return 0.f;
}

/** Utility function mirroring the above and returning the normalized movement direction vector */
static FVector GetMovementNodeSpeedDirection(AGearPawn *GearPawnToUse, BYTE SpeedType)
{
	if( GearPawnToUse )
	{
		if( SpeedType == EST_Velocity )
		{
			const FVector BaseVelocity = (GearPawnToUse->bAnimForceVelocityFromBase && GearPawnToUse->Base != NULL) ? GearPawnToUse->Base->Velocity : FVector(0.f);
			return (GearPawnToUse->Velocity + BaseVelocity).SafeNormal2D();
		}
		else if( SpeedType == EST_RootMotion && GearPawnToUse->Mesh )
		{
			return GearPawnToUse->Mesh->RootMotionVelocity.SafeNormal2D();
		}
		else if( SpeedType == EST_AccelAndMaxSpeed && !GearPawnToUse->Acceleration.IsNearlyZero() )
		{
			// Acceleration tells if the pawn is moving or not (idle channel or movement channel)
			// MaxSpeed tells which movement channel to play.
			return GearPawnToUse->Acceleration.SafeNormal2D();
		}
	}

	return FVector(1.f, 0.f, 0.f);
}


/**
 */
#define DEBUG_MOVEMENTNODE 0
void UGearAnim_MovementNode::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	PrevGroupRelPos	= GroupRelPos;
	GroupRelPos		= RootNode && Name_SynchGroupName != NAME_None ? RootNode->GetGroupRelativePosition(Name_SynchGroupName) : 0.f;

	if( !bRelevant && !bPlayingTransitionToIdle )
	{
		Super::TickAnim(DeltaSeconds, TotalWeight);
		return;
	}

	UBOOL	bSetChannelBySpeed	= TRUE;
	const FLOAT	CurrentSpeed	= GetCurrentSpeed();

	const UBOOL bWantsToPlayTransition	= GearPawnOwner && GearPawnOwner->IsDoingMove2IdleTransition();

#if DEBUG_MOVEMENTNODE // DEBUG
	debugf(TEXT("%3.2f CurrentSpeed: %4.2f, MovementPct: %2.2f, bWantsToPlayTransition: %d"), GWorld->GetTimeSeconds(), CurrentSpeed, GearPawnOwner ? GearPawnOwner->MovementPct : 0.f, bWantsToPlayTransition);
#endif

	// Player willing to play transition
	if( bShouldHandleTransitions && bWantsToPlayTransition && !bPlayingTransitionToIdle && bRelevant )
	{

#if DEBUG_MOVEMENTNODE // DEBUG
		debugf(TEXT("%3.2f Player willing to play transition..."), GWorld->GetTimeSeconds());
#endif

		// See if we can play a transition animation
		const INT	DominantChannel			= (GearPawnOwner->SpecialMove == SM_Walk2Idle) ? EMTC_Walk : EMTC_Run;
		const UBOOL	bChannelHasTransition	= Movements(DominantChannel).Move2IdleTransitions.Num() > 0;

		// Only try to transition if channel has an available transition
		if( bChannelHasTransition )
		{
			// See if we can find a transition to play
			FTransInfo*	TransitionInfo			= GetTransitionInfo(Movements(DominantChannel).Move2IdleTransitions);

			// See if transition node is relevant. If it is, then transition won't be seemless, we'll see a pop.
			const UBOOL	bTransitionNodeRelevant = Children(EMTC_Transition).Weight > ZERO_ANIMWEIGHT_THRESH;
			
			// This node needs to have full weight otherwise root motion will be weighted
			const UBOOL	bHasFullWeight			= NodeTotalWeight >= (1.f - ZERO_ANIMWEIGHT_THRESH);

			// See if we can resume current transition, if it exists.
			if( bTransitionNodeRelevant && LastTransInfo &&	Children(EMTC_Transition).Weight > TransWeightResumeTheshold &&
				!IsTransitionFinished((TransitionBlendOutTime+LastTransInfo->BlendTime) * 4.f) )
			{
#if DEBUG_MOVEMENTNODE // DEBUG
				debugf(TEXT("%3.2f Resume previous transition."), GWorld->GetTimeSeconds());
#endif
				// Resume transition...
				ResumeTransition(LastTransInfo);

				// Playing transition, disable auto channel adjustement by speed, lock on transition track
				bSetChannelBySpeed = FALSE;
			}
			// Found a transition with no animation (blend to idle directly)
			else if( TransitionInfo && TransitionInfo->bNoTransitionAnim )
			{
#if DEBUG_MOVEMENTNODE // DEBUG
				debugf(TEXT("%3.2f bNoTransitionAnim, Abort transition and blend to idle"), GWorld->GetTimeSeconds());
#endif

				// Abort transition and blend to idle directly.
				StopTransition(FALSE);
				bSetChannelBySpeed = FALSE;

				// If this transition has a custom blend time, use it, other than the default 'TransitionBlendOutTime'.
				if( TransitionInfo->BlendTime > 0.f )
				{
					SetActiveChild(EMTC_Idle, TransitionInfo->BlendTime * (1.f - Children(EMTC_Idle).Weight));
				}
			}
			// Found a transition animation to play
			else if( TransitionInfo && bHasFullWeight && !bTransitionNodeRelevant )
			{
#if DEBUG_MOVEMENTNODE // DEBUG
				debugf(TEXT("%3.2f Reached transition point. Trigger transition: %s for dominant channel: %d"), GWorld->GetTimeSeconds(), *TransitionInfo->TransName.ToString(), DominantChannel);
#endif
				// Let's start the transition
				StartTransition(TransitionInfo);

				// Playing transition, disable auto channel adjustement by speed, lock on transition track
				bSetChannelBySpeed = FALSE;
			}
			else
			{
				// We cannot resume previous transition, since it's not relevant anymore.
				LastTransInfo = NULL;

#if DEBUG_MOVEMENTNODE // DEBUG
				//debugf(TEXT("%3.2f Cannot transition to idle, delay to reach transition point..."), GWorld->GetTimeSeconds());

				// Figure out why we couldn't transition
				if( TransitionInfo  )
				{
					if( !bHasFullWeight )
					{
						debugf(TEXT("%3.2f couldn't transition because bHasFullWeight!!!! NodeTotalWeight: %3.2f"), GWorld->GetTimeSeconds(), NodeTotalWeight);
					}
					
					if( bTransitionNodeRelevant )
					{
						debugf(TEXT("%3.2f couldn't transition because bTransitionNodeRelevant!!!!"), GWorld->GetTimeSeconds());
						for(INT i=0; i<Children.Num(); i++)
						{
							debugf(TEXT("%d - weight:%f"), i, Children(i).Weight);
						}
					}
				} // if( TransitionInfo  )
#endif

			} // if( bIsThisNodeRelevant && !bTransitionNodeRelevant && TransitionInfo )
		} // if( bChannelHasTransition )
		else
		{
#if DEBUG_MOVEMENTNODE // DEBUG
			debugf(TEXT("%3.2f This channel doesn't have a transition. So stop transitioning."), GWorld->GetTimeSeconds());
#endif

			// This movement doesn't have a transition. So just blend to idle...
			StopTransition(FALSE);
			bSetChannelBySpeed = FALSE;
		}
	}
	// Playing move2idle transition
	else if( bPlayingTransitionToIdle )
	{
		// Have we finished playing the transition?
		if( IsTransitionFinished(TransitionBlendOutTime) )
		{
#if DEBUG_MOVEMENTNODE
			debugf(TEXT("%3.2f transition finished."), GWorld->GetTimeSeconds());
#endif

			StopTransition(FALSE);
			bSetChannelBySpeed = FALSE;

			// Reposition feet based on idle position
			RootNode->ForceGroupRelativePosition(Name_SynchGroupName, MoveCycleFirstStepStartPosition);
		}
		else
		{
			const UBOOL bIsInCover =		GearPawnOwner && GearPawnOwner->IsInCover();

			// For increased responsiveness, abort move2idle transition when a pawn in cover does a CoverAction, or wants to move again.
			const UBOOL bWantsToAbort =		bIsInCover && (GearPawnOwner->CoverAction != CA_Default || GearPawnOwner->CurrentSlotDirection != CD_Default);

			// If doing a move2idle transition outside of cover, disable movement when bumping against something
			// We do this because root motion will keep the player sliding against geometry, which looks bad.
			if( !bWantsToAbort && !bIsInCover && GearPawnOwner && GearPawnOwner->IsDoingMove2IdleTransition() )
			{
				// if doing move2idle transition, check for geometry obstruction, in that case we may want to abort movement
				if( GearPawnOwner->bForceMaxAccel || (GearPawnOwner->Mesh && GearPawnOwner->Mesh->RootMotionMode != RMM_Ignore) )
				{
					// Should we abort movement, but keep transition animation playing?
					if( (GWorld->GetTimeSeconds() - GearPawnOwner->LastBumpTime) < 0.33f  )
					{
#if DEBUG_MOVEMENTNODE
						debugf(TEXT("%3.2f Abort move2idle transition."), GWorld->GetTimeSeconds());
#endif
						GearPawnOwner->bForceMaxAccel = FALSE;

						if( GearPawnOwner->Mesh )
						{
							// added to prevent the rootmotion being clobbered, when we've abruptly transitioned from a move2idle
							if( bRelevant && (GearPawnOwner->SpecialMove == SM_Walk2Idle || GearPawnOwner->SpecialMove == SM_Run2Idle) )
							{
								GearPawnOwner->Mesh->RootMotionMode = RMM_Ignore;
							}
						}
					}
				}
			}

			// if pawn wants to move again, allow transition to blend back from idle transition to moving
			if( !bWantsToPlayTransition || bWantsToAbort )
			{
#if DEBUG_MOVEMENTNODE
				debugf(TEXT("%3.2f playing transition, player wants to move again, abort transition!"), GWorld->GetTimeSeconds());
#endif
				// Abort transition, player picks control back up.
				const UBOOL bClearPhysics = !bWantsToAbort;
				StopTransition( bClearPhysics );
			}
			else
			{
#if DEBUG_MOVEMENTNODE
				debugf(TEXT("%3.2f playing transition, waiting for it to finish."), GWorld->GetTimeSeconds());
#endif
				// Lock channel to transition child until we're done transitioning.
				bSetChannelBySpeed = FALSE;
			}
		}
	}

	// If pawn is in cover and not doing a special move and not intentially moving in a direction
	UBOOL bForceIdle = (GearPawnOwner && GearPawnOwner->IsInCover() && (!GearPawnOwner->IsDoingASpecialMove() || GearPawnOwner->IsDoingMove2IdleTransition()) && 
							(GearPawnOwner->CurrentSlotDirection != CD_Left && GearPawnOwner->CurrentSlotDirection != CD_Right));
	
	// Adjust animation channels based on speed
	if( bSetChannelBySpeed || bForceIdle )
	{
		// Target channel for blending
		INT	TargetChannel = ActiveChildIndex;

		// If CurrentSpeed is near zero or if PawnOwner is dead, force going into the Idle channel.
		if( CurrentSpeed <= KINDA_SMALL_NUMBER || (GearPawnOwner && GearPawnOwner->bPlayedDeath) || 
			// If pawn has started a mirror transition, force the idle channel
			// this is to fix some blending issues with HeavyWeapons. They have nasty issues with IK and mirror tables.
			(GearPawnOwner && GearPawnOwner->bDoingMirrorTransition && !GearPawnOwner->MirrorNode->bToggledMirrorStatus) ||
			// Idle was forced
			bForceIdle 
		)
		{
			TargetChannel = EMTC_Idle;
		}
		else 
		{
			// Default GroundSpeed of character is set to be 300.f
			// Adjust constraints relatively to pawn's own speed.
			const FLOAT GroundSpeedScale = bScaleConstraintsByBaseSpeed && GearPawnOwner ? Clamp<FLOAT>(GearPawnOwner->DefaultGroundSpeed / 300.f, 0.5f, 2.f) : 1.f;

			TargetChannel = Movements.Num() - 1;
			for(INT i=Movements.Num()-1; i>EMTC_Walk; i--)
			{
				const FLOAT PrevSpeed	= Movements(i-1).BaseSpeed * GroundSpeedScale;
				const FLOAT Threshold	= PrevSpeed + (Movements(i).BaseSpeed*GroundSpeedScale - PrevSpeed) * BlendDownPerc;

				if( CurrentSpeed <= Threshold )
				{
					TargetChannel = i-1;
				}
			}

			if( GearPawnOwner )
			{
				// If stopping power is applied and we're roadie running, then force us in that stance.
				if( GearPawnOwner->IsDoingSpecialMove(SM_RoadieRun) && CurrentSpeed > 10.f )
				{
					TargetChannel = EMTC_RoadieRun;
				}

				// Force Roadie Run animation when doing a SWAT Turn for the transition.
				if( TargetChannel > EMTC_Walk && GearPawnOwner->IsDoingSpecialMove(SM_StdLvlSwatTurn) )
				{
					TargetChannel = EMTC_RoadieRun;
				}
			}
		}

		// See if we're not allowed to play roadie run animations.
		if( TargetChannel == EMTC_RoadieRun && GearPawnOwner && GearPawnOwner->SpecialMove != SM_Berserker_Charge && GearPawnOwner->SpecialMove != SM_RoadieRun )
		{
			if( (!GearPawnOwner->bCanRoadieRun && !GearPawnOwner->bCanBeForcedToRoadieRun) || (GearPawnOwner->MyGearWeapon && !GearPawnOwner->MyGearWeapon->bAllowsRoadieRunning) )
			{
				TargetChannel--;
			}
		}

		// See if we really should play roadie run animation 
		if( TargetChannel == EMTC_RoadieRun && GearPawnOwner && !GearPawnOwner->IsDoingSpecialMove(SM_StdLvlSwatTurn) )
		{
			// Roadie running has to be forward or it looks goofy
			const FVector DirectionNormal = GetMovementNodeSpeedDirection(GearPawnOwner, SpeedType);
			if( (DirectionNormal | GearPawnOwner->Rotation.Vector()) < 0.3f )
			{
				TargetChannel--;
			}
		}

		// Make sure TargetChannel ends up being in a valid range. ie if we deleted some movements channels, don't trigger that animation.
		if( TargetChannel >= Movements.Num() )
		{
			TargetChannel = Movements.Num() - 1;
		}
		
		// transition to a different channel
		if( TargetChannel != ActiveChildIndex )
		{
#if DEBUG_MOVEMENTNODE
			debugf(TEXT("%3.2f [%s] [%s] Willing to switch from %d to %d"), GWorld->GetTimeSeconds(), *SkelComponent->GetOwner()->GetName(), *GetName(), ActiveChildIndex, TargetChannel);
#endif
			// Make sure we can blend out from children
			if(	ActiveChildIndex < 0 || !Children(ActiveChildIndex).Anim || Children(ActiveChildIndex).Anim->CanBlendOutFrom() )
			{
				FLOAT BlendTime = 0.f;

				// Blending to Idle
				if( TargetChannel < EMTC_Walk )
				{
					BlendTime = TransitionBlendOutTime;
				}
				// Blending from idle to movement
				else if( ActiveChildIndex < EMTC_Walk )
				{
					BlendTime = IdleBlendOutTime;
				}
				else if( TargetChannel < ActiveChildIndex )
				{
					BlendTime = BlendDownTime;
				}
				else
				{
					BlendTime = BlendUpTime;
				}
#if DEBUG_MOVEMENTNODE
				debugf(TEXT("%3.2f Set TargetChannel: %d, LastChannel: %d"), GWorld->GetTimeSeconds(), TargetChannel, ActiveChildIndex);
#endif

				// Blending from Idle to Movement. See if we need to reset starting position.
				if( ActiveChildIndex < EMTC_Walk && TargetChannel >= EMTC_Walk )
				{
					if( RootNode && Name_SynchGroupName != NAME_None )
					{
						UAnimNodeSequence* SynchMaster = RootNode->GetGroupSynchMaster(Name_SynchGroupName);
						
						// If no movement animation is being played, then start from right spot
						if( !SynchMaster || !SynchMaster->bRelevant )
						{
							RootNode->ForceGroupRelativePosition(Name_SynchGroupName, MoveCycleFirstStepStartPosition);
						}
					}
				}
				
				SetActiveChild(TargetChannel, BlendTime);
			} // Make sure we can blend out from children
#if DEBUG_MOVEMENTNODE
			else
			{
				debugf(TEXT("%3.2f [%s] [%s] Cannot BlendOutFrom, delay..."), GWorld->GetTimeSeconds(), *SkelComponent->GetOwner()->GetName(), *GetName());
			}
#endif

		}
	}

	// Scale the rate of the nodes based the ratio between the speed and their current constraint
	if( bScaleAnimationsPlayRateBySpeed )
	{
		// Scale active channel by current speed.
		// Go through all nodes below this channel and override their play rate.
		if( ActiveChildIndex >= EMTC_Walk && ActiveChildIndex < Movements.Num() )
		{
			const INT NumNodes = Movements(ActiveChildIndex).SeqNodes.Num();
			if( NumNodes > 0 && Movements(ActiveChildIndex).BaseSpeed > 0.f )
			{
                // we clamp the Rate here as there are cases where physics or some other portion of the engine will pass a 0 / INF value
                // and then one of the values that control the rate for anim playback goes bonkers causing inf loops.  10 is a good upper
                // bound for how fast the rate for anims should go
				const FLOAT Rate = Min( CurrentSpeed / Movements(ActiveChildIndex).BaseSpeed, 10.0f );

				for(INT NodeIdx=0; NodeIdx<NumNodes; NodeIdx++)
				{
					Movements(ActiveChildIndex).SeqNodes(NodeIdx)->Rate = Rate;
				}
			}
		}
		/*
		for(INT i=EMTC_Walk; i<Movements.Num(); i++)
		{
			const INT NumNodes = Movements(i).SeqNodes.Num();
			if( NumNodes > 0 && Movements(i).BaseSpeed > 0.f )
			{
                // we clamp the Rate here as there are cases where physics or some other portion of the engine will pass a 0 / INF value
                // and then one of the values that control the rate for anim playback goes bonkers causing inf loops.  10 is a good upper
                // bound for how fast the rate for anims should go
				const FLOAT Rate = Min( CurrentSpeed / Movements(i).BaseSpeed, 10.0f );

				for(INT NodeIdx=0; NodeIdx<NumNodes; NodeIdx++)
				{
					Movements(i).SeqNodes(NodeIdx)->Rate = Rate;
				}
			}
		}
		*/
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);	
}


/************************************************************************************
 * UGearAnim_DirectionalMove2Idle
 ***********************************************************************************/

void UGearAnim_DirectionalMove2Idle::SetTransition(FName TransitionName, FLOAT StartPosition)
{
	for( INT i=0; i<DTransList.Num(); i++ )
	{
		if( TransitionName == DTransList(i).TransitionName )
		{
			// make sure animations are set
			Anims(0).AnimName = DTransList(i).AnimName_Fd;
			Anims(1).AnimName = DTransList(i).AnimName_Bd;
			Anims(2).AnimName = DTransList(i).AnimName_Lt;
			Anims(3).AnimName = DTransList(i).AnimName_Rt;

			// Make sure animations are up to date
			CheckAnimsUpToDate();

			// Reset animation playback
			PlayAnim(FALSE, Rate, StartPosition);

			// Make sure root motion is forwarded to component
			RootBoneOption[0] = RBA_Translate;
			RootBoneOption[1] = RBA_Translate;

			break;
		}
	}
}


UBOOL UGearAnim_DirectionalMove2Idle::IsTransitionFinished(FLOAT DeltaTime)
{
	if( !bPlaying || (AnimSeq && ((AnimSeq->SequenceLength - CurrentTime) <= DeltaTime)) )
	{
		return TRUE;
	}

	return FALSE;
}


void UGearAnim_DirectionalMove2Idle::StopTransition()
{
	//RootBoneOption[0] = RBA_Discard;
	//RootBoneOption[1] = RBA_Discard;
}


/************************************************************************************
 * UGearAnim_AimOffset
 ***********************************************************************************/

void UGearAnim_AimOffset::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached PawnOwner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	APawn*	P = A ? A->GetAPawn() : NULL;

	// Below is support for weapon actor owned by Pawns.
	if( A && !P )
	{
		P = A->Instigator;
	}

	if( GearPawnOwner != P )
	{
		GearPawnOwner = P ? Cast<AGearPawn>(P) : NULL;
	}
	
	// Cache Mirror Node
	if( GIsEditor && MeshComp && MeshComp->Animations )
	{
		MirrorNode = Cast<UGearAnim_Mirror_Master>(MeshComp->Animations->FindAnimNode( FName(TEXT("MirrorNode")) ));
	}
}

/** Parent node is requesting a blend out. Give node a chance to delay that. */
UBOOL UGearAnim_AimOffset::CanBlendOutFrom()
{
	// See if any of our relevant children is requesting a delay.
	if( Children(0).Anim && !Children(0).Anim->CanBlendOutFrom() )
	{
		return FALSE;
	}

	return TRUE;
}


/** parent node is requesting a blend in. Give node a chance to delay that. */
UBOOL UGearAnim_AimOffset::CanBlendTo()
{
	// See if any of our relevant children is requesting a delay.
	if( Children(0).Anim && !Children(0).Anim->CanBlendTo() )
	{
		return FALSE;
	}

	return TRUE;
}

/** Pull aim information from Pawn */
FVector2D UGearAnim_AimOffset::GetAim() 
{ 
	// Start with previous Aim value.
	FVector2D	NewAim = Aim;

	// See which Pawn we should use to get the aim from
	AGearPawn*	PawnToUse = (bUseInteractionPawnAim && GearPawnOwner && GearPawnOwner->InteractionPawn) ? GearPawnOwner->InteractionPawn : GearPawnOwner;

	if( PawnToUse )
	{
		if( AimInput == AI_BrumakLeftGun || AimInput == AI_BrumakRightGun )
		{
			AGearPawn_LocustBrumakBase* Brumak = Cast<AGearPawn_LocustBrumakBase>(PawnToUse);
			if( Brumak )
			{
				if( AimInput == AI_BrumakLeftGun )
				{
					NewAim = Brumak->LeftGunAimPct - PawnToUse->AimOffsetPct;
				}
				else if( AimInput == AI_BrumakRightGun )
				{
					NewAim = Brumak->RightGunAimPct - PawnToUse->AimOffsetPct;
				}
			}
		}
		else if( AimInput == AI_PawnPositionAdjust )
		{
			NewAim = PawnToUse->PositionAdjustAimOffsetPct;
		}
		// Extract Aim from Pawn.
		// If bOnlyUpdateIn360Aiming flag is set and Pawn is not in 360 aiming, then
		// stop updating the AimOffset position from the Pawn, use the last value for blending.
		else if( !bOnlyUpdateIn360Aiming || PawnToUse->bDoing360Aiming )
		{
			NewAim	= PawnToUse->AimOffsetPct;
		}
	}
	// Vehicle case
	else if( SkelComponent->GetOwner() && SkelComponent->GetOwner()->IsA(AGearVehicle::StaticClass()) )
	{
		AGearVehicle* VehicleToUse = CastChecked<AGearVehicle>(SkelComponent->GetOwner());

		if( AimInput == AI_VehicleAimOffset )
		{
			NewAim = VehicleToUse->VehicleAimOffset;
		}
	}

	// Backup for next tick, without turn in place offset.
	Aim	= NewAim;

	if( AimInput != AI_PawnPositionAdjust )
	{
		// Add turn in place offset
		NewAim.X += TurnInPlaceOffset;

		// Update mirror node if needed for GearPawn
		if( GearPawnOwner && MirrorNode != GearPawnOwner->MirrorNode )
		{
			MirrorNode = GearPawnOwner->MirrorNode;
		}

		// If mesh is mirrored, flip horizontal axis
		const UBOOL bIsMirrored = (MirrorNode && MirrorNode->bIsMirrored);

		// Are we going to be mirrored or not?
		const UBOOL	bPendingMirrored	= (MirrorNode && MirrorNode->bPendingIsMirrored);

		// If bIsMirrorTransition is FALSE, then invert X when mirrored.
		// if bIsMirrorTransition is TRUE, then invert X when doing a mirrored to non mirrored transition.
		UBOOL bShouldInvertX = bIsMirrorTransition ? !bPendingMirrored : bIsMirrored;

		// MIrror dom when carrying crate
		if(PawnToUse && PawnToUse->CarriedCrate && PawnToUse == PawnToUse->CarriedCrate->DomPawn)
		{
			bShouldInvertX = TRUE;
		}

		if( bShouldInvertX )
		{
			NewAim.X = -NewAim.X;
		}
	}

	return NewAim;
}

void UGearAnim_AimOffset::PostAimProcessing(FVector2D &AimOffsetPct)
{
	// Save original AimOffset passed in.
	const FVector2D OriginalAimOffset = AimOffsetPct;

	// See which Pawn we should use to get the aim from
	AGearPawn*	PawnToUse = (bUseInteractionPawnAim && GearPawnOwner && GearPawnOwner->InteractionPawn) ? GearPawnOwner->InteractionPawn : GearPawnOwner;

	if( bTurnOffWhenReloadingWeapon && PawnToUse )
	{
		const UBOOL bIsReloading = PawnToUse->IsReloadingWeapon();
		if( bIsReloading && !bDoingWeaponReloadInterp )
		{
			bDoingWeaponReloadInterp = TRUE;
			ReloadingBlendTimeToGo = ReloadingBlendTime;
		}
		else if( !bIsReloading && bDoingWeaponReloadInterp )
		{
			bDoingWeaponReloadInterp = FALSE;
			ReloadingBlendTimeToGo = ReloadingBlendTime;
		}

		const FLOAT DeltaTime = GWorld->GetDeltaSeconds();
		const FVector2D Target = bDoingWeaponReloadInterp ? FVector2D(0.f,0.f) : AimOffsetPct;
		if( ReloadingBlendTimeToGo > DeltaTime )
		{
			ReloadingBlendTimeToGo		-= DeltaTime;
			const FVector2D Delta		= Target - LastPostProcessedAimOffset;
			const FVector2D BlendDelta	= Delta * Clamp<FLOAT>((DeltaTime / ReloadingBlendTimeToGo), 0.f, 1.f);
			AimOffsetPct = LastPostProcessedAimOffset + BlendDelta;
		}
		else
		{
			ReloadingBlendTimeToGo	= 0.f;
			AimOffsetPct = Target;
		}
	}
	
	// If LastAimX has been updated last frame.
	if( !bJustBecameRelevant )
	{
		// If wrapping and switching abruptly from once side to the other, trigger smooth interpolation
		if( (AimOffsetPct.X * LastAimOffset.X) < 0.f && Abs(AimOffsetPct.X - LastAimOffset.X) > 0.5f )
		{
			TurnAroundTimeToGo = TurnAroundBlendTime;
		}
	}

	// Perform interpolation
	if( TurnAroundTimeToGo > 0.f )
	{
		const FLOAT	DeltaTime = GWorld->GetDeltaSeconds();
		// Don't perform interpolation when doing a mirror transition or when the node just became relevant.
		if( TurnAroundTimeToGo > DeltaTime && !bJustBecameRelevant && PawnToUse && !PawnToUse->bDoingMirrorTransition )
		{
			TurnAroundTimeToGo		-= DeltaTime;
			const FLOAT Delta		= AimOffsetPct.X - LastPostProcessedAimOffset.X;
			const FLOAT BlendDelta	= Delta * Clamp<FLOAT>((DeltaTime / TurnAroundTimeToGo), 0.f, 1.f);
			AimOffsetPct.X			= LastPostProcessedAimOffset.X + BlendDelta;
		}
		else
		{
			TurnAroundTimeToGo = 0.f;
		}
	}

	// Save our aimoffset values to use for next frame.
	LastPostProcessedAimOffset = AimOffsetPct;
	LastAimOffset = OriginalAimOffset;
}

/************************************************************************************
 * UGearAnim_BlendAnimsByAim
 ***********************************************************************************/

void UGearAnim_BlendAnimsByAim::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}

}

/** Pull aim information from Pawn */
FVector2D UGearAnim_BlendAnimsByAim::GetAim() 
{ 
	// Start with previous Aim value.
	FVector2D	NewAim = Aim;

	// See which Pawn we should use to get the aim from
	AGearPawn*	PawnToUse = GearPawnOwner ? GearPawnOwner : NULL;

	if( PawnToUse )
	{
		if( AimInput == AI_BrumakLeftGun || AimInput == AI_BrumakRightGun )
		{
			AGearPawn_LocustBrumakBase* Brumak = Cast<AGearPawn_LocustBrumakBase>(PawnToUse);
			if( Brumak )
			{
				if( AimInput == AI_BrumakLeftGun )
				{
					NewAim = Brumak->LeftGunAimPct - PawnToUse->AimOffsetPct;
				}
				else if( AimInput == AI_BrumakRightGun )
				{
					NewAim = Brumak->RightGunAimPct - PawnToUse->AimOffsetPct;
				}
			}
		}
		else if( AimInput == AI_PawnPositionAdjust )
		{
			NewAim = PawnToUse->PositionAdjustAimOffsetPct;
		}
		// Extract Aim from Pawn
		else
		{
			NewAim	= PawnToUse->AimOffsetPct;
		}
	}

	// Backup for next tick, without turn in place offset.
	Aim	= NewAim;

	return NewAim;
}


/************************************************************************************
 * UGearSkelCtrl_Recoil
 ***********************************************************************************/

/** Pull aim information from Pawn */
FVector2D UGearSkelCtrl_Recoil::GetAim(USkeletalMeshComponent* SkelComponent) 
{ 
	// Update cached GearPawn pointer. Done to avoid casting every frame
	if( SkelComponent && GearPawnOwner != SkelComponent->GetOwner() )
	{
		GearPawnOwner = SkelComponent ? Cast<AGearPawn>(SkelComponent->GetOwner()) : NULL;
	}

	// extract Aim from Pawn.
	if( GearPawnOwner )
	{
		// Handle mirroring by flipping horizontal axis
		Aim.X = GearPawnOwner->AimOffsetPct.X;
		Aim.Y = GearPawnOwner->AimOffsetPct.Y;
	}

	return Aim;
}

/** Is skeleton currently mirrored */
UBOOL UGearSkelCtrl_Recoil::IsMirrored(USkeletalMeshComponent* SkelComponent) 
{
	return (GearPawnOwner && GearPawnOwner->bIsMirrored);
}


/************************************************************************************
 * UGearAnim_ReverseByDirection
 ***********************************************************************************/

void UGearAnim_ReverseByDirection::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}
}


void UGearAnim_ReverseByDirection::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// If not relevant, don't do anything
	if( !bRelevant )
	{
		Super::TickAnim(DeltaSeconds, TotalWeight);
		return;
	}

	if( GearPawnOwner )
	{
		// Take the most relevant velocity between Actor and root motion.
		const FVector PawnVel		= GearPawnOwner->Velocity;
		const FVector RootMotionVel	= SkelComponent ? SkelComponent->RootMotionVelocity : FVector(0.f);
		FVector	VelDir				= (PawnVel.Size2D() >= RootMotionVel.Size2D()) ? PawnVel : RootMotionVel;

		VelDir.Z = 0.0f;

		if( !VelDir.IsNearlyZero() )
		{
			VelDir = VelDir.SafeNormal();

			FVector LookDir = GearPawnOwner->Rotation.Vector();
			LookDir.Z		= 0.f;
			LookDir			= LookDir.SafeNormal();

			FVector LeftDir = LookDir ^ FVector(0.f,0.f,1.f);
			LeftDir			= LeftDir.SafeNormal();

			FLOAT LeftPct	= LeftDir | VelDir;

			bReversed		= LeftPct > 0.f;
		}
	}

	// Change anim play rate based direction. 
	// if reversed, reverse rate to play animation backwards.
	UBOOL bShouldReverse = bInvertDirection ? !bReversed : bReversed;

	// If Pawn is mirrored, invert animations.
	if( GearPawnOwner && GearPawnOwner->bIsMirrored )
	{
		bShouldReverse = !bShouldReverse;
	}

	if( bShouldReverse )
	{
		Rate = -Abs(Rate);
	}
	else
	{
		Rate = Abs(Rate);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

FLOAT UGearAnim_ReverseByDirection::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	return SliderPosition;
}

void UGearAnim_ReverseByDirection::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	SliderPosition	= NewSliderValue;
	bReversed		= NewSliderValue >= 0.5f;
}

FString UGearAnim_ReverseByDirection::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( SliderPosition >= 0.5f ? TEXT("Reversed") : TEXT("Forward") );
}


/************************************************************************************
 * UGearAnim_BlendAnimsByDirection
 ***********************************************************************************/

void UGearAnim_BlendAnimsByDirection::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}
}


/**
 * Updates weight of the 4 directional animation children by looking at the current velocity and heading of actor.
 */
void UGearAnim_BlendAnimsByDirection::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// if not relevant enough for final blend, pass right through.
	if( !bRelevant )
	{
		Super::TickAnim(DeltaSeconds, TotalWeight);
		return;
	}

	AActor* AOwner = SkelComponent->GetOwner();
	if( OwnerType == EOT_Base && AOwner )
	{
		AOwner = AOwner->Base;
	}

	if( AOwner )
	{
		if( bAddRotationRate )
		{
			if( bJustBecameRelevant )
			{
				LastYaw = AOwner->Rotation.Yaw;
			}

			// Rotation Rate
			YawRotationRate = ((FLOAT)FRotator::NormalizeAxis(AOwner->Rotation.Yaw - LastYaw)) / DeltaSeconds;
			LastYaw = AOwner->Rotation.Yaw;
		}
			
		// Velocity
		MoveDir = AOwner->Velocity;

		FVector LookDir = AOwner->Rotation.Vector().SafeNormal2D();
		FVector LeftDir = (LookDir ^ FVector(0.f,0.f,1.f)).SafeNormal2D();

		// Inverse left direction when mirrored.
		if( GearPawnOwner && GearPawnOwner->bIsMirrored )
		{
			LeftDir *= -1.f;
		}

		// Take rotation rate into account.
		if( bAddRotationRate )
		{
			MoveDir -= LeftDir * YawRotationRate * 0.01f;
		}

		if( MoveDir.SizeSquared2D() > KINDA_SMALL_NUMBER )
		{
			MoveDir = MoveDir.SafeNormal2D();

			FLOAT ForwardPct	= LookDir | MoveDir;
			FLOAT LeftPct		= LeftDir | MoveDir;

			DirAngle = appAcos(Clamp<FLOAT>(ForwardPct, -1.f, 1.f));
			if( LeftPct < 0.f )
			{
				DirAngle = -DirAngle;
			}
		}
	}

	FLOAT ForwardWeight;
	FLOAT BackwardWeight;
	FLOAT LeftWeight;
	FLOAT RightWeight;

	const double HalfPI	= 0.5f * PI;

	// Front is dominant
	if( Abs(DirAngle) <= HalfPI ) 
	{
		ForwardWeight	= 1.f - (Abs(DirAngle) / HalfPI);
		BackwardWeight	= 0.f;

		if( DirAngle > 0.f ) 
		{
			LeftWeight	= 1.f - ForwardWeight;
			RightWeight	= 0.f;
		}
		else
		{
			LeftWeight	= 0.f;
			RightWeight	= 1.f - ForwardWeight;
		}
	}
	// back is dominant
	else
	{
		ForwardWeight	= 0.f;
		BackwardWeight	= (Abs(DirAngle) / HalfPI) - 1.f;

		if( DirAngle > 0.f ) 
		{
			LeftWeight	= 1.f - BackwardWeight;
			RightWeight	= 0.f;
		}
		else
		{
			LeftWeight	= 0.f;
			RightWeight	= 1.f - BackwardWeight;
		}
	}

	// If not interpolating weights, set desired values right away
	if( bJustBecameRelevant )
	{
		Anims(0).Weight = ForwardWeight;
		Anims(1).Weight = BackwardWeight;
		Anims(2).Weight = LeftWeight;
		Anims(3).Weight = RightWeight;
	}
	// Otherwise interpolate from current weights to desired ones
	else
	{
		Anims(0).Weight = FInterpTo(Anims(0).Weight, ForwardWeight,		DeltaSeconds, BlendSpeed);
		Anims(1).Weight = FInterpTo(Anims(1).Weight, BackwardWeight,	DeltaSeconds, BlendSpeed);
		Anims(2).Weight = FInterpTo(Anims(2).Weight, LeftWeight,		DeltaSeconds, BlendSpeed);
		Anims(3).Weight = FInterpTo(Anims(3).Weight, RightWeight,		DeltaSeconds, BlendSpeed);

		FLOAT AccumulatedWeight = 0.f;
		for( INT i=0; i<4; i++ )
		{
			// Accumulate all weights
			AccumulatedWeight += Anims(i).Weight;
		}

		// Renormalize weights, so sum ends up being 1.
		for( INT i=0; i<4; i++ )
		{
			Anims(i).Weight /= AccumulatedWeight;
		}

#if !FINAL_RELEASE
		// Make sure sum is 1.
		check( Abs(1.f - (Anims(0).Weight + Anims(1).Weight + Anims(2).Weight + Anims(3).Weight)) <= ZERO_ANIMWEIGHT_THRESH );

		// make sure no child has a negative weight
		check(Anims(0).Weight >= 0.f);
		check(Anims(1).Weight >= 0.f);
		check(Anims(2).Weight >= 0.f);
		check(Anims(3).Weight >= 0.f);
#endif
	}

	// Tick children
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


FLOAT UGearAnim_BlendAnimsByDirection::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// DirAngle is between -PI and PI. Return value between 0.0 and 1.0 - so 0.5 is straight ahead.
	return 0.5f + (0.5f * (DirAngle / (FLOAT)PI));
}

void UGearAnim_BlendAnimsByDirection::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// Convert from 0.0 -> 1.0 to -PI to PI.
	DirAngle = (FLOAT)PI * 2.f * (NewSliderValue - 0.5f);
}

FString UGearAnim_BlendAnimsByDirection::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%3.2f%c"), DirAngle * (180.f/(FLOAT)PI), 176 );
}


/************************************************************************************
 * UGearAnim_TurnInPlace
 ***********************************************************************************/

void UGearAnim_TurnInPlace::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}

	// Cache anim transition Player nodes.
	PlayerNodes.Reset();
	TArray<UAnimNode*> Nodes;
	GetNodesByClass(Nodes, UGearAnim_TurnInPlace_Player::StaticClass());
	for(INT i=0; i<Nodes.Num(); i++)
	{
		UGearAnim_TurnInPlace_Player* PlayerNode = Cast<UGearAnim_TurnInPlace_Player>(Nodes(i));
		if( PlayerNode )
		{
			PlayerNodes.AddItem(PlayerNode);
		}
	}

	// Find Rotator Node
	UGearAnim_TurnInPlace_Rotator* RotatorNode = NULL;
	Nodes.Reset();
	MeshComp->Animations->GetNodesByClass(Nodes, UGearAnim_TurnInPlace_Rotator::StaticClass());
	for(INT i=0; i<Nodes.Num(); i++)
	{
		if( this->IsChildOf(Nodes(i)) )
		{
			RotatorNode = Cast<UGearAnim_TurnInPlace_Rotator>(Nodes(i));
		}
	}

	// Cache AimOffset nodes to avoid expensive iterations and casts.
	OffsetNodes.Reset();
	Nodes.Reset();
	MeshComp->Animations->GetNodesByClass(Nodes, UGearAnim_AimOffset::StaticClass());
	for(INT i=0; i<Nodes.Num(); i++)
	{
		if( !RotatorNode || Nodes(i)->IsChildOf(RotatorNode) )
		{
			UGearAnim_AimOffset* OffsetNode = Cast<UGearAnim_AimOffset>(Nodes(i));
			if( OffsetNode )
			{
				OffsetNodes.AddItem(OffsetNode);
			}
		}
	}
}


/** Get an AnimNodeSequence playing a transition animation */
UAnimNodeSequence* UGearAnim_TurnInPlace::GetAPlayerNode()
{
	for(INT i=0; i<PlayerNodes.Num(); i++)
	{
		if( PlayerNodes(i) != NULL && PlayerNodes(i)->AnimSeq && PlayerNodes(i)->bRelevant )
		{
			return PlayerNodes(i);
		}
	}

	return NULL;
}

/** Try to delay parent blend out if this node is playing a turn in place animation. */
UBOOL UGearAnim_TurnInPlace::CanBlendOutFrom()
{
	const UBOOL bCanBlendOut = (!bRelevant || !bDelayBlendOutToPlayAnim || !bPlayingTurnTransition);

#if 0
	debugf(TEXT("%3.2f [%s] [%s] CanBlendOutFrom. bCanBlendOut: %d, NodeTotalWeight: %3.2f, bDelayBlendOutToPlayAnim: %d, bPlayingTurnTransition: %d"), GWorld->GetTimeSeconds(), *SkelComponent->GetOwner()->GetName(), *GetName(), INT(bCanBlendOut), NodeTotalWeight, INT(bDelayBlendOutToPlayAnim), INT(bPlayingTurnTransition));
#endif

	return bCanBlendOut;
}


// If TRUE, use Root Bone Rotation from TurnInPlace transition animation.
// Otherwise, use linear interpolation approximation

// @laurent: Currently turned off because prediction code assumes linear interpolation.
// See if i can find a better way to get the prediction working with root rotation.
#define USE_ROOT_BONE_ROTATION 0


void UGearAnim_TurnInPlace::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Update WorkingGearPawn
	AGearPawn* WorkingGearPawn = GearPawnOwner;
	if( OwnerType == EOT_Base && GearPawnOwner )
	{
		if( GearPawnOwner->Base != CachedBaseGearPawn )
		{
			CachedBaseGearPawn = Cast<AGearPawn>(GearPawnOwner->Base);
		}
		WorkingGearPawn = CachedBaseGearPawn;
	}

	//@fixme - figure out a way to compensate for base rotation when calculating turns
	if( WorkingGearPawn && WorkingGearPawn->Base && WorkingGearPawn->Base->Physics == PHYS_Interpolating )
	{
		bInitialized = FALSE;
		Super::TickAnim(DeltaSeconds, TotalWeight);
		return;
	}

	// Get new Pawn rotation
	const INT NewPawnYaw	= WorkingGearPawn ? WorkingGearPawn->Rotation.Yaw : LastPawnYaw;
	PawnRotationRate		= 0.f;

	/** Check if Pawn allows the turn in place to be done */
	const UBOOL	bDoTurnInPlace	= WorkingGearPawn ? WorkingGearPawn->bCanDoTurnInPlaceAnim : (GIsEditor && !GIsGame);

	if( bDoTurnInPlace )
	{
		// if initialized, then accumulate rotation offset
		if( bInitialized )
		{
			// If this node is relevant, track Pawn rotation changes
			const INT DeltaYaw	= FRotator::NormalizeAxis(NewPawnYaw - LastPawnYaw);
			YawOffset			+= DeltaYaw;
			PawnRotationRate	= (FLOAT)DeltaYaw / DeltaSeconds;
		}

		// See if we could abort a transition to start another one
		UBOOL bCouldAbortTransition = FALSE;
		
		// If playing a transition
		if( bPlayingTurnTransition )
		{
			UAnimNodeSequence* PlayerNode = GetAPlayerNode();

			if( PlayerNode )
			{
#if !USE_ROOT_BONE_ROTATION	// LINEAR INTERPOLATION
				const FLOAT AnimLength = PlayerNode->GetAnimPlaybackLength();

				// laurent: Cheating here, linear interpolation of what the animation is supposed to do.
				// This should be using delta root rotations to be more accurate, but it's good enough for now.
				const FLOAT DeltaTurn = ((RotTransitions(CurrentTransitionIndex).RotationOffset / AnimLength) * DeltaSeconds);

				YawOffset -= appFloor(DeltaTurn); 
#endif
				const FLOAT RemainingTime = PlayerNode->GetTimeLeft(); // PlayerNode->AnimSeq->SequenceLength - PlayerNode->CurrentTime;

				// time to blend out?
				if(	!PlayerNode->bPlaying || RemainingTime <= TransitionBlendInTime )
				{
					// Set blend out
					if( Child2WeightTarget != 0.f )
					{
						SetBlendTarget(0.f, RemainingTime);
					}

					// done blending out? transition is finished.
					if( Child2Weight <= ZERO_ANIMWEIGHT_THRESH )
					{
#if 0	// DEBUG
						debugf(TEXT("%3.2f Done Playing TurnInPlace transition."), GWorld->GetTimeSeconds(), YawOffset, RotTransitions(CurrentTransitionIndex).RotationOffset);
#endif

						bPlayingTurnTransition = FALSE;

						// Done turning, renormalize angle.
						YawOffset = FRotator::NormalizeAxis(YawOffset);
					}
				}

				// We can potentially abort our current transition, only up until 50% of the animation
				if( PlayerNode->AnimSeq->SequenceLength > 0.f )
				{
					bCouldAbortTransition = (PlayerNode->CurrentTime / PlayerNode->AnimSeq->SequenceLength) < 0.5f;
				}
			}
			else
			{
				// we should never get here...
//				debugf(TEXT("No Player node while playing transition!!!"));
				SetBlendTarget(0.f, TransitionBlendInTime);
				bPlayingTurnTransition = FALSE;
				YawOffset = FRotator::NormalizeAxis(YawOffset);
			}
		}

		// if not playing a rotation transition, see if we should play one
		if( !bPlayingTurnTransition || bCouldAbortTransition )
		{
			FLOAT	BestOffset	= 0.f;
			INT		BestIndex	= INDEX_NONE;

			const FLOAT NormalizedYawOffset = (FLOAT)FRotator::NormalizeAxis(YawOffset);

			// Find out the time left for the current transition
			// Here we assume that the animation was already set on the node, and that it's the same for every transition...
			UAnimNodeSequence* PlayerNode	= GetAPlayerNode();
			const FLOAT TransitionDuration	= (bPlayingTurnTransition && PlayerNode && PlayerNode->AnimSeq) ? (PlayerNode->GetAnimPlaybackLength() * (1.f - PlayerNode->GetNormalizedPosition())) : (0.f);
			
			// Predict where Yaw Offset would be once the current transition is done playing.
			// This lets the player adjust his current turn to a more suitable one.
			const FLOAT PredictedYawOffset = NormalizedYawOffset + PawnRotationRate * TransitionDuration;

			// See if we can find a suitable transition
			for(INT i=0; i<RotTransitions.Num(); i++)
			{
				const UBOOL bSameDirection			= ((NormalizedYawOffset * RotTransitions(i).RotationOffset) >= 0.f);
				const UBOOL	bBeyondOffsetThreshold	= Abs(PredictedYawOffset) > (Abs(RotTransitions(i).RotationOffset) - TransitionThresholdAngle);

				if( bSameDirection && bBeyondOffsetThreshold ) 
				{
					// Needs to be our highest possibility
					if( BestIndex == INDEX_NONE || Abs(RotTransitions(i).RotationOffset) > Abs(BestOffset) )
					{
						BestOffset	= RotTransitions(i).RotationOffset;
						BestIndex	= i;
					}
				}
			}

			// Did we find a transition to play?
			if( BestIndex != INDEX_NONE )
			{
				// See if we can abort current transition to start a new (better) one
				if( bCouldAbortTransition )
				{
					const UBOOL bSameDirection			= (RotTransitions(CurrentTransitionIndex).RotationOffset * RotTransitions(BestIndex).RotationOffset) > 0.f;
					const UBOOL	bDifferentTransition	= BestIndex != CurrentTransitionIndex;
					const UBOOL bShorterTransition		= Abs(RotTransitions(BestIndex).RotationOffset) < Abs(RotTransitions(CurrentTransitionIndex).RotationOffset);
					// If switching to a shorter transition, make sure we stay within its MaxThreshold, otherwise we'll take another transition after that to go within the threshold.
					const UBOOL bCanTakeShorter			= Abs(PredictedYawOffset) < (Abs(RotTransitions(BestIndex).RotationOffset) + TransitionThresholdAngle);

					if( bSameDirection && bDifferentTransition && (!bShorterTransition || bCanTakeShorter) )
					{
						const FLOAT OldPosition = PlayerNode ? PlayerNode->CurrentTime : 0.f;

#if 0	// DEBUG
						debugf(TEXT("%3.2f AbortTransition to start a new one. YawOffset: %d, PredictedYawOffset: %6.0f, OldTurn: %3.2f, Picked: %3.f, OldPosition: %3.2f"), GWorld->GetTimeSeconds(), YawOffset, PredictedYawOffset, RotTransitions(CurrentTransitionIndex).RotationOffset, RotTransitions(BestIndex).RotationOffset, OldPosition);

						if( bShorterTransition )
						{
							debugf(TEXT("      Playing a Shorter transition"));
						}
						else
						{
							debugf(TEXT("      Playing a Longer transition"));
						}
#endif

						CurrentTransitionIndex = BestIndex;

						// Play transition on all transition anim players
						for(INT i=0; i<PlayerNodes.Num(); i++)
						{
							PlayerNodes(i)->PlayTransition(RotTransitions(CurrentTransitionIndex).TransName, PlayerNodes(i)->TransitionBlendTime);
							PlayerNodes(i)->SetPosition(OldPosition, FALSE);
						}
					}
				}
				else
				{
					bPlayingTurnTransition	= TRUE;
					bRootRotInitialized		= FALSE;
					CurrentTransitionIndex	= BestIndex;
					SetBlendTarget(1.f, TransitionBlendInTime);
#if 0	// DEBUG
					debugf(TEXT("%3.2f Play new transition. YawOffset: %d, PredictedYawOffset: %6.0f, Picked: %3.f"), GWorld->GetTimeSeconds(), YawOffset, PredictedYawOffset, RotTransitions(CurrentTransitionIndex).RotationOffset);
#endif
					// Play transition on all transition anim players
					for(INT i=0; i<PlayerNodes.Num(); i++)
					{
						PlayerNodes(i)->PlayTransition(RotTransitions(CurrentTransitionIndex).TransName, 0.f);
					}
				}
			}
		}
	}

	if( WorkingGearPawn )
	{
		// Update Last saved Pawn rotation.
		LastPawnYaw		= NewPawnYaw;
		// Node is now initialized and can track rotation changes.
		bInitialized	= TRUE;
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UGearAnim_TurnInPlace::OnBecomeRelevant()
{
	Super::OnBecomeRelevant();

	bInitialized = FALSE;
}

void UGearAnim_TurnInPlace::OnCeaseRelevant()
{
	Super::OnCeaseRelevant();

	// If this node is not relevant, then we don't perform any compensation.
	if( YawOffset != 0 )
	{
		YawOffset = 0;

		const INT NumOffsetNodes = OffsetNodes.Num();
		for(INT i=0; i<NumOffsetNodes; i++)
		{
			UGearAnim_AimOffset* OffsetNode = OffsetNodes(i);
			if( OffsetNode )
			{
				OffsetNode->TurnInPlaceOffset = 0.f;
			}
		}
	}

	// If playing a turning transition, then abort it.
	if( bPlayingTurnTransition )
	{
		bPlayingTurnTransition = FALSE;
		SetBlendTarget(0.f, 0.f);
	}
}


void UGearAnim_TurnInPlace::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	// Get Animation data
	{
		EXCLUDE_CHILD_TIME
		Super::GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
	}

	// Root Bone Rotation from Animation_
// This is currently disabled as turn in place is using constant rotation interpolation instead.
#if USE_ROOT_BONE_ROTATION	
	/*
	// compensate for SkeletalMesh RotOrigin offset. 
	// it is unfortunately applied between component and actor, while it should have been between mesh and component...
	const FMatrix CompToMeshAxisSpace	= FRotationMatrix(SkelComponent->SkeletalMesh->RotOrigin);

	// Convert Root bone rotation to normal
	const FVector RootBoneNormal		= FQuatRotationTranslationMatrix(Atoms(0).Rotation, FVector(0.f)).Rotator().Vector();

	// Convert Normal from mesh space to component space
	const FVector CompSpaceNormal		= CompToMeshAxisSpace.TransformNormal(RootBoneNormal);

	const INT NewRootYaw				= CompSpaceNormal.Rotation().Yaw;

	if( bPlayingTurnTransition && bRootRotInitialized && Children(1).Weight >= 1.f - ZERO_ANIMWEIGHT_THRESH )
	{
		const INT DeltaRootYaw = FRotator::NormalizeAxis(NewRootYaw - LastRootBoneYaw);

		// Update Yaw offset
		YawOffset -= DeltaRootYaw; 


	#if 0	// DEBUG
			debugf(TEXT("%3.2f Extracted Root Yaw from animation, DeltaRootYaw: %d, NewRootYaw: %d, YawOffset: %d"), GWorld->GetTimeSeconds(), DeltaRootYaw, NewRootYaw, YawOffset);
	#endif
	}

	LastRootBoneYaw		= NewRootYaw;
	bRootRotInitialized = TRUE;

	// Zero out the root bone rotation.
	Atoms(0).Rotation = FQuat::Identity;
	*/
#endif

	// Update all AimOffset nodes...
	const INT NumOffsetNodes = OffsetNodes.Num();
	if( NumOffsetNodes > 0 )
	{
		RelativeOffset	= ((FLOAT)YawOffset * NodeTotalWeight) / 16384.f;

#if 0
		debugf(TEXT("%3.2f RelativeOffset: %3.2f, DesiredRelativeOffset: %3.2f, YawOffset: %d, RotOffset: %3.2f"), GWorld->GetTimeSeconds(), RelativeOffset, DesiredRelativeOffset, YawOffset, RotTransitions(CurrentTransitionIndex).RotationOffset);
#endif

		for(INT i=0; i<NumOffsetNodes; i++)
		{
			UGearAnim_AimOffset* OffsetNode = OffsetNodes(i);
			if( OffsetNode )
			{
				OffsetNode->TurnInPlaceOffset = RelativeOffset;
			}
		}
	}

	// No need to save cached data, we're not doing anything here.
	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}


FLOAT UGearAnim_TurnInPlace::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	return ((FLOAT)YawOffset / (2.f * 65536.f)) + 0.5f;
}

void UGearAnim_TurnInPlace::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	check( Children.Num() == 2 );

	YawOffset = appCeil((NewSliderValue - 0.5f) * 2.f * 65536.f);
}

FString UGearAnim_TurnInPlace::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("Yaw Offset: %d"), YawOffset);
}


/************************************************************************************
 * UGearAnim_TurnInPlace_Rotator
 ***********************************************************************************/

void UGearAnim_TurnInPlace_Rotator::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache turn in place node
	TurnInPlaceNode = NULL;
	TArray<UAnimNode*> Nodes;
	GetNodesByClass(Nodes, UGearAnim_TurnInPlace::StaticClass());

	for(INT i=0; i<Nodes.Num(); i++)
	{
		// Make sure TurnInPlace node is children of this one.
		if( Nodes(i)->IsChildOf(this) )
		{
			TurnInPlaceNode = Cast<UGearAnim_TurnInPlace>(Nodes(i));
			if( TurnInPlaceNode )
			{
				break;
			}
		}
	}

	// Update cached warpawn owner to avoid expensive casting every tick.
	AActor*	A = MeshComp->GetOwner();
	if( GearPawnOwner != A )
	{
		GearPawnOwner = A != NULL ? Cast<AGearPawn>(A) : NULL;
	}
}

void UGearAnim_TurnInPlace_Rotator::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	// Get Animation data
	{
		EXCLUDE_CHILD_TIME
		Super::GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
	}

	if( TurnInPlaceNode && TurnInPlaceNode->YawOffset != 0 )
	{
		const FLOAT Alpha		= TurnInPlaceNode->NodeTotalWeight / NodeTotalWeight;
		const FLOAT MirrorDir	= (GearPawnOwner && GearPawnOwner->MirrorNode && GearPawnOwner->MirrorNode->bIsMirrored) ? -1.f : 1.f;
		const INT	YawOffset	= appCeil((TurnInPlaceNode->YawOffset * Alpha * MirrorDir) + GearPawnOwner->MeshYawOffset);

		if( YawOffset != 0 )
		{
			const FMatrix	CompToMeshTM = FRotationMatrix(SkelComponent->SkeletalMesh->RotOrigin);
			const FQuat		CompToMeshQuat(CompToMeshTM);

			// Transform root rotation from component space to mesh space
			const FQuat		MeshRootQuat = CompToMeshQuat * Atoms(0).Rotation;

			// Mesh space delta rotation turned into a quaternion
			const FQuat		DeltaQuat = FRotator(0, -YawOffset, 0).Quaternion();

			const FQuat		NewMeshRootQuat	= DeltaQuat * MeshRootQuat;

			// Transform back to component space
			Atoms(0).Rotation = (-CompToMeshQuat) * NewMeshRootQuat;
			Atoms(0).Rotation.Normalize();
		}
	}

	// Save results only we changed anything. Otherwise, just act like a pass through.
	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}


/************************************************************************************
 * UGearAnim_TurnInPlace_Player
 ***********************************************************************************/

/** Play a turn in place transition */
void UGearAnim_TurnInPlace_Player::PlayTransition(FName TransitionName, FLOAT BlendTime)
{
	// Find turn in place transition animation by name
	INT TIP_Index = INDEX_NONE;
	for(INT i=0; i<TIP_Transitions.Num(); i++)
	{
		if( TIP_Transitions(i).TransName == TransitionName )
		{
			TIP_Index = i;
			break;
		}
	}

	if( TIP_Index == INDEX_NONE )
	{
		debugf(TEXT("UGearAnim_TurnInPlace_Player: Transition not found for %s"), *TransitionName.ToString());
		return;
	}

	// Name of animation we want to play
	FName AnimName = TIP_Transitions(TIP_Index).AnimName;

	// See if we're already playing that animation
	INT AnimIndex = INDEX_NONE;
	for(INT i=0; i<Anims.Num(); i++)
	{
		if( Anims(i).AnimName == AnimName )
		{		
			AnimIndex = i;
			break;
		}
	}

	// If we haven't found an existing spot, fiend an unused one.
	if( AnimIndex == INDEX_NONE )
	{
		if( BlendTime > 0.f )
		{
			FLOAT BestWeight = 1.f;
			for(INT i=0; i<Anims.Num(); i++)
			{
				if( Anims(i).Weight < BestWeight )
				{
					BestWeight = Anims(i).Weight;
					AnimIndex = i;
					if( BestWeight == 0.f )
					{
						break;
					}
				}
			}

			// If our node had any weight, we're in trouble! (we'll see a pop)
			if( BestWeight > ZERO_ANIMWEIGHT_THRESH )
			{
				warnf(TEXT("UGearAnim_TurnInPlace_Player::PlayTransition, playing %s on channel %i that has a weight of: %f. Will cause a pop!"), *AnimName.ToString(), AnimIndex, BestWeight);
			}
		}
		else
		{
			AnimIndex = 0;
		}
	}

	// Set the animation to play
	Anims(AnimIndex).AnimName = AnimName;
	CheckAnimsUpToDate();

	// Switch to this channel.
	SetActiveChild(AnimIndex, BlendTime);

	// Start up animation
	PlayAnim(FALSE, Rate, 0.f);
}

void UGearAnim_TurnInPlace_Player::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Do nothing if BlendTimeToGo is zero.
	if( BlendTimeToGo > 0.f )
	{
		// So we don't overshoot.
		if( BlendTimeToGo <= DeltaSeconds )
		{
			BlendTimeToGo = 0.f; 

			const INT ChildNum = Anims.Num();
			for(INT i=0; i<ChildNum; i++)
			{
				Anims(i).Weight = (i == ActiveChildIndex ? 1.f : 0.f);
			}
		}
		else
		{
			const INT ChildNum = Anims.Num();
			for(INT i=0; i<ChildNum; i++)
			{
				// Amount left to blend.
				const FLOAT TargetWeight = (i == ActiveChildIndex ? 1.f : 0.f);
				const FLOAT BlendDelta = TargetWeight - Anims(i).Weight;
				Anims(i).Weight += (BlendDelta / BlendTimeToGo) * DeltaSeconds;
			}

			BlendTimeToGo -= DeltaSeconds;
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UGearAnim_TurnInPlace_Player::SetActiveChild(INT ChildIndex, FLOAT BlendTime)
{
	if( ChildIndex < 0 || ChildIndex >= Anims.Num() )
	{
		debugf( TEXT("UGearAnim_TurnInPlace_Player::SetActiveChild : %s ChildIndex (%d) outside number of Children (%d)."), *GetName(), ChildIndex, Anims.Num() );
		ChildIndex = 0;
	}

	if( BlendTime > 0.f )
	{
		// Scale by destination weight.
		BlendTime *= (1.f - Anims(ChildIndex).Weight);
	}

	BlendTimeToGo = BlendTime;
	ActiveChildIndex = ChildIndex;

	// If blend time is zero, then set weight instantly
	if( BlendTime == 0.0f )
	{
		for(INT i=0; i<Anims.Num(); i++)
		{
			Anims(i).Weight = (i == ActiveChildIndex ? 1.f : 0.f);
		}
	}
}


/************************************************************************************
 * UGearAnim_UpperBodyIKHack
 ***********************************************************************************/

void UGearAnim_UpperBodyIKHack::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	UpdateListOfRequiredBones();
}


void UGearAnim_UpperBodyIKHack::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	AActor* AOwner = SkelComponent->GetOwner();
	if( GearPawnOwner != AOwner )
	{
		GearPawnOwner = Cast<AGearPawn>(AOwner);
	}

	// Update list of required bones
	UpdateListOfRequiredBones();

	// Update Offset Weight Nodes List.
	if( !bUpdatedWeightNodesList || GIsEditor )
	{
		UpdateOffsetWeightNodesList();
	}

	// Cache Mirror Node
	if( GIsEditor && MeshComp && MeshComp->Animations )
	{
		CachedMirrorNode = Cast<UGearAnim_Mirror_Master>(MeshComp->Animations->FindAnimNode( FName(TEXT("MirrorNode")) ));
	}
}

void UGearAnim_UpperBodyIKHack::UpdateOffsetWeightNodesList()
{
	OffsetWeightNodesList.Reset();

	// Get all nodes
	TArray<UAnimNode*>	Nodes;
	SkelComponent->Animations->GetNodes(Nodes);

	// iterate through list of nodes
	for(INT i=0; i<Nodes.Num(); i++)
	{
		UAnimNode* Node = Nodes(i);

		if( Node && Node->NodeName != NAME_None )
		{
			// iterate through our list of names
			for(INT ANodeNameIdx=0; ANodeNameIdx<OffsetWeightNodesNamesList.Num(); ANodeNameIdx++)
			{
				if( Node->NodeName == OffsetWeightNodesNamesList(ANodeNameIdx) )
				{
					OffsetWeightNodesList.AddItem(Node);
					break;
				}
			}
		}
	}

	bUpdatedWeightNodesList = TRUE;
}

void UGearAnim_UpperBodyIKHack::UpdateListOfRequiredBones()
{
	// Empty required bones list
	RequiredBones.Reset();
	RequiredBonesMirrored.Reset();

	// Cache bone Indices
	for(INT i=0; i<BoneCopyArray.Num(); i++)
	{
		BoneCopyArray(i).SrcBoneIndex = SkelComponent->MatchRefBone(BoneCopyArray(i).SrcBoneName);
		BoneCopyArray(i).DstBoneIndex = SkelComponent->MatchRefBone(BoneCopyArray(i).DstBoneName);

		if( BoneCopyArray(i).SrcBoneIndex != INDEX_NONE && BoneCopyArray(i).DstBoneIndex != INDEX_NONE )
		{
			RequiredBones.AddItem(BoneCopyArray(i).SrcBoneIndex);
			RequiredBones.AddItem(BoneCopyArray(i).DstBoneIndex);
		}
	}

	for(INT i=0; i<BoneCopyArrayMirrored.Num(); i++)
	{
		BoneCopyArrayMirrored(i).SrcBoneIndex = SkelComponent->MatchRefBone(BoneCopyArrayMirrored(i).SrcBoneName);
		BoneCopyArrayMirrored(i).DstBoneIndex = SkelComponent->MatchRefBone(BoneCopyArrayMirrored(i).DstBoneName);

		if( BoneCopyArrayMirrored(i).SrcBoneIndex != INDEX_NONE && BoneCopyArrayMirrored(i).DstBoneIndex != INDEX_NONE )
		{
			RequiredBonesMirrored.AddItem(BoneCopyArrayMirrored(i).SrcBoneIndex);
			RequiredBonesMirrored.AddItem(BoneCopyArrayMirrored(i).DstBoneIndex);
		}
	}

	// Make sure parents are present in the array. Since we need to get the relevant bones in component space.
	UAnimNode::EnsureParentsPresent(RequiredBones, SkelComponent->SkeletalMesh);
	UAnimNode::EnsureParentsPresent(RequiredBonesMirrored, SkelComponent->SkeletalMesh);
}

void UGearAnim_UpperBodyIKHack::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Reset node's status
	bNodeDisabled = FALSE;

	if( GearPawnOwner )
	{
		// Disable node for heavy weapons
		if( bDisableForHeavyWeapons && GearPawnOwner->MyGearWeapon && GearPawnOwner->MyGearWeapon->WeaponType == WT_Heavy)
		{
			bNodeDisabled = TRUE;
		}
		else if( bDisableForMeatShield && (GearPawnOwner->SpecialMove == SM_Kidnapper || GearPawnOwner->SpecialMove == SM_Hostage || GearPawnOwner->SpecialMove == SM_Kidnapper_Execution) )
		{
			bNodeDisabled = TRUE;
		}
		else if( GearPawnOwner->SpecialMove == SM_GrapplingHook_Climb )
		{
			bNodeDisabled = TRUE;
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UGearAnim_UpperBodyIKHack::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	{
		EXCLUDE_CHILD_TIME
		Super::GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
	}

	// If Node should be disabled, then act as a pass through.
	if( bNodeDisabled )
	{
		// No need to cache results as well here, as we're not making any modifications.
		return;
	}

	if( GearPawnOwner && CachedMirrorNode != GearPawnOwner->MirrorNode )
	{
		CachedMirrorNode = GearPawnOwner->MirrorNode;
	}

	// Select arrays to use depending on mirrored flag
	const UBOOL bIsMirrored = (CachedMirrorNode && CachedMirrorNode->bIsMirrored);

	TArray<BYTE>&			Reqbones	= bIsMirrored ? RequiredBonesMirrored : RequiredBones;
	TArray<FBoneCopyInfo>&	CopyArray	= bIsMirrored ? BoneCopyArrayMirrored : BoneCopyArray;

	// See how many bones we have to post process, if none, then skip this part.
	const INT NumRequiredBones	= Reqbones.Num();
	const INT NumBonesToCopy	= CopyArray.Num();
	if( NumRequiredBones == 0 || NumBonesToCopy == 0 )
	{
		// No need to cache results as well here, as we're not making any modifications.
		return;
	}

	// Figure out the weight of our OffsetWeightNodesList
	FLOAT OffsetWeight = 0.f;
	for(INT i=0; i<OffsetWeightNodesList.Num() && OffsetWeight < 1.f; i++)
	{
		const UAnimNode* Node = OffsetWeightNodesList(i);
		if( Node && Node->bRelevant )
		{
			OffsetWeight += Node->NodeTotalWeight;
		}
	}
	
	OffsetWeight = Min(OffsetWeight, 1.f);

	const USkeletalMesh*	SkelMesh = SkelComponent->SkeletalMesh;
	const INT				NumBones = SkelMesh->RefSkeleton.Num();

	// We build the mesh-space matrices of the source bones.
	if( BoneTM.Num() < NumBones )
	{
		BoneTM.Reset();
		BoneTM.Add(NumBones);
	}

	// Transform required bones from parent space into component space
	for(INT i=0; i<NumRequiredBones; i++)
	{
		const INT BoneIndex = Reqbones(i);

		// transform required bones into component space
		if( BoneIndex == 0 )
		{
			Atoms(BoneIndex).ToTransform(BoneTM(BoneIndex));
		}
		else
		{
			FMatrix LocalBoneTM;
			Atoms(BoneIndex).ToTransform(LocalBoneTM);
			BoneTM(BoneIndex) = LocalBoneTM * BoneTM(SkelMesh->RefSkeleton(BoneIndex).ParentIndex);
		}
	}

	// Post processing, copy bone informations
	for(INT i=0; i<NumBonesToCopy; i++)
	{
		const INT SrcBoneIndex = CopyArray(i).SrcBoneIndex;
		const INT DstBoneIndex = CopyArray(i).DstBoneIndex;

		if( SrcBoneIndex != INDEX_NONE && DstBoneIndex != INDEX_NONE )
		{
			// @note: transform offset using DstBoneIndex, because in our usage case, this is the IKBone, so rotation is more exact.
			FVector SrcOrigin = BoneTM(SrcBoneIndex).GetOrigin() + OffsetWeight * BoneTM(DstBoneIndex).TransformNormal(CopyArray(i).PositionOffset);

			// Copy Mesh Space location of bone
			BoneTM(DstBoneIndex).SetOrigin(SrcOrigin);

			// Transform back to parent bone space
			FMatrix RelTM = BoneTM(DstBoneIndex) * BoneTM(SkelMesh->RefSkeleton(DstBoneIndex).ParentIndex).InverseSafe();
			const FBoneAtom	TransformedAtom(RelTM);
			Atoms(DstBoneIndex).Rotation	= TransformedAtom.Rotation;
			Atoms(DstBoneIndex).Translation	= TransformedAtom.Translation;

			// At this point the BoneTM Array is not correct anymore for any child bone of DstBoneIndex...
			// Fix if this becomes an issue.
		}
	}

	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}

/************************************************************************************
* UGearSkelCtrl_TurretConstrained
***********************************************************************************/

static INT RotationalTurn(INT Current, INT Desired, FLOAT DeltaRatePerSecond)
{
	const INT DeltaRate = appTrunc( DeltaRatePerSecond );

	if (DeltaRate == 0)
		return (Current & 65535);

	INT result = Current & 65535;
	Current = result;
	Desired = Desired & 65535;

	if (Current > Desired)
	{
		if (Current - Desired < 32768)
			result -= Min((Current - Desired), Abs(DeltaRate));
		else
			result += Min((Desired + 65536 - Current), Abs(DeltaRate));
	}
	else
	{
		if (Desired - Current < 32768)
			result += Min((Desired - Current), Abs(DeltaRate));
		else
			result -= Min((Current + 65536 - Desired), Abs(DeltaRate));
	}
	return (result & 65535);
}

/**********************************************************************************
* UTSkelControl_TurretConstrained
*
* Most of the UT Vehicles use this class to constrain their weapon turrets
**********************************************************************************/

/**
* This function will check a rotational value and make sure it's constrained within
* a given angle.  It returns the value
*/
static INT CheckConstrainValue(INT Rotational, INT MinAngle, INT MaxAngle)
{
	INT NormalizedRotational = Rotational & 65535;

	if (NormalizedRotational > 32767)
	{
		NormalizedRotational = NormalizedRotational - 65535;
	}

	// Convert from Degrees to Unreal Units

	MinAngle = appTrunc( FLOAT(MinAngle) * 182.0444);
	MaxAngle = appTrunc( FLOAT(MaxAngle) * 182.0444);

	if ( NormalizedRotational > MaxAngle )
	{
		return MaxAngle;
	}
	else if ( NormalizedRotational < MinAngle )
	{
		return MinAngle;
	}

	return NormalizedRotational;
}

/** Initialises turret, so its current direction is the way it wants to point. */
void UGearSkelCtrl_TurretConstrained::InitTurret(FRotator InitRot, USkeletalMeshComponent* SkelComp)
{
	// Convert the Desired to Local Space
	FVector LocalDesiredVect = SkelComp->LocalToWorld.InverseTransformNormal( InitRot.Vector() );
	BoneRotation = LocalDesiredVect.Rotation();

	ConstrainedBoneRotation = InitRot;
	DesiredBoneRotation = InitRot;
}

FRotator UGearSkelCtrl_TurretConstrained::GetClampedLocalDesiredRotation(const FRotator& UnclampedLocalDesired)
{
	// Convert the Desired to Local Space
	FRotator LocalDesired = UnclampedLocalDesired;

	LocalDesired.Yaw	*= bInvertYaw   ? -1 : 1;
	LocalDesired.Pitch	*= bInvertPitch ? -1 : 1;
	LocalDesired.Roll	*= bInvertRoll  ? -1 : 1;

	// Constrain the Desired Location.


	// Look up the proper step givin the current yaw

	FTurretConstraintData FMinAngle = MinAngle;
	FTurretConstraintData FMaxAngle = MaxAngle;

	INT NormalizedYaw = LocalDesired.Yaw & 65535;

	for (INT i=0;i<Steps.Num(); i++)
	{
		if ( NormalizedYaw >= Steps(i).StepStartAngle && NormalizedYaw <= Steps(i).StepEndAngle )
		{
			FMinAngle = Steps(i).MinAngle;
			FMaxAngle = Steps(i).MaxAngle;
			break;
		}
	}

	// constrain the rotation
	if (bConstrainYaw)
	{
		LocalDesired.Yaw = CheckConstrainValue(LocalDesired.Yaw, FMinAngle.YawConstraint,FMaxAngle.YawConstraint);
	}
	if (bConstrainPitch)
	{
		LocalDesired.Pitch = CheckConstrainValue(LocalDesired.Pitch, FMinAngle.PitchConstraint,FMaxAngle.PitchConstraint);
	}
	if (bConstrainRoll)
	{
		LocalDesired.Roll = CheckConstrainValue(LocalDesired.Roll, FMinAngle.RollConstraint,FMaxAngle.RollConstraint);
	}

	return LocalDesired;
}

/**
* This function performs the magic.  It will attempt to rotate the turret to face the rotational specified in DesiredBoneRotation.
* 
*/
void UGearSkelCtrl_TurretConstrained::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AGearVehicle* OwnerVehicle = Cast<AGearVehicle>(SkelComp->GetOwner());
	if ( bFixedWhenFiring && OwnerVehicle )
	{
		if (OwnerVehicle->SeatFlashLocation(AssociatedSeatIndex, FVector(0,0,0),TRUE) != FVector(0,0,0) ||
			OwnerVehicle->SeatFlashCount(AssociatedSeatIndex,0,TRUE) > 0 )
		{
			return;
		}
	}

	if ( bResetWhenUnattended && OwnerVehicle &&
		OwnerVehicle->Seats.IsValidIndex(AssociatedSeatIndex) &&
		(OwnerVehicle->SeatMask & (1 << AssociatedSeatIndex)) == 0 )
	{
		StrengthTarget = 0.0;
		ControlStrength = 0.0;
		Super::TickSkelControl(DeltaSeconds, SkelComp);
		return;
	}
	else
	{
		StrengthTarget = 1.0;
		ControlStrength = 1.0;
	}

	FRotator LocalDesired = GetClampedLocalDesiredRotation(SkelComp->LocalToWorld.InverseTransformNormal(DesiredBoneRotation.Vector()).Rotation());

	// If we are not Pointing at the desired rotation, rotate towards it
	FRotator LocalConstrainedBoneRotation = SkelComp->LocalToWorld.InverseTransformNormal(ConstrainedBoneRotation.Vector()).Rotation().GetDenormalized();

	if(bZeroCompletelyConstrained)
	{
		if(bConstrainPitch && (MaxAngle.PitchConstraint - MinAngle.PitchConstraint == 0))
		{
			LocalConstrainedBoneRotation.Pitch = 0;
		}

		if(bConstrainYaw && (MaxAngle.YawConstraint - MinAngle.YawConstraint == 0))
		{
			LocalConstrainedBoneRotation.Yaw = 0;
		}

		if(bConstrainRoll && (MaxAngle.RollConstraint - MinAngle.RollConstraint == 0))
		{
			LocalConstrainedBoneRotation.Roll = 0;
		}
	}

	if (LocalConstrainedBoneRotation != LocalDesired)
	{
		if (LagDegreesPerSecond>0 && SkelComp->GetOwner())
		{
			INT DeltaDegrees = appTrunc((LagDegreesPerSecond * 182.0444) * DeltaSeconds);

			if (LocalConstrainedBoneRotation.Yaw != LocalDesired.Yaw)
			{
				LocalConstrainedBoneRotation.Yaw = SkelComp->GetOwner()->fixedTurn(LocalConstrainedBoneRotation.Yaw, LocalDesired.Yaw, DeltaDegrees);
			}

			if (LocalConstrainedBoneRotation.Pitch != LocalDesired.Pitch)
			{
				LocalConstrainedBoneRotation.Pitch = SkelComp->GetOwner()->fixedTurn(LocalConstrainedBoneRotation.Pitch, LocalDesired.Pitch, appTrunc(DeltaDegrees * PitchSpeedScale) );
			}

			if (LocalConstrainedBoneRotation.Roll != LocalDesired.Roll)
			{
				LocalConstrainedBoneRotation.Roll = SkelComp->GetOwner()->fixedTurn(LocalConstrainedBoneRotation.Roll, LocalDesired.Roll, DeltaDegrees);
			}
		}
		else
		{
			LocalConstrainedBoneRotation = LocalDesired;
		}
	}
	// set the bone rotation to the final clamped value
	UBOOL bNewInMotion;
	if(BoneRotation == LocalConstrainedBoneRotation)
	{
		bNewInMotion = FALSE;
	}
	else
	{
		bNewInMotion = TRUE;
		BoneRotation = LocalConstrainedBoneRotation;
	}

	// also save the current world space rotation for future ticks
	// this is so that the movement of the actor/component won't affect the rotation rate
	ConstrainedBoneRotation = SkelComp->LocalToWorld.TransformNormal(LocalConstrainedBoneRotation.Vector()).Rotation();

	// find out if we're still in motion and call delegate if the status has changed

	if (bNewInMotion != bIsInMotion)
	{
		bIsInMotion = bNewInMotion;

		// Notify anyone listening

		if (DELEGATE_IS_SET(OnTurretStatusChange))
		{
			delegateOnTurretStatusChange(bIsInMotion);
		}
	}
	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

UBOOL UGearSkelCtrl_TurretConstrained::WouldConstrainPitch(INT TestPitch, USkeletalMeshComponent* SkelComp)
{
	FRotator TestRot = SkelComp->LocalToWorld.InverseTransformNormal(FRotator(TestPitch, 0, 0).Vector()).Rotation();
	return (GetClampedLocalDesiredRotation(TestRot).GetDenormalized() == TestRot.GetDenormalized());
}

/************************************************************************************
* UGearSkelCtrl_Trail
***********************************************************************************/

void UGearSkelCtrl_Trail::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	ThisTimstep = DeltaSeconds;
}

void UGearSkelCtrl_Trail::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);

	if( ChainLength < 2 )
	{
		return;
	}

	// The incoming BoneIndex is the 'end' of the spline chain. We need to find the 'start' by walking SplineLength bones up hierarchy.
	// Fail if we walk past the root bone.

	INT WalkBoneIndex = BoneIndex;
	OutBoneIndices.Add(ChainLength); // Allocate output array of bone indices.
	OutBoneIndices(ChainLength-1) = BoneIndex;

	for(INT i=1; i<ChainLength; i++)
	{
		INT OutTransformIndex = ChainLength-(i+1);

		// If we are at the root but still need to move up, chain is too long, so clear the OutBoneIndices array and give up here.
		if(WalkBoneIndex == 0)
		{
			debugf( TEXT("UGearSkelCtrl_Trail : Spline passes root bone of skeleton.") );
			OutBoneIndices.Empty();
			return;
		}
		else
		{
			// Get parent bone.
			WalkBoneIndex = SkelComp->SkeletalMesh->RefSkeleton(WalkBoneIndex).ParentIndex;

			// Insert indices at the start of array, so that parents are before children in the array.
			OutBoneIndices(OutTransformIndex) = WalkBoneIndex;
		}
	}
}

static void CopyRotationPart(FMatrix& DestMatrix, const FMatrix& SrcMatrix)
{
	DestMatrix.M[0][0] = SrcMatrix.M[0][0];
	DestMatrix.M[0][1] = SrcMatrix.M[0][1];
	DestMatrix.M[0][2] = SrcMatrix.M[0][2];

	DestMatrix.M[1][0] = SrcMatrix.M[1][0];
	DestMatrix.M[1][1] = SrcMatrix.M[1][1];
	DestMatrix.M[1][2] = SrcMatrix.M[1][2];

	DestMatrix.M[2][0] = SrcMatrix.M[2][0];
	DestMatrix.M[2][1] = SrcMatrix.M[2][1];
	DestMatrix.M[2][2] = SrcMatrix.M[2][2];
}

void UGearSkelCtrl_Trail::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	if( ChainLength < 2 )
	{
		return;
	}

	OutBoneTransforms.Add(ChainLength);

	// Build array of bone indices - starting at highest bone and running down to end of chain (where controller is)
	// Same code as in GetAffectedBones above!
	TArray<INT> ChainBoneIndices;
	INT WalkBoneIndex = BoneIndex;
	ChainBoneIndices.Add(ChainLength); // Allocate output array of bone indices.
	ChainBoneIndices(ChainLength-1) = BoneIndex;
	for(INT i=1; i<ChainLength; i++)
	{
		check(WalkBoneIndex != 0);

		// Get parent bone.
		WalkBoneIndex = SkelComp->SkeletalMesh->RefSkeleton(WalkBoneIndex).ParentIndex;

		// Insert indices at the start of array, so that parents are before children in the array.
		INT OutTransformIndex = ChainLength-(i+1);
		ChainBoneIndices(OutTransformIndex) = WalkBoneIndex;
	}

	// If we have >0 this frame, but didn't last time, record positions of all the bones.
	// Also do this if number has changed or array is zero.
	UBOOL bHasValidStrength = (ControlStrength > 0.f);
	if(TrailBoneLocations.Num() != ChainLength || (bHasValidStrength && !bHadValidStrength))
	{
		TrailBoneLocations.Empty();
		TrailBoneLocations.Add(ChainLength);

		for(INT i=0; i<ChainBoneIndices.Num(); i++)
		{
			INT ChildIndex = ChainBoneIndices(i);
			TrailBoneLocations(i) = (SkelComp->SpaceBases(ChildIndex) * SkelComp->LocalToWorld).GetOrigin();
		}
	}
	bHadValidStrength = bHasValidStrength;

	// Root bone of trail is not modified.
	INT RootIndex = ChainBoneIndices(0); 
	OutBoneTransforms(0)	= SkelComp->SpaceBases(RootIndex); // Local space matrix
	TrailBoneLocations(0)	= (OutBoneTransforms(0) * SkelComp->LocalToWorld).GetOrigin(); // World space location

	// Starting one below head of chain, move bones.
	for(INT i=1; i<ChainBoneIndices.Num(); i++)
	{
		// Parent bone position in world space.
		INT ParentIndex = ChainBoneIndices(i-1);
		FVector ParentPos = TrailBoneLocations(i-1);
		FVector ParentAnimPos = (SkelComp->SpaceBases(ParentIndex) * SkelComp->LocalToWorld).GetOrigin();

		// Child bone position in world space.
		INT ChildIndex = ChainBoneIndices(i);
		FVector ChildPos = TrailBoneLocations(i);
		FVector ChildAnimPos = (SkelComp->SpaceBases(ChildIndex) * SkelComp->LocalToWorld).GetOrigin();

		// Desired parent->child offset.
		FVector TargetDelta = (ChildAnimPos - ParentAnimPos);

		// Desired child position.
		FVector ChildTarget = ParentPos + TargetDelta;

		// Find vector from child to target
		FVector Error = ChildTarget - ChildPos;

		// Calculate how much to push the child towards its target
		FLOAT Correction = Clamp(ThisTimstep * TrailRelaxation, 0.f, 1.f);
		//FLOAT Correction = Clamp(TrailRelaxation, 0.f, 1.f);

		// Scale correction vector and apply to get new world-space child position.
		TrailBoneLocations(i) = ChildPos + (Error * Correction);

		// If desired, prevent bones stretching too far.
		if(bLimitStretch)
		{
			FLOAT RefPoseLength = TargetDelta.Size();
			FVector CurrentDelta = TrailBoneLocations(i) - TrailBoneLocations(i-1);
			FLOAT CurrentLength = CurrentDelta.Size();

			// If we are too far - cut it back (just project towards parent particle).
			if(CurrentLength - RefPoseLength > StretchLimit)
			{
				FVector CurrentDir = CurrentDelta / CurrentLength;
				TrailBoneLocations(i) = TrailBoneLocations(i-1) + (CurrentDir * (RefPoseLength + StretchLimit));
			}
		}

		// Modify child matrix
		OutBoneTransforms(i) = SkelComp->SpaceBases(ChildIndex);
		OutBoneTransforms(i).SetOrigin( SkelComp->LocalToWorld.Inverse().TransformFVector( TrailBoneLocations(i) ) );

		// Modify rotation of parent matrix to point at this one.

		// Calculate the direction that parent bone is currently pointing.
		FVector CurrentBoneDir = OutBoneTransforms(i-1).TransformNormal( GetAxisDirVector(ChainBoneAxis, bInvertChainBoneAxis) );
		CurrentBoneDir = CurrentBoneDir.SafeNormal();

		// Calculate vector from parent to child.
		FVector NewBoneDir = (OutBoneTransforms(i).GetOrigin() - OutBoneTransforms(i-1).GetOrigin()).SafeNormal();

		// Calculate a quaternion that gets us from our current rotation to the desired one.
		FQuat DeltaLookQuat = FQuatFindBetween(CurrentBoneDir, NewBoneDir);
		FQuatRotationTranslationMatrix DeltaTM( DeltaLookQuat, FVector(0.f) );

		// Apply to the current parent bone transform.
		FMatrix TmpMatrix = FMatrix::Identity;
		CopyRotationPart(TmpMatrix, OutBoneTransforms(i-1));
		TmpMatrix = TmpMatrix * DeltaTM;
		CopyRotationPart(OutBoneTransforms(i-1), TmpMatrix);
	}

	// For the last bone in the chain, use the rotation from the bone above it.
	CopyRotationPart(OutBoneTransforms(ChainLength-1), OutBoneTransforms(ChainLength-2));
}

/************************************************************************************
 * GearSkelCtrl_Spring
 ***********************************************************************************/

void UGearSkelCtrl_Spring::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	ThisTimstep = DeltaSeconds;
}

void UGearSkelCtrl_Spring::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);
	OutBoneIndices.AddItem(BoneIndex);
}

void UGearSkelCtrl_Spring::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	FVector TargetPos = (SkelComp->SpaceBases(BoneIndex) * SkelComp->LocalToWorld).GetOrigin();

	// Init values if first frame on
	UBOOL bHasValidStrength = (ControlStrength > 0.f);
	if((bHasValidStrength && !bHadValidStrength) || (TargetPos - BoneLocation).SizeSquared() > (ErrorResetThresh*ErrorResetThresh) )
	{
		BoneLocation = TargetPos;
		BoneVelocity = FVector(0,0,0);
	}
	bHadValidStrength = bHasValidStrength;

	// Damping is applied in ref frame of base actor
	FVector BaseRelVel = BoneVelocity;
	if(SkelComp && SkelComp->GetOwner() && SkelComp->GetOwner()->GetBase())
	{
		BaseRelVel -= SkelComp->GetOwner()->GetBase()->Velocity;
	}

	// Calculate error vector.
	FVector Error = (TargetPos - BoneLocation);
	// Calculate force based on error and vel
	FVector Force = (SpringStiffness * Error) + (-SpringDamping * BaseRelVel);
	// Integrate velocity
	BoneVelocity += (Force * ThisTimstep);
	// Integrate position
	BoneLocation += (BoneVelocity * ThisTimstep);

	// Force z to be correct if desired
	if(bNoZSpring)
	{
		BoneLocation.Z = TargetPos.Z;
	}

	// If desired, limit error
	if(bLimitDisplacement)
	{
		FVector CurrentDisp = BoneLocation - TargetPos;
		// Too far away - project back onto sphere around target.
		if(CurrentDisp.Size() > MaxDisplacement)
		{
			FVector DispDir = CurrentDisp.SafeNormal();
			BoneLocation = TargetPos + (MaxDisplacement * DispDir);
		}
	}

	// Now convert back into component space and output - rotation is unchanged.
	FMatrix OutBoneTM = SkelComp->SpaceBases(BoneIndex);
	OutBoneTM.SetOrigin( SkelComp->LocalToWorld.Inverse().TransformFVector(BoneLocation) );
	OutBoneTransforms.AddItem(OutBoneTM);
}


/************************************************************************************
 * UGearAnim_Slot
 ***********************************************************************************/


/************************************************************************************
 * UGearAnim_BlendPerBone
 ***********************************************************************************/

#define DEBUG_BLENDPERBONE 0

/** Do any initialisation, and then call InitAnim on all children. Should not discard any existing anim state though. */
void UGearAnim_BlendPerBone::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}

	// Masks and Children arrays do not match, argh!
	if( (MaskList.Num() + 1) != Children.Num() )
	{
		MaskList.Reset();
		if( Children.Num() > 1 )
		{
			MaskList.AddZeroed(Children.Num() - 1);
		}
	}

	// Update weights on all Mask
	// Skeletal Mesh might have changed, and may have been exported differently
	// (ie BoneIndices do not match, so we need to rexport everything based on bone names)
	for(INT i=0; i<MaskList.Num(); i++)
	{
		CalcMaskWeight(i);
	}

	// Update rules
	UpdateRules();
}


void UGearAnim_BlendPerBone::UpdateRules()
{
	//debugf(TEXT("%3.2f UpdateRules"), GWorld->GetTimeSeconds());

	// Cache nodes for weighting rules
	for(INT MaskIndex=0; MaskIndex<MaskList.Num(); MaskIndex++)
	{
		FPerBoneMaskInfo& Mask = MaskList(MaskIndex);

		if( Mask.WeightRuleList.Num() == 0 )
		{
			continue;
		}

		for(INT RuleIndex=0; RuleIndex<Mask.WeightRuleList.Num(); RuleIndex++)
		{
			FWeightRule& Rule = Mask.WeightRuleList(RuleIndex);

			// Find first node
			if( Rule.FirstNode.NodeName != NAME_None )
			{
				Rule.FirstNode.CachedNode = Cast<UAnimNodeBlendBase>(FindAnimNode(Rule.FirstNode.NodeName));
			}
			else
			{
				Rule.FirstNode.CachedNode = NULL;
			}

			// Find second node
			if( Rule.SecondNode.NodeName != NAME_None )
			{
				Rule.SecondNode.CachedNode = Cast<UAnimNodeBlendBase>(FindAnimNode(Rule.SecondNode.NodeName));
			}
			else
			{
				Rule.SecondNode.CachedNode = NULL;
			}
		}
	}
}


/** Track Changes, and trigger updates */
void UGearAnim_BlendPerBone::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// If nothing has changed, skip
	if( !PropertyThatChanged )
	{
		return;
	}

#if 0 // DEBUG
	debugf(TEXT("PropertyThatChanged: %s"), *PropertyThatChanged->GetFName());
#endif

	// Update weights on all Masks
	for(INT MaskIndex=0; MaskIndex<MaskList.Num(); MaskIndex++)
	{
		FPerBoneMaskInfo& Mask = MaskList(MaskIndex);

		if( PropertyThatChanged->GetFName() == FName(TEXT("PerBoneWeightIncrease")) )
		{
			for(INT BranchIndex=0; BranchIndex<Mask.BranchList.Num(); BranchIndex++)
			{
				FBranchInfo& Branch = Mask.BranchList(BranchIndex);

				// Ensure parameters are within reasonable range
				Branch.PerBoneWeightIncrease = Clamp<FLOAT>(Branch.PerBoneWeightIncrease, -1.f, 1.f);
			}
		}

		Mask.DesiredWeight = Clamp<FLOAT>(Mask.DesiredWeight, 0.f, 1.f);

		if( PropertyThatChanged->GetFName() == FName(TEXT("BlendTimeToGo")) )
		{
			Mask.bPendingBlend = TRUE;
            Mask.BlendTimeToGo = Clamp<FLOAT>(Mask.BlendTimeToGo, 0.f, 1.f);
		}

		if( PropertyThatChanged->GetFName() == FName(TEXT("BoneName")) ||
			PropertyThatChanged->GetFName() == FName(TEXT("PerBoneWeightIncrease")) ||
			PropertyThatChanged->GetFName() == FName(TEXT("RotationBlendType")) )
		{
			// Update weights
			CalcMaskWeight(MaskIndex);
		}

		if( PropertyThatChanged->GetFName() == FName(TEXT("NodeName")) )
		{
			UpdateRules();
		}
	}
}


/** Rename all child nodes upon Add/Remove, so they match their position in the array. */
void UGearAnim_BlendPerBone::RenameChildConnectors()
{
	const INT NumChildren = Children.Num();

	if( NumChildren > 0 )
	{
		Children(0).Name = FName(TEXT("Source"));

		for(INT i=1; i<NumChildren; i++)
		{
			Children(i).Name = FName(*FString::Printf(TEXT("Mask %2d"), i-1));
		}
	}
}


/** A child has been added, update Mask accordingly */
void UGearAnim_BlendPerBone::OnAddChild(INT ChildNum)
{
	Super::OnAddChild(ChildNum);

	// Update Mask to match Children array
	if( ChildNum > 0 )
	{
		INT MaskIndex = ChildNum - 1;

		if( MaskIndex < MaskList.Num() )
		{
			MaskList.InsertZeroed(MaskIndex, 1);
			// initialize with defaults
			CalcMaskWeight(MaskIndex);
		}
		else
		{
			MaskIndex = MaskList.AddZeroed(1);
			// initialize with defaults
			CalcMaskWeight(MaskIndex);
		}
	}
}


/** A child has been removed, update Mask accordingly */
void UGearAnim_BlendPerBone::OnRemoveChild(INT ChildNum)
{
	Super::OnRemoveChild(ChildNum);

	INT MaskIndex = ChildNum > 0 ? ChildNum-1 : 0;
	if( MaskIndex < MaskList.Num() )
	{
		// Update Mask to match Children array
		MaskList.Remove(MaskIndex);
	}
}


/**
 * Utility for creating the Mask PerBoneWeights array. 
 * Walks down the hierarchy increasing the weight by PerBoneWeightIncrease each step.
 */
void UGearAnim_BlendPerBone::CalcMaskWeight(INT MaskIndex)
{
	FPerBoneMaskInfo& Mask = MaskList(MaskIndex);

	// Clean per bone weights array
	Mask.PerBoneWeights.Reset();
	// Clear transform required bone indices array
	Mask.TransformReqBone.Reset();

	if( !SkelComponent || !SkelComponent->SkeletalMesh )
	{
		return;
	}

	TArray<FMeshBone>&	RefSkel =	SkelComponent->SkeletalMesh->RefSkeleton;
	const INT			NumBones =	RefSkel.Num();

	Mask.PerBoneWeights.AddZeroed(NumBones);
	Mask.PerBoneWeights.Shrink();
	
	/** Should transform parent space bone atoms to mesh space? */
	const UBOOL	bDoMeshSpaceTransform = (RotationBlendType == EBT_MeshSpace);

	FLOAT LastWeight = 0.f;
	for(INT i=0; i<NumBones; i++)
	{
		for(INT BranchIdx=0; BranchIdx<Mask.BranchList.Num(); BranchIdx++)
		{
			FBranchInfo& Branch = Mask.BranchList(BranchIdx);

			// If no bone name supplied abort. Assume it wanted to be cleared.
			if( Branch.BoneName == NAME_None )
			{
				continue;
			}

			const INT StartBoneIndex = SkelComponent->MatchRefBone(Branch.BoneName);

			if( StartBoneIndex != INDEX_NONE )
			{
				if( StartBoneIndex == i || SkelComponent->SkeletalMesh->BoneIsChildOf(i, StartBoneIndex) )
				{
					const FLOAT ParentWeight	= Mask.PerBoneWeights(RefSkel(i).ParentIndex);
					const FLOAT WeightIncrease	= ParentWeight + Branch.PerBoneWeightIncrease;
					Mask.PerBoneWeights(i)		= Clamp<FLOAT>(Mask.PerBoneWeights(i) + WeightIncrease, 0.f, 1.f);
				}
			}
		}

#if DEBUG_BLENDPERBONE //DEBUG
		// Log per bone weights
		debugf(TEXT("Bone: %3d, Weight: %1.2f"), i, Mask.PerBoneWeights(i));
#endif
		// If rotation blending is done in mesh space, then fill up the TransformReqBone array.
		if( bDoMeshSpaceTransform )
		{
			if( i == 0 )
			{
				LastWeight = Mask.PerBoneWeights(i);
			}
			else
			{
				// if weight different than previous one, then this bone needs to be blended in mesh space
				if( Mask.PerBoneWeights(i) != LastWeight )
				{
					Mask.TransformReqBone.AddItem(i);
					LastWeight = Mask.PerBoneWeights(i);

#if DEBUG_BLENDPERBONE 
					debugf(TEXT("  Bone added to TransformReqBone."));
#endif
				}
			}
		}

	}

	// Make sure parents are present
	EnsureParentsPresent(Mask.TransformReqBone, SkelComponent->SkeletalMesh);

#if DEBUG_BLENDPERBONE 
	for(INT i=0; i<Mask.TransformReqBone.Num(); i++)
	{
		debugf(TEXT("TransformReqBone: %d"), Mask.TransformReqBone(i));
	}
#endif	
}


static inline UBOOL CheckNodeRule(const FWeightNodeRule &NodeRule)
{
	if( NodeRule.CachedNode )
	{
		switch( NodeRule.WeightCheck )
		{
			case EWC_ChildIndexNotFullWeight	: return (NodeRule.CachedNode->Children(NodeRule.ChildIndex).Weight < 1.f - ZERO_ANIMWEIGHT_THRESH);
			case EWC_ChildIndexFullWeight		: return (NodeRule.CachedNode->Children(NodeRule.ChildIndex).Weight >= 1.f - ZERO_ANIMWEIGHT_THRESH);
			case EWC_ChildIndexRelevant			: return (NodeRule.CachedNode->Children(NodeRule.ChildIndex).Weight > ZERO_ANIMWEIGHT_THRESH);
			case EWC_ChildIndexNotRelevant		: return (NodeRule.CachedNode->Children(NodeRule.ChildIndex).Weight <= ZERO_ANIMWEIGHT_THRESH);
		}
	}

	return FALSE;
}


/** Ticking, updates weights... */
void UGearAnim_BlendPerBone::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Update weights for each branch
	const INT NumMasks = MaskList.Num();
	for(INT i=0; i<NumMasks; i++)
	{
		FPerBoneMaskInfo&	Mask		= MaskList(i);
		const INT			ChildIdx	= i + 1;
		FAnimBlendChild&	Child		= Children(ChildIdx);

		// See if this mask should be disabled for non local human players
		// (ie AI, other players in network...)
		if( Mask.bDisableForNonLocalHumanPlayers )
		{
			AActor* Owner		= SkelComponent->GetOwner();
			APawn*	PawnOwner	= Owner ? Owner->GetAPawn() : NULL;

			if( !PawnOwner || !PawnOwner->Controller || !PawnOwner->Controller->LocalPlayerController() )
			{
				Child.Weight = 0.f;
				continue;
			}
		}

		// Disable if CameraBoneMotionScale is zero
		if( Mask.bDisableIfCameraBoneMotionScaleIsZero )
		{
			if( GearPawnOwner && GearPawnOwner->CameraBoneMotionScale <= KINDA_SMALL_NUMBER )
			{
				Child.Weight = 0.f;
				continue;
			}
		}

		// Check rules, if they're all TRUE, then we deactivate the mask
		// This will turn the node into a pass through for optimization
		if( Mask.bWeightBasedOnNodeRules )
		{
			UBOOL	bRuleFailed = FALSE;

			for(INT RuleIndex=0; RuleIndex<Mask.WeightRuleList.Num(); RuleIndex++)
			{
				FWeightRule& Rule = Mask.WeightRuleList(RuleIndex);
				
				// Check rule on first node
				if( Rule.FirstNode.CachedNode )
				{
					bRuleFailed = !CheckNodeRule(Rule.FirstNode);
				}
				
				// Check rule on second node
				if( !bRuleFailed && Rule.SecondNode.CachedNode )
				{
					bRuleFailed = !CheckNodeRule(Rule.SecondNode);
				}

				// This rule failed, so no need to go further
				if( bRuleFailed )
				{
					break;
				}
			}

			// If none of the rules have failed, then we can lock the mask to a zero 0 weight (== pass through)
			if( !bRuleFailed )
			{
				Child.Weight = 0.f;
				continue;
			}
			// Otherwise, this child can be relevant.
			else if( Mask.BlendTimeToGo == 0.f )
			{
				Child.Weight = Mask.DesiredWeight;
			}
		}

		if( MaskList(i).BlendTimeToGo != 0.f )
		{
			const FLOAT	BlendDelta = Mask.DesiredWeight - Child.Weight; 

			if( Mask.bPendingBlend && Child.Anim )
			{
				// See if blend in is authorized by child
				if( BlendDelta > 0.f  && !Child.Anim->CanBlendTo() )
				{
					continue;
				}

				// See if blend out is authorized by child
				if( BlendDelta < 0.f && !Child.Anim->CanBlendOutFrom() )
				{
					continue;
				}
			}

			// Blend approved!
			Mask.bPendingBlend = FALSE;

			if( Abs(BlendDelta) > KINDA_SMALL_NUMBER && Mask.BlendTimeToGo > DeltaSeconds )
			{
				Child.Weight		+= (BlendDelta / Mask.BlendTimeToGo) * DeltaSeconds;
				Mask.BlendTimeToGo	-= DeltaSeconds;
			}
			else
			{
				Child.Weight		= Mask.DesiredWeight;
				Mask.BlendTimeToGo	= 0.f;
			}
		}	
		else
		{
			Child.Weight = Mask.DesiredWeight;
		}
	}

	// Call parent version.
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/** 
 * Control the weight of a given Mask.
 */
void UGearAnim_BlendPerBone::SetMaskWeight(INT MaskIndex, FLOAT DesiredWeight, FLOAT BlendTime)
{
	if( MaskIndex >= MaskList.Num() )
	{
		debugf(TEXT("SetMaskWeight, MaskIndex: %d out of bounds."), MaskIndex);
		return;
	}

	FPerBoneMaskInfo& Mask = MaskList(MaskIndex);

	// Set desired weight
	Mask.DesiredWeight = Clamp<FLOAT>(DesiredWeight, 0.f, 1.f);

	const INT ChildIdx		= MaskIndex + 1;
	FAnimBlendChild& Child	= Children(ChildIdx);

	const FLOAT BlendDelta	= Mask.DesiredWeight - Child.Weight; 
	UBOOL bCanDoBlend		= TRUE;

	// Scale blend time by weight left to blend. So it gives a constant blend time
	// No matter what state the blend is in already
	BlendTime *= Abs(BlendDelta);

	if( Child.Anim )
	{
		// See if blend in is authorized by child
		if( BlendDelta > 0.f && !Child.Anim->CanBlendTo() )
		{
			bCanDoBlend = FALSE;
		}

		// See if blend out is authorized by child
		if( BlendDelta < 0.f && !Child.Anim->CanBlendOutFrom() )
		{
			bCanDoBlend = FALSE;
		}
	}

	// If no time, then set instantly
	if( BlendTime < KINDA_SMALL_NUMBER )
	{
		if( !bCanDoBlend )
		{
			// If can't blend yet, delay it...
			Mask.BlendTimeToGo = (FLOAT)KINDA_SMALL_NUMBER;
		}
		else
		{
			Mask.BlendTimeToGo	= 0.f;
			Child.Weight		= Mask.DesiredWeight;
		}
	}
	// Blend over time
	else
	{
		Mask.bPendingBlend = TRUE;
		Mask.BlendTimeToGo = BlendTime;
	}
}


/** Parent node is requesting a blend out. Give node a chance to delay that. */
UBOOL UGearAnim_BlendPerBone::CanBlendOutFrom()
{
	// See if any of our relevant children is requesting a delay.
	if( bRelevant )
	{
		for(INT i=0; i<Children.Num(); i++)
		{
			if( Children(i).Anim && Children(i).Anim->bRelevant && !Children(i).Anim->CanBlendOutFrom() )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


/** parent node is requesting a blend in. Give node a chance to delay that. */
UBOOL UGearAnim_BlendPerBone::CanBlendTo()
{
	// See if any of our relevant children is requesting a delay.
	if( bRelevant )
	{
		for(INT i=0; i<Children.Num(); i++)
		{
			if( Children(i).Anim && Children(i).Anim->bRelevant && !Children(i).Anim->CanBlendTo() )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


/** 
 * Calculates total weight of children. 
 * Set a full weight on source, because it's potentially always feeding animations into the final blend.
 */
void UGearAnim_BlendPerBone::SetChildrenTotalWeightAccumulator(const INT Index)
{
	if( Index == 0 )
	{
		// Update the weight of this connection
		Children(Index).TotalWeight = NodeTotalWeight;
		// Update the accumulator to find out the combined weight of the child node
		Children(Index).Anim->TotalWeightAccumulator += NodeTotalWeight;
	}
	else
	{
		// Update the weight of this connection
		Children(Index).TotalWeight = NodeTotalWeight * Children(Index).Weight;;
		// Update the accumulator to find out the combined weight of the child node
		Children(Index).Anim->TotalWeightAccumulator += Children(Index).TotalWeight;
	}
}


/** 
 *	Utility for taking two arrays of bytes, which must be strictly increasing, and finding the intersection between them.
 *	That is - any item in the output should be present in both A and B. Output is strictly increasing as well.
 */
static void IntersectByteArrays(TArray<BYTE>& Output, const TArray<BYTE>& A, const TArray<BYTE>& B)
{
	INT APos = 0;
	INT BPos = 0;
	while(	APos < A.Num() && BPos < B.Num() )
	{
		// If value at APos is lower, increment APos.
		if( A(APos) < B(BPos) )
		{
			APos++;
		}
		// If value at BPos is lower, increment APos.
		else if( B(BPos) < A(APos) )
		{
			BPos++;
		}
		// If they are the same, put value into output, and increment both.
		else
		{
			Output.AddItem( A(APos) );
			APos++;
			BPos++;
		}
	}
}

extern TArray<FMatrix> GChild1CompSpace; // SourceMeshSpaceTM
extern TArray<FMatrix> GChild2CompSpace; // ResultMeshSpaceTM

/** @see UAnimNode::GetBoneAtoms. */
void UGearAnim_BlendPerBone::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	const TArray<FMeshBone>& RefSkel = SkelComponent->SkeletalMesh->RefSkeleton;
	
	// Handle case with no child connectors
	if( Children.Num() == 0 )
	{
		RootMotionDelta = FBoneAtom::Identity;
		bHasRootMotion	= 0;
		FillWithRefPose(Atoms, DesiredBones, RefSkel);
		return;
	}

	// Build array of relevant children to scan
	TArray<INT>	RelevantChildren;
	// Find index of the last child with a non-zero weight.
	INT LastChildIndex = INDEX_NONE;
	const INT NumChildren = Children.Num();
	
	RelevantChildren.Reserve(NumChildren);
	for(INT i=0; i<NumChildren; i++)
	{
		if( Children(i).Weight > ZERO_ANIMWEIGHT_THRESH )
		{
			LastChildIndex = i;
			RelevantChildren.AddItem(i);
		}
	}
	
	// If only the Source is relevant, then pass right through
	if( LastChildIndex == 0 )
	{
		if( Children(0).Anim )
		{
			EXCLUDE_CHILD_TIME
			Children(0).Anim->GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
		}
		else
		{
			RootMotionDelta = FBoneAtom::Identity;
			bHasRootMotion	= 0;
			FillWithRefPose(Atoms, DesiredBones, RefSkel);
		}

		// No need to save cached results here, nothing is done.
		return;
	}

	// Now, extract atoms for source animation
	{
		// Get bone atoms from child node (if no child - use ref pose).
		if( Children(0).Anim )
		{
			EXCLUDE_CHILD_TIME
			Children(0).Anim->GetBoneAtoms(Atoms, DesiredBones, Children(0).RootMotion, Children(0).bHasRootMotion);

			// Update our Root Motion flag.
			bHasRootMotion |= Children(0).bHasRootMotion;
		}
		else
		{
			Children(0).RootMotion		= FBoneAtom::Identity;
			Children(0).bHasRootMotion	= 0;
			FillWithRefPose(Atoms, DesiredBones, RefSkel);
		}
	}
	
	// If this local space blend of mesh space blend?
	const UBOOL bDoMeshSpaceTransform = (RotationBlendType == EBT_MeshSpace);
	const INT NumBones = RefSkel.Num();
	const INT NumRelevantChildren = RelevantChildren.Num();

	// Local space blend, multiple masks
	if( !bDoMeshSpaceTransform )
	{
		// Iterate over each relevant child getting its atoms
		for(INT RelevantChildIndex=1; RelevantChildIndex<NumRelevantChildren; RelevantChildIndex++)
		{
			const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
			const INT			MaskIndex	= ChildIndex - 1;
			FAnimBlendChild&	Child		= Children(ChildIndex);
			FPerBoneMaskInfo&	Mask		= MaskList(MaskIndex);

			// Do need to request atoms, so allocate array here.
			if( Mask.MaskAtoms.Num() != NumBones )
			{
				Mask.MaskAtoms.Reset();
				Mask.MaskAtoms.Add(NumBones);
				Mask.MaskAtoms.Shrink();
			}

			// Get bone atoms from child node (if no child - use ref pose).
			if( Child.Anim )
			{
				EXCLUDE_CHILD_TIME
				// @TODO: Avoid extra copy, by removing Mask.MaskAtoms and using a set of FBoneAtomArrays allocated locally
				FBoneAtomArray TempArray;
				TempArray.Add(NumBones);
				Child.Anim->GetBoneAtoms(TempArray, DesiredBones, Child.RootMotion, Child.bHasRootMotion);
				Mask.MaskAtoms = TempArray;
			}
			else
			{
				Child.RootMotion		= FBoneAtom::Identity;
				Child.bHasRootMotion	= 0;
				FillWithRefPose(Mask.MaskAtoms, DesiredBones, RefSkel);
			}
		}

		if( NumRelevantChildren == 2 )
		{
			LocalBlendSingleMask(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, RelevantChildren);
		}
		else
		{
			LocalBlendMultipleMasks(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, RelevantChildren);
		}
	}
	else
	{
		// If transforming to mesh space, allocate when needed
		if( GChild1CompSpace.Num() < NumBones )
		{
			GChild1CompSpace.Reset();
			GChild1CompSpace.Add(NumBones);
		}

		if( GChild2CompSpace.Num() < NumBones )
		{
			GChild2CompSpace.Reset();
			GChild2CompSpace.Add(NumBones);
		}

		// Iterate over each relevant child getting its atoms
		for(INT RelevantChildIndex=1; RelevantChildIndex<NumRelevantChildren; RelevantChildIndex++)
		{
			const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
			const INT			MaskIndex	= ChildIndex - 1;
			FAnimBlendChild&	Child		= Children(ChildIndex);
			FPerBoneMaskInfo&	Mask		= MaskList(MaskIndex);

			// Do need to request atoms, so allocate array here.
			if( Mask.MaskAtoms.Num() != NumBones )
			{
				Mask.MaskAtoms.Reset();
				Mask.MaskAtoms.Add(NumBones);
				Mask.MaskAtoms.Shrink();
			}

			// Transforming to mesh space, allocate when needed
			// Clear index
			Mask.TransformReqBoneIndex = 0;

			// Allocated array if needed
			if( Mask.MeshSpaceTM.Num() != NumBones )
			{
				Mask.MeshSpaceTM.Reset();
				Mask.MeshSpaceTM.Add(NumBones);
				Mask.MeshSpaceTM.Shrink();
			}

			// Get bone atoms from child node (if no child - use ref pose).
			if( Child.Anim )
			{
				EXCLUDE_CHILD_TIME
				// @TODO: Avoid extra copy, by removing Mask.MaskAtoms and using a set of FBoneAtomArrays allocated locally
				FBoneAtomArray TempArray;
				TempArray.Add(NumBones);
				Child.Anim->GetBoneAtoms(TempArray, DesiredBones, Child.RootMotion, Child.bHasRootMotion);
				Mask.MaskAtoms = TempArray;
			}
			else
			{
				Child.RootMotion		= FBoneAtom::Identity;
				Child.bHasRootMotion	= 0;
				FillWithRefPose(Mask.MaskAtoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
			}
		}

		if( NumRelevantChildren == 2 )
		{
			MeshSpaceBlendSingleMask(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, RelevantChildren);
		}
		else
		{
			MeshSpaceBlendMultipleMasks(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, RelevantChildren);
		}
	}

	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}

/** 
 * Special path for local blend w/ 1 mask.
 * We can simplify code a lot in this case. Fastest path.
 */
FORCEINLINE void UGearAnim_BlendPerBone::LocalBlendSingleMask(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren)
{
	const TArray<FMeshBone>& RefSkel = SkelComponent->SkeletalMesh->RefSkeleton;
	const INT NumBones = RefSkel.Num();
	const INT NumRelevantChildren = RelevantChildren.Num();

	// Make sure do indeed have 1 mask + source
	check(NumRelevantChildren == 2);

	const INT			MaskChildIndex	= RelevantChildren(1);
	const INT			MaskIndex		= MaskChildIndex - 1;
	FAnimBlendChild&	MaskChild		= Children(MaskChildIndex);

	// Iterate over each bone, and iterate over each branch to figure out the bone's weight
	// And perform blending.
	const INT NumDesiredBones = DesiredBones.Num();
	for(INT i=0; i<NumDesiredBones; i++)
	{
		const INT	BoneIndex	= DesiredBones(i);
		const UBOOL	bIsRootBone = (BoneIndex == 0);

		// Figure weight for this bone giving priority to the Mask
		const FLOAT	MaskBoneWeight = MaskChild.Weight * MaskList(MaskIndex).PerBoneWeights(BoneIndex);
		
		// If MaskBoneWeight is relevant, then do a blend.
		// Otherwise no need to do anything, Atoms() already has all Source bones.
		if( MaskBoneWeight > ZERO_ANIMWEIGHT_THRESH )
		{
			Atoms(BoneIndex).Blend(Atoms(BoneIndex), MaskList(MaskIndex).MaskAtoms(BoneIndex), MaskBoneWeight);
		}

		// Take care of root motion if we have to.
		if( bIsRootBone && (Children(0).bHasRootMotion || MaskChild.bHasRootMotion) )
		{
			if( Children(0).bHasRootMotion && MaskChild.bHasRootMotion )
			{
				RootMotionDelta.Blend(Children(0).RootMotion, MaskChild.RootMotion, MaskBoneWeight);
			}
			else if( Children(0).bHasRootMotion )
			{
				RootMotionDelta = Children(0).RootMotion;
			}
			else if( MaskChild.bHasRootMotion )
			{
				RootMotionDelta = MaskChild.RootMotion;
			}
		}
	}
}

/** 
 * Special path for local blend w/ multiple masks.
 * As opposed to MeshSpace blends, we can get rid OF a lot OF branches and make that code faster.
 */
FORCEINLINE void UGearAnim_BlendPerBone::LocalBlendMultipleMasks(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren)
{
	const TArray<FMeshBone>& RefSkel = SkelComponent->SkeletalMesh->RefSkeleton;
	const INT NumBones = RefSkel.Num();
	const INT NumRelevantChildren = RelevantChildren.Num();

	bHasRootMotion = 0;
	FLOAT AccumulatedRootMotionWeight = 0.f;

	// Temp Bone Atom used for blending.
	FBoneAtom BlendedBoneAtom;
	BlendedBoneAtom.Rotation = FQuat::Identity;
	BlendedBoneAtom.Translation = FVector(0,0,0);
	BlendedBoneAtom.Scale = 1.f;

	// Iterate over each bone, and iterate over each branch to figure out the bone's weight
	// And perform blending.
	const INT NumDesiredBones = DesiredBones.Num();
	for(INT i=0; i<NumDesiredBones; i++)
	{
		const INT	BoneIndex	= DesiredBones(i);
		const UBOOL	bIsRootBone = (BoneIndex == 0);

		FLOAT	AccumulatedWeight	= 0.f;
		UBOOL	bDidInitAtom		= FALSE;

		// Iterate over each child getting its atoms, scaling them and adding them to output (Atoms array)
		for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>0; RelevantChildIndex--)
		{
			const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
			const INT			MaskIndex	= ChildIndex - 1;
			FAnimBlendChild&	Child		= Children(ChildIndex);
			// Figure weight for this bone giving priority to the highest branch
			const FLOAT			BoneWeight	= (1.f - AccumulatedWeight) * Child.Weight * MaskList(MaskIndex).PerBoneWeights(BoneIndex);
			
			// And accumulate it, as the sum needs to be 1.f
			AccumulatedWeight = Min(AccumulatedWeight + BoneWeight, 1.f);

			// If weight is significant, do the blend...
			if( BoneWeight > ZERO_ANIMWEIGHT_THRESH )
			{
				// If this is the root bone and child has root motion, then accumulate it
				// And set flag saying the animation we'll forward from here will contain root motion.
				if( bIsRootBone && Child.bHasRootMotion )
				{
					bHasRootMotion				= 1;
					AccumulatedRootMotionWeight = Min(AccumulatedRootMotionWeight + BoneWeight, 1.f);
				}

				// If full weight, just do a straight copy, no blending - and no need to go further.
				if( BoneWeight >= (1.f - ZERO_ANIMWEIGHT_THRESH) )
				{
					BlendedBoneAtom = MaskList(MaskIndex).MaskAtoms(BoneIndex);
					break;
				}
				else
				{
					// We just write the first childrens atoms into the output array. Avoids zero-ing it out.
					if( !bDidInitAtom )
					{
						// Parent bone space bone atom weighting
						BlendedBoneAtom	= MaskList(MaskIndex).MaskAtoms(BoneIndex) * BoneWeight;
						bDidInitAtom	= TRUE;
					}
					else
					{
						// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
						if( (BlendedBoneAtom.Rotation | MaskList(MaskIndex).MaskAtoms(BoneIndex).Rotation) < 0.f )
						{
							BlendedBoneAtom.Rotation = BlendedBoneAtom.Rotation * -1.f;
						}

						BlendedBoneAtom += MaskList(MaskIndex).MaskAtoms(BoneIndex) * BoneWeight;
					}
				}
			}
		} //for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>0; RelevantChildIndex--)

		// Source gets remainder
		const FLOAT SourceBoneWeight = 1.f - AccumulatedWeight;

		// If Source is a little relevant, then we need to either copy or blend
		if( SourceBoneWeight > ZERO_ANIMWEIGHT_THRESH )
		{
			// If this is the root bone and child has root motion, then accumulate it
			// And set flag saying the animation we'll forward from here will contain root motion.
			if( bIsRootBone && Children(0).bHasRootMotion )
			{
				bHasRootMotion				= 1;
				AccumulatedRootMotionWeight = Min(AccumulatedRootMotionWeight + SourceBoneWeight, 1.f);
			}

			// We only need to touch up something if we have to blend. If source is full weight, we've already got the animation data in there.
			if( SourceBoneWeight < (1.f - ZERO_ANIMWEIGHT_THRESH) )
			{
				// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
				if( (Atoms(BoneIndex).Rotation | BlendedBoneAtom.Rotation) < 0.f )
				{
					BlendedBoneAtom.Rotation = BlendedBoneAtom.Rotation * -1.f;
				}

				Atoms(BoneIndex) *= SourceBoneWeight;
				Atoms(BoneIndex) += BlendedBoneAtom;

				// Normalize Rotation
				Atoms(BoneIndex).Rotation.Normalize();
			}
		}
		// Source not relevant. Everything comes from the mask(s)
		else
		{
			Atoms(BoneIndex) = BlendedBoneAtom;
		}

		// Do another pass for root motion
		if( bIsRootBone && bHasRootMotion )
		{
			AccumulatedWeight	= 0.f;
			bDidInitAtom		= FALSE;
			UBOOL bDidBlendAtom	= FALSE;

			// Iterate over each child getting its atoms, scaling them and adding them to output (Atoms array)
			for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>=0; RelevantChildIndex--)
			{
				const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
				FAnimBlendChild&	Child		= Children(ChildIndex);

				if( Child.bHasRootMotion )
				{
					const INT MaskIndex = ChildIndex - 1;
					FLOAT BoneWeight;

					if( ChildIndex > 0 )
					{
						// Figure weight for this bone giving priority to the highest branch
						BoneWeight			= (1.f - AccumulatedWeight) * Child.Weight * MaskList(MaskIndex).PerBoneWeights(BoneIndex);
						// And accumulate it, as the sum needs to be 1.f
						AccumulatedWeight	= Min(AccumulatedWeight + BoneWeight, 1.f);
					}
					else
					{
						// Source gets remainder
						BoneWeight = 1.f - AccumulatedWeight;
					}

					FBoneAtom WeightedRootMotion = Child.RootMotion * (BoneWeight / AccumulatedRootMotionWeight);

#if 0 // Debug Root Motion
					if( !WeightedRootMotion.Translation.IsZero() )
					{
						debugf(TEXT("%3.2f [%s]  Adding weighted (%3.2f) root motion trans: %3.2f, vect: %s. ChildWeight: %3.3f"), GWorld->GetTimeSeconds(), SkelComponent->GetOwner()->GetName(), BoneWeight, WeightedRootMotion.Translation.Size(), *WeightedRootMotion.Translation.ToString(), Children(i).Weight);
					}
#endif
					// Accumulate Root Motion
					if( !bDidInitAtom )
					{
						bDidInitAtom	= TRUE;
						RootMotionDelta = WeightedRootMotion;
					}
					else
					{
						// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
						if( (RootMotionDelta.Rotation | WeightedRootMotion.Rotation) < 0.f )
						{
							WeightedRootMotion.Rotation = WeightedRootMotion.Rotation * -1.f;
						}

						RootMotionDelta += WeightedRootMotion;
						bDidBlendAtom	= TRUE;
					}
				} // if( Child.bHasRootMotion )

			} // for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>=0; RelevantChildIndex--)

			// If we did blend root motion, normalize rotation quaternion
			if( bDidBlendAtom )
			{
				RootMotionDelta.Rotation.Normalize();
			}
		}

	}
}

/** 
 * Special path for mesh space blend w/ 1 mask.
 * We can simplify code a lot in this case. Faster path. Less branches and loops.
 */
FORCEINLINE void UGearAnim_BlendPerBone::MeshSpaceBlendSingleMask(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren)
{
	const TArray<FMeshBone>& RefSkel = SkelComponent->SkeletalMesh->RefSkeleton;
	const INT NumBones = RefSkel.Num();
	const INT NumRelevantChildren = RelevantChildren.Num();

	// Make sure do indeed have 1 mask + source
	check(NumRelevantChildren == 2);

	const INT			MaskChildIndex	= RelevantChildren(1);
	const INT			MaskIndex		= MaskChildIndex - 1;
	FPerBoneMaskInfo&	Mask			= MaskList(MaskIndex); 
	FAnimBlendChild&	MaskChild		= Children(MaskChildIndex);
	TArray<FBoneAtom>&	MaskAtoms		= MaskList(MaskIndex).MaskAtoms;
	TArray<FMatrix>&	MaskTM			= MaskList(MaskIndex).MeshSpaceTM;

	const INT			NumMaskTransformReqBones = Mask.TransformReqBone.Num();

	// Iterate over each bone, and iterate over each branch to figure out the bone's weight
	// And perform blending.
	const INT NumDesiredBones = DesiredBones.Num();
	for(INT i=0; i<NumDesiredBones; i++)
	{
		const INT	BoneIndex	= DesiredBones(i);
		const UBOOL	bIsRootBone = (BoneIndex == 0);
		const INT	ParentIndex	= RefSkel(BoneIndex).ParentIndex;

		// Figure out if this bone needs to be transformed to mesh space
		// It's an expensive operation, so prefer parent bone space blending whenever possible
		UBOOL bTransformBone = FALSE;
		if( Mask.TransformReqBoneIndex < NumMaskTransformReqBones && BoneIndex == Mask.TransformReqBone(Mask.TransformReqBoneIndex) )
		{
			Mask.TransformReqBoneIndex++;
			bTransformBone = TRUE;
		}

		// Figure weight for this bone giving priority to the Mask
		const FLOAT	MaskBoneWeight = MaskChild.Weight * MaskList(MaskIndex).PerBoneWeights(BoneIndex);

		// Transform parent bone space BoneAtoms into mesh space matrices
		if( bTransformBone )
		{
			// Transform mesh space rotation FBoneAtom to FMatrix
			MaskTM(BoneIndex) = FQuatRotationTranslationMatrix(MaskAtoms(BoneIndex).Rotation, FVector(0.f));
			GChild1CompSpace(BoneIndex) = FQuatRotationTranslationMatrix(Atoms(BoneIndex).Rotation, FVector(0.f));

			// Figure out if we're going to blend something, faster path otherwise
			// Because we basically need both bones in mesh space to blend them. Otherwise we can just copy the right one.
			const UBOOL	bWeAreBlending = (MaskBoneWeight > ZERO_ANIMWEIGHT_THRESH) && (MaskBoneWeight < 1.f - ZERO_ANIMWEIGHT_THRESH);

			if( !bIsRootBone )
			{
				// Transform to mesh space
				MaskTM(BoneIndex) *= MaskTM(ParentIndex);
				GChild1CompSpace(BoneIndex) *= GChild1CompSpace(ParentIndex);

				if( bWeAreBlending )
				{
					// If transforming to mesh space, then overwrite local rotation with mesh space rotation for weighting
					MaskAtoms(BoneIndex).Rotation = FQuat(MaskTM(BoneIndex));
					Atoms(BoneIndex).Rotation = FQuat(GChild1CompSpace(BoneIndex));

					// Do blend
					Atoms(BoneIndex).Blend(Atoms(BoneIndex), MaskAtoms(BoneIndex), MaskBoneWeight);

					// If blending in mesh space, we now need to turn the rotation back into parent bone space
					// Transform mesh space rotation FBoneAtom to FMatrix
					GChild2CompSpace(BoneIndex)	= FQuatRotationTranslationMatrix(Atoms(BoneIndex).Rotation, FVector(0.f));
				}
				// No need to blend, can do direct copy
				else 
				{
					GChild2CompSpace(BoneIndex)	= (MaskBoneWeight <= ZERO_ANIMWEIGHT_THRESH) ? GChild1CompSpace(BoneIndex) : MaskTM(BoneIndex);
				}

				// Transform mesh space rotation back to parent bone space
				const FMatrix RelativeTM	= GChild2CompSpace(BoneIndex) * GChild2CompSpace(ParentIndex).Inverse();
				Atoms(BoneIndex).Rotation	= FQuat(RelativeTM);
			}
			else
			{
				if( bWeAreBlending )
				{
					// Do blend
					Atoms(BoneIndex).Blend(Atoms(BoneIndex), MaskAtoms(BoneIndex), MaskBoneWeight);

					// If blending in mesh space, we now need to turn the rotation back into parent bone space
					// Transform mesh space rotation FBoneAtom to FMatrix
					GChild2CompSpace(BoneIndex)	= FQuatRotationTranslationMatrix(Atoms(BoneIndex).Rotation, FVector(0.f));
				}
				else 
				{
					// Source
					if( MaskBoneWeight <= ZERO_ANIMWEIGHT_THRESH )
					{
						GChild2CompSpace(BoneIndex)	= GChild1CompSpace(BoneIndex);
					}
					// Mask
					else
					{
						// Direct copy
						Atoms(BoneIndex) = MaskAtoms(BoneIndex);
						GChild2CompSpace(BoneIndex)	= MaskTM(BoneIndex);
					}
				}
			}
		}
		// Just do fast path for local space blend
		else
		{
			// If MaskBoneWeight is relevant, then do a blend.
			// Otherwise no need to do anything, Atoms() already has all Source bones.
			if( MaskBoneWeight > ZERO_ANIMWEIGHT_THRESH )
			{
				Atoms(BoneIndex).Blend(Atoms(BoneIndex), MaskList(MaskIndex).MaskAtoms(BoneIndex), MaskBoneWeight);
			}
		}

		// Take care of root motion if we have to.
		if( bIsRootBone && (Children(0).bHasRootMotion || MaskChild.bHasRootMotion) )
		{
			if( Children(0).bHasRootMotion && MaskChild.bHasRootMotion )
			{
				RootMotionDelta.Blend(Children(0).RootMotion, MaskChild.RootMotion, MaskBoneWeight);
			}
			else if( Children(0).bHasRootMotion )
			{
				RootMotionDelta = Children(0).RootMotion;
			}
			else if( MaskChild.bHasRootMotion )
			{
				RootMotionDelta = MaskChild.RootMotion;
			}
		}
	}
}

/** 
 * Hardcore path. Multiple masks and mesh space transform. This is the slowest path.
 */
FORCEINLINE void UGearAnim_BlendPerBone::MeshSpaceBlendMultipleMasks(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren)
{
	const TArray<FMeshBone>& RefSkel = SkelComponent->SkeletalMesh->RefSkeleton;
	const INT NumBones = RefSkel.Num();
	const INT NumRelevantChildren = RelevantChildren.Num();

	bHasRootMotion = 0;
	FLOAT AccumulatedRootMotionWeight = 0.f;

	// Temp Bone Atom used for blending.
	FBoneAtom BlendedBoneAtom;

	// Iterate over each bone, and iterate over each branch to figure out the bone's weight
	// And perform blending.
	const INT NumDesiredBones = DesiredBones.Num();
	for(INT i=0; i<NumDesiredBones; i++)
	{
		const INT	BoneIndex	= DesiredBones(i);
		const UBOOL	bIsRootBone = (BoneIndex == 0);

		FLOAT	AccumulatedWeight	= 0.f;
		UBOOL	bDidInitAtom		= FALSE;
		UBOOL	bTransformBone		= FALSE;

		// Figure out if this bone needs to be transformed to mesh space
		// It's an expensive operation, so prefer parent bone space blending whenever possible
		for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>0; RelevantChildIndex--)
		{
			const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
			const INT			MaskIndex	= ChildIndex - 1;
			FPerBoneMaskInfo&	Mask		= MaskList(MaskIndex); 

			if( Mask.TransformReqBoneIndex < Mask.TransformReqBone.Num() &&	BoneIndex == Mask.TransformReqBone(Mask.TransformReqBoneIndex) )
			{
				Mask.TransformReqBoneIndex++;
				bTransformBone = TRUE;
			}
		}

		// Iterate over each child getting its atoms, scaling them and adding them to output (Atoms array)
		for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>0; RelevantChildIndex--)
		{
			const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
			const INT			MaskIndex	= ChildIndex - 1;
			FAnimBlendChild&	Child		= Children(ChildIndex);
			// Figure weight for this bone giving priority to the highest branch
			const FLOAT			BoneWeight	= (1.f - AccumulatedWeight) * Child.Weight * MaskList(MaskIndex).PerBoneWeights(BoneIndex);
			
			// And accumulate it, as the sum needs to be 1.f
			AccumulatedWeight = Min(AccumulatedWeight + BoneWeight, 1.f);

			// Transform parent bone space BoneAtoms into mesh space matrices
			if( bTransformBone )
			{
				// Get current child's mesh space transform matrix
				TArray<FMatrix>& TM = MaskList(MaskIndex).MeshSpaceTM;

				// Transform mesh space rotation FBoneAtom to FMatrix
				TM(BoneIndex) = FQuatRotationTranslationMatrix(MaskList(MaskIndex).MaskAtoms(BoneIndex).Rotation, FVector(0.f));

				// Transform to mesh space
				if( !bIsRootBone )
				{
					TM(BoneIndex) *= TM(RefSkel(BoneIndex).ParentIndex);
				}
			}

			// If weight is significant, do the blend...
			if( BoneWeight > ZERO_ANIMWEIGHT_THRESH )
			{
				// If transforming to mesh space, then overwrite local rotation with mesh space rotation for weighting
				// No need to transform rotation for root bone, as it has no parent bone.
				if( bTransformBone && !bIsRootBone )
				{
					// Turn transform matrix rotation into quaternion.
					MaskList(MaskIndex).MaskAtoms(BoneIndex).Rotation = FQuat(MaskList(MaskIndex).MeshSpaceTM(BoneIndex));
				}

				// If this is the root bone and child has root motion, then accumulate it
				// And set flag saying the animation we'll forward from here will contain root motion.
				if( bIsRootBone && Child.bHasRootMotion )
				{
					bHasRootMotion				= 1;
					AccumulatedRootMotionWeight = Min(AccumulatedRootMotionWeight + BoneWeight, 1.f);
				}

				// If full weight, just do a straight copy, no blending - and no need to go further.
				if( BoneWeight >= (1.f - ZERO_ANIMWEIGHT_THRESH) )
				{
					BlendedBoneAtom = MaskList(MaskIndex).MaskAtoms(BoneIndex);
					break;
				}
				else
				{
					// We just write the first childrens atoms into the output array. Avoids zero-ing it out.
					if( !bDidInitAtom )
					{
						// Parent bone space bone atom weighting
						BlendedBoneAtom	= MaskList(MaskIndex).MaskAtoms(BoneIndex) * BoneWeight;
						bDidInitAtom	= TRUE;
					}
					else
					{
						// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
						if( (BlendedBoneAtom.Rotation | MaskList(MaskIndex).MaskAtoms(BoneIndex).Rotation) < 0.f )
						{
							BlendedBoneAtom.Rotation = BlendedBoneAtom.Rotation * -1.f;
						}

						BlendedBoneAtom += MaskList(MaskIndex).MaskAtoms(BoneIndex) * BoneWeight;
					}
				}
			}
		} //for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>0; RelevantChildIndex--)

		// Transform parent bone space BoneAtoms into mesh space matrices
		if( bTransformBone )
		{
			// Transform mesh space rotation FBoneAtom to FMatrix
			GChild1CompSpace(BoneIndex) = FQuatRotationTranslationMatrix(Atoms(BoneIndex).Rotation, FVector(0.f));

			// Transform to mesh space
			if( !bIsRootBone )
			{
				GChild1CompSpace(BoneIndex) *= GChild1CompSpace(RefSkel(BoneIndex).ParentIndex);
			}
		}

		// Source gets remainder
		const FLOAT SourceBoneWeight = 1.f - AccumulatedWeight;

		// If Source is a little relevant, then we need to either copy or blend
		if( SourceBoneWeight > ZERO_ANIMWEIGHT_THRESH )
		{
			// If this is the root bone and child has root motion, then accumulate it
			// And set flag saying the animation we'll forward from here will contain root motion.
			if( bIsRootBone && Children(0).bHasRootMotion )
			{
				bHasRootMotion				= 1;
				AccumulatedRootMotionWeight = Min(AccumulatedRootMotionWeight + SourceBoneWeight, 1.f);
			}

			// If transforming to mesh space, then overwrite local rotation with mesh space rotation for weighting
			// No need to transform rotation for root bone, as it has no parent bone.
			if( bTransformBone && !bIsRootBone )
			{
				// Turn transform matrix rotation into quaternion.
				Atoms(BoneIndex).Rotation = FQuat(GChild1CompSpace(BoneIndex));
			}

			// We only need to touch up something if we have to blend. If source is full weight, we've already got the animation data in there.
			if( SourceBoneWeight < (1.f - ZERO_ANIMWEIGHT_THRESH) )
			{
				// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
				if( (Atoms(BoneIndex).Rotation | BlendedBoneAtom.Rotation) < 0.f )
				{
					BlendedBoneAtom.Rotation = BlendedBoneAtom.Rotation * -1.f;
				}

				Atoms(BoneIndex) *= SourceBoneWeight;
				Atoms(BoneIndex) += BlendedBoneAtom;

				// Normalize Rotation
				Atoms(BoneIndex).Rotation.Normalize();
			}
		}
		// Source not relevant. Everything comes from the mask(s)
		else
		{
			Atoms(BoneIndex) = BlendedBoneAtom;
		}

		// If blending in mesh space, we now need to turn the rotation back into parent bone space
		if( bTransformBone )
		{
			// Transform mesh space rotation FBoneAtom to FMatrix
			GChild2CompSpace(BoneIndex)	= FQuatRotationTranslationMatrix(Atoms(BoneIndex).Rotation, FVector(0.f));

			// Transform mesh space rotation back to parent bone space
			if( !bIsRootBone )
			{
				const FMatrix RelativeTM	= GChild2CompSpace(BoneIndex) * GChild2CompSpace(RefSkel(BoneIndex).ParentIndex).Inverse();
				Atoms(BoneIndex).Rotation	= FQuat(RelativeTM);
			}
		}

		// Do another pass for root motion
		if( bIsRootBone && bHasRootMotion )
		{
			AccumulatedWeight	= 0.f;
			bDidInitAtom		= FALSE;
			UBOOL bDidBlendAtom	= FALSE;

			// Iterate over each child getting its atoms, scaling them and adding them to output (Atoms array)
			for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>=0; RelevantChildIndex--)
			{
				const INT			ChildIndex	= RelevantChildren(RelevantChildIndex);
				FAnimBlendChild&	Child		= Children(ChildIndex);

				if( Child.bHasRootMotion )
				{
					const INT MaskIndex = ChildIndex - 1;
					FLOAT BoneWeight;

					if( ChildIndex > 0 )
					{
						// Figure weight for this bone giving priority to the highest branch
						BoneWeight			= (1.f - AccumulatedWeight) * Child.Weight * MaskList(MaskIndex).PerBoneWeights(BoneIndex);
						// And accumulate it, as the sum needs to be 1.f
						AccumulatedWeight	= Min(AccumulatedWeight + BoneWeight, 1.f);
					}
					else
					{
						// Source gets remainder
						BoneWeight = 1.f - AccumulatedWeight;
					}

					FBoneAtom WeightedRootMotion = Child.RootMotion * (BoneWeight / AccumulatedRootMotionWeight);

#if 0 // Debug Root Motion
					if( !WeightedRootMotion.Translation.IsZero() )
					{
						debugf(TEXT("%3.2f [%s]  Adding weighted (%3.2f) root motion trans: %3.2f, vect: %s. ChildWeight: %3.3f"), GWorld->GetTimeSeconds(), SkelComponent->GetOwner()->GetName(), BoneWeight, WeightedRootMotion.Translation.Size(), *WeightedRootMotion.Translation.ToString(), Children(i).Weight);
					}
#endif
					// Accumulate Root Motion
					if( !bDidInitAtom )
					{
						bDidInitAtom	= TRUE;
						RootMotionDelta = WeightedRootMotion;
					}
					else
					{
						// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
						if( (RootMotionDelta.Rotation | WeightedRootMotion.Rotation) < 0.f )
						{
							WeightedRootMotion.Rotation = WeightedRootMotion.Rotation * -1.f;
						}

						RootMotionDelta += WeightedRootMotion;
						bDidBlendAtom	= TRUE;
					}
				} // if( Child.bHasRootMotion )

			} // for(INT RelevantChildIndex=NumRelevantChildren-1; RelevantChildIndex>=0; RelevantChildIndex--)

			// If we did blend root motion, normalize rotation quaternion
			if( bDidBlendAtom )
			{
				RootMotionDelta.Rotation.Normalize();
			}
		}

	}
}


/************************************************************************************
 * UGearAnimNotify_ToggleSkelControl
 ***********************************************************************************/

void UGearAnimNotify_ToggleSkelControl::Notify(UAnimNodeSequence* NodeSeq)
{
	// Some requirements need to be met!
	if( !NodeSeq || !NodeSeq->SkelComponent || !NodeSeq->AnimSeq )
	{
		return;
	}

	for(INT i=0; i<Controls.Num(); i++)
	{
		FCtrlInfo& Control = Controls(i);

		// Check if this Control requires to be called by a specific AnimNodeSequence
		// named 'SeqNodeName'.
		if( Control.SeqNodeName != NAME_None && NodeSeq->NodeName != Control.SeqNodeName )
		{
			continue;
		}

		// Find Bone Controller
		USkelControlBase* BoneController = NodeSeq->SkelComponent->FindSkelControl(Control.ControlName);

		if( BoneController )
		{
			// Animation triggering the notify play rate
			const FLOAT AnimPlayRate = NodeSeq->Rate * NodeSeq->AnimSeq->RateScale;

			if( bTurnOn )
			{
				BoneController->BlendInTime	= AnimPlayRate > 0.f ? BlendTime / AnimPlayRate : BlendTime;
				BoneController->SetSkelControlActive(TRUE);
			}
			else
			{
				BoneController->BlendOutTime	= AnimPlayRate > 0.f ? BlendTime / AnimPlayRate : BlendTime;
				BoneController->SetSkelControlActive(FALSE);
			}
		}
	}
}

/************************************************************************************
 * UGearSkelCtrl_CorpserIK
 ***********************************************************************************/


void UGearSkelCtrl_CorpserIK::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);

	// Get indices of the lower and upper limb bones.
	UBOOL bInvalidLimb = FALSE;
	if( BoneIndex == 0 )
	{
		bInvalidLimb = TRUE;
	}

	const INT LowerLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
	if( LowerLimbIndex == 0 )
	{
		bInvalidLimb = TRUE;
	}

	const INT UpperLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(LowerLimbIndex).ParentIndex;
	if( UpperLimbIndex == 0 )
	{
		bInvalidLimb = TRUE;
	}

	const INT ShoulderLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(UpperLimbIndex).ParentIndex;

	// If we walked past the root, this controlled is invalid, so return no affected bones.
	if( bInvalidLimb )
	{
		debugf( TEXT("USkelControlLimb : Cannot find 3 bones above controlled bone. Too close to root.") );
		return;
	}
	else
	{
		OutBoneIndices.Add(4);
		OutBoneIndices(0) = ShoulderLimbIndex;
		OutBoneIndices(1) = UpperLimbIndex;
		OutBoneIndices(2) = LowerLimbIndex;
		OutBoneIndices(3) = BoneIndex;
	}
}


void UGearSkelCtrl_CorpserIK::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);
	OutBoneTransforms.Add(4); // Allocate space for bone transforms.

	// First get indices of the lower and upper limb bones. We should have checked this in GetAffectedBones so can assume its ok now.
	const INT LowerLimbIndex	= SkelComp->SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
	const INT UpperLimbIndex	= SkelComp->SkeletalMesh->RefSkeleton(LowerLimbIndex).ParentIndex;
	const INT ShoulderLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(UpperLimbIndex).ParentIndex;

	OutBoneTransforms(0) = SkelComp->SpaceBases(ShoulderLimbIndex);
	OutBoneTransforms(1) = SkelComp->SpaceBases(UpperLimbIndex);
	OutBoneTransforms(2) = SkelComp->SpaceBases(LowerLimbIndex);
	OutBoneTransforms(3) = SkelComp->SpaceBases(BoneIndex);

	// If we have enough bones to work on (ie at least 3 bones between controlled hand bone and the root) continue.

	// Get desired position of effector.
	FMatrix			ComponentToFrame	= SkelComp->CalcComponentToFrameMatrix(BoneIndex, EffectorLocationSpace, EffectorSpaceBoneName);
	FMatrix			FrameToComponent	= ComponentToFrame.InverseSafe();
	const FVector	DesiredPos			= FrameToComponent.TransformFVector(EffectorLocation);

	// Get joint target (used for defining plane that joint should be in).
	ComponentToFrame					= SkelComp->CalcComponentToFrameMatrix(BoneIndex, JointTargetLocationSpace, JointTargetSpaceBoneName);
	const FVector	JointTargetPos		= FrameToComponent.TransformFVector(JointTargetLocation);

	//
	// Hack for 3rd 'shoulder' bone IK
	//

	// First handle the shoulder limb.
	// This bone points towards the Effector
	
	// Current position of Shoulder
	const FVector	ShoulderPos				= OutBoneTransforms(0).GetOrigin();
	// Current direction of shoulder bone

	const FVector	ShoulderDelta			= OutBoneTransforms(1).GetOrigin() - ShoulderPos;
	FLOAT			ShoulderLength			= ShoulderDelta.Size();
	const FVector	ShoulderDir				= ShoulderLength < (FLOAT)KINDA_SMALL_NUMBER ? FVector(1,0,0) : ShoulderDelta / ShoulderLength;

	// Get direction from shoulder to effector location
	const FVector	ShoulderEffectorDelta	= DesiredPos - ShoulderPos;
	FLOAT			ShoulderEffectorLength	= ShoulderEffectorDelta.Size();
	FVector			ShoulderEffectorDir		= ShoulderEffectorLength < (FLOAT)KINDA_SMALL_NUMBER ? FVector(1,0,0) : ShoulderEffectorDelta / ShoulderEffectorLength;

	// Find out how much we interp to extreme position.
	const FLOAT InterpFactor = Clamp<FLOAT>((ShoulderEffectorLength - InterpDistRange.X) / (InterpDistRange.Y - InterpDistRange.X), 0.f, 1.f);

	// Axis filtering in World Space.
	FVector WorldEffectorDir	= SkelComp->LocalToWorld.TransformFVector( ShoulderEffectorDir );
	FVector WorldShoulderDir	= SkelComp->LocalToWorld.TransformFVector( ShoulderDir );
	FVector ShoulderDesiredDir	= WorldEffectorDir;
	ShoulderDesiredDir.Z		= Lerp(WorldShoulderDir.Z, WorldEffectorDir.Z, InterpFactor * InterpFactor);
	ShoulderDesiredDir			= SkelComp->LocalToWorld.InverseTransformFVector(ShoulderDesiredDir).SafeNormal();

	const FLOAT ShoulderStretch = InterpFactor * ShoulderLengthStretchAllowance;
	if( ShoulderStretch > 0.f )
	{
		ShoulderLength *= (1.f + InterpFactor * ShoulderLengthStretchAllowance);
	}

	const FVector	ShoulderJointTargetDelta	= JointTargetPos - ShoulderPos;
	const FLOAT		ShoulderJointTargetLength	= ShoulderJointTargetDelta.Size();
	FVector			ShoulderJointPlaneNormal, ShoulderJointBendDir;

	// Same check as above, to cover case when JointTarget position is the same as RootPos.
	if( ShoulderJointTargetLength < (FLOAT)KINDA_SMALL_NUMBER )
	{
		ShoulderJointPlaneNormal = FVector(0,0,1);
	}
	else
	{
		ShoulderJointPlaneNormal = ShoulderDesiredDir ^ ShoulderJointTargetDelta;

		// If we are trying to point the limb in the same direction that we are supposed to displace the joint in, 
		// we have to just pick 2 random vector perp to DesiredDir and each other.
		if( ShoulderJointPlaneNormal.Size() < (FLOAT)KINDA_SMALL_NUMBER )
		{
			ShoulderDesiredDir.FindBestAxisVectors(ShoulderJointPlaneNormal, ShoulderJointBendDir);
		}
		else
		{
			ShoulderJointPlaneNormal.Normalize();
		}
	}

	{
		// Update ShoulderLimb
		FMatrix NewTM = BuildMatrixFromVectors(BoneAxis, (bInvertBoneAxis ? -1.f * ShoulderDesiredDir : ShoulderDesiredDir), 
												JointAxis, (bInvertJointAxis ? -1.f * ShoulderJointPlaneNormal : ShoulderJointPlaneNormal) );
		NewTM.SetOrigin(ShoulderPos);
		NewTM.RemoveScaling();
		OutBoneTransforms(0) = NewTM;
	}

	{	// Update UpperLimb
		FMatrix LocalBoneTM;
		SkelComp->LocalAtoms(UpperLimbIndex).ToTransform(LocalBoneTM);
		OutBoneTransforms(1) = LocalBoneTM * OutBoneTransforms(0);
		OutBoneTransforms(1).SetOrigin(ShoulderPos + ShoulderDesiredDir * ShoulderLength);
		OutBoneTransforms(1).RemoveScaling();
	}

	{	// Update LowerLimb
		FMatrix LocalBoneTM;
		SkelComp->LocalAtoms(LowerLimbIndex).ToTransform(LocalBoneTM);
		OutBoneTransforms(2) = LocalBoneTM * OutBoneTransforms(1);
	}

	{	// Update BoneIndex
		FMatrix LocalBoneTM;
		SkelComp->LocalAtoms(BoneIndex).ToTransform(LocalBoneTM);
		OutBoneTransforms(3) = LocalBoneTM * OutBoneTransforms(2);
	}

	//
	// 2 Bone IK --  Same as USkelControlLimb
	//

	// Get current position of root of limb.
	// All position are in Component space.
	const FVector RootPos			= OutBoneTransforms(1).GetOrigin();
	const FVector InitialJointPos	= OutBoneTransforms(2).GetOrigin();
	const FVector InitialEndPos		= OutBoneTransforms(3).GetOrigin();

	// If desired, calc the initial relative transform between the end effector bone and its parent.
	FMatrix EffectorRelTM = FMatrix::Identity;
	if( bMaintainEffectorRelRot ) 
	{
		EffectorRelTM = OutBoneTransforms(3) * OutBoneTransforms(2).InverseSafe();
	}

	const FVector	DesiredDelta		= DesiredPos - RootPos;
	FLOAT			DesiredLength		= DesiredDelta.Size();
	FVector			DesiredDir;

	// Check to handle case where DesiredPos is the same as RootPos.
	if( DesiredLength < (FLOAT)KINDA_SMALL_NUMBER )
	{
		DesiredLength	= (FLOAT)KINDA_SMALL_NUMBER;
		DesiredDir		= FVector(1,0,0);
	}
	else
	{
		DesiredDir		= DesiredDelta/DesiredLength;
	}

	const FVector	JointTargetDelta	= JointTargetPos - ShoulderPos;
	const FLOAT		JointTargetLength	= JointTargetDelta.Size();
	FVector JointPlaneNormal, JointBendDir;

	// Same check as above, to cover case when JointTarget position is the same as RootPos.
	if( JointTargetLength < (FLOAT)KINDA_SMALL_NUMBER )
	{
		JointBendDir		= FVector(0,1,0);
		JointPlaneNormal	= FVector(0,0,1);
	}
	else
	{
		JointPlaneNormal = DesiredDir ^ JointTargetDelta;

		// If we are trying to point the limb in the same direction that we are supposed to displace the joint in, 
		// we have to just pick 2 random vector perp to DesiredDir and each other.
		if( JointPlaneNormal.Size() < (FLOAT)KINDA_SMALL_NUMBER )
		{
			DesiredDir.FindBestAxisVectors(JointPlaneNormal, JointBendDir);
		}
		else
		{
			JointPlaneNormal.Normalize();

			// Find the final member of the reference frame by removing any component of JointTargetDelta along DesiredDir.
			// This should never leave a zero vector, because we've checked DesiredDir and JointTargetDelta are not parallel.
			JointBendDir = JointTargetDelta - ((JointTargetDelta | DesiredDir) * DesiredDir);
			JointBendDir.Normalize();
		}
	}

	// Find lengths of upper and lower limb in the ref skeleton.
	const FLOAT LowerLimbLength = SkelComp->SkeletalMesh->RefSkeleton(BoneIndex).BonePos.Position.Size();
	const FLOAT UpperLimbLength = SkelComp->SkeletalMesh->RefSkeleton(LowerLimbIndex).BonePos.Position.Size();
	const FLOAT MaxLimbLength	= LowerLimbLength + UpperLimbLength;

	FVector OutEndPos	= DesiredPos;
	FVector OutJointPos = InitialJointPos;

	// If we are trying to reach a goal beyond the length of the limb, clamp it to something solvable and extend limb fully.
	if( DesiredLength > MaxLimbLength )
	{
		OutEndPos	= RootPos + (MaxLimbLength * DesiredDir);
		OutJointPos = RootPos + (UpperLimbLength * DesiredDir);
	}
	else
	{
		// So we have a triangle we know the side lengths of. We can work out the angle between DesiredDir and the direction of the upper limb
		// using the sin rule:
		const FLOAT CosAngle = ((UpperLimbLength*UpperLimbLength) + (DesiredLength*DesiredLength) - (LowerLimbLength*LowerLimbLength))/(2 * UpperLimbLength * DesiredLength);

		// If CosAngle is less than 0, the upper arm actually points the opposite way to DesiredDir, so we handle that.
		const UBOOL bReverseUpperBone = (CosAngle < 0.f);

		// If CosAngle is greater than 1.f, the triangle could not be made - we cannot reach the target.
		// We just have the two limbs double back on themselves, and EndPos will not equal the desired EffectorLocation.
		if( CosAngle > 1.f || CosAngle < -1.f )
		{
			// Because we want the effector to be a positive distance down DesiredDir, we go back by the smaller section.
			if( UpperLimbLength > LowerLimbLength )
			{
				OutJointPos = RootPos + (UpperLimbLength * DesiredDir);
				OutEndPos	= OutJointPos - (LowerLimbLength * DesiredDir);
			}
			else
			{
				OutJointPos = RootPos - (UpperLimbLength * DesiredDir);
				OutEndPos	= OutJointPos + (LowerLimbLength * DesiredDir);
			}
		}
		else
		{
			// Angle between upper limb and DesiredDir
			const FLOAT Angle = appAcos(CosAngle);

			// Now we calculate the distance of the joint from the root -> effector line.
			// This forms a right-angle triangle, with the upper limb as the hypotenuse.
			const FLOAT JointLineDist = UpperLimbLength * appSin(Angle);

			// And the final side of that triangle - distance along DesiredDir of perpendicular.
			// ProjJointDistSqr can't be neg, because JointLineDist must be <= UpperLimbLength because appSin(Angle) is <= 1.
			const FLOAT ProjJointDistSqr	= (UpperLimbLength*UpperLimbLength) - (JointLineDist*JointLineDist);
			FLOAT		ProjJointDist		= appSqrt(ProjJointDistSqr);
			if( bReverseUpperBone )
			{
				ProjJointDist *= -1.f;
			}

			// So now we can work out where to put the joint!
			OutJointPos = RootPos + (ProjJointDist * DesiredDir) + (JointLineDist * JointBendDir);
		}
	}

	// Update transform for upper bone.
	FVector GraphicJointDir = JointPlaneNormal;
	if( bInvertJointAxis )
	{
		GraphicJointDir *= -1.f;
	}

	FVector UpperLimbDir = (OutJointPos - RootPos).SafeNormal();
	if( bInvertBoneAxis )
	{
		UpperLimbDir *= -1.f;
	}

	// Do some sanity checking, then use vectors to build upper limb matrix
	if((UpperLimbDir != GraphicJointDir) && (UpperLimbDir | GraphicJointDir) < 0.1f)
	{
		FMatrix UpperLimbTM = BuildMatrixFromVectors(BoneAxis, UpperLimbDir, JointAxis, GraphicJointDir);
		UpperLimbTM.SetOrigin(RootPos);
		OutBoneTransforms(1) = UpperLimbTM;
	}

	// Update transform for lower bone.
	FVector LowerLimbDir = (OutEndPos - OutJointPos).SafeNormal();
	if( bInvertBoneAxis )
	{
		LowerLimbDir *= -1.f;
	}

	// Do some sanity checking, then use vectors to build lower limb matrix
	if((LowerLimbDir != GraphicJointDir) && (LowerLimbDir | GraphicJointDir) < 0.1f)
	{
		FMatrix LowerLimbTM = BuildMatrixFromVectors(BoneAxis, LowerLimbDir, JointAxis, GraphicJointDir);
		LowerLimbTM.SetOrigin(OutJointPos);
		OutBoneTransforms(2) = LowerLimbTM;
	}

	// Update transform for end bone.
	if( bMaintainEffectorRelRot )
	{
		OutBoneTransforms(3) = EffectorRelTM * OutBoneTransforms(2);
	}

	OutBoneTransforms(3).SetOrigin(OutEndPos);
}


void UGearSkelCtrl_CorpserIK::DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex)
{
	Super::DrawSkelControl3D(View, PDI, SkelComp, BoneIndex);

	if( !SkelComp || !SkelComp->SkeletalMesh || ShoulderLengthStretchAllowance <= 0.f || InterpDistRange.Y <= 0.f )
	{
		return;
	}

	// Get Index of the Shoulder Bone
	UBOOL bInvalidLimb = FALSE;
	if( BoneIndex == 0 )
	{
		return;
	}
	
	const INT LowerLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
	if( LowerLimbIndex == 0 )
	{
		return;
	}

	const INT UpperLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(LowerLimbIndex).ParentIndex;
	if( UpperLimbIndex == 0 )
	{
		return;
	}

	const INT ShoulderLimbIndex = SkelComp->SkeletalMesh->RefSkeleton(UpperLimbIndex).ParentIndex;

	const FVector ShoulderBoneLoc = SkelComp->LocalToWorld.TransformFVector( SkelComp->SpaceBases(ShoulderLimbIndex).GetOrigin() );
	DrawCircle(PDI, ShoulderBoneLoc, FVector(1,0,0), FVector(0,1,0), FColor(0,255,0), InterpDistRange.X, 32, SDPG_Foreground);
	DrawCircle(PDI, ShoulderBoneLoc, FVector(1,0,0), FVector(0,1,0), FColor(255,0,0), InterpDistRange.Y, 32, SDPG_Foreground);
}


/************************************************************************************
 * UGearMorph_WeightByBoneAngle
 ***********************************************************************************/

/** Utility function to get an axis from a given matrix, using an EAxis enum */
static FVector GetMatrixAxis(FMatrix& Matrix, BYTE Axis)
{
	if( Axis == AXIS_X )
	{
		return Matrix.GetAxis(0);
	}
	else if( Axis == AXIS_Y )
	{
		return Matrix.GetAxis(1);
	}

	return Matrix.GetAxis(2);
}

/** Get bone's axis vector using existing SpaceBases array. */
static FVector GetBoneAxis(USkeletalMeshComponent* SkelComponent, const INT BoneIndex, const BYTE Axis, const UBOOL bInvert)
{
	INT MatrixAxis;
	
	// Convert Axis enum to matrix row.
	if( Axis == AXIS_X )
	{
		MatrixAxis = 0;
	}
	else if( Axis == AXIS_Y )
	{
		MatrixAxis = 1;
	}
	else
	{
		MatrixAxis = 2;
	}

	// Should we invert?
	if( bInvert )
	{
		return (SkelComponent->GetBoneMatrix(BoneIndex).GetAxis(MatrixAxis).SafeNormal() * -1.f);
	}

	return SkelComponent->GetBoneMatrix(BoneIndex).GetAxis(MatrixAxis).SafeNormal();
}

/** 
 * Updates the node's weight based on the angle between the given 2 bones.
 * It then scales its children morph targets by this weight and returns them.
 */
void UGearMorph_WeightByBoneAngle::GetActiveMorphs(TArray<FActiveMorph>& OutMorphs)
{
	if( !SkelComponent )
	{
		return;
	}

	// Make sure we have valid bone names
	const INT BaseBoneIndex		= SkelComponent->MatchRefBone(BaseBoneName);
	const INT AngleBoneIndex	= SkelComponent->MatchRefBone(AngleBoneName);

	if( BaseBoneIndex == INDEX_NONE || AngleBoneIndex == INDEX_NONE || 
		SkelComponent->SpaceBases.Num() <= BaseBoneIndex ||
		SkelComponent->SpaceBases.Num() <= AngleBoneIndex )
	{
		return;
	}

	// Figure out node's weight based on angle between 2 bones.
	const FVector BaseBoneDir	= GetBoneAxis(SkelComponent, BaseBoneIndex, BaseBoneAxis, bInvertBaseBoneAxis);
	const FVector AngleBoneDir	= GetBoneAxis(SkelComponent, AngleBoneIndex, AngleBoneAxis, bInvertAngleBoneAxis);

	// Figure out angle in degrees between the 2 bones.
	const FLOAT DotProduct	= Clamp<FLOAT>(AngleBoneDir | BaseBoneDir, -1.f, 1.f);
	const FLOAT RadAngle	= appAcos(DotProduct);
	Angle = RadAngle * 180.f / PI;

	// Figure out where we are in the Angle to Weight array.
	INT ArrayIndex = 0;
	const INT WeightArraySize = WeightArray.Num();
	while( ArrayIndex < WeightArraySize && WeightArray(ArrayIndex).Angle < Angle )
	{
		ArrayIndex++;
	}

	// Handle going beyond array size, or if array is empty.
	if( ArrayIndex >= WeightArraySize )
	{
		NodeWeight = (WeightArraySize > 0) ? WeightArray(WeightArraySize-1).TargetWeight : 0.f;
	}
	// If we're in between 2 valid key angles, then perform linear interpolation in between these.
	else if( ArrayIndex > 0 && 
		WeightArray(ArrayIndex).Angle > Angle && 
		WeightArray(ArrayIndex).Angle > WeightArray(ArrayIndex-1).Angle )
	{
		const FLOAT Alpha = (Angle - WeightArray(ArrayIndex-1).Angle) / (WeightArray(ArrayIndex).Angle - WeightArray(ArrayIndex-1).Angle);
		NodeWeight = Lerp(WeightArray(ArrayIndex-1).TargetWeight, WeightArray(ArrayIndex).TargetWeight, Alpha);
	}
	// Otherwise, just return the current angle's corresponding weight
	else
	{
		NodeWeight = WeightArray(ArrayIndex).TargetWeight;
	}

	// Support for Material Parameters
	if( bControlMaterialParameter )
	{
		UMaterialInterface* MaterialInterface = SkelComponent->GetMaterial(MaterialSlotId);

		// See if we need to update the MaterialInstanceConstant reference
		if( MaterialInterface != MaterialInstanceConstant )
		{
			MaterialInstanceConstant = NULL;
			if( MaterialInterface && MaterialInterface->IsA(UMaterialInstanceConstant::StaticClass()) )
			{
				MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInterface);
			}

			if( !MaterialInstanceConstant && SkelComponent->SkeletalMesh )
			{
				if( MaterialSlotId < SkelComponent->SkeletalMesh->Materials.Num() && SkelComponent->SkeletalMesh->Materials(MaterialSlotId) )
				{
					if( SkelComponent->bDisableFaceFXMaterialInstanceCreation )
					{
						debugf(TEXT("UGearMorph_WeightByBoneAngle: WARNING Unable to create MaterialInstanceConstant because bDisableFaceFXMaterialInstanceCreation is true!"));
					}
					else
					{
						UMaterialInstanceConstant* NewMaterialInstanceConstant = CastChecked<UMaterialInstanceConstant>( UObject::StaticConstructObject(UMaterialInstanceConstant::StaticClass(), SkelComponent) );
						NewMaterialInstanceConstant->SetParent(SkelComponent->SkeletalMesh->Materials(MaterialSlotId));
						INT NumMaterials = SkelComponent->Materials.Num();
						if( NumMaterials <= MaterialSlotId )
						{
							SkelComponent->Materials.AddZeroed(MaterialSlotId + 1 - NumMaterials);
						}
						SkelComponent->Materials(MaterialSlotId) = NewMaterialInstanceConstant;
						MaterialInstanceConstant = NewMaterialInstanceConstant;
					}
				}
			}
		}

		// Set Scalar parameter value
		if( MaterialInstanceConstant )
		{
			MaterialInstanceConstant->SetScalarParameterValue(ScalarParameterName, NodeWeight);
		}
	}

	// If node weight is irrelevant, do nothing.
	if( NodeWeight < ZERO_ANIMWEIGHT_THRESH )
	{
		return;
	}

	// This node should only have one connector.
	check(NodeConns.Num() == 1);
	FMorphNodeConn& Conn = NodeConns(0);

	// Temp storage.
	TArray<FActiveMorph> TempMorphs;

	// Iterate over each link from this connector.
	for(INT j=0; j<Conn.ChildNodes.Num(); j++)
	{
		// If there is a child node, call GetActiveMorphs on it.
		if( Conn.ChildNodes(j) )
		{
			TempMorphs.Empty();
			Conn.ChildNodes(j)->GetActiveMorphs(TempMorphs);

			// Iterate over each active morph, scaling it by this node's weight, and adding to output array.
			for(INT k=0; k<TempMorphs.Num(); k++)
			{
				OutMorphs.AddItem( FActiveMorph(TempMorphs(k).Target, TempMorphs(k).Weight * NodeWeight) );
			}
		}
	}
}

/** Draw bone axices on viewport when node is selected */
void UGearMorph_WeightByBoneAngle::Render(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if( !SkelComponent || !SkelComponent->SkeletalMesh )
	{
		return;
	}

	// Make sure we have valid bone names
	const INT BaseBoneIndex		= SkelComponent->MatchRefBone(BaseBoneName);
	const INT AngleBoneIndex	= SkelComponent->MatchRefBone(AngleBoneName);

	if( BaseBoneIndex == INDEX_NONE || AngleBoneIndex == INDEX_NONE || 
		SkelComponent->SpaceBases.Num() <= BaseBoneIndex ||
		SkelComponent->SpaceBases.Num() <= AngleBoneIndex )
	{
		return;
	}

	FStaticLODModel& LODModel = SkelComponent->SkeletalMesh->LODModels(SkelComponent->PredictedLODLevel);
	for(INT i=0; i<LODModel.RequiredBones.Num(); i++)
	{
		const INT BoneIndex = LODModel.RequiredBones(i);

		if( BoneIndex == BaseBoneIndex || BoneIndex == AngleBoneIndex )
		{
			const FVector	LocalBonePos	= SkelComponent->SpaceBases(BoneIndex).GetOrigin();
			const FVector	BonePos			= SkelComponent->LocalToWorld.TransformFVector(LocalBonePos);

			// Draw coord system at each bone
			PDI->DrawLine( BonePos, SkelComponent->LocalToWorld.TransformFVector( LocalBonePos + 3.75f * SkelComponent->SpaceBases(BoneIndex).GetAxis(0) ), FColor(255,0,0), SDPG_Foreground );
			PDI->DrawLine( BonePos, SkelComponent->LocalToWorld.TransformFVector( LocalBonePos + 3.75f * SkelComponent->SpaceBases(BoneIndex).GetAxis(1) ), FColor(0,255,0), SDPG_Foreground );
			PDI->DrawLine( BonePos, SkelComponent->LocalToWorld.TransformFVector( LocalBonePos + 3.75f * SkelComponent->SpaceBases(BoneIndex).GetAxis(2) ), FColor(0,0,255), SDPG_Foreground );

			// Draw axis considered for angle
			if( BoneIndex == BaseBoneIndex )
			{
				const FLOAT		Length = SkelComponent->SkeletalMesh->RefSkeleton(BoneIndex).BonePos.Position.Size() * (bInvertBaseBoneAxis ? -1.f : 1.f);
				const FVector	Extent = GetMatrixAxis(SkelComponent->SpaceBases(BoneIndex), BaseBoneAxis) * Length;
				PDI->DrawLine(BonePos, SkelComponent->LocalToWorld.TransformFVector(LocalBonePos + Extent), FColor(255,255,255), SDPG_Foreground);
			}
			else if( BoneIndex == AngleBoneIndex )
			{
				const FLOAT		Length = SkelComponent->SkeletalMesh->RefSkeleton(BoneIndex).BonePos.Position.Size() * (bInvertAngleBoneAxis ? -1.f : 1.f);
				const FVector	Extent = GetMatrixAxis(SkelComponent->SpaceBases(BoneIndex), AngleBoneAxis) * Length;
				PDI->DrawLine(BonePos, SkelComponent->LocalToWorld.TransformFVector(LocalBonePos + Extent), FColor(255,255,255), SDPG_Foreground);
			}
		}
	}
}


/** Draw angle between bones and node's weight on viewport */
void UGearMorph_WeightByBoneAngle::Draw(FViewport* Viewport, FCanvas* Canvas, const FSceneView* View)
{
	if( !SkelComponent || !SkelComponent->SkeletalMesh )
	{
		return;
	}

	// Make sure we have valid bone names
	const INT BaseBoneIndex		= SkelComponent->MatchRefBone(BaseBoneName);
	const INT AngleBoneIndex	= SkelComponent->MatchRefBone(AngleBoneName);

	if( BaseBoneIndex == INDEX_NONE || AngleBoneIndex == INDEX_NONE || 
		SkelComponent->SpaceBases.Num() <= BaseBoneIndex ||
		SkelComponent->SpaceBases.Num() <= AngleBoneIndex )
	{
		return;
	}

	const INT HalfX = Viewport->GetSizeX()/2;
	const INT HalfY = Viewport->GetSizeY()/2;

	FStaticLODModel& LODModel = SkelComponent->SkeletalMesh->LODModels(SkelComponent->PredictedLODLevel);
	for(INT i=0; i<LODModel.RequiredBones.Num(); i++)
	{
		const INT BoneIndex = LODModel.RequiredBones(i);
		if( BoneIndex == AngleBoneIndex )
		{
			const FVector	LocalBonePos	= SkelComponent->SpaceBases(BoneIndex).GetOrigin();
			const FVector	BonePos			= SkelComponent->LocalToWorld.TransformFVector(LocalBonePos);
			const FPlane	proj			= View->Project(BonePos);

			if( proj.W > 0.f )
			{
				const INT XPos = appTrunc(HalfX + ( HalfX * proj.X ));
				const INT YPos = appTrunc(HalfY + ( HalfY * (proj.Y * -1) ));

				const FString BoneString	= FString::Printf( TEXT("Angle: %3.0f, Weight %1.2f"), Angle, NodeWeight);

				DrawString(Canvas, XPos, YPos, *BoneString, GEngine->SmallFont, FColor(255,255,255));
				debugf(TEXT("%s"), *BoneString);
			}
		}
	}
}


/************************************************************************************
* UGearSkelCtrl_Prune
***********************************************************************************/

void UGearSkelCtrl_Prune::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	// make sure we have the right starting bone index
	INT StartBoneIndex = BoneIndex;
	{
		if (StartBoneName != NAME_None)
		{
			TArray<FMeshBone> const& RefSkel = SkelComp->SkeletalMesh->RefSkeleton;

			// search for BoneStartName
			for (INT i=0; i<RefSkel.Num(); ++i)
			{
				if (RefSkel(i).Name == StartBoneName)
				{
					StartBoneIndex = i;
					break;
				}
			}
		}
	}

	OutBoneIndices.AddItem(StartBoneIndex);
	NumAffectedBones = 1;
	CachedStartBoneIndex = StartBoneIndex;
}

void UGearSkelCtrl_Prune::CalculateNewBoneScales(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FLOAT>& OutBoneScales)
{
	OutBoneScales.AddItem(PrunedBoneScale);
}


/************************************************************************************
 * UGearAnim_AdditiveBlending
 ***********************************************************************************/

void UGearAnim_AdditiveBlending::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}
}

void UGearAnim_AdditiveBlending::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if( (bControlledByNearMisses || bControlledByDamage) && GearPawnOwner )
	{
		// Is node disabled?
		const UBOOL bIsDisabled		= (Child2WeightTarget <= ZERO_ANIMWEIGHT_THRESH);
		const UBOOL bBeingShotAt	= bControlledByNearMisses && (GWorld->GetWorldInfo()->TimeSeconds - GearPawnOwner->LastCringeTime < ControlHoldTime);
		const UBOOL bTakingDamage	= bControlledByDamage && (GWorld->GetWorldInfo()->TimeSeconds - GearPawnOwner->LastTookDamageTime < ControlHoldTime);
		const UBOOL	bShouldbeOn		= (bBeingShotAt || bTakingDamage) && !GearPawnOwner->IsReloadingWeapon();
		if( bShouldbeOn && bIsDisabled )
		{
			SetBlendTarget(ControlMaxBlendAlpha, BlendInTime);
		}
		else if( !bShouldbeOn && !bIsDisabled )
		{
			SetBlendTarget(0.f, BlendOutTime);
		}
	}


	// Update weights
	Super::TickAnim(DeltaSeconds, TotalWeight);

	// Override weights if we're a pass-through.
	if( bNotForHeavyWeapons && GearPawnOwner && GearPawnOwner->MyGearWeapon && (GearPawnOwner->MyGearWeapon->WeaponType == WT_Heavy) )
	{
		Children(0).Weight	= 1.f;
		Children(1).Weight	= 0.f;			
	}
}


/************************************************************************************
* UGearAnim_BlendByRefRelative
***********************************************************************************/

void UGearAnim_BlendByRefRelative::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache GearPawn owner to avoid casting every frame.
	if( GearPawnOwner != MeshComp->GetOwner() )
	{
		GearPawnOwner = Cast<AGearPawn>(MeshComp->GetOwner());
	}

	// Make sure ref pose data is up to date
	UpdateRefPoseData();
}

void UGearAnim_BlendByRefRelative::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// Make sure ref pose data is up to date
	UpdateRefPoseData();
}

void UGearAnim_BlendByRefRelative::PostAnimNodeInstance(UAnimNode* SourceNode)
{
	// This could be a pointer to the object owned by the source node.  Clear it 
	// so the node instance can create its own.
	RefPoseSeqNode = NULL;

	Super::PostAnimNodeInstance(SourceNode);
}

void UGearAnim_BlendByRefRelative::AnimSetsUpdated()
{
	// Make sure ref pose data is up to date
	UpdateRefPoseData();
}


/** 
 * Update RefPoseAtoms array. If taken from an animation, extract the data from it, and store it in RefPoseAtoms.
 * Use skelmesh ref pose otherwise.
 */
void UGearAnim_BlendByRefRelative::UpdateRefPoseData()
{
	// See if we can fill from animation. Use skelmesh ref pose otherwise.
	UBOOL bUsingRefPosAnim = FALSE;

	const USkeletalMesh*	SkelMesh = SkelComponent->SkeletalMesh;
	const INT				NumBones = SkelMesh->RefSkeleton.Num();

	// initialize RefPoseAtoms array
	if( RefPoseAtoms.Num() != NumBones )
	{
		RefPoseAtoms.Reset();
		RefPoseAtoms.Add(NumBones);
	}

	// Initialize Desired bones array. We take all.
	TArray<BYTE> DesiredBones;
	DesiredBones.Empty();
	DesiredBones.Add(NumBones);
	for(INT i=0; i<DesiredBones.Num(); i++)
	{
		DesiredBones(i) = i;
	}

	// We want to use an animation for ref pose
	if( RefPoseAnimName != NAME_None )
	{
		// If we don't have an AnimNodeSequence for animation extraction yet, create it.
		if( RefPoseSeqNode == NULL )
		{
			RefPoseSeqNode = ConstructObject<UAnimNodeSequence>(UAnimNodeSequence::StaticClass());
		}

		if( RefPoseSeqNode )
		{
			// Try setting the ref pose animation
			RefPoseSeqNode->SkelComponent = SkelComponent;
			RefPoseSeqNode->bDisableWarningWhenAnimNotFound = bDisableWarningWhenAnimNotFound;
			RefPoseSeqNode->SetAnim(RefPoseAnimName);

			if( RefPoseSeqNode->AnimSeq == NULL && !bDisableWarningWhenAnimNotFound )
			{
				debugf(TEXT(" UGearAnim_BlendByRefRelative::UpdateRefPoseData: RefPoseAnimName not found: %s"), *RefPoseAnimName.ToString());
			}
			else
			{
				// Extract bone atoms from animation data
				FBoneAtom	RootMotionDelta;
				INT			bHasRootMotion;
				
				FMemMark Mark(GMainThreadMemStack);

				FBoneAtomArray TempAtoms;
				TempAtoms.Add(NumBones);
				RefPoseSeqNode->GetBoneAtoms(TempAtoms, DesiredBones, RootMotionDelta, bHasRootMotion);
				RefPoseAtoms = TempAtoms;

				Mark.Pop();

				// Success!
				bUsingRefPosAnim = TRUE;
			}
		}
	}

	// If couldn't extract from animation, use skelmesh ref pose.
	if( !bUsingRefPosAnim )
	{
		FillWithRefPose(RefPoseAtoms, DesiredBones, SkelMesh->RefSkeleton);
	}
}

void UGearAnim_BlendByRefRelative::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	// See if results are cached.
	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	const INT NumAtoms = SkelComponent->SkeletalMesh->RefSkeleton.Num();
	check( NumAtoms == Atoms.Num() );

	// See if node should act as a pass through
	const UBOOL bPassThrough = bOnlyForRifleSet && GearPawnOwner && GearPawnOwner->MyGearWeapon && (GearPawnOwner->MyGearWeapon->WeaponType != WT_Normal);

	// Act as a pass through if no weight on child 1
	if( bPassThrough || Children(0).Weight >= (1.f - ZERO_ANIMWEIGHT_THRESH) || RefPoseAtoms.Num() != NumAtoms )
	{
		if( Children(0).Anim )
		{
			EXCLUDE_CHILD_TIME
			if( Children(0).bMirrorSkeleton )
			{
				GetMirroredBoneAtoms(Atoms, 0, DesiredBones, RootMotionDelta, bHasRootMotion);
			}
			else
			{
				Children(0).Anim->GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
			}
		}
		else
		{
			RootMotionDelta = FBoneAtom::Identity;
			bHasRootMotion	= 0;
			FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
		}

		// Save cached results only if we're modifying input. Otherwise we're just a pass-through.
		if( Children(0).bMirrorSkeleton )
		{
			SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
		}
		return;
	}

	FBoneAtomArray RelativeAtoms;
	RelativeAtoms.Add(NumAtoms);

	FBoneAtom	TmpBoneAtom;
	INT			TmpINT;

	{
		EXCLUDE_CHILD_TIME
		GetChildAtoms(0, Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
		GetChildAtoms(1, RelativeAtoms, DesiredBones, TmpBoneAtom, TmpINT);
	}

	const FBoneAtom SourceAtom = FBoneAtom::Identity;
	FBoneAtom DeltaAtom;

	for(INT j=0; j<DesiredBones.Num(); j++)
	{
		const INT BoneIndex = DesiredBones(j);

		// Calculate ref pose relative animation.
		DeltaAtom.Translation	= RelativeAtoms(BoneIndex).Translation - RefPoseAtoms(BoneIndex).Translation;
		DeltaAtom.Scale			= RelativeAtoms(BoneIndex).Scale / RefPoseAtoms(BoneIndex).Scale;
		DeltaAtom.Rotation		= RelativeAtoms(BoneIndex).Rotation * (-RefPoseAtoms(BoneIndex).Rotation);

		// Scale delta by weight
		if( Child2Weight < (1.f - ZERO_ANIMWEIGHT_THRESH) )
		{
			DeltaAtom.Blend(SourceAtom, DeltaAtom, Child2Weight);
		}

		// Add ref pose relative animation to base animation, only if rotation is significant.
		if( Square(DeltaAtom.Rotation.W) < 1.f - DELTA * DELTA )
		{
			Atoms(BoneIndex).Rotation = DeltaAtom.Rotation * Atoms(BoneIndex).Rotation;
		}
		Atoms(BoneIndex).Translation	+= DeltaAtom.Translation;
		Atoms(BoneIndex).Scale			*= DeltaAtom.Scale;
	}

	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}

/** Get bone atoms from child node (if no child - use ref pose). */
void UGearAnim_BlendByRefRelative::GetChildAtoms(INT ChildIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	if( Children(ChildIndex).Anim )
	{
		if( Children(ChildIndex).bMirrorSkeleton )
		{
			GetMirroredBoneAtoms(Atoms, ChildIndex, DesiredBones, RootMotionDelta, bHasRootMotion);
		}
		else
		{
			Children(ChildIndex).Anim->GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
		}
	}
	else
	{	
		RootMotionDelta	= FBoneAtom::Identity;
		bHasRootMotion = 0;
		FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
	}
}

/** 
 * Calculates total weight of children. 
 * Set a full weight on source, because it's always feeding animations into the final blend.
 */
void UGearAnim_BlendByRefRelative::SetChildrenTotalWeightAccumulator(const INT Index)
{
	if( Index == 0 )
	{
		// Update the weight of this connection
		Children(Index).TotalWeight = NodeTotalWeight;
		// Update the accumulator to find out the combined weight of the child node
		Children(Index).Anim->TotalWeightAccumulator += NodeTotalWeight;
	}
	else
	{
		// Update the weight of this connection
		Children(Index).TotalWeight = NodeTotalWeight * Children(Index).Weight;
		// Update the accumulator to find out the combined weight of the child node
		Children(Index).Anim->TotalWeightAccumulator += Children(Index).TotalWeight;
	}
}


/************************************************************************************
 * UGearSkelCtrl_IKRecoil
 ***********************************************************************************/

void UGearSkelCtrl_IKRecoil::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	bApplyControl = FALSE;

	if( ControlStrength > ZERO_ANIMWEIGHT_THRESH )
	{
		// if willing to play recoil, reset its state
		if( bPlayRecoil != bOldPlayRecoil )
		{
			bPlayRecoil = bOldPlayRecoil;

			Recoil.TimeToGo			= Recoil.TimeDuration;

			// ERS_Random == Start at random position along sine wave, 
			// ERS_Zero == Start at 0
			const FLOAT TWO_PI		= 2.f * (FLOAT)PI;
			Recoil.RotSinOffset.X	= Recoil.RotParams.X == ERS_Random ? appFrand() * TWO_PI : 0.f;
			Recoil.RotSinOffset.Y	= Recoil.RotParams.Y == ERS_Random ? appFrand() * TWO_PI : 0.f;
			Recoil.RotSinOffset.Z	= Recoil.RotParams.Z == ERS_Random ? appFrand() * TWO_PI : 0.f;

			Recoil.LocSinOffset.X	= Recoil.LocParams.X == ERS_Random ? appFrand() * TWO_PI : 0.f;
			Recoil.LocSinOffset.Y	= Recoil.LocParams.Y == ERS_Random ? appFrand() * TWO_PI : 0.f;
			Recoil.LocSinOffset.Z	= Recoil.LocParams.Z == ERS_Random ? appFrand() * TWO_PI : 0.f;

			Recoil.RotOffset		= FRotator(0,0,0);
			Recoil.LocOffset		= FVector(0.f);
		}

		if( Recoil.TimeToGo > DeltaSeconds )
		{
			Recoil.TimeToGo -= DeltaSeconds;

			if( Recoil.TimeToGo > 0.f )
			{
				bApplyControl = TRUE;

				// Smooth fade out
				const FLOAT TimePct			= Clamp<FLOAT>(Recoil.TimeToGo / Recoil.TimeDuration, 0.f, 1.f);
				const FLOAT Alpha			= TimePct*TimePct*(3.f - 2.f*TimePct);
				const FLOAT	AlphaTimesDelta	= Alpha * DeltaSeconds;

				// Recoil Bone Rotation, compute sin wave value for each component
				if( !Recoil.RotAmplitude.IsZero() )
				{
					if( Recoil.RotAmplitude.X != 0.f ) 
					{
						Recoil.RotSinOffset.X	+= AlphaTimesDelta * Recoil.RotFrequency.X;
						Recoil.RotOffset.Pitch	= appTrunc(Alpha * Recoil.RotAmplitude.X * appSin(Recoil.RotSinOffset.X));
					}
					if( Recoil.RotAmplitude.Y != 0.f ) 
					{
						Recoil.RotSinOffset.Y	+= AlphaTimesDelta * Recoil.RotFrequency.Y;
						Recoil.RotOffset.Yaw	= appTrunc(Alpha * Recoil.RotAmplitude.Y * appSin(Recoil.RotSinOffset.Y));
					}
					if( Recoil.RotAmplitude.Z != 0.f ) 
					{
						Recoil.RotSinOffset.Z	+= AlphaTimesDelta * Recoil.RotFrequency.Z;
						Recoil.RotOffset.Roll	= appTrunc(Alpha * Recoil.RotAmplitude.Z * appSin(Recoil.RotSinOffset.Z));
					}
				}

				// Recoil Bone Location, compute sin wave value for each component
				if( !Recoil.LocAmplitude.IsZero() )
				{
					if( Recoil.LocAmplitude.X != 0.f ) 
					{
						Recoil.LocSinOffset.X	+= AlphaTimesDelta * Recoil.LocFrequency.X;
						Recoil.LocOffset.X		= Alpha * Recoil.LocAmplitude.X * appSin(Recoil.LocSinOffset.X);
					}
					if( Recoil.LocAmplitude.Y != 0.f ) 
					{
						Recoil.LocSinOffset.Y	+= AlphaTimesDelta * Recoil.LocFrequency.Y;
						Recoil.LocOffset.Y		= Alpha * Recoil.LocAmplitude.Y * appSin(Recoil.LocSinOffset.Y);
					}
					if( Recoil.LocAmplitude.Z != 0.f ) 
					{
						Recoil.LocSinOffset.Z	+= AlphaTimesDelta * Recoil.LocFrequency.Z;
						Recoil.LocOffset.Z		= Alpha * Recoil.LocAmplitude.Z * appSin(Recoil.LocSinOffset.Z);
					}
				}
			}
		}
	}

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}


/** Pull aim information from Pawn */
FVector2D UGearSkelCtrl_IKRecoil::GetAim(USkeletalMeshComponent* InSkelComponent) 
{ 
	return Aim;
}

/** Is skeleton currently mirrored */
UBOOL UGearSkelCtrl_IKRecoil::IsMirrored(USkeletalMeshComponent* InSkelComponent) 
{
	return (GearPawnOwner && GearPawnOwner->bIsMirrored);
}

void UGearSkelCtrl_IKRecoil::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	// Current bone transform matrix in component space
	FMatrix NewBoneTM = SkelComp->SpaceBases(BoneIndex);

	// Extract Aim
	Aim = GetAim(SkelComp);

	// Actor to Aim transform matrix
	const FRotator	AimRotOffset( appTrunc(Aim.Y*16384), appTrunc(Aim.X*16384), 0);
	FMatrix ActorToAim = FRotationMatrix(AimRotOffset);
	ActorToAim.RemoveScaling();
	ActorToAim.SetOrigin(FVector(0.f));
	const FMatrix AimToActor = ActorToAim.Inverse();

	// Component to Actor transform matrix
	FMatrix	ComponentToActor = SkelComp->CalcComponentToFrameMatrix(BoneIndex, BCS_ActorSpace, NAME_None);
	ComponentToActor.RemoveScaling();
	ComponentToActor.SetOrigin(FVector(0.f));
	const FMatrix ActorToComponent = ComponentToActor.InverseSafe();

	// Add rotation offset in component space
	if( !Recoil.RotOffset.IsZero() )
	{
		FRotator RotOffset = Recoil.RotOffset;

		// Handle mirroring
		if( IsMirrored(SkelComp) )
		{
			RotOffset.Yaw = -RotOffset.Yaw;
		}

		FMatrix NewRotTM = NewBoneTM * (ComponentToActor * (AimToActor * FRotationMatrix(RotOffset) * ActorToAim) * ActorToComponent);	
		NewRotTM.SetOrigin(NewBoneTM.GetOrigin());
		NewBoneTM = NewRotTM;
	}

	// Add location offset in component space
	if( !Recoil.LocOffset.IsZero() )
	{
		FVector LocOffset = Recoil.LocOffset;

		// Handle mirroring
		if( IsMirrored(SkelComp) )
		{
			LocOffset.Y = -LocOffset.Y;
		}

		const FVector	TransInWorld	= ActorToAim.TransformNormal(LocOffset);
		const FVector	TransInComp		= ActorToComponent.TransformNormal(TransInWorld);
		const FVector	NewOrigin		= NewBoneTM.GetOrigin() + TransInComp;
		NewBoneTM.SetOrigin(NewOrigin);
	}

	EffectorLocation		= NewBoneTM.GetOrigin();
	EffectorLocationSpace	= BCS_ComponentSpace;

	Super::CalculateNewBoneTransforms(BoneIndex, SkelComp, OutBoneTransforms);

	// Fix up rotation
	const FVector NewOrigin = OutBoneTransforms(2).GetOrigin();
	OutBoneTransforms(2) = NewBoneTM;
	OutBoneTransforms(2).SetOrigin(NewOrigin);
}


/************************************************************************************
 * UGearSkelCtrl_FootPlanting
 ***********************************************************************************/

/** Handle interpolation of alpha used to blend between locked position and animated position. */
void UGearSkelCtrl_FootPlanting::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	// Store DeltaTime for use in CalculateNewBoneTransforms
	LastDeltaTime = DeltaSeconds;
}

void UGearSkelCtrl_FootPlanting::SetLockAlphaTarget(UBOOL bLock)
{
	if( bLock )
	{
		// When locking, interpolate only if already interpolating.
		// For smooth interpolation from animated to locked.
		// If wasn't interpolating, then foot matches animated pose which just got locked. So we're good.
		if( LockAlphaTarget != 1.f )
		{
			LockAlphaTarget = 1.f;
			if( LockAlphaBlendTimeToGo > 0.f )
			{
				LockAlphaBlendTimeToGo	= LockAlphaBlendTime - LockAlphaBlendTimeToGo;
			}
		}
	}
	else
	{
		if( LockAlphaTarget != 0.f )
		{
			LockAlphaTarget			= 0.f;
			LockAlphaBlendTimeToGo	= LockAlphaBlendTime - LockAlphaBlendTimeToGo;
		}
	}
}

void UGearSkelCtrl_FootPlanting::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	UBOOL bUseDifferentFootBone = FALSE;
	// Location of Bone used as IK end
	FMatrix LocalBoneTM = SkelComp->SpaceBases(BoneIndex);
	// Location of foot bone. This is the bone used for height test and locking on ground.
	// By default it's the same as the IK End. but it may not be...
	FMatrix LocalFootTM = SkelComp->SpaceBases(BoneIndex);

	// if we're using a different bone for foot testing/locking than IK,
	// we need to find the transform to the foot from the ik source. So we can position the IK target properly.
	if( FootBoneName != NAME_None )
	{
		const INT FootBoneIndex = SkelComp->MatchRefBone(FootBoneName);
		if( FootBoneIndex != INDEX_NONE && SkelComp->SkeletalMesh && SkelComp->SkeletalMesh->BoneIsChildOf(FootBoneIndex, BoneIndex) )
		{
			FMatrix LocalTM;
			// Special case when Foot is direct child of Bone. Don't need to setup an array.
			if( SkelComp->SkeletalMesh->RefSkeleton(FootBoneIndex).ParentIndex == BoneIndex )
			{
				SkelComp->LocalAtoms(FootBoneIndex).ToTransform(LocalTM); 
				LocalFootTM = LocalTM * LocalFootTM;
			}
			else
			{
				// Build child list in increasing index order. (parents first)
				INT TestBoneIndex = FootBoneIndex;
				TArray<INT> ChildList;
				while( TestBoneIndex > BoneIndex )
				{
					ChildList.InsertItem(TestBoneIndex, 0);
					TestBoneIndex = SkelComp->SkeletalMesh->RefSkeleton(TestBoneIndex).ParentIndex;
				}

				// Build IKSource to Foot bone transform
				for(INT i=0; i<ChildList.Num(); i++)
				{
					SkelComp->LocalAtoms(ChildList(i)).ToTransform(LocalTM);
					LocalFootTM = LocalTM * LocalFootTM;
				}
			}

			bUseDifferentFootBone = TRUE;
		}
	}

	// Handle IK Target foot bone
	UBOOL bUseIKTargetBone = FALSE;
	FVector LocalIKTarget;
	if( IKFootBoneName != NAME_None )
	{
		const INT IKTargetBoneIndex = SkelComp->MatchRefBone(IKFootBoneName);
		if( IKTargetBoneIndex != INDEX_NONE )
		{
			bUseIKTargetBone = TRUE;
			// Note: using spacebases, means we need to make sure this bone has been composed in a previous pass, otherwise we're using last frame's position.
			LocalIKTarget = SkelComp->SpaceBases(IKTargetBoneIndex).GetOrigin();
		}
	}

	FMatrix LocalToMesh = SkelComp->LocalToWorld;
	LocalToMesh.SetOrigin(FVector(0.f));
	LocalToMesh.RemoveScaling();

	const FVector LocalFootSrcLoc = LocalFootTM.GetOrigin();
	
	FVector WorldFootDstLoc = bUseIKTargetBone ? SkelComp->LocalToWorld.TransformFVector(LocalIKTarget) : SkelComp->LocalToWorld.TransformFVector(LocalFootSrcLoc);
	FVector HeightTestFootMeshSpace	= bUseIKTargetBone ? LocalToMesh.TransformFVector(LocalIKTarget) : LocalToMesh.TransformFVector(LocalFootSrcLoc);

	// if foot height is below threshold, then lock it in place!
	if( bDoFootLocking && HeightTestFootMeshSpace.Z <= FootLockZThreshold )
	{
		if( !bLockFoot )
		{
			LockedFootWorldLoc	= WorldFootDstLoc;
			bLockFoot			= TRUE;
			SetLockAlphaTarget(TRUE);
		}
		else
		{
			// Lock on X/Y, and keep updating Z.
			LockedFootWorldLoc.Z = WorldFootDstLoc.Z;
		}
	}
	else
	{
		if( bLockFoot )
		{
			bLockFoot = FALSE;
			SetLockAlphaTarget(FALSE);
		}

		// Only keep updating locked position if there is a blend from locked position going on
		if( LockAlphaBlendTimeToGo > 0.f )
		{
			// Lock on X/Y, and keep updating Z.
			LockedFootWorldLoc.Z = WorldFootDstLoc.Z;
		}
	}

	// Update LockAlpha
	if( LockAlphaBlendTimeToGo != 0.f || LockAlpha != LockAlphaTarget )
	{
		// Update the blend status, if one is active.
		const FLOAT BlendDelta = LockAlphaTarget - LockAlpha;

		if( LockAlphaBlendTimeToGo > LastDeltaTime )
		{
			LockAlpha += (BlendDelta / LockAlphaBlendTimeToGo) * LastDeltaTime;

			// Interpolate towards either Animated or Locked.
			const FVector TargetLocation = (LockAlphaTarget == 0.f) ? WorldFootDstLoc : LockedFootWorldLoc;
			const FVector TargetDelta = TargetLocation - LockFootLoc;
			LockFootLoc += (TargetDelta / LockAlphaBlendTimeToGo) * LastDeltaTime;

			LockAlphaBlendTimeToGo	-= LastDeltaTime;
		}
		else
		{
			LockAlphaBlendTimeToGo	= 0.f; 
			LockAlpha				= LockAlphaTarget;
		}
	}

	// depending on Alpha, interpolate LockFootLoc between locked position and animated position
	if( LockAlpha == 0.f )
	{
		// Use animated position
		LockFootLoc = WorldFootDstLoc;
	}
	else if( LockAlpha == 1.f )
	{
		// else, use locked position, which is LockedFootWorldLoc
		LockFootLoc = LockedFootWorldLoc;
	}

	// Setup effector
	EffectorLocationSpace	= BCS_WorldSpace;
	EffectorLocation		= LockFootLoc;

	// If using a different bone, offset to IK end point.
	if( bUseDifferentFootBone )
	{
		// World location of IK end point.
		const FVector WorldBoneLoc		= SkelComp->LocalToWorld.TransformFVector(LocalBoneTM.GetOrigin());
		const FVector WorldFootToBone	= WorldBoneLoc - SkelComp->LocalToWorld.TransformFVector(LocalFootSrcLoc);
		EffectorLocation += WorldFootToBone;
	}

	// If not locking foot, do nothing.
	if( !bLockFoot && (LockAlphaBlendTimeToGo == 0.f) && !bUseIKTargetBone )
	{
		return;
	}

	Super::CalculateNewBoneTransforms(BoneIndex, SkelComp, OutBoneTransforms);
}

void UGearSkelCtrl_FootPlanting::DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex)
{
	Super::DrawSkelControl3D(View, PDI, SkelComp, BoneIndex);

	const FVector LocalFootPos = SkelComp->SpaceBases(BoneIndex).GetOrigin();
	const FVector WorldFootPos = SkelComp->LocalToWorld.TransformFVector(LocalFootPos);

	// Draw current foot location
	DrawWireSphere(PDI, WorldFootPos, FColor(0,255,0), 8, 8, SDPG_Foreground);

	// Draw locked foot location
	if( bLockFoot )
	{
		DrawWireSphere(PDI, LockedFootWorldLoc, FColor(255,0,0), 8, 8, SDPG_Foreground);
	}

	// Draw Z Threshold
	PDI->DrawLine(FVector(-500,+000,FootLockZThreshold), FVector(+500,+000,FootLockZThreshold), FColor(0,0,255), SDPG_Foreground);
	PDI->DrawLine(FVector(+000,-500,FootLockZThreshold), FVector(+000,+500,FootLockZThreshold), FColor(0,0,255), SDPG_Foreground);

	PDI->DrawLine(FVector(-500,-500,FootLockZThreshold), FVector(+500,-500,FootLockZThreshold), FColor(0,0,255), SDPG_Foreground);
	PDI->DrawLine(FVector(-500,+500,FootLockZThreshold), FVector(+500,+500,FootLockZThreshold), FColor(0,0,255), SDPG_Foreground);

	PDI->DrawLine(FVector(-500,-500,FootLockZThreshold), FVector(-500,+500,FootLockZThreshold), FColor(0,0,255), SDPG_Foreground);
	PDI->DrawLine(FVector(+500,-500,FootLockZThreshold), FVector(+500,+500,FootLockZThreshold), FColor(0,0,255), SDPG_Foreground);
}


/************************************************************************************
 * UGearSkelCtrl_CCD_IK
 ***********************************************************************************/

void UGearSkelCtrl_CCD_IK::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);

	// Make sure we have at least two bones
	if( NumBones < 2 )
	{
		return;
	}

	OutBoneIndices.Add(NumBones);

	INT WalkBoneIndex = BoneIndex;
	for(INT i=NumBones-1; i>=0; i--)
	{
		if( WalkBoneIndex == 0 )
		{
			debugf( TEXT("UGearSkelCtrl_CCD_IK : Spline passes root bone of skeleton.") );
			OutBoneIndices.Reset();
			return;
		}

		// Look up table for bone indices
		OutBoneIndices(i) = WalkBoneIndex;
		WalkBoneIndex = SkelComp->SkeletalMesh->RefSkeleton(WalkBoneIndex).ParentIndex;
	}
}

/**
 * CCD IK or "Cyclic-Coordinate Descent Inverse kinematics"
 */
void UGearSkelCtrl_CCD_IK::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	FLOAT ChainLength = 0.f;
	TArray<INT>			BoneIndices;
	FBoneAtomArray	LocalAtoms;
	INT CurrentBoneIndex = BoneIndex;

	// Keep constraints up to date
	if( AngleConstraint.Num() != NumBones )
	{
		AngleConstraint.Reset();
		AngleConstraint.AddZeroed(NumBones);
	}

	BoneIndices.Add(NumBones);
	LocalAtoms.Add(NumBones);
	OutBoneTransforms.Add(NumBones);
	for(INT i=NumBones-1; i>=0; i--)
	{
		// Look up table for bone indices
		BoneIndices(i) = CurrentBoneIndex;
		// Copy local transforms
		LocalAtoms(i) = SkelComp->LocalAtoms(CurrentBoneIndex);
		// Calculate size of bone chain
		ChainLength += LocalAtoms(i).Translation.Size();
		// Copy space bases into output transforms.
		OutBoneTransforms(i) = SkelComp->SpaceBases(CurrentBoneIndex);

		CurrentBoneIndex = SkelComp->SkeletalMesh->RefSkeleton(CurrentBoneIndex).ParentIndex;
	}

	// Add Effector translation size as well.
	ChainLength += EffectorTranslationFromBone.Size();

	// Setting IK location
	FMatrix ComponentToFrame = SkelComp->CalcComponentToFrameMatrix(BoneIndex, EffectorLocationSpace, EffectorSpaceBoneName);
	// Translation in the component reference frame.
	FVector IKTargetLocation = ComponentToFrame.InverseSafe().TransformFVector(EffectorLocation);

	// Start Bone position.
	FVector StartBonePos = OutBoneTransforms(0).GetOrigin();
	
	// Make sure we're not over extending the chain.
	FVector	DesiredDelta = IKTargetLocation - StartBonePos;
	FLOAT DesiredLength = DesiredDelta.Size();

	// Check to handle case where IKTargetLocation is the same as StartBonePos.
	FVector DesiredDir;
	if( DesiredLength < Precision )
	{
		DesiredLength	= Precision;
		DesiredDir		= FVector(1,0,0);
	}
	else
	{
		DesiredDir		= DesiredDelta/DesiredLength;
	}

	// Make sure we're not over extending the chain.
	if( DesiredLength > ChainLength - Precision )
	{
		DesiredLength = ChainLength - Precision;
		DesiredDelta = DesiredDir * DesiredLength;
		IKTargetLocation = StartBonePos + DesiredDelta;
	}

	// Start with End of Chain bone.
	INT MaxIterations = MaxPerBoneIterations * (NumBones - 1);
	INT Index = bStartFromTail ? 1 : NumBones - 1;
	INT BoneCount = 1;
	IterationsCount = 0;
	FLOAT DistToTargetSq;
	do 
	{
		// End of current bone chain
		FVector ChainEndPos = (FTranslationMatrix(EffectorTranslationFromBone) * OutBoneTransforms(NumBones-1)).GetOrigin();
		FVector ChainEndToTarget = IKTargetLocation - ChainEndPos;
		DistToTargetSq = ChainEndToTarget.SizeSquared();

		if( DistToTargetSq > Precision )
		{
			// Current Bone Start Position (Head of the Bone)
			FVector BoneStartPosition	= OutBoneTransforms(Index-1).GetOrigin();
			FVector BoneToEnd			= ChainEndPos - BoneStartPosition;
			FVector BoneToTarget		= IKTargetLocation - BoneStartPosition;
			FVector BoneToEndDir		= BoneToEnd.SafeNormal();
			FVector BoneToTargetDir		= BoneToTarget.SafeNormal();

			// Make sure we have a valid setup to work with.
			if( (!BoneToEndDir.IsZero() && !BoneToTargetDir.IsZero()) && 
				// If bones are parallel, then no rotation is going to happen, so just skip to next bone.
				(!bNoTurnOptimization || (BoneToEndDir | BoneToTargetDir) < (1.f - SMALL_NUMBER)) )
			{
				FVector RotationAxis;
				FLOAT	RotationAngle;
				FindAxisAndAngle(BoneToEndDir, BoneToTargetDir, RotationAxis, RotationAngle);

				// Max turn steps.
				if( MaxAngleSteps > 0.f && RotationAngle > MaxAngleSteps )
				{
					RotationAngle = MaxAngleSteps;
				}

				FQuat RotationQuat(RotationAxis, RotationAngle);

				OutBoneTransforms(Index-1).SetOrigin(FVector(0.f));
				OutBoneTransforms(Index-1) = OutBoneTransforms(Index-1) * FQuatRotationTranslationMatrix(RotationQuat, FVector(0.f));
				OutBoneTransforms(Index-1).SetOrigin(BoneStartPosition);
				check(!OutBoneTransforms(Index-1).ContainsNaN());

				// Update local transform for this bone
				FMatrix ParentTM;
				if( Index >= 2 )
				{
					ParentTM = OutBoneTransforms(Index-2);
				}
				else
				{
					INT ParentBoneIndex = SkelComp->SkeletalMesh->RefSkeleton( BoneIndices(Index-1) ).ParentIndex;
					if( ParentBoneIndex == 0 )
					{
						ParentTM = FMatrix::Identity;
					}
					else
					{
						ParentTM = SkelComp->SpaceBases(ParentBoneIndex);
					}
				}

				LocalAtoms(Index - 1) = FBoneAtom(OutBoneTransforms(Index-1) * ParentTM.InverseSafe());

				// Apply angle constraint
				if( AngleConstraint(Index-1) > 0.f )
				{
					LocalAtoms(Index-1).Rotation.ToAxisAndAngle(RotationAxis, RotationAngle);

					// If we're beyond constraint, enforce limits.
					if( RotationAngle > AngleConstraint(Index-1) )
					{
						RotationAngle = AngleConstraint(Index-1);
						LocalAtoms(Index-1).Rotation = FQuat(RotationAxis, RotationAngle);
						
						FMatrix LocalBoneTM;
						LocalAtoms(Index-1).ToTransform(LocalBoneTM);
						OutBoneTransforms(Index-1) = LocalBoneTM * ParentTM;
					}
				}

				// Update world transforms for children if needed
				for(int i=Index; i<NumBones; i++)
				{
					FMatrix LocalBoneTM;
					LocalAtoms(i).ToTransform(LocalBoneTM);
					OutBoneTransforms(i) = LocalBoneTM * OutBoneTransforms(i-1);
				}
			}
		}

		// Handle looping through bones
		BoneCount++;
		if( bStartFromTail )
		{
			if( ++Index >= NumBones )
			{
				Index = 1;
				BoneCount = 1;
			}
		}
		else
		{
			if( --Index < 1 )
			{
				Index = NumBones - 1;
				BoneCount = 1;
			}
		}

	} while( IterationsCount++ < MaxIterations && DistToTargetSq > Precision );
}

INT UGearSkelCtrl_CCD_IK::GetWidgetCount()
{
	return 1;
}

FMatrix UGearSkelCtrl_CCD_IK::GetWidgetTM(INT WidgetIndex, USkeletalMeshComponent* SkelComp, INT BoneIndex)
{
	check(WidgetIndex == 0);
	FMatrix ComponentToFrame = SkelComp->CalcComponentToFrameMatrix(BoneIndex, EffectorLocationSpace, EffectorSpaceBoneName);
	FMatrix FrameToComponent = ComponentToFrame.InverseSafe() * SkelComp->LocalToWorld;
	FrameToComponent.SetOrigin( SkelComp->LocalToWorld.TransformFVector( ComponentToFrame.InverseSafe().TransformFVector(EffectorLocation) ) );

	return FrameToComponent;
}

void UGearSkelCtrl_CCD_IK::HandleWidgetDrag(INT WidgetIndex, const FVector& DragVec)
{
	check(WidgetIndex == 0);
	EffectorLocation += DragVec;
}

/************************************************************************************
* UGearSkelCtrl_Flamethrower
***********************************************************************************/

void UGearSkelCtrl_Flamethrower::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);

	AFlameThrowerSprayBase* const FlameSprayOwner = Cast<AFlameThrowerSprayBase>(SkelComp->GetOwner());

	if (FlameSprayOwner)
	{
		// affect everything in the bone chain
		OutBoneIndices.Add(FlameSprayOwner->BoneChain.Num());
		for (INT Idx=0; Idx<FlameSprayOwner->BoneChain.Num(); ++Idx)
		{
			OutBoneIndices(Idx) = FlameSprayOwner->BoneChain(Idx).BoneIndex;
		}
	}
}

void UGearSkelCtrl_Flamethrower::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	AFlameThrowerSprayBase* const FlameSprayOwner = Cast<AFlameThrowerSprayBase>(SkelComp->GetOwner());

	if (FlameSprayOwner)
	{
		FlameSprayOwner->bSkeletonHasBeenUpdated = TRUE;

		OutBoneTransforms.Add(FlameSprayOwner->BoneChain.Num());

		TArray<FVector> BoneWorldLocations;
		BoneWorldLocations.Add(FlameSprayOwner->BoneChain.Num());

		// walk the bones front to back, placing bones along seed path starting at seed 0
		TArray<FFlameSpraySeed>& Seeds = FlameSprayOwner->Seeds;
		FLOAT ChainLengthSum = 0.f;
		FLOAT SeedLengthSum = 0.f;
		INT StartSeedIdx = 1;
		FFlameSpraySeed const* PrevSeed = &Seeds(0);
		for (INT ChainBoneIdx=0; ChainBoneIdx<FlameSprayOwner->BoneChain.Num(); ++ChainBoneIdx)
		{
			INT const RefBoneIdx = FlameSprayOwner->BoneChain(ChainBoneIdx).BoneIndex;

			ChainLengthSum += SkelComp->LocalAtoms(RefBoneIdx).Translation.Size();

			// find place in seed chain that corresponds to this length
			// @fixme, this is suboptimal, but good enough to get it working.  ideally, keep track of 
			// length along the seed chain as we go.

			UBOOL bFoundASeed = FALSE;
			FVector NewBoneWorldLoc(0.f);

			for (INT SeedIdx=StartSeedIdx; SeedIdx<Seeds.Num(); ++SeedIdx)
			{
				FFlameSpraySeed const* const CurSeed = &Seeds(SeedIdx);

				// Segment points from Cur to Prev
				FVector Segment = PrevSeed->Location - CurSeed->Location;
				FLOAT SegmentLength = Segment.Size();

				SeedLengthSum += SegmentLength;

				if (SeedLengthSum >= ChainLengthSum)
				{
					// we've passed the desired distance, back up the required amount and save the position
					FVector const SegmentNorm = Segment / SegmentLength;
					FLOAT const BackUpDistance = SeedLengthSum - ChainLengthSum;

					NewBoneWorldLoc = CurSeed->Location + BackUpDistance * SegmentNorm;

					// store this for collision checking
					FlameSprayOwner->BoneChain(ChainBoneIdx).SeedChainLoc = ((FLOAT)SeedIdx-1) + (1.f - BackUpDistance / SegmentLength);

					bFoundASeed = TRUE;

					// for the next bone, we'll start at PrevSeed again, so don't count this segment length
					SeedLengthSum -= SegmentLength;
					StartSeedIdx = SeedIdx;
					break;
				}

				// get ready for next loop
				PrevSeed = CurSeed;
			}

			if (!bFoundASeed)
			{
				if (Seeds.Num() > 0)
				{
					// for now, stick to last seed.  can become an issue if seed speed is slower 
					// than spray animation speed, in which case we could extrapolate.
					NewBoneWorldLoc = Seeds.Last().Location;
					FlameSprayOwner->BoneChain(ChainBoneIdx).SeedChainLoc = Seeds.Num() - 1.f;
				}
				else
				{
					NewBoneWorldLoc = FlameSprayOwner->Location;
					FlameSprayOwner->BoneChain(ChainBoneIdx).SeedChainLoc = 0.f;
				}
			}

			BoneWorldLocations(ChainBoneIdx) = NewBoneWorldLoc;
			FlameSprayOwner->BoneChain(ChainBoneIdx).LastLoc = NewBoneWorldLoc;		// cache for later access
		}

		// smooth out the bone orientations and convert to component space 
		for (INT XformIdx=0; XformIdx<BoneWorldLocations.Num(); ++XformIdx)
		{
			FVector Tan;
			if ( XformIdx == (BoneWorldLocations.Num()-1) )
			{
				Tan = (BoneWorldLocations(XformIdx) - BoneWorldLocations(XformIdx-1)).SafeNormal();
			}
			else if (XformIdx == 0)
			{
				Tan = (BoneWorldLocations(XformIdx+1) - BoneWorldLocations(XformIdx)).SafeNormal();
			}
			else
			{
				Tan = (BoneWorldLocations(XformIdx+1) - BoneWorldLocations(XformIdx-1)).SafeNormal();
			}

			// debug rendering
			//static UBOOL bTest = 0;
			//if (bTest)
			//{
			//	FlameSprayOwner->DrawDebugBox(BoneWorldLocations(XformIdx), FVector(8,8,8), 255, 128, 128);
			//	FlameSprayOwner->DrawDebugLine(BoneWorldLocations(XformIdx)+Tan*64, BoneWorldLocations(XformIdx)-Tan*64, 255, 255, 255);
			//}

			FMatrix const WorldToComp = SkelComp->LocalToWorld.Inverse();
			// some world locations are right on top of each other, SafeNormal() tripped the safety case and returned a zero vector.
			FRotator const TangentRot = (Tan.IsZero() ? FlameSprayOwner->Rotation : Tan.Rotation()) + FlameSprayOwner->SkeletalSprayMesh->Rotation;
			FRotationTranslationMatrix const WorldTM(TangentRot, BoneWorldLocations(XformIdx));
			OutBoneTransforms(XformIdx) = WorldTM * WorldToComp;
		}
	}
}




/************************************************************************************
 * UGearSkelCtrl_FlamethrowerScaling
 ***********************************************************************************/


void UGearSkelCtrl_FlamethrowerScaling::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AFlameThrowerSprayBase* const FlameSprayOwner = Cast<AFlameThrowerSprayBase>(SkelComp->GetOwner());

	// update skel control with info from the parent spray object
	// will be NULL in the editor
	if (FlameSprayOwner != NULL)
	{
		CurrentAge = FlameSprayOwner->CurrentAge;
		LastVel = CurrentVel;
		CurrentVel = FInterpTo(LastVel, FlameSprayOwner->RotationSpeed, DeltaSeconds, VelocitySmoothingInterpSpeed);
	}	

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

void UGearSkelCtrl_FlamethrowerScaling::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);

	for (INT Idx=0; Idx<ScaleParams.Num(); ++Idx)
	{
		INT const BoneIdx = SkelComp->MatchRefBone(ScaleParams(Idx).BoneName);

		ScaleParams(Idx).CachedBoneIndex = BoneIdx;

		if (BoneIdx != INDEX_NONE)
		{
			OutBoneIndices.AddItem(BoneIdx);
		}
	}
}

void UGearSkelCtrl_FlamethrowerScaling::CalculateNewBoneScales(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FLOAT>& OutBoneScales)
{
	check(OutBoneScales.Num() == 0);

	AFlameThrowerSprayBase* const FlameSprayOwner = Cast<AFlameThrowerSprayBase>(SkelComp->GetOwner());
	if ( (FlameSprayOwner != NULL) && (FlameSprayOwner->Seeds.Num() > 0) )
	{
		for (INT Idx=0; Idx<ScaleParams.Num(); ++Idx)
		{
			if (ScaleParams(Idx).CachedBoneIndex != INDEX_NONE)
			{
				FLOAT FinalScale = 1.f;

				if (ScaleParams(Idx).bScaleIn)
				{
					FLOAT ScaleInScale = 1.f;
					FFlameScaleInParams& ScaleInParams = ScaleParams(Idx).ScaleInParams;

					// calculate parametric time
					FLOAT TimeParam = (CurrentAge - ScaleInParams.ScaleInTimeRange.X) / (ScaleInParams.ScaleInTimeRange.Y - ScaleInParams.ScaleInTimeRange.X);
					TimeParam = Clamp<FLOAT>(TimeParam, 0.f, 1.f);

					// apply pow function
					if (ScaleInParams.Pow != 1.f)
					{
						TimeParam = appPow(TimeParam, ScaleInParams.Pow);
					}

					// get scale from the range
					ScaleInScale = Lerp<FLOAT>(ScaleInParams.ScaleRange.X, ScaleInParams.ScaleRange.Y, TimeParam);
			
					FinalScale *= ScaleInScale;
				}

				if (ScaleParams(Idx).bScaleWithVelocity)
				{
					FLOAT VelocityScale = 1.f;
					FFlameVelocityScaleParams& VelScaleParams = ScaleParams(Idx).VelocityScaleParams;

					// calculate parametric time
					FLOAT VelParam = (CurrentVel - VelScaleParams.VelocityRange.X) / (VelScaleParams.VelocityRange.Y - VelScaleParams.VelocityRange.X);
					VelParam = Clamp<FLOAT>(VelParam, 0.f, 1.f);

					// apply pow function
					if (VelScaleParams.Pow != 1.f)
					{
						VelParam = appPow(VelParam, VelScaleParams.Pow);
					}

					// get scale from the range
					VelocityScale = Lerp<FLOAT>(VelScaleParams.ScaleRange.X, VelScaleParams.ScaleRange.Y, VelParam);

					FinalScale *= VelocityScale;
				}

				OutBoneScales.AddItem(FinalScale);
			}
		}
	}
	else
	{
		// fill in with 1's
		for (INT Idx=0; Idx<ScaleParams.Num(); ++Idx)
		{
			if (ScaleParams(Idx).CachedBoneIndex != INDEX_NONE)
			{
				FLOAT One = 1.f;
				OutBoneScales.AddItem(One);
			}
		}
	}
}



/************************************************************************************
 * UGearSkelCtrl_Copy
 ***********************************************************************************/

void UGearSkelCtrl_Copy::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	// Only have automatic control in-game. manual in editor.
	if( GIsGame )
	{
		// Is Control currently active?
		UBOOL bActive = (StrengthTarget > 0.f);
		// What we're going to change it to.
		UBOOL bNewActive = TRUE;

		// Owner
		AActor* AOwner = SkelComp ? SkelComp->GetOwner() : NULL;
		// Pawn Owner
		APawn*	PawnOwner = AOwner ? AOwner->GetAPawn() : NULL;

		// Update our cached GearPawn owner if needed.
		if( CachedGearPawn != PawnOwner )
		{
			CachedGearPawn = Cast<AGearPawn>(PawnOwner);
		}

		// Mirror node controls how this node is toggled.
		if( bMirror_Controlled )
		{
			// If we don't have a cached mirror node, then look for one now.
			// See if we can grab it from our Pawn.
			if( CachedGearPawn && CachedGearPawn->MirrorNode )
			{
				CachedMirrorNode = CachedGearPawn->MirrorNode;
			}
			// Otherwise try to find it within this AnimTree.
			else if( !CachedMirrorNode || GIsEditor )
			{
				CachedMirrorNode = Cast<UGearAnim_Mirror_Master>(SkelComp->Animations->FindAnimNode( FName(TEXT("MirrorNode")) ));
			}

			// If Mirror Node hasn't been found, then throw a warning, because this is an expensive operation we can do only once.
			if( !CachedMirrorNode && !GIsEditor )
			{
				debugf(TEXT("Warning! UGearSkelCtrl_Copy MirrorNode not found. Please toggle bMirror_Controlled back to FALSE."));
			}

			// If we have a mirror node, let it control the bone controller.
			if( CachedMirrorNode )
			{
				// Update cached status. And Change bActive state.
				const UBOOL bMirrorStatus = bInvertMirrorControl ? !CachedMirrorNode->bIsMirrored : CachedMirrorNode->bIsMirrored;
				bNewActive = bNewActive && !CachedMirrorNode->bPlayingTransition && bMirrorStatus;
			}
		}

		// Turn off during weapon switches.
		if( bNewActive && bDisableDuringWeaponSwitches && CachedGearPawn && CachedGearPawn->bSwitchingWeapons )
		{
			bNewActive = FALSE;
		}

		// Weapons controls how this node is toggled
		if( bNewActive && bHeavyWeaponControlled )
		{
			// Make sure cached weapon is updated.
			AGearWeapon* GearWeapon = CachedGearPawn ? CachedGearPawn->MyGearWeapon : NULL;

			if( CachedGearWeapon != GearWeapon )
			{
				CachedGearWeapon = GearWeapon;

				// Save flag so we don't have to do this check every frame, but only when we change weapons.
				bHeavyWeaponStatus = CachedGearWeapon && (CachedGearWeapon->WeaponType == WT_Heavy);
			}

			bNewActive = bNewActive && bHeavyWeaponStatus;
		}

		// Force node to be disabled during "cover break transitions" (pushout, coverslip, etc.), as heavy weapons don't like that.
		if( bNewActive && bHeavyWeaponStatus && CachedGearPawn && CachedGearPawn->CoverType != CT_None && 
			CachedGearPawn->SpecialMove != SM_None && CachedGearPawn->SpecialMoves(CachedGearPawn->SpecialMove)->bBreakFromCover )
		{
			bNewActive = FALSE;
		}

		// See if we need to update node status
		if( bNewActive != bActive )
		{
			SetSkelControlActive(bNewActive);
		}
	}

	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

/************************************************************************/
/* UGearAnim_RockwormTail_SynchToPrev                                   */
/************************************************************************/
void UGearAnim_RockwormTail_SynchToPrev::InitCachePtrs()
{
	if(!MyTailSeg)
	{
		//debugf(TEXT("WHAT?! UGearAnim_RockwormTail_SynchToPrev is being initialized on %s, which isnt' a rockworm tail segment!"),*MeshComp->GetOwner()->GetName());
		return;
	}

	// find our master animnodesequence
	if(MyTailSeg->PrevSegment && MyTailSeg->PrevSegment->Mesh && MyTailSeg->PrevSegment->Mesh->Animations)
	{
		TArray<UAnimNode*> AnimNodes;
		MyTailSeg->PrevSegment->Mesh->Animations->GetNodesByClass(AnimNodes,GetClass());
		if(AnimNodes.Num() > 0)
		{
			UGearAnim_RockwormTail_SynchToPrev* MasterSynchNode = Cast<UGearAnim_RockwormTail_SynchToPrev>(AnimNodes(0));
			if(MasterSynchNode)
			{
				MasterNodeToSyncTo = MasterSynchNode->MySynchNode;
			}			
		}
	}

	if(!MasterNodeToSyncTo)
	{
		debugf(TEXT("%s Found no master node to synch to!"), *MyTailSeg->GetName());
	}

	if(!MySynchNode)
	{
		debugf(TEXT("%s Found no local node to synch!"), *MyTailSeg->GetName());
	}
}

void UGearAnim_RockwormTail_SynchToPrev::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp,Parent);

	// find the animnodesequence we need to synchronize 
	TArray<UAnimNodeSequence*> Nodes;
	Children(0).Anim->GetAnimSeqNodes(Nodes);
	for( INT i=0; i < Nodes.Num(); i++ )
	{
		UAnimNodeSequence* SeqNode = Nodes(i);
		if( SeqNode->SynchGroupName == SynchGroupName )	// need to be from the same group name
		{
			MySynchNode = SeqNode;
			break;
		}
	}


	// cache ref to our tailseg
	MyTailSeg = Cast<ARockWorm_TailSegment>(MeshComp->GetOwner());
}


/** 
* Get relative position of a synchronized node. 
* Taking into account node's relative offset.
*/
static inline FLOAT GetNodeRelativePosition(UAnimNodeSequence* SeqNode)
{
	if( SeqNode && SeqNode->AnimSeq && SeqNode->AnimSeq->SequenceLength > 0.f )
	{
		// Find it's relative position on its time line.
		FLOAT RelativePosition = appFmod((SeqNode->CurrentTime / SeqNode->AnimSeq->SequenceLength), 1.f);
		if( RelativePosition < 0.f )
		{
			RelativePosition += 1.f;
		}

		return RelativePosition;
	}

	return 0.f;
}


/** 
* Find out position of a synchronized node given a relative position. 
* Taking into account node's relative offset.
*/
static inline FLOAT FindNodePositionFromRelative(UAnimNodeSequence* SeqNode, FLOAT RelativePosition)
{
	if( SeqNode && SeqNode->AnimSeq )
	{
		FLOAT SynchedRelPosition = appFmod(RelativePosition + SeqNode->SynchPosOffset, 1.f);
		if( SynchedRelPosition < 0.f )
		{
			SynchedRelPosition += 1.f;
		}

		return SynchedRelPosition * SeqNode->AnimSeq->SequenceLength;
	}

	return 0.f;
}

void UGearAnim_RockwormTail_SynchToPrev::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	Super::TickAnim(DeltaSeconds,TotalWeight);

	// if we have a tailseg ptr, and our tailsegptr's prev ptr is non null, but we dont' have cache nodes try and find them 
	if(MyTailSeg && MyTailSeg->PrevSegment && (!MySynchNode || !MasterNodeToSyncTo))
	{
		InitCachePtrs();
	}

	if(!MySynchNode)
	{
		return;
	}

	if(MySynchNode->bPlaying)
	{
		// if we have no master, we are the master
		const FLOAT MasterMoveDelta = MySynchNode->Rate * MySynchNode->AnimSeq->RateScale * DeltaSeconds;
		if(MasterNodeToSyncTo == NULL)
		{
			MySynchNode->AdvanceBy(MasterMoveDelta,DeltaSeconds,TRUE );
		}
		else if(MasterNodeToSyncTo->bPlaying)
		{
			// sync to master

			if(MySynchNode->AnimSeq && MySynchNode->AnimSeq->SequenceLength > 0.f)
			{

				// Find it's relative position on its time line.
				const FLOAT MasterRelativePosition = GetNodeRelativePosition(MasterNodeToSyncTo);

				
				const FLOAT NewTime		= FindNodePositionFromRelative(MySynchNode, MasterRelativePosition);
				FLOAT SlaveMoveDelta	= appFmod(NewTime - MySynchNode->CurrentTime, MySynchNode->AnimSeq->SequenceLength);

				
				// Make sure SlaveMoveDelta And MasterMoveDelta are the same sign, so they both move in the same direction.
				if( SlaveMoveDelta * MasterMoveDelta < 0.f )
				{
					if( SlaveMoveDelta >= 0.f )
					{
						SlaveMoveDelta -= MySynchNode->AnimSeq->SequenceLength;
					}
					else
					{
						SlaveMoveDelta += MySynchNode->AnimSeq->SequenceLength;
					}
				}

				// Move slave node to correct position
				MySynchNode->AdvanceBy(SlaveMoveDelta, DeltaSeconds, TRUE);
			}


		}

	}
}


void UGearAnim_BlendByReaverLean::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	Super::TickAnim(DeltaSeconds, TotalWeight);

	// Find the Reaver vehicle
	AActor* Owner = SkelComponent->GetOwner();
	if(!Owner)
	{
		return;
	}

	AVehicle_RideReaver_Base* Reaver = Cast<AVehicle_RideReaver_Base>(Owner);
	if(!Reaver)
	{
		Reaver = Cast<AVehicle_RideReaver_Base>(Owner->Base);
	}

	if(Reaver)
	{
		FVector ReaverY = Reaver->LocalToWorld().GetAxis(1);
		FLOAT StrafeVel = (Reaver->Velocity | ReaverY);
		FLOAT CurrentBlendAmount = (Reaver->AngularVelocity.Z * LeanYawFactor) + (StrafeVel * LeanStrafeFactor);
		CurrentBlendAmount = Clamp(CurrentBlendAmount, -1.f, 1.f);

		// We smooth out the blending
		LeanAmountHistory[LeanAmountSlot] = CurrentBlendAmount;
		if(++LeanAmountSlot >= 10)
		{
			LeanAmountSlot = 0;
		}
		FLOAT BlendAmount = GetFloatAverage(LeanAmountHistory, 10);

		if(BlendAmount < -LeanThreshold)
		{
			Children(0).Weight = -BlendAmount;
			Children(1).Weight = 1.f - Children(0).Weight;
			Children(2).Weight = 0.f;
		}
		else if(BlendAmount > LeanThreshold)
		{
			Children(2).Weight = BlendAmount;
			Children(1).Weight = 1.f - Children(2).Weight;
			Children(0).Weight = 0.f;
		}
		else
		{
			Children(0).Weight = 0.f;
			Children(1).Weight = 1.f;
			Children(2).Weight = 0.f;
		}

	}
}
