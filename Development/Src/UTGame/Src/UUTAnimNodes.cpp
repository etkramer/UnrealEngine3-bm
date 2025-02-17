//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"
#include "EngineAnimClasses.h"
#include "UTGameAnimationClasses.h"
#include "UTGameVehicleClasses.h"

IMPLEMENT_CLASS(UUTAnimBlendBase);
IMPLEMENT_CLASS(UUTAnimBlendByIdle);
IMPLEMENT_CLASS(UUTAnimBlendByTurnInPlace);
IMPLEMENT_CLASS(UUTAnimNodeSequence);
IMPLEMENT_CLASS(UUTAnimBlendByPhysics);
IMPLEMENT_CLASS(UUTAnimBlendByFall);
IMPLEMENT_CLASS(UUTAnimBlendByHoverJump);
IMPLEMENT_CLASS(UUTAnimBlendByPosture);
IMPLEMENT_CLASS(UUTAnimBlendByWeapon);
IMPLEMENT_CLASS(UUTAnimBlendByWeapType);
IMPLEMENT_CLASS(UUTAnimBlendByDriving);
IMPLEMENT_CLASS(UUTAnimBlendByVehicle);
IMPLEMENT_CLASS(UUTAnimBlendByFlying);
IMPLEMENT_CLASS(UUTAnimBlendByHoverboarding);
IMPLEMENT_CLASS(UUTAnimBlendBySpeed);
IMPLEMENT_CLASS(UUTAnimBlendBySlotActive);
IMPLEMENT_CLASS(UUTAnimNodeFramePlayer);
IMPLEMENT_CLASS(UUTAnimBlendByHoverboardTilt);
IMPLEMENT_CLASS(UUTAnimBlendByHoverboardTurn);
IMPLEMENT_CLASS(UUTAnimBlendByPhysicsVolume);
IMPLEMENT_CLASS(UUTAnimNodeCopyBoneTranslation);
IMPLEMENT_CLASS(UUTAnimNodeSeqWeap);
IMPLEMENT_CLASS(UUTAnimNodeJumpLeanOffset);


/** 
 * ClacDist - Takes 2 yaw values A & B and calculates the distance between them. 
 *
 *	@param	YawA	- First Yaw
 *  @param	YawB	- Second Yaw
 *  @param	Dist	- The distance between them is returned here
 *  
 *	@return			- Returns the sign needed to move from A to B
 */

static INT CalcDist(INT YawA, INT YawB, INT& Dist)
{
	INT Sign = 1;

	Dist = YawA - YawB;
	if ( Abs(Dist) > 32767 )
	{
		if (Dist > 0)
		{
			Sign = -1;
		}
		Dist = Abs( Abs(Dist) - 65536 );
	}
	else
	{
		if (Dist < 0)
		{
			Sign = -1;
		}
	}
	return Sign;
}


/**
 *  BlendByPhysics - This AnimNode type is used to determine which branch to player
 *  by looking at the current physics of the pawn.  It uses that value to choose a node
 */
void UUTAnimBlendByPhysics::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	// Get the Pawn Owner
	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		// Decrement 'pending change' countdown.
		PendingTimeToGo -= DeltaSeconds;

		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			// Get the current physics from the pawn
			INT CurrentPhysics = INT(POwner->Physics);

			// If the physics has changed, and there is a valid blend for it, blend to that value
			if ( LastPhysics != CurrentPhysics )
			{
				INT PhysicsIndex = PhysicsMap[CurrentPhysics];
				if (PhysicsIndex < 0)
				{
					PhysicsIndex = 0;
				}
				
				// Indicate what the next child to change to is..
				PendingChildIndex = PhysicsIndex;
				// .. and when
				// (don't do landing delay if we have only been in the air less than 0.5 secs)
				PendingTimeToGo = ((PendingChildIndex == 0) && (PendingTimeToGo < -0.5f))?LandBlendDelay:0.f;
			}

			LastPhysics = CurrentPhysics;			
		}

		// If its time to change, do it now.
		if(PendingTimeToGo <= 0.f)
		{
			if(ActiveChildIndex != PendingChildIndex)
			{
				SetActiveChild( PendingChildIndex,GetBlendTime(PendingChildIndex,FALSE) );
			}
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


/** Used to cache pointer to leaning node. */
void UUTAnimBlendByFall::InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim(meshComp,Parent);

	// See if this parent is a lean node, and cache a pointer to it if so.
	UUTAnimNodeJumpLeanOffset* LeanNode = Cast<UUTAnimNodeJumpLeanOffset>(Parent);
	if(LeanNode)
	{
		CachedLeanNode = LeanNode;
	}
}


/**
 * BlendByFall - Will use the pawn's Z Velocity to determine what type of blend to perform.  
 */

