/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"

#include "GearGameSoundClasses.h"
#include "GearGameVehicleClasses.h"
#include "EngineSoundClasses.h"

#include "UnNet.h"

IMPLEMENT_CLASS(AGUDManager)
IMPLEMENT_CLASS(UGUDToC)
IMPLEMENT_CLASS(UGUDData)
IMPLEMENT_CLASS(UGUDBank)
IMPLEMENT_CLASS(UGUDTypes)
IMPLEMENT_CLASS(UGearSoundGroup)

IMPLEMENT_CLASS(ABattleStatusMonitor)
IMPLEMENT_CLASS(ARemoteSpeaker_COGAnya)
IMPLEMENT_CLASS(ARemoteSpeaker_Generic)
IMPLEMENT_CLASS(AGearRemoteSpeaker)
IMPLEMENT_CLASS(AGearSpeechManager)

/** Helper macros */
#define DEBUG_GUDS_MESSAGES		0
#if DEBUG_GUDS_MESSAGES
#define GUDSDebugf		debugf
#else
#define	GUDSDebugf		__noop
#endif

// for runtime GUDS system logging, not streaming logging
#define GUDSLog		if (bDebugGUDSLogging) debugf
#define SpeechLog	if (bDebugSpeech) debugf

/** GUD Table of Contents class. */
FString UGUDToC::ToCPackageName = TEXT("GUDToC");
FName UGUDToC::ToCObjectName = FName(TEXT("GOW2GUDToC"));

/** A bool to turn on / off GUDS.  Useful for testing **/
const static UBOOL bUseGUDS = TRUE;


/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray   array to add referenced objects to via AddReferencedObject
 */
void UGUDToC::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
    // Allow base classes to add referenced objects.
    Super::AddReferencedObjects( ObjectArray );
}

void UGUDToC::Serialize(FArchive& Ar)
{
    Super::Serialize( Ar );

	Ar << TableOfContents;
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray   array to add referenced objects to via AddReferencedObject
 */
void AGUDManager::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
    // Allow base classes to add referenced objects.
    Super::AddReferencedObjects( ObjectArray );

    // Add TMap object references.
    for (TMap<FString,FGUDBankCollection>::TIterator It(GUDBankCollectionsMap); It; ++It)
    {
		FGUDBankCollection& Collection = It.Value();
		for (INT BankIndex = 0; BankIndex < Collection.LoadedAssets.Num(); BankIndex++)
		{
			FLoadedGUDAsset& LoadedAsset = Collection.LoadedAssets(BankIndex);
			AddReferencedObject(ObjectArray, LoadedAsset.Bank);
			AddReferencedObject(ObjectArray, LoadedAsset.AnimSet);
		}

		for (INT SpeakerIndex = 0; SpeakerIndex < Collection.Speakers.Num(); SpeakerIndex++)
		{
			if (Collection.Speakers(SpeakerIndex) != NULL && Collection.Speakers(SpeakerIndex)->ActorIsPendingKill())
			{
				Collection.Speakers.Remove(SpeakerIndex);
				SpeakerIndex--;
			}
			else
			{
				AddReferencedObject(ObjectArray, Collection.Speakers(SpeakerIndex));
			}
		}
    }

	for (TMap<FString,FGUDSpeakers>::TIterator PGBIt(PendingGUDBanksMap); PGBIt; ++PGBIt)
    {
		FGUDSpeakers& Speakers = PGBIt.Value();
		for (INT Index = 0; Index < Speakers.Speakers.Num(); Index++)
		{
			if (Speakers.Speakers(Index) != NULL && Speakers.Speakers(Index)->ActorIsPendingKill())
			{
				Speakers.Speakers.Remove(Index);
				Index--;
			}
			else
			{
				AddReferencedObject(ObjectArray, Speakers.Speakers(Index));
			}
		}
	}

    for (TMap<FString,FGUDSpeakers>::TIterator PGLIt(PendingGUDLoadsMap); PGLIt; ++PGLIt)
    {
		FGUDSpeakers& Speakers = PGLIt.Value();
		for (INT Index = 0; Index < Speakers.Speakers.Num(); Index++)
		{
			if (Speakers.Speakers(Index) != NULL && Speakers.Speakers(Index)->ActorIsPendingKill())
			{
				Speakers.Speakers.Remove(Index);
				Index--;
			}
			else
			{
				AddReferencedObject(ObjectArray, Speakers.Speakers(Index));
			}
		}
	}

    for (TMap<FString,FGUDSpeakers>::TIterator PGLLIt(PendingGUDLocalizedLoadsMap); PGLLIt; ++PGLLIt)
    {
		FGUDSpeakers& Speakers = PGLLIt.Value();
		for (INT Index = 0; Index < Speakers.Speakers.Num(); Index++)
		{
			if (Speakers.Speakers(Index) != NULL && Speakers.Speakers(Index)->ActorIsPendingKill())
			{
				Speakers.Speakers.Remove(Index);
				Index--;
			}
			else
			{
				AddReferencedObject(ObjectArray, Speakers.Speakers(Index));
			}
		}
	}
}
 
void AGUDManager::Serialize( FArchive& Ar )
{
    Super::Serialize( Ar );

    // Only serialize if the archive collects object references like e.g. garbage collection or the object
    // reference collector.
    if (Ar.IsObjectReferenceCollector())
    {
		Ar << GUDBankCollectionsMap;
		Ar << PendingGUDBanksMap;
		Ar << PendingGUDLoadsMap;
		Ar << PendingGUDLocalizedLoadsMap;
	}
}

/*-----------------------------------------------------------------------------
	Async package loading callback.
-----------------------------------------------------------------------------*/
/**
 *	Callback function used in loading the TableOfContents and Bank files.
 *
 *	@param	InPackage		The package that finished async loading.
 *	@param	InGUDManager	Pointer to GUDManager object that initiated the load.
 */
static void GUDManagerAsyncLoadPackageCompletionCallback(UObject* InPackage, void* InGUDManager)
{
	AGUDManager* GUDManager = (AGUDManager*)InGUDManager;
	if (InPackage)
	{
		GUDSDebugf(TEXT("GUDManager: Completed async load of %s"), *(InPackage->GetPathName()));
		// Is it a GUD bank... Store it off in it's appropriate slot.
		if (GUDManager->ProcessGUDBankPackage(InPackage) == FALSE)
		{
			// It may be the localized file for the gud bank...
			if (GUDManager->ProcessLocalizedGUDBankPackage(InPackage) == FALSE)
			{
				// Is it the table of contents package?
				//@todo.SAS. Store this in the game package - don't async load it.
				if (GUDManager->ProcessTableOfContentsPackage(InPackage) == FALSE)
				{
					warnf(TEXT("GUDManager: Unknown package %s"), *(InPackage->GetName()));
				}
			}
		}
	}
	else
	{
		warnf(TEXT("GUDManager: NULL Package as argument to GUDManagerAsyncLoadPackageCompletionCallback") );
	}
}

// AActor interface...
void AGUDManager::Spawned()
{
	GUDSDebugf(TEXT("GUDManager SPAWNED!"));

	if( (GUseSeekFreeLoading) && bUseGUDS && !bDisableGUDSStreaming )
	{
		// Find the TableOfContents...
		TableOfContents = FindObject<UGUDToC>(ANY_PACKAGE, *(UGUDToC::ToCObjectName.ToString()));
		if (TableOfContents)
		{
			GUDSDebugf(TEXT("Dumping TableOfContents:"));
			for (TMap<FString,FGUDCollection>::TIterator ToCIt(TableOfContents->TableOfContents); ToCIt; ++ToCIt)
			{
				GUDSDebugf(TEXT("\tGUD Class: %s"), *(ToCIt.Key()));
				FGUDCollection& Collection = ToCIt.Value();
				GUDSDebugf(TEXT("\t\t%s"), *(Collection.RootGUDBank.BankName));
				for (INT DumpIndex = 0; DumpIndex < Collection.GUDBanks.Num(); DumpIndex++)
				{
					GUDSDebugf(TEXT("\t\t%s"), *(Collection.GUDBanks(DumpIndex).BankName));
				}
			}
		}
		else
		{
			warnf(TEXT("GUDManager: Failed to find TableOfContents... no GUDs will be active!"));
		}
	}
}

/** Helper class for sorting GUDBankCollections by priority. */
class CompareGearSoundFGUDBankCollectionPointer
{											
public:										
	static inline INT Compare( FGUDBankCollection const * A, FGUDBankCollection const * B	)
	{				
		// Local players get first precedence, COG get 2nd
		UBOOL bAIsCOG = FALSE;													
		UBOOL bBIsCOG = FALSE;													

		// is A a local player?
		for (INT SpeakerIdx=0; SpeakerIdx<A->Speakers.Num(); ++SpeakerIdx)
		{				
			AGearPawn* GP = Cast<AGearPawn>(A->Speakers(SpeakerIdx));
			if (GP->IsHumanControlled())													
			{													
				// A is a player, assume it's greater
				return -1;													
			}													
			else													
			{													
				INT TeamNum = GP->GetTeamNum();													
				if ( (TeamNum == 0) || (TeamNum == 255) )			/* check 255 for Meatflags too */													
				{													
					bAIsCOG = TRUE;													
				}													
			}													
		}

		// is A a local player?
		for (INT SpeakerIdx=0; SpeakerIdx<B->Speakers.Num(); ++SpeakerIdx)
		{				
			AGearPawn* GP = Cast<AGearPawn>(B->Speakers(SpeakerIdx));
			if (GP->IsHumanControlled())													
			{													
				// B is a player, assume it's greater
				return 1;													
			}													
			else													
			{													
				INT TeamNum = GP->GetTeamNum();													
				if ( (TeamNum == 0) || (TeamNum == 255) )			/* check 255 for Meatflags too */													
				{													
					bBIsCOG = TRUE;													
				}													
			}													
		}

		if (bAIsCOG && !bBIsCOG)
		{
			return -1;
		}
		else if (bBIsCOG && !bAIsCOG)
		{
			return 1;
		}

		return 0;																		
	}											
};


/** 
 *	TickSpecial	- called by AActor::Tick
 *
 *	This is where the GUDManager will process any pending GUDBank requests,
 *	as well as removing any 0 ref count collections after a certain amount of time.
 *
 *	@param	DeltaSeconds	The time since the last tick.
 */
void AGUDManager::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// maybe update streaming
	if( (GUseSeekFreeLoading) && ( bUseGUDS ) && !bDisableGUDSStreaming )
	{
		if ( (WorldInfo->TimeSeconds - StreamingUpdateIntervalSec) > LastStreamingUpdateTime )
		{
			// general concept here...
			// make sure player(s) gets them
			// then make sure all cog get them, since human voices are so much more easily notices
			//		where there is repetition
			// them make sure everyone gets one	

			// first, take stock of how much memory we're using
			UBOOL bOverMemBudget = FALSE;
			ApproxTotalUsedMemory = 0;
			INT NumVarietyBanksLoaded = 0;
			for (TMap<FString,FGUDBankCollection>::TConstIterator It(GUDBankCollectionsMap); It; ++It)
			{
				FString GUDBankName = It.Key();

				FGUDCollection* ToC = TableOfContents->TableOfContents.Find(GUDBankName);

				FGUDBankCollection const& BC = It.Value();
				for (INT AssetIdx=0; AssetIdx<BC.LoadedAssets.Num(); AssetIdx++)
				{
					if (BC.LoadedAssets(AssetIdx).Bank)
					{
						// keep track of how many are loaded, for balancing
						++NumVarietyBanksLoaded;

						// 0 is root bank, rest are variety
						if (AssetIdx == 0)
						{
							ApproxTotalUsedMemory += ToC->RootGUDBank.ApproxBankSize;
						}
						else
						{
							ApproxTotalUsedMemory += ToC->GUDBanks(AssetIdx-1).ApproxBankSize;
						}
					}
				}
			}

			// if we're over budget
			if (ApproxTotalUsedMemory > GUDSMemoryBudget)
			{
				GUDSDebugf(TEXT("GUDS over memory budget! (%d bytes)"), GUDSMemoryBudget - ApproxTotalUsedMemory);
				bOverMemBudget = TRUE;
			}

			UBOOL bUnloadedAlready = FALSE;

			// collect and sort in priority order (e.g. players first)
			TArray<FGUDBankCollection const*> SortedBankCollections;
			for (TMap<FString,FGUDBankCollection>::TConstIterator It(GUDBankCollectionsMap); It; ++It)
			{
				FGUDBankCollection const* BC = &It.Value();
				if (BC->Speakers.Num() > 0)
				{
					SortedBankCollections.AddItem(BC);
				}
			}

			UBOOL bNeedToUnloadOrphanedGUDsPackage = FALSE;

			if (SortedBankCollections.Num() > 0)
			{
				Sort<FGUDBankCollection const*, CompareGearSoundFGUDBankCollectionPointer>(&SortedBankCollections(0),SortedBankCollections.Num());

				// deal out variety banks
				INT const VarietyBanksPerCollection = NumVarietyBanksLoaded / SortedBankCollections.Num();
				INT const RemainderVarietyBanks = NumVarietyBanksLoaded % SortedBankCollections.Num();

				for (INT BCIdx=0; BCIdx<SortedBankCollections.Num(); BCIdx++)
				{
					// in the over-memory case, we want to traverse the collections in reverse-sorted order
					// so we jettison banks from lower-priority characters first
					INT ActualCollIdx = bOverMemBudget ? (SortedBankCollections.Num() - BCIdx - 1) : BCIdx;
					FGUDBankCollection const* Coll = SortedBankCollections(BCIdx);
					
					// how many banks does this guy get?
					INT TotalBanksForThisCollection = VarietyBanksPerCollection;
					if (BCIdx < RemainderVarietyBanks)
					{
						TotalBanksForThisCollection++;
					}

					// count loaded banks (some array entries can be null, so can't use Num() directly)
					INT NumLoadedBanks = 0;
					for (INT AssetIdx=0; AssetIdx<Coll->LoadedAssets.Num(); ++AssetIdx)
					{
						if (Coll->LoadedAssets(AssetIdx).Bank)
						{
							++NumLoadedBanks;
						}
					}

					// also count banks in-flight for loading
					if (Coll->Speakers.Num() > 0)
					{
						FString const GUDBankName = GetSpeakerGUDBankName(Coll->Speakers(0));
						// look through Pending Loc loads
						for (TMap<FString,FGUDSpeakers>::TConstIterator PGLLIt(PendingGUDLocalizedLoadsMap); PGLLIt; ++PGLLIt)
						{
							FGUDSpeakers const& Speakers = PGLLIt.Value();
							if (Speakers.GUDBankName == GUDBankName)
							{
								++NumLoadedBanks;
							}
						}

						// look through pending cue bank loads
						for (TMap<FString,FString>::TConstIterator It(PendingGUDBanksToPawnGUDBanksMap); It; ++It)
						{
							FString const& PendingGUDBankName = It.Value();
							if (PendingGUDBankName == GUDBankName)
							{
								++NumLoadedBanks;
							}
						}
					}

					// corrective action
					if ( !bOverMemBudget && (NumLoadedBanks <= TotalBanksForThisCollection) )
					{
						if (NumLoadedBanks < VarietyBankCap)
						{
							// load one.  this setup will effectively deal one out to whoever is under the "balanced"
							// distribution until we run out of mem.
							// also, we know we have at least one speaker to be in the sorted list.
							LoadRandomVarietyBanks(Coll->Speakers(0), 1, FALSE);
						}
					}
					else if ( !bUnloadedAlready && 
								( (bOverMemBudget && (NumLoadedBanks > TotalBanksForThisCollection))
									|| ((NumLoadedBanks + 3) > TotalBanksForThisCollection) ) )			// out of balance by > 3

					{
						// so check to see if there are any orphaned packages. 
						// if there are break out and let the unloading code below take over
						for( TMap<FString,FGUDBankCollection>::TIterator It(GUDBankCollectionsMap); It; ++It )
						{
							const FGUDBankCollection& Coll = It.Value();
							if( Coll.Speakers.Num() == 0 )
							{
								bNeedToUnloadOrphanedGUDsPackage = TRUE;
								break;
							}
						}

						// unload one
						UnloadOldestVarietyBanks(Coll->Speakers(0), 1);
						bUnloadedAlready = TRUE;
					}
				}
			}

			// now look for orphaned collections, and expire them after a certain time
			// also look for any variety swapping we might want to do
			INT VarietySwapIndex = -1;
			if ( (WorldInfo->TimeSeconds - VarietyBankSwapIntervalSec) > LastVarietyBankSwapTime )
			{
				if (GUDBankCollectionsMap.Num() > 0)
				{
					VarietySwapIndex = appRand() % GUDBankCollectionsMap.Num();
				}
			}

			FString OldestOrphanedPackageName = TEXT("");
			FLOAT LongestOrphanedPackageRefCountTime = 0.0f;
			INT CurIndex=0;
			for (TMap<FString,FGUDBankCollection>::TIterator It(GUDBankCollectionsMap); It; ++It, ++CurIndex)
			{
				FGUDBankCollection& Coll = It.Value();
				if (Coll.Speakers.Num() == 0)
				{
					if (Coll.ZeroRefCountTime == 0.f)
					{
						// mark the time we became zero-refcount
						Coll.ZeroRefCountTime = WorldInfo->TimeSeconds;
					}
					else
					{
						// if time is up, completely flush this collection entry.
						if( (WorldInfo->TimeSeconds - Coll.ZeroRefCountTime) > OrphanedRootBankUnloadDelay )
						{
							FlushGUDBank(It.Key());
						}
						// if we needed to take corrective action then find the LRU package
						else if( bNeedToUnloadOrphanedGUDsPackage == TRUE )
						{
							if( ( Coll.ZeroRefCountTime < LongestOrphanedPackageRefCountTime )
								|| ( LongestOrphanedPackageRefCountTime == 0.0f ) // to catch the first time through the loop
								)
							{
								OldestOrphanedPackageName = It.Key();
								LongestOrphanedPackageRefCountTime = Coll.ZeroRefCountTime;
							}
						}
					}
				}
				else
				{
					// mark it as not having zero refcount
					Coll.ZeroRefCountTime = 0.f;

					// variety swapping?
					if (CurIndex == VarietySwapIndex)
					{
						LoadRandomVarietyBanks(Coll.Speakers(0), 1, TRUE);
						LastVarietyBankSwapTime = WorldInfo->TimeSeconds;
					}
				}

			}

			// if need to unload and we found a package to flush then flush all this varieties
			if( ( bNeedToUnloadOrphanedGUDsPackage == TRUE ) && ( LongestOrphanedPackageRefCountTime != 0.0f ) )
			{
				UBOOL bFoundVarietyBank = FALSE;
				FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(OldestOrphanedPackageName);

				if (GUDBankCollection)
				{
					// so if we have any variety packs flush ONE of them
					// we start at 1 because 0 is the root bank
					for (INT Index = 1; Index < GUDBankCollection->LoadedAssets.Num()-1; Index++)
					{
						if( GUDBankCollection->LoadedAssets(Index).Bank != NULL )
						{
							UnloadVarietyGUDBank(OldestOrphanedPackageName, Index);
							bFoundVarietyBank = TRUE;
							break;
						}
					}
				}
				
				// we have no variety packs left and this is the oldest collection so flush it
				if( bFoundVarietyBank == FALSE )
				{
					FlushGUDBank( OldestOrphanedPackageName );
				}
			}

			LastStreamingUpdateTime = WorldInfo->TimeSeconds;
		}
	}
}

/**
 * Internal helper function to determine if a given pawn has lines defined for the given GUDActionID.
 */
