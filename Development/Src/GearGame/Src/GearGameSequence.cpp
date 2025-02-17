//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "GearGame.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"

#include "GearGameSequenceClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"

IMPLEMENT_CLASS(USeqAct_RemoveDroppedPickups)

IMPLEMENT_CLASS(USeqAct_AIFactory)
IMPLEMENT_CLASS(USeqAct_AIMove)
IMPLEMENT_CLASS(USeqAct_AIGrapple)

IMPLEMENT_CLASS(USeqAct_GetPlayerTeam);
IMPLEMENT_CLASS(USeqAct_GetTeammate);

IMPLEMENT_CLASS(USeqAct_PrecacheResources);

IMPLEMENT_CLASS(USeqAct_RainCloudTurnOnOff);
IMPLEMENT_CLASS(USeqAct_RainDropsGroundTurnOnOff);
IMPLEMENT_CLASS(USeqAct_RainDropsSelfTurnOnOff);

IMPLEMENT_CLASS(USeqAct_GearPlayerAnim)
IMPLEMENT_CLASS(USeqAct_Speak)
IMPLEMENT_CLASS(USeqAct_TravelMap);
IMPLEMENT_CLASS(USeqAct_GearUseCommLink)
IMPLEMENT_CLASS(USeqAct_DummyWeaponFire)
IMPLEMENT_CLASS(USeqAct_TriggerGUDSEvent)
IMPLEMENT_CLASS(USeqAct_SetGUDSFrequency)
IMPLEMENT_CLASS(USeqAct_NotifyCoopSplit)
IMPLEMENT_CLASS(USeqAct_ToggleHOD)
IMPLEMENT_CLASS(USeqAct_FlockSpawner)

IMPLEMENT_CLASS(AGearControlHelper)
IMPLEMENT_CLASS(USeqAct_PopupCoverControl)
IMPLEMENT_CLASS(APopupCoverControlHelper)
IMPLEMENT_CLASS(USeqAct_DoorControl)
IMPLEMENT_CLASS(ADoorControlHelper)
IMPLEMENT_CLASS(USeqAct_TriggerTracker)
IMPLEMENT_CLASS(ATriggerTrackerHelper)

IMPLEMENT_CLASS(USeqAct_AIStealthTracker);

IMPLEMENT_CLASS(USeqCond_HasInventory)
IMPLEMENT_CLASS(USeqCond_IsCoop)
IMPLEMENT_CLASS(USeqCond_PawnType)
IMPLEMENT_CLASS(USeqCond_IsStunned)
IMPLEMENT_CLASS(USeqCond_GetDifficulty)
IMPLEMENT_CLASS(USeqCond_HasCOGTag)
IMPLEMENT_CLASS(USeqCond_HasDiscoverable)
IMPLEMENT_CLASS(USeqCond_IsInCover)
IMPLEMENT_CLASS(USeqCond_IsPreviewGrenade)

IMPLEMENT_CLASS(USeqEvt_EnteredRevivalState);
IMPLEMENT_CLASS(USeqEvt_Input);
IMPLEMENT_CLASS(USeqEvt_Interaction);
IMPLEMENT_CLASS(USeqEvt_CoopTouch);
IMPLEMENT_CLASS(USeqEvt_WDODamageModApplied);
IMPLEMENT_CLASS(USeqEvt_EnteredCover);
IMPLEMENT_CLASS(USeqEvt_LeftCover);
IMPLEMENT_CLASS(USeqEvt_ChainsawInteraction);

IMPLEMENT_CLASS(USeqVar_Anya)
IMPLEMENT_CLASS(USeqVar_GenericRemoteSpeaker)
IMPLEMENT_CLASS(USeqVar_Dom)
IMPLEMENT_CLASS(USeqVar_Marcus)

IMPLEMENT_CLASS(USeqAct_PlayCinematic); // is this deprecated
IMPLEMENT_CLASS(USeqAct_ControlGearsMovie);
IMPLEMENT_CLASS(USeqAct_WaitForGearsMovieComplete);

IMPLEMENT_CLASS(USeqAct_StreamByURL);

IMPLEMENT_CLASS(USeqAct_StoreVariable);
IMPLEMENT_CLASS(USeqAct_RetrieveVariable);

IMPLEMENT_CLASS(USeqAct_KillPlayers);
IMPLEMENT_CLASS(USeqAct_ModifyProperty);
IMPLEMENT_CLASS(ADummyWeaponFireActor);
IMPLEMENT_CLASS(USeqAct_ToggleConversationCamera)
IMPLEMENT_CLASS(USeqAct_ToggleConversation)
IMPLEMENT_CLASS(USeqAct_CauseExplosion)
IMPLEMENT_CLASS(USeqAct_SpectatorCameraPath)
IMPLEMENT_CLASS(USeqAct_SetWeather)
IMPLEMENT_CLASS(USeqAct_CringePawn)
IMPLEMENT_CLASS(USeqAct_BargeAddRocking)

IMPLEMENT_CLASS(USeqAct_DrawMessage)

IMPLEMENT_CLASS(USeqAct_Mutate)

IMPLEMENT_CLASS(USeqAct_Skorge_ChargeAndDuel)
IMPLEMENT_CLASS(USeqAct_BrumakControl)
IMPLEMENT_CLASS(USeqAct_Leviathan_Mouth)

IMPLEMENT_CLASS(USeqEvt_GearTouch)

IMPLEMENT_CLASS(USeqAct_ToggleGUDSStreaming)

IMPLEMENT_CLASS(USeqAct_CineCleanWorldPre)
IMPLEMENT_CLASS(USeqAct_CineCleanWorldPost)


void USeqAct_RemoveDroppedPickups::Activated()
{
	for (FDynamicActorIterator It; It; ++It)
	{
		AGearDroppedPickup *Pickup = Cast<AGearDroppedPickup>(*It);
		if (Pickup != NULL)
		{
			Pickup->eventRedrop();
		}
	}
}

/** 
 * Internal.  Given a list of UObject vars, build a list of associated GearPCs.  Will search for
 * GearPCs, as well as Pawns with GearPC controllers. Useful for handling Kismet actions.
 */
static void BuildGearPCTargets(TArray<UObject**> const& ObjVars, TArray<AGearPC*> &PCTargets)
{
	INT const NumObjVars = ObjVars.Num();

	for (INT ObjIdx=0; ObjIdx<NumObjVars; ++ObjIdx)
	{
		UObject* const ObjVar = *(ObjVars(ObjIdx));

		// looking for GearPCs here
		AGearPC* GPC = Cast<AGearPC>(ObjVar);
		if (GPC == NULL)
		{
			// see if we have a GearPawn that might in turn have a GearPC
			AGearPawn* const GearPawn = Cast<AGearPawn>(ObjVar);
			if (GearPawn)
			{
				GPC = Cast<AGearPC>(GearPawn->Controller);
			}
		}

		if (GPC != NULL)
		{
			PCTargets.AddItem(GPC);
		}
	}
}

/** 
 * Internal.  Given a list of UObject vars, build a list of associated GearPawns.  Will handle
 * indirect pawn refs, e.g. Controllers with associated Pawns. Useful for handling Kismet actions.
 */
static void BuildGearPawnTargets(TArray<UObject**> const& ObjVars, TArray<AGearPawn*> &PawnTargets)
{
	INT const NumObjVars = ObjVars.Num();

	for (INT ObjIdx=0; ObjIdx<NumObjVars; ++ObjIdx)
	{
		UObject* const ObjVar = *(ObjVars(ObjIdx));

		// looking for GearPawns here
		AGearPawn* GP = Cast<AGearPawn>(ObjVar);
		if (GP == NULL)
		{
			// see if we have a Controller that might be controlling a GearPawn
			AController* const Controller = Cast<AController>(ObjVar);
			if (Controller)
			{
				GP = Cast<AGearPawn>(Controller->Pawn);
			}
		}

		if (GP != NULL)
		{
			PawnTargets.AddItem(GP);
		}
	}
}

/** Given a target actor, try to find an associated GearPawn to use. */
static AGearPawn* GetGearPawnTarget(UObject* Obj)
{
	AGearPawn* TargetGP = Cast<AGearPawn>(Obj);

	if (TargetGP == NULL)
	{
		// maybe it's a controller
		AController* const Controller = Cast<AController>(Obj);
		if (Controller)
		{
			AVehicle* Vehicle = Cast<AVehicle>(Controller->Pawn);
			TargetGP = Vehicle ? Cast<AGearPawn>(Vehicle->Driver) : Cast<AGearPawn>(Controller->Pawn);
		}
		else
		{
			AVehicle* Vehicle = Cast<AVehicle>(Obj);
			if (Vehicle != NULL)
			{
				TargetGP = Cast<AGearPawn>(Vehicle->Driver);
			}
		}
	}

	return TargetGP;
}




void USeqAct_Mutate::SetTheHardReferences()
{
	PawnClass = FindObject<UClass>( ANY_PACKAGE, *(MutateClassNames((INT)MutateType)) );
}

void USeqAct_Mutate::PostLoad()
{
	Super::PostLoad();
	SetTheHardReferences();
}

void USeqAct_Mutate::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
	SetTheHardReferences();
}

/**
 * Grabs the list of all attached Objects, and then attempts to import any specified property
 * values for each one.
 */