void UUTAnimBlendByFall::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if ( TotalWeight > 0 )
	{
		// If we are not being rendered, reset the state to FBT_None and exit.

		if (SkelComponent != NULL &&
			SkelComponent->GetOwner() != NULL)
		{
			APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
			if ( POwner )
			{
				if (POwner->Physics == PHYS_Falling)
				{
					FLOAT FallingVelocity = POwner->Velocity.Z;
					FLOAT JumpAnimTime = 0.f;
	
					switch (FallState)
					{
						case FBT_Land:
						case FBT_DblJumpLand:
						case FBT_None:		//------------- We were inactive, determine the initial state
							bDidDoubleJump = FALSE;

							if ( FallingVelocity < 0 )			// Falling
							{
								ChangeFallState(FBT_Down);
								JumpAnimTime = GetAnimDuration((INT)FBT_Down);
							}
							else								// Jumping
							{
								ChangeFallState(FBT_Up);
								JumpAnimTime = GetAnimDuration((INT)FBT_Up);
							}
																
							for (INT i=0;i<Children.Num();i++)
							{
								if (i != FallState)
								{
									Children(i).Weight=0.0f;
								}
								else
								{
									Children(i).Weight=1.0f;
								}
							}

							// Reset leaning to upright and then blend to full lean over duration of jump anim
							if(CachedLeanNode)
							{
								CachedLeanNode->SetLeanWeight(0.f, 0.f);
								CachedLeanNode->SetLeanWeight(1.f, JumpAnimTime);
							}

							break;

						case FBT_Up:		//------------- We are jumping
						case FBT_DblJumpUp:
							{
								UAnimNodeSequence* SeqNode = CastChecked<UAnimNodeSequence>( Children((INT)FallState).Anim );
								if(SeqNode && !SeqNode->bPlaying)
								{
									if(FallState == FBT_Up)
									{
										ChangeFallState(FBT_Down);
									}
									else
									{
										ChangeFallState(FBT_DblJumpDown);
									}
								}
							}
						case FBT_Down:
						case FBT_DblJumpDown:
							if ( LastFallingVelocity < FallingVelocity )	// Double Jump
							{
								if (!bIgnoreDoubleJumps)
								{
									bDidDoubleJump = TRUE;
									ChangeFallState(FBT_DblJumpUp);

									// When executing double jump - quickly blend back to upright
									if(CachedLeanNode && !CachedLeanNode->bDodging)
									{
										CachedLeanNode->SetLeanWeight(0.f, ToDblJumpUprightTime);
									}
								}
								else
								{
									ChangeFallState(FBT_Up);
								}
							}

							// If we did the double jump, and are at 0.0 lean weight (have finished that lean weight blend), go back to leaning again.
							if(bDidDoubleJump && CachedLeanNode && CachedLeanNode->LeanWeight == 0.f)
							{
								JumpAnimTime = GetAnimDuration((INT)FBT_DblJumpUp);

								CachedLeanNode->SetLeanWeight(1.f, JumpAnimTime - ToDblJumpUprightTime);
								CachedLeanNode->bDoubleJumping = TRUE;								
							}

							// When going down, keep checking for time we are going to hit the ground.
							if(FallingVelocity < 0.f)
							{
								DWORD TraceFlags = TRACE_World;
								FCheckResult Hit(1.f);
								
								// the 1.2 is a fudge factor, just to feel good
								//FLOAT BTime = GetBlendTime(FBT_PreLand, FALSE) * 1.2f;
								FLOAT TraceTime = ::Max(PreLandTime, PreLandStartUprightTime);
//								debugf(TEXT("### %f"),BTime);

								FVector Grav(0.f, 0.f, POwner->GetGravityZ());

								// calculate how far to trace.  want to trace N seconds into the future, where
								// N is the blend-in time of the preland anim.  Using kinematic equation d=vt*.5at^2.
								FVector HowFar = POwner->Velocity * TraceTime + 0.5f * Grav * Square(TraceTime);

								// Line trace from bottom of cylinder to find how long until we hit the ground
								FLOAT CollHeight = 0.f;
								if(POwner->CylinderComponent)
								{
									CollHeight = POwner->CylinderComponent->CollisionHeight;
								}
								FVector TraceStart = POwner->Location + FVector(0,0,-CollHeight);
								//GWorld->LineBatcher->DrawLine(TraceStart + HowFar, TraceStart, FColor(255,255,255), SDPG_World);
								GWorld->SingleLineCheck(Hit, POwner, TraceStart + HowFar, TraceStart, TraceFlags);

								FLOAT HowLong = TraceTime * Hit.Time;

								// If sooner than PreLandTime, switch to the PreLand state
								if(HowLong < PreLandTime)
								{
									ChangeFallState(bDidDoubleJump ? FBT_DblJumpPreLand : FBT_PreLand);
								}

								// If sooner than the 'get upright' time, start getting upright in time for landing.
								if(HowLong < PreLandStartUprightTime)
								{
									if(CachedLeanNode && CachedLeanNode->LeanWeightTarget != 0.f)
									{
										CachedLeanNode->SetLeanWeight(0.f, PreLandStartUprightTime);
									}
								}
							}
							break;

						default:
							break;
					}
					LastFallingVelocity = FallingVelocity;
				}
				else if ( FallState != FBT_Land )
				{
					// Ensure we are not leaning once we have landed!
					if(CachedLeanNode && CachedLeanNode->LeanWeight != 0.f)
					{
						CachedLeanNode->SetLeanWeight(0.f, 0.f);
						CachedLeanNode->bDoubleJumping = FALSE;
					}

//					debugf(TEXT("### We have Landed"));
					ChangeFallState(bDidDoubleJump ? FBT_DblJumpLand : FBT_Land);
				}
			}
		}
	}
	else if ( FallState != FBT_None )
	{
//		debugf(TEXT("### We are inactive, setting state to none"));
		ChangeFallState(FBT_None);
	}

	Super::TickAnim(DeltaSeconds,TotalWeight);	// pass along for now until falls anims are separated			

}

void UUTAnimBlendByFall::SetActiveChild( INT ChildIndex, FLOAT BlendTime )
{
	Super::SetActiveChild(ChildIndex,BlendTime);

	if( ChildIndex < 0 || ChildIndex >= Children.Num() )
	{
		ChildIndex = 0;
	}

	UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>( Children(ChildIndex).Anim );
	// Don't restart anims (avoids popping)
	if(SeqNode && !SeqNode->bPlaying)
	{
		SeqNode->PlayAnim(SeqNode->bLooping, SeqNode->Rate);
	}
}


void UUTAnimBlendByFall::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child, PlayedTime, ExcessTime);

	if ( bDodgeFall && FallState == FBT_Up && Child == Children(FBT_Up).Anim )
	{
		ChangeFallState(FBT_Down);
	}
}
		

// Changes the falling state

void UUTAnimBlendByFall::ChangeFallState(EBlendFallTypes NewState)
{
	INT NS = NewState;
//	debugf(TEXT("%s::ChangeFallState from %i to %i"),*GetName(), FallState, NS);

	if (FallState != NewState)
	{
		FallState = NewState;
		if (FallState!=FBT_None)
		{
			SetActiveChild( NewState, GetBlendTime(NewState,FALSE) );
		}
	}
}

void UUTAnimBlendByFall::PostEditChange(UProperty* PropertyThatChanged)
{
	if(PropertyThatChanged->GetFName() == FName(TEXT("bIgnoreDoubleJumps")))
	{
		if(bIgnoreDoubleJumps)
		{
			while(Children.Num() != 4)
			{
				Children.Remove(4);
				OnRemoveChild(4);
			}
		}
		else
		{
			if(Children.Num() != 8)
			{
				for(INT i=4;i<8;++i)
				{
					const INT NewChildIndex = Children.AddZeroed();
					OnAddChild(NewChildIndex);
				}
			}
		}
	}
	MarkPackageDirty();
}

void UUTAnimBlendByFall::RenameChildConnectors()
{
	UUTAnimBlendByFall* DefFall = GetArchetype<UUTAnimBlendByFall>();
	for(INT i=0; i<Children.Num(); i++)
	{
		Children(i).Name = DefFall->Children(i).Name;
	}
}

void UUTAnimBlendByHoverJump::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim(MeshComp, Parent);

	if(MeshComp && MeshComp->GetOwner())
	{
		OwnerP = (MeshComp->GetOwner()->GetAPawn());
	}
	if(OwnerP)
	{
		OwnerHV = Cast<AUTHoverVehicle>(OwnerP);
	}
}