UBOOL AGUDManager::SpeakerHasLineForAction(AActor& Speaker, UINT ActionID) const
{
	UGUDBank const* DefaultGUDData = NULL;
	UBOOL bIsInCombat = FALSE;

	AGearPawn* const WPSpeaker = Cast<AGearPawn>(&Speaker);
	if (WPSpeaker && (WPSpeaker->MasterGUDBankClassNames.Num() > 0) && (WPSpeaker->LoadedGUDBank >= 0))
	{
		UBOOL bPlayingMP = FALSE;
		if (WorldInfo->GRI)
		{
			if (WorldInfo->GRI->GameClass != AGearGameSP_Base::StaticClass())
			{
				bPlayingMP = TRUE;
			}
		}
		bIsInCombat = bPlayingMP || 
					  bForceInCombatGUDS ||
					  (WPSpeaker->Controller && WPSpeaker->Controller->eventIsInCombat());

		// speaker is a warpawn, use that gud data
		const FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(WPSpeaker->MasterGUDBankClassNames(WPSpeaker->LoadedGUDBank));
		if (GUDBankCollection && (GUDBankCollection->LoadedAssets.Num() > 0))
		{
			DefaultGUDData = GUDBankCollection->LoadedAssets(0).Bank;
		}
	}
	else
	{
		AGearRemoteSpeaker *RemoteSpeaker = Cast<AGearRemoteSpeaker>(&Speaker);
		if (RemoteSpeaker && (RemoteSpeaker->MasterGUDBankClassNames.Num() > 0) && (RemoteSpeaker->LoadedGUDBank >= 0))
		{
			// speaker is a remote speaker, use that gud data
			const FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(RemoteSpeaker->MasterGUDBankClassNames(RemoteSpeaker->LoadedGUDBank));
			if (GUDBankCollection && (GUDBankCollection->LoadedAssets.Num() > 0))
			{
				DefaultGUDData = GUDBankCollection->LoadedAssets(0).Bank;
			}
			bIsInCombat = FALSE;		// remote speakers are never in combat
		}
	}

	if (DefaultGUDData)
	{
		if (ActionID < (UINT)DefaultGUDData->GUDActions.Num())
		{
			FGUDAction const* Action = &DefaultGUDData->GUDActions(ActionID);

			if (Action->LineIndices.Num() > 0)
			{
				return TRUE;
			}
			else if (bIsInCombat)
			{
				// check combat lines
				return (Action->CombatOnlyLineIndices.Num() > 0) ? TRUE : FALSE;
			}
			else
			{
				// check for normal and noncombat lines
				return (Action->NonCombatOnlyLineIndices.Num() > 0) ? TRUE : FALSE;
			}
		}
	}

	return FALSE;
}

/** Helpers for GUDS to determine if 2 pawns are on same team.  Assumes team 0 (player team) if Pawn is nonexistent */
static UBOOL OnSameTeam(APawn* WP1, APawn* WP2)
{
	INT TeamNum1 = WP1 ? WP1->GetTeamNum() : 0;
	INT TeamNum2 = WP2 ? WP2->GetTeamNum() : 0;
	return (TeamNum1 == TeamNum2) ? TRUE : FALSE;
}
static UBOOL OnSameTeam(APawn* WP, AGearRemoteSpeaker* RS)
{
	INT TeamNum1 = WP ? WP->GetTeamNum() : 0;
	INT TeamNum2 = RS ? RS->TeamIndex : 0;
	return (TeamNum1 == TeamNum2) ? TRUE : FALSE;
}

static UBOOL IsValidSpeaker(AGearPawn const* GP)
{
	if ( (GP == NULL) || 
		 ((GP->Health < 0.f) && !(GP->IsDBNO())) ||
		 GP->bDeleteMe ||
		 GP->bMuteGUDS ||
		 ( (GP->WorldInfo->TimeSeconds - GP->LastGUDLinePlayedTime) <= GP->MinTimeBetweenAnyGUDS )
		)
	{
		return FALSE;
	}

	return TRUE;
}

/**
 *	Register the speaker with the GUD manager.
 *	Handles loading gud banks and mounting FaceFXAnimSets.
 *
 *	@param	Speaker		The actor (pawn or remote speaker) being registered.
 *
 *	@param	UBOOL		TRUE if the speaker was successfully registered.
 *						FALSE if the speaker was not successfully registered.
 */
UBOOL AGUDManager::RegisterSpeaker(class AActor* Speaker)
{
	if (Speaker != NULL && !Speaker->ActorIsPendingKill())
	{
		GUDSDebugf(TEXT("GUDManager:   RegisterSpeaker %s"), *(Speaker->GetName()));

		if (WorldInfo->TimeSeconds < 3.f)
		{
			// on initial load, defer these registrations
			// helps load times, since LDs sometimes spawn pawns before streaming in 
			// the rest of the level.  This way, the level start needn't wait for the root banks to come in.
			DeferredSpeakerRegistrations.AddItem(Speaker);
			SetTimer( 3.f, FALSE, FName(TEXT("DoDeferredSpeakerRegistrations")) );
		}
		else
		{
			FString GUDBankName;
			FString FaceFXAnimSetName;
			UBOOL bLoadBank = FALSE;
			INT BankIndex = 0;

			// Have to handle Pawns and RemoteSpeakers...
			// They would ideally derive from the same base class that handles GUDS... but that is unlikely at this point.
			AGearPawn* GearPawn = Cast<AGearPawn>(Speaker);
			if (GearPawn)
			{
				INT GUDBankCount = GearPawn->MasterGUDBankClassNames.Num();
				if (GUDBankCount > 0)
				{
					// Pick a GUD bank...
					if (GUDBankCount > 1)
					{
						FLOAT Ratio = 1.0f / GUDBankCount;
						FLOAT Chance = appSRand();
						BankIndex = appTrunc(Chance / Ratio);
						BankIndex = Clamp<INT>(BankIndex, 0, GUDBankCount);
					}

					GUDBankName = GearPawn->MasterGUDBankClassNames(BankIndex);
					GUDSDebugf(TEXT("\tUsing gud bank %s"), *GUDBankName);

					bLoadBank = TRUE;
				}

	// should noe need to do anything with this
				// Check for an additional FaceFX anim set...
	// 			if (GearPawn->FAS_ChatterName.Len()Load != 0)
	// 			{
	// 				FaceFXAnimSetName = GearPawn->FAS_ChatterName;
	// 				GUDSDebugf(TEXT("\tAdditional AnimSet %s"), *FaceFXAnimSetName);
	// 			}
			}

			AGearRemoteSpeaker* GearRemoteSpeaker = Cast<AGearRemoteSpeaker>(Speaker);
			if (GearRemoteSpeaker)
			{
				INT GUDBankCount = GearRemoteSpeaker->MasterGUDBankClassNames.Num();
				if (GUDBankCount > 0)
				{
					// Pick a GUD bank...
					if (GUDBankCount > 0)
					{
						FLOAT Ratio = 1.0f / GUDBankCount;
						FLOAT Chance = appSRand();
						BankIndex = appTrunc(Chance / Ratio);
						BankIndex = Clamp<INT>(BankIndex, 0, GUDBankCount);
					}

					GUDBankName = GearRemoteSpeaker->MasterGUDBankClassNames(BankIndex);
					GUDSDebugf(TEXT("\tUsing gud bank %s"), *GUDBankName);

					bLoadBank = TRUE;
				}
			}

			if (bLoadBank)
			{
				UBOOL bResult = LoadGUDBank(Speaker, GUDBankName, FaceFXAnimSetName);
				if (bResult)
				{
					if (GearPawn)
					{
						GearPawn->LoadedGUDBank = BankIndex;
					}
					else
					if (GearRemoteSpeaker)
					{
						GearRemoteSpeaker->LoadedGUDBank = BankIndex;
					}

					return TRUE;
				}
			}
		}

	}

	return FALSE;
}

/**
 *	Unregister the speaker with the GUD manager.
 *	Handles unmounting FaceFXAnimSets, and if the last referencer, 
 *	freeing both GUD banks and anim sets.
 *
 *	@param	Speaker		The actor (pawn or remote speaker) being registered.
 *
 *	@param	UBOOL		TRUE if the speaker was successfully registered.
 *						FALSE if the speaker was not successfully registered.
 */
UBOOL AGUDManager::UnregisterSpeaker(class AActor* Speaker)
{
	if (Speaker)
	{
		GUDSDebugf(TEXT("GUDManager: UnregisterSpeaker %s"), *(Speaker->GetName()));
		UnloadGUDBank(Speaker);
		DeferredSpeakerRegistrations.RemoveItem(Speaker);
		return TRUE;
	}

	return FALSE;
}

void AGUDManager::DoDeferredSpeakerRegistrations()
{
	for (INT Idx=0; Idx<DeferredSpeakerRegistrations.Num(); ++Idx)
	{
		RegisterSpeaker(DeferredSpeakerRegistrations(Idx));
	}

	// flush
	DeferredSpeakerRegistrations.Empty();
}

/**
 *	Get the GUDBankName of the given speaker.
 *
 *	@param	Speaker		The speaker to get the GUDBankName for.
 *	@return	string		The GUDBankName.
 */
FString AGUDManager::GetSpeakerGUDBankName(class AActor* Speaker)
{
	FString GUDBankName;

	AGearPawn* GearPawn = Cast<AGearPawn>(Speaker);
	if (GearPawn)
	{
		if (GearPawn->LoadedGUDBank != -1)
		{
			// Find the bank name being used...
			GUDBankName = GearPawn->MasterGUDBankClassNames(GearPawn->LoadedGUDBank);
		}
	}
	else
	{
		AGearRemoteSpeaker* GearRemoteSpeaker = Cast<AGearRemoteSpeaker>(Speaker);
		if (GearRemoteSpeaker)
		{
			if (GearRemoteSpeaker->LoadedGUDBank != -1)
			{
				// Find the bank name being used...
				GUDBankName = GearRemoteSpeaker->MasterGUDBankClassNames(GearRemoteSpeaker->LoadedGUDBank);
			}
		}
	}

	return GUDBankName;
}

/** Helper struct for sorting the loaded assets time-wise. */
struct FGUDTimeSortHelper
{
	INT	LoadedAssetIndex;
	FLOAT LoadTime;

	FGUDTimeSortHelper(INT InLoadedAssetIndex, FLOAT InLoadTime) :
		  LoadedAssetIndex(InLoadedAssetIndex) 
		, LoadTime(InLoadTime)
	{
	}
};

IMPLEMENT_COMPARE_CONSTREF(FGUDTimeSortHelper,GearSound,{ return A.LoadTime > B.LoadTime ? 1 : -1; });

/**
 *	Load in BankCount random variety banks for the given Speaker.
 *
 *	@param	Speaker				The speaker to key on.
 *	@param	BankCount			The number of banks to load.
 *	@param	bUnloadEqualNumber	If TRUE, the same number of banks loaded will be unloaded.
 *	@return bool				TRUE if successful; FALSE if not.
 */
UBOOL AGUDManager::LoadRandomVarietyBanks(class AActor* Speaker,INT BankCount,UBOOL bUnloadEqualNumber)
{
	if (TableOfContents == NULL)
	{
		return FALSE;
	}

	FString GUDBankName = GetSpeakerGUDBankName(Speaker);
	FGUDBankCollection* Collection = GUDBankCollectionsMap.Find(GUDBankName);
	FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
	if (Collection && ToCCollection)
	{
		INT PossibleCount = ToCCollection->GUDBanks.Num();
		TArray<INT> UnloadedBanks;

		for (INT VarietyIndex = 0; VarietyIndex < PossibleCount; VarietyIndex++)
		{
			// See if it is already loaded!
			if (IsVarietyBankLoaded(GUDBankName, VarietyIndex) == FALSE)
			{
				UnloadedBanks.AddUniqueItem(VarietyIndex);
			}
		}

		// Make sure we don't unload if we don't load.
		if (UnloadedBanks.Num() == 0)
		{
			GUDSDebugf(TEXT("GUDManager::LoadRandomVarietyBanks: All banks already loaded for %s?"), *GUDBankName);
			return FALSE;
		}

		// If there are less banks unloaded than the requested count, then only do that many...
		BankCount = Min<INT>(BankCount, UnloadedBanks.Num());

		// Unload banks if requested...
		if (bUnloadEqualNumber == TRUE)
		{
			UnloadOldestVarietyBanks(Speaker, BankCount);
		}

		// Load banks...
		for (INT LoadIndex = 0; LoadIndex < BankCount; LoadIndex++)
		{
			INT LoadBankIndex = appTrunc(appSRand() * UnloadedBanks.Num());
			LoadVarietyGUDBank(GUDBankName, UnloadedBanks(LoadBankIndex));
			UnloadedBanks.Remove(LoadBankIndex);
		}
	}

	return TRUE;
}

/**
 *	Unload the oldest BankCount variety banks for the given Speaker.
 *
 *	@param	Speaker			The speaker to key on.
 *	@param	BankCount		The number of banks to unload.
 *	@return bool			TRUE if successful; FALSE if not.
 */
UBOOL AGUDManager::UnloadOldestVarietyBanks(class AActor* Speaker,INT BankCount)
{
	if (TableOfContents == NULL)
	{
		return FALSE;
	}

	FString GUDBankName = GetSpeakerGUDBankName(Speaker);
	FGUDBankCollection* Collection = GUDBankCollectionsMap.Find(GUDBankName);
	FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
	if (Collection && ToCCollection)
	{
		if (Collection->LoadedAssets.Num() <= 1)
		{
			// None loaded...
			return TRUE;
		}

		TArray<FGUDTimeSortHelper> TimeOrder;
		for (INT LoadedIndex = 1; LoadedIndex < Collection->LoadedAssets.Num(); LoadedIndex++)
		{
			if (Collection->LoadedAssets(LoadedIndex).Bank)
			{
				new(TimeOrder)FGUDTimeSortHelper(LoadedIndex, Collection->LoadedAssets(LoadedIndex).LoadTime);
			}
		}
		if (TimeOrder.Num() > 1)
		{
			Sort<USE_COMPARE_CONSTREF(FGUDTimeSortHelper,GearSound)>(&(TimeOrder(0)),TimeOrder.Num());
		}

		BankCount = Min<INT>(BankCount, TimeOrder.Num());
		for (INT RemoveIndex = 0; RemoveIndex < BankCount; RemoveIndex++)
		{
			UnloadVarietyGUDBank(GUDBankName, TimeOrder(RemoveIndex).LoadedAssetIndex - 1);
		}
	}

	return TRUE;
}

/**
 *	Unload all of the variety banks for the given Speaker.
 *
 *	@param	Speaker			The speaker to key on.
 *	@return bool			TRUE if successful; FALSE if not.
 */
UBOOL AGUDManager::UnloadAllVarietyBanks(class AActor* Speaker)
{
	FString GUDBankName = GetSpeakerGUDBankName(Speaker);
	return UnloadAllVarietyBanks(GUDBankName);
}

/**
 *	Unload all of the variety banks for ALL Speakers.
 *
 *	@return bool			TRUE if successful; FALSE if not.
 */
UBOOL AGUDManager::FlushOutAllVarietyBanks()
{
    for (TMap<FString,FGUDBankCollection>::TIterator It(GUDBankCollectionsMap); It; ++It)
    {
		FString GUDBankName = It.Key();
		UnloadAllVarietyBanks(GUDBankName);
	}
	return TRUE;
}

/**
* Internal helper function to choose a random GUDAction to play.
* Returns GUDAction_None if nothing could be chosen.
*/
EGUDActionID AGUDManager::ChooseActionToPlay_Normal(const struct FGUDEvent& Event, class AActor*& Speaker) const
{
	TArray<UINT>		AvailableActions;

	// aliases for simplicity, efficiency
	FGUDEventProperties const& EventProps = EventProperties(Event.Id);
	check(EventProps.ActionIDs.Num() >= 4);
	const UINT InstigatorAction = (UINT)EventProps.ActionIDs(GUDActionMap_Instigator);
	const UINT RecipientAction = (UINT)EventProps.ActionIDs(GUDActionMap_Recipient);
	const UINT TeammateWitnessAction = (UINT)EventProps.ActionIDs(GUDActionMap_TeamWitness);
	const UINT EnemyWitnessAction = (UINT)EventProps.ActionIDs(GUDActionMap_EnemyWitness);

	// make the short list of ok-to-use (as in, not none) actions 
	if (EventProps.ForcedSpeakerRole != GUDRole_None)
	{
		switch (EventProps.ForcedSpeakerRole)
		{
		case GUDRole_Instigator:
			AvailableActions.AddItem(InstigatorAction);
			break;
		case GUDRole_Recipient:
			AvailableActions.AddItem(RecipientAction);
			break;
		case GUDRole_EnemyWitness:
			AvailableActions.AddItem(EnemyWitnessAction);
			break;
		case GUDRole_TeammateWitness:
			AvailableActions.AddItem(TeammateWitnessAction);
			break;
		}
	}
	else
	{
		if (InstigatorAction != GUDAction_None)
		{
			AvailableActions.AddItem(InstigatorAction);
		}
		if (RecipientAction != GUDAction_None)
		{
			AvailableActions.AddItem(RecipientAction);
		}
		if (TeammateWitnessAction != GUDAction_None)
		{
			AvailableActions.AddItem(TeammateWitnessAction);
		}
		if (EnemyWitnessAction != GUDAction_None)
		{
			AvailableActions.AddItem(EnemyWitnessAction);
		}
	}

	// now to find us a speaker with a valid line
	UINT ChosenAction = GUDAction_None;
	Speaker = NULL;

	while ( (AvailableActions.Num() > 0) && (Speaker == NULL) )
	{
		const INT RandIdx = appRand() % AvailableActions.Num();
		ChosenAction = AvailableActions(RandIdx);

		if (ChosenAction == InstigatorAction)
		{
			AGearPawn* const CandidateSpeaker = Event.Instigator;
			if (IsValidSpeaker(CandidateSpeaker) && SpeakerHasLineForAction(*CandidateSpeaker, ChosenAction))
			{
				// the proper line exists, we are golden
				Speaker = CandidateSpeaker;
				break;
			}
		}
		else if (ChosenAction == RecipientAction)
		{
			AGearPawn* const CandidateSpeaker = Event.Recipient;
			if (IsValidSpeaker(CandidateSpeaker) && SpeakerHasLineForAction(*CandidateSpeaker, ChosenAction))
			{
				// the proper line exists, we are golden
				Speaker = CandidateSpeaker;
				break;
			}
		}
		else 
		{
			// CAN I GET A WITNESS?!?!?!?
			// find folks close to player to keep conversation where the player can hear
			UBOOL bIncludeTeammates = (ChosenAction == TeammateWitnessAction) ? TRUE : FALSE;
			UBOOL bIncludeEnemies = (ChosenAction == EnemyWitnessAction) ? TRUE : FALSE;
			Speaker = FindWitnessSpeaker(Event, ChosenAction, bIncludeTeammates, bIncludeEnemies, TRUE);
			if (Speaker != NULL)
			{
				break;
			}
		}

		// if I made it here, this action didn't pan out, remove it
		AvailableActions.Remove(RandIdx, 1);
	}

	return (Speaker != NULL) ? (EGUDActionID)ChosenAction : GUDAction_None;
}