void USeqAct_ModifyProperty::Activated()
{
	if (Properties.Num() > 0 && Targets.Num() > 0)
	{
		// for each Object found,
		for (INT Idx = 0; Idx < Targets.Num(); Idx++)
		{
			UObject *Obj = Targets(Idx);
			if (Obj != NULL)
			{
				// iterate through each property
				for (INT propIdx = 0; propIdx < Properties.Num(); propIdx++)
				{
					if (Properties(propIdx).bModifyProperty)
					{
						// find the property field
						UProperty *prop = Cast<UProperty>(Obj->FindObjectField(Properties(propIdx).PropertyName,1));
						if (prop != NULL)
						{
							debugf(TEXT("Applying property %s for object %s"),*prop->GetName(),*Obj->GetName());
							// import the property text for the new Object
							prop->ImportText(*(Properties(propIdx).PropertyValue),(BYTE*)Obj + prop->Offset,0,NULL);
						}
						else
						{
							debugf(TEXT("failed to find property in %s"),*Obj->GetName());
							// auto-add the pawn if property wasn't found on the controller
							if (Cast<AController>(Obj) != NULL)
							{
								Targets.AddUniqueItem(Cast<AController>(Obj)->Pawn);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		debugf(TEXT("no properties/targets %d"),Targets.Num());
	}
}

void USeqAct_ModifyProperty::CheckForErrors()
{
	Super::CheckForErrors();

	if (GWarn != NULL && GWarn->MapCheck_IsActive())
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, NULL, *FString::Printf(TEXT("\"Modify Property\" is for prototyping only and should be removed (%s)"), *GetPathName()));
	}
}

void USeqCond_IsInCover::Activated()
{
	Super::Activated();
	TArray<UObject**> ObjVars;
	GetObjectVars(ObjVars,TEXT("Players"));
	UBOOL bAllPlayersAreInCover = TRUE;
	for( INT Idx = 0; Idx < ObjVars.Num(); Idx++ )
	{
		AGearPawn *GP = Cast<AGearPawn>(*(ObjVars(Idx)));
		if (GP == NULL)
		{
			AController *C = Cast<AController>(*(ObjVars(Idx)));
			if( C != NULL )
			{
				GP = Cast<AGearPawn>(C->Pawn);
			}
		}
		if (GP != NULL)
		{
			bAllPlayersAreInCover = bAllPlayersAreInCover && GP->CoverType != CT_None;
			if (!bAllPlayersAreInCover)
			{
				break;
			}
		}
	}
	OutputLinks(bAllPlayersAreInCover?0:1).ActivateOutputLink();
}

void USeqCond_IsPreviewGrenade::Activated()
{
	Super::Activated();

	TArray<UObject**> ObjVars;
	GetObjectVars(ObjVars,TEXT("TestObject"));

	// MaxVars=1, so only testing first element in array
	UBOOL bIsPreviewGrenade = FALSE;
	if (ObjVars.Num() > 0)
	{
		AGearProj_Grenade* const Grenade = Cast<AGearProj_Grenade>(*(ObjVars(0)));
		if (Grenade && Grenade->bIsSimulating)
		{
			bIsPreviewGrenade = TRUE;
		}
	}

	OutputLinks(bIsPreviewGrenade?0:1).ActivateOutputLink();
}


void USeqCond_GetDifficulty::Activated()
{
	INT DifficultyLevel = DL_MAX;
	for (AController *Player = GWorld->GetFirstController(); Player != NULL; Player = Player->NextController)
	{
		AGearPC *WPC = Cast<AGearPC>(Player);
		if (WPC != NULL)
		{
			INT PlayerDifficultyLevel = WPC->eventGetCurrentDifficultyLevel();
			if (PlayerDifficultyLevel < DifficultyLevel)
			{
				DifficultyLevel = PlayerDifficultyLevel;
			}
		}
	}
	if (DifficultyLevel > OutputLinks.Num())
	{
		DifficultyLevel = 0;
	}
	OutputLinks(DifficultyLevel).ActivateOutputLink();
}

static APawn* FindPawn(UClass* PawnClass)
{
	for (APawn *Pawn = GWorld->GetWorldInfo()->PawnList; Pawn != NULL; Pawn = Pawn->NextPawn)
	{
		if (Pawn->IsA(PawnClass))
		{
			// if the Pawn is in a vehicle, return the vehicle instead
			return (Pawn->DrivenVehicle != NULL) ? Pawn->DrivenVehicle : Pawn;
		}
	}
	return NULL;
}

UObject** USeqVar_Marcus::GetObjectRef(INT Idx)
{
	if (Idx == 0)
	{
		if( GWorld != NULL )
		{
			APawn* Pawn = FindPawn(AGearPawn_COGMarcus::StaticClass());
			if (Pawn != NULL)
			{
				ObjValue = Pawn;
				// use the controller if possible, otherwise default to the pawn
				if (Pawn->Controller != NULL)
				{
					ObjValue = Pawn->Controller;
				}
			}
			else
			{
				ObjValue = NULL;
				KISMET_WARN(TEXT("Failed to find Marcus for scripting! %s"), *GetName());
			}
			if (ObjValue != NULL)
			{
				return &ObjValue;
			}
		}
	}
	return NULL;
}

UObject** USeqVar_Dom::GetObjectRef(INT Idx)
{
	if (Idx == 0)
	{
		if( GWorld != NULL )
		{
			APawn* Pawn = FindPawn(AGearPawn_COGDom::StaticClass());
			if (Pawn != NULL)
			{
				ObjValue = Pawn;
				// use the controller if possible, otherwise default to the pawn
				if (Pawn->Controller != NULL)
				{
					ObjValue = Pawn->Controller;
				}
			}
			else
			{
				ObjValue = NULL;
				KISMET_WARN(TEXT("Failed to find Dom for scripting! %s"), *GetName());
			}
			if (ObjValue != NULL)
			{
				return &ObjValue;
			}
		}
	}
	return NULL;
}

/**
 * Looks for the first attached actor with a team and then writes it out to
 * all attached int vars.
 */
void USeqAct_GetPlayerTeam::Activated()
{
	USequenceOp::Activated();
	TArray<UObject**> PlayerVars;
	GetObjectVars(PlayerVars,TEXT("Target"));
	UBOOL bFoundTeam = FALSE;
	BYTE TeamNum = 255;
	for (INT Idx = 0; Idx < PlayerVars.Num() && !bFoundTeam; Idx++)
	{
		AActor *Actor = Cast<AActor>(*(PlayerVars(Idx)));
		if (Actor != NULL)
		{
			TeamNum = Actor->GetTeamNum();
			bFoundTeam = TRUE;
		}
	}
	if (bFoundTeam)
	{
		// write out the team to attached variables
		TArray<INT*> TeamVars;
		GetIntVars(TeamVars,TEXT("Team"));
		for (INT Idx = 0; Idx < TeamVars.Num(); Idx++)
		{
			*(TeamVars(Idx)) = (INT)TeamNum;
		}
	}
}

/**
 * Overridden to get the closest teammates to the given player.
 */
void USeqAct_GetTeammate::Activated()
{
	USequenceOp::Activated();
	/*
	UBOOL bAssignedMember = 0;
	TArray<UObject**> playerVars;
	GetObjectVars(playerVars,TEXT("Player"));
	TArray<UObject**> teamVars;
	GetObjectVars(teamVars,TEXT("Teammate"));
	if (playerVars.Num() > 0 &&
		teamVars.Num() > 0)
	{
		// get the player's team
		AController *player = Cast<AController>(*playerVars(0));
		if (player == NULL)
		{
			APawn *pawn = Cast<APawn>(*playerVars(0));
			if (pawn != NULL)
			{
				player = pawn->Controller;
			}
		}
		KISMET_LOG(TEXT("Getting teammates for %s"),player!=NULL?player->GetName():TEXT("NULL"));
		if (player != NULL &&
			player->Pawn != NULL &&
			player->PlayerReplicationInfo != NULL)
		{
			AGearTeamInfo *team = Cast<AGearTeamInfo>(player->PlayerReplicationInfo->Team);
		}
	}
	// activate the proper output based on whether or not we assigned a teammate
	INT LinkIdx = (bAssignedMember?0:1);
	if( !OutputLinks(LinkIdx).bDisabled )
	{
		OutputLinks(LinkIdx).bHasImpulse = TRUE;
	}
	*/
}

void USeqAct_TravelMap::Activated()
{
	USequenceOp::Activated();

	FString TravelString = MapName + TEXT( "?" ) + Parameters; //TEXT( "ChapterStartPoint=38" ) ;
	//warnf( TEXT( "TravelString: %s " ), *TravelString );
	/*
	// grab the player
	for( FActorIterator It; It; ++It )
	{
		AGearPC *pc = Cast<AGearPC>(*It);
		if (pc != NULL)
		{	
			// add on the player loadout selection
			//@fixme - laurent replace this with whatever the actual loadout is
			travelString = FString::Printf(TEXT("%s?Primary=%s?Secondary=%s?Cash=%d"),
										   *travelString,
										   pc->PrimaryWeaponClass!=NULL?pc->PrimaryWeaponClass->GetName():TEXT("None"),
										   pc->SecondaryWeaponClass!=NULL?pc->SecondaryWeaponClass->GetName():TEXT("None"),
										   0);
			break;
		}
	}
	*/
	// set the pending URL
	GEngine->SetClientTravel( *TravelString, TRAVEL_Absolute );
}

UBOOL USeqEvt_CoopTouch::CheckUnTouchActivate(AActor *inOriginator, AActor *InInstigator, UBOOL bTest)
{
	// See if we're tracking the instigator, not the actual actor that caused the touch event.
	if( bUseInstigator )
	{
		AProjectile *Proj = Cast<AProjectile>(InInstigator);
		if( Proj && Proj->Instigator )
		{
			InInstigator = Proj->Instigator;
		}
	}

	// just remove the entry in the touched list
	INT TouchIdx = -1;
	while (TouchedList.FindItem(InInstigator,TouchIdx))
	{
		TouchedList.Remove(TouchIdx,1);
	}

	return FALSE;
}

UBOOL USeqEvt_CoopTouch::CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest, TArray<INT>* ActivateIndices, UBOOL bPushTop)
{
	KISMET_LOG(TEXT("Coop touch event %s check activate %s/%s %d"),*GetName(),*InOriginator->GetName(),*InInstigator->GetName(),bTest);
	// check class proximity types, accept any if no proximity types defined
	UBOOL bPassed = FALSE;
	if (bEnabled &&
		InInstigator != NULL)
	{
		if (ClassProximityTypes.Num() > 0)
		{
			for (INT Idx = 0; Idx < ClassProximityTypes.Num() && !bPassed; Idx++)
			{
				if (InInstigator->IsA(ClassProximityTypes(Idx)))
				{
					bPassed = TRUE;
				}
			}
			// make sure we're touching everybody
			if (bPassed)
			{
				for (INT Idx = 0; Idx < ClassProximityTypes.Num(); Idx++)
				{
					// first check the instigator
					if (!InInstigator->IsA(ClassProximityTypes(Idx)))
					{
						// check the touching list
						UBOOL bFoundTouch = FALSE;
						for (INT TouchIdx = 0; TouchIdx < TouchedList.Num(); TouchIdx++)
						{
							if (TouchedList(TouchIdx) != NULL &&
								TouchedList(TouchIdx)->IsA(ClassProximityTypes(Idx)))
							{
								bFoundTouch = TRUE;
								break;
							}
						}
						// not an instigator, and not found in touched list
						if (!bFoundTouch)
						{
							KISMET_LOG(TEXT("Failed to find touch for class %s"),*ClassProximityTypes(Idx)->GetName());
							// abort
							bPassed = FALSE;
							// but add them to the list
							TouchedList.AddUniqueItem(InInstigator);
							break;
						}
					}
				}
			}
		}
		else
		{
			bPassed = TRUE;
		}
		if (bPassed)
		{
			// check the base activation parameters, test only
			bPassed = USequenceEvent::CheckActivate(InOriginator,InInstigator,TRUE,ActivateIndices,bPushTop);
			KISMET_LOG(TEXT("- passed? %s"),bPassed?TEXT("yup"):TEXT("nope"));
		}
	}
	return bPassed;
}

/**
 * Verifies various interaction checks, and then passes up to the normal event checks.
 */
UBOOL USeqEvt_Interaction::CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest, TArray<INT>* ActivateIndices, UBOOL bPushTop)
{
	if (!bEnabled)
	{
		KISMET_LOG(TEXT("%s - Failed enabled"),*GetPathName());
		return FALSE;
	}
	// first validate the instigator
	APawn *Pawn = Cast<APawn>(InInstigator);
	if (Pawn == NULL || Pawn->Controller == NULL)
	{
		KISMET_LOG(TEXT("%s - Failed instigator"),*GetPathName());
		// invalid instigator
		return FALSE;
	}
	FVector InstigatorToOriginatorDir = (InOriginator->Location - InInstigator->Location);
	// next validate the interact distance
	if (bCheckInteractDistance && 
		InstigatorToOriginatorDir.Size() > InteractDistance)
	{
		KISMET_LOG(TEXT("%s - Failed distance"),*GetPathName());
		// failed distance
		return FALSE;
	}
	// next validate the interact FOV
	if (bCheckInteractFOV)
	{
		//@fixme - slight hack, bypass the fov check if already chainsawing
		AGearPawn *GP = Cast<AGearPawn>(InInstigator);
		if (GP == NULL || GP->SpecialMove != SM_ChainSawAttack_Object)
		{
			// get the viewpoint
			FVector CamLoc = Pawn->eventGetPawnViewLocation();
			FRotator CamRot = Pawn->Rotation;
			//Pawn->Controller->eventGetPlayerViewPoint(CamLoc,CamRot);
			FVector CamDir = CamRot.Vector();
			CamDir.Z = 0.f;
			CamDir.SafeNormal();

			FVector InstToOrig2D = InstigatorToOriginatorDir;
			InstToOrig2D.Z = 0.f;
			InstToOrig2D.SafeNormal();

			if ((InstToOrig2D | CamDir) < 1.f - InteractFOV)
			{
				KISMET_LOG(TEXT("%s - Failed FOV %d"),*GetPathName(),bTest);
				// failed FOV
				return FALSE;
			}
		}
	}
	// passed our checks, check the normal ones now
	return Super::CheckActivate(InOriginator,InInstigator,bTest,ActivateIndices,bPushTop);
}

//@fixme - refactor this out into a registerwithplayer function
UBOOL USeqEvt_EnteredCover::RegisterEvent()
{
	// register with the local player
	AGearPC *PC = NULL;
	for (INT Idx = 0; Idx < GEngine->GamePlayers.Num() && PC == NULL; Idx++) 
	{
		if (GEngine->GamePlayers(Idx) != NULL &&
			GEngine->GamePlayers(Idx)->Actor != NULL &&
			GEngine->GamePlayers(Idx)->Actor->LocalPlayerController())
		{
			PC = Cast<AGearPC>(GEngine->GamePlayers(Idx)->Actor);
		}
	}
	if (PC != NULL)
	{
		KISMET_LOG(TEXT("Entered cover event %s registering with player %s"),*GetFullName(),*PC->GetName());
		Originator = PC;
		PC->EnteredCoverEvents.AddUniqueItem(this);
		eventRegisterEvent();
		bRegistered = TRUE;
	}
	
	return bRegistered;
}

UBOOL USeqEvt_LeftCover::RegisterEvent()
{
	// register with the local player
	AGearPC *PC = NULL;
	for (INT Idx = 0; Idx < GEngine->GamePlayers.Num() && PC == NULL; Idx++) 
	{
		if (GEngine->GamePlayers(Idx) != NULL &&
			GEngine->GamePlayers(Idx)->Actor != NULL &&
			GEngine->GamePlayers(Idx)->Actor->LocalPlayerController())
		{
			PC = Cast<AGearPC>(GEngine->GamePlayers(Idx)->Actor);
		}
	}
	if (PC != NULL)
	{
		KISMET_LOG(TEXT("Left cover event %s registering with player %s"),*GetFullName(),*PC->GetName());
		Originator = PC;
		PC->LeftCoverEvents.AddUniqueItem(this);
		eventRegisterEvent();
		bRegistered = TRUE;
	}
	
	return bRegistered;
}

UBOOL USeqEvt_Input::RegisterEvent()
{
	if (!bRegistered)
	{
		TArray<AGearPC*> TargetPlayers;
		// register with the local player
		AGearPC *PC = NULL;
		for (INT Idx = 0; Idx < GEngine->GamePlayers.Num() && PC == NULL; Idx++) 
		{
			if (GEngine->GamePlayers(Idx) != NULL &&
				GEngine->GamePlayers(Idx)->Actor != NULL &&
				GEngine->GamePlayers(Idx)->Actor->LocalPlayerController())
			{
				PC = Cast<AGearPC>(GEngine->GamePlayers(Idx)->Actor);
				switch (RegisterType)
				{
				case IRT_AllPlayers:
					TargetPlayers.AddItem(PC);
					break;
				case IRT_MarcusOnly:
					if (Cast<AGearPawn_COGMarcus>(PC) != NULL)
					{
						TargetPlayers.AddItem(PC);
					}
					break;
				case IRT_DomOnly:
					if (Cast<AGearPawn_COGDom>(PC) != NULL)
					{
						TargetPlayers.AddItem(PC);
					}
					break;
				case IRT_COGOnly:
					if (PC->GetTeamNum() == 0)
					{
						TargetPlayers.AddItem(PC);
					}
					break;
				case IRT_LocustOnly:
					if (PC->GetTeamNum() == 1)
					{
						TargetPlayers.AddItem(PC);
					}
					break;
				}
			}
		}
		if (TargetPlayers.Num() > 0)
		{
			for (INT Idx = 0; Idx < TargetPlayers.Num(); Idx++)
			{
				PC = TargetPlayers(Idx);
				// create a duplicate of the event to avoid collision issues for multiple targets
				// create a unique name prefix so that we can guarantee no collisions with any other creation of Kismet objects (for serializing Kismet)
				FName DupName = MakeUniqueObjectName(ParentSequence, GetClass(), FName(*FString::Printf(TEXT("%s_InputDup"), *GetName())));
				USeqEvt_Input *Evt = (USeqEvt_Input*)StaticConstructObject(GetClass(),ParentSequence,DupName,RF_Transient,this);
				if ( ParentSequence->AddSequenceObject(Evt) )
				{
					KISMET_LOG(TEXT("Input event %s registering with player %s"),*Evt->GetFullName(),*PC->GetName());
					DuplicateEvts.AddItem(Evt);
					Evt->Originator = PC;
					PC->InputEvents.AddItem(Evt);
					Evt->bRegistered = TRUE;
					Evt->eventRegisterEvent();
				}
			}
			bRegistered = TRUE;
		}
	}
	return bRegistered;
}


UBOOL USeqEvt_Input::CheckInputActivate(BYTE Button,UBOOL bPressed)
{
	if (bEnabled)
	{
		// look for a button match
		UBOOL bHasMatchingButton = FALSE;
		for (INT Idx = 0; Idx < ButtonNames.Num(); Idx++)
		{
			if (ButtonNames(Idx) == Button)
			{
				bHasMatchingButton = TRUE;
				break;
			}
		}
		TArray<INT> ActivateIndices;
		ActivateIndices.AddItem(bPressed?0:1);
		// if we found a match and we are pass the normal activation rules
		if (bHasMatchingButton &&
			CheckActivate(Originator,Originator,0,&ActivateIndices))
		{
			// write out the button text
			TArray<FString*> StringVars;
			GetStringVars(StringVars,TEXT("Button"));
			if (StringVars.Num() > 0)
			{
				UEnum *EGameButtonsEnum = FindObject<UEnum>(ANY_PACKAGE,TEXT("EGameButtons"));
				if (EGameButtonsEnum != NULL)
				{
					for (INT Idx = 0; Idx < StringVars.Num(); Idx++)
					{
						*(StringVars(Idx)) = EGameButtonsEnum->GetEnum(Button).ToString();
					}
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}


UBOOL USeqEvt_EnteredRevivalState::CheckActivate( AActor *InOriginator, AActor *InInstigator, UBOOL bTest, TArray<INT>* ActivateIndices, UBOOL bPushTop )
{
	UBOOL bResult = Super::CheckActivate( InOriginator, InInstigator, bTest, ActivateIndices, bPushTop );

	if( bResult && bEnabled && !bTest )
	{
		AGearPawn* Inv = Cast<AGearPawn>(InInstigator);
		if( Inv )
		{
			// see if any victim variables are attached
			TArray<UObject**> InvVars;
			GetObjectVars(InvVars,TEXT("Pawn"));
			for (INT Idx = 0; Idx < InvVars.Num(); Idx++)
			{
				*(InvVars(Idx)) = Inv;
			}
		}
		else
		{
			bResult = 0;
		}
	}

	return bResult;
}


/** 
 * Sequence action to precache all viewport clients upon activation.
 */
void USeqAct_PrecacheResources::Activated()
{
	USequenceOp::Activated();
	// Precache all viewport clients.
	for( INT ViewportIndex=0; ViewportIndex<GEngine->GamePlayers.Num(); ViewportIndex++ )
	{
		ULocalPlayer* Player = GEngine->GamePlayers(ViewportIndex);
		if( Player && Player->ViewportClient )
		{
			Player->ViewportClient->Precache();
		}
	}
}


void USeqAct_PlayCinematic::Activated()
{
	USequenceOp::Activated();

	TArray<UObject**> PlayerVars;
	GetObjectVars(PlayerVars,TEXT("Target"));

	for (INT Idx = 0; Idx < PlayerVars.Num(); Idx++)
	{
		AGearPC* PC = Cast<AGearPC>(*(PlayerVars(Idx)));
		if( PC != NULL )
		{
			PC->eventShowCinematic();
		}
	}
}


/**
* Returns the first wave node in the given sound node.
*/
static USoundNodeWave* FindFirstWaveNode(USoundNode *rootNode)
{
	USoundNodeWave* waveNode = NULL;
	TArray<USoundNode*> chkNodes;
	chkNodes.AddItem(rootNode);
	while (waveNode == NULL &&
		chkNodes.Num() > 0)
	{
		USoundNode *node = chkNodes.Pop();
		if (node != NULL)
		{
			waveNode = Cast<USoundNodeWave>(node);
			for (INT Idx = 0; Idx < node->ChildNodes.Num() && waveNode == NULL; Idx++)
			{
				chkNodes.AddItem(node->ChildNodes(Idx));
			}
		}
	}
	return waveNode;
}

/**
 * Used to refresh the TTS data
 */
void USeqAct_Speak::PostEditChange( UProperty* PropertyThatChanged )
{
	if( PropertyThatChanged->GetName() == TEXT( "bUseTTS" )
		|| PropertyThatChanged->GetName() == TEXT( "TTSSpeaker" )
		|| PropertyThatChanged->GetName() == TEXT( "TTSSpokenText" ) )
	{
		bPCMGenerated = FALSE;
	}
}

/**
 * Creates the required data for TTS to work
 */
USoundCue* USeqAct_Speak::InitSpeakSoundCue( void )
{
#if WITH_TTS
	if( bUseTTS )
	{
		if( TTSSoundCue == NULL )
		{
			TTSSoundCue = ConstructObject<USoundCue>( USoundCue::StaticClass(), GetTransientPackage() );

			USoundNodeWave* SoundNodeWaveTTS = ConstructObject<USoundNodeWave>( USoundNodeWave::StaticClass(), TTSSoundCue );
			SoundNodeWaveTTS->bUseTTS = TRUE;
			SoundNodeWaveTTS->TTSSpeaker = TTSSpeaker;
			SoundNodeWaveTTS->SpokenText = TTSSpokenText;

			USoundNode* AttenuationNode = ConstructObject<USoundNode>( USoundNodeAttenuation::StaticClass(), TTSSoundCue );

			// Link the attenuation node to the wave.
			AttenuationNode->CreateStartingConnectors();
			AttenuationNode->ChildNodes( 0 ) = SoundNodeWaveTTS;

			// Link the attenuation node to root.
			TTSSoundCue->FirstNode = AttenuationNode;
		}

		// Expand the TTS data if required
		if( !bPCMGenerated )
		{
			if( GEngine && GEngine->Client )
			{
				UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
				if( AudioDevice )
				{
					USoundNodeWave* SoundNodeWaveTTS = ( USoundNodeWave* )TTSSoundCue->FirstNode->ChildNodes( 0 );
					debugf( NAME_DevAudio, TEXT( "Building TTS PCM data for '%s'" ), *TTSSoundCue->GetName() );
					AudioDevice->TextToSpeech->CreatePCMData( SoundNodeWaveTTS );
				}
			}
			bPCMGenerated = TRUE;
		}

		return( TTSSoundCue );
	}
#endif
	return( PlaySound );
}


AActor* USeqAct_Speak::GetSpeaker(AGearPawn*& PawnSpeaker, AGearRemoteSpeaker*& RemoteSpeaker, AVehicle_RideReaver_Base*& ReaverSpeaker) const
{
	PawnSpeaker = NULL;
	RemoteSpeaker = NULL;
	ReaverSpeaker = NULL;

	TArray<UObject**> SpeakerVars;
	GetObjectVars(SpeakerVars, TEXT("Speaker"));

	if (SpeakerVars.Num() > 0)
	{
		AActor* SpeakerActor = Cast<AActor>(*(SpeakerVars(0)));

		if (SpeakerActor)
		{
			// is it a pawn?
			PawnSpeaker = GetGearPawnTarget(SpeakerActor);
			if (PawnSpeaker)
			{
				SpeakerActor = PawnSpeaker;
			}
			else if (PawnSpeaker == NULL)
			{
				// couldn't find a pawn, see what else we can handle
				// remote speaker maybe?
				RemoteSpeaker = Cast<AGearRemoteSpeaker>(SpeakerActor);
				if (RemoteSpeaker == NULL)
				{
					// is a reaver? (least likely case)
					ReaverSpeaker = Cast<AVehicle_RideReaver_Base>(SpeakerActor);
					if (ReaverSpeaker == NULL)
					{
						// couldn't find an appropriate speaker!
						SpeakerActor = NULL;
					}
				}
			}
		}

		return SpeakerActor;
	}

	return NULL;
}

void USeqAct_Speak::ForceStopSpeech(USoundCue const* ActiveSoundCue)
{
	AGearPawn* PawnSpeaker = NULL;
	AGearRemoteSpeaker* RemoteSpeaker = NULL;
	AVehicle_RideReaver_Base* RideReaver = NULL;

	AActor* const SpeakerActor = GetSpeaker(PawnSpeaker, RemoteSpeaker, RideReaver);
	if (SpeakerActor)
	{
		// search for all audio components playing our SoundCue
		TArray<UAudioComponent*> ComponentsToStop;
		for (INT CompIdx = 0; CompIdx < SpeakerActor->Components.Num(); CompIdx++)
		{
			UAudioComponent* const AC = Cast<UAudioComponent>(SpeakerActor->Components(CompIdx));
			if ( AC && (AC->SoundCue == ActiveSoundCue) )
			{
				ComponentsToStop.AddUniqueItem(AC);
			}
		}

		// Then tell them to fade-out/stop
		for (INT CompIdx = 0; CompIdx < ComponentsToStop.Num(); CompIdx++)
		{
			// Will call 'Stop' when it's faded out, which will remove itself from Actors component array.
			ComponentsToStop(CompIdx)->FadeOut(FadeOutTime, 0.2f); 

			// need to stop facefx too?
			if (PawnSpeaker && (PawnSpeaker->FacialAudioComp == ComponentsToStop(CompIdx)) && PawnSpeaker->Mesh && PawnSpeaker->Mesh->IsPlayingFaceFXAnim())
			{
				// we're stopping a sound with an active facefx anim, so we need to stop the anim
				PawnSpeaker->Mesh->StopFaceFXAnim();
			}
		}

		SpeechFinished();

	}

	// see if there are any DelayedActivatedOps pending on the finished output of this op
	// handles case where Stop comes in after finished but before delayed output fires
	// in which case we want to avoid the fire.
	FSeqOpOutputLink& FinishedLink = OutputLinks(1);
	for (INT Idx = 0; Idx < ParentSequence->DelayedActivatedOps.Num(); Idx++)
	{
		FActivateOp &DelayedOp = ParentSequence->DelayedActivatedOps(Idx);

		// iterate through all linked inputs looking for linked ops
		for (INT InputIdx = 0; InputIdx < FinishedLink.Links.Num(); InputIdx++)
		{
			if (DelayedOp.Op == FinishedLink.Links(InputIdx).LinkedOp &&
				DelayedOp.InputIdx == FinishedLink.Links(InputIdx).InputLinkIdx)
			{
				// delete it
				ParentSequence->DelayedActivatedOps.Remove(Idx, 1);
				Idx--;
				break;
			}
		}
	}

	SoundDuration = 0.f;
	bStopped = TRUE;
}


void USeqAct_Speak::SpeechFinished()
{
	AGearPawn* PawnSpeaker = NULL;
	AGearRemoteSpeaker* RemoteSpeaker = NULL;
	AVehicle_RideReaver_Base* ReaverSpeaker = NULL;

	AActor* SpeakerActor = GetSpeaker(PawnSpeaker, RemoteSpeaker, ReaverSpeaker);

	// notify speaker he has stopped speaking
	if (PawnSpeaker)
	{
		PawnSpeaker->eventSpeakLineFinished();

		// stop facing
		if (bTurnBodyTowardsAddressee && PawnSpeaker->Controller)
		{
			PawnSpeaker->Controller->bForceDesiredRotation = FALSE;
		}
	}
	else if (RemoteSpeaker)
	{
		RemoteSpeaker->eventRemoteSpeakLineFinished();
	}
}

/**
 * Plays the given soundcue on all attached actors.
 */
void USeqAct_Speak::Activated()
{
	bStopped = FALSE;
	ExtraDelayStartTime = 0.f;

	USoundCue* ActiveSoundCue = InitSpeakSoundCue();

	if (ActiveSoundCue != NULL)
	{
		// the "play" input
		if (InputLinks(0).bHasImpulse || DBNOSpeakerToWaitFor != NULL)
		{
			// clear waiting DBNO
			DBNOSpeakerToWaitFor = NULL;

			AGearPawn* Speaker = NULL;
			AGearRemoteSpeaker* RemoteSpeaker = NULL;
			AVehicle_RideReaver_Base* RideReaver = NULL;
			AActor* SpeakerActor = NULL;

			// get our speaker
			SpeakerActor = GetSpeaker(Speaker, RemoteSpeaker, RideReaver);

			if (SpeakerActor == NULL)
			{
				// can't do much without a speaker, bail
				return;
			}

			// get our addressee
			TArray<UObject**> SpeakingToVars;
			AGearPawn* Addressee = NULL;

			GetObjectVars(SpeakingToVars, TEXT("SpeakingTo"));

			if (SpeakingToVars.Num() > 0)
			{
				Addressee = Cast<AGearPawn>(*(SpeakingToVars(0)));
				if (Addressee == NULL)
				{
					// check and see if it's a controller, in which case we should get the pawn
					AController *Controller = Cast<AController>(*(SpeakingToVars(0)));
					if (Controller)
					{
						Addressee = Cast<AGearPawn>(Controller->Pawn);
					}
				}
			}

			bSpokenLineACHasStartedPlaying = FALSE;

			if(RideReaver)
			{
				RideReaver->eventDriverSpeakLine(ActiveSoundCue, bSuppressSubtitles);
				KISMET_LOG(TEXT("- GearRideReaver %s speaking cue %s"),*Speaker->GetName(),*ActiveSoundCue->GetName());
			}
			else if (Speaker)
			{
				if(Speaker->IsDBNO())
				{
					// if speaker is DBNO, wait for him to not be DBNO
					DBNOSpeakerToWaitFor = Speaker;
					return;
				}
				Speaker->SpeakLine(Addressee, ActiveSoundCue, TEXT(""), 0.f, Speech_Scripted, SIC_Always, !bTurnHeadTowardsAddressee, FALSE, bSuppressSubtitles,ExtraHeadTurnTowardTime);

				// turn whole body using DesiredRotation functionality in the controller
				if (bTurnBodyTowardsAddressee && Addressee && Speaker->Controller)
				{
					Speaker->Controller->bForceDesiredRotation = TRUE;
					Speaker->Controller->DesiredRotation = (Addressee->Location - Speaker->Location).Rotation();
					Speaker->Controller->DesiredRotation.Pitch = 0;
					Speaker->Controller->DesiredRotation.Roll = 0;
				}

				if (GestureAnimName != NAME_None)
				{
					Speaker->eventPlaySpeechGesture(GestureAnimName);
				}

				SpokenLineAC = Speaker->CurrentlySpeakingLine;

				KISMET_LOG(TEXT("- GearPawn %s speaking cue %s"),*Speaker->GetName(),*ActiveSoundCue->GetName());
			}
			else
			{
				RemoteSpeaker->eventRemoteSpeakLine(Addressee, ActiveSoundCue, TEXT(""), 0.f, bSuppressSubtitles, SLBFilter_None, Speech_Scripted);
				SpokenLineAC = RemoteSpeaker->CurrentlySpeakingLine;
				KISMET_LOG(TEXT("- GearPawn %s speaking cue %s"),*RemoteSpeaker->GetName(),*ActiveSoundCue->GetName());
			}

			// use duration for timings.  less accurate.
			SoundDuration = ActiveSoundCue->GetCueDuration();
			// adjust using cue's pitch, which won't be as accurate (e.g. won't include effects of modulator), but it's better than nothing
			SoundDuration /= ActiveSoundCue->PitchMultiplier;

			if (SpokenLineAC)
			{
				bSpokenLineACHasStartedPlaying = SpokenLineAC->IsPlaying();
			}

			// adjust duration for AC's pitch
			KISMET_LOG(TEXT("-> sound duration: %2.1f"),SoundDuration);

			// Set this to false now, so UpdateOp doesn't call Activated again straight away.
			InputLinks(0).bHasImpulse = FALSE;
		}
		// the "stop" input
		else if (InputLinks(1).bHasImpulse)
		{
			ForceStopSpeech(ActiveSoundCue);
			// Set this to false now, so UpdateOp doesn't call Activated again straight away.
			InputLinks(1).bHasImpulse = FALSE;
		}
	}

	// activate the base output
	OutputLinks(0).ActivateOutputLink();
}

UBOOL USeqAct_Speak::UpdateOp(float DeltaTime)
{
	USoundCue* ActiveSoundCue = InitSpeakSoundCue();
	UBOOL bFinished = FALSE;

	// catch another attempt to play the sound again
	if (InputLinks(0).bHasImpulse)
	{
		if (!bActive)
		{
			Activated();
		}
	}
	else if (InputLinks(1).bHasImpulse)
	{
		ForceStopSpeech(ActiveSoundCue);
	}
	else if (ExtraDelayStartTime > 0.f)
	{
		
		if ( (GWorld->GetWorldInfo()->TimeSeconds - ExtraDelayStartTime) > ExtraDelay )
		{
			// really finished now
			bFinished = TRUE;
		}
	}
	else if (SpokenLineAC)
	{
		if ( !bSpokenLineACHasStartedPlaying && SpokenLineAC->IsPlaying() )
		{
			// need to defer this, since FaceFX doesn't necessarily start the cue right away
			bSpokenLineACHasStartedPlaying = TRUE;
		}
		else if ( bSpokenLineACHasStartedPlaying && !SpokenLineAC->IsPlaying() )
		{
			bFinished = TRUE;
		}

		// escape hatch -- if line hasn't started by now (seems to happen occasionally, silly FaceFX)
		// the we will just bail and continue as if everything was cool and froody.
		SoundDuration -= DeltaTime;
		if ( (SoundDuration <= 0.f) && !bSpokenLineACHasStartedPlaying )
		{
			bFinished = TRUE;
		}
	}
	else if(DBNOSpeakerToWaitFor != NULL)
	{
		// if pawn is dead, just move on
		if(DBNOSpeakerToWaitFor->Health <= 0 && !DBNOSpeakerToWaitFor->IsDBNO())
		{
			DBNOSpeakerToWaitFor=NULL;
			bFinished=TRUE;
		}
		// if pawn is alive and no longer DBNO, speak line now
		else if(!DBNOSpeakerToWaitFor->IsDBNO())
		{
			Activated();
		}
	}
	else
	{
		SoundDuration -= DeltaTime;
		if (SoundDuration <= 0.f)
		{
			bFinished = TRUE;
		}
	}

	if (bFinished)
	{
		if ( (ExtraDelay > 0.f) && (ExtraDelayStartTime <= 0.f) )
		{
			// start the "extra delay" time period
			ExtraDelayStartTime = GWorld->GetWorldInfo()->TimeSeconds;
			bFinished = FALSE;
		}
		else
		{
			SpeechFinished();
		}
	}

	return bFinished || bStopped;
}

UObject** USeqVar_Anya::GetObjectRef( INT Idx )
{
	// Note, could easily extend this to provide the physical, pawn version of Anya
	// based on a checkbox or something.

	if (ObjValue == NULL ||
		!ObjValue->IsA(ARemoteSpeaker_COGAnya::StaticClass()))
	{
		ObjValue = NULL;

		if( GWorld != NULL )
		{
			AWorldInfo* const WI = GWorld->GetWorldInfo();

			if (WI)
			{
				AGearGameSP_Base* const SPGame = Cast<AGearGameSP_Base>(WI->Game);
				if (SPGame)
				{

					ObjValue = SPGame->Anya;
				}
			}
		}
	}

	return USeqVar_Object::GetObjectRef( Idx );	
}

UObject** USeqVar_GenericRemoteSpeaker::GetObjectRef( INT Idx )
{
	if (ObjValue == NULL ||
		!ObjValue->IsA(ARemoteSpeaker_Generic::StaticClass()))
	{
		ObjValue = NULL;

		if( GWorld != NULL )
		{
			AWorldInfo* const WI = GWorld->GetWorldInfo();

			if (WI)
			{
				AGearGame* const WG = Cast<AGearGame>(WI->Game);
				if (WG)
				{
					ObjValue = WG->GenericRemoteSpeaker;
				}
			}
		}
	}

	return USeqVar_Object::GetObjectRef( Idx );	
}

void USeqAct_GearUseCommLink::Activated()
{
	Super::Activated();

	bStopped = FALSE;
	bStarted = FALSE;
	bFinished = FALSE;

	TArray<AGearPC*> GPCTargets;
	TArray<AGearPawn*> PawnTargets;
	{
		TArray<UObject**> ObjVars;
		GetObjectVars(ObjVars, TEXT("Target"));
		BuildGearPCTargets(ObjVars, GPCTargets);
		BuildGearPawnTargets(ObjVars, PawnTargets);
	}

	UBOOL bEnabling = InputLinks(0).bHasImpulse;
	for (INT PawnIdx=0; PawnIdx<PawnTargets.Num(); ++PawnIdx)
	{
		// the start input
		PawnTargets(PawnIdx)->eventSetConversing(bEnabling, bUseCommLink);
	}
	for (INT PCIdx=0; PCIdx<GPCTargets.Num(); ++PCIdx)
	{
		GPCTargets(PCIdx)->eventClientSetConversationMode(bEnabling, bPlayerCanAbort);
	}

	if (!bEnabling)
	{
		// "Disable" input has been activated
		InputLinks(1).bHasImpulse = FALSE;
		bAbortedByPlayer = FALSE;
		bStopped = TRUE;
	}
	else
	{
		AbortabilityDelayRemaining = AbortabilityDelay;
	}

	OutputLinks(0).ActivateOutputLink();
}



UBOOL USeqAct_GearUseCommLink::UpdateOp(FLOAT DeltaTime)
{
	if (AbortabilityDelayRemaining > 0.f)
	{
		AbortabilityDelayRemaining -= DeltaTime;
	}

	if (bStopped)
	{
		// we've already been stopped, just bail so we can deactivate
		return TRUE;
	}
	else if (InputLinks(0).bHasImpulse)
	{
		// nothing
	}
	else
	{
		TArray<UObject**> ObjVars;
		GetObjectVars(ObjVars, TEXT("Target"));
		TArray<AGearPawn*> PawnTargets;
		BuildGearPawnTargets(ObjVars, PawnTargets);

		if (InputLinks(1).bHasImpulse)
		{
			// "stop" input
			InputLinks(1).bHasImpulse = FALSE;
			bStopped = TRUE;
			bAbortedByPlayer = FALSE;

			for (INT PawnIdx=0; PawnIdx<PawnTargets.Num(); ++PawnIdx)
			{
				PawnTargets(PawnIdx)->eventSetConversing(FALSE);
			}

		//	OutputLinks(0).ActivateOutputLink();
		}
		else
		{
			// no inputs active, just update

			// poll all PC's were affecting and see if any of them have left conversation camera mode
			// without this action knowing about it.  if so, assume an abort.

			// get list of PCs we're affecting
			TArray<AGearPC*> GPCTargets;
			BuildGearPCTargets(ObjVars, GPCTargets);

			// check each GearPC for aborts
			INT const NumGPCTargets = GPCTargets.Num();
			for (INT TargetIdx=0; TargetIdx<NumGPCTargets; ++TargetIdx)
			{
				if ( GPCTargets(TargetIdx)->bPendingConversationAbort)
				{
					if (AbortabilityDelayRemaining > 0.f)
					{
						// can't abort yet, ignore for now
						GPCTargets(TargetIdx)->bPendingConversationAbort = FALSE;
					}
					else
					{
						bAbortedByPlayer = TRUE;
						bStopped = TRUE;

						// don't early out of this loop, make sure all pending flags get cleared.
						GPCTargets(TargetIdx)->bPendingConversationAbort = FALSE;

						AGearPawn* const GP = Cast<AGearPawn>(GPCTargets(TargetIdx)->Pawn);

						if (GP != NULL)
						{
							GP->eventSetConversing(FALSE);
						}
						if ( (AbortLines.Num() > 0) && GP && !GP->bSpeaking )
						{
							INT const RandIdx = RandHelper(AbortLines.Num());
							GP->SpeakLine(NULL, AbortLines(RandIdx), TEXT(""), 0.f, Speech_Scripted, SIC_Always, TRUE);
						}
					}
				}
			}

			if ( !bStopped && SingleLineSound && (PawnTargets.Num() > 0) )
			{
				AGearPawn* const Target = PawnTargets.Num() > 0 ? PawnTargets(0) : NULL;

				// update SingleLineSound functionality
				// note that the SingleLineSound will only apply to Target(0), if multiple Pawns are attached
				if (Target->bIsConversing && (!bUseCommLink || Target->bInCommLinkStance))
				{
					if (!bSingleLineSoundIsPlaying)
					{
						// start playing the line
						Target->SpeakLine(NULL, SingleLineSound, TEXT(""), 0.f, Speech_Scripted, SIC_Always, TRUE);

						// figure out the Latent duration
						USoundNodeWave* const Wave = FindFirstWaveNode(SingleLineSound->FirstNode);
						if (Wave != NULL)
						{
							SoundDuration = Wave->Duration + 0.2f;
						}
						else
						{
							SoundDuration = 0.f;
						}
						bSingleLineSoundIsPlaying = TRUE;
						OutputLinks(1).ActivateOutputLink();			// "started" output
					}
					else
					{
						// decrement timer, stop if the line is finished
						SoundDuration -= DeltaTime;
						if (SoundDuration < 0.f)
						{
							Target->eventSetConversing(FALSE);
							bStopped = FALSE;		
							bFinished = TRUE;		// when singlelinesound finishes, fire Finished output
							bSingleLineSoundIsPlaying = FALSE;
						}
					}
				}
			}
			else
			{
				if (!bStarted)
				{
					// check and see if dude made it into commlink mode yet
					for (INT PawnIdx=0; PawnIdx<PawnTargets.Num(); ++PawnIdx)
					{
						if (PawnTargets(PawnIdx)->bIsConversing)
						{
							bStarted = TRUE;
							OutputLinks(1).ActivateOutputLink();			// "started" output
							break;
						}
					}
				}
			}	
		}
	}

	return (bStopped || bFinished);
}

void USeqAct_GearUseCommLink::DeActivated()
{
	if (bStopped)
	{
		OutputLinks(2).ActivateOutputLink();
		if (bAbortedByPlayer)
		{
			OutputLinks(4).ActivateOutputLink();
		}
	}
	else if (bFinished)
	{
		OutputLinks(3).ActivateOutputLink();
	}

	// spin through the targets and turn off abortabilty for all of them
	if (bPlayerCanAbort)
	{
		TArray<AGearPC*> GPCTargets;
		{
			TArray<UObject**> ObjVars;
			GetObjectVars(ObjVars, TEXT("Target"));
			BuildGearPCTargets(ObjVars, GPCTargets);
		}
		INT const NumGPCTargets = GPCTargets.Num();
		for (INT TargetIdx=0; TargetIdx<NumGPCTargets; ++TargetIdx)
		{
			GPCTargets(TargetIdx)->eventClientSetConversationMode(FALSE);
		}
	}

	// make sure everyone is done with commlink
	if (bUseCommLink)
	{
		TArray<AGearPawn*> PawnTargets;
		{
			TArray<UObject**> ObjVars;
			GetObjectVars(ObjVars, TEXT("Target"));
			BuildGearPawnTargets(ObjVars, PawnTargets);
		}
		for (INT PawnIdx=0; PawnIdx<PawnTargets.Num(); ++PawnIdx)
		{
			// the start input
			PawnTargets(PawnIdx)->eventSetConversing(FALSE, bUseCommLink);
		}
	}
}

static UBOOL AreHardRefsBad( const USeqAct_AIFactory* TheSet )
{
	UBOOL Retval = FALSE;

	for( INT SetIdx = 0; SetIdx < TheSet->SpawnSets.Num(); ++SetIdx )
	{
		const FAISpawnInfo& Set = TheSet->SpawnSets(SetIdx);

		if(( Set.ControllerClass == NULL ) 
			|| ( Set.GearPawnClass == NULL ) 
			|| ( ( Set.GearPawnClass != NULL ) && (( Set.GearPawnClass->GetName() == TEXT( "GearPawn_LocustWretch" )) || ( Set.GearPawnClass->GetName() == TEXT( "GearPawn_COGGear" )) ))
			)
		{
			Retval = TRUE;
			break;
		}
	}

	return Retval;
}


void USeqAct_AIFactory::PostLoad()
{
	RepairAITypeSetToDoNotUseEnum();

	if( AreHardRefsBad(this) == TRUE )
	{
		SetTheHardReferences();
		Modify( TRUE ); // dirty the package
	}

    if (!bFixedUpAutoEnemyDefaults)
    {
        // force the defaults to false
        for (INT Idx = 0; Idx < SpawnSets.Num(); Idx++)
        {
            FAISpawnInfo &Set = SpawnSets(Idx);
            Set.bAutoAcquireEnemy = FALSE;
            Set.bAutoNotifyEnemy = FALSE;
        }
        // don't bother changing again
        bFixedUpAutoEnemyDefaults = TRUE;
    }


	// so due to the Struct's not getting the defaultstructproperties updated in the SpawnSet we need to update manually
	if( ObjInstanceVersion < 40 )
	{
		for (INT Idx = 0; Idx < SpawnSets.Num(); Idx++)
		{
			FAISpawnInfo &Set = SpawnSets(Idx);
			Set.bAllowHitImpactDecalsOnSkelMesh = TRUE;
			Set.bSpawnHitEffectDecals = TRUE;
			Set.bSpawnBloodTrailDecals = TRUE;
			Set.ShadowMode = LightShadow_Modulate;
		}
	}

	if( ObjInstanceVersion < 45 ) // when this was made the SequenceObject's version was at 1
	{
		for (INT Idx = 0; Idx < SpawnSets.Num(); Idx++)
		{
			FAISpawnInfo &Set = SpawnSets(Idx);
			Set.ShadowMode = LightShadow_Modulate;
		}
	}

	Super::PostLoad();
}


/** If we are using a DoNotUse enum then we need to set it to a valid enum **/
void USeqAct_AIFactory::RepairAITypeSetToDoNotUseEnum()
{
	for(INT SetIdx=0;SetIdx<SpawnSets.Num();++SetIdx)
	{
		FAISpawnInfo &Set = SpawnSets(SetIdx);
		if(Set.AIType >= SpawnInfo.Num() || SpawnInfo(Set.AIType).ControllerClassName == NAME_None || SpawnInfo(Set.AIType).PawnClassName == NAME_None)
		{
			if(GIsEditor && !GIsPlayInEditorWorld)
			{
				const FString BadSpawnSetMsg = FString::Printf(TEXT("%s spawnset idx %i was set to use an AIType which is invalid! Falling back on AIType_Drone!!"),*GetPathName(),SetIdx);
				warnf( *BadSpawnSetMsg );
				appMsgf( AMT_OK, *BadSpawnSetMsg);
			}
			Set.AIType=0;
		}
	}
}


void USeqAct_AIFactory::PostEditChange( UProperty* PropertyThatChanged )
{
	// repair any outdated enums
	RepairAITypeSetToDoNotUseEnum();
	// set the actual class references for cooking
	SetTheHardReferences();
	Super::PostEditChange(PropertyThatChanged);
}

void USeqAct_AIFactory::SetTheHardReferences()
{
	INT SpawnPointIdx = 0;
	// create all the spawn sets
	for( INT SetIdx = 0; SetIdx < SpawnSets.Num(); ++SetIdx )
	{
		FAISpawnInfo &Set = SpawnSets(SetIdx);
	
		if( Set.AIType < SpawnInfo.Num() )
		{
			Set.ControllerClass = FindObject<UClass>( ANY_PACKAGE, *(SpawnInfo((INT)Set.AIType).ControllerClassName.ToString()) ); 
			Set.GearPawnClass = FindObject<UClass>( ANY_PACKAGE, *(SpawnInfo((INT)Set.AIType).PawnClassName.ToString()) );
			if (Set.GearPawnClass == NULL)
			{
				debugf(TEXT("Failed to find pawn class for set entry %s"),*SpawnInfo((INT)Set.AIType).PawnClassName.ToString());
			}
		}
		else
		{
			debugf(TEXT("USeqAct_AIFactory::SetTheHardReferences, Set.AIType >= SpawnInfo.Num(), Cannot set ControllerClass and PawnClass :("));
		}

		Set.LoadoutClasses.Empty();
		for( INT LoadoutIndex=0; LoadoutIndex<Set.WeaponLoadOut.Num(); LoadoutIndex++ )
		{
			const FName& LoadoutName = InventoryTypeNames(Set.WeaponLoadOut(LoadoutIndex));
			UClass* LoadoutClass = FindObject<UClass>( ANY_PACKAGE, *LoadoutName.ToString() );
			if( LoadoutClass )
			{
				Set.LoadoutClasses.AddItem( LoadoutClass );
			}
			else
			{
				debugf(TEXT("Failed to find loadout class: %s"),*LoadoutName.ToString());
			}
		}
	}

}


IMPLEMENT_COMPARE_CONSTREF( INT, GearGame, { return (B - A); } )

void USeqAct_AIFactory::UpdateDynamicLinks()
{
	// make sure there is always at least one spawn set
	if (SpawnSets.Num() == 0)
	{
		// constructor will init defaults
		new(SpawnSets) FAISpawnInfo(EC_NativeConstructor);
		bFixedUpAutoEnemyDefaults = TRUE;
	}
	TArray<INT> UsedVarLinks;
	TArray<INT> UsedOutLinks;
	for (INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++)
	{
		FAISpawnInfo &Set = SpawnSets(SetIdx);
		// update the variable link descriptions
		Set.VarLinkDescs.Empty();
		for (INT Idx = 0; Idx < Set.SpawnTotal; Idx++)
		{
			Set.VarLinkDescs.AddItem(FString::Printf(TEXT("Set %d [%d]"),SetIdx+1,Idx+1));
		}
		Set.VarLinkDescs.AddItem(FString::Printf(TEXT("Set %d [Max Alive]"),SetIdx+1));
		Set.VarLinkDescs.AddItem(FString::Printf(TEXT("Set %d [Spawn Total]"),SetIdx+1));
		// update the variable links if necessary
		for (INT Idx = 0; Idx < Set.VarLinkDescs.Num(); Idx++)
		{
			UBOOL bFoundLink = FALSE;
			for (INT LinkIdx = 0; LinkIdx < VariableLinks.Num(); LinkIdx++)
			{
				if (VariableLinks(LinkIdx).LinkDesc == Set.VarLinkDescs(Idx))
				{
					bFoundLink = TRUE;
					UsedVarLinks.AddItem(LinkIdx);
					break;
				}
			}
			// add a new link for this desc
			if (!bFoundLink)
			{
				INT LinkIdx = VariableLinks.AddZeroed(1);
				FSeqVarLink &VarLink = VariableLinks(LinkIdx);
				UsedVarLinks.AddItem(LinkIdx);
				VarLink.LinkDesc = Set.VarLinkDescs(Idx);
				if (Idx >= Set.VarLinkDescs.Num() - 2)
				{
					// use int for the last 2 (max alive, spawn total)
					VarLink.ExpectedType = USeqVar_Int::StaticClass();
				}
				else
				{
					// and object for everything else
					VarLink.ExpectedType = USeqVar_Object::StaticClass();
				}
				VarLink.MinVars = 0;
				VarLink.MaxVars = 255;
				VarLink.bHidden = TRUE;
				VarLink.bWriteable = TRUE;
			}
		}

		// do the same for output links
		Set.OutLinkDescs.Empty();
		for (INT Idx = 0; Idx < Set.SpawnTotal; Idx++)
		{
			Set.OutLinkDescs.AddItem(FString::Printf(TEXT("Spawn %d [%d]"),SetIdx+1,Idx+1));
		}
		// update the output links if necessary
		for (INT Idx = 0; Idx < Set.OutLinkDescs.Num(); Idx++)
		{
			UBOOL bFoundLink = FALSE;
			for (INT LinkIdx = 0; LinkIdx < OutputLinks.Num(); LinkIdx++)
			{
				if (OutputLinks(LinkIdx).LinkDesc == Set.OutLinkDescs(Idx))
				{
					bFoundLink = TRUE;
					UsedOutLinks.AddItem(LinkIdx);
					break;
				}
			}
			// add a new link for this desc
			if (!bFoundLink)
			{
				INT LinkIdx = OutputLinks.AddZeroed(1);
				FSeqOpOutputLink &OutLink = OutputLinks(LinkIdx);
				UsedOutLinks.AddItem(LinkIdx);
				OutLink.LinkDesc = Set.OutLinkDescs(Idx);
				OutLink.bHidden = TRUE;
			}
		}
	}
	Sort<USE_COMPARE_CONSTREF(INT, GearGame)>(&UsedVarLinks(0),UsedVarLinks.Num());
	Sort<USE_COMPARE_CONSTREF(INT, GearGame)>(&UsedOutLinks(0),UsedOutLinks.Num());
	// remove any links that aren't used
	// NOTE: skip the predefined links
	INT DefaultLinksNum = ((USequenceOp*)(GetClass()->GetDefaultObject()))->VariableLinks.Num() - 1;
	for (INT Idx = VariableLinks.Num() - 1; Idx > DefaultLinksNum ; Idx--)
	{
		if (VariableLinks(Idx).PropertyName == NAME_None && !UsedVarLinks.ContainsItem(Idx))
		{
			VariableLinks.Remove(Idx,1);
		}
	}
	DefaultLinksNum = ((USequenceOp*)(GetClass()->GetDefaultObject()))->OutputLinks.Num() - 1;
	for (INT Idx = OutputLinks.Num() - 1; Idx > DefaultLinksNum ; Idx--)
	{
		if (!UsedOutLinks.ContainsItem(Idx))
		{
			OutputLinks.Remove(Idx,1);
		}
	}
	Super::UpdateDynamicLinks();
	// Force udpate of references
	SetTheHardReferences();
}

UBOOL USeqAct_AIFactory::CanSpawnAtLocation(FVector &ChkLocation, FVector ChkExtent, AActor* SpawnPointActor )
{
	// check for collision against any pawns at the location
	{
		FMemMark Mark(GMainThreadMemStack);
		FCheckResult *ChkResult = NULL;
		ChkResult = GWorld->MultiPointCheck(GMainThreadMemStack,ChkLocation,ChkExtent,TRACE_AllColliding);
		UBOOL bCollided = FALSE;
		for (FCheckResult *TestResult = ChkResult; TestResult != NULL && !bCollided; TestResult = TestResult->GetNext())
		{
			if (TestResult->Actor != NULL &&
				TestResult->Actor->IsA(APawn::StaticClass()))
			{
				bCollided = TRUE;
			}
		}
		Mark.Pop();
		if (bCollided)
		{
			return FALSE;
		}
	}
	// attempt to adjust the location so that the full extent won't penetrate
	{
		FCheckResult Hit;
		FVector NewChkExtent(ChkExtent.X,ChkExtent.Y,1.f);
		FVector Up(0,0,1);
		ANavigationPoint* Nav = Cast<ANavigationPoint>(SpawnPointActor);
		if( Nav )
		{
			Nav->GetUpDir( Up );
		}

		FVector TestLoc = ChkLocation - (Up * FVector(0,0,ChkExtent.Z*2.f));
		if (!GWorld->SingleLineCheck(Hit,NULL,TestLoc,ChkLocation,TRACE_AllColliding,NewChkExtent))
		{
			ChkLocation  = Hit.Location;
			ChkLocation += (ChkExtent * Up);
		}
	}
	return TRUE;
}

/**
 * Resets any transient info for the spawn sets.
 */
void USeqAct_AIFactory::ResetSpawnSets()
{
	// figure out if we're in coop or not
	INT PlayerCount = 0;
	for (AController *Controller = GWorld->GetFirstController(); Controller != NULL; Controller = Controller->NextController)
	{
		if (Controller->IsA(AGearPC::StaticClass()))
		{
			if (++PlayerCount > 1)
			{
				break;
			}
		}
	}
	UBOOL bIsCoop = (PlayerCount > 1);
	for (INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++)
	{
		FAISpawnInfo &Set = SpawnSets(SetIdx);
		Set.CurrentDelay = 0.f;
		Set.CurrentSpawns.Empty();
		Set.WatchList.Empty();
		if (Set.MaxAlive <= 0)
		{
			Set.MaxAlive = Set.SpawnTotal;
		}
		// if this set only works in coop, and we're in coop
		if (Set.bSpawnOnlyIfCoop && bIsCoop)
		{
			// then just pretend we spawned everything
			Set.SpawnedCount = Set.SpawnTotal;
		}
		else
		{
			// otherwise reset the spawned count
			Set.SpawnedCount = 0;
		}
	}
}

AActor* USeqAct_AIFactory::PickSpawnPoint(FVector& SpawnExtent, FVector& SpawnLocation, FRotator& SpawnRotation, AGearSpawner** Spawner, INT& SpawnerSlotIdx, INT& SpawnPointIdx, UBOOL bSkipChecks)
{
	// make sure the idx is valid
	if (SpawnPointIdx >= SpawnPoints.Num())
	{
		SpawnPointIdx = 0;
	}
	// shuffle the first time around if necessary
	if (SpawnPointIdx == 0 && SpawnPointSelectionMethod == SPSM_Shuffle)
	{
		// randomize the list of points
		for (INT Idx = 0; Idx < SpawnPoints.Num(); Idx++)
		{
			INT NewIdx = Idx + (appRand()%(SpawnPoints.Num()-Idx));
			SpawnPoints.SwapItems(NewIdx,Idx);
		}
	}
	// iterate through spawn points until we successfully spawn or run out spawn points
	AActor *SpawnPoint = NULL;
	UBOOL bFoundSpawnPoint = FALSE;
	while (!bFoundSpawnPoint && SpawnPointIdx < SpawnPoints.Num())
	{
		SpawnPoint = SpawnPoints(SpawnPointIdx);
		if (SpawnPoint != NULL)
		{
			*Spawner = Cast<AGearSpawner>(SpawnPoint);
			if (*Spawner != NULL)
			{
				if ((*Spawner)->eventGetSpawnSlot(SpawnerSlotIdx,SpawnLocation,SpawnRotation))
				{
					// assume spawners as always valid, so don't bother checking
					// the spawn location
					bFoundSpawnPoint = TRUE;
				}
				else
				{
					// be sure to clear the spawner property if we're not using it
					*Spawner = NULL;
					SpawnerSlotIdx = -1;
				}
			}
			else
			{
				ATurret* Turret = Cast<ATurret>(SpawnPoint);
				if( Turret != NULL )
				{
					SpawnLocation = Turret->eventGetEntryLocation();
					SpawnRotation = Turret->Rotation;

					bFoundSpawnPoint = TRUE;
				}
				else
				{
					SpawnLocation = SpawnPoint->Location;
					SpawnRotation = SpawnPoint->Rotation;
					
					// check for collisions at this spawn point
					if (bSkipChecks || CanSpawnAtLocation(SpawnLocation,SpawnExtent,SpawnPoint))
					{
						bFoundSpawnPoint = TRUE;
					}
				}
			}
		}
		SpawnPointIdx++;
	}
	if (!bFoundSpawnPoint)
	{
		*Spawner = NULL;
		SpawnPoint = NULL;
	}
	KISMET_LOG(TEXT("-- picked spawn point: %s"),SpawnPoint!=NULL?*SpawnPoint->GetPathName():TEXT("NULL"));
	return SpawnPoint;
}

/**
 * Check each set and see if any new pawns need to be created.
 */
void USeqAct_AIFactory::UpdateSpawnSets(FLOAT DeltaTime)
{
	INT SpawnPointIdx = 0;
	// create all the spawn sets
	for (INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++)
	{
		FAISpawnInfo &Set = SpawnSets(SetIdx);

		// for each thing currently spawning
		for (INT Idx = 0; Idx < Set.CurrentSpawns.Num(); Idx++)
		{
			APawn *CurrentSpawn = Set.CurrentSpawns(Idx);
			// if it's invalid
			if (CurrentSpawn == NULL || CurrentSpawn->bDeleteMe)
			{
				// remove and continue to the next
				debugf(TEXT("%s clearing null entry"),*GetName());
				Set.CurrentSpawns.Remove(Idx--,1);
				continue;
			}
			// if the pawn is finished spawning
			if (!CurrentSpawn->IsA(AGearPawn::StaticClass()) || !((AGearPawn*)CurrentSpawn)->bSpawning)
			{
				AGearAI *AI = Cast<AGearAI>(CurrentSpawn->Controller);
				if( AI != NULL )
				{
					if( CombatMood != AICM_None )
					{
						AI->eventSetCombatMood( CombatMood );
					}
					if( PerceptionMood != AIPM_None )
					{
						AI->eventSetPerceptionMood( PerceptionMood );
					}					

					// check for move target assignment
					if (MoveTargets.Num() > 0)
					{
						// pick a random for shared
						if (MoveAssignment == EA_Shared)
						{
							MoveTargetIdx = appRand() % MoveTargets.Num();
						}
						if (MoveTargetIdx >= 0 && MoveTargetIdx < MoveTargets.Num())
						{
							AI->eventSpawnerSetTether(MoveTargets(MoveTargetIdx),bMoveInterruptable);
							if (MoveAssignment == EA_UniquePerSpawn)
							{
								// increment the index to preserve unique assignments
								// (Wrap around so they are always properly moving somewhere!)
								MoveTargetIdx = (MoveTargetIdx + 1) % MoveTargets.Num();
							}
						}
					}
					else
					if( CombatZones.Num() > 0 )
					{
						AI->eventMoveToCombatZone();
					}

					// tell the AI to auto acquire enemies if necessary
					if( Set.bAutoAcquireEnemy )
					{
						AI->eventAutoAcquireEnemy();
					}
					// tell AI enemies about this new enemy
					if( Set.bAutoNotifyEnemy )
					{
						AI->eventAutoNotifyEnemy();
					}
				}
				// write out to any attached variable links
				TArray<UObject**> ObjectVars;
				GetObjectVars(ObjectVars,TEXT("Last Spawned"));
				if (Set.SpawnedCount < Set.VarLinkDescs.Num() - 2)
				{
					GetObjectVars(ObjectVars,*Set.VarLinkDescs(Set.SpawnedCount));
				}
				else
				{
					debugf(NAME_Warning,TEXT("Invalid index into VarLinkDescs, %d/%d, %s"),Set.SpawnedCount,Set.VarLinkDescs.Num(),*GetPathName());
				}
				// write out the spawned object list
				for (INT VarIdx = 0; VarIdx < VariableLinks.Num(); VarIdx++)
				{
					//@fixme: what if we add another variable link using object lists?
					if (VariableLinks(VarIdx).ExpectedType == USeqVar_ObjectList::StaticClass())
					{
						// iterate over each linked var, in case they're using multiple object lists (of course why would they?)
						for (INT LinkedVarIdx = 0; LinkedVarIdx < VariableLinks(VarIdx).LinkedVariables.Num(); LinkedVarIdx++)
						{
							USeqVar_ObjectList *ObjList = Cast<USeqVar_ObjectList>(VariableLinks(VarIdx).LinkedVariables(LinkedVarIdx));
							if (ObjList != NULL)
							{
								ObjList->ObjList.AddItem(CurrentSpawn);
							}
						}
						// no need to keep looking
						break;
					}
				}
				for (INT VarIdx = 0; VarIdx < ObjectVars.Num(); VarIdx++)
				{
					*(ObjectVars(VarIdx)) = CurrentSpawn;
				}
				// activate matching output link as well
				if (Set.SpawnedCount < Set.OutLinkDescs.Num())
				{
					for (INT OutIdx = 0; OutIdx < OutputLinks.Num(); OutIdx++)
					{
						if (!OutputLinks(OutIdx).bDisabled &&
							!(OutputLinks(OutIdx).bDisabledPIE && GIsEditor) &&
							OutputLinks(OutIdx).LinkDesc == Set.OutLinkDescs(Set.SpawnedCount))
						{
							OutputLinks(OutIdx).bHasImpulse = TRUE;
						}
					}
				}
				// and the default spawned output link
				OutputLinks(3).bHasImpulse = TRUE;
				// increment the spawned count
				Set.SpawnedCount++;
				// clear the current spawn
				Set.CurrentSpawns.Remove(Idx--,1);
			}
		}
		// if this set hasn't spawned everything yet,
		if (Set.SpawnedCount + Set.CurrentSpawns.Num() < Set.SpawnTotal && Set.WatchList.Num() < Set.MaxAlive)
		{
			// check to see if we're waiting on a delay
			if (Set.CurrentDelay > 0.f)
			{
				Set.CurrentDelay -= DeltaTime;
			}
			else
			{
				// keep spawning until we've reached the max alive or hit a spawn delay or hit the total for this spawner
				// also abort if a spawn failed so we don't end up in an endless loop
				UBOOL bSpawnFailed = FALSE;
				while( Set.WatchList.Num() < Set.MaxAlive 
					&& Set.CurrentDelay <= 0.f 
					&& Set.SpawnedCount + Set.CurrentSpawns.Num() < Set.SpawnTotal
					&& !bSpawnFailed )
				{
					// figure out what type of AI to spawn
					UClass *ControllerClass = Set.ControllerClass;
					UClass *PawnClass = Set.GearPawnClass;

					check( ControllerClass && "Resave map to fixup references" );
					check( PawnClass  && "Resave map to fixup references" );

					FVector SpawnExtent(PawnClass->GetDefaultActor()->GetCylinderExtent());
					AGearSpawner *Spawner = NULL;
					INT SpawnerSlotIdx = -1;
					FVector SpawnLocation(0.f);
					FRotator SpawnRotation;
					AActor *SpawnPoint = PickSpawnPoint(SpawnExtent,SpawnLocation,SpawnRotation,&Spawner,SpawnerSlotIdx,SpawnPointIdx);
					// if we found a spawn point
					if (SpawnPoint != NULL)
					{
						UBOOL bNeedToSpawn = TRUE;
						if( Set.AIType < SpawnInfo.Num() && SpawnInfo(Set.AIType).bUnique )
						{
							// if it's a unique spawn then look for an existing one
							for (FDynamicActorIterator It; It; ++It)
							{
								if (It->IsA(PawnClass))
								{
									// don't spawn a new one
									bNeedToSpawn = FALSE;
									// add it to the watch list
									Set.CurrentSpawns.AddItem((APawn*)*It);
									Set.WatchList.AddItem((APawn*)*It);
									break;
								}
							}
						}
						if (bNeedToSpawn)
						{
							// create the pawn first
							APawn *CurrentSpawn = Cast<APawn>(GWorld->SpawnActor(PawnClass,NAME_None,SpawnLocation,SpawnRotation,NULL,TRUE,FALSE,NULL,NULL));
							if (CurrentSpawn != NULL)
							{
								KISMET_LOG(TEXT("-- spawned %s at %s"),*CurrentSpawn->GetName(),*SpawnPoint->GetPathName());
								AGearPawn* CurrentGP = Cast<AGearPawn>(CurrentSpawn);
								// add to the list of currently spawning
								Set.CurrentSpawns.AddItem(CurrentSpawn);
								if (CurrentGP != NULL)
								{
									// Set bEnableEncroachCheckOnRagdoll from the Factory
									CurrentGP->bEnableEncroachCheckOnRagdoll = bEnableEncroachCheckOnRagdoll;
								}

								if( Set.ActorTag != NAME_None )
								{
									CurrentSpawn->Tag = Set.ActorTag;
								}

								// give it a controller
								AAIController *NewController = Cast<AAIController>(GWorld->SpawnActor(ControllerClass,NAME_None,SpawnLocation,SpawnRotation,NULL,TRUE,FALSE,NULL,NULL));
								if (NewController != NULL)
								{
									// set team and possess the pawn
									if( Set.AIType < SpawnInfo.Num() )
									{
										NewController->eventSetTeam(SpawnInfo((INT)Set.AIType).TeamIdx);
									}
									else
									{
										debugf(TEXT("USeqAct_AIFactory::UpdateSpawnSets, Set.AIType >= SpawnInfo.Num()!! cannot call NewController->EventSetTeam()"));
									}

									AGearAI *AI = Cast<AGearAI>(NewController);
									if(AI != NULL)
									{
										// copy over stale delete permissions (needs to happen before possess)
										AI->bDeleteWhenStale = Set.bAllowDeleteWhenStale;

										// copy over chance to react to combat
										AI->PlayIntialCombatReactionChance = Set.PlayIntialCombatReactionChance;
									}
									NewController->eventPossess(CurrentSpawn,FALSE);
									
									// update the squad information
									if (AI != NULL)
									{
										// Store the factory that spawned us
										AI->SpawnFactory = this;
										// Set squad name
										AI->eventSetSquadName(Set.SquadName,Set.bSquadLeader);
									}
									// check for cover assignment
									if( AI != NULL && CombatZones.Num() > 0 )
									{
										for( INT Idx = 0; Idx < CombatZones.Num(); Idx++ )
										{
											if( CombatZones(Idx) != NULL )
											{
												AI->CombatZoneList.AddItem(CombatZones(Idx));
											}
										}
									}
								}
								// create inventory for the pawn
								if (CurrentGP != NULL)
								{
									if (Set.LoadoutClasses.Num() > 0)
									{
										if(CurrentGP->InvManager != NULL)
										{
											CurrentGP->InvManager->eventDiscardInventory();

											// Reset animation sets
											if( CurrentGP->eventRestoreAnimSetsToDefault() )
											{
												CurrentGP->Mesh->UpdateAnimations();
											}

											for (INT InvIdx = 0; InvIdx < Set.LoadoutClasses.Num(); InvIdx++)
											{
												if (!CurrentGP->InvManager->eventFindInventoryType(Set.LoadoutClasses(InvIdx), FALSE))
												{
													// create the inventory, only activating if it's the first entry, and if no weapon is currently activated
													CurrentGP->eventCreateInventory(Set.LoadoutClasses(InvIdx),InvIdx != 0 || CurrentGP->Weapon != NULL);
												}
											}
										}
									}
									else
									{
										// use the defaults if none are specified
										CurrentGP->eventAI_AddDefaultInventory();
									}
									// set the allow inventory drops flag on the new pawn
									CurrentGP->bAllowInventoryDrops = Set.bAllowInventoryDrops;

									// set all of the "Spawn Effects" flags
									CurrentGP->bAllowHitImpactDecalsOnSkelMesh = Set.bAllowHitImpactDecalsOnSkelMesh;
									CurrentGP->bSpawnHitEffectDecals = Set.bSpawnHitEffectDecals;
									CurrentGP->bSpawnBloodTrailDecals = Set.bSpawnBloodTrailDecals;

									CurrentGP->bDisableDeathScreams = Set.bDisableDeathScreams;
								}

								// add it to the watch list
								Set.WatchList.AddItem(CurrentSpawn);
								// if created at a spawner, let the spawner take over
								if (Spawner != NULL && CurrentGP != NULL)
								{
									Spawner->eventHandleSpawn(CurrentGP, SpawnerSlotIdx);
								}
								else
								{
									// If created on a wall path node and this pawn is a wretch
									AGearWallPathNode* WallNode = Cast<AGearWallPathNode>(SpawnPoint);
									if( WallNode && Cast<AGearPawn_LocustWretchBase>(CurrentSpawn) )
									{
										if( WallNode->Base == NULL )
										{
											WallNode->FindBase();
										}
										FVector Up(0,0,1);
										WallNode->GetUpDir( Up );
										CurrentSpawn->setPhysics( PHYS_Spider, WallNode->Base, Up );
										CurrentSpawn->SetAnchor( WallNode );
									}
									else
									{
										AGearAI_Cover* CovAI = Cast<AGearAI_Cover>(NewController);
										ATurret* Turret = Cast<ATurret>(SpawnPoint);
										if( Turret != NULL && CovAI != NULL )
										{
											CovAI->eventUseTurret( Turret, TRUE, TRUE );
										}
										else
										{
											ANavigationPoint* Nav = Cast<ANavigationPoint>(SpawnPoint);
											if( Nav != NULL )
											{
												CurrentSpawn->SetAnchor( Nav );
											}
										}
									}
								}
								// send a notification to the pawn they've been spawned
								if (CurrentGP != NULL)
								{
									CurrentGP->eventPostAIFactorySpawned(this,SetIdx);
								}
#if !FINAL_RELEASE
								// add auto-debug text for all players in the game
								if (Set.AutoDebugText.Len() > 0)
								{
									for (AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController)
									{
										APlayerController *Player = Cast<APlayerController>(Controller);
										if (Player != NULL)
										{
											Player->eventAddDebugText(Set.AutoDebugText,CurrentSpawn);
										}
									}
								}
#endif
								if (CurrentGP != NULL)
								{
									// if we were set up not to allow DBNO, carry that through to the pawn
									if (Set.bDontAllowDBNO)
									{
										CurrentGP->bCanDBNO = FALSE;
									}

									// copy over the light shadow mode to the pawn
									if (CurrentGP->LightEnvironment != NULL)
									{
										CurrentGP->LightEnvironment->LightShadowMode = Set.ShadowMode;
										CurrentGP->LightEnvironment->BeginDeferredReattach();
									}

									// set the bool for disabling shadow casting entirely
									if(CurrentGP->bDisableShadows != Set.bDisableShadowCasting)
									{
										CurrentGP->bDisableShadows = Set.bDisableShadowCasting;
										CurrentGP->eventUpdateShadowSettings(!Set.bDisableShadowCasting);
									}

									AGearPRI *GPRI = Cast<AGearPRI>(CurrentGP->PlayerReplicationInfo);
									if (GPRI != NULL)
									{
										GPRI->bForceShowInTaccom = Set.bForceShowInTaccom;
									}
								}
								else
								{
									AGearVehicle* CurrengGV = Cast<AGearVehicle>(CurrentSpawn);
									if(CurrengGV != NULL)
									{
										if(CurrengGV->bDisableShadows != Set.bDisableShadowCasting)
										{
											CurrengGV->bDisableShadows = Set.bDisableShadowCasting;
											CurrengGV->eventUpdateShadowSettings(!Set.bDisableShadowCasting);
										}
									}
								}

							}
							// Spawning failed - potentially a deprecated class.
							else
							{
								bSpawnFailed = TRUE;
							}
						}
					}
					else
					{
						// no spawn points available, delay
						Set.CurrentDelay = 0.25f;
					}
					// check for a spawn delay
					if (Set.MaxSpawnDelay > 0.f)
					{
						Set.CurrentDelay = Set.MinSpawnDelay + ((Set.MaxSpawnDelay - Set.MinSpawnDelay) * appFrand());
					}
				}
			}
		}
	}
}

void USeqAct_AIFactory::Activated()
{
	if (InputLinks(0).bHasImpulse && !bAbortSpawns)
	{
		// apply max alive/spawn total linked vars
		for (INT Idx = 0; Idx < SpawnSets.Num(); Idx++)
		{
			FAISpawnInfo &Set = SpawnSets(Idx);
			TArray<INT*> IntVars;
			// max alive first
			GetIntVars(IntVars,*Set.VarLinkDescs(Set.VarLinkDescs.Num() - 2));
			if (IntVars.Num() > 0)
			{
				// apply the sum
				Set.MaxAlive = 0;
				for (INT VarIdx = 0; VarIdx < IntVars.Num(); VarIdx++)
				{
					Set.MaxAlive += *IntVars(VarIdx);
				}
			}
			// spawn total next
			IntVars.Empty();
			GetIntVars(IntVars,*Set.VarLinkDescs(Set.VarLinkDescs.Num() - 1));
			if (IntVars.Num() > 0)
			{
				// apply the sum
				Set.SpawnTotal = 0;
				for (INT VarIdx = 0; VarIdx < IntVars.Num(); VarIdx++)
				{
					Set.SpawnTotal += *IntVars(VarIdx);
				}
			}
		}
		if (SpawnPoints.Num() > 0)
		{
			// register with any spawners
			for (INT Idx = 0; Idx < SpawnPoints.Num(); Idx++)
			{
				AGearSpawner *Spawner = Cast<AGearSpawner>(SpawnPoints(Idx));
				if (Spawner != NULL)
				{
					Spawner->eventRegisterFactory(this);
				}
			}
		}
		
		// register with BSM
		AGearGameSP_Base* const WG = Cast<AGearGameSP_Base>(GWorld->GetWorldInfo()->Game);
		if (WG && WG->BattleMonitor)
		{
			WG->BattleMonitor->eventRegisterAIFactory(this);
		}

		bAllSpawned = FALSE;
		bAllDead = FALSE;
		bAbortSpawns = FALSE;
		DeadCount = 0;
		MoveTargetIdx = 0;
		ResetSpawnSets();
	}
}

UBOOL USeqAct_AIFactory::UpdateOp(FLOAT DeltaTime)
{
	// check to see if we should teleport any unique spawns
	if( InputLinks(0).bHasImpulse && !bAbortSpawns )
	{
		bEnteredCombat = FALSE;

		INT SpawnPointIdx = 0;
		for (INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++)
		{
			FAISpawnInfo &Set = SpawnSets(SetIdx);
			if( Set.AIType >= SpawnInfo.Num() )
			{
				debugf(TEXT("USeqAct_AIFactory Set.AIType >= SpawnInfo.Num() - Abort!!!"));
				continue;
			}

			if (Set.GearPawnClass == NULL)
			{
				debugf(NAME_Warning,TEXT("Invalid pawn class for AI Factory %s, MAP NEEDS TO BE UPDATED AND RESAVED Idx:%i"),*GetPathName(),Set.AIType);
				continue;
			}

			if( SpawnInfo(Set.AIType).bUnique )
			{
				debugf(TEXT("trying to find existing pawn for unique set class %s"),*Set.GearPawnClass->GetName());
				// try to find an existing pawn
				for (FDynamicActorIterator It; It; ++It)
				{
					if (It->IsA(Set.GearPawnClass))
					{
						APawn* WP = (APawn*)*It;
						debugf(TEXT("- found %s"),*WP->GetName());
						// look for a spawn point for this character
						FVector SpawnExtent(Set.GearPawnClass->GetDefaultActor()->GetCylinderExtent());
						AGearSpawner *Spawner = NULL;
						INT SpawnerSlotIdx = -1;
						FVector SpawnLocation(0.f);
						FRotator SpawnRotation;
						AActor *SpawnPoint = PickSpawnPoint(SpawnExtent,SpawnLocation,SpawnRotation,&Spawner,SpawnerSlotIdx,SpawnPointIdx,TRUE);
						if (SpawnPoint == NULL)
						{
							debugf(NAME_Warning,TEXT("Failed to find spawn location for %s"),*WP->GetName());
							//@fixme - teleport to the human player or something?
						}
						else
						{
							// if it's AI controlled
							AGearAI *AI = Cast<AGearAI>(WP->Controller);
							if (AI != NULL)
							{
								// then join the squad
								AI->eventSetSquadName(Set.SquadName,Set.bSquadLeader);

								// find the Local PC
								AGearPC* wfPC = NULL;
								for( INT Idx = 0; Idx < GEngine->GamePlayers.Num() && wfPC == NULL; ++Idx ) 
								{
									if (GEngine->GamePlayers(Idx) != NULL &&
										GEngine->GamePlayers(Idx)->Actor != NULL &&
										GEngine->GamePlayers(Idx)->Actor->LocalPlayerController())
									{
										wfPC = Cast<AGearPC>(GEngine->GamePlayers(Idx)->Actor);
									}
								}
								UBOOL bShouldForceTeleport = FALSE;
								// check to teleport
								if( (TeleportDistance > 0.f && (WP->Location - SpawnLocation).Size2D() > TeleportDistance )
									|| (TeleportZDistance > 0.f && Abs(WP->Location.Z - SpawnLocation.Z) > TeleportZDistance)
									|| ( bShouldForceTeleport == TRUE )
									)
								{
									// and teleport to the spawn point
									AI->eventTeleportToLocation(SpawnLocation,SpawnRotation);
								}
								else
								if (bForceMoveToSpawnLocation)
								{
									AI->eventSpawnerSetTether(SpawnPoint,FALSE);
								}
							}
							else
							{
								AGearPC* WPC = Cast<AGearPC>(WP->Controller);
								if (WPC != NULL)
								{
									//WPC->eventSetSquadName(Set.SquadName);
								}
								// otherwise teleport players if necessary
								if ((TeleportDistance > 0.f && (WP->Location - SpawnLocation).Size2D() > TeleportDistance) ||
									(TeleportZDistance > 0.f && Abs(WP->Location.Z - SpawnLocation.Z) > TeleportZDistance))
								{
									ANavigationPoint *NavPoint = Cast<ANavigationPoint>(SpawnPoint);
									if( ((WPC == NULL) || (NavPoint == NULL)) || !WPC->eventFindAvailableTeleportSpot(WP, NavPoint, TRUE) )
									{
										WP->SetLocation(SpawnLocation);
										WP->SetRotation(SpawnRotation);
									}
								}
							}

							Set.CurrentSpawns.AddItem(WP);
							Set.WatchList.AddItem(WP);
						}
						break;
					}
				}
			}
		}
	}
	// first check for an abort
	if (InputLinks(1).bHasImpulse)
	{
		bAbortSpawns = TRUE;
	}
	// check for all kill
	if (InputLinks(2).bHasImpulse)
	{
		// for each set,
		for (INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++)
		{
			FAISpawnInfo &Set = SpawnSets(SetIdx);
			// kill everything still alive,
			for (INT Idx = 0; Idx < Set.WatchList.Num(); Idx++)
			{
				APawn* Pawn = Set.WatchList(Idx);
				if (Pawn != NULL &&
					Pawn->Health > 0)
				{
					if (Pawn->IsA(AGearPawn::StaticClass()))
					{
						((AGearPawn*)Pawn)->eventScriptedDeath();
					}
					else if (Pawn->IsA(AGearVehicle::StaticClass()))
					{
						((AGearVehicle*)Pawn)->eventVehicleScriptedDeath();
					}
					else
					{
						GWorld->DestroyActor(Pawn);
					}
					if (!bUseDamageForKillAll)
					{
						AAIController *AIController = Cast<AAIController>(Pawn->Controller);
						GWorld->DestroyActor(Pawn);
						if( AIController != NULL )
						{
							GWorld->DestroyActor(AIController);
						}
					}
				}
			}
		}
		// prevent further spawning
		bAbortSpawns = TRUE;
	}

	// Remove all of the dead spawns from the watchlist
	RemoveDeadSpawnsFromWatchList();

	// check to see if the dead link should be activated
	if( !bActivatedDeadLink &&
		ActivateDeadLinkCount > 0 &&
		DeadCount >= ActivateDeadLinkCount )
	{
		TArray<INT*> NumDeadVars;
		GetIntVars(NumDeadVars,TEXT("Num Dead"));
		for( INT VarIdx = 0; VarIdx < NumDeadVars.Num(); VarIdx++ )
		{
			*(NumDeadVars(VarIdx)) = DeadCount;
		}

		if( bResetDeadLinkCount )
		{
			bActivatedDeadLink = FALSE;
			DeadCount = 0;
		}
		else
		{
			bActivatedDeadLink = TRUE;
		}

		// don't activate the number of dead output if supressing the alldead output (for checkpoint loading)
		if (!bSuppressAllDead)
		{
			OutputLinks(4).ActivateOutputLink();
		}
	}

	// if we think we haven't spawned everything
	if (!bAllSpawned)
	{
		// assume all have been spawned
		bAllSpawned = TRUE;
		if (!bAbortSpawns)
		{
			// check for something to spawn
			UpdateSpawnSets(DeltaTime);
			// and look for a set that hasn't spawned everything yet
			for (INT SetIdx = 0; SetIdx < SpawnSets.Num() && bAllSpawned; SetIdx++)
			{
				if (SpawnSets(SetIdx).SpawnedCount < SpawnSets(SetIdx).SpawnTotal)
				{
					bAllSpawned = FALSE;
				}
			}
		}
		// if all sets have spawned everything
		if (bAllSpawned)
		{
			OutputLinks(0).ActivateOutputLink();
		}
	}
	else
	if (!bAllDead)
	{
		// otherwise see if everything is dead yet
		bAllDead = TRUE;
		for (INT SetIdx = 0; SetIdx < SpawnSets.Num() && bAllDead; SetIdx++)
		{
			FAISpawnInfo &Set = SpawnSets(SetIdx);
			// if the list still has entries, then something must be alive
			if (Set.WatchList.Num() != 0)
			{
				bAllDead = FALSE;
			}
		}
		// if everything is dead and we aren't suppressing the output, then fire it
		if (bAllDead && !bSuppressAllDead)
		{
			// activate the output
			OutputLinks(1).ActivateOutputLink();
		}
	}
	return (bAllSpawned && bAllDead);
}

void USeqAct_AIFactory::DeActivated()
{
	// unregister with any spawners
	for (INT Idx = 0; Idx < SpawnPoints.Num(); Idx++)
	{
		AGearSpawner *Spawner = Cast<AGearSpawner>(SpawnPoints(Idx));
		if (Spawner != NULL)
		{
			Spawner->eventUnRegisterFactory(this);
		}
	}
	// unregister with BSM
	AGearGameSP_Base* const WG = Cast<AGearGameSP_Base>(GWorld->GetWorldInfo()->Game);
	if (WG && WG->BattleMonitor)
	{
		WG->BattleMonitor->eventUnRegisterAIFactory(this);
	}
	Super::DeActivated();
}

void USeqAct_AIFactory::NotifyCombatEntered()
{
	if( bActive && !bEnteredCombat )
	{
		bEnteredCombat = TRUE;
		OutputLinks(5).ActivateOutputLink();
	}
}

void USeqAct_AIFactory::NotifySpawnerDisabled(AGearSpawner *Spawner)
{
	if (bActive)
	{
		// see if this is the only spawner we're allowed to use
		UBOOL bHasOtherSpawnPoints = FALSE;
		for (INT Idx = 0; Idx < SpawnPoints.Num() && !bHasOtherSpawnPoints; Idx++)
		{
			if (SpawnPoints(Idx) != NULL &&
				SpawnPoints(Idx) != Spawner)
			{
				AGearSpawner *TestSpawner = Cast<AGearSpawner>(SpawnPoints(Idx));
				if (TestSpawner == NULL ||
					TestSpawner->bActive)
				{
					bHasOtherSpawnPoints = TRUE;
				}
			}
		}
		if (!bHasOtherSpawnPoints)
		{
			INT NumUnspawnedPawns = 0;

			// mark the spawned count as the spawned total so that no further spawning occurs
			for (INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++)
			{
				FAISpawnInfo &Set = SpawnSets(SetIdx);
				INT UnspawnedFromSet = Set.SpawnTotal - Set.SpawnedCount;
				NumUnspawnedPawns += UnspawnedFromSet;
				Set.SpawnedCount = Set.SpawnTotal - Set.CurrentSpawns.Num();
			}

			// Inform the game object of the factory disabling
			AGearGame *GameObj = Cast<AGearGame>(GetWorldInfo()->Game);
			if( GameObj )
			{
				GameObj->eventAIFactoryDisabled( NumUnspawnedPawns, this );
			}

			OutputLinks(2).ActivateOutputLink();
		}
	}
}

/** Remove dead pawns from the watchlist */
void USeqAct_AIFactory::RemoveDeadSpawnsFromWatchList()
{
	for ( INT SetIdx = 0; SetIdx < SpawnSets.Num(); SetIdx++ )
	{
		FAISpawnInfo &Set = SpawnSets(SetIdx);
		for ( INT Idx = 0; Idx < Set.WatchList.Num(); Idx++ )
		{
			// cull out any null/dead entries
			// A DBNO player is not considered dead.
			if (Set.WatchList(Idx) == NULL ||
				Set.WatchList(Idx)->bDeleteMe ||
				Set.WatchList(Idx)->bPlayedDeath ||
				(Set.WatchList(Idx)->Health <= 0 && (!Set.WatchList(Idx)->IsA(AGearPawn::StaticClass()) || !((AGearPawn*)Set.WatchList(Idx))->IsDBNO())) )
			{
				SpawnHasDied( Set, Idx );
			}
		}
	}
}

/** Called when a spawn is killed */
void USeqAct_AIFactory::SpawnHasDied( FAISpawnInfo &SpawnSet, INT WatchListIdx )
{
	// Increment death count
	DeadCount++;

	APawn* KilledSpawn = SpawnSet.WatchList(WatchListIdx);

	// Remove the spawn from the watch list
	SpawnSet.WatchList.Remove(WatchListIdx--,1);

	// Inform the game object of the death
	AGearGame *GameObj = Cast<AGearGame>(GetWorldInfo()->Game);
	if( GameObj )
	{
		GameObj->eventSpawnFromAIFactoryHasDied( KilledSpawn, this );
	}
}

void USeqCond_IsCoop::Activated()
{
	UBOOL bFoundMarcus = FALSE, bFoundDom = FALSE;
	for (APawn *Pawn = GWorld->GetWorldInfo()->PawnList; Pawn != NULL; Pawn = Pawn->NextPawn)
	{
		if (Pawn->IsPlayerOwned())
		{
			if (Pawn->IsA(AGearPawn_COGMarcus::StaticClass()))
			{
				bFoundMarcus = TRUE;
			}
			else
			if (Pawn->IsA(AGearPawn_COGDom::StaticClass()))
			{
				bFoundDom = TRUE;
			}
			if (bFoundMarcus && bFoundDom)
			{
				break;
			}
		}
	}
	if (bFoundMarcus && bFoundDom)
	{
		OutputLinks(0).ActivateOutputLink();
	}
	else
	{
		OutputLinks(1).ActivateOutputLink();
	}
}


/**
 * Checks to see if AI is stunned (Berserker mainly)
 */
void USeqCond_IsStunned::Activated()
{
	UBOOL bStunned = FALSE;

	Super::Activated();

	TArray<UObject**> ObjVars;
	GetObjectVars(ObjVars,TEXT("Target"));

	for( INT Idx = 0; Idx < ObjVars.Num(); Idx++ )
	{
		AGearPawn* P = Cast<AGearPawn>(*(ObjVars(Idx)));
		if( !P )
		{
            AController *C = Cast<AController>(*(ObjVars(Idx)));
			if( C && C->Pawn )
			{
				P = Cast<AGearPawn>(C->Pawn);
			}
		}

		if( P && P->SpecialMove == SM_Berserker_Stunned )
		{
			bStunned = TRUE;
			break;
		}
	}

	if( bStunned )
	{
		OutputLinks(0).ActivateOutputLink();
	}
	else
	{
		OutputLinks(1).ActivateOutputLink();
	}
}

void USeqCond_HasInventory::Activated()
{
	UBOOL bHasInventory = FALSE;
	for (INT Idx = 0; Idx < Players.Num(); Idx++)
	{
		APawn *Pawn = Cast<APawn>(Players(Idx));
		if (Pawn == NULL)
		{
			AController *Controller = Cast<AController>(Players(Idx));
			if (Controller != NULL)
			{
				Pawn = Controller->Pawn;
			}
		}
		if (Pawn != NULL && Pawn->InvManager != NULL)
		{
			for (INT InvIdx = 0; InvIdx < InventoryTypes.Num(); InvIdx++)
			{
				if (Pawn->InvManager->eventFindInventoryType(InventoryTypes(InvIdx)) != NULL)
				{
					bHasInventory = TRUE;
					break;
				}
			}
		}
	}
	if (bHasInventory)
	{
		OutputLinks(1).ActivateOutputLink();
	}
	else
	{
		OutputLinks(0).ActivateOutputLink();
	}
}

void USeqCond_HasCOGTag::Activated()
{
	TArray<UObject**> PlayerVars;
	TArray<UObject**> TagVars;
	UBOOL bHasIt = FALSE;

	// find the tag linked in
	AGearCrimsonOmenPickupFactoryBase* Tag = NULL;
	{
		GetObjectVars(TagVars, TEXT("COGTag"));
		if (TagVars.Num() > 0)
		{
			Tag = Cast<AGearCrimsonOmenPickupFactoryBase>(*(TagVars(0)));
		}
	}

	if (Tag)
	{
		// find players linked in
		GetObjectVars(PlayerVars, TEXT("Player(s)"));

		INT const NumPlayerVars = PlayerVars.Num();
		
		for (INT Idx=0; Idx<NumPlayerVars; ++Idx)
		{
			// try to find a GearPC for the var
			AGearPC* WPC = NULL;
			{
				WPC = Cast<AGearPC>(*(PlayerVars(Idx)));
				if (!WPC)
				{
					// maybe it's a pawn, in which case we should use it's controller
					APawn* P = Cast<APawn>(*(PlayerVars(Idx)));
					if (P)
					{
						WPC = Cast<AGearPC>(P->Controller);
					}
				}
			}
		}
	}

	if (bHasIt)
	{
		OutputLinks(1).ActivateOutputLink();
	}
	else
	{
		OutputLinks(0).ActivateOutputLink();
	}
}


void USeqCond_HasDiscoverable::Activated()
{
	UBOOL bHasIt = TRUE;

	// find the tag linked in
	AGearDiscoverablesPickupFactoryBase* Tag = NULL;
	{
		TArray<UObject**> TagVars;
		GetObjectVars(TagVars, TEXT("Discoverable"));
		if (TagVars.Num() > 0)
		{
			Tag = Cast<AGearDiscoverablesPickupFactoryBase>(*(TagVars(0)));
		}
	}

	if (Tag)
	{
		TArray<UObject**> ObjVars;
		TArray<AGearPC*> PCTargets;

		// find players linked in
		GetObjectVars(ObjVars, TEXT("Player(s)"));
		BuildGearPCTargets(ObjVars, PCTargets);

		// returns true only if every passed in player needs it
		INT const NumPCTargets = PCTargets.Num();
		for (INT Idx=0; Idx<NumPCTargets; ++Idx)
		{
			if (!PCTargets(Idx)->eventHasFoundDiscoverable(Tag->DISC_DiscoverableType))
			{
				bHasIt = FALSE;
				break;
			}
		}
	}

	if (bHasIt)
	{
		OutputLinks(1).ActivateOutputLink();
	}
	else
	{
		OutputLinks(0).ActivateOutputLink();
	}
}



void USeqCond_PawnType::DeActivated()
{
	APawn *Pawn = Cast<APawn>(CheckPawn);
	UBOOL bFoundMatch = FALSE;
	if (Pawn != NULL)
	{
		// for each output link and pawn type
		for (INT Idx = 0; Idx < PawnTypes.Num() && Idx < OutputLinks.Num(); Idx++)
		{
			// check each class name associated with the link
			TArray<FName>& NameList = PawnTypes(Idx).PawnTypes;
			for (INT NameIdx = 0; NameIdx < NameList.Num(); NameIdx++)
			{
				const FName& CheckName = NameList(NameIdx);
				for( UClass* TempClass=Pawn->GetClass(); TempClass; TempClass=(UClass*)TempClass->SuperField )
				{
					if( TempClass->GetFName()==CheckName )
					{
						// activate the output and stop further checking
						bFoundMatch = TRUE;
						OutputLinks(Idx).ActivateOutputLink();
						break;
					}
				}	
			}
		}
	}
	// if no match was found,
	if (!bFoundMatch)
	{
		// activate the unknown link
		OutputLinks(7).ActivateOutputLink();
	}
}


/** 
 * Aligns the temporarily-spawned weapon such that the muzzle point of the weapon lies on the 
 * origin pt and points toward the target point.
 */
void USeqAct_DummyWeaponFire::AlignWeaponMuzzleToActor(AActor* AlignTo, AActor* AimAt)
{
	UBOOL bAlignedOk = FALSE;

	// Default to location of 'align to' actor
	FVector AlignToLocation = AlignTo->Location;
	// See if we specified a socket name
	if(OriginSocketName != NAME_None)
	{
		// See if CollisionComponent of origin actor is skelmeshcomp
		USkeletalMeshComponent* OriginMesh = Cast<USkeletalMeshComponent>(AlignTo->CollisionComponent);
		if(OriginMesh)
		{
			// Look for socket
			FVector SocketLocation(0,0,0);
			UBOOL bFound = OriginMesh->GetSocketWorldLocationAndRotation(OriginSocketName, SocketLocation, NULL);
			if(bFound)
			{
				// Found it - use that location
				AlignToLocation = SocketLocation;
			}
		}
	}

	// all we need is to put the muzzle socket on originactor.  Rotation of the SpawnedWeapon doesn't matter.
	if ( SpawnedWeapon != NULL )
	{
		if ( SpawnedWeapon->Mesh != NULL )
		{
			if ( SpawnedWeapon->MuzzleSocketName != NAME_None )
			{
				USkeletalMeshComponent* const SkelMesh = Cast<USkeletalMeshComponent>(SpawnedWeapon->Mesh);
				if (SkelMesh != NULL && SkelMesh->SkeletalMesh != NULL)
				{
					USkeletalMeshSocket* const MuzzleSocket = SkelMesh->SkeletalMesh->FindSocket(SpawnedWeapon->MuzzleSocketName);
					if (MuzzleSocket)
					{
						FVector WorldSocketLoc(0,0,0);
						SkelMesh->GetSocketWorldLocationAndRotation(SpawnedWeapon->MuzzleSocketName, WorldSocketLoc, NULL);
						FVector WorldSocketOffset = WorldSocketLoc - SpawnedWeapon->Location;

						FVector LocalSocketOffset = FRotationMatrix(SpawnedWeapon->Rotation).Transpose().TransformNormal( WorldSocketOffset );

						FRotator AimRot = (AimAt->Location - AlignToLocation).Rotation();
						SpawnedWeapon->SetRotation(AimRot);

						FVector WorldMuzzleOffset = FRotationMatrix(AimRot).TransformNormal( LocalSocketOffset );
						FVector NewLoc = AlignToLocation - WorldMuzzleOffset;

						SpawnedWeapon->SetLocation(NewLoc);

						bAlignedOk = TRUE;
					}
				}
			}
		}
	}

	if (!bAlignedOk && SpawnedWeapon)
	{
		// if no mesh, assume muzzle componets are attached to spawnedweapon directly
		FRotator AimRot = (AimAt->Location - AlignToLocation).Rotation();
		SpawnedWeapon->SetRotation(AimRot);
		SpawnedWeapon->SetLocation(AlignToLocation);
	}
}

void USeqAct_DummyWeaponFire::SpawnDummyWeapon(AActor* OriginActor, AActor* TargetActor)
{
	// spawn the temporary weapon.  location doesn't matter, we'll align it later
	SpawnedWeapon = Cast<AGearWeapon>(GWorld->SpawnActor(WeaponClass, NAME_None, FVector(0.f), FRotator(0,0,0), NULL, TRUE, FALSE, NULL, NULL));
	if (SpawnedWeapon)
	{
		if (SpawnedWeapon->Mesh)
		{
			SpawnedWeapon->AttachComponent(SpawnedWeapon->Mesh);
			SpawnedWeapon->Mesh->SetHiddenGame(TRUE);
		}
		SpawnedWeapon->bHidden = FALSE;
		SpawnedWeapon->eventAttachMuzzleEffectsComponents(Cast<USkeletalMeshComponent>(SpawnedWeapon->Mesh));
		AlignWeaponMuzzleToActor(OriginActor, TargetActor);
		SpawnedWeapon->bSuppressMuzzleFlash = bSuppressMuzzleFlash;
		SpawnedWeapon->bSuppressTracers = bSuppressTracers;
		SpawnedWeapon->bSuppressImpactFX = bSuppressImpactFX;
		SpawnedWeapon->bSuppressAudio = bSuppressAudio;
		SpawnedWeapon->bSuppressDamage = bSuppressDamage;
		RemainingFireTime = 0.f;
		ShotsFired = 0;
	}
}

/** Helper function to return a random float in the range defined by a 2d vector. */
static FLOAT RandInRange(FVector2D const& RangeVec)
{
	return RangeVec.X + (RangeVec.Y - RangeVec.X) * appFrand();
}

/** Ticks object list processing.  Cycles objects when necessary, etc. */
void USeqAct_DummyWeaponFire::UpdateObjectList(FDummyFireObjectListParams& ListParams, TArray<UObject**> Objects, FLOAT DeltaTime)
{
	INT NumObjects = Objects.Num();

	if (NumObjects <= 1)
	{
		ListParams.CurrentObjIdx = 0;
	}
	else
	{
		UBOOL bChooseNewTarget = FALSE;

		if (ListParams.bDelay)
		{
			ListParams.DelayTimeRemaining -= DeltaTime;

			// delay finished?
			if (ListParams.DelayTimeRemaining <= 0.f)
			{
				ListParams.bDelay = FALSE;
				bChooseNewTarget = TRUE;
			}
		}
		else
		{
			ListParams.TimeUntilObjectChange -= DeltaTime;

			// time to switch?
			if (ListParams.TimeUntilObjectChange < 0.f)
			{
				// do we need a delay?
				FLOAT const DelayTime = RandInRange(ListParams.ObjectChangeDelay);
				if (DelayTime > 0.f)
				{
					ListParams.bDelay = TRUE;
					ListParams.DelayTimeRemaining = DelayTime;
				}
				else
				{
					bChooseNewTarget = TRUE;
				}
			}
		}

		if (bChooseNewTarget)
		{
			switch (ListParams.CyclingMethod)
			{
			case DFOCM_Sequential:
				ListParams.CurrentObjIdx = (ListParams.CurrentObjIdx + 1) % NumObjects;
				break;
			case DFOCM_Random:
				ListParams.CurrentObjIdx = appRand() % NumObjects;
				break;
			}

			ListParams.TimeUntilObjectChange = RandInRange(ListParams.SecondsPerObject);
		}
	}
}

/** Returns the currently active actor from an object list, or NULL if no actors are active. */
AActor* USeqAct_DummyWeaponFire::GetCurrentActorFromObjectList(FDummyFireObjectListParams const& ListParams, TArray<UObject**> Objects) const
{
	INT NumObjects = Objects.Num();
	if ( (NumObjects <= 0) || (ListParams.bDelay) )
	{
		// empty list
		return NULL;
	}
	else
	{
		INT ObjIdx = Clamp(ListParams.CurrentObjIdx, 0, NumObjects);  // just in case
		UObject* const Obj = *Objects(ObjIdx);

		APlayerController* const PC = Cast<APlayerController>(Obj);
		if (PC != NULL)
		{
			// return PlayerController's pawn, if possible
			if (PC->Pawn)
			{
				return PC->Pawn;
			}
			else
			{
				return PC;
			}
		}
		else
		{
			return Cast<AActor>(*(Objects(ObjIdx)));
		}
	}
}

void USeqAct_DummyWeaponFire::Activated()
{
	Super::Activated();

	if (InputLinks(0).bHasImpulse)
	{
		// Start Firing input
		
		bStopped = FALSE;
		bFinished = FALSE;

		// set up OriginActor and TargetActor vars from the input links
		AActor* TargetActor = NULL;
		AActor* OriginActor = NULL;
		{
			TArray<UObject**> TargetVars;
			TArray<UObject**> OriginVars;
			GetObjectVars(TargetVars, TEXT("Target"));
			if (TargetVars.Num() > 0)
			{
				// is it an actor?
				TargetActor = Cast<AActor>(*(TargetVars(0)));
				if (Cast<AController>(TargetActor) != NULL)
				{
					TargetActor = Cast<AController>(TargetActor)->Pawn;
				}
			}
			GetObjectVars(OriginVars, TEXT("Origin"));
			if (OriginVars.Num() > 0)
			{
				// is it an actor?
				OriginActor = Cast<AActor>(*(OriginVars(0)));
			}
		}

		if (TargetActor && OriginActor)
		{
			SpawnDummyWeapon(OriginActor, TargetActor);
		}

		if (ReplicatedActor == NULL && (GWorld->GetNetMode() == NM_DedicatedServer || GWorld->GetNetMode() == NM_ListenServer))
		{
			ReplicatedActor = (ADummyWeaponFireActor*)GWorld->SpawnActor(ADummyWeaponFireActor::StaticClass());
			if (ReplicatedActor != NULL)
			{
				ReplicatedActor->FireAction = this;
			}
		}
	}
	else
	{
		// Stop Firing input
		if (SpawnedWeapon)
		{
			SpawnedWeapon->eventWeaponStoppedFiring(FiringMode);
			bStopped = TRUE;
		}
	}

	// activate base output
	OutputLinks(0).ActivateOutputLink();
}

UBOOL USeqAct_DummyWeaponFire::UpdateOp(FLOAT DeltaTime)
{
	if (bStopped)
	{
		// we've already been stopped, just bail so we can deactivate
		return TRUE;
	}
	else if (InputLinks(0).bHasImpulse)
	{
		if (!bActive)
		{
			Activated();
		}
	}
	else if (InputLinks(1).bHasImpulse)
	{
		if (SpawnedWeapon)
		{
			SpawnedWeapon->eventWeaponStoppedFiring(FiringMode);
			bStopped = TRUE;
		}
		return TRUE;
	}
	else
	{
		AActor* TargetActor = NULL;
		AActor* OriginActor = NULL;
		{
			TArray<UObject**> TargetVars;
			GetObjectVars(TargetVars, TEXT("Target"));
			UpdateObjectList(MultipleTargetParams, TargetVars, DeltaTime);
			TargetActor = GetCurrentActorFromObjectList(MultipleTargetParams, TargetVars);

			TArray<UObject**> OriginVars;
			GetObjectVars(OriginVars, TEXT("Origin"));
			UpdateObjectList(MultipleOriginParams, OriginVars, DeltaTime);
			OriginActor = GetCurrentActorFromObjectList(MultipleOriginParams, OriginVars);

			// one of the connections is empty, just bail
			if ( (TargetVars.Num() == 0) || (OriginVars.Num() == 0) )
			{
				return TRUE;
			}
		}

		if (OriginActor && TargetActor)
		{
			AlignWeaponMuzzleToActor(OriginActor, TargetActor);
	
			if (SpawnedWeapon && TargetActor)
			{
				RemainingFireTime -= DeltaTime;
				if (RemainingFireTime <= 0.f)
				{
					if ( (ShotsFired >= ShotsToFire) && (!bShootUntilStopped) )
					{
						SpawnedWeapon->eventWeaponStoppedFiring(FiringMode);
						bFinished = TRUE;
						return TRUE;
					}
					else
					{
						SpawnedWeapon->eventDummyFire(FiringMode, TargetActor->Location, OriginActor, InaccuracyDegrees, TargetActor);
						RemainingFireTime = 60.f / SpawnedWeapon->WeaponRateOfFire;
						ShotsFired++;
						bFiring = TRUE;
						if (ReplicatedActor != NULL)
						{
							ReplicatedActor->eventNotifyShotFired(OriginActor, TargetActor);
						}
					}
				}
			}
			else
			{
				// no target or no weapon to shoot, bail
				return TRUE;
			}
		}
		else
		{
			if (bFiring)
			{
				if (SpawnedWeapon != NULL)
				{
					SpawnedWeapon->eventWeaponStoppedFiring(FiringMode);
				}
				bFiring = FALSE;
			}
		}
	}

	return FALSE;
}

void USeqAct_DummyWeaponFire::DeActivated()
{
	// clean house
	if (SpawnedWeapon)
	{
		GWorld->DestroyActor(SpawnedWeapon);
	}
	if (ReplicatedActor != NULL)
	{
		GWorld->DestroyActor(ReplicatedActor);
	}

	// activate appropriate output
	if (bStopped)
	{
		OutputLinks(2).ActivateOutputLink();
	}
	else if (bFinished)
	{
		OutputLinks(1).ActivateOutputLink();
	}
}

void USeqAct_DummyWeaponFire::CleanUp()
{
	Super::CleanUp();

	// make sure we clean up actors we spawn (never know if LDs going to leave us running during map unload...)
	if (SpawnedWeapon != NULL)
	{
		GWorld->DestroyActor(SpawnedWeapon);
	}
	if (ReplicatedActor != NULL)
	{
		GWorld->DestroyActor(ReplicatedActor);
	}
}

/**
 * Executes the action when it is triggered 
 */
void USeqAct_ControlGearsMovie::Activated()
{
	if (InputLinks(0).bHasImpulse)
	{
//		debugf(TEXT("PLAY *****************************************************    USeqAct_ControlGearsMovie =%s    *****************************************************"), 			*MovieName );

		// inform all clients
		UBOOL bFoundLocalPlayer = FALSE;
		for (AController* C = GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
		{
			AGearPC* PC = Cast<AGearPC>(C);
			if (PC != NULL)
			{
				bFoundLocalPlayer = bFoundLocalPlayer || (PC->LocalPlayerController() != NULL);
				PC->eventClientPlayMovie(MovieName);
			}
		}

		// play it locally if no local player controller (e.g. dedicated server)
		if( !bFoundLocalPlayer &&
			GFullScreenMovie )
		{
			// No local players, and we don't allow clients to pause movies
			UINT MovieFlags = MM_PlayOnceFromStream | MF_OnlyBackButtonSkipsMovie;

			GFullScreenMovie->GameThreadPlayMovie( ( EMovieMode )MovieFlags, *MovieName);
		}
	}
	else
	{
//		debugf(TEXT("STOP *****************************************************    USeqAct_ControlGearsMovie =%s    *****************************************************"), 			*MovieName );

		// inform all clients
		UBOOL bFoundLocalPlayer = FALSE;
		for (AController* C = GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
		{
			AGearPC* PC = Cast<AGearPC>(C);
			if (PC != NULL)
			{
				bFoundLocalPlayer = bFoundLocalPlayer || (PC->LocalPlayerController() != NULL);
				PC->eventClientStopMovie(0, FALSE, FALSE, FALSE);
			}
		}
		// stop it locally if no local player controller (e.g. dedicated server)
		if( !bFoundLocalPlayer &&
			GFullScreenMovie )
		{
			GFullScreenMovie->GameThreadStopMovie();
		}
	}
}


UBOOL USeqAct_WaitForGearsMovieComplete::UpdateOp(FLOAT DeltaTime)
{
	UBOOL bMovieComplete = FALSE;
	check(GFullScreenMovie);

	FString LastMovieName = GFullScreenMovie->GameThreadGetLastMovieName();
	// strip extension off the filename
	MovieName = FFilename(MovieName).GetBaseFilename();
	// If we are playing the loading move, say we are done
	if( LastMovieName == UCONST_LOADING_MOVIE ||
		// If a name was specified, but that is not what is playing, say we are done
		(MovieName != TEXT("") && FFilename(MovieName).GetBaseFilename() != LastMovieName) ||
		// If we are playing a movie we are interested in, and its done
		GFullScreenMovie->GameThreadIsMovieFinished(*MovieName) )
	{
		bMovieComplete = TRUE;
	}

	// activate the output if the movie finished
	if (bMovieComplete)
	{
		OutputLinks(0).ActivateOutputLink();
		// make sure the movie is stopped for everyone
		for (AController* C = GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
		{
			AGearPC* PC = Cast<AGearPC>(C);
			if (PC != NULL && !PC->LocalPlayerController())
			{
				PC->eventClientStopMovie(0, FALSE, FALSE, FALSE);
			}
		}
	}
	
	// stop ticking if we are done with the movie
	return bMovieComplete;
}

void USeqAct_TriggerGUDSEvent::Activated()
{
	AGearGame* const WG = Cast<AGearGame>(GetWorldInfo()->Game);
	
	if (WG)
	{
		// find instigator and recipient
		APawn* Instigator = NULL;
		APawn* Recipient = NULL;
		{
			TArray<UObject**> InstigatorVars;
			TArray<UObject**> RecipientVars;

			GetObjectVars(InstigatorVars, TEXT("Instigator"));
			if (InstigatorVars.Num() > 0)
			{
				// is it a pawn?
				Instigator = Cast<APawn>(*(InstigatorVars(0)));

				// maybe it's a controller?
				if (Instigator == NULL)
				{
					// maybe it's a controller, in which case we should get the pawn
					AController* const Controller = Cast<AController>(*(InstigatorVars(0)));
					if (Controller)
					{
						Instigator = Cast<APawn>(Controller->Pawn);
					}
				}
			}

			GetObjectVars(RecipientVars, TEXT("Recipient"));
			if (RecipientVars.Num() > 0)
			{
				// is it an actor?
				Recipient = Cast<APawn>(*(RecipientVars(0)));

				// maybe it's a controller?
				if (Recipient == NULL)
				{
					// maybe it's a controller, in which case we should get the pawn
					AController* const Controller = Cast<AController>(*(RecipientVars(0)));
					if (Controller)
					{
						Recipient = Cast<APawn>(Controller->Pawn);
					}
				}
			}
		}

		WG->eventTriggerGUDEvent(EventID, Instigator, Recipient, DelaySeconds);
	}
}

void USeqAct_SetGUDSFrequency::Activated()
{
	AGearGame* const GG = Cast<AGearGame>(GetWorldInfo()->Game);

	if (GG && GG->UnscriptedDialogueManager)
	{
		GG->UnscriptedDialogueManager->GlobalChanceToPlayMultiplier = GlobalFrequencyMultiplier;
	}
}

//========================================
// USeqAct_KillPlayers interface
void USeqAct_KillPlayers::Activated()
{
	TArray<AController*> Players;
	// build the list from what was specified
	if (Targets.Num() > 0)
	{
		for (INT Idx = 0; Idx < Targets.Num(); Idx++)
		{
			AController *Player = Cast<AController>(Targets(Idx));
			if (Player == NULL)
			{
				APawn *Pawn = Cast<APawn>(Targets(Idx));
				if (Pawn != NULL)
				{
					Player = Pawn->Controller;
				}
			}
			if (Player != NULL)
			{
				Players.AddUniqueItem(Player);
			}
		}
	}
	else
	{
		// otherwise use the bools specified
		for (AController *Controller = GWorld->GetFirstController(); Controller != NULL; Controller = Controller->NextController)
		{
			if (bKillEnemies && Controller->GetTeamNum() == 1)
			{
				Players.AddUniqueItem(Controller);
			}
			else
			if (bKillFriends && Controller->GetTeamNum() == 0 && !Controller->IsA(APlayerController::StaticClass()))
			{
				Players.AddUniqueItem(Controller);
			}
			else
			if (bKillPlayers && Controller->IsA(APlayerController::StaticClass()))
			{
				Players.AddUniqueItem(Controller);
			}
		}
	}

	AGearGRI* const GRI = Cast<AGearGRI>(GetWorldInfo()->GRI);
	if( GRI != NULL)
	{
		GRI->eventKillPlayers(Players,bShouldGib);
	}
}

void USeqAct_AIMove::OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex )
{
	Super::OnReceivedImpulse( ActivatorOp, InputLinkIndex );

	if( bActive && InputLinkIndex == 0 )
	{
		PublishLinkedVariableValues();
		ActivateCount++;
		Activated();
		eventActivated();
	}
}

void USeqAct_AIMove::PreActorHandle(AActor *Actor)
{
	AGearAI *AI = Cast<AGearAI>(Actor);
	if (AI != NULL)
	{
		AITargets.AddUniqueItem(AI);
		if (OutputLinks.Num() > 2)
		{
			// activate the moving output
			OutputLinks(2).ActivateOutputLink();
		}
	}
	Super::PreActorHandle(Actor);
}

UBOOL USeqAct_AIMove::UpdateOp(FLOAT DeltaTime)
{
	// If some AI have reached their goal this frame
	if( AIReachedGoal.Num() > 0 )
	{
		// Publish the AI to the object list
		for (INT VarIdx = 0; VarIdx < VariableLinks.Num(); VarIdx++)
		{
			if (VariableLinks(VarIdx).ExpectedType == USeqVar_ObjectList::StaticClass())
			{
				for( INT LinkedVarIdx = 0; LinkedVarIdx < VariableLinks(VarIdx).LinkedVariables.Num(); LinkedVarIdx++ )
				{
					USeqVar_ObjectList *ObjList = Cast<USeqVar_ObjectList>(VariableLinks(VarIdx).LinkedVariables(LinkedVarIdx));
					if( ObjList != NULL )
					{
						// First clear list
						ObjList->ObjList.Empty();
						// Add each AI that reached the goal to the object list
						for( INT ReachIdx = 0; ReachIdx < AIReachedGoal.Num(); ReachIdx++ )
						{
							if( AIReachedGoal(ReachIdx) == NULL )
								continue;
							ObjList->ObjList.AddItem(AIReachedGoal(ReachIdx));
						}				
					}
				}
				break;
			}			
		}
		// Activate "reached goal" output
		OutputLinks(1).ActivateOutputLink();

		// Empty pending list
		AIReachedGoal.Empty();

		// Keep operation around for at least one more tick
		return FALSE;
	}

	for (INT Idx = 0; Idx < AITargets.Num(); Idx++)
	{
		AGearAI *AI = AITargets(Idx);
		if (AI == NULL || 
			AI->MoveAction != this ||
			AI->Pawn == NULL ||
			AI->Pawn->Health <= 0)
		{
			AITargets.Remove(Idx--,1);
		}
	}
	return (AITargets.Num() == 0);
}

void USeqAct_AIMove::ReachedGoal(AGearAI *AI)
{
	AIReachedGoal.AddUniqueItem( AI );
	AITargets.RemoveItem(AI);
}

void USeqAct_AIMove::DeActivated()
{
	OutputLinks(0).ActivateOutputLink();
}

void USeqAct_ToggleHOD::Activated()
{
	AGearGRI* const GRI = Cast<AGearGRI>(GetWorldInfo()->GRI);

	if( GRI != NULL )
	{
		if( GRI->HODBeamManager != NULL )
		{
			GRI->HODBeamManager->eventSetEnabled(InputLinks(0).bHasImpulse, bSuppressAlert);
		}
		// go ahead and spawn the HOD Manager
		else
		{
			UClass* const BeamMgrClass = FindObject<UClass>(ANY_PACKAGE, TEXT("HOD_BeamManager"));
			if (BeamMgrClass)
			{
				GRI->HODBeamManager = Cast<AHOD_BeamManagerBase>(GWorld->SpawnActor(BeamMgrClass));
				if (GRI->HODBeamManager)
				{
					GRI->HODBeamManager->eventSetEnabled(InputLinks(0).bHasImpulse, bSuppressAlert);
				}
			}
		}
	}
}


void USeqAct_NotifyCoopSplit::Activated()
{
	AGearGameSP_Base* const WG = Cast<AGearGameSP_Base>(GetWorldInfo()->Game);
	if (WG)
	{
		WG->bInCoopSplit = InputLinks(0).bHasImpulse;
		WG->bInSoloSplit = WG->bInCoopSplit ? bSoloSplit : FALSE;
	}
}

void USeqEvt_WDODamageModApplied::PostEditChange(UProperty* PropertyThatChanged)
{
	AGearDestructibleObject const* const WDO = Cast<AGearDestructibleObject>(Originator);
	if (WDO)
	{
		FDestructibleSubobject const* SubObj = NULL;
		FObjectDamageModifier const* Mod = NULL;

		// validate sub object name
		if (SubObjectName != NAME_None)
		{
			// make sure SubObjectName is a valid one, NULL it out if it isn't
			for (INT Idx=0; Idx<WDO->SubObjects.Num(); ++Idx)
			{
				if (WDO->SubObjects(Idx).SubObjName == SubObjectName)
				{
					SubObj = &WDO->SubObjects(Idx);
					break;
				}
			}
		}
		if (SubObj == NULL)
		{
			// entered name was invalid, NULL it
			SubObjectName = NAME_None;
		}

		// validate mod name
		if ( SubObj && (DamageModName != NAME_None) )
		{
			for (INT Idx=0; Idx<SubObj->DamageMods.Num(); ++Idx)
			{
				if (DamageModName == SubObj->DamageMods(Idx).DamageModName)
				{
					Mod = &SubObj->DamageMods(Idx);
				}
			}
		}
		if (Mod == NULL)
		{
			DamageModName = NAME_None;
		}

		// set the title bar
		if (SubObj && Mod)
		{
			ObjName = FString::Printf( TEXT("%s (%s,%s) %s"), *WDO->GetName(), *SubObjectName.ToString(), *DamageModName.ToString(), *((USequenceObject*)GetClass()->GetDefaultObject())->ObjName );
		}
		else
		{
			ObjName = FString::Printf( TEXT("%s %s"), *WDO->GetName(), *((USequenceObject*)GetClass()->GetDefaultObject())->ObjName );
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Executes the action when it is triggered 
 */

FVector Debug_HangLoc;
UBOOL USeqAct_StreamByURL::UpdateOp(FLOAT DeltaTime)
{
	// @todo: Share this value with GearCheckpointManager.cpp
	const INT NUM_WEAPONS_TO_SAVE = 4;
	const INT NUM_PAWNS_TO_SAVE = 2;

	// only works in-game
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine)
	{
		if (Stage == 0)
		{
			// @hack: something to play a movie if we didn't go through the UI action
			if( GFullScreenMovie &&
				GFullScreenMovie->GameThreadIsMovieFinished(TEXT("")) )
			{
				GFullScreenMovie->GameThreadPlayMovie(MM_LoopFromMemory, UCONST_LOADING_MOVIE);
			}

			// Disable rumble to make sure the first frame doesn't start a rumble that keeps going while we are paused streaming textures.
			for(INT i=0; i<GEngine->GamePlayers.Num(); i++)
			{
				ULocalPlayer* LP = GEngine->GamePlayers(i);
				if(LP && LP->Actor)
				{
					LP->Actor->ForceFeedbackManager->bIsPaused = TRUE;
				}
			}

			// we're done if we get here (but leaving in Stage for Amitt's async changes)
			//Stage = 100;

			// wait a coupla frames, then pause
			Stage = 98;
		}

		// move on to next stage
		Stage++;
	}
	// if not in game engine, then just skip to end
	else
	{
		Stage = 100;
	}

	// are we done?
	// @todo Amitt, fix this
	UBOOL bIsFinished = Stage >= 100;
	if (bIsFinished)
	{
		if (OutputLinks.Num())
		{
			// activate output link and stop this crazy thing
			OutputLinks(0).ActivateOutputLink();
		}

		// reset the stage so we can go again
		Stage = 0;

		// leave movie playing longer with game paused for textures to stream in.
		// pause while streaming textures.
		if(GEngine && GEngine->GamePlayers(0) && GEngine->GamePlayers(0)->Actor)
		{
			AGearPC* WPC = CastChecked<AGearPC>(GEngine->GamePlayers(0)->Actor);
			WPC->eventWarmupPause(TRUE);
			check(GWorld->IsPaused());
		}
	}

	return bIsFinished;
}


void USeqAct_CauseExplosion::Activated()
{
	TArray<UObject**> TargetVars;
	GetObjectVars(TargetVars, TEXT("Target"));

	FVector ExploLoc;
	FRotator ExploRot;	
	UBOOL bGoodTarget = FALSE;
	if (TargetVars.Num() > 0)
	{
		// is it a pawn?
		AActor* TargetActor = Cast<AActor>(*(TargetVars(0)));
		if (TargetActor)
		{
			ExploLoc = TargetActor->Location;
			ExploRot = TargetActor->Rotation;
			bGoodTarget = TRUE;
		}
	}

	if (bGoodTarget)
	{
		if (CustomExplosionTemplate)
		{
			// explode using the given template
			AGearExplosionActorReplicated *ExploActor = Cast<AGearExplosionActorReplicated>(GWorld->SpawnActor(AGearExplosionActorReplicated::StaticClass(), NAME_None, ExploLoc, ExploRot, NULL,TRUE,FALSE,NULL,NULL));
			if (ExploActor)
			{
				ExploActor->eventExplode(CustomExplosionTemplate);
			}
		}
		else if (ProjectileExplosionToEmulate)
		{
			// emulate the explosion of a certian projectile class
			AGearExplosionActorReplicated *ExploActor = Cast<AGearExplosionActorReplicated>(GWorld->SpawnActor(AGearExplosionActorReplicated::StaticClass(), NAME_None, ExploLoc, ExploRot, NULL,TRUE,FALSE,NULL,NULL));
			if (ExploActor)
			{
				ExploActor->eventEmulateProjectileExplosion(ProjectileExplosionToEmulate);
			}
		}
	}
}

/*****************************************************************************
 USeqAct_GetMPGameType - Impulses the the gametype being played, send 0 for Error for newly unsupported gametypes
 @TODO: Once we have moved the gametypes into the main package via base classes we should cast check instead of string testing.
*****************************************************************************/
IMPLEMENT_CLASS(USeqAct_GetMPGameType)

/** Activated will perform all of the necessary tasks for this action. */
void USeqAct_GetMPGameType::Activated()
{
	for ( INT i = 0; i < OutputLinks.Num(); i++ )
	{
		OutputLinks(i).bHasImpulse = FALSE;
	}

	INT OutputLinkID = 0;

	AGameInfo* GameObj = GWorld->GetWorldInfo()->Game;
	if ( GameObj )
	{
		CurrentGameClass = GameObj->GetClass();

		// Can be either Warzone or Execution
		if ( appStricmp(*GameObj->GetClass()->GetName(), TEXT("GearGameTDM")) == 0 )
		{
			AGearGRI* const GRI = Cast<AGearGRI>(GetWorldInfo()->GRI);
			if ( GRI && GRI->bGameIsExecutionRules )
			{
				OutputLinkID = 8;
			}
			else
			{
				OutputLinkID = 1;
			}
		}
		else if ( appStricmp(*GameObj->GetClass()->GetName(), TEXT("GearGameKTL")) == 0 )
		{
			OutputLinkID = 2;
		}
		// Can be either Annex or KOTH
		else if ( appStricmp(*GameObj->GetClass()->GetName(), TEXT("GearGameAnnex")) == 0 )
		{
			AGearGRI* const GRI = Cast<AGearGRI>(GetWorldInfo()->GRI);
			if ( GRI && GRI->bAnnexIsKOTHRules )
			{
				OutputLinkID = 7;
			}
			else
			{
				OutputLinkID = 3;
			}
		}
		else if ( appStricmp(*GameObj->GetClass()->GetName(), TEXT("GearGameHorde")) == 0 )
		{
			OutputLinkID = 4;
		}
		else if ( appStricmp(*GameObj->GetClass()->GetName(), TEXT("GearGameWingman")) == 0 )
		{
			OutputLinkID = 5;
		}
		else if ( appStricmp(*GameObj->GetClass()->GetName(), TEXT("GearGameCTM")) == 0 )
		{
			OutputLinkID = 6;
		}
		AGearGameMP_Base *MPGame = Cast<AGearGameMP_Base>(GameObj);
		if (MPGame != NULL && MPGame->TrainingGroundsID != -1 && OutputLinks.Num() > 8)
		{
			OutputLinks(9).ActivateOutputLink();
		}
	}

	OutputLinks(OutputLinkID).ActivateOutputLink();
}

/*****************************************************************************
 USeqEvt_MPRoundStart - Send an event when an MP game starts the round 
 countdown and when the round actually starts.
*****************************************************************************/
IMPLEMENT_CLASS(USeqEvt_MPRoundStart)

/** Registers the event since the game object doesn't exist in the editor to attach the event to. */
UBOOL USeqEvt_MPRoundStart::RegisterEvent()
{
	// register with the game object
	AGameInfo *GameObj = GWorld->GetWorldInfo()->Game;
	if ( GameObj )
	{
		KISMET_LOG(TEXT("SeqEvt_MPRoundStart event %s registering with GameInfo %s"),*GetFullName(),*GameObj->GetName());
		Originator = GameObj;
		Originator->GeneratedEvents.AddUniqueItem(this);
		eventRegisterEvent();
		bRegistered = TRUE;
	}

	return bRegistered;
}

/*****************************************************************************
 USeqEvt_MPRoundEnd - Send an event when an MP game ends the round 
*****************************************************************************/
IMPLEMENT_CLASS(USeqEvt_MPRoundEnd)

/** Registers the event since the game object doesn't exist in the editor to attach the event to. */
UBOOL USeqEvt_MPRoundEnd::RegisterEvent()
{
	// register with the game object
	AGameInfo *GameObj = GWorld->GetWorldInfo()->Game;
	if ( GameObj )
	{
		KISMET_LOG(TEXT("Input event %s registering with GameInfo %s"),*GetFullName(),*GameObj->GetName());
		Originator = GameObj;
		Originator->GeneratedEvents.AddUniqueItem(this);
		eventRegisterEvent();
		bRegistered = TRUE;
	}

	return bRegistered;
}

/*****************************************************************************
 USeqAct_Countdown - Starts a countdown on all player's HUDs.
 Inputs:  Start - Start the countdown
          Stop - Stop the countdown
 Outputs: Out - Called after the timer was started
          Stopped - Called after the countdown was stopped
		  Expired - Called when the countdown has expired
*****************************************************************************/
IMPLEMENT_CLASS(USeqAct_Countdown)

/** Called when this action goes active.  It will either turn the coundown on or off. */
void USeqAct_Countdown::Activated()
{
	AGearGRI* const GRI = Cast<AGearGRI>(GetWorldInfo()->GRI);

	if ( GRI )
	{
		// start countdown
		if ( InputLinks(0).bHasImpulse )
		{
			InputLinks(0).bHasImpulse = FALSE;

			// start the countdown
			GRI->eventStartCountdown( TotalCountdownInSeconds );

			// activate "out" output
			OutputLinks(0).ActivateOutputLink();
		}
		// stop countdown
		else if ( InputLinks(1).bHasImpulse )
		{
			InputLinks(1).bHasImpulse = FALSE;

			// stop countdown if it's actually been turned on
			if ( GRI->ECDState != eCDS_None )
			{
				// stop the countdown
				GRI->eventStopCountdown();
			}
		}
	}
}

/** Called every tick and monitors the GRI to see if it should send any impulses */
UBOOL USeqAct_Countdown::UpdateOp( FLOAT DeltaTime )
{
	AGearGRI* const GRI = Cast<AGearGRI>(GetWorldInfo()->GRI);

	if ( GRI )
	{
		// if the countdown has no state bail
		if ( GRI->ECDState == eCDS_None )
		{
			InputLinks(0).bHasImpulse = FALSE;
			InputLinks(1).bHasImpulse = FALSE;
			return TRUE;
		}
		// Catch a countdown reset and stop
		else if ( InputLinks(0).bHasImpulse || InputLinks(1).bHasImpulse )
		{
			Activated();
		}
		// Countdown stopped
		else if ( GRI->ECDState == eCDS_Stopped )
		{
			// activate "Stopped" output
			OutputLinks(1).ActivateOutputLink();
			GRI->ECDState = eCDS_None;
			return TRUE;
		}
		// Countdown expired
		else if ( GRI->ECDState == eCDS_Expired )
		{
			// activate "Expired" output
			OutputLinks(2).ActivateOutputLink();
			GRI->ECDState = eCDS_None;
			return TRUE;
		}
	}

	return FALSE;
}

/*****************************************************************************
 SeqAct_ManageObjectives - Manages the objective system

 Inputs:
	Add Objective - Adds an objective to the system
	Completed Objective - Completes an already existing objective
	Failed Objective - Fails an already existing objective
	Abort Objective - Silently aborts an already existing objective
	Clear All Objectives - Silently clears all objectives from the system

 Outputs:
	Out - Just an action pass through
*****************************************************************************/
IMPLEMENT_CLASS(USeqAct_ManageObjectives)

/** Called when this action goes active. */
void USeqAct_ManageObjectives::Activated()
{
	UBOOL bAddedPlayers = FALSE;

	// If there are no targets then we will send this action to all player controllers
	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to all players
		for ( AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
		{
			if ( Controller->IsPlayerOwned() )
			{
				Targets.AddItem( Controller );
			}
		}
	}

	// Super class will send the action to all Targets
	Super::Activated();

	// If no targets were provided and we sent the action to all player controllers, empty the Targets array now.
	if ( bAddedPlayers )
	{
		Targets.Empty();
	}
}

/*****************************************************************************
 * SeqAct_GearAchievementUnlock - Kismet action that allows achievements to be unlocked
 * by events that occur in levels.  This action will attempt to unlock the achievement
 * for all players.
 *****************************************************************************/
IMPLEMENT_CLASS(USeqAct_GearAchievementUnlock)

/** Called when this action goes active. */
void USeqAct_GearAchievementUnlock::Activated()
{
	UBOOL bAddedPlayers = FALSE;

	// If there are no targets then we will send this action to all player controllers
	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to all players
		for ( AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
		{
			if ( Controller->IsPlayerOwned() )
			{
				Targets.AddItem( Controller );
			}
		}
	}

	// Super class will send the action to all Targets
	Super::Activated();

	// If no targets were provided and we sent the action to all player controllers, empty the Targets array now.
	if ( bAddedPlayers )
	{
		Targets.Empty();
	}
}

/*****************************************************************************
 * SeqAct_GearUnlockableUnlock - Kismet action that allows unlockables to be unlocked
 * by events that occur in levels.  This action will attempt to unlock the unlockable
 * for all players.
 *****************************************************************************/
IMPLEMENT_CLASS(USeqAct_GearUnlockableUnlock)

/** Called when this action goes active. */
void USeqAct_GearUnlockableUnlock::Activated()
{
	UBOOL bAddedPlayers = FALSE;

	// If there are no targets then we will send this action to all player controllers
	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to all players
		for ( AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
		{
			if ( Controller->IsPlayerOwned() )
			{
				Targets.AddItem( Controller );
			}
		}
	}

	// Super class will send the action to all Targets
	Super::Activated();

	// If no targets were provided and we sent the action to all player controllers, empty the Targets array now.
	if ( bAddedPlayers )
	{
		Targets.Empty();
	}
}

/*****************************************************************************
 SeqAct_ManageTutorials - Manages the tutorial system

 Inputs:
	Start Tutorial - Adds a tutorial to the system
	Stop Tutorial - Removes a tutorial from the system but does not complete it
	Complete Tutorial - Completes the tutorial and removes it from the system
	System On - Turns the tutorial system on, which will additionally load all the auto-initialized tutorials
	System Off - Turns the tutorial system off and removes all existing tutorials from the system

 Outputs:
	Out - Just an action pass through
	Completed - Level Designer way of completed a tutorials - completion can also happen from logic within the tutorial itself
	Stopped - Level Designer way of having a tutorial removed but not completed

*****************************************************************************/
IMPLEMENT_CLASS(USeqAct_ManageTutorials)

/** Called when this action goes active. */
void USeqAct_ManageTutorials::Activated()
{
	UBOOL bAddedPlayers = FALSE;

	// Early out if this tutorial is dead
	if ( bTutorialIsDead )
	{
		return;
	}

	// If there are no targets then we will send this action to all player controllers
	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to all players
		for ( AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
		{
			if ( Controller->IsPlayerOwned() && Controller->PlayerReplicationInfo && !Controller->PlayerReplicationInfo->bOnlySpectator )
			{
				Targets.AddItem( Controller );
			}
		}
	}

	// Super class will send the action to all Targets
	Super::Activated();

	// If no targets were provided and we sent the action to all player controllers, empty the Targets array now.
	if ( bAddedPlayers )
	{
		Targets.Empty();
	}
}

/**
* Polls to see if the async action is done
*
* @param ignored
*
* @return TRUE if the operation has completed, FALSE otherwise
*/
UBOOL USeqAct_ManageTutorials::UpdateOp(FLOAT)
{
	// Early out if this tutorial is dead
	if ( bTutorialIsDead )
	{
		return true;
	}

	// Allow the action to pass all inputs pulses in except for the system on/off
	if ( InputLinks(eMTINPUT_AddTutorial).bHasImpulse ||
		 InputLinks(eMTINPUT_RemoveTutorial).bHasImpulse ||
		 InputLinks(eMTINPUT_StartTutorial).bHasImpulse ||
		 InputLinks(eMTINPUT_StopTutorial).bHasImpulse ||
		 InputLinks(eMTINPUT_CompleteTutorial).bHasImpulse )
	{
		Activated();
	}

	return bActionIsDone;
}

/** Callback for when the event is deactivated. */
void USeqAct_ManageTutorials::DeActivated()
{
	// Early out if this tutorial is dead
	if ( bTutorialIsDead )
	{
		return;
	}

	// Don't fire an output if we are turning the system on/off
	if ( !InputLinks(eMTINPUT_SystemOn).bHasImpulse && !InputLinks(eMTINPUT_SystemOff).bHasImpulse )
	{
		OutputLinks(eMTOUTPUT_UISceneOpened).bHasImpulse = false;

		if ( bTutorialSystemIsDead )
		{
			// Activate the "System Dead" output
			OutputLinks(eMTOUTPUT_SystemDead).ActivateOutputLink();
			bTutorialIsDead = true;
		}
		else if ( bTutorialAlreadyCompleted )
		{
			// Activate the "Already Completed" output
			OutputLinks(eMTOUTPUT_AlreadyComplete).ActivateOutputLink();
			bTutorialIsDead = true;
		}
		else if ( bTutorialCompleted )
		{
			// Activate the "Completed" output
			OutputLinks(eMTOUTPUT_Completed).ActivateOutputLink();
			bTutorialIsDead = true;
		}
	}
}

/*****************************************************************************
  USeqAct_DisplayPathChoice - Action that will freeze the game and display a choice scene
 *****************************************************************************/
IMPLEMENT_CLASS(USeqAct_DisplayPathChoice)

void USeqAct_DisplayPathChoice::Activated()
{
	UBOOL bAddedPlayers = FALSE;

	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to first local player found
		for (FPlayerIterator It(GEngine); It; ++It)
		{
			if (It->Actor != NULL)
			{
				Targets.AddItem(It->Actor);
				break;
			}
		}
	}

	Super::Activated();

	if ( bAddedPlayers )
	{
		Targets.Empty();
	}
}

/**
* Polls to see if the async action is done
*
* @param ignored
*
* @return TRUE if the operation has completed, FALSE otherwise
*/
UBOOL USeqAct_DisplayPathChoice::UpdateOp( FLOAT )
{
	//  Whether the action is done or not
	if( bIsDone )
	{
		// Chose left
		if( bIsLeftChoiceResult )
		{
			OutputLinks(0).ActivateOutputLink();
		}
		// Chose right
		else
		{
			OutputLinks(1).ActivateOutputLink();
		}

		// Close the scene
		if ( PathChoiceSceneInstance )
		{
			eventUninitializePathChoice( PathChoiceSceneInstance->GetPlayerOwner() );
			PathChoiceSceneInstance->eventCloseScene();
		}
	}

	return bIsDone;
}

/*****************************************************************************
  UUSeqAct_ManagePOI - Action that will enable/disable POIs
 *****************************************************************************/
IMPLEMENT_CLASS(USeqAct_ManagePOI)

void USeqAct_ManagePOI::Activated()
{
	for( INT Idx = 0; Idx < Targets.Num(); Idx++ )
	{
		AGearPointOfInterest* pPOI = Cast<AGearPointOfInterest>(Targets(Idx));

		if( pPOI )
		{
			POI = pPOI;
			break;
		}
	}

	bIsDone = FALSE;
	POI_bEnabled = FALSE;

	ProcessInputImpulses();

	// Activate "Out" output
	OutputLinks(eMPOIOUTPUT_Out).ActivateOutputLink();
}

/**
* Polls to see if the async action is done
*
* @param ignored
*
* @return TRUE if the operation has completed, FALSE otherwise
*/
UBOOL USeqAct_ManagePOI::UpdateOp( FLOAT )
{
	if ( bIsDone )
	{
		OutputLinks(eMPOIOUTPUT_Expired).ActivateOutputLink();
	}
	else
	{
		ProcessInputImpulses();
	}

	return bIsDone;
}

/** Checks for input impulses and calls the proper function accordingly */
void USeqAct_ManagePOI::ProcessInputImpulses()
{
	// Return and force the action to be done if there is no POI reference
	if ( !POI )
	{
		OutputLinks(eMPOIOUTPUT_Expired).ActivateOutputLink();
		bIsDone = TRUE;
		return;
	}
	else
	{
		POI->POIAction = this;
	}

	UBOOL bSetValues = FALSE;

	// The POI is being enabled
	if ( InputLinks(eMPOIINPUT_On).bHasImpulse && !POI_bEnabled )
	{
		POI_bEnabled = TRUE;
		bSetValues = TRUE;
	}
	// The POI is being disabled
	else if ( InputLinks(eMPOIINPUT_Off).bHasImpulse && POI_bEnabled )
	{
		POI_bEnabled = FALSE;
		bSetValues = TRUE;
	}

	// Set the values if we had an impulse that changed the enable value
	if ( bSetValues )
	{
		POI->DisplayName = POI_DisplayName;
		POI->IconDuration = POI_IconDuration;
		POI->ForceLookType = POI_ForceLookType;
		POI->ForceLookDuration = POI_ForceLookDuration;
		POI->bForceLookCheckLineOfSight = POI_bForceLookCheckLineOfSight;
		POI->LookAtPriority = POI_LookAtPriority;
		POI->LookAtPriority = POI_LookAtPriority;
		POI->DesiredFOV = POI_DesiredFOV;
		POI->FOVCount = POI_FOVCount;
		POI->bDoTraceForFOV = POI_bFOVLineOfSightCheck;
		POI->bDisableOtherPOIs = POI_bDisableOtherPOIs;
		POI->EnableDuration = POI_EnableDuration;
		POI->CurrIconDuration = POI_IconDuration;
		POI->bLeavePlayerFacingPOI = POI_bLeavePlayerFacingPOI;

		POI->eventSetEnabled( POI_bEnabled );

		// Activate "Out" output
		OutputLinks(eMPOIOUTPUT_Out).ActivateOutputLink();
	}
}


/*****************************************************************************
 USeqAct_GearPlayerAnim
*****************************************************************************/

void USeqAct_GearPlayerAnim::Activated()
{
	Super::Activated();

	// Activate "Out" output.
	OutputLinks(0).ActivateOutputLink();
}

UBOOL USeqAct_GearPlayerAnim::UpdateOp(FLOAT DeltaTime)
{
	if( InputLinks(1).bHasImpulse )
	{
		Activated();
		InputLinks(1).bHasImpulse = FALSE;
		return TRUE;
	}

	for(INT Idx = 0; Idx < Targets.Num(); Idx++)
	{
		UObject *Obj = Targets(Idx);
		if( Obj && Obj->IsA(AGearPawn::StaticClass()) )
		{
			AGearPawn* GPawn = Cast<AGearPawn>(Obj);
			if( GPawn )
			{
				if( GPawn->BS_IsPlaying(GPawn->KismetBodyStance) )
				{
					// Look no further, we're playing the animation.
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

void USeqAct_GearPlayerAnim::DeActivated()
{
	if(OutputLinks.Num() < 2)
	{
		warnf(TEXT("OutputLinks for %s was %i in length, should be 2!"),*GetName(),OutputLinks.Num());
	}
	else
	{
		// activate the finished output link
		OutputLinks(1).ActivateOutputLink();
	}
}

/*****************************************************************************
 USeqAct_AIGrapple
*****************************************************************************/

void USeqAct_AIGrapple::Activated()
{
	Super::Activated();

	// Activate "Out" output.
	OutputLinks(0).ActivateOutputLink();
}

UBOOL USeqAct_AIGrapple::UpdateOp(FLOAT DeltaTime)
{
	for(INT Idx = 0; Idx < Targets.Num(); Idx++)
	{
		UObject *Obj = Targets(Idx);
		if( Obj && Obj->IsA(AGearPawn::StaticClass()) )
		{
			AGearPawn* GPawn = Cast<AGearPawn>(Obj);
			if( GPawn && GPawn->IsDoingSpecialMove(SM_GrapplingHook_Climb) )
			{
				// Look no further, we're climbing...
				return FALSE;
			}
		}
	}

	return TRUE;
}

void USeqAct_AIGrapple::DeActivated()
{
	// activate the finished output link
	OutputLinks(1).ActivateOutputLink();
}

void USeqAct_AIGrapple::PostLoad()
{
	if( GrappleRopeClass == NULL )
	{
		SetTheHardReferences();
		Modify( TRUE ); // dirty the package
	}

	Super::PostLoad();

}

void USeqAct_AIGrapple::PostEditChange( UProperty* PropertyThatChanged )
{
	SetTheHardReferences();
	Super::PostEditChange(PropertyThatChanged);
}


void USeqAct_AIGrapple::SetTheHardReferences()
{
	GrappleRopeClass = FindObject<UClass>( ANY_PACKAGE, *GrappleRopeClassName ); 
	//warnf( TEXT("USeqAct_AIGrapple::SetTheHardReferences(): %s"), *GrappleRopeClass->GetName() );
}


/************************************************************************
 USeqAct_DrawMessage                                                                     
************************************************************************/

void USeqAct_DrawMessage::Activated()
{
	UBOOL bAddedPlayers = FALSE;

	// If there are no targets then we will send this action to all player controllers
	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to all players
		for ( AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
		{
			if ( Controller->IsPlayerOwned() )
			{
				Targets.AddItem( Controller );
			}
		}
	}

	// Super class will send the action to all Targets
	Super::Activated();

	// If no targets were provided and we sent the action to all player controllers, empty the Targets array now.
	if ( bAddedPlayers )
	{
		Targets.Empty();
	}

	OutputLinks(0).ActivateOutputLink();
}

UBOOL USeqAct_DrawMessage::UpdateOp(FLOAT DeltaTime)
{
	// if we're not using a display time, just jump out right away
	if( DisplayTimeSeconds < 0.f || MessageText.Len() < 1)
	{
		return TRUE;
	}

	UBOOL bAddedPlayers = FALSE;

	// If there are no targets then we will send this action to all player controllers
	if ( Targets.Num() == 0 )
	{
		bAddedPlayers = TRUE;
		// send to all players
		for ( AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
		{
			if ( Controller->IsPlayerOwned() )
			{
				Targets.AddItem( Controller );
			}
		}
	}

	UBOOL bDone = TRUE;
	for(INT Idx = 0; Idx < Targets.Num(); Idx++)
	{
		AGearPC* pGearPC = Cast<AGearPC>(Targets(Idx));

		if(pGearPC != NULL)
		{
			if(pGearPC->MyGearHud != NULL && pGearPC->MyGearHud->KismetMessageEndTime > 0)
			{
				bDone = FALSE;
				break;
			}
		}
	
	}

	// If no targets were provided and we sent the action to all player controllers, empty the Targets array now.
	if ( bAddedPlayers )
	{
		Targets.Empty();
	}

	// if all of our targets are not displaying messages, then activate the timer expired output
	if(bDone == TRUE)
	{
		OutputLinks(1).ActivateOutputLink();
	}	
	return bDone;
}

/*****************************************************************************
  This kismet action sets the health and health regeneration of all players
 *****************************************************************************/
IMPLEMENT_CLASS(USeqAct_MPHealthManipulator)

/** Activated will perform all of the necessary tasks for this action. */
void USeqAct_MPHealthManipulator::Activated()
{
	AWorldInfo* const WI = GWorld->GetWorldInfo();
	if ( WI )
	{
		AGearGameMP_Base* const MPGame = Cast<AGearGameMP_Base>(WI->Game);
		if ( MPGame )
		{
			UBOOL bSetHealth = FALSE;
			UBOOL bSetRegen = FALSE;

			// Set all players' health
			if ( InputLinks(0).bHasImpulse )
			{
				bSetHealth = TRUE;
			}
			// Set all players' regeneration
			else if ( InputLinks(1).bHasImpulse )
			{
				bSetRegen = TRUE;
			}

			for (AController *Player = GWorld->GetFirstController(); Player != NULL; Player = Player->NextController)
			{
				AGearPC *WPC = Cast<AGearPC>(Player);
				if ( WPC && WPC->MyGearPawn && WPC->MyGearPawn->IsPlayerOwned() )
				{
					if ( bSetHealth )
					{
						WPC->MyGearPawn->Health = Health;
						WPC->MyGearPawn->DefaultHealth = Health;
					}

					if ( bSetRegen )
					{
						WPC->MyGearPawn->HealthRechargePercentPerSecond = HealthRegenPerSecond;
					}
				}
			}
		}
	}
}


void USeqAct_AIStealthTracker::OnCreated()
{
	Super::OnCreated();
	if( OutputLinks.Num() != ((Layers.Num() * 3) + 1) )
	{
		UpdateDynamicLinks();
	}
}

void USeqAct_AIStealthTracker::PostLoad()
{
	Super::PostLoad();
	if( OutputLinks.Num() != ((Layers.Num() * 3) + 1) )
	{
		UpdateDynamicLinks();
	}
}

void USeqAct_AIStealthTracker::Activated()
{
	if( InputLinks(0).bHasImpulse )
	{
		// Reset layers on activate
		for( INT LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++ )
		{
			Layers(LayerIdx).TrackerInfo.Empty();
		}

		OutputLinks(0).ActivateOutputLink();
		bAbortTracking = FALSE;
	}
}

void USeqAct_AIStealthTracker::OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex )
{
	Super::OnReceivedImpulse( ActivatorOp, InputLinkIndex );

	if( InputLinkIndex == 1 )
	{
		bAbortTracking = TRUE;
	}
}

IMPLEMENT_COMPARE_CONSTREF( FStealthLayerInfo, GearGame, { return (B.Radius - A.Radius) < 0 ? 1 : -1; } )

void USeqAct_AIStealthTracker::UpdateDynamicLinks()
{
	// Sort layers by smallest radius first
	Sort<USE_COMPARE_CONSTREF(FStealthLayerInfo, GearGame)>(&Layers(0),Layers.Num());

	// If different number of layers and descriptions
	if( Layers.Num() != LayerDescs.Num() )
	{
		OutputLinks.Remove( 1, OutputLinks.Num() - 1 );
		for( INT LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++ )
		{
			INT OutIdx = OutputLinks.AddZeroed( 3 );
			for( INT Idx = 0; Idx < 3; Idx++ )
			{
				FString AppendStr = ((Idx==0) ? TEXT("Alarm") : ((Idx == 1) ? TEXT("Track") : TEXT("Reset")));

				FSeqOpOutputLink &OutLink = OutputLinks(OutIdx+Idx);
				OutLink.LinkDesc = FString::Printf(TEXT("%s - %s"), *Layers(LayerIdx).LayerDesc, *AppendStr );
			}
		}
	}
	else
	{
		// Name of a layer probably changed
		for( INT LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++ )
		{
			if( Layers(LayerIdx).LayerDesc != LayerDescs(LayerIdx) )
			{
				INT OutIdx = 1 + (LayerIdx * 3); //+1 to skip past output link
				for( INT Idx = 0; Idx < 3; Idx++ )
				{
					FString AppendStr = ((Idx==0) ? TEXT("Alarm") : ((Idx == 1) ? TEXT("Track") : TEXT("Reset")));
					FSeqOpOutputLink &OutLink = OutputLinks(OutIdx+Idx);
					OutLink.LinkDesc = FString::Printf(TEXT("%s - %s"), *Layers(LayerIdx).LayerDesc, *AppendStr );
				}
			}
		}
	}

	LayerDescs.Empty(Layers.Num());
	for( INT LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++ )
	{
		LayerDescs.AddItem( Layers(LayerIdx).LayerDesc );
	}
	
	Super::UpdateDynamicLinks();
}

void USeqAct_AIStealthTracker::WriteOutputVars( AActor* Spotter, AActor* Spotted )
{
	// write out to any attached variable links
	TArray<UObject**> SpottedVars;
	GetObjectVars(SpottedVars,TEXT("Spotted"));
	for (INT VarIdx = 0; VarIdx < SpottedVars.Num(); VarIdx++)
	{
		*(SpottedVars(VarIdx)) = Spotted;
	}
	TArray<UObject**> SpotterVars;
	GetObjectVars(SpotterVars,TEXT("Spotter"));
	for (INT VarIdx = 0; VarIdx < SpotterVars.Num(); VarIdx++)
	{
		*(SpotterVars(VarIdx)) = Spotter;
	}
}

UBOOL USeqAct_AIStealthTracker::UpdateOp( FLOAT DeltaTime )
{
	// Tracking has been aborted
	if( bAbortTracking )
		return TRUE;

	// For each tracker
	for( INT TrackerIdx = 0; TrackerIdx < Trackers.Num(); TrackerIdx++ )
	{		
		AActor* Tracker = Cast<AActor>(Trackers(TrackerIdx));
		if( Tracker == NULL )
			continue;
		Tracker = Tracker->GetAController() ? Tracker->GetAController()->Pawn : Tracker;
		if( Tracker == NULL )
			continue;

		// For each target to be tracked
		for( INT TargetIdx = 0; TargetIdx < Targets.Num(); TargetIdx++ )
		{
			UBOOL bTargetTracked = FALSE;

			AActor* Target = Cast<AActor>(Targets(TargetIdx));
			if( Target == NULL )
				continue;
			Target = Target->GetAController() ? Target->GetAController()->Pawn : Target;
			if( Target == NULL )
				continue;

			APawn* TargetPawn = Target->GetAPawn();

			// For each layer
			for( INT LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++ )
			{
				FStealthLayerInfo& LayerInfo = Layers(LayerIdx);

				// Setup delegates for updating
				eventSetUpdateDelegate( LayerIdx );

				// If stealth layer should be ticked up
				if( delegateOnCheckStealthLayer( LayerIdx, Tracker, Target ) )
				{
					bTargetTracked = TRUE;
					
					// Update the layer... if true returned, layer timer has been triggered
					BYTE Result = delegateOnUpdateStealthLayer( LayerIdx, DeltaTime, Tracker, Target );
					if(  Result == LUR_StartTracking && (TargetPawn == NULL || TargetPawn->IsHumanControlled() || LayerInfo.bAIWillTriggerTrack ))
					{
						// Activate output link
						WriteOutputVars( Tracker, Target );
						OutputLinks(1+(LayerIdx*3)+1).ActivateOutputLink();
					}
					else
					if( Result == LUR_Alarm && (TargetPawn == NULL || TargetPawn->IsHumanControlled() || LayerInfo.bAIWillTriggerAlarm ))
					{
						// Activate output link
						WriteOutputVars( Tracker, Target );
						OutputLinks(1+(LayerIdx*3)).ActivateOutputLink();

						// If should deactivate after stealth broken
						if( bDeactivateOnTrigger )
						{
							// Return true to deactivate action
							return TRUE;
						}
					}
				}
				else
				{
					BYTE Result = delegateOnUpdateStealthLayer( LayerIdx, -DeltaTime, Tracker, Target );
					if( Result == LUR_StopTracking )
					{
						// Layer Reset!
						WriteOutputVars( Tracker, Target );
						OutputLinks(1+(LayerIdx*3)+2).ActivateOutputLink();
					}
				}

				// Only trigger one layer at a time for each target
				// (assumes smallest to largest radius ordering)
				if( bTargetTracked )
				{
					break;
				}
			}			
		}
	}

#if !FINAL_RELEASE
	if( bDrawDebugLayers && bActive )
	{
		for( INT LayerIdx = 0; LayerIdx < Layers.Num(); LayerIdx++ )
		{
			FStealthLayerInfo& Layer = Layers(LayerIdx);
			for( INT TrackerIdx = 0; TrackerIdx < Trackers.Num(); TrackerIdx++ )
			{
				AActor* Tracker = Cast<AActor>(Trackers(TrackerIdx));
				if( Tracker == NULL )
					continue;
				Tracker = Tracker->GetAController() ? Tracker->GetAController()->Pawn : Tracker;
				if( Tracker == NULL )
					continue;

				FStealthTrackerInfo* TrackerInfo = NULL;
				for( INT i = 0; i < Layer.TrackerInfo.Num(); i++ )
				{
					if( Layer.TrackerInfo(i).Tracker == Tracker )
					{
						TrackerInfo = &Layer.TrackerInfo(i);
						break;
					}
				}

				FColor LayerColor(255,0,0);
				if( TrackerInfo != NULL && TrackerInfo->TargetInfo.Num() > 0 )
				{
					LayerColor = FColor(0,255,0);
				}

				Tracker->DrawDebugCylinder( Tracker->Location, Tracker->Location, Layer.Radius, 20, LayerColor.R, LayerColor.G, LayerColor.B );

				if( Layer.bDoFOVCheck )
				{
					FVector ViewDir = Tracker->Rotation.Vector();
					AGearAI* AI = ((Tracker->GetAPawn() != NULL) ? Cast<AGearAI>(Tracker->GetAPawn()->Controller) : NULL);
					if( AI != NULL )
					{
						AI->GetLookDir(ViewDir);
					}

					FRotator Rot = FRotator(0,appTrunc(appAcos(Layer.FOV) * (32768.f / PI)),0);
					FVector Rt = FRotationMatrix(Rot).TransformNormal( ViewDir ); 
					FVector Lt = FRotationMatrix(Rot).Transpose().TransformNormal( ViewDir ); 
					
					FVector ViewLoc = (Tracker->GetAPawn() != NULL) ? Tracker->GetAPawn()->eventGetPawnViewLocation() : Tracker->Location;

					Tracker->DrawDebugLine( ViewLoc, ViewLoc + Rt * Layer.Radius, 0, 255, 0 );
					Tracker->DrawDebugLine( ViewLoc, ViewLoc + Lt * Layer.Radius, 0, 255, 0 );
				}
			}
		}
	}
#endif

	return FALSE;
}

UBOOL USeqAct_AIStealthTracker::CheckLayerDelegate_ByRange( INT LayerIdx, AActor* Tracker, AActor* Target )
{
	UBOOL bResult = FALSE;

	FStealthLayerInfo& LayerInfo = Layers(LayerIdx);
	// If target is too close to tracker
	FLOAT TargetToTrackerDist = (Target->Location - Tracker->Location).Size();
	//debugf(TEXT("[%f] %s Tracking %s [%s] -- dist %f / %f"), Target->WorldInfo->TimeSeconds, *Tracker->GetName(), *Target->GetName(), *Layer.LayerDesc, TargetToTrackerDist, Layer.Radius );
	if( TargetToTrackerDist < LayerInfo.Radius )
	{
		bResult = TRUE;

		APawn* TrackerPawn = Tracker->GetAPawn();
		APawn* TargetPawn  = Target->GetAPawn();
		if( LayerInfo.bDoVisibilityCheck )
		{
			FVector StartTrace = TrackerPawn ? TrackerPawn->eventGetPawnViewLocation() : Tracker->Location;
			FVector EndTrace   = TargetPawn  ? TargetPawn->eventGetPawnViewLocation()  : Target->Location;

			// Update layer if there is a clear line of sight
			FCheckResult Hit(1.f);
			bResult = bResult && GWorld->SingleLineCheck( Hit, Tracker, EndTrace, StartTrace, TRACE_World );
		}
		if( LayerInfo.bDoFOVCheck )
		{
			FVector ViewDir = Tracker->Rotation.Vector();
			AGearAI* AI = ((TrackerPawn != NULL) ? Cast<AGearAI>(TrackerPawn->Controller) : NULL);
			if( AI != NULL )
			{
				AI->GetLookDir(ViewDir);
			}

			FVector TrackerToTargetDir = (Target->Location-Tracker->Location).SafeNormal();
			FLOAT ViewDotTarget = ViewDir | TrackerToTargetDir;

			// Update layer if target is inside view cone
			bResult = bResult && (ViewDotTarget >= LayerInfo.FOV);
		}
	}
	return bResult;
}

UBOOL USeqAct_AIStealthTracker::CheckLayerDelegate_ByBehavior( INT LayerIdx, AActor* Tracker, AActor* Target )
{
	AGearPawn* GP = Cast<AGearPawn>(Target);
	if( GP != NULL )
	{
		if( GP->SpecialMove == SM_RoadieRun || 
			GP->SpecialMove == SM_EvadeFwd	||
			GP->SpecialMove == SM_EvadeBwd	||
			GP->SpecialMove == SM_EvadeRt	||
			GP->SpecialMove == SM_EvadeLt	||
			GP->SpecialMove == SM_EvadeFromCoverCrouching ||
			GP->SpecialMove == SM_EvadeFromCoverStanding  )
		{
			return CheckLayerDelegate_ByRange( LayerIdx, Tracker, Target );
		}
	}
	return FALSE;
}


/**
 *	Retrieves the Tracker and Target indices for the given layer
 *	If pair of indices doesn't exist, it creates new entries 
 */
UBOOL USeqAct_AIStealthTracker::GetTrackerTargetPairIndices( INT LayerIdx, AActor* Tracker, AActor* Target, INT& out_TrackerIdx, INT& out_TargetIdx, UBOOL bCreateEntry )
{
	FStealthLayerInfo& LayerInfo = Layers(LayerIdx);

	out_TrackerIdx = -1;
	out_TargetIdx  = -1;

	for( INT TrackerIdx = 0; TrackerIdx < LayerInfo.TrackerInfo.Num(); TrackerIdx++ )
	{
		FStealthTrackerInfo& TrackerInfo = LayerInfo.TrackerInfo(TrackerIdx);
		if( TrackerInfo.Tracker == Tracker )
		{
			out_TrackerIdx = TrackerIdx;
			for( INT TargetIdx = 0; TargetIdx < TrackerInfo.TargetInfo.Num(); TargetIdx++ )
			{
				FStealthTargetInfo& TargetInfo = TrackerInfo.TargetInfo(TargetIdx);
				if( TargetInfo.Target == Target )
				{
					out_TargetIdx = TargetIdx;
					break;
				}
			}
			break;
		}
	}

	if( bCreateEntry )
	{
		if( out_TargetIdx < 0 )
		{
			FStealthTrackerInfo* TrackerInfoPtr = NULL;
			if( out_TrackerIdx < 0 )
			{
				// Add new tracker info item to the list
				out_TrackerIdx =  LayerInfo.TrackerInfo.AddZeroed();
				TrackerInfoPtr = &LayerInfo.TrackerInfo(out_TrackerIdx);
				TrackerInfoPtr->Tracker = Tracker;
			}
			else
			{
				TrackerInfoPtr = &LayerInfo.TrackerInfo(out_TrackerIdx);
			}

			// Add new target info item to the list
			out_TrackerIdx = TrackerInfoPtr->TargetInfo.AddZeroed();
			TrackerInfoPtr->TargetInfo(out_TrackerIdx).Target = Target;
		}
	}

	return (out_TrackerIdx >= 0 && out_TargetIdx >= 0);
}

BYTE USeqAct_AIStealthTracker::UpdateLayerDelegate_ByTime( INT LayerIdx, FLOAT DeltaTime, AActor* Tracker, AActor* Target )
{
	BYTE Result = LUR_None;

	// Get indices for this pair - create new entry if ticking up to track
	INT TrackerIdx = -1, TargetIdx  = -1;
	if( GetTrackerTargetPairIndices( LayerIdx, Tracker, Target, TrackerIdx, TargetIdx, (DeltaTime > 0) ) )
	{
		FStealthLayerInfo&	 LayerInfo   = Layers(LayerIdx);
		FStealthTrackerInfo& TrackerInfo = LayerInfo.TrackerInfo(TrackerIdx);
		FStealthTargetInfo&  TargetInfo  = TrackerInfo.TargetInfo(TargetIdx);

		// If ticking up
		if( DeltaTime > 0 )
		{
			// If just starting to track
			if( TargetInfo.Timer <= 0 )
			{
				// Fire off kismet output
				Result = LUR_StartTracking;
			}

			// If not already over time limit
			if( TargetInfo.Timer < LayerInfo.TimeLimit )
			{
				// Update timer for the tracker/target pair
				TargetInfo.Timer += DeltaTime;
				// If time limit reached after update
				if( TargetInfo.Timer >= LayerInfo.TimeLimit )
				{
					// Fire off kismet output
					Result = LUR_Alarm;
				}
			}
		}
		else
		// Otherwise, if ticking down
		if( DeltaTime < 0 )
		{
			// Update the timer for the tracker/target pair
			TargetInfo.Timer += DeltaTime;
			// If timer has reached zero
			if( TargetInfo.Timer <= 0 )
			{
				// Remove the target entry for the pair
				TrackerInfo.TargetInfo.Remove( TargetIdx, 1 );
				if( TrackerInfo.TargetInfo.Num() == 0 )
				{
					// Fire off kismet output
					Result = LUR_StopTracking;
				}
			}
		}
	}

	return Result;
}


/************************************************************************
	 USeqAct_ToggleConversation
************************************************************************/

void USeqAct_ToggleConversation::Activated()
{
	// deprecated
	Super::Activated();
}
UBOOL USeqAct_ToggleConversation::UpdateOp(FLOAT DeltaTime)
{
	// deprecated
	return Super::UpdateOp(DeltaTime);
}
void USeqAct_ToggleConversation::DeActivated()
{
	// deprecated
	Super::DeActivated();
}


/************************************************************************
	 USeqAct_SpectatorCameraPath
************************************************************************/

void USeqAct_SpectatorCameraPath::Activated()
{
	// just to be safe, no shenanigans please
	PlayRate = 1.f;

	// get interpolation ready right off the bat, since we can switch to using
	// it at any time.  Do this before calling the super.
	InitInterp();

	// note we're skipping SeqAct_Interp::Activated.
	USequenceAction::Activated();
}

void USeqAct_SpectatorCameraPath::MoveToNextStop()
{
	CacheInterpData();
	if (bCachedInterpData && (!bIsPlaying || bPaused))
	{
		if (bSwapForwardAndReverse)
		{
			// go the other way
			bSwapForwardAndReverse = FALSE;
			MoveToPrevStop();
			bSwapForwardAndReverse = TRUE;
		}
		else
		{	
			Play();

			// calc InterpTimeToNextStop
			INT const NumEvents = CachedEventTrack->EventTrack.Num();
			if ( LastStopIdx == (NumEvents-1) )
			{
				// wraparound case
				DestinationStopIdx = 0;
				InterpTimeToNextStop = InterpData->InterpLength - CachedEventTrack->EventTrack(LastStopIdx).Time;
				InterpTimeToNextStop += CachedEventTrack->EventTrack(0).Time;
			}
			else if ( (LastStopIdx >= 0) && (LastStopIdx < NumEvents) )
			{
				DestinationStopIdx = LastStopIdx + 1;
				InterpTimeToNextStop = CachedEventTrack->EventTrack(DestinationStopIdx).Time - CachedEventTrack->EventTrack(LastStopIdx).Time;
			}
			else
			{
				// fallback case, shouldn't get here, 1.f is arbitrary
				DestinationStopIdx = 0;
				InterpTimeToNextStop = 1.f;
			}
		}
	}
}
void USeqAct_SpectatorCameraPath::MoveToPrevStop()
{
	CacheInterpData();
	if (bCachedInterpData && (!bIsPlaying || bPaused))
	{
		if (bSwapForwardAndReverse)
		{
			// go the other way
			bSwapForwardAndReverse = FALSE;
			MoveToNextStop();
			bSwapForwardAndReverse = TRUE;
		}
		else
		{
			Reverse();

			// calc InterpTimeToNextStop
			INT const NumEvents = CachedEventTrack->EventTrack.Num();
			if (LastStopIdx == 0)
			{
				// wraparound case
				DestinationStopIdx = NumEvents - 1;
				InterpTimeToNextStop = InterpData->InterpLength - CachedEventTrack->EventTrack(DestinationStopIdx).Time;
				InterpTimeToNextStop += CachedEventTrack->EventTrack(0).Time;
			}
			else if ( (LastStopIdx >= 0) && (LastStopIdx < NumEvents) )
			{
				DestinationStopIdx = LastStopIdx - 1;
				InterpTimeToNextStop = CachedEventTrack->EventTrack(LastStopIdx).Time - CachedEventTrack->EventTrack(DestinationStopIdx).Time;
			}
			else
			{
				// fallback case, shouldn't get here, 1.f is arbitrary
				DestinationStopIdx = 0;
				InterpTimeToNextStop = 1.f;
			}
		}
	}
}

void USeqAct_SpectatorCameraPath::UpdateCameraPosition(FLOAT DeltaTime)
{
	// 
	FLOAT const AdjustedDeltaTime = DeltaTime * InterpTimeToNextStop / TravelTimeBetweenStops;

	// StepInterp will modify Position
	FLOAT const PrevInterpTime = Position;

	// Update the current position and do any interpolation work.
	StepInterp(AdjustedDeltaTime, false);

	if (CachedCameraActor && CachedMoveTrack && CachedMoveTrackInst)
	{
		CachedCameraActor->MoveWithInterpMoveTrack(CachedMoveTrack, CachedMoveTrackInst, PrevInterpTime, AdjustedDeltaTime);
	}
}

/** Internal.  Mines the data for the bits we're interested in, caches it for later use. */
void USeqAct_SpectatorCameraPath::CacheInterpData()
{
	if (InterpData == NULL)
	{
		InitInterp();
		check(InterpData != NULL);
	}
	if (!bCachedInterpData && InterpData)
	{
		for (INT GroupIdx=0; GroupIdx<GroupInst.Num(); ++GroupIdx)
		{
			UInterpGroupInst* const CurGroupInst = GroupInst(GroupIdx);
			ACameraActor* const GroupCamActor = Cast<ACameraActor>( CurGroupInst->GetGroupActor() );

			if (GroupCamActor)
			{
				UInterpTrackInstMove* MoveTrackInst = NULL;
				UInterpTrackInstEvent* EventTrackInst = NULL;

				// search track instances to make sure there's a move track and an event track
				INT MoveTrackInstIdx = INDEX_NONE;
				INT EventTrackInstIdx = INDEX_NONE;

				for (INT TrackIdx=0; TrackIdx<CurGroupInst->TrackInst.Num(); ++TrackIdx)
				{
					UInterpTrackInst* const CurTrackInst = CurGroupInst->TrackInst(TrackIdx);
					if (CurTrackInst)
					{
						if ( (MoveTrackInst == NULL) && CurTrackInst->IsA(UInterpTrackInstMove::StaticClass()) )
						{
							MoveTrackInst = (UInterpTrackInstMove*)CurTrackInst;
							MoveTrackInstIdx = TrackIdx;
						}
						else if ( (EventTrackInst == NULL) && CurTrackInst->IsA(UInterpTrackInstEvent::StaticClass()) )
						{
							EventTrackInst = (UInterpTrackInstEvent*)CurTrackInst;
							EventTrackInstIdx = TrackIdx;
						}
					}
				}

				if (MoveTrackInst && EventTrackInst)
				{
					CachedCameraActor = GroupCamActor;
					
					CachedMoveTrackInst = MoveTrackInst;
					CachedMoveTrack = Cast< UInterpTrackMove >( CurGroupInst->Group->InterpTracks( MoveTrackInstIdx ) );
					check( CachedMoveTrack != NULL );

					CachedEventTrackInst = EventTrackInst;
					CachedEventTrack = Cast< UInterpTrackEvent >( CurGroupInst->Group->InterpTracks( EventTrackInstIdx ) );
					check( CachedEventTrack != NULL );

					bCachedInterpData = TRUE;
					break;
				}
			}
		}
	}
}

UBOOL USeqAct_SpectatorCameraPath::IsValidCameraPath()
{
	CacheInterpData();
	return (CachedCameraActor && CachedMoveTrack && CachedMoveTrackInst && CachedEventTrack && CachedEventTrackInst);
}

class ACameraActor* USeqAct_SpectatorCameraPath::GetAssociatedCameraActor()
{
	CacheInterpData();
	return CachedCameraActor;
}

void USeqAct_SpectatorCameraPath::NotifyEventTriggered(class UInterpTrackEvent const* EventTrack, INT EventIdx)
{
	// Note: don't call the super, since we don't care about having an event output on the kismet action.

	if (EventTrack == CachedEventTrack)
	{
		// we ignore the last stopped index, since reversing dir can retrigger the same event (we don't stop
		// exactly on the event).
		if (EventIdx != LastStopIdx)
		{
			// stop here
			Pause();
			
			// snap to the exact event position
			UpdateInterp(EventTrack->EventTrack(EventIdx).Time, FALSE, TRUE);

			LastStopIdx = EventIdx;
		}
	}
}


void USeqAct_SpectatorCameraPath::UpdateInterp(FLOAT NewPosition, UBOOL bPreview, UBOOL bJump)
{
	UBOOL const bWasPlaying = !bPaused && bIsPlaying;

	Super::UpdateInterp(NewPosition, bPreview, bJump);

	UBOOL const bNowPlaying = !bPaused && bIsPlaying;

	if (bWasPlaying && !bNowPlaying && CachedEventTrack)
	{
		// we got stopped this frame.  snap to the exact event position.
		Super::UpdateInterp(CachedEventTrack->EventTrack(LastStopIdx).Time, FALSE, TRUE);
	}
}

void USeqAct_SpectatorCameraPath::ResetToStartingPosition()
{
	// 'Jump' interpolation to the start (ie. will not fire events between current position and start).
	UpdateInterp(0.f, FALSE, TRUE);
	LastStopIdx = 0;
	DestinationStopIdx = 0;
}

void USeqAct_SpectatorCameraPath::UpdateConnectorsFromData()
{
	Super::UpdateConnectorsFromData();

	// hide any event outputs
	for (INT i=1; i<OutputLinks.Num(); ++i)
	{
		OutputLinks(i).bHidden = TRUE;
	}
}

FLOAT USeqAct_SpectatorCameraPath::GetInterpolationPercentage() const
{
	FLOAT CurrentInterpPct = 1.f;

	if (DestinationStopIdx != LastStopIdx)
	{
		check(bIsPlaying && !bPaused);

		FLOAT const DestTime = CachedEventTrack->EventTrack(DestinationStopIdx).Time;

		// find the remaining time in the interpolation
		INT const NumEvents = CachedEventTrack->EventTrack.Num();
		FLOAT TimeRemaining = 0.f;
		if ( (LastStopIdx == (NumEvents-1)) && (DestinationStopIdx == 0) )
		{
			// high->low wraparound
			if (Position > DestTime)
			{
				// we haven't wrapped yet
				TimeRemaining = DestTime + (InterpData->InterpLength - Position);
			}
			else
			{
				// we've already wrapped
				TimeRemaining = DestTime - Position;
			}
		}
		else if ( (LastStopIdx == 0) && (DestinationStopIdx == (NumEvents-1)) )
		{
			// low->high wraparound
			if (Position > DestTime)
			{
				// already wrapped
				TimeRemaining = Position - DestTime;
			}
			else
			{
				// haven't wrapped yet
				TimeRemaining = (InterpData->InterpLength - DestTime) + Position;
			}
		}
		else
		{
			TimeRemaining = Abs(Position - DestTime);
		}

		CurrentInterpPct = 1.f - TimeRemaining / InterpTimeToNextStop;
	}

	return CurrentInterpPct;
}


/************************************************************************
	 USeqAct_SetWeather
************************************************************************/

void USeqAct_SetWeather::Activated()
{
	Super::Activated();

	// assume the 0 input ("clear") by default
	EWeatherType DesiredWeather = WeatherType_Clear;

	UBOOL bOverrideEmitterHeight = FALSE;
	FLOAT EmitterHeight = 0.f;

	if (InputLinks(1).bHasImpulse)
	{
		// the "rain" input
		DesiredWeather = WeatherType_Rain;
		bOverrideEmitterHeight = bOverrideRainEmitterHeight;
		EmitterHeight = RainEmitterHeight;
	}
	else if (InputLinks(2).bHasImpulse)
	{
		// the "hail" input
		DesiredWeather = WeatherType_Hail;
		bOverrideEmitterHeight = bOverrideHailEmitterHeight;
		EmitterHeight = HailEmitterHeight;
	}

	AGearGRI* const GRI = Cast<AGearGRI>(GWorld->GetWorldInfo()->GRI);
	if (GRI)
	{
		GRI->eventSetCurrentWeather(DesiredWeather);
		if (bOverrideEmitterHeight)
		{
			GRI->eventSetWeatherEmitterHeight(EmitterHeight);
		}

	}
}

/************************************************************************
	USeqAct_PopupCoverControl
************************************************************************/
void USeqAct_PopupCoverControl::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	if( PropertyThatChanged != NULL )
	{
		if( PropertyThatChanged->GetFName() == FName(TEXT("MatineeObjList")) )
		{
			ResolveMatineeNames();
		}
	}
}


void USeqAct_PopupCoverControl::ResolveMatineeNames()
{
	MatineeList.Empty();

	USequence* GameSeq = GWorld->GetWorldInfo()->GetGameSequence();
	if( GameSeq != NULL )
	{
		for( INT Idx = 0; Idx < MatineeObjList.Num(); Idx++ )
		{
			TArray<USequenceObject*> ObjList;
			GameSeq->FindSeqObjectsByName( *MatineeObjList(Idx), TRUE, ObjList, TRUE );
			for( INT ObjIdx = 0; ObjIdx < ObjList.Num(); ObjIdx++ )
			{
				USeqAct_Interp* Interp = Cast<USeqAct_Interp>(ObjList(ObjIdx));
				if( Interp != NULL )
				{
					MatineeList.AddUniqueItem( Interp );						
					Interp->bInterpForPathBuilding	  = TRUE;
					Interp->InterpData->PathBuildTime = 1.f;
				}			
			}
		}
	}
}

void USeqAct_PopupCoverControl::Initialize()
{
	Super::Initialize();

	if( (GIsGame || GIsPlayInEditorWorld) && Helper != NULL )
	{
		Helper->SetBlockingVolumeCollision( FALSE, BlockingVolumes );
	}

	bCurrentPoppedUp = !bStartPoppedUp;
	InputLinks(0).bHasImpulse =  bStartPoppedUp;
	InputLinks(1).bHasImpulse = !bStartPoppedUp;
	ParentSequence->QueueSequenceOp( this, TRUE );
}

void USeqAct_PopupCoverControl::PrePathBuild( AScout* Scout )
{
	if( Helper == NULL || Helper->SeqObj != this )
	{
		ULevel* OuterLevel = Cast<ULevel>(GetRootSequence()->GetOuter());
		Helper = ConstructObject<APopupCoverControlHelper>(  APopupCoverControlHelper::StaticClass(), OuterLevel, NAME_None, RF_Transactional, APopupCoverControlHelper::StaticClass()->GetDefaultActor() );
		OuterLevel->Actors.AddItem( Helper );
		Helper->SeqObj = this;
		Helper->WorldInfo = GWorld->GetWorldInfo();
	}

	for( INT Idx = 0; Idx < BlockingVolumes.Num(); Idx++ )
	{
		ABlockingVolume* BV = BlockingVolumes(Idx);
		if( BV != NULL )
		{
			BV->bPathColliding = TRUE;
		}
	}
}

void USeqAct_PopupCoverControl::OnDelete()
{
	Super::OnDelete();

	if( Helper != NULL )
	{
		GWorld->DestroyActor( Helper );
	}
}

void DumpAllPaths( USeqAct_PopupCoverControl* C )
{
	for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	{
		debugf(TEXT("Dump %s ... %d"), *Nav->GetName(), Nav->PathList.Num() );
		for( INT PathIdx = 0; PathIdx < Nav->PathList.Num(); PathIdx++ )
		{
			UReachSpec* Spec = Nav->PathList(PathIdx);
			FCrossLevelReachSpec* Up   = C->Helper->GetRef( Spec->Start, Spec->End.Nav(), C->Helper->BlockedWhenUp,	  Spec->CollisionHeight, Spec->CollisionRadius );
			FCrossLevelReachSpec* Down = C->Helper->GetRef( Spec->Start, Spec->End.Nav(), C->Helper->BlockedWhenDown, Spec->CollisionHeight, Spec->CollisionRadius );
			debugf(TEXT(".... %s to %s %d %d Up %s %s Down %s %s"), 
				*Spec->GetName(), 
				*Spec->End->GetName(), 
				Spec->CollisionHeight,
				Spec->CollisionRadius, 
				Up?TEXT("TRUE"):TEXT("FALSE"),
				Up?*Up->Spec->GetName():TEXT("NULL"),
				Down?TEXT("TRUE"):TEXT("FALSE"),
				Down?*Down->Spec->GetName():TEXT("NULL")
			);
		}
	}

	debugf(TEXT("BLOCKWHENUP....."));
	for( INT Idx = 0; Idx < C->Helper->BlockedWhenUp.Num(); Idx++ )
	{
		UReachSpec* Spec = C->Helper->GetSpec( Idx, C->Helper->BlockedWhenUp );
		if( Spec )
		{
			debugf(TEXT("%d ... %s to %s (%s) H/R %d/%d Disabled? %s"), 
				Idx,
				*Spec->Start->GetName(),
				*Spec->End.Nav()->GetName(),
				*Spec->GetName(),
				 Spec->CollisionHeight,
				 Spec->CollisionRadius,
				 Spec->bDisabled?TEXT("TRUE"):TEXT("FALSE") );
		}
	}
	debugf(TEXT("BLOCKWHENDOWN....."));
	for( INT Idx = 0; Idx < C->Helper->BlockedWhenDown.Num(); Idx++ )
	{
		UReachSpec* Spec = C->Helper->GetSpec( Idx, C->Helper->BlockedWhenDown );
		if( Spec )
		{
			debugf(TEXT("%d ... %s to %s (%s) H/R %d/%d Disabled? %s"), 
				Idx,
				*Spec->Start->GetName(),
				*Spec->End.Nav()->GetName(),
				*Spec->GetName(),
				Spec->CollisionHeight,
				Spec->CollisionRadius,
				Spec->bDisabled?TEXT("TRUE"):TEXT("FALSE") );
		}
	}
}

void USeqAct_PopupCoverControl::PostPathBuild( AScout* Scout )
{
	if( Helper == NULL )
		return;

	// Try to resolve matinee names again if list is empty
	// Catches the case of adding a matinee name to the controller before naming the matinee object
	if( MatineeList.Num() == 0 )
	{
		ResolveMatineeNames();
	}	

	PublishLinkedVariableValues();

	Helper->BlockedWhenUp.Empty();
	Helper->BlockedWhenDown.Empty();
	MarkerList.Empty();

	//debug
	//DumpAllPaths( this );

	// For each cover link given by LDs
	GWarn->StatusUpdatef( 0, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

	for( INT LinkIdx = 0; LinkIdx < CoverLinks.Num(); LinkIdx++ )
	{
		ACoverLink* Link = CoverLinks(LinkIdx);
		if( Link == NULL )
		{
			CoverLinks.Remove( LinkIdx--, 1 );
			continue;
		}
		// For each slot in the link
		for( INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++ )
		{
			ACoverSlotMarker* Marker = Link->GetSlotMarker( SlotIdx );
			if( Marker == NULL )
				continue;

			// Trace forward from the link to see if there is a blocking volume in front of it
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult* FirstHit = NULL;
			FirstHit = GWorld->MultiLineCheck
			(
				GMainThreadMemStack,
				Marker->GetSlotLocation() + Marker->GetSlotRotation().Vector() * 64.f,
				Marker->GetSlotLocation(),
				FVector(5,5,5),
				TRACE_World,
				Marker
			);

			for( FCheckResult* TestHit = FirstHit; TestHit != NULL; TestHit = TestHit->GetNext() )
			{
				ABlockingVolume* BV = Cast<ABlockingVolume>(TestHit->Actor);
				// If hit a blocking volume which is in our valid list
				if( BV != NULL && BlockingVolumes.FindItemIndex( BV ) >= 0 )
				{
					// Add marker to our affected list
					MarkerList.AddUniqueItem( Marker );
				}
			}
			Mark.Pop();
		}
	}

	GWarn->StatusUpdatef( 2, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

	// Turn off collision on all given blocking volumes
	Helper->SetBlockingVolumeCollision( FALSE, BlockingVolumes );
	Helper->SetMatineePosition( TRUE, MatineeList );

	TArray<ANavigationPoint*> AddNavs;

	// For each marker...
	for( INT MarkerIdx = 0; MarkerIdx < MarkerList.Num(); MarkerIdx++ )
	{
		ACoverSlotMarker* Marker = MarkerList(MarkerIdx);

		// Set all Mantle and Coverslip reach specs to be blocked when cover is down
		for( INT PathIdx = 0; PathIdx < Marker->PathList.Num(); PathIdx++ )
		{
			UReachSpec* Spec = Marker->PathList(PathIdx);
			if(  Spec != NULL &&
				 Spec->Start != NULL &&
				*Spec->End != NULL )
			{
				UClass* SpecClass = Spec->GetClass();
				if( SpecClass == UMantleReachSpec::StaticClass() ||
					SpecClass == UCoverSlipReachSpec::StaticClass() )
				{
					Helper->AddSpecToList( Spec, Helper->BlockedWhenDown );
					Spec->Start->PathList.Remove( PathIdx--, 1 );
				}
			}
		}
	}
	GWarn->StatusUpdatef( 4, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));


	for( INT MarkerIdx = 0; MarkerIdx < MarkerList.Num(); MarkerIdx++ )
	{
		ACoverSlotMarker* Marker = MarkerList(MarkerIdx);
		Marker->bIgnoreSizeLimits = TRUE;

		// try to build a spec to every other pathnode in the world
		TArray<FNavigationOctreeObject*> NavObjects;  
		GWorld->NavigationOctree->RadiusCheck(Marker->Location, MAXPATHDIST, NavObjects);

		//for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
		for(INT Idx=0;Idx<NavObjects.Num();Idx++)
		{

			ANavigationPoint* Nav = NavObjects(Idx)->GetOwner<ANavigationPoint>();
			if( Nav != Marker && Nav != NULL )
			{
				UReachSpec* OldSpec = Marker->GetReachSpecTo( Nav );
				if( OldSpec != NULL )
				{
					if( OldSpec->GetClass() == USwatTurnReachSpec::StaticClass()	||
						OldSpec->GetClass() == USlotToSlotReachSpec::StaticClass()  )
					{
						continue;
					}
					if( OldSpec->GetClass() == UMantleReachSpec::StaticClass() ||
						OldSpec->GetClass() == UCoverSlipReachSpec::StaticClass() )
					{
						OldSpec = NULL;
					}
				}

				if( Marker->CanConnectTo( Nav, TRUE ) )
				{
					UClass*		ReachSpecClass  = Marker->GetReachSpecClass( Nav, Scout->GetDefaultReachSpecClass() );
					UReachSpec *newSpec			= ConstructObject<UReachSpec>(ReachSpecClass,Marker->GetOuter(),NAME_None);
					if( newSpec->defineFor( Marker, Nav, Scout ) )
					{
						if( OldSpec == NULL || *newSpec <= *OldSpec )
						{
							newSpec->PruneSpecList.AddItem( UReachSpec::StaticClass() );
							Marker->PathList.AddItem( newSpec );
							Helper->AddSpecToList( newSpec, Helper->BlockedWhenUp );

							//debug
							/*debugf(TEXT("A -- Added spec %s (%d/%d) from %s to %s"), 
								*newSpec->GetName(), newSpec->CollisionRadius, newSpec->CollisionHeight, *newSpec->Start->GetName(), *newSpec->End->GetName() );
							if( OldSpec != NULL )
								debugf(TEXT("..... OldSpec %s (%d/%d)"), *OldSpec->GetName(), OldSpec->CollisionRadius, OldSpec->CollisionHeight );*/

							ACoverSlotMarker* OtherMarker = Cast<ACoverSlotMarker>(Nav);
							if( OtherMarker == NULL ||
								MarkerList.FindItemIndex( OtherMarker ) < 0 )
							{
								AddNavs.AddUniqueItem( Nav );
								if( OldSpec != NULL )
								{
									Helper->AddSpecToList( OldSpec, Helper->BlockedWhenDown );
									OldSpec->Start->PathList.RemoveItem( OldSpec );
								}
							}
						}
					}
				}
			}
		}

		Marker->bIgnoreSizeLimits = FALSE;
	}
	GWarn->StatusUpdatef( 6, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));


	for( INT AddIdx = 0; AddIdx < AddNavs.Num(); AddIdx++ )
	{
		ANavigationPoint* Add = AddNavs(AddIdx);
		ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(Add);
		if( Marker != NULL )
		{
			 Marker->bIgnoreSizeLimits = TRUE;
		}

		TArray<FNavigationOctreeObject*> NavObjects;  
		GWorld->NavigationOctree->RadiusCheck(Add->Location, MAXPATHDIST, NavObjects);
		//for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
		for(INT Idx=0;Idx<NavObjects.Num();Idx++)
		{
			ANavigationPoint* Nav = NavObjects(Idx)->GetOwner<ANavigationPoint>();
			if( Nav != Add && Nav != NULL )
			{
				UReachSpec* OldSpec = Add->GetReachSpecTo( Nav );
				if(  OldSpec != NULL && 
					(OldSpec->GetClass() == USwatTurnReachSpec::StaticClass() ||
					 OldSpec->GetClass() == USlotToSlotReachSpec::StaticClass()) )
				{
					continue;
				}

				if( Add->CanConnectTo( Nav, TRUE ) )
				{
					UClass*		ReachSpecClass  = Add->GetReachSpecClass( Nav, Scout->GetDefaultReachSpecClass() );
					UReachSpec *newSpec			= ConstructObject<UReachSpec>(ReachSpecClass,Add->GetOuter(),NAME_None);
					if( newSpec->defineFor( Add, Nav, Scout ) )
					{
						//debug
						/*debugf(TEXT("B -- Added spec %s (%d/%d) from %s to %s"), 
							*newSpec->GetName(), newSpec->CollisionRadius, newSpec->CollisionHeight, *newSpec->Start->GetName(), *newSpec->End->GetName() );
						if( OldSpec != NULL )
						{
							debugf(TEXT("..... OldSpec %s (%d/%d)"), *OldSpec->GetName(), OldSpec->CollisionRadius, OldSpec->CollisionHeight );

							debugf(TEXT("..... <= %s ... supports %s"), (*newSpec <= *OldSpec)?TEXT("TRUE"):TEXT("FALSE"), OldSpec->supports(newSpec->CollisionRadius,newSpec->CollisionHeight,newSpec->reachFlags,newSpec->MaxLandingVelocity )?TEXT("TRUE"):TEXT("FALSE") );
						}*/

						if(  OldSpec == NULL || 
							 OldSpec->GetClass() != newSpec->GetClass() || 
							!OldSpec->supports(newSpec->CollisionRadius,newSpec->CollisionHeight,newSpec->reachFlags,newSpec->MaxLandingVelocity ) )
						{
							//debug
							//debugf(TEXT("add spec... %s -> BlockedWhenUp ... %s -> BlockedWhenDown"), *newSpec->GetName(), OldSpec ? *OldSpec->GetName() : TEXT("NULL") );

							Add->PathList.AddItem( newSpec );
							Helper->AddSpecToList( newSpec, Helper->BlockedWhenUp );
							if( OldSpec != NULL )
							{
								Helper->AddSpecToList( OldSpec, Helper->BlockedWhenDown );
								OldSpec->Start->PathList.RemoveItem( OldSpec );
							}
						}
					}
				}
			}
		}


		if( Marker != NULL )
		{
			Marker->bIgnoreSizeLimits = FALSE;
		}
	}
	GWarn->StatusUpdatef( 8, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

	for( INT MarkerIdx = 0; MarkerIdx < MarkerList.Num(); MarkerIdx++ )
	{
		ACoverSlotMarker* Marker = MarkerList(MarkerIdx);
		Marker->PrunePaths();
	}
	for( INT AddIdx = 0; AddIdx < AddNavs.Num(); AddIdx++ )
	{
		ANavigationPoint* Add = AddNavs(AddIdx);
		Add->PrunePaths();
	}

	for( INT BlockedIdx = 0; BlockedIdx < Helper->BlockedWhenUp.Num(); BlockedIdx++ )
	{
		UReachSpec* Spec = Helper->GetSpec( BlockedIdx, Helper->BlockedWhenUp );
		if( Spec != NULL )
		{
			if( Spec->bPruned )
			{
				//debug
				//debugf(TEXT("C -- Pruned spec %s from %s to %s"), *Spec->GetName(), *Spec->Start->GetName(), *Spec->End->GetName() );

				Helper->BlockedWhenUp.Remove( BlockedIdx--, 1 );
			}
			else
			{
				Spec->bDisabled = bStartPoppedUp;
			}
			if( Spec->Start != NULL )
			{
				Spec->Start->ForceUpdateComponents();
			}
		}
	}
	for( INT BlockedIdx = 0; BlockedIdx < Helper->BlockedWhenDown.Num(); BlockedIdx++ )
	{
		UReachSpec* Spec = Helper->GetSpec( BlockedIdx, Helper->BlockedWhenDown );
		if( Spec != NULL )
		{
			Spec->bDisabled = !bStartPoppedUp;
			Spec->Start->PathList.AddItem( Spec );
			if( Spec->Start != NULL )
			{
				Spec->Start->ForceUpdateComponents();
			}
		}
	}
	GWarn->StatusUpdatef( 9, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));


	for( INT MarkerIdx = 0; MarkerIdx < MarkerList.Num(); MarkerIdx++ )
	{
		ACoverSlotMarker* Marker = MarkerList(MarkerIdx);
		Marker->SortPathList();
	}
	for( INT AddIdx = 0; AddIdx < AddNavs.Num(); AddIdx++ )
	{
		ANavigationPoint* Add = AddNavs(AddIdx);
		Add->SortPathList();
	}

	Helper->SetBlockingVolumeCollision( TRUE, BlockingVolumes );
	Helper->SetMatineePosition( FALSE, MatineeList );

	for( INT BlockedIdx = 0; BlockedIdx < Helper->BlockedWhenUp.Num(); BlockedIdx++ )
	{
		FCrossLevelReachSpec& Spec = Helper->BlockedWhenUp(BlockedIdx);
		Spec.Spec = NULL;
	}
	for( INT BlockedIdx = 0; BlockedIdx < Helper->BlockedWhenDown.Num(); BlockedIdx++ )
	{
		FCrossLevelReachSpec& Spec = Helper->BlockedWhenDown(BlockedIdx);
		Spec.Spec = NULL;
	}

	//debug
	//DumpAllPaths( this );
	GWarn->StatusUpdatef( 10, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

}

void USeqAct_PopupCoverControl::Activated()
{	
	Super::Activated();

	RetryCount = 0;

	// Clear out matinee outputs
	if( OutputLinks.Num() == 4 )
	{
		OutputLinks.Remove( 3, 1 );
	}

	UBOOL bUp = FALSE;
	if( InputLinks(0).bHasImpulse )
	{
		bUp = TRUE;
	}
	else
	if( InputLinks(1).bHasImpulse )
	{
		bUp = FALSE;
	}
	else
	{
		bUp = !bCurrentPoppedUp;
	}

	SetCoverState( bUp );
}

UBOOL USeqAct_PopupCoverControl::CanRetryPopup()
{
	if( bRetryOnFailPopUp && Helper != NULL && bDesiredPoppedUp != bCurrentPoppedUp && (NumRetry == 0 || RetryCount <= NumRetry) )
	{
		return TRUE;
	}
	return FALSE;
}

UBOOL USeqAct_PopupCoverControl::UpdateOp( FLOAT DeltaTime )
{
	if( CanRetryPopup() )
	{
		if( Helper->WorldInfo->TimeSeconds > NextRetryTime )
		{
			SetCoverState( bDesiredPoppedUp );
		}
	}

	return Super::UpdateOp( DeltaTime );
}

void USeqAct_PopupCoverControl::DeActivated()
{
	if( bDesiredPoppedUp == bCurrentPoppedUp )
	{
		ActivateOutputLink(bCurrentPoppedUp?0:1);

		if( bAutoPlayMatinee )
		{
			INT OutIdx = OutputLinks.AddZeroed();
			check(OutIdx==3);
			FSeqOpOutputLink& OutLink = OutputLinks(OutIdx);
			
			INT InputIdx = bCurrentPoppedUp ? 0 : 1;
			for( INT MatIdx = 0; MatIdx < MatineeList.Num(); MatIdx++ )
			{
				USeqAct_Interp* Interp = MatineeList(MatIdx);
				if( Interp == NULL )
					continue;

				INT OutInIdx = OutLink.Links.AddZeroed();
				FSeqOpOutputInputLink& OutInLink = OutLink.Links(OutInIdx);
				OutInLink.LinkedOp = Interp;
				OutInLink.InputLinkIdx = InputIdx;
			}

			ActivateOutputLink(3);
		}
	}
	else
	{
		bDesiredPoppedUp = bCurrentPoppedUp;

		// Output for failure
		ActivateOutputLink(2);
	}
}

/** 
 *	When UP input triggered BLOCK BlockedWhenUp, UNBLOCK BlockedWhenDown
 *	When DOWN input triggered BLOCK BlockWhenDown, UNBLOCK BlockWhenUp
 */
void USeqAct_PopupCoverControl::SetCoverState( UBOOL bUp )
{
	bDesiredPoppedUp = bUp;
	if( bDesiredPoppedUp == bCurrentPoppedUp )
	{
		NextRetryTime = 0.f;
		return;
	}

	// If should check for encroacher and popping up
	UBOOL bEncroach = FALSE;
	if( bCheckEncroachOnPopUp && bDesiredPoppedUp )
	{
		// Loop through each blocking volume
		for( INT BlockIdx = 0; BlockIdx < BlockingVolumes.Num() && !bEncroach; BlockIdx++ )
		{
			ABlockingVolume* BV = BlockingVolumes(BlockIdx);
			if( BV == NULL )
				continue;

			// Turn on collision
			BV->SetCollision( TRUE, TRUE, FALSE );
			BV->CollisionComponent->SetBlockRigidBody( TRUE );

			// Check hash for encroachers
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult* FirstHit = GWorld->Hash ? GWorld->Hash->ActorEncroachmentCheck(GMainThreadMemStack, BV, BV->Location, BV->Rotation, TRACE_AllColliding & (~TRACE_LevelGeometry)) : NULL;	
			for( FCheckResult* Test = FirstHit; Test != NULL; Test = Test->GetNext() )
			{
				// If colliding actor blocks the BV
				if(	 Test->Actor != BV &&
					!Test->Actor->bWorldGeometry &&  
					!Test->Actor->IsBasedOn(BV) && 
					(Test->Component == NULL || 
						Test->Component->BlockNonZeroExtent) &&	
					 BV->IsBlockedBy( Test->Actor, Test->Component ) )
				{
					// Collision detected, break from loop
					bEncroach = TRUE;

					// If encroached by AI pawn... tell them to move!
					APawn* P = Test->Actor->GetAPawn();
					if( P != NULL )
					{
						AGearAI* AI = Cast<AGearAI>(P->Controller);
						if( AI != NULL )
						{
							AI->eventStepAsideFor( P );
						}
					}

					break;
				}
			}
			Mark.Pop();
		}

		// If hit something and not going to go up...
		if( bEncroach )
		{
			for( INT BlockIdx = 0; BlockIdx < BlockingVolumes.Num(); BlockIdx++ )
			{
				ABlockingVolume* BV = BlockingVolumes(BlockIdx);
				if( BV == NULL )
					continue;

				// Turn off collision
				BV->SetCollision( FALSE, TRUE, FALSE );
				BV->CollisionComponent->SetBlockRigidBody( FALSE );
			}
		}
	}

	// If not going to crush anything by moving...
	if( !bEncroach )
	{
		bCurrentPoppedUp = bUp;

		for( INT BlockIdx = 0; BlockIdx < BlockingVolumes.Num(); BlockIdx++ )
		{
			ABlockingVolume* BV = BlockingVolumes(BlockIdx);
			if( BV == NULL )
				continue;

			if( bCurrentPoppedUp )
			{
				// Turn on collision
				BV->SetCollision( TRUE, TRUE, FALSE );
				BV->CollisionComponent->SetBlockRigidBody( TRUE );
			}
			else
			{
				// Turn off collision
				BV->SetCollision( FALSE, TRUE, FALSE );
				BV->CollisionComponent->SetBlockRigidBody( FALSE );
			}

			BV->eventForceNetRelevant();
			BV->SetForcedInitialReplicatedProperty( FindObject<UProperty>(NULL,TEXT("Engine.Actor.bCollideActors")), BV->bCollideActors == BV->GetClass()->GetDefaultActor()->bCollideActors );
		}

		PublishLinkedVariableValues();

		if( Helper != NULL )
		{
			// For each path to block when up...
			for( INT PathIdx = 0; PathIdx < Helper->BlockedWhenUp.Num(); PathIdx++ )
			{
				UReachSpec* Spec = Helper->GetSpec( PathIdx, Helper->BlockedWhenUp );
				if( Spec != NULL )
				{
					// Disable it if UP impulse triggered, Enable it if DOWN impulse trigged
					Spec->bDisabled = bUp;
					Spec->Start->UpdateMaxPathSize();

					//debug
					//debugf(TEXT("X -- %s %s to %s ... %d"), *Spec->GetName(), *Spec->Start->GetName(), *Spec->End->GetName(), Spec->bDisabled );
				}
			}
			// For each path to block when down...
			for( INT PathIdx = 0; PathIdx < Helper->BlockedWhenDown.Num(); PathIdx++ )
			{
				UReachSpec* Spec = Helper->GetSpec( PathIdx, Helper->BlockedWhenDown );
				if( Spec != NULL )
				{
					Spec->bDisabled = !bUp;
					Spec->Start->UpdateMaxPathSize();

					//debug
					//debugf(TEXT("Z -- %s %s to %s ... %d"), *Spec->GetName(), *Spec->Start->GetName(), *Spec->End->GetName(), Spec->bDisabled );
				}
			}

			FLOAT Delay = bUp ? DelayAdjustCoverUp : DelayAdjustCoverDown;
			if( Delay > 0.f )
			{
				Helper->SetTimer( Delay, FALSE, FName(TEXT("AdjustCover")) );
			}
			else
			{
				Helper->eventAdjustCover();
			}	
		}	
	}
	else
	// Otherwise, if should retry
	if( CanRetryPopup() )
	{
		// Update retry counter and set retry time
		RetryCount++;
		NextRetryTime = Helper->WorldInfo->TimeSeconds + 0.25f;
	}
}

void AGearControlHelper::AddSpecToList( UReachSpec* newSpec, TArray<FCrossLevelReachSpec>& List )
{
	List.AddItem( FCrossLevelReachSpec(newSpec) );
}

UReachSpec* AGearControlHelper::GetSpec( INT Idx, TArray<FCrossLevelReachSpec>& List )
{
	if( Idx >= 0 && Idx < List.Num() )
	{
		FCrossLevelReachSpec& Spec = List(Idx);
		if( Spec.Spec != NULL )
		{
			return Spec.Spec;
		}
		else if( Spec.Start.Nav() != NULL )
		{
			for( INT i = 0; i < Spec.Start.Nav()->PathList.Num(); i++ )
			{
				UReachSpec* RS = Spec.Start.Nav()->PathList(i);
				if( RS != NULL && 
					RS->GetClass() == Spec.SpecClass	&&
					RS->CollisionHeight == Spec.Height	&&
					RS->CollisionRadius == Spec.Radius	&&
					RS->End.Nav()  == Spec.End.Nav()	)
				{
					return RS;
				}
			}
		}
	}

	return NULL;
}

FCrossLevelReachSpec* AGearControlHelper::GetRef( ANavigationPoint* Start, ANavigationPoint* End, TArray<FCrossLevelReachSpec>& List, INT Height, INT Radius )
{
	for( INT Idx = 0; Idx < List.Num(); Idx++ )
	{
		FCrossLevelReachSpec& Ref = List(Idx);
		if( Start == *Ref.Start	 &&
			End	  == *Ref.End    &&
			Height == Ref.Height &&
			Radius == Ref.Radius )
		{
			return &Ref;
		}
	}
	return NULL;
}

void AGearControlHelper::SetBlockingVolumeCollision( UBOOL bCollide, TArray<ABlockingVolume*>& BlockingVolumes )
{
	for( INT Idx = 0; Idx < BlockingVolumes.Num(); Idx++ )
	{
		ABlockingVolume* BV = BlockingVolumes(Idx);
		if( BV != NULL )
		{
			BV->SetCollision( bCollide, TRUE, FALSE );
			BV->CollisionComponent->SetBlockRigidBody( bCollide );
		}
	}
}

void AGearControlHelper::SetMatineePosition( UBOOL bEnd, TArray<USeqAct_Interp*>& MatineeList )
{
	if( bEnd )
	{
		for( INT MatineeIdx = 0; MatineeIdx < MatineeList.Num(); MatineeIdx++ )
		{
			USeqAct_Interp* Interp = MatineeList(MatineeIdx);
			if( Interp != NULL && Interp->Position != 1.f )
			{
				Interp->InitInterp();
				for( INT GroupIdx = 0; GroupIdx < Interp->GroupInst.Num(); GroupIdx++ )
				{
					Interp->GroupInst(GroupIdx)->SaveGroupActorState();
					AActor* GroupActor = Interp->GroupInst(GroupIdx)->GetGroupActor();
					if( GroupActor )
					{
						const UBOOL bOnlyCaptureChildren = FALSE;
						Interp->SaveActorTransforms( GroupActor, bOnlyCaptureChildren );
					}
				}
				Interp->UpdateInterp( 1.f, TRUE, TRUE );
			}
		}
	}
	else
	{
		for( INT MatineeIdx = 0; MatineeIdx < MatineeList.Num(); MatineeIdx++ )
		{
			USeqAct_Interp* Interp = MatineeList(MatineeIdx);
			if( Interp != NULL && Interp->Position != 0.f )
			{
				for( INT GroupIdx = 0; GroupIdx < Interp->GroupInst.Num(); GroupIdx++ )
				{
					Interp->GroupInst(GroupIdx)->RestoreGroupActorState();
				}
				Interp->RestoreActorTransforms();
				Interp->TermInterp();
				Interp->Position = 0.f;
			}
		}
	}
}

void APopupCoverControlHelper::ClearCrossLevelReferences()
{
	Super::ClearCrossLevelReferences();

	for( INT PathIdx = 0; PathIdx < BlockedWhenUp.Num(); PathIdx++ )
	{
		FCrossLevelReachSpec& Spec = BlockedWhenUp(PathIdx);
		if( (*Spec.Start == NULL && !Spec.Start.Guid.IsValid()) ||
			(*Spec.End   == NULL && !Spec.End.Guid.IsValid())   )
		{
			BlockedWhenUp.Remove(PathIdx--,1);
			continue;
		}
		if( *Spec.Start != NULL && Spec.Start->GetOutermost() != GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec.Start.Guid = *Spec.Start->GetGuid();
			
		}
		if( *Spec.End != NULL && Spec.End->GetOutermost() != GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec.End.Guid = *Spec.End->GetGuid();
		}
	}
	for( INT PathIdx = 0; PathIdx < BlockedWhenDown.Num(); PathIdx++ )
	{
		FCrossLevelReachSpec& Spec = BlockedWhenDown(PathIdx);
		if( (*Spec.Start == NULL && !Spec.Start.Guid.IsValid()) ||
			(*Spec.End   == NULL && !Spec.End.Guid.IsValid())   )
		{
			BlockedWhenDown.Remove(PathIdx--,1);
			continue;
		}
		if( *Spec.Start != NULL && Spec.Start->GetOutermost() != GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec.Start.Guid = *Spec.Start->GetGuid();

		}
		if( *Spec.End != NULL && Spec.End->GetOutermost() != GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec.End.Guid = *Spec.End->GetGuid();
		}
	}
}

void APopupCoverControlHelper::GetActorReferences( TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel )
{
	if( bHasCrossLevelPaths )
	{
		for( INT PathIdx = 0; PathIdx < BlockedWhenUp.Num(); PathIdx++ )
		{
			FCrossLevelReachSpec& Spec = BlockedWhenUp(PathIdx);
			if( Spec.Start.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if( ( bIsRemovingLevel && *Spec.Start != NULL) ||
					(!bIsRemovingLevel && *Spec.Start == NULL) )
				{
					ActorRefs.AddItem( &Spec.Start );
				}
			}
			if( Spec.End.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if( ( bIsRemovingLevel && *Spec.End != NULL) ||
					(!bIsRemovingLevel && *Spec.End == NULL) )
				{
					ActorRefs.AddItem( &Spec.End );
				}
			}
		}

		for( INT PathIdx = 0; PathIdx < BlockedWhenDown.Num(); PathIdx++ )
		{
			FCrossLevelReachSpec& Spec = BlockedWhenDown(PathIdx);
			if( Spec.Start.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if( ( bIsRemovingLevel && *Spec.Start != NULL) ||
					(!bIsRemovingLevel && *Spec.Start == NULL) )
				{
					ActorRefs.AddItem( &Spec.Start );
				}
			}
			if( Spec.End.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if( ( bIsRemovingLevel && *Spec.End != NULL) ||
					(!bIsRemovingLevel && *Spec.End == NULL) )
				{
					ActorRefs.AddItem( &Spec.End );
				}
			}
		}
	}
}

/************************************************************************
	USeqAct_DoorControl
************************************************************************/
void USeqAct_DoorControl::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	if( PropertyThatChanged != NULL )
	{
		if( PropertyThatChanged->GetFName() == FName(TEXT("MatineeObjList")) )
		{
			ResolveMatineeNames();
		}
	}
}

void USeqAct_DoorControl::ResolveMatineeNames()
{
	MatineeList.Empty();

	USequence* GameSeq = GWorld->GetWorldInfo()->GetGameSequence();
	if( GameSeq != NULL )
	{
		for( INT Idx = 0; Idx < MatineeObjList.Num(); Idx++ )
		{
			TArray<USequenceObject*> ObjList;
			GameSeq->FindSeqObjectsByName( *MatineeObjList(Idx), TRUE, ObjList, TRUE );
			for( INT ObjIdx = 0; ObjIdx < ObjList.Num(); ObjIdx++ )
			{
				USeqAct_Interp* Interp = Cast<USeqAct_Interp>(ObjList(ObjIdx));
				if( Interp != NULL )
				{
					MatineeList.AddUniqueItem( Interp );						
				}			
			}
		}
	}
}

void USeqAct_DoorControl::Initialize()
{
	Super::Initialize();

	// If playing the game... turn off the BV collision
	if( (GIsGame || GIsPlayInEditorWorld) && Helper != NULL )
	{
		Helper->SetBlockingVolumeCollision( FALSE, BlockingVolumes );
	}

	bCurrentOpen = !bStartOpen;
	CurStepToOpen = bStartOpen ? NumStepsToOpen : 0;
	SetDoorState( bStartOpen );
}


void USeqAct_DoorControl::PrePathBuild( AScout* Scout )
{
	if( Helper == NULL || Helper->SeqObj != this )
	{
		ULevel* OuterLevel = Cast<ULevel>(GetRootSequence()->GetOuter());
		Helper = ConstructObject<ADoorControlHelper>(ADoorControlHelper::StaticClass(), OuterLevel, NAME_None, RF_Transactional, ADoorControlHelper::StaticClass()->GetDefaultActor());
		OuterLevel->Actors.AddItem( Helper );
		Helper->SeqObj = this;
		Helper->WorldInfo = GWorld->GetWorldInfo();
	}

	for( INT Idx = 0; Idx < BlockingVolumes.Num(); Idx++ )
	{
		ABlockingVolume* BV = BlockingVolumes(Idx);
		if( BV != NULL )
		{
			BV->bPathColliding = TRUE;
		}
	}
}

void USeqAct_DoorControl::OnDelete()
{
	Super::OnDelete();

	if( Helper != NULL )
	{
		GWorld->DestroyActor( Helper );
	}
}

void USeqAct_DoorControl::PostPathBuild( AScout* Scout )
{
	if( Helper == NULL )
		return;

	GWarn->StatusUpdatef( 0, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

	// Try to resolve matinee names again if list is empty
	// Catches the case of adding a matinee name to the controller before naming the matinee object
	if( MatineeList.Num() == 0 )
	{
		ResolveMatineeNames();
	}

	PublishLinkedVariableValues();

	Helper->BlockedWhenShut.Empty();

	TMap< ANavigationPoint*, TArray<ANavigationPoint*> > NavMap;
	for( INT NavIdx = 0; NavIdx < NavPoints.Num(); NavIdx++ )
	{
		ANavigationPoint* Nav = NavPoints(NavIdx);
		if( Nav == NULL )
			continue;

		TArray<ANavigationPoint*> List;
		for( INT TestIdx = 0; TestIdx < NavPoints.Num(); TestIdx++ )
		{
			ANavigationPoint* TestNav = NavPoints(TestIdx);
			if( TestNav == NULL || TestNav == Nav )
				continue;

			// Trace forward from the nav to see if there is a blocking volume in front of it
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult* FirstHit = NULL;
			FirstHit = GWorld->MultiLineCheck
			(
				GMainThreadMemStack,
				Nav->Location,
				TestNav->Location,
				FVector(5,5,5),
				TRACE_World,
				Nav
			);

			for( FCheckResult* TestHit = FirstHit; TestHit != NULL; TestHit = TestHit->GetNext() )
			{
				// If hit a blocking volume which is in our valid list
				ABlockingVolume* BV = Cast<ABlockingVolume>(TestHit->Actor);
				if( BV != NULL && BlockingVolumes.FindItemIndex( BV ) >= 0 )
				{
					// Add to our affected list
					List.AddUniqueItem( TestNav );
					break;
				}
			}
			Mark.Pop();
		}

		if( List.Num() > 0 )
		{
			NavMap.Set( Nav, List );
		}
	}
	GWarn->StatusUpdatef( 3, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));


	Helper->SetBlockingVolumeCollision( FALSE, BlockingVolumes );
	Helper->SetMatineePosition( TRUE, MatineeList );
	
	for( TMap< ANavigationPoint*, TArray<ANavigationPoint*> >::TIterator It(NavMap); It; ++It )
	{
		ANavigationPoint* Nav = It.Key();
		TArray<ANavigationPoint*>& List = It.Value();

		for( INT TestIdx = 0; TestIdx < List.Num(); TestIdx++ )
		{
			ANavigationPoint* TestNav = List(TestIdx);
			if( Nav->CanConnectTo( TestNav, TRUE ) )
			{
				UClass*		ReachSpecClass  = Nav->GetReachSpecClass( TestNav, Scout->GetDefaultReachSpecClass() );
				UReachSpec *newSpec			= ConstructObject<UReachSpec>(ReachSpecClass,Nav->GetOuter(),NAME_None);
				if( newSpec->defineFor( Nav, TestNav, Scout ) )
				{
					Nav->PathList.AddItem( newSpec );
					Helper->AddSpecToList( newSpec, Helper->BlockedWhenShut );
				}
			}
		}
	}
	GWarn->StatusUpdatef( 6, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));


	Helper->SetBlockingVolumeCollision( TRUE, BlockingVolumes );
	Helper->SetMatineePosition( FALSE, MatineeList );
	
	for( TMap< ANavigationPoint*, TArray<ANavigationPoint*> >::TIterator It(NavMap); It; ++It )
	{
		ANavigationPoint* Nav = It.Key();
		Nav->PrunePaths();
	}

	for( INT BlockedIdx = 0; BlockedIdx < Helper->BlockedWhenShut.Num(); BlockedIdx++ )
	{
		UReachSpec* Spec = Helper->GetSpec( BlockedIdx, Helper->BlockedWhenShut );
		if( Spec != NULL )
		{
			if( Spec->bPruned )
			{
				Helper->BlockedWhenShut.Remove( BlockedIdx--, 1 );
			}
			else
			{
				Spec->bDisabled = !bStartOpen;
			}
			if( Spec->Start != NULL )
			{
				Spec->Start->ForceUpdateComponents();
			}
		}
	}

	for( TMap< ANavigationPoint*, TArray<ANavigationPoint*> >::TIterator It(NavMap); It; ++It )
	{
		ANavigationPoint* Nav = It.Key();
		Nav->SortPathList();
	}

	GWarn->StatusUpdatef( 8, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

	for( INT BlockedIdx = 0; BlockedIdx < Helper->BlockedWhenShut.Num(); BlockedIdx++ )
	{
		FCrossLevelReachSpec& Spec = Helper->BlockedWhenShut(BlockedIdx);
		Spec.Spec = NULL;
	}

	GWarn->StatusUpdatef( 10, 10, *FString::Printf(TEXT("%s PostPathBuild..."), *GetName() ));

}

void USeqAct_DoorControl::Activated()
{	
	Super::Activated();

	RetryCount = 0;
	bAborted = FALSE;

	// Clear out matinee outputs
	if( OutputLinks.Num() == 4 )
	{
		OutputLinks.Remove( 3, 1 );
	}

	UBOOL bOpen = FALSE;
	if( InputLinks(0).bHasImpulse )
	{
		bOpen = TRUE;
	}
	else
	if( InputLinks(1).bHasImpulse )
	{
		bOpen = FALSE;
	}
	else
	{
		bOpen = !bCurrentOpen;
	}

	SetDoorState( bOpen );
}

UBOOL USeqAct_DoorControl::CanRetryDoor()
{
	if( bRetryOnFail && Helper != NULL && bDesiredOpen != bCurrentOpen && (NumRetry == 0 || RetryCount <= NumRetry) )
	{
		return TRUE;
	}
	return FALSE;
}

UBOOL USeqAct_DoorControl::UpdateOp( FLOAT DeltaTime )
{
	if( CanRetryDoor() )
	{
		if( Helper->WorldInfo->TimeSeconds > NextRetryTime )
		{
			SetDoorState( bDesiredOpen );
		}
	}

	return Super::UpdateOp( DeltaTime );
}

void USeqAct_DoorControl::DeActivated()
{
	if( !bAborted && bDesiredOpen == bCurrentOpen )
	{
		ActivateOutputLink(bCurrentOpen?0:1);

		if( bAutoPlayMatinee )
		{
			INT OutIdx = OutputLinks.AddZeroed();
			check(OutIdx==3);
			FSeqOpOutputLink& OutLink = OutputLinks(OutIdx);

			INT InputIdx = bCurrentOpen ? 0 : 1;
			if( bStartOpen )
			{
				InputIdx = (InputIdx + 1) % 2;
			}
			for( INT MatIdx = 0; MatIdx < MatineeList.Num(); MatIdx++ )
			{
				USeqAct_Interp* Interp = MatineeList(MatIdx);
				if( Interp == NULL )
					continue;

				INT OutInIdx = OutLink.Links.AddZeroed();
				FSeqOpOutputInputLink& OutInLink = OutLink.Links(OutInIdx);
				OutInLink.LinkedOp = Interp;
				OutInLink.InputLinkIdx = InputIdx;
			}

			ActivateOutputLink(3);
		}
	}
	else
	if( bAborted )
	{
		bDesiredOpen = bCurrentOpen;

		// Output for failure
		ActivateOutputLink(2);
	}
}

void USeqAct_DoorControl::SetDoorState( UBOOL bOpen )
{
	bDesiredOpen = bOpen;
	if( bDesiredOpen == bCurrentOpen && (!bCurrentOpen || CurStepToOpen == NumStepsToOpen) )
	{
		bAborted = TRUE;
		NextRetryTime = 0.f;
		return;
	}

	// If should check for encroacher
	UBOOL bEncroach = FALSE;
	if( bCheckEncroachOnMove )
	{
		// Loop through each blocking volume
		for( INT BlockIdx = 0; BlockIdx < BlockingVolumes.Num() && !bEncroach; BlockIdx++ )
		{
			ABlockingVolume* BV = BlockingVolumes(BlockIdx);
			if( BV == NULL )
				continue;

			// Turn on collision
			BV->SetCollision( TRUE, TRUE, FALSE );
			BV->CollisionComponent->SetBlockRigidBody( TRUE );

			// Check hash for encroachers
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult* FirstHit = GWorld->Hash ? GWorld->Hash->ActorEncroachmentCheck(GMainThreadMemStack, BV, BV->Location, BV->Rotation, TRACE_AllColliding & (~TRACE_LevelGeometry)) : NULL;	
			for( FCheckResult* Test = FirstHit; Test != NULL; Test = Test->GetNext() )
			{
				// If colliding actor blocks the BV
				if(	 Test->Actor != BV &&
					!Test->Actor->bWorldGeometry &&  
					!Test->Actor->IsBasedOn(BV) && 
					(Test->Component == NULL || 
					Test->Component->BlockNonZeroExtent) &&	
					BV->IsBlockedBy( Test->Actor, Test->Component ) )
				{
					// Collision detected, break from loop
					bEncroach = TRUE;
					break;
				}
			}
			Mark.Pop();

			// Turn off collision
			BV->SetCollision( FALSE, TRUE, FALSE );
			BV->CollisionComponent->SetBlockRigidBody( FALSE );
		}
	}

	// If not going to crush anything by moving...
	if( !bEncroach )
	{
		bCurrentOpen  = bOpen;
		CurStepToOpen = bCurrentOpen ? CurStepToOpen + 1 : 0;

		// Don't update stuff until door is completely open (or shut)
		if( !bCurrentOpen || CurStepToOpen == NumStepsToOpen )
		{
			PublishLinkedVariableValues();

			if( Helper != NULL )
			{
				// For each path to block when shut...
				for( INT PathIdx = 0; PathIdx < Helper->BlockedWhenShut.Num(); PathIdx++ )
				{
					UReachSpec* Spec = Helper->GetSpec( PathIdx, Helper->BlockedWhenShut );
					if( Spec != NULL )
					{
						Spec->bDisabled = !bOpen;
						Spec->Start->UpdateMaxPathSize();
					}
				}

				FLOAT Delay = bOpen ? DelayAdjustCoverOpen : DelayAdjustCoverShut;
				if( Delay > 0.f )
				{
					Helper->SetTimer( Delay, FALSE, FName(TEXT("AdjustCover")) );
				}
				else
				{
					Helper->eventAdjustCover();
				}
			}
		}
	}
	else
	// Otherwise, if should retry
	if( CanRetryDoor() )
	{
		// Update retry counter and set retry time
		RetryCount++;
		NextRetryTime = Helper->WorldInfo->TimeSeconds + 0.25f;
	}
	else
	{
		bAborted = TRUE;
	}
}

void ADoorControlHelper::ClearCrossLevelReferences()
{
	Super::ClearCrossLevelReferences();

	for( INT PathIdx = 0; PathIdx < BlockedWhenShut.Num(); PathIdx++ )
	{
		FCrossLevelReachSpec& Spec = BlockedWhenShut(PathIdx);
		if( (*Spec.Start == NULL && !Spec.Start.Guid.IsValid()) ||
			(*Spec.End   == NULL && !Spec.End.Guid.IsValid())   )
		{
			BlockedWhenShut.Remove(PathIdx--,1);
			continue;
		}
		if( *Spec.Start != NULL && Spec.Start->GetOutermost() != GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec.Start.Guid = *Spec.Start->GetGuid();

		}
		if( *Spec.End != NULL && Spec.End->GetOutermost() != GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec.End.Guid = *Spec.End->GetGuid();
		}
	}
}

void ADoorControlHelper::GetActorReferences( TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel )
{
	if( bHasCrossLevelPaths )
	{
		for( INT PathIdx = 0; PathIdx < BlockedWhenShut.Num(); PathIdx++ )
		{
			FCrossLevelReachSpec& Spec = BlockedWhenShut(PathIdx);
			if( Spec.Start.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if( ( bIsRemovingLevel && *Spec.Start != NULL) ||
					(!bIsRemovingLevel && *Spec.Start == NULL) )
				{
					ActorRefs.AddItem( &Spec.Start );
				}
			}
			if( Spec.End.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if( ( bIsRemovingLevel && *Spec.End != NULL) ||
					(!bIsRemovingLevel && *Spec.End == NULL) )
				{
					ActorRefs.AddItem( &Spec.End );
				}
			}
		}
	}
}

void USeqAct_TriggerTracker::Initialize()
{
	Super::Initialize();

	if( Helper == NULL || Helper->SeqObj != this )
	{
		ULevel* OuterLevel = Cast<ULevel>(GetRootSequence()->GetOuter());
		Helper = ConstructObject<ATriggerTrackerHelper>(ATriggerTrackerHelper::StaticClass(), OuterLevel, NAME_None, RF_Transactional, ATriggerTrackerHelper::StaticClass()->GetDefaultActor());
		OuterLevel->Actors.AddItem( Helper );
		Helper->SeqObj = this;
		Helper->WorldInfo = GWorld->GetWorldInfo();
	}
}

void USeqAct_TriggerTracker::OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex )
{
	Super::OnReceivedImpulse( ActivatorOp, InputLinkIndex );

	if( InputLinkIndex == 0 )
	{
		SetTracking( TRUE );
	}
	else
	if( InputLinkIndex == 1 )
	{
		bAborted = TRUE;
		SetTracking( FALSE );
	}
}

void USeqAct_TriggerTracker::SetTracking( UBOOL bTrack )
{
	if( bTrack != bTracking )
	{
		bTracking = bTrack;

		PublishLinkedVariableValues();
		if( bTracking )
		{
			// setup list of volumes vs targets
			SetupTrackingList();
		}
		else
		{
			// clear list of volumes vs targets
			ClearTrackingList();
		}
	}
}

void USeqAct_TriggerTracker::DeActivated()
{
	Super::DeActivated();

	ClearTrackingList();
}

UBOOL USeqAct_TriggerTracker::UpdateOp( FLOAT DeltaTime )
{
	if( bTracking )
	{
		bAborted = FALSE;
		for( INT Idx = 0; Idx < InfoList.Num(); Idx++ )
		{
			if( !InfoList(Idx).bTouchedFrontTrigger )
			{
				return 0;
			}
		}
		return 1;
	}

	return 0;
}

void USeqAct_TriggerTracker::SetupTrackingList()
{
	if (FrontTrigVol != NULL && BackTrigVol != NULL)
	{
		BackTrigVol->AssociatedActor  = Helper;
		BackTrigVol->GotoState( FName(TEXT("AssociatedTouch")) );
		FrontTrigVol->AssociatedActor = Helper;
		FrontTrigVol->GotoState( FName(TEXT("AssociatedTouch")) );

		InfoList.Empty();
		for( INT Idx = 0; Idx < Targets.Num(); Idx++ )
		{
			AActor* Target = Cast<AActor>(Targets(Idx));
			if( Target == NULL )
				continue;
			AController* C = Target->GetAController();
			if( C != NULL )
			{
				Target = C->Pawn;
			}
			if( Target == NULL )
				continue;

			INT AddIdx = InfoList.AddZeroed();
			InfoList(AddIdx).Target = Target;
		}		
	}
}

void USeqAct_TriggerTracker::ClearTrackingList()
{
	if (FrontTrigVol != NULL && BackTrigVol != NULL)
	{
		if( BackTrigVol->AssociatedActor == Helper )
		{
			BackTrigVol->AssociatedActor = NULL;
			BackTrigVol->GotoState( NAME_None );
		}
		if( FrontTrigVol->AssociatedActor == Helper )
		{
			FrontTrigVol->AssociatedActor = NULL;
			FrontTrigVol->GotoState( NAME_None );
		}
	}
	InfoList.Empty();
}

/** Called when level unloads. */
void USeqAct_FlockSpawner::CleanUp()
{
	if (SpawnedWeapon)
	{
		GWorld->DestroyActor(SpawnedWeapon);
	}

	Super::CleanUp();
}

void USeqAct_FlockSpawner::SpawnDummyWeapon()
{
	// spawn the temporary weapon.  location doesn't matter, we'll align it later
	SpawnedWeapon = Cast<AGearWeapon>(GWorld->SpawnActor(WeaponClass, NAME_None, FVector(0.f), FRotator(0,0,0), NULL, TRUE, FALSE, NULL, NULL));
	if (SpawnedWeapon)
	{
		if (SpawnedWeapon->Mesh)
		{
			SpawnedWeapon->AttachComponent(SpawnedWeapon->Mesh);
			SpawnedWeapon->Mesh->SetHiddenGame(TRUE);
		}
		SpawnedWeapon->RemoteRole = ROLE_None;
		SpawnedWeapon->bHidden = FALSE;
		SpawnedWeapon->bSuppressMuzzleFlash = FALSE;
		SpawnedWeapon->bSuppressTracers = FALSE;
		SpawnedWeapon->bSuppressImpactFX = FALSE;
		SpawnedWeapon->bSuppressAudio = FALSE;
		SpawnedWeapon->bSuppressDamage = TRUE;
	}
}

/** Use the SpawnedWeapon (will create if missing) to dummy fire at target. */
void USeqAct_FlockSpawner::TriggerDummyFire(const FVector& SourceLocation, const FRotator& SourceRotation, const FVector& TargetLocation)
{
	// If we don't have a weapon for dummy fire, create one now
	if(!SpawnedWeapon && WeaponClass)
	{
		SpawnDummyWeapon();
	}

	if(SpawnedWeapon)
	{
		SpawnedWeapon->SetRotation(SourceRotation);
		SpawnedWeapon->SetLocation(SourceLocation);
		SpawnedWeapon->eventDummyFire(0, TargetLocation, NULL, AimError, NULL);
		SpawnedWeapon->eventWeaponStoppedFiring(0);
	}
}

/** Fired when an agent does his 'action' at a target, if 'bSpawnEffects' is TRUE */
void AFlockTestLocust::SpawnActionEffect(const FVector& ActionTarget)
{
	// Use timer to delay between firing 
	bFirePending = TRUE;
	FireTarget = ActionTarget;
	FireTime = GWorld->GetTimeSeconds() + 0.2f;
}

/** Used to check when PlayWeaponFire should be called. */
void AFlockTestLocust::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// If there is a weapon fire pending..
	if(bFirePending)
	{
		// see if its time
		if(GWorld->GetTimeSeconds() >= FireTime)
		{
			PlayWeaponFire();
			bFirePending = FALSE;
		}
	}
}

/** Calls TriggerDummyFire on the owning crowd spawner after a delay */
void AFlockTestLocust::PlayWeaponFire()
{
	// Cache pointer to gears spawner
	if(!FlockSpawner)
	{
		FlockSpawner = Cast<USeqAct_FlockSpawner>(Spawner);
	}

	// If we have gears spawner..
	if(FlockSpawner)
	{
		// .. call fire start location
		FVector FirePos = Location + (FRotationMatrix(Rotation).TransformNormal(FlockSpawner->FireOffset));
		FRotator FireRot = (FireTarget - FirePos).Rotation();

		// And fire projectile
		FlockSpawner->TriggerDummyFire(FirePos, FireRot, FireTarget);
	}
}

void USeqAct_StoreVariable::Activated()
{
	for (INT Idx = 0; Idx < GEngine->GamePlayers.Num(); Idx++) 
	{
		if (GEngine->GamePlayers(Idx) != NULL &&
			GEngine->GamePlayers(Idx)->Actor != NULL &&
			GEngine->GamePlayers(Idx)->Actor->LocalPlayerController())
		{
			AGearPC *PC = Cast<AGearPC>(GEngine->GamePlayers(Idx)->Actor);
			if (PC != NULL)
			{
				// search for an existing entry
				for (INT VarIdx = 0; VarIdx < PC->StoredKismetVariables.Num(); VarIdx++)
				{
					if (PC->StoredKismetVariables(VarIdx).VariableName == VariableName)
					{
						PC->StoredKismetVariables(VarIdx).ObjectValue = ObjectData;
						PC->StoredKismetVariables(VarIdx).FloatValue = FloatData;
						PC->StoredKismetVariables(VarIdx).BoolValue = BoolData;
						PC->StoredKismetVariables(VarIdx).IntValue = IntData;
						PC->StoredKismetVariables(VarIdx).StringValue = StringData;
						return;
					}
				}
				// add a new entry
				INT VarIdx = PC->StoredKismetVariables.AddZeroed(1);
				PC->StoredKismetVariables(VarIdx).VariableName = VariableName;
				PC->StoredKismetVariables(VarIdx).ObjectValue = ObjectData;
				PC->StoredKismetVariables(VarIdx).FloatValue = FloatData;
				PC->StoredKismetVariables(VarIdx).BoolValue = BoolData;
				PC->StoredKismetVariables(VarIdx).IntValue = IntData;
				PC->StoredKismetVariables(VarIdx).StringValue = StringData;
				return;
			}
		}
	}
}

void USeqAct_RetrieveVariable::Activated()
{
	for (INT Idx = 0; Idx < GEngine->GamePlayers.Num(); Idx++) 
	{
		if (GEngine->GamePlayers(Idx) != NULL &&
			GEngine->GamePlayers(Idx)->Actor != NULL &&
			GEngine->GamePlayers(Idx)->Actor->LocalPlayerController())
		{
			AGearPC *PC = Cast<AGearPC>(GEngine->GamePlayers(Idx)->Actor);
			if (PC != NULL)
			{
				// search for an existing entry
				for (INT VarIdx = 0; VarIdx < PC->StoredKismetVariables.Num(); VarIdx++)
				{
					if (PC->StoredKismetVariables(VarIdx).VariableName == VariableName)
					{
						ObjectData = PC->StoredKismetVariables(VarIdx).ObjectValue;
						BoolData = PC->StoredKismetVariables(VarIdx).BoolValue;
						FloatData = PC->StoredKismetVariables(VarIdx).FloatValue;
						IntData = PC->StoredKismetVariables(VarIdx).IntValue;
						StringData = PC->StoredKismetVariables(VarIdx).StringValue;
					}
				}
			}
		}
	}
}

UBOOL USeqAct_Skorge_ChargeAndDuel::UpdateOp(FLOAT DeltaTime)
{								
	// iterate through the Latent actors list,
	for( INT Idx = 0; Idx < LatentActors.Num(); Idx++ )
	{
		AActor *Actor = LatentActors(Idx);
		// if the actor is invalid or no longer is referencing this action
		if( Actor == NULL || Actor->IsPendingKill() || !Actor->LatentActions.ContainsItem(this) )
		{
			// remove the actor from the latent list
			LatentActors.Remove(Idx--,1);
		}
	}

	// return true when our Latentactors list is empty, to indicate all have finished processing
	return (!eventUpdate(DeltaTime) && LatentActors.Num() == 0);
}

void USeqAct_Skorge_ChargeAndDuel::DeActivated()
{
	if( OutputLinks.Num() > DuelResult )
	{
		OutputLinks(DuelResult).ActivateOutputLink();
	}
	DuelResult = 0;
}


UBOOL USeqAct_Leviathan_Mouth::UpdateOp(FLOAT DeltaTime)
{
	if( InputLinks(1).bHasImpulse )
	{
		if( !bAborted )
		{
			USeqAct_Latent::Activated();
			bAborted = TRUE;
		}
		return TRUE;
	}
	return Super::UpdateOp(DeltaTime);
}

void USeqAct_CringePawn::Activated()
{
	UBOOL const bStartingCringe = InputLinks(0).bHasImpulse;

	if (bRadialCringe)
	{
		// we cringe everyone within the specified radius of the target actors.
		if (CringeRadius > 0.f)
		{
			FLOAT const CringeRadSq = Square(CringeRadius);

			TArray<UObject**> Targets;
			GetObjectVars(Targets, TEXT("Target"));

			for (INT TargetIdx=0; TargetIdx<Targets.Num(); ++TargetIdx)
			{
				AActor* TargetActor = Cast<AActor>(*Targets(TargetIdx));
				if (TargetActor)
				{
					// Make all pawns within CringeRadius do a cringe.
					for ( APawn *P=TargetActor->WorldInfo->PawnList; P!=NULL; P=P->NextPawn )
					{
						AGearPawn* GP = Cast<AGearPawn>(P);
						if ( GP && (GP->Location - TargetActor->Location).SizeSquared() < CringeRadSq )
						{
							if (bStartingCringe)
							{
								GP->eventCringe(CringeDuration);
							} 
							else
							{
								GP->eventStopCringe();
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// cause all target pawns to cringe
		TArray<AGearPawn*> PawnTargets;
		{
			TArray<UObject**> ObjVars;
			GetObjectVars(ObjVars, TEXT("Target"));
			BuildGearPawnTargets(ObjVars, PawnTargets);
		}

		for (INT TargetIdx=0; TargetIdx<PawnTargets.Num(); ++TargetIdx)
		{
			AGearPawn* GP = PawnTargets(TargetIdx);
			if (bStartingCringe)	
			{
				GP->eventCringe(CringeDuration);
			} 
			else
			{
				GP->eventStopCringe();
			}
		}
	}
}

void USeqEvt_GearTouch::DoTouchActivation(AActor *InOriginator, AActor *InInstigator)
{
	if(RoadieRunFilterType != RRF_None || CoverStatusFilterType != GCF_None)
	{
		AGearPawn* GP = Cast<AGearPawn>(InInstigator);
		if(GP != NULL)
		{
			UBOOL bIsRoadieRunning = GP->IsDoingSpecialMove(SM_RoadieRun);
			if( 
				TouchedList.ContainsItem(InOriginator) ||
				(RoadieRunFilterType == RRF_IgnoreRoadieRunningGearPawns && bIsRoadieRunning) ||
				(RoadieRunFilterType == RRF_IgnoreNotRoadieRunningGearPawns && !bIsRoadieRunning) || 
				(CoverStatusFilterType == GCF_IgnoreInCoverHunkeredDown && GP->IsInCover() && !GP->IsLeaning()) ||
				(CoverStatusFilterType == GCF_IgnoreAllInCover && GP->IsInCover())
			  )
			{
				return;
			}
		}
	}

	Super::DoTouchActivation(InOriginator,InInstigator);
}

void USeqEvt_GearTouch::DoUnTouchActivation(AActor *InOriginator, AActor *InInstigator, INT TouchIdx)
{
	// if we're getting an untouch call but we're still in the touching list, check to see if we should untouch because the pawn might have changed roadie runstate
	if((RoadieRunFilterType != RRF_None || CoverStatusFilterType != GCF_None) && InInstigator && InInstigator->Touching.ContainsItem(InOriginator))
	{
		AGearPawn* GP = Cast<AGearPawn>(InInstigator);
		if(GP != NULL)
		{
			UBOOL bIsRoadieRunning = GP->IsDoingSpecialMove(SM_RoadieRun);
			if( 
				(RoadieRunFilterType == RRF_IgnoreRoadieRunningGearPawns && !bIsRoadieRunning) ||
				(RoadieRunFilterType == RRF_IgnoreNotRoadieRunningGearPawns && bIsRoadieRunning) || 
				(CoverStatusFilterType == GCF_IgnoreInCoverHunkeredDown && (!GP->IsInCover() || GP->IsLeaning())) ||
				(CoverStatusFilterType == GCF_IgnoreAllInCover && !GP->IsInCover())
				)
			{
				return;
			}
		}
	}

	Super::DoUnTouchActivation(InOriginator,InInstigator,TouchIdx);
}


void USeqAct_ToggleGUDSStreaming::Activated()
{
	UBOOL const bEnabling = InputLinks(0).bHasImpulse;

	if (!bEnabling)
	{
		// disable
		if (bFlushVarietyBanksOnDisable)
		{
			// flush and replicate flush request to client
			for( AController* C=GetWorldInfo()->ControllerList; C!=NULL; C=C->NextController)
			{
				AGearPC* PC = Cast<AGearPC>(C);
				if( PC )
				{
					PC->eventClientFlushGUDSVarietyBanks();
				}
			}
		}
	}

	// this only needs to be on server, since that initiates guds lines
	AGearGame* const GG = Cast<AGearGame>(GetWorldInfo()->Game);
	if (GG && GG->UnscriptedDialogueManager)
	{
		GG->UnscriptedDialogueManager->bDisableGUDSStreaming = !bEnabling;
	}
}


/** This will tell everyone to flish their GUDs and then turn off the GUDsManager's streaming in of new data **/
void USeqAct_CineCleanWorldPre::TurnOffAndFlushGUDs()
{
	// flush and replicate flush request to client
	for( AController* C=GetWorldInfo()->ControllerList; C!=NULL; C=C->NextController)
	{
		AGearPC* PC = Cast<AGearPC>(C);
		if( PC )
		{
			PC->eventClientFlushGUDSVarietyBanks();
		}
	}
	
	// this only needs to be on server, since that initiates guds lines
	AGearGame* const GG = Cast<AGearGame>(GetWorldInfo()->Game);
	if (GG && GG->UnscriptedDialogueManager)
	{
		GG->UnscriptedDialogueManager->bDisableGUDSStreaming = TRUE;
	}
}


/** This will turn gud streaming back.  And then the GUDs manager will start ticking and streaming in packages **/
void USeqAct_CineCleanWorldPost::RestoreGUDsStreaming()
{
	// this only needs to be on server, since that initiates guds lines
	AGearGame* const GG = Cast<AGearGame>(GetWorldInfo()->Game);
	if (GG && GG->UnscriptedDialogueManager)
	{
		GG->UnscriptedDialogueManager->bDisableGUDSStreaming = FALSE;
	}
}