void UUTAnimBlendByHoverJump::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if ( TotalWeight > 0.f )
	{
		// If we are not being rendered, reset the state to FBT_None and exit.
		if ( OwnerP )
		{
			FLOAT FallingVelocity = OwnerP->Velocity.Z;
			UBOOL bHoverForce = (OwnerHV && OwnerHV->CustomGravityScaling < ((AUTHoverVehicle*)(OwnerHV->GetClass()->GetDefaultActor()))->CustomGravityScaling); // // gravity being resisted
			switch (FallState)
			{
			case FBT_None:		//------------- We were inactive, determine the initial state
				ChangeFallState(FBT_Land); // fall through
				for (INT i=0;i<Children.Num();i++)
				{
					if (i != FallState)
					{
						Children(i).Weight=0.0f;
					}
					else
					{
						Children(i).Weight=1.0f;
					}
				}

				break;
			case FBT_Land:
					if(bHoverForce)
					{
							ChangeFallState(FBT_Up);
					}
				break;
			case FBT_Up:		//------------- We are jumping
				if (FallingVelocity <= 0 && bHoverForce)					// Begun to fall
						ChangeFallState(FBT_Down);
					else if(FallingVelocity <= 0) // released jump
						ChangeFallState(FBT_PreLand);
				break;						
			case FBT_Down:		//------------- We are falling
				if(bHoverForce)
				{
					if(FallingVelocity <= 0)
					{
						DWORD TraceFlags = TRACE_World;
						FCheckResult Hit(1.f);
						FLOAT BTime = GetBlendTime(FBT_PreLand, FALSE) * 1.5;
//								debugf(TEXT("### %f"),BTime);
						FVector HowFar = OwnerP->Velocity * BTime; //GetBlendTime(FBT_PreLand,FALSE) * 1.5;
//								GWorld->LineBatcher->DrawLine(POwner->Location + HowFar, POwner->Location,FColor(255,255,255));
						GWorld->SingleLineCheck(Hit, OwnerP, OwnerP->Location + HowFar, OwnerP->Location,TraceFlags);
					

						if ( Hit.Actor ) 
						{
							ChangeFallState(FBT_PreLand);
						}
					}
				}
				else
				{
					ChangeFallState(FBT_PreLand);
				}
				break;
			case FBT_PreLand:
					if (bHoverForce) // we may have released and re-pressed jump; so go back into hover mode
					{
						DWORD TraceFlags = TRACE_World;
						FCheckResult Hit(1.f);
						FLOAT BTime = GetBlendTime(FBT_PreLand, FALSE) * 1.5;
//								debugf(TEXT("### %f"),BTime);
						FVector HowFar = OwnerP->Velocity * BTime; //GetBlendTime(FBT_PreLand,FALSE) * 1.5;
//								GWorld->LineBatcher->DrawLine(POwner->Location + HowFar, POwner->Location,FColor(255,255,255));
						GWorld->SingleLineCheck(Hit, OwnerP, OwnerP->Location + HowFar, OwnerP->Location,TraceFlags);
					

						if ( !Hit.Actor ) 
						{
							ChangeFallState(FBT_Down);
						}
					}
					else if ( FallingVelocity >= 0)		// Hit Ground/Possibly right into a re-jump
					{
						ChangeFallState(FBT_Land);
					}
					break;

			default:
				break;
			}
			LastFallingVelocity = FallingVelocity;

		}
	}
	else if ( FallState != FBT_None )
	{
		ChangeFallState(FBT_None);
	}

	Super::Super::TickAnim(DeltaSeconds,TotalWeight);	// pass along for now until falls anims are separated			

}
/**
 *  BlendByPosture is used to determine if we should be playing the Crouch/walk animations, or the 
 *  running animations.
 */

void UUTAnimBlendByPosture::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	// Get the Pawn Owner
	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			if ( POwner->bIsCrouched && ActiveChildIndex!=1 )
			{
				SetActiveChild(1, BlendTime);
			}
			else if ( !POwner->bIsCrouched && ActiveChildIndex != 0 )
			{
				SetActiveChild(0, BlendTime);
			}
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/**
 * BlendByWeapon - This node is NOT automanaged.  Instead it's designed to have it's Fire/StopFire functions
 * called.  If it's playing a firing animation that's not looping (ie: not auto-fire) it will blend back out after the
 * animation completes.
 */

void UUTAnimBlendByWeapon::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child, PlayedTime, ExcessTime);

	// Call the script event if we are not looping.
	if (!bLooping)
	{
		eventAnimStopFire(BlendTime);
	}
	else if (LoopingAnim != NAME_None)
	{
		// loop with LoopingAnim instead of the original fire animation
		UAnimNodeSequence* FireNode = Cast<UAnimNodeSequence>(Children(1).Anim);
		if (FireNode != NULL)
		{
			FireNode->SetAnim(LoopingAnim);
			FireNode->PlayAnim(TRUE);
		}
	}
}

/**
 * This blend looks at the velocity of the player and blends depending on if they are moving or not
 */
void UUTAnimBlendByIdle::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Get the Pawn Owner
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		AActor* Owner = SkelComponent->GetOwner();
		if ( Owner )
		{
			//debugf(TEXT("Velocity %i"), Owner->Velocity.Size() );

			if ( Owner->Velocity.SizeSquared() < KINDA_SMALL_NUMBER )
			{
				if(ActiveChildIndex != 0)
				{
					SetActiveChild(0,BlendTime);
				}
			}
			else
			{
				if(ActiveChildIndex != 1)
				{
					SetActiveChild(1,BlendTime);
				}
			}
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimBlendByTurnInPlace::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	OwnerUTP = Cast<AUTPawn>(MeshComp->GetOwner());
}

/**
 * This blend looks at the velocity of the player root and blends in a 'turn in place' anim when turning on the spot.
 */
void UUTAnimBlendByTurnInPlace::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// Get the Pawn Owner
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		if ( OwnerUTP )
		{
			// If turning, blend to feet together pose
			if(Abs(OwnerUTP->RootYawSpeed) > RootYawSpeedThresh)
			{
				if(ActiveChildIndex != 1)
				{
					SetActiveChild(1, BlendTime);
				}
			}
			// If stopped..
			else
			{
				// .. and turn pose is active, blend to foot out/idle node.
				if(ActiveChildIndex != 0)
				{
					SetActiveChild(0, BlendTime);
					UAnimNodeSequence* IdleNode = Cast<UAnimNodeSequence>(Children(0).Anim);
					if(IdleNode)
					{
						IdleNode->SetAnim( FName(TEXT("Rotate_Settle_Rif")) );
						IdleNode->PlayAnim(FALSE, 1.f, 0.f);
					}
				}
			}
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


void UUTAnimBlendByTurnInPlace::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child, PlayedTime, ExcessTime);

	if(Child == Children(0).Anim)
	{
		Child->SetAnim(FName(TEXT("idle_ready_rif")));
		Child->PlayAnim(TRUE, 1.f, 0.f);
	}
}


void UUTAnimBlendByDriving::UpdateDrivingState()
{
	// Get the Pawn Owner
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if (POwner != NULL)
		{
			// We have to handle 2 cases - that this is used by a Pawn driving a vehicle, and used in a vehicle being driven.
			AVehicle* DrivenVehicle = POwner->DrivenVehicle;
			AVehicle* MyVehicle = POwner->GetAVehicle();
			if((DrivenVehicle != NULL && DrivenVehicle->bAttachDriver) || (MyVehicle && MyVehicle->bDriving))
			{
				SetBlendTarget(1,0);
			}
			else
			{
				SetBlendTarget(0,0);
			}
		}
	}
}