/** Internal function to find a witness speaker. */
AActor* AGUDManager::FindWitnessSpeaker(FGUDEvent const& Event, UINT ActionID, UBOOL bIncludeTeammates, UBOOL bIncludeEnemies, UBOOL bIncludeRemoteSpeakers) const
{
	if (GEngine->GamePlayers.Num() > 0)
	{
		TArray<AActor*>		CandidateWitnesses;
		FGUDEventProperties const& EventProps = EventProperties(Event.Id);

		// get all gearpawns nearby...
		AWorldInfo const* const WI = GWorld->GetWorldInfo();
		for (APawn *Pawn = WI->PawnList; Pawn != NULL; Pawn = Pawn->NextPawn)
		{
			AGearGameSP_Base* SPGame = Cast<AGearGameSP_Base>(WI->Game);

			AActor* DistCalcActor = Event.Instigator ? Event.Instigator : Event.Recipient;
			if (!DistCalcActor)
			{
				// fall back to player 0
				ULocalPlayer const* const Player = GEngine->GamePlayers(0);
				if (Player && Player->Actor)
				{
					DistCalcActor = Player->Actor->Pawn;
				}
			}
			FLOAT const DistSq = DistCalcActor ? (DistCalcActor->Location - Pawn->Location).SizeSquared() : FLT_MAX;

			if ( ((DistSq < EventProps.MaxWitnessDistSq) || !SPGame) &&		// no dist check in MP
				(Pawn != Event.Instigator) && (Pawn != Event.Recipient) &&
				!Pawn->bDeleteMe && (Pawn->Health > 0) && !Pawn->bHidden
				)
			{
				AGearPawn *GP = Cast<AGearPawn>(Pawn);

				if (IsValidSpeaker(GP))
				{
					UBOOL SameTeam = OnSameTeam(GP, Event.Instigator);
					if ( (SameTeam && bIncludeTeammates) || (!SameTeam && bIncludeEnemies) )
					{	
						CandidateWitnesses.AddItem(GP);
					}
				}
			}
		}

		// add any remote speakers as candidates as well
		if (bIncludeRemoteSpeakers)
		{
			AGearGameSP_Base* SPGame = Cast<AGearGameSP_Base>(WorldInfo->Game);
			if (SPGame && SPGame->Anya && SPGame->Anya->bEnabled && !SPGame->Anya->bMuteGUDS)
			{
				UBOOL SameTeam = OnSameTeam(Event.Instigator, SPGame->Anya);
				if ( (SameTeam && bIncludeTeammates) || (!SameTeam && bIncludeEnemies) )
				{	
					CandidateWitnesses.AddItem(SPGame->Anya);
				}
			}
		}

		// pick a random candidate witness, validating that it's a good pick
		while (CandidateWitnesses.Num() > 0)
		{
			// pick a candidate randomly...
			UINT RandWitnessIdx = appRand() % CandidateWitnesses.Num();
			AActor* const CandidateSpeaker = CandidateWitnesses(RandWitnessIdx);

			// if the potential speaker actually has lines defined...
			if (SpeakerHasLineForAction(*CandidateSpeaker, ActionID))
			{
				// we're golden, bail
				return CandidateSpeaker;
			}

			// if I made it here, this candidate witness didn't pan out, remove it from array
			CandidateWitnesses.Remove(RandWitnessIdx, 1);
		}
	}

	return NULL;
}

/**
 *	Load the GUD bank of the given name.
 *
 *	@param	Speaker				The actor loading the bank for.
 *	@param	GUDBankName			The name of the GUDBank class to load.
 *	@param	FaceFXAnimSetName	The FaceFXAnimSet to mount (optional).
 *
 *	@return UBOOL				TRUE if it was successful (or already loaded).
 *								FALSE if not.
 */