void UUTAnimBlendByDriving::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	UpdateDrivingState();
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimBlendByVehicle::UpdateVehicleState()
{
	// Get the Pawn Owner
	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			INT SeatIndex = 0;
			AUTVehicle* PVehicle = NULL;

			if ( Cast<AUTWeaponPawn>(POwner->DrivenVehicle) )
			{
				PVehicle = Cast<AUTWeaponPawn>(POwner->DrivenVehicle)->MyVehicle;
				SeatIndex = Cast<AUTWeaponPawn>(POwner->DrivenVehicle)->MySeatIndex;
			}
			else
			{
				PVehicle = Cast<AUTVehicle>(POwner->DrivenVehicle);
				SeatIndex = 0;
			}

			if (PVehicle)
			{
				ActiveChildIndex = 0;

				for (INT i=1; i<Children.Num(); i++)
				{
					UClass *Class = (UClass*) UObject::StaticFindObjectFast( UClass::StaticClass(), NULL, Children(i).Name, TRUE, TRUE );
					if (Class && PVehicle->IsA(Class))
					{
						ActiveChildIndex = i;
					}
				}

				if (ActiveChildIndex == 0)
				{
					if ( !bLastPawnDriving || LastVehicle != POwner->DrivenVehicle )
					{
						SetActiveChild(0, 0.0f);

						UAnimNodeSequence* Player;
						Player = Cast<UAnimNodeSequence>(Children(0).Anim);

						if ( Player && PVehicle && PVehicle->Seats(SeatIndex).bSeatVisible )
						{
							//debugf(TEXT("#### DrivingAnim=%s"),PVehicle->DrivingAnim);

							if (PVehicle->DrivingAnim != NAME_None) // If the child is an animation sequence just play it
							{
								Player->SetAnim(PVehicle->DrivingAnim);
								Player->PlayAnim(TRUE,1.0,1.0);
							}
						}
						LastVehicle = POwner->DrivenVehicle;	
						bLastPawnDriving = TRUE;
					}
					else
					{
						if (bLastPawnDriving)
						{
							UUTAnimNodeSequence* Player = Cast<UUTAnimNodeSequence>(Children(1).Anim);
							AUTVehicle* PVehicle = Cast<AUTVehicle>(POwner->DrivenVehicle);
							if (Player && PVehicle)
							{
								Player->StopAnim();
							}
						}
						bLastPawnDriving = FALSE;
						LastVehicle = NULL;
					}
				}
				else
				{
					SetActiveChild(ActiveChildIndex, 0.0f);
				}
			}
		}
	}
}

void UUTAnimBlendByVehicle::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	UpdateVehicleState();
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimBlendByHoverboarding::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if (POwner && POwner->DrivenVehicle)
		{
			AUTVehicle_Hoverboard* PHoverboard = Cast<AUTVehicle_Hoverboard>(POwner->DrivenVehicle);

			if ( PHoverboard )
			{
				//BoardLeanAmount = ::Clamp<FLOAT>(PHoverboard->Rotation.Roll / PHoverboard->LeanFactor, -1.0f, 1.0f);
				//debugf(TEXT("Lean: %.2f - %.2f = %.2f"), PHoverboard->LeanAmount, BoardLeanAmount, PHoverboard->LeanAmount - BoardLeanAmount);

				if (PHoverboard->bForceSpinWarmup)
				{
					SetActiveChild(13, 0.1f); // Spin left warmup animation
				}
				else if (PHoverboard->TrickJumpWarmup > 0.0f)
				{
					if (PHoverboard->OutputSteering > 0.0f)
					{
						SetActiveChild(13, 0.1f); // Spin left warmup animation
					}
					else if (PHoverboard->OutputSteering < 0.0f)
					{
						SetActiveChild(14, 0.1f); // Spin right warmup animation
					}
					else
					{
						SetActiveChild(7, 0.1f); // Trick jump warmup animation (crouch)
					}
				}
				else if (!PHoverboard->bVehicleOnGround)
				{
					if (PHoverboard->bTrickJumping && PHoverboard->bGrab1)
					{
						SetActiveChild(15, 0.1f); // Trick1 animation
					}
					else if (PHoverboard->bTrickJumping && PHoverboard->bGrab2)
					{
						SetActiveChild(16, 0.1f); // Trick2 animation
					}
					else if (PHoverboard->bTrickJumping)
					{
						if(Abs(PHoverboard->OutputSteering) > 0.0f)
						{
							SetActiveChild(12, 0.1f); // Spinning animation
						}
						else
						{
							SetActiveChild(0, 0.1f); // Hoverboard idle animation
						}
					}
					else if (PHoverboard->bInAJump)
					{
						SetActiveChild(8, 0.1f);  // Jumping up animation
					}
					else if(PHoverboard->TimeOffGround > 0.3f)
					{
						SetActiveChild(9, 0.2f);  // Falling down animation
					}
					else
					{
						SetActiveChild(0, 0.1f); // Hoverboard idle animation
					}
				}
				else if (PHoverboard->LandedCountdown > 0.0f)
				{
					SetActiveChild(10, 0.1f); // Landed animation
				}				
				else
				{
					SetActiveChild(0, 0.1f); // Hoverboard idle animation
				}
			}
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimBlendByHoverboarding::SetActiveChild( INT ChildIndex, FLOAT BlendTime )
{
	Super::SetActiveChild(ChildIndex,BlendTime);

	FLOAT AnimPosition = 0.0f;

	if (SkelComponent == NULL || SkelComponent->GetOwner() == NULL)
		return;

	APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
	if (!POwner)
		return;

	AUTVehicle_Hoverboard* PHoverboard = Cast<AUTVehicle_Hoverboard>(POwner->DrivenVehicle);
	if (!PHoverboard)
		return;

	UAnimNodeSequence* AnimSeq = Cast<UAnimNodeSequence>(Children(ActiveChildIndex).Anim);

	if (AnimSeq)
	{
		// Play the animation if this child is a sequence and not playing already and we just changed to it
		if (!AnimSeq->bPlaying && LastActiveChildIndex != ActiveChildIndex)
		{
			AnimSeq->PlayAnim( AnimSeq->bLooping, AnimSeq->Rate );							
		}
		
		//if (ActiveChildIndex == 1) // If turning left
		//{
		//	AnimSeq->SetPosition((PHoverboard->LeanAmount) * AnimSeq->GetAnimPlaybackLength(), FALSE);
		//}
		//else if (ActiveChildIndex == 2) // If turning right
		//{
		//	AnimSeq->SetPosition((PHoverboard->LeanAmount) * AnimSeq->GetAnimPlaybackLength(), FALSE);
		//}
		//else if (ActiveChildIndex == 4 || ActiveChildIndex == 5) // Left
		//{
		//	AnimSeq->SetPosition(-BoardLeanAmount * AnimSeq->GetAnimPlaybackLength(), FALSE);
		//}
		//else if (ActiveChildIndex == 3 || ActiveChildIndex == 6) // Right
		//{
		//	AnimSeq->SetPosition(BoardLeanAmount * AnimSeq->GetAnimPlaybackLength(), FALSE);
		//}

		LastActiveChildIndex = ActiveChildIndex;
	}
}


void UUTAnimBlendBySpeed::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		if (MaxSpeed <= MinSpeed)
		{
			debugf(NAME_Warning, TEXT("%s with MaxSpeed <= MinSpeed, increasing MaxSpeed"), *GetPathName());
			MaxSpeed = MinSpeed + 1.0;
		}

		Child2Weight = (SkelComponent->GetOwner()->Velocity.Size() - MinSpeed) / (MaxSpeed - MinSpeed);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/** 
 Our sequence player 
 */

void UUTAnimNodeSequence::OnAnimEnd(FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnAnimEnd(PlayedTime, ExcessTime);

	if ( SeqStack.Num() > 0 )
	{
		SetAnim( SeqStack(0) );
		SeqStack.Remove(0,1);
		

		UBOOL bSeqLoop = (SeqStack.Num() == 0) ? bLoopLastSequence : FALSE;
		PlayAnim(bSeqLoop, Rate,0.0);
	}
}

void UUTAnimNodeSequence::PlayAnimation(FName Sequence, FLOAT SeqRate, UBOOL bSeqLoop)
{
	SetAnim(Sequence);
	PlayAnim(bSeqLoop, SeqRate);
}

void UUTAnimNodeSequence::PlayAnimationSet(const TArray<FName>& Sequences,FLOAT SeqRate,UBOOL bLoopLast)
{
	if ( Sequences.Num() > 0 )
	{
		PlayAnimation(Sequences(0), SeqRate, FALSE);
		for ( INT i=1; i<Sequences.Num(); i++ )
		{
			SeqStack.AddItem( Sequences(i) );
		}
		bLoopLastSequence = bLoopLast;
	}
}

void UUTAnimNodeFramePlayer::SetAnimation(FName Sequence, FLOAT RateScale)
{
	SetAnim(Sequence);
	CurrentTime		= 0;
	PreviousTime	= 0;
	Rate			= RateScale;
	bLooping		= FALSE;
	bPlaying		= FALSE;
}

void UUTAnimNodeFramePlayer::SetAnimPosition(FLOAT Perc)
{
	if ( AnimSeq )
	{
		SetPosition( AnimSeq->SequenceLength * Perc * Rate, TRUE);
	}
}

/************************************************************************************
* UUTAnimBlendBase
***********************************************************************************/


void UUTAnimBlendBase::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	SliderPosition = NewSliderValue;

	const INT TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	if( ActiveChildIndex != TargetChannel )
	{
		FLOAT const BlendInTime = GetBlendTime(TargetChannel);
		SetActiveChild(TargetChannel, BlendInTime);
	}
}

FLOAT UUTAnimBlendBase::GetBlendTime(INT ChildIndex, UBOOL bGetDefault/*=0*/)
{
	if ( bGetDefault || (ChildBlendTimes.Num() == 0) || (ChildIndex < 0) || (ChildIndex >= ChildBlendTimes.Num()) || (ChildBlendTimes(ChildIndex) == 0.0f) )
	{
		return BlendTime;
	}
	else
	{
		return ChildBlendTimes(ChildIndex);
	}
}

FLOAT UUTAnimBlendBase::GetAnimDuration(INT ChildIndex)
{
	if((ChildIndex >= 0) && (ChildIndex < Children.Num()) && Children(ChildIndex).Anim)
	{
		UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>(Children(ChildIndex).Anim);
		if(SeqNode)
		{
			return SeqNode->GetAnimPlaybackLength();
		}
	}

	return 0.f;
}

void UUTAnimBlendByHoverboardTilt::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	// If we are in the game, grab the up vector from the hoverboard.
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if (POwner && POwner->DrivenVehicle)
		{
			AUTVehicle_Hoverboard* PHoverboard = Cast<AUTVehicle_Hoverboard>(POwner->DrivenVehicle);
			if ( PHoverboard && PHoverboard->Mesh )
			{
				UpVector = PHoverboard->Mesh->GetBoneAxis(UpperBodyName, AXIS_Z);
			}
		}
	}

	// Turn that into an angle and a magnitude
	FVector UseUp = UpVector;
	UseUp.Y *= TiltYScale;

	FLOAT TiltAngle = appAtan2(UseUp.Y, UseUp.X);
	FLOAT Mag = UseUp.Size2D();

	if(Mag < TiltDeadZone)
	{
		Children(0).Weight = 1.f;
		Children(1).Weight = 0.f;
		Children(2).Weight = 0.f;
		Children(3).Weight = 0.f;
		Children(4).Weight = 0.f;
	}
	else
	{
		FLOAT LeanWeight = Min<FLOAT>((Mag - TiltDeadZone) * TiltScale, 1.0);
		FLOAT UprightWeight = 1.f - LeanWeight;

		Children(0).Weight = UprightWeight;

		// Work out child weights based on TiltAngle.
		if(TiltAngle < -0.5f*PI) // Back and left
		{
			Children(3).Weight = (TiltAngle/(0.5f*PI)) + 2.f;
			Children(4).Weight = 0.f;

			Children(1).Weight = 0.f;
			Children(2).Weight = 1.f - Children(3).Weight;
		}
		else if(TiltAngle < 0.f) // Forward and left
		{
			Children(3).Weight = -TiltAngle/(0.5f*PI);
			Children(4).Weight = 0.f;

			Children(1).Weight = 1.f - Children(3).Weight;
			Children(2).Weight = 0.f;
		}
		else if(TiltAngle < 0.5f*PI) // Forward and right
		{
			Children(3).Weight = 0.f;
			Children(4).Weight = TiltAngle/(0.5f*PI);

			Children(1).Weight = 1.f - Children(4).Weight;
			Children(2).Weight = 0.f;
		}
		else // Back and right
		{
			Children(3).Weight = 0.f;
			Children(4).Weight = (-TiltAngle/(0.5f*PI)) + 2.f;

			Children(1).Weight = 0.f;
			Children(2).Weight = 1.f - Children(4).Weight;
		}

		for(INT i=1; i<5; i++)
		{
			Children(i).Weight *= LeanWeight;
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

FLOAT UUTAnimBlendByHoverboardTilt::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex);
	if( ValueIndex == 0 )
	{
		return (0.5f * UpVector.X) + 0.5f;
	}
	else
	{
		return ((-0.5f * UpVector.Y) + 0.5f);
	}
}

void UUTAnimBlendByHoverboardTilt::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex);
	if( ValueIndex == 0 )
	{
		UpVector.X = (NewSliderValue - 0.5f) * 2.f;
	}
	else
	{
		UpVector.Y = -1.f * (NewSliderValue - 0.5f) * 2.f;
	}
}

FString UUTAnimBlendByHoverboardTilt::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%3.2f %3.2f"), UpVector.X, UpVector.Y );
}