UBOOL AGUDManager::LoadGUDBank(AActor* Speaker, const FString& GUDBankName, const FString& FaceFXAnimSetName)
{
	// If a TOC exists, then we are using cooked content...
	// Otherwise, use the straight class-default setup.

	// See if the GUDBank is loaded
	FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(GUDBankName);
	if (GUDBankCollection == NULL)
	{
		if( (GUseSeekFreeLoading) && ( bUseGUDS ) && !bDisableGUDSStreaming )
		{
			// The TableOfContents might not have completed loading yet...
			if (TableOfContents)
			{
				// Pick a bank from the TOC.
				FGUDCollection* Collection = TableOfContents->TableOfContents.Find(GUDBankName);
				if (Collection)
				{
					FString LoadGUDBankName = Collection->RootGUDBank.BankName + TEXT("_SF");
					FString LocalizedPackageName = LoadGUDBankName + LOCALIZED_SEEKFREE_SUFFIX;
					GUDSDebugf(TEXT("LoadGUDBank: %s, %s, %s"), *GUDBankName, *LoadGUDBankName, *LocalizedPackageName);
					// See if it is in the pending lists...
					FGUDSpeakers* PendingLocalizedSpeakers = PendingGUDLocalizedLoadsMap.Find(LocalizedPackageName);
					if (PendingLocalizedSpeakers)
					{
						GUDSDebugf(TEXT("GUDManager: Found in pending localized: %s (for speaker %s"), 
							*LocalizedPackageName, *(Speaker->GetName()));
						// It was in the pending localized banks...
						PendingLocalizedSpeakers->Speakers.AddUniqueItem(Speaker);
						return TRUE;
					}

					FGUDSpeakers* PendingGUDBankSpeakers = PendingGUDBanksMap.Find(LoadGUDBankName);
					if (PendingGUDBankSpeakers)
					{
						GUDSDebugf(TEXT("GUDManager: Found in pending gud banks: %s (for speaker %s"), 
							*GUDBankName, *(Speaker->GetName()));
						// It was in the pending GUD banks...
						PendingGUDBankSpeakers->Speakers.AddUniqueItem(Speaker);
						return TRUE;
					}

					FGUDSpeakers* PendingLoadBankSpeakers = PendingGUDLoadsMap.Find(GUDBankName);
					if (PendingLoadBankSpeakers)
					{
						GUDSDebugf(TEXT("GUDManager: Found in pending load banks: %s (for speaker %s"), 
							*GUDBankName, *(Speaker->GetName()));
						// It was in the pending load banks...
						PendingLoadBankSpeakers->Speakers.AddUniqueItem(Speaker);
						return TRUE;
					}

					// MUST load the root bank first...
					if (LoadLocalizedPackage(GUDBankName, Collection->RootGUDBank.BankName, Speaker, NULL) == FALSE)
					{
						FString LoadGUDBankName = Collection->RootGUDBank.BankName + TEXT("_SF");
						LoadGUDBankPackage(GUDBankName, LoadGUDBankName, Speaker, NULL);
					}
				}
				else
				{
					warnf(TEXT("GUDS: No TOC entry for %s"), *GUDBankName);
				}
			}
			else
			{
				GUDSDebugf(TEXT("GUDS: Adding bank to pending list: %s"), *GUDBankName);
				FGUDSpeakers* CheckSpeakers = PendingGUDLoadsMap.Find(GUDBankName);
				if (CheckSpeakers)
				{
					// This could potentially happen if two speakers use the same GUDBank...
					CheckSpeakers->Speakers.AddUniqueItem(Speaker);
				}
				else
				{
					FGUDSpeakers TempSpeakers;
					appMemzero(&TempSpeakers, sizeof(FGUDSpeakers));
					FGUDSpeakers& NewSpeakers = PendingGUDLoadsMap.Set(GUDBankName, TempSpeakers);
					NewSpeakers.Speakers.AddUniqueItem(Speaker);
				}
			}

			return TRUE;
		}
		else
		{
			// Not seekfreeloading, so just load the default object directly.
			// Load the bank
			UClass* GUDSClass = LoadObject<UClass>(NULL, *(GUDBankName), NULL, LOAD_None, NULL);
			if (GUDSClass)
			{
				// Loaded object!!!!
				GUDSDebugf(TEXT("GUDManager: Loaded GUDS class data: %s"), *(GUDSClass->GetName()));
				UGUDBank* NewGUDBank = Cast<UGUDBank>(GUDSClass->GetDefaultObject());
				if (NewGUDBank)
				{
					FGUDBankCollection NewBankCollection;
					appMemzero(&NewBankCollection, sizeof(FGUDBankCollection));

					INT ItemIndex = NewBankCollection.LoadedAssets.Add();
					FLoadedGUDAsset& NewAsset = NewBankCollection.LoadedAssets(ItemIndex);
					NewAsset.Bank = NewGUDBank;
					NewAsset.LoadTime = 0.0f;
					NewAsset.AnimSet = NULL;

					GUDBankCollectionsMap.Set(GUDBankName, NewBankCollection);

					return TRUE;
				}
			}
		}
	}
	else
	{
		GUDSDebugf(TEXT("GUDManager: ***GUDBank already loaded: %s"), *GUDBankName);

		// Add the speaker to the list
		GUDBankCollection->Speakers.AddUniqueItem(Speaker);

		// add it to the speaker list of any in-flight loads, so those banks get hooked up properly as well
		//FGUDSpeakers* CheckSpeakers = PendingGUDBanksMap.Find(GUDBankName);
		//if (CheckSpeakers)
		//{
		//	CheckSpeakers->Speakers.AddUniqueItem(Speaker);
		//}
		//CheckSpeakers = PendingGUDLoadsMap.Find(GUDBankName);
		//if (CheckSpeakers)
		//{
		//	CheckSpeakers->Speakers.AddUniqueItem(Speaker);
		//}

		// Mount animsets if they are present (and it is a GearPawn)
		AGearPawn* GearPawn = Cast<AGearPawn>(Speaker);
		if (GearPawn && GearPawn->Mesh && GearPawn->Mesh->SkeletalMesh)
		{
			for (INT LoadedIndex = 0; LoadedIndex < GUDBankCollection->LoadedAssets.Num(); LoadedIndex++)
			{
				FLoadedGUDAsset& BaseAsset = GUDBankCollection->LoadedAssets(LoadedIndex);
				if (BaseAsset.AnimSet)
				{
					UFaceFXAsset* FaceFXAsset = GearPawn->Mesh->SkeletalMesh->FaceFXAsset;
					if (FaceFXAsset)
					{
						FaceFXAsset->MountFaceFXAnimSet(BaseAsset.AnimSet);
					}
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}

/**
 *	Unload the GUD bank of the given name.
 *	They are reference counted, so don't actually unload until the 
 *	ref count reaches zero!
 *
 *	@param	Speaker			The actor unloading the bank for.
 *
 *	@return UBOOL			TRUE if it was successful.
 *							FALSE if not (the bank wasn't found?).
 */
UBOOL AGUDManager::UnloadGUDBank(AActor* Speaker)
{
	FString GUDBankName = GetSpeakerGUDBankName(Speaker);
	AGearPawn* GearPawn = Cast<AGearPawn>(Speaker);

	FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(GUDBankName);
	if (GUDBankCollection)
	{
		if (GearPawn)
		{
			//@todo.SAS. For each asset in the collection, unmount if necessary...
			// The problem is that the skeleton is shared between many pawns...
		}
		// NOTE: The root bank will stay put. To get rid of it, you must explicity call RemoveGUDBank.
		if (GUDBankCollection->Speakers.Num() <= 0)
		{
			// No longer referenced... remove any variety banks.
			// don't unload here.  Let the ::Tick() hand it based on need
// 			for (INT Index = 1; Index < GUDBankCollection->LoadedAssets.Num(); Index++)
// 			{
// 				UnloadVarietyGUDBank(GUDBankName, Index - 1);
// 			}

#ifdef GUDMANAGER_REMOVE_ROOT_BANKS
			// remove the root GUD bank collection and all its references
			if( GUDBankCollection->LoadedAssets.Num() )
			{
				GUDBankCollection->LoadedAssets(0).Bank = NULL;

				FLoadedGUDAsset& Asset = GUDBankCollection->LoadedAssets(0);
				if (Asset.AnimSet)
				{
					for (INT ExistingSpeakerIndex = 0; ExistingSpeakerIndex < GUDBankCollection->Speakers.Num(); ExistingSpeakerIndex++)
					{
						AGearPawn* GearPawn = Cast<AGearPawn>(GUDBankCollection->Speakers(ExistingSpeakerIndex));
						if (GearPawn && Asset.AnimSet)
						{
							if (GearPawn->Mesh && GearPawn->Mesh->SkeletalMesh)
							{
								UFaceFXAsset* FaceFXAsset = GearPawn->Mesh->SkeletalMesh->FaceFXAsset;
								if (FaceFXAsset)
								{
									FaceFXAsset->UnmountFaceFXAnimSet(Asset.AnimSet);
								}
							}
						}
					}
					Asset.AnimSet = NULL;
				}				
			}
			GUDBankCollectionsMap.Remove(GUDBankName);
#endif
		}

		// this is the SUPER important part of this function.  It decrements the amount of speakers for this Collection 
		GUDBankCollection->Speakers.RemoveItem(Speaker);
	}

	return TRUE;
}

/** 
 *	Process the given package, treating it as the TableOfContents.
 *
 *	@param	InToCPackage	The ToC package.
 *	@return	UBOOL			TRUE if ToC was loaded; FALSE otherwise.
 */
UBOOL AGUDManager::ProcessTableOfContentsPackage(UObject* InToCPackage)
{
	FString PackageName = InToCPackage->GetName();
	if (PackageName.InStr(TEXT("GUDToC")) != -1)
	{
		// Find the table of contents
		FString GUDTocName(TEXT("GUDToC"));
		GUDTocName += TEXT(".GOW2GUDToC");
		TableOfContents = FindObject<UGUDToC>(ANY_PACKAGE, *(GUDTocName));
		if (TableOfContents)
		{
#if DEBUG_GUDS_MESSAGES
			GUDSDebugf(TEXT("Loaded GUD TableOfContents!"));
			for (TMap<FString,FGUDCollection>::TIterator ToCIt(TableOfContents->TableOfContents); ToCIt; ++ToCIt)
			{
				GUDSDebugf(TEXT("\tGUD Class: %s"), *(ToCIt.Key()));
				FGUDCollection& Collection = ToCIt.Value();
				GUDSDebugf(TEXT("\t\tROOT: %s"), *(Collection.RootGUDBank.BankName));
				for (INT DumpIndex = 0; DumpIndex < Collection.GUDBanks.Num(); DumpIndex++)
				{
					GUDSDebugf(TEXT("\t\t%s"), *(Collection.GUDBanks(DumpIndex).BankName));
				}
			}
#endif	//#if DEBUG_GUDS_MESSAGES

			// Kick off any pending banks...
			for (TMap<FString,FGUDSpeakers>::TIterator SpeakerIt(PendingGUDLoadsMap); SpeakerIt; ++SpeakerIt)
			{
				FString GUDBankName = SpeakerIt.Key();
				FGUDSpeakers& GUDSpeakers = SpeakerIt.Value();

				FGUDCollection* Collection = TableOfContents->TableOfContents.Find(GUDBankName);
				if (Collection)
				{
					FString LoadGUDBankName = Collection->RootGUDBank.BankName + TEXT("_SF");
					GUDSDebugf(TEXT("GUD Manager: Kicking off asyn load of %s"), *LoadGUDBankName);
					if (LoadLocalizedPackage(GUDBankName, Collection->RootGUDBank.BankName, NULL, &(GUDSpeakers.Speakers)) == FALSE)
					{
						LoadGUDBankPackage(GUDBankName, LoadGUDBankName, NULL, &(GUDSpeakers.Speakers));
					}
				}
				else
				{
					warnf(TEXT("GUDManager: Failed to find collection for %s"), *GUDBankName);
				}
			}
			return TRUE;
		}
		else
		{
			warnf(TEXT("GUDManager: Failed to load GUD TableOfContents - running un-cooked?"));
		}
	}

	return TRUE;
}

/** 
 *	Process the given package, treating it as a LocalizedGUDBank.
 *
 *	@param	InLocalizedGUDBank	The package to process.
 *	@return	UBOOL				TRUE if it was processed; FALSE otherwise.
 */
UBOOL AGUDManager::ProcessLocalizedGUDBankPackage(UObject* InLocalizedGUDBank)
{
	FString PackageName = InLocalizedGUDBank->GetName();
	FGUDSpeakers* LocalizedSpeakers = PendingGUDLocalizedLoadsMap.Find(*PackageName);
	if (LocalizedSpeakers)
	{
		LoadGUDBankPackage(LocalizedSpeakers->GUDBankName, LocalizedSpeakers->LoadGUDBankName, NULL, &(LocalizedSpeakers->Speakers), LocalizedSpeakers->VarietyIndex);
		PendingGUDLocalizedLoadsMap.Remove(PackageName);
		return TRUE;
	}

	return FALSE;
}

/** 
 *	Process the given package, treating it as a GUDBank.
 *
 *	@param	InGUDBank		The package to process.
 *	@return	UBOOL			TRUE if it was processed; FALSE otherwise.
 */
UBOOL AGUDManager::ProcessGUDBankPackage(UObject* InGUDBank)
{
	FString PackageName = InGUDBank->GetName();
	FString* MappedGUDBankName = PendingGUDBanksToPawnGUDBanksMap.Find(PackageName);
	if (MappedGUDBankName)
	{
		GUDSDebugf(TEXT("GUDManager: Completed load of GUDBank %s"), *PackageName);
		FGUDBankCollection* Collection = GUDBankCollectionsMap.Find(*MappedGUDBankName);
		if (Collection == NULL)
		{
			FGUDBankCollection NewBankCollection;

			appMemzero(&NewBankCollection, sizeof(FGUDBankCollection));
			FGUDBankCollection& NewBC = GUDBankCollectionsMap.Set(*MappedGUDBankName, NewBankCollection);
			Collection = &NewBC;
		}
		if (Collection)
		{
			// Grab the GUDBank from the package...
			FString ChoppedPackageName = PackageName;
			INT SFIndex = ChoppedPackageName.InStr(TEXT("_SF"));
			GUDSDebugf(TEXT("\t\tLooking at %d, %s"), SFIndex, *ChoppedPackageName);
			if (SFIndex != -1)
			{
				ChoppedPackageName = ChoppedPackageName.Left(SFIndex);
			}

			INT BankIndex = -1;
			FString* PendingGUDBankName = PendingGUDBanksToPawnGUDBanksMap.Find(PackageName);
			if (PendingGUDBankName)
			{
				FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(*PendingGUDBankName);
				if (ToCCollection)
				{
					if (ToCCollection->RootGUDBank.BankName != ChoppedPackageName)
					{
						// It's not the root...
						for (INT CheckToCIndex = 0; CheckToCIndex < ToCCollection->GUDBanks.Num(); CheckToCIndex++)
						{
							if (ToCCollection->GUDBanks(CheckToCIndex).BankName == ChoppedPackageName)
							{
								BankIndex = CheckToCIndex;
								break;
							}
						}
					}
				}
			}

			GUDSDebugf(TEXT("Found package at index %d: %s"), BankIndex, *ChoppedPackageName);

			// Assume it is a variety bank
			INT AssetIndex = BankIndex + 1;
			if (AssetIndex == 0)
			{
				if (Collection->LoadedAssets.Num() == 0)
				{
					Collection->LoadedAssets.AddZeroed();
				}
			}
			else
			{
				if (AssetIndex >= Collection->LoadedAssets.Num())
				{
					Collection->LoadedAssets.AddZeroed(AssetIndex - Collection->LoadedAssets.Num() + 1);
				}
			}
			FLoadedGUDAsset& Asset = Collection->LoadedAssets(AssetIndex);

			Asset.LoadTime = GWorld->GetTimeSeconds();

			FString GBName = ChoppedPackageName + TEXT(".GeneratedGUDBank");
			GUDSDebugf(TEXT("\t\tLooking for %s"), *GBName);
			Asset.Bank = FindObject<UGUDBank>(ANY_PACKAGE, *GBName);
			if (Asset.Bank)
			{
				// For each sound cue, verify the FaceFX group name...
				GUDSDebugf(TEXT("GUDBank: %s"), *(Asset.Bank->GetPathName()));
				if (AssetIndex == 0)
				{
					// Here, we assume that it is the root GUD bank.
					for (INT LineIndex = 0; LineIndex < Asset.Bank->GUDLines.Num(); LineIndex++)
					{
						FGUDLine& GUDLine = Asset.Bank->GUDLines(LineIndex);

#if DEBUG_GUDS_MESSAGES
						if (GUDLine.Audio && GUDLine.Audio->FaceFXAnimName != TEXT(""))
						{
							GUDSDebugf(TEXT("\tSoundCue............%s"), *(GUDLine.Audio->GetPathName()));
							GUDSDebugf(TEXT("\t\tFFAnimGroup.......%s"), *(GUDLine.Audio->FaceFXGroupName));
							GUDSDebugf(TEXT("\t\tFFAnimName........%s"), *(GUDLine.Audio->FaceFXAnimName));
						}
#endif	//#if DEBUG_GUDS_MESSAGES
					}
				}
				else
				{
					FLoadedGUDAsset& RootAsset = Collection->LoadedAssets(0);
					check(RootAsset.Bank);
					// It's not the root, so patch up the GUDLines and Actions...
					check(RootAsset.Bank->GUDLines.Num() == Asset.Bank->GUDLines.Num());
					for (INT LineIndex = 0; LineIndex < Asset.Bank->GUDLines.Num(); LineIndex++)
					{
						FGUDLine& SourceLine = Asset.Bank->GUDLines(LineIndex);
						if (SourceLine.Audio != NULL)
						{
							FGUDLine& RootLine = RootAsset.Bank->GUDLines(LineIndex);
							if (RootLine.Audio != NULL)
							{
								check(SourceLine.Audio->GetName() == RootLine.Audio->GetName());
							}
							else
							{
								RootLine = SourceLine;
							}
						}
					}

					check(RootAsset.Bank->GUDActions.Num() == Asset.Bank->GUDActions.Num());
					for (INT ActionIndex = 0; ActionIndex < Asset.Bank->GUDActions.Num(); ActionIndex++)
					{
						FGUDAction& SourceAction = Asset.Bank->GUDActions(ActionIndex);
						FGUDAction& RootAction = RootAsset.Bank->GUDActions(ActionIndex);
						if (SourceAction.CombatOnlyLineIndices.Num() > 0)
						{
							for (INT CLIndex = 0; CLIndex < SourceAction.CombatOnlyLineIndices.Num(); CLIndex++)
							{
								RootAction.CombatOnlyLineIndices.AddUniqueItem(SourceAction.CombatOnlyLineIndices(CLIndex));
							}
						}
						
						if (SourceAction.LineIndices.Num() > 0)
						{
							for (INT LIndex = 0; LIndex < SourceAction.LineIndices.Num(); LIndex++)
							{
								RootAction.LineIndices.AddUniqueItem(SourceAction.LineIndices(LIndex));
							}
						}

						if (SourceAction.NonCombatOnlyLineIndices.Num() > 0)
						{
							for (INT NCLIndex = 0; NCLIndex < SourceAction.NonCombatOnlyLineIndices.Num(); NCLIndex++)
							{
								RootAction.NonCombatOnlyLineIndices.AddUniqueItem(SourceAction.NonCombatOnlyLineIndices(NCLIndex));
							}
						}
					}
				}
			}

			FString GFFASName = ChoppedPackageName + TEXT(".GeneratedFaceFXAnimSet");
			Asset.AnimSet = FindObject<UFaceFXAnimSet>(ANY_PACKAGE, *GFFASName);
			FGUDSpeakers* Speakers = PendingGUDBanksMap.Find(*MappedGUDBankName);
			if (Speakers)
			{
				// Hook them up...
				GUDSDebugf(TEXT("GUDManager: Completed load of GUDBank %s"), *(*MappedGUDBankName));

				for (INT SpeakerIndex = 0; SpeakerIndex < Speakers->Speakers.Num(); SpeakerIndex++)
				{
					AGearPawn* GearPawn = Cast<AGearPawn>(Speakers->Speakers(SpeakerIndex));
					AGearRemoteSpeaker* RemoteSpeaker = Cast<AGearRemoteSpeaker>(Speakers->Speakers(SpeakerIndex));

					if (GearPawn || RemoteSpeaker)
					{
						Collection->Speakers.AddUniqueItem(Speakers->Speakers(SpeakerIndex));
					}
				}

				// Remove it from the pending map.
				PendingGUDBanksMap.Remove(*MappedGUDBankName);
			}
			else
			{
				if (Collection->Speakers.Num() == 0)
				{
					warnf(TEXT("GUDManager: Completed load of unknown GUDBank %s"), *(*MappedGUDBankName));
				}
			}

			// Hook up speakers...
			if (Asset.AnimSet != NULL)
			{
				for (INT ExistingSpeakerIndex = 0; ExistingSpeakerIndex < Collection->Speakers.Num(); ExistingSpeakerIndex++)
				{
					AGearPawn* GearPawn = Cast<AGearPawn>(Collection->Speakers(ExistingSpeakerIndex));
					if (GearPawn)
					{
						if (GearPawn->Mesh && GearPawn->Mesh->SkeletalMesh)
						{
							UFaceFXAsset* FaceFXAsset = GearPawn->Mesh->SkeletalMesh->FaceFXAsset;
							if (FaceFXAsset)
							{
								FaceFXAsset->MountFaceFXAnimSet(Asset.AnimSet);
							}
						}
					}
				}
			}
		}

		// Remove it from the pending map
		PendingGUDBanksToPawnGUDBanksMap.Remove(PackageName);

		return TRUE;
	}

	return FALSE;
}

INT AGUDManager::GetAvailableGUDSMemory() const
{
	INT UsedMem = ApproxTotalUsedMemory;

	// count up sizes of pending loads too
	for (TMap<FString,FGUDSpeakers>::TConstIterator PGLLIt(PendingGUDLocalizedLoadsMap); PGLLIt; ++PGLLIt)
	{
		FGUDSpeakers const& Speakers = PGLLIt.Value();
		UsedMem += Speakers.ApproxLoadBankSize;
	}

	return Max<INT>((GUDSMemoryBudget - UsedMem), 0);
}

/**
 *	Kick off a load of the localized version of the given GUDBank package...
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *	@param	LoadGUDBankName		The load (package) name of the GUDBank.
 *	@param	Speaker				The speaker to register for it.
 *	@param	Speakers			The array of speakers to register for it.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if it failed (ie, there is no loc version).
 */
UBOOL AGUDManager::LoadLocalizedPackage(const FString& GUDBankName, const FString& GUDBankLoadName, AActor* Speaker, TArrayNoInit<AActor*>* Speakers, INT VarietyIndex)
{
	// This is only used for seek free loading!
	if( (GUseSeekFreeLoading == FALSE) && bUseGUDS && !bDisableGUDSStreaming )
	{
		return TRUE;
	}

	// Form the name and check if it exists
	FString LoadGUDBankName = GUDBankLoadName + TEXT("_SF");
	FString LocalizedPackageName = LoadGUDBankName + LOCALIZED_SEEKFREE_SUFFIX;
	FString LocalizedFilename;
	if (GPackageFileCache->FindPackageFile(*LocalizedPackageName, NULL, LocalizedFilename))
	{	
		// see if we have room for it
		UBOOL bOkToLoad = FALSE;
		INT ApproxBankSize = 0;
		{
			FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
			if (ToCCollection)
			{
				if (VarietyIndex >= 0)
				{
					ApproxBankSize = ToCCollection->GUDBanks(VarietyIndex).ApproxBankSize;
				}
				else
				{
					ApproxBankSize = ToCCollection->RootGUDBank.ApproxBankSize;
				}
			}

			if ( (VarietyIndex < 0) || 
				( (ApproxBankSize > 0) && (ApproxBankSize < GetAvailableGUDSMemory()) ) )
			{
				bOkToLoad = TRUE;
			}
		}

		if(bOkToLoad)
		{
			// See if it is already pending...
			GUDSDebugf(TEXT("GUD Manager: Kicking off asyn load of %s"), *LocalizedPackageName);
			FGUDSpeakers* LocalizedSpeakers = PendingGUDLocalizedLoadsMap.Find(LocalizedPackageName);
			if (LocalizedSpeakers == NULL)
			{
				// It is not, so kick off a load and insert it into the pending map.
				UObject::LoadPackageAsync(LocalizedPackageName, 
					GUDManagerAsyncLoadPackageCompletionCallback, (void*)this);

				FGUDSpeakers TempSpeakers;
				appMemzero(&TempSpeakers, sizeof(FGUDSpeakers));
				FGUDSpeakers& NewSpeakers = PendingGUDLocalizedLoadsMap.Set(LocalizedPackageName, TempSpeakers);
				LocalizedSpeakers = &NewSpeakers;
				LocalizedSpeakers->GUDBankName = GUDBankName;
				LocalizedSpeakers->LoadGUDBankName = LoadGUDBankName;
				LocalizedSpeakers->ApproxLoadBankSize = ApproxBankSize;
				LocalizedSpeakers->VarietyIndex = VarietyIndex;
			}

			if (Speaker)
			{
				LocalizedSpeakers->Speakers.AddUniqueItem(Speaker);
			}
			else if (Speakers)
			{
				for (INT SpeakerIndex = 0; SpeakerIndex < Speakers->Num(); SpeakerIndex++)
				{
					LocalizedSpeakers->Speakers.AddUniqueItem((*Speakers)(SpeakerIndex));
				}
			}
		}

		// returning false means "there is no loc version", so we return true here, even though
		// nothing may have been loaded
		return TRUE;
	}

	// This will cause the loader to fall through to the non-localized case...
	return FALSE;
}

/**
 *	Kick off a load of the given GUDBank package...
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *	@param	LoadGUDBankName		The load (package) name of the GUDBank.
 *	@param	Speaker				The speaker to register for it.
 *	@param	Speakers			The array of speakers to register for it.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
 */
UBOOL AGUDManager::LoadGUDBankPackage(const FString& GUDBankName, const FString& LoadGUDBankName, AActor* Speaker, TArrayNoInit<AActor*>* Speakers, INT VarietyIndex)
{
	// This is only used for seek free loading!
	if( (GUseSeekFreeLoading == FALSE) && bUseGUDS && !bDisableGUDSStreaming )
	{
		return TRUE;
	}

	GUDSDebugf(TEXT("GUD Manager: Kicking off async load of %s"), *LoadGUDBankName);
	FString LocalizedFilename;
	if (GPackageFileCache->FindPackageFile(*LoadGUDBankName, NULL, LocalizedFilename))
	{
		UBOOL bOkToLoad = FALSE;
		FGUDSpeakers* CheckSpeakers = PendingGUDBanksMap.Find(GUDBankName);
		if (CheckSpeakers == NULL)
		{
			// see if we have room for it
			INT ApproxBankSize = 0;
			{
				FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
				if (ToCCollection)
				{
					if (VarietyIndex >= 0)
					{
						ApproxBankSize = ToCCollection->GUDBanks(VarietyIndex).ApproxBankSize;
					}
					else
					{
						ApproxBankSize = ToCCollection->RootGUDBank.ApproxBankSize;
					}
				}

				if ( (VarietyIndex < 0) || 
					( (ApproxBankSize > 0) && (ApproxBankSize < GetAvailableGUDSMemory()) ) )
				{
					bOkToLoad = TRUE;
				}
			}

			if (bOkToLoad)
			{
				// It's not already loading, so kick it off
				UObject::LoadPackageAsync(LoadGUDBankName, 
					GUDManagerAsyncLoadPackageCompletionCallback, (void*)this);
				PendingGUDBanksToPawnGUDBanksMap.Set(LoadGUDBankName, GUDBankName);

				CheckSpeakers = PendingGUDBanksMap.Find(GUDBankName);
				if (CheckSpeakers == NULL)
				{
					// Fill in the information for it
					FGUDSpeakers TempSpeakers;
					appMemzero(&TempSpeakers, sizeof(FGUDSpeakers));
					FGUDSpeakers& NewSpeakers = PendingGUDBanksMap.Set(GUDBankName, TempSpeakers);
					CheckSpeakers = &NewSpeakers;
				}

				// Inform clients. The netcode will automatically tell the client to load the package and handle synchronization of the objects inside,
				// but we need to tell the client how long to keep a reference to those objects.
				for (AController* C = WorldInfo->ControllerList; C != NULL; C = C->NextController)
				{
					AGearPC* PC = Cast<AGearPC>(C);
					if (PC != NULL && Cast<UNetConnection>(PC->Player) != NULL && Cast<UChildConnection>(PC->Player) == NULL)
					{
						PC->eventClientLoadGUDBank(FName(*LoadGUDBankName));
					}
				}
			}
		}

		if (bOkToLoad)
		{
			if (Speaker)
			{
				CheckSpeakers->Speakers.AddUniqueItem(Speaker);
			}
			else if (Speakers)
			{
				for (INT SpeakerIndex = 0; SpeakerIndex < Speakers->Num(); SpeakerIndex++)
				{
					CheckSpeakers->Speakers.AddUniqueItem((*Speakers)(SpeakerIndex));
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}

/**
 *	Completely remove the given GUDBank.
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if not.
 */
UBOOL AGUDManager::FlushGUDBank(const FString& GUDBankName)
{
	GUDSDebugf(TEXT("Flushing bank %s"), *GUDBankName);
	FGUDBankCollection* Collection = GUDBankCollectionsMap.Find(GUDBankName);
	if (Collection)
	{
		for (INT VarietyIndex = 1; VarietyIndex < Collection->LoadedAssets.Num(); VarietyIndex++)
		{
			UnloadVarietyGUDBank(GUDBankName, VarietyIndex - 1);
		}

		// Unmount the facefx, if any
		FLoadedGUDAsset& RootAsset = Collection->LoadedAssets(0);
		if (RootAsset.AnimSet)
		{
			for (INT ExistingSpeakerIndex = 0; ExistingSpeakerIndex < Collection->Speakers.Num(); ExistingSpeakerIndex++)
			{
				AGearPawn* GearPawn = Cast<AGearPawn>(Collection->Speakers(ExistingSpeakerIndex));
				if (GearPawn)
				{
					if (GearPawn->Mesh && GearPawn->Mesh->SkeletalMesh)
					{
						UFaceFXAsset* FaceFXAsset = GearPawn->Mesh->SkeletalMesh->FaceFXAsset;
						if (FaceFXAsset)
						{
							FaceFXAsset->UnmountFaceFXAnimSet(RootAsset.AnimSet);
						}
					}
				}
			}
		}

		// kill any in-flight bank loads.  
		// look through Pending Loc loads
		for (TMap<FString,FGUDSpeakers>::TIterator PGLLIt(PendingGUDLocalizedLoadsMap); PGLLIt; ++PGLLIt)
		{
			FGUDSpeakers const& Speakers = PGLLIt.Value();
			FString const& PendingGUDBankName = Speakers.GUDBankName;
			if (PendingGUDBankName == GUDBankName)
			{
				PGLLIt.RemoveCurrent();
			}
		}
		// look through pending cue bank loads
		for (TMap<FString,FString>::TIterator It(PendingGUDBanksToPawnGUDBanksMap); It; ++It)
		{
			FString const& PendingGUDBankName = It.Value();
			if (PendingGUDBankName == GUDBankName)
			{
				It.RemoveCurrent();
			}
		}
		for (TMap<FString,FGUDSpeakers>::TIterator It(PendingGUDLoadsMap); It; ++It)
		{
			FString const& PendingGUDBankName = It.Key();
			if (PendingGUDBankName == GUDBankName)
			{
				It.RemoveCurrent();
			}
		}
		for (TMap<FString,FGUDSpeakers>::TIterator It(PendingGUDBanksMap); It; ++It)
		{
			FString const& PendingGUDBankName = It.Key();
			if (PendingGUDBankName == GUDBankName)
			{
				It.RemoveCurrent();
			}
		}

		// Remove it from the loaded assets array
		Collection->LoadedAssets.Empty();

		// Remove it from the collections array
		GUDBankCollectionsMap.Remove(GUDBankName);

		// tell clients to discard the GUD bank references as well
		for (AController* C = WorldInfo->ControllerList; C != NULL; C = C->NextController)
		{
			AGearPC* PC = Cast<AGearPC>(C);
			if (PC != NULL && Cast<UNetConnection>(PC->Player) != NULL && Cast<UChildConnection>(PC->Player) == NULL)
			{
				PC->eventClientUnloadGUDBank(FName(*GUDBankName));
			}
		}
	}

	return TRUE;
}

/**
 *	See if the given variety bank is loaded or in the process of being loaded.
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *	@param	VarietyIndex		The index of the bank to check.
 *
 *	@return	UBOOL				TRUE if loaded; FALSE if not.
 */
UBOOL AGUDManager::IsVarietyBankLoaded(const FString& GUDBankName, INT VarietyIndex)
{
	//GUDSDebugf(TEXT("IsVarietyBankLoaded %s"), *GUDBankName);
	FGUDBankCollection* const Collection = GUDBankCollectionsMap.Find(GUDBankName);
	if (Collection)
	{
		INT const TrueIndex = VarietyIndex + 1;
		if (Collection->LoadedAssets.Num() > TrueIndex)
		{
			if (Collection->LoadedAssets(TrueIndex).Bank != NULL)
			{
				return TRUE;
			}
		}
	}

	FString LocalName = GUDBankName;

	FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
	// loading cue bank?
	if (ToCCollection == NULL)
	{
		return FALSE;
	}
	LocalName = ToCCollection->RootGUDBank.BankName;

	FString const LoadGUDBankName = LocalName + FString::Printf(TEXT("G%d%s"), VarietyIndex+1, STANDALONE_SEEKFREE_SUFFIX);
	if (PendingGUDBanksToPawnGUDBanksMap.Find(LoadGUDBankName) != NULL)
	{
		//GUDSDebugf(TEXT("... Found in PendingGUDBanksToPawnGUDBanksMap"));
		return TRUE;
	}

	// loading loc bank?
	FString LocalizedPackageName = LoadGUDBankName + LOCALIZED_SEEKFREE_SUFFIX;
	if (PendingGUDLocalizedLoadsMap.Find(LocalizedPackageName) != NULL)
	{
		//GUDSDebugf(TEXT("... Found in PendingGUDLocalizedLoadsMap"));
		return TRUE;
	}

	return FALSE;
}

/**
 *	Load the given variety bank for the GUDBankName collection.
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *	@param	VarietyIndex		The index of the bank to load. If -1, pick a random one.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
 */
UBOOL AGUDManager::LoadVarietyGUDBank(const FString& GUDBankName, INT VarietyIndex)
{
	if (TableOfContents == NULL)
	{
		return FALSE;
	}

	FGUDBankCollection* GBCollection = GUDBankCollectionsMap.Find(GUDBankName);
	if (GBCollection)
	{
		GUDSDebugf(TEXT("GUDManager::LoadVarietyGUDBank: Loading variety bank %d for %s"), VarietyIndex, *GUDBankName);

		// See if it is already loaded!
		if (IsVarietyBankLoaded(GUDBankName, VarietyIndex) == TRUE)
		{
			GUDSDebugf(TEXT("GUDManager::LoadVarietyGUDBank: Already loaded - %d, %s"), VarietyIndex, *GUDBankName);
			return TRUE;
		}

		// Find the TOC entry for it.
		FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
		if (ToCCollection == NULL)
		{
			warnf(TEXT("GUDManager::LoadVarietyGUDBank: Failed to find ToC entry for %s"), *GUDBankName);
			return FALSE;
		}

		// Is it in the collection?
		if (VarietyIndex >= ToCCollection->GUDBanks.Num())
		{
			warnf(TEXT("GUDManager::LoadVarietyGUDBank: Request index out of range (%d vs %s"), 
				VarietyIndex, ToCCollection->GUDBanks.Num());
			return FALSE;
		}

		if (LoadLocalizedPackage(GUDBankName, ToCCollection->GUDBanks(VarietyIndex).BankName, NULL, NULL, VarietyIndex) == FALSE)
		{
			FString LoadGUDBankName = ToCCollection->GUDBanks(VarietyIndex).BankName + TEXT("_SF");
			LoadGUDBankPackage(GUDBankName, LoadGUDBankName, NULL, NULL, VarietyIndex);
		}
	}

	return TRUE;
}


/**
 *	Unload the given variety bank for the GUDBankName collection.
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *	@param	VarietyIndex		The index of the bank to load.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
 */
UBOOL AGUDManager::UnloadVarietyGUDBank(const FString& GUDBankName, INT VarietyIndex)
{
	GUDSDebugf(TEXT("GUDManager::UnloadVarietyGUDBank: Unloading variety bank %d for %s"), VarietyIndex, *GUDBankName);

	FGUDBankCollection* Collection = GUDBankCollectionsMap.Find(GUDBankName);
	if (Collection)
	{
		INT TrueIndex = VarietyIndex + 1;
		if (Collection->LoadedAssets.Num() <= TrueIndex)
		{
			warnf(TEXT("GUDManager::UnloadVarietyBank: Bank at index %d is not loaded."), VarietyIndex);
			return FALSE;
		}

		// It's not the root, so clean up the GUDLines and Actions...
		FLoadedGUDAsset& RootAsset = Collection->LoadedAssets(0);
		FLoadedGUDAsset& Asset = Collection->LoadedAssets(TrueIndex);
		if (Asset.Bank)
		{
			// Remove all the lines from the root.
			for (INT LineIndex = 0; LineIndex < Asset.Bank->GUDLines.Num(); LineIndex++)
			{
				FGUDLine& SourceLine = Asset.Bank->GUDLines(LineIndex);
				if (SourceLine.Audio != NULL)
				{
					FGUDLine& RootLine = RootAsset.Bank->GUDLines(LineIndex);
					if (RootLine.Audio != NULL)
					{
						// Clear it out of the RootAsset
						RootAsset.Bank->GUDLines(LineIndex).Audio = NULL;
						RootAsset.Bank->GUDLines(LineIndex).Addressee = 0;
						RootAsset.Bank->GUDLines(LineIndex).ReferringTo = 0;
						RootAsset.Bank->GUDLines(LineIndex).ResponseEventIDs.Empty();
						//RootAsset.Bank->GUDLines(LineIndex).Text.Empty();
					}
				}
			}

			// Remove all the indices from the root action.
			for (INT ActionIndex = 0; ActionIndex < Asset.Bank->GUDActions.Num(); ActionIndex++)
			{
				FGUDAction& SourceAction = Asset.Bank->GUDActions(ActionIndex);
				FGUDAction& RootAction = RootAsset.Bank->GUDActions(ActionIndex);
				if (SourceAction.CombatOnlyLineIndices.Num() > 0)
				{
					for (INT CLIndex = 0; CLIndex < SourceAction.CombatOnlyLineIndices.Num(); CLIndex++)
					{
						RootAction.CombatOnlyLineIndices.RemoveItem(SourceAction.CombatOnlyLineIndices(CLIndex));
					}
				}
				
				if (SourceAction.LineIndices.Num() > 0)
				{
					for (INT LIndex = 0; LIndex < SourceAction.LineIndices.Num(); LIndex++)
					{
						RootAction.LineIndices.RemoveItem(SourceAction.LineIndices(LIndex));
					}
				}

				if (SourceAction.NonCombatOnlyLineIndices.Num() > 0)
				{
					for (INT NCLIndex = 0; NCLIndex < SourceAction.NonCombatOnlyLineIndices.Num(); NCLIndex++)
					{
						RootAction.NonCombatOnlyLineIndices.RemoveItem(SourceAction.NonCombatOnlyLineIndices(NCLIndex));
					}
				}
			}

			// tell clients to discard the GUD bank references as well
			FString VarietyBankName = Asset.Bank->GetOutermost()->GetName();
			for (AController* C = WorldInfo->ControllerList; C != NULL; C = C->NextController)
			{
				AGearPC* PC = Cast<AGearPC>(C);
				if (PC != NULL && Cast<UNetConnection>(PC->Player) != NULL && Cast<UChildConnection>(PC->Player) == NULL)
				{
					PC->eventClientUnloadGUDBank(FName(*VarietyBankName));
				}
			}

			// Only decrement if we are actually removing a bank...
			Collection->LoadedAssets(TrueIndex).Bank = NULL;
		}

		// Unmount the facefx, if any
		if (Asset.AnimSet)
		{
			for (INT ExistingSpeakerIndex = 0; ExistingSpeakerIndex < Collection->Speakers.Num(); ExistingSpeakerIndex++)
			{
				AGearPawn* GearPawn = Cast<AGearPawn>(Collection->Speakers(ExistingSpeakerIndex));
				if (GearPawn && Asset.AnimSet)
				{
					if (GearPawn->Mesh && GearPawn->Mesh->SkeletalMesh)
					{
						UFaceFXAsset* FaceFXAsset = GearPawn->Mesh->SkeletalMesh->FaceFXAsset;
						if (FaceFXAsset)
						{
							FaceFXAsset->UnmountFaceFXAnimSet(Asset.AnimSet);
						}
					}
				}
			}

			Collection->LoadedAssets(TrueIndex).AnimSet = NULL;
		}

		Collection->LoadedAssets(TrueIndex).LoadTime = 0.0f;
	}

	return TRUE;
}

/**
 *	Load all the variety banks for the given source GUDBank.
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
 */
UBOOL AGUDManager::LoadAllVarietyBanks(const FString& GUDBankName)
{
	if (GUseSeekFreeLoading == FALSE) 
	{
		return TRUE;
	}

	if (TableOfContents == NULL)
	{
		return FALSE;
	}

	FGUDBankCollection* GBCollection = GUDBankCollectionsMap.Find(GUDBankName);
	if (GBCollection)
	{
		GUDSDebugf(TEXT("GUDManager::LoadAllVarietyBanks: Loading all variety banks for %s"), *GUDBankName);

		// Find the TOC entry for it.
		FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
		if (ToCCollection == NULL)
		{
			warnf(TEXT("GUDManager::LoadAllVarietyBanks: Failed to find ToC entry for %s"), *GUDBankName);
			return FALSE;
		}
		for (INT BankIndex = 0; BankIndex < ToCCollection->GUDBanks.Num(); BankIndex++)
		{
			if (LoadVarietyGUDBank(GUDBankName, BankIndex) == FALSE)
			{
				warnf(TEXT("GUDManager::LoadAllVarietyBanks: Failed to load variety bank %d for %s"), BankIndex, *GUDBankName);
			}
		}
	}
	else
	{
		warnf(TEXT("GUDManager::LoadAllVarietyBanks: Couldn't find collection for %s"), *GUDBankName);
		return FALSE;
	}

	return TRUE;
}

/**
 *	Unload all the variety banks for the given source GUDBank.
 *
 *	@param	GUDBankName			The name of the GUDBank.
 *
 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
 */
UBOOL AGUDManager::UnloadAllVarietyBanks(const FString& GUDBankName)
{
	if (GUseSeekFreeLoading == FALSE)
	{
		return TRUE;
	}

	if (TableOfContents == NULL)
	{
		return FALSE;
	}

	FGUDBankCollection* GBCollection = GUDBankCollectionsMap.Find(GUDBankName);
	if (GBCollection)
	{
		GUDSDebugf(TEXT("GUDManager::UnloadAllVarietyBanks: Unloading all variety banks for %s"), *GUDBankName);

		// Find the TOC entry for it.
		FGUDCollection* ToCCollection = TableOfContents->TableOfContents.Find(GUDBankName);
		if (ToCCollection == NULL)
		{
			warnf(TEXT("GUDManager::UnloadAllVarietyBanks: Failed to find ToC entry for %s"), *GUDBankName);
			return FALSE;
		}
		for (INT BankIndex = 0; BankIndex < ToCCollection->GUDBanks.Num(); BankIndex++)
		{
			if (UnloadVarietyGUDBank(GUDBankName, BankIndex) == FALSE)
			{
				warnf(TEXT("GUDManager::UnloadAllVarietyBanks: Failed to unload variety bank %d for %s"), BankIndex, *GUDBankName);
			}
		}
	}
	else
	{
		warnf(TEXT("GUDManager::UnloadAllVarietyBanks: Couldn't find collection for %s"), *GUDBankName);
		return FALSE;
	}

	return TRUE;
}

void AGUDManager::RunTest(const FString& InPawnName, TArray<FString>& Tokens)
{
	GUDSDebugf(TEXT("RunTest on pawn %s"), *InPawnName);

	UGUDBank const* DefaultGUDData = NULL;
	UBOOL bIsInCombat = FALSE;

	INT DummyIndex;

	// Play action test...
	FString ActionTokenCheck(TEXT("Action"));

	if (Tokens.FindItem(ActionTokenCheck, DummyIndex) == TRUE)
	{
		check(Tokens.Num() > 1);
		INT ActionID = appAtoi(*(Tokens(1)));

		for (TObjectIterator<AGearPawn> It; It; ++It)
		{
			AGearPawn* GearPawn = *It;
			if (GearPawn->GetName().InStr(InPawnName) != -1)
			{
				if (SpeakerHasLineForAction(*GearPawn, ActionID))
				{
					// Play a GUDS action for this pawn
					if ((GearPawn->MasterGUDBankClassNames.Num() > 0) && (GearPawn->LoadedGUDBank >= 0))
					{
						GUDSDebugf(TEXT("GUDManager: Testing speaker: %s"), *(GearPawn->GetName()));
						UBOOL bPlayingMP = FALSE;
						if (WorldInfo->GRI)
						{
							if (WorldInfo->GRI->GameClass != AGearGameSP_Base::StaticClass())
							{
								bPlayingMP = TRUE;
							}
						}
						bIsInCombat = bPlayingMP || 
							bForceInCombatGUDS ||
							(GearPawn->Controller && GearPawn->Controller->eventIsInCombat());

						// speaker is a warpawn, use that gud data
						const FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(GearPawn->MasterGUDBankClassNames(GearPawn->LoadedGUDBank));
						if (GUDBankCollection && (GUDBankCollection->LoadedAssets.Num() > 0))
						{
							DefaultGUDData = GUDBankCollection->LoadedAssets(0).Bank;
						}

						USoundCue* AudioLine = NULL;
						if (DefaultGUDData && (ActionID < DefaultGUDData->GUDActions.Num()))
						{
							FGUDAction const* Action = &DefaultGUDData->GUDActions(ActionID);
							const TArray<INT> *LineIndices = NULL;
							TArray<INT> TempIndices;
							// use the combat lines if available and in combat
							if (bIsInCombat)
							{
								if (Action->CombatOnlyLineIndices.Num() > 0)
								{
									TempIndices = Action->LineIndices;
									TempIndices += Action->CombatOnlyLineIndices;
									LineIndices = &TempIndices;
								}
								else
								{
									LineIndices = &Action->LineIndices;
								}
							}
							else
							{
								if (Action->NonCombatOnlyLineIndices.Num() > 0)
								{
									TempIndices = Action->LineIndices;
									TempIndices += Action->NonCombatOnlyLineIndices;
									LineIndices = &TempIndices;
								}
								else
								{
									LineIndices = &Action->LineIndices;
								}
							}

							const INT NumLines = LineIndices->Num();
							if (NumLines > 0)
							{
								// choose one of the lines randomly
								const INT LineIdx = (appRand() % NumLines);
								const FGUDLine* outLine = &(DefaultGUDData->GUDLines((*LineIndices)(LineIdx)));

								if (outLine)
								{
									AudioLine = outLine->Audio;
								}
							}
						}

						if ((AudioLine != NULL) && (GearPawn->Mesh != NULL))
						{
							GearPawn->Mesh->PlayFaceFXAnim(AudioLine->FaceFXAnimSetRef, AudioLine->FaceFXAnimName, AudioLine->FaceFXGroupName,AudioLine);
						}
					}
				}
				else
				{
					warnf(TEXT("GUDManager: Action %4d, Speaker had no line: %s"),
						ActionID, *(GearPawn->GetName()));
				}
			}
		}
	}

	UBOOL bLoadTest = FALSE;
	UBOOL bUnloadTest = FALSE;
	// LoadAll test
	FString LoadAllTokenCheck(TEXT("LoadAll"));
	if (Tokens.FindItem(LoadAllTokenCheck, DummyIndex) == TRUE)
	{
		bLoadTest = TRUE;
	}
	// UnloadAll test
	FString UnloadAllTokenCheck(TEXT("UnloadAll"));
	if (Tokens.FindItem(UnloadAllTokenCheck, DummyIndex) == TRUE)
	{
		bUnloadTest = TRUE;
	}

	if (bLoadTest || bUnloadTest)
	{
		// Find the pawn...
		AGearPawn* GearPawn = NULL;
		for (TObjectIterator<AGearPawn> It; It; ++It)
		{
			GearPawn = *It;
			if (GearPawn->GetName().InStr(InPawnName) != -1)
			{
				break;
			}
			GearPawn = NULL;
		}

		if (GearPawn == NULL)
		{
			warnf(TEXT("GUDManager: Can't find pawn for %s"), *InPawnName);
			return;
		}
		else
		{
			GUDSDebugf(TEXT("GUDManager: Running %s test on %s"), 
				bLoadTest ? TEXT("LoadTest") : TEXT("UnloadTest"),
				*(GearPawn->GetName()));
		}

		// Grab the GUDBank name
		if (GearPawn->MasterGUDBankClassNames.Num() == 0)
		{
			warnf(TEXT("GUDManager: Pawn has no GUDBankClassNames: %s"), *(GearPawn->GetName()));
			return;
		}

		for (INT GBIndex = 0; GBIndex < GearPawn->MasterGUDBankClassNames.Num(); GBIndex++)
		{
			FString GUDBankName = GearPawn->MasterGUDBankClassNames(GBIndex);

			if (bLoadTest)
			{
				if (LoadAllVarietyBanks(GUDBankName))
				{
					GUDSDebugf(TEXT("GUDManager: LoadAllVarietyBanks successful for %s"), *GUDBankName);
				}
				else
				{
					GUDSDebugf(TEXT("GUDManager: LoadAllVarietyBanks failed for %s"), *GUDBankName);
				}
			}
			else
			{
				if (UnloadAllVarietyBanks(GUDBankName))
				{
					GUDSDebugf(TEXT("GUDManager: UnloadAllVarietyBanks successful for %s"), *GUDBankName);
				}
				else
				{
					GUDSDebugf(TEXT("GUDManager: UnloadAllVarietyBanks failed for %s"), *GUDBankName);
				}
			}
		}
	}

	// LoadRandom test...
	FString LoadRandomTokenCheck(TEXT("LoadRandom"));
	if (Tokens.FindItem(LoadRandomTokenCheck, DummyIndex) == TRUE)
	{
		check(Tokens.Num() > 1);
		INT Count = appAtoi(*(Tokens(1)));

		UBOOL bClearOldOnes = TRUE;
		if (Tokens.Num() > 2)
		{
			if (Tokens(2) == TEXT("NOCLEAR"))
			{
				bClearOldOnes = FALSE;
			}
		}

		for (TObjectIterator<AGearPawn> It; It; ++It)
		{
			AGearPawn* GearPawn = *It;
			if (GearPawn->GetName().InStr(InPawnName) != -1)
			{
				LoadRandomVarietyBanks(GearPawn, Count, bClearOldOnes);
				break;
			}
		}
	}

	// Load test...
	FString LoadTokenCheck(TEXT("Load"));
	if (Tokens.FindItem(LoadTokenCheck, DummyIndex) == TRUE)
	{
		check(Tokens.Num() > 1);
		INT VarietyIndex = appAtoi(*(Tokens(1)));
		for (TObjectIterator<AGearPawn> It; It; ++It)
		{
			AGearPawn* GearPawn = *It;
			if (GearPawn->GetName().InStr(InPawnName) != -1)
			{
				FString GUDBankName = GetSpeakerGUDBankName(GearPawn);
				LoadVarietyGUDBank(GUDBankName, VarietyIndex);
				break;
			}
		}
	}

	// Flush test
	FString FlushAllTokenCheck(TEXT("FlushAll"));
	if (InPawnName == FlushAllTokenCheck)
	{
		FlushOutAllVarietyBanks();
	}
}

EGUDActionID AGUDManager::ChooseActionToPlay_Directional(const struct FGUDEvent& Event, class AActor*& Speaker) const
{
	// instigator asking for direction to the recipient
	// if no recipient, assume asking for direction to the referencedpawn.

	// all directional responses will be relative to the asker/instigator's frame of reference.

	// directional priorities are as follows: behind, above, below, right, left, ahead
	// e.g. presumably "behind" is more useful than "above" for an enemy who is both above and behind you

	BYTE DesiredActionID = GUDAction_None;
	Speaker = NULL;

	AGearPawn* const TargetPawn = Event.Recipient ? Event.Recipient : Event.ReferencedPawn;

	// need both, else just bail
	if (Event.Instigator && TargetPawn)
	{
		FVector const InstigatorLoc = Event.Instigator->Location;
		FVector const TargetLoc = TargetPawn->Location;

		FVector InstigatorFVec;
		{
			// if Instigator is a player, use his aim direction, not the pawn's direction
			AGearPC* InstigatorPC = Cast<AGearPC>(Event.Instigator->Controller);
			if (InstigatorPC != NULL)
			{
				InstigatorFVec = InstigatorPC->Rotation.Vector();
			}
			else
			{
				InstigatorFVec = Event.Instigator->Rotation.Vector();
			}
		}

		FVector ToRecipientDir = TargetLoc - InstigatorLoc;

		FVector ToRecipientDirXY = ToRecipientDir;
		ToRecipientDirXY.Z = 0.f;
		ToRecipientDirXY.Normalize();

		FLOAT const DotXY = ToRecipientDirXY | InstigatorFVec;
		FGUDEventProperties const& EventProps = EventProperties(Event.Id);

		if (DotXY < 0.f)
		{
			// note we don't check up/down at all for enemies behind us.

			if (DotXY < -0.5f)		// .5 = cos(60deg)
			{
				// behind
				DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Behind);
			}
			else
			{
				// figure out left or right
				FVector const InstigatorRVec = InstigatorFVec ^ FVector(0,0,1);
				if ( (InstigatorRVec | ToRecipientDirXY) > 0.f )
				{
					// left
					DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Left);
				}
				else
				{
					// right
					DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Right);
				}
			}
		}
		else
		{
			// check up/down
			ToRecipientDir.Normalize();
			if (Abs(ToRecipientDir.Z) > 0.25f)			// sin(15deg) ~= .25
			{
				if (InstigatorLoc.Z < TargetLoc.Z)
				{
					// above
					DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Above);
				}
				else
				{
					// below
					DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Below);
				}
			}
		}
		if (DesiredActionID == GUDAction_None)
		{
			// check left/right/ahead
			if (DotXY < 0.94f)		// .94 is about 20 deg
			{
				// figure out left or right
				FVector const InstigatorRVec = InstigatorFVec ^ FVector(0,0,1);
				if ( (InstigatorRVec | ToRecipientDirXY) > 0.f )
				{
					// left
					DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Left);
				}
				else
				{
					// right
					DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Right);
				}
			}
			else
			{
				// ahead
				DesiredActionID = EventProps.ActionIDs(GUDActionMap_Dir_Ahead);
			}
		}

		if (DesiredActionID != GUDAction_None)
		{
			// now, find a speaker
			switch (EventProps.ForcedSpeakerRole)
			{
			case GUDRole_None:
			case GUDRole_TeammateWitness:
				// team witness by default
				Speaker = FindWitnessSpeaker(Event, DesiredActionID, TRUE, FALSE, FALSE);
				break;
			case GUDRole_Instigator:
				if (SpeakerHasLineForAction(*Event.Instigator, DesiredActionID) && IsValidSpeaker(Event.Instigator))
				{
					Speaker = Event.Instigator;
				}
				break;
			case GUDRole_Recipient:
				if (SpeakerHasLineForAction(*Event.Recipient, DesiredActionID) && IsValidSpeaker(Event.Recipient))
				{
					Speaker = Event.Recipient;
				}
				break;
			case GUDRole_EnemyWitness:
				Speaker = FindWitnessSpeaker(Event, DesiredActionID, FALSE, TRUE, FALSE);
				break;
			}

			if (Speaker == NULL)
			{
				// no speaker, clear the desired action for return
				DesiredActionID = GUDAction_None;
			}
		}
	}

	return (EGUDActionID) DesiredActionID;
}

EGUDActionID AGUDManager::ChooseActionToPlay_LocationDescription(const struct FGUDEvent& Event,class AActor*& Speaker) const
{
	// try to use description based on cover metadata
	BYTE DesiredActionID = GUDAction_None;
	Speaker = NULL;

	AGearPawn* const TargetPawn = Event.Recipient ? Event.Recipient : Event.ReferencedPawn;
	FGUDEventProperties const& EventProps = EventProperties(Event.Id);

	// need both, else just bail
	if (Event.Instigator && TargetPawn)
	{
		if (TargetPawn->IsInCover())
		{
			INT const CoverSlotIdx = (TargetPawn->CurrentSlotPct < 0.5f) ? TargetPawn->LeftSlotIdx : TargetPawn->RightSlotIdx;
			ECoverLocationDescription LocDesc = (ECoverLocationDescription) TargetPawn->CurrentLink->GetLocationDescription(CoverSlotIdx);
			if (LocDesc != CoverDesc_None)
			{
				// note: this assumes a correct layout of the GUDActionMap_LocationDescription enum
				DesiredActionID = EventProps.ActionIDs(LocDesc);
			}

			GUDSLog(TEXT("... Location Description for %s is %d"), *TargetPawn->GetName(), INT(LocDesc));
		}
		else
		{
			// check for "in the open!"
			// maybe do traces here?  
			DesiredActionID = EventProps.ActionIDs(GUDActionMap_LocDesc_InTheOpen);
		}
	}

	if (DesiredActionID != GUDAction_None)
	{
		// now, find a speaker
		switch (EventProps.ForcedSpeakerRole)
		{
		case GUDRole_None:
		case GUDRole_TeammateWitness:
			// team witness by default
			Speaker = FindWitnessSpeaker(Event, DesiredActionID, TRUE, FALSE, FALSE);
			break;
		case GUDRole_Instigator:
			if (SpeakerHasLineForAction(*Event.Instigator, DesiredActionID) && IsValidSpeaker(Event.Instigator))
			{
				Speaker = Event.Instigator;
			}
			break;
		case GUDRole_Recipient:
			if (SpeakerHasLineForAction(*Event.Recipient, DesiredActionID) && IsValidSpeaker(Event.Recipient))
			{
				Speaker = Event.Recipient;
			}
			break;
		case GUDRole_EnemyWitness:
			Speaker = FindWitnessSpeaker(Event, DesiredActionID, FALSE, TRUE, FALSE);
			break;
		}

		if (Speaker == NULL)
		{
			// no speaker, clear the desired action for return
			DesiredActionID = GUDAction_None;
		}
	}

	// the rest of the time, use direction
	return (EGUDActionID) DesiredActionID;
}


void AGUDManager::ChooseLineFromAction(BYTE ActionID, class AActor* Speaker, struct FGUDLine& outLine, class AGearPawn** outAddressee, class AGearPawn** outReferringTo, const struct FGUDEvent* Event)
{
	if (outAddressee)
	{
		*outAddressee = NULL;
	}
	if (outReferringTo)
	{
		*outReferringTo = NULL;
	}
	outLine.Audio = NULL;

	if ( (ActionID != GUDAction_None) && (Speaker != NULL) )
	{
		UGUDBank const* DefaultGUDData = NULL;
		UBOOL bIsInCombat = FALSE;

		// speaker can be of multiple base classes, sort that out	
		AGearPawn* const WPSpeaker = Cast<AGearPawn>(Speaker);
		if (WPSpeaker && (WPSpeaker->MasterGUDBankClassNames.Num() > 0) && (WPSpeaker->LoadedGUDBank >= 0))
		{
			const FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(WPSpeaker->MasterGUDBankClassNames(WPSpeaker->LoadedGUDBank));
			if (GUDBankCollection && (GUDBankCollection->LoadedAssets.Num() > 0))
			{
				DefaultGUDData = GUDBankCollection->LoadedAssets(0).Bank;

				UBOOL bPlayingMP = FALSE;
				if (WorldInfo->GRI)
				{
					if ( WorldInfo->GRI->GameClass->IsChildOf(AGearGameMP_Base::StaticClass()) )
					{
						bPlayingMP = TRUE;
					}
				}

				bIsInCombat = bPlayingMP || 
							  bForceInCombatGUDS ||
							  (WPSpeaker->Controller != NULL && WPSpeaker->Controller->eventIsInCombat());
			}
		}
		else
		{
			AGearRemoteSpeaker *RemoteSpeaker = Cast<AGearRemoteSpeaker>(Speaker);
			if (RemoteSpeaker && (RemoteSpeaker->MasterGUDBankClassNames.Num() > 0) && (RemoteSpeaker->LoadedGUDBank >= 0))
			{
				// speaker is a remote speaker, use that gud data
				const FGUDBankCollection* GUDBankCollection = GUDBankCollectionsMap.Find(RemoteSpeaker->MasterGUDBankClassNames(WPSpeaker->LoadedGUDBank));
				if (GUDBankCollection && (GUDBankCollection->LoadedAssets.Num() > 0))
				{
					DefaultGUDData = GUDBankCollection->LoadedAssets(0).Bank;
					bIsInCombat = FALSE;		// remote speakers are never in combat
				}
			}
		}

		if ( DefaultGUDData && (ActionID < (UINT)DefaultGUDData->GUDActions.Num()) )
		{
			FGUDAction const* Action = &DefaultGUDData->GUDActions(ActionID);

			const TArray<INT> *LineIndices = NULL;
			TArray<INT> TempIndices;
			// use the combat lines if available and in combat
			if (bIsInCombat)
			{
				if (Action->CombatOnlyLineIndices.Num() > 0)
				{
					TempIndices = Action->LineIndices;
					TempIndices += Action->CombatOnlyLineIndices;
					LineIndices = &TempIndices;
				}
				else
				{
					LineIndices = &Action->LineIndices;
				}
			}
			else
			{
				if (Action->NonCombatOnlyLineIndices.Num() > 0)
				{
					TempIndices = Action->LineIndices;
					TempIndices += Action->NonCombatOnlyLineIndices;
					LineIndices = &TempIndices;
				}
				else
				{
					LineIndices = &Action->LineIndices;
				}
			}

			const INT NumLines = LineIndices->Num();
			if (NumLines > 0)
			{
				// choose one of the lines randomly
				const INT LineIdx = (appRand() % NumLines);
				const INT GUDLineIdx = (*LineIndices)(LineIdx);
				FGUDLine L;
				L = DefaultGUDData->GUDLines(GUDLineIdx);			// struct copy

				outLine = L;
				if (Event != NULL)
				{
					// find addressee, in any
					if (outAddressee)
					{
						switch (outLine.Addressee)
						{
						case GUDRole_Instigator:
							*outAddressee = Event->Instigator;
							break;
						case GUDRole_Recipient:
							*outAddressee = Event->Recipient;
							break;
						}
					}

					// find referred-to pawn, if any
					if (outReferringTo)
					{
						switch (outLine.ReferringTo)
						{
						case GUDRole_Instigator:
							*outReferringTo = Event->Instigator;
							break;
						case GUDRole_Recipient:
							*outReferringTo = Event->Recipient;
							break;
						case GUDRole_ReferencedPawn:
							*outReferringTo = Event->ReferencedPawn;
							break;
						}
					}
				}
			}
		}
	}
}

// Will return Speaker as NULL if no valid line was chosen.
void AGUDManager::ChooseLineFromEvent(const struct FGUDEvent& Event,struct FGUDLine& outLine,class AActor*& outSpeaker,class AGearPawn*& outAddressee,class AGearPawn*& outReferringTo, BYTE& outActionID)
{
	// call the proper choice function.  didn't go with delegates here, to avoid the 
	// overhead, since this will be called semi-regularly
	outSpeaker = NULL;
	outActionID = GUDAction_None;

	FGUDEventProperties const& EventProps = EventProperties(Event.Id);
	switch(EventProps.ChooseActionMethod)
	{
	case GUDChooseMethod_Normal:
		outActionID = ChooseActionToPlay_Normal(Event, outSpeaker);
		break;
	case GUDChooseMethod_Directional:
		outActionID = ChooseActionToPlay_Directional(Event, outSpeaker);
		break;
	case GUDChooseMethod_LocationDescription:
		outActionID = ChooseActionToPlay_LocationDescription(Event, outSpeaker);
		break;
	}

	if ( (outActionID != GUDAction_None) && (outSpeaker != NULL) )
	{
		ChooseLineFromAction(outActionID, outSpeaker, outLine, &outAddressee, &outReferringTo, &Event);
	}
}


/** Internal, used for the GUDSLog calls. */
static inline FString GetNameSafe(UObject* const Obj)
{
	return Obj ? Obj->GetName() : FString("None");
}
/** Internal, used for the GUDSLog calls. */
static inline FString GetFullNameSafe(UObject* const Obj)
{
	return Obj ? Obj->GetFullName() : FString("None");
}



void AGUDManager::TriggerGUDEventInternal(BYTE ID,class APawn* InInstigator,class APawn* Recipient,class APawn* ReferringTo,FLOAT DelaySec,UBOOL bResponseEvent)
{
	if (Role == ROLE_Authority)
	{
		GUDSLog(TEXT("Got GUDEvent Trigger: %s Inst: %s Rec: %s RefTo: %s Delay: %f"), *eventGetEventName(ID), *GetNameSafe(InInstigator), *GetNameSafe(Recipient), *GetNameSafe(ReferringTo), DelaySec);

		if (ID != GUDEvent_None)
		{
			FGUDEventProperties const& EventProps = EventProperties(ID);

			// find pct chance to play
			FLOAT Chance = GlobalChanceToPlayMultiplier;

			if (EventProps.ChanceToPlayMP > 0.f )
			{
				UBOOL bPlayingMP = FALSE;
				if (WorldInfo->GRI && WorldInfo->GRI->GameClass)
				{
					if ( WorldInfo->GRI->GameClass->IsChildOf(AGearGameMP_Base::StaticClass()) )
					{
						bPlayingMP = TRUE;
					}
				}
			
				Chance *= (bPlayingMP) ? EventProps.ChanceToPlayMP : EventProps.ChanceToPlay;
			}
			else
			{
				Chance *= EventProps.ChanceToPlay;
			}

			// roll the dice on it right now...
			FLOAT Rand = appSRand();

#if !FINAL_RELEASE
			if (bDebugGUDEvents)
			{
				Rand = 0.f;
			}
#endif

			if ( ( Rand <= Chance ) &&
				 ( ( (WorldInfo->TimeSeconds - EventProps.TimeLastPlayed) > EventProps.MinTimeBetweenSec) || bDebugGUDEvents ) &&
				 (WorldInfo->Pauser == NULL) )
			{
				FGUDEvent NewEvent;
				NewEvent.Instigator = GetGUDSPawn(InInstigator);
				NewEvent.Recipient = GetGUDSPawn(Recipient);
				NewEvent.ReferencedPawn = Cast<AGearPawn>(ReferringTo);
				NewEvent.Id = ID;
				NewEvent.DelayTimeRemainingSec = DelaySec;
				NewEvent.bIsResponseEvent = bResponseEvent;

				// queue it up, decide later if we'll run it or not.
				QueuedEvents.AddItem(NewEvent);
			}
			else
			{
				GUDSLog(TEXT("GUDEvent discarded. %s Inst: %s Rec %s Rand/Chance: %f/%f TooSoon: %d Pauser: %s"), *eventGetEventName(ID), *GetNameSafe(InInstigator), *GetNameSafe(Recipient), Rand, Chance, UBOOL((WorldInfo->TimeSeconds - EventProps.TimeLastPlayed) <= EventProps.MinTimeBetweenSec), *GetNameSafe(WorldInfo->Pauser));
			}
		}
	}
}

AGearPawn* AGUDManager::GetGUDSPawn(class APawn* P) const
{
	if (P)
	{
		AGearPawn* const GP = Cast<AGearPawn>(P);
		if (GP)
		{
			// regular GearPawn
			return GP;
		}

		// vehicle (handles turrets, gearweaponpawns too)
		AVehicle const* V = Cast<AVehicle>(P);
		if (V)
		{
			return Cast<AGearPawn>(V->Driver);
		}
	}

	return NULL;
}

UBOOL AGUDManager::IsObserverLine(EGUDEventID EventID, EGUDActionID ActionID) const
{
	if (EventID != GUDEvent_None)
	{
		FGUDEventProperties const& EventProps = EventProperties(EventID);

		if (EventProps.ChooseActionMethod == GUDChooseMethod_Normal)
		{
			if ( (EventProps.ActionIDs(GUDActionMap_EnemyWitness) == ActionID) || (EventProps.ActionIDs(GUDActionMap_TeamWitness) == ActionID) )
			{
				return TRUE;
			}
		}
		else if ( (EventProps.ForcedSpeakerRole == GUDRole_TeammateWitness) || (EventProps.ForcedSpeakerRole == GUDRole_EnemyWitness) )
		{
			return TRUE;
		}
	}

	return FALSE;
}


void AGUDManager::HandleGUDEvent(const struct FGUDEvent& Event)
{
	FGUDEventProperties& EventProps = EventProperties(Event.Id);

	// choose a line...
	FGUDLine LineToPlay;
	AActor* Speaker = NULL;
	AGearPawn* Addressee = NULL;
	AGearPawn* ReferringTo = NULL;

	BYTE AID = 0;
	ChooseLineFromEvent(Event, LineToPlay, Speaker, Addressee, ReferringTo, AID);
	EGUDActionID const ActionID = EGUDActionID(AID);

	GUDSLog(TEXT("Handling GUD Event %s: Inst: %s Rec: %s Ref: %s Chosen Speaker: %s"), *eventGetEventName(Event.Id), *GetNameSafe(Event.Instigator), *GetNameSafe(Event.Recipient), *GetNameSafe(Event.ReferencedPawn), *GetNameSafe(Speaker));

	// for observers, add a slight delay to simulate reaction time.  feels more natural this way.
	FLOAT const DelaySec = (IsObserverLine(EGUDEventID(Event.Id), ActionID)) ? EventProps.ObserverDelay : 0.f;

	UBOOL bSpoke = PlayGUDSLineInternal(LineToPlay, Speaker, Addressee, ReferringTo, DelaySec, EventProps.MPBroadcastFilter);

	if (bSpoke)
	{
		EventProps.TimeLastPlayed = WorldInfo->TimeSeconds;
		for (INT Idx=0; Idx<EventProps.LinkedEvents.Num(); ++Idx)
		{
			BYTE const TmpEventID = EventProps.LinkedEvents(Idx);
			EventProperties(TmpEventID).TimeLastPlayed = WorldInfo->TimeSeconds;
		}
	}
}

UBOOL AGUDManager::PlayGUDSLineInternal(const FGUDLine& LineToPlay, AActor* Speaker, AGearPawn* Addressee, AGearPawn* ReferringTo, FLOAT DelaySec, BYTE MPBroadcastFilter)
{
	UBOOL bSpoke = FALSE;

	if (Speaker != NULL)
	{
		AGearPawn* const SpeakerGP = Cast<AGearPawn>(Speaker);
		if (SpeakerGP != NULL)
		{
			if ( (SpeakerGP->LastGUDLinePlayed != LineToPlay.Audio) || 
				 (LineToPlay.Audio == NULL) || 
				 ( (WorldInfo->TimeSeconds - SpeakerGP->LastGUDLinePlayedTime) > SpeakerGP->GUDLineRepeatMin ) )
			{
				GUDSLog(TEXT("... speaking: %s saying %s"), *GetNameSafe(SpeakerGP), *GetNameSafe(LineToPlay.Audio));
				bSpoke = SpeakerGP->SpeakLine(Addressee, LineToPlay.Audio, TEXT(""), DelaySec, Speech_GUDS, 0, FALSE, MPBroadcastFilter);

				if (bSpoke)
				{
					SpeakerGP->LastGUDLinePlayed = LineToPlay.Audio;
					SpeakerGP->LastGUDLinePlayedTime = WorldInfo->TimeSeconds;

					// set up responses
					if (LineToPlay.ResponseEventIDs.Num() > 0)
					{
						BYTE const ResponseEventID = LineToPlay.ResponseEventIDs(RandHelper(LineToPlay.ResponseEventIDs.Num()));
						if (ResponseEventID != GUDEvent_None)
						{
							FLOAT SpeakTime = 0.f;
							if (LineToPlay.Audio)
							{
								SpeakTime = LineToPlay.Audio->GetCueDuration() + 0.5f;
							}
							else
							{
								// time enough to read
								SpeakTime = 2.f;
							}	
							
							GUDSLog(TEXT("Triggering response event. %s Inst: %s Rec: %s RefTo: %s Delay: %f"), *eventGetEventName(ResponseEventID), *GetNameSafe(SpeakerGP), *GetNameSafe(Addressee), *GetNameSafe(ReferringTo), SpeakTime);
							TriggerGUDEventInternal(ResponseEventID, SpeakerGP, Addressee, ReferringTo, SpeakTime, TRUE);
						}
					}		
				}
			}
		}
		else
		{
			AGearRemoteSpeaker* const RemoteSpeaker = Cast<AGearRemoteSpeaker>(Speaker);
			if (RemoteSpeaker)
			{
				GUDSLog(TEXT("... speaking: %s saying %s"), *GetNameSafe(SpeakerGP), *GetNameSafe(LineToPlay.Audio));
				bSpoke = RemoteSpeaker->eventRemoteSpeakLine(Addressee, LineToPlay.Audio, TEXT(""), DelaySec, FALSE, MPBroadcastFilter, Speech_GUDS);
			}
		}
	}

	return bSpoke;
}

/** Used to perform a GUDS action without going through the event system.  Useful for specific one-off lines, e.g. Boomer "boom!" or certain Anya lines.  */
void AGUDManager::PlayGUDSAction(BYTE ActionID, class AActor* Speaker, class AGearPawn* Addressee, class AGearPawn* ReferringTo, BYTE MPBroadcastFilter)
{
	FGUDLine LineToPlay;
	GUDSLog(TEXT("PlayGUDSAction %s Spkr: %s Addr: %s RefTo: %s"), *eventGetActionName(ActionID), *GetNameSafe(Speaker), *GetNameSafe(Addressee), *GetNameSafe(ReferringTo) );
	ChooseLineFromAction(ActionID, Speaker, LineToPlay);
	PlayGUDSLineInternal(LineToPlay, Speaker, Addressee, ReferringTo, 0.f, MPBroadcastFilter);
}

void AGUDManager::NotifyExclusiveSpeech(class AGearPawn* WP,class USoundCue* Audio,BYTE SpeechPriority)
{
	if (Audio && WP)
	{
		BYTE TeamNum = WP->GetTeamNum();
		if (TeamNum > 1)
		{
			// non-cog/locust teams get lumped onto team 2 for array reasons
			// note that this doesn't happen in mp, so wingman isn't an issue here
			TeamNum = 2;
		}

		// note: allow locust to overlap speech, to get more enemy chatter
		if (TeamNum != 1)
		{
			// disable GUDS for that team
			TeamChannel[TeamNum] = 1;

			// figure duration of exclusive speech
			FLOAT ExtraDelay = 0.f;
			if (SpeechPriority == Speech_GUDS)
			{
				ExtraDelay = 0.5f;
			}
			else if (SpeechPriority >= Speech_Scripted)
			{
				ExtraDelay = 1.5f;
			}
			FLOAT const DelaySec = Audio->GetCueDuration() + ExtraDelay;

			// and re-enable soon as possible
			switch (TeamNum)
			{
			case 0:
				SetTimer(DelaySec, FALSE, FName(TEXT("TemporaryDisableFinished_0")));
				break;
			case 1:
				SetTimer(DelaySec, FALSE, FName(TEXT("TemporaryDisableFinished_1")));
				break;
			case 2:
				SetTimer(DelaySec, FALSE, FName(TEXT("TemporaryDisableFinished_2")));
				break;
			}
		}

	}
}

UBOOL AGUDManager::IsOkToPlayGUDSLine(INT TeamNum,UBOOL bResponseEvent) const
{
	if ( (TeamNum < 0) || ((TeamNum > 2) && TeamNum != 255) )
	{
		// invalid teamnum.  only 0, 1, 2, 255 are valid
		return FALSE;
	}

	if (bResponseEvent)
	{
		// ignore TeamChannel filters for response events
		return ( !bDisabled && (GlobalChanceToPlayMultiplier > 0.f) );
	}
	else
	{
		return ( !bDisabled && (TeamNum == 255 || TeamChannel[TeamNum] == 0) && (GlobalChanceToPlayMultiplier > 0.f) );
	}
}

void AGUDManager::DrawDebugStreaming(class UCanvas* Canvas)
{
	if (!Canvas)
	{
		return;
	}

	const INT DrawX = 50;

	// save state
	FLOAT OldX = Canvas->CurX, OldY = Canvas->CurY;
	FColor OldColor = Canvas->DrawColor;

	Canvas->SetPos(DrawX, 25);
	Canvas->SetDrawColor(255,255,255,255);

	Canvas->SetPos(DrawX, Canvas->CurY);
	FString Tmp = FString::Printf(TEXT("total mem ~%d"), ApproxTotalUsedMemory);
	Canvas->DrawText(Tmp);

	for (TMap<FString,FGUDBankCollection>::TConstIterator It(GUDBankCollectionsMap); It; ++It)
	{
		FGUDBankCollection const& BC = It.Value();

		FString GUDBankName = It.Key();
		FGUDCollection* const ToC = TableOfContents->TableOfContents.Find(GUDBankName);

		Canvas->SetPos(DrawX, Canvas->CurY);
		Canvas->DrawText(GUDBankName);
		
		for (INT AssetIdx=0; AssetIdx<BC.LoadedAssets.Num(); AssetIdx++)
		{
			FLoadedGUDAsset const& Asset = BC.LoadedAssets(AssetIdx);
			if (Asset.Bank)
			{
				INT ApproxMem = 0;
				if (AssetIdx == 0)
				{
					ApproxMem = ToC->RootGUDBank.ApproxBankSize;
				}
				else
				{
					ApproxMem = ToC->GUDBanks(AssetIdx-1).ApproxBankSize;
				}

				UPackage* Pkg = Asset.Bank->GetOutermost();
				//INT PackageFileSize = Pkg->GetFileSize();
				//
				//// loading cue bank?
				//FString LocalName = GUDBankName;
				//{
				//	INT Index = GUDBankName.InStr(TEXT("."));
				//	if (Index != INDEX_NONE)
				//	{
				//		LocalName = GUDBankName.Right(GUDBankName.Len() - Index - 1);
				//	}
				//	LocalName = LocalName.Replace(TEXT("GUDData_"), TEXT("GUD_"));
				//}

				//FString const LoadGUDBankName = (AssetIdx == 0) ? 
				//	LocalName + FString(TEXT("G_SF"))
				//	: LocalName + FString::Printf(TEXT("G%d_SF"), AssetIdx);
				//FString LocalizedPackageName = LoadGUDBankName + LOCALIZED_SEEKFREE_SUFFIX;

				//FString PkgFilename;
				//GPackageFileCache->FindPackageFile(*LocalizedPackageName, NULL, PkgFilename);
				//INT UncompFileSize = GFileManager->UncompressedFileSize(*PkgFilename);

				//INT CompFileSize = GFileManager->FileSize(*PkgFilename);

				//FString Txt = FString::Printf(TEXT("   %s (%d, %d, %d, %d)"), *Pkg->GetName(), PackageFileSize, UncompFileSize, CompFileSize, ApproxMem);
				FString Txt = FString::Printf(TEXT("   %s (%d)"), *Pkg->GetName(), ApproxMem);
				Canvas->SetPos(DrawX, Canvas->CurY);
				Canvas->DrawText(Txt);
			}
		}
	}

	// restore state
	Canvas->SetDrawColor(OldColor.R, OldColor.G, OldColor.B, OldColor.A);
	Canvas->SetPos(OldX, OldY);
}


/** This will log out all of the currently loaded guds data to the passed in FOutputDevice **/
void AGUDManager::LogoutStreamedPackageData( FOutputDevice& Ar )
{
	if( (GUseSeekFreeLoading) && bUseGUDS && !bDisableGUDSStreaming )
	{
		// get various header info
		const FString ApproxTotalUsedMemoryString = FString::Printf(TEXT("Total Mem: ~%d b %5.2f kb %5.2f mb"), ApproxTotalUsedMemory, ApproxTotalUsedMemory/1024.0f, ApproxTotalUsedMemory/1024.0f/1024.0f );
		Ar.Logf( *ApproxTotalUsedMemoryString );

		// now go over all of the loaded banks and log them out
		for( TMap<FString,FGUDBankCollection>::TConstIterator It(GUDBankCollectionsMap); It; ++It )
		{
			const FGUDBankCollection& BC = It.Value();

			const FString GUDBankName = It.Key();

			Ar.Logf( *GUDBankName );

			const FGUDCollection* ToC = TableOfContents->TableOfContents.Find(GUDBankName);

			for( INT AssetIdx = 0; AssetIdx < BC.LoadedAssets.Num(); ++AssetIdx )
			{
				const FLoadedGUDAsset& Asset = BC.LoadedAssets(AssetIdx);

				if( Asset.Bank != NULL )
				{
					INT ApproxMem = 0;
					if( AssetIdx == 0 )
					{
						ApproxMem = ToC->RootGUDBank.ApproxBankSize;
					}
					else
					{
						ApproxMem = ToC->GUDBanks(AssetIdx-1).ApproxBankSize;
					}

					UPackage* Pkg = Asset.Bank->GetOutermost();

					const FString ApproxVarietyPack = FString::Printf(TEXT("   %s (%d)"), *Pkg->GetName(), ApproxMem);
					Ar.Logf( *ApproxVarietyPack );

					// now print out all of the indiv soundcues in this variety package
					for( INT LineIdx = 0; LineIdx < Asset.Bank->GUDLines.Num(); ++LineIdx )
					{
						const FString SoundeCueName = Asset.Bank->GUDLines(LineIdx).Audio->GetFullName();

						// seems that the GUDLines is a sparse array
						if( SoundeCueName != TEXT( "None" ) )
						{
							const FString SoundCueLine = FString::Printf(TEXT("      %s"), *SoundeCueName );
							Ar.Logf( *SoundCueLine );
						}
					}
				}
			}
		}
	}
}


void UGearSoundGroup::PostLoad()
{
	Super::PostLoad();
	ValidateData();

	if (VoiceEfforts.Num() > 0)
	{
		LastEffortIdx.Add(VoiceEfforts.Num());
	}
}


void UGearSoundGroup::PostEditChange( class FEditPropertyChain& PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
	ValidateData();

	// @fixme, if Parent changes, ensure no dependency loop?
}



/** Enforces design that the "type" members match the enum/array position, so that content guys can see what they are editing. */
void UGearSoundGroup::ValidateData()
{
	// Note: patterned after GearPhysicalMaterialProperty
	//
	// we have a number of issues we need to solve here.
	// 0) someone has accidentally deleted an array entry and didn't notice it
	// 1) we have added new types which need to be added to the array
	// 2) we have a version based change where we have don't something non trivial (i.e. 1 is trivial)
	//
	// we must always look for missing entries
	// the others cases we can do based off version of the class 

	//// FoleySoundFX

	// make certain that we have all of the entries we should have.  (this takes care of the case of adding a new enum)
	while( FoleySoundFX.Num() < GearFoley_MAX )
	{
		FGearFoleyEntry NewEntry;
		appMemzero( &NewEntry, sizeof(NewEntry) );
		NewEntry.Id = FoleySoundFX.Num();

		FoleySoundFX.AddItem( NewEntry );
		MarkPackageDirty( TRUE );
		//warnf( TEXT("ADDED %s %d"), *GetFullName(), FXInfo.Num() );
	}

	// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
	for( INT Idx = 0; Idx < FoleySoundFX.Num() && Idx < GearFoley_MAX ; ++Idx )
	{
		FGearFoleyEntry const& Entry = FoleySoundFX( Idx );
		if( Entry.Id != Idx )
		{
			FGearFoleyEntry NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Id = FoleySoundFX.Num();

			FoleySoundFX.InsertItem( NewEntry, Idx );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
		}
	}


	// can do specific version remappings now

	//// VoiceEfforts
	if (ObjVersion != ClassVersion)
	{
		// do any version-specific upgrades
		//if ( (ObjVersion < 1) && (ClassVersion >= 1) )
		//{
		//	// version 0 to 1, renamed several outdated enums to "Unused"
		//	// serialization will set the fields using the old name to GearEffort_MAX
		//	VoiceEfforts(GearEffort_Unused00).Id = GearEffort_Unused00;
		//	VoiceEfforts(GearEffort_Unused01).Id = GearEffort_Unused01;
		//	VoiceEfforts(GearEffort_Unused02).Id = GearEffort_Unused02;
		//	VoiceEfforts(GearEffort_Unused03).Id = GearEffort_Unused03;
		//	VoiceEfforts(GearEffort_Unused04).Id = GearEffort_Unused04;
		//	VoiceEfforts(GearEffort_Unused05).Id = GearEffort_Unused05;
		//	VoiceEfforts(GearEffort_Unused06).Id = GearEffort_Unused06;
		//}

		if ( (ObjVersion < 2) && (ClassVersion >= 2) )
		{
			// version 1 to 2, removed several unused entries altogether
			// supercedes 0->1 upgrade
			VoiceEfforts.Remove(37);	// GearEffort_Unused06
			VoiceEfforts.Remove(15);	// GearEffort_Unused05
			VoiceEfforts.Remove(13);	// GearEffort_Unused04
			VoiceEfforts.Remove(9);		// GearEffort_Unused03
			VoiceEfforts.Remove(8);		// GearEffort_Unused02
			VoiceEfforts.Remove(7);		// GearEffort_Unused01
			VoiceEfforts.Remove(5);		// GearEffort_Unused00

			// repair ids
			for (INT Idx=0; Idx<VoiceEfforts.Num(); ++Idx)
			{
				VoiceEfforts(Idx).Id = Idx;
			}
		}

		if ( (ObjVersion < 3) && (ClassVersion >= 3) )
		{
			// version 2 to 3, removed OnFireSmall
			// renamed OnFireMedium to OnFirePain
			// renamed OnFireLongDeath to OnFireDeath
			VoiceEfforts.Remove(28);	// GearEffort_OnFireSmall

			// repair ids
			for (INT Idx=0; Idx<VoiceEfforts.Num(); ++Idx)
			{
				VoiceEfforts(Idx).Id = Idx;
			}
		}

		ObjVersion = ClassVersion;
		MarkPackageDirty(TRUE);
	}

	// make certain that we have all of the entries we should have.  (this takes care of the case of adding a new enum)
	while( VoiceEfforts.Num() < GearEffort_MAX )
	{
		FGearVoiceEffortEntry NewEntry;
		appMemzero( &NewEntry, sizeof(NewEntry) );
		NewEntry.Id = VoiceEfforts.Num();

		VoiceEfforts.AddItem( NewEntry );
		MarkPackageDirty( TRUE );
		//warnf( TEXT("ADDED %s %d"), *GetFullName(), FXInfo.Num() );
	}

	// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
	for( INT Idx = 0; Idx < VoiceEfforts.Num() && Idx < GearEffort_MAX ; ++Idx )
	{
		FGearVoiceEffortEntry const& Entry = VoiceEfforts( Idx );
		if( Entry.Id != Idx )
		{
			FGearVoiceEffortEntry NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Id = VoiceEfforts.Num();

			VoiceEfforts.InsertItem( NewEntry, Idx );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
		}
	}
}


class USoundCue* UGearSoundGroup::FindEffortCue(BYTE EffortID)
{
	UGearSoundGroup const* GSG = this;

	// search up parent chain looking for an entry with data
	while (GSG)
	{
		TArrayNoInit<USoundCue*> const& Sounds = GSG->VoiceEfforts(EffortID).Sounds;

		INT const NumSounds = Sounds.Num();
		if (NumSounds > 0)
		{
			INT RandIdx = RandHelper(NumSounds);

			// don't replay last one if we can help it
			if (RandIdx == LastEffortIdx(EffortID))
			{
				// choose next effort
				if (++RandIdx >= NumSounds)
				{
					RandIdx = 0;
				}
			}

			USoundCue* const Cue = Sounds(RandIdx);
			if (Cue)
			{
				LastEffortIdx(EffortID) = RandIdx;
				return Cue;
			}
		}

		GSG = GSG->Parent;
	}

	return NULL;
}

class USoundCue* UGearSoundGroup::FindFoleyCue(BYTE FoleyID) const
{
	UGearSoundGroup const* GSG = this;

	// search up parent chain looking for an entry with data
	while (GSG)
	{
		TArrayNoInit<USoundCue*> const& Sounds = GSG->FoleySoundFX(FoleyID).Sounds;

		INT const NumSounds = Sounds.Num();
		if (NumSounds > 0)
		{
			INT const RandIdx = RandHelper(NumSounds);
			USoundCue* const Cue = Sounds(RandIdx);
			if (Cue)
			{
				return Cue;
			}
		}

		GSG = GSG->Parent;
	}

	return NULL;
}



void UGearSoundGroup::DumpMemoryUsage(UBOOL bDetailed)
{
	debugf(TEXT("\n ==== GearSoundGroup object %s"), *GetName());

	// VOICE EFFORTS
	{
		if (bDetailed) debugf(TEXT("VOICE EFFORTS"));
		UEnum *GearVoiceEffortIDEnum = FindObject<UEnum>(ANY_PACKAGE,TEXT("GearVoiceEffortID"));

		INT TotalNumCues = 0;
		INT TotalNumWaves = 0;
		INT TotalMemUsage = 0;

		for( INT EffortIdx = 0; EffortIdx < VoiceEfforts.Num() && EffortIdx < GearEffort_MAX ; ++EffortIdx )
		{
			FGearVoiceEffortEntry const& Entry = VoiceEfforts( EffortIdx );
			INT EffortMemUsage = 0;

			INT NumCues = 0;
			INT NumWaves = 0;
			for (INT CueIdx=0; CueIdx<Entry.Sounds.Num(); ++CueIdx)
			{
				INT CueMemUsage = 0;

				USoundCue* SoundCue = Entry.Sounds(CueIdx);
				if (SoundCue)
				{
					TArray<USoundNodeWave*> Waves;
					SoundCue->RecursiveFindWaves( SoundCue->FirstNode, Waves );

					NumCues++;
					NumWaves += Waves.Num();

					for (INT WaveIdx=0; WaveIdx<Waves.Num(); ++WaveIdx)
					{
						USoundNodeWave* SoundNodeWave = Waves(WaveIdx);

#if PS3
						CueMemUsage += SoundNodeWave->CompressedPS3Data.GetBulkDataSize();;
#elif XBOX
						CueMemUsage += SoundNodeWave->CompressedXbox360Data.GetBulkDataSize();;
#else			
						switch( SoundNodeWave->DecompressionType )
						{
						case DTYPE_Native:
						case DTYPE_Preview:
							CueMemUsage += SoundNodeWave->SampleDataSize;
							break;

						case DTYPE_RealTime:
							CueMemUsage += SoundNodeWave->CompressedPCData.GetBulkDataSize();
							break;

						//case DTYPE_Setup:
						//case DTYPE_Invalid:
						//default:
						//	break;
						}
#endif
					}
				}

				EffortMemUsage += CueMemUsage;
			}


			FName EnumName(TEXT("Unknown"));
			if (GearVoiceEffortIDEnum != NULL)
			{
				EnumName = GearVoiceEffortIDEnum->GetEnum(EffortIdx);
			}

			if (bDetailed) debugf(TEXT("\tVoice Effort %s(%d) has %d cues and %d waves for a total of %.2f Kb"), *EnumName.ToString(), EffortIdx, NumCues, NumWaves, EffortMemUsage/1024.f);

			TotalNumCues += NumCues;
			TotalNumWaves += NumWaves;
			TotalMemUsage += EffortMemUsage;
		}

		debugf(TEXT("VOICE EFFORT TOTAL %d cues, %d Waves, %f Kb\n"), TotalNumCues, TotalNumWaves, TotalMemUsage/1024.f);
	}


	// FOLEYS
	{
		if (bDetailed) debugf(TEXT("FOLEYS"));
		UEnum *GearFoleyIDEnum = FindObject<UEnum>(ANY_PACKAGE,TEXT("GearFoleyID"));

		INT TotalNumCues = 0;
		INT TotalNumWaves = 0;
		INT TotalMemUsage = 0;

		for( INT EffortIdx = 0; EffortIdx < FoleySoundFX.Num() && EffortIdx < GearFoley_MAX ; ++EffortIdx )
		{
			FGearFoleyEntry const& Entry = FoleySoundFX( EffortIdx );
			INT EffortMemUsage = 0;

			INT NumCues = 0;
			INT NumWaves = 0;
			for (INT CueIdx=0; CueIdx<Entry.Sounds.Num(); ++CueIdx)
			{
				INT CueMemUsage = 0;

				USoundCue* SoundCue = Entry.Sounds(CueIdx);
				if (SoundCue)
				{
					TArray<USoundNodeWave*> Waves;
					SoundCue->RecursiveFindWaves( SoundCue->FirstNode, Waves );

					NumCues++;
					NumWaves += Waves.Num();

					for (INT WaveIdx=0; WaveIdx<Waves.Num(); ++WaveIdx)
					{
						USoundNodeWave* SoundNodeWave = Waves(WaveIdx);

#if PS3
						CueMemUsage += SoundNodeWave->CompressedPS3Data.GetBulkDataSize();;
#elif XBOX
						CueMemUsage += SoundNodeWave->CompressedXbox360Data.GetBulkDataSize();;
#else			
						switch( SoundNodeWave->DecompressionType )
						{
						case DTYPE_Native:
						case DTYPE_Preview:
							CueMemUsage += SoundNodeWave->SampleDataSize;
							break;

						case DTYPE_RealTime:
							CueMemUsage += SoundNodeWave->CompressedPCData.GetBulkDataSize();
							break;

							//case DTYPE_Setup:
							//case DTYPE_Invalid:
							//default:
							//	break;
						}
#endif
					}
				}

				EffortMemUsage += CueMemUsage;
			}

			FName EnumName(TEXT("Unknown"));
			if (GearFoleyIDEnum != NULL)
			{
				EnumName = GearFoleyIDEnum->GetEnum(EffortIdx);
			}

			if (bDetailed) debugf(TEXT("\tFoley SFX %s(%d) has %d cues and %d waves for a total of %.2f Kb"), *EnumName.ToString(), EffortIdx, NumCues, NumWaves, EffortMemUsage/1024.f);

			TotalNumCues += NumCues;
			TotalNumWaves += NumWaves;
			TotalMemUsage += EffortMemUsage;
		}

		debugf(TEXT("FOLEY TOTAL %d cues, %d Waves, %.2f Kb\n==="), TotalNumCues, TotalNumWaves, TotalMemUsage/1024.f);
	}
		
}

// CLIENT GUDS HANDLING

static void ClientGUDBankLoadCallback(UObject* InPackage, void* InPC)
{
	AGearPC* PC = (AGearPC*)InPC;
	if (InPackage != NULL)
	{
		// ATM it appears we just need to reference the GUDS bank object to get everything
		FString ChoppedPackageName = InPackage->GetName();
		INT SFIndex = ChoppedPackageName.InStr(TEXT("_SF"));
		if (SFIndex != -1)
		{
			ChoppedPackageName = ChoppedPackageName.Left(SFIndex);
		}
		UGUDBank* GUDBank = FindObject<UGUDBank>(NULL, *(ChoppedPackageName + TEXT(".GeneratedGUDBank")), FALSE); //@warning: hardcoded object name
		if (GUDBank == NULL)
		{
			debugf(NAME_Error, TEXT("Failed to find GUD bank %s.%s after loading GUD package %s"), *ChoppedPackageName, TEXT("GeneratedGUDBank"), *InPackage->GetName());
		}
		else
		{
			FName BankName = InPackage->GetFName();
			for (INT i = 0; i < PC->ClientGUDSReferences.Num(); i++)
			{
				if (PC->ClientGUDSReferences(i).PackageName == BankName)
				{
					PC->ClientGUDSReferences(i).Bank = GUDBank;
					UFaceFXAnimSet* FaceFXData = FindObject<UFaceFXAnimSet>(NULL, *(ChoppedPackageName + TEXT(".GeneratedFaceFXAnimSet")), FALSE); //@warning: hardcoded object name
					if (FaceFXData != NULL)
					{
						for (APawn* P = PC->WorldInfo->PawnList; P != NULL; P = P->NextPawn)
						{
							AGearPawn* GP = Cast<AGearPawn>(P);
							if ( GP != NULL && GP->Mesh != NULL && GP->Mesh->SkeletalMesh != NULL && GP->Mesh->SkeletalMesh->FaceFXAsset != NULL &&
								GP->MasterGUDBankClassNames.Num() > 0 && GP->MasterGUDBankClassNames.ContainsItem(GUDBank->SourceGUDBankPath) )
							{
								GP->Mesh->SkeletalMesh->FaceFXAsset->MountFaceFXAnimSet(FaceFXData);
							}
						}
						PC->ClientGUDSReferences(i).FaceFXData = FaceFXData;
					}
					return;
				}
			}
			// if it wasn't found, then the GUD bank was requested to be unloaded before loading completed
			// we do nothing here, so GC will eventually toss it
		}
	}
}

void AGearPC::ClientLoadGUDBank(FName BankName)
{
	if (GWorld->IsServer())
	{
		debugf(NAME_Error, TEXT("ClientLoadGUDBank() called on server!"));
	}
	else
	{
		// hrm, we don't have a GUID here. That'll be a problem if we ever support mods.
		FString BankPackageName = BankName.ToString();
		FString BankLocalizedPackageName = BankPackageName + LOCALIZED_SEEKFREE_SUFFIX;
		FString FileName;
		if (!GPackageFileCache->FindPackageFile(*BankPackageName, NULL, FileName, NULL))
		{
			debugf(NAME_DevStreaming, TEXT("Failed to find package for GUDS bank '%s' requested by server"), *BankName.ToString());
		}
		else
		{
			if (GPackageFileCache->FindPackageFile(*BankLocalizedPackageName, NULL, FileName, NULL))
			{
				// we don't need the callback for the _LOC file as the base is queued immediately after it
				LoadPackageAsync(BankLocalizedPackageName, NULL, (void*)this, NULL);
			}
			LoadPackageAsync(BankPackageName, ClientGUDBankLoadCallback, (void*)this, NULL);
			// add the package to the list now so that if it gets removed before the package is done loading, we know to immediately discard it
			// sanity check to make sure we don't have duplicates
			for (INT i = 0; i < ClientGUDSReferences.Num(); i++)
			{
				if (ClientGUDSReferences(i).PackageName == BankName)
				{
					return;
				}
			}
			INT i = ClientGUDSReferences.AddZeroed(1);
			ClientGUDSReferences(i).PackageName = BankName;
		}
	}
}

void AGearPC::ClientUnloadGUDBank(FName BankName)
{
	if (GWorld->IsServer())
	{
		debugf(NAME_Error, TEXT("ClientUnloadGUDBank() called on server!"));
	}
	else
	{
		FName BankName_SF = FName(*(BankName.ToString() + TEXT("_SF")));
		for (INT i = 0; i < ClientGUDSReferences.Num(); i++)
		{
			if (ClientGUDSReferences(i).PackageName == BankName_SF)
			{
				// unmount FaceFX data if necessary
				if (ClientGUDSReferences(i).Bank != NULL && ClientGUDSReferences(i).FaceFXData != NULL)
				{
					for (APawn* P = WorldInfo->PawnList; P != NULL; P = P->NextPawn)
					{
						AGearPawn* GP = Cast<AGearPawn>(P);
						if ( GP != NULL && GP->Mesh != NULL && GP->Mesh->SkeletalMesh != NULL && GP->Mesh->SkeletalMesh->FaceFXAsset != NULL &&
							GP->MasterGUDBankClassNames.Num() > 0 && GP->MasterGUDBankClassNames.ContainsItem(ClientGUDSReferences(i).Bank->SourceGUDBankPath) )
						{
							GP->Mesh->SkeletalMesh->FaceFXAsset->UnmountFaceFXAnimSet(ClientGUDSReferences(i).FaceFXData);
						}
					}
				}
				ClientGUDSReferences.Remove(i);
				return;
			}
		}
		debugf(NAME_Warning, TEXT("GUDS package %s unloaded by server wasn't in our list"), *BankName.ToString());
	}
}

void AGUDManager::ReplicateLoadedGUDBanks(AGearPC* PC)
{
	// send loaded banks
	for (TMap<FString,FGUDBankCollection>::TConstIterator It(GUDBankCollectionsMap); It; ++It)
	{
		FGUDBankCollection const* BC = &It.Value();
		for (INT i = 0; i < BC->LoadedAssets.Num(); i++)
		{
			if (BC->LoadedAssets(i).Bank != NULL)
			{
				// all of the other paths already have the "_SF" and this is assumed by ClientLoadGUDBank() so we need to append it here
				PC->eventClientLoadGUDBank(FName(*(BC->LoadedAssets(i).Bank->GetOutermost()->GetName() + TEXT("_SF"))));
			}
		}
	}
	// send banks pending load
	for (TMap<FString,FGUDSpeakers>::TConstIterator It(PendingGUDLoadsMap); It; ++It)
	{
		PC->eventClientLoadGUDBank(FName(*It.Key()));
	}
}

void AGUDManager::GUDBrowserMarkLineLooksGood()
{
	INT GUDLineIndex = eventGUDBrowserGetCurrentGUDLineIndex();
	if (GUDLineIndex != INDEX_NONE)
	{
		UGUDBank* DefBank = Cast<UGUDBank>(GUDBrowser_CurrentMasterBankClass->GetDefaultObject()); 
		if (DefBank)
		{
			DefBank->GUDLines(GUDLineIndex).bCookerProcessed = UCONST_GUDBROWSER_LOOKSGOOD;
			debugf(TEXT("** GUDSBrowser: Cue %s marked as LOOKS GOOD."), *GetFullNameSafe(DefBank->GUDLines(GUDLineIndex).Audio));
		}
	}
}
void AGUDManager::GUDBrowserMarkLineLooksBad()
{
	INT GUDLineIndex = eventGUDBrowserGetCurrentGUDLineIndex();
	if (GUDLineIndex != INDEX_NONE)
	{
		UGUDBank* DefBank = Cast<UGUDBank>(GUDBrowser_CurrentMasterBankClass->GetDefaultObject()); 
		if (DefBank)
		{
			DefBank->GUDLines(GUDLineIndex).bCookerProcessed = UCONST_GUDBROWSER_LOOKSBAD;
			debugf(TEXT("** GUDSBrowser: Cue %s marked as LOOKS BAD."), *GetFullNameSafe(DefBank->GUDLines(GUDLineIndex).Audio));
		}
	}
}

UBOOL AGearPawn::SpeakLine(class AGearPawn* Addressee,class USoundCue* Audio,const FString& DebugText,FLOAT DelaySec,BYTE Priority,BYTE IntCondition,UBOOL bNoHeadTrack,INT BroadcastFilter,UBOOL bSuppressSubtitle,FLOAT InExtraHeadTrackTime,UBOOL bClientSide)
{
	// shouldn't be calling this with replicated lines on client (it goes directly to PlayQueuedSpeakLine())
	if (!bClientSide && (Role < ROLE_Authority))
	{
		warnf(TEXT("** INVALID SPEAKLINE ** (Pawn=%s, Cue=%s)"), *GetNameSafe(this), *GetNameSafe(Audio));
		//check(FALSE);
	}

	UBOOL bInterrupt = FALSE;
	// look for conditions where we should interrupt
	switch (IntCondition)
	{
	case SIC_IfHigher:
		bInterrupt = (Priority > CurrentSpeechPriority);
		break;
	case SIC_IfSameOrHigher:
		bInterrupt = (Priority >= CurrentSpeechPriority);
		break;
	case SIC_Always:
		bInterrupt = TRUE;
		break;
	}

	SpeechLog(TEXT("SpeakLine(): Cue: %s bSpeaking=%d, bInterrupt=%d"), *GetNameSafe(Audio), bSpeaking, bInterrupt);

	if (!bSpeaking || bInterrupt)
	{
		QueuedSpeakLineParams.Addressee = Addressee;
		QueuedSpeakLineParams.Audio = Audio;
		QueuedSpeakLineParams.DebugText = DebugText;
		QueuedSpeakLineParams.bNoHeadTrack = bNoHeadTrack;
		QueuedSpeakLineParams.Priority = Priority;
		QueuedSpeakLineParams.BroadcastFilter = ESpeakLineBroadcastFilter(BroadcastFilter);
		QueuedSpeakLineParams.bSuppressSubtitle = bSuppressSubtitle;
		QueuedSpeakLineParams.DelayTime = DelaySec;
		QueuedSpeakLineParams.ExtraHeadTrackTime = InExtraHeadTrackTime;
		if (!bClientSide)
		{
			ReplicatedSpeakLineParams = QueuedSpeakLineParams;
			bNetDirty = TRUE;
			bForceNetUpdate = TRUE;
		}

		if (DelaySec > 0.f)
		{
			// play later
			SetTimer(DelaySec, FALSE, FName(TEXT("PlayQueuedSpeakLine")));
		}
		else
		{
			// play now!
			PlayQueuedSpeakLine();
		}

		return TRUE;
	}

	return FALSE;

}

void AGearPawn::PlayQueuedSpeakLine()
{
	// SpeechPriority values are scaled up by this value and then modified by the distance from player to sound source.
	// Currently Min/Max sound radius values are 2000/6000; SubtitlePriorityScale must always be at least as big as
	// the largest Max for a localized sound.
	FLOAT const SubtitlePriorityScale = 10000.f;

	// copy cached params to the ones we'll really be using
	// @line doesn't do anything, see CurrentSpeaklineparams = QueuedSpeakLineParams below
	//CurrentSpeakLineParams = ReplicatedSpeakLineParams;

	//`log("Inside PlayQueuedSpeakLine"@CurrentSpeakLineParams.Audio);

	// find the GUDManager, might need it later.
	AGUDManager* GUDSManager = NULL;
	if (ROLE_Authority == Role)
	{
		AGearGame* const GearGame = Cast<AGearGame>(WorldInfo->Game);
		if (GearGame)
		{
			GUDSManager = GearGame->UnscriptedDialogueManager;
		}
	}

	// if this is a delayed guds line, check with guds to be sure it's still ok to play
	// @fixme, will not stop for multi clients, since they don't have a gudmanager.	 minor for gears 1 though, since it
	// will only affect networked coop
	if ( (Role == ROLE_Authority) && (QueuedSpeakLineParams.Priority == Speech_GUDS) && (QueuedSpeakLineParams.DelayTime > 0.f) )
	{
		if (GUDSManager && !GUDSManager->IsOkToPlayGUDSLine(GetTeamNum()))
		{
			//`log("preventing line from playing, guds temp disabled");
			return;
		}
	}

	// Let vehicle know when driver is speaking
	AGearVehicle* GV = Cast<AGearVehicle>(DrivenVehicle);
	if(GV)
	{
		GV->eventDriverSpeaking();
	}

	// handle current speech, if any
	if (CurrentlySpeakingLine)
	{
		//`log("fading out"@CurrentlySpeakingLine.SoundCue@"in favor of"@CurrentSpeakLineParams.Audio);
		CurrentlySpeakingLine->FadeOut(0.2f, 0.f);
		eventSpeakLineFinished();
	}

	// copy cached params to the ones we'll really be using
	CurrentSpeakLineParams = QueuedSpeakLineParams;

	// playing competitive mp?
	UBOOL bVersusMulti = FALSE;
	if (WorldInfo->GRI && WorldInfo->GRI->GameClass)
	{
		if ( WorldInfo->GRI->GameClass->IsChildOf(AGearGameMP_Base::StaticClass()) )
		{
			bVersusMulti = TRUE;
		}
	}

	// play the audio
	FLOAT SpeakTime = 0.f;
	if (CurrentSpeakLineParams.Audio)
	{
		if (bVersusMulti)
		{
			if (ShouldFilterOutSpeech(CurrentSpeakLineParams.BroadcastFilter, CurrentSpeakLineParams.Addressee))
			{
//				`log(self@"Skipping line"@IsLocallyControlled());
				return;
			}
		}

		// If we have a FaceFX animation hooked up, play that instead
		// If the Face FX animation fails, fall back to just playing the sound. A log Warning will be issued in that case
		if ( (CurrentSpeakLineParams.Audio->FaceFXAnimName != TEXT("")) &&
			 eventPlayActorFaceFXAnim(CurrentSpeakLineParams.Audio->FaceFXAnimSetRef, CurrentSpeakLineParams.Audio->FaceFXGroupName, CurrentSpeakLineParams.Audio->FaceFXAnimName, CurrentSpeakLineParams.Audio) )
		{
			SpeechLog(TEXT("PlayQueuedSpeakLine(): Played line %s (via FaceFX)"), *GetNameSafe(CurrentSpeakLineParams.Audio));
			CurrentlySpeakingLine = eventGetFaceFXAudioComponent();
			CurrentlySpeakingLine->bSuppressSubtitles = CurrentSpeakLineParams.bSuppressSubtitle || ShouldSuppressSubtitlesForQueuedSpeakLine(bVersusMulti);
			// jack priority for scripted lines
			CurrentlySpeakingLine->bAlwaysPlay = (CurrentSpeakLineParams.Priority >= Speech_Scripted);
			CurrentlySpeakingLine->PitchMultiplier = SpeechPitchMultiplier;
		}
		else
		{
			CurrentlySpeakingLine = CreateAudioComponent( CurrentSpeakLineParams.Audio, FALSE, TRUE );
			if (CurrentlySpeakingLine)
			{
				AttachComponent(CurrentlySpeakingLine);

				CurrentlySpeakingLine->bAutoDestroy = TRUE;
				CurrentlySpeakingLine->Location = Location;
				CurrentlySpeakingLine->bSuppressSubtitles = CurrentSpeakLineParams.bSuppressSubtitle || ShouldSuppressSubtitlesForQueuedSpeakLine(bVersusMulti);

				// @todo: modulate priority based on distance to the player?
				CurrentlySpeakingLine->SubtitlePriority = CurrentSpeakLineParams.Priority * SubtitlePriorityScale;

				// jack priority for scripted lines
				CurrentlySpeakingLine->bAlwaysPlay = (CurrentSpeakLineParams.Priority >= Speech_Scripted);

				CurrentlySpeakingLine->PitchMultiplier = SpeechPitchMultiplier;

				CurrentlySpeakingLine->Play();
				SpeechLog(TEXT("PlayQueuedSpeakLine(): Played line %s (via AudioComponent)"), *GetNameSafe(CurrentSpeakLineParams.Audio));
			}
			else
			{
				SpeechLog(TEXT("*** Failed to create audio component for %s"), *GetNameSafe(CurrentSpeakLineParams.Audio));
			}
		}

		SpeakTime = CurrentSpeakLineParams.Audio->GetCueDuration() + 0.2f;
	}
	else
	{
		// time enough to read
		SpeakTime = 2.f;
	}

	if ( CurrentSpeakLineParams.Addressee && !CurrentSpeakLineParams.bNoHeadTrack && (CurrentSpeakLineParams.Addressee->HeadBoneNames.Num() > 0) )
	{
		// make sure previous timers don't turn off headtracking prematurely
		ClearTimer(FName(TEXT("DisableHeadTrack")));
		eventSetHeadTrackActor(CurrentSpeakLineParams.Addressee, CurrentSpeakLineParams.Addressee->HeadBoneNames(0), TRUE);
	}

	// debug text
	//if ( CurrentSpeakLineParams.DebugText != TEXT("") )
	//{
	//	// find the player
	//	if (GEngine->GamePlayers.Num() > 0)
	//	{
	//		APlayerController* AnyPC = GEngine->GamePlayers(0)->Actor;
	//		FString Msg = FString::Printf(TEXT("%s: %s"), *GetNameSafe(this), CurrentSpeakLineParams.DebugText);
	//		AnyPC->eventClientMessage(Msg);
	//		//PC.GetPlayerViewPoint(CamLoc, CamRot);
	//		//`log(Self@"(GUDS):"@CurrentSpeakLineParams.DebugText@"Range:"@VSize(CamLoc-Location));
	//	}
	//}

	CurrentSpeechPriority = CurrentSpeakLineParams.Priority;

	if ( (ROLE_Authority == Role) && (CurrentSpeechPriority >= Speech_GUDS) && !bVersusMulti && GUDSManager )
	{
		// tell guds that speech is going on, in case it wants to shuush for a little bit
		GUDSManager->NotifyExclusiveSpeech(this, CurrentSpeakLineParams.Audio, CurrentSpeechPriority);
	}

	ClearTimer(FName(TEXT("PlayQueuedSpeakLine")));		// just in case
	SetTimer(SpeakTime, FALSE, FName(TEXT("SpeakLineFinished")));
	
	bSpeaking = TRUE;
	
	// notify speech manager of dialogue
	AGearGRI* const GRI = Cast<AGearGRI>(WorldInfo->GRI);
	if (GRI && GRI->SpeechManager)
	{
		GRI->SpeechManager->eventNotifyDialogueStart(this, CurrentSpeakLineParams.Addressee, CurrentSpeakLineParams.Audio, CurrentSpeakLineParams.Priority);
	}
}

UBOOL AGearPawn::ShouldSuppressSubtitlesForQueuedSpeakLine(UBOOL bVersusMulti)
{
	if (bVersusMulti)
	{
		// no subtitles in multi
		return TRUE;
	}
	else
	{
		// in SP, locust lines get no subtitles
		if ( GetTeamNum() != 0 /*&& (CurrentSpeakLineParams.Priority < Speech_Scripted)*/ )
		{
			return TRUE;
		}
	}

	// any more conditions for controlling subtitles here?
	return FALSE;
}


UBOOL AGearPawn::ShouldFilterOutSpeech(BYTE Filter,class AGearPawn* Addressee)
{
	switch (Filter)
	{
	case SLBFilter_None:
		return FALSE;

	case SLBFilter_SpeakerTeamOnly:
		{
			// accept speech if there is a local player on the same team, reject otherwise
			for (INT PlayerIdx=0; PlayerIdx<GEngine->GamePlayers.Num(); ++PlayerIdx)
			{
				APlayerController* const PC = GEngine->GamePlayers(PlayerIdx)->Actor;
				if (PC->GetTeamNum() == GetTeamNum())
				{
					return FALSE;
				}
			}
		}
		return TRUE;

	case SLBFilter_SpeakerOnly:
		// accept speech if spoken by a local player, reject otherwise
		return !IsLocallyControlled();

	case SLBFilter_SpeakerAndAddresseeOnly:
		return !( IsLocallyControlled() || (Addressee && Addressee->IsLocallyControlled()) );
	}

	// just in case
	return FALSE;
}





void AGearSpeechManager::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	// periodically run through the dialogue stack and clean house
	if ( (WorldInfo->TimeSeconds - LastUpdateTime) > TimeBetweenUpdates )
	{
		for (INT Idx=0; Idx<DialogueStack.Num(); ++Idx)
		{
			FActiveDialogueLine& Line = DialogueStack(Idx);

			UBOOL bRemoveRecord = TRUE;

			if (Line.Audio && Line.Speaker)
			{
				AGearPawn* const GPSpeaker = Cast<AGearPawn>(Line.Speaker);
				if ( GPSpeaker &&
					 GPSpeaker->CurrentlySpeakingLine && 
					 (GPSpeaker->CurrentlySpeakingLine->SoundCue == Line.Audio) && 
					 GPSpeaker->CurrentlySpeakingLine->IsPlaying() )
				{
					bRemoveRecord = FALSE;
				}
				else
				{
					AGearRemoteSpeaker* const RemoteSpeaker = Cast<AGearRemoteSpeaker>(Line.Speaker);
					if ( RemoteSpeaker &&
						 RemoteSpeaker->CurrentlySpeakingLine && 
						 (RemoteSpeaker->CurrentlySpeakingLine->SoundCue == Line.Audio) && 
						 RemoteSpeaker->CurrentlySpeakingLine->IsPlaying() )
					{
						bRemoveRecord = FALSE;
					}
				}
			}

			if (bRemoveRecord)
			{
				DialogueStack.Remove(Idx, 1);
				Idx--;
			}
		}

		LastUpdateTime = WorldInfo->TimeSeconds;
	}

}