void UUTAnimBlendByHoverboardTurn::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	FVector AngVel(0,0,0);

	// If we are in the game, grab the angular velocity of the hoverboard
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if (POwner && POwner->DrivenVehicle)
		{
			AUTVehicle_Hoverboard* PHoverboard = Cast<AUTVehicle_Hoverboard>(POwner->DrivenVehicle);

			 if ( PHoverboard && PHoverboard->CollisionComponent && PHoverboard->CollisionComponent->BodyInstance && PHoverboard->CollisionComponent->BodyInstance->IsValidBodyInstance() )
			{
				AngVel = PHoverboard->CollisionComponent->BodyInstance->GetUnrealWorldAngularVelocity();
			}
		}
	}

	const FLOAT NewAnimWeight = Clamp<FLOAT>( AngVel.Z * TurnScale, -1.f, 1.f );
	const FLOAT MaxBlendChange = (MaxBlendPerSec * DeltaSeconds);
	const FLOAT DeltaBlend = Clamp<FLOAT>(NewAnimWeight - CurrentAnimWeight, -MaxBlendChange, MaxBlendChange);
	CurrentAnimWeight += DeltaBlend;

	if(CurrentAnimWeight > 0.f)
	{
		Children(1).Weight = CurrentAnimWeight;
		Children(0).Weight = 1.f - Children(1).Weight;
		Children(2).Weight = 0.f;
	}
	else
	{
		Children(2).Weight = -CurrentAnimWeight;
		Children(0).Weight = 1.f - Children(2).Weight;
		Children(1).Weight = 0.f;
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimBlendByFlying::TestBlend()
{
	FVector ActorSpaceVel(0,0,0);
	FVector2D TargetAim(0,0);
	static FLOAT FlyingDirStrength = 0.002f;
	if(SkelComponent && SkelComponent->GetOwner())
	{
		AActor* Owner = SkelComponent->GetOwner();
		AUTPawn* POwner = Cast<AUTPawn>(Owner->GetAPawn());
		if (POwner != NULL && POwner->FlyingDirOffset != NULL)
		{
			ActorSpaceVel = Owner->LocalToWorld().InverseTransformNormal(Owner->Velocity);

			// Y is 'forwards' on our node.
			TargetAim.X = Clamp(ActorSpaceVel.Y * FlyingDirStrength, -.5f, .5f);
			TargetAim.Y = Clamp(ActorSpaceVel.X * FlyingDirStrength, -.5f, .5f);

			// Blend with 0,0 as desired
			POwner->FlyingDirOffset->Aim.X = TargetAim.X;
			POwner->FlyingDirOffset->Aim.Y = TargetAim.Y;
			//debugf(TEXT("flydir %0.2f %0.2f"), TargetAim.X, TargetAim.Y);
		}
	}
}

void UUTAnimBlendByFlying::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim(MeshComp, Parent);
	
	//Cache the pawn
	if (MeshComp)
	{
		AActor* AOwner = MeshComp->GetOwner();
		if (AOwner)
		{
			Pawn = Cast<AUTPawn>(AOwner->GetAPawn());
		}
	}

	//Cache the children nodes of interest
	TArray<UAnimNode*> ChildNodes;

	//Flying animation switch (close/open/hover/flap)
	Children(1).Anim->GetNodesByClass(ChildNodes, UUTAnimBlendBase::StaticClass());
	check(ChildNodes.Num() == 1);
	FlyingMode = Cast<UUTAnimBlendBase>(ChildNodes(0));

	//Do we have flying transition anims
	bHasStartingAnim = FALSE;
	bHasEndingAnim = FALSE;

	if (SkelComponent)
	{
		bHasStartingAnim = SkelComponent->FindAnimSequence(StartingAnimName) != NULL ? TRUE : FALSE;
		bHasEndingAnim = SkelComponent->FindAnimSequence(EndingAnimName) != NULL ? TRUE : FALSE;
	}

	//Flying direction node
	ChildNodes.Empty();
	Children(1).Anim->GetNodesByClass(ChildNodes, UAnimNodeAimOffset::StaticClass());
	check(ChildNodes.Num() == 1);
	FlyingDir = Cast<UAnimNodeAimOffset>(ChildNodes(0));
}

void UUTAnimBlendByFlying::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child, PlayedTime, ExcessTime);

	if (Child->AnimSeqName == FName(TEXT("Wings_Open")) && FlyingState == Flying_OpeningWings)
	{
		//Change the state of flying
		FlyingMode->SetActiveChild(2, 0.0f);
		FlyingState = Flying_Flying;
	}
	else if (Child->AnimSeqName == FName(TEXT("Wings_Close")) && FlyingState == Flying_ClosingWings)
	{
		//Go back to falling/walking
		SetActiveChild(0, 0.1f);
		FlyingState = Flying_NotFlying;
	}
}

#define WING_OPENING_SPEED 1.5f
#define WING_CLOSING_SPEED 1.5f
void UUTAnimBlendByFlying::UpdateFlyingState()
{
	// Get the Pawn Owner
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		AActor* AOwner = SkelComponent->GetOwner();
		if (AOwner)
		{
			APawn* POwner = AOwner->GetAPawn();
			if (POwner != NULL)
			{
				switch (FlyingState)
				{
				case Flying_NotFlying:
					{
						if (POwner->Physics == PHYS_Flying)
						{
							//Set flying mode
							SetActiveChild(1, 0.1f);

							if (bHasStartingAnim)
							{
								//Open the wings
								FlyingState = Flying_OpeningWings;
								FlyingMode->SetActiveChild(1,0);
								FlyingMode->Children(1).Anim->PlayAnim(FALSE, WING_OPENING_SPEED);
							}
							else
							{
								FlyingMode->SetActiveChild(2, 0.0f);
								FlyingState = Flying_Flying;
							}
						}
						break;
					}
				case Flying_OpeningWings:
					{
						//If while opening the wings, we stop flying, close the wings
						if (POwner->Physics != PHYS_Flying)
						{
							FlyingState = Flying_ClosingWings;

							//Close the wings
							FlyingMode->SetActiveChild(0,0.1f);
							FlyingMode->Children(0).Anim->PlayAnim(FALSE, WING_CLOSING_SPEED);
						}
						break;
					}
				case Flying_Flying:
					{
						//If while flying, we stop flying, close the wings
						if (POwner->Physics != PHYS_Flying)
						{
							if (bHasEndingAnim)
							{
								FlyingState = Flying_ClosingWings;

								//Close the wings
								FlyingMode->SetActiveChild(0,0.1f);
								FlyingMode->Children(0).Anim->PlayAnim(FALSE, WING_CLOSING_SPEED);
							}
							else
							{
								//Go back to falling/walking
								FlyingState = Flying_NotFlying;
								SetActiveChild(0, 0.1f);
							}
						}
						else
						{
							//Continue flying
							TestBlend();
						}
						break;
					}
				case Flying_ClosingWings:
					{
						//If while closing the wings, we start flying again, open the wings
						if (POwner->Physics == PHYS_Flying)
						{
							FlyingState = Flying_OpeningWings;

							//Set flying mode
							SetActiveChild(1, 0.1f);
							//Open the wings
							FlyingMode->SetActiveChild(1,0);
							FlyingMode->Children(1).Anim->PlayAnim(FALSE, WING_CLOSING_SPEED);
						}
						break;
					}
				}
			}
		}
	}
}

void UUTAnimBlendByFlying::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	UpdateFlyingState();
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimBlendByPhysicsVolume::RenameChildConnectors()
{
	if (Children.Num() > 0)
	{
		Children(0).Name = NAME_Default;
		for (INT i = 1; i < Children.Num(); i++)
		{
			Children(i).Name = FName(*FString::Printf(TEXT("Child%d"), i));
		}
	}
}

void UUTAnimBlendByPhysicsVolume::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APhysicsVolume* NewVolume = SkelComponent->GetOwner()->PhysicsVolume;
		if (NewVolume == NULL)
		{
			NewVolume = SkelComponent->GetOwner()->WorldInfo->GetDefaultPhysicsVolume();
		}
		if (NewVolume != LastPhysicsVolume)
		{
			eventPhysicsVolumeChanged(NewVolume);
			LastPhysicsVolume = NewVolume;
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


void UUTAnimBlendByWeapType::WeapTypeChanged(FName NewAimProfileName)
{
	if(NewAimProfileName == FName(TEXT("Default")))
	{
		SetBlendTarget(0.f, 0.f);
	}
	else
	{
		SetBlendTarget(1.f, 0.f);
	}
}

/************************************************************************************
 * UUTAnimNodeCopyBoneTranslation
 ***********************************************************************************/

void UUTAnimNodeCopyBoneTranslation::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	FName AimProfileName = NAME_None;
	if(CachedAimNode)
	{
		FAimOffsetProfile* P = CachedAimNode->GetCurrentProfile();
		if(P)
		{
			AimProfileName = P->ProfileName;
		}
	}

	UpdateListOfRequiredBones(AimProfileName);
}


void UUTAnimNodeCopyBoneTranslation::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache pointer to aim node
	SeqWeaps.Empty();
	WeapTypeBlends.Empty();
	TArray<UAnimNode*> Nodes;
	MeshComp->Animations->GetNodes(Nodes);
	for (INT i=0; i<Nodes.Num(); i++)
	{
		UAnimNodeAimOffset* AimNode = Cast<UAnimNodeAimOffset>(Nodes(i));
		if( AimNode && AimNode->NodeName == FName(TEXT("AimNode")) )
		{
			CachedAimNode = AimNode;
		}

		UUTAnimNodeSeqWeap* SeqWeap = Cast<UUTAnimNodeSeqWeap>(Nodes(i));
		if(SeqWeap)
		{
			SeqWeaps.AddItem(SeqWeap);
		}

		UUTAnimBlendByWeapType* WeapTypeBlend = Cast<UUTAnimBlendByWeapType>(Nodes(i));
		if(WeapTypeBlend)
		{
			WeapTypeBlends.AddItem(WeapTypeBlend);
		}
	}

	// Update list of required bones
	FName AimProfileName = NAME_None;
	if(CachedAimNode)
	{
		FAimOffsetProfile* P = CachedAimNode->GetCurrentProfile();
		if(P)
		{
			AimProfileName = P->ProfileName;
		}
	}

	UpdateListOfRequiredBones(AimProfileName);
	OldAimProfileName = AimProfileName;
}

void UUTAnimNodeCopyBoneTranslation::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	Super::TickAnim(DeltaSeconds, TotalWeight);

	FName AimProfileName = NAME_None;
	if(CachedAimNode)
	{
		FAimOffsetProfile* P = CachedAimNode->GetCurrentProfile();
		if(P)
		{
			AimProfileName = P->ProfileName;
		}
	}

	// Based on if we are dual-wielding, change the bones that we copy.
	if(AimProfileName != OldAimProfileName)
	{
		UpdateListOfRequiredBones(AimProfileName);

		// Tell all the UTAnimNodeSeqWeap nodes about the change as well
		for(INT i=0; i<SeqWeaps.Num(); i++)
		{
			SeqWeaps(i)->WeapTypeChanged(AimProfileName);
		}

		// Tell all the UUTAnimBlendByWeapType nodes about the change as well
		for(INT i=0; i<WeapTypeBlends.Num(); i++)
		{
			WeapTypeBlends(i)->WeapTypeChanged(AimProfileName);
		}

		// Update OldAimProfileName
		OldAimProfileName = AimProfileName;
	}
}

void UUTAnimNodeCopyBoneTranslation::UpdateListOfRequiredBones(FName ProfileName)
{
	if(ProfileName == FName(TEXT("DualPistols")))
	{
		ActiveBoneCopyArray = DualWieldBoneCopyArray;
	}
	else
	{
		ActiveBoneCopyArray = DefaultBoneCopyArray;
	}

	// Empty required bones list
	RequiredBones.Empty();

	// Cache bone Indices
	for(INT i=0; i<ActiveBoneCopyArray.Num(); i++)
	{
		ActiveBoneCopyArray(i).SrcBoneIndex = SkelComponent->MatchRefBone(ActiveBoneCopyArray(i).SrcBoneName);
		ActiveBoneCopyArray(i).DstBoneIndex = SkelComponent->MatchRefBone(ActiveBoneCopyArray(i).DstBoneName);

		if( ActiveBoneCopyArray(i).SrcBoneIndex != INDEX_NONE && ActiveBoneCopyArray(i).DstBoneIndex != INDEX_NONE )
		{
			RequiredBones.AddItem(ActiveBoneCopyArray(i).SrcBoneIndex);
			RequiredBones.AddItem(ActiveBoneCopyArray(i).DstBoneIndex);
		}
	}

	// Make sure parents are present in the array. Since we need to get the relevant bones in component space.
	UAnimNode::EnsureParentsPresent(RequiredBones, SkelComponent->SkeletalMesh);
}

/** Temporary working space, increases in size for the biggest skeleton. */
static TArray<FMatrix> CopyBoneTM;

void UUTAnimNodeCopyBoneTranslation::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	Super::GetBoneAtoms(Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);



	TArray<BYTE>&			Reqbones	= RequiredBones;
	TArray<FBoneCopyInfo>&	CopyArray	= ActiveBoneCopyArray;

	// See how many bones we have to post process, if none, then skip this part.
	const INT NumRequiredBones	= Reqbones.Num();
	const INT NumBonesToCopy	= CopyArray.Num();
	if( NumRequiredBones == 0 || NumBonesToCopy == 0 )
	{
		SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
		return;
	}

	const USkeletalMesh*	SkelMesh = SkelComponent->SkeletalMesh;
	const INT				NumBones = SkelMesh->RefSkeleton.Num();

	// We build the mesh-space matrices of the source bones.
	CopyBoneTM.Reset();
	CopyBoneTM.Add(NumBones);

	// Transform required bones from parent space into component space
	for(INT i=0; i<NumRequiredBones; i++)
	{
		const INT BoneIndex = Reqbones(i);

		// transform required bones into component space
		if( BoneIndex == 0 )
		{
			Atoms(BoneIndex).ToTransform(CopyBoneTM(BoneIndex));
		}
		else
		{
			FMatrix LocalBoneTM;
			Atoms(BoneIndex).ToTransform(LocalBoneTM);
			CopyBoneTM(BoneIndex) = LocalBoneTM * CopyBoneTM(SkelMesh->RefSkeleton(BoneIndex).ParentIndex);
		}
	}

	// Post processing, copy bone informations
	for(INT i=0; i<NumBonesToCopy; i++)
	{
		const INT SrcBoneIndex = CopyArray(i).SrcBoneIndex;
		const INT DstBoneIndex = CopyArray(i).DstBoneIndex;

		if( SrcBoneIndex != INDEX_NONE && DstBoneIndex != INDEX_NONE )
		{
			// Copy Mesh Space location of bone
			CopyBoneTM(DstBoneIndex).SetOrigin(CopyBoneTM(SrcBoneIndex).GetOrigin());

			// Transform back to parent bone space
			FMatrix RelTM = CopyBoneTM(DstBoneIndex) * CopyBoneTM(SkelMesh->RefSkeleton(DstBoneIndex).ParentIndex).Inverse();
			const FBoneAtom	TransformedAtom(RelTM);
			Atoms(DstBoneIndex).Rotation	= TransformedAtom.Rotation;
			Atoms(DstBoneIndex).Translation	= TransformedAtom.Translation;

			// At this point the BoneTM Array is not correct anymore for any child bone of DstBoneIndex...
			// Fix if this becomes an issue.
		}
	}

	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}

void UUTAnimNodeSeqWeap::WeapTypeChanged(FName NewAimProfileName)
{
	FName NewSeqName = DefaultAnim;

	if(NewAimProfileName == FName(TEXT("SinglePistol")))
	{
		NewSeqName = SinglePistolAnim;
	}
	else if(NewAimProfileName == FName(TEXT("DualPistols")))
	{
		NewSeqName = DualPistolAnim;
	}
	else if(NewAimProfileName == FName(TEXT("ShoulderRocket")))
	{
		NewSeqName = ShoulderRocketAnim;
	}
	else if(NewAimProfileName == FName(TEXT("Stinger")))
	{
		NewSeqName = StingerAnim;
	}

	SetAnim(NewSeqName);
}

/** So we can tell this node apart - prefix name with 'Weap:' */
FString UUTAnimNodeSeqWeap::GetNodeTitle()
{
	FString SuperTitle = Super::GetNodeTitle();
	return ( FString(TEXT("Weap: ")) + SuperTitle);
}

void UUTAnimNodeJumpLeanOffset::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);

	// Cache pointer to aim node
	TArray<UAnimNode*> Nodes;
	MeshComp->Animations->GetNodesByClass(Nodes, UAnimNodeAimOffset::StaticClass());
	for (INT i=0; i<Nodes.Num(); i++)
	{
		UAnimNodeAimOffset* Node = Cast<UAnimNodeAimOffset>(Nodes(i));
		if( Node && Node->NodeName == FName(TEXT("AimNode")) )
		{
			CachedAimNode = Node;
			break;
		}
	}
}

void UUTAnimNodeJumpLeanOffset::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight  )
{
	// Get current aim profile
	FName AimProfileName = NAME_None;
	if(CachedAimNode)
	{
		FAimOffsetProfile* P = CachedAimNode->GetCurrentProfile();
		if(P)
		{
			AimProfileName = P->ProfileName;
		}
	}

	// Check to see if we are actually Dodging
	if(SkelComponent && SkelComponent->GetOwner() && SkelComponent->GetOwner()->GetAPawn())
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		FLOAT DodgeSpeedThresh = ((POwner->GroundSpeed * 1.5f) + POwner->GroundSpeed) * 0.5f ;

		if(POwner->Physics != PHYS_Falling)
		{
			bDodging = FALSE;
		}
		else if(POwner->Velocity.SizeSquared2D() > DodgeSpeedThresh*DodgeSpeedThresh)
		{
			bDodging = TRUE;
		}
	}

	// Based on WeapType or dodge status, change the current profile.
	if(AimProfileName != OldAimProfileName || bDodging != bOldDodging || bDoubleJumping != bOldDoubleJumping)
	{
		FString NewProfile = AimProfileName.ToString();

		if(bDoubleJumping)
		{
			NewProfile += FString(TEXT("_DBLJUMP"));
		}
		else if(bDodging)
		{
			NewProfile += FString(TEXT("_DODGE"));
		}

		SetActiveProfileByName( FName(*NewProfile) );

		// Update Old vars
		bOldDodging = bDodging;
		bOldDoubleJumping = bDoubleJumping;
		OldAimProfileName = AimProfileName;
	}

	if( BlendTimeToGo != 0.f )
	{
		// Amount we want to change Child2Weight by.
		const FLOAT BlendDelta = LeanWeightTarget - LeanWeight; 

		if( Abs(BlendDelta) > KINDA_SMALL_NUMBER && BlendTimeToGo > DeltaSeconds )
		{
			LeanWeight	+= (BlendDelta / BlendTimeToGo) * DeltaSeconds;
			BlendTimeToGo -= DeltaSeconds;
		}
		else
		{
			LeanWeight = LeanWeightTarget;
			BlendTimeToGo = 0.f;
		}
	}
	
	//debugf(TEXT("Lean: %f"), LeanWeight);

	FVector ActorSpaceVel(0,0,0);
	FVector2D TargetAim(0,0);
	if(SkelComponent && SkelComponent->GetOwner())
	{
		AActor* Owner = SkelComponent->GetOwner();
		ActorSpaceVel = Owner->LocalToWorld().InverseTransformNormal(Owner->Velocity);

		if(bMultiplyByZVelocity)
		{	
			// Y is 'forwards' on our node.
			// We multiply the up velocity as well, so they lean forwards when going up and backwards when coming down
			TargetAim.X = Clamp(ActorSpaceVel.Y * JumpLeanStrength * Owner->Velocity.Z, -1.f, 1.f);
			TargetAim.Y = Clamp(ActorSpaceVel.X * JumpLeanStrength * Owner->Velocity.Z, -1.f, 1.f);
		}
		else
		{
			// Y is 'forwards' on our node.
			TargetAim.X = Clamp(ActorSpaceVel.Y * JumpLeanStrength, -1.f, 1.f);
			TargetAim.Y = Clamp(ActorSpaceVel.X * JumpLeanStrength, -1.f, 1.f);
		}

		// Limit how quickly Aim can change
		FVector2D Delta = TargetAim - PreBlendAim;
		FLOAT MaxAimChange = DeltaSeconds * MaxLeanChangeSpeed;
		Delta.X = Clamp(Delta.X, -MaxAimChange, MaxAimChange);
		Delta.Y = Clamp(Delta.Y, -MaxAimChange, MaxAimChange);

		// Update aim direction
		PreBlendAim += Delta;

		// Blend with 0,0 as desired
		Aim.X = Lerp(0.f, PreBlendAim.X, LeanWeight);
		Aim.Y = Lerp(0.f, PreBlendAim.Y, LeanWeight);
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UUTAnimNodeJumpLeanOffset::SetLeanWeight( FLOAT WeightTarget, FLOAT BlendTime )
{
	LeanWeightTarget = Clamp<FLOAT>(WeightTarget, 0.f, 1.f);

	//debugf(TEXT("Target: %f Time : %f"), WeightTarget, BlendTime);

	// If we want this weight NOW - update weights straight away (dont wait for TickAnim).
	if( BlendTime <= 0.0f )
	{
		LeanWeight = LeanWeightTarget;
	}

	BlendTimeToGo = BlendTime;
}

void UUTAnimBlendBySlotActive::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	Super::InitAnim(MeshComp, Parent);
	check(Children.Num()==2);
	ChildSlot = Cast<UAnimNodeSlot>(Children(1).Anim);
}

void UUTAnimBlendBySlotActive::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if(ChildSlot && (ChildSlot->bIsPlayingCustomAnim || ChildSlot->BlendTimeToGo > KINDA_SMALL_NUMBER))
	{
		if(Child2WeightTarget < 0.5f)
		{
			SetBlendTarget(1.f, 0.f);
		}
	}
	else
	{
		if(Child2WeightTarget > 0.5f)
		{
			SetBlendTarget(0.f, 0.f);
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

